///////////////////////////////////////////////////////////
// Dialog box to Edit layer properties, parameters names, style,...
///////////////////////////////////////////////////////////
#include "browser_dialog.h"
#include "drill_chart_editlayer.h"
#include "report_build.h"
#include "gex_report.h"
#include "picktest_dialog.h"
#include "gex_group_of_files.h"

#include <qcolordialog.h>

// in report_build.cpp
extern CGexReport* gexReport;			// Handle to report class

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
EditLayerDialog::EditLayerDialog( QWidget* parent, bool modal, Qt::WindowFlags f ) 
	: QDialog( parent, f )
{
	setupUi(this);
	setModal(modal);
	
	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(buttonPickTestX,		SIGNAL(clicked()),				this, SLOT(OnPickTestX()));
    QObject::connect(buttonPickTestY,		SIGNAL(clicked()),				this, SLOT(OnPickTestY()));
    QObject::connect(buttonPickTestZ,		SIGNAL(clicked()),				this, SLOT(OnPickTestZ()));
    QObject::connect(pushButtonOk,			SIGNAL(clicked()),				this, SLOT(OnOk()));
    QObject::connect(lineEditLayerLabel,	SIGNAL(textChanged(QString)),	this, SLOT(OnLayerTitle(QString)));
    QObject::connect(comboBoxMarker,		SIGNAL(activated(int)),			this, SLOT(OnSelectMarker(int)));
    QObject::connect(spinBoxMarkerWidth,	SIGNAL(valueChanged(int)),		this, SLOT(OnMarkerWidth(int)));
    QObject::connect(pushButtonMarkerColor, SIGNAL(colorChanged(const QColor&)),	this, SLOT(OnMarkerColorChanged(const QColor&)));

	// Focus on Layer name field
	lineEditLayerLabel->setFocus();

	// Clear Parameters names
	
	lineEditParamNameX->setText("- none -");
	lineEditParamNameY->setText("- none -");
	lineEditParamNameZ->setText("- none -");
	lineEditParamLabelX->setText("- none -");
	lineEditParamLabelY->setText("- none -");
	lineEditParamLabelZ->setText("- none -");
	lineEditParamDataOffset->setText("0");
	lineEditParamDataScale->setText("1");

	// clear flags
	m_bNewParameterSelected = false;
	m_MarkerSelected=0;	// At pop-up time, Marker entry shown is 'Mean' (entry 0).

	// Startup options
	checkBoxBars->setChecked(true);				// Bars check-box
	checkBoxfittingCurve->setChecked(false);	// Fitting curve check-box
	checkBoxLines->setChecked(true);			// Lines check-box
	checkBoxspots->setChecked(true);			// Spots check-box
	spinBoxLineWidth->setValue(1);				// Set line width
	OnSelectMarker(m_MarkerSelected);			// Show 'Mean' Marker drawing attibutes.

	// For now: NO Z AXIS
	groupBoxZ->hide();
}

///////////////////////////////////////////////////////////
// Preset styles according to layer index...
///////////////////////////////////////////////////////////
void EditLayerDialog::resetVariables(int iEntry)
{
	cLayer.resetVariables(iEntry);

	// Refresh Marker GUI as the color & width may be different now.
	OnSelectMarker(m_MarkerSelected);
}

///////////////////////////////////////////////////////////
// Returns 'true' if one or more axis have a new parameter (so need to redraw chart)
///////////////////////////////////////////////////////////
bool EditLayerDialog::needResetViewport(void)
{
	return m_bNewParameterSelected;
}

