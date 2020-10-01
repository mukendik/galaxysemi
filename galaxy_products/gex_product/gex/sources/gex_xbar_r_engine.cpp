///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_xbar_r_engine.h"
#include "report_options.h"
#include <gqtl_log.h>
#include <gqtl_global.h>

///////////////////////////////////////////////////////////////////////////////////
// STD Includes
///////////////////////////////////////////////////////////////////////////////////
#include <limits>
#include "math.h"

///////////////////////////////////////////////////////////////////////////////////
// Extern Objects
///////////////////////////////////////////////////////////////////////////////////
extern CReportOptions ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
// Extern C Function
///////////////////////////////////////////////////////////////////////////////////
extern double	lfD2_Value(int iZ,int iW);

///////////////////////////////////////////////////////////////////////////////////
// Static Class members
///////////////////////////////////////////////////////////////////////////////////
QMap<int, double> GexXBarREngine::m_mapA2Constants;
QMap<int, double> GexXBarREngine::m_mapD3Constants;
QMap<int, double> GexXBarREngine::m_mapD4Constants;

///////////////////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////////////////

#define		A2_2TRIALS			double(1.880)
#define		A2_3TRIALS			double(1.023)
#define		A2_4TRIALS			double(0.729)
#define		A2_5TRIALS			double(0.577)
#define		A2_6TRIALS			double(0.483)
#define		A2_7TRIALS			double(0.419)
#define		A2_8TRIALS			double(0.373)
#define		A2_9TRIALS			double(0.337)
#define		A2_10TRIALS			double(0.308)
#define		A2_11TRIALS			double(0.285)
#define		A2_12TRIALS			double(0.266)
#define		A2_13TRIALS			double(0.249)
#define		A2_14TRIALS			double(0.235)
#define		A2_15TRIALS			double(0.223)
#define		A2_16TRIALS			double(0.212)
#define		A2_17TRIALS			double(0.203)
#define		A2_18TRIALS			double(0.194)
#define		A2_19TRIALS			double(0.187)
#define		A2_20TRIALS			double(0.180)
#define		A2_21TRIALS			double(0.173)
#define		A2_22TRIALS			double(0.167)
#define		A2_23TRIALS			double(0.162)
#define		A2_24TRIALS			double(0.157)
#define		A2_25TRIALS			double(0.153)

#define		D3_2TRIALS			double(0.0)
#define		D3_3TRIALS			double(0.0)
#define		D3_4TRIALS			double(0.0)
#define		D3_5TRIALS			double(0.0)
#define		D3_6TRIALS			double(0.0)
#define		D3_7TRIALS			double(0.076)
#define		D3_8TRIALS			double(0.136)
#define		D3_9TRIALS			double(0.184)
#define		D3_10TRIALS			double(0.223)
#define		D3_11TRIALS			double(0.256)
#define		D3_12TRIALS			double(0.284)
#define		D3_13TRIALS			double(0.308)
#define		D3_14TRIALS			double(0.329)
#define		D3_15TRIALS			double(0.348)
#define		D3_16TRIALS			double(0.364)
#define		D3_17TRIALS			double(0.379)
#define		D3_18TRIALS			double(0.392)
#define		D3_19TRIALS			double(0.404)
#define		D3_20TRIALS			double(0.414)
#define		D3_21TRIALS			double(0.425)
#define		D3_22TRIALS			double(0.434)
#define		D3_23TRIALS			double(0.443)
#define		D3_24TRIALS			double(0.452)
#define		D3_25TRIALS			double(0.459)

#define		D4_2TRIALS			double(3.267)
#define		D4_3TRIALS			double(2.574)
#define		D4_4TRIALS			double(2.282)
#define		D4_5TRIALS			double(2.114)
#define		D4_6TRIALS			double(2.004)
#define		D4_7TRIALS			double(1.924)
#define		D4_8TRIALS			double(1.864)
#define		D4_9TRIALS			double(1.816)
#define		D4_10TRIALS			double(1.777)
#define		D4_11TRIALS			double(1.774)
#define		D4_12TRIALS			double(1.716)
#define		D4_13TRIALS			double(1.692)
#define		D4_14TRIALS			double(1.671)
#define		D4_15TRIALS			double(1.652)
#define		D4_16TRIALS			double(1.636)
#define		D4_17TRIALS			double(1.621)
#define		D4_18TRIALS			double(1.608)
#define		D4_19TRIALS			double(1.596)
#define		D4_20TRIALS			double(1.586)
#define		D4_21TRIALS			double(1.575)
#define		D4_22TRIALS			double(1.566)
#define		D4_23TRIALS			double(1.557)
#define		D4_24TRIALS			double(1.548)
#define		D4_25TRIALS			double(1.541)

