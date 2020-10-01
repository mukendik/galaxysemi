#include "ft_pat_report_unit_private.h"
#include "ft_pat_report_unit.h"
#include "pat_recipe_io.h"
#include "gex_report.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "cpart_info.h"
#include "gqtl_log.h"
#include "gex_report_table.h"
#include "gex_pat_constants_extern.h"

#include <set>
#include <QSqlDatabase>
#include <QSharedPointer>

extern bool ComparePatResultNum(CPatDefinition* test1, CPatDefinition* test2);

// Sort by descending Severity score
bool ComparePatResultOutliers(CPatDefinition* test1, CPatDefinition* test2)
{
    return (test1->m_TotalFailures > test2->m_TotalFailures);
}

namespace GS
{
namespace Gex
{

FTPATReportUnitPrivate::FTPATReportUnitPrivate(FTPATReportUnit *parent)
    : mParent(parent), mPartsTested(0), mPartsRetested(0), mSplitReport(false),
      mSyncData(false)
{
    mOutputFormat = mParent->mGexReport->GetOption("output", "format").toString();
}

FTPATReportUnitPrivate::~FTPATReportUnitPrivate()
{
    qDeleteAll(mOutliers);
    mOutliers.clear();
}

const QString &FTPATReportUnitPrivate::GetErrorMessage() const
{
    return mErrorMessage;
}

bool FTPATReportUnitPrivate::Init()
{
    mErrorMessage.clear();
    mWarningLogs.clear();
    mDPATTestLimits.clear();
    mSPATTestLimits.clear();
    mDPATOutliers.clear();
    mPartsTested    = 0;
    mPartsRetested  = 0;
    mSyncData       = false;

    qDeleteAll(mOutliers);
    mOutliers.clear();

    QString lTraceabilityFile = mParent->mGexReport->getReportOptions()->GetPATTraceabilityFile();
    PATDbTraceabilityAbstract * lDbTraceability = PATDbTraceabilityAbstract::CreateDbTraceability(mParent->mGexReport,
                                                                                                  lTraceabilityFile,
                                                                                                  mErrorMessage);

    if (lDbTraceability)
    {
        CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                         NULL : mParent->mGexReport->getGroupsList().first();
        CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
        if(lFile == NULL)
        {
            mErrorMessage = "No datasets found.";
            return false;
        }

        // Retrieve the number of splitlot for the initial test
        int lSplitlotCount = 0;

        if (lDbTraceability->QuerySplitLotCount(lSplitlotCount,
                                                QString(lFile->getMirDatas().szPartType),
                                                QString(lFile->getMirDatas().szLot),
                                                QString(lFile->getMirDatas().szSubLot), 0) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        if (lSplitlotCount < lGroup->pFilesList.count())
        {
            mErrorMessage = "Failed to generate the report, there is too many ";
            mErrorMessage += "input datafiles comparing to splitlot count in the traceability file";
            return false;
        }

        // Allow to navigate in Interactive mode since we know we have all initial
        // Test data synchronized
        if (lSplitlotCount == lGroup->pFilesList.count())
            mSyncData = true;
        else
            mWarningLogs.append("Not all the data for Initial Test was provided via input files.");

        // Extracts the total parts tested and retested corresponding to the product, lot and sublot
        if (lDbTraceability->QueryTotalParts(QString(lFile->getMirDatas().szPartType),
                                             QString(lFile->getMirDatas().szLot),
                                             QString(lFile->getMirDatas().szSubLot),
                                             mPartsTested, mPartsRetested) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Extracts the recipe ID corresponding to the product, lot and sublot
        int lRecipeID   = -1;
        if (lDbTraceability->QueryRecipeID(QString(lFile->getMirDatas().szPartType),
                                           QString(lFile->getMirDatas().szLot),
                                           QString(lFile->getMirDatas().szSubLot), lRecipeID) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Extracts the recipe content used during the PAT processing
        QString lRecipeContent;
        if (lDbTraceability->QueryRecipeContent(lRecipeID, mRecipeName, lRecipeContent) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Loads the recipe content into the recipe instance
        QTextStream lRecipeStream(&lRecipeContent, QIODevice::ReadOnly);
        QSharedPointer<PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(lRecipeStream));
        if (lRecipeIO.isNull() || (lRecipeIO->Read(lRecipeContent.toLatin1(), mRecipe) == false))
        {
            mErrorMessage = QString("Failed to read recipe contents from PAT traceability DB %1")
                            .arg(lTraceabilityFile);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }

        // Extracts the outliers list and a detailled count
        if (lDbTraceability->QueryOutliers(mOutliers, mDPATOutliers, mWarningLogs) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Extracts the DPAT test limits
        if (lDbTraceability->QueryDPATTestLimits(mDPATTestLimits) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Extracts the SPAT test limits
        if (lDbTraceability->QuerySPATTestLimits(mSPATTestLimits) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Extracts the rolling limits
        if (lDbTraceability->QueryRollingLimits(mWarningLogs) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }

        // Extracts the warnings log
        if (lDbTraceability->QueryWarningsLog(mWarningLogs) == false)
        {
            mErrorMessage = lDbTraceability->GetErrorMessage();
            return false;
        }
    }
    else
        return false;

    if (mOutputFormat == "HTML")
    {
        mSplitReport = true;
        mReportPagesName.insert("OUTLIERS",     "advanced_fail.htm");
        mReportPagesName.insert("SPAT_LIMITS",  "advanced_spat.htm");
        mReportPagesName.insert("DPAT_RESULTS", "advanced_dpat_results.htm");
        mReportPagesName.insert("DPAT_LIMITS",  "advanced_dpat_limits.htm");
        mReportPagesName.insert("SUMMARY",      "advanced_summary.htm");
        mReportPagesName.insert("LOG",          "advanced_log.htm");
    }

    return true;
}

bool FTPATReportUnitPrivate::CheckForNewPage(const QString &lPageName, const QString& lBookmark,
                                             const QString& lSectionTitle)
{
    if(mSplitReport && lPageName.isEmpty() == false)
    {
        if(mParent->mGexReport->getReportFile() == NULL)
        {
            mErrorMessage = "Output report file missing.";

            GSLOG(SYSLOG_SEV_ERROR, "Unable to retrieve current report file.");

            return false;
        }

        // Close current page
        WriteReportLine("</body>");
        WriteReportLine("</html>");

        fclose(mParent->mGexReport->getReportFile());

        // Open new HTML page
        QString lPath = mParent->mGexReport->getReportOptions()->strReportDirectory +
                        QDir::separator() + "pages" + QDir::separator() + lPageName;

        mParent->mGexReport->setReportFile(fopen(lPath.toLatin1().constData(),"w"));

        if (mParent->mGexReport->getReportFile())
        {
            mParent->mGexReport->setAdvancedReportFile(mParent->mGexReport->getReportFile());

            // Write page break (ignored if not writing a flat HTML document)
            mParent->mGexReport->WriteHeaderHTML(mParent->mGexReport->getReportFile(), "#000000");
        }
        else
        {
            mErrorMessage = "Failed to create new report file.";

            GSLOG(SYSLOG_SEV_ERROR, "Unable to create a new report file.");

            return false;
        }
    }

    // Write the HTML section title
    if (mOutputFormat != "CSV")
        mParent->mGexReport->WriteHtmlSectionTitle(mParent->mGexReport->getReportFile(),
                                                   lBookmark, lSectionTitle);
    else
    {
        // For CSV output, add a empty line before the section title
        if (mOutputFormat == "CSV")
            WriteReportEmptyLine();
        WriteReportLine("", lSectionTitle);
    }

    return true;
}

bool FTPATReportUnitPrivate::CreateHeaderPage()
{
    CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                     NULL : mParent->mGexReport->getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    if(lFile == NULL)
    {
        mErrorMessage = "No datasets found";
        return false;
    }

    QString lProduct = QString(lFile->getMirDatas().szPartType).trimmed();
    QString lLotID   = QString(lFile->getMirDatas().szLot).trimmed();
    QString lSubLot  = QString(lFile->getMirDatas().szSubLot).trimmed();

    QString lDecorationKey      = "<td width=\"30%%\" bgcolor=\"#CCECFF\"><b>%1</b></td>";
    QString lDecorationValue    = "<td width=\"70%%\" bgcolor=\"#F8F8F8\">%1</td>";

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    // Global information
    WriteReportLine("<table border=\"0\" width=\"98%%\" cellspacing=\"0\">");
    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "RecipeFile");
    WriteTableCell(lDecorationValue, mRecipeName, true);
    WriteReportLine("</tr>");

    if(lProduct.isEmpty()==false)
    {
        WriteReportLine("<tr>");
        WriteTableCell(lDecorationKey, "Product");
        WriteTableCell(lDecorationValue, lProduct, true);
        WriteReportLine("</tr>");
    }

    if(lLotID.isEmpty()==false)
    {
        WriteReportLine("<tr>");
        WriteTableCell(lDecorationKey, "Lot");
        WriteTableCell(lDecorationValue, lLotID, true);
        WriteReportLine("</tr>");
    }

    if(lSubLot.isEmpty()==false)
    {
        WriteReportLine("<tr>");
        WriteTableCell(lDecorationKey, "SubLot");
        WriteTableCell(lDecorationValue, lSubLot, true);
        WriteReportLine("</tr>");
    }

    // Recipe information
    // #Baseline
    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "Total parts in baseline");
    WriteTableCell(lDecorationValue, QString::number(mRecipe.GetOptions().mFT_BaseLine), true);
    WriteReportLine("</tr>");

    // #Subsequent Baseline
    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "Total parts in subsequent baseline(per site)");
    WriteTableCell(lDecorationValue, QString::number(mRecipe.GetOptions().mFT_SubsequentBL), true);
    WriteReportLine("</tr>");

    // #Maximum outlier in baseline
    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "Maximum outliers allowed in base line");
    if (mRecipe.GetOptions().mFT_BaseLineMaxOutliers >= 0)
        WriteTableCell(lDecorationValue,
                       QString::number(mRecipe.GetOptions().mFT_BaseLineMaxOutliers), true);
    else
        WriteTableCell(lDecorationValue, "Disabled", true);
    WriteReportLine("</tr>");

    // #Minimun parts per site
    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "Minimum samples per site");
    WriteTableCell(lDecorationValue, QString::number(mRecipe.GetOptions().mFT_MinSamplesPerSite), true);
    WriteReportLine("</tr>");

    // #Tuning type
    if (mRecipe.GetOptions().mFT_TuningIsEnabled && (mRecipe.GetOptions().mFT_Tuning > 0))
    {
        // #Tuning size
        WriteReportLine("<tr>");
        WriteTableCell(lDecorationKey, "Tuning samples");
        WriteTableCell(lDecorationValue, QString::number(mRecipe.GetOptions().mFT_TuningSamples), true);
        WriteReportLine("</tr>");

        WriteReportLine("<tr>");
        WriteTableCell(lDecorationKey, "Tuning frequency");
        if (mRecipe.GetOptions().mFT_TuningType == FT_PAT_TUNING_EVERY_N_OUTLIERS)
            WriteTableCell(lDecorationValue,
                           QString("Every %1 outliers").arg(mRecipe.GetOptions().mFT_Tuning),
                           true);
        else if (mRecipe.GetOptions().mFT_TuningType == FT_PAT_TUNING_EVERY_N_PARTS)
            WriteTableCell(lDecorationValue,
                           QString("Every %1 parts").arg(mRecipe.GetOptions().mFT_Tuning),
                           true);
        WriteReportLine("</tr>");
    }

    // Total parts tested
    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "Total parts tested");
    WriteTableCell(lDecorationValue, QString::number(mPartsTested), true);
    WriteReportLine("</tr>");

    if (mPartsRetested > 0)
    {
        WriteReportLine("<tr>");
        WriteTableCell(lDecorationKey, "Total parts retested");
        WriteTableCell(lDecorationValue, QString::number(mPartsRetested), true);
        WriteReportLine("</tr>");
    }

    // Parts failing PAT
    double lYieldLoss = (100.0 * mOutliers.count()) / (double)mPartsTested;

    if(lYieldLoss > 100)
        lYieldLoss = 100;

    WriteReportLine("<tr>");
    WriteTableCell(lDecorationKey, "Parts failing PAT (including retest)");
    WriteTableCell(lDecorationValue,
                   QString("%1 (%2%)").arg(mOutliers.count()).arg(lYieldLoss, 0, 'g', 2),
                   true);
    WriteReportLine("</tr>");
    WriteReportLine("</table>");

    WriteReportLine("<br><br>");
    WriteReportLine("<b>Sections created:</b>");
    WriteReportLine("<ul>");
    WriteReportLine(QString("<li><a href=\"%1#part_fails\">Parts failing PAT limits</a></li>")
                    .arg(mReportPagesName.value("OUTLIERS")));

    if(mRecipe.GetOptions().bReport_SPAT_Limits)
    {
        WriteReportLine(QString("<li><a href=\"%1#all_stat_pat_limits\">Static-PAT Test Limits</a></li>")
                        .arg(mReportPagesName.value("SPAT_LIMITS")));
    }

    if(mRecipe.GetOptions().bReport_DPAT_Limits_Outliers ||
       mRecipe.GetOptions().bReport_DPAT_Limits_NoOutliers)
    {
        WriteReportLine(QString("<li><a href=\"%1#all_dyn_pat_results\">Dynamic-PAT Results</a></li>")
                        .arg(mReportPagesName.value("DPAT_RESULTS")));

        WriteReportLine(QString("<li><a href=\"%1#all_dyn_pat_limits\">Dynamic-PAT Test Limits</a></li>")
                        .arg(mReportPagesName.value("DPAT_LIMITS")));
    }

    WriteReportLine(QString("<li><a href=\"%1#all_tests_dyn_fails\">Summary: Tests failing PAT limits</a></li>")
                    .arg(mReportPagesName.value("SUMMARY")));
    WriteReportLine("</ul>");
    WriteReportLine("<br>");

    if(mWarningLogs.count() == 0)
        WriteReportLine(QString("<a href=\"%1#warning_log\"><b>Warning log</b>: Empty</a><br><br>")
                        .arg(mReportPagesName.value("LOG")));
    else
    {
        if(mWarningLogs.count() == 1)
            WriteReportLine(QString("<a href=\"%1#warning_log\"><b>Warning log</b>: 1 line</a><br><br>")
                            .arg(mReportPagesName.value("LOG")));
        else
            WriteReportLine(QString("<a href=\"%1#warning_log\"><b>Warning log</b>: %2 lines</a><br><br>")
                            .arg(mReportPagesName.value("LOG")).arg(mWarningLogs.count()));
    }

    // Insert a page break for flat html files
    mParent->mGexReport->WritePageBreak();

    return true;
}

bool FTPATReportUnitPrivate::CreatePartsFailingPATLimitsPage()
{
    CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                     NULL : mParent->mGexReport->getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if(lFile == NULL)
    {
        mErrorMessage = "No datasets found";
        return false;
    }

    if (CheckForNewPage(mReportPagesName.value("OUTLIERS"), "part_fails",
                        "Parts failing PAT limits") == false)
        return false;

    // Write backward link
    WriteBackwardLink();

    int lLineCount      = 0;
    int lMaxLine        = 15;
    int lLastRetestIdx  = -1;

    CPatOutlierPart * lOutlierPart = NULL;
    for (int lIdx = 0; lIdx < mOutliers.count(); ++lIdx)
    {
        lOutlierPart = mOutliers.at(lIdx);

        if (lLastRetestIdx != lOutlierPart->mRetestIndex)
        {
            // close table & goto next page
            WriteReportLine("</table>");
            mParent->mGexReport->WritePageBreak();

            // Reset line counter in page (so will reopen table if needed)
            lLineCount = 0;

            // Number of lines in table (page12 and following) is 15
            lMaxLine = 20;

            // write a header
            if (lOutlierPart->mRetestIndex == 0)
                WriteReportLine("<h2 align=\"left\"><font color=\"#006699\">%1</font></h2>", "Initial Test");
            else
                WriteReportLine("<h2 align=\"left\"><font color=\"#006699\">%1</font></h2>", QString("Retest# %1").arg(lOutlierPart->mRetestIndex));

            // Keep last retest index found
            lLastRetestIdx = lOutlierPart->mRetestIndex;
        }

        if (lLineCount == 0)
            BuildHeaderPartsFailingPATLimits();

        ++lLineCount;

        WriteReportLine("<tr>");
        WriteTableCell("<td width=\"10%%\" align=\"left\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>",
                       QString(lFile->getMirDatas().szLot).trimmed().toLatin1().constData());

        QString lPartID;
        if (mOutputFormat == "HTML" && mSyncData)
        {
            // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
            int lGroupID = mParent->mGexReport->getGroupForSite(lOutlierPart->iSite);
            if(lGroupID < 0)
                lGroupID = 0;

            // Regular html page: Build Link to SoftBin wafermap
            lPartID = "<a href=\"#_gex_drill--drill_3d=wafer_sbin";
            lPartID += "--g=" + QString::number(lGroupID);		// group#
            lPartID += "--f=0";		// file#
            lPartID += "--Test=0";	// test#: not used
            lPartID += "--Pinmap=0";		// pinmap#: not used
            lPartID += "--DieX=" + QString::number(lOutlierPart->iDieX);	// DieX position
            lPartID += "--DieY=" + QString::number(lOutlierPart->iDieY);	// DieY position
            lPartID += "\">";
            lPartID += lOutlierPart->strPartID;
            lPartID += "</a>";
        }
        else
            lPartID += lOutlierPart->strPartID;

        WriteTableCell("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>",
                       lPartID.toLatin1().constData());
        WriteTableCell("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>",
                       QString::number(lOutlierPart->iPatHBin));
        WriteTableCell("<td width=\"5%%\" align=\"center\" bgcolor=\"#F8F8F8\" valign=\"top\">%1</td>",
                       QString::number(lOutlierPart->iSite));

        QList<CPatFailingTest>::iterator            itPart;
        CPatDefinition *                            lPATDefinition = NULL;
        CPatFailingTest                             lFailTest;
        QString                                     lFailureType;
        QString                                     lHyperLink;
        CTest *                                     lTestCell   = NULL;
        double                                      lValue      = GEX_C_DOUBLE_NAN;
        QString                                     lDecoration("<td width=\"50%%\" align=\"left\" bgcolor=\"#F8F8F8\">");

        for (itPart = lOutlierPart->cOutlierList.begin();
             itPart != lOutlierPart->cOutlierList.end(); ++itPart)
        {
            lFailTest = *itPart;

            lPATDefinition = mRecipe.FindUniVariateRule(lFailTest.mTestNumber, lFailTest.mPinIndex,
                                                        lFailTest.mTestName);

            if (lPATDefinition == NULL)
            {
                mErrorMessage = "No PAT definitions found for test ";
                mErrorMessage += TestNumberLabel(lFailTest.mTestNumber, lFailTest.mPinIndex);
                mErrorMessage += " " + lFailTest.mTestName;

                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());

                return false;
            }

            if (lFailTest.mFailureMode == 0)
            {
                lFailureType = "Static";

                if(lFailTest.mFailureDirection < 0)
                    lPATDefinition->m_lStaticFailuresLow++;
                else
                    lPATDefinition->m_lStaticFailuresHigh++;
            }
            else
            {
                lFailureType = "Dynamic";

                // Update fail count of the test for this testing site
                if(lPATDefinition->m_DynamicFailCountPerSite.find(lOutlierPart->iSite) == lPATDefinition->m_DynamicFailCountPerSite.end())
                    lPATDefinition->m_DynamicFailCountPerSite[lOutlierPart->iSite] = 1;
                else
                    lPATDefinition->m_DynamicFailCountPerSite[lOutlierPart->iSite] = lPATDefinition->m_DynamicFailCountPerSite[lOutlierPart->iSite] +1;

                if(lFailTest.mFailureDirection < 0)
                    lPATDefinition->m_lDynamicFailuresLow[lPATDefinition->m_iOutlierLimitsSet]++;
                else
                    lPATDefinition->m_lDynamicFailuresHigh[lPATDefinition->m_iOutlierLimitsSet]++;
            }

            if(lFailTest.mFailureDirection < 0)
                lFailureType += "-";	// PAT Failure by being under the PAT low limit
            else
                lFailureType += "+";	// PAT Failure by being over the PAT high limit

            lFailureType += " ( ";

            // Build hyperlink for wafermap interactive
            if (mOutputFormat == "HTML" && mSyncData)
            {
                // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
                int lGroupID = mParent->mGexReport->getGroupForSite(lOutlierPart->iSite);
                if(lGroupID < 0)
                    lGroupID = 0;

                // Wafer map link
                lFailureType	+= "<a href=\"#_gex_drill--drill_3d=wafer_param_range";
                lFailureType	+= "--g=" + QString::number(lGroupID);		// group#
                lFailureType	+= "--f=0";		// file#
                lFailureType	+= "--Test=";		// test#
                lFailureType	+= QString::number(lFailTest.mTestNumber);
                lFailureType	+= "--Pinmap=-1";	// pin#
                lFailureType	+= QString::number(lFailTest.mPinIndex);
                lFailureType	+= "--DieX=" + QString::number(lOutlierPart->iDieX);	// DieX position
                lFailureType	+= "--DieY=" + QString::number(lOutlierPart->iDieY);	// DieY position
                lFailureType	+= "\">";
                lFailureType	+= QString::number(lFailTest.mValue);
                lFailureType    += "</a> ";

            }
            else
                lFailureType    += QString::number(lFailTest.mValue) + " ";

            // Returns pointer to correct cell. If cell doesn't exist ; its created. Test# mapping enabled
            if(lFile->FindTestCell(lFailTest.mTestNumber, lFailTest.mPinIndex,
                                   &lTestCell, true, false, lFailTest.mTestName) == 1)
                lFailureType += lTestCell->szTestUnits;

            lFailureType += " )";

            lValue = GEX_C_DOUBLE_NAN;

            if(lFailTest.mFailureDirection < 0)
            {
                // '-' outlier: PAT Failure by being under the PAT low limit
                if(lTestCell && (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                {
                    // PAT limit - OriginalTestLimit
                    lValue = lPATDefinition->m_lfLowLimit - lFailTest.mLowLimit;
                    if(lValue)
                        lValue = fabs((lFailTest.mValue - lPATDefinition->m_lfLowLimit) / lValue);
                }
            }
            else
            {
                // '+' outlier : PAT Failure by being over the PAT high limit
                if(lTestCell && (lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    // PAT limit - OriginalTestLimit
                    lValue = lPATDefinition->m_lfHighLimit - lFailTest.mHighLimit;
                    if(lValue)
                        lValue = fabs((lFailTest.mValue - lPATDefinition->m_lfHighLimit) / lValue);
                }
            }

            lFailureType += " - Limit Proximity: ";
            if(lValue != GEX_C_DOUBLE_NAN)
                lFailureType += QString::number(100*lValue, 'f', 2)+ "%";
            else
                lFailureType += "n/a";

            if (mOutputFormat == "HTML")
            {
                int lGroupID = mParent->mGexReport->getGroupForSite(lOutlierPart->iSite);
                if(lGroupID < 0)
                    lGroupID = 0;

                // Regular html page
                lHyperLink = "href=\"#_gex_drill--drill_chart=adv_trend--data=";
                lHyperLink += TestNumberLabel(lFailTest.mTestNumber, lFailTest.mPinIndex);

                if (lOutlierPart->lRunID != -1 && mSyncData)
                {
                    lHyperLink += "--marker=" + QString::number(lGroupID) + " ";
                    lHyperLink += QString::number(lOutlierPart->lRunID) + " ";
                    lHyperLink += QString::number(GEX_WAFMAP_INVALID_COORD) + " ";
                    lHyperLink += QString::number(GEX_WAFMAP_INVALID_COORD) + " ";
                    lHyperLink += QString::number(lFailTest.mValue) + " Outlier";
                }
                lHyperLink += "\"";
            }
            else
            {
                // Flat html to endup in one file
                lHyperLink = "href=\"#HistoT";
                lHyperLink += TestNumberLabel(lFailTest.mTestNumber, lFailTest.mPinIndex);

                lHyperLink += "\"";
            }

            lDecoration += "<a " + lHyperLink + "> ";
            lDecoration += "<b>T%1</b></a> %2 &nbsp;  &nbsp; %3 &nbsp; &nbsp;<br>";

            WriteTableCell(lDecoration,
                           QStringList() << TestNumberLabel(lFailTest.mTestNumber, lFailTest.mPinIndex) <<
                           mParent->mGexReport->buildDisplayName(lFailTest.mTestName) <<
                           lFailureType);

            lPATDefinition->m_TotalFailures++;
        }

        WriteTableCell("</td>", "", true);
        WriteReportLine("</tr>");

        // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
        if (mOutputFormat == "PPT" && ((lLineCount % lMaxLine) == 0))
        {
            // close table & goto next page
            WriteReportLine("</table>");
            mParent->mGexReport->WritePageBreak();

            // Reset line counter in page (so will reopen table if needed)
            lLineCount = 0;

            // Number of lines in table (page 2 and following) is 20
            lMaxLine = 20;
        }
    }

    if (lLineCount)
        WriteReportLine("</table><br>");

    // Insert a page break for flat html files
    mParent->mGexReport->WritePageBreak();

    return true;
}

bool FTPATReportUnitPrivate::CreateSPATTestsLimitsPage()
{
    CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                     NULL : mParent->mGexReport->getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if(lFile == NULL)
    {
        mErrorMessage = "No datasets found";
        return false;
    }

    if (CheckForNewPage(mReportPagesName.value("SPAT_LIMITS"), "all_stat_pat_limits",
                        "Static-PAT Test Limits") == false)
        return false;

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    // Write backward link
    WriteBackwardLink();

    QList<CPatDefinition*>                      lPatDefinition;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    bool                                        lIgnoreTest;

    for(itPATDefinifion = mRecipe.GetUniVariateRules().begin();
        itPATDefinifion != mRecipe.GetUniVariateRules().end(); ++itPATDefinifion)
    {
        // Only include tests that are StaticPAT enabled
        lIgnoreTest = false;

        if ((*itPATDefinifion)->m_lFailStaticBin == -1)
            lIgnoreTest = true;

        if(lIgnoreTest == false)
            lPatDefinition.append(*itPATDefinifion);
    }

    // Sort results in descending order
    qSort(lPatDefinition.begin(), lPatDefinition.end(), ComparePatResultNum);

    if(lPatDefinition.count() > 0)
    {
        // Open table
        QString lTableHeader;

        lTableHeader = "<table border=\"1\" cellspacing=\"1\" width=\"98%%\" style=\"font-size: ";
        lTableHeader += QString::number(mParent->mGexReport->iHthmSmallFontSizePixels);
        lTableHeader += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">";

        // write header
        WriteReportLine(lTableHeader, "");
        WriteReportLine("<tr>");
        WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "Test#");
        WriteTableCell("<td width=\"33%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "Test name");
        WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                       QStringList("SPAT") << "Fails");
        WriteTableCell("<td width=\"5%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                       QStringList("Shape") << "(without outliers)");
        WriteTableCell("<td width=\"7%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "Rule");
        WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "N");
        WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "T");
        WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                       QStringList("Original") << "Low Limit");
        WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                       QStringList("Original") << "High Limit");
        WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                       QStringList("Static PAT") << "Low Limit");
        WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                       QStringList("Static PAT") << "High Limit" , true);
        WriteReportLine("</tr>");
    }
    else
    {
        WriteReportLine("<br>");
        WriteReportLine("%1", "No Static-PAT rule defined for any test.");
        WriteReportLine("<br>");
    }

    if(lPatDefinition.count() > 0)
    {
        // Review all Static-PAT tests definitions
        CTest *                         lTestCell   = NULL;
        CPatDefinition *                lPatDef     = NULL;
        QList<PATTestLimits>::iterator  itTestLimit;
        PATTestLimits                   lTestLimit;
        QString                         lDecoration;

        // set of tesNumer, pinIndex
        std::set< std::pair<int, int> >                       lDisplayTestNumberOnce;

        for (itTestLimit = mSPATTestLimits.begin(); itTestLimit != mSPATTestLimits.end(); ++itTestLimit)
        {
            lTestLimit  = *itTestLimit;
            lPatDef     = FindUnivariateRule(lPatDefinition, lTestLimit);

            if (lPatDef == NULL)
                continue;

            // here only the test must be displayed. To avoid to have several times the same test displayed because
            // of multisite, we use a set to check if the pair <testNumeber, pinIndex> has already been  displayed.
            if(lDisplayTestNumberOnce.insert(
                        std::make_pair(lPatDef->m_lTestNumber, lPatDef->mPinIndex)).second == false)
                continue;

            // Get pointer to test cell with data
            if(lFile->FindTestCell(lPatDef->m_lTestNumber, lPatDef->mPinIndex,
                                   &lTestCell, true, false, lPatDef->m_strTestName) != 1)
                continue;

            WriteReportLine("<tr>");
            WriteTableCell("<td width=\"6%%\" bgcolor=\"#F8F8F8\"><b>%1</b></td>",
                           TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex));
            WriteTableCell("<td width=\"21%%\" bgcolor=\"#F8F8F8\">%1</td>",
                           lTestLimit.mTestName);

            // SPAT fail count
            if(lPatDef->m_lStaticFailuresLow || lPatDef->m_lStaticFailuresHigh)
                lDecoration = "<td align=\"center\" width=\"5%%\" bgcolor=\"#FF0000\">%1</td>";
            else
                lDecoration = "<td align=\"center\" width=\"5%%\" bgcolor=\"#FFFFCC\">%1</td>";

            WriteTableCell(lDecoration,
                           QString::number(lPatDef->m_lStaticFailuresLow + lPatDef->m_lStaticFailuresHigh));
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                           lTestLimit.mDistributionShape);

            switch(lPatDef->m_SPATRuleType)
            {
                case GEX_TPAT_SPAT_ALGO_SIGMA:
                    // SPAT Relax rule: N*Sigma or custom limits
                    WriteTableCell("<td width=\"8%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   gexSpatRuleSetItemsGUI[lPatDef->m_SPATRuleType]);
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   QString::number(lPatDef->m_lfSpatOutlierNFactor, 'g', 3));
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   "-");
                    break;

                case  GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA:
                    // SPAT AEC rule: 6*RobustSigma
                    WriteTableCell("<td width=\"8%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   gexSpatRuleSetItemsGUI[lPatDef->m_SPATRuleType]);
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   "-");
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   "-");
                    break;

                case GEX_TPAT_SPAT_ALGO_NEWLIMITS:
                    WriteTableCell("<td width=\"8%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   gexSpatRuleSetItemsGUI[lPatDef->m_SPATRuleType]);
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   QString::number(lPatDef->m_lfSpatOutlierNFactor, 'g', 3));
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   QString::number(lPatDef->m_lfSpatOutlierTFactor, 'g', 3));
                    break;

                case GEX_TPAT_SPAT_ALGO_RANGE:
                    // Range: Only report N factor, T factor not relevant.
                    WriteTableCell("<td width=\"8%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   gexSpatRuleSetItemsGUI[lPatDef->m_SPATRuleType]);
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   QString::number(lPatDef->m_lfSpatOutlierNFactor, 'g', 3));
                    WriteTableCell("<td align=\"center\" width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>",
                                   "-");
                    break;

                case GEX_TPAT_SPAT_ALGO_IGNORE:
                default:
                    break;
            }

            // Original program Low Limit
            if((lPatDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
            {
                WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                               QStringList(QString::number(lPatDef->m_lfLowLimit)) << lPatDef->m_strUnits);
            }
            else
                WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1</td>", "n/a");

            // Original program: High Limit
            if((lPatDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
            {
                WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                               QStringList(QString::number(lPatDef->m_lfHighLimit)) << lPatDef->m_strUnits);
            }
            else
                WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1</td>", "n/a");

            // Static-PAT limits
            WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                           QStringList(QString::number(lTestLimit.mPATLL)) << lPatDef->m_strUnits);
            WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                           QStringList(QString::number(lTestLimit.mPATHL)) << lPatDef->m_strUnits,
                           true);
            WriteReportLine("</tr>");
        }

        WriteReportLine("</table>");
    }

    return true;
}

