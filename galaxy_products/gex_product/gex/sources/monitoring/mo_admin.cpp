///////////////////////////////////////////////////////////
// Examinator Monitoring: Admin widget : ?
///////////////////////////////////////////////////////////
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "browser_dialog.h"
#include "engine.h"
#include "mo_admin.h"
#include "gex_constants.h"
#include "product_info.h"
#include "admin_engine.h"
#include "ym_event_log_gui.h"
#include "browser_dialog.h"
#include "scheduler_engine.h"

extern GexScriptEngine* pGexScriptEngine;

///////////////////////////////////////////////////////////
// Admin widget.
///////////////////////////////////////////////////////////
MonitoringWidget::MonitoringWidget( QWidget* parent, Qt::WindowFlags fl)
    : QWidget( parent, fl )
{
    setupUi(this);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    // Add icons to ListWidget
    QTreeWidgetItem *pQlwiItem = NULL;
    // first item
    pQlwiItem = mUi_qlwSelectionList->topLevelItem(0);
    pQlwiItem->setIcon(0,QIcon(QString(":/gex/icons/mo_admin_history.png")) );
    pQlwiItem->setText(0,QString("Local Raw History"));
    // second item
    pQlwiItem = mUi_qlwSelectionList->topLevelItem (1);
    pQlwiItem->setIcon(0,QIcon(QString(":/gex/icons/mo_admin_ftp.png")) );
    pQlwiItem->setText(0,QString("Local FTP logs"));
    // third item
    pQlwiItem = mUi_qlwSelectionList->topLevelItem (2);
    pQlwiItem->setIcon(0, QIcon(QString(":/gex/icons/mo_admin_report.png")) );
    pQlwiItem->setText(0,QString("Local Insertion Logs"));
    // fourth item
    pQlwiItem = mUi_qlwSelectionList->topLevelItem (3);
    pQlwiItem->setIcon(0, QIcon(QString(":/gex/icons/mo_admin_email.png")) );
    pQlwiItem->setText(0,QString("Local Email logs"));
    // fifth item
    pQlwiItem = mUi_qlwSelectionList->topLevelItem (4);
    m_poYMEventLogViewManager = 0;

    if(pQlwiItem){
        initLogManager(pQlwiItem, m_poEvenLogPage);
    }
    mUi_qlwSelectionList->setCurrentItem(mUi_qlwSelectionList->topLevelItem (0));
    QObject::connect(mUi_qlwSelectionList,		SIGNAL(itemSelectionChanged()), this, SLOT(OnMainSelectionChanged()));
    QObject::connect(comboFiles_History,		SIGNAL(activated(int)),		this, SLOT(OnFileSelected_History()));
    QObject::connect(buttonNext_History,		SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Next_History()));
    QObject::connect(buttonPrev_History,		SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Prev_History()));
    QObject::connect(comboFiles_GexFtp,			SIGNAL(activated(int)),		this, SLOT(OnFileSelected_GexFtp()));
    QObject::connect(buttonNext_GexFtp,			SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Next_GexFtp()));
    QObject::connect(buttonPrev_GexFtp,			SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Prev_GexFtp()));
    QObject::connect(buttonRefresh_GexFtp,		SIGNAL(clicked()),			this, SLOT(OnButtonRefresh_GexFtp()));
    QObject::connect(comboFiles_Report,			SIGNAL(activated(int)),		this, SLOT(OnFileSelected_Report()));
    QObject::connect(buttonNext_Report,			SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Next_Report()));
    QObject::connect(buttonPrev_Report,			SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Prev_Report()));
    QObject::connect(comboFiles_GexEmail,		SIGNAL(activated(int)),		this, SLOT(OnFileSelected_GexEmail()));
    QObject::connect(buttonNext_GexEmail,		SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Next_GexEmail()));
    QObject::connect(buttonPrev_GexEmail,		SIGNAL(clicked()),			this, SLOT(OnButtonSearch_Prev_GexEmail()));
    QObject::connect(buttonRefresh_GexEmail,	SIGNAL(clicked()),			this, SLOT(OnButtonRefresh_GexEmail()));

    QString	strFilter, strPixMapSource;
    QDir	dir;
    int		nIndex;

    // Load pixmaps
    strPixMapSource.sprintf("%s/images/mo_admin_info.png",
                            GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString().toLatin1().constData());
    CGexSystemUtils::NormalizePath(strPixMapSource);
    m_pxInfo = QPixmap(strPixMapSource);
    strPixMapSource.sprintf("%s/images/mo_admin_error.png",
                            GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString().toLatin1().constData());
    CGexSystemUtils::NormalizePath(strPixMapSource);
    m_pxError = QPixmap(strPixMapSource);
    strPixMapSource.sprintf("%s/images/mo_admin_warning.png",
                            GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString().toLatin1().constData());
    CGexSystemUtils::NormalizePath(strPixMapSource);
    m_pxWarning = QPixmap(strPixMapSource);
    strPixMapSource.sprintf("%s/images/mo_admin_success.png",
                            GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString().toLatin1().constData());
    CGexSystemUtils::NormalizePath(strPixMapSource);
    m_pxSuccess = QPixmap(strPixMapSource);


    // Get list of MO history log files
    QString strLogFolder = GS::Gex::Engine::GetInstance().GetSchedulerEngine()
            .Get(GS::Gex::SchedulerEngine::sMonitoringLogsFolderKey).toString();

    dir.setPath(strLogFolder);
    dir.setFilter(QDir::Files | QDir::Hidden);
    // Use 'reversed' order so that files are inserted in right order in the combo box
    dir.setSorting(QDir::Time|QDir::Reversed);
    strFilter = GEXMO_LOG_FILE_ROOT;
    strFilter += "*.log";
    m_strlLogFiles_History = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Insert all files in combo
    comboFiles_History->insertItems(0, m_strlLogFiles_History);

    // Select the most recent file
    // Load content of selected history file
    if(comboFiles_History->count() > 0)
    {
        nIndex = comboFiles_History->count();
        nIndex -= 1;
        comboFiles_History->setCurrentIndex(nIndex);
        LoadFile_History(comboFiles_History->currentText());
    }

    // Get list of GexFtp history log files
    dir.setPath(GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    dir.setFilter(QDir::Files | QDir::Hidden);
    // Use 'reversed' order so that files are inserted in right order in the combo box
    dir.setSorting(QDir::Time|QDir::Reversed);
    strFilter = "gex_ftp_*.log";
    m_strlLogFiles_GexFtp = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Insert all files in combo
    comboFiles_GexFtp->insertItems(0, m_strlLogFiles_GexFtp);

    // Select the most recent GexFtp file
    // Load content of selected GexFtp file
    if(comboFiles_GexFtp->count() > 0)
    {
        nIndex = comboFiles_GexFtp->count();
        nIndex -= 1;
        comboFiles_GexFtp->setCurrentIndex(nIndex);
        LoadFile_GexFtp(comboFiles_GexFtp->currentText());
    }

    // Get list of GexEmail history log files
    dir.setPath(GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    dir.setFilter(QDir::Files | QDir::Hidden);
    // Use 'reversed' order so that files are inserted in right order in the combo box
    dir.setSorting(QDir::Time|QDir::Reversed);
    strFilter = "gex_email_*.log";
    m_strlLogFiles_GexEmail = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Insert all files in combo
    comboFiles_GexEmail->insertItems(0, m_strlLogFiles_GexEmail);

    // Select the most recent GexEmail file
    // Load content of selected GexEmail file
    if(comboFiles_GexEmail->count() > 0)
    {
        nIndex = comboFiles_GexEmail->count();
        nIndex -= 1;
        comboFiles_GexEmail->setCurrentIndex(nIndex);
        LoadFile_GexEmail(comboFiles_GexEmail->currentText());
    }

    // Get list of MO report log files
    dir.setPath(strLogFolder);
    dir.setFilter(QDir::Files | QDir::Hidden);
    // Use 'reversed' order so that files are inserted in right order in the combo box
    dir.setSorting(QDir::Time|QDir::Reversed);
    strFilter = GEXMO_REPORT_FILE_ROOT;
    strFilter += "*.log";
    m_strlLogFiles_Report = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Insert all files in combo
    comboFiles_Report->insertItems(0, m_strlLogFiles_Report);

    // Select the most recent report file
    // Load content of selected history file
    if(comboFiles_Report->count() > 0)
    {
        nIndex = comboFiles_Report->count();
        nIndex -= 1;
        comboFiles_Report->setCurrentIndex(nIndex);
        LoadFile_Report(comboFiles_Report->currentText());
    }

    stackedWidget->setCurrentIndex(0);

    // PAT-38
    if (pGexScriptEngine)
    {
        QScriptValue lMonitoringWidgetSV = pGexScriptEngine->newQObject(this);
        if (!lMonitoringWidgetSV.isNull())
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Registering Monitoring GUI into Script Engine");
            pGexScriptEngine->globalObject().setProperty("GSMonitoringGUI", lMonitoringWidgetSV);
        }
    }
}

