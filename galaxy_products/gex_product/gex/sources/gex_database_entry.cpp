
#include "gex_database_entry.h"
#include "db_external_database.h"
#include "product_info.h"
//#include "gex_constants.h"
#include <gqtl_log.h>
#include "engine.h"
#include "db_engine.h"
#include "gex_shared.h"
// LOAD-BALANCING
#include "browser_dialog.h"
#include "admin_engine.h"
#include "read_system_info.h"
#include "scheduler_engine.h"

extern GexMainwindow *	pGexMainWindow;


#if defined(unix) || __MACH__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <QDir>
#include <QDomDocument>
#include <QHostAddress>

///////////////////////////////////////////////////////////
// Database structure constructor
///////////////////////////////////////////////////////////
GexDatabaseEntry::GexDatabaseEntry(QObject* parent)
    : QObject(parent),
      m_bIsLocalDatabase(false)
{
  m_nDatabaseId   = -1;
  mStatusFlags    = 0;
  mGuiFlags       = 0;
  m_nNodeId       = -1;
  m_fCacheSize    = 0.0;
  m_fDatabaseSize = 0.0;

  m_pExternalDatabase = NULL;
  m_bIsExternal     = false;    // True if using External/corporate database
  m_bIsReadOnly     = false;    // By default, database is not Read-Only
  m_bIsCompressed   = false;
  m_bHoldsFileCopy  = false;
  m_bIsSummaryOnly  =false;
  m_bIsBlackHole    =false;

  m_clLastUpdate    = GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
  m_LastStatusUpdate= GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
  m_LastStatusChecked= GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
}

GexDatabaseEntry::GexDatabaseEntry(const GexDatabaseEntry& o): QObject(o.parent())
{
    (*this)=o;
}

///////////////////////////////////////////////////////////
// Database structure destructor
///////////////////////////////////////////////////////////
GexDatabaseEntry::~GexDatabaseEntry()
{
  // Delete remote database management object if exists.
  if(m_pExternalDatabase != NULL)
    delete m_pExternalDatabase;
  m_pExternalDatabase=NULL;
}

///////////////////////////////////////////////////////////
// Database structure assignment operator
///////////////////////////////////////////////////////////
GexDatabaseEntry& GexDatabaseEntry::operator=(const GexDatabaseEntry& source)
{
  m_nNodeId                   = source.m_nNodeId;
  m_nDatabaseId               = source.m_nDatabaseId;
  m_mapGroupPermissions       = source.m_mapGroupPermissions;
  m_lstAllowedProducts        = source.m_lstAllowedProducts;
  m_lstAllowedLots            = source.m_lstAllowedLots;

  m_bIsLocalDatabase        = source.m_bIsLocalDatabase;
  m_bHoldsFileCopy          = source.m_bHoldsFileCopy;
  m_bIsCompressed           = source.m_bIsCompressed;
  m_bIsSummaryOnly          = source.m_bIsSummaryOnly;
  m_bIsBlackHole            = source.m_bIsBlackHole;
  m_bIsExternal             = source.m_bIsExternal;
  m_bIsReadOnly             = source.m_bIsReadOnly;
  m_fCacheSize              = source.m_fCacheSize;
  m_strLogicalName          = source.m_strLogicalName;
  m_strPhysicalName         = source.m_strPhysicalName;
  m_strPhysicalPath         = source.m_strPhysicalPath;
  m_strDescription          = source.m_strDescription;
  m_strImportFilePassword   = source.m_strImportFilePassword;
  m_strDeletePassword       = source.m_strDeletePassword;
  m_strDataTypeQuery        = source.m_strDataTypeQuery;

  return *this;
}

QString GexDatabaseEntry::NormalizeDatabaseFolderName(QString &strDatabaseFolder)
{
  QString strPath;
  QString strFolder;

  // Get the end of the folder
  strPath = QDir::cleanPath(strDatabaseFolder+ QDir::separator() + "..");
  strFolder = QDir::cleanPath(strDatabaseFolder).remove(strPath);

  // Build folder name from database name...replace any invalid char with a '_'
  strFolder = strFolder.replace(' ','_').remove(QRegExp("[&'@/{}\\\\,$=!#()%.+~ -]"));

  return QDir::cleanPath(strPath + QDir::separator() +strFolder);
}


void GexDatabaseEntry::SetPhysicalPath(QString value)
{
    if (m_pExternalDatabase)
    {
        m_pExternalDatabase->m_strDatabasePhysicalPath = value;
        GexDbPlugin_ID* pluginID = m_pExternalDatabase->GetPluginID();
        if (pluginID && pluginID->m_pPlugin)
        {
            pluginID->m_pPlugin->m_strDBFolder = value;
            // Update the setting file
            if(pluginID->m_pPlugin->m_pclDatabaseConnector
                    && pluginID->m_pPlugin->m_pclDatabaseConnector->m_strSettingsFile.isEmpty())
                pluginID->m_pPlugin->m_pclDatabaseConnector->m_strSettingsFile =
                        value + QDir::separator() + QString(DB_INI_FILE);
        }
    }
    m_strPhysicalPath = value;
}

void GexDatabaseEntry::SetPhysicalName(QString value)
{
    // Replace invalid chars for folder name
    m_strPhysicalName = NormalizeDatabaseFolderName(value);
}

void GexDatabaseEntry::SetLocalDB(bool value)
{
    m_bIsLocalDatabase = value;
}

void GexDatabaseEntry::SetHoldsFileCopy(bool value)
{
    m_bHoldsFileCopy = value;
}

void GexDatabaseEntry::SetCompressed(bool value)
{
    m_bIsCompressed = value;
}

void GexDatabaseEntry::SetSummaryOnly(bool value)
{
    m_bIsSummaryOnly = value;
}

void GexDatabaseEntry::SetBlackHole(bool value)
{
    m_bIsBlackHole = value;
}

void GexDatabaseEntry::SetExternal(bool value)
{
    m_bIsExternal = value;
}

