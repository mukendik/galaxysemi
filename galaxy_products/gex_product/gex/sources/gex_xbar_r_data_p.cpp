///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_xbar_r_data_p.h"
#include <gqtl_log.h>

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QString>

///////////////////////////////////////////////////////////////////////////////////
// STD Includes
///////////////////////////////////////////////////////////////////////////////////
#include<limits>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXbarRSubDatasetPrivate
//
// Description	:	Private class to store data related to a XBar R dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Construstor
///////////////////////////////////////////////////////////////////////////////////
GexXbarRSubDatasetPrivate::GexXbarRSubDatasetPrivate() :
		m_uiXBarValueCount(0),
		m_uiRValueCount(0),
		m_pOutlierDeviceArray(NULL),
		m_dXBarUCL(std::numeric_limits<double>::quiet_NaN()),
		m_dXBarLCL(std::numeric_limits<double>::quiet_NaN()),
		m_dXBarSumValues(0),
		m_dXBarMin(std::numeric_limits<double>::max()),
		m_dXBarMax(-std::numeric_limits<double>::max()),
		m_dRUCL(std::numeric_limits<double>::quiet_NaN()),
		m_dRLCL(std::numeric_limits<double>::quiet_NaN()),
		m_dRSumValues(0),
		m_dRMin(std::numeric_limits<double>::max()),
		m_dRMax(-std::numeric_limits<double>::max()),
		m_pXBarDataArray(NULL),
		m_pRDataArray(NULL),
		m_dLowTestLimit(std::numeric_limits<double>::quiet_NaN()),
		m_dHighTestLimit(std::numeric_limits<double>::quiet_NaN()),
		m_dEquipmentVariation(std::numeric_limits<double>::quiet_NaN()),
		m_dAppraiserVariation(std::numeric_limits<double>::quiet_NaN()),
		m_dICEquipmentVariation(std::numeric_limits<double>::quiet_NaN()),
		m_dICAppraiserVariation(std::numeric_limits<double>::quiet_NaN())
{
}

///////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////
double GexXbarRSubDatasetPrivate::XBarMean() const
{
	GEX_ASSERT(m_uiXBarValueCount > 0);

	if (m_uiXBarValueCount > 0)
		return (m_dXBarSumValues / (double) m_uiXBarValueCount);

	return std::numeric_limits<double>::quiet_NaN();
}

