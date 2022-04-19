#include "crtree.h"
#include "projectloader.h"
#include "mcmt.h"
#include <tchar.h>

#include "../tools/tdefs.h"
#include "core.h"
#include "cruser.h"

// global CR object
CRTree *pCR;

void PrjProps::initDef()
{
    cmt = _T("//");
    area = _T("<<");
    aend = _T(">>");
    block = _T("@@");
    label = _T("$$");
}

CRTree::CRTree()
    : modify(false)
    , confirmedPath(false)
    , updateCount(1)
	, insMode(EIM_PARENT)
{
    root.type = CR::NT_ROOT;
    root.branches.push_back(&fbase);
    root.branches.push_back(&tags);
    root.branches.push_back(&outline);
    root.name = _T("ROOT");

    fbase.type = CR::NT_FBASE;
    fbase.parent = &root;
    fbase.name = _T("FILES");

    tags.type = CR::NT_TAGS;
    tags.parent = &root;
    tags.name = _T("TAGS");
    tags.flags |= CRNode::NF_ACTIVE|CRNode::NF_CURRENT;

    outline.type = CR::NT_OUTLINE;
    outline.parent = &root;
    outline.name = _T("OUTLINE");
    outline.flags |= CRNode::NF_ACTIVE|CRNode::NF_CURRENT;
}

CRTree::~CRTree()
{
    // break links so that there are no errors when automatically deleting the tree
    root.branches.clear();  // it is necessary!
    fbase.parent = nullptr;
    tags.parent = nullptr;
    outline.parent = nullptr;

    fbase.removeBranches();
    tags.removeBranches();
    outline.removeBranches();
}

void CRTree::setCurrentProject(const String &crpath, const String &basedir,  bool confirmed)
{
    crFPath = crpath;
	baseDir = basedir;
    confirmedPath = confirmed;
}

bool CRTree::createByDir(const String &basename, const String &basedir, const String &filters, unsigned int flags)
{
    // create by directory
    StrList flist = str_split(filters, _T(';'));
    createEmpty(basename, basedir);
    modify = true;
    //suggestedPath = dlg.m_base + "/" + dlg.m_name + AppFileExt;
    //@ setCurrentProject(basedir + _T("/") + basename + AppFileExtU, basedir, false); -- already exists in createEmpty
    return ProjectLoader::loadSrcDir(&fbase, basedir, flist, nullptr, flags&1);
}

bool CRTree::createByLst(const String &basename, const String &basedir, const StrList &lst, unsigned int flags)
{
    // create by file list
    createEmpty(basename, basedir); // there is already setCurrentProject
    modify = true;
    //QString suggestedPath =
    //d->startupProject->projectDirectory().toString() // f
    //+ "/" + d->startupProject->displayName()         // d
    //+ AppFileExt;
    //@ setCurrentProject(basepath, false); -- Because createEmpty already has
    return ProjectLoader::loadFileList(&fbase, basedir, lst, nullptr, flags&1);
}

bool CRTree::isEmpty()
{
	return 
		fbase.branchesCount()==0 &&
		tags.branchesCount()==0 &&
		outline.branchesCount()==0;
}

void CRTree::createEmpty(const String &basename, const String &basedir)
{
    // create empty
    root.name = basename;   // not used
    fbase.removeBranches();
    fbase.name = _T("FILES");
   
    tags.removeBranches();
    outline.removeBranches();

    user.clearRecent(); //? but is it necessary?
	fmgr.clear();

    setCurrentProject(basedir + _T("/") + basename + AppFileExtU, basedir, false);
    modify = false; // empty project not contain data...
}

bool CRTree::updateByDir(const String &filters, unsigned int flags)
{
    // update (regenerate) by filesystem
    StrList flist = str_split(filters, _T(';'));
    modify = true;
    return ProjectLoader::loadSrcDir(&fbase, baseDir, flist, &fbase, flags&1);
}

bool CRTree::updateByLst(const StrList &lst, unsigned int flags)
{
    // update (regenerate) by filelist
    modify = true;
    return ProjectLoader::loadFileList(&fbase, baseDir, lst, &fbase, flags&1);
}

