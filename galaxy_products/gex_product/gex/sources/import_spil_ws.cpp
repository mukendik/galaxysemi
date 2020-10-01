//////////////////////////////////////////////////////////////////////
// import_spil_ws.cpp: Convert a SpilWs file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>


#include "import_spil_ws.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//A. Device      :AR9160B
//B. Lot No#     :N0H578.00
//C. Tester      :T7506
//D. Prober      :P1216
//E. Operator    :972194
//F. Probe Card  :AH0077
//G. Program     :AR9160B_J750_WS_2S_011608.xls
//H. Start Date  :2009/01/08
//I. Test Site   :SPIL
//
//==> Lot Summary :
//
//ID                 01     02     04     05     06     07     Total    Yield
//---------------------------------------------------------------------------
//             1   1965      0      1     28      8      1      2003   98.10%
//             2   1972      0      1     20      8      2      2003   98.45%
//==> Lot BinCount & Yield
//
//bin  1 bin  2 bin  4 bin  5 bin  6 bin  7
// 48271      3    150   1377    233     41
//96.40% 00.01% 00.30% 02.75% 00.47% 00.08%
//
//==> Lot Bin Description & Yield
//
//No.     Bin Description                          Count    Yield
//------  ----------------                         -----    ------
//1       Good_BIN1                                48271   96.40%
//2       Continuity                                   3   00.01%
//
//**********  wafer map  **********
//
//1. Wafer ID        :  N0H578-01
//2. F/N Location    :  180 (degree)
//
//7. Bincode Map in Hex Format :
//
//    0     2     4     6     8    10    12    14    16    18    20    22    24    26    28    30
//   -++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-++
//00|                                  01 01 01 03 01 01 01 01 04 01
//01|                            01 01 01 01 01 01 01 01 01 01 01 01 01 01
//02|                      01 01 01 01 01 04 01 01 01 01 01 01 01 01 01 01 01 01
//03|                   01 01 03 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01



// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

