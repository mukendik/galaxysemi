#ifdef GCORE15334

#include "gqtl_log.h"
#include "engine.h"
#include "message.h"
#include "gex_report.h"
#include "tb_toolbox.h"
#include "pat_defines.h"
#include "pat_options.h"
#include "pat_recipe_io.h"
#include "browser_dialog.h"
#include "pat_limits_dialog.h"
#include "tb_pat_outlier_removal.h"
#include "pat_recipe_editor.h"
#include "gex_pat_constants_extern.h"
#include "reticle_definition_dialog.h"
#include "nnr_definition_dialog.h"
#include "iddq_delta_definition_dialog.h"
#include "cluster_definition_dialog.h"
#include "gdbn_definition_dialog.h"
#include "mask_definition_dialog.h"

#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QColor>
#include <QString>

extern GexMainwindow *      pGexMainWindow;
extern CGexReport* gexReport;

extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);

// Sets Combo-box active string.
///////////////////////////////////////////////////////////
static	int getComboBoxTextItem(QComboBox *pCombo, QString &strComboText)
{
    QString strText;
    int iIndex=0;
    for(iIndex = 0;iIndex < pCombo->count(); iIndex++)
    {
        strText = pCombo->itemText(iIndex);
        if(strText == strComboText)
            return iIndex;
    }

    // String not in combo!
    return 0;
}

namespace GS
{
namespace Gex
{

PatLimitsDialog::PatLimitsDialog(COptionsPat *OptionsPat, QWidget* parent, bool modal, Qt::WFlags fl)
    : QDialog(parent, fl), mOptionsPat(OptionsPat)
{
    GSLOG(6, "new PatLimits Dialog");
    setupUi(this);
    setModal(modal);

    tabWidget4->removeTab(tabWidget4->indexOf(TabMonitoring));

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    // Fill final test traceability combobox
    comboFinalTest_Traceability->clear();
    comboFinalTest_Traceability->addItem(QIcon(QString::fromUtf8(":/gex/icons/explorer.png")),
                                         "PAT Settings stored in HTML file",
                                         QVariant(GEX_TPAT_TRACEABILITY_HTML));
    comboFinalTest_Traceability->addItem(QIcon(QString::fromUtf8(":/gex/icons/stop.png")),
                                         "Disabled",
                                         QVariant(GEX_TPAT_TRACEABILITY_NONE));


    // Fill Reticle Source combobox
    comboBoxReticleSource->clear();
    comboBoxReticleSource->addItem("From fixed values", QVariant(PATOptionReticle::RETICLE_SIZE_FIXED));
    comboBoxReticleSource->addItem("From file", QVariant(PATOptionReticle::RETICLE_SIZE_FILE));
    comboBoxReticleSource->setCurrentIndex(0);

    // Fill 'PAT limits built from' combo box
    comboBoxProcess->clear();
    comboBoxProcess->addItem("All Data / parts (any Bin)",  QVariant(GEX_TPAT_BUILDLIMITS_ALLBINS));
    comboBoxProcess->addItem("Good SoftBins only",          QVariant(GEX_TPAT_BUILDLIMITS_GOODSOFTBINS));
    comboBoxProcess->addItem("Good HardBins only",          QVariant(GEX_TPAT_BUILDLIMITS_GOODHARDBINS));
    comboBoxProcess->addItem("List of SoftBins...",         QVariant(GEX_TPAT_BUILDLIMITS_LISTSOFTBINS));
    comboBoxProcess->addItem("List of HardBins...",         QVariant(GEX_TPAT_BUILDLIMITS_LISTHARDBINS));
    comboBoxProcess->addItem("All SoftBins except...",      QVariant(GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS));
    comboBoxProcess->addItem("All HardBins except...",      QVariant(GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS));

    connect(pushButtonProcessFile,					SIGNAL(clicked()),		this, SLOT(OnOk()));
    connect(comboBoxRuleSet,						SIGNAL(activated(int)), this, SLOT(OnRuleSet()));
    connect(groupBoxStatic,                         SIGNAL(toggled(bool)),	this, SLOT(OnEnableLimits()));
    connect(groupBoxDynamic,						SIGNAL(toggled(bool)),	this, SLOT(OnEnableLimits()));
    connect(pushButtonCancel,						SIGNAL(clicked()),		this, SLOT(reject()));
    connect(checkBoxIgnoreCpk,						SIGNAL(toggled(bool)),	this, SLOT(OnIgnoreHighCpk()));
    connect(comboBoxDistribution,					SIGNAL(activated(int)), this, SLOT(OnDistribution()));
    connect(checkBoxFinalTest_BaselineOutliers,     SIGNAL(clicked()),		this, SLOT(OnBaseLineMaxOutliers()));
    connect(checkBoxHistoricalCpk,					SIGNAL(clicked()),		this, SLOT(OnHistoricalCpk()));
    connect(comboBoxAlgorithm,						SIGNAL(activated(int)), this, SLOT(OnAlgorithm()));
    connect(groupBoxCustomAlgorithm,				SIGNAL(toggled(bool)),	this, SLOT(OnCustomAlgorithm()));
    connect(comboBoxCustomPatLib,					SIGNAL(activated(int)), this, SLOT(OnPatPluginChanged()));
    connect(pushButtonIncrementVersion,             SIGNAL(clicked()),		this, SLOT(OnIncrementVersion()));

    // NNR
    connect(lineEditNNR_Bin,                SIGNAL(textChanged(QString)),
            this,                           SLOT(OnNNRBinChanged()));
    connect(pushButtonNNR_Color,            SIGNAL(clicked()),
            this,                           SLOT(OnNNRColorChanged()));
    connect(pushButtonAddNNR_Rule,          SIGNAL(clicked()),		this, SLOT(OnAddNNR_Rule()));
    connect(pushButtonEditNNR_Rule,         SIGNAL(clicked()),		this, SLOT(OnEditNNR_Rule()));
    connect(pushButtonDuplicateNNR_Rule,	SIGNAL(clicked()),		this, SLOT(OnDuplicateNNR_Rule()));
    connect(pushButtonUpNNR_Rule,           SIGNAL(clicked()),		this, SLOT(OnMoveUpNNR_Rule()));
    connect(pushButtonDownNNR_Rule,         SIGNAL(clicked()),		this, SLOT(OnMoveDownNNR_Rule()));
    connect(pushButtonRemoveNNR_Rule,       SIGNAL(clicked()),		this, SLOT(OnRemoveNNR_Rule()));
    connect(tableWidgetNNR_Rules,           SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
            this, SLOT(OnEditNNR_Rule()));

    // IDDQ-Delta
    connect(lineEditIDDQ_Delta_Bin,         SIGNAL(textChanged(QString)),
            this,                           SLOT(OnIDDQBinChanged()));
    connect(pushButtonIDDQ_Delta_Color,     SIGNAL(clicked()),
            this,                           SLOT(OnIDDQColorChanged()));
    connect(pushButtonAddIDDQ_Delta_Rule,		SIGNAL(clicked()),		this, SLOT(OnAddIDDQ_Delta_Rule()));
    connect(pushButtonEditIDDQ_Delta_Rule,		SIGNAL(clicked()),		this, SLOT(OnEditIDDQ_Delta_Rule()));
    connect(pushButtonDuplicateIDDQ_Delta_Rule,	SIGNAL(clicked()),		this, SLOT(OnDuplicateIDDQ_Delta_Rule()));
    connect(pushButtonUpIDDQ_Delta_Rule,	SIGNAL(clicked()),		this, SLOT(OnMoveUpIDDQ_Delta_Rule()));
    connect(pushButtonDownIDDQ_Delta_Rule,	SIGNAL(clicked()),		this, SLOT(OnMoveDownIDDQ_Delta_Rule()));
    connect(pushButtonRemoveIDDQ_Delta_Rule,	SIGNAL(clicked()),		this, SLOT(OnRemoveIDDQ_Delta_Rule()));
    connect(tableWidgetIDDQ_Delta_Rules, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnEditIDDQ_Delta_Rule()));

    // GDBN
    connect(lineEditBadClusterBin,          SIGNAL(textChanged(QString)),
            this,                           SLOT(OnGdbnBinChanged()));
    connect(pushButtonBadClusterColor,      SIGNAL(clicked()),
            this,                           SLOT(OnGdbnColorChanged()));
    connect(pushButtonAddGdbnRule,		SIGNAL(clicked()),		this, SLOT(OnAddGdbnRule()));
    connect(pushButtonEditGdbnRule,		SIGNAL(clicked()),		this, SLOT(OnEditGdbnRule()));
    connect(pushButtonDuplicateGdbnRule,	SIGNAL(clicked()),		this, SLOT(OnDuplicateGdbnRule()));
    connect(pushButtonUpGdbnRule,	SIGNAL(clicked()),		this, SLOT(OnMoveUpGdbnRule()));
    connect(pushButtonDownGdbnRule,	SIGNAL(clicked()),		this, SLOT(OnMoveDownGdbnRule()));
    connect(pushButtonRemoveGdbnRule,	SIGNAL(clicked()),		this, SLOT(OnRemoveGdbnRule()));
    connect(tableWidgetGdbnRules, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnEditGdbnRule()));

    // Reticle
    connect(lineEditBadReticule,            SIGNAL(textChanged(QString)),
            this,                           SLOT(OnReticleBinChanged()));
    connect(pushButtonBadReticule,          SIGNAL(clicked()),
            this,                           SLOT(OnReticleColorChanged()));
    connect(tableWidgetReticleRules,        SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnEditReticleRule()));
    connect(tableWidgetReticleRules,        SIGNAL(itemSelectionChanged()), this, SLOT(OnReticleListUpdated()));
    connect(pushButtonAddReticleRule,       SIGNAL(clicked()),    this,   SLOT(OnAddReticleRule()));
    connect(pushButtonEditReticleRule,      SIGNAL(clicked()),   this,   SLOT(OnEditReticleRule()));
    connect(pushButtonRemoveReticleRule,    SIGNAL(clicked()), this,   SLOT(OnRemoveReticleRule()));
    connect(pushButtonDuplicateReticleRule, SIGNAL(clicked()), this, SLOT(OnDuplicateReticleRule()));
    connect(pushButtonUpReticleRule,        SIGNAL(clicked()),		this, SLOT(OnMoveUpReticleRule()));
    connect(pushButtonDownReticleRule,      SIGNAL(clicked()),		this, SLOT(OnMoveDownReticleRule()));
    connect(comboBoxReticleSource,          SIGNAL(activated(int)), this, SLOT(OnReticleSourceChanged()));

    // Clustering
    connect(lineEditClusteringPotatoBin,        SIGNAL(textChanged(QString)),
            this,                               SLOT(OnClusteringBinChanged()));
    connect(pushButtonClusteringPotatoColor,    SIGNAL(clicked()),
            this,                               SLOT(OnClusteringColorChanged()));
    connect(pushButtonAddClusteringRule,		SIGNAL(clicked()),		this, SLOT(OnAddClusteringRule()));
    connect(pushButtonEditClusteringRule,		SIGNAL(clicked()),		this, SLOT(OnEditClusteringRule()));
    connect(pushButtonDuplicateClusteringRule,	SIGNAL(clicked()),		this, SLOT(OnDuplicateClusteringRule()));
    connect(pushButtonUpClusteringRule,	SIGNAL(clicked()),		this, SLOT(OnMoveUpClusteringRule()));
    connect(pushButtonDownClusteringRule,	SIGNAL(clicked()),		this, SLOT(OnMoveDownClusteringRule()));
    connect(pushButtonRemoveClusteringRule,	SIGNAL(clicked()),		this, SLOT(OnRemoveClusteringRule()));
    connect(tableWidgetClusteringRules, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnEditClusteringRule()));

    // Masks
    connect(pushButtonAddMask,		SIGNAL(clicked()),		this, SLOT(OnAddMask()));
    connect(pushButtonEditMask,		SIGNAL(clicked()),		this, SLOT(OnEditMask()));
    connect(pushButtonDuplicateMask,SIGNAL(clicked()),		this, SLOT(OnDuplicateMask()));
    connect(pushButtonRemoveMask,	SIGNAL(clicked()),		this, SLOT(OnRemoveMask()));
    connect(tableWidgetMasks,		SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnEditMask()));
    // Sort Masks by ascending
    tableWidgetMasks->setSortingEnabled(true);
    tableWidgetMasks->sortByColumn(0);


    // Rules precedence / Ordering
    connect(buttonRuleOrderUp,					SIGNAL(clicked()),		this, SLOT(OnRulePrecedenceUp()));
    connect(buttonRuleOrderDown,				SIGNAL(clicked()),		this, SLOT(OnRulePrecedenceDown()));

//    connect(tabWidgetWaferOptions,				SIGNAL( currentChanged(int)),		this, SLOT(OnToggleReticle()));
    connect(comboBoxProcess,						SIGNAL(activated(QString)),		this, SLOT(OnComboProcessChange()));
    connect(comboBoxOutlierSeverityLimits,			SIGNAL(activated(QString)),		this, SLOT(OnComboSeverityLimits()));
    connect(sliderHead,							SIGNAL(sliderMoved(int)),		this, SLOT(OnHeadSlider()));
    connect(sliderTail,							SIGNAL(sliderMoved(int)),		this, SLOT(OnTailSlider()));
    connect(lineEditRuleFactorHead,				SIGNAL(textChanged(QString)),	this, SLOT(OnHeadFactor()));
    connect(lineEditRuleFactorTail,				SIGNAL(textChanged(QString)),	this, SLOT(OnTailFactor()));
    connect(comboBoxDistributionPat,				SIGNAL(activated(QString)),		this, SLOT(OnEnablePat()));
    connect(spinBoxMinimumOutliersPerPart,			SIGNAL(valueChanged(int)),		this, SLOT(OnMinimumOutliersToFailPart()));
    connect(m_poSaveDefaultPatConfig,              SIGNAL(clicked()),              this, SLOT(saveDefaultPatmanConfigFile()));
    connect(spinBoxFinalTest_Baseline,  SIGNAL(valueChanged(int)),
            this,   SLOT(OnBaseLineChanged(int)));
    connect(spinBoxFT_TuningSamples,  SIGNAL(valueChanged(int)),
            this,   SLOT(OnTuningSamplesChanged(int)));
    connect(checkBoxFT_TuningFrequency,  SIGNAL(toggled(bool)),
            this,   SLOT(OnTuningFrequencyToggled(bool)));
    connect(doubleSpinBoxMVPATNear,     SIGNAL(valueChanged(double)),
            this, SLOT(OnMVPATNearDistanceChanged(double)));
    connect(doubleSpinBoxMVPATMedium,   SIGNAL(valueChanged(double)),
            this, SLOT(OnMVPATMediumDistanceChanged(double)));
    connect(doubleSpinBoxMVPATFar,      SIGNAL(valueChanged(double)),
            this, SLOT(OnMVPATFarDistanceChanged(double)));

    // Selection mode for rules: single line
    tableWidgetNNR_Rules->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableWidgetNNR_Rules->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidgetIDDQ_Delta_Rules->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableWidgetIDDQ_Delta_Rules->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidgetGdbnRules->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableWidgetGdbnRules->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidgetClusteringRules->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableWidgetClusteringRules->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidgetReticleRules->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableWidgetReticleRules->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidgetMasks->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidgetMasks->setSelectionMode(QAbstractItemView::SingleSelection);

    // Prevent from sorting items.
    listViewConfigName->setSortingEnabled(false);

    // Set minimum size for the 1st column
    listViewConfigName->setColumnWidth (0,150);

    // Defaults
    bCreate = true;	// save fields when 'Ok' button is clicked (create mode)
    checkBoxConfigName_JobName->setChecked(true);	// Default config file name is '<job_name>__patman_config.csv'

    // Bin field can only receive integer numbers
    lineEditFailStatic->setValidator(new QIntValidator( -1, 65535, this));
    lineEditFailDynamic->setValidator(new QIntValidator( -1, 65535, this));
    lineEditRuleFactor->setValidator(new QDoubleValidator( 0, 999.0, 4, this));
    lineEditRuleFactor->setText("");
    lineEditMinimumSamples->setValidator(new QIntValidator( 1, 65535, this));

    // Load Rule set list, set it to 'Smart & Adaptive)
    FillComboBox(comboBoxRuleSet, gexRuleSetItemsLongNamesGUI);
    comboBoxRuleSet->setCurrentIndex(GEX_TPAT_RULETYPE_SMARTID);

    // Fill Yield threshold lists
    int i;
    QString strString;
    comboFinalTest_YieldLevel->clear();
    comboFinalTest_CriticalBinsYield->clear();
    for(i=0;i<=100;i++)
    {
        strString = QString::number(i) + "%";

        // FINAL TEST: Yield level (good bin)
        comboFinalTest_YieldLevel->addItem(strString);

        // FINAL TEST: Critical bins yield loss
        comboFinalTest_CriticalBinsYield->addItem(strString);
    }
    // Select Yield threshold = 50%
    doubleSpinBoxCompositeYieldLevel->setValue(50.00);

    // Smart Rules tab
    comboBoxDistribution->clear();
    FillComboBox(comboBoxDistribution, gexSmartRuleSetItemsLongNamesGUI);	// List of distributions available for customization

    comboBoxAlgorithm->clear();
    FillComboBox(comboBoxAlgorithm, gexSmartAlgorithmLongNamesGUI);			// List of algorithms available for customization

    sliderHead->setRange(-1500,-50);	// Head H*Sigma space: -15.00 Sigma, -.50 Sigma
    sliderTail->setRange(50,1500);		// Tail T*Sigma space: 0.500 Sigma, +15.00 Sigma
    sliderHead->setValue(-600);			// Startup: -6*Sigma
    sliderTail->setValue(600);			// Startup: +6*Sigma
    lineEditRuleFactorHead->setValidator(new QDoubleValidator( -15.0, -0.5, 2, this));
    lineEditRuleFactorTail->setValidator(new QDoubleValidator( 0.5, 15.0, 2, this));
    sliderHead->setTickInterval (100);	// 1 tick mark every sigma
    sliderTail->setTickInterval (100);	// 1 tick mark every sigma

    // Clustering

    // Composite Wafer-Lot

    // Final test: Alarm timeout options
    // Clear combos
    comboFinalTest_Alarm_Outliers->clear();
    comboFinalTest_Alarm_Shapes->clear();
    comboFinalTest_Alarm_PatDrift->clear();
    comboFinalTest_Alarm_LowGoodYield->clear();
    comboFinalTest_Alarm_YieldLoss->clear();
    comboFinalTest_Alarm_ParamDrift->clear();
    comboFinalTest_Alarm_ParamCpk->clear();
    // Fill combos
    // 7103
    FillComboBox(comboFinalTest_Alarm_Outliers, gexFT_Alarm_Severities); // gexFT_Alarm_Timeout
    FillComboBox(comboFinalTest_Alarm_Shapes, gexFT_Alarm_Severities );
    FillComboBox(comboFinalTest_Alarm_PatDrift, gexFT_Alarm_Severities );
    FillComboBox(comboFinalTest_Alarm_LowGoodYield, gexFT_Alarm_Severities );
    FillComboBox(comboFinalTest_Alarm_YieldLoss, gexFT_Alarm_Severities );
    FillComboBox(comboFinalTest_Alarm_ParamDrift, gexFT_Alarm_Severities );
    FillComboBox(comboFinalTest_Alarm_ParamCpk, gexFT_Alarm_Severities );
    // Set default to 'Continue Production'
    comboFinalTest_Alarm_Outliers->setCurrentIndex(1);
    comboFinalTest_Alarm_Shapes->setCurrentIndex(1);
    comboFinalTest_Alarm_PatDrift->setCurrentIndex(1);
    comboFinalTest_Alarm_LowGoodYield->setCurrentIndex(1);
    comboFinalTest_Alarm_YieldLoss->setCurrentIndex(1);
    comboFinalTest_Alarm_ParamDrift->setCurrentIndex(1);
    comboFinalTest_Alarm_ParamCpk->setCurrentIndex(1);

    // Hide bunch of alarm
    comboFinalTest_Alarm_Shapes->setVisible(false);
    comboFinalTest_Alarm_PatDrift->setVisible(false);
    comboFinalTest_Alarm_LowGoodYield->setVisible(false);
    comboFinalTest_Alarm_YieldLoss->setVisible(false);
    comboFinalTest_Alarm_ParamDrift->setVisible(false);
    comboFinalTest_Alarm_ParamCpk->setVisible(false);

    textLabelFinalTest_Alarm_Shapes->setVisible(false);
    textLabelFinalTest_Alarm_PatDrift->setVisible(false);
    textLabelFinalTest_Alarm_LowGoodYield->setVisible(false);
    textLabelFinalTest_Alarm_YieldLoss->setVisible(false);
    textLabelFinalTest_Alarm_ParamDrift->setVisible(false);
    textLabelFinalTest_Alarm_ParamCpk->setVisible(false);

    // Minimum/Maximum for baseline
    spinBoxFinalTest_Baseline->setRange(FT_PAT_BASELINE_MIN_SIZE, FT_PAT_BASELINE_MAX_SIZE);
    // Minimum/Maximum for tuning buffer
    spinBoxFT_TuningSamples->setRange(FT_PAT_TUNING_MIN_SIZE, FT_PAT_TUNING_MAX_SIZE);

    // Load custom PAT plugins, and update GUI with list of custom PAT plugins available
    m_pExternalPat = new GexExternalPat();
    m_pExternalPat->LoadPlugins(m_pPluginList);
    m_pCurrentPlugin = NULL;
    FillCustomPatLibList();
    tabWidgetWaferOptions->setCurrentIndex(0);
    tabWidget->setCurrentIndex(0);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatLimitsDialog::~PatLimitsDialog()
{
    // Delete ressources
    m_pExternalPat->UnloadPlugins(m_pPluginList);
    delete m_pExternalPat;
    m_pExternalPat=0;
}