bool FTPATReportUnitPrivate::CreateDPATResultsPage()
{
    CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                     NULL : mParent->mGexReport->getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if(lFile == NULL)
    {
        mErrorMessage = "No datasets found";
        return false;
    }

    if (CheckForNewPage(mReportPagesName.value("DPAT_RESULTS"), "all_dyn_pat_results",
                        "Dynamic-PAT Results") == false)
        return false;

    mParent->mGexReport->SetPowerPointSlideName("Outliers: Dynamic-PAT Results");

    // Write backward link
    WriteBackwardLink();

    QList<CPatDefinition*>                      lPatDefinition;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    bool                                        lIgnoreTest;

    for(itPATDefinifion = mRecipe.GetUniVariateRules().begin();
        itPATDefinifion != mRecipe.GetUniVariateRules().end(); ++itPATDefinifion)
    {
        // Only include tests that are Dynamic-PAT enabled
        lIgnoreTest = false;
        if ((*itPATDefinifion)->m_lFailDynamicBin == -1 )
            lIgnoreTest = true;
        if((*itPATDefinifion)->mOutlierRule == GEX_TPAT_RULETYPE_IGNOREID)
            lIgnoreTest = true;

        // If outlier, check if we have to report outliers...
        if(((*itPATDefinifion)->m_DynamicFailCountPerSite.count() > 0) &&
           (mRecipe.GetOptions().bReport_DPAT_Limits_Outliers == false))
            lIgnoreTest = true;

        // If not outlier, check if we have to report tests without outlier...
        if(((*itPATDefinifion)->m_DynamicFailCountPerSite.count() == 0) &&
           (mRecipe.GetOptions().bReport_DPAT_Limits_NoOutliers == false))
            lIgnoreTest = true;

        // See if ignore this test...
        if(!lIgnoreTest)
            lPatDefinition.append(*itPATDefinifion);
    }

    // Sort results in descending order
    qSort(lPatDefinition.begin(), lPatDefinition.end(), ComparePatResultNum);

    // For csv only
    WriteReportEmptyLine();

    if (lPatDefinition.count() > 0)
    {
        QString lTableHeader;

        lTableHeader = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: ";
        lTableHeader += QString::number(mParent->mGexReport->iHthmSmallFontSizePixels);
        lTableHeader += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">";

        // For CSV output, add a empty line before the table
        if (mOutputFormat == "CSV")
            WriteReportEmptyLine();

        WriteReportLine(lTableHeader);

        WriteReportLine("<tr>");
        WriteTableCell("<td width=\"20%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "Test#");
        WriteTableCell("<td width=\"50%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>", "Test name");
        WriteTableCell("<td align=\"center\" width=\"15%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>",
                        "Site#");
        WriteTableCell("<td align=\"center\" width=\"15%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                        QStringList("DPAT") << "Fails", true);
        WriteReportLine("</tr>");
    }
    else
    {
        WriteReportLine("No Dynamic-PAT rule defined for any test.");
    }

    if (lPatDefinition.count() > 0)
    {
        CTest *                         lTestCell   = NULL;
        CPatDefinition *                lPatDef     = NULL;
        QList<PATTestLimits>::iterator  itTestLimit;
        PATTestLimits                   lTestLimit;
        QString                         lTableCell;
        QString                         lTestName;
        int                             lTestNumber = -1;
        int                             lPinIndex   = -1;
        int                             lSite       = -1;
        bool                            lTestMatch  = false;

        for (itTestLimit = mDPATTestLimits.begin(); itTestLimit != mDPATTestLimits.end(); ++itTestLimit)
        {
            lTestLimit  = *itTestLimit;
            lTestMatch  = IsTestMatch(lTestNumber, lPinIndex, lTestName, lTestLimit);

            // Test has changed, retrieve the corresponding univariate rule
            if (lTestMatch == false)
            {
                lPatDef     = FindUnivariateRule(lPatDefinition, lTestLimit);

                if (lPatDef == NULL)
                    continue;

                // Get pointer to test cell with data
                if(lFile->FindTestCell(lPatDef->m_lTestNumber, lPatDef->mPinIndex,
                                       &lTestCell, true, false, lPatDef->m_strTestName) != 1)
                    continue;
            }

            if (lTestMatch == false || lSite != lTestLimit.mSite)
            {
                WriteReportLine("<tr>");

                // Same test, only the site has changed. Do not repeat test# and test name
                if (lTestMatch == false)
                {
                    lTableCell = "<td width=\"20%%\" bgcolor=\"#F8F8F8\"><b><a ";

                    // Write Test# + hyperlink
                    if (mOutputFormat == "HTML")
                    {
                        // Regular html page
                        lTableCell += "href=\"#_gex_drill--drill_chart=adv_trend--data=";
                        lTableCell += TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex);

                        lTableCell += "\"";
                    }
                    else
                    {
                        // Flat html to endup in one file
                        lTableCell += "href=\"#HistoT";
                        lTableCell += TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex);

                        lTableCell += "\"";
                    }

                    lTableCell += "> %1</a></b></td>";
                    WriteTableCell(lTableCell,
                                   TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex));

                    WriteTableCell("<td width=\"50%%\" bgcolor=\"#F8F8F8\">%1</td>\n", lTestLimit.mTestName);
                }
                else
                {
                    WriteTableCell("<td width=\"20%%\" bgcolor=\"#F8F8F8\"></td>");
                    WriteTableCell("<td width=\"50%%\" bgcolor=\"#F8F8F8\"></td>");
                }

                WriteTableCell("<td align=\"center\" width=\"15%%\" bgcolor=\"#F8F8F8\">%1</td>\n",
                               QString::number(lTestLimit.mSite));

                lTableCell = "<td align=\"center\" width=\"15%%\" bgcolor=\"#";

                if(lPatDef->m_DynamicFailCountPerSite.contains(lTestLimit.mSite))
                    lTableCell += "FF0000";	// Alarm (fail count > 0)
                else
                    lTableCell += "FFFFCC";	// Fine (fail count = 0)

                lTableCell += "\"><a ";

                // Write Test# + hyperlink
                if (mOutputFormat == "HTML")
                {
                    // Regular html page
                    lTableCell += "href=\"" + mReportPagesName.value("DPAT_LIMITS");
                    lTableCell += "#T" + TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex);
                    lTableCell += "-S" + QString::number(lTestLimit.mSite);
                    lTableCell += "\"";
                }
                else
                {
                    // Flat html to endup in one file
                    lTableCell += "href=\"#T" + TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex);
                    lTableCell += "-S" + QString::number(lTestLimit.mSite);
                    lTableCell += "\"";
                }

                lTableCell += "> %1</a></td>";

                WriteTableCell(lTableCell,
                               QString::number(lPatDef->m_DynamicFailCountPerSite.value(lTestLimit.mSite)),
                               true);
                WriteReportLine("</tr>");

                lTestNumber = lTestLimit.mTestNumber;
                lPinIndex   = lTestLimit.mPinIndex;
                lTestName   = lTestLimit.mTestName;
                lSite       = lTestLimit.mSite;
            }
        }

        WriteReportLine("</table>");
        WriteReportLine("<br>");
    }

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    // Insert a page break for flat html files
    mParent->mGexReport->WritePageBreak();

    return true;
}

