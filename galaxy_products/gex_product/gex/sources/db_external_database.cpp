///////////////////////////////////////////////////////////
// External Database: Access functions. Interface to GexDb
// plugins
///////////////////////////////////////////////////////////

// Standard includes

// Qt includes
#include <qapplication.h>
#include <qmainwindow.h>
#include <qdir.h>
#include <qlibrary.h>
#include <qcursor.h>
#include <QFileInfo>
#include <QString>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gex_scriptengine.h>

// Local includes
#include "gex_shared.h"
#include "db_external_database.h"
#include "browser_dialog.h"
#include "engine.h"
#include "report_build.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "command_line_options.h"
#include "read_system_info.h"

// main.cpp
extern void                 WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow        *pGexMainWindow;
extern GexScriptEngine*     pGexScriptEngine;

// gex_constants.cpp
extern const char       *gexLabelFilterChoices[];

// report_build.cpp
extern CReportOptions   ReportOptions;      // Holds options (report_build.h)

extern CGexSkin*        pGexSkin;           // holds the skin settings

//////////////////////////////////////////////////////////////////////
// GexDbPlugin_ID: object holding information on a GexDB plugin
//////////////////////////////////////////////////////////////////////
GexDbPlugin_ID::GexDbPlugin_ID()
{
    m_pPlugin = NULL;
    m_pLibrary = NULL;
    m_bPluginConfigured = false;
}

GexDbPlugin_ID::~GexDbPlugin_ID()
{
    FP_gexdb_plugin_releaseobject Lib_gexdb_plugin_releaseobject;
    if(m_pLibrary && m_pPlugin)
    {
        Lib_gexdb_plugin_releaseobject = (FP_gexdb_plugin_releaseobject) m_pLibrary->resolve(GEXDB_PLUGIN_RELEASEOBJECT_NAME);
        if(Lib_gexdb_plugin_releaseobject)
            Lib_gexdb_plugin_releaseobject(m_pPlugin);
        delete m_pLibrary;
        m_pPlugin = NULL;
        m_pLibrary = NULL;
    }
}



bool GexDbPlugin_ID::LoadPlugin(QString &pluginFilePath, QMainWindow *pMainWindow)
{
    // Try to load plugin
    GexDbPlugin_Base            *pPlugin = NULL;
    QLibrary                    *pclLibrary = NULL;
    FP_gexdb_plugin_getobject   Lib_gexdb_plugin_getobject;

    GSLOG(SYSLOG_SEV_DEBUG, QString("Load plugin '%1'...")
          .arg(pluginFilePath)
          .toLatin1().constData());

    CGexSystemUtils::NormalizePath(pluginFilePath);	// CGexSystemUtils	clSysUtils;
    pclLibrary = new QLibrary(pluginFilePath);
    if(!pclLibrary)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cant create QLibrary for '%1' !")
              .arg( pluginFilePath.toLatin1().constData())
              .toLatin1().constData());
        return false;
    }

    if (!pclLibrary->load())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cannot load %1").arg( pluginFilePath).toLatin1().constData() );
        GSLOG(SYSLOG_SEV_ERROR, pclLibrary->errorString().toLatin1().data() );
        if (!QFile::exists(pluginFilePath))
            GSLOG(SYSLOG_SEV_ERROR, QString("File does not exist: %1").arg(pluginFilePath).toLatin1().data() );
        delete pclLibrary; pclLibrary=0;
        return false;
    }

    // Retrieve ptr on gexdb_plugin_getobject() function
    Lib_gexdb_plugin_getobject = (FP_gexdb_plugin_getobject) pclLibrary->resolve(GEXDB_PLUGIN_GETOBJECT_NAME);
    if(!Lib_gexdb_plugin_getobject)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("cannot resolve function '%1' !")
              .arg( GEXDB_PLUGIN_GETOBJECT_NAME)
              .toLatin1().constData());
        delete pclLibrary; pclLibrary=0;
        return false;
    }

    // Create plugin base object
    pPlugin = Lib_gexdb_plugin_getobject(GS::Gex::Engine::GetInstance().GetSystemInfo().strHostName,
                                         GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString(),
                                         "",
                                         GS::Gex::Engine::GetInstance().Get("UserFolder").toString(),
                                         gexLabelFilterChoices, pGexSkin,
                                         pGexScriptEngine,
                                         GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsCustomDebugMode());
    if(!pPlugin)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("failed to getobject for %1 !")
              .arg( pluginFilePath.toLatin1().constData())
              .toLatin1().constData());
        delete pclLibrary; pclLibrary=0;
        return false;
    }

    pPlugin->SetParentWidget(pMainWindow);
    // Init
    if (!pPlugin->Init())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to init plugin '%1' !")
              .arg( GEXDB_PLUGIN_GETOBJECT_NAME)
              .toLatin1().constData());
        return false;
    }

    m_pLibrary = pclLibrary;
    m_pPlugin = pPlugin;
    m_strFilePath = pluginFilePath;
    m_strFileName = QFileInfo(pluginFilePath).fileName();

    //GSLOG(SYSLOG_SEV_DEBUG, QString("Plugin loaded.") );
    return true;
}

QString GexDbPlugin_ID::pluginName()
{
    QString pluginName = "";
    if (m_pPlugin)
        m_pPlugin->GetPluginName(pluginName);

    return pluginName;
}

QString GexDbPlugin_ID::pluginFileName()
{
    return m_strFileName;
}

QString GexDbPlugin_ID::pluginFilePath()
{
    return m_strFilePath;
}

unsigned int GexDbPlugin_ID::pluginBuild()
{
    unsigned int nPluginBuild = 0;
    if (m_pPlugin)
        nPluginBuild = m_pPlugin->GetPluginBuild();

    return nPluginBuild;
}

void GexDbPlugin_ID::setConfigured(bool value)
{
    m_bPluginConfigured = value;
}

bool GexDbPlugin_ID::isConfigured()
{
    return m_bPluginConfigured;
}

//////////////////////////////////////////////////////////////////////
// GexRemoteDatabase
//////////////////////////////////////////////////////////////////////
// Error map

GBEGIN_ERROR_MAP(GexRemoteDatabase)
GMAP_ERROR(eLoadPlugin, "Failed loading plugin %s.")
GMAP_ERROR(eOpenSettingsFile, "Failed opening database settings file %s.")
GMAP_ERROR(eMarkerNotFound,"Marker not found: %s.")
GMAP_ERROR(eReadField,"Failed reading field %s in section %s.")
GMAP_ERROR(eMissingPlugin, "Following plugin was not found in the plugin directory (%s):\nFile  = %s\nName  = %s\nBuild = %d")
GMAP_ERROR(ePluginSettings, "Plugin %s failed reading its settings.\nFile = %s")
GMAP_ERROR(eCreateSettingsFile, "Failed creating database settings file %s.\nDisk protection error?")
GMAP_ERROR(eNoPluginLoaded, "No plugin loaded.")
GMAP_ERROR(ePluginNotConfigured, "Plugin %s is not configured.")
GMAP_ERROR(ePlugin, "%s")
GEND_ERROR_MAP(GexRemoteDatabase)

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexRemoteDatabase::GexRemoteDatabase()
{
    // Ptr to Gex mainn window
    m_pMainWindow = pGexMainWindow;
    // Handle to plugin loaded
    m_pPluginID = NULL;
    // Init GexDb plug-ins directory
    // Plugins are in "<Gex.exe folder>/plugins/db"
    m_strPluginDirectory = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() + "/plugins/db";
    CGexSystemUtils::NormalizePath(m_strPluginDirectory);
    m_lSplitlotID = -1;
    m_nTestingStage = -1;
}

GexRemoteDatabase::~GexRemoteDatabase()
{
    // Call Plugin destructor if a particular plugin is loaded
    if(m_pPluginID != NULL)
    {
        delete m_pPluginID;
        m_pPluginID = NULL;
    }
}

void GexRemoteDatabase::StopProcess()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Try to Stop Process").toLatin1().constData() );
    if (m_pPluginID && m_pPluginID->m_pPlugin)
        m_pPluginID->m_pPlugin->StopProcess();
}


