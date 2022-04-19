// functions to export
#include "core/crtree.h"
#include "core/mark.h"
#include "core/parse.h"
#include "core/cruser.h"
#include "core/markmaker.h"
#include "core/mcmt.h"
#include "core/scan.h"

#include <stdio.h>
#include <stdarg.h>
#include <wtypes.h>
#include <oleauto.h>

CRTree theCR;
CRTree *ptr = pCR = &theCR;
MarkMaker theMaker;
wchar_t buf[256];

void Log(const Char *msg, ...)
{
	static bool first = true;
	FILE *f = fopen("d:/PRG/CR/crlog.txt", first ? "wt" : "at");
	va_list v;
	va_start(v, msg);
	
	vfwprintf(f, msg, v);
	fwprintf(f, _T("\n"));

	fclose(f);
	va_end(v);
	first = false;
}

enum Flags {
	FL_None = 0,
	FL_Files = 1,
    FL_Outline = 2,
    FL_Tags = 4,
    FL_GenId = 8,
	FL_Inline = 16,
	FL_Merge = 32,
};

#pragma pack (push, 1)
struct RecentInfo
{
	BSTR name;
	unsigned int clr;
	unsigned int flags;
};

struct MarkupInfo
{
	BSTR  id;
    BSTR  tags;
    BSTR  name;
    BSTR  text;
	BSTR  path;
	BSTR  incpath;	// base for includes
    unsigned int flags;
	unsigned int nclr;	// node
	unsigned int mclr;	// marker
	unsigned int tclr;	// tree
};

struct McmtInfo
{
	BSTR beg;
	BSTR end;
};

// based on CR::Mark
struct HighlightInfo 
{
	int pos;    // position from the beginning of the text
    int row;    // line number==block number
    int col;    // column number==character number
    int lines;  // number of lines or characters per line (depending on what kind of marker it is)
    int chars;  // number of bytes from pos to end
	Clr color;  // color
};


// callback for adding/modifying a node in the tree
typedef void (__stdcall * ModifyCallback)(CRNode *node, CRNode *active);

// callback to get the code if it's open in the editor; otherwise null
typedef int (__stdcall * GetSrcCallback)(BSTR fpath, FItem *fitem, unsigned int mode, BSTR str);

// callback to add the result to the output window
typedef void (__stdcall * AddMsgCallback)(BSTR fpath, int line, BSTR msg);     

#pragma pack(pop)

// wrapper functions for callbacks
struct MsxScanInfo {
	GetSrcCallback fnGetSrc;
	AddMsgCallback fnAddMsg;
};

void msx_modifyWrapper(void *arg, CRNode *node, CRNode *active)
{
	((ModifyCallback)arg)(node, active);
}

int msx_getSrcWrapper(void *arg, FItem* fitem, int mode, const String& str, const Char* fpath)
{
	MsxScanInfo *info = (MsxScanInfo*)arg;
	return info->fnGetSrc(::SysAllocString(fpath), fitem, mode, ::SysAllocString(str.c_str()));
}

void msx_addMsgWrapper(void *arg, const Char* fpath, int line, const Char* msg)
{
	MsxScanInfo *info = (MsxScanInfo*)arg;
	info->fnAddMsg(::SysAllocString(fpath), line, ::SysAllocString(msg));
}

// API

extern "C" __declspec(dllexport) void initCR()
{
	pCR = &theCR;
}

extern "C" __declspec(dllexport) int getStatus()
{
	int status = 0;
	if(pCR->modify)
		status |= 1;
	if(pCR->confirmedPath)
		status |= 2;
	if(pCR->isEmpty())
		status |= 4;
	return status;
}

extern "C" __declspec(dllexport) BSTR getRecentPath() 
{
	return ::SysAllocString(pCR->crFPath.c_str());	
}

extern "C" __declspec(dllexport) int loadCR(const wchar_t *fpath)
{
	return pCR->loadCR(fpath);
}

