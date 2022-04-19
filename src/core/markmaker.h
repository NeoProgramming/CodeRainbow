#ifndef MARKMAKER_H
#define MARKMAKER_H

#include "crnode.h"

typedef void (*FnModifyTree)(void *arg, CRNode *node, CRNode *active);

class MarkMaker
{
public:
	void clear();
    int  beginMakeMarker(const String &fpath, int ri, CR::Type type, const String &str, int index);
    void endMakeMarker(FnModifyTree fnModify, void *arg);

    int  beginEditMarker(const String &fpath, const String &str, CRNode *enode);
    void endEditMarker(FnModifyTree fnModify, void *arg);
//signals:
//    void treeItemModified(CRNode *act, CRNode *node);
protected:
    void create(CRMark &mi, CRInfo &ni);
    void createFuid(CRInfo &ni, CR::Type type);
    void createTags(CRMark &mi, Clr clr);
    void createTag(const String &tag, Clr clr);
    void createElem(CRInfo &ni, CR::Type type);
    void createTsig(CRInfo &ni, CR::Type type);

	void updateFuid(CRInfo &ni);
	void updateElem(CRInfo &ni);

    String extractSignature(String &str, int index);
    String extractName(String &str, int index);
    String extractSuggestedId(String &str, int index);

    void treeItemModified(CRNode *act, CRNode *node);
public:
	FnModifyTree m_fnModify;
	void* m_pArg;
    CRMark m_mi;
    CRInfo m_ni;
    String m_head, m_tail;
    CRNode *m_nodeFuid, *m_nodeNote;
    int m_ti, m_ri;

    String m_fpath;     // open file being marked up/edited
    CRNode *m_nodeFile; // node corresponding to the file in FILES
//	FItem *m_fitem;		// the element corresponding to the open file being marked up/edited

    String m_str;
    String m_beg, m_end;
};

#endif // MARKMAKER_H
