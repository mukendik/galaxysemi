#include "reticle_definition_dialog.h"
#include "ui_reticle_definition_dialog.h"

#include "pat_defines.h"
#include "pat_options.h"
#include "pat_option_reticle.h"
#include "pat_reticle_map_recipe.h"
#include "pat_reticle_corner_rule.h"
#include "wafer_coordinate.h"
#include <gqtl_log.h>

#include <QMenu>
#include <QPushButton>

ReticleDefinitionDialog::ReticleDefinitionDialog(QWidget *parent) :
    QDialog(parent),
    mui(new Ui::ReticleDefinitionDialog)
{
    mui->setupUi(this);
    this->setWindowTitle("Reticle definition");
}


ReticleDefinitionDialog::~ReticleDefinitionDialog()
{
    delete mui;
    for (int lIter = 0; lIter < mMaxRow; ++lIter)
        delete mBadDiesMatrix[lIter];
    delete mBadDiesMatrix;
}


void ReticleDefinitionDialog::InitGui(const COptionsPat &patOptions)
{
    mMaxRow = 15;
    mMaxCol = 20;
    // define matrix of bad dies
    mBadDiesMatrix = new int*[mMaxRow];
    for (int lIter = 0; lIter < mMaxRow; ++lIter)
        mBadDiesMatrix[lIter] = new int [mMaxCol];

    // Init grid with green pix
    ClickableGridItemLabel* lPixLabel = NULL;
    QGridLayout* layout = qobject_cast<QGridLayout*>(mui->widgetInkingParam->layout());
    QString lGreenPic   = ":/gex/icons/green_square.png";

    // Init grid pics only first launch
    if (layout && layout->isEmpty())
    {
        for(int lRowIter = 0; lRowIter< mMaxRow; ++lRowIter)
        {
            for(int lColIter = 0; lColIter < mMaxCol; ++lColIter)
            {
                lPixLabel = new ClickableGridItemLabel(lRowIter, lColIter, this);
                connect(lPixLabel, SIGNAL(clicked(int,int)), this, SLOT(OnDieClicked(int, int)));
                lPixLabel->setStyleSheet("border: 1px solid black; border-radius: 30px;");
                lPixLabel->setPixmap(QPixmap(lGreenPic));
                layout->addWidget(lPixLabel, lRowIter, lColIter);
            }
        }
    }

    // Keep list of rule name already in use
    mRules.clear();
    for (int lIdx = 0; lIdx < patOptions.GetReticleRules().count(); ++lIdx)
    {
        mRules.append(patOptions.GetReticleRules().at(lIdx).GetRuleName());
    }

    // Reticle map source combobox
    mui->comboBox_ReticleMapSource->clear();
    mui->comboBox_ReticleMapSource->addItem("STDF SoftBin map",
                                            QVariant(GEX_PAT_WAFMAP_SRC_SOFTBIN));
    mui->comboBox_ReticleMapSource->addItem("STDF HardBin map",
                                            QVariant(GEX_PAT_WAFMAP_SRC_HARDBIN));
    mui->comboBox_ReticleMapSource->addItem("External map",
                                            QVariant(GEX_PAT_WAFMAP_SRC_PROBER));

    // Reticule Defectivity Check: Field selection
    mui->comboBoxFieldSelection->clear();
    mui->comboBoxFieldSelection->addItem("All fields", QVariant(PATOptionReticle::ALL_RETICLE_FIELDS));
    mui->comboBoxFieldSelection->addItem("List of fieds", QVariant(PATOptionReticle::LIST_RETICLE_FIELDS));
    mui->comboBoxFieldSelection->addItem("Edge fields", QVariant(PATOptionReticle::EDGE_RETICLE_FIELDS));
    mui->comboBoxFieldSelection->setCurrentIndex(0);

    // Select Yield threshold = 50%
    mui->doubleSpinBoxReticuleYieldLevel->setValue(50.00);

    // Rules
    mui->comboBoxReticleRule->clear();
    mui->comboBoxReticleRule->addItem("Repeating reticle patterns",
                                      QVariant(PATOptionReticle::REPEATING_PATTERNS));
    mui->comboBoxReticleRule->addItem("Corner rule",
                                      QVariant(PATOptionReticle::CORNER));
    mui->comboBoxReticleRule->addItem("Step defectivity check",
                                      QVariant(PATOptionReticle::STEP_DEFECTIVITY_CHECK));
    mui->checkBoxCornerBLeft->setChecked(true);
    mui->checkBoxCornerBRight->setChecked(true);
    mui->checkBoxCornerTLeft->setChecked(true);
    mui->checkBoxCornerTRight->setChecked(true);

    mui->spinBoxXInk->setValue(1);
    mui->spinBoxYInk->setValue(1);
    mui->spinBoxDiagInk->setValue(1);
    mui->spinBoxDiagOffXInk->setValue(0);
    mui->spinBoxDiagOffYInk->setValue(0);

    mui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    FillReticleMaskList(patOptions);

    ResetBadDies();

    RefreshUI();

    connect(mui->comboBoxReticleRule,       SIGNAL(activated(int)),
            this, SLOT(OnUpdateEnabledFrames()));
    connect(mui->checkBoxAllEdgeCorner,     SIGNAL(clicked()),
            this, SLOT(OnCheckAllCornerUpdated()));
    connect(mui->checkBoxCornerBLeft,       SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxCornerBRight,      SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeBottom,        SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxCornerTLeft,       SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxCornerTRight,      SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeTop,           SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeLeft,          SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeRight,         SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->spinBoxXInk,               SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxYInk,               SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxDiagInk,            SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxDiagOffXInk,        SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxDiagOffYInk,        SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->checkBoxIgnoreDiagBadDies, SIGNAL(clicked(bool)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->widgetInkingParam,         SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnCustomContextMenuInking(QPoint)));
    connect(mui->lineEditRuleName,          SIGNAL(editingFinished()),
            this, SLOT(OnUpdateOkButton()));
    connect(mui->lineEditRuleName,          SIGNAL(textChanged(const QString &)),
            this, SLOT(OnUpdateOkButton()));
    connect(mui->comboBoxFieldSelection,    SIGNAL(activated(int)),
            this, SLOT(OnUpdateDefectivityCheckSection()));
    connect(mui->lineEditFieldSelection,    SIGNAL(textChanged(QString)),
            this, SLOT(OnUpdateOkButton()));
    connect(mui->lineEditFieldSelection,    SIGNAL(editingFinished()),
            this, SLOT(OnUpdateOkButton()));

    mui->stackedWidgetReticle->setCurrentWidget(mui->pageRepeatingPattern);
}


