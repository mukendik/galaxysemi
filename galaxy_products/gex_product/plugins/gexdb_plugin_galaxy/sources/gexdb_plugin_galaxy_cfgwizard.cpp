#include "gexdb_plugin_galaxy_cfgwizard.h"
#include "gexdb_plugin_galaxy.h"
#include "gex_shared.h"
#include <gqtl_log.h>

#include <QMessageBox>
#include <QPushButton>
#include <QSqlRecord>
#include <QSqlError>
#include <QDir>
#include <QCryptographicHash>

///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
GexDbPlugin_Galaxy_CfgWizard::GexDbPlugin_Galaxy_CfgWizard(
        const QString & strHostName,
        const QString & strApplicationPath,
        const QString & strUserProfile,
        const bool bCustomerDebugMode,
        const QMap<QString,QString> &guiOptions,
        GexDbPlugin_Galaxy *pclDbPluginGalaxy,
        CGexSkin * pGexSkin,
        QWidget* parent )
    : QWizard(parent,Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint), m_pclCopyConnector(NULL),
      m_pclDbPluginGalaxy(pclDbPluginGalaxy),
      m_strApplicationPath(strApplicationPath),
      m_strUserProfile(strUserProfile), m_pGexSkin(pGexSkin)
{
    setupUi(this);

    // reset all attributes to default values
    Clear();

    //////////////////////////////////////
    // init
    // Set Examinator skin
    if (m_pGexSkin)
        m_pGexSkin->applyPalette(this);

    // attributes
    m_strHostName = strHostName;
    mGuiOptions = guiOptions;
    QString strPluginName;
    if (pclDbPluginGalaxy)
        pclDbPluginGalaxy->GetPluginName(strPluginName);
    m_pclCopyConnector = new GexDbPlugin_Connector(strPluginName+"_ConfigWizard",
                                                 pclDbPluginGalaxy);

    m_bCustomerDebugMode = bCustomerDebugMode;
    // init
    //////////////////////////////////////
    InitComboTdrDbType();
    UpdateGuiFromConnector();

    // Signal/Slot connections
    Connect();
}

/////////////////////////////////////////////////////////////////
//  Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy_CfgWizard::~GexDbPlugin_Galaxy_CfgWizard()
{
    Clear();
}

///////////////////////////////////////////////////////////
// Used to manage QWizard navigation
///////////////////////////////////////////////////////////
int GexDbPlugin_Galaxy_CfgWizard::nextId() const
{
    switch (currentId())
    {
    case GexDbPlugin_Galaxy_CfgWizard::eDbSettingsPage:

        UpdateConnectorFromGui(*m_pclCopyConnector);

        if(mUi_qcbDbCreationCheckBox->isChecked())
            return GexDbPlugin_Galaxy_CfgWizard::eDbCreationPage;
        else
            return GexDbPlugin_Galaxy_CfgWizard::eDbConnectionPage;
    case GexDbPlugin_Galaxy_CfgWizard::eDbCreationPage:
        return GexDbPlugin_Galaxy_CfgWizard::eDbConnectionPage;
    case GexDbPlugin_Galaxy_CfgWizard::eDbConnectionPage:
    default:
        return GexDbPlugin_Galaxy_CfgWizard::eDbSettingsPage;
    }
}

///////////////////////////////////////////////////////////
// User clicked the 'Back' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::back()
{
    restart();
}

///////////////////////////////////////////////////////////
// Set controls from data
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::Set(const GexDbPlugin_Connector & clDbConnector,
                                       bool bAutomaticStartup)
{
    if(!m_pclCopyConnector)
        m_pclCopyConnector = new GexDbPlugin_Connector(clDbConnector);
    else
    {
        delete m_pclCopyConnector;
        m_pclCopyConnector = new GexDbPlugin_Connector(clDbConnector);
    }
    m_pclCopyConnector->m_strPluginName += "_ConfigWizard";

    UpdateGuiFromConnector();
    mUi_qcbStartupType->setCurrentIndex(bAutomaticStartup?0:1);
}

///////////////////////////////////////////////////////////
// Get data from controls
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::Get(GexDbPlugin_Connector & clDbConnector,
                                       bool *pbAutomaticStartup)
{
    if(pbAutomaticStartup == NULL)
        return;

    // Save data
    if (mUi_qcbStartupType)
        *pbAutomaticStartup = (mUi_qcbStartupType->currentIndex() == 0);
    if (m_pclCopyConnector)
        m_pclCopyConnector->Disconnect();
    clDbConnector = *m_pclCopyConnector;
}

//////////////////////////////////////////////////////////////////
// accessors
//////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy_CfgWizard::SetDataBaseType(const GexDbPlugin_Galaxy::DataBaseType dbtEnuDataBaseType)
{
    m_eDataBaseType = dbtEnuDataBaseType;

    // everything went well !
    return true;
}

GexDbPlugin_Galaxy::DataBaseType GexDbPlugin_Galaxy_CfgWizard::GetDataBaseType() const
{
    return m_eDataBaseType;
}

bool GexDbPlugin_Galaxy_CfgWizard::IsReadOnly()
{
    QString lFieldName = "read_only";

    if (mGuiOptions.contains(lFieldName))
    {
        if (mGuiOptions.value(lFieldName) == "yes")
            return true;
        else if (mGuiOptions.value(lFieldName) == "no")
            return false;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Unable to use option %1 value: %2").
                  arg(lFieldName).arg(mGuiOptions.value(lFieldName)).
                  toLatin1().constData());
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_ERROR, QString("Missing GUI option %1").
          arg(lFieldName).toLatin1().constData());
    return false;
}

bool GexDbPlugin_Galaxy_CfgWizard::IsOpenModeCreation()
{
    QString lFieldName = "open_mode";

    if (mGuiOptions.contains(lFieldName))
    {
        if (mGuiOptions.value(lFieldName) == "creation")
            return true;
        else if (mGuiOptions.value(lFieldName) == "edition")
            return false;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                   QString("Unable to use option %1 value: %2").
                   arg(lFieldName).arg(mGuiOptions.value(lFieldName)).
                   toLatin1().constData());
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_ERROR, QString("Missing GUI option %1").arg(lFieldName).
          toLatin1().constData());
    return false;
}

