#include "drill_3d_viewer.h"
#include "vec.h"
#include <gqtl_log.h>

using namespace qglviewer;

///////////////////////////////////////////////////////////////////////////////
// Class GexGLViewer - Class which allows to display openGL graphics
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////
GexGLViewer::GexGLViewer(QWidget * pParent) : QGLViewer(pParent),
    m_vecPosition(50.0, 50.0, 60.0), m_vecSceneCenter(50.0, 50.0, 0.0)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Constructor of Gex GL Viewer...");
    //connect(this, SIGNAL(viewerInitialized()), this, SLOT(onInit()));

    m_eSelectionMode = GexGLViewer::LeftClickSelectObject;

    // Disable PREVIOUS mouse binding (and remove it from help mouse tab).
    setMouseBinding((int)Qt::ControlModifier |(int) Qt::LeftButton,					NO_CLICK_ACTION);
    setMouseBinding((int)Qt::ControlModifier | (int)Qt::MidButton,					NO_CLICK_ACTION);
    setMouseBinding((int)Qt::ControlModifier | (int)Qt::RightButton,					NO_CLICK_ACTION);
    setMouseBinding((int)Qt::ControlModifier | (int)Qt::LeftButton  | (int)Qt::MidButton,  NO_CLICK_ACTION);
    setMouseBinding((int)Qt::LeftButton | (int)Qt::MidButton,							NO_CLICK_ACTION);

    setMouseBinding(Qt::LeftButton,  NO_CLICK_ACTION, true);
    setMouseBinding(Qt::MidButton,   NO_CLICK_ACTION, true);
    setMouseBinding(Qt::RightButton, NO_CLICK_ACTION, true);

    // Z o o m   o n   r e g i o n
    setMouseBinding((int)Qt::ShiftModifier | (int)Qt::MidButton, CAMERA, NO_MOUSE_ACTION);
    // S e l e c t
    setMouseBinding((int)Qt::ShiftModifier | (int)Qt::LeftButton, NO_CLICK_ACTION);

    setMouseBinding((int)Qt::ControlModifier | (int)Qt::LeftButton,  NO_CLICK_ACTION, true);
    setMouseBinding((int)Qt::ControlModifier | (int)Qt::RightButton, NO_CLICK_ACTION, true);

    // S p e c i f i c   d o u b l e   c l i c k s
    setMouseBinding(Qt::LeftButton,  NO_CLICK_ACTION, true, Qt::RightButton);
    setMouseBinding(Qt::RightButton, NO_CLICK_ACTION, true, Qt::LeftButton);
    setMouseBinding(Qt::LeftButton,  NO_CLICK_ACTION, true, Qt::MidButton);
    setMouseBinding(Qt::RightButton, NO_CLICK_ACTION, true, Qt::MidButton);

    // Disable default accelerators (and remove it from help mouse tab).
    setShortcut(DRAW_AXIS,			0);
    setShortcut(DRAW_GRID,			0);
    setShortcut(DISPLAY_FPS,		0);
    setShortcut(ENABLE_TEXT,		0);
    setShortcut(EXIT_VIEWER,		0);
    setShortcut(SAVE_SCREENSHOT,	0);
    setShortcut(CAMERA_MODE,		0);
    setShortcut(FULL_SCREEN,		0);
    setShortcut(STEREO,				0);
    setShortcut(ANIMATION,			0);
    setShortcut(HELP,				0);
    setShortcut(EDIT_CAMERA,		0);
    setShortcut(MOVE_CAMERA_LEFT,	0);
    setShortcut(MOVE_CAMERA_RIGHT,	0);
    setShortcut(MOVE_CAMERA_UP,		0);
    setShortcut(MOVE_CAMERA_DOWN,	0);
    setShortcut(INCREASE_FLYSPEED,	0);
    setShortcut(DECREASE_FLYSPEED,	0);

    // K e y f r a m e s   s h o r t c u t   k e y s
    setPathKey(-1,   1);
    setPathKey(-1,   2);
    setPathKey(-1,   3);
    setPathKey(-1,   4);
    setPathKey(-1,   5);
    setPathKey(-1,   6);
    setPathKey(-1,   7);
    setPathKey(-1,   8);
    setPathKey(-1,   9);
    setPathKey(-1,	10);
    setPathKey(-1,	11);
    setPathKey(-1,	12);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexGLViewer::~GexGLViewer()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onInit()
