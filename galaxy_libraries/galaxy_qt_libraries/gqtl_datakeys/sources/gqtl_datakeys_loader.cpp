/******************************************************************************!
 * \file gqtl_datakeys_loader.cpp
 ******************************************************************************/
#include <QFileInfo>
#include "gqtl_datakeys_loader.h"
#include "gqtl_log.h"
#include "stdfparse.h"
#include "stdfrecords_v4.h"
#include "stdfrecords_v3.h"
#include "gqtl_utils.h"
#include "gs_data.h"

namespace GS
{
namespace QtLib
{

#define GEX_IMPORT_DATAORIGIN_LABEL         "[GALAXY_DATA_ORIGIN]"

/******************************************************************************!
 * \fn DatakeysLoader
 ******************************************************************************/
DatakeysLoader::DatakeysLoader(QObject * parent /*= NULL*/)
    : QObject(parent)
{

}

/******************************************************************************!
 * \fn ~DatakeysLoader
 ******************************************************************************/
DatakeysLoader::~DatakeysLoader()
{
}

/******************************************************************************!
 * \fn DatakeysLoaderSetExtraKeys
 ******************************************************************************/
void
DatakeysLoaderSetExtraKeys(DatakeysContent& keysContent,
                           bool fromInputFile)
{
    int intval = keysContent.Get("RetestCode").toInt();
    if (intval >= '0' &&
        intval <= '9')
    {
        keysContent.Set("RetestIndex",
                        QChar(intval - '0'), fromInputFile);
    }
    else if (intval == 'Y')
    {
        keysContent.Set("RetestIndex", 1, fromInputFile);  // 1st retest
    }
    else
    {
        keysContent.Set("RetestIndex", 0, fromInputFile);  // Not a retest
    }

    if (keysContent.Get("UserText").toString().
        startsWith(GEX_IMPORT_DATAORIGIN_LABEL))
    {
        keysContent.Set("DataOrigin",
                        keysContent.Get("UserText").toString().
                        section(":", 2, 2),
                        fromInputFile);
    }
    else
    {
        keysContent.Set("DataOrigin",
                        keysContent.Get("Facility").toString(),
                        fromInputFile);
    }
}

/******************************************************************************!
 * \fn Load 1
 ******************************************************************************/
bool
DatakeysLoader::Load(struct GsData* lGsData,
                     int lSqliteSplitlotId,
                     const QString& dataFilePath,
                     DatakeysContent& keysContent,
                     QString& errorString,
                     bool fromInputFile)
{
    // Clear error string
    errorString = "";

    // Check if data file exists
    if(!QFileInfo(dataFilePath).exists())
    {
        errorString = QString("File %1 does not exist.").arg(dataFilePath);
        return false;
    }

    // Reset values
    keysContent.Set("SBRTotalPartsAllSites", 0, fromInputFile);
    keysContent.Set("SBRTotalParts", 0, fromInputFile);
    keysContent.Set("SBRTotalGoodBinsAllSites", 0, fromInputFile);
    keysContent.Set("SBRTotalGoodBins", 0, fromInputFile);
    keysContent.Set("HBRTotalPartsAllSites", 0, fromInputFile);
    keysContent.Set("HBRTotalParts", 0, fromInputFile);
    keysContent.Set("HBRTotalGoodBinsAllSites", 0, fromInputFile);
    keysContent.Set("HBRTotalGoodBins", 0, fromInputFile);
    keysContent.Set("PassBinNotInList", -1, fromInputFile);
    keysContent.Set("PassBinNotInListStr", "", fromInputFile);
    keysContent.Set("PRRBinning", 65535, fromInputFile);
    keysContent.Set("PassBinNotInList", -1, fromInputFile);
    keysContent.Set("PassBinNotInListStr", "", fromInputFile);
    keysContent.Set("TestingSite", QStringList(), fromInputFile);
    keysContent.Set("TotalGoodBins", 0, fromInputFile);
    keysContent.Set("TotalParts", 0, fromInputFile);
    keysContent.Set("TotalRecordsScanned", 0, fromInputFile);

    if (QFileInfo(dataFilePath).suffix().toLower() == "sqlite")
    {
        return LoadSqlite(lGsData,
                          lSqliteSplitlotId,
                          dataFilePath,
                          keysContent,
                          errorString,
                          fromInputFile);
    }
    else
    {
        // Futur
        return false;
    }
}

/******************************************************************************!
 * \fn Load 2
 ******************************************************************************/
bool
DatakeysLoader::Load(const QString& dataFilePath,
                     DatakeysContent& keysContent,
                     QString& errorString,
                     bool* checkPassBinNotInList,
                     Range* range_PassBinlistForRejectTest,
                          GS::QtLib::Range *globalYieldBinCheck,
                          bool fromInputFile/*=false*/)
{
    // Clear error string
    errorString = "";

    // Check if data file exists
    if(!QFileInfo(dataFilePath).exists())
    {
        errorString = QString("File %1 does not exist.").arg(dataFilePath);
        return false;
    }

    // Reset values
    keysContent.Set("SBRTotalPartsAllSites", 0, fromInputFile);
    keysContent.Set("SBRTotalParts", 0, fromInputFile);
    keysContent.Set("SBRTotalGoodBinsAllSites", 0, fromInputFile);
    keysContent.Set("SBRTotalGoodBins", 0, fromInputFile);
    keysContent.Set("HBRTotalPartsAllSites", 0, fromInputFile);
    keysContent.Set("HBRTotalParts", 0, fromInputFile);
    keysContent.Set("HBRTotalGoodBinsAllSites", 0, fromInputFile);
    keysContent.Set("HBRTotalGoodBins", 0, fromInputFile);
    keysContent.Set("PassBinNotInList", -1, fromInputFile);
    keysContent.Set("PassBinNotInListStr", "", fromInputFile);
    keysContent.Set("PRRBinning", 65535, fromInputFile);
    keysContent.Set("PassBinNotInList", -1, fromInputFile);
    keysContent.Set("PassBinNotInListStr", "", fromInputFile);
    keysContent.Set("TestingSite", QStringList(), fromInputFile);
    keysContent.Set("TotalGoodBins", 0, fromInputFile);
    keysContent.Set("TotalParts", 0, fromInputFile);
    keysContent.Set("TotalRecordsScanned", 0, fromInputFile);

    // Intialize the StdfRecords counter
    for(int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; ++i)
    {
        keysContent.StdfRecordsCount[i] = 0;
    }

    // Open Data file: only STDF V4 supported for now
    GQTL_STDF::StdfParse	lStdfReader;		// STDF reader

    // Open STDF file
    if(!lStdfReader.Open(dataFilePath.toLatin1().constData()))
    {
        errorString = QString("Error opening file %1 (not STDF V4 format?).").
            arg(dataFilePath);
        return false;
    }

    // Read all records and load info into keys object
    int lStatus = 0;
    int lRecordType = 0;
    bool    lLoadResult = false;
    int     lVersion = STDF_V_UNKNOWN;
    bool    lVersionRes = lStdfReader.GetVersion(&lVersion);
    if(!lVersionRes || (lVersion == STDF_V_UNKNOWN))
    {
        errorString =
            QString("UNKNOWN %1 (not STDF format?).").arg(dataFilePath);
        return false;
    }

    qlonglong lTotalRecordsScanned = 0;
    lStatus = lStdfReader.LoadNextRecord(&lRecordType);
    while ((lStatus == GQTL_STDF::StdfParse::NoError) ||
           (lStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        lTotalRecordsScanned++;
        keysContent.StdfRecordsCount[lRecordType]++;

        lLoadResult = true;
        switch(lRecordType)
        {
        case GQTL_STDF::Stdf_Record::Rec_MIR:
            lLoadResult = LoadMIR(lVersion, lStdfReader, keysContent,
                                  errorString, fromInputFile);
            break;
        case GQTL_STDF::Stdf_Record::Rec_MRR:
            lLoadResult = LoadMRR(lVersion, lStdfReader, keysContent,
                                  errorString, fromInputFile);
            break;
        case GQTL_STDF::Stdf_Record::Rec_SBR:
            lLoadResult = LoadSBR(lVersion, lStdfReader, keysContent,
                                  errorString, globalYieldBinCheck);
            break;
        case GQTL_STDF::Stdf_Record::Rec_HBR:
            lLoadResult = LoadHBR(lVersion, lStdfReader, keysContent,
                                  errorString, checkPassBinNotInList,
                                  range_PassBinlistForRejectTest,
                                  globalYieldBinCheck);
            break;
        case GQTL_STDF::Stdf_Record::Rec_SDR:
            lLoadResult = LoadSDR(lVersion, lStdfReader, keysContent,
                                  errorString);
            break;
        case GQTL_STDF::Stdf_Record::Rec_WIR:
            lLoadResult = LoadWIR(lVersion, lStdfReader, keysContent,
                                  errorString, fromInputFile);
            break;
        case GQTL_STDF::Stdf_Record::Rec_WRR:
            lLoadResult = LoadWRR(lVersion, lStdfReader, keysContent,
                                  errorString, fromInputFile);
            break;
        case GQTL_STDF::Stdf_Record::Rec_WCR:
            lLoadResult = LoadWCR(lVersion, lStdfReader, keysContent,
                                  errorString, fromInputFile);
            break;
        case GQTL_STDF::Stdf_Record::Rec_DTR:
            lLoadResult = LoadDTR(lVersion, lStdfReader, keysContent,
                                  errorString);
            break;
        case GQTL_STDF::Stdf_Record::Rec_PRR:
            lLoadResult = LoadPRR(lVersion, lStdfReader, keysContent,
                                  errorString, checkPassBinNotInList,
                                  range_PassBinlistForRejectTest,
                                  globalYieldBinCheck);
            break;
        case GQTL_STDF::Stdf_Record::Rec_GDR:
        {
            GQTL_STDF::Stdf_GDR_V4 gdr;
            lStdfReader.ReadRecord( &gdr );

            // set deciphering mode for site number if needed
            GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser
                ( gdr, lStdfReader );

            break;
        }
        default:
            break;
        }

        if(!lLoadResult)
        {
            lStdfReader.Close();
            keysContent.Set("TotalRecordsScanned",
                            QVariant(lTotalRecordsScanned), fromInputFile);
            return false;
        }

        // Load next record
        lStatus = lStdfReader.LoadNextRecord(&lRecordType);
    }

    // Close STDF file
    lStdfReader.Close();
    keysContent.Set("TotalRecordsScanned", QVariant(lTotalRecordsScanned), fromInputFile);

    if(keysContent.Get("TotalParts").toInt() == 0)
    {
        if(keysContent.StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR]>0)
        {
            keysContent.Set("TotalParts",
                            keysContent.StdfRecordsCount[GQTL_STDF::Stdf_Record::Rec_PRR], fromInputFile);
        }
    }

    return true;
}

/******************************************************************************!
 * \fn LoadMIR
 ******************************************************************************/
bool
DatakeysLoader::LoadMIR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool fromInputFile  /*=false*/)
{
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_MIR_V4 lMIR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lMIR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        // Set fields in keys object
        keysContent.Set("SetupTime",
                        QVariant((qlonglong) lMIR.m_u4SETUP_T), fromInputFile);
        keysContent.Set("StartTime",
                        QVariant((qlonglong) lMIR.m_u4START_T), fromInputFile);

        keysContent.Set("Station",QVariant(lMIR.m_u1STAT_NUM), fromInputFile);
        // mode_code
        if(lMIR.m_c1MODE_COD == 'P')
        {
            keysContent.Set("ProdData",true, fromInputFile);
        }
        else
        {
            keysContent.Set("ProdData",false, fromInputFile);
        }
        keysContent.Set("DataType",(QChar)lMIR.m_c1MODE_COD, fromInputFile);
        keysContent.Set("TestFlow",(QChar)lMIR.m_c1MODE_COD, fromInputFile);

        // rtst_code
        keysContent.Set("RetestCode",
                        QVariant((QChar) lMIR.m_c1RTST_COD), fromInputFile);
        if (lMIR.m_c1RTST_COD >= '0' &&
            lMIR.m_c1RTST_COD <= '9')
        {
            // Retest count: 1 = first retest, etc.
            keysContent.Set("RetestIndex",
                            QChar(lMIR.m_c1RTST_COD - '0'), fromInputFile);
        }
        else if (lMIR.m_c1RTST_COD == 'Y')
        {
            keysContent.Set("RetestIndex", 1, fromInputFile);  // 1st retest
        }
            else
        {
            keysContent.Set("RetestIndex", 0, fromInputFile);  // Not a retest
        }

        // prot_cod #
        keysContent.Set("ProtectionCode",
                        QVariant((QChar) lMIR.m_c1PROT_COD), fromInputFile);
        // burn_time
        keysContent.Set("BurninTime",
                        QString::number(lMIR.m_u2BURN_TIM), fromInputFile);
        // cmode_code
        keysContent.Set("CommandModeCode",
                        QVariant((QChar) lMIR.m_c1CMOD_COD), fromInputFile);

        // LOT_ID
        keysContent.Set("Lot",lMIR.m_cnLOT_ID.trimmed(), fromInputFile);
        keysContent.Set("SubconLot", lMIR.m_cnLOT_ID.trimmed(), fromInputFile);
        keysContent.Set("TrackingLot", lMIR.m_cnLOT_ID.trimmed(),
                        fromInputFile);

        // PART_TYP: ProductID
        keysContent.Set("Product", lMIR.m_cnPART_TYP.trimmed(), fromInputFile);

        // Tester name
        keysContent.Set("TesterName",lMIR.m_cnNODE_NAM, fromInputFile);

        // Tester type
        keysContent.Set("TesterType", lMIR.m_cnTSTR_TYP, fromInputFile);

        // Program name
        keysContent.Set("ProgramName",lMIR.m_cnJOB_NAM, fromInputFile);

        // Program Revision
        keysContent.Set("ProgramRevision", lMIR.m_cnJOB_REV, fromInputFile);

        // Sublot
        keysContent.Set("SubLot", lMIR.m_cnSBLOT_ID, fromInputFile);

        // OPER_NAM
        keysContent.Set("Operator", lMIR.m_cnOPER_NAM, fromInputFile);

        // ignore...EXEC_TYP
        keysContent.Set("ExecType",QVariant(lMIR.m_cnEXEC_TYP), fromInputFile);
        // ignore...EXEC_VER
        keysContent.Set("ExecVersion",
                        QVariant(lMIR.m_cnEXEC_VER), fromInputFile);

        // TEST_COD
        keysContent.Set("TestingCode", lMIR.m_cnTEST_COD, fromInputFile);

        // TST_TEMP
        keysContent.Set("Temperature", lMIR.m_cnTST_TEMP, fromInputFile);

        // USER_TXT
        keysContent.Set("UserText",lMIR.m_cnUSER_TXT, fromInputFile);

        // ignore...AUX_FILE
        keysContent.Set("AuxiliaryFile",
                        QVariant(lMIR.m_cnAUX_FILE), fromInputFile);

        // Package type
        keysContent.Set("PackageType", lMIR.m_cnPKG_TYP, fromInputFile);

        // Family ID
        keysContent.Set("Family", lMIR.m_cnFAMLY_ID, fromInputFile);

        // DATE_COD
        keysContent.Set("DateCode", lMIR.m_cnDATE_COD, fromInputFile);

        // FACIL_ID
        keysContent.Set("Facility", lMIR.m_cnFACIL_ID , fromInputFile);

        // FLOOR_ID
        keysContent.Set("Floor", lMIR.m_cnFLOOR_ID, fromInputFile);

        // PROC_ID
        keysContent.Set("Process", lMIR.m_cnPROC_ID, fromInputFile);

        // OPER_FREQ
        keysContent.Set("FrequencyStep", lMIR.m_cnOPER_FRQ, fromInputFile);

        // SPEC_NAM
        keysContent.Set("SpecName", lMIR.m_cnSPEC_NAM, fromInputFile);

        // SPEC_VER
        keysContent.Set("SpecVersion",
                        QVariant(lMIR.m_cnSPEC_VER), fromInputFile);
        // FLOW_ID
        keysContent.Set("Flow",QVariant(lMIR.m_cnFLOW_ID), fromInputFile);
        // SETUP_ID
        keysContent.Set("Setup",QVariant(lMIR.m_cnSETUP_ID), fromInputFile);

        // DSGN_REV
        keysContent.Set("DesignRevision", lMIR.m_cnDSGN_REV, fromInputFile);

        // ENG_ID
        keysContent.Set("Engineering", QVariant(lMIR.m_cnENG_ID),
                        fromInputFile);
        // ROM_COD
        keysContent.Set("RomCode",QVariant(lMIR.m_cnROM_COD), fromInputFile);
        // SERL_NUM
        keysContent.Set("SerialNumber",
                        QVariant(lMIR.m_cnSERL_NUM), fromInputFile);
        // SUPR_NAME
        keysContent.Set("SupervisorName",
                        QVariant(lMIR.m_cnSUPR_NAM), fromInputFile);

        DatakeysLoaderSetExtraKeys(keysContent, fromInputFile);
    }
    else //STDF_V_3
    {
        GQTL_STDF::Stdf_MIR_V3 lMIR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lMIR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        // mode_code
        if(lMIR.m_c1MODE_COD == 'P')
        {
            keysContent.Set("ProdData",true, fromInputFile);
        }
        else
        {
            keysContent.Set("ProdData",false, fromInputFile);
        }
        keysContent.Set("DataType",(QChar)lMIR.m_c1MODE_COD, fromInputFile);
        keysContent.Set("TestFlow",(QChar)lMIR.m_c1MODE_COD, fromInputFile);

        // rtst_code
        keysContent.Set("RetestCode",QVariant((QChar) lMIR.m_c1RTST_COD), fromInputFile);
        if (lMIR.m_c1RTST_COD >= '0' &&
            lMIR.m_c1RTST_COD <= '9')
        {
            // Retest count: 1 = first retest, etc.
            keysContent.Set("RetestIndex",
                            QChar(lMIR.m_c1RTST_COD - '0'), fromInputFile);
        }
        else if (lMIR.m_c1RTST_COD == 'Y')
        {
            keysContent.Set("RetestIndex", 1, fromInputFile);  // 1st retest
        }
        else
        {
            keysContent.Set("RetestIndex", 0, fromInputFile);  // Not a retest
        }

        // prot_cod #
        keysContent.Set("ProtectionCode", QVariant(
                            (QChar) lMIR.m_c1PROT_COD), fromInputFile);
        // cmode_code
        keysContent.Set("CommandModeCode", QVariant(
                            (QChar) lMIR.m_c1CMOD_COD), fromInputFile);
        // Set fields in keys object
        keysContent.Set("SetupTime",
                        QVariant((qlonglong) lMIR.m_u4SETUP_T), fromInputFile);
        keysContent.Set("StartTime",
                        QVariant((qlonglong) lMIR.m_u4START_T), fromInputFile);

        // LOT_ID
        keysContent.Set("Lot",lMIR.m_cnLOT_ID.trimmed(), fromInputFile);
        keysContent.Set("SubconLot", lMIR.m_cnLOT_ID.trimmed(), fromInputFile);
        keysContent.Set("TrackingLot", lMIR.m_cnLOT_ID.trimmed(),
                        fromInputFile);

        // PART_TYP: ProductID
        keysContent.Set("Product", lMIR.m_cnPART_TYP.trimmed(), fromInputFile);

        // Program name
        keysContent.Set("ProgramName",lMIR.m_cnJOB_NAM, fromInputFile);
        // OPER_NAM
        keysContent.Set("Operator", lMIR.m_cnOPER_NAM, fromInputFile);
        // Tester name
        keysContent.Set("TesterName",lMIR.m_cnNODE_NAM, fromInputFile);

        // Tester type
        keysContent.Set("TesterType", lMIR.m_cnTSTR_TYP, fromInputFile);

        // ignore...EXEC_TYP
        keysContent.Set("ExecType",QVariant(lMIR.m_cnEXEC_TYP), fromInputFile);
        // ignore...EXEC_VER
        keysContent.Set("ExecVersion",
                        QVariant(lMIR.m_cnEXEC_TYP), fromInputFile);

        keysContent.Set("ProberType",
                        keysContent.Get("ProberName"), fromInputFile);

        // Sublot
        keysContent.Set("SubLot", lMIR.m_cnSBLOT_ID, fromInputFile);

        keysContent.Set("ProgramRevision", lMIR.m_cnJOB_REV, fromInputFile);
        // PROC_ID
        keysContent.Set("Process", lMIR.m_cnPROC_ID, fromInputFile);

        /* NOT FOUND
           // LOAD_ID: Prob card, Load board name
           ReadStringToField(&StdfFile,szString);
                keysContent.strLoadBoardName = szString;

                // LOAD_TYP: Load board type
                keysContent.strLoadBoardType = keysContent.strLoadBoardName;
           break;// MIR STDF V3 read, so register file into database
         */

        DatakeysLoaderSetExtraKeys(keysContent, fromInputFile);
    }

