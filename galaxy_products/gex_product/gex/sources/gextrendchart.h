#ifndef _GEX_TREND_CHART_H_
#define _GEX_TREND_CHART_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GexTrendPartInfo
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
class GexTrendPartInfo
{
public:
	
	GexTrendPartInfo(const QString& strPartID, int nRunID, int nPosID = -1);
	GexTrendPartInfo(const GexTrendPartInfo& trendPartInfo);
	~GexTrendPartInfo();

	const QString&	partID() const			{ return m_strPartID; }
	int				runID() const			{ return m_nRunID; }
	int				posID() const			{ return m_nPosID; }

	void			setPosID(int nPosID)	{ m_nPosID = nPosID; }

	GexTrendPartInfo&	operator=(const GexTrendPartInfo& trendPartInfo);
	bool				operator<(const GexTrendPartInfo& trendPartInfo) const;
	bool				operator==(const GexTrendPartInfo& trendPartInfo) const;

private:

	QString		m_strPartID;
	int			m_nRunID;
	int			m_nPosID;
};


///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GexTrendChart
//
// Description	:	chart class to draw the trend chart into report or interactive mode
//
///////////////////////////////////////////////////////////////////////////////////
class GexTrendChart : public GexAbstractChart
{

public:
	
    GexTrendChart(bool bPluginCall, int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
	virtual ~GexTrendChart();

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////

	bool			isValidDie() const								{ return m_bValidDie; }
	int				xDie() const									{ return m_nXDie; }
	int				yDie() const									{ return m_nYDie; }
	
	CGexFileInGroup *	markerFile() const						{ return m_pMarkerFile;	}
	CGexGroupOfFiles *	markerGroup() const						{ return m_pMarkerGroup; }
	long				markerGroupID() const					{ return m_lMarkerGroupID; }

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
	
    void			computeCursorValue(const QPoint& pointCursor);	// compute the X/Y values under the cursor

    QString			makeTooltip();									// generate the label tooltip's
    void			setViewportModeFromChartMode(int nChartMode);	// convert chart mode from Gex into viewport mode
	
protected:
	
    virtual void	buildOwnerChart();									// build the owner chart
    void			computeDataset();									// compute the owner dataset
	void			keepXTestViewport();
	
    bool			formatXAxisLabel();									// Should the X axis label be formatted?

    void			chartingLineStyle(CGexSingleChart * pLayerStyle);	// Update some characteristics
    bool			hasSpotsToChart(CGexSingleChart * pLayerStyle);		// return true if the layer contains spots on this chart

    void			plotLotMarkers(CTest * ptTestCell, bool bAggregateData = false);	// draw the lots markers

    // Specific plot for scripting custom markers
    void			plotSpecificScriptingCustomMarker(TestMarker * pTestMarker,
                                                      int nColor, CTest *ptTestCell,
                                                      double dCustomScaleFactor);

    bool 			computePointerDie();			// compute the XY position of the die pointed by cursor

	void			fillPartInfoReferenceMap();
	void			fillPartInfoList(CGexGroupOfFiles * pGroup, QList<GexTrendPartInfo> * pLstPartInfo);	


    /// \brief This function implements the virtual function of GexAbstractChart.
    ///        In this function, we draw all test limits
    void            customizeChart();

private:
	
    bool					m_bShowPartID;				 		// true to show part ID
	bool					m_bPluginCall;
    bool					m_bNumericalPartID;			 		// true if all part ID are numerical value

    bool					m_bValidDie;				 		// holds the die validity
    int						m_nXDie;					 		// holds the X position of the die
    int						m_nYDie;					 		// holds the Y position of the die
    long					m_lMarkerGroupID;			 		// holds the group ID associated with the die

    CGexFileInGroup *		m_pMarkerFile;				 		// holds the file associated with the die
    CGexGroupOfFiles *		m_pMarkerGroup;				 		// holds the group associated with the die

	QStringList				m_lstPartID;
    QMap<QString, int>		m_mapPosID;					 		// holds the posID of a PartID
	
	int						m_nMaxTextHeight;
	int						m_nMaxTextWidth;
		
    void					adjustXCursorValue();		 		// Round xCursor value to the right run number
};


#endif // _GEX_TREND_CHART_H_
