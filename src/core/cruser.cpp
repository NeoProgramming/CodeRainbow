#include "cruser.h"
#include "crnode.h"
#include "mcmt.h"

extern void Log(const Char *msg, ...);

void RecentItem::clear()
{
	mark = _T("");
	nclr = CLR_NONE;// QColor();
	mk_fuid = false;
	mk_elem = false;
	mk_tsig = false;
	gen_uid = false;
}

void RecentItem::fromMark(CRMark *mi, Clr clr, bool mcmt)
{
	// unless it's a metacomment
	if(mcmt)
		mark = makeMarkStr(*mi);
	else
		mark = mi->id;
	nclr = clr;
	mk_fuid = mi->mk_fuid;
	mk_elem = mi->mk_elem;
	mk_tsig = mi->mk_tsig;
	gen_uid = mi->gen_uid;
}

void RecentItem::toMark(CRMark *mi, Clr *clr, bool mcmt)
{
	if(mcmt)
		parseMarkStr(mark, 0, *mi);
	else
		mi->id = mark;
	*clr = nclr;
	mi->mk_fuid = mk_fuid;
	mi->mk_elem = mk_elem;
	mi->mk_tsig = mk_tsig;
	mi->gen_uid = gen_uid;
}

void CRUser::init(RecentCtl *ctl)
{
    // initialize built-in gui parts with callback interface
    for(int bi=0; bi<BTN_COUNT; bi++) {
        markButtons[bi].lsGui = ctl->createListGui();
        for(int ri=0; ri<RecentList::RECENT_COUNT; ri++) {
            markButtons[bi].items[ri].itGui = ctl->createItemGui();
        }
    }
}

void CRUser::free(RecentCtl *ctl)
{
	// initialize built-in gui parts with callback interface
	for (int bi = 0; bi<BTN_COUNT; bi++) {
		for (int ri = 0; ri<RecentList::RECENT_COUNT; ri++) {
			ctl->destroyItemGui(markButtons[bi].items[ri].itGui);
			markButtons[bi].items[ri].itGui = nullptr;
		}
		ctl->destroyListGui(markButtons[bi].lsGui);
		markButtons[bi].lsGui = nullptr;
	}
}

bool CRUser::loadCRUser(const String &path)
{
	pugi::xml_document doc;
    pugi::xml_node elemCR, elem;
    if(!doc.load_file(path.c_str()))
        return false;
	elem = doc.root(); // virtual root
    if(elem.empty())
        return false;
    elemCR = elem.first_child(); // true xml root ("coderainbow_user")
    if(elemCR.empty())
        return false;

	// node for recent list
    elem = elemCR.child(_T("recent_items"));
    if(!elem.empty())
        loadRecentItems(elem);

    // node for colors
    elem = elemCR.child(_T("recent_colors"));
    if(!elem.empty())
        loadRecentColors(elem);
    elem = elemCR.child(_T("predef_colors"));
    if(!elem.empty())
        loadPredefColors(elem);

	// recent reee clr
	elem = elemCR.child(_T("recent_globals"));
	if(!elem.empty())
		loadRecentGlobals(elem);
	
	return true;
}

bool CRUser::saveCRUser(const String &path)
{
	if(path.empty())
        return false;
    pugi::xml_document doc;
    pugi::xml_node node, nodeCR = doc.append_child(_T("coderainbow_user"));
	
	node = nodeCR.append_child(_T("recent_items"));
    saveRecentItems(node);
    node = nodeCR.append_child(_T("recent_colors"));
    saveRecentColors(node);
    node = nodeCR.append_child(_T("predef_colors"));
    savePredefColors(node);
	node = nodeCR.append_child(_T("recent_globals"));
    saveRecentGlobals(node);

	return doc.save_file(path.c_str());
}

void CRUser::loadRecentGlobals(pugi::xml_node par)
{
	pugi::xml_node elem = par.child(_T("recent_tree_clr"));
	clrRecentTree = rgbToVal(elem.attribute(_T("color")).value());
	
}

void CRUser::loadRecentItems(pugi::xml_node elem)
{
    for(int i=0; i<BTN_COUNT; i++)
        loadRecentList(elem, Typ(i));
}

void CRUser::loadRecentList(pugi::xml_node par, CR::Type t)
{
    pugi::xml_node elem = par.child(CR::typeName(t));
    RecentList &rec = markButtons[Idx(t)];
    int i = 0;

    for(i=0; i<RecentList::RECENT_COUNT; i++)
        rec.items[i].mark.clear();
    if(elem.empty())
        return;

    i = 0;
    for (pugi::xml_node br = elem.first_child(); !br.empty() && i<RecentList::RECENT_COUNT; br = br.next_sibling(), i++) {
        rec.items[i].mark = br.attribute(_T("mark")).value();
        rec.items[i].nclr = rgbToVal(br.attribute(_T("color")).value());
        rec.items[i].mk_fuid = br.attribute(_T("mk_fuid")).as_bool();
        rec.items[i].mk_elem = br.attribute(_T("mk_elem")).as_bool();
        rec.items[i].mk_tsig = br.attribute(_T("mk_tsig")).as_bool();
		rec.items[i].gen_uid = br.attribute(_T("gen_uid")).as_bool();
    }
}


