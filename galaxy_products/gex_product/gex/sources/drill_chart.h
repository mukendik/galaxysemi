/******************************************************************************!
 * \file drill_chart.h
 * \brief All classes used for 'Interactive Chart navigation
 ******************************************************************************/
#ifndef DRILL_CHART_H
#define DRILL_CHART_H

#include "QTitleFor.h"

#include "ui_drill_chart_dialog.h"
#include "ui_drill_custom_parameter_dialog.h"
//#include "wizards_handler.h"
#include <QList>
#include <qbitmap.h>
#include <QMap>
#include <qxchartnamesplitsextractor.h>


// Tab Offset 0: It is the 'Chart' window that is visible
//#define GEX_INTERACTIVE_CHART_TAB      0
// Tab Offset 0: It is the 'MultiChart' window that is visible
//#define GEX_INTERACTIVE_MULTICHART_TAB 1
// Tab Offset 1: It is the 'Table' window that is visible
//#define GEX_INTERACTIVE_TABLE_TAB      2

// Forward declaration
class CGexGroupOfFiles;
class CGexFileInGroup;
class CTest;
class CReportOptions;
class CGexChartOverlays;
class CGexSingleChart;

namespace Gex
{
    class InteractiveChartsWizard;
    class WizardHandler;
}


/******************************************************************************!
 * \class GexWizardChart
 ******************************************************************************/