void GexRemoteDatabase::GetSupportedTestingStages(QStringList & strlSupportedTestingStages)
{
    strlSupportedTestingStages.clear();
    if(m_pPluginID)
    {
        QString supportedTestingStages;
        m_pPluginID->m_pPlugin->GetSupportedTestingStages(supportedTestingStages);
        strlSupportedTestingStages = supportedTestingStages.split(";");
    }
    else
        GSLOG(SYSLOG_SEV_INFORMATIONAL, " m_pPluginID NULL ");
}

bool GexRemoteDatabase::SetTestingStage(const QString & strTestingStage)
{
    if(m_pPluginID)
        return m_pPluginID->m_pPlugin->SetTestingStage(strTestingStage);

    return false;
}

int GexRemoteDatabase::GetTestingStageEnum(const QString & strTestingStage)
{
    if(m_pPluginID)
        return m_pPluginID->m_pPlugin->GetTestingStageEnum(strTestingStage);

    return 0;
}

//////////////////////////////////////////////////////////////////////
// Return supported binning types.
// ConsolidatedType: "Y" or "N"
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetSupportedBinTypes(const QString &TestingStage, QStringList & strlBinTypes,
                                             const QString &ConsolidatedType/*="Y"*/)
{
    strlBinTypes.clear();

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetSupportedBinTypes(TestingStage, strlBinTypes, ConsolidatedType);
    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Retrieve list of available fields for the given testing stage
// output will contains all the fields
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetRdbFieldsList(	const QString & TestingStage, QStringList & output,
                                            const QString &In_GUI,				// Y, N or *
                                            const QString &BinType,				// BinType can be H, S or N (or even *)
                                            const QString &Custom,				// Custom can be Y, N or *
                                            const QString &TimeType,			// TimeType can be Y, N or *
                                            const QString &ConsolidatedType,	// ConsolidatedType can be Y, N or *
                                            const QString &Facts,				// Facts can be Y, N, or *
                                            bool  OnlyStaticMetaData
                                            )
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" for stage '%1' Bin='%2' Conso='%3': %4")
          .arg(TestingStage.toLatin1().data())
          .arg(BinType.toLatin1().data())
          .arg(ConsolidatedType.toLatin1().data())
          .arg(m_pPluginID==NULL?" : error (plugin NULL).":":")
          .toLatin1().constData());
    output.clear();

    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->GetRdbFieldsList(TestingStage, output, In_GUI, BinType, Custom, TimeType, ConsolidatedType, Facts, OnlyStaticMetaData);
}

//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetRdbFieldProperties(const QString& TestingStage, const QString& MetaDataName, QMap<QString,QString>& Properties)
{
    Properties.clear();

    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->GetRdbFieldProperties(TestingStage, MetaDataName, Properties);
}

