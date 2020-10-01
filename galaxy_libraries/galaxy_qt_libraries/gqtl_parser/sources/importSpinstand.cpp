//////////////////////////////////////////////////////////////////////
// import_csv.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importSpinstand.h"
#include "importConstants.h"

#include "gqtl_log.h"
#include <math.h>

#include <qfileinfo.h>

//Device:,667LSX,Lot:,1721958DDAE,Temp:,25,Date:,06/01/17,Time:,00:15:14,Oper:,JBD,
//Program:,SpinTest,Rev:,2.10x4,Test:,P1667SX2.AR3,Gear:,60SG.GER,Ini:,GTSMAG1A.ini,NumTests:,23,
// Name, Cnt_Pin1, Cnt_Pin2, Cnt_Pin3, POK, Count3.1, IRBat, Icc4.0_IccOff, Icc4.0_IccOn,...,Time, Result,
// Spec_Min,   3.00,  -0.70, 800.00,   3.00, 999.99,  -5.00,   4.60,   4.60,  -4.00,  25... ,0, 0,
// Spec_Max,  12.00,  -0.30, 1800.00,   3.85, -999.99,   0.50,   9.70,   9.70,   4.00, 25...,9999, 0,
// Tolerance,   0.00,   0.00,   0.00,   0.00,   1.50,   0.00,   0.00,   0.00,   0.00,   0...,0, 0,
// Vcc,   4.00,   0.00,   4.00,   5.00,   4.00, -18.00,   4.00,   4.00,   4.00,   4.00,  ...,0, 0,
// VOut,   5.00,  -1.50,   5.00,   5.00,   5.00,   5.00,   5.00,   5.00,   5.00,   5.00, ...,0, 0,
// AirGap, 236  , 236  , 236  , 236  , 122  , 122  , 122  , 122  , 122  , 122  , 122  , 1...,0, 0,
// Velocity,  50.00,  50.00,  50.00,  50.00,  50.00,  50.00,  50.00,  50.00,  50.00,  50....,0, 0,
//1,    6.64 ,   -0.528,    1292 ,    3.34,  9999.9,    0.21 ,   6.6626,   6.6504,  0.190..., 1.190, 2
//2,    6.68 ,   -0.498,    1335 ,    3.32,  9999.9,    0.21 ,   6.6162,   6.6797,  0.339..., 1.199, FAIL
//---
//Test, Vcc, Vout, Airgap, Velocity, Pass, Fail, Yield, Mean, , , , Min, , , , Max, , , , StdDev, , , , MinLimit, MaxLimit, Tolerance
//Cnt_Pin1,     4,     5, 236  ,    50,   28268,       0, 100.00,  6.68, , , ,  6.45, , , ,  6.96, , , , 0, , , ,  3.00, 12.00,  0.00
// ...
//AOA_C0,     4,     0, 236  ,    50,   28188,       0, 100.00, 497.61, , , , 417.97, , , , 571.29, , , , 0, , , , 200.00, 800.00,  0.00
//Avg Test Time,   1.19
//Count Passed, 2, TSG9705, 28188, 99.7
//Count Passed, 4, None, 0,  0.0
//Count Passed, 8, , 0,  0.0
//Count Failed, 80,  0.3


namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGSpinstandtoSTDF::CGSpinstandtoSTDF() : ParserBase(typeSpinstand, "Spinstand")
{
    mParameters = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSpinstandtoSTDF::~CGSpinstandtoSTDF()
{
    // Destroy list of Parameters tables.
     delete [] mParameters;

    // Destroy list of Bin tables.
    qDeleteAll(mBinning.values());
    mBinning.clear();
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with ALLEGRO Spinstand format
//////////////////////////////////////////////////////////////////////
bool CGSpinstandtoSTDF::IsCompatible(const QString &FileName)
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
    //Device:,667LSX,Lot:,1721958DDAE,Temp:,25,Date:,06/01/17,Time:,00:15:14,Oper:,JBD,
    QString lStrString = lCsvFile.readLine();
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (lStrCells.count() >= 12
        && lStrCells[0].contains("Device", Qt::CaseInsensitive)
        && lStrCells[2].contains("Lot", Qt::CaseInsensitive)
        && lStrCells[4].contains("Temp", Qt::CaseInsensitive)
        && lStrCells[6].contains("Date", Qt::CaseInsensitive)
        && lStrCells[8].contains("Time", Qt::CaseInsensitive)
        && lStrCells[10].contains("Oper", Qt::CaseInsensitive))
        lIsCompatible = true;
    else
        lIsCompatible =false;

    if(lIsCompatible)
    {
        //Program:,SpinTest,Rev:,2.10x4,Test:,P1667SX2.AR3,Gear:,60SG.GER,Ini:,GTSMAG1A.ini,NumTests:,23,
        lStrString = lCsvFile.readLine();
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        if (lStrCells.count() >= 12
            && lStrCells[0].contains("Program", Qt::CaseInsensitive)
            && lStrCells[2].contains("Rev", Qt::CaseInsensitive)
            && lStrCells[4].contains("Test", Qt::CaseInsensitive)
            && lStrCells[6].contains("Gear", Qt::CaseInsensitive)
            && lStrCells[8].contains("Ini", Qt::CaseInsensitive)
            && lStrCells[10].contains("NumTests", Qt::CaseInsensitive))
            lIsCompatible = true;
        else
            lIsCompatible =false;
    }

    if(lIsCompatible)
    {
        // Name, Cnt_Pin1, Cnt_Pin2, Cnt_Pin3, POK, Count3.1, IRBat, Icc4.0_IccOff, Icc4.0_IccOn,...,Time, Result,
        lStrString = lCsvFile.readLine().trimmed();
        if (!lStrString.startsWith("Name"))
            lIsCompatible =false;
    }

    if(lIsCompatible)
    {
        // Spec_Min,   3.00,  -0.70, 800.00,   3.00, 999.99,  -5.00,   4.60,   4.60,  -4.00,  25... ,0, 0,
        lStrString = lCsvFile.readLine().trimmed();
        if (!lStrString.startsWith("Spec_Min"))
            lIsCompatible =false;
    }

    if(lIsCompatible)
    {
        // Spec_Max,  12.00,  -0.30, 1800.00,   3.85, -999.99,   0.50,   9.70,   9.70,   4.00, 25...,9999, 0,
        lStrString = lCsvFile.readLine().trimmed();
        if (!lStrString.startsWith("Spec_Max"))
            lIsCompatible =false;
    }

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
bool CGSpinstandtoSTDF::ConvertoStdf(const QString &CsvFileName,
                                    QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( CsvFileName).toLatin1().constData());

    QString lStrString;

    // Open CSV file
    QFile lFile( CsvFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    //mFileSize = lFile.size()+1;

    // Check the compatibility
    //Device:,667LSX,Lot:,1721958DDAE,Temp:,25,Date:,06/01/17,Time:,00:15:14,Oper:,JBD,
    lStrString = ReadLine(lCsvFile);
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (!(lStrCells.count() >= 12
        && lStrCells[0].contains("Device", Qt::CaseInsensitive)
        && lStrCells[2].contains("Lot", Qt::CaseInsensitive)
        && lStrCells[4].contains("Temp", Qt::CaseInsensitive)
        && lStrCells[6].contains("Date", Qt::CaseInsensitive)
        && lStrCells[8].contains("Time", Qt::CaseInsensitive)
        && lStrCells[10].contains("Oper", Qt::CaseInsensitive)))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    //Program:,SpinTest,Rev:,2.10x4,Test:,P1667SX2.AR3,Gear:,60SG.GER,Ini:,GTSMAG1A.ini,NumTests:,23,
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (!(lStrCells.count() >= 12
            && lStrCells[0].contains("Program", Qt::CaseInsensitive)
            && lStrCells[2].contains("Rev", Qt::CaseInsensitive)
            && lStrCells[4].contains("Test", Qt::CaseInsensitive)
            && lStrCells[6].contains("Gear", Qt::CaseInsensitive)
            && lStrCells[8].contains("Ini", Qt::CaseInsensitive)
            && lStrCells[10].contains("NumTests", Qt::CaseInsensitive)))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // Name, Cnt_Pin1, Cnt_Pin2, Cnt_Pin3, POK, Count3.1, IRBat, Icc4.0_IccOff, Icc4.0_IccOn,...,Time, Result,
    lStrString = ReadLine(lCsvFile).trimmed();
    // Remove last empty column
    while(lStrString.endsWith(","))
    {
        lStrString = lStrString.left(lStrString.length()-1);
    }
    // Then split
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (!(lStrCells[0].contains("Name", Qt::CaseInsensitive)
            && lStrCells[lStrCells.count()-1].contains("Result", Qt::CaseInsensitive)
            && lStrCells[lStrCells.count()-2].contains("Time", Qt::CaseInsensitive)))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    mBinCell = lStrCells.count() - 1;
    mTimeCell = lStrCells.count() - 2;
    mTotalParameters = lStrCells.count() - 2 - SPINSTAND_RAW_FIRST_DATA;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 Total Parameters").arg( mTotalParameters).toLatin1().constData());
    // Allocate the buffer to hold the N parameters & results.
    try
    {
        mParameters = new ParserParameter[mTotalParameters];	// List of parameters
    }
    catch(const std::bad_alloc&)
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

    long    lTestNumber;
    QString lTestName;
    float   lValue;
    bool    ok;

    mParameterDirectory.SetFileName(GEX_CSV_PARAMETERS);
    // Set test names
    for (int i = 0; i < mTotalParameters; ++i)
    {
        lTestName = lStrCells[SPINSTAND_RAW_FIRST_DATA + i].trimmed();
        lTestNumber = mParameterDirectory.UpdateParameterIndexTable(lTestName);
        mParameters[i].SetTestName(lTestName);
        mParameters[i].SetTestNumber(lTestNumber);
    }
    // Set Low Limits
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (!(lStrCells[0].contains("Spec_Min", Qt::CaseInsensitive)))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }
    for (int i = 0; i < mTotalParameters; ++i)
    {
        lValue = lStrCells[SPINSTAND_RAW_FIRST_DATA + i].trimmed().toFloat(&ok);
        mParameters[i].SetLowLimit(lValue);
        mParameters[i].SetValidLowLimit(ok);
    }
    // Set High Limits
    lStrString = ReadLine(lCsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (!(lStrCells[0].contains("Spec_Max", Qt::CaseInsensitive)))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }
    for (int i = 0; i < mTotalParameters; ++i)
    {
        lValue = lStrCells[SPINSTAND_RAW_FIRST_DATA + i].trimmed().toFloat(&ok);
        mParameters[i].SetHighLimit(lValue);
        mParameters[i].SetValidHighLimit(ok);
    }

    // Read Test Condition
    while(lCsvFile.atEnd() == false)
    {
        QString lTestCond;
        QString lCondValue;

        lStrString = ReadLine(lCsvFile);
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        lTestCond = lStrCells[0].trimmed();
        lValue = lTestCond.toFloat(&ok);
        // Read until the first PartId
        if(ok)
            break;
        // We want to store only Cond when the value change
        // For each column, we will have empty set if the TestConds just before are the same
        // , some new TestConds values if the value just before was updated
        int lCount = lStrCells.count();
        for (int i=0; i<lCount; ++i)
        {
            if((SPINSTAND_RAW_FIRST_DATA + i) > mTotalParameters)
                break;
            // Check if have a new value
            if(lCount > (SPINSTAND_RAW_FIRST_DATA + i)
               && lCondValue != lStrCells[SPINSTAND_RAW_FIRST_DATA + i].trimmed())
            {
                lCondValue = lStrCells[SPINSTAND_RAW_FIRST_DATA + i].trimmed();
                mTestConds[i][lTestCond] = lCondValue;
            }
        }
    }

    if(lCsvFile.atEnd() == true)
    {
        // Incorrect header...this is not a valid CSV file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lFile.close();
        return false;
    }

    lCsvFile.seek(0);
    if(WriteStdfFile(lCsvFile, StdfFileName) != true)
    {
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    // All CSV file read...check if need to update the CSV Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    mLastError = errNoError;
    // Success parsing CSV file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool CGSpinstandtoSTDF::WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    ParserBinning *lBinning;

    if(lStdfFile.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mLastError = errWriteSTDF;

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


    QStringList lStrCells;
    QString lStrString;

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    //Device:,667LSX,Lot:,1721958DDAE,Temp:,25,Date:,06/01/17,Time:,00:15:14,Oper:,JBD,
    lStrString = ReadLine(CsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    lMIRRecord.SetDSGN_REV(lStrCells[1].trimmed());
    lMIRRecord.SetLOT_ID(lStrCells[3].trimmed());
    lMIRRecord.SetTST_TEMP(lStrCells[5].trimmed());

    QStringList lDateList = lStrCells[7].trimmed().split('/');
    QDate lDate(1900, 1, 1);
    if (lDateList.count() == 3)
    {
        int lYear = lDateList[2].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;

        lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
    }
    QTime lTime = QTime::fromString(lStrCells[9].trimmed(), "hh:mm:ss");
    mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSETUP_T(mStartTime);
    lMIRRecord.SetOPER_NAM(lStrCells[11].trimmed());

    //Program:,SpinTest,Rev:,2.10x4,Test:,P1667SX2.AR3,Gear:,60SG.GER,Ini:,GTSMAG1A.ini,NumTests:,23,
    lStrString = ReadLine(CsvFile);
    lStrCells = lStrString.split(",",QString::KeepEmptyParts);

    lMIRRecord.SetEXEC_TYP(lStrCells[1].trimmed());
    lMIRRecord.SetEXEC_VER(lStrCells[3].trimmed());
    lMIRRecord.SetJOB_NAM(lStrCells[5].trimmed());

    QMap<QString, QString > lMetaData;
    lMetaData[lStrCells[6].remove(":").trimmed()] = lStrCells[7].trimmed();
    lMetaData[lStrCells[8].remove(":").trimmed()] = lStrCells[9].trimmed();

    // Add undefined values in the cvs and mandatory to the STDF
    lMIRRecord.SetSTAT_NUM(1);
    lMIRRecord.SetMODE_COD(' ');
    lMIRRecord.SetRTST_COD(' ');
    lMIRRecord.SetPROT_COD(' ');
    lMIRRecord.SetBURN_TIM(65535);
    lMIRRecord.SetCMOD_COD(' ');
    lMIRRecord.SetTSTR_TYP("");
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":CSV";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    lMIRRecord.Write(lStdfFile);

    bool ok = false;
    int  lPartId = 0;
    int  lNbParts = 0;
    int  lNbGoodParts = 0;
    int  lTotalDuration = 0; // in milliseconde
    // Go to the first Part Result
    //Program:,SpinTest,Rev:,2.10
    // ...
    //Velocity,  50.00,  50.00,  50.00,
    //1,    6.64 ,   -0.528,    1292 ,    3.34,  9999.9,
    while(CsvFile.atEnd() == false) // Read all lines in file
    {
        lStrString = ReadLine(CsvFile);
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        if(lStrCells.size()>1)
        {
            lPartId = lStrCells[0].trimmed().toInt(&ok);
            if(ok)
                break;
        }
    }


    // Write Test results for each line read.
    BYTE	lValidLimit;
    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    while(CsvFile.atEnd() == false) // Read all lines in file
    {
        if (lStrCells.size() == 0)
        {
            // Read line
            lStrString = ReadLine(CsvFile);
            // ignore empty lines
            if(lStrString.isEmpty())
                continue;
            // Split line
            lStrCells = lStrString.split(",",QString::KeepEmptyParts);
            // ignore empty lines
            if(lStrCells.isEmpty())
                continue;
        }
        //28268,    6.59 ,   -0.500
        //---
        //Test, Vcc, Vout, Airgap, Velocity,
        lPartId = lStrCells[0].trimmed().toInt(&ok);
        // End of the Result Section
        if(!ok)
            break;

        if (lStrCells.size() < static_cast<int>(SPINSTAND_RAW_FIRST_DATA + mTotalParameters))
        {
            lStdfFile.Close();
            GSLOG(SYSLOG_SEV_ERROR, QString("The number of parameter in this line is less than the expected one = %1").arg(lStrString).toLatin1().constData());
            return false;
        }

        ++lNbParts;

        // Write PIR
        GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
        lPIRRecord.SetHEAD_NUM(1);
        lPIRRecord.SetSITE_NUM(1);
        lPIRRecord.Write(lStdfFile);

        // Read Parameter results for this record
        for(int lIndex=0; lIndex<mTotalParameters; ++lIndex)
        {
            // Check if have TestCond to dump
            if(mTestConds.contains(lIndex))
            {
                foreach(const QString& lTC, mTestConds[lIndex].keys())
                {
                    QString lValue = QString("COND: %1 = %2").arg(lTC).arg(mTestConds[lIndex][lTC]);
                    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
                    lDTRRecord.SetTEXT_DAT(lValue);
                    lDTRRecord.Write(lStdfFile);
                }
            }

            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            lPTRRecord.SetTEST_NUM(mParameters[lIndex].GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            lPTRRecord.SetSITE_NUM(1);
            lPTRRecord.SetPARM_FLG((stdf_type_b1)(0x40|0x80)); // B*1 Parametric test flags (drift, etc.)
            bool ok;
            float lValue = lStrCells[SPINSTAND_RAW_FIRST_DATA + lIndex].toFloat(&ok);
            // Ignore invalid value
            if(lStrCells[SPINSTAND_RAW_FIRST_DATA + lIndex].trimmed() == "9999.9")
                ok=false;
            if (ok)
            {
                lValidLimit = 0x40;
                if(mParameters[lIndex].GetValidLowLimit()
                        || mParameters[lIndex].GetValidHighLimit())
                {
                    lValidLimit = 0x00;
                    if(mParameters[lIndex].GetValidLowLimit()
                            && (mParameters[lIndex].GetLowLimit() > lValue))
                    {
                        lValidLimit = 0x80;       // Result Fail
                    }
                    if(mParameters[lIndex].GetValidHighLimit()
                            && (mParameters[lIndex].GetHighLimit() < lValue))
                    {
                        lValidLimit = 0x80;       // Result Fail
                    }
                }

                mParameters[lIndex].IncrementExecTest();
                if(lValidLimit > 0x0)
                    mParameters[lIndex].IncrementFailTest();

                lPTRRecord.SetTEST_FLG(lValidLimit);       // Result
                lPTRRecord.SetRESULT(lValue);
            }
            else
            {
                lPTRRecord.SetTEST_FLG(0x42);       // No result, no Pass/fail indication
                lPTRRecord.SetRESULT(0);
            }
            lPTRRecord.SetTEST_TXT(mParameters[lIndex].GetTestName());

            if (mParameters[lIndex].GetStaticHeaderWritten() == false)
            {
                mParameters[lIndex].SetStaticHeaderWritten(true);
                lPTRRecord.SetRES_SCAL(0);
                lPTRRecord.SetLLM_SCAL(0);
                lPTRRecord.SetHLM_SCAL(0);
                lValidLimit = 0x0e;
                if(mParameters[lIndex].GetValidLowLimit()==false)
                    lValidLimit |=0x40;
                if(mParameters[lIndex].GetValidHighLimit()==false)
                    lValidLimit |=0x80;
                lPTRRecord.SetOPT_FLAG(lValidLimit);
                lPTRRecord.SetLO_LIMIT(mParameters[lIndex].GetLowLimit());
                lPTRRecord.SetHI_LIMIT(mParameters[lIndex].GetHighLimit());
                lPTRRecord.SetUNITS(mParameters[lIndex].GetTestUnits());
            }
            lPTRRecord.Write(lStdfFile);
        };

        // Write PRR
        GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
        // Elapsed Time in millisecond
        if (mTimeCell >= lStrCells.size())
        {
            lStdfFile.Close();
            GSLOG(SYSLOG_SEV_ERROR, QString("The number of parameter in this line is less than the expected one = %1")
                  .arg(lStrString).toLatin1().constData());
            mLastError = errInvalidFormatParameter;
            return false;
        }
        float lValueFloat = lStrCells[mTimeCell].toFloat();
        lValueFloat*= 1000.0;
        int lElapsedTime = static_cast<int>(round(lValueFloat));
        lTotalDuration += lElapsedTime;
        lPRRRecord.SetTEST_T(lElapsedTime);
        lPRRRecord.SetPART_ID(QString::number(lPartId));
        lPRRRecord.SetPART_TXT(QString::number(lPartId));
        lPRRRecord.SetSITE_NUM(1);
        int lBin = lStrCells[mBinCell].toUShort();
        if(lStrCells[mBinCell] == "FAIL")
            lBin = 0;

        lPRRRecord.SetSOFT_BIN(lBin);
        lPRRRecord.SetHARD_BIN(lBin);

        BYTE lPartFlg = 0;
        if (lBin == 0)
            lPartFlg |= 0x08;
        else
            ++lNbGoodParts;
        lPRRRecord.SetPART_FLG(lPartFlg);
        lPRRRecord.Write(lStdfFile);
        // Fill software map
        if(mBinning.contains(lBin))
        {
            lBinning = mBinning[lBin];
            lBinning->SetBinCount((lBinning->GetBinCount()) + 1);
        }
        else
        {
            lBinning = new ParserBinning();

            lBinning->SetBinNumber(lBin);
            lBinning->SetPassFail(lBin > 0);
            if(lBinning->GetPassFail())
                lBinning->SetBinName("PASS");
            else
                lBinning->SetBinName("FAIL");
            lBinning->SetBinCount(1);
            mBinning.insert(lBinning->GetBinNumber(), lBinning);
        }


        lStrCells.clear();
    };

    // Read Summary line to SpecName
    // SpecName can be save in the MIR
    // But because it is stored at the end of the file
    // Chose to use a MetaData to store it
    while(CsvFile.atEnd() == false) // Read all lines in file
    {
        lStrString = ReadLine(CsvFile);
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        if((lStrCells.size()>1)
                && lStrCells[0].contains("Count Passed",Qt::CaseInsensitive))
        {
            lMetaData["SpecName"] = lStrCells[2].trimmed();
            break;
        }
    }

    // Dump MetaData
    foreach(const QString& lKey, lMetaData.keys())
    {
        GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
        AddMetaDataToDTR(lKey,lMetaData[lKey],&lDTRRecord);
        lDTRRecord.Write(lStdfFile);
    }

    // Write HBR
    QMap<int,ParserBinning *>::Iterator it;
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    for(it = mBinning.begin(); it != mBinning.end(); it++)
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
    for(it = mBinning.begin(); it != mBinning.end(); it++)
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

    // Write TSR
    GQTL_STDF::Stdf_TSR_V4 lTSRRecord;
    lTSRRecord.SetHEAD_NUM(255);
    for (int i = 0; i < mTotalParameters; ++i)
    {
        lTSRRecord.SetHEAD_NUM(255);
        lTSRRecord.SetSITE_NUM(1);
        lTSRRecord.SetTEST_TYP('P');
        lTSRRecord.SetTEST_NUM(mParameters[i].GetTestNumber());
        lTSRRecord.SetEXEC_CNT(mParameters[i].GetExecCount());
        lTSRRecord.SetFAIL_CNT(mParameters[i].GetExecFail());
        lTSRRecord.SetTEST_NAM(mParameters[i].GetTestName());
        lTSRRecord.Write(lStdfFile);
    }

    // Write PCR
    GQTL_STDF::Stdf_PCR_V4 lPCRRecord;
    lPCRRecord.SetGOOD_CNT(lNbGoodParts);
    lPCRRecord.SetPART_CNT(lNbParts);
    lPCRRecord.Write(lStdfFile);

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    // use the last date in the csv file
    mStartTime += (int)(lTotalDuration/1000);
    lMRRRecord.SetFINISH_T(mStartTime);
    lMRRRecord.SetDISP_COD(' ');
    lMRRRecord.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Skip empty line
//////////////////////////////////////////////////////////////////////
void  CGSpinstandtoSTDF::SpecificReadLine (QString &strString)
{
    if(strString.left(3) == ",,," && (strString.simplified().count(",")==strString.simplified().length()))
        strString = "";
}

}
}
