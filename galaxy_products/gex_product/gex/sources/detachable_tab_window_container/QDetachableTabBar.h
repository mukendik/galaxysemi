#ifndef QDETACHABLETABBAR_H
#define QDETACHABLETABBAR_H

#include <QTabBar>
#include <utility>
#include <map>

/**
 * \brief This is a detachable tab bar used in a QDetachableTabWidget.
 */
class QDetachableTabBar : public QTabBar
{
    Q_OBJECT

    // this class use a private feature declared here
    friend class QDetachableTabTextEdit;
    friend class QDetachableTabWidget;

    /**
     * \brief classical constructor.
     *
     * \param parent the parent of this widget
     */
    explicit QDetachableTabBar( QWidget *parent );

    /**
     * \brief Last position known after a mouse press event
     */
    QPoint mClickPosition;

    /**
     * \brief tab that is targeted at the position of the last click. -1 if no
     * tab is targeted
     */
    int mTargetedTabIndexWhenClick;

    /**
     * \brief The minimal distance to travel while left mouse button is pressed
     * to initial a detach tab. This distance is not taken in account in case of
     * moving a tab from a window to another one
     */
    float mMinRipOffDistance;

    /**
     * \brief This is a widget representing a preview window while a tab
     * dragging is in progress
     */
    QWidget *mPreviewWindow;

    /**
     * \brief Indicates if tabs can be renamed, moved or detached. They're lock
     * if a rename is active
     */
    bool mIsLocked;

    /**
     * \brief Indicates if a tab can be detached from its initial window to
     * make part of a new one.
     *
     * \param position destination at which create a new window
     *
     * \return true if tab can be detached, false otherwise
     */
    bool tabCanBeDetachedTo( const QPoint &position ) const;

    /**
     * \brief Private implementation of mouse release button event. Can lead to
     * the creation of a new window or a move of a tab from one window to
     * another
     *
     * \param event event informations
     */
    void processMouseReleaseEvent( QMouseEvent *event );

    /**
     * \brief Private implementation of mouse press button event. initiate a
     * tab dragging
     *
     * \param event event informations
     */
    void processMousePressEvent( QMouseEvent *event );

    /**
     * \brief move or detach a tab from the bar and place the new window at
     * specified position
     *
     * \param position the destination point. Represents coordinates inside
     * an existing tab bra in case of move or a point on the screen in case of
     * detach tab in a new window
     */
    void detachOrMoveTab( const QPoint &position );

    /**
     * \brief private implementation of processing a mouse move event. Processes
     * the tab dragging.
     *
     * \param event event informations
     */
    void processMouseMoveEvent( QMouseEvent *event );

    /**
     * \brief show a preview window for detachment at specified location.
     * Called while mouse is moving with left button pressed
     *
     * \param position position of the mouse's cursor
     */
    void previewWindow( const QPoint &position );

    /**
     * \brief private implementation of mouse double click event for this widget
     * Enable a tab rename operation.
     *
     * \param event event informations
     */
    void processMouseDoubleClickEvent( QMouseEvent *event );

    /**
     * \brief Change the title of the parent window according to a change in
     * the current active tab
     *
     * \param text The new title of the parent window
     */
    void changeParentWindowTitle( const QString &text );

public:
    /**
     * \brief manage of preview window
     */
    virtual ~QDetachableTabBar();

private :
    /**
     * \brief override the event : end dragging
     *
     * \param event event informations
     */
    void mouseReleaseEvent( QMouseEvent *event );

    /**
     * \brief override the event : start dragging
     *
     * \param event event informations
     */
    void mousePressEvent( QMouseEvent *event );

    /**
     * \brief override the event : drag tab to detach or move it
     *
     * \param event event informations
     */
    void mouseMoveEvent( QMouseEvent *event );

    /**
     * \brief override double click : edit title of the targeted tab
     *
     * \param event event informations
     */
    void mouseDoubleClickEvent( QMouseEvent *event );

    /**
     * \brief This method is called when a tab is inserted
     */
    void tabInserted( int );

    /**
     * \brief This method is called when a tab is removed
     */
    void tabRemoved( int );

    /**
     * \brief If the current bar is in edition (rename a tab) stop it by saving
     * modifications on the tab name
     */
    void stopEditing();

signals :
    void tabRenamed( QWidget *widget, const QString &string );

private slots :
    /**
     * @brief slot reacting on an endEdit signal from tab edit control
     * @param widget widget inside the tab renamed
     * @param text the text used in rename
     */
    void slotEndEdit( QWidget *widget, const QString & text )
    { emit tabRenamed( widget, text ); }
};

#endif // QDETACHABLETABBAR_H
