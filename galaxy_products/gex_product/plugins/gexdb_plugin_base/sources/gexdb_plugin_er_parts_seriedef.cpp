#include "gexdb_plugin_base.h"
#include <gqtl_log.h>

int GexDbPlugin_ER_Parts_SerieDef::s_numInstances=0;

GexDbPlugin_ER_Parts_SerieDef::GexDbPlugin_ER_Parts_SerieDef()
{
	s_numInstances++;
    GSLOG(SYSLOG_SEV_DEBUG, QString("GexDbPlugin_ER_Parts_SerieDef has now %1 instances ")
          .arg( s_numInstances).toLatin1().constData());

	m_strBinnings			= "PASS Bins only";
	m_bPlotSerie			= true;
	m_strChartingMode		= "Bars";
	m_nColor				= 0x00C8FF;
	m_strYieldExceptionRule = "Yield <= 70 %";
	m_strTableData			= "Yield & Volume";
	m_bWorkOnBinnings		= false;
	m_bParameterSerie		= false;

	// Defaults for new style variables (not present in the legacy serie definition string using the #ER# separators)
	m_strDataLabels			= "Top";
	m_strLineStyle			= "Line";
	m_strLineSpots			= "Square";
	m_strLineProperty		= "SolidLine";

	// Set yield limits variables
	m_uiYieldExceptionLimit_Type = 1;
	m_bYieldExceptionLimit_Strict = false;
	m_fYieldExceptionLimit = 70.0F;
}

GexDbPlugin_ER_Parts_SerieDef::GexDbPlugin_ER_Parts_SerieDef(QString & strSerieDefinition)
{
	s_numInstances++;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" now %1 instances ").arg( s_numInstances).toLatin1().constData());
	QString		strLimit;
	QRegExp		clRegExp("[<>=a-z%]");
	QStringList	strlSerieDefinition = strSerieDefinition.split("#ER#", QString::SkipEmptyParts);
	bool		bOK;

	m_strSerieName			= strlSerieDefinition[0];
	m_strBinnings			= strlSerieDefinition[1];
	m_bPlotSerie			= (strlSerieDefinition[2] == "1");
	m_strChartingMode		= strlSerieDefinition[3];
	m_nColor				= strlSerieDefinition[4].mid(2).toInt(&bOK, 16);	// Skip '0x' string so to read the RGB color
	m_strYieldExceptionRule = strlSerieDefinition[5];
	m_strTableData			= strlSerieDefinition[6];

	// Defaults for new style variables (not present in the legacy serie definition string using the #ER# separators)
	m_strDataLabels			= "Top";
	m_strLineStyle			= "Line";
	m_strLineSpots			= "Square";
	m_strLineProperty		= "SolidLine";

	// Set yield limits variables
	if(m_strYieldExceptionRule == "-")
		m_uiYieldExceptionLimit_Type = 0;
	else if(m_strYieldExceptionRule.indexOf("<") != -1)
		m_uiYieldExceptionLimit_Type = 1;
	else
		m_uiYieldExceptionLimit_Type = 2;
	if(m_strYieldExceptionRule.indexOf("=") != -1)
		m_bYieldExceptionLimit_Strict = false;
	else
		m_bYieldExceptionLimit_Strict = true;
	strLimit = m_strYieldExceptionRule.simplified().toLower();
	strLimit = strLimit.remove(" ");
	strLimit = strLimit.remove(clRegExp);
	m_fYieldExceptionLimit = strLimit.toFloat();
}

GexDbPlugin_ER_Parts_SerieDef::~GexDbPlugin_ER_Parts_SerieDef()
{	s_numInstances--;
}
