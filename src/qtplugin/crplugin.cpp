#include "crplugin.h"
#include "crconstants.h"
#include "crplugincore.h"
#include "crsettings.h"
#include "outputpane.h"
#include "infopane.h"
#include "contentwidgetfactory.h"
#include "crmodel.h"
#include "tabbar.h"
#include "toolbar.h"
#include "../core/core.h"
#include "../core/crtree.h"
#include "../gui/gui.h"
#include "../gui/tbstyle.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/find/textfindconstants.h>
#include <coreplugin/dialogs/ioptionspage.h>
#include <utils/stylehelper.h>
#include <utils/theme/theme.h>

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QBoxLayout>
#include <QFile>
#include <QMessageBox>
#include <QElapsedTimer>

QElapsedTimer gt1;

namespace CR {
namespace Internal {

CRPlugin *CRPlugin::inst = nullptr;

CRPlugin::CRPlugin()
    : m_tabBar(nullptr)
    , m_toolBar(nullptr)
    , m_styleUpdatedToBaseColor(false)
{
    // Create your members
    if(!inst)
        inst = this;
}

CRPlugin::~CRPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    delete pCR;
    pCR = nullptr;
    inst = nullptr;
}

void CRPlugin::addMenuCmd(const QString &text, const char *ID, void (CRPluginCore::*pfunc)(void))
{
    auto action = new QAction(text, this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, ID, Core::Context(Core::Constants::C_GLOBAL));
//  cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, &QAction::triggered, m_crPCore, pfunc);
    m_menu->addAction(cmd);
}

bool CRPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    //#0000
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    iconApp = QIcon(":/res/cr.png");

    // get the main window
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(Core::ICore::mainWindow());

    //@ creating of a global common core
	pCR = new CRTree;
	//@ creating GUI objects
	Gui::init();
    pCR->user.init(Gui::pGui);

    //@ creating a core object
    m_crPCore = new CRPluginCore(this);
    addAutoReleasedObject(m_crPCore);
    addAutoReleasedObject(m_crPCore->outputPane());
    addAutoReleasedObject(m_crPCore->infoPane());
    addAutoReleasedObject(m_crPCore->optionsPage());

    CRModel *model = m_crPCore->model();/// new CRModel(pCR, this);
    ContentWidgetFactory *factory = new ContentWidgetFactory(model);
    addAutoReleasedObject(factory);

    // creating the main menu
    m_menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    m_menu->menu()->setTitle(tr("CodeRainbow"));
    m_menu->menu()->setIcon(iconApp);

    // creating menu commands
    addMenuCmd(tr("Clear CR base"), Constants::ID_CR_CLEAR, &CRPluginCore::onProjectClear);
    addMenuCmd(tr("Open CR file"), Constants::ID_CR_OPEN, &CRPluginCore::onProjectOpen);
    addMenuCmd(tr("Save CR file"), Constants::ID_CR_SAVE, &CRPluginCore::onProjectSave);
    addMenuCmd(tr("Save CR file as..."), Constants::ID_CR_SAVEAS, &CRPluginCore::onProjectSaveAs);
    addMenuCmd(tr("Create CR by project"), Constants::ID_CR_GENPRJ, &CRPluginCore::onProjectNewFromPro);
    addMenuCmd(tr("Update CR by project"), Constants::ID_CR_UPDPRJ, &CRPluginCore::onProjectUpdFromPro);
    addMenuCmd(tr("Create CR by folder"), Constants::ID_CR_GENDIR, &CRPluginCore::onProjectNewFromDir);
    addMenuCmd(tr("Update CR by folder"), Constants::ID_CR_UPDDIR, &CRPluginCore::onProjectUpdFromDir);
    addMenuCmd(tr("Expand tree"), Constants::ID_CR_EXPAND, &CRPluginCore::onToolsExpandTree);
    addMenuCmd(tr("Options..."), Constants::ID_CR_OPTIONS, &CRPluginCore::onProjectOptions);
    addMenuCmd(tr("About..."), Constants::ID_CR_ABOUT, &CRPluginCore::onToolsAbout);

    // add a menu to the Tools group
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(m_menu);


    // create tabs and connect them to the main window
//@    connect(Core::ICore::instance(), SIGNAL(themeChanged()), this, SLOT(updateStyleToBaseColor()));
    connect(Core::EditorManager::instance(), SIGNAL(editorOpened(Core::IEditor*)), SLOT(onShowTabBar()));

    mainWindow->layout()->setSpacing(0);
    QWidget *wrapper = new QWidget(mainWindow);
    wrapper->setMinimumHeight(0);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(1, 1, 1, 1);
	
    m_toolBar = new ToolBar(mainWindow);
    m_tabBar = new TabBar(mainWindow);
	
