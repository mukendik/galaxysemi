//===============================================================================================
// Manage Production Recipe file (Read/Write)
//===============================================================================================

#include <QtWidgets>
#include <QRegExp>

#include "prod_recipe_file.h"
#include "patsrv_ini.h"

//===============================================================================================
// Single flow details: Constructor
//===============================================================================================
CRecipeFlow::CRecipeFlow()
{
    clear();
}

//===============================================================================================
// Single flow details: clear
//===============================================================================================
void CRecipeFlow::clear(void)
{
    m_iDataSources=0;		// 0=MAP, 1=STDF, 2= MAP+STDF (see FLOW_SOURCE_xxx)
    m_strGoodHbins="1";		// Good HBINs list (eg: "1-5,1000")
    m_strENG_RecipeName="";	// Engineering Recipe associated with this flow.
    m_strENG_Version="";	// Engineering Recipe version to use in this flow.
    m_strENG_RecipeFile="";	// Engineering Recipe file name.
}

//===============================================================================================
// Flow name of specific flow in PRO recipe
//===============================================================================================
QString CRecipeFlow::strSourceName(void)
{
    switch(m_iDataSources)
    {
        default:
        case FLOW_SOURCE_MAP: return FLOW_SOURCE_MAP_STRING;

        case FLOW_SOURCE_STDF: return FLOW_SOURCE_STDF_STRING;

        case FLOW_SOURCE_MAP_STDF: return FLOW_SOURCE_MAP_STDF_STRING;
    }
}

//===============================================================================================
// Flow ID of specific flow in PRO recipe
//===============================================================================================
int CRecipeFlow::iSourceID(QString strSourceName)
{
    if(strSourceName == FLOW_SOURCE_MAP_STRING)
        return FLOW_SOURCE_MAP;
    else
    if(strSourceName == FLOW_SOURCE_STDF_STRING)
        return FLOW_SOURCE_STDF;
    else
        return FLOW_SOURCE_MAP_STDF;
}

//===============================================================================================
// Structure holding recipe settings: constructor
//===============================================================================================
CPROD_Recipe::CPROD_Recipe()
{
    clear();
}

//===============================================================================================
// Structure holding recipe settings: constructor
//===============================================================================================
void	CPROD_Recipe::clear(void)
{
    m_strGroup="";
    m_strRecipeName="";	    // Recipe file name (without version# nor Galaxy suffix string)
    m_strRecipeFile="";	    // Recipe file name.
    m_strComment="";
    m_iVersion=1;		// Recipe version
    m_iBuild=0;
    m_iEnabled = 0;		// Rule disabled by default.
    m_iPPAT_Bin=-1;	    // PPAT overloading bin; -1 for no overload
    m_iGPAT_Bin=-1;	    // GPAT overloading bin; -1 for no overload
    m_iZPAT_Bin=-1;	    // ZPAT overloading bin; -1 for no overload
    m_iStdfOutput=2;	// 0= No STDF 1= STDF MAP only; 2= STDF Output - Full contents;

    // Clear flows
    m_cFlows.clear();
}

