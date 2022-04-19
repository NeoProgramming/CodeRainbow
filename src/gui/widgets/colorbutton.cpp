#include "colorbutton.h"
#include "colormenu.h"
#include <QPainter>

ColorButton::ColorButton( QWidget *parent ) : QPushButton(parent)
{
    // create a color menu (a simplified analogue of a color picker)
    m_colorMenu = new ColorMenu(this);
    connect(m_colorMenu, &ColorMenu::selected, this, &ColorButton::onColorChanged);
    setMenu(m_colorMenu);
}

void ColorButton::setColor(QColor clr)
{
    m_clr = clr;
	m_colorMenu->setColor(clr);
    QPixmap pix(16, 16);

    if(clr.isValid()) {
//        qDebug() << "setColor: " << clr << " is valid";
        pix.fill(clr);
        QIcon icon(pix);
        setIcon(icon);
    }
    else {
//        qDebug() << "setColor: " << clr << " is invalid";
        QPainter paint(&pix);
        paint.fillRect(0,8,8,8,Qt::gray);
        paint.fillRect(8,0,8,8,Qt::gray);
        paint.fillRect(0,0,8,8,Qt::white);
        paint.fillRect(8,8,8,8,Qt::white);
        QIcon icon(pix);
        setIcon(icon);
    }
}

QColor ColorButton::getColor()
{
    return m_clr;
}

void ColorButton::onColorChanged(QColor clr)
{
    setColor(clr);
    emit colorChanged(clr);
}
