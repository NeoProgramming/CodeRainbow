#pragma once
#include <string>
#include <list>
#include <codecvt>
#include <locale>
#include <sstream>
#include <tchar.h>
#include <boost/filesystem.hpp>


typedef unsigned int Clr;
#ifndef CLR_NONE
#define CLR_NONE ((Clr)0xffffffff)
#endif
#define clrValid(clr) ((clr) != CLR_NONE)

typedef wchar_t Char;
typedef const Char* CStr;

typedef std::wstring String;
typedef std::list<std::wstring> StrList;

typedef boost::filesystem::path Path;

extern std::wstring_convert<std::codecvt_utf8<wchar_t>> Converter;

String clrName(Clr clr);

void    str_split(const String &str, Char delim, StrList &strings);
StrList str_split(const String &str, Char delim);

Clr    rgbToVal(const String &sclr);
String rgbToStr(Clr clr);

unsigned int hexToNum(const String &str);
String numToHex(unsigned int val);

String pathToRel(const Path &base, const String &path);
String pathToAbs(const Path &base, const String &path);

String regex_escape(const String &str);

String fromUtf8(const char *str, size_t len);
String fromAscii(const char *str, size_t len);

//String toStr(const char *str, int len);
//String toStr(const wchar_t *str, int len);