    return true;
}

/******************************************************************************!
 * \fn LoadHBR
 ******************************************************************************/
bool
DatakeysLoader::LoadHBR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool* checkPassBinNotInList,
                        GS::QtLib::Range* range_PassBinlistForRejectTest,
                        GS::QtLib::Range* globalYieldBinCheck)
{
    if ((checkPassBinNotInList == NULL) ||
            (range_PassBinlistForRejectTest == NULL) ||
            (globalYieldBinCheck == NULL))
    {
        return true;
    }

    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_HBR_V4 lHBR;

        // HBR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lHBR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(keysContent.Get("HBRTotalPartsAllSites").isNull())
        {
            keysContent.Set("HBRTotalPartsAllSites", 0, true);
        }
        if(keysContent.Get("HBRTotalParts").isNull())
        {
            keysContent.Set("HBRTotalParts", 0, true);
        }
        if(keysContent.Get("HBRTotalGoodBins").isNull())
        {
            keysContent.Set("HBRTotalGoodBins", 0, true);
        }
        if(keysContent.Get("PassBinNotInList").isNull())
        {
            keysContent.Set("PassBinNotInList", -1, true);
        }
        if(keysContent.Get("PassBinNotInListStr").isNull())
        {
            keysContent.Set("PassBinNotInListStr", "", true);
        }

        if(lHBR.m_u1HEAD_NUM == 255)
        {
            keysContent.Set("HBRTotalPartsAllSites",
                            keysContent.
                            Get("HBRTotalPartsAllSites").toULongLong() +
                            lHBR.m_u4HBIN_CNT, true);
        }
        else  // Update total number of parts tested
        {
            keysContent.Set("HBRTotalParts",
                            keysContent.Get("HBRTotalParts").toULongLong() +
                            lHBR.m_u4HBIN_CNT, true);
        }

        if(globalYieldBinCheck->Contains(lHBR.m_u2HBIN_NUM) == true)
        {
            if (lHBR.m_u2HBIN_NUM == 255)
            {
                // Head=255: so number is cumul cound over ALL sites
                keysContent.Set("HBRTotalGoodBinsAllSites",
                                (qulonglong) lHBR.m_u4HBIN_CNT, true);
            }
            else  // Add up to merge with aother sites
            {
                keysContent.Set("HBRTotalGoodBins",
                                keysContent.
                                Get("HBRTotalGoodBins").toULongLong() +
                                lHBR.m_u4HBIN_CNT, true);
            }
        }

        // -- keep in memory the list of the HBinNum
        QVariant lListHBinNum = keysContent.Get("HBRListBinNumber");
        QString lStrListHBinNum;

        if(lListHBinNum.isNull() )
        {
            lStrListHBinNum.append(QString::number(lHBR.m_u2HBIN_NUM));
            keysContent.Set("HBRListBinNumber", lStrListHBinNum);
        }
        else
        {
            lStrListHBinNum = lListHBinNum.toString();
            //-- check that not already in the list
            if(lStrListHBinNum.contains(QString::number(lHBR.m_u2HBIN_NUM)) == false)
            {
                lStrListHBinNum.append(",");
                lStrListHBinNum.append(QString::number(lHBR.m_u2HBIN_NUM));
                keysContent.Set("HBRListBinNumber", lStrListHBinNum);

            }
        }

        // Check if PASS bin not in specified list?
        if (*checkPassBinNotInList &&
            ((lHBR.m_c1HBIN_PF == 'P') ||
             (lHBR.m_c1HBIN_PF == 'p')) &&
                !range_PassBinlistForRejectTest->Contains(lHBR.m_u2HBIN_NUM))
        {
            keysContent.Set("PassBinNotInList", lHBR.m_u2HBIN_NUM, true);
            keysContent.Set("PassBinNotInListStr", lHBR.m_cnHBIN_NAM, true);
        }

    }
    else//STDF_V_3
    {
        GQTL_STDF::Stdf_HBR_V3 lHBR;

        // HBR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lHBR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(keysContent.Get("HBRTotalParts").isNull())
        {
            keysContent.Set("HBRTotalParts", 0, true);
        }
        if(keysContent.Get("HBRTotalPartsAllSites").isNull())
        {
            keysContent.Set("HBRTotalPartsAllSites", 0, true);
        }
        if(keysContent.Get("HBRTotalGoodBins").isNull())
        {
            keysContent.Set("HBRTotalGoodBins", 0, true);
        }

        // Update total number of parts tested
        keysContent.Set("HBRTotalParts",
                        keysContent.Get("HBRTotalParts").toULongLong() +
                        lHBR.m_u4HBIN_CNT, true);
        keysContent.Set("HBRTotalPartsAllSites",
                        keysContent.Get("HBRTotalParts").toInt(), true);
        if(globalYieldBinCheck->Contains(lHBR.m_u2HBIN_NUM) == true)
        {
            keysContent.Set("HBRTotalGoodBins",
                            keysContent.Get("HBRTotalGoodBins").toULongLong() +
                            lHBR.m_u4HBIN_CNT, true);
        }
    }

    return true;
}

