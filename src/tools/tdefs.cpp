#include "tdefs.h"

std::wstring_convert<std::codecvt_utf8<wchar_t>> Converter;

String clrName(Clr clr)
{
	std::basic_stringstream<Char> ss;
	clr &= 0xffffff;
	ss << "#" << std::hex << clr;
	return ss.str();
}

Clr rgbToVal(const String &sclr)
{
    if(sclr.empty())
        return CLR_NONE;
	std::basic_stringstream<Char> ss;
	ss << sclr.substr(1);
	Clr a;
	ss >> std::hex >> a;
    return a;
}

String rgbToStr(Clr clr)
{
    if(clr == CLR_NONE)
        return String();
    return clrName( clr );
}

unsigned int hexToNum(const String &str)
{
	std::basic_stringstream<Char> ss;
	ss << str;
	unsigned int a;
	ss >> std::hex >> a;
    return a;
}

String numToHex(unsigned int val)
{
	std::basic_stringstream<Char> ss;
	ss << std::hex << val;
	return ss.str();
}

void str_split(const String &str, Char delim, StrList &strings)
{
	std::basic_istringstream<Char> f(str);
    String s;    
    while (std::getline(f, s, delim)) {
        if(!s.empty())
            strings.push_back(s);
    }
}

StrList str_split(const String &str, Char delim)
{
	StrList strings;
	str_split(str, delim, strings);
	return strings;
}

String pathToRel(const Path &base, const String &path)
{
    // decomposition of the path by the first question mark and converting the first part to a relative path
    if(path.empty())
        return path;
//    int q = path.indexOf('?');
//    if(q>=0)
//        return base.relativeFilePath(path.left(q)) + path.mid(q);
	return boost::filesystem::relative(path, base).generic_wstring(); // base.relative_path( relativeFilePath(path);
}

String pathToAbs(const Path &base, const String &path)
{
    // decomposition of the path by the first question mark and converting the first part to an absolute path
    if(path.empty())
        return path;
//    int q = path.indexOf('?');
//    if(q>=0)
//        return QDir::cleanPath( base.absoluteFilePath(path.left(q)) ) + path.mid(q);
	boost::system::error_code ec;
	return boost::filesystem::canonical(boost::filesystem::absolute(path, base), ec).make_preferred().wstring();
//	return boost::filesystem::canonical(boost::filesystem::absolute(path, base), ec).generic_wstring();
//    return boost::filesystem::absolute(path, base).generic_wstring(); // QDir::cleanPath( base.absoluteFilePath(path) );
}

String regex_escape(const String &str)
{
	// $  (  )  *  +  .  ?  [  ]  ^  {  |  }
	const Char specs[] = _T(".^$|()[]{}*+?\\");
	size_t len = str.length();
	String res;
	for(int i=0; i<len; i++) {
		const Char *p = specs;
		while(*p) {
			if(str[i] == *p) {
				res += '\\';
				break;
			}
			p++;
		}
		res += str[i];
    }
    return res;
}

String fromUtf8(const char *str, size_t len)
{
	String wstr;
	std::string src(str, len);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> good_converter;
	wstr = good_converter.from_bytes(src);
	return wstr;
}

String fromAscii(const char *str, size_t len)
{
	std::locale loc  = std::locale("");
	String wstr(len, 0);
    std::wstring::iterator j = wstr.begin();

    for(int i=0; i<len; ++i, ++j )
        *j = std::use_facet< std::ctype< wchar_t > > ( loc ).widen( str[i] );

    return wstr;
}

//String toStr(const char *str, int len)
//{@
//    return fromUtf8(str, len);
//}
//
//String toStr(const wchar_t *str, int len)
//{@
//	return String(str, len);
//}

