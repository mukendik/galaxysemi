//////////////////////////////////////////////////////////////////////
// import_prober_tsk.cpp: Convert a ProberTsk (Tokyo Electron Limited) file to STDF V4.0
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
#include "import_prober_tsk.h"
#include "dl4_tools.h"		// for read binary data
#include "import_constants.h"
#include "engine.h"

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

bool		gDumpTskRecord = false;

//////////////////////////////////////////////////////////////////////
// BINARY FUNCTIONS
//////////////////////////////////////////////////////////////////////
class CGProberTsk_StoredBuffer : public StoredObject
{
public:

    CGProberTsk_StoredBuffer() {bSameCPUType = !bSameCPUType;}	// Inversion

    gsint32 GetStorageSizeBytes()			   { return 0; };
    gsint32 LoadFromBuffer(const void* /*v*/) { return 0; };

    int	ReadByte(const void *v, BYTE *bData)
    {
        int nResult = StoredObject::ReadByte(v,bData);
        if(gDumpTskRecord)
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
        if(gDumpTskRecord)
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
        if(gDumpTskRecord)
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
        if(gDumpTskRecord)
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
        if(gDumpTskRecord)
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
            if( c == EOF )         /* return FALSE on unexpected EOF */
            {
                szString[i] = '\0';
                return 0;
            }
            szString[i++] = c;

            if(i >= nSize)
                break;
        }
        szString[i] = '\0';

        if(gDumpTskRecord)
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
CGProberTsktoSTDF::CGProberTsktoSTDF()
{
    m_lStartTime = 0;
    m_iTestResultAddress = 0;
    m_iLineCategorySize = 0;
    m_iLineCategoryAddress = 0;
    m_iDataMapVersion = 0;
    m_iMapFileConfiguration = 0;

    m_strWaferFlat = "Up";
    m_nXLocation = -32768;
    m_nYLocation = -32768;

    m_nPartNumber	= 0;
    m_nPassStatus	= -1;
    m_nHardBin		= -1;
    m_nSoftBin		= -1;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGProberTsktoSTDF::~CGProberTsktoSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGProberTsktoSTDF::GetLastError()
{
    QString strMapVersion;
    m_strLastError = "Import ProberTsk: ";

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
        case errUnsupportedMapVerion:
            strMapVersion = QString::number(m_iDataMapVersion) + ":" + GetDataMapVersion();
            m_strLastError += "Invalid file format: Unsupported ProberTsk Map Version ["+strMapVersion+"]...";
            break;
        case errLicenceExpired:
            m_strLastError += "License has expired or Data file out of date...";
            break;
    }

    // Return Error Message
    return m_strLastError;
}
//////////////////////////////////////////////////////////////////////
QString CGProberTsktoSTDF::GetDataMapVersion()
{
    QString strMapVersion;
    switch(m_iDataMapVersion)
    {
        case 0: strMapVersion += "Normal Map Data File"; break;
        case 1: strMapVersion += "Map Data File for 250.000 Chips"; break;
        case 2: strMapVersion += "Map Data File for 256 Multi Chips"; break;
        case 3: strMapVersion += "Map Data File for 256 Multi Chips correspondence form"; break;
        default:strMapVersion += "Unknown Map Data File";
    }
    return strMapVersion;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with ProberTsk format
//////////////////////////////////////////////////////////////////////
bool CGProberTsktoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;
    char	szBlock[PROBER_TSK_BLOCK_SIZE];

    // Open file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        return false;
    }
    int	iIndex;
    int iBlockSize = f.read(szBlock, 61);
    if(iBlockSize != 61)
    {
        // Incorrect header...this is not a ProberTsk file!
        return false;
    }

    char	szString[1024];
    gsint32	dWord;
    short	sWord;
    BYTE	bByte;

    CGProberTsk_StoredBuffer	clBinaryObject;

    iIndex = 0;


    /////////////////////
    // WAFER DATA