bool CRTree::loadCR(const String &path)
{
	// download full solution
    if(path.empty())
        return false;

	// get bas dir for paths
	Path basedir = Path(path).parent_path();
	
	setCurrentProject(path, basedir.c_str(), false);

    pugi::xml_document doc;
    pugi::xml_node elemCR, elem;
    if(!doc.load_file(path.c_str()))
        return false;
    elem = doc.root(); // virtual root
    if(elem.empty())
        return false;
    elemCR = elem.first_child(); // true xml root ("coderainbow")
    if(elemCR.empty())
        return false;

    // clear project tree
    createEmpty(_T(""), _T(""));
	   
    
    // load tags
    elem = elemCR.child(CR::typeName(CR::NT_TAGS));
    if(!elem.empty())
        loadNode(elem, &tags, Path());

    // load notes/outline
    elem = elemCR.child(CR::typeName(CR::NT_OUTLINE));
    if(!elem.empty())
        loadNode(elem, &outline, basedir);

    // load filesystem
    elem = elemCR.child(CR::typeName(CR::NT_FBASE));
    if(!elem.empty())
        loadNode(elem, &fbase, basedir);

	root.flags |= CRNode::NF_CHECKED;
	root.updateSubtreeVisibility(true);

    tags.name = _T("TAGS");
    fbase.name = _T("FILES");
    outline.name = _T("OUTLINE");
    
    setCurrentProject(path, basedir.c_str(), true);
	root.incpath = path;
    modify = false;

	// load recent
	user.loadCRUser(path+_T(".user"));

    return true;
}

bool CRTree::loadNode(pugi::xml_node elem, CRNode *node, const Path &base)
{
    node->type = CR::typeValue(elem.name());
    //node->flags = decodeFlags(elem.attribute("text").value());
    if( CR::isFile(node->type) )
        node->setFlags(CRNode::NF_ACTIVE, 0);

//	if(node->type == CR::NT_DIR)
//		node->type = CR::typeValue(elem.name());

	String inc = base.empty() ? String() : pathToAbs(base, elem.attribute(_T("include")).value());
	if(inc.empty()) {
		// this is an internal node
		node->text = elem.attribute(_T("text")).value();
		node->name = elem.attribute(_T("name")).value();
		node->id   = elem.attribute(_T("id")).value();
		if(!base.empty())
			node->fitem = fmgr.regFileItem( pathToAbs(base, elem.attribute(_T("path")).value()) );
		node->nclr = rgbToVal(elem.attribute(_T("color")).value());
        node->tclr = rgbToVal(elem.attribute(_T("tcolor")).value());
		node->flags = hexToNum(elem.attribute(_T("flags")).value());
		
		// until I know how to deal with flags
		node->flags |= (CRNode::NF_VISIBLE|CRNode::NF_CHECKED);

		for (pugi::xml_node br = elem.first_child(); !br.empty(); br = br.next_sibling()) {
			CRNode *branch = CRNode::mkNode(node);
			loadNode(br, branch, base);
		}
	}
	else {
		// this is an external included file
		pugi::xml_document doc;
		pugi::xml_node elemCR, elem;
		if(!doc.load_file(pathToAbs(base, inc).c_str()))
			return false;
		elem = doc.root(); // virtual root
		if(elem.empty())
			return false;
		elemCR = elem.first_child(); // true xml root ("coderainbow")
		if(elemCR.empty())
			return false;
		elem = elemCR.first_child(); // first and single root node
		
		Path basedir = Path(inc).parent_path();
		node->incpath = inc;
		loadNode(elem, node, basedir);
	}
    return true;
}

bool CRTree::saveCR(const String &path)
{
	// save whole solution (root non-recursive function)
    if(path.empty())
        return false;
	user.saveCRUser(path+_T(".user"));

    pugi::xml_document doc;
    pugi::xml_node elem, elemCR = doc.append_child(_T("coderainbow"));
    //QDir basedir = QFileInfo(path).absoluteDir();
	Path basedir = Path(path).parent_path();
	
    elem = elemCR.append_child(CR::typeName(CR::NT_TAGS));
    saveNode(elem, &tags, Path());

    elem = elemCR.append_child(CR::typeName(CR::NT_OUTLINE));
    saveNode(elem, &outline, basedir);

    elem = elemCR.append_child(CR::typeName(CR::NT_FBASE));
    saveNode(elem, &fbase, basedir);

    if( doc.save_file(path.c_str()) ) {
        modify = false;
        setCurrentProject(path, basedir.c_str(), true);
        return true;
    }
    return false;
}



