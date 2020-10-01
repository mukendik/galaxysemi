#include "QSnappedVBoxLayout.h"

QSnappedVBoxLayout::QSnappedVBoxLayout( QWidget *parent ) :
    QVBoxLayout( parent )
{
    // remove margins around this layout
    setContentsMargins( 0, 0, 0, 0 );

    // remove space between widgets contained in this layout
    setSpacing( 0 );
}