extern "C" __declspec(dllexport) int saveCR(const wchar_t *fpath)
{
	if(!fpath || (!fpath[0] && !pCR->confirmedPath))
		return 0;
	if(!fpath[0] && pCR->confirmedPath)
		return pCR->saveCR(pCR->crFPath);
	if(!fpath[0])
		return 0;
	return pCR->saveCR(fpath);
}

extern "C" __declspec(dllexport) void clearCR()
{
	pCR->createEmpty(_T(""), _T(""));
}

extern "C" __declspec(dllexport) CRNode* getRoot()
{
	return theCR.getRoot();
}

extern "C" __declspec(dllexport) CRNode* getFiles()
{
	return theCR.getFiles();
}

extern "C" __declspec(dllexport) CRNode* getTags()
{
	return theCR.getTags();
}

extern "C" __declspec(dllexport) CRNode* getOutline()
{
	return theCR.getOutline();
}

extern "C" __declspec(dllexport) int getChildCount(CRNode *root)
{
	if(!root)
		return 0;
	return root->branchesCount();
}

extern "C" __declspec(dllexport) CRNode* getChild(CRNode *root, int i)
{
	if(!root)
		return nullptr;
	return root->branchByIndex(i);
}

extern "C" __declspec(dllexport) CRNode* getPathNode(CRNode *node)
{
	if(!node)
		return nullptr;
	return node->getPathNode();
}

extern "C" __declspec(dllexport) BSTR getNodeBrief(CRNode *node) 
{
	if(!node)
		return nullptr;
	return ::SysAllocString(node->getInfo().c_str());
}

extern "C" __declspec(dllexport) BSTR getNodeDisp(CRNode *node) 
{
	if(!node)
		return nullptr;
	return ::SysAllocString(node->dispName().c_str());
}

extern "C" __declspec(dllexport) BSTR getNodeText(CRNode *node) 
{
	if(!node)
		return nullptr;
	return ::SysAllocString(node->text.c_str());
}

extern "C" __declspec(dllexport) BSTR getNodeFullName(CRNode *node) 
{
	if(!node)
		return nullptr;
	return ::SysAllocString(node->fullName().c_str());
}


extern "C" __declspec(dllexport) void setNodeText(CRNode *node, const wchar_t *text) 
{
	if(!node)
		return;
	pCR->setText(node, text);
}

extern "C" __declspec(dllexport) BSTR getNodeId(CRNode *node) 
{
	if(!node)
		return nullptr;
	return ::SysAllocString(node->id.c_str());
}

extern "C" __declspec(dllexport) BSTR getNodePath(CRNode *node) 
{
	if(!node)
		return nullptr;
	CRNode *fnode = node->getPathNode();
	if(!fnode || !node->fitem)
        return nullptr;
	return ::SysAllocString(fnode->fitem->fpath.c_str());
}

extern "C" __declspec(dllexport) BSTR getNodeDir(CRNode *node) 
{
	// working directory of the node; to the nearest path or incpath object
	// destination - to set the initial directory in the file selection dialog when adding a new file
	if(!node)
		return nullptr;
//	CRNode *fnode = node->getPathNode();	// return a node with a path;
//   if(!fnode)
//        return nullptr;
	String dir = node->getDir();
	return ::SysAllocString(dir.c_str());
}

extern "C" __declspec(dllexport) int getNodeType(CRNode *node) 
{
	if(!node || node->type < 0 || node->type >= CR::NT_ItemsCount)
		return 0;
	return node->type;
}

extern "C" __declspec(dllexport) int getNodeSysType(CRNode *node) 
{
	if(!node)
		return 0;
	return node->getSysType();
}

extern "C" __declspec(dllexport) BSTR getRecentItem(int ti, int ri, unsigned int *clr)
{
	*clr = pCR->user.markButtons[ti].items[ri].nclr;
	return ::SysAllocString(pCR->user.markButtons[ti].items[ri].mark.c_str());
}

extern "C" __declspec(dllexport) int canRemoveNode(CRNode *node) 
{
	return pCR->canRemoveNode(node);
}

extern "C" __declspec(dllexport) int removeNode(CRNode *node) 
{
	return pCR->removeNode(node);
}