//===============================================================================================
// Load PROD recipe file.
//===============================================================================================
bool	CPROD_Recipe::loadRecipe(QString strPROD_RecipeFile,QString &strErrorMessage)
{
    QString	    strString;
    CRecipeFlow	cFlowEntry;
    int		    iEntry,iTotalFlows;

    // Open Recipe file (.INI format)
    QSettings settings(strPROD_RecipeFile,QSettings::IniFormat);

    // Seek to global section
    settings.beginGroup("PROD_global");

    m_strGroup = settings.value("group","").toString();
    m_strRecipeName = settings.value("recipe_name","").toString();
    m_strRecipeFile = settings.value("recipe_file","").toString();
    m_strComment = settings.value("comment","").toString();
    m_iVersion = settings.value("version",-1).toInt();
    m_iBuild = settings.value("build",0).toInt();
    m_iPPAT_Bin = settings.value("ppat_bin",-1).toInt();
    m_iGPAT_Bin = settings.value("gpat_bin",-1).toInt();
    m_iZPAT_Bin = settings.value("zpat_bin",-1).toInt();
    m_iStdfOutput = settings.value("stdf_output",1).toInt();
    iTotalFlows = settings.value("total_flows","0").toInt();
    m_cFlows.clear();

    // Close group seek.
    settings.endGroup();

    // Check validity of this recipe file
    if(m_strGroup.isEmpty() || (m_iVersion < 0))
    {
        strErrorMessage = "Invalid PROD Recipe";
        return false;
    }

    // Read Flow definitions.
    for(iEntry=0;iEntry<iTotalFlows;iEntry++)
    {
        // Seek to relevant Flow: 'flow_X'
        strString.sprintf("flow_%d",iEntry);
        settings.beginGroup(strString);

        cFlowEntry.m_strFlow = settings.value("flow_name","").toString();
        cFlowEntry.m_iDataSources = settings.value("data_sources",0).toInt();	// 0=MAP, 1=STDF, 2= MAP+STDF (see FLOW_SOURCE_xxx)
        cFlowEntry.m_strGoodHbins = settings.value("good_hbins","1").toString();		// Good HBINs list (eg: "1-5,1000")
        cFlowEntry.m_strENG_RecipeName = settings.value("eng_recipe","").toString();		// Engineering Recipe associated with this flow.
        cFlowEntry.m_strENG_Version = settings.value("eng_version","").toString();	// Engineering Recipe version to use in this flow.
        cFlowEntry.m_strENG_RecipeFile = settings.value("eng_file","").toString();	// Engineering Recipe file.
        m_cFlows.append(cFlowEntry);

        // Close group seek.
        settings.endGroup();
    }

    return true;
}

//===============================================================================================
// Save PROD recipe file.
//===============================================================================================
bool CPROD_Recipe::saveRecipe(QString strPROD_RecipeFile,
                              QString& /*strErrorMessage*/)
{
    QString	    strString;
    CRecipeFlow	cFlowEntry;
    int		    iEntry;

    // Open Recipe file (.INI format)
    QSettings settings(strPROD_RecipeFile,QSettings::IniFormat);

    // Seek to global section
    settings.beginGroup("PROD_global");

    settings.setValue("group",m_strGroup);
    settings.setValue("recipe_name",m_strRecipeName);
    settings.setValue("recipe_file",m_strRecipeFile);
    settings.setValue("comment",m_strComment);
    settings.setValue("version",m_iVersion);
    settings.setValue("build",m_iBuild);
    settings.setValue("ppat_bin",m_iPPAT_Bin);
    settings.setValue("gpat_bin",m_iGPAT_Bin);
    settings.setValue("zpat_bin",m_iZPAT_Bin);
    settings.setValue("stdf_output",m_iStdfOutput);
    settings.setValue("total_flows",m_cFlows.count());

    // Close group seek.
    settings.endGroup();

    // Write Flow definitions.
    for(iEntry=0;iEntry<m_cFlows.count();iEntry++)
    {
        // Seek to relevant Flow: 'flow_X'
        strString.sprintf("flow_%d",iEntry);
        settings.beginGroup(strString);

        cFlowEntry = m_cFlows.at(iEntry);
        settings.setValue("flow_name",cFlowEntry.m_strFlow);
        settings.setValue("data_sources",cFlowEntry.m_iDataSources);	// 0=MAP, 1=STDF, 2= MAP+STDF (see FLOW_SOURCE_xxx)
        settings.setValue("good_hbins",cFlowEntry.m_strGoodHbins);		// Good HBINs list (eg: "1-5,1000")
        settings.setValue("eng_recipe",cFlowEntry.m_strENG_RecipeName);	// Engineering Recipe name associated with this flow.
        settings.setValue("eng_version",cFlowEntry.m_strENG_Version);	// Engineering Recipe version to use in this flow.
        settings.setValue("eng_file",cFlowEntry.m_strENG_RecipeFile);	// Engineering Recipe file

        // Close group seek.
        settings.endGroup();
    }
    return true;
}

