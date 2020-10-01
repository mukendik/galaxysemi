//////////////////////////////////////////////////////////////////////
// import_semi_g85.cpp: Convert a SEMI_G85 file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importSkyworksIFF.h"
#include "importConstants.h"
#include "gqtl_global.h"
#include <gqtl_log.h>
#include <QFileInfo>

const QString cHeader = "<HEADER>";
const QString cHeaderEnd = "</HEADER>";
const QString cLIMITS_DEF = "<LIMITS_DEF>";
const QString cLIMITS_DEFEnd = "</LIMITS_DEF>";
const QString cEquipConfig   = "<EQUIP_CONFIG>";
const QString cEquipConfigEnd   = "</EQUIP_CONFIG>";
const QString cTestStand     = "TEST_STAND";
const QString cTransformFile = "TRANSFORM_FILE";
const QString cHandlerType   = "HANDLER_TYPE";
const QString cHandlerID     = "HANDLER_ID";
const QString cLoadBoardID   = "LOAD_BOARD_ID";
const QString cProdeCardType = "PROBE_CARD_TYPE";
const QString cProdeCardID   = "PROBE_CARD_ID";
const QString cProberType    = "PROBER_TYPE";
const QString cProberID      = "PROBER_ID";
const QString cTesterType    = "TESTER_TYPE";
const QString cTesterID      = "TESTER_ID";
const QString cWaferUnits = "WAFER_UNITS";
const QString cWaferDiameter = "WAFER_DIAMETER";
const QString cTEST_FIXTURE = "TEST_FIXTURE";
const QString cWaferMapConfig = "<WAFER_MAP_CONFIG>";
const QString cWaferMapConfigEnd = "</WAFER_MAP_CONFIG>";
const QString cMM = "mm";
const QString cCM = "cm";
const QString cIN = "in";
const QString cUM = "um";
const QString cFlatLocation = "FLAT_LOCATION";
const QString cDIEWidth   = "DIE_WIDTH";
const QString cDIEHeight  = "DIE_HEIGHT";
const QString cCenterX    = "CENTER_X";
const QString cCenterY    = "CENTER_Y";
const QString cPositiveX  = "POSITIVE_X";
const QString cPositiveY  = "POSITIVE_Y";
const QString cFLDRows    = "FLD_ROWS";
const QString cSiteConfig = "SITE_CONFIG";
const QString cFieldX     = "FIELD_X";
const QString cFieldY     = "FIELD_Y";
const QString cFLDCols    = "FLD_COLS";
const QString cLimits            = "</LIMITS>";
const QString cSBin              = "SBIN";
const QString cBinDefinitionDtat = "</BIN_DEFINITION_DATA>";
const QString cSITEEqual = "SITE=";
const QString cSITE_NOEqual = "SITE_NO=";
const QString cPCM_SITE = "PCM_SITE";
const QString cDIE_ID = "DIE_ID";
const QString cDEVID = "DEVID";
const QString cCmd       = "<cmd>";

const QString cPatReportEnd = "</PAT_REPORT>";
const QString cPatReport = "<PAT_REPORT>";
const QString cNA        = "NA";
const QString cNALower   = "na";
const QString c935       = "-9E-35";
const QString c90035     = "-9.00E-35";
const QString cDATA      = "<DATA>";
const QString cDATAEnd   = "</DATA>";
const QString cSUBLOT = "SUBLOT=";
const QString cDATE = "DATE";
const QString cFILEPROCESSOR_VERSION = "FILEPROCESSOR_VERSION";
const QString cINPUT_FILE = "INPUT_FILE";
const QString cPROGRAM = "PROGRAM=";
const QString cPROGRAM_REVISION = "PROGRAM_REVISION";
const QString cPROGRAM_RELEASE = "PROGRAM_RELEASE";
const QString cLIMIT_SET_NAME = "LIMIT_SET_NAME";
const QString cLIMIT_SET_REVISION = "LIMIT_SET_REVISION";
const QString cLIMIT_SET_RELEASE = "LIMIT_SET_RELEASE";
const QString cFABEqual = "FAB=";
const QString cFAB = "FAB";
const QString cDATA_LOCATION = "DATA_LOCATION";
const QString cTECHNOLOGY = "TECHNOLOGY";
const QString cRETEST_PHASE = "RETEST_PHASE";
const QString cTEST_INSERTION = "TEST_INSERTION";
const QString cRETEST_INDEX = "RETEST_INDEX";
const QString cFAMILY = "FAMILY=";
const QString cPROCESS = "PROCESS=";
const QString cPROCESS_REVISION = "PROCESS_REVISION";
const QString cPRODUCT = "PRODUCT=";
const QString cPRODUCT_REVISION = "PRODUCT_REVISION";
const QString cSAP_MATERIAL_REVISION = "SAP_MATERIAL_REVISION";
const QString cLOT_ID = "LOT_ID=";
const QString cLOT_TYPE = "LOT_TYPE=";
const QString cSOURCE_LOT = "SOURCE_LOT";
const QString cWAFER_ID = "WAFER_ID=";
const QString cWAFER_NUMBER = "WAFER_NUMBER";
const QString cWAFER_NUMBEREqual = "WAFER_NUMBER=";
const QString cOPERATOR = "OPERATOR=";
const QString cFULL_OR_SAMPLE = "FULL_OR_SAMPLE";
const QString cTEST_CATEGORY = "TEST_CATEGORY";
const QString cTEST_TYPE = "TEST_TYPE";
const QString cPROBER_PASS = "PROBER_PASS";
const QString cPROBER_SETUP = "PROBER_SETUP";
const QString cEND_TIME = "END_TIME";
const QString cWAFER_END_TIME = "WAFER_END_TIME";
const QString cMAP_FILE = "MAP_FILE";
const QString cTEMPERATURE = "TEMPERATURE";
const QString cCAMPAIGN_1 = "CAMPAIGN_1";
const QString cCAMPAIGN_2 = "CAMPAIGN_2";
const QString cCAMPAIGN_3 = "CAMPAIGN_3";
const QString cRETEST_HBIN = "RETEST_HBIN";
const QString cSTARTING_MATERIAL = "STARTING_MATERIAL";
const QString cEPI_REACTOR = "EPI_REACTOR";
const QString cSUPPLIER_LOT_NUMBER = "SUPPLIER_LOT_NUMBER";
const QString cCASSETTE_POSITION = "CASSETTE_POSITION";
const QString cWAFER_START_TIME = "WAFER_START_TIME";
const QString cSTART_TIME = "START_TIME";
const QString cTRACKING_LOT = "TRACKING_LOT";
const QString cVENDOR_NAME = "VENDOR_NAME";
const QString cPROCESS_STEP = "PROCESS_STEP";
const QString cPROCESS_TYPE = "PROCESS_TYPE";
const QString cSB = "SB";
const QString cHB = "HB";
const QString cBIN_DEFINITION_DEF = "<BIN_DEFINITION_DEF>";
const QString cBIN_DEFINITION_DEFEnd = "</BIN_DEFINITION_DEF>";
const QString cBIN_DEFINITION_DATA = "<BIN_DEFINITION_DATA>";
const QString cDATA_DEFEnd = "</DATA_DEF>";


const QString cDIE_X = "DIE_X";
const QString cSITE_X = "SITE_X";
const QString cDIE_Y = "DIE_Y";
const QString cSITE_Y = "SITE_Y";

const QString cEPI_CASSETTE = "EPI_CASSETTE";
const QString cEPI_MOVEORDER = "EPI_MOVEORDER";
const QString cEPI_PONUMBER = "EPI_PONUMBER";
const QString cEPI_POTAG = "EPI_POTAG";
const QString cEPI_QLSERIALLASER = "EPI_QLSERIALLASER";
const QString cEPI_QLTESTID = "EPI_QLTESTID";
const QString cEPI_SHIPLOTID = "EPI_SHIPLOTID";
const QString cEPI_SLOT = "EPI_SLOT";
const QString cEPI_SUBSTRATELASER = "EPI_SUBSTRATELASER";
const QString cEPI_SUPPLIERID = "EPI_SUPPLIERID";
const QString cEPI_SUPPLIERLOTEqual = "EPI_SUPPLIERLOT=";
const QString cEPI_SUPPLIERLOT = "EPI_SUPPLIERLOT";
const QString cEPI_SUPPLIERLOTID = "EPI_SUPPLIERLOTID";
const QString cEPI_SUPPLIERWAFEREqual ="EPI_SUPPLIERWAFER=";
const QString cEPI_SUPPLIERWAFER ="EPI_SUPPLIERWAFER";
const QString cEPI_SUPPLIERWAFERID = "EPI_SUPPLIERWAFERID";
const QString cSITE = "<SITE>";
const QString cSITEEnd = "</SITE>";

