#ifndef MEMOFORM_H
#define MEMOFORM_H

#include <QWidget>
#include "../../core/crnode.h"

namespace Ui {
class MemoForm;
}

class MemoForm : public QWidget
{
    Q_OBJECT

public:
    explicit MemoForm(QWidget *parent = 0);
    ~MemoForm();
    void loadText(CRNode *node);
signals:
    void textUpdated(CRNode *node, const QString &text);
    void fixPressed();
private slots:
    void onEditorLostFocus();
private:
    Ui::MemoForm *ui;
    CRNode *m_node = nullptr;
};

#endif // MEMOFORM_H
