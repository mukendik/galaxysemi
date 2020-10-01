#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include "pickcombo_dialog.h"
#include "browser_dialog.h"

PickComboDialog::PickComboDialog(QWidget * pParent, Qt::WindowFlags fl) : QDialog(pParent, fl)
{
	setupUi(this);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(PushButtonOk,		SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(PushButtonCancel,	SIGNAL(clicked()), this, SLOT(reject()));
}

// Set dialog box texts
void PickComboDialog::setText(QString strTitle,QString strText1,QString strText2,QString strCheckBox)
{
    setWindowTitle(strTitle);
	TextLabel1->setText(strText1);
	TextLabel2->setText(strText2);
	if(strText2.isEmpty())
	{
		TextLabel2->hide();
		comboBox2->hide();
	}

	// Disable Check box if no text assigned to it.
	if(strCheckBox.isEmpty())
		checkBox->hide();
	else
		checkBox->setText(strCheckBox);
}

// Insert string into combo-box1 or combo-box2
void PickComboDialog::InsertItem(int iComboID,QString strEntry,bool bReset/*=false*/)
{
    QComboBox* comboBoxPt = (iComboID == 1) ? comboBox1:comboBox2;

	// Empty combo if requested
	if(bReset)
		comboBoxPt->clear();

	// Add item to combobox.
    comboBoxPt->insertItem(comboBoxPt->count(), strEntry);
}

// Current combo-box(1 or 2) item selected
int PickComboDialog::currentItem(int iComboID)
{
    QComboBox* comboBoxPt = (iComboID == 1) ? comboBox1:comboBox2;

	return comboBoxPt->currentIndex();
}

// Set/Clear check-box
void PickComboDialog::CheckBox(bool bCheckBox)
{
	checkBox->setChecked(bCheckBox);
}

// Is check box selected
bool PickComboDialog::isChecked(void)
{
	return checkBox->isChecked();
}
