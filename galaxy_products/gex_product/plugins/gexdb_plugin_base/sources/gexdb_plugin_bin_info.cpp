#include "gexdb_plugin_base.h"

int GexDbPlugin_BinInfo::n_instances=0;

GexDbPlugin_BinInfo::GexDbPlugin_BinInfo()
{
  n_instances++;
	Reset();
}

GexDbPlugin_BinInfo::~GexDbPlugin_BinInfo()
{
  n_instances--;
}

GexDbPlugin_BinInfo& GexDbPlugin_BinInfo::operator=(const GexDbPlugin_BinInfo& source)
{
    m_strBinName		= source.m_strBinName;
	m_cBinCat			= source.m_cBinCat;
	m_nBinCount			= source.m_nBinCount;
	m_nBinNo			= source.m_nBinNo;
	m_fLL1				= source.m_fLL1;
	m_fHL1				= source.m_fHL1;
	m_fLL2				= source.m_fLL2;
	m_fHL2				= source.m_fHL2;
	m_eFailStatus_SBL	= source.m_eFailStatus_SBL;
	m_eRuleType			= source.m_eRuleType;
	m_fN1				= source.m_fN1;
	m_fN2				= source.m_fN2;

	return *this;
}

/******************************************************************************!
 * \fn operator==
 ******************************************************************************/
bool GexDbPlugin_BinInfo::operator==(const GexDbPlugin_BinInfo& rhs) const
{
    return m_nBinNo == rhs.m_nBinNo;
}

/******************************************************************************!
 * \fn Reset
 ******************************************************************************/
void GexDbPlugin_BinInfo::Reset()
{
	m_strBinName		= "";
	m_cBinCat			= '?';
	m_nBinCount			= 0;
	m_nBinNo			= -1;
	m_fLL1 = m_fHL1 = m_fLL2 = m_fHL2 = -1.0F;
	m_eFailStatus_SBL	= eSyaFail_None;

	m_eRuleType			= eDefault;
	m_fN1 = m_fN2		= -1.0F;
}

