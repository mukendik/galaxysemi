/******************************************************************************!
 * \file g-trigger_engine_spektra.cpp
 * \brief Spektra: Create as many trigger files as Wafers (STDF files)
 ******************************************************************************/
#include "g-trigger_engine.h"
#include "gstdl_utils_c.h"


#include <QDir>

namespace GS
{
namespace GTrigger
{
/******************************************************************************!
 * \fn CreateTriggers_Spektra
 ******************************************************************************/
void GTriggerEngine::CreateTriggers_Spektra(const QString& SpektraCntFile)
{
    QString lCntFullName = mSummaryList.m_strSummaryFolder + "/" +
        SpektraCntFile;
    QString lTextMessage;

    // LOG
    lTextMessage =
        QString("Creating SPEKTRA triggers for %1").arg(lCntFullName);
    // GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(lTextMessage);

    // Example file name: B15K209.1_W24.CNT.CSV

    // Extract Lot-Splitlot (B15K209.1)
    CPromisTextFile lPromis;
    lPromis.strPromisLotID = SpektraCntFile.section("_", 0, 0);;

    // Extract split-lot from CNT file
    QString lLotID;
    CInputFile lInputFile(lCntFullName);
    if (! GetCntData(SpektraCntFile, &lInputFile, lLotID))
    {
        return;
    }

    // Check if Lot_ID from filename same as Lot_ID in file
    if (lPromis.strPromisLotID.toLower() != lLotID.toLower())
    {
        lTextMessage = "LotID extracted from filename (";
        lTextMessage += lPromis.strPromisLotID + ") differs from STDF LotID (";
        lTextMessage += lLotID + ")";
        ParsingError(lTextMessage, &lInputFile, false);
        return;
    }

    // Get list of wafer# that belong to a given split-lot
    QString lPromisLine, lPromisFormat;
    WaferFilesList lWaferList;
    if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder, lLotID,
                                   lPromis, lWaferList, false, lPromisLine,
                                   lPromisFormat,
                                   GS::GTrigger::ePatWafer) == false)
    {
        // Entry not found in promis file...check in Engineering promis file
        if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder, lLotID,
                                       lPromis, lWaferList, true, lPromisLine,
                                       lPromisFormat,
                                       GS::GTrigger::ePatWafer) == false)
        {
            if (lPromisLine.isEmpty())
            {
                lTextMessage = "No info found in PROMIS file (Lot=";
                lTextMessage += lLotID + ")";
            }
            else
            {
                lTextMessage = "Error parsing line in PROMIS file (Lot=";
                lTextMessage += lLotID + ")";
                lTextMessage += ". Line is \"" + lPromisLine + "\".";
            }
            ParsingError(lTextMessage, &lInputFile, false);
            return;
        }
    }

    // Get all files belonging to the Splitlot
    QDir lDir;
    QStringList::Iterator itFile;
    QStringList lFileList;
    QString lDataFile;
    QDateTime lStartTime;
    QString lProductID;
    QString lStdfLot;
    QString lSubLotID;
    unsigned lTotalCompositePatGtfCreated = 0;
    unsigned lTotalPatGtfCreated = 0;
    unsigned lTotalNoPatGtfCreated = 0;

    WaferFilesList::iterator lWaferIt;
    for (lWaferIt = lWaferList.begin();
         lWaferIt != lWaferList.end(); ++lWaferIt)
    {
        // Example of filename: B15K209.1_W01.Dta.CSV_2012Feb15.015902_gexmo.std
        // Build root path to file name (lot+wafer#)
        lDataFile = QString("%1_W%2.dta.csv_*.std").
            arg(lPromis.strPromisLotID).
            arg(lWaferIt.value().WaferID());

        // Find first file matching this criteria,
        // may not exist if not tested yet
        lDir.setPath(mStdfSpektraInFolder);
        lDir.setFilter(QDir::Files);
        lFileList = lDir.entryList(QStringList(lDataFile));
        // Add all files matching criteria to list
        // (as a wafer can have been tested or partially tested several times)
        for (itFile = lFileList.begin();
             itFile != lFileList.end(); ++itFile)
        {
            // Should always find only one file matching criteria!
            lDataFile = mStdfSpektraInFolder + "/" + (*itFile);
            lDataFile = QDir::cleanPath(lDataFile);
            // Extract some header information
            if (! GetStdfHeaderData(lDataFile, lStdfLot,
                                    lSubLotID, lProductID, lStartTime))
            {
                // Log warning message, but continue to check for other files
                lTextMessage = QString("Error reading STDF header data"
                                       " from file %1 (File=%2)").
                    arg(lDataFile).
                    arg(lInputFile.m_strFileName);
                LogMessage(lTextMessage, eWarning);
            }
            else
            {
                lWaferIt.value().AddDataFile(lDataFile, lStartTime);
            }
        }

        // Make sure files for this wafer are sorted
        lWaferIt.value().Sort();
    }

    // If some PAT files, read recipe, and check
    // if composite map GTF should be generated
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
                                    eFlow_Hvm_Spektra_Pat);

        // Parse recipe to check Composite PAT option
        lCompositePatEnabled = isCompositePatEnabled(lRecipeFile);
    }

    // If composite map option enabled, generate composite map
    if (lCompositePatEnabled)
    {
        QDateTime lTimeStamp = QDateTime::currentDateTime();

        if (! CreateCompositeTrigger(mStdfSpektraInFolder, lWaferList,
                                     lRecipeFile,
                                     lTimeStamp.toString("hhmmss_MMddyyyy"),
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
    LogMessage(QString("%1 wafers found in PROMIS file for"
                       " lot %2 (%3 PAT flow, %4 NoPAT flow, %5 Unknown flow)").
               arg(lWaferList.DistinctWafers(false)).
               arg(lPromis.strPromisLotID).
               arg(lWaferList.DistinctWafers(GS::GTrigger::ePatWafer, false)).
               arg(lWaferList.DistinctWafers(GS::GTrigger::eNoPatWafer, false)).
               arg(lWaferList.DistinctWafers(GS::GTrigger::eUnknownWafer,
                                             false)));
    LogMessage(QString("%1 files selected in folder %2 for"
                       " lot %3 (%4 PAT flow, %5 NoPAT flow)").
               arg(lWaferList.DistinctFiles()).
               arg(mStdfSpektraInFolder).
               arg(lPromis.strPromisLotID).
               arg(lWaferList.DistinctFiles(GS::GTrigger::ePatWafer)).
               arg(lWaferList.DistinctFiles(GS::GTrigger::eNoPatWafer)));

    // Create individual trigger files
    bool lGalaxyRetest = false;
    bool lDisablePromis = false;
    bool lEngPromis = false;
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
            lTotalPatGtfCreated =
                this->CreateTriggerFile(SpektraCntFile,
                                        lWaferIt.value(),
                                        lDataFile,
                                        SummaryInputFormatSpektra,
                                        lPromis,
                                        lLotID,
                                        lRunName,
                                        lCompositeMapFileName,
                                        lRecipeFile,
                                        lGalaxyRetest,
                                        lCompositePatEnabled,
                                        lDisablePromis,
                                        lEngPromis,
                                        lPromisFormat);
        }
    }

    // If at least one wafer file detected:
    // o copy file to replication folder if any defined.
    // o delete or rename the CNT file
    // o ensure we delete any previous summary log file
    //   (allows ensuring it will be recreated in all cases,
    //   including when split lots involved)
    if (lTotalPatGtfCreated)
    {
        // Check if replication required
        // (if so, copy input CNT file to specified folder)
        char* ptChar = getenv("GEX_SERVER_INPUT_REPLICATION_FOLDER");
        if (ptChar != NULL)
        {
            // Copy file for replication
            QFileInfo clFileInfo(lCntFullName);
            QString strDestFile;
            strDestFile = ptChar;
            strDestFile += "/" + clFileInfo.dir().dirName();
            if (! lDir.exists(strDestFile))
            {
                lDir.mkdir(strDestFile);
            }
            strDestFile += "/" + clFileInfo.fileName();
            ut_CopyFile(lCntFullName.toLatin1().constData(),
                        strDestFile.toLatin1().constData(),
                        UT_COPY_USETEMPNAME);
        }

        // Delete summary file if option in .INI set
        if (mDeletePrn)
        {
            // Delete summary
            lDir.remove(lCntFullName);
        }
        else
        {
            // Rename .CNT.CSV do .DONE...and also expand
            // the file name and move to output folder
            QString strNewName = lCntFullName + ".done";

            // Rename
            lDir.remove(strNewName);
            lDir.rename(lCntFullName, strNewName);
        }

        // Delete the summary log file.
        lDir.setPath(mProduction);
        QString strLotSummary = mProduction + "/" + lLotID + "-summary.log";
        lDir.remove(strLotSummary);
    }

    // Spektra .CNT.CSV file processed
    LogMessage(QString("Spektra PAT CNT file %1: created %2 composite PAT GTF,"
                       " %3 PAT flow GTF, %4 NoPAT flow GTF.").
               arg(SpektraCntFile).
               arg(lTotalCompositePatGtfCreated).
               arg(lTotalPatGtfCreated).
               arg(lTotalNoPatGtfCreated));
}

}
}
