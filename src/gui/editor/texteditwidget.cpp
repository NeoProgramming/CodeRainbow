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

#include <QtGui>
#include <QMessageBox>
#include <QScrollBar>
#include <QLineEdit>
#include <QHBoxLayout>
#include "texteditwidget.h"
#include "highlighter.h"
#include "../../core/core.h"

NumberBar::NumberBar(QWidget *parent) : QWidget(parent), edit(0) {
    setFixedWidth(fontMetrics().width(QString("00") + 3)); // Changed during paintEvent()
}

NumberBar::~NumberBar() {}

void NumberBar::setTextEdit(QTextEdit *_edit)
{
    edit = _edit;
    connect(edit->document()->documentLayout(), SIGNAL(update(const QRectF &)), this, SLOT(update()));
    connect(edit->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
}

void NumberBar::paintEvent(QPaintEvent *)
{
    QAbstractTextDocumentLayout *layout = edit->document()->documentLayout();
    int contentsY = edit->verticalScrollBar()->value();
    qreal pageBottom = contentsY + edit->viewport()->height();
    const QFontMetrics fm = fontMetrics();
    const int ascent = fm.ascent() + 1;
    int lineCount = 1;

    QPainter p(this);

    //TODO This loops around too much for large files. Make it more efficient.
    for (QTextBlock block = edit->document()->begin(); block.isValid(); block = block.next(), ++lineCount)
    {
        const QRectF boundingRect = layout->blockBoundingRect(block);

        QPointF position = boundingRect.topLeft();
        if (position.y() + boundingRect.height() < contentsY)
            continue;
        if (position.y() > pageBottom)
            break;

        const QString txt = QString::number(lineCount);
        p.drawText(width() - fm.width(txt) - 2, qRound(position.y()) - contentsY + ascent, txt);
    }

    const QString txt = QString::number(lineCount);
    if (txt.length() >= 2)
        setFixedWidth(fontMetrics().width(txt) + 3);
    else
        setFixedWidth(fontMetrics().width("00") + 3);
}

bool NumberBar::event(QEvent *event)
{
//   if (event->type() == QEvent::ToolTip) {
//     QHelpEvent *helpEvent = (QHelpEvent*)event;
//
//     if (currentRect.contains(helpEvent->pos()))
//       QToolTip::showText(helpEvent->globalPos(), "Current line");
//   }

    return QWidget::event(event);
}

bool isFixedPitch(const QFont & font) 
{
	const QFontInfo fi(font);
	qDebug() << fi.family() << fi.fixedPitch();
	return fi.fixedPitch();
}

QFont getMonospaceFont()
{
	QFont font("monospace");
	if (isFixedPitch(font)) return font;
	font.setStyleHint(QFont::Monospace);
	if (isFixedPitch(font)) return font;
	font.setStyleHint(QFont::TypeWriter);
	if (isFixedPitch(font)) return font;
	font.setFamily("courier");
	if (isFixedPitch(font)) return font;
	return font;
}

TextEditWidget::TextEditWidget(QWidget *parent)
    : QFrame(parent)
    , curFile("")
    , shownName("")
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(2);

    // Setup the main QTextEdit
    view = new QTextEdit(this);

    view->setTabStopWidth(fontMetrics().width(QString("W")) * 4);
    view->setAcceptRichText(false);

	font = getMonospaceFont();
    font.setPointSize(12);
    view->setFont(font);

    view->setLineWrapMode(QTextEdit::NoWrap);
    view->setFrameStyle(QFrame::NoFrame);
    view->installEventFilter(this);

    connect(view, SIGNAL(cursorPositionChanged()), this, SLOT(cursorChanged()));

    // Setup the line number pane
    numbers = new NumberBar(this);
    numbers->setTextEdit(view);

    box = new QHBoxLayout(this);
    box->setSpacing(0);
    box->setMargin(0);
    box->addWidget(numbers);
    box->addWidget(view);

    highlighter = new Highlighter(view->document());
    highlighterOn = true;

    cursorChanged();
}

TextEditWidget::~TextEditWidget() {}

QTextEdit* TextEditWidget::getTextEdit() const
{
    return view;
}

void TextEditWidget::toggleHighlighting()
{
    if (highlighterOn) {
        highlighter->setDocument((QTextDocument *)0);
        highlighterOn = false;
    } else {
        highlighter->setDocument(view->document());
        highlighterOn = true;
    }

    emit highlighting(highlighterOn);
}

void TextEditWidget::updateFont()
{
    view->setFont(font);
}

void TextEditWidget::setLineNumbering(bool _state)
{
    numbers->setVisible(_state);
}

bool TextEditWidget::moveToLine(int lineNo)
{
    //TODO Get this to work
    return true;
    if ((lineNo < 0) || (lineNo > getDocument()->blockCount()))
        return false;

    if (lineNo > view->textCursor().blockNumber())
        return view->textCursor().movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineNo - view->textCursor().blockNumber());
    else
        return view->textCursor().movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, view->textCursor().blockNumber() - lineNo);
}

QString TextEditWidget::getCurFile() const
{
    return curFile;
}

QString TextEditWidget::getShownName() const
{
    return shownName;
}

