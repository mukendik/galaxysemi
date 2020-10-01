#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "admin_gui.h"
#include "db_engine.h"
#include "db_external_database.h"
#include "engine.h"
#include "pickproduct_idsql_dialog.h"
#include "gexmo_constants.h"
#include "gex_shared.h"
#include "gex_database_entry.h"
#include "message.h"
#include "outlierremoval/outlierremoval_task.h"
#include "browser_dialog.h"
#include "mo_scheduler_gui.h"
#include "mo_task_create_gui.h"
#include "pickproduct_id_dialog.h"

extern GexMainwindow* pGexMainWindow;

GexMoCreateTaskOutlierRemoval::GexMoCreateTaskOutlierRemoval(QWidget* /*parent*/,
                              bool modal,
                              Qt::WindowFlags /*fl*/): cOutlierRemovalData(this)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    // Get database pointer
    m_pDatabaseEntry = NULL;
    m_pDatabaseEntry = (GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.isEmpty()) ?
                NULL :
                (GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.first());
    if(m_pDatabaseEntry != NULL)
        cOutlierRemovalData.strProductID = "";
    lineEditProductID->setText("");

    QObject::connect(buttonBox_OkCancel,            SIGNAL(accepted()),	this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,            SIGNAL(rejected()),	this, SLOT(OnCancel()));

    QObject::connect(PickProductID,					SIGNAL(clicked()),				this, SLOT(OnPickProductID()));
    QObject::connect(pushButtonMailingList,			SIGNAL(clicked()),				this, SLOT(OnMailingList()));
    QObject::connect(lineEditProductID,				SIGNAL(textChanged(QString)),	this, SLOT(OnProductsChange(QString)));
    QObject::connect(comboBoxEmailReportContents,	SIGNAL(activated(int)),			this, SLOT(OnMessageContents()));
    QObject::connect(comboBoxDatabase,				SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnDatabaseChanged()));
    QObject::connect(comboBoxGeneration,			SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onGenerationModeChange()));

    // Load list of databases
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(comboBoxDatabase, DB_TYPE_SQL|DB_STATUS_UPLOADED|DB_SUPPORT_INSERTION, DB_TDR_YM_PROD);

    // Check if empty database exists
    comboBoxDatabase->addItem("");
    comboBoxDatabase->setCurrentIndex(comboBoxDatabase->count()-1);

    // Ensure GUI only displays rules associated with selected database.
    OnDatabaseChanged();

    // Default Title
    LineEditTitle->setText("PAT Execution");

    // Bins list to monitor
    lineEditYieldLevel->setText("0.5");

    // Minimum parts required
    lineEditYieldMinimumParts->setText("50");

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lineEditEmailFrom->setText(GEX_EMAIL_PAT_MAN);
    else
        lineEditEmailFrom->setText(GEX_EMAIL_YIELD_MAN);

    onGenerationModeChange();


}

void GexMoCreateTaskOutlierRemoval::OnMailingList(void)
{
    // Info message about Mailing list file format.
    GS::Gex::Message::information(
        "", "Mailing list file format:\n\n"
        " o It is an ASCII file\n"
        " o It can hold multiple emails per line\n"
        " o email format is <address>@<domain>\n"
        " o email addresses separator is ';'\n\n");

    QString strMailingList;
    QFileDialog cFileDialog(this);
    strMailingList = cFileDialog.getOpenFileName(this, "Select mailing list", "", "Mailing list *.txt;;All Files (*.*)");
    if(strMailingList.isEmpty() == true)
        return;	// No mailing list file selected...return!

    // Save folder selected.
    lineEditEmailList->setText(strMailingList);
}

void GexMoCreateTaskOutlierRemoval::OnCancel(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "On cancel...");
    done(-1);
}

