//////////////////////////////////////////////////////////////////////
// import_tango.cpp: Convert a TANGO file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>


#include "import_tango.h"
#include "import_constants.h"
#include "engine.h"

// File format:
//<?xml version="1.0" ?>
//<TANGO_CP_FORMAT>
//<HEADER>
//<VERSION>1.4</VERSION>
//<LOT_ID>MB33129.1</LOT_ID>
//<OP_NAME>CP1</OP_NAME>
//<WAF_NO>1</WAF_NO>
//<WAFER_ID>MB33129.1-01</WAFER_ID>
//<PRODUCT_ID>QA6140</PRODUCT_ID>
//<TEST_FLAG>101</TEST_FLAG>
//<TEST_ROUND>R1</TEST_ROUND>
//<GROSS_DIE>186</GROSS_DIE>
//<PASS_CNT>173</PASS_CNT>
//<EQP_ID>TS-51</EQP_ID>
//<EQP_NAME></EQP_NAME>
//<SUBSYS_ID></SUBSYS_ID>
//<OPERATOR_ID>KYEC</OPERATOR_ID>
//<TEST_PG>1A6140W1A2</TEST_PG>
//<ST_TIME>2007-01-29 18:09:00</ST_TIME>
//<END_TIME>2007-01-29 18:24:00</END_TIME>
//<PROB_CARD_ID>PT0101408701</PROB_CARD_ID>
//<LOAD_BOARD_ID></LOAD_BOARD_ID>
//<TEMPERATURE>0</TEMPERATURE>
//<BIN_DEF_NAME>QA6140.CP1</BIN_DEF_NAME>
//<VENDOR_ID>ORISE</VENDOR_ID>
//<VENDORLOT_ID>B33129.1</VENDORLOT_ID>
//<NOTCH>R</NOTCH>
//<EXTEND_INFO>130004297-00001</EXTEND_INFO>
//<FAB_LOT_ID>B33129.1</FAB_LOT_ID>
//<PART_ID>QA6140A-T</PART_ID>
//<TEST_VENDOR_ID>KYEC</TEST_VENDOR_ID>
//<PKG_TYPE>LQFP64L</PKG_TYPE>
//<NOTCH>R</NOTCH>
//<XYDIR>1</XYDIR>
//</HEADER>
//<DIEDATA>
//<BINMAP>
//1 4 1 1
//1 5 1 1
//28 6 1 1
//</BINMAP>
//<DEFECT>
//</DEFECT>
//</DIEDATA>
//</TANGO_CP_FORMAT>

// OR
//
//<?xml version="1.0" ?>
//<TANGO_FT_FORMAT>
//// ...
//</HEADER>
//<LIMITS>
//<BIN>1|1|1|BIN1|0</BIN>
//<BIN>2|2|0|BIN2|0</BIN>
//<BIN>3|3|0|BIN3|0</BIN>
//<BIN>4|4|0|BIN4|0</BIN>
//<BIN>5|5|0|BIN5|0</BIN>
//<CAT>1|1|1|PASS|0</CAT>
//<CAT>2|3|0|SCAN_H|0</CAT>
//<CAT>3|3|0|SCAN_H|0</CAT>
//<CAT>4|5|0|SCAN_L|0</CAT>
//<CAT>71|2|0|t_osc_25MHz|0</CAT>
//</LIMITS>
//<FTDATA>
//<BINSUM>
//<BIN>1|95|95|0</BIN>
//<BIN>2|40|40|0</BIN>
//<BIN>3|102|102|0</BIN>
//<BIN>5|28|28|0</BIN>
//</BINSUM>
//<CATSUM>
//<CAT>1|95|95|0</CAT>
//<CAT>65|5|5|0</CAT>
//<CAT>63|1|1|0</CAT>
//</CATSUM>
//<LOTSUM>
//</LOTSUM>
//</FTDATA>
//<FAILURE_CNT></FAILURE_CNT>
//<DATALOG></DATALOG>
//</TANGO_FT_FORMAT>

// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar	*   GexProgressBar;         // Handle to progress bar in status bar



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTANGOBin::CTANGOBin(int iSWBin,	int iHWBin,	int iPF, QString strBinDesc, int nCnt)
{
    iSBin = iSWBin;
    iHBin = iHWBin;
    iPass = iPF;
    strBinName = strBinDesc;
    iCnt = nCnt;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTANGOtoSTDF::CTANGOtoSTDF()
{
    m_lStartTime    = 0;
    m_lEndTime      = 0;
    m_bFinalTest    = true;
    m_bBinSummary = false;
    m_cWaferFlat    = ' ';
    m_cPosX         = ' ';
    m_cPosY         = ' ';
    m_cRetestIndex  = '0';
}

CTANGOtoSTDF::~CTANGOtoSTDF()
{
    QMap<int,CTANGOBin*>::Iterator	itBin;
    for(itBin=m_mapSbrList.begin(); itBin!=m_mapSbrList.end(); itBin++)
    {
        delete m_mapSbrList[itBin.key()];
        m_mapSbrList[itBin.key()] = NULL;
    }
    for(itBin=m_mapHbrList.begin(); itBin!=m_mapHbrList.end(); itBin++)
    {
        delete m_mapHbrList[itBin.key()];
        m_mapHbrList[itBin.key()] = NULL;
    }

    m_mapSbrList.clear();
    m_mapHbrList.clear();
    m_mapPartBinning.clear();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CTANGOtoSTDF::GetLastError()
{
    m_strLastError = "Import TANGO: ";

    switch(m_iLastError)
    {
    default:
    case errNoError:
        m_strLastError += "No Error";
        break;
    case errOpenFail:
        m_strLastError += "Failed to open file";
        break;
    case errInvalidFormat:
        m_strLastError += "Invalid file format";
        break;
    case errWriteSTDF:
        m_strLastError += "Failed creating temporary file. Folder permission issue?";
        break;
    case errLicenceExpired:
        m_strLastError += "License has expired or Data file out of date...";
        break;
    }
    // Return Error Message
    return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Goto the marker szEndMarker
// And position the m_strLine after this marker
//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::GotoMarker(const char *szMarker, QString *pstrValue)
{
    int iPos;
    QString strValue;
    QString strMarker(szMarker);

    // Empty close element
    // Read the open element <BIN /> that is also a close element
    // If search a close element "/"+"TAG" and have the close char "/" in the original tag
    // ex: open = <BIN />, search /BIN / => open=close
    if(strMarker.startsWith("/") && strMarker.endsWith("/"))
        return true;

    strMarker = "<";
    strMarker += szMarker;
    strMarker += ">";
    QRegExp regTag(strMarker, Qt::CaseInsensitive);

    strValue = "";
    while(!m_pTangoFile->atEnd())
    {
        if(m_strLine.isEmpty() || m_strLine == " ")
            m_strLine = m_pTangoFile->readLine();
        if(m_strLine.isEmpty() || m_strLine == " ")
            continue;
        iPos = 0;
        iPos = regTag.indexIn(m_strLine, iPos);
        if(iPos >= 0)
        {
            // Go after the marker
            strValue += m_strLine.left(iPos);
            m_strLine = m_strLine.mid(iPos + strMarker.length());
            if(pstrValue)
                (*pstrValue) = strValue;
            return true;
        }
        strValue += m_strLine;
        m_strLine = "";
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// Find the next marker and return it's name
//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::NextMarker(QString *pstrNextMarker)
{
    int iPos;
    QRegExp regTag("<([^>]*)>", Qt::CaseInsensitive);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) m_pTangoFile->device()->pos() > iNextFilePos)
        {
            iProgressStep += 100/iFileSize + 1;
            iNextFilePos  += iFileSize/100 + 1;
            GexProgressBar->setValue(iProgressStep);
        }
    }
    QCoreApplication::processEvents();

    while(!m_pTangoFile->atEnd())
    {

        if(m_strLine.isEmpty() || m_strLine == " ")
            m_strLine = m_pTangoFile->readLine();
        if(m_strLine.isEmpty() || m_strLine == " ")
            continue;
        iPos = 0;
        iPos = regTag.indexIn(m_strLine, iPos);
        if(iPos >= 0)
        {
            // Go after the marker
            // And return the text for this marker
            (*pstrNextMarker) = regTag.cap(1);
            m_strLine = m_strLine.mid(iPos + (*pstrNextMarker).length() + 2);
            return true;
        }
        m_strLine = "";
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::ProcessLimits()
{
    QString     strMarker;
    QString     strValue;
    CTANGOBin*  pBin;
    int         iHBin, iSBin, iPass;
    QString     strPassFail, strBinName;

    while(NextMarker(&strMarker))
    {
        if(strMarker.startsWith("/LIMITS", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("BIN", Qt::CaseInsensitive))
        {
            if(m_bFinalTest)
            {
                //<BIN>1|1|1|BIN1|0</BIN>
                // HardBinNum | HardBinNum | PassFail | HardBinName | ?
                GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strValue);
                if(strValue.isEmpty()) continue;
                iHBin = strValue.section("|",0,0).toInt();
                strPassFail = strValue.section("|",2,2);
                iPass = (strPassFail == "P" || strPassFail == "1") ? 1 : 0;
                strBinName = strValue.section("|",3,3);
                pBin = new CTANGOBin(iHBin, -1, iPass, strBinName);
                m_mapHbrList[iHBin] = pBin;
            }
            else
            {
                //<BIN>1|1|1|BIN1|0</BIN>
                // SoftBinNum | HardBinNum | PassFail | HardBinName | ?
                GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strValue);
                if(strValue.isEmpty()) continue;
                iSBin = strValue.section("|",0,0).toInt();
                iHBin = strValue.section("|",1,1).toInt();
                strPassFail = strValue.section("|",2,2);
                iPass = (strPassFail == "P" || strPassFail == "1") ? 1 : 0;
                strBinName = strValue.section("|",3,3);
                pBin = new CTANGOBin(iSBin, iHBin, iPass, strBinName);
                m_mapSbrList[iSBin] = pBin;
            }
        }
        else if(strMarker.startsWith("CAT", Qt::CaseInsensitive))
        {
            //<CAT>1|1|1|PASS|0</CAT>
            // SoftBinNum | HardBinNum | PassFail | SoftBinName | ?
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strValue);
            if(strValue.isEmpty()) continue;
            iSBin = strValue.section("|",0,0).toInt();
            iHBin = strValue.section("|",1,1).toInt();
            strPassFail = strValue.section("|",2,2);
            iPass = (strPassFail == "P" || strPassFail == "1") ? 1 : 0;
            strBinName = strValue.section("|",3,3);
            pBin = new CTANGOBin(iSBin, iHBin, iPass, strBinName);
            m_mapSbrList[iSBin] = pBin;
        }
        else
            GotoMarker(QString("/"+strMarker).toLatin1().constData());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::ProcessFTData()
{
    QString strMarker;
    QString strValue;
    while(NextMarker(&strMarker))
    {
        if(strMarker.startsWith("/FTDATA", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("BINSUM", Qt::CaseInsensitive))
        {
            ProcessBinSum();
        }
        else if(strMarker.startsWith("CATSUM", Qt::CaseInsensitive))
        {
            ProcessCatSum();
        }
        else
            GotoMarker(QString("/"+strMarker).toLatin1().constData());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

bool CTANGOtoSTDF::ProcessBinSum()
{
    QString     strMarker;
    QString     strValue;
    CTANGOBin*  pBin;
    int         iBin, iCnt;

    m_bBinSummary = true;

    while(NextMarker(&strMarker))
    {
        if(strMarker.startsWith("/BINSUM", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("BIN", Qt::CaseInsensitive))
        {
            //<BIN>1|95|95|0</BIN>
            // HBinNum | Part count | Site1 count | Site2 count ...
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strValue);
            if(strValue.isEmpty()) continue;
            if(m_bFinalTest)
            {
                iBin = strValue.section("|",0,0).toInt();
                iCnt = strValue.section("|",1,1).toInt();
                if(m_mapHbrList.contains(iBin))
                    pBin = m_mapHbrList[iBin];
                else
                {
                    // No limits section before ?
                    pBin = new CTANGOBin(iBin, -1, (iBin==1 ? 1 : 0));
                    m_mapHbrList[iBin] = pBin;
                }
                pBin->iCnt = iCnt;
            }
            else
            {
                iBin = strValue.section("|",0,0).toInt();
                iCnt = strValue.section("|",1,1).toInt();
                if(m_mapSbrList.contains(iBin))
                    pBin = m_mapSbrList[iBin];
                else
                {
                    // No limits section before ?
                    pBin = new CTANGOBin(iBin);
                    m_mapSbrList[iBin] = pBin;
                }
                pBin->iCnt = iCnt;
            }
        }
        else
            GotoMarker(QString("/"+strMarker).toLatin1().constData());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

bool CTANGOtoSTDF::ProcessCatSum()
{
    QString     strMarker;
    QString     strValue;
    CTANGOBin*  pBin;
    int         iBin, iCnt;

    m_bBinSummary = true;

    while(NextMarker(&strMarker))
    {
        if(strMarker.startsWith("/CATSUM", Qt::CaseInsensitive))
            break;

        if(m_bFinalTest && strMarker.startsWith("CAT", Qt::CaseInsensitive))
        {
            //<CAT>1|95|95|0</CAT>
            // SoftBinNum | Count | ? | ?
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strValue);
            if(strValue.isEmpty()) continue;
            iBin = strValue.section("|",0,0).toInt();
            iCnt = strValue.section("|",1,1).toInt();
            if(m_mapSbrList.contains(iBin))
                pBin = m_mapSbrList[iBin];
            else
            {
                // No limits section before ?
                pBin = new CTANGOBin(iBin);
                m_mapSbrList[iBin] = pBin;
            }
            pBin->iCnt = iCnt;
        }
        else
            GotoMarker(QString("/"+strMarker).toLatin1().constData());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

bool CTANGOtoSTDF::ProcessDieData()
{
    QString strMarker;
    while(NextMarker(&strMarker))
    {
        if(strMarker.startsWith("/DIEDATA", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("BINMAP", Qt::CaseInsensitive))
        {
            ProcessBinMap();
        }
        else if(strMarker.startsWith("BINSUM", Qt::CaseInsensitive))
        {
            ProcessBinSum();
        }
        else
            GotoMarker(QString("/"+strMarker).toLatin1().constData());
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

bool CTANGOtoSTDF::ProcessBinMap()
{
    QRegExp regTag("<([^>]*)>", Qt::CaseInsensitive);
    QString strCoord, strBinPart;

    while(!m_pTangoFile->atEnd())
    {

        m_strLine = m_pTangoFile->readLine();
        if(m_strLine.isEmpty() || m_strLine == " ")
            continue;
        //26 6 1 1
        //X  Y SBin Die
        if(regTag.indexIn(m_strLine) >= 0)
            break;
        strCoord = m_strLine.section(" ",0,1);
        strBinPart = m_strLine.section(" ",2);
        m_mapPartBinning[strCoord]=strBinPart;
    }

    GotoMarker("/BINMAP");
    return true;
}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TANGO format
//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::IsCompatible(const char *szFileName)
{
    QString strLine;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hTangoFile(&f);

    // Check if first line is the correct TANGO header...
    //<?xml version="1.0" ?>
    do
        strLine = hTangoFile.readLine();
    while(!strLine.isNull() && strLine.isEmpty());

    if(!(strLine.startsWith("<?xml version", Qt::CaseInsensitive)))
    {
        // Incorrect header...this is not a TANGO file!
        // Close file
        f.close();
        return false;
    }
    //<TANGO_CP_FORMAT>
    //<TANGO_FT_FORMAT>
    do
        strLine = hTangoFile.readLine();
    while(!strLine.isNull() && strLine.isEmpty());

    if(!strLine.startsWith("<TANGO_CP_FORMAT", Qt::CaseInsensitive)&&!strLine.startsWith("<TANGO_FT_FORMAT", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a TANGO file!
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TANGO file
//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::ReadTangoFile(const char *TangoFileName)
{
    QString strDate;
    QString strTime;
    QString strString;
    QString strMarker;
    QString strDateTime;
    int     nYear=0, nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0;
    bool    bOK = true;
    QDateTime   clDateTime;

    // Open TANGO file
    QFile f( TangoFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TANGO file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    m_pTangoFile = new QTextStream(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size() + 1;

    // Check if first line is the correct TANGO header...
    //<?xml version="1.0" ?>
    if((!NextMarker(&strMarker)) || (!strMarker.startsWith("?xml version", Qt::CaseInsensitive)))
    {
        // Incorrect header...this is not a TANGO file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    //<TANGO_CP_FORMAT>
    //<TANGO_FT_FORMAT>
    if((!NextMarker(&strMarker)) ||
            (!(strMarker.startsWith("TANGO_CP_FORMAT", Qt::CaseInsensitive)||strMarker.startsWith("TANGO_FT_FORMAT", Qt::CaseInsensitive))))
    {
        // Incorrect header...this is not a TANGO file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    if(strMarker.startsWith("TANGO_FT_FORMAT", Qt::CaseInsensitive))
        m_bFinalTest = true;
    else
        m_bFinalTest = false;

    if(!GotoMarker("HEADER"))
    {
        // Incorrect header...this is not a TANGO file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    while(NextMarker(&strMarker))
    {
        if(strMarker.startsWith("/HEADER", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("LOT_ID", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strLotID);
        }
        else if(strMarker.startsWith("VENDORLOT_ID", Qt::CaseInsensitive))
        {
            // BG 11 Sep 2009: write vendorlot_id into sublot_id, unless sublot_is already contains a value (ie wafer_id).
            //                 this change is for Adv Silicon, it enables the vendorlot_id to be saved is STDF for FT files
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strString);
            if(m_strSubLotID.isEmpty())
                m_strSubLotID = strString;
        }
        else if(strMarker.startsWith("WAF_NO", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strWaferID);
            m_strSubLotID = m_strWaferID;
        }
        else if(strMarker.startsWith("OPERATOR_ID", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strOperName);
        }
        else if(strMarker.startsWith("PRODUCT_ID", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strPartType);
        }
        else if(strMarker.startsWith("EQP_NAME", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strTesterType);
        }
        else if(strMarker.startsWith("TEST_PG", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strJobName);
        }
        else if(strMarker.startsWith("ST_TIME", Qt::CaseInsensitive))
        {
            //2007-01-29 18:09:00
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strDateTime);
            if(strDateTime.isEmpty()) continue;
            strDate = strDateTime.section(" ", 0,0);
            strTime = strDateTime.section(" ", 1,1);
            nYear = strDate.section("-", 0, 0).toInt(&bOK);
            if(bOK)
                nMonth = strDate.section("-", 1, 1).toInt(&bOK);
            if(bOK)
                nDay = strDate.section("-", 2, 2).toInt(&bOK);
            if(bOK)
                nHour = strTime.section(":", 0, 0).toInt(&bOK);
            if(bOK)
                nMin = strTime.section(":", 1, 1).toInt(&bOK);
            if(bOK)
                nSec = strTime.section(":", 2, 2).toInt(&bOK);
            if(!bOK)
            {
                // Incorrect header...this is not a TANGO file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                // Close file
                f.close();
                return false;
            }
            clDateTime.setDate(QDate(nYear, nMonth, nDay));
            clDateTime.setTime(QTime(nHour, nMin, nSec));
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
        else if(strMarker.startsWith("END_TIME", Qt::CaseInsensitive))
        {
            //2007-01-29 18:09:00
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strDateTime);
            if(strDateTime.isEmpty()) continue;
            strDate = strDateTime.section(" ", 0,0);
            strTime = strDateTime.section(" ", 1,1);
            nYear = strDate.section("-", 0, 0).toInt(&bOK);
            if(bOK)
                nMonth = strDate.section("-", 1, 1).toInt(&bOK);
            if(bOK)
                nDay = strDate.section("-", 2, 2).toInt(&bOK);
            if(bOK)
                nHour = strTime.section(":", 0, 0).toInt(&bOK);
            if(bOK)
                nMin = strTime.section(":", 1, 1).toInt(&bOK);
            if(bOK)
                nSec = strTime.section(":", 2, 2).toInt(&bOK);
            if(!bOK)
            {
                // Incorrect header...this is not a TANGO file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                // Close file
                f.close();
                return false;
            }
            clDateTime.setDate(QDate(nYear, nMonth, nDay));
            clDateTime.setTime(QTime(nHour, nMin, nSec));
            clDateTime.setTimeSpec(Qt::UTC);
            m_lEndTime = clDateTime.toTime_t();
        }
        else if(strMarker.startsWith("TEMPERATURE", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strTestTemperature);
        }
        else if(strMarker.startsWith("OP_NAME", Qt::CaseInsensitive))
        {
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strString);
            if(m_strTestingCode.isEmpty())
                m_strTestingCode = strString;
        }
        else if(strMarker.startsWith("PKG_TYPE", Qt::CaseInsensitive))
        {
            // BG 11 Sep 2009: write PKG_TYPE in corresponding field in STDF.MIR.
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strPkgType);
        }
        else if(strMarker.startsWith("TEST_VENDOR_ID", Qt::CaseInsensitive))
        {
            // BG 11 Sep 2009: write TEST_VENDOR_ID in corresponding field in STDF.MIR.
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strFacility);
        }
        else if(strMarker.startsWith("TEST_FLAG", Qt::CaseInsensitive))
        {
            // BG 11 Sep 2009: write TEST_FLAG in corresponding field in STDF.MIR.
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &m_strTestingCode);
        }
        else if(strMarker.startsWith("TEST_ROUND", Qt::CaseInsensitive))
        {
            // BG 11 Sep 2009: write TEST_ROUND in corresponding field in STDF.MIR.
            //<TEST_ROUND>R1</TEST_ROUND>
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strString);
            if(strString.isEmpty()) continue;
            if((strString.size() == 2) && strString.startsWith("R", Qt::CaseInsensitive))
            {
                strString = strString.mid(1);
                strString.toUInt(&bOK);
                if(bOK)
                    m_cRetestIndex = strString.at(0).toLatin1();
            }
        }
        else if(strMarker.startsWith("NOTCH", Qt::CaseSensitive))
        {
            // BG 15 Sep 2009: support NOTCH field.
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strString);
            strString = strString.toLower();
            if(strString == "u")
                m_cWaferFlat = 'U';
            else if(strString == "r")
                m_cWaferFlat = 'R';
            else if(strString == "d")
                m_cWaferFlat = 'D';
            else if(strString == "l")
                m_cWaferFlat = 'L';
        }
        else if(strMarker.startsWith("XYDIR", Qt::CaseInsensitive))
        {
            // BG 15 Sep 2009: support XYDIR field.
            GotoMarker(QString("/"+strMarker).toLatin1().constData(), &strString);
            if(strString == "1")
            {
                m_cPosX = 'R';
                m_cPosY = 'U';
            }
            else if(strString == "2")
            {
                m_cPosX = 'L';
                m_cPosY = 'U';
            }
            else if(strString == "3")
            {
                m_cPosX = 'L';
                m_cPosY = 'D';
            }
            else if(strString == "4")
            {
                m_cPosX = 'R';
                m_cPosY = 'D';
            }
        }
        else
            GotoMarker(QString("/"+strMarker).toLatin1().constData());

    }

    while(NextMarker(&strMarker))
    {

        if(strMarker.startsWith("/TANGO_FT_FORMAT", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("/TANGO_CP_FORMAT", Qt::CaseInsensitive))
            break;

        if(strMarker.startsWith("LIMITS", Qt::CaseInsensitive))
        {
            ProcessLimits();
        }
        else if(strMarker.startsWith("FTDATA", Qt::CaseInsensitive))
        {
            ProcessFTData();
        }
        else if(strMarker.startsWith("DIEDATA", Qt::CaseInsensitive))
        {
            ProcessDieData();
        }
        else GotoMarker(QString("/"+strMarker).toLatin1().constData());
    }

    // Close file
    f.close();

    // Success parsing TANGO file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TANGO data parsed
//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing TANGO file into STDF database
        m_iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);                  // SUN CPU type
    StdfFile.WriteByte(4);                  // STDF V4
    StdfFile.WriteRecord();

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);          // Setup time
    StdfFile.WriteDword(m_lStartTime);          // Start time
    StdfFile.WriteByte(1);                      // Station
    StdfFile.WriteByte((BYTE) 'P');             // Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE)m_cRetestIndex);   // rtst_cod
    StdfFile.WriteByte((BYTE) ' ');             // prot_cod
    StdfFile.WriteWord(65535);                  // burn_tim
    StdfFile.WriteByte((BYTE) ' ');             // cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());        // Lot ID
    StdfFile.WriteString(m_strPartType.toLatin1().constData());     // Part Type / Product ID
    StdfFile.WriteString(m_strNodeName.toLatin1().constData());     // Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());   // Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());      // Job name
    StdfFile.WriteString("");                   // Job rev
    StdfFile.WriteString(m_strSubLotID.toLatin1().constData());     // sublot-id
    StdfFile.WriteString(m_strOperName.toLatin1().constData());     // operator
    StdfFile.WriteString("");                   // exec-type
    StdfFile.WriteString("");                   // exe-ver
    if(m_strTestingCode.isEmpty())              // test-cod
    {
        if(m_bFinalTest)
            StdfFile.WriteString("");
        else
            StdfFile.WriteString("WAFER");
    }
    else
        StdfFile.WriteString(m_strTestingCode.toLatin1().constData());
    StdfFile.WriteString(m_strTestTemperature.toLatin1().constData());	// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TANGO";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());// user-txt
    StdfFile.WriteString("");                                       // aux-file
    StdfFile.WriteString(m_strPkgType.toLatin1().constData());      // package-type
    StdfFile.WriteString("");                                       // familyID
    StdfFile.WriteString("");                                       // Date-code
    StdfFile.WriteString(m_strFacility.toLatin1().constData());     // Facility-ID
    StdfFile.WriteString("");                                       // FloorID
    StdfFile.WriteString("");                                       // ProcessID
    StdfFile.WriteString("");                                       // OPER_FRQ
    StdfFile.WriteString("");                                       // Spec_nam
    StdfFile.WriteString("");                                       // Spec_ver
    StdfFile.WriteString("");                                       // flow_id
    StdfFile.WriteString("");                                       // SETUP_ID

    StdfFile.WriteRecord();

    int         iWaferX, iWaferY;
    int         iBin=0;
    int         iSBin=0;
    int         iHBin=0;
    bool        bPass;
    long        iTotalGoodBin,iTotalFailBin;
    int         iPartNumber=0;
    int         iPartRetested=0;
    QString     strXY;
    QString     strBinPart;
    CTANGOBin*  pBin;
    bool        bCreateSbr,bCreateHbr;// if no summary information, have to create them
    bCreateSbr = m_mapSbrList.isEmpty();
    bCreateHbr = m_mapHbrList.isEmpty();
    if(!m_bBinSummary)
        bCreateSbr = bCreateHbr = true;

    if(!m_bFinalTest)
    {
        // Write WCR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 30;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteFloat(0.0F);                  // WAFR_SIZ
        StdfFile.WriteFloat(0.0F);                  // DIE_HT
        StdfFile.WriteFloat(0.0F);                  // DIE_WID
        StdfFile.WriteByte((BYTE)0);                // WF_UNITS
        StdfFile.WriteByte((BYTE)m_cWaferFlat);     // WF_FLAT
        StdfFile.WriteWord((WORD)-32768);           // CENTER_X
        StdfFile.WriteWord((WORD)-32768);           // CENTER_Y
        StdfFile.WriteByte((BYTE)m_cPosX);          // POS_X
        StdfFile.WriteByte((BYTE)m_cPosY);          // POS_Y
        StdfFile.WriteRecord();

        // Write WIR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);                      // Test head
        StdfFile.WriteByte(255);                    // Tester site (all)
        StdfFile.WriteDword(m_lStartTime);          // Start time
        StdfFile.WriteString(m_strWaferID.toLatin1().constData());// WaferID
        StdfFile.WriteRecord();
    }


    // Write all Parts result read on this wafer.: PIR....PRR
    iTotalGoodBin=iTotalFailBin=0;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    m_iTotalParameters = m_mapPartBinning.count();
    QStringList	clPartList;
    QMap<QString,QString>::Iterator itPartInfo;
    for(itPartInfo=m_mapPartBinning.begin(); itPartInfo!=m_mapPartBinning.end(); itPartInfo++)
    {

        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if(GexProgressBar != NULL)
        {
            m_iParameterCount++;
            while(m_iParameterCount > m_iNextParameter)
            {
                iProgressStep += 100/m_iTotalParameters + 1;
                m_iNextParameter  += m_iTotalParameters/100 + 1;
                GexProgressBar->setValue(iProgressStep);
            }
        }
        QCoreApplication::processEvents();

        // Parse part list

        strXY = itPartInfo.key();
        if(clPartList.contains(strXY))
            iPartRetested++;
        else
            clPartList.append(strXY);

        iWaferX = strXY.section(" ",0,0).simplified().toInt();
        iWaferY = strXY.section(" ",1,1).simplified().toInt();
        strBinPart = itPartInfo.value();
        iBin = strBinPart.section(" ",0,0).simplified().toInt();

        if(m_bFinalTest)
        {
            iHBin = iBin;
            if(bCreateHbr || (!m_mapHbrList.contains(iHBin)))
            {
                // no info about HBin and SBin
                bPass = (iHBin == 1);
                if(m_mapHbrList.contains(iHBin))
                    pBin = m_mapHbrList[iHBin];
                else
                {
                    // HBin1 for PASS else HBin0
                    pBin = new CTANGOBin(iHBin, -1, bPass);
                    m_mapHbrList[iHBin] = pBin;
                }
                pBin->iCnt++;
            }
            pBin = m_mapHbrList[iHBin];

            // Is part pass?
            if(pBin->iSBin < 0)
                pBin->iSBin = 65535; // Invalid SoftBin
            if(pBin->iPass < 0)
                pBin->iPass = (iHBin ==  1);
            bPass = (pBin->iPass == 1);
            iSBin = pBin->iSBin;
        }
        else
        {
            iSBin = iBin;
            if(bCreateSbr || (!m_mapSbrList.contains(iSBin)))
            {
                // no info about SBin
                bPass = (iSBin == 1);
                if(m_mapSbrList.contains(iSBin))
                    pBin = m_mapSbrList[iSBin];
                else
                {
                    // HBin1 for PASS else HBin0
                    pBin = new CTANGOBin(iSBin, bPass, bPass);
                    m_mapSbrList[iSBin] = pBin;
                }
                pBin->iCnt++;
            }
            pBin = m_mapSbrList[iSBin];

            // Is part pass?
            if(pBin->iHBin == -1)
            {
                if(pBin->iPass == -1)
                    pBin->iPass = (iSBin == 1);
                pBin->iHBin = pBin->iPass;
            }
            if(pBin->iPass == -1)
                pBin->iPass = (pBin->iHBin ==  1);
            bPass = (pBin->iPass == 1);
            iHBin = pBin->iHBin;
        }

        // Part number
        iPartNumber++;


        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);              // Test head
        StdfFile.WriteByte(1);              // Tester site
        StdfFile.WriteRecord();


        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);              // Test head
        StdfFile.WriteByte(1);              // Tester site:1,2,3,4 or 5
        if(bPass)
        {
            StdfFile.WriteByte(0);          // PART_FLG : PASSED
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte(8);          // PART_FLG : FAILED
            iTotalFailBin++;
        }
        StdfFile.WriteWord((WORD)0);        // NUM_TEST
        StdfFile.WriteWord(iHBin);          // HARD_BIN
        StdfFile.WriteWord(iSBin);          // SOFT_BIN
        StdfFile.WriteWord(iWaferX);        // X_COORD
        StdfFile.WriteWord(iWaferY);        // Y_COORD
        StdfFile.WriteDword(0);             // No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());// PART_ID
        StdfFile.WriteString("");           // PART_TXT
        StdfFile.WriteString("");           // PART_FIX
        StdfFile.WriteRecord();
    }

    if(!m_bFinalTest)
    {
        // Write WRR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);              // Test head
        StdfFile.WriteByte(255);            // Tester site (all)
        StdfFile.WriteDword(m_lEndTime);    // Time of last part tested
        StdfFile.WriteDword(iPartNumber);   // Parts tested: always 5
        StdfFile.WriteDword(iPartRetested); // Parts retested
        StdfFile.WriteDword(0);             // Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin); // Good Parts
        StdfFile.WriteDword(4294967295UL);  // Functionnal Parts
        StdfFile.WriteString(m_strWaferID.toLatin1().constData());// WaferID
        StdfFile.WriteRecord();
    }

    // Write SBR Bin
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    QMap<int,CTANGOBin*>::Iterator itBinInfo;
    for(itBinInfo=m_mapSbrList.begin(); itBinInfo!=m_mapSbrList.end(); itBinInfo++)
    {
        if(bCreateHbr)
        {
            // have to create HBin Summary from SBR information
            pBin = itBinInfo.value();
            if(pBin->iHBin == -1)
            {
                if(pBin->iPass == -1)
                    pBin->iPass = (iSBin == 1);
                pBin->iHBin = pBin->iPass;
            }
            if(pBin->iPass == -1)
                pBin->iPass = (pBin->iHBin ==  1);
            if(!m_mapHbrList.contains(pBin->iHBin))
            {
                m_mapHbrList[pBin->iHBin] = new CTANGOBin(pBin->iHBin,-1,pBin->iPass,pBin->strBinName);
            }
            m_mapHbrList[pBin->iHBin]->iCnt += pBin->iCnt;
        }

        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);                    // Test Head = ALL
        StdfFile.WriteByte(255);                    // Test sites = ALL
        StdfFile.WriteWord(itBinInfo.key());        // SBIN = 0
        StdfFile.WriteDword(itBinInfo.value()->iCnt);// Total Bins
        if(itBinInfo.value()->iPass == 1)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itBinInfo.value()->strBinName.toLatin1().constData());// Bin name
        StdfFile.WriteRecord();
    }


    // Write HBR Bin
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;

    for(itBinInfo=m_mapHbrList.begin(); itBinInfo!=m_mapHbrList.end(); itBinInfo++)
    {
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);                    // Test Head = ALL
        StdfFile.WriteByte(255);                    // Test sites = ALL
        StdfFile.WriteWord(itBinInfo.key());        // HBIN = 0
        StdfFile.WriteDword(itBinInfo.value()->iCnt);// Total Bins
        if(itBinInfo.value()->iPass == 1)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itBinInfo.value()->strBinName.toLatin1().constData());// Bin name
        StdfFile.WriteRecord();
    }


    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lEndTime);            // File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' TANGO file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CTANGOtoSTDF::Convert(const char *TangoFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TangoFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;

    iProgressStep = 0;
    iNextFilePos = 0;
    m_iNextParameter = 0;
    m_iTotalParameters = 0;
    m_iParameterCount = 0;


    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TangoFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }

    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(200);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }
    QCoreApplication::processEvents();

    if(ReadTangoFile(TangoFileName) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading TANGO file
    }

    if(WriteStdfFile(strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
                && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
                && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        QFile::remove(strFileNameSTDF);
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
            && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
            && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    // Convertion successful
    return true;
}

