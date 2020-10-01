///////////////////////////////////////////////////////////
// Class to create a New Database Entry
///////////////////////////////////////////////////////////
#include <gqtl_sysutils.h>
// Galaxy modules includes
#include <gqtl_skin.h>
#include <QFileDialog>
#include <QMessageBox>

#include "db_new_database_dialog.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "admin_engine.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "message.h"

#include <QStandardItemModel>


// report_build.cpp
extern CReportOptions     ReportOptions;      // Holds options (report_build.h)
extern CGexSkin*          pGexSkin;           // holds the skin settings

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CreateNewDatabaseDialog::CreateNewDatabaseDialog( QWidget* parent, bool modal )
    : QDialog( parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint )
{
    GSLOG(SYSLOG_SEV_DEBUG, " new CreateNewDatabaseDialog");
    setupUi(this);
    setModal(modal);

    // Set Examinator skin
    if (pGexSkin)
        pGexSkin->applyPalette(this);

    QObject::connect(PushButtonCreate,            SIGNAL(clicked()),
                     this, SLOT(OnButtonCreate()));
    QObject::connect(PushButtonCancel,            SIGNAL(clicked()),
                     this, SLOT(reject()));
    QObject::connect(checkBoxPasswordImportFile,  SIGNAL(clicked()),
                     this, SLOT(OnPasswordImportFiles()));
    QObject::connect(checkBoxDeleteDatabase,      SIGNAL(clicked()),
                     this, SLOT(OnPasswordDeleteDatabase()));
    //  QObject::connect(pushButtonMore,              SIGNAL(clicked()), this, SLOT(OnMore()));
    //  QObject::connect(buttonLoadCorporateProfile,  SIGNAL(clicked()), this, SLOT(OnImportCorporateDatabase()));
    //  QObject::connect(buttonSaveCorporateProfile,  SIGNAL(clicked()), this, SLOT(OnExportCorporateDatabase()));
    QObject::connect(comboBoxDatabaseCopyMode,    SIGNAL(activated(QString)),
                     this, SLOT(OnStorageType()));
    QObject::connect(LineEditDatabaseName,        SIGNAL(textChanged(QString)),
                     this, SLOT(UpdateCreateButton()));

    QString strDummyItem;

    // Fill list of GexDb database plugins
    m_pRemoteDatabase = new GexRemoteDatabase();
    m_pCurrentPlugin = NULL;

    // Check if license allows database plugins
    if(!(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed()))
    {
        strDummyItem = "<Your license doesn't support database plug-ins. Contact "+QString(GEX_EMAIL_SALES)+".>";
        comboPluginList->insertItem(comboPluginList->count(),strDummyItem);
    }
    //  else if(GS::Gex::Engine::GetInstance().GetAdminEngine() == NULL)
    //  {
    //    strDummyItem = "<Your installation doesn't support database plug-ins. Contact "+QString(GEX_EMAIL_SALES)+".>";
    //    comboPluginList->insertItem(strDummyItem);
    //  }
    else
    {
        m_pRemoteDatabase->GetAvailablePlugins(m_pPluginList);

        QList<GexDbPlugin_ID*>::iterator itPlugin = m_pPluginList.begin();

        while (itPlugin != m_pPluginList.end())
        {
            comboPluginList->insertItem(comboPluginList->count(),(*itPlugin)->pluginName());
            itPlugin++;
        }

        // If no plugin found, disable configure button, add dummy item
        if(m_pPluginList.count() == 0)
        {
            strDummyItem = "<No database plug-in found in directory ";
            strDummyItem += m_pRemoteDatabase->GetPluginDirectory();
            strDummyItem += ">";
            comboPluginList->insertItem(comboPluginList->count(),strDummyItem);
        }
        else
            OnDatabasePluginChanged();
    }

    // Signal/Slot connection
    connect(buttonConfigurePlugin,    SIGNAL(clicked()),
            this, SLOT(OnButtonConfigurePlugin()));
    connect(comboPluginList,          SIGNAL(activated(const QString &)),
            this, SLOT(OnDatabasePluginChanged()));

    // Set focus on the Database Name Edit field.
    LineEditDatabaseName->setFocus();

    // Passwords are by default disabled
    LineEditImportPassword->setEnabled(false);
    LineEditDeletePassword->setEnabled(false);

    // At launch, dialog box must be reduced, not expanded
    m_bExpanded = true;
    //  OnStorageType();
    //  OnMore();	// sets m_bExpanded to 'false' + resize dialog box.

    // Fill list with database types available...
    comboBoxDatabaseCopyMode->clear();

    int iIndex = 0;
    // ALLOW ALL TYPE OF DATABASES
    //if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    //  {
    //    comboBoxDatabaseCopyMode->insertItem(iIndex++,
    //                        "Compressed (Zip mode)",QVariant(DB_STORAGEMODE_ZIP));
    //    comboBoxDatabaseCopyMode->insertItem(iIndex++,
    //                        "Make a full Copy of the data files (Copy mode)",QVariant(DB_STORAGEMODE_COPY));
    //    comboBoxDatabaseCopyMode->insertItem(iIndex++,
    //                        "Only hold Links to the data files (Link mode)",QVariant(DB_STORAGEMODE_LINK));
    //    comboBoxDatabaseCopyMode->insertItem(iIndex++,
    //                        "Only hold Summary data (Summary mode)",QVariant(DB_STORAGEMODE_SUMMARY));

    //    // DISABLE USE OF FLAT DB
    //    qobject_cast<QStandardItemModel *>(comboBoxDatabaseCopyMode->model())->item(
    //                comboBoxDatabaseCopyMode->findData(QVariant(DB_STORAGEMODE_ZIP)))->setEnabled(false);
    //    qobject_cast<QStandardItemModel *>(comboBoxDatabaseCopyMode->model())->item(
    //                comboBoxDatabaseCopyMode->findData(QVariant(DB_STORAGEMODE_COPY)))->setEnabled(false);
    //    qobject_cast<QStandardItemModel *>(comboBoxDatabaseCopyMode->model())->item(
    //                comboBoxDatabaseCopyMode->findData(QVariant(DB_STORAGEMODE_LINK)))->setEnabled(false);
    //    qobject_cast<QStandardItemModel *>(comboBoxDatabaseCopyMode->model())->item(
    //                comboBoxDatabaseCopyMode->findData(QVariant(DB_STORAGEMODE_SUMMARY)))->setEnabled(false);
    //  }

    // ALLOW TO CREATE GEXDB LINK ONLY ON Yield-Man OR IF NO YieldManDb
    // 2012/03/06
    // Allow to create GEXDB link ONLY with Yield-Man Administration Server
    // 2012/04/11
    // Allow to create GEXDB link ONLY with GEX-Pro without YieldManDb

    //  if (GS::LPPlugin::ProductInfo::getInstance()->isTDRAllowed() ||
    //      (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
    //      (GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())) ||
    //      (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
    //      (!GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())))
    {
        comboBoxDatabaseCopyMode->insertItem(iIndex++,      "Link to Corporate database (mySQL,...)",QVariant(DB_STORAGEMODE_CORP));
    }

    //  if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    //    comboBoxDatabaseCopyMode->insertItem(iIndex++,
    //                                         "No storage (Black Hole mode!)",
    //                                         QVariant(DB_STORAGEMODE_NONE));

    // If ExaminatorWeb, some fields are not available (password, etc...)
    //  if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() == GEX_DATATYPE_ALLOWED_DATABASEWEB)
    //  {
    //    TextLabelDatabaseLocation->hide();
    //    comboBoxDatabaseLocation->hide();
    //    TextLabelDatabaseCopyMode->hide();
    //    comboBoxDatabaseCopyMode->hide();
    //    groupBoxProtect->hide();
    //    TextLabelDatabaseDescription->move(10,34);
    //    LineEditDatabaseDescription->move(113,31);
    ////    pushButtonMore->hide();
    ////    resize(460,70);
    //  }

    if((GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()))
    {
        TextLabelDatabaseLocation->hide();
        comboBoxDatabaseLocation->hide();
    }

    TextLabelDatabaseDescription->hide();
    LineEditDatabaseDescription->hide();

    // Default: create database in Uncompressed mode.
    comboBoxDatabaseCopyMode->setCurrentIndex(comboBoxDatabaseCopyMode->findData(QVariant(DB_STORAGEMODE_CORP)));

    OnStorageType();

    // Update GUI
    UpdateGui();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CreateNewDatabaseDialog::~CreateNewDatabaseDialog()
{
    // Delete ressources
    m_pRemoteDatabase->UnloadPlugins(m_pPluginList);
    delete m_pRemoteDatabase; m_pRemoteDatabase=0;
}

///////////////////////////////////////////////////////////
// Write settings for currently selected GexDB plugin...
///////////////////////////////////////////////////////////
bool CreateNewDatabaseDialog::GexDbPlugin_WriteSettings(const QString & strSettingsFile)
{
    return m_pRemoteDatabase->WriteSettings(m_pCurrentPlugin, strSettingsFile);
}

///////////////////////////////////////////////////////////
// Get last error message from Remote database object...
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::GexDbPlugin_GetLastError(QString & strError)
{
    m_pRemoteDatabase->GetLastError(strError);
}

///////////////////////////////////////////////////////////
// User clicked button to configure the GexDB plugin...
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnButtonConfigurePlugin()
{
    // Check if plugin selected
    if(!m_pCurrentPlugin)
        return;

    // Configure plugin
    m_pRemoteDatabase->ConfigurePlugin(m_pCurrentPlugin);

    // Create a new ADR associated with this TDR
    if(m_pCurrentPlugin->m_pPlugin->MustHaveAdrLink())
    {
        QString strPluginName = m_pCurrentPlugin->pluginName();
        GexDbPlugin_Connector lAdrConnector("Adr Connector");
        // Get the default value according to the TDR name
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateAdrDatabaseConnector(lAdrConnector, m_pCurrentPlugin->m_pPlugin->m_pclDatabaseConnector);

        GexDatabaseEntry *pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strPluginName,
                                                                                                                lAdrConnector.m_strHost_Name,
                                                                                                                lAdrConnector.m_uiPort,
                                                                                                                lAdrConnector.m_strDriver,
                                                                                                                lAdrConnector.m_strSchemaName,
                                                                                                                lAdrConnector.m_strDatabaseName);

        bool bStatus = true;
        if(pDatabaseEntry == NULL)
        {
            // build XML file path to External database settings (login, mappinng tables...)
            GexRemoteDatabase lRemoteDatabase;
            QString strPluginFile = m_pCurrentPlugin->pluginFileName();
            if(!lRemoteDatabase.LoadPluginID(strPluginFile, 1, strPluginName))
            {
                QMessageBox::warning(this,"", "Corrupted file");
                return;
            }
            // Initialize and Check Remote connection
            m_pAdrPlugin = lRemoteDatabase.GetPluginID();
            if(!m_pAdrPlugin || !m_pAdrPlugin->m_pPlugin)
            {
                QMessageBox::warning(this,"", "Cannot create GexDbPlugin_ID");
                return;
            }

            m_pAdrPlugin->m_pPlugin->m_pclDatabaseConnector = new GexDbPlugin_Connector("Adr Connector");

            QMap<QString, QString>      lProductInfoMap;
            QMap<QString, QString>      lGuiOptionsMap;
            // Set  product info
            lProductInfoMap.insert("product_id",
                                   QString::number(GS::LPPlugin::ProductInfo::getInstance()->getProductID()));
            lProductInfoMap.insert("optional_modules",
                                   QString::number(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()));
            // Set GUI options
            lGuiOptionsMap.insert("open_mode","creation");
            lGuiOptionsMap.insert("read_only","yes");
            lGuiOptionsMap.insert("allowed_database_type",GEXDB_ADR_KEY);

            // Copy ADR configuration
            *m_pAdrPlugin->m_pPlugin->m_pclDatabaseConnector = lAdrConnector;
            bStatus = m_pAdrPlugin->m_pPlugin->ConfigWizard(lProductInfoMap,lGuiOptionsMap);
            // Create a new ADR associated with this TDR
            if(bStatus)
            {

                lAdrConnector = *m_pAdrPlugin->m_pPlugin->m_pclDatabaseConnector;
                // Create the new Database Entry for the ADR
                QString lError;
                bStatus = GS::Gex::Engine::GetInstance().GetDatabaseEngine().CreateDatabaseEntry(m_pAdrPlugin,
                                                                                                 lAdrConnector.m_strDatabaseName
                                                                                                    + "@"
                                                                                                    + lAdrConnector.m_strHost_Name,
                                                                                                 lError);

                // Clean new pointeur
                delete m_pAdrPlugin->m_pPlugin->m_pclDatabaseConnector;
                m_pAdrPlugin->m_pPlugin->m_pclDatabaseConnector = NULL;

            }
            if(bStatus)
                m_pCurrentPlugin->m_pPlugin->SetAdrLinkName(lAdrConnector.m_strDatabaseName
                                                            + "@"
                                                            + lAdrConnector.m_strHost_Name);
        }
    }
    // Update GUI
    UpdateGui();
}