bool GexDbPlugin_Galaxy_CfgWizard::IsLinkCreationAllowed()
{
    if (!IsOpenModeCreation())
        return false;

    return true;
}


///////////////////////////////////////////////////////////
// User changed database type...
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::OnDatabaseDriverChanged()
{
    if (!m_pclCopyConnector)
    {
        GSLOG(SYSLOG_SEV_ERROR, "m_pclDbConnector NULL");
        return;
    }
    if(mUi_qcbDbDriver->count()<=0)
        return;     // mUi_qcbDbDriver cleared by update methods

    QString strDatabaseDriver = m_pclCopyConnector->GetDriverName(mUi_qcbDbDriver->currentText());
    if(strDatabaseDriver.isEmpty())
        return;     // during initialization, mUi_qcbDbDriver not always set

    GSLOG(SYSLOG_SEV_DEBUG,
           QString("GexDbPlugin_Galaxy_CfgWizard::OnDatabaseTypeChanged: Plug-in config wizard: database driver changed (%1), update GUI...")
           .arg(strDatabaseDriver).toLatin1().data() );

    if(m_pclCopyConnector->IsMySqlDB(strDatabaseDriver))
    {
        labelDbName->setText("Database name:");
        mUi_qsbDbPortNb->setValue(3306);

        mUi_qleDbName->setEnabled(false);

        mUi_qleDbName->setText(mUi_qleAdminName->text());

        mUi_qlDbRootNameLabel->setText("Root name");
        mUi_qlDbRootPasswordLabel->setText("Root password");
        mUi_qleDbRootName->setText("root");
        mUi_qleDbRootPassword->clear();
        if(m_eDataBaseType != GexDbPlugin_Galaxy::eYmAdminDb)
            mUi_qcbDbCreationCheckBox->setChecked(false);
        else
            labelDbPassword_Admin->setText("MySql Account password");
    }
    else
    {
        QString strErrMsg("Undefined database driver !");
        GSLOG(SYSLOG_SEV_ALERT, strErrMsg.toLatin1().constData());
        QMessageBox::warning(this, QString("Warning !"), strErrMsg);
        reject();
    }
}


void GexDbPlugin_Galaxy_CfgWizard::OnDbCreationChanged()
{
    bool lCreateDbRequested = mUi_qcbDbCreationCheckBox->isChecked();
    bool lIsDbCreationAllowed = IsLinkCreationAllowed() &&
            (m_pclDbPluginGalaxy->IsMonitoringMode() ||
             m_pclDbPluginGalaxy->IsTdrAllowed());

    if (lCreateDbRequested && !lIsDbCreationAllowed)
    {
        GEX_ASSERT(false);      /// TODO GSLOG
        mUi_qcbDbCreationCheckBox->setChecked(false);
        return;
    }

    mUi_qlTdrDbTypeLabel->setVisible(lCreateDbRequested);
    mUi_qcbTdrDbType->setVisible(lCreateDbRequested);
    mUi_qleDbRootName->setVisible(lCreateDbRequested);
    mUi_qleDbRootPassword->setVisible(lCreateDbRequested);
    mUi_qlDbRootNameLabel->setVisible(lCreateDbRequested);
    mUi_qlDbRootPasswordLabel->setVisible(lCreateDbRequested);

    InitComboTdrDbType();

    if(lCreateDbRequested)
        button(QWizard::NextButton)->setText("Create >");
    else
        button(QWizard::NextButton)->setText("Connect >");
}

void GexDbPlugin_Galaxy_CfgWizard::OnAdminNameChanged(QString strNewAdminName)
{
    if(!m_pclCopyConnector)
        return;

    QString strDriverName = m_pclCopyConnector->GetDriverName(mUi_qcbDbDriver->currentText());
    if(m_pclCopyConnector->IsMySqlDB(strDriverName))
    {
        mUi_qleDbName->setText(strNewAdminName);
        m_pclCopyConnector->m_strDatabaseName = m_pclCopyConnector->m_strSchemaName = strNewAdminName;
    }
    m_pclCopyConnector->m_strUserName_Admin = strNewAdminName;
}

void GexDbPlugin_Galaxy_CfgWizard::OnSettingsChange()
{
    if(!IsLinkCreationAllowed())       // connect only
        button(QWizard::FinishButton)->setEnabled(true);
}

void GexDbPlugin_Galaxy_CfgWizard::OnSettingsChange(QString strString)
{
    Q_UNUSED(strString);
    OnSettingsChange();
}


///////////////////////////////////////////////////////////
// Update GUI: enable/disable controls...
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::UpdateGui(const int nPageToUpdateId /*=-1*/)
{
    //////////////////////////////////////////////////
    // validity checks
    if(nPageToUpdateId<0)
    {
        restart();
        return;
    }

    if(nPageToUpdateId!=currentId())
        GEX_ASSERT(false);

    //////////////////////////////////////////////////
    // generic update

    // nothing to do

    //////////////////////////////////////////////////
    // specific update
    switch (nPageToUpdateId)
    {
    case GexDbPlugin_Galaxy_CfgWizard::eDbSettingsPage:
        UpdateSettingPage();
        return;
    case GexDbPlugin_Galaxy_CfgWizard::eDbCreationPage:
        ClearLogs();
        UpdateCreateDbPage();
        return;
    case GexDbPlugin_Galaxy_CfgWizard::eDbConnectionPage:
        ClearLogs();
        UpdateConnectDbPage();
        return;
    default:
        GSLOG(SYSLOG_SEV_ALERT, "Database configuration gui not correctly updated !");
        GEX_ASSERT(false);
        return;
    }
}


