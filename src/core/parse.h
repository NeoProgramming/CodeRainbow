#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <vector>
#include <list>
#include <stack>

#include "../tools/tdefs.h"
#include "../tools/tfbase.h"
#include "nlexer.h"
#include "mark.h"
#include "crnode.h"
#include "mcmt.h"
#include "kw.h"
#include "sig.h"

struct Layer {
    Clr rgb;
    CRNode *node;

    Layer(Clr c, CRNode*n)
        : rgb(c), node(n){}
};
// found occurrence
struct Found {
    int line;
    String str;
};
// list of found occurrences
typedef std::list<Found> FoundList;

// brace
struct Brace {
    int level;  // parenthesis nesting level
    int heads;  // number of users of the given bracket

    Brace() { level = -1; heads = 0; }
    Brace(int l, int h) { level=l; heads=h; }
};





// parser
template<typename TF>
class Parser
{
	enum { QUERY_TOP_POS = -2 };
    typedef typename TF::char_t char_t;
public:
    Parser(const char_t *_data, int _size, FItem *_fi)
        : data(_data)
        , size(_size)
        , fitem(_fi)
        , pBars(nullptr)
        , pWords(nullptr)
        , pCoverage(nullptr)
        , pTags(nullptr)
        , pFuids(nullptr)
        , pTokens(nullptr)
		, braceHeads(0)
		, braceLevel(0)
        , wait_uncovered(true)
		, wideLines(true)
    {}
    void parseMarkup(CR::MarkList *_pbl, CR::MarkList *_pwl);
    void parseCoverage(FoundList *_pll);
    CRNode* parseFindNode(int line, int col, CR::Mark *pm = nullptr);
    CRNode* parseFindNode(int pos, CR::Mark *pm = nullptr);
    void parseFindToken(const String &name, FoundList *_pfl);
    void parseFindTag(const String &tag, FoundList *_pfl);
    void parseFindFuid(const String &fuid, FoundList *_pfl);

    static typename NLexer<TF>::token_t nextLine(typename NLexer<TF>::token_t tok);
    static typename NLexer<TF>::token_t prevLine(typename NLexer<TF>::token_t tok);

	bool wideLines;
protected:
    void parse();
    void parseCmt(const typename NLexer<TF>::token_t &tok);
    void parseSig(const typename NLexer<TF>::token_t &tok, SigList &sl);

    void makeSigList(CRNode *locroot, SigList &sl);
    void makeSigList(CRNode *locroot, FItem *fitem, SigList &sl);
    void prepareSigList(SigList &sl);
    void prepareSig(const String &code, Sig &sig);
    bool compareSig(const typename NLexer<TF>::token_t &tok, Sig &sig);

    void handleMark(CR::Type mt, const Loc &loc, int toklen, Clr clr, CRNode *node);
    void handleBeginBrace();
    void handleEndBrace(const Loc &loc);

    void addFullMark(const Loc &start, int qlines, int qbytes, const Layer &lr);
    void addWordMark(const Loc &start, int len, const Layer &lr);
    void addWantedStr(FoundList *pFoundList, const typename NLexer<TF>::token_t &tok, const String &str);
private:
    const char_t *data;
    int size;
    FItem *fitem;				// file element to be parsed
    CR::MarkList *pBars;        // mark list
    CR::MarkList *pWords;       // words list (does not participate in coverage)
    FoundList *pCoverage;       // coverage list
    FoundList *pTags;			// tags list
    FoundList *pTokens;			// names (tokens) list
    FoundList *pFuids;			// fuid's list
    String wanted_str;          // tag/id to search
    Sig wanted_sig;             // signature to search
    std::stack<Layer> stColors; // color stack, last change point color
	Loc locNewline;				// start of current line
    Loc locTop;                 // start of current region
    int braceHeads;				// open brace wait flag for block
    std::stack<Brace> stBraces; // stack of bracket indices and separately the top of the stack
    int braceLevel;				// current global bracket index
    Loc locUncovered;           // location of the last uncovered area
    Loc locSig;                 // location of the last encountered signature
    bool sigMatched;            // last encountered signature
    bool wait_uncovered;		//
};

