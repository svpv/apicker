#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <gtkmm.h>
#include "waveform.h"
#include "csec.h"

class WaveformView : public Gtk::DrawingArea
{
public:
    WaveformView(Waveform *wf, Glib::RefPtr<Gtk::Adjustment> aj)
	: m_wf(wf), m_aj(aj)
    {
	set_size_request(1200, 280);
	m_aj->signal_value_changed().connect(
		sigc::mem_fun(*this, &WaveformView::queue_draw));
    }

    ~WaveformView()
    {
    }

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
	if (m_wf->m_v)
	    draw(cr);
	return true;
    }

    void draw(const Cairo::RefPtr<Cairo::Context> &cr)
    {
	size_t w = get_width();
	size_t h = get_height();

	cr->move_to(0, h - 256);
	cr->line_to(w, h - 256);

	cr->set_font_size(13);

	Cairo::TextExtents extents;
	cr->get_text_extents("0:00:00", extents);

	m_aj->set_page_size(w);
	size_t ix0 = m_aj->get_value();
	size_t rem = ix0 % 100;
	for (size_t i = rem ? 100 - rem : 0; i < w; i += 100) {
	    cr->move_to(i, h - 256);
	    cr->line_to(i, h - 256 - 3);
	    cr->move_to(i - extents.width / 2, h - 256 - extents.height / 2 - 4);
	    CSec t(i + ix0);
	    cr->show_text(t.strsec());
	}

	cr->stroke();

	cr->move_to(0, h);
	for (size_t i = 0; i < w; i++)
	    cr->line_to(i, h - m_wf->m_v[i + ix0]);
	cr->line_to(w, h);
	cr->fill();
    }
protected:
    Waveform *m_wf;
    Glib::RefPtr<Gtk::Adjustment> m_aj;
};

#endif