void ReticleDefinitionDialog::LoadGui(const PATOptionReticle &reticleRule)
{
    // Remove name of the rule being edited.
    mRules.removeAll(reticleRule.GetRuleName());

    disconnect(mui->comboBoxReticleRule,       SIGNAL(activated(int)),
            this, SLOT(OnUpdateEnabledFrames()));
    disconnect(mui->checkBoxAllEdgeCorner,     SIGNAL(clicked()),
            this, SLOT(OnCheckAllCornerUpdated()));
    disconnect(mui->checkBoxCornerBLeft,       SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxCornerBRight,      SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxEdgeBottom,        SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxCornerTLeft,       SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxCornerTRight,      SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxEdgeTop,           SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxEdgeLeft,          SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->checkBoxEdgeRight,         SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    disconnect(mui->spinBoxXInk,               SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    disconnect(mui->spinBoxYInk,               SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    disconnect(mui->spinBoxDiagInk,            SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    disconnect(mui->spinBoxDiagOffXInk,        SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    disconnect(mui->spinBoxDiagOffYInk,        SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    disconnect(mui->checkBoxIgnoreDiagBadDies, SIGNAL(clicked(bool)),
            this, SLOT(OnUpdateInkingParameters()));
    disconnect(mui->widgetInkingParam,         SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnCustomContextMenuInking(QPoint)));
    disconnect(mui->lineEditRuleName,          SIGNAL(editingFinished()),
            this, SLOT(OnUpdateOkButton()));
    disconnect(mui->lineEditRuleName,          SIGNAL(textChanged(const QString &)),
            this, SLOT(OnUpdateOkButton()));
    disconnect(mui->comboBoxFieldSelection,    SIGNAL(activated(int)),
            this, SLOT(OnUpdateDefectivityCheckSection()));
    disconnect(mui->lineEditFieldSelection,    SIGNAL(editingFinished()),
            this, SLOT(OnUpdateOkButton()));
    disconnect(mui->lineEditFieldSelection,    SIGNAL(textChanged(const QString &)),
            this, SLOT(OnUpdateOkButton()));

    // Bin and color
    mui->spinBoxFailBin_3->setValue(reticleRule.GetReticleSBin());
    mui->pushButtonFailBinColor1->setActiveColor(reticleRule.GetReticleColor());

    int lMaskIdx = mui->comboBox_ReticleMask->findText(reticleRule.GetReticleMaskName());

    if (lMaskIdx >= 0)
        mui->comboBox_ReticleMask->setCurrentIndex(lMaskIdx);
    else
        mui->comboBox_ReticleMask->setCurrentIndex(0);

    mui->groupBoxReticle->setChecked(reticleRule.IsReticleEnabled());

    mui->lineEditBadBinListReticle->setText(reticleRule.GetBadBinsReticleList().GetRangeList());
    mui->lineEditRuleName->setText(reticleRule.GetRuleName());
    mui->comboBox_ReticleMapSource->setCurrentIndex(reticleRule.GetReticle_WafermapSource());

    int lReticleRuleIdx = mui->comboBoxReticleRule->findData(reticleRule.GetRule());
    if (lReticleRuleIdx >= 0)
        mui->comboBoxReticleRule->setCurrentIndex(lReticleRuleIdx);
    else
        mui->comboBoxReticleRule->setCurrentIndex(0);

    mui->doubleSpinBoxReticuleYieldLevel->setValue(reticleRule.GetReticleYieldThreshold());
    mui->checkBoxEdgeLeft->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::LEFT));
    mui->checkBoxEdgeTop->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::TOP));
    mui->checkBoxCornerBRight->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_RIGHT));
    mui->checkBoxCornerTLeft->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::CORNER_TOP_LEFT));
    mui->checkBoxEdgeRight->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::RIGHT));
    mui->checkBoxCornerBLeft->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::CORNER_BOTTOM_LEFT));
    mui->checkBoxCornerTRight->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::CORNER_TOP_RIGHT));
    mui->checkBoxEdgeBottom->setChecked(reticleRule.IsActivatedCorner(PATOptionReticle::BOTTOM));
    mui->spinBoxXInk->setValue(reticleRule.GetXInk());
    mui->spinBoxYInk->setValue(reticleRule.GetYInk());
    mui->spinBoxDiagInk->setValue(reticleRule.GetDiagInk());
    mui->spinBoxDiagOffXInk->setValue(reticleRule.GetXOffDiag());
    mui->spinBoxDiagOffYInk->setValue(reticleRule.GetYOffDiag());
    mui->checkBoxIgnoreDiagBadDies->setChecked(reticleRule.IgnoreDiagonalBadDies());
 	mui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!mui->lineEditRuleName->text().isEmpty());

    int lFieldSelectionIdx = mui->comboBoxFieldSelection->findData(reticleRule.GetFieldSelection());
    if (lFieldSelectionIdx >= 0)
        mui->comboBoxFieldSelection->setCurrentIndex(lFieldSelectionIdx);
    mui->lineEditFieldSelection->setText(PATOptionReticle::FieldCoordinatesToString(reticleRule.GetFieldCoordinates()));
    mui->spinBoxRetYieldStepDefCheck->setValue(reticleRule.GetFieldThreshold());

    connect(mui->comboBoxReticleRule,       SIGNAL(activated(int)),
            this, SLOT(OnUpdateEnabledFrames()));
    connect(mui->checkBoxAllEdgeCorner,     SIGNAL(clicked()),
            this, SLOT(OnCheckAllCornerUpdated()));
    connect(mui->checkBoxCornerBLeft,       SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxCornerBRight,      SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeBottom,        SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxCornerTLeft,       SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxCornerTRight,      SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeTop,           SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeLeft,          SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->checkBoxEdgeRight,         SIGNAL(clicked()),
            this, SLOT(OnCheckAnyCornerUpdated()));
    connect(mui->spinBoxXInk,               SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxYInk,               SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxDiagInk,            SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxDiagOffXInk,        SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->spinBoxDiagOffYInk,        SIGNAL(valueChanged(int)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->checkBoxIgnoreDiagBadDies, SIGNAL(clicked(bool)),
            this, SLOT(OnUpdateInkingParameters()));
    connect(mui->widgetInkingParam,         SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnCustomContextMenuInking(QPoint)));
    connect(mui->lineEditRuleName,          SIGNAL(editingFinished()),
            this, SLOT(OnUpdateOkButton()));
    connect(mui->lineEditRuleName,          SIGNAL(textChanged(const QString &)),
            this, SLOT(OnUpdateOkButton()));
    connect(mui->comboBoxFieldSelection,    SIGNAL(activated(int)),
            this, SLOT(OnUpdateDefectivityCheckSection()));
    connect(mui->lineEditFieldSelection,    SIGNAL(editingFinished()),
            this, SLOT(OnUpdateOkButton()));
    connect(mui->lineEditFieldSelection,    SIGNAL(textChanged(const QString &)),
            this, SLOT(OnUpdateOkButton()));

    RefreshUI();
}


