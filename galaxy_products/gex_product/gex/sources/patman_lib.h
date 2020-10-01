#ifndef PATMAN_LIB_H
#define PATMAN_LIB_H

/*!
  *	\file patman_lib.h
  * \brief TODO
  * \author TODO
  */

#include "pat_defines.h"
#include "ui_tb_pat_limits_dialog.h"
#include "ui_tb_pat_dialog.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "pat_plugins.h"
#include "gex_report.h"
#include "gex_pat_processing.h"
#include "wafer_transform.h"
#include "gs_qa_dump.h"

class	CPatInfo;
class	CPatDefinition;
class   CPatPartFilter;
class	CParametricWaferMapNNR;
class   GexScriptEngine;

namespace GS
{
    namespace SE
    {
        class StatsEngine;
    }
}

#ifdef SOURCEFILE_PAT_LIB_CPP
    // Function prototypes
    int	patlib_isDistributionNormalLeftTailed(const CTest *ptTestCell);
    int	patlib_isDistributionNormalRightTailed(const CTest *ptTestCell);
    int	patlib_isDistributionNormal(const CTest *ptTestCell);
    int	patlib_isDistributionLogNormalLeftTailed(double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    int	patlib_isDistributionLogNormalRightTailed(double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    int	patlib_isDistributionLogNormal(double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    int	patlib_isDistributionMultiModal(const CTest *ptTestCell,double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    int	patlib_isDistributionClamped(const CTest *ptTestCell);
    int	patlib_isDistributionDualClamped(const CTest *ptTestCell,double lfQ1,double lfQ2,double lfQ3,double lfQ4);

    int	patlib_getDistributionNormalDirection(const CTest *ptTestCell);
    int	patlib_getDistributionLogNormalDirection(const CTest *ptTestCell);
    int	patlib_getDistributionClampedDirection(const CTest *ptTestCell);
    int patlib_getDistributionBiModalStats(const CTest *ptTestCell, double &lfMean1,
                                           double &lfSigma1, double &lfMean2,
                                           double &lfSigma2, double lfExponent,
                                           GS::Gex::PATPartFilter *lPartFilter = NULL);

    int patlib_GetDistributionID(QString strDistributionName);
    QString patlib_GetDistributionShortName(int iDistributionType);
    QString patlib_GetDistributionName(int iDistributionType);
    QString patlib_GetSeverityName(int iSeverityType);
    double patlib_GetOutlierFactor(const CPatInfo *lPatContext, CPatDefinition *ptPatDef,int iSite,bool bHeadFactor);
    bool patlib_IsDistributionSimilar(int iShape1,int iShape2);
    bool patlib_IsDistributionGaussian(CTest *ptTestCell, const CPatInfo *lPATContext);
    int patlib_GetDistributionType(CTest *ptTestCell,
                                   int lCategoryValueCount = 5,
                                   bool lAssumeIntegerCategory = true);
    bool	patlib_IsDistributionMatchingHistory(int iDistributionType_History,int iDistributionType_Current);
    void patlib_ComputeQuartilePercentage(const CTest *ptTestCell,double *lfQ1,double *lfQ2,double *lfQ3,double *lfQ4);
#else
    // Function prototypes
    extern int	patlib_isDistributionNormalLeftTailed(const CTest *ptTestCell);
    extern int	patlib_isDistributionNormalRightTailed(const CTest *ptTestCell);
    extern int	patlib_isDistributionNormal(const CTest *ptTestCell);
    extern int	patlib_isDistributionLogNormalLeftTailed(double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    extern int	patlib_isDistributionLogNormalRightTailed(double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    extern int	patlib_isDistributionLogNormal(double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    extern int	patlib_isDistributionMultiModal(CTest *ptTestCell,double lfQ1,double lfQ2,double lfQ3,double lfQ4);
    extern int	patlib_isDistributionClamped(const CTest *ptTestCell);
    extern int	patlib_isDistributionDualClamped(const CTest *ptTestCell,double lfQ1,double lfQ2,double lfQ3,double lfQ4);

    extern int	patlib_getDistributionNormalDirection(const CTest *ptTestCell);
    extern int	patlib_getDistributionLogNormalDirection(const CTest *ptTestCell);
    extern int	patlib_getDistributionBiModalStats(const CTest *ptTestCell, double &lfMean1,
                                                   double &lfSigma1, double &lfMean2,
                                                   double &lfSigma2, double lfExponent,
                                                   GS::Gex::PATPartFilter *lPartFilter = NULL);
    extern int	patlib_getDistributionClampedDirection(const CTest *ptTestCell);

    extern int patlib_GetDistributionID(QString strDistributionName);
    extern QString patlib_GetDistributionShortName(int iDistributionType);
    extern QString patlib_GetDistributionName(int iDistributionType);
    extern QString patlib_GetSeverityName(int iSeverityType);
    extern double patlib_GetOutlierFactor(const CPatInfo *lPatContext, CPatDefinition *ptPatDef,
                                          int iSite, bool bHeadFactor);
    extern bool patlib_IsDistributionSimilar(int iShape1,int iShape2);
    extern bool patlib_IsDistributionGaussian(CTest *ptTestCell,
                                              int lCategoryValueCount = 5,
                                              bool lAssumeIntegerCategory = true,
                                              int aMinConfThreshold = 2);
    extern int patlib_GetDistributionType(CTest *testCell,
                                          int categoryValueCount = 5,
                                          bool assumeIntegerCategory = true,
                                          int aMinConfidenceThreshold = 2,
                                          GS::Gex::PATPartFilter *partFilter = NULL);
    extern int GetMatchingPATDistribution(const QString& shape);
    extern int patlib_IdentifyDistribution(CTest *testCell,
                                           GS::SE::StatsEngine *statsEngine,
                                           int aMinConfThreshold,
                                           GS::Gex::PATPartFilter *partFilter);
    extern int patlib_IdentifyDistributionLegacy(CTest *testCell);
    extern bool	patlib_IsDistributionMatchingHistory(int iDistributionType_History,int iDistributionType_Current);
    extern void patlib_ComputeQuartilePercentage(const CTest *ptTestCell,double *lfQ1,double *lfQ2,double *lfQ3,double *lfQ4);
#endif

namespace GS
{
namespace Gex
{

#ifdef GCORE15334
    class PATProcessing;
#endif
    // Used only if GTM
    class	SiteTestResults;
}
}

typedef QMap<unsigned, CWaferMap*> CWafMapList;

// KLA/INF beginning of wafermap line keyword.
#define	KLA_ROW_DATA_TAB_STRING		"\t    RowData:"
#define	KLA_ROW_DATA_STRING			"RowData:"

// Used to hold info from KLA file required if generating SINF output file
class GexTbPatSinf
{
public:
    GexTbPatSinf();
    void clear(void);

    QString strDevice;
    QString	strLot;
    QString	strWafer;
    int		iFlatDirection;	// wafermap flat orientation
    QString	strBCEQ;		// List of Good bins
    QString	strNOTOUCHBC;	// List of No touch bins
    QString	strSKIPBC;		// List of Skip bins
    QString	strINKONLYBCBC;	// List of Ink only bins
    int		iRefPX;			// Reference X-die loc
    int		iRefPY;			// Reference Y-die loc
    int		iColRdc;		// Offset to reference X-die
    int		iRowRdc;		// Offset to reference Y-die
    int		iTotalDies;		// Total dies in MAP file
    int		iWaferAndPaddingCols;	// Total dies & padding dies over the X axis
    int		iWaferAndPaddingRows;	// Total dies & padding dies over the Y axis
    double	lfDieSiezX;		// Die size in X (in mm)
    double	lfDieSiezY;		// Die size in Y (in mm)
    int		iTotalMatchingMapDiesLoc;	// Holds total matching dies between STDF and MAP files
    int		iTotalMismatchingMapPassBin;
    int		iTotalMismatchingMapFailBin;
    int		iGoodPartsMap_DPAT_Failures;	// number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map.
    int		iGoodPartsMap_STDF_Missing;	// number of good die from the input wafer map that do not have matching STDF data.

    QMap<int,int> cMapBinCount_BeforePAT;	// Holds list of bin count of the input map Before PAT
    QMap<int,int> cMapBinCount_AfterPAT;	// Holds list of bin count of the input map After PAT

    QString	strNewWafermap;	// Wafermap ASCII array in SINF format, ready to use!
};


#ifdef GCORE15334

class GexTbPatDialog : public QDialog, public Ui::PatDialogBase
{
    Q_OBJECT

    //Q_PROPERTY(QTreeWidget mTreeWidgetDataFiles READ GetTreeWidgetDataFiles )

public:
    GexTbPatDialog(QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~GexTbPatDialog();

    void	clear();	// Resets variables

    QTreeWidget& GetTreeWidgetDataFiles() { return *treeWidgetDataFiles; }

    // Have Examinator script analyze file (so to build quartiles, etc...)
    // Note: Not available if running at Tester
//    bool	AnalyzeFile_AllBins(QString &strFileToProcess, QString &strErrorMessage);

    // Process file: Identify PAT and remove outliers (Post-Processing)
    // Note: Not available if running at Tester
    int		ProcessFile(bool bLaunchReportViewer, GS::Gex::PATProcessing &cFields, QString &strErrorMessage);

    GexTbPatSinf	m_cSinfInfo;			// Holds KLA/INF info that are used if requested to generate a SINF file

    // Error codes returned by Stdf functions
    enum MergeErrorCode
    {
        NoError,            // No error
        ProcessDone,        // No error: file processed
        NoFile,             // Input file doesn't exist!
        TimeStampError,     // Invalid STDF input file time stamp....
        ScriptError,        // Failed to start file analysis (script error)
        PatFileError,       // PAT Static Limits file is corrupted.
        ReportError,        // Failed to write report
        ReadError,          // Failed reading files to merge
        PatComputationError, // Failed computing PAT limits
        WriteError,         // Failed writing output file
        ReadCorrupted,      // Input file (in STDF format) didn't read well. Probably corrupted.
        WriteSTDF,          // Failed creating output STDF file
        IncompleteWafer,    // Not enough parts in wafer (< n% of GrossDie)
        PrePATProcessing,   // Failed to during Pre-PAT processing actions
        WaferExport         // Failed to export wafer into maps
    };

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    QWidget *createMapEntry();
    QTreeWidgetItem *AddGroup(int iGroup);
    // Get files in list view.
    void strGetFiles(QTreeWidgetItem *poItem, QStringList &strFiles);
    // Get Nth file in list view.
    void strGetFile(QString &lFile, int lGroupIdx = 0, int lFileIdx = 0);
    // NON-GUI Function to decide to attached or detach the window
    void ForceAttachWindow(bool bAttach=true, bool bToFront= true);

    // Public functions & variables shared with other modules
    bool	CreateTestNRR(int nGroupID,int nFileID,unsigned int &lTestNumber,int lPinmap,CParametricWaferMapNNR *ptNNR);

private:

    // Enables/Disqbles GUI buttons to avoid triggering multiple analysis while one is on going.
    void	enableGuiField(bool bEnable);

//    int		ConvertToSTDF(QString &strInputFile, QString &strErrorMessage);

    QString	getWafermapFormat(void);
    bool	getWafermapBinType(void);
    int		getGenerateStdfFile(void);

    // External PAT object (for handling custom PAT plugins)
    GexExternalPat          m_clExternalPat;
    GS::Gex::PATProcessing	m_cFields;                  // holds PAT processing options.
    // Holds FileID which dataset is in memory (only applies to multi-batch PAT processing GUI user only)
    // intitial value : -1
    int                 m_iReloadDataFileID;

    bool                bChildWindow;		// Flag to hold window status (detached or attached)
    long                m_iConfigFileLine;	 // Keeps track of config. file source line# in case of error to report...

protected:

    // Close event: if window is detached, this doesn close it but rather reparent it!
    void	closeEvent(QCloseEvent *e);

public slots:
    // Get the current Id of the file/entry in memory. Change at each PAT reload.
    int GetCurrentFileIdInMemory() { return m_iReloadDataFileID; }

    // Add file to the GUI listview.
    void AddFile(QString strFile, bool bClearList=true, bool bMerge = true);

    // Set recipe file (must be .csv format)
    // will call LoadPatConfigFile() if pGexTbTools
    QString SetRecipeFile(const QString &fullpath);

    // User forcing to reprocess/reload PAT result + data for a given file in GUI.
    QString	OnReProcessFile(int iFileID);
    // Attach / Detach window, toggle button.
    void	OnDetachWindow(void);
    // Called if user hits the ESCAPE key.
    void	reject(void);
    // Select file(s) to clean from outliers
    void	OnSelectFile();
    void    OnMoveFileUp();
    void    OnMoveFileDown();
    // Create trigger file(s) based on Dialog box settings
    void	OnCreateTriggerFile();
    // Create ZPAT exclusion map from file listed in GUI
    void	OnCreateZPAT_File();
    // Edit file properties: parts to list in report
    void	OnFileProperties(QTreeWidgetItem *poItem=0, int iColumn=0);

    // Select configuration file (with PAT static limits)
    // Note: Not available if running at Tester
    void	OnSelectConfigFile();
    // Select Composite/Exclusion map
    void	OnSelectCompositeExclusionFile();
    // ?
    void	OnProcessFile();
    // Edit the configuration file
    // Note: Not available if running at Tester
    void	OnEditConfigFile();
    void	OnRemoveFiles(void);
    // Removing ALL files form the list
    void	OnRemoveAll(void);
    void	OnWafermapFormat(void);
    // User dropping files into STDF list view.
    void	OnDropTestDataFiles(QDropEvent * e);

    // Auto adjust the column width to the content
    void	onItemChanged(QTreeWidgetItem*,int);
    void    OnAddGroup();
    void    retriveFieldsOptions(const QString &strGoup, const  QStringList& lGroupStdfFiles);
    void    showHideRemoveFile(QTreeWidgetItem*, QTreeWidgetItem*);
    bool    userMergeOrCreateGroups();
    // ?

public:
    QString retrieveMapFileName(QTreeWidgetItem *poItem);

    enum MapMergingError
    {
        MapMergingNoError,
        MapMergingFailedToConvert,
        MapMergingFailedToMerge,
        MapMergingErrorOpenFile

    };

private:
    void initComboBoxOutputWafermapFormat();

};

#endif

#endif
