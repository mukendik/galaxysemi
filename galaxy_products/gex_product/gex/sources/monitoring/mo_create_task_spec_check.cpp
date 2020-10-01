#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "gexmo_constants.h"
#include "gex_shared.h"
#include "admin_gui.h"
#include "browser.h"
#include "db_engine.h"
#include "db_external_database.h"
#include "db_gexdatabasequery.h"
#include "engine.h"
#include "pickproduct_idsql_dialog.h"
#include "gex_database_entry.h"
#include "gex_database_filter.h"
#include "message.h"
#include "onefile_wizard.h"
#include "browser_dialog.h"
#include "picktest_dialog.h"
#include "mo_scheduler_gui.h"
#include "mo_task_create_gui.h"
#include "pickproduct_id_dialog.h"


extern GexMainwindow* pGexMainWindow;

GexMoCreateTaskSpecCheck::GexMoCreateTaskSpecCheck(int nDatabaseType, QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl) , cSpecData(this)
{
    setupUi(this);
    setModal(modal);

    // Get database pointer
    m_pDatabaseEntry = NULL;
    m_pDatabaseEntry = (GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.isEmpty()) ?
                NULL :
                (GS::Gex::Engine::GetInstance().GetDatabaseEngine().mDatabaseEntries.first());
    if(m_pDatabaseEntry != NULL)
        cSpecData.strProductID = "";

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(buttonBox_OkCancel,	SIGNAL(accepted()),				this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,    SIGNAL(rejected()),				this, SLOT(OnCancel()));
    QObject::connect(PickProductID,			SIGNAL(clicked()),				this, SLOT(OnPickProductID()));
    QObject::connect(pushButtonMailingList, SIGNAL(clicked()),				this, SLOT(OnMailingList()));
    QObject::connect(lineEditProductID,		SIGNAL(textChanged(QString)),	this, SLOT(OnProductsChange(QString)));
    QObject::connect(PickTests,				SIGNAL(clicked()),				this, SLOT(OnPickTests()));
    connect(comboBoxDatabase,				SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnDatabaseChanged()));

    // Load list of databases
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(comboBoxDatabase, nDatabaseType);

    // Ensure GUI only displays rules associated with selected database.
    OnDatabaseChanged();


    // Default Title
    LineEditTitle->setText("Monitor Spec");

    //	// Load list of Spec info to monitor (test value, Cp, Cpk, mean, etc...)
    //	comboBoxMonitorInfo->clear();

    //	int nItem = 0;
    //	while(gexMoLabelSpecInfo[nItem])
    //		comboBoxMonitorInfo->insertItem(gexMoLabelSpecInfo[nItem++]);

    // Focus on 1st Edit field
    LineEditTitle->setFocus();

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lineEditEmailFrom->setText(GEX_EMAIL_PAT_MAN);
    else
        lineEditEmailFrom->setText(GEX_EMAIL_YIELD_MAN);


    // Clear flag
    bRebuildTestList = true;

    m_poMonitorTableWidget->resizeRowsToContents();
    m_poMonitorTableWidget->resizeColumnsToContents();
    addRowWithCombo(0);
}

void GexMoCreateTaskSpecCheck::OnMailingList(void)
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

void GexMoCreateTaskSpecCheck::OnCancel(void)
{
    done(-1);
}

