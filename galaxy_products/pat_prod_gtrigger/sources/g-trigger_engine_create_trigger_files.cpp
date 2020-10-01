/******************************************************************************!
 * \file g-trigger_engine_create_triggers.cpp
 * \brief Create as many trigger files as Wafers (STDF files)
 ******************************************************************************/
#include "g-trigger_engine.h"


#include <QFileInfo>
#include <QDir>

namespace GS
{
namespace GTrigger
{
/******************************************************************************!
 * \fn CreateTriggerFile
 ******************************************************************************/
unsigned int
GTriggerEngine::CreateTriggerFile(const QString& lSummaryFile,
                                  const WaferFiles& lWaferFiles,
                                  const QString& lDataFile,
                                  enum SummaryInputFormat lSummaryInputFormat,
                                  CPromisTextFile& lPromis,
                                  const QString& lStdfLot,
                                  const QString& lRunName,
                                  const QString& lCompositeMapFileName,
                                  QString& lRecipeFile,
                                  bool lGalaxyRetest,
                                  bool lCompositePatEnabled,
                                  bool lDisablePromis,
                                  bool lEngPromis,
                                  QString lPromisFormat)
{
    QFileInfo lFile;
    QString lStdfInFolder;
    QString lTriggerFileName;
    QFile lTriggerFile;
    QTextStream lTriggerStream;
    QString lTextMessage;
    QString lString;
    QString lOutFolderStdf;
    QDir lDir;
    unsigned int lTotalGtfCreated = 0;

    switch (lSummaryInputFormat)
    {
    case SummaryInputFormatEagle:
        lStdfInFolder = mStdfEagleInFolder;
        lOutFolderStdf = mStdfEagleOutFolderPat;
        break;
    case SummaryInputFormatSpektra:
        lStdfInFolder = mStdfSpektraInFolder;
        lOutFolderStdf = mStdfSpektraOutFolderPat;
        break;
    default:
        lStdfInFolder = mStdfInFolder;
        lOutFolderStdf = mOutFolderStdf;
        break;
    }

    lFile.setFile(lDataFile);

    // Build GTF file name
    QString lFolder = lFile.absolutePath();
    if (lCompositePatEnabled)
    {
        if (mOutput_Pat_Composite_Maps_Subfolder.isEmpty())
        {
            lFolder = QString("%1/zpat/%2").
                arg(lStdfInFolder).
                arg(lPromis.strPromisLotID);
        }
        else
        {
            lFolder = QString("%1/%2/%3").
                arg(lStdfInFolder).
                arg(mOutput_Pat_Composite_Maps_Subfolder).
                arg(lPromis.strPromisLotID);
        }
    }

    {
        // 3.3: try to rebuild the timestamp (swap time and date order)
        // for the GTF file name
        QDateTime lTimeStamp;
        QString lFileNameRoot;
        if (lSummaryInputFormat == SummaryInputFormatEagle &&
            extractDateTime_EagleStdf(lTimeStamp, lFileNameRoot,
                                      lFile.fileName()))
        {
            lTriggerFileName = QString("%1/%2%3.gtf").
                arg(lFolder).
                arg(lFileNameRoot).
                arg(lTimeStamp.toString("yyyyMMdd_hhmmss"));
        }
        else
        {
            lTriggerFileName = QString("%1/%2.gtf").
                arg(lFolder).
                arg(lFile.completeBaseName());
        }
    }

    // Keep track of total wafers identified
    // (and trigger files generated)
    ++lTotalGtfCreated;

    if (lSummaryInputFormat == SummaryInputFormatFetTest &&
        lGalaxyRetest)
    {
        // g-pat-retest application: check which
        // PROMIS split-lot associated with this wafer
        // and extract corresponding PROMIS info
        // Get list of wafer# that belong to a given split-lot
        // First re-construct LotID from filename
        // (ie X32606.5-10.dat_2007Oct22.173944_gexmo.std)
        QString strShortFileName = lFile.completeBaseName();
        QString lPromisLotID =
            strShortFileName.section('.', 0, 0) + ".";
        QString strTemp =
            strShortFileName.section('.', 1, 1).section('-', 1, 1);
        lPromisLotID += QString::number(strTemp.toInt());
        lPromis.clear();
        WaferFilesList lDummyWaferList;
        QString lPromisLine;
        if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder,
                                       lPromisLotID, lPromis,
                                       lDummyWaferList,
                                       false, lPromisLine,
                                       lPromisFormat) == false)
        {
            // Entry not found in promis file,
            // check in Engineering promis file
            if (getPROMIS_WafersInSplitLot(mSummaryList.
                                           m_strSummaryFolder,
                                           lPromisLotID, lPromis,
                                           lDummyWaferList,
                                           true, lPromisLine,
                                           lPromisFormat) == false)
            {
                if (lPromisLine.isEmpty())
                {
                    lTextMessage = "No info found in PROMIS file (Lot=";
                    lTextMessage += lPromisLotID +
                        ") for PRN file: " + lSummaryFile;
                }
                else
                {
                    lTextMessage =
                        "Error parsing line in PROMIS file (Lot=";
                    lTextMessage += lPromisLotID +
                        ") for PRN file: " + lSummaryFile;
                    lTextMessage +=
                        ". Line is \"" + lPromisLine + "\".";
                }
                LogMessage(lTextMessage);
                return lTotalGtfCreated;  //FIXME: Not 0 ?
            }
        }
        QString lSubLotID;
        QString lProductID;
        QDateTime lStartTime;
        bool lEngineeringRecipe = false;
        GetStdfHeaderData(const_cast<QString&>(lDataFile),
                          const_cast<QString&>(lStdfLot),
                          lSubLotID,
                          lProductID,
                          lStartTime);
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
    }
    else if (lSummaryInputFormat == SummaryInputFormatSpektra)
    {
        lPromisFormat = "vishay-hvm";
    }