#define		FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, trial) \
					map.insert(trial, nameConstant##_##trial##TRIALS);

#define		FILL_CONSTANTS_MAP(map, nameConstant)					\
				if (map.isEmpty())									\
				{													\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 2)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 3)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 4)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 5)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 6)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 7)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 8)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 9)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 10) \
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 11)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 12)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 13)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 14)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 15)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 16)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 17)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 18)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 19)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 20) \
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 21)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 22)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 23)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 24)	\
					FILL_CONSTANTS_MAP_TRIAL(map, nameConstant, 25)	\
				}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarREngineDataPoint
//
// Description	:	Represent a XBarR datapoint computed by the XBarREngine
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GexXBarREngineDataPoint::GexXBarREngineDataPoint() :
		m_dMinValue(std::numeric_limits<double>::max()),
		m_dMaxValue(-std::numeric_limits<double>::max()),
		m_dSumValues(0),
		m_uiValueCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////////
// Copy constructor
///////////////////////////////////////////////////////////////////////////////////
GexXBarREngineDataPoint::GexXBarREngineDataPoint(const GexXBarREngineDataPoint &other)
{
	*this = other;
}

///////////////////////////////////////////////////////////////////////////////////
// Assignement operator
///////////////////////////////////////////////////////////////////////////////////
GexXBarREngineDataPoint& GexXBarREngineDataPoint::operator=(const GexXBarREngineDataPoint& other)
{
	if (this != &other)
	{
		m_dMinValue		= other.m_dMinValue;
		m_dMaxValue		= other.m_dMaxValue;
		m_dSumValues	= other.m_dSumValues;
		m_uiValueCount	= other.m_uiValueCount;
	}

	return *this;
}

///////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////

void GexXBarREngineDataPoint::addValue(double dValue)
{
	if (!(isnan(dValue)))
	{
		m_dMinValue		= qMin(m_dMinValue, dValue);
		m_dMaxValue		= qMax(m_dMaxValue, dValue);
		m_dSumValues	+= dValue;
		++m_uiValueCount;
	}
	else
		GSLOG(SYSLOG_SEV_WARNING, "Added value is NAN");
}

void GexXBarREngineDataPoint::clear()
{
	m_dMinValue		= std::numeric_limits<double>::max();
	m_dMaxValue		= -std::numeric_limits<double>::max(),
	m_dSumValues		= 0;
	m_uiValueCount	= 0;
}