void MonitoringWidget::UpdateListOfFiles()
{
    QString					strFilter;
    QDir					dir;
    QStringList				strlFiles;
    QStringList::iterator	it;

    // Get list of MO history log files
    QString strLogFolder = GS::Gex::Engine::GetInstance().GetSchedulerEngine()
                .Get(GS::Gex::SchedulerEngine::sMonitoringLogsFolderKey).toString();

    GSLOG(SYSLOG_SEV_NOTICE, QString("Udating list of files in: %1").arg(strLogFolder).toLatin1().data() );

    dir.setPath(strLogFolder);
    dir.setFilter(QDir::Files | QDir::Hidden);
    dir.setSorting(QDir::Time);
    strFilter = GEXMO_LOG_FILE_ROOT;
    strFilter += "*.log";
    strlFiles = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Add new files to combo
    for(it = strlFiles.begin(); it != strlFiles.end(); it++)
    {
        if(m_strlLogFiles_History.indexOf(*it) == -1)
        {
            comboFiles_History->addItem(*it);
            m_strlLogFiles_History.append(*it);
        }
    }

    // Get list of GexFtp history log files
    dir.setPath(GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    dir.setFilter(QDir::Files | QDir::Hidden);
    dir.setSorting(QDir::Time);
    strFilter = "gex_ftp_*.log";
    strlFiles = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Add new files to combo
    for(it = strlFiles.begin(); it != strlFiles.end(); it++)
    {
        if(m_strlLogFiles_GexFtp.indexOf(*it) == -1)
        {
            comboFiles_GexFtp->addItem(*it);
            m_strlLogFiles_GexFtp.append(*it);
        }
    }

    // Get list of GexEmail history log files
    dir.setPath(GS::Gex::Engine::GetInstance().Get("UserFolder").toString());
    dir.setFilter(QDir::Files | QDir::Hidden);
    dir.setSorting(QDir::Time);
    strFilter = "gex_email_*.log";
    strlFiles = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Add new files to combo
    for(it = strlFiles.begin(); it != strlFiles.end(); it++)
    {
        if(m_strlLogFiles_GexEmail.indexOf(*it) == -1)
        {
            comboFiles_GexEmail->addItem(*it);
            m_strlLogFiles_GexEmail.append(*it);
        }
    }

    // Get list of MO report log files
    dir.setPath(strLogFolder);
    dir.setFilter(QDir::Files | QDir::Hidden);
    dir.setSorting(QDir::Time);
    strFilter = GEXMO_REPORT_FILE_ROOT;
    strFilter += "*.log";
    strlFiles = dir.entryList(QDir::nameFiltersFromString(strFilter));

    // Add new files to combo
    for(it = strlFiles.begin(); it != strlFiles.end(); it++)
    {
        if(m_strlLogFiles_Report.indexOf(*it) == -1)
        {
            comboFiles_Report->addItem(*it);
            m_strlLogFiles_Report.append(*it);
        }
    }
}

void MonitoringWidget::LoadFile_History(const QString & strMoHistoryLogFile)
{
    QString strLogFile = GS::Gex::Engine::GetInstance().GetSchedulerEngine()
                            .Get(GS::Gex::SchedulerEngine::sMonitoringLogsFolderKey).toString();
    strLogFile += strMoHistoryLogFile;

    // Reset content.
    textEditHistory->clear();

    QFile file(strLogFile); // Read the text from a file
    if(file.open(QIODevice::ReadOnly) == false)
        return;	// Failed opening Log file.

    // Load file into widget.
    QTextStream stream(&file);
    textEditHistory->setText(stream.readAll());

    // Scroll down to the end.
    textEditHistory->moveCursor(QTextCursor::End);

    // Close file
    file.close();
}

void MonitoringWidget::OnMoHistoryLogUpdated(const QString & strMoHistoryLogFile, const QString & strMessage)
{
    QFileInfo	clFileInfo(strMoHistoryLogFile);
    QString		strLogFile = clFileInfo.fileName();

    // If file not present in the combo box, add it
    if(m_strlLogFiles_History.indexOf(strLogFile) == -1)
    {
        m_strlLogFiles_History.append(strLogFile);
        comboFiles_History->addItem(strLogFile);
    }

    // If file selected, update the widget
    if(comboFiles_History->currentText() == strLogFile)
        textEditHistory->append(strMessage);
}

void MonitoringWidget::OnFileSelected_History(void)
{
    LoadFile_History(comboFiles_History->currentText());
}

void MonitoringWidget::OnButtonSearch_Next_History(void)
{
    QString strSearch = editSearch_History->text();

    if(strSearch.isEmpty())
        return;

    textEditHistory->find(strSearch, 0);
}

void MonitoringWidget::OnButtonSearch_Prev_History(void)
{
    QString strSearch = editSearch_History->text();

    if(strSearch.isEmpty())
        return;

    textEditHistory->find(strSearch, QTextDocument::FindBackward);
}

void MonitoringWidget::LoadFile_GexFtp(const QString & strGexFtpLogFile)
{
    QString	strLogFile = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/";
    strLogFile += strGexFtpLogFile;

    // Reset content.
    mUi_qtwGexFtp->clear();

    QFile file(strLogFile); // Read the text from a file
    if(file.open(QIODevice::ReadOnly) == false)
        return;	// Failed opening Log file.

    // Load file into widget.
    QTextStream		stream(&file);
    QString			strLine;
    QStringList		strlLine;
    int				iLastCheckedLine = 0;
    QTreeWidgetItem	*pQtwiItem = NULL;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    strLine = stream.readLine();
    while(!strLine.isNull())
    {
        iLastCheckedLine++;
        if(!strLine.isEmpty() && (strLine != "Date,Type,Ftp server,Message"))
        {
            // Add to list view
            strlLine = strLine.split(',', QString::KeepEmptyParts);
            if (strlLine.count() < 4)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Error while reading GexFTP log file, check: %1 at line %2")
                       .arg(strLogFile)
                       .arg(QString::number(iLastCheckedLine)).toLatin1().constData());
                // Go to next line
                strLine = stream.readLine();
                continue;
            }

            pQtwiItem = new QTreeWidgetItem(mUi_qtwGexFtp);
            pQtwiItem->setText(0, strlLine[0]);
            pQtwiItem->setText(1, strlLine[2]);
            pQtwiItem->setText(2, strlLine[3]);
            if(strlLine[1] == "Info")
                pQtwiItem->setIcon(0, m_pxInfo);
            if(strlLine[1] == "InfoTransfer")
                pQtwiItem->setIcon(0, m_pxSuccess);
            if(strlLine[1] == "Warning")
                pQtwiItem->setIcon(0, m_pxWarning);
            if(strlLine[1] == "Error")
                pQtwiItem->setIcon(0, m_pxError);
        }
        strLine = stream.readLine();
    }

    if(pQtwiItem)
    {
        mUi_qtwGexFtp->scrollToItem(pQtwiItem, QAbstractItemView::EnsureVisible);
    }

    // resize columns
    for(int ii=0; ii<mUi_qtwGexFtp->columnCount(); ii++)
    {
        mUi_qtwGexFtp->resizeColumnToContents(ii);
    }

    // Close file
    file.close();
    QGuiApplication::restoreOverrideCursor();
}

