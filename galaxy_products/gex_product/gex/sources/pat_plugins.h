///////////////////////////////////////////////////////////
// Pat plugins: access functions.
///////////////////////////////////////////////////////////

#ifndef GEX_PAT_PLUGINS_H
#define GEX_PAT_PLUGINS_H

// Standard includes

// QT includes 
#include <qstringlist.h>

// Galaxy modules includes
#include <gstdl_errormgr.h>

// Local includes
#include "gexpat_plugin_base.h"
#include "report_build.h"		// For WaferMap structure

// Forward declarations
class QLibrary;
class CPatInfo;

///////////////////////////////////////////////////////////
// GexPatPlugin_ID class: plugin identification...
///////////////////////////////////////////////////////////
class GexPatPlugin_ID
{
public:
	GexPatPlugin_ID();
	~GexPatPlugin_ID();

	QLibrary			*m_pLibrary;
	GexPatPlugin_Base	*m_pPlugin;
	QString				m_strFullFileName;
	QString				m_strFileName;
	QString				m_strPluginName;
	unsigned int		m_uiPluginBuild;
};

// External PAT plugins management
class GexExternalPat
{
public:
	GDECLARE_ERROR_MAP(GexExternalPat)
	{
		eLoadPlugin,				// Error loading plugin
		eNoPluginLoaded,			// No plugin loaded
		eMissingPlugin_2,			// Specified plugin not found in plgugin directory
		eMissingPlugin_3,			// Specified plugin not found in plgugin directory
		ePlugin						// Error in plugin function
	}
	GDECLARE_END_ERROR_MAP(GexExternalPat)

	// Constructor / destructor functions
	GexExternalPat();			// Consturctor
	~GexExternalPat();			// Destructor

	// Common functions
	QString			GetPluginDirectory(void) { return m_strPluginDirectory;	}
	void			LoadPlugins(QList<GexPatPlugin_ID*> & pPluginList);
	void			UnloadPlugins(QList<GexPatPlugin_ID*> & pPluginList);
	void			GetLastError(QString & strError);
	bool			LoadPlugin(QString & strPluginFileName);									// Load specified plugin 
	bool			LoadPlugin(const QString & strPluginName, unsigned int uiPluginBuild);		// Load specified plugin 

	// PAT functions
    bool			pat_wafermap(CWaferMap *pGexWaferMap, CPatInfo	*patInfo);					// Apply PAT to specified wafermap
	bool			pat_testlimits(int nTestNumber, int nPinNumber, const char *szTestName, int nSiteNumber, const CTestResult& testResult, double *plfLowLimit, double *plfHighLimit);	// Compute PAT limits for specified test

private:
	// Functions
	void			UpdatePluginExtension(QString &strFileName);

	// Variables
	GexPatPlugin_ID				*m_pPluginID;				// Ptr to loaded plugin
	QString						m_strPluginDirectory;		// Directory where plug-ins should be located
};

#endif // GEX_PAT_PLUGINS_H
