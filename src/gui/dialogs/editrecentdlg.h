#ifndef EDITRECENTDLG_H
#define EDITRECENTDLG_H

#include <QDialog>
#include <QMenu>
#include "../../core/crnode.h"
#include "../gui.h"

namespace Ui {
class EditRecentDlg;
}

class EditRecentDlg : public QDialog
{
    Q_OBJECT

public:
    explicit EditRecentDlg(RecentItem *rid, bool show_tsig, QWidget *parent = 0);
    ~EditRecentDlg();
    bool execDlg();
private:
    void accept();
private slots:
    void onChangeColor(QColor clr);
private:
    Ui::EditRecentDlg *ui;
    RecentItem *m_recentData;
    QColor m_clrMark;
    bool m_sig;
};

#endif // EDITRECENTDLG_H
