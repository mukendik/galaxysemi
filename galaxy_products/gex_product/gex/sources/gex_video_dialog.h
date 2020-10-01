///////////////////////////////////////////////////////////
// GEX video dialog - displays message on where to find
// videos on new GEX version
///////////////////////////////////////////////////////////

#ifndef GEX_VIDEO_DIALOG_H
#define GEX_VIDEO_DIALOG_H

#include "ui_gex_video_dialog.h"

class GexVideoDialog : public QDialog, public Ui::GexVideoDialogBase
{
	Q_OBJECT

public:

    GexVideoDialog( QWidget* parent = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~GexVideoDialog();

protected:

protected slots:

};

#endif // GEX_VIDEO_DIALOG_H
