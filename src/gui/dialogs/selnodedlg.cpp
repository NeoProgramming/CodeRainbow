#include "selnodedlg.h"
#include "ui_selnodedlg.h"
#include "../../core/crtree.h"
#include "../gui.h"

SelNodeDlg::SelNodeDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelNodeDlg)
    , selNode(nullptr)
{
    ui->setupUi(this);
    connect(ui->pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
    connect(ui->pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->treeContents, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*,int)));

//  QTreeWidgetItem* headerItem = ui->treeContents->headerItem();
//  headerItem->setText(0, "Items");
}

SelNodeDlg::~SelNodeDlg()
{
    delete ui;
}

bool SelNodeDlg::execDlg(CRNode *locroot, CRNode *exclude)
{
    if(!locroot || locroot == exclude) {
        qDebug() << "exclude:1" << locroot;
        return false;
    }
	setCursor(Qt::WaitCursor);
    ui->treeContents->clear();

    QTreeWidgetItem *root = new QTreeWidgetItem();
    ui->treeContents->addTopLevelItem(root);
    loadItem(root, locroot, exclude);
	root->setExpanded(true);
	setCursor(Qt::ArrowCursor);
	return exec() == QDialog::Accepted;
}

void SelNodeDlg::loadItem(QTreeWidgetItem *item, CRNode* node, CRNode *exclude)
{
    item->setText(0, QString::fromStdWString( node->dispName() ) );
    item->setData(0, Qt::UserRole, QVariant::fromValue<CRNode*>(node));
    item->setIcon(0, Gui::getTreeIcon( node->type ));
    loadLevel(item, node, exclude);
}

void SelNodeDlg::loadLevel(QTreeWidgetItem *treeNode, CRNode* locroot, CRNode *exclude)
{
    // load tree level
    Q_FOREACH(CRNode* node, locroot->branches) {
        if(node != exclude) {
            QTreeWidgetItem *item = new QTreeWidgetItem(treeNode);
            loadItem(item, node, exclude);
        }
	}
}

void SelNodeDlg::onOk()
{
    QTreeWidgetItem *item = ui->treeContents->currentItem();
    if(!item)
        return;
    selNode = item->data(0, Qt::UserRole).value<CRNode*>();
	accept();	
}

void SelNodeDlg::onItemDoubleClicked(QTreeWidgetItem*,int)
{
    onOk();
}

