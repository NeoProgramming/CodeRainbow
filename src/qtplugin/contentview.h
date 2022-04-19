#pragma once

#include <QTreeView>
#include <QMenu>

class QAbstractItemModel;
class CRNode;
class ColorMenu;
class QAction;

class ContentView : public QTreeView
{
    Q_OBJECT
public:
    ContentView(QAbstractItemModel *model, QWidget *parent = 0);
    virtual ~ContentView();
protected slots:
    void onItemDoubleClicked(const QModelIndex &index);
    void onTreeSelChanged(const QItemSelection &selected, const QItemSelection &deselected);
protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void closeEvent(QCloseEvent *event);
public:
    static ContentView* currentView;
    static bool inFocus;
};
