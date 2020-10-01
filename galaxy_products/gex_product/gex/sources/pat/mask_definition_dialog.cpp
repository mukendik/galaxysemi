
#include "mask_definition_dialog.h"
#include "pat_rules.h"

///////////////////////////////////////////////////////////
// Mask rule Dialog box: Constructor
///////////////////////////////////////////////////////////
MaskDefinitionDialog::MaskDefinitionDialog(bool bCreate,
                                   QWidget* parent,
                                   bool /*modal*/,
                                   Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);

    QObject::connect(pushButtonOk,					SIGNAL(clicked()),		this, SLOT(accept()));
    QObject::connect(pushButtonCancel,				SIGNAL(clicked()),		this, SLOT(reject()));
    QObject::connect(comboBoxWorkingArea,		SIGNAL(activated(int)),		this, SLOT(changeWorkingArea(int)));

    lineEditMaskName->setEnabled(bCreate);
}

///////////////////////////////////////////////////////////
// Mask array rule : Working area changed...
///////////////////////////////////////////////////////////
void MaskDefinitionDialog::changeWorkingArea(int /*iSelection*/)
{
    switch(comboBoxWorkingArea->currentIndex())
    {
        // Outer ring
        case 0:
            labelWaferImage->setPixmap(QPixmap(QString::fromUtf8(":/gex/icons/mask_outer_ring.png")));
            break;

        // Inner ring
        case 1:
            labelWaferImage->setPixmap(QPixmap(QString::fromUtf8(":/gex/icons/mask_inner_ring.png")));
            break;
    }
}

///////////////////////////////////////////////////////////
// Mask rule Dialog box: fill GUI
///////////////////////////////////////////////////////////
void	MaskDefinitionDialog::fillGUI(CMask_Rule &cRule)
{
    lineEditMaskName->setText(cRule.mRuleName);				// Rule name

    // Select Working area
    comboBoxWorkingArea->setCurrentIndex(cRule.mWorkingArea);	// Wafer source: STDF (Soft/Hard bin), Prober map

    // Select Ring radius area
    spinBoxRingWidth->setValue(cRule.mRadius);

    // Update GUI image
    changeWorkingArea(0);

    // Rule Enabled/Disabled
    groupBoxMask->setChecked(cRule.mIsEnabled);
}

///////////////////////////////////////////////////////////
// Mask rule Dialog box: read GUI into array
///////////////////////////////////////////////////////////
void	MaskDefinitionDialog::readGUI(CMask_Rule &cRule)
{
    // Rule name (ensure the ',' character is mapped so we do not confuse our CSV storing format)
    cRule.mRuleName = lineEditMaskName->text();
    cRule.mRuleName = cRule.mRuleName.replace(',',';');

    cRule.mWorkingArea = comboBoxWorkingArea->currentIndex();	// Working area
    cRule.mRadius = spinBoxRingWidth->value();	// Radius

    // Enabled/Disabled rule
    cRule.mIsEnabled = groupBoxMask->isChecked();
}
