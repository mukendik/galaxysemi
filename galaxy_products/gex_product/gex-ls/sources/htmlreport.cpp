#include "htmlreport.h"
#include "gex-ls.h"
#include "gqtl_skin.h"


///////////////////////////////////////////////////////////////////////////////////
// Class CHtmlReport - class which is used to create the html report
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CHtmlReport::CHtmlReport(CGexSkin * pGexSkin)
{
	CGexSystemUtils::GetUserHomeDirectory(m_strPath);
	m_strPath	+= GEXLS_REPORT_DIR; 

	m_strFile	= GEXLS_REPORT_FILE;
	m_pGexSkin	= pGexSkin;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CHtmlReport::~CHtmlReport()
{

}

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::setPath(const QString& strPath)
{
	m_strPath = strPath;
}

void CHtmlReport::setFile(const QString& strFile)
{
	m_strFile = strFile;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	QString	normalizedFilePath() const
//
// Description	:	Built the normalized file path of the report
//
// Return		:	QString	-	The file path of the report 
///////////////////////////////////////////////////////////////////////////////////
QString CHtmlReport::normalizedFilePath() const
{
	QString	strFilePath = m_strPath + m_strFile;

	CGexSystemUtils::NormalizePath(strFilePath);

	return strFilePath;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	bool createReport(const CStatisticManager&  statisticManager)
//
// Description	:	Generate the report with the datas provided by StatisticManager.
//
// Param		:	statisticManager	[in]	Datas used to generate the report
//					
// Return		:	bool	-	true if generated, otherwise false 
///////////////////////////////////////////////////////////////////////////////////
bool CHtmlReport::createReport(const CStatisticManager&  statisticManager)
{
	bool		bCreation	= false;

	// Build path to configuration file
    QFile		file(normalizedFilePath()); 
    
	if (file.open(QIODevice::WriteOnly) == true)
	{
		// Assign file handle to data stream
		m_txtStream.setDevice(&file);	
		
		// Write header file
		writeHeader();

		// Write section Global
		writeChapter("Global Information");

		// Write global information
		writeGlobalInfo(statisticManager);

		// Write section Pareto
		writeChapter("License Usage Pareto");

		// write pareto table
		writeParetoInfo(statisticManager);
		
		// Write section Group computer
		writeChapter("Connection Statistics");

		// write group computer table
		writeGroupInfo("Per computer", "Computer", statisticManager.connectionGroupComputer());

		// write group user table
		writeGroupInfo("Per user", "User", statisticManager.connectionGroupUser());

		// Write footer file 
		writeFooter();

		// close the file handle
		file.close();

		bCreation = true;
	}

	return bCreation;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeHeader()
//
// Description	:	Write the header of the html file
//
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeHeader()
{
	m_txtStream << "<html>" << endl;
	m_txtStream << "<!-- ***************************************************************************-->" << endl;
	m_txtStream << "<!-- " << szAppFullName << "-->" << endl;
	m_txtStream << "<!-- " << C_STDF_COPYRIGHT << ".   www.mentor.com-->" << endl;
	m_txtStream << "<!-- Quantix Examinator - the STDF detective-->" << endl;
	m_txtStream << "<!-- All rights reserved. Users of the program must be Registered Users (see-->" << endl;
	m_txtStream << "<!-- the Quantix examinator license terms).-->" << endl;
	m_txtStream << "<!-- The Quantix Examinator program (including its HTML pages and .png files)-->" << endl;
	m_txtStream << "<!-- is protected by copyright law and international treaties. You are no-->" << endl;
	m_txtStream << "<!-- allowed to alter any HTML page, .png file, or any other part of the-->" << endl;
	m_txtStream << "<!-- software. Unauthorized reproduction or distribution of this program,-->" << endl;
	m_txtStream << "<!-- or any portion of it, may result in severe civil and criminal -->" << endl;
	m_txtStream << "<!-- penalties, and will be prosecuted to the maximum extent possible.-->" << endl;
	m_txtStream << "<!-- ***************************************************************************-->" << endl;
	m_txtStream << "<head>" << endl;
	m_txtStream << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">" << endl;

	QString strTitle = QString("Report created with: %1 - www.mentor.com").arg(szAppFullName);
	
	m_txtStream << "<title>" << strTitle << "</title>" << endl;

	// Add a css in order to get a left padding of 5 pixel for each column in table
	m_txtStream << "<style type=\"text/css\">td { padding-left: 5px; padding-right: 1px;}</style>" << endl;
	m_txtStream << "</head>" << endl;
	m_txtStream << "<body bgcolor=\"" << HTML_COLOR_BACKGROUND<< "\" text=\"" << HTML_COLOR_TEXT << "\">" << endl;		
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeFooter()
//
// Description	:	Write the footer of the html file
//
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeFooter()
{
	QString strFooter = QString(C_HTML_FOOTER).arg(szAppFullName);

	m_txtStream << strFooter << endl;
	m_txtStream << "</body>" << endl;
	m_txtStream << "</html>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeChapter(const QString& strTitle)
//
// Description	:	Write the chapter title
//
// Param		:	strTitle	[in]	Chapter title
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeChapter(const QString& strTitle)
{
	m_txtStream << "<h1 align=\"left\"><font color=" << m_pGexSkin->htmlSectionTextColor() << ">" << strTitle << "</font></h1><br>" << endl;
	m_txtStream << "<img border=\"0\" src=\"./ruler.png\" width=\"650\" height=\"8\">" << endl;
	m_txtStream << "<br>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeSection(const QString& strTitle)
//
// Description	:	Write the section title
//
// Param		:	strTitle	[in]	Section title
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeSection(const QString& strTitle)
{
	m_txtStream << "<p align=\"left\"><font color=" << m_pGexSkin->htmlSectionTextColor() << " size=\"4\"><b>" << strTitle << "</b><br></font></p>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeGlobalInfo(const CStatisticManager& statisticManager)
//
// Description	:	Write the global information of the statistic
//
// Param		:	statisticManager	[in]	Datas manager
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeGlobalInfo(const CStatisticManager& statisticManager)
{
	m_txtStream << "<table border=\"0\" width=\"650\" cellspacing=\"1\">" << endl;

	writeGlobalLine("Period begin",								statisticManager.dateTimeFrom().toString(Qt::TextDate));
	writeGlobalLine("Period end",								statisticManager.dateTimeTo().toString(Qt::TextDate));
	writeGlobalLine("Period duration",							convertTimeToString(statisticManager.period()));
	writeGlobalLine("Inactive period (no connections)",			convertTimeToString(statisticManager.paretoLicense().find(0).value()));
	writeGlobalLine("Active period (at least one connection)",	convertTimeToString(statisticManager.period() - statisticManager.paretoLicense().find(0).value()));
	writeGlobalLine("Total connection time",					convertTimeToString(statisticManager.connectionGroupAll().totalTime()));
	writeGlobalLine("Mean connection time",						convertTimeToString(statisticManager.connectionGroupAll().meanTime()));
	writeGlobalLine("Total connections",						QString::number(statisticManager.connectionGroupAll().count()));
	writeGlobalLine("Max license used",							QString::number(statisticManager.maxLicenseUsed()));
	
	m_txtStream << "</table><br>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeParetoInfo(const CStatisticManager& statisticManager)
//
// Description	:	Write the pareto of license
//
// Param		:	statisticManager	[in]	Datas manager
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeParetoInfo(const CStatisticManager& statisticManager)
{
	long lPeriod = statisticManager.period();

	// Write table for total period
	writeParetoTable("Over total period", 0, lPeriod, statisticManager);

	// Define period with at least on connection
	it_mapParetoLicenseMap	itParetoLicense = statisticManager.paretoLicense().find(0);

	if (itParetoLicense != statisticManager.paretoLicense().end())
		lPeriod -= itParetoLicense.value();

	// Write table for active period
	writeParetoTable("Over active period", 1, lPeriod, statisticManager);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeParetoTable(const QString& strSection, int nFirstNode, long lPeriod, const CStatisticManager& statisticManager)
//
// Description	:	Write table for license usage pareto 
//
// Param		:	strSection			[in]	Section title
//					nFirstNode			[in]	First node of pareto
//					lPeriod				[in]	Period for this pareto
//					statisticManager	[in]	Datas manager
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeParetoTable(const QString& strSection, int nFirstNode, long lPeriod, const CStatisticManager& statisticManager)
{
	it_mapParetoLicenseMap	itParetoLicense;

	writeSection(strSection);

	m_txtStream << "<table border=\"0\" width=\"90%\" cellspacing=\"1\">" << endl;

	writeParetoHeader("Connections", "Time", "Percent", "Chart");
	
	for (int nNode = nFirstNode; nNode <= statisticManager.maxLicenseUsed(); nNode++)
	{
		itParetoLicense = statisticManager.paretoLicense().find(nNode);

		// If data exists for this node, write a new line
		if (itParetoLicense != statisticManager.paretoLicense().end())
		{	
			float fPercent =  (float) ((float) itParetoLicense.value() / (float) lPeriod) * 100.0;

			writeParetoLine(QString::number(nNode), convertTimeToString(itParetoLicense.value()), fPercent);
		}
	}
	
	m_txtStream << "</table><br>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeGroupInfo(const QString& strSection, const QString& strLabel, const mapConnectionGroup& connectionGroup)
//
// Description	:	Write the connection information by group
//
// Param		:	strSection		[in]	Section label
//					strLabel		[in]	Column label
//					connectionGroup	[in]	Datas computer for grouped connections
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeGroupInfo(const QString& strSection, const QString& strLabel, const mapConnectionGroup& connectionGroup)
{
	writeSection(strSection);

	m_txtStream << "<table border=\"0\" width=\"90%\" cellspacing=\"1\">" << endl;

	writeGroupHeader(strLabel, "Total time", "Mean time", "Connections");

	mapConnectionGroup::ConstIterator itConnectionGroup = connectionGroup.constBegin();

	while (itConnectionGroup != connectionGroup.end())
	{
		writeGroupLine(itConnectionGroup.key(), convertTimeToString(itConnectionGroup.value().totalTime()),
								convertTimeToString(itConnectionGroup.value().meanTime()), QString::number(itConnectionGroup.value().count()));
		itConnectionGroup++;
	}
			
	m_txtStream << "</table><br>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeParetoHeader(const QString& strLabel, const QString& strTime, const QString& strPercent, const QString& strChart)
//
// Description	:	Write the header of Pareto table
//
// Param		:	strLabel		[in]	Header of the column label
//					strTime			[in]	Header of the column time
//					strPercent		[in]	Header of the column percentage
//					strChart		[in]	Header of the column chart
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeParetoHeader(const QString& strLabel, const QString& strTime, const QString& strPercent, const QString& strChart)
{
	m_txtStream << "<tr>" << endl;

	m_txtStream << "<td width=\"15%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"center\"><b>"	<< strLabel		<< "</b></td>" << endl;
	m_txtStream << "<td width=\"40%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"left\"><b>"		<< strTime		<< "</b></td>" << endl;
	m_txtStream << "<td width=\"15%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"center\"><b>"	<< strPercent	<< "</b></td>" << endl;
	m_txtStream << "<td width=\"30%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"center\"><b>"	<< strChart		<< "</b></td>" << endl;
	
	m_txtStream << "</tr>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeGroupHeader(const QString& strLabel, const QString& strTime, const QString& strMean, const QString& strNbConnection)
//
// Description	:	Write the header of Group table
//
// Param		:	strLabel		[in]	Header of the column label
//					strTime			[in]	Header of the column time
//					strMean			[in]	Header of the column mean time
//					strNbConnection	[in]	Header of the column for connection number
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeGroupHeader(const QString& strLabel, const QString& strTime, const QString& strMean, const QString& strNbConnection)
{
	m_txtStream << "<tr>" << endl;

	m_txtStream << "<td width=\"20%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"left\"><b>"		<< strLabel			<< "</b></td>" << endl;
	m_txtStream << "<td width=\"40%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"left\"><b>"		<< strTime			<< "</b></td>" << endl;
	m_txtStream << "<td width=\"20%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"left\"><b>"		<< strMean			<< "</b></td>" << endl;
	m_txtStream << "<td width=\"20%\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << " align=\"center\"><b>"	<< strNbConnection	<< "</b></td>" << endl;
		
	m_txtStream << "</tr>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeGlobalLine(const QString& strLabel, const QString& strData)
//
// Description	:	Write a line for Global Information
//
// Param		:	strLabel		[in]	Label of the data information
//					strData			[in]	Data information
//					
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeGlobalLine(const QString& strLabel, const QString& strData)
{
	m_txtStream << "<tr>" << endl;

	if (strLabel.isEmpty())
	{
		m_txtStream << "<td width=\"275\" height=\"21\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << "><b><HR></b></td>" << endl;
		m_txtStream << "<td width=\"375\" height=\"21\" bgcolor=><HR></td>" << endl;
	}
	else
	{
		m_txtStream << "<td width=\"275\" height=\"21\" bgcolor= " << m_pGexSkin->htmlLabelBackgroundColor() << "><b>" << strLabel << "</b></td>" << endl;

		if (strData.isEmpty())
			m_txtStream << "<td width=\"375\" height=\"21\" bgcolor= " << m_pGexSkin->htmlDataBackgroundColor() << ">n/a</td>" << endl;
		else
			m_txtStream << "<td width=\"375\" height=\"21\" bgcolor= " << m_pGexSkin->htmlDataBackgroundColor() << ">" << strData << "</td>" << endl;		
	}
		
	m_txtStream << "</tr>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeParetoLine(const QString& strLabel, const QString& strTime, float fPercent)
//
// Description	:	Write a line for Pareto
//
// Param		:	strLabel		[in]	Label of the pareto line
//					strTime			[in]	Duration
//					fPercent		[in]	Percentage of use
//
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeParetoLine(const QString& strLabel, const QString& strTime, float fPercent)
{
	m_txtStream << "<tr>" << endl;

	m_txtStream << "<td width=\"15%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"center\"><b>"	<< strLabel								<< "</b></td>"	<< endl;
	m_txtStream << "<td width=\"40%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"left\">"		<< strTime								<< "</td>"		<< endl;
	m_txtStream << "<td width=\"15%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"center\">"	<< QString::number(fPercent, 'f', 2)	<< "</td>"		<< endl;

	if (fPercent < 1)
		fPercent = 1;
	
	m_txtStream << "<td width=\"30%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"left\"><img border=\"1\" src=\"./bar1.png\" width=\"" << fPercent * 2 << "\" height=\"10\"></td>" << endl;
	
	m_txtStream << "</tr>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void writeGroupLine(const QString& strLabel, const QString& strTime, const QString& strMean, const QString& strNbConnection)
//
// Description	:	Write a line for Group
//
// Param		:	strLabel		[in]	Label of the group line
//					strTime			[in]	Total time
//					strMean			[in]	Mean time
//					strNbConnection	[in]	Connection number
//
///////////////////////////////////////////////////////////////////////////////////
void CHtmlReport::writeGroupLine(const QString& strLabel, const QString& strTime, const QString& strMean, const QString& strNbConnection)
{
	m_txtStream << "<tr>" << endl;

	m_txtStream << "<td width=\"20%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"left\"> <b>"	<< strLabel			<< "</b></td>"	<< endl;
	m_txtStream << "<td width=\"40%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"left\">"		<< strTime			<< "</td>"		<< endl;
	m_txtStream << "<td width=\"20%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"left\">"		<< strMean			<< "</td>"		<< endl;
	m_txtStream << "<td width=\"20%\" bgcolor=" << m_pGexSkin->htmlDataBackgroundColor() << " align=\"center\">"	<< strNbConnection	<< "</td>"		<< endl;
		
	m_txtStream << "</tr>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	QString convertTimeToString(long lElapsedTime)
//
// Description	:	Convert time in seconds to string (ex: 3700 secs <=> 1 Hour 1 min 40 secs)
//
// Param		:	lElapsedTime	[in]	Elapsed time to convert (in seconds)
//
// Return		:	QString		-	Formatted string
///////////////////////////////////////////////////////////////////////////////////
QString CHtmlReport::convertTimeToString(long lElapsedTime)
{
	QString strTime;

	long lDays		= lElapsedTime / 86400;
	lElapsedTime	= lElapsedTime % 86400;
	long lHours		= lElapsedTime / 3600;
	lElapsedTime	= lElapsedTime % 3600;
	long lMins		= lElapsedTime / 60;
	lElapsedTime	= lElapsedTime % 60;
	long lSecs		= lElapsedTime;

	// If time lesser than one day, don't display the number of day
	if (lDays)
	{
		// Manage the 's' at the end 
		if (lDays > 1)
			strTime += QString::number(lDays) + " Days ";
		else
			strTime += QString::number(lDays) + " Day ";
	}

	// If lHours lesser than one hour, don't display the number of hour except if the time is greater than one day
	if (lHours || !strTime.isEmpty())
	{
		// Manage the 's' at the end 
		if (lHours > 1)
			strTime += QString::number(lHours) + " Hours ";
		else
			strTime += QString::number(lHours) + " Hour ";
	}

	// If lMins lesser than one minute, don't display the number of minutes except if the time is greater than one hour
	if (lMins || !strTime.isEmpty())
	{
		// Manage the 's' at the end 
		if (lMins > 1)
			strTime += QString::number(lMins) + " Mins ";
		else
			strTime += QString::number(lMins) + " Min ";
	}
	
	// Always display the seconds
	// Manage the 's' at the end 
	if (lSecs > 1)
		strTime += QString::number(lSecs) + " Secs ";
	else
		strTime += QString::number(lSecs) + " Sec ";

	return strTime;
}
