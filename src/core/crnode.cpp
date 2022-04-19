#include "crnode.h"
#include "crtree.h"
#include "parse.h"
#include <tchar.h>
#include <boost/algorithm/string.hpp>
#include "../tools/tdefs.h"

namespace CR {
bool isFS(CR::Type t)
{
    return t == NT_FILE || t == NT_DIR || t == NT_FBASE;
}
bool isFile(CR::Type t)
{
    return t == NT_FILE;
}
bool isSig(CR::Type t)   // sig (not mk)
{
    return t == NT_LSIG || t == NT_BSIG || t == NT_NAME;
}
bool isMcmt(CR::Type t)
{
    return t == NT_AREA || t == NT_LABEL || t == NT_BLOCK;
}
bool isTag(CR::Type t)
{
    return t == NT_TAG;
}
bool isMark(CR::Type t)
{
    return isMcmt(t) || isTag(t);
}
bool isItem(CR::Type t)
{
    return isMcmt(t) || isTag(t) || isSig(t);
}
bool isLocal(CR::Type t)
{
    return isMcmt(t) || isSig(t);
}
bool isSys(CR::Type t)
{
    return t == NT_ROOT || t == NT_FBASE || t == NT_TAGS || t == NT_OUTLINE;
}

static const Char *NodeTypes[] = {
	_T("node"),
	_T("root"),
	_T("fbase"),
	_T("tags"),
	_T("outline"),

	_T("file"),
	_T("dir"),
	_T("group"),
	_T("link"),
	_T("tag"),

	_T("area"),
	_T("label"),
	_T("block"),
	_T("lsig"),
	_T("bsig"),    
	_T("name"),

	_T("aend"),
	nullptr
};

CStr typeName(CR::Type type)
{
    if(type < 0 || type >= CR::NT_ItemsCount)
        return _T("<bad type>");
    return NodeTypes[type];
}

Type typeValue(CStr name)
{
    for(int i=0; NodeTypes[i]; i++) {
        if(strEqu(name, NodeTypes[i]))
            return (CR::Type)i;
    }
    return NT_NONE; // unknown!
}

} //end CR

String CRMark::tagStr()
{
    return boost::algorithm::join(tags, _T(","));
}

void CRMark::setTags(const String &str)
{
    tags.clear();
    str_split(str, _T(','), tags);
}

CRNode::~CRNode()
{
    removeBranches();
}

bool CRNode::canRemoveBranch(CRNode * &branch)
{
	NList::iterator it = std::find(branches.begin(), branches.end(), branch);
    if(it == branches.end())
        return false;
    return true;
}

bool CRNode::removeBranch(CRNode * &branch)
{
    //if(!branches.removeOne(branch))
    //    return false;
    NList::iterator it = std::find(branches.begin(), branches.end(), branch);
    if(it == branches.end())
        return false;
    branches.erase(it);
    delete branch;
    branch = nullptr;
    return true;
}

void CRNode::removeBranches()
{
//    qDebug() << "removeBranches: cnt=" << branches.count();
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *node = *i;
//        qDebug() << "removeBranches LOOP: " << node->path;
        *i = nullptr;
        // if the destructor is virtual, then exactly what is needed will be deleted
        delete node;
    }
    branches.clear();
}

CRNode* CRNode::mkNode(CRNode *root, CR::Type t)
{
    CRNode * node;
    node = new CRNode();
    node->type = t;
    node->parent = root;
    root->branches.push_back(node);
    return node;
}

CRNode* CRNode::mkFSNode(CRNode *root, CR::Type t, const String &name, const String &path, bool inc)
{
    CRNode * node = mkNode(root, t);
    node->name = name;
	//if(t != CR::NT_DIR)
	node->fitem = pCR->fmgr.regFileItem( path );
	if(inc)
		node->incpath = path + _T(".cr");
    return node;
}

String CRNode::getDir()
{
	// destination - to set the initial directory in the file selection dialog when adding a new file
    if(fitem && !fitem->fpath.empty()) {
        Path p(fitem->fpath);
        return p.parent_path().generic_wstring();
    }
    if(!incpath.empty()) {
        Path p(incpath);
        return p.parent_path().generic_wstring();
    }
	if(!parent)
		return _T("");
	return parent->getDir();
}

CRNode* CRNode::getPathNode()
{
	// find the "path" node, i.e. node containing the path for this node
    if((fitem && !fitem->fpath.empty()) && (CR::isFile(type) || CR::isItem(type)))
        return this;
    if(!parent)
        return nullptr;
    return parent->getPathNode();
}

CRNode* CRNode::getSysNode()
{
    if(CR::isSys(type))
        return this;
    if(!parent)
        return nullptr;
    return parent->getSysNode();
}

CR::Type CRNode::getSysType()
{
    if(CR::isSys(type))
        return type;
    if(!parent)
        return CR::NT_NONE;
    return parent->getSysType();
}

void CRNode::setFlags(unsigned int add, unsigned int del)
{
    flags |= add;
    flags &= ~del;
}

void CRNode::setSubtreeFlags(unsigned int add, unsigned int del)
{
    setFlags(add, del);
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *node = (*i);
        node->setSubtreeFlags(add, del);
    }
}

CRNode* CRNode::findCurrActive()
{
	// find active node:
	// if this one - return it
    if(flags & NF_CURRENT)
        return this;
	// search among children
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *node = (*i)->findCurrActive();
        if(node)
            return node;
    }
    // not found: due to recursion
    return nullptr;
}

