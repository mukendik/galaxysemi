// gexdb_plugin_base.cpp: implementation of the GexDbPlugin_Base class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes:
//
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_common.h"
#include "gexdb_plugin_base.h"
#include "gexdb_plugin_filetransfer_dialog.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "db_architecture.h"
#include "query_engine.h"
#include "query_progress_core.h"
#include "query_progress_gui.h"
#include "abstract_query_progress.h"
#include "product_info.h"

// Standard includes
#include <time.h>

// Qt includes

#include <QtGlobal>
#include <qfile.h>
#include <qsqlrecord.h>
#include <qwidget.h>
#include <qregexp.h>
#include <qlocale.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <QSqlError>
#include <QSqlDriver>
#include <QDir>

// Galaxy modules includes
#include <gstdl_crypto.h>
#include <gstdl_membuffer.h>
#include <gqtl_sysutils.h>
#include <gex_scriptengine.h>

#ifndef QT_DEBUG
#define PROCESS_EVENTS_INTERVAL                100        // Min interval between processEvents calls (milliseconds)
#else
#define PROCESS_EVENTS_INTERVAL                200       // Lets boost in order to insert faster
#endif

/////////////////////////////////////////////////////////////////////////////////////
// Key for encrypting password
/////////////////////////////////////////////////////////////////////////////////////
#define GEXDB_PLUGIN_CRYPTING_KEY	"gex@galaxysemi.com|gex|10-Jan-2006"

/////////////////////////////////////////////////////////////////////////////////////////
// SplitlotList OBJECT
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Compares 2 items of the Splitlot list
/////////////////////////////////////////////////////////////////////////////////////////
bool CompareSplitInfoOnStartAscending(GexDbPlugin_SplitlotInfo *splitlot1, GexDbPlugin_SplitlotInfo *splitlot2)
{
    return (splitlot1->m_uiStartTime > splitlot2->m_uiStartTime);
}

bool CompareSplitInfoOnStartDescending(GexDbPlugin_SplitlotInfo *splitlot1, GexDbPlugin_SplitlotInfo *splitlot2)
{
    return(splitlot1->m_uiStartTime < splitlot2->m_uiStartTime);
}

bool CompareSplitInfoOnLotIdAscending(GexDbPlugin_SplitlotInfo *splitlot1, GexDbPlugin_SplitlotInfo *splitlot2)
{
    return (splitlot1->m_strLotID > splitlot2->m_strLotID);
}

bool CompareSplitInfoOnLotIdDescending(GexDbPlugin_SplitlotInfo *splitlot1, GexDbPlugin_SplitlotInfo *splitlot2)
{
    return (splitlot1->m_strLotID < splitlot2->m_strLotID);
}



GexDbPlugin_ER_Parts_SerieData::~GexDbPlugin_ER_Parts_SerieData()
{
    clear();
//        while (!this->isEmpty())
//             delete this->takeFirst();
}



GexDbPlugin_ER_Parts_Graph::GexDbPlugin_ER_Parts_Graph(QStringList & strlGraphSplitValues)
{
    m_strlGraphSplitValues = strlGraphSplitValues;
}

GexDbPlugin_ER_Parts_Graph::~GexDbPlugin_ER_Parts_Graph()
{
//    while (!this->isEmpty())
//    {
//        GexDbPlugin_ER_Parts_Layer* firstItem = this->at(0);
//        if (firstItem && !firstItem->isEmpty())
//            delete firstItem;
//        else
//            this->takeFirst();
//    }
//    clear();
}

GexDbPlugin_ER_Parts_Layer::GexDbPlugin_ER_Parts_Layer(unsigned int uiSeries, QStringList & strlLayerSplitValues)
{
    m_uiDataPoints = 0;
    m_uiSeries = 0;
    m_strlLayerSplitValues = strlLayerSplitValues;

    unsigned int                    uiIndex;
    GexDbPlugin_ER_Parts_SerieData  *pSerieData;
    for(uiIndex=0; uiIndex<uiSeries; uiIndex++)
    {
        pSerieData = new GexDbPlugin_ER_Parts_SerieData();
        append(pSerieData);
        m_uiSeries++;
    }
}
GexDbPlugin_ER_Parts_Layer::~GexDbPlugin_ER_Parts_Layer()
{
//    while (!this->isEmpty())
//         delete this->takeFirst();
//    clear();
}

// Add data for 1 specific aggregate value
void GexDbPlugin_ER_Parts_Layer::Add(const QString & strLabel, unsigned int uiNbParts, unsigned int uiNbParts_Good, double lfTestTime, QList<unsigned int> & uilMatchingParts)
{
    unsigned int                    uiIndex;
    GexDbPlugin_ER_Parts_SerieData  *pSerieData;

    if(((unsigned int)uilMatchingParts.count()) != m_uiSeries)
        return;

    m_strlAggregateLabels.append(strLabel);
    m_uilNbParts.append(uiNbParts);
    m_uilNbParts_Good.append(uiNbParts_Good);
    m_lflTestTime.append(lfTestTime);
    for(uiIndex=0; uiIndex<m_uiSeries; uiIndex++)
    {
        pSerieData = at(uiIndex);
        pSerieData->Append(uilMatchingParts[uiIndex], uiNbParts);
    }
    m_uiDataPoints++;
}
// Add data for 1 specific aggregate value
void GexDbPlugin_ER_Parts_Layer::Add(const QString & strLabel, QList<unsigned int> & uilNbParts, QList<unsigned int> & uilMatchingParts)
{
    unsigned int                    uiIndex;
    GexDbPlugin_ER_Parts_SerieData  *pSerieData;

    if(((unsigned int)uilMatchingParts.count()) != m_uiSeries)
        return;
    if(((unsigned int)uilNbParts.count()) != m_uiSeries)
        return;

    m_strlAggregateLabels.append(strLabel);
    m_uilNbParts.append(0);
    m_uilNbParts_Good.append(0);
    m_lflTestTime.append(0.0);
    for(uiIndex=0; uiIndex<m_uiSeries; uiIndex++)
    {
        pSerieData = at(uiIndex);
        pSerieData->Append(uilMatchingParts[uiIndex], uilNbParts[uiIndex]);
    }
    m_uiDataPoints++;
}

// Add data for 1 specific aggregate value. This function is used for Yield vs Parameter scatter
// Serie 0 = Parameter
// Serie 1 = Yield
void GexDbPlugin_ER_Parts_Layer::Add(const QString & strLabel, unsigned int uiParameterExecs, double lfParameterSum, unsigned int uiTotalParts, unsigned int uiMatchingParts)
{
    GexDbPlugin_ER_Parts_SerieData   *pSerieData;

    m_strlAggregateLabels.append(strLabel);
    m_uilNbParts.append(0);
    m_uilNbParts_Good.append(0);
    m_lflTestTime.append(0.0);
    pSerieData = at(0);
    pSerieData->Append(uiParameterExecs, lfParameterSum);
    pSerieData = at(1);
    pSerieData->Append(uiMatchingParts, uiTotalParts);
    m_uiDataPoints++;
}