///////////////////////////////////////////////////////////
// Update GUI Enabled/Disabled fileds.
///////////////////////////////////////////////////////////
void	PatLimitsDialog::setEditMode(bool bCreateMode/*=true*/,bool bShowFileHeaderDetails/*=true*/, bool bWizardGui/*=false*/)
{
    if(!mOptionsPat)
        return;
    // 'true' means the dialog box will update the config file on exit ('Ok' button pressed)
    // 'false' means we edit an existing config and therefore some tabs must be disabled (the one with global fields written when creating config file from scratch)
    bCreate = bCreateMode;

    // Disable / Hide some fields that can only be set when creating the original config file (eg: global Pat bins, etc...)
    comboBoxRuleSet->setEnabled(bCreateMode);
    lineEditRuleFactor->setEnabled(bCreateMode);

    if(bShowFileHeaderDetails == false)
    {
        listViewConfigName->setEnabled(false);
    }
    else
    {
        listViewConfigName->setEnabled(true);
    }

    // Wizard title, or regular Dialog title?
    if(bWizardGui)
    {
        // Wizard display mode: change title & buttons accordingly
        comboBoxRuleSet->setEnabled(true);
        lineEditRuleFactor->setEnabled(true);

        setWindowTitle("Recipe Wizard: Global settings (Page 1)");
        pushButtonCancel->setText("Cancel");
        pushButtonProcessFile->setText("Next >>");
    }
    else
    {
        // Standard dialog box.
        setWindowTitle("Edit Recipe file");
        pushButtonCancel->setText("Cancel");
        pushButtonProcessFile->setText("OK");
    }
    // Make sure the GUI is reflecting the current configuration file edited.
    RefreshGUI();
}
///////////////////////////////////////////////////////////
// Checks if any Custom PAT libs available in plugin folder + update GUI accordingly
///////////////////////////////////////////////////////////
void	PatLimitsDialog::FillCustomPatLibList(void)
{
    // Fill combo with list of PAT plugins
    comboBoxCustomPatLib->clear();

    QList<GexPatPlugin_ID*>::iterator itPlugin = m_pPluginList.begin();

    while(itPlugin != m_pPluginList.end())
    {
        comboBoxCustomPatLib->addItem((*itPlugin)->m_strPluginName);

        // Check if the new entry corresponds to the one read from the recipe file
        if((*itPlugin)->m_strFileName == mOptionsPat->strCustomPatLibFile)
            m_pCurrentPlugin = (*itPlugin);

        itPlugin++;
    }

    // If no plugin found, add dummy item, and update GUI
    if(m_pPluginList.count() == 0)
    {
        QString strDummyItem = "<No GexPat plugin found in directory ";
        strDummyItem += m_pExternalPat->GetPluginDirectory();
        strDummyItem += ">";
        comboBoxCustomPatLib->addItem(strDummyItem);
        groupBoxCustomAlgorithm->setChecked(false);
        groupBoxCustomAlgorithm->setEnabled(false);
        checkBoxBadNeighbourhood_Custom->setChecked(false);
        checkBoxBadNeighbourhood_Custom->setEnabled(false);
    }
    else
    {
        groupBoxCustomAlgorithm->setEnabled(true);
        checkBoxBadNeighbourhood_Custom->setEnabled(true);

        // Select combo box item
        if(!m_pCurrentPlugin)
        {
            m_pCurrentPlugin = (m_pPluginList.isEmpty()) ? NULL :m_pPluginList.first();
        }
        if (m_pCurrentPlugin != NULL)
            comboBoxCustomPatLib->setCurrentIndex(getComboBoxTextItem(comboBoxCustomPatLib,
                                                                      m_pCurrentPlugin->m_strPluginName));
    }

    // Update GUI texts
    OnPatPluginChanged();
}


void PatLimitsDialog::RefreshMaskRules()
{
    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Masks' tab
    ///////////////////////////////////////////////////////////////////////
    // Mask Rules
    if(mOptionsPat->mMaskRules.count())
        qDeleteAll(mOptionsPat->mMaskRules);
    mOptionsPat->mMaskRules.clear();
    for(int iRuleID=0; iRuleID<tableWidgetMasks->rowCount();++iRuleID)
    {
        CMask_Rule *ptMaskRule = new CMask_Rule;
        mOptionsPat->mMaskRules.append(ptMaskRule);
        MaskRule_ReadGUI(iRuleID,*ptMaskRule);
    }
}

///////////////////////////////////////////////////////////
// Updates the GUI fileds based on the variables values.
///////////////////////////////////////////////////////////
void	PatLimitsDialog::CopyUIData(void)
{
    GSLOG(6, "GexTbPatLimitsDialog Refresh GUI");

    mOptionsPat->SetDefaultFactor(lineEditRuleFactor->text().toDouble());
    mOptionsPat->bStaticPAT  = groupBoxStatic->isChecked();
    mOptionsPat->bDynamicPAT = groupBoxDynamic->isChecked();
    mOptionsPat->SetMVPATEnabled(groupBoxMVPAT->isChecked());

    // Load GUI fields: 'Outlier rules' tab
    mOptionsPat->iFailStatic_SBin = lineEditFailStatic->text().toInt();
    mOptionsPat->iFailStatic_HBin = lineEditFailStatic->text().toInt();
    mOptionsPat->cStaticFailColor = pushButtonEditStaticColor->activeColor();

    mOptionsPat->iFailDynamic_SBin = lineEditFailDynamic->text().toInt();
    mOptionsPat->iFailDynamic_HBin = lineEditFailDynamic->text().toInt();
    mOptionsPat->cDynamicFailColor  = pushButtonEditDynamicColor->activeColor();

    mOptionsPat->SetMVPATSoftBin(lineEditMVPATBin->text().toInt());
    mOptionsPat->SetMVPATHardBin(lineEditMVPATBin->text().toInt());
    mOptionsPat->SetMVPATColor(pushButtonMVPATBinColor->activeColor());
    mOptionsPat->SetMVPATAutomaticGroupCreation(checkBoxMVPATAutoGroupCreation->isChecked());

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Options' tab
    ///////////////////////////////////////////////////////////////////////

    // Good Soft bin
    mOptionsPat->pGoodSoftBinsList->SetRange(lineEditGoodSoftBin->text());

    // Good Hard bin
    mOptionsPat->pGoodHardBinsList->SetRange(lineEditGoodHardBin->text());

    // Test key (how is a test identified: based on test#, test name, etc...)
    mOptionsPat->mOptionsTestKey  = comboBoxTestKey->currentIndex();
    mOptionsPat->SetAllSitesMerged(checkBoxAllSitesMerged->isChecked());

    // Bins to use to compute PAT limits
    mOptionsPat->iPatLimitsFromBin      = comboBoxProcess->itemData(comboBoxProcess->currentIndex()).toInt();
    mOptionsPat->strPatLimitsFromBin    = editProcess->text();

    mOptionsPat->bIgnoreIQR0 = checkBoxIgnoreNulIQR->isChecked();

    bool ok;

    if (checkBoxHistoricalCpk->isChecked())
        mOptionsPat->lfIgnoreHistoricalCpk = lineEditHistoricalCpk->text().toDouble(&ok);
    else
        mOptionsPat->lfIgnoreHistoricalCpk = -1;
    mOptionsPat->bIgnoreHistoricalCategories = checkBoxHistoricalCategory->isChecked();

    if(comboBoxScanGoodParts->currentIndex() == 0)
        mOptionsPat->bScanGoodPartsOnly = true;	// Good bins only
    else
        mOptionsPat->bScanGoodPartsOnly = false;	// All parts

    mOptionsPat->lfSmart_IgnoreHighCpk = lineEditCpk->text().toInt();
    mOptionsPat->mSmart_IgnoreHighCpkEnabled = checkBoxIgnoreCpk->isChecked();

    mOptionsPat->iMinimumSamples = lineEditMinimumSamples->text().toInt();
    mOptionsPat->iMinimumOutliersPerPart = spinBoxMinimumOutliersPerPart->value();

    // Stop on first PAt failure in flow?
    mOptionsPat->bStopOnFirstFail = checkBoxStopOnFirstFail->isChecked();

    // Do not allow PAT limits to be outside of spec limits.
    mOptionsPat->bStickWithinSpecLimits = checkBoxStickWithinSpecLimits->isChecked();

    // Category distribution definition
    mOptionsPat->bAssumeIntegerCategory = checkBoxAssumeIntegerCategory->isChecked();
    mOptionsPat->iCategoryValueCount = spinBoxCategoryValueCount->value();

    // Allow Custom PAT algorithms (user DLL)
    mOptionsPat->bCustomPatLib = groupBoxCustomAlgorithm->isChecked();

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Static-PAT' tab
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->bIgnoreHistoricalCategories = checkBoxHistoricalCategory->isChecked();
    mOptionsPat->bIgnoreIQR0 = checkBoxIgnoreNulIQR->isChecked();

    if (checkBoxHistoricalCpk->isChecked())
        mOptionsPat->lfIgnoreHistoricalCpk = lineEditHistoricalCpk->text().toInt();
    else
        mOptionsPat->lfIgnoreHistoricalCpk = -1;

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Multi-variate' tab
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->SetMVPATGroupCorrelation(doubleSpinBoxMVPATCorrThreshold->value());
    mOptionsPat->SetMVPATNormalShapeOnly(checkBoxMVPATNormalDistribution->isChecked());
    mOptionsPat->SetMVPATDistance(PAT::Near, doubleSpinBoxMVPATNear->value());
    mOptionsPat->SetMVPATDistance(PAT::Medium, doubleSpinBoxMVPATMedium->value());
    mOptionsPat->SetMVPATDistance(PAT::Far, doubleSpinBoxMVPATFar->value());
    mOptionsPat->SetMVPATIgnorePPATBins(checkBoxMVPATIgnoresPPATBins->isChecked());

    ///////////////////////////////////////////
    // Versioning tab
    ///////////////////////////////////////////
    mOptionsPat->strProductName = lineEditProduct->text();

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'WAFERT SORT' tab
    ///////////////////////////////////////////////////////////////////////
    // Fill Sort flows table
    mOptionsPat->strFlowNames.clear();
    mOptionsPat->strFlowComments.clear();
    mOptionsPat->strFlowKillSTDF_SBIN.clear();
    mOptionsPat->strFlowKillSTDF_HBIN.clear();
    mOptionsPat->strFlowKillMAP_BIN.clear();
    for(int iFlowID=0;iFlowID < tableWidgetFlowNames->rowCount(); iFlowID++)
    {
        // Flow name
        mOptionsPat->strFlowNames.append(tableWidgetFlowNames->item(iFlowID,0)->text());
        // Flow comment
        mOptionsPat->strFlowComments.append(tableWidgetFlowNames->item(iFlowID,1)->text());

        ///! to be verified
        // Kill STDF SBINs (SBIN dies to remove from map)
        //mOptionsPat->strFlowKillSTDF_SBIN.append(tableWidgetFlowNames->item(iFlowID,2)->text());
        // Kill STDF HBINs (SBIN dies to remove from map)
//        mOptionsPat->strFlowKillSTDF_HBIN.append(tableWidgetFlowNames->item(iFlowID,3)->text());
//        // Kill MAP BINs (BIN dies to remove from map)
//        mOptionsPat->strFlowKillMAP_BIN.append(tableWidgetFlowNames->item(iFlowID,4)->text());
    }


    // Load GUI sub-tab 'Precedence'
    mOptionsPat->strRulePrecedence.clear();
    for(int lIdx = 0; lIdx<listWidgetRulesOrder->count(); ++lIdx)
    {
        mOptionsPat->strRulePrecedence.append(listWidgetRulesOrder->item(lIdx)->text());
    }

    ///////////////////////////////////////////////////////////////////////
    // Load GUI sub-tab: 'NNR'
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->SetNNREnabled(groupBoxNNR->isChecked());
    mOptionsPat->SetNNRHardBin(lineEditNNR_Bin->text().toInt());
    mOptionsPat->SetNNRSoftBin(lineEditNNR_Bin->text().toInt());
    mOptionsPat->SetNNRColor(pushButtonNNR_Color->activeColor());

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'IDDQ-Delta' tab
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->mIsIDDQ_Delta_enabled = groupBoxIDDQ_Delta->isChecked();
    mOptionsPat->mIDDQ_Delta_SBin = lineEditIDDQ_Delta_Bin->text().toInt();
    mOptionsPat->mIDDQ_Delta_HBin = lineEditIDDQ_Delta_Bin->text().toInt();
    mOptionsPat->mIDDQ_Delta_Color = pushButtonIDDQ_Delta_Color->activeColor();


    // GUI: GPAT must include PPAT bins?
    mOptionsPat->mGPAT_IgnorePPatBins = checkBoxGPAT_IgnorePPATBins->isChecked();

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'GDBN' tab
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->mGDBNPatSBin = lineEditBadClusterBin->text().toInt();
    mOptionsPat->mGDBNPatHBin = lineEditBadClusterBin->text().toInt();
    mOptionsPat->mGDBNColor = pushButtonBadClusterColor->activeColor();
    mOptionsPat->mIsGDBNEnabled = groupBoxBadNeighbourhood->isChecked();


    // Custom PAT lib enabled?
    mOptionsPat->mGDBNCustomLib = checkBoxBadNeighbourhood_Custom->isChecked();

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Reticle' tab
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->SetReticleEnabled(groupBoxReticle->isChecked());
    mOptionsPat->SetReticleHardBin(lineEditBadReticule->text().toInt());
    mOptionsPat->SetReticleSoftBin(lineEditBadReticule->text().toInt());
    mOptionsPat->SetReticleColor(pushButtonBadReticule->activeColor());
    mOptionsPat->SetReticleSizeSource((PATOptionReticle::ReticleSizeSource)comboBoxReticleSource->currentData().toInt());
    mOptionsPat->SetReticleSizeX(spinBoxReticleX->value());
    mOptionsPat->SetReticleSizeY(spinBoxReticleY->value());

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'ZPAT: Composite / Wafer lot' tab
    ///////////////////////////////////////////////////////////////////////
    if(comboBox_ZPAT_BinType->currentIndex() == 0)
        mOptionsPat->bZPAT_SoftBin = true;
    else
        mOptionsPat->bZPAT_SoftBin = false;

    mOptionsPat->pBadBinsZPAT_List->SetRange(lineEditBadBinListZPAT->text());
    mOptionsPat->bZPAT_GDBN_Enabled = checkBoxZPAT_GDBN->isChecked();
    mOptionsPat->bZPAT_Reticle_Enabled = checkBoxZPAT_Reticle->isChecked();
    mOptionsPat->bZPAT_Clustering_Enabled = checkBoxZPAT_Clustering->isChecked();
    mOptionsPat->lfCompositeExclusionZoneYieldThreshold = doubleSpinBoxCompositeYieldLevel->value();
    mOptionsPat->SetExclusionZoneEnabled(groupBoxCompositeExclusionZone->isChecked());
    mOptionsPat->iCompositeZone_SBin = lineEditExclusionZone->text().toInt();
    mOptionsPat->iCompositeZone_HBin = lineEditExclusionZone->text().toInt();
    mOptionsPat->bMergeEtestStdf = groupBoxMergeEtestStdf->isChecked();
    mOptionsPat->iCompositeEtestStdf_SBin = lineEditMergeEtestStdf->text().toInt();
    mOptionsPat->iCompositeEtestStdf_HBin = lineEditMergeEtestStdf->text().toInt();
    // Button color
    mOptionsPat->cZpatColor = pushButtonZpatColor->activeColor();


    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Clustering' tab (outline bad clusters)
    ///////////////////////////////////////////////////////////////////////
    mOptionsPat->mClusteringPotato = groupBoxClusteringPotato->isChecked();
    // Clustering Bin#
    mOptionsPat->mClusteringPotatoSBin = lineEditClusteringPotatoBin->text().toInt();
    mOptionsPat->mClusteringPotatoHBin = lineEditClusteringPotatoBin->text().toInt();
    // Button color
    mOptionsPat->mClusteringPotatoColor = pushButtonClusteringPotatoColor->activeColor();


    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Masks' tab
    ///////////////////////////////////////////////////////////////////////
    // Mask Rules
    RefreshMaskRules();

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'FINAL TEST' tab
    ///////////////////////////////////////////////////////////////////////

    // Total parts in base line
    mOptionsPat->mFT_BaseLine = spinBoxFinalTest_Baseline->value();

    // Alarm level on outliers detected in baseline.
    mOptionsPat->mFT_BaseLineMaxOutliers = spinBoxFinalTest_BaseLineMaxOutliers->value();

    // Tuning samples
    mOptionsPat->mFT_TuningSamples = spinBoxFT_TuningSamples->value();

    // Dynamic PAT limits tuning frequency (0 = Never tune)
    mOptionsPat->mFT_TuningIsEnabled = checkBoxFT_TuningFrequency->isChecked();
    mOptionsPat->mFT_Tuning = spinBoxFinalTest_Tuning->value();
    mOptionsPat->mFT_TuningType = comboFinalTest_TuningType->currentIndex();
    // Maximum outliers allowed in a lot
    mOptionsPat->mFT_MaxOutlierParts = spinBoxFinalTest_MaxOutlierParts->value();
    // Minimum good yield expected
    mOptionsPat->mFT_YieldLevel = comboFinalTest_YieldLevel->currentIndex();
    // Maximum yield loss on critical Bins
    mOptionsPat->mFT_CriticalBinsYield = comboFinalTest_CriticalBinsYield->currentIndex();
    // List of critical bins
    if(mOptionsPat->mFT_CriticalBinsList != NULL)
    {
        mOptionsPat->mFT_CriticalBinsList->SetRange(lineEditFinalTest_CriticalBins->text());
    }

    // Final test: Alarm timeouts
    // 7103 : moved some members into propery map

    mOptionsPat->setProperty(ALARM_SEVERITY_TOO_MUCH_OUTLIERS_IN_BASELINE, comboFinalTest_Alarm_Outliers->currentIndex());
    mOptionsPat->setProperty(ALARM_SEVERITY_DISTRIB_SHAPE_CHANGED,comboFinalTest_Alarm_Shapes->currentIndex());
    mOptionsPat->setProperty(ALARM_SEVERITY_PAT_DRIFT, comboFinalTest_Alarm_PatDrift->currentIndex());
    mOptionsPat->setProperty(ALARM_SEVERITY_LOW_GOOD_YIELD,comboFinalTest_Alarm_LowGoodYield->currentIndex());
    mOptionsPat->setProperty(ALARM_SEVERITY_YIELD_LOSS,comboFinalTest_Alarm_YieldLoss->currentIndex());
    mOptionsPat->setProperty(ALARM_SEVERITY_PARAM_DRIFT,comboFinalTest_Alarm_ParamDrift->currentIndex());
    mOptionsPat->setProperty(ALARM_SEVERITY_PARAM_CPK,comboFinalTest_Alarm_ParamCpk->currentIndex());

    // Final test: Alarm emails
//        checkBoxFinalTest_Alarm_Outliers_Email->setChecked(mOptionsPat->iFT_Alarm_Email_Outliers);
//        checkBoxFinalTest_Alarm_Shapes_Email->setChecked(mOptionsPat->iFT_Alarm_Email_Shapes);
//        checkBoxFinalTest_Alarm_PatDrift_Email->setChecked(mOptionsPat->iFT_Alarm_Email_PatDrift);
//        checkBoxFinalTest_Alarm_LowGoodYield_Email->setChecked(mOptionsPat->iFT_Alarm_Email_LowGoodYield);
//        checkBoxFinalTest_Alarm_YieldLoss_Email->setChecked(mOptionsPat->iFT_Alarm_Email_YieldLoss);
//        checkBoxFinalTest_Alarm_ParamDrift_Email->setChecked(mOptionsPat->iFT_Alarm_Email_ParamDrift);
//        checkBoxFinalTest_Alarm_ParamCpk_Email->setChecked(mOptionsPat->iFT_Alarm_Email_ParamCpk);

    // Final test: Miscelaneous
    mOptionsPat->mFT_RunsPerPacket = spinBoxFinalTest_RunsPerPacket->value();

    mOptionsPat->mFT_Traceability = comboFinalTest_Traceability->itemData(comboFinalTest_Traceability->currentIndex()).toInt();

    // Report options
    mOptionsPat->bReport_Stats = checkBoxReport_Stats->isChecked();
    mOptionsPat->bReport_Histo = checkBoxReport_Histo->isChecked();
    // PAT-134
    mOptionsPat->bReport_Histo_NoOutliers = checkBoxReport_HistoForNotFailingPATtests->isChecked();
    mOptionsPat->bReport_Wafermap = checkBoxReport_Wafermap->isChecked();
    mOptionsPat->iReport_WafermapType = comboBoxReport_WafermapType->currentIndex(); // SBIN or HBIN
    mOptionsPat->bReport_Pareto = checkBoxReport_Pareto->isChecked();
    mOptionsPat->bReport_Binning = checkBoxReport_Binning->isChecked();
    mOptionsPat->bReport_SPAT_Limits = checkBoxReport_SPAT_Limits->isChecked();
    mOptionsPat->bReport_DPAT_Limits_Outliers = checkBoxReport_DPAT_Limits_Outliers->isChecked();
    mOptionsPat->bReport_DPAT_Limits_NoOutliers = checkBoxReport_DPAT_Limits_NotOutlier->isChecked();
    mOptionsPat->SetMVPATReportStdCharts(checkBoxReport_MVPATStdCharts->isChecked());
    mOptionsPat->SetMVPATReportCorrCharts(groupBoxReport_MVPATCorrCharts->isChecked());
    mOptionsPat->SetMVPATReportPCAProjection(checkBoxReport_MVPATCorrProjection->isChecked());
    mOptionsPat->SetMVPATReportPairs(checkBoxReportMVPATCorrPairs->isChecked()? GS::Gex::PAT::AllPairs : GS::Gex::PAT::ConsecutivePairs);
    mOptionsPat->SetMVPATReportMaxCharts(spinBoxReport_MVPATCorrMaxCharts->value());

    // Wafer options
    mOptionsPat->SetEnableYALimit(groupBoxReport_PatYieldAlarm->isChecked());
    mOptionsPat->SetOveralPatYALimit(lineEditOveralPatYALimit->text().toDouble());
}

