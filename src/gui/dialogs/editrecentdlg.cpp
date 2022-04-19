#include "editrecentdlg.h"
#include "ui_editrecentdlg.h"
#include "../widgets/colorbutton.h"
#include "../gui.h"
#include "../../core/mcmt.h"

EditRecentDlg::EditRecentDlg(RecentItem *rid, bool is_sig, QWidget *parent)
    : m_recentData(rid)
    , QDialog(parent)
    , m_sig(is_sig)
    , ui(new Ui::EditRecentDlg)
{
    ui->setupUi(this);

    if(!is_sig)
        ui->checkTsig->setChecked(0);
    ui->checkTsig->setVisible(is_sig);

    connect(ui->pushOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->clrMark, &ColorButton::colorChanged, this, &EditRecentDlg::onChangeColor);
}

EditRecentDlg::~EditRecentDlg()
{
    delete ui;
}

bool EditRecentDlg::execDlg()
{
    // from markup
    ui->lineId->setText( QString::fromStdWString(m_recentData->mark) );
    ui->checkFiles->setChecked( m_recentData->mk_fuid );
    ui->checkNotes->setChecked( m_recentData->mk_elem );
	ui->checkGenID->setChecked( m_recentData->gen_uid );
    if(m_sig)
        ui->checkTsig->setChecked( m_recentData->mk_tsig );
    m_clrMark = m_recentData->nclr;
    ui->clrMark->setColor(m_clrMark);

    return QDialog::exec() == QDialog::Accepted;
}

void EditRecentDlg::accept()
{
    QString id = ui->lineId->text().trimmed();
    // to markup
    m_recentData->mark = id.toStdWString();
    m_recentData->mk_fuid = ui->checkFiles->isChecked();
    m_recentData->mk_elem = ui->checkNotes->isChecked();
	m_recentData->gen_uid = ui->checkGenID->isChecked();
    if(m_sig)
        m_recentData->mk_tsig = ui->checkTsig->isChecked();
    m_recentData->nclr = m_clrMark.isValid() ? m_clrMark.rgb() : CLR_NONE;

    QDialog::accept();
}

void EditRecentDlg::onChangeColor(QColor clr)
{
    m_clrMark = clr;
}