// Update data (nb splitlots, wafers, lots, Maverick wafers, Maverick lots) for 1 specific aggregate value
void GexDbPlugin_ER_Parts_Layer::Update(const QString & strLabel, unsigned int uiNbSplitlots, unsigned int uiNbWafers, unsigned int uiNbLots, unsigned int uiNbMaverickWafers, unsigned int uiNbMaverickLots)
{
    int nIndex = m_strlAggregateLabels.indexOf(strLabel);
    if(nIndex >= 0)
    {
        m_uilNbSplitlots.append(uiNbSplitlots);
        m_uilNbWafers.append(uiNbWafers);
        m_uilNbLots.append(uiNbLots);
        m_uilNbMaverickWafers.append(uiNbMaverickWafers);
        m_uilNbMaverickLots.append(uiNbMaverickLots);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Constructor
//
// Argument(s) :
//
// Return type :
/////////////////////////////////////////////////////////////////////////////////////////
GexDbPlugin_SplitlotList::GexDbPlugin_SplitlotList(): QList<GexDbPlugin_SplitlotInfo*>()
{
    mSortSelector = eSortOnStart_t;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Description : Destructor
/////////////////////////////////////////////////////////////////////////////////////////
GexDbPlugin_SplitlotList::~GexDbPlugin_SplitlotList()
{
//    while (!this->isEmpty())
//         delete this->takeFirst();
    clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description : Sort list
//
// Argument(s) :
//
// Return type : void
/////////////////////////////////////////////////////////////////////////////////////////
void GexDbPlugin_SplitlotList::Sort(SortOn eSortSelector, bool ascending)
{
    // Set sort parameters
    mSortSelector = eSortSelector;
//    mSortAscending = ascending;

    // Now sort
    if(mSortSelector == eSortOnLotID)
    {
        if (ascending)
            qSort(begin(),end(), CompareSplitInfoOnStartAscending);
        else
            qSort(begin(),end(), CompareSplitInfoOnStartDescending);
    }
    else if (mSortSelector == eSortOnStart_t)
    {
        if (ascending)
            qSort(begin(),end(), CompareSplitInfoOnLotIdAscending);
        else
            qSort(begin(),end(), CompareSplitInfoOnLotIdDescending);
    }
}


///////////////////////////////////////////////////////////
// GexDbPlugin_FtpSettings class: ftp settings...
///////////////////////////////////////////////////////////

// Error map
GBEGIN_ERROR_MAP(GexDbPlugin_FtpSettings)
GMAP_ERROR(eMarkerNotFound,"Marker not found: %s.")
GEND_ERROR_MAP(GexDbPlugin_FtpSettings)

///////////////////////////////////////////////////////////
// Constructor / Destructor / Operators
///////////////////////////////////////////////////////////
GexDbPlugin_FtpSettings::GexDbPlugin_FtpSettings()
{
    m_uiPort = 21;
    m_bHostnameFromDbField = false;
}

GexDbPlugin_FtpSettings::~GexDbPlugin_FtpSettings()
{
}

GexDbPlugin_FtpSettings& GexDbPlugin_FtpSettings::operator=(const GexDbPlugin_FtpSettings& source)
{
    m_strHostName			= source.m_strHostName;
    m_strUserName			= source.m_strUserName;
    m_strPassword			= source.m_strPassword;
    m_uiPort				= source.m_uiPort;
    m_strPath				= source.m_strPath;
    m_bHostnameFromDbField	= source.m_bHostnameFromDbField;

    return *this;
}

///////////////////////////////////////////////////////////
// Load settings from file and init the calls variables
///////////////////////////////////////////////////////////
bool GexDbPlugin_FtpSettings::LoadSettings(QFile *pSettingsFile)
{
    // Assign file I/O stream
    QString		strString;
    QString		strKeyword;
    QString		strParameter;
    QTextStream hFile(pSettingsFile);

    // Rewind file first
    pSettingsFile->reset();

    // Search for marker used by this object's settings
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(strString.toLower() == "<ftpsettings>")
            break;
    }
    if(hFile.atEnd())
    {
        GSET_ERROR1(GexDbPlugin_FtpSettings, eMarkerNotFound, NULL, "<FtpSettings>");
        return false;	// Failed reading file.
    }

    while(!hFile.atEnd())
    {
        // Read line.
        strString = hFile.readLine().trimmed();
        strKeyword = strString.section('=',0,0);
        strParameter = strString.section('=',1);

        if(strString.toLower() == "</ftpsettings>")
            break;

        if(strKeyword.toLower() == "host")
        {
            m_strHostName = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "hostfromdb")
        {
            m_bHostnameFromDbField = strParameter.toInt() ? true : false;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "port")
        {
            m_uiPort = strParameter.toUInt();;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "username")
        {
            m_strUserName = strParameter;
            goto next_line_1;
        }

        if(strKeyword.toLower() == "userpassword")
        {
            // Decrypt password
            GexDbPlugin_Base::DecryptPassword(strParameter, m_strPassword);
            goto next_line_1;
        }

        if(strKeyword.toLower() == "path")
        {
            m_strPath = strParameter;
            goto next_line_1;
        }

next_line_1:
        strString = "";
    }

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Write settings to file using informations loaded in the class variables
///////////////////////////////////////////////////////////
bool GexDbPlugin_FtpSettings::WriteSettings(QTextStream *phFile)
{
    // Encrypt passwords
    QString strCryptedPassword = "";
    if(!m_strPassword.isEmpty())
        GexDbPlugin_Base::CryptPassword(m_strPassword, strCryptedPassword);

    // Write Ftp settings to file...
    *phFile << endl;
    *phFile << "<FtpSettings>" << endl;
    *phFile << "Host=" << m_strHostName << endl;
    *phFile << "HostFromDB=" << (m_bHostnameFromDbField ? 1 : 0) << endl;
    *phFile << "Port=" << m_uiPort << endl;
    *phFile << "UserName=" << m_strUserName << endl;
    *phFile << "UserPassword=" << strCryptedPassword << endl;
    *phFile << "Path=" << m_strPath << endl;
    *phFile << "</FtpSettings>" << endl;

    // Success
    return true;
}




//////////////////////////////////////////////////////////////////////
// Returns nb of micro-sec since internal timer started
//////////////////////////////////////////////////////////////////////
float GexDbPlugin_Query::Elapsed(float *pfQueryMainTimer/*=NULL*/)
{
#ifdef _WIN32
    if(m_bPerformanceCounterSupported)
    {
        LARGE_INTEGER	liPerformanceCounter;
        float			fElapsed;
        QueryPerformanceCounter(&liPerformanceCounter);
        if(pfQueryMainTimer)
        {
            fElapsed = (float)(liPerformanceCounter.QuadPart - m_liPerformanceCounter_Query.QuadPart);
            fElapsed *= 1000000.0F;
            fElapsed /= (float)m_ulPerformanceFrequency;
            *pfQueryMainTimer = fElapsed;
        }
        fElapsed = (float)(liPerformanceCounter.QuadPart - m_liPerformanceCounter_Ref.QuadPart);
        fElapsed *= 1000000.0F;
        fElapsed /= (float)m_ulPerformanceFrequency;
        return fElapsed;
    }
    else
    {
        if(pfQueryMainTimer)
            *pfQueryMainTimer = (float)m_clQueryTimer_Query.elapsed()*1000.0F;
        return (float)m_clQueryTimer_Ref.elapsed()*1000.0F;
    }
#else
    if(pfQueryMainTimer)
        *pfQueryMainTimer = (float)m_clQueryTimer_Query.elapsed()*1000.0F;
    return (float)m_clQueryTimer_Ref.elapsed()*1000.0F;
#endif
}

//////////////////////////////////////////////////////////////////////
// Prepare specified query
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Query::Prepare(const QString & strQuery)
{
    QString	strDebugMessage;

    m_strQuery = strQuery;

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
        StartTimer(false);

    // Busy cursor
    //qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    if (m_pPluginBase)
        emit m_pPluginBase->sBusy(true);

    // Exec query
    bool bStatus = prepare(strQuery);

    // Restore cursor
    //qApp->restoreOverrideCursor();
    if (m_pPluginBase)
        emit m_pPluginBase->sBusy(false);

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        float fTimer = Elapsed();
        if(bStatus)
            strDebugMessage = "Prepare SQL query: OK (";
        else
            strDebugMessage = "Prepare SQL query: NOK (";
        strDebugMessage += QString::number((int)(fTimer/1000.0F)) + " ms)";
        m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
    }

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Execute specified query
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Query::Execute()
{
    QString	strDebugMessage;

    // Debug mode ?
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        // put NO_QUERY_DUMP in your user_custom.pri
#ifndef NO_QUERY_DUMP
        strDebugMessage = "SQL query:";
        strDebugMessage += m_strQuery;
        m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
#endif
        StartTimer(true);
    }

    // Busy cursor
    //qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    if (m_pPluginBase)
        emit m_pPluginBase->sBusy(true);

    // Exec query
    bool bStatus = exec();
    if(!bStatus && m_pPluginBase->m_pclDatabaseConnector->IsMySqlDB())
    {
        GSLOG(SYSLOG_SEV_ERROR,( QString("ERROR QUERY=%1 ERRORNB=%2 ERRORMSG=%3")
                  .arg(lastQuery().toLatin1().constData())
                  .arg(lastError().number())
                  .arg(lastError().text().toLatin1().constData())).toLatin1().constData());

        // DeadLock catch
        if(lastError().number() == 1213)
        {
            // DeadLock Error: 1213 SQLSTATE: 40001 (ER_LOCK_DEADLOCK)
            // Deadlock found when trying to get lock; try restarting transaction
            GSLOG(SYSLOG_SEV_ERROR,"DEADLOCK DETECTED");
            // Dump MySql DeadLock Status
            // Use a new SqlQuery
            QSqlQuery clQuery(QSqlDatabase::database(m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName));
            clQuery.exec("show engine innodb status");
            clQuery.first();
            if(!clQuery.value(2).toString().isEmpty())
            {
            GSLOG(SYSLOG_SEV_DEBUG, (QString("QUERY=%1 STATUS=%2").arg(
                     clQuery.lastQuery().toLatin1().constData())
                     .arg(clQuery.value(2).toString().toLatin1().constData())).toLatin1().constData());
            }
        }
        else if(lastError().number() == 1172)
        {
            // Error: 1172 SQLSTATE: 42000 (ER_TOO_MANY_ROWS)
            // Message: Result consisted of more than one row
            GSLOG(SYSLOG_SEV_ERROR,"TOO MANY ROWS DETECTED");
        }
        else if(lastError().number() == 1329)
        {
            // Error: 1329 SQLSTATE: 02000 (ER_SP_FETCH_NO_DATA)
            // Message: No data - zero rows fetched, selected, or processed
            GSLOG(SYSLOG_SEV_ERROR,"NO DATA DETECTED");
        }
        else if((lastError().number() == 2006)
                || (lastError().number() == 2013))
        {
            // Error: 2006 (CR_SERVER_GONE_ERROR)
            // Message: MySQL server has gone away
            // Error: 2013
            // Message: Lost connection to MySQL server during query
            GSLOG(SYSLOG_SEV_ERROR,"SERVER HAS GONE AWAY");
            // Reconnect the server
            if(!m_pPluginBase->m_pclDatabaseConnector->IsConnected())
                m_pPluginBase->m_pclDatabaseConnector->Connect();
        }
    }

    // Restore cursor
    //qApp->restoreOverrideCursor();
    if (m_pPluginBase)
        emit m_pPluginBase->sBusy(false);

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        m_fTimer_DbQuery = Elapsed();
        m_fTimer_DbQuery_Cumul += m_fTimer_DbQuery;
        m_fTimer_DbIteration = 0;
        m_ulRetrievedRows = 0;

        if(!bStatus)
        {
#ifdef NO_QUERY_DUMP
            strDebugMessage = "SQL query:";
            strDebugMessage += m_strQuery;
            m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
#endif
            m_pPluginBase->WriteDebugMessageFile(lastError().text().replace('\n',' '));
        }
    }

    return bStatus;
}

bool GexDbPlugin_Query::canExecuteQuery() const
{
    if( m_pPluginBase != NULL )
    {
        if( m_pPluginBase->m_pclDatabaseConnector != NULL )
        {
            QSqlDatabase lDb = QSqlDatabase::database( m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName,
                                                       false );

            if( ! lDb.isOpen() && ! lDb.open() )
            {
                emit m_pPluginBase->sBusy(false);
                return false;
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Execute specified query
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Query::Execute(const QString & strQuery)
{
    QString	strDebugMessage;

    // Save query
    m_strQuery = strQuery;

    // Debug mode ?
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        // put NO_QUERY_DUMP in your user_custom.pri
#ifndef NO_QUERY_DUMP
        strDebugMessage = "SQL query:";
        strDebugMessage += strQuery;
        m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
#endif
        StartTimer(true);
    }

    // Busy cursor
    //qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    if (m_pPluginBase)
        emit m_pPluginBase->sBusy(true);

    // Exec query
    if( ! canExecuteQuery() ) return false;

    bool bStatus = exec(strQuery);
    if(!bStatus && m_pPluginBase && m_pPluginBase->m_pclDatabaseConnector->IsMySqlDB())
    {
        bool bRetry = false;
        GSLOG(SYSLOG_SEV_ERROR,( QString("ERROR QUERY=%1 ERRORNB=%2 ERRORMSG=%3")
                  .arg(lastQuery().toLatin1().constData())
                  .arg(lastError().number())
                  .arg(lastError().text().toLatin1().constData())).toLatin1().constData());

        // DeadLock Error: 1213 SQLSTATE: 40001 (ER_LOCK_DEADLOCK)
        if(lastError().number() == 1213)
        {
            // Deadlock found when trying to get lock; try restarting transaction
            GSLOG(SYSLOG_SEV_ERROR,"DEADLOCK DETECTED - try restarting transaction");
            // Dump MySql DeadLock Status
            // Use a new SqlQuery
            QSqlQuery clQuery(QSqlDatabase::database(m_pPluginBase->m_pclDatabaseConnector->m_strConnectionName));
            clQuery.exec("show engine innodb status");
            clQuery.first();
            if(!clQuery.value(2).toString().isEmpty())
            {
                GSLOG(SYSLOG_SEV_DEBUG, (QString("QUERY=%1 STATUS=%2").arg(
                     clQuery.lastQuery().toLatin1().constData())
                     .arg(clQuery.value(2).toString().toLatin1().constData())).toLatin1().constData());
            }
            bRetry = true;
        }
        // ERRORNB=1205 ERRORMSG=Lock wait timeout exceeded; try restarting transaction
        else if(lastError().number() == 1205)
        {
            // TIMEOUT found when trying to get lock; try restarting transaction
            GSLOG(SYSLOG_SEV_ERROR,"TIMEOUT DETECTED - try restarting transaction");
            bRetry = true;
        }
        else if(lastError().number() == 1172)
        {
            // Error: 1172 SQLSTATE: 42000 (ER_TOO_MANY_ROWS)
            // Message: Result consisted of more than one row
            GSLOG(SYSLOG_SEV_ERROR,"TOO MANY ROWS DETECTED");
        }
        else if(lastError().number() == 1329)
        {
            // Error: 1329 SQLSTATE: 02000 (ER_SP_FETCH_NO_DATA)
            // Message: No data - zero rows fetched, selected, or processed
            GSLOG(SYSLOG_SEV_ERROR,"NO DATA DETECTED");
        }
        else if((lastError().number() == 2006)
                || (lastError().number() == 2013))
        {
            // Error: 2006 (CR_SERVER_GONE_ERROR)
            // Message: MySQL server has gone away
            // Error: 2013 (CR_SERVER_LOST)
            // Message: Lost connection to MySQL server during query
            GSLOG(SYSLOG_SEV_ERROR,"SERVER HAS GONE AWAY - try restarting transaction");
            // Reconnect the server
            if(!m_pPluginBase->m_pclDatabaseConnector->IsConnected())
                m_pPluginBase->m_pclDatabaseConnector->Connect();
            bRetry = true;
        }
        if(bRetry)
        {
            if( ! canExecuteQuery() ) return false;
            bStatus = exec(strQuery);
            if(bStatus)
            {
                GSLOG(SYSLOG_SEV_ERROR,"try restarting transaction - QUERY PASS");
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR,"try restarting transaction - QUERY FAIL AGAIN");
            }
        }
    }

    // Restore cursor
    //qApp->restoreOverrideCursor();
    if (m_pPluginBase)
        emit m_pPluginBase->sBusy(false);

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        m_fTimer_DbQuery = Elapsed();
        m_fTimer_DbQuery_Cumul += m_fTimer_DbQuery;
        m_fTimer_DbIteration = 0;
        m_ulRetrievedRows = 0;

        if(!bStatus)
        {
#ifdef NO_QUERY_DUMP
            strDebugMessage = "SQL query:";
            strDebugMessage += strQuery;
            m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
#endif
            m_pPluginBase->WriteDebugMessageFile(lastError().text().replace('\n',' '));
        }
    }

    return bStatus;
}

bool GexDbPlugin_Query::Next()
{
    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
        StartTimer();

    bool bStatus = next();

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        float fElapsed;
        if(bStatus)
        {
            fElapsed = Elapsed();
            m_fTimer_DbIteration += fElapsed;
            m_fTimer_DbIteration_Cumul += fElapsed;
            m_ulRetrievedRows++;
            m_ulRetrievedRows_Cumul++;
        }
        else
            DumpPerformance();
    }

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Iterate to first query result
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Query::First()
{
    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
        StartTimer();

    bool bStatus = first();

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        float fElapsed;
        if(bStatus)
        {
            fElapsed = Elapsed();
            m_fTimer_DbIteration += fElapsed;
            m_fTimer_DbIteration_Cumul += fElapsed;
            m_ulRetrievedRows++;
            m_ulRetrievedRows_Cumul++;
        }
        else
            DumpPerformance();
    }

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Seek to specified query result row
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Query::Seek(int i)
{
    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
        StartTimer();

    bool bStatus = seek(i);

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        float fElapsed;
        if(bStatus)
        {
            fElapsed = Elapsed();
            m_fTimer_DbIteration += fElapsed;
            m_fTimer_DbIteration_Cumul += fElapsed;
            m_ulRetrievedRows++;
            m_ulRetrievedRows_Cumul++;
        }
    }

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// If in debug mode, dump the performance of the query
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Query::DumpPerformance()
{
#ifdef NO_DUMP_PERF
    return;
#endif

    // Debug mode ??
    if(m_pPluginBase && m_pPluginBase->m_bCustomerDebugMode)
    {
        QString strDebugMessage;
        if(m_ulRetrievedRows > 0)
        {
            float	fElapsed;
            Elapsed(&fElapsed);
            strDebugMessage =  "Query performance:\n";
            strDebugMessage += " Execution time           = " + QString::number(fElapsed/1000.0F, 'f', 2) + " ms\n";
            strDebugMessage += " SQL query execution      = " + QString::number(m_fTimer_DbQuery/1000.0F, 'f', 2) + " ms\n";
            strDebugMessage += " SQL query iteration      = " + QString::number(m_fTimer_DbIteration/1000.0F, 'f', 2) + " ms\n";
            strDebugMessage += " SQL query extracted rows = " + QString::number(m_ulRetrievedRows) + "\n";
            strDebugMessage += " GEX overhead             = " + QString::number((fElapsed - m_fTimer_DbIteration - m_fTimer_DbQuery)/1000.0F, 'f', 2) + " ms\n";
            m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
        }
        else
        {
            strDebugMessage = "Query returned no results.\n";
            m_pPluginBase->WriteDebugMessageFile(strDebugMessage);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_TestInfo_Stats class: holding test stats
////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_Stats::GexDbPlugin_TestInfo_Stats()
{
    Reset();
}

GexDbPlugin_TestInfo_Stats::GexDbPlugin_TestInfo_Stats(const GexDbPlugin_TestInfo_Stats & source)
{
    m_uiExecCount = source.m_uiExecCount;
    m_uiFailCount = source.m_uiFailCount;
    m_fMin = source.m_fMin;
    m_fMax = source.m_fMax;
    m_fSum = source.m_fSum;
    m_fSumSquare = source.m_fSumSquare;
    m_fTestTime = source.m_fTestTime;
    m_uiOptFlag = source.m_uiOptFlag;
}

//////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_Stats & GexDbPlugin_TestInfo_Stats::operator=(const GexDbPlugin_TestInfo_Stats & source)
{
    m_uiExecCount = source.m_uiExecCount;
    m_uiFailCount = source.m_uiFailCount;
    m_fMin = source.m_fMin;
    m_fMax = source.m_fMax;
    m_fSum = source.m_fSum;
    m_fSumSquare = source.m_fSumSquare;
    m_fTestTime = source.m_fTestTime;
    m_uiOptFlag = source.m_uiOptFlag;

    return *this;
}

GexDbPlugin_TestInfo_Stats::~GexDbPlugin_TestInfo_Stats()
{
}

void GexDbPlugin_TestInfo_Stats::Reset()
{
    m_uiExecCount = 0;
    m_uiFailCount = 4294967295u;
    m_fMin = 0.0F;
    m_fMax = 0.0F;
    m_fSum = 0.0F;
    m_fSumSquare = 0.0F;
    m_fTestTime = 0.0F;
    m_uiOptFlag = 0;
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_TestInfo class: holding test info
////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo::GexDbPlugin_TestInfo()
{
    m_pTestInfo_PTR = NULL;
    m_pTestInfo_MPR = NULL;
    m_pTestInfo_FTR = NULL;

    Reset();
}

GexDbPlugin_TestInfo::GexDbPlugin_TestInfo(const GexDbPlugin_TestInfo & source)
{
    m_lTestID							= source.m_lTestID;
    m_uiTestNumber						= source.m_uiTestNumber;
    m_strTestName						= source.m_strTestName;
    m_uiTestSeq							= source.m_uiTestSeq;
    m_cTestType							= source.m_cTestType;
    m_bTestExecuted						= source.m_bTestExecuted;
    m_bHaveSamples						= source.m_bHaveSamples;
    m_bStatsFromSummaryComplete			= source.m_bStatsFromSummaryComplete;
    m_bMinimumStatsFromSummaryAvailable	= source.m_bMinimumStatsFromSummaryAvailable;
    m_mapStatsFromSamples				= source.m_mapStatsFromSamples;
    m_mapStatsFromSummary				= source.m_mapStatsFromSummary;
    m_bStaticInfoWrittenToStdf			= source.m_bStaticInfoWrittenToStdf;

    if(source.m_pTestInfo_PTR)
    {
        m_pTestInfo_PTR = new GexDbPlugin_TestInfo_PTR();
        *(m_pTestInfo_PTR)	= *(source.m_pTestInfo_PTR);
    }
    else
        m_pTestInfo_PTR = NULL;
    if(source.m_pTestInfo_MPR)
    {
        m_pTestInfo_MPR = new GexDbPlugin_TestInfo_MPR();
        *(m_pTestInfo_MPR)	= *(source.m_pTestInfo_MPR);
    }
    else
        m_pTestInfo_MPR = NULL;
    if(source.m_pTestInfo_FTR)
    {
        m_pTestInfo_FTR = new GexDbPlugin_TestInfo_FTR();
        *(m_pTestInfo_FTR)	= *(source.m_pTestInfo_FTR);
    }
    else
        m_pTestInfo_FTR = NULL;
}

GexDbPlugin_TestInfo::~GexDbPlugin_TestInfo()
{
    if(m_pTestInfo_PTR)
        delete m_pTestInfo_PTR;
    if(m_pTestInfo_MPR)
        delete m_pTestInfo_MPR;
    if(m_pTestInfo_FTR)
        delete m_pTestInfo_FTR;
}

void GexDbPlugin_TestInfo::Reset()
{
    m_lTestID = -1;
    m_uiTestNumber = 0;
    m_strTestName = "";
    m_uiTestSeq = 0;
    m_cTestType = 'P';
    m_bTestExecuted = false;
    m_bHaveSamples = false;
    m_bStatsFromSummaryComplete = false;
    m_bMinimumStatsFromSummaryAvailable = false;
    m_mapStatsFromSamples.clear();
    m_mapStatsFromSummary.clear();
    m_bStaticInfoWrittenToStdf = false;

    if(m_pTestInfo_PTR)
    {
        delete m_pTestInfo_PTR;
        m_pTestInfo_PTR = NULL;
    }
    if(m_pTestInfo_MPR)
    {
        delete m_pTestInfo_MPR;
        m_pTestInfo_MPR = NULL;
    }
    if(m_pTestInfo_FTR)
    {
        delete m_pTestInfo_FTR;
        m_pTestInfo_FTR = NULL;
    }
}

void GexDbPlugin_TestInfo::ResetDynamicInfo()
{
    m_bTestExecuted = false;
    m_bStatsFromSummaryComplete = false;
    m_bMinimumStatsFromSummaryAvailable = false;
    if(m_pTestInfo_PTR)
        m_pTestInfo_PTR->ResetDynamicInfo();
    if(m_pTestInfo_MPR)
        m_pTestInfo_MPR->ResetDynamicInfo();
    if(m_pTestInfo_FTR)
        m_pTestInfo_FTR->ResetDynamicInfo();
}

//////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo & GexDbPlugin_TestInfo::operator=(const GexDbPlugin_TestInfo & source)
{
    m_lTestID							= source.m_lTestID;
    m_uiTestNumber						= source.m_uiTestNumber;
    m_strTestName						= source.m_strTestName;
    m_uiTestSeq							= source.m_uiTestSeq;
    m_cTestType							= source.m_cTestType;
    m_bTestExecuted						= source.m_bTestExecuted;
    m_bHaveSamples						= source.m_bHaveSamples;
    m_bStatsFromSummaryComplete			= source.m_bStatsFromSummaryComplete;
    m_bMinimumStatsFromSummaryAvailable	= source.m_bMinimumStatsFromSummaryAvailable;
    m_mapStatsFromSamples				= source.m_mapStatsFromSamples;
    m_mapStatsFromSummary				= source.m_mapStatsFromSummary;
    m_bStaticInfoWrittenToStdf			= source.m_bStaticInfoWrittenToStdf;

    if(m_pTestInfo_PTR)
    {
        delete m_pTestInfo_PTR;
        m_pTestInfo_PTR = NULL;
    }
    if(source.m_pTestInfo_PTR)
    {
        m_pTestInfo_PTR = new GexDbPlugin_TestInfo_PTR();
        *(m_pTestInfo_PTR)	= *(source.m_pTestInfo_PTR);
    }
    if(m_pTestInfo_MPR)
    {
        delete m_pTestInfo_MPR;
        m_pTestInfo_MPR = NULL;
    }
    if(source.m_pTestInfo_MPR)
    {
        m_pTestInfo_MPR = new GexDbPlugin_TestInfo_MPR();
        *(m_pTestInfo_MPR)	= *(source.m_pTestInfo_MPR);
    }
    if(m_pTestInfo_FTR)
    {
        delete m_pTestInfo_FTR;
        m_pTestInfo_FTR = NULL;
    }
    if(source.m_pTestInfo_FTR)
    {
        m_pTestInfo_FTR = new GexDbPlugin_TestInfo_FTR();
        *(m_pTestInfo_FTR)	= *(source.m_pTestInfo_FTR);
    }

    return *this;
}

bool GexDbPlugin_TestInfo::operator==(const GexDbPlugin_TestInfo & source)
{
    return (m_uiTestNumber == source.m_uiTestNumber);
}

bool GexDbPlugin_TestInfo::operator<(const GexDbPlugin_TestInfo & source)
{
    return (m_uiTestNumber < source.m_uiTestNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfoMap
// Map of <unsigned int, GexDbPlugin_TestInfo>
/////////////////////////////////////////////////////////////////////////////////////////
void GexDbPlugin_TestInfoMap::Reset()
{
    GexDbPlugin_TestInfoMap::Iterator it;
    for(it = begin(); it != end(); it++)
        it.value().Reset();
}

void GexDbPlugin_TestInfoMap::ResetDynamicInfo()
{
    GexDbPlugin_TestInfoMap::Iterator it;
    for(it = begin(); it != end(); it++)
        it.value().ResetDynamicInfo();
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_TestInfo_PTR class: holding specific info on 1 parametric test
////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_PTR::GexDbPlugin_TestInfo_PTR()
{
    // Static info
    m_uiStaticFlags = 0;
    m_strTestUnit = "";
    m_bHasLL = false;
    m_bHasHL = false;
    m_fLL = 0.0F;
    m_fHL = 0.0F;
    m_bHasSpecLL = false;
    m_bHasSpecHL = false;
    m_fSpecLL = 0.0F;
    m_fSpecHL = 0.0F;
    m_nResScal = GEXDB_INVALID_SCALING_FACTOR;	// Value for invalid: valid values are in range -12 to 15
    m_nLlScal = GEXDB_INVALID_SCALING_FACTOR;	// Value for invalid: valid values are in range -12 to 15
    m_nHlScal = GEXDB_INVALID_SCALING_FACTOR;	// Value for invalid: valid values are in range -12 to 15
}

GexDbPlugin_TestInfo_PTR::~GexDbPlugin_TestInfo_PTR()
{
}

void GexDbPlugin_TestInfo_PTR::ResetDynamicInfo()
{
    // Dynamic info
    m_uiDynamicFlagsList.clear();
    m_fResultList.clear();
}

//////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_PTR & GexDbPlugin_TestInfo_PTR::operator=(const GexDbPlugin_TestInfo_PTR & source)
{
    // Static info
    m_uiStaticFlags		= source.m_uiStaticFlags;
    m_strTestUnit		= source.m_strTestUnit;
    m_bHasLL			= source.m_bHasLL;
    m_bHasHL			= source.m_bHasHL;
    m_fLL				= source.m_fLL;
    m_fHL				= source.m_fHL;
    m_bHasSpecLL		= source.m_bHasSpecLL;
    m_bHasSpecHL		= source.m_bHasSpecHL;
    m_fSpecLL			= source.m_fSpecLL;
    m_fSpecHL			= source.m_fSpecHL;
    m_nResScal			= source.m_nResScal;
    m_nLlScal			= source.m_nLlScal;
    m_nHlScal			= source.m_nHlScal;
    mMultiLimits        = source.mMultiLimits;
    // Dynamic info
    m_uiDynamicFlagsList	= source.m_uiDynamicFlagsList;
    m_fResultList			= source.m_fResultList;

    return *this;
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_TestInfo_MPR class: holding specific info on 1 multi-result parametric test
////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_MPR::GexDbPlugin_TestInfo_MPR()
{
    // Static info
    m_uiStaticFlags = 0;
    m_strTestUnit = QString("");
    m_bHasLL = false;
    m_bHasHL = false;
    m_fLL = 0.0F;
    m_fHL = 0.0F;
    m_bHasSpecLL = false;
    m_bHasSpecHL = false;
    m_fSpecLL = 0.0F;
    m_fSpecHL = 0.0F;
    m_nTpin_ArrayIndex = 0;
    m_nResScal = GEXDB_INVALID_SCALING_FACTOR;	// Value for invalid: valid values are in range -12 to 15
    m_nLlScal = GEXDB_INVALID_SCALING_FACTOR;	// Value for invalid: valid values are in range -12 to 15
    m_nHlScal = GEXDB_INVALID_SCALING_FACTOR;	// Value for invalid: valid values are in range -12 to 15
}

GexDbPlugin_TestInfo_MPR::~GexDbPlugin_TestInfo_MPR()
{
}

void GexDbPlugin_TestInfo_MPR::ResetDynamicInfo()
{
    // Dynamic info
    m_uiDynamicFlagsList.clear();
    m_fResultList.clear();
    m_nTpin_PmrIndexList.clear();
}

//////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_MPR & GexDbPlugin_TestInfo_MPR::operator=(const GexDbPlugin_TestInfo_MPR & source)
{
    // Static info
    m_uiStaticFlags		= source.m_uiStaticFlags;
    m_strTestUnit		= source.m_strTestUnit;
    m_bHasLL			= source.m_bHasLL;
    m_bHasHL			= source.m_bHasHL;
    m_fLL				= source.m_fLL;
    m_fHL				= source.m_fHL;
    m_bHasSpecLL		= source.m_bHasSpecLL;
    m_bHasSpecHL		= source.m_bHasSpecHL;
    m_fSpecLL			= source.m_fSpecLL;
    m_fSpecHL			= source.m_fSpecHL;
    m_nTpin_ArrayIndex	= source.m_nTpin_ArrayIndex;
    m_nResScal			= source.m_nResScal;
    m_nLlScal			= source.m_nLlScal;
    m_nHlScal			= source.m_nHlScal;
    // Dynamic info
    m_uiDynamicFlagsList	= source.m_uiDynamicFlagsList;
    m_fResultList			= source.m_fResultList;
    m_nTpin_PmrIndexList	= source.m_nTpin_PmrIndexList;

    return *this;
}

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_TestInfo_FTR class: holding specific info on 1 functional test
////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_FTR::GexDbPlugin_TestInfo_FTR()
{
    // Static info
    // Dynamic info
    m_uiDynamicFlagsList.clear();
    m_strVectorNameList.clear();
    m_uiVectorOffsetList.clear();
}

GexDbPlugin_TestInfo_FTR::~GexDbPlugin_TestInfo_FTR()
{
}

void GexDbPlugin_TestInfo_FTR::ResetDynamicInfo()
{
    // Dynamic info
    m_uiDynamicFlagsList.clear();
    m_strVectorNameList.clear();
    m_uiVectorOffsetList.clear();
}

//////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfo_FTR & GexDbPlugin_TestInfo_FTR::operator=(const GexDbPlugin_TestInfo_FTR & source)
{
    // Static info
    // Dynamic info
    m_uiDynamicFlagsList	= source.m_uiDynamicFlagsList;
    m_strVectorNameList		= source.m_strVectorNameList;
    m_uiVectorOffsetList	= source.m_uiVectorOffsetList;

    return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfoContainer
// Container having ptrs on GexDbPlugin_TestInfo objects
/////////////////////////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfoContainer::GexDbPlugin_TestInfoContainer()
{
    m_pTestInfo = NULL;
    m_pNextTest = NULL;
    m_pNextTest_PTR = NULL;
    m_pNextTest_MPR = NULL;
    m_pNextTest_FTR = NULL;
}

GexDbPlugin_TestInfoContainer::~GexDbPlugin_TestInfoContainer()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_TestInfoList
// Holds a list of TestInfo containers
/////////////////////////////////////////////////////////////////////////////////////////
GexDbPlugin_TestInfoList::GexDbPlugin_TestInfoList()
{
    ResetData();
}

GexDbPlugin_TestInfoList::~GexDbPlugin_TestInfoList()
{
    ClearData();
}

void GexDbPlugin_TestInfoList::ResetData()
{
    m_pFirstTest		= NULL;
    m_pCurrentTest		= NULL;
    m_uiNbTests			= 0;
    m_pFirstTest_PTR	= NULL;
    m_pCurrentTest_PTR	= NULL;
    m_uiNbTests_PTR		= 0;
    m_pFirstTest_MPR	= NULL;
    m_pCurrentTest_MPR	= NULL;
    m_uiNbTests_MPR		= 0;
    m_pFirstTest_FTR	= NULL;
    m_pCurrentTest_FTR	= NULL;
    m_uiNbTests_FTR		= 0;

    // Empty testID lists
    m_strTestIdList_PTR = m_strTestIdList_MPR = m_strTestIdList_FTR = "";
}

void GexDbPlugin_TestInfoList::ClearData(bool bDeleteTestInfo/* = true*/)
{
    GexDbPlugin_TestInfoContainer *pTestInfoContainer;

    // Delete list
    while(m_pFirstTest != NULL)
    {
        pTestInfoContainer = m_pFirstTest;
        m_pFirstTest = pTestInfoContainer->m_pNextTest;
        if(bDeleteTestInfo)
            delete pTestInfoContainer->m_pTestInfo;
        delete pTestInfoContainer;
    }
    ResetData();
}

void GexDbPlugin_TestInfoList::Insert(GexDbPlugin_TestInfo *pTestInfo)
{
    // Create new container
    GexDbPlugin_TestInfoContainer *pNewTest = new GexDbPlugin_TestInfoContainer;
    pNewTest->m_pTestInfo = pTestInfo;
    m_uiNbTests++;

    // Check if first element of the list
    if(m_pFirstTest == NULL)
    {
        m_pFirstTest = m_pCurrentTest = pNewTest;
        if(pTestInfo->m_pTestInfo_PTR)
        {
            m_pFirstTest_PTR = m_pCurrentTest_PTR = pNewTest;
            m_uiNbTests_PTR++;
            // Add to testID list
            if(!m_strTestIdList_PTR.isEmpty())
                m_strTestIdList_PTR += ",";
            m_strTestIdList_PTR += QString::number(pTestInfo->m_lTestID);
        }
        else if(pTestInfo->m_pTestInfo_MPR)
        {
            m_pFirstTest_MPR = m_pCurrentTest_MPR = pNewTest;
            m_uiNbTests_MPR++;
            // Add to testID list
            if(!m_strTestIdList_MPR.isEmpty())
                m_strTestIdList_MPR += ",";
            m_strTestIdList_MPR += QString::number(pTestInfo->m_lTestID);
        }
        else
        {
            m_pFirstTest_FTR = m_pCurrentTest_FTR = pNewTest;
            m_uiNbTests_FTR++;
            // Add to testID list
            if(!m_strTestIdList_FTR.isEmpty())
                m_strTestIdList_FTR += ",";
            m_strTestIdList_FTR += QString::number(pTestInfo->m_lTestID);
        }
        return;
    }

    // 1. Insert into main list at right position (ordered by TestSeq)
    GexDbPlugin_TestInfoContainer *pPrevTest=NULL;
    // Rewind ?
    if((m_pCurrentTest == NULL) || (pTestInfo->m_uiTestSeq < m_pCurrentTest->m_pTestInfo->m_uiTestSeq))
        m_pCurrentTest = m_pFirstTest;
    // Get Insertion point
    while(m_pCurrentTest && (m_pCurrentTest->m_pTestInfo->m_uiTestSeq < pTestInfo->m_uiTestSeq))
    {
        pPrevTest = m_pCurrentTest;
        m_pCurrentTest = m_pCurrentTest->m_pNextTest;
    }
    // Insert
    pNewTest->m_pNextTest = m_pCurrentTest;
    if(pPrevTest)
        pPrevTest->m_pNextTest = pNewTest;
    else
        m_pFirstTest = pNewTest;
    // Update current pointer
    m_pCurrentTest = pNewTest;

    // 2. Insert into list dedicated to test type of test to insert (ordered by TestID)
    if(pTestInfo->m_pTestInfo_PTR)
    {
        UpdateList_PTR(pNewTest);
        m_uiNbTests_PTR++;
        // Add to testID list
        if(!m_strTestIdList_PTR.isEmpty())
            m_strTestIdList_PTR += ",";
        m_strTestIdList_PTR += QString::number(pTestInfo->m_lTestID);
    }
    else if(pTestInfo->m_pTestInfo_MPR)
    {
        UpdateList_MPR(pNewTest);
        m_uiNbTests_MPR++;
        // Add to testID list
        if(!m_strTestIdList_MPR.isEmpty())
            m_strTestIdList_MPR += ",";
        m_strTestIdList_MPR += QString::number(pTestInfo->m_lTestID);
    }
    else
    {
        UpdateList_FTR(pNewTest);
        m_uiNbTests_FTR++;
        // Add to testID list
        if(!m_strTestIdList_FTR.isEmpty())
            m_strTestIdList_FTR += ",";
        m_strTestIdList_FTR += QString::number(pTestInfo->m_lTestID);
    }
}

void GexDbPlugin_TestInfoList::UpdateList_PTR(GexDbPlugin_TestInfoContainer *pNewTest)
{
    // Check if first element of the list
    if(m_pFirstTest_PTR == NULL)
    {
        m_pFirstTest_PTR = m_pCurrentTest_PTR = pNewTest;
        return;
    }

    // Insert into PTR list at right position (ordered by TestID)
    GexDbPlugin_TestInfoContainer *pPrevTest=NULL;
    // Rewind ?
    if((m_pCurrentTest_PTR == NULL) || (pNewTest->m_pTestInfo->m_lTestID < m_pCurrentTest_PTR->m_pTestInfo->m_lTestID))
        m_pCurrentTest_PTR = m_pFirstTest_PTR;
    // Get Insertion point
    while(m_pCurrentTest_PTR && (m_pCurrentTest_PTR->m_pTestInfo->m_lTestID < pNewTest->m_pTestInfo->m_lTestID))
    {
        pPrevTest = m_pCurrentTest_PTR;
        m_pCurrentTest_PTR = m_pCurrentTest_PTR->m_pNextTest_PTR;
    }
    // Insert
    pNewTest->m_pNextTest_PTR = m_pCurrentTest_PTR;
    if(pPrevTest)
        pPrevTest->m_pNextTest_PTR = pNewTest;
    else
        m_pFirstTest_PTR = pNewTest;
    // Update current pointer
    m_pCurrentTest_PTR = pNewTest;
}

void GexDbPlugin_TestInfoList::UpdateList_MPR(GexDbPlugin_TestInfoContainer *pNewTest)
{
    // Check if first element of the list
    if(m_pFirstTest_MPR == NULL)
    {
        m_pFirstTest_MPR = m_pCurrentTest_MPR = pNewTest;
        return;
    }

    // Insert into MPR list at right position (ordered by TestID)
    GexDbPlugin_TestInfoContainer *pPrevTest=NULL;
    // Rewind ?
    if((m_pCurrentTest_MPR == NULL) || (pNewTest->m_pTestInfo->m_lTestID < m_pCurrentTest_MPR->m_pTestInfo->m_lTestID))
        m_pCurrentTest_MPR = m_pFirstTest_MPR;
    // Get Insertion point
    while(m_pCurrentTest_MPR && (m_pCurrentTest_MPR->m_pTestInfo->m_lTestID < pNewTest->m_pTestInfo->m_lTestID))
    {
        pPrevTest = m_pCurrentTest_MPR;
        m_pCurrentTest_MPR = m_pCurrentTest_MPR->m_pNextTest_MPR;
    }
    // Insert
    pNewTest->m_pNextTest_MPR = m_pCurrentTest_MPR;
    if(pPrevTest)
        pPrevTest->m_pNextTest_MPR = pNewTest;
    else
        m_pFirstTest_MPR = pNewTest;
    // Update current pointer
    m_pCurrentTest_MPR = pNewTest;
}

void GexDbPlugin_TestInfoList::UpdateList_FTR(GexDbPlugin_TestInfoContainer *pNewTest)
{
    // Check if first element of the list
    if(m_pFirstTest_FTR == NULL)
    {
        m_pFirstTest_FTR = m_pCurrentTest_FTR = pNewTest;
        return;
    }

    // Insert into FTR list at right position (ordered by TestID)
    GexDbPlugin_TestInfoContainer *pPrevTest=NULL;
    // Rewind ?
    if((m_pCurrentTest_FTR == NULL) || (pNewTest->m_pTestInfo->m_lTestID < m_pCurrentTest_FTR->m_pTestInfo->m_lTestID))
        m_pCurrentTest_FTR = m_pFirstTest_FTR;
    // Get Insertion point
    while(m_pCurrentTest_FTR && (m_pCurrentTest_FTR->m_pTestInfo->m_lTestID < pNewTest->m_pTestInfo->m_lTestID))
    {
        pPrevTest = m_pCurrentTest_FTR;
        m_pCurrentTest_FTR = m_pCurrentTest_FTR->m_pNextTest_FTR;
    }
    // Insert
    pNewTest->m_pNextTest_FTR = m_pCurrentTest_FTR;
    if(pPrevTest)
        pPrevTest->m_pNextTest_FTR = pNewTest;
    else
        m_pFirstTest_FTR = pNewTest;
    // Update current pointer
    m_pCurrentTest_FTR = pNewTest;
}

GexDbPlugin_TestInfo* GexDbPlugin_TestInfoList::FindTestByID_PTR(unsigned int uiTestID)
{
    if((m_pCurrentTest_PTR == NULL) || (m_pCurrentTest_PTR->m_pTestInfo->m_lTestID > uiTestID))
        m_pCurrentTest_PTR = m_pFirstTest_PTR;

    while(m_pCurrentTest_PTR)
    {
        if(m_pCurrentTest_PTR->m_pTestInfo->m_lTestID == uiTestID)
            return m_pCurrentTest_PTR->m_pTestInfo;
        m_pCurrentTest_PTR = m_pCurrentTest_PTR->m_pNextTest_PTR;
    }
    return NULL;
}

GexDbPlugin_TestInfo* GexDbPlugin_TestInfoList::FindTestByID_MPR(unsigned int uiTestID)
{
    if((m_pCurrentTest_MPR == NULL) || (m_pCurrentTest_MPR->m_pTestInfo->m_lTestID > uiTestID))
        m_pCurrentTest_MPR = m_pFirstTest_MPR;

    while(m_pCurrentTest_MPR)
    {
        if(m_pCurrentTest_MPR->m_pTestInfo->m_lTestID == uiTestID)
            return m_pCurrentTest_MPR->m_pTestInfo;
        m_pCurrentTest_MPR = m_pCurrentTest_MPR->m_pNextTest_MPR;
    }
    return NULL;
}

GexDbPlugin_TestInfo* GexDbPlugin_TestInfoList::FindTestByID_FTR(unsigned int uiTestID)
{
    if((m_pCurrentTest_FTR == NULL) || (m_pCurrentTest_FTR->m_pTestInfo->m_lTestID > uiTestID))
        m_pCurrentTest_FTR = m_pFirstTest_FTR;

    while(m_pCurrentTest_FTR)
    {
        if(m_pCurrentTest_FTR->m_pTestInfo->m_lTestID == uiTestID)
            return m_pCurrentTest_FTR->m_pTestInfo;
        m_pCurrentTest_FTR = m_pCurrentTest_FTR->m_pNextTest_FTR;
    }
    return NULL;
}

GexDbPlugin_TestInfo* GexDbPlugin_TestInfoList::FindTestByNb(unsigned int uiTestNb)
{
    // Just check if next test is the one we look for (in case test numbers are ordered)
    if((m_pCurrentTest != NULL) && (m_pCurrentTest->m_pNextTest != NULL) && (m_pCurrentTest->m_pNextTest->m_pTestInfo->m_uiTestNumber == uiTestNb))
        return m_pCurrentTest->m_pNextTest->m_pTestInfo;

    m_pCurrentTest = m_pFirstTest;
    while(m_pCurrentTest)
    {
        if(m_pCurrentTest->m_pTestInfo->m_uiTestNumber == uiTestNb)
            return m_pCurrentTest->m_pTestInfo;
        m_pCurrentTest = m_pCurrentTest->m_pNextTest;
    }
    return NULL;
}

void GexDbPlugin_TestInfoList::ResetDynamicTestInfo()
{
    GexDbPlugin_TestInfoContainer *pContainer = m_pFirstTest;
    while(pContainer)
    {
        pContainer->m_pTestInfo->ResetDynamicInfo();
        pContainer = pContainer->m_pNextTest;
    }
}

void GexDbPlugin_TestInfoList::ResetTestInfo()
{
    GexDbPlugin_TestInfoContainer *pContainer = m_pFirstTest;
    while(pContainer)
    {
        pContainer->m_pTestInfo->Reset();
        pContainer = pContainer->m_pNextTest;
    }
}

void GexDbPlugin_TestInfoList::GestTestListString(QString & strTestListString)
{
    GexDbPlugin_TestInfoContainer *pContainer;

    // Nb of tests
    strTestListString = QString::number(m_uiNbTests) + " tests, ";
    strTestListString += QString::number(m_uiNbTests_PTR) + " PTR tests,";
    strTestListString += QString::number(m_uiNbTests_MPR) + " MPR tests,";
    strTestListString += QString::number(m_uiNbTests_FTR) + " FTR tests\n";

    // Go through full list
    strTestListString += "Full TestList (TestSeq|TestID|TestNb ordered by TestSeq):\n";
    if(m_pFirstTest == NULL)
    {
        strTestListString += "(empty)\n";
        return;
    }

    pContainer = m_pFirstTest;
    while(pContainer)
    {
        strTestListString += QString::number(pContainer->m_pTestInfo->m_uiTestSeq);
        strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_lTestID);
        strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestNumber);
        strTestListString += ",";
        pContainer = pContainer->m_pNextTest;
    }
    strTestListString += "\n";

    // Go through PTR list
    strTestListString += "PTR TestList (TestID|TestSeq|TestNb ordered by TestID):\n";
    if(m_pFirstTest_PTR == NULL)
        strTestListString += "(empty)";
    else
    {
        pContainer = m_pFirstTest_PTR;
        while(pContainer)
        {
            strTestListString += QString::number(pContainer->m_pTestInfo->m_lTestID);
            strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestSeq);
            strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestNumber);
            strTestListString += ",";
            pContainer = pContainer->m_pNextTest_PTR;
        }
    }
    strTestListString += "\n";

    // Go through MPR list
    strTestListString += "MPR TestList (TestID|TestSeq|TestNb ordered by TestID):\n";
    if(m_pFirstTest_MPR == NULL)
        strTestListString += "(empty)";
    else
    {
        pContainer = m_pFirstTest_MPR;
        while(pContainer)
        {
            strTestListString += QString::number(pContainer->m_pTestInfo->m_lTestID);
            strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestSeq);
            strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestNumber);
            strTestListString += ",";
            pContainer = pContainer->m_pNextTest_MPR;
        }
    }
    strTestListString += "\n";

    // Go through FTR list
    strTestListString += "FTR TestList (TestID|TestSeq|TestNb ordered by TestID):\n";
    if(m_pFirstTest_FTR == NULL)
        strTestListString += "(empty)";
    else
    {
        pContainer = m_pFirstTest_FTR;
        while(pContainer)
        {
            strTestListString += QString::number(pContainer->m_pTestInfo->m_lTestID);
            strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestSeq);
            strTestListString += "|" + QString::number(pContainer->m_pTestInfo->m_uiTestNumber);
            strTestListString += ",";
            pContainer = pContainer->m_pNextTest_FTR;
        }
    }
    strTestListString += "\n";
}


///////////////////////////////////////////////////////////
// GexDbPlugin_Base class: database plugin base class...
///////////////////////////////////////////////////////////

// Error map
GBEGIN_ERROR_MAP(GexDbPlugin_Base)
// General
GMAP_ERROR_EX(eLibraryNotFound, "Plug-in library not found: %s",
              "Plugin library not found")
GMAP_ERROR_EX(eUnresolvedFunctions, "Unresolved function %s in plug-in library %s",
              "Unresolved functions in library")
GMAP_ERROR_EX(eFunctionNotSupported, "GexDB plugin: %s\nUnsupported function: %s.",
              "Function is not supported by the plugin")
GMAP_ERROR_EX(eInit, "Error initializing plug-in library %s",
              "Error initializing plug-in library")
GMAP_ERROR_EX(eMarkerNotFound, "Marker not found: %s.",
              "XML marker not found in settings file")
GMAP_ERROR_EX(eReadSettings, "Read settings error.",
              "Failed reading settings file")
GMAP_ERROR_EX(eWriteSettings, "Write settings error.",
              "Error writing settings file")
GMAP_ERROR_EX(eLicenceExpired, QString("License has expired or Data file\n%s out of date...\nPlease contact %1").arg(GEX_EMAIL_SALES).toLatin1().constData(),
              "File date out of Licence window!")
GMAP_ERROR_EX(eMemoryAllocation, "Memory allocation failure.",
              "Memory allocation error")
// STDF file manipulation
GMAP_ERROR_EX(eStdf_Open, "Failed opening STDF file.",
              "Failed opening STDF file")
GMAP_ERROR_EX(eStdf_Read, "Error reading record in STDF file.",
              "Error reading STDF file")
GMAP_ERROR_EX(eStdf_Corrupted, "Corrupted STDF file: %s.",
              "Corrupted STDF file")
GMAP_ERROR_EX(eStdf_DtrCommand_BadSyntax, "Bad syntax found in DTR record: %s.",
              "Found valid DTR command but with a wrong syntax")
GMAP_ERROR_EX(eStdf_DtrCommand_BadUsage,"Bad usage found in DTR record: %s.",
              "Found valid DTR command but with a wrong usage")
GMAP_ERROR_EX(eStdf_DtrSyntricity_BadSyntax,"Bad syntax found in DTR record: %s.",
              "Found DTR test condition with a wrong syntax")
// Data validation
GMAP_ERROR_EX(eValidation_AlreadyInserted, "Data for this splitlot has already been inserted into the DB (Testing stage=%s, Lot=%s, WaferID=%s, Tester=%s, Station=%d, Start_Time=%s).",
              "Data already inserted in the DB")
GMAP_ERROR_EX(eValidation_DuplicateTest, "Found parametric and functional tests with the same definition: test number %d (%s).",
              "Test with same test nb and test name appears with different type (PTR,MPR,FTR)")
GMAP_ERROR_EX(eValidation_DuplicateTestNumber, "Found tests with the same test number and different test name: test number %d (%s) from %s record.",
              "Test with same test nb and diff test name")
GMAP_ERROR_EX(eValidation_MultipleLimits, "More than one limit definition per site for test number %d (%s).",
              "Multiple limits per test/site")
GMAP_ERROR_EX(eValidation_MissingRecords, "Some mandatory records are missing in the STDF file (%s).",
              "Some mandatory records are missing in the STDF file (ie MIR, MRR)")
GMAP_ERROR_EX(eValidation_EmptyField, "Some mandatory fields are empty (%s).",
              "Some mandatory fields are empty (ie LotID)")
GMAP_ERROR_EX(eValidation_InvalidField, "Some fields are invalid (%s).",
              "Some fields are invalid (ie PartID)")
GMAP_ERROR_EX(eValidation_MultiRecords, "Detected multiple %s records in the same data file.",
              "STDF file has data for multiple records")
GMAP_ERROR_EX(eValidation_DbKeyOverload, "%s has been overloaded with value %s (through dbkeys mapping): %s.",
              "DbKeys has been overloaded with invalid value")
GMAP_ERROR_EX(eValidation_CountMismatch, "The number of %s(%d) and %s(%d) records does not match.",
              "Count record")
GMAP_ERROR_EX(eValidation_BinMismatch, "Binning mismatch: %s.",
              "Bin category doesn't match with PRR flag")
GMAP_ERROR_EX(eValidation_NotSupported, "Configuration not supported: %s.",
              "Unsupported configuration")
GMAP_ERROR_EX(eValidation_InconsistentTestNumber, "Inconsistent test number detected: %s.",
              "Inconsistent test number detected")
GMAP_ERROR_EX(eValidation_InconsistentTestResult, "Inconsistent test result detected: %s.",
              "Inconsistent test result detected")
GMAP_ERROR_EX(eValidation_InconsistentTestName, "Inconsistent test name detected: %s.",
              "Inconsistent test name detected")
GMAP_ERROR_EX(eValidation_InconsistentTestFlow, "Inconsistent test flow detected: %s.",
              "Inconsistent test flow detected")
GMAP_ERROR_EX(eValidation_TestConditionMultipleSources, "Multiple sources for Test Conditions detected: config file and DTR.",
              "Multiple sources for Test Conditions detected")
GMAP_ERROR_EX(eValidation_InvalidDbTypeForDTRTestcond, "Test condition detected in DTR. Test conditions are not handled with production databases.",
              "Test condition detected in DTR")
// Database errors
GMAP_ERROR_EX(eDB_InvalidTestingStage, "Invalid testing stage (%s).",
              "An invalid testing stage was specified")
GMAP_ERROR_EX(eDB_InvalidTransaction, "Invalid database transaction (%s).",
              "An invalid transaction for multi insertion")
GMAP_ERROR_EX(eDB_VersionMismatch, "TDR version (%s) not matching the version supported by the current plugin (%s).",
              "TDR version older than the version supported by the plugin")
GMAP_ERROR_EX(eDB_UnsupportedDriver, "Specified SQL driver is not supported: %s.",
              "Specified SQL driver is not supported")
GMAP_ERROR_EX(eDB_Connection, "Error connecting to the database %s.",
              "Error connecting to the DB")
GMAP_ERROR_EX(eDB_InvalidConnector, "Database connector is NULL.",
              "Database connector is NULL")
GMAP_ERROR_EX(eDB_OpenSqlLoaderFile, "Error opening file for SQL loader (check write permissions): %s.",
              "Error opening file to write SQL commands for the SQL loader")
GMAP_ERROR_EX(eDB_PacketSizeOverflow, "SQL query too big (max packet size overflow): %s.",
              "Overflow in packet size (SQL query bigger that max allowed size)")
GMAP_ERROR_EX(eDB_Query, "Error executing SQL query.\nQUERY = %s\nERROR = %s",
              "Error executing SQL query")
GMAP_ERROR_EX(eDB_SqlLoader, "Error executing SQL loader: %s.",
              "Error executing SQL loader")
GMAP_ERROR_EX(eDB_NoResult,"SQL query returned no result: %s.",
              "Query returned no result")
GMAP_ERROR_EX(eDB_NoResult_0,"SQL query returned no result.",
              "Query returned no result (don't display query in error message)")
GMAP_ERROR_EX(eDB_InsertionValidationProcedure, "DB validation procedure %s failed (%s).",
              "DB Validation Procedure returned status specifying file is not valid for insertion")
GMAP_ERROR_EX(eDB_InsertionPreProcessingProcedure, "DB preprocessing procedure %s failed (%s).",
              "DB Pre-Processing Stored Procedure returned status specifying file is not valid for insertion")
GMAP_ERROR_EX(eDB_InsertionPostProcessingProcedure, "DB postprocessing procedure %s failed (%s).",
              "DB Post-Processing Stored Procedure returned status specifying file is not valid for insertion")
GMAP_ERROR_EX(eDB_CustomIncrementalProcedure, "DB custom incremental procedure %s failed (%s).",
              "DB Custom Incremental Processing Stored Procedure returned status specifying IncrementalKeyword is not valid")
GMAP_ERROR_EX(eDB_NoMappingTable, "Missing mapping table to DB fields.",
              "Missing DB mapping table")
GMAP_ERROR_EX(eDB_MissingMapping, "Missing mapping to DB fields of following Examinator query fields: %s.",
              "Missing DB mapping of some required Examinator fields")
GMAP_ERROR_EX(eDB_MissingLink, "Following link used in the DB mapping is not defined: %s.",
              "Missing Link in DB mapping")
GMAP_ERROR_EX(eDB_EnumTables, "Error retrieving list of tables for database %s.",
              "Error retrieving list of tables")
GMAP_ERROR_EX(eDB_UpdateMapping, "Error updating mapping for database %s.",
              "Error updating DB mapping")
GMAP_ERROR_EX(eDB_CustomDbUpdate, "Error during custom DB update for %s: %s.",
              "Error during custom DB update")
GMAP_ERROR_EX(eDB_CheckDbVersion, "Error checking DB version: %s.",
              "Error checking DB version")
GMAP_ERROR_EX(eDB_CheckDbStatus, "Error checking DB status: %s.",
              "Error checking DB status")
GMAP_ERROR_EX(eDB_NotUptoDate, "DB is not up-to-date. DB version=%s, version supported by current plug-in=%s.",
              "DB is not up-to-date")
GMAP_ERROR_EX(eDB_NotIncrementalUpdatesPending, "No pending incremental updates.",
              "No incremental updates are pending")
GMAP_ERROR_EX(eDB_Abort, "Operation aborted by user",
              "Operation aborted by user")
GMAP_ERROR_EX(eDB_AliasOnMultiFieldFilter, "Aliases are not supported on multi-field filters (%s).",
              "Aliases are not supported on multi-field filters");
GMAP_ERROR_EX(eDB_ValuesMismatchMultiFieldFilter, "Nb of values doesn't match nb of fields in filter (%s).",
              "Values mismatch on multi-field filters (different nb of values as nb of fields)");
GMAP_ERROR_EX(eDB_NotTimeFieldMapping, "One of the Aggregate fields is not a time type: (%s).",
              "Field from DB mapping is not a time field.");
GMAP_ERROR_EX(eDB_Consolidation, "Flexible Consolidation error: %s.",
              "Error occurs during the consolidation process");
GMAP_ERROR_EX(eDB_Status, "Database Status error: %s.",
              "Error from database status");
// File manipulation
GMAP_ERROR_EX(eWyr_FileOpen, "Failed opening file.",
              "Failed opening WYR file")
GMAP_ERROR_EX(eWyr_FileAlreadyInserted, "WYR file already inserted for site '%s' into %s table.",
              "WYR file already inserted")
GMAP_ERROR_EX(eWyr_MissingSite, "Couldn't find WYR format for site '%s' into %s table.",
              "Couldn't find WYR format for current site")
GMAP_ERROR_EX(eWyr_MissingFormat, "Missing date format for field '%s' into %s table.",
              "Missing date format for WYR filed DATE_IN or DATE_OUT")
GMAP_ERROR_EX(eWyr_IncorrectFormat, "Incorrect format: %s.",
              "Incorrect format for WYR file and WYR table")
// Internal errors
GMAP_ERROR_EX(eInternal_MissingTest, QString("Couldn't find test %d (%s) in internal testlist. Please contact Quantix support at %1.").arg(GEX_EMAIL_SUPPORT).toLatin1().constData(),
              "Couldn't find specified test in the test list created during Pass1")
GMAP_ERROR_EX(eInternal_MissingTestPin, QString("Couldn't find test %d (%s), pin %d in internal testlist. Please contact Quantix support at %1.").arg(GEX_EMAIL_SUPPORT).toLatin1().constData(),
              "Couldn't find specified test with specified pin in the test list created during Pass1")
GMAP_ERROR_EX(eInternal_MissingRun, QString("Couldn't find run %d in run map. Please contact Quantix support at %1.").arg(GEX_EMAIL_SUPPORT).toLatin1().constData(),
              "Couldn't find specified run in run map (initialized in InitMaps)")
// Plug-in specific errors
GMAP_ERROR_EX(eDB_NonSequential_RunID, QString("The difference between 2 consecutive Ut_Id's is greater than 10\n(%s - %s).\nPlease contact Quantix support at %1.").arg(GEX_EMAIL_SUPPORT).toLatin1().constData(),
              "there is a non-sequential Ut_Id in the dataset (allow a difference of 10 between 2 consecutive Ut_Id's)")
GMAP_ERROR_EX(eDB_ConsolidatedFinalTestExtractionError, "An error has occured when extracting consolidated data in final test: \n%s.",
              "Consolidated Final Test Extraction Error")
GMAP_ERROR_EX(eDB_SYALotCheckError, "An error has occured when checking lot yield.",
              "the SYACheckLot failed")
GMAP_ERROR_EX(eDB_ExtractionError, "An error has occured while extracting data from DB.",
              "Build sql failed")
GMAP_ERROR_EX(eDB_SPMComputeLimitsError, "SPM Compute Limits error:\n%s.",
              "The SPM Compute Limits failed")
GMAP_ERROR_EX(eDB_SPMCheckLimitsError, "SPM Check Limits error:\n%s.",
              "The SPM Check Limits failed")
GEND_ERROR_MAP(GexDbPlugin_Base)

QString GexDbPlugin_Base::GetRuleNameFromRuleType(OutlierRule eOutlierRule)
{
    QString strRuleName;
    if(eOutlierRule == 	eNone)					strRuleName = QStringLiteral("None");
    if(eOutlierRule == 	eMeanNSigma)			strRuleName = QStringLiteral("Mean ± N*Sigma");
    if(eOutlierRule == 	eMedianNRobustSigma)	strRuleName = QStringLiteral("Median ± N*RobustSigma");
    if(eOutlierRule == 	eMedianNIQR)			strRuleName = QStringLiteral("Median ± N*IQR");
    if(eOutlierRule == 	ePercentil_0N100)		strRuleName = QStringLiteral("Percentile(N)|Percentile(100-N)");
    if(eOutlierRule == 	eQ1Q3NIQR)				strRuleName = QStringLiteral("Q1/Q3 ± N*IQR");
    if(eOutlierRule == 	eDefault)				strRuleName = QStringLiteral("Default");
    if(eOutlierRule == 	eManual)				strRuleName = QStringLiteral("Manual");
    return strRuleName;
}

OutlierRule GexDbPlugin_Base::GetRuleTypeFromRuleName(QString strRuleName)
{
    OutlierRule eOutlierRule = eNone;
    if(strRuleName.toLower() == QStringLiteral("None").toLower())
        eOutlierRule = eNone;
    else if(strRuleName.toLower() == QStringLiteral("Mean ± N*Sigma").toLower())
        eOutlierRule = eMeanNSigma;
    else if(strRuleName.toLower() == QStringLiteral("Median ± N*RobustSigma").toLower())
        eOutlierRule = eMedianNRobustSigma;
    else if(strRuleName.toLower() == QStringLiteral("Median ± N*IQR").toLower())
        eOutlierRule = eMedianNIQR;
    else if(strRuleName.toLower() == QStringLiteral("Percentile(N)|Percentile(100-N)").toLower())
        eOutlierRule = ePercentil_0N100;
    else if(strRuleName.toLower() == QStringLiteral("Q1/Q3 ± N*IQR").toLower())
        eOutlierRule = eQ1Q3NIQR;
    else if(strRuleName.toLower() == QStringLiteral("Default").toLower())
        eOutlierRule = eDefault;
    else if(strRuleName.toLower() == QStringLiteral("Manual").toLower())
        eOutlierRule = eManual;
    return eOutlierRule;
}

OutlierRule GexDbPlugin_Base::GetOutlierAlgorithmType(const QString & codingName)
{
    if(codingName == C_OUTLIERRULE_MEAN_N_SIGMA)
        return eMeanNSigma;
    if(codingName == C_OUTLIERRULE_MEDIAN_N_ROBUSTSIGMA)
        return eMedianNRobustSigma;
    if(codingName == C_OUTLIERRULE_MEDIAN_N_IQR)
        return eMedianNIQR;
    if(codingName == C_OUTLIERRULE_PERCENTILE_N)
        return ePercentil_0N100;
    if(codingName == C_OUTLIERRULE_Q1Q3_N_IQR)
        return eQ1Q3NIQR;
    if(codingName == C_OUTLIERRULE_DEFAULT)
        return eDefault;
    if(codingName == C_OUTLIERRULE_MANUAL)
        return eManual;
    return eNone;
}

GexDbPlugin_Base::GexDbPlugin_Base(const QString & strHostName,
                                   const QString & strApplicationPath,
                                   const QString & strUserProfile,
                                   const QString & strLocalFolder,
                                   const char *gexLabelFilterChoices[],
                                   const bool bCustomerDebugMode,
                                   CGexSkin * pGexSkin,
                                   const GexScriptEngine* gse, /*=NULL*/
                                   GexDbPlugin_Connector *pclDatabaseConnector/*=NULL*/):
    QObject(NULL)
{
    mGexScriptEngine=gse;
    //qDebug(QString("GexDbPlugin_Base::GexDbPlugin_Base"));
    // Init list of GEX fields
    m_gexLabelFilterChoices = gexLabelFilterChoices;
    for (int i=1; gexLabelFilterChoices[i]; i++)
        m_strlGexFields.append(gexLabelFilterChoices[i]);

    // Init some variables
    m_pGexSkin = pGexSkin;
    m_strHostName = strHostName;
    m_strApplicationPath = strApplicationPath;
    m_strUserProfile = strUserProfile;
    m_strLocalFolder = strLocalFolder;
    mParentWidget = NULL;

    m_bProfilerON = false;
    m_pmapFields_GexToRemote = NULL;
    m_pmapLinks_Remote = NULL;
    m_uiInsertionValidationOptionFlag = GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_ALL;
    m_uiInsertionValidationFailOnFlag = GEXDB_PLUGIN_INSERTIONVALIDATION_OPTION_FAILON_YIELDMAN;
    m_bCustomerDebugMode = bCustomerDebugMode;
    m_eStatsSource = eStatsFromSummaryThenSamples;
    m_strQuery = "";
    mDbArchitecture = NULL;
    mQueryEngine = NULL;
    mQueryProgress = NULL;

    // Should we create a Connector??
    m_pclDatabaseConnector = NULL;
    m_bPrivateConnector = true;

    if(pclDatabaseConnector != NULL)
        m_pclDatabaseConnector = pclDatabaseConnector;
}

GexDbPlugin_Base::~GexDbPlugin_Base()
{
    // Should we destroy the Connector ??
    if(m_bPrivateConnector && m_pclDatabaseConnector)
    {
        delete m_pclDatabaseConnector;
    }

    if (mQueryEngine != NULL)
    {
        delete mQueryEngine;
    }

    mParentWidget = NULL;
// DO NOT DELETE: already deleted by parent: GexMainWindow(Gui)/pluginBase(Core)
//    delete mQueryProgress;
}

///////////////////////////////////////////////////////////
// Set db login mode (std user/admin user)
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::SetAdminLogin(bool bAdminLogin)
{
    if(m_pclDatabaseConnector)
        m_pclDatabaseConnector->SetAdminLogin(bAdminLogin);
}

//////////////////////////////////////////////////////////////////////
// Check if Database up-to-date (schema...)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
// Check major/minor/build version
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::IsDbUpToDate(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                    unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name,
                                    unsigned int *puiLatestSupportedDbVersion_Build)
{
    // Init return variables
    *pbDbIsUpToDate = true;

    // Init latest supported version
    strLatestSupportedDbVersion_Name	= "<unknown>";
    *puiLatestSupportedDbVersion_Build	= 0;
    strCurrentDbVersion_Name = "<unknown>";
    *(puiCurrentDbVersion_Build) = 0;

    return true;
}
//////////////////////////////////////////////////////////////////////
// Check major/minor version
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::IsDbUpToDateForInsertion(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                                unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name,
                                                unsigned int *puiLatestSupportedDbVersion_Build)
{
    // Init return variables
    *pbDbIsUpToDate = true;

    // Init latest supported version
    strLatestSupportedDbVersion_Name	= "<unknown>";
    *puiLatestSupportedDbVersion_Build	= 0;
    strCurrentDbVersion_Name = "<unknown>";
    *(puiCurrentDbVersion_Build) = 0;

    return true;
}
//////////////////////////////////////////////////////////////////////
// Check major version
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::IsDbUpToDateForExtraction(bool *pbDbIsUpToDate, QString & strCurrentDbVersion_Name,
                                                 unsigned int *puiCurrentDbVersion_Build, QString & strLatestSupportedDbVersion_Name,
                                                 unsigned int *puiLatestSupportedDbVersion_Build)
{
    // Init return variables
    *pbDbIsUpToDate = true;

    // Init latest supported version
    strLatestSupportedDbVersion_Name	= "<unknown>";
    *puiLatestSupportedDbVersion_Build	= 0;
    strCurrentDbVersion_Name = "<unknown>";
    *(puiCurrentDbVersion_Build) = 0;

    return true;
}

//////////////////////////////////////////////////////////////////////
// Update Database (schema...)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::UpdateDb(QString /*command*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "UpdateDb()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Update Database (schema...): incremental update
// This function, if supported, must be overloaded in the derived class!!
// eTestingStage:
//		0 : to update all testingstage
//		else : to update only one testing stage
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::UpdateConsolidationProcess(int /*eTestingStage = 0*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "UpdateConsolidationProcess()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Purge selected splitlots
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::PurgeSplitlots(QStringList& /*strlSplitlots*/,
                                      QString& /*strTestingStage*/,
                                      QString& /*strCaseTitle*/,
                                      QString* /*pstrLog = NULL*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "PurgeSplitlots()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Global options info
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetGlobalOptionName(int /*nOptionNb*/,
                                           QString& /*strValue*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionName()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionValue(int /*nOptionNb*/,
                                            QString& /*strValue*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionValue()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionValue(QString /*strOptionName*/,
                                            QString& /*strValue*/)
{
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionValue()");

    return false;
}

bool GexDbPlugin_Base::SetGlobalOptionValue(QString /*strOptionName*/,
                                            QString& /*strValue*/)
{
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "SetGlobalOptionValue()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionValue(int /*nOptionNb*/,
                                            QString& /*strValue*/,
                                            bool& /*bIsDefined*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionValue()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionTypeValue(int /*nOptionNb*/,
                                                QString& /*strValue*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionTypeValue()");

    return false;
}

bool GexDbPlugin_Base::GetStorageEngineName(QString & /*strStorageEngine*/,QString & /*strStorageFormat*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetStorageEngineName()");

    return false;
}

bool GexDbPlugin_Base::GetSecuredMode(QString &/*strSecuredMode*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetSecuredMode()");

    return false;
}

bool GexDbPlugin_Base::UpdateSecuredMode(QString /*strSecuredMode*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "UpdateSecuredMode()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionDefaultValue(int /*nOptionNb*/,
                                                   QString& /*strValue*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionDefaultValue()");

    return false;
}

bool GexDbPlugin_Base::IsGlobalOptionValidValue(int /*nOptionNb*/,
                                                   QString& /*strValue*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "IsGlobalOptionValidValue()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionDescription(int /*nOptionNb*/,
                                                  QString& /*strValue*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionDescription()");

    return false;
}

bool GexDbPlugin_Base::GetGlobalOptionReadOnly(int /*nOptionNb*/,
                                               bool& /*bIsReadOnly*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetGlobalOptionReadOnly()");

    return false;
}

bool GexDbPlugin_Base::GetTotalSize(int& /*size*/)
{
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetTotalSize()");
    return false;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified wafer
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConsolidateWafer(QString& /*strLotID*/,
                                        QString& /*strWaferID*/,
                                        QString& /*strCaseTitle*/,
                                        QString* /*pstrLog = NULL*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ConsolidateWafer()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Consolidate all wafers for specified lot
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConsolidateWafers(QString& /*strLotID*/,
                                         QString& /*strCaseTitle*/,
                                         QString* /*pstrLog = NULL*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ConsolidateWafer()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Consolidate specified lot
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConsolidateLot(QString& /*strLotID*/,
                                      bool /*bConsolidateOnlySBinTable*/,
                                      bool /*bCallConsolidationFunction*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ConsolidateLot()");

    return false;
}

bool GexDbPlugin_Base::purgeDataBase(GexDbPlugin_Filter & /*cFilters*/,GexDbPlugin_SplitlotList &/*oSplitLotList*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "invalidateSplitlots()");

    return false;
}

bool GexDbPlugin_Base::exportCSVCondition(const QString &/*strCSVFileName*/, const QMap<QString, QString>& /*oConditions*/, GexDbPlugin_Filter &/*roFilters*/, GexDbPlugin_SplitlotList &/*oSplitLotList*/, QProgressDialog */*poProgress*/){
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "invalidateSplitlots()");

    return false;
}

bool GexDbPlugin_Base::FlagSplitlotsForIncrementalUpdate( const QStringList &/*splitlots*/, const QString &/*incrementalKey*/)
{ return true; }

bool GexDbPlugin_Base::UnFlagSplitlotsForIncrementalUpdate( const QStringList &/*splitlots*/, const QString &/*incrementalKey*/)
{ return true; }

bool GexDbPlugin_Base::SwitchFlagSplitlotsForIncrementalUpdate( const QStringList &/*splitlots*/, const QString &/*incrementalOldKey*/, const QString &/*incrementalNewKey*/)
{ return true; }

//////////////////////////////////////////////////////////////////////
// Run a specific incremental update
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::IncrementalUpdate(QString /*incrementalName*/, QString /*testingStage*/, QString /*target*/, QMap< QString, QString >  &/*summary*/)
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetIncrementalUpdatesCount(bool /*checkDatabase*/,
                                             int &incrementalSplitlots)
{
    incrementalSplitlots = 0;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetNextAutomaticIncrementalUpdatesList(QMap< QString,QMap< QString,QStringList > > & /*incrementalUpdatesList*/)
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// Get incremental updates
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetFirstIncrementalUpdatesList(QString /*incrementalName*/, QMap< QString,QMap< QString,QStringList > > & /*incrementalUpdatesList*/)
{
    return true;
}


//////////////////////////////////////////////////////////////////////
// Get incremental updates
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > &/*incrementalUpdates*/)
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// Set incremental updates
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::SetIncrementalUpdatesSettings(QMap< QString,QMap< QString,QString > > & /*incrementalUpdates*/)
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// Check incremental updates property
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::IsIncrementalUpdatesSettingsValidValue(QString &/*name*/, QString &/*value*/)
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// Insert Stdf File into the database
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool
GexDbPlugin_Base::InsertDataFile(struct GsData* ,
                                 int /*lSqliteSplitlotId*/,
                                 const QString& /*strDataFileName*/,
                                      GS::QtLib::DatakeysEngine& /*dbKeysEngine*/,
                                      bool* /*pbDelayInsertion*/,
                                      long* /*plSplitlotID*/,
                                      int* /*pnTestingStage*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "InsertStdfFile()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Insert Weekly Yield Report File into the database
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::InsertWyrDataFile(const QString& /*strDataFileName*/,
                                         const QString& /*strSiteName*/,
                                         const QString& /*strTestingStage*/,
                                         unsigned int /*uiWeekNb*/,
                                         unsigned int /*uiYear*/,
                                         bool* /*pbDelayInsertion*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "InsertWyrDataFile()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Insert an alarm for specified splitlot/testing stage
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::InsertAlarm(long /*lSplitlotID*/,
                                   int /*nTestingStage*/,
                                   GexDbPlugin_Base::AlarmCategories /*eAlarmCat*/,
                                   GexDbPlugin_Base::AlarmLevels /*eAlarmLevel*/,
                                   long /*lItemNumber*/,
                                   QString& /*strItemName*/,
                                   unsigned int /*uiFlags*/,
                                   float /*fLCL*/,
                                   float /*fUCL*/,
                                   float /*fValue*/,
                                   QString /*strUnits*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "InsertAlarm()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Insert an alarm for specified splitlot (wafer testing stage)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::InsertAlarm_Wafer(long /*lSplitlotID*/,
                                         GexDbPlugin_Base::AlarmCategories /*eAlarmCat*/,
                                         GexDbPlugin_Base::AlarmLevels /*eAlarmLevel*/,
                                         long /*lItemNumber*/,
                                         QString& /*strItemName*/,
                                         unsigned int /*uiFlags*/,
                                         float /*fLCL*/,
                                         float /*fUCL*/,
                                         float /*fValue*/,
                                         QString /*strUnits*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "InsertAlarm_Wafer()");

    return false;
}


//////////////////////////////////////////////////////////////////////
// Get data for Paroduction - UPH graph
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetDataForProd_UPH(GexDbPlugin_Filter& /*cFilters*/,
                                          GexDbPlugin_XYChartList& /*clXYChartList*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetDataForProd_UPH()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Get data for Production - Yield graph
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetDataForProd_Yield(GexDbPlugin_Filter& /*cFilters*/,
                                            GexDbPlugin_XYChartList& /*clXYChartList*/,
                                            int /*nBinning*/,
                                            bool /*bSoftBin*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetDataForProd_Yield()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Yield, UPH)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ER_Prod_GetParts(GexDbPlugin_Filter& /*cFilters*/,
                                        GexDbPlugin_ER_Parts& /*clER_PartsData*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ER_Prod_GetParts()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Advanced Enterprise Report
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::AER_GetDataset(const GexDbPluginERDatasetSettings& /*datasetSettings*/,
                                      GexDbPluginERDataset& /*datasetResult*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "AER_GetDataset()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Genealogy - Yield vs Yield)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ER_Genealogy_YieldVsYield_GetParts(GexDbPlugin_Filter& /*cFilters*/,
                                                          GexDbPlugin_ER_Parts& /*clER_PartsData*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ER_Genealogy_YieldVsYield_GetParts()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Compute data for Enterprise Report graphs (Genealogy - Yield vs Parameter)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ER_Genealogy_YieldVsParameter_GetParts(
        GexDbPlugin_Filter& /*cFilters*/,
        GexDbPlugin_ER_Parts& /*clER_PartsData*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ER_Genealogy_YieldVsParameter_GetParts()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Get bin counts for Enterprise Report graphs (Yield, UPH)
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ER_Prod_GetBinnings(GexDbPlugin_Filter& /*cFilters*/,
                                           GexDbPlugin_ER_Parts& /*clER_PartsData*/,
                                           GexDbPlugin_ER_Parts_Graph* /*pGraph*/,
                                           GexDbPlugin_ER_Parts_Layer* /*pLayer*/,
                                           const QString& /*strAggregateLabel*/,
                                           GexDbPlugin_BinList& /*clBinList*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "ER_Prod_GetBinnings()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Get data for Production - Consolidated Yield graph
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetDataForProd_ConsolidatedYield(GexDbPlugin_Filter& /*cFilters*/,
                                                        GexDbPlugin_XYChartList& /*clXYChartList*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetDataForProd_ConsolidatedYield()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Get data for WYR - Standard report
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::GetDataForWyr_Standard(GexDbPlugin_Filter& /*cFilters*/,
                                              GexDbPlugin_WyrData& /*cWyrData*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "GetDataForWyr_Standard()");

    return false;
}

bool GexDbPlugin_Base::QuerySplitlots(GexDbPlugin_Filter& /*cFilters*/,
                                      GexDbPlugin_SplitlotList& /*clSplitlotList*/, bool /*bPurge*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "QuerySplitlots()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// Execute Query and return results
// This function, if supported, must be overloaded in the derived class!!
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QuerySQL(QString& /*strQuery*/,
                                QList<QStringList>& /*listResults*/)
{
    // Set error and return
    QString strPluginName;
    GetPluginName(strPluginName);
    GSET_ERROR2(GexDbPlugin_Base, eFunctionNotSupported, NULL, strPluginName.toLatin1().constData(), "QuerySQL()");

    return false;
}

//////////////////////////////////////////////////////////////////////
// // Transfer remote data files to local FS
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::TransferDataFiles(tdGexDbPluginDataFileList &cMatchingFiles, const QString & strLocalDir)
{
    // Check if list of files is empty
    if(cMatchingFiles.isEmpty())
        return true;	// No file to download

    // Create Ftp transfer object
    GexDbFtpTransferDialog clFtpTransfer(m_strApplicationPath, strLocalDir, &cMatchingFiles, m_pGexSkin, mParentWidget);

    // Display dialog box (unless no file to download)
    if(clFtpTransfer.Start())
        clFtpTransfer.exec();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Returns details about last error
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::GetLastError(QString & strError)
{
    strError = GGET_LASTERRORMSG(GexDbPlugin_Base,this);
}

///////////////////////////////////////////////////////////
// Get stack of insertion warnings (not appropriate in the base class)
// Only plugins supporting insertion must override this function
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::GetWarnings(QStringList & strlWarnings)
{
    strlWarnings.clear();
}

void GexDbPlugin_Base::SetParentWidget(QWidget *parentWidget)
{
    mParentWidget = parentWidget;
}

bool GexDbPlugin_Base::Init()
{
    // Init time for ProcessEvents function
    mLastProcessEventsTime.start();
    if(m_pclDatabaseConnector != NULL)
    {
        m_bPrivateConnector = false;
        // Set ptr to plugin base parent
        m_pclDatabaseConnector->SetPluginBasePtr(this);
    }

    // If not in daemon mode Create SQL progress dialog
    if (!mGexScriptEngine->property("GS_DAEMON").toBool())
        mQueryProgress = new GS::DbPluginBase::QueryProgressGui(m_pGexSkin,mParentWidget);
    else
        mQueryProgress = new GS::DbPluginBase::QueryProgressCore(this);

    return true;
}

///////////////////////////////////////////////////////////
// Update progress bar
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::IncrementProgress(int prog /*= 1*/)
{
    mProgress += prog;
    emit sUpdateProgress(mProgress);
    QCoreApplication::processEvents();
}

///////////////////////////////////////////////////////////
// Reset progress bar
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::ResetProgress(bool forceCompleted)
{
    if(forceCompleted == true)
        mProgress = 100;
    else
        mProgress = 0;
    emit sResetProgress(mProgress);

    ProcessEvents();
}

void GexDbPlugin_Base::SetMaxProgress(int prog)
{
    emit sMaxProgress(prog);

    ProcessEvents();
}

void GexDbPlugin_Base::SetProgress(int prog)
{
    emit sUpdateProgress(prog);

    ProcessEvents();
}

void GexDbPlugin_Base::ProcessEvents()
{
    // Check interval
    if(mLastProcessEventsTime.elapsed() > PROCESS_EVENTS_INTERVAL)
    {
        QCoreApplication::processEvents();
        mLastProcessEventsTime.start();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Encrypt password
/////////////////////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::CryptPassword(const QString & strPassword, QString & strCryptedPasswordHexa)
{
    // Check string to encrypt
    if(strPassword.isEmpty())
    {
        strCryptedPasswordHexa = "";
        return;
    }

    QString		strHexaChar;
    CGMemBuffer	clPasswordBuffer, clCryptedPasswordBuffer;
    CGCrypto	clCrypto(GEXDB_PLUGIN_CRYPTING_KEY);

    // If encryption fails, strCryptedPassword will contain the plain password
    strCryptedPasswordHexa = strPassword;

    // Encrypt password
    clPasswordBuffer.CreateBuffer(strPassword.length()+5);
    clPasswordBuffer.SetDataMode(CGMemBuffer::eModeLittleEndian);
    if(clPasswordBuffer.WriteString(strPassword.toLatin1().data()) == FALSE)	return;
    if(clCrypto.Encrypt(clPasswordBuffer, clCryptedPasswordBuffer) == FALSE)	return;

    // Get encrypted string, and convert every character to hexadecimal format
    unsigned int uiLength = clCryptedPasswordBuffer.GetBufferSize();
    const BYTE *pData = clCryptedPasswordBuffer.GetData();
    strCryptedPasswordHexa = "";
    while(uiLength-- > 0)
    {
        strHexaChar.sprintf("%02X", *pData);
        strCryptedPasswordHexa += strHexaChar;
        pData++;
    }

#ifdef QT_DEBUG
#ifdef CASE4882
    // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
    // DO NOT ADD THIS GSLOG in RELEASE VERSION
    QString strLog = QString("Plain string=") + clPasswordBuffer.GetRawBufferToString(CGMemBuffer::eStringModeHex).c_str();
    GSLOG(SYSLOG_SEV_DEBUG, strLog.toLatin1().data());
    strLog = QString("Plain string=") + clPasswordBuffer.GetRawBufferToString(CGMemBuffer::eStringModeAscii).c_str();
    GSLOG(SYSLOG_SEV_DEBUG, strLog);
    strLog = QString("Crypted string=") + clCryptedPasswordBuffer.GetRawBufferToString(CGMemBuffer::eStringModeHex).c_str();
    GSLOG(SYSLOG_SEV_DEBUG, strLog);
#endif
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
// Decrypt password
/////////////////////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::DecryptPassword(const QString & strCryptedPasswordHexa, QString & strPassword)
{
    // Check string to decrypt
    if(strCryptedPasswordHexa.isEmpty())
    {
        strPassword = "";
        return;
    }

    CGMemBuffer		clPasswordBuffer, clCryptedPasswordBuffer;
    CGCrypto		clCrypto(GEXDB_PLUGIN_CRYPTING_KEY);
    const char		*pData;
    unsigned int	uiData;
    unsigned int	uiLengthHexa, uiLength;
    BYTE			*pCryptedBuffer;
    char			*pUncryptedBuffer;

    // Convert crypted password from hexa
    //pData = strCryptedPasswordHexa.latin1();
    QByteArray qByte= strCryptedPasswordHexa.toLatin1();
    pData = qByte.constData();
    uiLengthHexa = strCryptedPasswordHexa.length();

    pCryptedBuffer = new BYTE[uiLengthHexa];

    for(uiLength = 0; uiLengthHexa > 0; uiLength++, uiLengthHexa-=2)
    {
        sscanf(pData, "%02X", &uiData);
        pCryptedBuffer[uiLength] = (char)uiData;
        pData += 2;

        // case 5101
        // Check if the crypted password is not truncated
        if(uiLengthHexa < 2)
            break;
    }

    // Decrypt password
    clCryptedPasswordBuffer.CopyIn(pCryptedBuffer, uiLength, uiLength);
    clCrypto.Decrypt(clCryptedPasswordBuffer, clPasswordBuffer);
    clPasswordBuffer.SetDataMode(CGMemBuffer::eModeLittleEndian);
    uiLength = clPasswordBuffer.GetBufferSize();
    pUncryptedBuffer = new char[uiLength];
    //clPasswordBuffer.SetCurrentPos(0);      // pyc, 19/09/2011, debug test
    clPasswordBuffer.ReadString(pUncryptedBuffer, uiLength);
    strPassword = pUncryptedBuffer;

#ifdef QT_DEBUG
#ifdef BERNARD_GARROS
    // BG case 4882: Log for Debug (use EMERGENCY so that when starting with -LOGLEVEL_0, we have only these messages for easy debug)
    // DO NOT ADD THIS GSLOG in RELEASE VERSION
    QString strLog = QString("Crypted string=") + clCryptedPasswordBuffer.GetRawBufferToString(CGMemBuffer::eStringModeHex).c_str();
    GSLOG(SYSLOG_SEV_EMERGENCY, strLog);
    strLog = QString("Plain string=") + clPasswordBuffer.GetRawBufferToString(CGMemBuffer::eStringModeHex).c_str();
    GSLOG(SYSLOG_SEV_EMERGENCY, strLog);
    strLog = QString("Plain string=") + clPasswordBuffer.GetRawBufferToString(CGMemBuffer::eStringModeAscii).c_str();
    GSLOG(SYSLOG_SEV_EMERGENCY, strLog);
#endif
#endif

    delete [] pCryptedBuffer;
    delete [] pUncryptedBuffer;
}


//////////////////////////////////////////////////////////////////////
bool  GexDbPlugin_Base::ConfigWizard(QMap<QString,QString> info,
                                     QMap<QString,QString> guiOptions)
{
    mGexInfoMap = info;
    mWizardGuiOptionsMap = guiOptions;

    return ConfigWizard();
}

//////////////////////////////////////////////////////////////////////
// Load settings from DomElement and init the calls variables
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::LoadSettingsFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("PluginConfig");
    if (elt.isNull())
        return false;

    // Create Connector, unless not privately owned
    if(m_bPrivateConnector)
    {
        if(m_pclDatabaseConnector)
            delete m_pclDatabaseConnector;

        // Create a new connector
        m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName, this);

        // Load connector settings
        if(!m_pclDatabaseConnector->LoadSettingsFromDom(elt))
        {
            GSET_ERROR0(GexDbPlugin_Base, eReadSettings, GGET_LASTERROR(GexDbPlugin_Connector, m_pclDatabaseConnector));
            return false;
        }
    }

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Load plugin settings from file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::LoadSettings(QFile *pSettingsFile)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" from %1").arg( pSettingsFile->fileName()).toLatin1().constData() );

    // Create Connector, unless not privately owned
    if(m_bPrivateConnector)
    {
        if(m_pclDatabaseConnector)
            delete m_pclDatabaseConnector;

        // Create a new connector
        m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName, this);
        m_pclDatabaseConnector->m_strSettingsFile = pSettingsFile->fileName();

        // Load connector settings
        if(!m_pclDatabaseConnector->LoadSettings(pSettingsFile))
        {
            GSET_ERROR0(GexDbPlugin_Base, eReadSettings, GGET_LASTERROR(GexDbPlugin_Connector, m_pclDatabaseConnector));
            return false;
        }
    }

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create a new connector instance
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::CreateConnector()
{
    // Create Connector, unless not privately owned
    if(m_bPrivateConnector)
    {
        if(m_pclDatabaseConnector)
            delete m_pclDatabaseConnector;

        // Create a new connector
        m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName, this);
    }

    // Success
    return true;
}

QDomElement GexDbPlugin_Base::GetSettingsDom(QDomDocument &doc)
{
    QDomElement eltPluginConfig = doc.createElement("PluginConfig");
    // Write connector settings
    if(m_pclDatabaseConnector)
        eltPluginConfig.appendChild(m_pclDatabaseConnector->GetSettingsDom(doc));

    return eltPluginConfig;
}

//////////////////////////////////////////////////////////////////////
// Write plugin settings to file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::WriteSettings(QTextStream *phFile)
{
    // Write connector settings
    if(m_pclDatabaseConnector)
    {
        if(!m_pclDatabaseConnector->WriteSettings(phFile))
        {
            GSET_ERROR0(GexDbPlugin_Base, eWriteSettings, GGET_LASTERROR(GexDbPlugin_Connector, m_pclDatabaseConnector));
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Connect to corporate DB...
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConnectToCorporateDb()
{
    //qDebug("GexDbPlugin_Base::ConnectToCorporateDb: %s", m_pclDatabaseConnector->IsConnected()?"(already connected)":"");

    // Make sure ptr not null
    if(m_pclDatabaseConnector == NULL)
    {
        // Error: set error and return false
        GSET_ERROR0(GexDbPlugin_Base, eDB_InvalidConnector, NULL);
        return false;
    }

    // Check if already connected
    if(m_pclDatabaseConnector->IsConnected())
        return true;

    // Connect
    if(!m_pclDatabaseConnector->Connect())
    {
        // Error: set error and return false
        GSET_ERROR1(GexDbPlugin_Base, eDB_Connection, GGET_LASTERROR(GexDbPlugin_Connector, m_pclDatabaseConnector), m_pclDatabaseConnector->m_strDatabaseName.toLatin1().constData());
        return false;
    }

    return true;
}

QString GexDbPlugin_Base::Query_ComputeDateConstraints(GexDbPlugin_Filter &cFilters)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Compute Date Constraints for TimePeriod %1")
          .arg( cFilters.iTimePeriod ).toLatin1().constData());
    QDateTime clFromDateTime, clToDateTime;

    // Init time period filter flag
    cFilters.bUseTimePeriod = true;

    // Check the Calendar period to scan for
    switch(cFilters.iTimePeriod)
    {
    case GEX_QUERY_TIMEPERIOD_TODAY:
        clFromDateTime = QDateTime::currentDateTime();
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_LAST2DAYS:
        clFromDateTime = QDateTime::currentDateTime().addDays(-1);
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_LAST3DAYS:
        clFromDateTime = QDateTime::currentDateTime().addDays(-2);
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_LAST7DAYS:
        clFromDateTime = QDateTime::currentDateTime().addDays(-6);
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_LAST14DAYS:
        clFromDateTime = QDateTime::currentDateTime().addDays(-13);
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_LAST31DAYS:
        clFromDateTime = QDateTime::currentDateTime().addMonths(-1);
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_LAST_N_X:
        GSLOG(SYSLOG_SEV_DEBUG, QString("GEX_QUERY_TIMEPERIOD_LAST_N_X: TimeFactor = %1 TimeStep=%2...").arg(
                 cFilters.iTimeNFactor, cFilters.m_eTimeStep).toLatin1().constData());
        switch (cFilters.m_eTimeStep)
        {
        case GexDbPlugin_Filter::DAYS:
            clFromDateTime = QDateTime::currentDateTime().addDays( -cFilters.iTimeNFactor );
            break;
        case GexDbPlugin_Filter::WEEKS:
            clFromDateTime = QDateTime::currentDateTime().addDays( -cFilters.iTimeNFactor*7 );
            break;
        case GexDbPlugin_Filter::MONTHS:
            clFromDateTime = QDateTime::currentDateTime().addMonths(-cFilters.iTimeNFactor );
            break;
        case GexDbPlugin_Filter::QUARTERS:
            clFromDateTime = QDateTime::currentDateTime().addMonths(-cFilters.iTimeNFactor*4);
            break;
        case GexDbPlugin_Filter::YEARS:
            clFromDateTime = QDateTime::currentDateTime().addYears(-cFilters.iTimeNFactor);
            break;
        default:
            GSLOG(SYSLOG_SEV_ERROR, QString("Unknown time period step %1")
                  .arg( cFilters.m_eTimeStep).toLatin1().constData());
            break;
        }
        clFromDateTime.setTime(QTime(0, 0));

        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;
    case GEX_QUERY_TIMEPERIOD_THISWEEK:
        clFromDateTime = QDateTime::currentDateTime().addDays(1-QDate::currentDate().dayOfWeek());
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_THISMONTH:
        clFromDateTime = QDateTime::currentDateTime().addDays(1-QDate::currentDate().day());
        clFromDateTime.setTime(QTime(0, 0));
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;

    case GEX_QUERY_TIMEPERIOD_CALENDAR:
        clFromDateTime.setDate(cFilters.calendarFrom);
        clFromDateTime.setTime(cFilters.calendarFrom_Time);
        clToDateTime.setDate(cFilters.calendarTo);
        clToDateTime.setTime(cFilters.calendarTo_Time);
        break;

    default:
    case GEX_QUERY_TIMEPERIOD_ALLDATES:
        cFilters.bUseTimePeriod = false;
        clFromDateTime.setTime_t(0);
        clToDateTime = QDateTime::currentDateTime();
        clToDateTime.setTime(QTime(23,59,59));
        break;
    }

    cFilters.tQueryTo=QDateTime::currentDateTime().toTime_t();
    cFilters.tQueryFrom=0;
    // Set variables in filter struct
    cFilters.tQueryFrom = clFromDateTime.toTime_t();
    if (cFilters.tQueryFrom==-1)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("toTime_t() on From date %1 failed")
              .arg( clFromDateTime.toString(Qt::ISODate)).toLatin1().constData() );
        cFilters.tQueryFrom=0;
        //return "error: impossible to convert FromDate ";
    }

    cFilters.tQueryTo	= clToDateTime.toTime_t();
    if (cFilters.tQueryTo==-1)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("toTime_t() on To date %1 failed")
              .arg( clToDateTime.toString(Qt::ISODate)).toLatin1().constData() );
        cFilters.tQueryTo=QDateTime::currentDateTime().toTime_t();
        return "error: impossible to convert ToDate ";
    }
    return "ok";
}

///////////////////////////////////////////////////////////
// Query: empty lists used to build SQL queries
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_Empty()
{
    m_strlQuery_LinkConditions.clear();
    m_strlQuery_ValueConditions.clear();
    m_strlQuery_Fields.clear();
    m_strlQuery_OrderFields.clear();
    m_strlQuery_GroupFields.clear();
    m_mapQuery_TableAliases.clear();
    m_strQuery = "";

    return true;
}

//////////////////////////////////////////////////////////////////////
// Construct SQL string for a numerical filter
//
// syntax for the filter		: '>'<value> OR '<'<value> OR '>='<value> OR '<='<value> OR <value1> OR <value1>,<value2> OR <value1>-<value2> OR any combination
//
// SQL syntax					: "((<DbField> IN (100,101,102)) OR (<DbField> BETWEEN 160 AND 165))"
//
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::Query_BuildSqlString_NumericFilter(const QString & strDbField,
                                                          const QString & strFilter,
                                                          QString & strCondition,
                                                          bool bNegation/*=false*/)
{
    strCondition = "";
    if(strFilter.isEmpty())
    {
        strCondition += strDbField;
        if(bNegation)
            strCondition += " != ''";
        else
            strCondition += " = ''";
        return;
    }

    // First check if we have a GT (>) or LT (<) filter
    QString strTemp = strFilter.trimmed();
    if((strTemp.startsWith(">")) || (strTemp.startsWith("<"))
            || (strTemp.startsWith(">=")) || (strTemp.startsWith("<="))  ) // || (strTemp.startsWith("!="))
    {
        if (bNegation)
        {
            strCondition = "NOT";
        }
        strCondition += "(";
        strCondition += strDbField;
        strCondition += strTemp;
        strCondition += ")";
        return;
    }

    // Go through all elements of the filter
    QStringList::Iterator	itFields;
    QStringList				lFields = strFilter.split(QRegExp("[,;]"));
    QString					lFilterValues;
    bool					lIsSingleFilterValue = true;

    // 1-4,10,14
    // Final Expression => 1,2,3,4,10,14
    // Final Expression => (field BETWEEN 1 AND 4) OR field IN (10,14)
    for(itFields = lFields.begin(); itFields != lFields.end(); itFields++)
    {
        QString lSinglePattern = "-?\\d*\\.?\\d+|-?\\d+\\.?\\d*";
        QRegExp lRegExpSingleValue(QString("^" + lSinglePattern + "$"));
        QRegExp lRegExpInterval(QString("^(" + lSinglePattern + ")-(" + lSinglePattern + ")$"));

        // test single value
        if (lRegExpSingleValue.exactMatch(*itFields))
        {
            // 10 or 14
            // concate 10 to the final Expression
            if(lFilterValues.isEmpty())
                lFilterValues = *itFields;
            else
            {
                lIsSingleFilterValue = false;
                lFilterValues += "," + *itFields;
            }
            // => list of int for IN expression
        }
        else if (lRegExpInterval.exactMatch(*itFields))
        {
            QString lBeginValue, lEndValue;
            double lFirstValue, lSecondValue;
            lFirstValue = lRegExpInterval.cap(1).toDouble();
            lSecondValue = lRegExpInterval.cap(2).toDouble();
            // re-order if needed
            if (lFirstValue > lSecondValue)
            {
                lBeginValue = lRegExpInterval.cap(2);
                lEndValue = lRegExpInterval.cap(1);
            }
            else
            {
                lBeginValue = lRegExpInterval.cap(1);
                lEndValue = lRegExpInterval.cap(2);
            }

            if(!strCondition.isEmpty())
            {
                strCondition += " OR ";

            }
            strCondition += "(";
            strCondition += strDbField;
            strCondition += " BETWEEN ";
            strCondition += lBeginValue;
            strCondition += " AND ";
            strCondition += lEndValue;
            strCondition += ")";
            // => WHERE clause for BETWEEN expression
        }
        else
            GSLOG(SYSLOG_SEV_ERROR, QString("Used for %1, %2 is not recognized as a valid numeric value or interval").
                   arg(strDbField).
                   arg(*itFields).toLatin1().data());
    }

    // Check if values in the list of individual values
    if(!lFilterValues.isEmpty())
    {
        if(!strCondition.isEmpty())
        {
            strCondition += " OR ";
            strCondition += "(";
        }
        else
        {
            strCondition = "(";
        }
        strCondition += strDbField;

        if(lIsSingleFilterValue)
        {
            strCondition += "=";
            strCondition += lFilterValues;
            strCondition += ")";
        }
        else
        {

            strCondition += " IN (";
            strCondition += lFilterValues;
            strCondition += "))";
        }

    }

    if(bNegation)
    {
        strCondition = "NOT (" + strCondition + ")";
    }
}

//////////////////////////////////////////////////////////////////////
// Construct SQL string for a numerical filter
//
// syntax for the filter		: <expression1>#GEXDB#TO#<expression2>
//
// SQL syntax					: "(<DbField> BETWEEN <expression1> AND <expression2>))"
//
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::Query_BuildSqlString_ExpressionRange(const QString & strDbField, const QString & strFilter, QString & strCondition)
{
    strCondition = "";

    // Get the 2 parts of the range
    QStringList strlInterval = strFilter.split(QRegExp("#GEXDB#TO#"));
    if(strlInterval.count() != 2)
        return;

    // Compute SQL condition
    QString strBeginValue, strEndValue;
    strBeginValue = strlInterval[0];
    strEndValue = strlInterval[1];
    strCondition = "(";
    strCondition += strDbField;
    strCondition += " BETWEEN ";
    strCondition += strBeginValue;
    strCondition += " AND ";
    strCondition += strEndValue;
    strCondition += ")";
}

//////////////////////////////////////////////////////////////////////
// Construct SQL string for a string filter
//
// syntax for the filter		: <string1>[#GEXDB#OR#<string2>...]
//
// SQL syntax					: "(<DbField> = <string1> [OR <DbField> = <string2> ...])"
//
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::Query_BuildSqlString_StringFilter(
        const QString & strDbField,
        const QString & strFilter,
        QString & strCondition,
        bool bNegation/*=false*/)
{
    strCondition = "";

    // Check for empty field?
    if(strFilter.isEmpty())
    {
        strCondition += strDbField;
        if(bNegation)
            strCondition += " != ''";
        else
            strCondition += " = ''";
        return;
    }

    // Go through accepted values
    QStringList	strlElements = strFilter.split(GEXDB_PLUGIN_DELIMITER_OR);
    QStringList::iterator itElements;
    QString		strValue;
    for(itElements=strlElements.begin(); itElements!=strlElements.end(); itElements++)
    {
        strValue = *itElements;
        if(itElements != strlElements.begin())
        {
            if(bNegation)
                strCondition += " AND ";
            else
                strCondition += " OR ";
        }
        if(strValue.contains("*") || strValue.contains("?"))
        {
            strCondition += strDbField;
            if(bNegation)
                strCondition += " NOT LIKE '";
            else
                strCondition += " LIKE '";
            strCondition += GexDbPlugin_Base::Query_BuildSqlStringExpression(strValue, true);
            strCondition += "'";
        }
        else
        {
            strCondition += strDbField;
            if(bNegation)
                strCondition += " != '";
            else
                strCondition += " = '";
            strCondition += GexDbPlugin_Base::Query_BuildSqlStringExpression(strValue, false);;
            strCondition += "'";
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Construct SQL string for a string filter, and a field expression
//
// syntax for the filter		: <string1>[#GEXDB#OR#<string2>...]
//
// SQL syntax					: "(<DbField expression> = <string1> [OR <DbField> = <string2> ...])"
//
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Base::Query_BuildSqlString_FieldExpression_Numeric(
        const QString & strFieldExpression,
        const QString & strFilter,
        QString & strCondition)
{
    strCondition = "";

    // Check for empty field?
    if(strFilter.isEmpty())
    {
        strCondition += strFieldExpression;
        strCondition += " = ''";
        return;
    }

    // Go through accepted values
    QStringList	strlElements = strFilter.split(GEXDB_PLUGIN_DELIMITER_OR);
    QStringList::iterator itElements;
    QString		strValue;
    for(itElements=strlElements.begin(); itElements!=strlElements.end(); itElements++)
    {
        strValue = *itElements;
        if(itElements != strlElements.begin())
            strCondition += " OR ";
        strCondition += strFieldExpression;
        strCondition += " = ";
        strCondition += strValue;
    }
}

///////////////////////////////////////////////////////////
// Query: build SQL string from query lists
//
// Example:
//
// wt_lot.product_name
// MySQL/Oracle:	field = wt_lot.product_name
//					table = wt_lot
//
// GEXDB.wt_lot.product_name
// MySQL/Oracle:	field = GEXDB.wt_lot.product_name
//					table = GEXDB.wt_lot
//
// wt_lot.product_name=MyProduct
// MySQL/Oracle:	field = wt_lot.product_name AS MyProduct
//					table = wt_lot
//
// GEXDB.wt_lot.product_name=MyProduct
// MySQL/Oracle:	field = GEXDB.wt_lot.product_name AS MyProduct
//					table = GEXDB.wt_lot
//
// !!!!!!!!!! LISTS USE FOLLOWING SYNTAX: !!!!!!!!!!
//
// m_strlQuery_Fields					: list of fields to query
// syntax for each element				: <Type>[=<Alias>]|<Field>[|<Value>]	o The <Alias> is an alphanumerical alias to be used for the column
//																				o The <Value> field depends on the type of field specification
// syntax for Type						: 'Field' | 'Function' | 'DistinctFunction' | 'Expression' | 'ConsolidatedField'
// syntax for Field	or ConsolidatedField: <table>.<field> OR <schema>.<table>.<field>
// If <Type> = 'Function' or 'DistinctFunction':
// syntax for Value						: any valid SQL function ('COUNT' | 'MIN' | 'MAX' ...)
// If <Type> = 'Expression':
// syntax for Value						: any valid expression that will be used as is in the SELECT statement
//
// m_strlQuery_OrderFields				: list of fields to be used to order results
// syntax for each element				: <Field>|<Option>
// syntax for Field						: <table>.<field> OR <schema>.<table>.<field>
// syntax for Option					: 'ASC', 'DESC'
// example								: m_strlQuery_OrderFields.append(m_strTablePrefix + SPLITLOT_TN + ".retest_index|ASC");
//
// m_strlQuery_GroupFields				: list of fields to be used to group results
// syntax for each element				: <Field>
// syntax for Field						: <table>.<field> OR <schema>.<table>.<field>
//
// m_strlQuery_LinkConditions			: list of link conditions (between tables)
// syntax for each element				: <Field_1>|<Field_2>
// syntax for Field_N					: <table_N>.<field_N> OR <schema>.<table_N>.<field_N>
//
// m_strlQuery_ValueConditions			: list of value conditions (filters)
// syntax for each element				: <Field>[=<SQL expression>]|<ValueType>|<Value>[|<FieldExpression>]
// syntax for Field						: <table>.<field> OR <schema>.<table>.<field>
// syntax for SQL expression            : SQL expression could be any cross-SQL engine compatible (MySQL, Oracle,...) SQL expressions
//                                          for example : wt_run.run_id=mod(run_id, 10)|Numeric|0
//                                          Warning : no modification will be applied on this expression !
//                                          Warning : this feature is for the moment only available with Numeric tests
// syntax for ValueType					: "Numeric" OR "String" OR "NotString" OR "Expression" OR "ExpressionRange"
//										  "FieldExpression_String" OR "FieldExpression_Numeric"
// syntax for Value	(String)			: <value1> OR <value1>,<value2> OR <value1>,<value2>,<value3> OR ...
//                                        False : seems it works with GEXDB_PLUGIN_DELIMITER_OR only
// syntax for Value	(NotString)			: <value1> OR <value1>,<value2> OR <value1>,<value2>,<value3> OR ...
// syntax for Value	(Expression)		: <expression>
// syntax for Value	(Numeric)			: '>'<value> OR '<'<value> OR '>='<value> OR '<='<value> OR <value1> OR <value1>,<value2> OR <value1>-<value2> OR any combination
// syntax for Value	(ExpressionRange)	: <expression1>#GEXDB#TO#<expression2>
// exple :	m_strlQuery_ValueConditions.append( m_strTablePrefix + SPLITLOT_TN + ".lot_id|String|gal_lot");
//
// syntax for FieldExpression			: <expression>
//
// m_mapQuery_TableAliases				: map with table aliases to be used
//
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_BuildSqlString(
        QString & strSqlString,
        bool bDistinct/*=true*/,
        QString strOptimizeOnTable/*=""*/,
        GS::DbPluginBase::QueryEngine::BuildingRule buildingRule/*=UseFunctionalDep*/)
{
    // Init SQL srtring
    strSqlString = "";

    // Check query parameters
    if(m_strlQuery_Fields.empty())
        return false;

    // Construct query from different lists
    // SELECT <field1>,<field2>...
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    QStringList				strlQuery_Tables, strlFilterValues , strlElements, strlType, strlFields, strlAlias;
    QString					strDbField0, strDbField1, strDbTable0, strDbTable1, strDbFunction, strField, strType, strValue;
    QString					strSelect, strFrom, strWhere, strOrder, strGroup, strValueType, strFieldExpression, strCondition, strOption;
    QStringList::Iterator	itField, itTable, itCondition, itFilterValue;
    int						iItem=0;

    // SELECT
    if(bDistinct)
        strSelect = "SELECT DISTINCT";
    else
        strSelect = "SELECT";
    if(m_pclDatabaseConnector->IsOracleDB() && !strOptimizeOnTable.isEmpty())
    {
        strSelect += " /*+ INDEX(";
        strSelect += strOptimizeOnTable + " ";
        strSelect += strOptimizeOnTable + ") INDEX_PARALLEL(";
        strSelect += strOptimizeOnTable + " 16) FIRST_ROWS(1000) */";
    }
    strSelect += "\n";
    for(iItem = 0; iItem < m_strlQuery_Fields.count(); iItem++)
    {
        strlElements = m_strlQuery_Fields[iItem].split("|" );
        if(strlElements.count() < 2)
            return false;
        strlType = strlElements[0].trimmed().split("=");
        strType = strlType[0].trimmed().toLower();
        strField = strlElements[1].trimmed().toLower();
        if(strType == "field")
        {
            strlFields = strField.split("+");
            if(strlFields.size() == 1)
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                // Add field
                if(iItem > 0)
                    strSelect += ",\n";
                // On Oracle, if we have a 'Group by' and the field is not part of the group by, use max
                if(m_pclDatabaseConnector->IsOracleDB() && !m_strlQuery_GroupFields.isEmpty() && !m_strlQuery_GroupFields.contains(strField))
                    strSelect += "max(" + strDbField0 + ")";
                else
                    strSelect += strDbField0;
            }
            else
            {
                strField = "";
                for(itField = strlFields.begin(); itField != strlFields.end(); itField++)
                {
                    Query_NormalizeToken(*itField, strDbField0, strDbTable0);
                    // Add to list of tables
                    if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                        strlQuery_Tables.append(strDbTable0);
                    if(m_pclDatabaseConnector->IsMySqlDB())
                    {
                        if(strField.isEmpty())
                            strField = "concat(" + strDbField0;
                        else
                            strField += ",';'," + strDbField0;
                    }
                    else
                    {
                        if(strField.isEmpty())
                            strField = strDbField0;
                        else
                            strField += "||';'|| " + strDbField0;
                    }
                }
                if(m_pclDatabaseConnector->IsMySqlDB())
                    strField += ")";

                // Add field
                if(iItem > 0)
                    strSelect += ",\n";
                // On Oracle, if we have a 'Group by', use max
                if(m_pclDatabaseConnector->IsOracleDB() && !m_strlQuery_GroupFields.isEmpty())
                    strSelect += "max(" + strField + ")";
                else
                    strSelect += strField;
            }
        }
        else if(strType.endsWith("consolidatedfield"))
        {
            // to manage Custom Metadata with special cast to string
            // if CustomConsolidatedField then cast to VARCHAR
            bool bCastToString = strType.startsWith("custom");

            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);
            // Add field
            if(iItem > 0)
                strSelect += ",\n";
            if(m_pclDatabaseConnector->IsOracleDB())
            {
                strSelect += "case WHEN count(distinct " + strDbField0 + ") <=1 THEN case WHEN MAX(" + strDbField0;
                strSelect += ") is null OR MAX(" + strDbField0 + ")='' THEN 'n/a' ELSE MAX(";
                if(bCastToString)
                    strSelect += "CAST("+strDbField0+" as VARCHAR2(255))";
                else
                    strSelect += strDbField0;
                strSelect += ") END ELSE 'MULTI' END";
            }
            else
            {
                // gcore-1855 (MySQL only)
                strSelect += "case WHEN (count(distinct " + strDbField0 + ") + (" + strDbField0 + " is null)) <=1 THEN case WHEN (" + strDbField0;
                strSelect += ") is null OR (" + strDbField0 + ")='' THEN 'n/a' ELSE (";
                if(bCastToString)
                    strSelect += "CAST("+strDbField0+" as CHAR(255))";
                else
                    strSelect += strDbField0;
                strSelect += ") END ELSE 'MULTI' END";
            }
        }
        else if((strType == "function") || (strType == "distinctfunction"))
        {
            if(strlElements.count() < 3)
                return false;
            strlFields = strField.split(",");
            strField = "";
            for(itField = strlFields.begin(); itField != strlFields.end(); itField++)
            {
                Query_NormalizeToken(*itField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                if(!strField.isEmpty())
                    strField += ",";
                strField += strDbField0;
            }
            // Add field
            if(iItem > 0)
                strSelect += ",\n";
            if(strType == "function")
                strSelect += strlElements[2] + "(";
            else
                strSelect += strlElements[2] + "(distinct ";
            // Under Oracle, count(f1, f2) is not possible, so use concat
            if(m_pclDatabaseConnector->IsOracleDB() && (strlFields.size() > 1))
                strSelect += "concat(" + strField + ")";
            else
                strSelect += strField;
            strSelect += ")";
        }
        else if(strType == "expression")
        {
            if(strlElements.count() < 3)
                return false;
            Query_NormalizeToken(strField, strDbField0, strDbTable0);
            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);
            // Add expression
            if(iItem > 0)
                strSelect += ",\n";
            strSelect += strlElements[2];
        }
        else
            return false;

        // Add an alias??
        if(strlType.count() > 1)
        {
            strSelect += " AS ";
            strSelect += strlType[1].trimmed();
            // if duplicated alias
            if(strlAlias.contains(strlType[1].trimmed()))
                strSelect += "_"+QString::number(strlAlias.count(strlType[1].trimmed()));
            strlAlias.append(strlType[1].trimmed());
        }
    }
    strSelect += "\n";

    // WHERE: link conditions
    strWhere = "";
    for(iItem = 0; iItem < m_strlQuery_LinkConditions.count(); iItem++)
    {
        if(strWhere.isEmpty())
            strWhere = "WHERE\n(";
        else
            strWhere += "AND (";

        strlElements = m_strlQuery_LinkConditions[iItem].split("|");
        Query_NormalizeToken(strlElements[0], strDbField0, strDbTable0);
        Query_NormalizeToken(strlElements[1], strDbField1, strDbTable1);
        // Add to list of tables
        if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
            strlQuery_Tables.append(strDbTable0);
        if((!strDbTable1.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable1) == -1))
            strlQuery_Tables.append(strDbTable1);
        // Add condition
        strWhere += strDbField0;
        strWhere += "=";
        strWhere += strDbField1;
        strWhere += ")\n";
    }


    if (mQueryEngine)
    {
        mQueryEngine->ResetQuery();
        mQueryEngine->SetFormatedQueryFields(m_strlQuery_Fields);
        mQueryEngine->SetFormatedFilters(m_strlQuery_ValueConditions);
        mQueryEngine->SetBuildingRule(buildingRule);
        if (mQueryEngine->BuildQuery() && !mQueryEngine->WhereClause().isEmpty())
        {
            strWhere = "WHERE \n" + mQueryEngine->WhereClause();
            if (!mQueryEngine->SimplifiedValueConditions().isEmpty())
                m_strlQuery_ValueConditions = mQueryEngine->SimplifiedValueConditions();
        }
    }

    // WHERE: value conditions
    for(iItem = 0; iItem < m_strlQuery_ValueConditions.count(); iItem++)
    {
        if(strWhere.isEmpty())
            strWhere = "WHERE\n(";
        else
            strWhere += "AND (";

        // Split elements
        strlElements = m_strlQuery_ValueConditions[iItem].split("|");
        strField = strlElements[0];

        QString strSQLExpression=strField.section("=",1,1);

        // Check ValueType
        if (strlElements.size()>1)
            strValueType = strlElements[1];
        else
            strValueType = "";
        strValue = strFieldExpression = "";
        if(strlElements.count() > 2)
            strValue = strlElements[2];
        if(strlElements.count() > 3)
            strFieldExpression = strlElements[3];

        if (!strSQLExpression.isEmpty())
            if(strValueType.toLower() != "numeric")
            {
                GSLOG(SYSLOG_SEV_ERROR, "SQL expression syntax in WHERE query is ONLY supported with Numeric tests !");
                return false;
            }

        if(strValueType.toLower() == "numeric")
        {
            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);

            // Build SQL string for this filter
            if (!strSQLExpression.isEmpty())
                Query_BuildSqlString_NumericFilter(strSQLExpression, strValue, strCondition);
            else
                Query_BuildSqlString_NumericFilter(strDbField0, strValue, strCondition);

            strWhere += strCondition;
        }
        else if(strValueType.toLower() == "expressionrange")
        {
            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);

            // Build SQL string for this filter
            Query_BuildSqlString_ExpressionRange(strDbField0, strValue, strCondition);
            strWhere += strCondition;
        }
        else if(strValueType.toLower() == "string")
        {
            strlFields = strField.split("+");
            if(strlFields.size() == 1)
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);

                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);

                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strDbField0, strValue, strCondition);
                strWhere += strCondition;
            }
            else
            {
                strField = "";
                for(itField = strlFields.begin(); itField != strlFields.end(); itField++)
                {
                    Query_NormalizeToken(*itField, strDbField0, strDbTable0);

                    // Add to list of tables
                    if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                        strlQuery_Tables.append(strDbTable0);

                    if(m_pclDatabaseConnector->IsMySqlDB())
                    {
                        if(strField.isEmpty())
                            strField = "concat(" + strDbField0;
                        else
                            strField += ",';'," + strDbField0;
                    }
                    else
                    {
                        if(strField.isEmpty())
                            strField = strDbField0;
                        else
                            strField += "||';'|| " + strDbField0;
                    }
                }
                if(m_pclDatabaseConnector->IsMySqlDB())
                    strField += ")";

                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strField, strValue, strCondition);
                strWhere += strCondition;
            }
        }
        else if(strValueType.toLower() == "notstring")
        {
            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);

            // Build SQL string for this filter
            Query_BuildSqlString_StringFilter(strDbField0, strValue, strCondition, true);
            strWhere += strCondition;
        }
        else if(strValueType.toLower() == "expression")
        {
            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);

            // Add expression
            strWhere += strDbField0;
            strWhere += " = ";
            strWhere += strValue;
        }
        else if((strValueType.toLower() == "fieldexpression_string") && !strFieldExpression.isEmpty())
        {
            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);

            // Build SQL string for this filter
            Query_BuildSqlString_StringFilter(strFieldExpression, strValue, strCondition);
            strWhere += strCondition;
        }
        else if((strValueType.toLower() == "fieldexpression_numeric") && !strFieldExpression.isEmpty())
        {
            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);

            // Build SQL string for this filter
            Query_BuildSqlString_FieldExpression_Numeric(strFieldExpression, strValue, strCondition);
            strWhere += strCondition;
        }
        else
            return false;

        strWhere += ")\n";
    }

    // FROM
    itTable = strlQuery_Tables.begin();
    strFrom += "FROM\n";
    if (mQueryEngine && mQueryEngine->ValidQuery())
        strFrom += mQueryEngine->FromClause();
    else
    {
        for(iItem = 0; iItem < strlQuery_Tables.count(); iItem++)
        {
            strDbTable0 = strlQuery_Tables[iItem];
            if(iItem > 0)
                strFrom += ",\n";
            strFrom += strDbTable0;
            if(m_mapQuery_TableAliases.find(strDbTable0) != m_mapQuery_TableAliases.end())
            {
                strFrom += " ";
                strFrom += m_mapQuery_TableAliases[strDbTable0];
            }

        }
    }
    strFrom += "\n";

    // ORDER BY
    if(!m_strlQuery_OrderFields.empty())
    {
        strOrder = "ORDER BY ";
        for(iItem = 0; iItem < m_strlQuery_OrderFields.count(); iItem++)
        {
            strlElements = m_strlQuery_OrderFields[iItem].split("|");
            strlFields = strlElements[0].split("=");
            if(strlFields.count() > 1)
                strDbField0 = strlFields[1];
            else
                Query_NormalizeToken(strlElements[0], strDbField0, strDbTable0);
            // Add field
            if(iItem > 0)
                strOrder += ",";
            if(strlElements.count() > 1)
            {
                strOrder += strDbField0 + " ";
                strOrder += strlElements[1];
            }
            else
                strOrder += strDbField0;
        }
        strOrder += "\n";
    }

    // GROUP BY
    if(!m_strlQuery_GroupFields.empty())
    {
        strGroup = "GROUP BY ";
        for(iItem = 0; iItem < m_strlQuery_GroupFields.count(); iItem++)
        {
            Query_NormalizeToken(m_strlQuery_GroupFields[iItem], strDbField0, strDbTable0);
            // Add field
            if(iItem > 0)
                strGroup += ",";
            strGroup += strDbField0;
        }
        strGroup += "\n";
    }

    strSqlString = strSelect + strFrom + strWhere + strGroup + strOrder;

    return true;
}