bool FTPATReportUnitPrivate::CreateDPATTestsLimitsPage()
{
    CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                     NULL : mParent->mGexReport->getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if(lFile == NULL)
    {
        mErrorMessage = "No datasets found";
        return false;
    }

    if (CheckForNewPage(mReportPagesName.value("DPAT_LIMITS"), "all_dyn_pat_limits",
                        "Dynamic-PAT Test Limits") == false)
        return false;

    mParent->mGexReport->SetPowerPointSlideName("Outliers: Dynamic-PAT limits");

    // Write backward link
    WriteBackwardLink();

    CTest *                         lTestCell   = NULL;
    CPatDefinition *                lPatDef     = NULL;
    QList<PATTestLimits>::iterator  itTestLimit;
    PATTestLimits                   lTestLimit;
    bool                            lTestMatch  = false;
    int                             lTestNumber = -1;
    int                             lPinIndex   = -1;
    int                             lSite       = -1;
    int                             lTuning     = 0;
    int                             lOutlier    = 0;
    QString                         lTestName;
    QString                         lTuningName;
    QString                         lDecoration;
    QString                         lKey;

    for (itTestLimit = mDPATTestLimits.begin(); itTestLimit != mDPATTestLimits.end(); ++itTestLimit)
    {
        lTestLimit  = *itTestLimit;
        lTestMatch  = IsTestMatch(lTestNumber, lPinIndex, lTestName, lTestLimit);

        if (lTestMatch == false || lTestLimit.mSite != lSite)
        {
            lPatDef  = mRecipe.FindUniVariateRule(lTestLimit.mTestNumber,
                                                  lTestLimit.mPinIndex,
                                                  lTestLimit.mTestName);

            if (lPatDef == NULL)
                continue;

            // If outlier, check if we have to report outliers...
            if((lPatDef->m_DynamicFailCountPerSite.count() > 0) &&
               (mRecipe.GetOptions().bReport_DPAT_Limits_Outliers == false))
                continue;

            // If not outlier, check if we have to report tests without outlier...
            if((lPatDef->m_DynamicFailCountPerSite.count() == 0) &&
               (mRecipe.GetOptions().bReport_DPAT_Limits_NoOutliers == false))
                continue;

            // Get pointer to test cell with data
            if(lFile->FindTestCell(lPatDef->m_lTestNumber, lPatDef->mPinIndex,
                                   &lTestCell, true, false, lPatDef->m_strTestName) != 1)
                continue;

            if (lTestNumber != -1)
            {
                // close table
                WriteReportLine("</table>");
                WriteReportLine("<br><br>");

                lTuning = 0;
            }

            // For CSV output, add a empty line before the table
            if (mOutputFormat == "CSV")
                WriteReportEmptyLine();

            // Create bookmark
            lDecoration = "<h2 align=\"left\"><font color=\"#006699\">";
            lDecoration += "<a name=\"";
            lDecoration += "T" + TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex);
            lDecoration += "-S" + QString::number(lTestLimit.mSite);
            lDecoration += "\"></a>%1</font></h2>";

            WriteReportLine(lDecoration,
                            QString("Test %1 %2 - Site %3")
                            .arg(TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex))
                            .arg(lTestLimit.mTestName).arg(QString::number(lTestLimit.mSite)));
            WriteReportLine("<br>");

            // Open table
            QString lTableHeader;

            lTableHeader = "<table border=\"1\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: ";
            lTableHeader += QString::number(mParent->mGexReport->iHthmSmallFontSizePixels);
            lTableHeader += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">";

            // write header
            WriteReportLine(lTableHeader);
            WriteReportLine("<tr>");
            WriteTableCell("<td align=\"center\" width=\"10%%\" bgcolor=\"#CCFFFF\"></td>\n");
            WriteTableCell("<td align=\"center\" width=\"3%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                           QStringList("DPAT") << "Fails");
            WriteTableCell("<td align=\"center\" width=\"5%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>",
                           "Shape");
            WriteTableCell("<td align=\"center\" width=\"5%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                           QStringList("Outlier") << "rule");
            WriteTableCell("<td align=\"center\" width=\"4%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>",
                           "N Factor");
            WriteTableCell("<td align=\"center\" width=\"4%%\" bgcolor=\"#CCFFFF\"><b>%1</b></td>",
                           "T Factor");
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                           QStringList("Original") << "Low Limit");
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                           QStringList("Original") << "High Limit");
            WriteTableCell("<td align=\"center\" width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                           QStringList("Dynamic PAT") << "Low Limit");
            WriteTableCell("<td align=\"center\" width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1<br>%2</b></td>",
                           QStringList("Dynamic PAT") << "High Limit");
            WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCFFFF\"><b>%1</b>", "Misc. stats.", true);
            WriteReportLine("</tr>");

            lTestNumber = lTestLimit.mTestNumber;
            lPinIndex   = lTestLimit.mPinIndex;
            lTestName   = lTestLimit.mTestName;
            lSite       = lTestLimit.mSite;
        }

        // write baseline
        WriteReportLine("<tr>");

        if (lTestLimit.mRetestIndex == 0)
        {
            if (lTuning == 0)
                lTuningName = "Baseline";
            else if (lTuning == 1)
                lTuningName = "Subsequent Baseline";
            else
                lTuningName = "Tuning";
        }
        else
            lTuningName = "Retest";

        WriteTableCell("<td align=\"center\" width=\"10%%\" bgcolor=\"#F8F8F8\">%1</td>",
                       lTuningName);

        lKey = QString("%1:%2:%3")
               .arg(TestNumberLabel(lTestLimit.mTestNumber, lTestLimit.mPinIndex))
               .arg(lTestLimit.mSite).arg(lTestLimit.mRunID);

        if (mDPATOutliers.contains(lKey))
            lOutlier = mDPATOutliers.value(lKey);
        else
            lOutlier = 0;

        if (lOutlier > 0 )
            WriteTableCell("<td align=\"center\" width=\"3%%\" bgcolor=\"#FF0000\">%1</td>",
                           QString::number(lOutlier));
        else
            WriteTableCell("<td align=\"center\" width=\"3%%\" bgcolor=\"#FFFFCC\">%1</td>",
                           QString::number(lOutlier));

        WriteTableCell("<td width=\"5%%\" bgcolor=\"#F8F8F8\">%1</td>", lTestLimit.mDistributionShape);
        WriteTableCell("<td width=\"5%%\" bgcolor=\"#F8F8F8\">%1</td>", gexRuleSetItemsGUI[lPatDef->mOutlierRule]);

        if(lTestLimit.mNFactor && lTestLimit.mRetestIndex == 0)
            WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>",
                           QString::number(lTestLimit.mNFactor));
        else
            WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>","-");

        if(lTestLimit.mTFactor && lTestLimit.mRetestIndex == 0)
            WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>",
                           QString::number(lTestLimit.mTFactor));
        else
            WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>","-");

        // Low Limit
        if((lPatDef->m_lfLowLimit > -GEX_TPAT_FLOAT_INFINITE) && ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0))
        {
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                           QStringList(QString::number(lPatDef->m_lfLowLimit)) << lPatDef->m_strUnits);
        }
        else
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>", "n/a");

        // Original program: High Limit
        if((lPatDef->m_lfHighLimit < GEX_TPAT_FLOAT_INFINITE) && ((lTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
        {
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                           QStringList(QString::number(lPatDef->m_lfHighLimit)) << lPatDef->m_strUnits);
        }
        else
            WriteTableCell("<td width=\"7%%\" bgcolor=\"#F8F8F8\">%1</td>", "n/a");

        WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                       QStringList(QString::number(lTestLimit.mPATLL)) << lPatDef->m_strUnits);

        WriteTableCell("<td width=\"10%%\" bgcolor=\"#F8F8F8\">%1 %2</td>",
                       QStringList(QString::number(lTestLimit.mPATHL)) << lPatDef->m_strUnits);

        // Misc: display low-level statitics used for computing DPAT limits
        QStringList lTextCell;
        QString     lDecoration;

        if (lTestLimit.mRetestIndex == 0)
        {
            switch(lPatDef->mOutlierRule)
            {
                case GEX_TPAT_RULETYPE_SIGMAID:			// Rule 'sigma' ID (offset in combo)
                    lDecoration = "<td>%1<br>%2</td>";
                    lTextCell << QString("Mean=%1").arg(lTestLimit.mMean);
                    lTextCell << QString("Sigma=%1").arg(lTestLimit.mSigma);
                    break;
                case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:			// Rule 'N*RobustSigma' ID (offset in combo)
                    lDecoration = "<td>%1<br>%2</td>";
                    lTextCell << QString("Median=%1").arg(lTestLimit.mMedian);
                    lTextCell << QString("RSigma=%1").arg((lTestLimit.mQ3 - lTestLimit.mQ1)/1.35);
                    break;
                case GEX_TPAT_RULETYPE_Q1Q3IQRID:			// Rule 'Q1/Q3 IQR' ID (offset in combo)
                    lDecoration = "<td>%1<br>%2<br>%3</td>";
                    lTextCell << QString("Q1=%1").arg(lTestLimit.mQ1);
                    lTextCell << QString("Q3=%1").arg(lTestLimit.mQ3);
                    lTextCell << QString("IQR=%1").arg(lTestLimit.mQ3 - lTestLimit.mQ1);
                    break;
                case GEX_TPAT_RULETYPE_LIMITSID:			// Rule '%limit' ID (offset in combo)
                case GEX_TPAT_RULETYPE_NEWLIMITSID:			// Rule 'newlimit' ID (offset in combo)
                case GEX_TPAT_RULETYPE_RANGEID:			// Rule 'range' ID (offset in combo)
                case GEX_TPAT_RULETYPE_SMARTID:			// Rule 'smart' ID (offset in combo)
                case GEX_TPAT_RULETYPE_IGNOREID:			// Rule 'disabled' ID (offset in combo)
                case GEX_TPAT_RULETYPE_SEPARATORID:			// Rule separator
                case GEX_TPAT_RULETYPE_GAUSSIANID:			// Rule: Gaussian distribution
                case GEX_TPAT_RULETYPE_GAUSSIANTAILID:			// Rule: Gaussian+Tail distribution
                case GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID:		// Rule: Gaussian+Tail distribution
                case GEX_TPAT_RULETYPE_LOGNORMALID:					// Rule: LogNormal distribution
                case GEX_TPAT_RULETYPE_BIMODALID:			// Rule: Bi-Modal distribution, with each mode clearly apart from the other
                case GEX_TPAT_RULETYPE_MULTIMODALID:			// Rule: Multi-Modal distribution
                case GEX_TPAT_RULETYPE_CLAMPEDID:			// Rule: Clamped distribution
                case GEX_TPAT_RULETYPE_DUALCLAMPEDID:			// Rule: DoubleClamped distribution
                case GEX_TPAT_RULETYPE_CATEGORYID:			// Rule: Categories distribution
                case GEX_TPAT_RULETYPE_CUSTOMLIBID:
                    lDecoration = "<td>%1 %2<br>%3 %4<br>%5<br>%6</td>";
                    lTextCell << QString("Mean=%1").arg(lTestLimit.mMean);
                    lTextCell << QString("Median=%1").arg(lTestLimit.mMedian);
                    lTextCell << QString("Sigma=%1").arg(lTestLimit.mSigma);
                    lTextCell << QString("Q1=%1").arg(lTestLimit.mQ1);
                    lTextCell << QString("Q3=%1").arg(lTestLimit.mQ3);
                    lTextCell << QString("IQR=%1").arg(lTestLimit.mQ3 - lTestLimit.mQ1);
                    break;
            }
        }
        else
        {
            lDecoration = "<td align=\"center\">%1</td>";
            lTextCell << "-";
        }

        WriteTableCell(lDecoration, lTextCell, true);
        WriteReportLine("</tr>");

        ++lTuning;
    }

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    return true;
}