///////////////////////////////////////////////////////////
// Fill Dialog box contents
///////////////////////////////////////////////////////////
void EditLayerDialog::setLayerInfo(CGexSingleChart *pLayer, int nChartType)
{
	// Keep a copy of layer's info in our private buffer
	cLayer = *pLayer;

	// Layer name
	lineEditLayerLabel->setText(pLayer->strChartName);

	// X,Y,Z Parameters assigned
	lineEditParamNameX->setText(pLayer->strTestNameX);
	lineEditParamNameY->setText(pLayer->strTestNameY);
	lineEditParamNameZ->setText(pLayer->strTestNameZ);
	lineEditParamLabelX->setText(pLayer->strTestLabelX);
	lineEditParamLabelY->setText(pLayer->strTestLabelY);
	lineEditParamLabelZ->setText(pLayer->strTestLabelZ);

	// Drawing mode / Style
	pushButtonLineColor->setActiveColor(pLayer->cColor);
	checkBoxHide->setChecked(!pLayer->bVisible);
	checkBoxBars->setChecked(pLayer->bBoxBars);				// Bars check-box
	checkBox3DBars->setChecked(pLayer->bBox3DBars);			// 3D-Bars check-box
	checkBoxfittingCurve->setChecked(pLayer->bFittingCurve);// Fitting curve check-box
	checkBoxBellCurve->setChecked(pLayer->bBellCurve);		// Bell curve check-box
	checkBoxLines->setChecked(pLayer->bLines);				// Lines check-box
	checkBoxspots->setChecked(pLayer->bSpots);				// Spots check-box
	spinBoxLineWidth->setValue(pLayer->iLineWidth);		// Set line width
    comboBoxLineStyle->setCurrentIndex(pLayer->iLineStyle);	// Set line style: slid, dashed, etc..
    comboBoxLineSpotStyle->setCurrentIndex(pLayer->iSpotStyle);	// Set spot style: circle, rectangle, diamond, etc..
	
	switch(pLayer->iWhiskerMode)
	{
		case GEX_WHISKER_RANGE:
            comboBoxWhiskerType->setCurrentIndex(0);	// Range
			break;
		case GEX_WHISKER_Q1Q3:
            comboBoxWhiskerType->setCurrentIndex(1);	// Q1 - 1.5*IQR, Q3 + 1.5*IQR
			break;
		case GEX_WHISKER_IQR:
            comboBoxWhiskerType->setCurrentIndex(2);	// Q2 +/- 1.5*IQR
			break;
	}

	// Offset to apply to data in layer (trend chart only)
	lineEditParamDataOffset->setText(QString::number(pLayer->lfDataOffset,'g'));
	// Scale factor to apply to data in layer (trend chart only)
	lineEditParamDataScale->setText(QString::number(pLayer->lfDataScale,'g'));

	// Refreh GUI
	OnLayerTitle(pLayer->strChartName);
	// Refresh Marker GUI as the color & width may be different now.
	OnSelectMarker(m_MarkerSelected);

	// Enable/Disable  GUI widgets
	switch(nChartType)
	{
		default:
		case GEX_ADV_HISTOGRAM:
		case GEX_ADV_TREND:
		case GEX_ADV_CORRELATION:
			widgetStackDrawingMode->show();
			widgetStackDrawingMode->setCurrentIndex(0);	// Histo, Trend & scatter drawing mode options
			break;
		
		case GEX_ADV_CANDLE_MEANRANGE:
			widgetStackDrawingMode->show();
			widgetStackDrawingMode->setCurrentIndex(1);	// Boxplot drawing mode options
			break;

		case GEX_ADV_PROBABILITY_PLOT:
			widgetStackDrawingMode->hide();
			break;
	}
}

///////////////////////////////////////////////////////////
// Retrieve Dialog box contents
///////////////////////////////////////////////////////////
void EditLayerDialog::getLayerInfo(CGexSingleChart *pLayer)
{
	// Retrieve info from GUI
	cLayer.strTestLabelX = lineEditParamLabelX->text();
	cLayer.strTestLabelY = lineEditParamLabelY->text();
	cLayer.strTestLabelZ = lineEditParamLabelZ->text();

	// Return 
	*pLayer = cLayer;
}

