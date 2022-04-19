#include "contentview.h"

#include <QHeaderView>
#include <QMenu>
#include <QElapsedTimer>

#include "crmodel.h"
#include "../core/crnode.h"
#include "crplugincore.h"
#include "infopane.h"
#include "../gui/gui.h"
#include "../gui/widgets/colormenu.h"

// static
ContentView* ContentView::currentView = nullptr;
bool ContentView::inFocus = false;

ContentView::ContentView(QAbstractItemModel *model, QWidget *parent)
    : QTreeView(parent)
{
    setModel(model);

    connect(model,SIGNAL(modelReset()),SLOT(expandAll()));
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(onItemDoubleClicked(const QModelIndex &)));
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onTreeSelChanged(QItemSelection,QItemSelection)));

    header()->resizeSection(0, 200);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::Fixed);
//@    header()->close();
    setExpandsOnDoubleClick(false);
    setFrameStyle(QFrame::Plain);
    setFrameShape(QFrame::NoFrame);
    setContentsMargins(0, 0, 0, 0);
    setUniformRowHeights(true);
//  setAlternatingRowColors(true);
//  setStyleSheet("background-color: #ECECFE; alternate-background-color: #FFFFC2");
    setColumnWidth(0,400);
    setIconSize(QSize(20,20));

    currentView = this;
}

ContentView::~ContentView()
{
//@    m_crPCore->setCurrTreeView(nullptr);
    currentView = nullptr;
    qDebug() << "ContentView::~ContentView()";
}

//void ContentView::onItemChanged()

void ContentView::onItemDoubleClicked(const QModelIndex &index)
{
    //qDebug() << "onItemDoubleClicked:";
    //extern QElapsedTimer gt1;
    //gt1.start();

    CRModel *m = static_cast<CRModel*>(model());
    CRNode* node = index.data(Qt::UserRole).value<CRNode*>();
    m_crPCore->openNode(node);
}

void ContentView::onTreeSelChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList indices = selected.indexes();
    if(indices.count() > 0) {
        const QModelIndex &index = indices.first();
        CRNode* node = index.data(Qt::UserRole).value<CRNode*>();
        m_crPCore->infoPane()->loadText(node);
//      qDebug() << "ContentView::onTreeSelChanged:";
    }
}


void ContentView::contextMenuEvent(QContextMenuEvent *event)
{
    // depending on the type of node, we call one or another context menu
    QModelIndex index = indexAt(event->pos());
    CRNode* node = index.data(Qt::UserRole).value<CRNode*>();
    if(!node)
        return;
    QMenu *menu = m_crPCore->treeMenu(node);
    if(menu)
        menu->exec(mapToGlobal(event->pos()));
}

void ContentView::focusInEvent(QFocusEvent *event)
{
    currentView = this;
    inFocus = true;
}

void ContentView::focusOutEvent(QFocusEvent *event)
{
    inFocus = false;
}

void ContentView::closeEvent(QCloseEvent *event)
{
    currentView = nullptr;
    inFocus = false;
}


