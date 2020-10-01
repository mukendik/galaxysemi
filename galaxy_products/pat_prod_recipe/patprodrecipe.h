#ifndef PATPRODRECIPE_H
#define PATPRODRECIPE_H

// Include
#include "ui_PatProdRecipe.h"
#include "ui_flow_entry.h"

// Low level classes (read/write recipes)
#include "../pat_prod_admin/patsrv_ini.h"
#include "../pat_prod_admin/prod_recipe_file.h"

// User rights
#define	USER_RIGHTS_READONLY	0
#define	USER_RIGHTS_READWRITE	1
#define	USER_RIGHTS_ADMIN		2

class PatProdRecipe : public QDialog
{
    Q_OBJECT

public:
    PatProdRecipe(QWidget *parent = 0);
	void	setStandalone(bool bStandalone);
	void	refreshGUI(void);	// Refresh GUI hide/show fields.
	bool	isAutoIncrement(void) { return (ui.checkBoxAutoIncrement->isChecked()); };
	QString	m_strProdRecipe;	// Holds full path to recipe created when closing Dialog
	void	setPatBins(int iPpat,int iGpat,int iZpat);
	void	setNewRecipe_ENG(QString strOldRecipe_ENG,QString strNewRecipe_ENG,QString strNewVersion);
	// Variables.
	int		m_iAutoIncrement;			// Auto-increlent recipe version on 'save'

private:
    // GUI
    Ui_PatProdRecipe ui;

    // Private functions
	void		logEvent(QString strType,QString strString);    // Log messages to log file
	void		loadSettings_Path(void);	// Load global paths: eng + prod recipes
	QStringList	loadGroupList(void);		// Load list of user groups (from server)
	QStringList	loadENG_RecipesList(QString strGroup);	// Load list of ENG recipes & versions.

	// Update .INI file
	int		getProductionRecipeVersion(QString strProduct);
	void	setProductionRecipeVersion(QString strProduct,int Version);
	bool	checkValidEntries(void);

    // Private variables
	QString	m_strUserRecipePath;		// PAth to recipe edited (used when Save button pressed)
	QString	m_strConfigFile;			// Holds path to .INI file (in application folder)
	bool	m_bChanges;					// True if changes made to settings (and need to be saved)
	bool	m_Standalone;				// True if application executed in stabdalone mode
	QString	m_strENG_Path;				// PAth to ENG folder on server
	QString	m_strPROD_Path;				// PAth to PROD folder on server
	QString	m_strENG_RecipesIni;		// Path to Server Engineering .INI file
	QString	m_strPROD_RecipesIni;		// Path to Server Production .INI file
	QString m_strRecipesIni;			// Holds list or Recipes in production
	QStringList m_strFlowsAvailable;	// Valid flows available
	int     m_iPPAT_Bin;			// PPAT overloading bin; -1 for no overload
	int     m_iGPAT_Bin;			// GPAT overloading bin; -1 for no overload
	int     m_iZPAT_Bin;			// ZPAT overloading bin; -1 for no overload

public slots:
	// GUI
	void	pushFlow_GUI(int iRow,CRecipeFlow cFlow);
	void	pullFlow_GUI(int iRow,CRecipeFlow &cFlow);
	void	pushRecipe_GUI(CPROD_Recipe cRecipe);		// Push recipe details to GUI
	void	pullRecipe_GUI(CPROD_Recipe &cRecipe);		// Pull recipe details from GUI

	void    newRecipe(void);	// New Recipe
	void    loadRecipe(void);	// Load Recipe
	void	loadRecipe(QString strRecipeFile);
	void    saveRecipe(void);	// Save Recipe
	void	abortRecipe(void);	// Abort recipe

	void    newFlow(void);		// New Flow
	void    editFlow(void);		// Edit flow
	void    deleteFlow(void);	// Delete flow
	void    flowUp(void);		// Move flow Up
	void    flowDown(void);		// Move flow Down

protected:
		void	closeEvent(QCloseEvent *);
		void	dragEnterEvent ( QDragEnterEvent * event );
		void	dropEvent(QDropEvent *event);

};

class FlowDefinition : public QDialog
{
	Q_OBJECT

public:
	FlowDefinition(QStringList strFlows,QStringList strEngRecipes,QWidget *parent = 0);
	void	push_GUI(CRecipeFlow cFlow);
	void	pull_GUI(CRecipeFlow &cFlow);

public slots:
	void    onOk(void);

private:
	// GUI
	Ui_FlowDefinition ui;
};

#endif

