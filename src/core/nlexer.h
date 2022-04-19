#pragma once
#include "kw.h"
#include "token.h"

/*** TF specification (TextFormat)
struct TF
{
	static const char* next(const char* s);
 	static const char* match(const char *stream, const char *pattern);
};
*/

// implementation of the lexical analyzer
// maybe it's better to move LexerDOM into a separate class, and leave pure SAX here
template<typename TF>
class NLexer 
{
public:
	enum LexerFlags {
		LF_SKIP_SPACES = 0x1,
		LF_SKIP_NEWLINES = 0x2,
		LF_SKIP_BLOCKCMTS = 0x4,
		LF_SKIP_LINECMTS = 0x8,
		LF_MERGE_NEWLINES = 0x10
	};
	typedef typename TF::char_t char_t;
    typedef Token<char_t> token_t;
public:
	Loc scanLoc;
	bool blockCmt;	// we are in a block comment (for line-by-line analysis - the state of the previous line)
public:
	void init()
	{
		blockCmt = false;
		m_pKeyArr = nullptr;
        m_pKeyEnd = nullptr;
        m_nKeyOffs = 0;
        m_pOprArr = nullptr;
        m_pOprEnd = nullptr;
        m_nOprOffs = 0;
        scanLoc.row = 0;
        scanLoc.col = 0;
		skipSpaces = true;
		skipNewlines = true;
		skipLineCmts = true;
		skipBlockCmts = true;
		mergeNewlines = false;
	}
	NLexer()
	{
        init();
		m_pBuffer = nullptr;
        m_pEnd = nullptr;
        m_pCurr = nullptr;
		m_pLine = nullptr;
	}
	NLexer(const char_t *base, size_t size)
	{
		init();
		initBuffer(base, size);
	}

	void initBuffer(const char_t *base, size_t size)
	{
		m_pBuffer = base;
		m_pEnd = base + size;
		m_pCurr = base;
		m_pLine = base;
	}
    void initKeywords(const NKeyWord *begin, const NKeyWord *end, int offs)
	{
        m_pKeyArr = begin;	// first item
        m_pKeyEnd = end;	// after last item
        m_nKeyOffs = offs;
	}
    void initOperators(const NKeyWord* begin, const NKeyWord* end, int offs)
	{
        m_pOprArr = begin;
        m_pOprEnd = end;
        m_nOprOffs = offs;
	}
    void initState(const token_t *t)
	{
		// init lexer state from token
		m_pCurr = t->ptr;
		m_pLine = m_pCurr;	//@todo, find the beginning of a string, especially for utf8
	}

    const char_t* currLine()
    {
        return m_pLine;
    }

	// current position
	Loc updateLoc()
	{
        scanLoc.col = (unsigned)(m_pCurr - m_pLine);
        scanLoc.pos = m_pCurr - m_pBuffer;
		return scanLoc;
	}

    bool Scan(token_t *pToken)
	{
		// extracting the next token; loop - in case we skip some tokens (spaces etc.)
		pToken->ptr = m_pCurr;
		while (m_pCurr < m_pEnd)
		{
			// init token
            pToken->ptr = m_pCurr;
			pToken->length = 0;
			pToken->value = TOKreserved;
			pToken->loc = updateLoc();

            if (m_pCurr >= m_pEnd)
				return 0;

			// extract token
			if (blockCmt || TF::isBlockCmt(pToken->ptr))
			{
				extractBlockCmt(pToken);
				if (!skipBlockCmts) break;
			}
			else if (TF::isLineCmt(pToken->ptr))
			{
				extractLineCmt(pToken);
				if (!skipLineCmts) break;
			}
			else if (ct::isNewline(*pToken->ptr))
			{
				extractNewline(pToken);
				if (!skipNewlines) break;
			}
			else if (ct::isOper(*pToken->ptr))
			{
				extractOperator(pToken);
				break;
			}
			else if (ct::isIdnext(*pToken->ptr) || ct::isQuote(*pToken->ptr))
			{
				extractName(pToken);
				break;
			}
			else if (ct::isSpace(*pToken->ptr))
			{
				extractSpace(pToken);
				if (!skipSpaces) break;
			}
			else
			{
				extractBad(pToken);
				break;
			}
		}

        if (pToken->ptr >= m_pEnd)//@???
			return 0;
		return 1;
	}

public:
    // if the token is the last one, then skip does not work; I don't know if this is good or bad, and whether this feature is needed at all
	bool	skipSpaces;		// skip spaces automatically
	bool	skipNewlines;	// skip line breaks automatically
	bool	skipLineCmts;	// automatically skip single line comments
	bool	skipBlockCmts;	// skip block comments automatically
	bool    mergeNewlines;	// merge line breaks into one token
protected:
	const char_t*	m_pBuffer;	// source code buffer
	const char_t*	m_pEnd;		// end of buffer (points to trailing '\0')
	const char_t*	m_pCurr;    // current character
	const char_t*	m_pLine;    // start of current line
	
