//////////////////////////////////////////////////////////////////////
// import_pcm_hjtc.cpp: Convert a Pcm Hejian Technology file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QTextStream>
#include <QListIterator>

#include "import_pcm_hjtc.h"
#include "importConstants.h"
#include "parserParameter.h"
#include "parserWafer.h"

// File format:
//TYPE NO:EEMB4A2A,PCM SPEC:H10487_P4.1     ,LOT ID:N50T7,DATE:2010-03-13 23:21:20

/*GCORE-12059
//PART NO:9179-UMC AFA
//PROCESS NAME:NP18SQWX
*/

//ITEM:54,TOTAL:20 PCS,PASS:20 PCS,TRANSFER:20 PCS
//LOT,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,...,<MAX>,<MIN>,<AVG>,<STD>,<SPEC HIGH>,<SPEC LOW>
//WF#,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,9,9,9,...22,23,23,23,23,23,24,24,24,24,24,25,25,25,25,25
//S#,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,...B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L
//VTON10/.35,0.596   ,0.596   ,0.582   ,0.589... ,0.578   ,0.591   ,0.00429 ,0.71    ,0.51
//IDSN10/.35,0.00577 ,0.00585 ,0.00595 ,0.005... ,0.00576 ,0.00588 ,4.89E-05,0.00685 ,0.00515


namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmHjtctoSTDF::CGPcmHjtctoSTDF(): ParserBase(typePcmHjtc, "typePcmHjtc")
{
    mStartTime = 0;
}

CGPcmHjtctoSTDF::~CGPcmHjtctoSTDF()
{
    while (!mWaferList.isEmpty())
        delete mWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
ParserParameter *CGPcmHjtctoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
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

            ptParam->SetStaticHeaderWritten(false);
            (*itWaferBegin)->mParameterList.append(ptParam);
            mTotalParameters++;
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
    mTotalParameters++;

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
void CGPcmHjtctoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
    if(strName.isNull() == true)
        return;	// Ignore empty entry!

    // Find Pointer to the Parameter cell
    ParserParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);

    // Save the Test value in it
    strValue.remove ('x');	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
    strValue = strValue.trimmed();
    ptParam->SetTestValueForSite(iSiteID, strValue.toFloat() * GS_POW(10.0,ptParam->GetResultScale()));

    // Increment the number of execution
    ptParam->IncrementExecTest();
    // Check if the test is FAIL
    if(ptParam->GetValidLowLimit() && (ptParam->GetLowLimit() > strValue.toDouble()))
    {
        ptParam->IncrementFailTest();
    }
    else if(ptParam->GetValidHighLimit() && (ptParam->GetHighLimit() < strValue.toDouble()))
    {
        ptParam->IncrementFailTest();
    }
}

