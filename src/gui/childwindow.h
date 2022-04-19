#pragma once
#include <QMdiSubWindow>
#include <QTreeWidget>
#include <QToolBar>
#include "editor/texteditwidget.h"
#include "dialogs/markupdlg.h"
#include "../core/crnode.h"
#include "markmaker.h"

class MainWindow;

class ChildWindow : public QWidget
{
	Q_OBJECT

public:
    ChildWindow(MainWindow *mw, CRNode* idn);
	virtual ~ChildWindow();
    void init(QMdiSubWindow *subwnd);
    void findMcmt(const QString &mcmt);
	bool save();
	bool saveAs();
	void loadFile(const QString &fileName);
	bool saveFile(const QString &fileName);

    void markMcmt(int i, CR::Type type);

    void navPrev();
    void navNext();
    void navLocate();
public slots:
	void onDocumentWasModified(bool changed);
    void onTreeItemAdded(CRNode *act, CRNode *node);
public:
//    CRNode* m_nodeFile;
    TextEditWidget *m_pEdit;
    MarkMaker markMaker;

protected:

	void moveEvent ( QMoveEvent * evt );
	void resizeEvent ( QResizeEvent * evt );
	void closeEvent( QCloseEvent * evt );

    MainWindow *wndMain;
    QMdiSubWindow *wndSub;
    QString curFile;
    QString shownName;

};


