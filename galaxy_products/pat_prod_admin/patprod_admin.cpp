#include <QtGui>

#include "PatProdRecipe_Admin.h"
#include "recipe_profile_eng.h"
#include "recipe_profile_prod.h"


// Software revision
#define VERSION_ID	2.1

extern QApplication app;

PatProdRecipe_Admin::PatProdRecipe_Admin(QWidget *parent) : QDialog(parent,Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint)
{
    ui.setupUi(this);

	// Display Revision# in header bar
	QString strVersionName = "PROD Admin V" + QString::number(VERSION_ID,'f',1);
	setWindowTitle(strVersionName);	// eg: PROD Admin V2.1

    // Reset variables
	resize(365,125);
    m_bLogged = false;	    // true when user is logged
    m_bChanges = false;	    // true if edits done
	m_lfENG_SettingsVersion = 1.0;
	m_lfPROD_SettingsVersion = 1.0;
    ui.tabWidget->hide();   // User needs to login first!
	ui.comboBoxGroupList->hide();
	ui.lineEditRecipeFilter->hide();
	m_iPPAT_Bin = 81;
	m_iGPAT_Bin = 80;
	m_iZPAT_Bin = 87;

    // Build path to configuration file (.INI format)
    m_strConfigFile = app.applicationDirPath();
	m_strConfigFile += "/pat_server.ini";

    // Gui behavior settings
	ui.tableWidgetRecipes_ENG->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableWidgetUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tableWidgetUsers->setColumnHidden(1,true);   // Hide password column

	// Enable Drops + custom contexctual menu
	setAcceptDrops(true);
	ui.tableWidgetRecipes_PROD->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.tableWidgetRecipes_PROD->setColumnWidth(4,200);		// 'Flows / recipe details' column

	// Connect signals: tabs
	QObject::connect(ui.tabWidget,	    SIGNAL(currentChanged(int)),	this, SLOT(loadLogTab(void)));

	// Connect signals: Login/out
	QObject::connect(ui.pushButtonLogin,	    SIGNAL(clicked()),	this, SLOT(onLogin(void)));

	QObject::connect(ui.tableWidgetRecipes_PROD, SIGNAL(customContextMenuRequested(const QPoint&)),	this,	SLOT(onContextualMenu(const QPoint&)));

	// Connect signals: ENG Recipes
	QObject::connect(ui.pushButtonAddRecipe_ENG,	    SIGNAL(clicked()),	this, SLOT(onAddRecipe_ENG(void)));
	QObject::connect(ui.comboBoxGroupList,	    SIGNAL(activated(int)),	this, SLOT(onSelectGroup()));
	QObject::connect(ui.lineEditRecipeFilter,	    SIGNAL(textChanged(const QString &)),	this, SLOT(onSelectGroup()));
	QObject::connect(ui.pushButtonPropertiesRecipe_ENG, SIGNAL(clicked()),	this, SLOT(onPropertiesRecipe_ENG(void)));
	QObject::connect(ui.tableWidgetRecipes_ENG,	    SIGNAL(cellDoubleClicked(int,int)),	this, SLOT(onPropertiesRecipe_ENG(void)));
	QObject::connect(ui.pushButtonRemoveRecipe_ENG,	    SIGNAL(clicked()),	this, SLOT(onRemoveRecipe_ENG(void)));

	// Connect signals: PROD Recipes
	QObject::connect(ui.pushButtonNewRecipe_PROD,	    SIGNAL(clicked()),	this, SLOT(onNewRecipe_PROD(void)));
	QObject::connect(ui.pushButtonAddRecipe_PROD,	    SIGNAL(clicked()),	this, SLOT(onAddRecipe_PROD(void)));

	QObject::connect(ui.pushButtonPropertiesRecipe_PROD, SIGNAL(clicked()),	this, SLOT(onPropertiesRecipe_PROD(void)));
	QObject::connect(ui.pushButtonEditRecipe_PROD, SIGNAL(clicked()),	this, SLOT(onEditRecipe_PROD(void)));
	QObject::connect(ui.tableWidgetRecipes_PROD,	    SIGNAL(cellDoubleClicked(int,int)),	this, SLOT(onPropertiesRecipe_PROD(void)));
	QObject::connect(ui.pushButtonRemoveRecipe_PROD,	    SIGNAL(clicked()),	this, SLOT(onRemoveRecipe_PROD(void)));

	// Connect signals: Log
    QObject::connect(ui.comboBoxLogFile,	    SIGNAL(activated(int)),	this, SLOT(loadLogTabFile(void)));
    QObject::connect(ui.pushButtonRefreshLogView,   SIGNAL(clicked()),		this, SLOT(loadLogTabFile(void)));

	// Connect signals: Groups
	QObject::connect(ui.pushButtonAddGroup,	    SIGNAL(clicked()),	this, SLOT(onAddGroup(void)));
	QObject::connect(ui.pushButtonPropertiesGroup, SIGNAL(clicked()),	this, SLOT(onPropertiesGroup(void)));
	QObject::connect(ui.tableWidgetGroups,	    SIGNAL(cellDoubleClicked(int,int)),	this, SLOT(onPropertiesGroup(void)));
	QObject::connect(ui.pushButtonRemoveGroup,	    SIGNAL(clicked()),	this, SLOT(onRemoveGroup(void)));

    // Connect signals: Users
    QObject::connect(ui.pushButtonAddUser,	    SIGNAL(clicked()),	this, SLOT(onAddUser(void)));
    QObject::connect(ui.pushButtonPropertiesUser, SIGNAL(clicked()),	this, SLOT(onPropertiesUser(void)));
    QObject::connect(ui.tableWidgetUsers,	    SIGNAL(cellDoubleClicked(int,int)),	this, SLOT(onPropertiesUser(void)));
    QObject::connect(ui.pushButtonRemoveUser,	    SIGNAL(clicked()),	this, SLOT(onRemoveUser(void)));

	// Connect signals: Admin tab
	QObject::connect(ui.spinBoxPPAT_Bin,	    SIGNAL(valueChanged(int)),	this, SLOT(onAdmin_PPAT(void)));
	QObject::connect(ui.spinBoxGPAT_Bin,	    SIGNAL(valueChanged(int)),	this, SLOT(onAdmin_GPAT(void)));
	QObject::connect(ui.spinBoxZPAT_Bin,	    SIGNAL(valueChanged(int)),	this, SLOT(onAdmin_ZPAT(void)));
}

