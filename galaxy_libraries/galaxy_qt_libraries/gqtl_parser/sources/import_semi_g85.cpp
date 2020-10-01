//////////////////////////////////////////////////////////////////////
// import_semi_g85.cpp: Convert a SEMI_G85 file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>

#include "import_semi_g85.h"
#include "importConstants.h"

//WAFER_MAP = {
//WAFER_ID = "B83541.1-01-D6"
//MAP_TYPE = "ASCII"
//NULL_BIN = "."
//ROWS = 30
//COLUMNS = 47
//FLAT_NOTCH = 0
//CUSTOMER_NAME = "ADI"
//FORMAT_REVISION = "ADI0811D"
//SUPPLIER_NAME = "AMS"
//LOT_ID = "B83541.1"
//WAFER_SIZE = 200
//X_SIZE = 4075
//Y_SIZE = 6015
//REF_DIES = 1
//REF_DIE = 36 -2
//DIES = 1114
//BINS = 13
//BIN = "1" 956   "Pass" ""
//BIN = "3" 2     "Fail" ""
// ..
//BIN = "Q" 1     "Fail" ""
//BIN = "Z" 1     "Fail" "Reference Die"
//MAP = {
//....................1111181....................
//...............11H11111111111111...............

namespace GS
{
namespace Parser
{


ParserBinningG85::ParserBinningG85(QChar charBin) : ParserBinning()
{
    if(charBin.isLetter())
    {
        // A = ascii(65)
        // a = ascii(97)
        mBinNumber = charBin.unicode() - 55;
    }
    else
        mBinNumber = charBin.unicode() - 48;

    mBinCount=0;
    mPassFail=false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSEMI_G85toSTDF::CGSEMI_G85toSTDF() : ParserBase(typeSemi_G85, "SEMI_G85")
{
    mStartTime = 0;
    m_nWaferNb = 0;
    m_nWaferRows = 0;
    m_nWaferColumns = 0;
    m_nFlatNotch = 0;
    m_cPosX			= 'R';
    m_cPosY			= 'U';
    m_nWaferSize = 0;
    m_nXSize = 0;
    m_nYSize = 0;
    m_nNbRefDies = 0;
    m_nNbDies = 0;
    m_nNbBins = 0;
    m_nRefDieX = 0;
    m_nRefDieY = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSEMI_G85toSTDF::~CGSEMI_G85toSTDF()
{
    QMap<QChar,ParserBinningG85*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SEMI_G85 format
//////////////////////////////////////////////////////////////////////
bool CGSEMI_G85toSTDF::IsCompatible(const QString &FileName)
{
    QString strString;
    bool	bIsCompatible = false;

    // Open hCsmFile file
    QFile f( FileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hSemi_G85File(&f);

    // Check if first N line is the correct SEMI_G85 header...
    int	nLine = 0;

    while(!hSemi_G85File.atEnd())
    {
        strString = hSemi_G85File.readLine().simplified();

        if((strString.startsWith("WAFER_MAP = {",Qt::CaseInsensitive))
        || (strString.startsWith("LOT_SUMMARY = {",Qt::CaseInsensitive)))
        {
            bIsCompatible = true;
            break;
        }


        nLine++;

        if(nLine > 30)
            break;
    }

    // Close file
    f.close();

    return bIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SEMI_G85 file
//////////////////////////////////////////////////////////////////////
bool CGSEMI_G85toSTDF::ConvertoStdf(const QString &Semi_G85FileName, QString &StdfFileName)
{
    QString strString;
    QString strSection;
    QString strValue;

    // Open CSV file
    QFile f( Semi_G85FileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SEMI_G85 file
        m_iLastError = errOpenFail;
        m_strLastErrorSpecification = f.errorString();

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream lSemi_G85File(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
   /* mProgressStep = 0;
    mNextFilePos = 0;
    mFileSize = f.size() + 1;*/

    // Check if first line is the correct SEMI_G85 header...
    int iPos;
    strSection = f.fileName();
    iPos = strSection.lastIndexOf("/");
    if(iPos < strSection.lastIndexOf("\\"))
        iPos = strSection.lastIndexOf("\\");
    strSection = strSection.mid(iPos+1);
    m_strFabLotId = strSection.left(strSection.lastIndexOf("."));
    m_strWaferId = strSection.right(strSection.length()-strSection.lastIndexOf(".")-1);

    while(!lSemi_G85File.atEnd())
    {
        strSection = ReadLine(lSemi_G85File).simplified();

        if(strSection.startsWith("WAFER_MAP = {",Qt::CaseInsensitive))
            break;

        if(strSection.startsWith("LOT_SUMMARY = {",Qt::CaseInsensitive))
        {
            // Ignore this section
            // Go to WAFER_MAP marker

            bool bSaveWaferSummarySection = true;
            bool bInWaferSummarySection = false;
            // Save DateTime information for each wafer
            //wafer        1      2      3      4      5      6      7     11     14    dpw    date     time  yield
            //1         873      4     10     15     19      0      0     20     20     961  12-11-08  16:49  90.8%

            while(!lSemi_G85File.atEnd())
            {
                strSection = ReadLine(lSemi_G85File).simplified();

                if(strSection.startsWith("WAFER_MAP = {",Qt::CaseInsensitive))
                    break;

                if(bSaveWaferSummarySection)
                {
                    strString = strSection.simplified();

                    if(bInWaferSummarySection)
                    {
                        if(!strString.endsWith("%"))
                        {
                            bInWaferSummarySection = false;
                            bSaveWaferSummarySection = false;
                        }
                        else
                        {
                            int iPos = strString.count(" ");

                            m_lstDateTime.append(strString.section(" ",iPos-2,iPos-1));
                        }
                    }

                    if(strString.section(" ",0,0).toUpper() == "WAFER")
                    {
                        if(!strString.endsWith("date time yield",Qt::CaseInsensitive))
                            bSaveWaferSummarySection = false;

                        bInWaferSummarySection = true;
                    }

                }
            }
            break;
        }

        // Else collect some additionated information
        strValue = strSection.section(":",1).trimmed();
        strSection = strSection.left(strSection.indexOf(":")).trimmed().toUpper();

        if(strSection == "DEVICE_NAME")
        {
            //Device_Name       : AR8316
            m_strDeviceId = strValue;
        }
        else
        if(strSection == "LOT_ID")
        {
            //Lot_ID            : BA7311
            m_strLotId = strValue;
        }
        else
        if(strSection == "WAFER QTY")
        {
            //Wafer QTY         : 25
            m_nWaferNb = strValue.toInt();
        }
        else
        if(strSection == "TESTER_ID")
        {
            //Tester_ID         : TS-J75-001
            m_strTesterType = strValue;
        }
        else
        if(strSection == "PROBER_ID")
        {
            //Prober_ID         : PB-UF3-001
            m_strHandlerId = strValue;
        }
        else
        if(strSection == "OPERATOR_ID")
        {
            //Operator_ID       : SH01924
            m_strOperatorId = strValue;
        }
        else
        if(strSection == "PROBER_CARD")
        {
            //Prober_Card       : AR8316x1-001
            m_strHandlerType = strValue;
        }
        else
        if(strSection == "PROGRAM_NAME")
        {
            //Program_Name      : S16_J75_WS_single_081210.xls
            m_strJobName = strValue;
        }
        else
        if(strSection == "PROGRAM_REV")
        {
            //Program_Rev.      : A00
            m_strJobRev = strValue;
        }
        else
        if(strSection == "TEMPERATURE")
        {
            //Temperature       : 25 C
            m_strTestTemp = strValue;
        }
        else
        if(strSection == "TEST_SITE")
        {
            //Test_Site         : ATC
            m_strNodeName = strValue;
        }

    }



    if(strSection.startsWith("WAFER_MAP = {") == false)
    {
        // Incorrect header...this is not a SEMI_G85 file!
        m_iLastError = errInvalidFormatParameter;
        m_strLastErrorSpecification = "'WAFER_MAP = {' is missing";
        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // have a wafermap to save
    // write STDF file
    // Loop reading file until end is reached & generate STDF file dynamically.
    if(!WriteStdfFile(lSemi_G85File,StdfFileName))
    {
        QFile::remove(StdfFileName);
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();
    return true;

}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SEMI_G85 data parsed
//////////////////////////////////////////////////////////////////////
bool CGSEMI_G85toSTDF::WriteStdfFile(QTextStream &Semi_G85File, const QString &StdfFileName)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        m_iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    bool bStdfHeaderWriten = false;

    // Write Test results for each line read.
    QString		strString, strSection, strValue;
    int			iIndex;				// Loop index
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
        long iPartNumber;
        //FIXME: not used ?
        //long iTotalTests;
        //bool bPassStatus;

    while(!Semi_G85File.atEnd())
    {

        strSection = ReadLine(Semi_G85File).simplified();
        strValue = strSection.section(" =",1).trimmed();
        strSection = strSection.left(strSection.indexOf(" =")).trimmed().toUpper();

        // We have a WAFER_MAP = { marker
        if(strSection == "WAFER_MAP")
        {
            m_nWaferRows = 0;
            m_nWaferColumns = 0;
            m_nFlatNotch = 0;
            m_nNbRefDies = 0;
            m_nNbDies = 0;
            m_nNbBins = 0;
            m_nRefDieX = 0;
            m_nRefDieY = 0;

            // Force to default Pos
            // Semi G85
            // posX = 'R'
            // poxY = 'U'
            // WaferLeftUpCorner = 0.0
            m_cPosX			= 'R';
            m_cPosY			= 'U';

            continue;
        }

        if(strSection.startsWith("}"))
            continue;

        if(strSection == "WAFER_ID")
        {//WAFER_ID = "BA7311-1"
            m_strWaferId=strValue.remove('"');
        }
        else if(strSection == "WAFER_OCR")
        {//WAFER_OCR = "BA7311-01-E0"
            m_strWaferId=strValue.remove('"');
        }
        else if(strSection == "DEVICE")
        {//DEVICE =
            m_strDeviceId=strValue.remove('"');
        }
        else if(strSection == "PACKAGE")
        {//PACKAGE =
            m_strPackageId=strValue.remove('"');
        }
        else if(strSection == "NULL_BIN")
        {//NULL_BIN = "."
            m_strNullBin=strValue.remove('"');
        }
        else if(strSection == "PROBE_DATE")
        {//PROBE_DATE = 20061019084620
            QDate cDate(strValue.left(4).toInt(),strValue.mid(4,2).toInt(),strValue.mid(6,2).toInt());
            QTime cTime(strValue.mid(8,2).toInt(),strValue.mid(10,2).toInt(),strValue.right(2).toInt());
            QDateTime clDateTime(cDate,cTime);
            clDateTime.setTimeSpec(Qt::UTC);
            mStartTime = clDateTime.toTime_t();
        }
        else if(strSection == "ROWS")
        {//ROWS = 30
            m_nWaferRows=strValue.toInt();
        }
        else if(strSection == "COLUMNS")
        {//COLUMNS = 47
            m_nWaferColumns=strValue.toInt();
        }
        else if(strSection == "FLAT_NOTCH")
        {//FLAT_NOTCH = 0
            m_nFlatNotch=strValue.toInt();
        }
        else if(strSection == "CUSTOMER_NAME")
        {//CUSTOMER_NAME = "ADI"
            m_strCustomerName=strValue.remove('"');
        }
        else if(strSection == "FORMAT_REVISION")
        {//FORMAT_REVISION = "ADI0811D"
            m_strFormatRevision=strValue.remove('"');
        }
        else if(strSection == "SUPPLIER_NAME")
        {//SUPPLIER_NAME = "AMS"
            m_strSupplierName=strValue.remove('"');
        }
        else if(strSection == "LOT_ID")
        {//LOT_ID = "B83541.1"
            m_strLotId=strValue.remove('"');
            if(m_strWaferId.section("-",0,0) == m_strLotId)
                m_strWaferId = m_strWaferId.section("-",1);
        }
        else if(strSection == "WAFER_SIZE")
        {//WAFER_SIZE = 200
            m_nWaferSize=strValue.toInt();
        }
        else if(strSection == "X_SIZE")
        {//X_SIZE = 4075
            m_nXSize=strValue.toInt();
        }
        else if(strSection == "Y_SIZE")
        {//Y_SIZE = 6015
            m_nYSize=strValue.toInt();
        }
        else if(strSection == "REF_DIES")
        {//REF_DIES = 1
            //m_nNbRefDies=strValue.toInt();
        }
        else if(strSection == "REF_DIE")
        {//REF_DIE = 36 -2
            strSection=strValue.remove('"').trimmed();
            m_nRefDieX=strSection.section(" ",0,0).toInt();
            m_nRefDieY=strSection.section(" ",1).toInt();
        }
        else if(strSection == "DIES")
        {//DIES = 1114
            m_nNbDies=strValue.toInt();
        }
        else if(strSection == "BINS")
        {//BINS = 13
            m_nNbBins=strValue.toInt();
        }
        else if(strSection == "BIN")
        {//BIN =  "Z" 1     "Fail" "Reference Die"
            QChar	charBin;
            ParserBinningG85 *pNewBinInfo;
            charBin = strValue.section(" ",0,0).remove('"').toUpper().at(0);
            if(!m_qMapBins.contains(charBin))
            {
                int iIndex = 2;
                // Skip Yield information
                if(strValue.section(" ",iIndex,iIndex).remove('"').endsWith("%"))
                {
                    //BIN ="1"	   873  90.8% "Pass" "Pass"
                    iIndex++;
                }

                pNewBinInfo = new ParserBinningG85(charBin);
                pNewBinInfo->SetPassFail(strValue.section(" ",iIndex,iIndex).remove('"').startsWith("fail", Qt::CaseInsensitive) ? false : true);
                iIndex++;
                pNewBinInfo->SetBinName(strValue.section(" ",iIndex).remove('"'));

                pNewBinInfo->SetBinCount(0);

                m_qMapBins[charBin] = pNewBinInfo;
            }
        }
        else if(strSection == "MAP")
        {//MAP = {


            if(!bStdfHeaderWriten)
            {
                bStdfHeaderWriten = true;

                if((mStartTime < 1) && !m_lstDateTime.isEmpty())
                {// 12-11-08  16:49

                    strValue = m_lstDateTime.first();

                    QDate cDate(strValue.mid(6,2).toInt()+2000,strValue.left(2).toInt(),strValue.mid(3,2).toInt());
                    QTime cTime(strValue.mid(9,2).toInt(),strValue.mid(12,2).toInt());

                    QDateTime clDateTime(cDate,cTime);
                    clDateTime.setTimeSpec(Qt::UTC);
                    mStartTime = clDateTime.toTime_t();
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
                StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
                StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
                StdfFile.WriteString(m_strNodeName.toLatin1().constData());	// Node name
                StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
                StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
                StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
                StdfFile.WriteString("");					// sublot-id
                StdfFile.WriteString(m_strOperatorId.toLatin1().constData());	// operator
                StdfFile.WriteString("");					// exec-type
                StdfFile.WriteString("");					// exe-ver
                StdfFile.WriteString("WAFER");				// test-cod
                StdfFile.WriteString(m_strTestTemp.toLatin1().constData());	// test-temperature
                // Construct custom Galaxy USER_TXT
                QString	strUserTxt;
                strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
                strUserTxt += ":";
                strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
                strUserTxt += ":Semi G85";
                StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
                StdfFile.WriteString("");					// aux-file
                StdfFile.WriteString(m_strPackageId.toLatin1().constData());// package-type
                StdfFile.WriteString("");					// familyID
                StdfFile.WriteString("");					// Date-code
                StdfFile.WriteString("");					// Facility-ID
                StdfFile.WriteString("");					// FloorID
                StdfFile.WriteString("");					// ProcessID
                StdfFile.WriteRecord();

                if(!m_strHandlerId.isEmpty() || !m_strHandlerType.isEmpty())
                {
                    // Write SDR for last wafer inserted
                    RecordReadInfo.iRecordType = 1;
                    RecordReadInfo.iRecordSubType = 80;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(255);						// Test Head = ALL
                    StdfFile.WriteByte((BYTE)1);			// head#
                    StdfFile.WriteByte((BYTE)1);			// Group#
                    StdfFile.WriteByte((BYTE)1);			// site_count
                    StdfFile.WriteByte((BYTE)1);			// array of test site#
                    StdfFile.WriteString(m_strHandlerType.toLatin1().constData());	// HAND_TYP: Handler/prober type
                    StdfFile.WriteString(m_strHandlerId.toLatin1().constData());		// HAND_ID: Handler/prober name
                    StdfFile.WriteString("");				// CARD_TYP: Probe card type
                    StdfFile.WriteRecord();
                }
            }

            // Reset counters
            iTotalGoodBin=iTotalFailBin=0;
            iPartNumber=0;

            if(!m_lstDateTime.isEmpty())
            {// 12-11-08  16:49

                strValue = m_lstDateTime.takeFirst();

                QDate cDate(strValue.mid(6,2).toInt()+2000,strValue.left(2).toInt(),strValue.mid(3,2).toInt());
                QTime cTime(strValue.mid(9,2).toInt(),strValue.mid(12,2).toInt());

                QDateTime clDateTime(cDate,cTime);
                clDateTime.setTimeSpec(Qt::UTC);
                mStartTime = clDateTime.toTime_t();
            }

            // Write WIR of new Wafer.
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);							// Test head
            StdfFile.WriteByte(255);						// Tester site (all)
            StdfFile.WriteDword(mStartTime);				// Start time
            StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();

            int			iWaferColumns;
            int			iWaferRows;
            int			iStepX;
            int			iStepY;


            if(m_cPosX == 'R')
                iStepX = 1;
            else
                iStepX = -1;

            if(m_cPosY == 'U')
                iStepY = -1;
            else
                iStepY = 1;

            iWaferRows = -iStepY;

            // Write all Parameters read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
            while(Semi_G85File.atEnd() == false)
            {

                iWaferRows+= iStepY;


                // Read line
                strString = ReadLine(Semi_G85File).simplified().toUpper();
                if(strString.startsWith("}"))
                    break;	// Reach the end of valid data records...

                if((int)strString.length() != m_nWaferColumns)
                    m_nWaferColumns = strString.length();

                // Reset Pass/Fail flag.
                                //FIXME: not used ?
                                //bPassStatus = true;

                // Reset counters
                                //FIXME: not used ?
                                //iTotalTests = 0;
                iWaferColumns = -iStepX;


                // Read Parameter results for this record
                for(iIndex=0;iIndex<(int)strString.length();iIndex++)
                {
                    iWaferColumns+= iStepX;

                    if(strString.mid(iIndex,1) == m_strNullBin)
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
                    if((!m_qMapBins.contains(strString.at(iIndex)))
                    || (!m_qMapBins[strString.at(iIndex)]->GetPassFail()))
                    {
                        StdfFile.WriteByte(8);		// PART_FLG : FAILED
                        wSoftBin = wHardBin = 0;	// default FAIL bin
                        iTotalFailBin++;
                        if(m_qMapBins.contains(strString.at(iIndex)))
                        {
                            wSoftBin = m_qMapBins[strString.at(iIndex)]->GetBinNumber();
                            m_qMapBins[strString.at(iIndex)]->SetBinCount(m_qMapBins[strString.at(iIndex)]->GetBinCount()+1);
                        }
                    }
                    else
                    {
                        StdfFile.WriteByte(0);				// PART_FLG : PASSED
                        wSoftBin = m_qMapBins[strString.at(iIndex)]->GetBinNumber();
                        wHardBin = 1;	// PASS Hard bin
                        m_qMapBins[strString.at(iIndex)]->SetBinCount((m_qMapBins[strString.at(iIndex)]->GetBinCount())+1);
                        iTotalGoodBin++;
                    }
                    StdfFile.WriteWord((WORD)0);			// NUM_TEST
                    StdfFile.WriteWord(wHardBin);           // HARD_BIN
                    StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
                    StdfFile.WriteWord(iWaferColumns);	// X_COORD
                    StdfFile.WriteWord(iWaferRows);		// Y_COORD
                    StdfFile.WriteDword(0);					// No testing time known...
                    StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
                    StdfFile.WriteString("");				// PART_TXT
                    StdfFile.WriteString("");				// PART_FIX
                    StdfFile.WriteRecord();
                };
            };			// Read all lines with valid data records in file

            // Write WRR for last wafer inserted
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test head
            StdfFile.WriteByte(255);					// Tester site (all)
            StdfFile.WriteDword(0);						// Time of last part tested
            StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
            StdfFile.WriteDword(0);						// Parts retested
            StdfFile.WriteDword(0);						// Parts Aborted
            StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
            StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
            StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();
        }
    }

    if(!bStdfHeaderWriten)
    {
        // No data found
        m_iLastError = errInvalidFormatParameter;

        m_strLastErrorSpecification = "No 'MAP' data found";

        // Convertion failed.
        // Close STDF file.
        StdfFile.Close();
        return false;
    }

    // Write WCR for last wafer inserted
    float fValue;
    char	cOrientation = ' ';
    if(m_nFlatNotch==0) cOrientation='D';
    else if(m_nFlatNotch==90) cOrientation='L';
    else if(m_nFlatNotch==180) cOrientation='U';
    else if(m_nFlatNotch==270) cOrientation='R';


    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    fValue = (float)m_nWaferSize;
    StdfFile.WriteFloat(fValue);				// Wafer size
    fValue = (float)m_nXSize;
    fValue /= 1000;
    StdfFile.WriteFloat(fValue);				// Height of die
    fValue = (float)m_nYSize;
    fValue /= 1000;
    StdfFile.WriteFloat(fValue);				// Width of die
    StdfFile.WriteByte(3);						// Units are in millimeters
    StdfFile.WriteByte((BYTE) cOrientation);	// Orientation of wafer flat
    StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_X
    StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_Y
    StdfFile.WriteByte((BYTE) m_cPosX);			// POS_X
    StdfFile.WriteByte((BYTE) m_cPosY);			// POS_Y
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<QChar,ParserBinningG85*>::Iterator itMapBin;
    ParserBinningG85 * pBinInfo;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        pBinInfo = itMapBin.value();

        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(1);							// Test sites = ALL
        StdfFile.WriteWord(pBinInfo->GetBinNumber());	// HBIN = 0
        StdfFile.WriteDword(pBinInfo->GetBinCount());	// Total Bins
        if(pBinInfo->GetPassFail())
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(pBinInfo->GetBinName().toLatin1().constData());
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        pBinInfo = itMapBin.value();

        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(1);							// Test sites = ALL
        StdfFile.WriteWord(pBinInfo->GetBinNumber());	// HBIN = 0
        StdfFile.WriteDword(pBinInfo->GetBinCount());	// Total Bins
        if(pBinInfo->GetPassFail())
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(pBinInfo->GetBinName().toLatin1().constData());
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

}
}