extern "C" __declspec(dllexport) int setActiveNode(CRNode *node) 
{
	return pCR->setActive(node);
}

extern "C" __declspec(dllexport) CRNode* getActiveNode(CRNode *node) 
{
	return pCR->getActive(node);
}

extern "C" __declspec(dllexport) void setNodeParam(CRNode *node, void* param)
{
	node->setParam(param);
}

extern "C" __declspec(dllexport) void* getNodeParam(CRNode *node)
{
	return node->getParam();
}

extern "C" __declspec(dllexport) FItem* getFileItem(const wchar_t *fpath)
{
	return pCR->fmgr.getFileItem(fpath);
}

extern "C" __declspec(dllexport) CRNode* getParentNode(CRNode *node)
{
	if(!node)
		return nullptr;
	return node->parent;
}

extern "C" __declspec(dllexport) int getNodeInfo(CRNode *node, MarkupInfo *info)
{
	if(!node)
		return 0;
	info->id = ::SysAllocString(node->id.c_str());
	info->name = ::SysAllocString(node->name.c_str());
	info->text = ::SysAllocString(node->text.c_str());
	info->path = ::SysAllocString(node->fitem ? node->fitem->fpath.c_str() : _T(""));
	info->incpath = ::SysAllocString(node->incpath.c_str());
	info->nclr = node->nclr;
	info->flags = 0;
	if(node->flags & CRNode::NF_MERGETID)
		info->flags |= FL_Merge;
//	info->mclr = node->mclr;
	info->tclr = node->tclr;
	return 1;
}

extern "C" __declspec(dllexport) int setNodeInfo(CRNode *node, const MarkupInfo *info)
{
	if(!node)
		return 0;
	node->id = String(info->id, ::SysStringLen(info->id));
	node->name = String(info->name, ::SysStringLen(info->name));
	node->text = String(info->text, ::SysStringLen(info->text));
//	node->path = String(info->path, ::SysStringLen(info->path));
	node->nclr = info->nclr;
	if(info->flags & FL_Merge)
		node->flags |= CRNode::NF_MERGETID;
	else
		node->flags &= ~CRNode::NF_MERGETID;
	node->tclr = info->tclr;
	pCR->modify = true;
	return 1;
}

extern "C" __declspec(dllexport) int parseHighlight(const wchar_t *data, int size, FItem *fitem, int** ppMarkers)
{
	// get lists of blocks and words to highlight
	// ARGS: data - text
	//       size - text size in bytes
	//      fnode - file node (may be missing)
	//  ppMarkers - output array of markers
	// RETS: number of markers in the output array

    CR::MarkList bars, words;
    Parser<Utf16> parser(data, size, fitem);
    parser.parseMarkup(&bars, &words);
	int n = bars.size() + words.size();
	if(n <= 0)
		return -1;
		
	*ppMarkers = (int*)::CoTaskMemAlloc(sizeof(int) * n * 6);
	int j = 0, k = 0;
	for (CR::MarkList::iterator i = bars.begin(), e = bars.end(); i!=e; ++i)
	{
		if((i->node==nullptr) || (i->node->flags & CRNode::NF_VISIBLE)) {
			(*ppMarkers)[j++] = i->pos;
			(*ppMarkers)[j++] = i->row;
			(*ppMarkers)[j++] = i->col;
			(*ppMarkers)[j++] = i->lines;
			(*ppMarkers)[j++] = i->chars;
			(*ppMarkers)[j++] = i->color;
			k++;
		}
	}
	for (CR::MarkList::iterator i = words.begin(), e = words.end(); i!=e; ++i)
	{
		if((i->node==nullptr) || (i->node->flags & CRNode::NF_VISIBLE)) {
			(*ppMarkers)[j++] = i->pos;
			(*ppMarkers)[j++] = i->row;
			(*ppMarkers)[j++] = i->col;
			(*ppMarkers)[j++] = - i->lines;
			(*ppMarkers)[j++] = i->chars;
			(*ppMarkers)[j++] = i->color;
			k++;
		}
	}
	return k;//n;
}

