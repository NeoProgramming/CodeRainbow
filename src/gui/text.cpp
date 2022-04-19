#include "text.h"

/* QString operator=(const String &str)
{
    return QString::fromStdWString(str);
}

String operator=(const QString &str)
{
    return str.toStdWString();
}*/

QString extractCaretLine(QTextCursor &cursor, int *pcur)
{
	// universal function for getting a string
    // based on the results of the analysis: if there is a selection, then return the selection,
    // otherwise return the entire string;
    // also return the cursor position

    int posStart = cursor.selectionStart();
	int posEnd = cursor.selectionEnd();

    cursor.setPosition(posStart, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    int index = posStart - cursor.selectionStart();
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

    QString str = cursor.selectedText();
    if(pcur)
        *pcur = index;

    // restore selection
	cursor.setPosition(posStart, QTextCursor::MoveAnchor);
	cursor.setPosition(posEnd, QTextCursor::KeepAnchor);
    return str;
}

QString extractLine(QTextCursor &cursor, int pos)
{
    cursor.setPosition(pos, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

    QString str = cursor.selectedText();
    return str;
}

/*@
QString extractSignature(QTextCursor &cursor)
{
    // extract signature - or selected part or string
    int posStart = cursor.selectionStart();
    int posEnd = cursor.selectionEnd();
    if(posStart == posEnd) {
        // if there is no selection, select the entire line
        cursor.setPosition(posStart, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    // otherwise specific selection
    QString str = cursor.selectedText();
    str = removeComments(str);
    str.replace(QChar::ParagraphSeparator, ' ');
    str.replace(QChar('\r'), ' ');
    str.replace(QChar('\n'), ' ');
    return str;
}

QString extractName(QTextCursor &cursor)
{
    // extract NAME: highlight or word; used for signature-name (symbol)
    int posStart = cursor.selectionStart();
    int posEnd = cursor.selectionEnd();
    if(posStart == posEnd) {
        // if there is no selection, select the word
        cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
    // otherwise specific selection
    QString str = cursor.selectedText();



    qDebug() << "str == "<<str;

    return str;
}

QString extractSuggestedId(QTextCursor &cursor)
{
    // extract the suggested name - the identifier the cursor is on
    int posStart = cursor.selectionStart();
    int posEnd = cursor.selectionEnd();

    cursor.setPosition(posStart, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    // index is the distance from the beginning of the line to the cursor/selection start
    int index = posStart - cursor.selectionStart();
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

    // ALWAYS highlight the entire line
    QString str = cursor.selectedText();



    qDebug() << "str == "<<str;
    if(posStart == posEnd) {
        // if there is no selection - if the index fits into the string, we try to extract the identifier
        if(!extractId(str, index))
            replaceNonId(str);
        // and if it does not fit - and this cannot be
    }
    else {
        // if there is a specific selection - we are trying to build something by replacing
        replaceNonId(str);
    }

    return str;
}

QString extractEditText(QTextCursor &cursor, int &index)
{
    // always the whole line
    int posStart = cursor.selectionStart();
    cursor.setPosition(posStart, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

    QString str = cursor.selectedText();
    index = posStart;
    return str;
}
*/

void editMarkup(QTextCursor &cursor, const QString &str)
{
	// code for modifying the marker comment in the document
    // this is not very correct. indents are not taken into account and the fact that the MK may not be on a new line
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.insertText(str);
}

void makeMarkup(QTextCursor &cursor, const QString &start, const QString &end)
{
	// code for inserting a marker comment into the document
    // mark up the selected area with a start..end metacomment
    // all work through the cursor;
    if(start.isEmpty())
        return;

    qDebug() << "markupSelection: " << start << "," << end;

    int posStart = cursor.selectionStart();
    int posEnd = cursor.selectionEnd();

    // insert at end before at start to prevent shifting text positions
    if(!end.isEmpty()) {
        cursor.setPosition(posEnd, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
        cursor.insertText("\r\n" + end);
    }

    if(!start.isEmpty()) {
        cursor.setPosition(posStart, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        cursor.insertText(start + "\r\n");
    }
}
