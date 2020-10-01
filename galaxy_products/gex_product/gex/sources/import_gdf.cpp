//////////////////////////////////////////////////////////////////////
// import_gdf.cpp: Convert a .GDF file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <qmath.h>

#ifdef _WIN32
    #include "windows.h"        // For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_gdf.h"
#include "import_constants.h"
#include "gqtl_global.h"
#include "product_info.h"
#include <gqtl_log.h>

// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar *   GexProgressBar;         // Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGGDFtoSTDF::CGGDFtoSTDF()
{
    m_pfGdf     = NULL;
    m_phGdfFile = NULL;
    m_pStdfFile = NULL;

    m_lSetupT       = 0;
    m_lStartT       = 0;
    m_nStatNum      = 1;
    m_cModeCod      = 'P';
    m_cRtstCod      = ' ';
    m_cProtCod      = ' ';
    m_lBurnTim      = 65535;
    m_cCModCod      = ' ';
    m_strLotId      = "";
    m_strPartTyp    = "";
    m_strNodeNam    = "";
    m_strTstrTyp    = "";
    m_strJobNam     = "";
    m_strJobRev     = "";
    m_strSbLotId    = "";
    m_strOperNam    = "";
    m_strExecTyp    = "";
    m_strExecVer    = "";
    m_strTestCod    = "";
    m_strTstTemp    = "";
    m_strUserTxt    = "";
    m_strAuxFile    = "";
    m_strPckTyp     = "";
    m_strFamlyId    = "";
    m_strDateCod    = "";
    m_strFacilId    = "";
    m_strFloorId    = "";
    m_strProcId     = "";
    m_strOperFrq    = "";
    m_strSpecNam    = "";
    m_strSpecVer    = "";
    m_strFlowId     = "";
    m_strSetupId    = "";
    m_strDsgnRev    = "";
    m_strEngId      = "";
    m_strRomCod     = "";
    m_strSerlNum    = "";
    m_strSuprNam    = "";
    m_strHandType   = "";
    m_strHandId     = "";
    m_strCardId     = "";
    m_strLoadId     = "";
    m_strExtrId     = "";


    m_strWaferId        = "";
    m_nWaferNumDie      = 0;
    m_nWaferNumPassed   = 0;
    m_nWaferNumRetested = 0;


    m_nTotalParts   = -1;
    m_nTotalPassed  = -1;
    m_nTotalRetested= -1;

    m_strUserDesc   = " ";
    m_strExecDesc   = " ";
    m_nDispCode     = 0;

    m_lLastPartTime = 0;
    m_lstSites.clear();

    m_bMirWriten    = false;
    m_nPass         = 1;
    m_bGotoNextPass = false;

    m_nMaxTestNum   = -1;
    m_clGdfTestResult.test_num = 1;

    m_bAcceptMprDynamicPins = false;
}

CGGDFtoSTDF::~CGGDFtoSTDF()
{
    m_clGdfDutResult.reset();
    m_clGdfSummary.reset();
    if (m_clGdfSummary.pSite)
        delete m_clGdfSummary.pSite;
    m_clGdfPartCnt.reset();
    m_clGdfTestResult.reset();
    while(!m_clSbrList.isEmpty())
        delete m_clSbrList.takeFirst();
    QMap<int,CGGdfPinChannel*>::Iterator itPinChannel;
    for ( itPinChannel = m_mapPinChannel.begin(); itPinChannel != m_mapPinChannel.end(); ++itPinChannel )
    {
        if (itPinChannel.value())
            delete itPinChannel.value();
    }
    m_mapPinChannel.clear();
    m_mapPinNum.clear();
    QMap<QString,CGGdfTest*>::Iterator itTest;
    for ( itTest = m_mapTest.begin(); itTest != m_mapTest.end(); ++itTest )
    {
        delete itTest.value();
    }
    m_mapTest.clear();
    m_mapVarValue.clear();
    m_bin_name_mapping.clear();
    mBinNonNumber.clear();
    m_mapSoftBinDesc.clear();
}

QString CGGDFtoSTDF::GetLastError()
{
    m_strLastError = "Import GDF: ";

    switch(m_iLastError)
    {
    default:
    case errNoError:
        m_strLastError += "No Error ";
        break;
    case errWarningEndOfFile:
        m_strLastError += "*WARNING* \nUnexpected end of file. \n";
        m_strLastError += "This file may be truncated. \n";
        if(!m_stackMarkerTag.isEmpty() && !m_stackMarkerLine.isEmpty())
            m_strLastError += "[some data was ignored after "+m_stackMarkerTag.first()+" at line "+m_stackMarkerLine.first()+"] ";
        break;
    case errErrorOpenFail:
        m_strLastError += "Failed to open file ";
        break;
    case errErrorInvalidFormat:
        m_strLastError += "Invalid file format \n";
        if(!m_stackMarkerTag.isEmpty() && !m_stackMarkerLine.isEmpty())
            m_strLastError += "[near "+m_stackMarkerTag.first()+" at line "+m_stackMarkerLine.first()+"] \n";
        break;
    case errErrorEndOfFile:
        m_strLastError += "Invalid file format: Unexpected end of file \n";
        if(!m_stackMarkerTag.isEmpty() && !m_stackMarkerLine.isEmpty())
            m_strLastError += "[near "+m_stackMarkerTag.first()+" at line "+m_stackMarkerLine.first()+"] \n";
        break;
    case errErrorWriteSTDF:
        m_strLastError += "Failed creating temporary file. Folder permission issue? ";
        break;
    }
    if(!m_strLastErrorMsg.isEmpty())
        m_strLastError += " \n"+m_strLastErrorMsg;

    // Return error message
    return m_strLastError;
}

bool CGGDFtoSTDF::IsCompatible(const char *szFileName)
{
    QFileInfo cFileInfo(szFileName);
    // Only check if have the good extension !!!
    if(!(cFileInfo.suffix().left(3).toLower() == "gdf"))
        return false;

    // and then check if have some begin_ tag
    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    QTextStream hGdfFile(&f);

    // Find the correct Gdf header in the 100 first lines ...
    QString strLine;
    int     iPos;

    for(int i=0; i!=100; i++)
    {
        strLine = hGdfFile.readLine();
        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 60))
        {
            // if found a ':', probably a keyword
            // remove all space
            strLine = strLine.left(iPos).remove(' ').remove('\t').toLower();
            if(strLine.isEmpty() || (strLine.left(2) == "--"))
                continue;

            // Verify if found a key word
            if(strLine.left(6) == "begin_")
            {
                f.close();
                return true;
            }
            else if(strLine.left(3) == "bin")
            {
                // ignore lines like
                // Binlist: lot
                // Bin:     28  no_reprobe  16  FAIL    VDD_SHORT
                // some files can have a lot of bin

                i--;
            }
        }
    }

    // Incorrect header...this is not a Gdf file!
    f.close();
    return false;
}