///////////////////////////////////////////////////////////
// User changed database type...
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnDatabasePluginChanged()
{
    // Check if valid plugins loaded
    if(m_pPluginList.count() == 0)
        return;

    // Get selected plugin
    QString strSelected = comboPluginList->currentText();
    QList<GexDbPlugin_ID*>::iterator itPlugin;

    m_pCurrentPlugin = NULL;
    for(itPlugin = m_pPluginList.begin(); itPlugin != m_pPluginList.end(); itPlugin++)
    {
        if((*itPlugin)->pluginName() == strSelected)
        {
            m_pCurrentPlugin = (*itPlugin);
            break;
        }
    }

    // Update GUI
    UpdateGui();
}

///////////////////////////////////////////////////////////
// Update controls...
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::UpdateGui()
{
    // Update 'Create' button: enabled if non-empty Galaxy database name, and remote database connected
    UpdateCreateButton();

    // Hide/Show controls
    if(!m_pCurrentPlugin)
    {
        //    buttonSaveCorporateProfile->hide();
        //    buttonLoadCorporateProfile->hide();
        buttonConfigurePlugin->setEnabled(false);
    }
    else
    {
        buttonConfigurePlugin->setEnabled(true);
        //    if(m_pCurrentPlugin->isConfigured())
        //    {
        //      buttonSaveCorporateProfile->show();
        //      buttonLoadCorporateProfile->show();
        //    }
        //    else
        //    {
        //      buttonSaveCorporateProfile->hide();
        //      buttonLoadCorporateProfile->show();
        //    }
    }
}

