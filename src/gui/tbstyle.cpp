#include "TBStyle.h"
#include <QLayout>
#include <QString>
#include <QStyleOption>
#include <QtDebug>

TBStyle::TBStyle()
    : QCommonStyle()
{
//    Q_INIT_RESOURCE(shared);
}

QSize TBStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                    const QSize &size, const QWidget *widget) const
{
    QSize newSize = QCommonStyle::sizeFromContents(type, option, size, widget);


    switch (type) {
    case CT_RadioButton:
        newSize += QSize(20, 0);
        break;

    case CT_PushButton:
        newSize.setHeight(26);
        break;

    case CT_Slider:
        newSize.setHeight(27);
        break;

    default:
        break;
    }

    return newSize;
}

int TBStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    if (pm == PM_ToolBarIconSize)
        return 64;
    return QCommonStyle::pixelMetric(pm, opt, widget);
}
