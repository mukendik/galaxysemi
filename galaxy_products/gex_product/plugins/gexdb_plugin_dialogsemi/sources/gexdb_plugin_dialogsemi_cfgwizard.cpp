
#include "gexdb_plugin_dialogsemi_cfgwizard.h"
#include <gex_shared.h>


///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
GexDbPlugin_Dialogsemi_CfgWizard::GexDbPlugin_Dialogsemi_CfgWizard( const QStringList & strlGexFields, CGexSkin * pGexSkin, QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : Q3Wizard( parent, name, modal, fl )
{
	setupUi(this);
		
	// Set Examinator skin
	pGexSkin->applyPalette(this);
	
	// Init some members
	m_pclDbConnector = new GexDbPlugin_Connector("ConfigWizard");
	m_strlGexFields = strlGexFields;
	m_pButtonNext = nextButton();
	m_pButtonBack = backButton();

	// Select page
	showPage(pageDbConnection);
	pageDbConnection_labelDbStatus->clear();

	// Signal/Slot connections
	connect(comboBoxDbType, SIGNAL(activated(const QString &)), this, SLOT(OnDatabaseTypeChanged()));
	connect(editDbHostNameIP, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editDbName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editDbUserName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editDbPassword, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(this, SIGNAL(selected(const QString &)), this, SLOT(UpdateGui()));
	connect(finishButton(), SIGNAL(clicked()), this, SLOT(OnFinish()));
	connect(cancelButton(), SIGNAL(clicked()), this, SLOT(OnCancel()));

	// Update GUI
	UpdateGui();

	// Set focus to first control
	editDbHostNameIP->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
GexDbPlugin_Dialogsemi_CfgWizard::~GexDbPlugin_Dialogsemi_CfgWizard()
{
	delete m_pclDbConnector;
}

///////////////////////////////////////////////////////////
// User clicked the 'Finish' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi_CfgWizard::OnFinish()
{
	m_pclDbConnector->Disconnect();
}

///////////////////////////////////////////////////////////
// User clicked the 'Cancel' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi_CfgWizard::OnCancel()
{
	m_pclDbConnector->Disconnect();
}

///////////////////////////////////////////////////////////
// User clicked the 'Next' button
///////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi_CfgWizard::next()
{
	QString strDbError;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// Check which page is active

	if(currentPage() == pageDbConnection)
	{
		// Database connection settings page
		
		// Update connector
		m_pclDbConnector->m_strHostName = editDbHostNameIP->text();
		m_pclDbConnector->m_uiPort = spinBoxDbPortNb->value();
		m_pclDbConnector->m_strDriver = m_pclDbConnector->GetDriverName(comboBoxDbType->currentText());
		m_pclDbConnector->m_strDatabaseName = editDbName->text();
		m_pclDbConnector->m_strUserName = editDbUserName->text();
		m_pclDbConnector->m_strPassword = editDbPassword->text();

		// Disconnect if connected to DB
		pageDbConnection_labelDbStatus->clear();
		if(m_pclDbConnector->IsConnected())
			m_pclDbConnector->Disconnect();

		// Connect to DB
		if(!m_pclDbConnector->Connect())
		{
			m_pclDbConnector->GetLastError(strDbError);
			pageDbConnection_labelDbStatus->setText(strDbError);
            QApplication::restoreOverrideCursor();
			return;
		}

		// Wizard Next function
		Q3Wizard::next();

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
void GexDbPlugin_Dialogsemi_CfgWizard::back()
{
	// Check which page is active

	if(currentPage() == pageFinish)
	{
		// Database table selection page
		
		pageDbConnection_labelDbStatus->clear();

		// Disconnect
		m_pclDbConnector->Disconnect();

		// Wizard Back function
		Q3Wizard::back();

		// Update GUI
		UpdateGui();
		return;
	}
}

///////////////////////////////////////////////////////////
// Set controls from data
///////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi_CfgWizard::Set(const GexDbPlugin_Connector & clDbConnector)
{
	// Init members
	*m_pclDbConnector = clDbConnector;

	// Set values of GUI controls: Database connection settings page
	editDbHostNameIP->setText(m_pclDbConnector->m_strHostName);
	spinBoxDbPortNb->setValue(m_pclDbConnector->m_uiPort);
	editDbName->setText(m_pclDbConnector->m_strDatabaseName);
	editDbUserName->setText(m_pclDbConnector->m_strUserName);
	editDbPassword->setText(m_pclDbConnector->m_strPassword);
	comboBoxDbType->clear();
	if(m_pclDbConnector->EnumDrivers(m_strlDrivers, m_strlDrivers_FriendlyNames) && (m_strlDrivers.count() > 0))
	{
		comboBoxDbType->insertStringList(m_strlDrivers_FriendlyNames);
		comboBoxDbType->setCurrentItem(0);

		// Check if SQL driver from connector is supported
		int nIndex;
		for(nIndex=0; nIndex<comboBoxDbType->count(); nIndex++)
		{
			if(m_pclDbConnector->GetDriverName(comboBoxDbType->text(nIndex)) == m_pclDbConnector->m_strDriver)
				comboBoxDbType->setCurrentItem(nIndex);
		}
	}
	else
	{
		pageDbConnection_labelDbStatus->setText("No SQL Database drivers detected (DB Type). Please contact Galaxy support at "+QString(GEX_EMAIL_SUPPORT));
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
void GexDbPlugin_Dialogsemi_CfgWizard::Get(GexDbPlugin_Connector & clDbConnector)
{
	// Save data
	clDbConnector = *m_pclDbConnector;
}

///////////////////////////////////////////////////////////
// User changed database type...
///////////////////////////////////////////////////////////
void GexDbPlugin_Dialogsemi_CfgWizard::OnDatabaseTypeChanged()
{
	QString strDatabaseType = m_pclDbConnector->GetDriverName(comboBoxDbType->currentText());

	if(strDatabaseType == "QMYSQL3")
	{
		labelDbName->setText("Database name:");
		spinBoxDbPortNb->setValue(3306);
	}
	else if(strDatabaseType == "QOCI8")
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
void GexDbPlugin_Dialogsemi_CfgWizard::UpdateGui()
{
	QString strDbError;
	
	// Check which pae is active
	if(currentPage() == pageDbConnection)
	{
		// Database connection settings page
		
		m_pButtonNext->setText("Connect >");
		m_pButtonBack->setText("< Back");
		if(m_strlDrivers.count() == 0)
		{
			setNextEnabled(pageDbConnection, false);
			return;
		}
		if(editDbHostNameIP->text().isEmpty())
		{
			setNextEnabled(pageDbConnection, false);
			return;
		}
		if(editDbName->text().isEmpty())
		{
			setNextEnabled(pageDbConnection, false);
			return;
		}
		if(editDbUserName->text().isEmpty())
		{
			setNextEnabled(pageDbConnection, false);
			return;
		}
		if(editDbPassword->text().isEmpty())
		{
			setNextEnabled(pageDbConnection, false);
			return;
		}

		// All fields are filled, enable Next button
		setNextEnabled(pageDbConnection, true);
		return;
	}

	if(currentPage() == pageFinish)
	{
		// Last empty page
	
		m_pButtonNext->setText("Next >");
		m_pButtonBack->setText("< Disconnect");

		// All fields are filled, enable Finish button
		setFinishEnabled(pageFinish, true);
		return;
	}
}

