//////////////////////////////////////////////////////////////////////
// import_csv_wizard.cpp: Wizard to import/parse a .CSV file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <stdio.h>
#include <stdlib.h>
#include <QFileInfo>
#include <QCursor>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QTextStream>
#include <QMenu>
#include <QDateTime>
#include "message.h"
#include "gex_shared.h"
#include "tb_import_csv_wizard_dialog.h"
#include "import_constants.h"
#include "browser_dialog.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"

// in main.cpp
extern GexMainwindow *	pGexMainWindow;


static 	bool	m_bTestInLine;
static 	BYTE	*m_pColumnType;			// List of flags telling Column type (Data, or Label, etc,...)
static 	BYTE	*m_pLineType;			// List of flags telling Line type (Data, or Label, etc,...)
static  bool	bFailedExecution;
static	QString	m_strProfileTemplateFile;	// Parsing template file (if used)
static	bool	bApplyProfileToAllFiles=false;

#define	C_INFINITE_FLOAT	(float) 9e37

QMap <int,float>	map_lfScaling;		// Scaling value so result is always normalized (eg: 1e-3 if mVolts, etc...)
QMap <int,double>	map_lfLowLimit;		// Tests low-limits list
QMap <int,double>	map_lfHighLimit;	// Tests high-limit list

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGCSV_WizardtoSTDF::CGCSV_WizardtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGCSV_WizardtoSTDF::~CGCSV_WizardtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGCSV_WizardtoSTDF::GetLastError()
{
    strLastError = "Wizard Import CSV / TXT: ";

    switch(iLastError)
    {
        default:
        case errNoError:
            strLastError += "No Error";
            break;
        case errAbort:
            strLastError += "Manual abort";
            break;
        case errOpenFail:
            strLastError += "Failed to open file";
            break;
        case errWriteSTDF:
            strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            strLastError += "License has expired or Data file out of date...\nPlease contact "+QString(GEX_EMAIL_SALES);
            break;
    }
    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Checks if .CSL parsing definition available in folder
//////////////////////////////////////////////////////////////////////
bool CGCSV_WizardtoSTDF::isParsingConfigAvailable(QString strCsvFile,QString &strParsingConfigFile)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    // Clear variables
    strParsingConfigFile = "";

    // Get folder location (where .CSV / .TXT file is located)
    QDir	cDir;
    QFileInfo cFileInfo(strCsvFile);
    cDir.setPath(cFileInfo.path());
    cDir.setFilter(QDir::Files);
    QStringList strCslEntries = cDir.entryList(QDir::nameFiltersFromString("*.csl"));
    if(strCslEntries.count() != 1)
        return false;	// We should only find ONE .csl file

    // .csl parsing config file found...sucess, unless contents is NOT valid!
    QString strFile = cFileInfo.path() + "/" + strCslEntries[0];

    // Read Header of Parser template
    QString strString;
    QFile file(strFile);
    if (file.open(QIODevice::ReadOnly) == false)
        return false;	// Failed opening Tasks file.

    // Read Tasks definition File
    QTextStream hFile;
    hFile.setDevice(&file);	// Assign file handle to data stream

    strString= hFile.readLine();
    if(strString != "<gex_template>")
        return false;
    strString = hFile.readLine();
    if(strString != "<BlockType = parser_rules>")
        return false;

    // Success
    strParsingConfigFile = strFile;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' CSV file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool
CGCSV_WizardtoSTDF::Convert(const char* CsvFileName,
                            const char* strFileNameSTDF,
                            QString strParserConfig /*= ""*/)
{
    // No erro (default)
    iLastError = errNoError;

    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(CsvFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    QDateTime dtOutput	= fOutput.lastModified();
    QDateTime dtInput	= fInput.lastModified();

    if((f.exists() == true) && (dtOutput > dtInput))
        return true;

    // If Parser file specified, auto load it!
    if(QFile::exists(strParserConfig))
    {
        bApplyProfileToAllFiles = true;
        m_strProfileTemplateFile = strParserConfig;
    }

    // Display wizard
    ImportCsvWizardDialog cWizard;
    QString r=cWizard.setFile(CsvFileName,strFileNameSTDF);
    if (r.startsWith("error"))
        return false;

    // If a parsing profile already loaded, and we have to apply it to all files, then do it!
    if(bApplyProfileToAllFiles)
    {
        // Check if apply same parsing file as in previous files (if any already processed)
        cWizard.loadParserProfile();

        // Have file parsed
        return cWizard.ParseFile();
    }
    else if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        return false;
    }
    else
    {
        if(cWizard.exec())
            return true;	// Import successful
        else
            return false;	// Error or abort.
    }
}

//////////////////////////////////////////////////////////////////////
// Import CSV Wizard: Constructor
//////////////////////////////////////////////////////////////////////
ImportCsvWizardDialog::ImportCsvWizardDialog( QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(pushButton3,					SIGNAL(clicked()), this, SLOT(OnNext()));
    QObject::connect(checkBoxTabDelimiter,			SIGNAL(clicked()), this, SLOT(OnDelimiterChange()));
    QObject::connect(checkBoxSemicolonDelimiter,	SIGNAL(clicked()), this, SLOT(OnDelimiterChange()));
    QObject::connect(checkBoxCommaDelimiter,		SIGNAL(clicked()), this, SLOT(OnDelimiterChange()));
    QObject::connect(checkBoxSpaceDelimiter,		SIGNAL(clicked()), this, SLOT(OnDelimiterChange()));
    QObject::connect(buttonSaveParserProfile,		SIGNAL(clicked()), this, SLOT(OnSaveParserProfile()));
    QObject::connect(buttonLoadParserProfile,		SIGNAL(clicked()), this, SLOT(OnLoadParserProfile()));
    QObject::connect(checkBoxOtherDelimiter,		SIGNAL(clicked()), this, SLOT(OnDelimiterChange()));
    QObject::connect(spinBoxStartImportRow,			SIGNAL(valueChanged(int)),				this, SLOT(OnStartingCoordinateChange(int)));
    QObject::connect(mUi_pQtwExcelTableWidget,		SIGNAL(pressed(QModelIndex)),	this, SLOT(OnSelectionMade(QModelIndex)));
    QObject::connect(radioButtonTestInLine,			SIGNAL(toggled(bool)),					this, SLOT(OnUpdateTableLabels()));
    QObject::connect(lineEditOtherDelimiter,		SIGNAL(textChanged(QString)),			this, SLOT(OnDelimiterChange()));
    QObject::connect(spinBoxStartImportCol,			SIGNAL(valueChanged(int)),				this, SLOT(OnStartingCoordinateChange(int)));

    // Default: CSV parameter list on disk includes all known CSV parameters...
    bNewCsvParameterFound = false;

    m_pColumnType=NULL;	// List of flags telling Column type (Data, or Label, etc,...)
    m_pLineType=NULL;	// List of flags telling Line type (Data, or Label, etc,...)

    // Default delimiter: comma
    checkBoxCommaDelimiter->setChecked(true);
    lineEditOtherDelimiter->setEnabled(false);


    // Default parsing: One Parameters per line, Values are the same line
    radioButtonTestInLine->setChecked(true);
    m_bTestInLine		= true;

    resetPartInfoFlagForOffset();
    resetTestInfoFlagForOffset();

    m_lEditorRows		= 10;		// At startup, allocates 10 lines
    m_lEditorCols		= 10;		// At startup, allocates 10 columns

    // Unless the parsing profile is saved (or loaded) from disk, hide the 'use profile on all other files' check box...
    checkBoxParseAllFiles->hide();
}

//////////////////////////////////////////////////////////////////////
// Load CSV Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::LoadParameterIndexTable(void)
{
    QString	strCsvTableFile;
    QString	strString;

    strCsvTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strCsvTableFile += GEX_CSV_PARAMETERS;

    // Open CSV Parameter table file
    QFile f( strCsvTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hCsvTableFile(&f);

    // Skip comment lines
    do
    {
      strString = hCsvTableFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (hCsvTableFile.atEnd() == false));

    // Read lines
    pFullCsvParametersList.clear();
    strString = hCsvTableFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        pFullCsvParametersList.append(strString);
        // Read next line
        strString = hCsvTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save CSV Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::DumpParameterIndexTable(void)
{
    QString		strCsvTableFile;
    int	nIndex;

    strCsvTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strCsvTableFile += GEX_CSV_PARAMETERS;

    // Open CSV Parameter table file
    QFile f( strCsvTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hCsvTableFile(&f);

    // First few lines are comments:
    hCsvTableFile << "############################################################" << endl;
    hCsvTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hCsvTableFile << "# Quantix Examinator: CSV Parameters detected" << endl;
    hCsvTableFile << "# www.mentor.com" << endl;
    hCsvTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hCsvTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // pFullCsvParametersList.sort();
    for(nIndex=0;nIndex<pFullCsvParametersList.count();nIndex++)
    {
        // Write line
        hCsvTableFile << pFullCsvParametersList[nIndex] << endl;
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this CSV parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::UpdateParameterIndexTable(QString strParamName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(pFullCsvParametersList.isEmpty() == true)
    {
        // Load CSV parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(pFullCsvParametersList.indexOf(strParamName) < 0)
    {
        // Update list
        pFullCsvParametersList.append(strParamName);

        // Set flag to force the current CSV table to be updated on disk
        bNewCsvParameterFound = true;
    }
}

//////////////////////////////////////////////////////////////////////
// Get file to read and STDF output name.
//////////////////////////////////////////////////////////////////////
QString ImportCsvWizardDialog::setFile(QString strCsvFileName,QString strFileNameSTDF)
{
    m_strCsvFileName = strCsvFileName;
    m_strFileNameSTDF = strFileNameSTDF;

    // File name
    textLabelDataFile->setText(m_strCsvFileName);

    // Load file in table...
    QString r=LoadFileInTable();
    GSLOG(SYSLOG_SEV_DEBUG, QString(" for file %1 : %2")
          .arg( strCsvFileName.toLatin1().constData())
          .arg(r).toLatin1().constData());
    return r;
}

//////////////////////////////////////////////////////////////////////
// Delimiter change...reload file in table with new parsing method
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::OnDelimiterChange()
{
    // Update custom delimiter field
    lineEditOtherDelimiter->setEnabled(checkBoxOtherDelimiter->isChecked());

    // Force to reload file in table...but do not clear existing settings (eg: raw/line type)
    LoadFileInTable();
}

//////////////////////////////////////////////////////////////////////
// Import CSV Wizard: Load file
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::OnStartingCoordinateChange(int /*iStartFromRow*/)
{
    // Force to reload file in table...
    LoadFileInTable();
}

//////////////////////////////////////////////////////////////////////
// Contextual menu Table clicked
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::OnSelectionMade(QModelIndex qmiModelIndex)
{
    QString strTextTestInfo,strTextPartInfo;
    int		iOffsetTestInfo,iOffsetPartInfo;
    BYTE	*pFlagTestInfo,*pFlagPartInfo;

    Qt::MouseButtons mbMouseButton = QGuiApplication::mouseButtons();
    if(mbMouseButton == Qt::LeftButton)
        return;

    int row = qmiModelIndex.row();
    int col = qmiModelIndex.column();

    if(radioButtonTestInLine->isChecked())
    {
        // One Parameter per line.
        m_bTestInLine = true;

        // Test info handles
        iOffsetTestInfo = col;
        strTextTestInfo = "Change Column " + QString::number(col+1);
        pFlagTestInfo = &m_pColumnType[iOffsetTestInfo];

        // Part info handles
        iOffsetPartInfo = row;
        strTextPartInfo = "Change Line " + QString::number(row+1);
        pFlagPartInfo = &m_pLineType[iOffsetPartInfo];
    }
    else
    {
        // One Parameter per Column.
        m_bTestInLine = false;

        // Test info handles
        iOffsetTestInfo = row;
        strTextTestInfo = "Change Line " + QString::number(row+1);
        pFlagTestInfo = &m_pLineType[iOffsetTestInfo];

        // Part info handles
        iOffsetPartInfo = col;
        strTextPartInfo = "Change Column " + QString::number(col+1);
        pFlagPartInfo = &m_pColumnType[iOffsetPartInfo];
    }

    strTextTestInfo += " (";
    switch(*pFlagTestInfo)
    {
        case	GEX_TB_TEST_ID_LABEL:
            strTextTestInfo += GEX_TB_TEST_LABEL_LABEL;
            break;
        case	GEX_TB_TEST_ID_UNITS:
            strTextTestInfo += GEX_TB_TEST_LABEL_UNITS;
            break;
        case	GEX_TB_TEST_ID_LL:
            strTextTestInfo += GEX_TB_TEST_LABEL_LL;
            break;
        case	GEX_TB_TEST_ID_HL:
            strTextTestInfo += GEX_TB_TEST_LABEL_HL;
            break;
        case	GEX_TB_TEST_ID_DATA:
            strTextTestInfo += GEX_TB_TEST_LABEL_DATA;
            break;
        case	GEX_TB_TEST_ID_TEST:
            strTextTestInfo += GEX_TB_TEST_LABEL_TEST;
            break;
        case	GEX_TB_TEST_ID_IGNORE:
            strTextTestInfo += GEX_TB_TEST_LABEL_IGNORE;
            break;
    }
    strTextTestInfo += ") to...";

    strTextPartInfo += " (";
    switch(*pFlagPartInfo)
    {
        case	GEX_TB_PART_ID_TESTPF:
            strTextPartInfo += GEX_TB_PART_LABEL_TESTPF;
            break;
        case	GEX_TB_PART_ID_SBIN:
            strTextPartInfo += GEX_TB_PART_LABEL_SBIN;
            break;
        case	GEX_TB_PART_ID_HBIN:
            strTextPartInfo += GEX_TB_PART_LABEL_HBIN;
            break;
        case	GEX_TB_PART_ID_DIEX:
            strTextPartInfo += GEX_TB_PART_LABEL_DIEX;
            break;
        case	GEX_TB_PART_ID_DIEY:
            strTextPartInfo += GEX_TB_PART_LABEL_DIEY;
            break;
        case	GEX_TB_PART_ID_SBLOT:
            strTextPartInfo += GEX_TB_PART_LABEL_SBLOT;
            break;
        case	GEX_TB_PART_ID_SITE:
            strTextPartInfo += GEX_TB_PART_LABEL_SITE;
            break;
        case	GEX_TB_PART_ID_IGNORE:
            strTextPartInfo += GEX_TB_PART_LABEL_IGNORE;
            break;
    }
    strTextPartInfo += ") to...";

    QMenu menu( this );
    menu.addAction(strTextTestInfo);
    QAction * pActionLabel			= menu.addAction( "  - Label" );
    QAction * pActionTest			= menu.addAction( "  - Test#" );
    QAction * pActionUnits			= menu.addAction( "  - Units" );
    QAction * pActionLowL			= menu.addAction( "  - Low Limit" );
    QAction * pActionHighL			= menu.addAction( "  - High Limit" );
    QAction * pActionValues			= menu.addAction( "  - Data" );
    QAction * pActionIgnore			= menu.addAction( "  - Ignore" );
    menu.addSeparator();
    menu.addAction(strTextPartInfo);
    QAction * pActionSublotID		= menu.addAction( "  - Wafer ID" );
    QAction * pActionTestPassFail	= menu.addAction( "  - Device P/F flag" );
    QAction * pActionSoftBin		= menu.addAction( "  - Soft Bin#" );
    QAction * pActionHardBin		= menu.addAction( "  - Hard Bin#" );
    QAction * pActionDieX			= menu.addAction( "  - Die X" );
    QAction * pActionDieY			= menu.addAction( "  - Die Y" );
    QAction * pActionSite			= menu.addAction( "  - Site" );
    QAction * pActionPartIgnore		= menu.addAction( "  - Ignore" );

    menu.setMouseTracking(true);
    QAction * pActionResult = menu.exec(QCursor::pos());
    menu.setMouseTracking(true);

    int	iCellType=0;
    if(pActionResult == pActionLabel)
        iCellType = GEX_TB_TEST_ID_LABEL;
    if(pActionResult == pActionTest)
        iCellType = GEX_TB_TEST_ID_TEST;
    if(pActionResult == pActionUnits)
        iCellType = GEX_TB_TEST_ID_UNITS;
    if(pActionResult == pActionLowL)
        iCellType = GEX_TB_TEST_ID_LL;
    if(pActionResult == pActionHighL)
        iCellType = GEX_TB_TEST_ID_HL;
    if(pActionResult == pActionValues)
        iCellType = GEX_TB_TEST_ID_DATA;
    if(pActionResult == pActionIgnore)
        iCellType = GEX_TB_TEST_ID_IGNORE;
    if(pActionResult == pActionTestPassFail)
        iCellType = GEX_TB_PART_ID_TESTPF;
    if(pActionResult == pActionSoftBin)
        iCellType = GEX_TB_PART_ID_SBIN;
    if(pActionResult == pActionHardBin)
        iCellType = GEX_TB_PART_ID_HBIN;
    if(pActionResult == pActionDieX)
        iCellType = GEX_TB_PART_ID_DIEX;
    if(pActionResult == pActionDieY)
        iCellType = GEX_TB_PART_ID_DIEY;
    if(pActionResult == pActionSublotID)
        iCellType = GEX_TB_PART_ID_SBLOT;
    if(pActionResult == pActionSite)
        iCellType = GEX_TB_PART_ID_SITE;
    if(pActionResult == pActionPartIgnore)
        iCellType = GEX_TB_PART_ID_IGNORE;

    UpdateCellType(row,col,iCellType);
    repaintItems();
}


//////////////////////////////////////////////////////////////////////
// Enable disable interactive controls
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::setEnabledInteractiveControls(bool value)
{
    spinBoxStartImportRow->setEnabled(value);
    spinBoxStartImportCol->setEnabled(value);
    buttonLoadParserProfile->setEnabled(value);
    buttonSaveParserProfile->setEnabled(value);
    pushButton3->setEnabled(value);
    groupBoxDataPreview->setEnabled(value);
    groupBoxDelimiters->setEnabled(value);
    groupBoxOrientation->setEnabled(value);
    QCoreApplication::processEvents();
}


//////////////////////////////////////////////////////////////////////
// Check all Part Info type to set it to -1 if it was linked to
// selected offset
// if offset is set -1 reset all part info offset
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::resetPartInfoFlagForOffset(int offset/*=-1*/)
{
    // Holds line (or Col) offset to the SOTFWARE bin#
    if (m_lSoftBinOffset == offset || offset == -1)
        m_lSoftBinOffset = -1;
    // Holds line (or Col) offset to the HARDWARE bin#
    if (m_lHardBinOffset == offset || offset == -1)
        m_lHardBinOffset = -1;
    // Holds line (or Col) offset to the DieX (X loc)
    if (m_lDieXOffset == offset || offset == -1)
        m_lDieXOffset = -1;
    // Holds line (or Col) offset to the DieY (Y loc)
    if (m_lDieYOffset == offset || offset == -1)
        m_lDieYOffset = -1;
    // Holds line (or Col) offset to the SubotID / WaferID
    if (m_lSublotOffset == offset || offset == -1)
        m_lSublotOffset = -1;
    // Holds line (or Col) offset to the Site
    if (m_lSiteOffset == offset || offset == -1)
        m_lSiteOffset = -1;
}

//////////////////////////////////////////////////////////////////////
// Check all Test Info type to set it to -1 if it was linked to
// selected offset
// if offset is set -1 reset all test info offset
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::resetTestInfoFlagForOffset(int offset/*=-1*/)
{
    // Holds line (or Col) offset to the Label data
    if (m_lLabelOffset == offset || offset == -1)
        m_lLabelOffset = -1;
    // Holds line (or Col) offset to the Units data
    if (m_lUnitsOffset == offset || offset == -1)
        m_lUnitsOffset = -1;
    // Holds line (or Col) offset to the Test# data
    if (m_lTestOffset == offset || offset == -1)
        m_lTestOffset = -1;
    // Holds line (or Col) offset to the Low Limit data
    if (m_lLowLOffset == offset || offset == -1)
        m_lLowLOffset = -1;
    // Holds line (or Col) offset to the High Limit data
    if (m_lHighLOffset == offset || offset == -1)
        m_lHighLOffset = -1;
}

//////////////////////////////////////////////////////////////////////
// Update the line / col type (units, limits, data, etc...)
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::UpdateCellType(int row, int col, int iCellType)
{
    // Check for valid cell type.
    if(	iCellType < 0)
        return;

    ///		TO REVIEW : test id not necessary in col; part id not necessary in row;
    if (iCellType >= GEX_TB_TEST_ID_LABEL && iCellType <= GEX_TB_TEST_ID_TEST && col > m_lEditorCols)
        return;

    if (iCellType >= GEX_TB_PART_ID_TESTPF && iCellType <= GEX_TB_PART_ID_PARAMETER && row > m_lEditorRows)
        return;

    int		iOffsetTestInfo,iOffsetPartInfo;
    QTableWidgetItem *pQtwiHeaderItem;
    BYTE	*pFlagTestInfo,*pFlagPartInfo;

    if(radioButtonTestInLine->isChecked())
    {
        // One Parameter per line.
        m_bTestInLine = true;

        // Test info handle
        iOffsetTestInfo = col;
        pFlagTestInfo = &m_pColumnType[iOffsetTestInfo];

        // Part info handle
        iOffsetPartInfo = row;
        pFlagPartInfo = &m_pLineType[iOffsetPartInfo];
    }
    else
    {
        // One Parameter per Column.
        m_bTestInLine = false;

        // Test info handle
        iOffsetTestInfo = row;
        pFlagTestInfo = &m_pLineType[iOffsetTestInfo];

        // Part info handle
        iOffsetPartInfo = col;
        pFlagPartInfo = &m_pColumnType[iOffsetPartInfo];
    }

    // Setting labels is VERY LONG, ...so for faster processing, only first 256 Rows & Cols labels are defined!
    bool bSetLabel=false;
    if(iOffsetTestInfo <= GEX_TB_MAXLINES_PREVIEW)
        bSetLabel = true;

    // menu selection
    switch(iCellType)
    {
        case GEX_TB_TEST_ID_LABEL:
            // This line/column is of 'Label' type
            *pFlagTestInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_LABEL));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_LABEL));
                }
            }
            m_lLabelOffset = iOffsetTestInfo;		// Holds line (or Col) offset to the Label data
            break;

        case GEX_TB_TEST_ID_TEST:
            // This line/column is of 'Test#' type
            *pFlagTestInfo	= iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_TEST));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_TEST));
                }
            }
            m_lTestOffset	= iOffsetTestInfo;		// Holds line (or Col) offset to the test# data
            break;

        case GEX_TB_TEST_ID_UNITS:
            // This line/column is of 'Units' type
            *pFlagTestInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_UNITS));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_UNITS));
                }
            }
            m_lUnitsOffset = iOffsetTestInfo;		// Holds line (or Col) offset to the units data
            break;

        case GEX_TB_TEST_ID_LL:
            // This line/column is of 'Low L.' type
            *pFlagTestInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_LL));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_LL));
                }
            }
            m_lLowLOffset = iOffsetTestInfo;		// Holds line (or Col) offset to the Low Limit data
            break;

        case GEX_TB_TEST_ID_HL:
            // This line/column is of 'High L.' type
            *pFlagTestInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_HL));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_HL));
                }
            }
            m_lHighLOffset = iOffsetTestInfo;		// Holds line (or Col) offset to the High Limit data
            break;

        case GEX_TB_TEST_ID_DATA:
            // This line/column is of 'Data' type
            *pFlagTestInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_DATA));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_DATA));
                }
            }
            break;

        case GEX_TB_TEST_ID_IGNORE:
            // This line/column is of 'Ignore' type
            *pFlagTestInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_IGNORE));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetTestInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetTestInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_IGNORE));
                }
            }
            resetTestInfoFlagForOffset(iOffsetTestInfo);
            break;

        case GEX_TB_PART_ID_TESTPF:
            // This line/column is of 'Test pass/fail' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_TESTPF));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_TESTPF));
                }
            }
            break;

        case GEX_TB_PART_ID_SBIN:
            // This line/column is of 'Soft Bin' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_SBIN));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_SBIN));
                }
            }
            m_lSoftBinOffset = iOffsetPartInfo;		// Holds line (or Col) offset to the Soft Bin data
            break;

        case GEX_TB_PART_ID_HBIN:
            // This line/column is of 'Hard Bin' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_HBIN));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_HBIN));
                }
            }
            m_lHardBinOffset = iOffsetPartInfo;		// Holds line (or Col) offset to the Hard Bin data
            break;

        case GEX_TB_PART_ID_DIEX:
            // This line/column is of 'DieX' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_DIEX));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_DIEX));
                }
            }
            m_lDieXOffset = iOffsetPartInfo;		// Holds line (or Col) offset to the DieX data
            break;

        case GEX_TB_PART_ID_DIEY:
            // This line/column is of 'DieY' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_DIEY));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_DIEY));
                }
            }
            m_lDieYOffset = iOffsetPartInfo;		// Holds line (or Col) offset to the DieY data
            break;

        case GEX_TB_PART_ID_SBLOT:
            // This line/column is of 'SubLotID' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_SBLOT));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_SBLOT));
                }
            }
            m_lSublotOffset = iOffsetPartInfo;		// Holds line (or Col) offset to the Wafer ID data
            break;

        case GEX_TB_PART_ID_SITE:
            // This line/column is of 'Site' type
            *pFlagPartInfo = iCellType;
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_SITE));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_SITE));
                }
            }
            m_lSiteOffset = iOffsetPartInfo;		// Holds line (or Col) offset to the Site ID data
            break;

        case GEX_TB_PART_ID_IGNORE:
            // This line/column is of 'Ignore' type
            *pFlagPartInfo = iCellType;		// Save flag info
            if(bSetLabel)
            {
                if(m_bTestInLine)
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_IGNORE));
                }
                else
                {
                    pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iOffsetPartInfo);
                    // m_bTestInLine == true if test info in lines // horizontal header
                    // if test info in lines, part info in column
                    if(!pQtwiHeaderItem)
                    {
                        pQtwiHeaderItem = new QTableWidgetItem;
                        mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iOffsetPartInfo, pQtwiHeaderItem);
                    }
                    pQtwiHeaderItem->setText(tr(GEX_TB_PART_LABEL_IGNORE));
                }
            }
            resetPartInfoFlagForOffset(iOffsetPartInfo);
            break;
    }

    // If we've disabled a line / column, or set it to 'Data'  we're done here
    if((iCellType == GEX_TB_TEST_ID_IGNORE) || (iCellType == GEX_TB_TEST_ID_DATA) || (iCellType == GEX_TB_PART_ID_IGNORE))
        return;

    QString strText;
    int iMaxLabels;

    // We've toggled a line/column to 'Label' or 'Limit',etc...then make sure it is unique!
    if(m_bTestInLine)
    {
        // Check that no other column is using the same label type...if it does, switch it to 'Data'
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorCols);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        for(int iIndex=0;iIndex <= m_lEditorCols; iIndex++)
        {
            if((iIndex != iOffsetTestInfo) && (m_pColumnType[iIndex] == iCellType))
            {
                m_pColumnType[iIndex] = GEX_TB_TEST_ID_DATA;	// This line/column is of 'Data' type
                if(iIndex > iMaxLabels)
                    continue;
                pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iIndex);
                // m_bTestInLine == true if test info in lines // horizontal header
                // if test info in lines, part info in column
                if(!pQtwiHeaderItem)
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iIndex, pQtwiHeaderItem);
                }
                pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_DATA));
            }
        }

        // Check that no other line is using the same label type...if it does, switch it to 'Parameter'
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorRows);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        for(int iIndex=0;iIndex <= m_lEditorRows; iIndex++)
        {
            if((iIndex != iOffsetPartInfo) && (m_pLineType[iIndex] == iCellType))
            {
                m_pLineType[iIndex] = GEX_TB_PART_ID_PARAMETER;	// This line/column is of 'Parameter' type
                strText = "L" + QString::number(iIndex+m_lStartingRow+1) + ": Parameter-" + QString::number(iIndex+1);
                if(iIndex > iMaxLabels)
                    continue;

                pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iIndex);
                // m_bTestInLine == true if test info in lines // horizontal header
                // if test info in lines, part info in column
                if(!pQtwiHeaderItem)
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iIndex, pQtwiHeaderItem);
                }
                pQtwiHeaderItem->setText(strText);
            }
        }
    }
    else
    {
        // Check that no other column is using the same label type...if it does, switch it to 'Data'
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorCols);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        for(int iIndex=0;iIndex <= m_lEditorCols; iIndex++)
        {
            if((iIndex != iOffsetPartInfo) && (m_pColumnType[iIndex] == iCellType))
            {
                m_pColumnType[iIndex] = GEX_TB_PART_ID_PARAMETER;	// This line/column is of 'Parameter' type
                if(iIndex > iMaxLabels)
                    continue;
                strText ="Parameter-" + QString::number(iIndex+1);

                pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iIndex);
                // m_bTestInLine == true if test info in lines // horizontal header
                // if test info in lines, part info in column
                if(!pQtwiHeaderItem)
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iIndex, pQtwiHeaderItem);
                }
                pQtwiHeaderItem->setText(strText);
            }
        }

        // Check that no other line is using the same label type...if it does, switch it to 'Data'
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorRows);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        for(int iIndex=0;iIndex <= m_lEditorRows; iIndex++)
        {
            if((iIndex != iOffsetTestInfo) && (m_pLineType[iIndex] == iCellType))
            {
                m_pLineType[iIndex] = GEX_TB_TEST_ID_DATA;	// This line/column is of 'Data' type
                if(iIndex > iMaxLabels)
                    continue;

                pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iIndex);
                // m_bTestInLine == true if test info in lines // horizontal header
                // if test info in lines, part info in column
                if(!pQtwiHeaderItem)
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iIndex, pQtwiHeaderItem);
                }
                pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_DATA));
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Updates Table header labels...detect type of data
//////////////////////////////////////////////////////////////////////
QString ImportCsvWizardDialog::OnUpdateTableLabels(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("ColumnType = %1").arg( m_pColumnType?"ok":"NULL").toLatin1().constData());
    // Try to identify what each column is....
    QString strText;

    // If no file loaded yet...simply return!
    if(m_pColumnType == NULL)
        return "ok";

    // Disable controls during processing
    setEnabledInteractiveControls(false);

    // Handles to headers.
    QTableWidgetItem	*pQtwiHeaderItem;

    int	iIndex;
    int	iMaxLabels;
    if(radioButtonTestInLine->isChecked())
    {
        // One Parameter per line.
        m_bTestInLine = true;

        // Assume 1st Col is Parameter labels (one parameter per line)
        pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(0);
        // m_bTestInLine == true if test info in lines // horizontal header
        // if test info in lines, part info in column
        if(!pQtwiHeaderItem)
        {
            try
            {
                pQtwiHeaderItem = new QTableWidgetItem;
            }
            catch(const std::bad_alloc& e)
            {
                GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
                return "error : Memory allocation exception caught";
            }
            catch(...)
            {
                GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
                return "error : Exception caught";
            }

            mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(0, pQtwiHeaderItem);
        }
        pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_LABEL));

        m_lLabelOffset = 0;
        m_pColumnType[m_lLabelOffset] = GEX_TB_TEST_ID_LABEL;

        // Each line includes one parameter
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorRows);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        GSLOG(SYSLOG_SEV_DEBUG, QString("updating %1 labels...").arg( iMaxLabels).toLatin1().constData());
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Updating %1 labels...").arg(iMaxLabels) );
//        qApp->processEvents();
        for(iIndex=0;iIndex <= iMaxLabels; iIndex++)
        {
            strText = "L" + QString::number(iIndex+m_lStartingRow+1) + ": Parameter-" + QString::number(iIndex+1);

            pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iIndex);
            // m_bTestInLine == true if test info in lines // horizontal header
            // if test info in lines, part info in column
            if(!pQtwiHeaderItem)
            {
                try
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                }
                catch(const std::bad_alloc& e)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
                    return "error : Memory allocation exception caught ";
                }
                catch(...)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
                    return "error : Exception caught ";
                }
                mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iIndex, pQtwiHeaderItem);
            }
            pQtwiHeaderItem->setText(strText);

            m_pLineType[iIndex] = GEX_TB_PART_ID_PARAMETER;
        }

        // Label all following Columns to 'Data'
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorCols);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        GSLOG(SYSLOG_SEV_DEBUG, QString("updating %1 labels...").arg( iMaxLabels).toLatin1().constData());
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Updating %1 labels...").arg(iMaxLabels) );
//        qApp->processEvents();
        for(iIndex=1;iIndex <= iMaxLabels; iIndex++)
        {
            pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iIndex);
            // m_bTestInLine == true if test info in lines // horizontal header
            // if test info in lines, part info in column
            if(!pQtwiHeaderItem)
            {
                try
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                }
                catch(const std::bad_alloc& e)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
                    return "error : Memory allocation exception caught ";
                }
                catch(...)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
                    return "error : Exception caught ";
                }

                mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iIndex, pQtwiHeaderItem);
            }
            pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_DATA));

            m_pColumnType[iIndex] = GEX_TB_TEST_ID_DATA;
        }
    }
    else
    {
        // One Parameter per Column.
        m_bTestInLine = false;

        // Assume 1st Line is Parameter labels (one parameter per column)
        pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(0);
        // m_bTestInLine == true if test info in lines // horizontal header
        // if test info in lines, part info in column
        if(!pQtwiHeaderItem)
        {
            pQtwiHeaderItem = new QTableWidgetItem;
            mUi_pQtwExcelTableWidget->setVerticalHeaderItem(0, pQtwiHeaderItem);
        }
        pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_LABEL));

        m_lLabelOffset = 0;
        m_pLineType[m_lLabelOffset] = GEX_TB_TEST_ID_LABEL;

        // Each column includes one parameter
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorCols);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        GSLOG(SYSLOG_SEV_DEBUG, QString("updating %1 labels...").arg( iMaxLabels).toLatin1().constData());
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Updating %1 labels...").arg(iMaxLabels) );
//		qApp->processEvents();
        for(iIndex=0;iIndex <= iMaxLabels; iIndex++)
        {
            strText = "Parameter-" + QString::number(iIndex+1);

            pQtwiHeaderItem = mUi_pQtwExcelTableWidget->horizontalHeaderItem(iIndex);
            // m_bTestInLine == true if test info in lines // horizontal header
            // if test info in lines, part info in column
            if(!pQtwiHeaderItem)
            {
                pQtwiHeaderItem = new QTableWidgetItem;
                mUi_pQtwExcelTableWidget->setHorizontalHeaderItem(iIndex, pQtwiHeaderItem);
            }
            pQtwiHeaderItem->setText(strText);

            m_pColumnType[iIndex] = GEX_TB_PART_ID_PARAMETER;
        }

        // Label all following Lines  to 'Data'
        iMaxLabels = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorRows);	// Set maximum of GEX_TB_MAXLINES_PREVIEW labels (because this function is VERY CPU consuming)
        GSLOG(SYSLOG_SEV_DEBUG, QString("updating %1 labels...").arg( iMaxLabels).toLatin1().constData());
        for(iIndex=1;iIndex <= iMaxLabels; iIndex++)
        {
            pQtwiHeaderItem = mUi_pQtwExcelTableWidget->verticalHeaderItem(iIndex);
            // m_bTestInLine == true if test info in lines // horizontal header
            // if test info in lines, part info in column
            if(!pQtwiHeaderItem)
            {
                try
                {
                    pQtwiHeaderItem = new QTableWidgetItem;
                }
                catch(const std::bad_alloc &e)
                {
                    GSLOG(SYSLOG_SEV_CRITICAL, "bad_alloc exception caught");
                    return "error : bad_alloc exception caught";
                }
                catch(...)
                {
                    GSLOG(SYSLOG_SEV_CRITICAL, "exception caught");
                    return "error : exception caught";
                }

                mUi_pQtwExcelTableWidget->setVerticalHeaderItem(iIndex, pQtwiHeaderItem);
            }
            pQtwiHeaderItem->setText(tr(GEX_TB_TEST_LABEL_DATA));

            m_pLineType[iIndex] = GEX_TB_TEST_ID_DATA;
        }
    }

    // Force Table to be redrawn...
    QString r=repaintItems();
    // Enable controls during processing
    setEnabledInteractiveControls(true);
    return r;
}

