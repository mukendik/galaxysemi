#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "admin_gui.h"
#include "db_external_database.h"
#include "pickproduct_idsql_dialog.h"
#include "pickbin_single_dialog.h"
#include "gexmo_constants.h"
#include "gex_shared.h"
#include "gex_database_entry.h"
#include "gex_database_filter.h"
#include "db_engine.h"
#include "engine.h"
#include "message.h"
#include "yield/yield_task.h"
#include "browser_dialog.h"
#include "mo_scheduler_gui.h"
#include "mo_task_create_gui.h"
#include "pickproduct_id_dialog.h"

extern CGexSkin* pGexSkin;      // holds the skin settings
extern GexMainwindow *pGexMainWindow;

GexMoCreateTaskYieldCheck::GexMoCreateTaskYieldCheck(int nDatabaseType, QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl), cYieldData(this)
{
    GSLOG(SYSLOG_SEV_DEBUG,"new GexMoCreateTaskYieldCheck widget...");
    setupUi(this);
    setModal(modal);

    // Set Examinator skin
    if (pGexSkin)
        pGexSkin->applyPalette(this);

    // Get database pointer
    m_pDatabaseEntry = (GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.isEmpty()) ?
                NULL :
                (GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.first());
    if(m_pDatabaseEntry != NULL)
        cYieldData.strProductID = "";

    connect(buttonBox_OkCancel,			SIGNAL(accepted()),				this, SLOT(OnOk()));
    connect(buttonBox_OkCancel,			SIGNAL(rejected()),				this, SLOT(OnCancel()));
    connect(PickProductID,				SIGNAL(clicked()),				this, SLOT(OnPickProductID()));
    connect(PickBinnings,				SIGNAL(clicked()),				this, SLOT(OnPickBinningList()));
    connect(pushButtonMailingList,		SIGNAL(clicked()),				this, SLOT(OnMailingList()));
    connect(lineEditProductID,			SIGNAL(textChanged(QString)),	this, SLOT(OnProductsChange(QString)));
    connect(comboBoxEmailReportContents,SIGNAL(activated(int)),			this, SLOT(OnMessageContents()));
    connect(comboBoxYieldCheckType,		SIGNAL(activated(int)),			this, SLOT(OnYieldCheckType()));
    //connect(buttonLoadblFile,			SIGNAL(clicked()),				this, SLOT(OnLoadSblFile()));
    connect(comboBoxDatabase,			SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnDatabaseChanged()));

    // Load list of databases
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(comboBoxDatabase, nDatabaseType);

    // Ensure GUI only displays rules associated with selected database.
    OnDatabaseChanged();

    // Default Title
    LineEditTitle->setText("Monitor Yield");

    // Set Yield level combo box.
    QString strYieldAlarmPercentage;
    comboBoxYieldLevel->clear();
    for(int i=100;i>=0;i--)
    {
        strYieldAlarmPercentage = QString::number(i) + " %";
        comboBoxYieldLevel->addItem(strYieldAlarmPercentage);
    }

    // Bins list to monitor
    lineEditYieldBins->setText("1");
    // Minimum parts required
    lineEditYieldMinimumParts->setText("100");

    cYieldData.strYieldBinsList = "";
    cYieldData.lMinimumyieldParts = 6;
    cYieldData.eSYA_Rule = eNone;
    cYieldData.fSYA_N1_value = 6;
    cYieldData.fSYA_N2_value = 6;
    cYieldData.iSYA_Period = GEXMO_YIELD_SYA_PERIOD_6M;

    QString strValue;
    strValue = cYieldData.strEmailFrom;
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lineEditEmailFrom->setText(GEX_EMAIL_PAT_MAN);
    else
        lineEditEmailFrom->setText(GEX_EMAIL_YIELD_MAN);

    // Ensure correct GUI fields updated
    OnMessageContents();
}