///////////////////////////////////////////////////////////
// Query: add jointure
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_BuildSqlString_AddJoinConditions(QString & strJoins, QString & strTable, QStringList & strlValueConditions)
{
    QString						strDbField0, strDbTable0, strValueType, strFieldExpression, strOption, strCondition;
    QStringList					strlElements;
    QList<QString>::Iterator	it;

    it = strlValueConditions.begin();
    while(it != strlValueConditions.end())
    {
        strlElements = (*it).split("|");
        Query_NormalizeToken(strlElements[0], strDbField0, strDbTable0);

        // Check if the condition is on specified table
        if(strDbTable0 == strTable)
        {
            strJoins += " AND (";

            // Check ValueType
            strValueType = strlElements[1];
            strFieldExpression = "";
            if(strlElements.size() > 3)
                strFieldExpression = strlElements[3];
            if(strValueType.toLower() == "numeric")
            {
                // Build SQL string for this filter
                Query_BuildSqlString_NumericFilter(strDbField0, strlElements[2], strCondition);
                strJoins += strCondition;
            }
            else if(strValueType.toLower() == "expressionrange")
            {
                // Build SQL string for this filter
                Query_BuildSqlString_ExpressionRange(strDbField0, strlElements[2], strCondition);
                strJoins += strCondition;
            }
            else if(strValueType.toLower() == "string")
            {
                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strDbField0, strlElements[2], strCondition);
                strJoins += strCondition;
            }
            else if(strValueType.toLower() == "notstring")
            {
                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strDbField0, strlElements[2], strCondition, true);
                strJoins += strCondition;
            }
            else if(strValueType.toLower() == "expression")
            {
                // Add to list of tables
                // Add expression
                strJoins += strDbField0;
                strJoins += " = ";
                strJoins += strlElements[2];
            }
            else if((strValueType.toLower() == "fieldexpression_string") && !strFieldExpression.isEmpty())
            {
                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strFieldExpression, strlElements[2], strCondition);
                strJoins += strCondition;
            }
            else if((strValueType.toLower() == "fieldexpression_numeric") && !strFieldExpression.isEmpty())
            {
                // Build SQL string for this filter
                Query_BuildSqlString_FieldExpression_Numeric(strFieldExpression, strlElements[2], strCondition);
                strJoins += strCondition;
            }
            else
                return false;

            strJoins += ")";

            it = strlValueConditions.erase(it);
        }
        else
            it++;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: add jointure
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_BuildSqlString_AddJoin(QString & strJoins, QStringList & strlLinkConditions, QStringList & strlValueConditions, bool bIncludeFiltersInJoinCondition/*=true*/, GexDbPlugin_Base::QueryJoinType eQueryJoinType/*=GexDbPlugin_Base::eQueryJoin_Inner*/)
{
    QString						strTerminalTable, strDbField0, strDbTable0, strDbField1, strDbTable1;
    QString						strDbField0_2, strDbTable0_2, strDbField1_2, strDbTable1_2;
    QStringList					strlElements, strlElements_2;
    bool						bFirstJointure = strJoins.isEmpty();
    QList<QString>::iterator	it;

    // Find terminal element
    for(it=strlLinkConditions.begin(); it!=strlLinkConditions.end(); it++)
    {
        strlElements = (*it).split("|");
        Query_NormalizeToken(strlElements[0], strDbField0, strDbTable0);
        Query_NormalizeToken(strlElements[1], strDbField1, strDbTable1);
        if(strTerminalTable.isEmpty() || (strDbTable0 == strTerminalTable))
            strTerminalTable = strDbTable1;
    }
    if(strTerminalTable.isEmpty())
        return false;

    // If first jointure, add both tables
    if(bFirstJointure)
        strJoins = strTerminalTable + "\n";

    // Add all tables linked to this terminal and conditions
    it = strlLinkConditions.begin();
    while(it != strlLinkConditions.end())
    {
        strlElements = (*it).split("|");
        Query_NormalizeToken(strlElements[0], strDbField0, strDbTable0);
        Query_NormalizeToken(strlElements[1], strDbField1, strDbTable1);
        if(strDbTable1 == strTerminalTable)
        {
            // Add Jointure
            if(eQueryJoinType == GexDbPlugin_Base::eQueryJoin_LeftOuter)
                strJoins += "LEFT OUTER JOIN " + strDbTable0 + "\nON ";
            else
                strJoins += "INNER JOIN " + strDbTable0 + "\nON ";
            strJoins += strDbField0;
            strJoins += "=";
            strJoins += strDbField1;
            // Check if other links on same table
            it = strlLinkConditions.erase(it);
            while(it != strlLinkConditions.end())
            {
                strlElements_2 = (*it).split("|");
                Query_NormalizeToken(strlElements_2[0], strDbField0_2, strDbTable0_2);
                Query_NormalizeToken(strlElements_2[1], strDbField1_2, strDbTable1_2);
                if(strDbTable0_2 == strDbTable0)
                {
                    strJoins += " AND " + strDbField0_2;
                    strJoins += "=";
                    strJoins += strDbField1_2;
                    it = strlLinkConditions.erase(it);
                }
                else
                    it++;
            }

            // The filter conditions can be added in the ON clause, or in the WHERE clause
            if(bIncludeFiltersInJoinCondition)
            {
                // If first jointure, add conditions on initial table
                if(bFirstJointure)
                    Query_BuildSqlString_AddJoinConditions(strJoins, strTerminalTable, strlValueConditions);
                // Add conditions on joined table
                Query_BuildSqlString_AddJoinConditions(strJoins, strDbTable0, strlValueConditions);
            }
            strJoins += "\n";
        }
        else
            it++;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: build SQL string from query lists
//
// Example:
//
// wt_lot.product_name
// MySQL/Oracle:	field = wt_lot.product_name
//					table = wt_lot
//
// GEXDB.wt_lot.product_name
// MySQL/Oracle:	field = GEXDB.wt_lot.product_name
//					table = GEXDB.wt_lot
//
// wt_lot.product_name=MyProduct
// MySQL/Oracle:	field = wt_lot.product_name AS MyProduct
//					table = wt_lot
//
// GEXDB.wt_lot.product_name=MyProduct
// MySQL/Oracle:	field = GEXDB.wt_lot.product_name AS MyProduct
//					table = GEXDB.wt_lot
//
// !!!!!!!!!! LISTS USE FOLLOWING SYNTAX: !!!!!!!!!!
//
// m_strlQuery_Fields					: list of fields to query
// syntax for each element				: <Type>[=<Alias>]|<Field>[|<Value>]	o The <Alias> is an alphanumerical alias to be used for the column
//																				o The <Value> field depends on the type of field specification
// syntax for Type						: 'Field' | 'Function' | 'DistinctFunction' | 'Expression' | 'ConsolidatedField'
// syntax for Field	or ConsolidatedField: <table>.<field> OR <schema>.<table>.<field>
// If <Type> = 'Function' or 'DistinctFunction':
// syntax for Value						: any valid SQL function ('COUNT' | 'MIN' | 'MAX' ...)
// If <Type> = 'Expression':
// syntax for Value						: any valid expression that will be used as is in the SELECT statement
//
// m_strlQuery_OrderFields				: list of fields to be used to order results
// syntax for each element				: <Field>|<Option>
// syntax for Field						: <table>.<field> OR <schema>.<table>.<field>
// syntax for Option					: 'ASC', 'DESC'
//
// m_strlQuery_GroupFields				: list of fields to be used to group results
// syntax for each element				: <Field>
// syntax for Field						: <table>.<field> OR <schema>.<table>.<field>
//
// m_strlQuery_LinkConditions			: list of link conditions (between tables)
// syntax for each element				: <Field_1>|<Field_2>
// syntax for Field_N					: <table_N>.<field_N> OR <schema>.<table_N>.<field_N>
//
// m_strlQuery_ValueConditions			: list of value conditions (filters)
// syntax for each element				: <Field>|<ValueType>|<Value>[|<FieldExpression>]
// syntax for Field						: <table>.<field> OR <schema>.<table>.<field>
// syntax for ValueType					: "Numeric" OR "String" OR "NotString" OR "Expression" OR "ExpressionRange"
//										  "FieldExpression_String" OR "FieldExpression_Numeric"
// syntax for Value	(String)			: <value1> OR <value1>,<value2> OR <value1>,<value2>,<value3> OR ...
// syntax for Value	(NotString)			: <value1> OR <value1>,<value2> OR <value1>,<value2>,<value3> OR ...
// syntax for Value	(Expression)		: <expression>
// syntax for Value	(Numeric)			: '>'<value> OR '<'<value> OR '>='<value> OR '<='<value> OR <value1> OR <value1>,<value2> OR <value1>-<value2> OR any combination
// syntax for Value	(ExpressionRange)	: <expression1>#GEXDB#TO#<expression2>
//
// syntax for FieldExpression			: <expression>
//
// m_mapQuery_TableAliases				: map with table aliases to be used
//
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_BuildSqlString_UsingJoins(
        QString & strSqlString, bool bDistinct/*=true*/, QString strOptimizeOnTable/*=""*/,
        bool bIncludeFiltersInJoinCondition/*=false*/,
        GexDbPlugin_Base::QueryJoinType eQueryJoinType/*=GexDbPlugin_Base::eQueryJoin_Inner*/)
{
    // Init SQL srtring
    strSqlString = "";
    QString strSelect, strFrom, strWhere, strGroup, strOrder;
    bool r=Query_BuildSqlString_UsingJoins(strSelect, strFrom, strWhere, strGroup, strOrder, bDistinct, strOptimizeOnTable, bIncludeFiltersInJoinCondition, eQueryJoinType);
    strSqlString = strSelect + strFrom + strWhere + strGroup + strOrder;
    return r;
}

bool GexDbPlugin_Base::Query_BuildSqlString_UsingJoins(
        QString & strSelect, QString & strFrom, QString & strWhere, QString & strGroup,
        QString & strOrder, bool bDistinct/*=true*/, QString strOptimizeOnTable/*=""*/,
        bool bIncludeFiltersInJoinCondition/*=false*/,
        GexDbPlugin_Base::QueryJoinType eQueryJoinType/*=GexDbPlugin_Base::eQueryJoin_Inner*/)
{
    // Init SQL srtrings
    strSelect = strFrom = strWhere = strGroup = strOrder = "";

    // Check query parameters
    if(m_strlQuery_Fields.empty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "No parameter to query");
        return false;
    }

    // Construct query from different lists
    // SELECT <field1>,<field2>...
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    QStringList				strlQuery_Tables, strlQuery_JoinedTables, strlFilterValues , strlElements, strlType, strlFields, strlLinks, strlAlias;
    QString					strDbField0, strDbField1, strDbTable0, strDbTable1, strDbFunction, strField, strType, strValue;
    QString					strValueType, strFieldExpression, strCondition, strOption;
    QStringList::Iterator	itField, itTable, itCondition, itFilterValue;
    int					iItem;

    // SELECT
    if(bDistinct)
        strSelect = "SELECT DISTINCT";
    else
        strSelect = "SELECT";
    if(m_pclDatabaseConnector->IsOracleDB() && !strOptimizeOnTable.isEmpty())
    {
        strSelect += " /*+ INDEX(";
        strSelect += strOptimizeOnTable + " ";
        strSelect += strOptimizeOnTable + ") INDEX_PARALLEL(";
        strSelect += strOptimizeOnTable + " 16) FIRST_ROWS(1000) */";
    }
    strSelect += "\n";
    for(iItem = 0; iItem < m_strlQuery_Fields.size(); iItem++)
    {
        strlElements = m_strlQuery_Fields[iItem].split("|");
        if(strlElements.size() < 2)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid field format: %1").
                   arg(m_strlQuery_Fields[iItem]).toLatin1().data() );
            return false;
        }
        strlType = strlElements[0].trimmed().split("=");
        strType = strlType[0].trimmed().toLower();
        strField = strlElements[1].trimmed().toLower();
        if(strType == "field")
        {
            strlFields = strField.split("+");
            if(strlFields.size() == 1)
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                // Add field
                if(iItem > 0)
                    strSelect += ",\n";
                // On Oracle, if we have a 'Group by' and the field is not part of the group by, use max
                if(m_pclDatabaseConnector->IsOracleDB() && !m_strlQuery_GroupFields.isEmpty() && !m_strlQuery_GroupFields.contains(strField))
                    strSelect += "max(" + strDbField0 + ")";
                else
                    strSelect += strDbField0;
            }
            else
            {
                strField = "";
                for(itField = strlFields.begin(); itField != strlFields.end(); itField++)
                {
                    Query_NormalizeToken(*itField, strDbField0, strDbTable0);
                    // Add to list of tables
                    if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                        strlQuery_Tables.append(strDbTable0);
                    if(m_pclDatabaseConnector->IsMySqlDB())
                    {
                        if(strField.isEmpty())
                            strField = "concat(" + strDbField0;
                        else
                            strField += ",';'," + strDbField0;
                    }
                    else
                    {
                        if(strField.isEmpty())
                            strField = strDbField0;
                        else
                            strField += "||';'|| " + strDbField0;
                    }
                }
                if(m_pclDatabaseConnector->IsMySqlDB())
                    strField += ")";

                // Add field
                if(iItem > 0)
                    strSelect += ",\n";
                // On Oracle, if we have a 'Group by', use max
                if(m_pclDatabaseConnector->IsOracleDB() && !m_strlQuery_GroupFields.isEmpty())
                    strSelect += "max(" + strField + ")";
                else
                    strSelect += strField;
            }
        }
        else if(strType.endsWith("consolidatedfield"))
        {
            // to manage Custom Metadata with special cast to string
            // if CustomConsolidatedField then cast to VARCHAR
            bool bCastToString = strType.startsWith("custom");

            Query_NormalizeToken(strField, strDbField0, strDbTable0);

            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);
            // Add field
            if(iItem > 0)
                strSelect += ",\n";
            if(m_pclDatabaseConnector->IsOracleDB())
            {
                strSelect += "case WHEN count(distinct " + strDbField0 + ") <=1 THEN case WHEN MAX(" + strDbField0;
                strSelect += ") is null OR MAX(" + strDbField0 + ")='' THEN 'n/a' ELSE MAX(";
                if(bCastToString)
                    strSelect += "CAST("+strDbField0+" as VARCHAR2(255))";
                else
                    strSelect += strDbField0;
                strSelect += ") END ELSE 'MULTI' END";
            }
            else
            {
                // gcore-1855 (MySQL only)
                strSelect += "case WHEN (count(distinct " + strDbField0 + ") + (" + strDbField0 + " is null)) <=1 THEN case WHEN (" + strDbField0;
                strSelect += ") is null OR (" + strDbField0 + ")='' THEN 'n/a' ELSE (";
                if(bCastToString)
                    strSelect += "CAST("+strDbField0+" as CHAR(255))";
                else
                    strSelect += strDbField0;
                strSelect += ") END ELSE 'MULTI' END";
            }
        }
        else if((strType == "function") || (strType == "distinctfunction"))
        {
            if(strlElements.size() < 3)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid function format: %1").
                       arg(strlElements.join("|")).toLatin1().data() );
                return false;
            }
            strlFields = strField.split(",");
            strField = "";
            for(itField = strlFields.begin(); itField != strlFields.end(); itField++)
            {
                Query_NormalizeToken(*itField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                if(!strField.isEmpty())
                    strField += ",";
                strField += strDbField0;
            }
            // Add field
            if(iItem > 0)
                strSelect += ",\n";
            if(strType == "function")
                strSelect += strlElements[2] + "(";
            else
                strSelect += strlElements[2] + "(distinct ";
            // Under Oracle, count(f1, f2) is not possible, so use concat
            if(m_pclDatabaseConnector->IsOracleDB() && (strlFields.size() > 1))
                strSelect += "concat(" + strField + ")";
            else
                strSelect += strField;
            strSelect += ")";
        }
        else if(strType == "expression")
        {
            if(strlElements.size() < 3)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid expression format: %1").
                       arg(strlElements.join("|")).toLatin1().data() );
                return false;
            }
            Query_NormalizeToken(strField, strDbField0, strDbTable0);
            // Add to list of tables
            if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                strlQuery_Tables.append(strDbTable0);
            // Add expression
            if(iItem > 0)
                strSelect += ",\n";
            strSelect += strlElements[2];
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid field type: %1").
                   arg(strType).toLatin1().data() );
            return false;
        }

        // Add an alias??
        if(strlType.size() > 1)
        {
            strSelect += " AS ";
            strSelect += strlType[1].trimmed();
            // if duplicated alias
            if(strlAlias.contains(strlType[1].trimmed()))
                strSelect += "_"+QString::number(strlAlias.count(strlType[1].trimmed()));
            strlAlias.append(strlType[1].trimmed());
        }
    }
    strSelect += "\n";

    if (mQueryEngine)
    {
        mQueryEngine->ResetQuery();
        GS::DbPluginBase::QueryEngine::JointureType lJointure;
        if (eQueryJoinType ==GexDbPlugin_Base::eQueryJoin_Inner)
            lJointure = GS::DbPluginBase::QueryEngine::INNER;
        else
            lJointure = GS::DbPluginBase::QueryEngine::LEFTOUTER;
        mQueryEngine->SetFormatedQueryFields(m_strlQuery_Fields);
        mQueryEngine->SetFormatedFilters(m_strlQuery_ValueConditions);
        if (mQueryEngine->BuildQuery() && !mQueryEngine->JoinClause(lJointure).isEmpty())
        {
            strFrom = "FROM \n" + mQueryEngine->JoinClause(lJointure);
            if (!mQueryEngine->SimplifiedValueConditions().isEmpty())
                m_strlQuery_ValueConditions = mQueryEngine->SimplifiedValueConditions();
        }
    }


    // TABLE JOINS if link conditions
    if(strFrom.isEmpty() && m_strlQuery_LinkConditions.size() > 0)
    {
        QString strJoins = "";
        QStringList strlLinkConditions = m_strlQuery_LinkConditions;
        QStringList strlValueConditions = m_strlQuery_ValueConditions;
        while(strlLinkConditions.size() > 0)
            Query_BuildSqlString_AddJoin(strJoins, strlLinkConditions, strlValueConditions, bIncludeFiltersInJoinCondition, eQueryJoinType);
        strFrom = "FROM\n" + strJoins;
    }

    if(!bIncludeFiltersInJoinCondition || strFrom.isEmpty())
    {
        // WHERE: value conditions
        for(iItem = 0; iItem <  m_strlQuery_ValueConditions.count(); iItem++)
        {
            if(strWhere.isEmpty())
                strWhere = "WHERE\n(";
            else
                strWhere += "AND (";

            // Split elements
            strlElements = m_strlQuery_ValueConditions[iItem].split("|");
            strField = strlElements[0];

            // Check ValueType
            strValueType = strlElements[1];
            strValue = strFieldExpression = "";
            if(strlElements.count() > 2)
                strValue = strlElements[2];
            if(strlElements.count() > 3)
                strFieldExpression = strlElements[3];

            if(strValueType.toLower() == "numeric")
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);

                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                // Build SQL string for this filter
                Query_BuildSqlString_NumericFilter(strDbField0, strValue, strCondition);
                if(strCondition.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Empty numeric filter: %1").
                           arg(strField).toLatin1().data() );
                    return false;
                }
                strWhere += strCondition;
            }
            else if(strValueType.toLower() == "expressionrange")
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                // Build SQL string for this filter
                Query_BuildSqlString_ExpressionRange(strDbField0, strValue, strCondition);
                if(strCondition.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Empty expressionrange filter: %1").
                           arg(strField).toLatin1().data() );
                    return false;
                }
                strWhere += strCondition;
            }
            else if(strValueType.toLower() == "string")
            {
                strlFields = strField.split("+");
                if(strlFields.size() == 1)
                {
                    Query_NormalizeToken(strField, strDbField0, strDbTable0);

                    // Add to list of tables
                    if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                        strlQuery_Tables.append(strDbTable0);

                    // Build SQL string for this filter
                    Query_BuildSqlString_StringFilter(strDbField0, strValue, strCondition);
                    if(strCondition.isEmpty())
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Empty string filter: %1").
                               arg(strField).toLatin1().data() );
                        return false;
                    }
                    strWhere += strCondition;
                }
                else
                {
                    strField = "";
                    for(itField = strlFields.begin(); itField != strlFields.end(); itField++)
                    {
                        Query_NormalizeToken(*itField, strDbField0, strDbTable0);

                        // Add to list of tables
                        if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                            strlQuery_Tables.append(strDbTable0);

                        if(m_pclDatabaseConnector->IsMySqlDB())
                        {
                            if(strField.isEmpty())
                                strField = "concat(" + strDbField0;
                            else
                                strField += ",';'," + strDbField0;
                        }
                        else
                        {
                            if(strField.isEmpty())
                                strField = strDbField0;
                            else
                                strField += "||';'|| " + strDbField0;
                        }
                    }
                    if(m_pclDatabaseConnector->IsMySqlDB())
                        strField += ")";

                    // Build SQL string for this filter
                    Query_BuildSqlString_StringFilter(strField, strValue, strCondition);
                    if(strCondition.isEmpty())
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Empty filter: %1").
                               arg(strField).toLatin1().data() );
                        return false;
                    }
                    strWhere += strCondition;
                }
            }
            else if(strValueType.toLower() == "notstring")
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strDbField0, strValue, strCondition, true);
                if(strCondition.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Empty notstring filter: %1").
                           arg(strField).toLatin1().data() );
                    return false;
                }
                strWhere += strCondition;
            }
            else if(strValueType.toLower() == "expression")
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);
                // Add expression
                strWhere += strDbField0;
                strWhere += " = ";
                strWhere += strValue;
            }
            else if((strValueType.toLower() == "fieldexpression_string") && !strFieldExpression.isEmpty())
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);

                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);

                // Build SQL string for this filter
                Query_BuildSqlString_StringFilter(strFieldExpression, strValue, strCondition);
                if(strCondition.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Empty fieldexpression_string filter: %1").
                           arg(strField).toLatin1().data() );
                    return false;
                }
                strWhere += strCondition;
            }
            else if((strValueType.toLower() == "fieldexpression_numeric") && !strFieldExpression.isEmpty())
            {
                Query_NormalizeToken(strField, strDbField0, strDbTable0);
                // Add to list of tables
                if((!strDbTable0.isEmpty()) && (strlQuery_Tables.indexOf(strDbTable0) == -1))
                    strlQuery_Tables.append(strDbTable0);

                // Build SQL string for this filter
                Query_BuildSqlString_FieldExpression_Numeric(strFieldExpression, strValue, strCondition);
                if(strCondition.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Empty fieldexpression_numeric filter: %1").
                           arg(strField).toLatin1().data() );
                    return false;
                }
                strWhere += strCondition;
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid value type: %1").
                       arg(strValueType).toLatin1().data() );
                return false;
            }

            strWhere += ")\n";
        }
    }

    // Compute From if not already done
    if(strFrom.isEmpty())
    {
        // FROM
        itTable = strlQuery_Tables.begin();
        strFrom += "FROM\n";
        for(iItem = 0; iItem < strlQuery_Tables.count(); iItem++)
        {
            strDbTable0 = strlQuery_Tables[iItem];
            if(iItem > 0)
                strFrom += ",\n";
            strFrom += strDbTable0;
            if(m_mapQuery_TableAliases.find(strDbTable0) != m_mapQuery_TableAliases.end())
            {
                strFrom += " ";
                strFrom += m_mapQuery_TableAliases[strDbTable0];
            }
        }
        strFrom += "\n";
    }

    // ORDER BY
    if(!m_strlQuery_OrderFields.empty())
    {
        strOrder = "ORDER BY ";
        for(iItem = 0; iItem < m_strlQuery_OrderFields.count(); iItem++)
        {
            strlElements = m_strlQuery_OrderFields[iItem].split("|");
            strlFields = strlElements[0].split("=");
            if(strlFields.size() > 1)
                strDbField0 = strlFields[1];
            else
                Query_NormalizeToken(strlElements[0], strDbField0, strDbTable0);
            // Add field
            if(iItem > 0)
                strOrder += ",";
            if(strlElements.size() > 1)
            {
                strOrder += strDbField0 + " ";
                strOrder += strlElements[1];
            }
            else
                strOrder += strDbField0;
        }
        strOrder += "\n";
    }

    // GROUP BY
    if(!m_strlQuery_GroupFields.empty())
    {
        strGroup = "GROUP BY ";
        for(iItem = 0; iItem < m_strlQuery_GroupFields.size(); iItem++)
        {
            Query_NormalizeToken(m_strlQuery_GroupFields[iItem], strDbField0, strDbTable0);
            // Add field
            if(iItem > 0)
                strGroup += ",";
            strGroup += strDbField0;
        }
        strGroup += "\n";
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: extract normalized field name and table name from
// the DB field specifier retrieved from the mapping
//
// Example:
//
// wt_lot.product_name
// MySQL/OCI:	field = wt_lot.product_name
//				table = wt_lot
//
// GEXDB.wt_lot.product_name
// MySQL/OCI:	field = GEXDB.wt_lot.product_name
//				table = GEXDB.wt_lot
//
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_NormalizeToken(const QString & strQueryToken,
                                            QString & strDbField,
                                            QString & strDbTable)
{
    // Reset input vars
    strDbField = "";
    strDbTable = "";

    // Make sure token is not empty
    if(strQueryToken.isEmpty())
        return true;

    // Extract table name and field name
    if(m_pclDatabaseConnector->m_bUseQuotesInSqlQueries)
    {
        QStringList	strlTokens = strQueryToken.split(".");
        QString		strLastToken = strlTokens.last();

        // Construct DB table string
        for(QStringList::Iterator it = strlTokens.begin(); it != strlTokens.end(); ++it)
        {
            if(*it != strLastToken)
            {
                if(strDbTable.isEmpty())
                    strDbTable = "\"" + (*it).toLower() + "\"";
                else
                    strDbTable += ".\"" + (*it).toLower() + "\"";
            }
        }

        // Construct DB field string
        if(strDbTable.isEmpty())
            strDbField = strQueryToken.toLower();
        else
            strDbField = strDbTable + "." + strLastToken.toLower();
    }
    else
    {
        // Construct DB field and table strings
        strDbField = strQueryToken.toLower();
        strDbTable = strQueryToken.section("|", 0, 0);
        strDbTable = strDbTable.left(strDbTable.lastIndexOf('.')).toLower();
    }

    // Add the schema specification, as we are logged in as a standard user,
    // unless the table already has a schema specification (ie external table referenced
    // through the metadata mapping feature)
    Query_NormalizeTableName(strDbTable);

    return true;
}

///////////////////////////////////////////////////////////
// Query: normalize table name
//
// Example:
// wt_lot => gexdb.wt_lot
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::Query_NormalizeTableName(QString & strDbTable)
{
    // Make sure table name is not empty
    if(strDbTable.isEmpty())
        return;

    // Add the schema specification, as we are logged in as a standard user,
    // unless the table already has a schema specification (ie external table referenced
    // through the metadata mapping feature)
    if(!m_pclDatabaseConnector->m_strSchemaName.isEmpty() && (strDbTable.indexOf(".") == -1))
    {
        strDbTable = "." + strDbTable;
        strDbTable = m_pclDatabaseConnector->m_strSchemaName + strDbTable;
    }
}

///////////////////////////////////////////////////////////
// Query: Normalize alias used in SQL "AS" syntax
// (replace spaces, commas...)
//
// Example:
//
// "Device Polarity,  10" => "Device_Polarity_10"
//
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::Query_NormalizeAlias(QString & strQueryAlias)
{
    // Make sure alias is not empty
    if(strQueryAlias.isEmpty())
        return;

    // Simplify whitespaces
    strQueryAlias = strQueryAlias.simplified();

    // Replace characters
    strQueryAlias.replace(' ', '_');
    strQueryAlias.replace(',', '_');
    strQueryAlias.replace(';', '_');
    strQueryAlias.replace('(', '_');
    strQueryAlias.replace(')', '_');
    strQueryAlias.replace('[', '_');
    strQueryAlias.replace(']', '_');
    strQueryAlias.replace('/', '_');

    // case 2721 pyc, 22/07/2011
    strQueryAlias.replace("-", "_");
    strQueryAlias.replace(".", "_");
    strQueryAlias.replace("#", "_");

    // check at the end !
    while(IsReservedWord(strQueryAlias))
    {
        strQueryAlias += QString("_Gex");
    }
}

///////////////////////////////////////////////////////////
// Query: add condition string for a specified Link
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_AddLinkCondition(const QString & strLinkName)
{
    if(m_pmapLinks_Remote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Recursively construct condition string
    // Syntax is: <table1>.<field1>|<table2>.<field2>[|<link_name>]
    // Example1: "ft_lot.lot_id|ft_splitlot.lot_id"
    // Example2: "product.product_name|ft_lot.product_name|ft_lot-ft_splitlot"
    GexDbPlugin_Mapping_LinkMap::Iterator	it;
    QString									strDbLink=strLinkName, strQueryCondition;
    while(!strDbLink.isEmpty())
    {
        // Check if mapping available for specified link
        it = m_pmapLinks_Remote->find(strDbLink);
        if(it == m_pmapLinks_Remote->end())
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_MissingLink, NULL, strDbLink.toLatin1().constData());
            return false;
        }
        strDbLink = "";
        strQueryCondition = (*it).m_strSqlFullField1 + "|" + (*it).m_strSqlFullField2;
        strDbLink = (*it).m_strSqlTable2Link;

        // Add condition to the condition list
        if(m_strlQuery_LinkConditions.indexOf(strQueryCondition) == -1)
            m_strlQuery_LinkConditions.append(strQueryCondition);
    }

    return true;
}