///////////////////////////////////////////////////////////
// Create GexDb database
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy_CfgWizard::GenericCreateDatabase()
{
    if (!mUi_qcbDbCreationCheckBox)
        return false;

    // Add some logs
    if (!m_pclDbPluginGalaxy)
        return false;

    // retrieve from GUI TDR DB TYPE
    if (mUi_qcbTdrDbType->itemData(mUi_qcbTdrDbType->currentIndex()).toString() == QString(GEXDB_CHAR_TDR_KEY)) // Charac
        SetDataBaseType(GexDbPlugin_Galaxy::eCharacTdrDb);
    else if (mUi_qcbTdrDbType->itemData(mUi_qcbTdrDbType->currentIndex()).toString() == QString(GEXDB_MAN_PROD_TDR_KEY)) // Prod
        SetDataBaseType(GexDbPlugin_Galaxy::eManualProdDb);
    else if (mUi_qcbTdrDbType->itemData(mUi_qcbTdrDbType->currentIndex()).toString() == QString(GEXDB_YM_PROD_TDR_KEY)) // YM
        SetDataBaseType(GexDbPlugin_Galaxy::eProdTdrDb);
    else if (mUi_qcbTdrDbType->itemData(mUi_qcbTdrDbType->currentIndex()).toString() == QString(GEXDB_ADR_KEY)) // ADR
        SetDataBaseType(GexDbPlugin_Galaxy::eAdrDb);
    else if (mUi_qcbTdrDbType->itemData(mUi_qcbTdrDbType->currentIndex()).toString() == QString(GEXDB_ADR_LOCAL_KEY)) // local ADR
        SetDataBaseType(GexDbPlugin_Galaxy::eAdrLocalDb);
    else if (mUi_qcbTdrDbType->itemData(mUi_qcbTdrDbType->currentIndex()).toString() == QString(GEXDB_YM_PROD_TDR_KEY)) // local ADR
        SetDataBaseType(GexDbPlugin_Galaxy::eYmAdminDb);

    QStringList lListUninstall;

    m_pclCopyConnector->m_strDatabaseName = mUi_qleDbName->text();
    m_pclCopyConnector->m_strUserName = mUi_qleUserName->text();
    m_pclCopyConnector->m_strPassword = mUi_qleUserPassword->text();
    m_pclCopyConnector->m_strUserName_Admin = mUi_qleAdminName->text();
    m_pclCopyConnector->m_strPassword_Admin = mUi_qleAdminPassword->text();
    m_pclCopyConnector->m_strHost_IP = mUi_qleDbHostNameIP->text();
    m_pclCopyConnector->m_uiPort = mUi_qsbDbPortNb->value();
    m_pclCopyConnector->m_strDriver = m_pclCopyConnector->GetDriverName(mUi_qcbDbDriver->currentText());


    QString strPluginName;
    m_pclDbPluginGalaxy->GetPluginName(strPluginName);
    // List of script install
    if (!m_pclDbPluginGalaxy->CreateDatabase(*m_pclCopyConnector, m_eDataBaseType,
                                             mUi_qleDbRootName->text(), mUi_qleDbRootPassword->text(),
                                             lListUninstall, progressBar, true))
    {
        button(QWizard::BackButton)->setEnabled(true);
        return false;
    }

    // Put the YM tdr DB in the database type
    // SetDataBaseType(GexDbPlugin_Galaxy::eProdTdrDb);
    mUi_qcbDbCreationCheckBox->setChecked(false);
    button(QWizard::BackButton)->setEnabled(true);
    return true;
}


///////////////////////////////////////////////////////////
// Connect GexDb database
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy_CfgWizard::GenericConnectDataBase()
{
    // Update connector from nextId

    // Disconnect if connected to DB
    if(m_pclCopyConnector->IsConnected())
        m_pclCopyConnector->Disconnect();

    InsertIntoUpdateLog("<b>** "+m_pclCopyConnector->m_strSchemaName.toUpper()+" DATABASE CONNECTION **</b>");
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog("<b>o "+labelDbHostNameIP->text()+"</b> " + m_pclCopyConnector->m_strHost_Name);
    InsertIntoUpdateLog("<b>o "+labelDbPortNb->text()+"</b> " + QString::number(m_pclCopyConnector->m_uiPort));
    InsertIntoUpdateLog("<b>o "+labelDbDriver->text()+"</b> " + m_pclCopyConnector->m_strDriver);
    InsertIntoUpdateLog("<b>o "+labelDbName->text()+"</b> "+m_pclCopyConnector->m_strDatabaseName);
    InsertIntoUpdateLog("<b>o "+labelDbName_Admin->text()+"</b> " + m_pclCopyConnector->m_strUserName_Admin);
    if(!labelDbUserName->isHidden())
        InsertIntoUpdateLog("<b>o "+labelDbUserName->text()+"</b> " + m_pclCopyConnector->m_strUserName);
    InsertIntoUpdateLog(" ");

    // Resolve HostName
    bool bIsLocalHost;
    m_pclCopyConnector->ResolveHostName(
                    m_pclCopyConnector->m_strHost_Unresolved,
                    m_pclCopyConnector->m_strHost_Name,
                    m_pclCopyConnector->m_strHost_IP,
                    &bIsLocalHost);
    if(m_pclCopyConnector->m_strHost_Name.isEmpty()
            && m_pclCopyConnector->m_strHost_IP.isEmpty())
    {
        // unable to resolve HostName
        QString strErrMsg;
        strErrMsg = "Cannot resolve HostName/IP '%1'";
        InsertIntoUpdateLog(strErrMsg.arg(m_pclCopyConnector->m_strHost_Unresolved),true);
        return false;
    }

    // Connect to DB (user)
    if(m_eDataBaseType!=GexDbPlugin_Galaxy::eYmAdminDb)
    {
        m_pclCopyConnector->SetAdminLogin(false);
        if(!m_pclCopyConnector->Connect())
        {
            QString strErrMsg;
            m_pclCopyConnector->GetLastError(strErrMsg);
            InsertIntoUpdateLog(strErrMsg,true);

            return false;
        }
    }

    // Connect to DB (admin user)
    m_pclCopyConnector->SetAdminLogin(true);
    if(!m_pclCopyConnector->Connect())
    {
        QString strErrMsg;
        m_pclCopyConnector->GetLastError(strErrMsg);
        InsertIntoUpdateLog(strErrMsg,true);

        return false;
    }

    // Connect to DB (standard user)
    if(m_eDataBaseType == GexDbPlugin_Galaxy::eProdTdrDb)
    {
        if(m_pclDbPluginGalaxy->IsMonitoringMode())
        {
            m_pclCopyConnector->Disconnect();
            // Check if the DB is a eYmTdrDb
            bool bIsYmTdrDb = CheckDbTdrType(QString(GEXDB_YM_PROD_TDR_KEY));
            // Then check if it is allowed
            if(!bIsYmTdrDb)
            {
                QString strErrMsg;
                strErrMsg = "<b>o INVALID TDR TYPE</b>: ";
                for(int DtrType=0; DtrType<mUi_qcbTdrDbType->count(); DtrType++)
                {
                    if(DtrType > 0)
                        strErrMsg += " or ";
                    strErrMsg += "'"+mUi_qcbTdrDbType->itemText(DtrType)+"'";
                }
                strErrMsg += " requested";
                InsertIntoUpdateLog(strErrMsg,true);

                return false;
            }
            button(QWizard::FinishButton)->setText("Next >");
        }

        QString Msg = "<b>o TDR TYPE</b>: ";
        if(CheckDbTdrType(QString(GEXDB_YM_PROD_TDR_KEY)))
            Msg += QString(GEXDB_YM_PROD_TDR_NAME);
        if(CheckDbTdrType(QString(GEXDB_CHAR_TDR_KEY)))
            Msg += QString(GEXDB_CHAR_TDR_NAME);
        if(CheckDbTdrType(QString(GEXDB_MAN_PROD_TDR_KEY)))
            Msg += QString(GEXDB_MAN_PROD_TDR_NAME);
        if(CheckDbTdrType(QString(GEXDB_ADR_KEY)))
            Msg += QString(GEXDB_ADR_NAME);
        if(CheckDbTdrType(QString(GEXDB_ADR_LOCAL_KEY)))
            Msg += QString(GEXDB_ADR_LOCAL_NAME);
        InsertIntoUpdateLog(Msg);
    }

    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog("DATABASE CONNECTION: <b>Success</b>");
    InsertIntoUpdateLog(" ");
    InsertIntoUpdateLog(" ");

    // everything went well!
    return true;
}


