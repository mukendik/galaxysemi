///////////////////////////////////////////////////////////
// Plug-in Manager.
// Interface layer between Examinator and
// third-party applications
///////////////////////////////////////////////////////////

// QT includes
#include <qfile.h>

#include "engine.h"
#include "plugin_manager.h"
#include "plugin_constants.h"

///////////////////////////////////////////////////////////
// Contructor
///////////////////////////////////////////////////////////
CExaminatorPluginManager::CExaminatorPluginManager()
{
    // Handle to plugin loaded
    m_pPlugin = NULL;

    // Reset Error message
    m_strLastErrorMsg = "";

    // Init plug-ins directory
    // Plugins are in "<Gex.exe folder>/plugins/"
    m_strPluginDirectory = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    m_strPluginDirectory += "/plugins/";
    lPluginModuleID = 0;
    strPluginName = QString("");
    strHomePage = QString("");
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CExaminatorPluginManager::~CExaminatorPluginManager()
{
    // Call Plugin destructor
    if(m_pPlugin != NULL)
    {
        delete m_pPlugin;
        m_pPlugin = NULL;
    }
}

///////////////////////////////////////////////////////////
// Load plugin. nPluginType specifies if we must load a specific plugin,
// or scan the plugin folder, and load 1st plugin DLL found
///////////////////////////////////////////////////////////
bool CExaminatorPluginManager::LoadPlugin(int nPluginType)
{
    QString strPluginFullPath;	// Plugin path+name to load
#if defined unix
    QString	strExtension=".so";	// Library extension under Unix
#elif defined __MACH__
    QString	strExtension=".dylib";	// Library extension under Unix
#else
    QString	strExtension=".dll";// Library extension under Windows
#endif

    // Reset Error code
    SetError(eNoError);

    // Check if specific Plugin should be loaded
    switch(nPluginType)
    {
        case GEX_PLUGINTYPE_YIELD123:	// Load Yield-123 plugin
//			strPluginFullPath = m_strPluginDirectory + "yield123";
//			strPluginFullPath += strExtension;
//			m_pPlugin = (CExaminatorBasePlugin *) new CExaminatorPlugin_Yield123(strPluginFullPath, m_strPluginDirectory);
            break;

        case GEX_PLUGINTYPE_DLL:
            // Check if MASA MDE Plugin is present...<Gex.exe folder>/plugins/mdeglxy.dll or .so
//			strPluginFullPath = m_strPluginDirectory + "mdeglxy";
//			strPluginFullPath += strExtension;
//			if(QFile::exists(strPluginFullPath) == true)
//				m_pPlugin = (CExaminatorBasePlugin *) new CExaminatorPlugin_MasaMDE(strPluginFullPath, m_strPluginDirectory);
            break;

        default:
            break;
    }

    // No plugin found
    if(m_pPlugin == NULL)
    {
        SetError(eNoPlugin);
        return false;
    }

    // Init plugin
    if(m_pPlugin->Init() == false)
    {
        SetError(eInitPlugin);
        return false;
    }

    // Get plugin info
    lPluginModuleID = m_pPlugin->GetPluginID();	// Plugin ID
    strPluginName = m_pPlugin->GetPluginName();	// Plugin Name
    strHomePage = m_pPlugin->GetHomePage();		// Plugin-specific Home Page

    // Check Plugin ID!
    switch(lPluginModuleID)
    {
        case GEX_PLUGINID_MASAMDE:	// MASA MDE Plugin
            break;

        case GEX_PLUGINID_YIELD123:	// YIELD-123 Plugin
            break;

        default:	// Invalid plugin ID
            SetError(eInvalidPluginID);
            return false;
    }

    return true;	// Success
}

///////////////////////////////////////////////////////////
// Returns message for last error
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::SetError(int iErrorCode)
{
    switch(iErrorCode)
    {
        case eNoError:
            m_strLastErrorMsg = "No Error";
            break;
        case eInvalidPluginID:
            m_strLastErrorMsg.sprintf("Plug-in library (%s) returned an invalid Plug-in ID (%ld)", strPluginName.toLatin1().constData(), lPluginModuleID);
            break;
        case eNoPlugin:
            m_strLastErrorMsg.sprintf("No plug-in found in plug-in directory: %s", m_strPluginDirectory.toLatin1().constData());
            break;
        case eInitPlugin:
            m_strLastErrorMsg = GGET_LASTERRORMSG(CExaminatorBasePlugin, m_pPlugin);
            break;
    }
}

///////////////////////////////////////////////////////////
// Get command string of next command that the plugin should
// execute (next call to eventExecCommand())
///////////////////////////////////////////////////////////
QString CExaminatorPluginManager::GetExecCommand(void)
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return "";

    return m_pPlugin->GetPluginCommand();
}