//<HEADER>
//DATE=25-Feb-2015 15:33:28
//FILEPROCESSOR_VERSION=6.0.0.0
//INPUT_FILE=C:\Users\claudig\Desktop\STDFtoIFF\Eagle Probe\63741_6_6548975_11_EGL_X4_F.stdf.txt
//PROGRAM=WP:EGL_63741_6_X4_F_63741-J2
//PROGRAM_REVISION=F
//PROGRAM_RELEASE=NA
//LIMIT_SET_NAME=EGL_63741_6_X4_F_63741-J2
//LIMIT_SET_REVISION=F
//LIMIT_SET_RELEASE=NA
//FAB=NA
//DATA_LOCATION=NEWBURY PARK
//TECHNOLOGY=NA
//FAMILY=NA
//PROCESS=HBT4 M3 CU
//PROCESS_REVISION=NA
//PRODUCT=63741-J2
//PRODUCT_REVISION=NA
//SAP_MATERIAL_REVISION=NA
//LOT_ID=6548975.1
//LOT_TYPE=P
//SOURCE_LOT=6548975.S
//WAFER_ID=6548975-11
//WAFER_NUMBER=11
//FULL_OR_SAMPLE=NA
//TEST_CATEGORY=P
//TEST_TYPE=NA
//PROBER_PASS=NA
//PROBER_SETUP=NA
//START_TIME=16-Feb-2015 01:27:13
//END_TIME=16-Feb-2015 01:42:46
//OPERATOR=sm
//MAP_FILE=NA
//TEMPERATURE=NA
//CAMPAIGN_1=NA
//CAMPAIGN_2=NA
//CAMPAIGN_3=NA
//STARTING_MATERIAL=KP605
//EPI_REACTOR=24
//SUPPLIER_LOT_NUMBER=300309
//CASSETTE_POSITION=10
//</HEADER>
//<EQUIP_CONFIG>
//TRANSFORM_FILE=NA
//TESTER_ID=N_PRB44
//TESTER_TYPE=ETS364
//TEST_STAND=NA
//PROBER_ID=NA
//PROBER_TYPE=NA
//PROBE_CARD_ID=NA
//PROBE_CARD_TYPE=NA
//</EQUIP_CONFIG>
//<WAFER_MAP_CONFIG>
//WAFER_MAP_NAME=WP:EGL_63741_6_X4_F_63741-J2
//WAFER_DIAMETER=150
//....

const QString cIFF_DATA_BINMAP  =   "SBIN";
const QString cIFF_DATA_FT      =   "FT";
const QString cIFF_DATA_PROBE   =   "WP";
const QString cIFF_DATA_PCM     =   "PCM";

