#ifndef OVERVIEWBAR_H
#define OVERVIEWBAR_H

#include "waveform.h"

class OverviewBar : public Gtk::DrawingArea
{
public:
    OverviewBar(Waveform *wf) :
	m_wf(wf),
	m_avg(NULL), m_avgcnt(0), m_avgmax(0),
	m_ix0(0), m_x0(0), m_w0(0),
	m_drag(false)
    {
	set_size_request(1200, 48);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    }

    ~OverviewBar()
    {
	free(m_avg);
    }
    sigc::signal<void, size_t> sigix0;
protected:
    void setx0(double x)
    {
	if (x < 0)
	    x = 0;
	const size_t w = get_width();
	const size_t max_ix = m_wf->m_n - w;
	m_ix0 = x * m_wf->m_n / get_width();
	if (m_ix0 > max_ix)
	    m_ix0 = max_ix;
	sigix0.emit(m_ix0);
	m_x0 = m_ix0 * w / (double) m_wf->m_n;
	m_w0 = w * w / (double) m_wf->m_n;
    }

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
	const size_t w = get_width();
	const size_t h = get_height();

	if (m_w0 == 0)
	    setx0(0);
	else {
	    m_x0 = m_ix0 * w / (double) m_wf->m_n;
	    m_w0 = w * w / (double) m_wf->m_n;
	}

	draw_bg(cr, w, h);
	draw_wf(cr, w, h);
	return true;
    }

    void draw_bg(const Cairo::RefPtr<Cairo::Context> &cr, const size_t w, const size_t h)
    {
	cr->save();

	cr->set_source_rgba(0.5, 0.5, 0.5, 1.0);
	cr->rectangle(0, 0, w, h);
	cr->fill();

	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->rectangle(m_x0, 0, m_w0, h);
	cr->fill();

	cr->restore();
    }

    void mk_avg(const size_t w)
    {
	const double imul2 = m_wf->m_n / (2.0 * w);
	m_avg = (double *) realloc(m_avg, 2 * w * sizeof(double));
	m_avgcnt = 2 * w;

	m_avgmax = 0;
	size_t j = 0;
	for (size_t i = 0; i < 2 * w; i++) {
	    size_t nj = imul2 * (i + 0.5);
	    double dj = nj - j;
	    double sj = 0;
	    while (j < nj)
		sj += m_wf->m_v[j++];
	    if (dj)
		sj /= dj;
	    if (sj > m_avgmax)
		m_avgmax = sj;
	    m_avg[i] = sj;
	}
    }

    void draw_wf(const Cairo::RefPtr<Cairo::Context> &cr, const size_t w, const size_t h)
    {
	if (m_avgcnt != 2 * w)
	    mk_avg(w);

	cr->move_to(0, h);

	for (size_t i = 0; i < w; i++) {
	    cr->line_to(i + 0.25, h / m_avgmax * m_avg[2 * i]);
	    cr->line_to(i + 0.75, h / m_avgmax * m_avg[2 * i + 1]);
	}
	cr->line_to(w, h);
	cr->fill();
    }

    bool on_button_press_event(GdkEventButton *event) override
    {
	setx0(event->x);
	queue_draw();
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
	if (m_drag) {
	    setx0(event->x);
	    queue_draw();
	}
	return true;
    }
private:
    Waveform *m_wf;
    double *m_avg;
    size_t m_avgcnt;
    double m_avgmax;
    size_t m_ix0;
    double m_x0;
    double m_w0;
    bool m_drag;
};

#endif
