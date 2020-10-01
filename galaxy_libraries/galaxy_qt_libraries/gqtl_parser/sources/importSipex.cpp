/******************************************************************************!
 * \file importSipex.cpp
 ******************************************************************************/
#include <QTextStream>
#include <qfileinfo.h>
#include <math.h>

#include "parserParameter.h"
#include "parserWafer.h"
#include "importSipex.h"
#include "importConstants.h"
#include "gqtl_log.h"
#include "gqtl_global.h"
#include "gqtl_sysutils.h"

// macro defined in windows.h
#undef max
#undef min

#ifndef gex_min
# define gex_min(a, b) ((a) > (b) ? (b) : (a))
#endif
#ifndef gex_max
# define gex_max(a, b) ((a) < (b) ? (b) : (a))
#endif

// File format:
//LOT,DATE,WAFER,TESTNAME,SITE1,SITE2,SITE3,SITE4,SITE5,LIMITLO,LIMITHI,UNITS,PROCESS,TESTCHIP,REV,FC,CRITICAL,PART,ROUTE,WAFERSCRIBE,CUSTLOT
//4437A033  ,20090203,    1,16NPN30BETA7      ,  6.4811996e+001,  6.9055000e+001,  [...]   ,      19, 0,1,MS1548DZ          ,IP400     ,N38569    ,
//4437A033  ,20090203,    1,16NPN30BSO        ,  3.5005001e+001,  3.4765999e+001,  [...]   ,      19, 0,1,MS1548DZ          ,IP400     ,N38569    ,
//4437A033  ,20090203,    1,16NPN30CBO        ,  3.4700001e+001,  3.4200001e+001,  [...]   ,      19, 0,1,MS1548DZ          ,IP400     ,N38569    ,
//4437A033  ,20090203,    1,16NPN30CEO        ,  1.3186000e+001,  1.2936000e+001,  [...]   ,      19, 0,1,MS1548DZ          ,IP400     ,N38569    ,

namespace GS
{
namespace Parser
{

/******************************************************************************!
 * \fn SipextoSTDF
 * \brief Constructor
 ******************************************************************************/
SipextoSTDF::SipextoSTDF() : ParserBase(typeSipex, "Sipex")
{
    // Default: Sipex parameter list on disk includes all known Sipex parameters
    mStartTime = 0;
}

/******************************************************************************!
 * \fn ~SipextoSTDF
 * \brief Destructor
 ******************************************************************************/
SipextoSTDF::~SipextoSTDF()
{
    while (!mWaferList.isEmpty())
        delete mWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
ParserParameter *SipextoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
    ParserWafer	*		ptWafers;		// List of Wafers in PCM_HJTC file
    ParserParameter *	ptParam;		// List of parameters

    QList<ParserWafer*>::iterator itWaferBegin	= mWaferList.begin();
    QList<ParserWafer*>::iterator itWaferEnd	= mWaferList.end();

    while(itWaferBegin != itWaferEnd)
    {
        if((*itWaferBegin)->mWaferID == iWaferID)
        {
            // Update Min/Max SiteID
            if(iSiteID > 0)
            {
                (*itWaferBegin)->mLowestSiteID = std::min((*itWaferBegin)->mLowestSiteID,iSiteID);				// Lowest WaferID found in PCM_HJTC file
                (*itWaferBegin)->mHighestSiteID = std::max((*itWaferBegin)->mHighestSiteID,iSiteID);			// Highest WaferID found in PCM_HJTC file
            }

            // Found WaferID entry...now find Parameter entry
            QList<ParserParameter*>::iterator itParameterBegin	= (*itWaferBegin)->mParameterList.begin();
            QList<ParserParameter*>::iterator itParameterEnd	= (*itWaferBegin)->mParameterList.end();

            while(itParameterBegin != itParameterEnd)
            {
                if((*itParameterBegin)->GetTestName() == strParamName)
                    return (*itParameterBegin);	// Found the Parameter!

                // Not the Parameter we need...see next one.
                itParameterBegin++;
            };

            // Parameter not in list...create entry!
            // Create & add new Parameter entry in Wafer list
            ptParam = new ParserParameter;
            ptParam->SetTestName(strParamName);
            ptParam->SetTestUnit(strUnits);
            ptParam->SetHighLimit(0);
            ptParam->SetLowLimit(0);
            NormalizeLimits(ptParam);
            ptParam->SetValidLowLimit(false);		// Low limit defined
            ptParam->SetValidHighLimit(false);	// High limit defined

            ptParam->SetStaticHeaderWritten(false);
            (*itWaferBegin)->mParameterList.append(ptParam);

            // If Examinator doesn't have this PCM_HJTC parameter in his dictionnary, have it added.
            mParameterDirectory.UpdateParameterIndexTable(strParamName);

            // Return pointer to the Parameter
            return ptParam;
        }

        // Not the WaferID we need...see next one.
        itWaferBegin++;
    };

    // WaferID entry doesn't exist yet...so create it now!
    ptWafers = new ParserWafer;
    ptWafers->mLowestSiteID=ptWafers->mHighestSiteID=0;
    ptWafers->mWaferID = iWaferID;

    // Update Min/Max SiteID
    if(iSiteID > 0)
    {
        ptWafers->mLowestSiteID=iSiteID;				// Lowest WaferID found in PCM_HJTC file
        ptWafers->mHighestSiteID=iSiteID;				// Highest WaferID found in PCM_HJTC file
    }

    // Create & add new Parameter entry in Wafer list
    ptParam = new ParserParameter;
    ptParam->SetTestName(strParamName);
    ptParam->SetTestUnit(strUnits);
    ptParam->SetResultScale(0);
    ptParam->SetValidLowLimit(false);	// Low limit defined
    ptParam->SetValidHighLimit(false);	// High limit defined
    ptParam->SetStaticHeaderWritten(false);
    ptWafers->mParameterList.append(ptParam);

    // If Examinator doesn't have this PCM_HJTC parameter in his dictionnary, have it added.
    mParameterDirectory.UpdateParameterIndexTable(strParamName);

    // Add Wafer entry to existing list.
    mWaferList.append(ptWafers);

    // Return pointer to the Parameter
    return ptParam;
}

//////////////////////////////////////////////////////////////////////
// Save PCM_HJTC parameter result...
//////////////////////////////////////////////////////////////////////
void SipextoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
    if(strName.isNull() == true)
        return;	// Ignore empty entry!