void GexMoCreateTaskOutlierRemoval::OnOk(void)
{
    // Get Task title.
    cOutlierRemovalData.strTitle = LineEditTitle->text();
    if(cOutlierRemovalData.strTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    // Get targetted database name,Skip the [Local]/[Server] info
    QString strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section(" ",1);
    cOutlierRemovalData.strDatabase = strDatabaseLogicalName.simplified();

    // Get testing stage
    if(comboBoxTestingStage->isVisible())
        cOutlierRemovalData.strTestingStage = comboBoxTestingStage->currentText();
    else
        cOutlierRemovalData.strTestingStage = "";

    // Get Data source folder path
    cOutlierRemovalData.strProductID= lineEditProductID->text();
    if(cOutlierRemovalData.strProductID.isEmpty())
    {
        // Need to enter a folder path!
        GS::Gex::Message::warning("", "You must specify the ProductID!");
        lineEditProductID->setFocus();
        return;
    }

    cOutlierRemovalData.lfAlarmLevel = (lineEditYieldLevel->text()).toDouble();
    cOutlierRemovalData.iAlarmType = comboBoxYieldLevel->currentIndex();
    QString strString = lineEditYieldMinimumParts->text();
    cOutlierRemovalData.lMinimumyieldParts = strString.toLong();
    cOutlierRemovalData.strEmailFrom = lineEditEmailFrom->text();
    cOutlierRemovalData.strEmailNotify = lineEditEmailList->text();
    cOutlierRemovalData.bHtmlEmail = (comboBoxMailFormat->currentIndex() == 0) ? true : false;
    cOutlierRemovalData.iEmailReportType = (comboBoxEmailReportContents->count()==5) ? comboBoxEmailReportContents->currentIndex() : comboBoxEmailReportContents->currentIndex() + 1;
    cOutlierRemovalData.iNotificationType = comboBoxGeneration->currentIndex();
    cOutlierRemovalData.bNotifyShapeChange = checkBoxNotifyShapeChange->isChecked();
    cOutlierRemovalData.iExceptionLevel= comboBoxExceptionLevel->currentIndex();

    // Composite Wafer yield thresholds
    strString = lineEditCompositeEtestAlarm->text();
    cOutlierRemovalData.lCompositeEtestAlarm = strString.toLong();
    strString = lineEditCompositeExclusionZoneAlarm->text();
    cOutlierRemovalData.lCompositeExclusionZoneAlarm = strString.toLong();

    if(cOutlierRemovalData.strEmailNotify.isEmpty() == true)
    {
        // Need to specify an email address
        if(QMessageBox::question( this,
                                  GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                  "No email specified...\nAlarms won't send email notifications!\n\nDo you confirm this choice?",QMessageBox::Yes,QMessageBox::No | QMessageBox::Default) != QMessageBox::Yes)
        {
            lineEditEmailList->setFocus();
            return;
        }
    }

    if (cOutlierRemovalData.strEmailFrom.isEmpty() == true)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            cOutlierRemovalData.strEmailFrom = GEX_EMAIL_PAT_MAN;
        else
            cOutlierRemovalData.strEmailFrom = GEX_EMAIL_YIELD_MAN;
    }

    done(1);
}

