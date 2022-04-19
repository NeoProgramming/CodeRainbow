#include "mainwindow.h"

#include <QDir>
#include <QDockWidget>
#include <QEventLoop>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QStatusBar>
#include <QTextObject>
#include <QTimer>
#include <QtEvents>
#include <QFontDatabase>
#include <QToolButton>
#include <QSettings>
#include <QDesktopServices>
#include <QBitmap>
#include <QDebug>

#include "ctrlpanel.h"
#include "infopanel.h"
#include "dialogs/newprojectdlg.h"
#include "dialogs/prjpropsdlg.h"
#include "../tools/tfbase.h"
#include "../core/core.h"
#include "../core/crtree.h"
#include "../core/nlexer.h"
#include "../core/kw.h"


#if defined(Q_WS_WIN)
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

MainWindow::MainWindow()
{
	setUnifiedTitleAndToolBarOnMac(true);
    ui.setupUi(this);

#if defined(Q_WS_WIN)
    // Workaround for QMimeSourceFactory failing in QFileInfo::isReadable() for
    // certain user configs. See task: 34372
    qt_ntfs_permission_lookup = 0;
#endif
  
    iconApp = QIcon(":/res/cr.png");
    QApplication::setWindowIcon(iconApp);

	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	readSettings();

    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setViewMode(QMdiArea::TabbedView);
    mdiArea->setTabsClosable(true);
    mdiArea->setTabsMovable(true);

    QList<QTabBar *> tabBarList = mdiArea->findChildren<QTabBar*>();
	QTabBar *tabBar = tabBarList.at(0);
	if (tabBar) 
		tabBar->setExpanding(false);

    setCentralWidget(mdiArea);

    connect(ui.actionWindowClose,	SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));
    connect(ui.actionWindowCloseAll,SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));
    connect(ui.actionWindowTile,	SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));
    connect(ui.actionWindowCascade, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));

	connect(ui.actionToolsTest, SIGNAL(triggered()), this, SLOT(onToolsTest()));

    dockInfo = new QDockWidget(this);
    dockInfo->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dockInfo->setWindowTitle(tr("Output"));
    dockInfo->setObjectName(QLatin1String("msgbar"));
    panelInfo = new InfoPanel(dockInfo, this);
    dockInfo->setWidget(panelInfo);
    addDockWidget(Qt::BottomDockWidgetArea, dockInfo);

    dockSide = new QDockWidget(this);
    dockSide->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockSide->setWindowTitle(tr("Solution explorer"));
    dockSide->setObjectName(QLatin1String("slnbar"));
    panelCtrl = new CtrlPanel(dockSide, this);
    dockSide->setWidget(panelCtrl);
    addDockWidget(Qt::LeftDockWidgetArea, dockSide);


    connect(ui.actionProjectNew,    SIGNAL(triggered()), this, SLOT(onProjectNew()));
    connect(ui.actionProjectOpen,	SIGNAL(triggered()), this, SLOT(onProjectOpen()));
    connect(ui.actionProjectSave,	SIGNAL(triggered()), this, SLOT(onProjectSave()));
    connect(ui.actionProjectSaveAs,	SIGNAL(triggered()), this, SLOT(onProjectSaveAs()));
    connect(ui.actionProjectProps,	SIGNAL(triggered()), this, SLOT(onProjectProperties()));

    connect(ui.actionFileSave,      SIGNAL(triggered()), this, SLOT(onFileSave()));
    connect(ui.actionFileSaveAll,	SIGNAL(triggered()), this, SLOT(onFileSaveAll()));

	connect(ui.actionEditCut,		SIGNAL(triggered()), this, SLOT(onEditCut()));
	connect(ui.actionEditCopy,		SIGNAL(triggered()), this, SLOT(onEditCopy()));
	connect(ui.actionEditPaste,		SIGNAL(triggered()), this, SLOT(onEditPaste()));
	connect(ui.actionEditUndo,		SIGNAL(triggered()), this, SLOT(onEditUndo()));
	connect(ui.actionEditRedo,		SIGNAL(triggered()), this, SLOT(onEditRedo()));
	connect(ui.actionEditSelectAll,	SIGNAL(triggered()), this, SLOT(onEditSelectAll()));

    connect(ui.actionTreeUp,        SIGNAL(triggered()), this, SLOT(onNodeMoveUp()));
    connect(ui.actionTreeDown,      SIGNAL(triggered()), this, SLOT(onNodeMoveDown()));
    connect(ui.actionTreeLeft,      SIGNAL(triggered()), this, SLOT(onNodeMoveLeft()));
    connect(ui.actionTreeRight,     SIGNAL(triggered()), this, SLOT(onNodeMoveRight()));

    connect(ui.actionNavPrev,       SIGNAL(triggered()), this, SLOT(onNavPrev()));
    connect(ui.actionNavNext,       SIGNAL(triggered()), this, SLOT(onNavNext()));
    connect(ui.actionNavLocate,     SIGNAL(triggered()), this, SLOT(onNavLocate()));

    connect(ui.actionAppExit,		SIGNAL(triggered()), this, SLOT(onAppExit()));
    connect(ui.actionAppSettings,	SIGNAL(triggered()), this, SLOT(onAppSettings()));

	QAction *viewsAction = createPopupMenu()->menuAction();
	viewsAction->setText(tr("Toolbars & panels"));
    ui.menuView->addAction(viewsAction);

	ui.actionEditFind->setShortcut(QKeySequence::Find);
	ui.actionEditFindNext->setShortcut(QKeySequence::FindNext);
	ui.actionEditFindPrev->setShortcut(QKeySequence::FindPrevious);
	
    panelCtrl->initialize();
    panelInfo->initialize();
    QTimer::singleShot(0, this, SLOT(onInit()));
}

