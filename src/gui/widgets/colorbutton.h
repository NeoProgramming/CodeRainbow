#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
class ColorMenu;
class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    ColorButton( QWidget *parent );
    void setColor(QColor clr);
    QColor getColor();

signals:
    void colorChanged(QColor clr);
private slots:
    void onColorChanged(QColor clr);
private:
    QColor m_clr;
	ColorMenu *m_colorMenu;
};

#endif // COLORBUTTON_H