void GexDatabaseEntry::SetReadOnly(bool value)
{
    m_bIsReadOnly = value;
}

void GexDatabaseEntry::SetCacheSize(double value)
{
    m_fCacheSize = value;
}

void GexDatabaseEntry::SetDbSize(double value)
{
    m_fDatabaseSize = value;
}


void GexDatabaseEntry::SetDeletePasssword(QString value)
{
    m_strDeletePassword = value;
}

void GexDatabaseEntry::SetImportFilePasssword(QString value)
{
    m_strImportFilePassword = value;
}

void GexDatabaseEntry::SetLogicalName(QString value)
{
    m_strLogicalName = value;
    if (m_pExternalDatabase)
    {
        GexDbPlugin_ID *pPlugin = m_pExternalDatabase->GetPluginID();
        if(pPlugin && pPlugin->m_pPlugin)
        {
            pPlugin->m_pPlugin->SetTdrLinkName(m_strLogicalName);
        }
    }
}

void GexDatabaseEntry::SetDescription(QString value)
{
    m_strDescription = value;
}

void GexDatabaseEntry::SetStorageEngine(QString value)
{
    m_strStorageEngine = value;
}

QString GexDatabaseEntry::PhysicalPath() const
{
    return m_strPhysicalPath;
}

QString GexDatabaseEntry::PhysicalName() const
{
    return m_strPhysicalName;
}

QString GexDatabaseEntry::StorageType() const
{
    QString strStorage;
    if(IsSummaryOnly())
        strStorage = "Summary";         // Storage mode= Copy
    else if(IsBlackHole())
        strStorage = "BlackHole";       // Storage mode= Copy
    else if(IsCompressed())
        strStorage = "Zip";             // Storage mode= Zipped Copy
    else if(HoldsFileCopy())
        strStorage = "Copy";            // Storage mode= Copy
    else if(IsExternal())
        strStorage = "Copy";            // Storage mode= Copy
    else
        strStorage = "Link";            // Storage mode= Link

    return strStorage;
}

QString GexDatabaseEntry::GetTdrTypeName() const
{
    QString strTdrTypeName;
    if(m_pExternalDatabase)
        strTdrTypeName = m_pExternalDatabase->GetTdrTypeName();
    if(strTdrTypeName.isEmpty() || (strTdrTypeName == "Not recognized"))
    {
        strTdrTypeName = TdrTypeRecorded();
        if(strTdrTypeName == GEXDB_CHAR_TDR_KEY)
            strTdrTypeName = GEXDB_CHAR_TDR_NAME;
        else if(strTdrTypeName == GEXDB_YM_PROD_TDR_KEY)
            strTdrTypeName = GEXDB_YM_PROD_TDR_NAME;
        else if(strTdrTypeName == GEXDB_MAN_PROD_TDR_KEY)
            strTdrTypeName = GEXDB_MAN_PROD_TDR_NAME;
        else if(strTdrTypeName == GEXDB_ADR_KEY)
            strTdrTypeName = GEXDB_ADR_NAME;
        else if(strTdrTypeName == GEXDB_ADR_LOCAL_KEY)
            strTdrTypeName = GEXDB_ADR_LOCAL_NAME;
        else
            strTdrTypeName = "Not recognized";
    }

    return strTdrTypeName;
}

QString GexDatabaseEntry::IconName()
{
    QString strIcon;
    if(IsBlackHole())
    {
        if(IsStoredInDb())
            strIcon = ":/gex/icons/share.png";
        else
            strIcon = ":/gex/icons/database-blackhole.png";
    }
    else if(IsStoredInDb())
        strIcon = ":/gex/icons/database-share.png";
    else if(IsExternal())
    {
        if(IsCharacTdr())
            strIcon = ":/gex/icons/database-charac.png";
        else
            strIcon = ":/gex/icons/database.png";
    }
    else
        strIcon = ":/gex/icons/clipboard.png";

    return strIcon;
}

bool GexDatabaseEntry::IsCharacTdr()
{
    if (m_pExternalDatabase && m_pExternalDatabase->IsCharacTdr())
    {
        // Error invalid link
        if (!mTdrTypeRecorded.isEmpty() && mTdrTypeRecorded != QString(GEXDB_CHAR_TDR_KEY))
            GSLOG(SYSLOG_SEV_WARNING, QString("Database %1 doesn't' have the good TDR Type Recorded! - TDR=%2")
                   .arg(LogicalName()).arg(mTdrTypeRecorded)
                   .toLatin1().data());

        mTdrTypeRecorded = QString(GEXDB_CHAR_TDR_KEY);
        return true;
    }
    else if (mTdrTypeRecorded == QString(GEXDB_CHAR_TDR_KEY))
        return true;

    return false;
}

bool GexDatabaseEntry::IsManualProdTdr()
{
    if (m_pExternalDatabase && m_pExternalDatabase->IsManualProdTdr())
    {
        // Error invalid link
        if (!mTdrTypeRecorded.isEmpty() &&
                (mTdrTypeRecorded != QString(GEXDB_MAN_PROD_TDR_KEY)))
            GSLOG(SYSLOG_SEV_WARNING, QString("Database %1 doesn't' have the good TDR Type Recorded! - TDR=%2")
                   .arg(LogicalName()).arg(mTdrTypeRecorded)
                   .toLatin1().data());

        mTdrTypeRecorded = QString(GEXDB_MAN_PROD_TDR_KEY);
        return true;
    }
    else if (mTdrTypeRecorded == QString(GEXDB_MAN_PROD_TDR_KEY))
        return true;

    return false;
}

bool GexDatabaseEntry::IsYmProdTdr()
{
    if (m_pExternalDatabase && m_pExternalDatabase->IsYmProdTdr())
    {
        // Error invalid link
        if (!mTdrTypeRecorded.isEmpty() &&
                (mTdrTypeRecorded != QString(GEXDB_YM_PROD_TDR_KEY)))
            GSLOG(SYSLOG_SEV_WARNING, QString("Database %1 doesn't' have the good TDR Type Recorded! - TDR=%2")
                   .arg(LogicalName()).arg(mTdrTypeRecorded)
                   .toLatin1().data());

        mTdrTypeRecorded = QString(GEXDB_YM_PROD_TDR_KEY);
        return true;
    }
    else if (mTdrTypeRecorded == QString(GEXDB_YM_PROD_TDR_KEY))
        return true;

    return false;
}