double GexXbarRSubDatasetPrivate::RMean() const
{
	GEX_ASSERT(m_uiRValueCount > 0);

	if (m_uiRValueCount > 0)
		return (m_dRSumValues / (double) m_uiRValueCount);

	return std::numeric_limits<double>::quiet_NaN();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarRDatasetPrivate
//
// Description	:	Represent a computed dataset for XBar R chart
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Construstor
///////////////////////////////////////////////////////////////////////////////////
GexXBarRDatasetPrivate::GexXBarRDatasetPrivate(uint uiTrialsCount, uint uiDevicesCount, uint uiAppraisersCount, QString strUnits) :
	m_uiDevicesCount(uiDevicesCount),
	m_uiTrialsCount(uiTrialsCount),
	m_uiAppraisersCount(uiAppraisersCount),
	m_uiOutlierCount(0),
	m_strUnits(strUnits),
	m_dWorstCaseAV(std::numeric_limits<double>::quiet_NaN()),
	m_dWorstCaseRAndR(std::numeric_limits<double>::quiet_NaN()),
	m_pXBarRLabelArray(NULL),
	m_pEngine(NULL)
{
	GEX_ASSERT(m_uiTrialsCount > 0);
	GEX_ASSERT(m_uiDevicesCount > 0);
	GEX_ASSERT(m_uiAppraisersCount > 0);

	m_uiTotalSamples	= 0;
	m_dSumSamples		= 0.0;
	m_dMinSamples		= std::numeric_limits<double>::quiet_NaN();
	m_dMaxSamples		= std::numeric_limits<double>::quiet_NaN();

	if (m_uiTrialsCount > 0 && m_uiDevicesCount > 0 && m_uiAppraisersCount > 0)
	{
		// Create XBar and R engine
		m_pEngine = new GexXBarREngine(m_uiTrialsCount);

		// Allocate arrays for labels
		m_pXBarRLabelArray = new double[dataCount()];

		// Allocate arrays for XBar and R dataset
		m_allAppraiserDataset.m_pXBarDataArray	= new double[dataCount()];
		m_allAppraiserDataset.m_pRDataArray		= new double[dataCount()];

		// Allocate arrays for Oultier
		m_allAppraiserDataset.m_pOutlierDeviceArray	= new uchar[(deviceCount()/8) + 1];

		memset(m_allAppraiserDataset.m_pOutlierDeviceArray, 0, sizeof(uchar) * ((deviceCount()/8) + 1));

		// Initialize data arrays
		for (uint uiIndex = 0; uiIndex < dataCount(); ++uiIndex)
		{
			m_allAppraiserDataset.m_pXBarDataArray[uiIndex] = std::numeric_limits<double>::quiet_NaN();
			m_allAppraiserDataset.m_pRDataArray[uiIndex]	= std::numeric_limits<double>::quiet_NaN();
		}

		// Create subdataset classes per trial
		for (uint uiAppraiserIndex = 0; uiAppraiserIndex < m_uiAppraisersCount; ++uiAppraiserIndex)
		{
			GexXbarRSubDatasetPrivate * pSubDataset = new GexXbarRSubDatasetPrivate();

			pSubDataset->m_pXBarDataArray	= (m_allAppraiserDataset.m_pXBarDataArray + (uiAppraiserIndex * m_uiDevicesCount));
			pSubDataset->m_pRDataArray		= (m_allAppraiserDataset.m_pRDataArray + (uiAppraiserIndex * m_uiDevicesCount));

			m_lstAppraiserDataset.append(pSubDataset);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
GexXBarRDatasetPrivate::~GexXBarRDatasetPrivate()
{
	// Remove subdataset
	while (m_lstAppraiserDataset.isEmpty() == false)
		delete m_lstAppraiserDataset.takeFirst();

	// Delere Array labels
	if (m_pXBarRLabelArray)
	{
		delete m_pXBarRLabelArray;
		m_pXBarRLabelArray = NULL;
	}

	// Delete XBar array
	if (m_allAppraiserDataset.m_pXBarDataArray != NULL)
	{
		delete [] m_allAppraiserDataset.m_pXBarDataArray;
		m_allAppraiserDataset.m_pXBarDataArray = NULL;
	}

	// Delete R array
	if (m_allAppraiserDataset.m_pRDataArray != NULL)
	{
		delete [] m_allAppraiserDataset.m_pRDataArray;
		m_allAppraiserDataset.m_pRDataArray = NULL;
	}

	// Delete Outlier Array
	if (m_allAppraiserDataset.m_pOutlierDeviceArray)
	{
		delete [] m_allAppraiserDataset.m_pOutlierDeviceArray;
		m_allAppraiserDataset.m_pOutlierDeviceArray = NULL;
	}

	// Delete engine
	if (m_pEngine != NULL)
	{
		delete m_pEngine;
		m_pEngine = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////
uint GexXBarRDatasetPrivate::dataCount() const
{
	return (m_uiDevicesCount * m_uiAppraisersCount);
}

uint GexXBarRDatasetPrivate::trialCount() const
{
	return m_uiTrialsCount;
}

uint GexXBarRDatasetPrivate::appraiserCount() const
{
	return m_uiAppraisersCount;
}

uint GexXBarRDatasetPrivate::goodDeviceCount() const
{
	return (m_uiDevicesCount - m_uiOutlierCount);
}

QString GexXBarRDatasetPrivate::units() const
{
	return m_strUnits;
}

uint GexXBarRDatasetPrivate::deviceCount() const
{
	return m_uiDevicesCount;
}

double GexXBarRDatasetPrivate::XBarUCL() const
{
	return m_allAppraiserDataset.m_dXBarUCL;
}

double GexXBarRDatasetPrivate::XBarLCL() const
{
	return m_allAppraiserDataset.m_dXBarLCL;
}

double GexXBarRDatasetPrivate::XBarMean() const
{
	return m_allAppraiserDataset.XBarMean();
}

double GexXBarRDatasetPrivate::XBarMin() const
{
	return m_allAppraiserDataset.m_dXBarMin;
}

double GexXBarRDatasetPrivate::XBarMax() const
{
	return m_allAppraiserDataset.m_dXBarMax;
}

double * GexXBarRDatasetPrivate::XBarDataArray() const
{
	return m_allAppraiserDataset.m_pXBarDataArray;
}

double GexXBarRDatasetPrivate::RUCL() const
{
	return m_allAppraiserDataset.m_dRUCL;
}

double GexXBarRDatasetPrivate::RLCL() const
{
	return m_allAppraiserDataset.m_dRLCL;
}

double GexXBarRDatasetPrivate::RMean() const
{
	return m_allAppraiserDataset.RMean();
}

double GexXBarRDatasetPrivate::RMin() const
{
	return m_allAppraiserDataset.m_dRMin;
}

double GexXBarRDatasetPrivate::RMax() const
{
	return m_allAppraiserDataset.m_dRMax;
}

double * GexXBarRDatasetPrivate::RDataArray() const
{
	return m_allAppraiserDataset.m_pRDataArray;
}

double * GexXBarRDatasetPrivate::XBarRLabelArray() const
{
	return m_pXBarRLabelArray;
}

double GexXBarRDatasetPrivate::lowTestLimit() const
{
	return m_allAppraiserDataset.m_dLowTestLimit;
}

double GexXBarRDatasetPrivate::highTestLimit() const
{
	return m_allAppraiserDataset.m_dHighTestLimit;
}

double GexXBarRDatasetPrivate::lowTestLimitAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dLowTestLimit;
}

double GexXBarRDatasetPrivate::highTestLimitAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dHighTestLimit;
}

double GexXBarRDatasetPrivate::XBarUCLAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dXBarUCL;
}

double GexXBarRDatasetPrivate::XBarLCLAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dXBarLCL;
}

double GexXBarRDatasetPrivate::XBarMeanAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->XBarMean();
}

double GexXBarRDatasetPrivate::RUCLAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dRUCL;
}