bool GexDbPlugin_Galaxy_CfgWizard::CheckDbTdrType(QString dbType)
{
    QString lTdrDbType, lQuery, lErrorMsg;
    if(!m_pclCopyConnector)
        return false;

    QSqlQuery	sqlQuery = QSqlQuery(QSqlDatabase::database(m_pclCopyConnector->m_strConnectionName));

    QString strTable = mUi_qleAdminName->text() + ".global_info";

    // Check if db_type column exists
    QSqlRecord  clRecords = QSqlDatabase::database(m_pclCopyConnector->m_strConnectionName).record(strTable);
    if(!clRecords.contains("db_type"))
    {
        // Old build
        // default value is YM Prod
        return QString(GEXDB_YM_PROD_TDR_KEY)==dbType;
    }

    lQuery = "SELECT db_type FROM " + mUi_qleAdminName->text() + ".global_info";
    if(!sqlQuery.exec(lQuery))
    {
        lErrorMsg = "Error executing SQL query.\n";
        lErrorMsg += "QUERY=" + lQuery + "\n";
        lErrorMsg += "ERROR=" + sqlQuery.lastError().text();
        InsertIntoUpdateLog(lErrorMsg,true);
        return false;
    }

    lTdrDbType = m_pclCopyConnector->m_strSchemaName;
    lTdrDbType += dbType;
    QByteArray lHash = QCryptographicHash::hash(lTdrDbType.toLatin1(),QCryptographicHash::Md5);

    sqlQuery.first();
    if(sqlQuery.value(0).toString().isEmpty()
            || sqlQuery.value(0).toString() == QString(lHash.toHex()))
        return true;

    return false;
}

///////////////////////////////////////////////////////////
// Insert into update log
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::InsertIntoUpdateLog(const QString & strMessage, bool bRed)
{
    QString strColorMessage = strMessage;
    if(bRed)
        strColorMessage = QString("<FONT COLOR=\"red\">%1</FONT>").arg(strMessage);

    QString strLines; // = mUi_qlDbStatus->text();
    if(!strLines.isEmpty())
        strLines += "\n";
    strLines += strColorMessage;
    strLines = strLines.replace( QRegExp("<[^>]*>"), "" );
    if(strLines.count("\n") > 4)
        strLines = strLines.section("\n",strLines.count("\n")-4);

    mUi_qteConnectDbLog->append(strColorMessage);

    if(m_pclDbPluginGalaxy)
    {
        strLines = strColorMessage;
        strLines = strLines.replace("\n","\n<BR>",Qt::CaseInsensitive);
        m_pclDbPluginGalaxy->InsertIntoUpdateLog(strLines);
    }
    else
        mUi_qteCreateDbLog->append(strColorMessage);

    mUi_qteCreateDbLog->moveCursor(QTextCursor::End);
    mUi_qteConnectDbLog->moveCursor(QTextCursor::End);

    QCoreApplication::processEvents();
}

void GexDbPlugin_Galaxy_CfgWizard::UpdateLogMessage(const QString &message, bool isPlainText)
{
    QString lText = message;

    if(isPlainText)
        mUi_qteCreateDbLog->insertPlainText(lText);
    else
        mUi_qteCreateDbLog->insertHtml(lText);

    mUi_qteCreateDbLog->moveCursor(QTextCursor::End);
    mUi_qteCreateDbLog->ensureCursorVisible();

    QCoreApplication::processEvents();
}

void GexDbPlugin_Galaxy_CfgWizard::UpdateProgress(int prog)
{
    if(progressBar)
        progressBar->setValue(prog);
}

