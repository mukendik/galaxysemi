#include "ui_recipe_profile_eng.h"
#include "recipe_profile_eng.h"



//===============================================================================================
// Dialog box to Show/Edit ENG recipe profile
//===============================================================================================
CRecipeProfile_ENG::CRecipeProfile_ENG(QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::CRecipeProfile_ENG)
{
    m_ui->setupUi(this);

    // Connect signals
    QObject::connect(m_ui->pushButtonOk,	SIGNAL(clicked()),	this, SLOT(accept()));
	QObject::connect(m_ui->pushButtonCancel,SIGNAL(clicked()),	this, SLOT(reject()));
}

//===============================================================================================
// Destructor
//===============================================================================================
CRecipeProfile_ENG::~CRecipeProfile_ENG()
{
    delete m_ui;
}

//===============================================================================================
//===============================================================================================
void CRecipeProfile_ENG::changeEvent(QEvent *e)
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
// Load into GUI a Recipe profile
//===============================================================================================
void CRecipeProfile_ENG::writeProfile(CENG_Recipe cEntry,bool bAllowEnable)
{
	m_ui->comboBoxGroup->clear();
	m_ui->comboBoxGroup->addItems(cEntry.strGroups);
	int iGroupIndex = m_ui->comboBoxGroup->findText(cEntry.strGroupName);
	if(iGroupIndex < 0)
		iGroupIndex = 0;	// Force first group in case no group name defined yet.
	m_ui->comboBoxGroup->setCurrentIndex(iGroupIndex);

	// Enable/Disable field locked?
	m_ui->comboBoxStatus->setEnabled(bAllowEnable);

	// Status
	m_ui->comboBoxStatus->setCurrentIndex(cEntry.iEnabled);

	// Comments
	m_ui->lineEdit->setText(cEntry.strComment);

	// Info
	QString strDetails="";
	strDetails += "<b>Recipe:</b>&nbsp;" + cEntry.strRecipeName + "&nbsp;&nbsp;V";
	strDetails += QString::number(cEntry.iEng_Version) + "." + QString::number(cEntry.iEng_Build) + "<br>";
	strDetails += "<b>File:</b>&nbsp;" + cEntry.strRecipeFile;
	m_ui->textEditDetails->setText(strDetails);
}

//===============================================================================================
// Get from GUI a Recipe profile
//===============================================================================================
void CRecipeProfile_ENG::readProfile(CENG_Recipe &cEntry)
{
	cEntry.strGroupName = m_ui->comboBoxGroup->currentText();
    cEntry.iEnabled =  m_ui->comboBoxStatus->currentIndex();
	cEntry.strComment = m_ui->lineEdit->text();
}


