#include "croptionspage.h"
#include "crsettings.h"
#include "croptionswidget.h"
#include "crconstants.h"
#include "crplugin.h"
#include "crplugincore.h"

using namespace CR::Internal;

CROptionsPage::CROptionsPage(CRSettings *settings, QObject *parent) :
    Core::IOptionsPage(parent),
    m_settings(settings)
{
    setId("CR::Settings");
    setDisplayName(tr("CodeRainbow"));
    setCategory("CodeRainbow");
    setDisplayCategory(tr("CodeRainbow"));
    setCategoryIcon(Utils::Icon(QLatin1String(Constants::ICON_CRPLUGIN)));
}

CROptionsPage::~CROptionsPage()
{
}

bool CROptionsPage::matches(const QString &searchKeyWord) const
{
    return (searchKeyWord == QLatin1String("CR") || searchKeyWord == QLatin1String("CodeRainbow"));
}

QWidget *CROptionsPage::widget()
{
    if(m_widget == nullptr) {
        m_widget = new CROptionsWidget(m_settings);
        connect(m_widget, &CROptionsWidget::applyCurrentSetSettings, m_crPCore, &CRPluginCore::onSettingsChanged);
        connect(m_widget, &CROptionsWidget::applyCurrentSetSettings, CRPlugin::instance(), &CRPlugin::onSettingsChanged);
    }
    return m_widget;
}

void CROptionsPage::apply()
{
    Q_ASSERT(m_widget != nullptr);
    m_widget->applySettings();
    *m_settings = m_widget->settings();
}

void CROptionsPage::finish()
{
//    qDebug() << "finish";
}
