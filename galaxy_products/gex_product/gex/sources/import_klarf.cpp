//////////////////////////////////////////////////////////////////////
// import_klarf.cpp: Convert a KLARF file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "import_klarf.h"
#include "import_constants.h"
#include "engine.h"

// File format:
//FileVersion 1 1;
//FileTimestamp 11-22-06 12:34:05;
//TiffSpec 6.0     G     R;
//InspectionStationID "NONE" "Camtek_Falcon" "MHPC948";
//SampleType WAFER;
//SampleSize 1 152;
//ResultTimestamp 11-22-06 12:12:50;
//LotID "0";
//SampleOrientationMarkType NOTCH;
//OrientationMarkLocation DOWN;
// ...
//ClassLookup 101
//0 "No Revision"
//1 "Good"
//2 "2"
//3 "3"
// ...
//99 "99"
//100 "Solder";
//InspectionTest 1;
//SampleTestPlan 597
// 204 -233
// 204 -232
// ...
// 228 -210;
//AreaPerTest 124376592;
//DefectRecordSpec 11 DEFECTID X Y XREL YREL XINDEX YINDEX CLASSNUMBER TEST IMAGECOUNT IMAGELIST ;
//DefectList
//1 94128.1194136222 -99129.3769405257 287.119413622233 166.623059474252 209 -213 100 1 1 1 1 0;
//SummarySpec 4
// TESTNO NDEFECT NDIE NDEFDIE;
//SummaryList
// 1 1 597 1;
//EndOfFile;


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGKLARFtoSTDF::CGKLARFtoSTDF()
{
    m_lStartTime = 0;
    m_nBinGood	= 1;
    m_fDieHt	= 0;
    m_fDieWid	= 0;
    mPosX       = 'R';
    mPosY       = 'D';
}

