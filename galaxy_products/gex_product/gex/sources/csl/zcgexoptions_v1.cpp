#include <QString>
#include "zcsl.hpp"
#include "../scripting_io.h"
#include "../report_options.h"
#include "../report_build.h"
#include "../interactive_charts.h"		// Layer Style class.
#include <gqtl_log.h>
#include "product_info.h"

extern float m_fCslVersion;
extern CReportOptions	ReportOptions;	// Holds options (report_build.h)
extern CGexSingleChart *s_pChartStyle;
extern CBinColor		cBinColor;			// Holds the Bin# and color.

extern ZString gexChartStyle(const char * szField, const char * szValue);
extern ZString gexBinStyle(const char * szField, const char * szValue);
extern ZString gexSiteStyle(const char * szField, const char * szValue);

/**********************************************************************************************
**	bool addFlagOption(const char* szSection, const char* szField, const char* szValue)
**
**	add szValue option to the flag list focused by (szSection, szField) in ReportOptions
**	check if option already in the list
**********************************************************************************************/
bool addFlagOption(const char* szSection, const char* szField, const char* szValue)
{
    QString strSection(szSection), strField(szField), strValue(szValue);

    QString qstrVolumeMarkerOptions = (ReportOptions.GetOption(strSection, strField)).toString();
    QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));

    if(qstrlVolumeMarkerOptionList.contains(strValue))
        return true;	// nothing to do
    else
    {
        QString qstrNewOptionValue = QString("");

        if(qstrVolumeMarkerOptions.isEmpty())
            qstrNewOptionValue = strValue;
        else
            qstrNewOptionValue = qstrVolumeMarkerOptions + QString("|") + strValue;	// add option to the flag list

        ReportOptions.SetOption(strSection, strField, qstrNewOptionValue);		// replace old list
        return true;
    }

}

/**********************************************************************************************
**	void removeFlagOption(const char* szSection, const char* szField, const char* szValue)
**
**	remove szValue option to the flag list focused by (szSection, szField) in ReportOptions
**********************************************************************************************/
//void removeFlagOption(const char* szSection, const char* szField, const char* szValue)
//{
//	QString strSection(szSection), strField(szField), strValue(szValue);
//
//	QString qstrVolumeMarkerOptions = (ReportOptions.GetOption(strSection, strField)).toString();
//	QStringList qstrlVolumeMarkerOptionList = qstrVolumeMarkerOptions.split(QString("|"));
//
//	if(qstrlVolumeMarkerOptionList.removeAll(strValue) > 0)
//	{
//		QString qstrNewOptionValue = qstrlVolumeMarkerOptionList.join("|");
//
//		ReportOptions.SetOption(strSection, strField, qstrNewOptionValue);		// replace old list
//	}
//}


