#ifndef QSNAPPEDVBOXLAYOUT_H
#define QSNAPPEDVBOXLAYOUT_H

#include <QBoxLayout>

/**
 * \brief This is a subclass of QVBoxLayout, aiming to create a vertical layout
 * that snaps to edges
 */
class QSnappedVBoxLayout : public QVBoxLayout
{
    // friendly access for these classes
    friend class QDetachableTabWindowContainer;
    friend class QDetachableTabWindow;
    friend class QDetachableTabBar;

    /**
     * \brief Constructor of this layout, taking a widget to initialize its
     * parent
     */
    explicit QSnappedVBoxLayout( QWidget *parent = NULL );
};

#endif // QSNAPPEDVBOXLAYOUT_H
