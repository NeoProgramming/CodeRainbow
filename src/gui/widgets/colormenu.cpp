
#include <QPainter>
#include <QStyleOptionButton>

#include "colormenu.h"

#define COLORSIZE 20	// cell width and height
#define COLS 11			// number of columns
#define SPACE	4		// between cells
#define MARGINY 4		// indentation top and bottom
#define MARGINX 4		// indentation right and left
#define BUTTONW 32

ColorMenu::ColorMenu( QWidget *parent )
    : QMenu( parent )
{
//	this->setWindowFlags( Qt::Popup );
	colors = QColor::colorNames();
    int n = colors.count();

	m_width = COLS;
    m_height = (int)(n / COLS);

	m_height++;

	m_width  = m_width * COLORSIZE + SPACE + MARGINX  + 4;
	m_height = m_height * COLORSIZE + SPACE + MARGINY + BUTTONW + BUTTONW;

//@	setFrameShape(QFrame::StyledPanel);
//@	setFrameShadow(QFrame::Plain);
	resize( m_width, m_height  );

    setMouseTracking( true );   // tracking mouse movement while pressing keys
	this->setStyleSheet( "QFrame{background-color: rgb(255, 255, 255);border-color: rgb(0, 0, 0);}" );
	this->setObjectName( "popup" );
//    qDebug() << "N == " << n << " W= " << m_width << " H == " << m_height;
}

void ColorMenu::paintButton( QPainter *painter, QRectF rc, QString text, bool hover)
{
	if( !hover )
	{
		painter->setPen( Qt::black );
		painter->fillRect( rc, Qt::lightGray );
	}
	else
	{
		painter->setPen( Qt::blue );
		painter->fillRect( rc, Qt::yellow );
	}

	painter->drawRect( rc );
	painter->drawText( rc, text, Qt::AlignHCenter | Qt::AlignVCenter );
}

void ColorMenu::showEvent ( QShowEvent * )
{
	resize( m_width, m_height  );
}

void ColorMenu::paintEvent ( QPaintEvent * )
{
	int col, row;
	QPainter painter( this );

	painter.drawRect(0,0,m_width-1,m_height-1);

	// cells
	colorInfos.clear();
	for( int i = 0; i < colors.count(); i++ )
	{
		QRectF rc;
		col = i % COLS;
		row = (int)(i / COLS);

		rc.setLeft( col * COLORSIZE + SPACE + MARGINX );
		rc.setTop( row * COLORSIZE + SPACE + MARGINY + BUTTONW );

		rc.setWidth( COLORSIZE - SPACE );
		rc.setHeight( COLORSIZE - SPACE );

		QColor c( colors.at(i) );
    //    qDebug() << colors.at(i);

		painter.fillRect( rc, QBrush( c ) );
		//colorRects[c] = rc;
		colorInfos.append( ColorKeyInfo(c, rc) );
	}//for

	// mouse position
	QPointF mousePos = this->mapFromGlobal( QCursor::pos() );

	// default button
	rectAutomatic = QRect( SPACE, SPACE + MARGINY + 5, this->width() - 2 * SPACE, 20 );
    paintButton(&painter, rectAutomatic, tr( "No Color" ), rectAutomatic.contains( mousePos ) );

	// button "other colors"
	rectMoreColors = QRect( SPACE, (row + 1) * COLORSIZE + SPACE + MARGINY + 5 + BUTTONW, this->width() - 2 * SPACE, 20 );
	paintButton(&painter, rectMoreColors, tr( "More..." ), rectMoreColors.contains( mousePos ) );

	//hoverColor = 0;
	for( int i = 0; i < colorInfos.count(); i++)
	{
		if( colorInfos[i].rect.contains( mousePos ) )
		{
			QPainter painter( this );

			hoverColor = colorInfos[i].color;
			QRectF rc = colorInfos[i].rect;

			rc.setTop( rc.top() -2 );
			rc.setLeft( rc.left() -2 );
			rc.setBottom( rc.bottom() + 2 );
			rc.setRight( rc.right() + 2 );

			painter.setPen( QPen(Qt::blue, 2) );
			painter.drawRect( rc );
		}
	}
}

void ColorMenu::mouseMoveEvent ( QMouseEvent *e )
{
    this->repaint();
    QMenu::mouseMoveEvent(e);
}

void ColorMenu::mousePressEvent ( QMouseEvent *event )
{
	if( !rect().contains( event->pos() ) ) this->close();

    for( int i = 0; i < colorInfos.count(); i++) {
        if( colorInfos[i].rect.contains( event->pos() ) ) {
			hoverColor = colorInfos.at(i).color;
			emit selected( hoverColor );
            //emit triggered(menuAction());
            this->close();
		}
	}

    if( rectMoreColors.contains( event->localPos() ) ) {
		QColorDialog *dialog = new QColorDialog( editedColor, this );
        if( dialog->exec() ) {
			hoverColor = dialog->selectedColor();
			delete dialog;
			emit selected( hoverColor );
            //emit triggered(menuAction());
            this->close();
		}
	}

	else if( rectAutomatic.contains( event->localPos() ) )
	{
		hoverColor = QColor();
		emit selected( hoverColor );
        //emit triggered(menuAction());
        this->close();
	}
}

