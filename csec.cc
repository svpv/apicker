#include "csec.h"

CSec::CSec(unsigned csec)
    : m_csec(csec)
{
    char *p = m_buf;
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

CSec::CSec(const char *str)
    : m_csec(0)
{
    const char *s = str;
    char *p = m_buf;

    /* H */
    if (*s < '0' || *s > '9')
	goto error;
    m_csec += (*s - '0') * 100 * 3600;
    *p++ = *s++;

    if (*s != ':')
	goto error;
    *p++ = *s++;

    /* MM */
    if (*s < '0' || *s > '5')
	goto error;
    m_csec += (*s - '0') * 100 * 600;
    *p++ = *s++;

    if (*s < '0' || *s > '9')
	goto error;
    m_csec += (*s - '0') * 100 * 60;
    *p++ = *s++;

    if (*s != ':')
	goto error;
    *p++ = *s++;

    /* SS */
    if (*s < '0' || *s > '5')
	goto error;
    m_csec += (*s - '0') * 100 * 10;
    *p++ = *s++;

    if (*s < '0' || *s > '9')
	goto error;
    m_csec += (*s - '0') * 100;
    *p++ = *s++;

    if (*s == '\0') {
	*p++ = '.';
	*p++ = '0';
	*p++ = '0';
	*p++ = '\0';
	return;
    }

    if (*s != '.')
	goto error;
    *p++ = *s++;

    /* FF */
    if (*s < '0' || *s > '9')
	goto error;
    m_csec += (*s - '0') * 10;
    *p++ = *s++;

    if (*s == '\0') {
	*p++ = '0';
	*p++ = '\0';
	return;
    }

    if (*s < '0' || *s > '9')
	goto error;
    m_csec += (*s - '0') * 1;
    *p++ = *s++;

    /* Permit FFF */
    if (*s >= '0' && *s <= '9')
	s++;

    if (*s == '\0') {
	*p++ = '\0';
	return;
    }

error:
    throw ParseError();
}
