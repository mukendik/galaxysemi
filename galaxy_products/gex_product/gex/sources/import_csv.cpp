//////////////////////////////////////////////////////////////////////
// import_csv.cpp: Convert a .CSV file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "import_csv.h"
#include "gex_shared.h"
#include "engine.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "tb_import_csv_wizard_dialog.h"
#include "import_constants.h"
#include "temporary_files_manager.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "message.h"
#include <math.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

// File format:
// <Header>
// Parameter , <Parm1>,<Parm2>, ..., <ParmN>, [gex_bin], [gex_time]
// Tests#, <testName1>,<testName2>, ... , <testNameN>
// Patterns, <pattern1>,<pattern2>, ... ,<patternN>
// Unit, <unit1>,<unit2>,..., <unitN>
// USL, <UpperLimit1>, <UpperLimit2>,..., <UpperLimitN>
// LSL, <LowerLimit1>, <LowerLimit2>,..., <LowerLimitN>
// <Part#>, <Result1>, <Result2>,..., <ResultN>

// in main.cpp
extern QLabel *			GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar *	GexProgressBar;		// Handle to progress bar in status bar
#if MODULE == GEX
#include "browser_dialog.h"
extern GexMainwindow *	pGexMainWindow;
#endif
extern CReportOptions	ReportOptions;

