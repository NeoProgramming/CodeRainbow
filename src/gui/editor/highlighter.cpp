/***************************************************************************
 *   Copyright (C) 2007 by Alexandru Scvortov   *
 *   scvalex@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "highlighter.h"
#include <QtDebug>

#include "../../tools/tfbase.h"
#include "../../core/nlexer.h"
#include "../../core/kw.h"
#include "../../core/mcmt.h"

namespace mcmt {

enum BlockState {
    BS_MLCMT	= 0x80000000,	// multiline comment
    BS_CLRVALID = 0x40000000,	// replace to special unique color code

    BS_CLRMASK  = 0x00ffffff,	// color mask

    BP_BRCCOUNT = 24,			// brace count position in state
    BS_BRCMASK  = 0x3f << BP_BRCCOUNT,	// brace count mask
};
}

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
	// operators
    fmt[TOKoperator].setForeground(QColor(63, 72, 204));
    fmt[TOKoperator].setFontWeight(QFont::Bold);

	// keywords & language types
    fmt[TOKkeyword].setForeground(QColor(0x00, 0x00, 0xff));
    fmt[TOKkeyword].setFontWeight(QFont::Bold);

    // preprocessor
    fmt[TOKpreproc].setForeground(QColor(0xff, 0x00, 0xff));
    fmt[TOKpreproc].setFontWeight(QFont::Bold);
    
	// single quotation
    fmt[TOKsqstr].setForeground(QColor(255, 94, 174));

    // double quotation
    fmt[TOKdqstr].setForeground(QColor(0xcc, 0x87, 0x45));

    // numbers
    fmt[TOKnumber].setForeground(Qt::darkBlue);

    // single line comments
    fmt[TOKlinecmt].setForeground(Qt::darkGray);
    fmt[TOKlinecmt].setFontItalic(true);

    // multi line comments
    fmt[TOKblockcmt].setForeground(Qt::gray);
    fmt[TOKblockcmt].setFontItalic(true);
}

// called from editor automatically if block changed
void Highlighter::highlightBlock(const QString &text)
{
    int prev_state = ~previousBlockState();
    int curr_state = 0;
	
	highlightLineSyntax(text, prev_state, curr_state);
    highlightRainbowCmt(text, prev_state, curr_state);

    // update block state; if state changed, highlighter will be called for next line
    setCurrentBlockState(~curr_state);
}

void Highlighter::highlightLineSyntax(const QString &text, int prev_state, int &curr_state)
{
	// simple line syntax analyse with counting curve braces
    int brc = 0;//@ (prev_state & mcmt::BS_BRCMASK) >> mcmt::BP_BRCCOUNT;
	extern NKeyWord KW[];

	const ushort *beg = text.utf16();
	int size = text.size();
    NLexer<Utf16> lexer(beg, size);
    NLexer<Utf16>::token_t tok;
	lexer.skipLineCmts = false;
	lexer.blockCmt = (prev_state & mcmt::BS_MLCMT) ? true : false;
	lexer.initOperators(&KW[0], &KW[1], TOKlbrace);
	
    while(lexer.Scan(&tok)) {
	//	setFormat(tok.ptr-beg, tok.size, fmt[tok.value]);

		QTextCharFormat bc;
		bc.setBackground(QColor(255,brc*36,255));
        setFormat(tok.ptr-beg, tok.length, bc);
	}
	
	curr_state &= mcmt::BS_BRCMASK;
	curr_state |= (brc << mcmt::BP_BRCCOUNT);
	if(lexer.blockCmt)
		curr_state |= mcmt::BS_MLCMT;
	else
		curr_state &= ~mcmt::BS_MLCMT;
}

void Highlighter::highlightMultilineCmt(const QString &text, int prev_state, int &curr_state)
{
    // if no previous comments
    int startIndex = 0;
    if (!(prev_state & mcmt::BS_MLCMT))
        startIndex = text.indexOf(commentStartExpression);

    // if(and while) start comment found
    while (startIndex >= 0) {
        int endIndex = text.indexOf(commentEndExpression, startIndex);
        int commentLength;
        if (endIndex == -1) {
            // start without end
            curr_state |= mcmt::BS_MLCMT;
            commentLength = text.length() - startIndex;
        } else {
            // start with end - all comment in line
            commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
        }
        // mark comment
//@        setFormat(startIndex, commentLength, multiLineCommentFormat);
        // next search
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

void Highlighter::highlightRainbowCmt(const QString &text, int prev_state, int &curr_state)
{
    QTextBlock block = currentBlock();
    int num = block.blockNumber();
    QTextCursor cursor(block);
    QTextBlockFormat bf = block.blockFormat();

    CRInfo info;
    mcmt::MarkType mt = parseLine(text, info);

    if(mt == mcmt::MT_LABEL) {
        // new single-line label
        bf.setBackground(QBrush(QColor(info.clr | 0xff000000)));
    }
    else if(mt == mcmt::MT_AREA_START) {
        // begin of multiline area;
        curr_state &= ~mcmt::BS_CLRMASK;
        curr_state |= info.clr;
        curr_state |= mcmt::BS_CLRVALID;
        setBkColor(bf, curr_state);
    }
    else if(mt == mcmt::MT_AREA_END) {
        // end of multiline area; calculate color for next lines
        curr_state &= ~mcmt::BS_CLRVALID;
        setBkColor(bf, prev_state);
    }
    else {
        // middle of multiline area; we use color of previous line
        curr_state &= ~(mcmt::BS_CLRMASK | mcmt::BS_CLRVALID);
		curr_state |= (prev_state & (mcmt::BS_CLRMASK | mcmt::BS_CLRVALID));
        setBkColor(bf, prev_state);
    }

    cursor.setBlockFormat(bf);
}

void Highlighter::setBkColor(QTextBlockFormat &bf, int state)
{
    if(state & mcmt::BS_CLRVALID)
        bf.setBackground(QBrush(QColor(state & mcmt::BS_CLRMASK)));
    else
        bf.clearBackground();
}

