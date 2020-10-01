/******************************************************************************!
 * \file drillDataMining3D.h
 * \brief Interactive drill: 3D Data Mining
 ******************************************************************************/
#ifndef DRILL_DATA_MINING_3D_H
#define DRILL_DATA_MINING_3D_H

#include "QTitleFor.h"

#include <qgl.h>
#if __MACH__
#include "OpenGL/glu.h"
#else
#include <GL/glu.h>
#endif

#include "ui_drill_3d_dialog.h"
#include "drillObjectCell.h"
#include "drillData3DInfo.h"

#include <QSplitter>
#include <QTextBrowser>
#include <QtWidgets/QTreeWidget>

#include "font_3d.h"

#ifndef DRILL_NO_DATA_MINING
# include "vec.h"
#endif

class CTest;
class CGexFileInGroup;
class CFont_3D;
class CPartInfo;
class GexScaleColorWidget;
class GexPanelWidget;
class GexGLViewer;
class QSplitter;
using qglviewer::Vec;  // FIXME

namespace Gex {

/******************************************************************************!
 * \class DrillDataMining3D
 * \brief Drill 3D Dialog box
 ******************************************************************************/
class WizardHandler;
class InteractiveWafer3DWizard;
class DrillDataMining3D :
    public GS::Gex::QTitleFor< QDialog >, public Ui::drill_3d_basedialog
{
  Q_OBJECT

public:
  /*!
   * \enum DataMode
   */
  enum DataMode {
    standard,
    nnr
  };
  /*!
   * \fn DrillDataMining3D
   * \brief Constructor
   */
  DrillDataMining3D(QWidget* parent = 0, Qt::WindowFlags fl = Qt::Window);
  /*!
   * \fn ~DrillDataMining3D
   * \brief Destructor
   */
  ~DrillDataMining3D();


  int mIndex;

  /*!
   * \fn ShowChart
   */
  void ShowChart(QString strLink);
  /*!
   * \fn ShowChart
   */
  void ShowChart(int nGroupID, int nFileID, int nTestNumber, int nPinmap,
                 int nDrillType);
  /*!
   * \fn showWindows
   */
  void showWindows();
  /*!
   * \fn hideWindows
   */
  void hideWindows();
  /*!
   * \fn isDataAvailable
   */
  bool isDataAvailable(void);
  /*!
   * \fn ForceAttachWindow
   */
  //void ForceAttachWindow(bool bAttach = true, bool bToFront = true);
  /*!
   * \fn SelectDieLocation
   */
  void SelectDieLocation(CGexFileInGroup*, int iDieX, int iDieY,
                         bool bClearSelections);

  /*!
   * \var m_FullScreen
   */
  bool m_FullScreen;
  // bool mWafmapAlwaysRound; Remove 6015/7303 case change

#ifndef DRILL_NO_DATA_MINING
  /*!
   * \fn refreshViewer
   * \brief Add snapshot to the report file
   */
  void refreshViewer(bool bReset = false);
  /*!
   * \fn GrabGLViewer
   * \brief Grap the pixmap of the GLViewer/GLWidget
   *
   * QPixmap could be Null (use isNull())
   */
  QPixmap GrabGLViewer();
  /*!
   * \fn GrabWidget
   * \brief Grab the entire widget : 3d gl widget + the right stat html widget
   */
  QPixmap GrabWidget();

  void setWizardHandler(Gex::WizardHandler* lWizardHandler) { mWizardHandler = lWizardHandler; }
  void focusInEvent( QFocusEvent* );
  void AddNewOFRSection();
  void AddToAnExistingOFRSection(int indexSection);
  void AddToLastUpdatedOFRSection();
public slots:
  /**
   * @brief react on a tab renamed
   * @param widget the widget inside the renamed tab
   * @param text the text used to rename the tab
   */
  void OnTabRenamed( QWidget *widget, const QString &text );

  /*!
   * \fn OnProperties
   * \brief Show Selections Properties
   */
  void OnProperties();
  /*!
   * \fn OnDetachWindow
   */
  //void OnDetachWindow();
  /*!
   * \fn OnFindDie
   */
  void OnFindDie();
  /*!
   * \fn OnShowTestsExecutedInDie
   */
  void OnShowTestsExecutedInDie();
  /*!
   * \fn onPaintGL
   * \brief Overloads the slot for GL paint
   */
  void onPaintGL();
  /*!
   * \fn onSelectionDone
   * \brief Manages the select object event
   */
  void onSelectionDone(const QPoint&);
  /*!
   * \fn OnInteractiveCharts
   * \brief
   */
  void OnInteractiveCharts();
  /*!
   * \fn OnInteractiveTables
   * \brief
   */
  void OnInteractiveTables();

  void OnInteractiveWafermap();
  void OnSwitchInteractiveCharts();
private slots:
  /*!
   * \fn onChangeBinning
   * \brief Open a dialog box to change die binning
   */
  void onChangeBinning();
  /*!
   * \fn OnAddToOFR
   * \brief add the wafermap to the OFR report
   */
  void toJson(QJsonObject& lElt);
  /*!
   * \fn onRemovePart
   * \brief Remove part from data flow (erase all data results in run)
   */
  void onRemovePart();
  /*!
   * \fn onContextualMenu
   * \brief Open a contextual menu
   */
  void onContextualMenu(const QPoint&);
  /*!
   * \fn onPickTest
   * \brief Pick a test
   */
  void onPickTest();
  /*!
   * \fn onExportWaferMap
   * \brief Export wafermap into a file
   */
  void onExportWaferMap();
  /*!
   * \fn onChangeChartType
   * \brief Chart type selection changed
   */
  void onChangeChartType(int iIndex);
  /*!
   * \fn onChangeSpectrumColors
   * \brief Spectrum colors selection changed
   */
  void onChangeSpectrumColors(int iIndex);
  /*!
   * \fn onChangeGroup
   * \brief Group changed
   */
  void onChangeGroup(int nGroupID);
  /*!
   * \fn onSpectrumValueChanged
   * \brief Low or high value of the spectrum color has changed
   */
  void onSpectrumValueChanged();
  /*!
   * \fn OnChangeBinFilter
   */
  void OnChangeBinFilter(QTreeWidgetItem*, int);
  /*!
   * \fn onDataLoaded
   * \brief New data has been loaded to display
   */
  void onDataLoaded(bool, bool);
  /*!
   * \fn onTestNavigate
   * \brief Test has changed for the current data
   */
  void onTestNavigate();
  /*!
   * \fn onFilterParameters
   * \brief Allows to filter on a short test list
   */
  void onFilterParameters(bool bState);
  /*!
   * \fn onFilterChanged
   * \brief Update view with parameter list
   */
  void onFilterChanged(bool bState);

protected:
  /*!
   * \fn closeEvent
   * \brief Close event: if window is detached, this does not close it but
   *        rather reparent it !
   */
  void closeEvent(QCloseEvent* e);
  void keyPressEvent(QKeyEvent *e);
protected slots:
  void OnAddNewOFRSection();
  void OnAddToAnExistingOFRSection(QAction *action);
  void OnAddToLastUpdatedOFRSection();
private slots:
  /*!
   * \fn fillComboWafermap
   * \brief Fill the wafermap Combobox
   */
  void fillComboWafermap(int nGroupID);
private:

  Gex::WizardHandler* mWizardHandler;

  void  InitInteractiveView();

  /*!
   * \fn initGui
   * \brief Initialize GUI
   */
  void initGui();
  /*!
   * \fn updateGui
   * \brief Update the GUI
   */
  void updateGui();
  /*!
   * \fn exportParametricWaferMap
   * \brief Export parametric Wafermap into a text file
   */
  void exportParametricWaferMap();
  /*!
   * \fn findSelectedDie
   * \brief Find the die from the selected object
   */
  bool findSelectedDie(int nIndexObject, DrillObjectCell& dieCell);
  /*!
   * \fn isSelectedDie
   * \brief Find if a die is already selected
   */
  bool isSelectedDie(int nIndex, const DrillObjectCell& dieCell);
  /*!
   * \fn parseLink
   * \brief Parse an external link
   */
  void parseLink(QString strLink);
  /*!
   * \fn fillViewer
   * \brief Fill the viewer 3D. Fill the wafermap if need be
   */
  void fillViewer(bool bReset = false);
  /*!
   * \fn updateData
   * \brief Update data used to build the wafer map
   */
  void updateData();
  /*!
   * \fn resetViewerPosition
   * \brief Reset the viewer position
   */
  void resetViewerPosition();
  /*!
   * \fn resetSelection
   * \brief Reset selection
   */
  void resetSelection(bool resetAll=true);
  /*!
   * \fn makeOpenGLList
   * \brief Build the OpenGL list of vectors to be painted
   */
  void makeOpenGLList();
  /*!
   * \fn pGetPartInfo
   */
  CPartInfo* pGetPartInfo(int lObjectIndex);
  /*!
   * \fn CreatePropertiesFile
   * \brief Create HTML page header (Properties info)
   */
  void CreatePropertiesFile(QString strTitle);
  /*!
   * \fn ClosePropertiesFile
   * \brief Close HTML page
   */
  void ClosePropertiesFile();
  /*!
   * \fn WriteInfoLine
   */
  void WriteInfoLine(const char* szLabel,
                     const char* szText,
                     bool bAlarmLabelColor = false,
                     bool bAlarmTextolor = false);
  /*!
   * \fn
   */
  void WriteInfoLine(const char* szLabel,
                     QString strText,
                     bool bAlarmLabelColor = false,
                     bool bAlarmTextolor = false);
  /*!
   * \fn LoadLandscapeArray
   * \brief Load GEX data from STDF into 3D array for OpenGL manipulations
   */
  void LoadLandscapeArray();
  /*!
   * \fn LoadLandscapeArrayWafermap
   * \brief Load lanscape array from Wafermap data
   */
  void LoadLandscapeArrayWafermap();
  /*!
   * \fn LoadLandscapeArrayPackaged
   * \brief Load lanscape array from Packaged data
   */
  void LoadLandscapeArrayPackaged();
  /*!
   * \fn LoadLandscapeArrayHistogram
   * \brief Load lanscape array from Histogram data
   */
  void LoadLandscapeArrayHistogram();
  /*!
   * \fn LoadLandscapeArrayTrend
   * \brief Load lanscape array from Trend data
   */
  void LoadLandscapeArrayTrend();
  /*!
   * \fn renderScene
   * \brief Entry point to all scenes display
   */
  void renderScene();
  /*!
   * \fn renderLandscapeArrayWafermap
   * \brief Render OpenGL vectors to paint WaferMap
   */
  void renderLandscapeArrayWafermap();
  /*!
   * \fn renderLandscapeArrayPackaged
   * \brief Render OpenGL vectors to paint Packaged data
   */
  void renderLandscapeArrayPackaged();
  /*!
   * \fn renderLandscapeArrayHistogram
   * \brief Render OpenGL vectors to paint Histogram data
   */
  void renderLandscapeArrayHistogram();
  /*!
   * \fn renderLandscapeArrayTrend
   * \brief Render OpenGL vectors to paint Histogram data
   */
  void renderLandscapeArrayTrend();
  /*!
   * \fn renderLandscapeArrayGrid
   * \brief Render chart grid & scales (used as Histograms background)
   */
  void renderLandscapeArrayGrid(int iDir1, int iDir2, int iLimitDir,
                                int iMarkersDir);
  /*!
   * \fn renderMarker
   * \brief Draw chart marker
   */
  double renderMarker(int iPosition, int iGroupID, double lfValue,
                      double fRadius, bool bDashedLine, const char* szTitle);
  /*!
   * \fn renderText
   * \brief Draw text string
   */
  void renderText(const Vec& cOrigin, int iDirection, bool bFill, float fHeight,
                  const char* szString);
  /*!
   * \fn InitOpenGLFont
   * \brief Reset 3D font library: Initializes the OpenGL Font library
   */
  void InitOpenGLFont();
  /*!
   * \fn WriteTestStatisticsDetails
   */
  void WriteTestStatisticsDetails(CTest* ptTestCell, CGexFileInGroup* pFile);
  /*!
   * \fn ShowGlobalProperties
   * \brief Show global properties (about the data view)
   */
  void ShowGlobalProperties();
  /*!
   * \fn ShowDieProperties
   * \brief Show die properties (about the data view)
   */
  void ShowDieProperties();
  /*!
   * \fn ShowBinProperties
   * \brief Show bin properties (about the data view)
   */
  void ShowBinProperties(int iDrillReport);
  /*!
   * \fn ShowTestProperties
   * \brief Show Test properties (about the data view)
   */
  void ShowTestProperties(int iDrillReport);
  /*!
   * \fn SetPlotColor
   */
  void SetPlotColor(float fColor, const QColor& rgbColor = QColor());
  /*!
   * \fn SetPlotColorOutline
   */
  void SetPlotColorOutline(float fColor, const QColor& rgbColor = QColor(),
                           bool bRetested = false);

  /*!
   * \fn renderObject
   * \brief Render Object: calls relevant render functions (Line, Bar, etc.)
   */
  void renderObject(Vec* pDiePosition3D, int x, int y, float fColor,
                    float fRadius, float fHeight,
                    const QColor& rgbColor = QColor(), bool bRetested = false);

  /*!
   * \fn renderPoint
   * \brief Renders a solid/Outlined Point at the given location
   *        with given width
   */
  void renderPoint(const Vec& cOrigin, float fColor, bool bSolid, float iWidth,
                   float fHeight, const QColor& rgbColor = QColor(),
                   bool bRetested = false, float iLineWidth = 2.5);
  /*!
   * \fn renderLine
   * \brief Draw line from ground to pint
   */
  void renderLine(const Vec& cOrigin, float fColor, float radius,
                  const QColor& rgbColor = QColor(), float iLineWidth = 2.5);
  /*!
   * \fn renderBar
   * \brief Renders a solid/Outlined bar at the given location with given width
   */
  void renderBar(const Vec& cOrigin, float fColor, bool bSolid, float iWidth,
                 float fHeight, const QColor& rgbColor = QColor(),
                 bool bRetested = false, float iLineWidth = 2.5);
  /*!
   * \fn renderWire
   * \brief Renders a Wire structure
   */
  void renderWire(const Vec& cOrigin, float fColor, int x, int y,
                  const QColor& rgbColor = QColor());

  /*!
   * \fn drawSmoothShaded
   * \brief Renders a Surface structure
   */
  void drawSmoothShaded(const Vec& cOrigin, float fColor, int x, int y,
                        const QColor& rgbColor = QColor());
  /*!
   * \fn renderSolidCylinder
   * \brief Renders a solid cylinder at the given location with given radius
   */
  void renderSolidCylinder(const Vec& cOrigin, float radius, float fHeight,
                           int iSlices, int iStacks,
                           const QColor& rgbColor = QColor(),
                           float iLineWidth = 2.5);
  /*!
   * \fn renderSolidSphere
   * \brief Renders a solid sphere at the given location with given radius
   */
  void renderSolidSphere(const Vec& cCenter, float radius, int iSlices,
                         int iStacks, const QColor& rgbColor = QColor(),
                         float iLineWidth = 2.5);

  /*!
   * \fn renderWireSphere
   * \brief Renders a wire sphere at the given location with given radius
   */
  void renderWireSphere(const Vec& cCenter, float radius, int iSlices,
                        int iStacks, const QColor& rgbColor = QColor(),
                        float iLineWidth = 2.5);
  /*!
   * \var m_glDispList
   */
  GLuint m_glDispList;
  /*!
   * \var m_lstSelections
   * \brief List of objects selected
   */
  QList<DrillObjectCell> m_lstSelections;
  /*!
   * \var m_nMousePosX
   * \brief Xlast mouse position
   */
  int m_nMousePosX;
  /*!
   * \var m_nMousePosY
   * \brief Y last mouse position
   */
  int m_nMousePosY;
  /*!
   * \var m_fObjectCellSize
   * \brief % of space elementary 3D cube space each object uses. eg: 90%, 70%
   */
  float m_fObjectCellSize;
  /*!
   * \var hProperties
   * \brief Handle to HTML Porperties page
   */
  FILE* hProperties;
  /*!
   * \var strPropertiesPath
   * \brief Full path+name of Neptus Properties page
   */
  QString strPropertiesPath;
  /*!
   * \var hFont
   * \brief Keeps the handle to up to 4 different fonts
   */
  int hFont[4];
  /*!
   * \var m_nSpectrumColors
   */
  int m_nSpectrumColors;
  /*!
   * \var m_eMode
   */
  DataMode m_eMode;
  /*!
   * \var m_drillDataInfo
   */
  DrillData3DInfo m_drillDataInfo;
  /*!
   * \var m_pPropertiesBrowser
   */
  QTextBrowser* m_pPropertiesBrowser;
  /*!
   * \var m_pScaleColorWidget
   */
  GexScaleColorWidget* m_pScaleColorWidget;
  /*!
   * \var m_pViewer
   */
  GexGLViewer* m_pViewer;
  /*!
   * \var m_pPanelWidget
   */
  GexPanelWidget* m_pPanelWidget;
  /*!
   * \var m_pSplitter
   */
  QSplitter* m_pSplitter;
  /*!
   * \var m_lstNNRDie
   */
  QList<long> m_lstNNRDie;
  /*!
   * \var m_pclFont3D
   */
  CFont_3D* m_pclFont3D;
  /*!
   * \var SetRenderingMode
   * \brief Select the rendering mode
   */
  void SetRenderingMode();
  /*!
   * \fn calculateVertexNormals
   */
  void calculateVertexNormals();
  /*!
   * \fn averageNormals
   */
  void averageNormals();
  /*!
   * \fn createLandscape
   */
  void createLandscape(int size, int landscapePlans = -1);
  /*!
   * \fn destroyLandscape
   */
  void destroyLandscape();
  /*!
   * \fn destroyNormals
   */
  void destroyNormals();
  /*!
   * \fn drawLines
   */
  void drawLines();
  /*!
   * \fn drawPoints
   */
  void drawPoints();
  /*!
   * \fn drawBars
   */
  void drawBars();
  /*!
   * \fn drawWireframe
   */
  void drawWireframe();
  /*!
   * \fn drawSmoothShaded
   */
  void drawSmoothShaded();
  /*!
   * \fn setLandscapeSize
   */
  void setLandscapeSize(int);
  /*!
   * \fn fractalize
   */
  void fractalize();
  /*!
   * \enum MarkerPos
   */
  enum MarkerPos { LeftLoc, RightLoc, TopLoc, BottomLoc};
  /*!
   * \enum Direction
   */
  enum Direction { XRight, XLeft, YRight, YLeft, ZRight, ZLeft};
  /*!
   * \enum Axis
   */
  enum Axis { XAxis, YAxis, ZAxis };
  /*!
   * \enum RenderModes
   */
  enum RenderModes { FlatBars, SimpleLines, SolidBars, OutlinedBars,
                     SolidPoints, OutlinedPoints, Wireframe, SmoothShaded };
  /*!
   * \enum Views
   */
  enum Views { DefaultView, CurrentView };
  /*!
   * \var m_RenderMode
   */
  RenderModes m_RenderMode;
  /*!
   * \typedef gridNormals
   */
  typedef struct grid_normals {
    double u[3], l[3];
  } gridNormals;
  /*!
   * \typedef avgNormals
   * \brief Structure used to store the vertex normals for the landscape
   */
  typedef struct avg_normals {
    double n[3];
  } avgNormals;
  /*!
   * \typedef viewMatrix
   */
  typedef struct viewMatrix {
    GLfloat model[4][4];  // OpenGL model view matrix for the view
    GLfloat projection[4][4];  // OpenGL projection matrix for the view
  } viewMatrix;
  /*!
   * \var landscape
   * \brief Height field data
   */
  double** landscape;
  /*!
   * \var normals
   */
  gridNormals** normals;
  /*!
   * \var vertexNormals
   */
  avgNormals** vertexNormals;
  /*!
   * \var views
   */
  viewMatrix views[2];
  /*!
   * \var fLowColorHeight
   * \brief Height for the lowest color (Dark blue)
   */
  float fLowColorHeight;
  /*!
   * \var fHighColorHeight
   * \brief Height for the highest color (Light Red)
   */
  float fHighColorHeight;
  /*!
   * \var iHighBin
   * \brief Highest Bin to plot
   */
  int iHighBin;
  /*!
   * \var oldPos
   */
  QPoint oldPos;
  /*!
   * \var oldX
   */
  GLfloat oldX;
  /*!
   * \var oldY
   */
  GLfloat oldY;
  /*!
   * \var oldZ
   */
  GLfloat oldZ;
  /*!
   * \var initFractals
   */
  bool initFractals;
  /*!
   * \var landscapeSize
   */
  int landscapeSize;
  /*!
   * \var landscapeUsageX
   */
  int landscapeUsageX;
  /*!
   * \var landscapeUsageY
   */
  int landscapeUsageY;
  /*!
   * \var landscapePlans
   */
  int landscapePlans;
  /*!
   * \var landscapeObjects
   */
  int landscapeObjects;
  /*!
   * \var animationRunning
   */
  bool animationRunning;
  /*!
   * \var mouseButtonDown
   */
  bool mouseButtonDown;
#endif  // DRILL_NO_DATA_MINING
  friend class InteractiveWafer3DWizard;

  QString BuildInteractiveLinkChart();
  QString BuildInteractiveLinkWafermap();
};

}  // namespace Gex

#endif
