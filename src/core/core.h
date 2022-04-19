#pragma once
#include <tchar.h>
#include "../tools/tdefs.h"

#define APP_EXT ".cr"

const char AppName[] = "CodeRainbow";
const char AppFileExt[] = APP_EXT;
const Char AppFileExtU[] = _T(APP_EXT);
const char AppFileFilter[] = "CodeRainbow Files (*.cr)";

const char SrcFilters[] = "*.cpp;*.c;*.h";

const Char CR_CMT[]  = _T("//");
const Char CR_LABEL[] = _T("$$");
const Char CR_AREA[] = _T("<<");
const Char CR_AEND[] = _T(">>");
const Char CR_BLOCK[] = _T("@@");
