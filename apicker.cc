class Time
{
public:
    Time(unsigned csec)
    {
	char *p = buf;
	unsigned h = csec / 100 / 3600;
	csec -= h * 100 * 3600;
	if (h > 9)
	    h = 9;
	*p++ = '0' + h;
	*p++ = ':';
	unsigned min = csec / 100 / 60;
	csec -= min * 100 * 60;
	*p++ = '0' + min / 10;
	*p++ = '0' + min % 10;
	*p++ = ':';
	unsigned sec = csec / 100;
	csec -= sec * 100;
	*p++ = '0' + sec / 10;
	*p++ = '0' + sec % 10;
	*p++ = '.';
	*p++ = '0' + csec / 10;
	*p++ = '0' + csec % 10;
	*p++ = '\0';
    }

    const char *str()
    {
	buf[7] = '.';
	return buf;
    }

    const char *strsec()
    {
	buf[7] = '\0';
	return buf;
    }

protected:
    char buf[sizeof("0:00:00.00")];
};

#include <gtkmm.h>
#include "waveform.h"

class WaveformRenderer : public Gtk::DrawingArea
{
public:
    WaveformRenderer(Waveform *wf) : m_wf(wf)
    {
	set_size_request(1200, 280);
    }
    
    ~WaveformRenderer()
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
	    Time t(i);
	    cr->show_text(t.strsec());
	}

	cr->stroke();

	cr->move_to(0, h);
	for (size_t i = 0; i < w; i++)
	    cr->line_to(i, h - m_wf->m_v[i]);
	cr->line_to(w, h);
	cr->fill();
    }

    Waveform *m_wf;
    sigc::signal<int> scrolling;
};

int main(int argc, char *argv[])
{
    Gtk::Main app(argc, argv);

    Waveform wf(argv[1]);
    WaveformRenderer wr(&wf);

    Glib::RefPtr<Gtk::Adjustment> aj(Gtk::Adjustment::create(0, 0, wf.m_n));
    Gtk::Scrollbar sb(aj);

    Gtk::VBox vb;
    vb.pack_start(wr);
    vb.pack_start(sb);

    Gtk::Window win;
    win.add(vb);
    win.show_all();
    app.run(win);
    return 0;
}