///////////////////////////////////////////////////////////
// User clicked button create...
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnButtonCreate(void)
{

    QString lError;
    if(m_pCurrentPlugin)
    {
        if(!GS::Gex::Engine::GetInstance().GetDatabaseEngine().CreateDatabaseEntry(m_pCurrentPlugin, LineEditDatabaseName->text().trimmed(), lError))
        {
            QMessageBox::warning(this,"", lError);
        }
    }

    // If Remote database AND settings not exported, ask user if really don't want to save them!
    if((comboBoxDatabaseCopyMode->itemData(comboBoxDatabaseCopyMode->currentIndex()).toInt() == DB_STORAGEMODE_CORP)
            && m_strExportDatabaseSettings.isEmpty())
    {
        if( GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                &&  GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // Do not display this dialog
            // Database will be uploaded in ym_databases
        }
        else
        {
            // Give user one last chance to save settings to disk!
            bool lOk;
            GS::Gex::Message::request("Database Settings",
                                      "Do you want to export these connection settings to a XML file\n"
                                      "for faster setup of other computers ?", lOk);
            if (lOk)
            {
                OnExportCorporateDatabase();	// Save settnings to disk.
            }

        }
    }

    // Exit the dialog box with ACCEPT state
    accept();
}

///////////////////////////////////////////////////////////
// check Database link name
// return ok if valid
///////////////////////////////////////////////////////////
bool CreateNewDatabaseDialog::IsDbLinkNameValid(QString& strMessageToDisplay)
{
    strMessageToDisplay.clear();
    QString strDbLinkName = LineEditDatabaseName->text().trimmed();

    if(strDbLinkName.isEmpty())
    {
        strMessageToDisplay = QString("Please enter a database link name.")        ;
        return false;
    }

    if(GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDbLinkName, false))
    {
        strMessageToDisplay = QString("Please change database link name. This one is already used");
        return false;
    }

    // every checks are ok
    return true;
}


