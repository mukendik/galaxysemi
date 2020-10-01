#ifdef GCORE15334

#include "patman_lib.h"				// List of '#define' to return type of distribution (gaussian, lognormal, etc...)
#include <gqtl_log.h>
#include "product_info.h"

#include "browser_dialog.h"

extern CGexReport *		gexReport;		// report_build.cpp: Handle to report class

///////////////////////////////////////////////////////////
// ON Button clicked "Process file"
// Note: Not available if running at Tester
///////////////////////////////////////////////////////////
QString	GexTbPatDialog::OnReProcessFile(int iFileID)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("OnReProcessFile %1, current dataset in mem : %2")
          .arg( iFileID).arg( m_iReloadDataFileID).toLatin1().constData());

    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
        return "ok";

    // Check if Dataset already loaded
    if(m_iReloadDataFileID == iFileID)
    {
        // Tell dataset already in memory!
        // Message::information(this, GS::Gex::Engine::GetInstance().
        // GetAppFullName(), "Dataset already in memory!");
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Dataset %1 already in memory").arg( iFileID).toLatin1().constData());
        return "ok : dataset already in memory";
    }

    // Readback file names from the edit box.
    QTreeWidgetItem*	pTreeWidgetItem = treeWidgetDataFiles->topLevelItem(0); //invisibleRootItem(); : Anis ?
    QString				strInputFile;
    int					iLoopIndex = 1;
    int iTreeWidgetIdx = 0;

    while(pTreeWidgetItem != NULL)
    {
        //GSLOG(SYSLOG_SEV_DEBUG, QString("treeWidgetDataFiles item %1 : %2").arg( iLoopIndex, pTreeWidgetItem->text(4)).toLatin1().constData());.arg(        //GSLOG(SYSLOG_SEV_DEBUG, "treeWidgetDataFiles item %1 : %2").arg( iLoopIndex.arg( pTreeWidgetItem->text(4)).toLatin1().constData());
        // Get file name
        int iChildCount = pTreeWidgetItem->childCount();
        if(!pTreeWidgetItem->child(0))
            continue;

        strInputFile = pTreeWidgetItem->child(0)->text(5);
        if(iChildCount> 1 ){
            QFileInfo cFileInfo(strInputFile);
            strInputFile = cFileInfo.absolutePath();
            if(strInputFile.endsWith("/") == false)
                strInputFile += "/";
            strInputFile += cFileInfo.completeBaseName();
            strInputFile += "_merged.stdf";
        }

        // Get filtering option if any (note: only last filter in list is used!)
        m_cFields.strProcessPart = pTreeWidgetItem->child(0)->text(1).trimmed();

        // Find the Type of parts to process
        int iIndex=0;
        while(ProcessPartsItems[iIndex] != 0)
        {
            if( m_cFields.strProcessPart == ProcessPartsItems[iIndex])
                break;	// found matching string
            iIndex++;
        };	// loop until we have found the string entry.

        m_cFields.strProcessPart = gexFileProcessPartsItems[iIndex];
        m_cFields.strProcessList = pTreeWidgetItem->text(3);

        // Clean input file name strings in case they come from drag & drop (start with string 'file:///')
        if(strInputFile.startsWith("file:///"))
            strInputFile = QUrl(strInputFile).toLocalFile();

        // Remove leading \r & \n if any
        strInputFile.replace("\r","");
        strInputFile.replace("\n","");
        QFileInfo cFileInfo(strInputFile);
        m_cFields.mOutputFolderSTDF = cFileInfo.absolutePath();	// Output folder for new STDF
        //strInputFile += cFileInfo.absFilePath();		// Input STDF

        // Exit now if reach relevant FileID
        if(iLoopIndex == iFileID)
            break;

        // Get next file in list.
        iLoopIndex++;
        pTreeWidgetItem = treeWidgetDataFiles->topLevelItem(++iTreeWidgetIdx);
    };

    GSLOG(SYSLOG_SEV_NOTICE, QString("Found file for id %1 : '%2'")
          .arg(iFileID)
          .arg(strInputFile).toLatin1().constData());

    if (strInputFile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cannot find a file entry for id %1").arg( iFileID).toLatin1().constData());
        return "error : Cannot find a file entry for id "+QString::number(iFileID);
    }

    // Check if valid ID
    if(iLoopIndex != iFileID)
        return "error : invalid ID";	// internal Error!

    // Update Dataset# loaded in memory
    m_iReloadDataFileID = iFileID;

    // This is the STDF file to reprocess with PAT recipe!
    m_cFields.strSources.clear();
    m_cFields.strSources << strInputFile;

    // Save current master index page
    QString strHomePage = gexReport->reportAbsFilePath();

    // Re-Process file
    QString strError;

    // 5909 : do not show home page in this case in order to load the desired page later
    int r=ProcessFile(false, m_cFields, strError);
    if (r!=NoError)
    {

        GSLOG(SYSLOG_SEV_WARNING, QString("OnReProcessFile : ProcessFile returned error %1")
              .arg( r).toLatin1().constData());
        return "error : ProcessFile failed : error "+QString::number(r);
    }

    // Reload master index page
    gexReport->setLegacyReportName(strHomePage);
    //pGexMainWindow->LoadUrl(strHomePage); // case 5909 : do not load this page now but later

    return "ok";
//#endif
}

void GexTbPatDialog::OnDetachWindow(void)
{
    ForceAttachWindow(buttonDetachWindow->isChecked());
}

void GexTbPatDialog::onItemChanged(QTreeWidgetItem* /*pTreeWidgetItem*/,
                                   int nColumn)
{
    treeWidgetDataFiles->resizeColumnToContents(nColumn);
}

#endif

