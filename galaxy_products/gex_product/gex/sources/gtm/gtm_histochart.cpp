#include <QPainter>

#include "gtm_histochart.h"

Gtm_HistoChart::Gtm_HistoChart(QWidget * parent, Qt::WindowFlags fl)
    : QFrame(parent, fl)
{
    // Reset members
    mYieldAlarm = 0;
	// Reset Histogram variables
	clear();
}

Gtm_HistoChart::~Gtm_HistoChart(void)
{

}

void Gtm_HistoChart::clear(void)
{
	int iIndex;
	for(iIndex = 0; iIndex < GTM_SPC_HISTO_CELLS; iIndex++)
		iHistoCount[iIndex] = 0;	
	iHistoBarInUse = 0;				// Current Histo cell beeing used
}

void Gtm_HistoChart::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

	QPainter p(this);	
	QSize rSize = size();
	QRect rText;
	
	// Erase background
	p.eraseRect(0,0,rSize.width(),rSize.height());

	// Draw scales
	QPen	cPen(QColor(0,0,128));	// Pen color: dark blue.
	p.setPen(cPen);

	// Horizontal base line (0%)
	p.drawLine(50,rSize.height()-3,rSize.width()-3,rSize.height()-3);
	// Legend: '0%'
	rText.setRect(0,rSize.height()-15,50,12);
	p.drawText(rText,Qt::AlignRight | Qt::AlignTop,"0%");

	// Horizontal 100%
	p.drawLine(50,3,rSize.width()-3,3);
	// Legend: '100%'
	rText.setRect(0,3,50,12);
	p.drawText(rText,Qt::AlignRight | Qt::AlignBottom,"100%");
	
	// Vertical line
	p.drawLine(50,0,50,rSize.height()-3);

    // Plot sub-grid (in light grey color)
	cPen.setColor(QColor(192,192,192));
	cPen.setStyle(Qt::DotLine);
	p.setPen(cPen);
	int	iIndex;
	long iY;
	for(iIndex = 20; iIndex <= 80; iIndex += 20)
	{
		iY = ((rSize.height()-6) * iIndex)/100;
		p.drawLine(50,rSize.height()-3-iY,rSize.width()-3,rSize.height()-3-iY);
	}

	// Plot Histogram blocks (20 bars history)
	int	iHistoBar;
	int	iStepSize = (rSize.width()-50) / 20;
	// Reset pen style to solid line
	cPen.setStyle(Qt::SolidLine);
	p.setPen(cPen);
	// Brush will be either green or red (depends if bar is below or over the yield alarm threshold)
	float	fBarPercentage;
	for(iHistoBar=0;iHistoBar < GTM_SPC_HISTO_CELLS; iHistoBar++)
	{
		// Get random level for the bar
		fBarPercentage = ((float) iHistoCount[iHistoBar] * 100.0) / ((float) GTM_SPC_DATA_IN_CELL);

		// Define brush filling color
        if(mYieldAlarm > 0 && fBarPercentage < mYieldAlarm)
		{
            p.setBrush(QColor(198,0,0));	// Red bar (filling color)
            cPen.setColor(QColor(100,0,0));	// Dark red (outline)
			p.setPen(cPen);
		}
		else
		{
            p.setBrush(QColor(0,198,0));	// Green bar (filling)
            cPen.setColor(QColor(0,100,0));	// Dark green (outline)
			p.setPen(cPen);
		}

		// Compute bar Heigth
		iY = (long)(((float)(rSize.height()-6) * fBarPercentage)/100.0);

		// Draw histogram bar (filled)
		p.drawRect(50 + (iHistoBar*iStepSize),rSize.height()-3,iStepSize,-iY);
	}

	// Draw yield alarm threshold (if any)
    if(mYieldAlarm > 0)
	{
		// Set custom line color
		cPen.setColor(QColor(0,0,128));
		p.setPen(cPen);
		
		// Horizontal line of X% (Yield alarm threshold)
        iY = rSize.height()-3 - (((rSize.height()-6) * mYieldAlarm)/100);
		p.drawLine(50,iY,rSize.width()-3,iY);
		
		// Legend: 'XX%'
		rText.setRect(0,iY,50,12);
        QString strString = QString::number(mYieldAlarm) + "%"; //+ parent()->;
		p.drawText(rText,Qt::AlignRight | Qt::AlignBottom,strString);

	}

//	p.end();
}
