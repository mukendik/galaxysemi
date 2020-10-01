#include <QFile>


#include "gqtl_global.h"
#include "importWatAsmc.h"


// MIR RECORD CORRESPONDENCE
// MIR.PART_TYP      MIR.PROC_ID       MIR.TEST_COD      MOR.LOT_ID   MIR.SETUP_T              MIR.TSTR_TYP
// File format:
//Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
// NAME1  | PGateBV@ N-EPIShe DN_SubsS BN_OxShe N+SDShee P+SDShee SNGLX_2u 250+Shee SNGLX_1. Cnt_2.8_ Br_2.8_M PTSIN+Sh 1kP-RESS
//     2  |     349n     etRh     heet     etRh     tRho     tRho     mP+S     tRho     6uP+     M2/M     2/M1     eetR     heet
//  UNIT  |    Volts    KOhms    Ohms/    Ohms/    Ohms/    Ohms/    Mirco    Ohms/    Mirco     Ohms    nAmps    Ohms/    KOhms
//PARCODE |   QB0010   QB0021   QB0030   QB0040   QB0070   QB0080   QB0090   QB0200   QB0210   QB0220   QB0230   QB0240   QB0290
// L.S.L. |   12.000    3.400   31.000   25.000   40.000  115.000    1.600  210.000    1.300   26.000    0.000    5.800    0.800
// U.S.L. |   25.000    5.100   51.000   41.000  160.000  335.000    2.400  290.000    1.800   49.000   10.000   11.800    1.200
//-------------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key------Key---
//WAF   9 |   19.390    4.270   40.000   35.200   70.100  268.800C   2.071  272.000    1.587   42.100    0.155    8.000    0.927
//        |   19.390    4.163   39.300   34.200   68.900  265.200C   2.040  266.000    1.576   40.500    0.152    8.200    0.903
//

using namespace GS::Parser;

WatAsmcToStdf::WatAsmcToStdf():WatToStdf(typeWatAsmc, "WatAsmc")
{
    mParameterDirectory.SetFileName(GEX_WAT_ASMC_PARAMETERS);
}