///////////////////////////////////////////////////////////
// Update state of 'Create' button...
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::UpdateCreateButton(void)
{
    // Check database name
    QString strMsgToDisplay;
    if(!IsDbLinkNameValid(strMsgToDisplay))
    {
        QPalette qpLineEditPalette = LineEditDatabaseName->palette();
        qpLineEditPalette.setColor(QPalette::Text, Qt::red);
        qpLineEditPalette.setColor(QPalette::ToolTipText, Qt::red);
        LineEditDatabaseName->setToolTip(strMsgToDisplay);
        LineEditDatabaseName->setPalette(qpLineEditPalette);

        // Invalid Name: disable button
        PushButtonCreate->setEnabled(false);
        return;
    }
    else
    {
        LineEditDatabaseName->setToolTip(strMsgToDisplay);
        LineEditDatabaseName->setPalette(palette());

        // valid name: enable button
        PushButtonCreate->setEnabled(true);
    }

    // If not corporate database enable button
    if(comboBoxDatabaseCopyMode->itemData(comboBoxDatabaseCopyMode->currentIndex()).toInt() != DB_STORAGEMODE_CORP)
    {
        // Enable button
        PushButtonCreate->setEnabled(true);
        return;
    }

    // If corporate database selected, check if plugin configuration is OK
    if(!m_pCurrentPlugin)
    {
        // Empty Name: disable button
        PushButtonCreate->setEnabled(false);
        return;
    }
    PushButtonCreate->setEnabled(m_pCurrentPlugin->isConfigured());
}

