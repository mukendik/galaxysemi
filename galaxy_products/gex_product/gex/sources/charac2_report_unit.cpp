///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "charac2_report_unit.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "gex_report.h"
#include "gex_group_of_files.h"
#include "gex_charac_line_chart.h"
#include "engine.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QLabel>

///////////////////////////////////////////////////////////////////////////////////
// External functions or object
///////////////////////////////////////////////////////////////////////////////////
extern bool     CTestFailCountLessThan(CTest* s1, CTest* s2);
extern QString  formatHtmlImageFilename(const QString& strImageFileName);

namespace GS
{
namespace Gex
{

    Charac2ReportUnit::Charac2ReportUnit(CGexReport* pGexReport, const QString &key)
        : Gex::ReportUnit(pGexReport, key)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Charac2ReportUnit constructor: %1").arg( key).toLatin1().constData());
    }

    Charac2ReportUnit::~Charac2ReportUnit()
    {
       GSLOG(SYSLOG_SEV_DEBUG, "Charac2ReportUnit destructor...");
    }

    QString Charac2ReportUnit::PrepareSection(bool lValidSection)
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("ValidSection = %1").arg( lValidSection?"true":"false").toLatin1().constData());

        QString lOutputFormat         = mGexReport->GetOption("output", "format").toString();
        QString lOptionStorageDevice  = (mGexReport->GetOption("statistics","generic_galaxy_tests")).toString();

        if (lOutputFormat == "CSV")
        {
            // Generating .CSV report file.
            fprintf(mGexReport->getReportFile(), "\n\nAdvanced Characterization Report\n");
        }
        else if (mGexReport->getReportOptions()->isReportOutputHtmlBased())
        {
            // Generating HTML report file.
            // Open advanced.htm file
            if(OpenFile().startsWith("error") )
                return "error : cant open file";

            // Default: Text is Black
            mGexReport->WriteHeaderHTML(mGexReport->getReportFile(),"#000000");

            // Title + bookmark
            WriteHtmlSectionTitle(mGexReport->getReportFile(),
                                  "all_advanced","More Reports: Advanced Characterization Report");
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
                      QString(" will consider only top %1 failed tests ")
                      .arg( topNFail).toLatin1().constData());
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
                    ((ptTestCell->bTestType == '-') && (lOptionStorageDevice == "hide")))
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

    QString Charac2ReportUnit::CreatePages()
    {
        if (!mGexReport)
        {
            return "error : mGexReport null";
        }

        mGexReport->WriteText( "", "a", "all_advanced");
        mGexReport->WriteText(mGexReport->GetOption("adv_charac2", "report_title").toString(),
                             "h1", "style=\"color:#006699;text-align:center\"");
        mGexReport->EndLine();

        // Todo : disable if license not allowed
        if (!GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
          mGexReport->WriteText(ReportOptions.GetOption("messages", "upgrade").toString());
          return "ok";
        }

        if (mGexReport->getGroupsList().size() < 2)
        {
            mGexReport->WriteText(QString("Error : not enough groups : %1.\n").arg(mGexReport->getGroupsList().size()) );

            return "error : not enough group";
        }

        mGexReport->EndLine();

        QString lOutputFormat = mGexReport->GetOption("output", "format").toString().toLower();

        foreach(CTest * ptTestCell, mTestsListToReport)
        {
            GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Computing Test %1...").arg(ptTestCell->lTestNumber));
            QCoreApplication::instance()->QCoreApplication::processEvents();

            ptTestCell->UpdateProperties();

            QStringList reportContent;

            if (lOutputFormat != "csv")
            {
                QString         lImagePath;
                QString         lError = ComputeChart(ptTestCell, lImagePath);

                if (lError.startsWith("error"))
                    lImagePath = lError;

                reportContent.clear();
                reportContent.append(QString("<img src=\"%1\"></img>").arg(lImagePath) );

                mGexReport->BeginTable("border=1 cellpadding=0 cellspacing=0 align=center");

                mGexReport->WriteTableRow(reportContent);

                mGexReport->EndTable();
                mGexReport->EndLine();
            }

            // for all groups
            reportContent.clear();

            QStringList fields_sl = mGexReport->GetOption("adv_charac2", "field").toString().split("|", QString::SkipEmptyParts);

            if (fields_sl.isEmpty() == false)
            {
                const int   tcc   = mGexReport->GetTestConditionsCount();
                const int   ncol  = mGexReport->getGroupsList().size()+1;
                QStringList ls;
                QStringList lAttributes;

                mGexReport->BeginTable("border=1 cellpadding=0 cellspacing=0 align=center");

                if (lOutputFormat == "csv")
                {
                    reportContent.append(QString("Test %1 : %2").arg(ptTestCell->lTestNumber).arg(ptTestCell->strTestName));
                    mGexReport->WriteTableRow(reportContent,
                        QString("style=\"text-align:center;font-weight:bold;\" align=center colspan=%1").arg(ncol));
                }

                // for all groups
                ls.clear();
                lAttributes.clear();

                if (tcc==0)
                {
                    ls.append("<b>Var</b>");
                    lAttributes.append("align=\"center\"");

                    // Total widh is 100% of the remaining space
                    int lTotalWidth     = 100;
                    int lRemainingCols  = mGexReport->getGroupsList().count();
                    int lColWidth;

                    foreach(CGexGroupOfFiles* gof, mGexReport->getGroupsList())
                    {
                        // Compute the width in % of the column
                        if (lRemainingCols)
                            lColWidth   = lTotalWidth / lRemainingCols;
                        else
                            lColWidth   = lTotalWidth;

                        if (gof)
                            ls.append("<b>" + gof->strGroupName + "</b>");
                        else
                           ls.append("<b>n/a</b>");

                        lAttributes.append(QString("align=\"center\" width=\"%1%%\"").arg(lColWidth));

                        // Decrement the col width from the total width left
                        --lRemainingCols;
                        lTotalWidth -= lColWidth;

                    }

                    mGexReport->WriteTableRow(ls, lAttributes);
                }
                else
                {
                    for (int i=0; i<tcc; i++)
                    {
                        ls.clear();
                        lAttributes.clear();

                        ls.append("<b>" + mGexReport->GetTestConditions(i) + "</b>");
                        lAttributes.append("align=\"center\"");

                        // Total widh is 100% of the remaining space
                        int lTotalWidth     = 100;
                        int lRemainingCols  = mGexReport->getGroupsList().count();
                        int lColWidth       = 0;
                        int lColSpan        = 1;
                        QString conditionValue;

                        foreach(CGexGroupOfFiles* gof, mGexReport->getGroupsList())
                        {
                            // Compute the width in % of the column
                            if (lRemainingCols)
                                lColWidth   = lTotalWidth / lRemainingCols;
                            else
                                lColWidth   = lTotalWidth;

                           if (gof)
                           {
                               // Merge cell having same condition value
                               if (conditionValue.isEmpty())
                               {
                                   conditionValue   = gof->GetTestConditionsValue(mGexReport->GetTestConditions(i)).toString();
                                   lColSpan         = 1;
                               }
                               else if (conditionValue != gof->GetTestConditionsValue(mGexReport->GetTestConditions(i)).toString())
                               {
                                   ls.append("<b>" + conditionValue + "</b>");
                                   lAttributes.append(QString("align=\"center\" width=\"%1%%\" colspan=\"%2\"").arg(lColWidth).arg(lColSpan));

                                   conditionValue   = gof->GetTestConditionsValue(mGexReport->GetTestConditions(i)).toString();
                                   lColSpan         = 1;
                               }
                               else
                               {
                                   ++lColSpan;
                               }

                               // Empty condition name, so write it in a single cell
                               if (conditionValue.isEmpty())
                               {
                                   ls.append("<b>n/a</b>");
                                   lAttributes.append(QString("align=\"center\" width=\"%1%%\"").arg(lColWidth));
                                   lColSpan         = 1;
                               }
                           }
                           else
                           {
                               ls.append("<b>n/a</b>");
                               lAttributes.append(QString("align=\"center\" width=\"%1%%\"").arg(lColWidth));
                               lColSpan         = 1;
                           }

                           // Decrement the col width from the total width left
                           lTotalWidth -= lColWidth;
                           --lRemainingCols;
                        }

                        if (conditionValue.isEmpty() == false)
                        {
                            ls.append("<b>" + conditionValue + "</b>");
                            lAttributes.append(QString("align=\"center\" width=\"%1%%\" colspan=\"%2\"").arg(lColWidth).arg(lColSpan));
                        }

                        mGexReport->WriteTableRow(ls, lAttributes);
                    }
                }

                foreach(const QString &o, fields_sl)
                {
                    if (o.isEmpty() == false)
                    {
                        ls.clear();
                        lAttributes.clear();

                        ls.append(o);
                        lAttributes.append("align=\"center\"");

                        // Total widh is 100% of the remaining space
                        int lTotalWidth     = 100;
                        int lRemainingCols  = mGexReport->getGroupsList().count();
                        int lColWidth;

                        foreach(CGexGroupOfFiles* gof, mGexReport->getGroupsList())
                        {
                            // Compute the width in % of the column
                            if (lRemainingCols)
                                lColWidth   = lTotalWidth / lRemainingCols;
                            else
                                lColWidth   = lTotalWidth;

                            lAttributes.append(QString("align=\"center\" width=\"%1%%\"").arg(lColWidth));

                            // Decrement the col width from the total width left
                            lTotalWidth -= lColWidth;
                            --lRemainingCols;

                            if (!gof)
                            {
                                ls.append( "n/a" );
                                continue;
                            }

                            CTest* test=gof->FindTestCell(ptTestCell->lTestNumber, ptTestCell->strTestName, ptTestCell->lPinmapIndex);
                            if (!test)
                            {
                                ls.append( "n/a" );
                               continue;
                            }

                            test->UpdateProperties();
                            QVariant v = test->mProperties.value(o);

                            if (v.isNull()||!v.isValid())
                            {
                                ls.append( "n/a" );
                                continue;
                            }

                            QString s=v.toString();
                            if (v.type()==QVariant::Double)
                            {
                                double d=v.toDouble();
                                if ( d == GEX_C_DOUBLE_NAN || d == C_INFINITE || d == -C_INFINITE || d ==  C_NO_CP_CPK )
                                {
                                    s = "n/a";
                                }
                                else
                                    s=QString::number(d, 'g', 4);
                            }

                            ls.append(s.isEmpty()?"n/a":s );
                        }

                        mGexReport->WriteTableRow(ls, lAttributes);
                    }
                }

                mGexReport->EndTable();
                mGexReport->EndLine();
            }

            mGexReport->WritePageBreak();
        }

        mGexReport->EndLine();

        return "ok";
    }

    QString Charac2ReportUnit::CloseSection()
    {
        QString outputFormat = mGexReport->GetOption("output", "format").toString();

        if(outputFormat == "CSV")
        {
            // Generating .CSV report file: no specific task to perform when section written !
        }
        else if (mGexReport->getReportOptions()->isReportOutputHtmlBased())
        {
            if (outputFormat == "HTML")
                CloseReportFile();	// Close report file
        }

        return "ok";
    }

    int Charac2ReportUnit::GetChartWidth() const
    {
        QString lOutputFormat   = mGexReport->GetOption("output", "format").toString();
        QString lPaperFormat    = mGexReport->GetOption("output", "paper_format").toString();
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

    QString Charac2ReportUnit::ComputeChart(CTest * pTest, QString & lImagePath)
    {
        // Build BoxWhisker Characterization chart
        lImagePath = mGexReport->getReportOptions()->strReportDirectory;
        lImagePath += "/images/";
        lImagePath += mGexReport->
            BuildImageUniqueName(mGexReport->
                                 getReportOptions()->strReportDirectory +
                                 "/images/adv_charac2_linechart", pTest);

        GS::Gex::CharacLineChart lineChart(GEX_CHARTSIZE_AUTO, NULL);

//        lineChart.SetSerieVariables(QStringList(""));

        lineChart.setViewportModeFromChartMode(mGexReport->getReportOptions()->getAdvancedReportSettings());
        if (lineChart.computeData(mGexReport->getReportOptions(), pTest) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to compute data for test %1")
                  .arg( pTest->lTestNumber).toLatin1().constData());

            return QString("error : Failed to compute data for test %1").arg(pTest->lTestNumber);
        }

        lineChart.buildChart(5 * GetChartWidth()/6, GEX_CHARTSIZE_LARGE_Y);

        if (lineChart.drawChart(lImagePath, mGexReport->GetImageCopyrightString()) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to draw box whisker characterization chart for test %1")
                  .arg( pTest->lTestNumber).toLatin1().constData());

            return QString("error : Failed to draw box whisker characterization chart for test %1").arg(pTest->lTestNumber);
        }

        // Format image path for html
        lImagePath = formatHtmlImageFilename(lImagePath);

        // move path to relative
        lImagePath.replace(0, mGexReport->getReportOptions()->strReportDirectory.size(), "..");

        return "ok";
    }

} // namespace Gex
} // namespace GS