WatAsmcToStdf::~WatAsmcToStdf()
{
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT_ASMC format
//////////////////////////////////////////////////////////////////////
bool WatAsmcToStdf::IsCompatible(const QString &fileName)
{
    // Open hCsmFile file
    QFile f( fileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hWatAsmcFile(&f);

    //Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
    // Check if first line is the correct WAT_ASMC header...
    // Read WAT_ASMC information
    QString strString;
    do
        strString = hWatAsmcFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    f.close();

    if(!strString.section(":",0,0).simplified().toUpper().startsWith("TYPENAME", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a WAT_ASMC file!
        return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////
// Save WAT_ASMC parameter result...
//////////////////////////////////////////////////////////////////////
void WatAsmcToStdf::SaveParameterResult(int waferID, int siteID, QString name, QString strUnits, QString& value)
{
    if(name.isNull() == true)
        return;

    // Find Pointer to the Parameter cell
    WatScaledParameter *ptParam = static_cast<WatScaledParameter*>(FindParameterEntry(waferID, siteID, name, strUnits));

    // Save the Test value in it
    value                    = value.trimmed();
    float lValue             = value.toFloat() * GS_POW(10.0, ptParam->GetResultScale());
    ptParam->mValue[siteID]  = lValue;
}


WatParameter*   WatAsmcToStdf::CreateWatParameter(const QString& paramName, const QString& units)
{
    WatScaledParameter* ptParam       = new WatScaledParameter();
    ptParam->SetTestName (paramName);
    ptParam->SetTestUnit (units);
    NormalizeLimits(ptParam);
    ptParam->SetStaticHeaderWritten (false);

    return ptParam;
}



bool WatAsmcToStdf::ParseFile(const QString &fileName)
{

    QString strString;
    QString strSection;
    QString	strParameters[13];	// Holds the 13 Parameters name (WAT_ASMC file is organized by sets of 13 parameters columns)
    QString	strUnits[13];		// Holds the 13 Parameters Units
    QString	strLowLimits[13];	// Holds the 13 Parameters Low Limits
    QString	strHighLimits[13];	// Holds the 13 Parameters High Limits
    int		iWaferID=0;			// WaferID processed
    int		iSiteID=0;			// SiteID processed
    int		iIndex;				// Loop index

    // Open WAT_ASMC file
    QFile f( fileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        mLastError = errOpenFail;
        return false;
    }

    //Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
    // Check if first line is the correct WAT_ASMC header...
    // Read WAT_ASMC information
    QString strDate;
    QTextStream hWatAsmcFile(&f);
    strString = ReadLine(hWatAsmcFile);

    if(!strString.section(":",0,0).simplified().toUpper().startsWith("TYPENAME", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a WAT_ASMC file!
        mLastError = errInvalidFormatParameter;
        f.close();
        return false;
    }

    while(!strString.isEmpty())
    {
        strSection = strString.section(":",0,0).simplified().toUpper();
        strString = strString.section(":",1);

        //Typename:6M04471   Process:QUBIC1F   Stepcode:PQB512   Lot:6E2640   TestDate:   2006/09/07   Equip:7k07   Pg:  1
        if(strSection.startsWith("TYPENAME", Qt::CaseInsensitive))
        {
            mProductID  = strString.left(9).simplified();
            strString   = strString.mid(9);
        }
        else if(strSection.startsWith("PROCESS", Qt::CaseInsensitive))
        {
            mProcessID = strString.left(9).simplified();
            strString  = strString.mid(9);
        }
        else if(strSection.startsWith("STEPCODE", Qt::CaseInsensitive))
        {
            mTestCode = strString.left(8).simplified();
            strString = strString.mid(8);
        }
        else if(strSection.startsWith("LOT", Qt::CaseInsensitive))
        {
            mLotID    = strString.left(8).simplified();
            strString = strString.mid(8);
        }
        else if(strSection.startsWith("TESTDATE", Qt::CaseInsensitive))
        {
            strDate   = strString.left(15).simplified();
            strString = strString.mid(15);
        }
        else    if(strSection.startsWith("EQUIP", Qt::CaseInsensitive))
        {
            mTesterType = strString.left(6).simplified();
            strString   = strString.mid(6);
        }
        else if(strSection.startsWith("PG", Qt::CaseInsensitive))
        {
            // page number !
            strString = "";
        }
        else
        {
            // Incorrect header...this is not a WAT_ASMC file!
            mLastError = errInvalidFormatParameter;
            f.close();
            return false;
        }
    }

    if(!strDate.isEmpty())
    {
        QDate qDate(strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",2,2).toInt());
        QDateTime clDateTime(qDate);
        clDateTime.setTimeSpec(Qt::UTC);
        mStartTime = clDateTime.toTime_t();
    }

    // Read line with list of Parameters names

    // Loop reading file until end is reached
    while(!hWatAsmcFile.atEnd())
    {
        strString = ReadLine(hWatAsmcFile);
        if(strString.isEmpty() || strString.startsWith("Typename:", Qt::CaseInsensitive) || strString.startsWith("Typename:", Qt::CaseInsensitive))
            continue;

        if(strString.simplified().startsWith("NAME1", Qt::CaseInsensitive))
        {
            // NAME1  | PGateBV@ N-EPIShe DN_SubsS BN_OxShe N+SDShee P+SDShee SNGLX_2u 250+Shee SNGLX_1. Cnt_2.8_ Br_2.8_M PTSIN+Sh 1kP-RESS
            //     2  |     349n     etRh     heet     etRh     tRho     tRho     mP+S     tRho     6uP+     M2/M     2/M1     eetR     heet
            //  UNIT  |    Volts    KOhms    Ohms/    Ohms/    Ohms/    Ohms/    Mirco    Ohms/    Mirco     Ohms    nAmps    Ohms/    KOhms
            //PARCODE |   QB0010   QB0021   QB0030   QB0040   QB0070   QB0080   QB0090   QB0200   QB0210   QB0220   QB0230   QB0240   QB0290
            // L.S.L. |   12.000    3.400   31.000   25.000   40.000  115.000    1.600  210.000    1.300   26.000    0.000    5.800    0.800
            // U.S.L. |   25.000    5.100   51.000   41.000  160.000  335.000    2.400  290.000    1.800   49.000   10.000   11.800    1.200

            // Extract the 13 column names (or less if not all are filled)
            for(iIndex=0; iIndex<13; ++iIndex)
            {
                strSection              = strString.mid(9+iIndex*9,9).trimmed();	// Remove spaces
                strParameters[iIndex]   = strSection;
                strUnits[iIndex]        ="";
                strLowLimits[iIndex]    ="";
                strHighLimits[iIndex]   ="";
            }

            // Extract the 13 column Parameters second half of the name
            Extract(hWatAsmcFile, strParameters, true);

            // Extract the 13 column Units
            strString = ReadLine(hWatAsmcFile);
            for(iIndex=0; iIndex<13; ++iIndex)
            {
                strSection = strString.mid(9+iIndex*9,9).remove("/").remove("-").trimmed();	// Remove spaces
                if(!strSection.isEmpty())
                    strUnits[iIndex] = strSection;
            }

            // skip PARCODE
            strString = ReadLine(hWatAsmcFile);

            // Extract the 13 column Low Limits
            Extract(hWatAsmcFile, strLowLimits);

            // Extract the 13 column High Limits
            Extract(hWatAsmcFile, strHighLimits);

            // skip the line just after : -------------Key------Key------Key
            strString = ReadLine(hWatAsmcFile);
            continue;
        }

        if(strString.startsWith("WAF", Qt::CaseInsensitive))
        {
            // Extract WaferID, and save it to the parameter list as a parameter itself!
            iWaferID = strString.mid(3,4).toInt();
            iSiteID = 0;
        }

        iSiteID++;

        // Read line of Parameter data
        //WAF   9 |   19.390    4.270   40.000   35.200   70.100  268.800C   2.071  272.000    1.587   42.100    0.155    8.000    0.927
        //        |   19.390    4.163   39.300   34.200   68.900  265.200C   2.040  266.000    1.576   40.500    0.152    8.200    0.903

        // For each column, extract parameter value and save it
        for(iIndex=0; iIndex<13; ++iIndex)
        {
            strSection = strString.mid(9+iIndex*9,9).remove("C ");
            // Save parameter result in buffer
            if(iSiteID == 1)
            {
                // save limits
                SaveParameterLimit(strParameters[iIndex], strLowLimits[iIndex], eLowLimit, strUnits[iIndex]);
                SaveParameterLimit(strParameters[iIndex], strHighLimits[iIndex], eHighLimit, strUnits[iIndex]);
            }
            if(!strSection.isEmpty())
                SaveParameterResult(iWaferID, iSiteID, strParameters[iIndex], strUnits[iIndex], strSection);
        }
    };

    // Close file
    f.close();

    // All WAT_ASMC file read...check if need to update the WAT_ASMC Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Success parsing WAT_ASMC file
    return true;
}

void WatAsmcToStdf::Extract(QTextStream &hWatAsmcFile, QString* toFill, bool append)
{
    QString line = ReadLine(hWatAsmcFile);
    QString section;
    for(int index=0; index<13; ++index)
    {
        section = line.mid(9+index*9,9).trimmed();	// Remove spaces
        if(!section.isEmpty())
        {
            if(append)
                toFill[index].append(section);
            else
                toFill[index]   = section;
        }
    }
}

bool WatAsmcToStdf::WriteStdfFile(const char *stdFileName)
{
    // now generate the STDF file...

    if(mStdfParse.Open(stdFileName, STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        return false;
    }

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    mFamilyID = mProductID;
    SetMIRRecord(lMIRV4Record, ":WAT_ASMC");

    mStdfParse.WriteRecord(&lMIRV4Record);

    // Write Test results for each waferID
    char		szString[257];
    BYTE		iSiteNumber,bData;
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTestNumber,iPartNumber=0;
    float		fValue;
    bool		bPassStatus;

    WatParameter *                          ptParam		= 0;
    WatWafer *                              ptWafers	= 0;		// List of Wafers in WAT_ASMC file
    QList<WatWafer*>::iterator              itWafer		= mWaferList.begin();
    QList<WatParameter*>::iterator          itParam;

    while(itWafer != mWaferList.end())
    {
        ptWafers = (*itWafer);

        // Write WIR
        GQTL_STDF::Stdf_WIR_V4 lWIRV4Record;
        SetWIRRecord(lWIRV4Record,  ptWafers->mWaferID);
        mStdfParse.WriteRecord(&lWIRV4Record);
        lWIRV4Record.Reset();

        // Write all Parameters read on this wafer.: PTR....PTR, PRR
        iTotalGoodBin=iTotalFailBin=0;

        // Write PTRs for EACH of the X sites
        GQTL_STDF::Stdf_PIR_V4 lPIRV4Record;
        for(iSiteNumber = ptWafers->GetLowestSiteID(); iSiteNumber <= ptWafers->GetHighestSiteID(); ++iSiteNumber)
        {
            // before write PIR, PTRs, PRR
            // verify if we have some test executed from this site
            bPassStatus = false;

            itParam = ptWafers->mParameterList.begin();	// First test in list
            while(itParam  != ptWafers->mParameterList.end())
            {
                if((*itParam)->mValue.contains (iSiteNumber) == true)
                {
                    bPassStatus = true;
                    break;
                }

                ++itParam;
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

            GQTL_STDF::Stdf_PTR_V4 lPTRV4Record;
            itParam = ptWafers->mParameterList.begin();	// First test in list
            while(itParam != ptWafers->mParameterList.end())
            {
                ptParam = (*itParam);
                UpdateProgressBar();
                // Write the PTR if it exists for this site...
                if(ptParam->mValue.contains (iSiteNumber) == true)
                {
                    // Compute Test# (add user-defined offset)
                    iTestNumber = (long) mParameterDirectory.GetFullParametersList().indexOf(ptParam->GetTestName());
                    iTestNumber += GEX_TESTNBR_OFFSET_WAT_ASMC;		// Test# offset

                    lPTRV4Record.SetTEST_NUM(iTestNumber);			// Test Number`
                    lPTRV4Record.SetHEAD_NUM(1);				    // Test head
                    lPTRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
                    fValue = ptParam->mValue[iSiteNumber];
                    if((fValue < ptParam->GetLowLimit()) || (fValue > ptParam->GetHighLimit()))
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
                    lPTRV4Record.SetRESULT(fValue);	// Test result
                    if(ptParam->GetStaticHeaderWritten() == false)
                    {
                        // save Parameter name
                        lPTRV4Record.SetTEST_TXT(ptParam->GetTestName().toLatin1().constData());	// TEST_TXT
                        lPTRV4Record.SetALARM_ID("");							// ALARM_ID

                        bData = 2;	// Valid data.
                        if(ptParam->GetValidLowLimit()==false)
                            bData |=0x40;
                        if(ptParam->GetValidHighLimit()==false)
                            bData |=0x80;
                        lPTRV4Record.SetOPT_FLAG(bData);							// OPT_FLAG

                        lPTRV4Record.SetRES_SCAL(- static_cast<WatScaledParameter*>(ptParam)->GetResultScale());				// RES_SCALE
                        lPTRV4Record.SetLLM_SCAL(- static_cast<WatScaledParameter*>(ptParam)->GetResultScale());				// LLM_SCALE
                        lPTRV4Record.SetHLM_SCAL(- static_cast<WatScaledParameter*>(ptParam)->GetResultScale());				// HLM_SCALE
                        lPTRV4Record.SetLO_LIMIT(ptParam->GetValidLowLimit() ? ptParam->GetLowLimit() : 0);			// LOW Limit
                        lPTRV4Record.SetHI_LIMIT(ptParam->GetValidHighLimit() ? ptParam->GetHighLimit() : 0);			// HIGH Limit
                        lPTRV4Record.SetUNITS(ptParam->GetTestUnits());	// Units
                        ptParam->SetStaticHeaderWritten(true);
                    }
                    mStdfParse.WriteRecord(&lPTRV4Record);
                    lPTRV4Record.Reset();
                }

                // Next Parameter!
                itParam++;
            };

            // Write PRR
            GQTL_STDF::Stdf_PRR_V4 lPRRV4Record;
            lPRRV4Record.SetHEAD_NUM(1);					// Test head
            lPRRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site:1,2,3,4 or 5
            if(bPassStatus == true)
            {
                lPRRV4Record.SetPART_FLG(0);				// PART_FLG : PASSED
                wSoftBin = wHardBin = 1;
                ++iTotalGoodBin;
            }
            else
            {
                lPRRV4Record.SetPART_FLG(8);				// PART_FLG : FAILED
                wSoftBin = wHardBin = 0;
                ++iTotalFailBin;
            }

            lPRRV4Record.SetNUM_TEST((WORD)ptWafers->mParameterList.count());		// NUM_TEST
            lPRRV4Record.SetHARD_BIN(wHardBin);            // HARD_BIN
            lPRRV4Record.SetSOFT_BIN(wSoftBin);            // SOFT_BIN
            // wafers usually have 5 sites
            switch(iSiteNumber)
            {
                case 1:	// Center
                    lPRRV4Record.SetX_COORD(1);			// X_COORD
                    lPRRV4Record.SetY_COORD(1);			// Y_COORD
                    break;
                case 2:	// Left
                    lPRRV4Record.SetX_COORD(1);			// X_COORD
                    lPRRV4Record.SetY_COORD(2);			// Y_COORD
                    break;
                case 3:	// Right
                    lPRRV4Record.SetX_COORD(0);			// X_COORD
                    lPRRV4Record.SetY_COORD(1);			// Y_COORD
                    break;
                case 4:	// Lower-Left corner
                    lPRRV4Record.SetX_COORD(1);			// X_COORD
                    lPRRV4Record.SetY_COORD(0);			// Y_COORD
                    break;
                case 5:	// toUpper-Right corner
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
        GQTL_STDF::Stdf_WRR_V4 lWRRV4Record;
        SetWRRRecord(lWRRV4Record, ptWafers->mWaferID, iTotalGoodBin+iTotalFailBin , iTotalGoodBin) ;
        mStdfParse.WriteRecord(&lWRRV4Record);
        lWRRV4Record.Reset();

#if 0
        // NO SUMMARY FOR ETEST
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

    // Success
    return true;

}
