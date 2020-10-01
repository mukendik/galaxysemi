//////////////////////////////////////////////////////////////////////
// import_pcm_dongbu.cpp: Convert a PCM_DONGBU (Process control Monitor).csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <QFileInfo>
#include <QRegExp>
#include <QProgressBar>
#include <QApplication>
#include <QLabel>

#include "importPcmDongbu.h"
#include "time.h"
//
//Lot Number:'6084380'(Customer Device:AS4003A  Program:KAV013CN  Upload Time:2007/03/11)
//WAFER_ID,SITE,Poly1-Metal1 Capacitance,Metal1-Metal2 Capacitance,Metal2-Metal3 Capacitance
//USL, ,+8.600E-02,+9.100E-02,+9.100E-02,+1.300E+00,+4.600E+00, ,+1.260E+02,+1.700E+00, ,+1.
//LSL, ,+3.900E-02,+3.400E-02,+3.400E-02,+1.000E+00,+3.800E+00,+3.600E+01, ,+9.000E-01,+3.90
//AK6084380-01 ,1 ,+5.720E-02 ,+4.722E-02 ,+6.323E-02 ,+1.125E+00 ,+4.058E+00 ,+4.000E+01 ,+
//AK6084380-01 ,2 ,+5.575E-02 ,+4.836E-02 ,+6.142E-02 ,+1.126E+00 ,+4.052E+00 ,+4.160E+01 ,+
//AK6084380-01 ,3 ,+6.154E-02 ,+4.853E-02 ,+6.491E-02 ,+1.127E+00 ,+4.056E+00 ,+4.040E+01 ,+


namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
PCM_DONGBUtoSTDF::PCM_DONGBUtoSTDF() : ParserBase(typePCMDongbu, "PCM Dongbu")
{
    mStartTime =0;
}


