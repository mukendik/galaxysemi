#ifndef QDETACHABLETABTEXTEDIT_H
#define QDETACHABLETABTEXTEDIT_H

#include <QLineEdit>

class QDetachableTabBar;
class QDetachableTabWidget;

/**
 * \brief Custopn edition control used to modify a tab title in a
 * QDetachableTabBar widget.
 */
class QDetachableTabTextEdit : public QLineEdit
{
    Q_OBJECT

    // friendly access for these classes
    friend class QDetachableTabBar;
    friend class QDetachableTabWidget;

    /**
     * \brief the initial text in the tab
     */
    QString mInitialText;

    /**
     * \brief The old button to restore after this widget close
     */
    QWidget *mOldRightButton;

    /**
     * \brief indicates that the next cancel by esc key press will end the edition
     */
    bool mLastCancelUntilClose;

    /**
     * \brief index of the tab to edit
     */
    int mTabIndex;

    /**
     * \brief private implementation for key press event
     *
     * \param event event informations
     */
    void processKeyPressEvent( QKeyEvent *event );

    /**
     * \brief specific implementation of the mouse press event
     */
    void processMousePressEvent( QMouseEvent * );

    /**
     * \brief cancel the input
     */
    void cancelInput();

    /**
     * \brief validate the input
     *
     * \param text the new tab title to apply after the validation
     */
    void validateInput( const QString & text);

    /**
     * \brief close the widget with specified text set in the parent
     *
     * \param text this is the text which will be used to rename the tab in the
     * parent tab bar
     */
    void closeWithTextInParent( const QString &text );

    /**
     * \brief Constructor
     *
     * \param parent parent widget
     * \param text initial text of the tab before any modification occur
     * \param oldRightButton an eventual button to restore after this widget has
     * filled its role
     */
    explicit QDetachableTabTextEdit
        (
            QDetachableTabBar *parent,
            const QString &text,
            QWidget *oldRightButton
        );

    /**
     * \brief override key press : validate or cancel changes
     *
     * \param event event informations
     */
    void keyPressEvent( QKeyEvent *event );

    /**
     * \brief process this event for focus tasks
     * \param event the mouse event
     */
    void mousePressEvent( QMouseEvent *event );

signals :
    /**
     * @brief signal emitted when edition is terminated
     * @param widget widget in the renamed tab
     * @param text text entered
     */
    void endEdit( QWidget *widget, const QString &text );
};

#endif // QDETACHABLETABTEXTEDIT_H
