#include "ws_pat_report_unit.h"
#include "gex_report.h"
#include "gqtl_log.h"
#include "engine.h"
#include "browser.h"
#include "pat_info.h"
#include "pat_definition.h"
#include "product_info.h"

extern bool ComparePatResultScore(CPatDefinition* test1, CPatDefinition* test2);

namespace GS
{
namespace Gex
{

WSPATReportUnit::WSPATReportUnit(CGexReport * gr, const QString &cslkey)
    : ReportUnit(gr, cslkey) // , mPrivate(NULL)
{

}

WSPATReportUnit::~WSPATReportUnit()
{
}

QString WSPATReportUnit::PrepareSection(bool bValidSection)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("ValidSection = %1").arg(bValidSection ? "true":"false")
          .toLatin1().constData());

    QString lOutputFormat = mGexReport->GetOption("output", "format").toString();

    // Displays PAT-Man report created into STDF file
    if (lOutputFormat == "CSV")
    {
        // Generating .CSV report file.
        fprintf(mGexReport->getReportFile(),"\n\nPAT-Man History: Outliers report\n");
    }
    else if (mGexReport->getReportOptions()->isReportOutputHtmlBased())
    {
        // Generating HTML report file.
        // Open advanced.htm file
        QString lReturn = OpenFile();

        if (lReturn.startsWith("error"))
            return "error : cant open file : " + lReturn;

        mGexReport->WriteHeaderHTML(mGexReport->getReportFile(), "#000000"); // Default: Text is Black

        // Title + bookmark
        WriteHtmlSectionTitle(mGexReport->getReportFile(), "all_advanced",
                              "More Reports: PAT-Man History (Outliers report)");
    }

    return "ok";
}

