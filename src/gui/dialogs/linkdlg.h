#ifndef LINKDLG_H
#define LINKDLG_H

#include <QDialog>

namespace Ui {
class LinkDlg;
}

class LinkDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LinkDlg(QWidget *parent = 0);
    ~LinkDlg();
public:
    QString m_path;
private slots:
    void onOpen();
    void onPaste();
    void onSelect();
    void onOk();
private:
    Ui::LinkDlg *ui;
};

#endif // LINKDLG_H
