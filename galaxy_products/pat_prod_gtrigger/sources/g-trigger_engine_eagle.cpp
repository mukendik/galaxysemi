/******************************************************************************!
 * \file g-trigger_engine_eagle.cpp
 * \brief Eagle: Create as many trigger files as Wafers (STDF files)
 ******************************************************************************/
#include "g-trigger_engine.h"
#include "gstdl_utils_c.h"


#include <QDir>

// Example: D20D422_1_LotSummary_ETS193746_07152013.std
#define EAGLE_SUM_REGEXP "[^_]+_(\\d+)_LotSummary_ETS(\\d+_\\d+)\\.std"

namespace GS
{
namespace GTrigger
{
/******************************************************************************!
 * \fn CreateTriggers_Eagle
 ******************************************************************************/
void GTriggerEngine::CreateTriggers_Eagle(const QString& SummaryFile)
{
    QString lSummaryFullName = mSummaryList.m_strSummaryFolder + "/" +
        SummaryFile;
    QString lTextMessage;
    CInputFile lInputFile(lSummaryFullName);

    // LOG
    lTextMessage =
        QString("Creating EAGLE triggers for %1").arg(lSummaryFullName);
    // GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(lTextMessage);

    // Example file name: X50M012_1_LotSummary_ETS160857_01312008.std

    // Extract Lot-Splitlot (X50M012_1)
    QString lFilePrefix = SummaryFile.section("_", 0, 1);

    // Compute Promis LotID (X50M012.1)
    CPromisTextFile lPromis;
    lPromis.strPromisLotID = lFilePrefix;
    lPromis.strPromisLotID.replace('_', '.');
    QString lLotID_FromFile = lPromis.strPromisLotID.section('.', 0, 0);

    // Extract split-lot from summary file (MIR.Lot)
    // eg: X50M012.1.LotSummary.ETS160857.01312008.std
    QString lLotID_FromSummaryStdf;
    QString lLotID_FromDataStdf;
    QString lProductID, lSubLotID;
    QDateTime lStartTime;

    // Get some header information from the summary file
    if (! GetStdfHeaderData(lSummaryFullName, lLotID_FromSummaryStdf,
                            lSubLotID, lProductID, lStartTime))
    {
        lTextMessage = QString("Error reading STDF header data from file %1").
            arg(lSummaryFullName);
        ParsingError(lTextMessage, &lInputFile, false);
        return;
    }

    // Check if Lot_ID from filename same as Lot_ID in file
    if (lLotID_FromFile.toLower() != lLotID_FromSummaryStdf.toLower())
    {
        lTextMessage = "LotID extracted from filename (";
        lTextMessage += lLotID_FromFile + ") differs from STDF LotID (";
        lTextMessage += lLotID_FromSummaryStdf + ")";
        ParsingError(lTextMessage, &lInputFile, false);
        return;
    }

    // Get from PROMIS file list of wafer# that belong to a given split-lot
    QString lPromisLine, lPromisFormat;
    WaferFilesList lWaferList;
    // To be set to TRUE if lot only belongs to Engineering Promis file
    bool bEngPromis = false;
    if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder,
                                   lLotID_FromSummaryStdf, lPromis,
                                   lWaferList, false,
                                   lPromisLine, lPromisFormat) == false)
    {
        // Entry not found in promis file, check in Engineering promis file
        if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder,
                                       lLotID_FromSummaryStdf, lPromis,
                                       lWaferList, true,
                                       lPromisLine, lPromisFormat) == false)
        {
            if (lPromisLine.isEmpty())
            {
                lTextMessage = "No info found in PROMIS file (Lot=";
                lTextMessage += lLotID_FromSummaryStdf + ")";
            }
            else
            {
                lTextMessage = "Error parsing line in PROMIS file (Lot=";
                lTextMessage += lLotID_FromSummaryStdf + ")";
                lTextMessage += ". Line is \"" + lPromisLine + "\".";
            }
            ParsingError(lTextMessage, &lInputFile, false);
            return;
        }
        // Track that this lot is a ENGINNERING one
        bEngPromis = true;
    }

    // Get all files belonging to the Splitlot
    QString lMoveEagleFile;
    QDir cDir;
    QStringList::Iterator itFile;
    QStringList lFileList;
    QString lDataFile;
    QString lString;
    unsigned lTotalCompositePatGtfCreated = 0;
    unsigned lTotalPatGtfCreated = 0;
    unsigned lTotalNoPatGtfCreated = 0;

    WaferFilesList::iterator lWaferIt;
    for (lWaferIt = lWaferList.begin();
         lWaferIt != lWaferList.end();
         ++lWaferIt)
    {
        // Example of filename:
        // PAT flow: Y03K123_1_16_P_ETS171357_04022008.std
        // NO_PAT flow: Y03K123_1_16_N_ETS171357_04022008.std

        // Find files matching criteria, may not exist if not tested yet
        cDir.setPath(mStdfEagleInFolder);
        cDir.setFilter(QDir::Files);

        // 1) Look for PAT flow files
        lDataFile = QString("%1_%2_P_ETS*.std").
            arg(lFilePrefix).
            arg(lWaferIt.value().WaferID());
        lFileList = cDir.entryList(QStringList(lDataFile));
        // Add all Eagle files matching criteria to list
        // (as a wafer can have been tested or partially tested several times)
        for (itFile = lFileList.begin();
             itFile != lFileList.end(); ++itFile)
        {
            lWaferIt.value().SetWaferType(GS::GTrigger::ePatWafer);
            // Add full file name to list
            lMoveEagleFile = mStdfEagleInFolder + "/" + (*itFile);
            lMoveEagleFile = QDir::cleanPath(lMoveEagleFile);
            // Extract some header information
            if (! GetStdfHeaderData(lMoveEagleFile, lLotID_FromDataStdf,
                                    lSubLotID, lProductID, lStartTime))
            {
                // Log warning message, but continue to check for other files
                lTextMessage =
                    QString("Error reading STDF header data from"
                            " file %1 (File=%2)").
                    arg(lMoveEagleFile).
                    arg(lInputFile.m_strFileName);
                LogMessage(lTextMessage, eWarning);
            }
            else
            {
                lWaferIt.value().AddDataFile(lMoveEagleFile, lStartTime);
            }
        }

        // 2) If no PAT flow files found, look for NO_PAT flow files
        if (lWaferIt.value().GetWaferType() == GS::GTrigger::eUnknownWafer)
        {
            lDataFile = QString("%1_%2_N_ETS*.std").arg(lFilePrefix).arg(
                    lWaferIt.value().WaferID());
            lFileList = cDir.entryList(QStringList(lDataFile));
            // Add all Eagle files matching criteria to list (as a wafer
            // can have been tested or partially tested several times)
            for (itFile = lFileList.begin();
                 itFile != lFileList.end(); ++itFile)
            {
                lWaferIt.value().SetWaferType(GS::GTrigger::eNoPatWafer);
                // Add full file name to list
                lMoveEagleFile = mStdfEagleInFolder + "/" + (*itFile);
                lMoveEagleFile = QDir::cleanPath(lMoveEagleFile);
                // Extract some header information
                if (! GetStdfHeaderData(lMoveEagleFile, lLotID_FromDataStdf,
                                        lSubLotID, lProductID, lStartTime))
                {
                    // Log warning message,
                    // but continue to check for other files
                    lTextMessage =
                        QString("Error reading STDF header data from"
                                " file %1 (File=%2)").
                        arg(lMoveEagleFile).
                        arg(lInputFile.m_strFileName);
                    LogMessage(lTextMessage, eWarning);
                }
                else
                {
                    lWaferIt.value().AddDataFile(lMoveEagleFile, lStartTime);
                }
            }
        }

        // Make sure files for this wafer are sorted
        lWaferIt.value().Sort();
    }

    // If some wafers have PAT flow files, read recipe,
    // and check if composite map GTF should be generated
    QString lRecipeFile;
    QString lRunName;
    QString lCompositeMapFileName;
    bool lCompositePatEnabled = false;
    bool lEngineeringRecipe = false;
    if (lWaferList.DistinctWafers(GS::GTrigger::ePatWafer, true) > 0)
    {
        // Get name of recipe file
        lRecipeFile = getRecipeFile(lPromis.strGeometryName,
                                    lRunName,
                                    lEngineeringRecipe,
                                    eFlow_Lvm_Eagle_Pat);

        // Parse recipe to check Composite PAT option
        lCompositePatEnabled = isCompositePatEnabled(lRecipeFile);
    }

    // If composite map option enabled, generate composite map
    // Use only wafers using PAT flow for generating composite map
    if (lCompositePatEnabled)
    {
        QDateTime lTimeStamp;
        extractDateTime_Eagle(lTimeStamp, SummaryFile,
                              QString(EAGLE_SUM_REGEXP));

        // 3.3: change the timestamp (swap time and date order)
        // for the GTF file name
        if (! CreateCompositeTrigger(mStdfEagleInFolder, lWaferList,
                                     lRecipeFile,
                                     lTimeStamp.toString("yyyyMMdd_hhmmss"),
                                     lCompositeMapFileName,
                                     lPromis.strGeometryName))
        {
            lTextMessage =
                QString("Could not write GTF for composite PAT (ZPAT): %1").
                arg(lCompositeMapFileName);
            ParsingError(lTextMessage, &lInputFile, false);
            return;
        }

        ++lTotalCompositePatGtfCreated;
    }

    // Display wafers found
    LogMessage(QString("%1 wafers found in PROMIS file for lot %2"
                       " (%3 PAT flow, %4 NoPAT flow, %5 Unknown flow)").
               arg(lWaferList.DistinctWafers(false)).
               arg(lPromis.strPromisLotID).
               arg(lWaferList.DistinctWafers(GS::GTrigger::ePatWafer, false)).
               arg(lWaferList.DistinctWafers(GS::GTrigger::eNoPatWafer, false)).
               arg(lWaferList.DistinctWafers(GS::GTrigger::eUnknownWafer,
                                             false)));
    LogMessage(QString("%1 files selected in folder %2 for lot %3"
                       " (%4 PAT flow, %5 NoPAT flow)").
               arg(lWaferList.DistinctFiles()).arg(mStdfEagleInFolder).
               arg(lPromis.strPromisLotID).
               arg(lWaferList.DistinctFiles(GS::GTrigger::ePatWafer)).
               arg(lWaferList.DistinctFiles(GS::GTrigger::eNoPatWafer)));

    bool bDisablePromis = false;
    bool lGalaxyRetest = false;
    for (lWaferIt = lWaferList.begin();
         lWaferIt != lWaferList.end(); ++lWaferIt)
    {
        // Iterate through all wafers and generate either 1 GTF per
        // wafer or 1 GTF per file (depending on option 'MergeTestRetest')
        QStringList lWaferFilesList;
        lWaferIt.value().DataFiles(lWaferFilesList);
        for (int lIndex = 0; lIndex < lWaferFilesList.size(); ++lIndex)
        {
            QString lDataFile = lWaferFilesList.at(lIndex);
            if (mOutputTriggerFileFormat == javascriptFormat &&
                lWaferIt.value().GetWaferType() == GS::GTrigger::ePatWafer)
            {
                lTotalPatGtfCreated +=
                    this->CreateJSTriggerFile(SummaryFile,
                                              lWaferIt.value(),
                                              lDataFile,
                                              SummaryInputFormatEagle,
                                              lPromis,
                                              lLotID_FromSummaryStdf,
                                              lRunName,
                                              lCompositeMapFileName,
                                              lRecipeFile,
                                              lGalaxyRetest,
                                              lCompositePatEnabled,
                                              bDisablePromis,
                                              bEngPromis,
                                              lPromisFormat);
            }
            else
            {
                unsigned int lGtfCreated =
                    this->CreateTriggerFile(SummaryFile,
                                            lWaferIt.value(),
                                            lDataFile,
                                            SummaryInputFormatEagle,
                                            lPromis,
                                            lLotID_FromSummaryStdf,
                                            lRunName,
                                            lCompositeMapFileName,
                                            lRecipeFile,
                                            lGalaxyRetest,
                                            lCompositePatEnabled,
                                            bDisablePromis,
                                            bEngPromis,
                                            lPromisFormat);
                if (lWaferIt.value().GetWaferType() == GS::GTrigger::ePatWafer)
                {
                    lTotalNoPatGtfCreated += lGtfCreated;
                }
                else
                {
                    lTotalPatGtfCreated += lGtfCreated;
                }
            }
        }
    }

    // If at least one wafer file detected:
    // o copy file to replication folder if any defined.
    // o delete or rename the summary STDF file
    // o ensure we delete any previous summary log file
    //   (allows ensuring it will be recreated in all cases,
    //   including when split lots involved)
    if ((lTotalPatGtfCreated + lTotalNoPatGtfCreated) > 0)
    {
        // Check if replication required
        // (if so, copy input PRN file to specified folder)
        char* ptChar = getenv("GEX_SERVER_INPUT_REPLICATION_FOLDER");
        if (ptChar != NULL)
        {
            // Copy file for replication
            QFileInfo clFileInfo(lSummaryFullName);
            QString strDestFile;
            QDir clDir;
            strDestFile = ptChar;
            strDestFile += "/" + clFileInfo.dir().dirName();
            if (! clDir.exists(strDestFile))
            {
                clDir.mkdir(strDestFile);
            }
            strDestFile += "/" + clFileInfo.fileName();
            ut_CopyFile(
                lSummaryFullName.toLatin1().constData(),
                strDestFile.toLatin1().constData(),
                UT_COPY_USETEMPNAME);
        }

        // Delete summary file if option in .INI set
        if (mDeletePrn)
        {
            // Delete summary
            cDir.remove(lSummaryFullName);
        }
        else
        {
            // Rename .PRN do .DONE...and also expand the
            // file name and move to eagle output folder
            QString strNewName =
                mStdfEagleInFolder + "/" + SummaryFile + ".done";

            // Rename
            cDir.remove(strNewName);
            cDir.rename(lSummaryFullName, strNewName);
        }

        // Delete the summary log file
        cDir.setPath(mProduction);
        QString strLotSummary =
            mProduction + "/" + lLotID_FromSummaryStdf + "-summary.log";
        cDir.remove(strLotSummary);
    }

    // Eagle summary file processed
    LogMessage(QString("EAGLE Summary file %1: created %2 composite PAT GTF,"
                       " %3 PAT flow GTF, %4 NoPAT flow GTF.").
               arg(SummaryFile).
               arg(lTotalCompositePatGtfCreated).
               arg(lTotalPatGtfCreated).
               arg(lTotalNoPatGtfCreated));
}

}
}
