#include <cstring>
#include <iostream>
#include "waveform.h"
#include "waveformview.h"
#include "overviewbar.h"
#include "bplayer.h"
#include "csecspinbutton.h"

static void apicker(int argc, char *argv[])
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

    Gtk::Button bp("Play");
    Gtk::Button bs("Stop");
    Gtk::Button ok("OK");
    Gtk::Button bpl("Play left");
    Gtk::Button bpr("Play right");
    Gtk::Button bpt("Play through");
    bp.set_always_show_image(); bp.set_image_from_icon_name("media-playback-start");
    bs.set_always_show_image(); bs.set_image_from_icon_name("media-playback-stop");
    ok.set_always_show_image(); ok.set_image_from_icon_name("stock_redo");
    bpl.set_always_show_image(); bpl.set_image_from_icon_name("player_end");
    bpr.set_always_show_image(); bpr.set_image_from_icon_name("media-playback-start");
    bpt.set_always_show_image(); bpt.set_image_from_icon_name("media-seek-forward");

    bp.signal_clicked().connect([&]{ bg.play_bg(); });
    bs.signal_clicked().connect([&]{ bg.stop_bg(); });

    auto shortplay = [&](unsigned minus, unsigned plus)
    {
	unsigned pos = aj->get_value();
	unsigned begin = pos - minus;
	if (begin > pos)
	    begin = 0;
	unsigned end = pos + plus;
	if (end > wf.m_n)
	    end = wf.m_n;
	wv.short_passage(pos);
	ob.short_passage(pos);
	auto reset = [&aj, &wv, &ob, pos]
	{
	    aj->set_value(pos);
	    wv.short_passage();
	    ob.short_passage();
	};
	bg.play_bg(begin, end, reset);
    };

    bpl.signal_clicked().connect([&]{ shortplay(400, 000); });
    bpr.signal_clicked().connect([&]{ shortplay(000, 400); });
    bpt.signal_clicked().connect([&]{ shortplay(300, 300); });

    CSecSpinButton tc(aj);
    tc.set_width_chars(12);

    CSec cs1(0u);
    CSec cs2(wf.m_n);
    Gtk::Entry p1; p1.set_text(cs1.str()); p1.set_sensitive(false);
    Gtk::Entry p2; p2.set_text(cs2.str()); p2.set_sensitive(false);

    Gtk::Button s1("Set");
    Gtk::Button s2("Set");
    s1.set_always_show_image(); s1.set_image_from_icon_name("back");
    s2.set_always_show_image(); s2.set_image_from_icon_name("forward");
    s1.signal_clicked().connect([&]{ p1.set_text(tc.get_text()); });
    s2.signal_clicked().connect([&]{ p2.set_text(tc.get_text()); });

    auto on_ok = [&]
    {
	CSec cs1(p1.get_text().c_str());
	if (cs1.csec())
	    printf("-ss %s", cs1.str());
	CSec cs2(p2.get_text().c_str());
	if (cs2.csec() < wf.m_n) {
	    CSec cs3(cs2.csec() - cs1.csec());
	    printf("%s-t %s", cs1.csec() ? " " : "", cs3.str());
	}
	printf("\n");
	app.quit();
    };

    ok.signal_clicked().connect(on_ok);

    Gtk::VSeparator v1;
    Gtk::VSeparator v2;
    Gtk::VSeparator v3;

    Gtk::HBox hb(false, 1);
    hb.pack_start(p1);
    hb.pack_start(s1);
    hb.pack_start(v1);
    hb.pack_start(bpt);
    hb.pack_start(bpl);
    hb.pack_start(bpr);
    hb.pack_start(tc);
    hb.pack_start(bp);
    hb.pack_start(bs);
    hb.pack_start(v2);
    hb.pack_start(s2);
    hb.pack_start(p2);
    hb.pack_start(v3);
    hb.pack_start(ok);

    vb.pack_start(hb);

    Gtk::Window win;
    win.add(vb);
    win.show_all();
    app.run(win);
}

int main(int argc, char *argv[])
{
    try {
	apicker(argc, argv);
	return 0;
    }
    catch (const std::exception &e) {
	std::cerr << e.what() << std::endl;
	return 1;
    }
    catch (const char *s) {
	std::cerr << s << std::endl;
	return 1;
    }
}
