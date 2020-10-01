#ifndef QDETACHABLETABWINDOWCONTAINER_H
#define QDETACHABLETABWINDOWCONTAINER_H

#include <QWidget>

namespace Ui
{
class QDetachableTabWindowContainer;
}

class QDetachableTabWindow;

/**
 * \brief The widget that has to be used from the user side. Gives the
 * opportunity to create tab windows that are detachable
 */
class QDetachableTabWindowContainer : public QWidget
{
    Q_OBJECT

    // friendship relation with widgets that are making part of this system
    // not a design's flaw
    friend class QDetachableTabWidget;
    friend class QDetachableTabWindow;
    friend class IsActiveWindowInContainerPredicate;

public:
    /**
     * \brief Constructor, widget's compliant
     *
     * \param parent parent of this widget. Ensure ownership and resource
     * management by the Qt's engine
     */
    explicit QDetachableTabWindowContainer( QWidget *parent = 0 );

    /**
     * \brief Destructor used to free owned resources. Make this class derivable
     */
    virtual ~QDetachableTabWindowContainer();


    /**
     * @brief empty indicates if all the TabWindow have been closed
     */
    bool empty();

    /**
     * @brief focus on the tab of the active window containing the QWidget
     * widget
     * @param widget
     */
    void focusOnTab( QWidget *widget);

    /**
     * \brief add a tab in the active detachable tab window in the container.
     * The active window is the one which has the focus on
     *
     * \param widget content of the newly created tab
     * \param title Text inside the new ly create tab's title
     */
    void addDetachableTabInActiveWindow
        ( QWidget *widget, const QString &title );

    /**
     * \brief add an external detached window in the container at specified
     * position  if any
     *
     * \param widget content of the newly created tab
     * \param title Text inside the newly create tab's and windows's title
     * \param position the position of the new window. If not any position is
     * specified, the window will be created at the position of the container
     */
    void addDetachedWindowAt
        (
            QWidget *widget,
            const QString &title,
            const QPoint *position = NULL
        );

    /**
     * \brief Change the title of the window containing the specified widget.
     *
     * \param containedWidget the contained widget within the window whose the
     * title has to be changed.
     *
     * \param title the new title of the window
     */
    void changeWindowTitleByContent
        ( QWidget *containedWidget, const QString &title );

    /**
     * \brief Change the title of the tab containing the specified widget.
     *
     * \param containedWidget the contained widget within the window whose the
     * title has to be changed.
     *
     * \param title the new title of the window
     */
    void changeTabTitleByContent
        ( QWidget *containedWidget, const QString &title );

    /**
     * \brief Indicates if the specified window is the active one in the
     * container
     *
     * \param window window to check the activity in the container
     *
     * \return true if the specified window is the currently active one in the
     * container, false otherwise
     */
    bool isActiveWindow( const QDetachableTabWindow *window ) const;

    /**
     * \brief remove the specified external widget from the collection
     *
     * \param widget the widget inside the tab to remove
     */
    void removeTabWindowContaining(QWidget* widget);

    /**
     * @brief IsFirstWindowEmpty indicate if the first TabWindow still contains
     * tabWidget. The first window is in the main window of GEX, that is why
     * this information is required
     */
    bool IsFirstWindowEmpty() const { return firstWindowIsEmpty_; }

    /**
     * \brief Get the currently active windows. This is the one which has the
     * highest z-level
     *
     * \return the window having the highest z-level
     */
    QDetachableTabWindow * activeWindow();

    void replaceWidget
      ( QWidget *previousWidget, QWidget *newWidget, const QString &label );

private:

    bool                    firstWindowIsEmpty_;
    QDetachableTabWindow*   mainWindow_;


    /**
     * \brief this is specific to the designer of Qt
     */
    Ui::QDetachableTabWindowContainer *ui;

    /**
     * \brief container of detachable tab windows. A list looks good as there is
     * not much chance that user creates more than 100 windows.
     */
    QList< QDetachableTabWindow * > windows_;

    /**
     * \brief create the first window of this container
     */
    void createInitialWindow();

    /**
     * \brief give an external tabbed window at the specified position if any.
     * This window is picked from the windows_ container.
     *
     * \param position position of the mouse's cursor.
     *
     * \return A pointer on an instance of QDetachableTabWindow if any in the
     * collection at specified point, otherwise NULL
     */
    QDetachableTabWindow * getDetachableTabWindowAt( const QPoint &position );

    /**
     * \brief remove the specified external window from the collection
     *
     * \param widget the window to add to the container
     */
    void removeDetachableTabWindow( QDetachableTabWindow *widget );

    /**
     * \brief add the specified external window in the collection
     *
     * \param widget the window to add to the container
     */
    void addDetachableTabWindow( QDetachableTabWindow *widget );

    /**
     * \brief Fix provided pointer on a QPoint instance. If null, return a
     * QPoint containing this instance of QDetachableTabWindowContainer's
     * position, otherwise, return itself
     *
     * \param position position to fix; maybe null
     *
     * \return Position processed. Copy-elision here
     */
    QPoint fixPosition( const QPoint *position ) const;

    /**
     * \brief Refresh values of z-level for all windows in the container
     */
    void refreshZLevel();

    /**
     * \brief Give the specified window the highest z-level as well as the focus
     *
     * \param window the window to elevate
     */
    void setMaxZLevelFor( QDetachableTabWindow *window );

signals :
    /**
     * @brief forward a signal after a tab rename
     * @param widget the widget inside the renamed tab
     * @param text the text used for the rename
     */
    void tabRenamed( QWidget *widget, const QString &text );

public slots :
    /**
     * @brief forward a signal after a tab rename
     * @param widget the widget inside the renamed tab
     * @param text the text used for the rename
     */
    void slotTabRenamed( QWidget *widget, const QString &text )
    { emit tabRenamed( widget, text ); }
};

#endif // QDETACHABLETABWINDOWCONTAINER_H
