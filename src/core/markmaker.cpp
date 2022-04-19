#include "markmaker.h"
#include <boost/algorithm/string.hpp>
#include "crtree.h"
#include "mcmt.h"
#include "nlexer.h"


void MarkMaker::clear()
{
	m_fnModify = nullptr;
	m_pArg = nullptr;
    m_mi.clear();
    m_ni.clear();
	m_head.clear();
	m_tail.clear();
    m_nodeFuid = nullptr;
	m_nodeNote = nullptr;
    m_ti = -1;
	m_ri = -1;
    m_nodeFile = nullptr;
	m_str.clear();
	m_beg.clear();
	m_end.clear();
//	m_fitem = nullptr;
}

void MarkMaker::createFuid(CRInfo &ni, CR::Type type)
{
    // create a unique named node
    if(m_nodeFile && !ni.id.empty()) {
        CRNode *active = m_nodeFile->getCurrActive();
        if(!active)
            active = m_nodeFile;
        CRNode *node = pCR->addNode(active, type, ni);
        treeItemModified(active, node);   // for gui/cr_editor
    }
}

void MarkMaker::createTags(CRMark &mi, Clr clr)
{
    // update tag cloud based on tag list (for meta comments)
    for(StrList::iterator i=mi.tags.begin(), e=mi.tags.end(); i!=e; ++i) {
        createTag(*i, clr);
    }
}

void MarkMaker::createTag(const String &tag, Clr clr)
{
    // update the tag cloud for a single tag (for signatures and iterating over the list of tags for meta comments)
    if(!tag.empty() && !pCR->getTags()->findChildId(tag)) {
//        qDebug() << "createTag: " << clr << hex;
        CRNode *node = pCR->addNode(pCR->getTags()->getCurrActive(), CR::NT_TAG, tag, _T(""), _T(""), _T(""), clr);
        treeItemModified(pCR->getTags(), node);
    }
}

void MarkMaker::createElem(CRInfo &ni, CR::Type type)
{
    // create a non-unique node in outline
    if(!ni.id.empty()) {
        CRNode *active = pCR->getOutline()->getCurrActive();
        CRNode *node = pCR->addNode(active, type, ni);
		node->fitem = pCR->fmgr.regFileItem( m_fpath );	// inconvenient, it's better to pass fitem through interop
        treeItemModified(active, node);   // for cr_editor
    }
}

void MarkMaker::createTsig(CRInfo &ni, CR::Type type)
{
    // create signature tag
    CRNode *node = pCR->addNode(
		pCR->getTags()->getCurrActive(),	// active node - 
		type, 
		ni.id, 
		_T(""), 
		_T(""), 
		_T(""), 
		ni.nclr
		);
    treeItemModified(pCR->getTags(), node);
}

void MarkMaker::updateFuid(CRInfo &ni)
{
	m_nodeFuid->setData(m_ni);
	treeItemModified(nullptr, m_nodeFuid);
}

void MarkMaker::updateElem(CRInfo &ni)
{
	m_nodeNote->setData(m_ni);
	treeItemModified(nullptr, m_nodeNote);
}

void MarkMaker::create(CRMark &mi, CRInfo &ni)
{
    // create nodes (in the tree and, if necessary, in the code) based on the data in mi
//  qDebug() << "create: mi=" << mi << " ni=" << ni;

    if(!mi.mk_fuid && !mi.mk_elem)
        mi.id.clear();

    // in the tree - the need is determined internally
    // for the source 'id', by checkboxes, we determine in which sections of the tree to create
    if(mi.mk_fuid && !ni.id.empty())
        createFuid(ni, mi.type);
    if(mi.mk_elem && !ni.id.empty())
        createElem(ni, mi.type);
    if(mi.mk_tsig && CR::isSig(mi.type))
        createTsig(ni, mi.type);

    // tags - in fact a non-empty list
    createTags(mi, ni.nclr);

    // in code - if the node requires markup with a metacomment
    if(CR::isMark(mi.type)) {
        createMarkers(mi, m_beg, m_end);
    }
}


int MarkMaker::beginMakeMarker(const String &fpath, int ri, CR::Type type, const String &str, int index)
{
    // create a new MetaComment based on the recent list or from scratch in the dialog
    // ARGS: ri: -1 = new, 0 .. n = recent
    //     type: type of node being created
    //      str: line in the code at the cursor
    //    fnDlg: call dialog function
	// RETS: 0: do nothing, something is wrong
	//       1: no dialog required, create with default options
	//       2: dialog required
	

	clear();

    // use this variable
    m_str = str;
	boost::trim(m_str);
    // information about the node
    m_fpath = fpath;
    m_nodeFile = pCR->getFileNode(fpath);   // trying to find, but there may not be a node
    m_ri = ri;
    m_ti = CRUser::Idx(type);

    // if recent list item is set
    if(ri>=0) {
        // trying to get data (name, id, color) from the list of recent names
        if(!pCR->user.getFromRecentList(m_ti, m_ri, &m_mi, &m_ni.nclr))
            ri=-1; // the element is not in the list, we call the dialog
    }

    // set the type
    m_mi.type = type;

	// if the identifier is set explicitly (signature) - extract it; signatures MAY be repeated!
    // else retrieve the suggested name based on the id under the cursor
    if(type == CR::NT_NAME) {   // signature-name
        m_mi.id = extractName(m_str, index);
        // if such a signature already exists, we just do nothing (protection against re-adding the same signature)
        if(m_nodeFile && m_nodeFile->findChildId(m_mi.id))
            return 0;
    }
    else if(CR::isSig(type)) {  // other signatures
        m_mi.id = extractSignature(m_str, index);
        // if such a signature already exists, we just do nothing (protection against re-adding the same signature)
        if(m_nodeFile && m_nodeFile->findChildId(m_mi.id))
            return 0;
    }
    else {
        // suggested name based on string
        m_mi.id = extractSuggestedId(m_str, index);
    }

    // make sure the name is unique -- Only if there is a file node
    if(m_nodeFile && CR::isMcmt(type))
        ensureUniqueId(m_nodeFile, m_mi.id);

    // transfer the identifier from the markup structure to the node structure
    m_ni.id = m_mi.id;

    // if dialog is required
    if(ri<0 || !m_mi.gen_uid) {
        return 2;
       // if(!fnDlg(&mi, &ni))
       //     return false;
    }
    return 1;
}

