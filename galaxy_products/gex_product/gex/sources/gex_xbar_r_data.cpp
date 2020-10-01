///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_xbar_r_data.h"
#include "gex_xbar_r_data_p.h"

///////////////////////////////////////////////////////////////////////////////////
// STD Includes
///////////////////////////////////////////////////////////////////////////////////
#include<limits>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexXBarRDataset
//
// Description	:	Class used to store a dataset for XBar R charts
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GexXBarRDataset::GexXBarRDataset(uint uiTrialsCount, uint uiDevicesCount, uint uiAppraiserCount, QString strUnits)
	: m_pPrivate(new GexXBarRDatasetPrivate(uiTrialsCount, uiDevicesCount, uiAppraiserCount, strUnits))
{

}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
GexXBarRDataset::~GexXBarRDataset()
{
	if (m_pPrivate)
	{
		delete m_pPrivate;
		m_pPrivate = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Creator
///////////////////////////////////////////////////////////////////////////////////
GexXBarRDataset * GexXBarRDataset::create(uint uiTrialsCount, uint uiDevicesCount, uint uiAppraiserCount, QString strUnits)
{
	if (uiTrialsCount > 0 && uiDevicesCount > 0 && uiAppraiserCount > 0)
		return new GexXBarRDataset(uiTrialsCount, uiDevicesCount, uiAppraiserCount, strUnits);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////
void GexXBarRDataset::addDataPoint(uint uiAppraiserIndex, uint uiDeviceIndex, const QVector<double>& vecDataPoint)
{
	m_pPrivate->addDataPoint(uiAppraiserIndex, uiDeviceIndex, vecDataPoint);
}

void GexXBarRDataset::computeControlLimits()
{
	m_pPrivate->computeControlLimits();
}

void GexXBarRDataset::setLowTestLimit(uint uiAppraiserIndex, double dLowLimit)
{
	m_pPrivate->setLowTestLimit(uiAppraiserIndex, dLowLimit);
}

void GexXBarRDataset::setHighTestLimit(uint uiAppraiserIndex, double dHighLimit)
{
	m_pPrivate->setHighTestLimit(uiAppraiserIndex, dHighLimit);
}

void GexXBarRDataset::setAppraiserName(uint uiAppraiserIndex, const QString& strAppraiserName)
{
	m_pPrivate->setAppraiserName(uiAppraiserIndex, strAppraiserName);
}

uint GexXBarRDataset::dataCount() const
{
	return m_pPrivate->dataCount();
}

uint GexXBarRDataset::trialCount() const
{
	return m_pPrivate->dataCount();
}

uint GexXBarRDataset::appraiserCount() const
{
	return m_pPrivate->appraiserCount();
}

uint GexXBarRDataset::deviceCount() const
{
	return m_pPrivate->deviceCount();
}

QString GexXBarRDataset::units() const
{
	return m_pPrivate->units();
}

double GexXBarRDataset::XBarUCL() const
{
	return m_pPrivate->XBarUCL();
}

double GexXBarRDataset::XBarLCL() const
{
	return m_pPrivate->XBarLCL();
}

double GexXBarRDataset::XBarMean() const
{
	return m_pPrivate->XBarMean();
}

double GexXBarRDataset::XBarMin() const
{
	return m_pPrivate->XBarMin();
}

double GexXBarRDataset::XBarMax() const
{
	return m_pPrivate->XBarMax();
}

double * GexXBarRDataset::XBarDataArray() const
{
	return m_pPrivate->XBarDataArray();
}

double GexXBarRDataset::RUCL() const
{
	return m_pPrivate->RUCL();
}

double GexXBarRDataset::RLCL() const
{
	return m_pPrivate->RLCL();
}

double GexXBarRDataset::RMean() const
{
	return m_pPrivate->RMean();
}

double GexXBarRDataset::RMin() const
{
	return m_pPrivate->RMin();
}

double GexXBarRDataset::RMax() const
{
	return m_pPrivate->RMax();
}

double * GexXBarRDataset::RDataArray() const
{
	return m_pPrivate->RDataArray();
}

double * GexXBarRDataset::XBarRLabelArray() const
{
	return m_pPrivate->XBarRLabelArray();
}

double GexXBarRDataset::lowTestLimit() const
{
	return m_pPrivate->lowTestLimit();
}

double GexXBarRDataset::highTestLimit() const
{
	return m_pPrivate->highTestLimit();
}

double GexXBarRDataset::lowTestLimitAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->lowTestLimitAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::highTestLimitAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->highTestLimitAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::XBarUCLAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->XBarUCLAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::XBarLCLAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->XBarLCLAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::XBarMeanAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->XBarMeanAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::RLCLAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->RLCLAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::RUCLAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->RUCLAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::RMeanAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->RMeanAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::minSample() const
{
	return m_pPrivate->minSample();
}

double GexXBarRDataset::maxSample() const
{
	return m_pPrivate->maxSample();
}

double GexXBarRDataset::meanSample() const
{
	return m_pPrivate->meanSample();
}

uint GexXBarRDataset::goodDeviceCount() const
{
	return m_pPrivate->goodDeviceCount();
}

double GexXBarRDataset::EVAppraiser(uint uiAppraiserIndex) const
{
	return m_pPrivate->EVAppraiser(uiAppraiserIndex);
}

double GexXBarRDataset::appraiserVariation() const
{
	return m_pPrivate->appraiserVariation();
}

double GexXBarRDataset::equipmentVariation() const
{
	return m_pPrivate->equipmentVariation();
}

double GexXBarRDataset::ICAppraiserVariation() const
{
	return m_pPrivate->ICAppraiserVariation();
}

double GexXBarRDataset::ICEquipmentVariation() const
{
	return m_pPrivate->ICEquipmentVariation();
}

double GexXBarRDataset::ICRandR() const
{
	return m_pPrivate->ICRandR();
}

double GexXBarRDataset::RandR() const
{
	return m_pPrivate->RandR();
}

double GexXBarRDataset::WCAppraiserVariation() const
{
	return m_pPrivate->WCAppraiserVariation();
}

double GexXBarRDataset::WCRAndR() const
{
	return m_pPrivate->WCRAndR();
}

QString GexXBarRDataset::WCAppraisers() const
{
	return m_pPrivate->WCAppraisers();
}
