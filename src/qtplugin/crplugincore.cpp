#include "crplugincore.h"

#include "outputpane.h"
#include "infopane.h"
#include "crconstants.h"
#include "crmodel.h"
#include "contentview.h"
#include "../core/core.h"
#include "../core/crtree.h"
#include "../core/mcmt.h"
#include "../core/parse.h"
#include "../core/markmaker.h"
#include "../core/scan.h"
#include "../gui/text.h"
#include "../gui/dialogs/editrecentdlg.h"
#include "../gui/dialogs/newprojectdlg.h"
#include "../gui/dialogs/prjpropsdlg.h"
#include "../gui/dialogs/markupdlg.h"
#include "../gui/dialogs/nodedlg.h"
#include "../gui/dialogs/linkdlg.h"
#include "../gui/dialogs/selnodedlg.h"
#include "../gui/widgets/colormenu.h"

#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <aggregation/aggregate.h>
#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/find/ifindsupport.h>
#include "croptionspage.h"
#include "crsettings.h"
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
#include <utils/QtConcurrentTools>
#include <utils/fadingindicator.h>


#include <QPointer>
#include <QMouseEvent>
#include <QTextCursor>
#include <QMenu>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QTextBlock>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>
#include <QFileDialog>

using namespace CR;
using namespace CR::Internal;

CR::CRPluginCore* m_crPCore = nullptr;
static CRPluginCore *g_instance = nullptr;

class CR::Internal::CRPluginCorePrivate {
    friend class CRPluginCore;
public: // Core

    CRModel *model;
    CR::Internal::OutputPane* outputPane;
    CR::Internal::InfoPane* infoPane;
    CR::Internal::CRSettings* settings;
    CR::Internal::CROptionsPage* optionsPage;
//    Core::ActionContainer *contextMenu;
//    QList<Core::Command*> contextMenuHolderCommands;

    QPointer<Core::IEditor> currentEditor;
    QString currentFilePath;
    ProjectExplorer::Project* startupProject;
    QStringList filesInStartupProject;
    const QRegularExpression cppRegExp;

    QMenu m_menuSys;
    QMenu m_menuNode;
    QMenu m_menuTag;
    ColorMenu *colorMenu;
    QAction *colorAction;

public:
    CRPluginCorePrivate()
        : outputPane(nullptr)
        , currentEditor(nullptr)
        , startupProject(nullptr)
        , cppRegExp(QLatin1String(CR::Constants::CPP_SOURCE_FILES_REGEXP_PATTERN), QRegularExpression::CaseInsensitiveOption)
    {}
    ~CRPluginCorePrivate() {}
};

//#0001
CRPluginCore::CRPluginCore(QObject *parent)
    : QObject(parent)
    , d(new Internal::CRPluginCorePrivate())
    , m_wMain(Core::ICore::mainWindow())
{
    // singleton
    Q_ASSERT(g_instance == nullptr);
    g_instance = this;

    // settings
    d->settings = new CRSettings();
    d->settings->loadFromSettings(Core::ICore::settings());
    // model
    d->model = new CRModel(pCR);
    connect(d->model, &CRModel::checkChanged, this, &CRPluginCore::onNodeCheckChanged);
    // output panel
    d->outputPane = new OutputPane(this);
    // information panel
    d->infoPane = new InfoPane(this);
    // settings panel
    d->optionsPage = new CROptionsPage(d->settings);

    // side tree context menu
    d->m_menuSys.addAction(Gui::getToolIcon(CR::TI_ACTIVE),tr("Set current"),        this, SLOT(onNodeSetActive()));
    d->m_menuSys.addAction(Gui::getToolIcon(CR::TI_ADD),   tr("Add node..."),       this, SLOT(onNodeAddName()));
    d->m_menuSys.addAction(Gui::getTreeIcon(CR::NT_GROUP), tr("Add group..."),      this, SLOT(onNodeAddGroup()));
    d->m_menuSys.addAction(Gui::getTreeIcon(CR::NT_FILE),  tr("Add file..."),       this, SLOT(onNodeAddFile()));
    d->m_menuSys.addAction(Gui::getTreeIcon(CR::NT_LINK),  tr("Add link..."),       this, SLOT(onNodeAddLink()));
    d->m_menuSys.addAction(Gui::getToolIcon(CR::TI_INFO),  tr("Info..."),           this, SLOT(onNodeInfo()));
//  d->m_menuSys.addAction(Gui::getToolIcon(CR::TI_COVER), tr("Scan coverage"),     this, SLOT(onNodeCoverage()));

    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_ACTIVE),tr("Set current"),       this, SLOT(onNodeSetActive()));
    d->m_menuNode.addAction(tr("Find all"),          this, SLOT(onNodeFindAll()));
    d->m_menuNode.addAction(tr("Find in file"),      this, SLOT(onNodeFindInFile()));
    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_EDIT),  tr("Edit..."),           this, SLOT(onNodeEdit()));
    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_MOVE),  tr("Move node..."),      this, SLOT(onNodeMoveTo()));
    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_SETBASE),  tr("Set base..."),    this, SLOT(onNodeSetBase()));
    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_ADD),   tr("Add node..."),       this, SLOT(onNodeAddName()));
    d->m_menuNode.addAction(Gui::getTreeIcon(CR::NT_GROUP), tr("Add group..."),      this, SLOT(onNodeAddGroup()));
    d->m_menuNode.addAction(Gui::getTreeIcon(CR::NT_FILE),  tr("Add file..."),       this, SLOT(onNodeAddFile()));
    d->m_menuNode.addAction(Gui::getTreeIcon(CR::NT_LINK),  tr("Add link..."),       this, SLOT(onNodeAddLink()));
    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_REMOVE),tr("Remove node"),       this, SLOT(onNodeRemove()));
    d->m_menuNode.addAction(Gui::getToolIcon(CR::TI_INFO),  tr("Info..."),           this, SLOT(onNodeInfo()));