	const NKeyWord *m_pKeyArr, *m_pKeyEnd;	// keyword table
	const NKeyWord *m_pOprArr, *m_pOprEnd;	// operators table
    int m_nKeyOffs, m_nOprOffs;             // offsets to turn kw and op into an index

protected:	// group of functions for generating numeric and string representations
// here or some external Converter?

protected:	// group of functions to extract
    bool extractOperator(token_t *pToken)
	{
		// extract operator; sequential check of all groups of operators starting from the longest
		for (const NKeyWord *op = m_pOprArr; op != m_pOprEnd; op++)
		{
            if (TF::match(pToken->ptr, op->size, op->name))
			{
                pToken->value = (TOK)(op - m_pOprArr) + m_nOprOffs;
				pToken->length = op->size;
				m_pCurr += pToken->length;
				return true;
			}
		}
		pToken->value = TOKreserved;
		pToken->length = 1;
		m_pCurr += pToken->length;
		return false;
	}

    bool extractKeyword(token_t *pToken)
	{
		// see if it's a keyword
		for (const NKeyWord *kw = m_pKeyArr; kw != m_pKeyEnd; kw++)
		{
            if (TF::match(pToken->ptr, pToken->length, kw->name))
			{
				// keyword
                pToken->value = (TOK)(kw - m_pKeyArr) + m_nKeyOffs;
				pToken->length = kw->size;
				m_pCurr += pToken->length;
				return true;
			}
		}
		return false;
	}

    void extractName(token_t *pToken)
	{
		// extract name; go through the array until then
		// as long as characters satisfy id, consider situation 12e-7

		bool bSpec = 0;		// backslashes in a line
		bool bQuote1 = 0;	// single quote
		bool bQuote2 = 0;	// double quote
		TOK  tConst = TOKreserved;	// constant

		const char_t *p = pToken->ptr;
		// digit
		if (ct::isDigit(*p))
			tConst = TOKnumber;
		// single quote
		else if (ct::isQuote1(*p)) 
			bQuote1 = 1, tConst = TOKsqstr;
		// double quote
		else if (ct::isQuote2(*p))
			bQuote2 = 1, tConst = TOKdqstr;
		
		// immediately to the next character
		p = TF::next(p);

		// main loop
		while (p < m_pEnd)
		{
			// checking for the end of the line "line break", do not include the end of the line!!!
			if (ct::isNewline(*p))
				break;
			// end-of-line check for non-quotes: not an identifier character and not a quote
			if (!bQuote1 && !bQuote2 && !ct::isIdnext(*p) && !ct::isQuote(*p))
				break;
			// check for single quote
			if (!bSpec && !bQuote2 && ct::isQuote1(*p)) {
				bQuote1 = !bQuote1;
				tConst = TOKsqstr;
			}
			// check for double quote
			if (!bSpec && !bQuote1 && ct::isQuote2(*p)) {
				bQuote2 = !bQuote2;
				tConst = TOKdqstr;
			}
			// check for backslash - only for strings
			if ((bQuote1 || bQuote2) && !bSpec && ct::isSpec(*p))
				bSpec = 1;
			else
				bSpec = 0;
			p = TF::next(p);
		}

		// length in bytes
		pToken->length = (p - pToken->ptr);
		m_pCurr += pToken->length;

		// type
		if (tConst)
			pToken->value = tConst;
		else if (!extractKeyword(pToken))
			pToken->value = TOKidentifier;
	}

    void extractLineCmt(token_t *pToken)
	{
		// extract single line comment
		// on input: '//'
        pToken->value = TOKlinecmt;
		const char_t *p = pToken->ptr;
		while (p < m_pEnd && !ct::isNewline(*p))
			p = TF::next(p);
		pToken->length = (p - pToken->ptr);
		m_pCurr += pToken->length;
	}

	void handleNewline(const char_t *p)
	{
		// crlf -> one carry; all cr and lf are converted to crlf; //!todo
		if (p[0] == '\r' && p[1] == '\n')
            scanLoc.row--;
		else
			m_pLine = TF::next(p);
        scanLoc.row++;
	}

	void extractBlockCmt(token_t *pToken)
	{
		// extract multiline comment
		// on input: '/*'
		pToken->value = TOKblockcmt;
		const char_t *p = pToken->ptr;
		blockCmt = true;

		// until end of buffer or end of comment marker
		while (p < m_pEnd && !(p[0] == '*' && p[1] == '/')) {
			if (ct::isNewline(*p))
				handleNewline(p);
			p++;
		}

		// comment length
		pToken->length = (p - pToken->ptr);
	
		// take into account '*/'
		if(p[0] == '*' && p[1] == '/') {
			blockCmt = false;
			pToken->length += 2;
		}
		
		m_pCurr += pToken->length;
	}

    void extractNewline(token_t *pToken)
	{
		// extract one or more newline characters
        pToken->value = TOKnewline;
		pToken->length = 0;
		const char_t *p = pToken->ptr;
		while (p < m_pEnd && ct::isNewline(*p))
		{
			handleNewline(p);
			p = TF::next(p);
			if (!mergeNewlines)
				break;
		}
		pToken->length = (p - pToken->ptr);
		m_pCurr += pToken->length;
	}

    void extractSpace(token_t *pToken)
	{
		// extract space
		pToken->value = TOKspace;
		const char_t *p = pToken->ptr;
		while (p < m_pEnd && ct::isSpace(*p))
			p = TF::next(p);
		pToken->length = (p - pToken->ptr);
		m_pCurr += pToken->length;
	}

    void extractBad(token_t *pToken)
	{
		// bad symbol
		pToken->value = TOKreserved;
		pToken->length++;
		m_pCurr++;
	}
};


