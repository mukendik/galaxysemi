/****************************************************************************
** Deriven from gexftp_settings_widget_base.cpp
****************************************************************************/
#include <gqtl_sysutils.h>

#include "gexftp_settings_widget.h"
#include "gexftp_missingfield_dialog.h"
#include "gexftp_constants.h"
#include "gexftp_passwordconfirmation_dialog.h"
#include "gexftp_calendar_dialog.h"
#include "gexftp_settings.h"
#include "gexftp_browse_dialog.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <QCalendarWidget>
#include <QThreadPool>


/////////////////////////////////////////////////////////////////////////////////////
// Constructs a CGexFtpSettings_widget as a child of 'parent', with the
// name 'name' and widget flags set to 'f'.
//
// The dialog will by default be modeless, unless you set 'modal' to
// true to construct a modal dialog.
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpSettings_widget::CGexFtpSettings_widget(CGexFtpSettings * pDataSettings, QWidget* parent, Qt::WindowFlags fl) : QWidget(parent, fl), m_pDataSettings(pDataSettings)
{
	// Setup UI
	setupUi(this);

	// Make sure server settings are inserted at the bottom of the combo box
	comboProfileName->setInsertPolicy(QComboBox::InsertAtBottom);

	// Insert "New settings" item
	comboProfileName->insertItem(0, GEXFTP_NEW_SETTING);

	// Make sure the URLs are inserted at the bottom of the combo box
	comboURL->setInsertPolicy(QComboBox::InsertAtBottom);

	// Insert items of the file policy combo
	comboFilePolicy->insertItem(0, GEXFTP_FILEPOLICY_RENAME_TEXT);
	comboFilePolicy->insertItem(1, GEXFTP_FILEPOLICY_REMOVE_TEXT);
	comboFilePolicy->insertItem(2, GEXFTP_FILEPOLICY_LEAVE_TEXT);
    comboFilePolicy->setCurrentIndex(2);

    // Insert items of the download order combo
    comboDownloadOrder->insertItem(0, GEXFTP_DOWNLOAD_OLDEST_TO_NEWEST);
    comboDownloadOrder->insertItem(1, GEXFTP_DOWNLOAD_NEWEST_TO_OLDEST);

    // Init "Check age of Files" checkbox
	checkBoxDisabledDateTimeCheck->setChecked(false);
	
	// Init Next/Prev buttons
	buttonNextProfile->setEnabled(false);
	buttonPrevProfile->setEnabled(false);

	// Init 'Apply' button
	buttonApply->setEnabled(false);
	m_pDataSettings->endProfileChange();
	

	// Init search mode (recursive or not)
	checkBoxRecurse->setChecked(false);

	// Update Form
	OnNewProfileSelected(GEXFTP_NEW_SETTING);

	td_constitProfile itProfile = m_pDataSettings->dataFtp().lstProfile().constBegin();

	while (itProfile != m_pDataSettings->dataFtp().lstProfile().constEnd())
	{
		AddServerToList((*itProfile), true);

		itProfile++;
	}

	// Hide test button
	pushButtonTestRemoteFolder->setVisible(false);
	
	// signals and slots connections
    connect( buttonBrowseLocalDirectory,	SIGNAL( clicked() ),								this,			SLOT( OnButtonBrowseLocalFolder() ) );
	connect( buttonBrowseRemoteDirectory,	SIGNAL( clicked() ),								this,			SLOT( onButtonBrowseRemoteFolder() ) );
	connect( pushButtonTestRemoteFolder,	SIGNAL( clicked() ),								this,			SLOT( onButtonTestRemoteFolder() ) );
    connect( buttonApply,				SIGNAL( clicked() ),								this,			SLOT( OnButtonApply() ) );
    connect( this,						SIGNAL( sLocalDirectoryChanged(const QString &) ),	editLocalDir,	SLOT( setText(const QString&) ) );
    connect( buttonDelete,				SIGNAL( clicked() ),								this,			SLOT( OnButtonDelete() ) );
    connect( buttonHelp,				SIGNAL( clicked() ),								this,			SLOT( OnButtonHelp() ) );
    connect( editSettingsName,			SIGNAL( textChanged(const QString&) ),				this,			SLOT( OnProfileModified() ) );
    connect( checkBoxProfileEnabled,	SIGNAL( stateChanged(int) ),						this,			SLOT( OnProfileModified() ) );
    connect( comboURL,					SIGNAL( editTextChanged(QString)),					this,			SLOT( OnProfileModified() ) );
    connect( spinPortNb,				SIGNAL( valueChanged(int) ),						this,			SLOT( OnProfileModified() ) );
    connect( editLogin,					SIGNAL( textChanged(const QString&) ),				this,			SLOT( OnProfileModified() ) );
    connect( editPassword,				SIGNAL( textChanged(const QString&) ),				this,			SLOT( OnProfileModified() ) );
    connect( editRemoteDir,				SIGNAL( textChanged(const QString&) ),				this,			SLOT( OnProfileModified() ) );
    connect( editLocalDir,				SIGNAL( textChanged(const QString&) ),				this,			SLOT( OnProfileModified() ) );
    connect( editFileExtensions,		SIGNAL( textChanged(const QString&) ),				this,			SLOT( OnProfileModified() ) );
    connect( comboFilePolicy,			SIGNAL( activated(int) ),							this,			SLOT( OnProfileModified() ) );
    connect( comboDownloadOrder,		SIGNAL( activated(int) ),							this,			SLOT( OnProfileModified() ) );
    connect( checkBoxDisabledDateTimeCheck,SIGNAL( stateChanged(int)),						this,			SLOT( OnProfileModified() ) );
	connect( checkBoxDisabledDateTimeCheck,SIGNAL( clicked(bool)),							groupBoxDateWindow,SLOT( setDisabled(bool)) );
    connect( checkBoxDisabledDateTimeCheck,SIGNAL( clicked(bool)),							comboDownloadOrder,SLOT( setDisabled(bool)) );
    connect( groupBoxDateWindow,		SIGNAL( toggled(bool) ),							this,			SLOT( OnProfileModified() ) );
    connect( dateTimeEditFrom,			SIGNAL( dateTimeChanged(const QDateTime&) ),		this,			SLOT( OnProfileModified() ) );
    connect( dateTimeEditTo,			SIGNAL( dateTimeChanged(const QDateTime&) ),		this,			SLOT( OnProfileModified() ) );
	connect( checkBoxRecurse,			SIGNAL( stateChanged(int) ),						this,			SLOT( OnProfileModified() ) );
    connect( buttonFromCalendar,		SIGNAL( clicked() ),								this,			SLOT( OnButtonFromCalendar() ) );
    connect( buttonToCalendar,			SIGNAL( clicked() ),								this,			SLOT( OnButtonToCalendar() ) );
    connect( dateTimeEditFrom,			SIGNAL( dateTimeChanged(const QDateTime&) ),		this,			SLOT( OnFromDateChanged() ) );
    connect( dateTimeEditTo,			SIGNAL( dateTimeChanged(const QDateTime&) ),		this,			SLOT( OnToDateChanged() ) );
	connect( comboProfileName,			SIGNAL( activated(QString)),						this,			SLOT(OnNewProfileSelected(QString)));
    connect( buttonNextProfile,			SIGNAL( clicked()),									this,			SLOT(OnButtonNextProfile()));
    connect( buttonPrevProfile,			SIGNAL( clicked()),									this,			SLOT(OnButtonPrevProfile()));
	
	buttonFromCalendar->setVisible(false);
	buttonToCalendar->setVisible(false);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
CGexFtpSettings_widget::~CGexFtpSettings_widget()
{
    // no need to delete child widgets, Qt does it all for us
}

/////////////////////////////////////////////////////////////////////////////////////
// Handle key events to detect 'Enter' key on URL combo
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::keyPressEvent(QKeyEvent * e)
{
	if(((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) && (comboURL->hasFocus() == true))
	{
		// Enter/Return key has been pressed while in comboURL. Set focus on next widget (Login) and accept event.
		editLogin->setFocus();
		e->accept();
	}
	else
		e->ignore();
}

/////////////////////////////////////////////////////////////////////////////////////
// Validate server settings
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpSettings_widget::ValidateServer(const CGexFtpServer& clFtpServer, bool bDisplayMsg)
{
	CGexFtpMissingField_dialog dlg("", this);

	// Check validity of Setting Name field
	if(clFtpServer.settingsName().isEmpty())
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelProfileName->text());
			dlg.exec();
			editSettingsName->setFocus();
		}
		return false;
	}

	// Check validity of Ftp site field
	if(clFtpServer.ftpSiteURL().isEmpty())
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelURL->text());
			dlg.exec();
			comboURL->setFocus();
		}
		return false;
	}

	// Port Number
	if((clFtpServer.ftpPortNb() <= 0) || (clFtpServer.ftpPortNb() > 999))
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelPort->text());
			dlg.exec();
			spinPortNb->setFocus();
		}
		return false;
	}

	// Check validity of Login field
	if(clFtpServer.login().isEmpty())
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelLogin->text());
			dlg.exec();
			editLogin->setFocus();
		}
		return false;
	}

	// Check validity of Password field
	if(clFtpServer.password().isEmpty())
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelPassword->text());
			dlg.exec();
			editPassword->setFocus();
		}
		return false;
	}

	// Remote directory can be empty

	// Check validity of Local directory field
	if(clFtpServer.localDir().isEmpty())
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelLocalDir->text());
			dlg.exec();
			buttonBrowseLocalDirectory->setFocus();
		}
		return false;
	}

	// Check validity of File extensions field
	if(clFtpServer.fileExtensions().isEmpty())
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelFileExtensions->text());
			dlg.exec();
			editFileExtensions->setFocus();
		}
		return false;
	}

	// Check validity of File Policy field
	if(clFtpServer.filePolicy().isEmpty() || ((clFtpServer.filePolicy() != GEXFTP_FILEPOLICY_RENAME_TEXT) && (clFtpServer.filePolicy() != GEXFTP_FILEPOLICY_REMOVE_TEXT) && (clFtpServer.filePolicy() != GEXFTP_FILEPOLICY_LEAVE_TEXT)))
	{
		if(bDisplayMsg)
		{
			dlg.SetFieldName(labelFilePolicy->text());
			dlg.exec();
			comboFilePolicy->setFocus();
		}
		return false;
	}

    // Check validity of Download Order field
    if(clFtpServer.downloadOrder().isEmpty() || ((clFtpServer.downloadOrder() != GEXFTP_DOWNLOAD_OLDEST_TO_NEWEST) && (clFtpServer.downloadOrder() != GEXFTP_DOWNLOAD_NEWEST_TO_OLDEST)))
    {
        if(bDisplayMsg)
        {
            dlg.SetFieldName(labelSorting->text());
            dlg.exec();
            comboDownloadOrder->setFocus();
        }
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Add a server to the list
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::AddServerToList(const CGexFtpServer& clFtpServer, bool bFillSettingsForm)
{
	// Validate settings
	if(ValidateServer(clFtpServer, false) == false)
		return;
	
	// Fill the form?
	if(bFillSettingsForm)
	{
		int nIndex = comboURL->findText(clFtpServer.ftpSiteURL(), Qt::MatchExactly);

		if (nIndex != -1)
			comboURL->setCurrentIndex(nIndex);
		else
		{
			comboURL->insertItem(comboURL->count(), clFtpServer.ftpSiteURL());
			comboURL->setCurrentIndex(comboURL->count()-1);
		}

		editSettingsName->setText(clFtpServer.settingsName());
		spinPortNb->setValue(clFtpServer.ftpPortNb());
		editLogin->setText(clFtpServer.login());
		editPassword->setText(clFtpServer.password());
		editRemoteDir->setText(clFtpServer.remoteDir());
		checkBoxRecurse->setChecked(clFtpServer.recursiveSearch());
		editLocalDir->setText(clFtpServer.localDir());
		editFileExtensions->setText(clFtpServer.fileExtensions());
		comboFilePolicy->setCurrentIndex(comboFilePolicy->findText(clFtpServer.filePolicy(), Qt::MatchExactly));
        comboDownloadOrder->setCurrentIndex(comboDownloadOrder->findText(clFtpServer.downloadOrder(), Qt::MatchExactly));
        checkBoxProfileEnabled->setChecked(clFtpServer.profileEnabled());
		groupBoxDateWindow->setChecked(clFtpServer.useDateTimeWindow());
		dateTimeEditFrom->setDateTime(clFtpServer.dateFrom());
		labelFromDateTime->setText(dateTimeEditFrom->dateTime().toString());
		dateTimeEditTo->setDateTime(clFtpServer.dateTo());
		labelToDateTime->setText(dateTimeEditTo->dateTime().toString());
		checkBoxDisabledDateTimeCheck->setChecked(clFtpServer.disabledDateTimeCheck());
		groupBoxDateWindow->setDisabled(clFtpServer.disabledDateTimeCheck());
		// Disable 'Apply' button
		buttonApply->setEnabled(false);
		m_pDataSettings->endProfileChange();
	}

	if(comboProfileName->findText(clFtpServer.settingsName(), Qt::MatchExactly) == -1)
	{
		comboProfileName->insertItem(comboProfileName->count(), clFtpServer.settingsName());
		comboProfileName->setCurrentIndex(comboProfileName->count()-1);
	}
		
	// Update Next/Prev buttons
	updateProfileButton();
	
	// If new URL, add Server name to the URL combo
	AddToComboURL(clFtpServer.ftpSiteURL());

	// Enable Delete button
	buttonDelete->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// Browse to select a local folder to store Ftp files retrieved
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonBrowseLocalFolder()
{
	// Display Open file dialog so the user can select a folder
	QString strFileName = QFileDialog::getExistingDirectory(this, "Select the directory where to store downloaded files", editLocalDir->text());
	// Check if new file selected
	if((strFileName.isEmpty() == true) || (editLocalDir->text() == strFileName))
		return;

	SetLocalDirectory(strFileName);
}

/////////////////////////////////////////////////////////////////////////////////////
// Browse to select a remote folder to get Ftp files
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::onButtonBrowseRemoteFolder()
{
	GexFtpBrowseDialogArgs args;

    args.strCaption = "Select directory";
    args.strStartDirectory = editRemoteDir->text();
	args.strFtpSiteURL = comboURL->currentText().trimmed();
	args.iFtpPortNb = spinPortNb->value();
	args.strLogin = editLogin->text().trimmed();
	args.strPassword = editPassword->text().trimmed();
	args.bAllowCustomContextMenu = false;
		
	QString strDirName = GexFtpBrowseDialog::getExistingFtpDirectory(args);
	
	if((strDirName.isEmpty()) || (editRemoteDir->text() == strDirName ))
		return;

	editRemoteDir->setText(strDirName);
}


/////////////////////////////////////////////////////////////////////////////////////
// Browse to select a remote folder to get Ftp files
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::onButtonTestRemoteFolder()
{
	GexFtpBrowseDialogArgs args;
	args.parent = this;
	args.strCaption = "Select directory";
	args.strStartDirectory = editRemoteDir->text();
	args.strFtpSiteURL = comboURL->currentText();
	args.iFtpPortNb = spinPortNb->value();
	args.strLogin = editLogin->text();
	args.strPassword = editPassword->text();
	args.iTimerInterval = m_pDataSettings->dataGeneral().ftpTimeoutAfter() * 1000;
		
	QString strDir = editRemoteDir->text();
	QString strError;
	if( GexFtpBrowseDialog::isValidFtpDirectory(strDir, args, strError))
	{
		QMessageBox msgBox(QMessageBox::Warning, "Dir OK", strDir, QMessageBox::Ok|QMessageBox::Cancel, this);
		msgBox.exec();
	}
	else
	{
		QMessageBox msgBox(QMessageBox::Warning, "Bad Dir", strError, QMessageBox::Ok|QMessageBox::Cancel, this);
		msgBox.exec();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Set local directory
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::SetLocalDirectory(const QString& strLocalDirectory)
{
	QString strDirectory = strLocalDirectory;
	
	CGexSystemUtils::NormalizePath(strDirectory);

	// Emit signal telling a new local directory has been selected
	emit sLocalDirectoryChanged(strDirectory);
}

/////////////////////////////////////////////////////////////////////////////////////
// Save current Ftp server settings
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpSettings_widget::OnButtonApply()
{
	// Create new Ftp Server setting
	CGexFtpServer NewServer;
	QString strErrorMessage = "";

	if (!NewServer.setSettingsName(editSettingsName->text(),strErrorMessage))
	{
		QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}
	if (!NewServer.setFtpSiteURL(comboURL->currentText(),strErrorMessage))
	{
		QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}
	if (!NewServer.setLogin(editLogin->text(),strErrorMessage))
	{
		QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}
	if (!NewServer.setPassword(editPassword->text(),strErrorMessage))
	{
		QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}
	if (!NewServer.setLocalDir(editLocalDir->text(),strErrorMessage))
	{
        QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}
	if (!NewServer.setFileExtensions(editFileExtensions->text(),strErrorMessage))
	{
		QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}
	if (!NewServer.setRemoteDir(editRemoteDir->text(),strErrorMessage))
	{
		QMessageBox::warning(this, "Unable to apply settings", strErrorMessage);
		return false;
	}

	NewServer.setFtpPortNb(spinPortNb->value());
	NewServer.setFilePolicy(comboFilePolicy->currentText());
    NewServer.setDownloadOrder(comboDownloadOrder->currentText());
    NewServer.setProfileEnabled(checkBoxProfileEnabled->isChecked());
	NewServer.setUseDateTimeWindow(groupBoxDateWindow->isChecked());
	NewServer.setDateFrom(dateTimeEditFrom->dateTime());
	NewServer.setDateTo(dateTimeEditTo->dateTime());
	NewServer.setRecursiveSearch(checkBoxRecurse->isChecked());
	NewServer.setDisabledDateTimeCheck(checkBoxDisabledDateTimeCheck->isChecked());

	// Validate settings
	if(ValidateServer(NewServer, true) == false)
		return false;
	
	// All fields are valid.

	// Set focus on Settings name field
	editSettingsName->setFocus();

	// First check if an entry with the same name exists, if so ask if we should overwrite it
	td_itProfile itProfileFound;

	if (m_pDataSettings->dataFtp().findProfile(NewServer.settingsName(), itProfileFound))
	{
		// If password was modified, ask user to confirm it
		if(((*itProfileFound).password() != NewServer.password()) && (ConfirmPassword(NewServer.password()) == false))
			return false;
	}
	else
	{
		// Ask user to confirm it
		if(ConfirmPassword(NewServer.password()) == false)
			return false;
	}

	m_pDataSettings->dataFtp().addProfile(NewServer);

	// Update 'Apply' button
	buttonApply->setEnabled(false);
	m_pDataSettings->endProfileChange();
	
	// New server will be added to server list
	AddServerToList(NewServer, false);

	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Ask user to confirm password. Return true if same password entered, false else
/////////////////////////////////////////////////////////////////////////////////////
bool CGexFtpSettings_widget::ConfirmPassword(const QString & strPassword)
{
	CGexFtpPasswordConfirmation_dialog dlg(this);
	dlg.exec();
	QString strNewPassword = dlg.GetPassword().trimmed();
	if(strNewPassword != strPassword)
	{
		QMessageBox::warning(this, "Ftp Settings", "The password doesn't match.", QMessageBox::Ok | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton);
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Clear page
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::ClearSettingsForm()
{
	editSettingsName->clear();
	comboURL->clearEditText();
	editLogin->clear();
	editPassword->clear();
	editRemoteDir->clear();
	editLocalDir->clear();
	editFileExtensions->setText(GEXFTP_DEFAULT_EXTENSIONS);
	spinPortNb->setValue(GEXFTP_DEFAULT_PORT_NB);
    comboFilePolicy->setCurrentIndex(2);
    comboDownloadOrder->setCurrentIndex(comboDownloadOrder->findText(GEXFTP_DOWNLOAD_OLDEST_TO_NEWEST, Qt::MatchExactly));
    checkBoxProfileEnabled->setChecked(true);
	checkBoxRecurse->setChecked(false);
	checkBoxDisabledDateTimeCheck->setChecked(false);
	groupBoxDateWindow->setDisabled(false);
	dateTimeEditFrom->setDate(QDate::currentDate().addMonths(-1));
	dateTimeEditTo->setDate(QDate::currentDate().addMonths(1));
}

/////////////////////////////////////////////////////////////////////////////////////
// The 'Next profile' button has been clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonNextProfile()
{
	int nCurrentItem = comboProfileName->currentIndex();

	if(nCurrentItem < (comboProfileName->count()-1))
	{
		comboProfileName->setCurrentIndex(nCurrentItem+1);
		OnNewProfileSelected(comboProfileName->currentText());
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// The 'Previous profile' button has been clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonPrevProfile()
{
	int nCurrentItem = comboProfileName->currentIndex();

	if(nCurrentItem > 0)
	{
		comboProfileName->setCurrentIndex(nCurrentItem-1);
		OnNewProfileSelected(comboProfileName->currentText());
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// The selection in the profile combo gas changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnNewProfileSelected(const QString & strSettingsName)
{
	// Set focus on Settings name field
	editSettingsName->setFocus();

	// Check if different profile was really selected
	if(strSettingsName == editSettingsName->text())
		return;

	// Update Next/Prev buttons
	updateProfileButton();
	
	// Has the previous profile been modified??
	if(buttonApply->isEnabled())
	{
		if(QMessageBox::warning(this, "Ftp Settings", "The current Ftp profile has been modified.\nDo you want to apply those changes?", QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape) == QMessageBox::Yes)
			OnButtonApply();
	}

	// Check if "New settings" selected 
	if(strSettingsName == GEXFTP_NEW_SETTING)
	{
		// Clear settings form
		ClearSettingsForm();
		
		// Disable Delete button
		buttonDelete->setEnabled(false);

		// Disable 'Apply' button
		buttonApply->setEnabled(false);
		m_pDataSettings->endProfileChange();

		return;
	}

	td_itProfile itProfileFound;

	if (m_pDataSettings->dataFtp().findProfile(strSettingsName, itProfileFound))
	{
		editSettingsName->setText((*itProfileFound).settingsName());
		comboURL->setCurrentIndex(comboURL->findText((*itProfileFound).ftpSiteURL(), Qt::MatchExactly));
		spinPortNb->setValue((*itProfileFound).ftpPortNb());
		editLogin->setText((*itProfileFound).login());
		editPassword->setText((*itProfileFound).password());
		editRemoteDir->setText((*itProfileFound).remoteDir());
		editLocalDir->setText((*itProfileFound).localDir());
		editFileExtensions->setText((*itProfileFound).fileExtensions());
		comboFilePolicy->setCurrentIndex(comboFilePolicy->findText((*itProfileFound).filePolicy(), Qt::MatchExactly));
        comboDownloadOrder->setCurrentIndex(comboDownloadOrder->findText((*itProfileFound).downloadOrder(), Qt::MatchExactly));
        checkBoxProfileEnabled->setChecked((*itProfileFound).profileEnabled());
		groupBoxDateWindow->setChecked((*itProfileFound).useDateTimeWindow());
		dateTimeEditFrom->setDateTime((*itProfileFound).dateFrom());
		labelFromDateTime->setText(dateTimeEditFrom->dateTime().toString());
		dateTimeEditTo->setDateTime((*itProfileFound).dateTo());
		labelToDateTime->setText(dateTimeEditTo->dateTime().toString());
		checkBoxRecurse->setChecked((*itProfileFound).recursiveSearch());
		checkBoxDisabledDateTimeCheck->setChecked((*itProfileFound).disabledDateTimeCheck());
	}

	groupBoxDateWindow->setDisabled(checkBoxDisabledDateTimeCheck->isChecked());
	
	// Enable Delete button
	buttonDelete->setEnabled(true);

	// Disable 'Apply' button
	buttonApply->setEnabled(false);
	m_pDataSettings->endProfileChange();

}

/////////////////////////////////////////////////////////////////////////////////////
// Add an item to the URL combo, if it does not already contain an identical
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::AddToComboURL(const QString& strURL)
{
	if(comboURL->findText(strURL, Qt::MatchExactly) == -1)
		comboURL->insertItem(comboURL->count(), strURL);
}

/////////////////////////////////////////////////////////////////////////////////////
// The Delete button has been clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonDelete()
{
	// Confirmation message box
	if(QMessageBox::warning(this, "Ftp Settings", "Do you really want to delete the current profile?", QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape) == QMessageBox::No)
		return;

        m_pDataSettings->dataFtp().removeProfile(editSettingsName->text());

	// Delete entry from settings combo box
	comboProfileName->removeItem(comboProfileName->currentIndex());
	OnNewProfileSelected(comboProfileName->currentText());
	
	// Update 'Apply' button
	buttonApply->setEnabled(false);
	m_pDataSettings->endProfileChange();
}

/////////////////////////////////////////////////////////////////////////////////////
// Current profile has been modified
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnProfileModified()
{
	buttonApply->setEnabled(true);
	m_pDataSettings->beginProfileChange();
}

/////////////////////////////////////////////////////////////////////////////////////
// The Help button has been clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonHelp()
{
	// Emit signal to display help section
	emit sDisplayHelp();
}

/////////////////////////////////////////////////////////////////////////////////////
// The 'From calendar' button has been clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonFromCalendar()
{
	// Create Calendar object
	CGexFtpCalendarDialog*	pCalendar	= new CGexFtpCalendarDialog();
	QDate					clDate		= dateTimeEditFrom->dateTime().date();
	pCalendar->setDate(clDate);

	// Show calendar, let user pick a date
	if(pCalendar->exec() != 1)
		return;

	// Read date picked by the user
	QDateTime clDateTime;
	clDateTime.setDate(pCalendar->getDate());
	clDateTime.setTime(dateTimeEditFrom->dateTime().time());
	dateTimeEditFrom->setDateTime(clDateTime);

	// Update GUI accordingly
	labelFromDateTime->setText(dateTimeEditFrom->dateTime().toString());
}

/////////////////////////////////////////////////////////////////////////////////////
// The 'To calendar' button has been clicked
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnButtonToCalendar()
{
	// Create Calendar object
	CGexFtpCalendarDialog *	pCalendar	= new CGexFtpCalendarDialog();
	QDate					clDate		= dateTimeEditFrom->dateTime().date();
	pCalendar->setDate(clDate);

	// Show calendar, let user pick a date
	if(pCalendar->exec() != 1)
		return;

	// Read date picked by the user
	QDateTime clDateTime;
	clDateTime.setDate(pCalendar->getDate());
	clDateTime.setTime(dateTimeEditTo->dateTime().time());
	dateTimeEditTo->setDateTime(clDateTime);

	// Update GUI accordingly
	labelToDateTime->setText(dateTimeEditTo->dateTime().toString());
}

/////////////////////////////////////////////////////////////////////////////////////
// The 'From date' changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnFromDateChanged()
{
	labelFromDateTime->setText(dateTimeEditFrom->dateTime().toString());
}

/////////////////////////////////////////////////////////////////////////////////////
// The 'To date' changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::OnToDateChanged()
{
	labelToDateTime->setText(dateTimeEditTo->dateTime().toString());
}

/////////////////////////////////////////////////////////////////////////////////////
// Update Previous/Next button
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings_widget::updateProfileButton()
{
	int nCurrentItem = comboProfileName->currentIndex();
	
	// Update previous button
	if(nCurrentItem > 0)
		buttonPrevProfile->setEnabled(true);
	else
		buttonPrevProfile->setEnabled(false);
	
	// Update next button
	if(nCurrentItem < (comboProfileName->count()-1))
		buttonNextProfile->setEnabled(true);
	else
		buttonNextProfile->setEnabled(false);
}