///////////////////////////////////////////////////////////
// Retrieve Dialog box contents
///////////////////////////////////////////////////////////
CGexSingleChart *EditLayerDialog::getLayerInfo(void)
{
	// Return 
	return &cLayer;
}

///////////////////////////////////////////////////////////
// Layer title changed...
///////////////////////////////////////////////////////////
void EditLayerDialog::OnLayerTitle(const QString &strTitle)
{
	if(strTitle.length() > 0)
		pushButtonOk->setEnabled(true);
	else
		pushButtonOk->setEnabled(false);
	
	// Update internal buffer.
	cLayer.strChartName = strTitle;
}

///////////////////////////////////////////////////////////
// Select Parameter to chart in X, or Y, or Z
///////////////////////////////////////////////////////////
bool EditLayerDialog::OnPickTest(unsigned int &lLayerTestNumber,int &lLayerPinmapIndex,int &iGroupID,QString &strLayerTestName,QString &strLayerLabel)
{
	// Show TestList: Single selection mode.
	PickTestDialog dPickTest;
	
	// Check if List was successfuly loaded
	if(dPickTest.fillParameterList() == true)
	{
		// Disable selection of multiple layers: user can only pick a test from a given layer
		dPickTest.setMultipleGroups(true,false);
		// Prompt dialog box, let user pick tests from the list
		if(dPickTest.exec() == QDialog::Accepted)
		{
			// Get test# selected. string format: <Test#>.<Pinmap#> , <Test#>.<Pinmap#> , etc...
			QString		strTestsSelected		= dPickTest.testItemizedList();
			QStringList strTestsNamesSelected	= dPickTest.testItemizedListNames();

			if(strTestsSelected.isEmpty() == false)
			{
				// Save Save Parameter selected
				long	lTestNumber,lPinmapIndex;
				if(sscanf(strTestsSelected.toLatin1().constData(),"%lu%*c%ld",&lTestNumber,&lPinmapIndex) < 2)
					lPinmapIndex = GEX_PTEST;	// No Pinmap index specified.

				lLayerTestNumber = lTestNumber;
				lLayerPinmapIndex = lPinmapIndex;
				iGroupID = dPickTest.getGroupID();
				strLayerTestName = strTestsNamesSelected[0];

				// Update GUI: Show Test# & Test name.
				strLayerLabel = "";
                if(gexReport->getGroupsList().count() > 1 && iGroupID >= 0 && iGroupID < gexReport->getGroupsList().count())
				{
					// If multi layers, show layer name in test label....
                    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().at(iGroupID);
					if(pGroup != NULL)
						strLayerLabel += pGroup->strGroupName + " - ";
				}

				strLayerLabel += "Test " + QString::number(lTestNumber);
				if(lPinmapIndex != GEX_PTEST)
				{
					strLayerLabel += ".";
					strLayerLabel += QString::number(lPinmapIndex);
				}
				strLayerLabel += " : ";
				strLayerLabel += strTestsNamesSelected[0];

				// Flags that one axis now holds a new parameter...so Charts will need to be repaint + clear zoom.
				m_bNewParameterSelected = true;

				return true;
			}
		}
	}else
		dPickTest.setMultipleGroups(true,false);

	return false;
}

///////////////////////////////////////////////////////////
// Select Parameter to chart in X
///////////////////////////////////////////////////////////
void EditLayerDialog::OnPickTestX(void)
{
	QString strParameterFullName;

	// Get Parameter name to plot in X
	if(OnPickTest(cLayer.iTestNumberX,cLayer.iPinMapX,cLayer.iGroupX,cLayer.strTestNameX,strParameterFullName) == false)
		return;

	// Update GUI: Show Test# & Test name.
	lineEditParamNameX->setText(strParameterFullName);
	lineEditParamLabelX->setText(strParameterFullName);

	// Title is forced to the Test name in X...unless it already includes a string
	if(lineEditLayerLabel->text().isEmpty() == true)
		lineEditLayerLabel->setText(strParameterFullName);
}