bool GexDbPlugin_Base::Query_AddValueCondition(const QString & strQueryFilter,
                                               const QString & strQueryValue,
                                               bool bExpressionRange/*=false*/,
                                               bool bUseLinkConditions/*=true*/)
{
    if(m_pmapFields_GexToRemote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Make sure filter is not empty
    if(strQueryFilter.isEmpty() || (strQueryValue == "*"))
        return true;

    // Check if mapping available for specified field
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping = m_pmapFields_GexToRemote->find(strQueryFilter);
    if(itMapping == m_pmapFields_GexToRemote->end())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Missing mapping for field '%1' !").arg( strQueryFilter).toLatin1().constData() );
        GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strQueryFilter.toLatin1().constData());
        return false;
    }

    // Get mapping fields
    // Syntax is: <table>.<field>[|<link_name>]
    // Example1: "ft_splitlot.job_nam"
    // Example2: "product.product_name|product-ft_lot"
    // Example3: "GEXDB.wt_splitlot.product_name"
    QString	strDbLink, strQueryCondition;
    bool	bNumeric = (*itMapping).m_bNumeric;
    QString strValue = strQueryValue;

    if(bNumeric)
    {
        // In the filter condition, replace '|' with ',', because we use '|' in our internal
        // query syntax
        strValue.replace("|", ",");
        strQueryCondition = (*itMapping).m_strSqlFullField + "|Numeric|" + strValue;
    }
    else if(bExpressionRange)
        strQueryCondition = (*itMapping).m_strSqlFullField + "|ExpressionRange|" + strValue;
    else
    {
        // In the filter condition, replace '|' with GEXDB_PLUGIN_DELIMITER_OR, because we use '|' in our internal
        // query syntax
        strValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
        strQueryCondition = (*itMapping).m_strSqlFullField + "|String|" + strValue;
    }

    // Add condition to the condition list
    if(m_strlQuery_ValueConditions.indexOf(strQueryCondition) == -1)
        m_strlQuery_ValueConditions.append(strQueryCondition);

    // Link conditions
    strDbLink = (*itMapping).m_strSqlLinkName;
    if(bUseLinkConditions && !strDbLink.isEmpty())
        return Query_AddLinkCondition(strDbLink);

    return true;
}