//    m_menuNode.addAction(Gui::getToolIcon(CR::TI_COVER), tr("Scan coverage"),      this, SLOT(onNodeCoverage()));

    d->colorMenu = new ColorMenu(&d->m_menuNode);
    d->colorAction = d->m_menuNode.addMenu(d->colorMenu);
    d->colorAction->setText("Set node color");
    connect(d->colorMenu, &ColorMenu::selected, this, &CR::CRPluginCore::onNodeColorChanged);
    connect(d->colorMenu, &ColorMenu::selected, [this](QColor) {
        d->m_menuNode.close();
    });

/*~ //@doc: context menu
    Core::Context context(CppEditor::Constants::CPPEDITOR_ID);
    Core::ActionContainer *cppEditorContextMenu= Core::ActionManager::createMenu(CppEditor::Constants::M_CONTEXT);
    Core::ActionContainer *contextMenu = Core::ActionManager::createMenu(Constants::CONTEXT_MENU_ID);
    cppEditorContextMenu->addSeparator(context);
    cppEditorContextMenu->addMenu(contextMenu); */

    //@ connecting to development environment components for various purposes
    Core::EditorManager *editorManager = Core::EditorManager::instance();
    connect(editorManager, &Core::EditorManager::currentEditorChanged, this, &CRPluginCore::onCurrentEditorChanged);
    connect(editorManager, &Core::EditorManager::editorOpened,         this, &CRPluginCore::onEditorOpened);
    connect(editorManager, &Core::EditorManager::editorAboutToClose,   this, &CRPluginCore::onEditorAboutToClose);

    connect(ProjectExplorer::SessionManager::instance(),        &ProjectExplorer::SessionManager::startupProjectChanged,
            this, &CRPluginCore::onStartupProjectChanged);
    connect(ProjectExplorer::SessionManager::instance(),        &ProjectExplorer::SessionManager::projectAdded,
            this, &CRPluginCore::onProjectAdded);
    connect(ProjectExplorer::ProjectExplorerPlugin::instance(), &ProjectExplorer::ProjectExplorerPlugin::fileListChanged,
            this, &CRPluginCore::onProjectsFilesChanged);


}

CRPluginCore::~CRPluginCore()
{
    Core::EditorManager *editorManager = Core::EditorManager::instance();
    disconnect(editorManager, &Core::EditorManager::editorOpened,         this, &CRPluginCore::onEditorOpened);  //#0011
    disconnect(editorManager, &Core::EditorManager::currentEditorChanged, this, &CRPluginCore::onCurrentEditorChanged);  //#0009
    disconnect(editorManager, &Core::EditorManager::editorAboutToClose,   this, &CRPluginCore::onEditorAboutToClose); //#0013
    disconnect(ProjectExplorer::SessionManager::instance(), &ProjectExplorer::SessionManager::startupProjectChanged, this, &CRPluginCore::onStartupProjectChanged); //#0015
    disconnect(ProjectExplorer::ProjectExplorerPlugin::instance(), &ProjectExplorer::ProjectExplorerPlugin::fileListChanged, this, &CRPluginCore::onProjectsFilesChanged); //#0017

    d->settings->saveToSettings(Core::ICore::settings());
    delete d->settings;

    if(d)
        delete d;
//    d = nullptr;
}

CRModel *CRPluginCore::model()
{
    return d->model;
}

void CRPluginCore::close()
{
    //@ just in case removeDocumentParser code
    // Disconnect all signals between the parser and the core.

}

CRPluginCore *CRPluginCore::instance()
{
    return g_instance;
}

OutputPane *CRPluginCore::outputPane() const
{
    // return pointer to output pane
    return d->outputPane;
}


InfoPane *CRPluginCore::infoPane() const
{
    // return pointer to info bar
    return d->infoPane;
}

QMenu* CRPluginCore::treeMenu(CRNode *node) const
{
    if(CR::isSys(node->type))
        return &d->m_menuSys;
    else
        return &d->m_menuNode;
}

Core::IOptionsPage *CRPluginCore::optionsPage()
{
    return d->optionsPage;
}

CRSettings *CRPluginCore::settings() const
{
    return d->settings;
}

Core::IEditor* CRPluginCore::getOpenedEditor(QList<Core::IEditor *> &openedEditors, const QString &fileName)
{
    Q_FOREACH(Core::IEditor *editor, openedEditors) {
        QString path = editor->document()->filePath().toString();
        if(path == fileName)
            return editor;
    }
    return nullptr;
}

Core::IEditor* CRPluginCore::getOpenedEditor(const QString &fileName)
{
    // check if such a document is open in the editor
    QList<Core::IEditor *> openedEditors = Core::EditorManager::instance()->visibleEditors();
    return getOpenedEditor(openedEditors, fileName);
}

void CRPluginCore::setCurrEditor(Core::IEditor *editor)
{
    d->currentEditor = editor;
    if(editor != nullptr) {
        d->currentFilePath = editor->document()->filePath().toString();
        CRNode *fnode = pCR->getFileNode(d->currentFilePath.toStdWString());
        pCR->updateCurrents(fnode);
    }
    else {
        d->currentFilePath = QLatin1String("");
        pCR->updateCurrents(nullptr);
    }
    d->model->layoutModelAll();
}

//#0008
void CRPluginCore::onCurrentEditorChanged(Core::IEditor *editor)
{
    // changed (switched) the current editor (but not the content)
//  qDebug() << "CRPluginCore::onCurrentEditorChanged:";
    setCurrEditor(editor);
}

void CRPluginCore::onProjectAdded(ProjectExplorer::Project *project)
{
    if(project == nullptr)
        return;

    // the first time - we are trying to construct a path to a .cr file along the path to .pro and open
    if(pCR->pathRecentProject.empty())
        loadByPro(project->projectFilePath().toString());
}

//#0014
void CRPluginCore::onStartupProjectChanged(ProjectExplorer::Project *startupProject)
{
	// active project change event
    // fires also on project initial load
    // and after cleaning -- with the last project, which is wrong!

    d->filesInStartupProject.clear();
    d->startupProject = startupProject;

    if(startupProject == nullptr)
        return;

    if(d->startupProject != nullptr) {
        QStringList list = d->startupProject->files(ProjectExplorer::Project::SourceFiles);
        d->filesInStartupProject = list.filter(d->cppRegExp);
    }
}