void GexMoCreateTaskYieldCheck::OnMailingList(void)
{
    // Info message about Mailing list file format.
    GS::Gex::Message::information(
        "", "Mailing list file format:\n\n o It is an ASCII file\n"
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

void GexMoCreateTaskYieldCheck::OnCancel(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "Cancelling...");
    done(-1);
}

void GexMoCreateTaskYieldCheck::OnOk(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create Task YieldCheck On OK : Check type = %1")
          .arg(comboBoxYieldCheckType->currentIndex()).toLatin1().constData());

    // Get Task title.
    cYieldData.strTitle = LineEditTitle->text();
    if(LineEditTitle->isEnabled()
            &&  cYieldData.strTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task Rule name!");
        LineEditTitle->setFocus();
        return;
    }

    // Get targetted database name,Skip the [Local]/[Server] info
    QString strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(comboBoxDatabase->isEnabled() &&  strDatabaseLogicalName == "Unknown")
    {
        // Need to enter a folder path!
        GS::Gex::Message::warning("", "You must specify a valid database!");
        comboBoxDatabase->setFocus();
        return;
    }
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section(" ",1);
    cYieldData.strDatabase = strDatabaseLogicalName;




    // The iten "Check Bins per Site" should not be available for other DB than Quantix one
    if (comboBoxYieldCheckType->currentIndex()==1)
        if ( !m_pDatabaseEntry->IsExternal() )    // ToDo : Check that it is not a MedTronic DB !
        {
            // The message has been reviewed by Ric
            GS::Gex::Message::warning(
                "Illegal selection",
                "The feature 'check bins per site' is only available when "
                "using the Quantix Yield-Man standard relational database.");
            return;
        }


    //"Unknow", "FixedYieldTreshold", "SYASBL_File", "BinsPerSite", "SYASBL_RDB" };
    // The first is the
    cYieldData.SetAttribute("CheckType",
                            QVariant(GexMoYieldMonitoringTaskData::sCheckTypes[comboBoxYieldCheckType->currentIndex()+1]) );
    // Save it anyway even if the task type is not BinsPerSite
    cYieldData.SetAttribute("MaxDeltaBinsPerSite", QVariant( MaxDeltaDoubleSpinBox->value() ) );

    // Get testing stage
    if(comboBoxTestingStage->isVisible())
        cYieldData.strTestingStage = comboBoxTestingStage->currentText();
    else
        cYieldData.strTestingStage = "";

    // Get Data source folder path
    cYieldData.strProductID = lineEditProductID->text();
    if(lineEditProductID->isEnabled() && cYieldData.strProductID.isEmpty())
    {
        // Need to enter a folder path!
        GS::Gex::Message::warning("", "You must specify the ProductID!");
        lineEditProductID->setFocus();
        return;
    }

    cYieldData.strYieldBinsList = lineEditYieldBins->text();
    if (lineEditYieldBins->isEnabled() && cYieldData.strYieldBinsList.isEmpty())
    {
        GS::Gex::Message::warning("", "Please select at least one binning in the \"Bins list to check\" field.  You can specify a list of elements (separated by a comma), each one being either an individual bin number, or a range (n1-n2). You can also use the element picker to the right of the edit box.");
        lineEditYieldBins->setFocus();
        return;
    }

    cYieldData.iBiningType = comboBoxBinType->currentIndex();	// 0= Good Bins, 1=Failing Bins
    cYieldData.iAlarmLevel = 100-comboBoxYieldLevel->currentIndex();

    cYieldData.iAlarmIfOverLimit = comboBoxYieldDirection->currentIndex();
    QString strString = lineEditYieldMinimumParts->text();
    cYieldData.lMinimumyieldParts = strString.toLong();
    QString strValue;
    strValue = cYieldData.strEmailFrom;
    cYieldData.strEmailFrom = lineEditEmailFrom->text();
    cYieldData.strEmailNotify = lineEditEmailList->text();
    cYieldData.bHtmlEmail = (comboBoxMailFormat->currentIndex() == 0) ? true : false;
    cYieldData.iEmailReportType = comboBoxEmailReportContents->currentIndex();
    cYieldData.iNotificationType = comboBoxGeneration->currentIndex();
    cYieldData.iExceptionLevel = comboBoxExceptionLevel->currentIndex();

    if (lineEditEmailList->isEnabled()
            && cYieldData.strEmailNotify.isEmpty()  //.isEmpty() == true)
            )
    {
        // Need to specify an email address
        if(QMessageBox::question( this,
                                  GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
                                  "No email specified...\nYield alarms won't send email notifications!\n\nDo you confirm this choice?",
                                  QMessageBox::Yes,QMessageBox::No | QMessageBox::Default) != QMessageBox::Yes)
        {
            lineEditEmailList->setFocus();
            return;
        }
    }

    done(1);
}

