#ifndef MARKUPDLG_H
#define MARKUPDLG_H

#include <QDialog>
#include <QMenu>
#include "../../core/crnode.h"

namespace Ui {
class MarkupDlg;
}

class MarkupDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MarkupDlg(const String &fpath, CRMark *markEdit, CRInfo *infoEdit, QWidget *parent = 0);
    ~MarkupDlg();
    bool execDlg();
private:
    void accept();
    void updateCurrColor(QColor clr);
private slots:
    void onGenUniqueId();
    void onClickedCheckInline();
    void onChangeColor(QColor clr);
    void onAddTag();
	void onClear();
	void onRecentColorChanged(int index);
	void onPredefColorChanged(int index);
    void onClickedCheckFilesOutline();
    void onClickedCheckTags();

private:
    Ui::MarkupDlg *ui;
    CRNode *m_nodeFile;
    CRMark *m_markEdit;
    CRInfo *m_infoEdit;
    bool m_autoFillPath;
    QColor m_clrMark;
    QColor m_clrNode;
};

#endif // MARKUPDLG_H
