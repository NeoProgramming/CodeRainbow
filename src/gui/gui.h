#pragma once

#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QIcon>
#include <QComboBox>
#include <QSignalMapper>

#include "../core/crnode.h"
#include "../core/cruser.h"


// recent list for one type of metacomment
struct MarkItem : RecentItemGui {
    QIcon icon;
    QAction* actionMark; // created at initialization and always stored
    QAction* actionEdit;

    MarkItem() : actionMark(nullptr), actionEdit(nullptr) {}
};

// one mark button with recent list
struct MarkBtn : RecentListGui {
    QToolButton *tb;
    QMenu *menu;
    QString tip;
    QImage img;
    QSignalMapper *mapper;

    MarkBtn() :  tb(nullptr), menu(nullptr), mapper(nullptr) {}
};

inline MarkItem* MItem(RecentItem &it);
inline MarkBtn *MBtn(RecentList &ls);

class Gui : public RecentCtl {

public:
    virtual RecentItemGui *createItemGui();
    virtual RecentListGui *createListGui();
	virtual void destroyItemGui(RecentItemGui *p);
	virtual void destroyListGui(RecentListGui *p);

    static void init();
    static void term();

    static float luminance(QColor clr);

    static QIcon& getTreeIcon(int i);
    static QIcon& getToolIcon(int i);

    static QToolButton* createMenuButton(QMenu *menu, const QString &tooltip);
    static QMenu* createToolMenu(RecentList *mb, QObject *wgt, const char *slotNew, const char *slotRecent);
    static QIcon createSolidColorIcon(Clr clr, bool empty);
    static QIcon createQuarterColorIcon(QImage &base, RecentItem &d);

    static void createMultiTool(QToolBar *tbar, RecentList *mb, QObject *wgt, const char *res, const QString &tip,
                           const char *slotNew, const char *slotMenu, const char *slotClicked);
    static void updateMarkMenu(RecentList *mb);
    static void updateMarkMenuItem(RecentItem &item);
    static void createBtnTool(QToolBar *tbar, QObject *wgt, const char *res, const QString &tip, const char *slot);
    static void createBtnTool(QToolBar *tbar, const char *res, QAction *act, const QString &tip);

    static void loadColorCombo(QComboBox *combo, ColorItem *arr);
private:
    void initRes();

    QIcon iconsTree[CR::NT_ItemsCount];
    QIcon iconsTool[CR::TI_ItemsCount];
    QPixmap pixNoClr;
    QPixmap pixEmpty;
public:
    static Gui *pGui;
};
