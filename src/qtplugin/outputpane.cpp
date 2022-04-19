#include "outputpane.h"
#include "crplugincore.h"
#include "crconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <utils/utilsicons.h>

#include <QTreeView>
#include <QHeaderView>
#include <QToolButton>
#include <QApplication>
#include <QFileInfo>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QListWidget>

struct ItemEntry {
    QString path;
    int line;
};

// special declaration in order to be able to pack the node into variants
Q_DECLARE_METATYPE(ItemEntry*)

class CR::Internal::OutputPanePrivate {
public:
    QList<QWidget *> toolbarWidgets;
    QTreeWidget *treeWidget;
    QList<ItemEntry> items;

    OutputPanePrivate() {}
};

using namespace CR;
using namespace CR::Internal;

OutputPane::OutputPane(CRPluginCore *parent) :
    IOutputPane(parent),
    d(new OutputPanePrivate())
{
    d->treeWidget = new QTreeWidget();
    d->treeWidget->setColumnCount(2);
    d->treeWidget->setAlternatingRowColors(true);
    d->treeWidget->setStyleSheet("background-color: #ECECFE; alternate-background-color: #FFFFC2");
    int w = d->treeWidget->width()/2;
    QTreeWidgetItem* headerItem = d->treeWidget->headerItem();
    headerItem->setText(0, "Path:LineNum");
    headerItem->setText(1, "Line1");
    QHeaderView *headerView = d->treeWidget->header();
    headerView->resizeSection(0, w);
    headerView->resizeSection(1, w);
    connect(d->treeWidget, &QTreeWidget::itemDoubleClicked, this, &OutputPane::onItemDoubleClicked);
}

OutputPane::~OutputPane()
{
    delete d->treeWidget;
    for(QWidget* widget: d->toolbarWidgets) {
        delete widget;
    }
    delete d;
}

QWidget *OutputPane::outputWidget(QWidget *parent)
{
    Q_UNUSED(parent);
    return d->treeWidget;
}
//--------------------------------------------------

QList<QWidget *> OutputPane::toolBarWidgets() const
{
    return d->toolbarWidgets;
}

QString OutputPane::displayName() const
{
    return tr(Constants::OUTPUT_PANE_TITLE);
}

int OutputPane::priorityInStatusBar() const
{
    return 1;
}

void OutputPane::clearContents()
{
    clearMessages();
}

void OutputPane::visibilityChanged(bool visible)
{
    Q_UNUSED(visible);
}

void OutputPane::setFocus()
{
    d->treeWidget->setFocus();
}

bool OutputPane::hasFocus() const
{
    return d->treeWidget->hasFocus();
}

bool OutputPane::canFocus() const
{
    return true;
}

bool OutputPane::canNavigate() const
{
    return true;
}

bool OutputPane::canNext() const
{
    return d->treeWidget->topLevelItemCount() > 1;
}

bool OutputPane::canPrevious() const
{
    return d->treeWidget->topLevelItemCount() > 1;
}

void OutputPane::goToNext()
{
    /*~
    QModelIndex currentIndex;
    QModelIndexList currentSelectedList = d->treeView->selectionModel()->selectedIndexes();
    if(currentSelectedList.isEmpty() == false) {
        currentIndex = currentSelectedList.first();
    }
    QModelIndex nextIndex = d->treeView->indexBelow(currentIndex);
    if(nextIndex.isValid() == false) {
        nextIndex = d->treeView->model()->index(0, 0);
    }

    d->treeView->selectionModel()->select(nextIndex, QItemSelectionModel::SelectCurrent);
    mistakeSelected(nextIndex);
    */
}
//--------------------------------------------------

void OutputPane::goToPrev()
{
    /*~
    QModelIndex currentIndex;
    QModelIndexList currentSelectedList = d->treeView->selectionModel()->selectedIndexes();
    if(currentSelectedList.isEmpty() == false) {
        currentIndex = currentSelectedList.first();
    }

    QModelIndex previousIndex = d->treeView->indexAbove(currentIndex);
    if(previousIndex.isValid() == false) {
        previousIndex = d->treeView->model()->index(d->treeView->model()->rowCount() - 1, 0);
    }

    d->treeView->selectionModel()->select(previousIndex, QItemSelectionModel::SelectCurrent);
    mistakeSelected(previousIndex);
*/
}



void OutputPane::addMessage(const QString &path, int line, const QString &str)
{
    ItemEntry entry;
    entry.path = path;
    entry.line = line;
    d->items.push_back(entry);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, entry.path + ":" + QString::number(entry.line+1));
    item->setText(1, str);
    item->setData(0, Qt::UserRole, QVariant::fromValue<ItemEntry*>(&d->items.back()));
    d->treeWidget->addTopLevelItem(item);
}

void OutputPane::clearMessages()
{
    d->treeWidget->clear();
    d->items.clear();
}

void OutputPane::onItemDoubleClicked(QTreeWidgetItem *item)
{
    ItemEntry *entry = item->data(0, Qt::UserRole).value<ItemEntry*>();
    m_crPCore->openFileAndLine(entry->path, entry->line+1);
}
