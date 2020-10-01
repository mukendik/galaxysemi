#include <QtGui>

#include "PatProdRecipe_admin.h"
#include "user_profile.h"


extern QApplication app;


//===============================================================================================
// Check if valid password
//===============================================================================================
bool PatProdRecipe_Admin::checkPassword(CUserInfo &cUser)
{
    // If no users defined, then 'Admin/Admin' is default lomgin
    if(ui.tableWidgetUsers->rowCount() <= 0)
    {
		if((cUser.strUserName.compare("admin",Qt::CaseInsensitive) == 0) &&
		  (cUser.strPassword.compare("admin",Qt::CaseInsensitive) == 0))
		{
			cUser.iUserType = USER_RIGHTS_ADMIN;    // Admin privilege
			return true;    // Valid default Admin login.
		}
    }

    // Encrypt password so to check over login file
    cUser.strPassword = cUser.crypt(cUser.strPassword);

    // Scan list of user...
    int	iUser;
    QString strUsernameInList,strPasswordInList;
    int iTotalRows = ui.tableWidgetUsers->rowCount();
    for(iUser = 0; iUser < iTotalRows; iUser++)
    {
	strUsernameInList = ui.tableWidgetUsers->item(iUser,0)->text();
	strPasswordInList = ui.tableWidgetUsers->item(iUser,1)->text();
	cUser.iUserType = ui.tableWidgetUsers->item(iUser,2)->text().toInt();

	if((cUser.strUserName== strUsernameInList) && (cUser.strPassword == strPasswordInList))
	    return true;    // Valid login found.
    }

    return false;
}

//===============================================================================================
// Add User profile
//===============================================================================================
void PatProdRecipe_Admin::onAddUser(void)
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

	CUserProfile cUserDialog;
    CUserInfo	cUser;
    if(cUserDialog.exec() != 1)
	return;

    // Read entered user details
    cUserDialog.readUser(cUser,true);

    // Add user entry to list....unless already in list!
    int	iUser;
    int iTotalRows = ui.tableWidgetUsers->rowCount();
    for(iUser = 0; iUser < iTotalRows; iUser++)
    {
		if(cUser.strUserName.compare(ui.tableWidgetUsers->item(iUser,0)->text(),Qt::CaseInsensitive) == 0)
		{
			QMessageBox::warning(this,"New user","This user already exists!",QMessageBox::Ok);
			return;
		}
    }

    // Ok, new entry is valid, add it to the list!
    ui.tableWidgetUsers->setSortingEnabled(false);	// Disable sorting until we've added all fields to table!
    writeUserList_gui(iTotalRows,cUser);

    // Update list sorting
    ui.tableWidgetUsers->setSortingEnabled(true);	// Disable sorting unti lwe've added all fields to table!
    ui.tableWidgetUsers->sortItems(0);

	// Save Users tab
	saveSettings_Users();

	// Log event
	logEvent("Add User",cUser.strUserName);

    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Properties: edit User profile
//===============================================================================================
void PatProdRecipe_Admin::onPropertiesUser(void)
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
    int iRow = ui.tableWidgetUsers->currentRow();
    if(iRow < 0)
	return;

    // Read USer profile from GUI list
    CUserInfo cUser;
    if(readUserList_gui(iRow,cUser) == false)
	return;

    // Show/Edit entry
    CUserProfile cUserDialog;
    cUserDialog.writeUser(cUser);
    if(cUserDialog.exec() != 1)
	return;

    // Update Gui list
    ui.tableWidgetUsers->setSortingEnabled(false);	// Disable sorting until we've added all fields to table!
    cUserDialog.readUser(cUser,false);
    writeUserList_gui(iRow,cUser);
    ui.tableWidgetUsers->setSortingEnabled(true);	// Enable back sorting
    ui.tableWidgetUsers->sortItems(0);

	// Save Users tab
	saveSettings_Users();

	// Log event
	logEvent("Edit User",cUser.strUserName);

    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Remove User profile
//===============================================================================================
void PatProdRecipe_Admin::onRemoveUser(void)
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
    int iRow = ui.tableWidgetUsers->currentRow();
    if(iRow < 0)
		return;

    // Request confirmation before delete...
	CUserInfo cUser;
	cUser.strUserName = ui.tableWidgetUsers->item(iRow,0)->text();
	QString strText = "Delete user '" + cUser.strUserName + "' ?";
    if(QMessageBox::question(this,"Confirm delete",strText,QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) != QMessageBox::Yes)
		return;

    // Delete current line
    ui.tableWidgetUsers->removeRow(iRow);

	// Save Users tab
	saveSettings_Users();

	// Log event
	logEvent("Delete User",cUser.strUserName);

    // Flag changes made
    m_bChanges = true;
}

