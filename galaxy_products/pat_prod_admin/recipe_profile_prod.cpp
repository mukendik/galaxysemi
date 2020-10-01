#include "recipe_profile_prod.h"
#include "ui_recipe_profile_prod.h"


//===============================================================================================
// Dialog box to Show/Edit recipe profile
//===============================================================================================
CRecipeProfile_PROD::CRecipeProfile_PROD(QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::CRecipeProfile_PROD)
{
    m_ui->setupUi(this);

    // Connect signals
    QObject::connect(m_ui->pushButtonOk,	SIGNAL(clicked()),	this, SLOT(accept()));
	QObject::connect(m_ui->pushButtonCancel,SIGNAL(clicked()),	this, SLOT(reject()));
}

//===============================================================================================
// Destructor
//===============================================================================================
CRecipeProfile_PROD::~CRecipeProfile_PROD()
{
    delete m_ui;
}

//===============================================================================================
//===============================================================================================
void CRecipeProfile_PROD::changeEvent(QEvent *e)
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
// Load into GUI a PROD Recipe profile
//===============================================================================================
void CRecipeProfile_PROD::writeProfile(CPROD_Recipe cEntry,bool bAllowEnable)
{
	m_ui->comboBoxStatus->setCurrentIndex(cEntry.m_iEnabled);

	// Info
	QString strDetails="";
	strDetails += "<b>Group:</b>&nbsp;" + cEntry.m_strGroup + "<br>";
	strDetails += "<b>PROD Recipe:</b>&nbsp;" + cEntry.m_strRecipeName + "&nbsp;&nbsp;V";
	strDetails += QString::number(cEntry.m_iVersion) + "." + QString::number(cEntry.m_iBuild) + "<br>";
	strDetails += "<b>Flows:</b><br>&nbsp;&nbsp;" + cEntry.strFlowListDetails("<br>&nbsp;&nbsp;") + "<br>";
	strDetails += "<b>STDF Output:</b>&nbsp;" + cEntry.strStdfOutputMode() + "<br>";
	strDetails += "<b>PROD Recipe File:</b>&nbsp;" + cEntry.m_strRecipeFile + "<br>";
	strDetails += "<b>Comment:</b>&nbsp;" + cEntry.m_strComment + "<br>";
	m_ui->textEditDetails->setText(strDetails);

	// Enable/Disable field locked?
	m_ui->comboBoxStatus->setEnabled(bAllowEnable);
}

//===============================================================================================
// Get from GUI a Recipe profile
//===============================================================================================
void CRecipeProfile_PROD::readProfile(CPROD_Recipe &cEntry)
{
	cEntry.m_iEnabled =  m_ui->comboBoxStatus->currentIndex();
}
