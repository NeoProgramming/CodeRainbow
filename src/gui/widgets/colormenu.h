#ifndef NEOCOLORMENU_H
#define NEOCOLORMENU_H

#include <QMenu>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QColor>
#include <QRectF>
#include <QColorDialog>
#include <QHash>
#include <QApplication>

class ColorKeyInfo
{
public:

   QColor color;
   QRectF rect;

   ColorKeyInfo( QColor color, QRectF rect )
   {
      this->color = color;
      this->rect = rect;
   }
};

class ColorMenu : public QMenu
{
   Q_OBJECT

   QColor hoverColor;
   QStringList colors;
   QList<ColorKeyInfo> colorInfos;
   QRectF rectMoreColors;
   QRectF rectAutomatic;
   QColor editedColor;
   int m_width, m_height;
signals:
   void selected( QColor color );

public:
   ColorMenu( QWidget *parent );
   void setColor(QColor clr) 
   {
	   editedColor = clr;
   }
   
   void paintButton( QPainter *painter, QRectF rc, QString text, bool hover);
   void showEvent ( QShowEvent * );
   void paintEvent ( QPaintEvent * );
   void mouseMoveEvent ( QMouseEvent * );
   void mousePressEvent ( QMouseEvent *event );
};


#endif // NEOCOLORMENU_H
