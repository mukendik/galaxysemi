#include <QMessageBox>

#include "nnr_definition_dialog.h"
#include "engine.h"
#include "pat_rules.h"
#include "pat_options.h"

///////////////////////////////////////////////////////////
// NNR rule Dialog box: Constructor
///////////////////////////////////////////////////////////
NnrDefinitionDialog::NnrDefinitionDialog(QWidget* parent,
                                 bool /*modal*/,
                                 Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);

    connect(pushButtonOk,       SIGNAL(clicked()),
            this,               SLOT(onOK(void)));
    connect(pushButtonCancel,   SIGNAL(clicked()),
            this,               SLOT(reject()));
    connect(lineEditRuleName,   SIGNAL(editingFinished()),
            this,               SLOT(OnRuleNameChanged()));
    connect(lineEditRuleName,   SIGNAL(textChanged(const QString &)),
            this,               SLOT(OnRuleNameChanged()));

    // Initialize combo for NNR cluster size
    comboBoxNNR_Size->clear();

    comboBoxNNR_Size->addItem("5x5 dies", 5);
    comboBoxNNR_Size->addItem("7x7 dies", 7);
    comboBoxNNR_Size->addItem("9x9 dies", 9);
    comboBoxNNR_Size->addItem("11x11 dies", 11);
    comboBoxNNR_Size->addItem("13x13 dies", 13);
    comboBoxNNR_Size->addItem("15x15 dies", 15);
//    comboBoxNNR_Size->addItem("Whole wafer", -1);

    comboBoxNNR_Size->setCurrentIndex(0);

    pushButtonOk->setEnabled(false);
}

void NnrDefinitionDialog::InitGui(const COptionsPat &patOptions)
{
    // Keep list of rule name already in use
    mRules.clear();
    for (int lIdx = 0; lIdx < patOptions.GetNNRRules().count(); ++lIdx)
    {
        mRules.append(patOptions.GetNNRRules().at(lIdx).GetRuleName());
    }
}

///////////////////////////////////////////////////////////
// NNR rule Dialog box: OK button clicked
///////////////////////////////////////////////////////////
void	NnrDefinitionDialog::onOK(void)
{
    // Check if L.A.  not to high (in which case NNR benefits drops!)
    if(doubleSpinBoxNNR_LA->value() <= 70.0)
    {
        accept();	// Value low enough, no warning to display
        return;
    }

    // High L.A. value: confirm to keep it?
    if(QMessageBox::question( this,
       GS::Gex::Engine::GetInstance().Get("AppFullName").toString(),
       "Warning: Your specified a high L.A value (NNR prone to noise)\nWe recommand a value < 70% or even lower (E.g.: 40%)\nDo you confirm your selection?", QMessageBox::Yes,QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
        accept();	// User wants to keep a high L.A. value
        return;
    }
}

///////////////////////////////////////////////////////////
// NNR rule Dialog box: fill GUI
///////////////////////////////////////////////////////////
void	NnrDefinitionDialog::LoadGUI(CNNR_Rule &rule)
{
    // Remove current rule name from existing one
    mRules.removeAll(rule.GetRuleName());

    lineEditRuleName->setText(rule.GetRuleName());	// Rule name

    // Bin and color
    spinBoxFailBin->setValue(rule.GetSoftBin());
    pushButtonFailBinColor->setActiveColor(rule.GetFailBinColor());

    comboBoxNRR_Algorithm->setCurrentIndex(rule.GetAlgorithm());
    lineEditNNR_N->setText(QString::number(rule.GetNFactor()));

    // Cluster Siez (candidate template)
    int nIndex  = comboBoxNNR_Size->findData(rule.GetClusterSize());
    comboBoxNNR_Size->setCurrentIndex(nIndex);	// NxN matrix.

    // Location Averaging (Top X% dies to average) to have NNR enabled.
    doubleSpinBoxNNR_LA->setValue(rule.GetLA());

    // Rule Enabled/Disabled
    groupBoxNNR->setChecked(rule.IsEnabled());

    // Enable/disbale Ok button
    pushButtonOk->setEnabled(!lineEditRuleName->text().isEmpty());
}

///////////////////////////////////////////////////////////
// NNR rule Dialog box: read GUI into array
///////////////////////////////////////////////////////////
void	NnrDefinitionDialog::ReadGUI(CNNR_Rule &rule)
{
    // Rule name (ensure the ',' character is mapped so we do not confuse our CSV storing format)
    rule.SetRuleName(lineEditRuleName->text());
    rule.SetRuleName(rule.GetRuleName().replace(',',';'));

    // Bin and color
    rule.SetSoftBin(spinBoxFailBin->value());
    rule.SetHardBin(spinBoxFailBin->value());
    rule.SetFailBinColor(pushButtonFailBinColor->activeColor());

    // Algorithm
    rule.SetAlgorithm(comboBoxNRR_Algorithm->currentIndex());

    // N factor
    rule.SetNFactor(lineEditNNR_N->text().toDouble());

    // Cluster size: 5, 7 ,9, 11, 13, 15 or whole wafer
    rule.SetClusterSize(comboBoxNNR_Size->itemData(comboBoxNNR_Size->currentIndex()).toInt());

    // Location averaging in cluster (top X best dies to average)
    rule.SetLA(doubleSpinBoxNNR_LA->value());

    // Enabled/Disabled rule
    rule.SetIsEnabled(groupBoxNNR->isChecked());
}

void NnrDefinitionDialog::OnRuleNameChanged()
{
    if (mRules.contains(lineEditRuleName->text(), Qt::CaseInsensitive))
    {
        QPalette lPalette = lineEditRuleName->palette();
        lPalette.setColor(QPalette::Text, Qt::red);
        lPalette.setColor(QPalette::ToolTipText, Qt::red);
        lineEditRuleName->setToolTip("Please change rule name, this one is already in use");
        lineEditRuleName->setPalette(lPalette);
        pushButtonOk->setEnabled(false);
    }
    else
    {
        lineEditRuleName->setToolTip("Please enter a rule name");
        lineEditRuleName->setPalette(QPalette());
        pushButtonOk->setEnabled(!lineEditRuleName->text().isEmpty());
    }
}