namespace GS
{
namespace Parser
{

int IFFUnitPrefixToScale(char prefix)
{
    int lScale = 0;

    switch(prefix)
    {
        // List of units we accept to scale to m, u, p, f, etc...
        case 'm':	// Milli...
            lScale = 3;
            break;

        case 'u':	// Micro...
            lScale = 6;
            break;

        case 'n':	// Nano...
            lScale = 9;
            break;

        case 'p':	// Pico...
            lScale = 12;
            break;

        case 'f':	// Fento...
            lScale = 15;
            break;

        case '%':
            lScale = 2;
            break;

        case 'k':	// Kilo...
            lScale = -3;
            break;

        case 'M':	// Mega...
            lScale = -6;
            break;

        case 'G':	// Giga...
            lScale = -9;
            break;

        default:
            break;
    }

    return lScale;
}

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
SkyworksIFFToSTDF::SkyworksIFFToSTDF()
    : ParserBase(typeSkyworksIFF, "typeSkyworksIFF"), mFinishTime(0)
{
    mStartTime = 0;


    mMonth.insert("Jan", 1);
    mMonth.insert("Feb", 2);
    mMonth.insert("Mar", 3);
    mMonth.insert("Apr", 4);
    mMonth.insert("May", 5);
    mMonth.insert("Jun", 6);
    mMonth.insert("Jul", 7);
    mMonth.insert("Aug", 8);
    mMonth.insert("Sep", 9);
    mMonth.insert("Oct", 10);
    mMonth.insert("Nov", 11);
    mMonth.insert("Dec", 12);

}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
SkyworksIFFToSTDF::~SkyworksIFFToSTDF()
{
    qDeleteAll(mDTRRecords);
    //mDTRRecords.clear();
    //mHardBinning.clear();
    //mSoftBinning.clear();
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool SkyworksIFFToSTDF::IsCompatible(const QString &FileName)
{
    gsbool	lIsCompatible(false);

    QFile lFile(FileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can not open file %1").arg(FileName).toLatin1().constData());
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lFile);


    // Check the compatibility
    QString lStrString("");
    while (lStrString.isEmpty() && !lInputFile.atEnd())
    {
        lStrString = lInputFile.readLine().trimmed();
    }

    //<HEADER>
    if (lStrString.startsWith(cHeader, Qt::CaseInsensitive))
    {
        lIsCompatible = true;
    }
    else
    {
        lIsCompatible =false;
    }

    gsuint16 lLineNumber(0);
    if (lIsCompatible)
    {
        lIsCompatible = false;
        while (lLineNumber != 1000
               && !lInputFile.atEnd())
        {
            lStrString = lInputFile.readLine().trimmed();
            ++lLineNumber;
            if (lStrString.startsWith(cHeaderEnd, Qt::CaseInsensitive))
            {
                lStrString = lInputFile.readLine().trimmed();
                if (lStrString.startsWith(cEquipConfig, Qt::CaseInsensitive))
                {
                    lIsCompatible = true;
                    break;
                }
            }
        }
    }

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
gsbool SkyworksIFFToSTDF::ConvertoStdf(const QString &SkyworksIFFFileName, QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( SkyworksIFFFileName).toLatin1().constData());

    // Open SkyworksIFF file
    QFile lFile(SkyworksIFFFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SkyworksIFF file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lStreamFile(&lFile);

    if (!ReadHeaderSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // the current line have to be equal to </header>
    QString lLine = ReadLine(lStreamFile);
    if (!lLine.startsWith(cEquipConfig)
        || !ReadEquipConfigSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // the current line have to be equal to </header>
    if (mIFFType != cIFF_DATA_FT)
    {
        lLine = ReadLine(lStreamFile);
        if (lLine.startsWith(cWaferMapConfig))
        {
            if(!ReadWaferMapConfigSection(lStreamFile))
            {
                mLastError = errInvalidFormatParameter;
                QFile::remove(StdfFileName);
                // Close file
                lFile.close();
                return false;
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "Missing Wafer Map configuration section.");
            mLastError = MissingWaferConfig;
            return false;
        }
    }

    // Read <LIMITS_DEF> section  without doing any thing
    gsbool lHasLimitDef(false);
    lLine = ReadLine(lStreamFile);
    if (lLine.startsWith(cLIMITS_DEF))
    {
        lHasLimitDef = true;
        while (!lStreamFile.atEnd()
               && !lLine.startsWith(cLIMITS_DEFEnd))
        {
            lLine = ReadLine(lStreamFile);
        }
    }


    // The current line have to be equal to </LIMITS_DEF>
    if (lHasLimitDef)
        lLine = ReadLine(lStreamFile);
    gsbool lHasLimitsSection(false);
    if (lLine.startsWith("<LIMITS>"))
        lHasLimitsSection = true;
    if (lHasLimitsSection && !ReadLimitsSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Read <LIMITS_DEF> section  without doing any thing
    if (lHasLimitsSection)
        lLine = ReadLine(lStreamFile);
    gsbool lHasBinDefinitionDef(false);
    if (lLine.startsWith(cBIN_DEFINITION_DEF))
    {
        lHasBinDefinitionDef = true;
        while (!lStreamFile.atEnd()
               && !lLine.startsWith(cBIN_DEFINITION_DEFEnd))
        {
            lLine = ReadLine(lStreamFile);
        }
    }

    // The current line have to be equal to </BIN_DEFINITION_DEF>
    if (lHasBinDefinitionDef)
        lLine = ReadLine(lStreamFile);
    gsbool lHasBinDefinitionData(false);
    if (lLine.startsWith(cBIN_DEFINITION_DATA))
        lHasBinDefinitionData = true;
    if (lHasBinDefinitionData && !ReadBinDefinitonsSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Read until we are at the end of Data definition
    while (!lStreamFile.atEnd()
           &&!lLine.startsWith(cDATA_DEFEnd, Qt::CaseInsensitive))
    {
        lLine = ReadLine(lStreamFile);
    }

    if (lStreamFile.atEnd())
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    if(WriteStdfFile(lStreamFile, StdfFileName) != true)
    {
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    mLastError = errNoError;
    // Success parsing SkyworksIFF file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SkyworksIFF data parsed
//////////////////////////////////////////////////////////////////////
gsbool SkyworksIFFToSTDF::WriteStdfFile(QTextStream &streamFile, const QString &StdfFileName)
{

    // now generate the STDF file...
    GS::StdLib::Stdf    lStdfFile;
    gsuint16            lTotalParts(0), lBadParts(0);

    if(lStdfFile.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing SkyworksIFF file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    if (mIFFType.isEmpty())
    {
        mLastError = UnknownDataType;

        // Conversion failed
        return false;
    }

    // Check StartTime and FinishTime
    if (mStartTime <= 0 && mFinishTime > 0)
    {
        mStartTime = mFinishTime;
    }
    else if (mStartTime <= 0 && mFinishTime <= 0)
    {
        mStartTime  = QDateTime::currentDateTime().toTime_t();
        mFinishTime = mStartTime;
    }
    else if (mFinishTime <= 0)
    {
        mFinishTime = mStartTime;
    }

    // Setupt start and finish time
    mMIRRecord.SetSTART_T(mStartTime);
    mMIRRecord.SetSETUP_T(mStartTime);
    mWIRRecord.SetSTART_T(mStartTime);

    mMRRRecord.SetFINISH_T(mFinishTime);
    mWRRRecord.SetFINISH_T(mFinishTime);

    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    lStdfFile.SetStdfCpuType(lStdfFile.GetComputerCpuType());

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;
    lFARrecord.SetCPU_TYPE(lStdfFile.GetComputerCpuType());	 // Force CPU type to current computer platform.
    lFARrecord.SetSTDF_VER(4);                               // STDF V4
    lFARrecord.Write(lStdfFile);

    // Write MIR
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    if (mIFFType == cIFF_DATA_PCM)
        strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    else
        strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":SkyworksIFF";
    mMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    mMIRRecord.Write(lStdfFile);

    for (int i=0; i<mDTRRecords.size(); ++i)
    {
        mDTRRecords[i]->Write(lStdfFile);
    }
    // Write SDR
    mSDRRecord.Write(lStdfFile);

    if (mIFFType != cIFF_DATA_FT)
        mWIRRecord.Write(lStdfFile);

    QString lLine("");
    while (!streamFile.atEnd())
    {
        if (lLine.startsWith(cSITE, Qt::CaseInsensitive))
        {
            mPRRRecord.Reset();
            mPIRRecord.Reset();
            QString lPartId("");
            bool    lOk;
            int lExecutedtest(0);

            gschar lPartFlag = 0x0;
            gsuchar lSiteNum(1);
            while (!streamFile.atEnd()
                   && !lLine.startsWith(cSITEEnd, Qt::CaseInsensitive))
            {
                gsint16 lHbin(0);
                gsbool lWriteSbinInHbin(false);

                while (!streamFile.atEnd()
                       && !lLine.startsWith(cDATA, Qt::CaseInsensitive))
                {
                    if ((lLine.startsWith(cSITEEqual, Qt::CaseInsensitive))
                            || (lLine.startsWith(cSITE_NOEqual, Qt::CaseInsensitive)))
                    {
                        int lField = lLine.section("=", 1).toInt(&lOk);
                        if((lOk) && (lField<256))
                        {
                            lSiteNum = lField;
                            mPRRRecord.SetSITE_NUM(static_cast<gsuchar>(lSiteNum));
                        }
                    }
                    else if (lLine.startsWith(cPCM_SITE, Qt::CaseInsensitive))
                    {
                        // Get PCM_SITE (ET only), and use it for part_id and site_num
                        lPartId = GetNotNAString(lLine.section("=", 1));
                        if (lPartId != "")
                        {
                            mPRRRecord.SetPART_ID(lPartId);
                            int lField = lPartId.toInt(&lOk);
                            if((lOk) && (lField<256))
                            {
                                lSiteNum = lField;
                                mPRRRecord.SetSITE_NUM(static_cast<gsuchar>(lSiteNum));
                            }
                        }
                    }
                    else if (lLine.startsWith(cDIE_ID, Qt::CaseInsensitive)
                             || lLine.startsWith(cDEVID, Qt::CaseInsensitive))
                    {
                        // ET: use this field to populate part_id if not already set by PCM_SITE
                        // WT, FT: use this field to populate part_id
                        if (lPartId == "")
                        {
                            lPartId = GetNotNAString(lLine.section("=", 1));
                            if (lPartId != "")
                                mPRRRecord.SetPART_ID(lPartId);
                        }
                    }
                    else if (lLine.startsWith(cDIE_X, Qt::CaseInsensitive)
                             || lLine.startsWith(cSITE_X, Qt::CaseInsensitive))
                    {
                        if (GetNotNAString(lLine.section("=", 1)) != "")
                            mPRRRecord.SetX_COORD(lLine.section("=", 1).toLong());
                    }
                    else if (lLine.startsWith(cDIE_Y, Qt::CaseInsensitive)
                             || lLine.startsWith(cSITE_Y, Qt::CaseInsensitive))
                    {
                        if (GetNotNAString(lLine.section("=", 1)) != "")
                            mPRRRecord.SetY_COORD(lLine.section("=", 1).toLong());
                    }
                    else if (lLine.startsWith(cHB, Qt::CaseInsensitive))
                    {
                        lHbin = lLine.section("=", 1).toLong();
                        // if the HBin=-1, then, if the list of Hbin red from the <BIN_DEFINITION_DATA> section
                        // is empty, write the SBin, otherwise, write return error
                        if (lHbin == -1)
                        {
                            if(!mHardBinning.isEmpty())
                            {
                                GSLOG(SYSLOG_SEV_ERROR, "The HBIN=-1 and the list of Hbins from <BIN_DEFINITION_DATA> section is not empty");
                                mLastError = errInvalidFormatParameter;
                                return false;
                            }
                            else
                            {
                                lWriteSbinInHbin = true;
                                lLine = ReadLine(streamFile);
                                continue;
                            }
                        }
                        if (mFailHBin.contains(lHbin))
                            lPartFlag |= 0x08;

                        // Fill software map
                        if(mHardBinning.contains(lHbin))
                            mHardBinning[lHbin].SetBinCount((mHardBinning[lHbin].GetBinCount()) + 1);

                        mPRRRecord.SetHARD_BIN(lHbin);
                    }
                    else if (lLine.startsWith(cSB, Qt::CaseInsensitive))
                    {
                        gsint16 lSbin = lLine.section("=", 1).toLong();
                        if (lSbin == -1)
                        {
                            if(!mSoftBinning.isEmpty())
                            {
                                GSLOG(SYSLOG_SEV_ERROR, "The SBIN=-1 and the list of Sbins from <BIN_DEFINITION_DATA> section is not empty");
                                mLastError = errInvalidFormatParameter;
                                return false;
                            }
                            else
                            {
                                mPRRRecord.SetSOFT_BIN(lHbin);
                                lLine = ReadLine(streamFile);
                                continue;
                            }
                        }
//                        if (mFailSBin.contains(lSbin))
//                            lPartFlag |= 0x08;
                        mPRRRecord.SetSOFT_BIN(lSbin);

                        // Fill software map
                         QMap<int,ParserBinning>::iterator  lIterSBin = mSoftBinning.find(lSbin);
                        if(lIterSBin != mSoftBinning.end())
                            lIterSBin.value().SetBinCount(( lIterSBin.value().GetBinCount()) + 1);

                        if (lWriteSbinInHbin)
                        {
                            lWriteSbinInHbin = false;
                            mPRRRecord.SetHARD_BIN(lSbin);
                        }
                    }
                    lLine = ReadLine(streamFile);
                }

                // if DATA => end of the site section and start the data section. Write the prr.partFlag
                if (lLine.startsWith(cDATA, Qt::CaseInsensitive))
                {
                    // Write the PIR
                    mPIRRecord.SetSITE_NUM(lSiteNum);
                    mPIRRecord.Write(lStdfFile);

                    // GCORE-xxx : HTH
                    // Wrong integer type used as index which restricted the number of test written in the STDF to 256
                    // gsuint8 lIndex(0);
                    gsint32 lIndex(0);
                    lExecutedtest = 0;

                    while (!streamFile.atEnd()
                           && !lLine.startsWith(cDATAEnd, Qt::CaseInsensitive))
                    {
                        if (lLine.contains(cDATA, Qt::CaseInsensitive))
                        {
                            lLine = ReadLine(streamFile);
                            continue;
                        }
                        mPTRRecord.Reset();
                        if (mTestList.size() <=  lIndex)
                        {
                            GSLOG(SYSLOG_SEV_ERROR, QString("The number of results (%1) is greater than the number od test %2")
                                  .arg(lIndex)
                                  .arg(mTestList.size()).toLatin1().constData());
                            mLastError = errInvalidFormatParameter;
                            return false;
                        }
                        ParserParameter& lTest = mTestList[lIndex];

                        mPTRRecord.SetTEST_NUM(lTest.GetTestNumber());
                        mPTRRecord.SetHEAD_NUM(1);
                        mPTRRecord.SetSITE_NUM(lSiteNum);

                        // GCORE-8570 : HTH
                        // Limit must not be strict. Measurament equal to the low or high limit must be considered as
                        // PASS
                        mPTRRecord.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80)); // B*1 Parametric test flags (drift, etc.)

                        if (GetNotNAString(lLine) == "")
                        {
                            mPTRRecord.SetTEST_FLG(0x42);
                        }
                        else
                        {
                            ++lExecutedtest;
                            gsfloat64 lValue = GS::Core::NormalizeValue(lLine.toDouble(), lTest.GetResultScale());
                            mPTRRecord.SetRESULT(lValue);
                            if (((lTest.GetValidHighLimit() == true) && (lValue > lTest.GetHighLimit()))
                                || ((lTest.GetValidLowLimit() == true) && (lValue < lTest.GetLowLimit())))
                            {
                                mPTRRecord.SetTEST_FLG(0x80);
//                                lPartFlag |= 0x08;
                                lTest.IncrementFailTest();
                            }
                            else
                                mPTRRecord.SetTEST_FLG(0);
                        }

                        lTest.IncrementExecTest();
                        mPTRRecord.SetTEST_TXT(lTest.GetTestName());
                        if (!lTest.GetStaticHeaderWritten())
                        {
                            mPTRRecord.SetALARM_ID("");
                            gsuchar lOptFlg = 0x02;
                            mPTRRecord.SetRES_SCAL(lTest.GetResultScale());
                            mPTRRecord.SetLLM_SCAL(lTest.GetResultScale());
                            mPTRRecord.SetHLM_SCAL(lTest.GetResultScale());
                            if (lTest.GetValidLowLimit())
                                mPTRRecord.SetLO_LIMIT(lTest.GetLowLimit());// R*4 Low test limit value OPT_FLAGbit 4 or 6 = 1
                            else
                                lOptFlg |= 0x50;
                            if (lTest.GetValidHighLimit())
                                mPTRRecord.SetHI_LIMIT(lTest.GetHighLimit());// R*4 High test limit value OPT_FLAGbit 5 or 7 = 1
                            else
                                lOptFlg |= 0xA0;
                            mPTRRecord.SetUNITS(lTest.GetTestUnits());
                            if (lTest.GetValidLowSpecLimit())
                            {
                                mPTRRecord.SetLO_SPEC(lTest.GetLowSpecLimit());// R*4 Low specification limit value OPT_FLAGbit 2 = 1
                            }
                            else
                                lOptFlg |= 0x04;
                            if (lTest.GetValidHighSpecLimit())
                            {
                                mPTRRecord.SetHI_SPEC(lTest.GetHighSpecLimit());// R*4 High specification limit value OPT_FLAGbit 3 = 1
                            }
                            else
                                lOptFlg |= 0x08;
                            mPTRRecord.SetOPT_FLAG(lOptFlg);
                            lTest.SetStaticHeaderWritten(true);
                        }
                        mPTRRecord.Write(lStdfFile);


                        if(!lTest.GetStaticMultiLimitsWritten())
                        {
                            for (int lML = 0; lML < lTest.GetMultiLimitCount(); ++lML)
                            {
                                 GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
                                 QJsonObject            lMultiLimit;
                                 GS::Core::MultiLimitItem   lMLSet = lTest.GetMultiLimitSetAt(lML);
                                 lMLSet.CreateJsonFromMultiLimit(lMultiLimit, (int)lTest.GetTestNumber());

                                 lDTRRecord.SetTEXT_DAT(lMultiLimit);
                                 if (lDTRRecord.Write(lStdfFile) == false)
                                 {
                                     GSLOG(SYSLOG_SEV_ERROR,
                                           QString("Failed to write multi-limits DTR").toLatin1().constData());
                                     mLastError = errWriteSTDF;
                                     return false;
                                 }
                            }
                            lTest.SetStaticMultiLimitsWritten(true);
                        }
                        ++lIndex;
                        lLine = ReadLine(streamFile);
                    }

                }
                lLine = ReadLine(streamFile);
            }

            // Write PRR
            ++lTotalParts;

            if(lPartFlag & 0x08)
                ++lBadParts;

            mPRRRecord.SetPART_FLG(lPartFlag);

            // HARD_BIN field has not been set due to information not provided
            // use the counter lBadPart with the default 0/1 hard bin value
            if(mPRRRecord.IsFieldValid(GQTL_STDF::Stdf_PRR_V4::eposHARD_BIN) == false )
            {
                if(lPartFlag & 0x08)
                    mPRRRecord.SetHARD_BIN(0); // FAILED
                else
                    mPRRRecord.SetHARD_BIN(1); // PASS
            }

            // SOFT_BIN field has not been set due to information not provided
            // use the counter lBadPart with the default 0/1 soft bin value
            if(mPRRRecord.IsFieldValid(GQTL_STDF::Stdf_PRR_V4::eposSOFT_BIN) == false )
            {
                if(lPartFlag & 0x08)
                    mPRRRecord.SetSOFT_BIN(0); // FAILED
                else
                    mPRRRecord.SetSOFT_BIN(1); // PASS
            }

            mPRRRecord.SetNUM_TEST(lExecutedtest);

            mPRRRecord.Write(lStdfFile);
        }
        if (lLine.startsWith(cPatReport, Qt::CaseInsensitive))
        {
            qDeleteAll(mDTRRecords);
            mDTRRecords.clear();
            if (!ReadPatSection(streamFile))
            {
                mLastError = errInvalidFormatParameter;
                return false;
            }
            for (int i=0; i<mDTRRecords.size(); ++i)
            {
                mDTRRecords[i]->Write(lStdfFile);
            }
        }
        lLine = ReadLine(streamFile);
    }

    if (mIFFType != cIFF_DATA_FT)
    {
        // Write WCR
        mWCRRecord.Write(lStdfFile);

        // Write WRR
        mWRRRecord.SetHEAD_NUM(1);
        mWRRRecord.SetSITE_GRP(255);
        mWRRRecord.SetPART_CNT(lTotalParts);
        mWRRRecord.SetRTST_CNT(GS_MAX_UINT32);
        mWRRRecord.SetABRT_CNT(GS_MAX_UINT32);
        mWRRRecord.SetGOOD_CNT(lTotalParts - lBadParts);
        mWRRRecord.Write(lStdfFile);
    }

    // Write HBR
    // write HBRs from HBR list if it isn't empty, else write them from SBR list
    if (mHardBinning.size() > 0)
    {
        foreach(const int i, mHardBinning.keys())
        {
            mHBR.Reset();
            mHBR.SetHEAD_NUM(255);
            mHBR.SetSITE_NUM(1);
            mHBR.SetHBIN_NUM(mHardBinning[i].GetBinNumber());
            mHBR.SetHBIN_NAM(mHardBinning[i].GetBinName());
            if(mHardBinning[i].GetPassFail())
                mHBR.SetHBIN_PF('P');
            else
                mHBR.SetHBIN_PF('F');
            mHBR.SetHBIN_CNT(mHardBinning[i].GetBinCount());
            mHBR.Write(lStdfFile);
        }
    }
    else
    {
        foreach (const int i, mSoftBinning.keys())
        {
            mHBR.Reset();
            mHBR.SetHEAD_NUM(255);
            mHBR.SetSITE_NUM(1);
            mHBR.SetHBIN_NUM(mSoftBinning[i].GetBinNumber());
            mHBR.SetHBIN_NAM(mSoftBinning[i].GetBinName());
            if(mSoftBinning[i].GetPassFail())
                mHBR.SetHBIN_PF('P');
            else
                mHBR.SetHBIN_PF('F');
            mHBR.SetHBIN_CNT(mSoftBinning[i].GetBinCount());
            mHBR.Write(lStdfFile);
        }
    }

    // Write SBR
    // write SBRs from SBR list if it isn't empty, else write them from HBR list
    if (mSoftBinning.size() > 0)
    {
        foreach (const int i, mSoftBinning.keys())
        {
            mSBR.Reset();
            mSBR.SetHEAD_NUM(255);
            mSBR.SetSITE_NUM(1);
            mSBR.SetSBIN_NUM(mSoftBinning[i].GetBinNumber());
            mSBR.SetSBIN_NAM(mSoftBinning[i].GetBinName());
            if(mSoftBinning[i].GetPassFail())
                mSBR.SetSBIN_PF('P');
            else
                mSBR.SetSBIN_PF('F');
            mSBR.SetSBIN_CNT(mSoftBinning[i].GetBinCount());
            mSBR.Write(lStdfFile);
        }
    }
    else
    {
        foreach(const int i, mHardBinning.keys())
        {
            mSBR.Reset();
            mSBR.SetHEAD_NUM(255);
            mSBR.SetSITE_NUM(1);
            mSBR.SetSBIN_NUM(mHardBinning[i].GetBinNumber());
            mSBR.SetSBIN_NAM(mHardBinning[i].GetBinName());
            if(mHardBinning[i].GetPassFail())
                mSBR.SetSBIN_PF('P');
            else
                mSBR.SetSBIN_PF('F');
            mSBR.SetSBIN_CNT(mHardBinning[i].GetBinCount());
            mSBR.Write(lStdfFile);
        }
    }

    // No Binning information. Create default
    if(mHardBinning.isEmpty() && mSoftBinning.isEmpty())
    {
        // NO SUMMARY FOR ETEST
        // Write SBR Bin0 (FAIL)
        WriteSBR(lStdfFile, 0,lBadParts, false);

        // Write SBR Bin1 (PASS)
        WriteSBR(lStdfFile, 1, lTotalParts - lBadParts, true);

        // Write HBR Bin0 (FAIL)
        WriteHBR(lStdfFile, 0, lBadParts, false);

        // Write HBR Bin1 (PASS)
        WriteHBR(lStdfFile, 1,lTotalParts - lBadParts, true);
    }


    // Write TSR
    GQTL_STDF::Stdf_TSR_V4 lTSRRecord;
    lTSRRecord.SetHEAD_NUM(255);

    for (unsigned short lIndex=0; lIndex<mTestList.size(); ++lIndex)
     {
        ParserParameter& lTest = mTestList[lIndex];
        lTSRRecord.SetHEAD_NUM(255);
        lTSRRecord.SetSITE_NUM(1);
        lTSRRecord.SetTEST_TYP('P');
        lTSRRecord.SetTEST_NUM(lTest.GetTestNumber());
        lTSRRecord.SetEXEC_CNT(lTest.GetExecCount());
//        lTSRRecord.SetFAIL_CNT(lTest.GetExecFail());
        lTSRRecord.SetTEST_NAM(lTest.GetTestName());
        lTSRRecord.Write(lStdfFile);
    }

    // Write PCR
    mPCRRecord.SetHEAD_NUM(255);
    mPCRRecord.SetSITE_NUM(1);
    mPCRRecord.SetPART_CNT(lTotalParts);
    mPCRRecord.SetRTST_CNT(GS_MAX_UINT32);
    mPCRRecord.SetABRT_CNT(GS_MAX_UINT32);
    mPCRRecord.SetGOOD_CNT(lTotalParts - lBadParts);
    mPCRRecord.Write(lStdfFile);

    // write MRR
    mMRRRecord.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

void SkyworksIFFToSTDF::WriteHBR( GS::StdLib::Stdf& stdfFile, int binNumber, int totalBins, bool isPassed )
{
    mHBR.Reset();
    mHBR.SetHEAD_NUM(255);
    mHBR.SetSITE_NUM(1);
    mHBR.SetHBIN_NUM(binNumber);
    if(isPassed)
        mHBR.SetHBIN_PF('P');
    else
        mHBR.SetHBIN_PF('F');
    mHBR.SetHBIN_CNT(totalBins);
    mHBR.Write(stdfFile);
}

void SkyworksIFFToSTDF::WriteSBR(GS::StdLib::Stdf& stdfFile, int binNumber, int totalBins, bool isPassed)
{
    mSBR.Reset();
    mSBR.SetHEAD_NUM(255);
    mSBR.SetSITE_NUM(1);
    mSBR.SetSBIN_NUM(binNumber);
    if(isPassed)
        mSBR.SetSBIN_PF('P');
    else
        mSBR.SetSBIN_PF('F');
    mSBR.SetSBIN_CNT(totalBins);
    mSBR.Write(stdfFile);
}


gsbool SkyworksIFFToSTDF::ReadHeaderSection(QTextStream &lInputFileStream)
{
    QString     lString;
    QString     lTestInsertionValue;

    while (lString.compare(cHeaderEnd) != 0
           && !lInputFileStream.atEnd())
    {
        if (lString.startsWith(cPROGRAM, Qt::CaseInsensitive))
        {
            QString lProgram = lString.section("=", 1);

            mIFFType = lProgram.section(":", 0, 0).toUpper();

            if (mIFFType != cIFF_DATA_PCM && mIFFType != cIFF_DATA_FT &&
                mIFFType != cIFF_DATA_PROBE && mIFFType != cIFF_DATA_BINMAP)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Unknown IFF data type: %1")
                      .arg(mIFFType).toLatin1().constData());
                return false;
            }

            mMIRRecord.SetJOB_NAM(lProgram);
        }
        else if (lString.startsWith(cSUBLOT, Qt::CaseInsensitive))
        {
            mMIRRecord.SetSBLOT_ID(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cPROGRAM_REVISION, Qt::CaseInsensitive))
        {
            mMIRRecord.SetJOB_REV(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cLIMIT_SET_NAME, Qt::CaseInsensitive))
        {
            mMIRRecord.SetSPEC_NAM(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cLIMIT_SET_REVISION, Qt::CaseInsensitive))
        {
            mMIRRecord.SetSPEC_VER(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cFAMILY, Qt::CaseInsensitive))
        {
            mMIRRecord.SetFAMLY_ID(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cDATA_LOCATION, Qt::CaseInsensitive))
        {
            mMIRRecord.SetFACIL_ID(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cPROCESS, Qt::CaseInsensitive))
        {
            mMIRRecord.SetPROC_ID(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cPRODUCT, Qt::CaseInsensitive))
        {
            mMIRRecord.SetPART_TYP(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cLOT_ID, Qt::CaseInsensitive))
        {
                mMIRRecord.SetLOT_ID(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cLOT_TYPE, Qt::CaseInsensitive))
        {
            if (lString.section("=", 1).compare(cNA,Qt::CaseInsensitive) != 0)
                mMIRRecord.SetMODE_COD(lString.section("=", 1)[0].toLatin1());
        }
        else if (lString.startsWith(cWAFER_ID, Qt::CaseInsensitive))
        {
            if (lString.section("=", 1).compare(cNA,Qt::CaseInsensitive) != 0)
            {
                mWIRRecord.SetWAFER_ID(lString.section("=", 1));
                mWRRRecord.SetWAFER_ID(lString.section("=", 1));
            }
        }
        else if (lString.startsWith(cWAFER_NUMBEREqual, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cWAFER_NUMBER, lString.section("=", 1));
        }
        else if (lString.startsWith(cTEST_CATEGORY, Qt::CaseInsensitive))
        {
            if (lString.section("=", 1).compare(cNA,Qt::CaseInsensitive) != 0)
            {
                mMIRRecord.SetMODE_COD(lString.section("=", 1)[0].toLatin1());
            }
        }
        else if (lString.startsWith(cOPERATOR, Qt::CaseInsensitive))
        {
                mMIRRecord.SetOPER_NAM(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cTEMPERATURE, Qt::CaseInsensitive))
        {
                mMIRRecord.SetTST_TEMP(GetNotNAString(lString.section("=", 1)));
        }
        else if (lString.startsWith(cSTARTING_MATERIAL, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cSTARTING_MATERIAL, lString.section("=", 1));
        }
        else if (lString.startsWith(cFABEqual, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cFAB, lString.section("=", 1));
        }
        else if (lString.startsWith(cTECHNOLOGY, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cTECHNOLOGY, lString.section("=", 1));
        }
        else if (lString.startsWith(cRETEST_PHASE, Qt::CaseInsensitive))
        {
            // TEST_INSERTION takes precedence over RETEST_PHASE key
            if(lTestInsertionValue.isEmpty())
                lTestInsertionValue = lString.section("=", 1);
        }
        else if (lString.startsWith(cTEST_INSERTION, Qt::CaseInsensitive))
        {
            lTestInsertionValue = lString.section("=", 1);
        }
        else if (lString.startsWith(cRETEST_INDEX, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cRETEST_INDEX, lString.section("=", 1));
        }
        else if (lString.startsWith(cRETEST_HBIN, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cRETEST_HBIN, lString.section("=", 1));
        }
        else if (lString.startsWith(cTEST_CATEGORY, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cTEST_CATEGORY, lString.section("=", 1));
        }
        else if (lString.startsWith(cCAMPAIGN_1, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cCAMPAIGN_1, lString.section("=", 1));
        }
        else if (lString.startsWith(cCAMPAIGN_2, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cCAMPAIGN_2, lString.section("=", 1));
        }
        else if (lString.startsWith(cCAMPAIGN_3, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cCAMPAIGN_3, lString.section("=", 1));
        }
        else if (lString.startsWith(cSTARTING_MATERIAL, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cSTARTING_MATERIAL, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_REACTOR, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_REACTOR, lString.section("=", 1));
        }
        else if (lString.startsWith(cSUPPLIER_LOT_NUMBER, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cSUPPLIER_LOT_NUMBER, lString.section("=", 1));
        }
        else if (lString.startsWith(cCASSETTE_POSITION, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cCASSETTE_POSITION, lString.section("=", 1));
        }
        else if (lString.startsWith(cTRACKING_LOT, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cTRACKING_LOT, lString.section("=", 1));
        }
        else if (lString.startsWith(cVENDOR_NAME, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cVENDOR_NAME, lString.section("=", 1));
        }
        else if (lString.startsWith(cPROCESS_STEP, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cPROCESS_STEP, lString.section("=", 1));
        }
        else if (lString.startsWith(cPROCESS_TYPE, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cPROCESS_TYPE, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_CASSETTE, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_CASSETTE, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_MOVEORDER, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_MOVEORDER, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_PONUMBER, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_PONUMBER, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_POTAG, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_POTAG, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_QLSERIALLASER, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_QLSERIALLASER, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_QLTESTID, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_QLTESTID, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SHIPLOTID, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SHIPLOTID, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SLOT, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SLOT, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SUBSTRATELASER, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SUBSTRATELASER, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SUPPLIERID, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SUPPLIERID, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SUPPLIERLOTEqual, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SUPPLIERLOT, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SUPPLIERLOTID, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SUPPLIERLOTID, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SUPPLIERWAFEREqual, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SUPPLIERWAFER, lString.section("=", 1));
        }
        else if (lString.startsWith(cEPI_SUPPLIERWAFERID, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cEPI_SUPPLIERWAFERID, lString.section("=", 1));
        }
        // START_TIME=16-Feb-2015 01:27:13
        else if (lString.startsWith(cSTART_TIME, Qt::CaseInsensitive)
                 || lString.startsWith(cWAFER_START_TIME, Qt::CaseInsensitive))
        {
            lString = lString.section("=", 1).simplified();

            if (lString.compare(cNA,Qt::CaseInsensitive) != 0)
            {
                QStringList lDateList = lString.section(" ", 0, 0).split('-');
                QDate lDate(1900, 1, 1);
                if (lDateList.count() == 3)
                {
                    int lYear = lDateList[2].toInt();
                    if (lYear < 70)
                        lYear += 2000;
                    else if (lYear < 99)
                        lYear += 1900;

                    lDate.setDate(lYear, mMonth.value(lDateList[1]), lDateList[0].toInt());
                }
                QString lTimeString = lString.section(" ", 1, 1);
                QTime lTime = QTime::fromString(lTimeString, "hh:mm:ss");

                QDateTime lDateTime = QDateTime(lDate, lTime);

                if(lDateTime.isValid() || lDateTime.isDaylightTime())
                {
                    lDateTime.setTimeSpec(Qt::UTC);
                    mStartTime = lDateTime.toTime_t();
                }
            }
        }
        // END_TIME=16-Feb-2015 01:42:46
        else if (lString.startsWith(cEND_TIME, Qt::CaseInsensitive)
                 || lString.startsWith(cWAFER_END_TIME, Qt::CaseInsensitive))
        {
            lString = lString.section("=", 1).simplified();

            if (lString.compare(cNA,Qt::CaseInsensitive) != 0)
            {
                QStringList lDateList = lString.section(" ", 0, 0).split('-');
                QDate lDate(1900, 1, 1);
                if (lDateList.count() == 3)
                {
                    int lYear = lDateList[2].toInt();
                    if (lYear < 70)
                        lYear += 2000;
                    else if (lYear < 99)
                        lYear += 1900;

                    lDate.setDate(lYear, mMonth.value(lDateList[1]), lDateList[0].toInt());
                }

                QString lTimeString = lString.section(" ", 1, 1);
                QTime lTime = QTime::fromString(lTimeString, "hh:mm:ss");

                QDateTime lDateTime = QDateTime(lDate, lTime);

                if(lDateTime.isValid() || lDateTime.isDaylightTime())
                {
                    lDateTime.setTimeSpec(Qt::UTC);
                    mFinishTime = lDateTime.toTime_t();
                }
            }
        }

        lString = ReadLine(lInputFileStream);
    }

    // Add TestInsertion key if found
    if(!lTestInsertionValue.isEmpty())
        AddMetaDataToDTR(cTEST_INSERTION, lTestInsertionValue);

    if (lString.startsWith(cHeaderEnd,  Qt::CaseInsensitive))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section HEADER with success");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section HEADER with error");
        return false;
    }
}

gsbool SkyworksIFFToSTDF::ReadEquipConfigSection(QTextStream &lInputFileStream)
{
    QString lStrString("");
    while (lStrString.compare(cEquipConfigEnd, Qt::CaseInsensitive) != 0
           && !lInputFileStream.atEnd())
    {
        // For Probe files
        if (lStrString.startsWith(cTesterID, Qt::CaseInsensitive))
            mMIRRecord.SetNODE_NAM(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cTesterType, Qt::CaseInsensitive))
            mMIRRecord.SetTSTR_TYP(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cProberID, Qt::CaseInsensitive))
            mSDRRecord.SetHAND_ID(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cProberType, Qt::CaseInsensitive))
            mSDRRecord.SetHAND_TYP(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cProdeCardID, Qt::CaseInsensitive))
            mSDRRecord.SetCARD_ID(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cProdeCardType, Qt::CaseInsensitive))
            mSDRRecord.SetCARD_TYP(GetNotNAString(lStrString.section("=", 1)));

        // For FTPI files
        else if (lStrString.startsWith(cTEST_FIXTURE, Qt::CaseInsensitive))
            mSDRRecord.SetDIB_ID(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cLoadBoardID, Qt::CaseInsensitive))
            mSDRRecord.SetLOAD_ID(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cHandlerID, Qt::CaseInsensitive))
            mSDRRecord.SetHAND_ID(GetNotNAString(lStrString.section("=", 1)));
        else if (lStrString.startsWith(cHandlerType, Qt::CaseInsensitive))
            mSDRRecord.SetHAND_TYP(GetNotNAString(lStrString.section("=", 1)));

