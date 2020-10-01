//////////////////////////////////////////////////////////////////////
// import_prober_tel.cpp: Convert a ProberTel (Tokyo Electron Limited) file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "report_build.h"
#include "import_prober_tel.h"
#include "dl4_tools.h"		// for read binary data
#include "import_constants.h"
#include "engine.h"

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);

// main.cpp
extern QLabel *			GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar *	GexProgressBar;		// Handle to progress bar in status bar


#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

bool		gDumpRecord = false;

//////////////////////////////////////////////////////////////////////
// BINARY FUNCTIONS
//////////////////////////////////////////////////////////////////////
class CGProberTel_StoredBuffer : public StoredObject
{
public:
    CGProberTel_StoredBuffer(){ bSameCPUType = 1; }	// TELP8: do Not allow LSB/MSB swap

    gsint32 GetStorageSizeBytes()			   { return 0; }
    gsint32 LoadFromBuffer(const void* /*v*/) { return 0; }

    int	ReadByte(const void *v, BYTE *bData)
    {
        int nResult = StoredObject::ReadByte(v,bData);
        if(gDumpRecord)
        {
            QString strString;
            strString = "==== ReadByte(";
            strString += QString::number(*bData);
            strString += ")";
            WriteDebugMessageFile(strString);
        }
        return nResult;
    };
    int	ReadWord(const void *v, short *ptWord)
    {
        int nResult = StoredObject::ReadWord(v,ptWord);
        if(gDumpRecord)
        {
            QString strString;
            strString = "==== ReadWord(";
            strString += QString::number(*ptWord);
            strString += ")";
            WriteDebugMessageFile(strString);
        }
        return nResult;
    };
    int	ReadDword(const void *v, gsint32 *ptDword)
    {
        int nResult = StoredObject::ReadDword(v,ptDword);
        if(gDumpRecord)
        {
            QString strString;
            strString = "==== ReadDword(";
            strString += QString::number(*ptDword);
            strString += ")";
            WriteDebugMessageFile(strString);
        }
        return nResult;
    };
    int	ReadFloat(const void *v,  float *ptFloat)
    {
        int nResult = StoredObject::ReadFloat(v,ptFloat);
        if(gDumpRecord)
        {
            QString strString;
            strString = "==== ReadFloat(";
            strString += QString::number(*ptFloat);
            strString += ")";
            WriteDebugMessageFile(strString);
        }
        return nResult;
    };
    int	ReadDouble(const void *v, double *ptDouble)
    {
        int nResult = StoredObject::ReadDouble(v,ptDouble);
        if(gDumpRecord)
        {
            QString strString;
            strString = "==== ReadDouble(";
            strString += QString::number(*ptDouble);
            strString += ")";
            WriteDebugMessageFile(strString);
        }
        return nResult;
    };
    int	 ReadString(const void *v, char *szString, int nSize)
    {
        BYTE	*b = (BYTE*) v;
        char c = '\0';
        int i = 0;

        // Read one char from v
        while( (c = b[i]) != '\0' )
        {
            if( c == EOF )         /* return false on unexpected EOF */
            {
                szString[i] = '\0';
                return 0;
            }
            szString[i++] = c;

            if(i >= nSize)
                break;
        }
        szString[i] = '\0';

        if(gDumpRecord)
        {
            QString strString;
            strString = "==== ReadString(";
            strString += szString;
            strString += ")";
            WriteDebugMessageFile(strString);
        }
        return i;
    };
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGProberTelBaseToSTDF::CGProberTelBaseToSTDF()
{
    m_lStartTime    = 0;
    m_lEndTime      = 0;
    mTotalRecords   = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGProberTelBaseToSTDF::~CGProberTelBaseToSTDF()
{
    m_mapBinInfo.clear();
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGProberTelBaseToSTDF::GetLastError()
{
    m_strLastError = "Import ProberTel: ";

    switch(m_iLastError)
    {
        default:
        case errNoError:
            m_strLastError += "No Error";
            break;
        case errWarning:
            m_strLastError += "*WARNING*";
            break;
        case errOpenFail:
            m_strLastError += "Failed to open file";
            break;
        case errConversionFailed:
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
// Read and Parse the ProberTel file
//////////////////////////////////////////////////////////////////////
int CGProberTelBaseToSTDF::ReadProberTelFile(const char *strFileNameSTDF)
{
    QString			strString;

    // Debug message
    strString = "---- CGProberTeltoSTDF::ReadProberTelFile(";
    strString += m_strProberTelFileName;
    strString += ")";
    WriteDebugMessageFile(strString);

    // Open ProberTel file
    m_lStartTime = 0;
    m_lEndTime = -1;
    QFile f( m_strProberTelFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ProberTel file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return eConvertError;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;

    QCoreApplication::processEvents();

    QFileInfo clFile(m_strProberTelFileName);
    m_strDataFilePath = clFile.absolutePath();

    if(ReadDataRecords(f) == false)
    {
        f.close();
        return eConvertError;
    }

    if(WriteStdfFile(f, strFileNameSTDF) == false)
    {
        // Convertion failed.
        QFile::remove(strFileNameSTDF);
        f.close();
        return eConvertError;
    }

    f.close();

    // Success parsing ProberTel file
    return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' ProberTel file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGProberTelBaseToSTDF::Convert(const char *ProberTelFileName, QString &strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(ProberTelFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(ProberTelFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    m_strProberTelFileName = ProberTelFileName;

    int nStatus = ReadProberTelFile(strFileNameSTDF.toLatin1().constData());

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
        GexScriptStatusLabel->hide();


    return (nStatus == eConvertSuccess);
}

//////////////////////////////////////////////////////////////////////
// Read next binary block
//////////////////////////////////////////////////////////////////////
int	 CGProberTelBaseToSTDF::ReadBlock(QFile* pFile, char *data, qint64 len)
{
    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) pFile->pos() > iNextFilePos)
        {
            iProgressStep += 100/iFileSize + 1;
            iNextFilePos  += iFileSize/100 + 1;
            GexProgressBar->setValue(iProgressStep);
        }
    }
    QCoreApplication::processEvents();

    return pFile->read(data, len);
}

//////////////////////////////////////////////////////////////////////
// Create and write Stdf file
//////////////////////////////////////////////////////////////////////
bool CGProberTelBaseToSTDF::WriteStdfFile(QFile &f, const char *strFileNameSTDF)
{
    m_iLastError = ReadMapHeaderRecord(f);
    if(m_iLastError != errNoError)
    {
        f.close();
        return false;
    }

//    short	sWord;
    BYTE	bByte;

    /////////////////////
    // MAP DATA
    int iRecordIndex;
    int iTotalDies;
    int iTotalGoodBin;
    int iTotalFailBin;
    int iPartNumber;
    int iXpos;
    int iYpos;
    gsint32 iBin;
    bool bPass;
    bool bSkipDie;
    bool bUntested;

    iPartNumber = 0;
    iTotalGoodBin = 0;
    iTotalFailBin = 0;

    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        // Convertion failed.
        m_iLastError = errWriteSTDF;

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
    StdfFile.WriteByte((BYTE) m_strRtstCod.toLatin1());	// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strProductId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("Prober Tokyo Electron Limited");			// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());					// sublot-id
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
    strUserTxt += ":PROBER_TEL";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");							// aux-file
    StdfFile.WriteString("");							// package-type
    StdfFile.WriteString("");							// familyID
    StdfFile.WriteString("");							// Date-code
    StdfFile.WriteString("");							// Facility-ID
    StdfFile.WriteString("");							// FloorID
    StdfFile.WriteString("");							// ProcessID
    StdfFile.WriteString("");							// OperationFreq
    StdfFile.WriteString("");							// Spec-nam
    StdfFile.WriteString("");							// Spec-ver
    StdfFile.WriteString("");							// Flow-id
    StdfFile.WriteString("");							// setup_id
    StdfFile.WriteRecord();

    // Write WIR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);								// Test head
    StdfFile.WriteByte(255);							// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);					// Start time
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    /*
    // Write WCR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);							// Wafer size
    StdfFile.WriteFloat((float)iXpos);				// Height of die
    StdfFile.WriteFloat((float)iYpos);				// Width of die
    StdfFile.WriteByte(4);							// Units are in millimeters
    StdfFile.WriteByte(0);	// Orientation of wafer flat
    StdfFile.WriteRecord();
    */

    char *  lBlock = NULL;
    int     lBlockSize = 0;

    for(iRecordIndex=0; iRecordIndex < mTotalRecords; iRecordIndex++)
    {
        m_iLastError = ReadMapRowHeaderRecord(f, iXpos, iYpos, iTotalDies);
        if(m_iLastError != errNoError)
        {
            StdfFile.Close();

            // Incorrect header...this is not a ProberTel file!
            // Convertion failed.
            return false;
        }

        m_iLastError = ReadMapRowDataRecord(f, iTotalDies, &lBlock, lBlockSize);
        if(m_iLastError != errNoError)
        {
            StdfFile.Close();

            if (lBlock)
            {
                delete [] lBlock;
                lBlock = NULL;
            }

            // Incorrect header...this is not a ProberTel file!
            // Convertion failed.
            return false;
        }

        for(int lIndex=0; lIndex<iTotalDies; lIndex++)
        {
            bPass = bSkipDie = bUntested = false;

            //Die data : 2 Bytes
            if (ReadDieData(lBlock, lBlockSize, lIndex, iBin, bByte) == false)
            {
                StdfFile.Close();

                // Incorrect header...this is not a ProberTel file!
                // Convertion failed.
                m_iLastError = errConversionFailed;

                return false;
            }

            // Test results : D8
            bPass = ((0x1 & bByte) == 0);
            // Mandatory inking : D12
//            sWord = ((0x10 & bByte));
            // Skip die : D13
            bSkipDie = ((0x20 & bByte) == 0x20);

            // Untested : D15
            bUntested = ((0x80 & bByte) == 0x80);

            if(!bSkipDie && !bUntested)
            {
                if(!m_mapBinInfo.contains(iBin))
                {
                    m_mapBinInfo[iBin].bPass = bPass;
                    m_mapBinInfo[iBin].nBinNumber = iBin;
                    m_mapBinInfo[iBin].nNbCnt = 0;
                }

                m_mapBinInfo[iBin].nNbCnt++;

                // Write PIR
                RecordReadInfo.iRecordType = 5;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);			// Test head
                StdfFile.WriteByte(1);			// Tester site
                StdfFile.WriteRecord();

                iPartNumber++;

                // Write PRR
                RecordReadInfo.iRecordType = 5;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);			// Test head
                StdfFile.WriteByte(1);			// Tester site#:1
                if(bPass == true)
                {
                    StdfFile.WriteByte(0);				// PART_FLG : PASSED
                    iTotalGoodBin++;
                }
                else
                {
                    StdfFile.WriteByte(8);				// PART_FLG : FAILED
                    iTotalFailBin++;
                }
                StdfFile.WriteWord(0);					// NUM_TEST
                StdfFile.WriteWord(iBin);				// HARD_BIN
                StdfFile.WriteWord(iBin);				// SOFT_BIN
                StdfFile.WriteWord(iXpos);				// X_COORD
                StdfFile.WriteWord(iYpos);				// Y_COORD
                StdfFile.WriteDword(0);					// No testing time known...
                StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
                StdfFile.WriteString("");			// PART_TXT
                StdfFile.WriteString("");			// PART_FIX
                StdfFile.WriteRecord();
            }

            iXpos++;
        }

        if (lBlock)
        {
            delete [] lBlock;
            lBlock = NULL;
        }
    }

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lEndTime);			// Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
    StdfFile.WriteString( m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteString("");					// FabId
    StdfFile.WriteString("");					// FrameId
    StdfFile.WriteString("");					// MaskId
    StdfFile.WriteString("");					// UserDesc
    StdfFile.WriteRecord();

    // Write WCR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);							// Wafer size
    StdfFile.WriteFloat(0);							// Height of die
    StdfFile.WriteFloat(0);							// Width of die
    StdfFile.WriteByte(0);							// Units are in millimeters
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CGProberTelBinInfo>::Iterator itMapBin;
    for ( itMapBin = m_mapBinInfo.begin(); itMapBin != m_mapBinInfo.end(); ++itMapBin )
    {
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value().nBinNumber);	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value().nNbCnt);	// Total Bins
        if(itMapBin.value().bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString("");
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_mapBinInfo.begin(); itMapBin != m_mapBinInfo.end(); ++itMapBin )
    {
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value().nBinNumber);				// HBIN = 0
        StdfFile.WriteDword(itMapBin.value().nNbCnt);	// Total Bins
        if(itMapBin.value().bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString("");
        StdfFile.WriteRecord();
    }

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lEndTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    return true;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with ProberTel format
//////////////////////////////////////////////////////////////////////
CGProberTelThousandToSTDF::CGProberTelThousandToSTDF()
    : CGProberTelBaseToSTDF()
{
}

CGProberTelThousandToSTDF::~CGProberTelThousandToSTDF()
{

}

bool CGProberTelThousandToSTDF::IsCompatible(const char *szFileName)
{
    QString lString;
    char	lBlock[PROBER_TEL_BLOCK_SIZE];

    // Open file
    QFile lFile(szFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        return false;
    }

    int	lIndex = 0;
    int lBlockSize = lFile.read(lBlock, 61);
    if (lBlockSize != 61)
    {
        // Incorrect header...this is not a ProberTel file!
        return false;
    }

    char	szString[1024];
    short	sWord;
    int		nNumber;
    bool	bIsNumber;

    CGProberTel_StoredBuffer	lBinaryObject;

    /////////////////////
    // WAFER DATA

    //Wafer Id : 25 Ascii
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 25);
    lString = szString;
    //Wafet No : 2 Bytes
    // 00 - 99
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 2);
    lString = szString;
    nNumber = lString.simplified().toInt(&bIsNumber);
    if(!bIsNumber || (nNumber < 0) || (nNumber > 99))
    {
        lFile.close();
        return false;
    }

    // Cassette No : 1 Byte
    // 1 or 2
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 1);
    //Slot No : 2 Bytes
    // 01 - 25
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 2);
    lString = szString;
    nNumber = lString.simplified().toInt(&bIsNumber);
    if(!bIsNumber || (nNumber < 1) || (nNumber > 25))
    {
        lFile.close();
        return false;
    }

    // Test count : 1 Byte
    // 1 - F
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 1);

    /////////////////////
    // TEST TOTAL

    //Pass Total : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &sWord);
    //Fail Total : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &sWord);
    //Test Total : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &sWord);

    //Wafer start time : 12 Bytes (each byte = one value of the date)
    QDateTime lDateTime;
    lDateTime = CGProberTelThousandToSTDF::ByteArrayToDate(lBlock, lIndex);

    if(!lDateTime.isValid() && !lDateTime.isDaylightTime())
    {
        // Not a valid DateTime
        lFile.close();
        return false;
    }

    lFile.close();

    return true;
}