void	PatLimitsDialog::RefreshGUI()
{
    GSLOG(6, "GexTbPatLimitsDialog Refresh GUI");

    QString strString;
    // Display Recipe Version#
    int	iRecipeVersion = qMax(1,mOptionsPat->iRecipeVersion);	// Ensure version# starts at 1!
    strString = "Recipe Version: " + QString::number(iRecipeVersion);
    int	iRecipeBuild = qMax(0,mOptionsPat->iRecipeBuild);		// Ensure Build# starts at 0!
    strString += "." + QString::number(iRecipeBuild);
    textLabelRecipeVersion->setText(strString);
    lineEditRecipeRev->setText(strString);	// Versionning tab

    // Load GUI fields: 'Outlier rules' tab
    lineEditFailStatic->setText(QString::number(mOptionsPat->iFailStatic_SBin));
    pushButtonEditStaticColor->setActiveColor(mOptionsPat->cStaticFailColor);

    lineEditFailDynamic->setText(QString::number(mOptionsPat->iFailDynamic_SBin));
    pushButtonEditDynamicColor->setActiveColor(mOptionsPat->cDynamicFailColor);

    lineEditMVPATBin->setText(QString::number(mOptionsPat->GetMVPATSoftBin()));
    pushButtonMVPATBinColor->setActiveColor(mOptionsPat->GetMVPATColor());
    checkBoxMVPATAutoGroupCreation->setChecked(mOptionsPat->GetMVPATAutomaticGroupCreation());

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Options' tab
    ///////////////////////////////////////////////////////////////////////
    groupBoxStatic->setChecked(mOptionsPat->bStaticPAT);
    groupBoxDynamic->setChecked(mOptionsPat->bDynamicPAT);
    groupBoxMVPAT->setChecked(mOptionsPat->GetMVPATEnabled());

    // Good Soft bin
    lineEditGoodSoftBin->setText(mOptionsPat->pGoodSoftBinsList->GetRangeList());

    // Good Hard bin
    lineEditGoodHardBin->setText(mOptionsPat->pGoodHardBinsList->GetRangeList());

    // Test key (how is a test identified: based on test#, test name, etc...)
    comboBoxTestKey->setCurrentIndex(mOptionsPat->mOptionsTestKey);

    // Sites merged (only for Wafer Sort level)
    checkBoxAllSitesMerged->setChecked(mOptionsPat->GetAllSitesMerged());

    // Bins to use to compute PAT limits
    comboBoxProcess->setCurrentIndex(comboBoxProcess->findData(mOptionsPat->iPatLimitsFromBin));
    editProcess->setText(mOptionsPat->strPatLimitsFromBin);

    checkBoxIgnoreNulIQR->setChecked(mOptionsPat->bIgnoreIQR0);
    checkBoxHistoricalCategory->setChecked(mOptionsPat->bIgnoreHistoricalCategories);
    checkBoxHistoricalCpk->setChecked(mOptionsPat->lfIgnoreHistoricalCpk > 0);
    double lIgnoredCpk = (mOptionsPat->lfIgnoreHistoricalCpk > 0) ?  mOptionsPat->lfIgnoreHistoricalCpk : 20;
    lineEditHistoricalCpk->setText(QString::number(lIgnoredCpk));
    if(mOptionsPat->bScanGoodPartsOnly)
        comboBoxScanGoodParts->setCurrentIndex(0);	// Good bins only
    else
        comboBoxScanGoodParts->setCurrentIndex(1);	// All parts

    // Test key can be changed only at recipe creation
    comboBoxTestKey->setEnabled(false);

    if (mOptionsPat->GetRecipeType() != GS::Gex::PAT::RecipeFinalTest)
    {
        // Show/Enabled wafer sort field
        comboBoxScanGoodParts->setEnabled(true);
        comboBoxProcess->setEnabled(true);

        comboBoxOutlierSeverityLimits->setEnabled(true);

        checkBoxStopOnFirstFail->setVisible(true);

        textLabelMinimumSamples->setVisible(true);
        lineEditMinimumSamples->setVisible(true);

        textLabelMinimumOutliersPerPart->setVisible(true);
        spinBoxMinimumOutliersPerPart->setVisible(true);

        tabWidget6->setTabEnabled(tabWidget6->indexOf(tabOptionsWafer), true);

        checkBoxAllSitesMerged->setEnabled(true);
    }
    else
    {
        // Hide/Disable wafer sort field
        comboBoxProcess->setEnabled(false);
        comboBoxTestKey->setEnabled(false);

        comboBoxOutlierSeverityLimits->setEnabled(false);

        checkBoxStopOnFirstFail->setVisible(false);

        textLabelMinimumSamples->setVisible(false);
        lineEditMinimumSamples->setVisible(false);

        textLabelMinimumOutliersPerPart->setVisible(false);
        spinBoxMinimumOutliersPerPart->setVisible(false);

        tabWidget6->setTabEnabled(tabWidget6->indexOf(tabOptionsWafer), false);

        checkBoxAllSitesMerged->setEnabled(false);
    }

    lineEditCpk->setText(QString::number(mOptionsPat->lfSmart_IgnoreHighCpk));
    checkBoxIgnoreCpk->setChecked(mOptionsPat->mSmart_IgnoreHighCpkEnabled);

    lineEditMinimumSamples->setText(QString::number(mOptionsPat->iMinimumSamples));
    spinBoxMinimumOutliersPerPart->setValue(mOptionsPat->iMinimumOutliersPerPart);

    // Stop on first PAt failure in flow?
    if(mOptionsPat->bStopOnFirstFail)
        checkBoxStopOnFirstFail->setChecked(true);
    else
        checkBoxStopOnFirstFail->setChecked(false);

    // Do not allow PAT limits to be outside of spec limits.
    if(mOptionsPat->bStickWithinSpecLimits)
        checkBoxStickWithinSpecLimits->setChecked(true);
    else
        checkBoxStickWithinSpecLimits->setChecked(false);

    // Category distribution definition
    if(mOptionsPat->bAssumeIntegerCategory)
        checkBoxAssumeIntegerCategory->setChecked(true);
    else
        checkBoxAssumeIntegerCategory->setChecked(false);
    spinBoxCategoryValueCount->setValue(mOptionsPat->iCategoryValueCount);

    // Allow Custom PAT algorithms (user DLL)
    if(m_pPluginList.count() > 0)
        groupBoxCustomAlgorithm->setChecked(mOptionsPat->bCustomPatLib);

    ///////////////////////////////////////////////////////////////////////
    // 'Static-PAT' tab
    ///////////////////////////////////////////////////////////////////////
    checkBoxIgnoreNulIQR->setChecked(mOptionsPat->bIgnoreIQR0);
    checkBoxHistoricalCategory->setChecked(mOptionsPat->bIgnoreHistoricalCategories);

    if (mOptionsPat->lfIgnoreHistoricalCpk > -1)
    {
        checkBoxHistoricalCpk->setChecked(true);
        lineEditHistoricalCpk->setText(QString::number(mOptionsPat->lfIgnoreHistoricalCpk));
    }
    else
        checkBoxHistoricalCpk->setChecked(false);

    ///////////////////////////////////////////////////////////////////////
    // 'Multi-variate' tab
    ///////////////////////////////////////////////////////////////////////
    doubleSpinBoxMVPATCorrThreshold->setValue(mOptionsPat->GetMVPATGroupCorrelation());

    checkBoxMVPATNormalDistribution->setChecked(mOptionsPat->GetMVPATNormalShapeOnly());
    doubleSpinBoxMVPATNear->setValue(mOptionsPat->GetMVPATDistance(PAT::Near));
    doubleSpinBoxMVPATMedium->setValue(mOptionsPat->GetMVPATDistance(PAT::Medium));
    doubleSpinBoxMVPATFar->setValue(mOptionsPat->GetMVPATDistance(PAT::Far));
    checkBoxMVPATIgnoresPPATBins->setChecked(mOptionsPat->GetMVPATIgnorePPATBins());

    ///////////////////////////////////////////
    // Versioning tab
    ///////////////////////////////////////////
    lineEditProduct->setText(mOptionsPat->strProductName.trimmed());

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'WAFERT SORT' tab
    ///////////////////////////////////////////////////////////////////////

    if (mOptionsPat->GetRecipeType() != GS::Gex::PAT::RecipeFinalTest)
        tabWidget->setTabEnabled(tabWidget->indexOf(TabWaferSort), true);
    else
        tabWidget->setTabEnabled(tabWidget->indexOf(TabWaferSort), false);

    // Fill Sort flows table
    int	iFlowID;
    QString strItem;
    for(iFlowID=0;iFlowID < mOptionsPat->strFlowNames.count(); iFlowID++)
    {
        // Flow name
        tableWidgetFlowNames->setItem(iFlowID,0,new QTableWidgetItem(mOptionsPat->strFlowNames[iFlowID]));
        // Flow comment
        tableWidgetFlowNames->setItem(iFlowID,1,new QTableWidgetItem(mOptionsPat->strFlowComments[iFlowID]));
        // Kill STDF SBINs (SBIN dies to remove from map)
        strItem = (mOptionsPat->strFlowKillSTDF_SBIN.count() > iFlowID) ? (mOptionsPat->strFlowKillSTDF_SBIN[iFlowID]) : "";
        tableWidgetFlowNames->setItem(iFlowID,2,new QTableWidgetItem(strItem));
        // Kill STDF HBINs (SBIN dies to remove from map)
        strItem = (mOptionsPat->strFlowKillSTDF_HBIN.count() > iFlowID) ? (mOptionsPat->strFlowKillSTDF_HBIN[iFlowID]) : "";
        tableWidgetFlowNames->setItem(iFlowID,3,new QTableWidgetItem(strItem));
        // Kill MAP BINs (BIN dies to remove from map)
        strItem = (mOptionsPat->strFlowKillMAP_BIN.count() > iFlowID) ? (mOptionsPat->strFlowKillMAP_BIN[iFlowID]) : "";
        tableWidgetFlowNames->setItem(iFlowID,4,new QTableWidgetItem(strItem));
    }
    OnSetTotalFlows();

    // Load GUI sub-tab 'Precedence'
    listWidgetRulesOrder->clear();
    listWidgetRulesOrder->insertItems(0,mOptionsPat->strRulePrecedence);

    ///////////////////////////////////////////////////////////////////////
    // Load GUI sub-tab: 'NNR'
    ///////////////////////////////////////////////////////////////////////
    groupBoxNNR->setChecked(mOptionsPat->IsNNREnabled());
    lineEditNNR_Bin->setText(QString::number(mOptionsPat->GetNNRSoftBin()));
    pushButtonNNR_Color->setActiveColor(mOptionsPat->GetNNRColor());

    // Rules
    QList <CNNR_Rule >::iterator itNNR;
    CNNR_Rule lNNR_Rule;
    // Resize Table to hold all rules.
    tableWidgetNNR_Rules->setRowCount(mOptionsPat->GetNNRRules().count());
    int iRuleID=0;
    for(itNNR = mOptionsPat->GetNNRRules().begin(); itNNR != mOptionsPat->GetNNRRules().end(); ++itNNR, iRuleID++)
    {
        lNNR_Rule = *itNNR;

        // Fill GUI table (at given row) with specified NNR rule info.
        NNRRuleFillItem(iRuleID);
    }


    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'IDDQ-Delta' tab
    ///////////////////////////////////////////////////////////////////////
    groupBoxIDDQ_Delta->setChecked(mOptionsPat->mIsIDDQ_Delta_enabled);
    lineEditIDDQ_Delta_Bin->setText(QString::number(mOptionsPat->mIDDQ_Delta_SBin));
    pushButtonIDDQ_Delta_Color->setActiveColor(mOptionsPat->mIDDQ_Delta_Color);

    // Rules
    QList <CIDDQ_Delta_Rule>::iterator itIDDQ_Delta;
    CIDDQ_Delta_Rule lIDDQ_Delta_Rule;
    iRuleID=0;
    // Resize Table to hold all rules.
    tableWidgetIDDQ_Delta_Rules->setRowCount(mOptionsPat->mIDDQ_Delta_Rules.count());
    for(itIDDQ_Delta = mOptionsPat->mIDDQ_Delta_Rules.begin(); itIDDQ_Delta != mOptionsPat->mIDDQ_Delta_Rules.end(); ++itIDDQ_Delta, iRuleID++)
    {
        lIDDQ_Delta_Rule = *itIDDQ_Delta;

        // Fill GUI table (at given row) with specified IDDQ rule info.
        IDDQRuleFillItem(iRuleID);
    }

    // GUI: GPAT must include PPAT bins?
    checkBoxGPAT_IgnorePPATBins->setChecked(mOptionsPat->mGPAT_IgnorePPatBins);

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'GDBN' tab
    ///////////////////////////////////////////////////////////////////////
    lineEditBadClusterBin->setText(QString::number(mOptionsPat->mGDBNPatSBin));
    pushButtonBadClusterColor->setActiveColor(mOptionsPat->mGDBNColor);
    groupBoxBadNeighbourhood->setChecked(mOptionsPat->mIsGDBNEnabled);
    // Rules
    QList <CGDBN_Rule>::iterator lGDBNIter;
    CGDBN_Rule lGdbnRule;
    iRuleID=0;
    // Resize Table to hold all rules.
    tableWidgetGdbnRules->setRowCount(mOptionsPat->mGDBNRules.count());
    for(lGDBNIter = mOptionsPat->mGDBNRules.begin(); lGDBNIter != mOptionsPat->mGDBNRules.end(); ++lGDBNIter, iRuleID++)
    {
        lGdbnRule = *lGDBNIter;

        // Fill GUI table (at given row) with specified clustering rule info.
        GdbnRuleFillItem(iRuleID);
    }

    // Custom PAT lib enabled?
    if(m_pPluginList.count() > 0)
        checkBoxBadNeighbourhood_Custom->setChecked(mOptionsPat->mGDBNCustomLib);


    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'ZPAT: Composite / Wafer lot' tab
    ///////////////////////////////////////////////////////////////////////
    if(mOptionsPat->bZPAT_SoftBin)
        comboBox_ZPAT_BinType->setCurrentIndex(0);
    else
        comboBox_ZPAT_BinType->setCurrentIndex(1);
    lineEditBadBinListZPAT->setText(mOptionsPat->pBadBinsZPAT_List->GetRangeList());
    checkBoxZPAT_GDBN->setChecked(mOptionsPat->bZPAT_GDBN_Enabled);
    checkBoxZPAT_Reticle->setChecked(mOptionsPat->bZPAT_Reticle_Enabled);
    checkBoxZPAT_Clustering->setChecked(mOptionsPat->bZPAT_Clustering_Enabled);

    groupBoxCompositeExclusionZone->setChecked(mOptionsPat->GetExclusionZoneEnabled());
    doubleSpinBoxCompositeYieldLevel->setValue(mOptionsPat->lfCompositeExclusionZoneYieldThreshold);
    lineEditExclusionZone->setText(QString::number(mOptionsPat->iCompositeZone_SBin));
    groupBoxMergeEtestStdf->setChecked(mOptionsPat->bMergeEtestStdf);
    lineEditMergeEtestStdf->setText(QString::number(mOptionsPat->iCompositeEtestStdf_SBin));
    // Button color
    pushButtonZpatColor->setActiveColor(mOptionsPat->cZpatColor);


    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Clustering' tab (outline bad clusters)
    ///////////////////////////////////////////////////////////////////////
    groupBoxClusteringPotato->setChecked(mOptionsPat->mClusteringPotato);
    // Clustering Bin#
    lineEditClusteringPotatoBin->setText(QString::number(mOptionsPat->mClusteringPotatoSBin));
    // Button color
    pushButtonClusteringPotatoColor->setActiveColor(mOptionsPat->mClusteringPotatoColor);

    // Rules
    QList <CClusterPotatoRule>::iterator lItClusterPotato;
    CClusterPotatoRule lClusteringRule;
    iRuleID=0;
    // Resize Table to hold all rules.
    tableWidgetClusteringRules->setRowCount(mOptionsPat->mClusterPotatoRules.count());
    for(lItClusterPotato = mOptionsPat->mClusterPotatoRules.begin(); lItClusterPotato != mOptionsPat->mClusterPotatoRules.end(); ++lItClusterPotato, iRuleID++)
    {
        lClusteringRule = *lItClusterPotato;

        // Fill GUI table (at given row) with specified clustering rule info.
        ClusteringRuleFillItem(iRuleID);
    }

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: Reticle tab
    ///////////////////////////////////////////////////////////////////////
    groupBoxReticle->setChecked(mOptionsPat->GetReticleEnabled());
    lineEditBadReticule->setText(QString::number(mOptionsPat->GetReticleSoftBin()));
    pushButtonBadReticule->setActiveColor(mOptionsPat->GetReticleColor());
    comboBoxReticleSource->setCurrentIndex(comboBoxReticleSource->findData(mOptionsPat->GetReticleSizeSource()));
    spinBoxReticleX->setValue(mOptionsPat->GetReticleSizeX());
    spinBoxReticleY->setValue(mOptionsPat->GetReticleSizeY());

    OnReticleSourceChanged();
    InitReticleRuleslist();

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'Masks' tab
    ///////////////////////////////////////////////////////////////////////
    // Mask Rules
    QList <CMask_Rule *>::iterator itMask;
    CMask_Rule *ptMaskRule;
    iRuleID=0;
    // Resize Table to hold all rules.
    tableWidgetMasks->setRowCount(mOptionsPat->mMaskRules.count());
    for(itMask = mOptionsPat->mMaskRules.begin(); itMask != mOptionsPat->mMaskRules.end(); ++itMask, iRuleID++)
    {
        ptMaskRule = *itMask;

        // Fill GUI table (at given row) with specified clustering rule info.
        MaskRule_FillGUI(iRuleID,ptMaskRule);
    }
    // Sort Masks by ascending
    tableWidgetMasks->sortByColumn(0);

    ///////////////////////////////////////////////////////////////////////
    // Load GUI fields: 'FINAL TEST' tab
    ///////////////////////////////////////////////////////////////////////
    if (mOptionsPat->GetRecipeType() != GS::Gex::PAT::RecipeWaferSort)
        tabWidget->setTabEnabled(tabWidget->indexOf(TabFinalTest), true);
    else
        tabWidget->setTabEnabled(tabWidget->indexOf(TabFinalTest), false);

    // Total parts in base line
    spinBoxFinalTest_Baseline->setValue(mOptionsPat->mFT_BaseLine);

    // Alarm level on outliers detected in baseline.
    if(mOptionsPat->mFT_BaseLineMaxOutliers >= 0)
    {
        checkBoxFinalTest_BaselineOutliers->setChecked(true);
        spinBoxFinalTest_BaseLineMaxOutliers->setValue(mOptionsPat->mFT_BaseLineMaxOutliers);
    }
    else
    {
        checkBoxFinalTest_BaselineOutliers->setChecked(false);
        spinBoxFinalTest_BaseLineMaxOutliers->setValue(20);
    }

    // Tuning samples
    spinBoxFT_TuningSamples->setValue(mOptionsPat->mFT_TuningSamples);

    // Dynamic PAT limits tuning frequency (0 = Never tune)
    spinBoxFinalTest_Tuning->setValue(mOptionsPat->mFT_Tuning);
    comboFinalTest_TuningType->setCurrentIndex(mOptionsPat->mFT_TuningType);
    checkBoxFT_TuningFrequency->setChecked(mOptionsPat->mFT_TuningIsEnabled);
    spinBoxFinalTest_Tuning->setEnabled(mOptionsPat->mFT_TuningIsEnabled);
    comboFinalTest_TuningType->setEnabled(mOptionsPat->mFT_TuningIsEnabled);


    // Maximum outliers allowed in a lot
    spinBoxFinalTest_MaxOutlierParts->setValue(mOptionsPat->mFT_MaxOutlierParts);
    // Minimum good yield expected
    comboFinalTest_YieldLevel->setCurrentIndex(mOptionsPat->mFT_YieldLevel);
    // Maximum yield loss on critical Bins
    comboFinalTest_CriticalBinsYield->setCurrentIndex(mOptionsPat->mFT_CriticalBinsYield);
    // List of critical bins
    if(mOptionsPat->mFT_CriticalBinsList == NULL)
        lineEditFinalTest_CriticalBins->clear();
    else
        lineEditFinalTest_CriticalBins->setText(mOptionsPat->mFT_CriticalBinsList->GetRangeList());

    // Final test: Alarm timeouts
    // 7103 : moved some members into propery map
    comboFinalTest_Alarm_Outliers->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_Outliers
                mOptionsPat->property(ALARM_SEVERITY_TOO_MUCH_OUTLIERS_IN_BASELINE).toInt()
                );
    comboFinalTest_Alarm_Shapes->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_Shapes
                mOptionsPat->property(ALARM_SEVERITY_DISTRIB_SHAPE_CHANGED).toInt()
                );
    comboFinalTest_Alarm_PatDrift->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_PatDrift
                mOptionsPat->property(ALARM_SEVERITY_PAT_DRIFT).toInt()
                );
    comboFinalTest_Alarm_LowGoodYield->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_LowGoodYield
                mOptionsPat->property(ALARM_SEVERITY_LOW_GOOD_YIELD).toInt()
                );
    comboFinalTest_Alarm_YieldLoss->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_YieldLoss
                mOptionsPat->property(ALARM_SEVERITY_YIELD_LOSS).toInt()
                );
    comboFinalTest_Alarm_ParamDrift->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_ParamDrift
                mOptionsPat->property(ALARM_SEVERITY_PARAM_DRIFT).toInt()
                );
    comboFinalTest_Alarm_ParamCpk->setCurrentIndex(
                //mOptionsPat->iFT_Alarm_Timeout_ParamCpk
                mOptionsPat->property(ALARM_SEVERITY_PARAM_CPK).toInt()
                );

    // Final test: Alarm emails