void GexMoCreateTaskSpecCheck::OnOk(void)
{
    // Get Task title.
    cSpecData.strTitle = LineEditTitle->text();
    if(cSpecData.strTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    // Get targetted database name,Skip the [Local]/[Server] info
    QString strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName == "Unknown")
    {
        // Need to enter a folder path!
        GS::Gex::Message::warning("", "You must specify a valid database!");
        comboBoxDatabase->setFocus();
        return;
    }
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section(" ",1);
    cSpecData.strDatabase = strDatabaseLogicalName;

    // Get testing stage
    if(comboBoxTestingStage->isVisible())
        cSpecData.strTestingStage = comboBoxTestingStage->currentText();
    else
        cSpecData.strTestingStage = "";

    // Get ProductID
    cSpecData.strProductID= lineEditProductID->text();
    if(cSpecData.strProductID.isEmpty())
    {
        // Need to enter a folder path!
        GS::Gex::Message::warning("", "You must specify the ProductID!");
        lineEditProductID->setFocus();
        return;
    }

    // Parameter/Test#
    cSpecData.strParameter= lineEditParameter->text();
    if(cSpecData.strParameter.isEmpty())
    {
        // Need to enter a folder path!
        GS::Gex::Message::
            warning("", "You must specify the Parameter/Test# to monitor!");
        lineEditParameter->setFocus();
        return;
    }

    if(!m_poMonitorTableWidget->rowCount())
    {
        GS::Gex::Message::
            warning("", "You must specify at least on value to monitor!");
        //lineEditLowestValue->setFocus();
        return;
    }

    for(int iRow =0; iRow<m_poMonitorTableWidget->rowCount();iRow++){
        QComboBox *poCombo =  qobject_cast<QComboBox *> (m_poMonitorTableWidget->cellWidget(iRow, 0));
        int iMonitorInfo = poCombo->currentIndex();
        QString strMonitorInfo = poCombo->currentText();

        QString strLowSpec = (qobject_cast<QLineEdit *> (m_poMonitorTableWidget->cellWidget(iRow, 1)))->text();
        //m_poMonitorTableWidget->item(iRow, 1)->text();
        QString strHighSpec = (qobject_cast<QLineEdit *> (m_poMonitorTableWidget->cellWidget(iRow, 2)))->text();
        if(strLowSpec.isEmpty() && strHighSpec.isEmpty())
        {
            // Need to enter a Low or high spec
            GS::Gex::Message::warning(
                "",
                QString("You must specify a Lowest and/or Highest "
                        "value for monitored value '%1' at line %2!").
                arg(strMonitorInfo).arg(iRow+1));
            //lineEditLowestValue->setFocus();
            cSpecData.m_mapMonitoredParam.clear();
            return;
        }
        if(!strLowSpec.isEmpty() && strLowSpec.toDouble() > strHighSpec.toDouble()){
            // Need to enter a Low or high spec
            GS::Gex::Message::warning(
                "",
                QString("Lowest value for monitored value '%1' at "
                        "line %2! must be lower than Highest").
                arg(strMonitorInfo).arg(iRow+1));
            //lineEditLowestValue->setFocus();
            cSpecData.m_mapMonitoredParam.clear();
            return;
        }
        cSpecData.m_mapMonitoredParam.insert(iMonitorInfo, QPair<QString,QString>(strLowSpec,strHighSpec));
    }

    //	// What info to monitor on the parameter.
    //	cSpecData.iMonitorInfo = comboBoxMonitorInfo->currentIndex();
    //	// Lowest & Highest values allowed
    //	cSpecData.strLowSpec = lineEditLowestValue->text();
    //	cSpecData.strHighSpec = lineEditHighestValue->text();
    //	if(cSpecData.strLowSpec.isEmpty() && cSpecData.strHighSpec.isEmpty())
    //	{
    //		// Need to enter a Low or high spec
    //      warning(this, szAppFullName,
    //              "You must specify a Lowest and/or Highest value!");
    //		lineEditLowestValue->setFocus();
    //		return;
    //	}


    cSpecData.strEmailFrom = lineEditEmailFrom->text();
    if (cSpecData.strEmailFrom.isEmpty() == true)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            cSpecData.strEmailFrom = GEX_EMAIL_PAT_MAN;
        else
            cSpecData.strEmailFrom = GEX_EMAIL_YIELD_MAN;
    }
    cSpecData.strEmailNotify = lineEditEmailList->text();
    if(cSpecData.strEmailNotify.isEmpty() == true)
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

    cSpecData.bHtmlEmail = (comboBoxMailFormat->currentIndex() == 0) ? true : false;

    cSpecData.iExceptionLevel= comboBoxExceptionLevel->currentIndex();

    done(1);
}

