#ifdef _WIN32
#include <windows.h>
#endif

#include "gexftp_settings.h"
#include "gexftp_constants.h"
#include "gexftp_browse_dialog.h"
#include <gex_constants.h>

#include <gstdl_utils_c.h>
#include <gqtl_sysutils.h>
#include <gstdl_crypto.h>
#include <gstdl_membuffer.h>
#include <QProgressDialog>
#include <QIcon>
#include <QMessageBox>


extern char *	szAppFullName;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpSettingsDataGeneral::CGexFtpSettingsDataGeneral() : QObject()
{
	reset();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFtpSettingsDataGeneral::~CGexFtpSettingsDataGeneral()
{
}

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////
void CGexFtpSettingsDataGeneral::setVersion(const QString &strVersion)
{
	m_strVersion = strVersion;
	emit sDataGeneralChanged();
}


void CGexFtpSettingsDataGeneral::checkVersion(const QString &strVersion)
{
	if (m_strVersion != strVersion)
		m_bIsNewVersion = true;
	else
		m_bIsNewVersion = false;
}

void CGexFtpSettingsDataGeneral::setEvery(bool bEvery)
{
	m_bEvery = bEvery;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setDaily(bool bDaily)
{
	m_bDaily = bDaily;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setWeekly(bool bWeekly)
{
	m_bWeekly = bWeekly;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setEveryHour(int nHour)
{
	m_nEveryHour = nHour;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setEveryMinute(int nMinute)
{
	m_nEveryMinute = nMinute;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setEverySecond(int nSecond)
{
	m_nEverySecond = nSecond;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setDailyAt(const QTime& timeAt)
{
	m_timeDailyAt = timeAt;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setDailyEveryNDays(int nDays)
{
	m_nDailyEveryNDays = nDays;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setWeeklyAt(const QTime& timeAt)
{
	m_timeWeeklyAt = timeAt;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setWeeklyOn(int nDay)
{
	m_nWeeklyOn = nDay;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setNbDaysToKeepLog(int nDays)
{
	m_nNbDaysToKeepLog = nDays;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setFullLog(bool bFullLog)
{
	m_bFullLog = bFullLog;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setFtpTimeout(int nFtpTimeout)
{
	m_bFtpTimeout = (bool) nFtpTimeout;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setFtpTimeoutAfter(int nSecond)
{
	m_nFtpTimeoutAfter = nSecond;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setFileAgeBeforeTransfer_Min(int nMinutes)
{
	m_nFileAgeBeforeTransfer_Min = nMinutes;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setEnableMaxFilePerProfile(bool bUse)
{
	m_bEnableMaxFilePerProfile = bUse;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setMaxFilePerProfile(int iMaxFile)
{
	m_iMaxFilePerProfile = (iMaxFile > 0) ? iMaxFile : 0;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setMaxReconnectionAttempts(int iMaxReconnect)
{
	m_iMaxReconnectionAttempts = (iMaxReconnect > 0) ? iMaxReconnect : 0;

	emit sDataGeneralChanged();
}

void CGexFtpSettingsDataGeneral::setCaseInsensitive(bool bCaseInsensitive){
    m_bCaseInsensitive = bCaseInsensitive;
    emit sDataGeneralChanged();
}

bool CGexFtpSettingsDataGeneral::enableMaxFilePerProfile() const
{
	// To ensure it won't disable downloads, if m_iMaxFilePerProfile is equal or less than 0, consider it's disabled
	if (m_iMaxFilePerProfile > 0)
		return m_bEnableMaxFilePerProfile;
	else
		return false;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Set the defualt settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettingsDataGeneral::reset()
{
	m_bEvery						= true;
	m_bDaily						= false;
	m_bWeekly						= false;
	m_nEveryHour					= 0;
	m_nEveryMinute					= 30;
	m_nEverySecond					= 0;
	m_timeDailyAt					= QTime(12, 0, 0);
	m_nDailyEveryNDays				= 1;
	m_timeWeeklyAt					= QTime(12, 0, 0);
	m_nWeeklyOn						= 1;
	m_bFullLog						= false;
	m_nNbDaysToKeepLog				= 7;
	m_bFtpTimeout					= false;
	m_nFtpTimeoutAfter				= 60;
	m_nFileAgeBeforeTransfer_Min	= 5;
	m_bIsNewVersion					= true;
	m_bEnableMaxFilePerProfile		= false;
	m_iMaxFilePerProfile			= 1;
	m_iMaxReconnectionAttempts		= 0;
        m_bCaseInsensitive                      = false;
}

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpSettingsDataFtp::CGexFtpSettingsDataFtp() : QObject()
{
	reset();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFtpSettingsDataFtp::~CGexFtpSettingsDataFtp()
{
}

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
void CGexFtpSettingsDataFtp::addProfile(const CGexFtpServer& ftpServer, addType eType)
{
	td_itProfile itFind;

	if (findProfile(ftpServer.settingsName(), itFind))
		(*itFind) = ftpServer;
	else
		m_lstProfile.append(ftpServer);

	if (eType == addFromGui)
		emit sDataFtpChanged();
}

void CGexFtpSettingsDataFtp::removeProfile(const QString& strProfileName)
{
	td_itProfile itFind;

	if (findProfile(strProfileName, itFind))
	{
		m_lstProfile.erase(itFind);

		emit sDataFtpChanged();
	}
}

bool CGexFtpSettingsDataFtp::findProfile(const QString& strProfileName, td_itProfile& profileFound)
{
	profileFound		= m_lstProfile.begin();
	td_itProfile itEnd	= m_lstProfile.end();

	while(profileFound != itEnd)
	{
		if ((*profileFound).settingsName() == strProfileName)
			return true;

		profileFound++;
    }

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// Set the defualt settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettingsDataFtp::reset()
{
    m_lstProfile.clear();
}

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFtpSettings::CGexFtpSettings(const QString& strLocalFolder) : QObject()
{
	m_strSettingsFile = strLocalFolder + "/.gex_ftp.conf";

	CGexSystemUtils::NormalizePath(m_strSettingsFile);

	reset();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFtpSettings::~CGexFtpSettings()
{
}

void CGexFtpSettings::setServiceHomeDir(const QString& strHomeDir)
{
	m_strServiceHomeDir			= strHomeDir;
	m_strServiceSettingsFile	= m_strServiceHomeDir + "/.gex_ftp.conf";

	CGexSystemUtils::NormalizePath(m_strServiceHomeDir);
	CGexSystemUtils::NormalizePath(m_strServiceSettingsFile);

	onDataChanged();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Save the settings in the configuration file
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::save()
{
	save(m_strSettingsFile);
}

void CGexFtpSettings::saveService()
{
	save(m_strServiceSettingsFile);
}

void CGexFtpSettings::save(const QString& strSettingsFile)
{
	FILE*	hSettingsFile;
	bool	bResult = true;

	// Open settings file
    hSettingsFile = fopen(strSettingsFile.toLatin1().constData(), "w+t");
	if(hSettingsFile)
    {
		// General section
		if(ut_WriteSectionToIniFile(hSettingsFile,"General") == UT_ERR_OKAY)
		{
            ut_WriteEntryToIniFile(hSettingsFile, "GexFtpVersion",
                m_dataGeneral.version().toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalEvery",
                m_dataGeneral.every() ? "1" : "0");
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalEvery_Hours",
                QString::number(m_dataGeneral.everyHour()).
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalEvery_Minutes",
                QString::number(m_dataGeneral.everyMinute()).
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalEvery_Seconds",
                QString::number(m_dataGeneral.everySecond()).
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalDaily",
                m_dataGeneral.daily() ? "1" : "0");
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalDaily_Time",
                m_dataGeneral.dailyAt().toString().
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile,
                "SpoolIntervalDaily_Everydays",
                QString::number(m_dataGeneral.dailyEveryNDays()).
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalWeekly",
                m_dataGeneral.weekly() ? "1" : "0");
            ut_WriteEntryToIniFile(hSettingsFile, "SpoolIntervalWeekly_Time",
                m_dataGeneral.weeklyAt().toString().
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile,
                "SpoolIntervalWeekly_Weekday",
                QString::number(m_dataGeneral.weeklyOn()).
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "LogCleanupInterval",
                QString::number(m_nLogCleanupInterval).toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "DaysToKeepLogFiles",
                QString::number(m_dataGeneral.nbDaysToKeepLog()).
                toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "FullLog",
                m_dataGeneral.fullLog() ? "1" : "0");
            ut_WriteEntryToIniFile(hSettingsFile, "EnableFtpTimeout",
                m_dataGeneral.ftpTimeout() ? "1" : "0");
            ut_WriteEntryToIniFile(hSettingsFile, "FtpTimeoutSeconds",
               QString::number(m_dataGeneral.ftpTimeoutAfter()).
               toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "FileAgeBeforeTransfer_Min",
               QString::number(m_dataGeneral.fileAgeBeforeTransfer_Min()).
               toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "EnableMaxFilePerProfile",
               m_dataGeneral.enableMaxFilePerProfile() ? "1" : "0");
            ut_WriteEntryToIniFile(hSettingsFile, "MaxFilePerProfile",
               QString::number(m_dataGeneral.maxFilePerProfile()).
               toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "MaxReconnectionAttempts",
               QString::number(m_dataGeneral.maxReconnectionAttempts()).
               toLatin1().constData());
            ut_WriteEntryToIniFile(hSettingsFile, "CaseInsensitive",
               QString::number(m_dataGeneral.caseInsensitive()).
               toLatin1().constData());

			fputs("\n",hSettingsFile);

			// Ftp client pages
			QString		strSectionName, strCryptedPasswordHexa;
			int			nSettingsNumber = 1;

			for(td_constitProfile it = m_dataFtp.lstProfile().constBegin() ; it != m_dataFtp.lstProfile().constEnd() && bResult; ++it)
			{
				// Create section name: [Ftp:<settings name>]
				strSectionName = "Ftp:" + QString::number(nSettingsNumber++);

				// Write section name
                if (ut_WriteSectionToIniFile(hSettingsFile,
                                            strSectionName.toLatin1().
                                            constData()) == UT_ERR_OKAY)
				{
					// Crypt password
					CryptPassword((*it).password(), strCryptedPasswordHexa);

					// Write all entries
                    ut_WriteEntryToIniFile(hSettingsFile, "SettingsName",
                        ((*it).settingsName().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "SiteURL",
                        ((*it).ftpSiteURL().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "PortNb",
                        (QString::number((*it).ftpPortNb()).
                         toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "Login",
                        ((*it).login().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "Password",
                        (strCryptedPasswordHexa.toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "RemoteDir",
                        ((*it).remoteDir().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "RecursiveSearch",
                        (QString::number((*it).recursiveSearch()).
                         toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "LocalDir",
                        ((*it).localDir().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "FileExtensions",
                        ((*it).fileExtensions().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "FilePolicy",
                        ((*it).filePolicy().toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "DownloadOrder",
                        ((*it).downloadOrder().toLatin1().constData()));

					if((*it).profileEnabled() == true)
						ut_WriteEntryToIniFile(hSettingsFile, "ProfileEnabled", "1");
                    else
						ut_WriteEntryToIniFile(hSettingsFile, "ProfileEnabled", "0");

					if((*it).useDateTimeWindow() == true)
						ut_WriteEntryToIniFile(hSettingsFile, "UseDateTimeWindow", "1");
                    else
						ut_WriteEntryToIniFile(hSettingsFile, "UseDateTimeWindow", "0");

					if((*it).disabledDateTimeCheck() == true)
						ut_WriteEntryToIniFile(hSettingsFile, "DisabledDateTimeCheck", "1");
                    else
						ut_WriteEntryToIniFile(hSettingsFile, "DisabledDateTimeCheck", "0");

                    ut_WriteEntryToIniFile(hSettingsFile, "FromDate",
                        (QString::number((*it).dateFrom().toTime_t()).
                         toLatin1().constData()));
                    ut_WriteEntryToIniFile(hSettingsFile, "ToDate",
                        (QString::number((*it).dateTo().toTime_t()).
                         toLatin1().constData()));

					fputs("\n",hSettingsFile);
				}
                else
					bResult = false;
			}

			// Service page
			// Write section name: [Service]
			if(bResult && ut_WriteSectionToIniFile(hSettingsFile, "Service") == UT_ERR_OKAY)
			{
				// Write all entries
                ut_WriteEntryToIniFile(hSettingsFile, "HomeDir",
                                       m_strServiceHomeDir.
                                       toLatin1().constData());
				fputs("\n",hSettingsFile);
			}
			else
				bResult = false;
		}
		else
			bResult = false;

		// Close settings file
		fclose(hSettingsFile);
	}
    else
		bResult = false;

	// Update changed status
	if (bResult)
		m_bDataChanged = false;

	// emit a signal
	emit sSettingsSaved(bResult, strSettingsFile);
}

/////////////////////////////////////////////////////////////////////////////////////
// Load the settings in the configuration file
/////////////////////////////////////////////////////////////////////////////////////
int CGexFtpSettings::load()
{
	return load(m_strSettingsFile);
}

int CGexFtpSettings::loadService()
{
	return load(m_strServiceSettingsFile);
}

int CGexFtpSettings::load(const QString strSettingsFile)
{
	FILE *	hSettingsFile;
	int		iReadingStatus = 0;
	QString	strValue = "";
	bool	bLoadedSuccessfully = false;
	m_dataGeneral.setVersion(szAppFullName);
	m_mapErrTypeErrMsg.clear();

	// First set default settings
	reset();

	// Open settings file
    hSettingsFile = fopen(strSettingsFile.toLatin1().constData(), "rt");
	while (hSettingsFile && !bLoadedSuccessfully)
	{
		// General
		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "GexFtpVersion", strValue)) == 0)
			m_dataGeneral.checkVersion(strValue);
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalEvery", strValue)) == 0)
			m_dataGeneral.setEvery(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalEvery_Hours", strValue)) == 0)
			m_dataGeneral.setEveryHour(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalEvery_Minutes", strValue)) == 0)
			m_dataGeneral.setEveryMinute(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalEvery_Seconds", strValue)) == 0)
			m_dataGeneral.setEverySecond(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalDaily", strValue)) == 0)
			m_dataGeneral.setDaily(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalDaily_Time", strValue)) == 0)
			m_dataGeneral.setDailyAt(QTime::fromString(strValue));
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalDaily_Everydays", strValue)) == 0)
			m_dataGeneral.setDailyEveryNDays(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalWeekly", strValue)) == 0)
			m_dataGeneral.setWeekly(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalWeekly_Time", strValue)) == 0)
			m_dataGeneral.setWeeklyAt(QTime::fromString(strValue));
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "SpoolIntervalWeekly_Weekday", strValue)) == 0)
			m_dataGeneral.setWeeklyOn(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "LogCleanupInterval", strValue)) == 0)
			m_nLogCleanupInterval = strValue.toInt();
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "DaysToKeepLogFiles", strValue)) == 0)
			m_dataGeneral.setNbDaysToKeepLog(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "FullLog", strValue)) == 0)
		{
			m_dataGeneral.setFullLog(strValue.toInt());
			m_nLogFilter = GEXFTP_MSG_ALL;
		}
		else if (iReadingStatus == 2)
			break;
		else
			m_nLogFilter = GEXFTP_MSG_ERROR;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "EnableFtpTimeout", strValue)) == 0)
			m_dataGeneral.setFtpTimeout(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "FtpTimeoutSeconds", strValue)) == 0)
			m_dataGeneral.setFtpTimeoutAfter(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "FileAgeBeforeTransfer_Min", strValue)) == 0)
			m_dataGeneral.setFileAgeBeforeTransfer_Min(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "EnableMaxFilePerProfile", strValue)) == 0)
			m_dataGeneral.setEnableMaxFilePerProfile(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "MaxFilePerProfile", strValue)) == 0)
			m_dataGeneral.setMaxFilePerProfile(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "MaxReconnectionAttempts", strValue)) == 0)
			m_dataGeneral.setMaxReconnectionAttempts(strValue.toInt());
		else if (iReadingStatus == 2)
			break;

                if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, "General", "CaseInsensitive", strValue)) == 0)
                {
                        m_dataGeneral.setCaseInsensitive(strValue.toInt());
                }
                else if (iReadingStatus == 2)
                        break;

	// Servers
		// Create section name: [Ftp:<settings name>]
		int		nSettingsNumber = 1;
		QString strSectionName = "Ftp:" + QString::number(nSettingsNumber++);


		// Show checking progress
		QProgressDialog progress("New Version detected: " + QString(QLatin1String(szAppFullName)), "Abort", 0, 0);
		progress.setCancelButton(0); // Hide button Abort
		progress.setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowMinimizeButtonHint);
		progress.setWindowTitle(QString(QLatin1String(szAppFullName)));
		progress.setWindowIcon(QIcon(":/gex-ftp/images/resources/galaxy_icon.png"));

		// Read as many sections as found in the file
		while ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "SettingsName", strValue)) == 0)
		{
			// Read all settings
			CGexFtpServer	clFtpServer;
			QString strErrorMessage = "";
			clFtpServer.setSettingsName(strValue,strErrorMessage);

            // Case 6962
            // New setting: use default value to behave as before if option not present,
            // and to make sure profile is not discarded because invalid.
            clFtpServer.setDownloadOrder(GEXFTP_DOWNLOAD_OLDEST_TO_NEWEST);

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "SiteURL", strValue)) == 0)
				clFtpServer.setFtpSiteURL(strValue,strErrorMessage);
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "PortNb", strValue)) == 0)
				clFtpServer.setFtpPortNb(strValue.toUInt());
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "Login", strValue)) == 0)
				clFtpServer.setLogin(strValue,strErrorMessage);
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "Password", strValue)) == 0)
			{
				QString strPassword;
				// Decrypt password
				DecryptPassword(strValue, strPassword);
				clFtpServer.setPassword(strPassword,strErrorMessage);
			}
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "RemoteDir", strValue)) == 0)
				clFtpServer.setRemoteDir(strValue,strErrorMessage);
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "RecursiveSearch", strValue)) == 0)
				clFtpServer.setRecursiveSearch(strValue.toInt());
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "LocalDir", strValue)) == 0)
				clFtpServer.setLocalDir(strValue,strErrorMessage);
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "FileExtensions", strValue)) == 0)
				clFtpServer.setFileExtensions(strValue,strErrorMessage);
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "FilePolicy", strValue)) == 0)
				clFtpServer.setFilePolicy(strValue);
			else if (iReadingStatus == 2)
				break;

            if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "DownloadOrder", strValue)) == 0)
                clFtpServer.setDownloadOrder(strValue);
            else if (iReadingStatus == 2)
                break;

            if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "ProfileEnabled", strValue)) == 0)
				clFtpServer.setProfileEnabled(strValue.toInt());
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "UseDateTimeWindow", strValue)) == 0)
				clFtpServer.setUseDateTimeWindow(strValue.toInt());
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "DisabledDateTimeCheck", strValue)) == 0)
				clFtpServer.setDisabledDateTimeCheck(strValue.toInt());
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "FromDate", strValue)) == 0)
				clFtpServer.setDateFrom(QDateTime::fromTime_t(strValue.toInt()));
			else if (iReadingStatus == 2)
				break;

			if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "ToDate", strValue)) == 0)
				clFtpServer.setDateTo(QDateTime::fromTime_t(strValue.toInt()));
			else if (iReadingStatus == 2)
				break;

			// Check if path to remote folder is OK
			if (m_dataGeneral.isNewVersion())
			{
				checkRemoteFolderValidity(clFtpServer);
				if (!m_bRunAsAService)
				{
					progress.setLabelText("New Version detected: " + QString(QLatin1String(szAppFullName))
										  + "\n Checking profile #" + QString::number(nSettingsNumber -1) + ": "
										  + clFtpServer.settingsName() + "...");
					progress.show();
				}
			}

			// All settings read, add to list of servers
			m_dataFtp.addProfile(clFtpServer, CGexFtpSettingsDataFtp::addFromFile);

			// Create next section name: [Ftp:<settings name>]
			strSectionName = "Ftp:" + QString::number(nSettingsNumber++);
		}

		// Fatal error: exit application
		if (iReadingStatus == 2)
		{
			// Close settings file
			fclose(hSettingsFile);
			return 2;
		}

		if ((iReadingStatus = readIniFile(hSettingsFile, strSettingsFile, strSectionName, "HomeDir", strValue)) == 0)
		{
			m_strServiceHomeDir			= strValue;
			m_strServiceSettingsFile	= m_strServiceHomeDir + "/.gex_ftp.conf";
			// Normalize paths
			CGexSystemUtils::NormalizePath(m_strServiceHomeDir);
			CGexSystemUtils::NormalizePath(m_strServiceSettingsFile);
		}

		// Close settings file
		fclose(hSettingsFile);
		// Fatal error exit application
		if (iReadingStatus == 2)
			return 2;

		bLoadedSuccessfully = true;
	}

	if (m_dataGeneral.isNewVersion())
			save();

	connect(&m_dataGeneral,	SIGNAL(sDataGeneralChanged()),	this, SLOT(onDataChanged()));
	connect(&m_dataFtp,		SIGNAL(sDataFtpChanged()),		this, SLOT(onDataChanged()));

	emit sSettingsLoaded(bLoadedSuccessfully, strSettingsFile);

	// Check status
	if(bLoadedSuccessfully == false)
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// Read ini file
/////////////////////////////////////////////////////////////////////////////////////
int CGexFtpSettings::readIniFile(FILE *hFile, QString strFile, QString strSection, QString strEntry, QString &strValue)
{
	int		iBufferSize = 1024;
	char	szValue[iBufferSize];
	int		iErrorStatus = 0;
	int		iLastCheckedLine = 0;

    int iReadingStatus =
        ut_ReadFromIniFile(hFile, strSection.toLatin1().constData(),
                           strEntry.toLatin1().constData(), szValue,
                           iBufferSize, &iLastCheckedLine, 0);
	strValue = szValue;

	if(iReadingStatus != UT_ERR_OKAY)
		iErrorStatus = manageReadError(strFile, strSection, strEntry, iReadingStatus, iLastCheckedLine, iBufferSize);

	return iErrorStatus;
}


