#pragma once

#include <string>
#include <list>

// *.htm, *.shtml.htm

template<typename TF>
bool dosMatch(typename TF::char_t const *str, typename TF::char_t const *regexp, 
	std::list<std::basic_string<typename TF::char_t> > *res = nullptr)
{
	bool substr = false;
	int sch=0, rch=0;
	// loop: read the regular expression character and depending on
	// from its value we perform actions
	while(rch = TF::get(regexp)) {
		regexp = TF::next(regexp);
		switch(rch) {
		case _T('*'):	// any group of fname characters
			// skip any number of fname characters
			while(sch = TF::get(str)) {
				if(!ct::isFName(sch))
					break;
				if(substr && res) 
					res->back() += sch;
				str = TF::next(str);
			}
			break;
		case _T('?'):	// any fname character
			// skip one fname character
			sch = TF::get(str);
			if(!sch) 
				return 0;
			if(!ct::isFName(sch)) 
				return 0;
			if(substr && res) 
				res->back() += sch;
			str = TF::next(str);
			break;
		case _T('<'):	// start of substring formation
			if(res)
				res->push_back(_T(""));
			substr = true;
			break;
		case _T('>'):	// end of substring formation
			substr = false;
			break;
		default:
			// exact character equality
			sch = TF::get(str);
			if(!sch) 
				return 0;
			if(sch != rch) 
				return 0;
			if(substr && res) 
				res->back() += sch;
			str = TF::next(str);
			break;
		}
	}
	sch = TF::get(str);
	rch = TF::get(regexp);
	if(sch == rch) 
		return true;
	return false;
}