//        checkBoxFinalTest_Alarm_Outliers_Email->setChecked(mOptionsPat->iFT_Alarm_Email_Outliers);
//        checkBoxFinalTest_Alarm_Shapes_Email->setChecked(mOptionsPat->iFT_Alarm_Email_Shapes);
//        checkBoxFinalTest_Alarm_PatDrift_Email->setChecked(mOptionsPat->iFT_Alarm_Email_PatDrift);
//        checkBoxFinalTest_Alarm_LowGoodYield_Email->setChecked(mOptionsPat->iFT_Alarm_Email_LowGoodYield);
//        checkBoxFinalTest_Alarm_YieldLoss_Email->setChecked(mOptionsPat->iFT_Alarm_Email_YieldLoss);
//        checkBoxFinalTest_Alarm_ParamDrift_Email->setChecked(mOptionsPat->iFT_Alarm_Email_ParamDrift);
//        checkBoxFinalTest_Alarm_ParamCpk_Email->setChecked(mOptionsPat->iFT_Alarm_Email_ParamCpk);

    // Final test: Miscelaneous
    spinBoxFinalTest_RunsPerPacket->setValue(mOptionsPat->mFT_RunsPerPacket);

    int lIndex = comboFinalTest_Traceability->findData(QVariant(mOptionsPat->mFT_Traceability));

    if (lIndex != -1)
        comboFinalTest_Traceability->setCurrentIndex(lIndex);

    // Report options
    checkBoxReport_Stats->setChecked(mOptionsPat->bReport_Stats);
    checkBoxReport_Histo->setChecked(mOptionsPat->bReport_Histo);
    // PAT-134
    checkBoxReport_HistoForNotFailingPATtests->setChecked(mOptionsPat->bReport_Histo_NoOutliers);
    checkBoxReport_Wafermap->setChecked(mOptionsPat->bReport_Wafermap);
    comboBoxReport_WafermapType->setCurrentIndex(mOptionsPat->iReport_WafermapType); // SBIN or HBIN
    checkBoxReport_Pareto->setChecked(mOptionsPat->bReport_Pareto);
    checkBoxReport_Binning->setChecked(mOptionsPat->bReport_Binning);
    checkBoxReport_SPAT_Limits->setChecked(mOptionsPat->bReport_SPAT_Limits);
    checkBoxReport_DPAT_Limits_Outliers->setChecked(mOptionsPat->bReport_DPAT_Limits_Outliers);
    checkBoxReport_DPAT_Limits_NotOutlier->setChecked(mOptionsPat->bReport_DPAT_Limits_NoOutliers);
    checkBoxReport_MVPATStdCharts->setChecked(mOptionsPat->GetMVPATReportStdCharts());
    groupBoxReport_MVPATCorrCharts->setChecked(mOptionsPat->GetMVPATReportCorrCharts());
    checkBoxReport_MVPATCorrProjection->setChecked(mOptionsPat->GetMVPATReportPCAProjection());
    checkBoxReportMVPATCorrPairs->setChecked(mOptionsPat->GetMVPATReportPairs() == GS::Gex::PAT::AllPairs);
    spinBoxReport_MVPATCorrMaxCharts->setValue(mOptionsPat->GetMVPATReportMaxCharts());

    // Refresh GUI.
    OnCustomAlgorithm();	// Custom PAT Algorithm (library) enabled/disabled
    OnRuleSet();			// AEC-PAT, Smart, etc...
    OnDistribution();		// Redraw +/-N*Sigma for selected Smart distribution
    OnMinimumOutliersToFailPart(); // Number of outliers in flow to fail part...
    OnComboProcessChange();	// Compute PAT limits on: Good bin, All bins, ...
    // FINAL TEST tab
//	OnTuning();				// Enable/Disable Tuning frequency edit fields
    OnBaseLineMaxOutliers();// Enable/Disable maOutlier threshold alarm (in base line computation)

    // Wafer options
    groupBoxReport_PatYieldAlarm->setChecked(mOptionsPat->GetEnableYALimit());
    lineEditOveralPatYALimit->setText(QString::number(mOptionsPat->GetOveralPatYALimit()));
}

///////////////////////////////////////////////////////////
// Display ON-line help page for PAT technology.
///////////////////////////////////////////////////////////
void	PatLimitsDialog::ShowOnLinHelpPage(const QString &strMessage)
{
    // IF message to display, show popup.
    if(strMessage.isEmpty() == false)
    {
        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (! lOk)
        {
            return;
        }
    }

    QString strPatHelpPage;
    strPatHelpPage = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strPatHelpPage += GEX_NEPTUS_FOLDER;
    strPatHelpPage += GEXTB_BROWSER_PAT;

    pGexMainWindow->LoadUrl(strPatHelpPage);
}

///////////////////////////////////////////////////////////
// Tells if valid data, and display explanation message.
///////////////////////////////////////////////////////////
bool	PatLimitsDialog::isValidData(bool bShowMessage/*=true*/)
{
    bool	bNoData = false;
    QString strOptionStorageDevice;

    // No query executed yet...tell it!
    if(gexReport == NULL)
        bNoData = true;
    else
    if(gexReport->getGroupsList().count() <= 0)
        bNoData = true;

    QString strMessage;
    if(bNoData == true)
    {
        // Output File exists...overwrite it?
        strMessage = "To generate the 'Recipe file' (including Static PAT limits),\nyou first need to have Examinator analyze historical data.\nDo you want to see the on-line help section now?";

        // Display ON-line help about PAT technology.
        if(bShowMessage)
            ShowOnLinHelpPage(strMessage);
        return false;
    }

    // Data are available...then check if they've been collected with the right options: compute statistics from samples only
    strOptionStorageDevice = (ReportOptions.GetOption("statistics","computation")).toString();
    if(strOptionStorageDevice != "samples_only")
    {
        // Output File exists...overwrite it?
    strMessage = "Examinator statistics is not set to 'From Samples only'.\nYou need to toggle this option from the 'Options' tab prior\nto create your Outlier configuration file.\nDo you want to see the on-line help section now?";

        // Display ON-line help about PAT technology.
        if(bShowMessage)
            ShowOnLinHelpPage(strMessage);
        return false;
    }

    // Valid data available...set default product name in 'Product' edit field
    CGexGroupOfFiles	*pGroup;
    CGexFileInGroup		*pFile ;
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return false;
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return false;

    QStringList OColumnList;

    // Get product name & remove leading spaces.
    listViewConfigName->clear();

    // Get Design Revision (even if empty string)
    QString strString = pFile->getMirDatas().szDesignRev;
    strString = strString.trimmed();
    //new Q3ListViewItem(listViewConfigName,"Design Rev",strString);
    OColumnList.clear();OColumnList << "Design Rev"<<strString;
    new QTreeWidgetItem(listViewConfigName,OColumnList);

    // Get Job Revision (even if empty string)
    strString = pFile->getMirDatas().szJobRev;
    //new Q3ListViewItem(listViewConfigName,"Job Rev",strString);
    OColumnList.clear(); OColumnList <<"Job Rev"<<strString;
    new QTreeWidgetItem(listViewConfigName,OColumnList);

    // Get Job Name (even if empty string)
    strString = pFile->getMirDatas().szJobName;
    //new Q3ListViewItem(listViewConfigName,"Job Name",strString);
    OColumnList.clear(); OColumnList <<"Job Name"<<strString;
    new QTreeWidgetItem(listViewConfigName,OColumnList);

    // Get Product (even if empty string)
    strString = pFile->getMirDatas().szPartType;
    strString = strString.trimmed();
    //new Q3ListViewItem(listViewConfigName,"Product",strString);
    OColumnList.clear(); OColumnList <<"Product"<<strString;
    new QTreeWidgetItem(listViewConfigName,OColumnList);

//    QDialog diag;
//    QObject *poPa = listViewConfigName->parent();
//    listViewConfigName->setParent(&diag);
//    listViewConfigName->show();
//    diag.exec();
//    listViewConfigName->setParent((QWidget *)poPa);

    return true;
}

///////////////////////////////////////////////////////////
// Select mailing list for email notification
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMailingList(void)
{
    // Info message about Mailing list file format.
    GS::Gex::Message::information(
        "", "Mailing list file format:\n\n"
        " o It is an ASCII file\n"
        " o It can hold multiple emails per line\n"
        " o email format is <address>@<domain>\n"
        " o email addresses separator is ';'\n\n");

    QString strMailingList;
    QFileDialog cFileDialog(this);
    strMailingList = cFileDialog.getOpenFileName(this, "Select mailing list", "", "Mailing list *.txt;;All Files (*.*)");
    if(strMailingList.isEmpty() == TRUE)
        return;	// No mailing list file selected...return!
}

///////////////////////////////////////////////////////////
// Select mailing list for email notification
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnPatPluginChanged()
{
    QString			strSelection = comboBoxCustomPatLib->currentText();

    // Get ptr on plugin for selected entry in the combo
    QList<GexPatPlugin_ID*>::iterator itPlugin = m_pPluginList.begin();

    while(itPlugin != m_pPluginList.end())
    {
        // Check if the new entry corresponds to the one read from the recipe file
        if((*itPlugin)->m_strPluginName == strSelection)
        {
            m_pCurrentPlugin = (*itPlugin);
            break;
        }

        // Next plugin
        itPlugin++;
    }

    // Update GUI elements
    if(m_pCurrentPlugin)
    {
        textLabel_CustomPatLib_File->setText(m_pCurrentPlugin->m_strFileName);
        textLabel_CustomPatLib_Revision->setText(QString::number(m_pCurrentPlugin->m_uiPluginBuild));
    }
}

void PatLimitsDialog::OnMVPATNearDistanceChanged(double lValue)
{
    if (lValue > doubleSpinBoxMVPATMedium->value())
        doubleSpinBoxMVPATMedium->setValue(lValue);
}

void PatLimitsDialog::OnMVPATMediumDistanceChanged(double lValue)
{
    if (lValue < doubleSpinBoxMVPATNear->value())
        doubleSpinBoxMVPATNear->setValue(lValue);

    if (lValue > doubleSpinBoxMVPATFar->value())
        doubleSpinBoxMVPATFar->setValue(lValue);
}

void PatLimitsDialog::OnMVPATFarDistanceChanged(double lValue)
{
    if (lValue < doubleSpinBoxMVPATMedium->value())
        doubleSpinBoxMVPATMedium->setValue(lValue);
}

///////////////////////////////////////////////////////////
// Update Flow list table based on Flow count defined
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnSetTotalFlows()
{
    QString strString;
        //TODO: iUpdateMap_FlowNameID unused ?
        //int iUpdateMap_FlowNameID = mOptionsPat->iUpdateMap_FlowNameID;

    // Table line visible
    tableWidgetFlowNames->showRow(0);

    // Gety flow name in line
    strString = tableWidgetFlowNames->item(0,0)->text();
}
///////////////////////////////////////////////////////////
// User enables/disables the Static or Dynamic limits...
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEnableLimits()
{
    //	if(bStatic == false && bDynamic == false)
    //      Message::information(this, GS::Gex::Engine::GetInstance().
    //                           GetAppFullName(),
    //        "Warning: you have disabled all PAT limits...\nMake sure this is what you want!");
}

///////////////////////////////////////////////////////////
// User enables/disables to ignore tests with High Cpk
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnIgnoreHighCpk()
{
    lineEditCpk->setEnabled(checkBoxIgnoreCpk->isChecked());
}

//////////////////////////////////////////////////////////
// User enables/disables to ignore Static PAT limits for tests with historical High Cpk
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnHistoricalCpk()
{
    lineEditHistoricalCpk->setEnabled(checkBoxHistoricalCpk->isChecked());
}

///////////////////////////////////////////////////////////
// Custom PAT algorithms enabled
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnCustomAlgorithm()
{
    if(groupBoxCustomAlgorithm->isChecked())
    {
        // Custom PAT algorithms enabled
        checkBoxBadNeighbourhood_Custom->show();
    }
    else
    {
        // Custom PAT algorithms disabled
        checkBoxBadNeighbourhood_Custom->hide();
    }
}

///////////////////////////////////////////////////////////
// User selects the rule set: %limits, PAT/AEC-Q001 or Smart
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRuleSet()
{
    int	iRuleID = comboBoxRuleSet->currentIndex();

    // Show/Hide relevant fields
    bool	bShowNfactor=FALSE;
    switch(iRuleID)
    {
        case GEX_TPAT_RULETYPE_SIGMAID:			// Rule set: N*Sigma
        case GEX_TPAT_RULETYPE_ROBUSTSIGMAID:	// Rule set: N*RobustSigma
        case GEX_TPAT_RULETYPE_LIMITSID:		// Rule set: % of limits
        case GEX_TPAT_RULETYPE_Q1Q3IQRID:			// Rule set: Q1+N*IQR, Q2+N*IQR
            bShowNfactor = true;
            break;

        case GEX_TPAT_RULETYPE_SMARTID:		// Rule set: Smart & adaptive
        case GEX_TPAT_RULETYPE_RANGEID:		// Rule set: Range
        case GEX_TPAT_RULETYPE_IGNOREID:	// Rule set: Disabled
        case GEX_TPAT_RULETYPE_NEWLIMITSID:	// Rule set: Force new limits
            bShowNfactor = false;
            break;
    }

    if(bShowNfactor)
    {
        lineEditRuleFactor->show();
        textLabelRuleFactor->show();
        textLabelRuleFactorPrefix->show();
        textLabelRuleFactorSuffix->show();
    }
    else
    {
        lineEditRuleFactor->hide();
        textLabelRuleFactor->hide();
        textLabelRuleFactorPrefix->hide();
        textLabelRuleFactorSuffix->hide();
    }

    // update field contents (some may be hidden!).
    bool	bValue;
    float fValue = lineEditRuleFactor->text().toFloat(&bValue);
    if(bValue == false || fValue <= 0)
        lineEditRuleFactor->setText(gexRuleFactorItems[iRuleID]);
    textLabelRuleFactorPrefix->setText(gexRuleFactorPrefixItems[iRuleID]);
    textLabelRuleFactorSuffix->setText(gexRuleFactorUnits[iRuleID]);
    mOptionsPat->SetDefaultDynamicRule((COptionsPat::GexDefaultRuleSet)iRuleID);
    mOptionsPat->SetDefaultFactor(lineEditRuleFactor->text().toDouble());
}

///////////////////////////////////////////////////////////
// Customizer Head H*Sigma (using slider)
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnHeadSlider()
{
    // Get slider value, load it in the Edit box
    int iPos = sliderHead->value();
    double lfValue = (double)iPos / 100;
    lineEditRuleFactorHead->setText(QString::number(lfValue));

    // Redraw image + markers
    RedrawDistribution();
}

///////////////////////////////////////////////////////////
// Customizer Tail T*Sigma (using slider)
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnTailSlider()
{
    // Get slider value, load it in the Edit box
    int iPos = sliderTail->value();
    double lfValue = (double)iPos / 100;
    lineEditRuleFactorTail->setText(QString::number(lfValue));

    // Redraw image + markers
    RedrawDistribution();
}

///////////////////////////////////////////////////////////
// Customizer Head H*Sigma (using text edit field)
///////////////////////////////////////////////////////////
void PatLimitsDialog::OnHeadFactor()
{
    // Get value from the Edit box, update slider
    QString strValue = lineEditRuleFactorHead->text();
    int		nValue = (int)(strValue.toDouble() * 100.0);
    sliderHead->setValue(nValue);

    // Redraw image + markers
    RedrawDistribution();
}

///////////////////////////////////////////////////////////
// Customizer Tail T*Sigma (using text edit field)
///////////////////////////////////////////////////////////
void PatLimitsDialog::OnTailFactor()
{
    // Get value from the Edit box, update slider
    QString strValue = lineEditRuleFactorTail->text();
    int		nValue = (int)(strValue.toDouble() * 100.0);
    sliderTail->setValue(nValue);

    // Redraw image + markers
    RedrawDistribution();
}

///////////////////////////////////////////////////////////
// USer selects which oultier limits sets to edit (near, medium, far outleir limits)
///////////////////////////////////////////////////////////
void PatLimitsDialog::OnComboSeverityLimits()
{
    // As user changes the Severity limits set to edit, refresh the GUI.
    OnDistribution();
}

