#include "infopane.h"
#include "crconstants.h"
#include "crplugincore.h"
#include "../gui/editor/memoform.h"
#include "../core/crnode.h"
#include "../core/crtree.h"

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

class CR::Internal::InfoPanePrivate {
public:
    MemoForm *memoForm;
    QList<QWidget *> toolbarWidgets;
    InfoPanePrivate() {}
};

using namespace CR;
using namespace CR::Internal;

InfoPane::InfoPane(CRPluginCore *parent) :
    IOutputPane(parent),
    d(new InfoPanePrivate())
{
    d->memoForm = new MemoForm();
    connect(d->memoForm, &MemoForm::textUpdated, this, &InfoPane::onTextUpdated);
}

InfoPane::~InfoPane()
{
    disconnect(d->memoForm, &MemoForm::textUpdated, this, &InfoPane::onTextUpdated);
    delete d->memoForm;
    delete d;
}

void InfoPane::loadText(CRNode *node)
{
    d->memoForm->loadText(node);
}

void InfoPane::onTextUpdated(CRNode *node, const QString &text)
{
    pCR->setText(node, text.toStdWString());
}

QWidget *InfoPane::outputWidget(QWidget *parent)
{
    Q_UNUSED(parent);
    return d->memoForm;
}

QList<QWidget *> InfoPane::toolBarWidgets() const
{
    return d->toolbarWidgets;
}

QString InfoPane::displayName() const
{
    return tr(Constants::INFO_PANE_TITLE);
}

int InfoPane::priorityInStatusBar() const
{
    return 1;
}

void InfoPane::clearContents()
{
}

void InfoPane::visibilityChanged(bool visible)
{
    Q_UNUSED(visible);
}

void InfoPane::setFocus()
{
    d->memoForm->setFocus();
}

bool InfoPane::hasFocus() const
{
    return d->memoForm->hasFocus();
}

bool InfoPane::canFocus() const
{
    return true;
}

bool InfoPane::canNavigate() const
{
    return false;
}

bool InfoPane::canNext() const
{
    return false;
}

bool InfoPane::canPrevious() const
{
    return false;
}

void InfoPane::goToNext()
{
}

void InfoPane::goToPrev()
{
}