QDateTime CGProberTelThousandToSTDF::ByteArrayToDate(const char *data, int& index)
{
    //Wafer start time : 12 Bytes (each byte = one value of the date)
    QDateTime   lDateTime;

    if (data != NULL)
    {
        int     lYear;
        int     lMonth;
        int     lDay;
        int     lHour;
        int     lMin;
        int     lSec;
        BYTE    lByte;
        CGProberTel_StoredBuffer	lBinaryObject;

        //Wafer start time : 12 Bytes (each byte = one value of the date)
        //Wafer start time : 12 Bytes (each byte = one value of the date)
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lYear = lByte*10 + 2000;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lYear += lByte;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lMonth = lByte*10;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lMonth += lByte;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lDay = lByte*10;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lDay += lByte;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lHour = lByte*10;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lHour += lByte;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lMin = lByte*10;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lMin += lByte;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lSec = lByte*10;
        index += lBinaryObject.ReadByte(data+index, &lByte);
        lSec += lByte;

        lDateTime.setDate(QDate(lYear, lMonth, lDay));
        lDateTime.setTime(QTime(lHour, lMin, lSec));
    }

    return lDateTime;
}

bool CGProberTelThousandToSTDF::ReadDieData(char *data, int len, int dieIndex, gsint32 &bin, BYTE& status)
{
    if (data == NULL)
        return false;

    if (dieIndex * 2 >= len)
        return false;

    BYTE lByte;
    CGProberTel_StoredBuffer	lBinaryObject;

    //Bin data : 1 byte
    lBinaryObject.ReadByte(data+(dieIndex*2), &lByte);
    // Bin data : D0-7Ò
    bin = lByte;

    //Other info : 1 byte
    lBinaryObject.ReadByte(data+(dieIndex*2)+1, &status);

    return true;
}

