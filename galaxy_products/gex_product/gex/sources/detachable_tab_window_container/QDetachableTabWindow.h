#ifndef QDETACHABLETABWINDOW_H
#define QDETACHABLETABWINDOW_H

#include<QDialog>

class QDetachableTabWindowContainer;
class QDetachableTabWidget;

/**
 * \brief a predefined widget type with a vbox layout that accept extended tab
 * widget
 */
class QDetachableTabWindow : public QDialog
{
    // friendly access for these classes
    friend class QDetachableTabWidget;
    friend class QDetachableTabWindowContainer;
    friend struct WindowsZLevelCompare;

    /**
     * \brief the container that will contain all detached window currently
     * active
     */
    QDetachableTabWindowContainer *mWindowContainer;

    /**
     * \brief this is the custom tab widget inside this window
     */
    QDetachableTabWidget *mTabs;

    /**
     * \brief The depth level of the window. The higher this value is, the more
     * probable the window is visible to the user
     */
    std::size_t mZLevel;

    /**
     * \brief set up the content of the window, it's tab-based
     */
    void setupContent();

    /**
     * \brief private implementation of close event
     */
    void processCloseEvent( QCloseEvent * );

    /**
     * \brief private implementation of mouse press event
     */
    void processMousePressEvent( QMouseEvent * event );


    /**
     * @brief focus on the tab containing the QWidget widget
     * @param widget
     */
    void focusOnTab( QWidget *widget);


    /**
     * @brief indicate if this TabWIndow contains a seek widget
     * @param widget researched
     */
    bool containsWidget(QWidget *widget);


    /**
     * \brief insert a new tab in this window
     *
     * \param widget the content of the tab to add
     * \param title the title of the tab to add
     *
     * \return the new tab index
     */
    int addTab( QWidget *widget, const QString &title );

    /**
     * \brief Give the focus to the tab located at specified index
     *
     * \param index index of the tab to focus
     */
    void focusTab( int index );

    /**
     * \brief indicates count of tabs inside the window
     *
     * \return an integer representing the count of tab in the window
     */
    int count() const;

    /**
     * \brief accessor on tab widget
     *
     * \return the current tab widget within the layout of this window
     */
    QDetachableTabWidget * tabs() { return mTabs; }

    /**
     * \brief Constructor
     *
     * \param windowContainer thedetachable tab window container
     * \param parent the parent widget of this one
     */
    explicit QDetachableTabWindow
        (
            QDetachableTabWindowContainer *windowContainer,
            QWidget *parent = NULL
        );

    ~QDetachableTabWindow();
    void keyPressEvent(QKeyEvent *e);

    /**
     * \brief override close event to notify main window of destruction of this
     * window
     *
     * \param event event informations
     */
    void closeEvent( QCloseEvent *event );

    /**
     * \brief override mouse press event to give the active state of this window
     * inside the container window
     *
     * \param event event informations
     */
    void mousePressEvent( QMouseEvent *event );

    /**
     * \brief override focus on event to give the focused widget
     *
     * \param event event informations
     */
    void focusInEvent(QFocusEvent*);

    /**
     * Set to true by the event filter when esc keyboard is pressed
    */
    bool mDoNotClose;

public:
    void replaceWidget(QWidget *previousWidget, QWidget *newWidget, const QString &label);

    /**
     * @brief forceClose reset the variable mDoNotClose to false to force the close
     */
    void forceClose() {mDoNotClose = false; }
protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // QDETACHABLETABWINDOW_H