extern "C" __declspec(dllexport) int getRecentInfo(int ti, int ri, RecentInfo *info)
{
	if(ti<0 || ti>=CRUser::BTN_COUNT || ri<0 || ri>=CRUser::CLR_COUNT)
		return 0;
	info->name = ::SysAllocString(pCR->user.markButtons[ti].items[ri].mark.c_str());
	info->clr = pCR->user.markButtons[ti].items[ri].nclr;
	info->flags = 0;
	if(pCR->user.markButtons[ti].items[ri].mk_fuid)
		info->flags |= FL_Files;
	if(pCR->user.markButtons[ti].items[ri].mk_elem)
		info->flags |= FL_Outline;
	if(pCR->user.markButtons[ti].items[ri].mk_tsig)
		info->flags |= FL_Tags;
	if(pCR->user.markButtons[ti].items[ri].gen_uid)
		info->flags |= FL_GenId;
	return 1;
}

extern "C" __declspec(dllexport) int setRecentInfo(int ti, int ri, const RecentInfo *info)
{
	if(ti<0 || ti>=CRUser::BTN_COUNT || ri<0 || ri>=CRUser::CLR_COUNT)
		return 0;

	//swprintf(buf, L"test_struct_03.m_bstr : [%S].\r\n", info->name);
	//OutputDebugString(buf);	

	pCR->user.markButtons[ti].items[ri].mark = String(info->name, ::SysStringLen(info->name)); // bstr!
	pCR->user.markButtons[ti].items[ri].nclr = info->clr;
	pCR->user.markButtons[ti].items[ri].mk_fuid = !!(info->flags & FL_Files);
	pCR->user.markButtons[ti].items[ri].mk_elem = !!(info->flags & FL_Outline);
	pCR->user.markButtons[ti].items[ri].mk_tsig = !!(info->flags & FL_Tags);
	pCR->user.markButtons[ti].items[ri].gen_uid = !!(info->flags & FL_GenId);
	return 1;
}




extern "C" __declspec(dllexport) int beginMakeMarker(const wchar_t *fpath, int ri, CR::Type t, const wchar_t *str, int index)
{
	return theMaker.beginMakeMarker(fpath, ri, t, str, index);
}

extern "C" __declspec(dllexport) void endMakeMarker(McmtInfo *info, ModifyCallback fnModify)
{
	theMaker.endMakeMarker(msx_modifyWrapper, fnModify);
	info->beg = ::SysAllocString(theMaker.m_beg.c_str());
	info->end = ::SysAllocString(theMaker.m_end.c_str());
}

extern "C" __declspec(dllexport) int beginEditMarker(const wchar_t *fpath, const wchar_t *str, CRNode *enode)
{
	return theMaker.beginEditMarker(fpath, str, enode);
}

extern "C" __declspec(dllexport) void endEditMarker(BSTR *str, ModifyCallback fnModify)
{
	theMaker.endEditMarker(msx_modifyWrapper, fnModify);
	*str = ::SysAllocString(theMaker.m_str.c_str());
}

extern "C" __declspec(dllexport) void getMarkerInfo(MarkupInfo *info)
{
	// collect information about the created or edited marker into a structure
	info->id   = ::SysAllocString(theMaker.m_mi.id.c_str());
	info->tags = ::SysAllocString(theMaker.m_mi.tagStr().c_str());
	info->name = ::SysAllocString(theMaker.m_ni.name.c_str());
	info->text = ::SysAllocString(theMaker.m_ni.text.c_str());

	info->flags = 0;
	if(theMaker.m_mi.mk_fuid)
		info->flags |= FL_Files;
	if(theMaker.m_mi.mk_elem)
		info->flags |= FL_Outline;
	if(theMaker.m_mi.mk_tsig)
		info->flags |= FL_Tags;
	if(theMaker.m_mi.gen_uid)
		info->flags |= FL_GenId;
	if(theMaker.m_ni.flags & CRInfo::NF_MERGETID)
		info->flags |= FL_Merge;

	if(clrValid(theMaker.m_mi.mclr))
		info->flags |= FL_Inline;

	info->mclr = theMaker.m_mi.mclr;
	info->nclr = theMaker.m_ni.nclr;
}