bool CRTree::saveNode(pugi::xml_node elem, CRNode *node, const Path &base)
{
	String inc = node->incpath;
	bool ok = true;
	if(inc.empty()) {
		// this is an internal node
		elem.append_attribute(_T("text")).set_value( node->text.c_str() );
		elem.append_attribute(_T("name")).set_value( node->name.c_str() );
		elem.append_attribute(_T("id"))  .set_value( node->id  .c_str() );
		elem.append_attribute(_T("path")).set_value( node->fitem ? pathToRel(base, node->fitem->fpath).c_str() : _T("") );
		elem.append_attribute(_T("color")).set_value( rgbToStr(node->nclr).c_str() );
        elem.append_attribute(_T("tcolor")).set_value( rgbToStr(node->tclr).c_str() );
		elem.append_attribute(_T("flags")).set_value( numToHex(node->flags).c_str() );

		CRNode::NList::iterator i = node->branches.begin(), e = node->branches.end();
		while(i!=e) {
			pugi::xml_node ch = elem.append_child( CR::typeName((*i)->type) );
			ok &= saveNode(ch, *i, base);
			++i;
		}
	}
	else {
		// this is an external included file
		// insert a relative reference to it
		elem.append_attribute(_T("include")).set_value( pathToRel(base, node->incpath).c_str() );

		// create a file
		pugi::xml_document doc;
		pugi::xml_node elemCR = doc.append_child(_T("coderainbow"));
		Path basedir = Path(node->incpath).parent_path();

		elem = elemCR.append_child( CR::typeName(node->type) );
		node->incpath.clear();
		ok &= saveNode(elem, node, basedir);
		node->incpath = inc;

		ok &= doc.save_file(inc.c_str());
	}
    return ok;
}

Char* CRTree::canAcceptMarkup(CRNode *fnode, bool toFiles, bool toTags, bool toOutline, const String &id, const String &ts)
{/*@
    // all data validation checks for the new markup
    // check if we are only creating MK without entries in the tree
    if (!toFiles && !toOutline && !toTags)
        return _T("No nodes will be created! Check 'Files', 'Tags' and/or 'Outline' checkboxes");
    // id and tags missing at the same time
    if(CR::isSig(m_markEdit->type)) {
        if(id.isEmpty()) {
            return _T("Signature is empty!");
        }
    }
    else {
        if(id.isEmpty() && ts.isEmpty()) {
            return _T("ID and Tags are empty!");
        }
    }

    // the identifier has changed and this is in the file
    if((m_markEdit->id != id.toStdWString()) && (fnode && fnode->findChildId(id.toStdWString()))) {
        QMessageBox::warning(this, AppName, tr("ID is already in use!"), QMessageBox::Ok);
        return;
    }

    // not a signature and invalid characters in the identifier
    if(!CR::isSig(m_markEdit->type) && (id.indexOf(' ')>=0 || id.indexOf('#')>=0 || id.indexOf('@')>=0
            || id.indexOf('$')>=0 || id.indexOf('>')>=0 || id.indexOf('<')>=0)) {
        return _T("ID contains illegal characters");
    }

    // not a signature and invalid characters in the tags
    if(!CR::isSig(m_markEdit->type) && (ts.indexOf(' ')>=0 || ts.indexOf('#')>=0 || ts.indexOf('@')>=0
            || ts.indexOf('$')>=0 || ts.indexOf('>')>=0 || ts.indexOf('<')>=0)) {
        return _T("Tags contains illegal characters");
    }
*/
    return nullptr;
}

CRNode* CRTree::addNode(CRNode *nodeActive, CR::Type type, const String &id, const String &name, FItem *fitem,
                        const String &text, unsigned int nclr)
{
    CRNode *node = CRNode::mkNode(nodeActive, type);
    node->id = id;
    node->name = name;
	node->fitem = fitem;
    node->text = text;
    node->nclr = nclr;
    modify = true;
    return node;
}

CRNode* CRTree::addNode(CRNode *nodeActive, CR::Type type, const String &id, const String &name, const String &path,
                        const String &text, unsigned int nclr)
{
    return addNode(nodeActive, type, id, name, fmgr.regFileItem(path), text, nclr);
}

