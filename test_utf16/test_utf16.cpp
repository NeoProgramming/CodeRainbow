// test_utf16.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../src/core/crtree.h"
#include "../src/core/mark.h"
#include "../src/core/parse.h"

const wchar_t data[] = L"//@@\r\n double.cpp : Defines the entry point for the console application.\r\n\
//\r\n\
\r\n\
#include \"stdafx.h\"\r\n\
\r\n\
\r\n\
int _tmain(int argc, _TCHAR* argv[])\r\n\
{\r\n\
	double ddddx1 = 2.0, ddddx2 = 1.1;\r\n\
	\r\n\
	//TODO: lalala\r\n\
	//DEBUG: blablbla\r\n\
	\r\n\
	\r\n\
	\r\n\
	printf(\"%f\", ddddx1 - ddddx2);\r\n\
	getchar();\r\n\
	return 0;\r\n\
}\r\n\
\r\n\
};\r\n\
\r\n";

const int size = 289;

CRTree theCR;
CRTree *ptr = pCR = &theCR;

int _tmain(int argc, _TCHAR* argv[])
{
	printf("%d\n", sizeof(data));
	CRNode *fnode = nullptr;

	CR::MarkList bars, words;
    Parser<Utf16> parser(data, size, fnode);
    parser.parseMarkup(&bars, &words);


	getchar();
	return 0;
}