//////////////////////////////////////////////////////////////////////
// Clear variables
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::clear(void)
{
    // Clear variables to preset values.
    m_strLotId = "Lot - Quantix";
    m_strSubLotId = "";
    m_strPreviousSubLotId = "";
    m_strOperator = "www.mentor.com";
    m_strTester = "Quantix Examinator";
    m_strTestProgram = "";
}

//////////////////////////////////////////////////////////////////////
// Fill table with data file, using current delimiter selctions for parsing
//////////////////////////////////////////////////////////////////////
QString ImportCsvWizardDialog::LoadFileInTable()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return "error: This function is disabled in teradyne mode";
    }

    // Clear some variables
    clear();

    // Make sure no cell is selected.
    QList<QTableWidgetSelectionRange> qlSelectionRangeList = mUi_pQtwExcelTableWidget->selectedRanges();
    for(int ii=0; ii<qlSelectionRangeList.count(); ii++)
        mUi_pQtwExcelTableWidget->setRangeSelected(qlSelectionRangeList.at(ii), false);

    // Load CSV file  into the Table.
    QStringList strCells;
    QString strString;

    mUi_pQtwExcelTableWidget->setColumnCount(m_lEditorCols);
    mUi_pQtwExcelTableWidget->setRowCount(m_lEditorRows);

    // Open CSV file
    QFile f( m_strCsvFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        GS::Gex::Message::information("", "Failed reading data file\n"
                                      "Disk full, or access violation?");
        return "error : failed reading data file\nDisk full, or access violation";
    }

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building parameter list...");

    // Assign file I/O stream
    QTextStream hCsvFile(&f);
    long	lFileLineNumber=0;
    long	lTableLineNumber=0;
    long	lCount;
    int		iIndex;
    QTableWidgetItem *pCtwiTableWidgetItem;
    QString strSplitList="[";

    // Get lines to skip
    m_lStartingRow = spinBoxStartImportRow->value()-1;

    // Get lines to col
    m_lStartingCol = spinBoxStartImportCol->value()-1;

    // Define string split char list
    if(checkBoxSpaceDelimiter->isChecked())
        strSplitList += "\\s";

    if(checkBoxTabDelimiter->isChecked())
        strSplitList += "\\t";

    if(checkBoxSemicolonDelimiter->isChecked())
        strSplitList += ";";

    if(checkBoxCommaDelimiter->isChecked())
        strSplitList += ",";

    if(checkBoxOtherDelimiter->isChecked())
        strSplitList += lineEditOtherDelimiter->text();

    strSplitList += "]";
    m_rSplit.setPattern(strSplitList);

    do
    {
        // Extract all lines, and load them into the Table.
        strString = ReadLine(hCsvFile);
        if(strString.isEmpty())
            continue;

        lFileLineNumber++;

        // After 100 lines, simply keep readnig lines to see how many lines are in file.
        if(lFileLineNumber - m_lStartingRow > GEX_TB_MAXLINES_PREVIEW)
            lTableLineNumber++;
        else
        // Only import lines AFTER the specified 'Import from row'
        if(lFileLineNumber <= m_lStartingRow)
        {
            // Header section to skip: parse it just to see if LotID, operator info can be found!
            strCells = strString.split(m_rSplit, QString::KeepEmptyParts);
            lCount = strCells.count();
            if(lCount >= 2)
            {
                if(strCells[0].startsWith("Lot", Qt::CaseInsensitive))
                {
                    //  Extract LotID info
                    m_strLotId = strCells[1].trimmed();
                }
                else
                if(strCells[0].startsWith("SubLot", Qt::CaseInsensitive) || strCells[0].startsWith("Sub-Lot", Qt::CaseInsensitive) || strCells[0].startsWith("Sub Lot", Qt::CaseInsensitive))
                {
                    //  Extract SubLotID info
                    m_strSubLotId = strCells[1].trimmed();
                }
                else
                if(strCells[0].startsWith("Operator", Qt::CaseInsensitive))
                {
                    //  Extract Operator info
                    m_strOperator = strCells[1].trimmed();
                }
                else
                if(strCells[0].startsWith("Computer", Qt::CaseInsensitive) || strCells[0].startsWith("Tester", Qt::CaseInsensitive))
                {
                    //  Extract Tester info
                    m_strTester = strCells[1].trimmed();
                }
                else
                if(strCells[0].indexOf("Program",0,Qt::CaseInsensitive) >= 0)
                {
                    //  Extract Test program name info
                    m_strTestProgram = strCells[1].trimmed();
                }
            }
        }
        else
        {
            if(lTableLineNumber >= m_lEditorRows)
            {
                // Need to resize table...add 100 more lines!
                m_lEditorRows += 100;
                mUi_pQtwExcelTableWidget->setRowCount(m_lEditorRows);
            }

            // Extract the N column names
            strCells = strString.split(m_rSplit, QString::KeepEmptyParts);
            lCount = strCells.count();

            // If need be resize the columns count
            if(m_lEditorCols < lCount )
            {
                m_lEditorCols = lCount;
                mUi_pQtwExcelTableWidget->setColumnCount(m_lEditorCols);
            }

            // Get the first column
            QStringList::iterator it = strCells.begin();

            // Skip the N first column if need be
            for (iIndex = 0; iIndex < m_lStartingCol && it != strCells.end(); iIndex++)
                it++;

            iIndex = 0;
            for(; it != strCells.end(); it++)
            {
                pCtwiTableWidgetItem = mUi_pQtwExcelTableWidget->item(lTableLineNumber, iIndex);

                if(!pCtwiTableWidgetItem)
                {
                    pCtwiTableWidgetItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setItem(lTableLineNumber, iIndex, pCtwiTableWidgetItem);
                }
                pCtwiTableWidgetItem->setText( (*it).trimmed() );

                iIndex++;
            }
            // Fill leading empty cells with empty strings: makes sure we overwrite any previous data!
            while(iIndex < m_lEditorCols)
            {
                iIndex++;

                pCtwiTableWidgetItem = mUi_pQtwExcelTableWidget->item(lTableLineNumber, iIndex);

                if(!pCtwiTableWidgetItem)
                {
                    pCtwiTableWidgetItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setItem(lTableLineNumber, iIndex, pCtwiTableWidgetItem);
                }
                pCtwiTableWidgetItem->setText( QString("") );
            }

            // Update table line#
            lTableLineNumber++;
        }
    }
    while(hCsvFile.atEnd() == false);

    // Close file
    f.close();

    // Set the total exact number of lines to display
    m_lEditorRows = lTableLineNumber;	// Total lines (Rows) in the editor table
    mUi_pQtwExcelTableWidget->setRowCount(m_lEditorRows);

    // Initialize Header flags
    if(m_pColumnType != NULL)
        delete m_pColumnType;
    m_pColumnType=0;

    if(m_pLineType != NULL)
        delete m_pLineType;
    m_pLineType=0;

    // Allocate buffers
    try
    {
        m_pColumnType = new BYTE[m_lEditorCols+1];
        m_pLineType = new BYTE[m_lEditorRows+1];
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        return "error : Memory allocation exception caught ";
    }
    catch(...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        return "error : Exception caught ";
    }

    // Clear buffers
    for(iIndex=0;iIndex <= m_lEditorRows; iIndex++)
    {
        if (m_bTestInLine)
            m_pLineType[iIndex] = GEX_TB_PART_ID_PARAMETER;
        else
            m_pLineType[iIndex] = GEX_TB_TEST_ID_DATA;
    }
    for(iIndex=0;iIndex <= m_lEditorCols; iIndex++)
    {
        if (m_bTestInLine)
            m_pColumnType[iIndex] = GEX_TB_TEST_ID_DATA;
        else
            m_pColumnType[iIndex] = GEX_TB_PART_ID_PARAMETER;
    }

    // Initialize Header strings.
    QString r=OnUpdateTableLabels();
    if (r.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("OnUpdateTableLabels failed : %1").arg(r).toLatin1().data());
        GS::Gex::Message::information(
            "", QString("Tables labels update failed :\n%1").arg(r));
        return QString("error : OnUpdateTableLabels failed : %1").arg(r);
    }

    GSLOG(SYSLOG_SEV_DEBUG, "ok");
    return "ok";
}