bool CGProberTelThousandToSTDF::ReadDataRecords(QFile &inputFile)
{
    m_iLastError = ReadWaferDataRecord(inputFile);
    if(m_iLastError != errNoError)
    {
        return false;
    }


    m_iLastError = ReadTestTotalRecord(inputFile);
    if(m_iLastError != errNoError)
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read Tel Wafer Data Record
//////////////////////////////////////////////////////////////////////
int CGProberTelThousandToSTDF::ReadWaferDataRecord(QFile &inputFile)
{
    char	szBlock[31];
    int		iBlockSize = inputFile.read(szBlock, 31);
    if(iBlockSize != 31)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int		iIndex = 0;
    char	szString[1024];

    CGProberTel_StoredBuffer	clBinaryObject;

    /////////////////////
    // WAFER DATA

    //Wafer Id : 25 Ascii
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 25);
    m_strLotId = QString(szString).trimmed();
    //Wafet No : 2 Bytes
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    m_strWaferId = QString(szString).trimmed();
    // Cassette No : 1 Byte
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 1);
    m_strProductId = QString(szString).trimmed();
    //Slot No : 2 Bytes
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    m_strSubLotId = QString(szString).trimmed();
    // Test count : 1 Byte
    char lRetest;
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, &lRetest, 1);
    m_strRtstCod = lRetest;

    return errNoError;
}