bool TextEditWidget::getHighlighting() const
{
    return highlighterOn;
}

void TextEditWidget::setCurFile(QString _curFile) {
    curFile = _curFile;

    lastWrite = QFileInfo(curFile).lastModified();
}

void TextEditWidget::setShownName(QString _shownName) {
    shownName = _shownName;
}

Highlighter* TextEditWidget::getHighlighter() const {
    return highlighter;
}

QFont* TextEditWidget::getFont() {
    /*!
      @return The font used for the text.
      @note No changes to font are displayed until updateFont() is called.
      @sa updateFont()
    */
    return &font;
}

void TextEditWidget::cursorChanged()
{
    //TODO Implement current line highlighting with extra selections.

    currentLine = view->textCursor().blockNumber() + 1;

    emit cursorPositionChanged(currentLine, view->textCursor().columnNumber() + 1);

    QTextBlock block = view->textCursor().block();
    QTextCharFormat fmt = block.charFormat();
    QColor bg = view->palette().base().color();
    fmt.setBackground(bg);

    static QList<QTextEdit::ExtraSelection> selections;

    selections.clear();

    QTextEdit::ExtraSelection sel;
    sel.cursor = view->textCursor();
    sel.format = fmt;

    selections << sel;

    view->setExtraSelections(selections); //FIXME Doesn't work.

//     static QTextBlock cblock;
//     static int lineCount;
//
//     if ((currentLine == lineCount) && (currentLine != 1))
//         return;
//
//     //TODO Fix erronous delete row after bug.
//     //TODO Optimize.
//
//     bool mod = getDocument()->isModified();
//
//     QTextBlock block = highlight.block();
//     QTextBlockFormat fmt = block.blockFormat();
//     QColor bg = view->palette().base().color();
//     fmt.setBackground(bg);
//     highlight.setBlockFormat(fmt);
//
// //   if (cblock.isValid()) {
// //     qWarning("Previous block is valid. Attempting to remove highlight");
// //     highlight = QTextCursor(cblock);
// //     highlight.setBlockFormat(fmt);
// //     qWarning("Highlight removed");
// //   }
//
//     lineCount = 1;
//     for (cblock = view->document()->begin(); cblock.isValid(); cblock = cblock.next(), ++lineCount)
//         if (lineCount == currentLine) {
//             fmt = cblock.blockFormat();
//             QColor bg = QColor(192, 192, 192, 100);
//             fmt.setBackground(bg);
//
//             highlight = QTextCursor(cblock);
//             highlight.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
//             highlight.setBlockFormat(fmt);
//
//             break;
//         } else {
//             fmt = cblock.blockFormat();
//             QColor bg = view->palette().base().color();
//             fmt.setBackground(bg);
//
//             highlight = QTextCursor(cblock); // I'll go to Hell for this.
//             highlight.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
//             highlight.setBlockFormat(fmt);
//         }
//
//     getDocument()->setModified(mod);
}

bool TextEditWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj != view)
        return QFrame::eventFilter(obj, event);

    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = (QHelpEvent*)event;

        QTextCursor cursor = view->cursorForPosition(helpEvent->pos());
        cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

        QString word = cursor.selectedText();
        emit mouseHover(word);
        emit mouseHover(helpEvent->pos(), word);

//     QToolTip::showText(helpEvent->globalPos(), word);
    }

    return false;
}

QTextDocument* TextEditWidget::getDocument() {
    return view->document();
}

bool TextEditWidget::save(const char *codec) {
    /*!
      Saves the current document to the specifed file.
      @return True if save succesful, false otherwise.
    */

    QFile file(curFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(AppName),
                             tr("Cannot write file %1:\n%2.")
                             .arg(curFile)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    if(codec)
        out.setCodec(codec);
    out << view->toPlainText();

    lastWrite = QDateTime::currentDateTime();

    return true;
}

void TextEditWidget::load(const char *codec) {
    QFile file(curFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr(AppName),
                             tr("Cannot read file %1:\n%2.")
                             .arg(curFile)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    if(codec)
        in.setCodec(codec);
    view->setPlainText(in.readAll());

    lastWrite = QFileInfo(curFile).lastModified();
}

bool TextEditWidget::wasChanged() const
{
    //! @return True if the document was modified on file. False otherwise.
    if (curFile.isEmpty())
        return false;

    if (lastWrite.isValid() && (lastWrite < QFileInfo(curFile).lastModified()))
        return true;

    return false;
}

void TextEditWidget::markupSelection(const QString &start, const QString &end)
{
    // mark the selected area with start..end metacomment

    QTextCursor cursor = getTextEdit()->textCursor();

    // further all work through the cursor
    int posStart = cursor.selectionStart();
    int posEnd = cursor.selectionEnd();

    cursor.setPosition(posStart);
    int firstLine = cursor.blockNumber();
    cursor.setPosition(posEnd, QTextCursor::KeepAnchor);
    int lastLine = cursor.blockNumber();

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
	
    // reset the selection, it is no longer needed
    cursor.clearSelection();
    getTextEdit()->setTextCursor(cursor);
}

bool TextEditWidget::findFirst(const QString &str)
{
    return view->find(str, QTextDocument::FindCaseSensitively);
}