/////// implementation ///////

template<typename char_t>
int lenUntilNewline(const char_t *p)
{
    const char_t *s = p;
    while(*p && *p!='\r' && *p!='\n')
        p++;
    return p-s;
}

template<typename TF>
void Parser<TF>::handleMark(CR::Type mt, const Loc &loc, int remlen, Clr clr, CRNode *node)
{
    // handle marker of any type
    // called if:
    // 1. signature matched (in this case, the last token != the end of the string and the length of the last string is unknown!)
    // 2. a single-line token comment was caught (you can determine the position of the next token)
    // length from the beginning of the last token to the end of the string is passed to remlen
    // it matches the length of the token if it's a line comment

    // if label is one line
    if(mt == CR::NT_LABEL || mt == CR::NT_LSIG) {
        // if the visibility of this block is not disabled
        if(clrValid(clr) && (!node || (node->flags & CRNode::NF_VISIBLE))) {
            // if there is a top block
            if(locTop.row!=loc.row && !stColors.empty() )
                addFullMark(locTop, loc.row-locTop.row, loc.pos-locTop.pos+remlen, stColors.top() );

            // add the label itself
            addFullMark(loc, 1, remlen, Layer(clr, node));

            // put a request for the formation of a new upper line (next line)
            locTop.pos = QUERY_TOP_POS;
        }
    }

    // if area begin - start a new color, put the old one on the stack
    else if(mt == CR::NT_AREA) {
        // if the visibility of this block is not disabled
        if(clrValid(clr) && (!node || (node->flags & CRNode::NF_VISIBLE))) {
            // if there is a top block, end it
            if(locTop.row!=loc.row && !stColors.empty() )
                addFullMark(locTop, loc.row-locTop.row, loc.pos-locTop.pos+remlen, stColors.top() );

            // new top line - this line
            locTop = loc;
        }
        // save the old color on the stack, if there was one, and remember that this is the beginning of the block
        stColors.push(Layer(clr, node));
    }

    // if area end - close the current color, take the old one from the stack
    else if(mt == CR::NT_AEND) {
        // visibility of the block paired with this can be turned off;
        if(!stColors.empty()) {
            Layer &lr = stColors.top();
            if(clrValid(lr.rgb) && (!lr.node || (lr.node->flags & CRNode::NF_VISIBLE))) {
                // if there is a top block
                if( locTop.row != loc.row )
                    // label the top block
                    addFullMark(locTop, loc.row-locTop.row+1, loc.pos-locTop.pos+remlen, lr );

                // put a request for the formation of a new upper line (next line)
                locTop.pos = QUERY_TOP_POS;
            }
            // pop from stack
            stColors.pop();
        }
    }

    // if block - start a new color, put the old one on the stack
    else if(mt == CR::NT_BLOCK || mt == CR::NT_BSIG) {
        // if the visibility of this block is not disabled
        if(clrValid(clr) && (!node || (node->flags & CRNode::NF_VISIBLE))) {
            // increase the flag "waiting for the first opening bracket" (number of heads)
            braceHeads++;

            // if there is a top block, end it
            if(locTop.row!=loc.row && !stColors.empty() )
                // label the top block
                addFullMark(locTop, loc.row-locTop.row, loc.pos-locTop.pos+remlen, stColors.top() );

            // save the old color on the stack, if there was one, and remember that this is the beginning of the block
            stColors.push(Layer(clr, node));

            // new top line is this line
            locTop = loc;
        }
    }
}