/******************************************************************************!
 * \fn LoadSBR
 ******************************************************************************/
bool
DatakeysLoader::LoadSBR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        GS::QtLib::Range* globalYieldBinCheck)
{
    if(globalYieldBinCheck == NULL)
    {
        return true;
    }

    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_SBR_V4 lSBR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lSBR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        if(keysContent.Get("SBRTotalPartsAllSites").isNull())
        {
            keysContent.Set("SBRTotalPartsAllSites", 0, true);
        }

        if(keysContent.Get("SBRTotalParts").isNull())
        {
            keysContent.Set("SBRTotalParts", 0, true);
        }
        if(keysContent.Get("SBRTotalGoodBins").isNull())
        {
            keysContent.Set("SBRTotalGoodBins", 0, true);
        }

        if(lSBR.m_u1HEAD_NUM == 255)
        {
            keysContent.Set("SBRTotalPartsAllSites",
                            keysContent.
                            Get("SBRTotalPartsAllSites").toULongLong() +
                            lSBR.m_u4SBIN_CNT, true);
        }
        else  // Update total number of parts tested
        {
             keysContent.Set("SBRTotalParts",
                            keysContent.Get("SBRTotalParts").toULongLong() +
                            lSBR.m_u4SBIN_CNT, true);
        }

        if(globalYieldBinCheck->Contains(lSBR.m_u2SBIN_NUM) == true)
        {
            if (lSBR.m_u1HEAD_NUM == 255)
            {
                // Head=255: so number is cumul cound over ALL sites.
                keysContent.Set("SBRTotalGoodBinsAllSites",
                                (qulonglong) lSBR.m_u4SBIN_CNT, true);
            }
            else  // Add up to merge with aother sites
            {
                keysContent.Set("SBRTotalGoodBins",
                                keysContent.
                                Get("SBRTotalGoodBins").toULongLong() +
                                (qulonglong) lSBR.m_u4SBIN_CNT, true);
            }
        }

        // -- keep in memory the list of the HBinNum
        QVariant lListSBinNum = keysContent.Get("SBRListBinNumber");
        QString lStrListSBinNum;

        if(lListSBinNum.isNull() )
        {
            lStrListSBinNum.append(QString::number(lSBR.m_u2SBIN_NUM));
            keysContent.Set("SBRListBinNumber", lStrListSBinNum);
        }
        else
        {
            lStrListSBinNum = lListSBinNum.toString();
            //-- check that not already in the list
            if(lStrListSBinNum.contains(QString::number(lSBR.m_u2SBIN_NUM)) == false)
            {
                lStrListSBinNum.append(",");
                lStrListSBinNum.append(QString::number(lSBR.m_u2SBIN_NUM));
                keysContent.Set("SBRListBinNumber", lStrListSBinNum);
            }
        }
    }
    else//STDF_V_3
    {
        GQTL_STDF::Stdf_SBR_V3 lSBR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lSBR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        if(keysContent.Get("SBRTotalPartsAllSites").isNull())
        {
            keysContent.Set("SBRTotalPartsAllSites", 0, true);
        }

        if(keysContent.Get("SBRTotalParts").isNull())
        {
            keysContent.Set("SBRTotalParts", 0, true);
        }
        if(keysContent.Get("SBRTotalGoodBins").isNull())
        {
            keysContent.Set("SBRTotalGoodBins", 0, true);
        }
        // Update total number of parts tested
        keysContent.Set("SBRTotalParts",
                        keysContent.Get("SBRTotalParts").toULongLong() +
                        lSBR.m_u4SBIN_CNT, true);
        keysContent.Set("SBRTotalPartsAllSites",
                        keysContent.Get("SBRTotalParts").toULongLong());
        if(globalYieldBinCheck->Contains(lSBR.m_u2SBIN_NUM) == true)
        {
            keysContent.Set("SBRTotalGoodBins",
                            keysContent.Get("SBRTotalGoodBins").toULongLong() +
                            lSBR.m_u4SBIN_CNT, true);
        }
    }

    return true;
}

