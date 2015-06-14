#include <iostream>
#include <string>
#include "waveformgen.h"

int main(int argc, char *argv[])
{
    try {
	WaveformGen gen(argv[1]);
	gen.loop();
	if (argv[2])
	    gen.save(argv[2]);
	else {
	    std::string out(argv[1]);
	    out.append(".wf");
	    gen.save(out.c_str());
	}
    }
    catch (const std::exception &e) {
	std::cerr << e.what() << std::endl;
	return 1;
    }
    catch (const char *s) {
	std::cerr << s << std::endl;
	return 1;
    }
    return 0;
}
