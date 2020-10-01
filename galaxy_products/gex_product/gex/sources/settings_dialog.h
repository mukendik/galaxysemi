///////////////////////////////////////////////////////////
// All classes used for 'Settings'
///////////////////////////////////////////////////////////

#ifndef GEX_SETTINGS_H
#define GEX_SETTINGS_H
#include <qfiledialog.h>
#include <QTextStream>

#include "ui_settings_dialog.h"

#define ADV_ALL_TESTS	"All tests"
#define ADV_TEST_LIST	"Test list..."
#define ADV_TOP_N_FAILTEST	"Top N failing tests..."

class GexOneQueryWizardPage1;
class GexDatabaseEntry;

namespace GS
{
    namespace Gex
    {
        class CharacLineChartTemplate;
        class CharacBoxWhiskerTemplate;
        class CustomReportEnterpriseReportSection;
    }
}

class GexSettings : public QDialog, public Ui::settings_basedialog
{
    Q_OBJECT

public:
    GexSettings( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    void	ShowPage(void);
    void	UpdateBannerImages(int lProductID);
    void	enableInstantReportStandardCombos(bool bEnable);
    void	OnSelectReportTemplate(QString &strTemplateFile);
    bool	WriteScriptReportSettingsSection(FILE *hFile,bool bSetDrillParams,QString &strStartupPage);
    bool	WriteReportSettingsSection(FILE *hFile,bool bSetDrillParams,QString &strStartupPage);
    void	WriteReportSettingsSection_Stats(FILE *hFile);
    void	WriteReportSettingsSection_WaferMap(FILE *hFile);
    void	WriteReportSettingsSection_Histograms(FILE *hFile);
    bool	WriteReportSettingsSection_Advanced(FILE *hFile,bool bSetDrillParams);
    void    WriteReportSettingsSection_Limits(FILE *file);
    void	RefreshOutputFormat(int iOptionsOutputFormat=-1);
    void	OnDoInstantReport(void);

    void	FillListBox_TestStatistics(QComboBox *comboStats,bool bAllowDisabledItem);
    void	FillListBox_Histograms(QComboBox *comboHistogram,bool bAllowDisabledItem);
    void	FillListBox_Wafermap(QComboBox *comboWafer,bool bAllowDisabledItem);

    void	WriteBuildDrillSection(FILE *hFile,bool bWriteDrillSettings);

    int		iDrillType;					// Drilling type (wafer, histogram, etc....)
    int		iDrillSubType;				// Drilling sub-type (data over limits, over values, etc...)
    QString strDrillParameter;			// Drilling parameter (test name, etc...)
    QString	m_strTemplateFile;			// Holds path to Report Template to use (if selected under 'more reports')
    QStringList	m_strlAvailableTestingStages;	// Available testing stages
    QStringList	m_strlValidTestingStages;		// Valid testing stages for selected product
    QString		m_strFoundryTestingStage;		// Name of foundry testing stage

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void	dragEnterEvent(QDragEnterEvent *);
    void	dropEvent(QDropEvent *);

private:

    void	UpdateSkin(int lProductID);
    QString PickTestFromList(bool bAllowMultipleSelections, int nTestType, bool enableBlocksCompression=true);
    bool	WriteSettings_InteractiveReport(FILE *hFile,bool bSetDrillParams);

    void	loadSettings(const QString& strFileName);

public slots:
    void	reject(void);				// Called if user hits the ESCAPE key.
    void	OnChangeReportFormat(void);
    void	OnStatisticsValueChanged(void);
    void	OnWaferMapValueChanged(void);
    void	OnHistogramValueChanged(void);
    void	OnAdvancedValueChanged(void);
    void	OnImportScatterPairs(void);
    void	OnPickTestFromListStats(void);
    void	OnPickTestFromListWaferMap(void);
    void	OnPickTestFromListHistogram(void);
    void	OnPickTestFromListAdvanced(void);
    void	OnPickTestScatterX(void);
    void	OnPickTestScatterY(void);
    void	OnProductionReportOptions(void);
    void	OnProductionReportOptionsChanged(void);
    void	OnSelectReportTemplate(void);
    void	OnReportTemplateWizard(void);
    void	OnComboAdvancedReportChange(void);
  // Enable/Disable Test number list for Advanced report option
    void	OnComboAdvancedReportSettingsChange(void);
    void	OnComboHistogramTypeChange(void);
    void	OnComboHistogramChange(void);
    void	OnComboStatisticsChange(void);
    void	OnComboWaferMapChange(void);
    void	OnComboWaferMapTestChange(void);
    void	OnComboAdvancedTestsChanged(int);
    void    enableBuildReportButton(const QString &strText);
    void    disableBuildReportButton();

protected slots:

    void	onLoadSettings(void);
    void	onSaveSettings(void);

/////////////////////////////////////////////////////////
// CHARACTERIZATION REPORTS GUI
/////////////////////////////////////////////////////////
public:
    void	OnDoCharacterizationReport(void);

private:
    // General
    void    InitCharWizardPage();
    void    WriteSettings_CharacterizationReport(FILE *hFile);

protected slots:

    void    OnCharacWizardTemplateChanged(int);
    void    OnCharacWizardTestChanged(int);
    void    OnPickTestFromListCharWizard();
    void    OnCharWizardSettings();

private:

    GS::Gex::CharacLineChartTemplate *  mCharacLineChartTemplate;
    GS::Gex::CharacBoxWhiskerTemplate * mCharacBoxWhiskerTemplate;

/////////////////////////////////////////////////////////
// ENTERPRISE REPORTS GUI
/////////////////////////////////////////////////////////
public:
    void	OnDoSqlReport(void);
    void	UpdateFilters(bool bKeepSelectedFilters, bool bKeepValues=false);
    bool	IsEnterpriseSqlReport(void);

private:
    // General
    void	OnSqlReport_Startup(void);
    void	SqlReportSelected(int iSqlReport);
    /*!
     * \fn  OnSqlReport_LoadSettings
     */
    void OnSqlReport_LoadSettings(
        GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport = NULL);
    void	WriteSettings_SqlReport(FILE *hFile);
    void	SqlReport_Options_WriteSettings(QTextStream &hTemplateFile);
    int		m_SqlReportSelected;		// Holds SQL report type to create.

    // Production - UPH
    void	SqlReport_Prod_UPH_WriteSettings(QTextStream &hTemplateFile);

    // Production - Yield
    void	SqlReport_Prod_Yield_WriteSettings(QTextStream &hTemplateFile);

    // Production - Consolidated Yield
    void	SqlReport_Prod_ConsolidatedYield_WriteSettings(QTextStream &hTemplateFile);

    // Production - Yield Wizard
    void	SqlReport_Prod_YieldWizard_WriteSettings(QTextStream &hTemplateFile);

    // WYR - Standard
    void	SqlReport_Wyr_Standard_WriteSettings(QTextStream &hTemplateFile);

    // Genealogy - Yield vs Yield
    void	SqlReport_Genealogy_YieldVsYield_WriteSettings(QTextStream &hTemplateFile);

    // Genealogy - Yield vs Parameter
    void	SqlReport_Genealogy_YieldVsParameter_WriteSettings(QTextStream &hTemplateFile);

public slots:
    // General
    void	OnSqlReport_MissionChanged(void);
    void	OnSqlReport_MoreSettings(void);
    void	OnSqlReport_UpdateGui(void);

    // Production - UPH
    void	OnSqlReport_Prod_UPH(void);
    void	OnSqlReport_Prod_UPH_UpdateGui(void);

    // Production - Yield
    void	OnSqlReport_Prod_Yield(void);
    void	OnSqlReport_Prod_Yield_UpdateGui(void);
    void	OnSqlReport_Prod_Yield_PickFilter_Binning(void);

    // Production - Consolidated Yield
    void	OnSqlReport_Prod_ConsolidatedYield(void);
    void	OnSqlReport_Prod_ConsolidatedYield_UpdateGui(void);

    // Production - Yield Wizard
    void	OnSqlReport_Prod_YieldWizard(void);
    void	OnSqlReport_Prod_YieldWizard_UpdateGui(void);
    void	OnSqlReport_Prod_YieldWizard_NextPage(void);
    void	OnSqlReport_Prod_YieldWizard_PreviousPage(void);
    void	OnSqlReport_Prod_YieldWizard_AddSerie(void);
    void	OnSqlReport_Prod_YieldWizard_RemoveSerie(void);
    void	OnSqlReport_Prod_YieldWizard_ClearAllSeries(void);
    void	OnSqlReport_Prod_YieldWizard_EditSerie(void);
    void	OnSqlReport_Prod_YieldWizard_AddChartSplit(void);
    void	OnSqlReport_Prod_YieldWizard_RemoveChartSplit(void);
    void	OnSqlReport_Prod_YieldWizard_RemoveAllChartSplit(void);
    void	OnSqlReport_Prod_YieldWizard_AddLayerSplit(void);
    void	OnSqlReport_Prod_YieldWizard_RemoveLayerSplit(void);
    void	OnSqlReport_Prod_YieldWizard_RemoveAllLayerSplit(void);

    // WYR - Standard
    void	OnSqlReport_WYR_Standard(void);
    void	OnSqlReport_WYR_Standard_UpdateGui(void);
    void	OnSqlReport_WYR_Standard_PickFilter_Site();
    void	OnSqlReport_WYR_Standard_PickFilter_Year();
    void	OnSqlReport_WYR_Standard_PickFilter_Week();
    void	OnSqlReport_WYR_Standard_PickFilter(QString strQueryField, QComboBox *pQueryValue, QString strNarrowFilter_1, QComboBox *pNarrowValue_1, QString strNarrowFilter_2, QComboBox *pNarrowValue_2);

    // Genealogy - Yield vs Yield
    void	OnSqlReport_Genealogy_YieldVsYield(void);
    void	OnSqlReport_Genealogy_YieldVsYield_UpdateGui(void);
    void	OnSqlReport_Genealogy_YieldVsYield_OnPickBinlist_Xaxis();
    void	OnSqlReport_Genealogy_YieldVsYield_OnPickBinlist_Yaxis();
    void	OnSqlReport_Genealogy_YieldVsYield_OnPickProduct();

    // Genealogy - Yield vs Parameter
    void	OnSqlReport_Genealogy_YieldVsParameter(void);
    void	OnSqlReport_Genealogy_YieldVsParameter_UpdateGui(void);
    void	OnSqlReport_Genealogy_YieldVsParameter_OnPickParameter_Xaxis();
    void	OnSqlReport_Genealogy_YieldVsParameter_OnPickBinlist_Yaxis();
    void	OnSqlReport_Genealogy_YieldVsParameter_OnPickProduct();
};

#endif
