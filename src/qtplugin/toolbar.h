#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QIcon>

namespace Core {
class IEditor;
}

namespace CR {

class CRPluginCore;

namespace Internal {



class ToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget *parent = 0);
};

} // namespace Internal
} // namespace CR

#endif // TOOLBAR_H
