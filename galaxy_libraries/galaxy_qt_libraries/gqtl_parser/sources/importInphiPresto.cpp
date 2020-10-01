//////////////////////////////////////////////////////////////////////
// import_csv.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importInphiPrestro.h"
#include "importConstants.h"

#include "gqtl_log.h"
#include "gqtl_global.h"
#include <math.h>

#include <qfileinfo.h>

/// In some cases, we have an optional field Source_Lot just after the wafer_id
//Device_ID,Family,LOT_ID,Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,RETX,RETY,DIE,SERIAL,Chan,1100,1006,2000,2001,2002,3000,3001,3002,3001,3002,3008,3009,3003,3004,3005,3006,3007,3010,4000,4001,4002,4003,4004,4005,BIN,SoftBinDescription,DATE,TEST_START_TIME,TEST_END_TIME
//,,,,,,,,,,,,,,,,,,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
//,,,,,,,,,,,,,,,,,,Icc,Vos,N_Points,Start_Freq,Stop_Freq,S21_Pk,S22_Pk_Lo,S22_Pk_Lo_Freq,S22_Pk_Lo,S22_Pk_Lo_Freq,S22_Pk_Hi_10GHz,S22_Pk_Hi_10GHz_Freq,S22_Pk_Hi_12GHz,S22_Pk_Hi_12GHz_Freq,Zt,BW_Hist_Err,BW,Zt0,K1_min,K1_minfreq,K2_min,K2_minfreq,K3_min,K3_minfreq
//,,,,,,,,,,,,,,,,,,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA
//,,,,,,,,,,,,,,,,,,A,V,none,GHz,GHz,dB,dB,GHz,dB,GHz,dB,GHz,dB,GHz,ohm,GHz,GHz,ohm,none,GHz,none,GHz,none,GHz
//,,,,,,,,,,,,,,,,,,0.028,-0.024,None,None,None,None,None,None,None,None,None,None,None,None,5750.0,None,9.0,None,1.0,None,2.0,None,0.8,None
//,,,,,,,,,,,,,,,,,,0.039,0.018,None,None,None,None,-7.0,None,-10.0,None,-7.0,None,-7.0,None,7300.0,None,11.6,None,None,None,None,None,None,None
//1347TL-S01D,TIA,C5036,WFR-0074-1,NA,ULTRA5,NA,EUDYNA,1347TL-S01_SS,3.02,FD,25,EJJ,6,2,1,NA,Global,0.03135,0.0155,101,0.05,30.050001,41.64,None,None,-14.3,50000000.0,-9.16,8150000000.0,-9.16,8150000000.0,6076,10.01,9.71,6047,3.22,950000000.0,5.28,14150000000.0,4.47,20450000000.0,1,NA,06/19/2014,16:30:02,16:30:06
//1347TL-S01D,TIA,C5036,WFR-0074-1,NA,ULTRA5,NA,EUDYNA,1347TL-S01_SS,3.02,FD,25,EJJ,5,2,1,NA,Global,0.03176,0.0021,101,0.05,30.050001,41.51,None,None,-14.53,50000000.0,-9.17,8150000000.0,-9.17,8150000000.0,5945,10.15,9.85,6061,3.43,950000000.0,4.67,14150000000.0,4.35,20150000000.0,1,NA,06/19/2014,16:30:07,16:30:18
//1347TL-S01D,TIA,C5036,WFR-0074-1,NA,ULTRA5,NA,EUDYNA,1347TL-S01_SS,3.02,FD,25,EJJ,3,3,1,NA,Global,0.03288,0.0099,101,0.05,30.050001,41.41,None,None,-15.34,50000000.0,-9.22,8150000000.0,-9.22,8150000000.0,5681,10.37,10.07,5700,3.15,950000000.0,5.52,14150000000.0,4.22,20450000000.0,2,NA,06/19/2014,16:30:19,16:30:23

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
InphiPrestotoSTDF::InphiPrestotoSTDF() : ParserBase(typeInphiPresto, "InphiPresto")
{
    mTestList = NULL;
    mParameterDirectory.SetFileName(GEX_TRI_INPHIPRESTO_PARAMETERS);
    mFirstTestRaw = 0;
    mShift = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
InphiPrestotoSTDF::~InphiPrestotoSTDF()
{
    // Destroy list of Parameters tables.
    if(mTestList!=NULL)
        delete [] mTestList;

    // Destroy list of Bin tables.
    qDeleteAll(mSoftBinning.values());
    mSoftBinning.clear();

}



//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool InphiPrestotoSTDF::IsCompatible(const QString &FileName)
{
    bool	lIsCompatible(false);

    QFile lFile(FileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);


    // Check the compatibility
    // Obsolete header (must be rejected)
    //Device_ID,Family,LOT_ID,                           Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,RETX,RETY,DIE,    SERIAL,Chan,1100,...,3001,BIN,SoftBinDescription,DATE,TEST_START_TIME,TEST_END_TIME
    // For Final Test
    //Device_ID,Family,LOT_ID,Source_Lot,...,Source_Lot6,Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,RETX,RETY,DIE,    SERIAL,CHAN,12000,..,5900,HBIN,P/F,SoftBinDescription,DATE,TEST_START_TIME,TEST_END_TIME
    // For Wafer Sort
    //Device_ID,Family,LOT_ID,Source_Lot,...,Source_Lot6,Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,RETX,RETY,DIE,X,Y,SERIAL,Chan,12,......,700, BIN,P/F,SoftBinDescription,DATE,TEST_START_TIME,TEST_END_TIME

    QString lStrString = lCsvFile.readLine();
    // Read the first line
    // Check if have all mandatories fields
    if(!lStrString.startsWith("Device_ID,Family,LOT_ID",Qt::CaseInsensitive))
        lIsCompatible = false;
    // Must end with
    else if(!lStrString.endsWith("DATE,TEST_START_TIME,TEST_END_TIME",Qt::CaseInsensitive))
        lIsCompatible = false;
    else
        lIsCompatible = true;

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
bool InphiPrestotoSTDF::ConvertoStdf(const QString &CsvFileName,  QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( CsvFileName).toLatin1().constData());

    QString lStrString;

    // Open CSV file
    QFile lFile( CsvFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;
        mLastErrorMessage = lFile.errorString();
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);

    lStrString = ReadLine(lCsvFile);
    //Device_ID,Family,LOT_ID,Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,RETX,RETY,DIE,SERIAL,Chan,1100,1006,2000,2001,2002,3000,3001,3002,3001,BIN,SoftBinDescription,DATE,TEST_START_TIME,TEST_END_TIME
    //Device_ID,Family,LOT_ID,Source_Lot,Source_Lot2,Source_Lot3,Source_Lot4,Source_Lot5,Source_Lot6,Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,
    // Revision 4
    // Mandaroty fields for Final Test
    // Device_ID,Family,LOT_ID,...,Test Hardware,...TestProg,...,Temperature,...,SERIAL,...,HBIN,P/F,...,DATE,TEST_START_TIME,TEST_END_TIME
    // Mandaroty fields for Wafer Sort
    // Device_ID,Family,LOT_ID,...,Wafer_ID,...TestProg,...,Temperature,...,RETX,RETY,DIE,X,Y,...,BIN,P/F,...,DATE,TEST_START_TIME,TEST_END_TIME
    // Extract the headers
    //   o switch to UPPER case
    //   o remove '(comment')
    lStrString = lStrString.toUpper().replace("HBIN","BIN");
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    for(int i=0; i<lStrCells.count(); i++)
        mFieldPos[lStrCells[i].section("(",0,0).trimmed()] = i;

    // Check if have at least the RETX
    if(!mFieldPos.contains("RETX"))
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Missing mandatory column RETX";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }

    // To fix the GCORE-5081, read first-of-all 10 lines to go to a data line and check if we are in FT or WS file
    for (int i=0; i<8; i++)
        lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if(GetNotNAString(lStrCells[mFieldPos["RETX"]]).isEmpty())
    {
        // No RETX => must be FinalTest
        mIsWaferSortFile = false;
    }
    else
        mIsWaferSortFile = true;

    // Check if have all mandatories fields
    QStringList lMandatoryField;
    lMandatoryField << "DEVICE_ID" << "FAMILY" << "LOT_ID" << "WAFER_ID" << "TEST HARDWARE" << "PLATFORM" << "PROCESS";
    lMandatoryField << "FOUNDRY" << "TESTPROG" << "TESTPROG_REV" << "TEST TYPE" << "TEMPERATURE" << "OPERATOR_ID";
    lMandatoryField << "RETX" << "RETY" << "CHAN" << "BIN" << "P/F" << "DATE" << "TEST_START_TIME" << "TEST_END_TIME";
    if(mIsWaferSortFile)
        lMandatoryField << "X" << "Y";
    else
        lMandatoryField << "SERIAL";

    foreach(const QString lField, lMandatoryField)
    {
        if(!mFieldPos.contains(lField))
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "Missing mandatory column '"+lField+"'";
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            lFile.close();
            return false;
        }
    }

    // Check if have all mandatory value
    QStringList lMandatoryValue;
    lMandatoryValue << "DEVICE_ID" << "LOT_ID" << "TEMPERATURE" << "BIN" << "P/F" << "DATE" << "TEST_START_TIME" << "TEST_END_TIME";
    if(mIsWaferSortFile)
        lMandatoryValue << "WAFER_ID" << "X" << "Y";

    foreach(const QString lValue, lMandatoryValue)
    {
        if(GetNotNAString(lStrCells[mFieldPos[lValue]]).isEmpty())
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "Column '"+lValue+"' must have a valid value";
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            lFile.close();
            return false;
        }
    }

    lCsvFile.seek(0);

    // The first result is just after the Channel column
    mFirstTestRaw = mFieldPos["CHAN"] + 1;

    // The list of tests is between the "Chan" and "BIN"
    mTotalParameters = mFieldPos["BIN"] - mFieldPos["CHAN"] -1;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 Total Parameters").arg( mTotalParameters).toLatin1().constData());
    // Allocate the buffer to hold the N parameters & results.
    try
    {
        mTestList = new ParserParameter[mTotalParameters];	// List of parameters
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        lFile.close();
        return false;
    }
    catch(...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        lFile.close();
        return false;
    }

    // Check the compatibility
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    long lTestNumber;
    // Set test number in the csv parameters
    for (unsigned i = 0; i < mTotalParameters; ++i)
    {
        lTestNumber = lStrCells[mFirstTestRaw + i].toLong();
        mTestList[i].SetTestNumber(lTestNumber);
     }

    // Line 2 - Measurement group : nothing to do
    lStrString = ReadLine(lCsvFile);
    if (lStrString.section(' ', 1).simplified() != "")
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage ="The line n°2 for Measurement group is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }

    // Line 3 - Test name
    QString lTestName;
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    // Set test name in the csv parameters
    if ((unsigned)lStrCells.size() < mFirstTestRaw+mTotalParameters)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line n°3 for test name is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }
    for (unsigned i = 0; i < mTotalParameters; ++i)
    {
        lTestName = lStrCells[mFirstTestRaw + i];
        mTestList[i].SetTestName(lTestName);
    }

    // Line 4 - Test condition : nothing to do
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    // Set test condition in the csv parameters
    if ((unsigned)lStrCells.size() < mFirstTestRaw+mTotalParameters)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line line 4: Test condition is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }
    for (unsigned i = 0; i < mTotalParameters; ++i)
    {
        QString lCondition = lStrCells[mFirstTestRaw + i];
        if (!GetNotNAString(lCondition).isEmpty())
        {
            mTestList[i].SetTestName(mTestList[i].GetTestName() + lCondition);
        }
    }


    // read units
    int lScalingFactor = 0;
    // Line 5 - Unit
    if(!IsNotAtEndOfFile(lCsvFile))
    {
        lFile.close();
        return false;
    }
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    if ((unsigned)lStrCells.size() < mFirstTestRaw+mTotalParameters)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line 5 for units is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }
    lStrCells = lStrString.split(",");
    QString lUnitString;
    for(unsigned i=0; i<mTotalParameters; ++i)
    {
        lScalingFactor = 0;
        lUnitString = lStrCells[mFirstTestRaw + i];
        if (!GetNotNAString(lUnitString).isEmpty())
        {
            mTestList[i].SetTestUnit(GS::Core::NormalizeUnit(lUnitString, lScalingFactor));
            mTestList[i].SetResultScale(lScalingFactor);
        }
    }

    // Line 6 - Low limit
    if(!IsNotAtEndOfFile(lCsvFile))
    {
        lFile.close();
        return false;
    }
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    if ((unsigned)lStrCells.size() < mFirstTestRaw+mTotalParameters)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line 6 for low limit is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }
    QString lLowLimit;
    for (unsigned i = 0; i < mTotalParameters; ++i)
    {
        lLowLimit = lStrCells[mFirstTestRaw + i];
        if (!GetNotNAString(lLowLimit).isEmpty())
        {
            mTestList[i].SetLowLimit(GS::Core::NormalizeValue(lLowLimit.toDouble(), mTestList[i].GetResultScale()));
            mTestList[i].SetValidLowLimit(true);
        }
    }

    // Line 7 - High limit
    if(!IsNotAtEndOfFile(lCsvFile))
    {
        lFile.close();
        return false;
    }
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split("," ,QString::KeepEmptyParts);
    if ((unsigned)lStrCells.size() < mFirstTestRaw+mTotalParameters)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "The line 7 for high limit is wrongly formated";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        lFile.close();
        return false;
    }
    QString lHighLimit;
    for (unsigned i = 0; i < mTotalParameters; ++i)
    {
        lHighLimit = lStrCells[mFirstTestRaw + i];
        if (!GetNotNAString(lHighLimit).isEmpty())
        {
            mTestList[i].SetHighLimit(GS::Core::NormalizeValue(lHighLimit.toDouble(), mTestList[i].GetResultScale()));
            mTestList[i].SetValidHighLimit(true);
        }
    }

    if(!IsNotAtEndOfFile(lCsvFile))
    {
        lFile.close();
        return false;
    }

    if(WriteStdfFile(lCsvFile, StdfFileName) != true)
    {
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    mLastError = errNoError;
    mLastErrorMessage = "";
    // Success parsing CSV file
    return true;
}