template<typename TF>
void Parser<TF>::parseCmt(const typename NLexer<TF>::token_t &tok)
{
    String str = TF::toStr(tok.ptr, tok.length);

    // parse meta comment
    CRMark mi;
    CR::Type mt = parseLine(str, mi);
    if(mt == CR::NT_NONE)	// this is not a marker comment
        return;

    // if it's a search for unique ids
    if(pFuids && mi.id == wanted_str)
        addWantedStr(pFuids, tok, str);

    // if it's a tag search
    if(pTags && std::find(mi.tags.begin(), mi.tags.end(), wanted_str) != mi.tags.end())
        addWantedStr(pTags, tok, str);

	// define the node corresponding to the given marker comment
    CRNode *node = nullptr;
    // if the color is not set explicitly - we take it; if not, we take from the node
    if(!clrValid(mi.mclr)) {
		// if there is a file node
		if(fitem) {
			// looking in file node
			if(fitem->fnode)
				node = fitem->fnode->findChildId(mi.id);
			// failed - looking in the outline
			if(!node || node->nclr==CLR_NONE)
				node = pCR->findOutlineElem(fitem, mi.id);
		}
		// failed - search in tags
        if(!node || node->nclr==CLR_NONE)
            node = pCR->findTag(mi.tags);
		// as a result, we determine the color
        mi.mclr = node ? node->nclr : CLR_NONE;
    }

    handleMark(mt, tok.loc, tok.length, mi.mclr, node);
}

template<typename TF>
void Parser<TF>::handleBeginBrace()
{
    if(braceHeads) {
        // push the parenthesis level only if we need the closing parenthesis of the same level
        Brace b(braceLevel, braceHeads);
        stBraces.push(b);
        braceHeads = 0;
    }
    // increase the current bracket level
    braceLevel++;
}

template<typename TF>
void Parser<TF>::handleEndBrace(const Loc &loc)
{
    if(braceLevel > 0) {
        braceLevel--;
    }

    Brace top;
    if(!stBraces.empty())
        top = stBraces.top();

    if(braceLevel == top.level) {
        // adjusting the bracket stack
        if(!stBraces.empty())
            stBraces.pop();

        // standard block completion
        for(int i=0; i<top.heads; i++) {

            // if there is an upper block
            if(locTop.row!=loc.row && !stColors.empty() ) {
                // mark the top block
                addFullMark(locTop, loc.row-locTop.row+1, loc.pos-locTop.pos+1, stColors.top() );
            }

            // pop from stack
            if(!stColors.empty())
                stColors.pop();
        }

        // put a request for the formation of a new upper line (next line)
        locTop.pos = QUERY_TOP_POS;
    }
}

template<typename TF>
void Parser<TF>::prepareSig(const String &code, Sig &sig)
{
    extern NKeyWord KW[];
    sig.index = 0;

    NLexer<Utf16> lexer(code.c_str(), code.size());
    NLexer<Utf16>::token_t tok;
    lexer.skipLineCmts = false;
    lexer.initOperators(&KW[0], &KW[2], TOKlbrace);

    while(lexer.Scan(&tok)) {
        if(tok.value != TOKlinecmt) {
            sig.lexemes.push_back( tok );
        //    qDebug() << "prepare: " << QString::fromUtf16(tok.ptr, tok.length);
        }
    }
    // signatures of zero length should not be!
}

template<typename TF>
void Parser<TF>::makeSigList(CRNode *locroot, FItem *fitem, SigList &sl)
{
	if(!locroot)
		return;
    // search for all signatures in the given node and its child nodes
    locroot->walkNodes([&sl,&fitem](CRNode *node)->CRNode* {
        if(CR::isSig(node->type) && node->fitem == fitem)
            sl.push_back(Sig(node));
        return nullptr;
    });
}

template<typename TF>
void Parser<TF>::makeSigList(CRNode *locroot, SigList &sl)
{
	if(!locroot)
		return;
    // search for all signatures in the given node and its child nodes
    locroot->walkNodes([&sl](CRNode *node)->CRNode* {
        if(CR::isSig(node->type))
            sl.push_back(Sig(node));
        return nullptr;
    });
}