void MonitoringWidget::OnFileSelected_GexFtp(void)
{
    LoadFile_GexFtp(comboFiles_GexFtp->currentText());
}

void MonitoringWidget::OnButtonRefresh_GexFtp(void)
{
    LoadFile_GexFtp(comboFiles_GexFtp->currentText());
}

void MonitoringWidget::OnButtonSearch_Prev_GexFtp(void)
{
    QString strSearch = editSearch_GexFtp->text();

    if(strSearch.isEmpty())
        return;
}

void MonitoringWidget::OnButtonSearch_Next_GexFtp(void)
{
    QString strSearch = editSearch_GexFtp->text();

    if(strSearch.isEmpty())
        return;
}

void MonitoringWidget::LoadFile_GexEmail(const QString & strGexEmailLogFile)
{
    QString	strLogFile = GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + "/";
    strLogFile += strGexEmailLogFile;

    // Reset content.
    listView_GexEmail->clear();
    listView_GexEmail->setRootIsDecorated(false);

    QFile file(strLogFile); // Read the text from a file
    if(file.open(QIODevice::ReadOnly) == false)
        return;	// Failed opening Log file.

    // Load file into widget.
    QTextStream		stream(&file);
    QString			strLine;
    QStringList		strlLine;
    QTreeWidgetItem	*pItem = NULL;
    int				iLastCheckedLine = 0;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    strLine = stream.readLine();
    while(!strLine.isNull())
    {
        iLastCheckedLine++;
        if(!strLine.isEmpty() && (strLine != "Date,Type,Mail file,Message"))
        {
            // Add to list view
            strlLine = strLine.split(',', QString::KeepEmptyParts);
            if (strlLine.count() < 4)
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Error while reading GexEmail log file, check: %1 at line %2")
                       .arg(strLogFile)
                       .arg(QString::number(iLastCheckedLine)).toLatin1().constData());
                // Go to next line
                strLine = stream.readLine();
                continue;
            }

            pItem = new QTreeWidgetItem(listView_GexEmail);
            pItem->setText(0, strlLine[0]);
            pItem->setText(1, strlLine[2]);
            pItem->setText(2, strlLine[3]);
            if(strlLine[1] == "Info")
                pItem->setIcon(0, m_pxInfo);
            if(strlLine[1] == "InfoSent")
                pItem->setIcon(0, m_pxSuccess);
            if(strlLine[1] == "Warning")
                pItem->setIcon(0, m_pxWarning);
            if(strlLine[1] == "Error")
                pItem->setIcon(0, m_pxError);
        }
        strLine = stream.readLine();
    }

    if(pItem)
        listView_GexEmail->setVisible(true);

    // Close file
    file.close();
    QGuiApplication::restoreOverrideCursor();
}