void ReticleDefinitionDialog::ReadGUI(PATOptionReticle &reticleRule)
{
    // Extract data from UI to load reticle object
    reticleRule.SetReticleEnabled(mui->groupBoxReticle->isChecked());
    reticleRule.SetRuleName(
                mui->lineEditRuleName->text());
    reticleRule.SetReticleSBin(mui->spinBoxFailBin_3->value());
    reticleRule.SetReticleHBin(mui->spinBoxFailBin_3->value());
    reticleRule.SetReticleColor(mui->pushButtonFailBinColor1->activeColor());
    reticleRule.SetReticle_WafermapSource(
                mui->comboBox_ReticleMapSource->currentData().toInt());
    reticleRule.SetReticleYieldThreshold(
                mui->doubleSpinBoxReticuleYieldLevel->value());
    reticleRule.SetBadBinsReticleList(GS::QtLib::Range(mui->lineEditBadBinListReticle->text()));
    reticleRule.SetReticleMaskName(
                mui->comboBox_ReticleMask->currentText());
    reticleRule.SetRule(static_cast<PATOptionReticle::Rule>(
                mui->comboBoxReticleRule->currentData().toInt()));
    reticleRule.SetActivatedCorners(GetCheckedCornerEdges());
    reticleRule.SetXInk(mui->spinBoxXInk->value());
    reticleRule.SetYInk(mui->spinBoxYInk->value());
    reticleRule.SetDiagInk(mui->spinBoxDiagInk->value());
    reticleRule.SetXOffDiag(mui->spinBoxDiagOffXInk->value());
    reticleRule.SetYOffDiag(mui->spinBoxDiagOffYInk->value());
    reticleRule.SetIgnoreDiagonalBadDies(mui->checkBoxIgnoreDiagBadDies->isChecked());
    reticleRule.SetFieldSelection(static_cast<PATOptionReticle::FieldSelection>
                                  (mui->comboBoxFieldSelection->currentData().toInt()));
    reticleRule.SetFieldCoordinates(PATOptionReticle::FieldCoordinatesFromString(mui->lineEditFieldSelection->text()));
    reticleRule.SetFieldThreshold(mui->spinBoxRetYieldStepDefCheck->value());
}


