#include <QtGui>
#include <QGroupBox>
#include "patprodrecipe.h"


extern QApplication app;

//===============================================================================================
// Constructor
//===============================================================================================
PatProdRecipe::PatProdRecipe(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
	setAcceptDrops(true);

	// Application version
	this->setWindowTitle(QApplication::translate("PatProdRecipe", "Edit PROD Recipe ", 0, QApplication::UnicodeUTF8));

	// Disable sorting in Flow table as order is key: it's execution order!
	ui.tableWidgetFlows->setSortingEnabled(false);

    // Reset variables
    m_bChanges = false;	    // true if edits done
	m_Standalone = true;	// true if Dialog box executed as standalone GUI
	m_iPPAT_Bin = -1;
	m_iGPAT_Bin = -1;
	m_iZPAT_Bin = -1;

    // Build path to configuration file (.INI format)
    m_strConfigFile = app.applicationDirPath();
	m_strConfigFile += "/pat_server.ini";

	// Build user HOME Folder path.
	QDir cDir;
	m_strUserRecipePath = cDir.homePath();

	// Load setup file
	loadSettings_Path();

	// Connect signals: Recipe
	QObject::connect(ui.pushButtonNewRecipe, SIGNAL(clicked()),	this, SLOT(newRecipe(void)));
	QObject::connect(ui.pushButtonLoadRecipe, SIGNAL(clicked()),	this, SLOT(loadRecipe(void)));
	QObject::connect(ui.pushButtonSaveRecipe, SIGNAL(clicked()),	this, SLOT(saveRecipe(void)));

	// Connect signals: Flows
	QObject::connect(ui.pushButtonNewFlow, SIGNAL(clicked()),	this, SLOT(newFlow(void)));
	QObject::connect(ui.pushButtonRemoveFlow, SIGNAL(clicked()),	this, SLOT(deleteFlow(void)));
	QObject::connect(ui.pushButtonPropertiesFlow, SIGNAL(clicked()),	this, SLOT(editFlow(void)));
	QObject::connect(ui.pushButtonUp, SIGNAL(clicked()),	this, SLOT(flowUp(void)));
	QObject::connect(ui.pushButtonDown, SIGNAL(clicked()),	this, SLOT(flowDown(void)));
	QObject::connect(ui.tableWidgetFlows,	    SIGNAL(cellDoubleClicked(int,int)),	this, SLOT(editFlow(void)));

	// Ok / Cancel buttons
	QObject::connect(ui.pushButtonOk, SIGNAL(clicked()),	this, SLOT(saveRecipe()));
	QObject::connect(ui.pushButtonCancel, SIGNAL(clicked()),	this, SLOT(abortRecipe()));

	refreshGUI();

	// PAT Bins are read-only (only defined by Administrator)
	ui.groupBoxPatBins->setEnabled(false);

	// Load group list from server
	// Reset group list combobox
	ui.comboBoxGroup->clear();
	ui.comboBoxGroup->addItems(loadGroupList());

	// Start from clean page.
	newRecipe();
}

//===============================================================================================
// Support File drop.
//===============================================================================================
void PatProdRecipe::dragEnterEvent(QDragEnterEvent *e)
{
	// Accept Drag if files list dragged over.
	if(e->provides("text/uri-list"))
		e->acceptProposedAction();
}

//===============================================================================================
// Support File drop.
//===============================================================================================
void PatProdRecipe::dropEvent(QDropEvent *e)
{
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

	loadRecipe(strFileList.first());
}

//===============================================================================================
// Close signal...do save/cleanups
//===============================================================================================
void PatProdRecipe::closeEvent(QCloseEvent *e)
{
	// Save data if edits done
	if(m_bChanges)
		saveRecipe();
}


//===============================================================================================
// Support File drop.
//===============================================================================================
void PatProdRecipe::setStandalone(bool bStandalone)
{
	m_Standalone = bStandalone;
	refreshGUI();
}