extern "C" __declspec(dllexport) void setMarkerInfo(const MarkupInfo *info)
{
	// extract information about the created or edited marker from the structure
	theMaker.m_mi.id   = String(info->id, ::SysStringLen(info->id));
	theMaker.m_mi.setTags(String(info->tags, ::SysStringLen(info->tags)));
	theMaker.m_ni.name = String(info->name, ::SysStringLen(info->name));
	theMaker.m_ni.text = String(info->text, ::SysStringLen(info->text));

	theMaker.m_ni.id = theMaker.m_mi.id;

	theMaker.m_mi.mk_fuid = !!(info->flags & FL_Files);
	theMaker.m_mi.mk_elem = !!(info->flags & FL_Outline); 
	theMaker.m_mi.mk_tsig = !!(info->flags & FL_Tags);
	theMaker.m_mi.gen_uid = !!(info->flags & FL_GenId);

	if(info->flags & FL_Merge)
		theMaker.m_ni.flags |= CRInfo::NF_MERGETID;
	else
		theMaker.m_ni.flags &= ~CRInfo::NF_MERGETID;
	
	theMaker.m_mi.mclr = info->mclr;
	theMaker.m_ni.nclr = info->nclr;
}

extern "C" __declspec(dllexport) int moveNodeUp(CRNode *node)
{
	return pCR->moveNodeUp(node);
}

extern "C" __declspec(dllexport) int moveNodeDown(CRNode *node)
{
	return pCR->moveNodeDown(node);
}

extern "C" __declspec(dllexport) int moveNodeLeft(CRNode *node)
{
	return pCR->moveNodeLeft(node);
}

extern "C" __declspec(dllexport) int moveNodeRight(CRNode *node)
{
	return pCR->moveNodeRight(node);
}

extern "C" __declspec(dllexport) int moveNodeTo(CRNode *node, CRNode *npar)
{
	return pCR->moveNodeTo(node, npar);
}

extern "C" __declspec(dllexport) CRNode* addNode(CRNode *nodeActive, int type, const MarkupInfo *info)
{
	CRInfo crinfo;
//	Log("addNode: info = %X, pCR = %X", info, pCR);
//	Log("addNode: info->id = %X", info->id);

	crinfo.id   = String(info->id,   ::SysStringLen(info->id));
	crinfo.name = String(info->name, ::SysStringLen(info->name));
	crinfo.text = String(info->text, ::SysStringLen(info->text));
//	crinfo.path = p String(info->path, ::SysStringLen(info->path));
	crinfo.fitem = pCR->fmgr.regFileItem(String(info->path, ::SysStringLen(info->path)));
	crinfo.nclr = info->nclr;
	crinfo.tclr = info->tclr;
	return pCR->addNode(nodeActive, (CR::Type)type, crinfo);
}

extern "C" __declspec(dllexport) void checkNode(CRNode *node, int check)
{
	if(node) {
		if(check)
			node->flags |= CRNode::NF_CHECKED;
		else
			node->flags &= ~CRNode::NF_CHECKED;
		node->updateSubtreeVisibility(check);
	}
}

extern "C" __declspec(dllexport) int updateNode(CRNode *par, int type, const wchar_t *id, 
            const wchar_t *path, CRNode **node)
{
	// update the node; used to update or create a base on vcxproj/sln project files
	// RETS: 0=error,1=update,2=add
	if(!par || !node) 
		return 0;
	CRInfo ni;
	ni.id = id;
	ni.fitem = pCR->fmgr.regFileItem( path );
	int res = 0;
	*node = pCR->updateNode(par, (CR::Type)type, ni, &res);
	return res;
}

extern "C" __declspec(dllexport) int createByDir(const wchar_t *name, 
            const wchar_t *path, const wchar_t *filters, unsigned int flags)
{
	return pCR->createByDir(name, path, filters, flags);
}

