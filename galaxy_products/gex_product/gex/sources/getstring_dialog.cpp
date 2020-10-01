#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "getstring_dialog.h"
#include "browser_dialog.h"

#include <stdio.h>
#include <QToolTip>
#include <QFileDialog>

#if defined unix || __MACH__
#include <unistd.h>
#include <stdlib.h>
#endif

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GetStringDialog::GetStringDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(PushButtonOk,		SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect(PushButtonBrowse,	SIGNAL(clicked()), this, SLOT(OnBrowse()));
}

///////////////////////////////////////////////////////////
// Init. Dialog box fields: Single string input
///////////////////////////////////////////////////////////
void	GetStringDialog::setDialogText(const char * Title, const char *szFieldName, const char *szFieldValue, const char *szToolTip,bool bAllowPickPath,bool boolBrowseFoldersOnly)
{
    setWindowTitle(Title);
	TextLabel->setText(szFieldName);
	LineEdit->setText(szFieldValue);
	LineEdit->setSelection(0,1023);
    LineEdit->setToolTip(tr(szToolTip));
	LineEdit->setFocus();
	if(bAllowPickPath == false)
		PushButtonBrowse->hide();
	bBrowseFoldersOnly = boolBrowseFoldersOnly;

	// Show single-string input Widget
	stackedWidget->setCurrentIndex(0);
}

///////////////////////////////////////////////////////////
// Init. Dialog box fields: Dual string input
///////////////////////////////////////////////////////////
void	GetStringDialog::setDialogText(QString strTitle,QString strWelcomeText,QString strString1,QString &strValue1,QString strToolTip1,QString strString2,QString &strValue2,QString strToolTip2) 
{
    setWindowTitle(strTitle);
	TextLabel_Welcome->setText(strWelcomeText);

	TextLabel_1->setText(strString1);
	LineEdit_1->setText(strValue1);
    LineEdit_1->setToolTip(strToolTip1);
	
	TextLabel_2->setText(strString2);
	LineEdit_2->setText(strValue2);
    LineEdit_2->setToolTip(strToolTip2);

	LineEdit_1->setFocus();

	// Show Dual-string inputs Widget
	stackedWidget->setCurrentIndex(1);
}

///////////////////////////////////////////////////////////
// Returns signle-string entered.
///////////////////////////////////////////////////////////
void	GetStringDialog::getString(char *szString) 
{
	// Retrieve the string (if any)
    strcpy(szString, LineEdit->text().toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Dual string input, return selected string
///////////////////////////////////////////////////////////
QString	GetStringDialog::getString(int iStringID) 
{
	if(iStringID == 0)
		return LineEdit_1->text();
	else
		return LineEdit_2->text();
}

///////////////////////////////////////////////////////////
// Returns string entered.
///////////////////////////////////////////////////////////
void	GetStringDialog::OnBrowse()
{
	QString s;

	// Browse disk...
	if(bBrowseFoldersOnly)
	{
		// ... to pick a path...
		s = QFileDialog::getExistingDirectory(
                    this,
                    "Choose a directory",
                    LineEdit->text(),
					QFileDialog::ShowDirsOnly );
	}
	else
	{
		// ...to pick a file
		s = QFileDialog::getOpenFileName(this, "Select application to use", LineEdit->text(), "Application (*.exe)");
	}

	if(s.isEmpty() == true)
		return;	// Nothing entered!

	// Save selected path into edit field...
	LineEdit->setText(s);
	LineEdit->setSelection(0,1023);
}


void GetStringDialog::addConditions(QWidget *poWidget){

    if(m_poLayoutConditions && poWidget){
        m_poLayoutConditions->addWidget(poWidget);
    }

}

void GetStringDialog::checkGroupName(const QString &){
    QPalette oPal;
    if(m_oProhibitedName.contains(LineEdit->text())){
        oPal.setColor(QPalette::Text, Qt::red);
        PushButtonOk->setDisabled(true);
    } else {
        oPal.setColor(QPalette::Text, Qt::black);
        PushButtonOk->setDisabled(false);
    }
    LineEdit->setPalette(oPal);
}

void GetStringDialog::setProhibatedList(const QStringList &oProhibitedName){
    m_oProhibitedName = oProhibitedName;
    connect(LineEdit, SIGNAL(textChanged(const QString & )), this, SLOT(checkGroupName(const QString & )));

}