//===============================================================================================
// Set defaults PAT Bins
//===============================================================================================
void PatProdRecipe::setPatBins(int iPpat,int iGpat,int iZpat)
{
	m_iPPAT_Bin = iPpat;
	m_iGPAT_Bin = iGpat;
	m_iZPAT_Bin = iZpat;

	ui.spinBoxPPAT_Bin->setValue(m_iPPAT_Bin);
	ui.spinBoxGPAT_Bin->setValue(m_iGPAT_Bin);
	ui.spinBoxZPAT_Bin->setValue(m_iZPAT_Bin);
}

//===============================================================================================
// New ENG recipe name to used in case of recipe update
//===============================================================================================
void PatProdRecipe::setNewRecipe_ENG(QString strOldRecipe_ENG,QString strNewRecipe_ENG,QString strNewVersion)
{
	// Scan the PROD recipe flows, and update ENG recipes that need to.

	int	iRow;

	for(iRow=0; iRow < ui.tableWidgetFlows->rowCount(); iRow++)
	{
		// Read Flow info from GUI list
		CRecipeFlow		cFlow;
		pullFlow_GUI(iRow,cFlow);
		if(cFlow.m_strENG_RecipeFile == strOldRecipe_ENG)
		{
			cFlow.m_strENG_RecipeFile = strNewRecipe_ENG;
			cFlow.m_strENG_Version = strNewVersion;
		}

		// Write update flow, with new recipe info.
		pushFlow_GUI(iRow,cFlow);
	}
}

//===============================================================================================
// Refresh GUI hide/show fields.
//===============================================================================================
void PatProdRecipe::refreshGUI(void)
{
	if(m_Standalone)
	{
		// Show create/save icons
		ui.pushButtonNewRecipe->show();
		ui.pushButtonLoadRecipe->show();
		ui.pushButtonSaveRecipe->show();

		ui.pushButtonOk->hide();
		ui.pushButtonCancel->hide();
	}
	else
	{
		// Hide create/save icons
		ui.pushButtonNewRecipe->hide();
		ui.pushButtonLoadRecipe->hide();
		ui.pushButtonSaveRecipe->hide();

		ui.pushButtonOk->show();
		ui.pushButtonCancel->show();
	}

	// GUI update
	if(m_iAutoIncrement)
		ui.checkBoxAutoIncrement->setChecked(Qt::Checked);
	else
		ui.checkBoxAutoIncrement->setChecked(Qt::Unchecked);
}

//===============================================================================================
// Load list of production recipes (and their settings)
//===============================================================================================
void PatProdRecipe::loadSettings_Path(void)
{
	// Open .ini settings file.
	QSettings settings(m_strConfigFile,QSettings::IniFormat);

	// Folder where engineers release Recipe (top-level recipe path)
	m_strENG_Path  = settings.value("global/eng_path", "").toString();
	m_strENG_RecipesIni = m_strENG_Path + PATSRV_ENG_INI_FILE;
	m_strPROD_Path = settings.value("global/prod_path", "").toString();
	m_iAutoIncrement = settings.value("global/auto_increment", "0").toInt();
	m_strPROD_RecipesIni = m_strPROD_Path + PATSRV_PROD_INI_FILE;
	m_strFlowsAvailable = settings.value("global/flows", "EWS1").toString().split(" ");
}

//===============================================================================================
// New recipe
//===============================================================================================
void PatProdRecipe::newRecipe(void)
{
	CPROD_Recipe cRecipe;

	// Load GUI with Recipe details.
	pushRecipe_GUI(cRecipe);

	// Select first group in list
	ui.comboBoxGroup->setCurrentIndex(0);

	m_bChanges = true;
}