void PatLimitsDialog::OnReticleBinChanged()
{
    int lOldReticleBin = mOptionsPat->GetReticleSoftBin();
    int lNewReticleBin = lineEditBadReticule->text().toInt();

    if (lOldReticleBin != lNewReticleBin)
    {
        int lRow = 0;
        QList<PATOptionReticle>::iterator itReticleBegin = mOptionsPat->GetReticleRules().begin();
        QList<PATOptionReticle>::iterator itReticleEnd   = mOptionsPat->GetReticleRules().end();
        for (; itReticleBegin != itReticleEnd; ++itReticleBegin)
        {
            if ((*itReticleBegin).GetReticleSBin() == lOldReticleBin)
            {
                (*itReticleBegin).SetReticleSBin(lNewReticleBin);
                (*itReticleBegin).SetReticleHBin(lNewReticleBin);

                // Update list view
                ReticleRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->SetReticleSoftBin(lNewReticleBin);
        mOptionsPat->SetReticleHardBin(lNewReticleBin);
    }
}

void PatLimitsDialog::OnReticleColorChanged()
{
    QColor lOldReticleColor = mOptionsPat->GetReticleColor();
    QColor lNewReticleColor = pushButtonBadReticule->activeColor();

    if (lOldReticleColor != lNewReticleColor)
    {
        int lRow = 0;
        QList<PATOptionReticle>::iterator itReticleBegin = mOptionsPat->GetReticleRules().begin();
        QList<PATOptionReticle>::iterator itReticleEnd   = mOptionsPat->GetReticleRules().end();
        for (; itReticleBegin != itReticleEnd; ++itReticleBegin)
        {
            if ((*itReticleBegin).GetReticleColor() == lOldReticleColor)
            {
                (*itReticleBegin).SetReticleColor(lNewReticleColor);

                // Update list view
                ReticleRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->SetReticleColor(lNewReticleColor);
    }
}

///////////////////////////////////////////////////////////
// Display Distribution  & Markers
///////////////////////////////////////////////////////////
void PatLimitsDialog::RedrawDistribution()
{
    // Get distribution type selected
    int m_iDistribution = comboBoxDistribution->currentIndex();
    // Get algorithm type
    int m_iAlgorithm= comboBoxAlgorithm->currentIndex();

    // Load PNG image file with relevant distribution chart
    QString strImageFile;
    strImageFile = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    strImageFile += GEX_HELPIMAGES_FOLDER;

    // Check which set of Outlier limits is to be displayed
    int	iSeverityLimits = comboBoxOutlierSeverityLimits->currentIndex();	// GEX_TPAT_OUTLIER_SEVERITY_LIMIT_xxx (near, medium or far)

    // Get relevant images + plot N*Sigma markers
    int	iMeanHeadPos=0;	// To hold 'Mean' position on the drawing for the head side
    int	iMeanTailPos=0;	// To hold 'Mean' position on the drawing for the tail side
    int	iSigma=0;		// To hold the number of pixels for 1 sigma.
    double	lfHeadPos = fabs((double)sliderHead->value()/100.0);
    double	lfTailPos = fabs((double)sliderTail->value()/100.0);
    switch(m_iDistribution)
    {
        case GEX_TPAT_SMART_GAUSSIANID:
            // Gaussian distribution
            strImageFile += "patman_gaussian.png";
            // 6*sigma = 110 pixels on image
            iSigma = 18;
            // Mean position = Pixel 301 on image
            iMeanHeadPos = iMeanTailPos = 301;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadGaussian[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailGaussian[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_GAUSSIANTAILID:
            // Gaussian+tail distribution
            strImageFile += "patman_gaussian_tail.png";
            // 6*sigma = 110 pixels on image
            iSigma = 18;
            // Mean position = Pixel 290 on image
            iMeanHeadPos = iMeanTailPos =  290;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadGaussianTailed[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailGaussianTailed[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_GAUSSIANDUALTAILID:
            // Gaussian+ 2 tails distribution
            strImageFile += "patman_gaussian_2tails.png";
            // 6*sigma = 6*21 pixels on image
            iSigma = 21;
            // Mean position = Pixel 305 on image
            iMeanHeadPos = iMeanTailPos =  305;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadGaussianDoubleTailed[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailGaussianDoubleTailed[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_LOGNORMALID:
            // Log Normal distribution
            strImageFile += "patman_lognormal.png";
            // 6*sigma = 100 pixels on image
            iSigma = 17;
            // Mean position = Pixel 254 on image
            iMeanHeadPos = iMeanTailPos =  254;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadLogNormal[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailLogNormal[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_MULTIMODALID:
            // Multi-Modal distribution
            strImageFile += "patman_bimodal.png";
            // 6*sigma = 100 pixels on image
            iSigma = 17;
            // Mean position for Distribution#1
            iMeanHeadPos = 307;
            // Mean position for Distribution#2
            iMeanTailPos = 334;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadMultiModal[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailMultiModal[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_CLAMPEDID:
            // Clamped distribution
            strImageFile += "patman_clamped.png";
            // 6*sigma = 108 pixels on image
            iSigma = 18;
            // Mean position = Pixel 270 on image
            iMeanHeadPos = iMeanTailPos =  270;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadClamped[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailClamped[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_DUALCLAMPEDID:
            // Double-Clamped distribution
            strImageFile += "patman_doubleclamped.png";
            // 6*sigma = 108 pixels on image
            iSigma = 18;
            // Mean position = Pixel 304 on image
            iMeanHeadPos = iMeanTailPos =  304;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadDoubleClamped[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailDoubleClamped[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_CATEGORYID:
            // Category distribution
            strImageFile += "patman_category.png";
            // Range = 25 pixels on image
            iSigma = 25;
            // Mean position = Pixel 304 on image
            iMeanHeadPos = iMeanTailPos =  304;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadCategory[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailCategory[iSeverityLimits] = lfTailPos;
            break;

        case GEX_TPAT_SMART_UNKNOWNID:
            // Category distribution
            strImageFile += "patman_unknown.png";
            // Range = 25 pixels on image
            iSigma = 25;
            // Mean position = Pixel 304 on image
            iMeanHeadPos = iMeanTailPos =  304;
            // Save new outlier limits
            mOptionsPat->lfSmart_HeadUnknown[iSeverityLimits] = lfHeadPos;
            mOptionsPat->lfSmart_TailUnknown[iSeverityLimits] = lfTailPos;
            break;

    }

    // Load relevant image to show typical distribution.
    QPixmap	cPixmap;
    cPixmap.load(strImageFile);

    // Prepare to paint markers on image
    QString strValue;
    double	lfValue;
    int		nMarkerPos;
    QPainter p;
    p.begin(&cPixmap);

    // Draw -H*Sigma marker
    strValue = lineEditRuleFactorHead->text();
    lfValue = strValue.toDouble();
    nMarkerPos = iMeanHeadPos + (int)(lfValue*iSigma);
    p.setFont(QFont("Arial",10,QFont::DemiBold));
    p.setPen(QPen(Qt::red,2,Qt::DashDotDotLine));
    p.drawLine(nMarkerPos,20,nMarkerPos,250);  // Draw '-H*Sigma' marker

    // Show -H*Sigma legend (to the right of the marker)
    if(m_iAlgorithm != GEX_TPAT_SMART_ALGO_CUSTOMLIB)
        strValue += "*Sigma";
    else
        strValue = "LL: PAT Lib";
    QRect cRectHead(nMarkerPos,0,200,20);
    p.drawText(cRectHead,Qt::AlignLeft | Qt::AlignTop,strValue);

    // Draw +T*Sigma marker
    strValue = lineEditRuleFactorTail->text();
    lfValue = strValue.toDouble();
    nMarkerPos = iMeanTailPos + (int)(lfValue*iSigma);
    p.setPen(QPen(Qt::red,2,Qt::DashDotDotLine));
    p.drawLine(nMarkerPos,20,nMarkerPos,250);  // Draw '+T*Sigma' marker

    // Show +T*Sigma legend (to the left of the marker)
    if(m_iAlgorithm != GEX_TPAT_SMART_ALGO_CUSTOMLIB)
    strValue += "*Sigma";
    else
        strValue = "HL: PAT Lib";
    QRect cRectTail(nMarkerPos-200,0,200,20);
    p.drawText(cRectTail,Qt::AlignRight | Qt::AlignTop,strValue);

    p.end();

    // Draw image + markers.
    pixmapLabelDistribution->setPixmap(cPixmap);
}

///////////////////////////////////////////////////////////
// Customize N*Sigma space per type of distribution.
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnDistribution()
{
    // Get distribution type selected
    int m_iDistribution = comboBoxDistribution->currentIndex();

    // Check which set of Outlier limits is to be displayed
    int	iSeverityLimits = comboBoxOutlierSeverityLimits->currentIndex();	// GEX_TPAT_OUTLIER_SEVERITY_LIMIT_xxx (near, medium or far)

    // Reload default Distribution Haid & Tail limits.
    double	lfHeadPos=0.0,lfTailPos=0.0;
    bool	bPatEnable=FALSE;
    int	iAlgorithm=GEX_TPAT_SMART_ALGO_MEAN;
    switch(m_iDistribution)
    {
        case GEX_TPAT_SMART_GAUSSIANID:
            lfHeadPos = mOptionsPat->lfSmart_HeadGaussian[iSeverityLimits];		// -H*Sigma limit for Gaussian distribution
            lfTailPos = mOptionsPat->lfSmart_TailGaussian[iSeverityLimits];		// +T*Sigma limit for Gaussian distribution
            bPatEnable = mOptionsPat->bPAT_Gaussian;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_Gaussian;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_GAUSSIANTAILID:
            // Gaussian+tail distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadGaussianTailed[iSeverityLimits];	// -H*Sigma limit for Gaussian distribution+tailed
            lfTailPos = mOptionsPat->lfSmart_TailGaussianTailed[iSeverityLimits];	// +T*Sigma limit for Gaussian distribution+tailed;
            bPatEnable = mOptionsPat->bPAT_GaussianTailed;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_GaussianTailed;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_GAUSSIANDUALTAILID:
            // Gaussian+tail distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadGaussianDoubleTailed[iSeverityLimits];	// -H*Sigma limit for Gaussian distribution+2 tails
            lfTailPos = mOptionsPat->lfSmart_TailGaussianDoubleTailed[iSeverityLimits];	// +T*Sigma limit for Gaussian distribution+2 tails;
            bPatEnable = mOptionsPat->bPAT_GaussianDoubleTailed;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_GaussianDoubleTailed;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_LOGNORMALID:
            // Log Normal distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadLogNormal[iSeverityLimits];		// -H*Sigma limit for LogNormal distribution;
            lfTailPos = mOptionsPat->lfSmart_TailLogNormal[iSeverityLimits];		// +T*Sigma limit for LogNormal distribution
            bPatEnable = mOptionsPat->bPAT_LogNormal;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_LogNormal;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_MULTIMODALID:
            // Multi-Modal distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadMultiModal[iSeverityLimits];		// -H*Sigma limit for MultiModal distribution
            lfTailPos = mOptionsPat->lfSmart_TailMultiModal[iSeverityLimits];		// +T*Sigma limit for MulriModal distribution
            bPatEnable = mOptionsPat->bPAT_MultiModal;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_MultiModal;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_CLAMPEDID:
            // Clamped distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadClamped[iSeverityLimits];		// -H*Sigma limit for Clamped distribution
            lfTailPos = mOptionsPat->lfSmart_TailClamped[iSeverityLimits];		// +T*Sigma limit for Clamped distribution
            bPatEnable = mOptionsPat->bPAT_Clamped;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_Clamped;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_DUALCLAMPEDID:
            // Double-Clamped distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadDoubleClamped[iSeverityLimits];	// -H*Sigma limit for Double Clamped distribution
            lfTailPos = mOptionsPat->lfSmart_TailDoubleClamped[iSeverityLimits];	// +T*Sigma limit for Double Clamped distribution
            bPatEnable = mOptionsPat->bPAT_DoubleClamped;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_DoubleClamped;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_CATEGORYID:
            // Category distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadCategory[iSeverityLimits];		// -H*Range limit for Category distribution
            lfTailPos = mOptionsPat->lfSmart_TailCategory[iSeverityLimits];		// +T*Range limit for Category distribution
            bPatEnable = mOptionsPat->bPAT_Category;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_Category;	// Algorithm used to compute PAT limits
            break;

        case GEX_TPAT_SMART_UNKNOWNID:
            // Unknown distribution
            lfHeadPos = mOptionsPat->lfSmart_HeadUnknown[iSeverityLimits];		// -H*Range limit for Unknown distribution
            lfTailPos = mOptionsPat->lfSmart_TailUnknown[iSeverityLimits];		// +T*Range limit for Unknown distribution
            bPatEnable = mOptionsPat->bPAT_Unknown;	// 'true' if PAT limits activated for this distribution
            iAlgorithm = mOptionsPat->iAlgo_Unknown;	// Algorithm used to compute PAT limits
            break;
    }

    // Convert to correct sign
    lfHeadPos = -fabs(lfHeadPos);
    lfTailPos = fabs(lfTailPos);

    // Update Sliders.
    sliderHead->setValue((int)(100.0*lfHeadPos));
    sliderTail->setValue((int)(100.0*lfTailPos));

    // Update GUI (paint the relevant distribution + default outlier limits just set)
    OnHeadSlider();
    OnTailSlider();

    // Algorithm used to compute PAT limits
    comboBoxAlgorithm->setCurrentIndex(iAlgorithm);
    OnAlgorithm();

    // If PAT limits enabled/Disabled for this distribution...
    if(bPatEnable)
        comboBoxDistributionPat->setCurrentIndex(0);	// 'YES: PAT Enabled'
    else
        comboBoxDistributionPat->setCurrentIndex(1);	// 'NO: PAT Disabled'
    OnEnablePat();
}

///////////////////////////////////////////////////////////
// User moving from release X.1 to X+1.0
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnIncrementVersion()
{
    CopyUIData();

    // Save recipe, incrementing to new release#
    GS::Gex::PATRecipeEditor::GetInstance().WriteRecipe(true);

    // Refresh GUI
    RefreshGUI();
}

// GCORE-2149
const QString PatLimitsDialog::mPropNameCurrentAlgo("CurrentAlgorithm");

void PatLimitsDialog::OnAlgorithm()
{
    // Get distribution type selected
    int m_iDistribution = comboBoxDistribution->currentIndex();
    // Get algorithm type
    int m_iAlgorithm= comboBoxAlgorithm->currentIndex();
    GSLOG(SYSLOG_SEV_DEBUG, QString("On algorithm changed to '%1'").arg(comboBoxAlgorithm->currentText()).toLatin1().data() );

    // GCORE-2149
    if (!property(mPropNameCurrentAlgo.toLatin1().data()).isNull())
    {
        QString lPreviousAlgo=property(mPropNameCurrentAlgo.toLatin1().data()).toString();
        // save current params
        mHeadTailValuesPerAlgo.insert(lPreviousAlgo,
                    QPair<double,double>(lineEditRuleFactorHead->text().toDouble(), lineEditRuleFactorTail->text().toDouble()) );

        if (mHeadTailValuesPerAlgo.contains(comboBoxAlgorithm->currentText()))
        {
            QPair<double,double> lHeadAndTail=mHeadTailValuesPerAlgo.value(comboBoxAlgorithm->currentText());
            lineEditRuleFactorHead->setText(QString::number(lHeadAndTail.first));
            lineEditRuleFactorTail->setText(QString::number(lHeadAndTail.second));
        }
    }
    setProperty(mPropNameCurrentAlgo.toLatin1().data(), comboBoxAlgorithm->currentText() );

    switch(m_iDistribution)
    {
        case GEX_TPAT_SMART_GAUSSIANID:
            mOptionsPat->iAlgo_Gaussian = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_GAUSSIANTAILID:
            // Gaussian+tail distribution
            mOptionsPat->iAlgo_GaussianTailed = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_GAUSSIANDUALTAILID:
            // Gaussian+tail distribution
            mOptionsPat->iAlgo_GaussianDoubleTailed = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_LOGNORMALID:
            // Log Normal distribution
            mOptionsPat->iAlgo_LogNormal = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_MULTIMODALID:
            // Multi-Modal distribution
            mOptionsPat->iAlgo_MultiModal = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_CLAMPEDID:
            // Clamped distribution
            mOptionsPat->iAlgo_Clamped = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_DUALCLAMPEDID:
            // Double-Clamped distribution
            mOptionsPat->iAlgo_DoubleClamped = m_iAlgorithm;
            break;

        case GEX_TPAT_SMART_CATEGORYID:
            // Category distribution
            mOptionsPat->iAlgo_Category = m_iAlgorithm;
        break;

        case GEX_TPAT_SMART_UNKNOWNID:
            // Unknown distribution
            mOptionsPat->iAlgo_Unknown = m_iAlgorithm;
        break;
    }

    // Refresh GUI Factors legends accordingly
    QString strHeadLegend,strTailLegend;
    switch(m_iAlgorithm)
    {
        case GEX_TPAT_SMART_ALGO_MEAN:	// Algo: Mean +/- N*Sigma
            strHeadLegend = strTailLegend = "Sigma";
            // Show N & T edit boxes
            frameSmartRules->show();
            break;

        case GEX_TPAT_SMART_ALGO_MEDIAN:// Algo: Median +/- N*RobustSigma
            strHeadLegend = strTailLegend = "R.Sigma";
            // Show N & T edit boxes
            frameSmartRules->show();
            break;

        case GEX_TPAT_SMART_ALGO_Q1Q3IQR:// Algo: LL=Q1-N*IQR, HL = Q3+T*IQR
            strHeadLegend = "(Q1-N*IQR)";
            strTailLegend = "(Q3+T*IQR)";
            // Show N & T edit boxes
            frameSmartRules->show();
            break;

        case GEX_TPAT_SMART_ALGO_CUSTOMLIB:	// User custom PAT lib
            // Hide N & T edit boxes
            frameSmartRules->hide();
            break;
    }
    textLabelRuleFactorSuffixHead->setText(strHeadLegend);
    textLabelRuleFactorSuffixTail->setText(strTailLegend);

    // Redraw distribution
    RedrawDistribution();
}

///////////////////////////////////////////////////////////
// Enabled/Disable PAT limits for a given distribution
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEnablePat()
{
    // Get distribution type selected
    int m_iDistribution = comboBoxDistribution->currentIndex();

    // Get Enable/Disable selection.
    bool bPatEnable = (comboBoxDistributionPat->currentIndex() == 0) ? true: false;

    switch(m_iDistribution)
    {
        case GEX_TPAT_SMART_GAUSSIANID:
            mOptionsPat->bPAT_Gaussian = bPatEnable;
            break;

        case GEX_TPAT_SMART_GAUSSIANTAILID:
            // Gaussian+tail distribution
            mOptionsPat->bPAT_GaussianTailed = bPatEnable;
            break;

        case GEX_TPAT_SMART_GAUSSIANDUALTAILID:
            // Gaussian+tail distribution
            mOptionsPat->bPAT_GaussianDoubleTailed = bPatEnable;
            break;

        case GEX_TPAT_SMART_LOGNORMALID:
            // Log Normal distribution
            mOptionsPat->bPAT_LogNormal = bPatEnable;
            break;

        case GEX_TPAT_SMART_MULTIMODALID:
            // Multi-Modal distribution
            mOptionsPat->bPAT_MultiModal = bPatEnable;
            break;

        case GEX_TPAT_SMART_CLAMPEDID:
            // Clamped distribution
            mOptionsPat->bPAT_Clamped = bPatEnable;
            break;

        case GEX_TPAT_SMART_DUALCLAMPEDID:
            // Double-Clamped distribution
            mOptionsPat->bPAT_DoubleClamped = bPatEnable;
            break;

        case GEX_TPAT_SMART_CATEGORYID:
            // Category distribution
            mOptionsPat->bPAT_Category = bPatEnable;
            break;

        case GEX_TPAT_SMART_UNKNOWNID:
            // Unkown distribution
            mOptionsPat->bPAT_Unknown = bPatEnable;
            break;
    }

    groupBoxDistribution->setEnabled(bPatEnable);
}



void PatLimitsDialog::OnAddReticleRule()
{
    ReticleDefinitionDialog lReticleDialog(this);
    PATOptionReticle lRule;

    lRule.SetReticleSBin(mOptionsPat->GetReticleSoftBin());
    lRule.SetReticleHBin(mOptionsPat->GetReticleHardBin());
    lRule.SetReticleColor(mOptionsPat->GetReticleColor());

    // Load rule details into DialogBox
    lReticleDialog.InitGui(*mOptionsPat);
    lReticleDialog.LoadGui(lRule);

    // Display NNR Definition editor
    if(lReticleDialog.exec() != 1)
        return;

    // Enlarge rule list, add new entry.
    int lRuleCount = mOptionsPat->GetReticleRules().count();

    lReticleDialog.ReadGUI(lRule);	// Get Rule settings from GUI.

    mOptionsPat->GetReticleRules().append(lRule);

    // Fill table
    ReticleRuleFillItem(lRuleCount);	// Update list view

    // Select last line
    tableWidgetReticleRules->setCurrentCell(lRuleCount,0);

    OnReticleListUpdated();
}

void PatLimitsDialog::OnEditReticleRule()
{
    int lCurrentRow = tableWidgetReticleRules->currentRow();

    if (lCurrentRow >= 0 && lCurrentRow < mOptionsPat->GetReticleRules().count())
    {
        ReticleDefinitionDialog lReticleDialog(this);
        PATOptionReticle lRule = mOptionsPat->GetReticleRules().at(lCurrentRow);

        // Load rule details into DialogBox
        lReticleDialog.InitGui(*mOptionsPat);
        lReticleDialog.LoadGui(lRule);

        // Display NNR Definition editor
        if(lReticleDialog.exec() != 1)
            return;

        lReticleDialog.ReadGUI(lRule);	// Get Rule settings from GUI.

        mOptionsPat->GetReticleRules().replace(lCurrentRow, lRule);

        // Fill table
        ReticleRuleFillItem(lCurrentRow);	// Update list view

        // Select last line
        tableWidgetReticleRules->setCurrentCell(lCurrentRow,0);
    }
}

void PatLimitsDialog::OnRemoveReticleRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetReticleRules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->GetReticleRules().count())
    {
        // Request confirmation to erase given rule...
        QString strRule = "Confirm to delete rule:\n" + mOptionsPat->GetReticleRules().at(lCurrentRow).GetRuleName();
        bool lOk;
        GS::Gex::Message::request("", strRule, lOk);
        if (! lOk)
        {
            return;
        }

        // Remove rule
        mOptionsPat->GetReticleRules().removeAt(lCurrentRow);
        tableWidgetReticleRules->removeRow(lCurrentRow);

        // If current row was not the last, refresh the gui
        ReticleRuleFillItem(lCurrentRow);

        OnReticleListUpdated();
    }
}

void PatLimitsDialog::OnDuplicateReticleRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetReticleRules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->GetReticleRules().count())
    {
        PATOptionReticle lReticleRule = mOptionsPat->GetReticleRules().at(lCurrentRow);

        // Create name 'copy of...'
        lReticleRule.SetRuleName("Copy of " + lReticleRule.GetRuleName());

        // Add new rule in recipe
        mOptionsPat->GetReticleRules().append(lReticleRule);

        // Update gui
        ReticleRuleFillItem(mOptionsPat->GetReticleRules().count()-1);

        OnReticleListUpdated();
    }
}

///////////////////////////////////////////////////////////
// Reticle: Move rule UP
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveUpReticleRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetReticleRules->currentRow();
    if(lCurrentRow > 0 && lCurrentRow < mOptionsPat->GetReticleRules().count())
    {
        // Swap rules
        mOptionsPat->GetReticleRules().swap(lCurrentRow, lCurrentRow-1);

        // Refresh GUI
        ReticleRuleFillItem(lCurrentRow - 1);
        ReticleRuleFillItem(lCurrentRow);

        // Select above line
        tableWidgetReticleRules->setCurrentCell(lCurrentRow - 1, 0);

        OnReticleListUpdated();
    }
}
///////////////////////////////////////////////////////////
// Reticle: Move rule Down
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveDownReticleRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetReticleRules->currentRow();
    if(lCurrentRow + 1 < tableWidgetReticleRules->rowCount() && lCurrentRow + 1 < mOptionsPat->GetReticleRules().count())
    {
        // Swap rules
        mOptionsPat->GetReticleRules().swap(lCurrentRow, lCurrentRow+1);

        // Refresh GUI
        ReticleRuleFillItem(lCurrentRow);
        ReticleRuleFillItem(lCurrentRow + 1);

        // Select below line
        tableWidgetReticleRules->setCurrentCell(lCurrentRow + 1, 0);

        OnReticleListUpdated();
    }
}

void PatLimitsDialog::OnReticleListUpdated()
{
    int lCurrentRow     = tableWidgetReticleRules->currentRow();
    int lReticleRules   = tableWidgetReticleRules->rowCount();

    pushButtonDownReticleRule->setEnabled(lCurrentRow < lReticleRules-1);
    pushButtonUpReticleRule->setEnabled(lCurrentRow > 0);
    pushButtonDuplicateReticleRule->setEnabled(lCurrentRow >= 0);
    pushButtonRemoveReticleRule->setEnabled(lCurrentRow >= 0);
    pushButtonEditReticleRule->setEnabled(lCurrentRow >= 0);
}

void PatLimitsDialog::OnReticleSourceChanged()
{
    switch (comboBoxReticleSource->currentData().toInt())
    {
        case PATOptionReticle::RETICLE_SIZE_FIXED:
            labelReticleX->setVisible(true);
            labelReticleY->setVisible(true);
            spinBoxReticleX->setVisible(true);
            spinBoxReticleY->setVisible(true);
            break;

        case PATOptionReticle::RETICLE_SIZE_FILE:
            labelReticleX->setVisible(false);
            labelReticleY->setVisible(false);
            spinBoxReticleX->setVisible(false);
            spinBoxReticleY->setVisible(false);
            break;

        default:
            break;
    }

}


///////////////////////////////////////////////////////////
// NNR: Add rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnAddNNR_Rule()
{
    NnrDefinitionDialog cNNR_Rule(this, true, Qt::Dialog);
    CNNR_Rule lRule;

    lRule.SetSoftBin(mOptionsPat->GetNNRSoftBin());
    lRule.SetHardBin(mOptionsPat->GetNNRHardBin());
    lRule.SetFailBinColor(mOptionsPat->GetNNRColor());

    // Init UI
    cNNR_Rule.InitGui(*mOptionsPat);
    // Load rule details into DialogBox
    cNNR_Rule.LoadGUI(lRule);

    // Display NNR Definition editor
    if(cNNR_Rule.exec() != 1)
        return;

    // Enlarge rule list, add new entry.
    int lRuleCount = tableWidgetNNR_Rules->rowCount();
    tableWidgetNNR_Rules->insertRow(lRuleCount);

    // Fill table
    cNNR_Rule.ReadGUI(lRule);	// Get Rule settings from GUI.

    mOptionsPat->GetNNRRules().append(lRule);

    // Fill table
    NNRRuleFillItem(lRuleCount);	// Update list view

    // Select last line
    tableWidgetNNR_Rules->setCurrentCell(lRuleCount,0);

    // Select last line
    tableWidgetNNR_Rules->setCurrentCell(lRuleCount,0);
}

///////////////////////////////////////////////////////////
// NNR: Edit rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEditNNR_Rule()
{
    int lCurrentRow = tableWidgetNNR_Rules->currentRow();

    if (lCurrentRow >= 0 && lCurrentRow < mOptionsPat->GetNNRRules().count())
    {
        NnrDefinitionDialog lNNRDialog(this,true,Qt::Dialog);
        CNNR_Rule lRule = mOptionsPat->GetNNRRules().at(lCurrentRow);

        // Init UI
        lNNRDialog.InitGui(*mOptionsPat);
        // Load rule details into DialogBox
        lNNRDialog.LoadGUI(lRule);

        // Display NNR Definition editor
        if(lNNRDialog.exec() != 1)
            return;

        lNNRDialog.ReadGUI(lRule);	// Get Rule settings from GUI.

        mOptionsPat->GetNNRRules().replace(lCurrentRow, lRule);

        // Fill table
        NNRRuleFillItem(lCurrentRow);	// Update list view

        // Select last line
        tableWidgetNNR_Rules->setCurrentCell(lCurrentRow,0);
    }
}

///////////////////////////////////////////////////////////
// NNR: Duplicate rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnDuplicateNNR_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetNNR_Rules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->GetNNRRules().count())
    {
        CNNR_Rule lNNRRule = mOptionsPat->GetNNRRules().at(lCurrentRow);

        // Create name 'copy of...'
        lNNRRule.SetRuleName("Copy of " + lNNRRule.GetRuleName());

        // Add new rule in recipe
        mOptionsPat->GetNNRRules().append(lNNRRule);

        // Update gui
        NNRRuleFillItem(mOptionsPat->GetNNRRules().count()-1);

        OnNNRListUpdated();
    }
}

///////////////////////////////////////////////////////////
// NNR: Move rule UP
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveUpNNR_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetNNR_Rules->currentRow();
    if(lCurrentRow > 0 && lCurrentRow < mOptionsPat->GetNNRRules().count())
    {
        // Swap rules
        mOptionsPat->GetNNRRules().swap(lCurrentRow, lCurrentRow-1);

        // Refresh GUI
        NNRRuleFillItem((lCurrentRow - 1));
        NNRRuleFillItem(lCurrentRow);

        // Select above line
        tableWidgetNNR_Rules->setCurrentCell(lCurrentRow - 1, 0);

        OnNNRListUpdated();
    }
}

///////////////////////////////////////////////////////////
// NNR: Move rule DOWN
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveDownNNR_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetNNR_Rules->currentRow();
    if(lCurrentRow + 1 < tableWidgetNNR_Rules->rowCount() &&
            lCurrentRow + 1 < mOptionsPat->GetNNRRules().count())
    {
        // Swap rules
        mOptionsPat->GetNNRRules().swap(lCurrentRow, lCurrentRow+1);

        // Refresh GUI
        NNRRuleFillItem(lCurrentRow);
        NNRRuleFillItem(lCurrentRow + 1);

        // Select below line
        tableWidgetNNR_Rules->setCurrentCell(lCurrentRow + 1, 0);

        OnNNRListUpdated();
    }
}

///////////////////////////////////////////////////////////
// NNR: Remove rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRemoveNNR_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetNNR_Rules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->GetNNRRules().count())
    {
        // Request confirmation to erase given rule...
        QString strRule = "Confirm to delete rule:\n" + mOptionsPat->GetNNRRules().at(lCurrentRow).GetRuleName();
        bool lOk;
        GS::Gex::Message::request("", strRule, lOk);
        if (! lOk)
        {
            return;
        }

        // Remove rule
        mOptionsPat->GetNNRRules().removeAt(lCurrentRow);
        tableWidgetNNR_Rules->removeRow(lCurrentRow);

        // If current row was not the last, refresh the gui
        NNRRuleFillItem(lCurrentRow);

        OnNNRListUpdated();
    }
}

void PatLimitsDialog::OnIDDQBinChanged()
{
    int lOldIddqBin = mOptionsPat->mIDDQ_Delta_SBin;
    int lNewIddqBin = lineEditIDDQ_Delta_Bin->text().toInt();

    if (lOldIddqBin != lNewIddqBin)
    {
        int lRow = 0;
        QList<CIDDQ_Delta_Rule>::iterator itIddqBegin = mOptionsPat->mIDDQ_Delta_Rules.begin();
        QList<CIDDQ_Delta_Rule>::iterator itIddqEnd   = mOptionsPat->mIDDQ_Delta_Rules.end();
        for (; itIddqBegin != itIddqEnd; ++itIddqBegin)
        {
            if ((*itIddqBegin).GetSoftBin() == lOldIddqBin)
            {
                (*itIddqBegin).SetSoftBin(lNewIddqBin);
                (*itIddqBegin).SetHardBin(lNewIddqBin);

                // Update list view
                IDDQRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->mIDDQ_Delta_SBin = lNewIddqBin;
        mOptionsPat->mIDDQ_Delta_HBin = lNewIddqBin;
    }
}

void PatLimitsDialog::OnIDDQColorChanged()
{
    QColor lOldIddqColor = mOptionsPat->mIDDQ_Delta_Color;
    QColor lNewIddqColor = pushButtonIDDQ_Delta_Color->activeColor();

    if (lOldIddqColor != lNewIddqColor)
    {
        int lRow = 0;
        QList<CIDDQ_Delta_Rule>::iterator itIddqBegin = mOptionsPat->mIDDQ_Delta_Rules.begin();
        QList<CIDDQ_Delta_Rule>::iterator itIddqEnd   = mOptionsPat->mIDDQ_Delta_Rules.end();
        for (; itIddqBegin != itIddqEnd; ++itIddqBegin)
        {
            if ((*itIddqBegin).GetFailBinColor() == lOldIddqColor)
            {
                (*itIddqBegin).SetFailBinColor(lNewIddqColor);

                // Update list view
                IDDQRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->mIDDQ_Delta_Color = lNewIddqColor;
    }
}

///////////////////////////////////////////////////////////
// NNR rule: fill GUItable (at given row) with
//   specified rule info.

///////////////////////////////////////////////////////////
bool	PatLimitsDialog::NNRRuleFillItem(int row)
{

    // Add row if needed
    if (row < mOptionsPat->GetNNRRules().count())
    {
        if (row >= tableWidgetNNR_Rules->rowCount())
            tableWidgetNNR_Rules->insertRow(row);

        const CNNR_Rule& lRule = mOptionsPat->GetNNRRules().at(row);
        int lCol=-1;

        tableWidgetNNR_Rules->setItem(row, ++lCol, new QTableWidgetItem(lRule.GetRuleName()));						// Rule name
        tableWidgetNNR_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number((int)lRule.IsEnabled())));	// Enabled?
        tableWidgetNNR_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lRule.GetClusterSize())));	// NNR cluster size (5,7,9,11...)
        tableWidgetNNR_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lRule.GetLA())));	// Test density
        tableWidgetNNR_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lRule.GetAlgorithm())));	// NNR Algorithm (Stringent, Yield safe...)
        tableWidgetNNR_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lRule.GetNFactor())));	// N factor (eg: 6 for 6*Sigma)
        QTableWidgetItem* lFailBinItem = new QTableWidgetItem(QString::number(lRule.GetSoftBin()));
        lFailBinItem->setBackgroundColor(lRule.GetFailBinColor());
        tableWidgetNNR_Rules->setItem(row, ++lCol, lFailBinItem);	// FailBin


        // Make each item read-only
        QTableWidgetItem *lItem;
        for(lCol=0;lCol < tableWidgetNNR_Rules->columnCount(); ++lCol)
        {
            lItem = tableWidgetNNR_Rules->item(row,lCol);
            if (lItem)
                lItem->setFlags( lItem->flags() & ~(Qt::ItemIsEditable));
        }
        return true;
    }
    return false;
}

