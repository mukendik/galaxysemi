#include <QFileDialog>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qapplication.h>

#include "browser_dialog.h"
#include "timeperiod_dialog.h"
#include "calendar_dialog.h"
#include "report_build.h"

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
TimePeriodDialog::TimePeriodDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	:QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(FromDateCalendar,	SIGNAL(clicked()),				this, SLOT(OnFromDateCalendar()));
    QObject::connect(ToDateCalendar,	SIGNAL(clicked()),				this, SLOT(OnToDateCalendar()));
    QObject::connect(buttonBrowsePath,	SIGNAL(clicked()),				this, SLOT(OnBrowseFolder()));
    QObject::connect(buttonOk,			SIGNAL(clicked()),				this, SLOT(OnOk()));
    QObject::connect(FromDate,			SIGNAL(dateChanged(QDate)),		this, SLOT(OnFromDateSpinWheel(QDate)));
    QObject::connect(ToDate,			SIGNAL(dateChanged(QDate)),		this, SLOT(OnToDateSpinWheel(QDate)));
    QObject::connect(buttonOk_2,		SIGNAL(clicked()),				this, SLOT(OnCancel()));

	// Default dates are 'Today'
	QDate Today = QDate::currentDate();
	From = To = Today;
	// Default: look in sub-folders
	CheckBoxSubfolders->setChecked(true);

	// Update GUI accordingly
	UpdateFromToFields(false);
	TextStatus->setText("");
}