//////////////////////////////////////////////////////////////////////
// Read Parser profile from disk, and apply to file
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::OnLoadParserProfile()
{
    // Load Parsing details from template file
    m_strProfileTemplateFile = QFileDialog::getOpenFileName(this, "Load Parsing Template", m_strProfileTemplateFile, "Parsing template (*.csl)");

    // If no file selected, quiet return
    if(m_strProfileTemplateFile.isEmpty())
        return;

    // Load parsing file into memory
    loadParserProfile();
}

//////////////////////////////////////////////////////////////////////
// Read Parser profile from disk, and apply to file
//////////////////////////////////////////////////////////////////////
bool ImportCsvWizardDialog::loadParserProfile(void)
{
    // Read Parser template
    QFile file( m_strProfileTemplateFile);
    if (file.open(QIODevice::ReadOnly) == false)
        return false;	// Failed opening Tasks file.

    // Read Tasks definition File
    QTextStream hFile;
    hFile.setDevice(&file);	// Assign file handle to data stream

    // Check if valid header...or empty!
    QString strString;

    strString= hFile.readLine();
    if(strString != "<gex_template>")
        return false;
    strString = hFile.readLine();
    if(strString != "<BlockType = parser_rules>")
        return false;

    setEnabledInteractiveControls(false);

    bool	bTestInLine = true;
    int		iCellOffset=0;
    int		iCellType=0;
    do
    {
        // Read one line from file
        strString = hFile.readLine();

        // Total first lines to skip
        if(strString.startsWith("skip_line") == true)
        {
            strString = strString.section('=',1);
            spinBoxStartImportRow->setValue(1+strString.toInt());
        }

        // Total first columns to skip
        if(strString.startsWith("skip_column") == true)
        {
            strString = strString.section('=',1);
            spinBoxStartImportCol->setValue(1+strString.toInt());
        }

        // Space delimiter active?
        if(strString.startsWith("space_delimiter") == true)
        {
            if(strString.section('=',1) == "1")
                checkBoxSpaceDelimiter->setChecked(true);
            else
                checkBoxSpaceDelimiter->setChecked(false);
        }

        // Tab delimiter active?
        if(strString.startsWith("tab_delimiter") == true)
        {
            if(strString.section('=',1) == "1")
                checkBoxTabDelimiter->setChecked(true);
            else
                checkBoxTabDelimiter->setChecked(false);
        }

        // Semi-colon delimiter active?
        if(strString.startsWith("semicolon_delimiter") == true)
        {
            if(strString.section('=',1) == "1")
                checkBoxSemicolonDelimiter->setChecked(true);
            else
                checkBoxSemicolonDelimiter->setChecked(false);
        }

        // Comma delimiter active?
        if(strString.startsWith("comma_delimiter") == true)
        {
            if(strString.section('=',1) == "1")
                checkBoxCommaDelimiter->setChecked(true);
            else
                checkBoxCommaDelimiter->setChecked(false);
        }

        // Custom/Other delimiter active?
        if(strString.startsWith("other_delimiter") == true)
        {
            checkBoxOtherDelimiter->setChecked(true);
            lineEditOtherDelimiter->setEnabled(true);
            // Get character to the right of the equal sign
            strString = strString.section('=',1);
            lineEditOtherDelimiter->setText(strString);
        }

        // Parameter is in line or column?
        if(strString.startsWith("parameter_direction") == true)
        {
            if(strString.section('=',1) == "line")
            {
                radioButtonTestInLine->setChecked(true);
                bTestInLine = true;
            }
            else
            if(strString.section('=',1) == "col")
            {
                radioButtonTestInColumns->setChecked(true);
                bTestInLine = false;
            }
        }

        // Instructs to load file using parsing details just read earlier in the file (and follwing are line/column types)
        if(strString.startsWith("parse_data") == true)
            LoadFileInTable();

        // Read Line/column offset (MUST appear prior to a 'line_type' or 'col_type'
        if(strString.startsWith("cell_offset") == true)
        {
            strString = strString.section('=',1);
            iCellOffset = strString.toInt();

        }
        // Parameter offset
        if(strString.startsWith("cell_p_offset") == true)
        {
            strString = strString.section('=',1);
            iCellOffset = strString.toInt();

        }

        // Read Line/column type (label, data, units, etc...
        if(strString.startsWith("cell_type") == true)
        {
            iCellType = -1;

            strString = strString.section('=',1);
            if(strString.startsWith("label") == true)
                iCellType = GEX_TB_TEST_ID_LABEL;
            else
            if(strString.startsWith("test#") == true)
                iCellType = GEX_TB_TEST_ID_TEST;
            else
            if(strString.startsWith("units") == true)
                iCellType = GEX_TB_TEST_ID_UNITS;
            else
            if(strString.startsWith("low_l") == true)
                iCellType = GEX_TB_TEST_ID_LL;
            else
            if(strString.startsWith("high_l") == true)
                iCellType = GEX_TB_TEST_ID_HL;
            else
            if(strString.startsWith("data") == true)
                iCellType = GEX_TB_TEST_ID_DATA;
            else
            if(strString.startsWith("ignore") == true)
                iCellType = GEX_TB_TEST_ID_IGNORE;

            // Refresh GUI
            UpdateCellType( !bTestInLine ? iCellOffset : 0, bTestInLine ? iCellOffset : 0,iCellType);
        }

        // Read Line/column type: Soft bin, Hard bin, Die X/Y...
        if(strString.startsWith("cell_p_type") == true)
        {
            iCellType = -1;

            strString = strString.section('=',1);
            if(strString.startsWith("test_pf") == true)
                iCellType = GEX_TB_PART_ID_TESTPF;
            else
            if(strString.startsWith("soft_bin") == true)
                iCellType = GEX_TB_PART_ID_SBIN;
            else
            if(strString.startsWith("hard_bin") == true)
                iCellType = GEX_TB_PART_ID_HBIN;
            else
            if(strString.startsWith("die_x") == true)
                iCellType = GEX_TB_PART_ID_DIEX;
            else
            if(strString.startsWith("die_y") == true)
                iCellType = GEX_TB_PART_ID_DIEY;
            else
            if(strString.startsWith("sublot_id") == true)
                iCellType = GEX_TB_PART_ID_SBLOT;
            else
            if(strString.startsWith("site") == true)
                iCellType = GEX_TB_PART_ID_SITE;
            else
            if(strString.startsWith("ignore") == true)
                iCellType = GEX_TB_PART_ID_IGNORE;
            else
            if(strString.startsWith("parameter") == true)
                iCellType = GEX_TB_PART_ID_PARAMETER;

            // Refresh GUI
            UpdateCellType( bTestInLine ? iCellOffset : 0, !bTestInLine ? iCellOffset : 0,iCellType);
        }
    }
    while(hFile.atEnd() == false);
    repaintItems();
    setEnabledInteractiveControls(true);

    file.close();

    // Display check box to allow all future files to use this paring file as well
    checkBoxParseAllFiles->show();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Save Parser profile to disk
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::OnSaveParserProfile()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }


    // Save Parsing details into tempate file
    m_strProfileTemplateFile = QFileDialog::getSaveFileName(this,
        "Save Parsing details to Template...",
         m_strProfileTemplateFile,
        "Parsing template (*.csl)",
        NULL,
        QFileDialog::DontConfirmOverwrite);

    // If no file selected, quiet return
    if(m_strProfileTemplateFile.isEmpty())
        return;

    // Make sure file name ends with ".csl" extension
    if(m_strProfileTemplateFile.endsWith(".csl",Qt::CaseInsensitive) == false)
        m_strProfileTemplateFile += ".csl";

    // Check if file exists.
    QFile file( m_strProfileTemplateFile);
    if(file.exists() == true)
    {
        // File exists...overwrite it?
        bool lOk;
        GS::Gex::Message::request("", "File already exists. Overwrite it ?", lOk);
        if (! lOk)
        {
            return;
        }
    }

    // Build path to the 'Tasks' list.
    if (file.open(QIODevice::WriteOnly) == false)
    {
        GS::Gex::Message::
            critical("", "Failed creating file...folder is write protected?");
        return;
    }

    setEnabledInteractiveControls(false);

    // Write Tasks definition File
    QTextStream hFile;
    hFile.setDevice(&file);	// Assign file handle to data stream

    // Write file header
    hFile << "<gex_template>" << endl;
    hFile << "<BlockType = parser_rules>" << endl << endl;

    // Get lines to skip
    hFile << "skip_line =" << spinBoxStartImportRow->value()-1 << endl;

    // Get columns to skip
    hFile << "skip_column =" << spinBoxStartImportCol->value()-1 << endl;

    // Define string split char list
    if(checkBoxSpaceDelimiter->isChecked())
        hFile << "space_delimiter=1"<< endl;
    else
        hFile << "space_delimiter=0"<< endl;

    if(checkBoxTabDelimiter->isChecked())
        hFile << "tab_delimiter=1"<< endl;
    else
        hFile << "tab_delimiter=0"<< endl;

    if(checkBoxSemicolonDelimiter->isChecked())
        hFile << "semicolon_delimiter=1"<< endl;
    else
        hFile << "semicolon_delimiter=0"<< endl;

    if(checkBoxCommaDelimiter->isChecked())
        hFile << "comma_delimiter=1"<< endl;
    else
        hFile << "comma_delimiter=0"<< endl;

    if(checkBoxOtherDelimiter->isChecked())
        hFile << "other_delimiter=" << lineEditOtherDelimiter->text() << endl;

    // Specify if parameters are in Line or Column (Row)
    if(radioButtonTestInLine->isChecked())
        hFile << "parameter_direction=line" << endl;
    else
        hFile << "parameter_direction=col" << endl;

    // Instruct to parse file based on above parsing details
    hFile << "parse_data=now" << endl;

    // Dump Line/column type: Low / High Limit, Data, Ignore, Units, etc...
    int		iHighCellTestInfo,iHighCellPartInfo;
    BYTE	*pFlagTestInfo,*pFlagPartInfo;
    if(m_bTestInLine)
    {
        iHighCellTestInfo = m_lEditorCols;
        iHighCellPartInfo = m_lEditorRows;
        pFlagTestInfo = m_pColumnType;
        pFlagPartInfo = m_pLineType;
    }
    else
    {
        iHighCellTestInfo = m_lEditorRows;
        iHighCellPartInfo = m_lEditorCols;
        pFlagTestInfo = m_pLineType;
        pFlagPartInfo = m_pColumnType;
    }

    int iIndex;
    for(iIndex=0;iIndex <= iHighCellTestInfo; iIndex++)
    {
        hFile << "cell_offset=" << iIndex << endl;
        hFile << "cell_type=";

        switch(pFlagTestInfo[iIndex])
        {
            case GEX_TB_TEST_ID_LABEL:
                hFile << "label" << endl;
                break;
            case GEX_TB_TEST_ID_TEST:
                hFile << "test#" << endl;
                break;
            case GEX_TB_TEST_ID_UNITS:
                hFile << "units" << endl;
                break;
            case GEX_TB_TEST_ID_LL:
                hFile << "low_l" << endl;
                break;
            case GEX_TB_TEST_ID_HL:
                hFile << "high_l" << endl;
                break;
            case GEX_TB_TEST_ID_DATA:
                hFile << "data" << endl;
                break;
            case GEX_TB_TEST_ID_IGNORE:
                hFile << "ignore" << endl;
                break;
            default:
                hFile << endl;
                break;
        }
    }

    for(iIndex=0;iIndex <= iHighCellPartInfo; iIndex++)
    {
        hFile << "cell_p_offset=" << iIndex << endl;
        hFile << "cell_p_type=";

        switch(pFlagPartInfo[iIndex])
        {
            case GEX_TB_PART_ID_TESTPF:
                hFile << "test_pf" << endl;
                break;
            case GEX_TB_PART_ID_SBIN:
                hFile << "soft_bin" << endl;
                break;
            case GEX_TB_PART_ID_HBIN:
                hFile << "hard_bin" << endl;
                break;
            case GEX_TB_PART_ID_DIEX:
                hFile << "die_x" << endl;
                break;
            case GEX_TB_PART_ID_DIEY:
                hFile << "die_y" << endl;
                break;
            case GEX_TB_PART_ID_SBLOT:
                hFile << "sublot_id" << endl;
                break;
            case GEX_TB_PART_ID_SITE:
                hFile << "site" << endl;
                break;
            case GEX_TB_PART_ID_IGNORE:
                hFile << "ignore" << endl;
                break;
            case GEX_TB_PART_ID_PARAMETER:
                hFile << "parameter" << endl;
                break;
            default:
                hFile << endl;
                break;
        }
    }
    setEnabledInteractiveControls(true);
    // End of file.
    hFile << endl << "</gex_template>" << endl;
    file.close();

    // Display check box to allow all futre files to use this paring file as well
    checkBoxParseAllFiles->show();
}