CRNode* CRTree::addNode(CRNode *nodeActive, CR::Type type, const CRInfo &ni)
{
    return addNode(nodeActive, type, ni.id, ni.name, ni.fitem, ni.text, ni.nclr);
}

CRNode* CRTree::updateNode(CRNode *nodeActive, CR::Type type, const CRInfo &ni, int *res)
{
	CRNode *node = nodeActive->findChildId(ni.id);
	if(!node) {
		node = addNode(nodeActive, type, ni);
		if(res) 
			*res = node ? 2 : 0;
	}
	else {
		if(res)
			*res = 1;
	}
	return node;
}

bool CRTree::canRemoveNode(CRNode *node)
{
	if(!node || !node->parent)
        return false;
	if(CR::isSys(node->type))
        return false;
    CRNode *par = node->parent;
    if(!par->canRemoveBranch(node))
        return false;

    return true;
}

bool CRTree::removeNode(CRNode *node)
{
    if(!node || !node->parent)
        return false;
	if(CR::isSys(node->type))
        return false;
    CRNode *par = node->parent;
    if(!par->removeBranch(node))
        return false;

    modify = true;
    return true;
}

bool CRTree::moveNodeUp(CRNode* node)
{
    if(!node || !node->parent)
        return false;
	if(CR::isSys(node->type))
        return false;
    
    CRNode::NList::iterator i = std::find(node->parent->branches.begin(), node->parent->branches.end(), node);
    if(i==node->parent->branches.end())
        return false;
	// element must not be first
	if(i==node->parent->branches.begin())
		return false;

	CRNode::NList::iterator j = std::prev(i);
    node->parent->branches.erase(i);
    node->parent->branches.insert(j, node);
    modify = true;
    return true;
}

bool CRTree::moveNodeDown(CRNode* node)
{
    if(!node || !node->parent)
        return false;
	if(CR::isSys(node->type))
        return false;
    
    CRNode::NList::iterator i = std::find(node->parent->branches.begin(), node->parent->branches.end(), node);
    if(i==node->parent->branches.end())
        return false;
	// element must not be last
	if(i==std::prev(node->parent->branches.end()))
		return false;

    CRNode::NList::iterator j = std::next(i);
    node->parent->branches.erase(i);
    node->parent->branches.insert(j, node);
    modify = true;
    return true;
}

bool CRTree::moveNodeLeft(CRNode* node)
{
    if(!node || !node->parent)
        return false;
	if(CR::isSys(node->type))
        return false;
    if(CR::isSys(node->parent->type))
        return false;
    // if the node being moved or at least one of its children
    // depend on local root, we move outside of local root
    // then move is not allowed
    if(node->hasLocalNodes() && (node->parent==node->getPathNode()))
        return false;
//    if(CR::isName(node->type) && CR::isFile(node->parent->type))
//        return false;

    CRNode::NList::iterator i = std::find(node->parent->branches.begin(), node->parent->branches.end(), node);
    if(i==node->parent->branches.end())
        return false;

    CRNode *par = node->parent;

    node->parent->branches.erase(i);

    i = std::find(par->parent->branches.begin(), par->parent->branches.end(), par);
	++i;
	if(i != par->parent->branches.end())
		par->parent->branches.insert(i, node);
	else
		par->parent->branches.push_back(node);
    node->parent = par->parent;
    modify = true;
    return true;
}

bool CRTree::moveNodeRight(CRNode* node)
{
    if(!node || !node->parent)
        return false;
	if(CR::isSys(node->type))
        return false;
    
    CRNode::NList::iterator i = std::find(node->parent->branches.begin(), node->parent->branches.end(), node);
    if(i==node->parent->branches.end())
        return false;
	// we need the element to be not the first one, i.e. so that the element has a sibling in front of it
    if(i==node->parent->branches.begin())
        return false;

	CRNode::NList::iterator j = i; 
	--j;
    CRNode *par = *j;

    if(CR::isMcmt(node->type) && CR::isFile(par->type))
        return false;

    node->parent->branches.erase(i);
    par->branches.push_back(node);
    node->parent = par;
    modify = true;
    return true;
}