    //Operator Name : 20 bytes
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 20);
    //Device Name : 16 bytes
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 16);
    //Wafer Size : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    if((sWord < 40)
    || (sWord > 200))
    {
        f.close();
        return false;
    }
    //Machine No : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    if((sWord < 0)
    || (sWord > 999))
    {
        f.close();
        return false;
    }
    //Index size X : 4 bytes
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    //Index size Y : 4 bytes
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    //Flat Direction : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    if((sWord < 0)
    || (sWord > 360))
    {
        f.close();
        return false;
    }
    //Final Editing Machine : 1 byte
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    //Map Version : 1 byte
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    if((int)bByte > 4)
    {
        f.close();
        return false;
    }

    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the ProberTsk file
//////////////////////////////////////////////////////////////////////
int CGProberTsktoSTDF::ReadProberTskFile(const char *strFileNameSTDF)
{
    QString			strString;
    QString			strSection;
    QString			strValue;
    QString			strSite;
    CGProberTsk_StoredBuffer	clBinaryObject;

    // Debug message
    strString = "---- CGProberTsktoSTDF::ReadProberTskFile(";
    strString += m_strProberTskFileName;
    strString += ")";
    WriteDebugMessageFile(strString);

    // Open ProberTsk file
    m_lStartTime = 0;
    m_lEndTime = -1;
    QFile f( m_strProberTskFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ProberTsk file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return eConvertError;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileLoaded = 0;
    iFileSize = f.size() + 1;

    QCoreApplication::processEvents();

    QFileInfo clFile(m_strProberTskFileName);
    m_strDataFilePath = clFile.absolutePath();

    m_iLastError = ReadHeaderInformationRecord(f);
    if(m_iLastError != errNoError)
    {
        f.close();
        return eConvertError;
    }

    // Read Extension blocks
    // Ignore tem if error
    ReadHeaderInformationExtensionRecord(f);

    ReadTestResultExtensionRecord(f);


    m_iLastError = WriteStdfFile(f, strFileNameSTDF);
    if(m_iLastError != errNoError)
    {
        // Convertion failed.
        QFile::remove(strFileNameSTDF);
        f.close();
        return eConvertError;
    }


    f.close();

    // Success parsing ProberTsk file
    return eConvertSuccess;
}



//////////////////////////////////////////////////////////////////////
// Convert 'FileName' ProberTsk file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
int CGProberTsktoSTDF::Convert(const char *ProberTskFileName, QString &strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(ProberTskFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(ProberTskFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    m_strProberTskFileName = ProberTskFileName;

    int nStatus = ReadProberTskFile(strFileNameSTDF.toLatin1().constData());

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
int	 CGProberTsktoSTDF::ReadBlock(QFile* pFile, char *data, qint64 len)
{
    iFileLoaded += len;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) iFileLoaded > iNextFilePos)
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
// Read Tsk Wafer Data Record
//////////////////////////////////////////////////////////////////////
int CGProberTsktoSTDF::ReadHeaderInformationRecord(QFile &f)
{
    int		iIndex, lPrevIndex;
    char	szBlock[236];
    int		iBlockSize = ReadBlock(&f,szBlock, 236);
    if(iBlockSize != 236)
    {
        // Incorrect header...this is not a ProberTsk file!
        // Convertion failed.
        return errConversionFailed;
    }

    char	szString[1024];

    gsint32	dWord;
    short	sWord;
    BYTE	bByte;


    CGProberTsk_StoredBuffer	clBinaryObject;

    iIndex = 0;

    /////////////////////
    // WAFER TESTING SETUP DATA

    //Operator Name : 20 bytes
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 20);
    m_strOperatorName = QString(szString).trimmed();
    //Device Name : 16 bytes
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 16);
    m_strProductName = QString(szString).trimmed();
    //Wafer Size : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_fWaferSize = sWord;
    // 0.1 inch or mm
    if(m_fWaferSize < 100)
    {
        m_fWaferSize /= 10;
        m_iWaferUnit = 1;
    }
    else
        m_iWaferUnit = 3;
    //Machine No : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_strMachineId = QString::number(sWord);
    //Index size X : 4 bytes
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_fWaferDieSizeX = dWord;
    // 0.01 um
    // Do the conversion to the good unit
    // inch or mm
    // 0.01 um = 0.00001 mm
    // 0.01 um = 0.000001 cm
    // 2.54 cm = 1 inch
    if(m_iWaferUnit == 1)
        m_fWaferDieSizeX /= (2.54 * 1000000);
    else
        m_fWaferDieSizeX /= 100000;
    //Index size Y : 4 bytes
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_fWaferDieSizeY = dWord;
    // Do the conversion to the good unit
    if(m_iWaferUnit == 1)
        m_fWaferDieSizeY /= (2.54 * 1000000);
    else
        m_fWaferDieSizeY /= 100000;
    //Flat Direction : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    if(sWord == 0)
        m_strWaferFlat = "Up";
    else
    if(sWord == 90)
        m_strWaferFlat = "Right";
    else
    if(sWord == 180)
        m_strWaferFlat = "Down";
    else
        m_strWaferFlat = "Left";

    //Final Editing Machine : 1 byte
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    //Map Version : 1 byte
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    m_iDataMapVersion = bByte;
    if((m_iDataMapVersion == 1) || (m_iDataMapVersion > 4))
    {
        // Incorrect header...this is not a ProberTsk file!
        // Convertion failed.
        return errUnsupportedMapVerion;
    }

    //Map row size : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_iWaferRowSize = sWord;
    //Map line size : 2 bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_iWaferLineSize = sWord;
    //Map data form : 4 bytes
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    //Wafer Id : 21 Ascii
    lPrevIndex = iIndex;
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 21);
    m_strWaferId = QString(szString).trimmed();
    iIndex = lPrevIndex + 21;   // To be sure that the new index is correct
    //Measured times
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    m_strRtstCod = QString::number(bByte);
    //Lot No
    lPrevIndex = iIndex;
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 18);
    m_strLotId = QString(szString).trimmed();
    iIndex = lPrevIndex + 18;   // To be sure that the new index is correct
    // Cassette No : 1 Byte
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_strProductId = QString(szString).trimmed();
    //Slot No : 2 Bytes
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_strSubLotId = QString(szString).trimmed();

    // Wafer Probing Coordinate System Data
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);

    // Information per Die
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);

    QDateTime clDateTime;
    int iYear, iMonth, iDay, iHour, iMin, iSec;

    // Wafer Testing Start Time Data
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iYear = 2000 + QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iMonth = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iDay = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iHour = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iMin = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iSec = QString(szString).toInt();

    clDateTime.setDate(QDate(iYear, iMonth, iDay));
    clDateTime.setTime(QTime(iHour,iMin,iSec));

    if(clDateTime.isValid())
    {
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);

    // Wafer Testing End Time Data
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iYear = 2000 + QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iMonth = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iDay = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iHour = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iMin = QString(szString).toInt();
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iSec = QString(szString).toInt();

    clDateTime.setDate(QDate(iYear, iMonth, iDay));
    clDateTime.setTime(QTime(iHour,iMin,iSec));

    if(clDateTime.isValid())
    {
        clDateTime.setTimeSpec(Qt::UTC);
        m_lEndTime = clDateTime.toTime_t();
    }

    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);

    // Wafer Loading Time Data
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);

    // Wafer Unloading Time Data
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadString(szBlock+iIndex, szString, 2);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);

    // Machine No
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);

    // Special Letters
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);

    // Testing Result
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_iTotalDies = sWord;
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_iTotalDiesGood = sWord;
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_iTotalDiesFail = sWord;

    // Test Die Information Address
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_iTestResultAddress = dWord;

    // Number of Line category data
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_iLineCategorySize = dWord;

    // Line category address
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_iLineCategoryAddress = dWord;

    // Extended Map Information
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    m_iMapFileConfiguration = sWord;
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
    iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);

    return errNoError;
}
//////////////////////////////////////////////////////////////////////
int CGProberTsktoSTDF::ReadHeaderInformationExtensionRecord(QFile &f)
{
    if((m_iMapFileConfiguration & 0x8) == 0)
        return errNoError;

    // Goto header extension position
    int		iIndex;
    iIndex = 0;
    // Header Information Size
    iIndex += 236;
    // Test Result Information Size (row*line*6)
    iIndex += (m_iWaferRowSize*m_iWaferLineSize*6);
    // Line Category Size
    if((m_iMapFileConfiguration & 0x4) != 0)
        iIndex += m_iLineCategorySize*8;

    f.seek(iIndex);

    char	szBlock[172];
    int		iBlockSize = ReadBlock(&f,szBlock, 172);
    if(iBlockSize != 172)
    {
        // Convertion failed.
        return errConversionFailed;
    }

    gsint32	dWord;

    CGProberTsk_StoredBuffer	clBinaryObject;

    iIndex = 52;

    /////////////////////
    // WAFER TEST RESULT

    // Testing Result
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_iTotalDies = dWord;
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_iTotalDiesGood = dWord;
    iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &dWord);
    m_iTotalDiesFail = dWord;

    return errNoError;

}

