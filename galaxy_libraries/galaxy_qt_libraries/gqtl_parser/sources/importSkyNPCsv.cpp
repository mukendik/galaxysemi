//////////////////////////////////////////////////////////////////////
// import_csv.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importSkyNPCsv.h"
#include "importConstants.h"

#include "gqtl_log.h"
#include <math.h>

#include <qfileinfo.h>

//#Test ID,Test Level,Mask,Lot,Operator,Wafer,Map ID,X,Y,DUT Number,Equipment,Temperature,Program_Rev,Limits_File,PC Number,Test_Date,Test_Time,1.1_VCC1_B13_POS,1.2_VCC1_B17_POS,1.3_VBIAS_POS,1.4_VBIAS_SW_POS,1.5_VEN13_POS,1.6_VEN17_POS,1.7_VEN28_POS,1.8_VHPM_ET_POS,1.9_VMPM_POS,1.10_VLPM_POS,1.11_RFIN_B13_POS,1.12_RFIN_B17_POS,1.13_RFIN_B28_POS,1.14_RFOUT_B13_POS,1.15_RFOUT_B17_POS,2.1_Y1_COM_NEG,2.2_RFOUT_B17_NEG,2.3_RFOUT_B13_NEG,2.4_RFIN_B28_NEG,2.5_RFIN_B17_NEG,2.6_RFIN_B13_NEG,2.7_VLPM_NEG,2.8_VMPM_NEG,2.9_VHPM_ET_NEG,2.10_VEN28_NEG,2.11_VEN17_NEG,2.12_VEN13_NEG,2.13_VBIAS_SW_NEG,2.14_VBIAS_NEG,2.15_VCC1_B17_NEG,2.16_VCC1_B13_NEG,3.1_IRSET,4.1_IBIAS_B13_H_WC,4.2_IBIAS_SW_B13_H_WC,4.3_IEN_B13_H_WC,4.4_IHPM_B13_WC,4.5_VIREF1_B13_H_WC,4.6_VY1_B13_H_WC,4.7_VIREF2_B13_H_WC,4.8_IBIAS_B17_H_WC,4.9_IBIAS_SW_B17_H_WC,4.10_IEN_B17_H_WC,4.11_IHPM_B17_WC,4.12_IBIAS_B28_H_WC,4.13_IBIAS_SW_B28_H_WC,4.14_IEN_B28_H_WC,4.15_IHPM_B28_WC,4.16_VIREF1_B28_H_WC,4.17_VY1_B28_H_WC,4.18_VIREF2_B28_H_WC,4.19_IMPM_B13_WC,4.20_IMPM_B17_WC,4.21_IMPM_B28_WC,4.22_IBIAS_B13_L_WC,4.23_IBIAS_SW_B13_L_WC,4.24_IEN_B13_L_WC,4.25_ILPM_B13_WC,4.26_VIREF1_B13_L_WC,4.27_VY1_B13_L_WC,4.28_VIREF2_B13_L_WC,4.29_IBIAS_B17_L_WC,4.30_IBIAS_SW_B17_L_WC,4.31_IEN_B17_L_WC,4.32_ILPM_B17_WC,4.33_IBIAS_B28_L_WC,4.34_IBIAS_SW_B28_L_WC,4.35_IEN_B28_L_WC,4.36_ILPM_B28_WC,4.37_VIREF1_B28_L_WC,4.38_VY1_B28_L_WC,4.39_VIREF2_B28_L_WC,5.1_2HT_B13_CAP,5.2_2HT_B17_CAP,5.3_3HT_B13_CAP,5.4_3HT_B17_CAP,6.1_VBIAS_LKG_WC,6.2_VBIAS_SW_LKG_WC,6.3_GATE_LKG_WC,7.1_RFOUT_B13_7VLKG,7.2_RFOUT_B17_7VLKG,7.3_VCC1_B13_LKG,7.4_VCC1_B17_LKG,7.5_VBIAS_LKG,7.6_VBIAS_SW_LKG,7.7_RFOUT_B13_LKG,7.8_RFOUT_B17_LKG,7.9_GATE_LKG,SW_BIN,BIN,Failed_Parameters,Prober_X,Prober_Y,Site
//#LoLimit,,,,,,,,,,,,,,,,,8.300,8.300,7.050,7.150,7.300,7.300,7.300,7.270,7.270,7.270,1.100,1.100,1.100,8.050,8.070,-2.100,-1.220,-1.220,-1.220,-1.220,-1.200,-1.270,-1.270,-1.270,-1.260,-1.260,-1.260,-1.230,-1.230,-1.230,-1.230,0.600,3.000,-0.400,-6.000,-5.000,0.700,1.100,1.000,2.000,-0.400,-4.000,-5.000,3.000,-0.400,-4.000,-5.000,0.800,1.100,0.800,-0.050,-0.050,-0.050,1.000,-0.300,5.000,5.000,0.080,1.100,0.800,2.000,-0.100,-5.000,5.000,1.500,-0.400,-5.000,5.000,0.700,1.100,0.800,-0.150,-0.150,-0.150,-0.150,-0.170,5.690,-0.130,10.200,10.200,-0.100,-0.100,-0.150,-0.200,-0.100,-0.100,-0.150,
//#HiLimit,,,,,,,,,,,,,,,,,8.500,8.500,7.250,7.350,7.450,7.450,7.450,7.450,7.450,7.450,1.230,1.230,1.230,8.250,8.250,-1.850,-1.150,-1.150,-1.150,-1.150,-1.160,-1.210,-1.210,-1.210,-1.220,-1.220,-1.220,-1.190,-1.190,-1.190,-1.190,0.850,10.000,0.500,132.000,15.000,2.400,1.400,3.000,10.000,0.400,5.000,15.000,10.000,0.400,6.000,15.000,2.400,1.400,5.000,0.070,0.070,0.070,10.000,0.500,132.000,62.000,1.800,1.400,1.500,6.000,0.300,5.000,62.000,5.500,0.500,5.000,62.000,2.200,1.400,1.500,10.000,7.000,7.000,7.000,1.000,200.090,1.000,28.000,28.000,0.250,0.250,1.000,140.000,0.250,0.300,1.400,
//#Units,,,,,,,,,,,,,,,,,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,V,mA,mA,mA,uA,uA,V,V,V,mA,mA,uA,uA,mA,mA,uA,uA,V,V,V,uA,uA,uA,mA,mA,uA,uA,V,V,V,mA,mA,uA,uA,mA,mA,uA,uA,V,V,V,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,uA,
// ,Wafer,ZH021_6,6258682,LR,1, ,-1,-3,1120,4914,25C,EGL_ZH021_6_X2_F,EGL_ZH021_6_X2_F.pds,11,10/19/2014,19:59:28,8.406,8.401,7.179,7.256,7.357,7.369,7.369,7.367,7.361,7.363,1.173,1.174,1.173,8.167,8.173,-1.954,-1.181,-1.181,-1.173,-1.173,-1.173,-1.231,-1.231,-1.231,-1.228,-1.231,-1.230,-1.214,-1.199,-1.204,-1.204,0.712,5.320,0.097,44.263,0.082,1.733,1.236,1.801,5.253,0.073,-0.119,-0.031,5.256,0.074,-0.097,-0.169,1.739,1.235,1.796,0.024,0.023,0.023,3.045,0.120,43.794,20.173,1.347,1.216,1.088,3.016,0.097,0.085,20.016,3.016,0.096,-0.035,19.233,1.348,1.215,1.086,1.649,0.266,0.125,0.262,0.106,83.416,0.143,22.064,21.349,0.059,0.064,0.108,83.261,0.120,0.122,0.095,1,1,,0,94,2
// ,Wafer,ZH021_6,6258682,LR,1, ,-1,-3,1121,4914,25C,EGL_ZH021_6_X2_F,EGL_ZH021_6_X2_F.pds,11,10/19/2014,19:59:29,8.404,8.400,7.177,7.257,7.356,7.367,7.368,7.366,7.360,7.362,1.173,1.174,1.173,8.167,8.171,-1.952,-1.181,-1.181,-1.174,-1.173,-1.173,-1.231,-1.231,-1.231,-1.228,-1.231,-1.230,-1.214,-1.198,-1.203,-1.203,0.711,5.328,0.095,43.168,-0.012,1.732,1.236,1.803,5.257,0.073,-0.135,-0.081,5.257,0.072,-0.019,-0.169,1.740,1.235,1.800,0.024,0.024,0.023,3.043,0.116,44.263,18.920,1.347,1.216,1.088,3.026,0.095,-0.135,19.076,3.027,0.096,-0.019,18.920,1.349,1.215,1.090,1.649,0.111,0.124,0.266,0.106,82.192,0.149,22.001,21.335,0.059,0.065,0.093,82.035,0.119,0.122,0.093,1,1,,1,94,2



namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGSkyNPCsvtoSTDF::CGSkyNPCsvtoSTDF() : ParserBase(typeSkyNPCSV, "SkyNPCSV")
{
    mParameters = NULL;
    mHasSiteNumber = false;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSkyNPCsvtoSTDF::~CGSkyNPCsvtoSTDF()
{
    // Destroy list of Parameters tables.
    if(mParameters!=NULL)
        delete [] mParameters;

    // Destroy list of Bin tables.
    qDeleteAll(mSoftBinning.values());
    mSoftBinning.clear();

    // Destroy list of Bin tables.
    qDeleteAll(mHardBinning.values());
    mHardBinning.clear();
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool CGSkyNPCsvtoSTDF::IsCompatible(const QString &FileName)
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
    QString lStrString = lCsvFile.readLine();
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (lStrCells.count() >= SKYNP_RAW_FIRST_DATA
        && lStrCells[0].contains("#Test ID", Qt::CaseInsensitive)
        && lStrCells[1].contains("Test Level", Qt::CaseInsensitive)
        && lStrCells[2].contains("Mask", Qt::CaseInsensitive)
        && lStrCells[3].contains("Lot", Qt::CaseInsensitive)
        && lStrCells[4].contains("Operator", Qt::CaseInsensitive))
        lIsCompatible = true;
    else
        lIsCompatible =false;

    lStrString = lCsvFile.readLine();
    if (!lStrString.startsWith("#LoLimit"))
        lIsCompatible =false;

    lStrString = lCsvFile.readLine();
    if (!lStrString.startsWith("#HiLimit"))
        lIsCompatible =false;

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
bool CGSkyNPCsvtoSTDF::ConvertoStdf(const QString &CsvFileName,
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
    lStrString = ReadLine(lCsvFile);
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (lStrCells.count() >= SKYNP_RAW_FIRST_DATA
        && lStrCells[0].contains("#Test ID", Qt::CaseInsensitive)
        && lStrCells[1].contains("Test Level", Qt::CaseInsensitive)
        && lStrCells[2].contains("Mask", Qt::CaseInsensitive)
        && lStrCells[3].contains("Lot", Qt::CaseInsensitive)
        && lStrCells[4].contains("Operator", Qt::CaseInsensitive))
    {
        mTotalParameters=0;
        mSwBinCell = 0;
        mTotalParameters = lStrCells.count() - SKYNP_RAW_FIRST_DATA ;
        for (int i=0; i<lStrCells.count(); ++i)
            if (lStrCells[i].compare("sw_bin", Qt::CaseInsensitive) == 0)
            {
                mSwBinCell = i;
                mTotalParameters = i - SKYNP_RAW_FIRST_DATA;
            }

        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 Total Parameters").arg( mTotalParameters).toLatin1().constData());
        // Allocate the buffer to hold the N parameters & results.
        try
        {
            mParameters = new ParserParameter[mTotalParameters];	// List of parameters
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

        QString lTestNumber;
        QString lTestName;
        // Set test names in the csv parameters
        for (unsigned i = 0; i < mTotalParameters; ++i)
        {
            lTestNumber = lStrCells[SKYNP_RAW_FIRST_DATA + i].section('_', 0, 0);
            lTestName = lStrCells[SKYNP_RAW_FIRST_DATA + i].section('_', 1);
            // to be sure that GEX don't crash
            if (lTestName.isEmpty())
            {
                mParameters[i].SetTestNumber(1);
                mParameters[i].SetTestName("");
                continue;
            }
            mParameters[i].
                SetTestNumber(lTestNumber.replace(".", "").toLong());
            mParameters[i].SetTestName(lTestName);
            // UpdateParameterIndexTable(mCGEglParameter[i].strName);
        }
    }
    else
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }
    if(lStrString.contains("site", Qt::CaseInsensitive))
        mHasSiteNumber = true;


    // read low limits
    lStrString = ReadLine(lCsvFile);
    if (lStrString.contains("LoLimit", Qt::CaseInsensitive))
    {
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        for(unsigned i=0; i<mTotalParameters; ++i)
        {
            bool ok;
            float lValue = lStrCells[SKYNP_RAW_FIRST_DATA + i].toFloat(&ok);
            if (ok)
            {
                mParameters[i].SetLowLimit(lValue);
                mParameters[i].SetValidLowLimit(true);
            }
            else
            {
                mParameters[i].SetLowLimit(0);
                mParameters[i].SetValidLowLimit(false);
            }
        }
        lStrString = ReadLine(lCsvFile);
    }

    // read high limits
    if (lStrString.contains("HiLimit", Qt::CaseInsensitive))
    {
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        for(unsigned i=0; i<mTotalParameters; ++i)
        {
            bool ok;
            float lValue = lStrCells[SKYNP_RAW_FIRST_DATA + i].toFloat(&ok);
            if (ok)
            {
                mParameters[i].SetHighLimit(lValue);
                mParameters[i].SetValidHighLimit(true);
            }
            else
            {
                mParameters[i].SetHighLimit(0);
                mParameters[i].SetValidHighLimit(false);
            }
        }
        lStrString = ReadLine(lCsvFile);
    }

    // read units
    if (lStrString.contains("Units", Qt::CaseInsensitive))
    {
        lStrCells = lStrString.split(",",QString::KeepEmptyParts);
        for(unsigned i=0; i<mTotalParameters; ++i)
        {
            mParameters[i].SetTestUnit(lStrCells[SKYNP_RAW_FIRST_DATA + i]);
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

    mLastError = errNoError;
    // Success parsing CSV file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool CGSkyNPCsvtoSTDF::WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
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


    QStringList lFields;
    QString lLine;
    do
    {
        lLine = ReadLine(CsvFile);
        lFields = lLine.split(",");
    }while (lFields[0].simplified() != "");
    // Skip empty lines
    if (lLine.simplified() == "")
    {
        do
        {
            lLine = ReadLine(CsvFile);
            lFields = lLine.split(",");
        }while (lLine.simplified() == "");
    }

    lLine.replace(";",",");
    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    lMIRRecord.SetPART_TYP(lFields[2]);
    lMIRRecord.SetLOT_ID(lFields[3]);
    lMIRRecord.SetOPER_NAM(lFields[4]);
    lMIRRecord.SetNODE_NAM(lFields[10]);
    lMIRRecord.SetTST_TEMP(lFields[11]);
    lMIRRecord.SetJOB_NAM(lFields[12]);
    lMIRRecord.SetAUX_FILE(lFields[13]);

    QStringList lDateList = lFields[15].split('/');
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
    QTime lTime = QTime::fromString(lFields[16], "hh:mm:ss");
    mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetSETUP_T(mStartTime);
    long lNumberDies = 0;

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

    // Write Test results for each line read.
    QString lLineString, lLastTime, lLastDate;
    BYTE	lValidLimit;
    int lPartsPerWafer = 0, lGoodPartsPerWafer = 0;
    QString lWaferID = "";
    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    QList<int> lWaferIds;
    time_t lFinishTime = -1;
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
        lLineString.replace(";",",");
        ++lNumberDies;
        if (lFields.size() < static_cast<int>(SKYNP_RAW_FIRST_DATA + mTotalParameters))
        {
            lStdfFile.Close();
            GSLOG(SYSLOG_SEV_ERROR, QString("The number of parameter in this line is less than the expected one = %1").arg(lLineString).toLatin1().constData());
            return false;
        }
        // Write the WIR and WRR
        GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
        GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
        if ((lFields[1].compare("wafer", Qt::CaseInsensitive) == 0)
            && (!lWaferIds.contains(lFields[5].toInt())))
        {
            // Write WRR to the privious WIR if we find a new wafer ID
            if (lWaferIds.size() != 0) // Not the first wafer id found in the input file
            {
                // Write WRR
                lWRRRecord.SetHEAD_NUM(1);
                lWRRRecord.SetSITE_GRP(255);
                lFinishTime = -1;
                QDate lFinishDate;
                lDateList = lLastDate.split("/");
                if (lDateList.count() == 3)
                {
                    int lYear = lDateList[2].toInt();
                    if (lYear < 70)
                        lYear += 2000;
                    else if (lYear < 99)
                        lYear += 1900;

                    lFinishDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
                }
                lTime = QTime::fromString(lLastTime, "hh:mm:ss");
                lFinishTime = QDateTime(lFinishDate, lTime,Qt::UTC).toTime_t();
                mStartTime = lFinishTime;
                lWRRRecord.SetFINISH_T(lFinishTime);
                lWRRRecord.SetPART_CNT(lPartsPerWafer);
                lWRRRecord.SetGOOD_CNT(lGoodPartsPerWafer);
                lWRRRecord.SetWAFER_ID(lWaferID);
                lWRRRecord.Write(lStdfFile);

                // Reset count param
                lPartsPerWafer = 0;
                lGoodPartsPerWafer = 0;
            }

            lWaferIds.append(lFields[5].toInt());
            // Write WIR
            lWIRRecord.SetHEAD_NUM(1);
            lWIRRecord.SetSITE_GRP(255);
            lWIRRecord.SetSTART_T(mStartTime);
            lWaferID = QString::number(lFields[5].toInt());
            lWIRRecord.SetWAFER_ID(lWaferID);
            lWIRRecord.Write(lStdfFile);
        }

        ++lPartsPerWafer;
        // Write PIR
        GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
        lPIRRecord.SetHEAD_NUM(1);
        if (mHasSiteNumber)
            lPIRRecord.SetSITE_NUM(lFields[mSwBinCell+5].toUShort());
        else
            lPIRRecord.SetSITE_NUM(1);
        lPIRRecord.Write(lStdfFile);

        // Read Parameter results for this record
        for(unsigned lIndex=0; lIndex<mTotalParameters; ++lIndex)
        {
            // If it's a PTR (pattern name empty)
            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            lPTRRecord.SetTEST_NUM(mParameters[lIndex].GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            if (mHasSiteNumber)
                lPTRRecord.SetSITE_NUM(lFields[mSwBinCell+5].toUShort());
            else
                lPTRRecord.SetSITE_NUM(1);

            // GCORE-8570 : HTH
            // Limit must not be strict. Measurament equal to the low or high limit must be considered as
            // PASS
            lPTRRecord.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80)); // B*1 Parametric test flags (drift, etc.)

            bool ok;
            float lValue = lFields[SKYNP_RAW_FIRST_DATA + lIndex].toFloat(&ok);
            if (ok)
            {
                lPTRRecord.SetTEST_FLG(0x40);       // No result
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
        lLastDate = lFields[15];
        lLastTime = lFields[16];
        lFinishTime = -1;
        QDate lFinishDate;
        lDateList = lLastDate.split("/");
        if (lDateList.count() == 3)
        {
            int lYear = lDateList[2].toInt();
            if (lYear < 70)
                lYear += 2000;
            else if (lYear < 99)
                lYear += 1900;

            lFinishDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
        }
        lTime = QTime::fromString(lLastTime, "hh:mm:ss");
        lFinishTime = QDateTime(lFinishDate, lTime,Qt::UTC).toTime_t();
        lPRRRecord.SetTEST_T(lFinishTime);
        lPRRRecord.SetPART_ID("");
        if (mHasSiteNumber)
            lPRRRecord.SetSITE_NUM(lFields[mSwBinCell+5].replace(";","").toUShort());
        else
            lPRRRecord.SetSITE_NUM(1);
        int lSoftBin = lFields[mSwBinCell].toUShort();
        int lHardBin = lFields[mSwBinCell+1].toUShort();
        lPRRRecord.SetSOFT_BIN(lSoftBin);
        lPRRRecord.SetHARD_BIN(lHardBin);

        BYTE lPartFlg = 0;
        if (lSoftBin!=1 || lHardBin!=1)
            lPartFlg |= 0x08;
        else
            ++lGoodPartsPerWafer;
        lPRRRecord.SetPART_FLG(lPartFlg);
        gsint16 lXCoord = lFields[mSwBinCell+3].toInt();
        gsint16 lYCoord = (lFields[mSwBinCell+4].replace(';',"")).toInt();
        lPRRRecord.SetX_COORD(lXCoord);
        lPRRRecord.SetY_COORD(lYCoord);
        lPRRRecord.Write(lStdfFile);

        if (WriteReticleDTR(lFields, lStdfFile, lXCoord, lYCoord) == false)
            return false;

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
            lBinning->SetPassFail(lSoftBin == 1);
            if(lBinning->GetPassFail())
                lBinning->SetBinName("PASS");
            else
                lBinning->SetBinName("FAIL");
            lBinning->SetBinCount(1);
            mSoftBinning.insert(lBinning->GetBinNumber(), lBinning);
        }

        if(mHardBinning.contains(lHardBin))
        {
            lBinning = mHardBinning[lHardBin];
            lBinning->SetBinCount((lBinning->GetBinCount()) + 1);
        }
        else
        {
            lBinning = new ParserBinning();

            lBinning->SetBinNumber(lHardBin);
            lBinning->SetPassFail(lHardBin == 1);
            if(lBinning->GetPassFail())
                lBinning->SetBinName("PASS");
            else
                lBinning->SetBinName("FAIL");
            lBinning->SetBinCount(1);
            mHardBinning.insert(lBinning->GetBinNumber(), lBinning);
        }


        lFields.clear();
    };


    // Write the last WRR
    GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
    lWRRRecord.SetHEAD_NUM(1);
    lWRRRecord.SetSITE_GRP(255);
    lWRRRecord.SetFINISH_T(lFinishTime);
    lWRRRecord.SetPART_CNT(lPartsPerWafer);
    lWRRRecord.SetGOOD_CNT(lGoodPartsPerWafer);
    lWRRRecord.SetWAFER_ID(lWaferID);
    lWRRRecord.Write(lStdfFile);

    // Write HBR
    QMap<int,ParserBinning *>::Iterator it;
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    for(it = mHardBinning.begin(); it != mHardBinning.end(); it++)
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

//////////////////////////////////////////////////////////////////////
// Skip empty line
//////////////////////////////////////////////////////////////////////
void  CGSkyNPCsvtoSTDF::SpecificReadLine (QString &strString)
{
    if(strString.left(3) == ",,," && (strString.simplified().count(",")==strString.simplified().length()))
        strString = "";
}

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool CGSkyNPCsvtoSTDF::EmptyLine(const QString& line)
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


// The Json syntaxe
//{
//"TYPE":"RETICLE",
//"DIEX":integer,      // Die X coordinate on the map
//"DIEY":integer,      // Die Y coordinate on the map
//"RETX":integer,     // X coordinate within the reticle
//"RETY": integer     // Y coordinate within the reticle
//}
bool CGSkyNPCsvtoSTDF::WriteReticleDTR(const QStringList& fields,
                                       GS::StdLib::Stdf& lStdfFile,
                                       const gsint16 xCoord,
                                       const gsint16 yCoord)
{
    if (fields.count() < SKYNP_RAW_FIRST_DATA)
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    gsbool lOk(false);

    // Extract Pos X and PosY of the reticle on the map from column X and Y
    gsint8 lPosX = fields.at(SKYNP_POSX_NUMBER_RAW_DATA).toInt(&lOk);
    if (!lOk)
    {
        mLastError = InvalidReticlePosX;

        return false;
    }

    gsint8 lPosY = fields.at(SKYNP_POSY_NUMBER_RAW_DATA).toInt(&lOk);
    if (!lOk)
    {
        mLastError = InvalidReticlePosY;
        return false;
    }

    // Extract X and Y coordinate inside the reticle from the DUT Number
    // DUT Number format is YYXX or YXX
    gsint8 lRetX(0), lRetY(0);
    gsint16 lDutNumber = fields.at(SKYNP_DUT_NUMBER_RAW_DATA).toInt(&lOk);
    if (lOk)
    {
        lRetY = lDutNumber / 100;
        lRetX = lDutNumber % 100;
    }
    else
    {
        mLastError = InvalidDUTNumber;
        return false;
    }

    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
    QJsonObject            lReticleField;
    lReticleField.insert("TYPE", QJsonValue(QString("reticle")));
    lReticleField.insert("DIEX", QJsonValue(xCoord));
    lReticleField.insert("DIEY", QJsonValue((yCoord)));
    lReticleField.insert("RETX", QJsonValue((lRetX)));
    lReticleField.insert("RETY", QJsonValue((lRetY)));
    lReticleField.insert("RETPOSX", QJsonValue((lPosX)));
    lReticleField.insert("RETPOSY", QJsonValue((lPosY)));
    lDTRRecord.SetTEXT_DAT(lReticleField);
    lDTRRecord.Write(lStdfFile);
    return true;
}

std::string CGSkyNPCsvtoSTDF::GetErrorMessage(const int ErrorCode) const
{
    QString lError = "Import : ";

    switch(ErrorCode)
    {
        case InvalidReticlePosX:
            lError += "Invalid Reticle Pos X detected.";
            break;

        case InvalidReticlePosY:
            lError += "Invalid Reticle Pos Y detected.";
            break;

        case InvalidDUTNumber:
            lError += "Invalid DUT number detected.";
            break;

        default:
            lError += QString::fromStdString(ParserBase::GetErrorMessage(ErrorCode));
            break;
    }

    return lError.toStdString();
}

}
}