void MonitoringWidget::OnFileSelected_GexEmail(void)
{
    LoadFile_GexEmail(comboFiles_GexEmail->currentText());
}

void MonitoringWidget::OnButtonRefresh_GexEmail(void)
{
    LoadFile_GexEmail(comboFiles_GexEmail->currentText());
}

void MonitoringWidget::OnButtonSearch_Prev_GexEmail(void)
{
    QString strSearch = editSearch_GexEmail->text();

    if(strSearch.isEmpty())
        return;
}

void MonitoringWidget::OnButtonSearch_Next_GexEmail(void)
{
    QString strSearch = editSearch_GexEmail->text();

    if(strSearch.isEmpty())
        return;
}

QTreeWidgetItem * MonitoringWidget::LoadLine_Report(const QString & strLine, bool bEnsureItemVisible)
{
    QString         lLine = QString(strLine).replace("\n"," ").simplified();
    QStringList     lColumnsLine;
    QTreeWidgetItem *pItem = NULL;

    if(!lLine.isEmpty() && !lLine.startsWith("Date,Time,Status,FileName,Cause,DataPump,Directory"))
    {
        // Add to list view
        lColumnsLine = lLine.split(',', QString::KeepEmptyParts);
        // if (strlLine.count()==0) PYC, case 5279
        if (lColumnsLine.count()<7)
        {
            GEX_ASSERT(false);
            GSLOG(SYSLOG_SEV_WARNING, "load report line problem : check report format");
            return NULL;
        }

        listView_Report->setRootIsDecorated(false);
        pItem = new QTreeWidgetItem(listView_Report);
        pItem->setText(0, lColumnsLine[0]);
        pItem->setText(1, lColumnsLine[1]);
        pItem->setText(2, lColumnsLine[2]);
        pItem->setText(3, lColumnsLine[3]);
        pItem->setText(4, lColumnsLine[4]);
        pItem->setText(5, lColumnsLine[5]);
        pItem->setText(6, lColumnsLine[6]);
        if(lColumnsLine.count() > 7)
        {
            pItem->setText(7, lColumnsLine[7]);
            pItem->setText(8, lColumnsLine[8]);
            pItem->setText(9, lColumnsLine[9]);
        }
        else
        {
            pItem->setText(7, "n/a");
            pItem->setText(8, "n/a");
            pItem->setText(9, "n/a");
        }

        // Set adequate pixmap
        if(lColumnsLine[2] == "Inserted")
            pItem->setIcon(0, m_pxSuccess);
        if(lColumnsLine[2] == "Rejected")
            pItem->setIcon(0, m_pxError);
        if(lColumnsLine[2] == "Delayed")
            pItem->setIcon(0, m_pxWarning);
    }

    // Ensure item is visible?
    if(pItem && bEnsureItemVisible)
    {

        listView_Report->setVisible(true);
    }

    return pItem;
}