bool CRTree::moveNodeTo(CRNode *node, CRNode *npar)
{
	if (!node || !npar)
		return false;
    if(CR::isSys(node->type))
        return false;
    if(!node->parent)
        return false;
	CRNode::NList::iterator i = std::find(node->parent->branches.begin(), node->parent->branches.end(), node);
    if(i==node->parent->branches.end())
        return false;
    node->parent->branches.erase(i);
    npar->branches.push_back(node);
    node->parent = npar;
    modify = true;
    return true;
}

void CRTree::setText(CRNode* node, const String &text)
{
    node->text = text;
    modify = true;
}

void CRTree::setIncPath(CRNode* node, const Char *path)
{
	if(!path)
		node->incpath.clear();
	else
		node->incpath = path;
	modify = true;
}

CRNode* CRTree::getFileNode(const String &fpath)
{
	// look only for Files, and there may not be a file there. he's in Outline
	// i.e. applicable specifically for searching in Files
    return fbase.findFile(fpath);
}

CRNode* CRTree::addFileNode(const String &fpath, CRNode *par)
{
	boost::filesystem::path fi(fpath);
	return addNode(par ? par : &fbase, CR::NT_FILE, _T(""), fi.filename().wstring(), fpath, _T(""), CLR_NONE);
}

CRNode* CRTree::findTag(const StrList &taglist)
{
    return tags.findTag(taglist);
}

CRNode* CRTree::findOutlineElem(FItem *fitem, const String &id)
{
    return outline.findChildPathId(fitem, id);
}

CRNode* CRTree::getFNode(FItem *fitem)
{
	// find node by file element
	return fbase.walkNodes([&fitem](CRNode *node)->CRNode* {
        if(node->fitem == fitem)
            return node;
        return nullptr;
    });
}

CRNode* CRTree::getLocalRoot(CRNode *node)
{
    // local route to move; element cannot be moved outside the local root
    // usually one of the {base,tags,notes} elements
    // but in the event that the element does not have its own path, and is not required -
    // is the first parent element with a path
    // path required for metacomments and signatures, not tags
    // in addition, if there is AT LEAST ONE local element inside the grouping element without path -
    //- element becomes local

    // if there is a path - move within the system node
    if(node->fitem && !node->fitem->fpath.empty())
        return node->getSysNode();
    // if this is a tag tree - move within the system node
    if(node->getSysType() == CR::NT_TAGS)
        return node->getSysNode();
    // if the node is local or has other local nodes
    if(node->hasLocalNodes())
        return node->getPathNode();
    // otherwise return the system node
    return node->getSysNode();
}

CRNode *CRTree::getActive(CRNode *node)
{
	// find active node for given
	// get either a file or a system node
    if(!node || node->type == CR::NT_FBASE)
        return false;
	CRNode *root = node->getSysNode();
    if(root->type == CR::NT_FBASE)
        root = node->getPathNode();
	return root->findCurrActive();
}

bool CRTree::setActive(CRNode *node)
{
    // set node active; for a node in any section
    // get either a file or a system node
    if(!node || node->type == CR::NT_FBASE)
        return false;
    CRNode *root = node->getSysNode();
    if(root->type == CR::NT_FBASE)
        root = node->getPathNode();

    root->setSubtreeFlags(0, CRNode::NF_ACTIVE|CRNode::NF_CURRENT);
    node->setFlags(CRNode::NF_ACTIVE|CRNode::NF_CURRENT, 0);
	return true;
}

void CRTree::updateCurrents(CRNode *fnode)
{
//    qDebug() << "updateCurrents: " << fnode;
    // go through all the nodes of the FS, adjust the bits there
    // set NF_CURRENT to fnode or its children; in the rest we shoot

    // 1 turn off everything
    fbase.walkNodes([](CRNode *cnode) -> CRNode* {
        if(cnode->flags & CRNode::NF_ACTIVE) {
            cnode->setFlags(0, CRNode::NF_CURRENT);
        }
        return nullptr;
    });
    // 2 find the active one of the transferred file node and set it as current
    if(fnode) {
        fnode->walkNodes([](CRNode *cnode) -> CRNode* {
            if(cnode->flags & CRNode::NF_ACTIVE) {
                cnode->setFlags(CRNode::NF_CURRENT, 0);
                return cnode;
            }
            return nullptr;
        });
    }
}

void CRTree::convertBase(bool separate)
{
	// recursively go through the database and set all file elements incPath to match fpath

}
