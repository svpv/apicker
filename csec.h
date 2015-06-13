#ifndef CSEC_H
#define CSEC_H

class CSec
{
public:
    CSec(unsigned csec)
    {
	char *p = buf;
	unsigned h = csec / 100 / 3600;
	csec -= h * 100 * 3600;
	if (h > 9)
	    h = 9;
	*p++ = '0' + h;
	*p++ = ':';
	unsigned min = csec / 100 / 60;
	csec -= min * 100 * 60;
	*p++ = '0' + min / 10;
	*p++ = '0' + min % 10;
	*p++ = ':';
	unsigned sec = csec / 100;
	csec -= sec * 100;
	*p++ = '0' + sec / 10;
	*p++ = '0' + sec % 10;
	*p++ = '.';
	*p++ = '0' + csec / 10;
	*p++ = '0' + csec % 10;
	*p++ = '\0';
    }

    const char *str()
    {
	buf[7] = '.';
	return buf;
    }

    const char *strsec()
    {
	buf[7] = '\0';
	return buf;
    }

protected:
    char buf[sizeof("0:00:00.00")];
};

#endif
