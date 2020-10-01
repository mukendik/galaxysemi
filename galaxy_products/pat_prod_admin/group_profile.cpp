#include "group_profile.h"
#include "ui_group_profile.h"


//===============================================================================================
// Dialog box to Show/Edit group profile
//===============================================================================================
CGroupProfile::CGroupProfile(QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::CGroupProfile)
{
    m_ui->setupUi(this);

    // Connect signals
    QObject::connect(m_ui->pushButtonOk,	SIGNAL(clicked()),	this, SLOT(accept()));
    QObject::connect(m_ui->pushButtonCancel,	SIGNAL(clicked()),	this, SLOT(reject()));

	// Focus on group name
	m_ui->lineEditGroupName->setFocus();
}

//===============================================================================================
// Destructor
//===============================================================================================
CGroupProfile::~CGroupProfile()
{
    delete m_ui;
}

//===============================================================================================
//===============================================================================================
void CGroupProfile::changeEvent(QEvent *e)
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
// Load into GUI a Group profile
//===============================================================================================
void CGroupProfile::writeGroup(CGroupInfo cEntry)
{
	m_ui->lineEditGroupName->setText(cEntry.strGroupName);
}


//===============================================================================================
// Get from GUI a Group profile
//===============================================================================================
void CGroupProfile::readGroup(CGroupInfo &cEntry)
{
	cEntry.strGroupName = m_ui->lineEditGroupName->text();
}


