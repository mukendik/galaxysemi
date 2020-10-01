/****************************************************************************
** Deriven from gts_station_newlotdialog_base.h
****************************************************************************/

#ifndef gts_station_newlot_H
#define gts_station_newlot_H

#include <stdfparse.h>
#include <gqtl_datakeys.h>

#include "ui_gts_station_newlotdialog_base.h"

class GtsStation_Newlotdialog : public QDialog, public Ui::GtsStation_Newlotdialog_base
{
    Q_OBJECT

public:
    GtsStation_Newlotdialog(const GS::QtLib::DatakeysContent & KeysContent, QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~GtsStation_Newlotdialog();

	QString GetLotID() { return editLotID->text(); }
	QString GetSublotID() { return editSublotID->text(); }

private:

protected slots:
};

#endif // gts_station_newlot_H
