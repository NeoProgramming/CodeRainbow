#pragma once
#include <string.h>


// keyword
struct NKeyWord
{
	const char*		name;	// keyword or operator text
	size_t			size;	// token length in bytes
	NKeyWord()
	{
		name = 0;
		size = 0;
	}
	NKeyWord(const char *n)
	{
		name = n;
		size = strlen(n);
	}
};


