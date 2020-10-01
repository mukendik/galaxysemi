#include <QString>
#include "zcsl.hpp"
#include "../scripting_io.h"
#include "../report_options.h"
#include <gqtl_log.h>

extern float m_fCslVersion;			// Csl version
extern CReportOptions	ReportOptions;	// Holds options (report_build.h)

ZString gexOptions_v2(ZCsl* csl)
{
    if (!csl)
        return 0;
    const char *	szSection	= csl->get("section").constBuffer();	// Section name to setup
    const char *	szField		= csl->get("field").constBuffer();		// Field name to setup
    const char *	szValue		= csl->get("value").constBuffer();		// Field value

    QString strMessage;
    bool bGexOptionRslt = false;
    strMessage = QString("gexOptions('%1','%2','%3') using csl version %4 ")
                 .arg(szSection).arg(szField).arg(szValue).arg(m_fCslVersion);

    if(!qstricmp(szSection,"output"))
    {
        if(!qstricmp(szField,"format"))
        {
            // Set report output format: HTML
            if(!qstricmp(szValue,"html"))
            {
                bGexOptionRslt = ReportOptions.SetOption("output","format", "HTML");
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Microsoft Word
                if( (!qstricmp(szValue,"word")) || (!qstricmp(szValue,"doc")))
                {
                #if defined unix || __MACH__		// Set report output format= MS-Word
                    bGexOptionRslt = ReportOptions.SetOption("output","format", "PDF");		// MS-Word not available under unix : let s jump to ODT !
                #else
                    bGexOptionRslt = ReportOptions.SetOption("output","format", "DOC");
                #endif
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Powerpoint
                if(!qstricmp(szValue,"ppt"))
                {
                    #if defined unix || __MACH__
                        // Set report output format= PowerPoint
                        // PowerPoint not available under unix!
                        GSLOG(SYSLOG_SEV_NOTICE, "ToDo : swap from PPT to ODP format ");
                        bGexOptionRslt = ReportOptions.SetOption("output","format", "PDF"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;
                    #else
                        bGexOptionRslt = ReportOptions.SetOption("output","format", "PPT"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PPT;
                    #endif
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                if(!qstricmp(szValue,"csv") || !qstricmp(szValue,"excel"))
                {
                // Set report output format= HTML
                bGexOptionRslt = ReportOptions.SetOption("output","format", "CSV");
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Adobe PDF
                if(!qstricmp(szValue,"pdf"))
                {
                    // Set report output format= Adobe PDF
                    bGexOptionRslt = ReportOptions.SetOption("output","format", "PDF");
                    return (bGexOptionRslt)?1:0;	// No error.
                }
            else
                // Set report output format: Adobe PDF
                if(!qstricmp(szValue,"interactive"))
                {
                    // No report to create, only interactive mode!
                    bGexOptionRslt = ReportOptions.SetOption("output","format", "INTERACTIVE");
                    return (bGexOptionRslt)?1:0;	// No error.
                }
            else
                if(!qstricmp(szValue,"ODT"))
                {
                    bGexOptionRslt = ReportOptions.SetOption("output","format", "ODT");
                    return (bGexOptionRslt)?1:0;
                }

            GSLOG(SYSLOG_SEV_WARNING, QString("gexOptions : unknown output format '%1' !").arg(szValue).toLatin1().data() );
            WriteScriptMessage(QString("error : unknown output format '%1' !").arg(szValue), true);
            return (bGexOptionRslt)?1:0;
        }
    } // output

    if(!qstricmp(szSection,"pareto"))
    {
        if(!qstricmp(szField,"section"))
        {
            // Pareto sections to list (bits power of 2)
            long iParetoSection;
            int nScanfRslt;


            if(!qstricmp(szValue,"enabled"))
            {
                // Include section in report
                bGexOptionRslt = ReportOptions.SetOption(QString("pareto"), QString("section"), QString("cp|cpk|failures|failure_signature|soft_bin|hard_bin"));
                return (bGexOptionRslt)?1:0;
            }
            if(!qstricmp(szValue,"disabled"))
            {
                // Do NOT nclude section in report
                bGexOptionRslt = ReportOptions.SetOption(QString("pareto"), QString("section"), QString(""));
                return (bGexOptionRslt)?1:0;
            }

            nScanfRslt = sscanf(szValue,"%ld",&iParetoSection);
            if(nScanfRslt!=1)
                return (bGexOptionRslt)?1:0;
            else
            {
                if(iParetoSection&1)
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cp"), true);
                else
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cp"), false);

                if(iParetoSection&2)
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cpk"), true);
                else
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cpk"), false);

                if(iParetoSection&4)
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failures"), true);
                else
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failures"), false);

                if(iParetoSection&8)
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failure_signature"), true);
                else
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failure_signature"), false);

                if(iParetoSection&16)
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("soft_bin"), true);
                else
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("soft_bin"), false);

                if(iParetoSection&32)
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("hard_bin"), true);
                else
                    bGexOptionRslt &= ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("hard_bin"), false);

                return (bGexOptionRslt)?1:0;
            }
        }
    }

    if(!qstricmp(szSection,"multi_parametric"))
    {
        if(!qstricmp(szField,"option"))
        {
            QString strSection("dataprocessing"), strField("multi_parametric_merge_option"), strValue(szValue);
            if(strValue == QString("keepOne"))
                strValue = QString("keep_one");

            bGexOptionRslt = ReportOptions.SetOption(strSection, strField, strValue);
            return (bGexOptionRslt)?1:0;
        }
        if(!qstricmp(szField,"criteria"))
        {
            QString strSection("dataprocessing"), strField("multi_parametric_merge_criteria"), strValue(szValue);

            bGexOptionRslt = ReportOptions.SetOption(strSection, strField, strValue);
            return (bGexOptionRslt)?1:0;
        }
    }

    if(!qstricmp(szSection,"databases"))
    {
        //GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());
        if(!qstricmp(szField,"local_path"))
        {
            // Set Database local path
            if (QString(szValue)=="(default)" )
                return ReportOptions.SetOption("databases","local_path","default");
            else
                return ReportOptions.SetOption("databases","local_path", szValue); //ReportOptions.strDatabasesLocalPath = szValue;
        }

        if(!qstricmp(szField,"server_path"))
        {
            if (QString(szValue)=="(default)" )
                return ReportOptions.SetOption("databases","server_path","default");
            else
                return ReportOptions.SetOption("databases","server_path", szValue); //ReportOptions.strDatabasesServerPath = szValue;
        }
    }

    QString strSection(szSection), strField(szField), strValue(szValue);
    bGexOptionRslt = ReportOptions.SetOption(strSection, strField, strValue);

    return (bGexOptionRslt)?1:0;
}



ZString gexOptions_v2_1(ZCsl* csl)
{
    if (!csl)
        return 0;
    const char *	szSection	= csl->get("section").constBuffer();	// Section name to setup
    const char *	szField		= csl->get("field").constBuffer();		// Field name to setup
    const char *	szValue		= csl->get("value").constBuffer();		// Field value

    bool bGexOptionRslt = false;
    QString strMessage;
    strMessage = QString("gexOptions('%1','%2','%3') using csl version %4 ")
                 .arg(szSection).arg(szField).arg(szValue).arg(m_fCslVersion);

    if(!qstricmp(szSection,"output"))
    {
        if(!qstricmp(szField,"format"))
        {
            // Set report output format: HTML
            if(!qstricmp(szValue,"HTML"))
            {
                bGexOptionRslt = ReportOptions.SetOption("output","format", "HTML");
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Microsoft Word
                if((!qstricmp(szValue,"word")) || (!qstricmp(szValue,"DOC")))
                {
#if defined unix || __MACH__			// Set report output format= MS-Word
                bGexOptionRslt = ReportOptions.SetOption("output","format", "PDF");		// MS-Word not available under unix!
#else
                bGexOptionRslt = ReportOptions.SetOption("output","format", "DOC");
#endif
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Powerpoint
                if(!qstricmp(szValue,"PPT"))
                {
#if defined unix || __MACH__				// Set report output format= PowerPoint
                bGexOptionRslt = ReportOptions.SetOption("output","format", "PDF");			// PowerPoint not available under unix!
#else
                bGexOptionRslt = ReportOptions.SetOption("output","format", "PPT");
#endif
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                if(!qstricmp(szValue,"CSV") || !qstricmp(szValue,"excel"))
                {
                bGexOptionRslt = ReportOptions.SetOption("output","format", "CSV");
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Adobe PDF
                if(!qstricmp(szValue,"PDF"))
                {
                // Set report output format= Adobe PDF
                bGexOptionRslt = ReportOptions.SetOption("output","format", "PDF");
                return (bGexOptionRslt)?1:0;	// No error.
            }
            else
                // Set report output format: Adobe PDF
                if(!qstricmp(szValue,"INTERACTIVE"))
                {
                // No report to create, only interactive mode!
                bGexOptionRslt = ReportOptions.SetOption("output","format", "INTERACTIVE");
                return (bGexOptionRslt)?1:0;	// No error.
            }
        }
    } // output

    QString strSection(szSection), strField(szField), strValue(szValue);

    bGexOptionRslt = ReportOptions.SetOption(strSection, strField, strValue);

    if (!bGexOptionRslt)
        WriteScriptMessage(
             QString("Warning : setting option %1 - %2 to %3 failed.").arg(strSection).arg(strField).arg(strValue), true );
    return (bGexOptionRslt)?1:0;
}
