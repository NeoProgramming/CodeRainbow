#include "gui.h"
#include <QObject>
#include "../qtplugin/crconstants.h"

// static singleton pointer
Gui* Gui::pGui = nullptr;

inline MarkItem* MItem(RecentItem &it)
{
    return (MarkItem*)(it.itGui);
}

inline MarkBtn *MBtn(RecentList &ls)
{
    return (MarkBtn*)(ls.lsGui);
}

void Gui::init()
{
    if(!Gui::pGui)
        Gui::pGui = new Gui;
    Gui::pGui->initRes();
}

void Gui::term()
{
    //return;
    delete pGui;
    Gui::pGui = nullptr;
    qDebug() << "Gui::term";
}

void Gui::initRes()
{
    iconsTree[CR::NT_NONE]    = QIcon(":/node/res/node-node.png");
    iconsTree[CR::NT_ROOT]    = QIcon(":/node/res/node-root.png");
    iconsTree[CR::NT_FBASE]   = QIcon(":/node/res/node-base.png");
    iconsTree[CR::NT_TAGS]    = QIcon(":/node/res/node-tags.png");
    iconsTree[CR::NT_OUTLINE] = QIcon(":/node/res/node-outline.png");

    iconsTree[CR::NT_FILE]  = QIcon(":/node/res/node-file.png");
    iconsTree[CR::NT_DIR]   = QIcon(":/node/res/node-dir.png");
    iconsTree[CR::NT_GROUP] = QIcon(":/node/res/node-group.png");
    iconsTree[CR::NT_LINK]  = QIcon(":/node/res/node-link.png");

    iconsTree[CR::NT_AREA]  = QIcon(":/node/res/node-area.png");
    iconsTree[CR::NT_LABEL] = QIcon(":/node/res/node-label.png");
    iconsTree[CR::NT_BLOCK] = QIcon(":/node/res/node-block.png");
    iconsTree[CR::NT_LSIG]  = QIcon(":/node/res/node-lsig.png");
    iconsTree[CR::NT_BSIG]  = QIcon(":/node/res/node-bsig.png");
    iconsTree[CR::NT_TAG]   = QIcon(":/node/res/node-tag.png");
    iconsTree[CR::NT_NAME]  = QIcon(":/node/res/node-name.png");

    pixNoClr = QPixmap(":/misc/res/misc-noclr.png");
    pixEmpty = QPixmap(":/misc/res/misc-empty.png");

    iconsTool[CR::TI_INFO]  = QIcon(":/tools/res/tools-info.png");
    iconsTool[CR::TI_MOVE]  = QIcon(":/tools/res/tools-move.png");
    iconsTool[CR::TI_EDIT]  = QIcon(":/tools/res/tools-edit.png");
    iconsTool[CR::TI_REMOVE]= QIcon(":/tools/res/tools-remove.png");
    iconsTool[CR::TI_ADD]   = QIcon(":/tools/res/tools-add.png");
    iconsTool[CR::TI_COVER] = QIcon(":/tools/res/tools-coverage.png");
    iconsTool[CR::TI_ACTIVE]= QIcon(":/tools/res/tools-active.png");
    iconsTool[CR::TI_SETBASE]=QIcon(":/tools/res/tools-setbase.png");
}

QIcon& Gui::getTreeIcon(int i)
{
    if(!pGui)
        qDebug() << "getTreeIcon: ERROR!";
    if(i<0 || i>=CR::NT_ItemsCount)
        i=0;
    return pGui->iconsTree[i];
}

QIcon& Gui::getToolIcon(int i)
{
    if(!pGui)
        qDebug() << "getMiscIcon: ERROR!";
    if(i<0 || i>=CR::TI_ItemsCount)
        i=0;
    return pGui->iconsTool[i];
}

