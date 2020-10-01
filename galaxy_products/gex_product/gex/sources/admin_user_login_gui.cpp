///////////////////////////////////////////////////////////
// Database admin: Create/Delete database, insert files
///////////////////////////////////////////////////////////
#include <QShortcut>
#include <QMenu>
#include <QInputDialog>
#include <QSqlError>
#include <QMessageBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QRegExp>

#include "dir_access_base.h"
#include "admin_engine.h"
#include "message.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "admin_gui.h"
#include "mo_task.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_constants.h"
#include "pickuser_dialog.h"
#include "product_info.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "command_line_options.h"
#include <gqtl_log.h>
#include "libgexoc.h"
#include "csl/csl_engine.h"

// in main.cpp
//extern void                 WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *      pGexMainWindow;
extern CGexSkin*            pGexSkin;      // holds the skin settings
extern GexScriptEngine*     pGexScriptEngine;

// report_build.cpp
extern CReportOptions       ReportOptions;    // Holds options (report_build.h)

#include "read_system_info.h"

///////////////////////////////////////////////////////////
// Manage the Database Admin page
///////////////////////////////////////////////////////////
AdminUserLogin::AdminUserLogin(QWidget* parent,
                               bool /*modal*/,
                               Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);
    setModal(true);

    // Set Examinator skin
    if(pGexSkin)
        pGexSkin->applyPalette(this);

    connect(buttonBoxOk, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBoxOk, SIGNAL(rejected()), this, SLOT(onReject()));

    connect(comboBoxLogins, SIGNAL(activated(QString)), this, SLOT(onSelectLogin()));
    connect(comboBoxUserCreationType, SIGNAL(activated(QString)), this, SLOT(onSelectType()));

    m_bForceConnection = false;
    m_bAsAdmin = false;

    comboBoxGroup->hide();
    comboBoxProfile->hide();
    labelConnectionGroup->hide();
    labelConnectionProfile->hide();
    lineEditUserCreationLogin->hide();
    lineEditConfirmPassword->hide();
    labelConfirmPassWord->hide();

    QString strApplicationName = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    // Get the short name
    if(strApplicationName.count("-") > 1)
        strApplicationName = strApplicationName.section("-",0,1);
    else
        strApplicationName = strApplicationName.section("-",0,0);
    setWindowTitle(strApplicationName);
}


///////////////////////////////////////////////////////////
// GexYieldManDbLogin:
///////////////////////////////////////////////////////////
void AdminUserLogin::onAccept(void)
{
    if(pGexMainWindow == NULL)
        return;

    // User Connection
    if((tabWidget->currentWidget() == tabUserConnection)
            && (tabWidget->count() == 1))
    {
        CheckAdminConnection();
        return;
    }

    // User creation or User properties
    //((tabWidget->indexOf(tabUserCreation) > -1) || (tabWidget->indexOf(tabUserOptions) > -1))

    if(!comboBoxUserCreationGroup->isVisible())
        comboBoxUserCreationGroup->clear();

    if(!listWidgetUserCreationGroups->isVisible())
        listWidgetUserCreationGroups->clear();

    AdminUserGroup*  pGroup  = NULL;
    AdminUser*  pUser  = NULL;

    // User creation or User properties
    // Check if valid entry
    if(m_lstUsersId.isEmpty())
    {
        // For user creation
        // Check with login
        foreach(pUser, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers)
        {
            if(pUser->m_strLogin == lineEditUserCreationLogin->text())
                break;
            pUser = NULL;
        }
    }
    if(m_lstUsersId.count() == 1)
    {
        // for User properties
        foreach(pUser, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers)
        {
            if(pUser->m_nUserId == m_lstUsersId.first().toInt())
                break;
            pUser = NULL;
        }
    }

    if(tabWidget->indexOf(tabUserCreation) > -1)
    {
        // User creation or User properties
        QString strLogins;
        QString strName;
        QString strEmail;
        QString strPrimaryGroup;
        QString  strType;


        strLogins = lineEditUserCreationLogin->text();
        strName = lineEditUserCreationName->text();
        strEmail = lineEditUserCreationEmail->text();
        strType = comboBoxUserCreationType->currentText();
        strPrimaryGroup = comboBoxUserCreationGroup->currentText();

        if(!strPrimaryGroup.isEmpty())
        {
            QMap<int, AdminUserGroup*>::iterator itGroup;
            for(itGroup = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.begin(); itGroup != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.end(); itGroup++)
            {
                pGroup = itGroup.value();
                if(pGroup->m_strName == strPrimaryGroup)
                    break;
                pGroup = NULL;
            }
            if(pGroup == NULL)
            {
                // Invalid entry !!
                QMessageBox::warning(this,"User Properties",
                                     "Invalid entry !!");
                return;
            }
        }

        if(m_lstUsersId.isEmpty())
        {
            // For user creation

            // User creation
            // Check if valid entry
            if(pUser)
            {
                // Problem during the creation
                // Display the Error Message
                QMessageBox::warning(this,"User Creation",
                                     "Login already exist.\n"
                                     "Try another one, please.");
                return;
            }

            if(strName.isEmpty())
            {
                // Invalid Name
                QMessageBox::warning(this,"User Property",
                                     "No name specified!");
                return;
            }

            if(strEmail.isEmpty())
            {
                // Invalid Name
                QMessageBox::warning(this,"User Property",
                                     "No email specified!");
                return;
            }

            // Else create this new user
            pUser = new AdminUser;
            pUser->m_nUserId = 0;
            pUser->m_strLogin = strLogins;
            pUser->m_strPwd.clear();
            pUser->m_nProfileId = 0;

        }
        else
        {
            if(strEmail.isEmpty())
            {
                // Invalid Name
                QMessageBox::warning(this,"User Property",
                                     "No email specified!");
                return;
            }
        }

        // Save User properties
        pUser->m_strName = strName;
        pUser->m_strEmail = strEmail;

        if(pGroup
                && GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(pUser))
            pUser->m_nGroupId= pGroup->m_nGroupId;
        else
            pUser->m_nGroupId = 0;

        pUser->m_nType = GS::Gex::Engine::GetInstance().GetAdminEngine().StringToUserType(strType);

        if(tabWidget->indexOf(tabUserOptions) > -1)
            pUser->ResetAllAttributes();
    }

    if(tabWidget->indexOf(tabUserConnection) > -1)
    {
        QString strPassWord;
        QString strConfirmPassWord;
        strPassWord = lineEditPassword->text();
        strConfirmPassWord = lineEditPassword->text();

        // Check if have some Pwd
        // PassWord can be empty if reseted by an admin
        if(strPassWord.isEmpty() && (GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId == pUser->m_nUserId))
        {
            // Invalid passWord
            QMessageBox::warning(this,"User login", "Missing password !!");
            return;
        }

        // Check password
        if(lineEditConfirmPassword->isVisible())
        {
            strConfirmPassWord = lineEditConfirmPassword->text();
            if(strPassWord != strConfirmPassWord)
            {
                // Invalid passWord
                QMessageBox::warning(this,"User login",
                                     "Invalid confirm password !!");
                return;
            }
        }

        pUser->m_strPwd = strPassWord;

    }

    if(m_lstUsersId.isEmpty())
    {
        bool Status = GS::Gex::Engine::GetInstance().GetAdminEngine().SaveUser(pUser);
        pUser->TraceUpdate("CREATE","START","User creation");
        QString strError;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
        pUser->TraceUpdate("CREATE",(Status?"PASS":"FAIL"),(Status?"Created|Login="+pUser->m_strLogin:strError));
        if(!Status)
        {
            QString strError;
            GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
            QMessageBox::warning(this,"Creation error", strError);
            return;
        }
    }

    // Save users options
    // If all users are selected, apply this properties for all (userId=null)
    if(tableOptions->isEnabled())
    {

        bool bAskForOverWrite = (m_lstUsersId.count() > 1);
        if((m_lstUsersId.count()>1) &&
                (m_lstUsersId.count() ==  GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.count()))
        {
            QTableWidget* ptTable = tableOptions;
            ptTable->setFocus();
            for(int nCurrentRow = 0; nCurrentRow < ptTable->rowCount(); ++nCurrentRow){
                QString Option=ptTable->item(nCurrentRow,1)->text();
                QString Value =ptTable->item(nCurrentRow,2)->text();
                GS::Gex::Engine::GetInstance().GetAdminEngine().SetUserSettingsValue(
                            Option,
                            Value,
                            true);
            }
        }
        else
        {

            foreach(const QString &strUserId, m_lstUsersId)
            {
                pUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[strUserId.toInt()];
                if(pUser)
                {
                    // Save new user options
                    // Reset current user options
                    // Load user options from table

                    QTableWidget* ptTable = tableOptions;
                    ptTable->setFocus();

                    pUser->ResetAllAttributes();
                    for(int nCurrentRow = 0; nCurrentRow < ptTable->rowCount(); ++nCurrentRow)
                    {
                        if(bAskForOverWrite)
                        {
                            QString strValue = pUser->GetAttribute(ptTable->item(nCurrentRow,1)->text()).toString().trimmed();
                            QString strNewValue = ptTable->item(nCurrentRow,2)->text().trimmed();
                            if(!strValue.isEmpty() && !strNewValue.isEmpty() && (strValue != strNewValue))
                            {
                                // Ask to overwrite
                                QString strMessage = "The user "+pUser->m_strLogin+" already has this option.\n";
                                strMessage+= "Name = "+ptTable->item(nCurrentRow,1)->text()+"\n";
                                strMessage+= "Value = "+strValue+"\n\n";
                                strMessage+= "Do you want to overwrite it?";
                                QMessageBox mb("Replace option",
                                               strMessage,
                                               QMessageBox::Information,
                                               QMessageBox::Yes , QMessageBox::YesToAll,
                                               QMessageBox::No  | QMessageBox::Default | QMessageBox::Escape);
                                mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                                int eResult = mb.exec();
                                if(eResult == QMessageBox::YesToAll)
                                    bAskForOverWrite = false;
                                if (( eResult != QMessageBox::Yes )
                                        && (eResult != QMessageBox::YesToAll))
                                    continue;
                            }
                        }

                        pUser->SetAttribute(ptTable->item(nCurrentRow,1)->text(),QVariant(ptTable->item(nCurrentRow,2)->text()));
                    }
                    pUser->TraceUpdate("EDIT","START","Save properties");
                    bool Status = GS::Gex::Engine::GetInstance().GetAdminEngine().SaveUser(pUser);
                    QString strError;
                    GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);
                    pUser->TraceUpdate("EDIT",(Status?"PASS":"FAIL"),(Status?"Saved|Login="+pUser->m_strLogin:strError));
                }

                // Update Group/User association
                QString  strGroup;
                QList<QListWidgetItem*> lstItem = listWidgetUserCreationGroups->selectedItems();
                for(int iIndex=0; iIndex!=lstItem.count(); iIndex++)
                {
                    strGroup = lstItem.at(iIndex)->text();

                    QMap<int, AdminUserGroup*>::iterator itGroup;
                    for(itGroup = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.begin(); itGroup != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.end(); itGroup++)
                    {
                        pGroup = itGroup.value();
                        if(pGroup->m_strName == strGroup)
                        {
                            break;
                        }
                        pGroup = NULL;
                    }
                    if(pGroup == NULL)
                    {
                        // Invalid entry !!
                        continue;
                    }
                    if(!pGroup->m_lstUserIds.contains(pUser->m_nUserId))
                        pGroup->m_lstUserIds.append(pUser->m_nUserId);

                    GS::Gex::Engine::GetInstance().GetAdminEngine().SaveGroup(pGroup);

                }
            }
        }
    }

    accept();
}