void GexMoCreateTaskYieldCheck::LoadFields(
        GexMoYieldMonitoringTaskData *ptYieldData,
        QStringList *plistTestingStages/*=NULL*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Load fields for RuleName : '%1'")
          .arg(ptYieldData->strTitle).toLatin1().data());

    // Task Title
    LineEditTitle->setText(ptYieldData->strTitle );

    // Fill Testing stage combobox
    comboBoxTestingStage->clear();
    if(plistTestingStages)
        comboBoxTestingStage->addItems(*plistTestingStages);

    // Targeted database
    // Check if it is a new task (empty productId)
    if(!ptYieldData->strProductID.isEmpty())
    {
        int iIndex;
        QString strDatabaseName;
        QString strDatabaseLogicalName;
        strDatabaseName = ptYieldData->strDatabase;
        if(strDatabaseName.isEmpty())
            strDatabaseName = "Unknown";
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
            strDatabaseName = "Unknown";
            comboBoxDatabase->addItem(QIcon(QString::fromUtf8(":/gex/icons/error.png")), strDatabaseName);
            comboBoxDatabase->setCurrentIndex(comboBoxDatabase->count()-1);
        }
    }

    OnDatabaseChanged();

    // Testing stage
    if(!ptYieldData->strTestingStage.isEmpty())
    {
        int lIndex = comboBoxTestingStage->findText(ptYieldData->strTestingStage);
        if(lIndex != -1)
            comboBoxTestingStage->setCurrentIndex(lIndex);
    }

    // ProductID
    lineEditProductID->setText( ptYieldData->strProductID);

    // Good bins list
    lineEditYieldBins->setText(ptYieldData->strYieldBinsList);

    // Binning type (0=Good bins, 1=Failing bins).
    comboBoxBinType->setCurrentIndex(ptYieldData->iBiningType);

    // Yield level alarm
    comboBoxYieldLevel->setCurrentIndex(100-ptYieldData->iAlarmLevel);

    // Check if Yield Higher or Under Limit?
    comboBoxYieldDirection->setCurrentIndex(ptYieldData->iAlarmIfOverLimit);

    // Minimum parts to triger alarm
    lineEditYieldMinimumParts->setText(QString::number(ptYieldData->lMinimumyieldParts));

    // Set SBL/YBL data file
    //	if(QFile::exists(ptYieldData->strSblFile))
    QString checkType=ptYieldData->GetAttribute("CheckType").toString();
    //  << "Unknow" << "FixedYieldTreshold" << "SYASBL_File" << "BinsPerSite" << "SYASBL_RDB";
    if ( checkType=="FixedYieldTreshold" )
    {
        // Combo box: Enable basic yield check mode (over hard threshold)
        comboBoxYieldCheckType->setCurrentIndex(0);
        // SBL/YBL not set so far
        //lineEditSblFile->setText("");
    }
    else if ( checkType=="BinsPerSite" )
    {
        comboBoxYieldCheckType->setCurrentIndex(1);
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unsupported or illogic CheckType attribute '%1'").arg( checkType).toLatin1().constData() );
        ptYieldData->SetAttribute("CheckType", "FixedYieldTreshold");
        comboBoxYieldCheckType->setCurrentIndex(0);
    }

    MaxDeltaDoubleSpinBox->setValue( ptYieldData->GetAttribute("MaxDeltaBinsPerSite").toDouble() );

    QString strValue;
    strValue = ptYieldData->strEmailFrom;
    // 'From' Email address
    lineEditEmailFrom->setText(ptYieldData->strEmailFrom);

    // Emails to notify
    lineEditEmailList->setText(ptYieldData->strEmailNotify );

    // Email format: HTML or ASCII
    if(ptYieldData->bHtmlEmail)
        comboBoxMailFormat->setCurrentIndex(0);	// HTML (default)
    else
        comboBoxMailFormat->setCurrentIndex(1);	// TEXT

    // Email report contents
    comboBoxEmailReportContents->setCurrentIndex(ptYieldData->iEmailReportType);

    // Report notification type: Attach to email, or store on server?
    comboBoxGeneration->setCurrentIndex(ptYieldData->iNotificationType);

    // Exception level: Standard, Critical, etc...
    comboBoxExceptionLevel->setCurrentIndex(ptYieldData->iExceptionLevel);

    // show/Hide relevant fields
    OnMessageContents();
}

void	GexMoCreateTaskYieldCheck::LoadFields(CGexMoTaskYieldMonitor *ptTaskItem, QStringList *plistTestingStages/*=NULL*/)
{
    LoadFields(ptTaskItem->GetProperties(), plistTestingStages);
}

