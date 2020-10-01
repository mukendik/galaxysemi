#include <QFile>

#include "importWatTsmXml.h"

//<ADB_DOCUMENT
//    DataSource="ETEST_RAW"
//    FormatVersion="1.1"
//>
//<ETEST_LOT_RUN
//    Lot="T002675.1"
//    SourceLot=""
//    MeasureTime="2010/06/08_13:29:47"
//    Fab="FAB2"
//    Owner="COGNEX"
//>
//<ETEST_LIMITS>
//    <LIM ID="1" Desc="Res_MOPON Scribe_R2p 2x100u Ohm" Unit="" Tgt="" VL="-1000000000000000.000000" VH="1000000000000000.000000" SL="310.000000" SH="490.000000" CL="" CH="" Key="Y" SiteFails="" KeyItem="0.43" Module="" MinValidSites="4.000000" Comments="" />
//    <LIM ID="2" Desc="Res_MOPOP Scribe_R2p 2x100u Ohm" Unit="" Tgt="" VL="-1000000000000000.000000" VH="1000000000000000.000000" SL="260.000000" SH="360.000000" CL="" CH="" Key="Y" SiteFails="" KeyItem="0.43" Module="" MinValidSites="4.000000" Comments="" />
//</ETEST_LIMITS>
//<ETEST_WAFER_RUN
//    WaferNumber="01"
//    SiteCount="5"
//    ParameterCount="8"
//    WaferPass="Y"
//    Comments="F">
//    <SITE SiteID="1" SiteX="9" SiteY="11">
//        <T>1,4.11523e+02</T>
//        <T>2,3.15006e+02</T>
//    </SITE>
//    <SITE SiteID="2" SiteX="9" SiteY="7">
//        <T>1,3.79660e+02</T>
//        <T>2,3.10559e+02</T>
//    </SITE>
//</ETEST_WAFER_RUN>
//</ETEST_LOT_RUN>
//</ADB_DOCUMENT>


#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64
#define BIT7 128