void AdminUserLogin::CheckAdminConnection()
{

    GSLOG(SYSLOG_SEV_DEBUG, "GexYieldManDbLogin : onAccept Connection");

    QString strLogins;
    QString strPassWord;
    QString strConfirmPassWord;
    QString strGroup;
    QString strProfile;

    if(comboBoxLogins->isHidden())
        strLogins = lineEditUserCreationLogin->text().toLower().simplified();
    else
        strLogins = comboBoxLogins->currentText();
    // Check if the comboBoxLogins contains this string
    if(strLogins.isEmpty()
            || !SetCurrentComboItem(comboBoxLogins,strLogins))
    {
        // Invalid login
        QMessageBox::warning(this,"User login", "Invalid login !!");
        // Reset password
        lineEditUserCreationLogin->setText("");
        lineEditPassword->setText("");
        lineEditConfirmPassword->setText("");
        return;
    }

    // Check if user have to update Name or Email
    AdminUser*  pUser = NULL;
    foreach(pUser , GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers)
    {
        if(pUser->m_strLogin == strLogins)
            break;
        pUser = NULL;
    }
    // Only once
    if(pUser && labelUserCreationStatus->isHidden())
    {
        bool bUpdateName = pUser->m_strName.isEmpty();
        bool bUpdateMail = pUser->m_strEmail.isEmpty();
        QRegExp     RegExp;
        QStringList emails = pUser->m_strEmail.split(";");
        foreach(QString mail, emails)
        {
            mail = mail.simplified();
            if(mail.isEmpty())
                continue;
            RegExp.setPattern("^[_a-z0-9-]+(\\.[_a-z0-9-]+)*@[a-z0-9-]+(\\.[a-z0-9-]+)*(\\.[a-z]{2,4})$");
            RegExp.setCaseSensitivity(Qt::CaseInsensitive);
            bUpdateMail |= !RegExp.exactMatch(mail);
        }

        QString status;
        if(bUpdateName)
            status += "\n<BR>- update your name";

        if(bUpdateMail)
        {
            status += "\n<BR>- update your email";
            if(!pUser->m_strEmail.isEmpty())
                status += ": invalid email";
        }
        if(!status.isEmpty())
        {
            int line=5;
            if(labelUserCreationStatus->isHidden())
                gridLayout_2->addWidget(labelUserCreationStatus, line++, 0, 2, 2);
            labelUserCreationStatus->show();
            labelUserCreationStatus->setText("<b><font color=RED>Please fulfill your properties:</font></b>"+status);
            if(bUpdateName)
            {
                if(labelUserCreationName->isHidden())
                    gridLayout_2->addWidget(labelUserCreationName, line, 0, 1, 2);
                if(lineEditUserCreationName->isHidden())
                    gridLayout_2->addWidget(lineEditUserCreationName, line++, 1, 1, 2);
                labelUserCreationName->show();
                lineEditUserCreationName->show();
                lineEditUserCreationName->setEnabled(true);
                lineEditUserCreationName->setText(pUser->m_strName);
                lineEditUserCreationName->setFocus();
            }
            if(bUpdateMail)
            {
                if(labelUserCreationMail->isHidden())
                    gridLayout_2->addWidget(labelUserCreationMail, line, 0, 1, 2);
                if(lineEditUserCreationEmail->isHidden())
                    gridLayout_2->addWidget(lineEditUserCreationEmail, line++, 1, 2, 2);
                labelUserCreationMail->show();
                lineEditUserCreationEmail->show();
                lineEditUserCreationEmail->setEnabled(true);
                lineEditUserCreationEmail->setText(pUser->m_strEmail);
                if(!bUpdateName)
                    lineEditUserCreationEmail->setFocus();
            }
            return;
        }
    }

    strPassWord = lineEditPassword->text();
    strConfirmPassWord = lineEditPassword->text();
    if(lineEditConfirmPassword->isVisible())
        strConfirmPassWord = lineEditConfirmPassword->text();
    strGroup = comboBoxGroup->currentText();
    if(comboBoxProfile->currentText() != "none")
        strProfile = comboBoxProfile->currentText();

    // Check if have some Pwd
    if(strPassWord.isEmpty())
    {
        // Invalid passWord
        QMessageBox::warning(this,"User login", "Missing password !!");
        // Reset password
        lineEditPassword->setText("");
        lineEditConfirmPassword->setText("");
        return;
    }

    if(strPassWord != strConfirmPassWord)
    {
        // Invalid passWord
        QMessageBox::warning(this,"User login",
                             "Invalid confirm password !!");
        // Reset password
        lineEditPassword->setText("");
        lineEditConfirmPassword->setText("");
        return;
    }

    pUser = NULL;
    foreach(pUser , GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers)
    {
        if(pUser->m_strLogin == strLogins)
            break;
        pUser = NULL;
    }

    if(pUser == NULL)
    {
        // Invalid entry !!
        QMessageBox::warning(this,"Connection error", "Invalid entry !!");
        return;
    }

    if (lineEditUserCreationEmail->isVisible())
    {
        bool lEmptyEmail = true;
        QStringList emails = lineEditUserCreationEmail->text().split(";", QString::SkipEmptyParts);
        QRegExp     RegExp;
        foreach(QString mail, emails)
        {
            mail = mail.simplified();
            if(mail.isEmpty())
                continue;
            RegExp.setPattern("^[_a-z0-9-]+(\\.[_a-z0-9-]+)*@[a-z0-9-]+(\\.[a-z0-9-]+)*(\\.[a-z]{2,4})$");
            RegExp.setCaseSensitivity(Qt::CaseInsensitive);
            if (!RegExp.exactMatch(mail))
            {
                // Invalid email
                QMessageBox::warning(this,"User login", "Email address is invalid");
                return;
            }
            else
            {
                // At least on valid email address found
                lEmptyEmail = false;
            }
        }

        if (lEmptyEmail)
        {
            // Empty email
            QMessageBox::warning(this,"User login", "Email address is empty");
            return;
        }
    }

    // process could be long : lets disable the dialog for the user not to
    // repress any button

#ifdef __MACH__
    // Do not use setEnabled
    EnabledFieldItem(this->children(), false);
#else
    setEnabled(false);
#endif

    if(!GS::Gex::Engine::GetInstance().GetAdminEngine().ConnectUser(
                pUser->m_nUserId,strPassWord, GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()))
    {
        QString strError;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetLastError(strError);

        QMessageBox::warning(this,"Connection error", strError);
        // Reset password
        lineEditPassword->setText("");
        lineEditConfirmPassword->setText("");

#ifdef __MACH__
        // Do not use setEnabled
        EnabledFieldItem(this->children(), true);
#else
        setEnabled(true);
#endif
        lineEditPassword->setFocus();
        return;
    }
    accept();

    // Enabled or Disabled some access
    //if(pGexMainWindow->pWizardAdminGui)
    //    pGexMainWindow->pWizardAdminGui->EnableGexAccess();

    if(!labelUserCreationStatus->isHidden())
    {
        // Check if have some update
        bool bUpdateUser = false;
        if(!lineEditUserCreationEmail->isHidden()
                && !lineEditUserCreationEmail->text().isEmpty()
                && (pUser->m_strEmail != lineEditUserCreationEmail->text()))
        {
            bUpdateUser = true;
            pUser->m_strEmail = lineEditUserCreationEmail->text();
        }
        if(!lineEditUserCreationName->isHidden()
                && !lineEditUserCreationName->text().isEmpty()
                && (pUser->m_strName != lineEditUserCreationName->text()))
        {
            bUpdateUser = true;
            pUser->m_strName = lineEditUserCreationName->text();
        }
        if(bUpdateUser)
            GS::Gex::Engine::GetInstance().GetAdminEngine().SaveUser(pUser);
    }
#ifdef __MACH__
    // Do not use setEnabled
    EnabledFieldItem(this->children(), true);
#else
    setEnabled(true);
#endif


    // Then have to select the good Group
    if(!strGroup.isEmpty())
    {
        QMap<int, AdminUserGroup*>::iterator itGroup;
        for(itGroup = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.begin(); itGroup != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.end(); itGroup++)
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentGroup = itGroup.value();
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentGroup->m_strName == strGroup)
            {
                break;
            }
            GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentGroup = NULL;
        }
    }
    // Then have to load all Profiles and select the good
    if(!strProfile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_DEBUG, "GexYieldManDbLogin : onAccept Load all Profiles and select the good");
        /////////////////////
        // YIELDMAN PROFILES
        // Load all profiles
        QString strQuery;
        QSqlQuery clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));

        strQuery = "SELECT profile_id, user_id, name, description, os_login, permisions, script_name, script_content, creation_date, last_update FROM ym_users_profiles";
        strQuery+= " WHERE name='"+strProfile+"' AND user_id="+QString::number(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser->m_nUserId);
        strQuery+= " ORDER BY profile_id";
        // take the first
        if(clQuery.exec(strQuery))
        {
            int iIndex;
            AdminUserProfile *pProfile;
            if(clQuery.first())
            {
                iIndex = 0;

                // Check if already in the list
                if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapProfiles.contains(clQuery.value(0).toInt()))
                    pProfile = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapProfiles[clQuery.value(0).toInt()];
                else
                    pProfile = new AdminUserProfile;

                pProfile->m_nProfileId = clQuery.value(iIndex++).toInt();
                pProfile->m_nUserId = clQuery.value(iIndex++).toInt();
                pProfile->m_strName = clQuery.value(iIndex++).toString();
                pProfile->m_strDescription = clQuery.value(iIndex++).toString();
                pProfile->m_strOsLogin = clQuery.value(iIndex++).toString();
                pProfile->m_nPermissions = clQuery.value(iIndex++).toInt();
                pProfile->m_strScriptName = clQuery.value(iIndex++).toString();
                pProfile->m_strScriptContent = GS::Gex::Engine::GetInstance().GetAdminEngine().NormalizeSqlToScriptString(clQuery.value(iIndex++).toString());
                pProfile->m_clCreationDate = clQuery.value(iIndex++).toDateTime();
                pProfile->m_clUpdateDate = clQuery.value(iIndex++).toDateTime();

                GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapProfiles[pProfile->m_nProfileId] = pProfile;

            }
        }

        QMap<int, AdminUserProfile*>::iterator itProfile;
        for(itProfile = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapProfiles.begin(); itProfile != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapProfiles.end(); itProfile++)
        {
            GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentProfile = itProfile.value();
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentProfile->m_strName == strProfile)
                break;
            GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentProfile = NULL;
        }

        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentProfile)
        {
            QString lUserScript =   GS::Gex::Engine::GetInstance().Get("UserFolder").toString() +
                    "/" + GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentProfile->m_strScriptName;

            GS::Gex::Engine::GetInstance().SetStartupScript(lUserScript);
            // Load options: run startup script : $HOME/.<profile>.csl
            GS::Gex::CSLEngine::GetInstance().RunStartupScript(lUserScript);
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, "GexYieldManDbLogin : onAccept Check if have some options");
    // Check if have the disconnect option
    QString strValue;
    int    nValue = -1;

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin())
        nValue = YIELDMANDB_ADMIN_MIN_BEFORE_DISCONNECT;

    // Get global settings
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetSettingsValue("DB_MN_BEFORE_DISCONNECT",strValue))
        nValue = strValue.toInt();

    // Get settings for current Node
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetNodeSettingsValue("DB_MN_BEFORE_DISCONNECT",strValue))
        nValue = strValue.toInt();

    // Get settings for current User
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsValue("DB_MN_BEFORE_DISCONNECT",strValue))
        nValue = strValue.toInt();

    pGexMainWindow->pWizardAdminGui->m_bCheckAdminActivity = (nValue > 0);
    pGexMainWindow->pWizardAdminGui->m_nNbMinBeforeDisconnect = nValue;

    pGexMainWindow->pWizardAdminGui->OnCheckAdminActivity();

    QString strMessage = " ";
    strMessage += "Login["+strLogins+"]";
    if(!strGroup.isEmpty())
        strMessage += " Group["+strGroup+"]";
    if(!strProfile.isEmpty())
        strMessage += " Profile["+strProfile+"]";
    strMessage += " DB_MN_BEFORE_DISCONNECT["+QString::number(pGexMainWindow->pWizardAdminGui->m_nNbMinBeforeDisconnect)+"]";
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().data() );

    // Reload users list if updated
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        GS::Gex::Engine::GetInstance().GetAdminEngine().LoadUsersList();

    return;
}

