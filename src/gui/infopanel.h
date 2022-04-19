#ifndef INFOPANEL_H
#define INFOPANEL_H

#include "ui_infopanel.h"
#include <QListWidget>
#include <QTreeWidget>
#include "../core/crnode.h"

class MainWindow;

class InfoPanel : public QWidget
{
    Q_OBJECT
public:
    InfoPanel(QWidget *parent, MainWindow *h);
    void loadText(CRNode *node);
    void initialize();

public slots:
    void onMsgAdd(const char* message, int image, unsigned long param);
    void onMsgRemoveAll();

private slots:
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onEditorLostFocus();
    void onShowContentsMenu(const QPoint &pos);
    void processEvents();

private:
        
    Ui::InfoPanel ui;
    MainWindow *mw;
    CRNode *m_Node;

private:
    void timerEvent(QTimerEvent *e);
	void resizeEvent(QResizeEvent *e);
};

#endif // INFOPANEL_H
