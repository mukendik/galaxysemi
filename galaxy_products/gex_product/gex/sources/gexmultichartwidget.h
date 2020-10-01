#ifndef _GEX_MULTICHART_WIDGET_H_
#define _GEX_MULTICHART_WIDGET_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexsinglechartwidget.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QGridLayout>

class GexWizardChart;
class GexMultiChartWidget : public QWidget
{
    Q_OBJECT

public:

    GexMultiChartWidget(QWidget * pParent = NULL);
    ~GexMultiChartWidget();

    void	setMouseMode        (GexSingleChartWidget::MouseMode mouseMode);
    void	setViewportMode     (int nViewportMode);
    void    init                (GexWizardChart*   lWizardParent);

    void	resetViewPortManager(bool bEraseCustomViewport = false);
    void	resetChart          (bool bKeepViewport);

    void	onChartOptionChanged(GexAbstractChart::chartOption eChartOption);

public slots:

    void	replotChart( CReportOptions* lOption,  CTest* lTestCellX, CTest* lTestCellY, const GexSingleChartWidget * pSingleChartWidget = NULL);

private:

    GexSingleChartWidget *		m_pHistoChart;
    GexSingleChartWidget *		m_pTrendChart;
    GexSingleChartWidget *		m_pProbabilityPlotChart;
    GexSingleChartWidget *		m_pBoxPlotChart;
    QGridLayout *				m_pGridLayout;
    GexWizardChart*             mWizardParent;

protected slots:

    void				onKeepViewportChanged(bool isChecked);

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
    void				rebuildReport(const QString&);
    void				keepViewportChanged(bool);
};

#endif // _GEX_MULTICHART_WIDGET_H_