class GexWizardChart :
    public GS::Gex::QTitleFor< QDialog >, public Ui::drill_chart_basedialog
{
    Q_OBJECT

public:
    /*!
   * \fn GexWizardChart
   */
    GexWizardChart(QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0);
    /*!
   * \fn ~GexWizardChart
   */
    ~GexWizardChart();
    /*!
   * \fn ShowChart
   */
    void ShowChart(QString strLink = QString());
    /*!
   * \fn clear
   */
    void clear();
    /*!
   * \fn resetButtonStatus
   */
    void resetButtonStatus();
    /*!
   * \fn isDataAvailable
   */
    bool isDataAvailable();
    /*!
   * \fn strPropertiesPath
   */
    QString strPropertiesPath;
    /*!
   * \fn ShowPage
   */
    void ShowPage();
    /*!
   * \fn addChart
   * \brief Called from the scripting language
   */
    void addChart(bool         bUpdateGUI,
                  bool         bClear,
                  unsigned int iTestX,
                  int          iPinmapX,
                  QString      strNameX,
                  int          iGroupX = -1,
                  unsigned int iTestY = 0,
                  int          iPinmapY = -1,
                  QString      strNameY = "",
                  int          iGroupY = -1,
                  bool         = true);

    bool formatRequestDBName(QString& aChartName);
    bool buildTreeLayerHeader(const QString &aChartName);
    /*!
   * \fn exitInsertMultiLayers
   */
    void exitInsertMultiLayers(unsigned int lTestNumber,
                               int          lPinmapIndex,
                               QString      strParameterName);
    /*!
   * \fn OnHistogram
   */
    void OnHistogram();
    /*!
   * \fn OnTrend
   */
    void OnTrend();
    /*!
   * \fn OnScatter
   */
    void OnScatter();
    /*!
   * \fn OnBoxPlot
   */
    void OnBoxPlot();
    /*!
   * \fn OnProbabilityPlot
   */
    void OnProbabilityPlot();
    /*!
   * \fn pluginShowChart
   * \brief Called by plugin when want to display interactive chart
   */
    void pluginShowChart(int iRendering);

    //bool focusNextPrevChild(bool next);
    bool mTabFocusChartType;
    void OnKeyUpDown();
    bool eventFilter(QObject *obj, QEvent *event);
    /*!
   * \fn keyPressEvent
   */
    void keyPressEvent(QKeyEvent* e);
//    /*!
//   * \fn keyReleaseEvent
//   */
//    virtual void keyReleaseEvent(QKeyEvent* e);
    /*!
   * \fn currentTestCellX
   */
    CTest* currentTestCellX() const { return ptTestCell; }
    /*!
   * \fn currentTestCellY
   */
    CTest* currentTestCellY() const { return ptTestCellY; }
    /*!
   * \fn chartReportOptions
   */
    CReportOptions* chartReportOptions() const { return tmpReportOptions; }

    CGexChartOverlays*	getChartInfo() const { return mChartsInfo;}

    void SetEnabledCharts   (bool enabled);
    void initTestCell       (unsigned int lTestNumber, QString lTestName, int lPinmapIndex);
    void setWizardHandler   (Gex::WizardHandler* lWizardHandler) { mWizardHandler = lWizardHandler; }
    Gex::WizardHandler* getWizardHandler() { return mWizardHandler;}

    void                    AddNewOFRSection                    (const QString &chartTypeName);
    void                    AddToAnExistingOFRSection   (int indexSection, const QString& chartTypeName);
    void                    AddToLastUpdatedOFRSection  (const QString &chartTypeName);

    void    focusInEvent( QFocusEvent* );

    /**
     * @brief toJson -  serialize to json format
     * @param descriptionOut - JsonObject to be fill
     * @param chartTypeName - name of the Json section
     */
    void    toJson(QJsonObject& descriptionOut, const QString& chartTypeName);

    int mIndex;

private:

   //QList<CGexSingleChart*>  mSettings;
   CGexChartOverlays*       mChartsInfo;
   Gex::WizardHandler*      mWizardHandler;
   QxChartNameSplitsExtractor mQxChartNameSpliteredExtractor;



   // -- Tools buttons elements
   void    InitToolsButtons     ();
   void    InitSettingsButton   (QToolButton *button);
   void    InitInteractiveView  (QToolButton* button);

    enum { E_ICON_SELECT = 0, E_ICON_MOVE, E_ICON_ZOOM, E_ICON_SPLIT};
    QIcon mSelectIcon;
    QIcon mMoveIcon;
    QIcon mZoomIcon;
    QIcon mSplitDataIcon;

    QList<QIcon> mIconsMouse;

    QIcon mResetViewport;
    QIcon mKeepViewport;

    QAction* mAnchorAction;
    /*QAction* mSelectAction;
    QAction* mZoomAction;
    QAction* mMoveAction;*/
    QAction* mSelectZone;

    QAction* mPickTestAction;
    QAction* mPickTestYAction;
    QAction* mFilterTestAction;
    QAction* mFilterOderIDAction;

    /*!
   * \var pGroup
   */
    CGexGroupOfFiles* pGroup;
    /*!
   * \var pFile
   */
    CGexFileInGroup* pFile;
    /*!
   * \var pFileY
   * \brief Used for the scatter plot only
   */
    CGexFileInGroup* pFileY;
    /*!
   * \var ptTestCell
   * \brief Pointer to test cell to receive STDF info
   *        (in scatter plot: test in X axis)
   */
    CTest* ptTestCell;
    /*!
   * \var ptTestCellY
   * \brief Scatter plot only: Pointer to test cell plotted in Y
   */
    CTest* ptTestCellY;
    /*!
   * \var m_pPrevTestX
   */
    CTest* m_pPrevTestX;
    /*!
   * \var m_pPrevTestY
   */
    CTest* m_pPrevTestY;
    /*!
   * \var lTestNumber
   */
    unsigned long lTestNumber;
    /*!
   * \var lPinmapIndex
   */
    long lPinmapIndex;
    /*!
   * \var mMultipleTest;
   */
    bool mMultipleTest;
    /*!
   * \var m_eChartType
   * \brief Keeps track of rendering: Histogram, trend, etc.
   */
    GexAbstractChart::chartType m_eChartType;
    /*!
   * \var m_bSupportBoxWhisker
   * \brief True if bowwhisker is supported for Correlation chart
   */
    bool m_bSupportBoxWhisker;
    /*!
   * \var m_nMultiChartViewport
   * \brief Holds the viewport used for multicharts
   *        (over limits, over data, adaptive)
   */
    int m_nMultiChartViewport;
    /*!
   * \var m_iHistoViewport
   * \brief GEX_ADV_HISTOGRAM_OVERLIMITS, etc.
   */
    int m_iHistoViewport;
    /*!
   * \var m_iTrendViewport
   * \brief GEX_ADV_TREND_OVERLIMITS, etc.
   */
    int m_iTrendViewport;
    /*!
   * \var m_iScatterViewport
   * \brief GEX_ADV_CORR_OVERLIMITS, etc.
   */
    int m_iScatterViewport;
    /*!
   * \var m_iBoxPlotViewport
   * \brief GEX_ADV_CORR_OVERLIMITS, etc.
   */
    int m_iBoxPlotViewport;
    /*!
   * \var m_iProbabilityViewport
   */
    int m_iProbabilityViewport;
    /*!
   * \var m_bExternalAdmin
   * \brief 'true' if admin (erase layers, set markers, ...) is external
   *        (eg: plugin)
   */
    bool m_bExternalAdmin;
    /*!
   * \var tmpReportOptions
   * \brief Holds a copy of the default options
   *        (so we can locally overwrite them as needed)
   */
    CReportOptions* tmpReportOptions;
    /*!
   * \var pPixmapLayers
   * \brief List of Pixmap (colors) used to plot charts
   */
    QList<QPixmap> pPixmapLayers;
    /*!
   * \var iPrevShiftX
   */
    int iPrevShiftX;
    /*!
   * \var iPrevShiftY
   */
    int iPrevShiftY;
    /*!
   * \var pTabHistoScales
   */
    QWidget* pTabHistoScales;
    /*!
   * \var pTabLinesStyle
   */
    QWidget* pTabLinesStyle;
    /*!
   * \var m_KeyboardStatus
   * \brief Holds keyboard state (SHIFT, CTRL, etc.)
   */
    long m_KeyboardStatus;

    /*!
   * \fn clearChart
   */
    void clearChart(int iChartID = -1);
    /*!
   * \fn addChart 1
   */
    void addChart(bool bUpdateGUI, bool bClear, CGexSingleChart* pLayer, bool newChart);
    /*!
   * \fn addChart 2
   */
    void addChart(bool   bUpdateGUI,
                  bool   bClear,
                  CTest* ptTestCellX,
                  CTest* ptTestCellY = NULL,
                  int    iGroupX = -1,
                  int    iGroupY = -1);

   void ConnectAllElements();

    /*!
   * \fn GetStyle
   */
    void GetStyle();
    /*!
   * \fn ListRelevantRawData
   */
    void ListRelevantRawData();
    /*!
   * \fn CheckForDataAvailability
   */
    bool CheckForDataAvailability();
    /*!
   * \fn UpdateSkin
   */
    void UpdateSkin();
    /*!
   * \fn CopyDefaultStyle
   */
    void CopyDefaultStyle();
    /*!
   * \fn OnShowPartOnHistogram
   */
    void OnShowPartOnHistogram(int row, int col);
    /*!
   * \fn OnRemoveChart
   */
    void OnRemoveChart(bool bRemoveAll);
    /*!
   * \fn SelectWaferMapDiesForParameterRange
   * \brief Wafermap drill-down: show all dies where parameter within
   *        given range
   */
    void SelectWaferMapDiesForParameterRange(double lfLow, double lfHigh);


    /*!
   * \fn findFirstValidTestCell
   */
    CTest* findFirstValidTestCell();
    /*!
   * \fn findFirstValidTestCellY
   */
    CTest* findFirstValidTestCellY();
    /*!
     * \fn findLastValidTestCell
     */
    CTest *	findLastValidTestCell();
    /*!
     * \fn findLastValidTestCellY
     */
    CTest *	findLastValidTestCellY();

    /*!
     * \fn IsFirstTestCell
     */
    bool      IsFirstTestCell(const CTest * lTest);

    /*!
     * \fn IsFirstTestCellY
     */
    bool      IsFirstTestCellY(const CTest * lTest);

    /*!
     * \fn IsLastTestCell
     */
    bool      IsLastTestCell(const CTest * lTest);

    /*!
     * \fn IsLastTestCellY
     */
    bool      IsLastTestCellY(const CTest * lTest);


    /**
     * @brief ApplyNewSettings. Replace the current settings by those in param
     * @param newSettings :  list of the new settings
     */
    void ApplyNewSettings( const CGexChartOverlays *chartsInfo);

public slots:
  /**
   * @brief react on a tab renamed
   * @param widget the widget inside the renamed tab
   * @param text the text used to rename the tab
   */
  void OnTabRenamed( QWidget *widget, const QString &text );

    /*!
   * \fn PlotRelevantChart
   */
    void PlotRelevantChart();
    /*!
   * \fn OnDetachWindow
   */
   // void OnDetachWindow();
    /*!
   * \fn OnChangeTestSortOrder
   */
    void OnChangeTestSortOrder();
    /*!
   * \fn OnChangeStyle
   */
    void OnChangeStyle();
    /*!
     * \fn OnCheckBoxStack
     */
    void OnCheckBoxStack();
    /*!
   * \fn OnZoom
   */
    void OnZoom();
    /*!
   * \fn onSelectZone
   */
    void onSelectZone();
    /*!
   * \fn OnDrag
   */
    void OnDrag();
    /*!
   * \fn OnSelect
   */
    void OnSelect();
    /*!
   * \fn OnTabChange
   */
    void OnTabChange(int nCurrentTab = -1);
    /*!
   * \fn OnChangeDatasetWindow
   */
    void OnChangeDatasetWindow(const QString&);
    /*!
   * \fn onChangedLineColor
   */
    void onChangedLineColor(const QColor&);
    /*!
   * \fn OnChangeChartLayer
   */
    void OnChangeChartLayer(const QString&);
    /*!
   * \fn OnChangeLineStyle
   */
    void OnChangeLineStyle(const QString&);

    void  UpdateGUIGlobal      (const CGexChartOverlays *overlay);
    void  UpdateGUIByChart     (const CGexSingleChart* chart);

    /*!
   * \fn UpdateChartsList
   */
    void UpdateChartsList();
    /*!
   * \fn OnPreviousTest
   */
    void OnPreviousTest();
    /*!
   * \fn OnPreviousTestY
   */
    void OnPreviousTestY();
    /*!
   * \fn OnNextTest
   */
    void OnNextTest();
    /*!
   * \fn OnNextTestY
   */
    void OnNextTestY();
    /*!
   * \fn OnPickTest
   */
    void OnPickTest();
    /*!
   * \fn OnPickTestY
   */
    void OnPickTestY();
    /*!
   * \fn OnChartType
   */
    void OnChartType(int);
    /*!
   * \fn OnExportData
   */
    void OnExportData();
    /*!
   * \fn OnExportExcelClipboard
   */
    void OnExportExcelClipboard();
    /*!
   * \fn OnExportChartStats
   */
    void OnExportChartStats();
    /*!
   * \fn OnHistoBarSize
   */
    void OnHistoBarSize(int iTotalBars);
    /*!
   * \fn OnScaleType
   */
    void OnScaleTypeIndexChanged(int);
    void OnScaleType(const QString&);
    /*!
   * \fn OnCorrelationMode
   */
    void OnCorrelationMode(bool);
    /*!
   * \fn OnWhiskerOrientation
   */
    void OnWhiskerOrientation(int);
    /*!
   * \fn OnAddChart
   */
    void OnAddChart();
    /*!
   * \fn OnCustomParameter
   */
    void OnCustomParameter();
    /*!
   * \fn OnRemoveChart
   */
    void OnRemoveChart();
    /*!
   * \fn OnChartProperties
   */
    void OnChartProperties();
    /*!
   * \fn OnInteractiveTables
   */
    void OnInteractiveTables();
    /*!
   * \fn OnInteractiveWafermap
   */
    void OnInteractiveWafermap();
    /*!
   * \fn OnInteractiveCharts
   */
    void OnInteractiveCharts();
    /*!
   * \fn OnTableEdited
   */
    void OnTableEdited(int row, int col);
    /*!
   * \fn OnContextualMenu
   * \brief Contextual menu on Layer list
   */
    void OnContextualMenu(const QPoint&);
    /*!
   * \fn OnTableContextualMenu
   * \brief Contextual menu on table
   */
    void OnTableContextualMenu(const QPoint&);
    /*!
   * \fn OnEditLayer
   */
    void OnEditLayer(QTreeWidgetItem* pSelection, int nColumn);
    /*!
   * \fn onChangedChartBackgroundColor
   */
    void onChangedChartBackgroundColor(const QColor&);
    /*!
   * \fn OnRemoveTableDataPoints
   */
    void OnRemoveTableDataPoints();
    /*!
   * \fn resetBarsToDefault
   */
    void resetBarsToDefault(bool);
    /*!
   * \fn useCustomHistoBars
   */
    void useCustomHistoBars(bool);

    void SaveSettingsAsDefault          ();
    void ApplySettingsToAllOtherCharts  ();
    void RetrieveGUISettings            ();

protected slots:
    /*!
   * \fn onCurrentDieSelected
   */
    void onCurrentDieSelected();
    /*!
   * \fn onDeviceResults
   */
    void onDeviceResults(long lDeviceID, int x=-32768, int y=-32768);
    /*!
   * \fn onEditChartStyles
   */
    void onEditChartStyles();
    /*!
   * \fn onEditChartMarkers
   */
    void onEditChartMarkers();
    /*!
   * \fn onSelectWafermapRange
   */
    void onSelectWafermapRange(double dLowLimit, double dHighLimit);
    /*!
   * \fn onRemoveDatapointRange
   */
    void onRemoveDatapointRange(double dLowLimit,
                                double dHighLimit,
                                bool   bRemoveAssociatedParts);
    /*!
   * \fn onRemoveDatapointHigher
   */
    void onRemoveDatapointHigher(double dLimit, bool bRemoveAssociatedParts);
    /*!
   * \fn onRemoveDatapointLower
   */
    void onRemoveDatapointLower(double dLimit, bool bRemoveAssociatedParts);
    /*!
   * \fn onRemoveIQR
   */
    void onRemoveIQR(double dValue, bool bRemoveAssociatedParts);
    /*!
   * \fn onRemoveNSigma
   */
    void onRemoveNSigma(double dValue, bool bRemoveAssociatedParts);
    /*!
   * \fn OnFilterParameters
   */
    void OnFilterParameters();
    /*!
   * \fn onFilterChanged
   * \brief Update view with parameter list
   */
    void onFilterChanged(bool bState);
    /*!
   * \fn onRebuildReport
   */
    void onRebuildReport(const QString& format);
    /*!
   * \fn onSelectMulti
   */
    void onSelectMulti();
    /*!
   * \fn onMoveMulti
   */
    void onMoveMulti();
    /*!
   * \fn onZoomMulti
   */
    void onZoomMulti();
    /*!
   * \fn onHomeMulti
   */
    void onHomeMulti();
    /*!
   * \fn onHome
   */
    void onHome();
    /*!
   * \fn OnChangeDatasetMulti
   */
    void OnChangeDatasetMulti(const QString&);
    /*!
   * \fn onTestChanged
   */
    void onTestChanged();

    /// \brief Change the drawing type of the subset limits
    void OnChangeSubsetLimit();
    /*!
   * \fn OnDeselectHighlightedDie
   */
    void OnDeselectHighlightedDie();


signals:

    /*!
   * \fn testChanged
   */
    void testChanged();

protected:
    /*!
   * \fn closeEvent
   * \brief If window is detached, this doesn close it but rather reparent it
   */
    void closeEvent(QCloseEvent* e);
    /*!
   * \fn goHome
   */
    void goHome(bool bEraseCustomViewport = false);
    /*!
   * \fn goHomeMulti
   */
    void goHomeMulti(bool bEraseCustomViewport = false);
    /*!
   * \fn activateChartTab
   */
    void activateChartTab();
    /*!
   * \fn activateMuiltiChartTab
   */
    void activateMuiltiChartTab();
    /*!
   * \fn activateXBarRChartTab
   */
    void activateXBarRChartTab();
    /*!
   * \fn activateTableTab
   */
    void activateTableTab();
    /*!
   * \fn SetTrendChartXAxis
   */
    bool SetTrendChartXAxis(const QString strXAxisOpt);

public:
    /*!
   * \fn onRemoveDatapointScatter
   */
    void onRemoveDatapointScatter(const QList<QRectF>& rAreaToRemove,
                                  bool                 bRemoveAssociatedParts);
    /*!
   * \fn showPATLimitGroupBox
   */
    void showPATLimitGroupBox(CTest*                     poLastTest,
                              const QList<TestMarker*>& ptCustomMarkers);

public slots:
    /*!
   * \fn showHidePatMarker
   */
    void showHidePatMarker();
    /*!
   * \fn patMarkerIsEnabled
   */
    bool patMarkerIsEnabled(const QString& strMarker);
    /*!
     * \fn void onMlCheckboxChaged()
     * \brief Slot to be called when a multi-limit checkbox is checked/unchecked
     */
    void onMlCheckboxChaged();

    void OnShowHideParam();
    void ShowParam();

    void OnSwitchInteractiveWafermap();

protected:
    /*!
    * \var m_oPatMarkerMap
    */
    QMap<QString, QCheckBox* > m_oPatMarkerMap;
    /*!
    * \var m_oPatMarkerEnable
    */
    QMap<QString, bool> m_oPatMarkerEnable;
    /*!
    * \var m_poLastTest
    */
    CTest* m_poLastTest;

    /*!
    * \fn ShowHideRollingLimitsMarker
    */
    void ShowHideRollingLimitsMarker();

    /*! \var mMlCheckBoxes List of multi-limits checkbox pointers */
    QList<QCheckBox*>   mMlCheckBoxes;

    /*!
     * \fn ShowHideMultiLimitsMarkers()
     * \brief Shows or hides multi-limits checkboxes and containing groupbox
     */
    void ShowHideMultiLimitsMarkers();

public:
    /*!
    * \fn resetPatMarker
    */
    void resetPatMarker();
public:
    /*!
    * \fn updateChart
    */
    void updateChart();

private :
    /**
     * @brief called to update the OFR preview
     */
    void updateOFRManagerView();

    void ManageNextAndPreviousTestButton();
    void ReloadWizardHandler( bool removeAssociatedParts);
    QSize mFrameNavigationSize;

    friend class Gex::InteractiveChartsWizard;

    QString BuildInteractiveLinkWafermap();
    QString BuildInteractiveLinkChart();
};