void ReticleDefinitionDialog::RefreshUI()
{
    OnUpdateEnabledFrames();
    OnUpdateInkingArea();
    OnUpdateInkingParameters();
    OnUpdateDefectivityCheckSection();
}

PATOptionReticle::ActivatedCorners ReticleDefinitionDialog::GetCheckedCornerEdges()
{
    PATOptionReticle::ActivatedCorners lActivatedCorners = PATOptionReticle::NO_CORNER;

    lActivatedCorners |= (mui->checkBoxEdgeLeft->isChecked() ?
                                   PATOptionReticle::LEFT : PATOptionReticle::NO_CORNER);
    lActivatedCorners |= mui->checkBoxEdgeTop->isChecked() ?
                                   PATOptionReticle::TOP : PATOptionReticle::NO_CORNER;
    lActivatedCorners |= mui->checkBoxCornerBRight->isChecked() ?
                                   PATOptionReticle::CORNER_BOTTOM_RIGHT : PATOptionReticle::NO_CORNER;
    lActivatedCorners |= mui->checkBoxCornerTLeft->isChecked() ?
                                   PATOptionReticle::CORNER_TOP_LEFT : PATOptionReticle::NO_CORNER;
    lActivatedCorners |= mui->checkBoxEdgeRight->isChecked() ?
                                   PATOptionReticle::RIGHT : PATOptionReticle::NO_CORNER;
    lActivatedCorners |= mui->checkBoxCornerBLeft->isChecked() ?
                                   PATOptionReticle::CORNER_BOTTOM_LEFT : PATOptionReticle::NO_CORNER;
    lActivatedCorners |= mui->checkBoxCornerTRight->isChecked() ?
                                   PATOptionReticle::CORNER_TOP_RIGHT : PATOptionReticle::NO_CORNER;
    lActivatedCorners |= mui->checkBoxEdgeBottom->isChecked() ?
                                   PATOptionReticle::BOTTOM : PATOptionReticle::NO_CORNER;

    return lActivatedCorners;
}

