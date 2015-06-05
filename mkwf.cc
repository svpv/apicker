#include <iostream>
#include "waveformgen.h"

int main(int argc, char *argv[])
{
    try {
	WaveformGen gen(argv[1]);
	gen.loop();
	gen.save(argv[2]);
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