    layout->addWidget(m_toolBar);
    layout->addWidget(m_tabBar);
    layout->addWidget(mainWindow->centralWidget());

    wrapper->setLayout(layout);
    mainWindow->setCentralWidget(wrapper);

    m_tabBar->setVisible(m_crPCore->settings()->showTabs);

    // add buttons to the toolbar - standard buttons
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/cmd/res/cmd-save.png", tr("Save current document"), SLOT(onFileSave()));
//    createCmdTool(m_toolBar, ":/cmd/res/cmd-save.png",     Core::Constants::SAVE, tr("Save current document"));
    createCmdTool(m_toolBar, ":/cmd/res/cmd-saveall.png",  Core::Constants::SAVEALL, tr("Save all"));
    createCmdTool(m_toolBar, ":/cmd/res/cmd-search.png",   Core::Constants::ADVANCED_FIND, tr("Search..."));
    createCmdTool(m_toolBar, ":/cmd/res/cmd-find.png",     Core::Constants::FIND_IN_DOCUMENT, tr("Find in current document"));
    createCmdTool(m_toolBar, ":/cmd/res/cmd-findprev.png", Core::Constants::FIND_PREVIOUS, tr("Find previous"));
    createCmdTool(m_toolBar, ":/cmd/res/cmd-findnext.png", Core::Constants::FIND_NEXT, tr("Find next"));
 //   createCmdTool(m_toolBar, ":/cmd/res/cmd-findprev.png", Core::Constants::FIND_PREV_SELECTED);
 //   createCmdTool(m_toolBar, ":/cmd/res/cmd-findnext.png", Core::Constants::FIND_NEXT_SELECTED);
    m_toolBar->addSeparator();

    // add buttons to the toolbar - tree buttons
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tree/res/tree-up.png", tr("Move up"), SLOT(onNodeMoveUp()));
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tree/res/tree-down.png", tr("Move down"), SLOT(onNodeMoveDown()));
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tree/res/tree-left.png", tr("Move left"), SLOT(onNodeMoveLeft()));
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tree/res/tree-right.png", tr("Move right"), SLOT(onNodeMoveRight()));
    m_toolBar->addSeparator();

    // add buttons to the toolbar - metacomment markup tool buttons
    Gui::createMultiTool(m_toolBar, &pCR->user.markButtons[CRUser::Idx(CR::NT_AREA)], m_crPCore,
            ":/mark/res/mark-area.png",    tr("Add Area metacomment"),
            SLOT(onMarkNewArea()),  SLOT(onMarkMenuArea(int)),  SLOT(onMarkBtnArea()));
    Gui::createMultiTool(m_toolBar, &pCR->user.markButtons[CRUser::Idx(CR::NT_LABEL)], m_crPCore,
            ":/mark/res/mark-label.png",   tr("Add Label metacomment"),
            SLOT(onMarkNewLabel()), SLOT(onMarkMenuLabel(int)), SLOT(onMarkBtnLabel()));
    Gui::createMultiTool(m_toolBar, &pCR->user.markButtons[CRUser::Idx(CR::NT_BLOCK)], m_crPCore,
            ":/mark/res/mark-block.png",   tr("Add Block metacomment"),
            SLOT(onMarkNewBlock()), SLOT(onMarkMenuBlock(int)), SLOT(onMarkBtnBlock()));

    // add buttons to the toolbar - signature markup tool buttons
    Gui::createMultiTool(m_toolBar, &pCR->user.markButtons[CRUser::Idx(CR::NT_LSIG)], m_crPCore,
            ":/mark/res/mark-lsig.png",    tr("Add Label Signature"),
            SLOT(onMarkNewLSig()), SLOT(onMarkMenuLSig(int)), SLOT(onMarkBtnLSig()));
    Gui::createMultiTool(m_toolBar, &pCR->user.markButtons[CRUser::Idx(CR::NT_BSIG)], m_crPCore,
            ":/mark/res/mark-bsig.png",    tr("Add Block Signature"),
            SLOT(onMarkNewBSig()), SLOT(onMarkMenuBSig(int)), SLOT(onMarkBtnBSig()));
    Gui::createMultiTool(m_toolBar, &pCR->user.markButtons[CRUser::Idx(CR::NT_NAME)], m_crPCore,
            ":/mark/res/mark-name.png",    tr("Add Named Symbol"),
            SLOT(onMarkNewName()), SLOT(onMarkMenuName(int)), SLOT(onMarkBtnName()));

    m_toolBar->addSeparator();

    // add buttons to the toolbar - service tool buttons
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tools/res/tools-edit.png", tr("Edit mark"), SLOT(onToolsEdit()));
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tools/res/tools-coverage.png", tr("Check coverage"), SLOT(onToolsCoverage()));
    Gui::createBtnTool(m_toolBar, m_crPCore, ":/tools/res/tools-lookup.png", tr("Info lookup"), SLOT(onToolsLookup()));

