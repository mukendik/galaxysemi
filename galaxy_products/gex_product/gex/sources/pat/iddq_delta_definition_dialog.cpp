#include "iddq_delta_definition_dialog.h"
#include "pat_rules.h"

///////////////////////////////////////////////////////////
// NNR rule Dialog box: Constructor
///////////////////////////////////////////////////////////
IddqDeltaDefinitionDialog::IddqDeltaDefinitionDialog(QWidget* parent,
                                             bool /*modal*/,
                                             Qt::WindowFlags fl)
                                                 : QDialog(parent, fl)
{
    setupUi(this);

    QObject::connect(pushButtonOk,					SIGNAL(clicked()),		this, SLOT(accept()));
    QObject::connect(pushButtonCancel,				SIGNAL(clicked()),		this, SLOT(reject()));
}

///////////////////////////////////////////////////////////
// IDDQ-Delta rule Dialog box: fill GUI
///////////////////////////////////////////////////////////
void	IddqDeltaDefinitionDialog::fillGUI(CIDDQ_Delta_Rule &rule)
{
    lineEditRuleName->setText(rule.GetRuleName());	// Rule name

    // Bin and color
    spinBoxFailBin->setValue(rule.GetSoftBin());
    pushButtonFailBinColor->setActiveColor(rule.GetFailBinColor());

    lineEditIDDQ_Delta_PreStress->setText(rule.GetPreStress());
    lineEditIDDQ_Delta_PostStress->setText(rule.GetPostStress());
    checkBoxIDDQ_Delta_CaseSensitive->setChecked(rule.GetCaseSensitive());
    comboBoxIDDQ_Delta_Algorithm->setCurrentIndex(rule.GetAlgorithm());
    lineEditIDDQ_Delta_N->setText(QString::number(rule.GetNFactor()));

    // Rule Enabled/Disabled
    groupBoxIDDQ->setChecked(rule.IsEnabled());
}

///////////////////////////////////////////////////////////
// IDDQ-Delta rule Dialog box: read GUI into array
///////////////////////////////////////////////////////////
void	IddqDeltaDefinitionDialog::readGUI(CIDDQ_Delta_Rule &rule)
{
    // Rule name (ensure the ',' character is mapped so we do not confuse our CSV storing format)
    rule.SetRuleName(lineEditRuleName->text());
    rule.SetRuleName(rule.GetRuleName().replace(',',';'));

    // Bin and color
    rule.SetSoftBin(spinBoxFailBin->value());
    rule.SetHardBin(spinBoxFailBin->value());
    rule.SetFailBinColor(pushButtonFailBinColor->activeColor());

    rule.SetPreStress(lineEditIDDQ_Delta_PreStress->text());
    rule.SetPostStress(lineEditIDDQ_Delta_PostStress->text());
    rule.SetCaseSensitive(checkBoxIDDQ_Delta_CaseSensitive->isChecked());
    rule.SetAlgorithm(comboBoxIDDQ_Delta_Algorithm->currentIndex());
    rule.SetNFactor(lineEditIDDQ_Delta_N->text().toDouble());

    // Enabled/Disabled rule
    rule.SetIsEnabled(groupBoxIDDQ->isChecked());
}