void ReticleDefinitionDialog::ResetBadDies()
{
    ClearBadDies();

    // die 1 [bottom/left]
    mBadDiesMatrix[mMaxRow-1][0] = 1;
    // die 2 [top middle]
    mBadDiesMatrix[0][(int)(mMaxCol/2)] = 1;
    // die 3 [diag bad of die 2]
    mBadDiesMatrix[1][(int)(mMaxCol/2) + 1] = 1;
    // die 4 [bottom middle]
    mBadDiesMatrix[mMaxRow-1][(int)(mMaxCol/2)] = 1;
    // die 5 [right]
    mBadDiesMatrix[(int)(mMaxRow/2)][mMaxCol - 1] = 1;
    // die 6 [grouped with die 5]
    mBadDiesMatrix[(int)(mMaxRow/2) - 1][mMaxCol - 1] = 1;
    // die 7 [grouped with die 5]
    mBadDiesMatrix[(int)(mMaxRow/2) - 2][mMaxCol - 1] = 1;
    // die 8 [grouped with die 5 but more close to the center]
    mBadDiesMatrix[(int)(mMaxRow/2) - 2][mMaxCol - 2] = 1;
}

void ReticleDefinitionDialog::ClearBadDies()
{
    // Init bad dies matrix
    for(int lRowIter = 0; lRowIter< mMaxRow; ++lRowIter)
    {
        for(int lColIter = 0; lColIter < mMaxCol; ++lColIter)
        {
            mBadDiesMatrix[lRowIter][lColIter] = 0;
        }
    }
}

