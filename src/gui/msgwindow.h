#ifndef MSGWINDOW_H
#define MSGWINDOW_H

#include <QWidget>

class MsgWindow : public QWidget
{
	Q_OBJECT

public:
	MsgWindow(QWidget *parent = 0);
	~MsgWindow();

};

#endif // MSGWINDOW_H