void GexMoCreateTaskYieldCheck::OnPickProductID(void)
{
    GSLOG(SYSLOG_SEV_DEBUG, "On Pick Product ID...");
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
        // RDB SYL/SBL
        // no multi select
        if (cYieldData.eSYA_Rule >= 0)
            dPickFilter.fillList(strlProductList, false, false);
        else
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

    lineEditYieldBins->setText("");
}

void GexMoCreateTaskYieldCheck::OnPickBinningList()
{
    GSLOG(SYSLOG_SEV_DEBUG, "On Pick Binning List...");
    // Get Database name we have to look into...get string '[Local] <Database name>'
    QString strDatabaseName = comboBoxDatabase->currentText();
    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();
    if(m_pDatabaseEntry && m_pDatabaseEntry->m_pExternalDatabase)
    {
        QString strProductFilter = lineEditProductID->text();

        GexDatabaseFilter cFilter;

        cFilter.addNarrowFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strProductFilter);
        cFilter.strDataTypeQuery = comboBoxTestingStage->currentText();

        cFilter.strDatabaseLogicalName = strDatabaseName;

        // Get list of strings to fill in list box
        QStringList cBinningList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetBinningList(cFilter, true);
        if(cBinningList.count() == 0)
            return;

        // Fill Filter list with relevant strings
        PickBinSingleDialog dPickFilter;
        dPickFilter.fillList(cBinningList);
        dPickFilter.setMultipleSelection(true);

        // Prompt dialog box, let user pick Filter string from the list
        if(dPickFilter.exec() != 1)
            return;	// User 'Abort'

        // Return the list selected (with '|' replaced by ',', wich is more appropriate for a binlist)
        lineEditYieldBins->setText(dPickFilter.getBinsList().replace('|',','));
        lineEditYieldBins->setFocus();
    }
    else
    {
    }
}

void GexMoCreateTaskYieldCheck::OnProductsChange(const QString &strString)
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

void GexMoCreateTaskYieldCheck::OnYieldCheckType(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("On Yield Check type changed : %1 : %2")
          .arg( comboBoxYieldCheckType->currentIndex())
          .arg(comboBoxYieldCheckType->currentText()).toLatin1().data());

    // RDB SYL/SBL
    if(cYieldData.eSYA_Rule >= 0)
    {
        stackedWidget->setCurrentIndex(3); // The last page is SYASBL RDB
        comboBoxYieldCheckType->setCurrentIndex(1);
        cYieldData.SetAttribute("CheckType", "SYASBL_RDB");
        TextLabelBinType->hide();
        comboBoxBinType->hide();
        return;
    }

    // Show relevant Widget in stack.
    int iMode = comboBoxYieldCheckType->currentIndex();	// 0
    stackedWidget->setCurrentIndex(iMode);

    switch(iMode)
    {
    case 0:	// Standard Hard coded yield limit
        horizontalLayout_4->setEnabled(true);
        TextLabelBinType->show();
        comboBoxBinType->show();
        cYieldData.SetAttribute("CheckType", "FixedYieldTreshold");
        break;
    case 1: // Bins % per site
        TextLabelBinType->hide();
        horizontalLayout_4->setEnabled(false);
        comboBoxBinType->hide();
        cYieldData.SetAttribute("CheckType", "BinsPerSite");
        break;

    default:
        GSLOG(SYSLOG_SEV_WARNING, "Unknown check type mode");
        cYieldData.SetAttribute("CheckType", "Unknown");
        break;
    }
}

