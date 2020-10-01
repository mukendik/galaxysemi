//////////////////////////////////////////////////////////////////////
// import_spektra_datalog.cpp: Convert a .SpektraDatalog file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <QFileInfo>
#include "bin_map_store_factory.h"
#include "bin_map_item_base.h"
#include "import_spektra_datalog.h"
#include "converter_external_file.h"
#include "promis_interpreter_factory.h"
#include "promis_item_base.h"
#include "gqtl_global.h"
#include <gs_types.h>

// File Format
//DTA File Name,A04K041.1_W01
//DTA File Created Date Time, 22 02 2010 14:49:43
//TST File Name for DTA,DRSBA8583_02.TST
//CSV File Name,A04K041.1_W01.Dta
//CSV File Created Date Time,22 02 2010 14:54:56
//Operator Name,12985
//Station,A
//Device Name,605810TS
//Lot Name,A04K041.1
//Comment,_
//Tester#,SPEKTRA 02
//Handler#,EG 04
//Die Qty Per Wafer,233
//Test,,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
//Item,,X-AXIS,Y-AXIS,CONT,ISGS,IDSS,BVDSS,BVDSS,BVDSS,DIVID,DIVID,RDON,VP,SAME,VF,SAME,SAME,SAME,ISGS,ISGS,ISGS,ISGS
//Limit,,9.999k ,9.999k ,0.000  ,10.00uA,1.000uA,60.20 V,60.20 V,60.20 V,1.030  ,1.030  ,14.18mR,2.000 V,3.746  ,827.0mV,500.0m ,76.87  ,4.280m ,100.0uA,100.0uA,90.00nA,90.00nA
//Limit Min Max,,<,<,<,<,<,>,>,>,<,<,<,>,<,<,>,<,>,<,<,<,<
//Bias 1,,,,,VSG 5.00 V,VDS 60.0 V,ID 100 uA,ID 1.00mA,ID 10.0mA,T# 6,T# 7,ID 20.0 A,VDS 5.00 V,T# 12,IAK 7.00 A,T# 14,T# 6,T# 11,VSG 35.0 V,VSG 35.0 V,VSG 20.0 V,VSG 20.0 V
//Bias 2,,,,,IMAX 0.00 A,IMAX 0.00 A,VMAX 999  V,VMAX 999  V,VMAX 999  V,T# 7,T# 8,VG 10.0 V,ID 250 uA,,,,,,IMAX 0.00 A,IMAX 0.00 A,IMAX 0.00 A,IMAX 0.00 A
//Time,,780.0us,780.0us,780.0us,2.500ms,2.500ms,2.500ms,1.000ms,380.0us,0.000 s,0.000 s,500.0us,1.000ms,0.000 s,380.0us,0.000 s,0.000 s,0.000 s,2.500ms,2.500ms,80.00ms,80.00ms
//Wafer Data,W#,1
//Serial,Bin
//1,1,3,-1,PASS,0,10.00n,64.19 ,64.14 ,64.16 ,1.000 ,999.6m,6.150m,2.076 ,2.076 ,737.0m,737.0m,64.19 ,6.150m,15.60u,10.20u,1.530n,1.440n
//2,1,4,-1,PASS,0,0,64.20 ,64.22 ,64.23 ,999.6m,999.8m,5.785m,2.059 ,2.059 ,734.0m,734.0m,64.20 ,5.785m,15.70u,10.20u,1.560n,1.410n

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

const QString cComma = ",";
const QString cDot= ".";
const QString cSpace = " ";
const QString cHashtag = "#";
const QString cSemi = ":";

const QString cInfK = "9.999k";
const QString cUnitRegExp = "[munpfkKMGT]";

const char cMilli = 'm'; // Milli
const char cMicro = 'u'; // Micro
const char cNano = 'n'; // Nano
const char cPico = 'p'; // Pico
const char cFento = 'f'; // Fento
const char cUKilo = 'K'; // Kilo
const char cLKilo = 'k';
const char cMega = 'M'; // Mega
const char cGiga = 'G'; // Giga
const char cTera = 'T'; // Tera

const char cGFormat = 'g';

const QString cDieQty = "Die Qty Per Wafer,";
const QString cWaferData = "Wafer Data,";

const QString cSectionTST = "TST FILE NAME FOR DTA";
const QString cSectionCSV = "CSV FILE CREATED DATE TIME";
const QString cSectionOper = "OPERATOR NAME";
const QString cSectionStation = "STATION";
const QString cSectionDevice = "DEVICE NAME";
const QString cSectionLot = "LOT NAME";
const QString cSectionTester = "TESTER#";
const QString cSectionHandler = "HANDLER#";
const QString cSectionDieDty = "DIE QTY PER WAFER";
const QString cSectionWaferData = "WAFER DATA";

const QString cBias = "Bias ";

const QString cPass = "PASS";

const char cPassChar = 'P';
const char cFailChar = 'F';
const char cSpaceChar = ' ';
const QString cSpektraDatalog = ":SPEKTRA_DATALOG";

const QString cCMDGrossDie = "<cmd> gross_die=";