double GexXBarRDatasetPrivate::RLCLAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dRLCL;
}

double GexXBarRDatasetPrivate::RMeanAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->RMean();
}

double GexXBarRDatasetPrivate::EVAppraiser(uint uiAppraiserIndex) const
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	return m_lstAppraiserDataset.at(uiAppraiserIndex)->m_dEquipmentVariation;
}

double GexXBarRDatasetPrivate::appraiserVariation() const
{
	return m_allAppraiserDataset.m_dAppraiserVariation;
}

double GexXBarRDatasetPrivate::equipmentVariation() const
{
	return m_allAppraiserDataset.m_dEquipmentVariation;
}

double GexXBarRDatasetPrivate::ICAppraiserVariation() const
{
	return m_allAppraiserDataset.m_dICAppraiserVariation;
}

double GexXBarRDatasetPrivate::ICEquipmentVariation() const
{
	return m_allAppraiserDataset.m_dICEquipmentVariation;
}

double GexXBarRDatasetPrivate::WCAppraiserVariation() const
{
	return m_dWorstCaseAV;
}

double GexXBarRDatasetPrivate::WCRAndR() const
{
	return m_dWorstCaseRAndR;
}

QString GexXBarRDatasetPrivate::WCAppraisers() const
{
	return m_strMaxDiffRAndR;
}

double GexXBarRDatasetPrivate::ICRandR() const
{
	return m_pEngine->computeRAndR(m_allAppraiserDataset.m_dICEquipmentVariation, m_allAppraiserDataset.m_dICAppraiserVariation);
}

double GexXBarRDatasetPrivate::RandR() const
{
	return m_pEngine->computeRAndR(m_allAppraiserDataset.m_dEquipmentVariation, m_allAppraiserDataset.m_dAppraiserVariation);
}

