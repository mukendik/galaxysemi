///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "pickcoord_dialog.h"
#include "gqtl_skin.h"
#include "stdf.h"
#include "gex_constants.h"
#include "engine.h"
#include <gqtl_log.h>
#include "message.h"

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexSkin*	pGexSkin;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PickCoordDialog::PickCoordDialog(QWidget * pParent, Qt::WindowFlags fl) : QDialog(pParent, fl)
{
	GSLOG(SYSLOG_SEV_DEBUG, " new PickCoordDialog");
	setupUi(this);

	// Apply Examinator palette
	if (pGexSkin)
		pGexSkin->applyPalette(this);

	QObject::connect(PushButtonOk,		SIGNAL(clicked()),			this, SLOT(onOk()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()),			this, SLOT(reject()));
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PickCoordDialog::~PickCoordDialog()
{

}

///////////////////////////////////////////////////////////
// METHODS
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onOk()
//
// Description	:	Check the data validity.
//
///////////////////////////////////////////////////////////////////////////////////
void PickCoordDialog::onOk()
{
	m_rangeCoordinates.fromCoordinates(spinBoxXMin->value(), spinBoxXMax->value(), spinBoxYMin->value(), spinBoxYMax->value());

	switch (m_rangeCoordinates.validity())
	{
        case CGexRangeCoord::InvalidXCoordinate	:
            GS::Gex::Message::warning("", "X coordinates are not valid.");
            break;
        case CGexRangeCoord::InvalidYCoordinate	:
            GS::Gex::Message::warning("", "Y coordinates are not valid.");
            break;
    case CGexRangeCoord::ValidCoordinates :
            accept();
            break;
        default :
            GS::Gex::Message::warning("", "Coordinates are not valid.");
            break;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool setFile(const QString& strFile)
//
// Description	:	Read data file and find out min and max die coordinates
//
///////////////////////////////////////////////////////////////////////////////////
bool PickCoordDialog::setFile(const QString& strFile)
{
	int					iStatus;
    GS::StdLib::Stdf				StdfFile;
    GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
	QString				strDataFile = strFile;

	int					nDieXMin = 32768;
	int					nDieXMax = -32768;
	int					nDieYMin = 32768;
	int					nDieYMax = -32768;

	// If selected file is a CSV temporary edited file, then read the associated STDF file!
	if(strDataFile.endsWith(GEX_TEMPORARY_CSV) == true)
		// remove the custom extension from the temporary csv file...to rebuild the STDF original name!
		strDataFile = strDataFile.remove(GEX_TEMPORARY_CSV);

	// Open STDF file to read until MIR record found...have a 50K cache buffer used (instead of 2M !)
	iStatus = StdfFile.Open(strDataFile.toLatin1().constData(), STDF_READ, 1000000L);

    if(iStatus == GS::StdLib::Stdf::NoError)
	{
		// Read all file...until MRR is found.
		BYTE							bData;
		int								wData;
		int								nDieX;
		int								nDieY;

		// Read one record from STDF file.
		iStatus = StdfFile.LoadRecord(&StdfRecordHeader);

        while(iStatus == GS::StdLib::Stdf::NoError)
		{
			// Process STDF record read PRR records
			if((StdfRecordHeader.iRecordType == 5) && (StdfRecordHeader.iRecordSubType == 20))
			{
				switch(StdfRecordHeader.iStdfVersion)
				{
					case GEX_STDFV4:
						StdfFile.ReadByte(&bData);		// head number
						StdfFile.ReadByte(&bData);		// site number
						StdfFile.ReadByte(&bData);		// part flag
						StdfFile.ReadWord(&wData);		// number of tests
						StdfFile.ReadWord(&wData);		// HBIN
						StdfFile.ReadWord(&wData); 		// SBIN

                        if(StdfFile.ReadWord(&nDieX) != GS::StdLib::Stdf::NoError)	// Die X coord
							nDieX = -32768;	// no DIE X

                        if(StdfFile.ReadWord(&nDieY) != GS::StdLib::Stdf::NoError) // Die Y coord
							nDieY = -32768;	// no DIE Y

						break;

                default:
						nDieX = -32768;	// no DIE X
						nDieY = -32768;	// no DIE Y
						break;
				}

				// Valid X die coord, compare with min/max
				if (nDieX != -32768)
				{
					if (nDieX >= 32768)
						nDieX -= 65536;

					nDieXMin = gex_min(nDieXMin, nDieX);
					nDieXMax = gex_max(nDieXMax, nDieX);
				}

				// Valid Y die coord, compare with min/max
				if (nDieY != -32768)
				{
					if (nDieY >= 32768)
						nDieY -= 65536;

					nDieYMin = gex_min(nDieYMin, nDieY);
					nDieYMax = gex_max(nDieYMax, nDieY);
				}
			}

			// Read one record from STDF file.
			iStatus = StdfFile.LoadRecord(&StdfRecordHeader);
        }

		// End of file or error while reading...
        StdfFile.Close();
	}

	if (nDieXMin == 32768 || nDieXMax == -32768 || nDieYMin == 32768 || nDieYMax == -32768)
	{
		// Error message
        GS::Gex::Message::information("", "No valid X,Y coordinates found");
		return false;
	}
	else
	{
		m_rangeCoordinates.fromCoordinates(nDieXMin, nDieXMax, nDieYMin, nDieYMax);
		// initialize interface
		initUI();
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setCoordinates(const QString& strCoordinates)
//
// Description	:	Set default coordinates
//
///////////////////////////////////////////////////////////////////////////////////
void PickCoordDialog::setCoordinates(const QString& strCoordinates)
{
	m_rangeCoordinates.fromString(strCoordinates);

	// Valid coordinates, so update the ui
	if (m_rangeCoordinates.isValid())
		fillUI();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void initUI()
//
// Description	:	initialize the UI control
//
///////////////////////////////////////////////////////////////////////////////////
void PickCoordDialog::initUI()
{
	// Set the minimum and maximum coordinates
	spinBoxXMin->setMinimum(m_rangeCoordinates.coordXMin());
	spinBoxXMin->setMaximum(m_rangeCoordinates.coordXMax()-1);

	spinBoxXMax->setMinimum(m_rangeCoordinates.coordXMin()+1);
	spinBoxXMax->setMaximum(m_rangeCoordinates.coordXMax());

	spinBoxYMin->setMinimum(m_rangeCoordinates.coordYMin());
	spinBoxYMin->setMaximum(m_rangeCoordinates.coordYMax()-1);

	spinBoxYMax->setMinimum(m_rangeCoordinates.coordYMin()+1);
	spinBoxYMax->setMaximum(m_rangeCoordinates.coordYMax());

	// fill the UI with value
	fillUI();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fillUI()
//
// Description	:	fill the UI control
//
///////////////////////////////////////////////////////////////////////////////////
void PickCoordDialog::fillUI()
{
	spinBoxXMin->setValue(m_rangeCoordinates.coordXMin());
	spinBoxXMax->setValue(m_rangeCoordinates.coordXMax());
	spinBoxYMin->setValue(m_rangeCoordinates.coordYMin());
	spinBoxYMax->setValue(m_rangeCoordinates.coordYMax());
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString coordinates()
//
// Description	:	Returns a string containing coordinates selected
//
///////////////////////////////////////////////////////////////////////////////////
QString PickCoordDialog::coordinates()
{
	return m_rangeCoordinates.toString();
}