//////////////////////////////////////////////////////////////////////
// Signal, user clicked 'Next' button to parse file
//////////////////////////////////////////////////////////////////////
void ImportCsvWizardDialog::OnNext()
{
    int lMaxCols = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorCols);
    int lMaxRows = gex_min(GEX_TB_MAXLINES_PREVIEW,m_lEditorRows);
    // set buffer type
    if (m_bTestInLine)
    {
        for(int iIndex=lMaxRows;iIndex <= m_lEditorRows; iIndex++)
            m_pLineType[iIndex] = GEX_TB_PART_ID_PARAMETER;
    }
    else
    {
        for(int iIndex=lMaxCols;iIndex <= m_lEditorCols; iIndex++)
            m_pColumnType[iIndex] = GEX_TB_PART_ID_PARAMETER;
    }
    ParseFile();
}

//////////////////////////////////////////////////////////////////////
// Start import process
//////////////////////////////////////////////////////////////////////
bool ImportCsvWizardDialog::ParseFile()
{
    QTableWidgetItem	*pCtwiTableWidgetItem;

    // hide widget during parsing to solve a performance problem with repaintitems() method.
    hide();

    // Check if we have use a profile
    if(bApplyProfileToAllFiles == false)
    {
        bApplyProfileToAllFiles = checkBoxParseAllFiles->isChecked();
    }

    // Delete current table content.
    mUi_pQtwExcelTableWidget->clear();

    // Reset size...
    m_lEditorRows=1;	// At startup, allocates 10 lines.
    m_lEditorCols=1;	// At startup, allocates 10 columns.
    mUi_pQtwExcelTableWidget->setRowCount(m_lEditorRows);
    mUi_pQtwExcelTableWidget->setColumnCount(m_lEditorCols);

    // Open CSV file
    QFile f( m_strCsvFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        GS::Gex::Message::information(
            "", "Failed reading data file\nDisk full, or access violation?");
        return false;
    }

    // Assign file I/O stream
    QTextStream hCsvFile(&f);
    long	lFileLineNumber=0;
    long	lTableLineNumber=0;
    QStringList strCells;
    QString strString;
    long	lCount;

    do
    {
        // Extract all lines, and load them into the Table.
        strString = ReadLine(hCsvFile);
        if(strString.isEmpty())
            continue;

        lFileLineNumber++;

        // Only import lines AFTER the specified 'Import from row'
        if(lFileLineNumber > m_lStartingRow)
        {
            if(lTableLineNumber >= m_lEditorRows)
            {
                // Need to resize table...add 1000 more lines!
                m_lEditorRows += 1000;
                mUi_pQtwExcelTableWidget->setRowCount(m_lEditorRows);
            }

            // Extract the N column names
            strCells = strString.split(m_rSplit, QString::KeepEmptyParts);
            lCount = strCells.count();

            // If need be resize the columns count
            if(m_lEditorCols < lCount )
            {
                m_lEditorCols = lCount;
                mUi_pQtwExcelTableWidget->setColumnCount(m_lEditorCols);
            }

            // copy csv read line in table widget cells
            pCtwiTableWidgetItem = NULL;
            for(int nColIterator=0; nColIterator < m_lEditorCols; nColIterator++)
            {
                pCtwiTableWidgetItem = mUi_pQtwExcelTableWidget->item(lTableLineNumber, nColIterator);

                if(!pCtwiTableWidgetItem)
                {
                    pCtwiTableWidgetItem = new QTableWidgetItem;
                    mUi_pQtwExcelTableWidget->setItem(lTableLineNumber, nColIterator, pCtwiTableWidgetItem);
                }

                if(nColIterator < lCount)
                    pCtwiTableWidgetItem->setText(strCells.at(nColIterator));
                else
                    pCtwiTableWidgetItem->setText(QString(""));	// Fill leading empty cells with empty strings: makes sure we overwrite any previous data!
            }

            // Update table line#
            lTableLineNumber++;
        }
    }
    while(hCsvFile.atEnd() == false);

    // Close file
    f.close();

    // Erase Test limits lists
    map_lfLowLimit.clear();
    map_lfHighLimit.clear();

    // Set the total exact number of lines to display
    m_lEditorRows = lTableLineNumber;	// Total lines (Rows) in the editor table
    mUi_pQtwExcelTableWidget->setRowCount(m_lEditorRows);

    // Now, we're ready to dump the table into the STDF file...
    if(WriteStdfFile() == true)
    {
        // Success creating STDF file...check if need to update the CSV Parameter list on disk?
        if(bNewCsvParameterFound == true)
            DumpParameterIndexTable();

        done(1);
        return true;
    }
    else
    {
        // Failed dumping CSV data into STDF file
        done(0);
        return false;
    }
}


