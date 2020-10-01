#ifndef GEX_HTML_REPORT_H
#define GEX_HTML_REPORT_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-LS Includes
///////////////////////////////////////////////////////////////////////////////////
#include "statisticmanager.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QFile>
#include <QTextStream>
#include <gqtl_sysutils.h>

class CGexSkin;

extern char *szAppFullName;

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CHtmlReport

// Description	:	Generate the Html report
//
///////////////////////////////////////////////////////////////////////////////////
class CHtmlReport
{

public:
	
	CHtmlReport(CGexSkin * pGexSkin);
	~CHtmlReport();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
	
	const QString&		path() const				{ return m_strPath; }
	const QString&		file() const				{ return m_strFile; }
	
	void				setPath(const QString& strPath);
	void				setFile(const QString& strFile);
	
///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	bool				createReport(const CStatisticManager&  statisticManager);		// Generate the report with the datas provided by StatisticManager
	QString				normalizedFilePath() const;										// Built the normalized file path of the report

private:

	QString				convertTimeToString(long lElapsedTime);							// Convert time in seconds to string (ex: 3700 secs <=> 1 Hour 1 min 40 secs)
	
	void				writeHeader();													// Write the header of the html file
	void				writeChapter(const QString& strTitle);							// Write the chapter title
	void				writeSection(const QString& strTitle);							// Write the section title
	void				writeFooter();													// Write the footer of the html file
	void				writeGlobalInfo(const CStatisticManager& statisticManager);		// Write the global information of the statistic
	void				writeParetoInfo(const CStatisticManager& statisticManager);		// Write the pareto of license
	void				writeGroupInfo(const QString& strSection, const QString& strLabel, const mapConnectionGroup& connectionGroup);				// Write the connection information by group
	void				writeParetoHeader(const QString& strLabel, const QString& strTime, const QString& strPercent, const QString& strChart);		// Write the header of Pareto table
	void				writeGroupHeader(const QString& strLabel, const QString& strTime, const QString& strMean, const QString& strNbConnection);	// Write the header of Group table
	void				writeGlobalLine(const QString& strLabel, const QString& strInfo);															// Write a line for Global Information
	void				writeParetoLine(const QString& strLabel, const QString& strTime, float fPercent);											// Write a line for Pareto
	void				writeGroupLine(const QString& strLabel, const QString& strTime, const QString& strMean, const QString& strNbConnection);	// Write a line for Group
	void				writeParetoTable(const QString& strSection, int nFirstNode, long lPeriod, const CStatisticManager& statisticManager);		// Write table for license usage pareto 

	QString				m_strPath;				// default directory is <userhome>/gexreport
	QString				m_strFile;				// default file is constant GEXLS_REPORT_FILE
	QTextStream			m_txtStream;			// streamer for the report file
	
	CGexSkin * 			m_pGexSkin;				// skin parameter
};

#endif // GEX_HTML_REPORT_H
