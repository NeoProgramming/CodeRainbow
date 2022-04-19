#include "prjpropsdlg.h"
#include "ui_prjpropsdlg.h"

PrjPropsDlg::PrjPropsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrjPropsDlg)
{
    ui->setupUi(this);

    connect(ui->pushOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushDef, SIGNAL(clicked()), this, SLOT(onRestoreDefs()));
}

PrjPropsDlg::~PrjPropsDlg()
{
    delete ui;
}

void PrjPropsDlg::load()
{
    ui->lineCmt->setText(QString::fromStdWString(pp.cmt));
    ui->lineArea->setText(QString::fromStdWString(pp.area));
    ui->lineLabel->setText(QString::fromStdWString(pp.label));
    ui->lineEnd->setText(QString::fromStdWString(pp.aend));
}

bool PrjPropsDlg::execDlg()
{
    load();
    return QDialog::exec() == QDialog::Accepted;
}

void PrjPropsDlg::accept()
{
    pp.cmt = ui->lineCmt->text().toStdWString();
    pp.area = ui->lineArea->text().toStdWString();
    pp.label = ui->lineLabel->text().toStdWString();
    pp.aend = ui->lineEnd->text().toStdWString();
}

void PrjPropsDlg::onRestoreDefs()
{
    pp.initDef();
    load();
}