bool GexDatabaseEntry::IsAdr()
{
    if (m_pExternalDatabase && m_pExternalDatabase->IsAdr())
    {
        // Error invalid link
        if (!mTdrTypeRecorded.isEmpty() &&
                (mTdrTypeRecorded != QString(GEXDB_ADR_KEY)))
            GSLOG(SYSLOG_SEV_WARNING, QString("Database %1 doesn't' have the good TDR Type Recorded! - TDR=%2")
                   .arg(LogicalName()).arg(mTdrTypeRecorded)
                   .toLatin1().data());

        mTdrTypeRecorded = QString(GEXDB_ADR_KEY);
        return true;
    }
    else if (mTdrTypeRecorded == QString(GEXDB_ADR_KEY))
        return true;

    return false;
}

bool GexDatabaseEntry::IsLocalAdr()
{
    if (m_pExternalDatabase && m_pExternalDatabase->IsLocalAdr())
    {
        // Error invalid link
        if (!mTdrTypeRecorded.isEmpty() &&
                (mTdrTypeRecorded != QString(GEXDB_ADR_LOCAL_KEY)))
            GSLOG(SYSLOG_SEV_WARNING, QString("Database %1 doesn't' have the good TDR Type Recorded! - TDR=%2")
                   .arg(LogicalName()).arg(mTdrTypeRecorded)
                   .toLatin1().data());

        mTdrTypeRecorded = QString(GEXDB_ADR_LOCAL_KEY);
        return true;
    }
    else if (mTdrTypeRecorded == QString(GEXDB_ADR_LOCAL_KEY))
        return true;

    return false;
}

bool GexDatabaseEntry::IsUnknownTdr()
{
    if (IsCharacTdr())
        return false;
    if (IsManualProdTdr())
        return false;
    if (IsYmProdTdr())
        return false;
    if (IsAdr())
        return false;
    if (IsLocalAdr())
        return false;

    // nothing found- unkown TDR DB
    return true;
}

bool GexDatabaseEntry::MustHaveAdrLink()
{
    if (m_pExternalDatabase)
        return m_pExternalDatabase->MustHaveAdrLink();

    return false;
}

bool GexDatabaseEntry::HasAdrLink()
{
    if (m_pExternalDatabase)
        return !m_pExternalDatabase->GetAdrLinkName().isEmpty();

    return false;
}

bool GexDatabaseEntry::IsSecuredDatabase()
{
    if(m_pExternalDatabase)
        return m_pExternalDatabase->IsSecuredMode();
    return false;
}

GexDbPlugin_ID * GexDatabaseEntry::GetPluginID() const
{
    if (m_pExternalDatabase)
        return m_pExternalDatabase->GetPluginID();
    else
        return NULL;
}

bool GexDatabaseEntry::ConvertFromLegacy(QString dbFilesFolder)
{
    QDir dir(dbFilesFolder);
    if (!dir.exists())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Database folder %1 doesn't' exist!")
               .arg(dir.absolutePath())
               .toLatin1().data());
        return false;
    }

    QFile entryFile(dbFilesFolder + QDir::separator() + DB_LEGACY_ENTRY_FILE);
    QFile externalFile(dbFilesFolder + QDir::separator() + DB_LEGACY_EXTERNAL_FILE);
    // Check if both files exists
    if (!entryFile.exists() && !externalFile.exists())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Nothing to convert %1 and %2 doesn't exist!")
               .arg(entryFile.fileName())
               .arg(externalFile.fileName())
               .toLatin1().data());
        return false;
    }

    if(!entryFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to open %1!")
               .arg(entryFile.fileName())
               .toLatin1().data());
        return false;
    }

    // Create new file
    QFile newIniFile(dbFilesFolder + QDir::separator() + DB_INI_FILE);
    if(!newIniFile.open( QIODevice::WriteOnly ))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to create or open %1!")
               .arg(newIniFile.fileName())
               .toLatin1().data());
        return false;
    }
    QTextStream iniTextStream(&newIniFile);

    iniTextStream << "<gex_database_entry version=\"1.0\">" << endl;

    // Read & convert DB_LEGACY_ENTRY_FILE
    QTextStream entryTextStream(&entryFile);
    while (!entryTextStream.atEnd())
    {
        QString line = entryTextStream.readLine();
        QString newLine = "\t";
        if (line.contains("="))
        {
            QString tag = line.section("=", 0, 0).trimmed();
            QString value = line.section("=", 1).trimmed();
            newLine += "\t<" + tag + ">" + value + "</" + tag + ">";
        }
        else
            newLine += line;
        iniTextStream << "\t" << newLine << endl;
    }
    entryFile.close();

    if (externalFile.exists())
    {
        if(!externalFile.open( QIODevice::ReadOnly ))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to open %1!")
                   .arg(externalFile.fileName())
                   .toLatin1().data());
            return false;
        }

        // Read & convert DB_LEGACY_EXTERNAL_FILE
        QTextStream externalTextStream(&externalFile);
        while (!externalTextStream.atEnd())
        {
            QString line = externalTextStream.readLine();
            QString newLine = "\t";
            if (line.contains("="))
            {
                QString tag = line.section("=", 0, 0).trimmed();
                QString value = line.section("=", 1).trimmed();
                newLine += "\t<" + tag + ">" + value + "</" + tag + ">";
            }
            else
                newLine += line;
            iniTextStream << newLine << endl;
        }
    }

    iniTextStream << "</gex_database_entry>" << endl;
    externalFile.close();
    newIniFile.close();

    return true;
}