//////////////////////////////////////////////////////////////////////
// Write STDF PTR record for a given Parameter & Run
//////////////////////////////////////////////////////////////////////
int	ImportCsvWizardDialog::WritePTR(long m_lPartID,QString strValue,QString strLabel,QString strUnits,QString strLowL,QString strHighL, bool bNeedPir)
{
    int     iTestStatus;
    float	fLowL,fHighL,fScaling=1.0F;
    BYTE	uRes_scale=0;

    // On first run, save Label + test limits & compute scale factor if any...
    if(m_lPartID == 0)
    {
        UpdateParameterIndexTable(strLabel);		// Update Parameter master list if needed.

        // Check for scaling results...
        fScaling = 1.0f;
        uRes_scale = 0;

        if(strUnits.length() > 1)
        {
            char cPrefix = strUnits.at(0).toLatin1();
            switch(cPrefix)
            {
                // List of units we accept to scale to m, u, p, f, etc...
                case 'm':	// Milli...
                    fScaling = 1e-3f;
                    uRes_scale = 3;
                    break;

                case 'u':	// Micro...
                    fScaling = 1e-6f;
                    uRes_scale = 6;
                    break;

                case 'n':	// Milli...
                    fScaling = 1e-9f;
                    uRes_scale = 9;
                    break;

                case 'p':	// Pico...
                    fScaling = 1e-12f;
                    uRes_scale = 12;
                    break;

                case 'f':	// Fento...
                    fScaling = 1e-15f;
                    uRes_scale = 15;
                    break;

                case 'K':	// Kilo...
                    fScaling = 1e3f;
                    uRes_scale = 253;	// -3 in unsigned notation
                    break;

                case 'M':	// Mega...
                    fScaling = 1e6f;
                    uRes_scale = 250;	// -6 in unsigned notation
                    break;

                case 'G':	// Giga...
                    fScaling = 1e9f;
                    uRes_scale = 247;	// -9 in unsigned notation
                    break;

                case 'T':	// Tera...
                    fScaling = 1e12f;
                    uRes_scale = 244;	// -12 in unsigned notation
                    break;
                default:
                    fScaling = 1.0f;
                    uRes_scale = 0;
            }
            // If a scaling is detected, remove prefix from Units string
            if(uRes_scale != 0)
                strUnits = strUnits.mid(1);
        }
    }

    // Read test value.
    float fTestResult;
    char *ptChar = (char *) strValue.toLatin1().data();
    if(ptChar != NULL)
        iTestStatus = sscanf(ptChar,"%f",&fTestResult);
    else
        iTestStatus = 0;	//  Empty field!

    // Check if successfuly read value.
    if(iTestStatus <= 0)
    {
        m_bData = 2;		// Test result Invalid
        // No data available...but if First run, we'll have to write the PTR for test limits info (if any)
        if(m_lPartID > 0)
            return 0;
    }
    else{
        m_bData = 0;		// Test valid
    }

    //PIR can be written
    if(!m_iTotalTests)
    {
        // Write PIR (on first valid test found)
        if(bNeedPir)
            WritePIR();
        bNeedPir = false;

        // Save name of first test in flow (so to detect when run ends)
        m_strFirstTestInFlow = "["+QString::number(m_lTestNumber)+"]"+strLabel;
    }
    else
    {
        // Check if we've reached the end of the flow (ie: we are about to process again the same test name)
        if(("["+QString::number(m_lTestNumber)+"]"+strLabel) == m_strFirstTestInFlow)
        {
            // Write PRR to end previous run
            if(m_iTotalTests)
                WritePRR(m_lTotalPartsInWafer);

            // Write PIR (marks beginning of new flow)
            WritePIR();
            bNeedPir = false;
            m_iTotalTests = 0;
            m_iTotalPassFailTests=0;
        }
    }

    RecordReadInfo.iRecordType = 15;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);

    // Compute Test# (add user-defined offset)
    if (m_lTestNumber == -1)
    {
        m_lTestNumber	= (long) pFullCsvParametersList.indexOf(strLabel);
        m_lTestNumber	+= GEX_TESTNBR_OFFSET_CSV;		// Test# offset
    }

    StdfFile.WriteDword(m_lTestNumber);	// Test Number
    StdfFile.WriteByte(1);				// Test head
    StdfFile.WriteByte(m_iSiteID);		// Tester site#
    StdfFile.WriteByte(m_bData);		// TEST_FLG 0=test valid & passed, 2 = invalid result
    StdfFile.WriteByte(0x40|0x80);		// PARAM_FLG (limits are not strict)

    // Scale result
    if(m_lPartID != 0)
        fScaling = map_lfScaling[m_lTestNumber];
    fTestResult *= fScaling;

    StdfFile.WriteFloat(fTestResult);	// Test result

    if(m_lPartID == 0)
    {
        bool	bFlag;
        if(strLabel.isEmpty())
        {
            // No test name...
            QString strName = "TEST " + QString::number(m_lTestNumber);
            StdfFile.WriteString(strName.toLatin1().constData());	// TEST_TXT
        }
        else
            StdfFile.WriteString(strLabel.toLatin1().constData());	// TEST_TXT
        StdfFile.WriteString("");							// ALARM_ID
        m_bData = 2;
        fLowL = strLowL.toFloat(&bFlag);
        if(bFlag == false)
        {
            m_bData |=0x40;	// Missing LowL
            fLowL = -C_INFINITE_FLOAT;
        }
        else
            fLowL *= fScaling;	// Scale units

        fHighL = strHighL.toFloat(&bFlag);
        if(bFlag == false)
        {
            m_bData |=0x80;	// Missing HighL
            fHighL = C_INFINITE_FLOAT;
        }
        else
            fHighL *= fScaling;	// Scale units

        StdfFile.WriteByte(m_bData);		// OPT_FLAG
        StdfFile.WriteByte(uRes_scale);		// RES_SCALE
        StdfFile.WriteByte(uRes_scale);		// LLM_SCALE
        StdfFile.WriteByte(uRes_scale);		// HLM_SCALE
        StdfFile.WriteFloat(fLowL);			// LOW Limit
        StdfFile.WriteFloat(fHighL);		// HIGH Limit

        // Units
        StdfFile.WriteString(strUnits.toLatin1().constData());

        // Save Test limits
        map_lfLowLimit[m_lTestNumber]	= fLowL;	// Save Test LOW limit (so we can check test result's validity for each run)
        map_lfHighLimit[m_lTestNumber]	= fHighL;	// Save Test HIGH limit (so we can check test result's validity for each run)
        map_lfScaling[m_lTestNumber]	= fScaling;
    }
    else
    {
        fLowL	= map_lfLowLimit[m_lTestNumber];
        fHighL	= map_lfHighLimit[m_lTestNumber];
    }


    // Check data validity...and flag if test failed.
    if((fTestResult < fLowL) | (fTestResult > fHighL))
        bFailedExecution = true;

    StdfFile.WriteRecord();

    // Return if valid datalog
    return 1;
}

