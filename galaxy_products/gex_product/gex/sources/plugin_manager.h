///////////////////////////////////////////////////////////
// Plug-in Manager. 
// Interface layer between Examinator and 
// third-party applications
///////////////////////////////////////////////////////////

#ifndef GEX_PLUGIN_MGR_H
#define GEX_PLUGIN_MGR_H
#include <stdio.h>

#include "plugin_base.h"
#include "plugin_constants.h"

///////////////////////////////////////////////////////////
// Plugin type to load
///////////////////////////////////////////////////////////
#define GEX_PLUGINTYPE_DLL			0	// Check for custom dll in the plugin directory
#define GEX_PLUGINTYPE_YIELD123		1	// Use Yield-123 plugin

///////////////////////////////////////////////////////////
// Plugin manager class.
///////////////////////////////////////////////////////////
class CExaminatorPluginManager
{
public:
	CExaminatorPluginManager();
	~CExaminatorPluginManager();
	bool		LoadPlugin(int nPluginType);						// Load plugin. nPluginType specifies if we must load a specific plugin, or scan the plugin folder, and load 1st plugin DLL found
	QString		GetError(void) const { return m_strLastErrorMsg; }	// Returns message for last error
	QString		GetExecCommand(void);								// Returns command string of next plugin command to be executed 
	int			BuildGEXReport(void);								// Return 1 if standard GEX report should be created, 0 else
	bool		HTMLOutputOnly(void);								// Return true if only HTML format should be supported by plug-in, false else
	void		WriteScriptPluginOptions(FILE *hFile);

	// Dispatcher functions to relevant plugin. Some functions may be specific to a plugin, and may
	// not be implemented in all plugins (ie Train() is specific to the MASA MDE plugin).

	// Common plug-in functions (specialized for each plug-in)
	void	eventBeginData(void) const;												// Beginning of ALL datasets
	void	eventBeginDataset(const GP_DatasetDef & DatasetDef) const;
	void	eventDefineParameter(const GP_ParameterDef & ParameterDefinition) const;	// Called one time per parameter definition
	void	eventLot(const GP_LotDef & LotDefinition, const GP_SiteDescription& clGlobalEquipmentID, const GP_SiteDescriptionMap* pSiteEquipmentIDMap, bool bIgnoreThisSubLot = false) const;	// Lot, sub-lot (or waferID if wafer test) change notification
	void	eventParameterResult(const GP_DataResult & DataResult) const;				// Data result
	void	eventPartResult(const GP_PartResult & PartResult) const;					// Result for 1 part (binning, x, y...)
	void	eventEndDataset(void) const;												// End of current Dataset
	void	eventEndData(void) const;													// End of ALL Datasets
	void	eventSetCommand(const QString & strCommand);								// Set next Plugin command
	void	eventSetSetting(const QString & strSetting);								// Set plugin setting
	void	eventExecCommand(void);														// Execute Plugin command
	void	eventAbort(void) const;														// Abort current action.
	void	eventAction(const QString & strAction) const;								// Action notification: Button / Hyperlink clicked.
	void	eventInteractiveCmd(GP_InteractiveCmd & InteractiveCmd) const;				// Event notification: command from interactive module
	void	eventGenerateReport(void) const;											// Generate plugin report.

	// Plugin details
	long	lPluginModuleID;	// Unique plugin module ID: Specified and maintained by Galaxy. (GEX_PLUGIN_MODULE_xxxxx)
	QString strPluginName;		// Plugin name + version string, returned from third-party plugin DLL
	QString	strHomePage;		// HTML home page name.

private:
	void SetError(int iErrorCode);

private:
    Q_DISABLE_COPY(CExaminatorPluginManager)     // please define correctly = operator and copy constructor if necessary
	CExaminatorBasePlugin *m_pPlugin;	// Plugin loaded.
	QString m_strPluginDirectory;		// Directory where plug-ins should be located
	QString m_strLastErrorMsg;			// Last error message

	// Error codes
	enum ErrorCode 
	{
		eNoError,					// No error
		eInvalidPluginID,			// Invalid PluginID returned by the third-party Plugin DLL
		eNoPlugin,					// No plug-in found
		eInitPlugin					// Error initializing the plug-in
	};
};


#endif

