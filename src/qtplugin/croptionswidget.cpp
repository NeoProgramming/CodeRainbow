#include "croptionswidget.h"
#include "ui_croptionswidget.h"
#include "crsettings.h"
#include <utils/utilsicons.h>

using namespace CR::Internal;

CROptionsWidget::CROptionsWidget(const CR::Internal::CRSettings * const settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CROptionsWidget)
{
    ui->setupUi(this);
    updateWithSettings(settings);
}

CROptionsWidget::~CROptionsWidget()
{
    delete ui;
}

const CRSettings &CROptionsWidget::settings()
{
    m_settings.showTabs = ui->checkTabs->isChecked();
    m_settings.showHL = ui->checkLight->isChecked();
    m_settings.splitByFiles = ui->checkSF->isChecked();
    return m_settings;
}

void CROptionsWidget::applySettings()
{
    emit applyCurrentSetSettings();
}

void CROptionsWidget::updateWithSettings(const CRSettings * const settings)
{
    ui->checkTabs->setChecked(settings->showTabs);
    ui->checkLight->setChecked(settings->showHL);
    ui->checkSF->setChecked(settings->splitByFiles);
}