///////////////////////////////////////////////////////////
// Select SBL/YBL data file (Statistical Yield Limit, Yield Bin limit)
///////////////////////////////////////////////////////////
/*
void	GexMoCreateTaskYieldCheck::OnLoadSblFile(void)
{
    QString strDefaultSelection = lineEditSblFile->text();

    // Prompt user to select sbl file
    QString strSblFile = QFileDialog::getOpenFileName(this, "Select SBL/YBL File", strDefaultSelection, "Statistical Bin Limits SBL/YBL (*.sbl)");

    if(strSblFile.isEmpty())
        return;

    lineEditSblFile->setText(strSblFile);
}
*/

void GexMoCreateTaskYieldCheck::OnMessageContents(void)
{
    int	iSelection = comboBoxEmailReportContents->currentIndex();

    // If No Report to create, then hide fields to define where report is stored
    if(iSelection == 0)
    {
        TextLabelReportLocation->hide();
        comboBoxGeneration->hide();
    }
    else
    {
        TextLabelReportLocation->show();
        comboBoxGeneration->show();
    }

    // Show/hide fields related to SBL file
    OnYieldCheckType();

    // Under Unix: refuse Word generation
#if defined unix || __MACH__
    switch(iSelection)
    {
    case 0:	// Text
    case 2:	// CSV
    case 4:	// PDF
        break;
    case 1:	// WORD
        // Not available under unix...
        GS::Gex::Message::information(
            "",
            "The Word document generation is not available under Unix & Linux\n"
            "You need Yield-Man for Windows.\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        comboBoxEmailReportContents->setCurrentIndex(0);	// Reset output format to: ASCII

        // show/Hide relevant fields
        OnMessageContents();
        break;

    case 3:	// PowerPoint
        // Not available under unix...
        GS::Gex::Message::information(
            "", "The PowerPoint document generation is not available under "
            "Unix & Linux\nYou need Yield-Man for Windows\n\nContact " +
            QString(GEX_EMAIL_SALES) + " for more information!");
        comboBoxEmailReportContents->setCurrentIndex(0);	// Reset output format to: ASCII

        // show/Hide relevant fields
        OnMessageContents();
    }
#endif
}

void GexMoCreateTaskYieldCheck::OnDatabaseChanged(void)
{
    // Get database pointer
    GexDatabaseEntry *pDatabaseEntry=NULL;
    QString strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section(' ',1);
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName,false);

    // Do nothing if database not found
    if(!pDatabaseEntry)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Cannot find a DatabaseEntry !");
        return;
    }

    /*
    // The iten "Bin per Site" should nt be visible for file based DB
    if (pDatabaseEntry->bExternal)
    {
      comboBoxYieldCheckType->();
    }
  */

    lineEditProductID->setText("");
    lineEditYieldBins->setText("");

    QStringList listTestingStages;
    // For selecting data type to query (wafer sort, or final test,...)
    if(pDatabaseEntry->m_pExternalDatabase  && pDatabaseEntry->m_pExternalDatabase->IsTestingStagesSupported())
        pDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(listTestingStages);

    // Fill Testing stage combobox
    comboBoxTestingStage->clear();
    comboBoxTestingStage->addItems(listTestingStages);

    if(listTestingStages.isEmpty())
        comboBoxTestingStage->hide();
    else
        comboBoxTestingStage->show();

    m_pDatabaseEntry = pDatabaseEntry;

    // RDB SYL/SBL
    if(cYieldData.eSYA_Rule >= 0)
        PickBinnings->hide();
    else
    {
        if(m_pDatabaseEntry && m_pDatabaseEntry->m_pExternalDatabase)
            PickBinnings->show();
        else
            PickBinnings->hide();
    }
}
