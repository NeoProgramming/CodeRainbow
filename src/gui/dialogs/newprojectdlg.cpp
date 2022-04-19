#include "newprojectdlg.h"
#include <QFileDialog>
#include "../../core/core.h"

NewProjectDlg::NewProjectDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.pushOk,		 SIGNAL(clicked()), this, SLOT(onOk()));
	connect(ui.toolOverview, SIGNAL(clicked()), this, SLOT(onOverview()));
	connect(ui.pushCancel,	 SIGNAL(clicked()), this, SLOT(reject()));

    if(m_filters.isEmpty())
        m_filters = SrcFilters;
    ui.lineFilters->setText(m_filters);
}

NewProjectDlg::~NewProjectDlg()
{

}

bool NewProjectDlg::execDlg()
{
    return QDialog::exec() == QDialog::Accepted;
}

void NewProjectDlg::onOk()
{
	m_name = ui.lineTitle->text();
	m_base = ui.lineBase->text();
    m_filters = ui.lineFilters->text();
	accept();
}

void NewProjectDlg::onOverview()
{
    QString dir = QFileDialog::getExistingDirectory ( this, tr("Select base folder"));
    if (!dir.isEmpty()) {
        ui.lineBase->setText(dir);
        if(ui.lineTitle->text().isEmpty())
            ui.lineTitle->setText( QDir(dir).dirName() );
	}
}

