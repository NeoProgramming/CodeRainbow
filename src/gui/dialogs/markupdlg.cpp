#include "markupdlg.h"
#include "ui_markupdlg.h"
#include "../widgets/colorbutton.h"
#include "../gui.h"
#include "../../core/core.h"
#include "../../core/mcmt.h"
#include <QMessageBox>
#include "selnodedlg.h"

MarkupDlg::MarkupDlg(const String &fpath, CRMark *markEdit, CRInfo *infoEdit, QWidget *parent)
    : m_markEdit(markEdit)  // parsed metacomment
    , m_infoEdit(infoEdit)  // structure for filling in information about a unique node
    , QDialog(parent)
    , ui(new Ui::MarkupDlg)
    , m_autoFillPath(true)
{
    ui->setupUi(this);

    // to determine uniqueness within a file; the node may not be found - this is normal
    m_nodeFile = pCR->getFileNode (fpath);

    if(CR::isSig(markEdit->type)) {
        ui->plainTags->hide();
        ui->labelId->setText("Sig:");
        ui->pushGen->hide();
        ui->pushAddTag->hide();
    }
    else {
        ui->labelId->setText("Id:");
    }

    connect(ui->pushOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushGen, &QPushButton::clicked, this, &MarkupDlg::onGenUniqueId);
    connect(ui->checkInline, &QCheckBox::clicked, this, &MarkupDlg::onClickedCheckInline);
    connect(ui->clrMark, &ColorButton::colorChanged, this, &MarkupDlg::onChangeColor);
    connect(ui->pushAddTag, &QPushButton::clicked, this, &MarkupDlg::onAddTag);
	connect(ui->pushClear, &QPushButton::clicked, this, &MarkupDlg::onClear);
	connect(ui->comboRecentColors, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MarkupDlg::onRecentColorChanged);
	connect(ui->comboPredefColors, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MarkupDlg::onPredefColorChanged);

    connect(ui->checkFiles, &QCheckBox::clicked, this, &MarkupDlg::onClickedCheckFilesOutline);
    connect(ui->checkTags, &QCheckBox::clicked, this, &MarkupDlg::onClickedCheckTags);
    connect(ui->checkOutline, &QCheckBox::clicked, this, &MarkupDlg::onClickedCheckFilesOutline);
}

MarkupDlg::~MarkupDlg()
{
    delete ui;
}

bool MarkupDlg::execDlg()
{
    if(CR::isSig(m_markEdit->type)) {
        ui->checkInline->hide();
        ui->pushGen->hide();
        ui->checkInline->resize(0,0);
        ui->pushGen->resize(0,0);
    }

    // from markup
    ui->lineId->setText( QString::fromStdWString(m_markEdit->id) );
    ui->plainTags->setPlainText( QString::fromStdWString(m_markEdit->tagStr())); //@ todo: QTagsWidget
    ui->checkFiles->setChecked( m_markEdit->mk_fuid );
    ui->checkTags->setChecked( m_markEdit->mk_tsig );
    ui->checkOutline->setChecked( m_markEdit->mk_elem );
	ui->checkGenID->setChecked(m_markEdit->gen_uid);

    onClickedCheckFilesOutline();
    onClickedCheckTags();

    // from node
    ui->lineName->setText( QString::fromStdWString(m_infoEdit->name) );
    ui->textText->setPlainText(QString::fromStdWString(m_infoEdit->text));

    // color
    ui->checkInline->setChecked(clrValid(m_markEdit->mclr));
    m_clrMark = clrValid(m_markEdit->mclr) ? QColor(m_markEdit->mclr) : QColor();
    m_clrNode = clrValid(m_infoEdit->nclr) ? QColor(m_infoEdit->nclr) : QColor();
    ui->clrMark->setColor(clrValid(m_markEdit->mclr) ? m_clrMark : m_clrNode);

    // recent & predef colors
    Gui::loadColorCombo(ui->comboRecentColors, pCR->user.clrRecent);
    Gui::loadColorCombo(ui->comboPredefColors, pCR->user.clrPredef);

    return QDialog::exec() == QDialog::Accepted;
}