//////////////////////////////////////////////////////////////////////
// Read Tel Test record
//////////////////////////////////////////////////////////////////////
int CGProberTelThousandToSTDF::ReadTestTotalRecord(QFile &inputFile)
{
    char	lBlock[30];
    if(inputFile.read(lBlock, 30) != 30)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int		lIndex = 0;
    short	lWord;

    CGProberTel_StoredBuffer	lBinaryObject;

    /////////////////////
    // TEST TOTAL

    //Pass Total : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    //Fail Total : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    //Test Total : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);

    QDateTime clDateTime;

    //Wafer start time : 12 Bytes (each byte = one value of the date)
    clDateTime = CGProberTelThousandToSTDF::ByteArrayToDate(lBlock, lIndex);

    if(clDateTime.isValid() || clDateTime.isDaylightTime())
    {
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    //Wafer finish time : 12 Bytes (each byte = one value of the date)
    clDateTime = CGProberTelThousandToSTDF::ByteArrayToDate(lBlock, lIndex);

    if(clDateTime.isValid() || clDateTime.isDaylightTime())
    {
        clDateTime.setTimeSpec(Qt::UTC);
        m_lEndTime = clDateTime.toTime_t();
    }

    return errNoError;
}

int CGProberTelThousandToSTDF::ReadMapHeaderRecord(QFile &inputFile)
{
    char lBlock[5];

    if (inputFile.read(lBlock, 5) != 5)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int     lIndex = 0;
    short	lWord;
    BYTE	lByte;
    CGProberTel_StoredBuffer    lBinaryObject;

    //Number of records : 1 Bytes
    //Number of line in this wafer
    lIndex += lBinaryObject.ReadByte(lBlock+lIndex, &lByte);
    mTotalRecords = lByte;
    //Initial die distance X : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    //Initial die distance Y : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);

    return errNoError;
}

int CGProberTelThousandToSTDF::ReadMapRowDataRecord(QFile &inputFile, int totalDies, char **data, int &len)
{
    len     = totalDies*2;
    *data   = new char[len];

    // Read Die Data
    if (inputFile.read(*data, len) != len)
    {
        // Incorrect data...this is not a ProberTel file!
        // Convertion failed.
        m_iLastError = errConversionFailed;

        return errConversionFailed;
    }

    return errNoError;
}

int CGProberTelThousandToSTDF::ReadMapRowHeaderRecord(QFile &inputFile, int &origX, int &origY, int &dieCount)
{
    char lBlock[5];
    if(inputFile.read(lBlock, 5) != 5)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int     lIndex = 0;
    short	lWord;
    BYTE	lByte;
    CGProberTel_StoredBuffer    lBinaryObject;

    //First address X of record : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    origX = lWord;

    //First address Y of record : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    origY = lWord;

    //Number of dies : 1 Bytes
    lIndex += lBinaryObject.ReadByte(lBlock+lIndex, &lByte);
    dieCount = lByte;

    return errNoError;
}

CGProberTelMillionToSTDF::CGProberTelMillionToSTDF()
    : CGProberTelBaseToSTDF()
{

}

CGProberTelMillionToSTDF::~CGProberTelMillionToSTDF()
{

}

QDateTime CGProberTelMillionToSTDF::StringToDate(const QString &lDate)
{
    QDateTime   lDateTime;
    QRegExp     lDateExp("^\\d{12}$");

    if (lDateExp.exactMatch(lDate))
    {
        lDateTime.setDate(QDate(2000 + lDate.mid(0, 2).toInt(), lDate.mid(2, 2).toInt(), lDate.mid(4, 2).toInt()));
        lDateTime.setTime(QTime(lDate.mid(6, 2).toInt(), lDate.mid(8, 2).toInt(), lDate.mid(10, 2).toInt()));
    }
    return lDateTime;
}

bool CGProberTelMillionToSTDF::IsCompatible(const char *szFileName)
{
    QString lString;
    char	lBlock[PROBER_TEL_BLOCK_SIZE];

    // Open file
    QFile lFile(szFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        return false;
    }

    int	lIndex = 0;
    int lBlockSize = lFile.read(lBlock, 157);
    if (lBlockSize != 157)
    {
        // Incorrect header...this is not a ProberTel file!
        return false;
    }

    char	szString[1024];
    int		nNumber;
    bool	bIsNumber;

    CGProberTel_StoredBuffer	lBinaryObject;

    /////////////////////
    // TEST TOTAL

    //Pass Total : 11 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 11);
    //Fail Total : 11 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 11);
    //Test Total : 11 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 11);

    QDateTime clDateTime;

    //Wafer start time : 12 Bytes (each byte = one value of the date)
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 12);
    clDateTime = CGProberTelMillionToSTDF::StringToDate(szString);

    if(!clDateTime.isValid() && !clDateTime.isDaylightTime())
    {
        // Not a valid DateTime
        lFile.close();
        return false;
    }

    //Wafer finish time : 12 Bytes (each byte = one value of the date)
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 12);
    clDateTime = CGProberTelMillionToSTDF::StringToDate(szString);

    if(!clDateTime.isValid() && !clDateTime.isDaylightTime())
    {
        // Not a valid DateTime
        lFile.close();
        return false;
    }

    /////////////////////
    // WAFER DATA

    //Lot Id : 36 Ascii
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 36);
    lString = szString;
    //Wafet No : 2 Bytes
    // 00 - 99
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 2);
    lString = szString;
    nNumber = lString.simplified().toInt(&bIsNumber);
    if(!bIsNumber || (nNumber < 0) || (nNumber > 99))
    {
        lFile.close();
        return false;
    }

    // Cassette No : 1 Byte
    // 1 or 2
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 1);
    //Slot No : 2 Bytes
    // 01 - 25
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 2);
    lString = szString;
    nNumber = lString.simplified().toInt(&bIsNumber);
    if(!bIsNumber || (nNumber < 1) || (nNumber > 25))
    {
        lFile.close();
        return false;
    }

    // Test count : 1 Byte
    // 1 - F
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 1);

    // Map Coordinate System+BIN type + distance info + address info
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, szString, 58);

    lFile.close();

    return true;
}