void PatLimitsDialog::OnNNRListUpdated()
{
    int lCurrentRow     = tableWidgetNNR_Rules->currentRow();
    int lNNRRules       = tableWidgetNNR_Rules->rowCount();

    pushButtonDownNNR_Rule->setEnabled(lCurrentRow < lNNRRules-1);
    pushButtonUpNNR_Rule->setEnabled(lCurrentRow > 0);
    pushButtonDuplicateNNR_Rule->setEnabled(lCurrentRow >= 0);
    pushButtonRemoveNNR_Rule->setEnabled(lCurrentRow >= 0);
    pushButtonEditNNR_Rule->setEnabled(lCurrentRow >= 0);
}

///////////////////////////////////////////////////////////
// IDDQ-Delta: Add rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnAddIDDQ_Delta_Rule()
{
    IddqDeltaDefinitionDialog lIDDQRuleDialog(this,true,Qt::Dialog);
    CIDDQ_Delta_Rule lRule;

    lRule.SetSoftBin(mOptionsPat->mIDDQ_Delta_SBin);
    lRule.SetHardBin(mOptionsPat->mIDDQ_Delta_HBin);
    lRule.SetFailBinColor(mOptionsPat->mIDDQ_Delta_Color);

    // Load rule details into DialogBox
    lIDDQRuleDialog.fillGUI(lRule);

    // Display IDDQ-Delta Definition editor
    if(lIDDQRuleDialog.exec() != 1)
        return;

    // Enlarge rule list, add new entry.
    int lRuleCount = tableWidgetIDDQ_Delta_Rules->rowCount();
    tableWidgetIDDQ_Delta_Rules->insertRow(lRuleCount);

    lIDDQRuleDialog.readGUI(lRule);	// Get Rule settings from GUI.

    mOptionsPat->mIDDQ_Delta_Rules.append(lRule);

    // Fill table
    IDDQRuleFillItem(lRuleCount);	// Update list view

    // Select last line
    tableWidgetIDDQ_Delta_Rules->setCurrentCell(lRuleCount,0);
}

///////////////////////////////////////////////////////////
// IDDQ-DELTA: Edit rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEditIDDQ_Delta_Rule()
{
    int lCurrentRow = tableWidgetIDDQ_Delta_Rules->currentRow();

    if (lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mIDDQ_Delta_Rules.count())
    {
        IddqDeltaDefinitionDialog lIDDQDialog(this,true,Qt::Dialog);
        CIDDQ_Delta_Rule lRule = mOptionsPat->mIDDQ_Delta_Rules.at(lCurrentRow);

        // Load rule details into DialogBox
        lIDDQDialog.fillGUI(lRule);

        // Display IDDQ Definition editor
        if(lIDDQDialog.exec() != 1)
            return;

        lIDDQDialog.readGUI(lRule);	// Get Rule settings from GUI.

        mOptionsPat->mIDDQ_Delta_Rules.replace(lCurrentRow, lRule);

        // Fill table
        IDDQRuleFillItem(lCurrentRow);	// Update list view

        // Select last line
        tableWidgetIDDQ_Delta_Rules->setCurrentCell(lCurrentRow,0);
    }
}

///////////////////////////////////////////////////////////
// IDDQ-DELTA: Duplicate rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnDuplicateIDDQ_Delta_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetIDDQ_Delta_Rules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mIDDQ_Delta_Rules.count())
    {
        CIDDQ_Delta_Rule lRule = mOptionsPat->mIDDQ_Delta_Rules.at(lCurrentRow);

        // Create name 'copy of...'
        lRule.SetRuleName("Copy of " + lRule.GetRuleName());

        // Add new rule in recipe
        mOptionsPat->mIDDQ_Delta_Rules.append(lRule);

        // Update gui
        IDDQRuleFillItem(mOptionsPat->mIDDQ_Delta_Rules.count()-1);

        OnIDDQDeltaListUpdated();
    }
}

///////////////////////////////////////////////////////////
// IDDQ-DELTA: Move rule UP
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveUpIDDQ_Delta_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetIDDQ_Delta_Rules->currentRow();
    if(lCurrentRow > 0 && lCurrentRow < mOptionsPat->mIDDQ_Delta_Rules.count())
    {
        // Swap rules
        mOptionsPat->mIDDQ_Delta_Rules.swap(lCurrentRow, lCurrentRow-1);

        // Refresh GUI
        IDDQRuleFillItem((lCurrentRow - 1));
        IDDQRuleFillItem(lCurrentRow);

        // Select above line
        tableWidgetIDDQ_Delta_Rules->setCurrentCell(lCurrentRow - 1, 0);

        OnIDDQDeltaListUpdated();
    }
}

///////////////////////////////////////////////////////////
// IDDQ-DELTA: Move rule DOWN
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveDownIDDQ_Delta_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetIDDQ_Delta_Rules->currentRow();
    if(lCurrentRow + 1 < tableWidgetIDDQ_Delta_Rules->rowCount() &&
            lCurrentRow + 1 < mOptionsPat->mIDDQ_Delta_Rules.count())
    {
        // Swap rules
        mOptionsPat->mIDDQ_Delta_Rules.swap(lCurrentRow, lCurrentRow+1);

        // Refresh GUI
        IDDQRuleFillItem(lCurrentRow);
        IDDQRuleFillItem(lCurrentRow + 1);

        // Select below line
        tableWidgetIDDQ_Delta_Rules->setCurrentCell(lCurrentRow + 1, 0);

        OnIDDQDeltaListUpdated();
    }
}

///////////////////////////////////////////////////////////
// IDDQ-DELTA: Remove rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRemoveIDDQ_Delta_Rule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetIDDQ_Delta_Rules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mIDDQ_Delta_Rules.count())
    {
        // Request confirmation to erase given rule...
        QString strRule = "Confirm to delete rule:\n" + mOptionsPat->mIDDQ_Delta_Rules.at(lCurrentRow).GetRuleName();
        bool lOk;
        GS::Gex::Message::request("", strRule, lOk);
        if (! lOk)
        {
            return;
        }

        // Remove rule
        mOptionsPat->mIDDQ_Delta_Rules.removeAt(lCurrentRow);
        tableWidgetIDDQ_Delta_Rules->removeRow(lCurrentRow);

        // If current row was not the last, refresh the gui
        IDDQRuleFillItem(lCurrentRow);

        OnIDDQDeltaListUpdated();
    }
}

