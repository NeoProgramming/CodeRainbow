#pragma once

#include "crsettings.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QListWidgetItem;
QT_END_NAMESPACE

namespace CR {
namespace Internal {

namespace Ui {
class CROptionsWidget;
}

class CROptionsWidget : public QWidget
{
    Q_OBJECT

public:
    CROptionsWidget(const CRSettings* const settings, QWidget *parent = 0);
    ~CROptionsWidget();

    const CRSettings& settings();
    /*! Function to apply the settings in widgets added to the Core Options Widget.
     * This function will in turn cause the applyCurrentSetSettings() signal to be
     * emitted. */
    void applySettings();
signals:
    /*! Signal to notify other added options widgets that the specified settings can be applied. */
    void applyCurrentSetSettings();

private:
    void updateWithSettings(const CRSettings* const settings);
    Ui::CROptionsWidget *ui;
    CRSettings m_settings;
};

} // Internal
} // CR