//===============================================================================================
// Close signal...do save/cleanups
//===============================================================================================
void PatProdRecipe_Admin::closeEvent(QCloseEvent *e)
{
    // Ensure we logout and save data if needed
    onLogout();
}

//===============================================================================================
// Drag recipe profile
//===============================================================================================
void PatProdRecipe_Admin::dragEnterEvent(QDragEnterEvent *e)
{
	// Accept Drag if files list dragged over.
	if(e->provides("text/uri-list"))
		e->acceptProposedAction();
}

//===============================================================================================
// Droping recipe profile
//===============================================================================================
void PatProdRecipe_Admin::dropEvent(QDropEvent *e)
{
	// Ignore drops if not logged!
	if(m_bLogged == false)
		return;

	// Extract list of files dragged over...
	if(!e->provides("text/uri-list"))
	{
		// No files dropped...ignore drag&drop.
		e->ignore();
		return;
	}

	QString		strFileName;
	QStringList strFileList;
	QList<QUrl> lstUrls = e->mimeData()->urls();

	for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
	{
		strFileName = lstUrls.at(nUrl).toLocalFile();

		if (!strFileName.isEmpty())
			strFileList << strFileName;
	}

	if(strFileList.count() <= 0)
	{
		// Items dropped are not regular files...ignore.
		e->ignore();
		return;
	}

	// Accept drag & drop
	e->acceptProposedAction();

	// Check if ENG or PROD recipe file
	QString strFile = strFileList.first();

	// Check which tab is active
	switch(ui.tabWidget->currentIndex())
	{
		case 0:
			onAddRecipe_ENG(strFile);
			break;
		case 1:
			onAddRecipe_PROD(strFile);
			break;
	}
}