///////////////////////////////////////////////////////////
// GexYieldManDbLogin:
///////////////////////////////////////////////////////////
void AdminUserLogin::onReject(void)
{
    if((tabWidget->currentWidget() == tabUserConnection)
            && (tabWidget->count() == 1))
    {
        if(m_bForceConnection)
        {
            if(!m_bAsAdmin)
            {
                reject();

                // Admin Server
                // Update the database with current files saved
                // Synchronize with YieldManDB profiles
                if(GS::Gex::Engine::GetInstance().IsAdminServerMode(true))
                    GS::Gex::Engine::GetInstance().GetAdminEngine().SynchronizeProfiles(GS::Gex::Engine::GetInstance().GetDefaultUserProfile());

                // Check if multi-user options available. If so, ask which one to load...unless running in hidden mode
                if(GS::Gex::Engine::GetInstance().IsExistingDefaultProfile())
                {
                    // Default custom profile, use it!!
                    QString lUserScript = GS::Gex::Engine::GetInstance().GetDefaultProfileScript();

                    GS::Gex::Engine::GetInstance().SetStartupScript(lUserScript);

                    // Load options: run startup script : $HOME/.<profile>.csl
                    GS::Gex::CSLEngine::GetInstance().RunStartupScript(lUserScript);
                }
                else
                {
                    QStringList lProfiles = GS::Gex::Engine::GetInstance().GetProfiles();

                    if (lProfiles.count() == 1)
                    {
                        // Single custom profile, use it!!
                        // Get username selected
                        QString lUserScript = lProfiles.first();

                        GS::Gex::Engine::GetInstance().SetStartupScript(lUserScript);

                        // Load options: run startup script : $HOME/.<profile>.csl
                        GS::Gex::CSLEngine::GetInstance().RunStartupScript(lUserScript);
                    }
                    else if((GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden() == false) &&
                            lProfiles.count() > 1)
                    {
                        PickUserDialog cPickUser(this);
                        // Set GUI in 'Load' mode
                        cPickUser.setLoadMode(true);

                        // Display dialog box.
                        if(cPickUser.exec() == 1)
                        {
                            // Get username selected
                            QString lUserName;
                            QString lUserScript = GS::Gex::Engine::GetInstance().GetStartupScript();
                            cPickUser.getSelectedUserName(lUserName, lUserScript);

                            GS::Gex::Engine::GetInstance().SetStartupScript(lUserScript);

                            // Load options: run startup script : $HOME/.<profile>.csl
                            GS::Gex::CSLEngine::GetInstance().RunStartupScript(lUserScript);
                        }
                    }
                }
            }
        }
    }

    reject();
}


