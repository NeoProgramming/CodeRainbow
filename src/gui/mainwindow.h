#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "childwindow.h"
#include "tabwidget.h"
#include "gui.h"
#include "editor/texteditwidget.h"
#include <QTreeWidget>
#include <QMdiArea>

#include <QDockWidget>

class CtrlPanel;
class InfoPanel;


class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class CtrlPanel;
public:
    MainWindow();
    virtual ~MainWindow();
public:
	Ui::MainWindow ui;
    QMdiArea      *mdiArea;
    CtrlPanel	  *panelCtrl;
    InfoPanel	  *panelInfo;
    QDockWidget   *dockSide;
    QDockWidget   *dockInfo;

public slots:
    void onInit();

    void onProjectNew();
    void onProjectOpen();
    void onProjectSave();
    void onProjectSaveAs();
    void onProjectProperties();

    void onFileSave();
    void onFileSaveAll();
	void onToolsTest();
private:
    bool isModified();
    void setCurrentProject(const QString &path);
    void loadProject(const QString& path);
	void readSettings();
	void writeSettings();
    void closeDocWindow();
protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void onEditCut();
	void onEditCopy();
	void onEditPaste();
	void onEditUndo();
	void onEditRedo();
	void onEditSelectAll();

    void onNodeMoveUp();
    void onNodeMoveDown();
    void onNodeMoveLeft();
    void onNodeMoveRight();

    void onMarkAreaNew();
    void onMarkLabelNew();
    void onMarkBlockNew();
    void onMarkLSigNew();
    void onMarkBSigNew();
    void onMarkTagNew();

    void onMarkAreaClicked();
    void onMarkLabelClicked();
    void onMarkBlockClicked();
    void onMarkLSigClicked();
    void onMarkBSigClicked();
    void onMarkTagClicked();

    void onMarkAreaMenu(int i);
    void onMarkLabelMenu(int i);
    void onMarkBlockMenu(int i);
    void onMarkLSigMenu(int i);
    void onMarkBSigMenu(int i);
    void onMarkTagMenu(int i);

    void onNavPrev();
    void onNavNext();
    void onNavLocate();
	    
    void onAppSettings();
    void onAppAbout();
	void onAppExit();
public:
    void openNode(CRNode *node);
    void openDoc(CRNode *node, const QString &mcmt = "");
    void selectNode(CRNode *node);
    void closeWindows(CRNode *node);

private:
    ChildWindow* openNewDoc(CRNode *node);
    ChildWindow* openExistingDoc(CRNode *node);
    ChildWindow* getActiveDoc();
    

private:
    QIcon   iconApp;


};

#endif // MAINWINDOW_H
