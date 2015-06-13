#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <gtkmm.h>
#include "waveform.h"
#include "csec.h"

class WaveformView : public Gtk::DrawingArea
{
public:
    WaveformView(Waveform *wf) :
	m_wf(wf), m_ix0(0)
    {
	set_size_request(1200, 280);
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

	for (size_t i = 0; i < w; i += 100) {
	    cr->move_to(i, h - 256);
	    cr->line_to(i, h - 256 - 3);
	    cr->move_to(i - extents.width / 2, h - 256 - extents.height / 2 - 4);
	    CSec t(i);
	    cr->show_text(t.strsec());
	}

	cr->stroke();

	cr->move_to(0, h);
	for (size_t i = 0; i < w; i++)
	    cr->line_to(i, h - m_wf->m_v[i + m_ix0]);
	cr->line_to(w, h);
	cr->fill();
    }

    void newix0(size_t ix0)
    {
	m_ix0 = ix0;
	queue_draw();
    }
protected:
    Waveform *m_wf;
    size_t m_ix0;
};

#endif