using namespace GS::Parser;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
WatTsmXmlToStdf::WatTsmXmlToStdf():WatToStdf(typeWatTsmXml, "WatTsmXml"),mEndOfFile(false)
{
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
WatTsmXmlToStdf::~WatTsmXmlToStdf()
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WatTsmXml format
//////////////////////////////////////////////////////////////////////
bool WatTsmXmlToStdf::IsCompatible(const QString &szFileName)
{
    QString strSection;
    QString strValue;

    // Open hWatTsmXmlFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Tsm Xml file
        return false;
    }
    // Assign file I/O stream
    QTextStream hWatTsmXmlFile(&f);

    do
        strSection = hWatTsmXmlFile.readLine().simplified();
    while(!strSection.isNull() && strSection.isEmpty());

    strValue = strSection.section("<",1).trimmed();
    if(strValue.startsWith("ADB_DOCUMENT", Qt::CaseInsensitive))
    {
        strSection = hWatTsmXmlFile.readLine();
        strValue = strSection.section("=",1).remove('"').simplified();
        if(strValue.startsWith("ETEST_RAW", Qt::CaseInsensitive))
        {
            f.close();
            return true;
        }
    }

    f.close();
    return false;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the WatTsmXml file
//////////////////////////////////////////////////////////////////////
bool WatTsmXmlToStdf::ParseFile(const QString& watTsmXmlFileName)
{
    QString strSection;

    // Open file
    mFile.setFileName(watTsmXmlFileName);
    if(!mFile.open( QIODevice::ReadOnly ))
    {
        mLastError = errOpenFail;
        return false;
    }

    // Assign file I/O stream
    mWatTsmXmlFile.setDevice(&mFile);

    while(!mEndOfFile)
    {
        strSection = ReadNextTag().toUpper();

        if(strSection.startsWith("/"))
        {
            // Ignore this tag
            continue;
        }
        else if(strSection == "ADB_DOCUMENT")
        {
            continue;
        }
        else if(strSection == "ETEST_LOT_RUN")
        {
            // Save all Attributes
            QMap<QString,QString>::Iterator itAttribute;
            QString strAttribute, strValue;
            for(itAttribute = mTagAttributes.begin(); itAttribute != mTagAttributes.end(); itAttribute++)
            {
                strAttribute = itAttribute.key().toUpper();
                strValue = itAttribute.value();

                if(strAttribute == "LOT")
                    mLotID = strValue;
                else if(strAttribute == "SOURCELOT")
                    mSubLotID = strValue;
                else if(strAttribute == "MEASURETIME")
                {
                    // yyyy/mm/dd_hh24:mi:ss
                    QString strBuffer;
                    int iYear, iMonth, iDay, iHour, iMin, iSec;
                    // Extract the date
                    strBuffer = strValue.section("_",0,0);
                    iYear = strBuffer.section("/",0,0).toInt();
                    iMonth = strBuffer.section("/",1,1).toInt();
                    iDay = strBuffer.section("/",2).toInt();
                    // Extract the time
                    strBuffer = strValue.section("_",1);
                    iHour = strBuffer.section(":",0,0).toInt();
                    iMin = strBuffer.section(":",1,1).toInt();
                    iSec = strBuffer.section(":",2).toInt();

                    // Save this date
                    QDate clDate(iYear,iMonth,iDay);
                    QTime clTime(iHour,iMin,iSec);
                    QDateTime clDateTime(clDate,clTime);
                    clDateTime.setTimeSpec(Qt::UTC);
                    mStartTime = clDateTime.toTime_t();
                }
                else if(strAttribute == "FAB")
                    mProcessID = strValue;
                else if(strAttribute == "PRODUCT")
                    mProductID = strValue;
                else if(strAttribute == "OPERATOR")
                    mOperatorName = strValue;
                else if(strAttribute == "PROBECARD")
                    mProbId = strValue;
                else if(strAttribute == "TECHNOLOGY")
                    mPackageId = strValue;
                else if(strAttribute == "SPECFILENAME")
                    mSpecName = strValue;
                else if(strAttribute == "SPECFILEVERSION")
                    mSpecVer = strValue;
                else if(strAttribute == "SPECFILEINSIDE")
                    continue;
                else if(strAttribute == "TEMPERATURE")
                    mTemperature = strValue;
                else if(strAttribute == "TESTPROGRAM")
                    mJobName = strValue;
                else if(strAttribute == "EQUIPMENT")
                    mEquipmentId= strValue;
                else if(strAttribute == "FLATORIENTATION")
                    mWaferFlat = strValue;
                else if(strAttribute == "OWNER")
                    mOwnerName = strValue;

            }
        }
        else if(strSection == "ETEST_LIMITS")
        {
            continue;
        }
        else if(strSection == "LIM")
        {
            // Save test limit info
            QMap<QString,QString>::Iterator itAttribute;
            QString strAttribute, strValue;
            int		nTestNum;
            QString	strTestName;
            QString	strUnit;
            QString	strLowLimit;
            QString	strHighLimit;
            for(itAttribute = mTagAttributes.begin(); itAttribute != mTagAttributes.end(); itAttribute++)
            {
                strAttribute = itAttribute.key().toUpper();
                strValue = itAttribute.value();

                if(strAttribute == "ID")
                    nTestNum = strValue.toInt();
                else if(strAttribute == "DESC")
                    strTestName = strValue;
                else if(strAttribute == "UNIT")
                    strUnit = strValue;
                else if(strAttribute == "SL")
                    strLowLimit = strValue;
                else if(strAttribute == "SH")
                    strHighLimit = strValue;
            }
            mTests[nTestNum].SetTestNumber(nTestNum);
            mTests[nTestNum].SetTestName(strTestName);
            mTests[nTestNum].SetTestUnit(strUnit);

            bool lConvert = false;
            mTests[nTestNum].SetLowLimit(strLowLimit.toFloat(&lConvert));
            mTests[nTestNum].SetValidLowLimit(lConvert);

            mTests[nTestNum].SetHighLimit(strHighLimit.toFloat(&lConvert));
            mTests[nTestNum].SetValidHighLimit(lConvert);

            mTests[nTestNum].SetResultScale(0);

            // Check if have unit
            // else try to retrieve unit at the end of the TestName
            if(strUnit.isEmpty())
            {
                // Check if it is a known unit
                QString strKnownUnits = "|volt|ampere|ohm|farad|hertz|decibel|watt|second|kelvin|degree|siemens|coulomb";
                strKnownUnits += "|volts|amperes|amp|amps|ohms|farads|hertzs|hz|hzs|decibels|db|dbs|watts|seconds|sec|secs|kelvins|degrees|deg|degs|coulombs";
                strKnownUnits += "|v|a|o|f|w|s|k|c|";

                strUnit = strTestName.section(" ",strTestName.count(" "));

                if(strKnownUnits.indexOf("|"+strUnit.toLower()+"|") >= 0)
                {
                    mTests[nTestNum].SetTestUnit(strUnit);
                }
            }

            if(!mTests[nTestNum].GetTestUnits().isEmpty())
            {
                QString lTestUnit = mTests[nTestNum].GetTestUnits();
                double lLowLimit = mTests[nTestNum].GetLowLimit();
                double lHighLimit = mTests[nTestNum].GetHighLimit();

                int lScale = 0;
                NormalizeValues(lTestUnit, lLowLimit, lScale);
                mTests[nTestNum].SetResultScale(lScale);
                NormalizeValues(lTestUnit, lHighLimit, lScale);
                mTests[nTestNum].SetResultScale(lScale);
            }
        }
        else if(strSection == "ETEST_WAFER_RUN")
        {
            // Data
            break;
        }
        else if(!mTagName.isEmpty())
        {
            // Ignore this tag
            if(!GotoMarker(QString("/"+mTagName).toLatin1().constData()))
            {
                mLastError = errInvalidFormatParameter;
                // Convertion failed.
                // Close file
                mFile.close();
                return false;
            }
        }
    }

    if(strSection != "ETEST_WAFER_RUN")
    {
        mLastError = errInvalidFormatParameter;
        // Convertion failed.
        // Close file
        mFile.close();
        return false;
    }

    // Close file
   // f.close();

    return true;
}


//////////////////////////////////////////////////////////////////////
// Parse the file and split line between each tags
//////////////////////////////////////////////////////////////////////
QString WatTsmXmlToStdf::ReadNextTag()
{
    mTagAttributes.clear();
    mTagValue = "";
    mTagName = "";

    if(mEndOfFile)
        return "";

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    UpdateProgressBar(mWatTsmXmlFile);

    int iPos;
    int iSpacePos;
    QString strString;

    while(!mEndOfFile)
    {
        if(mWatTsmXmlFile.atEnd())
            mEndOfFile = mCurrentLine.isEmpty();
        else
        {
            if(mCurrentLine.isEmpty() || mCurrentLine == " ")
                mCurrentLine = mWatTsmXmlFile.readLine().trimmed();
        }

        if(mCurrentLine.left(1) == "<")
        {
            // save tag name
            mCurrentLine = mCurrentLine.remove(0,1).trimmed();

            QRegExp space( "\\s" );     // match whitespace
            iPos = space.indexIn(mCurrentLine);

            // if no space (end of tag or end of line)
            if((iPos < 0)
            ||((mCurrentLine.indexOf('>') > 0) && (mCurrentLine.indexOf('>') < iPos)))
                iPos = mCurrentLine.indexOf('>');
            if(iPos < 0)
                iPos = mCurrentLine.length();

            mTagName = mCurrentLine.left(iPos);
            mCurrentLine = mCurrentLine.remove(0,iPos).trimmed();

            iPos = mCurrentLine.indexOf('>');
            // Check if have attribute ID="" into this line
            if(mCurrentLine.indexOf('"') > -1)
                iPos = -1;
            while(iPos < 0)
            {
                // save all attributes
                //iSpacePos = space.search(m_strCurrentLine);
                // iSpacePos after the ID=""
                iSpacePos = mCurrentLine.indexOf('"',mCurrentLine.indexOf('"')+1)+1;
                // if no space (end of tag or end of line)
                if(iSpacePos < 0)
                    iSpacePos = mCurrentLine.indexOf('>');
                if(iSpacePos < 0)
                    iSpacePos = mCurrentLine.length();

                strString = mCurrentLine.left(iSpacePos);
                mCurrentLine = mCurrentLine.remove(0,iSpacePos).trimmed();

                if(!strString.isEmpty())
                    mTagAttributes[strString.section("=",0,0)] = strString.section("=",1).remove("\"");

                if(mCurrentLine.isEmpty() || mCurrentLine == " ")
                    mCurrentLine = mWatTsmXmlFile.readLine().trimmed();
                iPos = mCurrentLine.indexOf('>');
                // Check if have attribute ID="" into this line
                if(mCurrentLine.indexOf('"') > -1)
                    iPos = -1;

                if(mWatTsmXmlFile.atEnd())
                {
                    mEndOfFile = true;
                    return "";
                }
            }
            strString = mCurrentLine.left(iPos).trimmed();
            mCurrentLine = mCurrentLine.remove(0,iPos+1).trimmed();
            if(!strString.isEmpty())
                mTagAttributes[strString.section("=",0,0)] = strString.section("=",1).remove("\"");

            mTagValue = "";
            // Then go to the next (END-)tag and save the current value
            while(!mEndOfFile)
            {
                if(mCurrentLine.isEmpty() || mCurrentLine == " ")
                    mCurrentLine = mWatTsmXmlFile.readLine().trimmed();

                iPos = mCurrentLine.indexOf('<');
                if(iPos < 0)
                    iPos = mCurrentLine.length();

                // save tag value
                strString = mCurrentLine.left(iPos).trimmed();
                mCurrentLine = mCurrentLine.remove(0,iPos).trimmed();
                if(!strString.isEmpty())
                    mTagValue += strString + " ";

                if(iPos >= 0)
                    break;

                if(mWatTsmXmlFile.atEnd())
                {
                    mEndOfFile = true;
                    return "";
                }
            }
            return mTagName;
        }
        else
        {
            // bad pos ??
            // Try to found next tag
            mCurrentLine = "";
        }
    }
    return mTagName;
}


//////////////////////////////////////////////////////////////////////
bool WatTsmXmlToStdf::GotoMarker(const char *szEndMarker)
{
    QString strLine;
    QString strSection;
    QString strEndMarker(szEndMarker);
    strEndMarker = strEndMarker.toUpper();

    while(!mEndOfFile)
    {

        strLine = ReadNextTag();
        if(strLine.isEmpty() || strLine == " ")
            continue;

        // if found a 'tag', probably a keyword
        strSection = strLine.toUpper();
        if(strSection == strEndMarker)
            return true;
    }
    return false;
}



//////////////////////////////////////////////////////////////////////
// Read and Parse the WatTsmXml file
//////////////////////////////////////////////////////////////////////
bool WatTsmXmlToStdf::WriteStdfFile(const char *stdFileName)
{
    // now generate the STDF file...
    if(mStdfParse.Open(stdFileName, STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        mWatTsmXmlFile.device()->close();
        return false;
    }

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    mTestCode= "WAFER";
    mFamilyID = mProductID;
    SetMIRRecord(lMIRV4Record, ":WatTsm/XML", false);
    lMIRV4Record.SetOPER_FRQ("");
    lMIRV4Record.SetSPEC_NAM(mSpecName);
    lMIRV4Record.SetSPEC_VER(mSpecVer);

    mStdfParse.WriteRecord(&lMIRV4Record);


    // Write SDR for last wafer inserted
    GQTL_STDF::Stdf_SDR_V4 lRV4Record;
    lRV4Record.SetHEAD_NUM(255);            // Test Head = ALL
    lRV4Record.SetSITE_GRP((BYTE)1);        // Group#
    lRV4Record.SetSITE_CNT((BYTE)1);        // site_count
    lRV4Record.SetSITE_NUM((BYTE)1, 1);     // array of test site#
    lRV4Record.SetCARD_ID(mProbId);			// CARD_ID: Probe card name
    lRV4Record.SetEXTR_ID(mEquipmentId);	// EXTR_ID: Extra equipment name
    mStdfParse.WriteRecord(&lRV4Record);


    // Write WCR
    if(mWaferFlat.isEmpty())
        mWaferFlat = " ";
    GQTL_STDF::Stdf_WCR_V4  lWC4Record;
    lWC4Record.SetWAFR_SIZ(0);				// Wafer size
    lWC4Record.SetDIE_HT(0);				// Height of die
    lWC4Record.SetDIE_WID(0);				// Width of die
    lWC4Record.SetWF_UNITS(0);              // Units are in millimeters
    lWC4Record.SetWF_FLAT(mWaferFlat[0].toLatin1());      // Orientation of wafer flat
    mStdfParse.WriteRecord(&lWC4Record);

    // Write Test results for each line read.
    QString		strSection;
    QString     strWaferId;

    bool		bHaveValue;
    bool		bTestPass;
    float		fValue;
    int			nTestNum;
    int			nHardBin;
    int			nTotalGoodParts=0, nTotalParts=0;
    int			nSumGoodParts, nSumFailParts;
    int			nNbTests;
    int			nSiteId=0;
    int			nXpos=0, nYpos=0;
    bool		bPassStatus=0;
    //FIXME: not used ?
    //bool bWaferStatus=true;
    BYTE		bData=0;

    nSumFailParts = nSumGoodParts = 0;
    nNbTests = 0;
    // Write all Part read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR

    // Current tag is for the first wafer
    // Do not read next tag
    bool	bReadNextTag = false;
    strSection = mTagName;

    GQTL_STDF::Stdf_WIR_V4  lWIRRecord;
    GQTL_STDF::Stdf_WRR_V4  lWRRRecord;
    GQTL_STDF::Stdf_PIR_V4  lPIRV4Record;
    GQTL_STDF::Stdf_PRR_V4  lPRRV4Record;
    GQTL_STDF::Stdf_PTR_V4  lPTRV4Record;

    while(mWatTsmXmlFile.atEnd() == false)
    {
        // Read line
        if(bReadNextTag)
            strSection = ReadNextTag().toUpper();
        bReadNextTag = true;

        if(strSection == "/ETEST_LOT_RUN")
        {
            break;
        }
        else if(strSection == "/ADB_DOCUMENT")
        {
            break;
        }

        if(strSection == "ETEST_WAFER_RUN")
        {
            // Save all Attributes
            QMap<QString,QString>::Iterator itAttribute;
            QString strAttribute, strValue;
            for(itAttribute = mTagAttributes.begin(); itAttribute != mTagAttributes.end(); itAttribute++)
            {
                strAttribute = itAttribute.key().toUpper();
                strValue = itAttribute.value();

                if(strAttribute == "WAFERNUMBER")
                    strWaferId = strValue;
                else if(strAttribute == "SITECOUNT")
                    continue;
                else if(strAttribute == "PARAMETERCOUNT")
                    continue;
                //FIXME: not used ?
                //else
                //if(strAttribute == "WAFERPASS")
                //  bWaferStatus = (strValue.toUpper() == "Y");
                else if(strAttribute == "COMMENTS")
                    continue;
            }

            // Write WIR of new Wafer.
            SetWIRRecord(lWIRRecord, strWaferId.toInt());
            mStdfParse.WriteRecord(&lWIRRecord);
            lWIRRecord.Reset();

            nTotalParts= 0;
            bPassStatus = true;
            nTotalGoodParts = 0;

            continue;
        }
        else if(strSection == "/ETEST_WAFER_RUN")
        {
            // Write WRR for last wafer inserted
            SetWRRRecord(lWRRRecord, strWaferId.toInt(), nTotalParts, nTotalGoodParts);
            mStdfParse.WriteRecord(&lWRRRecord);
            lWRRRecord.Reset();
            continue;
        }
        else if(strSection == "SITE")
        {
            // Save all Attributes
            QMap<QString,QString>::Iterator itAttribute;
            QString strAttribute, strValue;
            for(itAttribute = mTagAttributes.begin(); itAttribute != mTagAttributes.end(); itAttribute++)
            {
                strAttribute = itAttribute.key().toUpper();
                strValue = itAttribute.value();

                if(strAttribute == "SITEID")
                    nSiteId = strValue.toInt();
                else if(strAttribute == "SITEX")
                    nXpos = strValue.toInt();
                else if(strAttribute == "SITEY")
                    nYpos = strValue.toInt();
            }


            // Write PIR
            lPIRV4Record.SetHEAD_NUM(1);							// Test head
            lPIRV4Record.SetSITE_NUM(nSiteId);					// Tester site
            mStdfParse.WriteRecord(&lPIRV4Record);
            lPIRV4Record.Reset();

            nNbTests = 0;
            bPassStatus = true;
        }
        else if(strSection == "/SITE")
        {
            nTotalParts++;

            // Write PRR
            lPRRV4Record.SetHEAD_NUM(1);                // Test head
            lPRRV4Record.SetSITE_NUM(nSiteId);			// Tester site#:1
            if(!bPassStatus)
            {
                lPRRV4Record.SetPART_FLG(8);			// PART_FLG : FAILED
                nHardBin = 0;
                nSumFailParts++;
            }
            else
            {
                lPRRV4Record.SetPART_FLG(0);                // PART_FLG : PASSED
                nTotalGoodParts++;
                nSumGoodParts++;
                nHardBin = 1;
            }
            lPRRV4Record.SetNUM_TEST(nNbTests);			// NUM_TEST
            lPRRV4Record.SetHARD_BIN(nHardBin);         // HARD_BIN
            lPRRV4Record.SetSOFT_BIN(nHardBin);         // SOFT_BIN
            lPRRV4Record.SetX_COORD(nXpos);				// X_COORD
            lPRRV4Record.SetY_COORD(nYpos);				// Y_COORD
            lPRRV4Record.SetTEST_T(0);					// No testing time known...
            lPRRV4Record.SetPART_ID(QString::number(nTotalParts));		// PART_ID
//            lPRRV4Record.SetPART_TXT("");				// PART_TXT
//            lPRRV4Record.SetPART_FIX();                 // PART_FIX
            mStdfParse.WriteRecord(&lPRRV4Record);

            nNbTests = 0;
            bPassStatus = true;

        }
        else if(strSection == "T")
        {

            nTestNum = mTagValue.section(",",0,0).toInt();
            fValue   = mTagValue.section(",",1).toFloat(&bHaveValue);

            if(bHaveValue)
            {
                nNbTests++;
                bTestPass = true;

                if(mTests.contains(nTestNum))
                {
                    // Check for limits
                    if((mTests[nTestNum].GetValidLowLimit())
                            && (mTests[nTestNum].GetLowLimit() > fValue))
                        bTestPass = false;
                    if((mTests[nTestNum].GetValidHighLimit())
                            && (mTests[nTestNum].GetHighLimit() < fValue))
                        bTestPass = false;
                }
                else
                {
                    mTests[nTestNum].SetTestNumber(nTestNum);
                    mTests[nTestNum].SetTestName("");
                    mTests[nTestNum].SetValidLowLimit(false);
                    mTests[nTestNum].SetValidHighLimit(false);
                    mTests[nTestNum].SetHighLimit(0.0);
                    mTests[nTestNum].SetLowLimit(0.0);
                    mTests[nTestNum].SetTestUnit("");
                    mTests[nTestNum].SetResultScale(0);
                }

                fValue *= GS_POW(10.0,mTests[nTestNum].GetResultScale());

                if(!bTestPass)
                    bPassStatus = false;

                // Write PTR
                lPTRV4Record.SetTEST_NUM(nTestNum);			// Test Number
                lPTRV4Record.SetHEAD_NUM(1);				    // Test head
                lPTRV4Record.SetSITE_NUM(nSiteId);              // Tester site#
                if(bTestPass)
                    bData = 0;                                  // Test passed
                else
                    bData = BIT7;                               // Test Failed
                lPTRV4Record.SetTEST_FLG(bData);				// TEST_FLG
                bData = BIT6|BIT7;
                lPTRV4Record.SetPARM_FLG(bData);				// PARAM_FLG
                lPTRV4Record.SetRESULT(fValue);                 // Test result


                // save Parameter name
                lPTRV4Record.SetTEST_TXT(mTests[nTestNum].GetTestName());	// TEST_TXT
                lPTRV4Record.SetALARM_ID("");						// ALARM_ID

                bData = 2;	// Valid data.
                if(!mTests[nTestNum].GetValidLowLimit())
                    bData |= BIT6;
                if(!mTests[nTestNum].GetValidHighLimit())
                    bData |= BIT7;
                lPTRV4Record.SetOPT_FLAG(bData);					      // OPT_FLAG

                lPTRV4Record.SetRES_SCAL(-mTests[nTestNum].GetResultScale());       // RES_SCALE
                lPTRV4Record.SetLLM_SCAL(-mTests[nTestNum].GetResultScale());       // LLM_SCALE
                lPTRV4Record.SetHLM_SCAL(-mTests[nTestNum].GetResultScale());       // HLM_SCALE
                lPTRV4Record.SetLO_LIMIT(mTests[nTestNum].GetLowLimit());	  // LOW Limit
                lPTRV4Record.SetHI_LIMIT(mTests[nTestNum].GetHighLimit());	  // HIGH Limit
                lPTRV4Record.SetUNITS(mTests[nTestNum].GetTestUnits());		      // Units

                mStdfParse.WriteRecord(&lPTRV4Record);
                lPTRV4Record.Reset();
            }

            if(!GotoMarker("/T"))
            {
                mLastError = errInvalidFormatParameter;
                mWatTsmXmlFile.device()->close();
                return false;
            }
        }

    };			// Read all lines with valid data records in file


    // Write HBR Bin0 (FAIL)
    WriteHBR(0, nSumFailParts, false, 255, 0);

    // Write HBR Bin1 (PASS)
    WriteHBR(1, nSumGoodParts, true, 255, 0);

    WriteSBR(0, nSumFailParts, false, 255, 0);

    // Write SBR Bin1 (PASS)
    WriteSBR(1, nSumGoodParts, true, 255, 0);

    // Write PCR for last wafer inserted
    GQTL_STDF::Stdf_PCR_V4  lPCRV4Record;
    lPCRV4Record.SetHEAD_NUM(255);					// Test Head = ALL
    lPCRV4Record.SetSITE_NUM(1);						// Test sites = ALL
    lPCRV4Record.SetPART_CNT(nSumFailParts + nSumGoodParts);	// Parts tested
    lPCRV4Record.SetRTST_CNT(0);						// Parts retested
    lPCRV4Record.SetABRT_CNT(0);						// Parts Aborted
    lPCRV4Record.SetGOOD_CNT(nSumGoodParts);			// Good Parts
    lPCRV4Record.SetFUNC_CNT(4294967295UL);			// Functionnal Parts
    mStdfParse.WriteRecord(&lPCRV4Record);

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRV4Record;
    lMRRV4Record.SetFINISH_T(mStartTime);			// File finish-time.
    mStdfParse.WriteRecord(&lMRRV4Record);

    // Close STDF file.
    mStdfParse.Close();

    mWatTsmXmlFile.device()->close();
    // Success
    return true;
}


//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void WatTsmXmlToStdf::NormalizeValues(QString &strUnits,double &fValue, int &nScale)
{
    QString strValue = strUnits;

    nScale = 0;

    // In strValue, the current value with unit
    if(strValue == "-")
    {
        strUnits = "";
        return;
    }

    if(strValue.toUpper() == "P/F")
    {
        return;
    }

    if(strValue.toUpper() == "_%")
    {
        strUnits = "%";
        return;
    }

    if(strUnits.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        return;
    }

    GS::Core::NormalizeUnit(static_cast<const QString>(strUnits[0]), nScale);
    fValue *= GS_POW(10.0,nScale);
    if(nScale)
        strUnits = strUnits.mid(1);	// Take all characters after the prefix.
}