void GexXBarRDatasetPrivate::addDataPoint(uint uiAppraiserIndex, uint uiDeviceIndex, const QVector<double>& vecPoint)
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);
	GEX_ASSERT(uiDeviceIndex < m_uiDevicesCount);

	// Compute XBar/R values for this point
	GexXBarREngineDataPoint dpEngineDataPoint = m_pEngine->computeDataPoint(vecPoint);

	// Assign result for each subdataset and main dataset
	if (dpEngineDataPoint.valueCount() > 0)
	{
		uint uiArrayIndex = (uiAppraiserIndex * m_uiDevicesCount) + uiDeviceIndex;

		m_allAppraiserDataset.m_pXBarDataArray[uiArrayIndex]	= dpEngineDataPoint.mean();
		m_allAppraiserDataset.m_pRDataArray[uiArrayIndex]		= dpEngineDataPoint.range();

		// Set computed value to the main dataset
		// XBar computation results
		m_allAppraiserDataset.m_dXBarSumValues		+= dpEngineDataPoint.sumValues();
		m_allAppraiserDataset.m_uiXBarValueCount	+= dpEngineDataPoint.valueCount();

		// XBar min/max values
		m_allAppraiserDataset.m_dXBarMin			= qMin(m_allAppraiserDataset.m_dXBarMin, dpEngineDataPoint.mean());
		m_allAppraiserDataset.m_dXBarMax			= qMax(m_allAppraiserDataset.m_dXBarMax, dpEngineDataPoint.mean());

		// R computation results
		m_allAppraiserDataset.m_dRSumValues			+= dpEngineDataPoint.range();
		m_allAppraiserDataset.m_uiRValueCount++;

		// R min/max values
		m_allAppraiserDataset.m_dRMin				= qMin(m_allAppraiserDataset.m_dRMin, dpEngineDataPoint.range());
		m_allAppraiserDataset.m_dRMax				= qMax(m_allAppraiserDataset.m_dRMax, dpEngineDataPoint.range());

		// Set computed value to the trial subdataset
		// Retrieve subdataset corresponding to the trial
		GexXbarRSubDatasetPrivate * pTrialDataset = m_lstAppraiserDataset[uiAppraiserIndex];

		// XBar computation results
		pTrialDataset->m_dXBarSumValues		+= dpEngineDataPoint.sumValues();
		pTrialDataset->m_uiXBarValueCount	+= dpEngineDataPoint.valueCount();

		// XBar min/max values
		pTrialDataset->m_dXBarMin			= qMin(pTrialDataset->m_dXBarMin, dpEngineDataPoint.mean());
		pTrialDataset->m_dXBarMax			= qMax(pTrialDataset->m_dXBarMax, dpEngineDataPoint.mean());

		// R computation results
		pTrialDataset->m_dRSumValues		+= dpEngineDataPoint.range();
		pTrialDataset->m_uiRValueCount++;

		// R min/max values
		pTrialDataset->m_dRMin				= qMin(pTrialDataset->m_dRMin, dpEngineDataPoint.range());
		pTrialDataset->m_dRMax				= qMax(pTrialDataset->m_dRMax, dpEngineDataPoint.range());

		// Statistic information
		m_dSumSamples						+= dpEngineDataPoint.sumValues();
		m_uiTotalSamples					+= dpEngineDataPoint.valueCount();
		m_dMinSamples						= (isnan(m_dMinSamples)) ? dpEngineDataPoint.minValue() : qMin(m_dMinSamples, dpEngineDataPoint.minValue());
		m_dMaxSamples						= (isnan(m_dMaxSamples)) ? dpEngineDataPoint.maxValue() : qMax(m_dMaxSamples, dpEngineDataPoint.maxValue());

		// Add label for this device in this apparaiser
		m_pXBarRLabelArray[uiArrayIndex]	= uiDeviceIndex+1;
	}
}