void CRPluginCore::loadByPro(const QString &pro_path)
{
    // construct a path and load .cr file by .pro file, by simple replacement of extensions
    QFileInfo fpath(pro_path);
    QString cr_path = fpath.absolutePath() + "/" + fpath.completeBaseName() + AppFileExt;
    loadProject(cr_path);
}

void CRPluginCore::recolorDoc(Core::IDocument *docPtr, TextEditor::BaseTextEditor *editor)
{
	//extern QElapsedTimer gt1;
	//gt1.start();

    if(!settings()->showHL)
        return;

    QString fileName = docPtr->filePath().toString();

    //@ check if there is a file in the project
    //@ bool cn = d->filesInStartupProject.contains(fileName, Qt::CaseInsensitive);

    // determine whether it is necessary to parse the document - we are looking for a file node with the same name
    CRNode *fnode = pCR->getFileNode(fileName.toStdWString());
    // if not in the project or in the tree - do not color
    //if(!cn && !fnode)
    //    return;
    // if the file is not in the cr project, we don't color it, at least I don't see the point yet
    if(!fnode)
        return;
	
	// get lists of blocks and words to highlight
    MarkList bars, words;
    parseSrcDocument(editor, fnode, bars, words);

    // colorize the document
    highlightMarks(fileName, fnode, bars, words);
	
	//qDebug() << gt1.elapsed();
}

void CRPluginCore::parseSrcDocument(TextEditor::BaseTextEditor *editor, CRNode *fnode, CR::MarkList &bars, CR::MarkList &words)
{
    // source array - get it once
    extern QElapsedTimer gt1;
    gt1.start();

    if (!editor) {
        qDebug() << "!editor";
        return;
    }
    QString src16 = editor->textDocument()->plainText();
    Parser<Utf16> parser((const wchar_t*)src16.utf16(), src16.size(), fnode);

    parser.parseMarkup(&bars, &words);

    int time = gt1.nsecsElapsed();
    qDebug() << "t="<< time;
}

void CRPluginCore::onDocContentsChanged()
{
    Core::IDocument *doc = qobject_cast<Core::IDocument*>(sender());
    TextEditor::BaseTextEditor *editor = TextEditor::BaseTextEditor::currentTextEditor();
    recolorDoc(doc, editor);
}

//#0012
void CRPluginCore::onEditorAboutToClose(Core::IEditor *editor)
{
//  qDebug() << "onEditorAboutToClose: ";
    disconnect(editor->document(), &Core::IDocument::contentsChanged, this, &CRPluginCore::onDocContentsChanged);
}

//#0010
void CRPluginCore::onEditorOpened(Core::IEditor *ieditor)
{
//  qDebug() << "CRPluginCore::onEditorOpened:";
    setCurrEditor(ieditor);
    connect(ieditor->document(), &Core::IDocument::contentsChanged, this, &CRPluginCore::onDocContentsChanged);
    // postponed call! yes, this is instead of PostMessage
//  QTimer::singleShot(0, this, &CRPluginCore::recolorDoc);

    TextEditor::BaseTextEditor *editor = qobject_cast<TextEditor::BaseTextEditor *>(ieditor);
    recolorDoc(editor->document(), editor);
}

//#0016
void CRPluginCore::onProjectsFilesChanged()
{
    onStartupProjectChanged(d->startupProject);
}

//#0021 addMisspelledWords
void CRPluginCore::highlightMarks(const QString &fileName, CRNode *fnode, const MarkList &bars, const MarkList &words)
{
    // a function that performs direct coloring; coloring is applied only to the current file
    if(d->currentFilePath != fileName)
        return;

    // get editor
    TextEditor::BaseTextEditor* baseEditor = qobject_cast<TextEditor::BaseTextEditor*>(d->currentEditor);
    if(baseEditor == nullptr)
        return;

    // get editor widget
    TextEditor::TextEditorWidget* editorWidget = baseEditor->editorWidget();
    if(editorWidget == nullptr)
        return;

    // get a document
    QTextDocument* document = editorWidget->document();
    if(document == nullptr)
        return;

    // if necessary - set the background color of the entire document at once
    if(fnode && (fnode->nclr != CLR_NONE) && (fnode->flags & CRNode::NF_VISIBLE))
        editorWidget->setStyleSheet("background-color:"+QColor(fnode->nclr).name()+";");
    else
        editorWidget->setStyleSheet("background-color:none;");

    // selection area for coloring
    QList<QTextEdit::ExtraSelection> selections;
    selections.reserve(bars.size() + words.size());

    // loop through stripes for coloring
    Q_FOREACH(const Mark& mark, bars) {
        // if color is not set
        if (mark.color == CLR_NONE)
            continue;
        // if visibility is off
        if ( mark.node && !(mark.node->flags & CRNode::NF_VISIBLE) )
            continue;

        // get block (=line) by number
        //const QTextBlock& block = document->findBlockByNumber(mark.row);
        //if ((block.isValid() == false))
        //    continue;

        // create a "cursor" there
        //QTextCursor cursor(block);
        //int cpos = cursor.position();
        //qDebug() << "line=" << mark.row << " cpos=" << cpos << " pos=" << mark.pos;

        // move a few lines down, thereby highlighting them
        // there's a delay!
        //cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, mark.lines);

		// much faster way

		QTextCursor cursor(document);
		cursor.setPosition(mark.pos, QTextCursor::MoveAnchor);
        cursor.setPosition(mark.pos + mark.chars + 1, QTextCursor::KeepAnchor);

		//qDebug() << "line=" << mark.row << " pos=" << mark.pos << " npos=" << mark.pos + mark.chars;

        // create a format with a given color
        QTextCharFormat format = cursor.charFormat();
        QColor backGroundColor = QColor(mark.color);
        format.setBackground(backGroundColor);

        // create a selection and put everything in it; // looks like this is an alternative to the setBlockFormat line
        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format = format;
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selections.append(selection);
    }

    // loop by words (it is arranged differently)
    Q_FOREACH(const Mark& mark, words) {
        // if color is not set
        if (mark.color == CLR_NONE)
            continue;
        // if visibility is off
        if ( mark.node && !(mark.node->flags & CRNode::NF_VISIBLE) )
            continue;
        // get block (=line) by number
        const QTextBlock& block = document->findBlockByNumber(mark.row);
        if ((block.isValid() == false))
            continue;

        // create a "cursor" there
        QTextCursor cursor(block);
        // put the cursor in the line on the desired character
        cursor.setPosition(cursor.position() + mark.col);
        // move cursor by character length
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, mark.chars);

        // create a format with a given color
        QTextCharFormat format = cursor.charFormat();
        QColor backGroundColor = QColor(mark.color);
        format.setBackground(backGroundColor);

        // create a selection and put everything in it; // looks like this is an alternative to the setBlockFormat line
        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format = format;
        selections.append(selection);
    }

    // apply all formats at once
    editorWidget->setExtraSelections(Core::Id(CR::Constants::CR_HIGHLIGHT_ID), selections);
}