//////////////////////////////////////////////////////////////////////
// Save PCM_HJTC parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGPcmHjtctoSTDF::SaveParameterLimit(QString strName, QString strValue, int iLimit)
{
    if((strName.isNull() == true) || (strName.isEmpty() == true))
        return;	// Ignore empty entry!
    if((strValue.isNull() == true) || (strValue.isEmpty() == true))
        return;	// Ignore empty entry!

    bool		bIsFloat;
    ParserParameter *ptParam;	// List of parameters
    QList<ParserWafer*>::iterator itBegin	= mWaferList.begin();
    QList<ParserWafer*>::iterator itEnd		= mWaferList.end();

    while(itBegin != itEnd)
    {
        // Get Parameter pointer...save limits in ALL WaferEntry.
        ptParam = FindParameterEntry((*itBegin)->mWaferID, 0, strName);
        switch(iLimit)
        {
            case eHighLimit:
                ptParam->SetHighLimit(strValue.toFloat(&bIsFloat) * GS_POW(10.0,ptParam->GetResultScale()));
                ptParam->SetValidHighLimit(bIsFloat);		// High limit defined

                break;
            case eLowLimit:
                ptParam->SetLowLimit(strValue.toFloat(&bIsFloat)  * GS_POW(10.0,ptParam->GetResultScale()));
                ptParam->SetValidLowLimit(bIsFloat);		// Low limit defined
                break;
        }

        // Move to next wafer entry
        itBegin++;
    };
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_HJTC format
//////////////////////////////////////////////////////////////////////
bool CGPcmHjtctoSTDF::IsCompatible(const QString &szFileName)
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
    QTextStream hPcmHjtcFile(&f);

    // Check if first line is the correct PCM_HJTC header...
    //TYPE NO:EEMB4A2A,PCM SPEC:H10487_P4.1     ,LOT ID:N50T7,DATE:2010-03-13 23:21:20
    do
        strString = hPcmHjtcFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    strString = strString.trimmed();	// remove leading spaces.

    if(!strString.startsWith("TYPE NO:", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        // Close file
        f.close();
        return false;
    }

    if(!strString.contains("PCM SPEC:", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        // Close file
        f.close();
        return false;
    }

    // Check if found the header parameters line
    //LOT,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,...,<MAX>,<MIN>,<AVG>,<STD>,<SPEC HIGH>,<SPEC LOW>
    //WF#,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,9,9,9,...22,23,23,23,23,23,24,24,24,24,24,25,25,25,25,25
    //S#,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,...B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L
    int nLine = 10;
    while(!strString.startsWith("LOT,",Qt::CaseInsensitive))
    {
        if((nLine <= 0) || hPcmHjtcFile.atEnd())
            break;
        strString = hPcmHjtcFile.readLine().simplified().remove(" ");
        nLine--;
    }

    if(!strString.startsWith("LOT,",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM file!
        // Close file
        f.close();
        return false;
    }
    strString = hPcmHjtcFile.readLine().simplified().remove(" ");
    if(!strString.startsWith("WF#,",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM file!
        // Close file
        f.close();
        return false;
    }
    strString = hPcmHjtcFile.readLine().simplified().remove(" ");
    if(!strString.startsWith("S#",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM file!
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM_HJTC file
//////////////////////////////////////////////////////////////////////
bool CGPcmHjtctoSTDF::ReadPcmHjtcFile(const QString &PcmHjtcFileName)
{
    QString		strString;
    QString		lSection;
    QString		lParameter;		//
    QString		lUnit;			//
    int			iIndex;				// Loop index
    QStringList	lstSections;

    // Open PCM_HJTC file
    QFile f( PcmHjtcFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening PCM_HJTC file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hPcmHjtcFile(&f);

    UpdateProgressBar();

    // Check if first line is the correct PCM_HJTC header...
    //TYPE NO:EEMB4A2A,PCM SPEC:H10487_P4.1     ,LOT ID:N50T7,DATE:2010-03-13 23:21:20
    strString = ReadLine(hPcmHjtcFile);
    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("TYPE NO:", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // Read PCM_HJTC information

    QString strDate, strTime;
    lstSections = strString.split(",");
    // Check if have the good count
    if(lstSections.count() <= 3)
    {
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    //TYPE NO:EEMB4A2A,PCM SPEC:H10487_P4.1     ,LOT ID:N50T7,DATE:2010-03-13 23:21:20
    lSection = lstSections[0];
    if(!lSection.startsWith("TYPE", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    mProductID = lSection.section(":",1);

    lSection = lstSections[1];
    if(!lSection.startsWith("PCM SPEC", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    mSpecName = lSection.section(":",1);

    lSection = lstSections[2];
    if(!lSection.startsWith("LOT", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    mLotID = lSection.section(":",1);

    lSection = lstSections[3];
    if(!lSection.startsWith("DATE", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PCM_HJTC file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    //DATE:2010-03-13 23:21:20
    strDate = lSection.section(":",1).section(" ",0,0);
    strTime =  lSection.section(":",1).section(" ",1);
    QString lDateTemplate;
    if (strDate.contains("/"))
    {
        lDateTemplate = "yyyy/MM/dd";
    }
    else if (strDate.contains("-"))
    {
        lDateTemplate = "yyyy-MM-dd";
    }
    else
    {
        // Incorrect header...this is not a PCM_HJTC file!
        mLastError = errInvalidFormatParameter;
        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QDate		lDate = QDate::fromString(strDate, lDateTemplate);
    QTime		lTime(strTime.section(":",0,0).toInt(),strTime.section(":",1,1).toInt(),strTime.section(":",2).toInt());
    QDateTime clDateTime(lDate,lTime);
    clDateTime.setTimeSpec(Qt::UTC);
    mStartTime = clDateTime.toTime_t();

    strString = ReadLine(hPcmHjtcFile);

    // Read line with list of Parameters names

    int nStartData = 0, nStartSpec = 0, nEndSpec = 0;
    QString	strLotsLine, strWafersLine, strSitesLine, strLotChar,
            strSiteChar;
    int		lWaferNb,lSiteNb;
    bool lHasUnit = false;
    bool lHasCritical = false;
    bool lHeaderRead = false;
    mCriticalTests.clear();

    if(!strString.startsWith("LOT,"))
    {
        while (!strString.startsWith("LOT,"))
        {
            strString = ReadLine(hPcmHjtcFile);

            if(strString.startsWith("PART NO,"))
            {
                mProductID = strString.section(",", 0, 0).section(":", 1, 1).trimmed();
                continue;
            }
            else if(strString.startsWith("PROCESS NAME,"))
            {
                mProcessID = strString.section(",", 0, 0).section(":", 1, 1).trimmed();
                continue;
            }
            else if(strString.startsWith("LOT,"))
            {
                lHeaderRead = true;
                strLotsLine = strString;
                nStartData = 1;
                if (strLotsLine.section(",", 1, 1).startsWith("UNIT"))
                {
                    lHasUnit = true;
                    ++nStartData;
                }
                if (strLotsLine.section(",", 2, 2).startsWith("CRITICAL"))
                {
                    lHasCritical = true;
                    ++nStartData;
                }

                nEndSpec = strLotsLine.count(",");
                continue;
            }

        }
    }

    // Loop reading file until end is reached
    //LOT,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,N50T7,...,<MAX>,<MIN>,<AVG>,<STD>,<SPEC HIGH>,<SPEC LOW>
    //WF#,6,6,6,6,6,7,7,7,7,7,8,8,8,8,8,9,9,9,9,9,10,10,10,10,10,11,11,11,11,11,12,12
    //S#,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,R,L,T,B,C,
    //VTON10/.35,0.596   ,0.596   ,0.582   ,0.589   ,0.594   ,0.592   ,0.592   ,...  ,0.602   ,0.578   ,0.591   ,0.00429 ,0.71    ,0.51
    do
    {
        lUnit = "";
        strString = ReadLine(hPcmHjtcFile);

        if(strString.startsWith("WF#"))
        {
            strWafersLine = strString;
            nStartSpec = strWafersLine.count(",");
            continue;
        }
        else if(strString.startsWith("S#"))
        {
            strSitesLine = strString;
            continue;
        }

        if((strLotsLine.isEmpty() || strWafersLine.isEmpty() || strSitesLine.isEmpty() || (nStartSpec==0)) && lHeaderRead)
        {
            // Incorrect header...this is not a PCM_HJTC file!
            mLastError = errInvalidFormatParameter;

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }


        // Read data
        lParameter = strString.section(",",0,0).trimmed();

        // Read Unit?
        if (lHasUnit) lUnit = strString.section(",", 1, 1);

        // Update CriticalTests MetaData ?
        if (lHasCritical)
        {
            QString lCritical = strString.section(",", 2, 2);
            if (lCritical.trimmed() == "1") mCriticalTests.append(lParameter);
        }

        // For each column, extract result value and save it
        for(iIndex=nStartData; iIndex<=nStartSpec; ++iIndex)
        {
            lSection = strString.section(",",iIndex,iIndex);
            strLotChar = strLotsLine.section(",",iIndex,iIndex);

            if(iIndex <= nStartSpec)
            {

                lWaferNb = strWafersLine.section(",",iIndex,iIndex).toInt();
                strSiteChar = strSitesLine.section(",",iIndex,iIndex);
                if(strSiteChar == "C")
                    lSiteNb = 1;	// Center
                else
                if(strSiteChar == "B")
                    lSiteNb = 2;	// Bottom (Down)
                else
                if(strSiteChar == "L")
                    lSiteNb = 3;	// Left
                else
                if(strSiteChar == "T")
                    lSiteNb = 4;	// Top
                else
                if(strSiteChar == "R")
                    lSiteNb = 5;	// Right
                else
                    lSiteNb = 6;	// Other site...


                    // Save parameter result in buffer
                if(!lSection.isEmpty())
                    SaveParameterResult(lWaferNb,lSiteNb,lParameter,lUnit,lSection);
            }
        }

        SaveParameterLimit(lParameter, strString.section(",", nEndSpec - 1, nEndSpec - 1), eHighLimit);
        SaveParameterLimit(lParameter, strString.section(",", nEndSpec, nEndSpec), eLowLimit);
    }
    while(!hPcmHjtcFile.atEnd());


    // Close file
    f.close();

    // All PCM_HJTC file read...check if need to update the PCM_HJTC Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Success parsing PCM_HJTC file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM_HJTC data parsed
//////////////////////////////////////////////////////////////////////
bool CGPcmHjtctoSTDF::WriteStdfFile(const QString &strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open(strFileNameSTDF.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing PCM_HJTC file into STDF database
        mLastError = errWriteSTDF;

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

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(mStartTime);			// Setup time
    StdfFile.WriteDword(mStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(mLotID.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(mProductID.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(mNodeName.toLatin1().constData());		// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString(mSpecName.toLatin1().constData());	// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString(mOperatorID.toLatin1().constData());	// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":PCM";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString(mProductID.toLatin1().constData());	// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("UMC");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString(mProcessID.toLatin1().constData());// ProcessID
    StdfFile.WriteString("");					// OperFreq
    StdfFile.WriteString(mSpecName.toLatin1().constData());	// Spec name

    StdfFile.WriteRecord();


    // MetaData CriticalTests

    // Write Test results for each waferID
    ParserWafer	*ptWafers;		// List of Wafers in PCM_HJTC file
    char		szString[257];
    BYTE		iSiteNumber,bData;
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTestNumber,iPartNumber=0;
    bool		bPassStatus;

    ParserParameter *ptParam;

    QListIterator<ParserWafer*> lstIteratorWafer(mWaferList);
    lstIteratorWafer.toFront();

    // To change when writing 1 Test for 1 DTR
    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
    QString lCtriticalTestLabel = "CriticalTests";
    QString lSeparator = ",";
    for (int lIter = 0; lIter < mCriticalTests.size(); ++lIter)
    {
        AddMetaDataToDTR(lCtriticalTestLabel, mCriticalTests.at(lIter) ,&lDTRRecord, true, lSeparator);
        lDTRRecord.Write(StdfFile);
    }

    while(lstIteratorWafer.hasNext())
    {
        ptWafers = lstIteratorWafer.next();

        // Write WIR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test head
        StdfFile.WriteByte(255);					// Tester site (all)
        StdfFile.WriteDword(mStartTime);			// Start time
        sprintf(szString,"%d",ptWafers->mWaferID);
        StdfFile.WriteString(szString);				// WaferID
        StdfFile.WriteRecord();

        // Write all Parameters read on this wafer.: PTR....PTR, PRR
        iTotalGoodBin=iTotalFailBin=0;

        // Write PTRs for EACH of the X sites
        for(iSiteNumber = ptWafers->mLowestSiteID; iSiteNumber <= ptWafers->mHighestSiteID; iSiteNumber++)
        {
            // before write PIR, PTRs, PRR
            // verify if we have some test executed from this site
            QList<ParserParameter*>::iterator itParameterBegin	= ptWafers->mParameterList.begin();
            QList<ParserParameter*>::iterator itParameterEnd	= ptWafers->mParameterList.end();

            bPassStatus = false;
            while(itParameterBegin != itParameterEnd)
            {
                if((*itParameterBegin)->HasTestValueForSite(iSiteNumber) == true)
                    bPassStatus = true;

                // Next Parameter!
                itParameterBegin++;
            };

            if(!bPassStatus)
                continue;

            // Write PIR for parts in this Wafer site
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);								// Test head
            StdfFile.WriteByte(iSiteNumber);					// Tester site
            StdfFile.WriteRecord();

            // Part number
            iPartNumber++;

            bPassStatus			= true;
            itParameterBegin	= ptWafers->mParameterList.begin();

            while(itParameterBegin != itParameterEnd)
            {
                ptParam = (*itParameterBegin);

                UpdateProgressBar();

                // Write the PTR if it exists for this site...
                if(ptParam->HasTestValueForSite(iSiteNumber) == true)
                {
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 10;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    // Compute Test# (add user-defined offset)
                    iTestNumber = (long) mParameterDirectory.GetFullParametersList().indexOf(ptParam->GetTestName());
                    iTestNumber += GEX_TESTNBR_OFFSET_HJTC;		// Test# offset
                    StdfFile.WriteDword(iTestNumber);			// Test Number
                    StdfFile.WriteByte(1);						// Test head
                    StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
                    if((ptParam->GetValidLowLimit() && ptParam->GetTestValueForSite(iSiteNumber) < ptParam->GetLowLimit()) ||
                       (ptParam->GetValidHighLimit() && ptParam->GetTestValueForSite(iSiteNumber) > ptParam->GetHighLimit()))
                    {
                        bData = 0200;	// Test Failed
                        bPassStatus = false;
                    }
                    else
                    {
                        bData = 0;		// Test passed
                    }
                    StdfFile.WriteByte(bData);							// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                    StdfFile.WriteFloat(ptParam->GetTestValueForSite(iSiteNumber));	// Test result
                    if(ptParam->GetStaticHeaderWritten() == false)
                    {
                        // save Parameter name without unit information
                        StdfFile.WriteString(ptParam->GetTestName().toLatin1().constData());	// TEST_TXT
                        StdfFile.WriteString("");							// ALARM_ID

                        bData = 2;	// Valid data.
                        if(ptParam->GetValidLowLimit()==false)
                            bData |=0x40;
                        if(ptParam->GetValidHighLimit()==false)
                            bData |=0x80;
                        StdfFile.WriteByte(bData);							// OPT_FLAG

                        StdfFile.WriteByte(-ptParam->GetResultScale());				// RES_SCALE
                        StdfFile.WriteByte(-ptParam->GetResultScale());				// LLM_SCALE
                        StdfFile.WriteByte(-ptParam->GetResultScale());				// HLM_SCALE
                        StdfFile.WriteFloat(ptParam->GetValidLowLimit() ? ptParam->GetLowLimit() : 0);			// LOW Limit
                        StdfFile.WriteFloat(ptParam->GetValidHighLimit() ? ptParam->GetHighLimit() : 0);		// HIGH Limit
                        StdfFile.WriteString(ptParam->GetTestUnits().toLatin1().constData());	// Units
                        ptParam->SetStaticHeaderWritten(true);
                    }
                    StdfFile.WriteRecord();
                }

                // Next Parameter!
                itParameterBegin++;
            };

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test head
            StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5
            if(bPassStatus == true)
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                wSoftBin = wHardBin = 1;
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                wSoftBin = wHardBin = 0;
                iTotalFailBin++;
            }
            StdfFile.WriteWord((WORD)ptWafers->mParameterList.count());		// NUM_TEST
            StdfFile.WriteWord(wHardBin);            // HARD_BIN
            StdfFile.WriteWord(wSoftBin);            // SOFT_BIN
            // 200mm wafers, usually have 5 sites, 300mm wafers usually have 9 sites
            switch(iSiteNumber)
            {
                case 1:	// Center
                    StdfFile.WriteWord(1);			// X_COORD
                    StdfFile.WriteWord(1);			// Y_COORD
                    break;
                case 2:	// Bottom (Down)
                    StdfFile.WriteWord(1);			// X_COORD
                    StdfFile.WriteWord(0);			// Y_COORD
                    break;
                case 3:	// Left
                    StdfFile.WriteWord(0);			// X_COORD
                    StdfFile.WriteWord(1);			// Y_COORD
                    break;
                case 4:	// Top
                    StdfFile.WriteWord(1);			// X_COORD
                    StdfFile.WriteWord(2);			// Y_COORD
                    break;
                case 5:	// Right
                    StdfFile.WriteWord(2);			// X_COORD
                    StdfFile.WriteWord(1);			// Y_COORD
                    break;
                default: // More than 5 sites?....give 0,0 coordonates
                    StdfFile.WriteWord(0);			// X_COORD
                    StdfFile.WriteWord(0);			// Y_COORD
                    break;
            }
            StdfFile.WriteDword(0);				// No testing time known...
            sprintf(szString,"%ld",iPartNumber);
            StdfFile.WriteString(szString);		// PART_ID
            StdfFile.WriteString("");			// PART_TXT
            StdfFile.WriteString("");			// PART_FIX
            StdfFile.WriteRecord();
        }


        // Write WRR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test head
        StdfFile.WriteByte(255);					// Tester site (all)
        StdfFile.WriteDword(mStartTime);			// Time of last part tested
        StdfFile.WriteDword(iTotalFailBin+iTotalGoodBin);	// Parts tested: always 5
        StdfFile.WriteDword(0);						// Parts retested
        StdfFile.WriteDword(0);						// Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
        StdfFile.WriteDword((DWORD)(-1));			// Functionnal Parts

        sprintf(szString,"%d",ptWafers->mWaferID);
        StdfFile.WriteString(szString);				// WaferID
        StdfFile.WriteRecord();
    };

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(mStartTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PCM_HJTC file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPcmHjtctoSTDF::ConvertoStdf(const QString& aPcmHjtcFileName, QString &aFileNameSTDF)
{
    // No erro (default)
    mLastError = errNoError;
    mTotalParameters = 0;


    if(ReadPcmHjtcFile(aPcmHjtcFileName) != true)
    {
        return false;	// Error reading PCM_HJTC file
    }

    if(WriteStdfFile(aFileNameSTDF) != true)
    {
        QFile::remove(aFileNameSTDF);
        return false;
    }

    // Convertion successful
    return true;
}

}
}