void GexMoCreateTaskSpecCheck::LoadFields(CGexMoTaskSpecMonitor *ptTaskItem)
{
    // Targeted database
    // Check if it is a new task (empty productId)
    if(ptTaskItem->GetProperties())
    {
        // Task Title
        LineEditTitle->setText(ptTaskItem->GetProperties()->strTitle);

        int iIndex;
        QString strDatabaseName;
        QString strDatabaseLogicalName;
        strDatabaseName = ptTaskItem->GetProperties()->strDatabase;
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
    if(!ptTaskItem->GetProperties()->strTestingStage.isEmpty())
    {
        int lIndex = comboBoxTestingStage->findText(ptTaskItem->GetProperties()->strTestingStage);
        if(lIndex != -1)
            comboBoxTestingStage->setCurrentIndex(lIndex);
    }

    // ProductID
    lineEditProductID->setText(ptTaskItem->GetProperties()->strProductID);

    // Parameter/Test#
    lineEditParameter->setText(ptTaskItem->GetProperties()->strParameter);

    QList<int> keysList = ptTaskItem->GetProperties()->m_mapMonitoredParam.keys();
    int iIdxRow = 0;

    if(!keysList.isEmpty()){

        m_poMonitorTableWidget->removeRow(0);
        foreach(int iMonitorInfo, keysList){
            addRowWithCombo(iIdxRow,iMonitorInfo);
            (qobject_cast<QLineEdit *> (m_poMonitorTableWidget->cellWidget(iIdxRow, 1)))->setText(ptTaskItem->GetProperties()->m_mapMonitoredParam[iMonitorInfo].first);
            (qobject_cast<QLineEdit *> (m_poMonitorTableWidget->cellWidget(iIdxRow, 2)))->setText(ptTaskItem->GetProperties()->m_mapMonitoredParam[iMonitorInfo].second);
        }
    }


    //	// What to monitor
    //	comboBoxMonitorInfo->setCurrentIndex(ptTaskItem->ptSpec->iMonitorInfo);

    //	// Low & High specs limits.
    //	lineEditLowestValue->setText(ptTaskItem->ptSpec->strLowSpec);
    //	lineEditHighestValue->setText(ptTaskItem->ptSpec->strHighSpec);

    // Email 'From':
    lineEditEmailFrom->setText(ptTaskItem->GetProperties()->strEmailFrom);

    // Emails to notify
    lineEditEmailList->setText(ptTaskItem->GetProperties()->strEmailNotify);

    // Email format: HTML or ASCII
    if(ptTaskItem->GetProperties()->bHtmlEmail)
        comboBoxMailFormat->setCurrentIndex(0);	// HTML (default)
    else
        comboBoxMailFormat->setCurrentIndex(1);	// TEXT

    // Exception level: Standard, Critical, etc...
    comboBoxExceptionLevel->setCurrentIndex(ptTaskItem->GetProperties()->iExceptionLevel);
}

void GexMoCreateTaskSpecCheck::OnPickProductID(void)
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
        strProductFilter = strProductFilter.replace("_","\\_").replace("%","\\%").replace("*","%").replace("?","_").replace(",","|");

        // Execute query on remote database
        bool qpl=m_pDatabaseEntry->m_pExternalDatabase->QueryProductList(clPluginFilter,strlProductList,strProductFilter);
        if (!qpl)
        {
            GSLOG(SYSLOG_SEV_ERROR, " QueryProductList failed !");
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

    lineEditParameter->setText("");
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void	GexMoCreateTaskSpecCheck::OnProductsChange(const QString &strString)
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

    // Forces to rebuild test list if 'Pich Test' icon is clicked.
    bRebuildTestList = true;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void	GexMoCreateTaskSpecCheck::OnPickTests(void)
{
    if(pGexMainWindow == NULL)
        return;

    // Get Database name we have to look into...get string '[Local] <Database name>'
    QString strDatabaseName = comboBoxDatabase->currentText();
    // Skip the [Local]/[Server] info, and extract database name.
    if(strDatabaseName.startsWith("[Local]") || strDatabaseName.startsWith("[Server]"))
        strDatabaseName = strDatabaseName.section("]",1).trimmed();

    if(m_pDatabaseEntry && m_pDatabaseEntry->m_pExternalDatabase)
    {
        QString strProductFilter = lineEditProductID->text();
        strProductFilter = strProductFilter.replace("_","\\_").replace("%","\\%").replace("*","%").replace("?","_").replace(",","|");

        GexDatabaseFilter cFilter;

        cFilter.addNarrowFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strProductFilter);
        //		QComboBox comboProduct;
        //		QComboBox comboProductValue;
        //		comboProduct.insertItem(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]);
        //		comboProductValue.insertItem(strProductFilter);
        //		cFilter.pQueryFilter = &comboProduct;
        //		cFilter.pQueryValue = &comboProductValue;
        //		cFilter.pNarrowFilter1 = NULL;
        //		cFilter.pNarrowValue1 = NULL;
        //		cFilter.pNarrowFilter2 = NULL;
        //		cFilter.pNarrowValue2 = NULL;
        //		cFilter.pNarrowFilter3 = NULL;
        //		cFilter.pNarrowValue3 = NULL;
        //		cFilter.pNarrowFilter4 = NULL;
        //		cFilter.pNarrowValue4 = NULL;
        //		cFilter.pNarrowFilter5 = NULL;
        //		cFilter.pNarrowValue5 = NULL;
        //		cFilter.pNarrowFilter6 = NULL;
        //		cFilter.pNarrowValue6 = NULL;
        //		cFilter.pNarrowFilter7 = NULL;
        //		cFilter.pNarrowValue7 = NULL;

        cFilter.strDataTypeQuery = comboBoxTestingStage->currentText();

        cFilter.strDatabaseLogicalName = strDatabaseName;

        // Get list of strings to fill in list box
        QStringList cParameterList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetParameterList(cFilter, true);

        // Fill Dialog box with parameter list...
        // Show TestList
        PickTestDialog dPickTest;

        // Allow Multiple selections.
        dPickTest.setMultipleSelection(true);

        // Check if List was successfuly loaded
        if (dPickTest.fillParameterList(cParameterList))
        {
            // Prompt dialog box, let user pick tests from the list
            if(dPickTest.exec() == QDialog::Accepted)
            {
                // Save the list selected into the edit field...
                lineEditParameter->setText(dPickTest.testList());
            }
        }
    }
    else if(m_pDatabaseEntry)
    {
        // Get first file matching the Product criteria
        QStringList sFilesMatchingQuery;
        GexDatabaseQuery cQuery;
        QString	strFilter = lineEditProductID->text();
        QRegExp rx("", Qt::CaseInsensitive);	// NOT case sensitive
        cQuery.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;
        cQuery.iProcessData = GEX_PROCESSPART_ALL;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PRODUCT;


        // Build path to the relevant Index file that holds all valid Filter values
        // Path= <Database name>/filters/<filter_name_ID>
        QString strIndexFile = m_pDatabaseEntry->PhysicalPath() + GEX_DATABASE_FILTER_FOLDER;
        strIndexFile += gexFilterChoices[GEX_QUERY_FILTER_PRODUCT];

        // Read Filter table file, so to fill list.
        QFile f(strIndexFile);
        if(f.open(QIODevice::ReadOnly))
        {
            // Filter table file exists...read it to see if matching entry found.
            QString strString;
            QTextStream hFilterTableFile(&f);	// Assign file handle to data stream

            // Read all lines, fill list box.
            do
            {
                strString = hFilterTableFile.readLine();
                if(strString.isEmpty() == false)
                {
                    rx.setPattern(strFilter);
                    // If wildcar used, set its support.
                    if(strFilter.indexOf("*") >= 0 || strFilter.indexOf("?") >= 0)
                        rx.setPatternSyntax(QRegExp::Wildcard);
                    else
                        rx.setPatternSyntax(QRegExp::RegExp);
                    if(rx.indexIn(strString) >= 0)
                    {
                        // Product matchinng product criteria...
                        cQuery.strDatabaseLogicalName = m_pDatabaseEntry->LogicalName();
                        cQuery.bLocalDatabase = m_pDatabaseEntry->IsStoredInFolderLocal();
                        cQuery.strProductID = strString;
                        // Find the 1st file matching this criteria
                        sFilesMatchingQuery = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFiles(&cQuery,1);
                        if(sFilesMatchingQuery.count() > 0)
                        {
                            f.close();
                            break;
                        }
                    }

                }
            }
            while(hFilterTableFile.atEnd() == false);

            // Close file
            f.close();
        }
        if(sFilesMatchingQuery.count() == 0)
        {
            GS::Gex::Message::information(
                "", "Sorry...No data available\nDatabases are probably empty");
            return;	// Probably no file in the database yet...
        }

        // Select this file as the one to scan to build its test list...
        pGexMainWindow->pWizardOneFile->OnSelectFile(sFilesMatchingQuery[0]);

        pGexMainWindow->SetWizardType(GEX_ONEFILE_WIZARD);

        if(bRebuildTestList)
            pGexMainWindow->iHtmlSectionsToSkip = 0;
        else
            pGexMainWindow->iHtmlSectionsToSkip = GEX_HTMLSECTION_ALL;

        // Show TestList
        PickTestDialog dPickTest;

        // Allow Multiple selections.
        dPickTest.setMultipleSelection(true);

        // Check if List was successfuly loaded
        if (dPickTest.fillParameterList(-1, bRebuildTestList))
        {
            // Prompt dialog box, let user pick tests from the list
            if(dPickTest.exec() == QDialog::Accepted)
            {
                // Save the list selected into the edit field...
                lineEditParameter->setText(dPickTest.testList());
            }
        }

        // Clear flag
        bRebuildTestList = false;
    }

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void GexMoCreateTaskSpecCheck::OnDatabaseChanged(void)
{
    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry=NULL;
    QString				strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section(' ',1);
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName,false);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    lineEditProductID->setText("");
    lineEditParameter->setText("");

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
}

void GexMoCreateTaskSpecCheck::addMonitorValue()
{
    int iRowCount= m_poMonitorTableWidget->rowCount();
    if(iRowCount>2)
    {
        GS::Gex::Message::
            information("", "Only 3 parameters spec are allowed by task");
        return;
    }

    addRowWithCombo(iRowCount);
}

void GexMoCreateTaskSpecCheck::removeMonitorValue()
{
    QWidget *poSender = qobject_cast<QWidget *> (sender());
    if(!poSender)
        return;
    int iRowCount= m_poMonitorTableWidget->rowCount();
    if(iRowCount == 1)
    {
        GS::Gex::Message::
            information("", "You must specify at least on value to monitor!");
        return;
    }
    for(int iRow =0; iRow<m_poMonitorTableWidget->rowCount(); iRow++){
        if(m_poMonitorTableWidget->cellWidget(iRow, 3) == poSender->parent()){
            m_poMonitorTableWidget->removeRow(iRow);
            return;
        }
    }
}

void GexMoCreateTaskSpecCheck::addRowWithCombo(int iRow, int iCurrent)
{
    m_poMonitorTableWidget->insertRow(iRow);
    QComboBox *poComboBoxMonitorInfo = new QComboBox;
    int nItem = 0;
    while(gexMoLabelSpecInfo[nItem])
        poComboBoxMonitorInfo->addItem(gexMoLabelSpecInfo[nItem++]);
    poComboBoxMonitorInfo->setCurrentIndex(iCurrent);
    m_poMonitorTableWidget->setCellWidget(iRow, 0, poComboBoxMonitorInfo);
    poComboBoxMonitorInfo->setSizeAdjustPolicy(QComboBox::AdjustToContents);


    QLineEdit *poItem = new QLineEdit;
    poItem->setValidator(new QDoubleValidator);
    m_poMonitorTableWidget->setCellWidget(iRow, 1, poItem);

    poItem = new QLineEdit;
    poItem->setValidator(new QDoubleValidator);
    m_poMonitorTableWidget->setCellWidget(iRow, 2, poItem);

    QWidget *poTempWidget = new QWidget();

    QPushButton *poAdd = new QPushButton();
    poAdd->setMinimumSize(QSize(16, 16));
    poAdd->setMaximumSize(QSize(18, 18));
    poAdd->setIcon(QIcon (":/gex/icons/add.png"));
    QObject::connect(poAdd, SIGNAL(clicked()), this, SLOT(addMonitorValue()));

    QPushButton *poRemove= new QPushButton();
    poRemove->setMinimumSize(QSize(16, 16));
    poRemove->setMaximumSize(QSize(18, 18));
    poRemove->setIcon(QIcon (":/gex/icons/remove.png"));
    QObject::connect(poRemove, SIGNAL(clicked()), this, SLOT(removeMonitorValue()));

    QGridLayout *poTempGridLayout = new QGridLayout(poTempWidget);
    poTempGridLayout->setSpacing(0);
    poTempGridLayout->setContentsMargins(0, 0, 0, 0);
    poTempGridLayout->addWidget(poAdd, 0, 0, 1, 1);
    poTempGridLayout->addWidget(poRemove, 0, 1, 1, 1);

    m_poMonitorTableWidget->setCellWidget(iRow, 3, poTempWidget);

    m_poMonitorTableWidget->resizeRowsToContents();
    m_poMonitorTableWidget->resizeColumnsToContents();
    if(iRow == 0)
        m_poMonitorTableWidget->adjustSize();

}