        // common to all files
        else if (lStrString.startsWith(cTransformFile, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cTransformFile, GetNotNAString(lStrString.section("=", 1)));
        }
        else if (lStrString.startsWith(cTestStand, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cTestStand, GetNotNAString(lStrString.section("=", 1)));
        }

        lStrString = ReadLine(lInputFileStream);
    }
    if (lStrString.startsWith(cEquipConfigEnd,  Qt::CaseInsensitive))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section EQUIP_CONFIG with success");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section EQUIP_CONFIG with error");
        return false;
    }
}

QString SkyworksIFFToSTDF::GetNotNAString(QString string)
{
    QString lEmpty;
    if (string == cNA || string == c935  || string == c90035)
        return lEmpty;
    else
        return string;
}

gsbool SkyworksIFFToSTDF::ReadWaferMapConfigSection(QTextStream &lInputFileStream)
{

    QString lStrString("");
    int     lRescaledUnits = false;

    while (lStrString.compare(cWaferMapConfigEnd, Qt::CaseInsensitive) != 0
           && !lInputFileStream.atEnd())
    {
        if (lStrString.startsWith(cWaferDiameter, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.compare(c935, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetWAFR_SIZ(lStrString.toFloat());
            }
        }
        else if (lStrString.startsWith(cWaferUnits, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.isEmpty() == false && lStrString.compare(cNA, Qt::CaseInsensitive) != 0)
            {
                if (lStrString.startsWith(cMM, Qt::CaseInsensitive))
                    mWCRRecord.SetWF_UNITS(3);
                else if (lStrString.startsWith(cCM, Qt::CaseInsensitive))
                    mWCRRecord.SetWF_UNITS(2);
                else if (lStrString.startsWith(cIN, Qt::CaseInsensitive))
                    mWCRRecord.SetWF_UNITS(1);
                else if (lStrString.startsWith(cUM, Qt::CaseInsensitive))
                {
                    mWCRRecord.SetWF_UNITS(3);
                    lRescaledUnits = true;
                }
            }
        }
        else if (lStrString.startsWith(cFlatLocation, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.isEmpty() == false && lStrString.compare(cNA, Qt::CaseInsensitive) != 0)
            {
                if (lStrString.startsWith('T', Qt::CaseInsensitive))
                    mWCRRecord.SetWF_FLAT('U');
                else if (lStrString.startsWith('B', Qt::CaseInsensitive))
                    mWCRRecord.SetWF_FLAT('D');
                else if (lStrString.startsWith('R', Qt::CaseInsensitive))
                    mWCRRecord.SetWF_FLAT('R');
                else if (lStrString.startsWith('L', Qt::CaseInsensitive))
                    mWCRRecord.SetWF_FLAT('L');
                else
                    GSLOG(SYSLOG_SEV_WARNING,
                          QString("Invalid FLAT_LOCATION found: %1").arg(lStrString).toLatin1().constData());
            }
            else
                mWCRRecord.SetWF_FLAT(' ');
        }
        else if (lStrString.startsWith(cDIEWidth, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.compare(c935, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetDIE_WID(lStrString.toFloat());
            }
        }
        else if (lStrString.startsWith(cDIEHeight, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.compare(c935, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetDIE_HT(lStrString.toFloat());
            }
        }
        else if (lStrString.startsWith(cCenterX, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.compare(c935, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetCENTER_X(lStrString.toShort());
            }

        }
        else if (lStrString.startsWith(cCenterY, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.compare(c935, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetCENTER_Y(lStrString.toShort());
            }

        }
        else if (lStrString.startsWith(cPositiveX, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.isEmpty() == false && lStrString.compare(cNA, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetPOS_X(lStrString.at(0).toLatin1());
            }
        }
        else if (lStrString.startsWith(cPositiveY, Qt::CaseInsensitive))
        {
            lStrString = lStrString.section("=", 1).trimmed();

            if (lStrString.isEmpty() == false && lStrString.compare(cNA, Qt::CaseInsensitive) != 0)
            {
                mWCRRecord.SetPOS_Y(lStrString.at(0).toLatin1());
            }
        }
        else if (lStrString.startsWith(cFLDRows, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cFLDRows, lStrString.section("=", 1));
        }
        else if (lStrString.startsWith(cFLDCols, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cFLDCols, lStrString.section("=", 1));
        }
        else if (lStrString.startsWith(cFieldY, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cFieldY, lStrString.section("=", 1));
        }
        else if (lStrString.startsWith(cFieldX, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cFieldX, lStrString.section("=", 1));
        }
        else if (lStrString.startsWith(cSiteConfig, Qt::CaseInsensitive))
        {
            AddMetaDataToDTR(cSiteConfig, lStrString.section("=", 1));
        }

        lStrString = ReadLine(lInputFileStream);
    }

    // Units were micro-meter, so rescale diameter and die heigth/width to mm
    if (lRescaledUnits == true)
    {
        if (mWCRRecord.IsFieldValid(GQTL_STDF::Stdf_WCR_V4::eposWAFR_SIZ))
            mWCRRecord.SetWAFR_SIZ(mWCRRecord.m_r4WAFR_SIZ / 1000);

        if (mWCRRecord.IsFieldValid(GQTL_STDF::Stdf_WCR_V4::eposDIE_HT))
            mWCRRecord.SetDIE_HT(mWCRRecord.m_r4DIE_HT / 1000);

        if (mWCRRecord.IsFieldValid(GQTL_STDF::Stdf_WCR_V4::eposDIE_WID))
            mWCRRecord.SetDIE_WID(mWCRRecord.m_r4DIE_WID / 1000);
    }

    if (lStrString.startsWith(cWaferMapConfigEnd,  Qt::CaseInsensitive))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section WAFER_MAP_CONFIG with success");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section WAFER_MAP_CONFIG with error");
        return false;
    }
}

gsbool SkyworksIFFToSTDF::ReadLimitsSection(QTextStream &lInputFileStream)
{

    QString     lStrString;
    int         lScalingFactor = 0;
    while (lStrString.compare(cLimits, Qt::CaseInsensitive) != 0
           && !lInputFileStream.atEnd())
    {
        QStringList lFields = lStrString.split(",");
        if (lFields.size() > 11)
        {
            lScalingFactor = 0;

            ParserParameter lTest;
            lTest.SetTestNumber(lFields[0].toLong());
            lTest.SetTestName(lFields[1]);

            // Set the unit normalized and extract the scaling factor if any
            if (lFields[2].compare(cNA) != 0)
            {
                lTest.SetTestUnit(GS::Core::NormalizeUnit(lFields[2], lScalingFactor, IFFUnitPrefixToScale));
                lTest.SetResultScale(lScalingFactor);
            }

            if (lFields[4] != cNA && lFields[4] != cNALower)
            {
                lTest.SetLowLimit(GS::Core::NormalizeValue(lFields[4].toDouble(), lScalingFactor));
                lTest.SetValidLowLimit(true);
            }
            if (lFields[5] != cNA && lFields[5] != cNALower)
            {
                lTest.SetHighLimit(GS::Core::NormalizeValue(lFields[5].toDouble(), lScalingFactor));
                lTest.SetValidHighLimit(true);
            }
            if (lFields[6] != cNA && lFields[6] != cNALower)
            {
                lTest.SetLowSpecLimit(GS::Core::NormalizeValue(lFields[6].toDouble(), lScalingFactor));
                lTest.SetValidLowSpecLimit(true);
            }
            if (lFields[7] != cNA && lFields[7] != cNALower)
            {
                lTest.SetHighSpecLimit(GS::Core::NormalizeValue(lFields[7].toDouble(), lScalingFactor));
                lTest.SetValidHighSpecLimit(true);
            }

            bool lHasMultiLimit(false);
            if(lFields[8].compare(cNA, Qt::CaseInsensitive) != 0
               || lFields[9].compare(cNA, Qt::CaseInsensitive) != 0
               || lFields[10].compare(cNA, Qt::CaseInsensitive) != 0
               || lFields[11].compare(cNA, Qt::CaseInsensitive) != 0)
            {
                lHasMultiLimit = true;
            }
            if (lHasMultiLimit)
            {
                // Values here are already scaled
                GS::Core::MultiLimitItem lMultiLimitItem;
                // We only increment the binning
                gsint16 lBin(1);
                if (lTest.GetValidLowLimit() || lTest.GetValidHighLimit())
                {
                    if (lTest.GetValidLowLimit())
                        lMultiLimitItem.SetLowLimit(lTest.GetLowLimit());
                    else
                        lMultiLimitItem.SetLowLimit(std::numeric_limits<double>::quiet_NaN());
                    if (lTest.GetValidHighLimit())
                        lMultiLimitItem.SetHighLimit(lTest.GetHighLimit());
                    else
                        lMultiLimitItem.SetHighLimit(std::numeric_limits<double>::quiet_NaN());
                    lMultiLimitItem.SetHardBin(lBin);
                    lMultiLimitItem.SetSoftBin(lBin);
                    lTest.AddMultiLimitItem(lMultiLimitItem, ParserParameter::KeepDuplicateMultiLimit);
                    ++lBin;
                }
                if (lTest.GetValidLowSpecLimit() || lTest.GetValidHighSpecLimit())
                {
                    if(lTest.GetValidLowSpecLimit())
                        lMultiLimitItem.SetLowLimit(lTest.GetLowSpecLimit());
                    else
                        lMultiLimitItem.SetLowLimit(std::numeric_limits<double>::quiet_NaN());
                    if(lTest.GetValidHighSpecLimit())
                        lMultiLimitItem.SetHighLimit(lTest.GetHighSpecLimit());
                    else
                        lMultiLimitItem.SetHighLimit(std::numeric_limits<double>::quiet_NaN());
                    lMultiLimitItem.SetHardBin(lBin);
                    lMultiLimitItem.SetSoftBin(lBin);
                    lTest.AddMultiLimitItem(lMultiLimitItem, ParserParameter::KeepDuplicateMultiLimit);
                    ++lBin;
                }
                // Values here are not scaled
                if (lFields[8].compare(cNA, Qt::CaseInsensitive) != 0
                    || lFields[9].compare(cNA, Qt::CaseInsensitive) != 0)
                {
                    if(lFields[8].compare(cNA, Qt::CaseInsensitive) != 0)
                        lMultiLimitItem.SetLowLimit(GS::Core::NormalizeValue(lFields[8].toDouble(), lScalingFactor));
                    else
                        lMultiLimitItem.SetLowLimit(std::numeric_limits<double>::quiet_NaN());

                    if(lFields[9].compare(cNA, Qt::CaseInsensitive) != 0)
                        lMultiLimitItem.SetHighLimit(GS::Core::NormalizeValue(lFields[9].toDouble(), lScalingFactor));
                    else
                        lMultiLimitItem.SetHighLimit(std::numeric_limits<double>::quiet_NaN());
                    lMultiLimitItem.SetHardBin(lBin);
                    lMultiLimitItem.SetSoftBin(lBin);
                    lTest.AddMultiLimitItem(lMultiLimitItem, ParserParameter::KeepDuplicateMultiLimit);
                    ++lBin;
                }
                if (lFields[10].compare(cNA, Qt::CaseInsensitive) != 0
                    || lFields[11].compare(cNA, Qt::CaseInsensitive) != 0)
                {
                    if(lFields[10].compare(cNA, Qt::CaseInsensitive) != 0)
                        lMultiLimitItem.SetLowLimit(GS::Core::NormalizeValue(lFields[10].toDouble(), lScalingFactor));
                    else
                        lMultiLimitItem.SetLowLimit(std::numeric_limits<double>::quiet_NaN());

                    if(lFields[11].compare(cNA, Qt::CaseInsensitive) != 0)
                        lMultiLimitItem.SetHighLimit(GS::Core::NormalizeValue(lFields[11].toDouble(), lScalingFactor));
                    else
                        lMultiLimitItem.SetHighLimit(std::numeric_limits<double>::quiet_NaN());
                    lMultiLimitItem.SetHardBin(lBin);
                    lMultiLimitItem.SetSoftBin(lBin);
                    lTest.AddMultiLimitItem(lMultiLimitItem, ParserParameter::KeepDuplicateMultiLimit);
                    ++lBin;
                }
            }
            mTestList.append(lTest);
        }
        lStrString = ReadLine(lInputFileStream);
    }
    if (lStrString.startsWith(cLimits, Qt::CaseInsensitive))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section LIMITS with success");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section LIMITS with error");
        return false;
    }
}

gsbool SkyworksIFFToSTDF::ReadBinDefinitonsSection(QTextStream &lInputFileStream)
{

    QString lStrString("");
    while (lStrString.compare(cBinDefinitionDtat, Qt::CaseInsensitive) != 0
           && !lInputFileStream.atEnd())
    {
        QStringList lFields = lStrString.split(",");
        if (lFields.size() >= 4)
        {
            ParserBinning lBin;
            lBin.SetBinNumber(lFields[0].toUShort());
            lBin.SetBinName(lFields[1]);
            lBin.SetPassFail((lFields[2]=="P" || lFields[2]=="p")? true:false);
            if (lFields[3].compare(cSBin,Qt::CaseInsensitive) == 0)
            {
                if (lBin.GetPassFail() == false)
                    mFailSBin.append(lFields[0].toUShort());
                mSoftBinning[lFields[0].toUShort()]=lBin;
            }
            else
            {
                if (lBin.GetPassFail() == false)
                    mFailHBin.append(lFields[0].toUShort());
                mHardBinning[lFields[0].toUShort()]=lBin;
            }
        }
        lStrString = ReadLine(lInputFileStream);
    }
    if (lStrString.startsWith(cBinDefinitionDtat, Qt::CaseInsensitive))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section BIN_DEFINITION_DATA with success");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section BIN_DEFINITION_DATA with error");
        return false;
    }
}

