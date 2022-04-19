#pragma once

#ifdef QT_CODE
#include <QTreeWidgetItem>
#include <QPersistentModelIndex>
#endif

#include <functional>
#include "../tools/tdefs.h"
#include "../tools/tfbase.h"
#include "token.h"
#include "sig.h"

namespace CR {
enum Type {
    NT_NONE,        // unknown node
    NT_ROOT,        // main root
    NT_FBASE,       // filesystem root
    NT_TAGS,        // tags root
    NT_OUTLINE,		// outline root

    NT_FILE,        // file
    NT_DIR,         // native directory
    NT_GROUP,       // virtual group
    NT_LINK,        // link to external resource
    NT_TAG,         // tag (multiple marker)

    NT_AREA,        // area mk
    NT_LABEL,       // label mk
    NT_BLOCK,       // block mk
    NT_LSIG,        // label signature
    NT_BSIG,        // block signature
    NT_NAME,        // name signature

    NT_AEND,        // special marker for end of AREA
    NT_ItemsCount
};
// see also gui.h struct MarkBtn

bool isFS(CR::Type t);
bool isFile(CR::Type t);
bool isSig(CR::Type t);
bool isMcmt(CR::Type t);
bool isTag(CR::Type t);
bool isMark(CR::Type t);
bool isItem(CR::Type t);
bool isLocal(CR::Type t);
bool isSys(CR::Type t);

enum ToolIcons 
{
    TI_INFO,
    TI_MOVE,
    TI_EDIT,
    TI_REMOVE,
    TI_ADD,
    TI_COVER,
    TI_ACTIVE,
    TI_SETBASE,

    TI_ItemsCount
};

CStr typeName(CR::Type type);
Type typeValue(CStr name);
}

struct CRMark 
{
    unsigned int mclr;  // metacomment background color, set DIRECTLY IN THE COMMENT through a hash
    String id;          // id, a strong identifier, exactly what is put in the code; OR signature
    StrList tags;       // list of tags separated by commas
    CR::Type type;
    bool mk_fuid;   // create in FILES
    bool mk_elem;   // create in OUTLINE
    bool mk_tsig;   // create in TAGS
	bool gen_uid;	// work without dialog, generate unique id automatically

	void clear() 
	{ 
		mclr=CLR_NONE;
		id.clear();
		tags.clear();
		type=CR::NT_NONE;
		mk_fuid=true;
		mk_elem=true;
		mk_tsig=true;
		gen_uid=false;
	}
    CRMark() 
	{ 
		clear(); 
	}
    String tagStr();
    void setTags(const String &str);
};


struct CRInfo 
{
	enum Flags {
        NF_ACTIVE = 0x1,    // internal active
        NF_CURRENT = 0x2,   // current active
        NF_CHECKED = 0x4,	// marked with a checkbox
        NF_VISIBLE = 0x8,	// actual visibility (taking into account the visibility of top-level elements)
		NF_MERGETID = 0x10, // Title+ID
    };

	unsigned int nclr; // metacomment background color set IN NODE
    unsigned int tclr; // background color IN TREE
	String id;         // identifier, exactly what is put in the code
	String name;       // "display name", "short comment", although usually the same as path
	String text;       // extended text description
	FItem *fitem;	   // file element or null, non-owning pointer
	unsigned int flags;// flags: activity flag

	void clear() 
	{ 
		nclr = CLR_NONE;
        tclr = CLR_NONE;
		id.clear(); 
		name.clear();
	//	path.clear();
		text.clear();
		fitem = nullptr;
		flags = NF_VISIBLE|NF_CHECKED;
	}
	CRInfo() 
	{
		clear();
	}
	void setData(const CRInfo &n)
	{
		nclr = n.nclr;
        tclr = n.tclr;
		id = n.id;
		name = n.name;
		text = n.text;
	}
};


class CRNode : public CRInfo
{
public:

   

public:
    CRNode() : CRInfo(), type(CR::NT_NONE),
        parent(nullptr), gparam(nullptr)
#ifdef QT_CODE 
		, item(nullptr)
#endif
	{}
    virtual ~CRNode();

	bool canRemoveBranch(CRNode* &branch);
    
    bool removeBranch(CRNode* &branch);
    void removeBranches();

    String absPath();
public:
    typedef std::vector<CRNode*>	NList;
    CRNode* parent;             
    NList branches;				
	String incpath;				// path to include, if any, for chains of cr-files
#ifdef QT_CODE 
    QTreeWidgetItem *item;      // for cr_editor
    QPersistentModelIndex idx;  // index in model (for cr_plugin)
#endif
    
    CR::Type type;
    void *gparam;               // gui paramter
    
public:
    CRNode* getParent();
    CRNode* getPathNode();
    CRNode* getSysNode();
    CR::Type getSysType();
    CRNode* getCurrActive();
	String getDir();

    void* getParam();
    void setParam(void *p);

    CRNode* findFile(const String &fpath);
    CRNode* findTag(const StrList &tags);
    CRNode* findChildId(const String &id);
    CRNode* findChildPath(const String &path);
    CRNode* findChildPathId(FItem *wanted_fitem, const String &wanted_id);
    bool hasLocalNodes();

    void setFlags(unsigned int add, unsigned int del);
    void setSubtreeFlags(unsigned int add, unsigned int del);
    CRNode* walkNodes(const std::function<CRNode*(CRNode*)> &fn);
    String dispName();
    String fullName();

    int branchesCount();
    CRNode* branchByIndex(int i);
    int branchIndex(CRNode* branch);

    String getInfo(char delim='\n');
    void updateSubtreeVisibility(bool vis);

    CRNode* findCurrActive();
public:
    static CRNode* mkNode(CRNode *root, CR::Type t = CR::NT_NONE);
    static CRNode* mkFSNode(CRNode *root, CR::Type t, const String &name, const String &path, bool inc);
};

#ifdef QT_CODE
// special declaration in order to be able to pack the node into variants
Q_DECLARE_METATYPE(CRNode*)

#endif