void MonitoringWidget::LoadFile_Report(const QString & strReportLogFile)
{
    QString strLogFile = GS::Gex::Engine::GetInstance().GetSchedulerEngine()
                                .Get(GS::Gex::SchedulerEngine::sMonitoringLogsFolderKey).toString();
    strLogFile += strReportLogFile;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Opening %1").arg( strReportLogFile).toLatin1().constData());

    // Reset content.
    listView_Report->clear();

    QFile file(strLogFile); // Read the text from a file
    if(file.open(QIODevice::ReadOnly) == false)
        return;	// Failed opening Log file.

    // Load file into widget.
    QTextStream		stream(&file);
    QString			strLine;
    QTreeWidgetItem	*pItem = NULL;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    strLine = stream.readLine();
    while(!strLine.isNull())
    {
        pItem = LoadLine_Report(strLine, false);
        strLine = stream.readLine();
    }

    if(pItem)
        listView_Report->setVisible(true);

    // Close file
    file.close();
    QGuiApplication::restoreOverrideCursor();
}

void MonitoringWidget::OnMoReportLogUpdated(const QString & strMoReportLogFile, const QString & strMessage)
{
    QFileInfo	clFileInfo(strMoReportLogFile);
    QString		strLogFile = clFileInfo.fileName();

    // If file not present in the combo box, add it
    if(m_strlLogFiles_Report.indexOf(strLogFile) == -1)
    {
        m_strlLogFiles_Report.append(strLogFile);
        comboFiles_Report->addItem(strLogFile);
    }

    // If file selected, update the widget
    if(comboFiles_Report->currentText() == strLogFile)
        LoadLine_Report(strMessage, true);
}