//////////////////////////////////////////////////////////////////////
// Write STDF WIR record
//////////////////////////////////////////////////////////////////////
void	ImportCsvWizardDialog::WriteWIR(void)
{
    // Check if Wafermap mode active...
    if((m_lDieXOffset < 0) || (m_lDieYOffset < 0))
        return;

    // Write WIR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);			// Test head
    StdfFile.WriteByte(1);			// Tester site#
    StdfFile.WriteDword(time(NULL));// START_T
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());	// sublot-id
    StdfFile.WriteRecord();
}

//////////////////////////////////////////////////////////////////////
// Write STDF PIR record
//////////////////////////////////////////////////////////////////////
void	ImportCsvWizardDialog::WritePIR(void)
{
    // Write PIR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);			// Test head
    StdfFile.WriteByte(m_iSiteID);			// Tester site#
    StdfFile.WriteRecord();
}

//////////////////////////////////////////////////////////////////////
// Write STDF WRR record
//////////////////////////////////////////////////////////////////////
void	ImportCsvWizardDialog::WriteWRR(long &lTotalPartsInWafer)
{
    // Check if Wafermap mode active...
    if((m_lDieXOffset < 0) || (m_lDieYOffset < 0))
        return;

    // Write WRR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site#
    StdfFile.WriteDword(time(NULL));			// START_T
    StdfFile.WriteDword(lTotalPartsInWafer);	// TOTAL parts
    StdfFile.WriteDword((DWORD)(4294967295UL));	// RETEST
    StdfFile.WriteDword((DWORD)(4294967295UL));	// ABORT
    StdfFile.WriteDword((DWORD)(4294967295UL));	// GOOD
    StdfFile.WriteDword((DWORD)(4294967295UL));	// FUNCT
    StdfFile.WriteString(m_strPreviousSubLotId.toLatin1().constData());	// sublot-id
    StdfFile.WriteRecord();

    // Reset count.
    lTotalPartsInWafer = 0;
}

