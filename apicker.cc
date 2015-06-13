#include "waveform.h"
#include "waveformview.h"
#include "overviewbar.h"

int main(int argc, char *argv[])
{
    Gtk::Main app(argc, argv);

    Waveform wf(argv[1]);

    Glib::RefPtr<Gtk::Adjustment> aj(Gtk::Adjustment::create(0, 0, wf.m_n));
    Gtk::Scrollbar sb(aj);

    WaveformView wv(&wf);
    OverviewBar ob(&wf);

    Gtk::VBox vb(false, 1);
    vb.pack_start(wv);
    vb.pack_start(ob);
    vb.pack_start(sb);

    Gtk::Window win;
    win.add(vb);
    win.show_all();
    app.run(win);
    return 0;
}
