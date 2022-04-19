#ifndef NOTEEDIT_H
#define NOTEEDIT_H
#include <QTextEdit>
#include <QFocusEvent>

class NoteEdit : public QTextEdit
{
    Q_OBJECT
public:
    NoteEdit(QWidget * parent = 0);
    NoteEdit(const QString & text, QWidget * parent = 0);

    void focusOutEvent(QFocusEvent *e);

signals:
    void lostFocus();
};

#endif // NOTEEDIT_H

