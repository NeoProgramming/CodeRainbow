#include "memoform.h"
#include "ui_memoform.h"

MemoForm::MemoForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MemoForm)
{
    ui->setupUi(this);

    connect(ui->noteEdit, &NoteEdit::lostFocus, this, &MemoForm::onEditorLostFocus);
//    connect
}

MemoForm::~MemoForm()
{
    disconnect(ui->noteEdit, &NoteEdit::lostFocus, this, &MemoForm::onEditorLostFocus);
    delete ui;
}

void MemoForm::loadText(CRNode *node)
{
    m_node = node;
    if(!node) {
        ui->noteEdit->clear();
        ui->noteEdit->setEnabled(false);
        ui->linePath->setText("Attention! This item has not node in tree!");
        ui->pushFix->show();
        ui->linePath->setStyleSheet("background-color:red;color:yellow");
    }
    else {
        ui->noteEdit->setPlainText(QString::fromStdWString(node->text));
        ui->noteEdit->setEnabled(true);
        ui->linePath->setText(QString::fromStdWString(node->fullName()));
        ui->pushFix->hide();
        ui->linePath->setStyleSheet("background-color:lightgreen;color:black");
    }
}

void MemoForm::onEditorLostFocus()
{
    if(m_node)
        emit textUpdated(m_node, ui->noteEdit->toPlainText());
}