QToolButton* Gui::createMenuButton(QMenu *menu, const QString &tooltip)
{
    QToolButton *btn = new QToolButton;
    btn->setPopupMode(QToolButton::MenuButtonPopup);
    btn->setMenu(menu);
//    btn->setAutoFillBackground(true);
    btn->setToolTip(tooltip);
    return btn;
}

QIcon Gui::createSolidColorIcon(Clr clr, bool empty)
{
    QPixmap pix(16, 16);
    QPainter painter(&pix);
    if(empty) {
        return QIcon();// pGui->iconEmpty;
    }
    else if(clr == CLR_NONE) {
        painter.fillRect(0,8,8,8,Qt::lightGray);
        painter.fillRect(8,0,8,8,Qt::lightGray);
        painter.fillRect(0,0,8,8,Qt::white);
        painter.fillRect(8,8,8,8,Qt::white);
    }
    else {
        pix.fill(clr);
    }
    QIcon icon(pix);
    return icon;
}

float Gui::luminance(QColor clr)
{
    // 0..255
    return (0.2126*clr.red() + 0.7152*clr.green() + 0.0722*clr.blue());
}

QIcon Gui::createQuarterColorIcon(QImage &base, RecentItem &d)
{
    // contrast symbol on the background
    const bool empty = d.mark.empty();
    const int width = base.width();
    const int height = base.height();
    QPixmap px = QPixmap(width, height);
    QPainter painter(&px);
    painter.drawImage(0,0,base);
    int y = height*3/4;
    int h = height/4;
    int w = width/4;
    if(empty) {
        painter.fillRect(  0, y, width, h, QBrush( pGui->pixEmpty ));
    }
    else if(d.nclr == CLR_NONE) {
        painter.fillRect(  0, y, w, h, Qt::lightGray);
        painter.fillRect(  w, y, w, h, Qt::white);
        painter.fillRect(2*w, y, w, h, Qt::lightGray);
        painter.fillRect(3*w, y, w, h, Qt::white);
    }
    else {
        painter.fillRect(  0, y, width, h, d.nclr);
    }

    return QIcon(px);
}

/*
QIcon Gui::createColorIcon(QIcon &base, QColor clr)
{
    // 1/4 of the pictogram from the bottom is filled with color
    QPixmap px = base.pixmap(32,32);
    QPainter painter(&px);
    if(clr.isValid())
        painter.fillRect(0,24,32,8,clr);
    return QIcon(px);
}*/

void Gui::updateMarkMenuItem(RecentItem &item)
{
//    qDebug() << "updateMarkMenuItem: " << item.mark << " a=" << item.action;

    bool empty = item.mark.empty();
    MItem(item)->icon = createSolidColorIcon(item.nclr, empty);
    MItem(item)->actionMark->setEnabled(!empty);

    if(empty) {
        MItem(item)->actionMark->setText(QString("<empty>"));
        MItem(item)->actionEdit->setText(QString("<empty>"));
    }
    else {
        MItem(item)->actionMark->setText(QString::fromStdWString(item.mark));
        MItem(item)->actionEdit->setText(QString::fromStdWString(item.mark));
    }
    MItem(item)->actionMark->setIcon(MItem(item)->icon);
    MItem(item)->actionEdit->setIcon(MItem(item)->icon);
}

QMenu* Gui::createToolMenu(RecentList *rl, QObject *wgt, const char *slotNew, const char *slotRecent)
{
    MarkBtn *mb = MBtn(*rl);
    mb->mapper = new QSignalMapper(wgt);
    QAction *actionNew = new QAction(QObject::tr("New..."), wgt);
    QObject::connect(actionNew, SIGNAL(triggered()), wgt, slotNew);

    QMenu* menu = new QMenu();
    menu->addAction(actionNew);

    QMenu *menuEdit = new QMenu(QObject::tr("Edit recent"));
    menu->addMenu(menuEdit);

    menu->addSeparator();

    for(int i=0; i<RecentList::RECENT_COUNT; i++) {
        MItem(rl->items[i])->actionMark = menu->addAction("", mb->mapper, SLOT(map()));
        MItem(rl->items[i])->actionEdit = menuEdit->addAction("", mb->mapper, SLOT(map()));
        updateMarkMenuItem(rl->items[i]);
        mb->mapper->setMapping(MItem(rl->items[i])->actionMark, i);
        mb->mapper->setMapping(MItem(rl->items[i])->actionEdit, i+RecentList::RECENT_COUNT);
    }

    QObject::connect(mb->mapper, SIGNAL(mapped(int)), wgt, slotRecent);
    return menu;
}

