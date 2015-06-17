#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <gtkmm.h>
#include "waveform.h"
#include "csec.h"

class WaveformView : public Gtk::DrawingArea
{
public:
    WaveformView(Waveform *wf, Glib::RefPtr<Gtk::Adjustment> aj) :
	m_wf(wf), m_aj(aj),
	m_cursor_x_centered(0), m_cursor_x_current(0), m_cursor_x_from_click(-9),
	m_wf_x(0), m_drag(false), m_scrolling(true)
    {
	set_size_request(1200, 280);
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
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

    void click(double x)
    {
	const size_t w = get_width();

	// bound x
	if (x < 0) x = 0;
	if (x > w) x = w;

	// idicate cursor_x position from click
	m_cursor_x_from_click = x;

	// set new position in the stream
	double diff = x - m_cursor_x_current;
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
		    sigc::mem_fun(*this, &WaveformView::tick), 10);
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
	double diff = m_cursor_x_current - m_cursor_x_centered;
	if (fabs(diff) < 5) {
	    m_cursor_x_from_click = -9;
	    m_tick.block();
	    goto out;
	}
	if (diff > 0)
	    diff -= 4;
	else
	    diff += 4;
	diff *= 0.99;
	m_cursor_x_from_click = m_cursor_x_centered + diff;
    out:
	queue_draw();
	return true;
    }

    Waveform *m_wf;
    Glib::RefPtr<Gtk::Adjustment> m_aj;
    double m_cursor_x_centered;
    double m_cursor_x_current;
    double m_cursor_x_from_click;
    size_t m_wf_x;
    sigc::connection m_tick;
    bool m_drag;
    bool m_scrolling;
};

#endif