void CRPluginCore::clearProject()
{
    pCR->createEmpty(_T(""), _T(""));
    //setCurrentProject("", false); //@ok
    resetTreePane();
    updateToolBar();
}

void CRPluginCore::loadProject(const QString& path)
{
    // loading the project; if it doesn't load, create an empty one
    if( pCR->loadCR(path.toStdWString()) ) {
        qDebug() << "loadProject: ok: " + path;
        //setCurrentProject(path, true); //@ok
    }
    else {
        qDebug() << "loadProject: no, create empty project: " + path;
        //if(d->startupProject)
        pCR->createEmpty(d->startupProject ? d->startupProject->displayName().toStdWString() : std::wstring(), QFileInfo(path).absolutePath().toStdWString());
        //setCurrentProject(path, false); //@xz << anyway
    }

    resetTreePane();
    updateToolBar();
}

void CRPluginCore::onProjectNewFromDir()
{
    // select project source: file or dir
    NewProjectDlg dlg;
    if(dlg.execDlg()) {

        pCR->createByDir(dlg.m_name.toStdWString(), dlg.m_base.toStdWString(), dlg.m_filters.toStdWString(), settings()->splitByFiles);

        //QString suggestedPath = dlg.m_base + "/" + dlg.m_name + AppFileExt;
        //setCurrentProject(suggestedPath, false);//@ok
        resetTreePane();
        updateToolBar();
    }
}

void CRPluginCore::onProjectUpdFromDir()
{
//  qDebug() << "onProjectUpdFromDir: " << pCR->getBaseNode()->path;
    if(pCR->getFiles()->path.empty()) {
        onProjectNewFromDir();
        return;
    }

    QString filters = QInputDialog::getText(NULL, "Input filters",
           "Filters:", QLineEdit::Normal, SrcFilters);
    if(!filters.isEmpty()) {
        qDebug() << "onProjectUpdFromDir";
        pCR->updateByDir(filters.toStdWString(), settings()->splitByFiles);
        resetTreePane();
        updateToolBar();
    }
}

void CRPluginCore::onProjectNewFromPro()
{
    QString filters = QInputDialog::getText(NULL, "Input filters",
           "Filters:", QLineEdit::Normal, SrcFilters);
    if(!filters.isEmpty()) {

        makeFromPro(filters, false);

        resetTreePane();
        updateToolBar();
    }
}

void CRPluginCore::onProjectUpdFromPro()
{
//  qDebug() << "onProjectUpdFromPro: " << pCR->getBaseNode()->path;
    if(pCR->getFiles()->path.empty()) {
        onProjectNewFromPro();
        return;
    }

    QString filters = QInputDialog::getText(NULL, "Input filters",
           "Filters:", QLineEdit::Normal, SrcFilters);
    if(!filters.isEmpty()) {
        makeFromPro(filters, true);
        resetTreePane();
        updateToolBar();
    }
}