bool GexDatabaseEntry::SetDbRestrictions()
{
    // TODO TDR
    // define if it has to be read only DB
    // if it has to be uploaded
    // if db id has to be be set to negative
    // if db has to be set as external or not

    // DEPENDING on DB TYPE, YM ADMIN DB,....
    return true;
}


void GexDatabaseEntry::SetExternalDbPluginIDPTR(GexDbPlugin_ID *pluginID)
{
    if (!m_pExternalDatabase)
    {
        m_pExternalDatabase = new GexRemoteDatabase();
        m_bIsExternal = true;
    }

    m_pExternalDatabase->SetPluginIDPTR(pluginID);
    // If has been set to null
    if (pluginID == NULL)
    {
        delete m_pExternalDatabase;
        m_pExternalDatabase = NULL;
    }
    else
    {
        if(pluginID->m_pPlugin)
            pluginID->m_pPlugin->SetTdrLinkName(LogicalName());
    }
}

bool GexDatabaseEntry::LoadDomElt(const QString &databasesPath,
                                      const QDomElement &node)
{
    if (node.isNull())
        return false;

    // Not mandatory, possible only if db link is stored in admin db
    LoadAdminDbFromDom(node);

    // Load database description
    if (!LoadDatabaseFromDom(node))
        return false;

    QString dbPhysPath = QDir::cleanPath(databasesPath +
                                         QDir::separator() +
                                         PhysicalName());

    if (m_bIsExternal)
    {
        // Check if license allows database plugins
        if (GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
            // Load plugin info into plugin
            if (!LoadExternalDbFromDom(node))
                return false;
        }
    }
    else
        SetConnected();

    SetPhysicalPath(dbPhysPath);

    return true;
}


bool GexDatabaseEntry::LoadFromXmlFile(QString iniFilePath)
{
    bool bStatus = false;

    QFile file(iniFilePath);
    if (!file.exists())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("%1 doesn't exists!")
               .arg(file.fileName())
               .toLatin1().data());
        return false;
    }

    if(!file.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to open %1!")
               .arg(file.fileName())
               .toLatin1().data());
        return false;
    }

    QString strErrorMsg;
    int iErrorLine, iErrorColumn;
    QDomDocument domDocument;
    if (domDocument.setContent(&file, &strErrorMsg, &iErrorLine, &iErrorColumn))
    {
        QFileInfo fileInfo(iniFilePath);
        QString databasesPath = QDir::cleanPath(fileInfo.dir().absolutePath() + QDir::separator() + "..");
        bStatus = LoadDomElt(databasesPath, domDocument.firstChildElement("gex_database_entry"));
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, QString("%1 at line %2, column %3.")
               .arg(strErrorMsg)
               .arg(QString::number(iErrorLine))
               .arg(QString::number(iErrorColumn))
              .toLatin1().constData());

    file.close();

    return bStatus;
}

bool GexDatabaseEntry::LoadFromDbFolder(QString dbFolderPath)
{
    QString iniFilePath = dbFolderPath + QDir::separator() + DB_INI_FILE;

    QFile file(iniFilePath);
    if (!file.exists())
    {
        if (!GexDatabaseEntry::ConvertFromLegacy(dbFolderPath))
        {
              GSLOG(SYSLOG_SEV_WARNING,
                QString("Unable to convert database link from %1!")
                   .arg(dbFolderPath)
                   .toLatin1().data());
            return false;
        }
    }

    return LoadFromXmlFile(iniFilePath);
}

