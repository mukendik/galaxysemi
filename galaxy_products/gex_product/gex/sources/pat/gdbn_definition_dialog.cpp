
#include "gdbn_definition_dialog.h"
#include "pat_rules.h"
#include "wafermap.h"
#include "gqtl_utils.h"
#include "pat_options.h"

///////////////////////////////////////////////////////////
// GDBN rule Dialog box: Constructor
///////////////////////////////////////////////////////////
GdbnDefinitionDialog::GdbnDefinitionDialog(QWidget* parent,
                                   bool /*modal*/,
                                   Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setupUi(this);

    // Fill edge die type combo box
    comboBoxEdgeDieType->insertItem(0, "All",		QVariant(CWaferMap::EdgeDieBoth));
    comboBoxEdgeDieType->insertItem(1, "Adjacent",	QVariant(CWaferMap::EdgeDieAdjacent));
    comboBoxEdgeDieType->insertItem(2, "Corner",	QVariant(CWaferMap::EdgeDieCorner));

    QObject::connect(pushButtonOk,					SIGNAL(clicked()),		this, SLOT(accept()));
    QObject::connect(pushButtonCancel,				SIGNAL(clicked()),		this, SLOT(reject()));
    QObject::connect(comboBoxNeighbourhoodSize,		SIGNAL(activated(int)),	this, SLOT(OnBadClusterSize(void)));
    QObject::connect(comboBoxNeighbourhoodAlgorithm,SIGNAL(activated(int)),	this, SLOT(OnNeighbourhoodAlgorithm(void)));
    QObject::connect(comboBoxWeightingEdgeDies,		SIGNAL(activated(int)),	this, SLOT(OnWeightingEdgeDie(void)));
}

///////////////////////////////////////////////////////////
// GDBN rule Dialog box: fill GUI
///////////////////////////////////////////////////////////
void	GdbnDefinitionDialog::fillGUI(CGDBN_Rule &rule, const  COptionsPat *patOptions)
{
    lineEditRuleName->setText(rule.mRuleName);						// Rule name
    lineEditBadBinNeighbors->setText(rule.mBadBinList->GetRangeList());
    comboBox_GDBN_BinSource->setCurrentIndex(rule.mWafermapSource);	// Map source: STDF Soft/Hard bin, Prober

    // Fill Mask list
    comboBox_GdbnMask->clear();
    int iIndex;
    comboBox_GdbnMask->addItem("-None-");

    for(iIndex=0; iIndex < patOptions->mMaskRules.count(); iIndex++)
        comboBox_GdbnMask->addItem(patOptions->mMaskRules.at(iIndex)->mRuleName);
    comboBox_GdbnMask->setCurrentIndex(comboBox_GdbnMask->findText(rule.mMaskName));

    comboBoxNeighbourhoodAlgorithm->setCurrentIndex(rule.mAlgorithm);	// GDBN algorithm(Squeeze , Weighting)
    doubleSpinBoxNeighbourhoodYieldLevel->setValue(rule.mYieldThreshold);
    switch(rule.mClusterSize)
    {
        case 3:
        default:
            comboBoxNeighbourhoodSize->setCurrentIndex(0);	// 3x3 matrix.
            break;
        case 5:
            comboBoxNeighbourhoodSize->setCurrentIndex(1);	// 5x5 matrix.
            break;
        case 7:
            comboBoxNeighbourhoodSize->setCurrentIndex(2);	// 7x7 matrix.
            break;
    }

    // Bin and color
    spinBoxFailBin->setValue(rule.mSoftBin);
    pushButtonFailBinColor->setActiveColor(rule.mFailBinColor);

    // Squeeze rule: Fail edges if bad neighbors?
    checkBoxNeighbourhoodFailEdges->setChecked(rule.mFailWaferEdges);

    // Weighting rule: Edge dies handling
    int nIndexEdgeDieType = comboBoxEdgeDieType->findData(QVariant(rule.mEdgeDieType));
    comboBoxEdgeDieType->setCurrentIndex(nIndexEdgeDieType);

    comboBoxWeightingEdgeDies->setCurrentIndex(rule.mEdgeDieWeighting);	// Weighting rule: Edge die handling
    doubleSpinBoxWeightingScale->setValue(rule.mEdgeDieWeightingScale);

    // GDBN: Weighting algorithm
    spinBoxGDBN_WeightAdjacent->setValue(rule.mAdjWeightLst.at(0));
    spinBoxGDBN_WeightDiagonal->setValue(rule.mDiagWeightLst.at(0));
    spinBoxGDBN_MinimumWeighting->setValue(rule.mMinimumWeighting);


    OnNeighbourhoodAlgorithm();	// GDBN algorithm selection
    OnWeightingEdgeDie();		// GDBN weighting edge die mode
    OnBadClusterSize();			// Cluster size
    comboBoxNeighbourhoodFail->setCurrentIndex(rule.mFailCount-1);

    // Rule Enabled/Disabled
    groupBoxGDBN->setChecked(rule.mIsEnabled);
}

