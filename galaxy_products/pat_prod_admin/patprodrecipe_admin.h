
#ifndef PatProdRecipe_Admin_H
#define PatProdRecipe_Admin_H

#include "ui_PatProdRecipe_admin.h"
#include "prod_recipe_file.h"

// User rights
#define	USER_RIGHTS_ENG		0	// Engineering rights: only Upload recipes
#define	USER_RIGHTS_PROD	1	// Production rights: Upload+Enabled/Disable
#define	USER_RIGHTS_ADMIN	2	// Administrator: All rights

class   CENG_Recipe
{
    public:
	CENG_Recipe(QStringList strFlowsAvailable,QStringList strGroupsAvailable);	// Constructor
	QStringList strFlows;		// List of valid flows to display
	QStringList	strGroups;		// List of valid groups

	QString	strGroupName;		// Group name to which this recipe belongs
    QString strRecipeName;      // Recipe file name (without version# nor Galaxy suffix string)
	QString strRecipeFile;		// Recipe file name
	QString strComment;			// Recipe comments
    int     iEnabled;           // 0=recipe not active in production, 1=active
	int     iEng_Version;       // Recipe Engineering version
	int     iEng_Build;			// Recipe build
	int     iProd_Version;      // Recipe Production version
};

class   CUserInfo
{
    public:
    CUserInfo();		// Constructor
    QString crypt(QString);	// Encrypt string
    QString decrypt(QString);	// Decrypt string

    QString strUserName;	// User name
    QString strPassword;	// User password
    int     iUserType;		// User priviledge: 0= read-only, 1 = admin.
};


class   CGroupInfo
{
	public:
	CGroupInfo();		// Constructor

	QString strGroupName;	// Group name
};

class PatProdRecipe_Admin : public QDialog
{
    Q_OBJECT

public:
	PatProdRecipe_Admin(QWidget *parent = 0);

private:
    // GUI
	Ui_PatProdRecipe_Admin ui;

    // Private functions
	void	loadSettings_VersionID(void);	// Check file version compatible with Executable
    void    logEvent(QString strType,QString strString);    // Log messages to log file
	void	loadSettings_Path(void);	// Load global paths: eng + prod recipes

    // Recipes
	void    loadSettings_ENG_Recipes(void);
	void    loadSettings_PROD_Recipes(void);
	void    saveSettings_ENG_Recipe(int iRow,bool bDeleteRecipe);
	void    saveSettings_PROD_Recipe(int iRow,bool bDeleteRecipe,bool bDeleteReleasedRecipe);
	bool    writeENG_RecipeList_gui(int iRow,CENG_Recipe cEntry);	// Write ENG recipe profile to GUI (table)
	bool    readENG_RecipeList_gui(int iRow,CENG_Recipe &cEntry);	// Read ENG recipe profile from GUI (table)
	bool    writePROD_RecipeList_gui(int iRow,CPROD_Recipe cEntry);	// Write ENG recipe profile to GUI (table)
	bool    readPROD_RecipeList_gui(int iRow,CPROD_Recipe &cEntry);	// Read ENG recipe profile from GUI (table)

	// Groups
	void    loadSettings_Groups(void);
	void    saveSettings_Groups(void);
	bool    writeGroupList_gui(int iRow,CGroupInfo cEntry);	// Write group profile to GUI (table)
	bool    readGroupList_gui(int iRow,CGroupInfo &cEntry);	// Read group profile from GUI (table)

	// Users
	void    loadSettings_Users(void);
	void    saveSettings_Users(void);
	bool    checkPassword(CUserInfo &cUser);
	bool    writeUserList_gui(int iRow,CUserInfo cEntry);	// Write user profile to GUI (table)
	bool    readUserList_gui(int iRow,CUserInfo &cEntry);	// Read user profile from GUI (table)

	// Admin
	void	readAdmin_gui(void);
	void	writeAdmin_gui(void);
	void    loadSettings_Admin(void);
	void    saveSettings_Admin(void);

	// Update .INI file
	int		getProductionRecipeVersion(QString strProduct);
	void	setProductionRecipeVersion(QString strProduct,int Version);

    // Private variables
    QString	m_strConfigFile;// Holds path to .INI file (in application folder)
    bool	m_bLogged;	// True if user logged in
    bool	m_bChanges;	// True if changes made to settings (and need to be saved)
	double	m_lfENG_SettingsVersion; // ENG Settings File version ID (X.Y)
	double	m_lfPROD_SettingsVersion; // PROD Settings File version ID (X.Y)
    QString	m_strEngPath;	// Path where engineers post their recipe files
    QString	m_strProdPath;	// Path where recipes are stored for production usage
	QString m_strENG_RecipesIni;// Holds list or ENG Recipes in production
	QString m_strPROD_RecipesIni;// Holds list or PROD Recipes in production
	QStringList m_strFlowsAvailable;	// Valid flows available
    CUserInfo	m_cUser;	// Loging details
	int		m_iPPAT_Bin;	// PPAT Bin#
	int		m_iGPAT_Bin;	// GPAT Bin#
	int		m_iZPAT_Bin;	// ZPAT Bin#

public slots:
	void	onLogin(void);
	void	onLogout(void);
	bool    loadSettings(void); // Load list of production recipes & their settings (from disk)

	// Contextual menu
	void	onContextualMenu(const QPoint & pos);

	// ENG Recipe tab
	void	onAddRecipe_ENG(void);
	void	onAddRecipe_ENG(QString strRecipeFile);
	void	onPropertiesRecipe_ENG(void);
	void	onRemoveRecipe_ENG(void);

	// PROD Recipe tab
	void	onNewRecipe_PROD(void);
	void	onAddRecipe_PROD(void);
	void	onAddRecipe_PROD(QString strRecipeFile,bool bForceOverwrite=false);
	void	onEditRecipe_PROD(void);
	void	onEditRecipe_PROD(QString strOldRecipe_ENG,QString strNewRecipe_ENG,QString strNewVersion);
	void	onPropertiesRecipe_PROD(void);
	void	onRemoveRecipe_PROD(void);

	void	onSelectGroup();

	// Log tab
	void    loadLogTab(void);	// Load Log tab with list of log files available
	void    loadLogTabFile(void);   // Show selected .log file

	// Groups tab
	void	onAddGroup(void);
	void	onPropertiesGroup(void);
	void	onRemoveGroup(void);
	void	refreshGroupsListCombo(void);
	QStringList strlGetGroupsList(void);

	// Users tab
	void	onAddUser(void);
	void	onPropertiesUser(void);
	void	onRemoveUser(void);

	// Admin tab
	void	onAdmin_PPAT(void);
	void	onAdmin_GPAT(void);
	void	onAdmin_ZPAT(void);

protected:
    	void closeEvent(QCloseEvent *);
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *);
};

#endif

