///////////////////////////////////////////////////////////
// All classes used for 'Analyze ONE file'
///////////////////////////////////////////////////////////

#ifndef ONEQUERY_WIZARD_H
#define ONEQUERY_WIZARD_H
#include "ui_db_single_query_dialog.h"

#include <qdatetime.h>
#include <stdio.h>

#include "gex_database_filter.h"

// Forward declarations
class GexDatabaseQuery;		// Extern class in db_transactions.cpp
class GexDatabaseFilter;
class GexDatabaseEntry;

class GexOneQueryWizardPage1 : public QDialog, public Ui::onequery_basedialog
{
    Q_OBJECT

public:
    GexOneQueryWizardPage1( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0, bool bUsedForPurge=false );
    ~GexOneQueryWizardPage1();

    enum NextAction
    {
        BUILDREPORT,
        EXPORTTOCSV
    };
    enum eDialogMode
    {
        eCreate,
        eEdit,
        eHouseKeeping,
        ePurge,
        ePATRecipe
    };
    enum eExtractionMode
    {
        eRawData,
        eConsolidatedData,
        e1outOfNsamples
    };

    void		ShowPage(void);
    void		PopupSkin(int iDialogMode,bool bPopup);
    void		WriteProcessFilesSection(FILE *hFile, bool bGenSTDFList = false);
    void		UpdateGui(void);

    //
    QStringList GetTestConditionsLevel() const;

    // Return the Query GUI fields into a Query structure.
    void		GetQueryFields(GexDatabaseQuery &cQuery);
    QString		GetFilterXmlLine(QComboBox *pFilter,QComboBox *pFilterString);
    QStringList GetFiltersXmlLines(void);
    bool		IsSqlDatabase() { return m_bSqlDatabase; }
    QString		SqlPickParameterList(bool bParametricOnly);
    QString		SqlPickParameterList(bool bParametricOnly, bool bMultiSelection, const QString & strDataType, const QString & strProductName);
    QString		SqlPickBinningList(bool bMultiSelection, bool bSoftBin=false);
    QString		SqlPickBinningList(bool bMultiSelection, QString strDataType, QString strProductName, bool bSoftBin=false);
    void		SqlPickProductList(QStringList & strlProducts, bool bMultiSelection, QString & strDataType, QString strValue="");
    void		SqlPickProductList_Genealogy(QStringList & strlProducts, bool bAllTestingStages, bool bMultiSelection, QString strDataType="");
    void		OnDatabaseChanged(bool bKeepValues=false);
    void		ResetRdbPluginHandles();
    void		GetPluginOptionsString(QString & strOptionsString, GexDatabaseEntry *pDatabaseEntry=NULL);
    void		SetPluginOptionsString(QString & strOptionsString, GexDatabaseEntry *pDatabaseEntry=NULL);
    void        UpdateExtractionModeUi(int extractionMode = -1);

    /**
     * \name FillDataTab
     * \brief fill the data tab with the appropriate files and options.
     *
     * \param dataDescription, a string that contains the description of the section
     */
    bool FillDataTab(const QString& dataDescription);

public slots:

    void	reject(void);		// Called if user hits the ESCAPE key.
    // Calendar related slots
    void	OnFromDateCalendar(void);
    void	OnToDateCalendar(void);
    void	OnFromDateSpinWheel(const QDate&);
    void	OnToDateSpinWheel(const QDate&);
    void	OnFromTimeSpinWheel(const QTime&);
    void	OnToTimeSpinWheel(const QTime&);