void CRPluginCore::onProjectClear()
{
    if(QMessageBox::question(m_wMain, AppName, tr("Are you sure to clear database?"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        return;
    clearProject();
}

void CRPluginCore::onProjectOpen()
{
    QString filter = AppFileFilter;
    QString path = QFileDialog::getOpenFileName(
                m_wMain,
                tr("Open File"),
                "",
                filter,
                &filter);
    if(!path.isEmpty())	{
        loadProject(path);
    }
}

void CRPluginCore::onProjectSave()
{
    if(!pCR->modify)
        return;
    if(!pCR->confirmedPath || !pCR->saveCR(pCR->pathRecentProject))
        onProjectSaveAs();
}

void CRPluginCore::onProjectSaveAs()
{
    qDebug() << "onProjectSaveAs: " << pCR->pathRecentProject;
    QString filter = AppFileFilter;
    QString path = QFileDialog::getSaveFileName(
                Core::ICore::mainWindow(),
                tr("Save File As"),
                QString::fromStdWString( pCR->pathRecentProject ),
                filter,
                &filter);

    if(!path.isEmpty()) {
        if (!path.endsWith(AppFileExt))
            path += AppFileExt;
        if(!pCR->saveCR(path.toStdWString()))
            QMessageBox::warning(Core::ICore::mainWindow(), "Save error", "Save error", QMessageBox::Ok);
        //else
        //    setCurrentProject(path, true); //@ok
    }
}

void CRPluginCore::onFileSave()
{
    // if we have a tree in focus, then we save the project, otherwise we redirect to the current file
    if(ContentView::inFocus) {
        onProjectSave();
    }
    else {
        Core::Command *cmd = Core::ActionManager::command(Core::Constants::SAVE);
        if(cmd) {
            QAction *act = cmd->action();
            if(act)
                act->trigger();
        }
    }
}

void CRPluginCore::onProjectOptions()
{

//    PrjPropsDlg dlg;
//    dlg.pp = pCR->pp;
//    if(dlg.execDlg())
//        pCR->pp = dlg.pp;
}

void CRPluginCore::onToolsExpandTree()
{
    resetTreePane();
}

void CRPluginCore::onToolsAbout()
{
    const char VERSION_STR[] = "Build date & time:\r\n" __DATE__ "  " __TIME__;
    QMessageBox::information(m_wMain, AppName, VERSION_STR, QMessageBox::Ok);
}

CRNode* CRPluginCore::getSelNode()
{
    if(!ContentView::currentView)
        return qDebug("!d->currentTreeView"), nullptr;
    QModelIndex index = ContentView::currentView->currentIndex();
    if(!index.isValid())
        return qDebug("!index.isValid()"), nullptr;

    QVariant data = ContentView::currentView->model()->data(index, Qt::UserRole);
    qDebug() << "getSelNode(): index == " << index << " " << data;

    CRNode *node = data.value<CRNode*>();
    return node;
}

void CRPluginCore::resetTreePane()
{
    d->model->resetModel();
}

void CRPluginCore::updateToolBar()
{
    for(int ti=0; ti<CRUser::BTN_COUNT; ti++)
        Gui::updateMarkMenu(&pCR->user.markButtons[ti]);
}

void CRPluginCore::updateTreePane(CRNode *node)
{
	if (node) {
		if(ContentView::currentView)
			ContentView::currentView->selectionModel()->clear();
	}
    d->model->layoutModel(pCR->getRoot());
    if(node) {
        QModelIndex sel = d->model->getIndexByNode(node);
		if(ContentView::currentView)
			ContentView::currentView->selectionModel()->setCurrentIndex(sel, QItemSelectionModel::Select);
    }
}

void CRPluginCore::beginRemoveTreePaneItem(CRNode *sel_node, int i)
{
    d->model->beginRemoveItem(sel_node, i);
}

void CRPluginCore::endRemoveTreePaneItem()
{
    d->model->endRemoveItem();
}

void CRPluginCore::updateOpenDocs()
{
    // update (recolor) all open documents
    pCR->invalidateSignatures();
    QList<Core::IEditor *> openedEditors = Core::EditorManager::instance()->visibleEditors();
    Q_FOREACH(Core::IEditor *ieditor, openedEditors) {
        Core::IDocument *doc = ieditor->document();
        TextEditor::BaseTextEditor *editor = qobject_cast<TextEditor::BaseTextEditor *>(ieditor);
        recolorDoc(doc, editor);
    }
}

void CRPluginCore::onNodeMoveUp()
{
    // working with the current (last active) tree view
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(pCR->moveNodeUp(node))
        updateTreePane(node);
}

void CRPluginCore::onNodeMoveDown()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(pCR->moveNodeDown(node))
        updateTreePane(node);
}

void CRPluginCore::onNodeMoveLeft()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(pCR->moveNodeLeft(node))
        updateTreePane(node);
}

void CRPluginCore::onNodeMoveRight()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(pCR->moveNodeRight(node))
        updateTreePane(node);
}

void CRPluginCore::onNodeSetActive()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    pCR->setActive(node);
    updateTreePane();
}

void CRPluginCore::onNodeFindInFile()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(CR::isSys(node->type))
        return;
    node = node->getPathNode();
    if(!node)
        return;
    findAllEntries(node);
}

void CRPluginCore::onNodeFindAll()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(CR::isSys(node->type))
        return;
    findAllEntries(node);
}

void CRPluginCore::onNodeEdit()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(CR::isSys(node->type))
        return;
    NodeDlg dlg(true, node->getSysType()==CR::NT_OUTLINE, node->getPathNode(), node, node->type, Core::ICore::mainWindow());
    if(dlg.execDlg()) {
        updateTreePane();
        updateOpenDocs();
    }
}

void CRPluginCore::onNodeAddName()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    CRInfo info;
    NodeDlg dlg(false, node->getSysType()==CR::NT_OUTLINE, node->getPathNode(), &info,
                node->getSysNode()==pCR->getTags() ? CR::NT_TAG : CR::NT_AREA, Core::ICore::mainWindow());
    if(dlg.execDlg()) {
        CRNode* child = pCR->addNode(node, dlg.m_type, info);
        updateTreePane();
    }
}

void CRPluginCore::onNodeAddGroup()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    QString name = QInputDialog::getText(NULL, "Input group name",
           "Name:", QLineEdit::Normal, "");
    if(!name.isEmpty()) {
        CRNode* child = pCR->addNode(node, CR::NT_GROUP, _T(""), name.toStdWString(), _T(""), _T(""), CLR_NONE);
        updateTreePane();
    }
}

void CRPluginCore::onNodeAddFile()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    QString path = QFileDialog::getOpenFileName(m_wMain, tr("Select file"), "", "*.*");
    if(!path.isEmpty()) {
        QString name = QFileInfo(path).fileName();
        CRNode* child = pCR->addNode(node, CR::NT_FILE, _T(""), name.toStdWString(), path.toStdWString(), _T(""), CLR_NONE);
        updateTreePane();
    }
}

void CRPluginCore::onNodeAddLink()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    LinkDlg dlg(m_wMain);
    if(dlg.exec() == QDialog::Accepted) {
        QString name = QUrl(dlg.m_path).host();
        if(name.isEmpty())
            name = QFileInfo(dlg.m_path).fileName();
        CRNode* child = pCR->addNode(node, CR::NT_LINK, _T(""), name.toStdWString(), dlg.m_path.toStdWString(), _T(""), CLR_NONE);
        updateTreePane();
    }
}

void CRPluginCore::onNodeMoveTo()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    SelNodeDlg dlg(m_wMain);
    CRNode *locroot = pCR->getLocalRoot(node);
    if(dlg.execDlg(locroot, node)) {
        if(pCR->moveNodeTo(node, dlg.selNode))
            updateTreePane(node);
    }
}

void CRPluginCore::onNodeSetBase()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    QString path = QFileDialog::getSaveFileName(m_wMain, tr("Enter file name"),
        QString::fromStdWString(node->getDir()), "*.cr");
    if(!path.isEmpty()) {
        pCR->setIncPath(node, path.toStdWString().c_str());
    }
}

