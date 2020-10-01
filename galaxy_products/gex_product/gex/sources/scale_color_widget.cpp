///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "scale_color_widget.h"
#include "report_build.h"
#include "gex_file_in_group.h"

///////////////////////////////////////////////////////////////////////////////////
// Class GexScaleColorWidget - class which 
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexScaleColorWidget::GexScaleColorWidget(QWidget * pParent /* = NULL */) : QWidget(pParent)
{
	setupUi(this);

	QObject::connect(horizontalSliderLowValue,	SIGNAL(sliderMoved(int)),		this,		SLOT(onLowValueChanged(int)));
	QObject::connect(horizontalSliderHighValue,	SIGNAL(sliderMoved(int)),		this,		SLOT(onHighValueChanged(int)));
	QObject::connect(horizontalSliderLowValue,	SIGNAL(valueChanged(int)),		this,		SLOT(onLowValueChanged(int)));
	QObject::connect(horizontalSliderHighValue,	SIGNAL(valueChanged(int)),		this,		SLOT(onHighValueChanged(int)));
	QObject::connect(checkBoxLockValue,			SIGNAL(stateChanged(int)),		this,		SLOT(onLockChanged()));

	m_bLockSpectrum		= false;

	checkBoxLockValue->setChecked(m_bLockSpectrum);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexScaleColorWidget::~GexScaleColorWidget()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void enableSpectrumColor(bool bEnable)
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
void GexScaleColorWidget::enableSpectrumColor(bool bEnable)
{
	CGexFileInGroup cFile(NULL,0,"",0,0,"","","");

	if (bEnable)
	{
		if (parametricScaleColor->testCell())
		{
			horizontalSliderLowValue->show();
			horizontalSliderHighValue->show();
			labelLowValue->show();
			labelHighValue->show();
			checkBoxLockValue->show();
				
			// Build string for low value
			QString strString = cFile.FormatTestResult(parametricScaleColor->testCell(), m_lfSpectrumLowValue, parametricScaleColor->testCell()->res_scal);
			labelLowValue->setText(strString);

			// Build string for high value
			strString = cFile.FormatTestResult(parametricScaleColor->testCell(), m_lfSpectrumHighValue, parametricScaleColor->testCell()->res_scal);
			labelHighValue->setText(strString);

			// Disable slider when range is null
			if (m_lfSpectrumMaxValue - m_lfSpectrumMinValue == 0.0)
			{	
				horizontalSliderLowValue->setEnabled(false);
				horizontalSliderHighValue->setEnabled(false);
			}
			else
			{	
				horizontalSliderLowValue->setEnabled(true);
				horizontalSliderHighValue->setEnabled(true);
			}	
		}
		else
		{
			horizontalSliderLowValue->hide();
			horizontalSliderHighValue->hide();
			labelLowValue->hide();
			labelHighValue->hide();
			checkBoxLockValue->hide();
		}
	}
	else
	{
		horizontalSliderLowValue->hide();
		horizontalSliderHighValue->hide();
		labelLowValue->hide();
		labelHighValue->hide();
		checkBoxLockValue->hide();
	}
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void setCurrentPage(scalePage ePage)
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
void GexScaleColorWidget::setCurrentPage(scalePage ePage)
{
	stackedWidget->setCurrentIndex(ePage);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void setColorSpectrumBounds(double dLowerBound, double dUpperBound)
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
void GexScaleColorWidget::setColorSpectrumBounds(double dLowerBound, double dUpperBound)
{
	if (parametricScaleColor->testCell()->lTestNumber < GEX_TESTNBR_OFFSET_EXT_SBIN || parametricScaleColor->testCell()->lTestNumber > GEX_TESTNBR_OFFSET_EXT_HBIN)
	{
		if (!m_bLockSpectrum)
		{
			m_lfSpectrumHighValue	= dUpperBound;
			m_lfSpectrumMaxValue	= dUpperBound;
			m_lfSpectrumLowValue	= dLowerBound;
			m_lfSpectrumMinValue	= dLowerBound;
			
			horizontalSliderLowValue->setValue(0);
			horizontalSliderHighValue->setValue(100);
		}

		parametricScaleColor->setRange(m_lfSpectrumLowValue, m_lfSpectrumHighValue);
	}
	else
		parametricScaleColor->setRange(dLowerBound, dUpperBound);

    mHighValueChanged = mLowValueChanged = false;
}
bool GexScaleColorWidget::lowValueChanged() const
{
    return mLowValueChanged;
}
bool GexScaleColorWidget::highValueChanged() const
{
    return mHighValueChanged;
}



///////////////////////////////////////////////////////////
// Low value has changed
///////////////////////////////////////////////////////////
void GexScaleColorWidget::onLowValueChanged(int nValue)
{
	m_lfSpectrumLowValue = m_lfSpectrumMinValue + (m_lfSpectrumMaxValue - m_lfSpectrumMinValue) * ((double) nValue / 100.0);

	if (nValue >= horizontalSliderHighValue->value())
		horizontalSliderHighValue->setValue(nValue+1);

	parametricScaleColor->setRange(m_lfSpectrumLowValue, m_lfSpectrumHighValue);
    mLowValueChanged = true;
	
	emit spectrumValueChanged();
}

///////////////////////////////////////////////////////////
// High value has changed
///////////////////////////////////////////////////////////
void GexScaleColorWidget::onHighValueChanged(int nValue)
{
	m_lfSpectrumHighValue = m_lfSpectrumMinValue + (m_lfSpectrumMaxValue - m_lfSpectrumMinValue) * ((double) nValue / 100.0);

	if (nValue <= horizontalSliderLowValue->value())
		horizontalSliderLowValue->setValue(nValue-1);

	parametricScaleColor->setRange(m_lfSpectrumLowValue, m_lfSpectrumHighValue);

    mHighValueChanged = true;
	
	emit spectrumValueChanged();
}

void GexScaleColorWidget::onLockChanged()
{
	m_bLockSpectrum = checkBoxLockValue->isChecked();
}
