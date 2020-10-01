//////////////////////////////////////////////////////////////////////
// import_STIF.cpp: Convert a STIF.inf file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>


#include "import_stif.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//WM - V1.1 - STMicroelectronics Wafer Map File
//
//LOT	P0060255_041
//WAFER	01
//PRODUCT	XF56186-1
//READER	M23871-01-C4
//XSTEP	893	UNITS	(0.1)MIL
//YSTEP	801	UNITS	(0.1)MIL
//FLAT	0
//XREF	-1340	UNITS	(0.1)MIL
//YREF	400	UNITS	(0.1)MIL
//XFRST	40
//YFRST	47
//PRQUAD	3
//COQUAD	3
//DIAM	7874
//XSTRP	0
//YSTRP	93
//NULBC	126
//GOODS	5968
//DATE	2010-08-29
//TIME	13:47:51
//RPSEL	0
//SETUP FILE	XF56186-1
//TEST SYSTEM	NSX1730
//OPERATOR	q
//PROBE CARD	XF56186-1 TH01_KLALF
//PROBER	NSX1730
//
//WMXDIM=84
//WMYDIM=94
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~              ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~                            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~                                ~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~                                    ~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~                                        ~~~~~~~~~~~~~~~~~~~~~~

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


CStifBinInfo::CStifBinInfo(const QString& strBin)
{
    int iBin = (int) strBin.at(0).toLatin1();
    if(iBin < 0)
        iBin += 256;

    nBinNb = iBin;
    nNbCnt = 0;
    bPass = iBin>=128;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSTIFtoSTDF::CGSTIFtoSTDF()
{
    m_lStartTime = 0;
    m_nWaferColumns = m_nWaferRows = 0;
    m_nFlatNotch = 0;
    m_nDiesXSize = m_nDiesYSize = 0;
    m_nStepX = m_nStepY = 1;
    m_nRefDieX = m_nRefDieY = 0;
    m_nUpperLeftX = m_nUpperLeftY = 0;
    m_fWaferSize = 0.0;
    m_nDiesXSize = m_nDiesYSize = 0;
    m_nDiesUnit = 0;
    m_fDiesScaleUnit = 0.0;

    m_strLastError = m_strLastMessage = "";
    m_iLastError = errNoError;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSTIFtoSTDF::~CGSTIFtoSTDF()
{
    QMap<QString,CStifBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
    m_qMapBins.clear();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSTIFtoSTDF::GetLastError()
{
    m_strLastError = "Import STIF: ";

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
            m_strLastError += "Invalid file format.\n";
            m_strLastError += m_strLastMessage;
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
// Check if File is compatible with STIF format
//////////////////////////////////////////////////////////////////////
bool CGSTIFtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;
    bool	bIsCompatible = false;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hSTIFFile(&f);

    while(!hSTIFFile.atEnd())
    {
        do
            strSection = hSTIFFile.readLine().simplified();
        while(!strSection.isNull() && strSection.isEmpty());

        strSection = strSection.toUpper();

        if(strSection.startsWith("WM - V")
        && strSection.endsWith(" - STMICROELECTRONICS WAFER MAP FILE"))
        {
            bIsCompatible = true;
            break;
        }
        else
        {
            // Incorrect header...this is not a STIF file!
            bIsCompatible = false;
            break;
        }
        strSection = "";
    }

    f.close();

    return bIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the STIF file
//////////////////////////////////////////////////////////////////////
bool CGSTIFtoSTDF::ReadSTIFFile(const char* STIFFileName,
                              const char* strFileNameSTDF)
{
    QString strLine;
    QString strSection;
    QString strValue;

    // Open CSV file
    QFile f( STIFFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening STIF file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();

    // Assign file I/O stream
    QTextStream hSTIFFile(&f);
    hSTIFFile.setCodec("ISO 8859-1");
    m_strNullBinCode = "~";
    m_nStepX = m_nStepY = 1;

    //WM - V1.1 - STMicroelectronics Wafer Map File
    strLine = ReadLine(hSTIFFile).simplified().toUpper();
    if(!strLine.endsWith("STMICROELECTRONICS WAFER MAP FILE"))
    {
        // Incorrect header...this is not a STIF file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    if(!strLine.startsWith("WM - V1.0") && !strLine.startsWith("WM - V1.1"))
    {
        // Incorrect header...this is not a STIF file!
        m_iLastError = errInvalidFormat;
        m_strLastMessage = "Version not supported ("+strLine+").";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    //LOT Lot number (on 7 or 8 digits)
    //WAFER Wafer number (on 2 digits)
    //PRODUCT Product name
    //READER Number read by the OCR as written on the wafer
    //XSTEP X die stepping followed by the unit used
    //YSTEP Y die stepping followed by the unit used
    //FLAT Flat or notch orientation in degrees (0,90,180,270), clockwise
    //XREF X distance from the wafer centre to the reference die centre, followed by the unit used
    //YREF Y distance from the wafer centre to the reference die centre, followed by the unit used
    //XFRST X index location of the reference die
    //YFRST Y index location of the reference die
    //XSTRP X index location of the first location of die in the wafer map (location in the upper left corner)
    //YSTRP Y index location of the first location of die in the wafer map (location in the upper left corner)
    //PRQUAD Probing quadrant used (1,2,3,4)
    //COQUAD Coordinate quadrant used (1,2,3,4)
    //DIAM Wafer size in mils
    //NULBC ASCII code number of the nul bin code.
    //GOODS Quantity of good dice, computed from wafer map
    //SETUP FILE Access path and file name of the set up file used by prober
    //RPSEL Number of picking alignment reference dice
    //DATE Start probing date (YYYY-MM-DD)
    //TIME Start probing time (hh:mm:ss)
    //OPERATOR Operator identification
    //TEST SYSTEM Tester identification
    //TEST PROG Tester program name
    //PROBE CARD Prob card used
    //PROBER Prober name
    //TEST DATA User supplied about test
    //XREFPn X index location of the reference point n
    //YREFPn Y index location of the reference point n
    //UNSUREBC ASCII code number of dice that are not systematically reliable in term of alignment
    //MERGEDATE Date of the merge operation on the wafer map file
    //MERGETIME Time of the merge operation on the wafer map file

    QDate clDate;
    QTime clTime;

    while(!hSTIFFile.atEnd())
    {
        strLine = ReadLine(hSTIFFile);
        strValue = strLine.section("\t",1).simplified();
        strSection = strLine.section("\t",0,0).simplified().toUpper();

        if(strSection.isEmpty())
            continue;

        if(strSection == "NULBC")
        {//NULBC	126
            // ASCII CODE
            m_strNullBinCode=QByteArray(1,strValue.toInt()).data();
        }
        else if(strSection == "LOT")
        {//LOT	B88280
            m_strLotId=strValue;
        }
        else if(strSection == "WAFER")
        {//WAFER	02
            m_strWaferId=strValue;
        }
        else if(strSection == "READER")
        {//READER	B88280-02
            m_strSubLotId=strValue;
            if(m_strLotId.isEmpty())
                m_strLotId = m_strSubLotId.left(9).remove("-");
            if(m_strWaferId.isEmpty())
                m_strWaferId = m_strSubLotId.mid(10,2);
        }
        else if(strSection == "PRODUCT")
        {//PRODUCT	XF50661_2C_PROD
            m_strDevice=strValue;
        }
        else if(strSection == "SETUP FILE")
        {//SETUP FILE	XF50661_2C_PROD
            m_strAuxFile=strValue;
        }
        else if(strSection == "OPERATOR")
        {//OPERATOR	AugTech
            m_strOperator=strValue;
        }
        else if(strSection == "TEST SYSTEM")
        {//TEST SYSTEM	NSX1746
            m_strTesterType=strValue;
        }
        else if(strSection == "TEST PROG")
        {//TEST PROG
            m_strJobName=strValue;
        }
        else if(strSection == "PROBE CARD")
        {//PROBE CARD	XF50661_2C_PROD CMOS-SSSL-98%
            m_strCardId=strValue;
        }
        else if(strSection == "PROBER")
        {//PROBER	NSX1746
            m_strHandId=strValue;
        }
        else if(strSection == "FLAT")
        {//FLAT	0
            m_nFlatNotch=strValue.toInt();
        }
        else if(strSection == "DIAM")
        {//DIAM	5905
            m_fWaferSize=strValue.toInt();
        }
        else if(strSection == "XSTEP")
        {//XSTEP	1312	UNITS	(0.1)MIL
            m_nDiesXSize = strValue.section(" ",0,0).toInt();
            m_nDiesUnit = 4;
            m_fDiesScaleUnit = strValue.section("(",1).section(")",0,0).toFloat();
        }
        else if(strSection == "YSTEP")
        {//YSTEP	1312	UNITS	(0.1)MIL
            m_nDiesYSize = strValue.section(" ",0,0).toInt();
            m_nDiesUnit = 4;
            m_fDiesScaleUnit = strValue.section("(",1).section(")",0,0).toFloat();
        }
        else if(strSection == "XFRST")
        {//XFRST	21
            m_nRefDieX=strValue.toInt();
        }
        else if(strSection == "YFRST")
        {//YFRST	21
            m_nRefDieY=strValue.toInt();
        }
        else if(strSection == "COQUAD")
        {//COQUAD	4
            //COQUAD Value	X grows to	Y grows to
            //1				Left		Down
            //2				Right		Down
            //3				Right		Up
            //4				Left		Up
            if(strValue == "1")
                m_nStepX = -1;
            if(strValue == "3")
                m_nStepY = -1;
            if(strValue == "4")
                m_nStepX = m_nStepY = -1;

        }
        else if(strSection == "XSTRP")
        {//XSTRP	27
            m_nUpperLeftX=strValue.toInt();
        }
        else if(strSection == "YSTRP")
        {//YSTRP	24
            m_nUpperLeftY=strValue.toInt();
        }
        else if(strSection == "DATE")
        {//DATE	2003-10-20
            clDate = QDate(strValue.section("-",0,0).toInt(),strValue.section("-",1,1).toInt(),strValue.section("-",2,2).toInt());
        }
        else if(strSection == "TIME")
        {//TIME	04:42:09
            clTime = QTime(strValue.section(":",0,0).toInt(),strValue.section(":",1,1).toInt(),strValue.section(":",2,2).toInt());
        }
        else if((strSection == "READER")
                ||(strSection == "XSTEP")
                ||(strSection == "YSTEP")
                ||(strSection == "XREF")
                ||(strSection == "YREF")
                ||(strSection == "PRQUAD")
                ||(strSection == "GOODS")
                ||(strSection == "RPSEL")
                ||(strSection == "OLIFORMAT")
                ||(strSection == "OLIPATH")
                ||(strSection == "MERGEDATE")
                ||(strSection == "MERGETIME")
                ||(strSection == "TEST DATA")
                ||(strSection == "XREFPn")
                ||(strSection == "YREFPn")
                ||(strSection == "UNSUREBC"))
        {
            // ignore
        }
        else if(strSection.startsWith("WMXDIM") || strSection.startsWith("WMYDIM"))
        {
            //Wafer Map Start here
            break;
        }
        else
        {
            // Incorrect header...this is not a STIF file!
            m_iLastError = errInvalidFormat;
            m_strLastMessage = "Keyword not supported ("+strSection+").";

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }
    }

    // Check the date
    if(!clDate.isValid())
    {
        // no wafer size
        // Incorrect header...this is not a STIF file!
        m_iLastError = errInvalidFormat;
        m_strLastMessage = "Invalid Date (Keyword DATE not present or Date format not supported).";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    QDateTime clDateTime(clDate,clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    // Wafer size
    if(!strLine.startsWith("WMXDIM"))
    {
        // no wafer size
        // Incorrect header...this is not a STIF file!
        m_iLastError = errInvalidFormat;
        m_strLastMessage = "Keyword WMXDIM is not present (Width of the map in dice).";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_nWaferColumns = strLine.section("=",1).simplified().toInt();

    strLine = ReadLine(hSTIFFile);
    if(!strLine.startsWith("WMYDIM"))
    {
        // no wafer size
        // Incorrect header...this is not a STIF file!
        m_iLastError = errInvalidFormat;
        m_strLastMessage = "Keyword WMYDIM is not present (Height of the map in dice).";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_nWaferRows = strLine.section("=",1).simplified().toInt();


    //~~~~~~~~~~~~~~~~~~~~~  ~~~~~~~~~~~~~~~~~~~~~
    // have a map to save
    // write STDF file
    // Loop reading file until end is reached & generate STDF file dynamically.
    if(!WriteStdfFile(&hSTIFFile,strFileNameSTDF))
    {
        QFile::remove(strFileNameSTDF);
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from STIF data parsed
//////////////////////////////////////////////////////////////////////
bool CGSTIFtoSTDF::WriteStdfFile(QTextStream *hSTIFFile, const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
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
    StdfFile.WriteString(m_strLotId.toLatin1().constData());	// Lot ID
    StdfFile.WriteString(m_strDevice.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());					// sublot-id
    StdfFile.WriteString(m_strOperator.toLatin1().constData());	// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":STIF";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString(m_strAuxFile.toLatin1().constData());		// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID
    StdfFile.WriteRecord();

    // Write SDR for last wafer inserted
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 80;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);				// Test Head = ALL
    StdfFile.WriteByte((BYTE)1);			// head#
    StdfFile.WriteByte((BYTE)1);			// Group#
    StdfFile.WriteByte((BYTE)0);			// site_count
    //StdfFile.WriteByte((BYTE)1);			// array of test site#
    StdfFile.WriteString("");				// HAND_TYP: Handler/prober type
    StdfFile.WriteString(m_strHandId.toLatin1().constData());	// HAND_ID: Handler/prober name
    StdfFile.WriteString("");				// CARD_TYP: Probe card type
    StdfFile.WriteString(m_strCardId.toLatin1().constData());	// CARD_ID: Probe card name
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString		strString, strAsciiBin;
    int			iIndexCol, iIndexRows;				// Loop index
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
  long iPartNumber;
  //FIXME: not used ?
  //long iTotalTests;
  //bool bPassStatus;
    int			iWaferRows;
    int			iWaferColumns;

    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=iIndexRows=0;

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);							// Test head
    StdfFile.WriteByte(255);						// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);				// Start time
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    iWaferRows = m_nUpperLeftY-m_nStepY;

    // Write all Parameters read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
    while(hSTIFFile->atEnd() == false)
    {
        // check the end of the wafer map
        if(m_nWaferRows == iIndexRows)
            break;

        // Read line
        strString = ReadLine(*hSTIFFile);
        if(strString == "\r")
            continue;
        if(strString.isEmpty())
            continue;

        iIndexRows++;
        iWaferRows += m_nStepY;
        iWaferColumns = m_nUpperLeftX-m_nStepX;

        // Reset Pass/Fail flag.
    //FIXME: not used ?
    //bPassStatus = true;

        // Reset counters
    //FIXME: not used ?
    //iTotalTests = 0;

        // Read Parameter results for this record
        for(iIndexCol=0;iIndexCol<m_nWaferColumns;iIndexCol++)
        {
            strAsciiBin = strString.mid(iIndexCol,1);
            // Check the end of the line
            if(strAsciiBin.isEmpty())
                break;
            if(strAsciiBin == "\r")
                break;

            iWaferColumns += m_nStepX;

            // Check if have to ignore this die
            if(strAsciiBin == m_strNullBinCode)
                continue;

            iPartNumber++;

            // Write PIR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site#:1
            StdfFile.WriteRecord();

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site#:1
            if(!m_qMapBins.contains(strAsciiBin))
            {
                CStifBinInfo *pBin = new CStifBinInfo(strAsciiBin);
                m_qMapBins[strAsciiBin] = pBin;
            }
            if(!m_qMapBins[strAsciiBin]->bPass)
            {
                StdfFile.WriteByte(8);		// PART_FLG : FAILED
                iTotalFailBin++;
                wHardBin = wSoftBin = m_qMapBins[strAsciiBin]->nBinNb;
                m_qMapBins[strAsciiBin]->nNbCnt++;
            }
            else
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                wHardBin = wSoftBin = m_qMapBins[strAsciiBin]->nBinNb;
                m_qMapBins[strAsciiBin]->nNbCnt++;
                iTotalGoodBin++;
            }
            StdfFile.WriteWord((WORD)0);			// NUM_TEST
            StdfFile.WriteWord(wHardBin);           // HARD_BIN
            StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
            StdfFile.WriteWord(iWaferColumns);		// X_COORD
            StdfFile.WriteWord(iWaferRows);			// Y_COORD
            StdfFile.WriteDword(0);					// No testing time known...
            StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
            StdfFile.WriteString("");				// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();
        };
    };			// Read all lines with valid data records in file

    QString strValue;
    QDate clDate;
    QTime clTime;
    while(hSTIFFile->atEnd() == false)
    {
        // check the end of the wafer map
        if(m_nWaferRows == iWaferRows)
            break;

        // Read line
        strString = ReadLine(*hSTIFFile);
        if(strString == "\r")
            continue;
        if(strString.isEmpty())
            continue;

        strValue = strString.section("\t",1);

        if(strString.startsWith("EDATE"))
            clDate = QDate(strValue.section("-",0,0).toInt(),strValue.section("-",1,1).toInt(),strValue.section("-",2,2).toInt());
        else
        if(strString.startsWith("ETIME"))
            clTime = QTime(strValue.section(":",0,0).toInt(),strValue.section(":",1,1).toInt(),strValue.section(":",2,2).toInt());
    }

    if(clDate.isValid())
    {
        QDateTime clDateTime(clDate,clTime);
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStopTime = clDateTime.toTime_t();
    }
    else
        m_lStopTime = m_lStartTime;

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStopTime);			// Time of last part tested
    StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    // Write WCR for last wafer inserted
    float fValue;
    char	cOrientation = ' ';
    if(m_nFlatNotch==0) cOrientation='D';
    else if(m_nFlatNotch==90) cOrientation='L';
    else if(m_nFlatNotch==180) cOrientation='U';
    else if(m_nFlatNotch==270) cOrientation='R';
    char cPosX = 'R';
    char cPosY = 'D';
    if(m_nStepX < 0) cPosX = 'L';
    if(m_nStepY < 0) cPosY = 'U';


    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(m_fWaferSize);					// Wafer size
    fValue = (float)m_nDiesXSize;
    StdfFile.WriteFloat(fValue*m_fDiesScaleUnit);		// Height of die
    fValue = (float)m_nDiesYSize;
    StdfFile.WriteFloat(fValue*m_fDiesScaleUnit);		// Width of die
    StdfFile.WriteByte(m_nDiesUnit);					// Units
    StdfFile.WriteByte((BYTE) cOrientation);			// Orientation of wafer flat
  StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_X
  StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_Y
    StdfFile.WriteByte((BYTE)cPosX);					// POS_X
    StdfFile.WriteByte((BYTE)cPosY);					// POS_Y

    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<QString,CStifBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(1);							// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value()->nBinNb);	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
        if(itMapBin.value()->bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString((itMapBin.key()).toLatin1().constData());
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(1);							// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value()->nBinNb);	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
        if(itMapBin.value()->bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString((itMapBin.key()).toLatin1().constData());
        StdfFile.WriteRecord();
    }


    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStopTime);		// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' STIF file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSTIFtoSTDF::Convert(const char *STIFFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(STIFFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;

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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(STIFFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadSTIFFile(STIFFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading STIF file
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
QString CGSTIFtoSTDF::ReadLine(QTextStream& hFile)
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