//===============================================================================================
// Structure holding user settings: constructor
//===============================================================================================
CUserInfo::CUserInfo()
{
    strUserName="";	    // Username
    strPassword="";	    // Password
    iUserType=0;	    // 0=standard user (read-only)
}

//===============================================================================================
// Encrypt string
//===============================================================================================
QString CUserInfo::crypt(QString strString)
{
    QString strCryptedHexString;
    QCryptographicHash hash(QCryptographicHash::Md4);
    QByteArray string(strString.toAscii());
    hash.addData(string);
    string=hash.result();

    // Convert each Qbyte to a 2 digits Hexa number (00 to FF)
    int		iIndex;
    unsigned char uValue;
    QString strTemp;
    for(iIndex=0;iIndex < string.count();iIndex++)
    {
	uValue = string.at(iIndex);
	strCryptedHexString += strTemp.sprintf("%02x",uValue);
    }
    return strCryptedHexString;
}

//===============================================================================================
// Write Users list (and their settings) to GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::writeUserList_gui(int iRow,CUserInfo cEntry)
{
    QString strString;
    int	    iCol=0;

    // Check if need to resize the table
    if(ui.tableWidgetUsers->rowCount() < iRow+1)
	ui.tableWidgetUsers->setRowCount(iRow+1);

    QTableWidgetItem *pItem = new QTableWidgetItem(cEntry.strUserName);
    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui.tableWidgetUsers->setItem(iRow, iCol, pItem);
    iCol++;

    // Password
    pItem = new QTableWidgetItem(cEntry.strPassword);
    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui.tableWidgetUsers->setItem(iRow, iCol, pItem);
    iCol++;

    // User type (read-only, admin, etc...)
    pItem = new QTableWidgetItem(QString::number(cEntry.iUserType));
    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui.tableWidgetUsers->setItem(iRow, iCol, pItem);
    iCol++;

    // Success
    return true;
}

//===============================================================================================
// Read Users profile from GUI (Table widget)
//===============================================================================================
bool PatProdRecipe_Admin::readUserList_gui(int iRow,CUserInfo &cEntry)
{
    QString strString;
    int	    iCol=0;

    // Check if row# out of range!
    if(iRow < 0 || iRow >= ui.tableWidgetUsers->rowCount())
	return false;

    // User name
    cEntry.strUserName = ui.tableWidgetUsers->item(iRow,iCol)->text();
    iCol++;

    // Password
    cEntry.strPassword = ui.tableWidgetUsers->item(iRow,iCol)->text();
    iCol++;

    // User type (read-only, admin, etc...)
    strString = ui.tableWidgetUsers->item(iRow,iCol)->text();
    cEntry.iUserType = strString.section('.',0,0).toInt();
    iCol++;

    // Success
    return true;
}

//===============================================================================================
// Load list of Users (and their settings)
//===============================================================================================
void PatProdRecipe_Admin::loadSettings_Users(void)
{
    QString	    strString;
    int		    iEntry;
    CUserInfo	    cEntry;

    // Open .ini users file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

    // Read all entries
    int iTotalUsers = settings.value("global/total_users", 0).toInt();
    for(iEntry = 0;iEntry < iTotalUsers; iEntry++)
    {
	// Seek to relevant Group: 'user_X'
	strString.sprintf("user_%d",iEntry);
	settings.beginGroup(strString);

	cEntry.strUserName = settings.value("name","").toString();
	cEntry.strPassword = settings.value("log","").toString();
	cEntry.iUserType = settings.value("type",0).toInt();

	// Close group seek.
	settings.endGroup();

	// Add entry details to GUI...unless invalid entry (no name specified)
	if(cEntry.strUserName.isEmpty() == false)
	    writeUserList_gui(iEntry,cEntry);
    };
    // Sort table
    ui.tableWidgetUsers->sortItems(0); // Sort over first column (user name)
}

//===============================================================================================
// Save list of users (and their settings) to disk
//===============================================================================================
void PatProdRecipe_Admin::saveSettings_Users(void)
{
    QString	    strString;
    int		    iEntry;
    CUserInfo	    cEntry;

    // Open .ini users file.
	QSettings settings(m_strPROD_RecipesIni,QSettings::IniFormat);

    // Total number of user profiles to dump to disk.
    int iTotalUsers = ui.tableWidgetUsers->rowCount();
    settings.setValue("global/total_users", iTotalUsers);
    for(iEntry = 0;iEntry < iTotalUsers; iEntry++)
    {
	// Read user profile from GUI
	readUserList_gui(iEntry,cEntry);

	// Seek to relevant Group: 'user_X'
	strString.sprintf("user_%d",iEntry);
	settings.beginGroup(strString);

	settings.setValue("name",cEntry.strUserName);
	settings.setValue("log",cEntry.strPassword);
	settings.setValue("type",cEntry.iUserType);

	// Close group seek.
	settings.endGroup();
    };
}