///////////////////////////////////////////////////////////
// User enables a password on 'Importing files to database'
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnPasswordImportFiles(void)
{
    if(checkBoxPasswordImportFile->isChecked() == true)
    {
        // Password activated: enable edit field + set focus on it.
        LineEditImportPassword->setEnabled(true);
        LineEditImportPassword->setFocus();
    }
    else
        LineEditImportPassword->setEnabled(false);
}

///////////////////////////////////////////////////////////
// User enables a password on 'Deleting a database'
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnPasswordDeleteDatabase(void)
{

    if(checkBoxDeleteDatabase->isChecked() == true)
    {
        // Password activated: enable edit field + set focus on it.
        LineEditDeletePassword->setEnabled(true);
        LineEditDeletePassword->setFocus();
    }
    else
        LineEditDeletePassword->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Database storage type selected. Update GUI accordingly.
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnStorageType(void)
{
    // Corporate database selected??
    bool bEnableCorporateTab = (comboBoxDatabaseCopyMode->itemData(comboBoxDatabaseCopyMode->currentIndex()).toInt() == DB_STORAGEMODE_CORP);

    // Corporate database selected
    if(bEnableCorporateTab)
    {
        stackedWidget->show();
        // Force 'More >>' Gui, and focus on 'Corporate database'
        m_bExpanded = false;

        // Select 'Corporate' Tab.
        stackedWidget->setCurrentIndex(PAGE_COORP_DATABASE);

        // Refresh GUI.
        OnMore();
    }
    else
    {
        stackedWidget->setCurrentIndex(PAGE_PROTECTION);
        if (comboBoxDatabaseCopyMode->itemData(comboBoxDatabaseCopyMode->
                                               currentIndex()).toInt() == DB_STORAGEMODE_NONE)
            stackedWidget->hide();
        else
            stackedWidget->show();
    }


    // Update state of 'Create' button
    UpdateCreateButton();

    //adjustSize();
}

///////////////////////////////////////////////////////////
// 'More' or 'Less' button to expand/reduce the list of fields
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnMore(void)
{
    if(m_bExpanded == true)
    {
        // Hide tab widget with details
        m_bExpanded = false;
        stackedWidget->hide();
        //    spacerDetails->changeSize(16, 16, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        adjustSize();

        // Change button label
        //    pushButtonMore->setText("More >>");
    }
    else
    {
        // Show tab widget with details
        m_bExpanded = true;
        stackedWidget->show();
        //    spacerDetails->changeSize(16, 16, QSizePolicy::Fixed, QSizePolicy::Fixed);
        //    adjustSize();

        // Change button label
        //    pushButtonMore->setText("Less <<");
    }
}

///////////////////////////////////////////////////////////
// Export 'Corporate Database' settings to XML settings file
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnExportCorporateDatabase(void)
{
    // Check if plugin selected
    if(!m_pCurrentPlugin)
        return;

    // Construct initial filename = <User home>/gexdb_external.txt
    CGexSystemUtils::GetUserHomeDirectory(m_strExportDatabaseSettings);
    m_strExportDatabaseSettings += "/" + QString(DB_INI_FILE);
    CGexSystemUtils::NormalizePath(m_strExportDatabaseSettings);

    // Ask user to chose a destination file
    m_strExportDatabaseSettings = QFileDialog::getSaveFileName(
                this,
                "Choose a filename to save under",
                m_strExportDatabaseSettings,
                "Corporate Database settings file (*.xml)"
                );

    if(m_strExportDatabaseSettings.isEmpty())
        return;

    GexDatabaseEntry databaseEntry(0);
    if (m_pCurrentPlugin->m_pPlugin &&
            m_pCurrentPlugin->m_pPlugin->m_pclDatabaseConnector)
    {
        if(LineEditDatabaseName->text().trimmed().isEmpty())
            databaseEntry.SetLogicalName( m_pCurrentPlugin->m_pPlugin->
                                          m_pclDatabaseConnector->m_strSchemaName);
        else
            databaseEntry.SetLogicalName(LineEditDatabaseName->text().trimmed());
    }

    databaseEntry.SetExternalDbPluginIDPTR(m_pCurrentPlugin);
    databaseEntry.SaveToXmlFile(m_strExportDatabaseSettings);
    // To avoid m_pCurrentPlugin to be deleted
    databaseEntry.SetExternalDbPluginIDPTR(NULL);
}

///////////////////////////////////////////////////////////
// Import 'Corporate Database' settings from XML settings file
///////////////////////////////////////////////////////////
void CreateNewDatabaseDialog::OnImportCorporateDatabase(void)
{
    // Construct initial filename = <User home>
    CGexSystemUtils::GetUserHomeDirectory(m_strExportDatabaseSettings);

    // Let the user select teh settings file
    m_strExportDatabaseSettings = QFileDialog::getOpenFileName(
                this,
                "Select Corporate Database settings file",
                m_strExportDatabaseSettings,
                "Remote Database details (*.xml)");

    // No file selected / cancel hit.
    if(m_strExportDatabaseSettings.isEmpty() == true)
        return;

    // Load selected settings file
    GexDatabaseEntry databaseEntry(0);
    if (databaseEntry.LoadFromXmlFile(m_strExportDatabaseSettings))
    {
        QMap<QString, QString>      lProductInfoMap;
        QMap<QString, QString>      lGuiOptionsMap;
        // Set  product info
        lProductInfoMap.insert("product_id",
                               QString::number(GS::LPPlugin::ProductInfo::getInstance()->getProductID()));
        lProductInfoMap.insert("optional_modules",
                               QString::number(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()));
        // Set GUI options
        lGuiOptionsMap.insert("open_mode","creation");
        lGuiOptionsMap.insert("read_only","no");
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            lGuiOptionsMap.insert("allowed_database_type",GEXDB_YM_PROD_TDR_KEY);
        else
            lGuiOptionsMap.insert("allowed_database_type",QString(GEXDB_CHAR_TDR_KEY)+","+QString(GEXDB_MAN_PROD_TDR_KEY));


        m_pCurrentPlugin = databaseEntry.GetPluginID();

        m_pCurrentPlugin->setConfigured(m_pCurrentPlugin->m_pPlugin->
                                        ConfigWizard(lProductInfoMap,
                                                     lGuiOptionsMap));
        databaseEntry.m_pExternalDatabase->SetPluginIDPTR(NULL);
    }

    //  m_pCurrentPlugin = m_pRemoteDatabase->LoadSettings(m_pPluginList, m_strExportDatabaseSettings);
    if(!m_pCurrentPlugin)
    {
        // Error reading Database settings file: get error message & display it.
        QString strPluginMessage;
        QString strErrorMessage;
        m_pRemoteDatabase->GetLastError(strPluginMessage);
        strErrorMessage = "*Error* Failed loading settings from file :";
        strErrorMessage += m_strExportDatabaseSettings;
        strErrorMessage += "\n";
        strErrorMessage += strPluginMessage;

        GS::Gex::Message::information("", strErrorMessage);

        // Set current plugin ptr to selected plugin
        OnDatabasePluginChanged();
        return;
    }

    // Select loaded plugin
    LineEditDatabaseName->setText(databaseEntry.LogicalName());
    comboPluginList->setCurrentIndex(comboPluginList->findText(m_pCurrentPlugin->pluginName()));
    //  m_pCurrentPlugin->setConfigured(true);
    //  OnDatabasePluginChanged();
}
