#ifndef WAVEFORMVIEWBASE_H
#define WAVEFORMVIEWBASE_H

#include <limits.h>
#include <vector>
#include <gtkmm/drawingarea.h>
#include <glibmm/main.h>

class WaveformViewBase : public Gtk::DrawingArea
{
protected:
    WaveformViewBase() :
	m_drag(false), m_mark(NOMARK),
	m_cursor_x_centered(0), m_cursor_x_current(0), m_cursor_x_from_click(-9)
    {
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    }
public:
    static const unsigned NOMARK = UINT_MAX;

    // enable short passage mode at around the mark, without scrolling
    void short_passage(unsigned mark = NOMARK)
    {
	m_mark = mark;
    }
protected:

    void draw_cu(const Cairo::RefPtr<Cairo::Context> &cr, size_t x, size_t h1, bool dash = false)
    {
	const size_t h = get_height();
	cr->set_source_rgba(0, 1.0, 0.3, 1.0);
	if (dash) {
	    const double dash_values[] = { 4, 4 };
	    const std::vector<double> dash_vector(dash_values, dash_values + 2);
	    cr->set_dash(dash_vector, 3);
	}
	cr->move_to(x, h);
	cr->line_to(x, h - h1);
	cr->stroke();
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
    unsigned m_mark;
    sigc::connection m_tick;

    double m_cursor_x_centered;
    double m_cursor_x_current;
    double m_cursor_x_from_click;
};

#endif