//////////////////////////////////////////////////////////////////////
// Write STDF PRR record
//////////////////////////////////////////////////////////////////////
void	ImportCsvWizardDialog::WritePRR(long &lTotalPartsInWafer)
{
    // Update number of valid parts seen
    m_lPartID++;

    // Update total parts in wafer (in case multple wafers in file)
    lTotalPartsInWafer++;

    // If no PTR written, then ignore PRR
    if(m_iTotalTests == 0)
    {
        bFailedExecution = false;
        return;
    }

    // Write PRR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);			// Test head
    StdfFile.WriteByte(m_iSiteID);	// Tester site#

    // PRR: PASS/FAIL info
    if(m_iTotalPassFailTests)
        StdfFile.WriteByte(8);			// PART_FLG : FAILING PART
    else
        StdfFile.WriteByte(0);			// PART_FLG : PASS PART

    // Check if Test Pass/Fail columns exist
    if(m_iTotalPassFailTests)
    {
        // Tests failed in the flow, overwrite Binning unless already defines!
        if(m_iSoftBin < 0 && m_iHardBin < 0)
            bFailedExecution = true;
    }

    // If only one of the bins not defined, force it to the other value
    if(m_iSoftBin < 0 && m_iHardBin >= 0)
        m_iSoftBin = m_iHardBin;
    else
    if(m_iHardBin < 0 &&  m_iSoftBin >= 0)
        m_iHardBin = m_iSoftBin;

    // Create a binning result
    if(bFailedExecution == true)
    {
        // This execution is a failing bin
        if(m_iSoftBin < 0)
            m_iSoftBin = 0;
        if(m_iHardBin < 0)
            m_iHardBin = 0;
        m_iTotalFailBin++;
    }
    else
    {
        // This execution is a passing bin
        if(m_iSoftBin < 0)
            m_iSoftBin = 1;
        if(m_iHardBin < 0)
            m_iHardBin = 1;
        m_iTotalGoodBin++;
    }

    StdfFile.WriteWord((WORD)m_iTotalTests);	// NUM_TEST
    StdfFile.WriteWord(m_iHardBin);				// HARD_BIN
    StdfFile.WriteWord(m_iSoftBin);				// SOFT_BIN
    StdfFile.WriteWord((WORD)m_iDieX);			// X_COORD
    StdfFile.WriteWord((WORD)m_iDieY);			// Y_COORD
    StdfFile.WriteDword(0);			// Testing time (if known, otherwise: 0)
    QString strString = "Part " + QString::number(m_lPartID);
    StdfFile.WriteString(strString.toLatin1().constData());	// PART_ID
    StdfFile.WriteString("");			// PART_TXT
    StdfFile.WriteString("");			// PART_FIX
    StdfFile.WriteRecord();

    // Reset flag
    bFailedExecution = false;
}

void ImportCsvWizardDialog::WriteMIR(void)
{
    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    long lStartTime = time(NULL);
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(lStartTime);			// Setup time
    StdfFile.WriteDword(lStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) 'E');				// Test Mode = ENGEENERING
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
    StdfFile.WriteString("");					// Part Type / Product ID
    StdfFile.WriteString(m_strTester.toLatin1().constData());	// Node name
    StdfFile.WriteString("");	// Tester Type
    StdfFile.WriteString(m_strTestProgram.toLatin1().constData());	// Job name
    StdfFile.WriteString("www.mentor.com");	// Job rev
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());	// sublot-id
    StdfFile.WriteString(m_strOperator.toLatin1().constData());	// operator
    StdfFile.WriteString(
      GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );		// exec-type
    StdfFile.WriteString(
      GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );		// exe-ver

    StdfFile.WriteRecord();
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data loaded into the Table.
//////////////////////////////////////////////////////////////////////
bool ImportCsvWizardDialog::WriteStdfFile(void)
{
    // now generate the STDF file...
    if(StdfFile.Open(m_strFileNameSTDF.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }


    long	lNotifyProgress=0;
    int		iStep;
    float	fTotalStep;

    int nRowCount = mUi_pQtwExcelTableWidget->rowCount();
    int nColumnCount = mUi_pQtwExcelTableWidget->columnCount();
    QTableWidgetItem *pQtwiItem = NULL;
    QString strItemtext;
    bool bConversionRslt = false;

    fTotalStep = nRowCount*nColumnCount;

    QProgressDialog progress( "Converting data file to STDF...", "Abort", 0, 100, this);
    progress.setModal(true);
    progress.setMinimumDuration(0);

    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        progress.show();

    progress.setValue(0);
    QCoreApplication::processEvents();

    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    StdfFile.SetStdfCpuType(StdfFile.GetComputerCpuType());

    // Write FAR (MIR written later, at first valid part & test found)
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(StdfFile.GetComputerCpuType());	// Force CPU type to current computer platform.
    StdfFile.WriteByte(4);					// STDF V4
    StdfFile.WriteRecord();

    // Write MIR
    WriteMIR();

    // Write Test results for each line (or column) read.
    QString strString;
    //QString strFirstTestInFlow;
    QString	strLabel,strUnits,strLowL,strHighL;

    // Reset counters
    bool	bIsNumber;
    bool	bNeedPir = true;
    m_iTotalGoodBin=m_iTotalFailBin=0;
    m_lPartID=0;
    bFailedExecution = false;
    m_lTotalPartsInWafer=0;

    // Dump all Parameters
    if(m_bTestInLine)
    {
        // One Test per line

        // Dump tests.
        for(int iCol=0 ;iCol < nColumnCount ;iCol++)
        {
            // Clear number of tests in run.
            m_iTotalTests = 0;
            m_iTotalPassFailTests = 0;

            // Extract global fields for this run
            if((iCol > GEX_TB_MAXLINES_PREVIEW) || (m_pColumnType[iCol] == GEX_TB_TEST_ID_DATA))
            {
                // Clear part info
                m_iSoftBin	= m_iHardBin = -1;
                m_iDieX		= m_iDieY = GEX_TB_UNDEFINED_DIE_LOC;
                m_iSiteID	= 255;

                // Get Bin#
                if(m_lSoftBinOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lSoftBinOffset, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iSoftBin = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to soft bin #... ").arg( strItemtext).toLatin1().constData());
                }
                if(m_lHardBinOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lHardBinOffset, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iHardBin = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to hard bin #... ").arg( strItemtext).toLatin1().constData());
                }

                // Get Die location
                if(m_lDieXOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lDieXOffset, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iDieX = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Die X... ").arg( strItemtext).toLatin1().constData());
                }
                if(m_lDieYOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lDieYOffset, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iDieY = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Die Y... ").arg( strItemtext).toLatin1().constData());
                }

                // Get site ID
                if (m_lSiteOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lSiteOffset, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iSiteID = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Site ID... ").arg( strItemtext).toLatin1().constData());
                }

                // The SublotID / WaferID is extracted from the table data
                if(m_lSublotOffset >= 0)
                {
                    m_strPreviousSubLotId = m_strSubLotId;

                    pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lSublotOffset, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_strSubLotId = strItemtext;

                    // Write WIR (if applicable & on first valid test found)
                    if((m_strPreviousSubLotId.isEmpty() == false) && (m_strPreviousSubLotId != m_strSubLotId))
                        WriteWRR(m_lTotalPartsInWafer);
                    if(m_strPreviousSubLotId != m_strSubLotId)
                    {
                        WriteWIR();
                        m_strPreviousSubLotId = m_strSubLotId;
                    }
                }
            }

            for(int iRow=0; iRow < nRowCount; iRow++)
            {
                // Update progress bar...
                if(lNotifyProgress & 64)
                {
                    // Notify Progress every 64 cells processed
                    iStep = (int) ((float)iCol*(float)nRowCount*100.0/fTotalStep);
                    progress.setValue(iStep);
                    QCoreApplication::processEvents();

                    if(progress.wasCanceled())
                        return true;
                }
                lNotifyProgress++;

                // Only Dump DATA columns! (Any data after col #100 are data, labels (etc) can only be defined in the first 100 cols!
                if((iCol <= GEX_TB_MAXLINES_PREVIEW) && (m_pColumnType[iCol] != GEX_TB_TEST_ID_DATA))
                    goto nextCol;

                // Check if this cell is a test 'P/F' status.
                if(m_pLineType[iRow] == GEX_TB_PART_ID_TESTPF)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    strString = strItemtext;

                    strString = strString.trimmed();
                    if(strString.startsWith("F", Qt::CaseInsensitive) || strString.startsWith("0", Qt::CaseInsensitive))
                        m_iTotalPassFailTests++;	// Keep track of total Fail tests in flow.
                }

                // Only Dump PARAMETER lines!
                if ((iRow > GEX_TB_MAXLINES_PREVIEW) || (m_pLineType[iRow] == GEX_TB_PART_ID_PARAMETER))
                {
                    // Cell value
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    strString = strItemtext;

                    strString = strString.trimmed();

                    if(m_lLabelOffset >= 0)
                    {
                        pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lLabelOffset);
                        if(pQtwiItem)
                            strItemtext = pQtwiItem->text();
                        else
                            strItemtext = QString("");
                        strLabel = strItemtext;

                        if(strLabel.isEmpty())
                            strLabel = "Parameter " + QString::number(iRow+1);
                    }
                    else
                        strLabel = "Parameter " + QString::number(iRow+1);
                    strLabel = strLabel.trimmed();

                    if(m_lUnitsOffset >= 0)
                    {
                        pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lUnitsOffset);
                        if(pQtwiItem)
                            strItemtext = pQtwiItem->text();
                        else
                            strItemtext = QString("");
                        strUnits = strItemtext;
                    }
                    else
                        strUnits = "";
                    strUnits = strUnits.trimmed();

                    if (m_lTestOffset >= 0)
                    {
                        pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lTestOffset);
                        if(pQtwiItem)
                            strItemtext = pQtwiItem->text();
                        else
                            strItemtext = QString("");

                        strItemtext = strItemtext.section('.',0,0);     // used to hack mpr test numbers export (include pin number)
                        m_lTestNumber = strItemtext.toInt(&bIsNumber);
                        GEX_ASSERT(bIsNumber);		// validation tool, can be commented

                        if(!bIsNumber)
                            continue;
                    }
                    else
                        m_lTestNumber = -1;

                    // On first run, get limits as well.
                    if(m_lPartID == 0)
                    {

                        if(m_lLowLOffset >= 0)
                        {
                            pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lLowLOffset);
                            if(pQtwiItem)
                                strItemtext = pQtwiItem->text();
                            else
                                strItemtext = QString("");
                            strLowL = strItemtext.trimmed();
                        }
                        else
                            strLowL = "";

                        if(m_lHighLOffset >= 0)
                        {
                            pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lHighLOffset);
                            if(pQtwiItem)
                                strItemtext = pQtwiItem->text();
                            else
                                strItemtext = QString("");
                            strHighL = strItemtext.trimmed();
                        }
                        else
                            strHighL = "";
                    }

                    // Dump PTR record for this parameter & run
                    m_iTotalTests += WritePTR(m_lPartID,strString,strLabel,strUnits,strLowL,strHighL,bNeedPir);
                }
            }
            // Write PRR
            if(m_iTotalTests)
            {
                WritePRR(m_lTotalPartsInWafer);
                bNeedPir = true;
            }