void GexDbPlugin_Galaxy_CfgWizard::ResetProgress(bool forceCompleted)
{
    int lProgress;
    if(progressBar)
    {
        if(forceCompleted == true)
            lProgress = 100;
        else
        {
            lProgress = 0;
            progressBar->setMaximum(100);
            progressBar->setTextVisible(true);
            progressBar->show();
        }
        progressBar->setValue(lProgress);
    }
}

void GexDbPlugin_Galaxy_CfgWizard::SetMaxProgress(int max)
{
    if(progressBar)
        progressBar->setMaximum(max);
}

///////////////////////////////////////////////////////////
// For debug purpose, write message to debug trace file
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::WriteDebugMessageFile(const QString & strMessage)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.left(65280).toLatin1().data());
}


///////////////////////////////////////////////////////////////
// used to set plugin connector parameters from gui
// return false if the connector isn't correctly set.
///////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy_CfgWizard::UpdateConnectorFromGui(GexDbPlugin_Connector& gdpcRefConnector) const
{
    gdpcRefConnector.m_strHost_Unresolved = mUi_qleDbHostNameIP->text();
    gdpcRefConnector.m_strHost_Name = mUi_qleDbHostNameIP->text();
    gdpcRefConnector.m_strHost_IP = mUi_qleDbHostNameIP->text();
    gdpcRefConnector.m_uiPort = mUi_qsbDbPortNb->value();
    gdpcRefConnector.m_strDriver = gdpcRefConnector.GetDriverName(mUi_qcbDbDriver->currentText());
    gdpcRefConnector.m_strDatabaseName = mUi_qleDbName->text();
    gdpcRefConnector.m_strSchemaName = mUi_qleAdminName->text();
    gdpcRefConnector.m_strUserName = mUi_qleUserName->text();
    gdpcRefConnector.m_strPassword = mUi_qleUserPassword->text();
    gdpcRefConnector.m_strUserName_Admin = mUi_qleAdminName->text();
    gdpcRefConnector.m_strPassword_Admin = mUi_qleAdminPassword->text();

    return true;
}


///////////////////////////////////////////////////////////////
// used to set plugin connector parameters from gui
// return false if the connector isn't correctly set.
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// used to update gui from current connector
///////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy_CfgWizard::UpdateGuiFromConnector()
{
    if(!m_pclCopyConnector)
        return false;

    // Set values of GUI controls: Database connection settings page
    GSLOG(SYSLOG_SEV_INFORMATIONAL, " Plug-in config wizard: initializing GUI...");

    if(m_pclCopyConnector->EnumDrivers(m_strlDrivers, m_strlDrivers_FriendlyNames) && (m_strlDrivers.count() > 0))
    {
        if (m_strlDrivers_FriendlyNames.empty())
        {
            InsertIntoUpdateLog("No SQL Database drivers detected (DB Type). Please contact Quantix support at "+QString(GEX_EMAIL_SUPPORT),true);
            return false;
        }

        GSLOG(SYSLOG_SEV_DEBUG, QString("Plug-in config wizard: updating GUI with list of %1 retrieved drivers...")
               .arg(m_strlDrivers_FriendlyNames.count()).toLatin1().data() );

        QStringList qslDbTypeComboBoxList;
        for(int ii=0; ii<mUi_qcbDbDriver->count(); ii++)
            qslDbTypeComboBoxList << mUi_qcbDbDriver->itemText(ii);

        qslDbTypeComboBoxList.sort();
        m_strlDrivers_FriendlyNames.sort();

        if(m_strlDrivers_FriendlyNames != qslDbTypeComboBoxList)
        {
            mUi_qcbDbDriver->clear();
            mUi_qcbDbDriver->addItems( m_strlDrivers_FriendlyNames );
            // If first driver is SQLite
            // select another
            if( ! m_strlDrivers_FriendlyNames.empty()
                &&(m_strlDrivers_FriendlyNames.first().indexOf("SQLITE") > 0)
                && (m_strlDrivers_FriendlyNames.count() > 1))
                mUi_qcbDbDriver->setCurrentIndex(1);
            else
                mUi_qcbDbDriver->setCurrentIndex(0); //Qt3 : mUi_qcbDbType->setCurrentItem(0);

            if (!m_pclCopyConnector->m_strDriver.isEmpty())
                if(!m_strlDrivers_FriendlyNames.contains(m_pclCopyConnector->GetFriendlyName(m_pclCopyConnector->m_strDriver)))
                {
                    mUi_qcbDbDriver->addItem(m_pclCopyConnector->GetFriendlyName(m_pclCopyConnector->m_strDriver));
                    InsertIntoUpdateLog("No ["+m_pclCopyConnector->m_strDriver+"]SQL Database drivers detected (DB Type). Please contact Quantix support at "+QString(GEX_EMAIL_SUPPORT),true);
                }

            // Check if SQL driver from connector is supported
            int nIndex;
            for(nIndex=0; nIndex<mUi_qcbDbDriver->count(); nIndex++)
            {
                if(m_pclCopyConnector->GetDriverName(mUi_qcbDbDriver->itemText(nIndex)) == m_pclCopyConnector->m_strDriver)
                    mUi_qcbDbDriver->setCurrentIndex(nIndex);
            }
        }
    }
    else
    {
        InsertIntoUpdateLog("No SQL Database drivers detected (DB Type). Please contact Quantix support at "+QString(GEX_EMAIL_SUPPORT),true);
    }

    // force default values
    OnDatabaseDriverChanged();

    // only update default values if connector settings are define
    if(!m_pclCopyConnector->m_strHost_Unresolved.isEmpty())
        mUi_qleDbHostNameIP->setText(m_pclCopyConnector->m_strHost_Unresolved);

    if(!m_pclCopyConnector->m_strDatabaseName.isEmpty())
        mUi_qleDbName->setText(m_pclCopyConnector->m_strDatabaseName);

    if(!m_pclCopyConnector->m_strUserName.isEmpty())
        mUi_qleUserName->setText(m_pclCopyConnector->m_strUserName);

    if(!m_pclCopyConnector->m_strPassword.isEmpty())
        mUi_qleUserPassword->setText(m_pclCopyConnector->m_strPassword);

    if(!m_pclCopyConnector->m_strUserName_Admin.isEmpty())
        mUi_qleAdminName->setText(m_pclCopyConnector->m_strUserName_Admin);

    if(!m_pclCopyConnector->m_strPassword_Admin.isEmpty())
        mUi_qleAdminPassword->setText(m_pclCopyConnector->m_strPassword_Admin);


    // If connector has a port set (!=0), force it
    if(m_pclCopyConnector->m_uiPort != 0)
        mUi_qsbDbPortNb->setValue(m_pclCopyConnector->m_uiPort);

    mUi_qcbDbCreationCheckBox->setChecked(false);
    if(mUi_qcbTdrDbType->count() == 1)
        mUi_qcbDbCreationCheckBox->setText(QString("Create a new %1 instance").arg(mUi_qcbTdrDbType->itemText(mUi_qcbTdrDbType->currentIndex())));
    else
        mUi_qcbDbCreationCheckBox->setText(QString("Create a new Database instance"));

    // everything went well !
    return true;
}


