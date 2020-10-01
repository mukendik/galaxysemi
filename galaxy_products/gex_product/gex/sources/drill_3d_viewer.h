#ifndef _DRILL_3D_VIEWER_H_
#define _DRILL_3D_VIEWER_H_

///////////////////////////////////////////////////////////
// Libraries Includes
///////////////////////////////////////////////////////////
#ifdef QT3_SUPPORT
#undef QT3_SUPPORT
#include <qglviewer.h>
#include <QMouseEvent>
#define QT3_SUPPORT
#else
#include <qglviewer.h>
#include <QMouseEvent>
#endif

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GexGLViewer
//
// Description	:	Allows to display graphics with OpenGL.
//					Derived from QGLViewer object implemented in the LibQGLViewer
//					library
//
///////////////////////////////////////////////////////////////////////////////////
class GexGLViewer : public QGLViewer
{
	Q_OBJECT

public:

	GexGLViewer(QWidget * pParent = NULL);
	virtual ~GexGLViewer();

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////
private:

	enum selectType
	{
		LeftClickSelectObject,
		RightClickSelectObject
	};
	
	qglviewer::Vec			m_vecPosition;
	qglviewer::Vec			m_vecSceneCenter;
	selectType				m_eSelectionMode;
	
	QPoint					m_ptClickEvent;							// Keeps the position when mouse was pressed
	
///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
public:

    //! \brief Called each time we have to redraw the scene
    virtual void			draw();
    //! \brief Called when selecting an object
    virtual void			drawWithNames();
    //! \brief Called after selecting an object
    virtual void			postSelection(const QPoint& point);
    //! \brief Define Home position and move to.
    void					setHome(const qglviewer::Vec& position, const qglviewer::Vec& sceneCenter);
    //! \brief Test to overload the QWidget::grab() function
    //QPixmap                 grab(const QRect &rectangle = QRect(QPoint(0, 0), QSize(-1, -1)));

protected:

	virtual void			mousePressEvent(QMouseEvent *);			// Called when mouse button is pressed
	virtual void			mouseReleaseEvent(QMouseEvent *);		// Called when mouse button is released
	virtual void			keyPressEvent(QKeyEvent *);				// Called when key is pressed
	
public slots:
	
	void					goHome();								// Move to the Home location

private slots:
	
    //! \brief Initialize openGL control
    void					init();
	
signals:

	void					viewerDrawing();							// signal emitted when drawing
	void					viewerLeftClickSelection(const QPoint&);	// signal emitted when selection is done by left click
	void					viewerRightClickSelection(const QPoint&);	// signal emitted when an object is done by right click
};

#endif // _DRILL_3D_VIEWER_H_
