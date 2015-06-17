#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <gtkmm/adjustment.h>
#include "waveform.h"
#include "waveformviewbase.h"
#include "csec.h"

class WaveformView : public WaveformViewBase
{
public:
    WaveformView(Waveform *wf, Glib::RefPtr<Gtk::Adjustment> aj) :
	m_wf(wf), m_aj(aj),
	m_wf_x(0), m_scrolling(true)
    {
	set_size_request(1200, 280);
	m_aj->signal_value_changed().connect(
		sigc::mem_fun(*this, &WaveformView::queue_draw));
    }

    void scrolling(bool yes)
    {
	m_scrolling = yes;
    }
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
	size_t w = get_width();
	size_t h = get_height();

	cr->move_to(0, h - 256);
	cr->line_to(w, h - 256);

	cr->set_font_size(13);

	Cairo::TextExtents extents;
	cr->get_text_extents("0:00:00", extents);

	// desired cursor position
	size_t cursor_x = w / 2;

	// current position in waveform
	size_t wf_c = m_aj->get_value();

	// leftmost position in waveform
	size_t wf_x = wf_c - w / 2;

	// if waveform position is too small, move cursor left
	if (wf_x > wf_c) {
	    wf_x = 0;
	    cursor_x = wf_c;
	}

	// if waveform position is too big, move cursor right
	if (wf_x + w > m_wf->m_n) {
	    wf_x = m_wf->m_n - w;
	    cursor_x = w - (m_wf->m_n - wf_c);
	}

	// when automatic scrolling disabled, use previous wf_x
	if (!m_scrolling) {
	    wf_x = m_wf_x;
	    cursor_x = wf_c - wf_x;
	}

	// cursor should be displayed here
	m_cursor_x_centered = cursor_x;

	// wavefrom displayed starting from here
	m_wf_x = wf_x;

	// however, when the user clicks, the new position is centered gradually
	if (m_cursor_x_from_click > -9) {
	    cursor_x = m_cursor_x_from_click;
	    wf_x = wf_c - cursor_x;
	    if (wf_x > wf_c)
		wf_x = 0;
	}

	// cursor is displayed here
	m_cursor_x_current = cursor_x;

	// draw second marks
	size_t r = wf_x % 100;
	for (size_t i = r ? 100 - r : 0; i < w; i += 100) {
	    cr->move_to(i, h - 256);
	    cr->line_to(i, h - 256 - 3);
	    cr->move_to(i - extents.width / 2, h - 256 - extents.height / 2 - 4);
	    CSec t(i + wf_x);
	    cr->show_text(t.strsec());
	}

	cr->stroke();

	// draw waveform
	cr->move_to(0, h);
	for (size_t i = 0; i < w; i++)
	    cr->line_to(i, h - m_wf->m_v[i + wf_x]);
	cr->line_to(w, h);
	cr->fill();

	// draw cursor
	cr->set_source_rgba(0, 1, 0.5, 1.0);
	cr->move_to(cursor_x, h);
	cr->line_to(cursor_x, h - 255);
	cr->stroke();

	return true;
    }

    void click(double x) override
    {
	double diff = click0(x);
	m_aj->set_value(m_aj->get_value() + diff);
	queue_draw();
    }

    Waveform *m_wf;
    Glib::RefPtr<Gtk::Adjustment> m_aj;
    size_t m_wf_x;
    bool m_scrolling;
};

#endif
