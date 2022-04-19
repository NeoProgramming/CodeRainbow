#pragma once

#include "../core/mark.h"
#include <coreplugin/ioutputpane.h>

class CRNode;
namespace CR {
class CRPluginCore;
namespace Internal {

class InfoPanePrivate;
class InfoPane : public Core::IOutputPane
{
    Q_OBJECT
public:
    InfoPane(CRPluginCore *parent = 0);
    ~InfoPane();
    void loadText(CRNode *node);
public slots:
    void onTextUpdated(CRNode *node, const QString &text);
public:
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


private:
    InfoPanePrivate* const d;
};

} // namespace Internal
} // namespace CR
