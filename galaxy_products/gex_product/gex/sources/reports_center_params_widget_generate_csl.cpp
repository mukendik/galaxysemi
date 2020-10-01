#include "reports_center_params_widget.h"
#include "reports_center_multifields.h"
#include "gex_constants.h"
#include "browser_dialog.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "settings_dialog.h"
#include "report_options.h"
#include <gqtl_log.h>
#include "csl/csl_engine.h"

// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);
// extern void ConvertFromScriptString(QString &strFile);		Not used
extern GexMainwindow *	pGexMainWindow;
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

QString CReportsCenterParamsWidget::GenerateCSL(QString of)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" grxml=%1 of=%2 ").
          arg(m_grxml_filename).arg(of).toLatin1().constData());

    int iFlags = GEX_SCRIPT_SECTION_GROUPS | GEX_SCRIPT_SECTION_REPORT;
    iFlags |= GEX_SCRIPT_SECTION_SETTINGS | GEX_SCRIPT_SECTION_OPTIONS;
    iFlags |= GEX_SCRIPT_SECTION_DRILL;

    QString csl=m_grxml_filename.remove(".grxml", Qt::CaseInsensitive)+".csl";
    if (QFile::exists(csl))
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString(" csl file %1 detected !").arg( csl).toLatin1().constData() );
        FILE *hFile = pGexMainWindow->CreateGexScript(GEX_SCRIPT_ASSISTANT);
        if(hFile == NULL)
        {	return QString("error : cant create script file 'GEX_SCRIPT_ASSISTANT' !");
        }

        char type='m';
        if (m_atts["Type"]=="MergeDS")
            type='m';
        else if (m_atts["Type"]=="CompareDS")
            type='c';
        QString r=GenerateProcessDataCSL(hFile, of, type);
        if (!r.startsWith("ok"))
        {
            fprintf(hFile,"\n// error while generating script !\n\n");
            pGexMainWindow->CloseGexScript(hFile, iFlags);
            return r;
        }

        // Do we have to set the CSLPATH before ?
                //fprintf( hFile, (QString("\n#include '%1'\n\n").arg(csl)).toLatin1().data() );
                fprintf( hFile, "\n#include '%s'\n\n", csl.toLatin1().data() );

        fclose(hFile);

        // raw replace options ?

        return "ok";
    }

    //int iWizardType = GEX_MIXQUERY_WIZARD;
    // Writes script footer : writes 'main' function+ call to 'Favorites'+SetOptions+SetDrill

    QString startup_page("home");

    FILE *hFile=NULL;

    QString r=WriteBeginningOfCSL(hFile, startup_page);
    if (!r.startsWith("ok")||hFile==NULL)
        return r;

    char type='m';
    if (m_atts["Type"]=="MergeDS")
        type='m';
    else if (m_atts["Type"]=="CompareDS")
        type='c';
    r=GenerateProcessDataCSL(hFile, of, type);

    if (!r.startsWith("ok"))
    {
        fprintf(hFile,"\n// error while generating script !\n\n");
        pGexMainWindow->CloseGexScript(hFile, iFlags);
        return r;
    }

    // Creates 'BuildReport' section...always write this section last (so HTML section list to exclude is known).
    pGexMainWindow->iHtmlSectionsToSkip=0;
    pGexMainWindow->WriteBuildReportSection(hFile, startup_page, true);

    pGexMainWindow->CloseGexScript(hFile, iFlags);

    return "ok";
}

