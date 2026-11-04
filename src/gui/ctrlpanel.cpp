#include "ctrlpanel.h"
#include "mainwindow.h"

#include <QIcon>
#include <QtGui>
#include <QtDebug>
#include <QFile>   		
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include "dialogs/linkdlg.h"
#include "dialogs/prjpropsdlg.h"

#include "../core/core.h"
#include "../core/crtree.h"


CtrlPanel::CtrlPanel(QWidget *parent, MainWindow *h)
    : QWidget(parent)
    , m_wMain(h)
{
    ui.setupUi(this);

    ui.treeContents->setUniformRowHeights(true);
    ui.treeContents->header()->setStretchLastSection(false);
    ui.treeContents->header()->setSectionResizeMode(QHeaderView::Fixed);
    ui.treeContents->setStyleSheet("background-color: #ECECFE; alternate-background-color: #FFFFC2");
	ui.treeContents->setColumnCount(1);
	ui.treeContents->setColumnWidth(0,400);
	ui.treeContents->header()->close();
	ui.treeContents->setIconSize(QSize(20,20));
    
	ui.listBookmarks->setUniformRowHeights(true);
    ui.listBookmarks->header()->setStretchLastSection(false);
    ui.listBookmarks->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    menuNode.addAction(tr("Set active"), this, SLOT(onNodeSetActive()));
    menuNode.addAction(tr("Edit..."), this, SLOT(onNodeEdit()));
    menuNode.addAction(tr("Add group..."), this, SLOT(onNodeAddGroup()));
    menuNode.addAction(tr("Add file..."), this, SLOT(onNodeAddFile()));
    menuNode.addAction(tr("Add link..."), this, SLOT(onNodeAddLink()));
    menuNode.addAction(tr("Remove"), this, SLOT(onNodeRemove()));
    menuNode.addAction(tr("Close all windows"), this, SLOT(onNodeCloseAllWindows()));

    menuBase.addAction(tr("Edit..."), this, SLOT(onNodeEdit()));
    menuBase.addAction(tr("Add group..."), this, SLOT(onNodeAddGroup()));
    menuBase.addAction(tr("Add file..."), this, SLOT(onNodeAddFile()));
    menuBase.addAction(tr("Add link..."), this, SLOT(onNodeAddLink()));
    menuBase.addAction(tr("Remove"), this, SLOT(onNodeRemove()));
    menuBase.addAction(tr("Close all windows"), this, SLOT(onNodeCloseAllWindows()));
}

CRNode* CtrlPanel::getSelNode(QTreeWidgetItem **pItem)
{
    QTreeWidgetItem *item = ui.treeContents->currentItem();
    if(!item)
        return 0;
    if(pItem)
        *pItem = item;
    return item->data(0, Qt::UserRole).value<CRNode*>();
}

void CtrlPanel::initialize()
{
    ui.tabWidget->setElideMode(Qt::ElideNone);	//show tab titles, do not elide them by placing "..."
	ui.tabWidget->setCurrentIndex(0);

    connect(ui.treeContents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onTreeContextMenu(QPoint)));
	connect(ui.treeContents, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui.treeContents, SIGNAL(itemSelectionChanged()), this, SLOT(onTreeSelChanged()));

    ui.treeContents->viewport()->installEventFilter(this);
    ui.treeContents->installEventFilter(this);
}

void CtrlPanel::processEvents()
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void CtrlPanel::loadTree()
{
	setCursor(Qt::WaitCursor);
	ui.treeContents->clear();
	QTreeWidgetItem *root = new QTreeWidgetItem();
	ui.treeContents->addTopLevelItem(root);
    loadTreeNode(root, pCR->getRoot());
	setCursor(Qt::ArrowCursor);
}

void CtrlPanel::updateNode(QTreeWidgetItem *item, CRNode* node)
{
    item->setData(0, Qt::UserRole, QVariant::fromValue<CRNode*>(node));
    item->setText(0, QString::fromStdWString ( node->dispName() ));
    item->setIcon(0, Gui::getTreeIcon(node->type));

    node->setParam(item); //<<bad??? bad align
    qDebug() << "updateNode " << node << " " << item << " " << node->getParam<QTreeWidgetItem*>();

    if(node->flags & CRNode::NF_ACTIVE)
        setItemBold(item, true);
}

