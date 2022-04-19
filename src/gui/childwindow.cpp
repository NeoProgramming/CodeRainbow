#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include "childwindow.h"
#include "../core/core.h"
#include "../core/crtree.h"
#include "../core/mcmt.h"
#include "mainwindow.h"
#include "ctrlpanel.h"


ChildWindow::ChildWindow(MainWindow *mw, CRNode* idn)
	: QWidget()
    , markMaker(idn, mw)
{
	setAttribute(Qt::WA_DeleteOnClose);
    wndMain = mw;
//    m_nodeFile = idn;

	m_pEdit = new TextEditWidget(this);
	m_pEdit->show();
    loadFile(markMaker.m_nodeFile->path);

	connect(m_pEdit->getDocument(), SIGNAL(modificationChanged(bool)), this, SLOT(onDocumentWasModified(bool)));
    connect(&markMaker, SIGNAL(treeItemAdded(CRNode*,CRNode*)), this, SLOT(onTreeItemAdded(CRNode*,CRNode*)));
}

ChildWindow::~ChildWindow()
{
}

void ChildWindow::init(QMdiSubWindow *subwnd)
{
    wndSub = subwnd;

	if (curFile.isEmpty())
		shownName = "untitled.txt";
	else
        shownName = QFileInfo(curFile).fileName();

	if (isWindowModified())
        wndSub->setWindowTitle(tr("%1*").arg(shownName));
	else
        wndSub->setWindowTitle(shownName);

	m_pEdit->setShownName(shownName);
}

void ChildWindow::onTreeItemAdded(CRNode *nodeActive, CRNode*node)
{
    wndMain->panelCtrl->addTreeItem(nodeActive->item, node);
}

void ChildWindow::findMcmt(const QString &mcmt)
{
    m_pEdit->findFirst(mcmt);
}

void ChildWindow::moveEvent ( QMoveEvent * event ) 
{
	if(m_pEdit)
        m_pEdit->move(0, 0);
}

void ChildWindow::resizeEvent ( QResizeEvent * event )
{
	if(m_pEdit)
        m_pEdit->resize(event->size().width(), event->size().height());
}

void ChildWindow::closeEvent( QCloseEvent * evt )
{
    if(m_pEdit->getDocument()->isModified()) {
        QMessageBox::StandardButton r = QMessageBox::question(this, AppName, "Save changes ?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
		if(r == QMessageBox::Cancel)
			return;
		if(r == QMessageBox::Yes)
			save();
	}
}

void ChildWindow::onDocumentWasModified(bool changed)
{
    setWindowModified(changed);
	m_pEdit->getDocument()->setModified(changed); //NOTE This shouldn't do anything, but it does.
	if (changed)
        wndSub->setWindowTitle(tr("%1*").arg(m_pEdit->getShownName()));
	else
        wndSub->setWindowTitle(tr("%1").arg(m_pEdit->getShownName()));
}

bool ChildWindow::save()
{
	if (m_pEdit->getCurFile().isEmpty())
		return saveAs();
	else
		return saveFile(curFile);
}

bool ChildWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, "NeoIDE");//, progName);
	if (fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

void ChildWindow::loadFile(const QString &fileName) 
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	m_pEdit->setCurFile(fileName);
    m_pEdit->load();

	QApplication::restoreOverrideCursor();

	curFile = fileName;
}

bool ChildWindow::saveFile(const QString &fileName) 
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	m_pEdit->setCurFile(fileName);
    bool ret = m_pEdit->save();

	QApplication::restoreOverrideCursor();

	if (!ret)
		return false;

	return true;
}

void ChildWindow::markMcmt(int ri, CR::Type type)
{
    // universal wrapper for calling from MainWidow; here we get the cursor
    QTextCursor cursor = m_pEdit->getTextEdit()->textCursor();
    markMaker.mark(ri, type, cursor);
}

void ChildWindow::navPrev()
{

}

void ChildWindow::navNext()
{

}

void ChildWindow::navLocate()
{

}

