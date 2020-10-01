#ifndef _GEX_DATASET_CONFIG_H_
#define _GEX_DATASET_CONFIG_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gextestmapping.h"
#include "gextestfilter.h"
#include "gexpartfilter.h"
#include "gex_test_to_create.h"
#include "gex_test_to_update.h"
///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexDatasetConfig
//
// Description	:	Class holding the config for a dataset
//
///////////////////////////////////////////////////////////////////////////////////
class CGexDatasetConfig
{
public:

    CGexDatasetConfig();
    ~CGexDatasetConfig();

    const CGexTestMapping&		testMapping() const			{ return m_testMapping;	}
    const GS::Gex::TestFilter&	testFilter() const			{ return m_testFilter; }
    const CGexPartFilter&		partFilter() const			{ return m_partFilter; }
    const GexTestToCreateList&	testToCreateList() const	{ return m_testToCreateList; }
    const GexTestToUpdateList&	testToUpdateList() const	{ return m_testToUpdateList; }

    void						load(const QString& strDatasetConfig);
    void						write(const QString& strDatasetConfig);

    void						addPartFilter(long lTestNumber, double dValue, CGexPartFilter::filterType eFilterType);
    void						addPartFilter(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eFilterType);

    friend class CGexTxtDatasetConfigIO;
    friend class CGexXmlDatasetConfigIO;

protected:

    CGexTestMapping		m_testMapping;
    GS::Gex::TestFilter	m_testFilter;
    CGexPartFilter		m_partFilter;
    GexTestToCreateList m_testToCreateList;
    GexTestToUpdateList m_testToUpdateList;

    QString				m_strDatasetConfig;
};

#endif // _GEX_DATASET_CONFIG_H_