/////////////////////////////////////////////////////////////////////////////////////
// Manage error: add to logs, message pop...
/////////////////////////////////////////////////////////////////////////////////////
int CGexFtpSettings::manageReadError(QString strFile, QString strSection, QString strEntry, int iErrorStatus, int iLastCheckedLine, int iBufferSize)
{
	int iStatus = 0;
	QString strErrorMessage = "Error while reading settings file " + strFile + ".\n\n";

	if (iErrorStatus == UT_ERR_OVERFLOW || iErrorStatus == UT_ERR_LINE_TRUNCATED)
	{
		iStatus = 2;
		strErrorMessage += "At line " + QString::number(iLastCheckedLine)
							+ ": maximum line length exceeded! "
							"Max. set to " + QString::number(iBufferSize - 1) +" characters.";
		// Show error message if not service mode
		if (!m_bRunAsAService)
		{
			QMessageBox qMBoxError;
			qMBoxError.setWindowIcon(QIcon(":/gex-ftp/images/resources/galaxy_icon.png"));
			qMBoxError.setWindowTitle("Fatal error!");
			qMBoxError.setText(strErrorMessage);
			qMBoxError.exec();
		}
		m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_ERROR, strErrorMessage);
	}
	else if(iErrorStatus == UT_ERR_NOSECTION)
	{
		iStatus = 1;
		// The process search if section exists, it's not an error
		if (!strSection.startsWith("Ftp:"))
		{
			strErrorMessage +="Unable to find section: " + strSection + ".";
			// Add error to logs
			m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_ERROR, strErrorMessage);
		}
	}
	else if (iErrorStatus == UT_ERR_NOENTRY)
	{
		iStatus = 1;
		strErrorMessage +="Unable to find entry: " + strEntry
						  + "in section: " + strSection + ".";
		// Add error to logs
		m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_ERROR, strErrorMessage);
	}


	return iStatus;
}