/******************************************************************************!
 * \fn LoadPRR
 ******************************************************************************/
bool
DatakeysLoader::LoadPRR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool* checkPassBinNotInList,
                        Range* range_PassBinlistForRejectTest,
                        GS::QtLib::Range* globalYieldBinCheck)
{
    if ((checkPassBinNotInList == NULL) ||
            (range_PassBinlistForRejectTest == NULL) ||
            (globalYieldBinCheck == NULL))
    {
        return true;
    }
    unsigned short lTestingSite;
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_PRR_V4 lPRR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lPRR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(keysContent.Get("PRRBinning").isNull())
        {
            keysContent.Set("PRRBinning", 65535, true);
        }

        if(keysContent.Get("PassBinNotInList").isNull())
        {
            keysContent.Set("PassBinNotInList", -1, true);
        }

        if(keysContent.Get("PassBinNotInListStr").isNull())
        {
            keysContent.Set("PassBinNotInListStr", "", true);
        }

        if(lPRR.m_u2HARD_BIN == 65535)
        {
            keysContent.Set("PRRBinning", lPRR.m_u2SOFT_BIN, true);
        }
        else
        {
            keysContent.Set("PRRBinning", lPRR.m_u2HARD_BIN, true);
        }

        // Check if PASS bin not in specified list?
        // Do not check 1st 'Test plan delimiter' dummy run used
        // on Eagle testers
        if(	*checkPassBinNotInList && ((lPRR.m_b1PART_FLG & 0x18) == 0) &&
                !((lPRR.m_u2NUM_TEST == 65535) && (lPRR.m_u2SOFT_BIN == 65535)) &&
                !range_PassBinlistForRejectTest->Contains(lPRR.m_u2HARD_BIN))
        {
            keysContent.Set("PassBinNotInList", lPRR.m_u2HARD_BIN, true);
            keysContent.Set("PassBinNotInListStr", "n/a (from PRR)", true);
        }
        lTestingSite =
            GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn( stdfReader, lPRR );
    }
    else//STDF_V_3
    {
        GQTL_STDF::Stdf_PRR_V3 lPRR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lPRR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        if(lPRR.m_u2HARD_BIN == 65535)
        {
            keysContent.Set("PRRBinning",lPRR.m_u2SOFT_BIN, true);
        }
        else
        {
            keysContent.Set("PRRBinning",lPRR.m_u2HARD_BIN, true);
        }
        lTestingSite = lPRR.m_u1SITE_NUM;
    }

    if(keysContent.Get("PRRBinning").toULongLong() <= 32767)
    {
        if(keysContent.Get("TestingSite").isNull())
        {
            keysContent.Set("TestingSite", QStringList(), true);
        }
        // Keep track of total sites
        if (keysContent.Get("TestingSite").toStringList().
            indexOf(QString::number(lTestingSite)) < 0)
        {
            QStringList lList = keysContent.Get("TestingSite").toStringList();
            lList.append(QString::number(lTestingSite));
            keysContent.Set("TestingSite",lList, true);
        }

        if(keysContent.Get("TotalGoodBins").isNull())
        {
            keysContent.Set("TotalGoodBins", 0, true);
        }
        if(keysContent.Get("TotalParts").isNull())
        {
            keysContent.Set("TotalParts", 0, true);
        }

        if (globalYieldBinCheck->
            Contains(keysContent.Get("PRRBinning").toInt()) == true)
        {
            keysContent.Set("TotalGoodBins",
                            keysContent.Get("TotalGoodBins").toInt() + 1, true);
        }
        keysContent.Set("TotalParts",
                        keysContent.Get("TotalParts").toInt() + 1, true);
    }

    return true;
}