//////////////////////////////////////////////////////////////////////
int CGProberTsktoSTDF::ReadTestResultExtensionRecord(QFile &f)
{
    // Only for 256 Multi
    if(m_iDataMapVersion < 2)
        return errNoError;

    // Check if Extented section exist
    if((m_iMapFileConfiguration & 0x10) == 0)
        return errNoError;

    // Goto header extension position
    int		iIndex;
    iIndex = 0;
    // Header Information Size
    iIndex += 236;
    // Test Result Information Size (row*line*6)
    iIndex += (m_iWaferRowSize*m_iWaferLineSize*6);
    // Line Category Size
    if((m_iMapFileConfiguration & 0x4) != 0)
        iIndex += m_iLineCategorySize*8;
    // Header Information Extension Size
    if((m_iMapFileConfiguration & 0x8) != 0)
        iIndex += 172;

    f.seek(iIndex);

    char	szBlock[4];
    short	sWord;
    int		iBlockSize;
    int		iTotalRecords;
    int		iRecordIndex;

    // Check if find some correct data
    bool bHaveData = false;

    CGProberTsk_StoredBuffer	clBinaryObject;
    iTotalRecords = m_iWaferRowSize*m_iWaferLineSize;

    // Read Test Result per Die
    for(iRecordIndex=0; iRecordIndex<iTotalRecords; iRecordIndex++)
    {
        if(f.atEnd())
            break;

        iBlockSize = ReadBlock(&f,szBlock, 4);
        if(iBlockSize != 4)
        {
            m_lstExtendedTestResult.clear();
            return errConversionFailed;
        }

        // J'ai un décalage de 2 byte ???
        // INCOMPREHENSIBLE
        // Lecture du deuxième word si le premier est null ?
        iIndex = 0;

        // First word
        iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
        if(sWord == 0)
            iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
        if(sWord != 0)
            bHaveData = true;

        m_lstExtendedTestResult.append(sWord);
    }

    if(!bHaveData)
        m_lstExtendedTestResult.clear();

    return errNoError;

}