/////////////////////////////////////////////////////////////////////////////////////
// Check if it's possible to connect on Ftp server and go to remote directory
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::checkRemoteFolderValidity(CGexFtpServer &clFtpServer)
{
	GexFtpBrowseDialogArgs args;
	args.strCaption = "";
	args.strFtpSiteURL = clFtpServer.ftpSiteURL();
	args.iFtpPortNb = clFtpServer.ftpPortNb();
	args.strLogin = clFtpServer.login();
	args.strPassword = clFtpServer.password();
	args.iTimerInterval = m_dataGeneral.ftpTimeoutAfter() * 1000;
	args.bAllowCustomContextMenu = false;

	QString strDir = clFtpServer.remoteDir();
	QString strError = "";

	// If directory found
	if( GexFtpBrowseDialog::isValidFtpDirectory(strDir, args, strError))
	{
		// If directory found by modififed
		if (strDir != clFtpServer.remoteDir())
		{
			// LOG CAUTION DIR MODIFIED
			m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_WARNING, "Profile \"" + clFtpServer.settingsName() + "\" checked and updated: remote dir changed from \"" + clFtpServer.remoteDir() + "\" to \"" + strDir + "\".");
			clFtpServer.setRemoteDir(strDir, strError);
			if (!strError.isEmpty())
				m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_WARNING,strError);
		}
		else
		{
			// LOG DIR OK
			m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_INFO, "Profile \"" + clFtpServer.settingsName() + "\" checked and validated!");
		}
	}
	else
	{
		if (!strError.isEmpty())
			m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_WARNING,strError);
		// LOG DIR NOT FOUND
		m_mapErrTypeErrMsg.insertMulti(GEXFTP_MSG_ERROR, "Profile \"" + clFtpServer.settingsName() + "\" checked and disabled: an error has occured while testing profile settings. Please check the profile.");
		// Disable profile
		clFtpServer.setProfileEnabled(false);
	}
}


