#pragma once

#include <coreplugin/dialogs/ioptionspage.h>
#include <QPointer>

namespace CR {
namespace Internal {

class CRSettings;
class CROptionsWidget;

class CROptionsPage : public Core::IOptionsPage
{
    Q_OBJECT
public:
    CROptionsPage(CRSettings* settings, QObject *parent = 0);
    virtual ~CROptionsPage();

    bool matches(const QString &searchKeyWord) const;
    QWidget *widget();
    void apply();
    void finish();
signals:

public slots:
private:
    CRSettings* const m_settings;
    QPointer<CROptionsWidget> m_widget;
};

} // namespace Internal
} // namespace SpellChecker
