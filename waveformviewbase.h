#ifndef WAVEFORMVIEWBASE_H
#define WAVEFORMVIEWBASE_H

#include <gtkmm/drawingarea.h>
#include <glibmm/main.h>

class WaveformViewBase : public Gtk::DrawingArea
{
protected:
    WaveformViewBase() :
	m_drag(false),
	m_cursor_x_centered(0), m_cursor_x_current(0), m_cursor_x_from_click(-9)
    {
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    }

    virtual void click(double x) = 0;

    double click0(double x)
    {
	const size_t w = get_width();

	// bound x
	if (x < 0) x = 0;
	if (x > w) x = w;

	// idicate cursor_x position from click
	m_cursor_x_from_click = x;

	double diff = x - m_cursor_x_current;
	return diff;
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
		    sigc::mem_fun(*this, &WaveformViewBase::tick), 10);
	return true;
    }

    bool on_motion_notify_event(GdkEventMotion* event) override
    {
	if (m_drag)
	    click(event->x);
	return true;
    }

    bool m_drag;
    sigc::connection m_tick;

    double m_cursor_x_centered;
    double m_cursor_x_current;
    double m_cursor_x_from_click;
};

#endif