gsbool InphiPrestotoSTDF::IsNotAtEndOfFile(const QTextStream& lCsvFile)
{
    if(lCsvFile.atEnd() == true)
    {
        // Incorrect header...this is not a valid CSV file!
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "Unexpected end of file";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        // Convertion failed. Close file
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool InphiPrestotoSTDF::WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    ParserBinning *lBinning;
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    GQTL_STDF::Stdf_WCR_V4 lWCRRecord;
    GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
    GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;

    if(lStdfFile.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mLastError = errWriteSTDF;
        mLastErrorMessage = "Enable to open the STDF file";
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        // Convertion failed.
        return false;
    }

    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    lStdfFile.SetStdfCpuType(lStdfFile.GetComputerCpuType());

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;
    lFARrecord.SetCPU_TYPE(lStdfFile.GetComputerCpuType());	 // Force CPU type to current computer platform.
    lFARrecord.SetSTDF_VER(4);                               // STDF V4
    lFARrecord.Write(lStdfFile);


    QStringList lFields;
    QString lLine;

    lLine = ReadLine(CsvFile);
    lFields = lLine.split(",");
    // Write MIR
    //Device_ID,Family,LOT_ID,Wafer_ID,Test Hardware,Platform,Process,Foundry,TestProg,TestProg_Rev,Test Type,Temperature,Operator_ID,RETX,RETY,DIE,SERIAL,Chan,
    lMIRRecord.SetPART_TYP(GetNotNAString(lFields[mFieldPos["DEVICE_ID"]]));
    lMIRRecord.SetFAMLY_ID(GetNotNAString(lFields[mFieldPos["FAMILY"]]));
    lMIRRecord.SetLOT_ID(GetNotNAString(lFields[mFieldPos["LOT_ID"]]));
    if (mIsWaferSortFile)
    {
        lWIRRecord.SetWAFER_ID(lFields[mFieldPos["WAFER_ID"]]);
        lWRRRecord.SetWAFER_ID(lFields[mFieldPos["WAFER_ID"]]);
    }
    if (GetNotNAString(lFields[mFieldPos["TEST HARDWARE"]]).isEmpty())
        lMIRRecord.SetSTAT_NUM(1);
    else
        lMIRRecord.SetSTAT_NUM(lFields[mFieldPos["TEST HARDWARE"]].toInt());
    lMIRRecord.SetTSTR_TYP(GetNotNAString(lFields[mFieldPos["PLATFORM"]]));
    lMIRRecord.SetPROC_ID(GetNotNAString(lFields[mFieldPos["PROCESS"]]));
    lMIRRecord.SetFACIL_ID(GetNotNAString(lFields[mFieldPos["FOUNDRY"]]));
    lMIRRecord.SetJOB_NAM(GetNotNAString(lFields[mFieldPos["TESTPROG"]]));
    lMIRRecord.SetJOB_REV(GetNotNAString(lFields[mFieldPos["TESTPROG_REV"]]));
    lMIRRecord.SetTEST_COD(GetNotNAString(lFields[mFieldPos["TEST TYPE"]]));
    lMIRRecord.SetTST_TEMP(GetNotNAString(lFields[mFieldPos["TEMPERATURE"]]));
    lMIRRecord.SetOPER_NAM(GetNotNAString(lFields[mFieldPos["OPERATOR_ID"]]));
    lSDRRecord.SetEXTR_ID(GetNotNAString(lFields[mFieldPos["CHAN"]]));

    mStartTime = GetDate(lFields[mFieldPos["DATE"]], lFields[mFieldPos["TEST_START_TIME"]]);
    if(mStartTime == 0)
    {
        lStdfFile.Close();
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QString("Invalid date/time format: date=%1, time=%2")
              .arg(lFields[mFieldPos["DATE"]]).arg(lFields[mFieldPos["TEST_START_TIME"]]);
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

        return false;
    }
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSETUP_T(mStartTime);

    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":CSV";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    lMIRRecord.Write(lStdfFile);

    if (mFieldPos.contains("SOURCE_LOT"))
    {
        QScopedPointer<GQTL_STDF::Stdf_DTR_V4> lDTRRecord(new GQTL_STDF::Stdf_DTR_V4);
        AddMetaDataToDTR("subcon_lot_id", lFields[mFieldPos["SOURCE_LOT"]], lDTRRecord.data());
        lDTRRecord->Write(lStdfFile);
    }

    // write SDR
    lSDRRecord.SetHEAD_NUM(1);
    lSDRRecord.SetSITE_GRP(1);
    lSDRRecord.SetSITE_CNT(1);
    lSDRRecord.SetSITE_NUM(1, 1);
    lSDRRecord.Write(lStdfFile);

    // Write the WIR
    if (mIsWaferSortFile)
    {
        // Write WIR
        lWIRRecord.SetHEAD_NUM(1);
        lWIRRecord.SetSITE_GRP(255);
        lWIRRecord.SetSTART_T(mStartTime);
        lWIRRecord.Write(lStdfFile);
    }


    // Write Test results for each line read.
    QString lLineString;
    BYTE	lValidLimit;
    int lPartsPerWafer = 0, lGoodPartsPerWafer = 0;
    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    gstime_t lFinishTime = -1;
    while(CsvFile.atEnd() == false) // Read all lines in file
    {
        if (lFields.size() == 0)
        {
            // Read line
            lLineString = ReadLine(CsvFile);
            // Split line
            lFields = lLineString.split(",",QString::KeepEmptyParts);
            // ignore empty lines
            if(lLineString.isEmpty())
                continue;
        }
        //++lNumberDies;
        if (lFields.size() < static_cast<int>(mFieldPos["TEST_END_TIME"]))
        {
            lStdfFile.Close();
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("The number of parameter in this line is less than the expected one = %1")
                  .arg(lLineString);
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());

            return false;
        }

        ++lPartsPerWafer;
        // Write PIR
        GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
        lPIRRecord.SetHEAD_NUM(1);
        lPIRRecord.SetSITE_NUM(1);
        lPIRRecord.Write(lStdfFile);

        // Read Parameter results for this record
        for(unsigned lIndex=0; lIndex<mTotalParameters; ++lIndex)
        {
            // If it's a PTR (pattern name empty)
            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            lPTRRecord.SetTEST_NUM(mTestList[lIndex].GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            lPTRRecord.SetSITE_NUM(1);
            lPTRRecord.SetPARM_FLG(0);
            bool ok;
            float lValue = GS::Core::NormalizeValue(lFields[mFirstTestRaw + lIndex].toDouble(&ok),
                                                    mTestList[lIndex].GetResultScale());
            if (ok)
            {
                lPTRRecord.SetTEST_FLG(0x40);       // Result, no P/F indication
                lPTRRecord.SetRESULT(lValue);
            }
            else
            {
                lPTRRecord.SetTEST_FLG(0x52);       // No result, no Pass/fail indication
                lPTRRecord.SetRESULT(0);
            }
            lPTRRecord.SetTEST_TXT(mTestList[lIndex].GetTestName());

            if (mTestList[lIndex].GetStaticHeaderWritten() == false)
            {
                mTestList[lIndex].SetStaticHeaderWritten(true);
                lPTRRecord.SetRES_SCAL(mTestList[lIndex].GetResultScale());
                lPTRRecord.SetLLM_SCAL(mTestList[lIndex].GetResultScale());
                lPTRRecord.SetHLM_SCAL(mTestList[lIndex].GetResultScale());
                lValidLimit = 0x0e;
                if(mTestList[lIndex].GetValidLowLimit()==false)
                    lValidLimit |=0x40;
                if(mTestList[lIndex].GetValidHighLimit()==false)
                    lValidLimit |=0x80;
                lPTRRecord.SetOPT_FLAG(lValidLimit);
                lPTRRecord.SetLO_LIMIT(mTestList[lIndex].GetLowLimit());
                lPTRRecord.SetHI_LIMIT(mTestList[lIndex].GetHighLimit());
                lPTRRecord.SetUNITS(mTestList[lIndex].GetTestUnits());
            }
            lPTRRecord.Write(lStdfFile);
        };

        // Write PRR
        GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
        mStartTime = GetDate(lFields[mFieldPos["DATE"]], lFields[mFieldPos["TEST_START_TIME"]]);
        if(mStartTime == 0)
        {
            lStdfFile.Close();
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("Invalid date/time format: date=%1, time=%2")
                  .arg(lFields[mFieldPos["DATE"]]).arg(lFields[mFieldPos["TEST_START_TIME"]]);
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            return false;
        }
        lFinishTime = GetDate(lFields[mFieldPos["DATE"]], lFields[mFieldPos["TEST_END_TIME"]]);
        if(lFinishTime == 0)
        {
            lStdfFile.Close();
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("Invalid date/time format: date=%1, time=%2")
                  .arg(lFields[mFieldPos["DATE"]]).arg(lFields[mFieldPos["TEST_END_TIME"]]);
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            return false;
        }
        // Check if we switched day, if so recompute finish time using next day
        if(lFinishTime < mStartTime)
            lFinishTime = GetDate(lFields[mFieldPos["DATE"]], lFields[mFieldPos["TEST_END_TIME"]], 1);

        lPRRRecord.SetTEST_T((lFinishTime - mStartTime)*1000);
        lPRRRecord.SetSITE_NUM(1);
        int lSoftBin = lFields[mFieldPos["BIN"]].toUShort();
        bool lBinFlag;
        QString lBinDesc;
        if(lFields[mFieldPos["P/F"]].toUpper()!="PASS"
                && lFields[mFieldPos["P/F"]].toUpper()!="FAIL")
        {            lStdfFile.Close();
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("Invalid P/F value = %1")
                  .arg(lFields[mFieldPos["P/F"]]);
            GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
            return false;
        }
        lBinFlag = lFields[mFieldPos["P/F"]].toUpper()=="PASS";

        if(GetNotNAString(lFields[mFieldPos["SOFTBINDESCRIPTION"]]).isEmpty())
            lBinDesc = (lBinFlag?"PASS":"FAIL");
        else
            lBinDesc = lFields[mFieldPos["SOFTBINDESCRIPTION"]];

        lPRRRecord.SetSOFT_BIN(lSoftBin);
        lPRRRecord.SetHARD_BIN(lSoftBin);

        BYTE lPartFlg = 0;
        if (!lBinFlag)
            lPartFlg |= 0x08;
        else
            ++lGoodPartsPerWafer;
        lPRRRecord.SetPART_FLG(lPartFlg);
        if (mIsWaferSortFile)
        {
            lPRRRecord.SetX_COORD(lFields[mFieldPos["X"]].toInt());
            lPRRRecord.SetY_COORD(lFields[mFieldPos["Y"]].toInt());
        }
        lPRRRecord.SetPART_ID(lFields[mFieldPos["SERIAL"]]);

        lPRRRecord.Write(lStdfFile);

        // Fill software map
        if(mSoftBinning.contains(lSoftBin))
        {
            lBinning = mSoftBinning[lSoftBin];
            lBinning->SetBinCount((lBinning->GetBinCount()) + 1);
        }
        else
        {
            lBinning = new ParserBinning();

            lBinning->SetBinNumber(lSoftBin);
            lBinning->SetPassFail(lBinFlag);
            lBinning->SetBinName(lBinDesc);
            lBinning->SetBinCount(1);
            mSoftBinning.insert(lBinning->GetBinNumber(), lBinning);
        }
        lFields.clear();
    };


    if (mIsWaferSortFile)
    {
        // Write WCR
        lWCRRecord.Write(lStdfFile);

        // Write WRR
        lWRRRecord.SetHEAD_NUM(1);
        lWRRRecord.SetSITE_GRP(255);
        lWRRRecord.SetFINISH_T(lFinishTime);
        lWRRRecord.SetPART_CNT(lPartsPerWafer);
        lWRRRecord.SetGOOD_CNT(lGoodPartsPerWafer);
        lWRRRecord.Write(lStdfFile);
    }
    // Write HBR
    QMap<int,ParserBinning *>::Iterator it;
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    for(it = mSoftBinning.begin(); it != mSoftBinning.end(); it++)
    {
        lBinning = *it;
        lHBRRecord.SetHEAD_NUM(255);
        lHBRRecord.SetSITE_NUM(255);
        lHBRRecord.SetHBIN_NUM(lBinning->GetBinNumber());
        lHBRRecord.SetHBIN_CNT(lBinning->GetBinCount());
        if(lBinning->GetPassFail())
            lHBRRecord.SetHBIN_PF('P');
        else
            lHBRRecord.SetHBIN_PF('F');
        lHBRRecord.SetHBIN_NAM(lBinning->GetBinName().toLatin1().constData());
        lHBRRecord.Write(lStdfFile);
    }

    // Write SBR
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    for(it = mSoftBinning.begin(); it != mSoftBinning.end(); it++)
    {
        lBinning = *it;
        lSBRRecord.SetHEAD_NUM(255);
        lSBRRecord.SetSITE_NUM(255);
        lSBRRecord.SetSBIN_NUM(lBinning->GetBinNumber());
        lSBRRecord.SetSBIN_CNT(lBinning->GetBinCount());
        if(lBinning->GetPassFail())
            lSBRRecord.SetSBIN_PF('P');
        else
            lSBRRecord.SetSBIN_PF('F');
        lSBRRecord.SetSBIN_NAM(lBinning->GetBinName().toLatin1().constData());
        lSBRRecord.Write(lStdfFile);
    }

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    // use the last date in the csv file
    lMRRRecord.SetFINISH_T(lFinishTime);
    lMRRRecord.SetDISP_COD(' ');
    lMRRRecord.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

gstime_t InphiPrestotoSTDF::GetDate(const QString date, const QString time, const int daysToAdd)
{
    // Try to compute date
    QDate lDate = QDate::fromString(date, "M/d/yyyy");
    if(lDate.isNull() || !lDate.isValid())
        return 0;

    // Add days if required
    if(daysToAdd != 0)
        lDate = lDate.addDays(daysToAdd);

    // Try to compute time
    QTime lTime = QTime::fromString(time, "h:m:s");
    if(lTime.isNull() || !lTime.isValid())
        return 0;

    // Return time_t from computed date/time
    return QDateTime(lDate, lTime, Qt::UTC).toTime_t();
}

QString InphiPrestotoSTDF::GetNotNAString(const QString string)
{
    QString lNew = string.trimmed();
    if ((lNew.compare("NA", Qt::CaseInsensitive) == 0)
            || (lNew.compare("None", Qt::CaseInsensitive) == 0))
        lNew = "";
    return lNew;
}


    //////////////////////////////////////////////////////////////////////
// Skip empty line
        //////////////////////////////////////////////////////////////////////
void  InphiPrestotoSTDF::SpecificReadLine (QString &line)
{
    if(line.left(3) == ",,," && (line.simplified().count(",")==line.simplified().length()))
        line = "";
    }

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool InphiPrestotoSTDF::EmptyLine(const QString& line)
{
    bool lEmpty(true);
    if (!line.isEmpty())
    {
        QStringList lStrCells = line.split(",", QString::KeepEmptyParts);
        for(int lIndexCell=0; lIndexCell<lStrCells.count(); ++lIndexCell)
        {
            if (!lStrCells[lIndexCell].isEmpty())
            {
                lEmpty = false;
                break;
            }
        }
    }
    return lEmpty;
}
}
}
