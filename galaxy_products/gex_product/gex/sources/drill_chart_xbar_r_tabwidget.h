#ifndef DRILL_CHART_XBAR_R_TABWIDGET_H
#define DRILL_CHART_XBAR_R_TABWIDGET_H

///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexsinglechartwidget.h"

namespace Ui {
	class drill_chart_xbar_r_tabwidget;
}

class GexWizardChart;
class GexXBarRTabWidget : public QWidget
{
    Q_OBJECT

public:
	explicit GexXBarRTabWidget(QWidget *parent = 0);
	~GexXBarRTabWidget();

	void	setMouseMode(GexSingleChartWidget::MouseMode mouseMode);
	void	setViewportMode(int nViewportMode);

	void	resetViewPortManager(bool bEraseCustomViewport = false);
    //void    setChartsInfo(CGexChartOverlays*	lChartsInfo);
    void    init(GexWizardChart*     lWizardParent);

public slots:

	void	replotChart(const GexSingleChartWidget * pSingleChartWidget = NULL);

protected:

	void	home(bool bEraseCustomViewport = false);

private slots:

	void	onSelect();
	void	onMove();
	void	onZoom();
	void	onHome();
	void	onControlChartChanged(int);
	void	onViewportChanged(int);
	void	onMouseModeChanged(GexSingleChartWidget::MouseMode);

private:
	Ui::drill_chart_xbar_r_tabwidget * ui;

    GexWizardChart*     mWizardParent;
   // CGexChartOverlays* mChartsInfo;
};

#endif // DRILL_CHART_XBAR_R_TABWIDGET_H