void CtrlPanel::loadTreeNode(QTreeWidgetItem *item, CRNode* node)
{
    updateNode(item, node);
    Q_FOREACH(CRNode* branch, node->branches) {
        addTreeItem(item, branch);
    }
}

void CtrlPanel::addTreeItem(QTreeWidgetItem *parItem, CRNode *node)
{
    QTreeWidgetItem *child = new QTreeWidgetItem(parItem);
    loadTreeNode(child, node);
}

void CtrlPanel::removeTreeItem(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    int index;
    if (parent) {
        index = parent->indexOfChild(item);
        delete parent->takeChild(index);
    }
    else {
        index = ui.treeContents->indexOfTopLevelItem(item);
        delete ui.treeContents->takeTopLevelItem(index);
    }
}

void CtrlPanel::setItemBold(QTreeWidgetItem *item, bool bold)
{
	QFont font = item->font(0);
	font.setBold( bold );
	item->setFont(0, font );
}

void CtrlPanel::onTreeContextMenu(const QPoint &pos)
{
    CRNode *node = getSelNode();
    if(!node) return;
    switch(node->type) {
    case CR::NT_FILE:
    case CR::NT_GROUP:
    case CR::NT_AREA:
    case CR::NT_LABEL:
    case CR::NT_BLOCK:
    case CR::NT_LSIG:
    case CR::NT_BSIG:
    case CR::NT_TAG:
    case CR::NT_LINK:
        menuNode.exec(ui.treeContents->viewport()->mapToGlobal(pos));
		break;
    case CR::NT_FBASE:
    case CR::NT_DIR:
        menuBase.exec(ui.treeContents->viewport()->mapToGlobal(pos));
        break;
	}
}

void CtrlPanel::onItemDoubleClicked(QTreeWidgetItem* curItem, int column)
{
    CRNode* node = curItem->data(0, Qt::UserRole).value<CRNode*>();
    m_wMain->openNode(node);
}

void CtrlPanel::onTreeSelChanged()
{
    CRNode *node = getSelNode();
    m_wMain->selectNode(node);
}

void CtrlPanel::onNodeCloseAllWindows()
{
    CRNode* node = getSelNode();
    if(!node) return;
    m_wMain->closeWindows(node);
}

void CtrlPanel::resizeEvent(QResizeEvent *e)
{
	int w = e->size().width();
	ui.treeContents->setColumnWidth(0,w);
}

void CtrlPanel::removeItem(QTreeWidgetItem *item)
{
	QTreeWidgetItem *parent = item->parent();
	int index;
    if (parent) {
		index = parent->indexOfChild(item);
		delete parent->takeChild(index);
	} 
    else {
		index = ui.treeContents->indexOfTopLevelItem(item);
		delete ui.treeContents->takeTopLevelItem(index);
	}
}

void CtrlPanel::updateSubtree(QTreeWidgetItem *item)
{
    CRNode *node = item->data(0, Qt::UserRole).value<CRNode*>();
    if(node->flags & CRNode::NF_ACTIVE)
        setItemBold(item, true);
    else
        setItemBold(item, false);
    int n = item->childCount();
    for(int i=0; i<n; i++)
        updateSubtree(item->child(i));
}

void CtrlPanel::onNodeSetActive()
{
    QTreeWidgetItem *item;
    CRNode* node = getSelNode(&item);
    if(!node) return;
    node->setSubtreeFlags(0, CRNode::NF_ACTIVE);
    node->setFlags(CRNode::NF_ACTIVE, 0);
    updateSubtree(item);
}

void CtrlPanel::onNodeEdit()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(CR::isSys(node->type))
        return;

	//@// собрать CRMark из узла
	CRMark mi;
	mi.type = node->type;
	mi.id = node->id;
	mi.mclr = node->nclr;
	if (CR::isTag(node->type))
		mi.tags.push_back(node->id);
	CR::Type st = node->getSysType();
	mi.mk_fuid = (st == CR::NT_FBASE);
	mi.mk_elem = (st == CR::NT_OUTLINE);
	mi.mk_tsig = (st == CR::NT_TAGS);
	mi.gen_uid = false;


	//@ const String &fpath, CRMark *markEdit, CRInfo *infoEdit, QWidget *parent = 0
    //@MarkupDlg dlg(node->getFile(), node, this);
	MarkupDlg dlg(node->getPath(), &mi, node, this);
    if(dlg.execDlg()) {
        updateNode(node->getParam<QTreeWidgetItem*>(), node);
    }
}