//===============================================================================================
// Load recipe from disk
//===============================================================================================
void PatProdRecipe::loadRecipe(QString strRecipeFile)
{
	QString			strErrorMessage;
	CPROD_Recipe	cRecipe;
	if(cRecipe.loadRecipe(strRecipeFile,strErrorMessage) == false)
	{
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Load GUI with Recipe details.
	pushRecipe_GUI(cRecipe);

	// Update path to recipe
	if(m_Standalone)
	{
		// Application standalone
		m_strUserRecipePath = strRecipeFile;
	}
	else
	{
		// Application called from 'Admin' tool
		QDir cDir;
		m_strUserRecipePath = cDir.homePath();
	}

	m_bChanges = false;
}

//===============================================================================================
// Load recipe to disk
//===============================================================================================
void PatProdRecipe::loadRecipe(void)
{
	// Les user select a recipe file
	QString	strErrorMessage;
	QString strPROD_RecipeFile = QFileDialog::getOpenFileName(this, "Select PROD recipe",m_strUserRecipePath,"Recipes (*.csv);;All files (*)");

	// Open file and ensures it is a valid recipe file!
	QFile f(strPROD_RecipeFile);
	if(f.exists() == false)
		return;	// File doesn't exist...quietly return!

	if(!f.open(QIODevice::ReadOnly))
	{
		strErrorMessage = "*PAT Error* PROD Recipe file access denied:" + strPROD_RecipeFile;
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Load selected PROD recipe
	loadRecipe(strPROD_RecipeFile);
}

//===============================================================================================
// Checks if valid entries...or reject settings!
//===============================================================================================
bool PatProdRecipe::checkValidEntries(void)
{
	// Get GUI details
	QString	strErrorMessage;
	CPROD_Recipe cRecipe;
	pullRecipe_GUI(cRecipe);

	// Ensure HBINS are all different!
	bool bBadBin=false;
	if((cRecipe.m_iPPAT_Bin >= 0) && (cRecipe.m_iGPAT_Bin >= 0) && (cRecipe.m_iPPAT_Bin == cRecipe.m_iGPAT_Bin))
		bBadBin = true;
	if((cRecipe.m_iPPAT_Bin >= 0) && (cRecipe.m_iZPAT_Bin >= 0) && (cRecipe.m_iPPAT_Bin == cRecipe.m_iZPAT_Bin))
		bBadBin = true;
	if((cRecipe.m_iGPAT_Bin >= 0) && (cRecipe.m_iZPAT_Bin >= 0) && (cRecipe.m_iGPAT_Bin == cRecipe.m_iZPAT_Bin))
		bBadBin = true;
	if(bBadBin)
	{
		// Some HBINS are identical, reject
		strErrorMessage = "PPAT, GPAT and ZPAT BINS\nmust all be different!";
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return false;
	}

	// If no recipe name defined, just ignore 'Save' function
	if(cRecipe.m_strRecipeName.isEmpty())
	{
		strErrorMessage = "Missing recipe name!";
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return false;
	}

	// If NO Flow defined, reject 'Save'; we need at least ONE Flow defined
	if(cRecipe.m_cFlows.count() <= 0)
	{
		strErrorMessage = "No flow defined!";
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return false;
	}

	// Success
	return true;
}

//===============================================================================================
// Save recipe to disk
//===============================================================================================
void PatProdRecipe::saveRecipe(void)
{
	// Check if valid entries
	if(checkValidEntries() == false)
		return;

	// Get GUI details
	QString	strErrorMessage;
	CPROD_Recipe cRecipe;
	pullRecipe_GUI(cRecipe);

	// Check if auto-increment on save?
	if(ui.checkBoxAutoIncrement->isChecked())
		cRecipe.m_iBuild++;

	QString strPROD_RecipeFile;
	QFileInfo	cFileInfo(m_strUserRecipePath);
	strPROD_RecipeFile = cRecipe.m_strRecipeName + "_Version" + QString::number(cRecipe.m_iVersion) + "." + QString::number(cRecipe.m_iBuild) + ".csv";

	if(m_Standalone)
	{
		m_strUserRecipePath= cFileInfo.absolutePath() + "/" + strPROD_RecipeFile;
		strPROD_RecipeFile = QFileDialog::getSaveFileName(this, "Save PROD recipe",m_strUserRecipePath,"Recipes (*.csv);;All files (*)");
		if(strPROD_RecipeFile.isEmpty())
			return;	// User abort
	}
	else
	{
		// Dialog called from the 'Admin' tool: so do NOT ask to confirm file name creation, use it!
		strPROD_RecipeFile= m_strUserRecipePath + "/" + strPROD_RecipeFile;
		if(QFile::exists(strPROD_RecipeFile))
		{
			if(QMessageBox::warning(this,"Warning","This recipe version was already used. Overwrite it?",QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
				return;
			// Remove file to overwrite.
			QFile::remove(strPROD_RecipeFile);
		}
	}

	// Extract Recipe name
	cFileInfo.setFile(strPROD_RecipeFile);
	cRecipe.m_strRecipeFile = cFileInfo.completeBaseName() + "." + cFileInfo.suffix();

	// Write recipe to disk
	if(cRecipe.saveRecipe(strPROD_RecipeFile,strErrorMessage) == false)
	{
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Reload recipe (so to show new release# in case auto-increment activated
	loadRecipe(strPROD_RecipeFile);

	// Save full path to file created
	m_strProdRecipe = strPROD_RecipeFile;

	m_bChanges = false;

	// If caller is Admin dialog box, then close this dialogbox!
	if(m_Standalone == false)
		done(1);
}

//===============================================================================================
// Abort recipe edition
//===============================================================================================
void PatProdRecipe::abortRecipe(void)
{
	done(-1);
}

//===============================================================================================
// New Flow
//===============================================================================================
void PatProdRecipe::newFlow(void)
{
	QString strGroup = ui.comboBoxGroup->currentText();
	FlowDefinition cFlowDef(m_strFlowsAvailable,loadENG_RecipesList(strGroup));

	// Flow editor
	if(cFlowDef.exec() != 1)
		return;

	// Insert in GUI
	int iRowIndex = ui.tableWidgetFlows->rowCount();
	ui.tableWidgetFlows->setRowCount(iRowIndex+1);
	CRecipeFlow		cFlow;
	cFlowDef.pull_GUI(cFlow);
	pushFlow_GUI(iRowIndex,cFlow);

	// Update list sorting
	ui.tableWidgetFlows->sortItems(0);

	m_bChanges = true;
}

//===============================================================================================
// Edit flow
//===============================================================================================
void PatProdRecipe::editFlow(void)
{
	// Get current item selected
	int iRow = ui.tableWidgetFlows->currentRow();
	if(iRow < 0)
		return;

	// Read Flow info from GUI list
	CRecipeFlow		cFlow;
	pullFlow_GUI(iRow,cFlow);

	// Flow editor
	QString strGroup = ui.comboBoxGroup->currentText();
	FlowDefinition cFlowDef(m_strFlowsAvailable,loadENG_RecipesList(strGroup));
	cFlowDef.push_GUI(cFlow);
	if(cFlowDef.exec() != 1)
		return;

	// Update GUI
	cFlowDef.pull_GUI(cFlow);
	pushFlow_GUI(iRow,cFlow);

	m_bChanges = false;

}

//===============================================================================================
// Delete flow
//===============================================================================================
void PatProdRecipe::deleteFlow(void)
{
	// Get current item selected
	int iRow = ui.tableWidgetFlows->currentRow();
	if(iRow < 0)
		return;

	// Read Flow info from GUI list
	CRecipeFlow		cFlow;
	pullFlow_GUI(iRow,cFlow);

	// Request confirmation before delete...
	QString strText = "Delete flow '" + cFlow.m_strFlow + "' ?";
	if(QMessageBox::question(this,"Confirm delete",strText,QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	// Delete current line
	ui.tableWidgetFlows->removeRow(iRow);

	// Flag changes made
	m_bChanges = true;
}


//===============================================================================================
// Move flow UP
//===============================================================================================
void PatProdRecipe::flowUp(void)
{
	// Get current item selected
	int iRow = ui.tableWidgetFlows->currentRow();
	if(iRow < 1)
		return;	// Can only move Up second flow or higher!

	// Read GUI
	CRecipeFlow		cFlow1,cFlow2;
	pullFlow_GUI(iRow-1,cFlow1);
	pullFlow_GUI(iRow,cFlow2);

	// Push back, swapping entries
	pushFlow_GUI(iRow-1,cFlow2);
	pushFlow_GUI(iRow,cFlow1);

	// Move Focus down
	ui.tableWidgetFlows->setCurrentCell(iRow-1,0);
}

//===============================================================================================
// Move flow DOWN
//===============================================================================================
void PatProdRecipe::flowDown(void)
{
	// Get current item selected
	int iRow = ui.tableWidgetFlows->currentRow();
	if(iRow < 0 || (iRow+1 == ui.tableWidgetFlows->rowCount()))
		return;	// Can only move down any flow except last one!

	// Read GUI
	CRecipeFlow		cFlow1,cFlow2;
	pullFlow_GUI(iRow,cFlow1);
	pullFlow_GUI(iRow+1,cFlow2);

	// Push back, swapping entries
	pushFlow_GUI(iRow,cFlow2);
	pushFlow_GUI(iRow+1,cFlow1);

	// Move Focus down
	ui.tableWidgetFlows->setCurrentCell(iRow+1,0);
}


//===============================================================================================
// Push one line of Flow details to GUI
//===============================================================================================
void PatProdRecipe::pushFlow_GUI(int iRow,CRecipeFlow cFlow)
{
	QTableWidgetItem	*pItem;
	int					iCol=0;

	// Flow
	pItem = new QTableWidgetItem(cFlow.m_strFlow);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetFlows->setItem(iRow, iCol, pItem);
	iCol++;

	// Source
	pItem = new QTableWidgetItem(cFlow.strSourceName());
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetFlows->setItem(iRow, iCol, pItem);
	iCol++;

	// Good HBins
	pItem = new QTableWidgetItem(cFlow.m_strGoodHbins);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetFlows->setItem(iRow, iCol, pItem);
	iCol++;

	// ENG Recipe name
	pItem = new QTableWidgetItem(cFlow.m_strENG_RecipeName);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetFlows->setItem(iRow, iCol, pItem);
	iCol++;

	// ENG Recipe Version
	pItem = new QTableWidgetItem(cFlow.m_strENG_Version);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetFlows->setItem(iRow, iCol, pItem);
	iCol++;

	// ENG Recipe File
	pItem = new QTableWidgetItem(cFlow.m_strENG_RecipeFile);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetFlows->setItem(iRow, iCol, pItem);
	iCol++;
}

//===============================================================================================
// Pull one line of Flow details to GUI
//===============================================================================================
void PatProdRecipe::pullFlow_GUI(int iRow,CRecipeFlow &cFlow)
{
	int					iCol=0;

	// Flow
	cFlow.m_strFlow = ui.tableWidgetFlows->item(iRow, iCol)->text();
	iCol++;

	// Data Sources
	cFlow.m_iDataSources = cFlow.iSourceID(ui.tableWidgetFlows->item(iRow, iCol)->text());
	iCol++;

	// Good HBins
	cFlow.m_strGoodHbins = ui.tableWidgetFlows->item(iRow, iCol)->text();
	iCol++;

	// ENG Recipe name
	cFlow.m_strENG_RecipeName = ui.tableWidgetFlows->item(iRow, iCol)->text();
	iCol++;

	// ENG Recipe Version
	cFlow.m_strENG_Version = ui.tableWidgetFlows->item(iRow, iCol)->text();
	iCol++;

	// ENG Recipe file
	cFlow.m_strENG_RecipeFile = ui.tableWidgetFlows->item(iRow, iCol)->text();
	iCol++;
}

//===============================================================================================
// Push recipe details to GUI
//===============================================================================================
void PatProdRecipe::pushRecipe_GUI(CPROD_Recipe cRecipe)
{
	ui.comboBoxGroup->setCurrentIndex(ui.comboBoxGroup->findText(cRecipe.m_strGroup));

	ui.lineEditRecipeName->setText(cRecipe.m_strRecipeName);
	ui.lineEditComment->setText(cRecipe.m_strComment);
	ui.lineEditRecipeVersion->setText(QString::number(cRecipe.m_iVersion)+"." + QString::number(cRecipe.m_iBuild));
	ui.spinBoxPPAT_Bin->setValue(cRecipe.m_iPPAT_Bin);
	ui.spinBoxGPAT_Bin->setValue(cRecipe.m_iGPAT_Bin);
	ui.spinBoxZPAT_Bin->setValue(cRecipe.m_iZPAT_Bin);
	ui.comboBoxStdf->setCurrentIndex(cRecipe.m_iStdfOutput);

	// Flows
	CRecipeFlow		cFlow;
	int				iRow,iCol;
	ui.tableWidgetFlows->setRowCount(cRecipe.m_cFlows.count());

	for(iRow = 0; iRow < cRecipe.m_cFlows.count();iRow++)
	{
		iCol=0;
		cFlow = cRecipe.m_cFlows.at(iRow);

		// Fill One Flow Table line
		pushFlow_GUI(iRow,cFlow);
	}
}

//===============================================================================================
// Pull recipe details from GUI
//===============================================================================================
void PatProdRecipe::pullRecipe_GUI(CPROD_Recipe &cRecipe)
{
	QString strString;
	bool	bOk;

	cRecipe.m_strGroup = ui.comboBoxGroup->currentText();
	cRecipe.m_strRecipeName = ui.lineEditRecipeName->text();
	cRecipe.m_strComment = ui.lineEditComment->text();

	// Version
	strString = ui.lineEditRecipeVersion->text();
	cRecipe.m_iVersion = strString.section('.',0,0).toInt(&bOk);
	if(!bOk)
		cRecipe.m_iVersion = 1;
	cRecipe.m_iBuild = strString.section('.',1,1).toInt(&bOk);
	if(!bOk)
		cRecipe.m_iBuild = 0;

	cRecipe.m_iPPAT_Bin = ui.spinBoxPPAT_Bin->value();
	cRecipe.m_iGPAT_Bin = ui.spinBoxGPAT_Bin->value();
	cRecipe.m_iZPAT_Bin = ui.spinBoxZPAT_Bin->value();
	cRecipe.m_iStdfOutput = ui.comboBoxStdf->currentIndex();

	// Flows
	CRecipeFlow		cFlow;
	int				iRow;
	cRecipe.m_cFlows.clear();

	for(iRow = 0; iRow < ui.tableWidgetFlows->rowCount();iRow++)
	{
		// Get one Flow Table line
		pullFlow_GUI(iRow,cFlow);

		// Add to data array
		cRecipe.m_cFlows.append(cFlow);
	}
}

//===============================================================================================
// Load list of Groups (and their settings)
//===============================================================================================
QStringList PatProdRecipe::loadGroupList(void)
{
	QStringList	strGroupList;
	QString	    strString;
	int		    iEntry;

	// Open .ini groups file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

	// Read all entries
	int iTotalGroups = settings.value("global/total_groups", 0).toInt();
	for(iEntry = 0;iEntry < iTotalGroups; iEntry++)
	{
		// Seek to relevant Group: 'group_X'
		strString.sprintf("group_%d",iEntry);
		settings.beginGroup(strString);

		strGroupList += settings.value("name","").toString();

		// Close group seek.
		settings.endGroup();
	};

	// Return list of groups identified
	return strGroupList;
}

//===============================================================================================
// Load list of ENG Recipes
//===============================================================================================
QStringList PatProdRecipe::loadENG_RecipesList(QString strGroup)
{
	QStringList	strENG_RecipesList;
	QString	    strString;
	int		    iEntry;
	int			iEnabled;

	// Allow '[None]' if no engineering recipe avialable
	strENG_RecipesList += "[None]";

	// Open .ini file.
	QSettings settings(m_strENG_RecipesIni,QSettings::IniFormat);

	// Read all entries
	int iTotalRecipes= settings.value("global/total_eng_recipes", 0).toInt();
	QStringList groups = settings.childGroups();
	for(iEntry = 0;iEntry < iTotalRecipes; iEntry++)
	{
		// Seek to each section
		settings.beginGroup(groups.at(iEntry));

		iEnabled = settings.value("enabled","0").toInt();

		// Check if Matching Group
		strString = settings.value("group_name","").toString();
		if(strGroup != strString)
			iEnabled = 0;	// Not matching group, ignore this ENG recipe!

		// Get ENG recipe name
		strString = settings.value("eng_recipe_name","").toString();

		if(iEnabled && (strString.isEmpty() == false))
		{
			strString += " - V" + settings.value("eng_version","").toString();
			strString += "." + settings.value("eng_build","").toString();
			strString += "  [File: " + settings.value("src_file","").toString() + "]";
			strENG_RecipesList += strString;
		}

		// Close group seek.
		settings.endGroup();
	};

	// Return list of ENG Recipes
	return strENG_RecipesList;
}
