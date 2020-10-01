///////////////////////////////////////////////////////////
// ALL Browser wizard pages for 'Analyze Dataset' (1 query) : Pick & SqlPick
///////////////////////////////////////////////////////////
#ifdef _WIN32
  #include <windows.h>
#endif

#include "gex_shared.h"
#include "db_engine.h"
#include "db_onequery_wizard.h"
#include "report_build.h"
#include "picktest_dialog.h"
#include "pickfilter_dialog.h"
#include "pickproduct_idsql_dialog.h"
#include "pickbin_single_dialog.h"
#include <gqtl_log.h>
#include "engine.h"

#include <qtooltip.h>

// in main.cpp
extern GexMainwindow	*pGexMainWindow;

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

extern bool FillComboBox(QComboBox * pCombo, const char * szTextTab[]);

///////////////////////////////////////////////////////////
// Pick Filter string from list, update GUI accordingly.
// Under this mode, filter IS contextual: and ONLY lists
// database values matching the other filter criteria
// As we have up-to 5 filters, we have this function fill
// a list box of the Query filter, but narrowing the fill
// checking all the other 7 filters match the lookup criteria.
///////////////////////////////////////////////////////////
void	GexOneQueryWizardPage1::PickFilterFromLiveList(const GexDatabaseFilter &dbFilter,
                                                       QComboBox * pQueryValue /*= NULL*/,
                                                       bool lStandardFilter /*= true*/)
{
    // Fill Filter list with relevant strings
    PickFilterDialog dPickFilter;
    dPickFilter.fillList(dbFilter);

    // Prompt dialog box, let user pick Filter string from the list
    if(dPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Save the list selected into the edit field...unless it is already in!
    QString strSelection = dPickFilter.filterList();

    if (pQueryValue)
    {
        for(int i = 0; i < pQueryValue->count(); i++)
        {
            if(strSelection == pQueryValue->itemText(i))
            {
                // Selection already in combo...simply select it.
                SetCurrentComboItem(pQueryValue, strSelection);
                return;
            }
        }

        pQueryValue->insertItem(pQueryValue->count(),strSelection);
        SetCurrentComboItem(pQueryValue, strSelection);

        // Reset HTML sections to create flag: ALL pages to create.
        if (lStandardFilter)
            OnStandardFilterChange("");
        else
            OnTCFilterChange("");
    }
}

///////////////////////////////////////////////////////////
// Display list of Parameters available in database, and
// return user selection
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
QString GexOneQueryWizardPage1::SqlPickParameterList(bool bParametricOnly)
{
    GexDatabaseFilter dbFilter;

    // Get the current filters
    FillDatabaseFilter(dbFilter);

    // Get list of strings to fill in list box
    QStringList cParameterList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetParameterList(dbFilter, bParametricOnly);

    // Fill Dialog box with parameter list...
    // Show TestList
    PickTestDialog dPickTest;

    // Allow Multiple selections.
    dPickTest.setMultipleSelection(true);

    // Check if List was successfuly loaded
    if (dPickTest.fillParameterList(cParameterList))
    {
        // Prompt dialog box, let user pick tests from the list
        if(dPickTest.exec() == QDialog::Accepted)
            // Return user selection
            return dPickTest.testList();
    }

    return "";
}

///////////////////////////////////////////////////////////
// Display list of Parameters available in database, and
// return user selection
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
QString	GexOneQueryWizardPage1::SqlPickParameterList(bool bParametricOnly,
                                                     bool bMultiSelection,
                                                     const QString & strDataType,
                                                     const QString & strProductName)
{
    GexDatabaseFilter dbFilter;

    // Get the current filters
    FillDatabaseFilter(dbFilter);

    // Set the query filter
    dbFilter.addNarrowFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strProductName);
//    dbFilter.setQueryFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strProductName);
    dbFilter.strDataTypeQuery = strDataType;

    // Get list of strings to fill in list box
    QStringList cParameterList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetParameterList(dbFilter, bParametricOnly);

    // Fill Dialog box with parameter list...
    // Show TestList
//	PickTestDialog dPickTest(false,true);		//PYC, 30/05/2011
    PickTestDialog dPickTest(NULL,true);

    // Allow Multiple selections ?
    dPickTest.setMultipleSelection(bMultiSelection);

    // Check if List was successfuly loaded
    if (dPickTest.fillParameterList(cParameterList))
    {
        // Prompt dialog box, let user pick tests from the list
        if(dPickTest.exec() == QDialog::Accepted)
            // Return user selection
            return dPickTest.testList();
    }

    return "";
}

