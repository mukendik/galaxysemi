#include "ft_pat_report_unit.h"
#include "ft_pat_report_unit_private.h"
#include "product_info.h"
#include "gqtl_log.h"
#include "gex_report.h"
#include "engine.h"
#include "browser.h"

namespace GS
{
namespace Gex
{

FTPATReportUnit::FTPATReportUnit(CGexReport * gr, const QString &cslkey)
    : ReportUnit(gr, cslkey), mPrivate(new FTPATReportUnitPrivate(this))
{

}

FTPATReportUnit::~FTPATReportUnit()
{
}

QString FTPATReportUnit::PrepareSection(bool bValidSection)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("ValidSection = %1").arg(bValidSection ? "true":"false").toLatin1().constData());

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

QString FTPATReportUnit::CreatePages()
{
    // Enable/disable some features...
    if (GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        fprintf(mGexReport->getReportFile(), "*ERROR* %s",
                ReportOptions.GetOption("messages", "upgrade").toString().toLatin1().data());

        return "ok";
    }

    if (mPrivate->Init())
    {
        if (mPrivate->CreateHeaderPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();

        if (mPrivate->CreatePartsFailingPATLimitsPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();

        if (mPrivate->CreateSPATTestsLimitsPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();

        if (mPrivate->CreateDPATResultsPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();

        if (mPrivate->CreateDPATTestsLimitsPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();

        if (mPrivate->CreateTestsFailingPATLimitsPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();

        if (mPrivate->CreateWarningLogsPage() == false)
            return "Error : " + mPrivate->GetErrorMessage();
    }
    else
    {
        fprintf(mGexReport->getReportFile(),
                "*ERROR* %s", mPrivate->GetErrorMessage().toLatin1().constData());
    }

    return "ok"; //mPrivate->CreatePages();
}

QString FTPATReportUnit::CloseSection()
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
            if((mGexReport->getReportOptions()->iHtmlSectionsToSkip & GEX_HTMLSECTION_ADVANCED) == 0)
            {
                // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
                // Writes HTML footer Application name/web site string
                if (lOutputFormat != "HTML")
                {
                    fprintf(mGexReport->getReportFile(), C_HTML_FOOTER,
                            Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
                }

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


}
}
