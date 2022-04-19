#include "mcmt.h"
#include "nlexer.h"
#include "core.h"
#include <tchar.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>

void createMarkers(CRMark &mi, String &beg, String &end)
{
    // for a given node, generate start and end metacomments
    beg = makeLine(mi, _T(""), _T(""));
    if(mi.type == CR::NT_AREA)
        end = pCR->pp.cmt+pCR->pp.aend;
    else
        end = _T("");
}

int strExtract(const String &buf, int start, const String &str)
{
    size_t n = str.length();
    if(buf.length() - start < n)
        return -1;
    for(size_t i=0; i<n; i++)
        if(buf[i+start] != str[i])
            return -1;
    return start + n;
}

// forming a significant part of the metacomment from the structure
String makeMarkStr(CRMark &mi)
{
    String str;
    if(!mi.id.empty()) {
        str = mi.id;
        str += _T(":");
    }
    str += mi.tagStr();
    if(mi.mclr != CLR_NONE) {
        str += clrName(mi.mclr);
    }
    return str;
}

// parsing a meaningful part of a metacomment into a structure
void parseMarkStr(const String &str, int i, CRMark &mi, String *tail)
{
    // name:tag1,tag2,tag3#clr
    // tag#clr
    // name:tag1,tag2,tag3
    // tag
    // tag1,tag2,tag3
    // tag1,tag2,tag3#clr
    enum ReadState {
        RS_NAME,
        RS_TAGS,
        RS_CLR,
        RS_TAIL
    } rs = RS_NAME;

    int n = str.size();
    int ibeg = i;
    while(i<=n) {
        Char c = (i==n) ? 0 : str[i];
        switch(rs) {
        case RS_NAME:
            if(c == ':') {
                mi.id = str.substr(ibeg, i-ibeg);
                ibeg = i+1;
            }
            else if(c == ',' || c == '#' || c == ' ' || c == '\t' || c == 0) {
                mi.tags.push_back(str.substr(ibeg, i-ibeg));
                ibeg = i+1;
            }
            if(c == ':' || c == ',')
                rs = RS_TAGS;
            else if(c == '#')
                rs = RS_CLR;
            else if(c == ' ' || c == '\t')
                rs = RS_TAIL;
            break;
        case RS_TAGS:
            if(c == ',' || c=='#' || c == ' ' || c == '\t' || c == 0) {
                mi.tags.push_back(str.substr(ibeg, i-ibeg));
                ibeg = i+1;
            }
            if(c == '#')
                rs = RS_CLR;
            else if(c == ' ' || c == '\t')
                rs = RS_TAIL;
            break;
        case RS_CLR:
            if(c == ' ' || c == '\t' || c == 0) {
                mi.mclr = rgbToVal(str.substr(ibeg, i-ibeg));
                ibeg = i+1;
                rs = RS_TAIL;
            }
            break;
        case RS_TAIL:
            if(c == 0)
                if(tail)
                    *tail = str.substr(ibeg-1);
            break;
        }
        i++;
    }
}

// constructing a metacomment
String makeLine(CRMark &mi, const String &head, const String &tail)
{
    String beg, clr, ids = makeMarkStr(mi);

    // < <cmt#clr
    if(mi.type == CR::NT_AREA)
        beg = head + pCR->pp.cmt+pCR->pp.area+ids+clr + tail;
    // $ $cmt#clr
    else if(mi.type == CR::NT_LABEL)
        beg = head + pCR->pp.cmt+pCR->pp.label+ids+clr + tail;
    // @ @cmt#clr
    if(mi.type == CR::NT_BLOCK)
        beg = head + pCR->pp.cmt+pCR->pp.block+ids+clr + tail;
    return beg;
}

// metacomment parsing
CR::Type parseLine(const String &str, CRMark &mi, String *head, String *tail)
{
    mi.mclr = CLR_NONE;
    mi.id = _T("");
    mi.type = CR::NT_NONE;

    int itag, start;

    // single line comment
    start = str.find(pCR->pp.cmt);
    if(start < 0)
        return CR::NT_NONE;
    if(head)
        *head = str.substr(0,start);
    start += pCR->pp.cmt.length();

    // > >  AREA_END
    itag = strExtract(str, start, pCR->pp.aend);
    if(itag >=0 ) {
        mi.type = CR::NT_AEND;
        return CR::NT_AEND;
    }

    // < <  AREA_START
    itag = strExtract(str, start, pCR->pp.area);
    if(itag >=0 ) {
        parseMarkStr(str, itag, mi, tail);
        mi.type = CR::NT_AREA;
        return CR::NT_AREA;
    }

    // $ $  LABEL
    itag = strExtract(str, start, pCR->pp.label);
    if(itag >= 0) {
        parseMarkStr(str, itag, mi, tail);
        mi.type = CR::NT_LABEL;
        return CR::NT_LABEL;
    }

    // @ @  BLOCK
    itag = strExtract(str, start, pCR->pp.block);
    if(itag >= 0) {
        parseMarkStr(str, itag, mi, tail);
        mi.type = CR::NT_BLOCK;
        return CR::NT_BLOCK;
    }

    return CR::NT_NONE;
}