nextCol: ;
        }
    }
    else		// One test per column
    {

        // Loop over line (parts)
        for(int iRow=0; iRow < nRowCount; iRow++)
        {
            // Clear number of tests in run.
            m_iTotalTests = 0;
            m_iTotalPassFailTests=0;

            // Extract global fields for this run
            if((iRow > GEX_TB_MAXLINES_PREVIEW) || (m_pLineType[iRow] == GEX_TB_TEST_ID_DATA))
            {
                // Clear part info
                m_iSoftBin	= m_iHardBin = -1;
                m_iDieX		= m_iDieY = GEX_TB_UNDEFINED_DIE_LOC;
                m_iSiteID	= 255;

                // Get Bin#
                if(m_lSoftBinOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lSoftBinOffset);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iSoftBin = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Soft bin #... ").arg( strItemtext).toLatin1().constData());
                }
                if(m_lHardBinOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lHardBinOffset);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iHardBin = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Hard bin #... ").arg( strItemtext).toLatin1().constData());
                }

                // Get Die location
                if(m_lDieXOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lDieXOffset);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iDieX = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Die X... ").arg( strItemtext).toLatin1().constData());
                }
                if(m_lDieYOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lDieYOffset);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iDieY = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Die Y... ").arg( strItemtext).toLatin1().constData());
                }

                // Get site ID
                if (m_lSiteOffset >= 0)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lSiteOffset);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_iSiteID = strItemtext.toInt(&bConversionRslt);
                    if (!bConversionRslt)
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to convert %1 to Site ID... ").arg( strItemtext).toLatin1().constData());
                }

                // The SublotID / WaferID is extracted from the table data
                if(m_lSublotOffset >= 0)
                {
                    m_strPreviousSubLotId = m_strSubLotId;

                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, m_lSublotOffset);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    m_strSubLotId = strItemtext;

                    // Write WIR (if applicable & on first valid test found)
                    if((m_strPreviousSubLotId.isEmpty() == false) && (m_strPreviousSubLotId != m_strSubLotId))
                        WriteWRR(m_lTotalPartsInWafer);
                    if(m_strPreviousSubLotId != m_strSubLotId)
                    {
                        WriteWIR();
                        m_strPreviousSubLotId = m_strSubLotId;
                    }
                }
            }

            // Loop over columns (tests in part)
            for(int iCol=0; iCol< nColumnCount; iCol++)
            {
                // Update progress bar...
                if(lNotifyProgress & 64)
                {
                    // Notify Progress every 64 cells processed
                    iStep = (int) ((float)iRow*(float)nColumnCount*100.0/fTotalStep);
                    progress.setValue(iStep);
                    QCoreApplication::processEvents();

                    if(progress.wasCanceled())
                        return true;
                }
                lNotifyProgress++;

                // Only Dump DATA lines! (Any data after line #100 are data, labels (etc) can only be defined in the first 100 lines!
                if((iRow <= GEX_TB_MAXLINES_PREVIEW) && (m_pLineType[iRow] != GEX_TB_TEST_ID_DATA))
                    goto nextLine;

                // Check if this cell is a test 'P/F' status.
                if(m_pColumnType[iCol] == GEX_TB_PART_ID_TESTPF)
                {
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    strString = strItemtext.trimmed();

                    if(strString.startsWith("F", Qt::CaseInsensitive) || strString.startsWith("0", Qt::CaseInsensitive))
                        m_iTotalPassFailTests++;	// Keep track of total Fail tests in flow.
                }

                // Only Dump PARAMETER cols!
                if (m_pColumnType[iCol] == GEX_TB_PART_ID_PARAMETER)
                {
                    // Cell value
                    pQtwiItem = mUi_pQtwExcelTableWidget->item(iRow, iCol);
                    if(pQtwiItem)
                        strItemtext = pQtwiItem->text();
                    else
                        strItemtext = QString("");
                    strString = strItemtext.trimmed();


                    if(m_lLabelOffset >= 0)
                    {
                        pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lLabelOffset, iCol);
                        if(pQtwiItem)
                            strItemtext = pQtwiItem->text();
                        else
                            strItemtext = QString("");
                        strLabel = strItemtext.trimmed();

                        if(strLabel.isEmpty())
                            strLabel = "Parameter " + QString::number(iCol+1);
                    }
                    else
                        strLabel = "Parameter " + QString::number(iCol+1);
                    strLabel = strLabel.trimmed();

                    if(m_lUnitsOffset >= 0)
                    {
                        pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lUnitsOffset, iCol);
                        if(pQtwiItem)
                            strItemtext = pQtwiItem->text();
                        else
                            strItemtext = QString("");
                        strUnits = strItemtext;
                    }
                    else
                        strUnits = "";
                    strUnits = strUnits.trimmed();

                    if (m_lTestOffset >= 0)
                    {
                        pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lTestOffset, iCol);
                        if(pQtwiItem)
                            strItemtext = pQtwiItem->text();
                        else
                            strItemtext = QString("");

                        strItemtext = strItemtext.section('.',0,0); // used to hack mpr export
                        m_lTestNumber = strItemtext.toInt(&bIsNumber);

                        if(!bIsNumber)
                            continue;
                    }
                    else
                        m_lTestNumber = -1;

                    // On first run, get limits as well.
                    if(m_lPartID == 0)
                    {

                        if(m_lLowLOffset >= 0)
                        {
                            pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lLowLOffset, iCol);
                            if(pQtwiItem)
                                strItemtext = pQtwiItem->text();
                            else
                                strItemtext = QString("");
                            strLowL = strItemtext;
                        }
                        else
                            strLowL = "";

                        if(m_lHighLOffset >= 0)
                        {
                            pQtwiItem = mUi_pQtwExcelTableWidget->item(m_lHighLOffset, iCol);
                            if(pQtwiItem)
                                strItemtext = pQtwiItem->text();
                            else
                                strItemtext = QString("");
                            strHighL = strItemtext;
                        }
                        else
                            strHighL = "";
                    }

                    // Dump PTR record for this parameter & run
                    m_iTotalTests += WritePTR(m_lPartID,strString,strLabel,strUnits,strLowL,strHighL,bNeedPir);
                }
            }

            // Write PRR
            if(m_iTotalTests)
            {
                WritePRR(m_lTotalPartsInWafer);
                bNeedPir = true;
            }
nextLine: ;
        }
    }

    // Write WRR (if applicable)
    WriteWRR(m_lTotalPartsInWafer);

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(time(NULL));			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}


QString ImportCsvWizardDialog::repaintItems()
{
    if(!isVisible())
        return QString("ok");

    int nRowCount = mUi_pQtwExcelTableWidget->rowCount();
    int nColumnCount = mUi_pQtwExcelTableWidget->columnCount();
    QTableWidgetItem *pQtwiSelectedItem;
    bool bRepaintRslt = true;
    GSLOG(SYSLOG_SEV_DEBUG, QString("repaint %1 cols x %2 rows Items... ")
          .arg( nColumnCount)
          .arg(nRowCount).toLatin1().constData());
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("Generating %1 cols per %2 rows...").arg(nColumnCount).arg(nRowCount) );

    /*
    void *t=malloc(nColumnCount*nRowCount*sizeof(QTableWidgetItem));
    if (!t)
    {
        QString r=QString( "Not enough available memory to create %1 items ").arg( nColumnCount*nRowCount );
        GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data());
        return QString("error : %1").arg(r);
    }
    if (t)
        free(t);
    */

    QCoreApplication::processEvents();
    for(int nColIterator = 0; nColIterator < nColumnCount; nColIterator++)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("repaint col %1 ... ").arg( nColIterator ).toLatin1().constData());
        void *t=malloc(nRowCount*sizeof(QTableWidgetItem));
        if (!t)
        {
            QString r=QString( "Not enough available memory to create %1 items (QTableWidgetItem)").arg( nRowCount);
            GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data());
            return QString("error : %1").arg(r);
        }
        if (t)
            free(t);

        for(int nRowIterator = 0; nRowIterator < nRowCount; nRowIterator++)
        {
            pQtwiSelectedItem = mUi_pQtwExcelTableWidget->item(nRowIterator, nColIterator);
            if(!pQtwiSelectedItem)
            {
                // the try catch does not seem to work because Qt catch the signal before us...
                try
                {
                    // new(std::nothrow) does not seem to change anything ..
                    pQtwiSelectedItem =
                        new QTableWidgetItem(QTableWidgetItem::Type);
                    if (!pQtwiSelectedItem)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "cant create a new QTableWidgetItem");
                        return "error : low memory";
                    }
                }
                catch(const std::bad_alloc& e)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
                    return "error : Memory allocation exception caught ";
                }
                catch(...)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
                    return "error : Exception caught";
                }

                mUi_pQtwExcelTableWidget->setItem(nRowIterator, nColIterator, pQtwiSelectedItem);
            }
            bRepaintRslt &= repaintItem(pQtwiSelectedItem, nColIterator, nRowIterator);
        }
    }

    return bRepaintRslt?"ok":"error";
}


bool ImportCsvWizardDialog::repaintItem(QTableWidgetItem *pQtwiItem, int nItemCol, int nItemRow)
{
    if(!pQtwiItem)
        return false;

    QBrush qbBrush;

    if(m_bTestInLine)
    {
        // One test per line...see which Columns are ignored
        if(m_pColumnType[nItemCol] == GEX_TB_TEST_ID_IGNORE || m_pLineType[nItemRow] == GEX_TB_PART_ID_IGNORE)
            qbBrush.setColor( Qt::lightGray );	// Paint with Light Gray for cells to ignore.
        else
            qbBrush.setColor( QColor(255,255,128) );	// Light Yellow for cells to parse/import.
    }
    else
    {
        // One test per Column...see which Lines are ignored
        if( (m_pLineType[nItemRow] == GEX_TB_TEST_ID_IGNORE)			||
            (m_pColumnType[nItemCol] == GEX_TB_PART_ID_IGNORE)	)
            qbBrush.setColor(Qt::lightGray);	// Paint with Light Gray for cells to ignore.
        else
            qbBrush.setColor(QColor(255,255,128));	// Light Yellow for cells to parse/import.
    }

    pQtwiItem->setData(Qt::BackgroundColorRole, qbBrush);

    // everything went well
    return true;
}


// inherited methods
void ImportCsvWizardDialog::showEvent ( QShowEvent * event )
{
    repaintItems();
    QDialog::showEvent(event);
}


//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString ImportCsvWizardDialog::ReadLine(QTextStream& hFile)
{
    QString strString;

    do
    {
        strString = hFile.readLine();
        // Skip empty CVS line
        if(strString.left(3) == ",,," && (strString.simplified().count(",")==strString.simplified().length()))
            strString = "";
    }
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}