    // Other slots
    void	OnDatasetName(void);
    void    OnOfflineSwitch(void);
    void	OnSelectDatabase(void);
    void	OnTimePeriod(void);
    void	OnMoreFilters(void);
    void	OnStandardFilterChange(const QString&);
    void	OnTCFilterChange(const QString&);
    void	OnComboProcessChange(void);
    void	OnConsolidatedChange(void);	// consolidated or not
    void	OnCreateMapTests(void);
    void	OnMapTests(void);
    void	OnPickFilter1(void);
    void	OnPickFilter2(void);
    void	OnPickFilter3(void);
    void	OnPickFilter4(void);
    void	OnPickFilter5(void);
    void	OnPickFilter6(void);
    void	OnPickFilter7(void);
    void	OnPickFilter8(void);
    void	OnPickTCFilter1();
    void	OnPickTCFilter2();
    void	OnPickTCFilter3();
    void	OnPickTCFilter4();
    void	OnPickTCFilter5();
    void	OnPickSplitFields();
    void	OnPickTestCondtions();
    void	OnPickParameterList();
    void	OnUpdateFilters();
    void    OnTabChanged(int);
    void	PickFilterFromLiveList(const GexDatabaseFilter &dbFilter,
                                   QComboBox *pQueryValue = NULL,
                                   bool lStandardFilter = true);
    void	OnAllFilters();
    QString ExportQueryToCSV();
    QString ExportToCSVConditionFormat();
    void    OnExecuteAction();
    void    setProhibatedList(const QStringList &oProhibitedName);
    void    checkGroupName(const QString &);

private slots:
    void OnKeepHierarchyUpToDateSelected(bool aIsSelected);
    void OnRebuildHierarchyOnDemandSelected(bool aIsSelected);
    void OnConditionsLevelChanged();
    void OnNullConditionDetected(QString aCondtion);


signals:
    void sExecuteAction();

private:

    bool    mIsHierarchyUpToDate;
    bool	bPopupMode;			// true if shows as a popup (when Compare queries)
    int		m_iDialogMode;		// Holds how this dialog box is used: to create a dabase, multi-databased, or HouseKeeping.
    int		iCalendarOffset;	// Hold Calendar Hights (if visible)
    bool	bMoreFilters;
    QDate	From;
    QDate	To;
    QTime	FromTime;
    QTime	ToTime;
    bool	m_bSqlDatabase;
    QWidget	*m_pRdbOptionsWidget;
    bool	m_bRdbOptionsInitialized;
    QStringList mNullTestConditions;

    void	UpdateSkin(int lProductID);
    void    UpdateConditionsRender();

    void    ConnectTCFilterControls(bool lConnect);
    void    FillDatabaseFilter(GexDatabaseFilter& dbFilter,
                               const QString &queryFieldName = QString());
    void	WriteFilterScriptLine(GexDatabaseEntry	*pDatabaseEntry,
                                  FILE *hFile,
                                  const QString& strFilterName,
                                  QString strFilterValue);
    void	WriteSplitFieldScriptLine(GexDatabaseEntry	*pDatabaseEntry,
                                      FILE * hFile,
                                      const QStringList &splitFields);
    void	setQueryFilter(GexDatabaseEntry	*pDatabaseEntry, GexDatabaseQuery &cQuery,
                           QString &strFilterName, QString &strFilterValue);

    void    FilterChanged(const QString&);
    void	UpdateFromToFields(bool);
    void	UpdateFilters(GexDatabaseEntry *pDatabaseEntry, bool bKeepSelectedFilters=true, bool bKeepValues=false);
    void	UpdateTestConditions(bool lKeepSelectedFilters = true, bool lKeepValues = true);


    void    SetSelectedTestConditions(const QStringList& selection);
    void    SetSynchronizedTestConditions(bool synchronized);
    void    InitNextActionLabels();
    void    InitExtractionModeLabels();
    void    InitComboBoxExtractionMode();
    void    InitNextActionFrame();
    GexDatabaseEntry* GetSelectedDatabase();
    void    BuildHierarchy();


    QString     m_strCSVDestinationPath;
    QStringList m_oProhibitedName;
    QStringList mSplitFieldsAvailable;

    QStringList mTestConditionsAvailable;
    QStringList mTestConditionsLevel;
    QStringList mTestConditionHierarchy;
    QMap<NextAction, QString>      mNextActionLabels;          ///<  List of action labels
    QMap<eExtractionMode, QString> mExtractionModeLabels;      ///<  List of extraction mode labels

    bool        mSynchronizedTestCond;

    bool        mPurgeDb;

};
#endif
