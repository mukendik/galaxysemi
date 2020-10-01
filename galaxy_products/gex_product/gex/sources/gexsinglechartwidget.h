#ifndef _GEX_SINGLECHART_WIDGET_H_
#define _GEX_SINGLECHART_WIDGET_H_

#include <QMenu>
#include "gexabstractchart.h"

class GexChartMenuInfo
{
public:

    GexChartMenuInfo()											{ m_dOutlierLimit = 0.0; m_pActionTriggered = NULL; m_dSplitValue=0.f; }

    QMenu *		menu()											{ return &m_chartMenu; }
    QAction *	actionTriggered()								{ return m_pActionTriggered; }
    double		outlierLimit() const							{ return m_dOutlierLimit; }

    void		setOutlierLimit(double dOutlierLimit)			{ m_dOutlierLimit = dOutlierLimit; }
    void		setActionTriggered(QAction * pAction)			{ m_pActionTriggered = pAction;	}

    double		m_dSplitValue;				// when splitting a dataset, use this value.
    double		m_dFactor;
    double		m_dScaleFactor;//Scale factor on test results


protected:

    QMenu		m_chartMenu;
    QAction *	m_pActionTriggered;
    double		m_dOutlierLimit;
};

class GexWizardChart;
class GexSingleChartWidget : public QWidget
{
    Q_OBJECT

public:

    enum MouseMode
    {
        mouseSelect,
        mouseDrag,
                mouseZoom,
                mouseZoneSelect
    };
        enum MouseStatus{
            unknow,
            released,
            pressed
        };

    GexSingleChartWidget(QWidget * pParent = NULL, bool bAutoRepaint = true);
    ~GexSingleChartWidget();

    GexAbstractChart *	abstractChart() const				{ return m_pAbstractChart; }
    void                deleteAbstractChart() 				{ delete m_pAbstractChart; m_pAbstractChart = 0; }
    MouseMode			mouseMode() const					{ return m_eMouseMode; }

    void	enableAutoRepaint(bool bAutoRepaint = true)		{ m_bAutoRepaint = bAutoRepaint; }
    void	setChartType(GexAbstractChart::chartType eChartType, CGexChartOverlays *lChartsInfo, int nViewportMode, bool bKeepViewport);
    void	setViewportMode(int nViewportMode);
    void	setMouseMode(MouseMode mouseMode);

    void	resetViewPortManager(bool bEraseCustomViewport = false);
    void	replotChart( CReportOptions* lOption,  CTest* lTestCellX, CTest* lTestCellY);
    void	keepViewport();
    void    init(GexWizardChart*   lWizardParent);

    void	onChartOptionChanged(GexAbstractChart::chartOption eChartOption);

protected:

    bool	isDrawAuthorized() const			{ return !m_pRubberBand; }

    void	setOutline(bool bOutline)			{ m_bOutline		= bOutline; }

    void	paintEvent( QPaintEvent * );
    void	mousePressEvent(QMouseEvent *);
    void	mouseReleaseEvent(QMouseEvent *);
    void	mouseMoveEvent(QMouseEvent *);
    void    keyReleaseEvent ( QKeyEvent * event ) ;
    void    keyPressEvent ( QKeyEvent * event );


    void	contextualMenu();
    void	drawOutLine();
    void	selectCurrentDieLocation();

private:

    int					zoomDirection() const;
    int					scrollDirection() const;

    bool				m_bAutoRepaint;			// Repaint will be automatically called after replotChart() function when true
    MouseMode			m_eMouseMode;

    bool				m_bTracking;
    bool				m_bOutline;				//
    QPoint				m_ptFirst;
    QPoint				m_ptLast;
    QRubberBand *		m_pRubberBand;
    QList<QRect>    m_pRubberBandScaterList;

    int					m_nChartType;			// Chart type (Box plot, probability plot)

    GexAbstractChart *	m_pAbstractChart;		// Pointer on the chart drawn

    GexChartMenuInfo *	m_pChartMenuInfo;

    QCursor				m_zoomCursor;
    MouseStatus             m_eMouseStatus;
    GexWizardChart*     mWizardParent;

    // private method just for Split purpose
    bool        WriteSplitGexFileInCSL(FILE *hFile, CGexGroupOfFiles* gof, CGexPartFilter::filterType eFilterType,
                                                        const QString& xml_fn_pattern, QString group_id_name, CTest *	ptTest, QString str_xml_output_path,
                                                        double dLow=C_INFINITE,double dHigh =C_INFINITE);
    void        processZoneSelection    ();
    void        RemoveZoneFromScatter   ();
    void        ApplyRemove             (bool );
    QString     GetOFRElementTypeName   ();

protected slots:

    void                OnAddNewOFRSection();
    void                OnAddToAnExistingOFRSection(QAction* action);
    void                OnAddToLastUpdatedOFRSection();

    void                OnDeselectHighlightedDie();
    void				onEditProperties();
    void				onEditStyles();
    void				onEditMarkers();
    void				onCopyChartToClipboard();
    void				onSaveChartToDisk();
    void				onDeviceResults();
    void				onWafermapDatapointHigher();
    void				onWafermapDatapointLower();
    void				onWafermapDatapointRange();
    void				onWafermapDie();
    void				onRebuildReport_Web();
    void				onRebuildReport_Word();
    void				onRebuildReport_PPT();
    void				onRebuildReport_PDF();
    void				onRebuildReport_Excel();
    void				onRemoveDatapointHigher();
    void				onRemoveDatapointLower();
    void				onRemoveDatapointRange();
    void				onRemoveIQR();
    void				onRemoveNSigma();
    void				onSplitDataset();		// when the user click on split in contextual menu
    void                            onSplitDatasetZone();

    void				onActionHovered(QAction*);

    void				onKeepViewportChanged(bool isChecked);
    void                onSelectScaterArea();
    void                UndoAll();

signals:

    void				currentDieSelected();
    void                deselectHighlightedDie();
    void				deviceResults(long);
    void				editStyles();
    void				editMarkers();
    void				selectWafermapRange(double, double);
    void				removeDatapointRange(double, double, bool);
    void				removeDatapointHigher(double, bool);
    void				removeDatapointLower(double, bool);
    void				removeIQR(double, bool);
    void				removeNSigma(double, bool);
    void				rebuildReport(const QString& strOutputFormat);
    void				viewportChanged(const GexSingleChartWidget *);
};

#endif // _GEX_SINGLECHART_WIDGET_H_
