#ifndef GEX_XBAR_R_ENGINE_H
#define GEX_XBAR_R_ENGINE_H

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QVector>
#include <QMap>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarREngineDataPoint
//
// Description	:	Represent a XBarR datapoint computed by the XBarREngine
//
///////////////////////////////////////////////////////////////////////////////////
class GexXBarREngineDataPoint
{
public:

	GexXBarREngineDataPoint();
	GexXBarREngineDataPoint(const GexXBarREngineDataPoint& other);

	GexXBarREngineDataPoint& operator=(const GexXBarREngineDataPoint& other);

	void		addValue(double dValue);
	void		clear();

	double		mean() const;
	double		range() const;
	double		minValue() const			{ return m_dMinValue; }
	double		maxValue() const			{ return m_dMaxValue; }
	double		sumValues() const			{ return m_dSumValues; }
	uint		valueCount() const			{ return m_uiValueCount; }

private:

	double		m_dMinValue;
	double		m_dMaxValue;
	double		m_dSumValues;
	uint		m_uiValueCount;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarREngine
//
// Description	:	Class used to compute data for XBar R charts
//
///////////////////////////////////////////////////////////////////////////////////

class GexXBarREngine
{
public:

	GexXBarREngine(uint uiTrialsCount);

	GexXBarREngineDataPoint		computeDataPoint(const QVector<double>& vecDataPoint);
	double						computeXBarUpperControlLimit(double dXBarMean, double dRMean);
	double						computeXBarLowerControlLimit(double dXBarMean, double dRMean);
	double						computeRUpperControlLimit(double dRMean);
	double						computeRLowerControlLimit(double dRMean);

	double						computeEquipmentVariation(double dRSumValues, uint uiValueCount);
	double						computeAppraiserVariation(double dXBarDiff, double dEV, uint uiValueCount);
	double						computeRAndR(double dEquipmentVariation, double dAppraiserVariation);

private:

	uint						m_uiTrialsCount;
	double						m_dRAndRSigma;

	static QMap<int, double>	m_mapA2Constants;
	static QMap<int, double>	m_mapD3Constants;
	static QMap<int, double>	m_mapD4Constants;
};

#endif // GEX_XBAR_R_ENGINE_H