///////////////////////////////////////////////////////////
// Select Parameter to chart in Y
///////////////////////////////////////////////////////////
void EditLayerDialog::OnPickTestY(void)
{
	QString strParameterFullName;

	// Get Parameter name to plot in Y
	if(OnPickTest(cLayer.iTestNumberY,cLayer.iPinMapY,cLayer.iGroupY,cLayer.strTestNameY,strParameterFullName) == false)
		return;

	// Update GUI: Show Test# & Test name.
	lineEditParamNameY->setText(strParameterFullName);
	lineEditParamLabelY->setText(strParameterFullName);
}

///////////////////////////////////////////////////////////
// Select Parameter to chart in Z
///////////////////////////////////////////////////////////
void EditLayerDialog::OnPickTestZ(void)
{
	QString strParameterFullName;

	// Get Parameter name to plot in Z
	if(OnPickTest(cLayer.iTestNumberZ,cLayer.iPinMapZ,cLayer.iGroupZ,cLayer.strTestNameZ,strParameterFullName) == false)
		return;

	// Update GUI: Show Test# & Test name.
	lineEditParamNameZ->setText(strParameterFullName);
	lineEditParamLabelZ->setText(strParameterFullName);
}

///////////////////////////////////////////////////////////
// Editing a marker info...
///////////////////////////////////////////////////////////
void EditLayerDialog::OnSelectMarker(int iMarker)
{
	// Keep track of marker currently selected
	m_MarkerSelected = iMarker;

	// Show style associated with this marker
	QColor	cColor;
	int		iLineWidth = 0;
	switch(m_MarkerSelected)
	{
		case 0: // Mean
			cColor		= cLayer.meanColor();
			iLineWidth	= cLayer.meanLineWidth();
			break;
		case 1: // Limits
			cColor		= cLayer.limitsColor();
			iLineWidth	= cLayer.limitsLineWidth();
			break;
		case 2: // Min.
			cColor		= cLayer.minColor();
			iLineWidth	= cLayer.minLineWidth();
			break;
		case 3: // Max.
			cColor		= cLayer.maxColor();
			iLineWidth	= cLayer.maxLineWidth();
			break;
		case 4: // Median
			cColor		= cLayer.medianColor();
			iLineWidth	= cLayer.medianLineWidth();
			break;
		case 5: // +/- 1sigma
			cColor		= cLayer.sigma2Color();
			iLineWidth	= cLayer.sigma2LineWidth();
			break;
		case 6: // +/- 1.5sigma
			cColor		= cLayer.sigma3Color();
			iLineWidth	= cLayer.sigma3LineWidth();
			break;
		case 7: // +/- 3sigma
			cColor		= cLayer.sigma6Color();
			iLineWidth	= cLayer.sigma6LineWidth();
			break;
		case 8: // +/- 6sigma
			cColor		= cLayer.sigma12Color();
			iLineWidth	= cLayer.sigma12LineWidth();
			break;
	}

	// Update GUI objects
	spinBoxMarkerWidth->setValue(iLineWidth);
	pushButtonMarkerColor->setActiveColor(cColor);
}