void CRPluginCore::onNodeRemove()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    if(CR::isSys(node->type))
        return;
    int ret = QMessageBox::warning(m_wMain, AppName, tr("Remove node?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {
        CRNode *par = node->parent;
        int n = par->branchIndex(node);
        beginRemoveTreePaneItem(par, n);
        if(pCR->removeNode(node)) {
            endRemoveTreePaneItem();
            updateOpenDocs();
        }
        else {
            QMessageBox::warning(m_wMain, AppName, tr("Unable to remove node"), QMessageBox::Ok);
        }
    }
}

void CRPluginCore::onNodeInfo()
{
    CRNode* node = getSelNode();
    if(!node) {
        QMessageBox::warning(m_wMain, AppName, tr("Null node!"), QMessageBox::Ok);
        return;
    }
    QString str = QString::fromStdWString( node->getInfo() );
    QMessageBox::warning(m_wMain, AppName, str, QMessageBox::Ok);
}

void CRPluginCore::onNodeCoverage()
{
    CRNode* node = getSelNode();
    if(!node)
        return;
    scan(ESM_COVERAGE, _T(""), node);
    outputPane()->showPage(1);
}

void CRPluginCore::onSettingsChanged()
{
    qDebug() << "onSettingsChanged";
}

void CRPluginCore::onNodeColorChanged(QColor clr)
{
    qDebug() << "onNodeColorChanged: " << clr;
//    if(ContentView::currentView)
//        ContentView::currentView->setFocus();
    CRNode* node = getSelNode();
    if(!node) return;
    node->tclr = clr.isValid() ? clr.rgb() : CLR_NONE;
    pCR->modify = true;
}


void CRPluginCore::onToolsCoverage()
{
    // enumeration of all project documents; which are open - we take from ide, closed ones we open ourselves
    // pass the root of the File System; otherwise it is impossible because need strict paths!
    //scanCoverage(pCR->getBase());
    scan(ESM_COVERAGE, _T(""), pCR->getFiles());
    outputPane()->showPage(1);
}


void CRPluginCore::onToolsEdit()
{
    //markMcmt(-2, CR::NT_NONE);
    editMcmt();
}

void CRPluginCore::onToolsLookup()
{
    // find the node under the mouse cursor
    CRNode *node = lookupNodeByCursor(nullptr);

    infoPane()->loadText(node);
    if(node)
        updateTreePane(node);
    // closed for now: I wanted to highlight the file node if I didn't find the usual one
    //else
    //    updateTreePane(fnode);
    infoPane()->showPage(1);
}

CRNode *CRPluginCore::getFNodeAndCursor(QTextCursor *pcursor, QString *pfpath)
{
    // sample code for working with qtc editor; separate into a function with two parameters ti and type
    using namespace TextEditor;
    BaseTextEditor *editor = BaseTextEditor::currentTextEditor();
    if (!editor)
        return qDebug("getNodeAndCursor: !editor"), nullptr;
    if(pcursor)
        *pcursor = editor->textCursor();

	// need the node of the file being edited and the current window
    // determine whether it is necessary to parse the document - we are looking for a file node with the same name
    Core::IDocument *doc = editor->document();
    if(!doc)
        return qDebug("getNodeAndCursor: !doc"), nullptr;

    // get file path
    QString fpath = doc->filePath().toString();
    if(pfpath)
        *pfpath = fpath;

    // get file node
    CRNode *fnode = pCR->getFileNode(fpath.toStdWString());
    // if there is no file node - construct a new node to the root(!!!)
    if(!fnode)
        fnode = pCR->addFileNode(fpath.toStdWString());

    return fnode;
}

void CRPluginCore::editRecentMcmt(int ri, CR::Type type)
{
    // ri: 0..n = recent
    ri -= RecentList::RECENT_COUNT;
    int ti = CRUser::Idx(type);

    EditRecentDlg dlg(&pCR->user.markButtons[ti].items[ri], CR::isSig(type), m_wMain);
    if(!dlg.execDlg())
        return;

    Gui::updateMarkMenu(&pCR->user.markButtons[ti]);
}

void CRPluginCore::editMcmt()
{
    // new editing option: look for the node and its position in the code
    // based on lookupNodeByCursor();
    QTextCursor cursor;
    QString fpath;
    CRNode *fnode = getFNodeAndCursor(&cursor, &fpath);
//@    if(!fnode) -- closed, suddenly we are without a file node?
//@        return;

    // text
    TextEditor::BaseTextEditor *editor = TextEditor::BaseTextEditor::currentTextEditor();
    if (!editor)
        return;
    // cursor position
    TextPos cpos = getCaretPos(cursor);

    // parsing
    CR::Mark info;

    QString src16 = editor->textDocument()->plainText();
    Parser<Utf16> parser((const wchar_t*)src16.utf16(), src16.size(), fnode);

    CRNode *enode = parser.parseFindNode(cpos.row, cpos.col, &info);
    // if not found - there is nothing there? (although it may be a non-working MC !!)
    if(!enode) //-- close because maybe without fnoda? What about tags and notes?
        return;
    // extract string by position
    QString str = extractLine(cursor, info.pos);

    // start editing
    MarkMaker maker;
    int r = maker.beginEditMarker(fpath.toStdWString(), str.toStdWString(), enode);
    if (r == 0)
        return;
    if(r == 2) {
        MarkupDlg dlg(fpath.toStdWString(), &maker.m_mi, &maker.m_ni, m_wMain);
        if(!dlg.execDlg())
            return;
    }
    // finish editing
    maker.endEditMarker(nullptr, nullptr);
	if(!maker.m_str.empty())
		editMarkup(cursor, QString::fromStdWString(maker.m_str));
    // update views
    updateTreePane();
    updateOpenDocs();
}

