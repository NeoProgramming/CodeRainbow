#include "contentwidgetfactory.h"

#include "crmodel.h"
#include "contentview.h"

ContentWidgetFactory::ContentWidgetFactory(CRModel *mdl)
    : model(mdl)
{
    setDisplayName(QLatin1String("CRTree"));
}

QString ContentWidgetFactory::displayName() const
{
    return QLatin1String("CRTree");
}

int ContentWidgetFactory::priority() const
{
    return 110;
}

Core::Id ContentWidgetFactory::id() const
{
    return Core::Id("CRTree");
}

Core::NavigationView ContentWidgetFactory::createWidget()
{
    Core::NavigationView n;
    ContentView *navigationView = new ContentView(model);
    n.widget = navigationView;
    return n;
}