/////////////////////////////////////////////////////////////////////////////////////
// Set the default settings
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::reset()
{
	m_bDataChanged = false;

	disconnect(&m_dataGeneral,	SIGNAL(sDataGeneralChanged()),	0, 0);
	disconnect(&m_dataFtp,		SIGNAL(sDataFtpChanged()),		0, 0);

#ifdef unix
	m_strServiceHomeDir = GEX_LOCAL_UNIXFOLDER;
#endif

#ifdef _WIN32
	char	szString[2048];

	// Get Windows folder...
	GetWindowsDirectoryA(szString,2047);
	m_strServiceHomeDir  = szString;
#endif

	m_strServiceSettingsFile = m_strServiceHomeDir + "/.gex_ftp.conf";

	// Normalize paths
	CGexSystemUtils::NormalizePath(m_strServiceHomeDir);
	CGexSystemUtils::NormalizePath(m_strServiceSettingsFile);

	dataGeneral().reset();
	dataFtp().reset();

	m_nLogCleanupInterval	= 3600 * 100;
	m_nLogFilter			= GEXFTP_MSG_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////
// Received a signal that a settings has changed
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::onDataChanged()
{
	m_bDataChanged = true;

	if (dataGeneral().fullLog())
		m_nLogFilter = GEXFTP_MSG_ALL;
	else
		m_nLogFilter = GEXFTP_MSG_ERROR;

	emit sSettingsChanged();
}

/////////////////////////////////////////////////////////////////////////////////////
// Encrypt password
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::CryptPassword(const QString & strPassword, QString & strCryptedPasswordHexa)
{
	QString		strHexaChar;
	CGMemBuffer	clPasswordBuffer, clCryptedPasswordBuffer;
	CGCrypto	clCrypto(GEXFTP_CRYPTING_KEY);

    // If encryption fails, strCryptedPassword will contain the plain password
    strCryptedPasswordHexa = strPassword;

	// Encrypt password
	clPasswordBuffer.CreateBuffer(strPassword.length()+5);
	clPasswordBuffer.SetDataMode(CGMemBuffer::eModeDefault);
	if(clPasswordBuffer.WriteString(strPassword.toLatin1().data()) == false)				return;
	if(clCrypto.Encrypt(clPasswordBuffer, clCryptedPasswordBuffer) == false)	return;

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
}

/////////////////////////////////////////////////////////////////////////////////////
// Decrypt password
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::DecryptPassword(const QString & strCryptedPasswordHexa, QString & strPassword)
{
	CGMemBuffer		clPasswordBuffer, clCryptedPasswordBuffer;
	CGCrypto		clCrypto(GEXFTP_CRYPTING_KEY);
	const char		*pData;
	unsigned int	uiData;
	unsigned int	uiLengthHexa, uiLength;
	BYTE			*pCryptedBuffer;
	char			*pUncryptedBuffer;
	QByteArray		byteArray = strCryptedPasswordHexa.toLatin1();

	// Convert crypted password from hexa
	pData = byteArray.data();
	uiLengthHexa = strCryptedPasswordHexa.length();
	pCryptedBuffer = new BYTE[uiLengthHexa];

	for(uiLength = 0; uiLengthHexa > 0; uiLength++, uiLengthHexa-=2)
	{
		sscanf(pData, "%02X", &uiData);
		pCryptedBuffer[uiLength] = (char)uiData;
		pData += 2;
	}

	// Decrypt password
	clCryptedPasswordBuffer.CopyIn(pCryptedBuffer, uiLength, uiLength);
	clCrypto.Decrypt(clCryptedPasswordBuffer, clPasswordBuffer);
	clPasswordBuffer.SetDataMode(CGMemBuffer::eModeDefault);
	uiLength = clPasswordBuffer.GetBufferSize();
	pUncryptedBuffer = new char[uiLength];
	clPasswordBuffer.ReadString(pUncryptedBuffer, uiLength);
	strPassword = pUncryptedBuffer;

	delete [] pCryptedBuffer;
	delete [] pUncryptedBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////
// To signal that a profile is currently in change (to block start button)
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::beginProfileChange()
{
	emit sProfileInChange(true);
}

/////////////////////////////////////////////////////////////////////////////////////
// To signal that a profile is changed (to leave start button)
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::endProfileChange()
{
	emit sProfileInChange(false);
}


/////////////////////////////////////////////////////////////////////////////////////
// Set if the application is run as a service or not
/////////////////////////////////////////////////////////////////////////////////////
void CGexFtpSettings::setRunAsAService(const bool bRunAsAService)
{
	m_bRunAsAService = bRunAsAService;
}