void CRUser::saveRecentItems(pugi::xml_node elem)
{
    for(int i=0; i<BTN_COUNT; i++)
        saveRecentList(elem, Typ(i));
}

void CRUser::saveRecentList(pugi::xml_node par, CR::Type t)
{
    pugi::xml_node elem = par.append_child(CR::typeName(t));
    if(elem.empty())
        return;
    RecentList &rec = markButtons[Idx(t)];
    for(int i=0; i<RecentList::RECENT_COUNT; i++) {
        RecentItem &d = rec.items[i];
        if(!d.mark.empty()) {
            pugi::xml_node ch = elem.append_child( _T("r") );
            ch.append_attribute(_T("mark")).set_value( d.mark.c_str() );
            ch.append_attribute(_T("color")).set_value( rgbToStr(d.nclr).c_str() );

            ch.append_attribute(_T("mk_fuid")).set_value(d.mk_fuid);
            ch.append_attribute(_T("mk_elem")).set_value(d.mk_elem);
            ch.append_attribute(_T("mk_tsig")).set_value(d.mk_tsig);
			ch.append_attribute(_T("gen_uid")).set_value(d.gen_uid);
        }
    }
}

void CRUser::clearRecent()
{
    for(int i=0; i<BTN_COUNT; i++)
        clearRecentList(Typ(i));
}

void CRUser::clearRecentList(CR::Type t)
{
    RecentList &rec = markButtons[Idx(t)];
    for(int i=0; i<RecentList::RECENT_COUNT; i++) {
        rec.items[i].clear();
    }
}

void CRUser::updateRecentColors(Clr clr, const String &name)
{
    // wrapping in the last color list
    if(clrRecent[0].clr != clr) {
        for(int i=CLR_COUNT-1; i>0; i--) {
            clrRecent[i] = clrRecent[i-1];
        }
        clrRecent[0].clr = clr;
        clrRecent[0].name = name;
    }
}

void CRUser::addToRecentList(int ti, CRMark *mi, Clr nclr)
{
    // add new markup to recent sizes list
    for(int i=RecentList::RECENT_COUNT-1; i>0; i--) {
        // transfer of not all item'a, tk. action doesn't need to be wrapped!
        markButtons[ti].items[i] = markButtons[ti].items[i-1];
    }
    RecentItem &d = markButtons[ti].items[0];
    d.nclr = nclr;
	d.fromMark(mi, nclr, CR::isMcmt(Typ(ti)));

    updateRecentColors(nclr, d.mark);
}

void CRUser::raiseInRecentList(int ti, int ri, CRMark *mi, Clr nclr)
{
    // raise an existing markup to the first position in the list of recent markups
    RecentItem d = markButtons[ti].items[ri];
    for(int i=ri; i>0; i--) {
        // transfer of not all item'a, tk. action doesn't need to be wrapped!
        markButtons[ti].items[i] = markButtons[ti].items[i-1];
    }
	d.fromMark(mi, nclr, CR::isMcmt(Typ(ti)));
	markButtons[ti].items[0] = d;

    updateRecentColors(nclr, d.mark);
}

bool CRUser::getFromRecentList(int ti, int ri, CRMark *mi, Clr *nclr)
{
    if(ri<0 || ti<0)
        return false;
    RecentItem &d = markButtons[ti].items[ri];

    if(d.mark.empty())
        return false;
	d.toMark(mi, nclr, CR::isMcmt(Typ(ti)));
    return true;
}

void CRUser::loadRecentColors(pugi::xml_node elem)
{
    loadColors(elem, clrRecent);
}

void CRUser::loadPredefColors(pugi::xml_node elem)
{
    loadColors(elem, clrPredef);
}

void CRUser::loadColors(pugi::xml_node elem, ColorItem *arr)
{
    int i=0;
    for (pugi::xml_node br = elem.first_child(); !br.empty() && i<CLR_COUNT; br = br.next_sibling(), i++) {
        arr[i].name = br.attribute(_T("name")).value();
        arr[i].clr = rgbToVal(br.attribute(_T("color")).value());
    }
}

void CRUser::saveRecentColors(pugi::xml_node elem)
{
    saveColors(elem, clrRecent);
}

void CRUser::savePredefColors(pugi::xml_node elem)
{
    saveColors(elem, clrPredef);
}

void CRUser::saveColors(pugi::xml_node elem, ColorItem *arr)
{
    for(int i=0; i<RecentList::RECENT_COUNT; i++) {
        pugi::xml_node ch = elem.append_child( _T("r") );
        ch.append_attribute(_T("name")).set_value( arr[i].name.c_str() );
        ch.append_attribute(_T("color")).set_value( rgbToStr(arr[i].clr).c_str() );
    }
}

void CRUser::saveRecentGlobals(pugi::xml_node par)
{
	pugi::xml_node elem = par.append_child( _T("recent_tree_clr") );
	elem.append_attribute(_T("color")).set_value( rgbToStr(clrRecentTree).c_str() );
}
