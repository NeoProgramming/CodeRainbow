#pragma once

#include "base_defs.h"
#include "ct.h"
#include "utf.h"
#include "tdefs.h"

template<typename TB, typename TP>
const bool strMatch(const TB *stream, size_t size, const TP *pattern)
{
	// size uses for restrict stream size and also for pattern size
    while(*stream && *pattern && size) {
        if(*stream != *pattern)
            return false;
        stream++;
        pattern++;
        size--;
    }
	// end pattern and size: ok
    if(!*pattern && !size)
        return true;
    return false;
}

template<typename T1, typename T2>
const bool strEqu(const T1 *str1, const T2 *str2)
{
    while(*str1 && *str2) {
        if(*str1 != *str2)
            return false;
        str1++;
        str2++;
    }
    if(*str1 != *str2)
        return false;
	return true;
}

template<typename T>
struct TFBase
{
    typedef  T char_t;

	static int get(const char_t* s)
	{
		return *s;
	}
	
	static const char_t* next(const char_t* s)
	{
		return s + 1;
	}

    static const bool match(const char_t *stream, size_t size, const char *pattern)
	{
        return strMatch<char_t, char>(stream, size, pattern);
	}

	// these functions are not in ct, because the size of the char_t matters here (to index the next character)
	static bool	isBlockCmt(const char_t *s)
	{
		return s[0] == '/' && s[1] == '*';
	}
	static bool	isLineCmt(const char_t *s)
	{
		return s[0] == '/' && s[1] == '/';
	}

};




struct Ascii : TFBase< char > 
{
	// ascii to utf16string
	static String toStr(const char_t *str, int len)
	{
		return fromAscii(str, len);
	}
};

struct Utf8 : TFBase < utf8_t >
{
	// utf8 to utf16string
	static String toStr(const char_t *str, int len)
	{
		return fromUtf8(str, len);
	}
	// override
	static int get(const utf8_t* s)
	{
		int ch = 0;
		char *p = (char*)&ch;
		unsigned int bytes = utf_ExpectedCharLength(ch);
		if(bytes > 4)
			return 0;
		while(bytes) {
			*p = *s;
			s++;
			p++;
			bytes--;
		}
		return ch;
	}
	// override
	static const utf8_t* next(const utf8_t* s)
	{
		unsigned int len = utf_ExpectedCharLength(*s);
		if (len == 0xff)
			return s + 1;
		return s + len;
	}
};

struct Utf16 : TFBase < utf16_t >
{
	// utf16 to utf16string
	static String toStr(const char_t *str, int len)
	{
		return String(str, len);
	}
};

struct Utf32 : TFBase < utf32_t >
{
};

