#ifndef CSECSPINBUTTON_H
#define CSECSPINBUTTON_H

#include <gtkmm/spinbutton.h>
#include "csec.h"

class CSecSpinButton : public Gtk::SpinButton
{
public:
    using Gtk::SpinButton::SpinButton;
protected:
    int on_input(double *v) override
    {
	try {
	    CSec cs(get_text().c_str());
	    *v = cs.csec();
	    return true;
	}
	catch (CSec::ParseError) {
	    *v = get_adjustment()->get_value();
	    return Gtk::INPUT_ERROR;
	}
    }
    bool on_output() override
    {
	unsigned csec = get_adjustment()->get_value();
	CSec sc(csec);
	set_text(sc.str());
	return true;
    }
};

#endif