bool FTPATReportUnitPrivate::CreateTestsFailingPATLimitsPage()
{
    if (CheckForNewPage(mReportPagesName.value("SUMMARY"),
                        "all_tests_dyn_fails", "Summary: Tests failing PAT limits") == false)
        return false;

    mParent->mGexReport->SetPowerPointSlideName("Outliers: Failing tests");

    // Write backward link
    WriteBackwardLink();

    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
    QList<CPatDefinition*>                      lPatDefinition;
    CPatDefinition *                            lPatDef     = NULL;
    bool                                        lIgnoreTest;

    for(itPATDefinifion = mRecipe.GetUniVariateRules().begin();
        itPATDefinifion != mRecipe.GetUniVariateRules().end(); ++itPATDefinifion)
    {
        lPatDef = *itPATDefinifion;

        // Only include tests that have PAT outliers
        lIgnoreTest = false;

        if(!lPatDef->m_TotalFailures)
            lIgnoreTest = true;

        if(lIgnoreTest == false)
            lPatDefinition.append(lPatDef);
    }

    if (lPatDefinition.count())
        // Sort results in descending order
        qSort(lPatDefinition.begin(), lPatDefinition.end(), ComparePatResultOutliers);
    else
    {
        WriteReportLine("No test failed any of the Outlier limits!");
        WriteReportEmptyLine();
        WriteReportEmptyLine();
    }

    QString lHyperLink;
    int     lLineCount  = 0;
    int     lMaxLine    = 15;

    // Write lines
    for(int lIdx = 0; lIdx < lPatDefinition.count(); ++lIdx)
    {
        lPatDef = lPatDefinition.at(lIdx);

        if (lLineCount == 0)
            BuildHeaderTestsFailingPATLimits();

        ++lLineCount;

        lHyperLink = "<td bgcolor=\"#F8F8F8\"><b><a ";

        if (mOutputFormat == "HTML")
        {
            // Regular html page
            lHyperLink += "href=\"#_gex_drill--drill_chart=adv_trend--data=";
            lHyperLink += TestNumberLabel(lPatDef->m_lTestNumber, lPatDef->mPinIndex);
            lHyperLink += "\"";
        }
        else
        {
            // Flat html to endup in one file
            lHyperLink += "href=\"#HistoT" + TestNumberLabel(lPatDef->m_lTestNumber, lPatDef->mPinIndex);
            lHyperLink += "\"";
        }

        lHyperLink += "> %1</a></b></td>";

        WriteReportLine("<tr>");
        WriteTableCell(lHyperLink, TestNumberLabel(lPatDef->m_lTestNumber, lPatDef->mPinIndex));
        WriteTableCell("<td bgcolor=\"#F8F8F8\">%1</td>", lPatDef->m_strTestName);
        WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>",
                       gexIgnoreSamplesSetItemsGUI[lPatDef->m_SamplesToIgnore]);
        WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>",
                       gexKeepOutliersSetItemsGUI[lPatDef->m_OutliersToKeep]);
        WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1</td>\n",
                       gexOutlierLimitsSetItemsGUI[lPatDef->m_iOutlierLimitsSet]);
        WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1 &nbsp;&nbsp;%2&nbsp;&nbsp; %3</td>",
                       QStringList() << QString::number(lPatDef->m_lStaticFailuresLow) << "/" <<
                       QString::number(lPatDef->m_lStaticFailuresHigh));
        WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\">%1 &nbsp;&nbsp;%2&nbsp;&nbsp; %3</td>",
                       QStringList() << QString::number(lPatDef->m_lDynamicFailuresLow[lPatDef->m_iOutlierLimitsSet]) <<
                       "/" << QString::number(lPatDef->m_lDynamicFailuresHigh[lPatDef->m_iOutlierLimitsSet]));
        WriteTableCell("<td align=\"center\" bgcolor=\"#F8F8F8\"><b>%1</b></td>\n",
                       QString::number(lPatDef->m_TotalFailures), true);
        WriteReportLine("</tr>");

        // Write maximum of 20 lines per page if creating PowerPoint slides (except first page that holds few lines).
        if(mOutputFormat == "PPT" && ((lLineCount % lMaxLine) == 0))
        {
            // close table & goto next page
            WriteReportLine("</table>");
            mParent->mGexReport->WritePageBreak();

            // Reset line counter in page (so will reopen table if needed)
            lLineCount = 0;

            // Number of lines in table (page 2 and following) is 20
            lMaxLine = 20;
        }
    }

    if (lLineCount)
        WriteReportLine("</table>");

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    // Insert a page break for flat html files
    mParent->mGexReport->WritePageBreak();

    return true;
}