bool CGProberTelMillionToSTDF::ReadDieData(char *data, int len, int dieIndex, gsint32 &bin, BYTE &status)
{
    if (data == NULL)
        return false;

    if (dieIndex * 5 >= len)
        return false;

    gsint32 lDWord;
    CGProberTel_StoredBuffer	lBinaryObject;

    //Bin data : 1 byte
    lBinaryObject.ReadDword(data+(dieIndex*5), &lDWord);
    // Bin data : D0-7Ò
    bin = lDWord;

    // Convert encoded bin to a bin#:
    if(bin)
        bin = (gsuint32)((double)log((double)bin)/log(2.0));

    //Other info : 1 byte
    lBinaryObject.ReadByte(data+(dieIndex*5)+4, &status);

    return true;
}

bool CGProberTelMillionToSTDF::ReadDataRecords(QFile &inputFile)
{
    m_iLastError = ReadTestTotalRecord(inputFile);
    if(m_iLastError != errNoError)
    {
        return false;
    }

    m_iLastError = ReadWaferDataRecord(inputFile);
    if(m_iLastError != errNoError)
    {
        return false;
    }

    return true;
}

int CGProberTelMillionToSTDF::ReadWaferDataRecord(QFile& inputFile)
{
    char	lBlock[100];
    if(inputFile.read(lBlock, 100) != 100)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int		lIndex = 0;
    char	lString[1024];
    CGProberTel_StoredBuffer    lBinaryObject;

    /////////////////////
    // WAFER DATA

    //Lot Id : 36 Ascii
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 36);
    m_strLotId = QString(lString).trimmed();
    //Wafet No : 2 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 2);
    m_strWaferId = QString(lString).trimmed();
    // Cassette No : 1 Byte
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 1);
    m_strProductId = QString(lString).trimmed();
    //Slot No : 2 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 2);
    m_strSubLotId = QString(lString).trimmed();
    // Test count : 1 Byte
    char lRetest;
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, &lRetest, 1);
    m_strRtstCod = lRetest;
    // Map Coordinate System+BIN type + distance info + address info
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 58);

    return errNoError;
}

