#ifndef OVERVIEWBAR_H
#define OVERVIEWBAR_H

#include "waveform.h"

class OverviewBar : public Gtk::DrawingArea
{
public:
    OverviewBar(Waveform *wf, Glib::RefPtr<Gtk::Adjustment> aj) :
	m_wf(wf), m_aj(aj),
	m_avg(NULL), m_avgcnt(0), m_avgmax(0),
	m_page_x_centered(0), m_page_x_current(0), m_page_x_from_click(-9),
	m_cursor_x(0), m_avg_c(0),
	m_drag(false)
    {
	set_size_request(1200, 48);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
	m_aj->signal_value_changed().connect(
		sigc::mem_fun(*this, &OverviewBar::maybe_queue_draw));
	mk_avg();
    }

    ~OverviewBar()
    {
	free(m_avg);
    }
protected:
    void maybe_queue_draw()
    {
	// current px position in avg waveform
	size_t avg_c = m_aj->get_value() / 16;
	if (avg_c != m_avg_c)
	    queue_draw();
    }

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
	const size_t w = get_width();
	const size_t h = get_height();

	size_t page_w = w / 16;
	size_t page_x = w / 2 - page_w / 2;

	// current px position in avg waveform
	size_t avg_c = m_aj->get_value() / 16;

	// leftmost px position in avg waveform
	size_t avg_x = avg_c - page_x - page_w / 2;

	// if avg position is too small, move page left
	if (avg_x > avg_c) {
	    avg_x = 0;
	    if (avg_c < page_w / 2)
		page_x = 0;
	    else
		page_x = avg_c - page_w / 2;
	}

	// if avg position is too big, move page right
	if (avg_x + w > m_avgcnt / 2) {
	    avg_x = m_avgcnt / 2 - w;
	    if (m_avgcnt / 2 - avg_c < page_w / 2)
		page_x = w - page_w;
	    else
		page_x = w - (m_avgcnt / 2 - avg_c) - page_w / 2;
	}

	// the page should be placed here
	m_page_x_centered = page_x;

	// however, when the user clicks, the new position is centered gradually
	if (m_page_x_from_click > -9) {
	    page_x = m_page_x_from_click;
	    avg_x = avg_c - page_x - page_w / 2;
	}

	// the page is dipsplayed here
	m_page_x_current = page_x;

	// cursor is displayed here
	m_cursor_x = avg_c - avg_x;

	// save current position in avg waveform
	m_avg_c = avg_c;

	draw_bg(cr, w, h, page_x, page_w);
	draw_wf(cr, w, h, avg_x * 2);
	draw_cu(cr, m_cursor_x, h);
	return true;
    }

    void draw_bg(const Cairo::RefPtr<Cairo::Context> &cr,
	    const size_t w, const size_t h,
	    const size_t page_x, const size_t page_w)
    {
	cr->save();

	cr->set_source_rgba(0.5, 0.5, 0.5, 1.0);
	cr->rectangle(0, 0, w, h);
	cr->fill();

	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->rectangle(page_x, 0, page_w, h);
	cr->fill();

	cr->restore();
    }

    void mk_avg()
    {
	m_avgcnt = m_wf->m_n / 8;
	m_avg = (double *) realloc(m_avg, (m_avgcnt + 1) * sizeof(double));

	m_avgmax = 0;
	const unsigned char *p = m_wf->m_v;
	for (size_t i = 0; i < m_avgcnt; i++) {
	    double sum = 0;
	    const unsigned char *p_end = p + 8;
	    while (p < p_end)
		sum += *p++;
	    sum /= 8;
	    if (sum > m_avgmax)
		m_avgmax = sum;
	    m_avg[i] = sum;
	}

	// just in case
	m_avg[m_avgcnt] = 0;
    }

    void draw_wf(const Cairo::RefPtr<Cairo::Context> &cr,
	    const size_t w, const size_t h, size_t ai)
    {
	cr->move_to(0, h);

	for (size_t i = 0; i < w; i++) {
	    cr->line_to(i + 0.25, h - h / m_avgmax * m_avg[ai + 2 * i]);
	    cr->line_to(i + 0.75, h - h / m_avgmax * m_avg[ai + 2 * i + 1]);
	}
	cr->line_to(w, h);
	cr->fill();
    }

    void draw_cu(const Cairo::RefPtr<Cairo::Context> &cr, size_t x, size_t h)
    {
	cr->set_source_rgba(0, 1, 0.5, 1.0);
	cr->move_to(x, 0);
	cr->line_to(x, h);
	cr->stroke();
    }

    void click(double x)
    {
	const size_t w = get_width();
	size_t page_w = w / 16;

	// bound x
	if (x < 0) x = 0;
	if (x > w) x = w;

	// translate click to page_x
	size_t page_x = x - page_w / 2;
	if (page_x > x)
	    page_x = 0;
	if (page_x > w - page_w)
	    page_x = w - page_w;

	// idicate page_x position from click
	m_page_x_from_click = page_x;

	// set new position in the stream
	double diff = (x - m_cursor_x) * 16;
	m_aj->set_value(m_aj->get_value() + diff);

	queue_draw();
    }

    bool on_button_press_event(GdkEventButton *event) override
    {
	if (m_tick)
	    m_tick.block();
	click(event->x);
	m_drag = true;
	return true;
    }

    bool on_button_release_event(GdkEventButton *event) override
    {
	m_drag = false;
	if (m_tick)
	    m_tick.unblock();
	else
	    m_tick = Glib::signal_timeout().connect(
		    sigc::mem_fun(*this, &OverviewBar::tick), 10);
	return true;
    }

    bool on_motion_notify_event(GdkEventMotion* event) override
    {
	if (m_drag)
	    click(event->x);
	return true;
    }

    bool tick()
    {
	double diff = m_page_x_current - m_page_x_centered;
	if (fabs(diff) < 5) {
	    m_page_x_from_click = -9;
	    m_tick.block();
	    goto out;
	}
	if (diff > 0)
	    diff -= 4;
	else
	    diff += 4;
	diff *= 0.99;
	m_page_x_from_click = m_page_x_centered + diff;
    out:
	queue_draw();
	return true;
    }
private:
    Waveform *m_wf;
    Glib::RefPtr<Gtk::Adjustment> m_aj;
    double *m_avg;
    size_t m_avgcnt;
    double m_avgmax;
    double m_page_x_centered;
    double m_page_x_current;
    double m_page_x_from_click;
    size_t m_cursor_x;
    size_t m_avg_c;
    sigc::connection m_tick;
    bool m_drag;
};

#endif