void MarkMaker::endMakeMarker(FnModifyTree fnModify, void *arg)
{
	m_fnModify = fnModify;
	m_pArg = arg;
	// create nodes in the tree
    create(m_mi, m_ni);
    // update recent list
    if(m_ri>=0)
        pCR->user.raiseInRecentList(m_ti, m_ri, &m_mi, m_ni.nclr);
    else
        pCR->user.addToRecentList(m_ti, &m_mi, m_ni.nclr);
    //return true;
}

int MarkMaker::beginEditMarker(const String &fpath, const String &str, CRNode *enode)
{
    // edit metacomment (i.e. useless for signatures);
    // parse the existing node, put it into the edit dialog,
    // according to the results of editing, correct the node
	// RETS: 0 - something is wrong, editing is impossible
    //       1 - ok, no dialog needed
    //       2 - ok, need dialog

	clear();

    m_fpath = fpath;
    

    CR::Type t, st;
	if(enode) {
		t = enode->type;
		st = enode->getSysType();
	}
	else {
		t = parseLine(str, m_mi, &m_head, &m_tail);
		st = CR::NT_NONE;
	}
    if(t == CR::NT_NONE || t == CR::NT_AEND)
        return 0;
	
	// file node; but maybe not!
	m_nodeFile = pCR->getFileNode(fpath);
	// try to find nodes that match the unique name from MK
    // if found, pass it, otherwise empty CRInfo (we can't pass nothing!)
    m_nodeFuid = (st==CR::NT_FBASE) ? enode : (m_nodeFile ? m_nodeFile->findChildId(m_mi.id) : nullptr);
	m_nodeNote = (st==CR::NT_OUTLINE) ? enode : pCR->findOutlineElem(pCR->fmgr.getFileItem(m_fpath), m_mi.id);

    m_mi.mk_fuid = (m_nodeFuid!=nullptr);
    m_mi.mk_elem = (m_nodeNote!=nullptr);
    m_mi.mk_tsig = !m_mi.tags.empty();
	
    CRNode *node;
    if(m_nodeFuid)
        node = m_nodeFuid;
    else if(m_nodeNote)
        node = m_nodeNote;
    else
		node = (st==CR::NT_TAGS) ? enode : pCR->findTag(m_mi.tags);

	if (node) {
        m_ni = *node;
		m_mi.id = m_ni.id;
		m_mi.type = t;
//		m_mi.merge_tid = !!(node->flags & CRNode::NF_MERGETID);
	}

    // dialog is called in ANY CASE, because This is editing, not creating.
    return 2;
//    if(!fnDlg(&mi, &ni))
//        return false;
}

void MarkMaker::endEditMarker(FnModifyTree fnModify, void *arg)
{
	m_fnModify = fnModify;
	m_pArg = arg;
    // tree update -- wrong
    if (m_mi.mk_fuid) {
        if (!m_nodeFuid)
            createFuid(m_ni, m_mi.type);
        else
			updateFuid(m_ni);            
    }
    if (m_mi.mk_elem) {
        if (!m_nodeNote)
            createElem(m_ni, m_mi.type);
        else
			updateElem(m_ni);
    }
    createTags(m_mi, m_ni.nclr);

    // adjust the meta comment in the code
    m_str = makeLine(m_mi, m_head, m_tail);
}

void MarkMaker::treeItemModified(CRNode *act, CRNode *node)
{
    // emit treeItemModified, now via callback
	if(m_fnModify)
		m_fnModify(m_pArg, node, act);
}


String MarkMaker::extractSignature(String &str, int index)
{
    // extract signature - or selected part or string
    str = removeComments(str);
    std::replace(str.begin(), str.end(), (Char)0x2029, _T(' '));
    std::replace(str.begin(), str.end(), _T('\r'), _T(' '));
    std::replace(str.begin(), str.end(), _T('\n'), _T(' '));
    return str;
}

String MarkMaker::extractName(String &str, int index)
{
    // extract NAME: selection or word; used for signature-name (symbol)
    // highlight the word
    extractId(str, index);
//  qDebug() << "str == "<<str;
    return str;
}

String MarkMaker::extractSuggestedId(String &str, int index)
{
    // extract the proposed name - the identifier on which the cursor is located
//  qDebug() << "str == "<<str;
    // trying to extract the identifier; if not, just replace all non-id characters with underscores
	if(!extractId(str, index))
        replaceNonId(str);
	// return the string, but it is also modified by reference
    return str;
}

