#ifndef _PARAMETRIC_SCALE_COLOR_WIDGET_H_
#define _PARAMETRIC_SCALE_COLOR_WIDGET_H_

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QtWidgets>

class CTest;

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GexParametricScaleColorWidget
//
// Description	:	Class to display parametrics scale colors
//
///////////////////////////////////////////////////////////////////////////////////
class GexParametricScaleColorWidget : public QWidget
{
public:
	
	GexParametricScaleColorWidget(QWidget * pParent = NULL);
	~GexParametricScaleColorWidget();

	CTest *		testCell() const								{ return m_pTestCell; }

	void		setRange(double dMinRange, double dMaxRange)	{ m_dMinRange = dMinRange; m_dMaxRange = dMaxRange; updateMarkerPos(); }
	void		setTestCell(CTest * pTestCell);	
	void		setSelection(double dValue);

	void		updateScale();									//	update the content of the scale
	
	QSize		minimumSizeHint() const							{ return QSize(100, 150); }
		
protected:
	
	void		drawCursor(QPainter& painter, int nMarkerPos, int nTextPos, const QString& strLabel);
	void		updateMarkerPos();
	void		computeMarker();

	void		paintEvent(QPaintEvent * pEvent);
	void		resizeEvent(QResizeEvent * pEvent);
	void		mouseMoveEvent(QMouseEvent * pEvent);

	QString		formatValue(double dValue, class CReportOptions* ro);

private:
	
	class GexPrivateParametricMarker
	{
	
	public:
		
		enum markerType
		{
			minimumValue,
			lowLimit,
			meanValue,
			selectedValue,
			highLimit,
			maximumValue
		};

		GexPrivateParametricMarker(markerType eType, float fMarkerPos);
		GexPrivateParametricMarker(const GexPrivateParametricMarker& marker);
		~GexPrivateParametricMarker();

		bool	operator<(const GexPrivateParametricMarker& marker) const;

		int			m_nRectPos;
		int 		m_nCursorPos;
		float		m_fMarkerPos;
		markerType	m_eMarkerType;
	};
	
	QList<GexPrivateParametricMarker>	m_lstMarker;

	double		m_dSelection;
	double		m_dLowLimit;
	double		m_dHighLimit;
	double		m_dMinValue;
	double		m_dMaxValue;
	double		m_dMean;
	double		m_dMinRange;
	double		m_dMaxRange;

	float		m_fLowLimitMarkerPos;
	float		m_fHighLimitMarkerPos;
	float		m_fMinValueMarkerPos;
	float		m_fMaxValueMarkerPos;
	float		m_fMeanMarkerPos;
	float		m_fSelectionMarkerPos;

	QPixmap		m_pixLeftCursor;
	QRect		m_rcScaleColor;

	CTest *		m_pTestCell;

	bool		m_bNeedUpdate;					// true if the widget need to be updated
};

#endif // _PARAMETRIC_SCALE_COLOR_WIDGET_H_
