#include "toolbar.h"
#include "crplugincore.h"
#include "crconstants.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/idocument.h>
#include <projectexplorer/session.h>

#include <QToolBar>

using namespace Core::Internal;
using namespace CR::Internal;
using namespace CR;


ToolBar::ToolBar(QWidget *parent)
    : QToolBar(parent)
{
    setMovable(true);
    setProperty("_q_custom_style_disabled", "true");
    setIconSize(QSize(Constants::TOOLBAR_ICON_SIZE, Constants::TOOLBAR_ICON_SIZE));

//	setAttribute(Qt::WA_LayoutUsesWidgetRect);

//    Core::EditorManager *em = Core::EditorManager::instance();
//    ProjectExplorer::SessionManager *sm = ProjectExplorer::SessionManager::instance();

//    QAction *generalAction = this->addAction(tr("General"));
//    generalAction->setCheckable(true);
//    QAction *sourceAction = this->addAction(tr("JSON Source"));
//    sourceAction->setCheckable(true);
}