void GexXBarRDatasetPrivate::computeControlLimits()
{
	// Compute XBar Lower Control Limit for the whole dataset
	m_allAppraiserDataset.m_dXBarLCL = m_pEngine->computeXBarLowerControlLimit(m_allAppraiserDataset.XBarMean(), m_allAppraiserDataset.RMean());

	// Compute XBar Upper Control Limit for the whole dataset
	m_allAppraiserDataset.m_dXBarUCL = m_pEngine->computeXBarUpperControlLimit(m_allAppraiserDataset.XBarMean(), m_allAppraiserDataset.RMean());

	// Compute R Lower Control Limit for the whole dataset
	m_allAppraiserDataset.m_dRLCL = m_pEngine->computeRLowerControlLimit(m_allAppraiserDataset.RMean());

	// Compute R Upper Control Limit for the whole dataset
	m_allAppraiserDataset.m_dRUCL = m_pEngine->computeRUpperControlLimit(m_allAppraiserDataset.RMean());

	// Compute EV
	m_allAppraiserDataset.m_dEquipmentVariation = m_pEngine->computeEquipmentVariation(m_allAppraiserDataset.m_dRSumValues, m_allAppraiserDataset.m_uiRValueCount);

	// Compute Control Limits per Appraiser
	GexXbarRSubDatasetPrivate * pAppraiserDataset		= NULL;
	GexXbarRSubDatasetPrivate * pAppraiserDatasetMin	= NULL;
	GexXbarRSubDatasetPrivate * pAppraiserDatasetMax	= NULL;

	for (uint uiAppraiser = 0; uiAppraiser < m_uiAppraisersCount; ++uiAppraiser)
	{
		pAppraiserDataset = m_lstAppraiserDataset[uiAppraiser];

		// Compute XBar Lower Control Limit for the appraiser
		pAppraiserDataset->m_dXBarLCL = m_pEngine->computeXBarLowerControlLimit(pAppraiserDataset->XBarMean(), pAppraiserDataset->RMean());

		// Compute XBar Upper Control Limit for the appraiser
		pAppraiserDataset->m_dXBarUCL = m_pEngine->computeXBarUpperControlLimit(pAppraiserDataset->XBarMean(), pAppraiserDataset->RMean());

		// Compute R Lower Control Limit for the appraiser
		pAppraiserDataset->m_dRLCL = m_pEngine->computeRLowerControlLimit(pAppraiserDataset->RMean());

		// Compute R Upper Control Limit for the appraiser
		pAppraiserDataset->m_dRUCL = m_pEngine->computeRUpperControlLimit(pAppraiserDataset->RMean());

		// Compute Appraiser EV
		pAppraiserDataset->m_dEquipmentVariation = m_pEngine->computeEquipmentVariation(pAppraiserDataset->m_dRSumValues, pAppraiserDataset->m_uiRValueCount);

		if (pAppraiserDatasetMin == NULL || (pAppraiserDataset->XBarMean() < pAppraiserDatasetMin->XBarMean()))
			pAppraiserDatasetMin = pAppraiserDataset;

		if (pAppraiserDatasetMax == NULL || (pAppraiserDataset->XBarMean() > pAppraiserDatasetMax->XBarMean()))
			pAppraiserDatasetMax = pAppraiserDataset;
	}

	char	cMask;
	uint	uiDeviceBit	= 0;

	for (uint uiDevice = 0; uiDevice < m_uiDevicesCount; ++uiDevice)
	{
		uiDeviceBit = uiDevice / 8;
		cMask = 0x01 << (uiDevice % 8);

		for (uint uiAppraiser = 0; uiAppraiser < m_uiAppraisersCount; ++uiAppraiser)
		{
			pAppraiserDataset = m_lstAppraiserDataset[uiAppraiser];

			if (isnan(pAppraiserDataset->m_pRDataArray[uiDevice]) == false)
			{
				if ((isnan(pAppraiserDataset->m_dRUCL) == false && pAppraiserDataset->m_pRDataArray[uiDevice] > pAppraiserDataset->m_dRUCL) ||
						(isnan(pAppraiserDataset->m_dRLCL) == false && pAppraiserDataset->m_pRDataArray[uiDevice] < pAppraiserDataset->m_dRLCL))
				{
					if ((m_allAppraiserDataset.m_pOutlierDeviceArray[uiDeviceBit] & cMask) == 0)
					{
						m_uiOutlierCount++;
						m_allAppraiserDataset.m_pOutlierDeviceArray[uiDeviceBit] |= cMask;
					}
				}
			}
		}
	}

	double dXBarDiff = pAppraiserDatasetMax->XBarMean() - pAppraiserDatasetMin->XBarMean();

	// Compute AV
	m_allAppraiserDataset.m_dAppraiserVariation = m_pEngine->computeAppraiserVariation(dXBarDiff, m_allAppraiserDataset.m_dEquipmentVariation, m_allAppraiserDataset.m_uiRValueCount);

	// Compute IC variables
	double	dICXBarLowerMean	= std::numeric_limits<double>::quiet_NaN();
	double	dICXBarHigherMean	= std::numeric_limits<double>::quiet_NaN();
	double	dICRSumValues		= 0.0;
	double	dICXBarDiff			= std::numeric_limits<double>::quiet_NaN();
	uint	uiICValueCount		= 0;

	for (uint uiAppraiser = 0; uiAppraiser < m_uiAppraisersCount; ++uiAppraiser)
	{
		double	dICAppSumValues		= 0.0;
		double	dICAppXBarSumValues	= 0.0;
		double	dICAppXBarMean		= std::numeric_limits<double>::quiet_NaN();
		uint	uiICAppValueCount	= 0;

		for (uint uiDevice = 0; uiDevice < m_uiDevicesCount; ++uiDevice)
		{
			pAppraiserDataset = m_lstAppraiserDataset[uiAppraiser];

			if (isnan(pAppraiserDataset->m_pRDataArray[uiDevice]) == false)
			{
				uiDeviceBit = uiDevice / 8;
				cMask = 0x01 << (uiDevice % 8);

				if ((m_allAppraiserDataset.m_pOutlierDeviceArray[uiDeviceBit] & cMask) == 0)
				{
					dICAppXBarSumValues	+= pAppraiserDataset->m_pXBarDataArray[uiDevice];
					dICAppSumValues		+= pAppraiserDataset->m_pRDataArray[uiDevice];
					uiICAppValueCount++;
				}
			}
		}

		// Compute local IC EV
		m_allAppraiserDataset.m_dICEquipmentVariation = m_pEngine->computeEquipmentVariation(dICAppSumValues, uiICAppValueCount);

		// Compute IC XBar mean
		if (uiICAppValueCount > 0)
			dICAppXBarMean = dICAppXBarSumValues / uiICAppValueCount;

		if (isnan(dICXBarLowerMean) || (!isnan(dICAppXBarMean) && dICAppXBarMean < dICXBarLowerMean))
			dICXBarLowerMean = dICAppXBarMean;

		if (isnan(dICXBarHigherMean) || (!isnan(dICAppXBarMean) && dICAppXBarMean > dICXBarHigherMean))
			dICXBarHigherMean = dICAppXBarMean;

		dICRSumValues	+= dICAppSumValues;
		uiICValueCount	+= uiICAppValueCount;
	}

	if (!isnan(dICXBarLowerMean) && !isnan(dICXBarHigherMean))
		dICXBarDiff = dICXBarHigherMean - dICXBarLowerMean;

	// Compute IC EV
	m_allAppraiserDataset.m_dICEquipmentVariation = m_pEngine->computeEquipmentVariation(dICRSumValues, uiICValueCount);

	// Compute IC AV
	m_allAppraiserDataset.m_dICAppraiserVariation = m_pEngine->computeAppraiserVariation(dICXBarDiff, m_allAppraiserDataset.m_dICEquipmentVariation, uiICValueCount);

	// Worst Case
	for (uint uiBaseAppraiser = 0; uiBaseAppraiser < m_uiAppraisersCount; ++uiBaseAppraiser)
	{
		for (uint uiAppraiser = uiBaseAppraiser+1; uiAppraiser < m_uiAppraisersCount; ++uiAppraiser)
		{
			double	dWCXBarLowerMean	= std::numeric_limits<double>::quiet_NaN();
			double	dWCXBarHigherMean	= std::numeric_limits<double>::quiet_NaN();
			double	dWCXBarDiff			= std::numeric_limits<double>::quiet_NaN();

			double	dEV					= std::numeric_limits<double>::quiet_NaN();
			double	dAV					= std::numeric_limits<double>::quiet_NaN();
			double  dRAndR				= std::numeric_limits<double>::quiet_NaN();
			double	dWCRSumValues		= 0.0;
			uint	uiWCValueCount		= 0;

			QString	strAppraisers;

			pAppraiserDataset = m_lstAppraiserDataset[uiBaseAppraiser];

			strAppraisers		= pAppraiserDataset->m_strAppraiserName;
			dWCRSumValues		+= pAppraiserDataset->m_dRSumValues;
			uiWCValueCount		+= pAppraiserDataset->m_uiRValueCount;

			if (isnan(dWCXBarLowerMean) || (!isnan(pAppraiserDataset->XBarMean()) && pAppraiserDataset->XBarMean() < dWCXBarLowerMean))
				dWCXBarLowerMean = pAppraiserDataset->XBarMean();

			if (isnan(dWCXBarHigherMean) || (!isnan(pAppraiserDataset->XBarMean()) && pAppraiserDataset->XBarMean() > dWCXBarHigherMean))
				dWCXBarHigherMean = pAppraiserDataset->XBarMean();

			pAppraiserDataset = m_lstAppraiserDataset[uiAppraiser];

			strAppraisers		+= "|" + pAppraiserDataset->m_strAppraiserName;
			dWCRSumValues		+= pAppraiserDataset->m_dRSumValues;
			uiWCValueCount		+= pAppraiserDataset->m_uiRValueCount;

			if (isnan(dWCXBarLowerMean) || (!isnan(pAppraiserDataset->XBarMean()) && pAppraiserDataset->XBarMean() < dWCXBarLowerMean))
				dWCXBarLowerMean = pAppraiserDataset->XBarMean();

			if (isnan(dWCXBarHigherMean) || (!isnan(pAppraiserDataset->XBarMean()) && pAppraiserDataset->XBarMean() > dWCXBarHigherMean))
				dWCXBarHigherMean = pAppraiserDataset->XBarMean();

			// Compute XBar Diff
			if (!isnan(dWCXBarLowerMean) && !isnan(dWCXBarHigherMean))
				dWCXBarDiff = dWCXBarHigherMean - dWCXBarLowerMean;

			// Compute EV
			dEV = m_pEngine->computeEquipmentVariation(dWCRSumValues, uiWCValueCount);

			// Compute AV
			dAV = m_pEngine->computeAppraiserVariation(dWCXBarDiff, dEV, uiWCValueCount);

			// Compute R&R
			dRAndR = m_pEngine->computeRAndR(dEV, dAV);

			// Worst AV
			if (isnan(m_dWorstCaseAV) || dAV > m_dWorstCaseAV)
				m_dWorstCaseAV = dAV;

			// Worst R&R
			if (isnan(m_dWorstCaseRAndR) || dRAndR > m_dWorstCaseRAndR)
			{
				m_dWorstCaseRAndR = dRAndR;
				m_strMaxDiffRAndR = strAppraisers;
			}
		}
	}
}

