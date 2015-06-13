#ifndef OVERVIEWBAR_H
#define OVERVIEWBAR_H

#include "waveform.h"
#include "cursor.h"

class OverviewBar : public Gtk::DrawingArea
{
public:
    OverviewBar(Waveform *wf) :
	m_wf(wf),
	m_avg(NULL), m_avgcnt(0), m_avgmax(0),
	m_ix(0), m_x0(0), m_w0(0)
    {
	set_size_request(1200, 48);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
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

	draw_bg(cr, w, h);
	draw_wf(cr, w, h);
	return true;
    }

    void draw_bg(const Cairo::RefPtr<Cairo::Context> &cr, const size_t w, const size_t h)
    {
	m_x0 = m_ix * w / (double) m_wf->m_n;
	m_w0 = w * w / (double) m_wf->m_n;

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
	double x = event->x;
	m_ix = x / get_width() * m_wf->m_n;
	queue_draw();
	return true;
    }

    bool on_motion_notify_event(GdkEventMotion* event) override
    {
	GdkWindow *w = get_window()->gobj();
	if (event->x < m_x0 || event->x > m_x0 + m_w0)
	    gdk_window_set_cursor(w, NULL);
	else {
	    GdkCursor *c = cursor_get(CURSOR_HAND_OPEN);
	    gdk_window_set_cursor(w, c);
	    gdk_cursor_unref(c);
	}
	return true;
    }
private:
    Waveform *m_wf;
    double *m_avg;
    size_t m_avgcnt;
    double m_avgmax;
    size_t m_ix;
    double m_x0;
    double m_w0;
};

#endif
