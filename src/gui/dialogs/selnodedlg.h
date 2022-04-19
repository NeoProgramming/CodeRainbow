#ifndef SELNODEDLG_H
#define SELNODEDLG_H

#include <QDialog>
#include <QStringList>
#include <QTreeWidgetItem>
#include "../../core/crnode.h"

namespace Ui {
class SelNodeDlg;
}

class SelNodeDlg : public QDialog
{
    Q_OBJECT
public:
    explicit SelNodeDlg(QWidget *parent = nullptr);
    ~SelNodeDlg();

    bool execDlg(CRNode *locroot, CRNode *exclude);
    CRNode* selNode;
private slots:
    void onOk();
    void onItemDoubleClicked(QTreeWidgetItem*,int);
private:
    void loadItem(QTreeWidgetItem *item, CRNode* node, CRNode *exclude);
    void loadLevel(QTreeWidgetItem *treeNode, CRNode* locroot, CRNode *exclude);
    Ui::SelNodeDlg *ui;
};

#endif // SELNODEDLG_H
