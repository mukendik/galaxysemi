//////////////////////////////////////////////////////////////////////
// import_pcm_gsmc.cpp: Convert a PcmGsmc .csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include "import_pcm_gsmc.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

// PCM Gsmc format
//Lot Id,X00000001.1,GSMC Part Id,G0000A-NNN-0000000,Technology,A018,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Equipment Id,XXXXXXA,Customer Part Id,000001-0001-00001,Operator Id,2327,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Test Name,Gxxxxx_W.tst,Limit File,Gxxxxxx.lim,Criteria,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Quantity,25,Pcm Spec,L018,Date/Time,20:13.0,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//ProbeCardId,13050,PFlatOrientation,RIGHT,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//Wafer No.,Site id,BVN10L.18T,BVNt10L.35T,BVP10L.18T,BVPt10L.3T,BvCgnw,BvC
//1,-2|0,4.20E+00,9.00E+00,-5.30E+00,-7.50E+00,4.50E+00,7.90E+00,-4.80E+00,
//1,0|2,4.20E+00,9.00E+00,-5.30E+00,-7.50E+00,4.50E+00,7.90E+00,-4.90E+00,-
//...
//ope_id,parameter_name,unit,valid_low,valid_high,key_flag,spec_low,spec_high,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//626,BVN10L.18T,V, , ,Y ,3.60E+00,null,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
//969,RsNpoW.18,ohm/sq,1.0000E-03,1.0000E+04,Y ,1.0000E+00,1.0000E+01,

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmGsmctoSTDF::CGPcmGsmctoSTDF()
{
    m_lStartTime = 0;
    m_nPass = 0;

    m_pPcmGsmcParameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPcmGsmctoSTDF::~CGPcmGsmctoSTDF()
{
    // Destroy list of Parameters tables.
    if(m_pPcmGsmcParameter!=NULL)
        delete [] m_pPcmGsmcParameter;
    m_mapNameParameter.clear();
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPcmGsmctoSTDF::GetLastError()
{
    strLastError = "Import PCM Gsmc: ";

    switch(iLastError)
    {
        default:
        case errNoError:
            strLastError += "No Error";
            break;
        case errOpenFail:
            strLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            strLastError += "Invalid file format";
            break;
        case errInvalidFormatLowInRows:
            strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
            break;
        case errNoLimitsFound:
            strLastError += "Invalid file format: Specification Limits not found";
            break;
        case errWriteSTDF:
            strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            strLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PcmGsmc format
//////////////////////////////////////////////////////////////////////
bool CGPcmGsmctoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strValue;
    QString	strSite;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        return false;
    }
    // Assign file I/O stream
    QTextStream hPcmGsmcFile(&f);

    // Check if line is the correct PcmGsmc header...
    //Wafer No.,Site id,BVN10L.18T,BVNt10L.35T,BVP10L.18T,BVPt10L.3T,BvCgnw,BvCgnwH,BvCgpw,BvCgpwH,IbriM

    int nLine = 0;
    do
    {
        nLine++;
        strString = hPcmGsmcFile.readLine().simplified();
        if(strString.startsWith("Wafer No.,Site id,",Qt::CaseInsensitive))
            break;
        if(nLine > 10)
            break;
        strString = "";
    }
    while(!strString.isNull() && strString.isEmpty());

    if(!strString.startsWith("Wafer No.,Site id,",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PcmGsmc file!
        f.close();
        return false;
    }
    f.close();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PcmGsmc file
//////////////////////////////////////////////////////////////////////
bool CGPcmGsmctoSTDF::ReadPcmGsmcFile(const char *PcmGsmcFileName, const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bStatus;
    int		iIndex;				// Loop index

    // Open CSV file
    QFile f( PcmGsmcFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening PcmGsmc file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;
    m_nPass = 1;


    // Assign file I/O stream
    QTextStream hPcmGsmcFile(&f);

    m_nWaferID = 1;
    m_lStartTime = 0;


    //Lot Id,X00000001.1,GSMC Part Id,G0000A-NNN-0000000,Technology,A018,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    //Equipment Id,XXXXXXA,Customer Part Id,000001-0001-00001,Operator Id,2327,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    //Test Name,Gxxxxx_W.tst,Limit File,Gxxxxxx.lim,Criteria,2,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    //Quantity,25,Pcm Spec,L018,Date/Time,20:13.0,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    //ProbeCardId,13050,PFlatOrientation,RIGHT,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
    //Wafer No.,Site id,BVN10L.18T,BVNt10L.35T,BVP10L.18T,BVPt10L.3T,BvCgnw,BvCgnwH,BvCgpw,BvCgpwH,IbriM

    QDate	clDate;
    QTime	clTime;

    strString = ReadLine(hPcmGsmcFile);

    // Read PCM Gsmc information
    while(!hPcmGsmcFile.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hPcmGsmcFile);

        if(strString.startsWith("Wafer No.,Site id,",Qt::CaseInsensitive))
        {
            // Data
            break;
        }

        while(strString.startsWith(","))
        {
            strString = strString.section(",",1);
        }

        strSection = strString.section(",",0,0).simplified();
        strString = strString.section(",",1).simplified();

        //Lot Id,X00000001.1,
        if(strSection.startsWith("Lot Id",Qt::CaseInsensitive))
        {
            m_strLotID = strString.section(",",0,0);
        }
        //GSMC Part Id,G0000A-NNN-0000000,
        else if(strSection.startsWith("GSMC Part Id",Qt::CaseInsensitive))
        {
            m_strPartType = strString.section(",",0,0);
        }
        //Technology,A018,
        else if(strSection.startsWith("Technology",Qt::CaseInsensitive))
        {
            m_strProcessId = strString.section(",",0,0);
        }
        //Equipment Id,XXXXXXA,
        else if(strSection.startsWith("Equipment Id",Qt::CaseInsensitive))
        {
            m_strNodeName = strString.section(",",0,0);
        }
        //Customer Part Id,000001-0001-00001,
        else if(strSection.startsWith("Customer Part Id",Qt::CaseInsensitive))
        {
            m_strProgramId = strString.section(",",0,0);
        }
        //Operator Id,2327,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
        else if(strSection.startsWith("Operator Id",Qt::CaseInsensitive))
        {
            m_strOperatorName = strString.section(",",0,0);
        }
        //Limit File,Gxxxxxx.lim,
        else if(strSection.startsWith("Limit File",Qt::CaseInsensitive))
        {
            m_strAuxFile = strString.section(",",0,0);
        }
        //Pcm Spec,L018,
        else if(strSection.startsWith("Pcm Spec",Qt::CaseInsensitive))
        {
            m_strSpecName = strString.section(",",0,0);
        }
        //Date/Time,2009-01-01 08:53:36,
        else if(strSection.startsWith("Date/Time",Qt::CaseInsensitive))
        {
            //Date Time:2009-01-01 08:53:36
            QString	strDate = strString.section(" ",0,0);
            clDate = QDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());

            QString strTime = strString.section(",",0,0).section(" ",1,1);
            clTime.setHMS(strTime.section(":",0,0).toInt(), strTime.section(":",1,1).toInt(), strTime.section(":",2,2).toInt());
        }
        //PFlatOrientation,RIGHT,
        else if(strSection.startsWith("PFlatOrientation",Qt::CaseInsensitive))
        {
            m_strWaferFlat = strString.section(",",0,0);
        }
        //Test Name,Gxxxxx_W.tst,
        //Criteria,2,
        //Quantity,25,
        //ProbeCardId,13050,
        else if(strSection.startsWith("ProbeCardId",Qt::CaseInsensitive))
        {
            m_strProbeCardId = strString.section(",",0,0);
        }

        strString = strString.section(",",1);

    }

    QDateTime clDateTime(clDate,clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    //Column,Row,Part ID,Wafer ID,# of pits
    if(!strString.startsWith("Wafer No.,Site id,",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PcmGsmc file!
        f.close();
        return false;
    }

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);

    // Count the number of parameters specified in the line
    m_iTotalParameters=lstSections.count();
    // If no parameter specified...ignore!
    if(m_iTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid PcmGsmc file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    m_pPcmGsmcParameter = new CGPcmGsmcParameter[m_iTotalParameters];	// List of parameters

    // Extract the N column names
    // Do not count first 2 fields
    for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
    {
        strSection = lstSections[iIndex];

        if(strSection.isEmpty())
            break;

        m_pPcmGsmcParameter[iIndex].nNumber = iIndex;
        m_pPcmGsmcParameter[iIndex].strName = strSection;
        m_pPcmGsmcParameter[iIndex].strUnits = "";
        m_pPcmGsmcParameter[iIndex].nScale = 0;
        m_pPcmGsmcParameter[iIndex].bValidHighLimit = false;
        m_pPcmGsmcParameter[iIndex].bValidLowLimit = false;
        m_pPcmGsmcParameter[iIndex].fHighLimit = 0;
        m_pPcmGsmcParameter[iIndex].fLowLimit = 0;
        m_pPcmGsmcParameter[iIndex].bValidSpecHighLimit = false;
        m_pPcmGsmcParameter[iIndex].bValidSpecLowLimit = false;
        m_pPcmGsmcParameter[iIndex].fSpecHighLimit = 0;
        m_pPcmGsmcParameter[iIndex].fSpecLowLimit = 0;
        m_pPcmGsmcParameter[iIndex].bStaticHeaderWritten = false;

        m_mapNameParameter[strSection] = &m_pPcmGsmcParameter[iIndex];
    }

    m_iTotalParameters = iIndex;

    // Go to the limit section at the end of the file
    while(!hPcmGsmcFile.atEnd())
    {
        strString = ReadLine(hPcmGsmcFile).remove(" ").toLower();
        if(strString.left(47) == "ope_id,parameter_name,unit,valid_low,valid_high")
            break;
    }

    if(strString.left(47) == "ope_id,parameter_name,unit,valid_low,valid_high")
    {
        // Have Limit spec

        int iLimitFilePos = hPcmGsmcFile.device()->pos();

        CGPcmGsmcParameter *pParameter;
        while(!hPcmGsmcFile.atEnd())
        {
            strString = ReadLine(hPcmGsmcFile).simplified();

            // Save Parameter info
            if(!m_mapNameParameter.contains(strString.section(",",1,1)))
                continue;

            pParameter = m_mapNameParameter[strString.section(",",1,1)];

            pParameter->nNumber = strString.section(",",0,0).toInt();

            pParameter->strUnits = strString.section(",",2,2);
            NormalizeLimits(pParameter->strUnits, pParameter->nScale);

            pParameter->fLowLimit = strString.section(",",3,3).toFloat(&pParameter->bValidLowLimit);
            pParameter->fHighLimit = strString.section(",",4,4).toFloat(&pParameter->bValidHighLimit);

            pParameter->fSpecLowLimit = strString.section(",",6,6).toFloat(&pParameter->bValidSpecLowLimit);
            pParameter->fSpecHighLimit = strString.section(",",7,7).toFloat(&pParameter->bValidSpecHighLimit);

            pParameter->bStaticHeaderWritten = false;

        }

        // 100% before the end of the file
        iFileSize = iLimitFilePos;
    }

    // Restart at the begining
    f.close();

    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }


    // Go to result data
    while(!hPcmGsmcFile.atEnd())
    {
        strString = ReadLine(hPcmGsmcFile);

        if(strString.startsWith("Wafer No.,Site id,",Qt::CaseInsensitive))
        {
            // Data
            break;
        }
    }

    if(!strString.startsWith("Wafer No.,Site id,",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a valid PcmGsmc file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        f.close();
        return false;
    }


    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hPcmGsmcFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Success parsing PcmGsmc file
    f.close();
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PcmGsmc data parsed
//////////////////////////////////////////////////////////////////////
bool CGPcmGsmctoSTDF::WriteStdfFile(QTextStream *hPcmGsmcFile, const char *strFileNameSTDF)
{
    m_nPass = 2;
    iNextFilePos = 0;

    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        iLastError = errWriteSTDF;

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

    if(m_lStartTime <= 0)
        m_lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// Setup time
    StdfFile.WriteDword(m_lStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strPartType.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(m_strNodeName.toLatin1().constData());	// Node name
    StdfFile.WriteString("PCM Gsmc");			// Tester Type
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());	// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString(m_strOperatorName.toLatin1().constData());	// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    strUserTxt += ":PCM_GSMC";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString((char *)m_strAuxFile.toLatin1().constData());	// aux-file
    StdfFile.WriteString((char *)m_strPackageID.toLatin1().constData());	// package-type
    StdfFile.WriteString(m_strFamilyID.toLatin1().constData());	// familyID
    StdfFile.WriteString("");							// Date-code
    StdfFile.WriteString("");							// Facility-ID
    StdfFile.WriteString("");							// FloorID
    StdfFile.WriteString((char *)m_strProcessId.toLatin1().constData());	// ProcessID
    StdfFile.WriteString("");							// OperationFreq
    StdfFile.WriteString((char *)m_strSpecName.toLatin1().constData());	// Spec-nam
    StdfFile.WriteString("");							// Spec-ver
    StdfFile.WriteString("");							// Flow-id
    StdfFile.WriteString("");							// setup_id
    StdfFile.WriteRecord();

    if(!m_strProbeCardId.isEmpty())
    {
        // Write SDR
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 80;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte((BYTE)1);			// head#
        StdfFile.WriteByte((BYTE)1);			// Group#
        StdfFile.WriteByte((BYTE)1);			// site_count
        StdfFile.WriteByte((BYTE)1);			// array of test site# (dummy!)
        StdfFile.WriteString("");				// HAND_TYP: Handler/prober type
        StdfFile.WriteString("");				// HAND_ID: Handler/prober name
        StdfFile.WriteString("");				// CARD_TYP: Probe card type
        StdfFile.WriteString(m_strProbeCardId.toLatin1().constData());	// CARD_ID: Probe card name
        StdfFile.WriteString("");				// LOAD_TYP: Load board type
        StdfFile.WriteRecord();
    }

    // Write Test results for each line read.
    QString strString;
    QString strSection;
    float	fValue;				// Used for readng floating point numbers.
    int		iIndex;				// Loop index
    int		iXpos, iYpos;
    int		iBin;
    int		iSite;
    int		iCurrentWaferId = -1;
    bool	bPassStatus;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTotalTests,iPartNumber;
    bool		bStatus;
    QStringList	lstSections;
    BYTE		bData;

  //FIXME: not used ?
  //bool bWriteWir;

    // Reset counters
  //FIXME: not used ?
  //bWriteWir = true;
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;
    iSite = 1;

    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

    //Wafer No.,Site id,BVN10L.18T,BVNt10L.35T,BVP10L.18T,BVPt10L.3T,BvCgnw,BvC
    //1,-2|0,4.20E+00,9.00E+00,-5.30E+00,-7.50E+00,4.50E+00,7.90E+00,-4.80E+00,
    //1,0|2,4.20E+00,9.00E+00,-5.30E+00,-7.50E+00,4.50E+00,7.90E+00,-4.90E+00,-
    while(hPcmGsmcFile->atEnd() == false)
    {

        // Read line
        strString = ReadLine(*hPcmGsmcFile);

        if(strString.startsWith("ope_id,parameter_name",Qt::CaseInsensitive))
            break;

        lstSections = strString.split(",",QString::KeepEmptyParts);

        // Check if have the good count
        if(lstSections.count() < m_iTotalParameters)
        {
            bStatus = false;
            if(lstSections.count() > 0)
                lstSections[0].toInt(&bStatus);
            if(!bStatus)
                break;

            iLastError = errInvalidFormatLowInRows;

            StdfFile.Close();
            // Convertion failed.
            return false;
        }

        if(lstSections[0].isEmpty())
            continue;

        // Extract WaferId
        m_nWaferID = lstSections[0].toInt();

        if(m_nWaferID != iCurrentWaferId)
        {
            // New wafer
            // Close the last
            if(iCurrentWaferId > -1)
            {
                // Write WRR for last wafer inserted
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);						// Test head
                StdfFile.WriteByte(255);					// Tester site (all)
                StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
                StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
                StdfFile.WriteDword(0);						// Parts retested
                StdfFile.WriteDword(0);						// Parts Aborted
                StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
                StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
                StdfFile.WriteString(QString::number(iCurrentWaferId).toLatin1().constData());	// WaferID
                StdfFile.WriteString("");					// FabId
                StdfFile.WriteString("");					// FrameId
                StdfFile.WriteString("");					// MaskId
                StdfFile.WriteString("");					// UserDesc
                StdfFile.WriteRecord();

            }

            // Write WIR of new Wafer.
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);								// Test head
            StdfFile.WriteByte(255);							// Tester site (all)
            StdfFile.WriteDword(m_lStartTime);					// Start time
            StdfFile.WriteString(QString::number(m_nWaferID).toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();

            // For each wafer, have to write limit in the first PTR
            for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
            {
                m_pPcmGsmcParameter[iIndex].bStaticHeaderWritten = false;
            }

            iCurrentWaferId = m_nWaferID;
            iTotalGoodBin = iTotalFailBin = iTotalTests = 0;
            iPartNumber = 0;
        }

        // Part number
        iPartNumber++;

        // Extract Column,Row
        strString = lstSections[1];
        iXpos = iYpos = -32768;
        iSite = strString.toInt(&bPassStatus);
        if(!bPassStatus)
        {
            iXpos = strString.section("|",0,0).toInt();
            iYpos = strString.section("|",1,1).toInt();

            if(iXpos == 0)
            {
                switch(iYpos)
                {
                    case 0:
                        iSite = 1;	// Center
                        break;
                    case -2:
                        iSite = 2;	// Down
                        break;
                    case 2:
                        iSite = 4;	// Top
                        break;
                }
            }
            else
            {
                switch(iXpos)
                {
                    case -2:
                        iSite = 3;	// Left
                        break;
                    case 2:
                        iSite = 5;	// Right
                        break;
                }
            }
        }

        iBin = 1;
        bPassStatus = true;

        // Reset counters
        iTotalTests = 0;

        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);					// Test head
        StdfFile.WriteByte(iSite);				// Tester site
        StdfFile.WriteRecord();

        // Read Parameter results for this record
        for(iIndex=2;iIndex<m_iTotalParameters;iIndex++)
        {
            strSection = lstSections[iIndex];
            fValue = strSection.toFloat(&bStatus);
            if(bStatus == true)
            {
                // Valid test result...write the PTR
                iTotalTests++;

                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                // Compute Test# (add user-defined offset)
                StdfFile.WriteDword(m_pPcmGsmcParameter[iIndex].nNumber);			// Test Number
                StdfFile.WriteByte(1);				// Test head
                StdfFile.WriteByte(iSite);			// Tester site#

                if(!m_pPcmGsmcParameter[iIndex].bValidLowLimit && !m_pPcmGsmcParameter[iIndex].bValidHighLimit)
                {
                    // No pass/fail information
                    bData = 0x40;
                }
                else if(((m_pPcmGsmcParameter[iIndex].bValidLowLimit==true) && (fValue < m_pPcmGsmcParameter[iIndex].fLowLimit)) ||
                   ((m_pPcmGsmcParameter[iIndex].bValidHighLimit==true) && (fValue > m_pPcmGsmcParameter[iIndex].fHighLimit)))
                {
                    bData = 0200;	// Test Failed
                    iBin = 0;
                    bPassStatus = false;
                }
                else
                {
                    bData = 0;		// Test passed
                }
                StdfFile.WriteByte(bData);							// TEST_FLG
                StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pPcmGsmcParameter[iIndex].nScale));						// Test result
                if(m_pPcmGsmcParameter[iIndex].bStaticHeaderWritten == false)
                {
                    StdfFile.WriteString(m_pPcmGsmcParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
                    StdfFile.WriteString("");							// ALARM_ID
                    bData = 2;	// Valid data.
                    if(m_pPcmGsmcParameter[iIndex].bValidSpecLowLimit==false)
                        bData |=0x4;
                    if(m_pPcmGsmcParameter[iIndex].bValidSpecHighLimit==false)
                        bData |=0x8;
                    if(m_pPcmGsmcParameter[iIndex].bValidLowLimit==false)
                        bData |=0x40;
                    if(m_pPcmGsmcParameter[iIndex].bValidHighLimit==false)
                        bData |=0x80;
                    StdfFile.WriteByte(bData);							// OPT_FLAG
                    StdfFile.WriteByte(-m_pPcmGsmcParameter[iIndex].nScale);			// RES_SCALE
                    StdfFile.WriteByte(-m_pPcmGsmcParameter[iIndex].nScale);			// LLM_SCALE
                    StdfFile.WriteByte(-m_pPcmGsmcParameter[iIndex].nScale);			// HLM_SCALE
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fLowLimit * GS_POW(10.0,m_pPcmGsmcParameter[iIndex].nScale));		// LOW Limit
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fHighLimit * GS_POW(10.0,m_pPcmGsmcParameter[iIndex].nScale));		// HIGH Limit
                    StdfFile.WriteString(m_pPcmGsmcParameter[iIndex].strUnits.toLatin1().constData());	// Units
                    StdfFile.WriteString("");											//
                    StdfFile.WriteString("");											//
                    StdfFile.WriteString("");											//
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fSpecLowLimit * GS_POW(10.0,m_pPcmGsmcParameter[iIndex].nScale));	// LOW Spec Limit
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fSpecHighLimit * GS_POW(10.0,m_pPcmGsmcParameter[iIndex].nScale));	// HIGH Spec Limit
                    m_pPcmGsmcParameter[iIndex].bStaticHeaderWritten = true;
                }
                StdfFile.WriteRecord();
            }	// Valid test result
            else
            {
                if(m_pPcmGsmcParameter[iIndex].bStaticHeaderWritten == false)
                {
                    RecordReadInfo.iRecordType = 15;
                    RecordReadInfo.iRecordSubType = 10;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    // Compute Test# (add user-defined offset)
                    StdfFile.WriteDword(m_pPcmGsmcParameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_TESSERA);			// Test Number
                    StdfFile.WriteByte(1);						// Test head
                    StdfFile.WriteByte(iSite);					// Tester site#

                    // No pass/fail information
                    bData = 0x52;
                    StdfFile.WriteByte(bData);							// TEST_FLG
                    StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
                    StdfFile.WriteFloat(0);								// Test result
                    StdfFile.WriteString(m_pPcmGsmcParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
                    StdfFile.WriteString("");							// ALARM_ID
                    bData = 2;	// Valid data.
                    if(m_pPcmGsmcParameter[iIndex].bValidLowLimit==false)
                        bData |=0x40;
                    if(m_pPcmGsmcParameter[iIndex].bValidHighLimit==false)
                        bData |=0x80;
                    StdfFile.WriteByte(bData);							// OPT_FLAG
                    StdfFile.WriteByte(-m_pPcmGsmcParameter[iIndex].nScale);			// RES_SCALE
                    StdfFile.WriteByte(-m_pPcmGsmcParameter[iIndex].nScale);			// LLM_SCALE
                    StdfFile.WriteByte(-m_pPcmGsmcParameter[iIndex].nScale);			// HLM_SCALE
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fLowLimit);		// LOW Limit
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fHighLimit);		// HIGH Limit
                    StdfFile.WriteString(m_pPcmGsmcParameter[iIndex].strUnits.toLatin1().constData());	// Units
                    StdfFile.WriteString("");											//
                    StdfFile.WriteString("");											//
                    StdfFile.WriteString("");											//
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fSpecLowLimit);		// LOW Spec Limit
                    StdfFile.WriteFloat(m_pPcmGsmcParameter[iIndex].fSpecHighLimit);	// HIGH Spec Limit
                    m_pPcmGsmcParameter[iIndex].bStaticHeaderWritten = true;
                    StdfFile.WriteRecord();

                }
            }
        }		// Read all results on line

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(iSite);		// Tester site#:1
        if(bPassStatus == true)
        {
            StdfFile.WriteByte(0);				// PART_FLG : PASSED
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte(8);				// PART_FLG : FAILED
            iTotalFailBin++;
        }
        StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
        StdfFile.WriteWord(iBin);				// HARD_BIN
        StdfFile.WriteWord(iBin);				// SOFT_BIN
        StdfFile.WriteWord(iXpos);				// X_COORD
        StdfFile.WriteWord(iYpos);				// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
        StdfFile.WriteString("");			// PART_TXT
        StdfFile.WriteString("");			// PART_FIX
        StdfFile.WriteRecord();
    };			// Read all lines with valid data records in file


    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
    StdfFile.WriteString(QString::number(m_nWaferID).toLatin1().constData());	// WaferID
    StdfFile.WriteString("");					// FabId
    StdfFile.WriteString("");					// FrameId
    StdfFile.WriteString("");					// MaskId
    StdfFile.WriteString("");					// UserDesc
    StdfFile.WriteRecord();

    if(!m_strWaferFlat.isEmpty())
    {
        // Write WCR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 30;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteFloat(0);							// Wafer size
        StdfFile.WriteFloat(0);							// Height of die
        StdfFile.WriteFloat(0);							// Width of die
        StdfFile.WriteByte(0);							// Units are in millimeters
        StdfFile.WriteByte((BYTE) (char)m_strWaferFlat[0].toLatin1());	// Orientation of wafer flat
        StdfFile.WriteRecord();
    }

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PcmGsmc file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPcmGsmctoSTDF::Convert(const char *PcmGsmcFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(PcmGsmcFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;
    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(100);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmGsmcFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmGsmcFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    if(ReadPcmGsmcFile(PcmGsmcFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading PcmGsmc file
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    // Convertion successful
    return true;
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGPcmGsmctoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
{
    nScale = 0;
    if(strUnit.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        return;
    }

    QChar cPrefix = strUnit[0];
    switch(cPrefix.toLatin1())
    {
        case 'm': // Milli
            nScale = -3;
            break;
        case 'u': // Micro
            nScale = -6;
            break;
        case 'n': // Nano
            nScale = -9;
            break;
        case 'p': // Pico
            nScale = -12;
            break;
        case 'f': // Fento
            nScale = -15;
            break;
        case 'K': // Kilo
            nScale = 3;
            break;
        case 'M': // Mega
            nScale = 6;
            break;
        case 'G': // Giga
            nScale = 9;
            break;
        case 'T': // Tera
            nScale = 12;
            break;
    }
    if(nScale)
        strUnit = strUnit.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGPcmGsmctoSTDF::ReadLine(QTextStream& hFile)
{
    QString strString;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) hFile.device()->pos() > iNextFilePos)
        {
            iProgressStep += 100/iFileSize + 1;
            iNextFilePos  += iFileSize/100 + 1;
            GexProgressBar->setValue(iProgressStep + ((m_nPass-1)*100));
        }
    }
    QCoreApplication::processEvents();

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}