void CRPluginCore::markMcmt(int ri, CR::Type type)
{
    // ri: -2 = edit, -1 = new, 0 .. n = recent
    QTextCursor cursor;
    QString fpath;
    CRNode *fnode = getFNodeAndCursor(&cursor, &fpath);
//@    if(!fnode)  // the absence of a file node is not provided :( (but this, in theory, cannot be because the node, if not found, is constructed)
//@        return;-- there may be no file node!
    int index;
    QString str = extractCaretLine(cursor, &index);

    MarkMaker maker;
    int r = maker.beginMakeMarker(fpath.toStdWString(), ri, type, str.toStdWString(), index);
    if(r == 0)
        return;
    if(r == 2) {
        MarkupDlg dlg(fpath.toStdWString(), &maker.m_mi, &maker.m_ni, m_wMain);
        if(!dlg.execDlg())
            return;
    }

    maker.endMakeMarker(nullptr, nullptr);
    makeMarkup(cursor, QString::fromStdWString(maker.m_beg), QString::fromStdWString(maker.m_end));
    // interface update
    Gui::updateMarkMenu(&pCR->user.markButtons[CRUser::Idx(type)]);
    updateTreePane();
    updateOpenDocs();
    onToolsLookup();
}


void CRPluginCore::onMarkNewArea()
{
    markMcmt(-1, CR::NT_AREA);
}

void CRPluginCore::onMarkNewLabel()
{
    markMcmt(-1, CR::NT_LABEL);
}

void CRPluginCore::onMarkNewBlock()
{
    markMcmt(-1, CR::NT_BLOCK);
}

void CRPluginCore::onMarkNewLSig()
{
    markMcmt(-1, CR::NT_LSIG);
}

void CRPluginCore::onMarkNewBSig()
{
    markMcmt(-1, CR::NT_BSIG);
}

void CRPluginCore::onMarkNewName()
{
    markMcmt(-1, CR::NT_NAME);
}


void CRPluginCore::onMarkBtnArea()
{
    markMcmt(0, CR::NT_AREA);
}

void CRPluginCore::onMarkBtnLabel()
{
    markMcmt(0, CR::NT_LABEL);
}

void CRPluginCore::onMarkBtnBlock()
{
    markMcmt(0, CR::NT_BLOCK);
}

void CRPluginCore::onMarkBtnLSig()
{
    markMcmt(0, CR::NT_LSIG);
}

void CRPluginCore::onMarkBtnBSig()
{
    markMcmt(0, CR::NT_BSIG);
}

void CRPluginCore::onMarkBtnName()
{
    markMcmt(0, CR::NT_NAME);
}


void CRPluginCore::onMarkMenuArea(int i)
{
    if(i < RecentList::RECENT_COUNT)
        markMcmt(i, CR::NT_AREA);
    else
        editRecentMcmt(i, CR::NT_AREA);
}

void CRPluginCore::onMarkMenuLabel(int i)
{
    if(i < RecentList::RECENT_COUNT)
        markMcmt(i, CR::NT_LABEL);
    else
        editRecentMcmt(i, CR::NT_LABEL);
}

void CRPluginCore::onMarkMenuBlock(int i)
{
    if(i < RecentList::RECENT_COUNT)
        markMcmt(i, CR::NT_BLOCK);
    else
        editRecentMcmt(i, CR::NT_BLOCK);
}

void CRPluginCore::onMarkMenuLSig(int i)
{
    if(i < RecentList::RECENT_COUNT)
        markMcmt(i, CR::NT_LSIG);
    else
        editRecentMcmt(i, CR::NT_LSIG);
}

void CRPluginCore::onMarkMenuBSig(int i)
{
    if(i < RecentList::RECENT_COUNT)
        markMcmt(i, CR::NT_BSIG);
    else
        editRecentMcmt(i, CR::NT_BSIG);
}

void CRPluginCore::onMarkMenuName(int i)
{
    if(i < RecentList::RECENT_COUNT)
        markMcmt(i, CR::NT_NAME);
    else
        editRecentMcmt(i, CR::NT_NAME);
}


void CRPluginCore::makeFromPro(const QString &filters, bool upd)
{
    // make CR tree from files of PRO
    // filters to regexp
    QStringList extlist = filters.split(';');
    // get total list of files
    QStringList list = d->startupProject->files(ProjectExplorer::Project::SourceFiles);
    // apply
    if(!upd)
        d->filesInStartupProject.clear();
    Q_FOREACH(const QString &r, extlist)
        d->filesInStartupProject += list.filter(QRegExp(r, Qt::CaseInsensitive, QRegExp::Wildcard));

    Utils::FileName f = d->startupProject->projectDirectory();
    StrList sl;
    Q_FOREACH(QString s, d->filesInStartupProject)
        sl.push_back(s.toStdWString());

//  qDebug() << "makeFromPro == " << f << "upd=" << upd;
    if(upd)
        pCR->updateByLst(sl, settings()->splitByFiles);
    else {

        pCR->createByLst(d->startupProject->displayName().toStdWString(), f.toString().toStdWString(), sl, settings()->splitByFiles);

        QString suggestedPath =
                d->startupProject->projectDirectory().toString() // f
                + "/" + d->startupProject->displayName()         // d
                + AppFileExt;
        //setCurrentProject(suggestedPath, false); //@ok
    }
}

void CRPluginCore::openNode(CRNode *node)
{
    if(!node)
        return;

    if(CR::isFile(node->type)) {
        openFileAndLine(QString::fromStdWString(node->path), -1);
    }
    else if(CR::isMcmt(node->type)) {
        CRNode *fnode = node->getPathNode();
        if(!fnode)
            return;
        openFileAndMark(QString::fromStdWString(fnode->path),
                        QRegExp(QString::fromStdWString(makeMarkRegExp(node->id, node->type))));
    }
    else if(CR::isSig(node->type)) {
        if(node->getSysType() == CR::NT_TAGS) {
            scan(ESM_TOKENS, node->id, pCR->getFiles());
            outputPane()->showPage(1);
        }
        else {
            CRNode *fnode = node->getPathNode();
            if(!fnode)
                return;
            openFileAndMark(QString::fromStdWString(fnode->path),
                            QRegExp(QString::fromStdWString(makeSigRegExp(node->id))));
        }
    }
    else if(CR::isTag(node->type)) {
        scan(ESM_TAGS, node->id, pCR->getFiles());
        outputPane()->showPage(1);
    }
}

