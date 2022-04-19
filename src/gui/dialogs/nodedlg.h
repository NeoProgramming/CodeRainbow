#ifndef NodeDlg_H
#define NodeDlg_H

#include <QDialog>
#include <QMenu>
#include "../../core/crnode.h"

namespace Ui {
class NodeDlg;
}

class NodeDlg : public QDialog
{
    Q_OBJECT

public:
    explicit NodeDlg(bool editMode, bool enPath, CRNode *nodeFile, CRInfo *infoEdit, CR::Type type, QWidget *parent = 0);
    ~NodeDlg();
    bool execDlg();
private:
    void accept();
    void updateGui();
private slots:
    void onSelType(CR::Type type);
    void onPathOverview();
public:
    CR::Type m_type;
private:
    Ui::NodeDlg *ui;
    CRNode *m_nodeFile;
    CRInfo *m_infoEdit;
    bool m_editMode;
    bool m_enPath;
    QMenu m_typesMenu;
};

#endif // NodeDlg_H