    // Find Pointer to the Parameter cell
    ParserParameter *lParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);
    if(lParam == 0)
        return;

    // Save the Test value in it
    strValue.remove ('x');	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
    strValue = strValue.trimmed();
    lParam->SetTestValueForSite(iSiteID, strValue.toFloat() * GS_POW(10.0,lParam->GetResultScale()));
}

//////////////////////////////////////////////////////////////////////
// Save PCM_HJTC parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void SipextoSTDF::SaveParameterLimit(QString strName,QString strValue,int iLimit)
{
    if(strName.isNull() == true)
        return;	// Ignore empty entry!
    if(strValue.isNull() == true)
        return;	// Ignore empty entry!

    bool		bIsFloat;
    ParserParameter *lParam;	// List of parameters
    QList<ParserWafer*>::iterator itBegin	= mWaferList.begin();
    QList<ParserWafer*>::iterator itEnd		= mWaferList.end();

    while(itBegin != itEnd)
    {
        // Get Parameter pointer...save limits in ALL WaferEntry.
        lParam = FindParameterEntry((*itBegin)->mWaferID,0,strName);
        if(lParam == 0)
            return;
        switch(iLimit)
        {
            case eHighLimit:
                lParam->SetHighLimit(strValue.toFloat(&bIsFloat) * GS_POW(10.0,lParam->GetResultScale()));
                lParam->SetValidHighLimit(bIsFloat);		// High limit defined

                break;
            case eLowLimit:
                lParam->SetLowLimit(strValue.toFloat(&bIsFloat)  * GS_POW(10.0,lParam->GetResultScale()));
                lParam->SetValidLowLimit(bIsFloat);		// Low limit defined
                break;
        }

        // Move to next wafer entry
        itBegin++;
    };
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void SipextoSTDF::NormalizeLimits(ParserParameter *pParameter)
{
    int	value_scale=0;
    if(pParameter->GetTestUnits().length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        pParameter->SetResultScale(0);
        return;
    }

    QChar cPrefix = pParameter->GetTestUnits()[0];
    switch(cPrefix.toLatin1())
    {
        case 'm': // Milli
            value_scale = -3;
            break;
        case 'u': // Micro
            value_scale = -6;
            break;
        case 'n': // Nano
            value_scale = -9;
            break;
        case 'p': // Pico
            value_scale = -12;
            break;
        case 'f': // Fento
            value_scale = -15;
            break;
        case 'K': // Kilo
            value_scale = 3;
            break;
        case 'M': // Mega
            value_scale = 6;
            break;
        case 'G': // Giga
            value_scale = 9;
            break;
        case 'T': // Tera
            value_scale = 12;
            break;
    }
    pParameter->SetLowLimit(pParameter->GetLowLimit() * GS_POW(10.0,value_scale));
    pParameter->SetHighLimit(pParameter->GetHighLimit() * GS_POW(10.0,value_scale));
    pParameter->SetResultScale(value_scale);
    if(value_scale)
        pParameter->SetTestUnit(pParameter->GetTestUnits().mid(1));	// Take all characters after the prefix.
}

/******************************************************************************!
 * \fn IsCompatible
 ******************************************************************************/
bool SipextoSTDF::IsCompatible(const QString& aFileName)
{
    // Open
    QFile lFile(aFileName);
    if (!lFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    QTextStream lCsvFile(&lFile);

    // Read
    QString lString;
    do
    {
        lString = lCsvFile.readLine();
    }
    while (!lString.isNull() && lString.isEmpty());
    lString = lString.simplified().remove(" ");

    // Check
    if (!lString.startsWith(
            "LOT,DATE,WAFER,TESTNAME,SITE1,SITE2,SITE3,SITE4,SITE5,"
            "LIMITLO,LIMITHI,UNITS,PROCESS,TESTCHIP,REV,FC,CRITICAL,"
            "PART,ROUTE,WAFERSCRIBE,CUSTLOT", Qt::CaseInsensitive))
    {
        return false;
    }

    return true;
}

/******************************************************************************!
 * \fn ConvertoStdf
 ******************************************************************************/
bool SipextoSTDF::ConvertoStdf(const QString& aCsvFileName, QString &aStdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg(aCsvFileName).toLatin1().constData());

    // Open
    QFile lFile(aCsvFileName);
    if (!lFile.open(QIODevice::ReadOnly))
    {
        mLastError = errOpenFail;
        mLastErrorMessage = lFile.errorString();
        GSLOG(SYSLOG_SEV_ERROR, mLastErrorMessage.toLatin1().constData());
        return false;
    }
    QTextStream lCsvFile(&lFile);

    // Skip first line checked by IsCompatible()
    QString lString(this->ReadLine(lCsvFile).simplified().remove(" "));
    // Check
    if (!lString.startsWith(
            "LOT,DATE,WAFER,TESTNAME,SITE1,SITE2,SITE3,SITE4,SITE5,"
            "LIMITLO,LIMITHI,UNITS,PROCESS,TESTCHIP,REV,FC,CRITICAL,"
            "PART,ROUTE,WAFERSCRIBE,CUSTLOT", Qt::CaseInsensitive))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    mMetaData.clear();
    mCriticalTests.clear();
    mParameterDirectory.SetFileName(GEX_SIPEX_PARAMETERS);

    // Constant fields
    lString = this->ReadLine(lCsvFile).simplified().remove(" ");
    QStringList lFieldList = lString.split(',', QString::KeepEmptyParts);;
    lString = lFieldList[1];  // DATE
    QDate lDate(lString.left(4).toInt(), lString.mid(4, 2).toInt(), lString.right(2).toInt());  // YYYYMMDD
    QDateTime lDateTime(lDate);
    lDateTime.setTimeSpec(Qt::UTC);
    mStartTime = lDateTime.toTime_t();

    mProcessID = lFieldList[12];    // PROCESS
    mJobName = lFieldList[13];      // TESTCHIP
    mJobRev = lFieldList[14];       // REV
    mProductID = lFieldList[17];    // PART
    mFlowID = lFieldList[18];       // ROUTE
    mEngID = lFieldList[19];        // WAFERSCRIBE
    mLotID = lFieldList[20];        // CUSTLOT

    mMetaData["SubconLot"] = lFieldList[0];

    // Read
    int lWaferID;
    QString lSection;
    QString lParameter;
    QString lUnit;
    int lIndex;
    do
    {
        if (lFieldList.size() < 21)
        {
            mLastError = errInvalidFormatParameter;
            return false;
        }

        lWaferID = lFieldList[2].toInt();
        lParameter = lFieldList[3];
        lUnit = lFieldList[11];
        if ((lUnit.toUpper() == "NONE") ||
            (lUnit.toUpper() == "NA") ||
            (lUnit.toUpper() == "N/A"))
        {
            lUnit = "";
        }

        for (lIndex = 0; lIndex < 5; ++lIndex)
        {
            lSection = lFieldList[4 + lIndex];
            if (!lSection.isEmpty())
            {
                this->SaveParameterResult(lWaferID, lIndex + 1, lParameter, lUnit, lSection);
            }
        }

        this->SaveParameterLimit(lParameter, lFieldList[9], eLowLimit);
        this->SaveParameterLimit(lParameter, lFieldList[10], eHighLimit);

        // Check if the test is CRITICAL
        if(lFieldList[16].simplified() == "1" )
        {
            if(!mCriticalTests.contains(lParameter))
                mCriticalTests << lParameter;
        }

        // Check if last line processed (Should normally never happen as statistcs should follow)
        if (lCsvFile.atEnd())
        {
            break;
        }

        lString = this->ReadLine(lCsvFile).remove(" ");
        lFieldList = lString.split(",", QString::KeepEmptyParts);

    }
    while (!lString.isEmpty());

    // All PCM_HJTC file read...check if need to update the PCM_HJTC Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    return this->WriteStdfFile(lCsvFile, aStdfFileName);
}


/******************************************************************************!
 * \fn WriteStdfFile
 ******************************************************************************/
bool SipextoSTDF::WriteStdfFile(QTextStream& aStreamFile, const QString& aFileNameSTDF)
{
    (void) aStreamFile;
    GS::StdLib::Stdf lStdfFile;

    if (lStdfFile.Open(aFileNameSTDF.toUtf8().data(), STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        mLastError = errWriteSTDF;
        return false;
    }

    // Check StartTime
    if (mStartTime <= 0)
    {
        mStartTime = QDateTime::currentDateTime().toTime_t();
    }

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;
    lFARrecord.SetCPU_TYPE(1);
    lFARrecord.SetSTDF_VER(4);
    lFARrecord.Write(lStdfFile);

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRrecord;
    lMIRrecord.SetSETUP_T(mStartTime);
    lMIRrecord.SetSTART_T(mStartTime);
    lMIRrecord.SetSTAT_NUM(1);
    lMIRrecord.SetTEST_COD("P");
    lMIRrecord.SetRTST_COD(' ');
    lMIRrecord.SetPROT_COD(' ');
    lMIRrecord.SetBURN_TIM(65535);
    lMIRrecord.SetCMOD_COD(' ');
    lMIRrecord.SetLOT_ID(mLotID.toLatin1().constData());
    lMIRrecord.SetPART_TYP(mProductID.toLatin1().constData());
    lMIRrecord.SetFLOW_ID(mFlowID.toLatin1().constData());
    lMIRrecord.SetJOB_NAM(mJobName.toLatin1().constData());
    lMIRrecord.SetJOB_REV(mJobRev.toLatin1().constData());
    lMIRrecord.SetENG_ID(mEngID.toLatin1().constData());
    lMIRrecord.SetPROC_ID(mProcessID.toLatin1().constData());

    lMIRrecord.SetTEST_COD("WAFER");
    // Construct custom Galaxy USER_TXT
    QString lUserTxt;
    lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += ":";
    lUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    lUserTxt += ":SIPEX";
    lMIRrecord.SetUSER_TXT(lUserTxt.toLatin1().data());

    lMIRrecord.Write(lStdfFile);

    // Dump MetaData
    foreach(const QString& lKey, mMetaData.keys())
    {
        GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
        AddMetaDataToDTR(lKey,mMetaData[lKey],&lDTRRecord);
        lDTRRecord.Write(lStdfFile);
    }

    // Dump CriticalTests MetaData
    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
    QString lCtriticalTestLabel = "CriticalTests";
    QString lSeparator = ",";
    for (int lIter = 0; lIter < mCriticalTests.size(); ++lIter)
    {
        AddMetaDataToDTR(lCtriticalTestLabel, mCriticalTests.at(lIter) ,&lDTRRecord, true, lSeparator);
        lDTRRecord.Write(lStdfFile);
    }


    // Write Test results for each waferID
    ParserWafer* lWafers;  // List of Wafers in Sipex file
    BYTE lSite, lData;
    WORD lBin;
    long lNbPartGood, lNbPart = 0;;
    long lTesNumber, lNbTest = 0;
    bool lPassStatus;

    QListIterator<ParserWafer*> lIteratorWafer(mWaferList);
    lIteratorWafer.toFront();

    while (lIteratorWafer.hasNext())
    {
        lWafers = lIteratorWafer.next();

        // Write WIR
        GQTL_STDF::Stdf_WIR_V4 lWIRrecord;
        lWIRrecord.SetHEAD_NUM(1);
        lWIRrecord.SetSITE_GRP(255);
        lWIRrecord.SetSTART_T(mStartTime);
        lWIRrecord.SetWAFER_ID(QString::number(lWafers->mWaferID).toLatin1().data());
        lWIRrecord.Write(lStdfFile);

        // Write all Parameters read on this wafer.: PTR...PTR, PRR
        lNbPart = 0;
        lNbPartGood = 0;

        // Write PTRs for EACH of the X sites
        for (lSite = lWafers->mLowestSiteID; lSite <= lWafers->mHighestSiteID; lSite++)
        {
            QList<ParserParameter*>::iterator itParameterBegin	= lWafers->mParameterList.begin();
            QList<ParserParameter*>::iterator itParameterEnd	= lWafers->mParameterList.end();

            // before write PIR, PTRs, PRR, verify if we have some test executed from this site
            lPassStatus = false;
            while(itParameterBegin != itParameterEnd)
            {
                if((*itParameterBegin)->HasTestValueForSite(lSite) == true)
                {
                    lPassStatus = true;
                    break;
                }
                // Next Parameter!
                itParameterBegin++;
            }

            if (!lPassStatus)
            {
                continue;
            }

            // Part number
            ++lNbPart;
            lNbTest = 0;

            // Write PIR for parts in this Wafer site
            GQTL_STDF::Stdf_PIR_V4 lPIRrecord;
            lPIRrecord.SetHEAD_NUM(1);
            lPIRrecord.SetSITE_NUM(lSite);
            lPIRrecord.Write(lStdfFile);

            lPassStatus = true;
            ParserParameter* lParam;
            itParameterBegin	= lWafers->mParameterList.begin();
            itParameterEnd	= lWafers->mParameterList.end();

            // before write PIR, PTRs, PRR, verify if we have some test executed from this site
            lPassStatus = false;
            while(itParameterBegin != itParameterEnd)
            {
                lParam = (*itParameterBegin);

                // Write the PTR if it exists for this site
                if (! lParam->HasTestValueForSite(lSite))
                {
                    // Next Parameter!
                    itParameterBegin++;
                    continue;
                }

                ++lNbTest;

                GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
                // Compute Test# (add user-defined offset)
                lTesNumber = (long) mParameterDirectory.GetFullParametersList().indexOf(lParam->GetTestName());
                lTesNumber += GEX_TESTNBR_OFFSET_SIPEX;  // Test# offset
                lPTRRecord.SetTEST_NUM(lTesNumber);
                lPTRRecord.SetHEAD_NUM(1);
                // Tester site:1,2,3,4 or 5, etc.
                lPTRRecord.SetSITE_NUM(lSite);

                // Increment the number of execution
                lParam->IncrementExecTest();
                // Check if the test is FAIL
                if(lParam->GetValidLowLimit()
                        && (lParam->GetLowLimit() > lParam->GetTestValueForSite(lSite)))
                {
                    lData = 0200;  // Test Failed
                    lParam->IncrementFailTest();
                }
                else if(lParam->GetValidHighLimit()
                        && (lParam->GetHighLimit() < lParam->GetTestValueForSite(lSite)))
                {
                    lData = 0200;  // Test Failed
                    lParam->IncrementFailTest();
                }
                else
                {
                    lData = 0;  // Test passed
                }

                lPTRRecord.SetTEST_FLG(lData);
                lPTRRecord.SetPARM_FLG((stdf_type_b1) (0x40 | 0x80));
                lPTRRecord.SetRESULT(lParam->GetTestValueForSite(lSite));
                if (lParam->GetStaticHeaderWritten() == false)
                {
                    // Save Parameter name without unit information
                    lPTRRecord.SetTEST_TXT(lParam->GetTestName().toLatin1().constData());
                    // OPT_FLAG
                    lData = 2;  // Valid data
                    if (lParam->GetValidLowLimit()==false)
                    {
                        lData |= 0x40;
                    }
                    if (lParam->GetValidHighLimit()==false)
                    {
                        lData |= 0x80;
                    }
                    lPTRRecord.SetOPT_FLAG(lData);
                    lPTRRecord.SetRES_SCAL(-lParam->GetResultScale());
                    lPTRRecord.SetLLM_SCAL(-lParam->GetResultScale());
                    lPTRRecord.SetHLM_SCAL(-lParam->GetResultScale());
                    lPTRRecord.SetLO_LIMIT(lParam->GetValidLowLimit() ? lParam->GetLowLimit() : 0);
                    lPTRRecord.SetHI_LIMIT(lParam->GetValidHighLimit() ? lParam->GetHighLimit() : 0);
                    lPTRRecord.SetUNITS(lParam->GetTestUnits().toLatin1().constData());
                    lParam->SetStaticHeaderWritten(true);
                }
                lPTRRecord.Write(lStdfFile);

                // Next Parameter!
                itParameterBegin++;
            }

            // Write PRR
            GQTL_STDF::Stdf_PRR_V4 lPRRrecord;
            lPRRrecord.SetHEAD_NUM(1);
            lPRRrecord.SetSITE_NUM(lSite);
            if (lPassStatus == true)
            {
                lPRRrecord.SetPART_FLG(0);
                lBin = 1;
                ++lNbPartGood;
            }
            else
            {
                lPRRrecord.SetPART_FLG(8);
                lBin = 0;
            }
            lPRRrecord.SetNUM_TEST((WORD) lNbTest);
            lPRRrecord.SetHARD_BIN(lBin);
            lPRRrecord.SetSOFT_BIN(lBin);
            // 200mm wafers usually have 5 sites, 300mm wafers usually have 9 sites
            switch (lSite)
            {
            case 1:  // Center
                lPRRrecord.SetX_COORD(1);
                lPRRrecord.SetY_COORD(1);
                break;
            case 2:  // Left
                lPRRrecord.SetX_COORD(1);
                lPRRrecord.SetY_COORD(2);
                break;
            case 3:  // Right
                lPRRrecord.SetX_COORD(0);
                lPRRrecord.SetY_COORD(1);
                break;
            case 4:  // Lower-Left corner
                lPRRrecord.SetX_COORD(1);
                lPRRrecord.SetY_COORD(0);
                break;
            case 5:  // Upper-Right corner
                lPRRrecord.SetX_COORD(2);
                lPRRrecord.SetY_COORD(1);
                break;
            default:
                // More than 5 sites, give 0,0 coordonates
                lPRRrecord.SetX_COORD(0);
                lPRRrecord.SetY_COORD(0);
                break;
            }
            lPRRrecord.SetPART_ID(QString::number(lNbPart).toLatin1().data());
            lPRRrecord.Write(lStdfFile);
        }

        // Write WRR
        GQTL_STDF::Stdf_WRR_V4 lWRRrecord;
        lWRRrecord.SetHEAD_NUM(1);
        lWRRrecord.SetSITE_GRP(255);
        lWRRrecord.SetFINISH_T(mStartTime);
        // Parts tested: always 5
        lWRRrecord.SetPART_CNT(lNbPart);
        lWRRrecord.SetGOOD_CNT(lNbPartGood);
        lWRRrecord.SetWAFER_ID(QString::number(lWafers->mWaferID).toLatin1().data());
        lWRRrecord.Write(lStdfFile);
    }

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRrecord;
    lMRRrecord.SetFINISH_T(mStartTime);
    lMRRrecord.Write(lStdfFile);

    return true;
}

}
}