void ReticleDefinitionDialog::OnDieClicked(int row, int col)
{
    // if bad set it to good
    if (mBadDiesMatrix[row][col] == 1)
    {
        mBadDiesMatrix[row][col] = 0;
    }
    // else set it to bad
    else
    {
        mBadDiesMatrix[row][col] = 1;
    }
    // re paint
    OnUpdateInkingParameters();
}

void ReticleDefinitionDialog::OnCheckAllCornerUpdated()
{
   bool lCheckAll = mui->checkBoxAllEdgeCorner->isChecked();

   mui->checkBoxEdgeLeft->setChecked(lCheckAll);
   mui->checkBoxEdgeTop->setChecked(lCheckAll);
   mui->checkBoxCornerBRight->setChecked(lCheckAll);
   mui->checkBoxCornerTLeft->setChecked(lCheckAll);
   mui->checkBoxEdgeRight->setChecked(lCheckAll);
   mui->checkBoxCornerBLeft->setChecked(lCheckAll);
   mui->checkBoxCornerTRight->setChecked(lCheckAll);
   mui->checkBoxEdgeBottom->setChecked(lCheckAll);

   OnUpdateInkingArea();
}

void ReticleDefinitionDialog::OnCheckAnyCornerUpdated()
{
    bool lCheckAll = true;

    lCheckAll &= mui->checkBoxEdgeLeft->isChecked();
    lCheckAll &= mui->checkBoxEdgeTop->isChecked();
    lCheckAll &= mui->checkBoxCornerBRight->isChecked();
    lCheckAll &= mui->checkBoxCornerTLeft->isChecked();
    lCheckAll &= mui->checkBoxEdgeRight->isChecked();
    lCheckAll &= mui->checkBoxCornerBLeft->isChecked();
    lCheckAll &= mui->checkBoxCornerTRight->isChecked();
    lCheckAll &= mui->checkBoxEdgeBottom->isChecked();

    mui->checkBoxAllEdgeCorner->setChecked(lCheckAll);

    OnUpdateInkingArea();
}

void ReticleDefinitionDialog::OnCustomContextMenuInking(QPoint)
{
    QMenu *lMenu=new QMenu(this);
    lMenu->addAction("Reset bad dies", this, SLOT(OnResetBadDiesRequested()));
    lMenu->addAction("Clear bad dies", this, SLOT(OnClearBadDiesRequested()));
    lMenu->exec(QCursor::pos());
}

void ReticleDefinitionDialog::OnResetBadDiesRequested()
{
    ResetBadDies();
    OnUpdateInkingParameters();
}

void ReticleDefinitionDialog::OnClearBadDiesRequested()
{
    ClearBadDies();
    OnUpdateInkingParameters();
}

void ReticleDefinitionDialog::OnUpdateOkButton()
{
    bool lOkEnable = true;

    if (mui->lineEditRuleName->text().isEmpty() || mRules.contains(mui->lineEditRuleName->text(), Qt::CaseInsensitive))
    {
        QPalette lPalette = mui->lineEditRuleName->palette();
        lPalette.setColor(QPalette::Text, Qt::red);
        lPalette.setColor(QPalette::ToolTipText, Qt::red);
        mui->lineEditRuleName->setToolTip("Please change rule name, this one is already in use");
        mui->lineEditRuleName->setPalette(lPalette);

        lOkEnable = false;
    }
    else
    {
        mui->lineEditRuleName->setToolTip("Please enter a rule name");
        mui->lineEditRuleName->setPalette(QPalette());
        mui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!mui->lineEditRuleName->text().isEmpty());
    }

    if (mui->comboBoxFieldSelection->currentData().toInt() == PATOptionReticle::LIST_RETICLE_FIELDS)
    {
        bool lOkCoord = false;

        PATOptionReticle::FieldCoordinatesFromString(mui->lineEditFieldSelection->text(), &lOkCoord);

        if (lOkCoord == false)
        {
            QPalette lPalette = mui->lineEditFieldSelection->palette();
            lPalette.setColor(QPalette::Text, Qt::red);
            mui->lineEditFieldSelection->setPalette(lPalette);

            lOkEnable = false;
        }
        else
        {
            mui->lineEditFieldSelection->setPalette(QPalette());
        }
    }

    mui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(lOkEnable);
}

