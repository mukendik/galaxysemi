#ifndef GEX_XBAR_R_DATA_P_H
#define GEX_XBAR_R_DATA_P_H

///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_xbar_r_engine.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QList>
#include <QString>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXbarRSubDatasetPrivate
//
// Description	:	Private class to store data related to a XBar R dataset
//
///////////////////////////////////////////////////////////////////////////////////
class GexXbarRSubDatasetPrivate
{
public:

	GexXbarRSubDatasetPrivate();

	double		XBarMean() const;
	double		RMean() const;

	uint		m_uiXBarValueCount;			// Nb of XBar values added into the sub dataset
	uint		m_uiRValueCount;			// Nb of R values added into the sub dataset

	uchar *		m_pOutlierDeviceArray;		// Array of device outside control limits

	double		m_dXBarUCL;					// XBar Upper Control Limit for the subdataset
	double		m_dXBarLCL;					// XBar Lower Control Limit for the subdataset
	double		m_dXBarSumValues;			// Sum of the XBar values for the subdataset
	double		m_dXBarMin;					// XBar min value
	double		m_dXBarMax;					// XBar max value

	double		m_dRUCL;					// R Upper Control Limit for the subdataset
	double		m_dRLCL;					// R Lower Control Limit for the subdataset
	double		m_dRSumValues;				// Sum of the R values for the subdataset
	double		m_dRMin;					// R min value
	double		m_dRMax;					// R max value

	double *	m_pXBarDataArray;			// Array of XBar values for the subdataset
	double *	m_pRDataArray;				// Array of R values for the subdataset

	double		m_dLowTestLimit;			// Low limit of the current test for the subdataset
	double		m_dHighTestLimit;			// High limit of the current test for the subdataset

	double		m_dEquipmentVariation;		// Equipment Variation
	double		m_dAppraiserVariation;		// Reproducibility or Appraiser variation

	double		m_dICEquipmentVariation;	// Equipment Variation
	double		m_dICAppraiserVariation;	// Reproducibility or Appraiser variation

	QString		m_strAppraiserName;			// Appraiser name
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarRDatasetPrivate
//
// Description	:	Represent a computed dataset for XBar R chart
//
///////////////////////////////////////////////////////////////////////////////////
class GexXBarRDatasetPrivate
{
public:

	GexXBarRDatasetPrivate(uint uiTrialsCount, uint uiDevicesCount, uint uiAppraisersCount, QString strUnits);
	~GexXBarRDatasetPrivate();

	uint								dataCount() const;
	uint								trialCount() const;
	uint								deviceCount() const;
	uint								appraiserCount() const;
	uint								goodDeviceCount() const;
	QString								units() const;

	// XBar data
	double								XBarUCL() const;
	double								XBarLCL() const;
	double								XBarMean() const;
	double								XBarMin() const;
	double								XBarMax() const;
	double *							XBarDataArray() const;

	double								XBarUCLAppraiser(uint uiAppraiserIndex) const;
	double								XBarLCLAppraiser(uint uiAppraiserIndex) const;
	double								XBarMeanAppraiser(uint uiAppraiserIndex) const;

	// R data
	double								RUCL() const;
	double								RLCL() const;
	double								RMean() const;
	double								RMin() const;
	double								RMax() const;
	double *							RDataArray() const;

	double								RUCLAppraiser(uint uiAppraiserIndex) const;
	double								RLCLAppraiser(uint uiAppraiserIndex) const;
	double								RMeanAppraiser(uint uiAppraiserIndex) const;

	// Common data
	double								lowTestLimit() const;
	double								highTestLimit() const;
	double								lowTestLimitAppraiser(uint uiAppraiserIndex) const;
	double								highTestLimitAppraiser(uint uiAppraiserIndex) const;

	double *							XBarRLabelArray() const;

	void								setLowTestLimit(uint uiAppraiserIndex, double dLowLimit);
	void								setHighTestLimit(uint uiAppraiserIndex, double dHighLimit);
	void								setAppraiserName(uint uiAppraiserIndex, const QString& strAppraiserName);

	void								addDataPoint(uint uiTrialIndex, uint uiDeviceIndex, const QVector<double>& vecPoint);
	void								computeControlLimits();

	double								minSample() const;
	double								maxSample() const;
	double								meanSample() const;

	double								EVAppraiser(uint uiAppraiserIndex) const;
	double								equipmentVariation() const;
	double								appraiserVariation() const;
	double								RandR() const;

	double								ICEquipmentVariation() const;
	double								ICAppraiserVariation() const;
	double								ICRandR() const;

	double								WCAppraiserVariation() const;
	double								WCRAndR() const;
	QString								WCAppraisers() const;

protected:

	uint								m_uiDevicesCount;
	uint								m_uiTrialsCount;
	uint								m_uiAppraisersCount;
	uint								m_uiOutlierCount;
	QString								m_strUnits;

	uint								m_uiTotalSamples;
	double								m_dSumSamples;
	double								m_dMinSamples;
	double								m_dMaxSamples;

	double								m_dWorstCaseAV;
	double								m_dWorstCaseRAndR;
	QString								m_strMaxDiffRAndR;

	GexXbarRSubDatasetPrivate			m_allAppraiserDataset;
	QList<GexXbarRSubDatasetPrivate*>	m_lstAppraiserDataset;
	double *							m_pXBarRLabelArray;

	GexXBarREngine *					m_pEngine;
};


#endif // GEX_XBAR_R_DATA_P_H
