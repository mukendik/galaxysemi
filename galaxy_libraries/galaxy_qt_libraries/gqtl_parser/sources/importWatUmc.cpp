
#include <QFile>
#include "importWatUmc.h"


namespace GS
{
namespace Parser
{

WatUmcToStdf::WatUmcToStdf():WatToStdf(typeWatUmc, "WatUmc")
{
    mParameterDirectory.SetFileName(GEX_WAT_UMC_PARAMETERS);
}

WatUmcToStdf::~WatUmcToStdf()
{
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT_UMC format
//////////////////////////////////////////////////////////////////////
bool WatUmcToStdf::IsCompatible(const QString& szFileName)
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
    QTextStream hWatUmcFile(&f);

    // Check if first line is the correct WAT_UMC header...
    //UMC_PRODUCT,CUSTOMER_PRODUCT,LOT,WAFER,PARAMETER,MEASURE_TIME,MAX_VALUE,MIN_VALUE,AVERAGE,STD_DEV,T_VALUE,B_VALUE,C_VALUE,L_VALUE,R_VALUE,MONTH,WEEK,PROCESS,SPEC_LOW,SPEC_TARGET,SPEC_HIGH,VALID_LOW,VALID_HIGH
    do
        strString = hWatUmcFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    f.close();

    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("UMC_PRODUCT,CUSTOMER_PRODUCT,LOT,WAFER,PARAMETER,MEASURE_TIME", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a WAT_UMC file!
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the WAT_UMC file
//////////////////////////////////////////////////////////////////////
bool WatUmcToStdf::ParseFile(const QString &fileName)
{
    QString		strString;
    QString		strSection;
    QString		strParameter;		//
    QString		strUnit;			//
    int			iWaferID;			// WaferID processed
    int			iIndex;				// Loop index
    QStringList	lstSections;

    // Open WAT_UMC file
    QFile f( fileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        mLastError = errOpenFail;
        return false;
    }
    // Assign file I/O stream
    QTextStream hWatUmcFile(&f);

    // Check if first line is the correct WAT_UMC header...
    //UMC_PRODUCT,CUSTOMER_PRODUCT,LOT,WAFER,PARAMETER,MEASURE_TIME,MAX_VALUE,MIN_VALUE,AVERAGE,STD_DEV,T_VALUE,B_VALUE,C_VALUE,L_VALUE,R_VALUE,MONTH,WEEK,PROCESS,SPEC_LOW,SPEC_TARGET,SPEC_HIGH,VALID_LOW,VALID_HIGH
    strString = ReadLine(hWatUmcFile);
    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("UMC_PRODUCT,CUSTOMER_PRODUCT,LOT,WAFER,PARAMETER,MEASURE_TIME", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a WAT_UMC file!
        mLastError = errInvalidFormatParameter;
        f.close();
        return false;
    }

    // Read WAT_UMC information
    //JBL0GA-A0,FUSION_REV_C,D2RPP.1,1,BVDN10/.18,07/11/2006 21:01,4.400000095,4.400000095,4.400000095,,4.400000095,4.400000095,4.400000095,4.400000095,4.400000095,11,45,VCOL1816A5,3.5,,5,,

    QString strDate;

    strString = ReadLine(hWatUmcFile);
    lstSections = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if(lstSections.count() <= 16)
    {
        mLastError = errInvalidFormatParameter;

        f.close();
        return false;
    }

    //UMC_PRODUCT
    //JBL0GA-A0
    mProductID = lstSections[0];
    //CUSTOMER_PRODUCT
    //FUSION_REV_C
    mJobName = lstSections[1];
    //LOT
    //D2RPP.1
    mLotID = lstSections[2];
    //MEASURE_TIME
    //07/11/2006 21:01
    strDate = lstSections[5];
    QTime		qTime(strDate.section(" ",1,1).section(":",0,0).toInt(),strDate.section(" ",1,1).section(":",1,1).toInt());
    QDate		qDate(strDate.section(" ",0,0).section("/",2,2).toInt(),strDate.section(" ",0,0).section("/",0,0).toInt(),strDate.section(" ",0,0).section("/",1,1).toInt());
    QDateTime clDateTime(qDate,qTime);
    clDateTime.setTimeSpec(Qt::UTC);
    mStartTime = clDateTime.toTime_t();

    //PROCESS
    //VCOL1816A5
    mProcessID= lstSections[16];

    // Read line with list of Parameters names
    iWaferID = -1;

    // Loop reading file until end is reached
    while(!hWatUmcFile.atEnd())
    {
        if(iWaferID > -1)
        {
            // Read next line in file...
            strString = ReadLine(hWatUmcFile);
            lstSections = strString.split(",",QString::KeepEmptyParts);
            // Check if have the good count
            if(lstSections.count() <= 3)
            {
                mLastError = errInvalidFormatParameter;
                f.close();
                return false;
            }
        }
        if(mLotID != lstSections[2])
        {
            // Found the end of the lot
            // Ignore the end of the file
            break;
        }

        // Extract WaferID, and save it to the parameter list as a parameter itself!
        iWaferID = lstSections[3].toInt();

        //JBL0GA-A0,FUSION_REV_C,D2RPP.1,1,BVDN10/.18,07/11/2006 21:01,4.400000095,4.400000095,4.400000095,,4.400000095,4.400000095,4.400000095,4.400000095,4.400000095,11,45,VCOL1816A5,3.5,,5,,
        // Read data

        strParameter = "";
        if(lstSections.count() > 4)
            strParameter = lstSections[4];
        strUnit =  "";

        // For each column, extract result value and save it
        for(iIndex=0;iIndex<5;iIndex++)
        {
            strSection = "";
            if(lstSections.count() > 10+iIndex)
                strSection = lstSections[10+iIndex];
                // Save parameter result in buffer
            if(!strSection.isEmpty())
                SaveParameterResult(iWaferID,iIndex+1,strParameter,strUnit,strSection);
        }

        if(lstSections.count() > 18)
            SaveParameterLimit(strParameter,lstSections[18],eLowLimit, strUnit);
        if(lstSections.count() > 20)
            SaveParameterLimit(strParameter,lstSections[20],eHighLimit, strUnit);

    }

    // Close file
    f.close();

    // All WAT_UMC file read...check if need to update the WAT_UMC Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Success parsing WAT_UMC file
    return true;
}

bool WatUmcToStdf::WriteStdfFile (const char *stdFileName)
{
    // now generate the STDF file...
    if(mStdfParse.Open((char*)stdFileName, STDF_WRITE) == false)
    {
        // Failed importing WAT_UMC file into STDF database
        mLastError = errWriteSTDF;
        return false;
    }

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    mTestCode= "WAFER";
    SetMIRRecord(lMIRV4Record, ":WAT_UMC");
    mStdfParse.WriteRecord(&lMIRV4Record);

    // Write Test results for each waferID
    char		szString[257];
    BYTE		iSiteNumber,bData;
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTestNumber,iPartNumber=0;
    bool		bPassStatus;

    WatParameter *                      ptParam		= 0;
    WatWafer *                          ptWafers	= 0;		// List of Wafers in WAT_UMC file
    QList<WatWafer*>::iterator          itWafer		= mWaferList.begin();
    QList<WatParameter*>::iterator      itParam;

    GQTL_STDF::Stdf_WIR_V4 lWIRV4Record;
    GQTL_STDF::Stdf_PIR_V4 lPIRV4Record;
    GQTL_STDF::Stdf_PTR_V4 lPTRV4Record;
    GQTL_STDF::Stdf_PRR_V4 lPRRV4Record;
    GQTL_STDF::Stdf_WRR_V4 lWRRV4Record;
    while(itWafer != mWaferList.end())
    {
        ptWafers = (*itWafer);

        // Write WIR
        SetWIRRecord(lWIRV4Record, ptWafers->mWaferID);
        mStdfParse.WriteRecord(&lWIRV4Record);
        lWIRV4Record.Reset();

        // Write all Parameters read on this wafer.: PTR....PTR, PRR
        iTotalGoodBin = iTotalFailBin = 0;

        // Write PTRs for EACH of the X sites
        for(iSiteNumber = ptWafers->mLowestSiteID; iSiteNumber <= ptWafers->mHighestSiteID; iSiteNumber++)
        {
            // before write PIR, PTRs, PRR
            // verify if we have some test executed from this site
            bPassStatus = false;

            itParam = ptWafers->mParameterList.begin();	// First test in list
            while(itParam != ptWafers->mParameterList.end())
            {
                if((*itParam)->mValue.contains(iSiteNumber) == true)
                {
                    bPassStatus = true;
                    break;
                }

                // Next Parameter!
                itParam++;
            };

            if(!bPassStatus)
                continue;

            // Write PIR for parts in this Wafer site
            lPIRV4Record.SetHEAD_NUM(1);								// Test head
            lPIRV4Record.SetSITE_NUM(iSiteNumber);					// Tester site
            mStdfParse.WriteRecord(&lPIRV4Record);
            lPIRV4Record.Reset();

            // Part number
            iPartNumber++;
            bPassStatus = true;

            itParam = ptWafers->mParameterList.begin();	// First test in list
            while(itParam != ptWafers->mParameterList.end())
            {
                ptParam = (*itParam);

                // Write the PTR if it exists for this site...
                if(ptParam->mValue.contains (iSiteNumber) == true)
                {
                    // Compute Test# (add user-defined offset)
                    iTestNumber = (long) mParameterDirectory.GetFullParametersList().indexOf(ptParam->GetTestName());
                    iTestNumber += GEX_TESTNBR_OFFSET_WAT_UMC;		// Test# offset
                    lPTRV4Record.SetTEST_NUM(iTestNumber);			// Test Number
                    lPTRV4Record.SetHEAD_NUM(1);						// Test head
                    lPTRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
                    if((ptParam->GetValidLowLimit() && (ptParam->mValue[iSiteNumber] < ptParam->GetLowLimit()))
                            || (ptParam->GetValidHighLimit() &&(ptParam->mValue[iSiteNumber] > ptParam->GetHighLimit())))
                    {
                        bData = 0200;	// Test Failed
                        bPassStatus = false;
                    }
                    else
                    {
                        bData = 0;		// Test passed
                    }
                    lPTRV4Record.SetTEST_FLG(bData);							// TEST_FLG
                    lPTRV4Record.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80));						// PARAM_FLG
                    lPTRV4Record.SetRESULT(ptParam->mValue[iSiteNumber]);	// Test result
                    if(ptParam->GetStaticHeaderWritten() == false)
                    {
                        // save Parameter name without unit information
                        lPTRV4Record.SetTEST_TXT(ptParam->GetTestName().toLatin1().constData());	// TEST_TXT
                        lPTRV4Record.SetALARM_ID("");							// ALARM_ID

                        bData = 2;	// Valid data.
                        if(ptParam->GetValidLowLimit()==false)
                            bData |=0x40;
                        if(ptParam->GetValidHighLimit()==false)
                            bData |=0x80;
                        lPTRV4Record.SetOPT_FLAG(bData);							// OPT_FLAG

                        int lScale = static_cast<WatScaledParameter*>(ptParam)->GetResultScale();
                        lPTRV4Record.SetRES_SCAL(-lScale);				// RES_SCALE
                        lPTRV4Record.SetLLM_SCAL(-lScale);				// LLM_SCALE
                        lPTRV4Record.SetHLM_SCAL(-lScale);				// HLM_SCALE
                        lPTRV4Record.SetLO_LIMIT(ptParam->GetValidLowLimit() ? ptParam->GetLowLimit() : 0);			// LOW Limit
                        lPTRV4Record.SetHI_LIMIT(ptParam->GetValidHighLimit() ? ptParam->GetHighLimit() : 0);		// HIGH Limit
                        lPTRV4Record.SetUNITS(ptParam->GetTestUnits().toLatin1().constData());	// Units
                        ptParam->SetStaticHeaderWritten(true);
                    }
                    mStdfParse.WriteRecord(&lPTRV4Record);
                    lPTRV4Record.Reset();
                }

                // Next Parameter!
                itParam++;
            };

            // Write PRR
            lPRRV4Record.SetHEAD_NUM(1);					// Test head
            lPRRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site:1,2,3,4 or 5
            if(bPassStatus == true)
            {
                lPRRV4Record.SetPART_FLG(0);					// PART_FLG : PASSED
                wSoftBin = wHardBin = 1;
                iTotalGoodBin++;
            }
            else
            {
                lPRRV4Record.SetPART_FLG(8);					// PART_FLG : FAILED
                wSoftBin = wHardBin = 0;
                iTotalFailBin++;
            }
            lPRRV4Record.SetNUM_TEST((WORD)ptWafers->mParameterList.count());		// NUM_TEST
            lPRRV4Record.SetHARD_BIN(wHardBin);            // HARD_BIN
            lPRRV4Record.SetSOFT_BIN(wSoftBin);            // SOFT_BIN
            // 200mm wafers, usually have 5 sites, 300mm wafers usually have 9 sites
            switch(iSiteNumber)
            {
            case 1:	// Top
                lPRRV4Record.SetX_COORD(1);			// X_COORD
                lPRRV4Record.SetY_COORD(0);			// Y_COORD
                break;
            case 2:	// Down
                lPRRV4Record.SetX_COORD(1);			// X_COORD
                lPRRV4Record.SetY_COORD(2);			// Y_COORD
                break;
            case 3:	// Center
                lPRRV4Record.SetX_COORD(1);			// X_COORD
                lPRRV4Record.SetY_COORD(1);			// Y_COORD
                break;
            case 4:	// Left
                lPRRV4Record.SetX_COORD(0);			// X_COORD
                lPRRV4Record.SetY_COORD(1);			// Y_COORD
                break;
            case 5:	// Right
                lPRRV4Record.SetX_COORD(2);			// X_COORD
                lPRRV4Record.SetY_COORD(1);			// Y_COORD
                break;
            default: // More than 5 sites?....give 0,0 coordonates
                lPRRV4Record.SetX_COORD(0);			// X_COORD
                lPRRV4Record.SetY_COORD(0);			// Y_COORD
                break;
            }
            lPRRV4Record.SetTEST_T(0);				// No testing time known...
            sprintf(szString,"%ld",iPartNumber);
            lPRRV4Record.SetPART_ID(szString);		// PART_ID
            lPRRV4Record.SetPART_TXT("");			// PART_TXT
            lPRRV4Record.SetPART_FIX();             // PART_FIX
            mStdfParse.WriteRecord(&lPRRV4Record);
            lPRRV4Record.Reset();
        }


        // Write WRR
        SetWRRRecord(lWRRV4Record, ptWafers->mWaferID, iTotalFailBin+iTotalGoodBin, iTotalGoodBin);
        mStdfParse.WriteRecord(&lWRRV4Record);
        lWRRV4Record.Reset();

#if O
        // NO SUMMARY FOR ETEST
        // Write SBR Bin0 (FAIL)
        WriteSBR(0,iTotalFailBin, false);

        // Write SBR Bin1 (PASS)
        WriteSBR(1,iTotalGoodBin, true);

        // Write HBR Bin0 (FAIL)
        WriteHBR(0, iTotalFailBin, false);

        // Write HBR Bin1 (PASS)
        WriteHBR(1,iTotalGoodBin, true);
#endif
        // Not the WaferID we need...see next one.
        itWafer++;
    };

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRV4Record;
    lMRRV4Record.SetFINISH_T(mStartTime);			// File finish-time.
    mStdfParse.WriteRecord(&lMRRV4Record);

    // Close STDF file.
    mStdfParse.Close();

    return true;
}

}
}