bool FTPATReportUnitPrivate::CreateWarningLogsPage()
{
    CGexGroupOfFiles *  lGroup = mParent->mGexReport->getGroupsList().isEmpty() ?
                                     NULL : mParent->mGexReport->getGroupsList().first();
    CGexFileInGroup *   lFile  = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if(lFile == NULL)
    {
        mErrorMessage = "No datasets found";
        return false;
    }

    if (CheckForNewPage(mReportPagesName.value("LOG"), "warning_log",
                        "Warning Log") == false)
        return false;

    mParent->mGexReport->SetPowerPointSlideName("Warning Log");

    // Write backward link
    WriteBackwardLink();

    // For CSV output, add a empty line before the warnings
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    // Check if results available
    if(mWarningLogs.count() <= 0)
    {
        WriteReportLine("%1", "No Warning detected during PAT processing!");
    }
    else
    {
        for(int lIdx = 0; lIdx < mWarningLogs.count(); ++lIdx)
        {
            WriteReportLine("%1", mWarningLogs.at(lIdx));
            WriteReportLine("<br>");
        }

        WriteReportLine("<br>");
    }

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    return true;
}

bool FTPATReportUnitPrivate::IsTestMatch(unsigned long TestNumber, long PinIndex,
                                         const QString &TestName,
                                         const PATTestLimits &PatTestLimit)
{
    bool lMatch = false;

    switch(mRecipe.GetOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            if(TestNumber == static_cast<unsigned long>(PatTestLimit.mTestNumber) &&
               PinIndex == PatTestLimit.mPinIndex)
            {
                lMatch = true;
            }
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            if(TestName.compare(PatTestLimit.mTestName) == 0 &&
               PinIndex == PatTestLimit.mPinIndex)
            {
                lMatch = true;
            }
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            if(TestNumber == static_cast<unsigned long>(PatTestLimit.mTestNumber) &&
               TestName.compare(PatTestLimit.mTestName) == 0 &&
               PinIndex == PatTestLimit.mPinIndex)
            {
                lMatch = true;
            }
            break;

        default:
            break;
    }

    return lMatch;
}