///////////////////////////////////////////////////////////
// Check if standard GEX report should be created
// return 0: report should not be built
// return 1: report should be built
///////////////////////////////////////////////////////////
int CExaminatorPluginManager::BuildGEXReport(void)
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return 1;

    return m_pPlugin->BuildGEXReport();
}

///////////////////////////////////////////////////////////
// Check if only HTML format supported
// return true: only HTML format is supported
// return false: other formats should be supported (Word...)
///////////////////////////////////////////////////////////
bool CExaminatorPluginManager::HTMLOutputOnly(void)
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return true;

    return m_pPlugin->HTMLOutputOnly();
}

///////////////////////////////////////////////////////////
// Write 'SetPluginOptions' section in the script file
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::WriteScriptPluginOptions(FILE *hFile)
{
    if(hFile == NULL)
        return;

    // Writes section
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"// Plugin options\n");
    fprintf(hFile,"//////////////////////////////////////////\n");
    fprintf(hFile,"SetPluginOptions()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"\n");
    fprintf(hFile,"  // Sets all data analysis global options\n");
    fprintf(hFile,"  gexOptions('report','build','%s');\n", (BuildGEXReport() == 0) ? "false" : "true");

    // Message to console saying 'success loading options'
    fprintf(hFile,"\n\n  sysLog('* Plugin Options loaded! *');\n");

    // Close function
    fprintf(hFile,"}\n");
    fprintf(hFile,"\n");
}

///////////////////////////////////////////////////////////
// Common plugin functions (specialized for each plug-in)
///////////////////////////////////////////////////////////

////////////////////////////////////////////////////
// 'SetCommand' event: set next command to be executed
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventSetCommand(const QString & strCommand)
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventSetCommand(strCommand);
}

////////////////////////////////////////////////////
// 'SetSetting' event: set plugin setting
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventSetSetting(const QString & strSetting)
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventSetSetting(strSetting);
}

///////////////////////////////////////////////////////////
// Dispatch 'ExecCommand' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventExecCommand(void)
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventExecCommand();
}

///////////////////////////////////////////////////////////
// Dispatch 'BeginData' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventBeginData(void) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventBeginData();
}

///////////////////////////////////////////////////////////
// Dispatch 'BeginDataset' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventBeginDataset(const GP_DatasetDef & DatasetDef) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventBeginDataset(DatasetDef);
}

///////////////////////////////////////////////////////////
// Dispatch 'DefineParameter' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventDefineParameter(const GP_ParameterDef & ParameterDefinition) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventDefineParameter(ParameterDefinition);
}

///////////////////////////////////////////////////////////
// Dispatch 'Lot' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventLot(const GP_LotDef & LotDefinition, const GP_SiteDescription& clGlobalEquipmentID, const GP_SiteDescriptionMap* pSiteEquipmentIDMap, bool bIgnoreThisSubLot/* = false*/) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventLot(LotDefinition, clGlobalEquipmentID, pSiteEquipmentIDMap, bIgnoreThisSubLot);
}

///////////////////////////////////////////////////////////
// Dispatch 'ParameterResult' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventParameterResult(const GP_DataResult & DataResult) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventParameterResult(DataResult);
}

///////////////////////////////////////////////////////////
// Dispatch 'PartResult' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventPartResult(const GP_PartResult & PartResult) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventPartResult(PartResult);
}

///////////////////////////////////////////////////////////
// Dispatch 'EndDataset' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventEndDataset(void) const	// End of current Dataset
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventEndDataset();
}

///////////////////////////////////////////////////////////
// Dispatch 'EndData' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventEndData(void) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventEndData();
}

///////////////////////////////////////////////////////////
// Dispatch 'GenerateReport' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventGenerateReport(void) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventGenerateReport();
}

///////////////////////////////////////////////////////////
// Dispatch 'Abort' event notification to relevant plugin
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventAbort(void) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventAbort();
}

///////////////////////////////////////////////////////////
// Examinator Hyperlink clicked.
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventAction(const QString & strAction) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventAction(strAction);
}

///////////////////////////////////////////////////////////
// Command from interactive module.
///////////////////////////////////////////////////////////
void CExaminatorPluginManager::eventInteractiveCmd(GP_InteractiveCmd & InteractiveCmd) const
{
    // Check if any Plugin loaded.
    if(m_pPlugin == NULL)
        return;

    m_pPlugin->eventInteractiveCmd(InteractiveCmd);
}