void MainWindow::closeEvent(QCloseEvent *event) 
{
    if( isModified() ) {
        int ret = QMessageBox::warning(this, tr(AppName),
            tr("Project was modified. Save changes before quit?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
        if(ret == QMessageBox::Yes) {
            if(pCR->modify)
                onProjectSave();
            onFileSaveAll();
        }
    }

	writeSettings();
	event->accept();
}

void MainWindow::readSettings() 
{
    QSettings settings("NeoProgramming", "CodeRainbow");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
    pCR->pathRecentProject = settings.value("recentProject", "").toString();
	
	resize(size);
	move(pos);
}

void MainWindow::writeSettings() 
{
    QSettings settings("NeoProgramming", "CodeRainbow");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
    settings.setValue("recentProject", pCR->pathRecentProject);
}


void MainWindow::onInit()
{
    loadProject(pCR->pathRecentProject);

    Gui::createMultiTool(ui.toolBarMarkup, &pCR->markButtons[MarkBtn::Idx(CRNode::NT_AREA)], this,
            ":/mark/res/mark-area.png", tr("Area"),
            SLOT(onMarkAreaNew()), SLOT(onMarkAreaMenu(int)), SLOT(onMarkAreaClicked()));
    Gui::createMultiTool(ui.toolBarMarkup, &pCR->markButtons[MarkBtn::Idx(CRNode::NT_LABEL)], this,
            ":/mark/res/mark-label.png", tr("Label"),
            SLOT(onMarkLabelNew()), SLOT(onMarkLabelMenu(int)), SLOT(onMarkLabelClicked()));
    Gui::createMultiTool(ui.toolBarMarkup, &pCR->markButtons[MarkBtn::Idx(CRNode::NT_BLOCK)], this,
            ":/mark/res/mark-block.png",tr("Block"),
            SLOT(onMarkAreasNew()), SLOT(onMarkAreasMenu(int)),SLOT(onMarkAreasClicked()));
    Gui::createMultiTool(ui.toolBarMarkup, &pCR->markButtons[MarkBtn::Idx(CRNode::NT_LSIG)], this,
            ":/mark/res/mark-lsig.png", tr("Label Signature"),
            SLOT(onMarkLSigNew()), SLOT(onMarkLSigMenu(int)), SLOT(onMarkLSigClicked()));
    Gui::createMultiTool(ui.toolBarMarkup, &pCR->markButtons[MarkBtn::Idx(CRNode::NT_BSIG)], this,
            ":/mark/res/mark-bsig.png", tr("Block Signature"),
            SLOT(onMarkBSigNew()), SLOT(onMarkBSigMenu(int)), SLOT(onMarkBSigClicked()));
}
  
MainWindow::~MainWindow()
{

}

void MainWindow::onAppAbout()
{
    QMessageBox box(this);
    box.setText(QString::fromLatin1("<p align=\"center\"><h3>CodeRainbow</h3></p>"
                                    "<p align=\"center\">version 0.1</p>"
                                    "<p>CodeRainbow</p>"
                                    "<p>Copyright (C) 2018</p>"));
    box.setWindowTitle(tr("About CodeRainbow 0.1"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}

void MainWindow::onAppExit()
{
	qApp->closeAllWindows();	
}

void MainWindow::onProjectNew()
{
    // select project source: file or dir
    NewProjectDlg dlg;
    if(dlg.execDlg()) {
        pCR->create(dlg.m_name, dlg.m_base, dlg.m_filters);
        panelCtrl->loadTree();
        setCurrentProject("");
    }
}

void MainWindow::onProjectOpen()
{
    QString filter = AppFileFilter;
    QString path = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "",
                filter,
                &filter);
    if(!path.isEmpty())	{
        loadProject(path);
	}
}

void MainWindow::onProjectSave()
{
    if(!pCR->save(pCR->pathRecentProject))
        onProjectSaveAs();
}

void MainWindow::onProjectSaveAs()
{
    QString filter = AppFileFilter;
    QString path = QFileDialog::getSaveFileName(
                this,
                tr("Save File As"),
                pCR->pathRecentProject,
                filter,
                &filter);
    if(!path.isEmpty()) {
        if (!path.endsWith(AppFileExt))
            path += AppFileExt;
        if(!pCR->save(path))
            QMessageBox::warning(this, "Save error", "Save error", QMessageBox::Ok);
        else
            setCurrentProject(path);
    }
}

void MainWindow::onFileSave()
{
    // save current opened file
}

void MainWindow::onFileSaveAll()
{
    // save all opened files
    QList<QMdiSubWindow *> wl = mdiArea->subWindowList();
    QList<QMdiSubWindow *>::Iterator i = wl.begin(), e = wl.end();
    while(i != e) {
		QMdiSubWindow *subwnd = *i;
		ChildWindow *view = qobject_cast<ChildWindow *>(subwnd->widget());
        if(view->isWindowModified()) {
			view->save();
			view->onDocumentWasModified(false);
		}
		++i;
	}
}

void MainWindow::onProjectProperties()
{
    PrjPropsDlg dlg;
    dlg.pp = pCR->pp;
    if(dlg.execDlg())
        pCR->pp = dlg.pp;
}

void MainWindow::onAppSettings()
{

}

bool MainWindow::isModified()
{
    if(pCR->modify)
        return true;
    QList<QMdiSubWindow *> wl = mdiArea->subWindowList();
    QList<QMdiSubWindow *>::Iterator i = wl.begin(), e = wl.end();
    while(i != e) {
        QMdiSubWindow *subwnd = *i;
        ChildWindow *view = qobject_cast<ChildWindow *>(subwnd->widget());
        if(view && view->isWindowModified())
            return true;
        ++i;
    }
    return false;
}

void MainWindow::loadProject(const QString& path)
{
    if( pCR->load(path) ) {
        panelCtrl->loadTree();
        setCurrentProject(path);
    }
}

void MainWindow::setCurrentProject(const QString &path)
{
    pCR->pathRecentProject = path;
    if(pCR->pathRecentProject.isEmpty())
        setWindowTitle("<new project> - CodeRainbow");
    else
        setWindowTitle(path + " - CodeRainbow");
}

void MainWindow::selectNode(CRNode *node)
{
    panelInfo->loadText(node);
}

void MainWindow::openDoc(CRNode *node, const QString &mcmt)
{
    if(!QFileInfo::exists(node->path))
        return (void)QMessageBox::warning(this, AppName, "Doc path not exists", QMessageBox::Ok);
    qApp->setOverrideCursor(Qt::WaitCursor);
    ChildWindow* child = openExistingDoc(node);
    if(!child)
        child = openNewDoc(node);
    qApp->restoreOverrideCursor();
    if(!child)
        return (void)QMessageBox::warning(this, AppName, "Doc open error", QMessageBox::Ok);
    else if(!mcmt.isEmpty())
        child->findMcmt(mcmt);
}

void MainWindow::openNode(CRNode *node)
{
    if(!node)
		return;
    else if(node->type == CRNode::NT_BASE || node->type == CRNode::NT_DIR || node->type == CRNode::NT_GROUP)
        return;

    if(node->type == CRNode::NT_FILE) {
        openDoc(node);
    }
    else if(node->type == CRNode::NT_LINK) {
        QDesktopServices::openUrl(QUrl(node->path));
    }
    else if(CRNode::isMark(node->type)) {
        CRNode *fnode = node->getFile();
        openDoc(fnode, node->path);
    }
}

ChildWindow* MainWindow::openExistingDoc(CRNode *node)
{
    QList<QMdiSubWindow *> wl = mdiArea->subWindowList();
	QList<QMdiSubWindow *>::Iterator i = wl.begin();
	QList<QMdiSubWindow *>::Iterator e = wl.end();
    while(i != e) {
		QMdiSubWindow *subwnd = *i;
		ChildWindow *view = qobject_cast<ChildWindow *>(subwnd->widget());
        if(view->markMaker.m_nodeFile == node) {
            mdiArea->setActiveSubWindow(subwnd);
			return view;
		}
		++i;
	}
	return 0;
}

ChildWindow * MainWindow::openNewDoc(CRNode *node)
{
    ChildWindow *child = new ChildWindow(this, node);
    QMdiSubWindow *subwnd = mdiArea->addSubWindow(child);
    child->init(subwnd);
	child->show();
	return child;
}

ChildWindow *MainWindow::getActiveDoc()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
		return qobject_cast<ChildWindow *>(activeSubWindow->widget());
	return 0;
}

void MainWindow::closeWindows(CRNode* node)
{
    QList<QMdiSubWindow *> wl = mdiArea->subWindowList();
    QList<QMdiSubWindow *>::Iterator i = wl.begin(), e = wl.end();
    while(i != e) {
		QMdiSubWindow *subwnd = *i;
		ChildWindow *view = qobject_cast<ChildWindow *>(subwnd->widget());
        if(view) {
 			view->close();
 		}
		++i;
	}
}





void MainWindow::onEditCut()
{
    ChildWindow *wnd = getActiveDoc();
 	if(wnd)
 		wnd->m_pEdit->getTextEdit()->cut();
}

void MainWindow::onEditCopy()
{
    ChildWindow *wnd = getActiveDoc();
	if(wnd)
		wnd->m_pEdit->getTextEdit()->copy();
}

void MainWindow::onEditPaste()
{
    ChildWindow *wnd = getActiveDoc();
	if(wnd)
		wnd->m_pEdit->getTextEdit()->paste();
}

void MainWindow::onEditUndo()
{
    ChildWindow *wnd = getActiveDoc();
	if(wnd)
		wnd->m_pEdit->getTextEdit()->undo();
}

void MainWindow::onEditRedo()
{
    ChildWindow *wnd = getActiveDoc();
	if(wnd)
		wnd->m_pEdit->getTextEdit()->redo();
}

void MainWindow::onEditSelectAll()
{
    ChildWindow *wnd = getActiveDoc();
	if(wnd)
		wnd->m_pEdit->getTextEdit()->selectAll();
}

void MainWindow::closeDocWindow()
{
    mdiArea->closeActiveSubWindow();
}

void MainWindow::onMarkAreaMenu(int i)
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(i, CRNode::NT_AREA);
}

void MainWindow::onMarkLabelMenu(int i)
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(i, CRNode::NT_LABEL);
}

