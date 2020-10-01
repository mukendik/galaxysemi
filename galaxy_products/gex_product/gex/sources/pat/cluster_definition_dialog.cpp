
#include "cluster_definition_dialog.h"
#include "pat_rules.h"
#include "wafermap.h"
#include "gqtl_utils.h"
#include "pat_options.h"
#include "pat_defines.h"
#include "math.h"


///////////////////////////////////////////////////////////
// Clustering rule Dialog box: Constructor
///////////////////////////////////////////////////////////
ClusterDefinitionDialog::ClusterDefinitionDialog(QWidget* parent,
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
    QObject::connect(comboBoxOutlineMatrixSize,		SIGNAL(activated(int)),		this, SLOT(changeOutlineMatrixSize(int)));
    QObject::connect(comboBoxWeightingEdgeDies,		SIGNAL(activated(int)),	this, SLOT(OnWeightingEdgeDie(void)));
}

///////////////////////////////////////////////////////////
// Clustering array rule : Outline matrix size changed...
///////////////////////////////////////////////////////////
void ClusterDefinitionDialog::changeOutlineMatrixSize(int /*iSelection*/)
{
    switch(comboBoxOutlineMatrixSize->currentIndex())
    {
        // 3x3 matrix
        case 0:
            textLabel_Ring2->hide();
            spinBoxBadClusterWeightAdjacent_R2->hide();
            spinBoxBadClusterWeightDiagonal_R2->hide();
            textLabel_Ring3->hide();
            spinBoxBadClusterWeightAdjacent_R3->hide();
            spinBoxBadClusterWeightDiagonal_R3->hide();

            break;

        // 5x5 matrix
        case 1:
            textLabel_Ring2->show();
            spinBoxBadClusterWeightAdjacent_R2->show();
            spinBoxBadClusterWeightDiagonal_R2->show();
            textLabel_Ring3->hide();
            spinBoxBadClusterWeightAdjacent_R3->hide();
            spinBoxBadClusterWeightDiagonal_R3->hide();
            break;

        // 7x7 matrix
        case 2:
            textLabel_Ring2->show();
            spinBoxBadClusterWeightAdjacent_R2->show();
            spinBoxBadClusterWeightDiagonal_R2->show();
            textLabel_Ring3->show();
            spinBoxBadClusterWeightAdjacent_R3->show();
            spinBoxBadClusterWeightDiagonal_R3->show();
            break;
    }
}



