#pragma once

#include "../core/mark.h"
#include <coreplugin/ioutputpane.h>
#include <QTreeWidgetItem>

namespace CR {
class CRPluginCore;

namespace Internal {

class SpellingMistakesModel;

class OutputPanePrivate;
class OutputPane : public Core::IOutputPane
{
    Q_OBJECT
public:
    OutputPane(CRPluginCore *parent = 0);
    ~OutputPane();

    QWidget *outputWidget(QWidget *parent);
    QList<QWidget*> toolBarWidgets() const;
    QString displayName() const;
    int priorityInStatusBar() const;
    void clearContents();
    void visibilityChanged(bool visible);
    void setFocus();
    bool hasFocus() const;
    bool canFocus() const;
    bool canNavigate() const;
    bool canNext() const;
    bool canPrevious() const;
    void goToNext();
    void goToPrev();

    void addMessage(const QString &path, int line, const QString &str);
    void clearMessages();
public slots:
    void onItemDoubleClicked(QTreeWidgetItem *item);
private:
    OutputPanePrivate* const d;
};

} // namespace Internal
} // namespace CR
