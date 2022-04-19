#include "crsettings.h"
#include "../qtplugin/crconstants.h"

using namespace CR::Internal;

CRSettings::CRSettings()
    : QObject(nullptr)
    , showTabs(true)
    , showHL(true)
    , splitByFiles(true)
{
}

CRSettings::CRSettings(const CRSettings &settings)
    : QObject(settings.parent())
    , showTabs(settings.showTabs)
    , showHL(settings.showHL)
    , splitByFiles(settings.splitByFiles)
{
}

CRSettings::~CRSettings()
{
}

void CRSettings::saveToSettings(QSettings *settings) const
{
    settings->beginGroup(QLatin1String(Constants::CORE_SETTINGS_GROUP));
    settings->setValue(QLatin1String(Constants::SETTING_SHOW_TABS), showTabs);
    settings->setValue(QLatin1String(Constants::SETTING_SHOW_HL), showHL);
    settings->setValue(QLatin1String(Constants::SETTING_SPLIT_FILES), splitByFiles);
    settings->endGroup();
    settings->sync();
}

void CRSettings::loadFromSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String(Constants::CORE_SETTINGS_GROUP));
    showTabs = settings->value(QLatin1String(Constants::SETTING_SHOW_TABS), showTabs).toBool();
    showTabs = settings->value(QLatin1String(Constants::SETTING_SHOW_HL), showHL).toBool();
    showTabs = settings->value(QLatin1String(Constants::SETTING_SPLIT_FILES), splitByFiles).toBool();
    settings->endGroup(); // CORE_SETTINGS_GROUP
}

CRSettings &CRSettings::operator =(const CRSettings &other)
{
    bool settingsSame = (operator==(other));
    if(settingsSame == false) {
        this->showTabs = other.showTabs;
        this->showHL = other.showHL;
        this->splitByFiles = other.splitByFiles;
        emit settingsChanged();
    }
    return *this;
}

bool CRSettings::operator ==(const CRSettings &other) const
{
    bool different = false;
    different = different | (showTabs       != other.showTabs);
    different = different | (showHL         != other.showHL);
    different = different | (splitByFiles   != other.splitByFiles);
    return (different == false);
}