///////////////////////////////////////////////////////////
// Select Clustering Weighting edge die handling
///////////////////////////////////////////////////////////
void	ClusterDefinitionDialog::OnWeightingEdgeDie(void)
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
// Clustering rule Dialog box: fill GUI
///////////////////////////////////////////////////////////
void	ClusterDefinitionDialog::fillGUI(CClusterPotatoRule &cRule, const COptionsPat *patOptions)
{
    lineEditRuleName->setText(cRule.mRuleName);						// Rule name
    lineEditBadBinIdentifyList->setText(cRule.mBadBinIdentifyList->GetRangeList());		// Bad bin list identifying a bad cluster
    lineEditBadBinInkingList->setText(cRule.mBadBinInkingList->GetRangeList());		// Bad bin list identifying a bad cluster
    comboBox_ClusterBinSource->setCurrentIndex(cRule.mWaferSource); // Map source: STDF Soft/Hard bin, Prober

    // Bin and color
    spinBoxFailBin->setValue(cRule.mSoftBin);
    pushButtonFailBinColor->setActiveColor(cRule.mFailBinColor);

    // Fill Mask list
    comboBox_ClusterMask->clear();
    int iIndex;
    comboBox_ClusterMask->addItem("-None-");
    for(iIndex=0; iIndex < patOptions->mMaskRules.count(); iIndex++)
        comboBox_ClusterMask->addItem(patOptions->mMaskRules.at(iIndex)->mRuleName);
    comboBox_ClusterMask->setCurrentIndex(comboBox_ClusterMask->findText(cRule.mMaskName));

    doubleSpinBoxClusterSize->setValue(fabs(cRule.mClusterSize));	// Cluster size
    if(cRule.mClusterSize >= 0)
        comboBoxClusterSizeType->setCurrentIndex(0);			// Size in 'Dies'
    else
        comboBoxClusterSizeType->setCurrentIndex(1);			// Size in '% of gross die'

    spinBoxClusteringOutlineWidth->setValue(cRule.mOutlineWidth);

    // Light outline rule
    groupBoxLightOutline->setChecked(cRule.mIsLightOutlineEnabled);
    comboBoxOutlineMatrixSize->setCurrentIndex(cRule.mOutlineMatrixSize);
    spinBoxBadClusterWeightAdjacent_R1->setValue(cRule.mAdjWeightLst[0]);
    spinBoxBadClusterWeightDiagonal_R1->setValue(cRule.mDiagWeightLst[0]);


    spinBoxBadClusterWeightAdjacent_R2->setValue(cRule.mAdjWeightLst.size() > 1?cRule.mAdjWeightLst[1]:2);
    spinBoxBadClusterWeightDiagonal_R2->setValue(cRule.mDiagWeightLst.size() > 1?cRule.mDiagWeightLst[1]:1);
    spinBoxBadClusterWeightAdjacent_R3->setValue(cRule.mAdjWeightLst.size() > 2?cRule.mAdjWeightLst[2]:2);
    spinBoxBadClusterWeightDiagonal_R3->setValue(cRule.mDiagWeightLst.size() > 2?cRule.mDiagWeightLst[2]:1);
    spinBoxBadClusterMinimumWeighting->setValue(cRule.mFailWeight);
    // Show/hide widgets to define Matrix ratios
    changeOutlineMatrixSize(cRule.mOutlineMatrixSize);

    checkBoxIgnoreScratchLines->setChecked(cRule.mIgnoreScratchLines);			// Ignore scratch lines?
    checkBoxIgnoreScratchRows->setChecked(cRule.mIgnoreScratchRows);			// Ignore scratch rows?
    checkBoxIgnoreDiagonalBadDies->setChecked(cRule.mIgnoreDiagonalBadDies);	// Ignore diag. dies?

    comboBoxWeightingEdgeDies->setCurrentIndex(cRule.mEdgeDieWeighting);// Edge-die rule type
    doubleSpinBoxWeightingScale->setValue(cRule.mEdgeDieWeightingScale);

    // Select Edge Die Type
    int nIndexEdgeDieType = comboBoxEdgeDieType->findData(cRule.mEdgeDieType);		// Edge-die type
    comboBoxEdgeDieType->setCurrentIndex(nIndexEdgeDieType);

    OnWeightingEdgeDie();		// Clustering weighting edge die mode

    // Rule Enabled/Disabled
    groupBoxCluster->setChecked(cRule.mIsEnabled);
}

