#pragma once

#include "../pugixml/pugixml.hpp"
#include "../tools/tdefs.h"

#include "crnode.h"
#include "cruser.h"
#include "fmanager.h"


enum EInsertMode {
	EIM_PARENT,
	EIM_PREDECESSOR,
	EIM_SUBSEQUENT,
	EIM_REPLACEMENT
};

struct PrjProps {
    String cmt;    // line comment in programming language
    // do you need it in settings? because if everyone adjusts for themselves, unification will disappear!
    String area;   // area start
    String aend;   // area end
    String label;  // label
    String block;  // block

public:
    PrjProps() { initDef(); }
    void initDef();
};



// cross-platform tree manager
class CRTree {
    friend class CRNode;
public:
    CRTree();
    ~CRTree();

    void setCurrentProject(const String &crpath, const String &basedir, bool confirmed);
    void invalidateSignatures()
    {
        updateCount++;
    }

	bool isEmpty();

    void createEmpty(const String &basename, const String &basedir);
    bool createByDir(const String &basename, const String &basedir, const String &filters, unsigned int flags);
    bool createByLst(const String &basename, const String &basedir, const StrList &lst, unsigned int flags);

    bool updateByDir(const String &filters, unsigned int flags);
    bool updateByLst(const StrList &lst, unsigned int flags);

    bool loadCR(const String &path);
    bool saveCR(const String &path);

    Char* canAcceptMarkup(CRNode *fnode, bool toFiles, bool toTags, bool toOutline, const String &id, const String &ts);

	CRNode* addNode(CRNode *nodeActive, CR::Type type, const String &id, const String &name, FItem *fitem, const String &text, unsigned int nclr);
    CRNode* addNode(CRNode *nodeActive, CR::Type type, const String &id, const String &name, const String &path, const String &text, unsigned int nclr);
    CRNode* addNode(CRNode *nodeActive, CR::Type type, const CRInfo &ni);

	CRNode* updateNode(CRNode *nodeActive, CR::Type type, const CRInfo &ni, int *res = nullptr);

    CRNode* findTag(const StrList &taglist);
    CRNode* findOutlineElem(FItem *fitem, const String &id);

	bool canRemoveNode(CRNode *node);
    bool removeNode(CRNode *node);
    bool moveNodeUp(CRNode* node);
    bool moveNodeDown(CRNode* node);
    bool moveNodeLeft(CRNode* node);
    bool moveNodeRight(CRNode* node);
    bool moveNodeTo(CRNode *node, CRNode *npar);
    void setText(CRNode* node, const String &text);
	void setIncPath(CRNode* node, const Char *path);
    bool setActive(CRNode *node);
	CRNode *getActive(CRNode *node);
    void updateCurrents(CRNode *fnode);
	void convertBase(bool separate);
	
    CRNode* getRoot() { return &root; }
    CRNode* getFiles() { return &fbase; }
    CRNode* getTags() { return &tags; }
    CRNode* getOutline() { return &outline; }
	CRNode* getFNode(FItem *fitem);
    CRNode* getLocalRoot(CRNode *node);

    CRNode* getFileNode(const String &fpath);
    CRNode* addFileNode(const String &fpath, CRNode *par = nullptr);

public:
    PrjProps pp;
    bool modify;
	CRUser user;
	FManager fmgr;
    String crFPath;	// path to root .cr file
	String baseDir; // base directory (probably used to be fbase.path)
    bool confirmedPath;
    int  updateCount;
	EInsertMode insMode;
private:
    CRNode root;
    CRNode fbase;
    CRNode tags;
    CRNode outline;

private:
    bool loadNode(pugi::xml_node elem, CRNode *node, const Path &base);
    bool saveNode(pugi::xml_node elem, CRNode *node, const Path &base);
    
};

// global object
extern CRTree *pCR;

