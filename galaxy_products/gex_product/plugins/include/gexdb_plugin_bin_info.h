#ifndef GEXDB_PLUGIN_BIN_INFO_H
#define GEXDB_PLUGIN_BIN_INFO_H

#include <QMap>
#include <QVariant>
#include "gexdb_plugin_base.h"	// for OutlierRule

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_BinInfo
// Holds info for 1 binning
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_BinInfo
{
public:
	GexDbPlugin_BinInfo();
    ~GexDbPlugin_BinInfo();
    // Reset bin_no to -1, all the 4 limits are set to -1, OutlierRule type set to Default, ...
	void Reset();
    // assignment operator : copy bin name, bin cat, bin count, bin no, rule type, N1, N2, ...
	GexDbPlugin_BinInfo& operator=(const GexDbPlugin_BinInfo& source);
    /*!
     * \fn operator==
     */
    bool operator==(const GexDbPlugin_BinInfo& rhs) const;

	QString			m_strBinName;		// BIN name
	QChar			m_cBinCat;			// BIN category ('P' or 'F')
	int				m_nBinCount;		// Nb of parts with this binning
	int				m_nBinNo;			// BIN number (SYA: bin# is -1 for SYL)
	float			m_fLL1;				// Binning LL1 (SYA: -1.0 if limit disabled)
	float			m_fHL1;				// Binning HL1 (SYA: -1.0 if limit disabled)
	float			m_fLL2;				// Binning LL2 (SYA: -1.0 if limit disabled)
	float			m_fHL2;				// Binning HL2 (SYA: -1.0 if limit disabled)
	SyaFailStatus	m_eFailStatus_SBL;	// SBL fail status

	// Used by SYA
	OutlierRule	m_eRuleType;	// used by SYA
	float			m_fN1;
	float			m_fN2;
private :
    QMap< QString, QVariant > mAttributes;
	static int n_instances;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_BinList
// Holds a map of GexDbPlugin_BinInfo objects
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_BinList: public QList<GexDbPlugin_BinInfo*>
{
public:
    GexDbPlugin_BinList(int eSortSelector=eSortOnBinCount, bool bSortAscending=true);

    virtual ~GexDbPlugin_BinList();

    typedef enum
	{
		eSortOnBinNo,			// Sorting should be done on BinNo field
		eSortOnBinName,			// Sorting should be done on BinName field
		eSortOnBinCount			// Sorting should be done on BinCount field
    }SortOn;

// PUBLIC METHODS
public:
	void SetSortMode(SortOn eSortSelector, bool bAscending)
	{
        GexDbPlugin_BinList::sSortSelector = eSortSelector;
        GexDbPlugin_BinList::sSortAscending = bAscending;
	}
    void Sort(SortOn eSortSelector, bool bAscending);

    // For sorting
    static int  sSortSelector;     // Tells on which var to sort
    static bool sSortAscending;	// true if sorting should be ascending
};

#endif // GEXDB_PLUGIN_BIN_INFO_H