void CGGDFtoSTDF::LoadParameterIndexTable(void)
{
    QString strFileName;
    QString strString;

    strFileName  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strFileName += GEX_GDF_PARAMETERS;

    // Open Gdf Parameter table file
    QFile f( strFileName );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hGdfFile(&f);

    // Skip comment lines
    do
    {
        strString = hGdfFile.readLine();
    }
    while((strString.indexOf("----------------------") < 0) && (hGdfFile.atEnd() == false));

    // Read lines
    m_lstTestName.clear();
    strString = hGdfFile.readLine();
    while (strString.isNull() == false)
    {
        // Save Parameter name in list
        m_lstTestName.append(strString);
        // Read next line
        strString = hGdfFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save GDF Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::DumpParameterIndexTable(void)
{
    QString     strFileName;

    strFileName  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    strFileName += GEX_GDF_PARAMETERS;

    // Open Gdf Parameter table file
    QFile f( strFileName );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hGdfFile(&f);

    // First few lines are comments:
    hGdfFile << "############################################################" << endl;
    hGdfFile << "# DO NOT EDIT THIS FILE!" << endl;
    hGdfFile << "# Quantix Examinator: GDF Parameters detected" << endl;
    hGdfFile << "# www.mentor.com" << endl;
    hGdfFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hGdfFile << "-----------------------------------------------------------" << endl;

    // Write lines
    for (QStringList::const_iterator
         iter  = m_lstTestName.begin();
         iter != m_lstTestName.end(); ++iter) {
        // Write line
        hGdfFile << *iter << endl;
    }

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this GDF parameter in his dictionnary, have to add it.
//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::FindParameterIndex(QString strParamName)
{

    if(m_bModeDatabase)
    {
        // Check if Parameter name already in table...if not, add it to the list
        // the new full list will be dumped to the disk at the end.
        m_clGdfTestResult.test_num = m_lstTestName.indexOf(strParamName) + 1;
        if(m_clGdfTestResult.test_num == 0)
        {
            // Update list
            m_lstTestName.append(strParamName);
            m_clGdfTestResult.test_num = m_lstTestName.indexOf(strParamName) + 1;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' GDF file, to STDF 'strFileNameSTDF' file
// bModeDatabase used to ignore all test_num and used the index table saved
//////////////////////////////////////////////////////////////////////
int CGGDFtoSTDF::Convert(const char *szFileNameGdf, const char *szFileNameSTDF, bool bModeDatabase, bool bMonitoringMode)
{
    int nLstCount = 0;
    // No erro (default)
    m_iLastError = errNoError;
    m_bModeDatabase = bModeDatabase;
    m_bMonitoringMode = bMonitoringMode;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(szFileNameGdf);
    QFileInfo fOutput(szFileNameSTDF);

    QFile f( szFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return m_iLastError;

    if(m_bModeDatabase)
    {
        // Load GDF parameter table from disk...
        LoadParameterIndexTable();
        nLstCount = m_lstTestName.count();
    }

    // Check if 'GEX_FETTEST_BINMAP_FILE' environment variable is set
    char *ptChar = getenv("GEX_GDF_ACCEPT_MPR_DYNAMIC_PINS");
    if(ptChar)
        m_bAcceptMprDynamicPins = true;

    // If STDF file already exists...do not rebuild it!
    QFile fStdf(szFileNameSTDF);
    //if(fStdf.exists() == true)
    //  return true;

    // Open GDF file
    m_pfGdf = new QFile(szFileNameGdf);
    if(!m_pfGdf->open( QIODevice::ReadOnly ))
    {
        // Failed Opening GDF file
        m_iLastError = errErrorOpenFail;

        // Convertion failed.
        return m_iLastError;
    }
    // Assign file I/O stream
    m_phGdfFile = new QTextStream(m_pfGdf);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    m_iProgressStep = 0;
    m_iNextFilePos = 0;
    m_nCurrentLine = 0;
    m_iFileSize = m_pfGdf->size() + 1;

    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;
    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(200);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(szFileNameGdf).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    //////////////////////////////////////////////////////////////////////
    // First parse
    // Only read file
    // Parse the file until the first good part to have the complete flow
    // and the list of all pins name for PMR records
    m_nPass = 1;
    m_bGotoNextPass = false;


    if(ParseGdfFile() != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        if(m_iLastError == errNoError)
            m_iLastError = errErrorInvalidFormat;
        // Close file
        m_pfGdf->close();
        return m_iLastError;    // Error reading GDF file
    }


    //////////////////////////////////////////////////////////////////////
    // Second parse
    // Read gdf file and Write stdf file
    // Have collect the list of all pins name, can create STDF file with PMR records
    m_strWaferId    = "";
    m_nWaferNumDie      = 0;
    m_nWaferNumPassed   = 0;
    m_nWaferNumRetested = 0;

    m_nTotalParts   = -1;
    m_nTotalPassed  = -1;
    m_nTotalRetested= -1;

    m_bMirWriten    = false;
    m_nPass         = 2;
    m_bGotoNextPass = false;
    m_nCurrentLine = 0;
    if(GexProgressBar != NULL)
    {

        GexProgressBar->setMaximum(m_iProgressStep+100);
        // adapt ProgressStep to the new size
        m_iProgressStep = (int)((100.0+(float)m_iProgressStep)/(200.0/(float)m_iProgressStep));
        m_iNextFilePos  = 0;
    }


    //Restart at the beggining of the file
    m_phGdfFile->seek(0);
    m_strLine.clear();

    m_pStdfFile = new GS::StdLib::Stdf(); // true for mode Debug

    if(m_pStdfFile->Open((char*)szFileNameSTDF,STDF_WRITE,STDF_READWRITE_BLOCK) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing GDF file into STDF database
        m_iLastError = errErrorWriteSTDF;

        // Convertion failed.
        // Close file
        m_pfGdf->close();
        return m_iLastError;
    }

    bool bParseResult = ParseGdfFile();
    if(!bParseResult || !m_bMirWriten)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        if(m_iLastError == errNoError)
            m_iLastError = errErrorInvalidFormat;
        m_pfGdf->close();
        m_pStdfFile->Close();
        fStdf.remove(szFileNameSTDF);
        return m_iLastError;    // Error reading GDF file
    }

    m_pfGdf->close();
    m_pStdfFile->Close();
    // Convertion successful


    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
            && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
            && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    if(m_bModeDatabase && (nLstCount != m_lstTestName.count()))
        DumpParameterIndexTable();
    return m_iLastError;
}


//////////////////////////////////////////////////////////////////////
// Parse the GDF file and write STDF file
//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::ParseGdfFile()
{
    if(!ParseGdfCode())
    {
        // Check if truncated file
        if(!m_phGdfFile->atEnd())
            return false;

        // Warning
        // Check if we are in monitoring mode
        if(m_bMonitoringMode)
        {
            m_iLastError = errErrorEndOfFile;
            return false;
        }

        m_iLastError = errWarningEndOfFile;
    }
    // Success parsing GDF file

    // first verify if the MIR was writen else save it
    CheckWriteMir();

    WritePcr();
    WriteMrr();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Parse the GDF file and write STDF file
//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::ParseGdfCode()
{
    QString strLine;
    QString strString;
    QString strSection;

    int iPos;
    do
    {
        if(m_bGotoNextPass)
            return true;


        if((m_strLine.indexOf(':') > 0) && m_strLine.startsWith("data_format:", Qt::CaseInsensitive))
            strLine = m_strLine;
        else
            strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');

        if((iPos > 0) && (iPos < 60))
        {
            // if found a ':', probably a keyword
            // remove all space
            strSection = strLine.left(iPos).remove(' ').toLower();
            if(strSection.isEmpty() || (strSection.left(2) == "--"))
                continue;

            // check if have data available
            if(strLine.indexOf("not_available") > 0)
                continue;

            // remove "PH_"
            if(strSection.left(3) == "ph_")
                strSection = strSection.mid(3);

            // Verify if found a key word
            if(strSection.left(6) == "begin_")
            {
                if (strSection == "begin_data_ffc_pinlist")
                {

                    if(!AnalyzePinList("end_data_ffc_pinlist"))
                        return false;
                }
                else if (strSection == "begin_data_input_pinlist")
                {

                    if(!AnalyzePinList("end_data_input_pinlist"))
                        return false;
                }
                else if (strSection == "begin_data_pins")
                {
                    if(!AnalyzeDataPins())
                        return false;

                }
                else if (strSection == "begin_limit_data")
                {
                    if(!AnalyzeLimitData())
                        return false;
                }
                else if (strSection == "begin_user_data")
                {

                    if(!AnalyzeUserData())
                        return false;
                }
                else if(strSection == "begin_data")
                {
                    if(!GotoMarker(QString("end_data").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_data_before_retest")
                {

                    if(!GotoMarker(QString("end_data_before_retest").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_data_failed_vectors")
                {

                    if(!GotoMarker(QString("end_data_failed_vectors").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_data_ffc")
                {

                    if(!GotoMarker(QString("end_data_ffc").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_data_ffc_label")
                {

                    if(!GotoMarker(QString("end_data_ffc_label").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_data_overon")
                {

                    if(!GotoMarker(QString("end_data_overon").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_data_tf")
                {

                    if(!GotoMarker(QString("end_data_tf").toLatin1().constData()))
                        return false;
                }
                else if((strSection == "begin_pin_index_info")
                        ||  (strSection == "begin_pin_index_info_format"))
                {

                    if(!GotoMarker(QString("end_pin_index_info").toLatin1().constData()))
                        return false;
                }
                else if (strSection == "begin_shmoo_data")
                {

                    if(!GotoMarker(QString("end_shmoo_data").toLatin1().constData()))
                        return false;
                }
                else
                {
                    // unknown section
                    // skip it
                    if(!GotoMarker(QString(strSection.replace("begin_","end_")).toLatin1().constData()))
                        return false;
                }

            }
            else if (strSection == "data_format")
            {
                // found the "data_format:" keyword
                // parse the end of the line to know which info we have to collect

                strString = strLine.right(strLine.length()-iPos-1).remove(' ').toLower();

                if(strString.left(3) == "x,y")
                {
                    m_nWaferNumDie = 0;
                    m_nWaferNumPassed = 0;
                    m_nWaferNumRetested = 0;
                    // wafer info
                    m_clGdfDutResult.reset();

                    m_clGdfDutResult.nFlags = WAFER_ID;
                    m_clGdfDutResult.bSite = (strString.indexOf("site") > 0);
                    if(!AnalyzeDutResult())
                        return false;
                }
                else if(strString.left(3) == "pkg")
                {
                    // package info
                    m_clGdfDutResult.reset();

                    m_clGdfDutResult.nFlags = PACKAGE_ID;
                    m_clGdfDutResult.bSite = (strString.indexOf("site") > 0);
                    if(!AnalyzeDutResult())
                        return false;
                }
                else if(strString.left(2) == "sw")
                {
                    WriteTsr();
                    // summary info
                    m_clGdfSummary.reset();
                    m_clGdfSummary.NumberOfSite = 0;
                    if(m_clGdfSummary.pSite)
                        delete m_clGdfSummary.pSite;
                    m_clGdfSummary.pSite = NULL;
                    m_clGdfSummary.NumberOfSite = strString.count("site");

                    if(m_clGdfSummary.NumberOfSite > 0)
                        m_clGdfSummary.pSite = new float[m_clGdfSummary.NumberOfSite];

                    if(!AnalyzeSummary())
                        return false;
                }
                else if(strString.left(4) == "site")
                {
                    // summary info
                    m_clGdfPartCnt.reset();
                    m_clGdfPartCnt.bSite = true;

                    if(!AnalyzePartCnt())
                        return false;
                }
                else if(strString.left(6) == "number")
                {
                    // summary info
                    m_clGdfPartCnt.reset();
                    m_clGdfPartCnt.bSite = false;

                    if(!AnalyzePartCnt())
                        return false;
                }
            }
            else if(strSection.left(11) == "data_format")
            {
                // "data_format_ffc"
                // "data_format_ffc_label"
                // "data_format_ffc_pinlist"
                // "data_format_input_pinlist"
                // "data_format_pins"
                // "data_format_test"
            }
            else if (strSection == "bin")
            {
                // nothing to do
            }
            else if (strSection == "binlist")
            {
                // Check if Binning mapping list
                strLine = strLine.trimmed();
                if(strLine.contains("wafer",Qt::CaseInsensitive)
                        || strLine.contains("lot",Qt::CaseInsensitive))
                {
                    if(!AnalyzeSoftBinNameMapping())
                        return false;
                }
                // If PAT-Man or Yield-Man, allow to delay conversion for 8 hours max. as PROMIS file may not be updated yet.
                else if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                {
                    m_strLastErrorMsg = "Unknown 'Binlist' section";
                    m_iLastError = errErrorInvalidFormat;
                    return false;
                }

            }
            else if (strSection == "failed_pins")
            {
            }
            else if (strSection.left(8) == "wafer_id")
            {
                m_strWaferId = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if (strSection.left(8) == "test_num")
            {
                m_clGdfTestResult.test_num = strLine.right(strLine.length()-iPos-1).remove(' ').toInt();
            }
            else if((strSection.left(7) == "setup_t")||(strSection.left(7) == "start_t")||(strSection == "datetime")||(strSection == "waferdatetime"))
            {
                //01/21/07 10:10
                //20091130092112
                //20080417 052326
                //20030323_204746
                QString strDate;
                QString strTime;

                strString = strLine.right(strLine.length()-iPos-1).simplified();
                strDate = strString.left(8);
                strTime = strString.mid(8).remove(' ').remove('_').remove(':');

                // Normalize date and time
                if(strDate.indexOf("/") > 0)
                    strDate = "20"+strDate.mid(6)+strDate.remove("/").left(4);

                QDate clDate(strDate.left(4).toInt(),
                             strDate.mid(4,2).toInt(),
                             strDate.mid(6,2).toInt());
                // verify if there is 2 digit to the hour (can have ms)
                iPos = 2;
                if((strTime.length() == 5) || (strTime.length() == 7))
                    iPos = 1;
                QTime clTime(strTime.mid(0,iPos).toInt(),
                             strTime.mid(0+iPos,2).toInt(),
                             strTime.mid(2+iPos,2).toInt());
                QDateTime clDateTime(clDate, clTime);

                if(strSection.left(7) == "setup_t")
                    m_lSetupT = clDateTime.toTime_t();
                else
                {
                    m_lStartT = clDateTime.toTime_t();
                    if(m_lSetupT <= 0) m_lSetupT = m_lStartT;
                }
            }
            else if(strSection.left(8) == "stat_num")
            {
                strString = strLine.right(strLine.length()-iPos-1).remove(' ');
                strString = strString.toLower();
                m_nStatNum = strString.toInt();
            }
            else if((strSection.left(8) == "mode_cod")||(strSection.left(8) == "platform")||(strSection.left(8) == "test_group"))
            {
                strString = strLine.right(strLine.length()-iPos-1).remove(' ');
                strString = strString.toUpper();
                m_cModeCod = strString[0].toLatin1();
            }
            else if(strSection == "production_test")
            {
                strString = strLine.right(strLine.length()-iPos-1).remove(' ');
                strString = strString.toUpper();
                if(strString.indexOf("YES"))
                    strString = "PROD";
                else
                    strString = "ENG";
                m_cModeCod = strString[0].toLatin1();
            }
            else if(strSection.left(8) == "rtst_cod")
            {
                m_cRtstCod = strLine.right(strLine.length()-iPos-1)[0].toLatin1();
            }
            else if(strSection.left(8) == "prot_cod")
            {
                m_cProtCod = strLine.right(strLine.length()-iPos-1)[0].toLatin1();
            }
            else if(strSection.left(8) == "burn_tim")
            {
                strString = strLine.right(strLine.length()-iPos-1).remove(' ');
                strString = strString.toLower();
                m_lBurnTim = strString.toInt();
            }
            else if(strSection.left(8) == "cmod_cod")
            {
                m_cCModCod = strLine.right(strLine.length()-iPos-1)[0].toLatin1();
            }
            else if((strSection == "lot_id") || (strSection == "lot"))
            {
                m_strLotId = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "part_typ") || (strSection == "product_id")|| (strSection == "device_code") || (strSection == "device_name")
                    || (strSection == "part_number") || (strSection == "device_id")|| (strSection == "circuit_id"))
            {
                m_strPartTyp = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "node_nam")||(strSection == "hostname")||(strSection == "tester_id"))
            {
                m_strNodeNam = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "tstr_typ") || (strSection == "tester_type") || (strSection == "tester") || (strSection == "system_id"))
            {
                m_strTstrTyp = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(7) == "job_nam") || (strSection.left(9) == "test_prog") || (strSection.left(9) == "program_n"))
            {
                m_strJobNam = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(7) == "job_rev") || (strSection.left(7) == "job_num") || (strSection.left(8) == "test_ver"))
            {
                m_strJobRev = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(7) == "sub_lot") || (strSection.left(5) == "sblot"))
            {
                m_strSbLotId = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "oper_nam") || (strSection.left(8) == "login_na") || (strSection == "operator_id") || (strSection == "op_id"))
            {
                m_strOperNam = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "exec_typ") || (strSection == "gdf_processing_program"))
            {
                m_strExecTyp = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "exec_ver") || (strSection.left(15) == "gdf_processing_") || (strSection.left(9) == "software_"))
            {
                m_strExecVer = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "test_cod") || (strSection.left(8) == "test_des") || (strSection.left(8) == "test_mod") || (strSection.left(8) == "test_typ") || (strSection.left(8) == "data_typ") || (strSection.left(4) == "flow") || (strSection.left(8) == "test_flo"))
            {
                m_strTestCod = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if((strSection.left(8) == "tst_temp") || (strSection == "temperature"))
            {
                m_strTstTemp = strLine.right(strLine.length()-iPos-1).remove(' ');
            }
            else if(strSection.left(8) == "user_txt")
            {
                m_strUserTxt = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "aux_file")
            {
                m_strAuxFile = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(7) == "pck_typ")
            {
                m_strPckTyp = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "famly_id")
            {
                m_strFamlyId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(6) == "family")
            {
                m_strHandType = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(7) == "prober_") || (strSection.left(8) == "handler_") || (strSection.left(6) == "plugin"))
            {
                m_strHandId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "date_cod")
            {
                m_strDateCod = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(8) == "facil_id") || (strSection.left(8) == "test_sit"))
            {
                m_strFacilId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "floor_id")
            {
                m_strFloorId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(7) == "proc_id")
            {
                m_strProcId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "oper_frq")
            {
                m_strOperFrq = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(8) == "spec_nam") || (strSection.left(8) == "test_pro"))
            {
                m_strSpecNam = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(8) == "spec_ver") || (strSection.left(8) == "test_rev") || (strSection.left(8) == "test_ver"))
            {
                m_strSpecVer = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(4) == "flow")
            {
                m_strFlowId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "setup_id")
            {
                m_strSetupId = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(8) == "dsgn_rev") || (strSection.left(12) == "device_rev"))
            {
                m_strDsgnRev = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(6) == "eng_id") || (strSection == "mod_lot_id"))
            {
                m_strEngId = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(7) == "rom_cod")
            {
                m_strRomCod = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "serl_num")
            {
                m_strSerlNum = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(8) == "supr_nam") || (strSection.left(8) == "supervis"))
            {
                m_strSuprNam = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "usr_desc")
            {
                m_strUserDesc = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "exec_des")
            {
                m_strExecDesc = strLine.right(strLine.length()-iPos-1);
            }
            else if(strSection.left(8) == "disp_cod")
            {
                m_nDispCode = strLine.right(strLine.length()-iPos-1).remove(' ').toInt();
            }
            else if((strSection == "extr_id") || (strSection.left(9) == "equipment"))
            {
                m_strExtrId = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection == "card_id") || (strSection.left(9) == "probecard"))
            {
                m_strCardId = strLine.right(strLine.length()-iPos-1);
            }
            else if((strSection.left(5) == "load_") || (strSection == "dut_board_id"))
            {
                m_strLoadId = strLine.right(strLine.length()-iPos-1);
            }


            // map all values
            m_mapVarValue.insert(strSection, strLine.right(strLine.length()-iPos-1).simplified());
        }
        // else skip the line

    }
    while(!m_phGdfFile->atEnd());
    // Success parsing GDF file

    return true;
}

//////////////////////////////////////////////////////////////////////
// If no bin# used in file, but Bin names instead, then use a
//  mapping table
int CGGDFtoSTDF::GetSoftBinMappingNumber(QString strBinString, int iDefaultBin/*=-1*/)
{
    QString strBin = strBinString.trimmed();
    bool    bIsNumber;
    int     iBin = strBin.toInt(&bIsNumber);
    if(!bIsNumber)
        iBin = iDefaultBin;
    if(iBin == -1)
        iBin = 0;

    if(m_bin_name_mapping.count() <= 0)
    {
        if(iDefaultBin == -1)
            return iBin;                // entry not found, then return number as read in file...
        else
            return iDefaultBin; // Return the default Bin# provided by calling function.
    }

    // Bin1 must stick to Bin1
    if(strBin == "1")
        return 1;

    // A Softbin mapping table exists...then use it!
    int iIndex = mBinNonNumber.indexOf(strBin);
    if(!bIsNumber && iIndex >= 0)
        return FIRST_BIN_NON_NUMBER + iIndex; // entry found...return its value: we allocated Bin# from 2 and higher...which explains the +2!
    else
        return iBin;                // entry not found, then return number as read in file...
}


//////////////////////////////////////////////////////////////////////
// If no bin# used in file, but Bin names instead, then use a
//  mapping table to retrieve the Bin name based on its Bin#
QString CGGDFtoSTDF::GetSoftBinMappingName(int iSoftBin, QString strBinString)
{
    QString strBin = strBinString.trimmed();

    if(iSoftBin == 1 || mBinNonNumber.count() <= 0)
        return strBin;              // entry not found, or Bin1 then return string as read in file...

    // A Softbin mapping table exists...then use it!
    QString strBinName;
    // As the auti bin# allocation starts from 2, this explains the -2!
    if(iSoftBin <= 1)
        strBinName = strBin;
    else
        if(iSoftBin >= FIRST_BIN_NON_NUMBER)
            strBinName = mBinNonNumber[iSoftBin - FIRST_BIN_NON_NUMBER] + " (" + strBin + ")";
    else
        strBinName = strBin;

    // Return string: "<BinName> ( <Test Name failing this Bin> )"
    return strBinName;
}

//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::AnalyzeSoftBinNameMapping()
{
    QString strLine;
    QString strBinMappingName;

    if(m_bGotoNextPass)
        return true;
    do
    {
        strLine = ReadLine().simplified();
        if(strLine.isEmpty() || (strLine.left(2) == "--") || strLine.startsWith("Data_format:", Qt::CaseInsensitive))
            return true; // Exit on empty line

        // Line format:
        // Bin:     CT  no_reprobe  02  FAIL    dut_cont_fail

        if(strLine.startsWith("Bin:", Qt::CaseInsensitive) == false)
            return false;   // Not an empty line, and doesn't starts with expected keyword...ERROR!

        // Extract Bin mapping name & Bin#
        // Line: Soft_Bin_name, probing type, Hard Binning number, PASS/FAIL, Bin name
        strBinMappingName = strLine.section(' ',1,1);
        if(m_bin_name_mapping.indexOf(strBinMappingName) < 0)
        {
            m_bin_name_mapping.append(strBinMappingName);
            bool lIsInt;
            /*int lBinNum = */strBinMappingName.toInt(&lIsInt);
            if (!lIsInt)
                mBinNonNumber.append(strBinMappingName);
        }
        m_mapSoftBinDesc[strLine.section(' ',5,5)] = strBinMappingName;
    }
    while(!m_phGdfFile->atEnd());

    return false;
}

bool CGGDFtoSTDF::AnalyzeDutResult()
{
    int     iPos;
    bool    bWriteDutPir = false;
    bool    bWriteDutPrr = false;
    bool    bIsInt;
    QString strLine;
    QString strWord;
    QString strString;

    QDate       clDate;
    QTime       clTime;

    // first we have to find the "begin_data:"
    if(!GotoMarker(QString("begin_data:").toLatin1().constData()))
        return false;

    do
    {
        if(m_bGotoNextPass)
            return true;

        // Write the last result before to read the new
        if(bWriteDutPir)
        {
            // write result ?
            // first verify if the MIR was writen else save it
            CheckWriteMir();

            if(m_clGdfDutResult.nFlags == WAFER_ID)
            {
                if(m_nWaferNumDie == 0)
                    WriteWir();

                m_nWaferNumDie++;
                if(m_clGdfDutResult.P_F)
                    m_nWaferNumPassed++;
            }
            // store_pir
            WritePir();

            // init some var
            QMap<QString,int>::iterator itTestFlow;

            for(itTestFlow = m_mapTestsFlow.begin(); itTestFlow!=m_mapTestsFlow.end(); itTestFlow++)
            {
                // When only one execution, TestName
                // If more than one execution, TestName[x]
                if(m_mapTestsFlow[itTestFlow.key()] <= 0)
                {
                    if(m_nPass == 1)
                        m_mapTestsFlow[itTestFlow.key()] = -1;
                }
                else
                    m_mapTestsFlow[itTestFlow.key()] = 0;
            }
            //m_mapTestsFlow.clear();

        }
        bWriteDutPir = false;

        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 60))
        {
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();
            if(strString.isEmpty() || (strString.left(2) == "--"))
                continue; // skip empty line

            // Verify if found a key word
            if (strString == "begin_user_data")
            {

                if(!AnalyzeUserData())
                    return false;
            }
            else if (strString == "begin_data_pins")
            {

                if(!AnalyzeDataPins())
                    return false;
            }
            else if (strString == "begin_limit_data")
            {

                if(!AnalyzeLimitData())
                    return false;
            }
            else if (strString == "begin_data_overon")
            {

                if(!GotoMarker(QString("end_data_overon").toLatin1().constData()))
                    return false;
            }
            else if (strString == "begin_data")
            {
                if(!m_stackMarkerTag.isEmpty())
                    m_stackMarkerTag.pop_front();
                if(!m_stackMarkerLine.isEmpty())
                    m_stackMarkerLine.pop_front();
                return false;
            }
            else if((strString.left(6) == "begin_")
                    &&  (strString.indexOf("_format") == -1))
            {
                if(strString == "begin_data_before_retest")
                    m_nWaferNumRetested++;  // must enter and count TODO

                // skip it
                if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                    return false;
            }
            else if(strString == "data_format_test")
            {
                if(!AnalyzeTestResult())
                    return false;
                if(bWriteDutPrr)
                {
                    // save the last PRR

                    WritePrr();

                    // One Pass one
                    // Stop the parse at the first part PASS
                    //if((m_nPass == 1)
                    //&& (m_clGdfDutResult.P_F == 1))
                    //  m_bGotoNextPass = true;

                    m_clGdfDutResult.reset();
                }
                bWriteDutPrr = false;
            }
            continue;
        }

        strLine = strLine.simplified();
        if(strLine.isEmpty() || (strLine.left(2) == "--"))
            continue; // skip empty line

        if(strLine.left(8).toLower() == "end_data")
        {
            // save the last PRR
            if(bWriteDutPrr)
            {
                WritePrr();

                // One Pass one
                // Stop the parse at the first part PASS
                //if((m_nPass == 1)
                //&& (m_clGdfDutResult.P_F == 1))
                //  m_bGotoNextPass = true;
            }
            bWriteDutPrr = false;
            m_clGdfDutResult.reset();

            // if wafer
            // save WRR
            if(m_clGdfDutResult.nFlags == WAFER_ID)
                WriteWrr();
            return true;
        }
        else
        {

            // save the last PRR
            if(bWriteDutPrr)
            {
                WritePrr();

                // One Pass one
                // Stop the parse at the first part PASS
                //if((m_nPass == 1)
                //&& (m_clGdfDutResult.P_F == 1))
                //  m_bGotoNextPass = true;
            }
            bWriteDutPrr = false;

            // One Pass one
            // Stop the parse at the first part PASS
            //if((m_nPass == 1)
            //&& (m_clGdfDutResult.P_F == 1))
            //  m_bGotoNextPass = true;

            m_clGdfDutResult.reset();

            int iResultPos = 1;
            if(m_clGdfDutResult.nFlags == PACKAGE_ID)
                iResultPos = 2;

            strString = strLine;
            iPos = 0;
            while(!strString.isEmpty())
            {
                iPos = strString.indexOf(' ');
                strWord = strString.left(iPos);
                if(iPos < 1)
                    strString = "";
                else
                    strString = strString.right(strString.length()-iPos-1);

                switch(iResultPos)
                {
                case 1: // X,Y
                {
                    iPos = strWord.indexOf(',');
                    m_clGdfDutResult.X = strWord.left(iPos).toInt();
                    m_clGdfDutResult.Y = strWord.right(strWord.length()-iPos-1).toInt();
                    iResultPos++;
                }
                    break;
                case 2:
                    m_clGdfDutResult.Pkg_id = strWord.toInt(&bIsInt);
                    if(!bIsInt)
                    {
                        // Try extract from 11.2
                        m_clGdfDutResult.Pkg_id = strWord.remove(".").toInt();
                    }
                    break;

                case 3: // SOFT bin
                    m_clGdfDutResult.SW_Bin = GetSoftBinMappingNumber(strWord);
                    break;

                case 4: // HARD bin
                    m_clGdfDutResult.HW_Bin = strWord.toInt();
                    break;

                case 5: m_clGdfDutResult.P_F = (strWord.toLower() == "pass"); break;
                case 6: clDate.setDate(strWord.left(4).toInt(),
                                      strWord.mid(4,2).toInt(),
                                      strWord.right(2).toInt()); break;
                case 7:
                {
                    // if only one digit for the hour
                    iPos = 2;
                    if((strWord.length() == 5) || (strWord.length() == 7))
                        iPos = 1;
                    clTime.setHMS(strWord.left(iPos).toInt(),
                                  strWord.mid(iPos,2).toInt(),
                                  strWord.mid(2+iPos,2).toInt());
                    QDateTime clDateTime(clDate,clTime);
                    m_clGdfDutResult.Time = clDateTime.toTime_t();
                    m_lLastPartTime = m_clGdfDutResult.Time;
                    if(m_lSetupT <= 0) m_lSetupT = m_lLastPartTime;
                    if(m_lStartT <= 0) m_lStartT = m_lLastPartTime;
                }
                    break;
                case 8: m_clGdfDutResult.Temp = ToFloat(strWord); break;
                case 9:
                {
                    iPos = strWord.indexOf('/');
                    m_clGdfDutResult.FF = strWord.left(iPos).toInt();
                    strWord = strWord.right(strWord.length()-iPos-1);
                    iPos = strWord.indexOf('/');
                    m_clGdfDutResult.TF = strWord.left(iPos).toInt();
                    m_clGdfDutResult.TT = strWord.right(strWord.length()-iPos-1).toInt();
                    if(!m_clGdfDutResult.bSite) iResultPos++;
                }
                    break;
                case 10: m_clGdfDutResult.Site = strWord.toInt();
                    if(!m_lstSites.contains(m_clGdfDutResult.Site))
                        m_lstSites << m_clGdfDutResult.Site;
                    break;
                case 11: m_clGdfDutResult.Bin_desc = GetSoftBinMappingName(m_clGdfDutResult.SW_Bin,strWord); break;
                default: continue;
                }
                iResultPos++;
            }

            bWriteDutPir = true;
            bWriteDutPrr = true;
        }
    }
    while(!m_phGdfFile->atEnd());

    m_clGdfDutResult.reset();
    return false;
}

bool CGGDFtoSTDF::AnalyzeTestResult()
{
    int     iPos=0;
    bool    bWriteTest = false;
    QString strLine;
    QString strWord;
    QString strString;

    // we can have a "begin_data_before_retest:"
    // first we have to find the "begin_data:"
    if(!GotoMarker(QString("begin_data_tf:").toLatin1().constData()))
        return false;

    m_clGdfTestResult.reset();

    do
    {
        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 40))
        {
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();

            if(strString.isEmpty() || (strString.left(2) == "--") || (strString.startsWith("None", Qt::CaseInsensitive)))
                continue; // skip empty line

            // Verify if found a key word
            if (strString == "begin_user_data")
            {
                if(bWriteTest)
                {
                    if(!WriteResult())
                        return false;

                    m_clGdfTestResult.reset();
                    m_nMaxTestNum = m_clGdfTestResult.test_num > m_nMaxTestNum ? m_clGdfTestResult.test_num : m_nMaxTestNum;
                    bWriteTest=false;
                }

                if(!AnalyzeUserData())
                    return false;
            }
            else if (strString == "begin_data_pins")
            {

                if(!AnalyzeDataPins())
                    return false;
            }
            else if (strString == "begin_limit_data")
            {

                if(!AnalyzeLimitData())
                    return false;
            }
            else if (strString == "begin_data_tf")
            {
                if(!m_stackMarkerTag.isEmpty())
                    m_stackMarkerTag.pop_front();
                if(!m_stackMarkerLine.isEmpty())
                    m_stackMarkerLine.pop_front();
                return false;
            }
            else if((strString.left(6) == "begin_")
                    && (strString.indexOf("_format") == -1))
            {
                // skip it
                if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                    return false;
            }
            else if (strString == "test_number")
            {
                m_clGdfTestResult.test_num = strLine.right(strLine.length()-iPos-1).remove(' ').toInt();
            }

            continue;
        }

        strLine = strLine.trimmed();
        if(strLine.isEmpty() || (strLine.left(2) == "--") || (strLine.left(4).toLower() == "none"))
            continue; // skip empty line

        strString = strLine;
        if(strString.toLower() == "end_data_tf")
        {
            if(bWriteTest)
            {
                if(!WriteResult())
                    return false;
                m_clGdfTestResult.reset();
                m_nMaxTestNum = m_clGdfTestResult.test_num > m_nMaxTestNum ? m_clGdfTestResult.test_num : m_nMaxTestNum;
                bWriteTest = false;
            }

            return true;
        }
        else
        {
            if(bWriteTest)
            {
                if(!WriteResult())
                    return false;

                m_clGdfTestResult.reset();
                m_nMaxTestNum = m_clGdfTestResult.test_num > m_nMaxTestNum ? m_clGdfTestResult.test_num : m_nMaxTestNum;
                bWriteTest=false;
            }
            int     iResultPos = 1;
            bool    bFloat;
            QString strResult;

            strString = strLine.simplified();
            iPos = 0;
            while(!strString.isEmpty())
            {
                iPos = strString.indexOf(' ');
                strWord = strString.left(iPos);
                if(iPos < 1)
                    strString = "";
                else
                    strString = strString.right(strString.length()-iPos-1);

                switch(iResultPos)
                {
                case 1: m_clGdfTestResult.test_name = strWord; break;
                case 2: m_clGdfTestResult.seq_name = strWord; break;
                case 3: m_clGdfTestResult.Result = (strWord.toLower() == "pass"); break;
                case 4: // res0
                case 6: // res1
                case 8: // res2
                case 10:// res3
                {
                    if(IsFunctionalTestSequence(m_clGdfTestResult.seq_name))
                    {
                        strString = "";
                        break;
                    }
                    ToFloat(strWord,&bFloat);
                    if(!bFloat)
                        strString = ""; // end of the parse
                    //else if(strWord == "0.000000")
                    //  strString = ""; // end of the parse
                    else
                        strResult = strWord;
                    break;
                }
                case 5: // unit0
                case 7: // unit1
                case 9: // unit2
                case 11:// unit3
                {
                    if(strWord == "-")
                        strString = "";
                    else
                    {
                        m_clGdfTestResult.ResUnit.append(strResult);
                        m_clGdfTestResult.ResUnit.append(strWord);
                        strString = ""; // take only the first result !!!!!!
                    }
                    break;
                }
                default: return false;
                }
                iResultPos++;
            }

            bWriteTest = true;
        }
    }
    while(!m_phGdfFile->atEnd());

    m_clGdfTestResult.reset();
    return false;
}

bool CGGDFtoSTDF::AnalyzeSummary()
{
    int     iPos;
    bool    bWriteSummary = false;
    QString strLine;
    QString strWord;

    QString strString;

    // first we have to find the "begin_data:"
    if(!GotoMarker(QString("begin_data:").toLatin1().constData()))
        return false;

    do
    {
        if(m_bGotoNextPass)
            return true;

        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 60))
        {
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();
            if(strString.isEmpty() || (strString.left(2) == "--"))
                continue; // skip empty line

            // Verify if found a key word
            if((strString.left(6) == "begin_")
                    && (strString.indexOf("_format") == -1))
            {
                if(strString == "begin_data")
                {
                    if(!m_stackMarkerTag.isEmpty())
                        m_stackMarkerTag.pop_front();
                    if(!m_stackMarkerLine.isEmpty())
                        m_stackMarkerLine.pop_front();
                    return false;
                }

                // skip it
                if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                    return false;
            }
            continue;
        }

        strLine = strLine.simplified();
        if(strLine.isEmpty() || (strLine.left(2) == "--"))
            continue; // skip empty line

        strString = strLine;
        if(strString.toLower() == "end_data")
        {
            // write result ?
            if(bWriteSummary)
            {
                WriteHbr();
                SaveSbr();
            }

            WriteSbr();
            m_clGdfSummary.reset();
            return true;
        }
        else
        {
            if(bWriteSummary)
            {
                // write result ?
                WriteHbr();
                SaveSbr();
                m_clGdfSummary.reset();
                m_lstSites.clear();
            }

            int iResultPos = 1;
            strString = strLine;
            iPos = 0;
            while(!strString.isEmpty())
            {
                iPos = strString.indexOf(' ');
                strWord = strString.left(iPos);
                if(iPos < 1)
                    strString = "";
                else
                    strString = strString.right(strString.length()-iPos-1);

                switch(iResultPos)
                {
                case 1: // SOFT Bin
                    if(m_bin_name_mapping.indexOf(strWord) < 0)
                    {
                        // Try with softBin desc
                        if(m_mapSoftBinDesc.contains(strString.section(" ",strString.count(" "))))
                            strWord = m_mapSoftBinDesc[strString.section(" ",strString.count(" "))];
                    }
                    m_clGdfSummary.SW_Bin = GetSoftBinMappingNumber(strWord);
                    break;

                case 2: // HARD Bin
                    m_clGdfSummary.HW_Bin = strWord.toInt();
                    break;

                case 3: m_clGdfSummary.P_F = (strWord.toLower() == "pass"); break;
                case 4: m_clGdfSummary.Number = strWord.toInt(); break;
                case 5: m_clGdfSummary.Prcnt = ToFloat(strWord);
                    if(m_clGdfSummary.NumberOfSite == 0) iResultPos = 13; break;
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                    m_clGdfSummary.pSite[iResultPos-6] = ToFloat(strWord);
                    if(m_clGdfSummary.NumberOfSite == (iResultPos - 5)) iResultPos = 13;
                    break;
                case 14: m_clGdfSummary.Bin_desc = GetSoftBinMappingName(m_clGdfSummary.SW_Bin,strWord); break;
                default: continue;
                }
                iResultPos++;
            }

            if(m_clGdfSummary.P_F)
                m_nTotalPassed = m_clGdfSummary.Number - 1;
            bWriteSummary = true;
        }
    }
    while(!m_phGdfFile->atEnd());

    m_clGdfSummary.reset();

    return false;
}


//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::AnalyzePartCnt()
{
    int     iPos;
    QString strLine;
    QString strWord;
    QString strString;

    // first we have to find the "begin_data:"
    if(!GotoMarker(QString("begin_data:").toLatin1().constData()))
        return false;

    do
    {
        if(m_bGotoNextPass)
            return true;

        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 60))
        {
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();

            if(strString.isEmpty() || (strString.left(2) == "--"))
                continue; // skip empty line

            // Verify if found a key word
            if((strString.left(6) == "begin_")
                    && (strString.indexOf("_format") == -1))
            {
                if(strString == "begin_data")
                {
                    if(!m_stackMarkerTag.isEmpty())
                        m_stackMarkerTag.pop_front();
                    if(!m_stackMarkerLine.isEmpty())
                        m_stackMarkerLine.pop_front();
                    return false;
                }

                // skip it
                if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                    return false;
            }
            continue;
        }

        strLine = strLine.simplified();
        if(strLine.isEmpty() || (strLine.left(2) == "--"))
            continue; // skip empty line

        strString = strLine;
        if(strString.toLower() == "end_data")
            return true;
        else
        {
            int iResultPos = 1;
            iPos = 0;
            while(!strString.isEmpty())
            {
                iPos = strString.indexOf(' ');
                strWord = strString.left(iPos);
                if(iPos < 1)
                    strString = "";
                else
                    strString = strString.right(strString.length()-iPos-1);

                if(!m_clGdfPartCnt.bSite)
                    iResultPos++;

                switch(iResultPos)
                {
                case 1: m_clGdfPartCnt.Site = strWord.toInt();
                    if(!m_lstSites.contains(m_clGdfPartCnt.Site))
                        m_lstSites << m_clGdfPartCnt.Site;
                    break;
                case 2: m_clGdfPartCnt.Number = strWord.toInt(); break;
                case 3: m_clGdfPartCnt.Prcnt = strWord.toInt(); break;
                default: continue;
                }
                iResultPos++;
            }
            m_nTotalParts = m_clGdfPartCnt.Number - 1;
        }
    }
    while(!m_phGdfFile->atEnd());

    m_clGdfPartCnt.reset();

    return false;
}


//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::AnalyzeUserData()
{
    /*
    if(m_nPass == 1)
    {
        if(!GotoMarker(QString("end_user_data").toLatin1().constData()))
            return false;
        return true;
    }
    */
    int     iPos;
    QString strLine;
    QString strWord;
    QString strString;
    QString strName;
    bool    bReadLimitInfo = false;

    // Accept other block of user_data under this block;
    int     nBeginUserDataCount = 0;

    do
    {
        if(m_bGotoNextPass)
            return true;

        // process only if we found a "--Limit_information"
        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 60))
        {
            bReadLimitInfo = false;
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();

            if(strLine.isEmpty()
                    || ((strLine.left(2) == "--") && (strLine != "--Limit_information")))
                continue; // skip empty line

            // Verify if found a key word
            if(strString == "variablevalue")
            {
                strString = strLine.right(strLine.length()-iPos-1).simplified();

                // Check if it is a TestName result
                // VARIABLE VALUE: Testname = <test_name>, Result = <result> <units>
                // VARIABLE VALUE: Testname = <test_name>, LowL = <low_limit> , HighL = <high_limit> , Result = <result> <units>, Status = <P/F>
                if(strString.startsWith("TestName", Qt::CaseInsensitive))
                {
                    // Have found user test
                    // Have to generated PTR

                    bool    bHaveLowLimit = false;
                    bool    bHaveHighLimit = false;
                    float   fResult=0.0F;
                    float   fLowLimit=0.0F;
                    float   fHighLimit=0.0F;
                    QString strUnit;
                    CGGdfTestResult clTestResult;

                    fLowLimit = fHighLimit = 0;

                    while(!strString.isEmpty())
                    {
                        strWord = strString.section("=",0,0).remove(' ').toUpper();
                        strString = strString.section("=",1);

                        //clTestResult.seq_name;
                        if(strWord == "TESTNAME")
                            clTestResult.test_name = strString.section(",",0,0).simplified();
                        else
                            if(strWord == "TESTNUM")
                                clTestResult.test_num = strString.section(",",0,0).remove(' ').toInt();
                            else
                                if(strWord == "RESULT")
                                {
                                    strUnit = strString.section(",",0,0).section(" ",0,0).remove(' ');
                                    fResult = strUnit.toFloat();
                                    clTestResult.ResUnit.append(strUnit);
                                    strUnit = strString.section(",",0,0).section(" ",1).remove(' ');
                                    clTestResult.ResUnit.append(strUnit);
                                }
                                else
                                    if(strWord == "LOWL")
                                    {
                                        fLowLimit = strString.section(",",0,0).remove(' ').toFloat(&bHaveLowLimit);
                                    }
                                    else
                                        if(strWord == "HIGHL")
                                        {
                                            fHighLimit = strString.section(",",0,0).remove(' ').toFloat(&bHaveHighLimit);
                                        }
                                        else
                                            if(strWord == "STATUS")
                                            {
                                                clTestResult.Result = (strString.section(",",0,0).remove(' ').toLower() == "p");
                                            }

                        strString = strString.section(",",1);
                    }

                    // Check if this test was already saved
                    // Then create new test for each other execution
                    strName = clTestResult.test_name;
                    // first cas : multi-site
                    if(m_clGdfDutResult.bSite)
                        strName = clTestResult.test_name + "[" + QString::number(m_clGdfDutResult.Site) + "]";


                    if(m_mapTestsFlow.contains(strName))
                    {
                        // Entry created on Pass 1
                        // if > -1, have to use Exec index

                        if(m_mapTestsFlow[strName] > -1)
                        {
                            clTestResult.test_name += "[" + QString::number(m_mapTestsFlow[strName]) + "]";

                            // Save the exec count for this run
                            m_mapTestsFlow[strName]++;
                        }
                    }
                    else
                    {
                        // on Pass 1
                        // Initialization
                        if(m_nPass == 1)
                            m_mapTestsFlow[strName] = 0;
                    }

                    // On PASS 1, do not create test
                    if(m_nPass == 2)
                    {
                        CGGdfTest* pclTest = GetTest(clTestResult.test_name);

                        if(bHaveLowLimit || bHaveHighLimit)
                        {
                            SetLimit(pclTest,0,"",strUnit,QString::number(fLowLimit),strUnit,QString::number(fHighLimit));

                            if(clTestResult.Result == -1)
                            {
                                // try to set the PassFail flag
                                clTestResult.Result = true;
                                if(bHaveLowLimit && (fResult < fLowLimit))
                                    clTestResult.Result = false;
                                if(bHaveHighLimit && (fResult > fLowLimit))
                                    clTestResult.Result = false;
                            }
                        }

                        FindParameterIndex(clTestResult.test_name);
                        if((clTestResult.test_num == 0) && (pclTest->test_num == 0))
                        {
                            if(m_bModeDatabase)
                                clTestResult.test_num = pclTest->test_num = m_clGdfTestResult.test_num;
                            else
                                clTestResult.test_num = pclTest->test_num = (++m_nMaxTestNum);
                        }


                        // write result
                        WritePtr(&clTestResult, pclTest);
                    }

                }
                else
                {
                    QString strVar;
                    strString = strString.simplified();
                    // save value found

                    while(!strString.isEmpty())
                    {
                        iPos = strString.indexOf('=');
                        if(strString.indexOf('=')  > 1)
                        {
                            strVar = strString.section("=",0,0).simplified().replace(' ','_');

                            strString = strString.section("=",1);

                            if(strString.indexOf('=') > 1)
                            {
                                // ignore this line
                                strString = "";
                                break;

                            }
                            else
                            {
                                strWord = strString.simplified();
                                strString = "";
                            }


                            // var = val
                            // Write PTR result only if have a good value

                            bool bIsFloat;
                            ToFloat(strWord, &bIsFloat);
                            if(bIsFloat)
                            {
                                CGGdfTestResult clTestResult;
                                clTestResult.test_name = strVar;                        // Test name
                                clTestResult.ResUnit.append(strWord);                   // Value string


                                // Check if this test was already saved
                                // Then create new test for each other execution
                                strName = clTestResult.test_name;
                                // first cas : multi-site
                                if(m_clGdfDutResult.bSite)
                                    strName = clTestResult.test_name + "[" + QString::number(m_clGdfDutResult.Site) + "]";


                                if(m_mapTestsFlow.contains(strName))
                                {
                                    // Entry created on Pass 1
                                    // if > -1, have to use Exec index

                                    if(m_mapTestsFlow[strName] > -1)
                                    {
                                        clTestResult.test_name += "[" + QString::number(m_mapTestsFlow[strName]) + "]";

                                        // Save the exec count for this run
                                        m_mapTestsFlow[strName]++;
                                    }
                                }
                                else
                                {
                                    // on Pass 1
                                    // Initialization
                                    if(m_nPass == 1)
                                        m_mapTestsFlow[strName] = 0;
                                }

                                // On PASS 1, do not create test
                                if(m_nPass == 2)
                                {
                                    CGGdfTest* pclTest = GetTest(clTestResult.test_name);   // Get handle to test name


                                    FindParameterIndex(clTestResult.test_name);
                                    if((clTestResult.test_num == 0) && (pclTest->test_num == 0))
                                    {
                                        if(m_bModeDatabase)
                                            clTestResult.test_num = pclTest->test_num = m_clGdfTestResult.test_num;
                                        else
                                            clTestResult.test_num = pclTest->test_num = (++m_nMaxTestNum);

                                    }

                                    // write result
                                    WritePtr(&clTestResult, pclTest);
                                }
                            }

                            strVar = strWord = "";
                        }
                        else
                            if(strString.contains(' ') == 1)
                            {
                                strVar = strString.section(' ',0,0);
                                strWord = strString.section(' ',1);
                                strString = "";
                                // save it
                                m_mapVarValue.insert(strVar, strWord);
                            }
                            else
                                strString = "";

                    }
                }
            }
            else
                // Accept this tag
                if(strString == "begin_user_data")
                {
                    nBeginUserDataCount ++;
                }
                else
                    if((strString.left(6) == "begin_")
                            && (strString.indexOf("_format") == -1))
                    {
                        // code ignored
                        // string parsed before
                        if(strString == "begin_user_data")
                        {
                            if(!m_stackMarkerTag.isEmpty())
                                m_stackMarkerTag.pop_front();
                            if(!m_stackMarkerLine.isEmpty())
                                m_stackMarkerLine.pop_front();
                            return false;
                        }

                        // skip it
                        if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                            return false;
                    }
            continue;
        }

        strString = strLine;
        strString = strString.remove(' ').toLower();

        if(strString.isEmpty()
                || ((strString.left(2) == "--") && (strString != "--limit_information")))
            continue; // skip empty line

        if(strString == "end_user_data")
        {
            if(nBeginUserDataCount > 0)
            {
                nBeginUserDataCount--;
                continue;
            }

            return true;
        }
        else
            if(strString == "--limit_information")
                bReadLimitInfo = true;
            else
                if(bReadLimitInfo)
                {
                    strString = strLine.simplified();
                    if(strString.count(" ") != 4)
                    {
                        if(strString.count(" ") != 1)
                            continue;
                        // spread info ??
                    }

                    int iResultPos = 1;
                    bool    bIsFloat;
                    QString strSuite, strLowUnits, strHighUnits;
                    float   fLowValue=0, fHighValue=0;
                    iPos = 0;
                    while(!strString.isEmpty())
                    {

                        iPos = strString.indexOf(' ');
                        strWord = strString.left(iPos);
                        if(iPos < 1)
                            strString = "";
                        else
                            strString = strString.right(strString.length()-iPos-1);

                        switch(iResultPos)
                        {
                        case 1: strSuite = strWord; break;
                        case 2:
                        {
                            fLowValue = ToFloat(strWord,&bIsFloat);
                            if(bIsFloat)
                                break;
                            else
                                continue;
                            break;
                        }
                        case 3: strLowUnits = strWord; break;
                        case 4:
                        {
                            fHighValue = ToFloat(strWord,&bIsFloat);
                            if(bIsFloat)
                                break;
                            else
                                continue;
                            break;
                        }
                        case 5: strHighUnits = strWord; break;
                        default: continue;
                        }
                        iResultPos++;
                    }
                    // save result
                    CGGdfTest* pclTest = GetTest(strSuite);
                    CGGdfLimit* pLimit;

                    pLimit = pclTest->GetLimit(); // add a limit info at the end of the limit_list
                    pLimit->opt_flag = (char) (BIT1|BIT2|BIT3|BIT6|BIT7);
                    pLimit->lo_limit = fLowValue;
                    pLimit->llm_scal = PrefixUnitToScall(strLowUnits);
                    pLimit->hi_limit = fHighValue;
                    pLimit->hlm_scal = PrefixUnitToScall(strHighUnits);
                    pLimit->units = PrefixUnitToUnit(strLowUnits);
                    pLimit->low_limit_param = "";
                    pLimit->high_limit_param = "";
                    pLimit->hi_spec = pLimit->lo_spec = 0;
                    pLimit->limitName = "";

                }
    }
    while(!m_phGdfFile->atEnd());

    return false;
}



