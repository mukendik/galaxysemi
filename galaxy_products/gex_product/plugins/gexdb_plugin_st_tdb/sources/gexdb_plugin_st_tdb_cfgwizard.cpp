#include "gexdb_plugin_st_tdb_cfgwizard.h"
#include "gexdb_plugin_itemselectiondialog.h"
#include "gexdb_plugin_st_tdb_constants.h"
#include <gex_shared.h>


///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
GexDbPlugin_StTdb_CfgWizard::GexDbPlugin_StTdb_CfgWizard( const QStringList & strlGexFields, CGexSkin * pGexSkin, QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : Q3Wizard( parent, name, modal, fl )
{
	setupUi(this);

	// Set Examinator skin
	pGexSkin->applyPalette(this);

	// Init some members
	m_pGexSkin = pGexSkin;
	m_pclDbConnector = new GexDbPlugin_Connector("ConfigWizard");
	m_strlGexFields = strlGexFields;
	m_pButtonNext = nextButton();
	m_pButtonBack = backButton();

	// Select page
	showPage(pageDbConnection);
	pageDbConnection_labelDbStatus->clear();
	pageTableSelection_labelDbStatus->clear();
	pageDbMapping_labelDbStatus->clear();

	// Signal/Slot connections
	connect(comboBoxDbType, SIGNAL(activated(const QString &)), this, SLOT(OnDatabaseTypeChanged()));
	connect(editDbHostNameIP, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editDbName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editDbUserName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editDbPassword, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editFtpHostNameIP, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editFtpUserName, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(editFtpPassword, SIGNAL(textChanged(const QString &)), this, SLOT(UpdateGui()));
	connect(this, SIGNAL(selected(const QString &)), this, SLOT(UpdateGui()));
	connect(finishButton(), SIGNAL(clicked()), this, SLOT(OnFinish()));
	connect(cancelButton(), SIGNAL(clicked()), this, SLOT(OnCancel()));
	connect(listViewMapping, SIGNAL(doubleClicked(Q3ListViewItem*)), this, SLOT(OnFieldMappingMap()));

	// Update GUI
	UpdateGui();

	// Set focus to first control
	editDbHostNameIP->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
GexDbPlugin_StTdb_CfgWizard::~GexDbPlugin_StTdb_CfgWizard()
{
	delete m_pclDbConnector;
}

///////////////////////////////////////////////////////////
// User clicked the 'Finish' button
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb_CfgWizard::OnFinish()
{
	m_pclDbConnector->Disconnect();
}

///////////////////////////////////////////////////////////
// User clicked the 'Cancel' button
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb_CfgWizard::OnCancel()
{
	m_pclDbConnector->Disconnect();
}

///////////////////////////////////////////////////////////
// User clicked the 'Next' button
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb_CfgWizard::next()
{
	QString							strDbError, strString, strDbField, strGalaxyField;
	QMap<QString,QString>::Iterator	itMappingKey;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// Check which page is active

	if(currentPage() == pageDbConnection)
	{
		// Database connection settings page
		
		// Update connector
		m_pclDbConnector->m_strHost_Unresolved = editDbHostNameIP->text();
		m_pclDbConnector->m_strHost_Name = editDbHostNameIP->text();
		m_pclDbConnector->m_strHost_IP = editDbHostNameIP->text();
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

		// Get list of available tables
		m_strlTables.clear();
		if(!m_pclDbConnector->EnumTables(m_strlTables))
		{
			m_pclDbConnector->GetLastError(strDbError);
			pageDbConnection_labelDbStatus->setText(strDbError);
			m_pclDbConnector->Disconnect();
			QApplication::restoreOverrideCursor();
			return;
		}
		if(m_strlTables.count() == 0)
		{
			pageDbConnection_labelDbStatus->setText("This database doesn't have any tables.");
			m_pclDbConnector->Disconnect();
			QApplication::restoreOverrideCursor();
			return;
		}

		// Tables retrieved, fill listbox
		listBoxTables->clear();
		listBoxTables->insertStringList(m_strlTables);
		listBoxTables->sort();

		// Select table
		Q3ListBoxItem *pItem;
		listBoxTables->setCurrentItem(0);
		if(!m_strTableName.isEmpty() && (pItem = listBoxTables->findItem(m_strTableName)))
			listBoxTables->setCurrentItem(pItem);

		// Wizard Next function
		Q3Wizard::next();

		// Update GUI
		UpdateGui();
		QApplication::restoreOverrideCursor();
		return;
	}

	if(currentPage() == pageTableSelection)
	{
		// Database table selection page
		
		// Update table name
		m_strTableName = listBoxTables->currentText();
		strString = "Fields mapping for: ";
		strString += m_strTableName;
		labelMapping->setText(strString);

		// Update field mapping
		int nItemsRemoved, nItemsAdded;
		if(!m_pclDbConnector->UpdateMapping(&nItemsRemoved, &nItemsAdded, m_strTableName, m_strlGexFields, m_cFieldsMapping))
		{
			m_pclDbConnector->GetLastError(strDbError);
			pageTableSelection_labelDbStatus->setText(strDbError);
			QApplication::restoreOverrideCursor();
			return;
		}
		strDbError.sprintf(	"The field mapping table has been updated for table %s.%s:\no %d items have been removed\no %d items have been added.",
							m_pclDbConnector->m_strDatabaseName.latin1(),
							m_strTableName.latin1(),
							nItemsRemoved,
							nItemsAdded);
		pageDbMapping_labelDbStatus->setText(strDbError);

		// Fill Mapping table
		listViewMapping->clear();

		// Populate field mapping list view and update Ftp "Hostname from database field" checkbox
		strString = "From Database (?)";
		checkBoxFtpHostNameFromDB->setText(strString);
		checkBoxFtpHostNameFromDB->setEnabled(false);

		for(itMappingKey = m_cFieldsMapping.begin(); itMappingKey != m_cFieldsMapping.end(); ++itMappingKey)
		{
			strDbField = itMappingKey.key();
			strGalaxyField = itMappingKey.data();

			// Insert item in listview
			new Q3ListViewItem(listViewMapping, 
				strDbField,			// Remote database Key
				strGalaxyField);	// Galaxy mapped Key

			// If host name field, update ftp GUI
			if(strGalaxyField == GEXDB_PLUGIN_ST_TDB_FIELD_HOSTNAME)
			{
				strString = "From Database (" + strDbField;
				strString += ")";
				checkBoxFtpHostNameFromDB->setText(strString);
				checkBoxFtpHostNameFromDB->setEnabled(true);
			}
		}

		// Sort list view
		listViewMapping->setSorting(0);
		listViewMapping->sort();

		// Wizard Next function
		Q3Wizard::next();

		// Update GUI
		UpdateGui();
		QApplication::restoreOverrideCursor();
		return;
	}

	if(currentPage() == pageDbMapping)
	{
		// Database mapping page
		
		// Update Ftp "Hostname from database field" checkbox
		strString = "From Database (?)";
		checkBoxFtpHostNameFromDB->setText(strString);
		checkBoxFtpHostNameFromDB->setEnabled(false);

		for(itMappingKey = m_cFieldsMapping.begin(); itMappingKey != m_cFieldsMapping.end(); ++itMappingKey)
		{
			strDbField = itMappingKey.key();
			strGalaxyField = itMappingKey.data();

			// If host name field, update ftp GUI
			if(strGalaxyField == GEXDB_PLUGIN_ST_TDB_FIELD_HOSTNAME)
			{
				strString = "From Database (" + strDbField;
				strString += ")";
				checkBoxFtpHostNameFromDB->setText(strString);
				checkBoxFtpHostNameFromDB->setEnabled(true);
			}
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
void GexDbPlugin_StTdb_CfgWizard::back()
{
	// Check which page is active

	if(currentPage() == pageTableSelection)
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

	if(currentPage() == pageDbMapping)
	{
		// Database mapping page

		pageTableSelection_labelDbStatus->clear();

		// Wizard Back function
		Q3Wizard::back();

		// Update GUI
		UpdateGui();
		return;
	}

	if(currentPage() == pageFtpSettings)
	{
		// Ftp settings page

		pageDbMapping_labelDbStatus->clear();

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
void GexDbPlugin_StTdb_CfgWizard::Set(const GexDbPlugin_Connector & clDbConnector, const GexDbPlugin_FtpSettings & clFtpSettings, const QString & strTableName, const QMap<QString,QString> & cFieldsMapping)
{
	// Init members
	*m_pclDbConnector = clDbConnector;
	m_strTableName = strTableName;
	m_cFieldsMapping = cFieldsMapping;	

	// Set values of GUI controls: Database connection settings page
	editDbHostNameIP->setText(m_pclDbConnector->m_strHost_Name);
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

	// Set values of GUI controls: Ftp settings page
	editFtpHostNameIP->setText(clFtpSettings.m_strHostName);
	spinBoxDbPortNb->setValue(clFtpSettings.m_uiPort);
	editFtpUserName->setText(clFtpSettings.m_strUserName);
	editFtpPassword->setText(clFtpSettings.m_strPassword);

	// Set some default values depending on database type, and update GUI
	OnDatabaseTypeChanged();

	// If connector has a port set (!=0), force it
	if(m_pclDbConnector->m_uiPort != 0)
		spinBoxDbPortNb->setValue(m_pclDbConnector->m_uiPort);
}

///////////////////////////////////////////////////////////
// Get data from controls
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb_CfgWizard::Get(GexDbPlugin_Connector & clDbConnector, GexDbPlugin_FtpSettings & clFtpSettings, QString & strTableName, QMap<QString,QString> & cFieldsMapping)
{
	// Save data
	clDbConnector = *m_pclDbConnector;
	clFtpSettings.m_strHostName = editFtpHostNameIP->text();
	clFtpSettings.m_bHostnameFromDbField = checkBoxFtpHostNameFromDB->isChecked();
	clFtpSettings.m_uiPort = spinBoxFtpPortNb->value();
	clFtpSettings.m_strUserName = editFtpUserName->text();
	clFtpSettings.m_strPassword = editFtpPassword->text();
	strTableName = m_strTableName;
	cFieldsMapping = m_cFieldsMapping;	
}

///////////////////////////////////////////////////////////
// User changed database type...
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb_CfgWizard::OnDatabaseTypeChanged()
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
void GexDbPlugin_StTdb_CfgWizard::UpdateGui()
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

	if(currentPage() == pageTableSelection)
	{
		// Database table selection page
	
		m_pButtonNext->setText("Next >");
		m_pButtonBack->setText("< Disconnect");
		
		// Enable Next button
		setNextEnabled(pageTableSelection, true);
		return;
	}

	if(currentPage() == pageDbMapping)
	{
		// Database mapping page
	
		m_pButtonNext->setText("Next >");
		m_pButtonBack->setText("< Back");
		return;
	}

	if(currentPage() == pageFtpSettings)
	{
		// Ftp settings page
	
		m_pButtonNext->setText("Next >");
		m_pButtonBack->setText("< Back");
		if(editFtpHostNameIP->text().isEmpty())
		{
			setFinishEnabled(pageFtpSettings, false);
			return;
		}
		if(editFtpUserName->text().isEmpty())
		{
			setFinishEnabled(pageFtpSettings, false);
			return;
		}
		if(editFtpPassword->text().isEmpty())
		{
			setFinishEnabled(pageFtpSettings, false);
			return;
		}

		// All fields are filled, enable Finish button
		setFinishEnabled(pageFtpSettings, true);
		return;
	}
}

///////////////////////////////////////////////////////////
// User double-clicked a field for mapping...
///////////////////////////////////////////////////////////
void GexDbPlugin_StTdb_CfgWizard::OnFieldMappingMap(void)
{
	// Let the user select the table to be used
	GexDbPlugin_ItemSelectionDialog clFieldSelectionDlg("Please select the GEX field to map to:", m_strlGexFields, m_pGexSkin);
	int nReturn = clFieldSelectionDlg.exec();
	if(nReturn == QDialog::Rejected)
		return;

	// Update mapping
	QString strGalaxyField = clFieldSelectionDlg.listBoxItemName->currentText();
	QString strDbField = listViewMapping->currentItem()->text(0);
	if(strGalaxyField == GEXDB_PLUGIN_ST_TDB_FIELD_NONE)
		strGalaxyField = "";
	else
		m_cFieldsMapping[strDbField] = strGalaxyField;

	// Update list view
	listViewMapping->currentItem()->setText(1, strGalaxyField);
}