//////////////////////////////////////////////////////////////////////
// Get consolidated field name corresponding to specified decorated field name
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetConsolidatedFieldName(const QString & strTestingStage, const QString & strDecoratedField,
                                                 QString & strConsolidatedField, bool *pbIsNumeric /*= NULL*/,
                                                 bool * pbIsBinning /*= NULL*/, bool * pbIsTime /*= NULL*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetConsolidatedFieldName(strTestingStage, strDecoratedField, strConsolidatedField, pbIsNumeric, pbIsBinning, pbIsTime);
    if(!bStatus)
    {
        SetLastErrorFromPlugin();
        GSLOG(SYSLOG_SEV_WARNING, "plugin call to GetConsolidatedFieldName failed !");
    }

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return list of filters mapped by the plugin
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("GexRemoteDatabase::GetLabelFilterChoices on %1...")
          .arg(strDataTypeQuery).toLatin1().data() );
    strlLabelFilterChoices.clear();
    if(!m_pPluginID)
        return;
    m_pPluginID->m_pPlugin->GetLabelFilterChoices(strDataTypeQuery, strlLabelFilterChoices);
}

//////////////////////////////////////////////////////////////////////
// Return list of consolidated filters mapped by the plugin
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetConsolidatedLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("GexRemoteDatabase::GetLabelFilterChoices on %1...")
          .arg(strDataTypeQuery).toLatin1().data() );
    strlLabelFilterChoices.clear();
    if(!m_pPluginID)
        return;
    m_pPluginID->m_pPlugin->GetConsolidatedLabelFilterChoices(strDataTypeQuery, strlLabelFilterChoices);
}

//////////////////////////////////////////////////////////////////////
// Return list of filters mapped by the plugin at lot level
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetLabelFilterChoicesLotLevel(const QString & strDataTypeQuery,
                                                      QStringList & strlLabelFilterChoices)
{
    GSLOG(SYSLOG_SEV_DEBUG, "GexRemoteDatabase::GetLabelFilterChoicesLotLevel...");

    strlLabelFilterChoices.clear();

    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->GetLabelFilterChoicesLotLevel(strDataTypeQuery, strlLabelFilterChoices);
}

//////////////////////////////////////////////////////////////////////
// Return list of filters mapped by the plugin at wafer level
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetLabelFilterChoicesWaferLevel(const QString & strDataTypeQuery,
                                                        QStringList & strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();

    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->GetLabelFilterChoicesWaferLevel(strDataTypeQuery, strlLabelFilterChoices);
}

//////////////////////////////////////////////////////////////////////
// Get available GexDb plugins
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetAvailablePlugins(QList<GexDbPlugin_ID*> & pPluginList)
{
    GSLOG(SYSLOG_SEV_DEBUG, m_strPluginDirectory.toLatin1().data() );

    QDir						clDir;
    QStringList					pList;
    QStringList::Iterator		it;
    GexDbPlugin_ID				*pPluginID=0;
    int							num_plugins_ok=0;

    // First unload plugins
    UnloadPlugins(pPluginList);

    // Get list of library files (*.dll, *.so) in plugin directory
    clDir.setPath(m_strPluginDirectory);

#if defined unix
#ifdef QT_DEBUG
    pList =
        clDir.entryList(QStringList() << "*d.so", QDir::Files, QDir::Unsorted);
#else
    pList =
        clDir.entryList(QStringList() << "*.so", QDir::Files, QDir::Unsorted);
#endif
#elif defined __MACH__
#ifdef QT_DEBUG
    pList = clDir.entryList(QStringList() << "*d.dylib", QDir::Files, QDir::Unsorted);
#else
     pList = clDir.entryList(QStringList() << "*.dylib", QDir::Files, QDir::Unsorted);
#endif
#else
#ifdef QT_DEBUG
    pList = clDir.entryList(QStringList() << "*d.dll", QDir::Files, QDir::Unsorted);
#else
    pList = clDir.entryList(QStringList() << "*.dll", QDir::Files, QDir::Unsorted);
#endif
#endif

    // Scan list : check if plugin can be loaded, and get its name and version
    QString strFile;
    for(it = pList.begin(); it != pList.end(); ++it )
    {
        // Load library
        strFile = m_strPluginDirectory + "/";
        strFile += (*it);
        CGexSystemUtils::NormalizePath(strFile);
        pPluginID = new GexDbPlugin_ID;
        if (pPluginID->LoadPlugin(strFile, m_pMainWindow))
        {
            if (m_pMainWindow && pPluginID->m_pPlugin)
                connect(pPluginID->m_pPlugin, SIGNAL(sBusy(bool)), m_pMainWindow, SLOT(SetBusyness(bool)));
            pPluginList.append(pPluginID);
            num_plugins_ok++;
        }
        else
        {
            delete pPluginID;
            pPluginID = NULL;
        }
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 plugins loaded.")
          .arg(num_plugins_ok)
          .toLatin1().constData());
}

//////////////////////////////////////////////////////////////////////
// Unload available GexDb plugins
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::UnloadPlugins(QList<GexDbPlugin_ID*> & pPluginList)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 plugins to remove...")
          .arg(pPluginList.size())
          .toLatin1().data() );

    while (!pPluginList.isEmpty())
        delete pPluginList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Get list of available GexDb plugins
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::ConfigurePlugin(GexDbPlugin_ID *pPluginID)
{
    // Check plugin ptr
    if((pPluginID == NULL) || (pPluginID->m_pPlugin == NULL))
        return;

    QMap<QString, QString>      lProductInfoMap;
    QMap<QString, QString>      lGuiOptionsMap;

    // Set  product info
    lProductInfoMap.insert("product_id",
        QString::number(GS::LPPlugin::ProductInfo::getInstance()->getProductID()));
    lProductInfoMap.insert("optional_modules",
        QString::number(GS::LPPlugin::ProductInfo::getInstance()->getOptionalModules()));

    // Set GUI options
    lGuiOptionsMap.insert("open_mode","creation");
    lGuiOptionsMap.insert("read_only","no");
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        lGuiOptionsMap.insert("allowed_database_type",GEXDB_YM_PROD_TDR_KEY);
    else
        lGuiOptionsMap.insert("allowed_database_type",QString(GEXDB_CHAR_TDR_KEY)+","+QString(GEXDB_MAN_PROD_TDR_KEY));

    // Call plugin configuration wizard
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Calling plug-in configuration wizard...");
    pPluginID->setConfigured(pPluginID->m_pPlugin->ConfigWizard(lProductInfoMap, lGuiOptionsMap));
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "plug-in configuration wizard completed...");
}

//////////////////////////////////////////////////////////////////////
// If required, update name of plugin file read from settings
// o strFileName is the name of the plugin library (without path!)
// o update extension(.dll, .so)
// o update debug/release suffix
// o update prefix (libXXXX on unix, no XXXX on windows)
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::UpdatePluginFileName(QString &strFileName)
{
    // Make sure file name is not empty
    if(strFileName.isEmpty())
        return;

    QFileInfo		clFileInfo(strFileName);

#ifdef QT_DEBUG
    // Mac config
    // libgexdb_plugin_galaxyd.1.0.0.dylib
    // Remove extension (ie .so, .dll)
    strFileName = clFileInfo.baseName();
    // Check if old debug postfix
    if(strFileName.endsWith("_d", Qt::CaseInsensitive))
        strFileName.chop(2);
    // Check debug/release postfix
    if(!strFileName.endsWith("d", Qt::CaseInsensitive))
        strFileName += "d";
#else
    // Mac config
    // libgexdb_plugin_galaxyd.1.0.0.dylib
    // Remove extension (ie .so, .dll)
    strFileName = clFileInfo.baseName();
    // Check if old debug postfix
    if(strFileName.endsWith("_d", Qt::CaseInsensitive))
        strFileName.chop(2);
    // Check debug/release postfix
    if(strFileName.endsWith("d", Qt::CaseInsensitive))
        strFileName.chop(1);
#endif

#if defined unix
    // UNIX
    // Check prefix
    if(!strFileName.startsWith("lib", Qt::CaseInsensitive))
        strFileName.prepend(QString("lib"));
    // Add suffix
    strFileName += ".so";
#elif defined __MACH__
    // UNIX
    // Check prefix
    if(!strFileName.startsWith("lib", Qt::CaseInsensitive))
        strFileName.prepend(QString("lib"));
    // Add suffix
    strFileName += ".dylib";
#else
    // WINDOWS
    // Check prefix
    if(strFileName.startsWith("lib", Qt::CaseInsensitive))
        strFileName.remove(0, 3);
    // Add suffix
    strFileName += ".dll";
#endif

    CGexSystemUtils::NormalizePath(strFileName);
}


// TODO this function has to be removed : too dangerous
void GexRemoteDatabase::SetPluginIDPTR(GexDbPlugin_ID *pPluginID)
{
    m_pPluginID = pPluginID;
}


bool GexRemoteDatabase::LoadPluginFromDom(const QDomElement &node)
{
    QDomElement eltExternalDb = node.firstChildElement("ExternalDatabase");
    if(!eltExternalDb.isNull())
    {
        //this->m_strDatabasePhysicalPath = this->m_strDatabasePhysicalPath.remove(QString(GEX_DATABASE_EXTERNAL_DB_DEF));

        QDomElement nodeIdentification = eltExternalDb.firstChildElement("Identification");
        if (!nodeIdentification.isNull())
        {
            // Free current plugin if loaded
            if(m_pPluginID != NULL)
            {
                delete m_pPluginID;
                m_pPluginID = NULL;
            }

            QString strMessage, strPluginFileName, strPluginName = "";
            unsigned int uiPluginBuild = 0;
            bool bReadBuildOK  = false;

            // Could we read all identification fields?
            strPluginFileName = ReadPluginFileNameFromDom(nodeIdentification);
            if(strPluginFileName.isEmpty())
            {
                GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginFile", "<Identification>");
                return false;	// Corrupted file.
            }
            uiPluginBuild = ReadPluginBuildFromDom(nodeIdentification).toInt(&bReadBuildOK);
            if(!bReadBuildOK)
            {
                GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginBuild", "<Identification>");
                return false;	// Corrupted file.
            }
            strPluginName = ReadPluginNameFromDom(nodeIdentification);
            if(strPluginName.isEmpty())
            {
                GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginName", "<Identification>");
                return false;	// Corrupted file.
            }

            if(!LoadPluginID(strPluginFileName, uiPluginBuild, strPluginName))
            {
                return false;	// Corrupted file.
            }

            // Debug message
            strMessage = "GexRemoteDatabase::LoadPlugin: ";
            strMessage += m_pPluginID->pluginName();
            GSLOG(SYSLOG_SEV_DEBUG, strMessage.toLatin1().data() );

            m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;
            // Load Plugin settings
            if(!m_pPluginID->m_pPlugin->LoadSettingsFromDom(eltExternalDb))
            {
                GSET_ERROR2(GexRemoteDatabase, ePluginSettings, GGET_LASTERROR(GexDbPlugin_Base, m_pPluginID->m_pPlugin), strPluginName.toLatin1().constData(), "");
                //delete m_pPluginID;
                //m_pPluginID = NULL;
                return false;
            }
        }
        else
        {
            GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<Identification>");
            return false;
        }
    }
    else
    {
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<ExternalDatabase>");
        return false;
    }

    // Success
    return true;
}

QString GexRemoteDatabase::ReadPluginFileNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("PluginFile");
    QString pluginFileName = "";
    if (!elt.isNull())
        pluginFileName = elt.text();
    else
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<PluginFile>");

    return pluginFileName;
}

QString GexRemoteDatabase::ReadPluginBuildFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("PluginBuild");
    QString pluginBuild = "";
    if (!elt.isNull())
        pluginBuild = elt.text();
    else
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<PluginName>");

    return pluginBuild;
}

QString GexRemoteDatabase::ReadPluginNameFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("PluginName");
    QString pluginName = "";
    if (!elt.isNull())
        pluginName = elt.text();
    else
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<PluginBuild>");

    return pluginName;
}


//////////////////////////////////////////////////////////////////////
// Load plugin specified in settings file
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::LoadPlugin(const QString &strSettingsFile)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadPlugin '%1'...").arg(strSettingsFile).toLatin1().data() );

    this->m_strDatabasePhysicalPath = strSettingsFile;
    this->m_strDatabasePhysicalPath = this->m_strDatabasePhysicalPath.remove(QString(GEX_DATABASE_EXTERNAL_DB_DEF));

    // Free current plugin if loaded
    if(m_pPluginID != NULL)
    {
        delete m_pPluginID;
        m_pPluginID = NULL;
    }

    // Open settings file
    QFile clSettingsFile(strSettingsFile);
    if(!clSettingsFile.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed opening database settings file '%1'")
               .arg(strSettingsFile).toLatin1().constData() );
        GSET_ERROR1(GexRemoteDatabase, eOpenSettingsFile, NULL, strSettingsFile.toLatin1().constData());
        return false;	// Failed opening file.
    }

    // Assign file I/O stream
    QTextStream hFile(&clSettingsFile);
    QString		strString;
    QString		strKeyword;
    QString		strParameter;

    // Get first non-empty line
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(!strString.isEmpty())
            break;
    }

    // Do we have expected line??
    if(hFile.atEnd() || (strString.toLower() != "<externaldatabase>"))
    {
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<ExternalDatabase>");
        clSettingsFile.close();
        return false;	// Corrupted file.
    }

    // Now look for identification marker
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(strString.toLower() == "<identification>")
            break;
    }
    if(hFile.atEnd())
    {
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<Identification>");
        clSettingsFile.close();
        return false;	// Corrupted file.
    }

    QString			strPluginFileName, strPluginName;
    unsigned int	uiPluginBuild = 0;
    bool			bReadBuildOK = true;

    while(!hFile.atEnd())
    {
        // Read line.
        strString = hFile.readLine().trimmed();
        strKeyword = strString.section('=',0,0);
        strParameter = strString.section('=',1);

        if(strString.toLower() == "</identification>")
            break;

        if(strKeyword.toLower() == "pluginfile")
            strPluginFileName = strParameter;

        if(strKeyword.toLower() == "pluginbuild")
            uiPluginBuild = strParameter.toUInt(&bReadBuildOK);

        if(strKeyword.toLower() == "pluginname")
            strPluginName = strParameter;
    }

    // Could we read all identification fields?
    if(strPluginFileName.isEmpty())
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginFile", "<Identification>");
        clSettingsFile.close();
        return false;	// Corrupted file.
    }
    if(!bReadBuildOK)
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginBuild", "<Identification>");
        clSettingsFile.close();
        return false;	// Corrupted file.
    }
    if(strPluginName.isEmpty())
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginName", "<Identification>");
        clSettingsFile.close();
        return false;	// Corrupted file.
    }

    if(!LoadPluginID(strPluginFileName, uiPluginBuild, strPluginName))
    {
        clSettingsFile.close();
        return false;	// Corrupted file.
    }


    // Debug message
    strString = "GexRemoteDatabase::LoadPlugin: ";
    strString += m_pPluginID->pluginName();
    GSLOG(SYSLOG_SEV_DEBUG, strString.toLatin1().data() );

    m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;
    // Load Plugin settings
    if(!m_pPluginID->m_pPlugin->LoadSettings(&clSettingsFile))
    {
        GSET_ERROR2(GexRemoteDatabase, ePluginSettings, GGET_LASTERROR(GexDbPlugin_Base, m_pPluginID->m_pPlugin), strPluginName.toLatin1().constData(), strSettingsFile.toLatin1().constData());
        //delete m_pPluginID;
        //m_pPluginID = NULL;
        clSettingsFile.close();
        return false;
    }

    // Success
    clSettingsFile.close();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Load plugin
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::LoadPluginID(QString &strPluginFileName, unsigned int uiPluginBuild, QString &strPluginName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("LoadPlugin '%1' version %2 from file %3 in dir %4...")
           .arg(strPluginName).arg(uiPluginBuild)
           .arg(strPluginFileName)
           .arg(m_strPluginDirectory)
           .toLatin1().data()
           );

    // Free current plugin if loaded
    if(m_pPluginID != NULL)
    {
        delete m_pPluginID;
        m_pPluginID = NULL;
    }

    // Could we read all identification fields?
    if(strPluginFileName.isEmpty())
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginFile", "<Identification>");
        return false;	// Corrupted file.
    }
    if(uiPluginBuild == 0)
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginBuild", "<Identification>");
        return false;	// Corrupted file.
    }
    if(strPluginName.isEmpty())
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginName", "<Identification>");
        return false;	// Corrupted file.
    }

    // Try to load plugin
    UpdatePluginFileName(strPluginFileName);

    // Craete full filename and load library
    QString strString;
    QString strPluginFullFileName = m_strPluginDirectory + "/";
    strPluginFullFileName += strPluginFileName;
    CGexSystemUtils::NormalizePath(strPluginFullFileName);	// CGexSystemUtils	clSysUtils;

    // Get plugin identification
    m_pPluginID = new GexDbPlugin_ID;
    if (!m_pPluginID->LoadPlugin(strPluginFullFileName, m_pMainWindow))
    {
        delete m_pPluginID;
        m_pPluginID = NULL;
        return false;
    }

    if (m_pMainWindow && m_pPluginID->m_pPlugin)
        connect(m_pPluginID->m_pPlugin, SIGNAL(sBusy(bool)), m_pMainWindow, SLOT(SetBusyness(bool)));

    m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;
    // Get Connection Time out from ReportOptions
    //m_pPluginID->m_pPlugin->m_nConnectionTimeOut = ReportOptions.GetOption("databases","database_timeout").toInt();

    // Debug message
    strString = m_pPluginID->pluginName();
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data() );

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read GexDb plugin settings
//////////////////////////////////////////////////////////////////////
GexDbPlugin_ID *GexRemoteDatabase::LoadSettings(QList<GexDbPlugin_ID*> & pPluginList, const QString &strFile)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Load Settings in '%1'")
          .arg(strFile)
          .toLatin1().data());

    // Open settings file
    QFile clSettingsFile(strFile);
    if(!clSettingsFile.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Failed opening database settings file '%1'")
              .arg( strFile.toLatin1().constData())
              .toLatin1().constData());
        GSET_ERROR1(GexRemoteDatabase, eOpenSettingsFile, NULL, strFile.toLatin1().constData());
        return NULL;	// Failed opening file.
    }

    // Assign file I/O stream
    QTextStream hFile(&clSettingsFile);
    QString     strString;
    QString     strKeyword;
    QString     strParameter;

    // Get first non-empty line
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(!strString.isEmpty())
            break;
    }

    // Do we have expected line??
    if(hFile.atEnd() || (strString.toLower() != "<externaldatabase>"))
    {
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<ExternalDatabase>");
        clSettingsFile.close();
        return NULL;	// Corrupted file.
    }

    // Now look for identification marker
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(strString.toLower() == "<identification>")
            break;
    }
    if(hFile.atEnd())
    {
        GSET_ERROR1(GexRemoteDatabase, eMarkerNotFound, NULL, "<Identification>");
        clSettingsFile.close();
        return NULL;	// Corrupted file.
    }

    QString         strPluginFile, strPluginName;
    unsigned int    uiPluginBuild = 0;
    bool            bReadBuildOK = true;

    while(!hFile.atEnd())
    {
        // Read line.
        strString = hFile.readLine().trimmed();
        strKeyword = strString.section('=',0,0);
        strParameter = strString.section('=',1);

        if(strString.toLower() == "</identification>")
            break;

        if(strKeyword.toLower() == "pluginfile")
            strPluginFile = strParameter;

        if(strKeyword.toLower() == "pluginbuild")
            uiPluginBuild = strParameter.toUInt(&bReadBuildOK);

        if(strKeyword.toLower() == "pluginname")
            strPluginName = strParameter;
    }

    // Could we read all identification fields?
    if(strPluginFile.isEmpty())
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginFile", "<Identification>");
        clSettingsFile.close();
        return NULL;	// Corrupted file.
    }
    if(!bReadBuildOK)
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginBuild", "<Identification>");
        clSettingsFile.close();
        return NULL;	// Corrupted file.
    }
    if(strPluginName.isEmpty())
    {
        GSET_ERROR2(GexRemoteDatabase, eReadField, NULL, "PluginName", "<Identification>");
        clSettingsFile.close();
        return NULL;	// Corrupted file.
    }

    // Check if specified plugin available
    GexDbPlugin_ID * pPluginID = NULL;
    UpdatePluginFileName(strPluginFile);

    QList<GexDbPlugin_ID*>::iterator itPlugin = pPluginList.begin();

    while(itPlugin != pPluginList.end())
    {
        if(((*itPlugin)->pluginFileName() == strPluginFile)
                && ((*itPlugin)->pluginBuild() == uiPluginBuild)
                && ((*itPlugin)->pluginName() == strPluginName))
        {
            pPluginID = (*itPlugin);
            break;
        }

        itPlugin++;
    }

    if(!pPluginID)
    {
        GSET_ERROR4(GexRemoteDatabase, eMissingPlugin, NULL, m_strPluginDirectory.toLatin1().constData(), strPluginFile.toLatin1().constData(),
                    strPluginName.toLatin1().constData(), uiPluginBuild);
        clSettingsFile.close();
        return NULL;	// Plugin not found.
    }

    //
    pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;
    // Load Plugin settings
    if(!pPluginID->m_pPlugin->LoadSettings(&clSettingsFile))
    {
        GSET_ERROR2(GexRemoteDatabase, ePluginSettings, GGET_LASTERROR(GexDbPlugin_Base, pPluginID->m_pPlugin), strPluginName.toLatin1().constData(), strFile.toLatin1().constData());
        clSettingsFile.close();
        return NULL;	// Plugin not found.
    }

    // Success
    clSettingsFile.close();
    return pPluginID;
}

///////////////////////////////////////////////////////////
// Write GexDb plugin settings
///////////////////////////////////////////////////////////
bool GexRemoteDatabase::WriteSettings(GexDbPlugin_ID *pPluginID, const QString &strFile)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Write Settings in '%1'").arg(strFile).toLatin1().data() );

    // Check plugin ptr
    if((pPluginID == NULL) || (pPluginID->m_pPlugin == NULL))
    {
        GSET_ERROR0(GexRemoteDatabase, eNoPluginLoaded, NULL);
        return false;
    }

    // Check if plugin configured
    if(!pPluginID->isConfigured())
    {
        GSET_ERROR1(GexRemoteDatabase, ePluginNotConfigured, NULL, pPluginID->pluginName().toLatin1().constData());
        return false;
    }

    // Open settings file
    QFile clSettingsFile(strFile);
    if(!clSettingsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        GSET_ERROR1(GexRemoteDatabase, eCreateSettingsFile, NULL, strFile.toLatin1().constData());
        return false;	// Failed creating file.
    }

    // Assign file I/O stream
    QTextStream hFile(&clSettingsFile);

    // Save the Release Plugin file name instead to the Debug
    QString		strPluginFileName = pPluginID->pluginFileName();
    if(strPluginFileName.contains("d."))
    {
        // try with release plugin
        strPluginFileName = strPluginFileName.replace("d.",".");
    }

    // Start definition marker
    hFile << "<ExternalDatabase>" << endl;

    // Write plugin identification
    hFile << endl;
    hFile << "<Identification>" << endl;
    hFile << "PluginFile=" << strPluginFileName << endl;
    hFile << "PluginName=" << pPluginID->pluginName() << endl;
    hFile << "PluginBuild=" << QString::number(pPluginID->pluginBuild()) << endl;
    hFile << "</Identification>" << endl;

    // Write plugin settings
    hFile << endl;
    hFile << "<PluginConfig>" << endl;
    if(!pPluginID->m_pPlugin->WriteSettings(&hFile))
    {
        SetLastErrorFromPlugin();
        clSettingsFile.close();
        clSettingsFile.remove();
        return false;
    }
    hFile << endl;
    hFile << "</PluginConfig>" << endl;

    // End definition marker
    hFile << endl;
    hFile << "</ExternalDatabase>" << endl << endl;

    // Clean close
    clSettingsFile.close();

    if (pPluginID->m_pPlugin->m_pclDatabaseConnector)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("for plugin driver = '%1'")
               .arg(pPluginID->m_pPlugin->m_pclDatabaseConnector->m_strDriver)
               .toLatin1().constData() );
        if (pPluginID->m_pPlugin->m_pclDatabaseConnector->IsSQLiteDB())
            GSLOG(SYSLOG_SEV_DEBUG, QString("SQLite file '%1'")
                  .arg( strFile.toLatin1().constData())
                  .toLatin1().constData());
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "pPluginID->m_pPlugin->m_pclDatabaseConnector NULL !");
    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Returns true if insertion supported, false else
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsInsertionSupported()
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "Is insertion supported : PluginID null !");
        return false;
    }
    bool r=m_pPluginID->m_pPlugin->IsInsertionSupported();
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Is insertion supported: %1")
      //    .arg(r?"yes":"no").toLatin1().data() );
    return r;
}

//////////////////////////////////////////////////////////////////////
// Returns true if DB update supported, false else
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsUpdateSupported()
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Is Update Supported : PluginID null !").toLatin1().data());
        return false;
    }

    bool r=m_pPluginID->m_pPlugin->IsUpdateSupported();
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Is Update Supported : %1 : %2")
      //     .arg(m_pPluginID->pluginFileName()).arg(r?"no":"yes").toLatin1().data() );
    return r;
}

//////////////////////////////////////////////////////////////////////
// Returns true if testing stages are supported, false else
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsTestingStagesSupported()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Is Testing Stages Supported...");

    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsTestingStagesSupported();
}

//////////////////////////////////////////////////////////////////////
// Returns true if parameter selection supported (for filtering), false else
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsParameterSelectionSupported()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Is Parameter Selection Supported...");

    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsParameterSelectionSupported();
}

//////////////////////////////////////////////////////////////////////
// Returns true if Enterprise Reports supported, false else
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsEnterpriseReportsSupported()
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsEnterpriseReportsSupported();
}

//////////////////////////////////////////////////////////////////////
// Returns true if Reports Center supported, false else
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsReportsCenterSupported()
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsReportsCenterSupported();
}

//////////////////////////////////////////////////////////////////////
// Returns true if the specified testing stage is Final Test
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsTestingStage_FinalTest(const QString & strTestingStage)
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsTestingStage_FinalTest(strTestingStage);
}

//////////////////////////////////////////////////////////////////////
// Returns true if the specified testing stage is Foundry (E-Test)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsTestingStage_Foundry(const QString & strTestingStage)
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsTestingStage_Foundry(strTestingStage);
}

//////////////////////////////////////////////////////////////////////
// Returns the name of Foundry testing stage (E-Test)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetTestingStageName_Foundry(QString & strTestingStage)
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->GetTestingStageName_Foundry(strTestingStage);
}

//////////////////////////////////////////////////////////////////////
// Check if Customer debug mode is activated
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsCustomerDebugModeActivated()
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsCustomerDebugModeActivated();
}

//////////////////////////////////////////////////////////////////////
// Check if Database up-to-date (schema...)
//////////////////////////////////////////////////////////////////////
// Check major/minor/build version
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsDbConnected()
{
    if (!m_pPluginID || !m_pPluginID->m_pPlugin)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Is Db Up To Date : PluginID null");
        return false;
    }
    if (!m_pPluginID->m_pPlugin->m_pclDatabaseConnector)
        return false;

    return m_pPluginID->m_pPlugin->m_pclDatabaseConnector->IsConnected();

}

//////////////////////////////////////////////////////////////////////
// Check if Database up-to-date (schema...)
//////////////////////////////////////////////////////////////////////
// Check major/minor/build version
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsDbUpToDate(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                     unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name,
                                     unsigned int *puiLatestSupportedDbVersion_Build)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Is Db up to date for '%1'...").arg(m_strDatabasePhysicalPath).toLatin1().constData());
    if (!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Is Db Up To Date : PluginID null");
        return false;
    }

    m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;

    bool bStatus = m_pPluginID->m_pPlugin->IsDbUpToDate(
                pbDbIsUpToDate, strCurrentDbVersion_Name, puiCurrentDbVersion_Build,
                strLatestSupportedDbVersion_Name, puiLatestSupportedDbVersion_Build);

    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Is Db Up To Date : CurrentDbVersion:%1 LatestSupported:%2 ")
               .arg(strCurrentDbVersion_Name).arg(strLatestSupportedDbVersion_Name).toLatin1().data() );
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Check major/minor version
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsDbUpToDateForInsertion(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                                 unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name,
                                                 unsigned int *puiLatestSupportedDbVersion_Build)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Is DB up to date for insertion for DB Folder=%1").arg(m_strDatabasePhysicalPath).toLatin1().constData());
    if (!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Is Db Up To Date For Insertion : PluginID null.");
        return false;
    }

    m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;

    bool bStatus = m_pPluginID->m_pPlugin->IsDbUpToDateForInsertion(
                pbDbIsUpToDate, strCurrentDbVersion_Name, puiCurrentDbVersion_Build,
                strLatestSupportedDbVersion_Name, puiLatestSupportedDbVersion_Build);

    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Is Db Up To Date For Insertion : CurrentDbVersion:%1 LatestSupported:%2 ")
               .arg(strCurrentDbVersion_Name).arg(strLatestSupportedDbVersion_Name).toLatin1().data() );
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Check major version
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsDbUpToDateForExtraction(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                                  unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name,
                                                  unsigned int *puiLatestSupportedDbVersion_Build)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("DBFolder=%1").arg(m_strDatabasePhysicalPath).toLatin1().constData());
    if (!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Is Db Up To Date : PluginID null.");
        return false;
    }

    m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;

    bool bStatus = m_pPluginID->m_pPlugin->IsDbUpToDateForExtraction(
                pbDbIsUpToDate, strCurrentDbVersion_Name, puiCurrentDbVersion_Build,
                strLatestSupportedDbVersion_Name, puiLatestSupportedDbVersion_Build);

    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Is Db Up To Date For Extraction : CurrentDbVersion:%1 LatestSupported:%2 ")
               .arg(strCurrentDbVersion_Name).arg(strLatestSupportedDbVersion_Name).toLatin1().data() );
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Run a specific incremental update
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString >  &summary)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Incremental Updates...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->IncrementalUpdate(incrementalName, testingStage, target, summary);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetIncrementalUpdatesCount(bool checkDatabase, int &incrementalSplitlots)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Get Incremental Updates...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetIncrementalUpdatesCount(checkDatabase, incrementalSplitlots);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates to do according to the settings
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetNextAutomaticIncrementalUpdatesList(QMap< QString,QMap< QString,QStringList > > & IncrementalUpdatesList)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Get Incremental Updates list To Do...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetNextAutomaticIncrementalUpdatesList(IncrementalUpdatesList);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates to do according to the settings
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetFirstIncrementalUpdatesList(QString incrementalName, QMap< QString,QMap< QString,QStringList > > & IncrementalUpdatesList)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Get Incremental Updates list To Do...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetFirstIncrementalUpdatesList(incrementalName, IncrementalUpdatesList);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > & incrementalUpdates)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Get Incremental Updates...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetIncrementalUpdatesSettings(incrementalUpdates);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Set incremental updates
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::SetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > & IncrementalUpdates)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Set Incremental Updates...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->SetIncrementalUpdatesSettings(IncrementalUpdates);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Check incremental updates property
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsIncrementalUpdatesSettingsValidValue(QString &lName, QString &lValue)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Set Incremental Updates...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->IsIncrementalUpdatesSettingsValidValue(lName, lValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Check if automatic incremental updates are enabled
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsAutomaticIncrementalUpdatesEnabled()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Is automatic incremental updates enabled...");

    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsAutomaticIncrementalUpdatesEnabled();
}

int GexRemoteDatabase::IsConsolidationInProgress(QString testingStage, QString lot, QString sublots, QString wafers, QString consoType, QString testFlow, QString consoLevel, QString testInsertion)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Checking consolidation in progress");

    if(!m_pPluginID)
        return false;

    int bStatus = m_pPluginID->m_pPlugin->IsConsolidationInProgress(testingStage, lot, sublots, wafers, consoType, testFlow, consoLevel, testInsertion);
    if(bStatus == 2)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::IsAutomaticStartup()
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->IsAutomaticStartup();
}

//////////////////////////////////////////////////////////////////////
// Set automatic startup are enabled
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::SetAutomaticStartup(bool bValue)
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->SetAutomaticStartup(bValue);
}

const QString GexRemoteDatabase::GetTdrLinkName()
{
    if(!m_pPluginID)
        return QString("");

    return m_pPluginID->m_pPlugin->GetTdrLinkName();
}

//////////////////////////////////////////////////////////////////////
// Set Adr Link Name
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::SetTdrLinkName(const QString& value)
{
    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->SetTdrLinkName(value);
}

const QString GexRemoteDatabase::GetAdrLinkName()
{
    if(!m_pPluginID)
        return QString("");

    return m_pPluginID->m_pPlugin->GetAdrLinkName();
}

//////////////////////////////////////////////////////////////////////
// Set Adr Link Name
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::SetAdrLinkName(const QString& value)
{
    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->SetAdrLinkName(value);
}

//////////////////////////////////////////////////////////////////////
// true if this TDR must have an ADR link
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::MustHaveAdrLink()
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->MustHaveAdrLink();
}

//////////////////////////////////////////////////////////////////////
// Update Database (schema...)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::UpdateDb(QString command)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->UpdateDb(command);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}


//////////////////////////////////////////////////////////////////////
// Update Database (schema...): consolidated update
// eTestingStage:
//		0 : to update all testingstage
//		else : to update only one testing stage
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::UpdateConsolidationProcess(int eTestingStage/*=0*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->UpdateConsolidationProcess(eTestingStage);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Set/Get insertion validation flags
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::SetInsertionValidationOptionFlag(unsigned int uiInsertionValidationOptionFlag)
{
    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->SetInsertionValidationOptionFlag(uiInsertionValidationOptionFlag);
}

void GexRemoteDatabase::SetInsertionValidationFailOnFlag(unsigned int uiInsertionValidationFailOnFlag)
{
    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->SetInsertionValidationFailOnFlag(uiInsertionValidationFailOnFlag);
}

unsigned int GexRemoteDatabase::GetInsertionValidationOptionFlag()
{
    if(!m_pPluginID)
        return 0;

    return m_pPluginID->m_pPlugin->GetInsertionValidationOptionFlag();
}

unsigned int GexRemoteDatabase::GetInsertionValidationFailOnFlag()
{
    if(!m_pPluginID)
        return 0;

    return m_pPluginID->m_pPlugin->GetInsertionValidationFailOnFlag();
}

//////////////////////////////////////////////////////////////////////
// Insert data file into DB
//////////////////////////////////////////////////////////////////////
bool
GexRemoteDatabase::InsertDataFile(struct GsData* lGsData,
                                  int lSqliteSplitlotId,
                                  const QString& strDataFileName,
                                  GS::QtLib::DatakeysEngine& dbKeysEngine,
                                  bool* pbDelayInsertion)
{
    if(!m_pPluginID)
        return false;
    if (!m_pPluginID->m_pPlugin)
        return false;

    m_pPluginID->m_pPlugin->m_strDBFolder=this->m_strDatabasePhysicalPath;
    if (pGexMainWindow)
    {
        connect(m_pPluginID->m_pPlugin, SIGNAL(sUpdateProgress(int)), pGexMainWindow, SLOT(UpdateProgress(int)));
        connect(m_pPluginID->m_pPlugin, SIGNAL(sResetProgress(bool)), pGexMainWindow, SLOT(ResetProgress(bool)));
        connect(m_pPluginID->m_pPlugin, SIGNAL(sMaxProgress(int)), pGexMainWindow, SLOT(SetMaxProgress(int)));
    }
    bool bStatus = m_pPluginID->m_pPlugin->
        InsertDataFile(lGsData,
                       lSqliteSplitlotId,
                       strDataFileName,
                       dbKeysEngine,
                       pbDelayInsertion,
                       &m_lSplitlotID,
                       &m_nTestingStage);
    if (pGexMainWindow)
    {
        disconnect(m_pPluginID->m_pPlugin, SIGNAL(sUpdateProgress(int)), pGexMainWindow, SLOT(UpdateProgress(int)));
        disconnect(m_pPluginID->m_pPlugin, SIGNAL(sResetProgress(bool)), pGexMainWindow, SLOT(ResetProgress(bool)));
        disconnect(m_pPluginID->m_pPlugin, SIGNAL(sMaxProgress(int)), pGexMainWindow, SLOT(SetMaxProgress(int)));
    }
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Insert WYR data file into DB
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::InsertWyrDataFile(const QString& strDataFileName,
                                          const QString& strSiteName,
                                          const QString& strTestingStage,
                                          unsigned int uiWeekNb,
                                          unsigned int uiYear,
                                          bool* pbDelayInsertion)
{
    if(!m_pPluginID)
        return false;
    if(!m_pPluginID->m_pPlugin)
        return false;

    if (pGexMainWindow)
    {
        connect(m_pPluginID->m_pPlugin, SIGNAL(sUpdateProgress(int)), pGexMainWindow, SLOT(UpdateProgress(int)));
        connect(m_pPluginID->m_pPlugin, SIGNAL(sResetProgress(bool)), pGexMainWindow, SLOT(ResetProgress(bool)));
        connect(m_pPluginID->m_pPlugin, SIGNAL(sMaxProgress(int)), pGexMainWindow, SLOT(SetMaxProgress(int)));
    }
    bool bStatus = m_pPluginID->m_pPlugin->InsertWyrDataFile(strDataFileName, strSiteName, strTestingStage, uiWeekNb, uiYear, pbDelayInsertion);
    if (pGexMainWindow)
    {
        disconnect(m_pPluginID->m_pPlugin, SIGNAL(sUpdateProgress(int)), pGexMainWindow, SLOT(UpdateProgress(int)));
        disconnect(m_pPluginID->m_pPlugin, SIGNAL(sResetProgress(bool)), pGexMainWindow, SLOT(ResetProgress(bool)));
        disconnect(m_pPluginID->m_pPlugin, SIGNAL(sMaxProgress(int)), pGexMainWindow, SLOT(SetMaxProgress(int)));
    }
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}


bool GexRemoteDatabase::InsertAlarm(GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->InsertAlarm(m_lSplitlotID, m_nTestingStage, eAlarmCat, eAlarmLevel, lItemNumber, strItemName, uiFlags, fLCL, fUCL, fValue, strUnits);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Insert an alarm for specified splitlot (wafer testing stage)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::InsertAlarm_Wafer(GexDbPlugin_Base::AlarmCategories eAlarmCat, GexDbPlugin_Base::AlarmLevels eAlarmLevel, long lItemNumber, QString & strItemName, unsigned int uiFlags, float fLCL, float fUCL, float fValue, QString strUnits)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->InsertAlarm_Wafer(m_lSplitlotID, eAlarmCat, eAlarmLevel, lItemNumber, strItemName, uiFlags, fLCL, fUCL, fValue, strUnits);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryField(GexDbPlugin_Filter & cFilters,
                                   QStringList & cMatchingValues,
                                   bool bSoftBin/*=false*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("QueryField on Stage='%1' Field='%2' conso=%3...")
           .arg(cFilters.strDataTypeQuery).arg(cFilters.mQueryFields.join(";")).arg(cFilters.bConsolidatedData?"true":"false")
           .toLatin1().data() );

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QueryField(cFilters, cMatchingValues, bSoftBin);
    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("GexRemoteDatabase QueryField error : plugin QueryField failed !").toLatin1().data() );
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return all tests (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QueryTestlist(cFilters, cMatchingValues, bParametricOnly);
    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
        SetLastErrorFromPlugin();
    }

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return all binnings (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryBinlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin/*=false*/,bool bIncludeBinName/*=false*/, bool bProdDataOnly/*=false*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QueryBinlist(cFilters, cMatchingValues, bSoftBin, true, bIncludeBinName, bProdDataOnly);
    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return all products for genealogy reports, with data for at least
// 2 testing stages (use only date in the filter)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryProductList_Genealogy(GexDbPlugin_Filter & cFilters,
                                                   QStringList & cMatchingValues,
                                                   bool bAllTestingStages)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "QueryProductList for Genealogy...");

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QueryProductList_Genealogy(cFilters, cMatchingValues, bAllTestingStages);
    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin) );
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return all products (use only date in the filter)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryProductList(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,QString strProductName/*=""*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QueryProductList(cFilters, cMatchingValues, strProductName);
    if(!bStatus)
    {
        GSLOG(SYSLOG_SEV_ERROR, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin) );
        SetLastErrorFromPlugin();
    }
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return all Test conditions (according to all the filters)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryTestConditionsList(GexDbPlugin_Filter & filters,
                                                QStringList & matchingTestConditions)
{
    bool status = m_pPluginID->m_pPlugin->QueryTestConditionsList(filters, matchingTestConditions);

    if (!status)
    {
        GSLOG(SYSLOG_SEV_ERROR, GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin));
        SetLastErrorFromPlugin();
    }

    return status;
}

//////////////////////////////////////////////////////////////////////
// Return all valid Data files (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::QueryDataFiles(
        GexDbPlugin_Filter & cFilters,
        const QString & strTestlist,
        tdGexDbPluginDataFileList & cMatchingFiles,	// list of extracted stdf files
        const QString & strDatabasePhysicalPath,
        const QString & strLocalDir,
        bool *pbFilesCreatedInFinalLocation,
        GexDbPlugin_Base::StatsSource eStatsSource)
{
    if(!m_pPluginID)
    {
        GSLOG(SYSLOG_SEV_ERROR, " m_pPluginID NULL !");
        return false;
    }

    bool bStatus = m_pPluginID->m_pPlugin->QueryDataFiles(
                cFilters, strTestlist,
                cMatchingFiles, strDatabasePhysicalPath,
                strLocalDir, pbFilesCreatedInFinalLocation, eStatsSource);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return data for Production - UPH graph (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetDataForProd_UPH(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetDataForProd_UPH(cFilters, clXYChartList);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Yield, UPH)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ER_Prod_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ER_Prod_GetParts(cFilters, clER_PartsData);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Yield, UPH)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::AER_GetDataset(const GexDbPluginERDatasetSettings & datasetSettings, GexDbPluginERDataset& datasetResult)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->AER_GetDataset(datasetSettings, datasetResult);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ER_Genealogy_YieldVsYield_GetParts(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ER_Genealogy_YieldVsYield_GetParts(cFilters, clER_PartsData);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Genealogy - Yield vs Parameter)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ER_Genealogy_YieldVsParameter_GetParts(
        GexDbPlugin_Filter & cFilters,
        GexDbPlugin_ER_Parts & clER_PartsData)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ER_Genealogy_YieldVsParameter_GetParts(cFilters, clER_PartsData);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// // Get bin counts for Enterprise Report graphs (Yield, UPH)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ER_Prod_GetBinnings(GexDbPlugin_Filter & cFilters, GexDbPlugin_ER_Parts & clER_PartsData, GexDbPlugin_ER_Parts_Graph *pGraph, GexDbPlugin_ER_Parts_Layer *pLayer, const QString & strAggregateLabel, GexDbPlugin_BinList & clBinList)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ER_Prod_GetBinnings(cFilters, clER_PartsData, pGraph, pLayer, strAggregateLabel, clBinList);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return data for Production - Yield graph (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetDataForProd_Yield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList, int nBinning, bool bSoftBin)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetDataForProd_Yield(cFilters, clXYChartList, nBinning, bSoftBin);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return data for Production - Consolidated Yield graph (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetDataForProd_ConsolidatedYield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetDataForProd_ConsolidatedYield(cFilters, clXYChartList);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return data for WYR - Standard report (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetDataForWyr_Standard(GexDbPlugin_Filter & cFilters, GexDbPlugin_WyrData & cWyrData)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetDataForWyr_Standard(cFilters, cWyrData);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::QuerySQL(QString & strQuery, QList<QStringList> & listResults)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QuerySQL(strQuery, listResults);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::QuerySplitlots(GexDbPlugin_Filter & cFilters, GexDbPlugin_SplitlotList & clSplitlotList,bool bPurge)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->QuerySplitlots(cFilters, clSplitlotList, bPurge);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// // Transfer remote data files to local FS
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::TransferDataFiles(tdGexDbPluginDataFileList &cMatchingFiles, const QString & strLocalDir)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->TransferDataFiles(cMatchingFiles, strLocalDir);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Returns details about last error
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(GexRemoteDatabase,this);
}

//////////////////////////////////////////////////////////////////////
// Set details about last error
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::SetLastErrorFromPlugin()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
    {
        QString lError;
        lError = GetPluginName();
        lError+= " \n";
        lError+= GetPluginErrorCode();
        lError+= ": ";
        lError+= GetPluginErrorMsg();

        // Set Remote error
        GSET_ERROR1(GexRemoteDatabase, ePlugin, NULL, lError.toLatin1().constData());
    }
}

//////////////////////////////////////////////////////////////////////
// Get stack of insertion warnings
//////////////////////////////////////////////////////////////////////
void GexRemoteDatabase::GetWarnings(QStringList & strlWarnings)
{
    strlWarnings.clear();

    if(!m_pPluginID)
        return;

    m_pPluginID->m_pPlugin->GetWarnings(strlWarnings);
}


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
QString GexRemoteDatabase::GetPluginName()
{
    QString strPluginName = "Plugin not found";
    if(m_pPluginID)
    {
        strPluginName = m_pPluginID->pluginFileName();
        if(m_pPluginID->m_pPlugin)
            m_pPluginID->m_pPlugin->GetPluginName(strPluginName);
    }

    return strPluginName;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
QString GexRemoteDatabase::GetPluginErrorMsg()
{
    QString lError = "Plugin not found";
    if(m_pPluginID && m_pPluginID->m_pPlugin)
        lError = GGET_LASTERRORMSG(GexDbPlugin_Base, m_pPluginID->m_pPlugin);

    return lError;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
QString GexRemoteDatabase::GetPluginErrorCode()
{
    QString lError = "Plugin not found";
    if(m_pPluginID && m_pPluginID->m_pPlugin)
        lError = "TDR-"+QString::number(GGET_LASTERRORCODE(GexDbPlugin_Base, m_pPluginID->m_pPlugin));

    return lError;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
QString GexRemoteDatabase::GetPluginErrorDescription()
{
    QString lError = "Plugin not found";
    if(m_pPluginID && m_pPluginID->m_pPlugin)
        lError = GGET_LASTERRORDESC(GexDbPlugin_Base, m_pPluginID->m_pPlugin);

    return lError;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
QString GexRemoteDatabase::GetTdrTypeName()
{
    QString strTdrType = "";
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            m_pPluginID->m_pPlugin->GetTdrTypeName(strTdrType);

    return strTdrType;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
QString GexRemoteDatabase::GetTdrStorageEngine()
{
    QString lTdrStorageEngine,LTdrStorageFormat;
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            m_pPluginID->m_pPlugin->GetStorageEngineName(lTdrStorageEngine,LTdrStorageFormat);

    return lTdrStorageEngine;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsCharacTdr()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            return m_pPluginID->m_pPlugin->IsCharacTdr();

    return false;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsManualProdTdr()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            return m_pPluginID->m_pPlugin->IsManualProdTdr();

    return false;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsYmProdTdr()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            return m_pPluginID->m_pPlugin->IsYmProdTdr();

    return false;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsAdr()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            return m_pPluginID->m_pPlugin->IsAdr();

    return false;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsLocalAdr()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
            return m_pPluginID->m_pPlugin->IsLocalAdr();

    return false;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::IsSecuredMode()
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
    {
        QString lSecuredMode;
        if(!m_pPluginID->m_pPlugin->GetSecuredMode(lSecuredMode))
        {
            SetLastErrorFromPlugin();
            return false;
        }
        return (lSecuredMode.toUpper() == "SECURED");
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::SetSecuredMode(bool toSecured)
{
    if(m_pPluginID && m_pPluginID->m_pPlugin)
    {
        QString lSecuredMode = "PUBLIC";
        if(toSecured)
            lSecuredMode = "SECURED";
        if(!m_pPluginID->m_pPlugin->UpdateSecuredMode(lSecuredMode))
        {
            SetLastErrorFromPlugin();
            return false;
        }
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Returns a pointer on an options GUI, if the plugin supports custom options
//////////////////////////////////////////////////////////////////////
QWidget *GexRemoteDatabase::GetRdbOptionsGui()
{
    GSLOG(SYSLOG_SEV_DEBUG, "GexRemoteDatabase::GetRdbOptionsGui...");

    if(!m_pPluginID)
        return NULL;

    return m_pPluginID->m_pPlugin->GetRdbOptionsWidget();
}

//////////////////////////////////////////////////////////////////////
// Returns an options string, if the plugin supports custom options
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetRdbOptionsString(QString & strRdBOptionsString)
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->GetRdbOptionsString(strRdBOptionsString);
}

//////////////////////////////////////////////////////////////////////
// Sets the plug-in options GUI according to options string passed as
// argument
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::SetRdbOptionsString(const QString & strRdBOptionsString)
{
    if(!m_pPluginID)
        return false;

    return m_pPluginID->m_pPlugin->SetRdbOptionsString(strRdBOptionsString);
}

//////////////////////////////////////////////////////////////////////
// Purge selected splitlots
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::PurgeSplitlots(QStringList & strlSplitlots, QString & strTestingStage, QString & strCaseTitle, QString *pstrLog/*=NULL*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->PurgeSplitlots(strlSplitlots, strTestingStage, strCaseTitle, pstrLog);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Get global options info
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::GetGlobalOptionName(int nOptionNb, QString & strValue)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Get Global Option Name for id %1")
          .arg( nOptionNb)
          .toLatin1().constData());

    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionName(nOptionNb, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::GetGlobalOptionTypeValue(int nOptionNb, QString & strValue)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionTypeValue(nOptionNb, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::GetGlobalOptionValue(int nOptionNb, QString & strValue)
{
    bool bIsDefined;
    return GetGlobalOptionValue(nOptionNb, strValue, bIsDefined);
}

bool GexRemoteDatabase::GetGlobalOptionValue(int nOptionNb, QString & strValue,bool &bIsDefined)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionValue(nOptionNb, strValue, bIsDefined);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::GetGlobalOptionValue(QString strOptionName, QString &strValue)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionValue(strOptionName, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::SetGlobalOptionValue(QString strOptionName, QString &strValue)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->SetGlobalOptionValue(strOptionName, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::GetGlobalOptionDefaultValue(int nOptionNb, QString & strValue)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionDefaultValue(nOptionNb, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::IsGlobalOptionValidValue(int nOptionNb, QString & strValue)
{
    if(m_pPluginID == NULL)
        return false;

    if(m_pPluginID->m_pPlugin == NULL)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->IsGlobalOptionValidValue(nOptionNb, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}


bool GexRemoteDatabase::GetGlobalOptionDescription(int nOptionNb, QString & strValue)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionDescription(nOptionNb, strValue);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::GetGlobalOptionReadOnly(int nOptionNb, bool &bIsReadOnly)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->GetGlobalOptionReadOnly(nOptionNb, bIsReadOnly);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::GetTotalSize(int &size)
{
    if(!m_pPluginID)
        return false;
    bool bStatus = m_pPluginID->m_pPlugin->GetTotalSize(size);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified wafer
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ConsolidateWafer(QString & strLotID, QString & strWaferID, QString & strCaseTitle, QString *pstrLog/*=NULL*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ConsolidateWafer(strLotID, strWaferID, strCaseTitle, pstrLog);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Consolidate all wafers for specified lot
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ConsolidateWafers(QString & strLotID, QString & strCaseTitle, QString *pstrLog/*=NULL*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ConsolidateWafers(strLotID, strCaseTitle, pstrLog);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified lot
//////////////////////////////////////////////////////////////////////
bool GexRemoteDatabase::ConsolidateLot(QString & strLotID, bool bConsolidateOnlySBinTable/*=false*/, bool bCallConsolidationFunction/*=true*/)
{
    if(!m_pPluginID)
        return false;

    bool bStatus = m_pPluginID->m_pPlugin->ConsolidateLot(strLotID, bConsolidateOnlySBinTable, bCallConsolidationFunction);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}

bool GexRemoteDatabase::purgeDataBase(
        GexDbPlugin_Filter & roFilters,
        GexDbPlugin_SplitlotList &oSplitLotList)
{
    bool bStatus = m_pPluginID->m_pPlugin->purgeDataBase(roFilters, oSplitLotList);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;

}

bool GexRemoteDatabase::exportCSVCondition(const QString &strCSVFileName, const QMap<QString, QString>& oConditions, GexDbPlugin_Filter &roFilters, GexDbPlugin_SplitlotList &oSplitLotList, QProgressDialog *poProgress){
    bool bStatus = m_pPluginID->m_pPlugin->exportCSVCondition(strCSVFileName, oConditions, roFilters, oSplitLotList, poProgress);
    if(!bStatus)
        SetLastErrorFromPlugin();

    return bStatus;
}
