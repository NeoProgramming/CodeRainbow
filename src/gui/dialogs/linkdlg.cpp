#include "linkdlg.h"
#include "ui_linkdlg.h"
#include <QDesktopServices>
#include <QClipboard>
#include <QUrl>
#include <QFileDialog>

LinkDlg::LinkDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LinkDlg)
{
    ui->setupUi(this);
    ui->lineLink->setText(m_path);
    connect(ui->pushOpen, SIGNAL(clicked()), this, SLOT(onOpen()));
    connect(ui->pushPaste, SIGNAL(clicked()), this, SLOT(onPaste()));
    connect(ui->pushSelect, SIGNAL(clicked()), this, SLOT(onSelect()));
    connect(ui->pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
    connect(ui->pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

LinkDlg::~LinkDlg()
{
    delete ui;
}

void LinkDlg::onOpen()
{
    QDesktopServices::openUrl(QUrl(ui->lineLink->text()));
}

void LinkDlg::onPaste()
{
    QClipboard *clipboard = QApplication::clipboard();
    ui->lineLink->setText( clipboard->text() );
}

void LinkDlg::onSelect()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select file"), ui->lineLink->text(), "*.*");
    if(!path.isEmpty())
        ui->lineLink->setText(path);
}

void LinkDlg::onOk()
{
    m_path = ui->lineLink->text();
    QDialog::accept();
}