void MainWindow::onMarkBlockMenu(int i)
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(i, CRNode::NT_BLOCK);
}

void MainWindow::onMarkLSigMenu(int i)
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(i, CRNode::NT_LSIG);
}

void MainWindow::onMarkBSigMenu(int i)
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(i, CRNode::NT_BSIG);
}

void MainWindow::onMarkTagMenu(int i)
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(i, CRNode::NT_TAG);
}

void MainWindow::onMarkAreaNew()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(-1, CRNode::NT_AREA);
}

void MainWindow::onMarkLabelNew()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(-1, CRNode::NT_LABEL);
}

void MainWindow::onMarkBlockNew()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(-1, CRNode::NT_BLOCK);
}

void MainWindow::onMarkLSigNew()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(-1, CRNode::NT_LSIG);
}

void MainWindow::onMarkBSigNew()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(-1, CRNode::NT_BSIG);
}

void MainWindow::onMarkTagNew()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(-1, CRNode::NT_TAG);
}


void MainWindow::onMarkAreaClicked()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(0, CRNode::NT_AREA);
}

void MainWindow::onMarkLabelClicked()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(0, CRNode::NT_LABEL);
}

void MainWindow::onMarkBlockClicked()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(0, CRNode::NT_BLOCK);
}

void MainWindow::onMarkLSigClicked()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(0, CRNode::NT_LSIG);
}