bool GexDbPlugin_Galaxy_CfgWizard::UpdateSettingPage()
{
    if(!UpdateGuiFromConnector())
    {
        GEX_ASSERT(false);
        return false;
    }

    ////////////////////////////////////////////////////////
    // buttons :
    button(QWizard::BackButton)->setEnabled(false);
    button(QWizard::NextButton)->setEnabled(true);
    if(mUi_qcbDbCreationCheckBox->isChecked())
        button(QWizard::NextButton)->setText("Create >");
    else
        button(QWizard::NextButton)->setText("Connect >");

    // updated at the end of the method cause of OnSettingsChange() slot
    if(!IsLinkCreationAllowed())
        button(QWizard::FinishButton)->setText("Apply");
    else
        button(QWizard::FinishButton)->setText("Finish");


    ////////////////////////////////////////////////////////
    bool lIsDbCreationAllowed = IsLinkCreationAllowed() &&
            (m_pclDbPluginGalaxy->IsMonitoringMode() ||
             m_pclDbPluginGalaxy->IsTdrAllowed());
    mUi_qcbDbCreationCheckBox->setVisible(lIsDbCreationAllowed);
    mUi_qleDbRootPassword->clear();


    ////////////////////////////////////////////////////////
    // gui elements enabled (read only mode, ...)
    mUi_qleDbHostNameIP->setEnabled(!IsReadOnly());
    mUi_qsbDbPortNb->setEnabled(!IsReadOnly());
    mUi_qcbDbDriver->setEnabled(!IsReadOnly());

    QString strDriverName = m_pclCopyConnector->GetDriverName(mUi_qcbDbDriver->currentText());
    if(!m_pclCopyConnector->IsMySqlDB(strDriverName))
        mUi_qleDbName->setEnabled(!IsReadOnly());
    else
        mUi_qleDbName->setEnabled(false);
    mUi_qleUserName->setEnabled(!IsReadOnly());
    mUi_qleUserPassword->setEnabled(!IsReadOnly());
    mUi_qleAdminName->setEnabled(!IsReadOnly());
    mUi_qleAdminPassword->setEnabled(!IsReadOnly());
    mUi_qcbStartupType->setEnabled(!IsReadOnly());

    ////////////////////////////////////////////////////////
    // specific gui updates
    switch (m_eDataBaseType)
    {
    case GexDbPlugin_Galaxy::eYmAdminDb:
    {
        setWindowTitle("Yield-Man Server configuration wizard");
        // only cancel / create with yield man admin db
        button(QWizard::BackButton)->hide();
        button(QWizard::FinishButton)->hide();

        mUi_qleUserName->setEnabled(false);
        if(mUi_qleUserName->text().isEmpty())
            mUi_qleUserName->setText("root");
        mUi_qleUserName->hide();        labelDbUserName->hide();
        mUi_qleUserPassword->hide();    labelDbPassword->hide();

        mUi_qleAdminName->setText(m_pclCopyConnector->m_strUserName_Admin);
        mUi_qleAdminName->setEnabled(false);

        mUi_qleAdminPassword->clear();
        mUi_qleUserPassword->clear();

        mUi_qcbStartupType->hide();
        mUi_qcbDbCreationCheckBox->setText("Create a new Yield-Man Server database instance");
        mUi_qcbDbCreationCheckBox->hide();
        mUi_qcbDbCreationCheckBox->setChecked(true);

        // comment : not developped to assume that m_eDataBaseType can change
        labelDbName_Admin->setText("Yield-Man Server account:");
        labelDbPassword_Admin->setText("Account password");
        QString Info;
        Info = "<br><b>Yield-Man Server account and password:</b>\n";
        // GCORE-1151 : Remove Oracle traces from V7.3 package
        Info+= "<ul><li>This is the <b>MySQL account</b> that the Yield-Man application will use to interact with this database.\n";
        Info+= "<li>This account is used by all GEX applications as a 'Life Thread' with the Yield-Man Server.\n";
        Info+= "<li>This account password will not be communicated to any GEX users.\n";
        Info+= "<li>This is not the password that the user will enter to log in to a GEX application.\n";
        Info+= "</ul>";

        labelDbInfo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        labelDbInfo->setMaximumHeight(16777215);
        labelDbInfo->setTextFormat(Qt::RichText);
        labelDbInfo->setText(Info);

        mUi_qgbDbOptions->hide();
        mUi_qcbDbCreationCheckBox->setChecked(true);    // m_bAllowDatabaseCreation ?

        break;
    }
    case GexDbPlugin_Galaxy::eAdrLocalDb:
    case GexDbPlugin_Galaxy::eAdrDb:
        mUi_qcbDbCreationCheckBox->setChecked(true);    // m_bAllowDatabaseCreation by default
    case GexDbPlugin_Galaxy::eProdTdrDb:
    case GexDbPlugin_Galaxy::eCharacTdrDb:
    case GexDbPlugin_Galaxy::eManualProdDb:
    {
        if(mUi_qleDbHostNameIP->text().isEmpty() && !m_strHostName.isEmpty())
            mUi_qleDbHostNameIP->setText(m_strHostName);

        mUi_qleUserName->show();        labelDbUserName->show();
        mUi_qleUserPassword->show();    labelDbPassword->show();
        //mUi_qleAdminName->setEnabled(true);
        mUi_qcbStartupType->show();

        // set default values if undefined
        if(mUi_qleUserName->text().isEmpty())
            mUi_qleUserName->setText(m_pclCopyConnector->m_strUserName);
        if(mUi_qleUserPassword->text().isEmpty())
            mUi_qleUserPassword->setText(m_pclCopyConnector->m_strPassword);
        if(mUi_qleAdminName->text().isEmpty())
            mUi_qleAdminName->setText(m_pclCopyConnector->m_strUserName_Admin);
        if(mUi_qleAdminPassword->text().isEmpty())
            mUi_qleAdminPassword->setText(m_pclCopyConnector->m_strPassword_Admin);

        mUi_qcbStartupType->show();

        break;
    }
    default:
        break;
    }

    OnDbCreationChanged();    // force gui update

    // Set focus to first control
    mUi_qleDbHostNameIP->setFocus();

    ////////////////////////////////////////////////////////
    // buttons :
    button(QWizard::FinishButton)->setEnabled(false);

    return true;
}