    // Open file (write mode)
    lTriggerFile.setFileName(lTriggerFileName);
    if (lTriggerFile.open(QIODevice::WriteOnly))
    {
        lTriggerStream.setDevice(&lTriggerFile);

        // Write trigger file contents
        lTriggerStream << "<GalaxyTrigger>" << endl;
        if (lSummaryInputFormat == SummaryInputFormatEagle &&
            lWaferFiles.GetWaferType() == GS::GTrigger::eNoPatWafer)
        {
            lTriggerStream << "Action = WAFER_EXPORT" << endl;
        }
        else
        {
            lTriggerStream << "Action = PAT" << endl;
        }

        // Display G-Trigger revision#
        lTriggerStream << "# Trigger file created by: " << mAppName <<
            endl << endl;

        // Log file enabled
        lTriggerStream << "#Create log file with summary details" <<
            endl;
        lTriggerStream << "LogFile = " + mOutFolderLog << endl;

        // STDF file path
        lTriggerStream << "#Define input file" << endl;
        lTriggerStream << "DataSource=" + lDataFile << endl;
        if (mMergeTestRetest)
        {
            // Add all files for this wafer in the same GTF
            // (files are sorted by date)
            QStringList lWaferFilesList;
            int lIndex = 0;  //FIXME see old version
            lWaferFiles.DataFiles(lWaferFilesList);
            while (++lIndex < lWaferFilesList.size())
            {
                lTriggerStream << "DataSource=" +
                    lWaferFilesList.at(lIndex) << endl;
            }
        }

        // External map file: add to GTF if an external map can be found
        this->AddOptionalSourceToGtf(lTriggerStream, lStdfLot,
                                     lWaferFiles.WaferID());

        if (lSummaryInputFormat != SummaryInputFormatEagle ||
            lWaferFiles.GetWaferType() != GS::GTrigger::eNoPatWafer)
        {
            // Composite PAT file
            if (lCompositePatEnabled)
            {
                lTriggerStream << "#Composite PAT input file" << endl;
                lTriggerStream << "CompositeFile=" +
                    lCompositeMapFileName << endl;
            }

            // Recipe
            lTriggerStream << "#Define custom recipe file" << endl;
            lTriggerStream << "Recipe=" << lRecipeFile << endl;

            // Overload STDF Start/Stop TimeStamp fields?
            if (mUpdateTimeFields)
            {
                lTriggerStream << "UpdateTimeFields=yes" << endl;
            }
            else
            {
                lTriggerStream << "UpdateTimeFields=no" << endl;
            }
        }

        // Check if gross die count available from extrenal PROMIS file
        if (lPromis.iGrossDie > 0)
        {
            lTriggerStream << "#Define Gross die count per wafer" <<
                endl;
            lTriggerStream << "GrossDie=" <<
                QString::number(lPromis.iGrossDie) << endl;
        }

        // Add Gross Die Ratio that should be used to
        // check if a wafer should be accepted
        lTriggerStream << "#Define Gross die ratio to be used to"
            " accept a wafer (parts > ratio*GrossDie)" << endl;
        lTriggerStream << "ValidWaferThreshold_GrossDieRatio=" <<
            QString::number(mValidWaferThreshold_GrossDieRatio) << endl;

        // Add flag specifying if incomplete wafers should be deleted
        lTriggerStream <<
            "#Flag specifying if incomplete wafers should be deleted" <<
            endl;
        lTriggerStream << "DeleteIncompleteWafers=";
        if (mDeleteIncompleteWafers)
        {
            lTriggerStream << "1" << endl;
        }
        else
        {
            lTriggerStream << "0" << endl;
        }

        // Wafermap options
        lTriggerStream << "#Wafermap options for inkless exports" <<
            endl;
        switch (lPromis.iFlat)
        {
        case 0:  // Down
            lTriggerStream << "Notch = Down" << endl;
            break;
        case 90:  // Left
            lTriggerStream << "Notch = Left" << endl;
            break;
        case 180:  // Up
            lTriggerStream << "Notch = Up" << endl;
            break;
        case 270:  // Right
            lTriggerStream << "Notch = Right" << endl;
            break;
        default:
            break;
        }
        lTriggerStream << "RotateWafer = Disabled" << endl;

        // Output files to create
        lTriggerStream << "#Define output format & path" << endl;
        if (lEngPromis)
        {
            lTriggerStream << "##### ENGINEERING LOT #####" << endl;
            lTriggerStream << "Output = STDF " + mOutFolderEngStdf +
                "/" + lPromis.strGeometryName + "/stdf" << endl;
            lTriggerStream << "Output = KLA_SINF " + mOutFolderInf <<
                endl;
            // Disable the call to the promis SHELL
            lDisablePromis = true;
        }
        else
        {
            lFolder = mOutFolderInf;
            if (mAddSiteLocationToInfOutputDir)
            {
                lFolder += "/" + lPromis.strSiteLocation;
                if (! lDir.exists(lFolder) &&
                    ! lDir.mkdir(lFolder))
                {
                    lTextMessage =
                        "Couldn't create SINF output folder (" +
                        lFolder;
                    lTextMessage += "). Normal folder (" +
                        mOutFolderInf +
                        ") will be used in trigger file ";
                    lTextMessage += lTriggerFileName;
                    LogMessage(lTextMessage);
                    lFolder = mOutFolderInf;
                }
            }
            lTriggerStream << "Output = STDF " +
                lOutFolderStdf + "/" +
                lPromis.strGeometryName + "/stdf" << endl;
            lTriggerStream << "Output = KLA_SINF " + lFolder << endl;
        }

        // Delete input STDF file
        lTriggerStream << "DeleteDataSource=no" << endl;

        // Define Shell (if configured within g-trigger.ini)
        if (lDisablePromis == false)
        {
            if (lWaferFiles.GetWaferType() == GS::GTrigger::eNoPatWafer)
            {
                if (lPromisFormat == "vishay-hvm" &&
                    ! mShell_Fet_NoPAT_HVM.isEmpty())
                {
                    lTriggerStream <<
                        "    # Define Post-WAFER_EXPORT shell command" <<
                        endl;
                    lTriggerStream << "    Shell = " <<
                        mShell_Fet_NoPAT_HVM << endl;
                }
                else if (! mShell_Fet_NoPAT.isEmpty())
                {
                    lTriggerStream <<
                        "    # Define Post-WAFER_EXPORT shell command" <<
                        endl;
                    lTriggerStream << "    Shell = " <<
                        mShell_Fet_NoPAT << endl;
                }
            }
            else
            {
                if (lPromisFormat == "vishay-hvm" &&
                    ! mShell_HVM.isEmpty())
                {
                    lTriggerStream <<
                        "    # Define Post-PAT shell command" << endl;
                    lTriggerStream << "    Shell = " <<
                        mShell_HVM << endl;
                }
                else if (! mShell.isEmpty())
                {
                    lTriggerStream <<
                        "    # Define Post-PAT shell command" << endl;
                    lTriggerStream << "    Shell = " <<
                        mShell << endl;
                }
            }
        }

        if (lSummaryInputFormat == SummaryInputFormatEagle)
        {
            // Overload Product with Geometry name
            lTriggerStream <<
                "#Overload Product name (with Geometry)" << endl;
            lTriggerStream << "Product = " +
                lPromis.strGeometryName << endl;
        }

        lTriggerStream << "</GalaxyTrigger>" << endl;
        lTriggerFile.close();

        // Create output folders
        if (QFile::exists(lRecipeFile) ||
            (lSummaryInputFormat == SummaryInputFormatEagle &&
             lWaferFiles.GetWaferType() == GS::GTrigger::ePatWafer))
        {
            // Ensure output folders are created
            // before .GTF file is processed
            // INF folder
            lString = mOutFolderInf;
            lDir.mkdir(lString);
            // LOG folder
            if (lSummaryInputFormat != SummaryInputFormatEagle ||
                lWaferFiles.GetWaferType() != GS::GTrigger::eNoPatWafer)
            {
                lString = mOutFolderLog;
                lDir.mkdir(lString);
            }
            // STDF folder
            lString = lOutFolderStdf;
            lDir.mkdir(lOutFolderStdf);
            lString += "/" + lPromis.strGeometryName;
            lDir.mkdir(lString);
            lString += "/stdf";
            lDir.mkdir(lString);
            // STDF ENG folder
            if (lEngPromis)
            {
                lString = mOutFolderEngStdf;
                lDir.mkdir(lString);
                lString += "/" + lPromis.strGeometryName;
                lDir.mkdir(lString);
                lString += "/stdf";
                lDir.mkdir(lString);
            }
        }
    }
    else
    {
        LogMessage("Failed creating trigger file: " + lTriggerFileName,
                   eError);
    }

