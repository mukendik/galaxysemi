//===============================================================================================
// DEBUG
// todo:
// - Administrate rules precedence
// - Support production version (vs engineering version)
//===============================================================================================

#include <QtGui>
#include <QRegExp>

#include "PatProdRecipe_Admin.h"
#include "recipe_profile_eng.h"
#include "recipe_profile_prod.h"
#include "../pat_prod_recipe/patprodrecipe.h"


extern QApplication app;

//===============================================================================================
// Contextual menu
//===============================================================================================
void PatProdRecipe_Admin::onContextualMenu(const QPoint& /*pos*/)
{
	QMenu	menu(this);
	QIcon	cIconProperties("properties.png");
	QIcon	cIcon("");

	// Build menu.
	menu.addAction(cIconProperties,"Properties...", this, SLOT(onPropertiesRecipe_PROD()));
	menu.addAction(cIcon,"Edit Recipe...", this, SLOT(onEditRecipe_PROD()));

	menu.setMouseTracking(true);
	menu.exec(QCursor::pos());
	menu.setMouseTracking(false);
}

//===============================================================================================
// Add ENG recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onAddRecipe_ENG(QString strRecipeFile)
{
	// Open file and ensures it is a valid recipe file!
	QFile f(strRecipeFile);
	if(f.exists() == false)
	return;	// File doesn't exist...quietly return!
	if(!f.open(QIODevice::ReadOnly))
	{
		QString strErrorMessage = "*PAT Error* Recipe file access denied:" + strRecipeFile;
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Assign file I/O stream
	QString	strErrorMessage;
	QString	strString;
	QString	strSection;
	QString	strKeyword;
	QTextStream hRecipeFile(&f);
	CENG_Recipe cEntry(m_strFlowsAvailable,strlGetGroupsList());
	bool	bOk;
	bool	bSectionFound=false;
	bool	bRecipeNameFound=false;
	bool	bRecipeVersionFound=false;

	// Set group in case filetr selected
	cEntry.strGroupName = ui.comboBoxGroupList->currentText();

	// Need to find section '<Outlier_Options>'
	do
	{
		// Read line.
		strString = hRecipeFile.readLine();
		strString = strString.trimmed();
		strKeyword = strString.section(',',0,0);

		if(strString != "<Outlier_Options>")
			bSectionFound=true;
		if(bSectionFound && strKeyword.compare("Product",Qt::CaseInsensitive) == 0)
		{
			bRecipeNameFound = true;
			cEntry.strRecipeName = strString.section(',',1,1).trimmed();
		}
		if(bSectionFound && strKeyword.compare("RecipeVersion",Qt::CaseInsensitive) == 0)
		{
			bRecipeVersionFound = true;
			strSection = strString.section(',',1,1);
			cEntry.iEng_Version = strSection.toInt(&bOk);
			if(!bOk || (cEntry.iEng_Version < 0))
				cEntry.iEng_Version = 1;
			strSection = strString.section(',',2,2);
			cEntry.iEng_Build = strSection.toInt(&bOk);
			if(!bOk || (cEntry.iEng_Build < 0))
				cEntry.iEng_Build = 0;
		}

		// Check if all info wanted found...
		if(bRecipeVersionFound)
			break;
	}
	while(hRecipeFile.atEnd() == false);

	if(! bRecipeVersionFound)
	{
		strErrorMessage = "*PAT Error* Invalid ENG Recipe file : " + strRecipeFile;
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Extract Recipe name from file (if exists)
	QFileInfo cFileInfo(strRecipeFile);
	cEntry.strRecipeFile = strString = cFileInfo.completeBaseName();	// Engineering Recipe name
	cEntry.strRecipeFile += "." + cFileInfo.suffix();
	strString = strString.section('_',0,0);
	if(strString.isEmpty() == false)
		cEntry.strRecipeName = strString;	// File name holds recipe as first sub-string, use it!

	// Add this entry to the Engineering recipe list...but in DISABLED mode!
	ui.tableWidgetRecipes_ENG->setSortingEnabled(false);	// Disable sorting until we've added all fields to table!
	int iRowIndex = ui.tableWidgetRecipes_ENG->rowCount();
	ui.tableWidgetRecipes_ENG->setRowCount(iRowIndex+1);
	writeENG_RecipeList_gui(iRowIndex,cEntry);

	// Edit profile of just added entry!
	ui.tableWidgetRecipes_ENG->setCurrentCell(iRowIndex,0);    // Ensure added line is selected.
	onPropertiesRecipe_ENG();	// Edit entry + force sorting

	// Readback current selection (as sorting may have changed entry position)
	// Reload structure with latest edits (eg: recipe name may habe changed, etc)
	iRowIndex = ui.tableWidgetRecipes_ENG->currentRow();
	readENG_RecipeList_gui(iRowIndex,cEntry);

	// Copy recipe file to engineering folder
	QFile   cFile;   // Source Recipe (Engineering folder)
	QString strDestinationFileFullPath;

	// Production File name
	cEntry.strRecipeFile = cFileInfo.completeBaseName() + "." + cFileInfo.suffix();

	// Full engineering file path+name
	strDestinationFileFullPath = m_strEngPath + "/" + cEntry.strRecipeFile;

	if(QFile::exists(strDestinationFileFullPath) == true)
	{
		strErrorMessage = "* Warning * ENG Recipe already uploaded.\nUpload a new version.\nFile: " + strDestinationFileFullPath;
			return; // Cancel
	}

	if(cFile.copy(strRecipeFile,strDestinationFileFullPath) == false)
	{
		strErrorMessage = "* Error * Failed to copy ENG recipe to server folder!\nRoot cause: File already exists!\n\nFile: " + strDestinationFileFullPath;
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Update .INI file
	saveSettings_ENG_Recipe(iRowIndex,false);

	// Log info to log file
	QString strText = cEntry.strRecipeName + " - Version" + QString::number(cEntry.iProd_Version);
	logEvent("Upload ENG Recipe",strText);

	// Update list sorting
	ui.tableWidgetRecipes_ENG->setSortingEnabled(true);	// Enable back sorting
	ui.tableWidgetRecipes_ENG->sortItems(0);

	// Flag changes made
	m_bChanges = true;
}

//===============================================================================================
// Add ENG recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onAddRecipe_ENG(void)
{
	// Check if Groups exist
	if(ui.tableWidgetGroups->rowCount() <= 0)
	{
		QMessageBox::warning(this,"Error","You must first create Groups!",QMessageBox::Ok);
		return;
	}

    // Les user select a recipe file
	QString strUserHome="";
	QString strRecipeFile = QFileDialog::getOpenFileName(this, "Select ENG recipe",strUserHome,"Recipes (*.csv);;All files (*)");

	onAddRecipe_ENG(strRecipeFile);
}

//===============================================================================================
// Properties: edit recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onPropertiesRecipe_ENG(void)
{
	CRecipeProfile_ENG cProfile;

    // Get current item selected
	int iRow = ui.tableWidgetRecipes_ENG->currentRow();
	if((iRow < 0) || ui.tableWidgetRecipes_ENG->isRowHidden(iRow))
		return;

    // Read Recipe profile from GUI list
	CENG_Recipe cEntry(m_strFlowsAvailable,strlGetGroupsList());
	if(readENG_RecipeList_gui(iRow,cEntry) == false)
		return;

    // Load profile editor with relevant profile details
	int iOriginalStatus = cEntry.iEnabled;
	QString strOriginalRecipeName = cEntry.strRecipeName;
	cProfile.writeProfile(cEntry,(m_cUser.iUserType != USER_RIGHTS_ENG));
    if(cProfile.exec() != 1)
		return;

    // Read updated profile
    cProfile.readProfile(cEntry);

    // Update Recipe list
	writeENG_RecipeList_gui(iRow,cEntry);

	// Update .INI file
	saveSettings_ENG_Recipe(iRow,false);

    // Log info to log file
	QString strEngRecipe = cEntry.strRecipeName + "_Version" + QString::number(cEntry.iEng_Version);
	strEngRecipe += (cEntry.iEnabled) ? " [Enabled]" : " [*Disabled*]";
	logEvent("Edit ENG Recipe",strEngRecipe);

	// If Recipe toggled from Disabled to Enabled, then disable all other ENG versions of this same recipe!
	QString strOld_ENG_Enabled;
	QString strNewVersion;
	if((iOriginalStatus == 0) && (cEntry.iEnabled))
	{
		int iIndex;
		QTableWidgetItem *pItem;

		strOld_ENG_Enabled = "";
		for(iIndex = 0; iIndex < ui.tableWidgetRecipes_ENG->rowCount(); iIndex++)
		{
			if((ui.tableWidgetRecipes_ENG->item(iIndex,1)->text() == cEntry.strRecipeName) &&
			   (ui.tableWidgetRecipes_ENG->item(iIndex,2)->text() == "Enabled") &&
			   (iIndex != iRow))
			{
				// Disable this recipe version!
				pItem = new QTableWidgetItem("Disabled");
				pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				pItem->setBackgroundColor(QColor(255,128,128));	// Red background
				ui.tableWidgetRecipes_ENG->setItem(iIndex,2,pItem);

				// Keep track of the original ENG recipe that was enabled
				strOld_ENG_Enabled = ui.tableWidgetRecipes_ENG->item(iIndex,4)->text();

				// Update .INI file
				saveSettings_ENG_Recipe(iIndex,false);

				// Find production Recipe using same product.
				if(ui.tableWidgetRecipes_PROD->rowCount() > 0)
				{
					for(iIndex = 0; iIndex < ui.tableWidgetRecipes_PROD->rowCount(); iIndex++)
					{
						if((ui.tableWidgetRecipes_PROD->item(iIndex,1)->text() == cEntry.strRecipeName))
						{
							// Edit PROD recipe to point to new ENG recipe file!
							ui.tableWidgetRecipes_PROD->setCurrentCell(iIndex,0);

							// Specify new recipe name to point to.
							strNewVersion = QString::number(cEntry.iEng_Version) + "." + QString::number(cEntry.iEng_Build);
							onEditRecipe_PROD(strOld_ENG_Enabled,cEntry.strRecipeFile,strNewVersion);
						}
					}
				}
			}

		}

#if 0
		// - Check PROD recipes that were using this ENG recipe (but other version)
		PatProdRecipe cProdRecipeDialog;
		CPROD_Recipe cProdEntry;
		CRecipeFlow	cFlow;
		int	iProdFlow;
		QString strProdRecipeName,tmpFile;
		QDir cDir;

		// Make TMP folder
		tmpFile =  m_strProdPath + "/tmp";
		cDir.mkdir(tmpFile);

		// Load Dialog box with relevant recipe info
		if(ui.tableWidgetRecipes_PROD->rowCount() > 0)
		{
			for(iIndex = 0; iIndex < ui.tableWidgetRecipes_PROD->rowCount(); iIndex++)
			{
				strProdRecipeName = m_strProdPath + "/" + ui.tableWidgetRecipes_PROD->item(iIndex,1)->text() + "_Version" + ui.tableWidgetRecipes_PROD->item(iIndex,3)->text() + ".csv";
				tmpFile = m_strProdPath + "/tmp/" + ui.tableWidgetRecipes_PROD->item(iIndex,1)->text() + "_Version" + ui.tableWidgetRecipes_PROD->item(iIndex,3)->text() + ".csv";
				cProdRecipeDialog.setStandalone(false);
				// Ensure PAT Bins are the global ones defined by the Admin user.
				cProdRecipeDialog.setPatBins(m_iPPAT_Bin,m_iGPAT_Bin,m_iZPAT_Bin);

				cProdRecipeDialog.loadRecipe(strProdRecipeName);
				cProdRecipeDialog.pullRecipe_GUI(cProdEntry);

				// Read recipe profile from GUI
				readPROD_RecipeList_gui(iIndex,cProdEntry);

				for(iProdFlow = 0; iProdFlow < cProdEntry.m_cFlows.count(); iProdFlow++)
				{
					cFlow = cProdEntry.m_cFlows.at(iProdFlow);

					// Update ENG recipe name that PROD recipe is pointing to!
					if(cFlow.m_strENG_RecipeFile == strOld_ENG_Enabled)
					{
						// Update Recipe name
						cFlow.m_strENG_RecipeFile = cEntry.strRecipeFile;
						cFlow.m_strENG_Version = QString::number(cEntry.iEng_Version) + "." + QString::number(cEntry.iEng_Build);

						// Update info in list
						cProdEntry.m_cFlows.replace(iProdFlow,cFlow);

						// Update GUI
						writePROD_RecipeList_gui(iIndex,cProdEntry);
					}
				}

				/////////////
				// Remove GUI line juste edited
				ui.tableWidgetRecipes_PROD->removeRow(iIndex);

				// Add PROD recipe just EDITED
				onAddRecipe_PROD(tmpFile,true);
				cDir.remove(tmpFile);
			}
		}
#endif
	}

	// If recipe name changed in GUI, rename recipe file as well
	if(strOriginalRecipeName != cEntry.strRecipeName)
	{
		QDir cDir;
		QString strOldName = m_strEngPath + "/" + strOriginalRecipeName;
		QString strNewName = m_strEngPath + "/" + cEntry.strRecipeName;
		cDir.remove(strNewName);
		if(cDir.rename(strOldName,strNewName) == false)
		{
			// Warning failed to rename recipe file
			QMessageBox::warning(this,"Error","Failed to rename recipe file on server...",QMessageBox::Ok);
		}
	}

    // Flag changes made
	m_bChanges = true;
}

//===============================================================================================
// Remove recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onRemoveRecipe_ENG(void)
{
    // Get current item selected
	int iRow = ui.tableWidgetRecipes_ENG->currentRow();
	if((iRow < 0) || ui.tableWidgetRecipes_ENG->isRowHidden(iRow))
		return;

	// Read Recipe profile from GUI list
	CENG_Recipe cEntry(m_strFlowsAvailable,strlGetGroupsList());
	if(readENG_RecipeList_gui(iRow,cEntry) == false)
		return;

	// If 'Engineering user' and recipe 'Enabled', reject action!
	if((m_cUser.iUserType == USER_RIGHTS_ENG) && (cEntry.iEnabled))
	{
		QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
		return;
	}

    // Request confirmation before delete...
	QString strText = "Delete '" + cEntry.strRecipeName + "' ?";
    if(QMessageBox::question(this,"Confirm delete",strText,QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	// Update .INI file
	saveSettings_ENG_Recipe(iRow,true);

    // Log info to log file
	strText = cEntry.strRecipeName + "_Version" + ui.tableWidgetRecipes_ENG->item(iRow,3)->text();
	logEvent("Delete ENG Recipe",strText);

    // Delete current line
	ui.tableWidgetRecipes_ENG->removeRow(iRow);

    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Create New PROD recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onNewRecipe_PROD(void)
{
	// If 'Engineering user' and recipe 'Enabled', reject action!
	if(m_cUser.iUserType == USER_RIGHTS_ENG)
	{
		QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
		return;
	}

	// Check if Groups exist
	if(ui.tableWidgetGroups->rowCount() <= 0)
	{
		QMessageBox::warning(this,"Error","You must first create Groups!",QMessageBox::Ok);
		return;
	}
	// Dialog box is launched as Child window
	PatProdRecipe cProdRecipeDialog;
	cProdRecipeDialog.setStandalone(false);
	cProdRecipeDialog.setPatBins(m_iPPAT_Bin,m_iGPAT_Bin,m_iZPAT_Bin);

	// Display dialog box.
	if(cProdRecipeDialog.exec() < 0)
		return;	// User cancelled PROD recipe creation

	// Add PROD recipe juste created
	onAddRecipe_PROD(cProdRecipeDialog.m_strProdRecipe,true);

	// Remove temporary file created
	QDir cDir;
	cDir.remove(cProdRecipeDialog.m_strProdRecipe);

	// Flag changes made
	m_bChanges = true;
}

//===============================================================================================
// Edit existing PROD recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onEditRecipe_PROD(void)
{
	onEditRecipe_PROD("","","");
}

//===============================================================================================
// Update PROD recipe to point to a new ENG recipe file.
//===============================================================================================
void PatProdRecipe_Admin::onEditRecipe_PROD(QString strOldRecipe_ENG,QString strNewRecipe_ENG,QString strNewVersion)
{
	// If 'Engineering user' and recipe 'Enabled', reject action!
	if(m_cUser.iUserType == USER_RIGHTS_ENG)
	{
		QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
		return;
	}

	// Check if Groups exist
	if(ui.tableWidgetGroups->rowCount() <= 0)
	{
		QMessageBox::warning(this,"Error","You must first create Groups!",QMessageBox::Ok);
		return;
	}

	// Get current item selected
	int iRow = ui.tableWidgetRecipes_PROD->currentRow();
	if((iRow < 0) || ui.tableWidgetRecipes_PROD->isRowHidden(iRow))
		return;

	// Read Recipe profile from GUI list
	CPROD_Recipe cEntry;
	if(readPROD_RecipeList_gui(iRow,cEntry) == false)
		return;

	// Path to PROD recipe to edit
	QString strOriginalRecipeName = m_strProdPath + "/" + cEntry.m_strRecipeFile;

	// Dialog box is launched as Child window
	PatProdRecipe cProdRecipeDialog;
	cProdRecipeDialog.setStandalone(false);

	// Load Dialog box with relevant recipe info
	cProdRecipeDialog.loadRecipe(strOriginalRecipeName);

	// Ensure PAT Bins are the global ones defined by the Admin user.
	cProdRecipeDialog.setPatBins(m_iPPAT_Bin,m_iGPAT_Bin,m_iZPAT_Bin);

	// Specify we need to replace a ENG recipe name with a new version
	cProdRecipeDialog.setNewRecipe_ENG(strOldRecipe_ENG,strNewRecipe_ENG,strNewVersion);

	// Display dialog box (unless it was only an update of the ENG recipe!)
	bool bForceOverwrite;
	if(strOldRecipe_ENG.isEmpty() && strNewRecipe_ENG.isEmpty())
	{
		// Display dialog box
		if(cProdRecipeDialog.exec() < 0)
			return;	// User cancelled PROD recipe creation

		// Do not allow overwritting existing PROD recipe
		bForceOverwrite = false;
	}
	else
	{
		// Updating existing PROD recipe file name...
		cProdRecipeDialog.m_strProdRecipe = strOriginalRecipeName;

		// Allow overwritting existing PROD recipe
		QDir cDir;
		cDir.remove(cProdRecipeDialog.m_strProdRecipe);
		bForceOverwrite = false;

		// Disable autoincrement
		cProdRecipeDialog.m_iAutoIncrement = false;
		cProdRecipeDialog.refreshGUI();

		// Save edited recipe in user folder
		cProdRecipeDialog.saveRecipe();

	}

	// If 'Auto-Increment': keep previous version but disabled it
	if(strOldRecipe_ENG.isEmpty() && strNewRecipe_ENG.isEmpty() && cProdRecipeDialog.isAutoIncrement())
	{
		// Auto-increment ENABLED

		// Add PROD recipe just EDITED as NEW entry (keep older entry)
		onAddRecipe_PROD(cProdRecipeDialog.m_strProdRecipe,false);
	}
	else
	{
		// Auto-increment DISABLED

		// Remove GUI line juste edited
		ui.tableWidgetRecipes_PROD->removeRow(iRow);

		// Add PROD recipe just EDITED
		onAddRecipe_PROD(cProdRecipeDialog.m_strProdRecipe,bForceOverwrite);
	}

	// Flag changes made
	m_bChanges = true;
}

//===============================================================================================
// Add PROD recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onAddRecipe_PROD(QString strRecipeFile,bool bForceOverwrite/*=false*/)
{
	QString strErrorMessage;
	CPROD_Recipe cEntry;
	QFile   cFile;

	QFileInfo	cFileInfo(strRecipeFile);
	if(cEntry.loadRecipe(strRecipeFile,strErrorMessage) == false)
	{
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Copy file to Production folder
	QString strDestination = m_strProdPath + "/";
	strDestination += cFileInfo.completeBaseName();	// Engineering Recipe name
	strDestination += "." + cFileInfo.suffix();

	// Do we force overwrite if destination exists?
	if(bForceOverwrite)
		cFile.remove(strDestination);

	if(QFile::exists(strDestination) == true)
	{
		strErrorMessage = "* Warning * PROD Recipe file name already exists in folder.\nConfirm to overwrite it?\nFile: "+ strDestination;
		if(QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;
		// Remove file to overwrite
		cFile.remove(strDestination);
	}

	if(cFile.copy(strRecipeFile,strDestination) == false)
	{
		strErrorMessage = "* Error * Failed to copy production recipe to server folder!\nRoot cause: File already exists!\n\nFile: " + strDestination;
		QMessageBox::warning(this,"Error",strErrorMessage,QMessageBox::Ok);
		return;
	}

	// Extract Recipe name from file (if exists)
	QString		strString;

	// Add this entry to the Production recipe list...but in DISABLED mode!
	ui.tableWidgetRecipes_PROD->setSortingEnabled(false);	// Disable sorting until we've added all fields to table!
	int iRowIndex = ui.tableWidgetRecipes_PROD->rowCount();
	ui.tableWidgetRecipes_PROD->setRowCount(iRowIndex+1);
	writePROD_RecipeList_gui(iRowIndex,cEntry);

	// Edit profile of just added entry!
	ui.tableWidgetRecipes_PROD->setCurrentCell(iRowIndex,0);    // Ensure added line is selected.
	onPropertiesRecipe_PROD();	// Edit entry + force sorting

	// Readback current selection (as sorting may have changed entry position)
	// Reload structure with latest edits (eg: recipe name may habe changed, etc)
	iRowIndex = ui.tableWidgetRecipes_PROD->currentRow();
	readPROD_RecipeList_gui(iRowIndex,cEntry);

	// Enable production release if needed
	saveSettings_PROD_Recipe(iRowIndex,false,false);

	// Log info to log file
	QString strText = cEntry.m_strRecipeName + "_Version" + QString::number(cEntry.m_iVersion);
	logEvent("Upload PROD Recipe",strText);

	// Update list sorting
	ui.tableWidgetRecipes_PROD->setSortingEnabled(true);	// Enable back sorting
	ui.tableWidgetRecipes_PROD->sortItems(0);

	// Flag changes made
	m_bChanges = true;
}

//===============================================================================================
// Add PROD recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onAddRecipe_PROD(void)
{
	// Check if Groups exist
	if(ui.tableWidgetGroups->rowCount() <= 0)
	{
		QMessageBox::warning(this,"Error","You must first create Groups!",QMessageBox::Ok);
		return;
	}

	// Les user select a recipe file
	QString strErrorMessage;
	QString strUserHome="";
	QString strRecipeFile = QFileDialog::getOpenFileName(this, "Select PROD recipe",strUserHome,"Recipes (*.csv);;All files (*)");

	onAddRecipe_PROD(strRecipeFile);
}

//===============================================================================================
// Properties: edit recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onPropertiesRecipe_PROD(void)
{
	CRecipeProfile_PROD cProfile;

	// Get current item selected
	int iRow = ui.tableWidgetRecipes_PROD->currentRow();
	if((iRow < 0) || ui.tableWidgetRecipes_PROD->isRowHidden(iRow))
		return;

	// Read Recipe profile from GUI list
	CPROD_Recipe cEntry;
	if(readPROD_RecipeList_gui(iRow,cEntry) == false)
		return;

	// Load profile editor with relevant profile details
	int iOriginalStatus = cEntry.m_iEnabled;
	QString strOriginalRecipeName = cEntry.m_strRecipeName;
	cProfile.writeProfile(cEntry,(m_cUser.iUserType != USER_RIGHTS_ENG));
	if(cProfile.exec() != 1)
		return;

	// Read updated profile
	cProfile.readProfile(cEntry);

	// Update Recipe list
	writePROD_RecipeList_gui(iRow,cEntry);


	// Log info to log file
	QString strText = cEntry.m_strRecipeName + "_Version" + QString::number(cEntry.m_iVersion);
	strText += (cEntry.m_iEnabled) ? " now [Enabled]" : " now [*Disabled*]";
	logEvent("Edit PROD Recipe",strText);

	// If Recipe toggled from Disabled to Enabled, then disable all other versions of this same recipe!
	if((iOriginalStatus == 0) && (cEntry.m_iEnabled))
	{
		int iIndex;
		QTableWidgetItem *pItem;

		for(iIndex = 0; iIndex < ui.tableWidgetRecipes_PROD->rowCount(); iIndex++)
		{
			if((ui.tableWidgetRecipes_PROD->item(iIndex,1)->text() == cEntry.m_strRecipeName) &&
			   (ui.tableWidgetRecipes_PROD->item(iIndex,2)->text() == "Enabled") &&
			   (iIndex != iRow))
			{
				// Disable this recipe version!
				pItem = new QTableWidgetItem("Disabled");
				pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				pItem->setBackgroundColor(QColor(255,128,128));	// Red background
				ui.tableWidgetRecipes_PROD->setItem(iIndex,2,pItem);

				// Update .INI file: disable this PROD recipe entry.
				saveSettings_PROD_Recipe(iIndex,false,true);
			}
		}

		// Enable production release if needed
		saveSettings_PROD_Recipe(iRow,false,false);
	}
	else
	// If Recipe toggled from Enabled to Disabled: ensure to delete released recipe
	if((iOriginalStatus == 1) && (cEntry.m_iEnabled == false))
	{
		// Enable production release if needed
		saveSettings_PROD_Recipe(iRow,false,true);
	}

	// Flag changes made
	m_bChanges = true;
}


//===============================================================================================
// Remove PROD recipe profile
//===============================================================================================
void PatProdRecipe_Admin::onRemoveRecipe_PROD(void)
{
	// Get current item selected
	int iRow = ui.tableWidgetRecipes_PROD->currentRow();
	if((iRow < 0) || ui.tableWidgetRecipes_PROD->isRowHidden(iRow))
		return;

	// Read Recipe profile from GUI list
	CPROD_Recipe cEntry;
	if(readPROD_RecipeList_gui(iRow,cEntry) == false)
		return;

	// If 'Engineering user' and recipe 'Enabled', reject action!
	if((m_cUser.iUserType == USER_RIGHTS_ENG) && (cEntry.m_iEnabled))
	{
		QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
		return;
	}

	// Request confirmation before delete...
	QString strText = "Delete Line " + QString::number(iRow+1) + " -> '" + cEntry.m_strRecipeName;
	strText += "' Version " + QString::number(cEntry.m_iVersion) + "." + QString::number(cEntry.m_iBuild) + " ?";
	if(QMessageBox::question(this,"Confirm delete",strText,QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

	// Enable production release if needed
	saveSettings_PROD_Recipe(iRow,true,cEntry.m_iEnabled);

	// Log info to log file
	strText = cEntry.m_strRecipeName + "_Version" + ui.tableWidgetRecipes_PROD->item(iRow,3)->text();
	logEvent("Delete PROD Recipe",strText);

	// Delete current line
	ui.tableWidgetRecipes_PROD->removeRow(iRow);

	// Flag changes made
	m_bChanges = true;
}
//===============================================================================================
// Specific group selected: refresh Recipe list to only show recipes matching criteria
//===============================================================================================
void PatProdRecipe_Admin::onSelectGroup()
{
	// Hide all recipe lines not belonging to the selected group
	QString strGroup = ui.comboBoxGroupList->currentText();
	int iIndex = ui.comboBoxGroupList->currentIndex();

	if(iIndex == 0)
		strGroup = "";	// Display ALL recipes (all groups).

	QRegExp strRegExp(ui.lineEditRecipeFilter->text());
	strRegExp.setPatternSyntax(QRegExp::Wildcard);

	QString strGroupLine,strRecipeLine;
	int iTotalEntries = ui.tableWidgetRecipes_ENG->rowCount();
	for(iIndex = 0; iIndex < iTotalEntries; iIndex++)
	{
		// ENG tab Get Group name & recipe name for given line
		strGroupLine = ui.tableWidgetRecipes_ENG->item(iIndex,0)->text();
		strRecipeLine = ui.tableWidgetRecipes_ENG->item(iIndex,1)->text();

		if(strGroup.isEmpty() || (strGroupLine == strGroup))
		{
			if(strRegExp.exactMatch(strRecipeLine))
				ui.tableWidgetRecipes_ENG->showRow(iIndex);	// Show line
			else
				ui.tableWidgetRecipes_ENG->hideRow(iIndex);	// Hide line
		}
		else
			ui.tableWidgetRecipes_ENG->hideRow(iIndex);	// Hide line
	}

	iTotalEntries = ui.tableWidgetRecipes_PROD->rowCount();
	for(iIndex = 0; iIndex < iTotalEntries; iIndex++)
	{
		// PROD tab Get Group name & recipe name for given line
		strGroupLine = ui.tableWidgetRecipes_PROD->item(iIndex,0)->text();
		strRecipeLine = ui.tableWidgetRecipes_PROD->item(iIndex,1)->text();

		if(strGroup.isEmpty() || (strGroupLine == strGroup))
		{
			if(strRegExp.exactMatch(strRecipeLine))
				ui.tableWidgetRecipes_PROD->showRow(iIndex);	// Show line
			else
				ui.tableWidgetRecipes_PROD->hideRow(iIndex);	// Hide line
		}
		else
			ui.tableWidgetRecipes_PROD->hideRow(iIndex);	// Hide line
	}
}

//===============================================================================================
// Structure holding recipe settings: constructor
//===============================================================================================
CENG_Recipe::CENG_Recipe(QStringList strFlowsAvailable,QStringList strGroupsAvailable)
{
	strFlows = strFlowsAvailable; // List of valid flows
	strGroups = strGroupsAvailable; // List of groups available
	strGroupName="";		// Group name.
    strRecipeName="";	    // Recipe file name (without version# nor Galaxy suffix string)
    strRecipeFile="";	    // Recipe file name.
	strComment="";			// Comments
    iEnabled=0;		    // 0=recipe not active (default) in production, 1=active
	iEng_Version=1;		// Recipe version
}

//===============================================================================================
// Write ENG recipe profile (and their settings) to GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::writeENG_RecipeList_gui(int iRow,CENG_Recipe cEntry)
{
    QString strString;
    int	    iCol=0;

	// Disable sorting
	ui.tableWidgetRecipes_ENG->setSortingEnabled(false);

	QTableWidgetItem *pItem = new QTableWidgetItem(cEntry.strGroupName);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_ENG->setItem(iRow, iCol, pItem);
	iCol++;

	pItem = new QTableWidgetItem(cEntry.strRecipeName);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_ENG->setItem(iRow, iCol, pItem);
	iCol++;

    // Recipe enabled?
    if(cEntry.iEnabled)
		pItem = new QTableWidgetItem("Enabled");
	else
	{
		pItem = new QTableWidgetItem("Disabled");
		pItem->setBackgroundColor(QColor(255,128,128));	// Red background
	}

    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_ENG->setItem(iRow, iCol, pItem);
    iCol++;

	// Engineering Version# & Build#
	strString = QString::number(cEntry.iEng_Version) + "." + QString::number(cEntry.iEng_Build);
	pItem = new QTableWidgetItem(strString);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_ENG->setItem(iRow, iCol, pItem);
	iCol++;

	 // Recipe file name
    pItem = new QTableWidgetItem(cEntry.strRecipeFile);
    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_ENG->setItem(iRow, iCol, pItem);
    iCol++;

	// Comment
   pItem = new QTableWidgetItem(cEntry.strComment);
   pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
   ui.tableWidgetRecipes_ENG->setItem(iRow, iCol, pItem);
   iCol++;

    // Success
    return true;
}

//===============================================================================================
// Write PROD recipe profile (and their settings) to GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::writePROD_RecipeList_gui(int iRow,CPROD_Recipe cEntry)
{
	QString strString;
	int	    iCol=0;

	// Disable sorting
	ui.tableWidgetRecipes_PROD->setSortingEnabled(false);

	QTableWidgetItem *pItem = new QTableWidgetItem(cEntry.m_strGroup);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	pItem = new QTableWidgetItem(cEntry.m_strRecipeName);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// Recipe enabled?
	if(cEntry.m_iEnabled)
		pItem = new QTableWidgetItem("Enabled");
	else
	{
		pItem = new QTableWidgetItem("Disabled");
		pItem->setBackgroundColor(QColor(255,128,128));	// Red background
	}

	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// Engineering Version# & Build#
	strString = QString::number(cEntry.m_iVersion) + "." + QString::number(cEntry.m_iBuild);
	pItem = new QTableWidgetItem(strString);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// Flow list
	pItem = new QTableWidgetItem(cEntry.strFlowListDetails());
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// STDF output mode
	pItem = new QTableWidgetItem(cEntry.strStdfOutputMode());
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// PROD Recipe file: hide column (as per ST request)
	ui.tableWidgetRecipes_PROD->hideColumn(iCol);
	pItem = new QTableWidgetItem(cEntry.m_strRecipeFile);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// Comment
	pItem = new QTableWidgetItem(cEntry.m_strComment);
	pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetRecipes_PROD->setItem(iRow, iCol, pItem);
	iCol++;

	// Success
	return true;
}

//===============================================================================================
// Read ENG recipe profile from GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::readENG_RecipeList_gui(int iRow,CENG_Recipe &cEntry)
{
    QString strString;
    int	    iCol=0;

    // Check if row# out of range!
	if(iRow < 0 || iRow >= ui.tableWidgetRecipes_ENG->rowCount())
		return false;

	// Group name
	cEntry.strGroupName = ui.tableWidgetRecipes_ENG->item(iRow,iCol)->text();
	iCol++;

	// Recipe name
	cEntry.strRecipeName = ui.tableWidgetRecipes_ENG->item(iRow,iCol)->text();
	iCol++;

    // Recipe enabled?
	cEntry.iEnabled = (ui.tableWidgetRecipes_ENG->item(iRow,iCol)->text() == "Enabled") ? 1:0;
    iCol++;

	// Engineering Version# & Build#
	strString = ui.tableWidgetRecipes_ENG->item(iRow,iCol)->text();
	cEntry.iEng_Version = strString.section('.',0,0).toInt();
	cEntry.iEng_Build = strString.section('.',1,1).toInt();
	iCol++;

	// Recipe name
	cEntry.strRecipeFile = ui.tableWidgetRecipes_ENG->item(iRow,iCol)->text();
    iCol++;

	// Comment
	cEntry.strComment = ui.tableWidgetRecipes_ENG->item(iRow,iCol)->text();
	iCol++;

    // Success
    return true;
}

//===============================================================================================
// Read PROD recipe profile from GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::readPROD_RecipeList_gui(int iRow,CPROD_Recipe &cEntry)
{
	QString strString,strErrorMessage;

	// Check if row# out of range!
	if(iRow < 0 || iRow >= ui.tableWidgetRecipes_PROD->rowCount())
		return false;

	// Recipe name
	cEntry.m_strRecipeFile = m_strProdPath + "/" + ui.tableWidgetRecipes_PROD->item(iRow,6)->text();

	// Full reload from disk (so to have all Flows)
	cEntry.loadRecipe(cEntry.m_strRecipeFile,strErrorMessage);

	// Status: Recipe enabled?
	cEntry.m_iEnabled = (ui.tableWidgetRecipes_PROD->item(iRow,2)->text() == "Enabled") ? 1:0;

	return true;
}

//===============================================================================================
// Get Production version for given product
//===============================================================================================
int PatProdRecipe_Admin::getProductionRecipeVersion(QString strProduct)
{
	// Open .ini settings file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);
	QString strProductEntry = strProduct + "/prod_version";
	return settings.value(strProductEntry, 0).toInt();
}

//===============================================================================================
// Set Production version for given product
//===============================================================================================
void PatProdRecipe_Admin::setProductionRecipeVersion(QString strProduct,int Version)
{
	// Open .ini settings file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);
	QString strProductEntry = strProduct + "/prod_version";
	settings.setValue(strProductEntry,Version);
}

//===============================================================================================
// Load list of ENG recipes (and their settings)
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_ENG_Recipes(void)
{    
    QString	    strString;
	int		    iEntry,iValidEntryIndex;
	CENG_Recipe	cEntry(m_strFlowsAvailable,strlGetGroupsList());

    // Open .ini settings file.
	QSettings settings(m_strENG_RecipesIni,QSettings::IniFormat);

    // Read all entries
	int iTotalEntries = settings.value("global/total_eng_recipes", 0).toInt();

	// Resize table accordingly
	QStringList groups = settings.childGroups();
	iTotalEntries = groups.count();
	ui.tableWidgetRecipes_ENG->setRowCount(iTotalEntries);

	iValidEntryIndex = 0;
	for(iEntry = 0;iEntry < iTotalEntries; iEntry++)
    {
		// Seek to section
		settings.beginGroup(groups.at(iEntry));
	
		cEntry.strGroupName = settings.value("group_name","").toString();
		cEntry.strRecipeName = settings.value("eng_recipe_name","").toString();
		cEntry.strComment = settings.value("comment","").toString();
		strString = settings.value("enabled","").toString();
		cEntry.iEnabled = settings.value("enabled",1).toInt();
		cEntry.iEng_Version = settings.value("eng_version",1).toInt();
		cEntry.iEng_Build = settings.value("eng_build",0).toInt();
		cEntry.strRecipeFile = settings.value("src_file","").toString();
	
		// Close group seek.
		settings.endGroup();
	
		// Add entry details to GUI...unless invalid entry (no name specified)
		if(cEntry.strRecipeName.isEmpty() == false)
		{
			writeENG_RecipeList_gui(iValidEntryIndex++,cEntry);
		}
    };
	// Update table size
	ui.tableWidgetRecipes_ENG->setRowCount(iValidEntryIndex);
    // Sort table
	ui.tableWidgetRecipes_ENG->sortItems(0); // Sort over first column
}

//===============================================================================================
// Load list of PROD recipes (and their settings)
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_PROD_Recipes(void)
{
	QString			strString;
	QString			strErrorMessage;
	int				iEntry,iValidEntryIndex;
	CPROD_Recipe	cEntry;

	// Open .ini settings file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

	// Read all entries
	int iTotalEntries = settings.value("global/total_prod_recipes", 0).toInt();

	// Resize table accordingly
	QStringList groups = settings.childGroups();
	iTotalEntries = groups.count();
	ui.tableWidgetRecipes_PROD->setRowCount(iTotalEntries);

	iValidEntryIndex = 0;
	for(iEntry = 0;iEntry < iTotalEntries; iEntry++)
	{
		// Seek to section
		settings.beginGroup(groups.at(iEntry));
		// Get PROD recipe name
		cEntry.m_strRecipeFile = settings.value("src_file","").toString();
		strString = m_strProdPath + "/" + cEntry.m_strRecipeFile;

		// Get status: enabled/disabled
		cEntry.m_iEnabled = settings.value("enabled","0").toInt();

		// Load recipe details + update GUI
		if(cEntry.loadRecipe(strString,strErrorMessage))
		{
			writePROD_RecipeList_gui(iValidEntryIndex++,cEntry);
		}

		// Close group seek.
		settings.endGroup();	
	};
	// Update table size
	ui.tableWidgetRecipes_PROD->setRowCount(iValidEntryIndex);
	// Sort table
	ui.tableWidgetRecipes_PROD->sortItems(0); // Sort over first column
}

//===============================================================================================
// Save specific ENG recipe (and its settings) to disk
//===============================================================================================
void PatProdRecipe_Admin::saveSettings_ENG_Recipe(int iRow,bool bDeleteRecipe)
{
    QString	    strString;
	QString	    strENG_RecipeFiles,strFileName;
	QFile		cFile;
	CENG_Recipe	cEntry(m_strFlowsAvailable,strlGetGroupsList());

    // Open .ini settings file.
	QSettings settings(m_strENG_RecipesIni,QSettings::IniFormat);

    // Total number of recipe profiles to dump to disk.
	int iTotalEntries = ui.tableWidgetRecipes_ENG->rowCount();
	if(bDeleteRecipe && (iTotalEntries > 0))
		iTotalEntries--;	// If entry about to be removed, decrement recepe count
	settings.setValue("global/total_eng_recipes", iTotalEntries);

	// Read recipe profile from GUI
	readENG_RecipeList_gui(iRow,cEntry);

	// Seek to relevant Group: 'recipe_X'
	strString = cEntry.strRecipeName + "_V" + QString::number(cEntry.iEng_Version) + "." + QString::number(cEntry.iEng_Build);
	settings.beginGroup(strString);
	// Delete entry?
	if(bDeleteRecipe)
	{
		settings.remove("");

		// Delete ENG recipe file
		QString strFile = m_strEngPath + "/" + cEntry.strRecipeFile;
		cFile.remove(strFile);
	}
	else
	{
		settings.setValue("group_name",cEntry.strGroupName);
		settings.setValue("eng_recipe_name",cEntry.strRecipeName);
		settings.setValue("comment",cEntry.strComment);
		settings.setValue("enabled",cEntry.iEnabled);
		settings.setValue("eng_version",cEntry.iEng_Version);
		settings.setValue("eng_build",cEntry.iEng_Build);
		settings.setValue("src_file",cEntry.strRecipeFile);	
	}
	// Close group seek.
	settings.endGroup();
}

//===============================================================================================
// Save given PROD recipe (and its settings) to disk
//===============================================================================================
void PatProdRecipe_Admin::saveSettings_PROD_Recipe(int iRow,bool bDeleteRecipe,bool bDeleteReleasedRecipe)
{
	QString	    strString;
	QString	    strReleasedProdRecipe,strFileName;
	QDir		cDir;
	QFile cFile;
	CRecipeFlow	cRecipeOneFlow;
	CPROD_Recipe cEntry;
	CPROD_Recipe cReleaseRecipe;

	// Open .ini settings file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

	// Total number of recipe profiles to dump to disk.
	int iTotalEntries = ui.tableWidgetRecipes_PROD->rowCount();
	if(bDeleteRecipe && (iTotalEntries > 0))
		iTotalEntries--;	// If entry about to be removed, decrement recepe count
	settings.setValue("global/total_prod_recipes", iTotalEntries);

	// Production Release sub-folder
	strReleasedProdRecipe = m_strProdPath + "/released";
	cDir.mkdir(strReleasedProdRecipe);

	// Read recipe profile from GUI
	readPROD_RecipeList_gui(iRow,cEntry);

	// Seek to relevant Group
	strString = cEntry.m_strRecipeName + "_V" + QString::number(cEntry.m_iVersion) + "." + QString::number(cEntry.m_iBuild);
	settings.beginGroup(strString);

	// Delete entry?
	if(bDeleteRecipe)
	{
		settings.remove("");

		// Delete PROD recipe file
		QString strFile = m_strProdPath + "/" + cEntry.m_strRecipeFile;
		cFile.remove(strFile);
	}
	else
	{
		// Enabled/Disabled recipe
		settings.setValue("enabled",cEntry.m_iEnabled);
		// PAth to PROD recipe
		settings.setValue("src_file",cEntry.m_strRecipeFile);
	}
	// Close group seek.
	settings.endGroup();

	// Create overload file that PAT-Man will use
	strReleasedProdRecipe = m_strProdPath + "/released";
	strFileName = cEntry.m_strRecipeName + "." + QString::number(cEntry.m_iVersion) + ".csv";
	strReleasedProdRecipe += "/" + strFileName;

	// Remove released recipe if requested
	if(bDeleteReleasedRecipe)
		cDir.remove(strReleasedProdRecipe);

	// If enabled, move new copy.
	if(cEntry.m_iEnabled)
	{
		// Copy PROD recipe to 'released' folder
		QString strSource = m_strProdPath + "/" + cEntry.m_strRecipeFile;
		cDir.remove(strReleasedProdRecipe);
		cFile.copy(strSource,strReleasedProdRecipe);
	}
}
