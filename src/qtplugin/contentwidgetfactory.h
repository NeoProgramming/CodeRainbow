#pragma once

#include <coreplugin/inavigationwidgetfactory.h>

class CRModel;

class ContentWidgetFactory : public Core::INavigationWidgetFactory
{
    CRModel *model;
public:    
    ContentWidgetFactory(CRModel *mdl);

    QString displayName() const;
    int priority() const;
    Core::Id id() const;
    Core::NavigationView createWidget();
};