//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::AnalyzePinList(QString strEndMarker)
{
    int     iPos;
    QString strLine;
    QString strWord;
    QString strString;
    do
    {
        if(m_bGotoNextPass)
            return true;

        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 40))
        {
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();

            if(strString.isEmpty() || (strString.left(2) == "--"))
                continue; // skip empty line

            // Verify if found a key word
            if((strString.left(6) == "begin_")
                    && (strString.indexOf("_format") == -1))
            {
                strWord=strEndMarker.replace("end_","begin_");
                if(strString == strWord)
                {
                    if(!m_stackMarkerTag.isEmpty())
                        m_stackMarkerTag.pop_front();
                    if(!m_stackMarkerLine.isEmpty())
                        m_stackMarkerLine.pop_front();
                    return false;
                }

                // skip it
                if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                    return false;
            }
            continue;
        }

        strString = strLine;
        strString = strString.remove(' ').toLower();

        if(strString.isEmpty() || (strString.left(2) == "--"))
            continue; // skip empty line

        if(strString == strEndMarker)
            return true;
        else
        {
            strString = strLine.simplified();

            int iResultPos = 1;
            bool    bIsInt;
            CGGdfPinChannel*    pclGdfPinChannel = NULL;
            iPos = 0;
            while(!strString.isEmpty())
            {

                iPos = strString.indexOf(' ');
                strWord = strString.left(iPos);
                if(iPos < 1)
                    strString = "";
                else
                    strString = strString.right(strString.length()-iPos-1);

                switch(iResultPos)
                {
                case 1:
                {
                    int iBitPosition = strWord.toInt(&bIsInt);
                    if(bIsInt)
                    {
                        pclGdfPinChannel = new CGGdfPinChannel();
                        pclGdfPinChannel->iBitPosition = iBitPosition;
                        break;
                    }
                    else continue;
                }
                default:
                {
                    if(!pclGdfPinChannel->strPinName.isEmpty())
                        pclGdfPinChannel->lstChannel.append(pclGdfPinChannel->strPinName);

                    pclGdfPinChannel->strPinName = strWord;
                }
                }
                iResultPos++;
            }
            // save result
            if(pclGdfPinChannel)
            {
                // store this result
                m_mapPinChannel.insert(pclGdfPinChannel->iBitPosition, pclGdfPinChannel);
            }
        }
    }
    while(!m_phGdfFile->atEnd());

    return false;
}