bool GexDatabaseEntry::LoadNameFromDom(const QDomElement &node)
{
    QString lTagName = "Name";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
    {
        QString dbName = elt.text();
        m_strDatabaseRef = dbName;
        SetLogicalName(dbName);
        SetPhysicalName(dbName);
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadDescriptionFromDom(const QDomElement &node)
{
    QString lTagName = "Description";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
        m_strDescription = elt.text();
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadStorageModeFromDom(const QDomElement &node)
{
    QString lTagName = "Storage";
    QDomElement elt = node.firstChildElement(lTagName);
    if (!elt.isNull())
    {
        m_bHoldsFileCopy = false;	// Hold Zipped copy of files
        m_bIsCompressed = false;
        m_bIsSummaryOnly = false;	// Hold Zipped copy of files
        m_bIsBlackHole = false;
        //m_bIsExternal = false;
        m_bIsReadOnly = false;
        QString value = elt.text();
        if(value.startsWith("Zip") == true)
        {
            m_bHoldsFileCopy = true;	// Hold Zipped copy of files
            m_bIsCompressed = true;
        }
        else if(value.startsWith("Copy") == true)
        {
            m_bHoldsFileCopy = true;	// Hold copy of files
        }
        else if(value.startsWith("Summary") == true)
        {
            m_bHoldsFileCopy = true;
            m_bIsSummaryOnly = true;	// Hold Summary only
        }
        else if(value.startsWith("BlackHole") == true)
        {
            m_bHoldsFileCopy = true;
            m_bIsBlackHole = true;	// NO Storage!
        }
        // else is a 'Link only'
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadStorageEngineFromDom(const QDomElement &node)
{
    QString lTagName = "StorageEngine";
    QDomElement elt = node.firstChildElement(lTagName);
    m_strStorageEngine = "";
    if (!elt.isNull())
    {
        m_strStorageEngine = elt.text();
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadIsExtDbReadOnlyFromDom(const QDomElement &node)
{
    QString lTagName = "ExternalDatabaseReadOnly";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
    {
        bool lIsValidElt = true;
        QString value = elt.text();
        if(value.toInt(&lIsValidElt))
            m_bIsReadOnly = true;	// Database is READ ONLY
        else
            m_bIsReadOnly = false;	// Database is READ+WRITE
        if (!lIsValidElt)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(lTagName).toLatin1().data() );
            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadIsExternalDbFromDom(const QDomElement &node)
{
    QString lTagName = "ExternalDatabase";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
    {
        bool lIsValidElt = true;
        QString value = elt.text();
        if(value.toInt(&lIsValidElt) == 1)
            m_bIsExternal = true;
        else
            m_bIsExternal = false;
        if (!lIsValidElt)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(lTagName).toLatin1().data() );
            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadSizeFromDom(const QDomElement &node)
{
    QString lTagName = "Size";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
    {
        bool lIsValidElt = true;
        QString value = elt.text();
        m_fCacheSize = (value.toDouble(&lIsValidElt)/(1024.0*1024.0));
        if (!lIsValidElt)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(lTagName).toLatin1().data() );
            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadImportPwdFromDom(const QDomElement &node)
{
    QString lTagName = "ImportPwd";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
        m_strImportFilePassword = elt.text();
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadDeletePwdFromDom(const QDomElement &node)
{
    QString lTagName = "DeletePwd";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
        m_strDeletePassword = elt.text();
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadTdrTypeFromDom(const QDomElement &node)
{
    mTdrTypeRecorded = "";
    QDomElement lElt = node.firstChildElement("TdrType");
    if (!lElt.isNull() &&
            ((lElt.text() == QString(GEXDB_CHAR_TDR_KEY)) ||
             (lElt.text() == QString(GEXDB_MAN_PROD_TDR_KEY)) ||
             (lElt.text() == QString(GEXDB_YM_PROD_TDR_KEY)) ||
             (lElt.text() == QString(GEXDB_ADR_KEY)) ||
             (lElt.text() == QString(GEXDB_ADR_LOCAL_KEY))))
        mTdrTypeRecorded = lElt.text();
    else
        return false;

    return true;
}

bool GexDatabaseEntry::LoadDbIdFromDom(const QDomElement &node)
{
    QString lTagName = "databaseid";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
    {
        bool lIsValidElt = true;
        m_nDatabaseId = elt.text().toInt(&lIsValidElt);
        if (!lIsValidElt)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(lTagName).toLatin1().data() );
            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadNodeIdFromDom(const QDomElement &node)
{
    QString lTagName = "nodeid";
    QDomElement elt = node.firstChildElement(lTagName);

    if (!elt.isNull())
    {
        bool lIsValidElt = true;
        m_nNodeId = elt.text().toInt(&lIsValidElt);
        if (!lIsValidElt)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(lTagName).toLatin1().data() );
            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}

bool GexDatabaseEntry::LoadAdminDbFromDom(const QDomElement &node)
{
    QDomElement eltAdminDb = node.firstChildElement("admindb");
    if (!eltAdminDb.isNull())
    {
        bool lIsValidElt = true;
        if (!LoadDbIdFromDom(eltAdminDb))
            lIsValidElt = false;
        if (!LoadNodeIdFromDom(eltAdminDb))
            lIsValidElt = false;
        if (!lIsValidElt)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(eltAdminDb.tagName()).toLatin1().data() );
            return false;
        }
    }
    else
        return false;

    return true;
}

bool GexDatabaseEntry::LoadDatabaseFromDom(const QDomElement &node)
{
    QString lTagName = "database";
    QDomElement eltDb = node.firstChildElement(lTagName);

    if (!eltDb.isNull())
    {
        bool lIsValidDatabase = true;
        if (!LoadNameFromDom(eltDb))
            lIsValidDatabase = false;
        if (!LoadDescriptionFromDom(eltDb))
            lIsValidDatabase = false;
        if (!LoadIsExternalDbFromDom(eltDb))
            lIsValidDatabase = false;
        if (!LoadStorageModeFromDom(eltDb))
            lIsValidDatabase = false;
        if (!IsStoredInDb() && !LoadIsExtDbReadOnlyFromDom(eltDb))
            lIsValidDatabase = false;
        if (!IsStoredInDb() && !LoadSizeFromDom(eltDb))
            lIsValidDatabase = false;
        if (!IsStoredInDb() && !LoadImportPwdFromDom(eltDb))
            lIsValidDatabase = false;
        if (!IsStoredInDb() && !LoadDeletePwdFromDom(eltDb))
            lIsValidDatabase = false;
        LoadTdrTypeFromDom(eltDb);
        LoadStorageEngineFromDom(eltDb);
        if (!lIsValidDatabase)
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
                   arg(lTagName).toLatin1().data() );
            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to find element %1").
               arg(lTagName).toLatin1().data() );
        return false;
    }

    return true;
}


bool GexDatabaseEntry::LoadExternalDbFromDom(const QDomElement &node)
{
    m_bHoldsFileCopy = true;
    if (!node.isNull())
    {
        // LoadPlugin
        if (m_pExternalDatabase)
            delete m_pExternalDatabase;

        m_pExternalDatabase = new GexRemoteDatabase();
        if(m_pExternalDatabase->LoadPluginFromDom(node))
        {
            // Initialize and Check Remote connection
            GexDbPlugin_ID *pPlugin = m_pExternalDatabase->GetPluginID();
            if(pPlugin && pPlugin->m_pPlugin && pPlugin->m_pPlugin->m_pclDatabaseConnector)
            {
                if(pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name.toLower() == "localhost")
                    pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name
                            = GS::Gex::Engine::GetInstance().GetSystemInfo().strHostName;

                // Try to resolve host name domain<->IP
                // Make sure server name is in IP format
                QString strHostName = "???";
                QString strHostIP = "???";
                QString strHostNameIP = pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name;

                QHostAddress clHostAddr;
                if(clHostAddr.setAddress(strHostNameIP) == false)
                {
                    // The server name is in domain name format.
                    strHostName = strHostNameIP;
                }
                else
                {
                    // The server name is in IP address format. Try to retrieve the server name.
                    strHostIP = strHostNameIP;
                    unsigned long ul_HostAddress = (unsigned long)clHostAddr.toIPv4Address();
                    ul_HostAddress = ntohl(ul_HostAddress);
                    char *pHostAddress = (char *)&ul_HostAddress;
                    struct hostent *host = NULL;
                    host = gethostbyaddr(pHostAddress, 4, AF_INET);
                    if(host)
                        strHostName = host->h_name;
                    else
                        strHostName = strHostIP;	// Server name not found. Use the IP address
                }

                if(strHostNameIP.toUpper() != strHostName.toUpper())
                    pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name = strHostName;

                pPlugin->m_pPlugin->SetTdrLinkName(LogicalName());
            }
        }
        else
        {
            QString strError;
            m_pExternalDatabase->GetLastError(strError);
            delete m_pExternalDatabase;
            m_pExternalDatabase = NULL;
            GSLOG(SYSLOG_SEV_WARNING, QString("cant load plugin : %1").
                   arg(strError).toLatin1().data() );
            mStatusFlags = 0;
            mGuiFlags = 0;
            return false;
        }
    }
    else
        return false;

    return true;
}

bool GexDatabaseEntry::SetConnected()
{
    SetStatusFlags(STATUS_CONNECTED);

    return true;
}

QDomElement GexDatabaseEntry::GetDomElt(QDomDocument &domDoc) const
{
    QDomElement domEltRoot = domDoc.createElement("gex_database_entry");

    // dbadmin node
    if (IsStoredInDb())
        domEltRoot.appendChild(GetAdminDbDom(domDoc));

    // database node
    domEltRoot.appendChild(GetDatabaseDom(domDoc));


    // ExternalDatabase node
    domEltRoot.appendChild(GetExternalDbDom(domDoc));

    return domEltRoot;
}

bool GexDatabaseEntry::CopyToDatabasesFolder(const QString &filePath,
                                             const QString &databasesFolder)
{
    // Move the file into the right folder
    QString lDbName = PhysicalName();
    QString lDbFolder = QDir::cleanPath(databasesFolder +
                                       QDir::separator() +
                                       lDbName);
    QDir dir(lDbFolder);
    if (dir.exists())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Can't store this link, database folder %1 already exist!")
               .arg(dir.absolutePath())
               .toLatin1().data());
        return false;
    }

    if (!dir.mkpath(lDbFolder))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to create %1!")
               .arg(dir.absolutePath())
               .toLatin1().data());
        return false;
    }

    QString newFile = QDir::cleanPath(lDbFolder +
                                      QDir::separator() +
                                      DB_INI_FILE);
    if (!QFile::copy(filePath, newFile))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to copy %1 to %2!")
               .arg(filePath)
               .arg(newFile)
               .toLatin1().data());
        return false;
    }
    SetPhysicalPath(lDbFolder);
    return true;
}


QDomElement GexDatabaseEntry::GetAdminDbDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt;
    QDomElement domEltAdminDb = doc.createElement("admindb");
    // DB ID
    domElt = doc.createElement("databaseid");
    domText = doc.createTextNode(QString::number(m_nDatabaseId));
    domElt.appendChild(domText);
    domEltAdminDb.appendChild(domElt);
    // Node ID
    domElt = doc.createElement("nodeid");
    domText = doc.createTextNode(QString::number(m_nNodeId));
    domElt.appendChild(domText);
    domEltAdminDb.appendChild(domElt);

    return domEltAdminDb;
}

QDomElement GexDatabaseEntry::GetDatabaseDom(QDomDocument &doc) const
{
    QDomElement domEltDatabase = doc.createElement("database");
    // name node
    domEltDatabase.appendChild(GetNameDom(doc));
    // desc node
    domEltDatabase.appendChild(GetDescriptionDom(doc));
    // TdrType node
    domEltDatabase.appendChild(GetTdrTypeDom(doc));
    // Storage node
    domEltDatabase.appendChild(GetStorageModeDom(doc));
    // Storage Engine node
    domEltDatabase.appendChild(GetStorageEngineDom(doc));
    // ExternalDatabase node
    domEltDatabase.appendChild(GetIsExternalDbDom(doc));
    if (!IsStoredInDb())
    {
        // ExternalDatabaseReadOnly node
        domEltDatabase.appendChild(GetIsExtDbReadOnlyDom(doc));
        // Size node
        domEltDatabase.appendChild(GetSizeDom(doc));
        // ImportPwd node
        domEltDatabase.appendChild(GetImportPwdDom(doc));
        // DeletePwd node
        domEltDatabase.appendChild(GetDeletePwdDom(doc));
    }

    return domEltDatabase;
}

QDomElement GexDatabaseEntry::GetNameDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Name");
    domText = doc.createTextNode(LogicalName());
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetDescriptionDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Description");
    domText = doc.createTextNode(m_strDescription);
    domElt.appendChild(domText);

    return domElt;
}


QDomElement GexDatabaseEntry::GetStorageModeDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Storage");
    domText = doc.createTextNode(m_strDescription);
    if(m_bIsSummaryOnly == true)
        domText = doc.createTextNode("Summary");		// Storage mode= Copy
    else if(m_bIsBlackHole == true)
        domText = doc.createTextNode("BlackHole");		// Storage mode= Copy
    else if(m_bIsCompressed == true)
        domText = doc.createTextNode("Zip");		// Storage mode= Zipped Copy
    else if(m_bHoldsFileCopy == true)
        domText = doc.createTextNode("Copy");		// Storage mode= Copy
    else
        domText = doc.createTextNode("Link");		// Storage mode= Link
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetStorageEngineDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("StorageEngine");
    domText = doc.createTextNode(m_strStorageEngine);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetIsExtDbReadOnlyDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("ExternalDatabaseReadOnly");
    if(m_bIsExternal && m_bIsReadOnly)
        domText = doc.createTextNode("1");
    else
        domText = doc.createTextNode("0");
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetIsExternalDbDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("ExternalDatabase");
    if(m_bIsExternal)
        domText = doc.createTextNode("1");
    else
        domText = doc.createTextNode("0");
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetSizeDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("Size");
    domText = doc.createTextNode(QString::number(m_fCacheSize*(1024.0*1024.0)));
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetImportPwdDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("ImportPwd");
    domText = doc.createTextNode(m_strImportFilePassword);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetDeletePwdDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt = doc.createElement("DeletePwd");
    domText = doc.createTextNode(m_strDeletePassword);
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDatabaseEntry::GetTdrTypeDom(QDomDocument &doc) const
{
    QDomText lDomText;
    QDomElement lDomElt = doc.createElement("TdrType");
    QString lTrdType = "";

    if (m_pExternalDatabase)
    {
        GexDbPlugin_ID * pPluginId = m_pExternalDatabase->GetPluginID();
        if (pPluginId && pPluginId->m_pPlugin)
        {
            if (m_pExternalDatabase->IsCharacTdr())
                lTrdType = QString(GEXDB_CHAR_TDR_KEY);
            else if (m_pExternalDatabase->IsManualProdTdr())
                lTrdType = QString(GEXDB_MAN_PROD_TDR_KEY);
            else if (m_pExternalDatabase->IsYmProdTdr())
                lTrdType = QString(GEXDB_YM_PROD_TDR_KEY);
            else if (m_pExternalDatabase->IsAdr())
                lTrdType = QString(GEXDB_ADR_KEY);
            else if (m_pExternalDatabase->IsLocalAdr())
                lTrdType = QString(GEXDB_ADR_LOCAL_KEY);
        }
    }
    if (lTrdType.isEmpty() && !mTdrTypeRecorded.isEmpty())
        lTrdType = mTdrTypeRecorded;

    lDomText = doc.createTextNode(lTrdType);
    lDomElt.appendChild(lDomText);

    return lDomElt;
}

QDomElement GexDatabaseEntry::GetExternalDbDom(QDomDocument &doc) const
{
    QDomElement domEltExtDatabase;
    GexDbPlugin_ID * pPluginId = NULL;
    if (m_pExternalDatabase)
    {
        pPluginId = m_pExternalDatabase->GetPluginID();
        if (pPluginId && pPluginId->m_pPlugin)
        {
            domEltExtDatabase = doc.createElement("ExternalDatabase");
            domEltExtDatabase.appendChild(pPluginId->m_pPlugin->GetSettingsDom(doc));
            // Node identification
            QDomElement domEltIdentification = doc.createElement("Identification");
            QDomElement domElt;
            QDomText domText;
            // File
            domElt = doc.createElement("PluginFile");
            domText = doc.createTextNode(pPluginId->pluginFileName().
                                         replace("d.","."));
            domElt.appendChild(domText);
            domEltIdentification.appendChild(domElt);
            // Name
            domElt = doc.createElement("PluginName");
            domText = doc.createTextNode(pPluginId->pluginName());
            domElt.appendChild(domText);
            domEltIdentification.appendChild(domElt);
            // Build
            domElt = doc.createElement("PluginBuild");
            domText = doc.createTextNode(QString::number(pPluginId->pluginBuild()));
            domElt.appendChild(domText);
            domEltIdentification.appendChild(domElt);

            domEltExtDatabase.appendChild(domEltIdentification);
        }
    }

    return domEltExtDatabase;
}

bool GexDatabaseEntry::SaveToXmlFile(const QString &destFile) const
{
    QString destinationFile = destFile;
    if (destinationFile.isEmpty())
        destinationFile = QDir::cleanPath(m_strPhysicalPath + QDir::separator() + DB_INI_FILE);

    QFile file(destinationFile);
    QFileInfo fileInfo(file);
    QDir dir;

    // Check dir & create it if needed
    if (!fileInfo.dir().exists())
    {
        if (!dir.mkpath(fileInfo.dir().absolutePath()))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to create  %1!")
                   .arg(fileInfo.dir().absolutePath())
                   .toLatin1().data());
            return false;
        }
    }
    // Check file & create it if needed
    if (!file.open(QIODevice::WriteOnly))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to open %1!")
               .arg(file.fileName())
               .toLatin1().data());
        return false;
    }

    // Create dom document
    QDomDocument domDocument;
    QDomElement elmtRoot = GetDomElt(domDocument);
    if (elmtRoot.isNull())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Root Dom Element is NULL!")
               .toLatin1().data());
        return false;
    }
    // Set version
    elmtRoot.setAttribute("version", "1.0");
    domDocument.appendChild(elmtRoot);
    QTextStream textStream(&file);
    // Save dom doc to file
    domDocument.save(textStream, 4);
    file.close();
    return true;
}

