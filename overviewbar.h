#ifndef OVERVIEWBAR_H
#define OVERVIEWBAR_H

#include "waveform.h"

class OverviewBar : public Gtk::DrawingArea
{
public:
    OverviewBar(Waveform *wf)
	: m_wf(wf), m_avg(NULL), m_ix(0)
    {
	set_size_request(1200, 48);
	add_events(Gdk::BUTTON_PRESS_MASK);
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
	double x0 = m_ix * w / (double) m_wf->m_n;
	double smallw = w * w / (double) m_wf->m_n;

	cr->save();

	cr->set_source_rgba(0.5, 0.5, 0.5, 1.0);
	cr->rectangle(0, 0, w, h);
	cr->fill();

	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
	cr->rectangle(x0, 0, smallw, h);
	cr->fill();

	cr->restore();
    }

    void draw_wf(const Cairo::RefPtr<Cairo::Context> &cr, const size_t w, const size_t h)
    {
	const double imul2 = m_wf->m_n / (2.0 * w);
	m_avg = (double *) realloc(m_avg, 2 * w * sizeof(double));

	double max = 0;
	size_t j = 0;
	for (size_t i = 0; i < 2 * w; i++) {
	    size_t nj = imul2 * (i + 0.5);
	    double dj = nj - j;
	    double sj = 0;
	    while (j < nj)
		sj += m_wf->m_v[j++];
	    if (dj)
		sj /= dj;
	    if (sj > max)
		max = sj;
	    m_avg[i] = sj;
	}

	cr->move_to(0, h);

	for (size_t i = 0; i < w; i++) {
	    cr->line_to(i + 0.25, h / max * m_avg[2 * i]);
	    cr->line_to(i + 0.75, h / max * m_avg[2 * i + 1]);
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
private:
    Waveform *m_wf;
    double *m_avg;
    size_t m_ix;
};

#endif
