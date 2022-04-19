#include "nodedlg.h"
#include "ui_nodedlg.h"
#include "../widgets/colorbutton.h"
#include "../gui.h"
#include "../../core/core.h"
#include "../../core/mcmt.h"
#include <QMessageBox>
#include <QFileDialog>

NodeDlg::NodeDlg(bool editMode, bool enPath, CRNode *nodeFile, CRInfo *infoEdit, CR::Type type, QWidget *parent)
    : m_nodeFile(nodeFile)
    , m_infoEdit(infoEdit)
    , m_type(type)
    , QDialog(parent)
    , ui(new Ui::NodeDlg)
    , m_editMode(editMode)
    , m_enPath(enPath)
{
    ui->setupUi(this);

    connect(ui->pushOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushOverview, SIGNAL(clicked()), this, SLOT(onPathOverview()));

    if(type == CR::NT_TAG) {
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_TAG), tr("Tag"), [this](){onSelType(CR::NT_TAG);});
        ui->labelId->setText("Tag");
        ui->lineId->setToolTip("Tag");
        ui->lineId->setPlaceholderText("Tag");
    }
    else {
        if(CR::isSig(type)) {
            ui->labelId->setText("Signature");
            ui->lineId->setToolTip("Signature");
            ui->lineId->setPlaceholderText("Signature");
        }
        else {
            ui->labelId->setText("Id");
            ui->lineId->setToolTip("Id");
            ui->lineId->setPlaceholderText("Id");
        }
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_AREA), tr("Area"), [this](){onSelType(CR::NT_AREA);});
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_LABEL), tr("Label"), [this](){onSelType(CR::NT_LABEL);});
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_BLOCK), tr("Block"), [this](){onSelType(CR::NT_BLOCK);});
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_LSIG), tr("LSig"), [this](){onSelType(CR::NT_LSIG);});
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_BSIG), tr("BSig"), [this](){onSelType(CR::NT_BSIG);});
        m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_NAME), tr("Name"), [this](){onSelType(CR::NT_NAME);});
    }
    m_typesMenu.addSeparator();
    m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_GROUP), tr("Group"), [this](){onSelType(CR::NT_GROUP);});
    m_typesMenu.addAction(Gui::getTreeIcon(CR::NT_LINK), tr("Link"), [this](){onSelType(CR::NT_LINK);});

    ui->pushType->setMenu(&m_typesMenu);
    onSelType(type);

    updateGui();

    ui->pushType->setEnabled(!m_editMode);

    ui->lineId->setFocus();
}

NodeDlg::~NodeDlg()
{
    delete ui;
}

void NodeDlg::updateGui()
{
    bool en = m_enPath || !m_editMode && CR::isFile(m_type);
    ui->linePath->setEnabled(en);
    ui->pushOverview->setEnabled(en);
    ui->labelPath->setEnabled(en);
}

void NodeDlg::onSelType(CR::Type type)
{
    m_type = type;
    ui->pushType->setText(QString::fromUtf16((const ushort *)CR::typeName(type)));
    ui->pushType->setIcon(Gui::getTreeIcon(type));

    updateGui();
}

bool NodeDlg::execDlg()
{
//    setWindowTitle("Edit node");
    // from node
    ui->lineName->setText( QString::fromStdWString(m_infoEdit->name) );
    ui->lineId->setText( QString::fromStdWString(m_infoEdit->id) );
    ui->linePath->setText( QString::fromStdWString(m_infoEdit->path) );
    ui->textText->setPlainText( QString::fromStdWString(m_infoEdit->text) );
    ui->clrNode->setColor(clrValid(m_infoEdit->nclr) ? QColor(m_infoEdit->nclr) : QColor());

    return QDialog::exec() == QDialog::Accepted;
}

void NodeDlg::accept()
{
    QString id = ui->lineId->text().trimmed();

    if(id.isEmpty()) {
        QMessageBox::warning(this, AppName, tr("ID field is empty!"), QMessageBox::Ok);
        return;
    }
    if(CR::isMcmt(m_type) && (!m_infoEdit || m_infoEdit->id!=id.toStdWString()) && (m_nodeFile && m_nodeFile->findChildId(id.toStdWString()))) {
        QMessageBox::warning(this, AppName, tr("ID is already in use!"), QMessageBox::Ok);
        return;
    }
    if(!CR::isSig(m_type) && (id.indexOf(' ')>=0 || id.indexOf('#')>=0 || id.indexOf('@')>=0
            || id.indexOf('$')>=0 || id.indexOf('>')>=0 || id.indexOf('<')>=0)) {
        QMessageBox::warning(this, AppName, tr("ID contains illegal characters"), QMessageBox::Ok);
        return;
    }

    // to node
    m_infoEdit->name = ui->lineName->text().toStdWString();
    m_infoEdit->id = id.toStdWString();
    m_infoEdit->text = ui->textText->toPlainText().toStdWString();
    m_infoEdit->nclr = ui->clrNode->getColor().isValid() ? ui->clrNode->getColor().rgb() : CLR_NONE;
    if(CR::isFile(m_type))
        m_infoEdit->path = ui->linePath->text().toStdWString();
    pCR->modify = true;

    QDialog::accept();
}

void NodeDlg::onPathOverview()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select file"), "", "*.*");
    if(!path.isEmpty())
        ui->linePath->setText(path);
}


