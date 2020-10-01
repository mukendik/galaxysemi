#ifndef _gcolorpushbutton_h_
#define _gcolorpushbutton_h_

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QPushButton>

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GColorPushButton
//
// Description	:	ColorPushButton is a colored push button.
//					It allows to open a color dialog when user press it 
//
///////////////////////////////////////////////////////////////////////////////////
class GColorPushButton : public QPushButton
{
	Q_OBJECT

public:

	GColorPushButton(QWidget * pParent = NULL);
	virtual ~GColorPushButton();

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////
	void			setActiveColor(const QColor& clrActive);
	const QColor&	activeColor() const							{ return m_clrActive; }
	
private:

	QColor			m_clrActive;							// Active color used

private slots:

	void			onEditColor();							// Allows to change the active color

signals:

	void			colorChanged(const QColor&);			// signal emitted when active color is changed
};

#endif // _gcolorpushbutton_h_
