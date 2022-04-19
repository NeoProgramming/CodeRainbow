#pragma once

#include "cr_global.h"
#include <QToolBar>
#include <extensionsystem/iplugin.h>
#include <coreplugin/actionmanager/actionmanager.h>

namespace CR {

class CRPluginCore;

namespace Internal {

class TabBar;
class ToolBar;

class CRPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "cr_plugin.json")

public:
    CRPlugin();
    ~CRPlugin();

    static CRPlugin *instance()
    {
        return inst;
    }
    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();
public slots:
    void onSettingsChanged();
private slots:
    QString getStylesheetPatternFromFile(const QString &filepath);
    void updateStyleToBaseColor();
    void onShowTabBar();
private:
    static inline QString getQssStringFromColor(const QColor &color);
    void addMenuCmd(const QString &text, const char *ID, void (CRPluginCore::*pfunc)(void));
    void createCmdTool(QToolBar *tbar, const char *res, const char *cmd, const QString &tip);
private:
    static CRPlugin *inst;
    TabBar *m_tabBar;
    ToolBar *m_toolBar;
    Core::ActionContainer *m_menu;
    bool m_styleUpdatedToBaseColor;
    QIcon iconApp;
};

} // namespace Internal
} // namespace CR
