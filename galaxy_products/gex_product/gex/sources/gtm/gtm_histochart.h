#ifndef GTM_HISTOCHART_H
#define GTM_HISTOCHART_H

#include <QFrame>
#include <QPaintEvent>

#define	GTM_SPC_HISTO_CELLS		20			// total Histogram bars available on chart.
#define	GTM_SPC_DATA_IN_CELL	10			// total parts per histogram cell

class Gtm_HistoChart : public QFrame
{
  Q_OBJECT
  Q_DISABLE_COPY(Gtm_HistoChart)
  int mYieldAlarm;

public:
    Gtm_HistoChart( QWidget *parent, Qt::WindowFlags fl = 0 );
    virtual ~Gtm_HistoChart(void);

    void setYieldAlarmLevel(const int lYieldAlarm) {mYieldAlarm = lYieldAlarm;}
	void clear(void);						// Resets histogram variables
	void paintEvent(QPaintEvent * event);	// Paint histogram bars

	// SPC Histogram chart.
    int	iHistoCount[GTM_SPC_HISTO_CELLS];			// Buffer holding last XX Histogram bars of Bin1
    int	iHistoBarInUse;								// Current Histo cell beeing used
};

#endif // GTM_HISTOCHART_H
