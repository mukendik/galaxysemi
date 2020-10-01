#ifndef GEXDB_PLUGIN_SYA_H
#define GEXDB_PLUGIN_SYA_H

//#include "gexdb_plugin_base.h"	// for GexDbPlugin_BinList
#include <QString>

// SYA Fail status
enum SyaFailStatus
{
	eSyaFail_None,
	eSyaFail_Standard,
	eSyaFail_Critical
};

#include "gexdb_plugin_bin_info.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_SYA_Item
// Holds 1 item (wafer or lot) for SYL/SBL simulation
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_SYA_Item
{
public:
	GexDbPlugin_SYA_Item()
	{
		Reset();
	}
	~GexDbPlugin_SYA_Item()
	{
		Reset();
	}
	void Reset(void)
	{
		m_strLotID = "";
		m_strWaferID = "";
		m_uiPartsTested = 0;
		m_uiGrossDie = 0;
		m_uiPartsForYieldComputation = 0;
		m_uiTotalGood = 0;
		m_fYield = 0.0F;
		m_clBinList.clear();
		m_eFailStatus_SYL = eSyaFail_None;
		m_eFailStatus_SBL = eSyaFail_None;
	};

	QString					m_strLotID;						// LotID of this item
	QString					m_strWaferID;					// WaferID of this item
	QDateTime				m_clWaferDate;					// Date wafer was tested
	unsigned int			m_uiPartsTested;				// Total parts tested
	unsigned int			m_uiGrossDie;					// Gross Die
	unsigned int			m_uiPartsForYieldComputation;	// Nb parts to be used for yield computation
	unsigned int			m_uiTotalGood;					// Total good parts
	float					m_fYield;						// Yield for this item
	GexDbPlugin_BinList		m_clBinList;					// List of binnigs + count
	SyaFailStatus			m_eFailStatus_SYL;				// SYL fail status
	SyaFailStatus			m_eFailStatus_SBL;				// SBL fail status
};


/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_SYA_ItemList
// Holds a list of item GexDbPlugin_SYA_Item pointers
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_SYA_ItemList : public QList<GexDbPlugin_SYA_Item *>
{
public:
	GexDbPlugin_SYA_ItemList()
	{
		Reset();
		m_listBinnings.SetSortMode(GexDbPlugin_BinList::eSortOnBinNo, true);
	};
	~GexDbPlugin_SYA_ItemList()
	{
		Reset();
	};
	void Reset(void)
	{
		while(!isEmpty())
			delete takeFirst();
		m_uiItemsProcessed = 0;
		m_listBinnings.clear();
	};
	unsigned int		m_uiItemsProcessed;
	GexDbPlugin_BinList	m_listBinnings;
};

#endif // GEXDB_PLUGIN_SYA_H
