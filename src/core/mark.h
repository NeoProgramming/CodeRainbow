#ifndef MARK_H
#define MARK_H
#include "../tools/tdefs.h"

class CRNode;

namespace CR {

class Mark {
public:
    Mark()
        : pos(-1)
        , row(-1)
        , col(-1)
        , lines(0)
        , chars(0)
        , color(CLR_NONE)
        , node(nullptr)
    {}
    int pos;    // position from the beginning of the text
    int row;    // line number==block
    int col;    // column number==character
    int lines;  // number of lines or characters per line (depending on what kind of marker it is)
    int chars;  // number of bytes from pos to end

    Clr color;  
    CRNode *node;    // the node corresponding to the given coloring; for back tracing purposes
};

typedef std::list<Mark> MarkList;

typedef MarkList::iterator MarkListIter;
typedef MarkList::const_iterator MarkListConstIter;

} // ns CR


#endif // MARK_H