void GexMoCreateTaskOutlierRemoval::LoadFields(CGexMoTaskOutlierRemoval *ptTaskItem, QStringList *plistTestingStages/*=NULL*/)
{
    // Task Title
    LineEditTitle->setText(ptTaskItem->GetProperties()->strTitle);

    // Fill Testing stage combobox
    comboBoxTestingStage->clear();
    if(plistTestingStages)
        comboBoxTestingStage->addItems(*plistTestingStages);

    // Targeted database
    // Check if it is a new task (empty productId)
    if(!ptTaskItem->GetProperties()->strProductID.isEmpty())
    {
        int iIndex;
        QString strDatabaseName;
        QString strDatabaseLogicalName;
        strDatabaseName = ptTaskItem->GetProperties()->strDatabase;
        for(iIndex=0; iIndex<comboBoxDatabase->count(); iIndex++)
        {
            strDatabaseLogicalName = comboBoxDatabase->itemText(iIndex);
            if(strDatabaseLogicalName == strDatabaseName)
                break;
            if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
                strDatabaseLogicalName = strDatabaseLogicalName.section(" ",1);

            if(strDatabaseLogicalName == strDatabaseName)
                break;
        }
        if(iIndex < comboBoxDatabase->count())
        {
            // Database was found in the list
            comboBoxDatabase->setCurrentIndex(iIndex);
        }
        else
        {
            // Add a dummy entry
            comboBoxDatabase->addItem(strDatabaseName);
            comboBoxDatabase->setCurrentIndex(comboBoxDatabase->count()-1);
        }
    }

    OnDatabaseChanged();

    // Testing stage
    if(!ptTaskItem->GetProperties()->strTestingStage.isEmpty())
    {
        int lIndex = comboBoxTestingStage->findText(ptTaskItem->GetProperties()->strTestingStage);
        if(lIndex != -1)
            comboBoxTestingStage->setCurrentIndex(lIndex);
    }

    // ProductID
    lineEditProductID->setText(ptTaskItem->GetProperties()->strProductID);

    // Yield alarm threshold value
    lineEditYieldLevel->setText(QString::number(ptTaskItem->GetProperties()->lfAlarmLevel));

    // Yield Alarm Type (% of yield loss, or parts yield loss )
    comboBoxYieldLevel->setCurrentIndex(ptTaskItem->GetProperties()->iAlarmType);

    // Minimum parts to triger alarm
    lineEditYieldMinimumParts->setText(QString::number(ptTaskItem->GetProperties()->lMinimumyieldParts));

    // Composite Wafer: Maximum discrepancies allowed for each wafer in the lot
    lineEditCompositeEtestAlarm->setText(QString::number(ptTaskItem->GetProperties()->lCompositeEtestAlarm));

    // Composite Wafer: Maximum number of dies to force to reject on each wafer in lot (exclusion zone)
    lineEditCompositeExclusionZoneAlarm->setText(QString::number(ptTaskItem->GetProperties()->lCompositeExclusionZoneAlarm));

    // Email 'From'
    lineEditEmailFrom->setText(ptTaskItem->GetProperties()->strEmailFrom);

    // Emails to notify
    lineEditEmailList->setText(ptTaskItem->GetProperties()->strEmailNotify);

    // Email format: HTML or ASCII
    if(ptTaskItem->GetProperties()->bHtmlEmail)
        comboBoxMailFormat->setCurrentIndex(0);	// HTML (default)
    else
        comboBoxMailFormat->setCurrentIndex(1);	// TEXT

    // Email report contents
    comboBoxEmailReportContents->setCurrentIndex(comboBoxEmailReportContents->count() == 5 ?
                                                    ptTaskItem->GetProperties()->iEmailReportType :
                                                    ptTaskItem->GetProperties()->iEmailReportType-1 );

    // Notification if distribution shape is changing (eg: Gaussian to LogNormal)
    checkBoxNotifyShapeChange->setChecked(ptTaskItem->GetProperties()->bNotifyShapeChange);

    // Report notification type: Attach to email, or store on server?
    comboBoxGeneration->setCurrentIndex(ptTaskItem->GetProperties()->iNotificationType);

    // Exception level: Standard, Critical, etc...
    comboBoxExceptionLevel->setCurrentIndex(ptTaskItem->GetProperties()->iExceptionLevel);

    // show/Hide relevant fields
    OnMessageContents();
}