void CRPluginCore::findAllEntries(CRNode *node)
{
    if(!node)
        return;
    if(CR::isFile(node->type)) {
        openFileAndLine(QString::fromStdWString(node->path), -1);
    }
    else if(CR::isMcmt(node->type)) {
        scan(ESM_FUIDS, node->id, pCR->getFiles());
        outputPane()->showPage(1);
    }
    else if(CR::isSig(node->type)) {    // find ONE token, not a chain!
        scan(ESM_TOKENS, node->id, pCR->getFiles());
        outputPane()->showPage(1);
    }
    else if(CR::isTag(node->type)) {
        scan(ESM_TAGS, node->id, pCR->getFiles());
        outputPane()->showPage(1);
    }
}

void CRPluginCore::openFileAndLine(const QString &filePath, int line)
{
    // open file and highlight line by number
    if (!QFileInfo(filePath).exists())
        return;

    Core::IEditor *editor = Core::EditorManager::openEditor(filePath);
    editor->setUserData(CR::Constants::CR_UID, 0);
    if(editor && line >= 0) {
        editor->gotoLine(line);
    }
}

void CRPluginCore::openFileAndMark(const QString &filePath, const QRegExp &rexp)
{
    // open file and highlight line by metacomment or signature
    if (!QFileInfo(filePath).exists())
        return;

    Core::IEditor *editor = Core::EditorManager::openEditor(filePath);
    if (!editor)
        return;

    TextEditor::TextEditorWidget *editorWidget = qobject_cast<TextEditor::TextEditorWidget*>(editor->widget());
    if (!editorWidget)
        return;

    editorWidget->moveCursor(QTextCursor::Start);
    bool r = editorWidget->find(rexp, QTextDocument::FindWholeWords | QTextDocument::FindCaseSensitively);
    if(!r) {
        QMessageBox::question(m_wMain, AppName, tr("Mark not found"), QMessageBox::Ok);
    }
}

void CRPluginCore::onAboutToClose()
{
    qDebug() << "CRPluginCore::onAboutToClose:";
    if(pCR->modify) {
        if(QMessageBox::question(m_wMain, AppName, tr("Your CR data was changed. Save changes?"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
            return;
        if(!pCR->saveCR(pCR->pathRecentProject))
            onProjectSaveAs();
    }
    // disconnect all

}

void CRPluginCore::onSaveAll(bool)
{
    onProjectSave();
}

void CRPluginCore::onClearSession(bool)
{
    qDebug() << "onClearSession";
    clearProject();
}

struct QtcScanInfo
{
    QList<Core::IEditor *> openedEditors;
    CR::Internal::OutputPane* outputPane;
};


void qtc_addMsgWrapper(void *arg, const Char* fpath, int line, const Char* msg)
{
    // qtc add message wrapper implementation
    QtcScanInfo *info = (QtcScanInfo*)arg;
    info->outputPane->addMessage(QString::fromUtf16((const ushort*)fpath), line, QString::fromUtf16((const ushort*)msg));
}

int qtc_getSrcWrapper(void *arg, CRNode *node, int mode, const String &str, const Char* fpath)
{
    // open source wrapper implementation for qtc
    QtcScanInfo *info = (QtcScanInfo*)arg;
    Core::IEditor* ieditor = CRPluginCore::getOpenedEditor(info->openedEditors, QString::fromStdWString(fpath));
    if(!ieditor)
        return 0;
    TextEditor::BaseTextEditor *editor = qobject_cast<TextEditor::BaseTextEditor *>(ieditor);
    if(!editor)
        return 0;

    QString src16 = editor->textDocument()->plainText();
    scanBuf<Utf16>(node, mode, str, (const wchar_t*)src16.utf16(), src16.size(), qtc_addMsgWrapper, arg);
    return 1;
}


void CRPluginCore::scan(int mode, const String &str, CRNode *root)
{
	// generic scan function
    // if the file is open in the IDE, then we take the code from the IDE, otherwise we open it and read from the disk
    QtcScanInfo info;
    info.openedEditors = Core::EditorManager::instance()->visibleEditors();
    info.outputPane = d->outputPane;
    d->outputPane->clearContents();

    scanCR<Utf16>(root, str, mode, qtc_getSrcWrapper, qtc_addMsgWrapper, &info);
}

TextPos CRPluginCore::getCaretPos(QTextCursor &cursor)
{
    int posStart = cursor.selectionStart();
    cursor.setPosition(posStart);
    TextPos cpos;
    cpos.row = cursor.blockNumber();
    cpos.col = cursor.positionInBlock();
    return cpos;
}

CRNode* CRPluginCore::lookupNodeByCursor(CR::Mark *pm)
{
    // search node by cursor
    QTextCursor cursor;
    CRNode *fnode = getFNodeAndCursor(&cursor, nullptr);
    if(!fnode)
        return nullptr;
    // text
    TextEditor::BaseTextEditor *editor = TextEditor::BaseTextEditor::currentTextEditor();
    if (!editor)
        return nullptr;

    // cursor position
    TextPos cpos = getCaretPos(cursor);

    // parsing
    QString src16 = editor->textDocument()->plainText();
    Parser<Utf16> parser((const wchar_t*)src16.utf16(), src16.size(), fnode);
    CRNode *enode = parser.parseFindNode(cpos.row, cpos.col, pm);

	// special logic here: if pm==nullptr, then you can try to return fnode instead of node
    // but it doesn't seem to be used
    if(!pm && !enode)
        return fnode;
    return enode;
}


void CRPluginCore::onNodeCheckChanged(CRNode *node)
{
    //qDebug() << "onNodeCheckChanged: " << node->dispName();
    node->updateSubtreeVisibility(!!(node->flags & CRNode::NF_CHECKED));
    updateOpenDocs();
}
