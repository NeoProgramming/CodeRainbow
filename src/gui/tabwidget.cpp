#include "tabwidget.h"
#include <QTabBar>

TabWidget::TabWidget(QWidget *parent) : QMdiArea(parent) 
{
	QList<QTabBar *> tabBarList = findChildren<QTabBar*>();
	m_tabBar = tabBarList.at(0);
}

TabWidget::~TabWidget() 
{
}

void TabWidget::contextMenuEvent(QContextMenuEvent* e) 
{
    Q_UNUSED(e);
    emit contextMenuAt(QCursor::pos(), m_tabBar->tabAt(mapFromGlobal(QCursor::pos())));
}

int TabWidget::count()
{
	return m_tabBar->count();
}