CPatDefinition *FTPATReportUnitPrivate::FindUnivariateRule(QList<CPatDefinition *> UnivariateRules, const PATTestLimits &PatTestLimit)
{
    CPatDefinition * lUnivariate = NULL;

    for(int lIdx = 0; lIdx < UnivariateRules.count() && lUnivariate == NULL; ++lIdx)
    {
        switch(mRecipe.GetOptions().mTestKey)
        {
            case GEX_TBPAT_KEY_TESTNUMBER:
                if(UnivariateRules.at(lIdx)->m_lTestNumber == (unsigned long) PatTestLimit.mTestNumber &&
                   UnivariateRules.at(lIdx)->mPinIndex == PatTestLimit.mPinIndex)
                {
                    lUnivariate = UnivariateRules.at(lIdx);
                }
                break;

            case GEX_TBPAT_KEY_TESTNAME:
                if(UnivariateRules.at(lIdx)->m_strTestName.compare(PatTestLimit.mTestName) == 0 &&
                   UnivariateRules.at(lIdx)->mPinIndex == PatTestLimit.mPinIndex)
                {
                    lUnivariate = UnivariateRules.at(lIdx);
                }
                break;

            case GEX_TBPAT_KEY_TESTMIX:
                if(UnivariateRules.at(lIdx)->m_lTestNumber == (unsigned long) PatTestLimit.mTestNumber &&
                   UnivariateRules.at(lIdx)->m_strTestName.compare(PatTestLimit.mTestName) == 0 &&
                   UnivariateRules.at(lIdx)->mPinIndex == PatTestLimit.mPinIndex)
                {
                    lUnivariate = UnivariateRules.at(lIdx);
                }
                break;

            default:
                return NULL;
                break;
        }
    }

    return lUnivariate;
}

