#include <float.h>
#include <limits>
#include <limits.h>

#include <QMap>
#include <QLabel>
#include <QCoreApplication>

#include "cpart_info.h"
#include "gex_report_unit.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "charac1_report_unit.h"
#include "gex_report.h"
#include "product_info.h"
#include <gqtl_log.h>
#include "gex_charac_boxwhisker_chart.h"
#include "gex_charac_histo_chart.h"
#include "engine.h"

extern QString			formatHtmlImageFilename(const QString& strImageFileName);
extern bool             CTestFailCountLessThan(CTest* s1, CTest* s2);

namespace GS
{
namespace Gex
{
    Charac1ReportUnit::Charac1ReportUnit(CGexReport* gr, const QString &key)
        : ReportUnit(gr, key)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new Charac1ReportUnit : %1").arg( key).toLatin1().constData());
    }

    Charac1ReportUnit::~Charac1ReportUnit()
    {
       GSLOG(SYSLOG_SEV_INFORMATIONAL, "destructor...");
    }

    QString Charac1ReportUnit::PrepareSection(bool bValidSection)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("ValidSection = %1").arg( bValidSection?"true":"false").toLatin1().constData());
        QString strOutputFormat=mGexReport->GetOption("output", "format").toString();
        QString strOptionStorageDevice  = (mGexReport->GetOption("statistics","generic_galaxy_tests")).toString();

        if (strOutputFormat=="CSV")
        {
            // Generating .CSV report file.
            fprintf(mGexReport->getReportFile(),"\n\nAdvanced Characterization Report\n");
        }
        else
        if (mGexReport->getReportOptions()->isReportOutputHtmlBased())
         //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        {
            // Generating HTML report file.
            // Open advanced.htm file
            if(OpenFile().startsWith("error") )
             return "error : cant open file";	//Stdf::ReportFile;

            mGexReport->WriteHeaderHTML(mGexReport->getReportFile(),"#000000");	// Default: Text is Black

            // Title + bookmark
            WriteHtmlSectionTitle(mGexReport->getReportFile(),"all_advanced","More Reports: Advanced Characterization Report");
        }

        // Build the test list to report
        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles * pGroup = mGexReport->getGroupsList().isEmpty() ? NULL : mGexReport->getGroupsList().first();

        if (!pGroup)
        {
            GSLOG(SYSLOG_SEV_WARNING, " error : cant retrieve first pGroup in pGroupsList !");
            return "error";
        }

        int topNFail = 0;

        if (mGexReport->getReportOptions()->strAdvancedTestList.startsWith("top"))
        {
            bool ok = false;
            topNFail = QString( mGexReport->getReportOptions()->strAdvancedTestList ).section(' ',1,1).toInt(&ok);

            if (!ok)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString(" failed to understand '%1'")
                      .arg( mGexReport->getReportOptions()->strAdvancedTestList)
                      .toLatin1().constData() );
                topNFail = 5;
            }
            else
            {
                GSLOG(SYSLOG_SEV_NOTICE,
                      QString(" will consider only top %1 failed tests ").arg( topNFail)
                      .toLatin1().constData());
            }
        }

        CTest * ptTestCell = pGroup->cMergedData.ptMergedTestList;
        while(ptTestCell != NULL)
        {
            // Check if valid test (samples and not master test of a MultiParametric)
            // or not a parametric / multiparametric (eg: functional) test, ignore!
            // or ignore Generic Galaxy Parameters
            if((CGexReport::validSamplesExecsOverGroups(ptTestCell->lTestNumber,
                                                        ptTestCell->lPinmapIndex,
                                                        ptTestCell->strTestName,
                                                        mGexReport->getGroupsList()) <= 0) ||
                    (ptTestCell->lResultArraySize > 0) ||
                    (ptTestCell->bTestType == 'F') ||
                    ((ptTestCell->bTestType == '-') && (strOptionStorageDevice == "hide")))
            {
                // Point to next test cell
                ptTestCell = ptTestCell->GetNextTest();
                continue;
            }

            if (mGexReport->getReportOptions()->strAdvancedTestList.startsWith("top") )
                mTestsListToReport.append(ptTestCell);	// we will sort and trim later
            else if (mGexReport->getReportOptions()->strAdvancedTestList=="all")
                mTestsListToReport.append(ptTestCell);
            else
            {
                // assuming we are in given TestsList	mode
                if (mGexReport->getReportOptions()->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber, ptTestCell->lPinmapIndex))
                    mTestsListToReport.append(ptTestCell);
            }

            // Point to next test cell
            ptTestCell = ptTestCell->GetNextTest();
        };	// Loop until all test cells read.

        if (mGexReport->getReportOptions()->strAdvancedTestList.startsWith("top") )
        {
            qSort(mTestsListToReport.begin(), mTestsListToReport.end(), CTestFailCountLessThan);

            // forget undesired Tests
            while(mTestsListToReport.size() > topNFail)
                mTestsListToReport.removeLast();
        }

        return "ok";
    }

    QString Charac1ReportUnit::CreatePages()
    {
        if (!mGexReport)
        {
            return "error : mGexReport null";
        }

        mGexReport->WriteText( "", "a", "all_advanced");
        mGexReport->WriteText(mGexReport->GetOption("adv_charac1", "report_title").toString(),
                             "h1", "style=\"color:#006699;text-align:center\"");
        mGexReport->EndLine();

        // Todo : disable if license not allowed
        if (!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
          QString m=ReportOptions.GetOption("messages", "upgrade").toString();
          mGexReport->WriteText(m);
          return "ok";
        }

        if (mGexReport->getGroupsList().size()<2)
        {
            mGexReport->WriteText( QString("Error : not enough groups : %1.\n").arg(mGexReport->getGroupsList().size()) );
            return "error : not enough group";
        }

        QString lTitlesAttributes=" style=\"font-weight:bold;text-align:center\"";
        QString lFirstTableLineAtts=" style=\"background-color:#CCECFF; font-weight:bold; text-align:center \""; // border:medium solid blue;
        // atts of tables : // empty-cells:show  ?
        QString lTableAtts=
          "align=center style=\"align:center; background-color:#F8F8F8; border:1; border-color:#111111; border-collapse:collapse; cellspacing:2; cellpadding:0; \"";

        mGexReport->EndLine();

        QStringList lCharacFields = mGexReport->GetOption("adv_charac1", "field").toString().split("|");
        QStringList lExtraFields = mGexReport->GetOption("adv_charac1", "extra_fields").toString().split("|", QString::SkipEmptyParts);
        if (lExtraFields .size()>0) lCharacFields.append(lExtraFields );

        const int lTestCondCount = mGexReport->GetTestConditionsCount();
        const int lColCount = mGexReport->getGroupsList().size() + 1;
        int lLeftMargin = 25;

        QString lOutputFormat = mGexReport->GetOption("output", "format").toString().toLower();
        QString lPrinterType=mGexReport->GetOption("output", "pdf_printer").toString();
        QString lOutputPaperFormat=mGexReport->GetOption("output", "paper_format").toString().toLower();

        GSLOG(SYSLOG_SEV_NOTICE, QString("Char report creating pages: num col=%1 num of conditions: %2")
              .arg(lColCount).arg(lTestCondCount).toLatin1().data() );
        // GCORE-118
        // HTML 4 font size is limited to an integer number from 0 to 7 ....
        // But a size of 7 is non sense, no need huge font size, 5 should be enough.
        // POC shown that HTML4 font size 0 can support up to :
        // - 30 cells max in landscape mode (about 10chars per cell)
        // - 13 cells max in portrait mode
        unsigned lHTML4FontSize=0; // the minimum HTML4 font size
        if (lOutputPaperFormat=="portrait")
        {
            if (lColCount < 13) lHTML4FontSize=(unsigned)floor((1.f-((float)lColCount)/13.f)*6.f);
        }
        else // landscape
        {
            if (lColCount < 30) lHTML4FontSize=(unsigned)floor((1.f-((float)lColCount)/30.f)*6.f);
        }
        // Font size in cm is not supported by htmldoc so no need
        // float lFontSizeInCm=ncol>0?(20.f/((float)ncol*6.f)):-1.f;
        // if html output format then let s try the CSS3 viewport percent font size that seems to work
        // Let s consider max 6, 7, 8... characters per cell ?
        // Max column allowed in the table to avoid rendering issues
        int lMaxColPerTable = GetMaxColumns();;
        float lViewPort = -1.f;//lMaxColPerTable>0?(100.f/((float)lMaxColPerTable*7.f)):10.f;
        GSLOG(SYSLOG_SEV_NOTICE, QString("Charac report: best HTML4 font size= %1  Best HTML CSS viewport fontsize=%2")
              .arg(lHTML4FontSize).arg(lViewPort).toLatin1().data() );

        // Draw chart and table for each test
        foreach(CTest* ptTestCell, mTestsListToReport)
        {
            GS::Gex::Engine::GetInstance().UpdateLabelStatus( QString(" Computing Test %1...").arg(ptTestCell->lTestNumber) );
            QCoreApplication::instance()->QCoreApplication::processEvents();

            ptTestCell->UpdateProperties();

            // Draw chart
            if (lOutputFormat == "csv")
            {
                HtmlTableRow lTableRow;
                HtmlTableItem lTableItem;
                lTableItem.mData = QString("Test %1 : %2").arg(ptTestCell->lTestNumber).arg(ptTestCell->strTestName);
                lTableItem.mAttribute = QString("style=\"text-align:center;font-weight:bold;\" align=center colspan=%1").arg(lColCount);
                lTableRow.append(lTableItem);
                mGexReport->WriteTableRow(lTableRow);
            }
            else
            {
                WriteChart(ptTestCell, lColCount, lLeftMargin);
            }


            if (lOutputFormat == "csv")
            {
                WriteTableSplit(mGexReport->getGroupsList(),
                               lOutputFormat,
                               lPrinterType,
                               lHTML4FontSize,
                               lViewPort,
                               lCharacFields,
                               lTestCondCount,
                               ptTestCell);
            }
            else
            {
                int lTableCount = 1;
                if (lColCount >1) lTableCount = 1 + ((lColCount - 1) / lMaxColPerTable);

                for (int lTabId = 0; lTabId < lTableCount; ++lTabId)
                {
                    int lGroupFirstItem = lTabId * lMaxColPerTable;
                    QList<CGexGroupOfFiles*> lColToReport = mGexReport->getGroupsList().mid(lGroupFirstItem, lMaxColPerTable);
                    WriteTableSplit(lColToReport,
                                    lOutputFormat,
                                    lPrinterType,
                                    lHTML4FontSize,
                                    lViewPort,
                                    lCharacFields,
                                    lTestCondCount,
                                    ptTestCell);
                }

            }

            mGexReport->WritePageBreak();
        } // for each test cell


        mGexReport->EndLine();

        WriteGroupsSummaryTable(lTitlesAttributes, lTableAtts, lFirstTableLineAtts);
        mGexReport->EndLine();

        return "ok";
    }

    QString Charac1ReportUnit::WriteGroupsSummaryTable(QString lTitlesAttributes, QString lTableAtts, QString lFirstTableLineAtts)
    {
        mGexReport->BeginTable(lTableAtts);
        QList<QString> labels = QList<QString>() << "Groups summary" << "  " << "(Top)";
        QList<QString> anchors = QList<QString>() << "name=\"GroupsSummary\"" << "" << "href=\"#Top\"";
        mGexReport->WriteTableRow(labels, lTitlesAttributes, anchors);
        mGexReport->EndTable();
        mGexReport->EndLine();

        // Groups summary :
        mGexReport->BeginTable(lTableAtts);
        //mGexReport->WriteInfoLine(QString("Groups (%1): ").arg(mGexReport->getGroupsList().size()).toLatin1().data(), "<br>\n");
        QList<QString> l;
        l << "Group name" << "Part type" << "Total valid file" << "Number of tests" << "Parts count (from MIR)";
        mGexReport->WriteTableRow(l, lFirstTableLineAtts);
        foreach(CGexGroupOfFiles* gof, mGexReport->getGroupsList())
            if (gof)
            {
                l.clear();
                l << gof->strGroupName;
                l << (gof->pFilesList.size()>0?gof->pFilesList.at(0)->getMirDatas().szPartType:"?");
                l << QString::number(gof->GetTotalValidFiles());
                l << QString::number(gof->cMergedData.GetNumberOfTests());
                l << QString::number( gof->GetTotalMirDataPartCount() );
                mGexReport->WriteTableRow(l,"style=\"text-align:center\"");
            }
            else
            {
                return "error : found a NULL GroupOfFiles";
            }
        mGexReport->EndTable();
        return "ok";
    }

    QString Charac1ReportUnit::CloseSection()
    {
        QString of=mGexReport->GetOption("output", "format").toString();
        if(of=="CSV")
        {
            // Generating .CSV report file: no specific task to perform when section written !
        }
        else if (mGexReport->getReportOptions()->isReportOutputHtmlBased())  //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
        {
            if (of == "HTML")
                CloseReportFile();	// Close report file
        }
        return "ok";
    }

    int Charac1ReportUnit::GetChartWidth() const
    {
        QString lOutputFormat   = mGexReport->GetOption("output", "format").toString();
        QString lPaperFormat    = mGexReport->GetOption("output", "paper_format").toString();
//        QString lPaperSize      = mGexReport->GetOption("output", "paper_size").toString();
        int     lWidth          = 800;

        if (lOutputFormat == "PPT")
            lWidth = 800;
        else if (lOutputFormat == "PDF")
        {
            if (lPaperFormat == "portrait")
                lWidth = 1200;
            else
                lWidth = 1200;
        }
        else if (lOutputFormat == "DOC")
        {
            if (lPaperFormat == "portrait")
                lWidth = 750;
            else
                lWidth = 1000;
        }
        else if (lOutputFormat == "HTML")
            lWidth = 900;
        else if (lOutputFormat == "ODT")
            lWidth = 600;

        return lWidth;
    }

    void Charac1ReportUnit::WriteChart(CTest *aTest,
                                       const int aColCount,
                                       int aLeftMargin)
    {
        mGexReport->BeginTable("border=1 cellpadding=0 cellspacing=0 align=center");
        int lBottomMargin;
        QString lImagePath, lResult;
        QList<QString > lLineData, lLineAttributes;

        lResult = ComputeBoxWhiskerChart(aTest, lImagePath, lBottomMargin, aLeftMargin);
        if (lResult.startsWith("error"))
            lImagePath = lResult;

        lLineData.append(QString("<img src=\"%1\"></img>").arg(lImagePath) );
        lLineAttributes.append(QString(" colspan=%1").arg(aColCount));

        lResult = ComputeHistoChart(aTest, lImagePath, lBottomMargin);
        if (lResult.startsWith("error"))
            lImagePath=lResult;

        lLineData.append(QString("<img src=\"%1\"></img>").arg(lImagePath) );
        mGexReport->WriteTableRow(lLineData, lLineAttributes); // hack : find the number of column
        mGexReport->EndTable();
        mGexReport->EndLine();
    }



    void Charac1ReportUnit::BuildZeroTestCondTableRow(HtmlTableRow& aTableRow,
                                                      const QList<CGexGroupOfFiles *> lColsToReport)
    {
        aTableRow.clear();

        HtmlTableItem lItem;
        lItem.mData = "<b>Var</b>";
        lItem.mAttribute = "align=\"center\"";
        aTableRow.append(lItem);

        // Total widh is 100% of the remaining space
        int lRemainingWidth = 100;
        int lRemainingCols  = mGexReport->getGroupsList().count();
        int lColWidth;

        foreach(CGexGroupOfFiles* lGroup, lColsToReport)
        {
            // Compute the width in % of the column
            if (lRemainingCols)
                lColWidth = lRemainingWidth / lRemainingCols;
            else
                lColWidth= lRemainingWidth;

            if (lGroup)
                lItem.mData = "<b>" + lGroup->strGroupName + "</b>";
            else
                lItem.mData = "<b>n/a</b>";

            lItem.mAttribute = QString("align=\"center\" width=\"%1%%\"").arg(lColWidth);

            aTableRow.append(lItem);

            // Decrement the col width from the total width left
            --lRemainingCols;
            lRemainingWidth -= lColWidth;

        }

    }


    void Charac1ReportUnit::BuildTestCondTableRow(HtmlTableRow &aTableRow,
                                                  int aTestCondId,
                                                  const QList<CGexGroupOfFiles *> lColsToReport)
    {
        aTableRow.clear();

        HtmlTableItem lItem;
        lItem.mData = "<b>" + mGexReport->GetTestConditions(aTestCondId) + "</b>";
        lItem.mAttribute = "align=\"center\"";
        aTableRow.append(lItem);

        // Total widh is 100% of the remaining space
        int lRemainingWidth = 100;
        int lRemainingCols  = mGexReport->getGroupsList().count();
        int lColWidth       = 0;
        int lColSpan        = 1;
        QString lCondValue;

        foreach(CGexGroupOfFiles* lGroup, lColsToReport)
        {
            // Compute the width in % of the column
            if (lRemainingCols)
                lColWidth = lRemainingWidth / lRemainingCols;
            else
                lColWidth = lRemainingWidth;

            if (lGroup)
            {
                // Merge cell having same condition value
                if (lCondValue.isEmpty())
                {
                    lCondValue = lGroup->GetTestConditionsValue(mGexReport->GetTestConditions(aTestCondId)).toString();
                    lColSpan         = 1;
                }
                else if (lCondValue != lGroup->GetTestConditionsValue(mGexReport->GetTestConditions(aTestCondId)).toString())
                {
                    lItem.mData = "<b>" + lCondValue + "</b>";
                    lItem.mAttribute = QString("align=\"center\" width=\"%1%%\" colspan=\"%2\"").arg(lColWidth).arg(lColSpan);
                    aTableRow.append(lItem);

                    lCondValue = lGroup->GetTestConditionsValue(mGexReport->GetTestConditions(aTestCondId)).toString();
                    lColSpan         = 1;
                }
                else
                {
                    ++lColSpan;
                }

                // Empty condition name, so write it in a single cell
                if (lCondValue.isEmpty())
                {
                    lItem.mData = "<b>n/a</b>";
                    lItem.mAttribute = QString("align=\"center\" width=\"%1%%\"").arg(lColWidth);
                    aTableRow.append(lItem);
                    lColSpan         = 1;
                }
            }
            else
            {
                lItem.mData = "<b>n/a</b>";
                lItem.mAttribute = QString("align=\"center\" width=\"%1%%\"").arg(lColWidth);
                aTableRow.append(lItem);
                lColSpan         = 1;
            }

            // Decrement the col width from the total width left
            lRemainingWidth -= lColWidth;
            --lRemainingCols;
        }

        if (lCondValue.isEmpty() == false)
        {
            lItem.mData = "<b>" + lCondValue + "</b>";
            lItem.mAttribute = QString("align=\"center\" width=\"%1%%\" colspan=\"%2\"").arg(lColWidth).arg(lColSpan);
            aTableRow.append(lItem);
        }
    }

    void Charac1ReportUnit::BuildCharacFieldsTableRow(HtmlTableRow &aTableRow,
                                                      const QString& aField,
                                                      CTest* aTest,
                                                      const QList<CGexGroupOfFiles *> lColsToReport)
    {
        aTableRow.clear();
        HtmlTableItem lItem;
        lItem.mData = aField;
        lItem.mAttribute = "align=\"center\"";
        aTableRow.append(lItem);

        // Total widh is 100% of the remaining space
        int lRemainingWidth = 100;
        int lRemainingCols  = mGexReport->getGroupsList().count();
        int lColWidth;

        foreach(CGexGroupOfFiles* lGroup, lColsToReport)
        {
            // Compute the width in % of the column
            if (lRemainingCols)
                lColWidth  = lRemainingWidth / lRemainingCols;
            else
                lColWidth = lRemainingWidth;

            lItem.mAttribute = QString("align=\"center\" width=\"%1%%\"").arg(lColWidth);

            // Decrement the col width from the total width left
            lRemainingWidth -= lColWidth;
            --lRemainingCols;

            lItem.mData = "n/a";
            if (!lGroup)
            {
                aTableRow.append(lItem);
                continue;
            }

            CTest* lTest = lGroup->FindTestCell(aTest->lTestNumber, aTest->strTestName, aTest->lPinmapIndex);
            if (!lTest)
            {
                aTableRow.append(lItem);
                continue;
            }

            lTest->UpdateProperties();
            QVariant lFieldValue = lTest->mProperties.value(aField);

            if (lFieldValue.isNull()||!lFieldValue.isValid())
            {
                aTableRow.append(lItem);
                continue;
            }

            QString lStringValue = lFieldValue.toString();
            if (lFieldValue.type() == QVariant::Double)
            {
                double lDoubleValue = lFieldValue.toDouble();
                if ( lDoubleValue == GEX_C_DOUBLE_NAN || lDoubleValue == C_INFINITE ||
                     lDoubleValue == -C_INFINITE || lDoubleValue ==  C_NO_CP_CPK )
                {
                    lStringValue = "n/a";
                }
                else
                {
                    lStringValue=QString::number(lDoubleValue, 'g', 4);
                }
            }

        }

    }



    void Charac1ReportUnit::WriteTableSplit(QList<CGexGroupOfFiles *> lColsToReport,
                                            const QString& aOutputFormat,
                                            const QString& aPrinterType,
                                            unsigned aHTML4FontSize,
                                            float aViewPort,
                                            const QStringList& aCharacFields,
                                            const int aTestCondCount,
                                            CTest * aTest)
    {
        HtmlTableRow lTableRow;
        // Open new table
        if (aOutputFormat=="pdf" && aPrinterType=="HTMLDOC")
        {
            mGexReport->WriteToReportFile(QString("<font size=%1>").arg(aHTML4FontSize) );
            mGexReport->BeginTable(
                        QString("border=1 cellpadding=0 cellspacing=0 align=center width=\"100%%\" ")
                        .toLatin1().data());
        }
        else
        {
            mGexReport->BeginTable(
                        QString("border=1 cellpadding=0 cellspacing=0 align=center width=\"100%%\" style=\"font-size:%1vw\" ")
                        .arg(aViewPort).toLatin1().data());
        }

        if (aCharacFields.isEmpty() == false)
        {
            // If no test conditions, build line without test cond
            if (aTestCondCount==0)
            {
                BuildZeroTestCondTableRow(lTableRow, lColsToReport);
                mGexReport->WriteTableRow(lTableRow);
            }
            // If test conditions, build line with test cond
            else
            {
                for (int lTCdId =0; lTCdId < aTestCondCount; lTCdId++)
                {
                    BuildTestCondTableRow(lTableRow, lTCdId, lColsToReport);
                    mGexReport->WriteTableRow(lTableRow);
                }
            }

            // Build line with charac fields
            foreach(const QString &lField, aCharacFields)
            {
                if (lField.isEmpty() == true) continue;

                BuildCharacFieldsTableRow(lTableRow, lField, aTest, lColsToReport);
                mGexReport->WriteTableRow(lTableRow);
            }
        }

        mGexReport->EndTable();
        if (aOutputFormat =="pdf" && aPrinterType=="HTMLDOC")
        {
            mGexReport->WriteToReportFile("</font>\n");
        }

        mGexReport->EndLine();
    }

    int Charac1ReportUnit::GetMaxColumns()
    {
        int lMaxDiffCol = 0;
        int lRefStringLength = 0;

        for (int lTCdId =0; lTCdId < mGexReport->GetTestConditionsCount(); lTCdId++)
        {
            int lLongestStringForTc = 0;
            int lDiffCol = 0;
            QString lCondValue;
            foreach(CGexGroupOfFiles* lGroup, mGexReport->getGroupsList())
            {
                if (lGroup)
                {
                    if (lCondValue != lGroup->GetTestConditionsValue(mGexReport->GetTestConditions(lTCdId)).toString())
                    {
                        ++lDiffCol;
                        lCondValue = lGroup->GetTestConditionsValue(mGexReport->GetTestConditions(lTCdId)).toString();
                        lLongestStringForTc = qMax(lLongestStringForTc, lCondValue.size());
                    }
                }
            }
            if (lDiffCol > lMaxDiffCol)
            {
                lMaxDiffCol = lDiffCol;
                lRefStringLength = lLongestStringForTc;
            }
        }
        int lMaxChar = 100;

        return lMaxChar/lRefStringLength;
    }

    QString Charac1ReportUnit::ComputeBoxWhiskerChart(CTest* test, QString& image_path, int &bottomMargin, int &leftMargin)
    {
        // Build BoxWhisker Characterization chart
        image_path = mGexReport->getReportOptions()->strReportDirectory;
        image_path += "/images/";
        image_path += mGexReport->
            BuildImageUniqueName(mGexReport->
                                 getReportOptions()->strReportDirectory +
                                 "/images/adv_charac1_boxwhisker", test);

        // Create Box-whisker chart
        GexCharacBoxWhiskerChart boxWhiskerChart(GEX_CHARTSIZE_AUTO, NULL);
        boxWhiskerChart.setViewportModeFromChartMode(mGexReport->getReportOptions()->getAdvancedReportSettings());
        if (boxWhiskerChart.computeData(mGexReport->getReportOptions(), test) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to compute data for test %1")
                  .arg( test->lTestNumber).toLatin1().constData());

            return QString("error : Failed to compute data for test %1").arg(test->lTestNumber);
        }

        boxWhiskerChart.buildChart(5 * GetChartWidth()/6, GEX_CHARTSIZE_LARGE_Y);

        if (boxWhiskerChart.drawChart(image_path, mGexReport->GetImageCopyrightString()) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to draw box whisker characterization chart for test %1")
                  .arg( test->lTestNumber).toLatin1().constData());

            return QString("error : Failed to draw box whisker characterization chart for test %1")
                    .arg(test->lTestNumber);
        }

        // Format image path for html
        image_path = formatHtmlImageFilename(image_path);

        // Get the bottom margin computed.
        // Will be used to align the histo chart with the box whisker
        bottomMargin = boxWhiskerChart.bottomMargin();

        // Get the left margin computed.
        // Will be used to align statistics table with the box whisker
        leftMargin = boxWhiskerChart.leftMargin();

        // move path to relative
        image_path.replace(0, mGexReport->getReportOptions()->strReportDirectory.size(), "..");

        return "ok";
    }

    QString Charac1ReportUnit::ComputeHistoChart(CTest *test,
                                                 QString &image_path,
                                                 int bottomMargin)
    {
        // Build Histo Characterization chart
        image_path = mGexReport->getReportOptions()->strReportDirectory;
        image_path += "/images/";
        image_path += mGexReport->
            BuildImageUniqueName(mGexReport->
                                 getReportOptions()->strReportDirectory +
                                 "/images/adv_charac1_histo", test);

        // Create Histo chart
        GexCharacHistoChart histoChart(GEX_CHARTSIZE_AUTO, 0, NULL);
        histoChart.setViewportModeFromChartMode(mGexReport->getReportOptions()->getAdvancedReportSettings());
        histoChart.ForceBottomMargin(bottomMargin);

        if (histoChart.computeData(mGexReport->getReportOptions(), test) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to compute data for test %1")
                  .arg( test->lTestNumber).toLatin1().constData());

            return QString("error : Failed to compute data for test %1").arg(test->lTestNumber);
        }

        histoChart.buildChart(GetChartWidth()/6, GEX_CHARTSIZE_LARGE_Y);

        if (histoChart.drawChart(image_path, "") == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to draw histo characterization chart for test %1")
                  .arg( test->lTestNumber).toLatin1().constData());

            return QString("error : Failed to draw histo characterization chart for test %1")
                    .arg(test->lTestNumber);
        }

        // Format image path for html
        image_path = formatHtmlImageFilename(image_path);

        // move path to relative
        image_path.replace(0, mGexReport->getReportOptions()->strReportDirectory.size(), "..");

        return "ok";
    }
} // namespace Gex
} // namespace GS