CGKLARFtoSTDF::~CGKLARFtoSTDF()
{
    QMap<QString,QStringList*>::Iterator itPart;
    for(itPart=m_mapPartBinning.begin(); itPart!=m_mapPartBinning.end(); itPart++)
    {
        delete m_mapPartBinning[itPart.key()];
        m_mapPartBinning[itPart.key()] = NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGKLARFtoSTDF::GetLastError()
{
    m_strLastError = "Import KLARF: ";

    switch(m_iLastError)
    {
        default:
        case errNoError:
            m_strLastError += "No Error";
            break;
        case errOpenFail:
            m_strLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            m_strLastError += "Invalid file format";
            break;
        case errWriteSTDF:
            m_strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            m_strLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with KLARF format
//////////////////////////////////////////////////////////////////////
bool CGKLARFtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hKlarfFile(&f);

    // Check if first line is the correct KLARF header...
    // FileVersion 1 1;
    do
        strString = hKlarfFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("FileVersion", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a KLARF file!
        // Close file
        f.close();
        return false;
    }
    // read next record
    do
        strString = hKlarfFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("FileTimestamp", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a KLARF file!
        // Close file
        f.close();
        return false;
    }

    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the KLARF file
//////////////////////////////////////////////////////////////////////
bool CGKLARFtoSTDF::ReadKlarfFile(const char *KlarfFileName)
{
    QString				strValue;
    QString				strString;
    QString				strSection;
    int					iValue, iPos, iIndex;
    QMap<QString,int>	mapFieldPos;

    // Open KLARF file
    QFile f( KlarfFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening KLARF file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size() + 1;

    // Assign file I/O stream
    QTextStream hKlarfFile(&f);

    // Check if first line is the correct KLARF header...
    // FileVersion 1 1;
    strString = ReadLine(hKlarfFile);
    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("FileVersion", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a KLARF file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    // read next record
    strString = ReadLine(hKlarfFile);
    strString = strString.trimmed();	// remove leading spaces.
    if(!strString.startsWith("FileTimestamp", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a KLARF file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // read next record
    strString = ReadLine(hKlarfFile);
    strString = strString.trimmed();

    // Read KLARF information
    while(!hKlarfFile.atEnd())
    {
        if(strString.isEmpty())
        {
            hKlarfFile.skipWhiteSpace();
            strString = ReadLine(hKlarfFile).simplified();
        }

        strSection = strString.section(" ",0,0).simplified();
        strString = strString.section(" ",1).simplified();
        while(strString.isEmpty() && !hKlarfFile.atEnd())
            strString = ReadLine(hKlarfFile).simplified();
        strValue = strString.section(';',0,0).remove('"').simplified();
        if(strString.indexOf(';'))
            strString = ";"+strString.section(';',1).simplified();

        if(strSection.startsWith("EndOfFile", Qt::CaseInsensitive))
            break;
        else if (strSection.startsWith("WCR.POS_X", Qt::CaseInsensitive))
        {
            if (strValue.compare("RIGHT", Qt::CaseInsensitive) == 0)
            {
                mPosX = 'R';
            }
            else if (strValue.compare("LEFT", Qt::CaseInsensitive) == 0)
            {
                mPosX = 'L';
            }
            else
            {
                mPosX = ' ';
            }
        }
        else if (strSection.startsWith("WCR.POS_Y", Qt::CaseInsensitive))
        {
            if (strValue.compare("DOWN", Qt::CaseInsensitive) == 0)
            {
                mPosY = 'D';
            }
            else if (strValue.compare("UP", Qt::CaseInsensitive) == 0)
            {
                mPosY = 'U';
            }
            else
            {
                mPosY = ' ';
            }
        }
        else if(strSection.startsWith("SampleType", Qt::CaseInsensitive))
        {
            if(!strValue.startsWith("WAFER", Qt::CaseInsensitive))
            {
                // Incorrect header...this is not a KLARF file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                // Close file
                f.close();
                return false;
            }
        }
        else
        if(strSection.startsWith("LotID", Qt::CaseInsensitive))
        {
            m_strLotID = strValue;
        }
        else
        if(strSection.startsWith("StepId", Qt::CaseInsensitive))
        {
            m_strStepId = strValue;
        }
        else
        if(strSection.startsWith("SetupID", Qt::CaseInsensitive))
        {
            m_strSetupId = strValue.section(" ",0,0).simplified();
        }
        else
        if(strSection.startsWith("DeviceID", Qt::CaseInsensitive))
        {
            m_strDeviceID = strValue;
        }
        else
        if(strSection.startsWith("WaferID", Qt::CaseInsensitive))
        {
            m_strWaferID = strValue;
        }
        else
        if(strSection.startsWith("OrientationMarkLocation", Qt::CaseInsensitive))
        {
            m_strNotch = strValue.toUpper();
        }
        else
        if(strSection.startsWith("DiePitch", Qt::CaseInsensitive))
        {
            m_fDieHt = strValue.section(" ",0,0).simplified().toFloat();
            m_fDieWid = strValue.section(" ",1,1).simplified().toFloat();
        }
        else
        if(strSection.startsWith("ClassLookup", Qt::CaseInsensitive))
        {
            // Parse bin list
            int nNbBin = strValue.simplified().toInt();
            for(int iIndex=0 ; iIndex<nNbBin ; iIndex++)
            {
                strString = ReadLine(hKlarfFile).simplified();
                strSection = strString.section(" ",0,0).simplified().toUpper();
                strValue = strString.section(" ",1).remove(";").remove('"').simplified();
                m_mapBinName[strSection.toInt()] = strValue;
                m_mapBinCnt[strSection.toInt()] = 0;
                if(strValue.startsWith("Good", Qt::CaseInsensitive))
                    m_nBinGood = strSection.toInt();
            }
        }
        else
        if(strSection.startsWith("SampleTestPlan", Qt::CaseInsensitive))
        {
            // Parse part list
            int iValue = strValue.simplified().toInt();
            for(iIndex=0 ; iIndex<iValue ; iIndex++)
            {
                strValue = strString = ReadLine(hKlarfFile).simplified();
                strValue = strValue.remove(";").remove('"').simplified();
                m_mapPartBinning[strValue] = NULL;
            }
        }
        else
        // ignore if(strSection.startsWith("AreaPerTest", Qt::CaseInsensitive))
        if(strSection.startsWith("DefectRecordSpec", Qt::CaseInsensitive))
        {
            // get the number of field in the DefectRecordSpec
            iValue = strValue.section(' ',0,0).simplified().toInt();
            strValue = strValue.section(' ',1);
            iPos = 0;
            for(iIndex=0 ; iIndex<iValue ; iIndex++)
            {
                if(strValue.isEmpty())
                {
                    // have to read next line
                    strValue = strString = ReadLine(hKlarfFile).simplified();
                    strValue = strValue.remove(";").remove('"').simplified();
                    iPos = 0;
                }
                // read each field and save it position in the qMapList
                mapFieldPos[strValue.section(' ',iPos,iPos).simplified().toUpper()] = iIndex;
                iPos++;
            }
        }
        else
        if(strSection.startsWith("DefectList", Qt::CaseInsensitive))
        {
            // Parse part list
            bool	bIsInt;
            QString strCoord;
            QStringList *pPartBin=NULL;

            // Verify if have all needed fields
            if(mapFieldPos.contains("DEFECTID")
            && mapFieldPos.contains("XINDEX")
            && mapFieldPos.contains("YINDEX")
            && mapFieldPos.contains("CLASSNUMBER"))
            {
                strValue.section(" ",mapFieldPos["DEFECTID"],
                mapFieldPos["DEFECTID"]).simplified().toInt(&bIsInt);
                while(bIsInt)
                {
                    strCoord = strValue.section(" ",mapFieldPos["XINDEX"],mapFieldPos["XINDEX"]).simplified();
                    strCoord += " " + strValue.section(" ",mapFieldPos["YINDEX"],mapFieldPos["YINDEX"]).simplified();
                    if(m_mapPartBinning.contains(strCoord))
                    {
                        // add bin result for this part
                        pPartBin = m_mapPartBinning[strCoord];
                        if(pPartBin == NULL)
                            m_mapPartBinning[strCoord] = new QStringList;
                        m_mapPartBinning[strCoord]->append(strValue.section(" ",mapFieldPos["CLASSNUMBER"],mapFieldPos["CLASSNUMBER"]).simplified());
                    }

                    if(strString.indexOf(';')>0)
                        break;
                    strString = ReadLine(hKlarfFile).simplified();
                    strValue = strString.section(';',0,0).remove('"').simplified();
                    strValue.section(" ",0,0).simplified().toInt(&bIsInt);
                }
            }
            // else ignore this record
        }
        else if(strSection.startsWith("ResultTimestamp", Qt::CaseInsensitive))
        {
            QString strDate = strValue.simplified().section(" ",0,0);
            strValue = strValue.simplified().section(" ",1).remove(";");
            QDate clDate(2000+strDate.section("-",2,2).toInt(),strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt());
            QTime clTime=QTime::fromString(strValue);
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }

            // goto end of record
        while((strString.indexOf(';')<0) && !hKlarfFile.atEnd())
            strString = ReadLine(hKlarfFile);

        strString = strString.section(';',1);
    }

    // Close file
    f.close();

    // Success parsing KLARF file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from KLARF data parsed
//////////////////////////////////////////////////////////////////////
bool CGKLARFtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;

    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing KLARF file into STDF database
        m_iLastError = errWriteSTDF;

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
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":KLARF";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");								// aux-file
    StdfFile.WriteString("");								// package-type
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
    StdfFile.WriteString("");								// Date-code
    StdfFile.WriteString("");								// Facility-ID
    StdfFile.WriteString("");								// FloorID
    StdfFile.WriteString(m_strProcessID.toLatin1().constData());	// ProcessID
    StdfFile.WriteString(m_strStepId.toLatin1().constData());		// OPER_FRQ
    StdfFile.WriteString("");								// Spec_nam
    StdfFile.WriteString("");								// Spec_ver
    StdfFile.WriteString("");								// flow_id
    StdfFile.WriteString(m_strSetupId.toLatin1().constData());		// SETUP_ID

    StdfFile.WriteRecord();

    int			iWaferX, iWaferY;
    long		iTotalGoodBin,iTotalFailBin;
    int			iPartNumber=0;
    int			iPartRetested=0;

    // Write WIR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);			// Start time
    StdfFile.WriteString(m_strWaferID.toLatin1().constData());				// WaferID
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);						// Wafer size
    StdfFile.WriteFloat(m_fDieHt);				// Height of die
    StdfFile.WriteFloat(m_fDieWid);				// Width of die
    StdfFile.WriteByte(0);						// Unknown Units
    StdfFile.WriteRecord();


    // Write all Parts result read on this wafer.: PIR....PRR
    iTotalGoodBin=iTotalFailBin=0;

    int iTotalParts = m_mapPartBinning.count();
    int iNextPart = 0;
    int iPartCount = 0;

    QMap<QString,QStringList*>::Iterator itPartInfo;
    for(itPartInfo=m_mapPartBinning.begin(); itPartInfo!=m_mapPartBinning.end(); itPartInfo++)
    {

        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if(GexProgressBar != NULL)
        {
            iPartCount++;
            while(iPartCount > iNextPart)
            {
                iProgressStep += 100/iTotalParts + 1;
                iNextPart  += iTotalParts/100 + 1;
                GexProgressBar->setValue(iProgressStep);
            }
        }
        QCoreApplication::processEvents();

        // Parse part list
        QString strXY;

        strXY = itPartInfo.key();
        iWaferX = strXY.section(" ",0,0).simplified().toInt();
        iWaferY = strXY.section(" ",1,1).simplified().toInt();

        QStringList *pPartBin=itPartInfo.value();
        if(pPartBin == NULL)
        {
            pPartBin = new QStringList;
            pPartBin->append(QString::number(m_nBinGood));
        }
        else
        {
            iPartRetested += pPartBin->count()-1;
        }

        // Part number
        iPartNumber++;

        QString	strBin;
        QStringList::Iterator itPartBin;
        for(itPartBin=pPartBin->begin(); itPartBin!=pPartBin->end(); itPartBin++)
        {
            strBin = *itPartBin;
            m_mapBinCnt[strBin.toInt()]++;

            // Write PIR for parts in this Wafer site
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);				// Test head
            StdfFile.WriteByte(1);				// Tester site
            StdfFile.WriteRecord();


            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site:1,2,3,4 or 5
            if(strBin == QString::number(m_nBinGood))
            {
                StdfFile.WriteByte(0);			// PART_FLG : PASSED
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);			// PART_FLG : FAILED
                iTotalFailBin++;
            }
            StdfFile.WriteWord((WORD)0);		// NUM_TEST
            StdfFile.WriteWord(strBin.toInt()); // HARD_BIN
            StdfFile.WriteWord(strBin.toInt()); // SOFT_BIN
            StdfFile.WriteWord(iWaferX);		// X_COORD
            StdfFile.WriteWord(iWaferY);		// Y_COORD
            StdfFile.WriteDword(0);				// No testing time known...
            StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
            StdfFile.WriteString("");			// PART_TXT
            StdfFile.WriteString("");			// PART_FIX
            StdfFile.WriteRecord();
        }


    }

    // Write WRR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
    StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
    StdfFile.WriteDword(iPartRetested);			// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword(4294967295UL);			// Functionnal Parts

    StdfFile.WriteString(m_strWaferID.toLatin1().constData());				// WaferID
    StdfFile.WriteRecord();

    // WCR
    char    cOrientation = ' ';
    if (m_strNotch.compare("DOWN") == 0)
    {
        cOrientation='D';
    }
    else if (m_strNotch.compare("LEFT") == 0)
    {
        cOrientation='L';
    }
    else if (m_strNotch.compare("RIGHT") == 0)
    {
        cOrientation='R';
    }
    else if (m_strNotch.compare("UP") == 0)
    {
        cOrientation='U';
    }

    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);						// Wafer size
    StdfFile.WriteFloat((float) m_fDieHt);		// Height of die
    StdfFile.WriteFloat((float) m_fDieWid);		// Width of die
    StdfFile.WriteByte(0);                      // Units are in millimeters
    StdfFile.WriteByte((BYTE) cOrientation);	// Orientation of wafer flat
    StdfFile.WriteWord((WORD)-32768);           // X coordinate undefined
    StdfFile.WriteWord((WORD)-32768);           // Y coordinate undefined
    StdfFile.WriteByte(mPosX);                  // POSX
    StdfFile.WriteByte(mPosY);                  // POSY
    StdfFile.WriteRecord();

    // Write SBR Bin
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    QMap<int,QString>::Iterator itBinInfo;
    for(itBinInfo=m_mapBinName.begin(); itBinInfo!=m_mapBinName.end(); itBinInfo++)
    {
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);					// Test Head = ALL
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteWord(itBinInfo.key());		// SBIN = 0
        StdfFile.WriteDword(m_mapBinCnt[itBinInfo.key()]);// Total Bins
        if(itBinInfo.key() == m_nBinGood)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(m_mapBinName[itBinInfo.key()].toLatin1().constData());// Bin name
        StdfFile.WriteRecord();
    }


    // Write HBR Bin
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;

    for(itBinInfo=m_mapBinName.begin(); itBinInfo!=m_mapBinName.end(); itBinInfo++)
    {
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);					// Test Head = ALL
        StdfFile.WriteByte(255);					// Test sites = ALL
        StdfFile.WriteWord(itBinInfo.key());						// SBIN = 0
        StdfFile.WriteDword(m_mapBinCnt[itBinInfo.key()]);			// Total Bins
        if(itBinInfo.key() == m_nBinGood)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(m_mapBinName[itBinInfo.key()].toLatin1().constData());// Bin name
        StdfFile.WriteRecord();
    }


    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(0);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' KLARF file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGKLARFtoSTDF::Convert(const char *KlarfFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(KlarfFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    bool bHideProgressAfter=true;
    bool bHideLabelAfter=false;

    iProgressStep = 0;
    iNextFilePos = 0;

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(KlarfFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }

    if(GexProgressBar != NULL)
    {
        bHideProgressAfter = GexProgressBar->isHidden();
        GexProgressBar->setMaximum(200);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }
    QCoreApplication::processEvents();

    if(ReadKlarfFile(KlarfFileName) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading KLARF file
    }

    iNextFilePos = 0;

    if(WriteStdfFile(strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        QFile::remove(strFileNameSTDF);
        return false;
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
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGKLARFtoSTDF::ReadLine(QTextStream& hFile)
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
            GexProgressBar->setValue(iProgressStep);
        }
    }
    QCoreApplication::processEvents();

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}