///////////////////////////////////////////////////////////
// GexYieldManDbLogin:
///////////////////////////////////////////////////////////
void AdminUserLogin::onEditLogin()
{

    labelUserCreationStatus->hide();
    labelUserCreationName->hide();
    labelUserCreationMail->hide();
    lineEditUserCreationName->hide();
    lineEditUserCreationEmail->hide();

    if(comboBoxLogins->isHidden())
    {


        QString strLogin;
        strLogin = lineEditUserCreationLogin->text().toLower().simplified();

        if(strLogin.isEmpty())
            return;

        if(strLogin != lineEditUserCreationLogin->text())
            lineEditUserCreationLogin->setText(strLogin);

        // Check if the comboBoxLogins contains this string
        if(strLogin.isEmpty()
                || !SetCurrentComboItem(comboBoxLogins,strLogin))
        {
            // Invalid entry
            QPalette qpLineEditPalette = lineEditUserCreationLogin->palette();
            qpLineEditPalette.setColor(QPalette::Text, Qt::red);
            lineEditUserCreationLogin->setPalette(qpLineEditPalette);

            lineEditPassword->setText("");
            lineEditConfirmPassword->setText("");

            lineEditPassword->setEnabled(false);
            comboBoxProfile->setEnabled(false);
            return;
        }
        else
        {
            lineEditUserCreationLogin->setPalette(palette());
            lineEditPassword->setEnabled(true);
            comboBoxProfile->setEnabled(true);
        }
    }
}
///////////////////////////////////////////////////////////
// GexYieldManDbLogin:
///////////////////////////////////////////////////////////
void AdminUserLogin::onSelectLogin(void)
{
    labelUserCreationStatus->hide();
    labelUserCreationName->hide();
    labelUserCreationMail->hide();
    lineEditUserCreationName->hide();
    lineEditUserCreationEmail->hide();

    QString strLogin = comboBoxLogins->currentText();
    if(comboBoxLogins->isHidden())
    {
        strLogin = lineEditUserCreationLogin->text().toLower().simplified();
        // Check if the comboBoxLogins contains this string
        if(strLogin.isEmpty()
                || !SetCurrentComboItem(comboBoxLogins,strLogin))
            return;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "Update combo Profiles");

    // Update the Group list and the Profile list
    comboBoxGroup->clear();
    comboBoxProfile->clear();
    lineEditPassword->clear();
    lineEditConfirmPassword->clear();

    lineEditPassword->setWindowIcon(QPixmap(QString::fromUtf8(":/gex/icons/lock.png")));
    lineEditConfirmPassword->setWindowIcon(QPixmap(QString::fromUtf8(":/gex/icons/lock.png")));

    lineEditPassword->setFocus();


    QString strQuery;
    QSqlQuery clQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pDatabaseConnector->m_strConnectionName));
    strQuery = "SELECT user_id, profile_id , group_id, type, pwd FROM ym_users";
    strQuery+= " WHERE login='"+strLogin+"'";

    if(!clQuery.exec(strQuery))
        return;
    if(!clQuery.first())
        return;

    int    iUserId;
    int    iDefaultProfileId;
    int    iDefaultGroupId;
    QString strDefaultGroup;
    QString strPwd;

    iUserId = clQuery.value(0).toInt();
    iDefaultProfileId = clQuery.value(1).toInt();
    iDefaultGroupId = clQuery.value(2).toInt();
    strPwd = clQuery.value(4).toString();

    comboBoxGroup->hide();
    labelConnectionGroup->hide();

    comboBoxProfile->show();
    labelConnectionProfile->show();
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
        comboBoxProfile->hide();
        labelConnectionProfile->hide();
    }

    if(strPwd.isEmpty())
    {
        // Show confirmPassWord
        lineEditConfirmPassword->show();
        labelConfirmPassWord->show();
    }
    else
    {
        // Show confirmPassWord
        lineEditConfirmPassword->hide();
        labelConfirmPassWord->hide();
    }

    // Update the list of Group
    if(!comboBoxGroup->isHidden())
    {
        strQuery = "SELECT ym_groups.group_id, ym_groups.name, ym_groups.profile_id FROM ym_groups, ym_users";
        strQuery+= " WHERE ym_groups.group_id=ym_users.group_id";
        strQuery+= " AND ym_users.user_id="+QString::number(iUserId);

        if(!clQuery.exec(strQuery))
            return;

        while(clQuery.next())
        {
            SetCurrentComboItem(comboBoxGroup, clQuery.value(1).toString(), true, QString(":/gex/icons/yieldmandb_groups.png"));
            if(clQuery.value(0).toInt() == iDefaultGroupId)
            {
                strDefaultGroup = clQuery.value(1).toString();
                if(iDefaultProfileId == 0)
                    iDefaultProfileId = clQuery.value(2).toInt();
            }
        }
        strQuery = "SELECT ym_groups.group_id, ym_groups.name, ym_groups.profile_id FROM ym_groups, ym_groups_users";
        strQuery+= " WHERE ym_groups.group_id=ym_groups_users.group_id";
        strQuery+= " AND ym_groups_users.user_id="+QString::number(iUserId);

        if(!clQuery.exec(strQuery))
            return;

        while(clQuery.next())
        {
            SetCurrentComboItem(comboBoxGroup, clQuery.value(1).toString(), true, QString(":/gex/icons/yieldmandb_groups.png"));
            if(clQuery.value(0).toInt() == iDefaultGroupId)
            {
                strDefaultGroup = clQuery.value(1).toString();
                if(iDefaultProfileId == 0)
                    iDefaultProfileId = clQuery.value(2).toInt();
            }
        }
        if(!strDefaultGroup.isEmpty())
            SetCurrentComboItem(comboBoxGroup, strDefaultGroup, true, QString(":/gex/icons/yieldmandb_groups.png"));

        if(comboBoxGroup->count() == 0)
        {
            comboBoxGroup->hide();
            labelConnectionGroup->hide();
        }

    }

    // Update the list of profile
    if(!comboBoxProfile->isHidden())
    {
        strQuery = "SELECT DISTINCT name FROM ym_users_profiles";
        strQuery+= " WHERE user_id="+QString::number(iUserId);
        if(!clQuery.exec(strQuery))
            return;

        SetCurrentComboItem(comboBoxProfile, "none",
                            true, QString(":/gex/icons/yieldmandb_profiles.png"));

        while(clQuery.next())
        {
            SetCurrentComboItem(comboBoxProfile, clQuery.value(0).toString(),
                                true, QString(":/gex/icons/yieldmandb_profiles.png"));
        }
        // Select the good one
        if(iDefaultProfileId > 0)
        {
            strQuery = "SELECT name FROM ym_users_profiles";
            strQuery+= " WHERE profile_id="+QString::number(iDefaultProfileId);
            if(clQuery.exec(strQuery) && clQuery.first())
            {
                SetCurrentComboItem(comboBoxProfile, clQuery.value(0).toString(), true,
                                    QString(":/gex/icons/yieldmandb_profiles.png"));
            }
        }
        else
            SetCurrentComboItem(comboBoxProfile, "none",
                                true, QString(":/gex/icons/yieldmandb_profiles.png"));

        if(comboBoxProfile->count() == 1)
        {
            comboBoxProfile->clear();
            comboBoxProfile->hide();
            labelConnectionProfile->hide();
        }
    }

}

