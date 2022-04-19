#ifndef CRPLUGINCORE_H
#define CRPLUGINCORE_H

#include <QObject>
#include <QSettings>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditor.h>
#include "../core/mark.h"
#include "../core/crnode.h"

namespace Core { class IOptionsPage; }

struct TextPos
{
    int row, col;
};

//------------------------------------------------------------
class CRModel;
class CRNode;
class ContentView;
// you can not do it this way ! but in vain! enum CRNode::Type;
namespace CR {
    class IOptionsPage;
namespace Internal {
    class CRPluginCorePrivate;    // PIMPL
    class OutputPane;
    class InfoPane;
    class CRSettings;
}


class CRPluginCore : public QObject
{
    Q_OBJECT
public:
    explicit CRPluginCore(QObject *parent = nullptr);
    virtual ~CRPluginCore();
    void close();
    static CRPluginCore* instance();
    CRModel *model();

    void openNode(CRNode *node);
    void findAllEntries(CRNode *node);
    void openFileAndLine(const QString &fpath, int line);
    void openFileAndMark(const QString &fpath, const QRegExp &rexp);

 //   Core::IOptionsPage* optionsPage();
    Internal::OutputPane* outputPane() const;
    Internal::InfoPane*   infoPane() const;
    QMenu*                treeMenu(CRNode *node) const;
    Core::IOptionsPage* optionsPage();
    Internal::CRSettings* settings() const;

public slots:
    void onProjectNewFromDir();
    void onProjectUpdFromDir();
    void onProjectNewFromPro();
    void onProjectUpdFromPro();
    void onProjectClear();
    void onProjectOpen();
    void onProjectSave();
    void onProjectSaveAs();
    void onProjectOptions();
    void onFileSave();
    void onToolsExpandTree();
    void onToolsAbout();
    void onNodeColorChanged(QColor clr);
    void onSettingsChanged();
private slots:

    void onNodeMoveUp();
    void onNodeMoveDown();
    void onNodeMoveLeft();
    void onNodeMoveRight();

    void onNodeFindAll();
    void onNodeFindInFile();
    void onNodeSetActive();
    void onNodeEdit();
    void onNodeMoveTo();
    void onNodeSetBase();
    void onNodeAddName();
    void onNodeAddGroup();
    void onNodeAddFile();
    void onNodeAddLink();
    void onNodeRemove();
    void onNodeInfo();
    void onNodeCoverage();

    void onToolsEdit();
    void onToolsCoverage();
    void onToolsLookup();

    // "new..." menu item clicked
    void onMarkNewArea();
    void onMarkNewLabel();
    void onMarkNewBlock();
    void onMarkNewLSig();
    void onMarkNewBSig();
    void onMarkNewName();

    // button clicked
    void onMarkBtnArea();
    void onMarkBtnLabel();
    void onMarkBtnBlock();
    void onMarkBtnLSig();
    void onMarkBtnBSig();
    void onMarkBtnName();

    // recent menu item clicked
    void onMarkMenuArea(int i);
    void onMarkMenuLabel(int i);
    void onMarkMenuBlock(int i);
    void onMarkMenuLSig(int i);
    void onMarkMenuBSig(int i);
    void onMarkMenuName(int i);

    void onCurrentEditorChanged(Core::IEditor* editor);
    void onEditorOpened(Core::IEditor* editor);
    void onEditorAboutToClose(Core::IEditor* editor);
    void onStartupProjectChanged(ProjectExplorer::Project* startupProject);
    void onProjectAdded(ProjectExplorer::Project *project);
    void onProjectsFilesChanged();

    void onAboutToClose();
    void onSaveAll(bool);
    void onClearSession(bool);

    void onDocContentsChanged();
    void onNodeCheckChanged(CRNode *node);
public:
    static Core::IEditor* getOpenedEditor(QList<Core::IEditor *> &openedEditors, const QString &fileName);
    static Core::IEditor* getOpenedEditor(const QString &fileName);
private:
    void clearProject();
    void loadProject(const QString& path);

    void updateToolBar();
    void resetTreePane();
    void updateTreePane(CRNode *node = nullptr);
    void beginRemoveTreePaneItem(CRNode *sel_node = nullptr, int i=-1);
    void endRemoveTreePaneItem();
    void updateOpenDocs();

    CRNode* getSelNode();
    CRNode* getFNodeAndCursor(QTextCursor *pcursor, QString *pfpath);

    void markMcmt(int ri, CR::Type type);
    void editMcmt();
    void editRecentMcmt(int ri, CR::Type type);

    void loadByPro(const QString &pro_path);
    void makeFromPro(const QString &filters, bool upd);

    void recolorDoc(Core::IDocument *docPtr, TextEditor::BaseTextEditor *editor);
    void parseSrcDocument(TextEditor::BaseTextEditor *editor, CRNode *fnode, CR::MarkList &ml, CR::MarkList &wl);
    void highlightMarks(const QString& fileName, CRNode *fnode, const MarkList &bars, const CR::MarkList& words);
    void setCurrEditor(Core::IEditor *editor);

    void scan(int mode, const String &str, CRNode *root);
    CRNode* lookupNodeByCursor(CR::Mark *pm);

    TextPos getCaretPos(QTextCursor &cursor);
private:
    Internal::CRPluginCorePrivate* const d;   // PIMPL
    QWidget *m_wMain;
};

}//ns CR

extern CR::CRPluginCore* m_crPCore;

#endif // CRPLUGINCORE_H