bool GexDbPlugin_Galaxy_CfgWizard::UpdateCreateDbPage()
{
    if(!mUi_qcbDbCreationCheckBox->isChecked())
    {
        GEX_ASSERT(false);
        return false;
    }

    if(m_eDataBaseType!=GexDbPlugin_Galaxy::eYmAdminDb)
    {
        QMessageBox msgBox(this);

        m_pGexSkin->applyPalette(&msgBox);

        QString strMessage = "Database Settings:\n";
        strMessage+= "\n"+labelDbHostNameIP->text()+" " + mUi_qleDbHostNameIP->text();
        strMessage+= "\n"+labelDbPortNb->text()+" " + QString::number(mUi_qsbDbPortNb->value());
        strMessage+= "\n"+labelDbName->text()+" "+mUi_qleDbName->text();
        strMessage+= "\n"+labelDbName_Admin->text()+" " + mUi_qleAdminName->text();
        if(!labelDbUserName->isHidden())
            strMessage+= "\n"+labelDbUserName->text()+" " + mUi_qleUserName->text();

        strMessage+= "\n\nDo you want to create this new Database?\n";

        msgBox.setWindowTitle("Database Creation");
        msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        msgBox.setText(strMessage);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        // Multiple files selected: ask if Compare or Merge?
        msgBox.setButtonText( QMessageBox::Yes, "Create" );
        msgBox.setButtonText( QMessageBox::Cancel, "&Cancel" );

        if(msgBox.exec() == QMessageBox::Cancel)
        {
            restart();
            return false;
        }
    }

    ////////////////////////////////////////////////////
    // buttons (during create db)
    button(QWizard::BackButton)->setEnabled(false);
    button(QWizard::NextButton)->setEnabled(false);
    button(QWizard::FinishButton)->setEnabled(false);

    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    bool bDbCreationResult = GenericCreateDatabase();
    QGuiApplication::restoreOverrideCursor();

    if(m_eDataBaseType == GexDbPlugin_Galaxy::eYmAdminDb)
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        button(QWizard::BackButton)->setEnabled(false);
        button(QWizard::CancelButton)->setEnabled(false);
        bool bDbConnectiontionResult = GenericConnectDataBase();
        QGuiApplication::restoreOverrideCursor();

        if(bDbConnectiontionResult) // even if creation isn't ok (cause of ym_admin_db already exists)
            done(QDialog::Accepted);
        else
        {
            button(QWizard::BackButton)->setEnabled(true);
            button(QWizard::CancelButton)->setEnabled(true);
        }
    }

    if(!bDbCreationResult)
        GSLOG(SYSLOG_SEV_ERROR,"Can't create database");


    ////////////////////////////////////////
    // buttons

    button(QWizard::BackButton)->setEnabled(true);
    button(QWizard::BackButton)->setText("< Back");

    if(m_eDataBaseType == GexDbPlugin_Galaxy::eYmAdminDb)
    {
        button(QWizard::NextButton)->hide();
        button(QWizard::FinishButton)->hide();
    }
    else
    {
        button(QWizard::NextButton)->setEnabled(bDbCreationResult);
        button(QWizard::NextButton)->setText("Connect >");
        button(QWizard::FinishButton)->setEnabled(false);
    }

    return bDbCreationResult;
}

bool GexDbPlugin_Galaxy_CfgWizard::UpdateConnectDbPage()
{
    switch (m_eDataBaseType)
    {
    case GexDbPlugin_Galaxy::eCharacTdrDb:
    case GexDbPlugin_Galaxy::eManualProdDb:
    case GexDbPlugin_Galaxy::eProdTdrDb:
    case GexDbPlugin_Galaxy::eAdrDb:
    case GexDbPlugin_Galaxy::eAdrLocalDb:
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        bool bConnectionRslt = GenericConnectDataBase();
        QGuiApplication::restoreOverrideCursor();

        if(!bConnectionRslt)
            GSLOG(SYSLOG_SEV_ERROR, "Can't connect to the database");


        ////////////////////////////////////////
        // buttons

        button(QWizard::BackButton)->setEnabled(true);
        if(bConnectionRslt)
            button(QWizard::BackButton)->setText("< Disconnect");
        else
            button(QWizard::BackButton)->setText("< Back");
        button(QWizard::NextButton)->setEnabled(false);
        button(QWizard::NextButton)->setText("Next >");

        button(QWizard::FinishButton)->setEnabled(bConnectionRslt);

        return bConnectionRslt;
    }
    case GexDbPlugin_Galaxy::eYmAdminDb:
    {
        GEX_ASSERT(false);
    }

    default:
        GEX_ASSERT(false); // GSLOG to do ?
        return false;
    }
}

