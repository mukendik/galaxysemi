#ifndef QDETACHABLETABWIDGET_H
#define QDETACHABLETABWIDGET_H

#include <QTabWidget>

class QDetachableTabWindowContainer;
class QDetachableTabBar;
class QDetachableTabTextEdit;

/**
 * \brief extended tab widget that will contain an extended tab bar
 */
class QDetachableTabWidget : public QTabWidget
{
    // need some signal/slot handling in this class
    Q_OBJECT

    // declare its tab bar as friend. Needed to access private feature. This is
    // not a flaw in the design
    friend class QDetachableTabBar;
    friend class QDetachableTabWindow;
    friend class QDetachableTabTextEdit;

    /**
     * \brief the container that contains all detachable tab window that are
     * created and currently active
     */
    QDetachableTabWindowContainer *mWindowContainer;

    /**
     * \brief the custom tab bar of this widget
     */
    QDetachableTabBar *mTabBar;

    /**
     * \brief try to move a tab from a window to another one
     *
     * \param index index of the tab in the tab bar.
     * \param position position of the mouse's pointer
     *
     * \return true if tab has moved, false otherwise
     */
    bool tryMoveTab( int index, const QPoint &position );

    /**
     * \brief detach a tab located at specified index, and place the new window
     * at the specified position
     *
     * \param index index of the tab in the tab bar.
     * \param position position of the mouse's pointer
     */
    void detachTab( int index, const QPoint &position );

    /**
     * \brief create an external window with specified content
     *
     * \param tabTitle the text inside the new tab that will be created in the
     * new detached window
     * \param tabContent the widget inside the tab
     * \param position position of the mouse's pointer
     */
    void createExternalWindow
        (
            const QString &tabTitle,
            QWidget *tabContent,
            const QPoint &position
        );

    /**
     * \brief Change the title of the parent window according to a change in
     * the current active tab
     *
     * \param text The new title of the parent window
     */
    void changeParentWindowTitle( const QString &text );

    /**
     * \brief Indicates if the underlying tab bar is locked
     *
     * \return true if tab bar is locked false otherwise
     */
    bool isTabBarLocked() const;

    /**
     * \brief Give the tab specified by provided index the focus
     *
     * \param index index of the tab to give the focus on
     */
    void focusTab( int index );

    /**
     * \brief Interat over the tabs and call the close of the widget inside
     *
     */
    void closeWidgetTabs();

    /**
     * \brief Constructor of this widget
     *
     * \param windowContainer pointer on the detachable tab window container
     * \param parent the parent of this widget
     */
    explicit  QDetachableTabWidget
        (
            QDetachableTabWindowContainer *windowContainer,
            QWidget *parent = NULL
        );

    ~QDetachableTabWidget();

signals :
    /**
     * @brief forward a signal after a tab rename
     * @param widget the widget inside the renamed tab
     * @param text the text used for the rename
     */
    void tabRenamed( QWidget *widget, const QString &text );

public slots :
    /**
     * \brief response to a close tab request signal
     *
     * \param index index of the tab to be closed in the tab bar.
     */
    void closeTab( int index );

    /**
     * \brief slot acting on a currentChanged signal
     * \param index the new index of the current tab
     */
    void currentTabChanged( int index );
};

#endif // QDETACHABLETABWIDGET_H