template<typename TF>
void Parser<TF>::prepareSigList(SigList &sl)
{
    // for each signature we parse, save the result
    for (SigList::iterator i = sl.begin(), e = sl.end(); i != e; ++i) {
        Sig &sig = (*i);

        prepareSig(sig.node->id, sig);
    }
}

template<typename TF>
void Parser<TF>::addWantedStr(FoundList *pFoundList, const typename NLexer<TF>::token_t &tok, const String &str)
{
    Found found;
    found.line = tok.loc.row;
    //int space = str.indexOf(' ');
    //found.cmt = space>=0 ? str.mid(space+1) : str;

    found.str = str;
    NLexer<TF>::token_t tokNL = Parser::nextLine(tok);
    found.str += TF::toStr(tokNL.ptr, tokNL.length);

    pFoundList->push_back(found);
}

template<typename TF>
bool Parser<TF>::compareSig(const typename NLexer<TF>::token_t &tok, Sig &sig)
{
    int n = sig.lexemes.size();
    if(n > 0) {
        // two iterations: first we compare with the token at the current index, then if not equal - at zero
        for(int i=0; i<2; i++) {
            // if the token matches the current token of the given signature, we continue tracking
            if(tok.isEqu(sig.lexemes[sig.index])) {
                if(sig.index == 0)
                    sig.locFirst = tok.loc;
                sig.index ++;
                break;
            }
            // if the token does not match the current token of the given signature, we reset the tracking
            // but maybe then it matches the first token of this signature
            else {
                sig.index = 0;
            }
        }

        // if this signature is passed to the end - it matched!
        if(sig.index == n) {
            // reset tracking (for next occurrence)
            sig.index = 0;
            return true;
        }
    }
    return false;
}

template<typename TF>
void Parser<TF>::parseSig(const typename NLexer<TF>::token_t &tok, SigList &sl)
{
    // at the input of the next file token
    if(pTokens && compareSig(tok, wanted_sig)) {
        String str = TF::toStr(tok.ptr, tok.length);
        addWantedStr(pTokens, tok, str);
    }

    // loop through all file signatures and compare the current token of each signature with the next file token
    // if some signature completely matched the sequence
    for(SigList::iterator i = sl.begin(), e = sl.end(); i!=e; ++i) {
        // we work with a specific node, inside which a specific signature
        Sig &sig = (*i);
        if(compareSig(tok, sig)) {
            // if this node is not hidden
            if(sig.node->flags & CRNode::NF_VISIBLE) {
                if(sig.node->type == CR::NT_NAME)
                    addWordMark(sig.locFirst, tok.loc.col+tok.length-sig.locFirst.col, Layer(sig.node->nclr, sig.node));
                else
                    handleMark(sig.node->type, sig.locFirst, tok.loc.pos-sig.locFirst.pos+lenUntilNewline(tok.ptr), sig.node->nclr, sig.node);
            }

            // for coating treatment
            sigMatched = true;
            locSig = sig.locFirst;
            return;
        }
    }
    locSig = Loc();
}

template<typename TF>
void Parser<TF>::addFullMark(const Loc &start, int qlines, int qbytes, const Layer &lr)
{
    // add block marker
    if(!pBars)
        return;
    CR::Mark m;
    m.row = start.row;
    m.pos = start.pos;
    m.col = -1;   // sign of selection of whole lines
    m.lines = qlines;
    m.chars = qbytes;

    m.color = lr.rgb;
    m.node = lr.node;
    pBars->push_back(m);
}

template<typename TF>
void Parser<TF>::addWordMark(const Loc &start, int len, const Layer &lr)
{
    // add word marker
    if(!pWords)
        return;
    CR::Mark m;
    m.row = start.row;
    m.pos = start.pos;
    m.col = start.col;
    m.lines = 1;
    m.chars = len;

    m.color = lr.rgb;
    m.node = lr.node;
    pWords->push_back(m);
}