extern "C" __declspec(dllexport) int updateByDir(const wchar_t *filters, unsigned int flags)
{
	return pCR->updateByDir(filters, flags);
}

extern "C" __declspec(dllexport) int insertByPath(const wchar_t *path)
{
	StrList lst;
	lst.push_back(path);
	return pCR->updateByLst(lst, 0);
}

// general scanning function
extern "C" __declspec(dllexport) void scanRoot(CRNode *root, unsigned int mode, const wchar_t *str, GetSrcCallback fnGetSrc, AddMsgCallback fnAddMsg)
{
	MsxScanInfo info;
	info.fnGetSrc = fnGetSrc;
	info.fnAddMsg = fnAddMsg;
	scanCR<Utf16>(root, str, mode, msx_getSrcWrapper, msx_addMsgWrapper, &info);
}

// scan function for open file
extern "C" __declspec(dllexport) void scanSrc(FItem *fitem, unsigned int mode, const wchar_t *str, const wchar_t *text, int size, AddMsgCallback fnAddMsg)
{
	MsxScanInfo info;
	info.fnGetSrc = nullptr;
	info.fnAddMsg = fnAddMsg;
	scanBuf<Utf16>(fitem, mode, str, text, size, msx_addMsgWrapper, &info);
}

// scanning function to search for a node at the cursor
extern "C" __declspec(dllexport) CRNode* findNodeByCursor(FItem* fitem, const wchar_t *text, int size, int row, int col)
{
	Parser<Utf16> parser(text, size, fitem);
    CRNode *node = parser.parseFindNode(row, col);
	if(!node && fitem) 
		return fitem->fnode;
	return node;
}

// scanning function to search for a node at the cursor
extern "C" __declspec(dllexport) CRNode* findNodeByCpos(FItem* fitem, const wchar_t *text, int size, int pos)
{
	Parser<Utf16> parser(text, size, fitem);
    CRNode *node = parser.parseFindNode(pos);
	return node;
}

// advanced scanning function to search for a node by the cursor
extern "C" __declspec(dllexport) CRNode* findNodeInfoByCursor(FItem* fitem, const wchar_t *text, int size, int row, int col, HighlightInfo *info)
{
	Parser<Utf16> parser(text, size, fitem);
	CR::Mark m;
    CRNode *node = parser.parseFindNode(row, col, &m);
	if(node && info) {
		info->chars = m.chars;
		info->col = m.col;
		info->color = m.color;
		info->lines = m.lines;
		info->pos = m.pos;
		info->row = m.row;
	}
	return node;
}

// signature invalidation - internal counter increment by which the current lists of signatures are regenerated
extern "C" __declspec(dllexport) void invalidateSignatures()
{
	pCR->invalidateSignatures();
}

// return a regular expression to search for MK in the text
extern "C" __declspec(dllexport) BSTR makeRegExp(CRNode *node)
{
	String rexp;
	if(!node)
		return nullptr;
	if(CR::isSig(node->type))
		rexp = makeSigRegExp(node->id);
	else
		rexp = makeMarkRegExp(node->id, node->type);
	return ::SysAllocString(rexp.c_str());
}

extern "C" __declspec(dllexport) void convertNode(CRNode *node, const wchar_t *path)
{
	if(!node)
		return;
	pCR->setIncPath(node, path);
}

extern "C" __declspec(dllexport) void convertBase(unsigned int flags)
{
	// flags: 0x1 - incPath
	pCR->convertBase(flags&1);
//	pCR->setIncPath(node, path);
}

extern "C" __declspec(dllexport) void setNodeTColor(CRNode *node, unsigned int tclr)
{
	if(!node)
		return;
	node->tclr = tclr;
	if(tclr != CLR_NONE)
		pCR->user.clrRecentTree = tclr;
	pCR->modify = true;
}

extern "C" __declspec(dllexport) unsigned int getNodeTColor(CRNode *node)
{
	if(!node)
		return pCR->user.clrRecentTree;
	return node->tclr;
}

extern "C" __declspec(dllexport) void setActiveNodeMode(int mode)
{
	pCR->insMode = (EInsertMode)mode;
}