/******************************************************************************!
 * \fn LoadMRR
 ******************************************************************************/
bool
DatakeysLoader::LoadMRR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool fromInputFile  /*=false*/)
{
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_MRR_V4 lMRR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lMRR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        if(!keysContent.IsOverloaded("FinishTime"))
        {
            keysContent.Set("FinishTime", (qulonglong) lMRR.m_u4FINISH_T,
                            fromInputFile);
        }
    }
    else //STDF_V_3
    {
        GQTL_STDF::Stdf_MRR_V3 lMRR;

        // MIR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lMRR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(!keysContent.IsOverloaded("FinishTime"))
        {
            keysContent.Set("FinishTime", (qulonglong) lMRR.m_u4FINISH_T,
                            fromInputFile);
        }
    }

    return true;
}

/******************************************************************************!
 * \fn LoadSDR
 ******************************************************************************/
bool
DatakeysLoader::LoadSDR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool fromInputFile  /*=false*/)
{
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_SDR_V4 lSDR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lSDR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        // head#
        keysContent.Set("HeadNumber", QVariant(lSDR.m_u1HEAD_NUM),
                        fromInputFile);
        stdf_type_u1 wData = lSDR.m_u1SITE_CNT;
        QStringList lSites;
        while(wData)
        {
            // GCORE-5239
            // wData must be decremented before accessing to the array, otherwise we are going to read
            // memory outside the array. And index 0 won't be read
            --wData;
            lSites += QString::number(lSDR.m_ku1SITE_NUM[wData]);
        }

        keysContent.Set("SiteNumbers",
                        QVariant(lSites.join((","))), fromInputFile);

        // HAND_TYP: Handler/prober type
        keysContent.Set("ProberType", lSDR.m_cnHAND_TYP, fromInputFile);

        // HAND_ID: Handler/prober name
        keysContent.Set("ProberName", lSDR.m_cnHAND_ID, fromInputFile);

        // ignore...probe card type
        keysContent.Set("CardType",QVariant(lSDR.m_cnCARD_TYP), fromInputFile);
        // ignore...probe card name
        keysContent.Set("Card",QVariant(lSDR.m_cnCARD_ID), fromInputFile);

        // LOAD_TYP: Load board type
        keysContent.Set("LoadBoardType",lSDR.m_cnLOAD_TYP, fromInputFile);

        // LOAD_ID: Load board name
        keysContent.Set("LoadBoard",lSDR.m_cnLOAD_ID, fromInputFile);

        // DIB_TYP: Load board type
        keysContent.Set("DibType",lSDR.m_cnDIB_TYP, fromInputFile);

        keysContent.Set("DibName",lSDR.m_cnDIB_ID, fromInputFile);

        // CABL_TYP: Interface cable type
        keysContent.Set("CableType",
                        QVariant(lSDR.m_cnCABL_TYP), fromInputFile);
        // CABL_ID: Interface cable name
        keysContent.Set("Cable",QVariant(lSDR.m_cnCABL_ID), fromInputFile);
        // CONT_TYP: Handler contactor type
        keysContent.Set("ContactorType",
                        QVariant(lSDR.m_cnCONT_TYP), fromInputFile);
        // CONT_ID: Handler contactor name
        keysContent.Set("Contactor",QVariant(lSDR.m_cnCONT_ID), fromInputFile);
        // LASR_TYP: Laser type
        keysContent.Set("LaserType",
                        QVariant(lSDR.m_cnLASR_TYP), fromInputFile);
        // LASR_ID: Laser name
        keysContent.Set("Laser",QVariant(lSDR.m_cnLASR_ID), fromInputFile);

        // EXTR_TYP: Extra equipment type
        keysContent.Set("ExtraType", lSDR.m_cnEXTR_TYP, fromInputFile);

        // EXTR_ID: Extra equipment name
        keysContent.Set("ExtraName", lSDR.m_cnEXTR_ID, fromInputFile);
    }
    else //STDF_V_3
    {
        QString lDataFilePath;
        stdfReader.GetFileName(&lDataFilePath);

        errorString = QString("Error reading STDF file %1 (%2).").
            arg(lDataFilePath).
            arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
    }

    return true;
}