///////////////////////////////////////////////////////////
// Editing a marker Line width...
///////////////////////////////////////////////////////////
void EditLayerDialog::OnMarkerWidth(int /*iwidth*/)
{
	// Update internal structure to keep track if the new Marker line width
	int iLineWidth = spinBoxMarkerWidth->value();

	// Save Line Width in relevant marker variable
	switch(m_MarkerSelected)
	{
		case 0: // Mean
			cLayer.setMeanLineWidth(iLineWidth);
			break;
		case 1: // Limits
			cLayer.setLimitsLineWidth(iLineWidth);
			break;
		case 2: // Min.
			cLayer.setMinLineWidth(iLineWidth);
			break;
		case 3: // Max.
			cLayer.setMaxLineWidth(iLineWidth);
			break;
		case 4: // Median
			cLayer.setMedianLineWidth(iLineWidth);
			break;
		case 5: // +/- 1sigma
			cLayer.set2SigmaLineWidth(iLineWidth);
			break;
		case 6: // +/- 1.5sigma
			cLayer.set3SigmaLineWidth(iLineWidth);
			break;
		case 7: // +/- 3sigma
			cLayer.set6SigmaLineWidth(iLineWidth);
			break;
		case 8: // +/- 6sigma
			cLayer.set12SigmaLineWidth(iLineWidth);
			break;
	}
}

///////////////////////////////////////////////////////////
// Change the Marker drawing color for a given layer.
///////////////////////////////////////////////////////////
void EditLayerDialog::OnMarkerColorChanged(const QColor& colorChanged)
{
    // Save color in relevant marker variable
	switch(m_MarkerSelected)
	{
		case 0: // Mean
			cLayer.setMeanColor(colorChanged);
			break;
		case 1: // Limits
			cLayer.setLimitsColor(colorChanged);
			break;
		case 2: // Min.
			cLayer.setMinColor(colorChanged);
			break;
		case 3: // Max.
			cLayer.setMaxColor(colorChanged);
			break;
		case 4: // Median
			cLayer.setMedianColor(colorChanged);
			break;
		case 5: // +/- 1sigma
			cLayer.set2SigmaColor(colorChanged);
			break;
		case 6: // +/- 1.5sigma
			cLayer.set3SigmaColor(colorChanged);
			break;
		case 7: // +/- 3sigma
			cLayer.set6SigmaColor(colorChanged);
			break;
		case 8: // +/- 6sigma
			cLayer.set12SigmaColor(colorChanged);
			break;
	}
}

///////////////////////////////////////////////////////////
// Ok button pressed: update Properties buffer
///////////////////////////////////////////////////////////
void EditLayerDialog::OnOk(void)
{
	// Readback Drawing modes
	cLayer.bBoxBars			= checkBoxBars->isChecked();					// Bars check-box
	cLayer.bBox3DBars		= checkBox3DBars->isChecked();					// 3D-Bars check-box
	cLayer.bVisible			= checkBoxHide->isChecked() ? false: true;		// Layer Visible?
	cLayer.bFittingCurve	= checkBoxfittingCurve->isChecked();			// Fitting curve check-box
	cLayer.bBellCurve		= checkBoxBellCurve->isChecked();				// Bell curve check-box
	cLayer.bLines			= checkBoxLines->isChecked();					// Lines check-box
	cLayer.bSpots			= checkBoxspots->isChecked();					// Spots check-box
	cLayer.iLineWidth		= spinBoxLineWidth->value();					// Get line width
	cLayer.cColor			= pushButtonLineColor->activeColor();			// Line color
	cLayer.iLineStyle		= comboBoxLineStyle->currentIndex();			// Get line style: slid, dashed, etc..
	cLayer.iSpotStyle		= comboBoxLineSpotStyle->currentIndex();		// Get spot style: circle, rectangle, diamond, etc..

	// Offset to apply to data in layer (trend chart only)
	cLayer.lfDataOffset		= lineEditParamDataOffset->text().toDouble();
	// Scale factor to apply to data in layer (trend chart only)
	cLayer.lfDataScale		= lineEditParamDataScale->text().toDouble();

	cLayer.iWhiskerMode		= comboBoxWhiskerType->currentIndex();

	// Flags that major changes done (scale/offset), need to fully repaint and zoom-out on chart
	if(cLayer.lfDataOffset != 0 || cLayer.lfDataScale != 1)
		m_bNewParameterSelected = true;

	// Markers: Already up-to-date as updated live on GUI signals.

	// Ok status
	done(1);
}