//===============================================================================================
// Login/Logout
//===============================================================================================
void PatProdRecipe_Admin::onLogin(void)
{
    if(m_bLogged == false)
    {
		// Load list of recipes and their settings + valid Users
		loadSettings();

		m_cUser.strUserName = ui.lineEditUsername->text();
		m_cUser.strPassword = ui.lineEditPassword->text();

		if(checkPassword(m_cUser) == false)
		{
			QMessageBox::warning(this,"Error","Invalid login!",QMessageBox::Ok);
			return;
		}

		// Resize screen to bigger area
		resize(954,481);

		// Update GUI based on user rights
		switch(m_cUser.iUserType)
		{
			case USER_RIGHTS_ENG: // Read-only user
			// Remove all icons to edit/create entries.
			ui.tableWidgetUsers->hide();
			ui.pushButtonAddUser->hide();
			ui.pushButtonPropertiesUser->hide();
			ui.pushButtonRemoveUser->hide();
			ui.groupBox_Binning->setEnabled(false);
			break;
			case USER_RIGHTS_PROD: // Read-Write user

			ui.tableWidgetUsers->hide();
			ui.pushButtonAddUser->hide();
			ui.pushButtonPropertiesUser->hide();
			ui.pushButtonRemoveUser->hide();
			ui.groupBox_Binning->setEnabled(false);
			break;
			case USER_RIGHTS_ADMIN: // Admin user
			ui.tableWidgetUsers->show();
			ui.pushButtonAddUser->show();
			ui.pushButtonPropertiesUser->show();
			ui.pushButtonRemoveUser->show();
			ui.groupBox_Binning->setEnabled(true);
			break;
		}
		// User now logged-in
		m_bLogged = true;

		// Update Function for login/logout button
		ui.pushButtonLogin->setText("Logout");

		// Update GUI.
		ui.tabWidget->show();
		ui.lineEditUsername->setEnabled(false);
		ui.lineEditPassword->setEnabled(false);
		ui.comboBoxGroupList->show();
		ui.lineEditRecipeFilter->show();

		// Load Log tab
		loadLogTab();

		// Refresh Recipe list
		onSelectGroup();

		// Log info to log file
		logEvent("Login",m_cUser.strUserName);
    }
    else
    {
		// Log info to log file
		logEvent("Logout",m_cUser.strUserName);

		// User login-out
		m_bLogged = false;

		// Update Function for login/logout button
		ui.pushButtonLogin->setText("Login");

		// Update GUI.
		ui.tabWidget->hide();
		ui.lineEditUsername->setEnabled(true);
		ui.lineEditPassword->setEnabled(true);
		ui.comboBoxGroupList->hide();
		ui.lineEditRecipeFilter->hide();

		// Set focus on login username
		ui.lineEditUsername->setFocus();

		// Logout process (save settings, etc)
		onLogout();
	}
}

//===============================================================================================
// Logout
//===============================================================================================
void PatProdRecipe_Admin::onLogout(void)
{
	// Save Admin info
	saveSettings_Admin();

	// Resize GUI
	resize(365,125);
}

//===============================================================================================
// Load log tab with list of logs available
//===============================================================================================
void PatProdRecipe_Admin::loadLogTab(void)
{
    // Point to folder with log files.
    QString strPath = m_strProdPath + "/log";
    QDir cDir(strPath,"*.log", QDir::Time,QDir::Files);
    QStringList strLogList = cDir.entryList();
    ui.comboBoxLogFile->clear();
    int iIndex;
    for(iIndex=0;iIndex < strLogList.count(); iIndex++)
	ui.comboBoxLogFile->addItem(strLogList.at(iIndex));

    // Select first item in list
    ui.comboBoxLogFile->setCurrentIndex(0);

    // If only one selection available, select it!
    if(strLogList.count() == 1)
		loadLogTabFile();
}

