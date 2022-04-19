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


#ifndef TEXTEDITWIDGET_H
#define TEXTEDITWIDGET_H

#include <QFrame>
#include <QTextCursor>
#include <QDateTime>
#include <QTextEdit>

class QTextEdit;
class QHBoxLayout;
class Highlighter;
class QTextDocument;

/*!
  The column containing the line numbers.
  @author Alexandru Scvortov <scvalex@gmail.com>
*/
class NumberBar : public QWidget {
    Q_OBJECT

public:
    NumberBar(QWidget *parent);
    ~NumberBar();

    void setTextEdit(QTextEdit *_edit);
    void paintEvent(QPaintEvent *event);

protected:
    bool event(QEvent *event);

    QTextEdit *edit;
    QRect currentRect;
};

/**
  The text editor.
  @author Alexandru Scvortov <scvalex@gmail.com>
*/
class TextEditWidget : public QFrame {
    Q_OBJECT

public:
    TextEditWidget(QWidget *parent = 0);
    ~TextEditWidget();

    QTextEdit* getTextEdit() const;
    QFont* getFont();
    QTextDocument* getDocument();
    Highlighter* getHighlighter() const;

    QString getCurFile() const;
    QString getShownName() const;

    void setCurFile(QString _curFile);
    void setShownName(QString _shownName);

    bool getHighlighting() const;
    void markupSelection(const QString &start, const QString &end);

    bool save(const char *codec = nullptr);
    void load(const char *codec = nullptr);
    bool wasChanged() const;
    bool findFirst(const QString &str);
private:
    //! @internal Used to get tooltip events from the view for the hover signal.
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    //! Toggles syntax highlighting.
    void toggleHighlighting();

    /*!
      Displays any changest to font.
      @sa getFont()
    */
    void updateFont();

    //! Sets the line numbers column visibility to true or false.
    void setLineNumbering(bool _state);

    /*!
      Attempts to move to line lineNo.
      @return True, if move was succesful.
      @return False, otherwise.
    */
    bool moveToLine(int lineNo);

protected slots:
    //! @internal Used to update the highlight on the current line.
    void cursorChanged();

signals:
    /*!
      Emmited when the Highlighter is activated or deactivated.
      @param state The current state of the Highlighter.
    */
    void highlighting(bool state);

    /*!
      Emitted when the mouse is hovered over the text edit component.
      @param word The word under the mouse pointer.
    */
    void mouseHover(const QString &word);

    /*!
      Emmited when the mouse is hovered over the text edit component.
      @param pos The position of the mouse pointer.
      @param word The word under the mouse pointer.
    */
    void mouseHover(const QPoint &pos, const QString &word);

    /*!
      @param line The current line.
      @param column The current character in the current line.
    */
    void cursorPositionChanged(int line, int column);

private:
    QTextEdit *view;
    NumberBar *numbers;
    QHBoxLayout *box;
    int currentLine;

    //! Previous cursor.
    QTextCursor highlight;

    QString curFile;
    QString shownName;

    Highlighter *highlighter;
    bool highlighterOn;

    QFont font;

    QDateTime lastWrite;
};

#endif