QString WSPATReportUnit::CreatePages()
{
    // Enable/disable some features...
    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        fprintf(mGexReport->getReportFile(), "*ERROR* %s",
                ReportOptions.GetOption("messages", "upgrade").toString().toLatin1().data());

        return "ok";
    }

    // Check if Pareto of Failing PAT rules available
    long                            lTotalPartsProcessed    = 0;
    QString                         lTraceability;
    QString                         lOutputFormat   = mGexReport->GetOption("output", "format").toString();
    CPatDefinition *                lPatDef         = NULL;
    QList<CPatDefinition*>			lPatFamilyFailures;
    QList<CPatDefinition*>			lPatRuleFailures;

    // If dataset is over multiple wafers, ensure we remove any per/wafer low-level PAT report!
    if(mGexReport->getGroupsList().count() > 0 &&
       mGexReport->getGroupsList().first()->pFilesList.count() > 1)
        lTraceability = "";
    else
        lTraceability = mGexReport->strPatTraceabilityReport;

    if (FindPATFailuresPareto(lPatRuleFailures, lPatFamilyFailures, lTotalPartsProcessed))
    {
        double  lYieldLoss;
        QString lPatFamily;
        QString lRuleName;
        QString lPATYieldLoss;

        // Write lines
        if(lPatFamilyFailures.count() > 0)
        {
            if (lOutputFormat != "CSV")
            {
                lTraceability += "<h1><font color=\"#006699\">PAT FAMILY Failures Pareto:</h1><br>";
                lTraceability += "<table border=\"1\" width=\"450\">\n";
                lTraceability += "<tr>\n<td width=\"150\" bgcolor=\"#CCECFF\"><b>PAT Family</b></td>\n<td align=\"center\" width=\"150\" bgcolor=\"#CCECFF\"><b>Fail Count</b>\n</td>\n<td align=\"center\" width=\"150\" bgcolor=\"#CCECFF\"><b>Yield Loss %</b>\n</td></tr>\n";
            }
            else
            {
                lTraceability += "PAT FAMILY Failures Pareto:\n";
                lTraceability += "PAT Family,Fail count,Yield Loss %\n";
            }
        }

        foreach(lPatDef, lPatFamilyFailures)
        {
            // Compute Yield Loss %
            if(lTotalPartsProcessed)
            {
                lYieldLoss      = (100.00*(double)lPatDef->m_lSeverityScore) / (double)lTotalPartsProcessed;
                lPATYieldLoss   = QString::number(lYieldLoss,'f',2) + " %";
            }
            else
                lPATYieldLoss = "-";

            if (lOutputFormat != "CSV")
            {
                lTraceability += "<tr>\n<td width=\"150\">" + lPatDef->m_strTestName + "</td>\n";
                lTraceability += "<td align=\"center\" width=\"150\">" +
                                 QString::number((int)lPatDef->m_lSeverityScore) + "</td>\n";
                lTraceability += "<td align=\"center\" width=\"150\">" + lPATYieldLoss + "</td>\n";
                lTraceability += "</tr>\n";
            }
            else
                lTraceability += lPatDef->m_strTestName + "," +
                                 QString::number((int)lPatDef->m_lSeverityScore) + ","
                                 + lPATYieldLoss + "\n" ;
        }
        if (lOutputFormat != "CSV")
            lTraceability += "</table>\n";

        // Write lines
        if(lPatRuleFailures.count() > 0)
        {
            if (lOutputFormat != "CSV")
            {
                lTraceability += "<h1><font color=\"#006699\">PAT RULES Failures Pareto:</h1><br>";
                lTraceability += "<table border=\"1\" width=\"450\">\n";
                lTraceability += "<tr>\n<td width=\"150\" bgcolor=\"#CCECFF\"><b>PAT Family</b></td>\n<td width=\"150\" bgcolor=\"#CCECFF\"><b>Rule name</b></td>\n<td align=\"center\" width=\"150\" bgcolor=\"#CCECFF\"><b>Fail Count</b>\n</td><td align=\"center\" width=\"150\" bgcolor=\"#CCECFF\"><b>Yield Loss %</b>\n</td></tr>\n";
            }
            else
            {
                lTraceability += "PAT RULES Failures Pareto:\n";
                lTraceability += "PAT Family,Rule name,Fail count,Yield Loss %\n";
            }
        }

        foreach(lPatDef, lPatRuleFailures)
        {
            // Compute Yield Loss %
            if(lTotalPartsProcessed)
            {
                lYieldLoss      = (100.00*(double)lPatDef->m_lSeverityScore)/ (double)lTotalPartsProcessed;
                lPATYieldLoss   = QString::number(lYieldLoss,'f',2) + " %";
            }
            else
                lPATYieldLoss = "-";

            lPatFamily  = lPatDef->m_strTestName.section(":",0,0);
            lRuleName   = lPatDef->m_strTestName.section(":",1);

            if (lOutputFormat != "CSV")
            {
                lTraceability += "<tr>\n<td width=\"150\">" + lPatFamily + "</td>\n";
                lTraceability += "<td width=\"150\">" + lRuleName + "</td>\n";
                lTraceability += "<td align=\"center\" width=\"150\">" + QString::number((int)lPatDef->m_lSeverityScore) + "</td>\n";
                lTraceability += "<td align=\"center\" width=\"150\">" + lPATYieldLoss + "</td>\n";
                lTraceability += "</tr>\n";
            }
            else
                lTraceability += lPatFamily + "," + lRuleName + "," +
                                 QString::number((int)lPatDef->m_lSeverityScore) + "," +
                                 lPATYieldLoss + "\n" ;
        }
        if (lOutputFormat != "CSV")
            lTraceability += "</table>\n";
    }

    if(lTraceability.isEmpty())
    {
        fprintf(mGexReport->getReportFile(), "No PAT-Man report found in the data analyzed.\n");
    }
    else
    {
        // Check if need to convert CR-LF to HTML <br>
        if (lOutputFormat != "CSV")
        {
            // replace CR-LF with <br> if the string doesn't hold any <br> already!
            if(lTraceability.indexOf("<br>") < 0)
                lTraceability.replace("\r","<br>");
        }

        fprintf(mGexReport->getReportFile(), "%s", lTraceability.toLatin1().constData());
    }

    // Clear liste of pointor
    qDeleteAll(lPatFamilyFailures);
    qDeleteAll(lPatRuleFailures);

    lPatFamilyFailures.clear();
    lPatRuleFailures.clear();

    return "ok";
}

