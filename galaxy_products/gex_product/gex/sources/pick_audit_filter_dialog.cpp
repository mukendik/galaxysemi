#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "pick_audit_filter_dialog.h"
#include "browser_dialog.h"

#include <stdio.h>
#include <qfiledialog.h>

#if defined unix || __MACH__
#include <unistd.h>
#include <stdlib.h>
#endif

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PickAuditFilterDialog::PickAuditFilterDialog(QWidget* parent,
                                             bool modal,
                                             Qt::WindowFlags f)
    : QDialog( parent, f )
{
    // Initialize ui
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(PushButtonOk,			SIGNAL(clicked()),		this, SLOT(accept()));
    QObject::connect(comboBoxFilterType,	SIGNAL(activated(int)), this, SLOT(OnFilterType()));

    lineEditProduct->setFocus();
}

PickAuditFilterDialog::~PickAuditFilterDialog()
{
}

///////////////////////////////////////////////////////////
// Fiter type changed...
///////////////////////////////////////////////////////////
void PickAuditFilterDialog::OnFilterType(void)
{
    switch(comboBoxFilterType->currentIndex())
    {
        case 0:	//	Ignore Test names...
            lineEditFilterString->setText("continuity*");
            break;
        case 1:	//	Ignore Test numbers...
            lineEditFilterString->setText("1,2-15,17");
            break;
    }
}


///////////////////////////////////////////////////////////
// Load GUI fields...
///////////////////////////////////////////////////////////
void PickAuditFilterDialog::setFields(QString strProduct,QString strFilterType,QString strFilterString)
{
    lineEditProduct->setText(strProduct);
    int i = comboBoxFilterType->findText(strFilterType);
    if (i != -1)
        comboBoxFilterType->setCurrentIndex(i);
    else if (comboBoxFilterType->isEditable())
        comboBoxFilterType->setEditText(strFilterType);
    else
        comboBoxFilterType->setItemText(comboBoxFilterType->currentIndex(), strFilterType);
//    comboBoxFilterType->setCurrentText(strFilterType);
    lineEditFilterString->setText(strFilterString);
}

///////////////////////////////////////////////////////////
// Return product string
///////////////////////////////////////////////////////////
QString PickAuditFilterDialog::getProduct(void)
{
    QString strString = lineEditProduct->text();
    if(strString.length() <= 0)
        return "*";
    else
        return strString;
}

///////////////////////////////////////////////////////////
// Return filter type (test name, test number)
///////////////////////////////////////////////////////////
QString PickAuditFilterDialog::getFilterType(void)
{
    return comboBoxFilterType->currentText();
}

///////////////////////////////////////////////////////////
// Return filter string
///////////////////////////////////////////////////////////
QString PickAuditFilterDialog::getFilterString(void)
{
    return lineEditFilterString->text();
}