void ReticleDefinitionDialog::OnUpdateDefectivityCheckSection()
{
    int lFields = mui->comboBoxFieldSelection->currentData().toInt();

    switch(lFields)
    {
        case PATOptionReticle::ALL_RETICLE_FIELDS:
        case PATOptionReticle::EDGE_RETICLE_FIELDS:
            mui->lineEditFieldSelection->setVisible(false);
            break;

        case PATOptionReticle::LIST_RETICLE_FIELDS:
            mui->lineEditFieldSelection->setVisible(true);
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "Unknown fields selection");
            break;
    }

    OnUpdateOkButton();
}

void ReticleDefinitionDialog::OnUpdateEnabledFrames()
{
    // Enable / Disable group box according to chosen rule
    if (mui->comboBoxReticleRule->currentData() == PATOptionReticle::CORNER)
    {
        mui->stackedWidgetReticle->setCurrentWidget(mui->pageCornerRule);
    }
    else if (mui->comboBoxReticleRule->currentData() == PATOptionReticle::REPEATING_PATTERNS)
    {
        mui->stackedWidgetReticle->setCurrentWidget(mui->pageRepeatingPattern);
    }
    else if (mui->comboBoxReticleRule->currentData() == PATOptionReticle::STEP_DEFECTIVITY_CHECK)
    {
        mui->stackedWidgetReticle->setCurrentWidget(mui->pageDefectivityCheck);
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, "Invalid Reticle rules detected");
    }
}


void ReticleDefinitionDialog::FillReticleMaskList(const COptionsPat &patOptions)
{
    mui->comboBox_ReticleMask->clear();
    mui->comboBox_ReticleMask->addItem("-None-");
    for(int lIdx = 0; lIdx < patOptions.mMaskRules.count(); ++lIdx)
        mui->comboBox_ReticleMask->addItem(patOptions.mMaskRules.at(lIdx)->mRuleName);

    mui->comboBox_ReticleMask->setCurrentIndex(0);
}


void ReticleDefinitionDialog::OnUpdateInkingArea()
{
    QString lGreenPic = ":/gex/icons/green_square.png";
    QString lYellowPic = ":/gex/icons/yellow_square.png";

    // Init grid with green pix
    QLabel* lPixLabel = NULL;
    QGridLayout* layout = qobject_cast<QGridLayout*>(mui->widgetCornerEdgeArea->layout());
    // Init only if needed
    if (layout && layout->isEmpty())
    {
        for(int lRowIter = 0; lRowIter< mMaxRow; ++lRowIter)
        {
            for(int lColIter = 0; lColIter < mMaxCol; ++lColIter)
            {
                lPixLabel = new QLabel();
                lPixLabel->setStyleSheet("border: 1px solid black; border-radius: 30px;");
                lPixLabel->setPixmap(QPixmap(lGreenPic));
                layout->addWidget(lPixLabel, lRowIter, lColIter);
            }
        }
    }

    // Update pix depending on checkboxes values
    lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(mMaxRow - 1, 0)->widget());
    lPixLabel->setPixmap(mui->checkBoxCornerBLeft->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));

    for(int lRowIter =1; lRowIter<(mMaxRow - 1); ++lRowIter)
    {
        lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(lRowIter, 0)->widget());
        lPixLabel->setPixmap(mui->checkBoxEdgeLeft->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));
    }

    lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(0, 0)->widget());
    lPixLabel->setPixmap(mui->checkBoxCornerTLeft->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));

    for(int lColIter =1; lColIter<(mMaxCol - 1); ++lColIter)
    {
        lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(0, lColIter)->widget());
        lPixLabel->setPixmap(mui->checkBoxEdgeTop->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));
    }

    lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(0, mMaxCol - 1)->widget());
    lPixLabel->setPixmap(mui->checkBoxCornerTRight->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));

    for(int lRowIter =1; lRowIter<(mMaxRow - 1); ++lRowIter)
    {
        lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(lRowIter, mMaxCol - 1)->widget());
        lPixLabel->setPixmap(mui->checkBoxEdgeRight->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));
    }

    lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(mMaxRow - 1, mMaxCol - 1)->widget());
    lPixLabel->setPixmap(mui->checkBoxCornerBRight->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));

    for(int lColIter =1; lColIter<(mMaxCol - 1); ++lColIter)
    {
        lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
            (mui->widgetCornerEdgeArea->layout())->itemAtPosition(mMaxRow -1, lColIter)->widget());
        lPixLabel->setPixmap(mui->checkBoxEdgeBottom->isChecked()
            ? QPixmap(lYellowPic) : QPixmap(lGreenPic));
    }

    // update inked bad dies...
    OnUpdateInkingParameters();
}