void MainWindow::onMarkBSigClicked()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(0, CRNode::NT_BSIG);
}

void MainWindow::onMarkTagClicked()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->markMcmt(0, CRNode::NT_TAG);
}

void MainWindow::onNavPrev()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->navPrev();
}

void MainWindow::onNavNext()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->navNext();
}

void MainWindow::onNavLocate()
{
    ChildWindow *wnd = getActiveDoc();
    if(wnd)
        wnd->navLocate();
}

void MainWindow::onNodeMoveUp()
{
    panelCtrl->moveItemUp();
}

void MainWindow::onNodeMoveDown()
{
    panelCtrl->moveItemDown();
}

void MainWindow::onNodeMoveLeft()
{
    panelCtrl->moveItemLeft();
}

void MainWindow::onNodeMoveRight()
{
    panelCtrl->moveItemRight();
}

void MainWindow::onToolsTest()
{
	extern NKeyWord KW[];

	QString path = QFileDialog::getOpenFileName(this, tr("Select file"), "", "*.*");
	if(!path.isEmpty()) {
		FILE *f = fopen(path.toLocal8Bit().constData(), "rb");
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *buf = new char[size+2];
		memset(buf, 0, size+2);
		fread(buf, size, 1, f);
		fclose(f);


		// test
		int i=0;
		// lexical analyzer
		NLexer<Utf8> lexer(buf, size);
		NLexer<Utf8>::token_t tok;
		lexer.skipLineCmts = false;
		lexer.initOperators(&KW[0], &KW[1], TOKlbrace);

		// scan loop
		while(lexer.Scan(&tok)) {
			i++;

			if(tok.value == TOKlinecmt) {
				QByteArray bt(tok.ptr, tok.length);
				qDebug() << "//" << tok.loc.linnum << "," << tok.loc.charnum << "s=" << tok.length << "v=" << bt;
			}
            
		}

		delete[] buf;
	}
}