//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::AnalyzeDataPins()
{
    int     iPos;
    QString strLine;
    QString strWord;
    QString strString;


    // for the moment skip all data pin information
    //GotoMarker(QString("end_data_pins");
    //return true;

    if(IsFunctionalTestSequence(m_clGdfTestResult.seq_name))
        return GotoMarker(QString("end_data_pins").toLatin1().constData());

    // if no pin list, have to create a data pin list for PMR and MPR
    do
    {
        if(m_bGotoNextPass)
            return true;

        strLine = ReadLine();

        // GCORE-2043
        // Check if this is a keyword
        // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
        // Must start with a char
        iPos = -1;
        if(!strLine.simplified().isEmpty()
                && strLine.simplified().at(0).isLetter())
            iPos = strLine.indexOf(':');
        if((iPos > 0) && (iPos < 40))
        {
            // keyword found
            // have to skip it
            // if found a ':', probably a keyword
            // remove all space
            strString = strLine.left(iPos).remove(' ').toLower();

            if(strString.isEmpty() || (strString.left(2) == "--"))
                continue; // skip empty line

            // Verify if found a key word
            if((strString.left(6) == "begin_")
                    && (strString.indexOf("_format") == -1))
            {
                if(strString == "begin_data_pins")
                {
                    if(!m_stackMarkerTag.isEmpty())
                        m_stackMarkerTag.pop_front();
                    if(!m_stackMarkerLine.isEmpty())
                        m_stackMarkerLine.pop_front();
                    return false;
                }

                // skip it
                if(!GotoMarker(QString(strString.replace("begin_","end_")).toLatin1().constData()))
                    return false;
            }
            continue;
        }

        strString = strLine;
        strString = strString.remove(' ');

        if(strString.isEmpty() || (strString.left(2) == "--"))
            continue; // skip empty line

        if(strString.toLower() == "end_data_pins")
            return true;
        else
            if(1)
            {
                strString = strLine.simplified();

                int iResultPos = 1;
                QString strResult;
                QString strStatus;
                iPos = 0;
                while(!strString.isEmpty())
                {

                    iPos = strString.indexOf(' ');
                    strWord = strString.left(iPos);
                    if(iPos < 1)
                        strString = "";
                    else
                        strString = strString.right(strString.length()-iPos-1);

                    switch(iResultPos)
                    {
                    case 1: m_clGdfTestResult.PinName = strWord;
                        break;
                    case 2:
                        strStatus = strWord;
                        break;
                    case 3:
                        if(QString("PFNU").contains(strWord))
                        {
                            if(strString.isEmpty()
                                    && !m_clGdfTestResult.Result
                                    && (strStatus == "F"))
                            {

                                // Save the first fail Pin for functional test
                                m_clGdfTestResult.ResUnit.clear();
                                if(!GotoMarker(QString("end_data_pins").toLatin1().constData()))
                                    return false;
                                return true;
                            }
                            else
                                if(strString.isEmpty()
                                        && !m_clGdfTestResult.Result)
                                {

                                    // Save the first fail Pin for functional test
                                    strStatus = "N";
                                }
                                else
                                {
                                    // Pass information without result
                                    // Ignore all DataPins
                                    m_clGdfTestResult.ResUnit.clear();
                                    if(!GotoMarker(QString("end_data_pins").toLatin1().constData()))
                                        return false;
                                    return true;
                                }
                        }
                        else
                            strResult = strWord;
                        break;
                    case 4:
                        if(strResult.isEmpty())
                            strWord = "";

                        if(strStatus == "N")
                        {
                            // this result is not available
                            // goto the next
                            iResultPos = 1;

                        }
                        else
                        {

                            // Update PinNum;
                            QString strPinNameNum = QString::number((int)m_clGdfTestResult.ResUnit.count()/2) + " - " + m_clGdfTestResult.test_name;
                            m_clGdfTestResult.ResUnit.append(strResult);
                            m_clGdfTestResult.ResUnit.append(strWord);
                            if(!m_mapPinNum.contains(strPinNameNum))
                            {
                                int iPinNum = -1;
                                QMap<int,CGGdfPinChannel*>::Iterator it;
                                for ( it = m_mapPinChannel.begin(); it != m_mapPinChannel.end(); ++it )
                                {
                                    if(it.value()->strPinName == m_clGdfTestResult.PinName)
                                    {
                                        iPinNum = it.key();
                                        break;
                                    }
                                }
                                if(iPinNum == -1)
                                {
                                    // Not in the list
                                    CGGdfPinChannel* pPinChannel = new CGGdfPinChannel();
                                    pPinChannel->iBitPosition = m_mapPinChannel.count()+1;
                                    pPinChannel->strPinName = m_clGdfTestResult.PinName;
                                    pPinChannel->lstChannel.append(m_clGdfTestResult.PinName);
                                    m_mapPinChannel[pPinChannel->iBitPosition]=pPinChannel;
                                    iPinNum = pPinChannel->iBitPosition;
                                }

                                m_mapPinNum[strPinNameNum] = iPinNum;
                            }


                            // take only the first result !!!!!!
                            strString = "";
                            continue;


                            iResultPos = 1;
                        }
                        break;
                    default:break;
                    }
                    iResultPos++;
                }
            }
    }
    while(!m_phGdfFile->atEnd());

    return false;
}