/******************************************************************************!
 * \fn LoadWIR
 ******************************************************************************/
bool
DatakeysLoader::LoadWIR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool fromInputFile  /*=false*/)
{
    QString lWaferID;
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_WIR_V4 lWIR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lWIR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        lWaferID = lWIR.m_cnWAFER_ID;
        keysContent.Set("WaferStartTime", (qulonglong) lWIR.m_u4START_T,
                        fromInputFile);
    }
    else //STDF_V_3
    {
        GQTL_STDF::Stdf_WIR_V3 lWIR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lWIR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        lWaferID = lWIR.m_cnWAFER_ID;
    }

    keysContent.Set("Wafer", lWaferID, fromInputFile);

    // In case of multiple wafers per file, keep a list of Wafer IDs
    QString lWaferIdList = keysContent.Get("WaferIdList").toString();
    if(!lWaferIdList.isEmpty())
    {
        lWaferIdList += ",";
    }
    lWaferIdList += lWaferID;
    keysContent.Set("WaferIdList", lWaferIdList, fromInputFile);

    return true;
}

/******************************************************************************!
 * \fn LoadWRR
 ******************************************************************************/
bool
DatakeysLoader::LoadWRR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool fromInputFile  /*=false*/)
{
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_WRR_V4 lWRR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lWRR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(!keysContent.IsOverloaded("FinishTime"))
        {
            keysContent.Set("FinishTime", (qlonglong) lWRR.m_u4FINISH_T,
                            fromInputFile);
        }
        keysContent.Set("WaferFinishTime", (qlonglong) lWRR.m_u4FINISH_T,
                        fromInputFile);
    }
    else //STDF_V_3
    {
        GQTL_STDF::Stdf_WRR_V3 lWRR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lWRR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(!keysContent.IsOverloaded("FinishTime"))
        {
            keysContent.Set("FinishTime", (qlonglong) lWRR.m_u4FINISH_T,
                            fromInputFile);
        }

    }

    return true;
}