void GexMoCreateTaskOutlierRemoval::OnPickProductID(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, "...");
    if(m_pDatabaseEntry && m_pDatabaseEntry->m_pExternalDatabase)
    {
        // Get product List
        QStringList	strlProductList;

        // Fill external database query object
        GexDbPlugin_Filter clPluginFilter(this);
        clPluginFilter.strlQueryFilters.clear();
        clPluginFilter.strDataTypeQuery = comboBoxTestingStage->currentText();
        clPluginFilter.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;

        QString strProductFilter = lineEditProductID->text();

        // Execute query on remote database
        bool qpl=m_pDatabaseEntry->m_pExternalDatabase->QueryProductList(clPluginFilter,strlProductList,strProductFilter);
        if (!qpl)
        {
            GSLOG(SYSLOG_SEV_ERROR, " QueryProductList failed ! ");
            QString d="Impossible to query available products !";
            bool			bDbUpToDate=true;
            QString			strCurrentVersionName, strLastSupportedVersion;
            unsigned int	uiCurrentVersion, uiLastSupportedVersion;
            if (!m_pDatabaseEntry->m_pExternalDatabase->IsDbUpToDateForExtraction(&bDbUpToDate, strCurrentVersionName,
                                                                                  &uiCurrentVersion, strLastSupportedVersion, &uiLastSupportedVersion))
                d="Error : impossible to retrieve database version !";
            if (!bDbUpToDate)
                d="Your database is NOT uptodate. Please update your database to a newer version.";
            GS::Gex::Message::critical("GEX error", d);
            return;
        }
        if(strlProductList.count() == 0)
        {
            GS::Gex::Message::warning("GEX warning", "No product available!");
            return;
        }

        // Fill Filter list with relevant strings
        PickProductIdSQLDialog dPickFilter;
        dPickFilter.fillList(strlProductList, true, false);

        // Prompt dialog box, let user pick Filter string from the list
        strlProductList.clear();
        if(dPickFilter.exec() != 1)
            return;	// User 'Abort'

        // Set selected list
        dPickFilter.getProductList(strlProductList);

        // If a product has been selected, display it
        if(strlProductList.count() > 0)
        {
            // Set ProductID
            lineEditProductID->setText(strlProductList.join(","));
            lineEditProductID->setFocus();
        }
    }
    else
    {
        PickProductIdDialog cPickProductID(this,m_pDatabaseEntry);
        if(cPickProductID.exec() != 1)
            return;
        QString strProductID = cPickProductID.getSelection();
        if(strProductID.isEmpty())
            return;
        // Set ProductID
        lineEditProductID->setText(strProductID);
        lineEditProductID->setFocus();
    }
}

void GexMoCreateTaskOutlierRemoval::OnProductsChange(const QString &strString)
{
    // Check if user combining ? or * with the '|' character: it's not allowed!
    if(((strString.indexOf("*") >= 0) || (strString.indexOf("?") >= 0)) &&
            (((strString.indexOf("|") >= 0)) || (strString.indexOf(",") >= 0)))
    {
        GS::Gex::Message::information(
            "", "You can't combine the '*' or '?' wildcar\n"
            "with the OR '|' ',' characters. Use either one grammar.");
    }

    // Set focus on edit field
    lineEditProductID->setFocus();
}

void GexMoCreateTaskOutlierRemoval::OnMessageContents(void)
{
    // Under Unix: refuse Word generation
#if defined unix || __MACH__
    QVariant    lComboData  = comboBoxEmailReportContents->itemData(comboBoxEmailReportContents->currentIndex());
    int         lDefaultIndex = comboBoxEmailReportContents->findData(GEX_SETTINGS_OUTPUT_PDF);	// Default outputformat

    switch(lComboData.toInt())
    {
    case GEX_SETTINGS_OUTPUT_HTML:	// HTML
    case GEX_SETTINGS_OUTPUT_CSV:	// CSV
    case GEX_SETTINGS_OUTPUT_PDF:	// PDF
        break;

    case GEX_SETTINGS_OUTPUT_WORD:	// WORD
        // Not available under unix...
        GS::Gex::Message::information(
            "", "The Word document generation is not available under "
            "Unix & Linux\nYou need the Windows version.\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information.");
        comboBoxEmailReportContents->setCurrentIndex(lDefaultIndex);	// Reset output format to: ASCII

        // show/Hide relevant fields
        OnMessageContents();
        break;

    case GEX_SETTINGS_OUTPUT_PPT:	// PowerPoint
        // Not available under unix...
        GS::Gex::Message::information(
            "", "The PowerPoint document generation is not available under "
            "Unix & Linux\nYou need the Windows version.\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information.");
        comboBoxEmailReportContents->setCurrentIndex(lDefaultIndex);	// Reset output format to: ASCII

        // show/Hide relevant fields
        OnMessageContents();
        break;
    }
#endif
}