template<typename TF>
void Parser<TF>::parse()
{
    // lexical analyzer
    extern NKeyWord KW[];

    NLexer<TF> lexer(data, size);
    NLexer<TF>::token_t tok;
    lexer.skipLineCmts = false;
    lexer.skipNewlines = false;
    lexer.initOperators(&KW[0], &KW[2], TOKlbrace);

	// REGENERATION
    // look for all nodes with signatures that may be in this document
    // this is best done in advance when creating and editing nodes, and kept in file nodes
    // this mechanism allows you to do this in advance if you need to update the view
    if(fitem && (pCR->updateCount != fitem->updCount)) {
		fitem->sigs.clear();

		fitem->fnode = pCR->getFNode(fitem);

        makeSigList(fitem->fnode, fitem->sigs); // here you need a file node !!!
        makeSigList(pCR->getTags(), fitem->sigs);
        makeSigList(pCR->getOutline(), fitem, fitem->sigs);
        prepareSigList(fitem->sigs);

        fitem->updCount = pCR->updateCount;
    //  qDebug() << "updCount = " << fnode->updCount;
    }

    bool mkNewline = false;
    // scan loop
    while(lexer.Scan(&tok)) {
        // main parsing
        sigMatched = false;

        // state machine that fires on the first token after a line break
        if (mkNewline) {
            locNewline = tok.loc;
            mkNewline = false;
            if(locTop.pos == QUERY_TOP_POS)
                locTop = tok.loc;
        }

        // token parsing - single line comment or language token
        if(tok.value == TOKnewline)
            mkNewline = true;
        else if(tok.value == TOKlinecmt)
            parseCmt(tok);
        else if(fitem)
            parseSig(tok, fitem->sigs);

        // handling curly braces
        if(tok.value == TOKlbrace)
            handleBeginBrace();
        else if(tok.value == TOKrbrace)
            handleEndBrace(tok.loc);

        // coverage processing
        if(pCoverage && tok.value != TOKlinecmt && tok.value != TOKnewline && tok.value != TOKspace && tok.value != TOKblockcmt) {
            // if it is the start of an uncovered area (or a signature to be cancelled)
            if(wait_uncovered && tok.value != TOKrbrace && stColors.empty()) {
            //  qDebug() << "addUncovered: " << QString::fromUtf8(tok.ptr, tok.length) << " v=" << tok.value;
                addWantedStr(pCoverage, tok, _T(""));

                locUncovered = tok.loc;
                wait_uncovered = false;
            }
            // otherwise if we are inside the cover
            else if(!stColors.empty()) {
                wait_uncovered = true;
            }
            // if the signature has just been caught there
            if(sigMatched && locSig == locUncovered) {
                if(!pCoverage->empty()) {
                    Found d = pCoverage->back();
                    pCoverage->pop_back();
                //  qDebug() << "popUncovered: " << d+1 << " wu=" << wait_uncovered;
                    wait_uncovered = true;
                }
            }
        }
    }
}

template<typename TF>
void Parser<TF>::parseMarkup(CR::MarkList *_pbl, CR::MarkList *_pwl)
{
    // parsing for coloring
    pBars = _pbl;
    pWords = _pwl;
    parse();
    pBars = nullptr;
    pWords = nullptr;
}

template<typename TF>
void Parser<TF>::parseCoverage(FoundList *_pcl)
{
    // parsing to build coverage
    pCoverage = _pcl;
    parse();
    pCoverage = nullptr;
}