/******************************************************************************!
 * \fn LoadWCR
 ******************************************************************************/
bool
DatakeysLoader::LoadWCR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                        QString& errorString,
                        bool fromInputFile  /*=false*/)
{
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_WCR_V4 lWCR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lWCR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }

        if(!keysContent.IsOverloaded("WaferNotch"))
        {
            keysContent.Set("WaferNotch", QVariant(QChar(lWCR.m_c1WF_FLAT)),
                            fromInputFile);
        }
        // 1463 : need these to generate Olympus map
        keysContent.Set("WaferSize", lWCR.m_r4WAFR_SIZ, fromInputFile);
        keysContent.Set("WaferUnits", lWCR.m_u1WF_UNITS, fromInputFile);
        keysContent.Set("ChipSizeX", lWCR.m_r4DIE_WID, fromInputFile);
        keysContent.Set("ChipSizeY", lWCR.m_r4DIE_HT, fromInputFile);
        keysContent.Set("WaferCenterX", lWCR.m_i2CENTER_X, fromInputFile);
        keysContent.Set("WaferCenterY", lWCR.m_i2CENTER_Y, fromInputFile);
        keysContent.Set("WaferPosX", lWCR.m_c1POS_X, fromInputFile);
        keysContent.Set("WaferPosY", lWCR.m_c1POS_Y, fromInputFile);

    }
    else //STDF_V_3
    {
        GQTL_STDF::Stdf_WCR_V3 lWCR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lWCR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        if(!keysContent.IsOverloaded("WaferNotch"))
        {
            keysContent.Set("WaferNotch", QVariant(QChar(lWCR.m_c1WF_FLAT)),
                            fromInputFile);
        }
        }

    return true;
}

/******************************************************************************!
 * \fn LoadDTR
 ******************************************************************************/
bool
DatakeysLoader::LoadDTR(int stdfVersion,
                        GQTL_STDF::StdfParse& stdfReader,
                        DatakeysContent& keysContent,
                             QString & errorString)
{
    if(stdfVersion == STDF_V_4)
    {
        GQTL_STDF::Stdf_DTR_V4 lDTR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lDTR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        unsigned int	lGrossDie;
        bool			lOK;
        if(lDTR.IsGexCommand_GrossDie(&lGrossDie, &lOK) && lOK)
        {
            keysContent.Set("GrossDie",lGrossDie, true);
        }
        else
        {
            // Check if it contains meta data
            QJsonObject lMetaData = lDTR.GetGsJson("md");
            if(lMetaData != QJsonObject())
            {
                QString lKey = lMetaData.value("KEY").toString();
                QString lValue = lMetaData.value("VALUE").toString();
                bool lConcat = false;
                QString lSep = ",";
                if(lMetaData.contains("ACTION"))
                    lConcat = lMetaData.value("ACTION").toString().contains("concat",Qt::CaseInsensitive);
                if(lMetaData.contains("SEP"))
                    lSep = lMetaData.value("SEP").toString();

                // Before to append, check if there is an existing value
                if(lConcat && !keysContent.Get(lKey).toString().isEmpty())
                {
                    lValue.prepend(QString("%1%2").arg(keysContent.Get(lKey).toString()).arg(lSep));
                }
                keysContent.Set(lKey, lValue, true);
            }
        }

        // 1463
        if (lDTR.m_cnTEXT_DAT.startsWith("/*GalaxySemiconductorJS*/") ||
            lDTR.m_cnTEXT_DAT.startsWith("/*GalaxySemiconductor*/"))
        {
            // Denso did not listen to the specs and
            // have split JS hooks in several DTR
            // Let's concatenate all JS DTR in 1 property
            QString lNewJSDTR;
            if (keysContent.property("GalaxySemiJSDTR").isValid())
            {
                lNewJSDTR=keysContent.property("GalaxySemiJSDTR").toString();
            }
            lNewJSDTR+=lDTR.m_cnTEXT_DAT;
            keysContent.setProperty("GalaxySemiJSDTR",lNewJSDTR);
        }
    }
    else //STDF_V_3
    {
        GQTL_STDF::Stdf_DTR_V3 lDTR;

        // SDR record
        if (stdfReader.
            ReadRecord(dynamic_cast<GQTL_STDF::Stdf_Record*>(&lDTR)) == false)
        {
            // Error reading STDF file
            // Convertion failed
            QString lDataFilePath;
            stdfReader.GetFileName(&lDataFilePath);
            errorString = QString("Error reading STDF file %1 (%2).").
                arg(lDataFilePath).
                arg(GGET_LASTERRORMSG(StdfParse, &stdfReader));
            return false;
        }
        unsigned int	lGrossDie;
        bool			lOK;
        if(lDTR.IsGexCommand_GrossDie(&lGrossDie, &lOK) && lOK)
        {
            keysContent.Set("GrossDie",lGrossDie, true);
        }

    }

    return true;
}