CRNode* CRNode::getCurrActive()
{
    CRNode* node = findCurrActive();
    if(node)
        return node;
    flags |= NF_ACTIVE|NF_CURRENT;
    return this;
}

CRNode* CRNode::getParent()
{
    return parent;
}

void* CRNode::getParam()
{
    return gparam;
}

void CRNode::setParam(void *p)
{
    gparam = p;
}

CRNode* CRNode::walkNodes(const std::function<CRNode*(CRNode*)> &fn)
{
    CRNode* res = fn(this);
    if(res)
        return res;
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        res = (*i)->walkNodes(fn);
        if(res)
            return res;
    }
    return nullptr;
}

CRNode* CRNode::findFile(const String &fpath)
{
    // find a node with a specific path (in the node and all its children)
    if(CR::isFile(type) && fitem) {
        //if(!path.compare(fpath, Qt::CaseInsensitive))
        if(boost::filesystem::equivalent(fitem->fpath, fpath))
            return this;
    }
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        if(CRNode* fn = (*i)->findFile(fpath))
            return fn;
    }
    return nullptr;
}

CRNode* CRNode::findChildPath(const String& wanted_path)
{
    // find the path in the node and all its children
    if(wanted_path.empty())
        return nullptr;
    if(fitem && boost::filesystem::equivalent(fitem->fpath, wanted_path))
        return this;
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *node = (*i)->findChildPath(wanted_path);
        if(node)
            return node;
    }
    return nullptr;
}

CRNode* CRNode::findChildId(const String &wanted_id)
{
    // find the id in the node and all its children
    if(wanted_id.empty())
        return nullptr;
    if(id == wanted_id)
        return this;
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *node = (*i)->findChildId(wanted_id);
        if(node)
            return node;
    }
    return nullptr;
}

CRNode* CRNode::findChildPathId(FItem *wanted_fitem, const String &wanted_id)
{
    // find the path and id in the node and all its children
    if(wanted_fitem == nullptr || wanted_id.empty())
        return nullptr;
    if(id == wanted_id && fitem == wanted_fitem)
        return this;
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *node = (*i)->findChildPathId(wanted_fitem, wanted_id);
        if(node)
            return node;
    }
    return nullptr;
}

CRNode* CRNode::findTag(const StrList &tags)
{
    // find the tag node from the given list of tags
    if(CR::isTag(type)) {
        //if(tags.contains(id))
		if((std::find(tags.begin(), tags.end(), id) != tags.end()))
            return this;
    }

    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        if(CRNode* fn = (*i)->findTag(tags))
            return fn;
    }
    return nullptr;
}

String CRNode::dispName()
{
	static const String unnamed(_T("<unnamed node!>"));
	// short display name of the node
    if(name.empty()) {
        if(id.empty()) {
			if(!fitem) {
				return unnamed;
			}
			return fitem->fpath;
		}
        return id;
    }
	if(flags & NF_MERGETID)
		return name + _T(" ") + id;
    return name;
}

String CRNode::getInfo(char delim)
{
	// debug information about the node
    String str;
	Char buf[32];
    str += _T("TYPE = "); str += CR::typeName(type); str += delim;
    str += _T("ID   = "); str += id; str += delim;
    str += _T("NAME = "); str += name; str += delim;
    str += _T("PATH = "); str += fitem ? fitem->fpath : _T(""); str += delim;
	str += _T("BASE = "); str += incpath; str += delim;
    str += _T("NCLR = "); str += clrName(nclr); str += delim;
    str += _T("PAR = ");  str += parent ? parent->dispName() : _T("nullptr"); str += delim;
    str += _T("FLAGS = ");  str += clrName(flags); str += delim;
	swprintf(buf, L"%p", this);
	str += _T("PTR = "); str += buf; str += delim;
	swprintf(buf, L"%d", branchesCount());
	str += _T("BRANCHES = "); str += buf; str += delim;
	swprintf(buf, L"%p", gparam);
	str += _T("GPARAM = "); str += buf; str += delim;
//    str += "IDX = ";str += idx.isValid() ? idx.data(Qt::DisplayRole).toString() : "<invalid>";
    return str;
}

String CRNode::fullName()
{
    if(!parent)
        return _T("");//dispName(); -- for root without a name
    return parent->fullName() + _T(" / ") + dispName();
}

int CRNode::branchesCount()
{
    return branches.size();
}

CRNode* CRNode::branchByIndex(int i)
{
    if(i<0 || i>=branchesCount())
        return nullptr;
    NList::iterator it = branches.begin();
    std::advance(it, i);
    return *it;
}

int CRNode::branchIndex(CRNode* branch)
{
    NList::iterator it = std::find(branches.begin(), branches.end(), branch);
    if(it == branches.end())
        return -1;
    return std::distance(branches.begin(), it);
}

bool CRNode::hasLocalNodes()
{
    // is this node local?
    // it is one if it is itself local
    // or any of its child nodes is local
	// locality is the lack of own path

    if(CR::isLocal(type) && !fitem)//@ path.empty())
        return true;

	for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        if((*i)->hasLocalNodes())
            return true;
    }
    return false;
}

void CRNode::updateSubtreeVisibility(bool vis)
{
    // recursively process the visible flag according to the checked flags
    vis = vis && !!(flags & CRNode::NF_CHECKED);
    if(vis)
        flags |= CRNode::NF_VISIBLE;
    else
        flags &= ~CRNode::NF_VISIBLE;
    for(NList::iterator i = branches.begin(), e = branches.end(); i!=e; ++i) {
        CRNode *ch = (*i);
        ch->updateSubtreeVisibility(vis);
    }
}

