#include "noteedit.h"

NoteEdit::NoteEdit(QWidget * parent)
    : QTextEdit(parent)
{
}

NoteEdit::NoteEdit(const QString & text, QWidget * parent)
    : QTextEdit(text, parent)
{
}

void NoteEdit::focusOutEvent(QFocusEvent *e)
{
    if (e->lostFocus() )
        emit lostFocus();

    QTextEdit::focusOutEvent(e);
}