extern void				WriteDebugMessageFile(const QString & strMessage);

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGCSVtoSTDF::CGCSVtoSTDF()
{
    // Default: CSV parameter list on disk includes all known CSV parameters...
    bNewCsvParameterFound	= false;
    nUnitsMode				= GEX_OPTIONS_TOOLBOX_UNITS_NORMALIZED;

    pCGCsvParameter = NULL;

    lStartTime = lSetupTime = lFinishTime = -1;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGCSVtoSTDF::~CGCSVtoSTDF()
{
    // Destroy list of Parameters tables.
    if(pCGCsvParameter!=NULL)
        delete [] pCGCsvParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGCSVtoSTDF::GetLastError()
{
    strLastError = "Import CSV / TXT: ";

    switch(iLastError)
    {
    default:
    case errNoError:
        strLastError += "No Error";
        break;
    case errOpenFail:
        strLastError += "Failed to open file";
        break;
    case errInvalidFormatParameter:
        strLastError += "Invalid file format: Didn't find 'Parameter' line\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
        break;
    case errInvalidFormatLowInRows:
        strLastError += "Invalid file format: 'Parameter' line too short, missing rows\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
        break;
    case errInvalidFormatMissingUnit:
        strLastError += "Invalid file format: 'Unit' line missing\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
        break;
    case errInvalidFormatMissingUSL:
        strLastError += "Invalid file format: 'USL' line missing\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
        break;
    case errInvalidFormatMissingLSL:
        strLastError += "Invalid file format: 'LSL' line missing\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
        break;
    case errWriteSTDF:
        strLastError += "Failed creating temporary file. Folder permission issue?";
        break;
    case errLicenceExpired:
        strLastError += "License has expired or Data file out of date...";
        break;
    }
    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load CSV Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGCSVtoSTDF::LoadParameterIndexTable(void)
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
void CGCSVtoSTDF::DumpParameterIndexTable(void)
{
    QString		strCsvTableFile;
    int			nIndex;

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
    for(nIndex = 0; nIndex < pFullCsvParametersList.count(); nIndex++)
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
void CGCSVtoSTDF::UpdateParameterIndexTable(QString strParamName)
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
// Clear variables
//////////////////////////////////////////////////////////////////////
void CGCSVtoSTDF::clear(void)
{
    m_lfWaferDiameter=0.0;			// Wafer diameter
    m_lfWaferDieWidth=0.0;			// Die X size
    m_lfWaferDieHeight=0.0;			// Die Y size
    m_cWaferUnits=0;				// Unknown units
    m_cWaferFlat=' ';				// Flat orientation: U, D, R or L
    m_iWaferCenterDieX = m_iWaferCenterDieY = -32768;	// coordinates of center die on wafer
    m_cWaferPosX = m_cWaferPosY = ' ';	// Positive X & Y direction of wafer
    lStartTime = lSetupTime = lFinishTime = -1;
    mSpecVer = "";
    mFlowId = "";
    mSetupId = "";
    mDsgnRev = "";
    mEngId = "";
    mSerlNum = "";
    mSuprNam = "";
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the CSV file
// bAllowExtendedCsv= true if allow to input CSV file with
// custom first columns for Sbin,Hbin,dieXY,Site# & time.
//////////////////////////////////////////////////////////////////////
bool CGCSVtoSTDF::ReadCsvFile(const char* CsvFileName,
                              const char* strFileNameSTDF,
                              bool /*bAllowExtendedCsv*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( CsvFileName).toLatin1().constData());

    QString strString;
    QString strSection;
    bool	bStatus;
    unsigned int uIndex; // Loop index

    // Clear variables
    clear();

    // Open CSV file
    QFile f( CsvFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hCsvFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size()+1;

    strTesterName = strTesterType = strProgramName= "CSV";				// Job name
    strProgramRev = strOperator = strExecType = strExecRev = "CSV";
    strTestCode = strFacilityID = strFloorID = strProcessID = "CSV";
    byRtstCode = ' ';       // default value if unknown
    m_qlCssv4SdrList.clear();
    iBurninTime = 65535;
    strlPassSBins.clear();
    strlPassHBins.clear();

    // Erase Binning count tables (only used if extended CSV file)
    SoftBinMap.clear();
    HardBinMap.clear();

    // Get current time/date, in case no dates specifie in CSV file
    time_t lDateTime = -1;

    // Check if valid CSV header...extract header data.
    do
    {
        strString = ReadLine(hCsvFile);
        if(strString.isEmpty())
            continue;

        strSection = strString.section(',',0,0);	// First field is 'Parameter'
        strSection = strSection.trimmed();	// remove leading spaces.
        strSection = strSection.toLower();
        if(strSection == "lot")
        {
            strLotID = strString.section(',',1,1);
            strLotID = strLotID.trimmed();
        }
        else if(strSection == "sublot")
        {
            strSubLotID = strString.section(',',1,1);
            strSubLotID = strSubLotID.trimmed();
        }

        // Wafer definition info
        else if((strSection == "wafer") || (strSection == "waferid"))
        {
            strWaferID = strString.section(',',1,1);
            strWaferID = strWaferID.trimmed();
        }

        else if(strSection == "wafer_diameter")		// old name
        {
            strString = strString.section(',',1,1);
            m_lfWaferDiameter = strString.toFloat();
        }

        else if(strSection == "wafer-diameter")
        {
            strString = strString.section(',',1,1);
            m_lfWaferDiameter = strString.toFloat();
        }

        else if(strSection == "die_width")			// old name
        {
            strString = strString.section(',',1,1);
            m_lfWaferDieWidth = strString.toFloat();
        }

        else if(strSection == "wafer_width")
        {
            strString = strString.section(',',1,1);
            m_lfWaferDieWidth = strString.toFloat();
        }

        else if(strSection == "die_height")			// old name
        {
            strString = strString.section(',',1,1);
            m_lfWaferDieHeight = strString.toFloat();
        }

        else if(strSection == "wafer_height")
        {
            strString = strString.section(',',1,1);
            m_lfWaferDieHeight = strString.toFloat();
        }

        else if(strSection == "wafer_units")
        {
            strString = strString.section(',',1,1);
            m_cWaferUnits = strString.toInt();
            if(m_cWaferUnits != 0 && m_cWaferUnits != 1 && m_cWaferUnits != 2 && m_cWaferUnits != 3 && m_cWaferUnits != 4 )
                m_cWaferFlat = 0;
        }

        else if(strSection == "wafer_flat")
        {
            m_cWaferFlat = ' ';

            strString = strString.section(',',1,1);
            if(!strString.isEmpty())
            {
                m_cWaferFlat = strString.at(0).toUpper().toLatin1();
                if(m_cWaferFlat != 'U' && m_cWaferFlat!= 'D' && m_cWaferFlat!= 'R' && m_cWaferFlat!= 'L')
                    m_cWaferFlat = ' ';
            }
        }

        else if(strSection == "wafer_center_x")
        {
            strString = strString.section(',',1,1);
            m_iWaferCenterDieX = strString.toInt();
        }

        else if(strSection == "wafer_center_y")
        {
            strString = strString.section(',',1,1);
            m_iWaferCenterDieY = strString.toInt();
        }

        else if(strSection == "wafer_pos_x")
        {
            m_cWaferPosX = ' ';

            strString		= strString.section(',',1,1);
            if(!strString.isEmpty())
            {
                m_cWaferPosX	= strString.at(0).toUpper().toLatin1();
                if(m_cWaferPosX!='L' && m_cWaferPosX!='R')
                    m_cWaferPosX = ' ';
            }
        }

        else if(strSection == "wafer_pos_y")
        {
            m_cWaferPosY = ' ';

            strString		= strString.section(',',1,1);
            if(!strString.isEmpty())
            {
                m_cWaferPosY	= strString.at(0).toUpper().toLatin1();
                if(m_cWaferPosY!='U' && m_cWaferPosY!='D')
                    m_cWaferPosY = ' ';
            }
        }

        else if(strSection == "temperature")
        {
            strTemperature = strString.section(',',1,1);
            strTemperature = strTemperature.trimmed();
        }

        else if(strSection == "packagetype")
        {
            strPackageType = strString.section(',',1,1);
            strPackageType = strPackageType.trimmed();
        }

        else if(strSection == "family")
        {
            strFamilyID = strString.section(',',1,1);
            strFamilyID = strFamilyID.trimmed();

        }

        else if(strSection == "freqstep")
        {
            strFrequencyStep = strString.section(',',1,1);
            strFrequencyStep = strFrequencyStep.trimmed();

        }
        else if(strSection == "specversion")
        {
            mSpecVer = strString.section(',',1,1);
            mSpecVer = mSpecVer.trimmed();
        }
        else if(strSection == "flowid")
        {
            mFlowId = strString.section(',',1,1);
            mFlowId = mFlowId.trimmed();
        }
        else if(strSection == "setupid")
        {
            mSetupId = strString.section(',',1,1);
            mSetupId = mSetupId.trimmed();
        }
        else if(strSection == "designrevision")
        {
            mDsgnRev = strString.section(',',1,1);
            mDsgnRev = mDsgnRev.trimmed();
        }
        else if(strSection == "engineeringlotid")
        {
            mEngId = strString.section(',',1,1);
            mEngId = mEngId.trimmed();
        }
        else if(strSection == "testerserialnumber")
        {
            mSerlNum = strString.section(',',1,1);
            mSerlNum = mSerlNum.trimmed();
        }
        else if(strSection == "supervisorname")
        {
            mSuprNam = strString.section(',',1,1);
            mSuprNam = mSuprNam.trimmed();
        }
        else if(strSection == "operator")
        {
            strOperator = strString.section(',',1,1);
            strOperator = strOperator.trimmed();

        }

        else if(strSection == "product")
        {
            strProductID = strString.section(',',1,1);
            strProductID = strProductID.trimmed();

        }

        else if(strSection == "testername")
        {
            strTesterName = strString.section(',',1,1);
            strTesterName = strTesterName.trimmed();

        }

        else if(strSection == "testername")
        {
            strTesterName = strString.section(',',1,1);
            strTesterName = strTesterName.trimmed();

        }

        else if(strSection == "testertype")
        {
            strTesterType = strString.section(',',1,1);
            strTesterType = strTesterType.trimmed();

        }

        else if(strSection == "programname")
        {
            strProgramName = strString.section(',',1,1);
            strProgramName = strProgramName.trimmed();

        }

        else if(strSection == "programrevision")
        {
            strProgramRev = strString.section(',',1,1);
            strProgramRev = strProgramRev.trimmed();

        }

        else if(strSection == "exectype")
        {
            strExecType = strString.section(',',1,1);
            strExecType = strExecType.trimmed();

        }

        else if(strSection == "execrevision")
        {
            //strExecRev = strString.section(',',1,1);      //case 4059
            strExecRev = strString.section(',',1,-1);
            strExecRev = strExecRev.trimmed();

        }

        else if(strSection == "rtstcode")
        {
            QString strRtstCode = strString.section(',',1,1);
            strRtstCode = strRtstCode.trimmed();

            QStringList qslValidValueList;
            qslValidValueList << "Y" << "N" << " " << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9";

            if (!qslValidValueList.contains(strRtstCode))
            {
                GSLOG(SYSLOG_SEV_CRITICAL, QString("Invalide RtstCode value in csv file : %1")
                      .arg( strRtstCode).toLatin1().constData());
                strRtstCode = ' ';      // default value
            }

            QByteArray qbaRtstCode(strRtstCode.toLatin1());
            byRtstCode = (BYTE)(*qbaRtstCode.data());


        }

        else if(strSection == "testcode")
        {
            strTestCode = strString.section(',',1,1);
            strTestCode = strTestCode.trimmed();

        }

        else if(strSection == "facility")
        {
            strFacilityID = strString.section(',',1,1);
            strFacilityID = strFacilityID.trimmed();

        }

        else if(strSection == "floor")
        {
            strFloorID = strString.section(',',1,1);
            strFloorID = strFloorID.trimmed();

        }

        else if(strSection == "process")
        {
            strProcessID = strString.section(',',1,1);
            strProcessID = strProcessID.trimmed();

        }

        else if(strSection == "burnintime")		// old name
        {
            strString = strString.section(',',1,1);
            iBurninTime = strString.toInt();

        }

        else if(strSection == "burntime")
        {
            strString = strString.section(',',1,1);
            iBurninTime = strString.toInt();

        }


        else if( (strSection == "load board id") || (strSection == "loadboardname") )	// loadboardname = old name
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strLoadBoardName = strString.section(',',1,1);
                strLoadBoardName = strLoadBoardName.trimmed();
                m_qlCssv4SdrList.last().SetLOAD_ID(strLoadBoardName);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, load board id should be preceded by '--- site details:");
               GEX_ASSERT(false);
            }


        }


        else if( (strSection == "load board type") || (strSection == "loadboardtype") )	// loadboardtype = old name
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strLoadBoardType = strString.section(',',1,1);
                strLoadBoardType = strLoadBoardType.trimmed();
                m_qlCssv4SdrList.last().SetLOAD_TYP(strLoadBoardType);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, load board type should be preceded by '--- site details:");
               GEX_ASSERT(false);
            }


        }

        else if(strSection == "dib board type")
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strDibBoardType = strString.section(',',1,1);
                strDibBoardType = strDibBoardType.trimmed();
                m_qlCssv4SdrList.last().SetDIB_TYP(strDibBoardType);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, dib board type should be preceded by '--- site details:");
               GEX_ASSERT(false);
            }


        }

        else if(strSection == "dib board id")
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strDibBoardName = strString.section(',',1,1);
                strDibBoardName = strDibBoardName.trimmed();
                m_qlCssv4SdrList.last().SetDIB_ID(strDibBoardName);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, dib board id should be preceded by '--- site details:");
               GEX_ASSERT(false);
            }


        }

        else if(strSection == "--- site details:")
        {
            GQTL_STDF::Stdf_SDR_V4 cssv4SdrRecord;

            // set default values
            cssv4SdrRecord.SetHEAD_NUM(1);
            cssv4SdrRecord.SetSITE_GRP(1);
            cssv4SdrRecord.SetSITE_CNT(1);
            cssv4SdrRecord.SetSITE_NUM(0,1);        // start count by 0

            // head#
            strString = strString.section(",", 1, 1);
            strString = strString.trimmed();
            strString = strString.toLower();
            strString = strString.section("head #",1,1);
            strString = strString.trimmed();

            bool bIsConvertible=false;
            int nHeadNumber = strString.toInt(&bIsConvertible);

            if( bIsConvertible && (nHeadNumber>=0) && (nHeadNumber<=255) )
                cssv4SdrRecord.SetHEAD_NUM( (stdf_type_u1)nHeadNumber );

            m_qlCssv4SdrList.append(cssv4SdrRecord);


        }

        else if(strSection == "site group")
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strSiteGrp = strString.section(',',1,1);
                strSiteGrp = strSiteGrp.trimmed();

                bool bIsValidGroup = false;
                stdf_type_u1 stu1SiteGrp;
                int nSiteGrp = strSiteGrp.toInt(&bIsValidGroup);

                if( (nSiteGrp<0)||(nSiteGrp>255)||(!bIsValidGroup) )
                {
                    GSLOG(SYSLOG_SEV_WARNING, "Invalid site group in csv file ! (correspond to SDR.SITE_GRP)");
                    stu1SiteGrp = 1;
                }
                else
                    stu1SiteGrp = nSiteGrp;

                m_qlCssv4SdrList.last().SetSITE_GRP(stu1SiteGrp);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, handler id should be preceded by '--- site details:");
               GEX_ASSERT(false);
            }


        }



        else if(strSection == "testing sites")
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strTestingSitesString = strString.section(',',1,1);
                strTestingSitesString = strTestingSitesString.trimmed();

                QStringList qslTestingSitesList = strTestingSitesString.split(QString(" "), QString::SkipEmptyParts);
                QList<stdf_type_u1> qlTestingSitesList;
                bool bIsWellConverted=false;
                int nSite=0;

                for(int ii=0; ii<qslTestingSitesList.count(); ii++)
                {
                    nSite = (qslTestingSitesList.at(ii)).toInt(&bIsWellConverted);
                    if( bIsWellConverted && (nSite>=0) && (nSite<=255) )
                        qlTestingSitesList.append((stdf_type_u1)nSite);
                }

                m_qlCssv4SdrList.last().SetSITE_CNT(qlTestingSitesList.count());
                for(int ii=0; ii<qlTestingSitesList.count(); ii++)
                    m_qlCssv4SdrList.last().SetSITE_NUM(ii, qlTestingSitesList.at(ii));
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, 'testing sites' should be preceded by '--- site details:'");
               GEX_ASSERT(false);
            }

        }

        else if( (strSection == "handler id") || (strSection == "probername") )		// probername = old name
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strProberName = strString.section(',',1,1);
                strProberName = strProberName.trimmed();
                m_qlCssv4SdrList.last().SetHAND_ID(strProberName);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, handler id should be preceded by '--- site details:");
                GEX_ASSERT(false);
            }


        }

        else if( (strSection == "handler type") || (strSection == "probertype") )        // probertype : old name
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strProberType = strString.section(',',1,1);
                strProberType = strProberType.trimmed();
                m_qlCssv4SdrList.last().SetHAND_TYP(strProberType);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, handler type should be preceded by '--- site details:");
                GEX_ASSERT(false);
            }

        }

        else if( strSection == "probe card id" )
        {
            if(!m_qlCssv4SdrList.isEmpty())
            {
                QString strProbeCardID = strString.section(',',1,1);
                strProbeCardID = strProbeCardID.trimmed();
                m_qlCssv4SdrList.last().SetCARD_ID(strProbeCardID);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "Wrong csv format, 'probe card id' should be preceded by '--- site details:");
                GEX_ASSERT(false);
            }


        }

        else if(strSection == "sbinspass")
        {
            strlPassSBins = strString.section(',',1,1).trimmed().split(":");

        }

        else if(strSection == "hbinspass")
        {
            strlPassHBins = strString.section(',',1,1).trimmed().split(":");

        }

        // Date info
        else if(strSection == "date")
        {
            strSection = strString.section(',',1,1);// Date format: YYYY_MM_DD [HH:MM:SS]
            strSection = strSection.trimmed();
            // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
            int iDay,iMonth,iYear;
            int	iHours,iMinutes,iSeconds;
            QString	strDate,strTime;
            strDate = strSection.section(' ',0,0);
            strTime = strSection.section(' ',1,1);
            strSection = strDate.section('_',0,0);	// First field is Year
            iYear = strSection.toInt();
            strSection = strDate.section('_',1,1);	// Second field is Month
            iMonth = strSection.toInt();
            strSection = strDate.section('_',2,2);	// Third field is Day
            iDay = strSection.toInt();

            strSection = strTime.section(':',0,0);	// First field is Hours
            iHours = strSection.toInt();
            strSection = strTime.section(':',1,1);	// Second field is Minutes
            iMinutes = strSection.toInt();
            strSection = strTime.section(':',2,2);	// Third field is Seconds
            iSeconds = strSection.toInt();

            QDate CsvDate(iYear,iMonth,iDay);
            QTime CsvTime(iHours,iMinutes,iSeconds);
            QDateTime CsvDateTime(CsvDate);
            if(CsvDateTime.isValid() && CsvDate.year()>1970)
            {
                CsvDateTime.setTimeSpec(Qt::UTC);
                CsvDateTime.setTime(CsvTime);
                lDateTime = CsvDateTime.toTime_t();
            }

        }

        // Setup time
        else if(strSection == "setuptime")
        {
            strSection = strString.section(',',1,1);// Date format: YYYY_MM_DD [HH:MM:SS]
            strSection = strSection.trimmed();
            // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
            int iDay,iMonth,iYear;
            int	iHours,iMinutes,iSeconds;
            QString	strDate,strTime;
            strDate = strSection.section(' ',0,0);
            strTime = strSection.section(' ',1,1);
            strSection = strDate.section('_',0,0);	// First field is Year
            iYear = strSection.toInt();
            strSection = strDate.section('_',1,1);	// Second field is Month
            iMonth = strSection.toInt();
            strSection = strDate.section('_',2,2);	// Third field is Day
            iDay = strSection.toInt();

            strSection = strTime.section(':',0,0);	// First field is Hours
            iHours = strSection.toInt();
            strSection = strTime.section(':',1,1);	// Second field is Minutes
            iMinutes = strSection.toInt();
            strSection = strTime.section(':',2,2);	// Third field is Seconds
            iSeconds = strSection.toInt();

            QDate CsvDate(iYear,iMonth,iDay);
            QTime CsvTime(iHours,iMinutes,iSeconds);
            QDateTime CsvDateTime(CsvDate);
            if(CsvDateTime.isValid() && CsvDate.year()>1970)
            {
                CsvDateTime.setTimeSpec(Qt::UTC);
                CsvDateTime.setTime(CsvTime);
                lSetupTime = CsvDateTime.toTime_t();
            }

        }

        // Start time
        else if(strSection == "starttime")
        {
            strSection = strString.section(',',1,1);// Date format: YYYY_MM_DD [HH:MM:SS]
            strSection = strSection.trimmed();
            // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
            int iDay,iMonth,iYear;
            int	iHours,iMinutes,iSeconds;
            QString	strDate,strTime;
            strDate = strSection.section(' ',0,0);
            strTime = strSection.section(' ',1,1);
            strSection = strDate.section('_',0,0);	// First field is Year
            iYear = strSection.toInt();
            strSection = strDate.section('_',1,1);	// Second field is Month
            iMonth = strSection.toInt();
            strSection = strDate.section('_',2,2);	// Third field is Day
            iDay = strSection.toInt();

            strSection = strTime.section(':',0,0);	// First field is Hours
            iHours = strSection.toInt();
            strSection = strTime.section(':',1,1);	// Second field is Minutes
            iMinutes = strSection.toInt();
            strSection = strTime.section(':',2,2);	// Third field is Seconds
            iSeconds = strSection.toInt();

            QDate CsvDate(iYear,iMonth,iDay);
            QTime CsvTime(iHours,iMinutes,iSeconds);
            QDateTime CsvDateTime(CsvDate);
            if(CsvDateTime.isValid() && CsvDate.year()>1970)
            {
                CsvDateTime.setTimeSpec(Qt::UTC);
                CsvDateTime.setTime(CsvTime);
                lStartTime = CsvDateTime.toTime_t();
            }

        }

        // Finish Time
        else if(strSection == "finishtime")
        {
            strSection = strString.section(',',1,1);// Date format: YYYY_MM_DD [HH:MM:SS]
            strSection = strSection.trimmed();
            // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
            int iDay,iMonth,iYear;
            int	iHours,iMinutes,iSeconds;
            QString	strDate,strTime;
            strDate = strSection.section(' ',0,0);
            strTime = strSection.section(' ',1,1);
            strSection = strDate.section('_',0,0);	// First field is Year
            iYear = strSection.toInt();
            strSection = strDate.section('_',1,1);	// Second field is Month
            iMonth = strSection.toInt();
            strSection = strDate.section('_',2,2);	// Third field is Day
            iDay = strSection.toInt();

            strSection = strTime.section(':',0,0);	// First field is Hours
            iHours = strSection.toInt();
            strSection = strTime.section(':',1,1);	// Second field is Minutes
            iMinutes = strSection.toInt();
            strSection = strTime.section(':',2,2);	// Third field is Seconds
            iSeconds = strSection.toInt();

            QDate CsvDate(iYear,iMonth,iDay);
            QTime CsvTime(iHours,iMinutes,iSeconds);
            QDateTime CsvDateTime(CsvDate);
            if(CsvDateTime.isValid() && CsvDate.year()>1970)
            {
                CsvDateTime.setTimeSpec(Qt::UTC);
                CsvDateTime.setTime(CsvTime);
                lFinishTime = CsvDateTime.toTime_t();
            }

        }

        // SOFT Binning name
        else if(strSection == "softbinname")
        {
            bool	bOk;
            int		iBinNumber;
            QString	strBinName;

            iBinNumber = strString.section(',',1,1).trimmed().toInt(&bOk);
            if(!bOk)
                continue;
            strBinName = strString.section(',',2,2);

            // Create binning entry.
            struct CBinCell *ptCell;
            ptCell = new CBinCell;
            ptCell->lBinCount = 0;
            ptCell->lBinIndex = iBinNumber;
            ptCell->strBinName = strBinName;
            SoftBinMap[iBinNumber] = ptCell;

        }

        // HARD Binning name
        else if(strSection == "hardbinname")
        {
            bool	bOk;
            int		iBinNumber;
            QString	strBinName;

            iBinNumber = strString.section(',',1,1).trimmed().toInt(&bOk);
            if(!bOk)
                continue;
            strBinName = strString.section(',',2,2);

            // Create binning entry.
            struct CBinCell *ptCell;
            ptCell = new CBinCell;
            ptCell->lBinCount = 0;
            ptCell->lBinIndex = iBinNumber;
            ptCell->strBinName = strBinName;
            HardBinMap[iBinNumber] = ptCell;

        }

        // Units mode
        if (strSection == "unitsmode")
        {
            QString strUnitsMode = strString.section(',',1,1);

            if (strUnitsMode.compare("normalized", Qt::CaseInsensitive) == 0)
                nUnitsMode = GEX_OPTIONS_TOOLBOX_UNITS_NORMALIZED;
            else if (strUnitsMode.compare("scaling_factor", Qt::CaseInsensitive) == 0)
                nUnitsMode = GEX_OPTIONS_TOOLBOX_UNITS_SCALING_FACTOR;
        }

        else if((strSection == "parameter") || (strSection == "parameters"))
            break;	// found first line to process
    }
    while(hCsvFile.atEnd() == false);

    if(hCsvFile.atEnd() == true)
    {
        // Incorrect header...this is not a valid CSV file!
        iLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Get current time/date, in case no dates specifie in CSV file
    if(lDateTime == -1)
    {
        // Check if have at least one valid date
        if(lSetupTime > -1)
             lDateTime = lSetupTime;
        else if(lStartTime > -1)
            lDateTime = lStartTime;
        else if(lFinishTime > -1)
            lDateTime = lFinishTime;
        else
            lDateTime = time(NULL);
    }
    // Use current time in case Header doesn't include a date/time line
    if(lStartTime == -1)
        lStartTime = lDateTime;
    if(lSetupTime == -1)
        lSetupTime = lDateTime;
    if(lFinishTime == -1)
        lFinishTime = lDateTime;

    // Count the number of parameters specified in the line
    QStringList strCells;
    QString strTestNumber,strTestPinmap;
    uTotalParameters=0;

    strCells = strString.split(",",QString::KeepEmptyParts);
    uTotalParameters = strCells.count();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 Total Parameters").arg( uTotalParameters).toLatin1().constData());
    // If no parameter specified...ignore!
    if(uTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid CSV file!
        iLastError = errInvalidFormatLowInRows;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    try
    {
        pCGCsvParameter = new CGCsvParameter[uTotalParameters];	// List of parameters
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        f.close();
        return false;
    }
    catch(...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        f.close();
        return false;
    }

    // Extract the N column names
    for (uIndex = 0;
         uIndex < uTotalParameters &&
         uIndex + 1 < (unsigned int) strCells.count();
         uIndex++)
    {
        strSection = strCells[uIndex + 1];
        strSection = strSection.trimmed();	// Remove spaces
        if(strSection.isEmpty())
            strSection = "-?-";	// Default name for any missing label!
        pCGCsvParameter[uIndex].lTestNumber = (unsigned long)-1;
        pCGCsvParameter[uIndex].strName = strSection;
        UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
        pCGCsvParameter[uIndex].bStaticHeaderWritten = false;
        pCGCsvParameter[uIndex].lScale = 0;
    }

    // Read next line...may be:
    // a) 'Tests#' if CSV file created by Examinator!
    // or
    // b)  Unit, A, mV, , , Hz
    strString = ReadLine(hCsvFile);
    // BG 27/07/2004 : use split and [] operator to access fields instead of section member (very slow)
    strCells = strString.split(",",QString::KeepEmptyParts);
    strSection = strCells[0].trimmed();	// Get first field and remove spaces
    bEditedDataFile = false;
    if((strSection.toLower() == "tests#") || (strSection.toLower() == "test#"))
    {
        // Check if first parameters are reserved keywords.
        if(pCGCsvParameter[0].strName == "SBIN" &&
                pCGCsvParameter[1].strName == "HBIN" &&
                pCGCsvParameter[2].strName == "DIE_X" &&
                pCGCsvParameter[3].strName == "DIE_Y" &&
                pCGCsvParameter[4].strName == "SITE" &&
                pCGCsvParameter[5].strName == "TIME" )
        {
            bEditedDataFile = true;	// Format compliant with CSV file created by Examinator (e.g:Edited data file)
            m_lStartParsingOffset = CSV_RAW_TIME+1;

            if(pCGCsvParameter[CSV_RAW_TOTAL_TESTS].strName == "TOTAL_TESTS")
                m_lStartParsingOffset = CSV_RAW_TOTAL_TESTS+1;

            if(pCGCsvParameter[CSV_RAW_LOT_ID].strName == "LOT_ID")
                m_lStartParsingOffset = CSV_RAW_LOT_ID+1;

            if(pCGCsvParameter[CSV_RAW_WAFER_ID].strName == "WAFER_ID")
                m_lStartParsingOffset = CSV_RAW_WAFER_ID+1;
        }

        // Extract test#, Pinmap index.
        for (uIndex = 0;
             uIndex < uTotalParameters &&
             uIndex + 1 < (unsigned int) strCells.count();
             uIndex++)
        {
            strSection = strCells[uIndex + 1].trimmed();	// Get field and remove spaces

            // string extracted is test#: either 'xxx' or 'xxx.yyy', or empty!
            if(strSection.isEmpty())
            {
                pCGCsvParameter[uIndex].lTestNumber = (unsigned long)-1;
                pCGCsvParameter[uIndex].lPinmapIndex = -1;
            }
            else
            {
                strTestNumber   = strSection.section('.',0,0);
                strTestPinmap   = strSection.section('.',1,1);
                pCGCsvParameter[uIndex].lTestNumber = strTestNumber.toLong();

                if(strTestPinmap.isEmpty())
                    pCGCsvParameter[uIndex].lPinmapIndex = -1;
                else
                    pCGCsvParameter[uIndex].lPinmapIndex = strTestPinmap.toLong();
            }
        }

        // Read next line: Parameters Patterns ('Patterns')
        strString = ReadLine(hCsvFile);
        strCells = strString.split(",",QString::KeepEmptyParts);
        strSection = strString.section(',',0,0);	// First field is 'Patterns'
        strSection = strSection.trimmed();	// remove leading spaces.
    }

    if((strSection.toLower() == "pattern") || (strSection.toLower() == "patterns"))
    {
        // Extract pattern name
        for (uIndex = 0;
             uIndex < uTotalParameters &&
             uIndex + 1 < (unsigned int) strCells.count();
             uIndex++)
        {
            // Get field and remove spaces
            strSection = strCells[uIndex + 1].trimmed();

            // string extracted is pattern name or "n/a" for functional test without pattern name, or empty for non functional test!
            pCGCsvParameter[uIndex].strPatternName = strSection;
        }

        // Read next line: Parameters Units ('Unit')
        strString = ReadLine(hCsvFile);
        strSection = strString.section(',',0,0);	// First field is 'Unit'
        strSection = strSection.trimmed();			// remove leading spaces.
    }

    if((strSection.toLower() != "unit") && (strSection.toLower() != "units"))
    {
        // Incorrect header...this is not a valid CSV file!
        iLastError = errInvalidFormatMissingUnit;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Units
    strCells = strString.split(",",QString::KeepEmptyParts);

    for (uIndex = 0;
         uIndex < uTotalParameters &&
         uIndex + 1 < (unsigned int) strCells.count();
         uIndex++)
    {
        strSection = strCells[uIndex + 1];
        strSection = strSection.trimmed();	// Remove spaces
        pCGCsvParameter[uIndex].strUnits = normalizeTestUnits(strSection,
                                                              pCGCsvParameter[uIndex].lScale);
    }

    // Read line with list of Parameters UpperLimit
    // eg: USL, 1.0454, 12.566, , , 9.00
    strString = ReadLine(hCsvFile, true);

    strSection = strString.section(',',0,0);	// First field is 'USL' or 'HighL'
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.toLower() != "usl" && strSection.toLower() != "highl")
    {
        // Incorrect header...this is not a valid CSV file!
        iLastError = errInvalidFormatMissingUSL;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Upper Limits
    strCells = strString.split(",",QString::KeepEmptyParts);
    for (uIndex = 0;
         uIndex < uTotalParameters &&
         uIndex + 1 < (unsigned int) strCells.count();
         uIndex++)
    {
        strSection = strCells[uIndex + 1].trimmed();
        pCGCsvParameter[uIndex].fHighLimit = normalizeTestResult(strSection.toFloat(&bStatus),
                                                                 pCGCsvParameter[uIndex].lScale);
        pCGCsvParameter[uIndex].bValidHighLimit = bStatus;
    }

    // Read line with list of Parameters LowerLimit
    // eg: LSL, 1.0454, 12.566, , , 9.00
    strString = ReadLine(hCsvFile, true);

    strSection = strString.section(',',0,0);	// First field is 'LSL' or 'LowL'
    strSection = strSection.trimmed();	// remove leading spaces.
    if(strSection.toLower() != "lsl" && strSection.toLower() != "lowl")
    {
        // Incorrect header...this is not a valid CSV file!
        iLastError = errInvalidFormatMissingLSL;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Extract the N column Lower Limits
    strCells = strString.split(",",QString::KeepEmptyParts);
    for (uIndex = 0;
         uIndex < uTotalParameters &&
         uIndex + 1 < (unsigned int) strCells.count();
         uIndex++)
    {
        strSection = strCells[uIndex + 1].trimmed();
        pCGCsvParameter[uIndex].fLowLimit =
                normalizeTestResult(strSection.toFloat(&bStatus),
                                    pCGCsvParameter[uIndex].lScale);
        pCGCsvParameter[uIndex].bValidLowLimit = bStatus;
    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hCsvFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Close file
    f.close();

    // All CSV file read...check if need to update the CSV Parameter list on disk?
    if(bNewCsvParameterFound == true)
        DumpParameterIndexTable();

    // Success parsing CSV file
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
int	CGCSVtoSTDF::ResultDisplayScaleFactor(unsigned uIndex)
{
    // If no units defined, do not rescale.
    if(pCGCsvParameter[uIndex].strUnits.isEmpty())
        return 0;

    // If no smart-scaling enabled, return!
    //if(!ReportOptions.bStatsSmartScaling)
    //	return 0;

    // Do not customize scaling if units are not normalized.
    char *ptChar = (char *) pCGCsvParameter[uIndex].strUnits.toLatin1().constData();
    switch(*ptChar)
    {
    case 'm':
    case 'M':
    case 'K':
    case 'G':
    case 'T':
    case 'u':
    case 'n':
    case 'p':
    case 'f':
        return 0;
    }

    int	iExponent = 0;

    if (nUnitsMode == GEX_OPTIONS_TOOLBOX_UNITS_NORMALIZED)
    {
        // Check the limits
        float	fLowLimit=0;
        float	fHighLimit=0;

        if(pCGCsvParameter[uIndex].bValidLowLimit)
            fLowLimit = fabs(pCGCsvParameter[uIndex].fLowLimit);

        if(pCGCsvParameter[uIndex].bValidHighLimit)
            fHighLimit = fabs(pCGCsvParameter[uIndex].fHighLimit);

        float fMiddle = (fHighLimit + fLowLimit)/2;
        if(!fMiddle)
            return	0;

        int	iExponent= (int) log10(fMiddle);	// Get power of 10 for the middle point of the limits in absolute value
        if(iExponent <= -13)
            return -15;	// Fento
        if(iExponent >= 9)
            return 9;	// Tera
    }
    else
        iExponent = pCGCsvParameter[uIndex].lScale;

    switch(iExponent)
    {
    case -12:
    case -11:
    case -10:
        return 12;	// 'n'
    case -9:
    case -8:
    case -7:
        return 9;	// 'p'
    case -6:
    case -5:
    case -4:
        return 6;	// 'u'
    case -1:
    case -2:
    case -3:
        return 3;	// 'm'

    default:
    case 0:
    case 1:
    case 2:
        return 0;

    case 3:
    case 4:
    case 5:
        return -3;	// K
    case 6:
    case 7:
    case 8:
        return -6;	// M
    }
}

//////////////////////////////////////////////////////////////////////
// Write SDR records in stdf file
/// GS::StdLib::StdfRecordReadInfo must be associated with StdfFile !!
//////////////////////////////////////////////////////////////////////
bool CGCSVtoSTDF::WriteSdrRecords(GS::StdLib::Stdf & csRefStdfFile)
{
    for(int ii=0; ii<m_qlCssv4SdrList.count(); ii++)
    {
        GQTL_STDF::Stdf_SDR_V4 cssv4SDRRecord = m_qlCssv4SdrList.at(ii);
        if(!(cssv4SDRRecord.Write(csRefStdfFile)))
            return false;
    }

    // everything went well !
    return true;
}



//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool CGCSVtoSTDF::WriteStdfFile(QTextStream *hCsvFile, const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    StdfFile.SetStdfCpuType(StdfFile.GetComputerCpuType());

    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(StdfFile.GetComputerCpuType());	// Force CPU type to current computer platform.
    StdfFile.WriteByte(4);					// STDF V4
    StdfFile.WriteRecord();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(lSetupTime);			// Setup time
    StdfFile.WriteDword(lStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    //StdfFile.WriteByte((BYTE) ' ');				// rtst_cod // case 4059
    StdfFile.WriteByte(byRtstCode);             // rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(iBurninTime);			// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(strLotID.toLatin1().constData());			// Lot ID
    StdfFile.WriteString(strProductID.toLatin1().constData());		// Part Type / Product ID
    StdfFile.WriteString(strTesterName.toLatin1().constData());		// Node name
    StdfFile.WriteString(strTesterType.toLatin1().constData());		// Tester Type
    StdfFile.WriteString(strProgramName.toLatin1().constData());	// Job name
    StdfFile.WriteString(strProgramRev.toLatin1().constData());		// Job rev
    StdfFile.WriteString(strSubLotID.toLatin1().constData());		// sublot-id
    StdfFile.WriteString(strOperator.toLatin1().constData());		// operator
    StdfFile.WriteString(strExecType.toLatin1().constData());		// exec-type
    StdfFile.WriteString(strExecRev.toLatin1().constData());		// exe-ver
    StdfFile.WriteString(strTestCode.toLatin1().constData());		// test-cod
    StdfFile.WriteString(strTemperature.toLatin1().constData());	// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":CSV";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");										// aux-file
    StdfFile.WriteString(strPackageType.toLatin1().constData());	// package-type
    StdfFile.WriteString(strFamilyID.toLatin1().constData());		// familyID
    StdfFile.WriteString("");										// Date-code
    StdfFile.WriteString(strFacilityID.toLatin1().constData());		// Facility-ID
    StdfFile.WriteString(strFloorID.toLatin1().constData());		// FloorID
    StdfFile.WriteString(strProcessID.toLatin1().constData());		// ProcessID
    StdfFile.WriteString(strFrequencyStep.toLatin1().constData());	// Frequency/Step
    StdfFile.WriteString("");                                       // Spec_Nam
    StdfFile.WriteString(mSpecVer.toLatin1().constData());          // Spec_Ver
    StdfFile.WriteString(mFlowId.toLatin1().constData());           // Flow_Id
    StdfFile.WriteString(mSetupId.toLatin1().constData());          // Setup_Id
    StdfFile.WriteString(mDsgnRev.toLatin1().constData());          // Dsgn_Rev
    StdfFile.WriteString(mEngId.toLatin1().constData());            // Eng_Id
    StdfFile.WriteString("");                                       // Rom_cod
    StdfFile.WriteString(mSerlNum.toLatin1().constData());          // Serl_Num
    StdfFile.WriteString(mSuprNam.toLatin1().constData());          // Supr_Nam


    StdfFile.WriteRecord();

    // WCR + WIR (if WaferID was defined)
    if(strWaferID.isEmpty() == false)
    {
        // WCR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 30;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteFloat(m_lfWaferDiameter);			// Wafer diameter
        StdfFile.WriteFloat(m_lfWaferDieHeight);		// Height of die
        StdfFile.WriteFloat(m_lfWaferDieWidth);			// Width of die
        StdfFile.WriteByte(m_cWaferUnits);				// Units for diameter, height of die and  width of die
        StdfFile.WriteByte(m_cWaferFlat);				// Wafer flat orientation
        StdfFile.WriteWord(m_iWaferCenterDieX);			// X coordinate of center die on wafer
        StdfFile.WriteWord(m_iWaferCenterDieY);			// Y coordinate of center die on wafer
        StdfFile.WriteByte(m_cWaferPosX);				// Positive X direction of wafer
        StdfFile.WriteByte(m_cWaferPosY);				// Positive Y direction of wafer
        StdfFile.WriteRecord();

        // WIR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);				// head#
        StdfFile.WriteByte(255);			// group# (all groups)
        StdfFile.WriteDword(lStartTime);	// Setup time
        StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID.
        StdfFile.WriteRecord();
    }

    // Write SDR
    if(!WriteSdrRecords(StdfFile))
    {
        StdfFile.Close();
        return false;
    }

    // Write Test results for each line read.
    QStringList strCells;
    QString strString;
    QString strSection;
    float	fValue;				// Used for readng floating point numbers.
    unsigned uIndex;				// Loop index
    BYTE	bData;
    WORD	wSoftBin,wHardBin;
    // BG 23/07/2004 : added iData
    int				iDieX,iDieY,iSite,iExecTime,iData;
    long			iTotalGoodBin,iTotalFailBin;
    unsigned long	lTestNumber;
    long			iTotalTests,iPartNumber;
    bool			bStatus,bPassStatus,bValue;
    int				iRes_scale;

    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    while(hCsvFile->atEnd() == false)
    {
        // Read line
        strString = ReadLine(*hCsvFile);
        // ignore empty lines
        if(strString.isEmpty())
            continue;

        // Part number
        iPartNumber++;

        // Pass/Fail flag.
        bPassStatus = true;

        // Reset counters
        iTotalTests = 0;

        // Split line
        strCells = strString.split(",",QString::KeepEmptyParts);

        // If CSV file include binning,Die location data, extract them...
        // If CSV file doesn't include Binning,Die location info...use defaults.
        iDieX = iDieY = -32768;
        iExecTime = 0;
        wSoftBin = 65535;
        wHardBin = 0;
        iSite = 1;
        uIndex = 0;
        if(bEditedDataFile)
        {
            // BG 23/07/2004 : only read valid cells
            // CSV file includes Binning,Die location info...extract them.
            if(strCells.count() > CSV_RAW_SBIN+1)
            {
                iData = strCells[CSV_RAW_SBIN+1].toInt(&bStatus);
                if(bStatus == true)
                    wSoftBin = wHardBin = iData;	// Force hard bin = soft bin (in case hard bin not defined!)
            }
            if(strCells.count() > CSV_RAW_HBIN+1)
            {
                iData = strCells[CSV_RAW_HBIN+1].toInt(&bStatus);
                if(bStatus == true)
                    wHardBin = iData;
            }
            if(strCells.count() > CSV_RAW_DIEX+1)
            {
                iData = strCells[CSV_RAW_DIEX+1].toInt(&bStatus);
                if(bStatus == true)
                    iDieX = iData;
            }
            if(strCells.count() > CSV_RAW_DIEY+1)
            {
                iData = strCells[CSV_RAW_DIEY+1].toInt(&bStatus);
                if(bStatus == true)
                    iDieY = iData;
            }
            if(strCells.count() > CSV_RAW_SITE+1)
            {
                iData = strCells[CSV_RAW_SITE+1].toInt(&bStatus);
                if(bStatus == true)
                    iSite = iData;
            }
            if(strCells.count() > CSV_RAW_TIME+1)
            {
                // case 4059
                //iData = strCells[CSV_RAW_TIME+1].toInt(&bStatus);
                //					if(bStatus == true)
                //						iExecTime = iData;
                QString strStockData = strCells[CSV_RAW_TIME+1];
                double lStockData = strStockData.toDouble(&bStatus);
                if(bStatus == true)
                {
                    iData = qRound(qreal(lStockData*1000)); // *1000 cause of sec. in csv and ms in stdf
                    iExecTime = iData;
                }
            }
            // Read Parameter from first column AFTER last advanced field.
            uIndex = m_lStartParsingOffset;
        }

        // BG 23/07/2004 : add PIR
        // Write PIR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(iSite);		// Tester site#
        StdfFile.WriteRecord();

        // Read Parameter results for this record
        while(uIndex < uTotalParameters &&
              uIndex + 1 < (unsigned int) strCells.count())
        {
            // If it's a PTR (pattern name empty)
            if (pCGCsvParameter[uIndex].strPatternName.isEmpty())
            {
                // Valid test result...write the PTR
                iTotalTests++;

                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                // Compute Test# (add user-defined offset)
                if((long)pCGCsvParameter[uIndex].lTestNumber < 0)
                {
                    lTestNumber = (long) pFullCsvParametersList.indexOf(pCGCsvParameter[uIndex].strName);
                    lTestNumber += GEX_TESTNBR_OFFSET_CSV;		// Test# offset
                    pCGCsvParameter[uIndex].lTestNumber = lTestNumber;	// Save Test number for future runs.
                }
                else
                    lTestNumber = pCGCsvParameter[uIndex].lTestNumber;

                StdfFile.WriteDword(lTestNumber);			// Test Number
                StdfFile.WriteByte(1);						// Test head

                // Write Site# (if known, or 1 otherwise)
                StdfFile.WriteByte(iSite);				// Tester site#

                fValue = strCells[uIndex+1].trimmed().toFloat(&bStatus);
                if(bStatus == true)
                {
                    fValue = normalizeTestResult(fValue, pCGCsvParameter[uIndex].lScale);
                    if(((pCGCsvParameter[uIndex].bValidLowLimit==true) && (fValue < pCGCsvParameter[uIndex].fLowLimit)) ||
                            ((pCGCsvParameter[uIndex].bValidHighLimit==true) && (fValue > pCGCsvParameter[uIndex].fHighLimit)))
                    {
                        bData = 0200;	// Test Failed
                        bPassStatus = false;
                    }
                    else
                    {
                        bData = 0;		// Test passed
                    }
                    StdfFile.WriteByte(bData);							// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                    StdfFile.WriteFloat(fValue);						// Test result
                }
                else
                {
                    StdfFile.WriteByte(0x12);							// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                    StdfFile.WriteFloat(0);                             // Test result
                }
                StdfFile.WriteString(pCGCsvParameter[uIndex].strName.toLatin1().constData());	// TEST_TXT

                // Write static part (limits + test name) ?
                if(pCGCsvParameter[uIndex].bStaticHeaderWritten == false)
                {
                    StdfFile.WriteString("");							// ALARM_ID
                    bData = 2;	// Valid data.
                    if(pCGCsvParameter[uIndex].bValidLowLimit==false)
                        bData |=0x40;
                    if(pCGCsvParameter[uIndex].bValidHighLimit==false)
                        bData |=0x80;
                    StdfFile.WriteByte(bData);							// OPT_FLAG
                    iRes_scale = ResultDisplayScaleFactor(uIndex);
                    StdfFile.WriteByte(iRes_scale);						// RES_SCALE
                    StdfFile.WriteByte(0);								// LLM_SCALE
                    StdfFile.WriteByte(0);								// HLM_SCALE
                    StdfFile.WriteFloat(pCGCsvParameter[uIndex].fLowLimit);			// LOW Limit
                    StdfFile.WriteFloat(pCGCsvParameter[uIndex].fHighLimit);		// HIGH Limit
                    StdfFile.WriteString(pCGCsvParameter[uIndex].strUnits.toLatin1().constData());	// Units
                    pCGCsvParameter[uIndex].bStaticHeaderWritten = true;
                }
                StdfFile.WriteRecord();
            }	// Valid test result
            // Else if it's FTR (pattern name not empty)
            else if (!pCGCsvParameter[uIndex].strPatternName.isEmpty())
            {

                bValue = strCells[uIndex+1].trimmed().toInt(&bStatus);
                if(bStatus == true)
                {
                    // Valid test result...write the FTR
                    iTotalTests++;

                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 20;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    // Compute Test# (add user-defined offset)
                    if((long)pCGCsvParameter[uIndex].lTestNumber < 0)
                    {
                        lTestNumber = (long) pFullCsvParametersList.indexOf(pCGCsvParameter[uIndex].strName);
                        lTestNumber += GEX_TESTNBR_OFFSET_CSV;		// Test# offset
                        pCGCsvParameter[uIndex].lTestNumber = lTestNumber;	// Save Test number for future runs.
                    }
                    else
                        lTestNumber = pCGCsvParameter[uIndex].lTestNumber;

                    bValue = normalizeTestResult(bValue, pCGCsvParameter[uIndex].lScale);

                    StdfFile.WriteDword(lTestNumber);			// Test Number
                    StdfFile.WriteByte(1);						// Test head

                    // Write Site# (if known, or 1 otherwise)
                    StdfFile.WriteByte(iSite);				// Tester site#

                    if(bValue == 0)
                        bData = 0x80;		// Test Failed
                    else
                        bData = 0x00;		// Test passed

                    StdfFile.WriteByte(bData);							// TEST_FLG
                    bData = 0xf7;
                    StdfFile.WriteByte(bData);							// OPT_FLG
                    StdfFile.WriteDword(0);								// CYCL_CNT
                    StdfFile.WriteDword(0);								// REL_VADR
                    StdfFile.WriteDword(0);								// REPT_CNT

                    if(bValue == 0)
                        StdfFile.WriteDword(1);							// NUM_FAIL
                    else
                        StdfFile.WriteDword(0);							// NUM_FAIL

                    StdfFile.WriteDword(0);								// XFAIL_AD
                    StdfFile.WriteDword(0);								// YFAIL_AD
                    StdfFile.WriteWord(0);								// VECT_OFF

                    StdfFile.WriteWord(0);								// RTN_ICNT
                    StdfFile.WriteWord(0);								// PGM_ICNT

                    // We don't handle that data for the moment --> 4 empty fields
                    // RTN_INDX
                    // RTN_STAT
                    // PGM_INDX
                    // PGM_STAT

                    StdfFile.WriteDBitField(0, NULL);					// FAIL_PIN
                    QString strValue;
                    if (pCGCsvParameter[uIndex].strPatternName == "n/a")
                        strValue = "";
                    else
                        strValue = pCGCsvParameter[uIndex].strPatternName;
                    StdfFile.WriteString(strValue.toLatin1().constData());	// VECT_NAM



                    StdfFile.WriteString("");							// TIME_SET (not handle)
                    StdfFile.WriteString("");							// OP_CODE	(not handle)
                    StdfFile.WriteString(pCGCsvParameter[uIndex].strName.toLatin1().constData());	// TEST_TXT
                    StdfFile.WriteString("");							// ALARM_ID	(not handle)
                    StdfFile.WriteString("");							// PROG_TXT	(not handle)
                    StdfFile.WriteString("");							// RSLT_TXT	(not handle)
                    StdfFile.WriteRecord();
                }
            }

            // Move to next parameter column to read
            uIndex++;
        };	// Read all results on line

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        // Tester site#
        StdfFile.WriteByte(iSite);	// Tester site#
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);				// PART_FLG : PASSED
            // Create a binning result unless it is already in the CSV file...
            if(bEditedDataFile == false)
                wSoftBin = wHardBin = 1;
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte(8);				// PART_FLG : FAILED
            // Create a binning result unless it is already in the CSV file...
            if(bEditedDataFile == false)
                wSoftBin = wHardBin = 0;
            iTotalFailBin++;
        }

        // BG 23/07/2004 : should we force a nb of tests for the wafermap to be correct????
        if(bEditedDataFile && (iTotalTests == 0))
            iTotalTests = 1;
        StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
        StdfFile.WriteWord(wHardBin);           // HARD_BIN
        StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
        StdfFile.WriteWord(iDieX);				// X_COORD
        StdfFile.WriteWord(iDieY);				// Y_COORD
        StdfFile.WriteDword(iExecTime);			// Testing time (if known, otherwise: 0)
        strSection = strString.section(',',0,0);
        // Extract integer# from string (if possible)

        // case 4059, PYC
        if(strSection.startsWith(QString("PID-")))
            strSection = strSection.section(QString("PID-"),1,-1);
        else
            strSection = QString("");
        StdfFile.WriteString(strSection.toLatin1().constData());	// PART_ID
        StdfFile.WriteRecord();

        // Advanced CSV file, then create accurate HBIN and SBIN summaries.
        if(bEditedDataFile == true)
        {
            // Update SBIN count only if valid SBIN!
            if(wSoftBin != 65535)
            {
                if(SoftBinMap.contains(wSoftBin) == false)
                {
                    // First time we meet this SoftBin...create its structure!
                    struct CBinCell *ptCell;
                    ptCell = new CBinCell;
                    ptCell->lBinCount = 1;
                    ptCell->lBinIndex = wSoftBin;
                    SoftBinMap[wSoftBin] = ptCell;
                }
                else
                    SoftBinMap[wSoftBin]->lBinCount++;
            }

            // Update HBIN count (HBIN cannot be invalid)!
            if(HardBinMap.contains(wHardBin) == false)
            {
                // First time we meet this SoftBin...create its structure!
                struct CBinCell *ptCell;
                ptCell = new CBinCell;
                ptCell->lBinCount = 1;
                ptCell->lBinIndex = wHardBin;
                HardBinMap[wHardBin] = ptCell;
            }
            else
                HardBinMap[wHardBin]->lBinCount++;
        }
    }	// Read all lines in file

    // WRR (if WaferID was defined)
    if(strWaferID.isEmpty() == false)
    {
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test Head
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteDword(lFinishTime);			// File finish-time.
        StdfFile.WriteDword(iPartNumber);			// Total Parts tested
        StdfFile.WriteDword(0);						// Total Parts re-tested
        StdfFile.WriteDword(0);						// Total Parts aborted
        StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
        StdfFile.WriteRecord();
    }

    // If no binning data given in the CSV file, write our extrapolation.
    if(bEditedDataFile == false)
    {
        // Write SBR Bin0 (FAIL)
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 50;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);					// Test Head = ALL
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteWord(0);						// SBIN = 0
        StdfFile.WriteDword(iTotalFailBin);			// Total Bins
        StdfFile.WriteByte('F');
        StdfFile.WriteRecord();

        // Write SBR Bin1 (PASS)
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 50;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);					// Test Head = ALL
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteWord(1);						// SBIN = 1
        StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
        StdfFile.WriteByte('P');
        StdfFile.WriteRecord();

        // Write HBR Bin0 (FAIL)
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 40;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);					// Test Head = ALL
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteWord(0);						// HBIN = 0
        StdfFile.WriteDword(iTotalFailBin);			// Total Bins
        StdfFile.WriteByte('F');
        StdfFile.WriteRecord();

        // Write HBR Bin1 (PASS)
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 40;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);					// Test Head = ALL
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteWord(1);						// HBIN = 1
        StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
        StdfFile.WriteByte('P');
        StdfFile.WriteRecord();
    }
    else
    {
        // Check if list of PASS bins was specified, otherwise, consider Bin1 as PASS
        if(strlPassSBins.count() == 0)
            strlPassSBins.append("1");
        if(strlPassHBins.count() == 0)
            strlPassHBins.append("1");

        // Create SBR list.
        char cPassFailStatus;
        CSoftBinMap::iterator sit;
        for ( sit = SoftBinMap.begin(); sit != SoftBinMap.end(); ++sit )
        {
            if(strlPassSBins.indexOf(QString::number(sit.value()->lBinIndex)) != -1)
                cPassFailStatus = 'P';
            else
                cPassFailStatus = 'F';
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 50;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(255);					// Test Head = ALL
            StdfFile.WriteByte(255);					// Test sites = ALL
            StdfFile.WriteWord(sit.value()->lBinIndex);	// SBIN#
            StdfFile.WriteDword(sit.value()->lBinCount);	// SBIN count
            StdfFile.WriteByte(cPassFailStatus);
            StdfFile.WriteString(sit.value()->strBinName.toLatin1().constData());	// SBIN name
            StdfFile.WriteRecord();
            // delete cell created.
            delete sit.value();
        }
        // Create HBR list.
        CHardBinMap::iterator hit;
        for ( hit = HardBinMap.begin(); hit != HardBinMap.end(); ++hit )
        {
            if(strlPassHBins.indexOf(QString::number(hit.value()->lBinIndex)) != -1)
                cPassFailStatus = 'P';
            else
                cPassFailStatus = 'F';
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 40;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(255);					// Test Head = ALL
            StdfFile.WriteByte(255);					// Test sites = ALL
            StdfFile.WriteWord(hit.value()->lBinIndex);	// HBIN#
            StdfFile.WriteDword(hit.value()->lBinCount);	// HBIN count
            StdfFile.WriteByte(cPassFailStatus);
            StdfFile.WriteString(hit.value()->strBinName.toLatin1().constData());	// HBIN name
            StdfFile.WriteRecord();
            // delete cell created.
            delete hit.value();
        }
    }

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test Head
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iPartNumber);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(lFinishTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// If CSV file with multiple lots, slit it in smaller temporary CSV files (one lot per CSV)
//////////////////////////////////////////////////////////////////////
bool CGCSVtoSTDF::SplitLots(QString CsvFileName,QStringList	&strSplitFiles)
{
    QString strString;

    // Open CSV file
    QFile fInputFile(CsvFileName);
    if(!fInputFile.open( QIODevice::ReadOnly ))
        return false;	// No split done

    // Assign file I/O stream
    QTextStream hCsvFile(&fInputFile);

    // For ProgressBar
    iFileSize = fInputFile.size()+1;

    // Check if valid CSV header...extract header data.
    bool	bMultiLots=false;
    do
    {
        strString = ReadLine(hCsvFile);
        if(strString.startsWith("<end_dataset>", Qt::CaseInsensitive))
        {
            bMultiLots = true;
            break;	// Exit loop
        }
    }
    while(hCsvFile.atEnd() == false);

    // If only one lot in this file, then no split required
    if(bMultiLots == false)
    {
        // Close file
        fInputFile.close();
        return false;
    }

    // Rewinf CSV input file
    hCsvFile.seek(0);
    strSplitFiles.clear();

    // Split file into multiple CSV files.
    QDir		cDir;
    QString		strSplitFile;
    QFileInfo	cFileInfo(CsvFileName);
    QFile		fSplitFile;
    QTextStream	hSplitFile;
    int			iSplitNumber=0;

    // Make sure the database /.temp/ folder exists!
    //strSplitFile = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_TEMP_FOLDER;
    strSplitFile = GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator();
    cDir.mkdir(strSplitFile);

    while(hCsvFile.atEnd() == false)
    {
        // Keeps track of total split files created
        iSplitNumber++;

        // Open split file
        //strSplitFile = pDatabaseEntry->PhysicalPath() + GEX_DATABASE_TEMP_FOLDER + cFileInfo.baseName(true) + "_split_" + QString::number(iSplitNumber) + ".csv";
        strSplitFile = GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator()
                + cFileInfo.baseName() + "_split_" + QString::number(iSplitNumber) + ".csv";
        fSplitFile.setFileName(strSplitFile);
        if(!fSplitFile.open( QIODevice::WriteOnly ))
            return false;	// Failed creating split file
        hSplitFile.setDevice(&fSplitFile);

        // Read input CSv until reach 'end of lot' marker
        bool isEmptyDataset = true;
        do
        {
            strString = ReadLine(hCsvFile);
            if(strString.startsWith("<end_dataset>", Qt::CaseInsensitive))
            {
                if(isEmptyDataset == false)
                {
                    bMultiLots = true;
                    break;	// Exit loop: close this split file
                }
                //else dataset is empty, do nothing and we will continue on with next dataset
            }
            else
            {
                isEmptyDataset = false;
                hSplitFile << strString << endl;
            }
        }
        while(hCsvFile.atEnd() == false);
        fSplitFile.close();

        // This temporary CSV split file will have to be erased when Examinator closes...ecause we only keep the STDF version created from it!
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strSplitFile, TemporaryFile::BasicCheck);

        // Add this new file to the list of files to insert in the database
        strSplitFiles += strSplitFile;
    };

    // Close file
    fInputFile.close();

    // Successfuly split CSV file.
    return true;
}

bool CGCSVtoSTDF::Convert(const char *CsvFileName,
                          const char *strFileNameSTDF,
                          bool bAllowExtendedCsv,
                          bool bMonitoring,
                          bool bAllowWizard)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(CsvFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    if (!UpdateOptions(&ReportOptions))
        GSLOG(SYSLOG_SEV_DEBUG, " error while updating options !");

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;

    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;
    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(100);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(CsvFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();


    if(ReadCsvFile(CsvFileName,strFileNameSTDF,bAllowExtendedCsv) == true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return true;	// Convertion successful
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
            && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
            && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    // Convertion failed...then need to display the Custom Import form (unless we're running in monitoring mode)
    //    if(bMonitoring) *5960*
    //        return false;	// Error reading CSV file

    // Check if Parsing file available...if so, allow using it!
    CGCSV_WizardtoSTDF cImportWizard;
    QString strParserConfig;
    bool	bValidParsingConfig = cImportWizard.isParsingConfigAvailable(CsvFileName,strParserConfig);

    if((bAllowWizard == false || bMonitoring) && (bValidParsingConfig == false))
    {
        return false;
    }

    // We're in GUI mode...or we have a parser config file defined, so we can launch the import..
    return cImportWizard.Convert(CsvFileName,strFileNameSTDF,strParserConfig);
}

bool CGCSVtoSTDF::UpdateOptions(CReportOptions* ro)
{
    if (!ro)
        return false;

    bool ok = true;	// set this to false if you detect an GetOption() error

    // Duplicate test
    QString strTestMergeRule = ro->GetOption("dataprocessing","duplicate_test").toString();
    if (strTestMergeRule == "merge")
        m_eTestMergeRule = MERGE_TEST_NUMBER;
    else if (strTestMergeRule == "merge_name")
        m_eTestMergeRule = MERGE_TEST_NAME;
    else if (strTestMergeRule == "no_merge")
        m_eTestMergeRule = NEVER_MERGE_TEST;
    else
    {
        ok=false;
        GSLOG(SYSLOG_SEV_WARNING, QString(" error : unknown option '%1' for 'dataprocessing::duplicate_test' ")
              .arg( strTestMergeRule).toLatin1().constData());
        WriteDebugMessageFile(QString("CGCSVtoSTDF::UpdateOptions : warning : unknown option '%s' for 'dataprocessing::duplicate_test'' !").arg(strTestMergeRule));
    }

    return ok;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGCSVtoSTDF::ReadLine(QTextStream& hFile, bool aLetEmptyValues /*= false*/)
{
    QString strString;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL && iFileSize > 0)
    {
        while((int) hFile.device()->pos() > iNextFilePos)
        {
            iProgressStep += 100/iFileSize + 1;
            iNextFilePos  += iFileSize/100 + 1;
            GexProgressBar->setValue(iProgressStep);
        }
    }
    QCoreApplication::processEvents();

    do
    {
        strString = hFile.readLine().trimmed();
        // Skip empty CVS line
        if(strString.left(3) == ",,," && (strString.simplified().count(",")==strString.simplified().length()))
            strString = "";
    }
    while(!strString.isNull() && strString.isEmpty());

    // Remove the last comma if any
    while(strString.endsWith(",") && !aLetEmptyValues)
        strString = strString.left(strString.length()-1).trimmed();

    return strString;

}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGCSVtoSTDF::normalizeTestUnits(const QString& strTestUnits, long& lScaleFactor) const
{
    QString strNormalizedTestUnits = strTestUnits;

    // If units are not mormalized, then do it
    if(nUnitsMode == GEX_OPTIONS_TOOLBOX_UNITS_SCALING_FACTOR && strTestUnits.size() > 1)
    {
        switch(strTestUnits[0].toLatin1())
        {
        case 'K' :	lScaleFactor = 3;
            break;

        case 'M' :	lScaleFactor = 6;
            break;

        case 'G' :	lScaleFactor = 9;
            break;

        case 'T' :	lScaleFactor = 12;
            break;

        case 'm' :	lScaleFactor = -3;
            break;

        case 'u' :	lScaleFactor = -6;
            break;

        case 'n' :	lScaleFactor = -9;
            break;

        case 'p' :	lScaleFactor = -12;
            break;

        case 'f' :	lScaleFactor = -15;
            break;

        case '%' :	lScaleFactor = -2;
            break;

        default  :	lScaleFactor = 0;
            break;
        }

        if(lScaleFactor !=0 )
            strNormalizedTestUnits = strTestUnits.right(strTestUnits.count()-1);
    }

    return strNormalizedTestUnits;
}

double CGCSVtoSTDF::normalizeTestResult(double dValue, long lScaleFactor) const
{
    double dNormalizedTestResult = dValue;

    // If results are not mormalized, then do it
    if(nUnitsMode == GEX_OPTIONS_TOOLBOX_UNITS_SCALING_FACTOR)
    {
        switch(lScaleFactor)
        {
        case -3		:	dNormalizedTestResult *= 1e-3;
            break;

        case -6		:	dNormalizedTestResult *= 1e-6;
            break;

        case -9		:	dNormalizedTestResult *= 1e-9;
            break;

        case -12	:	dNormalizedTestResult *= 1e-12;
            break;

        case -15	:	dNormalizedTestResult *= 1e-15;
            break;

        case 2		:	dNormalizedTestResult *= 1e2;
            break;

        case 3		:	dNormalizedTestResult *= 1e3;
            break;

        case 6		:	dNormalizedTestResult *= 1e6;
            break;

        case 9		:	dNormalizedTestResult *= 1e9;
            break;

        case 12		:	dNormalizedTestResult *= 1e12;
            break;

        default		:	break;
        }
    }

    return dNormalizedTestResult;
}


//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool CGCSVtoSTDF::EmptyLine(const QString& line)
{
    bool empty(true);
    if (!line.isEmpty())
    {
        QStringList strCells = line.split(",", QString::KeepEmptyParts);
        foreach (const QString &strCell, strCells)
        {
            if (!strCell.isEmpty())
            {
                empty = false;
                break;
            }
        }
    }
    return empty;
}
