#include <qimage.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qcombobox.h>

#ifdef _CHARTDIR_TEST
#include <chartdir.h>
#endif

#include "chartdirector_dialog.h"

///////////////////////////////////////////////////////////
// OBJECT: ChartDirectorFrame
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
ChartDirectorFrame::ChartDirectorFrame( QWidget* parent, Qt::WindowFlags fl )
    : QFrame( parent, fl )
{
	// Default chart type
	m_nChartType = 2;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
ChartDirectorFrame::~ChartDirectorFrame()
{
}

///////////////////////////////////////////////////////////
// Update graph
///////////////////////////////////////////////////////////
void ChartDirectorFrame::UpdateChart(int nChartType)
{
	if(nChartType != m_nChartType)
	{
		m_nChartType = nChartType;
		update();
	}
}

void ChartDirectorFrame::paintEvent(QPaintEvent* /*pPaintEvent*/)
{
#ifdef _CHARTDIR_TEST
	switch(m_nChartType)
	{
		case 0:
			DrawBasicTestChart();
			break;

		case 1:
			DrawBasicHistogram();
			break;

		case 2:
			DrawBasicTrendChart();
			break;

		default:
			DrawUnderConstruction("Unknown");
			break;
	}
#else
	DrawChartDirDisabled();
#endif
}

void ChartDirectorFrame::DrawBasicTestChart()
{
#ifdef _CHARTDIR_TEST
	//The values to display on the meter
	long	lWidth=250,lHeight=75;

    double value0 = 30.99;
    double value1 = 45.35;
    double value2 = 77.64;

    //Create an LinearMeter object of size 250 x 75 pixels, using silver
    //background with a 2 pixel black 3D depressed border.
    LinearMeter *lm = new LinearMeter(lWidth, lHeight, Chart::silverColor(), 0, -2);

    //Set the scale region top-left corner at (15, 25), with size of 220 x 20
    //pixels. The scale labels are located on the top (implies horizontal meter)
    lm->setMeter(15, 25, 220, 20, Chart::Top);

    //Set meter scale from 0 - 100, with a tick every 10 units
    lm->setScale(0, 100, 10);

    //Set 0 - 50 as green (99ff99) zone, 50 - 80 as yellow (ffff66) zone, and 80
    //- 100 as red (ffcccc) zone
    lm->addZone(0, 50, 0x99ff99);
    lm->addZone(50, 80, 0xffff66);
    lm->addZone(80, 100, 0xffcccc);

    //Add deep red (000080), deep green (008000) and deep blue (800000) pointers
    //to reflect the values
    lm->addPointer(value0, 0x80);
    lm->addPointer(value1, 0x8000);
    lm->addPointer(value2, 0x800000);

    //Add a label at bottom-left (10, 68) using Arial Bold/8 pts/red (c00000)
    lm->addText(10, 68, "Temp C", "arialbd.ttf", 8, 0xc00000, Chart::BottomLeft);

    //Add three text boxes to show the values in this meter
    lm->addText(148, 70, lm->formatValue(value0, "2"), "arial.ttf", 8, 0x6666ff,
        Chart::BottomRight)->setBackground(0, 0, -1);
    lm->addText(193, 70, lm->formatValue(value1, "2"), "arial.ttf", 8, 0x33ff33,
        Chart::BottomRight)->setBackground(0, 0, -1);
    lm->addText(238, 70, lm->formatValue(value2, "2"), "arial.ttf", 8, 0xff3333,
        Chart::BottomRight)->setBackground(0, 0, -1);

    //Output chart as Device Indpendent Bitmap with file header
    MemBlock m = lm->makeChart(Chart::BMP);

	QImage	cImage;
	cImage.loadFromData((unsigned char*) m.data,m.len);

    //Get the device context
	QPainter p;
	QPoint pPoint(10, 10);
	p.begin( this );
	p.drawImage(pPoint, cImage);
	p.end();

    //free up resources
    delete lm;
#endif
}

void ChartDirectorFrame::DrawBasicHistogram()
{
#ifdef _CHARTDIR_TEST
	//The data for the bar chart
	double data[] = {85, 156, 179.5, 211, 123};

	//The labels for the bar chart
	const char *labels[] = {"Mon", "Tue", "Wed", "Thu", "Fri"};

	//Create a XYChart object of size 250 x 250 pixels
	XYChart *c = new XYChart(250, 250);

	//Set the plotarea at (30, 20) and of size 200 x 200 pixels
	c->setPlotArea(30, 20, 200, 200);

	//Add a bar chart layer using the given data
	c->addBarLayer(DoubleArray(data, sizeof(data)/sizeof(data[0])));

	//Set the labels on the x axis.
	c->xAxis()->setLabels(StringArray(labels, sizeof(labels)/sizeof(labels[0])))
        ;
    //Output chart as Device Indpendent Bitmap with file header
    MemBlock m = c->makeChart(Chart::BMP);

	QImage	cImage;
	cImage.loadFromData((unsigned char*) m.data,m.len);

    //Get the device context
	QPainter p;
	QPoint pPoint(10, 10);
	p.begin( this );
	p.drawImage(pPoint, cImage);
	p.end();

	//free up resources
	delete c;
#endif
}

void ChartDirectorFrame::DrawBasicTrendChart()
{
	DrawUnderConstruction("Basic Trend chart");
}

void ChartDirectorFrame::DrawUnderConstruction(const QString & strChartName)
{
	QString strMessage;

	strMessage.sprintf("Chart: %s\n\nUNDER CONSTRUCTION...", strChartName.toLatin1().constData());

	QPainter p;														// our painter
	p.begin( this );												// start painting the widget
	p.setPen( Qt::red );											// red outline
	p.setBrush( Qt::yellow );											// yellow fill
	p.setFont(QFont("Times New Roman", 24, QFont::Normal, true));	// font
	p.drawText(rect(), Qt::AlignCenter, strMessage);					// text
	p.end();														// painting done
}

void ChartDirectorFrame::DrawChartDirDisabled()
{
	QString strMessage;

	strMessage = "ChartDir is not enabled...";

	QPainter p;														// our painter
	p.begin( this );												// start painting the widget
	p.setPen( Qt::red );												// red outline
	p.setBrush( Qt::yellow );											// yellow fill
	p.setFont(QFont("Times New Roman", 24, QFont::Normal, true));	// font
	p.drawText(rect(), Qt::AlignCenter, strMessage);					// text
	p.end();														// painting done
}

///////////////////////////////////////////////////////////
// OBJECT: ChartDirectorDiamog
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
ChartDirectorDialog::ChartDirectorDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	QObject::connect(comboChartType,	SIGNAL(activated(int)), this, SLOT(OnChartSelection(int)));
    QObject::connect(buttonOk,			SIGNAL(clicked()),		this, SLOT(accept()));

	// Add frame to display ChartDirector graphs
    m_pFrame = new ChartDirectorFrame(this);
    m_pFrame->setFrameShape( QFrame::StyledPanel );
    m_pFrame->setFrameShadow( QFrame::Raised );
    //layout4->addWidget( m_pFrame );

	comboChartType->setCurrentIndex(2);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
ChartDirectorDialog::~ChartDirectorDialog()
{
	delete m_pFrame; m_pFrame=NULL;
}

///////////////////////////////////////////////////////////
// Chart type selected
///////////////////////////////////////////////////////////
void ChartDirectorDialog::OnChartSelection(int nChartType)
{
	m_pFrame->UpdateChart(nChartType);
}