void PatLimitsDialog::OnGdbnBinChanged()
{
    int lOldGdbnBin = mOptionsPat->mGDBNPatSBin;
    int lNewGdbnBin = lineEditBadClusterBin->text().toInt();

    if (lOldGdbnBin != lNewGdbnBin)
    {
        int lRow = 0;
        QList<CGDBN_Rule>::iterator itGdbnBegin = mOptionsPat->mGDBNRules.begin();
        QList<CGDBN_Rule>::iterator itGdbnEnd   = mOptionsPat->mGDBNRules.end();
        for (; itGdbnBegin != itGdbnEnd; ++itGdbnBegin)
        {
            if ((*itGdbnBegin).mSoftBin == lOldGdbnBin)
            {
                (*itGdbnBegin).mSoftBin = lNewGdbnBin;
                (*itGdbnBegin).mHardBin = lNewGdbnBin;

                // Update list view
                GdbnRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->mGDBNPatSBin = lNewGdbnBin;
        mOptionsPat->mGDBNPatHBin = lNewGdbnBin;
    }
}

void PatLimitsDialog::OnGdbnColorChanged()
{
    QColor lOldGdbnColor = mOptionsPat->mGDBNColor;
    QColor lNewGdbnColor = pushButtonBadClusterColor->activeColor();

    if (lOldGdbnColor != lNewGdbnColor)
    {
        int lRow = 0;
        QList<CGDBN_Rule>::iterator itGdbnBegin = mOptionsPat->mGDBNRules.begin();
        QList<CGDBN_Rule>::iterator itGdbnEnd   = mOptionsPat->mGDBNRules.end();
        for (; itGdbnBegin != itGdbnEnd; ++itGdbnBegin)
        {
            if ((*itGdbnBegin).mFailBinColor == lOldGdbnColor)
            {
                (*itGdbnBegin).mFailBinColor = lNewGdbnColor;

                // Update list view
                GdbnRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->mGDBNColor = lNewGdbnColor;
    }
}

void PatLimitsDialog::OnIDDQDeltaListUpdated()
{
    int lCurrentRow = tableWidgetIDDQ_Delta_Rules->currentRow();
    int lRulesCount = tableWidgetIDDQ_Delta_Rules->rowCount();

    pushButtonDownIDDQ_Delta_Rule->setEnabled(lCurrentRow < lRulesCount-1);
    pushButtonUpIDDQ_Delta_Rule->setEnabled(lCurrentRow > 0);
    pushButtonDuplicateIDDQ_Delta_Rule->setEnabled(lCurrentRow >= 0);
    pushButtonRemoveIDDQ_Delta_Rule->setEnabled(lCurrentRow >= 0);
    pushButtonEditIDDQ_Delta_Rule->setEnabled(lCurrentRow >= 0);
}

///////////////////////////////////////////////////////////
// IDDQ_Delta rule: fill GUItable (at given row) with
//   specified rule info.
///////////////////////////////////////////////////////////
bool PatLimitsDialog::IDDQRuleFillItem(int row)
{
    // Add row if needed
    if (row < mOptionsPat->mIDDQ_Delta_Rules.count())
    {
        if (row >= tableWidgetIDDQ_Delta_Rules->rowCount())
            tableWidgetIDDQ_Delta_Rules->insertRow(row);

        const CIDDQ_Delta_Rule& lRule = mOptionsPat->mIDDQ_Delta_Rules.at(row);
        int lCol=-1;
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(lRule.GetRuleName()));				// Rule name
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number((int) lRule.IsEnabled())));	// Enabled
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(lRule.GetPreStress()));		// Pre-stress sub-string to find in test names
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(lRule.GetPostStress()));	// Post-stress sub-string to find in test names
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number((int) lRule.GetCaseSensitive())));	// true if case sensitive string checks.
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lRule.GetAlgorithm())));	// IDDQ Algorithm (N*sigma, IQR,...)
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lRule.GetNFactor())));	// N factor (eg: 6 for 6*Sigma)
        QTableWidgetItem* lFailBinItem = new QTableWidgetItem(QString::number(lRule.GetSoftBin()));
        lFailBinItem->setBackgroundColor(lRule.GetFailBinColor());
        tableWidgetIDDQ_Delta_Rules->setItem(row, ++lCol, lFailBinItem);	// FailBin

        // Make each item read-only
        QTableWidgetItem *lItem;
        for(lCol=0;lCol < tableWidgetIDDQ_Delta_Rules->columnCount(); lCol++)
        {
            lItem = tableWidgetIDDQ_Delta_Rules->item(row,lCol);
            if (lItem)
                lItem->setFlags( lItem->flags() & ~(Qt::ItemIsEditable));
        }
        return true;
    }
    return false;
}


///////////////////////////////////////////////////////////
// Clustering rule: fill GUItable (at given row) with
//   specified clustering rule info.

///////////////////////////////////////////////////////////
bool	PatLimitsDialog::ClusteringRuleFillItem(int row)
{

    // Add row if needed
    if (row < mOptionsPat->mClusterPotatoRules.count())
    {
        if (row >= tableWidgetClusteringRules->rowCount())
            tableWidgetClusteringRules->insertRow(row);

        const CClusterPotatoRule& lClusteringRule = mOptionsPat->mClusterPotatoRules.at(row);
        int lCol=-1;
        QString strString;
        int		iIndex;

        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(lClusteringRule.mRuleName));
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mIsEnabled)));		// Enabled?
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(lClusteringRule.mMaskName));
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(lClusteringRule.mBadBinIdentifyList->GetRangeList()));	// Binlist identifying bad cluster
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(lClusteringRule.mBadBinInkingList->GetRangeList()));	// Binlist inking bad cluster
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mWaferSource)));	// Sbin/Hbin/Prober map

        if(lClusteringRule.mClusterSize < 0)
            strString = QString::number(-lClusteringRule.mClusterSize) + "%";
        else
            strString = QString::number(lClusteringRule.mClusterSize);
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(strString));									// Cluster size
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mOutlineWidth)));		// Outline width
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mIsLightOutlineEnabled)));		// Light outline ?

        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(3+2*lClusteringRule.mOutlineMatrixSize)));			// Fail weight threshold of bad dies.
        strString = "";
        for(iIndex=0;iIndex <= lClusteringRule.mOutlineMatrixSize; iIndex++)
        {
            if(iIndex)
                strString += "/";
            strString += QString::number(lClusteringRule.mAdjWeightLst[iIndex]);
        }
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(strString));			// Adjacent weight for bad dies

        strString = "";
        for(iIndex=0;iIndex <= lClusteringRule.mOutlineMatrixSize; iIndex++)
        {
            if(iIndex)
                strString += "/";
            strString += QString::number(lClusteringRule.mDiagWeightLst[iIndex]);
        }
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(strString));			// Diagonal weight for bad dies

        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mFailWeight)));			// Fail weight threshold of bad dies.

        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mIgnoreScratchLines)));	// Ignore scratch lines?
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mIgnoreScratchRows)));	// Ignore scratch rows?
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mIgnoreDiagonalBadDies)));
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mEdgeDieType)));				// Edge-die type
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mEdgeDieWeighting)));		// Edge-die rule type
        tableWidgetClusteringRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lClusteringRule.mEdgeDieWeightingScale)));	// Factor

        QTableWidgetItem* lFailBinItem = new QTableWidgetItem(QString::number(lClusteringRule.mSoftBin));
        lFailBinItem->setBackgroundColor(lClusteringRule.mFailBinColor);
        tableWidgetClusteringRules->setItem(row, ++lCol, lFailBinItem);	// FailBin

        // Make each item read-only
        QTableWidgetItem *ptItem;
        for(lCol=0;lCol < tableWidgetClusteringRules->columnCount(); ++lCol)
        {
            ptItem = tableWidgetClusteringRules->item(row,lCol);
            if (ptItem)
                ptItem->setFlags( ptItem->flags() & ~(Qt::ItemIsEditable));
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////
// Clustering: Add rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnAddClusteringRule()
{
    ClusterDefinitionDialog lClusteringDialog(this,true,Qt::Dialog);
    CClusterPotatoRule lRule;

    lRule.mSoftBin = mOptionsPat->mClusteringPotatoSBin;
    lRule.mHardBin = mOptionsPat->mClusteringPotatoHBin;
    lRule.mFailBinColor = mOptionsPat->mClusteringPotatoColor;

    // Load rule details into DialogBox
    lClusteringDialog.fillGUI(lRule, mOptionsPat);

    // Display IDDQ-Delta Definition editor
    if(lClusteringDialog.exec() != 1)
        return;

    lClusteringDialog.readGUI(lRule);

    // Enlarge rule list, add new entry.
    int lRuleCount = tableWidgetClusteringRules->rowCount();
    tableWidgetClusteringRules->insertRow(lRuleCount);

    mOptionsPat->mClusterPotatoRules.append(lRule);

    // Fill table
    ClusteringRuleFillItem(lRuleCount);	// Update list view

    // Select last line
    tableWidgetClusteringRules->setCurrentCell(lRuleCount,0);
}

///////////////////////////////////////////////////////////
// Clustering: Edit rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEditClusteringRule()
{
    int lCurrentRow = tableWidgetClusteringRules->currentRow();

    if (lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mClusterPotatoRules.count())
    {
        ClusterDefinitionDialog lClusteringDialog(this,true,Qt::Dialog);
        CClusterPotatoRule lRule = mOptionsPat->mClusterPotatoRules.at(lCurrentRow);

        // Load rule details into DialogBox
        lClusteringDialog.fillGUI(lRule, mOptionsPat);

        // Display IDDQ Definition editor
        if(lClusteringDialog.exec() != 1)
            return;

        lClusteringDialog.readGUI(lRule);	// Get Rule settings from GUI.

        mOptionsPat->mClusterPotatoRules.replace(lCurrentRow, lRule);

        // Fill table
        ClusteringRuleFillItem(lCurrentRow);	// Update list view

        // Select last line
        tableWidgetClusteringRules->setCurrentCell(lCurrentRow,0);
    }
}

///////////////////////////////////////////////////////////
// Clustering: Duplicate rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnDuplicateClusteringRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetClusteringRules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mClusterPotatoRules.count())
    {
        CClusterPotatoRule lRule = mOptionsPat->mClusterPotatoRules.at(lCurrentRow);

        // Create name 'copy of...'
        lRule.mRuleName = "Copy of " + lRule.mRuleName;

        // Add new rule in recipe
        mOptionsPat->mClusterPotatoRules.append(lRule);

        // Update gui
        ClusteringRuleFillItem(mOptionsPat->mClusterPotatoRules.count()-1);

        OnClusteringListUpdated();
    }
}

///////////////////////////////////////////////////////////
// Clustering: Move rule UP
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveUpClusteringRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetClusteringRules->currentRow();
    if(lCurrentRow > 0 && lCurrentRow < mOptionsPat->mClusterPotatoRules.count())
    {
        // Swap rules
        mOptionsPat->mClusterPotatoRules.swap(lCurrentRow, lCurrentRow-1);

        // Refresh GUI
        ClusteringRuleFillItem(lCurrentRow - 1);
        ClusteringRuleFillItem(lCurrentRow);

        // Select above line
        tableWidgetClusteringRules->setCurrentCell(lCurrentRow - 1, 0);

        OnClusteringListUpdated();
    }
}
///////////////////////////////////////////////////////////
// Clustering: Move rule Down
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveDownClusteringRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetClusteringRules->currentRow();
    if(lCurrentRow + 1 < tableWidgetClusteringRules->rowCount() &&
            lCurrentRow + 1 < mOptionsPat->mClusterPotatoRules.count())
    {
        // Swap rules
        mOptionsPat->mClusterPotatoRules.swap(lCurrentRow, lCurrentRow+1);

        // Refresh GUI
        ClusteringRuleFillItem(lCurrentRow);
        ClusteringRuleFillItem(lCurrentRow + 1);

        // Select below line
        tableWidgetClusteringRules->setCurrentCell(lCurrentRow + 1, 0);

        OnClusteringListUpdated();
    }
}

///////////////////////////////////////////////////////////
// Clustering: Remove rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRemoveClusteringRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetClusteringRules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mClusterPotatoRules.count())
    {
        // Request confirmation to erase given rule...
        QString strRule = "Confirm to delete rule:\n" + mOptionsPat->mClusterPotatoRules.at(lCurrentRow).mRuleName;
        bool lOk;
        GS::Gex::Message::request("", strRule, lOk);
        if (! lOk)
        {
            return;
        }

        // Remove rule
        mOptionsPat->mClusterPotatoRules.removeAt(lCurrentRow);
        tableWidgetClusteringRules->removeRow(lCurrentRow);

        // If current row was not the last, refresh the gui
        ClusteringRuleFillItem(lCurrentRow);

        OnClusteringListUpdated();
    }
}
///////////////////////////////////////////////////////////
// Mask rule: fill GUItable (at given row) with
//   specified mask rule info.
///////////////////////////////////////////////////////////
bool	PatLimitsDialog::MaskRule_FillGUI(int iRow,CMask_Rule *ptRule)
{
    tableWidgetMasks->setItem(iRow, 0, new QTableWidgetItem(ptRule->mRuleName));
    tableWidgetMasks->setItem(iRow, 1, new QTableWidgetItem(QString::number(ptRule->mIsEnabled)));		// Enabled?
    tableWidgetMasks->setItem(iRow, 2, new QTableWidgetItem(QString::number(ptRule->mWorkingArea)));    // Working area
    tableWidgetMasks->setItem(iRow, 3, new QTableWidgetItem(QString::number(ptRule->mRadius)));			// Radius

    // Make each item read-only
    int iCol;
    QTableWidgetItem *ptItem;
    for(iCol=0;iCol < tableWidgetMasks->columnCount(); iCol++)
    {
        ptItem = tableWidgetMasks->item(iRow,iCol);
        if (ptItem)
            ptItem->setFlags( ptItem->flags() & ~(Qt::ItemIsEditable));
    }
    return true;
}

void PatLimitsDialog::InitReticleRuleslist()
{
    int lCount = mOptionsPat->GetReticleRules().count();

    tableWidgetReticleRules->setRowCount(lCount);

    for (int lIdx = 0; lIdx < lCount; ++lIdx)
    {
        ReticleRuleFillItem(lIdx);
    }

    // Refresh button
    OnReticleListUpdated();
}

void PatLimitsDialog::ReticleRuleFillItem(int row)
{
    // Add row if needed
    if (row < mOptionsPat->GetReticleRules().count())
    {
        if (row >= tableWidgetReticleRules->rowCount())
            tableWidgetReticleRules->insertRow(row);

        const PATOptionReticle& lRule = mOptionsPat->GetReticleRules().at(row);

        int lCol = 0;

        // Rule Name
        tableWidgetReticleRules->setItem(row, lCol, new QTableWidgetItem(lRule.GetRuleName()));
        // Status
        tableWidgetReticleRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number((int)lRule.IsReticleEnabled())));
        // Reticle bad bins list
        tableWidgetReticleRules->setItem(row, ++lCol, new QTableWidgetItem(lRule.GetBadBinsReticleList().GetRangeList()));
        // Size
        QString lLabel;
        if (lRule.GetRule() == PATOptionReticle::REPEATING_PATTERNS)
        {
            lLabel = "Repeating patterns";
        }
        else if (lRule.GetRule() == PATOptionReticle::CORNER)
        {
            lLabel = "Corner rule";
        }
        else
        {
            lLabel = "Step defectivity check";
        }
        // Rule
        tableWidgetReticleRules->setItem(row, ++lCol, new QTableWidgetItem(lLabel));
        QTableWidgetItem* lFailBinItem = new QTableWidgetItem(QString::number(lRule.GetReticleSBin()));
        lFailBinItem->setBackgroundColor(lRule.GetReticleColor());
        tableWidgetReticleRules->setItem(row, ++lCol, lFailBinItem);	// FailBin

        // Make each item read-only
        QTableWidgetItem *lWidgetItem;
        for(lCol = 0; lCol < tableWidgetReticleRules->columnCount(); ++lCol)
        {
            lWidgetItem = tableWidgetReticleRules->item(row, lCol);
            if (lWidgetItem)
                lWidgetItem->setFlags( lWidgetItem->flags() & ~(Qt::ItemIsEditable));
        }
        tableWidgetReticleRules->resizeColumnsToContents();
    }
}

///////////////////////////////////////////////////////////
// Mask rule: Read GUI table (at given row) and return
//   Mask rule info.
///////////////////////////////////////////////////////////
bool	PatLimitsDialog::MaskRule_ReadGUI(int iRow,CMask_Rule &cRule)
{
    cRule.mRuleName = tableWidgetMasks->item(iRow,0)->text();
    cRule.mIsEnabled = tableWidgetMasks->item(iRow,1)->text().toInt();
    cRule.mWorkingArea = tableWidgetMasks->item(iRow,2)->text().toInt();
    cRule.mRadius = tableWidgetMasks->item(iRow,3)->text().toInt();

    return true;
}

///////////////////////////////////////////////////////////
// Mask: Add definition
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnAddMask()
{
    MaskDefinitionDialog cMaskRule(true,this,true,Qt::Dialog);
    CMask_Rule* cRule = new CMask_Rule;

    // Load rule details into DialogBox
    cMaskRule.fillGUI(*cRule);

    // Display Clustering Definition editor
    if(cMaskRule.exec() != 1)
        return;

    // Enlarge rule list, add new entry.
    tableWidgetMasks->insertRow(0);

    // Fill table
    cMaskRule.readGUI(*cRule);  // Get Rule settings from GUI
    MaskRule_FillGUI(0, cRule);  // Update list view

    // Select 1st line
    tableWidgetMasks->setCurrentCell(0,0);

    // Force sorting
    tableWidgetMasks->sortByColumn(0);

    // Ensure recipe config file updated to display latest MAsks created
    //autoSave();

    mOptionsPat->mMaskRules.append(cRule);
}

///////////////////////////////////////////////////////////
// Mask: Duplicate definition
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnDuplicateMask()
{
    // Check which Rule is currently selected...
    int iCurrentRow = tableWidgetMasks->currentRow();
    if(iCurrentRow < 0)
        return;

    // Extract rule details from selected GUI line
    CMask_Rule* cRule = new CMask_Rule;
    MaskRule_ReadGUI(iCurrentRow, *cRule);

    // Create name 'copy of...'
    cRule->mRuleName = "Copy of " + cRule->mRuleName;

    // Insert this as a new entry.
    tableWidgetMasks->insertRow(iCurrentRow+1);

    // Fill table
    MaskRule_FillGUI(iCurrentRow+1, cRule); // Update list view

    // Ensure recipe config file updated to display latest MAsks created
    //autoSave();

    mOptionsPat->mMaskRules.append(cRule);
}

///////////////////////////////////////////////////////////
// Mask: Edit definition
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEditMask()
{
    // Check which Rule is currently selected...
    int iCurrentRow = tableWidgetMasks->currentRow();
    if( iCurrentRow < 0)
        return;

    // Extract rule details from selected GUI line
    CMask_Rule cRule;
    MaskRule_ReadGUI(iCurrentRow,cRule);

    // Display Clustering Definition editor
    MaskDefinitionDialog cMaskRule(false,this,true,Qt::Dialog);
    cMaskRule.fillGUI(cRule);	// Load rule details into DialogBox
    if(cMaskRule.exec() != 1)
        return;

    // Readback edits from DialogBox
    cMaskRule.readGUI(cRule);

    // Update TableView.
    MaskRule_FillGUI(iCurrentRow,&cRule);	// Update list view

    // Ensure recipe config file updated to display latest MAsks created
    //autoSave();
}

///////////////////////////////////////////////////////////
// Mask: Remove definition
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRemoveMask()
{
    // Check which Rule is currently selected...
    int iCurrentRow = tableWidgetMasks->currentRow();
    if(iCurrentRow < 0)
        return;

    // Request confirmation to erase given rule...
    QString strRule = "Confirm to delete mask:\n" + tableWidgetMasks->item(iCurrentRow,0)->text();
    bool lOk;
    GS::Gex::Message::request("", strRule, lOk);
    if (! lOk)
    {
        return;
    }

    // Removce row
    tableWidgetMasks->removeRow(iCurrentRow);

    // Ensure recipe config file updated to display latest MAsks created
    //autoSave();

    qDeleteAll(mOptionsPat->mMaskRules);
    mOptionsPat->mMaskRules.clear();
    for(int iRuleID = 0; iRuleID < tableWidgetMasks->rowCount(); ++iRuleID)
    {
        CMask_Rule* ptMaskRule = new CMask_Rule;
        mOptionsPat->mMaskRules.append(ptMaskRule);
        MaskRule_ReadGUI(iRuleID, *ptMaskRule);
    }
}


///////////////////////////////////////////////////////////
// GDBN rule: fill GUItable (at given row) with
//   specified clustering rule info.

