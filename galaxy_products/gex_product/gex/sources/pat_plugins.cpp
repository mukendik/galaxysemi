///////////////////////////////////////////////////////////
// Pat plugins: access functions. Interface to Gex PAT
// plugins
///////////////////////////////////////////////////////////

// Standard includes

// Qt includes
#include <qapplication.h>
#include <qdir.h>
#include <qlibrary.h>
#include <qcursor.h>

// Galaxy modules includes
#include <gqtl_sysutils.h>

// Local includes
#include "engine.h"
#include "pat_plugins.h"
#include "pat_info.h"

//////////////////////////////////////////////////////////////////////
// GexPatPlugin_ID: object holding information on a GexPAT plugin
//////////////////////////////////////////////////////////////////////
GexPatPlugin_ID::GexPatPlugin_ID()
{
    m_pPlugin = NULL;
    m_pLibrary = NULL;
}

GexPatPlugin_ID::~GexPatPlugin_ID()
{
    FP_gexpat_plugin_releaseobject Lib_gexpat_plugin_releaseobject;
    if(m_pLibrary && m_pPlugin)
    {
        Lib_gexpat_plugin_releaseobject = (FP_gexpat_plugin_releaseobject) m_pLibrary->resolve(GEXPAT_PLUGIN_RELEASEOBJECT_NAME);
        if(Lib_gexpat_plugin_releaseobject)
            Lib_gexpat_plugin_releaseobject(m_pPlugin);
        delete m_pLibrary;
        m_pPlugin = NULL;
        m_pLibrary = NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// GexExternalPat
//////////////////////////////////////////////////////////////////////
// Error map
GBEGIN_ERROR_MAP(GexExternalPat)
    GMAP_ERROR(eLoadPlugin, "Failed loading plugin from plugin directory (%s):\nFile  = %s")
    GMAP_ERROR(eNoPluginLoaded, "No plugin loaded.")
    GMAP_ERROR(eMissingPlugin_2, "Following plugin was not found in the plugin directory (%s):\nFile  = %s")
    GMAP_ERROR(eMissingPlugin_3, "Following plugin was not found in the plugin directory (%s):\nName  = %s\nBuild = %d")
    GMAP_ERROR(ePlugin, "Error in plugin %s")
GEND_ERROR_MAP(GexExternalPat)

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexExternalPat::GexExternalPat()
{
    //CGexSystemUtils	clSysUtils;

    // Handle to plugin loaded
    m_pPluginID = NULL;

    // Init GexPat plug-ins directory
    // Plugins are in "<Gex.exe folder>/plugins/pat"
    m_strPluginDirectory = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/plugins/pat";
    CGexSystemUtils::NormalizePath(m_strPluginDirectory);
}

GexExternalPat::~GexExternalPat()
{
    // Call Plugin destructor if a particular plugin is loaded
    if(m_pPluginID != NULL)
    {
        delete m_pPluginID;
        m_pPluginID = NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// Load available GexPat plugins
//////////////////////////////////////////////////////////////////////
void GexExternalPat::LoadPlugins(QList<GexPatPlugin_ID*> & pPluginList)
{
    QDir						clDir;
    QStringList					pList;
    QStringList::Iterator		it;
    QLibrary					*pclLibrary;
    FP_gexpat_plugin_getobject	Lib_gexpat_plugin_getobject;
    GexPatPlugin_Base			*pPlugin;
    GexPatPlugin_ID				*pPluginID;
    //CGexSystemUtils				clSysUtils;

    // First unload plugins
    UnloadPlugins(pPluginList);

    // Get list of library files (*.dll, *.so) in plugin directory
    clDir.setPath(m_strPluginDirectory);
#if defined unix
    pList = clDir.entryList(QDir::nameFiltersFromString("*.so"), QDir::Files, QDir::Unsorted);
#elif defined __MACH__
    pList = clDir.entryList(QDir::nameFiltersFromString("*.dylib"), QDir::Files, QDir::Unsorted);
#else
    pList = clDir.entryList(QDir::nameFiltersFromString("*.dll"), QDir::Files, QDir::Unsorted);
#endif

    // Scan list: check if plugin can be loaded, and get its name and version
    QString strFile;
    for(it = pList.begin(); it != pList.end(); ++it )
    {
        // Load library
        strFile = m_strPluginDirectory + "/";
        strFile += (*it);
        CGexSystemUtils::NormalizePath(strFile);
        pclLibrary = new QLibrary(strFile);
        if(pclLibrary)
        {
            // Retrieve ptr on gexpat_plugin_getobject() function
            Lib_gexpat_plugin_getobject = (FP_gexpat_plugin_getobject) pclLibrary->resolve(GEXPAT_PLUGIN_GETOBJECT_NAME);
            if(Lib_gexpat_plugin_getobject)
            {
                // Create plugin base object
                pPlugin = Lib_gexpat_plugin_getobject(
                    GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString().toLatin1().constData(),
                    GS::Gex::Engine::GetInstance().Get("UserFolder").toString().toLatin1().constData());
                if(pPlugin)
                {
                    // Get plugin identification
                    pPluginID = new GexPatPlugin_ID;
                    pPluginID->m_pLibrary = pclLibrary;
                    pPluginID->m_pPlugin = pPlugin;
                    pPluginID->m_strFullFileName = strFile;
                    pPluginID->m_strFileName = (*it);
                    pPluginID->m_strPluginName = pPlugin->GetPluginName();
                    pPluginID->m_uiPluginBuild = pPlugin->GetPluginBuild();
                    pPluginList.append(pPluginID);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Unload available GexPat plugins
//////////////////////////////////////////////////////////////////////
void GexExternalPat::UnloadPlugins(QList<GexPatPlugin_ID*> & pPluginList)
{
    while (!pPluginList.isEmpty())
        delete pPluginList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// If required, update extension of plugin file read from settings
// (.dll, .so)
//////////////////////////////////////////////////////////////////////
void GexExternalPat::UpdatePluginExtension(QString &strFileName)
{
    QFileInfo		clFileInfo(strFileName);
    //CGexSystemUtils	clSysUtils;

#if defined unix || __MACH__
    if(clFileInfo.suffix().toLower() == "dll")
    {
        strFileName.truncate(strFileName.length()-3);
        strFileName += "so";
    }
#else
    if(clFileInfo.suffix().toLower() == "so")
    {
        strFileName.truncate(strFileName.length()-2);
        strFileName += "dll";
    }
#endif

    CGexSystemUtils::NormalizePath(strFileName);
}

//////////////////////////////////////////////////////////////////////
// Load specified plugin
//////////////////////////////////////////////////////////////////////
bool GexExternalPat::LoadPlugin(QString & strPluginFileName)
{
    QString strPluginFullFileName;

    // Free current plugin if loaded
    if(m_pPluginID != NULL)
    {
        delete m_pPluginID;
        m_pPluginID = NULL;
    }

    // Try to load plugin
    GexPatPlugin_Base			*pPlugin=0;
    QLibrary					*pclLibrary;
    FP_gexpat_plugin_getobject	Lib_gexpat_plugin_getobject;
    UpdatePluginExtension(strPluginFileName);

    // Create full filename and load library
    //CGexSystemUtils	clSysUtils;
    strPluginFullFileName = m_strPluginDirectory + "/";
    strPluginFullFileName += strPluginFileName;
    CGexSystemUtils::NormalizePath(strPluginFullFileName);
    pclLibrary = new QLibrary(strPluginFullFileName);
    if(!pclLibrary)
    {
        GSET_ERROR2(GexExternalPat, eMissingPlugin_2, NULL, m_strPluginDirectory.toLatin1().constData(), strPluginFileName.toLatin1().constData());
        return false;
    }

    // Retrieve ptr on gexpat_plugin_getobject() function
    Lib_gexpat_plugin_getobject = (FP_gexpat_plugin_getobject) pclLibrary->resolve(GEXPAT_PLUGIN_GETOBJECT_NAME);
    if(!Lib_gexpat_plugin_getobject)
    {
        GSET_ERROR2(GexExternalPat, eLoadPlugin, NULL, m_strPluginDirectory.toLatin1().constData(), strPluginFileName.toLatin1().constData());
        delete pclLibrary; pclLibrary=0;
        return false;
    }

    // Create plugin base object
    pPlugin = Lib_gexpat_plugin_getobject(GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString().toLatin1().constData(),
                                          GS::Gex::Engine::GetInstance().Get("UserFolder").toString().toLatin1().constData());
    if(!pPlugin)
    {
        GSET_ERROR2(GexExternalPat, eLoadPlugin, NULL, m_strPluginDirectory.toLatin1().constData(), strPluginFileName.toLatin1().constData());
        delete pclLibrary; pclLibrary=0;
        return false;
    }

    // Get plugin identification
    m_pPluginID = new GexPatPlugin_ID;
    m_pPluginID->m_pLibrary = pclLibrary;
    m_pPluginID->m_pPlugin = pPlugin;
    m_pPluginID->m_strFullFileName = strPluginFullFileName;
    m_pPluginID->m_strFileName = strPluginFileName;
    m_pPluginID->m_strPluginName = pPlugin->GetPluginName();
    m_pPluginID->m_uiPluginBuild = pPlugin->GetPluginBuild();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Load specified plugin
//////////////////////////////////////////////////////////////////////
bool GexExternalPat::LoadPlugin(const QString & strPluginName, unsigned int uiPluginBuild)
{
    QList<GexPatPlugin_ID*>	pPluginList;
    QString					strPluginFullFileName;

    // Load available plugins
    LoadPlugins(pPluginList);

    // Check if specified plugin available
    GexPatPlugin_ID *					pPluginID	= NULL;
    QList<GexPatPlugin_ID*>::iterator	itPlugin	= pPluginList.begin();

    while(itPlugin != pPluginList.end())
    {
        if(((*itPlugin)->m_uiPluginBuild == uiPluginBuild) && ((*itPlugin)->m_strPluginName == strPluginName))
        {
            pPluginID = (*itPlugin);
            break;
        }

        itPlugin++;
    }

    if(!pPluginID)
    {
        UnloadPlugins(pPluginList);
        GSET_ERROR3(GexExternalPat, eMissingPlugin_3, NULL, m_strPluginDirectory.toLatin1().constData(), strPluginName.toLatin1().constData(), uiPluginBuild);
        return false;	// Plugin not found.
    }

    // Load the plugin
    bool bStatus = LoadPlugin(pPluginID->m_strFileName);
    UnloadPlugins(pPluginList);

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Apply PAT on specified wafermap
//////////////////////////////////////////////////////////////////////
bool GexExternalPat::pat_wafermap(CWaferMap *pGexWaferMap, CPatInfo	*patInfo)
{
    if(!m_pPluginID)
    {
        GSET_ERROR0(GexExternalPat, eNoPluginLoaded, NULL);
        return false;
    }

    // Fill Plugin WaferMap object
    GexPatPlugin_WaferMap clPluginWaferMap(pGexWaferMap->SizeX, pGexWaferMap->SizeY);
    clPluginWaferMap.m_nHighDieX = pGexWaferMap->iHighDieX;
    clPluginWaferMap.m_nHighDieY = pGexWaferMap->iHighDieY;
    clPluginWaferMap.m_nLowDieX = pGexWaferMap->iLowDieX;
    clPluginWaferMap.m_nLowDieY = pGexWaferMap->iLowDieY;
    clPluginWaferMap.m_nSizeX = pGexWaferMap->SizeX;
    clPluginWaferMap.m_nSizeY = pGexWaferMap->SizeY;
    int nIndex;
    for(nIndex=0; nIndex<(pGexWaferMap->SizeX*pGexWaferMap->SizeY); nIndex++)
        clPluginWaferMap.m_pnBinnings[nIndex] = pGexWaferMap->getWafMap()[nIndex].getBin();

    // Call plugin wafermap function
    if(m_pPluginID->m_pPlugin->pat_wafermap(&clPluginWaferMap, patInfo->GetRecipeOptions().mGDBNPatSBin, patInfo->GetRecipeOptions().iFailDynamic_SBin) == GexPatPlugin_Base::eNoError)
    {
        // Update Gex WaferMap
        int                 nX, nY;
        QString             lKey;
        CPatDieCoordinates  lGPATOutlier;
        for(nIndex = 0; nIndex < (pGexWaferMap->SizeX*pGexWaferMap->SizeY); nIndex++)
        {
            if(clPluginWaferMap.m_pnBinnings[nIndex] == patInfo->GetRecipeOptions().mGDBNPatSBin)
            {
                // Compute die location
                pGexWaferMap->coordFromIndex(nIndex, nX, nY);

                // This good bin passes the conditions to be failed (enough bad neighbors, and squeezed between failures)
                lKey = QString::number(nX) + "." + QString::number(nY);
                lGPATOutlier.mDieX = nX;
                lGPATOutlier.mDieY = nY;

                // Save PAT definition in our list
                patInfo->mGDBNOutliers.insert(lKey, lGPATOutlier);	// Holds the Good Die X,Y position to fail
            }
        }

        return true;
    }

    GSET_ERROR0(GexExternalPat, ePlugin, NULL);
    return false;
}

//////////////////////////////////////////////////////////////////////
// Compute PAT limits for specified test
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexExternalPat::pat_testlimits(int nTestNumber, int nPinNumber, const char *szTestName, int nSiteNumber, const CTestResult& testResult, double *plfLowLimit, double *plfHighLimit)
{
    if(!m_pPluginID)
    {
        GSET_ERROR0(GexExternalPat, eNoPluginLoaded, NULL);
        return false;
    }

    // Allocate an array of double to give the sample results to the plugin
    double * pTestResults = NULL;

    if (testResult.count() > 0)
    {
        pTestResults = new double[testResult.count()];

        // fill the array with the sample results
        for (int nCount = 0; nCount < testResult.count(); ++nCount)
            pTestResults[nCount] = testResult.resultAt(nCount);
    }

    // Call plugin test limits computation function
    int nError = m_pPluginID->m_pPlugin->pat_testlimits(nTestNumber, nPinNumber, szTestName, nSiteNumber, pTestResults, testResult.count(), plfLowLimit, plfHighLimit);

    // Free the memory
    if (pTestResults)
    {
        delete pTestResults;
        pTestResults = NULL;
    }

    if (nError == GexPatPlugin_Base::eNoError)
        return true;

    GSET_ERROR0(GexExternalPat, ePlugin, NULL);
    return false;
}

//////////////////////////////////////////////////////////////////////
// Returns details about last error
//////////////////////////////////////////////////////////////////////
void GexExternalPat::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(GexExternalPat,this);
}