QString WSPATReportUnit::CloseSection()
{
    QString lOutputFormat = mGexReport->GetOption("output", "format").toString();

    if (lOutputFormat == "CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if (mGexReport->getReportOptions()->isReportOutputHtmlBased())
    {
        // Close last Histogram page created...so we can now build the index.
        if(mGexReport->getReportFile())
        {
            // close page...if this section was created.
            if((mGexReport->getReportOptions()->iHtmlSectionsToSkip & GEX_HTMLSECTION_ADVANCED) == 0 &&
               lOutputFormat == "HTML")
            {
                // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
                // Writes HTML footer Application name/web site string
                fprintf(mGexReport->getReportFile(), C_HTML_FOOTER,
                        Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
                fprintf(mGexReport->getReportFile(),"</body>\n");
                fprintf(mGexReport->getReportFile(),"</html>\n");
                CloseReportFile();	// Close report file
            }

            // Write page break (ignored if not writing a flat HTML document)
            mGexReport->WritePageBreak();
        }
    }

    return "ok";
}

bool WSPATReportUnit::FindPATFailuresPareto(QList<CPatDefinition*> &lRuleFailures,
                                            QList<CPatDefinition*> &lFamilyFailures,
                                            long& lTotalPartsProcessed)
{
    QMap <QString,long>             lPatRulesFailures;
    QMap <QString,long>             lPatFamiliesFailures;
    QMap <QString,long>::Iterator	itPatFailures;
    CGexGroupOfFiles *              lGroup      = NULL;
    CGexFileInGroup *               lFile       = NULL;
    CPatDefinition *                lPatDef     = NULL;

    lGroup = mGexReport->getGroupsList().isEmpty() ? NULL : mGexReport->getGroupsList().first();
    if(lGroup && (lGroup->pFilesList.count() > 0))
    {
        for(int lIdx = 0; lIdx < lGroup->pFilesList.count(); ++lIdx)
        {
            QString lTempString;

            lFile = lGroup->pFilesList.at(lIdx);

            // Keep track of total parts processed (so to compute Yield Loss %)
            lTotalPartsProcessed += lFile->getWaferMapData().iTotalPhysicalDies;

            for(int lIdx = 0; lIdx < lFile->m_strPart_TXT.count(); ++lIdx)
            {
                // Extract PAT Fail info from Part text (if any)
                // format <Family> : <Rule>
                lTempString = lFile->m_strPart_TXT.at(lIdx);
                lTempString = lTempString.section("gexPatFail: ",1);

                // eg: 'Cluster : Process Clustering'
                if(lTempString.isEmpty() == false)
                {
                    if(lPatRulesFailures.find(lTempString) == lPatRulesFailures.end())
                        lPatRulesFailures[lTempString] = 1;
                    else
                        lPatRulesFailures[lTempString] = lPatRulesFailures[lTempString] + 1;

                    // Extract Family
                    lTempString = lTempString.section(':',0,0);
                    if(lPatFamiliesFailures.find(lTempString) == lPatFamiliesFailures.end())
                        lPatFamiliesFailures[lTempString] = 1;
                    else
                        lPatFamiliesFailures[lTempString] = lPatFamiliesFailures[lTempString] + 1;
                }
            }
        }

        // Scan summary of PAT failure Families:
        for(itPatFailures = lPatFamiliesFailures.begin(); itPatFailures != lPatFamiliesFailures.end();
            ++itPatFailures )
        {
            lPatDef = new CPatDefinition;
            lPatDef->m_strTestName     = itPatFailures.key();		// PAT failure name Family name (eg: GDBN, Cluster, etc)
            lPatDef->m_lSeverityScore  = itPatFailures.value();		// PAT failure count

            lFamilyFailures.append(lPatDef);
        }

        // Sort results in descending order
        qSort(lFamilyFailures.begin(), lFamilyFailures.end(), ComparePatResultScore);

        // Scan summary of PAT RULES failures
        for(itPatFailures = lPatRulesFailures.begin(); itPatFailures != lPatRulesFailures.end();
            ++itPatFailures )
        {
            lPatDef = new CPatDefinition;
            lPatDef->m_strTestName     = itPatFailures.key();		// PAT failure name
            lPatDef->m_lSeverityScore  = itPatFailures.value();		// PAT failure count

            lRuleFailures.append(lPatDef);
        }

        // Sort results in descending order
        qSort(lRuleFailures.begin(), lRuleFailures.end(), ComparePatResultScore);

        return true;
    }

    return false;
}

}
}
