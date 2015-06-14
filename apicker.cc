#include <cstring>
#include "waveform.h"
#include "waveformview.h"
#include "overviewbar.h"
#include "aplayer.h"

int main(int argc, char *argv[])
{
    Gtk::Main app(argc, argv);

    APlayer ap(argv[1]);

    const char *av2 = argv[2];
    std::string av2s(argv[1]);
    if (av2 == NULL) {
	av2s.append(".wf");
	av2 = av2s.c_str();
    }

    Waveform wf(av2);

    Glib::RefPtr<Gtk::Adjustment> aj(Gtk::Adjustment::create(0, 0, wf.m_n));
    Gtk::Scrollbar sb(aj);

    WaveformView wv(&wf, aj);
    OverviewBar ob(&wf, aj);

    ap.sig_bg_pos.connect(sigc::mem_fun(aj.operator->(), &Gtk::Adjustment::set_value));

    Gtk::VBox vb(false, 1);
    vb.pack_start(wv);
    vb.pack_start(ob);
    vb.pack_start(sb);

    Gtk::Button bp("Play", true);
    Gtk::Button bs("Stop", true);
    bp.signal_clicked().connect(sigc::mem_fun(ap, &APlayer::play_bga));
    bs.signal_clicked().connect(sigc::mem_fun(ap, &APlayer::stop_bg));

    Gtk::HBox hb(false, 1);
    hb.pack_start(bp);
    hb.pack_start(bs);
    vb.pack_start(hb);

    Gtk::Window win;
    win.add(vb);
    win.show_all();
    app.run(win);
    return 0;
}