///////////////////////////////////////////////////////////
// GexYieldManDbLogin:
///////////////////////////////////////////////////////////
void AdminUserLogin::onSelectType(void)
{
    // Update the Group list

    QString strType = comboBoxUserCreationType->currentText();

    if(strType.isEmpty()
            || (strType == GS::Gex::Engine::GetInstance().GetAdminEngine().UserTypeToString(YIELDMANDB_USERTYPE_MASTER_ADMIN)))
    {
        comboBoxUserCreationGroup->hide();
        listWidgetUserCreationGroups->hide();
        labelUserCreationGroup->hide();
        labelUserCreationOtherGroup->hide();
    }
    else
    {
        comboBoxUserCreationGroup->hide();
        listWidgetUserCreationGroups->hide();
        labelUserCreationGroup->hide();
        labelUserCreationOtherGroup->hide();
    }
}

///////////////////////////////////////////////////////////
// Table mouse Right-click
///////////////////////////////////////////////////////////
void AdminUserLogin::onTableOptionContextualMenu(const QPoint& ptMousePoint)
{
    if(pGexMainWindow == NULL)
        return;

    QMenu               *pMenu = new QMenu(this);
    QTableWidgetItem    *ptItemOption = NULL;
    QTableWidgetItem    *ptItemValue = NULL;

    QMap<QAction*,int>	lstActions;
    // If at least one item in table
    if(tableOptions->rowCount() > 0)
    {
        ptItemOption = tableOptions->item(tableOptions->rowAt(ptMousePoint.y()),1);
        ptItemValue = tableOptions->item(tableOptions->rowAt(ptMousePoint.y()),2);
    }

    // If on an existing line
    // Option delete
    if(ptItemOption)
    {
        lstActions[pMenu->addAction("Delete option '"+ptItemOption->text()+"'" )] = -3;
 //       lstActions[pMenu->addAction("Edit option '"+ptItemOption->text()+"'" )] = -2;
    }

    pMenu->addSeparator();
    lstActions[pMenu->addAction("Add User Option ...")] = -1;

    QStringList listOptions;
    for(int nCurrentRow = 0; nCurrentRow < tableOptions->rowCount(); ++nCurrentRow)
        listOptions += tableOptions->item(nCurrentRow,1)->text();

    foreach(const QString &strOption, GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsName())
        if(!listOptions.contains(strOption))
            lstActions[pMenu->addAction("- "+strOption )] = GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsName().indexOf(strOption);
    lstActions[pMenu->addAction("- custom option")] = GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsName().count();

    if(pMenu->actions().count() == 0)
        return;

    // Display menu...
    pMenu->setMouseTracking(true);
    QAction *pActionResult = pMenu->exec(QCursor::pos());
    pMenu->setMouseTracking(false);

    if(pActionResult == NULL)
        return;

    if(lstActions[pActionResult] == -1)
        return;

    // Check menu selection activated
    if(lstActions[pActionResult] == -3)
    {
        //-- delete the current row;
        tableOptions->removeRow(tableOptions->currentRow());
    }
    else
    {
        // edit or new Option
        if(lstActions[pActionResult] != -2)
            ptItemOption = ptItemValue = NULL;

        bool bNewEntry = true;
        QString strName, strValue;
        int nCurrentRow = tableOptions->rowCount();
        if(ptItemOption)
        {
            bNewEntry = false;
            strName = ptItemOption->text();
            strValue = ptItemValue->text();
            nCurrentRow = tableOptions->currentRow();
        }
        else
        {
            tableOptions->setRowCount(nCurrentRow+1);
            if((lstActions[pActionResult] >= 0)
                    && (lstActions[pActionResult] < GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsName().count()))
                strName = GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsName()[lstActions[pActionResult]];
        }

        tableOptions->setRowHeight(nCurrentRow, 20);
        AdminGui* pAdmin = pGexMainWindow->pWizardAdminGui;

        QString lToolTip;
        if(!strName.isEmpty())
        {

            if(strValue.isEmpty())
                strValue = GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDefaultValue(strName);

            lToolTip = "Option = "+strName;
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(strName).isEmpty())
                lToolTip += "\nType = "+GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(strName);
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(strName).isEmpty())
                lToolTip += "\n"+GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(strName);
        }


        pAdmin->fillCellData(tableOptions,nCurrentRow,0,"000"); //QString::number(pUser->m_nUserId));
        pAdmin->fillCellData(tableOptions,nCurrentRow,1,strName,lToolTip,!bNewEntry,bNewEntry);
        pAdmin->fillCellData(tableOptions,nCurrentRow,2,strValue,lToolTip,false,true);
        pAdmin->fillCellData(tableOptions,nCurrentRow,3,GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(strName),lToolTip,true,false);

        // Set minimum size for the columns
        int nIndex = 0;
        tableOptions->setColumnWidth (nIndex++,50);
        tableOptions->setColumnWidth (nIndex++,150);   // Name
        tableOptions->setColumnWidth (nIndex++,200);   // Value
    }
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
// FOR CONNECTION
///////////////////////////////////////////////////////////
void AdminUserLogin::ShowPageUserConnection()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Show User connection dialog");

    // Hide tabUser and tabGroup
    while(tabWidget->count() > 1)
        tabWidget->removeTab(1);

    if(m_bAsAdmin)
        tabWidget->setTabText(0,"Administrator Login");
    else
        tabWidget->setTabText(0,"User Login");

    labelUserCreationStatus->hide();
    labelUserCreationName->hide();
    labelUserCreationMail->hide();
    lineEditUserCreationName->hide();
    lineEditUserCreationEmail->hide();
    labelUserCreationStatus->clear();
    lineEditUserCreationName->clear();
    lineEditUserCreationEmail->clear();


    // Update GUI
    comboBoxLogins->clear();
    lineEditUserCreationLogin->clear();
    comboBoxGroup->clear();
    comboBoxProfile->clear();
    lineEditPassword->clear();
    lineEditConfirmPassword->clear();

    lineEditPassword->setWindowIcon(QPixmap(QString::fromUtf8(":/gex/icons/lock.png")));
    lineEditConfirmPassword->setWindowIcon(QPixmap(QString::fromUtf8(":/gex/icons/lock.png")));

    // Reload users list if updated
    if (! GS::Gex::Engine::GetInstance().GetAdminEngine().LoadUsersList())
    {
        return;
    }

    QDateTime   lLastConnection = GS::Gex::Engine::GetInstance().GetClientDateTime().addYears(-10);
    QString     strDefaultUser;
    AdminUser*  pUser;
    QMap<int, AdminUser*>::iterator itUser;

    for(itUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.begin(); itUser != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.end(); itUser++)
    {
        pUser = itUser.value();

        if(!m_bAsAdmin // all users if not AsAdmin else only Admin users
                || (m_bAsAdmin && GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(pUser)))
        {
            SetCurrentComboItem(comboBoxLogins, pUser->m_strLogin, true,
                                QString(":/gex/icons/yieldmandb_users.png"));
            if((lLastConnection < pUser->m_clAccessDate)
                    && (GS::Gex::Engine::GetInstance().GetAdminEngine().GetOsLogin() == pUser->m_strOsLogin))
            {
                strDefaultUser = pUser->m_strLogin;
                lLastConnection = pUser->m_clAccessDate;
            }
        }
    }

    if(comboBoxLogins->count() == 0)
    {
        // Invalid passWord
        QString msg = "Cannot find user";
        if(m_bAsAdmin)
            msg+= " Admin";
        if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetDirAccessPlugin())
            msg+= ".\nDirectory Access Manager not loaded";
        msg+=".";
        QMessageBox::warning(this,"User login",
                             msg);
        return;
    }

    if(!strDefaultUser.isEmpty())
        SetCurrentComboItem(comboBoxLogins, strDefaultUser, true, QString(":/gex/icons/yieldmandb_users.png"));

    if(!m_bAsAdmin)
    {

        if(lineEditUserCreationLogin->isHidden())
        {
            QWidget::setTabOrder(lineEditUserCreationLogin, lineEditPassword);
            gridLayout_2->addWidget(lineEditUserCreationLogin , 0, 1, 1, 1);
            lineEditUserCreationLogin->show();
            connect(lineEditUserCreationLogin, SIGNAL(textChanged(QString)), this, SLOT(onEditLogin()));
            connect(lineEditUserCreationLogin, SIGNAL(editingFinished()), this, SLOT(onSelectLogin()));
        }
        comboBoxLogins->hide();
        lineEditUserCreationLogin->setText(strDefaultUser);
    }

    if(m_bForceConnection && !m_bAsAdmin)
    {
        QList<QAbstractButton*> lstButtons = buttonBoxOk->buttons();
        QAbstractButton* pButton;
        QString strText;
        for(int i=0; i!=lstButtons.count(); i++)
        {
            pButton = lstButtons.at(i);
            strText = pButton->text();
            if(strText.toLower() == "cancel")
                pButton->setText("Work OffLine");
        }
    }

    onSelectLogin();

    // Make Widget visible.
    show();
    QCoreApplication::processEvents();
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void AdminUserLogin::ShowPageUserOptions(QStringList lstUsersId)
{
    if(pGexMainWindow == NULL)
        return;

    m_lstUsersId = lstUsersId;
    m_lstGroupsId.clear();
    m_lstProfileId.clear();

    // Update GUI
    comboBoxUserCreationGroup->clear();
    comboBoxUserCreationType->clear();
    lineEditUserCreationEmail->clear();
    lineEditUserCreationLogin->clear();
    lineEditUserCreationName->clear();
    listWidgetUserCreationGroups->clear();

    // Delete all pages
    while(tabWidget->count() > 0)
        tabWidget->removeTab(0);

    if(lstUsersId.isEmpty())
        return;

    // User Login
    // Update GUI
    comboBoxLogins->clear();
    comboBoxLogins->setEnabled(false);
    comboBoxGroup->clear();
    comboBoxGroup->setEnabled(false);
    comboBoxProfile->clear();
    comboBoxGroup->hide();
    comboBoxProfile->hide();

    lineEditPassword->clear();
    lineEditConfirmPassword->clear();


    AdminGui* pAdmin = pGexMainWindow->pWizardAdminGui;

    // Check if all columns are created
    if(tableOptions->columnCount() < 4)
    {
        // Have to create all needed columns
        tableOptions->setColumnCount(4);
    }

    // Reset rows
    tableOptions->setRowCount(0);
    tableOptions->setColumnHidden(0,true);

    QStringList lstLabels;
    lstLabels << "UserId" << "Option name" << "Value" << "Description";
    tableOptions->setHorizontalHeaderLabels(lstLabels);
    tableOptions->verticalHeader()->setVisible(false);
    tableOptions->setSelectionBehavior(QAbstractItemView::SelectRows);

    if(lstUsersId.count() == 1)
    {

        AdminUser *pUser = NULL;
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(lstUsersId.first().toInt()))
            pUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[lstUsersId.first().toInt()];
        if(pUser == NULL)
            return;
        SetCurrentComboItem(comboBoxLogins, pUser->m_strLogin, true, QString(":/gex/icons/yieldmandb_users.png"));
        lineEditPassword->setWindowIcon(QPixmap(QString::fromUtf8(":/gex/icons/lock.png")));
        lineEditConfirmPassword->setWindowIcon(QPixmap(QString::fromUtf8(":/gex/icons/lock.png")));

        lineEditUserCreationLogin->setText(pUser->m_strLogin);
        lineEditUserCreationName->setText(pUser->m_strName);
        lineEditUserCreationEmail->setText(pUser->m_strEmail);
        lineEditPassword->setText(pUser->m_strPwd);
        lineEditConfirmPassword->setText(pUser->m_strPwd);

        tabWidget->insertTab(tabWidget->count(),tabUserOptions,"Options");
        labelOptions->setText("Options for '"+pUser->m_strLogin+"'");

        comboBoxUserCreationGroup->setEnabled(false);
        comboBoxUserCreationType->setEnabled(false);

        lineEditUserCreationLogin->setReadOnly(true);
        lineEditUserCreationName->setReadOnly(true);
        lineEditUserCreationEmail->setReadOnly(true);
        listWidgetUserCreationGroups->setEnabled(false);

        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(pUser))
        {
            // Impossible to change the User Type / Group / Other Groups
            lineEditUserCreationName->setReadOnly(true);
            comboBoxUserCreationType->hide();
            labelUserCreationType->hide();
        }
        else
        {
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
                    && GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges())
            {
                AdminUserGroup*  pGroup = NULL;
                QMap<int, AdminUserGroup*>::iterator itGroup;
                for(itGroup = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.begin(); itGroup != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.end(); itGroup++)
                {
                    pGroup = itGroup.value();
                    SetCurrentComboItem(comboBoxUserCreationGroup,pGroup->m_strName,true);
                    if(pGroup->m_nGroupId != pUser->m_nGroupId)
                    {
                        // Insert item into ListView
                        int nCurrentRow = listWidgetUserCreationGroups->count();
                        listWidgetUserCreationGroups->insertItem(nCurrentRow,pGroup->m_strName);

                        if(pGroup->m_lstUserIds.contains(pUser->m_nUserId))
                            listWidgetUserCreationGroups->setCurrentRow(nCurrentRow,QItemSelectionModel::SelectCurrent);
                    }
                }
            }

            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.contains(pUser->m_nGroupId))
                SetCurrentComboItem(comboBoxUserCreationGroup,GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups[pUser->m_nGroupId]->m_strName,true);

            SetCurrentComboItem(comboBoxUserCreationType,GS::Gex::Engine::GetInstance().GetAdminEngine().UserTypeToString(YIELDMANDB_USERTYPE_USER),true);
        }
        SetCurrentComboItem(comboBoxUserCreationType,GS::Gex::Engine::GetInstance().GetAdminEngine().UserTypeToString(pUser->m_nType));

        QString lInfo;
        lInfo = "Creation date:   \t"+pUser->m_clCreationDate.toString()+"\n";
        lInfo+= "Last update:     \t"+pUser->m_clUpdateDate.toString()+"\n";
        lInfo+= "Last connection: \t"+pUser->m_clAccessDate.toString()+"\n";
        labelUserCreationStatus->setText(lInfo);

        // Show user options
        QMap<QString, QVariant> mapAttributes = pUser->GetAttributes();
        foreach(const QString &key, mapAttributes.keys())
        {
            // Insert item into ListView
            int nCurrentRow = tableOptions->rowCount();
            tableOptions->setRowCount(1+nCurrentRow);
            tableOptions->setRowHeight(nCurrentRow, 20);

            QString lToolTip = "Option = "+key;
            QString lUserSettingType        = GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(key);
            QString lUserSettingDescription = GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key);
            if(!lUserSettingType.isEmpty())
                lToolTip += "\nType = "+ lUserSettingType;
            if(!lUserSettingDescription.isEmpty())
                lToolTip += "\n"+ lUserSettingDescription;

            // Add entry: title, task type, Frequency, Alarm condition, email addresses
            int nIndex = 0;

            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,QString::number(pUser->m_nUserId));
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,key,lToolTip);
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++, mapAttributes[key].toString(), lToolTip, false);
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++, lUserSettingDescription);
        }
    }
    else if(lstUsersId.count() != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.count())
    {
        tabWidget->insertTab(tabWidget->count(),tabUserOptions,"Options");
        labelOptions->setText("Options for "+QString::number(lstUsersId.count())+" users");
        // Retrieve shared options
        AdminUser *pUser = NULL;
        QMap<QString, QStringList> mapOptions;
        foreach(const QString &UserId, lstUsersId)
        {
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(UserId.toInt()))
                pUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[UserId.toInt()];
            else
                continue;
            QMap<QString, QVariant> mapAttributes = pUser->GetAttributes();
            foreach(const QString &key, mapAttributes.keys())
                mapOptions[key] += mapAttributes[key].toString();
        }
        foreach(const QString &key, mapOptions.keys())
        {
            if(mapOptions[key].count() < lstUsersId.count())
                continue;
            QStringList values;
            foreach(const QString &value, mapOptions[key])
                if(!values.contains(value)) values.append(value);
            if(values.count() != 1)
                continue;

            // Insert item into ListView
            int nCurrentRow = tableOptions->rowCount();
            tableOptions->setRowCount(1+nCurrentRow);
            tableOptions->setRowHeight(nCurrentRow, 20);

            QString lToolTip = "Option = "+key;
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(key).isEmpty())
                lToolTip += "\nType = "+GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(key);
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key).isEmpty())
                lToolTip += "\n"+GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key);

            // Add entry: title, task type, Frequency, Alarm condition, email addresses
            int nIndex = 0;
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,QString::number(pUser->m_nUserId));
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,key,lToolTip);
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,values.takeFirst(),lToolTip, false);
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key));
        }
    }
    else
    {
        tabWidget->insertTab(tabWidget->count(),tabUserOptions,"Options");
        labelOptions->setText("Options for all users");
        // Retrieve global options
        QMap<QString,QString> mapOptions;
        GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingsValues(mapOptions,true);
        foreach(const QString &key, mapOptions.keys())
        {
            // Insert item into ListView
            int nCurrentRow = tableOptions->rowCount();
            tableOptions->setRowCount(1+nCurrentRow);
            tableOptions->setRowHeight(nCurrentRow, 20);

            QString lToolTip = "Option = "+key;
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(key).isEmpty())
                lToolTip += "\nType = "+GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingType(key);
            if(!GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key).isEmpty())
                lToolTip += "\n"+GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key);

            // Add entry: title, task type, Frequency, Alarm condition, email addresses
            int nIndex = 0;
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,"000");
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,key,lToolTip);
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,mapOptions[key],lToolTip, false);
            pAdmin->fillCellData(tableOptions,nCurrentRow,nIndex++,GS::Gex::Engine::GetInstance().GetAdminEngine().GetUserSettingDescription(key));
        }
    }

    // Set minimum size for the columns
    int nIndex = 0;
    tableOptions->setColumnWidth (nIndex++,50);
    tableOptions->setColumnWidth (nIndex++,150);   // Name
    tableOptions->setColumnWidth (nIndex++,200);   // Value

    tableOptions->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(tableOptions,			SIGNAL(customContextMenuRequested ( const QPoint & )),	this,SLOT(onTableOptionContextualMenu(const QPoint&)));

    onSelectType();
    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// Show page: make it visible & update GUI fields