void Gui::createMultiTool(QToolBar *tbar, RecentList *rl, QObject *wgt, const char *res, const QString &tip,
                          const char *slotNew, const char *slotMenu, const char *slotClicked)
{
    MarkBtn *mb = MBtn(*rl);
    mb->menu = createToolMenu(rl, wgt, slotNew, slotMenu);
    mb->img = QImage(QLatin1String(res));
    mb->tip = tip;
    mb->tb = createMenuButton(mb->menu, tip);
    QObject::connect(mb->tb, SIGNAL(clicked()), wgt, slotClicked);
    tbar->addWidget(mb->tb);
    mb->tb->setIcon( createQuarterColorIcon( mb->img, rl->items[0] ) );
}

void Gui::updateMarkMenu(RecentList *rl)
{
    MarkBtn *mb = MBtn(*rl);
    for(int i=0; i<RecentList::RECENT_COUNT; i++)
        updateMarkMenuItem(rl->items[i]);
    mb->tb->setIcon( createQuarterColorIcon( mb->img, rl->items[0] ) );
    if(!rl->items[0].mark.empty())
        mb->tb->setToolTip( mb->tip + "\r\n" + QString::fromStdWString(rl->items[0].mark) + "\r\n"
                + (rl->items[0].gen_uid ? "GEN ":"")
                + (rl->items[0].mk_fuid ? "+FILES ":"")
                + (rl->items[0].mk_tsig ? "+TAGS ":"")
                + (rl->items[0].mk_elem ? "+OUTLINE":"")
                );
}

void Gui::createBtnTool(QToolBar *tbar, const char *res, QAction *act, const QString &tip)
{
    QToolButton *tb = new QToolButton();
    QIcon icon = QIcon(QLatin1String(res));
    // tb->setDefaultAction(act);
    tb->setIcon(icon);
    tb->setText("");
    tb->setToolTip(tip);
    tbar->addWidget(tb);
    QObject::connect(tb, &QToolButton::clicked, act, &QAction::trigger);
//    QObject::connect(tb, &QToolButton::clicked, [=]() {
//        act->trigger();
//    });
}

void Gui::createBtnTool(QToolBar *tbar, QObject *wgt, const char *res, const QString &tip, const char *slotClicked)
{
    QToolButton *tb = new QToolButton();
    QIcon icon = QIcon(QLatin1String(res));
    tb->setIcon(icon);
//  tb->setAutoFillBackground(true);
    tb->setToolTip(tip);
    QObject::connect(tb, SIGNAL(clicked()), wgt, slotClicked);
    tbar->addWidget(tb);
}

RecentItemGui* Gui::createItemGui()
{
    return new MarkItem;
}

RecentListGui* Gui::createListGui()
{
    return new MarkBtn;
}

void Gui::destroyItemGui(RecentItemGui *p)
{

}

void Gui::destroyListGui(RecentListGui *p)
{

}

void Gui::loadColorCombo(QComboBox *combo, ColorItem *arr)
{
    combo->addItem(createSolidColorIcon(CLR_NONE, false), "<No Color>");
    for(int i=0; i<CRUser::CLR_COUNT; i++) {
        if(!arr[i].name.empty()) {
            QIcon icon = createSolidColorIcon(arr[i].clr, false);
            combo->addItem(icon, QString::fromStdWString(arr[i].name));
        }
    }
}
