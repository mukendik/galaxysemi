#include <QtGui>

#include "PatProdRecipe_admin.h"
#include "group_profile.h"


extern QApplication app;



//===============================================================================================
// Add Group profile
//===============================================================================================
void PatProdRecipe_Admin::onAddGroup(void)
{
	// Check user privileges
	switch(m_cUser.iUserType)
	{
		case USER_RIGHTS_ENG: // Read-only user
		case USER_RIGHTS_PROD: // Read/Write user
			QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
			return;	// This user can't create
			break;
		case USER_RIGHTS_ADMIN: // Admin user
			break;
	}

	CGroupProfile cGroupDialog;
	CGroupInfo	cGroup;
	if(cGroupDialog.exec() != 1)
	return;

	// Read entered Group details
	cGroupDialog.readGroup(cGroup);

	// Add group entry to list....unless already in list!
	int	iGroup;
	int iTotalRows = ui.tableWidgetGroups->rowCount();
	for(iGroup = 0; iGroup < iTotalRows; iGroup++)
    {
		if(cGroup.strGroupName.compare(ui.tableWidgetGroups->item(iGroup,0)->text(),Qt::CaseInsensitive) == 0)
		{
			QMessageBox::warning(this,"New group","This group already exists!",QMessageBox::Ok);
			return;
		}
    }

    // Ok, new entry is valid, add it to the list!
	ui.tableWidgetGroups->setSortingEnabled(false);	// Disable sorting until we've added all fields to table!
	writeGroupList_gui(iTotalRows,cGroup);

    // Update list sorting
	ui.tableWidgetGroups->setSortingEnabled(true);	// Disable sorting until we've added all fields to table!
	ui.tableWidgetGroups->sortItems(0);

	// Add entry to combo-box list
	ui.comboBoxGroupList->addItem(cGroup.strGroupName);

	// Save settings to disk
	saveSettings_Groups();

	// Log info to log file
	logEvent("Add Group",cGroup.strGroupName);


    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Properties: edit Group profile
//===============================================================================================
void PatProdRecipe_Admin::onPropertiesGroup(void)
{
	// Check user privileges
	switch(m_cUser.iUserType)
	{
		case USER_RIGHTS_ENG: // Read-only user
		case USER_RIGHTS_PROD: // Read/Write user
			QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
			return;	// This user can't create
			break;
		case USER_RIGHTS_ADMIN: // Admin user
			break;
	}

	// Get current item selected
	int iRow = ui.tableWidgetGroups->currentRow();
    if(iRow < 0)
	return;

	// Read Group profile from GUI list
	CGroupInfo cGroup;
	if(readGroupList_gui(iRow,cGroup) == false)
		return;

    // Show/Edit entry
	CGroupProfile cGroupDialog;
	cGroupDialog.writeGroup(cGroup);
	if(cGroupDialog.exec() != 1)
		return;

    // Update Gui list
	ui.tableWidgetGroups->setSortingEnabled(false);	// Disable sorting until we've added all fields to table!
	cGroupDialog.readGroup(cGroup);
	writeGroupList_gui(iRow,cGroup);
	ui.tableWidgetGroups->setSortingEnabled(true);	// Enable back sorting
	ui.tableWidgetGroups->sortItems(0);

	// Save settings to disk
	saveSettings_Groups();

	// Log info to log file
	logEvent("Edit Group",cGroup.strGroupName);

    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Remove Group profile
//===============================================================================================
void PatProdRecipe_Admin::onRemoveGroup(void)
{
	// Check user privileges
	switch(m_cUser.iUserType)
	{
		case USER_RIGHTS_ENG: // Read-only user
		case USER_RIGHTS_PROD: // Read/Write user
			QMessageBox::warning(this,"Restricted action","You don't have enough privileges for this action!",QMessageBox::Ok);
			return;	// This user can't create
			break;
		case USER_RIGHTS_ADMIN: // Admin user
			break;
	}

	// Get current item selected
	int iRow = ui.tableWidgetGroups->currentRow();
    if(iRow < 0)
		return;

    // Request confirmation before delete...
	CGroupInfo cGroup;
	cGroup.strGroupName = ui.tableWidgetGroups->item(iRow,0)->text();
	QString strText = "Delete group '" + cGroup.strGroupName + "' ?";
    if(QMessageBox::question(this,"Confirm delete",strText,QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
	return;

    // Delete current line
	ui.tableWidgetGroups->removeRow(iRow);

	// Save settings to disk
	saveSettings_Groups();

	// Log info to log file
	logEvent("Delete Group",cGroup.strGroupName);

    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Reload GroupList combobox filter with list of groups in 'Groups' tab
//===============================================================================================
void PatProdRecipe_Admin::refreshGroupsListCombo(void)
{
	int		    iEntry;
	CGroupInfo	cEntry;
	int iTotalGroups = ui.tableWidgetGroups->rowCount();

	// Empty combobox
	ui.comboBoxGroupList->clear();

	// First selection
	ui.comboBoxGroupList->addItem("All groups");

	// Fill list
	for(iEntry = 0;iEntry < iTotalGroups; iEntry++)
	{
		// Read group profile from GUI
		readGroupList_gui(iEntry,cEntry);

		// Add entry to combo
		ui.comboBoxGroupList->addItem(cEntry.strGroupName);
	}
}

// Returns list of Groups available.
QStringList PatProdRecipe_Admin::strlGetGroupsList(void)
{
	QStringList strGroups;
	int iIndex;
	CGroupInfo	cEntry;
	for(iIndex = 0; iIndex < ui.tableWidgetGroups->rowCount(); iIndex++)
	{
		// Read group profile from GUI
		readGroupList_gui(iIndex,cEntry);

		strGroups << cEntry.strGroupName;
	}

	return strGroups;
}

//===============================================================================================
// Structure holding group settings: constructor
//===============================================================================================
CGroupInfo::CGroupInfo()
{
	strGroupName="";	    // Group name
}

//===============================================================================================
// Write Group list (and their settings) to GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::writeGroupList_gui(int iRow,CGroupInfo cEntry)
{
    QString strString;
    int	    iCol=0;

    // Check if need to resize the table
	if(ui.tableWidgetGroups->rowCount() < iRow+1)
		ui.tableWidgetGroups->setRowCount(iRow+1);

	QTableWidgetItem *pItem = new QTableWidgetItem(cEntry.strGroupName);
    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui.tableWidgetGroups->setItem(iRow, iCol, pItem);
    iCol++;

    // Success
    return true;
}

//===============================================================================================
// Read Groups profile from GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::readGroupList_gui(int iRow,CGroupInfo &cEntry)
{
    QString strString;
    int	    iCol=0;

    // Check if row# out of range!
	if(iRow < 0 || iRow >= ui.tableWidgetGroups->rowCount())
		return false;

	// Group name
	cEntry.strGroupName = ui.tableWidgetGroups->item(iRow,iCol)->text();
    iCol++;

    // Success
    return true;
}

//===============================================================================================
// Load list of Groups (and their settings)
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_Groups(void)
{
    QString	    strString;
    int		    iEntry;
	CGroupInfo	cEntry;

	// Open .ini groups file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

	// Reset group list combobox
	ui.comboBoxGroupList->clear();

    // Read all entries
	int iTotalGroups = settings.value("global/total_groups", 0).toInt();
	for(iEntry = 0;iEntry < iTotalGroups; iEntry++)
    {
		// Seek to relevant Group: 'group_X'
		strString.sprintf("group_%d",iEntry);
		settings.beginGroup(strString);

		cEntry.strGroupName = settings.value("name","").toString();

		// Close group seek.
		settings.endGroup();

		// Add entry details to GUI...unless invalid entry (no name specified)
		if(cEntry.strGroupName.isEmpty() == false)
			writeGroupList_gui(iEntry,cEntry);
    };
    // Sort table
	ui.tableWidgetGroups->sortItems(0); // Sort over first column (group name)

	// Reload Groups list in filter combo-box
	refreshGroupsListCombo();
}

//===============================================================================================
// Save list of groups (and their settings) to disk
//===============================================================================================
void PatProdRecipe_Admin::saveSettings_Groups(void)
{
    QString	    strString;
    int		    iEntry;
	CGroupInfo	cEntry;

	// Open .ini groups file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

	// Total number of group profiles to dump to disk.
	int iTotalGroups = ui.tableWidgetGroups->rowCount();
	settings.setValue("global/total_groups", iTotalGroups);
	for(iEntry = 0;iEntry < iTotalGroups; iEntry++)
    {
		// Read group profile from GUI
		readGroupList_gui(iEntry,cEntry);

		// Seek to relevant Group: 'group_X'
		strString.sprintf("group_%d",iEntry);
		settings.beginGroup(strString);

		settings.setValue("name",cEntry.strGroupName);

		// Close group seek.
		settings.endGroup();
    };
}