bool GexDatabaseEntry::IncrementalUpdate(QString incrementalName, QString testingStage, QString target, QMap< QString, QString >  &summary)
{
    bool lStatus = true;
    QMap< QString, QString >  lCommand;
    lCommand["db_update_name"]=summary["db_update_name"]=incrementalName;
    lCommand["testing_stage"]=summary["testing_stage"]=testingStage;
    lCommand["target"]=summary["target"]=target;

    TraceExecution(GS::Gex::Engine::GetInstance().GetAdminEngine().JoinCommand(lCommand),
                   "START",
                   GS::Gex::Engine::GetInstance().GetAdminEngine().JoinCommand(summary));

    // Initialize
    summary["status"] = QString::number(GS::Gex::DatabaseEngine::Passed);
    summary.remove("error");
    summary.remove("errorcode");
    summary.remove("errorshortdesc");

    // For TDR with ADR
    // Before to start the consolidation
    // Check if have all what we need
    bool lPostJobProcess = (incrementalName == "AGENT_WORKFLOW") && MustHaveAdrLink();
    QString lAdrLink;
    if(lPostJobProcess)
    {
        if(m_pExternalDatabase)
            lAdrLink = m_pExternalDatabase->GetAdrLinkName();
        if(lAdrLink.isEmpty())
        {
            // Error: no Adr found
            summary["status"] = QString::number(GS::Gex::DatabaseEngine::Failed);
            summary["error"] = "No ADR link";
            summary["errorcode"] = "9999";
            summary["errorshortdesc"] = "No ADR link";
            lStatus = false;
        }
    }


    if(lStatus)
    {
        lStatus = m_pExternalDatabase->IncrementalUpdate(incrementalName,testingStage,target,summary);

        // Map the status to the enum GS::Gex::DatabaseEngine::ExecutionStatus
        // and update ErrorCode
        if(lStatus && (summary["status"] == "PASS"))
        {
            summary["status"] = QString::number(GS::Gex::DatabaseEngine::Passed);
        }
        else
        {
            if(summary["status"] == "DELAY")
            {
                summary["status"] = QString::number(GS::Gex::DatabaseEngine::Delay);
            }
            else
            {
                summary["status"] = QString::number(GS::Gex::DatabaseEngine::Failed);
            }
            summary["error"] = m_pExternalDatabase->GetPluginErrorMsg();
            summary["errorcode"] = m_pExternalDatabase->GetPluginErrorCode();
            summary["errorshortdesc"] = m_pExternalDatabase->GetPluginErrorDescription();
        }
    }

    TraceExecution(GS::Gex::Engine::GetInstance().GetAdminEngine().JoinCommand(lCommand),
                   "STOP",
                   GS::Gex::Engine::GetInstance().GetAdminEngine().JoinCommand(summary));

    return lStatus;
}