void ReticleDefinitionDialog::OnUpdateInkingParameters()
{
    QString lGreenPic   = ":/gex/icons/green_square.png";
    QString lRedPic     = ":/gex/icons/red_square.png";
    QString lBluePic    = ":/gex/icons/blue_square.png";
    QLabel* lPixLabel  = NULL;

    GS::Gex::PatReticleMapRecipe    lReticleMap(mMaxCol, mMaxRow, 1);
    PATOptionReticle                lReticleSettings;

    ReadGUI(lReticleSettings);

    // Force bad bins liste to match with reticle in the recipe GUI
    // Bin 1 is a good die, all other bins are bad
    lReticleSettings.SetBadBinsReticleList(GS::QtLib::Range("0, 2-65535"));

    // bad dies locations
    for(int lRowIter = 0; lRowIter< mMaxRow; ++lRowIter)
    {
        for(int lColIter = 0; lColIter < mMaxCol; ++lColIter)
        {
            if (mBadDiesMatrix[lRowIter][lColIter] == 1)
                lReticleMap.SetBin(GS::Gex::WaferCoordinate(lColIter, lRowIter), 2);
        }
    }

    GS::Gex::PatReticleCornerRule lAlgo(lReticleSettings);

    if (lAlgo.ProcessReticleMap(&lReticleMap))
    {
        // Draw
        for(int lRowIter = 0; lRowIter< mMaxRow; ++lRowIter)
        {
            for(int lColIter = 0; lColIter < mMaxCol; ++lColIter)
            {
                lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
                            (mui->widgetInkingParam->layout())
                            ->itemAtPosition(lRowIter, lColIter)->widget());
                if (lReticleMap.GetBin(GS::Gex::WaferCoordinate(lColIter, lRowIter)) == 2)
                    lPixLabel->setPixmap(QPixmap(lRedPic));
                else
                    lPixLabel->setPixmap(QPixmap(lGreenPic));
            }
        }

        // Outliers
    }

    QList<GS::Gex::WaferCoordinate> lOutlier = lAlgo.GetOutliers();

    for (int lIdx = 0; lIdx < lOutlier.count(); ++lIdx)
    {
        lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
                    (mui->widgetInkingParam->layout())
                    ->itemAtPosition(lOutlier.at(lIdx).GetY(), lOutlier.at(lIdx).GetX())->widget());
        lPixLabel->setPixmap(QPixmap(lBluePic));
    }


    // Draw bad dies
    for(int lRowIter = 0; lRowIter< mMaxRow; ++lRowIter)
    {
        for(int lColIter = 0; lColIter < mMaxCol; ++lColIter)
        {
            if (mBadDiesMatrix[lRowIter][lColIter] == 1)
            {
                lPixLabel = qobject_cast<QLabel*>(qobject_cast<QGridLayout*>
                            (mui->widgetInkingParam->layout())
                            ->itemAtPosition(lRowIter, lColIter)->widget());
                lPixLabel->setPixmap(QPixmap(lRedPic));
            }
        }
    }
 }
