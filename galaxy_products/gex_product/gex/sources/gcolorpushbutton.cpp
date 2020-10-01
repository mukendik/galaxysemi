#include "gcolorpushbutton.h"

#include <QColorDialog>

///////////////////////////////////////////////////////////////////////////////
// Class GColorPushButton - Class which represents a colored push button
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////
GColorPushButton::GColorPushButton(QWidget * pParent) : QPushButton(pParent)
{
	QObject::connect(this,	SIGNAL(clicked()),	this, SLOT(onEditColor()));

	setText("");
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////	
GColorPushButton::~GColorPushButton()
{
}

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void setActiveColor(const QColor& clrActive)
//
// Description	:	Sets the active color and fill up the pixmap
//
// Param		:	const QColor&	- 	the new active color
//
///////////////////////////////////////////////////////////////////////////////////
void GColorPushButton::setActiveColor(const QColor& clrActive)
{
	m_clrActive = clrActive;

	if (m_clrActive.isValid())
	{
		QPixmap buttonPixmap(width()/2, height()/2);
		buttonPixmap.fill(m_clrActive);
	
        setIcon(buttonPixmap);
        setIconSize(buttonPixmap.size());
	}
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void onEditColor()
//
// Description	:	Open QColorDialog to select a new active color
//
///////////////////////////////////////////////////////////////////////////////////
void GColorPushButton::onEditColor()
{
	QColor c = QColorDialog::getColor(activeColor(), this);
    
	if(c.isValid() == false)
		return;

    // Valid color selected, read it.
	setActiveColor(c);

	emit colorChanged(c);
}
