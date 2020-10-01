///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexdatasetconfig.h"
#include "gexdatasetconfigio.h"
#include "gex_constants.h"
#include "ctest.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexDatasetConfig
//
// Description	:	Class holding the config for a dataset
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexDatasetConfig::CGexDatasetConfig()
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexDatasetConfig::~CGexDatasetConfig()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void load(const QString& strDatasetConfig)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CGexDatasetConfig::load(const QString& strDatasetConfig)
{
	m_strDatasetConfig = strDatasetConfig;

	if (m_strDatasetConfig.isEmpty() == false)
	{
		CGexAbstractDatasetConfigIO * pDatasetConfigIO = CGexAbstractDatasetConfigIO::create(m_strDatasetConfig);

		if (pDatasetConfigIO)
		{
			pDatasetConfigIO->read(this);

			delete pDatasetConfigIO;
			pDatasetConfigIO = NULL;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void write(const QString& strDatasetConfig)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CGexDatasetConfig::write(const QString& strDatasetConfig)
{
	m_strDatasetConfig = strDatasetConfig;

	if (m_strDatasetConfig.isEmpty() == false)
	{
		CGexXmlDatasetConfigIO datasetConfigIO(m_strDatasetConfig);

		datasetConfigIO.write(this);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addPartFilter(long lTestNumber, double dValue, CGexPartFilter::filterType eFilterType)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CGexDatasetConfig::addPartFilter(long lTestNumber, double dValue, CGexPartFilter::filterType eFilterType)
{
	switch (eFilterType)
	{
		case CGexPartFilter::filterOver		:	addPartFilter(lTestNumber, dValue, GEX_C_DOUBLE_NAN, eFilterType);
												break;

		case CGexPartFilter::filterUnder	:	addPartFilter(lTestNumber, GEX_C_DOUBLE_NAN, dValue, eFilterType);
												break;

		default								:	break;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addPartFilter(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eFilterType)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CGexDatasetConfig::addPartFilter(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eFilterType)
{
	m_partFilter.addFilter(lTestNumber, dLowValue, dHighValue, eFilterType);
}
