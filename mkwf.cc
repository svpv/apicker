#include <iostream>
#include "makewaveform.h"

int main(int argc, char *argv[])
{
    try {
	MakeWaveform maker(argv[1]);
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