void GexXBarRDatasetPrivate::setLowTestLimit(uint uiAppraiserIndex, double dLowLimit)
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	if (isnan(m_allAppraiserDataset.m_dLowTestLimit))
		m_allAppraiserDataset.m_dLowTestLimit = dLowLimit;
	else
		m_allAppraiserDataset.m_dLowTestLimit = qMin(m_allAppraiserDataset.m_dLowTestLimit, dLowLimit);

	m_lstAppraiserDataset[uiAppraiserIndex]->m_dLowTestLimit = dLowLimit;
}

void GexXBarRDatasetPrivate::setHighTestLimit(uint uiAppraiserIndex, double dHighLimit)
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	if (isnan(m_allAppraiserDataset.m_dHighTestLimit))
		m_allAppraiserDataset.m_dHighTestLimit = dHighLimit;
	else
		m_allAppraiserDataset.m_dHighTestLimit = qMax(m_allAppraiserDataset.m_dHighTestLimit, dHighLimit);

	m_lstAppraiserDataset[uiAppraiserIndex]->m_dHighTestLimit = dHighLimit;
}

void GexXBarRDatasetPrivate::setAppraiserName(uint uiAppraiserIndex, const QString& strAppraiserName)
{
	GEX_ASSERT(uiAppraiserIndex < m_uiAppraisersCount);

	m_lstAppraiserDataset[uiAppraiserIndex]->m_strAppraiserName = strAppraiserName;
}

double GexXBarRDatasetPrivate::minSample() const
{
	return m_dMinSamples;
}

double GexXBarRDatasetPrivate::maxSample() const
{
	return m_dMaxSamples;
}

double GexXBarRDatasetPrivate::meanSample() const
{
	if (m_uiTotalSamples)
		return (m_dSumSamples / ((double) m_uiTotalSamples));

	return std::numeric_limits<double>::quiet_NaN();
}
