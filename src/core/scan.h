#pragma once

#include <fstream>
#include "../tools/tfbase.h"
#include "../ted/text_encoding_detect.h"
#include "crnode.h"
#include "parse.h"

enum EScanMode {
	ESM_COVERAGE,
	ESM_TOKENS,
	ESM_FUIDS,
	ESM_TAGS,

	ESF_ModeMask  = 0xF,       // 4 bits per mode
	ESF_Single    = 0x10000,   // look for a single occurrence and immediately open the file
    ESF_ThisFile  = 0x20000,   // search only in fnode file
};

// callback for processing the source opened in the editor;
// returns 0 if
typedef int (*FnGetSrc)(void *arg, FItem *fitem, int mode, const String& str, const Char *fpath);
typedef void  (*FnAddMsg)(void *arg, const Char *fpath, int line, const Char *msg);

// scan one open source
template<typename TF>
void scanBuf(FItem *fitem, int mode, const String &str, typename TF::char_t const *text, int size, FnAddMsg fn_addmsg, void *arg)
{
    typedef Parser<TF> ParserTF;
    typedef TF::char_t char_t;

    FoundList lines;
    ParserTF parser(text, size, fitem);
    if(mode==ESM_COVERAGE)
        parser.parseCoverage(&lines);
    else if(mode==ESM_TOKENS)
        parser.parseFindToken(str, &lines);
    else if(mode==ESM_FUIDS)
        parser.parseFindFuid(str, &lines);
    else if(mode==ESM_TAGS)
        parser.parseFindTag(str, &lines);

    // add results via second callback
    for(FoundList::iterator i=lines.begin(), e=lines.end(); i!=e; ++i) {
        fn_addmsg(arg, fitem->fpath.c_str(), i->line, i->str.c_str());
    }
}

// scanning the whole tree or part of it
template<typename TF>
void scanCR(CRNode *root, const String &str, int mode, FnGetSrc fn_getsrc, FnAddMsg fn_addmsg, void *arg)
{
	// ARGS: root - node of the tree, starting from which we are looking for
	//       mode - bitmask + mode enumeration
	//  fn_getsrc - function to get open source from the editor
	//  fn_addmsg - function to add message to results
	//        arg - argument for functions

	typedef Parser<TF> ParserTF;
	typedef TF::char_t char_t;

	bool only_root = !!(mode & ESF_ThisFile);
	mode &= ESF_ModeMask; 

	// the trick is that in the tree, files can be children of non-files, so a full bypass
    root->walkNodes([&](CRNode *node)->CRNode* {
		// files only (not directories!) -- files can also be any element in OUTLINE
		// first condition: look for only one file
		if( (!only_root || root==node) && (node->type!=CR::NT_DIR) && node->fitem && !node->fitem->fpath.empty() ) { 
            // if the external handler did not scan the text - scan from the file
            if( !fn_getsrc(arg, node->fitem, mode, str, node->fitem->fpath.c_str()) ) {
				
				// open and download file
				std::ifstream ifs(node->fitem->fpath, std::ios::binary|std::ios::ate);
				std::ifstream::pos_type pos = ifs.tellg();
                int size = (int)pos;
				if(size >= 0) {
					unsigned char* text = new unsigned char[size + 2*sizeof(int)];
					ifs.seekg(0, std::ios::beg);
					ifs.read((char*)text, pos);
					ifs.close();
					*((int*)(text+size)) = 0;

					// determine the encoding
					using namespace AutoIt::Common;
					TextEncodingDetect textDetect;
					TextEncodingDetect::Encoding encoding = textDetect.DetectEncoding(text, size);

					// text in the buffer is ready; start scanning
					// just need to see what encoding the String has (BE or LE) and rotate the bytes as needed
					if(	encoding == TextEncodingDetect::ASCII || encoding == TextEncodingDetect::ANSI || encoding == TextEncodingDetect::None)
						scanBuf<Ascii>(node->fitem, mode, str, (Ascii::char_t const*)text, size/sizeof(Ascii::char_t), fn_addmsg, arg);
					else if( encoding == TextEncodingDetect::UTF8_BOM || encoding == TextEncodingDetect::UTF8_NOBOM)
						scanBuf<Utf8>(node->fitem, mode, str, (Utf8::char_t const*)text, size/sizeof(Utf8::char_t), fn_addmsg, arg);
					else if( TextEncodingDetect::UTF16_LE_BOM || encoding == TextEncodingDetect::UTF16_LE_NOBOM)
						scanBuf<Utf16>(node->fitem, mode, str, (Utf16::char_t const*)text, size/sizeof(Utf16::char_t), fn_addmsg, arg);
					else if (encoding == TextEncodingDetect::UTF16_BE_BOM || encoding == TextEncodingDetect::UTF16_BE_NOBOM)
						scanBuf<Utf16>(node->fitem, mode, str, (Utf16::char_t const*)text, size/sizeof(Utf16::char_t), fn_addmsg, arg);

					// free memory
					delete[] text;
				}
            }
        }
        return nullptr;
    });
}



