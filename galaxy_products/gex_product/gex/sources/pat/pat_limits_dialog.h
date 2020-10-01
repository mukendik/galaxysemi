#ifdef GCORE15334

#ifndef PAT_LIMITS_DIALOG_H
#define PAT_LIMITS_DIALOG_H

#include "ui_tb_pat_limits_dialog.h"
#include "pat_rules.h"
#include "pat_plugins.h"
class COptionsPat;
class PATOptionReticle;

namespace GS
{
namespace Gex
{

//! \class Diaog box to Create the Outlier Config file.
class PatLimitsDialog : public QDialog, public Ui::PatLimitsDialogBase
{
    Q_OBJECT

public:
    //! \brief GCORE-2449
    //! \brief Current selected algo
    static const QString mPropNameCurrentAlgo;
    //! \brief Current N and T values
    QMap<QString, QPair<double,double> > mHeadTailValuesPerAlgo;

    //! \brief Create Recipe File - constructor
    PatLimitsDialog(COptionsPat *OptionsPat, QWidget* parent = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~PatLimitsDialog();

    Q_INVOKABLE void	setEditMode(bool bCreateMode=true,bool bShowFileHeaderDetails=true, bool bWizardGui=false);
    Q_INVOKABLE bool	isValidData(bool bShowMessage=true);
    /// \brief return the number of mask in tableWidgetMasks
    //Q_INVOKABLE int     getMasksSize() const; // unimplemented ?
    Q_INVOKABLE QString BuildRecipeFileName();

private:
    QString	strConfigFile;	// Outlier Configuration file created.
    bool	bCreate;		// 'false' if the dialog box doesn't save its fields when 'Ok' is prerssed (then caller handles the save function)
    Q_INVOKABLE void RefreshGUI(void);
    Q_INVOKABLE void CopyUIData(void);
    Q_INVOKABLE void FillCustomPatLibList(void);
    Q_INVOKABLE void ShowOnLinHelpPage(const QString &strMessage);
    Q_INVOKABLE void RedrawDistribution();


    void RefreshMaskRules();

    //		NNR rules table management
    void    OnNNRListUpdated();
    bool	NNRRuleFillItem(int row);

    //		IDDQ-DELTA rules table management
    void    OnIDDQDeltaListUpdated();
    bool	IDDQRuleFillItem(int row);

    //		GDBN rules table management
    void    OnGdbnListUpdated();
    bool	GdbnRuleFillItem(int row);

    //		Clustering rules table management
    void    OnClusteringListUpdated();
    bool	ClusteringRuleFillItem(int row);

    //		Mask rules
    bool	MaskRule_ReadGUI(int iRow,CMask_Rule &cRule);
    bool	MaskRule_FillGUI(int iRow,CMask_Rule *ptRule);

    //      Reticle rules
    void    InitReticleRuleslist();
    void    ReticleRuleFillItem(int row);

    int                     m_iDistribution;	// Distribution type selected.
    GexExternalPat *        m_pExternalPat;		// External PAT object (for handling custom PAT plugins)
    QList<GexPatPlugin_ID*>	m_pPluginList;			// List of available plugins
    GexPatPlugin_ID *       m_pCurrentPlugin;		// Plugin currently selected
    COptionsPat *           mOptionsPat;

public slots:
    void	OnOk();
    void	OnEnableLimits();
    void	OnSetTotalFlows();
    void	OnIgnoreHighCpk();
    void	OnHistoricalCpk();
    void	OnRuleSet();
    void	OnCustomAlgorithm();
    void	OnDistribution();
    //! \brief On Algorithm used for computing PAT limits changed
    void	OnAlgorithm();
    void	OnIncrementVersion();
    void	OnEnablePat();
    void	OnHeadSlider();
    void	OnTailSlider();
    void	OnHeadFactor();
    void	OnTailFactor();
    void	OnComboSeverityLimits();
//    void	OnToggleReticle();

    // Reticle rules
    void    OnReticleBinChanged();
    void    OnReticleColorChanged();
    void    OnAddReticleRule();
    void    OnEditReticleRule();
    void    OnRemoveReticleRule();
    void    OnDuplicateReticleRule();
    void    OnMoveUpReticleRule();
    void    OnMoveDownReticleRule();
    void    OnReticleListUpdated();
    void    OnReticleSourceChanged();

    //    void OnReticleSizeSource();
//    void OnUpdateInkingArea();
//    void OnUpdateInkingParameters();
//    void	OnReticleSize();
//    void	OnBadReticleColor(const QColor&);
    void	saveDefaultPatmanConfigFile();

    // NNR rules
    void    OnNNRBinChanged();
    void    OnNNRColorChanged();
    void	OnAddNNR_Rule();
    void	OnDuplicateNNR_Rule();
    void	OnEditNNR_Rule();
    void	OnMoveUpNNR_Rule();
    void	OnMoveDownNNR_Rule();
    void	OnRemoveNNR_Rule();

    // IDDQ-Delta rules
    void    OnIDDQBinChanged();
    void    OnIDDQColorChanged();
    void	OnAddIDDQ_Delta_Rule();
    void	OnDuplicateIDDQ_Delta_Rule();
    void	OnEditIDDQ_Delta_Rule();
    void	OnMoveUpIDDQ_Delta_Rule();
    void	OnMoveDownIDDQ_Delta_Rule();
    void	OnRemoveIDDQ_Delta_Rule();

    // GDBN rules
    void    OnGdbnBinChanged();
    void    OnGdbnColorChanged();
    void	OnAddGdbnRule();
    void	OnDuplicateGdbnRule();
    void	OnEditGdbnRule();
    void	OnMoveUpGdbnRule();
    void	OnMoveDownGdbnRule();
    void	OnRemoveGdbnRule();

    // Clustering rules
    void    OnClusteringBinChanged();
    void    OnClusteringColorChanged();
    void	OnAddClusteringRule();
    void	OnDuplicateClusteringRule();
    void	OnEditClusteringRule();
    void	OnMoveUpClusteringRule();
    void	OnMoveDownClusteringRule();
    void	OnRemoveClusteringRule();

    // Masks definitions
    void	OnAddMask();
    void	OnDuplicateMask();
    void	OnEditMask();
    void	OnRemoveMask();

    // Rules precedence / Ordering
    void	OnRulePrecedenceUp();
    void	OnRulePrecedenceDown();

    // FT PAT
    void    OnBaseLineChanged(int lNewBaseline);
    void    OnTuningSamplesChanged(int lNewTuningSamples);
    void    OnTuningFrequencyToggled(bool lChecked);
    void	OnComboProcessChange();
    void	OnMinimumOutliersToFailPart();
    void	OnBaseLineMaxOutliers();
    void	OnMailingList();
    void	OnPatPluginChanged();
    void    OnMVPATNearDistanceChanged(double lValue);
    void    OnMVPATMediumDistanceChanged(double lValue);
    void    OnMVPATFarDistanceChanged(double lValue);
};

}
}
#endif // PAT_LIMITS_DIALOG_H
#endif