const char cDownOrientation = 'D';
const char cLeftOrientation = 'L';
const char cUpOrientation = 'U';
const char cRightOrientation = 'R';

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSpektraDatalogtoSTDF::CGSpektraDatalogtoSTDF(): ParserBase(typeSpektraDatalog, "typeSpektraDatalog")
{
    // Default: SpektraDatalog parameter list on disk includes all known SpektraDatalog parameters...
    mStartTime = 0;
    mDataOffset = 2;
    mWaferId = 1;
    mDiesUnit = -1;
    mGoodParts = 0;
    mFailParts = 0;
    mTotalParts = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSpektraDatalogtoSTDF::~CGSpektraDatalogtoSTDF()
{
    mParameterList.clear();
    mSpektraDatalogBinning.clear();
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGSpektraDatalogtoSTDF::NormalizeValue(QString &aValue, QString &aUnit, int &aScaleFactor)
{
    aScaleFactor = 0;
    aUnit = "";

    if(aValue.toLower() == cInfK)
    {
        aValue = "";
        return;
    }

    // Try to extract the unit from the limit string
    QString lTmpValue = aValue;
    bool lIsValueaNumber;
    int lUnitOffset = 0;

    while(lTmpValue.length()>0)
    {
        lTmpValue.toFloat(&lIsValueaNumber);
        if(lIsValueaNumber) break;

        aUnit = lTmpValue.right(1) + aUnit;
        lTmpValue = lTmpValue.left(lTmpValue.length()-1).simplified();
        lUnitOffset++;
    }

    // No Unit or invalid Unit ==> Nothing to do
    if((lUnitOffset == 0) || (lUnitOffset > 2)) return;

    aValue = lTmpValue;

    if(aUnit.length() == 1)
    {
        // Check if have unit or only scale
        QString lScale = aUnit.right(1);
        if(!lScale.contains(QRegExp(cUnitRegExp)))
        {
            // units too short to include a prefix, then keep it 'as-is'
            return;
        }
    }

    QChar cPrefix = aUnit[0];
    switch(cPrefix.toLatin1())
    {
    case cMilli: // Milli
        aScaleFactor = -3;
        break;
    case cMicro: // Micro
        aScaleFactor = -6;
        break;
    case cNano: // Nano
        aScaleFactor = -9;
        break;
    case cPico: // Pico
        aScaleFactor = -12;
        break;
    case cFento: // Fento
        aScaleFactor = -15;
        break;
    case cUKilo: // Kilo
    case cLKilo:
        aScaleFactor = 3;
        break;
    case cMega: // Mega
        aScaleFactor = 6;
        break;
    case cGiga: // Giga
        aScaleFactor = 9;
        break;
    case cTera: // Tera
        aScaleFactor = 12;
        break;
    }
    if(aScaleFactor) aUnit = aUnit.mid(1);	// Take all characters after the prefix.

    // Apply the scale to the limit value
    float lScaledValue = aValue.toFloat() * GS_POW(10.0,aScaleFactor);

    aValue = QString::number(lScaledValue, cGFormat, 10);
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SpektraDatalog format
//////////////////////////////////////////////////////////////////////
bool CGSpektraDatalogtoSTDF::IsCompatible(const QString &aFileName)
{
    QString strString;

    // Open hCsmFile file
    QFile f(aFileName);
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    QTextStream hSpektraDatalogFile(&f);

    // Check if first line is the correct SpektraDatalogheader...
    //DTA File Name,A04K041.1_W01
    do strString = hSpektraDatalogFile.readLine();
    while(!strString.isNull() && strString.isEmpty() && !hSpektraDatalogFile.atEnd());

    if(!strString.simplified().startsWith("DTA File Name",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        return false;
    }

    // This is a Spektra file
    // Check if it is a Datalog file
    //DTA File Name,A04K041.1_W01
    // ...
    //Die Qty Per Wafer,233
    //...
    //Wafer Data,W#,1
    bool bFoundGrossDieLine = false;
    bool bFoundWaferNoLine = false;
    int nLine = 30;
    while(!hSpektraDatalogFile.atEnd())
    {
        nLine--;
        strString = hSpektraDatalogFile.readLine();
        if(nLine <= 0)
            break;
        if(strString.startsWith(cDieQty,Qt::CaseInsensitive))
            bFoundGrossDieLine = true;
        if(strString.startsWith(cWaferData,Qt::CaseInsensitive))
            bFoundWaferNoLine = true;
        if(bFoundGrossDieLine && bFoundWaferNoLine)
            break;
    }

    f.close();

    if(bFoundGrossDieLine && bFoundWaferNoLine) return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PromisSpektraDatalog file
//////////////////////////////////////////////////////////////////////
bool CGSpektraDatalogtoSTDF::ReadPromisSpektraDataFile(const QString& aFilePath)
{
    if(mSubLotId.isEmpty())
    {
        // Failed Opening SpektraDatalog file
        mLastError = errMissingData;
        mLastErrorMessage = "No Promis Lot Id defined";

        // Convertion failed.
        return false;
    }

    // Open CSV file
    QFile f( aFilePath );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SpektraDatalog file
        mLastError = errOpenFail;
        mLastErrorMessage = aFilePath+"\n"+f.errorString();

        // Convertion failed.
        return false;
    }

    try
    {
        QString     lPromisKey = mLotId + cDot + QString::number(mWaferId);     //<Lot_ID.Wafer_ID>

        mPromisInterpreter.reset( Qx::BinMapping::PromisInterpreterFactory< Qx::BinMapping::promis_hvm_wt >
                                  ::MakePromisInterpreter( lPromisKey.toStdString(),
                                                           aFilePath.toStdString(),
                                                           ConverterExternalFile::GetExternalFileName( aFilePath )
                                                           .toStdString() ) );

        mDiesUnit   = 3;
        mFacilId    = mPromisInterpreter->GetPromisItem().GetFabLocation().c_str();
        mProcId     = mPromisInterpreter->GetPromisItem().GetDiePart().c_str();
        mPartType   = mPromisInterpreter->GetPromisItem().GetGeometryName().c_str();
        mGrossDie   = mPromisInterpreter->GetPromisItem().GetGrossDiePerWafer().c_str();
        mDiesXSize  = QString::fromStdString(mPromisInterpreter->GetPromisItem().GetDieWidth()).toFloat();
        mDiesYSize  = QString::fromStdString(mPromisInterpreter->GetPromisItem().GetDieHeight()).toFloat();
        mFlatNotch  = QString::fromStdString(mPromisInterpreter->GetPromisItem().GetFlatOrientation()).toInt();
        mTestCod    = mPromisInterpreter->GetPromisItem().GetSiteLocation().c_str();
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SpektraBinmap file
//////////////////////////////////////////////////////////////////////
bool CGSpektraDatalogtoSTDF::ReadSpektraBinmapFile(const QString& aFilePath)
{
    try
    {
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::hvm_ws_spektra >
                            ::MakeBinMapStore( aFilePath.toStdString(),
                                               mExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SpektraDatalog file
//////////////////////////////////////////////////////////////////////
bool CGSpektraDatalogtoSTDF::ProcessSpektraDatalogFile(
        const QString &aSpektraDatalogFileName,
        const QString &aPromisDataFilePath,
        const QString &aBinmapFilePath,
        const QString &aFileNameSTDF)
{
    QString   strString;
    QString   strSection;
    qint64    lTestPosition = -1;

    // Open SpektraDatalog file
    QFile lSpektraDataLogfile( aSpektraDatalogFileName );
    if(!lSpektraDataLogfile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SpektraDatalog file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream hSpektraDatalogFile(&lSpektraDataLogfile);

    // Check if first line is the correct SpektraSpektraDatalog header...
    //DTA File Name,A04K041.1.Dta
    strString = ReadLine(hSpektraDatalogFile);

    if(!strString.simplified().startsWith("DTA File Name",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }

    // Read SpektraSpektraDatalog information
    //DTA File Name,A04K041.1_W01
    //DTA File Created Date Time, 22 02 2010 14:49:43
    //TST File Name for DTA,DRSBA8583_02.TST
    //CSV File Name,A04K041.1_W01.Dta
    //CSV File Created Date Time,22 02 2010 14:54:56
    //Operator Name,12985
    //Station,A
    //Device Name,605810TS
    //Lot Name,A04K041.1
    //Comment,_
    //Tester#,SPEKTRA 02
    //Handler#,EG 04
    //Die Qty Per Wafer,233

    QDateTime clDateTime;

    while(!hSpektraDatalogFile.atEnd())
    {
        strString = ReadLine(hSpektraDatalogFile);
        strSection = strString.section(cComma,0,0).toUpper();

        if(strSection.isEmpty()) continue;

        if(strSection == cSectionTST)
        {
            //TST File Name for DTA,DRSBA8583_02.TST
            mJobName = strString.section(cComma,1);
        }
        else if(strSection == cSectionCSV)
        {
            //CSV File Created Date Time,22 02 2010 14:54:56
            QString strYear, strMonth, strDay;
            QString strHour,strMin,strSec;
            strString = strString.section(cComma,1).simplified();
            strYear = strString.section(cSpace,2,2);
            strMonth = strString.section(cSpace,1,1);
            strDay = strString.section(cSpace,0,0);

            strString = strString.section(cSpace,3).simplified();
            strHour = strString.section(cSemi,0,0);
            strMin = strString.section(cSemi,1,1);
            strSec = strString.section(cSemi,2,2);

            clDateTime = QDateTime(QDate(strYear.toInt(),strMonth.toInt(),strDay.toInt()),
                                   QTime(strHour.toInt(),strMin.toInt(),strSec.toInt()),
                                   Qt::UTC);

        }
        else if(strSection == cSectionOper)
        {
            //Operator Name,12985
            mOperatorId = strString.section(cComma,1);
        }
        else if(strSection == cSectionStation)
        {
            //Station,A
            mTesterName = strString.section(cComma,1);
        }
        else if(strSection == cSectionDevice)
        {
            //Device Name,605810TS
            mProcId = mSuprName = strString.section(cComma,1);
        }
        else if(strSection == cSectionLot)
        {
            //Lot Name,A04K041.1
            mSubLotId = strString.section(cComma,1);
            mLotId = mSubLotId.section(cDot,0,0);
        }
        else if(strSection == cSectionTester)
        {
            //Tester#,SPEKTRA 02
            mTesterType = strString.section(cComma,1);
        }
        else if(strSection == cSectionHandler)
        {
            //Handler#,EG 04
            mHandlerId = mExtraId = strString.section(cComma,1);
        }
        else if(strSection == cSectionDieDty)
        {
            //Die Qty Per Wafer,233
            mGrossDie = strString.section(cComma,1);

            // Keep the position of the line just after 'Die Qty Per Wafer'
            lTestPosition = hSpektraDatalogFile.pos();
        }
        else if(strSection == cSectionWaferData)
        {
            //Wafer Data,W#,13
            mWaferId = strString.section(cComma,2,2).simplified().toInt();
            break;
        }
    }

    if(strSection != cSectionWaferData || lTestPosition == -1)
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }

    if (ConverterExternalFile::Exists(QFileInfo(aSpektraDatalogFileName).absolutePath()))
    {
        if(!ReadPromisSpektraDataFile(aPromisDataFilePath))
        {
            // Convertion failed.
            // Close file
            lSpektraDataLogfile.close();
            mLastErrorMessage += ". \nSpektra Promis data cannot be retrieved";
            return false;
        }
    }

    if (ConverterExternalFile::Exists(QFileInfo(aSpektraDatalogFileName).absolutePath()))
    {
        if(!ReadSpektraBinmapFile(aBinmapFilePath))
        {
            // Convertion failed.
            // Close file
            lSpektraDataLogfile.close();
            mLastErrorMessage += ". \nSpektra Bin mapping cannot be retrieved";
            return false;
        }
    }

    mStartTime = clDateTime.toTime_t();

    //Save test information
    QString strTestNames, strTestNumbers, strTestLimit, strTestLimitSpec, strTestTime;

    // Go back to the line hust after 'Die Qty Per Wafer,'
    hSpektraDatalogFile.seek(lTestPosition);

    //Test,,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
    strString = ReadLine(hSpektraDatalogFile).simplified();
    if(!strString.startsWith("Test,,"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }
    strTestNumbers = strString;

    //Item,,X-AXIS,Y-AXIS,CONT,ISGS,IDSS,BVDSS,BVDSS,BVDSS,DIVID,DIVID,RDON,VP,SAME,VF,SAME,SAME,SAME,ISGS,ISGS,ISGS,ISGS
    strString = ReadLine(hSpektraDatalogFile).simplified();
    if(!strString.startsWith("Item,,"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }
    strTestNames = strString;

    //Limit,,9.999k ,9.999k ,0.000  ,10.00uA,1.000uA,60.20 V,60.20 V,60.20 V,1.030  ,1.030  ,14.18mR,2.000 V,3.746  ,827.0mV,500.0m ,76.87  ,4.280m ,100.0uA,100.0uA,90.00nA,90.00nA
    strString = ReadLine(hSpektraDatalogFile).simplified();
    if(!strString.startsWith("Limit,,"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }
    strTestLimit = strString;

    //Limit Min Max,,<,<,<,<,<,>,>,>,<,<,<,>,<,<,>,<,>,<,<,<,<
    strString = ReadLine(hSpektraDatalogFile).simplified();
    if(!strString.startsWith("Limit Min Max,,"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }
    strTestLimitSpec = strString;

    //Bias 1,,,,,VSG 5.00 V,VDS 60.0 V,ID 100 uA,ID 1.00mA,ID 10.0mA,T# 6,T# 7,ID 20.0 A,VDS 5.00 V,T# 12,IAK 7.00 A,T# 14,T# 6,T# 11,VSG 35.0 V,VSG 35.0 V,VSG 20.0 V,VSG 20.0 V
    //Bias 2,,,,,IMAX 0.00 A,IMAX 0.00 A,VMAX 999  V,VMAX 999  V,VMAX 999  V,T# 7,T# 8,VG 10.0 V,ID 250 uA,,,,,,IMAX 0.00 A,IMAX 0.00 A,IMAX 0.00 A,IMAX 0.00 A
    strString = ReadLine(hSpektraDatalogFile).simplified();
    while(strString.startsWith(cBias,Qt::CaseInsensitive))
    {
        strString = ReadLine(hSpektraDatalogFile).simplified();
    }

    //Time,,780.0us,780.0us,780.0us,2.500ms,2.500ms,2.500ms,1.000ms,380.0us,0.000 s,0.000 s,500.0us,1.000ms,0.000 s,380.0us,0.000 s,0.000 s,0.000 s,2.500ms,2.500ms,80.00ms,80.00ms
    if(!strString.startsWith("Time,,"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }
    strTestTime = strString;

    // Allocate the buffer to hold the N parameters & results.
    mTotalParameters = strTestNumbers.count(cComma);
    mParameterList.resize(mTotalParameters);	// List of parameters
    QString lTmpUnit, lTmpValue;
    int lTmpScale;
    for(int lIndex=mDataOffset;lIndex<mTotalParameters;lIndex++)
    {
        ParserParameter& lParam = mParameterList[lIndex];
        lParam.SetTestNumber(strTestNumbers.section(cComma,lIndex,lIndex).simplified().toInt());
        lParam.SetTestName(strTestNames.section(cComma,lIndex,lIndex).simplified());

        lTmpUnit = "";
        // extract limit
        lTmpValue = strTestLimit.section(cComma,lIndex,lIndex).simplified();
        NormalizeValue(lTmpValue, lTmpUnit, lTmpScale);
        lParam.SetTestUnit(lTmpUnit);
        lParam.SetResultScale(lTmpScale);

        // check if low or high
        bool lIsValidLimit = false;
        if(strTestLimitSpec.section(cComma,lIndex,lIndex).simplified() == ">")
        {
            lParam.SetLowLimit(lTmpValue.toFloat(&lIsValidLimit));
            lParam.SetValidLowLimit(lIsValidLimit);
        }
        else
        {
            lParam.SetHighLimit(lTmpValue.toFloat(&lIsValidLimit));
            lParam.SetValidHighLimit(lIsValidLimit);
        }

        // extract test time
        lTmpValue = strTestTime.section(cComma,lIndex,lIndex).simplified();
        NormalizeValue(lTmpValue,lTmpUnit,lTmpScale);
        lParam.SetExecTime(lTmpValue.toFloat());
    }

    //Wafer Data,W#,13
    strString = ReadLine(hSpektraDatalogFile).simplified();
    if(!strString.startsWith("Wafer Data,W#,"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }

    //Serial,Bin
    strString = ReadLine(hSpektraDatalogFile).simplified();
    if(!strString.startsWith("Serial,Bin"))
    {
        // Incorrect header...this is not a SpektraSpektraDatalog file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }

    if(!WriteStdfFile(hSpektraDatalogFile,aFileNameSTDF))
    {
        QFile::remove(aFileNameSTDF);
        // Close file
        lSpektraDataLogfile.close();
        return false;
    }

    // Close file
    lSpektraDataLogfile.close();

    // Success parsing SpektraDatalog file
    return true;
}

bool CGSpektraDatalogtoSTDF::GetInputFiles(
        const QString& aSpektraDatalogFileName,
        QString &aPromisFilePath,
        QString &aBinMapFilePath)
{
    aPromisFilePath = aBinMapFilePath = "";

    // Check if converter_external_file exists
    if (!ConverterExternalFile::Exists(aSpektraDatalogFileName))
        return true;

    // Get Promis File
    QString lExternalFileName, lExternalFileFormat, lExternalFileError;
    ConverterExternalFile::GetPromisFile(
                aSpektraDatalogFileName,
                "wafer",
                "prod",
                lExternalFileName,
                lExternalFileFormat,
                lExternalFileError);

    mPromisFilePath = lExternalFileName;

    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'GEX_PROMIS_SPEKTRA_DATA' file defined";
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    if (!QFile::exists(lExternalFileName))
    {
        mLastError = errReadPromisFile;
        mLastErrorMessage = appendPromisExceptionInfo();
        return false;
    }
    aPromisFilePath = lExternalFileName;

    // Get Bin Map File
    lExternalFileName = lExternalFileFormat = lExternalFileError = "";
    ConverterExternalFile::GetBinmapFile(
                aSpektraDatalogFileName,
                "wafer",
                "prod",
                lExternalFileName,
                lExternalFileFormat,
                lExternalFileError);

    mBinMapFilePath = lExternalFileName;
    if(lExternalFileName.isEmpty())
    {
        if(lExternalFileError.isEmpty())
            lExternalFileError = "No 'GEX_SPEKTRA_BINMAP_FILE' file defined";
        mLastError = errMissingData;
        mLastErrorMessage = lExternalFileError;
        return false;
    }
    if (!QFile::exists(lExternalFileName))
    {
        mLastError = errReadBinMapFile;
        mLastErrorMessage = appendBinMappingExceptionInfo();
        return false;
    }
    aBinMapFilePath = lExternalFileName;

    return true;
}

bool CGSpektraDatalogtoSTDF::IsInTheBinMapStore(int aNumber)
{
    if(mBinMapStore.isNull()) return false;

    try
    {
        mBinMapStore->GetBinMapItemByTestNumber(aNumber);
        return true;
    }
    catch(const Qx::BinMapping::BinMappingExceptionForUserBase &)
    {
        return false;
    }
}
//////////////////////////////////////////////////////////////////////
// Create STDF file from SpektraDatalog data parsed
//////////////////////////////////////////////////////////////////////
bool CGSpektraDatalogtoSTDF::WriteStdfFile(QTextStream &aSpektraDatalogFile, const QString &aFileNameSTDF)
{
    if(mStdfParse.Open(aFileNameSTDF.toLatin1().constData(), STDF_WRITE) == false)
    {
        // Failed importing SpektraDatalog file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Write FAR
    //WriteFARRecord();

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    WriteMIRRecord();

    // Write SDR
    WriteSDRRecord();

    // Write Test results for each line read.
    QString		lString, lStringValue;
    int			lGoodParts, lTotalGoodBin, lTotalFailBin,
                lTotalTests, lPartNumber,
                lIndex,
                lXCoordWafer, lYCoordWafer,
                lBinNumber,
                lScaleFactor,
                lIndexHeader = 0;
    bool		lPassStatus, lIsTestPass, lStopOnFail, lIsNumber;
    float		lFloatValue;


    // Reset counters
    lGoodParts=lTotalGoodBin=lTotalFailBin=0;
    lPartNumber=0;
    lXCoordWafer = lYCoordWafer = -32768;

    // Write WIR
    WriteWIRRecord();

    // Write DTR
    WriteDTRRecord();

    // Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR
    // Read SpektraDatalog result
    QStringList lstSections;
    while(!aSpektraDatalogFile.atEnd())
    {
        lString = ReadLine(aSpektraDatalogFile);

        lstSections = lString.split(cComma,QString::KeepEmptyParts);
        if(lstSections.size() <= 3) continue;

        lIndexHeader = 0;
        lPartNumber = lstSections[lIndexHeader++].toInt(&lPassStatus);
        lBinNumber = lstSections[lIndexHeader++].toInt();
        lXCoordWafer = lstSections[lIndexHeader++].toInt();
        lYCoordWafer = lstSections[lIndexHeader++].toInt();


        //-- new bin encountered
        if(mSpektraDatalogBinning.find(lBinNumber) == mSpektraDatalogBinning.end())
        {
            ParserBinning lBinItem;
            if(IsInTheBinMapStore(lBinNumber))
            {
                const Qx::BinMapping::BinMapItemBase& lItem = mBinMapStore->GetBinMapItemByTestNumber(lBinNumber);

                lBinItem.SetBinNumber(lItem.GetBinNumber());
                lBinItem.SetBinName(lItem.GetBinName().c_str());
            }
            else
            {
                lBinItem.SetBinNumber(lBinNumber);
            }

            bool lIsPass = (lBinNumber == 1);
            lBinItem.SetPassFail(lIsPass);

            if(lIsPass && lBinItem.GetBinName().isEmpty())
            {
                lBinItem.SetBinName(cPass);
            }


            lBinItem.SetBinCount(1);
            mSpektraDatalogBinning.insert(lBinNumber, lBinItem);
        }
        else
        {
            mSpektraDatalogBinning[lBinNumber].SetBinCount(mSpektraDatalogBinning[lBinNumber].GetBinCount() + 1);
        }


        // Write PIR
        WritePIRRecord();

        // Reset Pass/Fail flag.
        lPassStatus = mSpektraDatalogBinning[lBinNumber].GetPassFail();

        // Reset counters
        lTotalTests = 0;
        lIsTestPass = true;
        lStopOnFail = false;

        for(lIndex=mDataOffset;lIndex<lstSections.size();lIndex++)
        {
            if(lIndex >= mTotalParameters) break;

            ParserParameter& lParam = mParameterList[lIndex];

            // value format = 5.800m
            // Have to extract the scale
            lFloatValue = 0.0;
            lStringValue = lstSections[lIndex];

            // no execution for this parameter
            if(lStringValue.isEmpty()) continue;

            if(lParam.IsParametricTest())
            {
                NormalizeValue(lStringValue,lString,lScaleFactor);
                lFloatValue = lStringValue.toFloat();
            }

            // For the first execution, check if the values is valid
            // Then considere this test as Parametric for all other execution
            if(!lParam.GetStaticHeaderWritten()
                    && !lParam.IsParametricTest())
            {
                NormalizeValue(lStringValue,lString,lScaleFactor);
                lFloatValue = lStringValue.toFloat(&lIsNumber);
                lParam.SetIsParamatricTest(lIsNumber);
            }

            // Store all values
            lStopOnFail = false;

            lTotalTests++;
            lIsTestPass = true;
            if(lParam.IsParametricTest())
            {
                if(lParam.GetValidLowLimit())
                {
                    lIsTestPass &= (lParam.GetLowLimit() < lFloatValue);
                }
                if(lParam.GetValidHighLimit())
                {
                    lIsTestPass &= (lParam.GetHighLimit() > lFloatValue);
                }
            }

            lParam.SetExecCount(lParam.GetExecCount() + 1);
            if(!lIsTestPass) lParam.SetExecFail(lParam.GetExecFail() + 1);

            if(lParam.IsParametricTest())
            {
                // Write PTR
                WritePTRRecord(lParam, lIsTestPass, lStopOnFail, lFloatValue);
            }
            else
            {
                lParam.SetStaticHeaderWritten(true);
                // Write FTR
                WriteFTRRecord(lParam, lIsTestPass, lStringValue);
            }

            lStopOnFail |= !lIsTestPass;
        }

        // Write PRR
        WritePRRRecord(lPassStatus, lTotalTests, lBinNumber, lXCoordWafer, lYCoordWafer, lPartNumber, lTotalGoodBin, lGoodParts, lTotalFailBin);
    }

    // Write WRR
    WriteWRRRecord(lTotalGoodBin, lTotalFailBin);

    // Write WCR
    WriteWCRRecord();

    // Write TSR
    for(int iIndex=mDataOffset; iIndex<mTotalParameters; iIndex++)
    {
        WriteTSRRecord(mParameterList[iIndex]);
    }

    // Write HBR
    QMap<int,ParserBinning>::Iterator it;
    for(it = mSpektraDatalogBinning.begin(); it != mSpektraDatalogBinning.end(); it++)
    {
        WriteHBRRecord(mSpektraDatalogBinning[it.key()]);
    }

    // Write SBR
    for(it = mSpektraDatalogBinning.begin(); it != mSpektraDatalogBinning.end(); it++)
    {
        WriteSBRRecord(mSpektraDatalogBinning[it.key()]);
    }

    if(mTotalParts == 0)
    {
        mTotalParts = lTotalGoodBin + lTotalFailBin;
        mGoodParts = lTotalGoodBin;
        mFailParts = lTotalFailBin;
    }

    // Write PCR
    WritePCRRecord();

    // Write MRR
    WriteMRRRecord();

    // Close STDF file.
    mStdfParse.Close();

    // Success
    return true;
}

void CGSpektraDatalogtoSTDF::WriteFARRecord()
{
    mFARrecord.Reset();
    mFARrecord.SetCPU_TYPE(1);          // Force CPU type to current computer platform.
    mFARrecord.SetSTDF_VER(4);          // STDF V4
    mStdfParse.WriteRecord(&mFARrecord);
}

void CGSpektraDatalogtoSTDF::WriteMIRRecord()
{
    mMIRRecord.Reset();
    mMIRRecord.SetSETUP_T(mStartTime);			// Setup time
    mMIRRecord.SetSTART_T(mStartTime);			// Start time
    mMIRRecord.SetSTAT_NUM(1);						// Station
    mMIRRecord.SetMODE_COD((BYTE) cPassChar);				// Test Mode = PRODUCTION
    mMIRRecord.SetRTST_COD((BYTE) cSpaceChar);				// rtst_cod
    mMIRRecord.SetPROT_COD((BYTE) cSpaceChar);				// prot_cod
    mMIRRecord.SetBURN_TIM(65535);					// burn_tim
    mMIRRecord.SetCMOD_COD((BYTE) cSpaceChar);				// cmod_cod
    mMIRRecord.SetLOT_ID(mLotId);               // Lot ID
    mMIRRecord.SetPART_TYP(mPartType);          // Part Type / Product ID
    mMIRRecord.SetNODE_NAM(mTesterName);        // Node name
    mMIRRecord.SetTSTR_TYP(mTesterType);        // Tester Type
    mMIRRecord.SetJOB_NAM(mJobName);            // Job name
    mMIRRecord.SetSBLOT_ID(mSubLotId);          // sublot-id
    mMIRRecord.SetOPER_NAM(mOperatorId);        // operator
    mMIRRecord.SetTEST_COD(mTestCod);           // test-cod
    // Construct custom Galaxy USER_TXT
    QString	lUserTxt;
    lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += cSemi;
    lUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    lUserTxt += cSpektraDatalog;
    mMIRRecord.SetUSER_TXT(lUserTxt);           // user-txt
    mMIRRecord.SetFAMLY_ID(mProductId);     // familyID
    mMIRRecord.SetFACIL_ID(mFacilId);		// Facility-ID
    mMIRRecord.SetPROC_ID(mProcId);		// ProcessID
    mMIRRecord.SetSUPR_NAM(mSuprName);      // SUPR_NAM
    mStdfParse.WriteRecord(&mMIRRecord);
}

void CGSpektraDatalogtoSTDF::WriteSDRRecord()
{
    mSDRRecord.Reset();
    mSDRRecord.SetHEAD_NUM(255);				// Test Head = ALL
    mSDRRecord.SetSITE_GRP((BYTE)1);			// Group#
    mSDRRecord.SetSITE_CNT((BYTE)1);            // site_count
    mSDRRecord.SetSITE_NUM(0, 1);               // array of test site#
    mSDRRecord.SetHAND_ID(mHandlerId);		// HAND_ID: Handler/prober name
    mSDRRecord.SetEXTR_ID(mExtraId);		// EXTR_ID:Extra equipment ID
    mStdfParse.WriteRecord(&mSDRRecord);
}

void CGSpektraDatalogtoSTDF::WriteWIRRecord()
{
    mWIRRecord.Reset();
    mWIRRecord.SetHEAD_NUM(1);						// Test head
    mWIRRecord.SetSITE_GRP(255);					// Tester site (all)
    mWIRRecord.SetSTART_T(mStartTime);			// Start time
    mWIRRecord.SetWAFER_ID(QString::number(mWaferId));	// WaferID
    mStdfParse.WriteRecord(&mWIRRecord);
}

void CGSpektraDatalogtoSTDF::WriteDTRRecord()
{
    mDTRRecord.Reset();
    mDTRRecord.SetTEXT_DAT(cCMDGrossDie+mGrossDie);
    mStdfParse.WriteRecord(&mDTRRecord);
}

void CGSpektraDatalogtoSTDF::WritePIRRecord()
{
    mPIRRecord.Reset();
    // Write PIR for parts in this Wafer site
    mPIRRecord.SetHEAD_NUM(1);              // Test head
    mPIRRecord.SetSITE_NUM(1);				// Tester site
    mStdfParse.WriteRecord(&mPIRRecord);
}

void CGSpektraDatalogtoSTDF::WritePTRRecord(
        ParserParameter &aParam,
        bool aIsTestPass,
        bool aIsStopOnFail,
        float aValue)
{
    mPTRRecord.Reset();
    mPTRRecord.SetTEST_NUM(aParam.GetTestNumber());			// Test Number
    mPTRRecord.SetHEAD_NUM(1);					// Test head
    mPTRRecord.SetSITE_NUM(1);					// Tester site:1,2,3,4 or 5, etc.

    BYTE lData;

    if(aIsStopOnFail)
    {
        // Test not executed
        lData = BIT1|BIT4;
    }
    else
    {
        if(aIsTestPass) lData = 0;	// Test passed
        else lData = BIT7;	// Test Failed
    }

    mPTRRecord.SetTEST_FLG(lData);							// TEST_FLG
    lData = BIT6|BIT7;
    mPTRRecord.SetPARM_FLG(lData);							// PARAM_FLG
    mPTRRecord.SetRESULT(aValue);		// Test result
    if(!aParam.GetStaticHeaderWritten())
    {
        aParam.SetStaticHeaderWritten(true);

        // save Parameter name
        mPTRRecord.SetTEST_TXT(aParam.GetTestName());	// TEST_TXT

        lData = 2;	// Valid data.
        if(!aParam.GetValidLowLimit())
            lData |= BIT6;
        if(!aParam.GetValidHighLimit())
            lData |= BIT7;
        mPTRRecord.SetOPT_FLAG(lData);							// OPT_FLAG

        mPTRRecord.SetRES_SCAL(-aParam.GetResultScale());	// RES_SCALE
        mPTRRecord.SetLLM_SCAL(-aParam.GetResultScale());	// LLM_SCALE
        mPTRRecord.SetHLM_SCAL(-aParam.GetResultScale());	// HLM_SCALE
        mPTRRecord.SetLO_LIMIT(aParam.GetLowLimit());	// LOW Limit
        mPTRRecord.SetHI_LIMIT(aParam.GetHighLimit());// HIGH Limit
        mPTRRecord.SetUNITS(aParam.GetTestUnits());		// Units
    }
    mStdfParse.WriteRecord(&mPTRRecord);
}

void CGSpektraDatalogtoSTDF::WriteFTRRecord(
        const ParserParameter &aParam,
        bool aIsTestPass,
        const QString& aValue)
{
    mFTRRecord.Reset();
    mFTRRecord.SetTEST_NUM(aParam.GetTestNumber());      // Test Number
    mFTRRecord.SetHEAD_NUM(1);							// Test head
    mFTRRecord.SetSITE_NUM(1);							// Tester site:1,2,3,4 or 5, etc.

    BYTE lData;
    if(aIsTestPass)
        lData = 0;		// Test passed
    else
        lData = BIT7;	// Test Failed
    mFTRRecord.SetTEST_FLG(lData);                  // TEST_FLG
    mFTRRecord.SetVECT_NAM(aValue);                 // vect_name
    mFTRRecord.SetTEST_TXT(aParam.GetTestName());	// test_txt: test name
    mFTRRecord.SetRSLT_TXT(aValue);                 // rslt_txt
    mStdfParse.WriteRecord(&mFTRRecord);
}

void CGSpektraDatalogtoSTDF::WritePRRRecord(
        bool aIsPassStatus,
        int aTotalTests,
        int aBinNumber,
        int aXWafer,
        int aYWafer,
        int aPartNumber,
        int &aTotalGoodBin,
        int &aGoodParts,
        int &aTotalFailBin)
{
    mPRRRecord.Reset();
    mPRRRecord.SetHEAD_NUM(1);					// Test head
    mPRRRecord.SetSITE_NUM(1);					// Tester site:1,2,3,4 or 5
    if(aIsPassStatus == true)
    {
        mPRRRecord.SetPART_FLG(0);				// PART_FLG : PASSED
        aTotalGoodBin++;
        aGoodParts++;
    }
    else
    {
        mPRRRecord.SetPART_FLG(8);				// PART_FLG : FAILED
        aTotalFailBin++;
    }
    mPRRRecord.SetNUM_TEST((WORD)aTotalTests);	// NUM_TEST
    mPRRRecord.SetHARD_BIN(aBinNumber);         // HARD_BIN
    mPRRRecord.SetSOFT_BIN(aBinNumber);         // SOFT_BIN
    mPRRRecord.SetX_COORD(aXWafer);			// X_COORD
    mPRRRecord.SetY_COORD(aYWafer);			// Y_COORD
    mPRRRecord.SetPART_ID(QString::number(aPartNumber));// PART_ID
    mStdfParse.WriteRecord(&mPRRRecord);
}

void CGSpektraDatalogtoSTDF::WriteWRRRecord(int aTotalGoodBin, int aTotalFailbin)
{
    mWRRRecord.Reset();
    mWRRRecord.SetHEAD_NUM(1);						// Test head
    mWRRRecord.SetSITE_GRP(255);					// Tester site (all)
    mWRRRecord.SetFINISH_T(mStartTime);			// Time of last part tested
    mWRRRecord.SetPART_CNT(aTotalGoodBin + aTotalFailbin);// Parts tested: always 5
    mWRRRecord.SetGOOD_CNT(aTotalGoodBin);			// Good Parts
    mWRRRecord.SetRTST_CNT(0);                      // Parts retested
    mWRRRecord.SetABRT_CNT(0);                      // Parts aborted
    mWRRRecord.SetFUNC_CNT((DWORD)(-1));			// Functionnal Parts
    mWRRRecord.SetWAFER_ID(QString::number(mWaferId));// WaferID
    mStdfParse.WriteRecord(&mWRRRecord);
}

void CGSpektraDatalogtoSTDF::WriteWCRRecord()
{
    if(mDiesUnit >= 0)
    {
        char lOrientation = cSpaceChar;
        if(mFlatNotch==0) lOrientation = cDownOrientation;
        else if(mFlatNotch==90) lOrientation = cLeftOrientation;
        else if(mFlatNotch==180) lOrientation = cUpOrientation;
        else if(mFlatNotch==270) lOrientation = cRightOrientation;

        mWCRRecord.Reset();
        mWCRRecord.SetWAFR_SIZ(0);						// Wafer size
        mWCRRecord.SetDIE_HT(mDiesXSize);			// Height of die
        mWCRRecord.SetDIE_WID(mDiesYSize);			// Width of die
        mWCRRecord.SetWF_UNITS(mDiesUnit);			// Units are in millimeters
        mWCRRecord.SetWF_FLAT((BYTE) lOrientation);	// Orientation of wafer flat
        mStdfParse.WriteRecord(&mWCRRecord);
    }
}

void CGSpektraDatalogtoSTDF::WriteTSRRecord(const ParserParameter &aParam)
{
    mTSRRecord.Reset();
    mTSRRecord.SetHEAD_NUM(255);						// Test head
    mTSRRecord.SetSITE_NUM(255);						// Tester site (all)
    if(aParam.IsParametricTest())
        mTSRRecord.SetTEST_TYP(cPassChar);
    else
        mTSRRecord.SetTEST_TYP(cFailChar);
    mTSRRecord.SetTEST_NUM(aParam.GetTestNumber());	// Test Number
    mTSRRecord.SetEXEC_CNT(aParam.GetExecCount());		// Number of test executions
    mTSRRecord.SetFAIL_CNT(aParam.GetExecFail());		// Number of test failures
    mTSRRecord.SetALRM_CNT(4294967295UL);								// Number of alarmed tests
    mTSRRecord.SetTEST_NAM(aParam.GetTestName());// TEST_NAM
    BYTE lOptFlag = BIT0|BIT1|BIT3|BIT4|BIT5|BIT6|BIT7;
    mTSRRecord.SetOPT_FLAG(lOptFlag);		// OPT_FLG
    mTSRRecord.SetTEST_TIM(aParam.GetExecTime());	// Test Execution time
    mStdfParse.WriteRecord(&mTSRRecord);
}

void CGSpektraDatalogtoSTDF::WriteHBRRecord(const ParserBinning &aBin)
{
    mHBRRecord.Reset();
    mHBRRecord.SetHEAD_NUM(255);						// Test Head = ALL
    mHBRRecord.SetSITE_NUM(255);						// Test sites = ALL
    mHBRRecord.SetHBIN_NUM(aBin.GetBinNumber());			// HBIN
    mHBRRecord.SetHBIN_CNT(aBin.GetBinCount());			// Total Bins
    if(aBin.GetPassFail())
        mHBRRecord.SetHBIN_PF(cPassChar);
    else
        mHBRRecord.SetHBIN_PF(cFailChar);
    mHBRRecord.SetHBIN_NAM(aBin.GetBinName());
    mStdfParse.WriteRecord(&mHBRRecord);
}

void CGSpektraDatalogtoSTDF::WriteSBRRecord(const ParserBinning &aBin)
{
    mSBRRecord.Reset();
    mSBRRecord.SetHEAD_NUM(255);						// Test Head = ALL
    mSBRRecord.SetSITE_NUM(255);						// Test sites = ALL
    mSBRRecord.SetSBIN_NUM(aBin.GetBinNumber());			// HBIN
    mSBRRecord.SetSBIN_CNT(aBin.GetBinCount());			// Total Bins
    if(aBin.GetPassFail())
        mSBRRecord.SetSBIN_PF(cPassChar);
    else
        mSBRRecord.SetSBIN_PF(cFailChar);
    mSBRRecord.SetSBIN_NAM(aBin.GetBinName());
    mStdfParse.WriteRecord(&mSBRRecord);
}

void CGSpektraDatalogtoSTDF::WritePCRRecord()
{
    mPCRRecord.Reset();
    mPCRRecord.SetHEAD_NUM(255);					// Test Head = ALL
    mPCRRecord.SetSITE_NUM(255);					// Test sites = ALL
    mPCRRecord.SetPART_CNT(mTotalParts);			// Total Parts tested
    mPCRRecord.SetRTST_CNT(0);                      // Total Parts restested
    mPCRRecord.SetABRT_CNT(0);                      // Total Parts aborted
    mPCRRecord.SetGOOD_CNT(mGoodParts);			// Total GOOD Parts
    mStdfParse.WriteRecord(&mPCRRecord);
}

void CGSpektraDatalogtoSTDF::WriteMRRRecord()
{
    mMRRRecord.Reset();
    mMRRRecord.SetFINISH_T(mStartTime);			// File finish-time.
    mStdfParse.WriteRecord(&mMRRRecord);
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SpektraDatalog file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSpektraDatalogtoSTDF::ConvertoStdf(const QString& aSpektraDatalogFileName, QString &aFileNameSTDF)
{
    // No erro (default)
    mLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(aSpektraDatalogFileName);
    QFileInfo fOutput(aFileNameSTDF);

    QFile f( aFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified())) return true;

    QString lPromisDataFilePath;
    QString lBinmapFilePath;

    mExternalFilePath = ConverterExternalFile::GetExternalFileName( QFileInfo(aSpektraDatalogFileName ).absolutePath());
    if (!GetInputFiles(QFileInfo(aSpektraDatalogFileName).absolutePath(), lPromisDataFilePath, lBinmapFilePath))
    {
        return false;
    }

/*       int lOverallFileSize = aSpektraDatalogFileName.size() +
                            QFile(lPromisDataFilePath).size() +
                            QFile(lBinmapFilePath).size();
*/

    bool lIsReadOK = ProcessSpektraDatalogFile(
                                        aSpektraDatalogFileName,
                                        lPromisDataFilePath,
                                        lBinmapFilePath,
                                        aFileNameSTDF);

    return lIsReadOK;
}

} // namespace Parser
} // namespace GS