void GexMoCreateTaskOutlierRemoval::OnDatabaseChanged(void)
{
    comboBoxTestingStage->hide();

    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry=NULL;
    QString				strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section(' ',1);

    if(strDatabaseLogicalName == "")
        return;

    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName,false);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    lineEditProductID->setText("");

    QStringList listTestingStages;
    // For selecting data type to query (wafer sort, or final test,...)
    if(pDatabaseEntry->m_pExternalDatabase
            && pDatabaseEntry->m_pExternalDatabase->IsTestingStagesSupported())
        pDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(listTestingStages);

    // Fill Testing stage combobox
    comboBoxTestingStage->clear();
    comboBoxTestingStage->addItems(listTestingStages);

    if(listTestingStages.isEmpty())
        comboBoxTestingStage->hide();
    else
        comboBoxTestingStage->show();

    m_pDatabaseEntry = pDatabaseEntry;
}

void GexMoCreateTaskOutlierRemoval::onGenerationModeChange()
{
    // 0= Report attached to email, 1= keep on server, only email its URL
    int lItemIndex = -1;
    int lGenType = comboBoxGeneration->currentIndex();
    QVariant lComboEmailContentData = comboBoxEmailReportContents->itemData(comboBoxEmailReportContents->currentIndex());

    if(lGenType == 0)
    {
        comboBoxEmailReportContents->clear();
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/gex/icons/word.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/gex/icons/csv_spreadsheet.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/gex/icons/powerpoint.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/gex/icons/pdf.png"), QSize(), QIcon::Normal, QIcon::On);
        comboBoxEmailReportContents->clear();

        comboBoxEmailReportContents->addItem(icon2, "Full WORD report", GEX_SETTINGS_OUTPUT_WORD);
        comboBoxEmailReportContents->addItem(icon3, "Full CSV report", GEX_SETTINGS_OUTPUT_CSV);
        comboBoxEmailReportContents->addItem(icon4, "Full PowerPoint report", GEX_SETTINGS_OUTPUT_PPT);
        comboBoxEmailReportContents->addItem(icon5, "Full PDF report", GEX_SETTINGS_OUTPUT_PDF);
    }
    else
    {
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/gex/icons/explorer.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/gex/icons/word.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/gex/icons/csv_spreadsheet.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/gex/icons/powerpoint.png"), QSize(), QIcon::Normal, QIcon::On);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/gex/icons/pdf.png"), QSize(), QIcon::Normal, QIcon::On);
        comboBoxEmailReportContents->clear();

        comboBoxEmailReportContents->addItem(icon1,"Full HTML report", GEX_SETTINGS_OUTPUT_HTML);
        comboBoxEmailReportContents->addItem(icon2, "Full WORD report", GEX_SETTINGS_OUTPUT_WORD);
        comboBoxEmailReportContents->addItem(icon3, "Full CSV report", GEX_SETTINGS_OUTPUT_CSV);
        comboBoxEmailReportContents->addItem(icon4, "Full PowerPoint report", GEX_SETTINGS_OUTPUT_PPT);
        comboBoxEmailReportContents->addItem(icon5, "Full PDF report", GEX_SETTINGS_OUTPUT_PDF);
    }

    lItemIndex = comboBoxEmailReportContents->findData(lComboEmailContentData);
    if (lItemIndex != -1)
        comboBoxEmailReportContents->setCurrentIndex(lItemIndex);
    else
    {
        comboBoxEmailReportContents->setCurrentIndex(0);
#if defined unix || __MACH__
        lItemIndex = comboBoxEmailReportContents->findData(GEX_SETTINGS_OUTPUT_PDF);
        comboBoxEmailReportContents->setCurrentIndex(lItemIndex);
#endif
    }
}