///////////////////////////////////////////////////////////
void AdminUserLogin::ShowPageGroupProperties(QStringList lstGroupsId)
{
    // Hide tabUser and tabGroup
    m_lstUsersId.clear();
    m_lstGroupsId = lstGroupsId;
    m_lstProfileId.clear();

    tabWidget->removeTab(1);
    tabWidget->removeTab(0);

    // Update GUI
    comboBoxGroupCreationAdmin->clear();
    comboBoxGroupCreationProfile->clear();
    lineEditGroupCreationDescription->clear();
    lineEditGroupCreationLots->clear();
    lineEditGroupCreationProducts->clear();
    lineEditGroupCreationName->clear();

    AdminUserGroup *pGroup = NULL;
    if(!lstGroupsId.isEmpty()
            && GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups.contains(lstGroupsId.first().toInt()))
        pGroup = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapGroups[lstGroupsId.first().toInt()];
    if(pGroup == NULL)
    {
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
                && !GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges())
            return;

        // For creation

        AdminUser*  pUser;
        QMap<int, AdminUser*>::iterator itUser;

        for(itUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.begin(); itUser != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.end(); itUser++)
        {
            pUser = itUser.value();
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(pUser))
                SetCurrentComboItem(comboBoxGroupCreationAdmin, pUser->m_strLogin, true);
        }

        onSelectType();
        // Make Widget visible.
        show();
        return;
    }

    tabWidget->setTabText(0,"Group Properties");

    // For Property
    lineEditGroupCreationName->setText(pGroup->m_strName);
    lineEditGroupCreationDescription->setText(pGroup->m_strDescription);

    bool bEnabled = true;

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser == NULL)
        bEnabled = false;

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().m_pCurrentUser
            && !GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges())
        bEnabled = false;

    comboBoxGroupCreationProfile->setEnabled(bEnabled);
    lineEditGroupCreationDescription->setEnabled(bEnabled);
    lineEditGroupCreationLots->setEnabled(bEnabled);
    lineEditGroupCreationProducts->setEnabled(bEnabled);
    lineEditGroupCreationName->setEnabled(bEnabled);
    comboBoxGroupCreationAdmin->setEnabled(bEnabled);


    if(GS::Gex::Engine::GetInstance().GetAdminEngine().HasUserGroupAdminPrivileges()
            || !GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.contains(pGroup->m_nUserId))
    {
        QString        strDefaultUser;
        AdminUser*  pUser;
        QMap<int, AdminUser*>::iterator itUser;

        for(itUser = GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.begin(); itUser != GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers.end(); itUser++)
        {
            pUser = itUser.value();
            if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsUserAdmin(pUser))
                SetCurrentComboItem(comboBoxGroupCreationAdmin, pUser->m_strLogin, true);
            if(strDefaultUser.isEmpty() && (pGroup->m_nUserId == pUser->m_nUserId))
                strDefaultUser = pUser->m_strLogin;
        }

        if(!strDefaultUser.isEmpty())
            SetCurrentComboItem(comboBoxGroupCreationAdmin, strDefaultUser, true);
    }
    else
    {
        SetCurrentComboItem(comboBoxGroupCreationAdmin, GS::Gex::Engine::GetInstance().GetAdminEngine().m_mapUsers[pGroup->m_nUserId]->m_strLogin, true);
    }

    lineEditGroupCreationProducts->setText(pGroup->m_lstAllowedProducts.join("|"));
    lineEditGroupCreationLots->setText(pGroup->m_lstAllowedLots.join("|"));

    onSelectType();
    // Make Widget visible.
    show();
}

