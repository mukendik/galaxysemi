#ifndef GEX_XBAR_R_DATA_H
#define GEX_XBAR_R_DATA_H

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QVector>

///////////////////////////////////////////////////////////
// Private object
///////////////////////////////////////////////////////////
class GexXBarRDatasetPrivate;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarRDataset
//
// Description	:	Class used to store a dataset for XBar R charts
//
///////////////////////////////////////////////////////////////////////////////////

class GexXBarRDataset
{
	GexXBarRDataset(uint uiTrialsCount, uint uiDevicesCount, uint uiAppraiserCount, QString strUnits);

public:

	~GexXBarRDataset();

	static GexXBarRDataset *	create(uint uiTrialsCount, uint uiDevicesCount, uint uiAppraiserCount, QString strUnits);

	void						addDataPoint(uint uiAppraiserIndex, uint uiDeviceIndex, const QVector<double>& vecDataPoint);
	void						computeControlLimits();

	uint						dataCount() const;
	uint						trialCount() const;
	uint						deviceCount() const;
	uint						appraiserCount() const;
	uint						goodDeviceCount() const;
	QString						units() const;

	// XBar data
	double						XBarUCL() const;
	double						XBarLCL() const;
	double						XBarMean() const;
	double						XBarMin() const;
	double						XBarMax() const;
	double *					XBarDataArray() const;

	double						XBarUCLAppraiser(uint uiAppraiserIndex) const;
	double						XBarLCLAppraiser(uint uiAppraiserIndex) const;
	double						XBarMeanAppraiser(uint uiAppraiserIndex) const;

	// R data
	double						RUCL() const;
	double						RLCL() const;
	double						RMean() const;
	double						RMin() const;
	double						RMax() const;
	double *					RDataArray() const;

	double						RUCLAppraiser(uint uiAppraiserIndex) const;
	double						RLCLAppraiser(uint uiAppraiserIndex) const;
	double						RMeanAppraiser(uint uiAppraiserIndex) const;

	// Common data
	double						lowTestLimit() const;
	double						highTestLimit() const;
	double						lowTestLimitAppraiser(uint uiAppraiserIndex) const;
	double						highTestLimitAppraiser(uint uiAppraiserIndex) const;
	double *					XBarRLabelArray() const;

	void						setLowTestLimit(uint uiAppraiserIndex, double dLowLimit);
	void						setHighTestLimit(uint uiAppraiserIndex, double dHighLimit);
	void						setAppraiserName(uint uiAppraiserIndex, const QString& strAppraiserName);

	double						minSample() const;
	double						maxSample() const;
	double						meanSample() const;

	double						EVAppraiser(uint uiAppraiserIndex) const;
	double						equipmentVariation() const;
	double						appraiserVariation() const;
	double						RandR() const;

	double						ICEquipmentVariation() const;
	double						ICAppraiserVariation() const;
	double						ICRandR() const;

	double						WCAppraiserVariation() const;
	double						WCRAndR() const;
	QString						WCAppraisers() const;

private:

	GexXBarRDatasetPrivate	*	m_pPrivate;
};

#endif // GEX_XBAR_R_DATA_H