void MonitoringWidget::OnFileSelected_Report(void)
{
    LoadFile_Report(comboFiles_Report->currentText());
}

void MonitoringWidget::OnButtonSearch_Prev_Report(void)
{
    QString strSearch = editSearch_Report->text();

    if(strSearch.isEmpty())
        return;
}

void MonitoringWidget::OnButtonSearch_Next_Report(void)
{
    QString strSearch = editSearch_Report->text();

    if(strSearch.isEmpty())
        return;
}

void MonitoringWidget::OnMainSelectionChanged(void)
{
    // Update list of log files in combo boxes
    UpdateListOfFiles();

    switch(mUi_qlwSelectionList->indexOfTopLevelItem(mUi_qlwSelectionList->currentItem()))
    {
    case 0:
        stackedWidget->setCurrentWidget(pageHistory);
        break;

    case 1:
        stackedWidget->setCurrentWidget(pageGexFtpLog);
        break;

    case 2:
        stackedWidget->setCurrentWidget(pageReport);
        break;

    case 3:
        stackedWidget->setCurrentWidget(pageGexEmailLog);
        break;
    case 4:
    case -1:
    default:
        if(isYMEvenLogsEnabled() && m_poYMEventLogViewManager){
            QTreeWidgetItem *poItem = mUi_qlwSelectionList->currentItem();
            if(!poItem)
                break;
            if(m_poYMEventLogViewManager->getYMEventLogTopNode() == poItem){
                poItem->setExpanded(true);
                if(m_poYMEventLogViewManager->getYMEventLogTopNode()->childCount()){
                    if(m_poYMEventLogViewManager->getYMEventLogTopNode()->childCount() > 1){
                        mUi_qlwSelectionList->setCurrentItem(m_poYMEventLogViewManager->getYMEventLogTopNode()->child(1));
                    }else
                        mUi_qlwSelectionList->setCurrentItem(m_poYMEventLogViewManager->getYMEventLogTopNode()->child(0));
                }
            } else if(poItem->parent() && m_poYMEventLogViewManager->getYMEventLogTopNode() == poItem->parent()){
                stackedWidget->setCurrentWidget(m_poEvenLogPage);
                m_poYMEventLogViewManager->updateLogViewer(poItem);
            }
        }
        break;
    }
}


