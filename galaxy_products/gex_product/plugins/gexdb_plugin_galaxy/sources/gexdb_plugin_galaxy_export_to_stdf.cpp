// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>

// Standard includes
#include <math.h>

// Qt includes
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QDateTime>
#include <QMap>
#include <QDir>

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>
#include <gqtl_sysutils.h>

bool
GexDbPlugin_Galaxy::BeginStdf(GQTL_STDF::StdfParse& f,
                              GexDbPlugin_Galaxy_SplitlotInfo& clSplitlotInfo,
                              GexDbPlugin_Galaxy_TestFilter& /*clTestFilter*/,
                              const QString& /*strLocalDir*/,
                              const QString& desired_notch)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Begin Stdf ");

    // Write MIR
    if(!WriteMIR(f, clSplitlotInfo))
    {
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteMIR");
        return false;
    }

    // Write SDR
    if(!WriteSDR(f, clSplitlotInfo))
    {
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteSDR ! Aborting...");
        return false;
    }

    // Write PMR
    if(!WritePinMapRecords(f, clSplitlotInfo))
    {
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, "error in WritePinMapRecords ! Aborting...");
        return false;
    }

    GexDbPlugin_Galaxy_WaferInfo		*pclWaferInfo = GetWaferInfo(clSplitlotInfo);
    // Write DTR with gross_die
    if(pclWaferInfo
            && (pclWaferInfo->m_uiGrossDie > 0)
            && (pclWaferInfo->m_uiFlags & FLAG_WAFERINFO_GROSSDIEOVERLOADED))
    {
        GQTL_STDF::Stdf_DTR_V4 clDTR;
        QString strUserTxt = "<cmd> gross_die=" + QString::number(pclWaferInfo->m_uiGrossDie);
        // Set DTR fields
        clDTR.SetTEXT_DAT(strUserTxt);
        // Write record
        f.WriteRecord(&clDTR);
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, " cant write DTR (WaferInfo problem) !");

	if(pclWaferInfo && !desired_notch.isEmpty() && desired_notch!="from_db")
		pclWaferInfo->m_cWaferFlat=desired_notch.at(0).toLatin1();

    // Write WCR
    if(pclWaferInfo && !WriteWCR(f, pclWaferInfo))
    {
        delete pclWaferInfo;
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteWCR");
        return false;
    }

    // Write WIR
    if(pclWaferInfo && !WriteWIR(f, clSplitlotInfo, pclWaferInfo))
    {
        delete pclWaferInfo;
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteWIR ");
        return false;
    }

    return true;
}

bool
GexDbPlugin_Galaxy::EndStdf(GQTL_STDF::StdfParse& f,
                            GexDbPlugin_Galaxy_SplitlotInfo& clSplitlotInfo, GexDbPlugin_Filter &cFilters,
                            GexDbPlugin_Galaxy_TestFilter& /*clTestFilter*/,
                            const QString& /*strLocalDir*/,
                            const QString& desired_notch)
{
    GSLOG(SYSLOG_SEV_DEBUG, "End stdf...");

	// Write WRR
	GexDbPlugin_Galaxy_WaferInfo		*pclWaferInfo = GetWaferInfo(clSplitlotInfo);
	// check me : Dont we have to build WaferInfo from the first splitlot ?
	if(pclWaferInfo && !desired_notch.isEmpty() && desired_notch!="from_db")
		pclWaferInfo->m_cWaferFlat=desired_notch.at(0).toLatin1();

    if (pclWaferInfo && !WriteWRR(f, clSplitlotInfo, pclWaferInfo))
    {
        delete pclWaferInfo;
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WriteWRR ! Aborting...");
        return false;
    }


    // BG 27 Mai 2011: Hack to write PAT DTR records for last splitlot
    // (to ensure ST PAT URD with consolidated extraction contain PAT report data)
    // Write PAT DTRs
    if((m_strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST) && !WritePatDtrRecords(f, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, " error in WritePatDtrRecords ! Aborting...");
        return false;
    }

    // Write summary records : No summary records when consolidated wafer ?
    // 4343 : if any filters on run table, do not write SummaryRecords
    bool any_run_filters=false;
// To restore to disable case 6408
    /*
    QString res=cFilters.ContainsFilterOnTable( m_strTablePrefix + "run", (GexDbPlugin_Base*)this, any_run_filters);
    if (res.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cannot check metadata").arg( res).toLatin1().constData()));
        return false;
    }
    */
    if((m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST) && !WriteSummaryRecords(f, clSplitlotInfo, any_run_filters,cFilters))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteSummaryRecords");
        return false;
    }

    // Write MRR
    if(!WriteMRR(f, clSplitlotInfo))
    {
        if(pclWaferInfo)
            delete pclWaferInfo;
        f.Close();
        GSLOG(SYSLOG_SEV_ERROR, "error in WriteMRR ! Aborting...");
        return false;
    }

    return true;
}