bool GexDatabaseEntry::TraceExecution(QString Command, QString Status, QString Summary)
{

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && IsStoredInDb())
    {
        // Process contains the command ligne
        // Arg1=v1|Arg2=v2|...
        QString ExecutionStatus = Status;
        QString ExecutionSummary = Summary;
        QString ExecutionComment;
        if(mActionMngAttributes.contains("ActionId"))
        {
            // Include the ActionId
            ExecutionComment = "action_id=" + mActionMngAttributes["ActionId"];
        }

        if(Status == "START")
            ExecutionStatus = Status;
        else if(Status.isEmpty()
                || (Status == "STOP")
                || Status.startsWith("ok:",Qt::CaseInsensitive))
            ExecutionStatus = "PASS";
        else if(Status.startsWith("delay:",Qt::CaseInsensitive))
            ExecutionStatus = "DELAY";
        else
            ExecutionStatus = "FAIL";

        if(Status.startsWith("ok:",Qt::CaseInsensitive)
                || Status.startsWith("delay:",Qt::CaseInsensitive)
                || Status.startsWith("error:",Qt::CaseInsensitive)){
            ExecutionSummary = Status.section(":",1);
        }

        if((Status != "START")
                && Summary.contains("Status=",Qt::CaseInsensitive))
        {
            QMap<QString,QString> mapSummary = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Summary.toLower());
            QString StatusNumber = mapSummary["status"].toUpper();
            if(StatusNumber == "0") ExecutionStatus = "PASS";
            else if(StatusNumber == "1") ExecutionStatus = "FAIL";
            else if(StatusNumber == "2") ExecutionStatus = "FAIL";
            else if(StatusNumber == "3") ExecutionStatus = "DELAY";
            else if(StatusNumber == "PASS") ExecutionStatus = "PASS";
            else if(StatusNumber == "DELAY") ExecutionStatus = "DELAY";
            else if(StatusNumber == "FAIL") ExecutionStatus = "FAIL";
            ExecutionSummary = Summary;

            if((ExecutionStatus == "PASS")
                    && (mapSummary.contains("info") && !mapSummary["info"].isEmpty()))
            {
                GS::Gex::Engine::GetInstance().GetAdminEngine()
                        .AddNewEvent("EXECUTION","INCREMENTAL_UPDATE",0
                                     ,0,m_nDatabaseId
                                     ,Command,"WARNING",mapSummary["info"],"");
            }
        }

        if((ExecutionStatus == "FAIL") && !ExecutionSummary.contains("error=",Qt::CaseInsensitive))
            ExecutionSummary = "error="+ExecutionSummary;

        GS::Gex::Engine::GetInstance().GetAdminEngine()
                .AddNewEvent("EXECUTION","INCREMENTAL_UPDATE",0
                             ,0,m_nDatabaseId
                             ,Command,ExecutionStatus,ExecutionSummary,ExecutionComment);
    }
    return true;

}