double GexXBarREngineDataPoint::mean() const
{
	GEX_ASSERT(m_uiValueCount > 0);

	if (m_uiValueCount > 0)
		return (m_dSumValues / ((double) m_uiValueCount));
	else
		GSLOG(SYSLOG_SEV_WARNING, "NB samples is 0");

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngineDataPoint::range() const
{
	if (m_dMinValue != std::numeric_limits<double>::max() && m_dMaxValue != (-std::numeric_limits<double>::max()))
		return qAbs(m_dMaxValue - m_dMinValue);
	else
	{
		if (m_dMinValue == std::numeric_limits<double>::max())
			GSLOG(SYSLOG_SEV_WARNING, "Min value is not defined");

		if (m_dMaxValue == -std::numeric_limits<double>::max())
			GSLOG(SYSLOG_SEV_WARNING, "Max value is not defined");
	}

	return std::numeric_limits<double>::quiet_NaN();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarREngine
//
// Description	:	Class used to compute data for XBar R charts
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GexXBarREngine::GexXBarREngine(uint uiTrialsCount) : m_uiTrialsCount(uiTrialsCount), m_dRAndRSigma(std::numeric_limits<double>::quiet_NaN())
{
	FILL_CONSTANTS_MAP(m_mapA2Constants, A2);
	FILL_CONSTANTS_MAP(m_mapD3Constants, D3);
	FILL_CONSTANTS_MAP(m_mapD4Constants, D4);

	bool bRRSigma = false;

	m_dRAndRSigma = ReportOptions.GetOption("adv_boxplot","r&r_sigma").toDouble(&bRRSigma);

	if (bRRSigma == false)
		GSLOG(SYSLOG_SEV_WARNING, "Can't retrieve R&R sigma value from options");

}

///////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////

GexXBarREngineDataPoint GexXBarREngine::computeDataPoint(const QVector<double>& vecDataPoint)
{
	GexXBarREngineDataPoint dpXBarR;

	for (int nTrialIndex = 0; nTrialIndex < vecDataPoint.size(); ++nTrialIndex)
		dpXBarR.addValue(vecDataPoint.at(nTrialIndex));

	return dpXBarR;
}

double GexXBarREngine::computeXBarLowerControlLimit(double dXBarMean, double dRMean)
{
	if (m_mapA2Constants.contains(m_uiTrialsCount) && (isnan(dXBarMean) == 0) && (isnan(dRMean) == 0))
		return (double) (dXBarMean - (m_mapA2Constants.value(m_uiTrialsCount) * dRMean));
	else
	{
		if (isnan(dXBarMean))
			GSLOG(SYSLOG_SEV_WARNING, "XBar mean value is NAN");

		if (isnan(dRMean))
			GSLOG(SYSLOG_SEV_WARNING, "R mean value is NAN");

		if (m_mapA2Constants.contains(m_uiTrialsCount) == false)
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find A2 constants for %1 trials").arg( m_uiTrialsCount).toLatin1().constData());
	}

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngine::computeXBarUpperControlLimit(double dXBarMean, double dRMean)
{
	if (m_mapA2Constants.contains(m_uiTrialsCount) && (isnan(dXBarMean) == 0) && (isnan(dRMean) == 0))
		return (double) (dXBarMean + (m_mapA2Constants.value(m_uiTrialsCount) * dRMean));
	else
	{
		if (isnan(dXBarMean))
			GSLOG(SYSLOG_SEV_WARNING, "XBar mean value is NAN");

		if (isnan(dRMean))
			GSLOG(SYSLOG_SEV_WARNING, "R mean value is NAN");

		if (m_mapA2Constants.contains(m_uiTrialsCount) == false)
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find A2 constants for %1 trials").arg( m_uiTrialsCount).toLatin1().constData());
	}

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngine::computeRLowerControlLimit(double dRMean)
{
	if (m_mapD3Constants.contains(m_uiTrialsCount) && (isnan(dRMean) == 0))
		return (double) (m_mapD3Constants.value(m_uiTrialsCount) * dRMean);
	else
	{
		if (isnan(dRMean))
			GSLOG(SYSLOG_SEV_WARNING, "R mean value is NAN");

		if (m_mapD3Constants.contains(m_uiTrialsCount) == false)
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find D3 constants for %1 trials").arg( m_uiTrialsCount).toLatin1().constData());
	}

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngine::computeRUpperControlLimit(double dRMean)
{
	if (m_mapD4Constants.contains(m_uiTrialsCount) && (isnan(dRMean) == 0))
		return (double) (m_mapD4Constants.value(m_uiTrialsCount) * dRMean);
	else
	{
		if (isnan(dRMean))
			GSLOG(SYSLOG_SEV_WARNING, "R mean value is NAN");

		if (m_mapD3Constants.contains(m_uiTrialsCount) == false)
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find DSYSLOG_SEV_WARNING constants for %1 trials").arg( m_uiTrialsCount).toLatin1().constData());
	}

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngine::computeEquipmentVariation(double dRSumValues, uint uiValueCount)
{
	double dD2Constants = lfD2_Value(uiValueCount, m_uiTrialsCount);

	if ((isnan(dRSumValues) == 0) && dD2Constants != 0.0 && !isnan(m_dRAndRSigma))
		return (((m_dRAndRSigma * dRSumValues) / ((double)uiValueCount)) / dD2Constants);
//		return (((3.05 * dRSumValues) / ((double)uiValueCount)));
	else
	{
		if (isnan(dRSumValues))
			GSLOG(SYSLOG_SEV_WARNING, "R sum values is NAN");

		if (dD2Constants == 0.0)
			GSLOG(SYSLOG_SEV_WARNING, "D2 constants is equal to 0");

		if (isnan(m_dRAndRSigma))
			GSLOG(SYSLOG_SEV_WARNING, "R&R sigma is NAN");
	}

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngine::computeAppraiserVariation(double dXBarDiff, double dEV, uint uiValueCount)
{
	if (!isnan(dXBarDiff)  && !isnan(m_dRAndRSigma) && uiValueCount != 0 && m_uiTrialsCount != 0)
	{
		double dD2Constants = lfD2_Value(1, m_uiTrialsCount);

		if (dD2Constants != 0.0 )
		{
			double dValue	= (m_dRAndRSigma * dXBarDiff) / dD2Constants;
//			double dValue	= (3.05 * dXBarDiff);

			return sqrt(fabs((dValue * dValue) - (dEV * dEV)/(m_uiTrialsCount * uiValueCount)));
		}
	}
	else
	{
		if (isnan(dXBarDiff))
			GSLOG(SYSLOG_SEV_WARNING, "XBar diff is NAN");

		if (uiValueCount == 0)
			GSLOG(SYSLOG_SEV_WARNING, "Samples count is equal to 0");

		if (isnan(m_dRAndRSigma))
			GSLOG(SYSLOG_SEV_WARNING, "R&R sigma is NAN");
	}

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXBarREngine::computeRAndR(double dEquipmentVariation, double dAppraiserVariation)
{
	double dRandR = std::numeric_limits<double>::quiet_NaN();

	if (!isnan(dEquipmentVariation) && !isnan(dAppraiserVariation))
		dRandR = sqrt(GS_POW(dEquipmentVariation, 2.0) + GS_POW(dAppraiserVariation, 2.0));
	else
	{
		if (isnan(dEquipmentVariation))
			GSLOG(SYSLOG_SEV_WARNING, "Equipment variation is NAN");

		if (isnan(dAppraiserVariation))
			GSLOG(SYSLOG_SEV_WARNING, "Appraiser variation is NAN");
	}

	return dRandR;
}