///////////////////////////////////////////////////////////
// GDBN rule Dialog box: read GUI into array
///////////////////////////////////////////////////////////
void	GdbnDefinitionDialog::readGUI(CGDBN_Rule &rule)
{
    // Rule name (ensure the ',' character is mapped so we do not confuse our CSV storing format)
    rule.mRuleName = lineEditRuleName->text();
    rule.mRuleName = rule.mRuleName.replace(',',';');

    // Bin and color
    rule.mSoftBin = spinBoxFailBin->value();
    rule.mHardBin = spinBoxFailBin->value();
    rule.mFailBinColor = pushButtonFailBinColor->activeColor();

    // Get mask rule
    rule.mMaskName = comboBox_GdbnMask->currentText();
    rule.mMaskName = rule.mMaskName.replace(',',';');

    if(rule.mBadBinList != NULL)
        delete rule.mBadBinList;
    rule.mBadBinList = new GS::QtLib::Range(
       lineEditBadBinNeighbors->text().toLatin1().constData());
    rule.mWafermapSource = comboBox_GDBN_BinSource->currentIndex();	// Map source: STDF Soft/Hard bin, Prober
    rule.mAlgorithm = comboBoxNeighbourhoodAlgorithm->currentIndex();	// GDBN Algorithm: Squeeze, Weighting rule...
    rule.mFailWaferEdges = checkBoxNeighbourhoodFailEdges->isChecked();
    rule.mYieldThreshold = doubleSpinBoxNeighbourhoodYieldLevel->value();

    // Squeeze algorithm
    rule.mClusterSize = 3+ 2*comboBoxNeighbourhoodSize->currentIndex();	// Matrix size: 3,5 or 7
    rule.mFailCount = 1 + comboBoxNeighbourhoodFail->currentIndex();		// Fail count in matrix

    // Edge dies
    rule.mEdgeDieType				= comboBoxEdgeDieType->itemData(comboBoxEdgeDieType->currentIndex()).toInt();	// Edge die type
    rule.mEdgeDieWeighting		= comboBoxWeightingEdgeDies->currentIndex();									// Weighting rule: Edge die handling
    rule.mEdgeDieWeightingScale	= doubleSpinBoxWeightingScale->value();											// Scaling factor over computed weight

    // Weighting algorithm
    rule.mAdjWeightLst.clear();
    rule.mDiagWeightLst.clear();
    rule.mAdjWeightLst.append(spinBoxGDBN_WeightAdjacent->value());
    rule.mDiagWeightLst.append(spinBoxGDBN_WeightDiagonal->value());
    rule.mMinimumWeighting = spinBoxGDBN_MinimumWeighting->value();

    // Enabled/Disabled rule
    rule.mIsEnabled = groupBoxGDBN->isChecked();
}

///////////////////////////////////////////////////////////
// Select GDBN Algorithm
// Available modes:
// - Weighting: define weight for each of the 8 surrounding fail dies
// - Squeeze: detect when good die is squeezed between fail dies.
///////////////////////////////////////////////////////////
void	GdbnDefinitionDialog::OnNeighbourhoodAlgorithm(void)
{
    int iAlgorithm = comboBoxNeighbourhoodAlgorithm->currentIndex();
    switch(iAlgorithm)
    {
        case GEX_TPAT_GDBN_ALGO_SQUEEZE:	// 'Squeeze rule'
            stackedWidget_GDBN_Algorithm->setCurrentIndex(0);
            break;

        case GEX_TPAT_GDBN_ALGO_WEIGHT:		// 'Weighting rule'
            stackedWidget_GDBN_Algorithm->setCurrentIndex(1);
            break;
    }
}


///////////////////////////////////////////////////////////
// Select GDBN Weighting edge die handling
///////////////////////////////////////////////////////////
void	GdbnDefinitionDialog::OnWeightingEdgeDie(void)
{
    switch(comboBoxWeightingEdgeDies->currentIndex())
    {
        case GEX_TPAT_GPAT_EDGE_IGNORE:
        case GEX_TPAT_GPAT_EDGE_GOOD:
        case GEX_TPAT_GPAT_EDGE_BAD:
            doubleSpinBoxWeightingScale->hide();
            break;

        case GEX_TPAT_GPAT_EDGE_SCALE:
            doubleSpinBoxWeightingScale->show();
            break;
    }
}

///////////////////////////////////////////////////////////
// User selects a specific cluster size, update the list of valid Bad count...
///////////////////////////////////////////////////////////
void	GdbnDefinitionDialog::OnBadClusterSize(void)
{
    int	iClusterSize = 3+ 2*comboBoxNeighbourhoodSize->currentIndex();
    iClusterSize *= iClusterSize;
    comboBoxNeighbourhoodFail->clear();
    comboBoxNeighbourhoodFail->addItem("1 Bad Die in the Neighborhood");

    QString strString;
    for(int iIndex = 2; iIndex < iClusterSize; iIndex++)
    {
        strString = QString::number(iIndex) + " Bad Dies in the Neighborhood";
        comboBoxNeighbourhoodFail->addItem(strString);
    }
}
