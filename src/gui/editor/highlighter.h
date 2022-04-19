#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include "../../core/token.h"


class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightMultilineCmt(const QString &text, int prev_state, int &curr_state);
    void highlightRainbowCmt(const QString &text, int prev_state, int &curr_state);
    void highlightLineSyntax(const QString &text, int prev_state, int &curr_state);
    void highlightBlock(const QString &text);
    void setBkColor(QTextBlockFormat &bf, int state);
	
    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat fmt[TOKCOUNT];
};

#endif