    return lTotalGtfCreated;
}

/******************************************************************************!
 * \fn CreateTriggerFile
 ******************************************************************************/
unsigned int
GTriggerEngine::CreateJSTriggerFile(const QString& lSummaryFile,
                                    const WaferFiles& lWaferFiles,
                                    const QString& lDataFile,
                                    enum SummaryInputFormat lSummaryInputFormat,
                                    CPromisTextFile& lPromis,
                                    const QString& lStdfLot,
                                    const QString& lRunName,
                                    const QString& lCompositeMapFileName,
                                    QString& lRecipeFile,
                                    bool lGalaxyRetest,
                                    bool lCompositePatEnabled,
                                    bool lDisablePromis,
                                    bool lEngPromis,
                                    QString lPromisFormat)
{
    QFileInfo lFile;
    QString lStdfInFolder;
    QString lTriggerFileName;
    QFile lTriggerFile;
    QTextStream lTriggerStream;
    QString lTextMessage;
    QString lString;
    QString lOutFolderStdf;
    QDir lDir;
    unsigned int lTotalJsCreated = 0;

    switch (lSummaryInputFormat)
    {
    case SummaryInputFormatEagle:
        lStdfInFolder = mStdfEagleInFolder;
        lOutFolderStdf = mStdfEagleOutFolderPat;
        break;
    case SummaryInputFormatSpektra:
        lStdfInFolder = mStdfSpektraInFolder;
        lOutFolderStdf = mStdfSpektraOutFolderPat;
        break;
    default:
        lStdfInFolder = mStdfInFolder;
        lOutFolderStdf = mOutFolderStdf;
        break;
    }

    lFile.setFile(lDataFile);

    // Build JS file name
    QString lFolder = lFile.absolutePath();
    if (lCompositePatEnabled)
    {
        if (mOutput_Pat_Composite_Maps_Subfolder.isEmpty())
        {
            lFolder = QString("%1/zpat/%2").
                arg(lStdfInFolder).
                arg(lPromis.strPromisLotID);
        }
        else
        {
            lFolder = QString("%1/%2/%3").
                arg(lStdfInFolder).
                arg(mOutput_Pat_Composite_Maps_Subfolder).
                arg(lPromis.strPromisLotID);
        }
    }

    {
        // 3.3: try to rebuild the timestamp (swap time and date order)
        // for the JS file name
        QDateTime lTimeStamp;
        QString lFileNameRoot;
        if (lSummaryInputFormat == SummaryInputFormatEagle &&
            extractDateTime_EagleStdf(lTimeStamp, lFileNameRoot,
                                      lFile.fileName()))
        {
            lTriggerFileName = QString("%1/%2%3.js").
                arg(lFolder).
                arg(lFileNameRoot).
                arg(lTimeStamp.toString("yyyyMMdd_hhmmss"));
        }
        else
        {
            lTriggerFileName = QString("%1/%2.js").
                arg(lFolder).
                arg(lFile.completeBaseName());
        }
    }

    // Keep track of total wafers identified
    // (and trigger files generated)
    ++lTotalJsCreated;

    if (lSummaryInputFormat == SummaryInputFormatFetTest &&
        lGalaxyRetest)
    {
        // g-pat-retest application: check which
        // PROMIS split-lot associated with this wafer
        // and extract corresponding PROMIS info
        // Get list of wafer# that belong to a given split-lot
        // First re-construct LotID from filename
        // (ie X32606.5-10.dat_2007Oct22.173944_gexmo.std)
        QString strShortFileName = lFile.completeBaseName();
        QString lPromisLotID =
            strShortFileName.section('.', 0, 0) + ".";
        QString strTemp =
            strShortFileName.section('.', 1, 1).section('-', 1, 1);
        lPromisLotID += QString::number(strTemp.toInt());
        lPromis.clear();
        WaferFilesList lDummyWaferList;
        QString lPromisLine;
        if (getPROMIS_WafersInSplitLot(mSummaryList.m_strSummaryFolder,
                                       lPromisLotID, lPromis,
                                       lDummyWaferList,
                                       false, lPromisLine,
                                       lPromisFormat) == false)
        {
            // Entry not found in promis file,
            // check in Engineering promis file
            if (getPROMIS_WafersInSplitLot(mSummaryList.
                                           m_strSummaryFolder,
                                           lPromisLotID, lPromis,
                                           lDummyWaferList,
                                           true, lPromisLine,
                                           lPromisFormat) == false)
            {
                if (lPromisLine.isEmpty())
                {
                    lTextMessage = "No info found in PROMIS file (Lot=";
                    lTextMessage += lPromisLotID +
                        ") for PRN file: " + lSummaryFile;
                }
                else
                {
                    lTextMessage =
                        "Error parsing line in PROMIS file (Lot=";
                    lTextMessage += lPromisLotID +
                        ") for PRN file: " + lSummaryFile;
                    lTextMessage +=
                        ". Line is \"" + lPromisLine + "\".";
                }
                LogMessage(lTextMessage);
                return lTotalJsCreated;  //FIXME: Not 0 ?
            }
        }
        QString lSubLotID;
        QString lProductID;
        QDateTime lStartTime;
        bool lEngineeringRecipe = false;
        GetStdfHeaderData(const_cast<QString&>(lDataFile),
                          const_cast<QString&>(lStdfLot),
                          lSubLotID,
                          lProductID,
                          lStartTime);
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

    }
    else if (lSummaryInputFormat == SummaryInputFormatSpektra)
    {
        lPromisFormat = "vishay-hvm";
    }

    // Open file (write mode)
    lTriggerFile.setFileName(lTriggerFileName);
    if (lTriggerFile.open(QIODevice::WriteOnly))
    {
        lTriggerStream.setDevice(&lTriggerFile);

        // Display G-Trigger revision#
        lTriggerStream << "// Trigger file created by: " << mAppName <<
            endl << endl;
        lTriggerStream << "var trigVar = {" << endl;

        if (lSummaryInputFormat == SummaryInputFormatEagle &&
            lWaferFiles.GetWaferType() == GS::GTrigger::eNoPatWafer)
        {
            lTriggerStream << "    testFlow: 'Eagle_NOPAT'," << endl;
        }
        else
        {
            lTriggerStream << "    testFlow: 'Eagle_PAT'," << endl;
        }


        // Log file enabled
        lTriggerStream << "    // Create log file with summary details" <<
            endl;
        lTriggerStream << "    LogFile: '" <<
            mOutFolderLog.replace('\\','/') << "'," << endl;

        // STDF file path
        lTriggerStream << "    // Define input file" << endl;
        lTriggerStream << "    DataSource: '" <<
            lDataFile.simplified().replace('\\','/') << "'," << endl;
        if (mMergeTestRetest)
        {
            // Add all files for this wafer in the same JS
            // (files are sorted by date)
            QStringList lWaferFilesList;
            int lIndex = 0;  //FIXME see old version
            lWaferFiles.DataFiles(lWaferFilesList);
            while (++lIndex < lWaferFilesList.size())
            {
                lTriggerStream << "    DataSource: '" <<
                    lWaferFilesList.at(lIndex).simplified().replace('\\','/') <<
                    "'," << endl;
            }
        }

        // External map file: add to JS if an external map can be found
        this->AddOptionalSourceToJs(lTriggerStream, lStdfLot,
                                    lWaferFiles.WaferID());

        if (lSummaryInputFormat != SummaryInputFormatEagle ||
            lWaferFiles.GetWaferType() != GS::GTrigger::eNoPatWafer)
        {
            // Composite PAT file
            if (lCompositePatEnabled)
            {
                lTriggerStream << "    // Composite PAT input file" << endl;
                lTriggerStream << "    CompositeFile: '" +
                    lCompositeMapFileName.simplified().replace('\\','/') <<
                    "'," << endl;
            }

            // Recipe
            lTriggerStream << "    // Define custom recipe file" << endl;
            lTriggerStream << "    Recipe: '" <<
                lRecipeFile.replace('\\','/') << "'," << endl;

            // Overload STDF Start/Stop TimeStamp fields?
            if (mUpdateTimeFields)
            {
                lTriggerStream << "    UpdateTimeFields: 'yes'," << endl;
            }
            else
            {
                lTriggerStream << "    UpdateTimeFields: 'no'," << endl;
            }
        }

        // Check if gross die count available from extrenal PROMIS file
        if (lPromis.iGrossDie > 0)
        {
            lTriggerStream << "    // Define Gross die count per wafer" <<
                endl;
            lTriggerStream << "    GrossDie: '" <<
                QString::number(lPromis.iGrossDie) << "'," << endl;
        }

        // Add Gross Die Ratio that should be used to
        // check if a wafer should be accepted
        lTriggerStream << "    // Define Gross die ratio to be used to"
            " accept a wafer (parts > ratio*GrossDie)" << endl;
        lTriggerStream << "    GrossDieRatio: '" <<
            QString::number(mValidWaferThreshold_GrossDieRatio) << "'," << endl;

        // Add flag specifying if incomplete wafers should be deleted
        lTriggerStream <<
            "    // Flag specifying if incomplete wafers should be deleted" <<
            endl;
        if (mDeleteIncompleteWafers)
        {
            lTriggerStream << "    DeleteIncompleteWafers: '1'," << endl;
        }
        else
        {
            lTriggerStream << "    DeleteIncompleteWafers: '0'," << endl;
        }

        // Wafermap options
        lTriggerStream << "    // Wafermap options for inkless exports" <<
            endl;
        switch (lPromis.iFlat)
        {
        case 0:
            lTriggerStream << "    Notch: 'Down'," << endl;
            break;
        case 90:
            lTriggerStream << "    Notch: 'Left'," << endl;
            break;
        case 180:
            lTriggerStream << "    Notch: 'Up'," << endl;
            break;
        case 270:
            lTriggerStream << "    Notch: 'Right'," << endl;
            break;
        default:
            break;
        }
        lTriggerStream << "    RotateWafer: 'Disabled'," << endl;

        // Output files to create
        lTriggerStream << "    // Define output format & path" << endl;
        if (lEngPromis)
        {
            lTriggerStream << "    // ENGINEERING LOT" << endl;
            lTriggerStream << "    STDFOutput: '" <<
                mOutFolderEngStdf.replace('\\','/') << "'," << endl;
            lTriggerStream << "    KLA_SINFOutput: '" <<
                mOutFolderInf.replace('\\','/') << "'," << endl;
            // Disable the call to the promis SHELL
            lDisablePromis = true;
        }
        else
        {
            lFolder = mOutFolderInf;
            if (mAddSiteLocationToInfOutputDir)
            {
                lFolder += "/" + lPromis.strSiteLocation;
                if (! lDir.exists(lFolder) &&
                    ! lDir.mkdir(lFolder))
                {
                    lTextMessage =
                        "Couldn't create SINF output folder (" + lFolder +
                        "). Normal folder (" + mOutFolderInf +
                        ") will be used in trigger file " + lTriggerFileName;
                    LogMessage(lTextMessage);
                    lFolder = mOutFolderInf;
                }
            }
            lTriggerStream << "    STDFOutput: '" <<
                lOutFolderStdf.replace('\\','/') << "'," << endl;
            lTriggerStream << "    KLA_SINFOutput: '" <<
                lFolder.replace('\\','/') << "'," << endl;
        }

        // Delete input STDF file
        lTriggerStream << "    DeleteDataSource: 'no'," << endl;

        //FIXME:
        lTriggerStream << "    lotID: '" << lStdfLot << "'," << endl;
        lTriggerStream << "    waferID: '" << lWaferFiles.WaferID() << "'," <<
            endl;

        // Define Shell (if configured within g-trigger.ini)
        if (lDisablePromis == false)
        {
            if (lWaferFiles.GetWaferType() == GS::GTrigger::eNoPatWafer)
            {
                if (lPromisFormat == "vishay-hvm" &&
                    ! mShell_Fet_NoPAT_HVM.isEmpty())
                {
                    lTriggerStream <<
                        "    // Define Post-WAFER_EXPORT shell command" <<
                        endl;
                    lTriggerStream << "    Shell: '" <<
                        mShell_Fet_NoPAT_HVM.replace('\\','/') << "'," << endl;
                }
                else if (! mShell_Fet_NoPAT.isEmpty())
                {
                    lTriggerStream <<
                        "    // Define Post-WAFER_EXPORT shell command" <<
                        endl;
                    lTriggerStream << "    Shell: '" <<
                        mShell_Fet_NoPAT.replace('\\','/') << "'," << endl;
                }
            }
            else
            {
                if (lPromisFormat == "vishay-hvm" &&
                    ! mShell_HVM.isEmpty())
                {
                    lTriggerStream <<
                        "    // Define Post-PAT shell command" << endl;
                    lTriggerStream << "    Shell: '" <<
                        mShell_HVM.replace('\\','/') << "'," << endl;
                }
                else if (! mShell.isEmpty())
                {
                    lTriggerStream <<
                        "    // Define Post-PAT shell command" << endl;
                    lTriggerStream << "    Shell: '" <<
                        mShell.replace('\\','/') << "'," << endl;
                }
            }
        }

        if (lSummaryInputFormat == SummaryInputFormatEagle)
        {
            // Overload Product with Geometry name
            lTriggerStream <<
                "    // Overload Product name (with Geometry)" << endl;
            lTriggerStream << "    Product: '" <<
                lPromis.strGeometryName << "'," << endl;
            lTriggerStream << "    mergeNum: 0" << endl;
        }

        lTriggerStream <<
            "};" <<
            endl <<
            endl;

        if (lSummaryInputFormat == SummaryInputFormatEagle)
        {
            lTriggerStream <<
                "// The double quote do not pass in the Exec shell script"
                           << endl <<
                "// Change double quote to % to easily"
                " identify where the double"
                           << endl <<
                "// quotes need to be in order to parse"
                " the JSON.strignify results"
                           << endl <<
                "var keys = Object.keys(trigVar);"
                           << endl <<
                "jsonStr = '{';"
                           << endl <<
                "for (var i = 0; i < keys.length; i++) {"
                           << endl <<
                "    jsonStr += '\\%' + keys[i] + '\\%\\:\\%' +"
                " trigVar[keys[i]] + '\\%,';"
                           << endl <<
                "}"
                           << endl <<
                "jsonStr = jsonStr.substring(0, jsonStr.lastIndexOf(','));"
                           << endl <<
                "jsonStr += '}';"
                           << endl << endl <<
                "// Call The js MapMerge Script to merge the additional maps"
                           << endl <<
                "if (trigVar.testFlow === 'Eagle_PAT') {"
                           << endl <<
                "    var nodeCall = 'node ';"
                           << endl <<
                "    var jsModule = '" <<
                mMapMergeScript.replace('\\','/') << " ';"
                           << endl <<
                "    GSEngine.ExecShellScript(nodeCall + jsModule + jsonStr);"
                           << endl <<
                "}"
                           << endl;
        }

        lTriggerFile.close();

        // Create output folders
        if (QFile::exists(lRecipeFile) ||
            (lSummaryInputFormat == SummaryInputFormatEagle &&
             lWaferFiles.GetWaferType() == GS::GTrigger::ePatWafer))
        {
            // Ensure output folders are created
            // before .JS file is processed
            // INF folder
            lString = mOutFolderInf;
            lDir.mkdir(lString);
            // LOG folder
            if (lSummaryInputFormat != SummaryInputFormatEagle ||
                lWaferFiles.GetWaferType() != GS::GTrigger::eNoPatWafer)
            {
                lString = mOutFolderLog;
                lDir.mkdir(lString);
            }
            // STDF folder
            lString = lOutFolderStdf;
            lDir.mkdir(lOutFolderStdf);
            lString += "/" + lPromis.strGeometryName;
            lDir.mkdir(lString);
            lString += "/stdf";
            lDir.mkdir(lString);
            // STDF ENG folder
            if (lEngPromis)
            {
                lString = mOutFolderEngStdf;
                lDir.mkdir(lString);
                lString += "/" + lPromis.strGeometryName;
                lDir.mkdir(lString);
                lString += "/stdf";
                lDir.mkdir(lString);
            }
        }
    }
    else
    {
        LogMessage("Failed creating trigger file: " + lTriggerFileName,
                   eError);
    }

    return lTotalJsCreated;
}

}
}
