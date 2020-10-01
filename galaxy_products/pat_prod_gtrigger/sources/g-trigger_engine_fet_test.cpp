/******************************************************************************!
 * \file g-trigger_engine_spektra.cpp
 * \brief Fet-Test: Create as many trigger files as Wafers (STDF files)
 ******************************************************************************/
#include "g-trigger_engine.h"
#include "gstdl_utils_c.h"


// includes from GEX
#include <converter_external_file.h>

#include <QDir>

namespace GS
{
namespace GTrigger
{
/******************************************************************************!
 * \fn CreateTriggers_FetTest
 ******************************************************************************/
void GTriggerEngine::CreateTriggers_FetTest(const QString& SummaryFile)
{
    QString lPrnFullName = mSummaryList.m_strSummaryFolder + "/" + SummaryFile;
    QString lTextMessage;

    // LOG
    lTextMessage =
        QString("Creating FETTEST triggers for %1").arg(lPrnFullName);
    // GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(lTextMessage);

    // Get product name assosiacted to this Lot
    QString lRunName;
    QString lRecipeFile = "";
    bool bGalaxyRetest;
    bool lEngineeringRecipe = false;
    // May be forced to 'true' if
    // Galaxy ReTest .PRN instructs to disable Promis Shell batch file
    bool bDisablePromis = false;
    WaferFilesList lWaferList;
    CInputFile lInputFile(lPrnFullName);
    if (getProductName(lPrnFullName, lRunName, bGalaxyRetest, bDisablePromis,
                       lEngineeringRecipe, lWaferList) == false)
    {
        // Failed to read .PRN or identify Product name in file
        lTextMessage = "Failed reading .PRN to extract product name";
        ParsingError(lTextMessage, &lInputFile, true);
        return;
    }

    // Extract root name from .PRN file name
    // (root name is common name with .DAT / .STDF data files)
    // Eg: X04INI.PRN => Root name is 'X04IN'
    // (we drop the last digit in name + extension)
    QString strRootName = SummaryFile.section(".", 0, 0);  // Remove extension

    /////////////////////////////
    QString lLotID;
    QString lPromisLotID;
    QString lSubLotID;
    QString lError;
    int     lWaferNumber;
    bool bOk;
    bOk = ConverterExternalFile::DecryptLot(strRootName, lLotID, lWaferNumber, lError);
    if (! bOk)
    {
        // Failed to read .PRN or identify Product name in file
        lTextMessage = "Failed decrypting LotID: root name=" + strRootName;
        lTextMessage+= " - "+lError;
        ParsingError(lTextMessage, &lInputFile, true);
        return;
    }
    ////////////////////////////

    // Build LotID.Wafer#
    lPromisLotID = lLotID + "." + QString::number(lWaferNumber);
    lPromisLotID = lPromisLotID.toUpper();

    // Check which datafiles (wafers) exists
    // We assume a maximum of 50 wafers per lot
    QString lPromisLine;
    QString lPromisFormat;
    QString lDataFile;
    QDir lDir;
    QTextStream hTrigger;
    QString lProductID;
    QString lStdfLot;
    QStringList lFileList;
    CPromisTextFile lPromis;
    QStringList::Iterator itFile;
    WaferFilesList::iterator lWaferIt;
    QDateTime lStartTime;
    bool lCompositePatEnabled = false;
    QString lCompositeMapFileName;
    unsigned lTotalCompositePatGtfCreated = 0;
    unsigned lTotalPatGtfCreated = 0;
    unsigned lTotalNoPatGtfCreated = 0;

    // If Standard .PRN file, need to identify the STDF files to process
    // Otherwise use the list found in Galaxy Retest .PRN
    if (bGalaxyRetest == false)
    {
        // Get list of wafer# that belong to a given split-lot
        lPromis.clear();
        if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder,
                                       lPromisLotID, lPromis, lWaferList, false,
                                       lPromisLine, lPromisFormat,
                                       GS::GTrigger::ePatWafer) == false)
        {
            // Entry not found in promis file...check in Engineering promis file
            if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder,
                                           lPromisLotID, lPromis, lWaferList,
                                           true, lPromisLine, lPromisFormat,
                                           GS::GTrigger::ePatWafer) == false)
            {
                if (lPromisLine.isEmpty())
                {
                    lTextMessage = "No info found in PROMIS file (Lot=";
                    lTextMessage += lPromisLotID + ")";
                }
                else
                {
                    lTextMessage = "Error parsing line in PROMIS file (Lot=";
                    lTextMessage += lPromisLotID + ")";
                    lTextMessage += ". Line is \"" + lPromisLine + "\".";
                }
                ParsingError(lTextMessage, &lInputFile, false);
                return;
            }
        }

        for (lWaferIt = lWaferList.begin();
             lWaferIt != lWaferList.end(); ++lWaferIt)
        {
            // Build root path to file name (lot+wafer#)
            lDataFile = QString("%1-%2.dat_*.std").
                arg(lPromis.strPromisLotID).
                arg(lWaferIt.value().WaferID());

            // Find first file matching this criteria,
            // may not exist if not tested yet
            lDir.setPath(mStdfInFolder);
            lDir.setFilter(QDir::Files);
            lFileList = lDir.entryList(QStringList(lDataFile));
            // Add all files matching criteria to list (as a wafer can
            // have been tested or partially tested several times)
            for (itFile = lFileList.begin();
                 itFile != lFileList.end(); ++itFile)
            {
                // Should always find only one file matching criteria
                lDataFile = mStdfInFolder + "/" + (*itFile);
                lDataFile = QDir::cleanPath(lDataFile);
                // Extract some header information
                if (! GetStdfHeaderData(lDataFile, lStdfLot, lSubLotID,
                                        lProductID, lStartTime))
                {
                    // Log warning message,
                    // but continue to check for other files
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

        // If some PAT files, read recipe, and check if
        // composite map GTF should be generated
        if (lWaferList.DistinctWafers(GS::GTrigger::ePatWafer, true) > 0)
        {
            // Get name of recipe file
            if (lPromisFormat == "vishay-hvm")
            {
                lRecipeFile = getRecipeFile(lProductID,
                                            lRunName,
                                            lEngineeringRecipe,
                                            eFlow_Hvm_FetTest_Pat);
            }
            else
            {
                lRecipeFile = getRecipeFile(lProductID,
                                            lRunName,
                                            lEngineeringRecipe,
                                            eFlow_Lvm_FetTest_Pat);
            }

            // Parse recipe to check Composite PAT option
            lCompositePatEnabled = isCompositePatEnabled(lRecipeFile);
        }

        // If composite map option enabled, generate composite map
        if (lCompositePatEnabled)
        {
            // CHECKME: which timestamp to use?
            QDateTime lTimeStamp = QDateTime::currentDateTime();
            bool lStatus = CreateCompositeTrigger(mStdfInFolder,
                                                  lWaferList,
                                                  lRecipeFile,
                                                  lTimeStamp.
                                                  toString("hhmmss_MMddyyyy"),
                                                  lCompositeMapFileName,
                                                  lPromis.strGeometryName);
            if (lStatus == false)
            {
                lTextMessage =
                    QString("Could not write GTF for composite PAT (ZPAT): %1").
                    arg(lCompositeMapFileName);
                ParsingError(lTextMessage, &lInputFile, false);
                return;
            }

            ++lTotalCompositePatGtfCreated;
        }
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
               arg(lWaferList.DistinctFiles()).
               arg(mStdfInFolder).arg(lPromis.strPromisLotID).
               arg(lWaferList.DistinctFiles(GS::GTrigger::ePatWafer)).
               arg(lWaferList.DistinctFiles(GS::GTrigger::eNoPatWafer)));

    // Create individual trigger files
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
                this->CreateTriggerFile(SummaryFile,
                                        lWaferIt.value(),
                                        lDataFile,
                                        SummaryInputFormatFetTest,
                                        lPromis,
                                        lLotID,
                                        lRunName,
                                        lCompositeMapFileName,
                                        lRecipeFile,
                                        bGalaxyRetest,
                                        lCompositePatEnabled,
                                        bDisablePromis,
                                        lEngPromis,
                                        lPromisFormat);
        }
    }

    // If at least one wafer file detected:
    // o copy file to replication folder if any defined.
    // o delete or rename the PRN file
    // o ensure we delete any previous summary log file
    //   (allows ensuring it will be recreated in all cases,
    //   including when split lots involved)
    if (lTotalPatGtfCreated)
    {
        // Decode again the encoded lot#
        QString lError;
        bool bOk;
        bOk = ConverterExternalFile::DecryptLot(strRootName, lPromisLotID, lWaferNumber, lError);
        if (! bOk)
        {
            // Failed to read .PRN or identify Product name in file
            lTextMessage = "Failed decrypting LotID: root name=" + strRootName;
            lTextMessage+= " - "+lError;
            ParsingError(lTextMessage, &lInputFile, true);
            return;
        }

        // Check if replication required
        // (if so, copy input PRN file to specified folder)
        char* ptChar = getenv("GEX_SERVER_INPUT_REPLICATION_FOLDER");
        if (ptChar != NULL)
        {
            // Copy file for replication
            QFileInfo clFileInfo(lPrnFullName);
            QString strDestFile;
            QDir clDir;
            strDestFile = ptChar;
            strDestFile += "/" + clFileInfo.dir().dirName();
            if (! clDir.exists(strDestFile))
            {
                clDir.mkdir(strDestFile);
            }
            strDestFile += "/" + clFileInfo.fileName();
            ut_CopyFile(lPrnFullName.toLatin1().constData(),
                        strDestFile.toLatin1().constData(),
                        UT_COPY_USETEMPNAME);
        }

        // Delete .PRN file if option in .INI set,
        // or if Galaxy ReTest .PRN file (engineering)
        if (mDeletePrn || bGalaxyRetest)
        {
            // Delete .PRN
            lDir.remove(lPrnFullName);
        }
        else
        {
            // Rename .PRN do .DONE and also expand the
            // file name to uncompressed (base 10) format
            QString strNewName = mSummaryList.m_strSummaryFolder + "/" +
                lPromisLotID + ".prn.done";

            // Delete Old .PRN before caling rename function.
            lDir.remove(strNewName);
            lDir.rename(lPrnFullName, strNewName);
        }

        // Delete the summary log file
        lDir.setPath(mProduction);
        QString strLotSummary =
            mProduction + "/" + lPromisLotID + "-summary.log";
        lDir.remove(strLotSummary);
    }

    // Report .PRN file processed
    LogMessage(QString("FETTEST PAT PRN file %1: created %2 composite PAT GTF,"
                       " %3 PAT flow GTF, %4 NoPAT flow GTF.").
               arg(SummaryFile).
               arg(lTotalCompositePatGtfCreated).
               arg(lTotalPatGtfCreated).
               arg(lTotalNoPatGtfCreated));
}

}
}