bool MonitoringWidget::isYMEvenLogsEnabled(){

    return (GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() &&
             GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectToAdminServer());

}

void MonitoringWidget::initLogManager(QTreeWidgetItem *poItem, QWidget *poContainer){
    if(!poItem || !poContainer)
        return ;

    bool bIsOk = false;
    if(isYMEvenLogsEnabled()){
        m_poYMEventLogViewManager = new GexYMEventLogViewManager(poItem, poContainer);
        if(m_poYMEventLogViewManager->getErrorCode() != GexYMEventLogViewManager::noError){
            m_poYMEventLogViewManager->handleError(this);
            delete m_poYMEventLogViewManager;
            m_poYMEventLogViewManager = 0;
        }else{
            bIsOk = true;
            QObject::connect(m_poReloadViewManager, SIGNAL(clicked()), m_poYMEventLogViewManager, SLOT(reloadViewManager()));
            QObject::connect(m_poSaveViewManager, SIGNAL(clicked()), m_poYMEventLogViewManager, SLOT(saveViewManager()));

        }
    }

    if(!bIsOk){
        if(poItem)
            delete poItem;
    }
}

MonitoringWidget::~MonitoringWidget(){
    if(m_poYMEventLogViewManager)
        delete m_poYMEventLogViewManager;
}

