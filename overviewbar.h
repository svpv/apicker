#ifndef OVERVIEWBAR_H
#define OVERVIEWBAR_H

#include "waveform.h"

class OverviewBar : public Gtk::DrawingArea
{
public:
    OverviewBar(Waveform *wf, Glib::RefPtr<Gtk::Adjustment> m_aj) :
	m_wf(wf), m_aj(m_aj),
	m_avg(NULL), m_avgcnt(0), m_avgmax(0),
	m_drag(false)
    {
	set_size_request(1200, 48);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
	m_aj->signal_value_changed().connect(
		sigc::mem_fun(*this, &OverviewBar::queue_draw));
    }

    ~OverviewBar()
    {
	free(m_avg);
    }
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
	const size_t w = get_width();
	const size_t h = get_height();

	// page small image width
	double w1 = w / 16;
	// page small image left start
	double x1 = w / 2 - w1 / 2;

	m_aj->set_page_size(w);
	size_t ix0 = m_aj->get_value();

	// adjust for the beginning
	if (ix0 / 16 < x1)
	    x1 = ix0 / 16;

	if (m_avgcnt == 0)
	    mk_avg(w);

	// ai is the index in avg waveform
	size_t ai = ix0 / 8 - x1 * 2;

	// adjust for the end
	ssize_t lack = ai + w * 2 - m_avgcnt;
	if (lack > 0) {
	    ai -= lack;
	    x1 += lack / 2;
	}

	draw_bg(cr, w, h, x1, w1);
	draw_wf(cr, w, h, ai);
	return true;
    }

    void draw_bg(const Cairo::RefPtr<Cairo::Context> &cr,
	    const size_t w, const size_t h,
	    const size_t x1, const size_t w1)
    {
	cr->save();

	cr->set_source_rgba(0.5, 0.5, 0.5, 1.0);
	cr->rectangle(0, 0, w, h);
	cr->fill();

	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->rectangle(x1, 0, w1, h);
	cr->fill();

	cr->restore();
    }

    void mk_avg(const size_t w)
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

    void click(double x)
    {
	size_t w = get_width();
	double w1 = w / 16;

	m_aj->set_page_size(w);
	double ix0 = m_aj->get_value();

	double x1 = w / 2 - w1 / 2;

	if (ix0 / 16 < x1)
	    x1 = ix0 / 16;

	size_t ai = ix0 / 8 - x1 * 2;
	ssize_t lack = ai + w * 2 - m_avgcnt;
	if (lack > 0) {
	    ai -= lack;
	    x1 += lack / 2;
	}

	ix0 += (x - x1) * 16;

	m_aj->set_value(ix0);
	queue_draw();
    }

    bool on_button_press_event(GdkEventButton *event) override
    {
	click(event->x);
	m_drag = true;
	return true;
    }

    bool on_button_release_event(GdkEventButton *event) override
    {
	m_drag = false;
	return true;
    }

    bool on_motion_notify_event(GdkEventMotion* event) override
    {
	if (m_drag)
	    click(event->x);
	return true;
    }
private:
    Waveform *m_wf;
    Glib::RefPtr<Gtk::Adjustment> m_aj;
    double *m_avg;
    size_t m_avgcnt;
    double m_avgmax;
    bool m_drag;
};

#endif