//////////////////////////////////////////////////////////////////////
// Create and write Stdf file
//////////////////////////////////////////////////////////////////////
int CGProberTsktoSTDF::WriteStdfFile(QFile &f, const char *strFileNameSTDF)
{

    // Only for Normal and 256 Multi
    if(m_iMapFileConfiguration == 1)
        return errConversionFailed;

    // First check if have Test Result Section
    // and go to the good address
    if(m_iTestResultAddress > 0)
        f.seek(m_iTestResultAddress);

    int		iIndex;
    int		iBlockSize;
    char	szBlock[6];
    short	sWord;

    CGProberTsk_StoredBuffer	clBinaryObject;
    iIndex = 0;

    /////////////////////
    // MAP DATA

    QMap<int, CGProberTskBinInfo> mapHBinInfo;
    QMap<int, CGProberTskBinInfo> mapSBinInfo;

    int iTotalRecords;
    int iRecordIndex;

    int iTotalGoodBin;
    int iTotalFailBin;
    int iPartNumber;
    int iXpos;
    int iYpos;
    int iHBin=0;
    int iSBin=0;
    int iSite;
    bool bPass;
    bool bSkipDie;
    bool bUntested;
    bool bHaveResult;


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
        return errWriteSTDF;
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
    StdfFile.WriteByte((BYTE) m_strRtstCod[0].toLatin1());	// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strProductId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("Prober Tokyo 90A/UF Serie");			// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString(GetDataMapVersion().toLatin1().constData());				// exec-type
    StdfFile.WriteString(QString::number(m_iDataMapVersion).toLatin1().constData());// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":PROBER_TSK";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");							// aux-file
    StdfFile.WriteString("");							// package-type
    StdfFile.WriteString("90A/UF");						// familyID
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

    // Write WCR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(m_fWaferSize);					// Wafer size
    StdfFile.WriteFloat(m_fWaferDieSizeX);				// Height of die
    StdfFile.WriteFloat(m_fWaferDieSizeY);				// Width of die
    StdfFile.WriteByte(m_iWaferUnit);					// Units
    StdfFile.WriteByte((BYTE) m_strWaferFlat[0].toLatin1());	// Wafer Flat
    StdfFile.WriteRecord();


    iTotalRecords = m_iWaferRowSize*m_iWaferLineSize;

    // Read Test Result per Die
    for(iRecordIndex=0; iRecordIndex<iTotalRecords; iRecordIndex++)
    {
        if(m_iTestResultAddress == 0)
            break;

        if(f.atEnd())
            break;

        bPass = bSkipDie = bUntested = bHaveResult = false;

        iBlockSize = ReadBlock(&f,szBlock, 6);
        if(iBlockSize != 6)
        {
            StdfFile.Close();

            // Incorrect header...this is not a ProberTsk file!
            // Convertion failed.
            return errConversionFailed;
        }

        iIndex = 0;

        // First word
        iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
        bUntested = ((0xC000 & sWord) == 0);
        if(!bUntested)
        {
            iHBin = (0xC000 & sWord) >> 14;
            bPass = (iHBin == 1);

        }
        iXpos = (0x01FF) & sWord;

        // Second word
        iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
        bSkipDie = ((0xC000 & sWord) == 0);
        iYpos = (0x01FF) & sWord;

        // Third word
        iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
        bHaveResult = (0x8000 & sWord) != 0;
        iSite = (0x3F00 & sWord) >> 8;
        iSBin = (0x003F & sWord);


        if(!m_lstExtendedTestResult.isEmpty())
        {
            sWord = m_lstExtendedTestResult.takeFirst();
            iSite = (0xFF00 & sWord) >> 8;
            iSBin = (0x00FF & sWord);
        }


        if(bSkipDie || bUntested)
            continue;

        // Add 1 to calculate actual site number and category data
        iSite++;
        iSBin++;

        if(!mapHBinInfo.contains(iHBin))
        {
            if(iHBin == 1)
                mapHBinInfo[iHBin].strBinName = "Pass";
            else
                mapHBinInfo[iHBin].strBinName = "Fail " + QString::number(iHBin-1);

            mapHBinInfo[iHBin].bPass = bPass;
            mapHBinInfo[iHBin].nBinNumber = iHBin;
            mapHBinInfo[iHBin].nNbCnt = 0;
        }

        mapHBinInfo[iHBin].nNbCnt++;

        if(!mapSBinInfo.contains(iSBin))
        {
            mapSBinInfo[iSBin].bPass = bPass;
            mapSBinInfo[iSBin].nBinNumber = iSBin;
            mapSBinInfo[iSBin].nNbCnt = 0;
        }

        mapSBinInfo[iSBin].nNbCnt++;

        // Write PIR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(iSite);		// Tester site
        StdfFile.WriteRecord();

        iPartNumber++;

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(iSite);		// Tester site#:1
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
        StdfFile.WriteWord(iHBin);				// HARD_BIN
        StdfFile.WriteWord(iSBin);				// SOFT_BIN
        StdfFile.WriteWord(iXpos);				// X_COORD
        StdfFile.WriteWord(iYpos);				// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
        StdfFile.WriteString("");			// PART_TXT
        StdfFile.WriteString("");			// PART_FIX
        StdfFile.WriteRecord();

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

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CGProberTskBinInfo>::Iterator itMapBin;
    for ( itMapBin = mapHBinInfo.begin(); itMapBin != mapHBinInfo.end(); ++itMapBin )
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
        StdfFile.WriteString(itMapBin.value().strBinName.toLatin1().constData());
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = mapSBinInfo.begin(); itMapBin != mapSBinInfo.end(); ++itMapBin )
    {
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value().nBinNumber);	// SBIN = 0
        StdfFile.WriteDword(itMapBin.value().nNbCnt);	// Total Bins
        if(itMapBin.value().bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value().strBinName.toLatin1().constData());
        StdfFile.WriteRecord();
    }

    mapHBinInfo.clear();
    mapSBinInfo.clear();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lEndTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    return errNoError;
}

