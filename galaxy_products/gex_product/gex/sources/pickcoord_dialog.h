#ifndef PICK_COORD_DIALOG_H
#define PICK_COORD_DIALOG_H

///////////////////////////////////////////////////////////
// UI Includes
///////////////////////////////////////////////////////////
#include "ui_pickcoord_dialog.h"
#include "classes.h"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	PickCoordDialog
//
// Description	:	Class used to define the coordinates to filter on
//
///////////////////////////////////////////////////////////////////////////////////
class PickCoordDialog : public QDialog, public Ui::PickCoordDialogBase
{
	Q_OBJECT

public:
	
	PickCoordDialog(QWidget* parent = 0, Qt::WindowFlags f = 0 );
	~PickCoordDialog();

	bool			setFile(const QString& strFile);				// Read data file and find out min and max die coordinates
	void			setCoordinates(const QString& strCoordinates);	// Set default coordinates

	QString			coordinates();									// Returns a string containing coordinates selected

private slots:

	void			onOk();
	
private:

	void			initUI();										// initialize the UI control
	void			fillUI();										// fill the UI control
	
	CGexRangeCoord	m_rangeCoordinates;								// Holds the coordinates range
};

#endif // PICK_COORD_DIALOG_H