//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::IsFunctionalTestSequence(QString &strSequenceName)
{
    QString strWord = strSequenceName.toLower();

    if((strWord == "hp_functional")
            || (strWord == "functional")
            || (strWord == "functional.xx")
            || (strWord == "dumfunc.fct")
            || (strWord == "adc_capture")
            || (strWord == "analog_connect")
            || (strWord == "buffer_capture")
            || (strWord == "dac_capture")
            || (strWord == "dps_connectivity")
            || (strWord == "dps_status")
            || (strWord == "connect")
            || (strWord == "disconnect")
            || (strWord == "start_sequencer")
            || (strWord == "test_state")
            || (strWord == "temperature")
            || (strWord == "header")
            || (strWord == "set_utility_lines")
            || (strWord == "get_utility_lines")
            || (strWord == "hp_ux_escape")
            || (strWord == "fw_escape")
            || (strWord == "golden_device")
            || (strWord == "memory_analysis")
            || (strWord == "comment"))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::AnalyzeLimitData()
{

    // Force to save limit each time they are defined
    // => allow multi-limits definition
    // case 7860
    if(false)
    //if(m_nPass == 2)
    {
        // => NO DYNAMIC LIMIT POSSIBILITY
        // To have dynamic limit
        // we must analyze limit in Pass 2
        GotoMarker(QString("end_limit_data").toLatin1().constData());
        return true;
    }

    int     iPos;
    bool    bFloat;
    QString strWord;
    QString strString;
    CGGdfTest* pclTest;
    do
    {
        if(m_bGotoNextPass)
            return true;

        strString = ReadLine().remove(' ');

        if( strString.isEmpty() || (strString.left(2) == "--")
                || (strString.left(4) == "SPRM"))
            continue; // skip empty line

        if(strString.left(4) == "SQSL")
        {
            bFloat = true;
            pclTest = GetTest(m_clGdfTestResult.test_name);

            m_clGdfTestResult.VectorName = strString.section('"',1,1).trimmed();
            if(!m_clGdfTestResult.VectorName.isEmpty())
                pclTest->vector_name = m_clGdfTestResult.VectorName;
            continue;
        }

        if(strString.toLower() == "end_limit_data")
            return true;
        else
            if(1)
            {
                QString strMeasName;
                QString strMinUnit;
                QString strMaxUnit;
                QString strMinVal;
                QString strMaxVal;
                int     i;

                iPos = strString.indexOf(';');
                strWord = strString.left(iPos);

                if((iPos < 0)
                        || IsFunctionalTestSequence(strWord))
                    continue; // no limit info

                bFloat = true;
                pclTest = GetTest(m_clGdfTestResult.test_name);
                /*  mixed signal cases */
                if((strWord == "adc_linearity")
                        || (strWord == "buffer_dist")
                        || (strWord == "dac_distortion"))
                {
                    //SetSpreadFlag(curr_suite, 0);
                    for (i=0; i<4; i++)
                    {
                        strMeasName = strString.section(";",(7*i)+3,(7*i)+3);
                        strMinUnit = strMaxUnit = strString.section(";",(7*i)+4,(7*i)+4);
                        strMinVal = strString.section(";",(7*i)+5,(7*i)+5);
                        strMaxVal = strString.section(";",(7*i)+6,(7*i)+6);
                        //save
                        if(!SetLimit(pclTest,0,strMeasName,strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                            continue;
                    }
                }
                else if(strWord == "crosstalk")
                {
                    strMeasName = strString.section(";",10,10);
                    strMinUnit = strMaxUnit = strString.section(";",11,11);
                    strMinVal = strString.section(";",12,12);
                    strMaxVal = strString.section(";",13,13);
                    //save 4 time
                    //save
                    if(!SetLimit(pclTest,0,strMeasName,strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                    //save
                    if(!SetLimit(pclTest,1,"dummy1",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                    //save
                    if(!SetLimit(pclTest,2,"dummy2",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                    //save
                    if(!SetLimit(pclTest,3,"dummy3",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if((strWord == "adc_distortion")
                        || (strWord == "dac_linearity"))
                {
                    //SetSpreadFlag(curr_suite, 0);
                    for (i=0; i<5; i++)
                    {
                        strMeasName = strString.section(";",(7*i)+3,(7*i)+3);
                        strMinUnit = strMaxUnit = strString.section(";",(7*i)+4,(7*i)+4);
                        strMinVal = strString.section(";",(7*i)+5,(7*i)+5);
                        strMaxVal = strString.section(";",(7*i)+6,(7*i)+6);
                        //save
                        if(!SetLimit(pclTest,0,strMeasName,strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                            continue;
                    }
                }
                /*  non-mixed signal cases */
                else if(strWord == "frequency")
                {
                    strMinUnit = strMaxUnit = strString.section(";",2,2);
                    strMinVal = strString.section(";",3,3);
                    strMaxVal = strString.section(";",4,4);
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "inp_volt_sensitivity")
                {
                    ToFloat(strString.section(";",3,3),&bFloat);
                    if(bFloat)
                    {
                        strMinUnit = strMaxUnit = strString.section(";",2,2);
                        strMinVal = strString.section(";",3,3);
                        strMaxVal = strString.section(";",4,4);
                    }
                    else
                    {
                        strMinUnit = strMaxUnit = strString.section(";",3,3);
                        strMinVal = strString.section(";",4,4);
                        strMaxVal = strString.section(";",5,5);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,"",""))
                        continue;
                    //save
                    if(!SetLimit(pclTest,1,"","","",strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "jitter")
                {
                    ToFloat(strString.section(";",15,15),&bFloat);
                    if(bFloat)
                    {
                        strMinUnit = strString.section(";",14,14);
                        strMinVal = strString.section(";",15,15);

                        strMaxUnit = strString.section(";",16,16);
                        strMaxVal = strString.section(";",17,17);
                    }
                    else
                    {
                        strMinUnit = strString.section(";",4,4);
                        strMinVal = strString.section(";",5,5);
                        strMaxUnit = strString.section(";",6,6);
                        strMaxVal = strString.section(";",7,7);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"","","",strMaxUnit,strMaxVal))
                        continue;
                    //save
                    if(!SetLimit(pclTest,1,"","","",strMaxUnit,strMaxVal))
                        continue;
                }
                else if((strWord == "global_search") || (strWord == "global_search_track"))
                {
                    ToFloat(strString.section(";",6,6),&bFloat);
                    if(bFloat)
                    { // F330
                        strMinUnit = strMaxUnit = strString.section(";",5,5);
                        ToFloat(strString.section(";",12,12),&bFloat);
                        if(!bFloat)
                            strMinVal = strString.section(";",6,6);
                        else
                            strMinVal = strString.section(";",12,12);

                        ToFloat(strString.section(";",13,13),&bFloat);
                        if(!bFloat)
                            strMaxVal = strString.section(";",7,7);
                        else
                            strMaxVal = strString.section(";",13,13);
                    }
                    else
                    { // F660
                        strMinUnit = strMaxUnit = strString.section(";",4,4);
                        strMinVal = strString.section(";",10,10);
                        strMaxVal = strString.section(";",11,11);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "out_volt_sensitivity")
                {
                    ToFloat(strString.section(";",3,3),&bFloat);
                    if(bFloat)
                    {
                        strMinUnit = strMaxUnit = strString.section(";",2,2);
                        strMinVal = strString.section(";",3,3);
                        strMaxVal = strString.section(";",4,4);
                    }
                    else
                    {
                        strMinUnit = strMaxUnit = strString.section(";",3,3);
                        strMinVal = strString.section(";",4,4);
                        strMaxVal = strString.section(";",5,5);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"","","",strMaxUnit,strMaxVal))
                        continue;
                    //save
                    if(!SetLimit(pclTest,1,"",strMinUnit,strMinVal,"",""))
                        continue;
                }
                else if(strWord == "setup_time")
                {
                    ToFloat(strString.section(";",9,9),&bFloat);
                    if(bFloat)
                    {
                        strMinUnit = strMaxUnit = strString.section(";",8,8);
                        strMinVal = strString.section(";",9,9);
                        strMaxVal = strString.section(";",10,10);
                    }
                    else
                    {
                        strMinUnit = strString.section(";",4,4);
                        strMinVal = strString.section(";",5,5);
                        if(!bFloat) continue; // error
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "hold_time")
                {
                    ToFloat(strString.section(";",9,9),&bFloat);
                    if(bFloat)
                    { // F330
                        strMinUnit = strString.section(";",8,8);
                        strMinVal = strString.section(";",9,9);
                    }
                    else
                    { // F660
                        strMinUnit = strString.section(";",4,4);
                        strMinVal = strString.section(";",5,5);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "spec_search")
                {
                    ToFloat(strString.section(";",6,6),&bFloat);
                    if(bFloat)
                    {
                        strMinUnit = strMaxUnit = strString.section(";",4,4);
                        strMinVal = strString.section(";",6,6);
                        strMaxVal = strString.section(";",7,7);
                    }
                    else
                    {
                        strMinUnit = strMaxUnit = strString.section(";",5,5);
                        strMinVal = strString.section(";",7,7);
                        strMaxVal = strString.section(";",8,8);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if((strWord == "standby_current")
                        || (strWord == "operating_current")
                        || (strWord == "dvm"))
                {
                    strMinUnit = strString.section(";",2,2);
                    strMinVal = strString.section(";",3,3);
                    strMaxUnit = strString.section(";",4,4);
                    strMaxVal = strString.section(";",5,5);
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "production_iddq")
                {
                    strMinUnit = strString.section(";",3,3);
                    strMinVal = strString.section(";",4,4);
                    strMaxUnit = strString.section(";",5,5);
                    strMaxVal = strString.section(";",6,6);
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "continuity")
                {
                    ToFloat(strString.section(";",6,6),&bFloat);
                    if(bFloat)
                    { // F330
                        strMinUnit = strString.section(";",5,5);
                        strMinVal = strString.section(";",6,6);
                        strMaxUnit = strString.section(";",7,7);
                        strMaxVal = strString.section(";",8,8);
                    }
                    else
                    { // F660
                        strMinUnit = strString.section(";",6,6);
                        strMinVal = strString.section(";",7,7);
                        strMaxUnit = strString.section(";",12,12);
                        strMaxVal = strString.section(";",13,13);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "high_z")
                {
                    strMinUnit = strString.section(";",6,6);
                    strMinVal = strString.section(";",7,7);
                    strMaxUnit = strString.section(";",8,8);
                    strMaxVal = strString.section(";",9,9);
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "propagation_delay")
                {
                    ToFloat(strString.section(";",9,9),&bFloat);
                    if(bFloat)
                    {
                        strMaxUnit = strString.section(";",8,8);
                        strMaxVal = strString.section(";",9,9);
                        strMinUnit = strString.section(";",10,10);
                        strMinVal = strString.section(";",11,11);
                    }
                    else
                    {
                        strMaxUnit = strString.section(";",4,4);
                        strMaxVal = strString.section(";",5,5);
                        strMinUnit = strString.section(";",6,6);
                        strMinVal = strString.section(";",7,7);
                    }
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if((strWord == "iddq")
                        || (strWord == "general_pmu") // F330
                        || (strWord == "general_pmu1")) // F660)
                {
                    strMaxUnit = strString.section(";",8,8);
                    strMaxVal = strString.section(";",9,9);
                    strMinUnit = strString.section(";",10,10);
                    strMinVal = strString.section(";",11,11);
                    if(!bFloat) continue; // error
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "param_functional")
                {
                    strMinUnit = strString.section(";",22,22);
                    strMinVal = strString.section(";",23,23);
                    strMaxUnit = strString.section(";",24,24);
                    strMaxVal = strString.section(";",25,25);
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }
                else if(strWord == "output_dc")
                {
                    ToFloat(strString.section(";",13,13),&bFloat);
                    if(bFloat)
                    { // F330
                        strMinUnit = strString.section(";",12,12);
                        strMinVal = strString.section(";",13,13);
                        strMaxUnit = strString.section(";",6,6);
                        strMaxVal = strString.section(";",7,7);
                        //save
                        if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                            continue;

                        strMinUnit = strString.section(";",6,6);
                        strMinVal = strString.section(";",8,8);
                        strMaxUnit = strString.section(";",12,12);
                        strMaxVal = strString.section(";",14,14);
                        //save
                        if(!SetLimit(pclTest,1,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                            continue;
                    }
                    else
                    { // F660
                        strMinUnit = strString.section(";",8,8);
                        strMinVal = strString.section(";",9,9);
                        strMaxUnit = strString.section(";",11,11);
                        strMaxVal = strString.section(";",12,12);
                        if(!bFloat) continue; // error
                        //save
                        if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                            continue;

                        strMinUnit = strString.section(";",8,8);
                        strMinVal = strString.section(";",10,10);
                        strMaxUnit = strString.section(";",11,11);
                        strMaxVal = strString.section(";",13,13);
                        //save
                        if(!SetLimit(pclTest,1,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                            continue;
                    }
                }
                else if((strWord == "leakage")
                        || (strWord == "input_dc"))
                {
                    strMinUnit = strString.section(";",8,8);
                    strMinVal = strString.section(";",9,9);
                    strMaxUnit = strString.section(";",11,11);
                    strMaxVal = strString.section(";",12,12);
                    if(!bFloat) continue; // error
                    //save
                    if(!SetLimit(pclTest,0,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;

                    strMinUnit = strString.section(";",8,8);
                    strMinVal = strString.section(";",10,10);
                    strMaxUnit = strString.section(";",11,11);
                    strMaxVal = strString.section(";",13,13);
                    //save
                    if(!SetLimit(pclTest,1,"",strMinUnit,strMinVal,strMaxUnit,strMaxVal))
                        continue;
                }

                // save result
            }
    }
    while(!m_phGdfFile->atEnd());

    return false;
}



//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::GotoMarker(const char *szEndMarker)
{
    int iPos;
    QString strLine;
    QString strSection;
    QString strEndMarker = QString(szEndMarker).toLower();
    int     iEndMarkerLength = strEndMarker.length();


    while(!m_phGdfFile->atEnd())
    {

        strLine = ReadLine().trimmed().left(iEndMarkerLength+15).toLower();

        if(strLine.left(iEndMarkerLength) == strEndMarker)
        {
            // extract the word, if no space, return the word
            // remove all space
            strSection = strLine.left(strLine.indexOf(' '));

            if(strSection == strEndMarker)
                return true;
        }

        if(strLine.left(6) == ("begin_"))
        {
            // extract the word, if no space, return the word
            // remove all space
            strSection = strLine.left(strLine.indexOf(' '));

            if((strSection.indexOf("_format") == -1))
            {
                iPos = strSection.indexOf(':');
                if((iPos > 0) && (iPos < 40))
                {
                    strSection = strSection.left(iPos).replace("begin_","end_");

                    // Allow only begin_user_data under a block begin_user_data
                    // else error
                    if((strEndMarker == "end_user_data")
                            && (strSection != "end_user_data"))
                    {
                        m_stackMarkerTag.push_front(strLine.left(iPos));
                        m_stackMarkerLine.push_front(QString::number(m_nCurrentLine));

                        return false;
                    }

                    if(!GotoMarker(strSection.toLatin1().constData()))
                        return false;
                }
                else
                    return false;
            }
        }
    }

    return false;
}



//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::SetLimit(CGGdfTest*pclTest, int iIndex, QString strLimitName,
                           QString strLowUnit, QString strLowValue,
                           QString strHighUnit, QString strHighValue)
{
    bool    bFloat = false;
    float   fLowValue = 0, fHighValue = 0;
    QString strLowParam, strHighParam;

    if((strLowValue.isEmpty() && strHighValue.isEmpty()) || (strLowUnit.isEmpty() && strHighUnit.isEmpty()))
        return false;

    fLowValue = ToFloat(strLowValue,&bFloat);
    if(!bFloat && !strLowValue.isEmpty()) return false;
    if(!strLowValue.isEmpty() && (strLowValue.left(1) == "@"))
        strLowParam = strLowValue.right(strLowValue.length()-1);

    fHighValue = ToFloat(strHighValue,&bFloat);
    if(!bFloat && !strHighValue.isEmpty()) return false;
    if(!strHighValue.isEmpty() && (strHighValue.left(1) == "@"))
        strHighParam = strHighValue.right(strHighValue.length()-1);

    //save
    CGGdfLimit* pLimit = pclTest->GetLimit(iIndex,strLimitName);
    bool bNewLimit = pLimit->SetLimit(strLowUnit,fLowValue,strHighUnit,fHighValue,strLowParam,strHighParam);
    if(pclTest->bTestSaved && bNewLimit)
    {
        pclTest->bLimitSaved = false;
        pclTest->lstLimitSavedForSites.clear();
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
CGGdfTest* CGGDFtoSTDF::GetTest(QString strSuite)
{
    CGGdfTest* pclTest;
    QMap<QString,CGGdfTest*>::Iterator itTest = m_mapTest.find(strSuite);
    if(itTest != m_mapTest.end())
    {   // exist
        pclTest = itTest.value();
    }
    else
    {   // create it
        pclTest = new CGGdfTest();
        pclTest->test_name = strSuite;
        m_mapTest.insert(strSuite, pclTest);

        m_clGdfTestResult.test_num++;
    }

    return pclTest;
}



//////////////////////////////////////////////////////////////////////
// first have to verify if this is a user variable and then try to convert it into float
//////////////////////////////////////////////////////////////////////
float CGGDFtoSTDF::ToFloat(QString strVar, bool* pbFloat)
{
    QString strValue;

    strValue = ToValue(strVar);
    return strValue.toFloat(pbFloat);
}



//////////////////////////////////////////////////////////////////////
// try to find if this variable was declared and initialized by the user
// return the current value
//////////////////////////////////////////////////////////////////////
QString CGGDFtoSTDF::ToValue(QString strVar)
{
    QString strValue;

    strValue = strVar;
    if(!strValue.isEmpty() && (strValue.left(1) == "@") && (m_mapVarValue.contains(strValue.right(strValue.length()-1))))
    {   // VARIABLE VALUE
        QMap<QString,QString>::Iterator itVar;
        itVar = m_mapVarValue.find(strValue.right(strValue.length()-1));
        strValue = itVar.value();
    }
    return strValue;
}


//////////////////////////////////////////////////////////////////////
// Write STDF file
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteFar()
{
    if(m_nPass == 1)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(1);                  // SUN CPU type
    m_pStdfFile->WriteByte(4);                  // STDF V4
    m_pStdfFile->WriteRecord();
}

//////////////////////////////////////////////////////////////////////
// Checks if MIR still not written (eg: if no datalog and reach summary section)
void CGGDFtoSTDF::CheckWriteMir()
{
    if(m_nPass == 1)
        return;
    // If MIR already written, don't do it again!
    if(m_bMirWriten)
        return;

    // Write first few STDF records
    WriteFar();
    WriteMir();
    WritePmr();
    m_bMirWriten = true;
}

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteMir()
{
    if(m_nPass == 1)
        return;

    if(m_lStartT <= 0)
        m_lStartT = QDateTime::currentDateTime().toTime_t();

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    m_pStdfFile->WriteHeader(&RecordReadInfo);

    m_pStdfFile->WriteDword(m_lSetupT);                     // Setup time
    m_pStdfFile->WriteDword(m_lStartT);                     // Start time
    m_pStdfFile->WriteByte((BYTE) m_nStatNum);              // Station
    m_pStdfFile->WriteByte((BYTE) m_cModeCod);              // Test Mode
    m_pStdfFile->WriteByte((BYTE) m_cRtstCod);              // rtst_cod
    m_pStdfFile->WriteByte((BYTE) m_cProtCod);              // prot_cod
    m_pStdfFile->WriteWord(m_lBurnTim);                     // burn_time
    m_pStdfFile->WriteByte((BYTE) m_cCModCod);              // cmod_cod
    m_pStdfFile->WriteString(m_strLotId.toLatin1().constData());        // Lot ID
    m_pStdfFile->WriteString(m_strPartTyp.toLatin1().constData());  // Part Type / Product ID
    m_pStdfFile->WriteString(m_strNodeNam.toLatin1().constData());  // Node name
    m_pStdfFile->WriteString(m_strTstrTyp.toLatin1().constData());  // Tester Type
    m_pStdfFile->WriteString(m_strJobNam.toLatin1().constData());   // Job name
    m_pStdfFile->WriteString(m_strJobRev.toLatin1().constData());   // job_rev
    m_pStdfFile->WriteString(m_strSbLotId.toLatin1().constData());  // sblot_id
    m_pStdfFile->WriteString(m_strOperNam.toLatin1().constData());  // oper_nam
    m_pStdfFile->WriteString(m_strExecTyp.toLatin1().constData());  // exec_typ
    m_pStdfFile->WriteString(m_strExecVer.toLatin1().constData());  // exec_ver
    m_pStdfFile->WriteString(m_strTestCod.toLatin1().constData());  // test_cod
    m_pStdfFile->WriteString(m_strTstTemp.toLatin1().constData());  // tst_temp
    // Construct custom Galaxy USER_TXT
    QString strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":HP93K_HP83K";
    m_pStdfFile->WriteString(strUserTxt.toLatin1().constData());    // user-txt
    //m_pStdfFile->WriteString(m_strUserTxt));  // user_txt
    m_pStdfFile->WriteString(m_strAuxFile.toLatin1().constData());  // aux_file
    m_pStdfFile->WriteString(m_strPckTyp.toLatin1().constData());   // pkg_typ
    m_pStdfFile->WriteString(m_strFamlyId.toLatin1().constData());  // famly_id
    m_pStdfFile->WriteString(m_strDateCod.toLatin1().constData());  // date_cod
    m_pStdfFile->WriteString(m_strFacilId.toLatin1().constData());  // facil_id
    m_pStdfFile->WriteString(m_strFloorId.toLatin1().constData());  // floor_id
    m_pStdfFile->WriteString(m_strProcId.toLatin1().constData());   // proc_id
    m_pStdfFile->WriteString(m_strOperFrq.toLatin1().constData());  // oper_frq
    m_pStdfFile->WriteString(m_strSpecNam.toLatin1().constData());  // spec_nam
    m_pStdfFile->WriteString(m_strSpecVer.toLatin1().constData());  // spec_ver
    m_pStdfFile->WriteString(m_strFlowId.toLatin1().constData());   // flow_id
    m_pStdfFile->WriteString(m_strSetupId.toLatin1().constData());  // setup_id
    m_pStdfFile->WriteString(m_strDsgnRev.toLatin1().constData());  // dsgn_rev
    m_pStdfFile->WriteString(m_strEngId.toLatin1().constData());        // eng_id
    m_pStdfFile->WriteString(m_strRomCod.toLatin1().constData());   // rom_cod
    m_pStdfFile->WriteString(m_strSerlNum.toLatin1().constData());  // serl_num
    m_pStdfFile->WriteString(m_strSuprNam.toLatin1().constData());  // supr_nam

    m_pStdfFile->WriteRecord();

    // Write SDR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 80;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte((BYTE)1);            // head#
    m_pStdfFile->WriteByte((BYTE)1);            // Group#
    m_pStdfFile->WriteByte((BYTE)m_clGdfSummary.NumberOfSite);          // site_count
    foreach(int lSite, m_lstSites)
        m_pStdfFile->WriteByte((BYTE)lSite);            // array of test site# (dummy!)
    m_pStdfFile->WriteString(m_strHandType.toLatin1().constData()); // HAND_TYP: Handler/prober type
    m_pStdfFile->WriteString(m_strHandId.toLatin1().constData());   // HAND_ID: Handler/prober name
    m_pStdfFile->WriteString("");                               // CARD_TYP: Probe card type
    m_pStdfFile->WriteString(m_strCardId.toLatin1().constData());// CARD_ID: Probe card name
    m_pStdfFile->WriteString("");                               // LOAD_TYP: Load board type
    m_pStdfFile->WriteString(m_strLoadId.toLatin1().constData());   // LOAD_ID: Load board name
    m_pStdfFile->WriteString("");                               // DIB_TYP: DIB board type
    m_pStdfFile->WriteString("");                               // DIB_ID: DIB board name
    m_pStdfFile->WriteString("");                               // CABL_TYP: Interface cable type
    m_pStdfFile->WriteString("");                               // CABL_ID: Interface cable name
    m_pStdfFile->WriteString("");                               // CONT_TYP: Handler contactor type
    m_pStdfFile->WriteString("");                               // CONT_ID: Handler contactor name
    m_pStdfFile->WriteString("");                               // LASR_TYP: Laser type
    m_pStdfFile->WriteString("");                               // LASR_ID: Laser name
    m_pStdfFile->WriteString("");                               // EXTR_TYP: Extra equipment type
    m_pStdfFile->WriteString(m_strExtrId.toLatin1().constData());// EXTR_ID: Extra equipment name
    m_pStdfFile->WriteRecord();

    m_bMirWriten = true;
}



//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteMrr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteDword(m_lLastPartTime);       // Date and time last part tested
    m_pStdfFile->WriteByte(m_nDispCode);            // Lot disposition code
    m_pStdfFile->WriteString(m_strUserDesc.toLatin1().constData());// Lot description supplied by user
    m_pStdfFile->WriteString(m_strExecDesc.toLatin1().constData());// Lot description supplied by exec
    m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WritePcr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(255);                    // Test head number
    m_pStdfFile->WriteByte(1);                      // Test site number

    // variables was initialize to -1
    if(m_nTotalParts > -1) m_nTotalParts++;
    if(m_nTotalRetested > -1) m_nTotalRetested++;
    if(m_nTotalPassed > -1) m_nTotalPassed++;

    m_pStdfFile->WriteDword(m_nTotalParts);         // Number of parts tested
    m_pStdfFile->WriteDword(m_nTotalRetested);                  // Number of parts retested
    m_pStdfFile->WriteDword(4294967295UL);                  // Number of aborts during testing
    m_pStdfFile->WriteDword(m_nTotalPassed);        // Number of good (passed) parts tested
    m_pStdfFile->WriteDword(4294967295UL);                  // Number of functional parts tested
    m_pStdfFile->WriteRecord();
}

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteHbr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    // Write MIR if still not done (eg: if no datalog, and only summary data available)
    CheckWriteMir();

    // Write HBR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(255);                        // Test Head = ALL
    m_pStdfFile->WriteByte(1);                          // Test sites = ALL
    GetSoftBinMappingNumber(m_clGdfSummary.Bin_desc,m_clGdfSummary.HW_Bin);
    m_pStdfFile->WriteWord(m_clGdfSummary.HW_Bin);      // HBIN = 0
    m_pStdfFile->WriteDword(m_clGdfSummary.Number);     // Total Bins
    if(m_clGdfSummary.P_F)
        m_pStdfFile->WriteByte('P');
    else
        m_pStdfFile->WriteByte('F');
    m_pStdfFile->WriteString(m_clGdfSummary.Bin_desc.toLatin1().constData());
    m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::SaveSbr()
{
    if(m_nPass == 1)
        return;

    m_clSbrList.append(new CGGdfSbr(m_clGdfSummary.SW_Bin,
                                    m_clGdfSummary.P_F,
                                    m_clGdfSummary.Number,
                                    m_clGdfSummary.Prcnt,
                                    m_clGdfSummary.Bin_desc));
}


//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteSbr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    CGGdfSbr*           pSbr;
    GS::StdLib::StdfRecordReadInfo   RecordReadInfo;
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    // Write MIR if still not done (eg: if no datalog, and only summary data available)
    CheckWriteMir();

    while(!m_clSbrList.isEmpty())
    {
        pSbr = m_clSbrList.takeFirst();
        if(pSbr == NULL)
            return;

        // Write SBR
        m_pStdfFile->WriteHeader(&RecordReadInfo);
        m_pStdfFile->WriteByte(255);                    // Test Head = ALL
        m_pStdfFile->WriteByte(1);                      // Test sites = ALL
        m_pStdfFile->WriteWord(pSbr->SW_Bin);           // SBIN = 0
        m_pStdfFile->WriteDword(pSbr->Number);          // Total Bins
        if(pSbr->P_F)
            m_pStdfFile->WriteByte('P');
        else
            m_pStdfFile->WriteByte('F');
        m_pStdfFile->WriteString(pSbr->Bin_desc.toLatin1().constData());
        m_pStdfFile->WriteRecord();
    }
}

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WritePmr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    // Write MIR if still not done (eg: if no datalog, and only summary data available)
    CheckWriteMir();

    // Write PMR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 60;

    CGGdfPinChannel* pclGdfPinChannel;
    QMap<int,CGGdfPinChannel*>::Iterator it;
    for ( it = m_mapPinChannel.begin(); it != m_mapPinChannel.end(); ++it )
    {
        pclGdfPinChannel = it.value();
        for (QStringList::const_iterator
             iter  = pclGdfPinChannel->lstChannel.begin();
             iter != pclGdfPinChannel->lstChannel.end(); ++iter) {
            m_pStdfFile->WriteHeader(&RecordReadInfo);
            m_pStdfFile->WriteWord(pclGdfPinChannel->iBitPosition);
            m_pStdfFile->WriteWord(0);
            m_pStdfFile->WriteString((*iter).toLatin1().constData());
            m_pStdfFile->WriteString(pclGdfPinChannel->strPinName.toLatin1().constData());
            m_pStdfFile->WriteString("");
            m_pStdfFile->WriteByte(1);                      // Test head number
            m_pStdfFile->WriteByte(1);                      // Test site number
            m_pStdfFile->WriteRecord();
        }
    }
}

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteWir()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write WIR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(1);                      // Test head number
    m_pStdfFile->WriteByte(255);                    // Site group number
    m_pStdfFile->WriteDword(m_lLastPartTime);       // Time first part tested
    m_pStdfFile->WriteString(m_strWaferId.toLatin1().constData());  // WaferID
    m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteWrr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write WRR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(1);                  // Test head number
    m_pStdfFile->WriteByte(255);                // Site group number
    m_pStdfFile->WriteDword(m_lLastPartTime);   // Date and time last part tested
    m_pStdfFile->WriteDword(m_nWaferNumDie);    // Number of parts tested
    m_pStdfFile->WriteDword(m_nWaferNumRetested);// Number of parts retested
    m_pStdfFile->WriteDword(4294967295UL);              // Number of aborted during testing
    m_pStdfFile->WriteDword(m_nWaferNumPassed); // Number of good (passed) parts tested
    m_pStdfFile->WriteDword(4294967295UL);      // Number of functional parts tested
    m_pStdfFile->WriteString(m_strWaferId.toLatin1().constData());  // WaferID
    m_pStdfFile->WriteString("");               // Fab wafer Id
    m_pStdfFile->WriteString("");               // Wafer frame Id
    m_pStdfFile->WriteString("");               // Wafer mask Id
    m_pStdfFile->WriteString(m_strUserDesc.toLatin1().constData());// Wafer description supplied by user
    m_pStdfFile->WriteString("");               // Wafer description supplied by exec
    m_pStdfFile->WriteRecord();
}


void CGGDFtoSTDF::WritePir()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write PIR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 10;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(1);                      // Test head
    m_pStdfFile->WriteByte(m_clGdfDutResult.bSite ? m_clGdfDutResult.Site : 1); // Test site number
    m_pStdfFile->WriteRecord();
}

void CGGDFtoSTDF::WritePrr()
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    char    bit_field=0;
    QString strString;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

    m_nTotalParts++;

    if(m_clGdfDutResult.nFlags == WAFER_ID)
    {
        strString.sprintf("%d", m_nWaferNumDie);
        if(m_clGdfDutResult.Retest == 1)
        {
            bit_field = BIT1;
            m_nTotalRetested++;
        }
    }
    else
    {
        strString.sprintf("%d",m_clGdfDutResult.Pkg_id);
        if(m_clGdfDutResult.Retest == 1)
        {
            bit_field = BIT0;
            m_nTotalRetested++;
        }
    }

    if(m_clGdfDutResult.P_F == 0)
        bit_field |= BIT3;
    else
        m_nTotalPassed++;


    // Write PRR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 20;
    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteByte(1);              // Test head
    m_pStdfFile->WriteByte(m_clGdfDutResult.bSite ? m_clGdfDutResult.Site : 1); // Tester site:1,2,3,4 or 5
    m_pStdfFile->WriteByte(bit_field);                  // PART_FLG : PASSED
    m_pStdfFile->WriteWord(m_clGdfDutResult.TT);        // NUM_TEST
    m_pStdfFile->WriteWord(m_clGdfDutResult.HW_Bin);    // HARD_BIN
    m_pStdfFile->WriteWord(m_clGdfDutResult.SW_Bin);    // SOFT_BIN
    m_pStdfFile->WriteWord(m_clGdfDutResult.X);         // X_COORD
    m_pStdfFile->WriteWord(m_clGdfDutResult.Y);         // Y_COORD
    m_pStdfFile->WriteDword(0);                         // No testing time known...
    m_pStdfFile->WriteString(strString.toLatin1().constData()); // PART_ID
    m_pStdfFile->WriteString(m_clGdfDutResult.Bin_desc.toLatin1().constData());// PART_TXT
    m_pStdfFile->WriteString("");                       // PART_FIX

    m_pStdfFile->WriteRecord();
}

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteTsr()
{
    if(m_nPass == 1)
        return;

    // first verify if the MIR was writen else save it
    CheckWriteMir();

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write TSR
    RecordReadInfo.iRecordType = 10;
    RecordReadInfo.iRecordSubType = 30;

    QMap<QString,CGGdfTest*>::Iterator itTest;
    CGGdfTest*  pclTest;
    QMap<QString,CGGdfPin*>::Iterator itPin;
    CGGdfPin*   pclPin;
    for ( itTest = m_mapTest.begin(); itTest != m_mapTest.end(); ++itTest )
    {
        pclTest = itTest.value();
        for ( itPin = pclTest->pin_list.begin(); itPin != pclTest->pin_list.end(); ++itPin )
        {
            pclPin = itPin.value();
            m_pStdfFile->WriteHeader(&RecordReadInfo);

            m_pStdfFile->WriteByte(255);                // Test head
            m_pStdfFile->WriteByte(1);                  // Tester site
            if(pclTest->nTestType == 1)
                m_pStdfFile->WriteByte('P');            // Parametric
            else if(pclTest->nTestType == 2)
                m_pStdfFile->WriteByte('M');            // MultiParametric
            else if(pclTest->nTestType == 3)
                m_pStdfFile->WriteByte('F');            // Functionel
            else
                m_pStdfFile->WriteByte(' ');            // Unknown
            m_pStdfFile->WriteDword(pclPin->test_num);  // Test Number
            m_pStdfFile->WriteDword(pclPin->results.exec_cnt);// Number of test executions
            m_pStdfFile->WriteDword(pclPin->results.fail_cnt);// Number of test failures
            m_pStdfFile->WriteDword(4294967295UL);              // Number of alarmed tests
            m_pStdfFile->WriteString(pclTest->test_name.toLatin1().constData());// TEST_NAM
            m_pStdfFile->WriteString(pclTest->seq_name.toLatin1().constData());// SEQ_NAME
            m_pStdfFile->WriteString("");               // TEST_LBL
            m_pStdfFile->WriteByte(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);// OPT_FLG
            m_pStdfFile->WriteFloat(0);                 // Values
            m_pStdfFile->WriteFloat(0);                 // Values
            m_pStdfFile->WriteFloat(0);                 // Values
            m_pStdfFile->WriteFloat(0);                 // Values
            m_pStdfFile->WriteFloat(0);                 // Values

            m_pStdfFile->WriteRecord();
        }
    }

}

//////////////////////////////////////////////////////////////////////
bool CGGDFtoSTDF::WriteResult()
{
    // write result ?
    // only when etrieve a new entry or at the end
    // Force the creation in pass 1
    CGGdfTest* pclTest = GetTest(m_clGdfTestResult.test_name);
    FindParameterIndex(m_clGdfTestResult.test_name);
    if(pclTest->test_num == 0)
    {
        // is a new test
        // save the current test_num
        pclTest->test_num = m_clGdfTestResult.test_num;
    }
    // Determine the TestType
    if(pclTest->nTestType == 4)
    {
        if(m_clGdfTestResult.ResUnit.count() == 0)
        {
            pclTest->nTestType = 3;
            pclTest->nPinCount = 0;
        }
        else
            if(m_clGdfTestResult.ResUnit.count() == 2)
            {
                pclTest->nTestType = 1;
                pclTest->nPinCount = 1;
            }
            else
            {
                pclTest->nTestType = 2;
                pclTest->nPinCount = (m_clGdfTestResult.ResUnit.count()/2);
            }
    }
    // 1=PTR 2=MPR 3=FTR
    // Check if MPR have always the same Pin count
    // If not have to reject the file
    if((pclTest->nTestType == 2) && (pclTest->nPinCount != (m_clGdfTestResult.ResUnit.count()/2)))
    {
        if(!m_bAcceptMprDynamicPins)
        {
            m_strLastErrorMsg = "Test " + pclTest->test_name + " was detected with several Pin counts";
            m_iLastError = errErrorInvalidFormat;
            return false;
        }
        if(!pclTest->bOnlySaveMinMaxPinResult)
        {
            // Update PinNum;
            for(int i=1; i<3; i++)
            {
                QString strPinName;
                if(i==1)
                    strPinName = "MIN";
                else
                    strPinName = "MAX";

                QString strPinNameNum = QString::number((int)i) + " - " +  pclTest->test_name;
                if(m_mapPinNum.contains(strPinNameNum))
                {
                    m_mapPinNum.take(strPinNameNum);
                }
                if(!m_mapPinNum.contains(strPinNameNum))
                {
                    int iPinNum = -1;
                    QMap<int,CGGdfPinChannel*>::Iterator it;
                    for ( it = m_mapPinChannel.begin(); it != m_mapPinChannel.end(); ++it )
                    {
                        if(it.value()->strPinName == strPinName)
                        {
                            iPinNum = it.key();
                            break;
                        }
                    }
                    if(iPinNum == -1)
                    {
                        // Not in the list
                        CGGdfPinChannel* pPinChannel = new CGGdfPinChannel();
                        pPinChannel->iBitPosition = m_mapPinChannel.count()+1;
                        pPinChannel->strPinName = strPinName;
                        pPinChannel->lstChannel.append(strPinName);
                        m_mapPinChannel[pPinChannel->iBitPosition]=pPinChannel;
                        iPinNum = pPinChannel->iBitPosition;
                    }

                    m_mapPinNum[strPinNameNum] = iPinNum;
                }
            }
        }
        pclTest->bOnlySaveMinMaxPinResult = true;

    }

    if((m_nPass == 2) && (m_clGdfTestResult.ResUnit.count()>1) && pclTest->bOnlySaveMinMaxPinResult)
    {
        // Save only the min and the max for this MPR
        int     iExp;
        float   fMin;
        float   fMax;
        float   fFloat;
        QString strMinUnit;
        QString strMaxUnit;
        QString strUnit;
        QString strCurrentUnit;
        QString strCurrentFloat;
        QString strMinFloat;
        QString strMaxFloat;

        strCurrentFloat = m_clGdfTestResult.ResUnit.takeFirst();
        strCurrentUnit = m_clGdfTestResult.ResUnit.takeFirst();
        fFloat = ToFloat(strCurrentFloat);
        strUnit = strCurrentUnit;
        iExp = PrefixUnitToScall(strUnit);
        strUnit = PrefixUnitToUnit(strUnit);
        fFloat /= GS_POW(10.0,iExp);
        fMin = fMax = fFloat;
        strMinFloat = strMaxFloat = strCurrentFloat;
        strMinUnit = strMaxUnit = strCurrentUnit;

        while(!m_clGdfTestResult.ResUnit.isEmpty())
        {

            strCurrentFloat = m_clGdfTestResult.ResUnit.takeFirst();
            strCurrentUnit = m_clGdfTestResult.ResUnit.takeFirst();
            fFloat = ToFloat(strCurrentFloat);
            strUnit = strCurrentUnit;
            iExp = PrefixUnitToScall(strUnit);
            strUnit = PrefixUnitToUnit(strUnit);
            fFloat /= GS_POW(10,iExp);

            if(fFloat < fMin)
            {
                fMin =  fFloat;
                strMinUnit = strCurrentUnit;
                strMinFloat = strCurrentFloat;
            }
            if(fFloat > fMax)
            {
                fMax =  fFloat;
                strMaxUnit = strCurrentUnit;
                strMaxFloat = strCurrentFloat;
            }
        }

        m_clGdfTestResult.ResUnit.clear();
        m_clGdfTestResult.ResUnit.append(strMinFloat);
        m_clGdfTestResult.ResUnit.append(strMinUnit);
        m_clGdfTestResult.ResUnit.append(strMinFloat);
        m_clGdfTestResult.ResUnit.append(strMinUnit);
        m_clGdfTestResult.ResUnit.append(strMaxFloat);
        m_clGdfTestResult.ResUnit.append(strMaxUnit);


    }

    /*
     * Strangely, everytime this function is called, PIR=PRR-1 even with the crapy file from ASE
     if (mNumOfPRR!=mNumOfPIR)
        GSLOG(SYSLOG_SEV_NOTICE, QString("Illegal part regarding PIR(%1) PRR(%2)").arg(mNumOfPRR).arg(mNumOfPIR)
              .toLatin1().data()
              );
    */

    if(pclTest->nTestType == 1)
        WritePtr();
    else
        if(pclTest->nTestType == 2)
            WriteMpr();
        else
            WriteFtr();

    return true;
}

//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WritePtr(CGGdfTestResult* pclTestResultParam, CGGdfTest*  pclTestParam)
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    CGGdfTestResult* pclTestResult;
    if(pclTestResultParam)
        pclTestResult = pclTestResultParam;
    else
        pclTestResult = &m_clGdfTestResult;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write PTR
    RecordReadInfo.iRecordType = 15;
    RecordReadInfo.iRecordSubType = 10;

    int     iExp;
    bool    bHaveLimit = false;
    BYTE    bData = 0;
    float   fFloat;
    QString strValue;
    QString strUnit;
    CGGdfPin*   pclPin = NULL;
    CGGdfTest*  pclTest= pclTestParam;
    CGGdfLimit* pclLimit=NULL;

    if(pclTestResult->Result > 1) // bad result (PFNU)
        return;

    if(pclTest == NULL)
        pclTest = GetTest(pclTestResult->test_name);

    if(pclTest->test_num == 0)
    {
        // is a new test
        // save the current test_num
        pclTest->test_num = pclTestResult->test_num;
    }
    //else
    //  m_clGdfTestResult.test_num = pclTest->test_num;
    // save information
    pclTest->seq_name = pclTestResult->seq_name;
    pclTest->nTestType = 1;
    for (int i = 0; i < pclTestResult->ResUnit.count(); ++i)
    {
        bHaveLimit = false;
        //if(!pclTest->bLimitSaved)
        bHaveLimit = pclTest->HaveLimit(i/2,pclTestResult->PinName);
        if(bHaveLimit)
            pclLimit = pclTest->GetLimit(i/2,pclTestResult->PinName);
        pclPin = pclTest->GetPin(i/2,pclTestResult->PinName);
        pclPin->test_num = pclTest->test_num;
        pclPin->results.exec_cnt++;
        if(!pclTestResult->Result)
            pclPin->results.fail_cnt++;

        m_pStdfFile->WriteHeader(&RecordReadInfo);

        m_pStdfFile->WriteDword(pclTest->test_num);     // Test Number
        m_pStdfFile->WriteByte(1);                              // Test head
        m_pStdfFile->WriteByte(m_clGdfDutResult.bSite ? m_clGdfDutResult.Site : 1); // Tester site:1,2,3,4 or 5

        strValue = pclTestResult->ResUnit[i++];
        if(pclTestResult->ResUnit.count() > 1)
            strUnit = pclTestResult->ResUnit[i];
        else
            strUnit = "";

        bData = 0;
        if(strValue.isEmpty())
        {
            // No result available
            fFloat = 0;
            iExp = 0;
            strUnit = "";
            bData |= BIT1;
        }
        else
        {
            fFloat = ToFloat(strValue);
            iExp = PrefixUnitToScall(strUnit);
            fFloat /= GS_POW(10,iExp);
            strUnit = PrefixUnitToUnit(strUnit);
        }
        //iExp = 0;
        if(pclTestResult->Result == -1)
            m_pStdfFile->WriteByte(bData|BIT6); // NO P/F info      // TEST_FLG
        else
            if(pclTestResult->Result)
                m_pStdfFile->WriteByte(bData);      // passed           // TEST_FLG
            else
                m_pStdfFile->WriteByte(bData|BIT7); // failed           // TEST_FLG
        m_pStdfFile->WriteByte(BIT6|BIT7);                      // PARAM_FLG
        m_pStdfFile->WriteFloat(fFloat);                        // Test result

        // Build test name + pin name (if any)
        QString strTestName = pclTestResult->test_name;
        if(!pclTestResult->PinName.isEmpty())
        {
            strTestName += " - " + pclTestResult->PinName;
        }

        m_pStdfFile->WriteString(strTestName.toLatin1().constData());       // TEST_TXT

        // case 7860
        if(bHaveLimit && m_clGdfDutResult.bSite)
        {
            // save limits for each site
            if(!pclTest->lstLimitSavedForSites.contains(m_clGdfDutResult.Site))
            {
                pclTest->lstLimitSavedForSites << m_clGdfDutResult.Site;
                pclTest->bLimitSaved = false;
            }
        }

        if(bHaveLimit && !pclTest->bLimitSaved)
        {
            m_pStdfFile->WriteString("");                   // ALARM_ID
            if(iExp == 0)
                m_pStdfFile->WriteByte(BIT0|BIT1);          // OPT_FLAG
            else
                m_pStdfFile->WriteByte(BIT1);               // OPT_FLAG
            m_pStdfFile->WriteByte(iExp);                   // SCAL
            m_pStdfFile->WriteByte(pclLimit->llm_scal);     // SCAL
            m_pStdfFile->WriteByte(pclLimit->hlm_scal);     // SCAL
            m_pStdfFile->WriteFloat(pclLimit->lo_limit);    // LIMIT
            m_pStdfFile->WriteFloat(pclLimit->hi_limit);    // LIMIT
            m_pStdfFile->WriteString(pclLimit->units.toLatin1().constData());// TEST_UNIT
            m_pStdfFile->WriteString("");                   //
            m_pStdfFile->WriteString("");                   //
            m_pStdfFile->WriteString("");                   //
            m_pStdfFile->WriteFloat(pclLimit->lo_spec);     // LIMIT
            m_pStdfFile->WriteFloat(pclLimit->hi_spec);     // LIMIT
            pclTest->bLimitSaved = true;
            pclTest->bTestSaved = true;
        }
        else if(!pclTest->bTestSaved)
        {
            m_pStdfFile->WriteString("");                           // ALARM_ID
            m_pStdfFile->WriteByte(BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);// OPT_FLAG
            m_pStdfFile->WriteByte(iExp);                   // SCAL
            m_pStdfFile->WriteByte(0);                      // SCAL
            m_pStdfFile->WriteByte(0);                      // SCAL
            m_pStdfFile->WriteFloat(0);                     // LIMIT
            m_pStdfFile->WriteFloat(0);                     // LIMIT
            m_pStdfFile->WriteString(strUnit.toLatin1().constData());// TEST_UNIT
            m_pStdfFile->WriteString("");                   //
            m_pStdfFile->WriteString("");                   //
            m_pStdfFile->WriteString("");                   //
            m_pStdfFile->WriteFloat(0);                     // LIMIT
            m_pStdfFile->WriteFloat(0);                     // LIMIT
            pclTest->bTestSaved = true;
        }

        m_pStdfFile->WriteRecord();
    }
}


//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteMpr(CGGdfTestResult* pclTestResultParam, CGGdfTest*  pclTestParam)
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    CGGdfTestResult* pclTestResult=0;
    if(pclTestResultParam)
        pclTestResult = pclTestResultParam;
    else
        pclTestResult = &m_clGdfTestResult;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write MPR
    RecordReadInfo.iRecordType = 15;
    RecordReadInfo.iRecordSubType = 15;

    int     iExp=0;
    bool    bHaveLimit = false;
    float   fFloat;
    QString strUnit;
    CGGdfTest*  pclTest=pclTestParam;
    CGGdfLimit* pclLimit=NULL;

    if(pclTestResult->Result > 1) // bad result (PFNU)
        return;

    // No pin result info but MPR
    if(pclTestResult->ResUnit.count() < 2) // bad result
        return;

    if(pclTest == NULL)
        pclTest = GetTest(pclTestResult->test_name);

    if(pclTest->test_num == 0)
    {
        // is a new test
        // save the current test_num
        pclTest->test_num = pclTestResult->test_num;
    }
    //else
    //  m_clGdfTestResult.test_num = pclTest->test_num;
    // save information
    pclTest->seq_name = pclTestResult->seq_name;
    pclTest->nTestType = 2;

    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteDword(pclTest->test_num);             // Test Number
    m_pStdfFile->WriteByte(1);                              // Test head
    m_pStdfFile->WriteByte(m_clGdfDutResult.bSite ? m_clGdfDutResult.Site : 1); // Tester site

    if(pclTestResult->Result == -1)
        m_pStdfFile->WriteByte(BIT6);   // NO P/F info      // TEST_FLG
    else
        if(pclTestResult->Result)
            m_pStdfFile->WriteByte(0);      // passed           // TEST_FLG
        else
            m_pStdfFile->WriteByte(BIT7);   // failed           // TEST_FLG

    m_pStdfFile->WriteByte(BIT6|BIT7);                      // PARAM_FLG
    m_pStdfFile->WriteWord(pclTestResult->ResUnit.count()/2 - 1);   // RTN_ICNT
    m_pStdfFile->WriteWord(pclTestResult->ResUnit.count()/2 - 1);   // RSLT_CNT
    for (int i = 0; i != ((pclTestResult->ResUnit.count()-2)/2+1)/2; ++i) {
        m_pStdfFile->WriteByte(0);                  // RTN_STAT
    }

    bHaveLimit = pclTest->HaveLimit(0,"");

    // case 7860
    if(bHaveLimit && m_clGdfDutResult.bSite)
    {
        // save limits for each site
        if(!pclTest->lstLimitSavedForSites.contains(m_clGdfDutResult.Site))
        {
            pclTest->lstLimitSavedForSites << m_clGdfDutResult.Site;
            pclTest->bLimitSaved = false;
        }
    }

    if(bHaveLimit && pclTest->bLimitSaved)
        bHaveLimit = false;

    if(bHaveLimit)
        pclLimit = pclTest->GetLimit(0,"");

    for (int i = 2; i < pclTestResult->ResUnit.count(); ++i) {
        fFloat = ToFloat(pclTestResult->ResUnit[i++]);
        iExp = PrefixUnitToScall(pclTestResult->ResUnit[i]);
        fFloat /= GS_POW(10,iExp);
        //iExp = 0;
        strUnit = PrefixUnitToUnit(pclTestResult->ResUnit[i]);

        m_pStdfFile->WriteFloat(fFloat);                        // Test result
    }

    // Build test name + pin name (if any)
    QString strTestName = pclTestResult->test_name;
    m_pStdfFile->WriteString(strTestName.toLatin1().constData());       // TEST_TXT
    m_pStdfFile->WriteString("");                               // ALARM_ID
    // SC : 2011 01
    // Because some product can use specific option that save only pin result when FAIL
    // some MPR can have different number of pin in a same file
    // Have to always save RTN_INDX
    //if(!pclTest->bTestSaved) // already saved
    {
        BYTE bData = 0;
        if(iExp == 0)
            bData |= BIT0;
        bData |= BIT1|BIT2|BIT3;
        if(!bHaveLimit)
            bData |= BIT4 | BIT5;
        m_pStdfFile->WriteByte(bData);					// OPT_FLAG
        m_pStdfFile->WriteByte(iExp);					// SCAL
        if(bHaveLimit)
        {
            m_pStdfFile->WriteByte(pclLimit->llm_scal);		// SCAL
            m_pStdfFile->WriteByte(pclLimit->hlm_scal);		// SCAL
            m_pStdfFile->WriteFloat(pclLimit->lo_limit);	// LIMIT
            m_pStdfFile->WriteFloat(pclLimit->hi_limit);	// LIMIT
        }
        else
        {
            m_pStdfFile->WriteByte(0);		// SCAL
            m_pStdfFile->WriteByte(0);		// SCAL
            m_pStdfFile->WriteFloat(0);	// LIMIT
            m_pStdfFile->WriteFloat(0);	// LIMIT
        }

        m_pStdfFile->WriteFloat(0);					// StartIn
        m_pStdfFile->WriteFloat(0);					// IncrIn

        for (int i = 2; i < pclTestResult->ResUnit.count(); ++i) {
            strTestName =
                    QString::number(i/2) + " - " +  pclTestResult->test_name;
            m_pStdfFile->WriteWord(m_mapPinNum[strTestName]);	// RTN_INDX
            ++i;
        }
        m_pStdfFile->WriteString(strUnit.toLatin1().constData());// TEST_UNIT
        m_pStdfFile->WriteString("");					//
        m_pStdfFile->WriteString("");					//
        m_pStdfFile->WriteString("");					//
        m_pStdfFile->WriteString("");					//
        m_pStdfFile->WriteFloat(0);		// LIMIT
        m_pStdfFile->WriteFloat(0);		// LIMIT
        pclTest->bLimitSaved = true;
        pclTest->bTestSaved = true;
    }

    m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void CGGDFtoSTDF::WriteFtr(CGGdfTest*	pclTestParam)
{
    if(m_nPass == 1)
        return;

    if(!m_bMirWriten)
        return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    // Write FTR
    RecordReadInfo.iRecordType = 15;
    RecordReadInfo.iRecordSubType = 20;

    if(m_clGdfTestResult.Result > 1)					// bad result (PFNU)
        return;

    // save information
    CGGdfTest* pclTest = pclTestParam;

    if(pclTest == NULL)
        pclTest = GetTest(m_clGdfTestResult.test_name);

    if(pclTest->test_num == 0)
    {
        // is a new test
        // save the current test_num
        pclTest->test_num = m_clGdfTestResult.test_num;
    }
    //else
    //	m_clGdfTestResult.test_num = pclTest->test_num;
    pclTest->seq_name = m_clGdfTestResult.seq_name;
    pclTest->nTestType = 3;


    if(m_clGdfTestResult.Result)
    {
        // Reset PinName
        m_clGdfTestResult.PinName = "";
    }
    else
    {
        CGGdfPin* pPin = pclTest->GetPin(0,m_clGdfTestResult.PinName); // take the good pin
        pPin->test_num = pclTest->test_num;
        pPin->results.exec_cnt++;
        pPin->results.fail_cnt++;

    }




    m_pStdfFile->WriteHeader(&RecordReadInfo);
    m_pStdfFile->WriteDword(pclTest->test_num);// Test Number
    m_pStdfFile->WriteByte(1);							// Test head
    m_pStdfFile->WriteByte(m_clGdfDutResult.bSite ? m_clGdfDutResult.Site : 1);	// Tester site:1,2,3,4 or 5, etc.
    if(m_clGdfTestResult.Result == -1)
        m_pStdfFile->WriteByte(BIT6);	// no P/F info		// TEST_FLG
    else
        if(m_clGdfTestResult.Result)
            m_pStdfFile->WriteByte(0);		// passed			// TEST_FLG
        else
            m_pStdfFile->WriteByte(BIT7);	// failed

    // save empty field for report_readfile.cpp
    m_pStdfFile->WriteByte(255);	// opt_flg

    // Always save all
    // GEX doesn't like small FTR records !!!
    //if(!pclTest->bTestSaved) // already saved
    {

        m_pStdfFile->WriteDword(0);		// cycl_cnt
        m_pStdfFile->WriteDword(0);		// rel_vadr
        m_pStdfFile->WriteDword(0);		// rept_cnt
        m_pStdfFile->WriteDword(0);		// num_fail
        m_pStdfFile->WriteDword(0);		// xfail_ad
        m_pStdfFile->WriteDword(0);		// yfail_ad
        m_pStdfFile->WriteWord(0);		// vect_off
        m_pStdfFile->WriteWord(0);		// rtn_icnt
        m_pStdfFile->WriteWord(0);
        m_pStdfFile->WriteWord(0);
        if(!pclTest->vector_name.isEmpty())
            m_pStdfFile->WriteString(pclTest->vector_name.toLatin1().constData());	// vect_name
        else
            m_pStdfFile->WriteString(m_clGdfTestResult.PinName.toLatin1().constData());	// vect_name
        m_pStdfFile->WriteString("");	// time_set
        m_pStdfFile->WriteString("");	// op_code
        m_pStdfFile->WriteString(m_clGdfTestResult.test_name.toLatin1().constData());	// test_txt: test name
        m_pStdfFile->WriteString("");	// alarm_id
        m_pStdfFile->WriteString("");	// prog_txt
        m_pStdfFile->WriteString("");	// rslt_txt
        //m_pStdfFile->WriteByte(0);	// patg_num
        //m_pStdfFile->WriteString("");	// spin_map
    }

    pclTest->bTestSaved = true;

    m_pStdfFile->WriteRecord();

}


//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGGDFtoSTDF::ReadLine()
{

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        int iCurrentFilePos = (int) m_phGdfFile->device()->pos();
        int iCurrentProgressStep = m_iProgressStep;
        while(iCurrentFilePos > m_iNextFilePos)
        {
            m_iProgressStep += 100/m_iFileSize + 1;
            m_iNextFilePos  += m_iFileSize/100 + 1;
        }
        if(iCurrentProgressStep < m_iProgressStep)
        {
            GexProgressBar->setValue(m_iProgressStep);
            QCoreApplication::processEvents();
        }
    }
    else
    {
        QCoreApplication::processEvents();
    }


    do
    {
        m_strLine = m_phGdfFile->readLine();
        if(m_strLine.indexOf('\t') > -1)
            m_strLine = m_strLine.replace('\t',' ');
        m_nCurrentLine++;
    }
    while(!m_phGdfFile->atEnd() && !m_strLine.isNull() && m_strLine.isEmpty());

#if 0
    // Ignore this code
    // only saved pertinante marker ...
    if(m_nPass == 2)
    {
        if((m_strLine.contains("begin_",false)) || (m_strLine.contains("end_",false)))
        {
            QString strLine = m_strLine; // Current marker if any
            strLine = strLine.simplified();
            if(!(strLine.left(2) == "--"))
            {
                // extract the word, if no space, return the word
                // remove all space
                strLine = strLine.left(strLine.indexOf(' ')).toLower();
                if((strLine.left(6) == "begin_")
                        && (strLine.indexOf("_format") == -1))
                {
                    // GCORE-2043
                    // Check if this is a keyword
                    // Data_format_test: or Begin_data_tf: or Begin_user_data: or ...
                    // Must start with a char
                    int iPos = -1;
                    if(!strLine.simplified().isEmpty()
                            && strLine.simplified().at(0).isLetter())
                        iPos = strLine.indexOf(':');

                    if((iPos > 0) && (iPos < 40))
                    {
                        m_stackMarkerTag.push_front(strLine.left(iPos));
                        m_stackMarkerLine.push_front(QString::number(m_nCurrentLine));
                    }
                }
                if((strLine.left(4) == "end_")
                        && (strLine.indexOf("_format") == -1))
                {
                    if(!m_stackMarkerTag.isEmpty())
                        m_stackMarkerTag.pop_front();
                    if(!m_stackMarkerLine.isEmpty())
                        m_stackMarkerLine.pop_front();
                }
            }
        }
    }
#endif

    return m_strLine;

}

//////////////////////////////////////////////////////////////////////
// Structures
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// CGGdfDutResult
//////////////////////////////////////////////////////////////////////
CGGdfDutResult::CGGdfDutResult()
{
    nFlags = WAFER_ID;
    bSite = false;
    reset();
}


void CGGdfDutResult::reset()
{
    X		= -32768;
    Y		= -32768;
    Pkg_id	= -1;
    SW_Bin	= -1;
    HW_Bin	= -1;
    P_F		= -1;
    Time	= -1;
    Temp	= -1;
    FF		= -1;
    TF		= -1;
    TT		= -1;
    Site	= -1;
    Bin_desc= " ";
    Retest	= 0;
}

//////////////////////////////////////////////////////////////////////
// CGGdfTestResult
//////////////////////////////////////////////////////////////////////
CGGdfTestResult::CGGdfTestResult()
{
    test_num	= 0;
    test_name	= "";
    seq_name	= "";
    reset();
}

void CGGdfTestResult::reset()
{
    //test_name	= ""; // save the last result for pin data
    //seq_name	= "";
    Result		= -1;
    ResUnit.clear();
    PinName		= "";
    VectorName	= "";
    LimitName	= "";
};

//////////////////////////////////////////////////////////////////////
// CGGdfSummary
//////////////////////////////////////////////////////////////////////
CGGdfSummary::CGGdfSummary()
{
    NumberOfSite = 0;
    pSite = NULL;
    reset();
}

void CGGdfSummary::reset()
{
    int i;
    SW_Bin	= -1;
    HW_Bin	= -1;
    P_F		= -1;
    Number	= -1;
    Prcnt	= -1;
    Bin_desc= " ";

    for(i=0; i<NumberOfSite;i++)
        pSite[i]=0;
}

//////////////////////////////////////////////////////////////////////
// CGGdfSbr
//////////////////////////////////////////////////////////////////////
CGGdfSbr::CGGdfSbr(int iSW_Bin,int iP_F,int iNumber,float fPrcnt,QString strBin_desc)
{
    SW_Bin		= iSW_Bin;
    P_F			= iP_F;
    Number		= iNumber;
    Prcnt		= fPrcnt;
    Bin_desc	= strBin_desc;
}

//////////////////////////////////////////////////////////////////////
// CGGdfPartCnt
//////////////////////////////////////////////////////////////////////
CGGdfPartCnt::CGGdfPartCnt()
{
    bSite = false;
    reset();
}
void CGGdfPartCnt::reset()
{
    Site	= -1;
    Number	= -1;
    Prcnt	= -1;
}

//////////////////////////////////////////////////////////////////////
// CGGdfTest
//////////////////////////////////////////////////////////////////////
CGGdfTest::CGGdfTest()
{
    bLimitSaved	= false;
    bTestSaved	= false;
    test_num	= 0;
    test_name   = "";
    seq_name    = "";
    vector_name = "";
    spreadFlag  = 0;
    nTestType   = 4;
    bOnlySaveMinMaxPinResult = false;
    pin_list.clear();
    limit_list.clear();
    nPinCount=0;
}


CGGdfTest::~CGGdfTest()
{
    QMap<QString,CGGdfPin*>::Iterator itPin;
    for ( itPin = pin_list.begin(); itPin != pin_list.end(); ++itPin )
    {
        delete itPin.value();
    }
    pin_list.clear();
    QMap<QString,CGGdfLimit*>::Iterator itLimit;
    for ( itLimit = limit_list.begin(); itLimit != limit_list.end(); ++itLimit )
    {
        delete itLimit.value();
    }
    limit_list.clear();
}

CGGdfPin* CGGdfTest::GetPin(int iIndex, QString strPinName)
{
    QString     strKey;
    CGGdfPin*   pPin;

    strKey.sprintf("%d",iIndex);
    strKey += strPinName;
    if(pin_list.contains(strKey))
    {   // exist
        QMap<QString,CGGdfPin*>::Iterator itPin;
        itPin = pin_list.find(strKey);
        pPin = itPin.value();
    }
    else
    {   // create it
        pPin = new CGGdfPin();
        pin_list.insert(strKey,pPin);
        pPin->testNameSuffix = strKey;
    }
    return pPin;
}

CGGdfLimit* CGGdfTest::GetLimit(int iIndex, QString strLimitName)
{
    QString     strKey;
    CGGdfLimit* pLimit = NULL;

    strKey.sprintf("%d",iIndex);
    strKey += strLimitName;
    if(limit_list.contains(strKey))
    {   // exist
        QMap<QString,CGGdfLimit*>::Iterator itLimit;
        itLimit = limit_list.find(strKey);
        pLimit = itLimit.value();
    }
    else
    {   // create it
        pLimit = new CGGdfLimit();
        limit_list.insert(strKey,pLimit);
        pLimit->limitName = strLimitName;
    }
    return pLimit;
}

bool CGGdfTest::HaveLimit(int iIndex, QString strLimitName)
{
    QString strKey;

    strKey.sprintf("%d",iIndex);
    strKey += strLimitName;

    return limit_list.contains(strKey);
}

//////////////////////////////////////////////////////////////////////
// CGGdfResults
//////////////////////////////////////////////////////////////////////
CGGdfResults::CGGdfResults()
{
    exec_cnt    = 0;
    fail_cnt    = 0;
    alarm_tests = 0;
    opt_flag    = 0;
    pad_byte    = 0;
    test_min    = 0;
    test_max    = 0;
    tst_mean    = 0;
    tst_sdev    = 0;
    tst_sums    = 0;
    tst_sqrs    = 0;
}

//////////////////////////////////////////////////////////////////////
// CGGdfPin
//////////////////////////////////////////////////////////////////////
CGGdfPin::CGGdfPin()
{
    testNameSuffix = "";
    test_num = 0;
}

//////////////////////////////////////////////////////////////////////
// CGGdfLimit
//////////////////////////////////////////////////////////////////////
CGGdfLimit::CGGdfLimit()
{
    opt_flag    = ' ';
    units       = "";
    llm_scal    = 0;
    hlm_scal    = 0;
    lo_limit    = 0;
    low_limit_param="";
    hi_limit    = 0;
    high_limit_param="";
    lo_spec     = 0;
    hi_spec     = 0;
    limitName   = "";
}

bool CGGdfLimit::SetLimit(QString strLowUnits, float fLowValue,
                          QString strHighUnits, float fHighValue,
                          QString strLowParam, QString strHighParam)
{
    bool    bChanged = false;
    int     low_scale_exp, high_scale_exp;
    float   low_limit, high_limit;
    QString strUnit;

    low_scale_exp = PrefixUnitToScall(strLowUnits);
    high_scale_exp = PrefixUnitToScall(strHighUnits);

    low_limit = fLowValue;
    high_limit = fHighValue;

    low_limit /= GS_POW(10.0,low_scale_exp);
    high_limit /= GS_POW(10.0,high_scale_exp);

    if(!strLowUnits.isEmpty())
    {
        opt_flag &= (~BIT4&~BIT6);  /* low limit is valid */
        strUnit = PrefixUnitToUnit(strLowUnits);
    }
    else
        opt_flag |= (BIT4|BIT6);    /* low limit is invalid */
    if(!strHighUnits.isEmpty())
    {
        opt_flag &= (~BIT5&~BIT7);  /* high limit is valid */
        strUnit = PrefixUnitToUnit(strHighUnits);
    }
    else
        opt_flag |= (BIT5|BIT7);    /* high limit is invalid */

    bChanged = ((units != strUnit) || (low_limit_param != strLowParam) || (high_limit_param != strHighParam)
                || (lo_limit != low_limit) || (hi_limit != high_limit));
    units = strUnit;
    low_limit_param = strLowParam;
    high_limit_param = strHighParam;
    llm_scal = 0;//low_scale_exp;
    hlm_scal = 0;//high_scale_exp;
    lo_limit = low_limit;
    hi_limit = high_limit;
    lo_spec = 0;
    hi_spec = 0;

    return bChanged;
}

//////////////////////////////////////////////////////////////////////
// CGGdfPinChannel
//////////////////////////////////////////////////////////////////////
CGGdfPinChannel::CGGdfPinChannel()
{
    reset();
}

CGGdfPinChannel::~CGGdfPinChannel()
{
    reset();
}

void CGGdfPinChannel::reset()
{
    iBitPosition = 0;
    lstChannel.clear();
    strPinName = "";

}


//////////////////////////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
int PrefixUnitToScall(QString strUnit)
{
    if(strUnit.length() < 2) return 0;

    QString strPrefix = strUnit.left(2);
    if(strPrefix == "EX") return -18;
    if(strPrefix == "PE") return -15;

    strPrefix = strUnit.left(1);
    if(strPrefix == "T") return -12;
    if(strPrefix == "G") return -9;
    if(strPrefix == "M") return -6;
    if(strPrefix == "K") return -3;
    if(strPrefix == "%") return 2;
    if(strPrefix == "m") return 3;
    if(strPrefix == "u") return 6;
    if(strPrefix == "n") return 9;
    if(strPrefix == "p") return 12;
    if(strPrefix == "f") return 15;
    if(strPrefix == "a") return 18;

    return 0;
}

//////////////////////////////////////////////////////////////////////
QString PrefixUnitToUnit(QString strUnit)
{
    QString strPrefix;
    if(strUnit.length() < 2)
        return strUnit;

    strPrefix = strUnit.left(2);
    if((strPrefix == "EX") || (strPrefix == "PE"))
        return strUnit.right(strUnit.length()-2);

    strPrefix = strUnit.left(1);

    if((strPrefix == "T") || (strPrefix == "G") || (strPrefix == "M") || (strPrefix == "K")
            || (strPrefix == "%") || (strPrefix == "m") || (strPrefix == "u") || (strPrefix == "n")
            || (strPrefix == "p") || (strPrefix == "f") || (strPrefix == "a"))
        return strUnit.right(strUnit.length()-1);;

    return strUnit;
}

//////////////////////////////////////////////////////////////////////
int StringToHex(QString strHexa)
{
    int nInteger = 0;
    if(!strHexa.isEmpty())
        sscanf(strHexa.toLatin1().constData(),"%x",&nInteger);
    return nInteger;
}