///////////////////////////////////////////////////////////
// Query: add condition string for a specified filter/value
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_AddValueCondition_MultiField(QStringList & strlFields, const QString & strQueryValue, bool bUseLinkConditions/*=true*/)
{
    if(m_pmapFields_GexToRemote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Make sure filter is not empty
    if(strlFields.isEmpty() || (strQueryValue == "*"))
        return true;

    // Iterate through field list, and create vlaue expression
    int										nIndex;
    QString									strField, strValue, strDbLink, strQueryCondition;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping;
    for(nIndex = 0; nIndex < strlFields.size(); nIndex++)
    {
        strField = strlFields.at(nIndex);

        // Check if mapping available for specified field
        itMapping = m_pmapFields_GexToRemote->find(strField);
        if(itMapping == m_pmapFields_GexToRemote->end())
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strField.toLatin1().constData());
            return false;
        }

        // Add mapping field
        // Syntax is: <table>.<field>[|<link_name>]
        // Example1: "ft_splitlot.job_nam"
        // Example2: "product.product_name|product-ft_lot"
        // Example3: "GEXDB.wt_splitlot.product_name"
        if(nIndex > 0)
            strQueryCondition += "+";
        strQueryCondition += (*itMapping).m_strSqlFullField;

        // Add Link condition
        strDbLink = (*itMapping).m_strSqlLinkName;
        if(bUseLinkConditions && !strDbLink.isEmpty() && !Query_AddLinkCondition(strDbLink))
            return false;
    }

    // Create final value condition
    strValue = strQueryValue;

    // In the filter condition, replace '|' with GEXDB_PLUGIN_DELIMITER_OR, because we use '|' in our internal
    // query syntax
    strValue.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
    strQueryCondition += "|String|" + strValue;

    // Add condition to the condition list
    if(m_strlQuery_ValueConditions.indexOf(strQueryCondition) == -1)
        m_strlQuery_ValueConditions.append(strQueryCondition);

    return true;
}