//===============================================================================================
// Load log tab with list of logs available
//===============================================================================================
void PatProdRecipe_Admin::loadLogTabFile(void)
{
    QString strFile =  m_strProdPath + "/log/" + ui.comboBoxLogFile->currentText();

    // Read file, and load contents in Editfield
    ui.textEditLogFile->clear();
    QFile cFile(strFile);
    if(!cFile.open(QIODevice::ReadOnly | QIODevice::Text))
	return;

    QTextStream hLog(&cFile);
    QString strLine;

    do
    {
	strLine = hLog.readLine();
	ui.textEditLogFile->append(strLine);
    }
    while(!hLog.atEnd());
}

//===============================================================================================
// Load list of production recipes (and their settings)
//===============================================================================================
bool PatProdRecipe_Admin::loadSettings(void)
{    
	// Load Path details (eng + prod folders)
	loadSettings_Path();

	// Get Version of the data file to load (so to check if version is acceptable)
	loadSettings_VersionID();

	// Load ENG recipes tab
	loadSettings_ENG_Recipes();

	// Load PROD recipes tab
	loadSettings_PROD_Recipes();

	// Load Groups tab
	loadSettings_Groups();

    // Load Users tab
    loadSettings_Users();

	// Load Admin tab
	loadSettings_Admin();

    m_bChanges = false;

	// Check if sucessfully loaded mandatory config file
	if(m_strEngPath.isEmpty() || m_strProdPath.isEmpty())
	{
		QMessageBox::warning(this,"Error","Missing 'pat_server.ini' file in your application folder!",QMessageBox::Ok);
		return false;
	}
	return true;
}

//===============================================================================================
// Load list of production recipes (and their settings)
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_Path(void)
{
	// Open .ini settings file.
	QSettings settings(m_strConfigFile,QSettings::IniFormat);

	// Folder where engineers release Recipe (top-level recipe path)
	m_strEngPath  = settings.value("global/eng_path", "").toString();
	m_strProdPath = settings.value("global/prod_path", "").toString();
	m_strFlowsAvailable = settings.value("global/flows", "EWS1").toString().split(" ");

	// File to hold list recipes in production, version, etc...
	m_strENG_RecipesIni = m_strEngPath + "/.pat_eng.ini";
	m_strPROD_RecipesIni = m_strProdPath + "/.pat_prod.ini";
}

//===============================================================================================
// Get Version # of settings files (ENG & PROD)
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_VersionID(void)
{
	// Open ENG Recipes .ini settings file + Read file version
	QSettings settings_ENG(m_strENG_RecipesIni,QSettings::IniFormat);
	m_lfENG_SettingsVersion = settings_ENG.value("global/version_id", "1.0").toDouble();

	// Open PROD Recipes .ini settings file + Read file version
	QSettings settings_PROD(m_strPROD_RecipesIni,QSettings::IniFormat);
	m_lfPROD_SettingsVersion = settings_PROD.value("global/version_id", "1.0").toDouble();

	if((m_lfENG_SettingsVersion > VERSION_ID) || (m_lfPROD_SettingsVersion > VERSION_ID))
	{
		// Version too old...ask what to do?
		QMessageBox::warning(this,"Warning","You are running an old Recipe Admin version.\nYou need to update.");
		exit(EXIT_FAILURE);
	}
}

// Get All PAT bins from GUI
void PatProdRecipe_Admin::readAdmin_gui(void)
{
	m_iPPAT_Bin  = ui.spinBoxPPAT_Bin->value();
	m_iGPAT_Bin  = ui.spinBoxGPAT_Bin->value();
	m_iZPAT_Bin  = ui.spinBoxZPAT_Bin->value();
}