/******************************************************************************!
 * \fn LoadSqlite
 ******************************************************************************/
#define gs_data_isset(s) s != NULL && *s != '\0' && ::strcmp(s, "(null)") != 0
#define DKLOADSQLITE_CHECKERROR \
    if (gs_data_getLastError(d)) \
    { \
        errorString = QString("%1 %2").arg(d->error).arg(d->strerror); \
        return false; \
    }

bool
DatakeysLoader::LoadSqlite(struct GsData* d,
                           int splitlotId,
                           const QString& dataFilePath,
                           DatakeysContent& keysContent,
                           QString& errorString,
                           bool fromInputFile)
{
    const char* strval;

    GSLOG(SYSLOG_SEV_DEBUG, "LoadSqlite begin");

    if (d == NULL)
    {
        errorString = QString("Error reading SQLite file %1 (%2).").
            arg(dataFilePath).
            arg(gs_data_getLastStrError(d));
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "get setup_t");
    strval = gs_data_getKeyStr(d, splitlotId, "setup_t");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SetupTime",
                        (qlonglong) ::atol(strval), fromInputFile);
    }

    GSLOG(SYSLOG_SEV_DEBUG, "get start_t");
    strval = gs_data_getKeyStr(d, splitlotId, "start_t");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("StartTime",
                        (qlonglong) ::atol(strval), fromInputFile);
    }

    GSLOG(SYSLOG_SEV_DEBUG, "get finish_t");
    strval = gs_data_getKeyStr(d, splitlotId, "finish_t");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("FinishTime",
                        (qlonglong) ::atol(strval), fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "stat_num");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Station",
                        (qlonglong) ::atol(strval), fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "prod_data");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ProdData",
                        (*strval == 'P') ? true : false, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "data_provider");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("DataOrigin", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "mode_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("DataType", (QChar) *strval, fromInputFile);
        keysContent.Set("TestFlow", (QChar) *strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "rtst_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("RetestCode", (QChar) *strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "prot_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ProtectionCode", (QChar) *strval, fromInputFile);
    }

    const char* lDbVersionNb =
        gs_data_getTableKeyValue(d,
                                 "ft_gtl_info",
                                 QString("gtl_key='db_version_nb' and "
                                         "splitlot_id='%1'").
                                 arg(splitlotId).toUtf8().constData());
    if (lDbVersionNb == NULL || gs_data_getLastError(d))
    {
        lDbVersionNb =
            gs_data_getTableKeyValue(d,
                                     "global_settings",
                                     "key='db_version_nb'");
    }
    DKLOADSQLITE_CHECKERROR

    strval = gs_data_getKeyStr(d, splitlotId, "retest_phase");
    if (gs_data_getLastError(d))
    {
        if (::atoi(lDbVersionNb) == 3)
        {
            errorString =
                QString("%1 %2 (db_version_nb=%3)").
                arg(d->error).
                arg(d->strerror).
                arg(lDbVersionNb);
            return false;
        }
    }
    if (gs_data_isset(strval))
    {
        keysContent.Set("TestInsertion", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "test_insertion");
    if (gs_data_getLastError(d))
    {
        if (::atoi(lDbVersionNb) > 3)
        {
            errorString =
                QString("%1 %2 (db_version_nb=%3)").
                arg(d->error).
                arg(d->strerror).
                arg(lDbVersionNb);
            return false;
        }
    }
    if (gs_data_isset(strval))
    {
        keysContent.Set("TestInsertion", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "retest_index");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("RetestIndex", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "retest_hbins");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("RetestBinList", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "consolidation_algo");
    if (gs_data_getLastError(d))
    {
        if (::atoi(lDbVersionNb) > 3)
        {
            errorString =
                QString("%1 %2 (db_version_nb=%3)").
                arg(d->error).
                arg(d->strerror).
                arg(lDbVersionNb);
            return false;
        }
    }
    if (gs_data_isset(strval))
    {
        keysContent.Set("ConsolidationAlgo", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "burn_tim");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("BurninTime", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "cmod_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("CommandModeCode", (QChar) *strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "lot_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Lot", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "subcon_lot_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SubconLot", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "subcon_lot_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SubconLot", strval, fromInputFile);
        keysContent.Set("TrackingLot", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "part_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Product", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "tester_name");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("TesterName", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "tester_type");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("TesterType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "job_nam");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ProgramName", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "job_rev");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ProgramRevision", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "sublot_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SubLot", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "oper_nam");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Operator", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "exec_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ExecType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "exec_ver");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ExecVersion", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "test_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("TestingCode", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "tst_temp");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Temperature", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "user_txt");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("UserText", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "aux_file");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("AuxiliaryFile", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "pkg_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("PackageType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "famly_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Family", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "date_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("DateCode", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "facil_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Facility", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "floor_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Floor", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "proc_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Process", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "oper_frq");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("FrequencyStep", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "spec_nam");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SpecName", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "spec_ver");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SpecVersion", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "flow_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Flow", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "setup_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Setup", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "dsgn_rev");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("DesignRevision", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "eng_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Engineering", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "rom_cod");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("RomCode", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "serl_num");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SerialNumber", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "supr_nam");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("SupervisorName", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "handler_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ProberType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "handler_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ProberName", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "card_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("CardType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "card_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Card", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "loadboard_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("LoadBoardType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "loadboard_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("LoadBoard", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "dib_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("DibType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "dib_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("DibName", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "cable_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("CableType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "cable_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Cable", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "contactor_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ContactorType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "contactor_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Contactor", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "laser_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("LaserType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "laser_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("Laser", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "extra_typ");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ExtraType", strval, fromInputFile);
    }

    strval = gs_data_getKeyStr(d, splitlotId, "extra_id");
    DKLOADSQLITE_CHECKERROR
    if (gs_data_isset(strval))
    {
        keysContent.Set("ExtraName", strval, fromInputFile);
    }

    return true;
}

}  //END namespace QtLib
}  //END namespace GS