int CGProberTelMillionToSTDF::ReadTestTotalRecord(QFile& inputFile)
{
    char	lBlock[57];
    if(inputFile.read(lBlock, 57) != 57)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int		lIndex = 0;
    char	lString[1024];
    CGProberTel_StoredBuffer    lBinaryObject;
    QDateTime                   lDateTime;

    /////////////////////
    // TEST TOTAL

    //Pass Total : 11 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 11);
    //Fail Total : 11 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 11);
    //Test Total : 11 Bytes
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 11);

    //Wafer start time : 12 Bytes (each byte = one value of the date)
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 12);
    lDateTime = CGProberTelMillionToSTDF::StringToDate(lString);

    if(lDateTime.isValid() || lDateTime.isDaylightTime())
    {
        lDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = lDateTime.toTime_t();
    }

    //Wafer start time : 12 Bytes (each byte = one value of the date)
    lIndex += lBinaryObject.ReadString(lBlock+lIndex, lString, 12);
    lDateTime = CGProberTelMillionToSTDF::StringToDate(lString);

    if(lDateTime.isValid() || lDateTime.isDaylightTime())
    {
        lDateTime.setTimeSpec(Qt::UTC);
        m_lEndTime = lDateTime.toTime_t();
    }

    return errNoError;
}

int CGProberTelMillionToSTDF::ReadMapHeaderRecord(QFile &inputFile)
{
    char lBlock[2];

    if (inputFile.read(lBlock, 2) != 2)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int     lIndex = 0;
    short	lWord;
    CGProberTel_StoredBuffer    lBinaryObject;

    //Number of records : 2 Bytes
    //Number of line in this wafer
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    mTotalRecords = lWord;

    return errNoError;
}

int CGProberTelMillionToSTDF::ReadMapRowDataRecord(QFile &inputFile, int totalDies, char **data, int &len)
{
    len     = totalDies*5;
    *data   = new char[len];

    // Read Die Data
    if (inputFile.read(*data, len) != len)
    {
        // Incorrect data...this is not a ProberTel file!
        // Convertion failed.
        m_iLastError = errConversionFailed;

        return errConversionFailed;
    }

    return errNoError;
}

int CGProberTelMillionToSTDF::ReadMapRowHeaderRecord(QFile &inputFile, int &origX, int &origY, int &dieCount)
{
    char lBlock[6];

    if (inputFile.read(lBlock, 6) != 6)
    {
        // Incorrect header...this is not a ProberTel file!
        // Convertion failed.
        return errConversionFailed;
    }

    int     lIndex = 0;
    short	lWord;
    CGProberTel_StoredBuffer    lBinaryObject;

    //First address X of record : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    origX = lWord;

    //First address Y of record : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    origY = lWord;

    //Number of dies : 2 Bytes
    lIndex += lBinaryObject.ReadWord(lBlock+lIndex, &lWord);
    dieCount = lWord;

    return errNoError;
}