void FTPATReportUnitPrivate::BuildHeaderPartsFailingPATLimits()
{
    QString lTableHeader;

    lTableHeader = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: ";
    lTableHeader += QString::number(mParent->mGexReport->iHthmSmallFontSizePixels);
    lTableHeader += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">";

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    WriteReportLine(lTableHeader, "");
    WriteReportLine("<tr>");
    WriteTableCell("<td width=\"10%%\" bgcolor=\"#CCECFF\"><b>%1</b></td>", "LotID");
    WriteTableCell("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>%1</b></td>", "PartID");
    WriteTableCell("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>%1</b></td>", "PatBin");
    WriteTableCell("<td width=\"5%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>%1</b></td>", "Site");
    WriteTableCell("<td width=\"50%%\" align=\"center\" bgcolor=\"#CCECFF\"><b>%1</b></td>",
                   "Tests failing PAT Limits (Outliers)", true);
    WriteReportLine("</tr>");

    mParent->mGexReport->SetPowerPointSlideName("Outliers: Parts");
}

void FTPATReportUnitPrivate::BuildHeaderTestsFailingPATLimits()
{
    QString lTableHeader;

    lTableHeader = "<table border=\"0\" cellspacing=\"2\" width=\"98%%\" style=\"font-size: ";
    lTableHeader += QString::number(mParent->mGexReport->iHthmSmallFontSizePixels);
    lTableHeader += "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">";

    // For CSV output, add a empty line before the table
    if (mOutputFormat == "CSV")
        WriteReportEmptyLine();

    WriteReportLine(lTableHeader, "");
    WriteReportLine("<tr>");
    WriteTableCell("<td bgcolor=\"#CCECFF\"><b>%1</b></td>", "Test");
    WriteTableCell("<td bgcolor=\"#CCECFF\"><b>%1</b></td>", "Name");
    WriteTableCell("<td align=\"center\" bgcolor=\"#CCECFF\"><b>%1<br>%2</b></td>",
                   QStringList() << "Samples" << "to ignore");
    WriteTableCell("<td align=\"center\" bgcolor=\"#CCECFF\"><b>%1<br>%2</b></td>",
                   QStringList() << "Outliers" << "to keep");
    WriteTableCell("<td align=\"center\" bgcolor=\"#CCECFF\"><b>%1<br>%2</b></td>",
                   QStringList() << "Outlier" << "Limits set");
    WriteTableCell("<td align=\"center\" bgcolor=\"#CCECFF\"><b>%1<br>%2</b></td>",
                   QStringList() << "Static Fails" << "Low / High");
    WriteTableCell("<td align=\"center\" bgcolor=\"#CCECFF\"><b>%1<br>%2</b></td>",
                   QStringList() << "Dynamic Fails" << "Low / High");
    WriteTableCell("<td align=\"center\" bgcolor=\"#CCECFF\"><b>%1<br>%2</b></td>",
                   QStringList() << "Total" << "PAT Fails", true);
    WriteReportLine("</tr>");

    mParent->mGexReport->SetPowerPointSlideName("Outlier: Tests");
}