///////////////////////////////////////////////////////////
// Display task in enabled or disabled mode
///////////////////////////////////////////////////////////
void AdminUserLogin::EnabledFieldItem(QObjectList lstObject, bool bEnabled)
{
    QObjectList lstChild = lstObject;
    QObject*  pChild;

    while(!lstChild.isEmpty())
    {
        pChild = lstChild.takeFirst();
        QString strChild = pChild->objectName();
        QString strType = pChild->metaObject()->className();
        if(strType == "CGexPB")
        {
            CGexPB *pChildItem = (CGexPB *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        if(strType == "QLineEdit")
        {
            QLineEdit *pChildItem = (QLineEdit *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QTextEdit")
        {
            QTextEdit *pChildItem = (QTextEdit *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QDateEdit")
        {
            QDateEdit *pChildItem = (QDateEdit *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QTimeEdit")
        {
            QTimeEdit *pChildItem = (QTimeEdit *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QCheckBox")
        {
            QCheckBox *pChildItem = (QCheckBox *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QSpinBox")
        {
            QSpinBox *pChildItem = (QSpinBox *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QDoubleSpinBox")
        {
            QDoubleSpinBox *pChildItem = (QDoubleSpinBox *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QRadioButton")
        {
            QRadioButton *pChildItem = (QRadioButton *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QGroupBox")
        {
            QGroupBox *pChildItem = (QGroupBox *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QComboBox")
        {
            QComboBox *pChildItem = (QComboBox *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QToolButton")
        {
            QToolButton *pChildItem = (QToolButton *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QTableWidget")
        {
            QTableWidget *pChildItem = (QTableWidget *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QWidget")
        {
            QWidget *pChildItem = (QWidget *)pChild;
            pChildItem->setEnabled(bEnabled);
        }
        else if(strType == "QPushButton")
        {
            QPushButton *pChildItem = (QPushButton *)pChild;
            if(!pChildItem->text().endsWith("ok",Qt::CaseInsensitive) && !pChildItem->text().endsWith("cancel",Qt::CaseInsensitive))
                pChildItem->setEnabled(bEnabled);
        }
        else
            lstChild += pChild->children();
    }

}

///////////////////////////////////////////////////////////
// Display task in enabled or disabled mode
///////////////////////////////////////////////////////////
void AdminUserLogin::EnabledFieldTable(QTableWidget *pTable, bool bEnabled)
{
    bool bReadOnly = !bEnabled;

    // DISABLE THIS OPTION
    bReadOnly = true;

    if(pTable == NULL)
        return;

    QTableWidgetItem *ptItem;
    int iNbRow    = pTable->rowCount();
    int iNbColumn  = pTable->columnCount();

    for(int iRow = 0; iRow < iNbRow; iRow++)
    {
        for(int iColumn = 0 ; iColumn < iNbColumn; iColumn++)
        {
            ptItem = pTable->item(iRow,iColumn);
            if(ptItem)
            {
                if(bReadOnly)
                    ptItem->setFlags(ptItem->flags() & ~Qt::ItemIsEditable);
                else
                    ptItem->setFlags(ptItem->flags() | Qt::ItemIsEditable);
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Set current item in combo box on item havin specified text
///////////////////////////////////////////////////////////
bool AdminUserLogin::SetCurrentComboItem(QComboBox *pCombo, QString strItem, bool bInsertIfMissing, QString strIcon)
{
    int nItem;

    QString strComboItem;
    for(nItem=0; nItem<pCombo->count(); nItem++)
    {
        strComboItem = pCombo->itemText(nItem);
        if(strComboItem == strItem)
        {
            pCombo->setCurrentIndex(nItem);
            pCombo->setEditText(strItem);
            return true;
        }
    }

    if(!bInsertIfMissing)
        return false;

    // Insert/select item
    if(strIcon.isEmpty())
        pCombo->insertItem(pCombo->count(), strItem);
    else
        pCombo->insertItem(pCombo->count(), QPixmap(QString::fromUtf8(strIcon.toLatin1().constData())),strItem);
    for(nItem=0; nItem<pCombo->count(); nItem++)
    {
        if(pCombo->itemText(nItem) == strItem)
        {
            pCombo->setCurrentIndex(nItem);
            pCombo->setEditText(strItem);
            return true;
        }
    }

    return false;
}