void MarkupDlg::accept()
{
    bool toFiles = ui->checkFiles->isChecked();
    bool toTags = ui->checkTags->isChecked();   // for MC this is the permission of the TAGS field, for SIG this is the use of the signature (id) as a tag
    bool toOutline = ui->checkOutline->isChecked();

    //@ Char* msg = pCR->canAcceptMarkup(toFiles, toTags, toOutline, id, ts);

    // checking if we are only creating MK without entries in the tree
    if (!toFiles && !toOutline && !toTags) {
        QMessageBox::warning(this, AppName, tr("No nodes will be created! Check 'Files', 'Tags' and/or 'Outline' checkboxes"), QMessageBox::Ok);
        return;
    }

    // getting strings with id/signature and tags
    QString id, ts;
    id = ui->lineId->text().trimmed();
    if(toTags && !CR::isSig(m_markEdit->type)) {
        ts = ui->plainTags->toPlainText().trimmed();
        ts.replace(' ', ',');
        ts.replace('\n', ',');
        ts.replace(QChar::ParagraphSeparator, ',');
    }

    // id and tags missing at the same time
    if(CR::isSig(m_markEdit->type)) {
        if(id.isEmpty()) {
            QMessageBox::warning(this, AppName, tr("Signature is empty!"), QMessageBox::Ok);
            return;
        }
    }
    else {
        if(id.isEmpty() && ts.isEmpty()) {
            QMessageBox::warning(this, AppName, tr("ID and Tags are empty!"), QMessageBox::Ok);
            return;
        }
    }

    // the identifier has changed and this is in the file
    if((m_markEdit->id != id.toStdWString()) && (m_nodeFile && m_nodeFile->findChildId(id.toStdWString()))) {
        QMessageBox::warning(this, AppName, tr("ID is already in use!"), QMessageBox::Ok);
        return;
    }

    // not a signature and invalid characters in the identifier
    if(!CR::isSig(m_markEdit->type) && (id.indexOf(' ')>=0 || id.indexOf('#')>=0 || id.indexOf('@')>=0
            || id.indexOf('$')>=0 || id.indexOf('>')>=0 || id.indexOf('<')>=0)) {
        QMessageBox::warning(this, AppName, tr("ID contains illegal characters"), QMessageBox::Ok);
        return;
    }

    // not a signature and invalid characters in the tags
    if(!CR::isSig(m_markEdit->type) && (ts.indexOf(' ')>=0 || ts.indexOf('#')>=0 || ts.indexOf('@')>=0
            || ts.indexOf('$')>=0 || ts.indexOf('>')>=0 || ts.indexOf('<')>=0)) {
        QMessageBox::warning(this, AppName, tr("Tags contains illegal characters"), QMessageBox::Ok);
        return;
    }
	
    // to markup
    m_markEdit->id = id.toStdWString();
    m_markEdit->setTags(ts.toStdWString());

    // bits; tsig creates a signature in tags only for isSig(), this is hardwired into markmaker
    m_markEdit->mk_fuid = toFiles;
    m_markEdit->mk_elem = toOutline;
    m_markEdit->mk_tsig = toTags;
	m_markEdit->gen_uid = ui->checkGenID->isChecked();

    // to node
    m_infoEdit->id = id.toStdWString();
    m_infoEdit->name = ui->lineName->text().toStdWString();
    m_infoEdit->text = ui->textText->toPlainText().toStdWString();

    // color
    bool chk = ui->checkInline->isChecked();
    m_markEdit->mclr = (chk && m_clrMark.isValid()) ? m_clrMark.rgb() : CLR_NONE;
    m_infoEdit->nclr = m_clrNode.isValid() ? m_clrNode.rgb() : CLR_NONE;

    QDialog::accept();
}

void MarkupDlg::onClickedCheckInline()
{
    bool chk = ui->checkInline->isChecked();
    if(chk)
        ui->clrMark->setColor(m_clrMark);
    else
        ui->clrMark->setColor(m_clrNode);
}

void MarkupDlg::onChangeColor(QColor clr)
{

    ui->comboRecentColors->setCurrentIndex(0);
    ui->comboPredefColors->setCurrentIndex(0);
    updateCurrColor(clr);
}

void MarkupDlg::onGenUniqueId()
{
    String id = ui->lineId->text().trimmed().toStdWString();
    if(m_nodeFile)
        ensureUniqueId(m_nodeFile, id);
    ui->lineId->setText( QString::fromStdWString( id ) );
}

void MarkupDlg::onAddTag()
{
    SelNodeDlg dlg(this);
    if(dlg.execDlg(pCR->getTags(), nullptr)) {
        if(dlg.selNode && CR::isTag(dlg.selNode->type)) {
            QString ts = ui->plainTags->toPlainText();
            QStringList sl = ts.split(',', QString::SkipEmptyParts);
            sl.push_back(QString::fromStdWString(dlg.selNode->id));
            ts = sl.join(',');
            ui->plainTags->setPlainText(ts);
        }
    }
}

void MarkupDlg::onClear()
{
	ui->lineId->clear();
	ui->plainTags->clear();
}

void MarkupDlg::onRecentColorChanged(int index)
{
    ui->clrMark->setColor(QColor());
    ui->comboPredefColors->setCurrentIndex(0);
    if(index==0)
        updateCurrColor(CLR_NONE);
    else
        updateCurrColor(pCR->user.clrRecent[index-1].clr);
}

void MarkupDlg::onPredefColorChanged(int index)
{
    ui->clrMark->setColor(QColor());
    ui->comboRecentColors->setCurrentIndex(0);
    if(index==0)
        updateCurrColor(CLR_NONE);
    else
        updateCurrColor(pCR->user.clrPredef[index-1].clr);
}

void MarkupDlg::updateCurrColor(QColor clr)
{
    bool chk = ui->checkInline->isChecked();
    if(chk)
        m_clrMark = clr;
    else
        m_clrNode = clr;
}

void MarkupDlg::onClickedCheckFilesOutline()
{
    bool chk1 = ui->checkFiles->isChecked();
    bool chk2 = ui->checkOutline->isChecked();
    ui->lineId->setEnabled(chk1 || chk2);
}

void MarkupDlg::onClickedCheckTags()
{
    bool chk = ui->checkTags->isChecked();
    ui->plainTags->setEnabled(chk);
    ui->pushAddTag->setEnabled(chk);
}


