#pragma once
#include "loc.h"

enum TOK
{
    TOKreserved,
    TOKidentifier,
    TOKnewline,
    TOKspace,
    TOKblockcmt,
    TOKlinecmt,
    TOKnumber,
    TOKsqstr,
    TOKdqstr,

    TOKoperator,
    TOKkeyword,
    TOKpreproc,

	TOKlbrace,
	TOKrbrace,

    TOKCOUNT
};

template<typename char_t>
struct Lexeme
{
    const char_t *ptr;
    int value;
    size_t length;	// length in characters, not in bytes[???]

    template<typename char2_t>
    bool isEqu(const Lexeme<char2_t> &lex) const
    {
        //@bad! works only for ascii
        if(length != lex.length || value != lex.value)
            return false;
        for(size_t i=0; i<length; i++)
            if(ptr[i]!=lex.ptr[i])
                return false;
        return true;
        //@return !strncmp(ptr, lex.ptr, length*sizeof(char_t));
    }
};

template<typename char_t>
struct Token : public Lexeme<char_t>
{
    Loc loc;
};

