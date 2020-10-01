#include "user_profile.h"
#include "ui_user_profile.h"


//===============================================================================================
// Dialog box to Show/Edit recipe profile
//===============================================================================================
CUserProfile::CUserProfile(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CUserProfile)
{
    m_ui->setupUi(this);

    // Connect signals
    QObject::connect(m_ui->pushButtonOk,	SIGNAL(clicked()),	this, SLOT(accept()));
    QObject::connect(m_ui->pushButtonCancel,	SIGNAL(clicked()),	this, SLOT(reject()));

    // Focus on recipe name
    m_ui->lineEditUserName->setFocus();
}

//===============================================================================================
// Destructor
//===============================================================================================
CUserProfile::~CUserProfile()
{
    delete m_ui;
}

//===============================================================================================
//===============================================================================================
void CUserProfile::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//===============================================================================================
// Load into GUI a User profile
//===============================================================================================
void CUserProfile::writeUser(CUserInfo cEntry)
{
    m_ui->lineEditUserName->setText(cEntry.strUserName);
    m_ui->lineEditPassword->setText(cEntry.strPassword);
    // Password not editable
    m_ui->lineEditPassword->setEnabled(false);
    m_ui->label_Password->setEnabled(false);
    m_ui->comboBoxType->setCurrentIndex(cEntry.iUserType);
}


//===============================================================================================
// Get from GUI a User profile
//===============================================================================================
void CUserProfile::readUser(CUserInfo &cEntry,bool bEncrypt/*=false*/)
{
    cEntry.strUserName = m_ui->lineEditUserName->text();
    cEntry.strPassword = m_ui->lineEditPassword->text();
    // Encrypt password ?
    if(bEncrypt)
	cEntry.strPassword = cEntry.crypt(cEntry.strPassword);
    cEntry.iUserType =  m_ui->comboBoxType->currentIndex();
}


