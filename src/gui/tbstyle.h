#ifndef TBStyle_H
#define TBStyle_H

#include <QCommonStyle>

QT_USE_NAMESPACE

class TBStyle : public QCommonStyle
{
public:
    TBStyle();

    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const override;


    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;
};

#endif