ZString gexOptions_v1(ZCsl* csl)
{
    QString			strSetValue="1";	// can be used preset 'value' field
    int				argc		= csl->get("argCount").asInt();
    const char *	szSection	= csl->get("section").constBuffer();	// Section name to setup
    const char *	szField		= csl->get("field").constBuffer();		// Field name to setup
    const char *	szValue		= csl->get("value").constBuffer();		// Field value

    if(argc > 3)
        strSetValue = csl->get("setvalue").constBuffer();

    QString strMessage;
    strMessage = QString("gexOptions('%1','%2','%3')")
                 .arg(szSection).arg(szField).arg(szValue);

    if(!qstricmp(szSection,"reset"))
    {
        // Reset options
        if(qstricmp(szField,"all"))
        {
            strMessage += ": Invalid parameter";
            WriteScriptMessage(strMessage,true);
            return 0;
        }

        if(!qstricmp(szValue,"default"))
        {
            // Reset all options to default
            ReportOptions.Reset(true);
            return 1;	// No error.
        }
        else
            if(!qstricmp(szValue,"clear"))
            {
            // Reset all options to 0!
            ReportOptions.Reset(false);
            return 1;	// No error.
        }
    }

    // DATABASES section
    if(!qstricmp(szSection,"databases"))
    {
        //GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data());
        if(!qstricmp(szField,"local_path"))
        {
            // Set Database local path
            if (QString(szValue)=="(default)" )
                ReportOptions.SetOption("databases","local_path","default");
            else
                ReportOptions.SetOption("databases","local_path", szValue); //ReportOptions.strDatabasesLocalPath = szValue;
            return 1;	// No error.
        }

        if(!qstricmp(szField,"server_path"))
        {
            if (QString(szValue)=="(default)" )
                ReportOptions.SetOption("databases","server_path","default");
            else
                ReportOptions.SetOption("databases","server_path", szValue); //ReportOptions.strDatabasesServerPath = szValue;
            return 1;	// No error.
        }

        if(!qstricmp(szField,"rdb_default_parameters"))
        {
            if(!qstricmp(szValue,"all"))
            {
                ReportOptions.SetOption("databases","rdb_default_parameters","all"); //ReportOptions.iRdbQueryDefaultParameters = GEX_OPTION_RDB_EXTRACT_PARAMETERS_ALL;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"none"))
            {
                ReportOptions.SetOption("databases","rdb_default_parameters","none"); //ReportOptions.iRdbQueryDefaultParameters = GEX_OPTION_RDB_EXTRACT_PARAMETERS_NONE;
                return 1;	// No error.
            }
        }
        //GSLOG(" Warning : unknown 'databases'' option !");
        return 0;
    }

    // OUTPUT section
    if(!qstricmp(szSection,"output"))
    {
        if(!qstricmp(szField,"format"))
        {
            // Set report output format: HTML
            if(!qstricmp(szValue,"html"))
            {
                ReportOptions.SetOption("output","format", "HTML"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_HTML;
                return 1;	// No error.
            }
            else
            // Set report output format: Microsoft Word
            if( (!qstricmp(szValue,"word")) || (!qstricmp(szValue,"doc")))
            {
#if defined unix || __MACH__			// Set report output format= MS-Word
                    ReportOptions.SetOption("output","format", "PDF"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;		// MS-Word not available under unix!
#else
        // case 4768
          ReportOptions.SetOption("output","format", "DOC"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_WORD;
#endif
                    return 1;	// No error.
            }
            else
            // Set report output format: Powerpoint
            if(!qstricmp(szValue,"ppt"))
            {
#if defined unix || __MACH__				// Set report output format= PowerPoint
                ReportOptions.SetOption("output","format", "PDF"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;		// PowerPoint not available under unix!
#else
        ReportOptions.SetOption("output","format", "PPT"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PPT;
#endif
                return 1;	// No error.
            }
            else
            if(!qstricmp(szValue,"csv") || !qstricmp(szValue,"excel"))
            {
                // Set report output format= HTML
                ReportOptions.SetOption("output","format", "CSV"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_CSV;
                return 1;	// No error.
            }
            else
            // Set report output format: Adobe PDF
            if(!qstricmp(szValue,"pdf"))
            {
                // Set report output format= Adobe PDF
                // case 4768
                /*
                if( (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GEX_DATATYPE_ALLOWED_CREDENCE_ASL)
                            || (GS::LPPlugin::ProductInfo::getInstance()->getProductID() ==  GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE)
                            || (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GEX_DATATYPE_ALLOWED_SZ)
                  || (GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GEX_DATATYPE_ALLOWED_LTX) )
                  */
                ReportOptions.SetOption("output","format", "PDF"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;
                //ReportOptions.SetOption("output","format", "HTML"); //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_HTML;	// PDF format is only for the professional Edition or ASL-OEM or SZ-OEM version!
                return 1;	// No error.
            }
            else
            // Set report output format: Adobe PDF
            if(!qstricmp(szValue,"interactive"))
            {
                // No report to create, only interactive mode!
                ReportOptions.SetOption("output","format", "INTERACTIVE");
                //ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_INTERACTIVEONLY;
                return 1;	// No error.
            }

            GSLOG(SYSLOG_SEV_WARNING, QString("gexOptions : unknown output format '%1' !").arg( szValue).toLatin1().constData());
            WriteScriptMessage(QString("warning : unknown output format '%1' !").arg(szValue), true);
            return 1;
        }

        // Home Page text (for Word, PPT and PDF reports)
        /*
        if(!qstricmp(szField,"front_page_text"))
        {
            ReportOptions.strFrontPageText = szValue;
            // Remove empty paragraphs
            ReportOptions.strFrontPageText.replace("<p> </p>","");
            return 1;	// No error.
        }
        */

        // Home Page Image (for Word, PPT and PDF reports)
        /*
        if(!qstricmp(szField,"front_page_image"))
        {
            ReportOptions.strFrontPageImage = szValue;
            return 1;	// No error.
        }
        */


        // Test names truncation (in Word, PDF, PPT reports)
        if(!qstricmp(szField,"truncate_names"))
        {
            if(!qstricmp(szValue,"yes"))
            {
                ReportOptions.SetOption("output", "truncate_names", QString("32"));
                return 1;	// No error.
            }
            else
            if(!qstricmp(szValue,"no"))
            {
//				ReportOptions.iTruncateLabels = -1;	// Do not truncate labels!
                ReportOptions.SetOption("output", "truncate_names", QString("-1"));
                return 1;	// No error.
            }
            else
            {
                // Custom length (eg: 20 characters)
                int nTruncateLabels;
                // if(sscanf(szValue,"%d",&ReportOptions.iTruncateLabels) != 1)
                if(sscanf(szValue,"%d",&nTruncateLabels) != 1)
                {
                    ReportOptions.SetOption("output", "truncate_names", QString("32"));
                    return 1;
                }
                ReportOptions.SetOption("output", "truncate_names", QString::number(nTruncateLabels));
                return 1;	// No error.
            }
        }

        /*
        // Paper size: Letter or A4
        if(!qstricmp(szField,"paper_size"))
        {
            if(!qstricmp(szValue,"Letter"))
            {
                ReportOptions.iPaperSize = GEX_OPTION_PAPER_SIZE_LETTER;
                return 1;	// No error.
            }
            else
            if(!qstricmp(szValue,"A4"))
            {
                ReportOptions.iPaperSize = GEX_OPTION_PAPER_SIZE_A4;
                return 1;	// No error.
            }
        }
        */

        /*
        // Paper format: portrait or landscape
        if(!qstricmp(szField,"paper_format"))
        {
            if(!qstricmp(szValue,"portrait"))
            {
                ReportOptions.bPortraitFormat = true;
                return 1;	// No error.
            }
            else
            if(!qstricmp(szValue,"landscape"))
            {
                ReportOptions.bPortraitFormat = false;
                return 1;	// No error.
            }
        }
        */

        if(!qstricmp(szField,"location"))
        {
            // Set report output location
            //ReportOptions.strOutputPath = szValue;
            if (QString(szValue) == "(default)" || QString(szValue) == "default")
                ReportOptions.SetOption("output","location","default");
            else if (QString(szValue) == "" || QString(szValue) == "same")
                ReportOptions.SetOption("output","location","same");
            else
            {
                ReportOptions.SetOption("output","location","specified");
                ReportOptions.SetOption("output","location_path", QString(szValue));
            }

            return 1;	// No error.
        }

        QString v(szValue);
        QString f(szField);
        if (f=="front_page_text")
            v.replace("<p> </p>","");
        ReportOptions.SetOption("output", f, v);

        return 1;
        // all options are now saved even if not listed in this file.
        // It is useful for anybody to add an otion variable without modifyng CReportOptions, zcgexlib.cpp,...
        // just query ReportOption with GetOption("myvariable") and you will have your option value, "" if none was in the .csl
        //ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Monitoring section
    if(!qstricmp(szSection,"monitoring"))
    {
        // History size section
        /*		CRO_cleanUp
        if(!qstricmp(szField,"history"))
        {
            if(!qstricmp(szValue,"none"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_NONE;
                return 1;
            }
            if(!qstricmp(szValue,"1week"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_1WEEK;
                return 1;
            }
            if(!qstricmp(szValue,"2week"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_2WEEK;
                return 1;
            }
            if(!qstricmp(szValue,"3week"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_3WEEK;
                return 1;
            }
            if(!qstricmp(szValue,"1month"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_1MONTH;
                return 1;
            }
            if(!qstricmp(szValue,"2month"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_2MONTH;
                return 1;
            }
            if(!qstricmp(szValue,"3month"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_3MONTH;
                return 1;
            }
            if(!qstricmp(szValue,"4month"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_4MONTH;
                return 1;
            }
            if(!qstricmp(szValue,"5month"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_5MONTH;
                return 1;
            }
            if(!qstricmp(szValue,"6month"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_6MONTH;
                return 1;
            }
            if(!qstricmp(szValue,"1year"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_1YEAR;
                return 1;
            }
            if(!qstricmp(szValue,"all"))
            {
                ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_ALL;
                return 1;
            }
        }
        */
        // Home page fields
        if(!qstricmp(szField,"home_page"))
        {
            // Flags for fields to list in Home page
            long iMonitorHome;
            int nScanfRslt;

            nScanfRslt = sscanf(szValue,"%ld",&iMonitorHome);

            if(nScanfRslt!=1)
                return 0;
            else
            {
                if(iMonitorHome&1)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("home_page"), QString("show_products"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("home_page"), QString("show_products"), false);

                if(iMonitorHome&2)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("home_page"), QString("show_testers"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("home_page"), QString("show_testers"), false);

                return 1;
            }


            /*		CRO_cleanUp

            //	#define	GEXMO_OPTION_HOME_PRODUCT		1
            //	#define	GEXMO_OPTION_HOME_TESTER		2

            sscanf(szValue,"%ld",&ReportOptions.iMonitorHome);
            return 1;	// No error.
            */
        }
        // Products page fields
        if(!qstricmp(szField,"product_page"))
        {
            // Flags for fields to list in Products page
            long iMonitorProduct;
            int nScanfRslt;

            nScanfRslt = sscanf(szValue,"%ld",&iMonitorProduct);

            if(nScanfRslt!=1)
                return 0;
            else
            {
                if(iMonitorProduct&1)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("total_parts"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("total_parts"), false);

                if(iMonitorProduct&2)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("total_good"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("total_good"), false);

                if(iMonitorProduct&4)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("total_fail"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("total_fail"), false);

                if(iMonitorProduct&8)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("yield"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("yield"), false);

                if(iMonitorProduct&16)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("alarm_limit"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("alarm_limit"), false);

                if(iMonitorProduct&32)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("yield_chart"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("product_page"), QString("yield_chart"), false);


                return 1;
            }


            /*		CRO_cleanUp

            //	#define	GEXMO_OPTION_PRODUCTS_PARTS		1
            //	#define	GEXMO_OPTION_PRODUCTS_GOOD		2
            //	#define	GEXMO_OPTION_PRODUCTS_FAIL		4
            //	#define	GEXMO_OPTION_PRODUCTS_YIELD		8
            //	#define	GEXMO_OPTION_PRODUCTS_ALARM		16
            //	#define	GEXMO_OPTION_PRODUCTS_CHART		32

            sscanf(szValue,"%ld",&ReportOptions.iMonitorProduct);
            return 1;	// No error.
            */
        }
        // Testers page fields
        if(!qstricmp(szField,"tester_page"))
        {
            // Flags for fields to list in Tester page
            long iMonitorTester;
            int nScanfRslt;

            nScanfRslt = sscanf(szValue,"%ld",&iMonitorTester);

            if(nScanfRslt!=1)
                return 0;
            else
            {
                if(iMonitorTester&1)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("product"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("product"), false);

                if(iMonitorTester&2)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("operator"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("operator"), false);

                if(iMonitorTester&4)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("program_name"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("program_name"), false);

                if(iMonitorTester&8)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("total_parts"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("total_parts"), false);

                if(iMonitorTester&16)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("total_good"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("total_good"), false);

                if(iMonitorTester&32)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("total_fail"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("total_fail"), false);

                if(iMonitorTester&64)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("yield"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("yield"), false);

                if(iMonitorTester&128)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("alarm_limit"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("alarm_limit"), false);

                if(iMonitorTester&256)
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("yield_chart"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("monitoring"), QString("tester_page"), QString("yield_chart"), false);


                return 1;
            }


            /*		CRO_cleanUp

            //	#define	GEXMO_OPTION_TESTERS_PRODUCT	1
            //	#define	GEXMO_OPTION_TESTERS_OPERATOR	2
            //	#define	GEXMO_OPTION_TESTERS_PROGRAM	4
            //	#define	GEXMO_OPTION_TESTERS_PARTS		8
            //	#define	GEXMO_OPTION_TESTERS_GOOD		16
            //	#define	GEXMO_OPTION_TESTERS_FAIL		32
            //	#define	GEXMO_OPTION_TESTERS_YIELD		64
            //	#define	GEXMO_OPTION_TESTERS_ALARM		128
            //	#define	GEXMO_OPTION_TESTERS_CHART		256

            sscanf(szValue,"%ld",&ReportOptions.iMonitorTester);
            return 1;	// No error.
            */
        }
    }

    // Duplicate test section
    if(!qstricmp(szSection,"duplicate_test"))
    {
        /* CRO_cleanUp by HTH case# 4188
        ReportOptions.bMergeDuplicateTestNumber = false;
        ReportOptions.bMergeDuplicateTestName = false;
        if(!qstricmp(szField,"merge"))
        {
            // Merge test with identical test# (ignore test name)
            ReportOptions.bMergeDuplicateTestNumber = true;
            return 1;	// No error.
        }
        if(!qstricmp(szField,"merge_name"))
        {
            // Merge test with identical test name (ignore test#)
            ReportOptions.bMergeDuplicateTestName = true;
            return 1;	// No error.
        }
        if(!qstricmp(szField,"no_merge"))
        {
            // No merge same test numbers if test name is different
            return 1;	// No error.
        }
        */

        if(!qstricmp(szField,"merge"))
        {
            // Merge test with identical test# (ignore test name)
            ReportOptions.SetOption("dataprocessing", "duplicate_test", "merge");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"merge_name"))
        {
            // Merge test with identical test name (ignore test#)
            ReportOptions.SetOption("dataprocessing", "duplicate_test", "merge_name");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"no_merge"))
        {
            // No merge same test numbers if test name is different
            ReportOptions.SetOption("dataprocessing", "duplicate_test", "no_merge");
            return 1;	// No error.
        }
    }

    // Sample section
    /*		CRO_cleanUp
    if(!qstricmp(szSection,"samples"))
    {
        if(!qstricmp(szField,"hbin_list"))
        {
            ReportOptions.m_pSamplesHBinList = new CGexRange(szValue);
            return 1;	// No error.
        }
    }
    */


    // Scaling section
    if(!qstricmp(szSection,"scaling"))
    {
        if(!qstricmp(szField,"none"))
        {
            //ReportOptions.iSmartScaling= GEX_UNITS_RESCALE_NONE;
            ReportOptions.SetOption("dataprocessing", "scaling", "none");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"smart"))
        {
            //ReportOptions.iSmartScaling=GEX_UNITS_RESCALE_SMART;
            ReportOptions.SetOption("dataprocessing", "scaling", "smart");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"normalized"))
        {
            //ReportOptions.iSmartScaling= GEX_UNITS_RESCALE_NORMALIZED;
            ReportOptions.SetOption("dataprocessing", "scaling", "normalized");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"to_limits"))
        {
            //ReportOptions.iSmartScaling= GEX_UNITS_RESCALE_TO_LIMITS;
            ReportOptions.SetOption("dataprocessing", "scaling", "to_limits");
            return 1;	// No error.
        }
        GSLOG(SYSLOG_SEV_NOTICE, QString(" option scaling '%1' unknown !").arg( szField ).toLatin1().constData());
        WriteScriptMessage(QString(" Warning : option scaling '%1' unknown !").arg(szField),true );
        return 1;
    }

    // PARTID section
    if(!qstricmp(szSection,"part_id"))
    {
        if(!qstricmp(szField,"show"))
        {
            // Sets to SHOW PartID in reports
            /* CRO_cleanUp by HTH case# 4189
            ReportOptions.bShowPartID = true;
            */

            ReportOptions.SetOption("dataprocessing", "part_id", "show");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"hide"))
        {
            // Sets to SHOW PartID in reports
            /* CRO_cleanUp by HTH case# 4189
            ReportOptions.bShowPartID = false;
            */

            ReportOptions.SetOption("dataprocessing", "part_id", "hide");
            return 1;	// No error.
        }
    }

    // Functional Test processing.
    if(!qstricmp(szSection,"functional_tests"))
    {
        if(!qstricmp(szField,"disabled"))
        {
            // Sets to Ignore STDF.FTR records
            // CRO_cleanUp by SC case# 4195
            //ReportOptions.bIgnoreFunctionalTests = true;
            ReportOptions.SetOption("dataprocessing", "functional_tests", "disabled");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"enabled"))
        {
            // Sets to Process STDF.FTR records
            // CRO_cleanUp by SC case# 4195
            //ReportOptions.bIgnoreFunctionalTests = false;
            ReportOptions.SetOption("dataprocessing", "functional_tests", "enabled");
            return 1;	// No error.
        }
    }

    // SORTING section
    if(!qstricmp(szSection,"sorting"))	// CRO_cleanUp
    {
        if(!qstricmp(szField,"none"))
        {
            // Sets data sorting field
            // ReportOptions.iSorting = GEX_SORTING_NONE;
            ReportOptions.SetOption("dataprocessing", "sorting", "none");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"date"))
        {
            // ReportOptions.iSorting = GEX_SORTING_BYDATE;
            ReportOptions.SetOption("dataprocessing", "sorting", "date");
            return 1;	// No error.
        }

        WriteScriptMessage(QString("warning : unknown option '%1' '%2'!").arg(szSection).arg(szField), true);
        return 0;
    }

    // FAILURES count section
    if(!qstricmp(szSection,"fail_count"))
    {
        if(!qstricmp(szField,"all"))
        {
            // Failure count mode = ALL failures are counted
            // CRO_cleanUp by SC case# 4191
            //ReportOptions.iFailures = GEX_FAILCOUNT_ALL;
            ReportOptions.SetOption("dataprocessing", "fail_count", "all");
            return 1;	// No error.
        }
        if(!qstricmp(szField,"first"))
        {
            // Failure count mode = only 1st failure in flow counted
            // CRO_cleanUp by SC case# 4191
            //ReportOptions.iFailures = GEX_FAILCOUNT_FIRST;
            ReportOptions.SetOption("dataprocessing", "fail_count", "first");
            return 1;	// No error.
        }
    }

    // Multi-Parametric tests: merge pins under same name?
    if(!qstricmp(szSection,"multi_parametric"))
    {
        if(!qstricmp(szField,"merge"))
        {
            // Merge: Merge all multi-parametric pins under same test name
            //ReportOptions.bMultiParametricMerge = true;
            ReportOptions.SetOption("dataprocessing", "multi_parametric_merge_mode", "merge");
            return 1;	// No error.
        }
        else if(!qstricmp(szField,"no_merge"))
        {
            // No merge: keep one test per pin tested.
            //ReportOptions.bMultiParametricMerge = false;
            ReportOptions.SetOption("dataprocessing", "multi_parametric_merge_mode", "no_merge");
            return 1;	// No error.
        }
        else
        {
            WriteScriptMessage(
                    QString(" unknown option 'multi_parametric' '%1' ").arg(szField),
                            true);
            return 0;
        }

        if(!qstricmp(szField,"criteria"))
        {
            ReportOptions.SetOption("dataprocessing","multi_parametric_merge_criteria", szValue);
            return 1;
            /*
            // Use first value as merged result
            if(!qstricmp(szValue,"first"))
            {
                ReportOptions.cMultiResultValue = GEX_MULTIRESULT_USE_FIRST;
                return 1;
            }
            // Use last value as merged result
            if(!qstricmp(szValue,"last"))
            {
                ReportOptions.cMultiResultValue = GEX_MULTIRESULT_USE_LAST;
                return 1;
            }
            // Use min value as merged result
            if(!qstricmp(szValue,"min"))
            {
                ReportOptions.cMultiResultValue = GEX_MULTIRESULT_USE_MIN;
                return 1;
            }
            // Use max value as merged result
            if(!qstricmp(szValue,"max"))
            {
                ReportOptions.cMultiResultValue = GEX_MULTIRESULT_USE_MAX;
                return 1;
            }
            // Use mean value as merged result
            if(!qstricmp(szValue,"mean"))
            {
                ReportOptions.cMultiResultValue = GEX_MULTIRESULT_USE_MEAN;
                return 1;
            }
            // Use median value as merged result
            if(!qstricmp(szValue,"median"))
            {
                ReportOptions.cMultiResultValue = GEX_MULTIRESULT_USE_MEDIAN;
                return 1;
            }
            */
        }

        if(!qstricmp(szField,"option"))
        {
            //		CRO_cleanUp
            QString strSection("dataprocessing"), strField("multi_parametric_merge_option"), strValue(szValue);
            if(strValue == QString("keepAll"))
            {
                ReportOptions.SetOption(strSection, strField, QString("keep_all"));
                return 1;
            }
            else if(strValue == QString("keepOne"))
            {
                ReportOptions.SetOption(strSection, strField, QString("keep_one"));
                return 1;
            }
            else
            {
                return 0;
            }


            //ReportOptions.SetOption("dataprocessing", "multi_parametric_merge_option", szField);
            //return 1;
            // Keep all results collected
            /*
            if(!qstricmp(szValue,"keepAll"))
            {
                //ReportOptions.bMultiParametricAlwaysKeepOne = false;

                return 1;
            }
            // Keep a single result
            if(!qstricmp(szValue,"keepOne"))
            {
                ReportOptions.bMultiParametricAlwaysKeepOne = true;
                return 1;
            }
            */
        }

    }

    // STDF Compliancy handling
    if(!qstricmp(szSection,"stdf_compliancy")) // CRO_CleanUp
    {
        if(!qstricmp(szField,"stringent"))
        {
            // Expect STDF files to be fully compliant
            //ReportOptions.bStringentStdfCompliancy = true;
            ReportOptions.SetOption("dataprocessing", "stdf_compliancy", "stringent");
            return 1;
        }
        if(!qstricmp(szField,"flexible"))
        {
            // Do best to handle any STDF file!
            //ReportOptions.bStringentStdfCompliancy = false;
            ReportOptions.SetOption("dataprocessing", "stdf_compliancy", "flexible");
            return 1;
        }

        WriteScriptMessage(QString("warning : unknown option '%1' '%2'!").arg(szSection).arg(szField), true);
        return 0;
    }

    // Folder where intermediate files are created (STDF & zip)
    if(!qstricmp(szSection,"stdf_intermediate"))
    {
        /* CRO_cleanUp by HTH case# 4193
        ReportOptions.strIntermediateStdfFolder = szField;
        if(QFile::exists(szField) || ReportOptions.strIntermediateStdfFolder.isEmpty() ||
           ReportOptions.strIntermediateStdfFolder.startsWith("(default)", Qt::CaseInsensitive))
            return 1;	// Valid folder defined.
            */

        QString strIntermediateStdfFolder = szField;

        if (QFile::exists(szField))
        {
            ReportOptions.SetOption("dataprocessing", "stdf_intermediate", "custom");
            ReportOptions.SetOption("dataprocessing", "stdf_intermediate_path", szField);
        }
        else if (strIntermediateStdfFolder.isEmpty() || strIntermediateStdfFolder.startsWith("(default)", Qt::CaseInsensitive))
        {
            ReportOptions.SetOption("dataprocessing", "stdf_intermediate", "same");
            ReportOptions.SetOption("dataprocessing", "stdf_intermediate_path", szField);
        }
        else
        {
            strMessage += ": Invalid 'stdf_intermediate' folder path";
            WriteScriptMessage(strMessage,true);
            return 0;
        }

         return 1;	// Valid folder defined.
    }

    // Speed Optimization
    if(!qstricmp(szSection,"speed"))
    {
        if(!qstricmp(szField,"adv_stats"))
        {
            // When to compute Advanced Statistics
            {
                if(!qstricmp(szValue,"always"))
                {
                    // Always
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_ALWAYS;
                    ReportOptions.SetOption("speed","adv_stats","always");
                    return 1;
                }
                if(!qstricmp(szValue,"50mb"))
                {
                    // If less than 50Mb of data
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_50MB;
                    ReportOptions.SetOption("speed","adv_stats","50mb");
                    return 1;
                }
                if(!qstricmp(szValue,"100mb"))
                {
                    // If less than 100Mb of data
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_100MB;
                    ReportOptions.SetOption("speed","adv_stats", "100mb");
                    return 1;
                }
                if(!qstricmp(szValue,"200mb"))
                {
                    // If less than 200Mb of data
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_200MB;
                    ReportOptions.SetOption("speed","adv_stats","200mb");
                    return 1;
                }
                if(!qstricmp(szValue,"300mb"))
                {
                    // If less than 300Mb of data
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_300MB;
                    ReportOptions.SetOption("speed","adv_stats","300mb");
                    return 1;
                }
                if(!qstricmp(szValue,"400mb"))
                {
                    // If less than 400Mb of data
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_400MB;
                    ReportOptions.SetOption("speed","adv_stats","400mb");
                    return 1;
                }
                if(!qstricmp(szValue,"500mb"))
                {
                    // If less than 500Mb of data
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_500MB;
                    ReportOptions.SetOption("speed","adv_stats","500mb");
                    return 1;
                }
                if(!qstricmp(szValue,"never"))
                {
                    // Never
                    // ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_NEVER;
                    ReportOptions.SetOption("speed","adv_stats","never");
                    return 1;
                }

            }

        }

        if(!qstricmp(szField,"collect_samples"))
        {
            // Option outdated!
            return 1;
        }
    }

    // Data cleaning section (with outlier former terminology retro-compatibility)
    if(!qstricmp(szSection,"data") || !qstricmp(szSection,"outlier"))
    {
        if(!qstricmp(szField,"cleaning") || !qstricmp(szField,"removal"))
        {

            if(!qstricmp(szValue,"none"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_NONE;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", QString(szValue));
                return 1;
            }
            if(!qstricmp(szValue,"1sigma"))
            {
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                //ReportOptions.fOutlierRemoveN_Factor = 0.5;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "0.5");
                return 1;
            }
            if(!qstricmp(szValue,"2sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 1.0;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "1.0");
                return 1;
            }
            if(!qstricmp(szValue,"3sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 1.5;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "1.5");
                return 1;
            }
            if(!qstricmp(szValue,"4sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 2.0;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "2.0");
                return 1;
            }
            if(!qstricmp(szValue,"5sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 2.5;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "2.5");
                return 1;
            }
            if(!qstricmp(szValue,"6sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 3.0;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "3.0");
                return 1;
            }
            if(!qstricmp(szValue,"7sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 3.5;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "3.5");
                return 1;
            }
            if(!qstricmp(szValue,"8sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 4.0;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "4.0");
                return 1;
            }
            if(!qstricmp(szValue,"9sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = 4.5;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "4.5");
                return 1;
            }
            if(!qstricmp(szValue,"10sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "5.0");
                //ReportOptions.fOutlierRemoveN_Factor = 5.0;
                return 1;
            }
            if(!qstricmp(szValue,"12sigma"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "6.0");
                //ReportOptions.fOutlierRemoveN_Factor = 6.0;
                return 1;
            }
            if(!qstricmp(szValue,"n_sigma"))
            {
                // N*Sigma.
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = strSetValue.toFloat();
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", strSetValue);
                return 1;
            }
            if(!qstricmp(szValue,"exclude_n_sigma"))
            {
                // Exclude N*Sigma.: Inliner filtering.
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_INLINER_SIGMA;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "exclude_n_sigma");
                //ReportOptions.fOutlierRemoveN_Factor = strSetValue.toFloat();
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", strSetValue);
                return 1;
            }
            if(!qstricmp(szValue,"100%"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_100LIM;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_pourcent");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "100");
                return 1;
            }
            if(!qstricmp(szValue,"150%"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_150LIM;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_pourcent");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "150");
                return 1;
            }
            if(!qstricmp(szValue,"200%"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_200LIM;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_pourcent");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "200");
                return 1;
            }
            if(!qstricmp(szValue,"250%"))
            {
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_250LIM;
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_pourcent");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "250");
                return 1;
            }
            if(!qstricmp(szValue,"300%"))
            {
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_pourcent");
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "300");
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_300LIM;
                return 1;
            }

            if(!qstricmp(szValue,"iqr"))
            {
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_iqr");
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_IQR;
                //ReportOptions.fOutlierRemoveIQR_Factor = 1.5;
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", "1.5");
                return 1;
            }
            if(!qstricmp(szValue,"n_iqr"))
            {
                // Q1-N*IQR , Q3+N*IQR
                ReportOptions.SetOption("dataprocessing","data_cleaning_mode", "n_iqr");
                //ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_IQR;
                //ReportOptions.fOutlierRemoveIQR_Factor = strSetValue.toFloat();
                ReportOptions.SetOption("dataprocessing","data_cleaning_value", strSetValue);
                return 1;
            }
            WriteScriptMessage(QString("Unknow data_cleaning value '%1'").arg(szValue), true );
            return 0;
        }
    }

    // STATISTICS section
    if(!qstricmp(szSection,"statistics"))
    {
        if(!qstricmp(szField,"section_name"))
        {
            // ReportOptions.strStatsSectionTitle = szValue;
            ReportOptions.SetOption(szSection, szField, szValue);
            return 1;
        }

        // CRO_cleanUp
        if(!qstricmp(szField,"computation"))
        {
            if(!qstricmp(szValue,"summary_only"))
            {
                // ReportOptions.iStatsSource = GEX_STATISTICS_FROM_SUMMARY_ONLY;
                ReportOptions.SetOption("statistics","computation","summary_only");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"samples_only"))
            {
                // ReportOptions.iStatsSource= GEX_STATISTICS_FROM_SAMPLES_ONLY;
                ReportOptions.SetOption("statistics","computation","samples_only");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"samples_then_summary") || !qstricmp(szValue,"samples"))
            {
                // ReportOptions.iStatsSource= GEX_STATISTICS_FROM_SAMPLES_THEN_SUMMARY;
                ReportOptions.SetOption("statistics","computation","samples_then_summary");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"summary_then_samples") || !qstricmp(szValue,"summary"))
            {
                // ReportOptions.iStatsSource= GEX_STATISTICS_FROM_SUMMARY_THEN_SAMPLES;
                ReportOptions.SetOption("statistics","computation","summary_then_samples");
                return 1;	// No error.
            }
        }

        if(!qstricmp(szField,"cp_cpk_computation"))
        {
            /*
            if(!qstricmp(szValue,"standard"))
            {
                ReportOptions.bStatsCpCpkPercentileFormula= false;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"percentile"))
            {
                ReportOptions.bStatsCpCpkPercentileFormula= true;
                return 1;	// No error.
            }
            */
            ReportOptions.SetOption(szSection, szField, szValue);
            return 1;
        }

        if(!qstricmp(szField,"smart_scaling"))
        {
            // CRO_cleanup
            if(!qstricmp(szValue,"no"))
            {
                //ReportOptions.iSmartScaling= GEX_UNITS_RESCALE_NONE;
                ReportOptions.SetOption("dataprocessing","scaling","none");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"yes"))
            {
                //ReportOptions.iSmartScaling=GEX_UNITS_RESCALE_SMART;
                ReportOptions.SetOption("dataprocessing","scaling","smart");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"normalized"))
            {
                //ReportOptions.iSmartScaling= GEX_UNITS_RESCALE_NORMALIZED;
                ReportOptions.SetOption("dataprocessing","scaling","normalized");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"to_limits"))
            {
                //ReportOptions.iSmartScaling= GEX_UNITS_RESCALE_TO_LIMITS;
                ReportOptions.SetOption("dataprocessing","scaling","to_limits");
                return 1;	// No error.
            }

            WriteScriptMessage(QString("Error : unknow smart_scaling value '%1'").arg(szValue), true );
            return 0;
        }

        if(!qstricmp(szField,"field"))
        {
            // CRO_cleanUp
            QString strNewField;
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            // standard fields
            qstrlPossibleValueList.clear();
            qstrlPossibleValueList << "test_name" << "test_type" << "limits" << "spec_limits" << "stats_source" << "exec_count"
                                << "test_flow_id" << "fail_count" << "fail_percent" << "fail_bin" << "removed_count" << "mean"
                                << "sigma" << "2sigma" << "3sigma" << "6sigma" << "min" << "max" << "range" << "cp" << "cpk"
                                << "yield";

            if(qstrlPossibleValueList.contains(strValue))
            {
                strNewField = QString("fields");
                addFlagOption(szSection, strNewField.toLatin1().constData(), strValue.toLatin1().constData());
                return 1;
            }

            // gage study fields
            qstrlPossibleValueList.clear();
            qstrlPossibleValueList << "ev" << "av" << "r&r" << "gb" << "pv" << "tv" << "p_t";

            if(qstrlPossibleValueList.contains(strValue))
            {
                strNewField = QString("gage_fields");
                addFlagOption(szSection, strNewField.toLatin1().constData(), strValue.toLatin1().constData());
                return 1;
            }

            // advanced fields
            qstrlPossibleValueList.clear();
            qstrlPossibleValueList << "shape" << "cpkL" << "cpkH" << "skew" << "kurtosis" << "P0.5%" << "P0.5";
            qstrlPossibleValueList << "P2.5%" << "P2.5" << "P10%" << "P10" << "quartile1" << "P25%" << "P25" ;
            qstrlPossibleValueList << "quartile2" << "P50%" << "P50" << "quartile3" << "P75%" << "P75" << "P90%" << "P90" ;
            qstrlPossibleValueList << "P97.5%" << "P97.5" << "P99.5%" << "P99.5" << "interquartiles" << "SigmaInterQuartiles";

            if(qstrlPossibleValueList.contains(strValue))
            {
                strNewField = QString("adv_fields");
                if(strValue == QString("cpkL"))
                {
                    strValue = QString("cpk_l");
                }
                if(strValue == QString("cpkH"))
                {
                    strValue = QString("cpk_h");
                }
                if(strValue == QString("P0.5%"))
                {
                    strValue = QString("P0.5");
                }
                if(strValue == QString("P2.5%"))
                {
                    strValue = QString("P2.5");
                }
                if(strValue == QString("P10%"))
                {
                    strValue = QString("P10");
                }
                if(strValue == QString("P25%"))
                {
                    strValue = QString("quartile1");
                }
                if(strValue == QString("P25"))
                {
                    strValue = QString("quartile1");
                }
                if(strValue == QString("P50%"))
                {
                    strValue = QString("quartile2");
                }
                if(strValue == QString("P50"))
                {
                    strValue = QString("quartile2");
                }
                if(strValue == QString("P75%"))
                {
                    strValue = QString("quartile3");
                }
                if(strValue == QString("P75"))
                {
                    strValue = QString("quartile3");
                }
                if(strValue == QString("P90%"))
                {
                    strValue = QString("P90");
                }
                if(strValue == QString("P97.5%"))
                {
                    strValue = QString("P97.5");
                }
                if(strValue == QString("P99.5%"))
                {
                    strValue = QString("P99.5");
                }
                if(strValue == QString("interquartiles"))
                {
                    strValue = QString("inter_quartiles");
                }
                if(strValue == QString("SigmaInterQuartiles"))
                {
                    strValue = QString("sigma_inter_quartiles");
                }

                addFlagOption(szSection, strNewField.toLatin1().constData(), strValue.toLatin1().constData());
                return 1;
            }


            // compare study fields
            qstrlPossibleValueList.clear();
            qstrlPossibleValueList << "mean_shift" << "t_test" << "sigma_shift" << "max_range" << "cp_shift" << "cpk_shift";

            if(qstrlPossibleValueList.contains(strValue))
            {
                strNewField = QString("compare_fields");
                addFlagOption(szSection, strNewField.toLatin1().constData(), strValue.toLatin1().constData());
                return 1;
            }

            strMessage += ": Unknown 'statistics', 'field' option ";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;

        }

        // CRO_cleanUp
        if(!qstricmp(szField,"sorting"))
        {
            // Sets Statistics table sorting field
            if(!qstricmp(szValue,"test_number"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_TESTNUMBER;
                ReportOptions.SetOption("statistics","sorting","test_number");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"test_name"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_TESTNAME;
                ReportOptions.SetOption("statistics","sorting","test_name");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"test_flow_id"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_TESTFLOWID;
                ReportOptions.SetOption("statistics","sorting","test_flow_id");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_MEAN;
                ReportOptions.SetOption("statistics","sorting","mean");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean_shift"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_MEAN_SHIFT;
                ReportOptions.SetOption("statistics","sorting","mean_shift");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"sigma"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_SIGMA;
                ReportOptions.SetOption("statistics","sorting","sigma");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cp"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_CP;
                ReportOptions.SetOption("statistics","sorting","cp");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cpk"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_CPK;
                ReportOptions.SetOption("statistics","sorting","cpk");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"r&r"))
            {
                // ReportOptions.iStatsTableSorting = GEX_SORTING_R_R;
                ReportOptions.SetOption("statistics","sorting","r&r");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"correlation"))
            {
                // ignored: no longer supported!
                return 1;	// No error.
            }
        }

        // cpk alarms
        {
            /* CRO_cleanUp
            if(!qstricmp(szField,"alarm_test_cpk"))
            {

                // Sets Cpk RED alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableRedAlarmCpk) != 1)
                    ReportOptions.fStatsTableRedAlarmCpk = -1;	// Ignore alarm!
                return 1;
            }

            if(!qstricmp(szField,"warning_test_cpk"))
            {

                // Sets Cpk YELLOW alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableYellowAlarmCpk) != 1)
                    ReportOptions.fStatsTableYellowAlarmCpk = -1;	// Ignore alarm!
                return 1;
            }
            */


            double lfOptionStorageDevice;
            int nScanfRslt;

            if(!qstricmp(szField,"alarm_test_cpk"))
            {
                // Sets Cpk RED alarm level!
                nScanfRslt = sscanf(szValue,"%lf",&lfOptionStorageDevice);
                if( (nScanfRslt != 1) || (lfOptionStorageDevice<0) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                ReportOptions.SetOption("statistics", "alarm_test_cpk", QString::number(lfOptionStorageDevice));
                return 1;
            }

            if(!qstricmp(szField,"warning_test_cpk"))
            {

                // Sets Cpk YELLOW alarm level!
                nScanfRslt = sscanf(szValue,"%lf",&lfOptionStorageDevice);
                if( (nScanfRslt != 1) || (lfOptionStorageDevice<0) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                ReportOptions.SetOption("statistics", "warning_test_cpk", QString::number(lfOptionStorageDevice));
                return 1;
            }
        }


        // yield alarms
        {
            double lfOptionStorageDevice;
            int nScanfRslt;

            if(!qstricmp(szField,"alarm_test_yield"))
            {
                // Sets Test yield RED alarm level!
                nScanfRslt = sscanf(szValue,"%lf",&lfOptionStorageDevice);

                if( (nScanfRslt != 1) || (lfOptionStorageDevice<0) || (lfOptionStorageDevice>100) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                ReportOptions.SetOption("statistics", "alarm_test_yield", QString::number(lfOptionStorageDevice));

                return 1;
            }
            if(!qstricmp(szField,"warning_test_yield"))
            {
                // Sets Test yield YELLOW alarm level!
                nScanfRslt = sscanf(szValue,"%lf",&lfOptionStorageDevice);

                if( (nScanfRslt != 1) || (lfOptionStorageDevice<0) || (lfOptionStorageDevice>100) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                ReportOptions.SetOption("statistics", "warning_test_yield", QString::number(lfOptionStorageDevice));

                return 1;
            }

            /* CRO_cleanUp
            if(!qstricmp(szField,"alarm_test_yield"))
            {
                // Sets Test yield RED alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableRedAlarmTestYield) != 1)
                    ReportOptions.fStatsTableRedAlarmTestYield = -1;	// Ignore alarm!

                return 1;
            }
            if(!qstricmp(szField,"warning_test_yield"))
            {
                // Sets Test yield YELLOW alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableYellowAlarmTestYield) != 1)
                    ReportOptions.fStatsTableYellowAlarmTestYield = -1;	// Ignore alarm!
                return 1;
            }
            */
        }

        // drift options
        {
            if(!qstricmp(szField,"alarm_mean"))
            {
                double lfOptionStorageDevice;
                int nScanResult;

                nScanResult = sscanf(szValue,"%lf",&lfOptionStorageDevice);

                if( (nScanResult != 1) || (lfOptionStorageDevice<0) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                // Sets Mean shift alarm level!
                ReportOptions.SetOption("statistics","alarm_mean",QString::number(lfOptionStorageDevice));
                return 1;
            }

            if(!qstricmp(szField,"alarm_sigma"))
            {
                double lfOptionStorageDevice;
                int nScanResult;

                nScanResult = sscanf(szValue,"%lf",&lfOptionStorageDevice);

                if( (nScanResult != 1) || (lfOptionStorageDevice<0) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                // Sets Sigma shift alarm level!
                ReportOptions.SetOption("statistics","alarm_sigma",QString::number(lfOptionStorageDevice));
                return 1;
            }

            if(!qstricmp(szField,"alarm_cpk"))
            {
                double lfOptionStorageDevice;
                int nScanResult;

                nScanResult = sscanf(szValue,"%lf",&lfOptionStorageDevice);

                if( (nScanResult != 1) || (lfOptionStorageDevice<0) )
                    lfOptionStorageDevice = -1;	// Ignore alarm!

                // Sets Cpk shift alarm level!
                ReportOptions.SetOption("statistics","alarm_cpk",QString::number(lfOptionStorageDevice));
                return 1;
            }

            /* CRO_cleanUp
            if(!qstricmp(szField,"alarm_mean"))
            {
                // Sets Mean shift alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableAlarmMeanDrift) != 1)
                    ReportOptions.fStatsTableAlarmMeanDrift = -1;	// Ignore alarm!
                return 1;

            }
            if(!qstricmp(szField,"alarm_sigma"))
            {
                // Sets Sigma shift alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableAlarmSigmaDrift) != 1)
                    ReportOptions.fStatsTableAlarmSigmaDrift = -1;	// Ignore alarm!
                return 1;

            }
            if(!qstricmp(szField,"alarm_cpk"))
            {
                // Sets Cpk shift alarm level!
                if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableAlarmCpkDrift) != 1)
                    ReportOptions.fStatsTableAlarmCpkDrift = -1;	// Ignore alarm!
                return 1;
            }
            */
        }


        // CRO_cleanUp
        if(!qstricmp(szField,"mean_drift_formula"))
        {
            // Sets the formula to use to compute the MEAN drift.
            if(!qstricmp(szValue,"value"))
            {
                ReportOptions.SetOption("statistics","mean_drift_formula","value");
                // ReportOptions.iStatsMeanDriftFormula = GEX_DRIFT_ALARM_VALUE;	// Drift of value
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.SetOption("statistics","mean_drift_formula","limits");
                // ReportOptions.iStatsMeanDriftFormula = GEX_DRIFT_ALARM_LIMITS;	// Drift of limits space
                return 1;	// No error.
            }
        }


        if(!qstricmp(szField,"generic_galaxy_tests"))		// CRO_cleanUp
        {
            // Generic Galaxy tests.
            if(!qstricmp(szValue,"show"))
            {
                // ReportOptions.bIncludeGenericTests = true;	// Auto Generate
                ReportOptions.SetOption("statistics","generic_galaxy_tests","show");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"hide"))
            {
                // ReportOptions.bIncludeGenericTests = false;	// Ignore
                ReportOptions.SetOption("statistics","generic_galaxy_tests","hide");
                return 1;	// No error.
            }
        }
    }


    // WAFER MAP section
    if(!qstricmp(szSection,"wafer"))
    {
        if(!qstricmp(szField,"section_name"))
        {
            /* CRO_cleanup
            ReportOptions.strWafmapSectionTitle = szValue;
            */
            ReportOptions.SetOption(szSection, szField, szValue);
            return 1;
        }

        if(!qstricmp(szField,"chart_show"))
        {
            /* CRO_cleanUp by HTH case# 3801
            if(sscanf(szValue,"%d",&ReportOptions.iWafmapShow) != 1)
                ReportOptions.iWafmapShow = GEX_OPTION_WAFMAP_STACKED;
            */

//#define	GEX_OPTION_WAFMAP_STACKED						0x01		// Draw Stacked wafers
//#define	GEX_OPTION_WAFMAP_INDIVIDUAL					0x02		// Draw all individual wafers.
//#define GEX_OPTION_WAFMAP_DIEMISMATCH_BIN				0x04		// Display wafermap with bin mismatches
//#define GEX_OPTION_WAFMAP_DIEMISMATCH_BIN_TO_BIN		0x08		// Display wafermap with bin to bin mismatches

            int		nWafermapShow;
            QString strWafermapShow;

            if(sscanf(szValue, "%d", &nWafermapShow) != 1)
                nWafermapShow = 0x01;

            if(nWafermapShow & 0x01)
                addFlagOption("wafer", "chart_show", "stacked");

            if (nWafermapShow & 0x02)
                addFlagOption("wafer", "chart_show", "all_individual");

            if (nWafermapShow & 0x04)
                addFlagOption("wafer", "chart_show", "bin_mismatch");

            if (nWafermapShow & 0x08)
                addFlagOption("wafer", "chart_show", "bin_to_bin");

            return 1;	// No error.
        }

        if(!qstricmp(szField,"parametric_stacked"))
        {
            /* CRO_cleanUp by HTH case# 4290
            if(sscanf(szValue,"%d",&ReportOptions.iWafmapParametricStacked) != 1)
                ReportOptions.iWafmapParametricStacked = GEX_OPTION_WAFMAP_PARAM_STACKED_MEAN;
            */

            int nWaferParametricStacked = 1;

            if(sscanf(szValue, "%d", &nWaferParametricStacked) != 1)
                nWaferParametricStacked = 1;

            if (nWaferParametricStacked == 1)
                ReportOptions.SetOption("wafer", "parametric_stacked", "mean");
            else if (nWaferParametricStacked == 1)
                ReportOptions.SetOption("wafer", "parametric_stacked", "median");

            return 1;	// No error.
        }

        if(!qstricmp(szField,"bin_stacked"))
        {
            /* CRO_cleanUp by HTH case# 4291
            if(sscanf(szValue,"%d",&ReportOptions.iWafmapBinStacked) != 1)
                ReportOptions.iWafmapBinStacked = GEX_OPTION_WAFMAP_BIN_STACKED_BINCOUNT;
            */

            int nWafmapBinStacked = 1;

            if(sscanf(szValue,"%d",&nWafmapBinStacked) != 1)
                nWafmapBinStacked = 1;

            switch (nWafmapBinStacked)
            {
                default :
                case 1	:	ReportOptions.SetOption("wafer", "bin_stacked", "bin_count");
                            break;

                case 2	:	ReportOptions.SetOption("wafer", "bin_stacked", "pf_pass_if_all_pass");
                            break;
            }

            return 1;	// No error.
        }

        /*
        if(!qstricmp(szField,"gross_die"))
        {
            if(sscanf(szValue,"%d", &ReportOptions.iGrossDieCount) != 1)
                ReportOptions.iGrossDieCount = -1;
            return 1;	// No error.
        }
        */
        if(!qstricmp(szField,"chart_view"))
        {
            // Outdated functions. No longer supported. Quietly exit
            if(!qstricmp(szValue,"mirror_x"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapMirrorX = true;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "mirror_x", true);
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"no_mirror_x"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapMirrorX = false;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "mirror_x", false);
                return 1;	// No error.
            }

            if(!qstricmp(szValue,"mirror_y"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapMirrorY = true;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "mirror_y", true);
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"no_mirror_y"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapMirrorY = false;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "mirror_y", false);
                return 1;	// No error.
            }

            if(!qstricmp(szValue,"all_parts"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapFullWafer = true;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "all_parts", true);
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"filtered_parts"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapFullWafer = false;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "all_parts", false);
                return 1;	// No error.
            }

            if(!qstricmp(szValue,"shape_round"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapAlwaysRound = true;	// Resize dies to always make wafermap look round.
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "shape_round", true);
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"shape_default"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.bWafmapAlwaysRound = false;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "visual_options", "shape_round", false);
                return 1;	// No error.
            }

#if 1 // BG: WaferMap orientation options
            if(!qstricmp(szValue,"positive_x_right"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.iWafmapPositiveX = GEX_OPTION_WAFMAP_POSITIVE_RIGHT;
                */

                ReportOptions.SetOption("wafer", "positive_x", "right");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"positive_x_left"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.iWafmapPositiveX = GEX_OPTION_WAFMAP_POSITIVE_LEFT;
                */

                ReportOptions.SetOption("wafer", "positive_x", "left");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"positive_x_auto"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.iWafmapPositiveX = GEX_OPTION_WAFMAP_POSITIVE_AUTO;
                */
                ReportOptions.SetOption("wafer", "positive_x", "auto");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"positive_y_up"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.iWafmapPositiveY = GEX_OPTION_WAFMAP_POSITIVE_UP;
                */

                ReportOptions.SetOption("wafer", "positive_y", "up");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"positive_y_down"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.iWafmapPositiveY = GEX_OPTION_WAFMAP_POSITIVE_DOWN;
                */

                ReportOptions.SetOption("wafer", "positive_y", "down");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"positive_y_auto"))
            {
                /* CRO_cleanUp by HTH case# 3821
                ReportOptions.iWafmapPositiveY = GEX_OPTION_WAFMAP_POSITIVE_AUTO;
                */

                ReportOptions.SetOption("wafer", "positive_y", "auto");
                return 1;	// No error.
            }
#endif
        }
        if(!qstricmp(szField,"low_yield_pattern"))
        {
            // Extract Low-Yield pattern threshold value (used to detect failing patterns in stacked wafers)
            /* CRO_cleanUp
            if(sscanf(szValue,"%f",&ReportOptions.fWafmapStackedLowYieldAlarm) != 1)
                ReportOptions.fWafmapStackedLowYieldAlarm = 33;
            if(ReportOptions.fWafmapStackedLowYieldAlarm < 0)
                ReportOptions.fWafmapStackedLowYieldAlarm = 0;
            else
            if(ReportOptions.fWafmapStackedLowYieldAlarm > 100)
                ReportOptions.fWafmapStackedLowYieldAlarm = 100;
            // */
            float fOptionStorageDevice = 33.0f;
            int nScanRslt = sscanf(szValue,"%f",&fOptionStorageDevice);

            if(nScanRslt != 1)
                fOptionStorageDevice = 33.0f;

            if(fOptionStorageDevice < 0)
                fOptionStorageDevice = 0.0f;
            else if(fOptionStorageDevice > 100)
                fOptionStorageDevice = 100.0f;

            ReportOptions.SetOption("wafer","low_yield_pattern",QString::number(fOptionStorageDevice));

                return 1;	// No error.
        }

        // Allow comparing wafer with different size?
        if(!qstricmp(szField,"compare"))
        {
            if(!qstricmp(szValue,"any_size"))
            {
                /* CRO_cleanUp by HTH case# 3820
                ReportOptions.bWafmapCompareAnySize = true;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "compare", "any_size", true);
                return 1;
            }
            if(!qstricmp(szValue,"diemismatch_table_off"))
            {
                /* CRO_cleanUp by HTH case# 3820
                ReportOptions.bWafmapIncludeDieMismatchTable = false;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "compare", "diemismatch_table", false);
                return 1;
            }
            if(!qstricmp(szValue,"deltayield_section_off"))
            {
                /* CRO_cleanUp by HTH case# 3820
                ReportOptions.bWafmapIncludeDeltaYieldSection = false;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "compare", "deltayield_section", false);
                return 1;
            }
        }

        if(!qstricmp(szField,"chart_size"))
        {
            // CRO_cleanUp
            // Sets fields wafer map chart size
            if(!qstricmp(szValue,"small"))
            {
                // ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_SMALL;
                ReportOptions.SetOption("wafer","chart_size","small");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"medium"))
            {
                // ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_MEDIUM;
                ReportOptions.SetOption("wafer","chart_size","medium");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"large"))
            {
                // ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_LARGE;
                ReportOptions.SetOption("wafer","chart_size","large");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"auto"))
            {
                // ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_AUTO;
                ReportOptions.SetOption("wafer","chart_size","auto");
                return 1;	// No error.
            }
        }

        /*
        if(!qstricmp(szField,"retest_policy"))
        {
            // Highest bin: promote the highest bin value for each die
            if(!qstricmp(szValue,"highest_bin"))
            {
                ReportOptions.bWafMapRetest_HighestBin = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"last_bin"))
            {
                // Last bin: Use Last bin value found for each die (default)
                ReportOptions.bWafMapRetest_HighestBin = false;
                return 1;	// No error.
            }
        }
        */
        if(!qstricmp(szField,"marker"))
        {
            // Sets wafer map markers
            if(!qstricmp(szValue,"bin"))
            {
                /* CRO_cleanUp by HTH case# 3819
                ReportOptions.bWafmapMarkerBinInDie = true;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "marker", "bin", true);
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"retest"))
            {
                /* CRO_cleanUp by HTH case# 3819
                ReportOptions.bWafmapMarkerDieRetested = true;
                */

                ReportOptions.SetSpecificFlagOption("wafer", "marker", "retest", true);

                return 1;	// No error.
            }
        }
        if(!qstricmp(szField,"alarms"))
        {
            // Sets the wafermap export alarms when flat differs from file
            if (!qstricmp(szValue, "flat_differs_on"))
            {
                /* CRO_cleanUp by HTH case# 3822
                ReportOptions.bWafmapExportAlarmFlatDiffers = true;
                */

                ReportOptions.SetSpecificFlagOption(szSection, szField, QString("flat_differs"), true);
                return 1;
            }

            if (!qstricmp(szValue, "flat_differs_off"))
            {
                /* CRO_cleanUp by HTH case# 3822
                ReportOptions.bWafmapExportAlarmFlatDiffers = false;
                */

                ReportOptions.SetSpecificFlagOption(szSection, szField, QString("flat_differs"), false);
                return 1;
            }

            // Sets the wafermap export alarms when no flat is detected from file
            if (!qstricmp(szValue, "no_flat_on"))
            {
                /* CRO_cleanUp by HTH case# 3822
                ReportOptions.bWafmapExportAlarmNoFlat = true;
                */

                ReportOptions.SetSpecificFlagOption(szSection, szField, QString("no_flat"), true);
                return 1;
            }

            if (!qstricmp(szValue, "no_flat_off"))
            {
                /* CRO_cleanUp by HTH case# 3822
                ReportOptions.bWafmapExportAlarmNoFlat = false;
                */

                ReportOptions.SetSpecificFlagOption(szSection, szField, QString("no_flat"), false);
                return 1;
            }
        }
    }

    // HISTOGRAM section
    if(!qstricmp(szSection,"histogram"))
    {
        if(!qstricmp(szField,"section_name"))
        {
            /* CRO_cleanup
            ReportOptions.strHistoSectionTitle = szValue;
            */
            ReportOptions.SetOption(szSection, szField, szValue);
            return 1;
        }
    }

    // Advanced CHART section
    if(!qstricmp(szSection,"adv_chart"))
    {
        if(!qstricmp(szField,"field"))
        {
            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "limits" << "exec_count" << "mean" << "quartile2" << "sigma" << "removed_count";
            qstrlPossibleValueList << "removed_count" << "fail_count" << "cp" << "cpk" << "min" << "max";

            if(qstrlPossibleValueList.contains(strValue))
            {
                strSection = QString("adv_histogram");

                if(strValue == QString("limits"))
                {
                    ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, (bool) strSetValue.toInt());
                    return 1;
                }

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }

            strMessage += ": Unknown 'adv_chart', 'field' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;


            // Sets fields to list in Statistics table.
            /*	CRO_cleanUp
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.bHistoStatsLimits = (bool) strSetValue.toInt();
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"exec_count"))
            {
                ReportOptions.bHistoStatsExec = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                ReportOptions.b$HistoStatsMean = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"quartile2"))
            {
                ReportOptions.bHistoStatsQuartile2 = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"sigma"))
            {
                ReportOptions.bHistoStatsSigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"removed_count"))
            {
                ReportOptions.bHistoStatsOutlier = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"fail_count"))
            {
                ReportOptions.bHistoStatsFails = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cp"))
            {
                ReportOptions.bHistoStatsCp = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cpk"))
            {
                ReportOptions.bHistoStatsCpk = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"min"))
            {
                ReportOptions.bHistoStatsMin = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"max"))
            {
                ReportOptions.bHistoStatsMax = true;
                return 1;	// No error.
            }
            */
        }
    }

    // Advanced HISTOGRAM section
    if(!qstricmp(szSection,"adv_histogram"))
    {
        if(!qstricmp(szField,"field"))
        {
            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "limits" << "exec_count" << "mean" << "quartile2" << "sigma";
            qstrlPossibleValueList << "fail_count" << "cp" << "cpk";

            if(qstrlPossibleValueList.contains(strValue))
            {
                if(strValue == QString("limits"))
                {
                    ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, (bool) strSetValue.toInt());
                    return 1;
                }

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }

            strMessage += ": Unknown 'adv_histogram', 'field' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;

            // Sets fields to list in Statistics table.
            /*	CRO_cleanUp
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.bHistoStatsLimits = (bool) strSetValue.toInt();
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"exec_count"))
            {
                ReportOptions.bHistoStatsExec = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                ReportOptions.bHistoStatsMean = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"quartile2"))
            {
                ReportOptions.bHistoStatsQuartile2 = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"sigma"))
            {
                ReportOptions.bHistoStatsSigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"removed_count"))
            {
                ReportOptions.bHistoStatsOutlier = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"fail_count"))
            {
                ReportOptions.bHistoStatsFails = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cp"))
            {
                ReportOptions.bHistoStatsCp = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cpk"))
            {
                ReportOptions.bHistoStatsCpk = true;
                return 1;	// No error.
            }
            */
        }

        /*
        if(!qstricmp(szField,"chart_size"))
        {
            // Sets fields histogram chart size
            if(!qstricmp(szValue,"banner"))
            {
                ReportOptions.iHistoChartSize = GEX_CHARTSIZE_BANNER;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"small"))
            {
                ReportOptions.iHistoChartSize = GEX_CHARTSIZE_SMALL;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"medium"))
            {
                ReportOptions.iHistoChartSize = GEX_CHARTSIZE_MEDIUM;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"large"))
            {
                ReportOptions.iHistoChartSize = GEX_CHARTSIZE_LARGE;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"auto"))
            {
                ReportOptions.iHistoChartSize = GEX_CHARTSIZE_AUTO;
                return 1;	// No error.
            }
        }
        */

        if(!qstricmp(szField,"chart_type"))
        {
            // CRO_cleanUp

            // Sets fields histogram chart type
            if(!qstricmp(szValue,"bars"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARS;
                ReportOptions.SetOption("adv_histogram","chart_type","bars");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"3d_bars"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_3DBARS;
                ReportOptions.SetOption("adv_histogram","chart_type","3d_bars");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"bars_outline"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSOUTLINE;
                ReportOptions.SetOption("adv_histogram","chart_type","bars_outline");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"curve"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_SPLINE;
                ReportOptions.SetOption("adv_histogram","chart_type","curve");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"bars_curve"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSSPLINE;
                ReportOptions.SetOption("adv_histogram","chart_type","bars_curve");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"gaussian"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_GAUSSIAN;
                ReportOptions.SetOption("adv_histogram","chart_type","gaussian");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"bars_gaussian"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSGAUSSIAN;
                ReportOptions.SetOption("adv_histogram","chart_type","bars_gaussian");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"bars_outline_gaussian"))
            {
                // ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSOUTLINEGAUSSIAN;
                ReportOptions.SetOption("adv_histogram","chart_type","bars_outline_gaussian");
                return 1;	// No error.
            }
        }

        /*
        if(!qstricmp(szField,"y_axis"))
        {
            // histogram Y-axis: Display percentage scale
            if(!qstricmp(szValue,"percentage"))
            {
                ReportOptions.bHistoFrequencyCountScale = false;
                return 1;	// No error.
            }
            // histogram Y-axis: Display count / hits scale
            if(!qstricmp(szValue,"hits"))
            {
                ReportOptions.bHistoFrequencyCountScale = true;
                return 1;	// No error.
            }
        }
        */

        /*
        if(!qstricmp(szField,"total_bars"))
        {
            if(sscanf(szValue,"%d",&ReportOptions.iHistoClasses) != 1)
                ReportOptions.iHistoClasses = TEST_ADVHISTOSIZE;	// No value, force to default (40)
            if(ReportOptions.iHistoClasses < 2)
                ReportOptions.iHistoClasses = 2;
            if(ReportOptions.iHistoClasses > 10000)
                ReportOptions.iHistoClasses = 10000;
            return 1;	// No error.
        }
        */
        if(!qstricmp(szField,"marker"))
        {
            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "test_name" << "title" << "mean" << "median" << "limits" << "speclimits";
            qstrlPossibleValueList << "2sigma" << "3sigma" << "6sigma" << "12sigma";

            if(qstrlPossibleValueList.contains(strValue))
            {
                if(strValue == QString("title"))
                    strValue = QString("test_name");

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }

            strMessage += ": Unknown 'adv_histogram', 'marker' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;
        }
    }

    // Advanced TREND section
    if(!qstricmp(szSection,"adv_trend"))
    {
        /*
        if(!qstricmp(szField,"chart_size"))
        {
            // Sets fields trend chart size
            if(!qstricmp(szValue,"banner"))
            {
                ReportOptions.iTrendChartSize = GEX_CHARTSIZE_BANNER;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"small"))
            {
                ReportOptions.iTrendChartSize = GEX_CHARTSIZE_SMALL;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"medium"))
            {
                ReportOptions.iTrendChartSize = GEX_CHARTSIZE_MEDIUM;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"large"))
            {
                ReportOptions.iTrendChartSize = GEX_CHARTSIZE_LARGE;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"auto"))
            {
                ReportOptions.iTrendChartSize = GEX_CHARTSIZE_AUTO;
                return 1;	// No error.
            }
        }
        */

        /*
        if(!qstricmp(szField,"chart_type"))
        {
            // Sets fields Trend chart type
            if(!qstricmp(szValue,"lines"))
            {
                ReportOptions.iTrendChartType = GEX_CHARTTYPE_LINES;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"spots"))
            {
                ReportOptions.iTrendChartType = GEX_CHARTTYPE_SPOTS;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"lines_spots"))
            {
                ReportOptions.iTrendChartType = GEX_CHARTTYPE_LINESSPOTS;
                return 1;	// No error.
            }
        }
        */
        /*
        if (!qstricmp(szField,"x_axis"))
        {
            if(!qstricmp(szValue,"run_id"))
            {
                ReportOptions.iTrendXAxis = GEX_TREND_XAXIS_RUNID;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"part_id"))
            {
                ReportOptions.iTrendXAxis = GEX_TREND_XAXIS_PARTID;
                return 1;	// No error.
            }
        }
        */
        if(!qstricmp(szField,"marker"))
        {

            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "test_name" << "title" << "mean" << "median" << "limits" << "speclimits";
            qstrlPossibleValueList << "lot" << "sublot" << "group_name" << "2sigma" << "3sigma" << "6sigma" << "12sigma";

            if(qstrlPossibleValueList.contains(strValue))
            {
                if(strValue == QString("title"))
                    strValue = QString("test_name");

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }


            // Sets Advanced trend markers
            /*		CRO_cleanUp
            if(!qstricmp(szValue,"test_name") || !qstricmp(szValue,"title"))
            {
                ReportOptions.bTrendMarkerTitle = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                ReportOptions.bTrendMarkerMean = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"median"))
            {
                if(GS::LPPlugin::ProductInfo::getInstance()->isProfessionalEdition())
                    ReportOptions.bTrendMarkerMedian = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.bTrendMarkerLimits = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"speclimits"))
            {
                ReportOptions.bTrendMarkerSpecLimits = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"lot"))
            {
                ReportOptions.bTrendMarkerLot= true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"sublot"))
            {
                ReportOptions.bTrendMarkerSubLot= true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"group_name"))
            {
                ReportOptions.bTrendMarkerGroupName= true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"2sigma"))
            {
                ReportOptions.bTrendMarker2Sigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"3sigma"))
            {
                ReportOptions.bTrendMarker3Sigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"6sigma"))
            {
                ReportOptions.bTrendMarker6Sigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"12sigma"))
            {
                ReportOptions.bTrendMarker12Sigma = true;
                return 1;	// No error.
            }
            */
        }
        /*
        if(!qstricmp(szField,"rolling_yield"))
        {
            // Extract number of parts to use for a rolling yield (default is 200)
            if(sscanf(szValue,"%d",&ReportOptions.iTrendRollingYieldParts) != 1)
                ReportOptions.iTrendRollingYieldParts = 200;
            if(ReportOptions.iTrendRollingYieldParts < 1)
                ReportOptions.iTrendRollingYieldParts = 1;
            return 1;	// No error.
        }
        */
    }

    // Advanced CORRELATION section
    if(!qstricmp(szSection,"adv_correlation"))
    {
        /*
         // case 3842
        if(!qstricmp(szField,"chart_size"))
        {
            // Sets fields correlation chart size
            if(!qstricmp(szValue,"small"))
            {
                ReportOptions.iScatterChartSize = GEX_CHARTSIZE_SMALL;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"medium"))
            {
                ReportOptions.iScatterChartSize = GEX_CHARTSIZE_MEDIUM;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"large"))
            {
                ReportOptions.iScatterChartSize = GEX_CHARTSIZE_LARGE;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"auto"))
            {
                ReportOptions.iScatterChartSize = GEX_CHARTSIZE_AUTO;
                return 1;	// No error.
            }
        }
        */

        if(!qstricmp(szField,"marker"))
        {
            // Sets Advanced correlation markers
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "test_name" << "title" << "mean" << "median" << "limits" << "speclimits";
            qstrlPossibleValueList << "2sigma" << "3sigma" << "6sigma" << "12sigma";

            if(qstrlPossibleValueList.contains(strValue))
            {
                if(strValue == QString("title"))
                    strValue = QString("test_name");

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }

        }

        if(!qstricmp(szField,"chart_type"))
        {
            // Sets fields Scatter chart type
            if(!qstricmp(szValue,"lines"))
            {
                ReportOptions.iScatterChartType = GEX_CHARTTYPE_SPOTS;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"spots"))
            {
                ReportOptions.iScatterChartType = GEX_CHARTTYPE_SPOTS;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"lines_spots"))
            {
                ReportOptions.iScatterChartType = GEX_CHARTTYPE_SPOTS;
                return 1;	// No error.
            }
        }
    }

    // Advanced Probability plot section
    if(!qstricmp(szSection,"adv_probabilityplot"))
    {
        /*
        if(!qstricmp(szField,"chart_size"))
        {
            // Sets fields correlation chart size
            if(!qstricmp(szValue,"small"))
            {
                ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_SMALL;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"medium"))
            {
                ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_MEDIUM;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"large"))
            {
                ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_LARGE;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"auto"))
            {
                ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_AUTO;
                return 1;	// No error.
            }
        }
        */
        if(!qstricmp(szField,"marker"))
        {

            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "test_name" << "title" << "mean" << "median" << "limits" << "speclimits";
            qstrlPossibleValueList << "2sigma" << "3sigma" << "6sigma" << "12sigma";

            if(qstrlPossibleValueList.contains(strValue))
            {
                if(strValue == QString("title"))
                    strValue = QString("test_name");

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }

            strMessage += ": Unknown 'adv_probabilityplot', 'marker' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;
    }

        /*
        if(!qstricmp(szField,"y_axis"))
        {
            // Sets Advanced correlation markers
            if(!qstricmp(szValue,"sigma"))
            {
                ReportOptions.bProbPlotYAxisSigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"percentage"))
            {
                ReportOptions.bProbPlotYAxisSigma = false;
                return 1;	// No error.
            }
        }
        */
    }

    // Advanced BoxPlot section
    if(!qstricmp(szSection,"adv_boxplot_ex"))
    {
        /*
        if(!qstricmp(szField,"chart_size"))
        {
            // Sets fields correlation chart size
            if(!qstricmp(szValue,"small"))
            {
                ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_SMALL;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"medium"))
            {
                ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_MEDIUM;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"large"))
            {
                ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_LARGE;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"auto"))
            {
                ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_AUTO;
                return 1;	// No error.
            }
        }
        */

        if(!qstricmp(szField,"marker"))
        {
            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "test_name" << "title" << "mean" << "median" << "limits" << "speclimits";
            qstrlPossibleValueList << "2sigma" << "3sigma" << "6sigma" << "12sigma";


            if(qstrlPossibleValueList.contains(strValue))
            {
                if(strValue == QString("title"))
                    strValue = QString("test_name");

                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }


            strMessage += ": Unknown 'adv_boxplot_ex', 'marker' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;


            // Sets Advanced correlation markers
            /*		CRO_cleanUp
            if(!qstricmp(szValue,"test_name")  || !qstricmp(szValue,"title"))
            {
                ReportOptions.bBoxPlotExMarkerTitle = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                ReportOptions.bBoxPlotExMarkerMean = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"median"))
            {
                // Only supported by Examinator Professional edition!
                if(GS::LPPlugin::ProductInfo::getInstance()->isProfessionalEdition())
                    ReportOptions.bBoxPlotExMarkerMedian = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.bBoxPlotExMarkerLimits = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"speclimits"))
            {
                ReportOptions.bBoxPlotExMarkerSpecLimits = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"2sigma"))
            {
                ReportOptions.bBoxPlotExMarker2Sigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"3sigma"))
            {
                ReportOptions.bBoxPlotExMarker3Sigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"6sigma"))
            {
                ReportOptions.bBoxPlotExMarker6Sigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"12sigma"))
            {
                ReportOptions.bBoxPlotExMarker12Sigma = true;
                return 1;	// No error.
            }
            */
        }

        /*
        if(!qstricmp(szField,"orientation"))
        {
            // Sets Advanced correlation markers
            if(!qstricmp(szValue,"vertical"))
            {
                ReportOptions.bBoxPlotExVertical = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"horizontal"))
            {
                ReportOptions.bBoxPlotExVertical = false;
                return 1;	// No error.
            }
        }
        */
    }

    // Advanced PRODUCTION YIELD/VOLUME section
    if(!qstricmp(szSection,"adv_production_yield"))
    {
        if(!qstricmp(szField,"chart_type"))
        {
            // Plot Yield chart only
            if(!qstricmp(szValue,"yield"))
            {
                // ReportOptions.bProdYieldChart= true;
                // ReportOptions.bProdVolumeChart= false;
                ReportOptions.SetOption("adv_production_yield","chart_type","yield");
                return 1;	// No error.
            }
            // Plot Volume chart only
            if(!qstricmp(szValue,"volume"))
            {
                // ReportOptions.bProdVolumeChart= true;
                // ReportOptions.bProdYieldChart= false;
                ReportOptions.SetOption("adv_production_yield","chart_type","volume");
                return 1;	// No error.
            }
            // Plot Yield/Volume combined chart
            if(!qstricmp(szValue,"yield_volume"))
            {
                // ReportOptions.bProdVolumeChart= true;
                // ReportOptions.bProdYieldChart= true;
                ReportOptions.SetOption("adv_production_yield","chart_type","yield_volume");
                return 1;	// No error.
            }
        }
        if(!qstricmp(szField,"marker"))
        {
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;
            qstrlPossibleValueList << "title" << "yield" << "volume";

            if(qstrlPossibleValueList.contains(strValue))
            {
                addFlagOption(szSection, szField, szValue);
                return 1;
            }


            /* CRO_cleanUp
            // Display title
            if(!qstricmp(szValue,"title"))
            {
                ReportOptions.bProdMarkerTitle= true;
                return 1;	// No error.
            }
            // Display Yield percentage for each data point
            if(!qstricmp(szValue,"yield"))
            {
                ReportOptions.bProdMarkerYield= true;
                return 1;	// No error.
            }
            // Display volume couont for each bar
            if(!qstricmp(szValue,"volume"))
            {
                ReportOptions.bProdMarkerVolume= true;
                return 1;	// No error.
            }
            */
        }
    }

    // Gage R&R section
    if(!qstricmp(szSection,"adv_boxplot"))
    {
        if(!qstricmp(szField,"field"))
        {
            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "limits" << "mean" << "sigma" << "median" << "cp" << "cpk";
            qstrlPossibleValueList << "repeatability" << "ev" << "av" << "r&r" << "gb" << "pv" << "tv" << "p_t";

            if(qstrlPossibleValueList.contains(strValue))
            {
                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }


            strMessage += ": Unknown 'adv_boxplot', 'field' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;




            // Sets fields to list boxplot
            /*		CRO_cleanUp
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.bBoxplotTestLimits = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                ReportOptions.bBoxplotMean = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"sigma"))
            {
                ReportOptions.bBoxplotSigma = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"median"))
            {
                ReportOptions.bBoxplotMedian = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cp"))
            {
                ReportOptions.bBoxplotCp = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cpk"))
            {
                ReportOptions.bBoxplotCpk = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"repeatability"))
            {
                ReportOptions.bBoxplotRepeatability = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"ev"))
            {
                ReportOptions.bBoxplotRepeatabilityEV = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"av"))
            {
                ReportOptions.bBoxplotReproducibilityAV = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"r&r"))
            {
                ReportOptions.bBoxplotR_and_R = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"gb"))
            {
                ReportOptions.bBoxplotGB = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"pv"))
            {
                ReportOptions.bBoxplotPV = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"tv"))
            {
                ReportOptions.bBoxplotTV = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"p_t"))
            {
                ReportOptions.bBoxplotP_T = true;
                return 1;	// No error.
            }
            */
        }

        if(!qstricmp(szField,"chart_type"))
        {
            // Sets boxplot charting mode to: Over Limits
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.SetOption("adv_boxplot","chart_type","limits");
                // ReportOptions.iBoxplotCharting = GEX_BOXPLOTTYPE_LIMITS;
                return 1;	// No error.
            }
            // Sets boxplot charting mode to: Over Range
            if(!qstricmp(szValue,"range"))
            {
                ReportOptions.SetOption("adv_boxplot","chart_type","range");
                // ReportOptions.iBoxplotCharting = GEX_BOXPLOTTYPE_RANGE;
                return 1;	// No error.
            }
            // Sets boxplot charting mode to: Adaptive (over Limits if one dataset, over range otherwise)
            if(!qstricmp(szValue,"adaptive"))
            {
                ReportOptions.SetOption("adv_boxplot","chart_type","adaptive");
                // ReportOptions.iBoxplotCharting = GEX_BOXPLOTTYPE_ADAPTIVE;
                return 1;	// No error.
            }
        }
        if(!qstricmp(szField,"sorting"))
        {
            // Sets boxplot table sorting field
            // CRO_cleanUp
            if(!qstricmp(szValue,"test_number"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_TESTNUMBER;
                ReportOptions.SetOption("adv_boxplot","sorting","test_number");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"test_name"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_TESTNAME;
                ReportOptions.SetOption("adv_boxplot","sorting","test_name");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"test_flow_id"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_TESTFLOWID;
                ReportOptions.SetOption("adv_boxplot","sorting","test_flow_id");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"mean"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_MEAN;
                ReportOptions.SetOption("adv_boxplot","sorting","mean");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"sigma"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_SIGMA;
                ReportOptions.SetOption("adv_boxplot","sorting","sigma");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cp"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_CP;
                ReportOptions.SetOption("adv_boxplot","sorting","cp");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"cpk"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_CPK;
                ReportOptions.SetOption("adv_boxplot","sorting","cpk");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"r&r"))
            {
                // ReportOptions.iBoxplotSorting = GEX_SORTING_R_R;
                ReportOptions.SetOption("adv_boxplot","sorting","r&r");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"correlation"))
            {
                // ignored: no longer supported!
                return 1;	// No error.
            }
        }
        /*
        if(!qstricmp(szField,"r&r_sigma"))
        {
            // Get R&R N*Sigma rule (default is N=5.15)
            if(sscanf(szValue,"%lf",&ReportOptions.lfR_R_Nsigma) != 1)
                ReportOptions.lfR_R_Nsigma = 5.15;
            if(ReportOptions.lfR_R_Nsigma < 1)
                ReportOptions.lfR_R_Nsigma  = 1;
            if(ReportOptions.lfR_R_Nsigma > 12)
                ReportOptions.lfR_R_Nsigma = 12;
            return 1;	// No error.
        }
        */

        /*
        if(!qstricmp(szField,"direction"))
        {
            if(!qstricmp(szValue,"vertical"))
            {
                ReportOptions.bBoxPlotExVertical = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"horizontal"))
            {
                ReportOptions.bBoxPlotExVertical = false;
                return 1;	// No error.
            }
        }
        */

        if(!qstricmp(szField,"%"))
        {
            if(!qstricmp(szValue,"over_tv"))
            {
                //ReportOptions.bBoxplotShiftOverTV = true;
                ReportOptions.SetOption(szSection, "delta", szValue);
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"over_limits"))
            {
                //ReportOptions.bBoxplotShiftOverTV = false;
                ReportOptions.SetOption(szSection, "delta", szValue);
                return 1;	// No error.
            }
            WriteScriptMessage(QString("warning : unknown option '%1' '%2'!").arg(szSection).arg(szField), true);
            return 0;
        }
    }

    // Pareto section
    if(!qstricmp(szSection,"pareto"))
    {
        if(!qstricmp(szField,"section_name"))
        {
            /* CRO_cleanup
            ReportOptions.strParetoSectionTitle = szValue;
            */
            ReportOptions.SetOption(szSection, szField, szValue);
            return 1;
        }

        if(!qstricmp(szField,"excludebinnings"))
        {
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;
            qstrlPossibleValueList << "pass" << "fail";

            if(qstrlPossibleValueList.contains(strValue))
            {
                ReportOptions.SetSpecificFlagOption(szSection, szField, strValue, true);
                return 1;
            }

            WriteScriptMessage(QString("warning : unknown option '%1' '%2'!").arg(szSection).arg(szField), true);
            return 0;

            /* CRO_cleanup
            if(!qstricmp(szValue,"pass"))
            {
                ReportOptions.bParetoExcludePassBins = true;
                return 1;
            }
            if(!qstricmp(szValue,"fail"))
            {
                ReportOptions.bParetoExcludeFailBins = true;
                return 1;
            }
            */
        }


        if(!qstricmp(szField,"section"))
        {
            // Pareto sections to list (bits power of 2)
            long iParetoSection;
            int nScanfRslt;


            if(!qstricmp(szValue,"enabled"))
            {
                // Include section in report
                ReportOptions.SetOption(QString("pareto"), QString("section"), QString("cp|cpk|failures|failure_signature|soft_bin|hard_bin"));
                return 1;
            }
            if(!qstricmp(szValue,"disabled"))
            {
                // Do NOT nclude section in report
                ReportOptions.SetOption(QString("pareto"), QString("section"), QString(""));
                return 1;
            }

            nScanfRslt = sscanf(szValue,"%ld",&iParetoSection);
            if(nScanfRslt!=1)
                return 0;
            else
            {
                if(iParetoSection&1)
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cp"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cp"), false);

                if(iParetoSection&2)
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cpk"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("cpk"), false);

                if(iParetoSection&4)
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failures"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failures"), false);

                if(iParetoSection&8)
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failure_signature"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("failure_signature"), false);

                if(iParetoSection&16)
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("soft_bin"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("soft_bin"), false);

                if(iParetoSection&32)
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("hard_bin"), true);
                else
                    ReportOptions.SetSpecificFlagOption(QString("pareto"), QString("section"), QString("hard_bin"), false);

                return 1;
            }


            /*		CRO_cleanUp

            //// Pareto sections to list (bits power of 2)
            //#define	GEX_OPTION_PARETO_CP			0x1		// Display 'Cp pareto' section
            //#define	GEX_OPTION_PARETO_CPK			0x2		// Display 'Cpk pareto' section
            //#define	GEX_OPTION_PARETO_FAIL			0x4		// Display 'Failures pareto' section
            //#define	GEX_OPTION_PARETO_FAILSIG		0x8		// Display 'Failures signature' section
            //#define	GEX_OPTION_PARETO_SBIN			0x10	// Display 'Soft bin pareto' section
            //#define	GEX_OPTION_PARETO_HBIN			0x20	// Display 'Soft bin pareto' section
            //#define	GEX_OPTION_PARETO_ALL			0x3F	// Display all pareto sections


            if(!qstricmp(szValue,"enabled"))
            {
                // Include section in report
                ReportOptions.iParetoSection = GEX_OPTION_PARETO_ALL;
                return 1;
            }
            if(!qstricmp(szValue,"disabled"))
            {
                // Do NOT nclude section in report
                ReportOptions.iParetoSection = 0;
                return 1;
            }
            // Check if integer value specified...
            if(sscanf(szValue,"%d",&ReportOptions.iParetoSection) == 1)
                return 1;
            */
        }

        /*
        if(!qstricmp(szField,"cutoff_cp"))
        {
            // Sets Cp cutoff limit
            if(sscanf(szValue,"%lf",&ReportOptions.lfParetoCpCutoff) != 1)
                ReportOptions.lfParetoCpCutoff = -1;	// No cutoff, report ALL cp results
            return 1;
        }
        */

        /*
        if(!qstricmp(szField,"cutoff_cpk"))
        {
            // Sets Cpk cutoff limit
            if(sscanf(szValue,"%lf",&ReportOptions.lfParetoCpkCutoff) != 1)
                ReportOptions.lfParetoCpkCutoff = -1;	// No cutoff, report ALL cpk results
            return 1;
        }
        */
        /*
        if(!qstricmp(szField,"cutoff_failure"))
        {
            // Set maximum failures to report
            if(sscanf(szValue,"%d",&ReportOptions.iParetoFailCutoff) != 1)
                ReportOptions.iParetoFailCutoff = -1;	// No cutoff, report ALL failures
            else
                if(ReportOptions.iParetoFailCutoff < 0)
                    ReportOptions.iParetoFailCutoff = -1;	// Report ALL
            return 1;
        }
        */
        if(!qstricmp(szField,"cutoff_signature_failure"))  // CRO_cleanup
        {
            double lfParetoFailPatternCutoff;
            // Sets % of signature failures to report
            if ((sscanf(szValue,"%lf",&lfParetoFailPatternCutoff) != 1) ||
                    (lfParetoFailPatternCutoff < 0) ||
                    (lfParetoFailPatternCutoff > 100))
            {
                WriteScriptMessage(QString("warning : unknown option '%1' '%2'!").arg(szSection).arg(szField), true);
                return 0;
            }
            else
                ReportOptions.SetOption(szSection, szField, QString::number(lfParetoFailPatternCutoff));


            /* CRO_cleanUp
            if(sscanf(szValue,"%lf",&ReportOptions.lfParetoFailPatternCutoff) != 1)
                ReportOptions.lfParetoFailPatternCutoff = -1;	// No cutoff, report ALL failing signatures
            else
                if(ReportOptions.lfParetoFailPatternCutoff < 0)
                    ReportOptions.lfParetoFailPatternCutoff = -1;	// Report ALL
            else
                if(ReportOptions.lfParetoFailPatternCutoff > 100)
                    ReportOptions.lfParetoFailPatternCutoff = 100;
            */
            return 1;
        }
    }

    // Binning section
    if(!qstricmp(szSection,"binning"))
    {
        if(!qstricmp(szField,"section_name"))
        {
            /* CRO_cleanup
            ReportOptions.strBinningSectionTitle = szValue;
            */
            ReportOptions.SetOption(szSection, szField, szValue);
            return 1;
        }

        if(!qstricmp(szField,"section"))
        {
            if(!qstricmp(szValue,"enabled"))
            {
                // Include section in report
                // ReportOptions.bBinningSection = true;
                ReportOptions.SetOption("binning","section","enabled");

                return 1;
            }
            if(!qstricmp(szValue,"disabled"))
            {
                // Do NOT nclude section in report
                // ReportOptions.bBinningSection = false;
                ReportOptions.SetOption("binning","section","disabled");

                return 1;
            }


            /*
            if(!qstricmp(szValue,"enabled"))
            {
                // Include section in report
                ReportOptions.bBinningSection = true;
                return 1;
            }
            if(!qstricmp(szValue,"disabled"))
            {
                // Do NOT nclude section in report
                ReportOptions.bBinningSection = false;
                return 1;
            }
            */
        }

        if(!qstricmp(szField,"computation"))
        {
            if(!qstricmp(szValue,"wafer_map"))
            {
                // ReportOptions.bBinningUseWafermapOnly= true;
                // ReportOptions.bBinningUseSamplesOnly= false;
                ReportOptions.SetOption("binning","computation","wafer_map");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"summary"))
            {
                // ReportOptions.bBinningUseWafermapOnly= false;
                // ReportOptions.bBinningUseSamplesOnly= false;
                ReportOptions.SetOption("binning","computation","summary");
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"samples"))
            {
                // ReportOptions.bBinningUseWafermapOnly= false;
                // ReportOptions.bBinningUseSamplesOnly= true;
                ReportOptions.SetOption("binning","computation","samples");
                return 1;	// No error.
            }
        }
    }

    // Advanced DATALOG section
    if(!qstricmp(szSection,"adv_datalog"))
    {
        if(!qstricmp(szField,"field"))
        {
            // CRO_cleanUp
            QString strSection = QString(szSection);
            QString strField = QString(szField);
            QString strValue = QString(szValue);
            QStringList qstrlPossibleValueList;

            qstrlPossibleValueList << "comment" << "test_number" << "test_name" << "limits" << "die_loc";

            if(qstrlPossibleValueList.contains(strValue))
            {
                ReportOptions.SetSpecificFlagOption( strSection, strField, strValue, true);
                return 1;
            }


            strMessage += ": Unknown 'adv_datalog', 'field' option";
            WriteScriptMessage(QString("warning : unknown option '%1' '%2' '%3'!").arg(szSection).arg(szField).arg(szValue),true);
            return 0;



            // Sets fields to list in datalog
            /* CRO_cleanUp
            if(!qstricmp(szValue,"comment"))
            {
                ReportOptions.bDatalogTableComments = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"test_number"))
            {
                ReportOptions.bDatalogTableTestNumber= true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"test_name"))
            {
                ReportOptions.bDatalogTableTestName = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"limits"))
            {
                ReportOptions.bDatalogTableLimits = true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"die_loc"))
            {
                ReportOptions.bDatalogTableDieXY = true;
                return 1;	// No error.
            }
            */
        }
        /*
        if(!qstricmp(szField,"format"))
        {
            if(!qstricmp(szValue,"1row"))
            {
                ReportOptions.bDatalogSingleRow= true;
                return 1;	// No error.
            }
            if(!qstricmp(szValue,"2rows"))
            {
                ReportOptions.bDatalogSingleRow= false;
                return 1;	// No error.
            }
        }
        */
    }

    // Wha-if section
    if(!qstricmp(szSection,"adv_what_if"))
    {
        if(!qstricmp(szField,"pass_bin"))
        {
            // Sets What-if Passing bin value
            /* CRO_cleanUp by HTH case# 4236
            if(sscanf(szValue,"%d",&ReportOptions.iWhatIf_PassBin) != 1)
                ReportOptions.iWhatIf_PassBin = 1;	// Default
            */
            int nPassBin = 1;

            if(sscanf(szValue, "%d", &nPassBin) != 1)
                nPassBin = 1;	// Default

            ReportOptions.SetOption("adv_what_if", "pass_bin", QString::number(nPassBin));
            return 1;
        }
        if(!qstricmp(szField,"fail_bin"))
        {
            // Sets What-if Failing bin value
            /* CRO_cleanUp by HTH case# 4236
            if(sscanf(szValue,"%d",&ReportOptions.iWhatIf_FailBin) != 1)
                ReportOptions.iWhatIf_FailBin = 0;	// Default
            */
            int nFailBin = 0;

            if(sscanf(szValue, "%d", &nFailBin) != 1)
                nFailBin = 1;	// Default

            ReportOptions.SetOption("adv_what_if", "fail_bin", QString::number(nFailBin));
            return 1;
        }
        if(!qstricmp(szField,"fail_bin_is_pass"))
        {
            // Sets What-if Failing bin as a PASS bin type.
            int iValue;
            if(sscanf(szValue,"%d",&iValue) != 1)
                iValue = 0;	// Default

            /* CRO_cleanUp by HTH case# 4236
            ReportOptions.bWhatIf_FailBinIsPass = (bool) iValue;
            */

            ReportOptions.SetOption("adv_what_if", "fail_bin_is_pass", (iValue == 0) ? "false" : "true");
            return 1;
        }
    }

    // Pearson correlation section
    if(!qstricmp(szSection,"adv_pearson"))
    {
        if(!qstricmp(szField,"cutoff"))
        {
            // Sets Pearson's cutoff limit
            /* CRO_cleanUp by HTH case# 4237
            if(sscanf(szValue,"%lf",&ReportOptions.lfPearsonCutoff) != 1)
                ReportOptions.lfPearsonCutoff = 0.8;	// Default
            */

            double dPearsonCutOff;

            if(sscanf(szValue,"%lf", &dPearsonCutOff) != 1)
                dPearsonCutOff = 0.8;	// Default

            ReportOptions.SetOption("adv_pearson", "cutoff", QString::number(dPearsonCutOff));
            return 1;
        }
        if(!qstricmp(szField,"sorting"))
        {
            // Sets Pearson's sorting mode
            if(!qstricmp(szValue,"test_name"))
            {
                ReportOptions.SetOption("adv_pearson", "sorting", "test_name");
                return 1;
            }
            if(!qstricmp(szValue,"pearson"))
            {
                ReportOptions.SetOption("adv_pearson", "sorting", "pearson_dsc");
                return 1;
            }
        }
    }

    // Pearson correlation section
    if(!qstricmp(szSection,"toolbox"))
    {
        if(!qstricmp(szField,"csv_sorting"))
        {
            // Sets CSV export parser sorting mode
            if(!qstricmp(szValue,"flow_id"))
            {
                /* CRO_cleanUp by HTH case# 4238
                ReportOptions.bToolboxCsvSortTestFlow = true;
                */

                ReportOptions.SetOption("toolbox", "csv_sorting", "flow_id");
                return 1;
            }
            if(!qstricmp(szValue,"test_id"))
            {
                /* CRO_cleanUp by HTH case# 4238
                ReportOptions.bToolboxCsvSortTestFlow = false;
                */

                ReportOptions.SetOption("toolbox", "csv_sorting", "test_id");
                return 1;
            }
        }
        if(!qstricmp(szField,"csv_split_export"))
        {
            // Enabled/Diusabled splitting CSV generation into 250-rows CSV files.
            if(!qstricmp(szValue,"on"))
            {
                /* CRO_cleanUp by HTH case# 4238
                ReportOptions.bToolboxCsvSplitExport = true;
                */

                ReportOptions.SetOption("toolbox", "csv_split_export", "true");
                return 1;
            }
            if(!qstricmp(szValue,"off"))
            {
                /* CRO_cleanUp by HTH case# 4238
                ReportOptions.bToolboxCsvSplitExport = false;
                */

                ReportOptions.SetOption("toolbox", "csv_split_export", "false");
                return 1;
            }
        }
        if(!qstricmp(szField,"csv_units_mode"))
        {
            // Sets CSV export Normalized units
            if(!qstricmp(szValue,"normalized"))
            {
                /* CRO_cleanUp by HTH case# 4238
                ReportOptions.nToolboxCsvUnitsMode = GEX_OPTIONS_TOOLBOX_UNITS_NORMALIZED;
                */

                ReportOptions.SetOption("toolbox", "csv_units_mode", "normalized");
                return 1;
            }
            if(!qstricmp(szValue,"scaling_factor"))
            {
                /* CRO_cleanUp by HTH case# 4238
                ReportOptions.nToolboxCsvUnitsMode = GEX_OPTIONS_TOOLBOX_UNITS_SCALING_FACTOR;
                */

                ReportOptions.SetOption("toolbox", "csv_units_mode", "scaling_factor");
                return 1;
            }
        }
    }

    // Application Preferences section
    if(!qstricmp(szSection,"preferences"))
    {
        // Define custom text editor.
        if(!qstricmp(szField,"text_editor"))
        {
            if(!qstricmp(szValue,"(default)"))
            {
#if (defined __sun__ || __hpux__)
                ReportOptions.SetOption("preferences", "text_editor", "textedit");	//ReportOptions.strTextEditor = "textedit";	// Editor under Solaris & HP-UX
#elif (defined __linux__)
                ReportOptions.SetOption("preferences", "text_editor", "gedit");	//ReportOptions.strTextEditor = "gedit";	// Editor under Linux
#else
                ReportOptions.SetOption("preferences", "text_editor", "wordpad"); //ReportOptions.strTextEditor = "wordpad";	// Editor under windows
#endif
                return 1;	// No error.
            }
            else
            {
                ReportOptions.SetOption("preferences", "text_editor", szValue); //ReportOptions.strTextEditor = szValue;
                return 1;	// No error.
            }
        }
        // Define custom SpreadSheet editor.
        if(!qstricmp(szField,"ssheet_editor"))
        {
            if(!qstricmp(szValue,"(default)"))
            {
#if (defined __sun__ || __hpux__)
                ReportOptions.SetOption("preferences","ssheet_editor","textedit");  	//ReportOptions.m_PrefMap["ssheet_editor"]="textedit"; // Editor under Solaris & HP-UX
#elif (defined __linux__)
                ReportOptions.SetOption("preferences","ssheet_editor","oocalc"); //ReportOptions.m_PrefMap["ssheet_editor"]="oocalc"; // Editor under Linux
#else
                ReportOptions.SetOption("preferences","ssheet_editor","excel"); //ReportOptions.m_PrefMap["ssheet_editor"]="excel";	// Editor under windows
#endif
                return 1;	// No error.
            }
            else
            {
                ReportOptions.SetOption("preferences","ssheet_editor",szValue); //ReportOptions.m_PrefMap["ssheet_editor"]=szValue;
                return 1;	// No error.
            }
        }
        // Define custom PDF editor.
        if(!qstricmp(szField,"pdf_editor"))
        {
            if(!qstricmp(szValue,"(default)"))
            {
#if (defined __sun__ || __hpux__)
                ReportOptions.SetOption("preferences","pdf_editor","gpdf");  	// PDF under Solaris & HP-UX
#elif (defined __linux__)
                ReportOptions.SetOption("preferences","pdf_editor","evince"); // PDF under Linux
#else
                ReportOptions.SetOption("preferences","pdf_editor","AcroRd32.exe");// PDF under windows
#endif
                return 1;	// No error.
            }
            else
            {
                ReportOptions.SetOption("preferences","pdf_editor",szValue);
                return 1;	// No error.
            }
        }

        //ReportOptions.SetOption("preferences",QString(szField), QString(szValue) );
        //return 1;
    }


    // Build/No build report section
    if(!qstricmp(szSection,"report"))
    {
        if(!qstricmp(szField,"build"))
        {
            // Flag to tell if Build report or not after data query.
            int iBuildReport=1;
            bool bSetOptionRslt = true;		// to reproduce old behaviour

            sscanf(szValue,"%d",&iBuildReport);
            if(iBuildReport)
            {
                bSetOptionRslt = ReportOptions.SetOption(QString("report"), QString("build"), QString("true"));
                //		CRO_cleanUp
                // ReportOptions.bBuildReport = true;
            }
            else
            {
                bSetOptionRslt = ReportOptions.SetOption(QString("report"), QString("build"), QString("false"));
                //		CRO_cleanUp
                // ReportOptions.bBuildReport = false;
            }
            return bSetOptionRslt;
        }
    }

    // Charting style section (Interactive Charts)
    if(!qstricmp(szSection,"chart_style"))
    {
        return gexChartStyle(szField, szValue);

        /*int	iR,iG,iB;
        int iWidth;

        // Chart background color
        if(!qstricmp(szField,"bkg_color"))
        {
            sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
            ReportOptions.cBkgColor = QColor(iR,iG,iB);
            return 1;	// No error.
        }

        // Overlay curve's name on chart?
        if(!qstricmp(szField,"show_legend"))
        {
            if(!qstricmp(szValue,"1"))
                ReportOptions.bPlotLegend = true;
            else
                ReportOptions.bPlotLegend = false;
            return 1;	// No error.
        }

        // Plot Bars (applies to histograms only)
        if(!qstricmp(szField,"box_bars"))
        {
            if(!qstricmp(szValue,"1"))
                s_pChartStyle->bBoxBars = true;
            else
                s_pChartStyle->bBoxBars = false;
            return 1;	// No error.
        }
        if(!qstricmp(szField,"box_3d_bars"))
        {
            if(!qstricmp(szValue,"1"))
                s_pChartStyle->bBox3DBars = true;
            else
                s_pChartStyle->bBox3DBars = false;
            return 1;	// No error.
        }

        // Plot fitting curve
        if(!qstricmp(szField,"fitting_curve"))
        {
            if(!qstricmp(szValue,"1"))
                s_pChartStyle->bFittingCurve = true;
            else
                s_pChartStyle->bFittingCurve = false;
            return 1;	// No error.
        }

        // Plot gaussian / bell curve
        if(!qstricmp(szField,"bell_curve"))
        {
            if(!qstricmp(szValue,"1"))
                s_pChartStyle->bBellCurve = true;
            else
                s_pChartStyle->bBellCurve = false;
            return 1;	// No error.
        }

        // Box-Plot whisker type: Range or Q2 +/- 1.5*IQR
        if(!qstricmp(szField,"box_whisker"))
        {
            if(!qstricmp(szValue,"range"))
            {
                s_pChartStyle->iWhiskerMode= GEX_WHISKER_RANGE;	// Range
                return 1;	// No error.
            }
            else
                if(!qstricmp(szValue,"q1q3"))
                {
                s_pChartStyle->iWhiskerMode= GEX_WHISKER_Q1Q3;	// Q1-1.5*IQR, Q3+1.5*IQR
                return 1;	// No error.
            }
            else
                if(!qstricmp(szValue,"iqr"))
                {
                s_pChartStyle->iWhiskerMode= GEX_WHISKER_IQR;	// Median+/-1.5IQR
                return 1;	// No error.
            }
        }

        // Plot lines
        if(!qstricmp(szField,"lines"))
        {
            if(!qstricmp(szValue,"1"))
                s_pChartStyle->bLines = true;
            else
                s_pChartStyle->bLines = false;
            return 1;	// No error.
        }

        // Plot Spots
        if(!qstricmp(szField,"spots"))
        {
            if(!qstricmp(szValue,"1"))
                s_pChartStyle->bSpots = true;
            else
                s_pChartStyle->bSpots = false;
            return 1;	// No error.
        }

        // Get line width
        if(!qstricmp(szField,"line_width"))
        {
            sscanf(szValue,"%d",&s_pChartStyle->iLineWidth);
            return 1;	// No error.
        }

        // Get line style (solid, dashed,...)
        if(!qstricmp(szField,"line_style"))
        {
            sscanf(szValue,"%d",&s_pChartStyle->iLineStyle);
            return 1;	// No error.
        }

        // Get spot style (cross, triangle, round,...)
        if(!qstricmp(szField,"spot_style"))
        {
            sscanf(szValue,"%d",&s_pChartStyle->iSpotStyle);
            return 1;	// No error.
        }

        // Get charting RGB colors
        if(!qstricmp(szField,"rgb_color"))
        {
            sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
            s_pChartStyle->cColor = QColor(iR,iG,iB);
            return 1;	// No error.
        }

        ///////////////////////////////////////
        // Markers
        ///////////////////////////////////////

        // Mean marker: line width & color
        if(!qstricmp(szField,"marker_mean"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->setMeanLineWidth(iWidth);
            s_pChartStyle->setMeanColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // Median marker: line width & color
        if(!qstricmp(szField,"marker_median"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth, &iR,&iG,&iB);
            s_pChartStyle->setMedianLineWidth(iWidth);
            s_pChartStyle->setMedianColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // MIN marker: line width & color
        if(!qstricmp(szField,"marker_min"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->setMinLineWidth(iWidth);
            s_pChartStyle->setMinColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // MAX marker: line width & color
        if(!qstricmp(szField,"marker_max"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->setMaxLineWidth(iWidth);
            s_pChartStyle->setMaxColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // Limits marker: line width & color
        if(!qstricmp(szField,"marker_limits"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->setLimitsLineWidth(iWidth);
            s_pChartStyle->setLimitsColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // 2sigma marker: line width & color
        if(!qstricmp(szField,"marker_2sigma"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->set2SigmaLineWidth(iWidth);
            s_pChartStyle->set2SigmaColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // 3sigma marker: line width & color
        if(!qstricmp(szField,"marker_3sigma"))
        {
            sscanf(szValue,"%d %d %d %d ",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->set3SigmaLineWidth(iWidth);
            s_pChartStyle->set3SigmaColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // 6sigma marker: line width & color
        if(!qstricmp(szField,"marker_6sigma"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->set6SigmaLineWidth(iWidth);
            s_pChartStyle->set6SigmaColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        // 12sigma marker: line width & color
        if(!qstricmp(szField,"marker_12sigma"))
        {
            sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
            s_pChartStyle->set12SigmaLineWidth(iWidth);
            s_pChartStyle->set12SigmaColor(QColor(iR,iG,iB));
            return 1;	// No error.
        }

        if(!qstricmp(szField,"chart_layer"))
        {
            // Get Layer ID
            sscanf(szValue,"%d",&iR);

            // Check if reset layer list
            if(iR < 0)
            {
                while (!ReportOptions.pLayersStyleList.isEmpty())
                    delete ReportOptions.pLayersStyleList.takeFirst();

                return 1;
            }

            // Allocate structure to copy style info
            CGexSingleChart	* pChartStyle = new CGexSingleChart(*s_pChartStyle);
//			*pChartStyle = *s_pChartStyle;

            // Add structure to list of styles.
            ReportOptions.pLayersStyleList.append(pChartStyle);
            return 1;	// No error.
        }*/
    }

    // Binning style section (custom bin colors to be used in Binning & Wafer map reports)
    if(!qstricmp(szSection,"bin_style"))
    {
        return gexBinStyle(szField, szValue);
        /*
        int	iR,iG,iB;

        // Empty the list of custom style.
        if(!qstricmp(szField,"clear"))
        {
            ReportOptions.hardBinColorList.clear();
            ReportOptions.softBinColorList.clear();

            return 1;	// No error.
        }

        // Tells if custom colors to be used or not.
        if(!qstricmp(szField,"custom_colors"))
        {
            if(!qstricmp(szValue,"1"))
                ReportOptions.bUseCustomBinColors = true;
            else
                ReportOptions.bUseCustomBinColors = false;
            return 1;	// No error.
        }

        // Read The Bin# list
        if(!qstricmp(szField,"bin_list"))
        {
            cBinColor.cBinRange = new CGexRange(szValue);
            return 1;	// No error.
        }

        // Read The Bin RGB color
        if(!qstricmp(szField,"bin_color"))
        {
            sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
            cBinColor.cBinColor = QColor(iR,iG,iB);
            return 1;	// No error.
        }

        // Insert the BinColor definition to the Hard bin or Soft bin list
        if(!qstricmp(szField,"add_bin"))
        {
            if(!qstricmp(szValue,"soft_bin"))
                ReportOptions.softBinColorList.append(cBinColor);
            else
                ReportOptions.hardBinColorList.append(cBinColor);
            return 1;	// No error.
        }*/
    }


    // GLOBAL INFO section
    /*
    if(!qstricmp(szSection,"global_info"))
    {
        if(!qstricmp(szField,"detail_level"))
        {
            if(!qstricmp(szValue,"summarized"))
            {
                // ReportOptions.iGlobalInfoDetailLevel = GEX_OPTION_GLOBAL_INFO_SUMMARIZED;
                ReportOptions.SetOption("global_info","detail_level","summarized");
                return 1;	// No error.
            }
            else
                if(!qstricmp(szValue,"detailed"))
                {
                // ReportOptions.iGlobalInfoDetailLevel = GEX_OPTION_GLOBAL_INFO_DETAILED;
                ReportOptions.SetOption("global_info","detail_level","detailed");
                return 1;	// No error.
            }
        }
    }
    */

    GSLOG(SYSLOG_SEV_DEBUG, QString(" setting option : '%1', '%2' to '%3'").arg(szSection).arg(szField).arg(szValue)
           .toLatin1().data() );
    ReportOptions.SetOption(QString(szSection), QString(szField), QString(szValue) );

    //strMessage += ": option."; //strMessage += ": Invalid parameter";
    WriteScriptMessage(strMessage,false);
    return 1; // Now : accepts all options even if not used inside Gex
    //return 0;
} // gexOptions

