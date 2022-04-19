#pragma once

#include <vector>
#include <list>
#include "../tools/tfbase.h"
#include "token.h"
#include "loc.h"


class CRNode;

// signature
struct Sig {
    CRNode *node;   // signature node in the tree
    std::vector< Lexeme<Utf16::char_t> > lexemes;   // a vector of tokens forming a signature
    int index;      // index in the vector of tokens used for searching
    Loc locFirst;   // location of the first token of the signature when searching
    Sig(CRNode *n=nullptr)
        : node(n), index(0) {}
};
// list of signatures
typedef std::list<Sig> SigList;

// file
struct FItem {
	String	fpath;	// the path to the file; in principle, by construction it cannot be empty, but the checks in the code remained from the previous version
	SigList sigs;	// signatures of this file
	int updCount;	// regeneration counter
	CRNode *fnode;	// file node corresponding to this file (if any)

	FItem(const String &path)
		: fpath(path)
		, updCount(0)
		, fnode(nullptr)
	{}
};
// list of files
typedef std::list<FItem*> FList;