/******************************************************************************!
 * \class CreateCustomParameter
 ******************************************************************************/
class CreateCustomParameter
        : public QDialog,
        public Ui::drill_custom_parameter_basedialog
{
    Q_OBJECT

public:
    /*!
   * \fn CreateCustomParameter
   */
    CreateCustomParameter(QWidget*   parent = 0,
                          bool       modal = false,
                          Qt::WindowFlags f = 0);
    /*!
   * \fn setExpression
   */
    void setExpression(QString strExpression, bool bClear = true);
    /*!
   * \fn getExpression
   */
    QString getExpression();
    /*!
   * \fn isValidExpression
   */
    bool isValidExpression(long lSampleIndex, int iGroup = 0);
    /*!
   * \var m_GroupID
   */
    int m_GroupID;
    /*!
   * \var m_lOriginalTestNumber
   */
    unsigned long m_lOriginalTestNumber;
    /*!
   * \var m_lOriginalTestPinMap
   */
    long m_lOriginalTestPinMap;

private:
    /*!
   * \var m_strExpression
   */
    QString m_strExpression;

public slots:
    /*!
   * \fn OnLess
   */
    void OnLess();
    /*!
   * \fn OnEqual
   */
    void OnEqual();
    /*!
   * \fn OnGreater
   */
    void OnGreater();
    /*!
   * \fn OnLessEqual
   */
    void OnLessEqual();
    /*!
   * \fn OnNotEqual
   */
    void OnNotEqual();
    /*!
   * \fn OnGreaterEqual
   */
    void OnGreaterEqual();
    /*!
   * \fn OnPickOriginalTest
   */
    void OnPickOriginalTest();
    /*!
   * \fn OnPickTest
   */
    void OnPickTest();
    /*!
   * \fn OnClearExpression
   */
    void OnClearExpression();
    /*!
   * \fn OnOk
   */
    void OnOk();
};

#endif