//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
PCM_DONGBUtoSTDF::~PCM_DONGBUtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_DONGBU format
//////////////////////////////////////////////////////////////////////
bool PCM_DONGBUtoSTDF::IsCompatible(const QString &fileName)
{
	QString strString;

	// Open hCsmFile file
    QFile f( fileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmDongbuFile(&f);
	QRegExp	qRegExp("[\\( \\)]+");

	// Check if first line is the correct PCM_DONGBU header...
	//Lot Number:'6084380'(Customer Device:AS4003A  Program:KAV013CN  Upload Time:2007/03/11)
	//WAFER_ID,SITE,Poly1-Metal1 Capacitance,Metal1-Metal2 Capacitance,Metal2-Metal3 Capacitance
	do
		strString = hPcmDongbuFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	f.close();
	
	strString = strString.trimmed();	// remove leading spaces.

	// extract Lot Number
	if(strString.startsWith("Lot Number", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
		return false;
	}
	strString = strString.section(":",1);
	strString = strString.section(qRegExp,1);

	// Product Name
	if(strString.startsWith("Customer Device", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
		return false;
	}
	strString = strString.section(":",1);
	strString = strString.section(qRegExp,1);

    // DBH Code: Optional
    if(strString.startsWith("DBH Code", Qt::CaseInsensitive) == true)
    {
        strString = strString.section(":",1);
        strString = strString.trimmed().section(qRegExp,1);
    }

	// Program Name
	if(strString.startsWith("Program", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
		return false;
	}
	strString = strString.section(":",1);
	strString = strString.section(qRegExp,1);

	// Upload Time
	if(strString.startsWith("Upload Time", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM_DONGBU file
//////////////////////////////////////////////////////////////////////
bool PCM_DONGBUtoSTDF::ReadPcmDongbuFile(const QString &PcmDongbuFileName, const QString &strFileNameSTDF)
{
	QString strString;
    QString lSection;
    bool	lStatus;
    int		lIndex;				// Loop index

	// Open CSV file
    QFile lFile( PcmDongbuFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PCM_DONGBU file
        mLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

	// Assign file I/O stream
    QTextStream hPcmDongbuFile(&lFile);
	QRegExp	qRegExp("[\\( \\)]+");

	// Check if first line is the correct PCM_DONGBU header...
	//Lot Number:'6084380'(Customer Device:AS4003A  Program:KAV013CN  Upload Time:2007/03/11)
	//WAFER_ID,SITE,Poly1-Metal1 Capacitance,Metal1-Metal2 Capacitance,Metal2-Metal3 Capacitance
	strString = ReadLine(hPcmDongbuFile);
	strString = strString.trimmed();	// remove leading spaces.

	// extract Lot Number
	if(strString.startsWith("Lot Number", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
	strString = strString.section(":",1);
    mLotID = strString.section(qRegExp,0,0).remove("'");
	strString = strString.section(qRegExp,1);

	// Product Name
	if(strString.startsWith("Customer Device", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
	strString = strString.section(":",1);
    mProductID = strString.section(qRegExp,0,0).remove("'");
	strString = strString.section(qRegExp,1);

    // DBH Code: Optional
    if(strString.startsWith("DBH Code", Qt::CaseInsensitive) == true)
    {
        strString = strString.section(":",1);
        mOperatorName = strString.trimmed().section(qRegExp,0,0).remove("'");
        strString = strString.trimmed().section(qRegExp,1);
    }


	// Program Name
	if(strString.startsWith("Program", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
	strString = strString.section(":",1);
    mProgramID = strString.section(qRegExp,0,0).remove("'");
	strString = strString.section(qRegExp,1);

	// Upload Time
	if(strString.startsWith("Upload Time", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
	strString = strString.section(":",1);
	QString strDate = strString.section(qRegExp,0,0).remove("'");
	strString = strString.section(qRegExp,1);

	// Date format: 2007/03/11
	QDate qDate = QDate::fromString(strDate,Qt::ISODate);
	QDateTime clDateTime(qDate);
	clDateTime.setTimeSpec(Qt::UTC);
    mStartTime = clDateTime.toTime_t();

	//WAFER_ID,SITE,Poly1-Metal1 Capacitance,Metal1-Metal2 Capacitance,Metal2-Metal3 Capacitance
	strString = ReadLine(hPcmDongbuFile);
	strString = strString.trimmed();	// remove leading spaces.
	if(strString.startsWith("WAFER_ID", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);

	// Count the number of parameters specified in the line
	// Do not count first 2 fields
    mTotalParameters=lstSections.count() - 3;
	// If no parameter specified...ignore!
    if(mTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
//    mParameter = new CGPcmDongbuParameter[m_iTotalParameters];	// List of parameters

    ParserParameter lParam;
	// Extract the N column names
    for(lIndex=0;lIndex<mTotalParameters;lIndex++)
	{
        lSection = lstSections[lIndex+2].trimmed();	// Remove spaces
        lParam.SetTestName(lSection);
//		UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
        long lTestNumber = mParameterDirectory.UpdateParameterIndexTable(lSection);
        lParam.SetTestNumber(lTestNumber);
        lParam.SetStaticHeaderWritten(false);
        mParameter.append(lParam);
	}


	int	iLimits =0;
	strString = ReadLine(hPcmDongbuFile);
	if(!strString.startsWith("USL"))
	{
		// Incorrect header...this is not a valid PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
	// found the HIGH limits
	iLimits |= 1;
    lstSections = strString.split(",",QString::KeepEmptyParts);
	// Check if have the good count
    if(lstSections.count() < mTotalParameters+2)
	{
        mLastError = errInvalidFormatLowInRows;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
			
	// Extract the N column Upper Limits
    for(lIndex=0;lIndex<mTotalParameters;lIndex++)
	{
        lSection = lstSections[lIndex+2].trimmed();
        mParameter[lIndex].SetHighLimit(lSection.toFloat(&lStatus));
        mParameter[lIndex].SetValidHighLimit(lStatus);
	}

	strString = ReadLine(hPcmDongbuFile);
	if(!strString.startsWith("LSL"))
	{
		// Incorrect header...this is not a valid PCM_DONGBU file!
        mLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}
	// found the Low limits
	iLimits |= 2;
    lstSections = strString.split(",", QString::KeepEmptyParts);
	// Check if have the good count
    if(lstSections.count() < mTotalParameters+2)
	{
        mLastError = errInvalidFormatLowInRows;
		
		// Convertion failed.
		// Close file
        lFile.close();
		return false;
	}

	// Extract the N column Lower Limits
    for(lIndex=0;lIndex<mTotalParameters;lIndex++)
	{
        lSection = lstSections[lIndex+2].trimmed();
        mParameter[lIndex].SetLowLimit(lSection.toFloat(&lStatus));
        mParameter[lIndex].SetValidLowLimit(lStatus);
	}

	if(iLimits != 3)
	{
		// Incorrect header...this is not a valid PCM_DONGBU file!: we didn't find the limits!
        mLastError = errNoLimitsFound;
		
		// Convertion failed.
        lStatus = false;
	}
	else
	{
		// Loop reading file until end is reached & generate STDF file dynamically.
        lStatus = WriteStdfFile(&hPcmDongbuFile,strFileNameSTDF);
        if(!lStatus)
			QFile::remove(strFileNameSTDF);
	}

	// Close file
    lFile.close();

	// All PCM_DONGBU file read...check if need to update the PCM_DONGBU Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

	// Success parsing PCM_DONGBU file
    return lStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM_DONGBU data parsed
//////////////////////////////////////////////////////////////////////
bool PCM_DONGBUtoSTDF::WriteStdfFile(QTextStream *pcmDongbuFile, const QString &fileNameSTDF)
{
    mParameterDirectory.SetFileName(GEX_PCM_DONGBU_PARAMETERS);

    if(mStdfParse.Open(fileNameSTDF.toStdString().c_str(), STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        return false;
    }

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

	// Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    GQTL_STDF::Stdf_WIR_V4 lWIRV4Record;
    GQTL_STDF::Stdf_WRR_V4 lWRRV4Record;
    GQTL_STDF::Stdf_PIR_V4 lPIRV4Record;
    GQTL_STDF::Stdf_PRR_V4 lPRRV4Record;

    lMIRV4Record.SetSETUP_T(mStartTime);			// Setup time
    lMIRV4Record.SetSTART_T(mStartTime);			// Start time
    lMIRV4Record.SetSTAT_NUM(1);					// Station
    lMIRV4Record.SetMODE_COD((BYTE) 'P');				// Test Mode = PRODUCTION
    lMIRV4Record.SetLOT_ID(mLotID.toLatin1().constData());		// Lot ID
    lMIRV4Record.SetPART_TYP(mProductID.toLatin1().constData());	// Part Type / Product ID
    lMIRV4Record.SetJOB_NAM(mProgramID.toLatin1().constData());	// Job name
    lMIRV4Record.SetOPER_NAM(mOperatorName.toLatin1().constData());		// operator
    lMIRV4Record.SetTEST_COD("WAFER");				// test-cod
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
	strUserTxt += ":PCM_DONGBU";
    lMIRV4Record.SetUSER_TXT(strUserTxt.toLatin1().constData());	// user-txt
    lMIRV4Record.SetFAMLY_ID(mProductID.toLatin1().constData());	// familyID
    mStdfParse.WriteRecord(&lMIRV4Record);

	// Write Test results for each line read.
    QString strString;
    char	szString[257];
	QString strSection;
	QString	strWaferID;
    float	fValue;				// Used for readng floating point numbers.
	int		iIndex;				// Loop index
    int		lSiteNumber;
    BYTE		bData;
    uint16_t	lSoftBin, lHardBin;
    long		lTotalGoodBin, lTotalFailBin;
    long		lTestNumber, lTotalTests,lPartNumber;
    bool		lStatus, lPassStatus;
	QStringList	lstSections;
	// Reset counters
    lTotalGoodBin=lTotalFailBin=0;
    lPartNumber=0;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
    while(pcmDongbuFile->atEnd() == false)
	{

		// Part number
        lPartNumber++;

		// Read line
        strString = ReadLine(*pcmDongbuFile);
        lstSections = strString.split(",", QString::KeepEmptyParts);
		// Check if have the good count
        if(lstSections.count() < mTotalParameters+2)
		{
            mLastError = errInvalidFormatLowInRows;
			
            mStdfParse.Close();
			// Convertion failed.
			return false;
		}
		
		// Extract WaferID
		strSection = lstSections[0].trimmed();
		if(strSection != strWaferID)
		{
			// Write WRR in case we have finished to write wafer records.
			if(strWaferID.isEmpty() == false)
			{
                // WRR
                lWRRV4Record.SetHEAD_NUM(1);						// Test head
                lWRRV4Record.SetSITE_GRP(255);					// Tester site (all)
                lWRRV4Record.SetFINISH_T(mStartTime);			// Time of last part tested
                lWRRV4Record.SetPART_CNT(lTotalGoodBin+lTotalFailBin);	// Parts tested: always 5
                lWRRV4Record.SetRTST_CNT(0);						// Parts retested
                lWRRV4Record.SetABRT_CNT(0);						// Parts Aborted
                lWRRV4Record.SetGOOD_CNT(lTotalGoodBin);			// Good Parts
                lWRRV4Record.SetFUNC_CNT(-1);				// Functionnal Parts
                lWRRV4Record.SetWAFER_ID(strWaferID.toLatin1().constData());	// WaferID
                mStdfParse.WriteRecord(&lWRRV4Record);
			}

            lTotalGoodBin=lTotalFailBin=0;
			// For each wafer, have to write limit in the first PTR
            for(iIndex=0;iIndex<mTotalParameters;iIndex++)
                mParameter[iIndex].SetStaticHeaderWritten(false);

			// Write WIR of new Wafer.
			strWaferID = strSection;

            lWIRV4Record.SetHEAD_NUM(1);								// Test head
            lWIRV4Record.SetSITE_GRP(255);							// Tester site (all)
            lWIRV4Record.SetSTART_T(mStartTime);					// Start time
            lWIRV4Record.SetWAFER_ID(strWaferID.toLatin1().constData());	// WaferID
            mStdfParse.WriteRecord(&lWIRV4Record);
		}

		// Reset Pass/Fail flag.
        lPassStatus = true;

		// Reset counters
        lTotalTests = 0;

		// Extract Site
        lSiteNumber = lstSections[1].trimmed().toInt();

		// Write PIR for parts in this Wafer site
        lPIRV4Record.SetHEAD_NUM(1);								// Test head
        lPIRV4Record.SetSITE_NUM(lSiteNumber);					// Tester site
        mStdfParse.WriteRecord(&lPIRV4Record);

        GQTL_STDF::Stdf_PTR_V4 lPTRRV4Record;
		// Read Parameter results for this record
        for(iIndex=0;iIndex<mTotalParameters;iIndex++)
		{
			strSection = lstSections[iIndex+2].trimmed();
            fValue = strSection.toFloat(&lStatus);
            if(lStatus == true)
			{
				// Valid test result...write the PTR
                lTotalTests++;

				// Compute Test# (add user-defined offset)
                lTestNumber = mParameter[iIndex].GetTestNumber();
                lTestNumber += GEX_TESTNBR_OFFSET_PCM_DONGBU;	// Test# offset
                lPTRRV4Record.SetTEST_NUM(lTestNumber);			// Test Number
                lPTRRV4Record.SetHEAD_NUM(1);						// Test head
                lPTRRV4Record.SetSITE_NUM(lSiteNumber);			// Tester site#
                if(((mParameter[iIndex].GetValidLowLimit() == true) && (fValue < mParameter[iIndex].GetLowLimit())) ||
                   ((mParameter[iIndex].GetValidHighLimit() == true) && (fValue > mParameter[iIndex].GetHighLimit())))
				{
					bData = 0200;	// Test Failed
                    lPassStatus = false;
				}
				else
				{
					bData = 0;		// Test passed
				}
                lPTRRV4Record.SetTEST_FLG(bData);							// TEST_FLG
                lPTRRV4Record.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80));						// PARAM_FLG
                lPTRRV4Record.SetRESULT(fValue);						// Test result
                if(mParameter[iIndex].GetStaticHeaderWritten() == false)
				{
                    lPTRRV4Record.SetTEST_TXT(mParameter[iIndex].GetTestName().toLatin1().constData());	// TEST_TXT
//                    lPTRRV4Record.Set("");							// ALARM_ID
					bData = 2;	// Valid data.
                    if(mParameter[iIndex].GetValidLowLimit() == false)
						bData |=0x40;
                    if(mParameter[iIndex].GetValidHighLimit() == false)
						bData |=0x80;
                    lPTRRV4Record.SetOPT_FLAG(bData);							// OPT_FLAG
                    lPTRRV4Record.SetRES_SCAL(0);								// RES_SCALE
                    lPTRRV4Record.SetLLM_SCAL(0);								// LLM_SCALE
                    lPTRRV4Record.SetHLM_SCAL(0);								// HLM_SCALE
                    lPTRRV4Record.SetLO_LIMIT(mParameter[iIndex].GetLowLimit());			// LOW Limit
                    lPTRRV4Record.SetHI_LIMIT(mParameter[iIndex].GetHighLimit());		// HIGH Limit
                    lPTRRV4Record.SetUNITS("");							// No Units
                    mParameter[iIndex].SetStaticHeaderWritten(true);
				}
                mStdfParse.WriteRecord(&lPTRRV4Record);
			}	// Valid test result
		}		// Read all results on line

		// Write PRR
        lPRRV4Record.SetHEAD_NUM(1);			// Test head
        lPRRV4Record.SetSITE_NUM(lSiteNumber);// Tester site#:1
        if(lPassStatus == true)
		{
            lPRRV4Record.SetPART_FLG(0);				// PART_FLG : PASSED
            lSoftBin = lHardBin = 1;
            lTotalGoodBin++;
		}
		else
		{
            lPRRV4Record.SetPART_FLG(8);				// PART_FLG : FAILED
            lSoftBin = lHardBin = 0;
            lTotalFailBin++;
		}
        lPRRV4Record.SetNUM_TEST(lTotalTests);	// NUM_TEST
        lPRRV4Record.SetHARD_BIN(lHardBin);           // HARD_BIN
        lPRRV4Record.SetSOFT_BIN(lSoftBin);           // SOFT_BIN
        int lX, lY;
        switch(lSiteNumber)
		{
				case 1:	// Left
                    lX = 1;			// X_COORD
                    lY = 4;			// Y_COORD
					break;
				case 2:	// Down
                    lX = 5;			// X_COORD
                    lY = 6;			// Y_COORD
					break;
				case 3:	// Center
                    lX = 5;			// X_COORD
                    lY = 3;			// Y_COORD
					break;
				case 4:	// Top
                    lX = 5;			// X_COORD
                    lY = 1;			// Y_COORD
					break;
				case 5:	// Right
                    lX = 8;			// X_COORD
                    lY = 4;			// Y_COORD
					break;
				case 6:	// Upper-Right corner
                    lX = 7;			// X_COORD
                    lY = 2;			// Y_COORD
					break;
				case 7:	// Upper-Left corner
                    lX = 3;			// X_COORD
                    lY = 2;			// Y_COORD
					break;
				case 8:	// Lower-Left corner
                    lX = 3;			// X_COORD
                    lY = 5;			// Y_COORD
					break;
				case 9:	// Lower-Right corner
                    lX = 7;			// X_COORD
                    lY = 5;			// Y_COORD
					break;
			default: // More than 5 sites?....give 0,0 coordonates
                lX = 0;			// X_COORD
                lY = 0;			// Y_COORD
				break;
		}
        lPRRV4Record.SetX_COORD(lX);
        lPRRV4Record.SetY_COORD(lY);
        lPRRV4Record.SetTEST_T(0);				// No testing time known...
        sprintf(szString,"%ld",lPartNumber);
        lPRRV4Record.SetPART_ID(szString);		// PART_ID
        mStdfParse.WriteRecord(&lPRRV4Record);

	};			// Read all lines with valid data records in file

	// Write WRR for last wafer inserted
    lWRRV4Record.SetHEAD_NUM(1);						// Test head
    lWRRV4Record.SetSITE_GRP(255);					// Tester site (all)
    lWRRV4Record.SetFINISH_T(mStartTime);			// Time of last part tested
    lWRRV4Record.SetPART_CNT(lTotalGoodBin+lTotalFailBin);	// Parts tested: always 5
    lWRRV4Record.SetRTST_CNT(0);						// Parts retested
    lWRRV4Record.SetABRT_CNT(0);						// Parts Aborted
    lWRRV4Record.SetGOOD_CNT(lTotalGoodBin);			// Good Parts
    lWRRV4Record.SetFUNC_CNT(-1);				// Functionnal Parts
    lWRRV4Record.SetWAFER_ID(strWaferID.toLatin1().constData());	// WaferID
    mStdfParse.WriteRecord(&lWRRV4Record);
	// Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRV4Record;
    lMRRV4Record.SetFINISH_T(mStartTime);			// File finish-time.
    mStdfParse.WriteRecord(&lMRRV4Record);

	// Close STDF file.
    mStdfParse.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PCM_DONGBU file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool PCM_DONGBUtoSTDF::ConvertoStdf(const QString &PcmDongbuFileName,  QString &strFileNameSTDF)
{
	// No erro (default)
    mLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(PcmDongbuFileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

    UpdateProgressMessage("Converting data from file "+QFileInfo(PcmDongbuFileName).fileName()+"...");

    if(ReadPcmDongbuFile(PcmDongbuFileName,strFileNameSTDF) != true)
	{
		return false;	// Error reading PCM_DONGBU file
	}
	
	// Convertion successful
	return true;
}

}
}