QString CReportsCenterParamsWidget::GenerateProcessDataCSL(FILE* hFile, QString of, char type)
{
    if (!hFile)
        return "error";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Generate ProcessData in CSL...");

    // Writes 'SetProcessData' section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// List Queries groups to process\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\tgexCslVersion('2.10');");
    fprintf(hFile,"  // %s \n", (type=='m')?"Merge mode":"Compare mode");
    fprintf(hFile,"  var group_id;  // Holds group_id when creating groups\n\n");

    // hack to impose/overload some options
    //   gexOptions('output','format','...');
        fprintf(hFile, "\n\t// small hack to impose output format from ReportsCenter\n");
        fprintf(hFile, "\tgexOptions('output','format', '%s');\n", of.toLatin1().data() );

    QString lp(ReportOptions.GetOption("databases","local_path").toString());
    ConvertToScriptString(lp);
        fprintf(hFile, "\tgexOptions('databases','local_path', '%s');\n", lp.toLatin1().data() );

    QString sp(ReportOptions.GetOption("databases","server_path").toString());
    ConvertToScriptString(sp);
        fprintf(hFile, "\tgexOptions('databases','server_path', '%s');\n", sp.toLatin1().data() );

        fprintf(hFile, "\tgexOptions('output','location','%s');\n", (ReportOptions.GetOption("output","location").toString()).toLatin1().constData());

    QString olp(ReportOptions.GetOption("output","location_path").toString());
    ConvertToScriptString(olp);
        fprintf(hFile, "\tgexOptions('output','location_path', '%s');\n", olp.toLatin1().data() );

    // Write script sequence
    fprintf(hFile,"\n  // Single query...\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    //fprintf(hFile,"  gexQuery('db_report','Analyse dataset from ReportsCenter');\n");
    fprintf(hFile,"  gexQuery('db_report','%s');\n", m_atts["Title"].toLatin1().data() );

    if (type=='m')
     fprintf(hFile,"  group_id = gexGroup('insert','My Title');\n\n");

    GexDatabaseEntry* dbe=ReportsCenterWidget::GetInstance()->GetCurrentDatabaseEntry();
    if (!dbe)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : cant retrieve current GexDatabaseEntry !");
        return "error : cant retrieve current GexDatabaseEntry";
    }

    if (dbe->IsStoredInFolderLocal())
        fprintf(hFile,"\tgexQuery('db_location','[Local]');\n" );
    else
        fprintf(hFile,"\tgexQuery('db_location','[Server]');\n" );

    fprintf(hFile,"\tgexQuery('db_name','%s');\n", dbe->LogicalName().toLatin1().data() );

    fprintf(hFile,"\tgexQuery('db_offline_query','false');\n" );
    //fprintf(hFile,"\tgexQuery('db_testlist','*');\n");

    fprintf(hFile,"\n\t// exporting %d datasets...\n\n", m_DatasetIDToDatasetMap.size() );

    QMap<QString, CReportsCenterDataset*>::iterator it=m_DatasetIDToDatasetMap.begin();
    for (it=m_DatasetIDToDatasetMap.begin(); it!=m_DatasetIDToDatasetMap.end(); it++)
    {
        if (!it.value())
            continue;
        CReportsCenterDataset* ds=(CReportsCenterDataset*)it.value();
        //QString dsid=it.value();

        QString r=ds->ExportToCsl(hFile, (type=='m')?"group_id":"");
        if (r.startsWith("error"))
        {
            GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().constData());
            return QString("error : failed to export DataSet '%1' to CSL : %2")
                    .arg(ds->m_atts["label"])
                    .arg(r);
        }
    }

    fprintf(hFile,"\n\tsysLog(' Query/queries set ! *');\n");

    fprintf(hFile,"\tgexCslVersion('1.0');");	// Hack ?

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");

    return QString("ok");
}

QString CReportsCenterParamsWidget::WriteBeginningOfCSL(FILE* &hFile, QString startup_page )
{
    if(GS::Gex::CSLEngine::GetInstance().IsRunning())
        return QString("error : script already running. Please retry later.");

    // Creates header of Gex Assistant script file.
    hFile = pGexMainWindow->CreateGexScript(GEX_SCRIPT_ASSISTANT);
    if(hFile == NULL)
        return QString("error : cant create script file 'GEX_SCRIPT_ASSISTANT' !");

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        //GEX_ASSERT(false);
        return QString("error : can't write option section");
    }

    // SetReportType() section
    //pGexMainWindow->pWizardSettings->WriteScriptReportSettingsSection(hFile,false, startup_page );
    fprintf(hFile,"\n\nSetReportType()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");

    if (m_atts["SetReportType"]!="")
    {
        fprintf(hFile,"\n\t// GRXML overloading\n\t");
        QString SRT=m_atts["SetReportType"];
        QMap<int, QMap< QString, QString> >::iterator oit=m_options_properties.begin();
        for (oit=m_options_properties.begin(); oit!=m_options_properties.end(); oit++)
        {
            QString ID=oit.value()["ID"];
            if (ID.isEmpty())
                continue;
            int pid=oit.key();
            if (pid<0)
                continue;
            ID.prepend("$O{"); ID.append("}");
            QVariant cv=m_pb->GetCurrentValue(pid);
            SRT.replace(ID, cv.toString());
        }
        fprintf(hFile, "%s", SRT.toLatin1().data() );
        fprintf(hFile,"\n" );
    }
    else
        pGexMainWindow->pWizardSettings->WriteReportSettingsSection(hFile, false, startup_page);

    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");

    //
    pGexMainWindow->pWizardSettings->WriteBuildDrillSection(hFile,false);
    return QString("ok");
}