gsbool SkyworksIFFToSTDF::ReadPatSection(QTextStream &lInputFileStream)
{
    QString     lLine("");
    while (lLine.compare(cPatReportEnd, Qt::CaseInsensitive) != 0
           && !lInputFileStream.atEnd())
    {
        lLine = lInputFileStream.readLine();
        if (lLine.startsWith(cCmd))
        {
            GQTL_STDF::Stdf_DTR_V4* lDTRRecord = new GQTL_STDF::Stdf_DTR_V4;
            lDTRRecord->SetTEXT_DAT(lLine);
            mDTRRecords.append(lDTRRecord);
        }

    }
    if (lLine.startsWith(cPatReportEnd, Qt::CaseInsensitive))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section PAT_REPORT with success");
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Read the section PAT_REPORT with error");
        return false;
    }
}


gsbool SkyworksIFFToSTDF::AddMetaDataToDTR(const QString &key, QString fieldValue)
{
    GQTL_STDF::Stdf_DTR_V4* lDTRRecord = new GQTL_STDF::Stdf_DTR_V4;
   // QString lFieldValue  = fieldValue;
    if(!( (fieldValue.compare(cNA, Qt::CaseInsensitive) != 0)
         &&(fieldValue.compare(c935, Qt::CaseInsensitive) != 0)))
        fieldValue = "";
    ParserBase::AddMetaDataToDTR(key, fieldValue,lDTRRecord);
    mDTRRecords.append(lDTRRecord);
    return true;
}

std::string SkyworksIFFToSTDF::GetErrorMessage(const int ErrorCode) const
{
    QString lError = "Import : ";

    switch(ErrorCode)
    {
        case UnknownDataType:
            lError += "Unable to detect IFF data type";
            break;

        case MissingWaferConfig:
            lError += "Wafermap configuration section is missing";
            break;

        default:
            lError += QString::fromStdString(ParserBase::GetErrorMessage(ErrorCode));
            break;
    }

    return lError.toStdString();
}


}
}