CSpilWsBinInfo::CSpilWsBinInfo(int iBin)
{
    nBinNb = iBin;
    nNbCnt=0;
    bPass=false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSpilWstoSTDF::CGSpilWstoSTDF()
{
    m_lStartTime = 0;
    m_lEndTime = 0;
    m_nWaferColumns = 0;
    m_nFlatNotch = 0;
    m_nBinType = 10;
    m_nRefDieX = -1;
    m_nRefDieY = -1;
    m_nBinType = 10;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSpilWstoSTDF::~CGSpilWstoSTDF()
{
    QMap<int,CSpilWsBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSpilWstoSTDF::GetLastError()
{
    m_strLastError = "Import SpilWs: ";

    if(!m_strLastErrorSpecification.isEmpty())
        m_strLastError += "\n\n" + m_strLastErrorSpecification + "\n\n";

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
// Check if File is compatible with SpilWs format
//////////////////////////////////////////////////////////////////////
bool CGSpilWstoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hSpilWsFile(&f);

    // Check if first N line is the correct SpilWs header...
    int	nLine = 0;

    while(!hSpilWsFile.atEnd())
    {
        strString = hSpilWsFile.readLine().remove("=").simplified();



        if((strString.startsWith("> Lot Summary",Qt::CaseInsensitive))
        || (strString.startsWith("> Lot Bin",Qt::CaseInsensitive))
        || (strString.startsWith("> Lot SW_Bin",Qt::CaseInsensitive))
        || (strString.startsWith("> Lot SW_Bin Summary",Qt::CaseInsensitive)))
        {
            bIsCompatible = true;
            break;
        }


        nLine++;

        if(nLine > 50)
            break;
    }

    // Close file
    f.close();

    return bIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SpilWs file
//////////////////////////////////////////////////////////////////////
bool CGSpilWstoSTDF::ReadSpilWsFile(const char* SpilWsFileName,
                                    const char* strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;

    // Open CSV file
    QFile f( SpilWsFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SpilWs file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream hSpilWsFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();


    // Check if first line is the correct SpilWs header...

    while(!hSpilWsFile.atEnd())
    {
        strString = strSection = ReadLine(hSpilWsFile);

        if((strString.startsWith("**********",Qt::CaseInsensitive))
        && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
            break;

        // it's a comment line
        if(strSection.startsWith("*********************************",Qt::CaseInsensitive))
            continue;;

        // Else collect some additionated informationt
        // Delete item number
        // A. Device      :AR9160B
        if((strSection.indexOf(". ") > 0) && (strSection.indexOf(". ") < 3))
            strSection = strSection.section(". ",1);

        strValue = strSection.section(":",1).trimmed();
        strSection = strSection.section(":",0,0);
        strSection = strSection.replace('_',' ').remove(".").trimmed().toUpper();

        if(strSection.startsWith("=="))
        {
            // ==> section
            strSection = strSection.remove("=").remove(" ");
            strSection = strSection.section("&",0,0);

            if((strSection.startsWith(">LOT"))
            && (strSection.endsWith("SUMMARY")))
            {

                // Skip this section

                //ID                 01     02
                strSection = ReadLine(hSpilWsFile);
                //--------------------------
                strSection = ReadLine(hSpilWsFile);
                while(!hSpilWsFile.atEnd())
                {
                    strString = strSection = ReadLine(hSpilWsFile);

                    if((strString.startsWith("**********",Qt::CaseInsensitive))
                    && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
                        break;

                    if(strSection.startsWith("----"))
                    {
                        // Read the next sum line
                        strString = strSection = ReadLine(hSpilWsFile);
                        // and break
                        break;
                    }


                    if(!strSection.trimmed().endsWith("%"))
                    {
                        // Incorrect header...this is not a SpilWs file!
                        m_iLastError = errInvalidFormat;
                        m_strLastErrorSpecification = "Inconsistency section '==> Lot Summary' format";

                        // Convertion failed.
                        // Close file
                        f.close();
                        return false;
                    }
                }

            }
            else
            if((strSection.startsWith(">LOT"))
            && (strSection.endsWith("BINCOUNT")))
            {
                while(!hSpilWsFile.atEnd())
                {
                    strString = strSection = ReadLine(hSpilWsFile);

                    if((strString.startsWith("**********",Qt::CaseInsensitive))
                    && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
                        break;

                    if(strSection.simplified().endsWith("%"))
                        break;

                }

            }
            else
            if((strSection.startsWith(">LOT"))
            && (strSection.endsWith("BINDESCRIPTION")))
            {
                int				iBin;
                CSpilWsBinInfo *pNewBinInfo;

                //No.     Bin Description                          Count    Yield
                strSection = ReadLine(hSpilWsFile);
                //------  ----------------                         -----    ------
                strSection = ReadLine(hSpilWsFile);
                // Check if start with "--"
                if(!strSection.trimmed().startsWith("----"))
                {
                    // Incorrect header...this is not a SpilWs file!
                    m_iLastError = errInvalidFormat;

                    // Convertion failed.
                    m_strLastErrorSpecification = "Inconsistency section '==> Lot Bin Description' format";
                    // Close file
                    f.close();
                    return false;
                }


                while(!hSpilWsFile.atEnd())
                {
                    strString = strSection = ReadLine(hSpilWsFile);

                    if((strString.startsWith("**********",Qt::CaseInsensitive))
                    && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
                        break;

                    strSection = strSection.simplified();

                    if(!strSection.endsWith("%"))
                        break;

                    //1       Good_BIN1                                48271   96.40%

                    iBin = strSection.section(" ",0,0).remove('"').toInt();
                    if(!m_qMapBins.contains(iBin))
                    {
                        pNewBinInfo = new CSpilWsBinInfo(iBin);
                        pNewBinInfo->bPass = (iBin == 1);
                        pNewBinInfo->strName = strSection.section(" ",1,1).remove('"');

                        pNewBinInfo->nNbCnt = 0;

                        m_qMapBins[iBin] = pNewBinInfo;
                    }
                }
            }

        }
        else
        {
            if((strSection == "DEVICE")
            || (strSection == "DEVICE NAME"))
            {
                //Device      :AR9160B
                m_strDeviceId = strValue;
            }
            else
            if((strSection == "LOT NO#")
            || (strSection == "LOT ID"))
            {
                //Lot No#     :N0H578.00
                m_strLotId = strValue;
            }
            else
            if((strSection == "TESTER")
            || (strSection == "TESTER ID"))
            {
                //Tester      :T7506
                m_strTesterType = strValue;
            }
            else
            if((strSection == "PROBER")
            || (strSection == "PROBER ID"))
            {
                //Prober      :P1216
                m_strHandlerId = strValue;
            }
            else
            if((strSection == "OPERATOR")
            || (strSection == "OPERATOR ID"))
            {
                //Operator    :972194
                m_strOperatorId = strValue;
            }
            else
            if((strSection == "PROBE CARD")
            || (strSection == "PROBER CARD"))
            {
                //Probe Card  :AH0077
                m_strHandlerType = strValue;
            }
            else
            if((strSection == "PROGRAM")
            || (strSection == "PROGRAM NAME"))
            {
                //Program     :AR9160B_J750_WS_2S_011608.xls
                m_strJobName = strValue;
            }
            else
            if(strSection == "PROGRAM REV")
            {
                //Program_Rev.      : 1.0
                m_strJobRev = strValue;
            }
            else
            if(strSection == "TEMPERATURE")
            {
                //Temperature      : 25 C
                m_strTemperature = strValue;
            }
            else
            if(strSection == "START DATE")
            {
                //Start Date  :2009/01/08
                strValue = strValue.replace("-","/");

                QDate cDate(strValue.section("/",0,0).toInt(), strValue.section("/",1,1).toInt(),strValue.section("/",2,2).toInt());
                QTime cTime(0,0,0);
                QDateTime clDateTime(cDate,cTime);
                clDateTime.setTimeSpec(Qt::UTC);
                m_lStartTime = clDateTime.toTime_t();
            }
            else
            if(strSection == "TEST SITE")
            {
                //Test Site   :SPIL
                m_strNodeName = strValue;
            }
        }

        strString = strSection;
        if((strString.startsWith("**********",Qt::CaseInsensitive))
        && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
            break;
    }



    if((!strSection.startsWith("**********"))
    && (!strSection.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
    {
        // Incorrect header...this is not a SpilWs file!
        m_iLastError = errInvalidFormat;
        m_strLastErrorSpecification = "Wafer Map section is missing";

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // have a wafermap to save
    // write STDF file
    // Loop reading file until end is reached & generate STDF file dynamically.
    if(!WriteStdfFile(&hSpilWsFile,strFileNameSTDF))
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
// Create STDF file from SpilWs data parsed
//////////////////////////////////////////////////////////////////////
bool CGSpilWstoSTDF::WriteStdfFile(QTextStream *hSpilWsFile, const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    bool bStdfIsOpened = false;

    // Write Test results for each line read.
    QString		strString, strSection, strValue;
    QString		strBin;
    int			iIndex;				// Loop index
    long		iTotalGoodBin,iTotalFailBin;
        long iPartNumber;
        //FIXME: not used ?
        //long iTotalTests;
        //bool bPassStatus;

    int			iWaferRows;

    while(!hSpilWsFile->atEnd())
    {

        strString = strSection = ReadLine(*hSpilWsFile);

        // Delete item number
        // A. Device      :AR9160B
        if((strSection.indexOf(". ") > 0) && (strSection.indexOf(". ") < 3))
            strSection = strSection.section(". ",1);


        // We have a **********  wafer map  ********** marker
        if((strString.startsWith("**********",Qt::CaseInsensitive))
        && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
            continue;

        strValue = strSection.section(":",1).simplified();
        strSection = strSection.section(":",0,0).replace('_',' ');
        strSection = strSection.section("&",0,0).simplified().toUpper();

        if(strSection == "WAFER ID")
        {//1. Wafer ID        :  N0H578-01
            m_strWaferId=strValue.remove('"');
        }
        else
        if(strSection == "F/N LOCATION")
        {//2. F/N Location    :  180 (degree)
            m_nFlatNotch=strValue.section(" ",0,0).toInt();
        }
        else
        if(strSection == "WAFER NOTCH")
        {//2. Wafer_Notch           : D
            if(strValue == "D")
                m_nFlatNotch=0;
            else
            if(strValue == "L")
                m_nFlatNotch=90;
            else
            if(strValue == "U")
                m_nFlatNotch=180;
            else
            if(strValue == "R")
                m_nFlatNotch=270;
        }
        else
        if(strSection == "DIE PER WAFER")
        {//3. Die per Wafer   :  2003

        }
        else
        if(strSection == "START TIME")
        {//4. Start Time      :  2009/01/08 21:14:22

            strValue = strValue.replace("-","/");

            QDate cDate(strValue.section("/",0,0).toInt(),strValue.section("/",1,1).toInt(),strValue.section("/",2).left(2).toInt());
            strValue = strValue.section(" ",1);
            QTime cTime(strValue.section(":",0,0).toInt(),strValue.section(":",1,1).toInt(),strValue.section(":",2).toInt());
            QDateTime clDateTime(cDate,cTime);
            clDateTime.setTimeSpec(Qt::UTC);


            m_lStartTime = clDateTime.toTime_t();

        }
        else
        if(strSection == "END TIME")
        {//5. End Time        :  2009/01/08 21:15:14

            strValue = strValue.replace("-","/");

            QDate cDate(strValue.section("/",0,0).toInt(),strValue.section("/",1,1).toInt(),strValue.section("/",2).left(2).toInt());
            strValue = strValue.section(" ",1);
            QTime cTime(strValue.section(":",0,0).toInt(),strValue.section(":",1,1).toInt(),strValue.section(":",2).toInt());
            QDateTime clDateTime(cDate,cTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lEndTime = clDateTime.toTime_t();
        }
        else
        if(strSection.endsWith("BINCOUNT"))
        {//6. BinCount & Yield:

            // Skip this section
            while(!hSpilWsFile->atEnd())
            {
                strString = strSection = ReadLine(*hSpilWsFile);

                if((strString.startsWith("**********",Qt::CaseInsensitive))
                && (strString.remove("*").remove(" ").endsWith("wafermap",Qt::CaseInsensitive)))
                    break;

                strSection = strSection.simplified();
                if(strSection.endsWith("%"))
                    break;
            }
        }
        else
        if((strSection.startsWith("BINCODE MAP"))
        || (strSection.indexOf("BIN WAFERMAP") >= 0))
        {//7. Bincode Map in Hex Format :
            // Read the wafer map

            if(strSection.indexOf("HEX") > 0)
                m_nBinType = 16;
            else
                m_nBinType = 10;

            if(!bStdfIsOpened)
            {
                bStdfIsOpened = true;

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
                StdfFile.WriteString("");					// test-temperature
                // Construct custom Galaxy USER_TXT
                QString	strUserTxt;
                strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
                strUserTxt += ":";
                strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
                strUserTxt += ":Spil Ws";
                StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
                StdfFile.WriteString("");					// aux-file
                StdfFile.WriteString("");					// package-type
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

            // read the first number   0     2     4     6
            strSection = ReadLine(*hSpilWsFile).simplified();
            m_nRefDieX = strSection.section(" ",0,0).toInt();

            // Goto the line -++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-
            strSection = ReadLine(*hSpilWsFile).simplified();
            // Check if start with "-+"
            if(strSection.indexOf("-+")<0)
            {
                // Incorrect header...this is not a SpilWs file!
                m_iLastError = errInvalidFormat;
                m_strLastErrorSpecification = "Inconsistency Wafer Map format";

                // Convertion failed.
                // Close file
                StdfFile.Close();
                return false;
            }


            int iStart, iStop, iBinSize;
            iStart = strSection.indexOf("-");
            iStop = strSection.indexOf("-", iStart+1);
            iBinSize = iStop - iStart;

            strValue = "-";
            m_strNullBin = " ";
            for(iIndex=1; iIndex<iBinSize; iIndex++)
            {
                strValue+="+";
                m_strNullBin+=" ";
            }
            m_nWaferColumns = strSection.count(strValue);

            // Write WIR of new Wafer.
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);							// Test head
            StdfFile.WriteByte(255);						// Tester site (all)
            StdfFile.WriteDword(m_lStartTime);				// Start time
            StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();

            iWaferRows = 0;
            // Write all Parameters read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR

            // Read all other line until -++-++-++-++-++-++-++-++-++-++-++-++-++-++-++-
            while (!hSpilWsFile->atEnd())
            {
                iWaferRows++;
                strSection = ReadLine(*hSpilWsFile);

                if(strSection.trimmed().startsWith("-+"))
                    break;

                // Check if the line is in the good format
                if(strSection.indexOf("|") < 1)
                    break;


                // Read the first number for StartY
                if(m_nRefDieY < 0)
                    m_nRefDieY = strSection.section("|",0,0).toInt();

                // Extract the line
                strSection = strSection.section("|",1,1);

                // Reset Pass/Fail flag.
                                //FIXME: not used ?
                                //bPassStatus = true;

                // Reset counters
                                //FIXME: not used ?
                                //iTotalTests = 0;


                // Read Parameter results for this record
                for(iIndex=0; iIndex<m_nWaferColumns; iIndex++)
                {
                    strBin = strSection.mid(iIndex*(iBinSize),iBinSize);

                    strBin = strBin.replace('.',' ');

                    if(strBin == m_strNullBin)
                        continue;


                    CSpilWsBinInfo *pBinInfo;
                    int iBin;
                    bool ok;
                    // Convert the bin result if in Hexadecimal
                    iBin = strBin.simplified().toInt(&ok,m_nBinType);


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

                    if(!m_qMapBins.contains(iBin))
                    {
                        pBinInfo = new CSpilWsBinInfo(iBin);
                        pBinInfo->bPass = (iBin == 1);
                        m_qMapBins[iBin] = pBinInfo;
                    }
                    pBinInfo = m_qMapBins[iBin];

                    if(!pBinInfo->bPass)
                    {
                        StdfFile.WriteByte(8);		// PART_FLG : FAILED
                        iTotalFailBin++;
                        pBinInfo->nNbCnt++;
                    }
                    else
                    {
                        StdfFile.WriteByte(0);				// PART_FLG : PASSED
                        pBinInfo->nNbCnt++;
                        iTotalGoodBin++;
                    }
                    StdfFile.WriteWord((WORD)0);			// NUM_TEST
                    StdfFile.WriteWord(iBin);				// HARD_BIN
                    StdfFile.WriteWord(iBin);				// SOFT_BIN
                    StdfFile.WriteWord(iIndex+m_nRefDieX);		// X_COORD
                    StdfFile.WriteWord(iWaferRows+m_nRefDieY);	// Y_COORD
                    StdfFile.WriteDword(0);					// No testing time known...
                    StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
                    StdfFile.WriteString("");				// PART_TXT
                    StdfFile.WriteString("");				// PART_FIX
                    StdfFile.WriteRecord();
                }

            }

            // Write WRR for last wafer inserted
            RecordReadInfo.iRecordType = 2;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test head
            StdfFile.WriteByte(255);					// Tester site (all)
            StdfFile.WriteDword(m_lEndTime);			// Time of last part tested
            StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
            StdfFile.WriteDword(0);						// Parts retested
            StdfFile.WriteDword(0);						// Parts Aborted
            StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
            StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
            StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
            StdfFile.WriteRecord();

        }
    }

    if(!bStdfIsOpened)
    {
        // Incorrect header...this is not a SpilWs file!
        m_iLastError = errInvalidFormat;
        m_strLastErrorSpecification = "No Wafer Map found";
        // Convertion failed.
        return false;
    }

    char	cOrientation = ' ';
    if(m_nFlatNotch==0) cOrientation='D';
    else if(m_nFlatNotch==90) cOrientation='L';
    else if(m_nFlatNotch==180) cOrientation='U';
    else if(m_nFlatNotch==270) cOrientation='R';

    // Write WCR for last wafer inserted

    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);						// Wafer size
    StdfFile.WriteFloat(0);						// Height of die
    StdfFile.WriteFloat(0);						// Width of die
    StdfFile.WriteByte(3);						// Units are in millimeters
    StdfFile.WriteByte((BYTE) cOrientation);	// Orientation of wafer flat
  StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_X
  StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_Y
    StdfFile.WriteByte((BYTE) 'R');				// POS_X
    StdfFile.WriteByte((BYTE) 'D');				// POS_Y
    StdfFile.WriteRecord();


    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CSpilWsBinInfo*>::Iterator itMapBin;
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
        StdfFile.WriteString(itMapBin.value()->strName.toLatin1().constData());
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
        StdfFile.WriteString(itMapBin.value()->strName.toLatin1().constData());
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
// Convert 'FileName' SpilWs file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSpilWstoSTDF::Convert(const char *SpilWsFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(SpilWsFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SpilWsFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadSpilWsFile(SpilWsFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading SpilWs file
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
QString CGSpilWstoSTDF::ReadLine(QTextStream& hFile)
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

    do{
        strString = hFile.readLine();
        strString = strString.trimmed();
    }while(!strString.isNull() && strString.isEmpty());

    return strString;

}