///////////////////////////////////////////////////////////
// Query: return nb of fields to query
// (1 GEX field -> 1 or several DB field)
// Returns -1 if error
///////////////////////////////////////////////////////////
int GexDbPlugin_Base::Query_FieldsToQuery(const QString & strQueryField, QStringList & /*strlFields*/)
{
    // Make sure field is not empty
    if(strQueryField.isEmpty())
        return 0;

    // Check if alias is specified
    QStringList	strlElements = strQueryField.split("=");
    QString		strField = strlElements[0];

    // Make sure no alias was defined
    if(strlElements.size() > 1)
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_AliasOnMultiFieldFilter, NULL, strField.toLatin1().constData());
        return -1;
    }

    return 1;
}

///////////////////////////////////////////////////////////
// Query: add select string for a specified field
// (multi-field)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_AddField_MultiField(QStringList & strlFields, QStringList & strlDbField, QStringList & strlDbTable, bool bUseLinkConditions/*=true*/)
{
    // Clear variables
    strlDbField.clear();
    strlDbTable.clear();

    if(m_pmapFields_GexToRemote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Iterate through field list, and create field expression
    int										nIndex;
    QString									strField, strDbField, strDbTable, strDbLink, strFieldExpression;
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping;

    strFieldExpression = "Field|";
    for(nIndex = 0; nIndex < strlFields.size(); nIndex++)
    {
        strField = strlFields.at(nIndex);

        // Check if mapping available for specified field
        itMapping = m_pmapFields_GexToRemote->find(strField);
        if(itMapping == m_pmapFields_GexToRemote->end())
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strField.toLatin1().constData());
            return false;
        }

        // Get mapping fields
        // Syntax is: <table>.<field>[|<link_name>]
        // Example1: "ft_splitlot.job_nam"
        // Example2: "product.product_name|product-ft_lot"
        // Example3: "GEXDB.wt_splitlot.product_name"
        strDbField = (*itMapping).m_strSqlFullField;
        strDbTable = (*itMapping).m_strSqlTable;
        strDbLink = (*itMapping).m_strSqlLinkName;

        // Add field
        if(nIndex > 0)
            strFieldExpression += "+";
        strFieldExpression += strDbField;

        // Link conditions
        if(bUseLinkConditions && !strDbLink.isEmpty() && !Query_AddLinkCondition(strDbLink))
            return false;

        // Keep track of fields/tables added
        strlDbField << strDbField;
        strlDbTable << strDbTable;
    }

    if(m_strlQuery_Fields.indexOf(strFieldExpression) == -1)
        m_strlQuery_Fields.append(strFieldExpression);

    return true;
}

