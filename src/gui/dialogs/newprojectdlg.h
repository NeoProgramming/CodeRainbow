#ifndef NEWPROJECTDLG_H
#define NEWPROJECTDLG_H

#include <QDialog>
#include "ui_newprojectdlg.h"

class NewProjectDlg : public QDialog
{
	Q_OBJECT

public:
	NewProjectDlg(QWidget *parent = 0);
	~NewProjectDlg();
    bool execDlg();
public slots:
	void onOk();
	void onOverview();
public:
    QString m_name, m_base, m_filters;
private:
	Ui::NewProjectDlg ui;
};

#endif // NEWPROJECTDLG_H