    // ide close event
    Core::ICore *pcore = Core::ICore::instance();
    connect(pcore, SIGNAL(coreAboutToClose()), m_crPCore, SLOT(onAboutToClose()));

    // save all command
    Core::Command *cmd = Core::ActionManager::command(Core::Constants::SAVEALL);
    connect(cmd->action(), SIGNAL(triggered(bool)), m_crPCore, SLOT(onSaveAll(bool)));

    // close all command
    cmd = Core::ActionManager::command("ProjectExplorer.ClearSession");
    connect(cmd->action(), SIGNAL(triggered(bool)), m_crPCore, SLOT(onClearSession(bool)));

    return true;
}

void CRPlugin::createCmdTool(QToolBar *tbar, const char *res, const char *idcmd, const QString &tip)
{
    // add a standard command to the toolbar
    Core::Command *cmd = Core::ActionManager::command(idcmd);
    if(!cmd)
        qDebug() << "!cmd for " << idcmd;
    else {
        QAction *act = cmd->action();
        if(!act)
            qDebug() << "!act for " << idcmd;
        else
            Gui::createBtnTool(tbar, res, act, tip);
    }
}

void CRPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
//    qDebug() << "start";
//    QTimer::singleShot(1000, this, SLOT(onPostInit()));
}

void CRPlugin::onSettingsChanged()
{
    qDebug()     << "onSettingsChanged";
    m_tabBar->setVisible(m_crPCore->settings()->showTabs);
}

ExtensionSystem::IPlugin::ShutdownFlag CRPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

QString CRPlugin::getStylesheetPatternFromFile(const QString &filepath)
{
    QFile stylesheetFile(filepath);
    if (!stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromUtf8(stylesheetFile.readAll());
}

void CRPlugin::updateStyleToBaseColor()
{
    Utils::Theme *theme = Utils::creatorTheme();

    QString baseColorQss;
    QString borderColorQss;
    QString highlightColorQss;
    QString selectedTabBorderColorQss;
    QString shadowColorQss;

    if(theme->preferredStyles().isEmpty()) {
        baseColorQss = getQssStringFromColor(Utils::StyleHelper::baseColor().lighter(130));
        borderColorQss = getQssStringFromColor(Utils::StyleHelper::borderColor());
        highlightColorQss = getQssStringFromColor(Utils::StyleHelper::baseColor());
        selectedTabBorderColorQss
                = getQssStringFromColor(Utils::StyleHelper::highlightColor().lighter());
        shadowColorQss = getQssStringFromColor(Utils::StyleHelper::shadowColor());
    } else { // Flat widget style
        baseColorQss
                = getQssStringFromColor(theme->color(Utils::Theme::BackgroundColorHover));
        borderColorQss = getQssStringFromColor(theme->color(Utils::Theme::BackgroundColorHover));
        highlightColorQss = getQssStringFromColor(theme->color(Utils::Theme::BackgroundColorDark));
        selectedTabBorderColorQss
                = getQssStringFromColor(theme->color(Utils::Theme::BackgroundColorDark));
        shadowColorQss = getQssStringFromColor(theme->color(Utils::Theme::BackgroundColorNormal));
    }

    QString stylesheetPattern = getStylesheetPatternFromFile(QStringLiteral(":/styles/default.qss"));

    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%FRAME_BACKGROUND_COLOR%"), highlightColorQss);
    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_SELECTED_BORDER_COLOR%"), selectedTabBorderColorQss);
    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_SELECTED_BACKGROUND_COLOR%"), baseColorQss);
    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_SELECTED_BOTTOM_BORDER_COLOR%"), baseColorQss);

    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_BACKGROUND_COLOR_FROM%"), shadowColorQss);
    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_BACKGROUND_COLOR_TO%"), shadowColorQss);
    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_BORDER_COLOR%"), borderColorQss);
    stylesheetPattern = stylesheetPattern.replace(QLatin1String("%TAB_BOTTOM_BORDER_COLOR%"), borderColorQss);

    m_tabBar->setStyleSheet(stylesheetPattern);
}

void CRPlugin::onShowTabBar()
{
    updateStyleToBaseColor();

    if (m_styleUpdatedToBaseColor) {
        disconnect(Core::EditorManager::instance(), SIGNAL(editorOpened(Core::IEditor*)),
                   this, SLOT(onShowTabBar()));
        return;
    }

    m_styleUpdatedToBaseColor = true;
}

QString CRPlugin::getQssStringFromColor(const QColor &color)
{
    return QString::fromLatin1("rgba(%1, %2, %3, %4)").arg(
                QString::number(color.red()),
                QString::number(color.green()),
                QString::number(color.blue()),
                QString::number(color.alpha()));
}

} // namespace Internal
} // namespace CR