// Set All PAT bins in GUI
void PatProdRecipe_Admin::writeAdmin_gui(void)
{
	ui.spinBoxPPAT_Bin->setValue(m_iPPAT_Bin);
	ui.spinBoxGPAT_Bin->setValue(m_iGPAT_Bin);
	ui.spinBoxZPAT_Bin->setValue(m_iZPAT_Bin);
}

// PPAT GUI field changed
void PatProdRecipe_Admin::onAdmin_PPAT(void)
{
	m_iPPAT_Bin  = ui.spinBoxPPAT_Bin->value();
}

// GPAT GUI field changed
void PatProdRecipe_Admin::onAdmin_GPAT(void)
{
	m_iGPAT_Bin  = ui.spinBoxGPAT_Bin->value();
}

// ZPAT GUI field changed
void PatProdRecipe_Admin::onAdmin_ZPAT(void)
{
	m_iZPAT_Bin  = ui.spinBoxZPAT_Bin->value();
}

//===============================================================================================
// Load Admin data
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_Admin(void)
{
	// Open .ini settings file.
	QSettings settings(m_strConfigFile,QSettings::IniFormat);

	// Get Default PAT Bins
	m_iPPAT_Bin  = settings.value("admin/ppat_bin", "81").toInt();
	m_iGPAT_Bin  = settings.value("admin/gpat_bin", "80").toInt();
	m_iZPAT_Bin  = settings.value("admin/zpat_bin", "87").toInt();

	// Load into GUI
	writeAdmin_gui();
}


//===============================================================================================
// Save Admin data
//===============================================================================================
void PatProdRecipe_Admin::saveSettings_Admin(void)
{
	// Open .ini settings file.
	QSettings settings(m_strConfigFile,QSettings::IniFormat);

	// Read GUI PAT Bins
	readAdmin_gui();

	// Ensure HBINS are all different!
	bool bBadBin=false;
	if(m_iPPAT_Bin == m_iGPAT_Bin)
		bBadBin = true;
	if(m_iPPAT_Bin == m_iZPAT_Bin)
		bBadBin = true;
	if(m_iGPAT_Bin == m_iZPAT_Bin)
		bBadBin = true;

	if(bBadBin)
	{
		// Some HBINS are identical, reject
		QString strErrorMessage = "PPAT, GPAT and ZPAT BINS\nmust all be different!\nFor now, forced to 81,80,87";
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		m_iPPAT_Bin = 81;
		m_iGPAT_Bin = 80;
		m_iZPAT_Bin = 87;
	}
	// Save to disk.
	settings.setValue("admin/ppat_bin", m_iPPAT_Bin);
	settings.setValue("admin/gpat_bin", m_iGPAT_Bin);
	settings.setValue("admin/zpat_bin", m_iZPAT_Bin);

	// Application version used to create files (ENG & PROD)
	QSettings settings_ENG(m_strENG_RecipesIni,QSettings::IniFormat);
	settings_ENG.setValue("global/version_id", VERSION_ID);
	QSettings settings_PROD(m_strPROD_RecipesIni,QSettings::IniFormat);
	settings_PROD.setValue("global/version_id", VERSION_ID);
}

//===============================================================================================
// Log event to log file
// Log file: <production_folder>/log/MM-YYYY.log
//===============================================================================================
void PatProdRecipe_Admin::logEvent(QString strType,QString strString)
{
    // Create folder if doesn't exist
    QString strLogFile = m_strProdPath + "/log";
    QDir cDir;
    cDir.mkdir(strLogFile);
    strLogFile += "/" + QDate::currentDate().toString("MM-yyyy") + ".log";
    QFile cFile(strLogFile);
    if(!cFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	return;

    // Log message
    QTextStream hLog(&cFile);
    hLog << QDateTime::currentDateTime().toString("[dd-MM-yyyy hh:mm:ss] ") << strType << " - " << strString << endl;
}