void GexDbPlugin_Galaxy_CfgWizard::InitComboTdrDbType()
{
    mUi_qcbTdrDbType->clear();
    if (!m_pclDbPluginGalaxy)
        return;

    QStringList lAllowedTypes = mGuiOptions["allowed_database_type"].split(",");

    if(lAllowedTypes.contains(GEXDB_CHAR_TDR_KEY))
    {
        mUi_qcbTdrDbType->addItem(GEXDB_CHAR_TDR_NAME, QVariant(GEXDB_CHAR_TDR_KEY));
        SetDataBaseType(GexDbPlugin_Galaxy::eCharacTdrDb);
    }

    if(lAllowedTypes.contains(GEXDB_MAN_PROD_TDR_KEY))
    {
        mUi_qcbTdrDbType->addItem(GEXDB_MAN_PROD_TDR_NAME, QVariant(GEXDB_MAN_PROD_TDR_KEY));
        SetDataBaseType(GexDbPlugin_Galaxy::eManualProdDb);
    }

    if(lAllowedTypes.contains(GEXDB_YM_PROD_TDR_KEY))
    {
        mUi_qcbTdrDbType->addItem(GEXDB_YM_PROD_TDR_NAME, QVariant(GEXDB_YM_PROD_TDR_KEY));
        SetDataBaseType(GexDbPlugin_Galaxy::eProdTdrDb);
    }

    if(lAllowedTypes.contains(GEXDB_ADR_KEY))
    {
        mUi_qcbTdrDbType->addItem(GEXDB_ADR_NAME, QVariant(GEXDB_ADR_KEY));
        SetDataBaseType(GexDbPlugin_Galaxy::eAdrDb);
    }

    if(lAllowedTypes.contains(GEXDB_ADR_LOCAL_KEY))
    {
        mUi_qcbTdrDbType->addItem(GEXDB_ADR_LOCAL_NAME, QVariant(GEXDB_ADR_LOCAL_KEY));
        SetDataBaseType(GexDbPlugin_Galaxy::eAdrLocalDb);
    }

    if(lAllowedTypes.contains(GEXDB_ADMIN_DB_KEY))
    {
        mUi_qcbTdrDbType->addItem(GEXDB_ADMIN_DB_NAME, QVariant(GEXDB_ADMIN_DB_KEY));
        SetDataBaseType(GexDbPlugin_Galaxy::eYmAdminDb);
    }


    // Force and hide the Startup options
    if((m_eDataBaseType == GexDbPlugin_Galaxy::eYmAdminDb)
            || (m_eDataBaseType == GexDbPlugin_Galaxy::eAdrDb)
            || (m_eDataBaseType == GexDbPlugin_Galaxy::eAdrLocalDb))
    {
        mUi_qgbDbOptions->hide();
        mUi_qcbStartupType->setCurrentIndex(0);
    }
    else
        mUi_qgbDbOptions->show();

    mUi_qcbTdrDbType->hide();
    mUi_qlTdrDbTypeLabel->hide();
    if(mUi_qcbDbCreationCheckBox->isChecked())
    {
        mUi_qcbTdrDbType->show();
        mUi_qlTdrDbTypeLabel->show();
    }
}



///////////////////////////////////////////////////////////////
// used to connect internal elements
///////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::Connect()
{
    // gui elements connections
    connect(m_pclDbPluginGalaxy, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateLogMessage(QString,bool)));
    connect(m_pclDbPluginGalaxy, SIGNAL(sUpdateProgress(int)), this, SLOT(UpdateProgress(int)));
    connect(m_pclDbPluginGalaxy, SIGNAL(sResetProgress(bool)), this, SLOT(ResetProgress(bool)));
    connect(m_pclDbPluginGalaxy, SIGNAL(sMaxProgress(int)), this, SLOT(SetMaxProgress(int)));

    connect(mUi_qcbDbDriver, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnDatabaseDriverChanged()));

    connect(mUi_qleAdminName, SIGNAL(textChanged(QString)),
            this, SLOT(OnAdminNameChanged(QString)));

    connect(mUi_qcbDbCreationCheckBox, SIGNAL(clicked()),
            this, SLOT(OnDbCreationChanged()));

    connect(this, SIGNAL(rejected()), this, SLOT(Clear()));

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(UpdateGui(int)));

    connect(mUi_qleDbHostNameIP, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qsbDbPortNb, SIGNAL(valueChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qcbDbDriver, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qleDbName, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qleUserName, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qleUserPassword, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qleAdminName, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qleAdminPassword, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));

    connect(mUi_qcbStartupType, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));

    connect(mUi_qleDbRootName, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));
    connect(mUi_qleDbRootPassword, SIGNAL(textChanged(QString)),
            this, SLOT(OnSettingsChange(QString)));


    // buttons
    button(QWizard::BackButton)->disconnect();  // allow to 'overload' non virtual back method
    connect(button(QWizard::BackButton), SIGNAL(clicked()), this, SLOT(back()));
}


///////////////////////////////////////////////////////////////
// used to reset all attributes (free memory)
///////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy_CfgWizard::Clear()
{
    // internal datas attributes
    if (m_pclCopyConnector)
    {
        delete m_pclCopyConnector;
        m_pclCopyConnector = NULL;
    }
    m_bCustomerDebugMode = false;

    // gui attributes
    ClearLogs();
    mUi_qcbDbCreationCheckBox->setChecked(false);
    mUi_qleDbHostNameIP->clear();
    mUi_qleDbName->clear();
    mUi_qleUserName->clear();
    mUi_qleUserPassword->clear();
    mUi_qleAdminName->clear();
    mUi_qleAdminPassword->clear();
    mUi_qleDbRootName->clear();
    mUi_qleDbRootPassword->clear();

    // gui and internal datas attributes
    m_strHostName=QString();
    m_strlDrivers.clear();
    m_strlDrivers_FriendlyNames.clear();

    setStartId(GexDbPlugin_Galaxy_CfgWizard::eDbSettingsPage);
}


void GexDbPlugin_Galaxy_CfgWizard::ClearLogs()
{
    mUi_qteCreateDbLog->clear();
    mUi_qteConnectDbLog->clear();
}
