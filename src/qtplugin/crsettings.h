#pragma once

#include <QObject>
#include <QSettings>

namespace CR {
namespace Internal {

class CRSettings : public QObject
{
    Q_OBJECT
public:
    CRSettings();
    CRSettings(const CRSettings& settings);
    ~CRSettings();

    void saveToSettings(QSettings* settings) const;
    void loadFromSettings(QSettings* settings);

    CRSettings& operator=(const CRSettings& other);
    bool operator==(const CRSettings& other) const;

    bool showTabs;
    bool showHL;
    bool splitByFiles;
signals:
    void settingsChanged();

};

} // namespace Internal
} // namespace SpellChecker
