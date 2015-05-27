#include <iostream>
#include "rms.h"

int main(int argc, char *argv[])
{
    try {
	RMS generator(argv[1]);
	generator.loop();
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