void CtrlPanel::onNodeAddGroup()
{
    CRNode* node = getSelNode();
    if(!node) return;
    QString name = QInputDialog::getText(NULL, "Input group name",
           "Name:", QLineEdit::Normal, "");
    if(!name.isEmpty()) {
         addChild(node, CR::NT_GROUP, name, "");
    }
}

void CtrlPanel::onNodeAddFile()
{
    CRNode* node = getSelNode();
    if(!node) return;
    QString path = QFileDialog::getOpenFileName(this, tr("Select file"), "", "*.*");
    if(!path.isEmpty()) {
        QString name = QFileInfo(path).fileName();
        addChild(node, CR::NT_FILE, name, path);
    }
}

void CtrlPanel::onNodeAddLink()
{
    CRNode* node = getSelNode();
    if(!node) return;
    LinkDlg dlg;
    if(dlg.exec() == QDialog::Accepted) {
        QString name = QUrl(dlg.m_path).host();
        if(name.isEmpty())
            name = QFileInfo(dlg.m_path).fileName();
        addChild(node, CR::NT_LINK, name, dlg.m_path);
    }
}

void CtrlPanel::onNodeRemove()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(CR::isSys(node->type))
        return;
    int ret = QMessageBox::warning(this, AppName, tr("Remove node?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {
        if(pCR->removeNode(node))
            removeTreeItem(node->getParam<QTreeWidgetItem*>());
        else
            QMessageBox::warning(this, AppName, tr("Unable to remove node"), QMessageBox::Ok);
    }
}

void CtrlPanel::addChild(CRNode *parNode, CR::Type t, const QString &name, const QString &path)
{
    CRNode *child = pCR->addNode(parNode, t, name.toStdWString(), path.toStdWString(), _T(""), _T(""), -1);
    addTreeItem(parNode->getParam<QTreeWidgetItem*>(), child);
}

void CtrlPanel::moveItemUp()
{
    QTreeWidgetItem *item = nullptr;
    CRNode* node = getSelNode(&item);
    if(!node || !item)
        return;
    if(pCR->moveNodeUp(node)) {
        QTreeWidgetItem * parItem = item->parent();
        int i = parItem->indexOfChild(item);
        item = parItem->takeChild(i);
        parItem->insertChild(i-1, item);
        ui.treeContents->setCurrentItem(item);
    }
}

void CtrlPanel::moveItemDown()
{
    QTreeWidgetItem *item = nullptr;
    CRNode* node = getSelNode(&item);
    if(!node || !item)
        return;
    if(pCR->moveNodeDown(node)) {
        QTreeWidgetItem * parItem = item->parent();
        int i = parItem->indexOfChild(item);
        item = parItem->takeChild(i);
        parItem->insertChild(i+1, item);
        ui.treeContents->setCurrentItem(item);
    }
}

void CtrlPanel::moveItemLeft()
{
    QTreeWidgetItem *item = nullptr;
    CRNode* node = getSelNode(&item);
    if(!node || !item)
        return;
    if(pCR->moveNodeLeft(node)) {
        QTreeWidgetItem * parItem = item->parent();
        int i = parItem->indexOfChild(item);
        item = parItem->takeChild(i);
        i = parItem->parent()->indexOfChild(parItem);
        parItem = parItem->parent();
        parItem->insertChild(i+1, item);
        ui.treeContents->setCurrentItem(item);
    }
}

void CtrlPanel::moveItemRight()
{
    QTreeWidgetItem *item = nullptr;
    CRNode* node = getSelNode(&item);
    if(!node || !item)
        return;
    if(pCR->moveNodeRight(node)) {
        QTreeWidgetItem * parItem = item->parent();
        int i = parItem->indexOfChild(item);
        item = parItem->takeChild(i);
        parItem = parItem->child(i-1);
        parItem->addChild(item);
        ui.treeContents->setCurrentItem(item);
    }
}
