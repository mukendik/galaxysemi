//////////////////////////////////////////////////////////////////////
// import_mcube.cpp: Convert a Mcube .csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include "import_mcube.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//
//header
//filespecrev,1.0
//lot,61
//wafer,03
//teststep,FT
//...
///header
//testdef
//iddq,test#,dtg#,head#,site#,part#,temp(C),avdd(V),dvdd(V),|,iddq(uA),iavddq(uA),idvddq(uA),P/F
//os,test#,dtg#,head#,site#,part#,temp(C),avdd(V),dvdd(V),|,test[NC](uA),vpp[AVSS](uA),int(uA),sck(uA),sda(uA),P/F
///testdef
//
//limitsdef
//iddq,test#,|,iddq_iddq_min(uA),iddq_iddq_max(uA),iddq_avdd_min(uA),iddq_avdd_max(uA),iddq_dvdd_min(uA),iddq_dvdd_max(uA)
//os,test#,|,iil(uA),iih(uA)
///limitsdef
//
//limitsdata
//iddq,300,|,0,10,-5,5,-5,5
//os,100,|,-1.5,-0.2
///limitsdata
//
//iddq,300,@,0,3,56697643,25.0,3.6,3.6,|,1.44,1.01,0.43,P
//OS,100,@,0,3,56697643,25.0,0,0,|,-0.53,-0.68,-0.53,-0.53,-0.52,P
//bin,1000,100420124523.78,0,1,236191768,25,2.80,2.80,|,1,1

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGMcubetoSTDF::CGMcubetoSTDF()
{
    m_lStartTime = m_lStopTime = 0;
    m_strTesterType = "E320";
    m_bIsWafer = false;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGMcubetoSTDF::~CGMcubetoSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGMcubetoSTDF::GetLastError()
{
    strLastError = "Import Mcube: ";

    switch(iLastError)
    {
        default:
        case errNoError:
            strLastError += "No Error";
            break;
        case errOpenFail:
            strLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            strLastError += "Invalid file format";
            break;
        case errInvalidSectionFormat:
            strLastError += "Invalid file format: \ninvalid section ";
            if(m_nCurrentLine > 0)
                strLastError += " at line " + QString::number(m_nCurrentLine) + " \n";
            strLastError += strErrorMessage;
            break;
         case errInvalidTestFormat:
             strLastError += "Invalid file format: \ninvalid Test ";
             if(m_nCurrentLine > 0)
                 strLastError += " at line " + QString::number(m_nCurrentLine) + " \n";
             strLastError += strErrorMessage;
             break;
        case errInvalidBinResult:
            strLastError += "Invalid file format: Mcube bin discrepancy \n";
            if(m_nCurrentLine > 0)
                strLastError += " at line " + QString::number(m_nCurrentLine) + " \n";
            strLastError += strErrorMessage;
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
// Check if File is compatible with Mcube format
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hMcubeFile(&f);

    // Check if found the header line
    do
    {
        strString = hMcubeFile.readLine().trimmed();
        strString = strString.section(",,",0,0);
        // ignore comments line
        if(!strString.isEmpty() && strString[0] == '#')
            strString = "";
    }
    while(!strString.isNull() && strString.isEmpty());

    //header
    if(strString.toUpper() != "HEADER")
    {
        f.close();
        return false;
    }

    ///header
    int nLine = 0;
    while(!hMcubeFile.atEnd())
    {
        strString = hMcubeFile.readLine().trimmed();
        strString = strString.section(",,",0,0);

        // ignore comments line
        if(strString.isEmpty() || strString[0] == '#')
            continue;

        if(strString.toUpper() == "/HEADER")
        {
            f.close();
            return true;
        }
        nLine++;
        if(nLine > 20)
            break;
    }

    // Incorrect header...this is not a Mcube file!
    f.close();
    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the Mcube file
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::ReadMcubeFile(const char *McubeFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strValue;
    bool	bStatus;

    // Open CSV file
    QFile f( McubeFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Mcube file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;
    m_nCurrentLine = 0;

    // Assign file I/O stream
    QTextStream hMcubeFile(&f);
    QString strDateTime;

    // File name convention
    //LotID_Step_Tester_DTG_Wafer#.log
    strString = McubeFileName;
    if(strString.count("_") >= 4)
    {
        // Remove the file ext
        QFileInfo clFile(McubeFileName);
        strString = clFile.fileName();
        strString = strString.section(".",0,strString.count(".")-1);

        // Extract information from file name
        m_strLotID = strString.section("_",0,0);
        m_strTestCode = strString.section("_",1,1);
        m_strTesterID = strString.section("_",2,2);
        strDateTime = strString.section("_",3,3);

        // Then optional
        bool bIsNumber;
        strValue = strString.section("_",4,4);
        strValue.toInt(&bIsNumber);
        if(bIsNumber)
            m_strWaferID = strValue;

        if(strValue.startsWith("Cycle"))
        {
            m_strExecType = strValue.remove("Cycle");
            m_strTemperature = strString.section("_",5,5).remove("Temp");
            m_strExecVer = strString.section("_",6,6).remove("S");
        }
    }

    m_lStartTime = m_lStopTime = GetDateTimeFromString(strDateTime);

    strString = ReadLine(hMcubeFile).simplified();
    //header
    if(strString.toLower() != "header")
    {
        f.close();
        iLastError = errInvalidFormat;
        return false;
    }

    if(!ReadHeaderInformation(hMcubeFile))
    {
        f.close();
        return false;
    }


    strString = "";
    while(!hMcubeFile.atEnd() && strString.isEmpty())
        strString = ReadLine(hMcubeFile).simplified();

    //binnames
    if(strString.toLower() == "binnames")
    {
        if(!ReadBinNamesInformation(hMcubeFile))
        {
            f.close();
            return false;
        }

        strString = "";
        while(!hMcubeFile.atEnd() && strString.isEmpty())
            strString = ReadLine(hMcubeFile).simplified();
    }

    //testdef
    if(strString.toLower() != "testdef")
    {
        f.close();
        iLastError = errInvalidSectionFormat;
        strErrorMessage = "section Test Definitions (testdef, /testdef) not found ";
        return false;
    }

    if(!ReadTestDefInformation(hMcubeFile))
    {
        f.close();
        return false;
    }


    strString = "";
    while(!hMcubeFile.atEnd() && strString.isEmpty())
        strString = ReadLine(hMcubeFile).simplified();

    //limitsdef
    if(strString.toLower() == "limitsdef")
    {
        if(!ReadLimitsDefInformation(hMcubeFile))
        {
            f.close();
            return false;
        }

        strString = "";
        while(!hMcubeFile.atEnd() && strString.isEmpty())
            strString = ReadLine(hMcubeFile).simplified();

    }

    //limitsdata
    if(strString.toLower() == "limitsdata")
    {
        if(!ReadLimitsDataInformation(hMcubeFile))
        {
            f.close();
            return false;
        }
    }

    strString = "";
    while(!hMcubeFile.atEnd() && strString.isEmpty())
        strString = ReadLine(hMcubeFile).simplified();

    if(hMcubeFile.atEnd())
    {
        f.close();
        iLastError = errInvalidSectionFormat;
        strErrorMessage = "no data found ";
        return false;
    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hMcubeFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Success parsing Mcube file
    f.close();
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// ReadHeaderInformation
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::ReadHeaderInformation(QTextStream &hMcubeFile)
{
    QString strString, strSection;
    QString strValue;
    bool	bIsNumber;
    int		nValue;

    // Read Mcube information
    while(!hMcubeFile.atEnd())
    {
        strString = ReadLine(hMcubeFile);

        strSection = strString.section(",",0,0).trimmed().toLower();
        strString = strString.section(",",1).trimmed();

        if(strSection == "/header")
            break;

        if(strSection == "passsoftbin")
        {
            while(!strString.isEmpty())
            {
                strValue = strString.section(",",0,0).trimmed();
                strString = strString.section(",",1).trimmed();

                nValue = strValue.toInt(&bIsNumber);
                if(bIsNumber)
                {
                    m_mapMcubeSoftBinning[nValue].bPass = true;
                    m_mapMcubeSoftBinning[nValue].nNumber = nValue;
                    m_mapMcubeSoftBinning[nValue].strName = "PASS BIN";
                }
            }

        }
        else if(strSection == "passhardbin")
        {
            while(!strString.isEmpty())
            {
                strValue = strString.section(",",0,0).trimmed();
                strString = strString.section(",",1).trimmed();

                nValue = strValue.toInt(&bIsNumber);
                if(bIsNumber)
                {
                    m_mapMcubeHardBinning[nValue].bPass = true;
                    m_mapMcubeHardBinning[nValue].nNumber = nValue;
                    m_mapMcubeHardBinning[nValue].strName = "PASS BIN";
                }
            }
        }
        else if(strSection == "filespecrev")
        {
            m_strSpecRev = strString;
        }
        else if(strSection == "parserrev")
        {
            m_strParserRev = strString;
        }
        else if(strSection == "lot")
        {
            m_strLotID = strString;
        }
        else if(strSection == "wafer")
        {
            m_strWaferID = strString;
        }
        else if(strSection == "teststep")
        {
            m_strTestCode = strString;
        }
        else if(strSection == "retest")
        {
            m_strRetestCode = strString;
        }
        else if(strSection == "testhouse")
        {
            m_strFacilID = strString;
        }
        else if(strSection == "tester")
        {
            m_strTesterType = strString;
        }
        else if(strSection == "loadboard")
        {
            m_strLoadBoardID = strString;
        }
        else if(strSection == "programrev")
        {
            m_strJobRev = strString;
        }
        else if(strSection == "limitsfile")
        {
            m_strJobName = strString;
        }

        if(strSection == "binnames")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'binnames' found into a Header (header, /header) section";
            return false;
        }

        if(strSection == "testdef")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'testdef' found into a Header (header, /header) section";
            return false;
        }

        if(strSection == "limitsdef")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'limitsdef' found into a Header (header, /header) section";
            return false;
        }

        if(strSection == "limitsdata")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'limitsdata' found into a Header (header, /header) section";
            return false;
        }

        if(strString.count(",") > 4)
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "invalid line found into a Header (header, /header) section";
            return false;
        }

    }

    if(m_mapMcubeSoftBinning.count() == 0)
    {
        iLastError = errInvalidSectionFormat;
        strErrorMessage = "missing 'passsoftbin' keyword into (header, /header) section";
        return false;
    }

    if(m_mapMcubeHardBinning.count() == 0)
    {
        iLastError = errInvalidSectionFormat;
        strErrorMessage = "missing 'passhardbin' keyword into (header, /header) section";
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// ReadTestDefInformation
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::ReadTestDefInformation(QTextStream &hMcubeFile)
{
    QString strString, strSection;

    m_iTestCondIndex = -1;

    // Read Mcube information
    while(!hMcubeFile.atEnd())
    {
        strString = ReadLine(hMcubeFile);

        strSection = strString.section(",",0,0).trimmed().toLower();
        strString = strString.section(",",1).trimmed();

        if(strSection == "/testdef")
            break;

        //iddq,test#,dtg#,head#,site#,part#,temp(C),avdd(V),dvdd(V),|,iddq(uA),iavddq(uA),idvddq(uA),P/F
        m_mapMcubeTestDef[strSection] = strString;

        if(m_iTestCondIndex == -1)
        {
            // Check if have the good part info
            if(!strString.startsWith("test#,dtg#,head#,site#,part#,",Qt::CaseInsensitive))
            {
                iLastError = errInvalidSectionFormat;
                strErrorMessage = "section Test Definitions (testdef, /testdef) - keywords 'test#,dtg#,head#,site#,part#' not present";
                return false;
            }


            m_iTestCondIndex = strString.count("#")+1;
        }

        if(m_iTestCondIndex != strString.count("#")+1)
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "section Test Definitions (testdef, /testdef) - keywords 'test#,dtg#,head#,site#,part#' not present";
            return false;
        }

        if(strSection == "limitsdef")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'limitsdef' found into a  Test Definitions (testdef, /testdef) section";
            return false;
        }

        if(strSection == "limitsdata")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'limitsdata' found into a  Test Definitions (testdef, /testdef) section";
            return false;
        }

    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// ReadBinNamesInformation
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::ReadBinNamesInformation(QTextStream &hMcubeFile)
{
    QString strString, strSection;

    // Read Mcube information
    // softbin1,Good Bin
    // hardbin1,Good Bin
    bool bIsNum;
    int nBinNum;
    while(!hMcubeFile.atEnd())
    {
        strString = ReadLine(hMcubeFile);

        strSection = strString.section(",",0,0).trimmed().toLower();
        strString = strString.section(",",1).trimmed();

        if(strSection == "/binnames")
            break;

        if(strSection.count("softbin"))
        {
            nBinNum = strSection.remove(QRegExp("softbin[0]*")).toInt(&bIsNum);
            if(bIsNum)
            {
                if(!m_mapMcubeSoftBinning.contains(nBinNum))
                {
                    m_mapMcubeSoftBinning[nBinNum].nNumber = nBinNum;
                    m_mapMcubeSoftBinning[nBinNum].bPass = false;
                    m_mapMcubeSoftBinning[nBinNum].nCount = 0;
                }
                m_mapMcubeSoftBinning[nBinNum].strName = strString;
            }
        }
        else if(strSection.count("hardbin"))
        {
            nBinNum = strSection.remove(QRegExp("hardbin[0]*")).toInt(&bIsNum);
            if(bIsNum)
            {
                if(!m_mapMcubeHardBinning.contains(nBinNum))
                {
                    m_mapMcubeHardBinning[nBinNum].nNumber = nBinNum;
                    m_mapMcubeHardBinning[nBinNum].bPass = false;
                    m_mapMcubeHardBinning[nBinNum].nCount = 0;
                }
                m_mapMcubeHardBinning[nBinNum].strName = strString;
            }
        }
        else
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword '"+strSection+"' found into a  Bin Names Definitions (binnames, /binnames) section";
            return false;
        }



    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// ReadLimitsDefInformation
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::ReadLimitsDefInformation(QTextStream &hMcubeFile)
{
    QString strString, strSection;

    // Read Mcube information
    while(!hMcubeFile.atEnd())
    {
        strString = ReadLine(hMcubeFile);

        strSection = strString.section(",",0,0).trimmed().toLower();
        strString = strString.section(",",1).trimmed();

        if(strSection == "/limitsdef")
            break;

        //iddq,test#,|,iddq_iddq_min(uA),iddq_iddq_max(uA),iddq_avdd_min(uA),iddq_avdd_max(uA),iddq_dvdd_min(uA),iddq_dvdd_max(uA)
        m_mapMcubeLimitsDef[strSection] = strString;

        if(strSection == "limitsdata")
        {
            iLastError = errInvalidSectionFormat;
            strErrorMessage = "keyword 'limitsdata' found into a  Limit Definitions (limitsdef, /limitsdef) section";
            return false;
        }

    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// ReadLimitsDataInformation
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::ReadLimitsDataInformation(QTextStream &hMcubeFile)
{
    QString strString, strSection;
    QString strTestName;
    QString	strTestNumber;
    QString strTestDef;

    QString	strLimitName;
    QString	strLimitData;
    QString strLimitDef;
    QString	strLimitValue;
    QString strUnit;
    bool	bIsNumber;
    int		nTestNumber;
    int		nIndex;
    int		nScale;

    bool	bHaveLL;
    float	fLLValue;
    QString	strLLDef;
    bool	bHaveHL;
    float	fHLValue;
    QString strHLDef;

    // Read Mcube information
    while(!hMcubeFile.atEnd())
    {
        strString = ReadLine(hMcubeFile);

        strSection = strString.section(",",0,0).trimmed().toLower();
        strString = strString.section(",",1).trimmed();

        if(strSection == "/limitsdata")
            break;

        //iddq,300,|,0,10,-5,5,-5,5
        // Check if Parameter if present in m_mapMcubeTestDef
        if(m_mapMcubeTestDef.contains(strSection))
        {
            strTestName = strSection;
            strTestDef = m_mapMcubeTestDef[strSection];
            strLimitDef = m_mapMcubeLimitsDef[strSection];
            strTestNumber = strString.section(",",0,0);
            nTestNumber = strTestNumber.toInt(&bIsNumber);
            if(!bIsNumber)
                continue;

            // for each limit def, populate test limits
            strLimitDef = strLimitDef.section("|",1).section(",",1);
            strLimitData = strString.section("|",1).section(",",1);
            strTestDef = strTestDef.section("|",1).section(",",1);

            //iddq(uA),iavddq(uA),idvddq(uA),P/F
            //iddq_iddq_min(uA),iddq_iddq_max(uA),iddq_avdd_min(uA),iddq_avdd_max(uA),iddq_dvdd_min(uA),iddq_dvdd_max(uA)
            //0,10,-5,5,-5,5

            // find the good test def
            for(nIndex=0; nIndex<=strTestDef.count(","); nIndex++)
            {
                bHaveLL = bHaveHL = false;
                strUnit = "";

                strLimitName = strTestDef.section(",",nIndex,nIndex);
                strLLDef = strLimitDef.section(",",nIndex*2,nIndex*2);
                strLimitValue = strLimitData.section(",",nIndex*2,nIndex*2);
                fLLValue = strLimitValue.toFloat(&bIsNumber);
                if(bIsNumber)
                    bHaveLL = true;
                strHLDef = strLimitDef.section(",",nIndex*2+1,nIndex*2+1);
                strLimitValue = strLimitData.section(",",nIndex*2+1,nIndex*2+1);
                fHLValue = strLimitValue.toFloat(&bIsNumber);
                if(bIsNumber)
                    bHaveHL = true;

                // extract the unit
                if(strLimitName.count("(") > 0)
                {
                    strUnit = strLimitName.section("(",1).remove(")");
                    strLimitName = strLimitName.section("(",0,0);
                }

                NormalizeLimits(strUnit,nScale);

                // save this test
                if(!m_mapMcubeParameters.contains(nTestNumber + nIndex))
                {
                    m_mapMcubeParameters[nTestNumber + nIndex].bStaticHeaderWritten = false;
                    m_mapMcubeParameters[nTestNumber + nIndex].bValidHighLimit = false;
                    m_mapMcubeParameters[nTestNumber + nIndex].bValidLowLimit = false;
                    m_mapMcubeParameters[nTestNumber + nIndex].fHighLimit = 0.0;
                    m_mapMcubeParameters[nTestNumber + nIndex].fLowLimit = 0.0;
                    m_mapMcubeParameters[nTestNumber + nIndex].nNumber = nTestNumber + nIndex;
                    m_mapMcubeParameters[nTestNumber + nIndex].nScale = nScale;
                    m_mapMcubeParameters[nTestNumber + nIndex].strTestName = strTestName;
                    m_mapMcubeParameters[nTestNumber + nIndex].strTestCondition = "";
                    m_mapMcubeParameters[nTestNumber + nIndex].strTestPin = strLimitName;
                    m_mapMcubeParameters[nTestNumber + nIndex].strUnits = strUnit;
                }


                if(bHaveLL)
                {
                    // extract the unit
                    if(strLLDef.count("(") > 0)
                    {
                        strUnit = strLLDef.section("(",1).remove(")");
                        NormalizeLimits(strUnit,nScale);
                    }
                    fLLValue *= GS_POW(10.0,nScale);

                    m_mapMcubeParameters[nTestNumber + nIndex].bValidLowLimit = true;
                    m_mapMcubeParameters[nTestNumber + nIndex].fLowLimit = fLLValue;
                }
                if(bHaveHL)
                {
                    // extract the unit
                    if(strHLDef.count("(") > 0)
                    {
                        strUnit = strHLDef.section("(",1).remove(")");
                        NormalizeLimits(strUnit,nScale);
                    }
                    fHLValue *= GS_POW(10.0,nScale);

                    m_mapMcubeParameters[nTestNumber + nIndex].bValidHighLimit = true;
                    m_mapMcubeParameters[nTestNumber + nIndex].fHighLimit = fHLValue;
                }
            }
        }
    }

    return true;
}
//////////////////////////////////////////////////////////////////////
// Create STDF file from Mcube data parsed
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::WriteStdfFile(QTextStream *hMcubeFile, const char *strFileNameSTDF)
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

    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);					// SUN CPU type
    StdfFile.WriteByte(4);					// STDF V4
    StdfFile.WriteRecord();

    //� CP = Chip Probe Populates WT tables. PROD data
    //� CPR = Chip Probe Retest Populates WT tables. PROD data
    //� FT = Final test Populates FT tables. PROD data
    //� Rx = Final test retest Populates FT tables. PROD data
    //� RFT = Rerun Final test Populates FT tables. PROD data
    //� RRx = Rerun final test retest Populates FT tables. PROD data
    //� QA = QA Populates FT tables. CHAR data
    //� EQA = Engineering Q Populates FT tables. CHAR data
    //� EQC = Engineering QC Populates FT tables. CHAR data
    //� TC = Temperature Char Populates FT tables. CHAR data

    m_bIsWafer = false;
    BYTE	bTestMode = 'P';
    BYTE	bRtstCod = ' ';

    if(m_strTestCode.startsWith("CP",Qt::CaseInsensitive))
        m_bIsWafer = true;

    if(m_strTestCode.startsWith("QA",Qt::CaseInsensitive))
        bTestMode = 'Q';

    if(m_strTestCode.startsWith("EQ",Qt::CaseInsensitive))
        bTestMode = 'Q';

    if(m_strTestCode.startsWith("TC",Qt::CaseInsensitive))
        bTestMode = 'Q';

    if(!m_strRetestCode.isEmpty())
        bRtstCod = (BYTE) m_strRetestCode[0].toLatin1();

    if(!m_strCurrentLine.startsWith("/"))
    {
        long lTime = GetDateTimeFromString(m_strCurrentLine.section(",",2,2).remove("."));
        if(lTime > 0)
            m_lStartTime = lTime;
    }

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// Setup time
    StdfFile.WriteDword(m_lStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) bTestMode);		// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) bRtstCod);		// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
    StdfFile.WriteString("");					// Part Type / Product ID
    StdfFile.WriteString(m_strTesterID.toLatin1().constData());		// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());					// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString(m_strExecType.toLatin1().constData());		// exec-type
    StdfFile.WriteString(m_strExecVer.toLatin1().constData());		// exe-ver
    StdfFile.WriteString(m_strTestCode.toLatin1().constData());		// test-cod
    StdfFile.WriteString(m_strTemperature.toLatin1().constData());	// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":MCUBE_"+m_strTesterType.toUpper();
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");		// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// date_cod
    StdfFile.WriteString(m_strFacilID.toLatin1().constData());					// facil_id
    StdfFile.WriteString("");					// floor_id
    StdfFile.WriteString("");					// proc_id
    StdfFile.WriteString("");					// oper_frq
    StdfFile.WriteString("053-0001-017 Mcube Tester ASCII Log File Format");// spec_nam
    StdfFile.WriteString(m_strSpecRev.toLatin1().constData());		// spec_ver
    StdfFile.WriteRecord();

    // Write SDR for last wafer inserted
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 80;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);				// Test Head = ALL
    StdfFile.WriteByte((BYTE)1);			// head#
    StdfFile.WriteByte((BYTE)1);			// Group#
    StdfFile.WriteByte((BYTE)0);			// site_count
    //StdfFile.WriteByte((BYTE)1);			// array of test site#
    StdfFile.WriteString("");				// HAND_TYP: Handler/prober type
    StdfFile.WriteString("");				// HAND_ID: Handler/prober name
    StdfFile.WriteString("");				// CARD_TYP: Probe card type
    StdfFile.WriteString("");				// CARD_ID: Probe card name
    StdfFile.WriteString("");				// LOAD_TYP: Load board type
    StdfFile.WriteString(m_strLoadBoardID.toLatin1().constData());		// LOAD_ID: Load board name
    StdfFile.WriteString("");				// LOAD_TYP:	Load board type
    StdfFile.WriteString("");				// LOAD_ID:	Load board ID
    StdfFile.WriteString("");				// DIB_TYP:	DIB board type
    StdfFile.WriteString("");				// DIB_ID:	DIB board ID
    StdfFile.WriteString("");				// CABL_TYP:	Interface cable type
    StdfFile.WriteString("");				// CABL_ID:	Interface cable ID
    StdfFile.WriteString("");				// CONT_TYP:	Handler contactor type
    StdfFile.WriteString("");				// CONT_ID:	Handler contactor ID
    StdfFile.WriteString("");				// LASR_TYP:	Laser type
    StdfFile.WriteString("");				// LASR_ID:	Laser ID
    StdfFile.WriteString(m_strParserRev.toLatin1().constData());	// EXTR_TYP:	Extra equipment type field
    StdfFile.WriteString("");				// EXTR_ID:	Extra equipment ID
    StdfFile.WriteRecord();
    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR


    if(m_bIsWafer)
    {
        // Write WIR of new Wafer.
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);								// Test head
        StdfFile.WriteByte(255);							// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);					// Start time
        StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();

    }

    // Write Test results for each line read.
    QString		strString;
    float		fValue;				// Used for reading floating point numbers.
    int			nIndex;
    int			nXpos=-32768;
    int			nYpos=-32768;
    int			nSoftBin,nHardBin;
    bool		bPartStatus;
    bool		bPassStatus;
    long		iTotalGoodBin,iTotalFailBin;
  long iTotalTestsExecuted;
  //FIXME: not used ?
  //long iPartNumber;
    long		lMinTestingTime;
    long		lMaxTestingTime;

    bool		bWritePIR;
    BYTE		bData;

    QStringList lstTestDefsParsed;
    QString		strFirstFailTest;
    QString		strTestName;
    QString		strTestCond;
    QString		strTestDef;
    QString		strUnit;
    QString		strValue;
    bool		bIsNumber;
    bool		bTestStatus;
    int			nTestNumber;
    int			nScale;
    int			nCount;

    QMap<int,CGMcubeBinning>::Iterator it;
    QMap<int,int>::Iterator itSite;

    // For retested parts
    QStringList lstPartTested;
    int			iTotalRetests;

    //iddq,test#,dtg#,head#,site#,part#,temp(C),avdd(V),dvdd(V),|,iddq(uA),iavddq(uA),idvddq(uA),P/F
    QString strDtg;
    int nHead=0;
    int	nSite=0;
    int	nPart=0;
    QString strParId;
    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iTotalRetests = 0;
  //FIXME: not used ?
  //iPartNumber=0;
    bWritePIR = true;
    iTotalTestsExecuted = 0;
    bPartStatus = true;
    bPassStatus = true;
    bTestStatus = true;

    lMinTestingTime = lMaxTestingTime = -1;

    //iddq,300,@,0,3,56697643,25.0,3.6,3.6,|,1.44,1.01,0.43,P
    //OS,100,@,0,3,56697643,25.0,0,0,|,-0.53,-0.68,-0.53,-0.53,-0.52,P

    if(!m_strCurrentLine.startsWith("/"))
        strString = m_strCurrentLine;

    while(hMcubeFile->atEnd() == false)
    {
        // Read line
        if(strString.isEmpty())
            strString = ReadLine(*hMcubeFile).simplified();

        // Get testing time
        strDtg = strString.section(",",2,2).remove(".");
        long lTime = GetDateTimeFromString(strDtg);
        if(lTime > 0)
        {
            if(lMinTestingTime<0) lMinTestingTime=lTime;
            lMinTestingTime = qMin(lMinTestingTime, lTime);
            lMaxTestingTime = qMax(lMaxTestingTime, lTime);
        }

        strTestName = strString.section(",",0,0).toLower();

        if(!bWritePIR && ((strTestName == "bin") || (strString.isEmpty())))
        {
            // binning
            strString = strString.section("|",1).section(",",1);

            // empty bin
            if(strString == ",")
                strString = "";

            nSoftBin = strString.section(",",0,0).toInt();
            nHardBin = strString.section(",",1,1).toInt();

            if(strString.isEmpty())
            {
                // error
                iLastError = errInvalidBinResult;
                strErrorMessage = "Missing bin result";
                return false;
            }

            if(!m_mapMcubeHardBinning.contains(nHardBin))
            {
                m_mapMcubeHardBinning[nHardBin].nNumber = nHardBin;
                m_mapMcubeHardBinning[nHardBin].bPass = false;
                m_mapMcubeHardBinning[nHardBin].nCount = 0;
            }
            if(!m_mapMcubeSoftBinning.contains(nSoftBin))
            {
                m_mapMcubeSoftBinning[nSoftBin].nNumber = nSoftBin;
                m_mapMcubeSoftBinning[nSoftBin].bPass = false;
                m_mapMcubeSoftBinning[nSoftBin].nCount = 0;
            }

            if(!m_mapMcubeHardBinning[nHardBin].mapSiteCount.contains(nSite))
                m_mapMcubeHardBinning[nHardBin].mapSiteCount[nSite] = 0;
            if(!m_mapMcubeSoftBinning[nSoftBin].mapSiteCount.contains(nSite))
                m_mapMcubeSoftBinning[nSoftBin].mapSiteCount[nSite] = 0;

            bPassStatus = m_mapMcubeHardBinning[nHardBin].bPass;

            // bPartStatus is updated with bTestPass Flag
            // bPassStatus is the bin Pass/Fail flag
            // if bPartStatus not = bPassStatus then error
            if(bPartStatus != bPassStatus)
            {
                // error
                iLastError = errInvalidBinResult;
                if(bPassStatus)
                    strErrorMessage = strFirstFailTest + " but binned as PASS and Bin " + strString;
                else
                    strErrorMessage = "Data records have all pass results and end in 'P' but binned as FAIL and Bin " + strString;
                return false;
            }

            m_mapMcubeHardBinning[nHardBin].mapSiteCount[nSite]++;
            m_mapMcubeSoftBinning[nSoftBin].mapSiteCount[nSite]++;
            m_mapMcubeHardBinning[nHardBin].nCount++;
            m_mapMcubeSoftBinning[nSoftBin].nCount++;

            if(lMaxTestingTime>0)
                m_lStopTime = qMax(m_lStopTime,lMaxTestingTime);

            long lTestingTime = 0;
            lTestingTime = (lMaxTestingTime - lMinTestingTime)*1000;

            lMinTestingTime = lMaxTestingTime = -1;

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(nHead);			// Test head
            StdfFile.WriteByte(nSite);			// Tester site#:1
            if(bPartStatus == true)
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                iTotalFailBin++;
            }
            StdfFile.WriteWord((WORD)iTotalTestsExecuted);	// NUM_TEST
            StdfFile.WriteWord(nHardBin);			// HARD_BIN
            StdfFile.WriteWord(nSoftBin);			// SOFT_BIN
            StdfFile.WriteWord(nXpos);				// X_COORD
            StdfFile.WriteWord(nYpos);				// Y_COORD
            StdfFile.WriteDword(lTestingTime);						// testing time
            StdfFile.WriteString(strParId.toLatin1().constData());	// PART_ID
            StdfFile.WriteString("");			// PART_TXT
            StdfFile.WriteString("");			// PART_FIX
            StdfFile.WriteRecord();

            if(m_bIsWafer)
            {
                strString = QString::number(nXpos) + "|" + QString::number(nYpos);
                if(lstPartTested.contains(strString))
                    iTotalRetests++;
                else
                    lstPartTested.append(strString);
            }

            bWritePIR = true;
            strString = "";

            // Reset counters
            strFirstFailTest = "";
            iTotalTestsExecuted = 0;
            lstTestDefsParsed.clear();
            bPartStatus = true;
            bPassStatus = true;
            bTestStatus = true;

            // Skip next line
            strString = ReadLine(*hMcubeFile).simplified();
            continue;
        }

        if(!m_mapMcubeTestDef.contains(strTestName))
        {
            // error
            iLastError = errInvalidTestFormat;
            strErrorMessage = strTestName+" unknonwn";
            return false;
        }

        strTestDef = m_mapMcubeTestDef[strTestName].section("|",1).section(",",1);
        if(!lstTestDefsParsed.contains(strTestName))
            lstTestDefsParsed.append(strTestName);
        nTestNumber = strString.section(",",1,1).toInt(&bIsNumber);

        if(!bIsNumber)
        {
            strString = "";
            continue;
        }

        if(!m_mapMcubeParameters.contains(nTestNumber))
        {

            //,iddq(uA),iavddq(uA),idvddq(uA),P/F
            //,iddq_min(uA),iddq_max(uA),avdd_min(uA),avdd_max(uA),dvdd_min(uA),dvdd_max(uA)
            //,0,10,-5,5,-5,5

            // find the good test def
            for(nIndex=0; nIndex<=strTestDef.count(","); nIndex++)
            {
                strValue = strTestDef.section(",",nIndex,nIndex).section("(",0,0);
                strUnit = strTestDef.section(",",nIndex,nIndex).section("(",1).remove(")");
                NormalizeLimits(strUnit,nScale);
                // save this test
                if(!m_mapMcubeParameters.contains(nTestNumber + nIndex))
                {
                    m_mapMcubeParameters[nTestNumber + nIndex].bStaticHeaderWritten = false;
                    m_mapMcubeParameters[nTestNumber + nIndex].bValidHighLimit = false;
                    m_mapMcubeParameters[nTestNumber + nIndex].bValidLowLimit = false;
                    m_mapMcubeParameters[nTestNumber + nIndex].fHighLimit = 0.0;
                    m_mapMcubeParameters[nTestNumber + nIndex].fLowLimit = 0.0;
                    m_mapMcubeParameters[nTestNumber + nIndex].nNumber = nTestNumber + nIndex;
                    m_mapMcubeParameters[nTestNumber + nIndex].nScale = nScale;
                    m_mapMcubeParameters[nTestNumber + nIndex].strTestName = strTestName;
                    m_mapMcubeParameters[nTestNumber + nIndex].strTestCondition = "";
                    m_mapMcubeParameters[nTestNumber + nIndex].strTestPin = strValue;
                    m_mapMcubeParameters[nTestNumber + nIndex].strUnits = strUnit;
                }

            }
        }

        if(bWritePIR)
        {
            nHead = strString.section(",",3,3).toInt();
            nSite = strString.section(",",4,4).toInt();
            strParId = strString.section(",",5,5);
            nPart = strParId.toInt();


            // convert nPart as X,Y
            if(nPart == 0)
            {
                // case 5815
                nXpos = nYpos = -32768;
                strParId = "";

            }
            else
            {
                nXpos = (int) (0xFF & nPart);
                nYpos = (int) ((0xFF00 & nPart) >> 8);
            }

            // Write PIR

            // Write PIR for parts in this Wafer site
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(nHead);					// Test head
            StdfFile.WriteByte(nSite);				// Tester site
            StdfFile.WriteRecord();

            bWritePIR = false;
        }

        strTestCond = strString.section(",",m_iTestCondIndex).section(",|",0,0);
        strTestCond = strTestCond.replace(",","_");

        // Check if have results
        if(strString.count("|") == 0)
        {
            // error
            iLastError = errInvalidTestFormat;
            strErrorMessage = "Test "+strTestName+"["+QString::number(nTestNumber)+"], Data record is incomplete";
            return false;
        }
        // go to result
        strString = strString.section("|",1).section(",",1);

        // Remove last ','
        if(strString.right(1) == ",")
            strString = strString.left(strString.length() - 1);

        // if no data goto the next result
        if(strString.isEmpty())
            continue;

        // check if testdef match with testdata
        // Exact count
        if(strString.count(",") != strTestDef.count(","))
        {
            if(strTestDef.endsWith("P/F",Qt::CaseInsensitive)
            && (strString.count(",") == (strTestDef.count(",")-1)))
            {
                // Missing PassFlag
                // Then try to recover the PassFlag
                nIndex = strTestDef.count(",");
                if(!m_mapMcubeParameters.contains(nTestNumber + nIndex))
                {
                    // error
                    iLastError = errInvalidTestFormat;
                    strErrorMessage = "Test "+strTestName+"["+QString::number(nTestNumber)+"] unknonwn";
                    return false;
                }
                /*
                // bTestStatus is updated according to his test limits
                // if no test limits, use the bPassStatus
                for(nIndex=0; nIndex<=strString.count(","); nIndex++)
                {
                    if(m_mapMcubeParameters[nTestNumber + nIndex].bValidHighLimit
                    && (m_mapMcubeParameters[nTestNumber + nIndex].fHighLimit < strString.section(",",nIndex,nIndex).toFloat()))
                    {
                        bTestStatus = false;
                        break;
                    }

                    if(m_mapMcubeParameters[nTestNumber + nIndex].bValidLowLimit
                    && (m_mapMcubeParameters[nTestNumber + nIndex].fLowLimit > strString.section(",",nIndex,nIndex).toFloat()))
                    {
                        bTestStatus = false;
                        break;
                    }

                }
                */
            }

            // TestDef must match with TestData
            iLastError = errInvalidTestFormat;
            strErrorMessage = "Test "+strTestName+"["+QString::number(nTestNumber)+"] \ntestdef["+strTestDef+"]\n not matches with \ntestdata["+strString+"]";
            return false;
        }

        // Use the PassFlag for test without testLimits
        // If the PassFlag is missing and if the test doesn't have limits, the PassFlag is PASS
        bPassStatus = strString.right(1).toUpper() != "F";

        // for each value
        nCount = strString.count(",");
        for(nIndex=0; nIndex<=nCount; nIndex++)
        {
            // save this test
            if(!m_mapMcubeParameters.contains(nTestNumber + nIndex))
            {
                // error
                iLastError = errInvalidTestFormat;
                strErrorMessage = "Test "+strTestName+"["+QString::number(nTestNumber)+"] unknonwn";
                return false;
            }

            strValue = strString.section(",",nIndex,nIndex);

            iTotalTestsExecuted++;
            if(m_mapMcubeParameters[nTestNumber + nIndex].strTestPin.toLower() == "p/f")
            {
                bTestStatus = strString.right(1).toUpper() != "F";
                if(!bTestStatus && strFirstFailTest.isEmpty())
                    strFirstFailTest = "Functional test " + strTestName + "[" + QString::number(nTestNumber + nIndex) + "] is listed as FAIL";

                // Save as FTR
                if((strValue.toUpper() == "P") || (strValue.toUpper() == "F"))
                {

                    m_mapMcubeParameters[nTestNumber + nIndex].strTestCondition = strTestCond;

                    // Write FTR
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 20;

                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteDword(m_mapMcubeParameters[nTestNumber + nIndex].nNumber);// Test Number
                    StdfFile.WriteByte(nHead);							// Test head
                    StdfFile.WriteByte(nSite);							// Tester site:1,2,3,4 or 5, etc.
                    if(bTestStatus)
                        bData = 0;		// Test passed
                    else
                        bData = BIT7;	// Test Failed
                    StdfFile.WriteByte(bData);							// TEST_FLG

                    // save the name of the test
                    StdfFile.WriteByte(0);				// opt_flg
                    StdfFile.WriteDword(0);				// cycl_cnt
                    StdfFile.WriteDword(0);				// rel_vadr
                    StdfFile.WriteDword(0);				// rept_cnt
                    StdfFile.WriteDword(0);				// num_fail
                    StdfFile.WriteDword(0);				// xfail_ad
                    StdfFile.WriteDword(0);				// yfail_ad
                    StdfFile.WriteWord(0);				// vect_off
                    StdfFile.WriteWord(0);				// rtn_icnt
                    StdfFile.WriteWord(0);
                    StdfFile.WriteWord(0);
                    StdfFile.WriteString("");			// vect_name
                    StdfFile.WriteString("");			// time_set
                    StdfFile.WriteString("");			// op_code

                    // save Parameter name
                    strValue = m_mapMcubeParameters[nTestNumber + nIndex].strTestName;
                    strValue+= "_";
                    if(nIndex > 0)
                    {
                        strValue+= m_mapMcubeParameters[nTestNumber + nIndex].strTestPin;
                        strValue+= "_";
                    }
                    strValue+= m_mapMcubeParameters[nTestNumber + nIndex].strTestCondition;

                    StdfFile.WriteString(strValue.toLatin1().constData());	// test_txt: test name
                    StdfFile.WriteString("");	// alarm_id
                    StdfFile.WriteString("");	// prog_txt
                    StdfFile.WriteString("");	// rslt_txt


                    StdfFile.WriteRecord();
                }
                else
                {
                    // error
                    iLastError = errInvalidTestFormat;
                    strErrorMessage = "Test "+strTestName+"["+QString::number(nTestNumber)+"] P/F with invalid result";
                    return false;
                }

            }
            else
            {
                fValue = strValue.toFloat(&bIsNumber);
                if(bIsNumber)
                {
                    // Save as PTR

                    fValue *= GS_POW(10.0,m_mapMcubeParameters[nTestNumber + nIndex].nScale);

                    // Then try to recover the PassFlag
                    // bTestStatus is updated according to his test limits
                    // if no test limits, then Pass
                    bTestStatus = true;

                    if(m_mapMcubeParameters[nTestNumber + nIndex].bValidHighLimit
                    && (m_mapMcubeParameters[nTestNumber + nIndex].fHighLimit < fValue))
                        bTestStatus = false;

                    if(m_mapMcubeParameters[nTestNumber + nIndex].bValidLowLimit
                    && (m_mapMcubeParameters[nTestNumber + nIndex].fLowLimit > fValue))
                        bTestStatus = false;

                    if(!bTestStatus && strFirstFailTest.isEmpty())
                        strFirstFailTest = "Test " + strTestName + "[" + QString::number(nTestNumber + nIndex) + "]" + " has results outside the limits is listed as FAIL";

                    // Write PTR
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 10;

                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteDword(m_mapMcubeParameters[nTestNumber + nIndex].nNumber);// Test Number
                    StdfFile.WriteByte(nHead);						// Test head
                    StdfFile.WriteByte(nSite);						// Tester site:1,2,3,4 or 5, etc.
                    if(bTestStatus)
                        bData = 0;		// Test passed
                    else
                        bData = BIT7;	// Test Failed
                    StdfFile.WriteByte(bData);							// TEST_FLG
                    bData = BIT6|BIT7;
                    StdfFile.WriteByte(bData);							// PARAM_FLG
                    StdfFile.WriteFloat(fValue);						// Test result
                    if(!m_mapMcubeParameters[nTestNumber + nIndex].bStaticHeaderWritten)
                    {
                        m_mapMcubeParameters[nTestNumber + nIndex].strTestCondition = strTestCond;
                        m_mapMcubeParameters[nTestNumber + nIndex].bStaticHeaderWritten = true;

                        // save Parameter name
                        strValue = m_mapMcubeParameters[nTestNumber + nIndex].strTestName;
                        strValue+= "_";
                        strValue+= m_mapMcubeParameters[nTestNumber + nIndex].strTestPin;
                        strValue+= "_";
                        strValue+= m_mapMcubeParameters[nTestNumber + nIndex].strTestCondition;

                        StdfFile.WriteString(strValue.toLatin1().constData());	// TEST_TXT
                        StdfFile.WriteString("");							// ALARM_ID

                        bData = 2;	// Valid data.
                        if(!m_mapMcubeParameters[nTestNumber + nIndex].bValidLowLimit)
                            bData |= BIT6;
                        if(!m_mapMcubeParameters[nTestNumber + nIndex].bValidHighLimit)
                            bData |= BIT7;
                        StdfFile.WriteByte(bData);							// OPT_FLAG

                        StdfFile.WriteByte(-m_mapMcubeParameters[nTestNumber + nIndex].nScale);	// RES_SCALE
                        StdfFile.WriteByte(-m_mapMcubeParameters[nTestNumber + nIndex].nScale);	// LLM_SCALE
                        StdfFile.WriteByte(-m_mapMcubeParameters[nTestNumber + nIndex].nScale);	// HLM_SCALE
                        StdfFile.WriteFloat(m_mapMcubeParameters[nTestNumber + nIndex].fLowLimit);	// LOW Limit
                        StdfFile.WriteFloat(m_mapMcubeParameters[nTestNumber + nIndex].fHighLimit);// HIGH Limit
                        StdfFile.WriteString(m_mapMcubeParameters[nTestNumber + nIndex].strUnits.toLatin1().constData());		// Units
                    }
                    StdfFile.WriteRecord();
                }
                else if(!strValue.isEmpty())
                {
                    // error
                    iLastError = errInvalidTestFormat;
                    strErrorMessage = "Test "+strTestName+"["+QString::number(nTestNumber)+"] invalid result";
                    return false;
                }

            }

            bPartStatus &= bTestStatus;
        }

        strString = "";
    };			// Read all lines with valid data records in file

    // Write the last PRR if truncated file
    if(!bWritePIR && strString.isEmpty())
    {
        // Check if all TestDefs parsed
        if(lstTestDefsParsed.count() < m_mapMcubeTestDef.count())
            bPartStatus = false;

        if(bPartStatus)
            nHardBin = nSoftBin = 0;
        else
            nHardBin = nSoftBin = 100;

        if(!m_mapMcubeHardBinning.contains(nHardBin))
        {
            m_mapMcubeHardBinning[nHardBin].nNumber = nHardBin;
            m_mapMcubeHardBinning[nHardBin].bPass = bPartStatus;
            m_mapMcubeHardBinning[nHardBin].mapSiteCount[nSite] = 0;
            m_mapMcubeHardBinning[nHardBin].nCount = 0;
        }
        if(!m_mapMcubeSoftBinning.contains(nSoftBin))
        {
            m_mapMcubeSoftBinning[nSoftBin].nNumber = nSoftBin;
            m_mapMcubeSoftBinning[nSoftBin].bPass = bPartStatus;
            m_mapMcubeSoftBinning[nSoftBin].mapSiteCount[nSite] = 0;
            m_mapMcubeSoftBinning[nSoftBin].nCount = 0;
        }

        bPassStatus = m_mapMcubeHardBinning[nHardBin].bPass;

        m_mapMcubeHardBinning[nHardBin].mapSiteCount[nSite]++;
        m_mapMcubeSoftBinning[nSoftBin].mapSiteCount[nSite]++;
        m_mapMcubeHardBinning[nHardBin].nCount++;
        m_mapMcubeSoftBinning[nSoftBin].nCount++;

        if(lMaxTestingTime>0)
            m_lStopTime = qMax(m_lStopTime,lMaxTestingTime);

        long lTestingTime = 0;
        lTestingTime = (lMaxTestingTime - lMinTestingTime)*1000;

        lMinTestingTime = lMaxTestingTime = -1;

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(nHead);			// Test head
        StdfFile.WriteByte(nSite);			// Tester site#:1
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);				// PART_FLG : PASSED
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte(8);				// PART_FLG : FAILED
            iTotalFailBin++;
        }
        StdfFile.WriteWord((WORD)iTotalTestsExecuted);	// NUM_TEST
        StdfFile.WriteWord(nHardBin);			// HARD_BIN
        StdfFile.WriteWord(nSoftBin);			// SOFT_BIN
        StdfFile.WriteWord(nXpos);				// X_COORD
        StdfFile.WriteWord(nYpos);				// Y_COORD
        StdfFile.WriteDword(lTestingTime);						// testing time
        StdfFile.WriteString(strParId.toLatin1().constData());	// PART_ID
        StdfFile.WriteString("");			// PART_TXT
        StdfFile.WriteString("");			// PART_FIX
        StdfFile.WriteRecord();

        if(m_bIsWafer)
        {
            strString = QString::number(nXpos) + "|" + QString::number(nYpos);
            if(lstPartTested.contains(strString))
                iTotalRetests++;
            else
                lstPartTested.append(strString);
        }

        bWritePIR = true;
    }

    if(m_lStopTime <= 0)
        m_lStopTime = m_lStartTime;

    if(m_bIsWafer)
    {
        // Write WRR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test head
        StdfFile.WriteByte(255);					// Tester site (all)
        StdfFile.WriteDword(m_lStopTime);			// Time of last part tested
        StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
        StdfFile.WriteDword(iTotalRetests);			// Parts retested
        StdfFile.WriteDword(0);						// Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
        StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
        StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
        StdfFile.WriteString("");					// FabId
        StdfFile.WriteString("");					// FrameId
        StdfFile.WriteString("");					// MaskId
        StdfFile.WriteString("");	// UserDesc
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    for(it = m_mapMcubeHardBinning.begin(); it != m_mapMcubeHardBinning.end(); it++)
    {
        if(m_mapMcubeHardBinning[it.key()].nCount == 0)
            continue;

        // Write HBR/site
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);				// Test Head
        StdfFile.WriteByte(255);				// Test sites
        StdfFile.WriteWord(m_mapMcubeHardBinning[it.key()].nNumber);		// HBIN
        StdfFile.WriteDword(m_mapMcubeHardBinning[it.key()].nCount);		// Total Bins
        if(m_mapMcubeHardBinning[it.key()].bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(m_mapMcubeHardBinning[it.key()].strName.toLatin1().constData());
        StdfFile.WriteRecord();

        for(itSite = m_mapMcubeHardBinning[it.key()].mapSiteCount.begin();
        itSite != m_mapMcubeHardBinning[it.key()].mapSiteCount.end();
        itSite++)
        {
            if(itSite.value() == 0)
                continue;

            // Write HBR/site
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(nHead);						// Test Head
            StdfFile.WriteByte(itSite.key());				// Test sites
            StdfFile.WriteWord(m_mapMcubeHardBinning[it.key()].nNumber);		// HBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
            if(m_mapMcubeHardBinning[it.key()].bPass)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString(m_mapMcubeHardBinning[it.key()].strName.toLatin1().constData());
            StdfFile.WriteRecord();
        }

    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    for(it = m_mapMcubeSoftBinning.begin(); it != m_mapMcubeSoftBinning.end(); it++)
    {
        if(m_mapMcubeSoftBinning[it.key()].nCount == 0)
            continue;

        // Write SBR/site
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head
        StdfFile.WriteByte(255);						// Test sites
        StdfFile.WriteWord(m_mapMcubeSoftBinning[it.key()].nNumber);		// SBIN
        StdfFile.WriteDword(m_mapMcubeSoftBinning[it.key()].nCount);			// Total Bins
        if(m_mapMcubeSoftBinning[it.key()].bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(m_mapMcubeSoftBinning[it.key()].strName.toLatin1().constData());
        StdfFile.WriteRecord();

        for(itSite = m_mapMcubeSoftBinning[it.key()].mapSiteCount.begin();
        itSite != m_mapMcubeSoftBinning[it.key()].mapSiteCount.end();
        itSite++)
        {
            if(itSite.value() == 0)
                continue;

            // Write SBR/site
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(nHead);						// Test Head
            StdfFile.WriteByte(itSite.key());				// Test sites
            StdfFile.WriteWord(m_mapMcubeSoftBinning[it.key()].nNumber);		// SBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
            if(m_mapMcubeSoftBinning[it.key()].bPass)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString(m_mapMcubeSoftBinning[it.key()].strName.toLatin1().constData());
            StdfFile.WriteRecord();
        }
    }

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iTotalGoodBin + iTotalFailBin);			// Total Parts tested
    StdfFile.WriteDword(iTotalRetests);			// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStopTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' Mcube file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGMcubetoSTDF::Convert(const char *McubeFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(McubeFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(McubeFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(McubeFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    if(ReadMcubeFile(McubeFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading Mcube file
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

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGMcubetoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
{
    nScale = 0;
    strUnit = strUnit.simplified();

    if(strUnit.startsWith("value",Qt::CaseInsensitive))
        strUnit = "";

    if(strUnit.startsWith("count",Qt::CaseInsensitive))
        return;


    if(strUnit.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        return;
    }

    QChar cPrefix = strUnit[0];
    switch(cPrefix.toLatin1())
    {
        case 'm': // Milli
            nScale = -3;
            break;
        case 'u': // Micro
            nScale = -6;
            break;
        case 'n': // Nano
            nScale = -9;
            break;
        case 'p': // Pico
            nScale = -12;
            break;
        case 'f': // Fento
            nScale = -15;
            break;
        case 'K': // Kilo
            nScale = 3;
            break;
        case 'M': // Mega
            nScale = 6;
            break;
        case 'G': // Giga
            nScale = 9;
            break;
        case 'T': // Tera
            nScale = 12;
            break;
    }
    if(nScale)
        strUnit = strUnit.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
//return lDateTime from string strDateTime "100514153912" = 2010 May 14, 15:39:12.
//////////////////////////////////////////////////////////////////////
long CGMcubetoSTDF::GetDateTimeFromString(QString strDateTime)
{
    int		nYear, nMonth, nDay;
    int		nHour, nMin, nSec;
    long	lDateTime;
    QString strDT = strDateTime;

    if(strDT.length()<12)
        return 0;

    if((strDT.length()>=14) && strDT.startsWith("20"))
        strDT = strDT.mid(2);

    nYear = strDT.mid(0,2).toInt() + 2000;
    nMonth = strDT.mid(2,2).toInt();
    nDay = strDT.mid(4,2).toInt();
    nHour = strDT.mid(6,2).toInt();
    nMin= strDT.mid(8,2).toInt();
    nSec = strDT.mid(10,2).toInt();

    QDate clDate(nYear,nMonth,nDay);
    QTime clTime(nHour,nMin,nSec);
    QDateTime clDateTime(clDate,clTime);

    clDateTime.setTimeSpec(Qt::UTC);
    lDateTime = clDateTime.toTime_t();
    return lDateTime;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGMcubetoSTDF::ReadLine(QTextStream& hFile)
{
    QString strString;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) hFile.device()->pos() > iNextFilePos)
        {
            iProgressStep += 100/iFileSize + 1;
            iNextFilePos  += iFileSize/100 + 1;
            GexProgressBar->setValue(iProgressStep);
        }
    }
    QCoreApplication::processEvents();

    while(!hFile.atEnd() && !strString.isNull() && strString.isEmpty())
    {
        strString = hFile.readLine().trimmed();
        while(strString.right(1) == ",")
            strString = strString.left(strString.length()-1);

        // ignore comments line
        if(!strString.isEmpty() && strString[0] == '#')
            strString = "";
        m_nCurrentLine++;
    };

    m_strCurrentLine = strString;

    return strString;

}
