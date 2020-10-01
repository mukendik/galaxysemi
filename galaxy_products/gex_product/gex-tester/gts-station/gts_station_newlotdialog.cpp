/****************************************************************************
** Deriven from gts_station_newlotdialog_base.cpp
****************************************************************************/
#include <qlineedit.h>

#include "gts_station_newlotdialog.h"


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a GtsStation_Newlotdialog as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
/////////////////////////////////////////////////////////////////////////////////////
GtsStation_Newlotdialog::GtsStation_Newlotdialog(const GS::QtLib::DatakeysContent & KeysContent, QWidget* parent, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
    // Setup UI
    setupUi(this);
    editLotID->setText(KeysContent.Get("Lot").toString());
    editSublotID->setText(KeysContent.Get("SubLot").toString());
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GtsStation_Newlotdialog::~GtsStation_Newlotdialog()
{
}

