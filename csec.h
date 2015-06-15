/*
 * Centisecond timecodes, H:MM:SS[.FF]
 */

#ifndef CSEC_H
#define CSEC_H

#include <exception>

class CSec
{
public:
    CSec(unsigned csec);
    CSec(const char *str);

    class ParseError : public std::exception
    {
    public:
	const char *what() const throw() override
	{
	    return "timecode parse error";
	}
    };

    unsigned csec()
    {
	return m_csec;
    }

    const char *str()
    {
	m_buf[7] = '.';
	return m_buf;
    }

    const char *strsec()
    {
	m_buf[7] = '\0';
	return m_buf;
    }

protected:
    unsigned m_csec;
    char m_buf[sizeof("0:00:00.00")];
};

#endif
