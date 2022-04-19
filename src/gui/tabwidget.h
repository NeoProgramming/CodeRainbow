#pragma once
#include <QTabWidget>
#include <QTabBar>
#include <QMdiArea>

class TabWidget : public QMdiArea
{
    Q_OBJECT

public:
    TabWidget(QWidget *parent = 0);
    ~TabWidget();

	int count();
	QTabBar *tabBar()
	{
		return m_tabBar;
	}
signals:
    void contextMenuAt(const QPoint &, int _tab);

protected:
    void contextMenuEvent(QContextMenuEvent *);

protected:
	QTabBar *m_tabBar;
};

