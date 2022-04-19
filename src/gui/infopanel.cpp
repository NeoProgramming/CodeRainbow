#include "infopanel.h"
#include "mainwindow.h"
#include <QtGui>
#include <QtDebug>
#include <QFile>   		
#include <QTextStream>	
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <stdlib.h>
#include <limits.h>
#include "../core/crtree.h"

InfoPanel::InfoPanel(QWidget *parent, MainWindow *h)
    : QWidget(parent)
    , mw(h)
    , m_Node(nullptr)
{
    ui.setupUi(this);
    ui.treeMessages->setUniformRowHeights(true);
    ui.treeMessages->header()->setStretchLastSection(false);
    ui.treeMessages->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui.treeMessages->setStyleSheet("background-color: #FED890; alternate-background-color: #F2DE80");
	ui.treeMessages->setColumnCount(1);
	QTreeWidgetItem* headerItem = ui.treeMessages->headerItem();
	
	QHeaderView * header = ui.treeMessages->header();
	header->setSectionResizeMode(QHeaderView::Interactive);
	ui.treeMessages->setColumnWidth(0,400);

	ui.treeMessages->header()->close();
	ui.treeMessages->setIconSize(QSize(20,20));
}

void InfoPanel::initialize()
{
    ui.tabWidget->setElideMode(Qt::ElideNone);	//show tab titles, do not elide them by placing "..."
    
	connect(ui.treeMessages, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui.textEdit, SIGNAL(lostFocus()), this, SLOT(onEditorLostFocus()));

    ui.treeMessages->viewport()->installEventFilter(this);
    ui.treeMessages->installEventFilter(this);
}

void InfoPanel::processEvents()
{
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void InfoPanel::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e);
    static int opacity = 255;
    mw->setWindowOpacity((opacity-=4)/255.0);
    if (opacity<=0)
        qApp->quit();
}

void InfoPanel::onShowContentsMenu(const QPoint &pos)
{
    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(sender());
    if (!treeWidget)
        return;
    QTreeWidgetItem *item = treeWidget->itemAt(pos);
    if (!item)
        return;
}

void InfoPanel::onItemDoubleClicked(QTreeWidgetItem* curItem, int column)
{

}

void InfoPanel::loadText(CRNode *node)
{
    m_Node = node;
    if(!node) {
        ui.textEdit->clear();
        ui.textEdit->setEnabled(false);
    }
    else {
        ui.textEdit->setPlainText(node->text);
        ui.textEdit->setEnabled(true);
    }
}

void InfoPanel::resizeEvent(QResizeEvent *e)
{
	int w = e->size().width();
	ui.treeMessages->resize(e->size());
	ui.treeMessages->setColumnWidth(0,w);
    ui.textEdit->resize(e->size());
}

void InfoPanel::onMsgAdd(const char* message, int image, unsigned long param)
{
	// add new message to message list/tree
	QTreeWidgetItem *root = new QTreeWidgetItem();
	root->setText(0, message);
	root->setData(0, Qt::UserRole, (uint)param);
	ui.treeMessages->addTopLevelItem(root);
}

void InfoPanel::onMsgRemoveAll()
{
	// remove all messages from message list/tree
	int index = ui.treeMessages->topLevelItemCount()-1;
    while(index >= 0) {
		delete ui.treeMessages->takeTopLevelItem(index);
		index--;
	}
}

void InfoPanel::onEditorLostFocus()
{
    if(m_Node) {
        pCR->setText(m_Node, ui.textEdit->toPlainText());
    }
}

