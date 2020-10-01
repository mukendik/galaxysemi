#include "gexdb_plugin_base.h"
#include "gexdb_plugin_bin_info.h"

bool CompareBins(GexDbPlugin_BinInfo *binInfo1, GexDbPlugin_BinInfo *binInfo2)
{
    switch (GexDbPlugin_BinList::sSortSelector)
    {
        case GexDbPlugin_BinList::eSortOnBinNo:
        {
            if(binInfo1->m_nBinNo > binInfo2->m_nBinNo)
            {
                return GexDbPlugin_BinList::sSortAscending ? false : true;
            }

            if(binInfo1->m_nBinNo < binInfo2->m_nBinNo)
            {
                return GexDbPlugin_BinList::sSortAscending ? true : false;
            }

            return false;
        };
        case GexDbPlugin_BinList::eSortOnBinName:
        {
            if(binInfo1->m_strBinName > binInfo2->m_strBinName)
            {
                return GexDbPlugin_BinList::sSortAscending ? false : true;
            }

            if(binInfo1->m_strBinName < binInfo2->m_strBinName)
            {
                return GexDbPlugin_BinList::sSortAscending ? true : false;
            }

            return false;
        };
        case GexDbPlugin_BinList::eSortOnBinCount:
        {
            if(binInfo1->m_nBinCount > binInfo2->m_nBinCount)
            {
                return GexDbPlugin_BinList::sSortAscending ? false: true;
            }

            if(binInfo1->m_nBinCount < binInfo2->m_nBinCount)
            {
                return GexDbPlugin_BinList::sSortAscending ? true : false;
            }

            return false;
        }
    }
    return false;
}


int GexDbPlugin_BinList::sSortSelector = 0;
bool GexDbPlugin_BinList::sSortAscending = false;

GexDbPlugin_BinList::GexDbPlugin_BinList(int eSortSelector, bool bSortAscending)
{
  // Set autodelete flag to true
//  setAutoDelete(true);

  // Set sorting parameters
  GexDbPlugin_BinList::sSortAscending = bSortAscending;
  GexDbPlugin_BinList::sSortSelector = eSortSelector;
}

GexDbPlugin_BinList::~GexDbPlugin_BinList()
{
    clear();
}

void GexDbPlugin_BinList::Sort(SortOn eSortSelector, bool bAscending)
{
    // Set sort parameters
    GexDbPlugin_BinList::sSortSelector = eSortSelector;
    GexDbPlugin_BinList::sSortAscending = bAscending;

    // Now sort
    qSort(begin(), end(), CompareBins);
}
