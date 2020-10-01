///////////////////////////////////////////////////////////
// Dialog box to Edit Chart options : Chart Title, Axis legends,...
///////////////////////////////////////////////////////////
#include "drill_editchart_options.h"
#include "browser_dialog.h"

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
EditChartOptionsDialog::EditChartOptionsDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);
	
	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(pushButtonOk,			SIGNAL(clicked()),		this, SLOT(OnOk()));
    QObject::connect(groupBoxChartTitle,	SIGNAL(toggled(bool)),	this, SLOT(OnToggleTitle(bool)));
    QObject::connect(groupBoxChartLegendX,	SIGNAL(toggled(bool)),	this, SLOT(OnToggleAxisX(bool)));
    QObject::connect(groupBoxChartLegendY,	SIGNAL(toggled(bool)),	this, SLOT(OnToggleAxisY(bool)));
}


///////////////////////////////////////////////////////////
// Title check box clicked
///////////////////////////////////////////////////////////
void EditChartOptionsDialog::OnToggleTitle(bool bState)
{
	if(bState)
		lineEditChartTitle->setFocus();
}

///////////////////////////////////////////////////////////
// X-Axis check box clicked
///////////////////////////////////////////////////////////
void EditChartOptionsDialog::OnToggleAxisX(bool bState)
{
	if(bState)
		lineEditAxisLegendX->setFocus();
}

///////////////////////////////////////////////////////////
// Y-Axis  check box clicked
///////////////////////////////////////////////////////////
void EditChartOptionsDialog::OnToggleAxisY(bool bState)
{
	if(bState)
		lineEditAxisLegendY->setFocus();
}

///////////////////////////////////////////////////////////
// Loads GUI with up-to-date values
///////////////////////////////////////////////////////////
void EditChartOptionsDialog::setVariables(CGexChartOverlays *pChartOverlays)
{
    int iChartToBeUsed = (pChartOverlays->getAppliedToChart()!= - 1 )
            ? pChartOverlays->getAppliedToChart()
            : pChartOverlays->getViewportRectangle().begin().key();

	// Check/Uncheck relevant selections
    groupBoxChartTitle->setChecked(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bChartTitle);
    groupBoxChartLegendX->setChecked(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLegendX);
    groupBoxChartLegendY->setChecked(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLegendY);

	// Load title + XY-legend fields
    lineEditChartTitle->setText(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.strChartTitle);
    lineEditAxisLegendX->setText(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.strAxisLegendX);
    lineEditAxisLegendY->setText(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.strAxisLegendY);

	// Load scale type: Linear or Logarithmic
    if(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLogScaleX)
        comboBoxScaleX->setCurrentIndex(1);
	else
        comboBoxScaleX->setCurrentIndex(0);
    if(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLogScaleY)
        comboBoxScaleY->setCurrentIndex(1);
	else
        comboBoxScaleY->setCurrentIndex(0);

	// Load Viewport dimensions.

    lineEditLowX->setText(QString::number(pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfLowX));
    lineEditHighX->setText(QString::number(pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfHighX));
    lineEditLowY->setText(QString::number(pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfLowY));
    lineEditHighY->setText(QString::number(pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfHighY));

}

///////////////////////////////////////////////////////////
// Update structures base on GUI changes
///////////////////////////////////////////////////////////
void EditChartOptionsDialog::getVariables(CGexChartOverlays *pChartOverlays)
{
    int iChartToBeUsed = (pChartOverlays->getAppliedToChart()!= - 1 )
            ? pChartOverlays->getAppliedToChart()
            : pChartOverlays->getViewportRectangle().begin().key();

	// Update Title variable
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bChartTitle = groupBoxChartTitle->isChecked();
    if(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bChartTitle)
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.strChartTitle = lineEditChartTitle->text();

	// X axis Legend
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLegendX = groupBoxChartLegendX->isChecked();
    if(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLegendX)
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.strAxisLegendX = lineEditAxisLegendX->text();

	// Y axis Legend
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLegendY = groupBoxChartLegendY->isChecked();
    if(pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLegendY)
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.strAxisLegendY = lineEditAxisLegendY->text();

	// New viewport dimensions...
	QString strValue;
	strValue = lineEditLowX->text();
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfLowX = strValue.toDouble();
	strValue = lineEditHighX->text();
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfHighX = strValue.toDouble();
	strValue = lineEditLowY->text();
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfLowY = strValue.toDouble();
	strValue = lineEditHighY->text();
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].lfHighY = strValue.toDouble();
    pChartOverlays->getViewportRectangle()[iChartToBeUsed].bForceViewport = true;

	// Scale type: Linear or Logarithmic?
	if(comboBoxScaleX->currentIndex())
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLogScaleX = true;
	else
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLogScaleX = false;
	if(comboBoxScaleY->currentIndex())
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLogScaleY = true;
	else
        pChartOverlays->getViewportRectangle()[iChartToBeUsed].cChartOptions.bLogScaleY = false;

	// Clear zoom to this new custom viewport...
	pChartOverlays->pStart.setX(0);
	pChartOverlays->pStart.setY(100);
	pChartOverlays->pEnd.setX(0);
	pChartOverlays->pEnd.setY(100);
	pChartOverlays->lfZoomFactorX = pChartOverlays->lfZoomFactorY =  1.0;
}


///////////////////////////////////////////////////////////
// Ok button pressed: update Properties buffer
///////////////////////////////////////////////////////////
void EditChartOptionsDialog::OnOk(void)
{
	// Ok status
	done(1);
}