/*
// restoring a metacomment from information; used for editing
void makeMCmt(QString &str, CRMark &mark)
{
    str = "//";
    if(mark.type == CR::NT_AEND)
        str += pCR->pp.aend;
    else if(mark.type == CR::NT_AREA)
        str += pCR->pp.area;
    else if(mark.type == CR::NT_LABEL)
        str += pCR->pp.label;
    else if(mark.type == CR::NT_BLOCK)
        str += pCR->pp.block;

    str += makeMarkStr(mark);

    if(mark.clr != CLR_NONE)
        str += QColor(mark.clr).name();
}*/

String makeSigRegExp(const String &sig)
{
	// regexps in each system (Qt/C#/Java) are different, so we return a string
    // replace all spaces to regex spaces, other symbols to scaped regex symbols
    // replace comments to regex ignores
//  qDebug() << "makeSigRegExp: " << sig;

    String regexp;
    NLexer<Utf16> lexer(sig.c_str(), sig.size());
    NLexer<Utf16>::token_t tok;
    while(lexer.Scan(&tok)) {
//      qDebug() << "token: " << QString::fromUtf16(tok.ptr, tok.length);
        regexp += regex_escape(String(tok.ptr, tok.length));
        regexp += _T("[\\s\\t\\n\\r]*");
    }
//  qDebug() << "makeSigRegExp == " << regexp;
    return regexp;
}

String makeMarkRegExp(const String &id, CR::Type type)
{
    String regexp;
    regexp = regex_escape(CR_CMT);
    regexp += _T("\\s*");
    if(type == CR::NT_AREA)
        regexp += regex_escape(CR_AREA);
    else if(type == CR::NT_BLOCK)
        regexp += regex_escape(CR_BLOCK);
    else if(type == CR::NT_LABEL)
        regexp += regex_escape(CR_LABEL);
    regexp += _T("\\s*");
    regexp += regex_escape(id);
    return regexp;
}

String removeComments(const String &str)
{
    String sig;
    enum State {
        S_CODE,
        S_LCMT,
        S_MCMT
    } s = S_CODE;
    int len = str.length();
    int i = 0, lci, mci;
    while(i<len) {
        switch(s){
        case S_CODE:
            lci = str.find(_T("//"), i);
            if(lci<0) lci=len;
            mci = str.find(_T("/*"), i);
            if(mci<0) mci=len;
            if(lci < mci) {
                sig += str.substr(i, lci-i);
                s = S_LCMT;
                i = lci+1;
            }
            else {
                sig += str.substr(i, mci-i);
                s = S_MCMT;
                i = mci+1;
            }
            break;
        case S_LCMT:
            i = str.find(0x2029, i);	// QChar::ParagraphSeparator
            if(i>=0) {
                sig += _T(" ");
                i++;
                s = S_CODE;
            }
            break;
        case S_MCMT:
            i = str.find(_T("*/"), i);
            if(i>=0) {
                sig += _T(" ");
                i+=2;
                s = S_CODE;
            }
            break;
        }
    }
    return sig;
}

bool extractId(String &str, int index)
{
    // trying to extract an id from a string
    int len = str.length();
    if(index < 0 || index > len)
        return false;
    int start = index, end = index;
    const Char *ptr = str.c_str();
    // move left with i-1, non-leading
    for(int i=index-1; i>=0; i--) {
        if(!ct::isIdnext(*(ptr+i)))
            break;
        start = i;
    }
    // movement to the right with i, leading
    for(end=index; end<len; end++) {
        if(!ct::isIdnext(*(ptr+end)))
            break;
    }
    if(start == end)
        return false;
    str = str.substr(start, end-start);
    return true;
}

void replaceNonId(String &str)
{
    // replace non-identifiers with underscores
//    qDebug() << "replaceNonId: " << str;
    boost::trim(str);
	int len = str.length();
	for(int i=0; i<len; i++) {
		if(!ct::isIdnext(str[i]))
			str[i] = '_';
	}
    //str.replace(QRegExp("[^a-zA-Z\\d]+"), "_");
}

void ensureUniqueId(CRNode *fnode, String &id)
{
    // retrieve the ID and make sure it's unique
    // extract the digital part and provide the first number which is not in the file
    if(id.empty())
        return;
    // get a pointer to the digital part
    const Char *beg = id.c_str();
    const Char *dig = beg + id.size() - 1;
    unsigned int num = 0, k = 1;
    while(dig >= beg) {
        if(*dig >= '0' && *dig <= '9') {
            num += k*(*dig - '0');
            k *= 10;
            dig--;
        }
        else
            break;
    }
    // went out; now we select a free name starting from num + 1
    String tid;
    if(dig >= beg)
        tid = id.substr(0, dig-beg+1);
    String nid = tid;

    while(1) {
        if(!fnode->findChildId(nid)) {
            id = nid;
            break;
        }
        num++;
        nid = tid + std::to_wstring(static_cast<long long>(num));
    }
}
