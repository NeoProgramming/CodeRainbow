#pragma once

#include "../pugixml/pugixml.hpp"
#include "../tools/tdefs.h"
#include "crnode.h"

struct RecentItemGui {};
struct RecentListGui {};

struct RecentCtl {
    virtual RecentItemGui *createItemGui() = 0;
    virtual RecentListGui *createListGui() = 0;

	virtual void destroyItemGui(RecentItemGui *p) = 0;
	virtual void destroyListGui(RecentListGui *p) = 0;
};

struct ColorItem {
    Clr clr;
    String name;

    ColorItem()
    {
        clr = CLR_NONE;
    }
};

struct RecentItem {
    RecentItem() : itGui(nullptr)
    {
        clear();
    }

    String mark;	// id + tags + inlineclr
    Clr nclr;		// nodeclr // QColor
    bool mk_fuid;
    bool mk_elem;
    bool mk_tsig;
	bool gen_uid;
    // the case when the Go embed concept would come in handy
    RecentItemGui *itGui;

	void clear();
	void fromMark(CRMark *mi, Clr nclr, bool mcmt);
	void toMark(CRMark *mi, Clr *nclr, bool mcmt);
};

// one mark button with recent list
struct RecentList {

    RecentList() : lsGui(nullptr) {}

    enum { RECENT_COUNT = 10 };
    // fixed array of menu items of each button
    RecentItem items[RECENT_COUNT];
    // the case when the Go embed concept would come in handy
    RecentListGui *lsGui;
};

struct CRUser {
    CRUser() 
		: clrRecentTree(CLR_NONE) 
	{}

    enum { BTN_COUNT = 6, CLR_COUNT = 10 } ;
    // fixed array of buttons
	RecentList markButtons[BTN_COUNT];
    // fixed array of last colors
    ColorItem clrRecent[CLR_COUNT];
    // fixed array of predefined colors
    ColorItem clrPredef[CLR_COUNT];
	// last color for tree
	Clr clrRecentTree;

	static int Idx(CR::Type t)
    {
        return t - CR::NT_AREA;
    }
    static CR::Type Typ(int i)
    {
        return (CR::Type)(i + CR::NT_AREA);
    }

    void init(RecentCtl *ctl);
	void free(RecentCtl *ctl);

	bool loadCRUser(const String &path);
	bool saveCRUser(const String &path);

	void loadRecentItems(pugi::xml_node elem);
	void saveRecentItems(pugi::xml_node elem);
	void clearRecent();

    void loadRecentColors(pugi::xml_node elem);
    void loadPredefColors(pugi::xml_node elem);
    void loadColors(pugi::xml_node elem, ColorItem *arr);
	void loadRecentGlobals(pugi::xml_node elem);

    void saveRecentColors(pugi::xml_node elem);
    void savePredefColors(pugi::xml_node elem);
    void saveColors(pugi::xml_node elem, ColorItem *arr);
	void saveRecentGlobals(pugi::xml_node elem);

    void updateRecentColors(Clr nclr, const String &name);

	void loadRecentList(pugi::xml_node par, CR::Type t);
    void saveRecentList(pugi::xml_node par, CR::Type t);
    void clearRecentList(CR::Type t);	

	void addToRecentList(int ti, CRMark *mi, Clr nclr);
    void raiseInRecentList(int ti, int ri, CRMark *mi, Clr nclr);
    bool getFromRecentList(int ti, int ri, CRMark *mi, Clr *nclr);
};
