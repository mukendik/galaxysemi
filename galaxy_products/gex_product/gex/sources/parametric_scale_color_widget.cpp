///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "parametric_scale_color_widget.h"
#include "report_build.h"
#include "gex_file_in_group.h"
#include "gex_report.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QBitmap>

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport*		gexReport;			// Handle to report class
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

#define OUT_OF_SCALE	-1

///////////////////////////////////////////////////////////////////////////////////
// Class GexParametricScaleColorWidget - class which
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexParametricScaleColorWidget::GexParametricScaleColorWidget(QWidget * pParent /*= NULL*/) : QWidget(pParent)
{
	m_dSelection	= GEX_C_DOUBLE_NAN;
	m_dLowLimit		= GEX_C_DOUBLE_NAN;
	m_dHighLimit	= GEX_C_DOUBLE_NAN;
	m_dMinValue		= GEX_C_DOUBLE_NAN;
	m_dMaxValue		= GEX_C_DOUBLE_NAN;
	m_dMean			= GEX_C_DOUBLE_NAN;
	m_dMinRange		= GEX_C_DOUBLE_NAN;
	m_dMaxRange		= GEX_C_DOUBLE_NAN;

	m_lstMarker.clear();

	m_fLowLimitMarkerPos	= OUT_OF_SCALE;
	m_fHighLimitMarkerPos	= OUT_OF_SCALE;
	m_fMinValueMarkerPos	= OUT_OF_SCALE;
	m_fMaxValueMarkerPos	= OUT_OF_SCALE;
	m_fMeanMarkerPos		= OUT_OF_SCALE;
	m_fSelectionMarkerPos	= OUT_OF_SCALE;

	m_bNeedUpdate			= false;

	m_pTestCell				= NULL;

	m_pixLeftCursor = QPixmap (QString::fromUtf8(":/gex/icons/lcursor.png"));

	QBitmap bitmapMask = m_pixLeftCursor.createMaskFromColor(QColor(255, 255, 255), Qt::MaskInColor);
	m_pixLeftCursor.setMask(bitmapMask);

	m_rcScaleColor.setCoords(20, 20, 40, rect().height() - 21);

	setMouseTracking(true);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexParametricScaleColorWidget::~GexParametricScaleColorWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString	formatValue(double dValue);
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
QString	GexParametricScaleColorWidget::formatValue(double dValue, CReportOptions* ro)
{
	QString strValue;
	CGexFileInGroup cFile(NULL,0,"",0,0,"","","");
	cFile.UpdateOptions(ro); // CheckMe ! FormatTestResult() uses m_eScalingMode !

	if (m_pTestCell->lTestNumber < GEX_TESTNBR_OFFSET_EXT_SBIN || m_pTestCell->lTestNumber > GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T)
		strValue = cFile.FormatTestResult(m_pTestCell, dValue, m_pTestCell->res_scal);
	else
		strValue = QString::number((int) dValue);

	return strValue;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void updateScale()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::updateScale()
{
	if (m_bNeedUpdate)
	{
		// Repaint the widget
		repaint();

		// Set update to false
		m_bNeedUpdate = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setTestCell(CTest * pTestCell)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::setTestCell(CTest * pTestCell)
{
	m_pTestCell = pTestCell;

	m_dMaxValue = m_pTestCell->lfSamplesMax;
	m_dMinValue = m_pTestCell->lfSamplesMin;

	if((m_pTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
		m_dLowLimit = m_pTestCell->GetCurrentLimitItem()->lfLowLimit;
	else
		m_dLowLimit =  GEX_C_DOUBLE_NAN;

	if((m_pTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
		m_dHighLimit = m_pTestCell->GetCurrentLimitItem()->lfHighLimit;
	else
		m_dHighLimit = GEX_C_DOUBLE_NAN;

	m_dMean = m_pTestCell->lfMean;

	// update marker position
	updateMarkerPos();
}

void GexParametricScaleColorWidget::setSelection(double dValue)
{
	m_dSelection = dValue;

	// update marker position
	updateMarkerPos();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void mouseMoveEvent(QMouseEvent * pEvent)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::mouseMoveEvent(QMouseEvent * pEvent)
{
	QString strTooltip("");

	if (m_rcScaleColor.contains(pEvent->pos()))
	{
		double	dRange	= m_dMaxRange - m_dMinRange;
		float	fOffset = (m_rcScaleColor.bottom() - pEvent->pos().y()) / (float) m_rcScaleColor.height();
		double	dValue	= m_dMinRange + (dRange * fOffset);

		strTooltip = formatValue(dValue, &ReportOptions);	// Check me : do we have to use global ReportOptions ?
	}

	setToolTip(strTooltip);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void resizeEvent(QResizeEvent * pEvent)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::resizeEvent(QResizeEvent * pEvent)
{
	QWidget::resizeEvent(pEvent);

	m_rcScaleColor.setCoords(20, 20, 40, rect().height() - 21);

	computeMarker();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void paintEvent(QPaintEvent * pEvent)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::paintEvent(QPaintEvent* /*pEvent*/)
{
	QPainter	painter(this);
	QPen		originPen = painter.pen();
	int			nRed;
	int			nGreen;
	int			nBlue;

	for (int nPos = m_rcScaleColor.top(); nPos < m_rcScaleColor.bottom(); nPos++)
	{
		int nValue = ((m_rcScaleColor.bottom() - nPos) * 100) / m_rcScaleColor.height();

		if(nValue <= 25)
		{
			nRed	= 0;
			nBlue	= 255;
			nGreen	= (255 * nValue) / 25;
		}
		else if(nValue <= 50)
		{
			nGreen	= 255;
			nRed	= 0;
			nBlue	= (255 * (50 - nValue)) / 25;
		}
		else if(nValue <= 75)
		{
			nGreen	= 255;
			nBlue	= 0;
			nRed	= (255 * (nValue - 50)) / 25;
		}
		else
		{
			nRed	= 255;
			nBlue	= 0;
			nGreen	= (255 * (100 - nValue)) / 25;
		}

    painter.setPen(QColor((int) (0.6 * nRed),
                          (int) (0.6 * nGreen),
                          (int) (0.6 * nBlue)));
		painter.drawLine(m_rcScaleColor.left(), nPos, m_rcScaleColor.right(), nPos);
	}

	painter.setPen(originPen);

	QString strLabel;

	for (int nMarker = 0; nMarker < m_lstMarker.count(); nMarker++)
	{
		switch(m_lstMarker.at(nMarker).m_eMarkerType)
		{
			case GexPrivateParametricMarker::lowLimit		:	strLabel = " Low Limit : " + formatValue(m_dLowLimit, &ReportOptions);
																break;

			case GexPrivateParametricMarker::highLimit		:	strLabel = " High Limit : " + formatValue(m_dHighLimit, &ReportOptions);
																break;

			case GexPrivateParametricMarker::maximumValue	:	strLabel = " Max : " + formatValue(m_dMaxValue, &ReportOptions);
																break;

			case GexPrivateParametricMarker::minimumValue	:	strLabel = " Min : " + formatValue(m_dMinValue, &ReportOptions);
																break;

			case GexPrivateParametricMarker::meanValue		:	strLabel = " Mean : " + formatValue(m_dMean, &ReportOptions);
																break;

			case GexPrivateParametricMarker::selectedValue	:	strLabel = " Selected die : " + formatValue(m_dSelection, &ReportOptions);
																break;

			default											:	strLabel = " unknown marker";
																break;
		}

		drawCursor(painter, m_lstMarker.at(nMarker).m_nCursorPos, m_lstMarker.at(nMarker).m_nRectPos, strLabel);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void drawCursor(QPainter& painter, float fMarkerPos, const QString& strLabel)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::drawCursor(QPainter& painter, int nMarkerPos, int nTextPos, const QString& strLabel)
{
	QRect rcCursor;
	QRect rcBounding	= fontMetrics().boundingRect(strLabel);

	rcCursor.setTop(nTextPos - 10);
	rcCursor.setBottom(rcCursor.top() + rcBounding.height());
	rcCursor.setLeft(m_rcScaleColor.left() + 40);
	rcCursor.setRight(rcCursor.left() + rcBounding.width());

	painter.drawRect(rcCursor);
	painter.fillRect(rcCursor, QBrush(QColor(255, 255, 204)));
	painter.drawPixmap(m_rcScaleColor.left() + 25, nMarkerPos - 4, m_pixLeftCursor.width() , m_pixLeftCursor.height(), m_pixLeftCursor);
	painter.drawLine(m_rcScaleColor.left() + 31, nMarkerPos, m_rcScaleColor.left() + 39, nTextPos);
	painter.drawText(rcCursor, strLabel);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void updateMarkerPos()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::updateMarkerPos()
{
	if (m_pTestCell)
	{
		bool	bParametric = (m_pTestCell->lTestNumber < GEX_TESTNBR_OFFSET_EXT_SBIN || m_pTestCell->lTestNumber > GEX_TESTNBR_OFFSET_EXT_TEST_TIME_T) ? true : false;
		double	dRange		= m_dMaxRange - m_dMinRange;

		// reset all marker position
		m_lstMarker.clear();

		m_fLowLimitMarkerPos	= OUT_OF_SCALE;
		m_fHighLimitMarkerPos	= OUT_OF_SCALE;
		m_fMinValueMarkerPos	= OUT_OF_SCALE;
		m_fMaxValueMarkerPos	= OUT_OF_SCALE;
		m_fMeanMarkerPos		= OUT_OF_SCALE;
		m_fSelectionMarkerPos	= OUT_OF_SCALE;

		if (bParametric)
		{
			if (m_dLowLimit >= m_dMinRange && m_dLowLimit <= m_dMaxRange)
			{
				if (dRange)
					m_fLowLimitMarkerPos = (m_dLowLimit - m_dMinRange) / dRange;
				else
					m_fLowLimitMarkerPos = 0.0;

				m_lstMarker.append(GexPrivateParametricMarker(GexPrivateParametricMarker::lowLimit, m_fLowLimitMarkerPos));
			}

			if (m_dHighLimit >= m_dMinRange && m_dHighLimit <= m_dMaxRange)
			{
				if (dRange)
					m_fHighLimitMarkerPos = (m_dHighLimit - m_dMinRange) / dRange;
				else
					m_fHighLimitMarkerPos = 0.0;


				m_lstMarker.append(GexPrivateParametricMarker(GexPrivateParametricMarker::highLimit, m_fHighLimitMarkerPos));
			}

			if (m_dMean >= m_dMinRange && m_dMean <= m_dMaxRange)
			{
				if (dRange)
					m_fMeanMarkerPos = (m_dMean - m_dMinRange) / dRange;
				else
					m_fMeanMarkerPos = 0.0;

				m_lstMarker.append(GexPrivateParametricMarker(GexPrivateParametricMarker::meanValue, m_fMeanMarkerPos));
			}
		}

		if (m_dMinValue >= m_dMinRange && m_dMinValue <= m_dMaxRange)
		{
			if (dRange)
				m_fMinValueMarkerPos = (m_dMinValue - m_dMinRange) / dRange;
			else
				m_fMinValueMarkerPos = 0.0;

			m_lstMarker.append(GexPrivateParametricMarker(GexPrivateParametricMarker::minimumValue, m_fMinValueMarkerPos));
		}

		if (m_dMaxValue >= m_dMinRange && m_dMaxValue <= m_dMaxRange)
		{
			if (dRange)
				m_fMaxValueMarkerPos = (m_dMaxValue - m_dMinRange) / dRange;
			else
				m_fMaxValueMarkerPos = 0.0;

			m_lstMarker.append(GexPrivateParametricMarker(GexPrivateParametricMarker::maximumValue, m_fMaxValueMarkerPos));
		}

		if (m_dSelection >= m_dMinRange && m_dSelection <= m_dMaxRange)
		{
			if (dRange)
				m_fSelectionMarkerPos = (m_dSelection - m_dMinRange) / dRange;
			else
				m_fSelectionMarkerPos = 0.0;

			m_lstMarker.append(GexPrivateParametricMarker(GexPrivateParametricMarker::selectedValue, m_fSelectionMarkerPos));
		}

		qSort(m_lstMarker.begin(), m_lstMarker.end());

		m_bNeedUpdate = true;

		computeMarker();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void computeMarker()
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexParametricScaleColorWidget::computeMarker()
{
	for (int nMarker = 0; nMarker < m_lstMarker.count(); nMarker++)
	{
    m_lstMarker[nMarker].m_nCursorPos =
      m_rcScaleColor.bottom() - (int) (m_rcScaleColor.height() * m_lstMarker[nMarker].m_fMarkerPos);
		m_lstMarker[nMarker].m_nRectPos		= m_lstMarker[nMarker].m_nCursorPos;
	}

	for (int nMarker = 0; nMarker < m_lstMarker.count(); nMarker++)
	{
		if (nMarker > 0)
		{
			if (m_lstMarker[nMarker].m_nRectPos - 20 < m_lstMarker[nMarker-1].m_nRectPos)
				m_lstMarker[nMarker].m_nRectPos = m_lstMarker[nMarker-1].m_nRectPos + 20;
		}
	}

	for (int nMarker = m_lstMarker.count()-1; nMarker >= 0; nMarker--)
	{
		if (m_lstMarker[nMarker].m_nRectPos > m_rcScaleColor.bottom())
			m_lstMarker[nMarker].m_nRectPos = m_rcScaleColor.bottom();

		if (nMarker > 0)
		{
			if (m_lstMarker[nMarker].m_nRectPos - 20 < m_lstMarker[nMarker-1].m_nRectPos)
				m_lstMarker[nMarker-1].m_nRectPos = m_lstMarker[nMarker].m_nRectPos - 20;
		}
	}
}

GexParametricScaleColorWidget::GexPrivateParametricMarker::GexPrivateParametricMarker(markerType eType, float fMarkerPos)
{
	m_eMarkerType	= eType;
	m_fMarkerPos	= fMarkerPos;
	m_nCursorPos	= OUT_OF_SCALE;
	m_nRectPos		= OUT_OF_SCALE;
}

GexParametricScaleColorWidget::GexPrivateParametricMarker::GexPrivateParametricMarker(const GexPrivateParametricMarker& marker)
{
	m_eMarkerType	= marker.m_eMarkerType;
	m_fMarkerPos	= marker.m_fMarkerPos;
	m_nCursorPos	= marker.m_nCursorPos;
	m_nRectPos		= marker.m_nRectPos;
}

GexParametricScaleColorWidget::GexPrivateParametricMarker::~GexPrivateParametricMarker()
{

}

bool GexParametricScaleColorWidget::GexPrivateParametricMarker::operator<(const GexPrivateParametricMarker& marker) const
{
	if (marker.m_fMarkerPos == m_fMarkerPos)
		return marker.m_eMarkerType < m_eMarkerType;
	else
		return marker.m_fMarkerPos < m_fMarkerPos;
}