//===============================================================================================
// Returns list of flows using specified data source (STDF or OLI/MAP)
//===============================================================================================
bool	CPROD_Recipe::getFlowList(int iDataSource,QStringList &strList)
{
    // Clear list
    strList.clear();

    // Enumerate list of flows
    CRecipeFlow cRecipeOneFlow;
    QString strString = "";
    int		iEntry;
    for(iEntry=0;iEntry<m_cFlows.count();iEntry++)
    {
        cRecipeOneFlow = m_cFlows.at(iEntry);
        switch(iDataSource)
        {
        case FLOW_SOURCE_MAP:
           if((cRecipeOneFlow.m_iDataSources == FLOW_SOURCE_MAP) || (cRecipeOneFlow.m_iDataSources == FLOW_SOURCE_MAP_STDF))
                strList += cRecipeOneFlow.m_strFlow;	// This flow is using the iDataSource
           break;
        case FLOW_SOURCE_STDF:
           if((cRecipeOneFlow.m_iDataSources == FLOW_SOURCE_STDF) || (cRecipeOneFlow.m_iDataSources == FLOW_SOURCE_MAP_STDF))
                strList += cRecipeOneFlow.m_strFlow;	// This flow is using the iDataSource
           break;
       }
    }

    // If NO flow found, add "NONE" to the list
    if(strList.count() == 0)
    {
        strList += "NONE";
        return false;	// NO Flow for this iDataSource
    }
    else
        return true;	// At least one flow using iDataSource
}

//===============================================================================================
// Returns string with list of flows + details
//===============================================================================================
QString	CPROD_Recipe::strFlowListDetails(QString strSeparator/*=" ; "*/)
{
    // Enumerate list of flows
    CRecipeFlow cRecipeOneFlow;
    QString strString = "";
    int		iEntry;
    for(iEntry=0;iEntry<m_cFlows.count();iEntry++)
    {
        cRecipeOneFlow = m_cFlows.at(iEntry);
        if(strString.isEmpty() == false)
            strString += strSeparator;
        strString += cRecipeOneFlow.m_strFlow;	// Flow name
        strString += ":" + cRecipeOneFlow.m_strENG_RecipeFile;	// Recipe file name
#if 0
        strString += ":" + cRecipeOneFlow.m_strENG_RecipeName;	// Recipe name
        strString += " V" + cRecipeOneFlow.m_strENG_Version;// Recipe version
#endif

        strString += " (" + cRecipeOneFlow.strSourceName() + ")";	// Data Source:OLI,STDF,Both
    }

    return strString;
}

//===============================================================================================
// Loads PPAT/GPAT & ZPAT bins
//===============================================================================================
void	CPROD_Recipe::setPatBins(int iPpat,int iGpat,int iZpat)
{
    m_iPPAT_Bin = iPpat;
    m_iGPAT_Bin = iGpat;
    m_iZPAT_Bin = iZpat;
}

//===============================================================================================
// Returns string with STDF output mode
//===============================================================================================
QString	CPROD_Recipe::strStdfOutputMode(void)
{
    switch(m_iStdfOutput)
    {
        default:
        case 0: return "Disabled";
        case 1: return "Wafmap";
        case 2: return "Full";
    }
}

#if 0
//===============================================================================================
// Get Production version for given product
//===============================================================================================
int PatProdRecipe::getProductionRecipeVersion(QString strProduct)
{
    // Open .ini settings file.
    QSettings settings(m_strRecipesIni,QSettings::IniFormat);
    QString strProductEntry = strProduct + "/prod_version";
    return settings.value(strProductEntry, 0).toInt();
}

//===============================================================================================
// Set Production version for given product
//===============================================================================================
void PatProdRecipe::setProductionRecipeVersion(QString strProduct,int Version)
{
    // Open .ini settings file.
    QSettings settings(m_strRecipesIni,QSettings::IniFormat);
    QString strProductEntry = strProduct + "/prod_version";
    settings.setValue(strProductEntry,Version);
}

#endif

