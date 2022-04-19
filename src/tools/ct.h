#pragma once

namespace ct
{
	bool	isSpace(int c);
	bool	isNewline(int c);
	bool	isBlank(int c);
	bool	isDigit(int c);
	bool	isHexDigit(int c);
	bool	isOctDigit(int c);
	bool	isBinDigit(int c);
	bool    isQuote1(int c);
	bool	isQuote2(int c);
	bool	isQuote(int c);
	bool	isOper(int c);
	bool	isAlpha(int c);
	bool	isIdstart(int c);
	bool	isIdnext(int c);
	bool    isString(int c);
	bool	isSpec(int c);
	bool	isAlnum(int c);
	bool	isExp(int c);
	bool    isFName(int c);
};