///////////////////////////////////////////////////////////
// User want to use the calendar object to set 'From' date 
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnFromDateCalendar(void)
{
	// Create Calendar object
	CalendarDialog *pCalendar = new CalendarDialog();
	pCalendar->setDate(From);

	// Show calendar, let user pick a date
	if(pCalendar->exec() != 1)
		return;
	
	// Read date picked by the user
	From = pCalendar->getDate();
	
	// Update GUI accordingly
	UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the calendar object to set 'To' date 
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnToDateCalendar(void)
{
	// Create Calendar object
	CalendarDialog *pCalendar = new CalendarDialog();
	pCalendar->setDate(To);

	// Show calendar, let user pick a date
	if(pCalendar->exec() != 1)
		return;

	// Read date picked by the user
	To = pCalendar->getDate();

	// Update GUI accordingly
	UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the Date SpinWheel object to set 'From' date 
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnFromDateSpinWheel(const QDate& Fromdate)
{
	// If no change...ignore (also avoids infinite call due to 'UpdateFromToFields')
	if(From == Fromdate)
		return;

	// Update variable
	From = Fromdate;

	// Update GUI accordingly
	UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// User want to use the Date SpinWheel object to set 'To' date 
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnToDateSpinWheel(const QDate& Todate)
{
	// If no change...ignore (also avoids infinite call due to 'UpdateFromToFields')
	if(To == Todate)
		return;

	// Update variable
	To = Todate;

	// Update GUI accordingly
	UpdateFromToFields(false);
}

///////////////////////////////////////////////////////////
// Swap dates if needed, update fields.
///////////////////////////////////////////////////////////
void	TimePeriodDialog::UpdateFromToFields(bool bCheckForSwap)
{
	QDate Date;

	// Check if need to swap dates!
	if((bCheckForSwap == true) && (From > To))
	{
		Date = To;
		To= From;
		From= Date;
	}

	// Update 'From' date field + comment
	FromDate->setDate(From);
	TextLabelFromDate->setText(From.toString(Qt::TextDate));

	// Update 'To' date field + comment
	ToDate->setDate(To);
	TextLabelToDate->setText(To.toString(Qt::TextDate));
}

///////////////////////////////////////////////////////////
// User browses to specify a folder
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnBrowseFolder(void)
{
	QString s;
	
	// Get current path entered (if any).
	s = LineEditPath->text();

	// If no path define yet, start from default directory.
	if(s.isEmpty() == true)
		s = ".";

	// Popup directory browser
	s = QFileDialog::getExistingDirectory(
                    this,
                    "Choose a directory (path to STDF files to import)",
					s,
					QFileDialog::ShowDirsOnly );

	// Check if valid selection
	if(s.isEmpty() == true)
		return;

	// Save folder selected.
	LineEditPath->setText(s);
}

///////////////////////////////////////////////////////////
// User clicked 'Ok' button...process fields.
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnOk(void)
{
	// Disabled 'Ok' button'
	buttonOk->setEnabled(false);

	// Erase list of files if any exist.
	ImportFile.clear();
	lFilesChecked  = 0;
	lFilesSelected = 0;

	// Check Start-ending date if need to swap them!
	UpdateFromToFields(true);

	// Recursive function: looks for all stdf files in directory matching criteria.
	CheckFilesInDirectory(LineEditPath->text());
	
	// Full folders scan completed: enable 'Ok' button'...for next call!
	buttonOk->setEnabled(true);
	TextStatus->setText("");

	// Notify dialog box that it can return to caller!
	done(1);
}

///////////////////////////////////////////////////////////
// User clicked 'Ok' button...process fields.
///////////////////////////////////////////////////////////
void	TimePeriodDialog::OnCancel(void)
{

	// Empty list as user aborted process.
	ImportFile.clear();

	// Enable 'Ok' button'...for next call!
	buttonOk->setEnabled(true);
	TextStatus->setText("");

	// Notify dialog box that it can return to caller!...ABORT
	done(0);
}

///////////////////////////////////////////////////////////
// Check for all STDF file found in the given directory
///////////////////////////////////////////////////////////
void	TimePeriodDialog::CheckFilesInDirectory(QString strFolder)
{
	QDir			ScanDir;
	QDir			ScanFiles;
	QStringList 	pList;
	QStringList::Iterator it;
	QString			strSubFolder;
	
	// If have to check sub-folders...do it!
	if(CheckBoxSubfolders->isChecked() == true)
	{
		ScanDir.setPath(strFolder);
        pList = ScanDir.entryList(QDir::nameFiltersFromString("*"),QDir::Dirs,QDir::Unsorted);

		// Scan list and do recurssive calls to sub-folders 
		for(it = pList.begin(); it != pList.end(); ++it ) 
		{
			strSubFolder = strFolder + "/" + *it;
			// Only include valid sub-directories.
			if(*it != "." && *it != "..")
				CheckFilesInDirectory(strSubFolder);
		}
	}

	// Get list of files matching filter.
	ScanFiles.setPath(strFolder);
    pList = ScanFiles.entryList(QDir::nameFiltersFromString("*.std;*.stdf"),QDir::Files,QDir::Unsorted);

	// Update 'Files Checked' counter
	lFilesChecked++;
	// Every 16 files analyzed, update screen !
	UpdateStatus();

	// Scan list: save files into our list, and recurssive call to sub-folders 
	QString	strFile;
	for(it = pList.begin(); it != pList.end(); ++it ) 
	{
		// Check the file for matching criteria...and add it to the list of STDF files to import if pass filter
		strFile = ScanFiles.path();
		strFile += "/";
		strFile += (*it);
#ifdef _WIN32
		// Ensure path is DOS compatible!
        int i = strFile.indexOf( '/' );
		while(i>=0)
		{
			strFile[i]='\\';
            i = strFile.indexOf( '/',i );
		};
#endif
		// Import this file...if it matches the filter criteria!
		CheckImportFile(strFile);
	}
}

int TimePeriodDialog::ConvertToSTDFV4( const QString &fileName, QString &convertedFileName ) const
{
    GS::Gex::ConvertToSTDF StdfConvert;
    convertedFileName = fileName + "_galaxy.std";
    bool isFileCreated = false;
    QString errorMessage;

    int convertStatus = StdfConvert.Convert
            ( false, true, false, true,
              fileName,convertedFileName,
              "", isFileCreated, errorMessage );

    if(convertStatus == GS::Gex::ConvertToSTDF::eConvertError)
    {
        // Exit cleanup: erase temporary files.
        QDir currentDirectory;
        currentDirectory.remove(convertedFileName);

        errorMessage = "*Error* Failed parsing data file. File may be corrupted:\n" + fileName;
        return ReadError;
    }

    // Check if Input file was already in STDF format!
    if(isFileCreated == false)
        convertedFileName = fileName;

    return NoError;
}

///////////////////////////////////////////////////////////
// Check if file matches Import criteria. If ok, add it to import list
///////////////////////////////////////////////////////////
void	TimePeriodDialog::CheckImportFile(QString strFile)
{
    GS::StdLib::Stdf				StdfFile;
    GS::StdLib::StdfRecordReadInfo	StdfRecordHeader;
	int					iStatus;

    QString convertedFileName;

    // convert the file if needed
    iStatus = ConvertToSTDFV4( strFile, convertedFileName );

    if(iStatus != GS::StdLib::Stdf::NoError)
        return;	// Error opening file!

	// Open STDF file to read until MIR record found...have a 50K cache buffer used (instead of 2M !)
    iStatus = StdfFile.Open(convertedFileName.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
		return;	// Error opening file!


	// Read all STDF file...until MIR record is found.
	do
	{
		// Read one record from STDF file.
		iStatus = StdfFile.LoadRecord(&StdfRecordHeader);			
        if(iStatus != GS::StdLib::Stdf::NoError)
		{
			// End of file, No MIR found!...ignore this file!
			StdfFile.Close();	// Clean close.
			return;
		}

		// Process STDF record read.
		if((StdfRecordHeader.iRecordType == 1) && (StdfRecordHeader.iRecordSubType == 10))
		{
			BYTE	bData;
			int		wData;
            time_t	tSetupTime = time_t(), tStartTime = time_t();
			char	szString[MIR_STRING_SIZE];
			char	szLot[MIR_STRING_SIZE];			// Lot name
			char	szPartType[MIR_STRING_SIZE];	// Device name
			char	szNodeName[MIR_STRING_SIZE];	// Tester name
			char	szJobName[MIR_STRING_SIZE];		// Program name
			bool	bPassFilter;
			QDate	Date;


			// MIR found, get data!
			switch(StdfRecordHeader.iStdfVersion)
			{
            default :
                break;

			  case GEX_STDFV4:
				StdfFile.ReadDword((long *)&tSetupTime);	// Setup_T
				StdfFile.ReadDword((long *)&tStartTime);	// Start_T
				StdfFile.ReadByte(&bData);					// statio #
				StdfFile.ReadByte(&bData);					// mode_code
				StdfFile.ReadByte(&bData);					// rtst_code
				StdfFile.ReadByte(&bData);					// prot_cod #
				StdfFile.ReadWord(&wData);					// burn_time
				StdfFile.ReadByte(&bData);					// cmode_code
				ReadStringToField(&StdfFile,szLot);			// Lot name
				ReadStringToField(&StdfFile,szPartType);	// Device name
				ReadStringToField(&StdfFile,szNodeName);	// Tester name
				ReadStringToField(&StdfFile,szString);		// TesterType
				ReadStringToField(&StdfFile,szJobName);		// Program name
#if 0
				// Extra data that may be used in future filters
				ReadStringToField(&StdfFile,szString);		// JobRev
				ReadStringToField(&StdfFile,szString);		// Sublot
				ReadStringToField(&StdfFile,szString);		// Operator
#endif
				break;
				}

			// Check if this file passes the filter...
			bPassFilter = true;

			// Check if 'start' date matched Calendar...
			QDateTime FileDateTime;
			FileDateTime.setTime_t(tStartTime);
			if((FileDateTime.date() < From) || (FileDateTime.date() > To))
				bPassFilter = false;	// This file hasn't been created in the time period we want...

			// Now, will check test info filter
            QRegExp rx("", Qt::CaseInsensitive, QRegExp::Wildcard);	// NOT case sensitive, support wildchar.

			// Check if 'tester name' matching filter
			rx.setPattern(TesterNameFilter->text());
            if(rx.indexIn(szNodeName) < 0)
				bPassFilter = false;  // Tester name not macthing criteria

			// Check if 'device name' matching filter
			rx.setPattern(DeviceNameFilter->text());
            if(rx.indexIn(szPartType) < 0)
				bPassFilter = false;  // device name not macthing criteria

			// Check if 'program name' matching filter
			rx.setPattern(ProgramNameFilter->text());
            if(rx.indexIn(szJobName) < 0)
				bPassFilter = false;  // program name not macthing criteria

			// Check if 'lot name' matching filter
			rx.setPattern(LotNameFilter->text());
            if(rx.indexIn(szLot) < 0)
				bPassFilter = false;  // lot name not macthing criteria

			// Check if ALL filers passed...if so, we add this file name to our 'import' list
			if(bPassFilter == true)
			{
				ImportFile.append(strFile);	// Add full file path.
				lFilesSelected++;
			}

			// Clean close STDF file
			StdfFile.Close();
			return;
		}
	}
	while(1);	// Loop until MIR or end of file found
}

///////////////////////////////////////////////////////////
// Reads a string from STDF file
///////////////////////////////////////////////////////////
void	TimePeriodDialog::ReadStringToField(GS::StdLib::Stdf *pStdfFile,char *szField)
{
	char	szString[257];	// A STDF string is 256 bytes long max!
	
	// Empties string.
	*szField=0;

	// Read string from STDF file.
    if(pStdfFile->ReadString(szString)  != GS::StdLib::Stdf::NoError)
	  return;
    // Security: ensures we do not overflow destination buffer !
	szString[MIR_STRING_SIZE-1] = 0;
	strcpy(szField,szString);
}

///////////////////////////////////////////////////////////
// Tells current number of files analyzed / selected
///////////////////////////////////////////////////////////
void	TimePeriodDialog::UpdateStatus(void)
{
	// Only update screen every 16 files/folders checked.
	if((lFilesChecked & 0xf) != 0)
		return;

	QString strString;

	strString = "Files selected/checked:  " + QString::number(lFilesSelected) + " / " + QString::number(lFilesChecked);
	TextStatus->setText(strString);
	// Get screen updated!
    QCoreApplication::processEvents();
}