bool GexDatabaseEntry::TraceUpdate(QString Command,QString Status,QString Summary)
{
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && IsStoredInDb())
    {
        // Update the Events flow
        GS::Gex::Engine::GetInstance().GetAdminEngine()
                .AddNewEvent("UPDATE","DATABASE",
                              0,0,m_nDatabaseId,Command,
                              Status,Summary,"");

        if((Command == "DELETE") && (Status == "PASS"))
            GS::Gex::Engine::GetInstance().GetAdminEngine()
                    .CleanActions("mutex IS NULL AND database_id="+QString::number(m_nDatabaseId));

    }
    return true;
}


bool GexDatabaseEntry::TraceInfo()
{
    QDateTime cCurrentDateTime=GS::Gex::Engine::GetInstance().GetClientDateTime();
    QString strErrorMessage = cCurrentDateTime.toString("[d MMMM yyyy h:mm:ss] ");
    strErrorMessage	+= "["+LogicalName()+"] - "+ m_strLastInfoMsg + "\n";

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog(strErrorMessage);

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && IsStoredInDb())
    {
        // Update the Events flow
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewEvent("UPDATE","DATABASE",
                                                       0,0,m_nDatabaseId,"",
                                                       "INFO","["+LogicalName()+"] - "+ m_strLastInfoMsg,"");
    }
    return true;
}


bool GexDatabaseEntry::SetActionMng(int ActionId,QString Command)
{
    mActionMngAttributes.clear();
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && ActionId > 0)
    {
        mActionMngAttributes = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Command);
        mActionMngAttributes["ActionId"] = QString::number(ActionId);
    }
    return true;
}

bool GexDatabaseEntry::UpdateActionMng(QString /*Summary*/)
{
    return true;
}

QString GexDatabaseEntry::GetActionMngAttribute(QString key)
{
    QString value;

    // Case insensitive
    if(mActionMngAttributes.contains(key))
        value = mActionMngAttributes[key];
    else if(mActionMngAttributes.contains(key.toLower()))
        value = mActionMngAttributes[key.toLower()];

    return value;
}
