//////////////////////////////////////////////////////////////////////
// import_fet_test.cpp: Convert a FET_TEST (TMT) file to STDF V4.0
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
#include <QVector>
#include <limits>

#include "parserPromisFile.h"
#include "import_fet_test.h"
#include "dl4_tools.h"		// for read binary data
#include "importConstants.h"
#include "converter_external_file.h"
#include <gqtl_log.h>
#include "bin_map_store_factory.h"
#include "bin_map_item_base.h"
#include "bin_mapping_exceptions.h"

// Binary File format:
//                            9400 Data File (.DAT)
//                            ---------------------
//Memory
//(Byte)      Description                            Name           Type
//------      -----------                            ----           ----
//MAX. 32 Test, 8 Segment, (1536 Byte/Record)
//--------------
//Header Record (#0)
//Byte
//------------------
//1           "D" 1 Char ASCII (Data File)                          ASCII
//2           "4" 1 Char ASCII (9400)                               ASCII
//3-4         Start SN (1-32,766)                    SSN            Fixed Binary
//5-6         End SN (1-32,766)                      ESN            Fixed Binary
//7           Number of SN's per 1536 Byte Record    SNNUM          Fixed (7)
//             (1-192)
//8-9         SN Size (8-1283) Bytes                 SNSIZE         Fixed Binary
//10-41       32 Test #'s of DL Sort                                Binary
//42-73       32 FCN #'s of Test in DL Sort                         Binary
//74-113      Data File Rem  39 Char Var                            Var ASCII
//114-153     Data File Date 39 Char Var                            Var ASCII
//154         Start Segment (1-8) Fixed (7)                         Binary
//155         End Segment (1-8) Fixed (7)                           Binary
//156         Flags  Bit (0)                         DFG1           Binary
//             Bit 0  1=Wafermap (X,Y Co-ordinate)
//                    0=Non Wafermap (Serial #)
//             Bit 1  0=Not Converted to Version 12
//                    1=Converted to Version 12
//157         Starting X co-ordinate  Bit (8)        SCX            Binary
//158         Starting Y co-ordinate  Bit (8)        SCY            Binary
//159         Ending X co-ordinate  Bit (8)          ECX            Binary
//160         Ending Y co-ordinate  Bit (8)          ECY            Binary
//161-164     Spare
//165-179     Run filename at creation time (14 char var ascii)     Var Ascii
//180-194     Test filename at creation time (14 char var ascii)    Var Ascii
//195-1536    Spare
//
//Data Record #1-N
//Byte
//**************
//1-2         Serial Number  0=None (0-32,766)                      Fixed Binary
//             Or If Wafermapping
//             Byte 1   X Co-ordinate                               Binary
//             Byte 2   Y Co-ordinate                               Binary
//3           Bin Result Bits 0-4=(1 to 24),Bit 7=1 Reject          Binary
//4           Flag Test #1                           RSLTF          Binary
//             Bit 0  Cover on Pass=1
//             Bit 1  Cover on Fail=1
//             Bit 2  Test Done=1
//             Bit 3  Test Fail=1
//             Bit 4  Test Over=1
//             Bit 5  Data Logged=1
//             Bit 6  Test Less=1
//             Bit 7  Result Converted to Floating
//
//                        -------------------------------
//Memory
//(Byte)      Description                            Name           Type
//------      -----------                            ----           ----
//                    Point=1
//5-8         Test #1 Seg #1 Result                                 Fl. Pt.
//If Single Seg. Single Test
//**************
//9-10        Serial Number + 1                                     Fixed Binary
//             Or If Wafermapping
//             Byte 9  X Co-ordinate                                Binary
//             Byte 10 Y Co-ordinate                                Binary
//11          Bin Result                                            Binary
//12          Flag                                   RSLTF          Binary
//13-16       Test #1 Seg #1 Result etc up to MAX                   Fl. Pt.
//             that fits in 1536 Byte record
//
//If Single Seg. Multiple Test
//**************
//9           Test #2  Flag                          RSLTF          Binary
//10-13       Test #2  Result etc up to MAX that                    Fl. Pt.
//             fits in 1536 Byte record
//14-18       Test #3
//etc
//**************
//If Single Test Multiple Seg.
//**************
//9           Test #1 Seg #2  Flag                   RSLTF          Binary
//10-13       Test #1 Seg #2  Result etc up to MAX                  Fl. Pt.
//             that fits in 1536 Byte record
//14-18               Seg #N
//etc
//**************
//If Multiple Test Multiple Seg.
//**************
//9           Test #1 Seg #2  Flag                   RSLTF          Binary
//10-13       Test #1 Seg #2  Result                                Fl. Pt.
//14-18             .
//etc               .
//            Test #1 Seg #N  Flag                   RSLTF          Binary
//            Test #1 Seg #N  Result                                Fl. Pt.
//                  .
//                  .
//            Test #N Seg #N  Flag                   RSLTF          Binary
//            Test #N Seg #N  Result                                Fl. Pt.
//**************
//
//SNSIZE=[(Test Cnt) x (Segment Cnt) x (5 Bytes)] + 3 Bytes
// #Bytes per serial number (8-1283 Bytes)
//**************
//SNNUM=1536/SNSIZE  Fractional part is truncated
// #Serial Number per record (1-192)
//
// main.cpp

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

const char* cPassBin = "PASS";

namespace GS
{
namespace Parser
{

static const char REGEXP_FETTEST_QA_FT_LVM[] = "[a-zA-Z\\d-]+_([a-zA-Z\\d]+)\\.(\\d{1,2})[ABab]?_FT_\\d{14}\\.(.+)";

class Vishay_BinMap
{
public:
    Vishay_BinMap()
    {
    }

    Vishay_BinMap(const Vishay_BinMap& source)
    {
        nTestNumber = source.nTestNumber;
        strTestName = source.strTestName;
        nBinning = source.nBinning;
        strBinName = source.strBinName;
        nNewTestNumber = source.nNewTestNumber;
    }

    Vishay_BinMap& operator=(const Vishay_BinMap& source)	// assignment operator
    {
        nTestNumber = source.nTestNumber;
        strTestName = source.strTestName;
        nBinning = source.nBinning;
        strBinName = source.strBinName;
        nNewTestNumber = source.nNewTestNumber;
        return *this;
    }

