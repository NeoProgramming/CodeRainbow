#ifndef PRJPROPSDLG_H
#define PRJPROPSDLG_H

#include <QDialog>
#include "../../core/crtree.h"

namespace Ui {
class PrjPropsDlg;
}

class PrjPropsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit PrjPropsDlg(QWidget *parent = 0);
    ~PrjPropsDlg();
    bool execDlg();
private:
    void load();
    void accept();
    void onRestoreDefs();
public:
    PrjProps pp;
private:
    Ui::PrjPropsDlg *ui;
};

#endif // PRJPROPSDLG_H
