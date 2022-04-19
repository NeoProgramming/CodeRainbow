#pragma once
#include <QString>
#include <QTextCursor>
#include "../tools/tdefs.h"

QString extractCaretLine(QTextCursor &cursor, int *pcur);
QString extractLine(QTextCursor &cursor, int pos);

void makeMarkup(QTextCursor &cursor, const QString &start, const QString &end);
void editMarkup(QTextCursor &cursor, const QString &str);

//QString operator=(const String &str);
//String operator=(const QString &str);

/*
QString extractName(QTextCursor &cursor);
QString extractSignature(QTextCursor &cursor);
QString extractSuggestedId(QTextCursor &cursor);
QString extractEditText(QTextCursor &cursor, int &index);
*/