///////////////////////////////////////////////////////////
// Query: add select string for a specified field
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_AddField(const QString & strQueryField, QString & strDbField, QString & strDbTable, bool bUseLinkConditions/*=true*/, bool bConsolidated/*=false*/)
{
    // Make sure field is not empty
    if(strQueryField.isEmpty())
        return true;

    if(m_pmapFields_GexToRemote == NULL)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoMappingTable, NULL);
        return false;
    }

    // Check if alias is specified
    QStringList	strlElements = strQueryField.split("=");
    QString		strField = strlElements[0];

    // Check if mapping available for specified field
    GexDbPlugin_Mapping_FieldMap::Iterator itMapping = m_pmapFields_GexToRemote->find(strField);
    if(itMapping == m_pmapFields_GexToRemote->end())
    {
        GSET_ERROR1(GexDbPlugin_Base, eDB_MissingMapping, NULL, strField.toLatin1().constData());
        return false;
    }

    // Get mapping fields
    // Syntax is: <table>.<field>[|<link_name>]
    // Example1: "ft_splitlot.job_nam"
    // Example2: "product.product_name|product-ft_lot"
    // Example3: "GEXDB.wt_splitlot.product_name"
    QString		strDbLink;

    strDbField = (*itMapping).m_strSqlFullField;
    strDbTable = (*itMapping).m_strSqlTable;
    strDbLink = (*itMapping).m_strSqlLinkName;

    // Add field
    strField = "";
    if (bConsolidated)
    {
        // to manage Custom Metadata with special cast to string
        if((*itMapping).m_bCustom)
            strField = "Custom";
        strField += "ConsolidatedField";
    }
    else
        strField = "Field";
    if(strlElements.count() > 1)
    {
        strField += "=";
        strField += strlElements[1];
    }
    strField += "|";
    strField += strDbField;
    if(m_strlQuery_Fields.indexOf(strField) == -1)
        m_strlQuery_Fields.append(strField);

    // Link conditions
    if(bUseLinkConditions && !strDbLink.isEmpty())
        return Query_AddLinkCondition(strDbLink);

    return true;
}

///////////////////////////////////////////////////////////
// Query: add time period condition to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_AddTimePeriodCondition(GexDbPlugin_Filter &cFilters)
{
    // Check if time period should be used
    if(!cFilters.bUseTimePeriod)
        return true;

    // Default: add condition based on time_t from and to values
    QString strFilterCondition;
    strFilterCondition =	QString::number(cFilters.tQueryFrom);
    strFilterCondition +=	"-";
    strFilterCondition +=	QString::number(cFilters.tQueryTo);
    return Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, strFilterCondition);
}