    QString	strTestName;
    QString	strBinName;
    int		nTestNumber;
    int		nBinning;
    int		nNewTestNumber;
};

//////////////////////////////////////////////////////////////////////
class StoredBuffer : public StoredObject
{
public:
    gsint32 GetStorageSizeBytes()			   { return 0; }
    gsint32 LoadFromBuffer(const void* /*v*/) { return 0; }
};


//////////////////////////////////////////////////////////////////////
// Return the mapped Test name (and test number) for a given test#
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::GetFetTestName(int aTestNumber, QString & aTestName)
{
    // Check if FetTest Test->Binning mapping file is present.
    // If so, use this mapping file to get test names from test numbers
    if(mBinMapStore.isNull() == false)
    {
       return RetrieveTestName(aTestNumber, aTestName);
    }

    // If there is no promis ft file, use hard coded test names, otherwise return false
    if(m_bPromisFtFile == false)
    {
        // No FetTest Test->Binning mapping file, use hard-coded test names
        switch(aTestNumber)
        {
        case 1:
            aTestName = "IGSS @ + 100% VGS";
            break;

        case 2:
            aTestName = "IGSS @ - 100% VGS";
            break;

        case 3:
            aTestName = "IGSS @ + 120% VGS";
            break;

        case 4:
            aTestName = "IGSS @ - 120% VGS";
            break;

        case 5:
            aTestName = "GS SHORT";
            break;

        case 11:
            aTestName = "IGDO";
            break;

        case 20:
            aTestName = "IR#2";
            break;

        case 21:
            aTestName = "VF @ + 100% IF";
            break;

        case 22:
            aTestName = "VF @ - 100% IF";
            break;

        case 23:
            aTestName = "VF @ + 50% IF";
            break;

        case 24:
            aTestName = "VF @ - 50% IF";
            break;

        case 25:
            aTestName = "D VF#1";
            break;

        case 26:
            aTestName = "Vth(OPEN)";
            break;

        case 27:
            aTestName = "Vth1(MIN~MAX)";
            break;

        case 28:
            aTestName = "Vth1(MIN)";
            break;

        case 29:
            aTestName = "Vth1(MAX)";
            break;

        case 30:
            aTestName = "VR";
            break;

        case 31:
            aTestName = "IGSS @ + High Voltage";
            break;

        case 32:
            aTestName = "IGSS @ - High Volatge";
            break;

        case 33:
            aTestName = "IR#1";
            break;

        case 34:
            aTestName = "D VF#2";
            break;

        case 50:
            aTestName = "IBEV";
            break;

        case 51:
            aTestName = "IDSX(100%VDS)";
            break;

        case 52:
            aTestName = "IDSX(50%VDS)";
            break;

        case 53:
            aTestName = "DS SHORT";
            break;

        case 54:
            aTestName = "IDSX  @High Voltage";
            break;

        case 76:
            aTestName = "BVDSX @ 250uA";
            break;

        case 77:
            aTestName = "BVDSX @ 5mA";
            break;

        case 78:
            aTestName = "BVDSX @ 10mA";
            break;

        case 79:
            aTestName = "D BV#1";
            break;

        case 80:
            aTestName = "D BV#2";
            break;

        case 100:
            aTestName = "VDSP_UIS";
            break;

        case 102:
            aTestName = "VDSP";
            break;

        case 103:
            aTestName = "UIS";
            break;

        case 104:
            aTestName = "IDSX(100%VDS after UIS)";
            break;

        case 126:
            aTestName = "rDSon #1 @ VGS= 10V";
            break;

        case 127:
            aTestName = "rDSon #2 @ VGS= 4.5V";
            break;

        case 128:
            aTestName = "rDSon #3 @ VGS= 2.5V";
            break;

        case 129:
            aTestName = "rDSon #4 @ VGS= 1.8V";
            break;

        case 130:
            aTestName = "rDSon #5 @ VGS= 1.5V";
            break;

        case 131:
            aTestName = "rDSon #6 @ VGS= 1.2V";
            break;

        case 133:
            aTestName = "rDSon #7 @ VGS= 2.7V";
            break;

        case 134:
            aTestName = "rDSon #8 @ VGS= 6V";
            break;

        case 201:
            aTestName = "VSDP";
            break;

        default:
            aTestName = "T"+QString::number(aTestNumber);
            break;
        }

        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGFET_TESTtoSTDF::CGFET_TESTtoSTDF(): ParserBase(typeFetTest, "typeFetTest")
{
    mConverterStatus = ConvertSuccess;

    m_lStartTime = 0;
    m_iSSN = 0;
    m_iESN = 0;
    m_iSNNUM = 0;
    m_iSNSIZE = 0;
    m_iTestNumber = 0;
    m_iSegNumber = 0;
    m_iStartSegment = 0;
    m_iEndSegment = 0;
    m_bWaferMap = false;
    m_iStartingX = 0;
    m_iStartingY = 0;
    m_iEndingX = 0;
    m_iEndingY = 0;
    m_bPromisFtFile = false;
    mIsNewFormat = false;
    mStartLargeSN = 0;
    mEndLargeSN = 0;
    m_strSubLotId = m_strLotId= m_strOperator = m_strProber = "" ;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGFET_TESTtoSTDF::~CGFET_TESTtoSTDF()
{
    QMap<int,ParserBinning*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
    for ( itMapBin = m_qMapBins_Soft.begin(); itMapBin != m_qMapBins_Soft.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
    QMap<int,ParserParameter*>::Iterator itMapParam;
    for ( itMapParam = m_qMapParameterList.begin(); itMapParam != m_qMapParameterList.end(); ++itMapParam )
    {
        delete itMapParam.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
ConverterStatus CGFET_TESTtoSTDF::GetConvertStatus() const
{
    return mConverterStatus;
}

int CGFET_TESTtoSTDF::GetLookupTestNumber(int aTestNumber) const
{
    int lLookUpTestNumber = aTestNumber;

    if (m_bWaferMap == false)
    {
        // When test# > 100 and <= 200, substract 100 to execute the look up in the bin mapping file
        if (aTestNumber > 100 && aTestNumber <= 200)
            lLookUpTestNumber -= 100;
    }

    return lLookUpTestNumber;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with FET_TEST format
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::IsCompatible(const QString& szFileName)
{
    char	szBlock[FET_TEST_BLOCK_SIZE];

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    int	iIndex;
    int iBlockSize = f.read(szBlock, FET_TEST_BLOCK_SIZE);
    if(iBlockSize != FET_TEST_BLOCK_SIZE)
    {
        // Incorrect header...this is not a FET_TEST file!
        // Close file
        f.close();
        return false;
    }

    BYTE	bByte;
    StoredBuffer	clBinaryObject;

    iIndex = 0;
    //"D" 1 Char ASCII (Data File)
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    if(bByte != 'D')
    {
        // Incorrect header...this is not a FET_TEST file!
        // Close file
        f.close();
        return false;
    }
    //"4" 1 Char ASCII (9400)
    iIndex += clBinaryObject.ReadByte(szBlock+iIndex, &bByte);
    if(bByte != '4')
    {
        // Incorrect header...this is not a FET_TEST file!
        // Close file
        f.close();
        return false;
    }
    f.close();

    return true;
}


///////////////////////////////////////////////////////////////////////
// Read and Parse TEST->BIN mapping file
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadBinMapFile(QString strBinmapFileName, QString strType, QString strFormat)
{
    if(strBinmapFileName.isEmpty())
        return true;

    if(strType == "wafer")
    {
        if(strFormat == "vishay-hvm")
        {
            m_specificFlowType = FET_FLOW_HVM_WS;
            return ReadBinMapFile_HVM_WS(strBinmapFileName);
        }
        else
        {
            m_specificFlowType = FET_FLOW_LVM_WS;
            return ReadBinMapFile_LVM_WS(strBinmapFileName);
        }
    }
    else
        if(strType == "final")
        {
            if(strFormat == "vishay-hvm")
            {
                m_specificFlowType = FET_FLOW_HVM_FT;
                return ReadBinMapFile_HVM_FT(strBinmapFileName);
            }
            else
            {
                m_specificFlowType = FET_FLOW_LVM_FT;
                return ReadBinMapFile_LVM_FT(strBinmapFileName);
            }
        }
        else
            return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse TEST->BIN mapping file (Wafer Sort)
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadBinMapFile_LVM_WS(QString strBinmapFileName)
{
    try
    {
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ws >
                            ::MakeBinMapStore( strBinmapFileName.toStdString(),
                                               mExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse TEST->BIN mapping file (Wafer Sort)
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadBinMapFile_HVM_WS(QString strBinmapFileName)
{

    try
    {
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::hvm_ws_fet_test >
                            ::MakeBinMapStore( strBinmapFileName.toStdString(),
                                               mExternalFilePath.toStdString() ) );


    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse TEST->BIN mapping file (Final Test)
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadBinMapFile_LVM_FT(QString strBinmapFileName)
{
    try
    {
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ft_ft >
                            ::MakeBinMapStore( strBinmapFileName.toStdString(),
                                               mExternalFilePath.toStdString() ) );
        //GCORE-16441
        QString lErrorMsg = "";
        if(!getOptDefaultBinValuesfromExternalFile(mExternalFilePath, lErrorMsg))
        {
            mLastErrorMessage = lErrorMsg;
            return false;
            //GSLOG(SYSLOG_SEV_INFORMATIONAL, qPrintable("getOptDefaultBinValuesfromExternalFile return false: " + lErrorMsg));
        }
    }
    catch(const Qx::BinMapping::InvalidLvmFtFtBinMappingFileFormat &lException)
    {
        mLastErrorMessage = lException.what();
        return false;
    }
    catch(const Qx::BinMapping::CannotOpenBinMappingFile &lException)
    {
        mLastErrorMessage = lException.what();
        return false;
    }
    catch(const Qx::BinMapping::DuplicatedTestInLvmFtBinMappingFile &lException)
    {
        mLastErrorMessage = lException.what();
        return false;
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse TEST->BIN mapping file (Final Test)
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadBinMapFile_HVM_FT(QString strBinmapFileName)
{
    // Debug message
    QString strString = "---- CGFET_TESTtoSTDF::ReadBinMapFile_HVM_FT(): from file " + strBinmapFileName;
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());
    strString = "---- NOT IMPLEMENTED ";
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());

    mLastError = errReadBinMapFile;
    mLastErrorMessage = "Incorrect BinMap format [HVM FT].\n";
    mLastErrorMessage+= "---- NOT IMPLEMENTED";


    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the FET_TEST file
//////////////////////////////////////////////////////////////////////
ConverterStatus CGFET_TESTtoSTDF::ReadFetTestFile(const QString& FetTestFileName,  QString& strFileNameSTDF)
{
    QString       strString;
    StoredBuffer  clBinaryObject;

    // Debug message
    strString = "---- CGFET_TESTtoSTDF::ReadFetTestFile(";
    strString += FetTestFileName;
    strString += ", ";
    strString += strFileNameSTDF;
    strString += ")";
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());

    // Open FET_TEST file
    m_strFetTestFileName = FetTestFileName;
    m_lStartTime = 0;
    QFile f( FetTestFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening FET_TEST file
        mLastError = errOpenFail;

        // Convertion failed.
        return ConvertError;
    }

    QFileInfo clFile(FetTestFileName);
    m_strDataFilePath = clFile.absolutePath();

    int	i,iIndex;
    int iBlockSize = ReadBlock(&f,m_szBlock, FET_TEST_BLOCK_SIZE);
    if(iBlockSize != FET_TEST_BLOCK_SIZE)
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return ConvertError;
    }

    BYTE	bByte;
    short	sShort;
    char	szString[256];

    iIndex = 0;
    //"D" 1 Char ASCII (Data File)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    if(bByte != 'D')
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return ConvertError;
    }
    //"4" 1 Char ASCII (9400)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    if(bByte != '4')
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return ConvertError;
    }
    //Start SN (1-32,766)
    iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
    m_iSSN = (int) sShort;
    //End SN (1-32,766)
    iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
    m_iESN = (int) sShort;
    //Number of SN's per 1536 Byte Record
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iSNNUM = (int) bByte;
    //SN Size (8-1283) Bytes
    iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
    m_iSNSIZE = (int) sShort;
    //32 Test #'s of DL Sort
    ParserParameter *pTestParameter;
    for(i=0 ; i<32 ; i++)
    {
        iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        if(bByte != 0)
            m_iTestNumber = i+1;

        m_iTestN[i] = (int) bByte;
        if(m_iTestN[i] > 0)
        {
            pTestParameter = new ParserParameter();
            pTestParameter->SetTestNumber(m_iTestN[i]);
            m_qMapParameterList[m_iTestN[i]] = pTestParameter;
        }
    }
    // compute the Segment number
    // SNSIZE=(TestCnt * SegCnt * 5) + 3
    m_iSegNumber = (m_iSNSIZE - 3)/5 / m_iTestNumber;

    //32 FCN #'s of Test in DL Sort
    for(i=0 ; i<32 ; i++)
    {
        iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        m_iFcnN[i] = (int) bByte;
        if(m_iFcnN[i] > 0)
        {
            if(m_qMapParameterList.contains(m_iTestN[i]))
            {
                pTestParameter = m_qMapParameterList[m_iTestN[i]];
                // pTestParameter->m_nFunctionNum = m_iFcnN[i];
            }
        }
    }
    //Data File Rem  39 Char Var
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strDataFileRem = szString;
    iIndex += 40;
    //Data File Date 39 Char Var
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strDataFileDate = szString;
    iIndex += 40;
    //Start Segment (1-8) Fixed (7)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iStartSegment = (int) bByte;
    //End Segment (1-8) Fixed (7)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iEndSegment = (int) bByte;
    //Flags  Bit (0)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_bWaferMap = bByte & BIT0;
    mIsNewFormat = bByte & BIT2;

    //Starting X co-ordinate  Bit (8)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iStartingX = (int) bByte;
    //Starting Y co-ordinate  Bit (8)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iStartingY = (int) bByte;
    //Ending X co-ordinate  Bit (8)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iEndingX = (int) bByte;
    //Ending Y co-ordinate  Bit (8)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    m_iEndingY = (int) bByte;
    //Spare
    iIndex = 164;
    //Run filename at creation time (14 char var ascii)
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strRunFile = szString;
    iIndex += 15;
    //Test filename at creation time (14 char var ascii)
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strTestFile = szString;
    iIndex += 15;

    if( mIsNewFormat )
    {
        iIndex = 423;
        clBinaryObject.ReadDword( m_szBlock + iIndex, &mStartLargeSN );
        iIndex += 4;
        clBinaryObject.ReadDword( m_szBlock + iIndex, &mEndLargeSN);
    }

    QRegExp clRegExp("[-/=\\s]");
    QString	strMonth = m_strDataFileDate.section(clRegExp,0,0).simplified();
    QString	strDay = m_strDataFileDate.section(clRegExp,1,1).simplified();
    QString	strYear = m_strDataFileDate.section(clRegExp,2,2).simplified();

    // If no date exists, use current date
    if(strDay.isEmpty() || strMonth.isEmpty() || strYear.isEmpty())
    {
        strDay = QString::number(QDate::currentDate().day());
        strMonth = QString::number(QDate::currentDate().month());
        strYear = QString::number(QDate::currentDate().year());
    }

    if(!strMonth.isEmpty() && !strDay.isEmpty() && !strYear.isEmpty())
    {
        if(strYear.toInt() < 100)
            strYear = QString::number(2000+strYear.toInt());
        if(strMonth.toInt() == 0)
        {
            // try to find the good month
            for(i=1; i>=12; i++)
            {
                if(strMonth.startsWith(QDate::shortMonthName(i), Qt::CaseInsensitive))
                {
                    strMonth = QString::number(i);
                    break;
                }
            }
        }

        QDate clDate(strYear.toInt(), strMonth.toInt(), strDay.toInt());
        QTime clTime;
        QDateTime clDateTime(clDate,clTime);
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    QString strCprFile = clFile.baseName() + ".CPR";

    ReadCprFile(strCprFile.toLatin1().constData());

    if(!m_strTestFile.isEmpty())
        ReadTesFile(m_strTestFile.toLatin1().constData());

    // Check if we could retrieve a date, else use .dat creation date
    if(m_lStartTime < 0)
    {
        QDateTime clDateTime = QDateTime::currentDateTime();
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    mConverterStatus = WriteStdfFile(&f,strFileNameSTDF);
    if(mConverterStatus != ConvertSuccess)
    {
        QFile::remove(strFileNameSTDF);
    }

    f.close();

    // Return parsing FET_TEST file
    return mConverterStatus;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the FET_TEST .TES file
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadTesFile(const char* TestFileName)
{
    QString       strString;
    StoredBuffer  clBinaryObject;

    // Debug message
    strString = "---- CGFET_TESTtoSTDF::ReadTesFile(";
    strString += TestFileName;
    strString += ")";
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());

    // Open FET_TEST file
    strString = m_strDataFilePath + "/";
    strString += TestFileName;
    QFile f(strString.toLatin1().constData() );
    // file doesn't exist
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Check if .TES is not in <fettest> sub-folder....
        strString = m_strDataFilePath + "/fettest/";
        strString += TestFileName;
        f.setFileName(strString);

        if(!f.open( QIODevice::ReadOnly ))
        {
            // .TES file not found....!
            mLastError = errInvalidFormatParameter;

            // Convertion failed.
            return false;
        }
    }

    int i,iIndex;
    int iBlockSize = f.read(m_szBlock, 96);
    if(iBlockSize != 96)
    {
        // Incorrect header...this is not a FET_TEST test file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    BYTE  bByte;
    BYTE  bByte15;
    short sShort;
    float fFloat;
    char  szString[256];

    iIndex = 0;
    //"D" 1 Char ASCII (Data File)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    if(bByte != 'T')
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    //"4" 1 Char ASCII (9400)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    if(bByte != '4')
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }


    //File Name 10 Char Var
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 11;
    //Bits 14
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    //Bits 15
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    bByte15 = bByte;
    //Bits 16
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    //Rem 39 Variable ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 40;
    //Date 39 Variable ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 40;

    // iIndex = 96
    // Read the next record
    ParserParameter *pTestParameter;
    bool  bIsNAN=false;
    int   iFnNumber;
    int   iNbTests = 125;
    if(bByte15 & BIT0)
        iNbTests = 250;

    // i is the test number
    for(i=1; i<=iNbTests; i++)
    {
        // read next block of 50 bytes
        iBlockSize = ReadBlock(&f,m_szBlock, 50);
        if(iBlockSize != 50)
        {
            // Incorrect header...this is not a FET_TEST test file!
            mLastError = errInvalidFormatParameter;

            // Convertion failed.
            // Close file
            f.close();
            return false;
        }
        if(!m_qMapParameterList.contains(i))
            continue;

        pTestParameter = m_qMapParameterList[i];

        iIndex = 0;
        //Function #
        iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        iFnNumber = (int)bByte;
        //Min Limit
        iIndex += clBinaryObject.ReadFloat(m_szBlock+iIndex, &fFloat, &bIsNAN);
        if(!bIsNAN && (-1e38 < fFloat) && (fFloat< 1e38))
        {
            pTestParameter->SetValidLowLimit(true);
            pTestParameter->SetLowLimit(fFloat);
        }
        //Max Limit
        iIndex += clBinaryObject.ReadFloat(m_szBlock+iIndex, &fFloat, &bIsNAN);
        if(!bIsNAN && (-1e38 < fFloat) && (fFloat< 1e38))
        {
            pTestParameter->SetValidHighLimit(true);
            pTestParameter->SetHighLimit(fFloat);
        }
        //Cover on Pass
        iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        //Cover on Fail
        iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        if(iFnNumber != 68)
        {
            //Auto Rng/Result Rng
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Cur. Rng/Start Rng
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //DCS Rng/PLC
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //V3 Volts
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V3 Cond. Code
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //V2 Volts
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V2 Current
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V2 Range
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //V1B Volts
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V1B Current
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V1B Range
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //V1A Volts
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V1A Current
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //V1A Range
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Test Time
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //Test Time 2 (300 NA)
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //Test Time 3 (30 NA)
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //Test Time 4 (3 NA)
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //Test Time 5 (300 PA)
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //Mod. DAC or IC/IB Rng.
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            //Bits45
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits46
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits47
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits48
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            // Signed or Absolute Limit Comparaison
            if(!(bByte & BIT0))
            {
                // Absolute comparison
                // true if have to make an absolute comparison else signed comparison
                pTestParameter->SetSpecificFlags(1);
                if(pTestParameter->GetValidLowLimit() && (pTestParameter->GetLowLimit()<0))
                    pTestParameter->SetLowLimit(-pTestParameter->GetLowLimit());
                if(pTestParameter->GetValidHighLimit() && (pTestParameter->GetHighLimit()<0))
                    pTestParameter->SetHighLimit(-pTestParameter->GetHighLimit());

            }
            //Bits49
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits50
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        }
        else
        {
            //Result Label
            clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
            strString = szString;
            iIndex += 7;
            //Result Unit
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            pTestParameter->SetTestUnit(QString(bByte));
            //Destination Test #
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Destination Function #
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Destination Forcing Parameter Range
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Destination Parameter Label
            clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
            strString = szString;
            iIndex += 4;
            //Spare
            iIndex = 30;
            //Constant
            iIndex += clBinaryObject.ReadFloat(m_szBlock+iIndex, &fFloat);
            //Test #1
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Test #2
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Operator #1
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Operator #2
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Segment #1
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Segment #2
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Spare
            iIndex = 45;
            //Bits45
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits46
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits47
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits48
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            // Signed or Absolute Limit Comparaison
            if(!(bByte & BIT0))
            {
                // Absolute comparison
                // true if have to make an absolute comparison else signed comparison
                pTestParameter->SetSpecificFlags(1);
                if(pTestParameter->GetValidLowLimit() && (pTestParameter->GetLowLimit()<0))
                    pTestParameter->SetLowLimit(-pTestParameter->GetLowLimit());
                if(pTestParameter->GetValidHighLimit() && (pTestParameter->GetHighLimit()<0))
                    pTestParameter->SetHighLimit(-pTestParameter->GetHighLimit());

            }
            //Bits49
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            //Bits50
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        }
    }

    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the FET_TEST .CPR file
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ReadCprFile(const char *CprFileName)
{
    QString       strString;
    StoredBuffer  clBinaryObject;

    // Debug message
    strString = "---- CGFET_TESTtoSTDF::ReadCprFile(";
    strString += CprFileName;
    strString += ")";
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());

    // Open FET_TEST CPR file if exist
    strString = m_strDataFilePath + "/";
    strString += CprFileName;
    QFile f(strString.toLatin1().constData() );
    // file doesn't exist
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Check if .CPR is not in <fettest> sub-folder....
        strString = m_strDataFilePath + "/fettest/";
        strString += CprFileName;
        f.setFileName(strString);

        if(!f.open(QIODevice::ReadOnly))
        {
            // .TES file not found....!
            mLastError = errInvalidFormatParameter;

            // Convertion failed.
            return false;
        }
    }

    int i,iIndex;
    int iBlockSize = f.read(m_szBlock, FET_TEST_BLOCK_SIZE);
    if(iBlockSize != FET_TEST_BLOCK_SIZE)
    {
        // Incorrect header...this is not a FET_TEST CPR file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    BYTE	bByte;
    short	sShort;
    char	szString[256];

    iIndex = 0;
    //"P" (Class Probe File)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    if(bByte != 'P')
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    //"4" 1 Char ASCII (9400)
    iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
    if(bByte != '4')
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }


    iIndex = 245;
    // goto Run file name at creation (14 char var ascii) Var Ascii
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 15;
    // test if the good associated CPR and DAT file
    if(!m_strRunFile.isEmpty()
            && (m_strRunFile != strString))
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // goto Test file name at creation (14 char var ascii) Var Ascii
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 15;
    if(!m_strTestFile.isEmpty()
            && (m_strTestFile != strString))
    {
        // Incorrect header...this is not a FET_TEST file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    iIndex = 2;
    //Wafer # File Size (1-100) NUMWAF Fixed Binary
    iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex,&sShort);
    //Rem 39 Variable ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 40;
    //Date 39 Variable ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    if(m_lStartTime < 0)
    {
        m_strDataFileDate = szString;

        QRegExp clRegExp("[-/=\\s]");
        QString	strMonth = m_strDataFileDate.section(clRegExp,0,0).simplified();
        QString	strDay = m_strDataFileDate.section(clRegExp,1,1).simplified();
        QString	strYear = m_strDataFileDate.section(clRegExp,2,2).simplified();

        if(!strMonth.isEmpty() && !strDay.isEmpty() && !strYear.isEmpty())
        {
            if(strYear.toInt() < 100)
                strYear = QString::number(2000+strYear.toInt());
            if(strMonth.toInt() == 0)
            {
                // try to find the good month
                for(i=1; i>=12; i++)
                {
                    if(strMonth.startsWith(QDate::shortMonthName(i), Qt::CaseInsensitive))
                    {
                        strMonth = QString::number(i);
                        break;
                    }
                }
            }

            QDate clDate(strYear.toInt(), strMonth.toInt(), strDay.toInt());
            QTime clTime;
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
    }

    iIndex += 40;
    //Header Line # 15 Char Var CLINE Var ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    strString = szString;
    iIndex += 16;
    //Header Operator 39 Char Var COPER Var ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strOperator = szString;
    iIndex += 40;
    //Header Lot # 15 Char Var CLOT Var ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strSubLotId = m_strLotId = szString;
    iIndex += 16;
    //Header Prober 3 Char Var CPROB Var ASCII
    clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
    m_strProber = szString;
    iIndex += 4;

    iIndex=1036;
    // 100 * 4 Char Var Directory CWNAM Var ASCII
    for(i=1; i<=100; i++)
    {
        // Wafer Name (eg 1A,200..)
        clBinaryObject.ReadString(m_szBlock+iIndex,szString, &sShort);
        if(m_strWaferId.isEmpty())
            m_strWaferId = szString;
        iIndex += 4;
    }
    f.close();

    return true;
}

bool CGFET_TESTtoSTDF::RetrieveTestName(int aTestNumber, QString &aOutputTestName)
{
    try
    {
        int llookupTestNumber = GetLookupTestNumber(aTestNumber);

        const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStore->GetBinMapItemByTestNumber( llookupTestNumber );
        aOutputTestName = QString::fromStdString( lItem.GetTestName() );
    }
    catch( const std::exception &lException )
    {
        mLastErrorMessage = lException.what() + appendBinMappingExceptionInfo();
        return false;
    }

    return true;
}

ConverterStatus CGFET_TESTtoSTDF::IssueErrorForInexistingBinMapItem(int alookupTestNumber)
{
    // TODO: set error message, delay conversion
    mLastError = errReadBinMapFile;
    mLastErrorMessage = "Test[" + QString::number(alookupTestNumber) + "] not found in BIN mapping file " + appendBinMappingExceptionInfo();

    // Even with Examinator, return a Delay
    // If PAT-Man or Yield-Man, allow to delay conversion for 8 hours max.
    // check if file date is 8 hours or older...
    QFileInfo cFileInfo(m_strFetTestFileName);
    QDateTime cDataTime = cFileInfo.lastModified();

    // Old enough?
    if(cDataTime.secsTo(QDateTime::currentDateTime()) <= 8*3600)
        return ConvertDelay;	// File not old enough to be rejected!
    else
        // Reject convertion.
        return ConvertError;
}

bool CGFET_TESTtoSTDF::RetrieveBinInfo(int aTestNumber, int &aBinSoft, QString& aBinName ) const
{
    try
    {
        int llookupTestNumber = GetLookupTestNumber(aTestNumber);

        const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStore->GetBinMapItemByTestNumber( llookupTestNumber );
        aBinSoft = lItem.GetSoftBinNumber();
        aBinName = QString::fromStdString( lItem.GetBinName() );
    }
    catch( ... )
    {
        return false;
    }

    return true;
}

void CGFET_TESTtoSTDF::UpdateSoftBinMap(int aBinNum, const QString &aBinName, bool lPassStatus)
{
    if(!m_qMapBins_Soft.contains(aBinNum))
    {
        ParserBinning *pBin = new ParserBinning();

        pBin->SetBinName(aBinName);
        pBin->SetBinCount(0);
        pBin->SetPassFail(lPassStatus);
        m_qMapBins_Soft[aBinNum] = pBin;
    }
    m_qMapBins_Soft[aBinNum]->IncrementBinCount(1);
}

///////////////////////////////////////////////////////////
// Open file, retries until timout reached.
///////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::openFile(QFile &fFile, QIODevice::OpenModeFlag openMode,QString &strFileName, int iTimeoutSec/*=2*/)
{
    // Debug message
    QString strString;
    strString = "---- CGFET_TESTtoSTDF::openFile(";
    strString += strFileName;
    strString += ", ";
    strString += QString::number(iTimeoutSec);
    strString += ")";
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().constData());

    fFile.setFileName(strFileName);

    QTime	cTime,cWait;
    QTime	cTimeoutTime = QTime::currentTime();
    cTimeoutTime = cTimeoutTime.addSecs(iTimeoutSec);

    do
    {
        // Open file
        if(fFile.open(openMode) == true)
            return true;	// Success opening file.

        // Failed accessing to file....then wait 15msec & retry.
        cWait = cTime.addMSecs(15);
        do
        {
            cTime = QTime::currentTime();
        }
        while(cTime < cWait);
    }
    while(cTime < cTimeoutTime);

    // Timeout error: failed to open file.
    return false;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from FET_TEST data parsed
// x32gu2.dat
// Z08D0202.DAT
//////////////////////////////////////////////////////////////////////
ConverterStatus CGFET_TESTtoSTDF::WriteStdfFile(QFile *hFile,  QString& strFileNameSTDF)
{
    // Build LotID and WaferID if not already defined
    QString     lTmpString, lTestName;
    QFileInfo   lFileInfo(m_strFetTestFileName);
    bool        lBase36WaferId  = false;	// Set to true if concludes that WaferID to be xtracted from file name...
    bool        lBase36LotId    = false;	// Set to true if concludes that WaferID to be xtracted from file name...

    // Debug message
    lTmpString = "---- CGFET_TESTtoSTDF::WriteStdfFile(";
    lTmpString += strFileNameSTDF;
    lTmpString += ")";
    GSLOG(SYSLOG_SEV_DEBUG, lTmpString.toLatin1().constData());

    // FETTEST QA FT_LVM flow
    QRegExp lRegExpFtLvmFlow(REGEXP_FETTEST_QA_FT_LVM);
    if (lRegExpFtLvmFlow.exactMatch(lFileInfo.fileName()))
    {
        m_strLotId      = lRegExpFtLvmFlow.capturedTexts().at(1);
        m_strSubLotId   = m_strLotId + "." + lRegExpFtLvmFlow.capturedTexts().at(2);
    }
    else
    {
        QString     lBaseName = lFileInfo.baseName();
        bool        lOk;

        if(m_strWaferId.isEmpty())
        {
            // Take last digit of file name(eg: W45M41O.DAT -> O)
            lTmpString = lBaseName.right(1);
            // Convert from base 36.
            int lWaferNumber = lTmpString.toInt(&lOk,36);
            if(lOk)
            {
                m_strWaferId.sprintf("%02d", lWaferNumber);
                lBase36WaferId = true;
            }
        }

        if(m_strLotId.isEmpty())
        {
            // Take last digit of file name(eg: W45M41O.DAT -> 41O)
            lTmpString = lBaseName.right(3);
            // Take first two digits of sub-string (eg: 41O -> 41)
            lTmpString = lTmpString.left(2);
            // Convert from base 36.
            lTmpString.toInt(&lOk,36);
            if(lOk)
                lBase36LotId = true;
        }

        if (lBase36WaferId && lBase36LotId)
        {
            m_strLotId = lBaseName;
            m_strSubLotId = lBaseName;
        }
    }

    // Defaults Product ID to .RUN file name (in case info not fund in .CSV file)
    ParserPromisFile lPromisFile;
    lPromisFile.SetProductId(m_strRunFile.section(".",0,0));
    lPromisFile.SetSublotId(m_strSubLotId);

    // Clear variables
    m_bPromisFtFile = false;

    if(ConverterExternalFile::Exists(m_strDataFilePath))
    {
        // Check if converter_external_file exists
        QString strExternalFileType;
        QString strExternalFileName;
        QString strExternalFileFormat;
        QString strExternalFileError;
        mExternalFilePath = ConverterExternalFile::GetExternalFileName( m_strDataFilePath );
        if(m_bWaferMap)
        {

            strExternalFileType = "wafer";
            if(ConverterExternalFile::GetPromisFile(m_strDataFilePath,strExternalFileType,"prod",strExternalFileName,strExternalFileFormat,strExternalFileError))
            {

                // Debug message
                lTmpString.sprintf("---- CGFET_TESTtoSTDF::ReadCustomDataFile(): from ExternalFile XML = %s",strExternalFileName.toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, lTmpString.toLatin1().constData());
            }

            mPromisFilePath = strExternalFileName;
            if(!strExternalFileName.isEmpty())
            {
                QString lCryptedLot = m_strLotId;
                int     lWaferNumber;

                if(ConverterExternalFile::DecryptLot(lCryptedLot, m_strLotId, lWaferNumber, mLastErrorMessage) == false)
                {
                    return ConvertError;	// Reject convertion.
                }

                m_strWaferId.sprintf("%02d", lWaferNumber);
                QString lPromisKey = m_strLotId + "." + QString::number(lWaferNumber);

                // Check if custom text file to read for extracting additional data (eg: Vishay PROMIS_DATA_PATH)
                if(!lPromisFile.ReadPromisDataFile(strExternalFileName,
                                                   strExternalFileType,
                                                   strExternalFileFormat,
                                                   lPromisKey,
                                                   m_strDataFilePath))
                {
                    mLastErrorMessage = lPromisFile.GetLastErrorMessage();
                    // Nothing found in promis file, check if Engineering Promis file exists...
                    if(ConverterExternalFile::GetPromisFile(m_strDataFilePath,"wafer","eng",strExternalFileName,strExternalFileFormat,strExternalFileError))
                    {
                        // Debug message
                        lTmpString.sprintf("---- CGFET_TESTtoSTDF::ReadCustomDataFile(): from ExternalFile XML = %s",strExternalFileName.toLatin1().constData());
                        GSLOG(SYSLOG_SEV_DEBUG, lTmpString.toLatin1().constData());
                    }

                    if(!lPromisFile.ReadPromisDataFile(strExternalFileName,
                                                       strExternalFileType,
                                                       strExternalFileFormat,
                                                       lPromisKey,
                                                       ConverterExternalFile::GetExternalFileName( m_strDataFilePath )))
                    {
                        // Custom PROMIS file specified, but missing entry in it.
                        mLastErrorMessage = lPromisFile.GetLastErrorMessage();
                        mLastError = errReadPromisFile;

                        // Even with Examinator, return a Delay
                        // If PAT-Man or Yield-Man, allow to delay conversion for 8 hours max. as PROMIS file may not be updated yet.
                        // check if file date is 8 hours or older...
                        QDateTime cDataTime = lFileInfo.lastModified();

                        // Old enough?
                        if(cDataTime.secsTo(QDateTime::currentDateTime()) <= 8*3600)
                        {
                            return ConvertDelay;	// File not old enough to be rejected!
                        }
                        else
                        {
                            // Reject convertion.
                            return ConvertError;
                        }
                    }
                }
            }
        }
        else
        {
            strExternalFileType = "final";
            if(ConverterExternalFile::GetPromisFile(m_strDataFilePath,strExternalFileType,"prod",strExternalFileName,strExternalFileFormat,strExternalFileError))
            {
                // Debug message
                lTmpString.sprintf("---- CGFET_TESTtoSTDF::ReadCustomDataFile(): from ExternalFile XML = %s",strExternalFileName.toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, lTmpString.toLatin1().constData());
            }
            mPromisFilePath = strExternalFileName;

            if(!strExternalFileName.isEmpty())
            {
                // Check if entry can be found in FT Promis file
                if(!lPromisFile.ReadPromisDataFile(strExternalFileName,
                                                   "final",
                                                   strExternalFileFormat,
                                                   m_strSubLotId,
                                                   mExternalFilePath))
                {
                    if(lPromisFile.GetLastErrorCode() == ParserPromisFile::errPrFieldNb)
                    {
                    mLastErrorMessage = strExternalFileName
                            + " specified in converter external file "
                            + mExternalFilePath
                            + ".\nMissing columns. At least 8 columns are expected.";
                    }
                    else
                    {
                        mLastErrorMessage = lPromisFile.GetLastErrorMessage();
                    }
                    // Custom PROMIS file specified, but missing entry in it.
                    mLastError = errReadPromisFile;

                    // Even with Examinator, return a Delay
                    // If PAT-Man or Yield-Man, allow to delay conversion for 8 hours max. as PROMIS file may not be updated yet.
                    // check if file date is 8 hours or older...
                    QDateTime cDataTime = lFileInfo.lastModified();

                    // Old enough?
                    if(cDataTime.secsTo(QDateTime::currentDateTime()) <= 8*3600)
                        return ConvertDelay;	// File not old enough to be rejected!
                    else
                        // Reject convertion.
                        return ConvertError;
                }

                // Final-Test .dat file.
                lBase36WaferId = lBase36LotId = false;
                m_bPromisFtFile = true;
            }
        }

        // Check if Test->Binning mapping file to overload softbin
        if(ConverterExternalFile::GetBinmapFile(
                    m_strDataFilePath,strExternalFileType,"prod",
                    strExternalFileName,strExternalFileFormat,strExternalFileError))
        {
            mBinMapFilePath = m_strDataFilePath + strExternalFileName;
            // Debug message
            lTmpString.sprintf("---- CGFET_TESTtoSTDF::ReadCustomDataFile(): from ExternalFile XML = %s",strExternalFileName.toLatin1().constData());
            GSLOG(SYSLOG_SEV_DEBUG, lTmpString.toLatin1().constData());
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, strExternalFileError.toLatin1().constData());
            mLastError = errConverterExternalFile;
            mLastErrorMessage = strExternalFileError;
            return ConvertError;
        }


        if(!strExternalFileName.isEmpty())
        {
            if(!ReadBinMapFile(strExternalFileName,strExternalFileType,strExternalFileFormat))
            {
                // Bin mapping file specified, but error loading it.
                mLastError = errReadBinMapFile;

                // Even with Examinator, return a Delay
                // If PAT-Man or Yield-Man, allow to delay conversion for 8 hours max.
                // check if file date is 8 hours or older...
                QFileInfo cFileInfo(m_strFetTestFileName);
                QDateTime cDataTime = cFileInfo.lastModified();

                // Old enough?
                if(cDataTime.secsTo(QDateTime::currentDateTime()) <= 8*3600)
                    return ConvertDelay;	// File not old enough to be rejected!
                else
                    // Reject convertion.
                    return ConvertError;
            }
        }
    }

    // Overload STDF file name if this file had Base 36 name coding, or if FT Promis file...
    if(lBase36WaferId && lBase36LotId)
    {
        // Overload STDF file name.
        lFileInfo.setFile(strFileNameSTDF);
        lTmpString = lFileInfo.absolutePath() + "/"
                    + lPromisFile.GetSublotId() + "-"
                    + m_strWaferId + "."
                    + lFileInfo.completeSuffix();
        strFileNameSTDF = lTmpString;
    }
    else if(m_bPromisFtFile)
    {
        // Overload STDF file name.
        lFileInfo.setFile(strFileNameSTDF);

        lTmpString = lFileInfo.absolutePath() + "/" + lPromisFile.GetSublotId() + ".";

        if (lRegExpFtLvmFlow.exactMatch(lFileInfo.fileName()))
            lTmpString += lRegExpFtLvmFlow.capturedTexts().at(3);
        else
            lTmpString += lFileInfo.completeSuffix();

        strFileNameSTDF = lTmpString;
    }

    // Debug message
    lTmpString = "---- CGFET_TESTtoSTDF::WriteStdfFile(): generating STDF file ";
    lTmpString += strFileNameSTDF;
    GSLOG(SYSLOG_SEV_DEBUG, lTmpString.toLatin1().constData());

    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open(strFileNameSTDF.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing FET_TEST file into STDF database
        mLastError = errWriteSTDF;

        // Delay convertion.
        return ConvertDelay;
    }

    // Write FAR
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);					// SUN CPU type
    StdfFile.WriteByte(4);					// STDF V4
    StdfFile.WriteRecord();

    // Extract .RUN file name (truncate extension)
    QString strProgramName = m_strRunFile.section(".",0,0);
    QString lTesterType = (lPromisFile.GetTesterType().isEmpty()) ? QStringLiteral("FetTest") : lPromisFile.GetTesterType();

    // Write ATR
    lTmpString = "Quantix (FetTest converter)";
    RecordReadInfo.iRecordType = 0;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(QDateTime::currentDateTime().toTime_t());       // MOD_TIM
    StdfFile.WriteString(lTmpString.toLatin1().constData());				// CMD_LINE
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
    StdfFile.WriteString(m_strLotId.toLatin1().constData());                        // Lot ID
    StdfFile.WriteString(lPromisFile.GetProductId().toLatin1().constData());		// Part Type / Product ID
    StdfFile.WriteString(lPromisFile.GetEquipmentID().toLatin1().constData());		// Node name
    StdfFile.WriteString(lTesterType.toLatin1().constData());            // Tester Type
    StdfFile.WriteString(strProgramName.toLatin1().constData());			// Job name
    StdfFile.WriteString("");												// Job rev
    StdfFile.WriteString(lPromisFile.GetSublotId().toLatin1().constData());		// sublot-id
    StdfFile.WriteString(m_strOperator.toLatin1().constData());		// operator
    StdfFile.WriteString("");									// exec-type
    StdfFile.WriteString("");									// exe-ver
    StdfFile.WriteString(lPromisFile.GetTestCode().toLatin1().constData());	// test-cod
    StdfFile.WriteString("");									// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":FET_TEST";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());				// user-txt
    StdfFile.WriteString(m_strTestFile.toLatin1().constData());			// aux-file
    StdfFile.WriteString(lPromisFile.GetPackageType().toLatin1().constData());		// package-type
    StdfFile.WriteString("");												// familyID
    StdfFile.WriteString(lPromisFile.GetDateCode().toLatin1().constData());			// Date-code
    StdfFile.WriteString(lPromisFile.GetFacilityID().toLatin1().constData());			// Facility-ID
    StdfFile.WriteString("");												// FloorID
    StdfFile.WriteString(lPromisFile.GetProcID().toLatin1().constData());				// ProcessID
    StdfFile.WriteString("");												// OPER_FRQ
    StdfFile.WriteString("");												// SPEC_NAM
    StdfFile.WriteString("");												// SPEC_VER
    StdfFile.WriteString("");												// FLOW_ID
    StdfFile.WriteString("");												// SETUP_ID
    StdfFile.WriteString("");												// DSGN_REV
    StdfFile.WriteString("");												// ENG_ID
    StdfFile.WriteString(lPromisFile.GetRomCode().toLatin1().constData());	// ROM_COD
    StdfFile.WriteRecord();

    // Write DTR with Gross Die
    if(lPromisFile.GetGrossDieCount() > 0)
    {
        strUserTxt = "<cmd> gross_die=" + QString::number(lPromisFile.GetGrossDieCount());
        RecordReadInfo.iRecordType = 50;
        RecordReadInfo.iRecordSubType = 30;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteString(strUserTxt.toLatin1().constData());				// ASCII text string
        StdfFile.WriteRecord();
    }

    // Write die-tracking DTRs
    if(!lPromisFile.GetPackageType().isEmpty())
    {
        strUserTxt = "<cmd> die-tracking die=1;wafer_product=" + lPromisFile.GetProductId();
        strUserTxt += ";wafer_lot=" + m_strLotId;
        strUserTxt += ";wafer_sublot=" + lPromisFile.GetSublotId();
        RecordReadInfo.iRecordType = 50;
        RecordReadInfo.iRecordSubType = 30;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteString(strUserTxt.toLatin1().constData());
        StdfFile.WriteRecord();
        if(!lPromisFile.GetPromisLotId_D2().isEmpty() && !lPromisFile.GetGeometryName_D2().isEmpty())
        {
            lTmpString = lPromisFile.GetPromisLotId_D2().section('.', 0, 0);
            strUserTxt = "<cmd> die-tracking die=2;wafer_product=" + lPromisFile.GetGeometryName_D2();
            strUserTxt += ";wafer_lot=" + lTmpString;
            strUserTxt += ";wafer_sublot=" + lPromisFile.GetPromisLotId_D2();
            RecordReadInfo.iRecordType = 50;
            RecordReadInfo.iRecordSubType = 30;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteString(strUserTxt.toLatin1().constData());
            StdfFile.WriteRecord();
            if(!lPromisFile.GetPromisLotId_D3().isEmpty() && !lPromisFile.GetGeometryName_D3().isEmpty())
            {
                lTmpString = lPromisFile.GetPromisLotId_D3().section('.', 0, 0);
                strUserTxt = "<cmd> die-tracking die=3;wafer_product=" + lPromisFile.GetGeometryName_D3();
                strUserTxt += ";wafer_lot=" + lTmpString;
                strUserTxt += ";wafer_sublot=" + lPromisFile.GetPromisLotId_D3();
                RecordReadInfo.iRecordType = 50;
                RecordReadInfo.iRecordSubType = 30;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteString(strUserTxt.toLatin1().constData());
                StdfFile.WriteRecord();
                if(!lPromisFile.GetPromisLotId_D4().isEmpty() && !lPromisFile.GetGeometryName_D4().isEmpty())
                {
                    lTmpString = lPromisFile.GetPromisLotId_D4().section('.', 0, 0);
                    strUserTxt = "<cmd> die-tracking die=4;wafer_product=" + lPromisFile.GetGeometryName_D4();
                    strUserTxt += ";wafer_lot=" + lTmpString;
                    strUserTxt += ";wafer_sublot=" + lPromisFile.GetPromisLotId_D4();
                    RecordReadInfo.iRecordType = 50;
                    RecordReadInfo.iRecordSubType = 30;
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteString(strUserTxt.toLatin1().constData());
                    StdfFile.WriteRecord();
                }
            }
        }
    }

    if(!m_strProber.isEmpty() || !lPromisFile.GetExtraID().isEmpty())
    {
        // Write SDR
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 80;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte((BYTE)1);			// head#
        StdfFile.WriteByte((BYTE)1);			// Group#
        StdfFile.WriteByte((BYTE)1);			// site_count
        StdfFile.WriteByte((BYTE)1);			// array of test site# (dummy!)
        StdfFile.WriteString(m_strProber.toLatin1().constData());		// HAND_TYP: Handler/prober type
        StdfFile.WriteString(m_strProber.toLatin1().constData());		// HAND_ID: Handler/prober name
        StdfFile.WriteString("");								// CARD_TYP: Probe card type
        StdfFile.WriteString("");								// CARD_ID: Probe card name
        StdfFile.WriteString("");								// LOAD_TYP: Load board type
        StdfFile.WriteString("");								// LOAD_ID: Load board name
        StdfFile.WriteString("");								// DIB_TYP: DIB board type
        StdfFile.WriteString("");								// DIB_ID: DIB board name
        StdfFile.WriteString("");								// CABL_TYP: Interface cable type
        StdfFile.WriteString("");								// CABL_ID: Interface cable name
        StdfFile.WriteString("");								// CONT_TYP: Handler contactor type
        StdfFile.WriteString("");								// CONT_ID: Handler contactor name
        StdfFile.WriteString("");								// LASR_TYP: Laser type
        StdfFile.WriteString("");								// LASR_ID: Laser name
        StdfFile.WriteString("");								// EXTR_TYP: Extra equipment type
        StdfFile.WriteString(lPromisFile.GetExtraID().toLatin1().constData());	// EXTR_ID: Extra equipment name
        StdfFile.WriteRecord();
    }

    if(m_bWaferMap)
    {
        // Write WCR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 30;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteFloat(0.0);			// Wafer diameter: unknown
        StdfFile.WriteFloat(lPromisFile.GetDieH());		// Die Heigth
        StdfFile.WriteFloat(lPromisFile.GetDieW());		// Die Width
        StdfFile.WriteByte((BYTE)3);		// Units: mm
        StdfFile.WriteByte((BYTE)lPromisFile.GetFlat());	// Flat orientation: U, D, R or L
        StdfFile.WriteRecord();


        // Write WIR of new Wafer.
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);							// Test head
        StdfFile.WriteByte(255);						// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);				// Start time
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }

    // Write Test results for each line read.
    int     iTotalGoodBin=0;
    int     iTotalFailBin=0;
    int     iTotalTests=0;
    int     iSerialNumber=0;
    int     iPartNumber=0;
    bool    bPartIsPass=true, bAllTestsPass=true;
    BYTE    bData;


    StoredBuffer clBinaryObject;
    int     lBufferPos=0;
    int     iTest=0;
    int     iSeg=0;
    int     iIndex=0;
    int     iBin=0, iBin_Soft=0;
    int     iTestNumber=0;
    bool    bTestPass;
    int     iSNNUM = m_iSNNUM;
    int     iSNSIZE=0;
    int     iBlockSize=0;
    BYTE    bByte=0;
    short   sShort=0;
    gsint32    lLong=0;
    float   fFloat=0;
    int     iAllFlags=0;
    int     iFlag=0;
    int     iXWafer=0;
    int     iYWafer=0;
    bool    bIsNAN=false;
    ParserParameter *pTestParameter;

    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=iSerialNumber=0;

    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR
    // Read FET_TEST Data Record
    iIndex = 0;
    iBlockSize = 0;
    while(!hFile->atEnd() || (lBufferPos < FET_TEST_BLOCK_SIZE))
    {
        // Read next record
        if(iBlockSize == 0)
        {
            iBlockSize = ReadBlock(hFile,m_szBlock, FET_TEST_BLOCK_SIZE);
            iIndex = 0;
            iSNNUM = 0;
            if(iBlockSize != FET_TEST_BLOCK_SIZE)
            {
                if(hFile->atEnd())
                    break;

                // Incorrect header...this is not a FET_TEST file!
                mLastError = errInvalidFormatParameter;

                // Convertion failed.
                return ConvertError;
            }

            // New bug : CONCATENED FILES
            // Check if the Block start with a new FetTest header
            lBufferPos = 0;
            //"D" 1 Char ASCII (Data File)
            lBufferPos += clBinaryObject.ReadByte(m_szBlock+lBufferPos, &bByte);
            if(bByte == 'D')
            {	//"4" 1 Char ASCII (9400)
                lBufferPos += clBinaryObject.ReadByte(m_szBlock+lBufferPos, &bByte);
                if(bByte == '4')
                {	//Start SN (1-32,766)
                    lBufferPos += clBinaryObject.ReadWord(m_szBlock+lBufferPos, &sShort);
                    if(m_iSSN == (int) sShort)
                    {	//End SN (1-32,766)
                        lBufferPos += clBinaryObject.ReadWord(m_szBlock+lBufferPos, &sShort);
                        if(m_iESN == (int) sShort)
                        {	//Number of SN's per 1536 Byte Record
                            lBufferPos += clBinaryObject.ReadByte(m_szBlock+lBufferPos, &bByte);
                            if((int) bByte <= 192)
                            {	//SN Size (8-1283) Bytes
                                lBufferPos += clBinaryObject.ReadWord(m_szBlock+lBufferPos, &sShort);
                                if((int) sShort <= 1283)
                                {
                                    // Multi headers found
                                    mLastError = errInvalidFormatParameter;
                                    mLastErrorMessage = "Multi headers found - Concatened files?";

                                    // Convertion failed.
                                    return ConvertError;
                                }
                            }
                        }
                    }
                }
            }
        }
        bByte = 0;
        for(lBufferPos=iIndex; lBufferPos<FET_TEST_BLOCK_SIZE; lBufferPos++)
        {
            if(m_szBlock[lBufferPos] != 0)
            {
                bByte = 1;
                break;
            }
            if(iSNSIZE == m_iSNSIZE)
            {
                iSNNUM++;
                iSNSIZE = 0;
            }
            iSNSIZE++;
        }
        if(bByte == 0)
        {
            iBlockSize = 0;
            continue;
        }

        iIndex = iSNNUM * m_iSNSIZE;
        iSNNUM++;
        // Read Part Result
        iTotalTests = 0;
        iPartNumber++;
        // Serial Number or XY

        if(m_bWaferMap)
        {
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            iXWafer = (int) bByte;
            iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
            iYWafer = (int) bByte;
            iSerialNumber = iPartNumber;
        }
        else if( mIsNewFormat )
        {
            iIndex += clBinaryObject.ReadDword( m_szBlock + iIndex, &iSerialNumber );
        }
        else
        {
            iIndex += clBinaryObject.ReadWord(m_szBlock+iIndex, &sShort);
            iSerialNumber = (int) sShort;
            iXWafer = iYWafer = -32768;
        }
        // Bin result
        iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
        iBin = (int) bByte & ~BIT7;
        iBin_Soft = iBin;
        bPartIsPass = !(bByte & BIT7);

        if(!m_qMapBins.contains(iBin))
        {
            ParserBinning *pBin = new ParserBinning();

            pBin->SetBinCount(0);
            m_qMapBins[iBin] = pBin;
            if(!bPartIsPass)
                m_qMapBins[iBin]->SetPassFail(false);
            else
                m_qMapBins[iBin]->SetPassFail(true);

        }

        m_qMapBins[iBin]->IncrementBinCount(1);

        // Write PIR
        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);					// Test head
        StdfFile.WriteByte(1);					// Tester site
        StdfFile.WriteRecord();

        // Reset failed test and test not found variables
        m_nFailedTest = -1;
        bAllTestsPass = true;

        // Size m_iSNSIZE
        for(iTest=0 ; iTest < m_iTestNumber; iTest++)
        {
            QVector<float>	fTabValues(m_iSegNumber);
            // GCORE-1780 - Missing Test after analyzing .DAT data type within Examinator
            // For MPR with at least one invalid pin result and at least one valid pin result
            QVector<float>	fTabFlags(m_iSegNumber);
            bIsNAN=false;
            iFlag = 0;
            iAllFlags = 0;
            bTestPass = true;

            for(iSeg=0; iSeg<m_iSegNumber; iSeg++)
            {
                iIndex += clBinaryObject.ReadByte(m_szBlock+iIndex, &bByte);
                iFlag = (int) bByte;
                iAllFlags += iFlag;
                bTestPass &= !(iFlag & BIT3);
                if(iFlag & BIT7)
                {
                    iIndex += clBinaryObject.ReadFloat(m_szBlock+iIndex, &fFloat, &bIsNAN);
                    bIsNAN = !(!bIsNAN && (-1e38 < fFloat) && (fFloat< 1e38));
                }
                else
                {
                    iIndex += clBinaryObject.ReadDword(m_szBlock+iIndex, &lLong);
                    fFloat = (float) lLong;
                }
                fTabValues[iSeg] = fFloat;
                fTabFlags[iSeg] = iFlag;
            }
            if(iAllFlags == 0)	// no data
                continue;

            iTotalTests++;
            bAllTestsPass &= bTestPass;
            iTestNumber = m_iTestN[iTest];

            pTestParameter = m_qMapParameterList[iTestNumber];

            // Retrieve test name
            if(!GetFetTestName(iTestNumber, lTestName))
                return IssueErrorForInexistingBinMapItem(iTestNumber);

            if(m_iSegNumber == 1)
            {
                // Write PTR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(iTestNumber);           // Test Number
                StdfFile.WriteByte(1);						// Test head
                StdfFile.WriteByte(1);						// Tester site:1,2,3,4 or 5, etc.
                bData = 0;
                if(bIsNAN)
                    bData |= BIT1;
                if(!bTestPass)
                {
                    if(m_specificFlowType == FET_FLOW_LVM_FT)
                    {
                       //the following responds to the Vishay mapping format algorithm that is described in schema "Vishay_STDFtoSTDF_BinMapping_Algorithm.png"
                       if(iTestNumber == 0 || lTestName.size() == 0)
                       {
                           continue;
                       }
                       else
                       {
                           try
                           {
                               int llookupTestNumber = GetLookupTestNumber(iTestNumber);
                               const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStore->GetBinMapItemByTestNumber(llookupTestNumber);
                               if(lItem.GetEnabled())
                               {
                                   bData |= BIT7;      // Test Failed
                                   m_nFailedTest = iTestNumber;
                               }
                           }
                           catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
                           {
                               bData |= BIT7;  // Test Failed
                               m_nFailedTest = iTestNumber;
                           }
                           catch (...)
                           {
                               mLastError = errParserSpecific;
                               mLastErrorMessage = "Wrong bin mapping system used.";
                               return ConvertError;
                           }
                       }
                    }
                    else
                    {
                        bData |= BIT7; // Test Failed
                        m_nFailedTest = iTestNumber;
                    }
                }
                StdfFile.WriteByte(bData);					// TEST_FLG
                bData = BIT6|BIT7;
                StdfFile.WriteByte(bData);					// PARAM_FLG
                // Verify if have AbsoluteComparison Flag
                // then save an absolute value
                // true if have to make an absolute comparison else signed comparison
                if((pTestParameter->GetSpecificFlags()==1) && (fTabValues[0]<0))
                    StdfFile.WriteFloat(-fTabValues[0]);		// Test result
                else
                    StdfFile.WriteFloat(fTabValues[0]);			// Test result
                if(!pTestParameter->GetStaticHeaderWritten())
                {
                    StdfFile.WriteString(lTestName.toLatin1().constData());	// TEST_TXT
                    StdfFile.WriteString("");				// ALARM_ID
                    bData = 0;
                    // No Scale result
                    if(pTestParameter->GetResultScale() == 0)
                        bData |= BIT0;
                    // Bit1 = 1
                    bData |= BIT1;
                    // No LowLimit
                    if(!pTestParameter->GetValidLowLimit())
                        bData |= BIT6;
                    // No HighLimit
                    if(!pTestParameter->GetValidHighLimit())
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);							// OPT_FLAG
                    StdfFile.WriteByte(-pTestParameter->GetResultScale());		// RES_SCALE
                    StdfFile.WriteByte(-pTestParameter->GetResultScale());		// LLM_SCALE
                    StdfFile.WriteByte(-pTestParameter->GetResultScale());		// HLM_SCALE
                    StdfFile.WriteFloat(pTestParameter->GetLowLimit());	// LOW Limit
                    StdfFile.WriteFloat(pTestParameter->GetHighLimit());	// HIGH Limit
                    StdfFile.WriteString(pTestParameter->GetTestUnits().toLatin1().constData());	// Units
                }
                StdfFile.WriteRecord();
            }
            else
            {
                // Write MPR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 15;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(iTestNumber);           // Test Number
                StdfFile.WriteByte(1);						// Test head
                StdfFile.WriteByte(1);						// Tester site:1,2,3,4 or 5, etc.
                bData = 0;
                if(bIsNAN)
                    bData |= BIT1;
                if(!bTestPass)
                {
                    if(m_specificFlowType == FET_FLOW_LVM_FT)
                    {
                       //the following responds to the Vishay mapping format algorithm that is described in schema "Vishay_STDFtoSTDF_BinMapping_Algorithm.png"
                       if(iTestNumber == 0 || lTestName.size() == 0)
                       {
                           continue;
                       }
                       else
                       {
                           try
                           {
                               int llookupTestNumber = GetLookupTestNumber(iTestNumber);
                               const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStore->GetBinMapItemByTestNumber(llookupTestNumber);
                               if(lItem.GetEnabled())
                               {
                                   bData |= BIT7;      // Test Failed
                                   m_nFailedTest = iTestNumber;
                               }
                           }
                           catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
                           {
                               bData |= BIT7;  // Test Failed
                               m_nFailedTest = iTestNumber;
                           }
                           catch (...)
                           {
                               mLastError = errParserSpecific;
                               mLastErrorMessage = "Wrong bin mapping system used.";
                               return ConvertError;
                           }
                       }
                    }
                    else
                    {
                        bData |= BIT7; // Test Failed
                        m_nFailedTest = iTestNumber;
                    }
                }
                StdfFile.WriteByte(bData);					// TEST_FLG
                bData = BIT6|BIT7;
                StdfFile.WriteByte(bData);					// PARAM_FLG
                StdfFile.WriteWord(m_iSegNumber);			// RTN_ICNT
                StdfFile.WriteWord(m_iSegNumber);			// RSLT_CNT

                for(iSeg=0; iSeg!=(m_iSegNumber+1)/2; iSeg++)
                    StdfFile.WriteByte(0);						// RTN_STAT
                for(iSeg=0; iSeg<m_iSegNumber; iSeg++)
                {
                    if(fTabFlags[iSeg] == 0)
                        StdfFile.WriteFloat(std::numeric_limits<float>::quiet_NaN());
                    else
                    {
                        // true if have to make an absolute comparison else signed comparison
                        if((pTestParameter->GetSpecificFlags()==1) && (fTabValues[iSeg]<0))
                            StdfFile.WriteFloat(-fTabValues[iSeg]);	// Test result
                        else
                            StdfFile.WriteFloat(fTabValues[iSeg]);	// Test result
                    }
                }

                StdfFile.WriteString(lTestName.toLatin1().constData());	// TEST_TXT
                StdfFile.WriteString("");				// ALARM_ID

                if(!pTestParameter->GetStaticHeaderWritten())
                {
                    // No Scale result
                    bData = 0;
                    if(pTestParameter->GetResultScale() == 0)
                        bData |= BIT0;
                    // Bit1 = 1
                    bData |= BIT1;
                    // No LowLimit
                    if(!pTestParameter->GetValidLowLimit())
                        bData |= BIT6;
                    // No HighLimit
                    if(!pTestParameter->GetValidHighLimit())
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);							// OPT_FLAG
                    StdfFile.WriteByte(-pTestParameter->GetResultScale());		// RES_SCALE
                    StdfFile.WriteByte(-pTestParameter->GetResultScale());		// LLM_SCALE
                    StdfFile.WriteByte(-pTestParameter->GetResultScale());		// HLM_SCALE
                    StdfFile.WriteFloat(pTestParameter->GetLowLimit());	// LOW Limit
                    StdfFile.WriteFloat(pTestParameter->GetHighLimit());	// HIGH Limit
                    StdfFile.WriteFloat(0);								// StartIn
                    StdfFile.WriteFloat(0);								// IncrIn

                    for(iSeg=0; iSeg<m_iSegNumber; iSeg++)
                        StdfFile.WriteWord(iSeg);						// rtn_indx

                    StdfFile.WriteString(pTestParameter->GetTestUnits().toLatin1().constData());	// Units
                }
                StdfFile.WriteRecord();
            }
            pTestParameter->SetStaticHeaderWritten(true);
        }

        // Check if Binning should be mapped
                if(m_specificFlowType == FET_FLOW_LVM_FT)
        {
           if(!bPartIsPass)
           {
               if(bAllTestsPass)
               {
                   //part is Fail but no test failed -> try to map using default values (not tested for test data is missing for this use case)
                   if(mDefaultBinSet)
                   {
                       iBin_Soft = mDefaultBinNumber;
                       UpdateSoftBinMap(mDefaultBinNumber, mDefaultBinName, false);
                   }
                   else
                   {
                       mLastError = errConverterExternalFile;
                       mLastErrorMessage = QStringLiteral("Failed mapping binning for part %1. The part is FAIL with all enabled tests being PASS, and there is no default binning defined in file %2.\n").arg(iPartNumber).arg(mExternalFilePath);
                       return ConvertError;
                   }
               }
               else
               {
                  if(mBinMapStore.isNull() == false)
                  {
                      if(m_nFailedTest != -1)
                      {
                          QString lBinName;

                          if (RetrieveBinInfo(m_nFailedTest, iBin_Soft, lBinName))
                              UpdateSoftBinMap(iBin_Soft, lBinName, false);
                          else
                              return IssueErrorForInexistingBinMapItem(m_nFailedTest);
                      }
                      else
                      {
                         // Update Soft bin map for the FAIL bin (with no test failed)
                                                   if(mDefaultBinSet)
                         {
                             iBin_Soft = mDefaultBinNumber;
                             UpdateSoftBinMap(mDefaultBinNumber, mDefaultBinName, false);
                         }
                         else
                         {
                             mLastError = errConverterExternalFile;
                             mLastErrorMessage = QStringLiteral("Failed mapping binning for part %1. The part is FAIL with all enabled tests being PASS, and there is no default binning defined in file %2.\n").arg(iPartNumber).arg(mExternalFilePath);
                             return ConvertError;
                         }
                      }
                  }
                  else
                  {
                      // Update Soft bin map for the FAIL bin
                      UpdateSoftBinMap(iBin_Soft, "FAIL_BIN", false);
                  }
               }
           }
           else
           {
               // Update Soft bin map for this PASS bin
               UpdateSoftBinMap(iBin_Soft, cPassBin, true);
           }
           iBin = iBin_Soft;
        }

        else//not FET_FLOW_LVM_FT
        {
           if(!bPartIsPass && !bAllTestsPass)
           {
              if(mBinMapStore.isNull() == false)
               {
                   if(m_nFailedTest != -1)
                   {
                       QString lBinName;
                       if (RetrieveBinInfo(m_nFailedTest, iBin_Soft, lBinName))
                           UpdateSoftBinMap(iBin_Soft, lBinName, false);
                       else
                           return IssueErrorForInexistingBinMapItem(m_nFailedTest);
                   }
                   else
                   {
                       // Update Soft bin map for the FAIL bin (with no test failed)
                       UpdateSoftBinMap(iBin_Soft, "FAIL_BIN (no test failed)", false);
                   }
               }
               else
               {
                   // Update Soft bin map for the FAIL bin
                   UpdateSoftBinMap(iBin_Soft, "FAIL_BIN", false);
               }
           }
           else
           {
               // Update Soft bin map for this PASS bin
               UpdateSoftBinMap(iBin_Soft, cPassBin, true);
           }
        }

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);				// Test head
        StdfFile.WriteByte(1);				// Tester site:1,2,3,4 or 5
        if(bPartIsPass)
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
        StdfFile.WriteWord(iBin_Soft);			// SOFT_BIN
        StdfFile.WriteWord(iXWafer);			// X_COORD
        StdfFile.WriteWord(iYWafer);			// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(QString::number(iSerialNumber).toLatin1().constData());// PART_ID
        StdfFile.WriteString("");				// PART_TXT
        StdfFile.WriteString("");				// PART_FIX
        StdfFile.WriteRecord();
    }


    // Check if have some results : if not, reject empty file
    if(iPartNumber == 0)
    {
        // Error: do not genetarte an empty STDF file
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = "No test result found in the file";
        return ConvertError;
    }

    if(m_bWaferMap)
    {
        // Write WRR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test head
        StdfFile.WriteByte(255);					// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
        StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
        StdfFile.WriteDword(0);						// Parts retested
        StdfFile.WriteDword(0);						// Parts Aborted
        StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
        StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,ParserBinning*>::Iterator itMapBin;
    //GCORE-16441: FT flows must have HBR same as SBR
    if(m_specificFlowType == FET_FLOW_LVM_FT || m_specificFlowType == FET_FLOW_HVM_FT)
    {
       for ( itMapBin = m_qMapBins_Soft.begin(); itMapBin != m_qMapBins_Soft.end(); ++itMapBin )
       {
           // Write HBR
           StdfFile.WriteHeader(&RecordReadInfo);
           StdfFile.WriteByte(255);                                            // Test Head = ALL
           StdfFile.WriteByte(255);                                            // Test sites = ALL
           StdfFile.WriteWord(itMapBin.key());                         // HBIN = 0
           StdfFile.WriteDword(itMapBin.value()->GetBinCount());       // Total Bins
           if(itMapBin.value()->GetPassFail())
           {
               StdfFile.WriteByte('P');
               StdfFile.WriteString(cPassBin); // For the bin name to "PASS"
           }
           else
           {
               StdfFile.WriteByte('F');
               StdfFile.WriteString((itMapBin.value()->GetBinName().toLatin1().constData()));
           }
           StdfFile.WriteRecord();
       }
    }
    else
    {
       for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
       {
           // Write HBR
           StdfFile.WriteHeader(&RecordReadInfo);
           StdfFile.WriteByte(255);						// Test Head = ALL
           StdfFile.WriteByte(255);						// Test sites = ALL
           StdfFile.WriteWord(itMapBin.key());				// HBIN = 0
           StdfFile.WriteDword(itMapBin.value()->GetBinCount());	// Total Bins
           if(itMapBin.value()->GetPassFail())
               StdfFile.WriteByte('P');
           else
               StdfFile.WriteByte('F');
           StdfFile.WriteString("");
           StdfFile.WriteRecord();
       }
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_qMapBins_Soft.begin(); itMapBin != m_qMapBins_Soft.end(); ++itMapBin )
    {
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(itMapBin.key());				// HBIN = 0
        StdfFile.WriteDword(itMapBin.value()->GetBinCount());	// Total Bins
        if(itMapBin.value()->GetPassFail())
        {
            StdfFile.WriteByte('P');
            StdfFile.WriteString(cPassBin); // For the bin name to "PASS"
        }
        else
        {
            StdfFile.WriteByte('F');
            StdfFile.WriteString((itMapBin.value()->GetBinName().toLatin1().constData()));
        }
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

    // Check if PROMIS conversion: if so, reject incomplete wafermaps
    if(lPromisFile.GetGrossDieCount() > 0)
    {
        // Check if at least 50% of wafer is present!
        int iTotalDies = iTotalGoodBin + iTotalFailBin;
        float fPercentage = ((float)iTotalDies * 100.0) / (float) lPromisFile.GetGrossDieCount();
        if(fPercentage < 50)
        {
            // Error: wafermap incomplete.
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = "Wafermap is incomplete";
            return ConvertError;
        }
    }

    // Success
    return ConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' FET_TEST file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGFET_TESTtoSTDF::ConvertoStdf(const QString& FetTestFileName, QString &strFileNameSTDF)
{
    // No erro (default)
    mLastError = errNoError;
    mLastErrorMessage = "";

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(FetTestFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    mConverterStatus = ReadFetTestFile(FetTestFileName, strFileNameSTDF);

    return (mConverterStatus == ConvertSuccess);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
int	 CGFET_TESTtoSTDF::ReadBlock(QFile* pFile, char *data, qint64 len)
{
    return pFile->read(data, len);
}

}
}