template<typename TF>
CRNode* Parser<TF>::parseFindNode(int line, int col, CR::Mark *pm)
{
    // parsing to search for a node by row and column - option 1 (with row and column number)
	// the advantage is that the return is the same as the draw, i.e. the area after the closing curly brace
	// gets selected by line*col but not by pos..pos+chars
    CR::MarkList bl, wl;
    pBars = &bl;
    pWords = &wl;
    parse();
    // analyze the resulting list - first the words, then the areas
    for(CR::MarkList::const_iterator i=wl.begin(), e=wl.end(); i!=e; ++i) {
        const CR::Mark &m = (*i);
        if(line == m.row && col >= m.col && col <= m.col + m.chars) {
			if(pm)
				*pm = m;
            return m.node;
		}
    }
    for(CR::MarkList::const_iterator i=bl.begin(), e=bl.end(); i!=e; ++i) {
        const CR::Mark &m = (*i);
        if(line >= m.row && line < m.row + m.lines) {
			if(pm)
				*pm = m;
            return m.node;
		}
    }
    pBars = nullptr;
    pWords = nullptr;
    return nullptr;
}

template<typename TF>
CRNode* Parser<TF>::parseFindNode(int pos, CR::Mark *pm)
{
    // parsing to search for a node by row and column - option 2 (with absolute position from the beginning of the text)
    CR::MarkList bl, wl;
    pBars = &bl;
    pWords = &wl;
    parse();
    // analyze the resulting list - first the words, then the areas
    for(CR::MarkList::const_iterator i=wl.begin(), e=wl.end(); i!=e; ++i) {
        const CR::Mark &m = (*i);
        if(pos >= m.pos && pos <= m.pos + m.chars) {
			if(pm)
				*pm = m;
            return m.node;
		}
    }
    for(CR::MarkList::const_iterator i=bl.begin(), e=bl.end(); i!=e; ++i) {
        const CR::Mark &m = (*i);
        if(pos >= m.pos && pos < m.pos + m.chars) {
			if(pm)
				*pm = m;
            return m.node;
		}
    }
    pBars = nullptr;
    pWords = nullptr;
    return nullptr;
}

template<typename TF>
void Parser<TF>::parseFindToken(const String &code, FoundList *_pfl)
{
    // find token in code
    pTokens = _pfl;
    //wanted_str = code;
    prepareSig(code, wanted_sig);
    // parse the code here and apply the logic similar to the search for signatures in the search
    parse();
    pTokens = nullptr;
}

template<typename TF>
void Parser<TF>::parseFindFuid(const String &fuid, FoundList *_pfl)
{
    // find unique identifier in MK
    pFuids = _pfl;
    wanted_str = fuid;
    parse();
    pFuids = nullptr;
}

template<typename TF>
void Parser<TF>::parseFindTag(const String &tag, FoundList *_pfl)
{
    // find tag in MC
    pTags = _pfl;
    wanted_str = tag;
    parse();
    pTags = nullptr;
}

template<typename TF>
typename NLexer<TF>::token_t Parser<TF>::prevLine(typename NLexer<TF>::token_t tok)
{
    // static, go to previous line, including double /r/n
    // UNDONE; it would be nice to also have a null character before the beginning of the text
    tok.ptr --;
    tok.length = 0;

    // double line break character - just such a sequence
    if(tok.ptr[0]=='\n' && tok.ptr[-1]=='\r')
        tok.ptr--;

    return tok;
}

template<typename TF>
typename NLexer<TF>::token_t Parser<TF>::nextLine(typename NLexer<TF>::token_t tok)
{
    // static, go to next line, double /r/n
    tok.ptr += tok.length;
    tok.length = 0;

//    qDebug() << QString::asprintf("%X %X %X", tok.ptr[0], tok.ptr[1], tok.ptr[2]);

    // double line break character - just such a sequence
    if(tok.ptr[0]=='\r' && tok.ptr[1]=='\n')
        tok.ptr++;

    if(*tok.ptr == '\r' || *tok.ptr == '\n') {
        tok.ptr++;

        tok.loc.row++;
        tok.loc.col=0;
        while(tok.ptr[tok.length]!=0 && tok.ptr[tok.length]!='\r' && tok.ptr[tok.length]!='\n')
            tok.length++;
    }
    return tok;
}


#endif // PARSE_H