///////////////////////////////////////////////////////////
bool	PatLimitsDialog::GdbnRuleFillItem(int row)
{
    // Add row if needed
    if (row < mOptionsPat->mGDBNRules.count())
    {
        if (row >= tableWidgetGdbnRules->rowCount())
            tableWidgetGdbnRules->insertRow(row);

        const CGDBN_Rule& lGdbnRule = mOptionsPat->mGDBNRules.at(row);
        int lCol=-1;
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(lGdbnRule.mRuleName));
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number((int)lGdbnRule.mIsEnabled))); // Enabled
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(lGdbnRule.mMaskName));
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mYieldThreshold)));	// Yield threshold
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(lGdbnRule.mBadBinList->GetRangeList()));		// GDBN bad bins list
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mWafermapSource)));	// Sbin/Hbin/Prober map
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mAlgorithm)));		// Algorihm: Squeeze/Weighting
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number((int)lGdbnRule.mFailWaferEdges))); // Edge dies ignored?

        // Squeeze algorithm
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mClusterSize)));		// Matrix size
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mFailCount)));		// Fail coutn allowed

        // Weighting algorithm
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mAdjWeightLst.at(0))));			// Adjacent weight for bad dies
        tableWidgetGdbnRules->setItem(row, ++lCol, new QTableWidgetItem(QString::number(lGdbnRule.mDiagWeightLst.at(0))));			// Diagonal weight for bad dies
        tableWidgetGdbnRules->setItem(row, ++lCol,new QTableWidgetItem(QString::number(lGdbnRule.mMinimumWeighting)));			// Fail weight threshold of bad dies.
        tableWidgetGdbnRules->setItem(row, ++lCol,new QTableWidgetItem(QString::number(lGdbnRule.mEdgeDieType)));				// Weighting rule: Edge die type
        tableWidgetGdbnRules->setItem(row, ++lCol,new QTableWidgetItem(QString::number(lGdbnRule.mEdgeDieWeighting)));			// Weighting rule: Edge die handling
        tableWidgetGdbnRules->setItem(row, ++lCol,new QTableWidgetItem(QString::number(lGdbnRule.mEdgeDieWeightingScale)));	// Scaling factor over computed weight
        QTableWidgetItem* lFailBinItem = new QTableWidgetItem(QString::number(lGdbnRule.mSoftBin));
        lFailBinItem->setBackgroundColor(lGdbnRule.mFailBinColor);
        tableWidgetGdbnRules->setItem(row, ++lCol, lFailBinItem);	// FailBin

        // Make each item read-only
        QTableWidgetItem *lItem;
        for(lCol=0; lCol < tableWidgetGdbnRules->columnCount(); lCol++)
        {
            lItem = tableWidgetGdbnRules->item(row,lCol);
            if (lItem)
                lItem->setFlags( lItem->flags() & ~(Qt::ItemIsEditable));
        }
        return true;
    }
    return false;
}

void PatLimitsDialog::OnClusteringListUpdated()
{
    int lCurrentRow = tableWidgetClusteringRules->currentRow();
    int lRulesCount = tableWidgetClusteringRules->rowCount();

    pushButtonDownClusteringRule->setEnabled(lCurrentRow < lRulesCount-1);
    pushButtonUpClusteringRule->setEnabled(lCurrentRow > 0);
    pushButtonDuplicateClusteringRule->setEnabled(lCurrentRow >= 0);
    pushButtonRemoveClusteringRule->setEnabled(lCurrentRow >= 0);
    pushButtonEditClusteringRule->setEnabled(lCurrentRow >= 0);
}

void PatLimitsDialog::OnGdbnListUpdated()
{
    int lCurrentRow = tableWidgetGdbnRules->currentRow();
    int lRulesCount = tableWidgetGdbnRules->rowCount();

    pushButtonDownGdbnRule->setEnabled(lCurrentRow < lRulesCount-1);
    pushButtonUpGdbnRule->setEnabled(lCurrentRow > 0);
    pushButtonDuplicateGdbnRule->setEnabled(lCurrentRow >= 0);
    pushButtonRemoveGdbnRule->setEnabled(lCurrentRow >= 0);
    pushButtonEditGdbnRule->setEnabled(lCurrentRow >= 0);
}

///////////////////////////////////////////////////////////
// GDBN: Add rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnAddGdbnRule()
{

    GdbnDefinitionDialog lGDBNDialog(this,true,Qt::Dialog);
    CGDBN_Rule lRule;

    lRule.mSoftBin = mOptionsPat->mGDBNPatSBin;
    lRule.mHardBin = mOptionsPat->mGDBNPatHBin;
    lRule.mFailBinColor = mOptionsPat->mGDBNColor;

    // Load rule details into DialogBox
    lGDBNDialog.fillGUI(lRule, mOptionsPat);

    // Display IDDQ-Delta Definition editor
    if(lGDBNDialog.exec() != 1)
        return;

    lGDBNDialog.readGUI(lRule);

    // Enlarge rule list, add new entry.
    int lRuleCount = tableWidgetGdbnRules->rowCount();
    tableWidgetGdbnRules->insertRow(lRuleCount);

    mOptionsPat->mGDBNRules.append(lRule);

    // Fill table
    GdbnRuleFillItem(lRuleCount);	// Update list view

    // Select last line
    tableWidgetGdbnRules->setCurrentCell(lRuleCount,0);
}

///////////////////////////////////////////////////////////
// GDBN: Edit rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnEditGdbnRule()
{
    int lCurrentRow = tableWidgetGdbnRules->currentRow();

    if (lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mGDBNRules.count())
    {
        GdbnDefinitionDialog lGDBNDialog(this,true,Qt::Dialog);
        CGDBN_Rule lRule = mOptionsPat->mGDBNRules.at(lCurrentRow);

        // Load rule details into DialogBox
        lGDBNDialog.fillGUI(lRule, mOptionsPat);

        // Display IDDQ Definition editor
        if(lGDBNDialog.exec() != 1)
            return;

        lGDBNDialog.readGUI(lRule);	// Get Rule settings from GUI.

        mOptionsPat->mGDBNRules.replace(lCurrentRow, lRule);

        // Fill table
        GdbnRuleFillItem(lCurrentRow);	// Update list view

        // Select last line
        tableWidgetGdbnRules->setCurrentCell(lCurrentRow,0);
    }
}

///////////////////////////////////////////////////////////
// GDBN: Duplicate rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnDuplicateGdbnRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetGdbnRules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mGDBNRules.count())
    {
        CGDBN_Rule lRule = mOptionsPat->mGDBNRules.at(lCurrentRow);

        // Create name 'copy of...'
        lRule.mRuleName = "Copy of " + lRule.mRuleName;

        // Add new rule in recipe
        mOptionsPat->mGDBNRules.append(lRule);

        // Update gui
        GdbnRuleFillItem(mOptionsPat->mGDBNRules.count()-1);

        OnGdbnListUpdated();
    }
}

///////////////////////////////////////////////////////////
// GDBN: Move rule UP
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveUpGdbnRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetGdbnRules->currentRow();
    if(lCurrentRow > 0 && lCurrentRow < mOptionsPat->mGDBNRules.count())
    {
        // Swap rules
        mOptionsPat->mGDBNRules.swap(lCurrentRow, lCurrentRow-1);

        // Refresh GUI
        GdbnRuleFillItem((lCurrentRow - 1));
        GdbnRuleFillItem(lCurrentRow);

        // Select above line
        tableWidgetGdbnRules->setCurrentCell(lCurrentRow - 1, 0);

        OnGdbnListUpdated();
    }
}

///////////////////////////////////////////////////////////
// GDBN: Move rule DOWN
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMoveDownGdbnRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetGdbnRules->currentRow();
    if(lCurrentRow + 1 < tableWidgetGdbnRules->rowCount() &&
            lCurrentRow + 1 < mOptionsPat->mGDBNRules.count())
    {
        // Swap rules
        mOptionsPat->mGDBNRules.swap(lCurrentRow, lCurrentRow+1);

        // Refresh GUI
        GdbnRuleFillItem(lCurrentRow);
        GdbnRuleFillItem(lCurrentRow + 1);

        // Select below line
        tableWidgetGdbnRules->setCurrentCell(lCurrentRow + 1, 0);

        OnGdbnListUpdated();
    }
}

///////////////////////////////////////////////////////////
// GDBN: Remove rule
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRemoveGdbnRule()
{
    // Check which Rule is currently selected...
    int lCurrentRow = tableWidgetGdbnRules->currentRow();
    if(lCurrentRow >= 0 && lCurrentRow < mOptionsPat->mGDBNRules.count())
    {
        // Request confirmation to erase given rule...
        QString strRule = "Confirm to delete rule:\n" + mOptionsPat->mGDBNRules.at(lCurrentRow).mRuleName;
        bool lOk;
        GS::Gex::Message::request("", strRule, lOk);
        if (! lOk)
        {
            return;
        }

        // Remove rule
        mOptionsPat->mGDBNRules.removeAt(lCurrentRow);
        tableWidgetGdbnRules->removeRow(lCurrentRow);

        // If current row was not the last, refresh the gui
        GdbnRuleFillItem(lCurrentRow);

        OnGdbnListUpdated();
    }
}

void PatLimitsDialog::OnClusteringBinChanged()
{
    int lOldClusterBin = mOptionsPat->mClusteringPotatoSBin;
    int lNewClusterBin = lineEditClusteringPotatoBin->text().toInt();

    if (lOldClusterBin != lNewClusterBin)
    {
        int lRow = 0;
        QList<CClusterPotatoRule>::iterator itClusterBegin = mOptionsPat->mClusterPotatoRules.begin();
        QList<CClusterPotatoRule>::iterator itClusterEnd   = mOptionsPat->mClusterPotatoRules.end();
        for (; itClusterBegin != itClusterEnd; ++itClusterBegin)
        {
            if ((*itClusterBegin).mSoftBin == lOldClusterBin)
            {
                (*itClusterBegin).mSoftBin = lNewClusterBin;
                (*itClusterBegin).mHardBin = lNewClusterBin;

                // Update list view
                ClusteringRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->mClusteringPotatoSBin = lNewClusterBin;
        mOptionsPat->mClusteringPotatoHBin = lNewClusterBin;
    }
}

void PatLimitsDialog::OnClusteringColorChanged()
{
    QColor lOldClusterColor = mOptionsPat->mClusteringPotatoColor;
    QColor lNewClusterColor = pushButtonClusteringPotatoColor->activeColor();

    if (lOldClusterColor != lNewClusterColor)
    {
        int lRow = 0;
        QList<CClusterPotatoRule>::iterator itClusterBegin = mOptionsPat->mClusterPotatoRules.begin();
        QList<CClusterPotatoRule>::iterator itClusterEnd   = mOptionsPat->mClusterPotatoRules.end();
        for (; itClusterBegin != itClusterEnd; ++itClusterBegin)
        {
            if ((*itClusterBegin).mFailBinColor == lOldClusterColor)
            {
                (*itClusterBegin).mFailBinColor = lNewClusterColor;

                // Update list view
                ClusteringRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->mClusteringPotatoColor = lNewClusterColor;
    }
}

///////////////////////////////////////////////////////////
// Define Bin list to use for building the PAT dynamic limits
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnComboProcessChange()
{
    switch(comboBoxProcess->itemData(comboBoxProcess->currentIndex()).toInt())
    {
        case GEX_TPAT_BUILDLIMITS_ALLBINS:          // All bins
        case GEX_TPAT_BUILDLIMITS_GOODSOFTBINS:		// Good soft bins only
        case GEX_TPAT_BUILDLIMITS_GOODHARDBINS:		// Good hard bins only
            editProcess->setEnabled(false);
            break;
        case GEX_TPAT_BUILDLIMITS_LISTSOFTBINS:		// List of soft bins
        case GEX_TPAT_BUILDLIMITS_LISTHARDBINS:		// List of hard bins
        case GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS:	// All soft bins except...
        case GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS:	// All hard bins except...
            editProcess->setEnabled(true);
            break;
    }
}

///////////////////////////////////////////////////////////
// Define Bin list to use for building the PAT dynamic limits
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnMinimumOutliersToFailPart()
{
    if (mOptionsPat->GetRecipeType() != GS::Gex::PAT::RecipeFinalTest)
    {
        if(spinBoxMinimumOutliersPerPart->value() == 1)
        {
            // One outlier in flow fails part
            checkBoxStopOnFirstFail->show();
        }
        else
        {
            // Two outliers (or more) in flow fail part
            checkBoxStopOnFirstFail->setChecked(false);
            checkBoxStopOnFirstFail->hide();
        }
    }
}

///////////////////////////////////////////////////////////
// Rules oreder: Move selection UP
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRulePrecedenceUp()
{
    int iCurrentRow = listWidgetRulesOrder->currentRow();
    if(iCurrentRow == 0 && iCurrentRow != -1)
        return;	// Item already #1 in list!

    // Save item name to move UP
    QString strLabel = listWidgetRulesOrder->item (iCurrentRow)->text();

    // Move current item one position UP
    listWidgetRulesOrder->takeItem(iCurrentRow);
    listWidgetRulesOrder->insertItem (iCurrentRow-1, strLabel);

    // Select item moved up
    listWidgetRulesOrder->setCurrentRow(iCurrentRow-1);
}

///////////////////////////////////////////////////////////
// Rules oreder: Move selection DOWN
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnRulePrecedenceDown()
{
    int iCurrentRow = listWidgetRulesOrder->currentRow();
    if(iCurrentRow == -1 || iCurrentRow == listWidgetRulesOrder->count()-1)
        return;	// Item already last in list!

    // Save item name to move Down
    QString strLabel = listWidgetRulesOrder->item (iCurrentRow)->text();

    // Move current item one position Down
    listWidgetRulesOrder->takeItem(iCurrentRow);
    listWidgetRulesOrder->insertItem (iCurrentRow+1, strLabel);

    // Select item moved up
    listWidgetRulesOrder->setCurrentRow(iCurrentRow+1);
}

void PatLimitsDialog::OnBaseLineChanged(int lNewBaseline)
{
    if (lNewBaseline > spinBoxFT_TuningSamples->value())
        spinBoxFT_TuningSamples->setValue(lNewBaseline);
}

void PatLimitsDialog::OnTuningSamplesChanged(int lNewTuningSamples)
{
    if (lNewTuningSamples < spinBoxFinalTest_Baseline->value())
        spinBoxFinalTest_Baseline->setValue(lNewTuningSamples);
}

void PatLimitsDialog::OnTuningFrequencyToggled(bool lActivated)
{
    spinBoxFinalTest_Tuning->setEnabled(lActivated);
    comboFinalTest_TuningType->setEnabled(lActivated);
}

///////////////////////////////////////////////////////////
// Final test: Enable/Disable Outlier threshold alarm (in base line computation)
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnBaseLineMaxOutliers()
{
    // Enable/Disable Edit fields
    spinBoxFinalTest_BaseLineMaxOutliers->setEnabled(checkBoxFinalTest_BaselineOutliers->isChecked());
}

void	PatLimitsDialog::saveDefaultPatmanConfigFile()
{
    //QString lErrorMessage;
    QString lRecipeName;

    lRecipeName += GS::Gex::Engine::GetInstance().Get("UserFolder").toString() + QDir::separator();
    lRecipeName += GS::Gex::PAT::GetDefaultRecipeName(mOptionsPat->GetRecipeType());

    GS::Gex::PATRecipeIO* lRecipeIO = PATRecipeIO::CreateRecipeIo(PATRecipeIO::JSON);

    if (lRecipeIO)
    {
        // Update PAT recipe object with GUI data
        CopyUIData();

        // Write the recipe options
        lRecipeIO->Write(*mOptionsPat, lRecipeName);

        delete lRecipeIO;
        lRecipeIO = NULL;
    }
}

void PatLimitsDialog::OnNNRBinChanged()
{
    int lOldBinNNR = mOptionsPat->GetNNRSoftBin();
    int lNewBinNNR = lineEditNNR_Bin->text().toInt();

    if (lOldBinNNR != lNewBinNNR)
    {
        int lRow = 0;
        QList<CNNR_Rule>::iterator itNNRBegin = mOptionsPat->GetNNRRules().begin();
        QList<CNNR_Rule>::iterator itNNREnd   = mOptionsPat->GetNNRRules().end();
        for (; itNNRBegin != itNNREnd; ++itNNRBegin)
        {
            if ((*itNNRBegin).GetSoftBin() == lOldBinNNR)
            {
                (*itNNRBegin).SetSoftBin(lNewBinNNR);
                (*itNNRBegin).SetHardBin(lNewBinNNR);

                // Update list view
                NNRRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->SetNNRSoftBin(lNewBinNNR);
        mOptionsPat->SetNNRHardBin(lNewBinNNR);
    }
}

void PatLimitsDialog::OnNNRColorChanged()
{
    QColor lOldNNRColor = mOptionsPat->GetNNRColor();
    QColor lNewNNRColor = pushButtonNNR_Color->activeColor();

    if (lOldNNRColor != lNewNNRColor)
    {
        int lRow = 0;
        QList<CNNR_Rule>::iterator itNNRBegin = mOptionsPat->GetNNRRules().begin();
        QList<CNNR_Rule>::iterator itNNREnd   = mOptionsPat->GetNNRRules().end();
        for (; itNNRBegin != itNNREnd; ++itNNRBegin)
        {
            if ((*itNNRBegin).GetFailBinColor() == lOldNNRColor)
            {
                (*itNNRBegin).SetFailBinColor(lNewNNRColor);

                // Update list view
                NNRRuleFillItem(lRow);
            }

            ++lRow;
        }

        mOptionsPat->SetNNRColor(lNewNNRColor);
    }
}


///////////////////////////////////////////////////////////
// Create PAT recipe file
///////////////////////////////////////////////////////////
void	PatLimitsDialog::OnOk()
{
    CopyUIData();
    accept();
}


QString PatLimitsDialog::BuildRecipeFileName()
{
    // Initializes variables.
    CGexGroupOfFiles *pGroup=0;
    CGexFileInGroup	*pFile=0;
    QString strFolderName="";
    QString	strRecipeName;

    QTreeWidgetItem *ptItem = NULL;

    //Get path to folder of last file analyzed!
    if(gexReport != NULL)
    {
        pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
        if(pGroup != NULL)
        {
            pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            if(pFile != NULL)
            {
                QFileInfo cFileInfo(pFile->strFileName);
                strFolderName = cFileInfo.absolutePath() + "/";
            }
        }
    }

    // If no folder defined, see if we can use a previous path to recipe...
    if(strFolderName.isEmpty())
    {
        if(strConfigFile.isEmpty())
            strFolderName = "/";
        else
        {
            QFileInfo cFileInfo(strConfigFile);
            strFolderName = cFileInfo.absolutePath() + "/";
        }
    }

    if (!mOptionsPat)
    {
        GSLOG(3, "Can't include ProductName in recipe file name because mOptionsPat null");
    }
    else
    {
        if(lineEditProduct->text().isEmpty())
        {
            ptItem = listViewConfigName->topLevelItem(3);	// Handle to 'Product' info.

            if (ptItem)
                mOptionsPat->strProductName = ptItem->text(1);	// Product
            else
                GSLOG(SYSLOG_SEV_WARNING, "NULL pointer detected for Product");
        }
        else
            mOptionsPat->strProductName = lineEditProduct->text();
    }

    // If include 'Product' name in string...
    if(checkBoxConfigName_Product->isChecked())
    {
        // case 6914
        if (!mOptionsPat->strProductName.isEmpty()) // on my master, it is always empty
            strRecipeName += mOptionsPat->strProductName;
        else
        {
            ptItem = listViewConfigName->topLevelItem(3); // Product appears now in the 4th position ?
            if (ptItem)
            {
                strRecipeName += ptItem->text(1);
            }
            else
                GSLOG(SYSLOG_SEV_WARNING, "NULL pointer detected for Product");
        }
    }

    // If include 'Job name' in string
    if(checkBoxConfigName_JobName->isChecked())
    {
        ptItem = listViewConfigName->topLevelItem(2);	// Pointing 'Job name'
        if (ptItem)
        {
            if(strRecipeName.isEmpty() == false)
                strRecipeName += "_";
            strRecipeName += ptItem->text(1);               // Job name
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "NULL pointer detected for Job Name");
    }

    // If include 'Job revision' in string
    if(checkBoxConfigName_JobRevision->isChecked())
    {
        ptItem = listViewConfigName->topLevelItem(1);	// Pointing 'Job rev'

        if (ptItem)
        {
            if(strRecipeName.isEmpty() == false)
                strRecipeName += "_";
            strRecipeName += ptItem->text(1);               // Job revision
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "NULL pointer detected for Job Revision");
    }

    // If include 'Design revision' in string
    if(checkBoxConfigName_DesignRevision->isChecked())
    {
        ptItem = listViewConfigName->topLevelItem(0);	// Pointing 'Design rev'

        if (ptItem)
        {
            if(strRecipeName.isEmpty() == false)
                strRecipeName += "_";
            strRecipeName += ptItem->text(1);               // Design revision
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, "NULL pointer detected for Design Revision");
    }

    // Check if no recipe name specified yet
    if(strRecipeName.isEmpty())
        strRecipeName = mOptionsPat->strProductName;

    // Add recipe version# X.Y
    int iRecipeVersion = qMax(1,mOptionsPat->iRecipeVersion);
    strRecipeName += "_Version" + QString::number(iRecipeVersion);
    strRecipeName += "." + QString::number(mOptionsPat->iRecipeBuild);

    strRecipeName += "_patman_config.json";

    // Build full path to Recipe file
    strConfigFile = strFolderName + strRecipeName;

    strConfigFile = QFileDialog::getSaveFileName(NULL,
                                                "Create Outlier Recipe file...",
                                                strConfigFile,
                                                "Recipe file(*.json)",
                                                NULL/*,
                                                QFileDialog::DontConfirmOverwrite*/);

    // If no file selected, ignore command.
    if(strConfigFile.isEmpty() == false)
    {
        // Make sure file extension is .json
        if(strConfigFile.endsWith(".json", Qt::CaseInsensitive) == false)
            strConfigFile += ".json";
    }
    return strConfigFile;
}

}
}
#endif