///////////////////////////////////////////////////////////
// Clustering rule Dialog box: read GUI into array
///////////////////////////////////////////////////////////
void	ClusterDefinitionDialog::readGUI(CClusterPotatoRule &cRule)
{
    // Rule name (ensure the ',' character is mapped so we do not confuse our CSV storing format)
    cRule.mRuleName = lineEditRuleName->text();
    cRule.mRuleName = cRule.mRuleName.replace(',',';');

    // Bin and color
    cRule.mSoftBin = spinBoxFailBin->value();
    cRule.mHardBin = spinBoxFailBin->value();
    cRule.mFailBinColor = pushButtonFailBinColor->activeColor();

    // Get Mask
    cRule.mMaskName = comboBox_ClusterMask->currentText();
    cRule.mMaskName = cRule.mMaskName.replace(',',';');

    if(cRule.mBadBinIdentifyList != NULL)
    {
        delete cRule.mBadBinIdentifyList;
        cRule.mBadBinIdentifyList = NULL;
    }
    cRule.mBadBinIdentifyList =
       new GS::QtLib::Range(lineEditBadBinIdentifyList->text().toLatin1().constData());
    if(cRule.mBadBinInkingList != NULL)
        delete cRule.mBadBinInkingList;
    cRule.mBadBinInkingList =
       new GS::QtLib::Range(lineEditBadBinInkingList->text().toLatin1().constData());
    cRule.mWaferSource = comboBox_ClusterBinSource->currentIndex();	// Map source: STDF Soft/Hard bin, Prober
    cRule.mClusterSize = doubleSpinBoxClusterSize->value();
    if(comboBoxClusterSizeType->currentIndex() == 1)
        cRule.mClusterSize = -cRule.mClusterSize;	// Negative value means '% over gross die'

    cRule.mOutlineWidth = spinBoxClusteringOutlineWidth->value();

    // Light outline rule
    cRule.mIsLightOutlineEnabled = groupBoxLightOutline->isChecked();
    cRule.mOutlineMatrixSize = comboBoxOutlineMatrixSize->currentIndex();

    //-- adjacente weight
    cRule.mAdjWeightLst.clear();
    cRule.mAdjWeightLst.push_back(spinBoxBadClusterWeightAdjacent_R1->value());
    //-- if ring 2 activated, add it
    if(spinBoxBadClusterWeightAdjacent_R2->isHidden() == false)
    {
        cRule.mAdjWeightLst.push_back(spinBoxBadClusterWeightAdjacent_R2->value());
    }
    else
    {
        cRule.mAdjWeightLst.push_back(2);
    }

    //-- if ring 3 activated, add it
    if(spinBoxBadClusterWeightAdjacent_R3->isHidden() == false)
    {
        cRule.mAdjWeightLst.push_back(spinBoxBadClusterWeightAdjacent_R3->value());
    }
    else
    {
        cRule.mAdjWeightLst.push_back(2);
    }

    //-- diagonal weight
    cRule.mDiagWeightLst.clear();
    cRule.mDiagWeightLst.push_back(spinBoxBadClusterWeightDiagonal_R1->value());
     //-- if ring 2 activated, add it
    if(spinBoxBadClusterWeightDiagonal_R2->isHidden() == false)
    {
        cRule.mDiagWeightLst.push_back(spinBoxBadClusterWeightDiagonal_R2->value());
    }
    else
    {
        cRule.mDiagWeightLst.push_back(1);
    }

    //-- if ring 3 activated, add it
    if(spinBoxBadClusterWeightDiagonal_R3->isHidden() == false)
    {
        cRule.mDiagWeightLst.push_back(spinBoxBadClusterWeightDiagonal_R3->value());
    }
    else
    {
        cRule.mDiagWeightLst.push_back(1);
    }


    cRule.mFailWeight = spinBoxBadClusterMinimumWeighting->value();

    // False-alamr mamagement
    cRule.mIgnoreScratchLines				= checkBoxIgnoreScratchLines->isChecked();			// Ignore scratch lines?
    cRule.mIgnoreScratchRows				= checkBoxIgnoreScratchRows->isChecked();			// Ignore scratch rows?
    cRule.mIgnoreDiagonalBadDies			= checkBoxIgnoreDiagonalBadDies->isChecked();		// Ignore diag. dies?
    cRule.mEdgeDieWeighting			= comboBoxWeightingEdgeDies->currentIndex();		// Edge-die rule type
    cRule.mEdgeDieWeightingScale	= doubleSpinBoxWeightingScale->value();
    cRule.mEdgeDieType				= comboBoxEdgeDieType->itemData(comboBoxEdgeDieType->currentIndex()).toInt();

    // Enabled/Disabled rule
    cRule.mIsEnabled = groupBoxCluster->isChecked();
}
