#include <cstring>
#include "waveform.h"
#include "waveformview.h"
#include "overviewbar.h"
#include "bplayer.h"
#include "csecspinbutton.h"

int main(int argc, char *argv[])
{
    Gtk::Main app(argc, argv);

    if (argv[1] == NULL)
	throw "no argv";

    const char *av2 = argv[2];
    std::string av2s(argv[1]);
    if (av2 == NULL) {
	av2s.append(".wf");
	av2 = av2s.c_str();
    }

    Waveform wf(av2);
    Glib::RefPtr<Gtk::Adjustment> aj(Gtk::Adjustment::create(0, 0, wf.m_n));
    BPlayer bg(argv[1], aj);

    Gtk::Scrollbar sb(aj);

    WaveformView wv(&wf, aj);
    OverviewBar ob(&wf, aj);

    Gtk::VBox vb(false, 1);
    vb.pack_start(wv);
    vb.pack_start(ob);
    vb.pack_start(sb);

    Gtk::Button bp("Play", true);
    Gtk::Button bs("Stop", true);
    bp.signal_clicked().connect(sigc::mem_fun(bg, &BPlayer::play_bg));
    bs.signal_clicked().connect(sigc::mem_fun(bg, &BPlayer::stop_bg));

    Glib::RefPtr<Gtk::Adjustment> aj1(Gtk::Adjustment::create(0, 0, wf.m_n));
    Glib::RefPtr<Gtk::Adjustment> aj2(Gtk::Adjustment::create(0, 0, wf.m_n));
    CSecSpinButton tc1(aj1);
    CSecSpinButton tc2(aj2);

    Gtk::HBox hb(false, 1);
    hb.pack_start(tc1);
    hb.pack_start(bp);
    hb.pack_start(bs);
    vb.pack_start(hb);
    hb.pack_start(tc2);

    Gtk::Window win;
    win.add(vb);
    win.show_all();
    app.run(win);
    return 0;
}
