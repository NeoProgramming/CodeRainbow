#ifndef CTRLPANEL_H
#define CTRLPANEL_H

#include "ui_ctrlpanel.h"
#include <QListWidget>
#include <QTreeWidget>
#include <QMenu>
#include "../core/crnode.h"

class MainWindow;

class CtrlPanel : public QWidget
{
    Q_OBJECT
public:
    CtrlPanel(QWidget *parent, MainWindow *h);
    inline QTabWidget *tabWidget() const  { return ui.tabWidget; }
   
    CRNode* getSelNode(QTreeWidgetItem **pItem = nullptr);
public:
    void initialize();	//setup connections and actions and call initTabs()
    void loadTree();
    void addTreeItem(QTreeWidgetItem *parItem, CRNode *node);
    void removeTreeItem(QTreeWidgetItem *item);
    void moveItemUp();
    void moveItemDown();
    void moveItemLeft();
    void moveItemRight();
private slots:
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onTreeSelChanged();
    void onTreeContextMenu(const QPoint &pos);
    void processEvents();

    void onNodeSetActive();
    void onNodeEdit();
    void onNodeAddGroup();
    void onNodeAddFile();
    void onNodeAddLink();
    void onNodeRemove();
    void onNodeCloseAllWindows();
private:
    void updateNode(QTreeWidgetItem *item, CRNode* node);
    void addChild(CRNode *parNode, CRNode::Type t, const QString &name, const QString &path);
    void loadTreeNode(QTreeWidgetItem *item, CRNode* node);
    void setItemBold(QTreeWidgetItem *item, bool bold);
    void removeItem(QTreeWidgetItem *item);

    void updateSubtree(QTreeWidgetItem *item);
private:
    void resizeEvent(QResizeEvent *);
private:
    Ui::CtrlPanel ui;
    MainWindow *m_wMain;
    QMenu menuBase;
    QMenu menuNode; // without dir
};

#endif // CTRLPANEL_H