void FTPATReportUnitPrivate::WriteBackwardLink()
{
    if(mSplitReport)
    {
        WriteReportLine("<br>");
        WriteReportLine("Back to <a href=\"advanced.htm\">Table of contents</a>");
        WriteReportLine("<br><br>");
    }
}

void FTPATReportUnitPrivate::WriteReportEmptyLine()
{
    fprintf(mParent->mGexReport->getReportFile(), "\n");
}

void FTPATReportUnitPrivate::WriteReportLine(const QString &lDecoration, const QString &lText)
{
    QStringList lTextList;

    if (lText.isEmpty() == false)
        lTextList.append(lText);

    WriteReportLine(lDecoration, lTextList);
}

void FTPATReportUnitPrivate::WriteReportLine(const QString& lDecoration, const QStringList &lText)
{
    QString lCell;

    if (mOutputFormat != "CSV")
        lCell = lDecoration;

    for (int lIdx = 0; lIdx < lText.count(); ++lIdx)
    {
        if (mOutputFormat != "CSV")
            lCell = lCell.arg(lText.at(lIdx));
        else
            lCell += lText.at(lIdx);
    }

    fprintf(mParent->mGexReport->getReportFile(), "%s", lCell.toLatin1().constData());

    if (mOutputFormat != "CSV" || lText.count() > 0)
        WriteReportEmptyLine();
}

void FTPATReportUnitPrivate::WriteTableCell(const QString &lDecoration, const QString &lText,
                                            bool lLast)
{
    QStringList lTextList;

    if (lText.isEmpty() == false)
        lTextList.append(lText);

    WriteTableCell(lDecoration, lTextList, lLast);
}

void FTPATReportUnitPrivate::WriteTableCell(const QString &lDecoration, const QStringList &lText,
                                            bool lLast)
{
    QString lCell;

    if (mOutputFormat != "CSV")
        lCell = lDecoration;

    for (int lIdx = 0; lIdx < lText.count(); ++lIdx)
    {
        if (mOutputFormat != "CSV")
            lCell = lCell.arg(lText.at(lIdx));
        else
            lCell += lText.at(lIdx) + " ";
    }

    fprintf(mParent->mGexReport->getReportFile(), "%s",  lCell.toLatin1().constData());

    if (mOutputFormat != "CSV" || lLast)
        WriteReportEmptyLine();
    else
        fprintf(mParent->mGexReport->getReportFile(), ",");
}

}
}