//
// Description	:	Initialize openGL control
//					Called when catch signal viewerInitialized()
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::init()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Init Gex GL viewer...");
    makeCurrent();
    glFlush();

    glClearColor(0.0f, 0.0588f, 0.2f, 0.0f);	// Let OpenGL clear background color to dark-blue.
    glShadeModel(GL_SMOOTH);					// enable flat shading

    glMatrixMode( GL_MODELVIEW );

    // Init some values for camera
    camera()->setSceneRadius(0.5f * Vec(100.0, 100.0, 0.0).norm());
    camera()->setUpVector(Vec(0.0, 0.0, 1.0));
    QGLViewer::init();
    doneCurrent();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void draw()
//
// Description	:	Override base function.
//					Call each time we have to redraw the scene
//					emit a signal viewerDrawing()
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::draw()
{
    emit viewerDrawing();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void goHome()
//
// Description	:	Move to the Home location
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::goHome()
{
    camera()->setPosition(m_vecPosition);
    camera()->setSceneCenter(m_vecSceneCenter);
    camera()->setOrientation(Quaternion());
    camera()->lookAt(camera()->sceneCenter());

    updateOverlayGL();
    updateGL();

    setFocus();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setHome(const Vec& position, const Vec& sceneCenter)()
//
// Description	:	Define the new Home location and move to.
//
// Param		:	position	[in]	Position of the camera
//					sceneCenter	[in]	Posistion of the scene center
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::setHome(const Vec& position, const Vec& sceneCenter)
{
    m_vecPosition		= position;
    m_vecSceneCenter	= sceneCenter;

    goHome();
}
/*
QPixmap GexGLViewer::grab(const QRect &rectangle)
{
    GSLOG(5, QString("GexGLViewer grab %1 per %2...").arg(this->width()).arg(this->height()).toLatin1().data() );
    //return renderPixmap(this->width(), this->height()); // returns a null pixmap...
    QImage lImage = this->grabFrameBuffer();
    return QPixmap::fromImage(lImage);
}
*/

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void drawWithNames()
//
// Description	:	Overloaded function
//					It draws each selectable object of the scene, enclosed by calls
//					to glPushName()/glPopName() to tag the object with an integer id.
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::drawWithNames()
{
    glPushMatrix();

    glPushName(0);
    draw();
    glPopName();

    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void postSelection(const QPoint& point)
//
// Description	:	called when user has made a selection
//
// Param		:	point [in]	Mouse position when selection has been done
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::postSelection(const QPoint& point)
{
    if (m_eSelectionMode == GexGLViewer::LeftClickSelectObject)
        emit viewerLeftClickSelection(point);
    else if (m_eSelectionMode == GexGLViewer::RightClickSelectObject)
        emit viewerRightClickSelection(point);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void mousePressEvent(QMouseEvent * pEvent)
//
// Description	:	Called when mouse button is pressed
//
// Param		:	pEvent [in]	Pointer on the mouse event
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::mousePressEvent(QMouseEvent * pEvent)
{
    m_ptClickEvent = pEvent->pos();

    QGLViewer::mousePressEvent(pEvent);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void mousePressEvent(QMouseEvent * pEvent)
//
// Description	:	Called when mouse button is released
//
// Param		:	pEvent [in]	Pointer on the mouse event
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::mouseReleaseEvent(QMouseEvent * pEvent)
{
    QGLViewer::mouseReleaseEvent(pEvent);

    // Left or right click, check if we have to manage selection
    if (pEvent->button() & Qt::LeftButton || pEvent->button() & Qt::RightButton)
    {
        QPoint ptPoint = pEvent->pos() - m_ptClickEvent;

        if (ptPoint.manhattanLength() < 10)
        {
            if (pEvent->button() & Qt::LeftButton)
                m_eSelectionMode = GexGLViewer::LeftClickSelectObject;
            else if (pEvent->button() & Qt::RightButton)
                m_eSelectionMode = GexGLViewer::RightClickSelectObject;

            select(pEvent);
            updateGL();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void keyPressEvent(QKeyEvent * pEvent)
//
// Description	:	Called when key is pressed
//
// Param		:	pEvent [in]	Pointer on the key event
//
///////////////////////////////////////////////////////////////////////////////////
void GexGLViewer::keyPressEvent(QKeyEvent * pEvent)
{
    if (pEvent->key() == Qt::Key_Plus)
    {
        QWheelEvent wheelEvent(QPoint(0,0), -120, Qt::NoButton, Qt::NoModifier);

        QGLViewer::wheelEvent(&wheelEvent);
    }
    else if (pEvent->key() == Qt::Key_Minus)
    {
        QWheelEvent wheelEvent(QPoint(0,0), 120, Qt::NoButton, Qt::NoModifier);

        QGLViewer::wheelEvent(&wheelEvent);
    }
    QGLViewer::keyPressEvent(pEvent);
}
