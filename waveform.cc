#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "waveform.h"

Waveform::Waveform(const char *fname)
{
    int fd = open(fname, O_RDONLY);
    if (fd < 0)
	throw "cannot open waveform file";

    struct stat st;
    if (fstat(fd, &st) < 0)
	throw "cannot stat waveform file";

    if (st.st_size <= 8)
	throw "waveform file is too small";

    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
	close(fd);
	throw "cannot map waveform file";
    }
    close(fd);

    m_n = st.st_size - 8;
    m_v = (unsigned char *) map + 8;

    const char magic[] = "WF1";
    if (memcmp(map, magic, sizeof(magic)) != 0) {
	munmap(m_v - 8, m_n + 4);
	throw "bad magic in waveform file";
    }
}

Waveform::~Waveform()
{
    if (munmap(m_v - 8, m_n + 4) < 0)
	throw "cannot unmap waveform file";
}
