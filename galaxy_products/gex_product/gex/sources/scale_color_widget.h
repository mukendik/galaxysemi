#ifndef _SCALE_COLOR_WIDGET_H_
#define _SCALE_COLOR_WIDGET_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "ui_scale_color_widget.h"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	GexScaleColorWidget
//
// Description	:	Class to display scale colors
//
///////////////////////////////////////////////////////////////////////////////////
class GexScaleColorWidget : public QWidget, public Ui::scaleColorWidget
{

	Q_OBJECT

public:

	enum scalePage
	{
		scalePageBinning = 0,
		scalePageParametric,
		scalePageParametricPassFail,
	};

	GexScaleColorWidget(QWidget * pParent = NULL);
	~GexScaleColorWidget();

	double							spectrumColorLowValue() const		{ return m_lfSpectrumLowValue;}
	double							spectrumColorHighValue() const		{ return m_lfSpectrumHighValue;}

	void							enableSpectrumColor(bool bEnable);
	void							setCurrentPage(scalePage ePage);

	void							setColorSpectrumBounds(double dLowerBound, double dUpperBound);

	void							unLockBrowsing()					{ checkBoxLockValue->setChecked(false); }

    bool                            lowValueChanged() const;

    bool                            highValueChanged() const;

private:

    double							m_lfSpectrumLowValue;
	double							m_lfSpectrumHighValue;
	double							m_lfSpectrumMinValue;
	double							m_lfSpectrumMaxValue;
	bool							m_bLockSpectrum;
    bool                            mLowValueChanged;
    bool                            mHighValueChanged;
	
private slots:
	
	void							onLowValueChanged(int nValue);			// Low value has changed
	void							onHighValueChanged(int nValue);			// High value has changed
	void							onLockChanged();

signals:
	
	void							spectrumValueChanged();
};

#endif // _SCALE_COLOR_WIDGET_H_
