#include "gexdb_plugin_medtronic_cfgwizard.h"

#include <QPushButton>
#include <gex_shared.h>
#include <QWizard>

#undef QT3_SUPPORT
///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
GexDbPlugin_Medtronic_CfgWizard::GexDbPlugin_Medtronic_CfgWizard( const QStringList & strlGexFields, CGexSkin * pGexSkin, QWidget* parent, const char* /*name*/, bool /*modal*/, Qt::WindowFlags /*fl*/ )
    : QWizard(parent,Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setupUi(this);

    // Set Examinator skin
    pGexSkin->applyPalette(this);

    // Init some members
    m_pclDbConnector = new GexDbPlugin_Connector("ConfigWizard");
    m_strlGexFields = strlGexFields;
    m_pButtonNext = (QPushButton*) button(QWizard::NextButton);
    m_pButtonBack = (QPushButton*) button(QWizard::BackButton);

    // Select page
    //showPage(pageDbConnection);
    pageDbConnection_labelDbStatus->clear();

    // Signal/Slot connections
    connect(comboBoxDbType, SIGNAL(activated(const QString &)), this, SLOT(OnDatabaseTypeChanged()));
    connect(editDbHostNameIP, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
    connect(editDbName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
    connect(editDbUserName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
    connect(editDbPassword, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
    connect(this, SIGNAL(selected(const QString &)), this, SLOT(UpdateGui()));
    connect(button(QWizard::NextButton), SIGNAL(clicked()), this, SLOT(next()));
    connect(button(QWizard::BackButton), SIGNAL(clicked()), this, SLOT(back()));
    connect(button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(OnFinish()));
    connect(button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(OnCancel()));

    // Update GUI
    UpdateGui();

    // Set focus to first control
    editDbHostNameIP->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
GexDbPlugin_Medtronic_CfgWizard::~GexDbPlugin_Medtronic_CfgWizard()
{
    delete m_pclDbConnector;
}

///////////////////////////////////////////////////////////
// User clicked the 'Finish' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::OnFinish()
{
    m_pclDbConnector->Disconnect();
}

///////////////////////////////////////////////////////////
// User clicked the 'Cancel' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::OnCancel()
{
    m_pclDbConnector->Disconnect();
}

///////////////////////////////////////////////////////////
// User clicked the 'Next' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::next()
{
    QString strDbError;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // Check which page is active

    if(currentId() == GexDbPlugin_Medtronic_CfgWizard::eDbConnectionPage)
    {
        // Database connection settings page

        // Update connector
        m_pclDbConnector->m_strHost_Unresolved = editDbHostNameIP->text();
        m_pclDbConnector->m_strHost_Name = editDbHostNameIP->text();
        m_pclDbConnector->m_strHost_IP = editDbHostNameIP->text();
        m_pclDbConnector->m_uiPort = spinBoxDbPortNb->value();
        m_pclDbConnector->m_strDriver = m_pclDbConnector->GetDriverName(comboBoxDbType->currentText());
        m_pclDbConnector->m_strDatabaseName = editDbName->text();
        m_pclDbConnector->m_strUserName = m_pclDbConnector->m_strUserName_Admin =editDbUserName->text();
        m_pclDbConnector->m_strPassword = m_pclDbConnector->m_strPassword_Admin = editDbPassword->text();

        // Disconnect if connected to DB
        pageDbConnection_labelDbStatus->clear();
        if(m_pclDbConnector->IsConnected())
            m_pclDbConnector->Disconnect();

        QString lText;
        lText = "<b>** "+m_pclDbConnector->m_strSchemaName.toUpper()+" DATABASE CONNECTION **</b>";
        lText+= "<BR>\n ";
        lText+= "<BR>\n<b>o "+labelDbHostNameIP->text()+"</b> " + m_pclDbConnector->m_strHost_Name;
        lText+= "<BR>\n<b>o "+labelDbPortNb->text()+"</b> " + QString::number(m_pclDbConnector->m_uiPort);
        lText+= "<BR>\n<b>o "+labelDbType->text()+"</b> " + m_pclDbConnector->m_strDriver;
        lText+= "<BR>\n<b>o "+labelDbName->text()+"</b> "+m_pclDbConnector->m_strDatabaseName;
        lText+= "<BR>\n<b>o "+labelDbUserName->text()+"</b> " + m_pclDbConnector->m_strUserName;
        lText+= "<BR>\n ";

        pageDbConnection_labelDbStatus->setText(lText);

        // Connect to DB
        if(!m_pclDbConnector->Connect())
        {
            m_pclDbConnector->GetLastError(strDbError);
            lText += strDbError;
            pageDbConnection_labelDbStatus->setText(lText);
            QApplication::restoreOverrideCursor();
            return;
        }

        lText+= "<BR>\nDATABASE CONNECTION: <b>Success</b>";
        pageDbConnection_labelDbStatus->setText(lText);

        // Update GUI
        UpdateGui();
        QApplication::restoreOverrideCursor();
        return;
    }

    QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////
// User clicked the 'Back' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::back()
{
    // Check which page is active

    if(currentId() == GexDbPlugin_Medtronic_CfgWizard::eDbSettingsPage)
    {
        // Database table selection page

        pageDbConnection_labelDbStatus->clear();

        // Disconnect
        m_pclDbConnector->Disconnect();

        // Update GUI
        UpdateGui();
        return;
    }
}

///////////////////////////////////////////////////////////
// Set controls from data
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::Set(const GexDbPlugin_Connector & clDbConnector)
{
    // Init members
    *m_pclDbConnector = clDbConnector;

    // Set values of GUI controls: Database connection settings page
    editDbHostNameIP->setText(m_pclDbConnector->m_strHost_Name);
    spinBoxDbPortNb->setValue(m_pclDbConnector->m_uiPort);
    editDbName->setText(m_pclDbConnector->m_strDatabaseName);
    editDbUserName->setText(m_pclDbConnector->m_strUserName);
    editDbPassword->setText(m_pclDbConnector->m_strPassword);
    comboBoxDbType->clear();
    if(m_pclDbConnector->EnumDrivers(m_strlDrivers, m_strlDrivers_FriendlyNames) && (m_strlDrivers.count() > 0))
    {
        comboBoxDbType->insertItems(0, m_strlDrivers_FriendlyNames);
        comboBoxDbType->setCurrentIndex(0);

        // Check if SQL driver from connector is supported
        int nIndex;
        for(nIndex=0; nIndex<comboBoxDbType->count(); nIndex++)
        {
            if(m_pclDbConnector->GetDriverName(comboBoxDbType->itemText(nIndex)) == m_pclDbConnector->m_strDriver)
                comboBoxDbType->setCurrentIndex(nIndex);
        }
    }
    else
    {
        pageDbConnection_labelDbStatus->setText("No SQL Database drivers detected (DB Type). Please contact Quantix support at "+QString(GEX_EMAIL_SUPPORT));
    }

    // Set some default values depending on database type, and update GUI
    OnDatabaseTypeChanged();

    // If connector has a port set (!=0), force it
    if(m_pclDbConnector->m_uiPort != 0)
        spinBoxDbPortNb->setValue(m_pclDbConnector->m_uiPort);
}

///////////////////////////////////////////////////////////
// Get data from controls
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::Get(GexDbPlugin_Connector & clDbConnector)
{
    // Save data
    clDbConnector = *m_pclDbConnector;
}

///////////////////////////////////////////////////////////
// User changed database type...
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::OnDatabaseTypeChanged()
{
    QString strDatabaseType = m_pclDbConnector->GetDriverName(comboBoxDbType->currentText());

    if(m_pclDbConnector->IsMySqlDB(strDatabaseType))
    {
        labelDbName->setText("Database name:");
        spinBoxDbPortNb->setValue(3306);
    }
    else if(m_pclDbConnector->IsOracleDB(strDatabaseType))
    {
        labelDbName->setText("TNS name:");
        spinBoxDbPortNb->setValue(1521);
    }
    else
        labelDbName->setText("Database name:");

    // Update GUI
    UpdateGui();
}

///////////////////////////////////////////////////////////
// Update GUI: enable/disable controls...
///////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_CfgWizard::UpdateGui()
{
    QString strDbError;

    // Check which pae is active
    if(currentId() == GexDbPlugin_Medtronic_CfgWizard::eDbSettingsPage)
    {
        // Database connection settings page

        m_pButtonNext->setText("Connect >");
        m_pButtonBack->setText("< Back");
        if(m_strlDrivers.count() == 0)
        {
            //setNextEnabled(pageDbConnection, false);
            button(QWizard::NextButton)->setEnabled(false);
            return;
        }
        if(editDbHostNameIP->text().isEmpty())
        {
            //setNextEnabled(pageDbConnection, false);
            button(QWizard::NextButton)->setEnabled(false);
            return;
        }
        if(editDbName->text().isEmpty())
        {
            //setNextEnabled(pageDbConnection, false);
            button(QWizard::NextButton)->setEnabled(false);
            return;
        }
        if(editDbUserName->text().isEmpty())
        {
            //setNextEnabled(pageDbConnection, false);
            button(QWizard::NextButton)->setEnabled(false);
            return;
        }
        if(editDbPassword->text().isEmpty())
        {
            //setNextEnabled(pageDbConnection, false);
            button(QWizard::NextButton)->setEnabled(false);
            return;
        }

        // All fields are filled, enable Next button
        //setNextEnabled(pageDbConnection, true);
        button(QWizard::NextButton)->setEnabled(true);
        return;
    }

    if(currentId() == GexDbPlugin_Medtronic_CfgWizard::eDbConnectionPage)
    {
        // Last empty page

        m_pButtonNext->setText("Next >");
        m_pButtonBack->setText("< Disconnect");
        button(QWizard::NextButton)->setEnabled(false);

        // All fields are filled, enable Finish button
        //setFinishEnabled(pageFinish, true);
        button(QWizard::FinishButton)->setEnabled(true);
        return;
    }
}

///////////////////////////////////////////////////////////
// Used to manage QWizard navigation
///////////////////////////////////////////////////////////
int GexDbPlugin_Medtronic_CfgWizard::nextId() const
{
    switch (currentId())
    {
    case GexDbPlugin_Medtronic_CfgWizard::eDbSettingsPage:
        return GexDbPlugin_Medtronic_CfgWizard::eDbConnectionPage;

    default:
        return GexDbPlugin_Medtronic_CfgWizard::eDbSettingsPage;
    }
}