///////////////////////////////////////////////////////////
// Query: add filters to query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_AddFilters(GexDbPlugin_Filter &cFilters)
{
    QString						strQueryFilter, strQueryField, strQueryValue;
    QStringList::ConstIterator	it;
    int							nNbFields;
    QStringList					strlFields;

    for(it = cFilters.strlQueryFilters.begin(); it != cFilters.strlQueryFilters.end(); it++)
    {
        strQueryFilter = (*it);
        strQueryField = strQueryFilter.section('=', 0, 0);
        strQueryValue = strQueryFilter.section('=', 1, 1);
        nNbFields = Query_FieldsToQuery(strQueryField, strlFields);
        if(nNbFields == -1)
            return false;
        if(nNbFields > 1)
        {
            if(!Query_AddValueCondition_MultiField(strlFields, strQueryValue))
                return false;
        }
        else
        {
            if(Query_AddValueCondition(strQueryField, strQueryValue))
            {
                if(strQueryField == m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR])
                {
                    // In the filter condition, replace '|' with ',', because we use '|' in our internal
                    // query syntax
                    strQueryValue.replace("|", ",");
                    cFilters.strSiteFilterValue = strQueryValue;
                }
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Query: construct query string for field query
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConstructFieldQuery(GexDbPlugin_Filter &cFilters,
                                           QString & strQuery,
                                           bool bDistinct,
                                           QuerySort eQuerySort)
{
    GSLOG(SYSLOG_SEV_DEBUG, "");
    QString		strDbField, strDbTable, strQueryField;
    QStringList	strlFields, strlDbField, strlDbTable;
    int			nNbFields;

    // Clear query string
    strQuery = "";

    // Make sure we have a field to query on
    if(cFilters.mQueryFields.isEmpty())
        return true;

    //if(eQuerySort != GexDbPlugin_Base::eQuerySort_None)
    //	strQueryField += "=GexDbSortField";

    // Construct query string:
    // SELECT <table.field>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)

    QStringList::const_iterator itQueryField = cFilters.mQueryFields.constBegin();

    cFilters.nNbQueryFields =0;

    while (itQueryField != cFilters.mQueryFields.constEnd())
    {
        strQueryField = (*itQueryField);

        // Set field to query. Check if multiple fields
        nNbFields = Query_FieldsToQuery(strQueryField, strlFields);
        if(nNbFields == -1)
            return false;

        cFilters.nNbQueryFields += nNbFields;

        if(nNbFields > 1)
            Query_AddField_MultiField(strlFields, strlDbField, strlDbTable);
        else
            Query_AddField(strQueryField, strDbField, strDbTable);

        // Add order condition
        if(nNbFields == 1)
        {
            if(eQuerySort == GexDbPlugin_Base::eQuerySort_Asc)
            {
                // If DISTINCT => no need to add ORDER BY
                if(!bDistinct)
                {
                    m_strlQuery_OrderFields.append(strDbField + "|ASC");
                }
            }
            else if(eQuerySort == GexDbPlugin_Base::eQuerySort_Desc)
                m_strlQuery_OrderFields.append(strDbField + "|DESC");
        }

        ++itQueryField;
    }

    // Set filters
    Query_AddFilters(cFilters);

    // Add time period condition
    Query_AddTimePeriodCondition(cFilters);

    // Check all query parameters
    if(m_strlQuery_Fields.isEmpty())
        return false;

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery, bDistinct);

    return true;
}

QString GexDbPlugin_Base::Query_BuildSqlStringExpression( const QString &aValue, bool aForSqlLIKE/*=false*/)
{
   /* This process is called
    * when a SQL LIKE expression is build
    * or when a SQL = expression is build
    * to normalize the filter
   * For this 2 method, the SQL syntax is different
   * SQL LIKE expression:
   * GEX uses WildChards * and ?
   * SQL uses WildChards % and _
   * The goal is to transfer the GEX syntax to SQL
   * and to escape the SQL WildChards
   *  %  become \% => ESCAPE FOR SQL
   *  _  become \_ => ESCAPE FOR SQL
   *  \* become *  => not WildChard
   *  \? become ?  => not WildChard
   *  *  become %  => the WildChard
   *  ?  become _  => the WildChard
   *  \  become \\\\ => BackSlash FOR SQL LIKE
   *
   * SQL = expression
   * double the BackSlash
   */

    // To search for “\”,
    // specify it as “\\” for = expression
    // specify it as “\\\\” for LIKE expression
    // this is because the backslashes are stripped
    // once by the parser and again when the pattern match is made,
    // leaving a single backslash to be matched against.

    // destination string
    QString lDest;
    // First apply the Escape needed for SQL = expression
    // Remove non Latin char
    if(aValue.isNull() || aValue.isEmpty() || (aValue[0].toLatin1() == 0))
    {
        lDest = "";
    }
    else
    {
        // replace non latin1 characters by "?"
        lDest = QString::fromLatin1(aValue.toLatin1());
    }

    // Replace temporarely with a PlaceHolder
    lDest = lDest.replace("\\*","#GEX#STAR#");
    lDest = lDest.replace("\\?","#GEX#QUESTION#");

    // Double the quote
    lDest.replace("'","''");
    // Special Character Escape Sequences
    // Double the BackSlash for the MySql parser
    lDest.replace("\\","\\\\");

    if(aForSqlLIKE)
    {

        // BackSlash FOR SQL LIKE
        // Double for the MySql pattern
        lDest = lDest.replace("\\","\\\\");

        // ESCAPE FOR SQL
        // Escape %
        lDest = lDest.replace("%","\\%");
        // Escape _
        lDest = lDest.replace("_","\\_");

        // WildChard FOR SQL LIKE
        // WildChard * to %
        lDest = lDest.replace("*","%");
        // WildChard ? to _
        lDest = lDest.replace("?","_");

    }

    // Go back from PlaceHolder
    lDest = lDest.replace("#GEX#STAR#","*");
    lDest = lDest.replace("#GEX#QUESTION#","?");

    return lDest;
}


//////////////////////////////////////////////////////////////////////
// Return all valid values for a field (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QueryField(GexDbPlugin_Filter& cFilters,
                                  QStringList& cMatchingValues,
                                  //TODO: bSoftBin unused ?
                                  bool /* bSoftBin = false*/,
                                  bool bDistinct/* = true*/,
                                  QuerySort eQuerySort/* = eQuerySort_Asc*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, " Query Field ");
    // Clear returned stringlist
    cMatchingValues.clear();

    // Check database connection
    if(!ConnectToCorporateDb())
    {
        GSLOG(SYSLOG_SEV_ERROR, "cant connect to DB !");
        return false;
    }

    // Compute query date constraints
    QString r=Query_ComputeDateConstraints(cFilters);
    if (r.startsWith("error"))
        GSLOG(SYSLOG_SEV_WARNING, r.toLatin1().data());

    // Construct SQL query
    QString strQuery;
    ConstructFieldQuery(cFilters, strQuery, bDistinct, eQuerySort);

    // Execute query
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Populate stringlist
    QString fieldValue;
    QString matchingValues;

    while(clGexDbQuery.Next())
    {
        matchingValues.clear();
        bool validValue = false;

        // extract values for all requested fields (concatenated with '|' as separator)
        for (int fieldIdx = 0; fieldIdx < clGexDbQuery.record().count(); ++fieldIdx)
        {
            if(!clGexDbQuery.value(fieldIdx).isNull())
                fieldValue = clGexDbQuery.value(fieldIdx).toString();
            else
                fieldValue.clear();

            if (fieldIdx != 0)
                matchingValues += "|";

            if(!fieldValue.isEmpty())
            {
                matchingValues += fieldValue;

                validValue = true;
            }
        }

        // Add results if all values are valid
        if (validValue)
            cMatchingValues.append(matchingValues);
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 values found for queried field.")
          .arg( cMatchingValues.size()).toLatin1().constData());

    return true;
}

///////////////////////////////////////////////////////////////////////
// Query: construct query string to retrieve all tests matching filters
///////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConstructTestlistQuery(GexDbPlugin_Filter &cFilters, QString& strQuery, QString& strTestNumField, QString& strTestNameField, QString strTestTypeField /*=""*/)
{
    QString strDbField, strDbTable;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    Query_Empty();

    // Set field to query
    Query_AddField(strTestNumField, strDbField, strDbTable);
    Query_AddField(strTestNameField, strDbField, strDbTable);
    if(!strTestTypeField.isEmpty())
    {
        Query_AddField(strTestTypeField, strDbField, strDbTable);
    }

    // Set filters
    Query_AddFilters(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery,
                         true,
                         "",
                         GS::DbPluginBase::QueryEngine::IgnoreFunctionalDep);

    return true;
}

bool GexDbPlugin_Base::ConstructFlowListQuery(GexDbPlugin_Filter &cFilters, QString &strQuery, QString &strFlowField)
{
    QString strDbField, strDbTable;

    // Clear query string
    strQuery = "";

    // Construct query string:
    Query_Empty();

    // Set field to query
    Query_AddField(strFlowField, strDbField, strDbTable);

    // Set filters
    Query_AddFilters(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery,
                         true,
                         "",
                         GS::DbPluginBase::QueryEngine::IgnoreFunctionalDep);

    return true;
}

///////////////////////////////////////////////////////////////////////
// Query: construct query string to retrieve all retest phases matching filters
///////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConstructInsertionListQuery(GexDbPlugin_Filter &cFilters, QString &strQuery, QString &strInsertionField)
{
    QString strDbField, strDbTable;

    // Clear query string
    strQuery = "";

    // Construct query string:
    Query_Empty();

    // Set field to query
    Query_AddField(strInsertionField, strDbField, strDbTable);

    // Set filters
    Query_AddFilters(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery,
                         true,
                         "",
                         GS::DbPluginBase::QueryEngine::IgnoreFunctionalDep);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all tests (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QueryTestlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues, bool bParametricOnly)
{
    QString				strQuery, strTestNumField, strTestNameField;
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Clear returned stringlist
    cMatchingValues.clear();

    // Make sure some filters are defined to avoid querying about a potentially huge table with a full scan
    if(cFilters.strlQueryFilters.isEmpty())
    {
        // All filters are empty, don't execute the query, give a message for the user
        cMatchingValues.append("No filters are defined:");
        cMatchingValues.append("to avoid a full scan on the test information table,");
        cMatchingValues.append("please define at least one filter.");
        return true;
    }

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    clGexDbQuery.setForwardOnly(true);

    /////////////////////////////////////////////////////////////////////////////
    // PTR tests
    /////////////////////////////////////////////////////////////////////////////
    // Construct SQL query
    strTestNumField = GEXDB_PLUGIN_DBFIELD_TESTNUM;
    strTestNameField = GEXDB_PLUGIN_DBFIELD_TESTNAME;
    ConstructTestlistQuery(cFilters, strQuery, strTestNumField, strTestNameField);

    // Execute query
    if(!strQuery.isEmpty() && !clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Add to stringlist
    while(clGexDbQuery.Next())
    {
        cMatchingValues.append(clGexDbQuery.value(0).toString());
        cMatchingValues.append(clGexDbQuery.value(1).toString());
        cMatchingValues.append("P");
    }

    /////////////////////////////////////////////////////////////////////////////
    // MPR tests
    /////////////////////////////////////////////////////////////////////////////
    // Construct SQL query
    strTestNumField = GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR;
    strTestNameField = GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR;
    ConstructTestlistQuery(cFilters, strQuery, strTestNumField, strTestNameField);

    // Execute query
    if(!strQuery.isEmpty() && !clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Add to stringlist
    while(clGexDbQuery.Next())
    {
        cMatchingValues.append(clGexDbQuery.value(0).toString());
        cMatchingValues.append(clGexDbQuery.value(1).toString());
        cMatchingValues.append("MP");
    }

    /////////////////////////////////////////////////////////////////////////////
    // FTR tests
    /////////////////////////////////////////////////////////////////////////////
    if(!bParametricOnly)
    {
        // Construct SQL query
        strTestNumField = GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR;
        strTestNameField = GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR;
        ConstructTestlistQuery(cFilters, strQuery, strTestNumField, strTestNameField);

        // Execute query
        if(!strQuery.isEmpty() && !clGexDbQuery.Execute(strQuery))
        {
            // Display error
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
            return false;
        }

        // Add to stringlist
        while(clGexDbQuery.Next())
        {
            cMatchingValues.append(clGexDbQuery.value(0).toString());
            cMatchingValues.append(clGexDbQuery.value(1).toString());
            cMatchingValues.append("F");
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////
// Query: construct query string to retrieve all binnings matching filters
///////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::ConstructBinlistQuery(GexDbPlugin_Filter &cFilters, QString & strQuery, const QString & strBinType, bool bSoftBin/*=false*/, bool bClearQueryFirst/*=true*/,bool bIncludeBinName/*=false*/, bool bProdDataOnly/*=false*/)
{
    QString strDbField, strDbTable, strCondition;

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    if(bClearQueryFirst)
    {
        Query_Empty();
    }
    // Set field to query
    if(bSoftBin)
    {
        Query_AddField(GEXDB_PLUGIN_DBFIELD_SBIN, strDbField, strDbTable);
        if(bIncludeBinName)
        {
            Query_AddField(GEXDB_PLUGIN_DBFIELD_SBIN_NAME, strDbField, strDbTable);
        }
    }
    else
    {
        Query_AddField(GEXDB_PLUGIN_DBFIELD_HBIN, strDbField, strDbTable);
        if(bIncludeBinName)
        {
            Query_AddField(GEXDB_PLUGIN_DBFIELD_HBIN_NAME, strDbField, strDbTable);
        }
    }

    // Check if which type of binnings should be retrieved
    if(strBinType.toLower() == "pass")
    {
        if(bSoftBin)
        {
            Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_SBIN_PF, "P");
        }
        else
        {
            Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_HBIN_PF, "P");
        }
    }
    else if(strBinType.toLower() == "fail")
    {
        if(bSoftBin)
        {
            Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_SBIN_PF, "F");
        }
        else
        {
            Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_HBIN_PF, "F");
        }
    }

    // Set filters
    Query_AddFilters(cFilters);

    if (!Query_AddTimePeriodCondition(cFilters))
    {
        qDebug("GexDbPlugin_Base::ConstructBinlistQuery: Query_AddTimePeriodCondition failed !");
    }

    // Add data filter
    if(bProdDataOnly)
    {
        Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_PROD_DATA,"Y");
    }
    Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT,"Y");

    // Construct query from table and conditions
    Query_BuildSqlString_UsingJoins(strQuery);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all binnings (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QueryBinlist(GexDbPlugin_Filter & cFilters,QStringList & cMatchingValues,bool bSoftBin/*=false*/,bool bClearQueryFirst/*=true*/,bool bIncludeBinName/*=false*/, bool bProdDataOnly/*=false*/)
{
    QString strBinning;

    // Clear returned stringlist
    cMatchingValues.clear();

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Construct SQL query
    QString strQuery;

    Query_ComputeDateConstraints(cFilters);

    ConstructBinlistQuery(cFilters, strQuery, "all", bSoftBin, bClearQueryFirst, bIncludeBinName, bProdDataOnly);

    // Execute query
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Populate stringlist
    while(clGexDbQuery.Next())
    {
        strBinning = QString::number(clGexDbQuery.value(0).toInt());
        if(bIncludeBinName && !clGexDbQuery.isNull(1))
            strBinning += "#GEX#" + clGexDbQuery.value(1).toString();
        cMatchingValues.append(strBinning);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all products (use only date in the filter)
// This function must be defined in the derived class
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QueryProductList(GexDbPlugin_Filter& /*cFilters*/,
                                        QStringList& cMatchingValues,
                                        QString /*strProductName = ""*/)
{
    cMatchingValues.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all products for genealogy reports, with data for at least
// 2 testing stages (use only date in the filter)
// This function must be defined in the derived class
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QueryProductList_Genealogy(GexDbPlugin_Filter& /*cFilters*/,
                                                  QStringList& cMatchingValues,
                                                  bool /*bAllTestingStages*/)
{
    cMatchingValues.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all test conditions corresponding to the splitlots
// According to the given filters
// This function must be defined in the derived class
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::QueryTestConditionsList(GexDbPlugin_Filter &/*cFilters*/, QStringList &cMatchingValues)
{
    cMatchingValues.clear();
    return true;
}

///////////////////////////////////////////////////////////
// Execute query (write and cumul execution time if in debug mode)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_Execute(QSqlQuery & clQuery, const QString & strQuery)
{
    QString	strDebugMessage, strTemp;

    // Save query
    m_strQuery = strQuery;

    // Debug message ??
    if(m_bCustomerDebugMode)
    {
        strTemp = strQuery;
        if(strTemp.endsWith("\n"))
            strTemp.truncate(strTemp.length()-1);
        strDebugMessage = "\nSQL query:";
        strDebugMessage += strTemp;

#ifndef NO_QUERY_DUMP
        WriteDebugMessageFile(strDebugMessage);
#endif

        m_clQueryTimer.start();
    }

    // Wait cursor
    // qApp->setOverrideCursor(QCursor(Qt::BusyCursor));
    emit sBusy(true);

    // Exec query
    bool bStatus = clQuery.exec(strQuery);

    // Restore cursor
    // qApp->restoreOverrideCursor();
    emit sBusy(false);

    // Debug message ??
    if(m_bCustomerDebugMode)
    {
        m_uiTimer_DbQuery = m_clQueryTimer.elapsed();
        m_uiTimer_DbQuery_Cumul_Total += m_uiTimer_DbQuery;
        m_uiTimer_DbQuery_Cumul_Partial += m_uiTimer_DbQuery;

        m_clQueryTimer.start();
    }

    return bStatus;
}

///////////////////////////////////////////////////////////
// Iterate to next query result item (cumul execution time if in debug mode)
///////////////////////////////////////////////////////////
bool GexDbPlugin_Base::Query_Next(QSqlQuery & clQuery)
{
    QTime			clTime;
    unsigned int	uiExecutionTime, uiExecutionTime_Cumul;
    QString			strDebugMessage;

    // Compute execution time if in debug mode
    if(m_bCustomerDebugMode)
        clTime.start();

    // Exec query
    bool bStatus = clQuery.next();

    // Debug message ??
    if(m_bCustomerDebugMode)
    {
        uiExecutionTime = clTime.elapsed();
        uiExecutionTime_Cumul = m_clQueryTimer.elapsed();
        if(bStatus)
        {
            m_uiTimer_DbIteration += uiExecutionTime;
            m_uiTimer_DbIteration_Cumul_Partial += uiExecutionTime;
            m_uiTimer_DbIteration_Cumul_Total += uiExecutionTime;
        }
        else
        {
#ifndef NO_DUMP_PERF
            strDebugMessage = "Query performance:\n";
            strDebugMessage += "DB query execution = " + QString::number(m_uiTimer_DbQuery) + "ms\n";
            strDebugMessage += "DB query iteration = " + QString::number(m_uiTimer_DbIteration) + "ms\n";
            strDebugMessage += "Global iteration = " + QString::number(uiExecutionTime_Cumul) + "ms\n";
            WriteDebugMessageFile(strDebugMessage);
#endif
        }
    }

    return bStatus;
}

///////////////////////////////////////////////////////////
// For debug purpose, write partial performance to debug trace file
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::WritePartialPerformance(const char* szFunction)
{
#ifdef NO_DUMP_PERF
    return;
#endif

    if(m_bCustomerDebugMode)
    {
        QString strMessage =	"---- ";
        strMessage += szFunction;
        strMessage += " performance:\n";
        strMessage += "---- Execution time           = " + QString::number(m_clExtractionPerf.m_fTime_Partial/1000.0F, 'f', 2) + " ms\n";
        strMessage += "---- SQL query execution      = " + QString::number(m_clExtractionPerf.m_fTimer_DbQuery_Partial/1000.0F, 'f', 2) + " ms\n";
        strMessage += "---- SQL query iteration      = " + QString::number(m_clExtractionPerf.m_fTimer_DbIteration_Partial/1000.0F, 'f', 2) + " ms\n";
        strMessage += "---- SQL query extracted rows = " + QString::number(m_clExtractionPerf.m_ulNbRows_Partial) + " \n";
        WriteDebugMessageFile(strMessage);
    }
}


///////////////////////////////////////////////////////////////////////////////////
// return true, if the word is reserved in connected db or if no db connected.
///////////////////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Base::IsReservedWord(QString strWordToTest)
{
    if(!m_pclDatabaseConnector)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Database connection not defined !");
        return true;
    }

    if(m_pclDatabaseConnector->m_lstSqlReservedWords.contains(strWordToTest.toUpper()))
        return true;

    return false;
}

bool GexDbPlugin_Base::IsTdrAllowed()
{
    bool            lOk = false;
    unsigned int    lOptionalmodules;
    QString         lFieldName = "optional_modules";

    if (mGexInfoMap.contains(lFieldName))
    {
        lOptionalmodules = mGexInfoMap.value(lFieldName).toUInt(&lOk);
        if (lOk)
            return lOptionalmodules & GS::LPPlugin::LicenseProvider::eTDR;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Unable to use info %1 value: %2").
                  arg(lFieldName).arg(mGexInfoMap.value(lFieldName)).
                  toLatin1().constData());
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_ERROR, QString("Missing info %1").arg(lFieldName).
          toLatin1().constData());
    return false;
}

bool GexDbPlugin_Base::IsMonitoringMode()
{
    bool lOk = false;
    long lProductId;
    QString lFieldName = "product_id";

    if (mGexInfoMap.contains(lFieldName))
    {
        lProductId = mGexInfoMap.value(lFieldName).toLong(&lOk);
        if (lOk)
            return ((lProductId == GS::LPPlugin::LicenseProvider::eYieldMan) || (lProductId == GS::LPPlugin::LicenseProvider::eYieldManEnterprise)
                    || (lProductId == GS::LPPlugin::LicenseProvider::ePATMan) || (lProductId == GS::LPPlugin::LicenseProvider::ePATManEnterprise));
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Unable to use info %1 value: %2").
                  arg(lFieldName).arg(mGexInfoMap.value(lFieldName)).
                  toLatin1().constData());
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_ERROR, QString("Missing info %1").arg(lFieldName).
          toLatin1().constData());
    return false;
}

bool GexDbPlugin_Base::LoadDatabaseArchitecture(QDomDocument &document)
{
    if (mDbArchitecture)
    {
        delete mDbArchitecture;
        mDbArchitecture = NULL;
        mQueryEngine->SetDbArchitecture(NULL);
    }

    mDbArchitecture = new GS::DbPluginBase::DbArchitecture(
                m_pclDatabaseConnector->m_strSchemaName,
                this);
    return mDbArchitecture->LoadFromDom(document);
}


bool GexDbPlugin_Base::LoadQueryEngine()
{
    if (!mDbArchitecture)
        return false;

    if (!mQueryEngine)
        mQueryEngine = new GS::DbPluginBase::QueryEngine;


    return mQueryEngine->SetDbArchitecture(mDbArchitecture);
}

void GexDbPlugin_Base::WriteDebugMessageFile(const QString & strMessage, bool /*bUpdateGlobalTrace=false*/) const
{
    QString strLogMessage;

    if(m_pclDatabaseConnector && m_pclDatabaseConnector->m_nConnectionSID>0)
        strLogMessage = "[SID:"+QString::number(m_pclDatabaseConnector->m_nConnectionSID)+"] ";
    strLogMessage += strMessage;
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1").arg( strLogMessage.left(65280).toLatin1().constData()).toLatin1().constData());
        }

///////////////////////////////////////////////////////////
// Display Error message
///////////////////////////////////////////////////////////
void GexDbPlugin_Base::DisplayErrorMessage()
{
    QString strErrorMessage;
    QString strTitle = "Quantix Examinator plug-in: ";
    strTitle += m_strPluginName;

    GetLastError(strErrorMessage);
    if (!mGexScriptEngine->property("GS_DAEMON").toBool())
        QMessageBox::critical(NULL, strTitle, strErrorMessage);
    else
        GSLOG(SYSLOG_SEV_CRITICAL, (m_strPluginName + ": " + strErrorMessage).
              toLatin1().constData());
}