///////////////////////////////////////////////////////////
// Display list of Binnings available in database for a given
// DataType, ProductName, and return user selection
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
QString GexOneQueryWizardPage1::SqlPickBinningList(bool bMultiSelection, QString strDataType, QString strProductName, bool bSoftBin/*=false*/)
{
    GexDatabaseFilter dbFilter;

    // Get the current filters
    FillDatabaseFilter(dbFilter);

    // Set the query filter
    dbFilter.addNarrowFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strProductName);
//    dbFilter.setQueryFilter(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], strProductName);
    dbFilter.strDataTypeQuery = strDataType;

    // Get list of strings to fill in list box
    QStringList cBinningList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetBinningList(dbFilter, bSoftBin);
    if(cBinningList.count() == 0)
        return "";

    // Fill Filter list with relevant strings
    PickBinSingleDialog dPickFilter;
    dPickFilter.fillList(cBinningList);
    dPickFilter.setMultipleSelection(bMultiSelection);

    // Prompt dialog box, let user pick Filter string from the list
    if(dPickFilter.exec() != 1)
        return "";	// User 'Abort'

    // Return the list selected (with '|' replaced by ',', wich is more appropriate for a binlist)
    return dPickFilter.getBinsList().replace('|',',');
}

///////////////////////////////////////////////////////////
// Display list of Binnings available in database, and
// return user selection
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
QString GexOneQueryWizardPage1::SqlPickBinningList(bool bMultiSelection, bool bSoftBin/*=false*/)
{
    GexDatabaseFilter dbFilter;

    // Get current filters
    FillDatabaseFilter(dbFilter);

    // Get list of strings to fill in list box
    QStringList cBinningList = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetBinningList(dbFilter, bSoftBin);
    if(cBinningList.count() == 0)
        return "";

    // Fill Filter list with relevant strings
    PickBinSingleDialog dPickFilter;
    dPickFilter.fillList(cBinningList);
    dPickFilter.setMultipleSelection(bMultiSelection);

    // Prompt dialog box, let user pick Filter string from the list
    if(dPickFilter.exec() != 1)
        return "";	// User 'Abort'

    // Return the list selected (with '|' replaced by ',', wich is more appropriate for a binlist)
    return dPickFilter.getBinsList().replace('|',',');
}

/////////////////////////////////////////////////////////////////////
// Display list of Products available in database, and
// return user selection
// Note:
// 1) only applies to SQL databases (Galaxy, mySQL,...)
// 2) applies to genealogy, will return only products having
//    data for at least 2 testing stages (based on tracking_lot)
/////////////////////////////////////////////////////////////////////
void GexOneQueryWizardPage1::SqlPickProductList_Genealogy(QStringList & strlProducts, bool bAllTestingStages, bool bMultiSelection, QString strDataType)
{
    // Only use the date filter, and eventually the testing stage
    GexDatabaseFilter dbFilter;

    // Get the current filter
    FillDatabaseFilter(dbFilter);

    if(strDataType.isEmpty() == false)
        dbFilter.strDataTypeQuery = strDataType;

    // Get list of strings to fill in list box
    strlProducts = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetProductList_Genealogy(dbFilter, bAllTestingStages);
    if(strlProducts.count() == 0)
        return;

    // Fill Filter list with relevant strings
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 products found. Fill and exec PickProductDialog...")
          .arg(strlProducts.count())
          .toLatin1().constData());
    PickProductIdSQLDialog dPickFilter;
    dPickFilter.fillList(strlProducts, bMultiSelection, true);

    // Prompt dialog box, let user pick Filter string from the list
    strlProducts.clear();
    if(dPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    dPickFilter.getProductList(strlProducts);
}

///////////////////////////////////////////////////////////
// Display list of Products available in database, and
// return user selection
// Note: only applies to SQL databases (Galaxy, mySQL,...)
///////////////////////////////////////////////////////////
void
GexOneQueryWizardPage1::SqlPickProductList(QStringList& strlProducts,
                                           bool bMultiSelection,
                                           QString& strDataType,
                                           QString /*strValue = ""*/)
{
    // Only use the date filter, and eventually the testing stage
    GexDatabaseFilter dbFilter;

    // Get the current filter
    FillDatabaseFilter(dbFilter);

    if(strDataType.isEmpty() == false)
        dbFilter.strDataTypeQuery = strDataType;

    // Get list of strings to fill in list box
    strlProducts = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QueryGetProductList(dbFilter);
    if(strlProducts.count() == 0)
        return;

    // Fill Filter list with relevant strings
    PickProductIdSQLDialog dPickFilter;
    dPickFilter.fillList(strlProducts, bMultiSelection, false);

    // Prompt dialog box, let user pick Filter string from the list
    strlProducts.clear();
    if(dPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    dPickFilter.getProductList(strlProducts);
}
