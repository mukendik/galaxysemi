///////////////////////////////////////////////////////////
// Plug-in Base. 
// Interface layer between Examinator and 
// third-party applications
///////////////////////////////////////////////////////////
// Standard includes
#include <qdatetime.h>

// QT includes
#include <qcheckbox.h>

// Local includes
#include "plugin_base.h"
#include "plugin_eventdump_dialog.h"

///////////////////////////////////////////////////////////
// Class for display of event contents (for DEBUG)
///////////////////////////////////////////////////////////
GP_EventDebug::GP_EventDebug()	// Default constructor
{
	Reset();
}

void GP_EventDebug::Reset()		// Reset all flags
{
	m_bDisplay_AllEvents = true;
	m_bDiscard_AllEvents = false;
	for(int i=0 ; i<=PLUGIN_EVENTID_MAX ; i++)
	{
		m_bDisplay_SingleEvent[i] = true;
		m_bDiscard_SingleEvent[i] = false;
	}
}

void GP_EventDebug::NewLot()	// New lot event received
{
	m_bDisplay_AllEvents = !m_bDiscard_AllEvents;
	for(int i=0 ; i<=PLUGIN_EVENTID_MAX ; i++)
	{
		m_bDisplay_SingleEvent[i] = !m_bDiscard_SingleEvent[i];
	}
}

void GP_EventDebug::DisplayEvent(int nEventID, const QString & strEventName, const QString & strEventDump)	// Display event content
{
	if(m_bDisplay_AllEvents && m_bDisplay_SingleEvent[nEventID])
	{
		PluginEventDumpDialog dlg(strEventName, strEventDump);
		dlg.exec();
		if(dlg.checkBoxDiscardEvent_Lot->isChecked())
			m_bDisplay_SingleEvent[nEventID] = false;
		if(dlg.checkBoxDiscardEvent_Dataset->isChecked())
		{
			m_bDisplay_SingleEvent[nEventID] = false;
			m_bDiscard_SingleEvent[nEventID] = true;
		}
		if(dlg.checkBoxDiscardAllEvents_Lot->isChecked())
			m_bDisplay_AllEvents = false;
		if(dlg.checkBoxDiscardAllEvents_Dataset->isChecked())
		{
			m_bDisplay_AllEvents = false;
			m_bDiscard_AllEvents = true;
		}
	}
}

///////////////////////////////////////////////////////////
// Data containers
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Plugin Info Details
///////////////////////////////////////////////////////////
GP_PluginDef::GP_PluginDef()	// Default constructor
{
	m_lPluginModuleID = -1;
	m_strPluginName = "";
}

GP_PluginDef::GP_PluginDef( const GP_PluginDef *source )  // copy constructor
{
	m_lPluginModuleID = source->m_lPluginModuleID;
	m_strPluginName = source->m_strPluginName;
}

GP_PluginDef& GP_PluginDef::operator=( const GP_PluginDef &s )	// assignment operator
{
	m_lPluginModuleID = s.m_lPluginModuleID;
	m_strPluginName = s.m_strPluginName;

	 return *this; 
}

///////////////////////////////////////////////////////////
// Interactive command Definition
///////////////////////////////////////////////////////////
GP_InteractiveCmd::GP_InteractiveCmd()	// Default constructor
{
	m_uiSiteIndex				= 0;
	m_nSiteNb					= 0;
	m_uiParameterIndex			= 0;
	m_uiParameterNumber			= 0;
	m_strParameterName			= "";
	m_nChartType				= 0;
	m_nAdvancedReportSettings	= 0;
}

GP_InteractiveCmd::GP_InteractiveCmd( const GP_InteractiveCmd *source )  // copy constructor
{
	m_uiSiteIndex = source->m_uiSiteIndex;
	m_nSiteNb = source->m_nSiteNb;
	m_uiParameterIndex = source->m_uiParameterIndex;
	m_uiParameterNumber = source->m_uiParameterNumber;
	m_strParameterName = source->m_strParameterName;
	m_nChartType = source->m_nChartType;
	m_nAdvancedReportSettings = source->m_nAdvancedReportSettings;
}

GP_InteractiveCmd& GP_InteractiveCmd::operator=( const GP_InteractiveCmd &s )	// assignment operator
{
	m_uiSiteIndex = s.m_uiSiteIndex;
	m_nSiteNb = s.m_nSiteNb;
	m_uiParameterIndex = s.m_uiParameterIndex;
	m_uiParameterNumber = s.m_uiParameterNumber;
	m_strParameterName = s.m_strParameterName;
	m_nChartType = s.m_nChartType;
	m_nAdvancedReportSettings = s.m_nAdvancedReportSettings;

	return *this; 
}

void GP_InteractiveCmd::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "GP_InteractiveCommand";
	// Site index
	strFields += ";SiteIndex=";
	strFields += QString::number(m_uiSiteIndex);
	// Site number
	strFields += ";SiteNumber=";
	strFields += QString::number(m_nSiteNb);
	// Parameter index
	strFields += ";ParameterIndex=";
	strFields += QString::number(m_uiParameterIndex);
	// Parameter number
	strFields += ";ParameterNumber=";
	strFields += QString::number(m_uiParameterNumber);
	// Parameter name
	strFields += ";ParameterName=";
	strFields += m_strParameterName;
	// Chart type
	strFields += ";ChartType=";
	strFields += QString::number(m_nChartType);
	// Advanced report settings
	strFields += ";AdvancedReportSettings=";
	strFields += QString::number(m_nAdvancedReportSettings);

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Dataset definition
///////////////////////////////////////////////////////////
GP_DatasetDef::GP_DatasetDef()	// Default constructor
{
	m_lGroupID = 0;
	m_strGroupName = "";
	m_lTotalParameters = 0;
	m_lTotalBatches = 0;
}

GP_DatasetDef::GP_DatasetDef( const GP_DatasetDef *source )  // copy constructor
{
	m_lGroupID = source->m_lGroupID;
	m_strGroupName = source->m_strGroupName;
	m_lTotalParameters = source->m_lTotalParameters;
	m_lTotalBatches = source->m_lTotalBatches;
}

GP_DatasetDef& GP_DatasetDef::operator=( const GP_DatasetDef &s )	// assignment operator
{
	m_lGroupID = s.m_lGroupID;
	m_strGroupName = s.m_strGroupName;
	m_lTotalParameters = s.m_lTotalParameters;
	m_lTotalBatches = s.m_lTotalBatches;

	return *this; 
}

void GP_DatasetDef::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "GP_DatasetDef";
	// GroupName
	strFields += ";GroupName=";
	strFields += m_strGroupName;
	// GroupID
	strFields += ";GroupID=";
	strFields += QString::number(m_lGroupID);
	// Parameters
	strFields += ";Parameters=";
	strFields += QString::number(m_lTotalParameters);
	// Sublots
	strFields += ";Batches=";
	strFields += QString::number(m_lTotalBatches);

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Parameter definition
///////////////////////////////////////////////////////////
GP_ParameterDef::GP_ParameterDef()	// Default constructor
{
	ResetData();
}

void GP_ParameterDef::ResetData()		// Reset object data
{
	m_nParameterIndex = -1;
	m_nParameterNumber = -1;
	m_strName = "";
	m_strUnits = "";
	m_lfLowL = 0.0;
	m_lfHighL = 0.0;	
	m_bTestType = ' ';
	m_bHasLowL = false;
	m_bHasHighL = false;
	m_bStrictLowL = true;
	m_bStrictHighL = true;
	m_nLowLScaleFactor = 0;
	m_nHighLScaleFactor = 0;
	m_nResultScaleFactor = 0;
}

GP_ParameterDef::GP_ParameterDef( const GP_ParameterDef *source )  // copy constructor
{
	m_nParameterIndex = source->m_nParameterIndex;
	m_nParameterNumber = source->m_nParameterNumber;
	m_strName = source->m_strName;
	m_strUnits = source->m_strUnits;
	m_lfLowL = source->m_lfLowL;
	m_lfHighL = source->m_lfHighL;
	m_bTestType = source->m_bTestType;
	m_bHasLowL = source->m_bHasLowL;
	m_bHasHighL = source->m_bHasHighL;
	m_bStrictLowL = source->m_bStrictLowL;
	m_bStrictHighL = source->m_bStrictHighL;
	m_nLowLScaleFactor = source->m_nLowLScaleFactor;
	m_nHighLScaleFactor = source->m_nHighLScaleFactor;
	m_nResultScaleFactor = source->m_nResultScaleFactor;
}

GP_ParameterDef& GP_ParameterDef::operator=( const GP_ParameterDef &s )	// assignment operator
{
	m_nParameterIndex = s.m_nParameterIndex;
	m_nParameterNumber = s.m_nParameterNumber;
	m_strName = s.m_strName;
	m_strUnits = s.m_strUnits;
	m_lfLowL = s.m_lfLowL;
	m_lfHighL = s.m_lfHighL;
	m_bTestType = s.m_bTestType;
	m_bHasLowL = s.m_bHasLowL;
	m_bHasHighL = s.m_bHasHighL;
	m_bStrictLowL = s.m_bStrictLowL;
	m_bStrictHighL = s.m_bStrictHighL;
	m_nLowLScaleFactor = s.m_nLowLScaleFactor;
	m_nHighLScaleFactor = s.m_nHighLScaleFactor;
	m_nResultScaleFactor = s.m_nResultScaleFactor;

	return *this; 
}

void GP_ParameterDef::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "GP_ParameterDef";
	// ParameterIndex
	strFields += ";Index=";
	strFields += QString::number(m_nParameterIndex);
	// ParameterNumber
	strFields += ";Number=";
	strFields += QString::number(m_nParameterNumber);
	// Name
	strFields += ";Name=";
	strFields += m_strName;
	// Units
	strFields += ";Units=";
	strFields += m_strUnits;
	// Type
	strFields += ";Type=";
	QChar clChar((char)m_bTestType);
	strFields += clChar;
	// LL
	strFields += ";Has LL=";
	if(m_bHasLowL)
		strFields += "Y";
	else
		strFields += "N";
	strFields += ";Strict LL=";
	if(m_bStrictLowL)
		strFields += "Y";
	else
		strFields += "N";
	strFields += ";LL=";
	strFields += QString::number(m_lfLowL);
	strFields += ";LL ScaleFactor=";
	strFields += QString::number(m_nLowLScaleFactor);
	// HL
	strFields += ";Has HL=";
	if(m_bHasHighL)
		strFields += "Y";
	else
		strFields += "N";
	strFields += ";Strict HL=";
	if(m_bStrictHighL)
		strFields += "Y";
	else
		strFields += "N";
	strFields += ";HL=";
	strFields += QString::number(m_lfHighL);
	strFields += ";HL ScaleFactor=";
	strFields += QString::number(m_nHighLScaleFactor);
	// Result Scale factor
	strFields += ";Result ScaleFactor=";
	strFields += QString::number(m_nResultScaleFactor);

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Parameter statistics
///////////////////////////////////////////////////////////
GP_ParameterStats::GP_ParameterStats()	// Default constructor
{
	ResetData();
}

void GP_ParameterStats::ResetData()	// Reset object data
{
	m_uiNbExecs = 0;
	m_uiNbOutliers = 0;
	m_uiNbFails = 0;
	m_lfSumX = 0.0F;
	m_lfSumX2 = 0.0F;
	m_fMin = 0.0F;
	m_fMax = 0.0F;
	m_fMean = 0.0F;
	m_fSigma = 0.0F;
	m_fCp = 0.0F;
	m_fCpk = 0.0F;
	m_fRange = 0.0F;
}

GP_ParameterStats::GP_ParameterStats( const GP_ParameterStats *source )  // copy constructor
{
	m_uiNbExecs = source->m_uiNbExecs;
	m_uiNbOutliers = source->m_uiNbOutliers;
	m_uiNbFails = source->m_uiNbFails;
	m_lfSumX = source->m_lfSumX;
	m_lfSumX2 = source->m_lfSumX2;
	m_fMin = source->m_fMin;
	m_fMax = source->m_fMax;
	m_fMean = source->m_fMean;
	m_fSigma = source->m_fSigma;
	m_fCp = source->m_fCp;
	m_fCpk = source->m_fCpk;
	m_fRange = source->m_fRange;
}

GP_ParameterStats& GP_ParameterStats::operator=( const GP_ParameterStats &s )	// assignment operator
{
	m_uiNbExecs = s.m_uiNbExecs;
	m_uiNbOutliers = s.m_uiNbOutliers;
	m_lfSumX = s.m_lfSumX;
	m_lfSumX2 = s.m_lfSumX;
	m_uiNbFails = s.m_uiNbFails;
	m_fMin = s.m_fMin;
	m_fMax = s.m_fMax;
	m_fMean = s.m_fMean;
	m_fSigma = s.m_fSigma;
	m_fCp = s.m_fCp;
	m_fCpk = s.m_fCpk;
	m_fRange = s.m_fRange;

	return *this; 
}

void GP_ParameterStats::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "GP_ParameterStats";
	// Nb Execs
	strFields += ";Nb executions=";
	strFields += QString::number(m_uiNbExecs);
	// Nb outliers
	strFields += ";Nb outliers=";
	strFields += QString::number(m_uiNbOutliers);
	// Nb fails
	strFields += ";Nb fails=";
	strFields += QString::number(m_uiNbFails);
	// SumX
	strFields += ";SumX=";
	strFields += QString::number(m_lfSumX);
	// SumX2
	strFields += ";SumX2=";
	strFields += QString::number(m_lfSumX2);
	// Min
	strFields += ";Min=";
	strFields += QString::number(m_fMin);
	// Max
	strFields += ";Max=";
	strFields += QString::number(m_fMax);
	// Range
	strFields += ";Range=";
	strFields += QString::number(m_fRange);
	// Mean
	strFields += ";Mean=";
	strFields += QString::number(m_fMean);
	// Sigma
	strFields += ";Sigma=";
	strFields += QString::number(m_fSigma);
	// Cp
	strFields += ";Cp=";
	strFields += QString::number(m_fCp);
	// Cpk
	strFields += ";Cpk=";
	strFields += QString::number(m_fCpk);

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Site description information (read from SDR record)
///////////////////////////////////////////////////////////
GP_SiteDescription::GP_SiteDescription()
{
	m_nHeadNum				= -1;	// -1 until an SDR is found
	m_nSiteNum				= -1;	// -1 until an SDR is found
	m_strHandlerProberID	= "";
	m_strProbeCardID		= "";
	m_strLoadBoardID		= "";
	m_strDibBoardID			= "";
	m_strInterfaceCableID	= "";
	m_strHandlerContactorID	= "";
	m_strLaserID			= "";
	m_strExtraEquipmentID	= "";
	m_strHandlerType		= "";
	m_strProbeCardType		= "";
	m_strLoadBoardType		= "";

}

GP_SiteDescription::~GP_SiteDescription()
{
}

GP_SiteDescription& GP_SiteDescription::operator+=(const GP_SiteDescription& aSite)
{
	m_strHandlerProberID += aSite.m_strHandlerProberID;
	m_strProbeCardID += aSite.m_strProbeCardID;
	m_strLoadBoardID += aSite.m_strLoadBoardID;
	m_strDibBoardID += aSite.m_strDibBoardID;
	m_strInterfaceCableID += aSite.m_strInterfaceCableID;
	m_strHandlerContactorID += aSite.m_strHandlerContactorID;
	m_strLaserID += aSite.m_strLaserID;
	m_strExtraEquipmentID += aSite.m_strExtraEquipmentID;

	m_strHandlerType += aSite.m_strHandlerType;
	m_strProbeCardType += aSite.m_strProbeCardType;
	m_strLoadBoardType += aSite.m_strLoadBoardType;

	return *this;
}

GP_SiteDescription& GP_SiteDescription::operator=(const GP_SiteDescription& aSite)
{
	m_nHeadNum = aSite.m_nHeadNum;
	m_nSiteNum = aSite.m_nSiteNum;
	m_strHandlerProberID = aSite.m_strHandlerProberID;
	m_strProbeCardID = aSite.m_strProbeCardID;
	m_strLoadBoardID = aSite.m_strLoadBoardID;
	m_strDibBoardID = aSite.m_strDibBoardID;
	m_strInterfaceCableID = aSite.m_strInterfaceCableID;
	m_strHandlerContactorID = aSite.m_strHandlerContactorID;
	m_strLaserID = aSite.m_strLaserID;
	m_strExtraEquipmentID = aSite.m_strExtraEquipmentID;

	m_strHandlerType = aSite.m_strHandlerType;
	m_strProbeCardType = aSite.m_strProbeCardType;
	m_strLoadBoardType = aSite.m_strLoadBoardType;

	return *this;
}

void GP_SiteDescription::GetDump(QString & strFields) const
{
	QString strPartsPerSite;

	// Object name
	strFields = "OBJECT=";
	strFields += "GP_SiteDescription";
	// Head number
	strFields += ";Head number=";
	strFields += QString::number(m_nHeadNum);
	// Site number
	strFields += ";Site number=";
	strFields += QString::number(m_nSiteNum);
	// Handler/Prober ID
	strFields += ";Handler/Prober ID=";
	strFields += m_strHandlerProberID;
	// ProberCard ID
	strFields += ";ProberCard ID=";
	strFields += m_strProbeCardID;
	// LoadBoard ID
	strFields += ";LoadBoard ID=";
	strFields += m_strLoadBoardID;
	// DibBoard ID
	strFields += ";DibBoard ID=";
	strFields += m_strDibBoardID;
	// InterfaceCable ID
	strFields += ";InterfaceCable ID=";
	strFields += m_strInterfaceCableID;
	// HandlerContactor ID
	strFields += ";HandlerContactor ID=";
	strFields += m_strHandlerContactorID;
	// Laser ID
	strFields += ";Laser ID=";
	strFields += m_strLaserID;
	// ExtraEquipment ID
	strFields += ";ExtraEquipment ID=";
	strFields += m_strExtraEquipmentID;

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Lot definition
///////////////////////////////////////////////////////////
GP_LotDef::GP_LotDef()	// Default constructor
{
	m_strProductID = "";
	m_strDatasetOrigin = "";
	m_strLotID = "";
	m_strSubLotID = "";
	m_strWaferID = "";
	m_strTesterName = "";
	m_strTesterType = "";
	m_strOperatorName = "";
	m_strJobName = "";
	m_strJobRevision = "";
	m_strExecType = "";
	m_strExecVersion = "";
	m_strTestCode = "";
	m_strTestTemperature = "";
	m_lStartTime = 0;
	m_lFinishTime = 0;
	m_lTotalParts = 0;
	m_uiProgramRuns = 0;
	m_puiPartsPerSite = NULL;
}

GP_LotDef::GP_LotDef( const GP_LotDef *source )  // copy constructor
{
	m_strProductID = source->m_strProductID;
	m_strDatasetOrigin = source->m_strDatasetOrigin;
	m_strLotID = source->m_strLotID;
	m_strSubLotID = source->m_strSubLotID;
	m_strWaferID = source->m_strWaferID;
	m_strTesterName = source->m_strTesterName;
	m_strTesterType = source->m_strTesterType;
	m_strOperatorName = source->m_strOperatorName;
	m_strJobName = source->m_strJobName;
	m_strJobRevision = source->m_strJobRevision;
	m_strExecType = source->m_strExecType;
	m_strExecVersion = source->m_strExecVersion;
	m_strTestCode = source->m_strTestCode;
	m_strTestTemperature = source->m_strTestTemperature;
	m_lStartTime = source->m_lStartTime;
	m_lFinishTime = source->m_lFinishTime;
	m_lTotalParts = source->m_lTotalParts;
	m_uiProgramRuns = source->m_uiProgramRuns;
	m_puiPartsPerSite = source->m_puiPartsPerSite;
}

GP_LotDef& GP_LotDef::operator=( const GP_LotDef &s )  // assignment operator
{
	m_strProductID = s.m_strProductID;
	m_strDatasetOrigin = s.m_strDatasetOrigin;
	m_strLotID = s.m_strLotID;
	m_strSubLotID = s.m_strSubLotID;
	m_strWaferID = s.m_strWaferID;
	m_strTesterName = s.m_strTesterName;
	m_strTesterType = s.m_strTesterType;
	m_strOperatorName = s.m_strOperatorName;
	m_strJobName = s.m_strJobName;
	m_strJobRevision = s.m_strJobRevision;
	m_strExecType = s.m_strExecType;
	m_strExecVersion = s.m_strExecVersion;
	m_strTestCode = s.m_strTestCode;
	m_strTestTemperature = s.m_strTestTemperature;
	m_lStartTime = s.m_lStartTime;
	m_lFinishTime = s.m_lFinishTime;
	m_lTotalParts = s.m_lTotalParts;
	m_uiProgramRuns = s.m_uiProgramRuns;
	m_puiPartsPerSite = s.m_puiPartsPerSite;

	return *this; 
}

void GP_LotDef::GetDump(QString & strFields) const
{
	QString		strPartsPerSite;
	QDateTime	clDateTime;

	// Object name
	strFields = "OBJECT=";
	strFields += "GP_LotDef";
	// ProductID
	strFields += ";ProductID=";
	strFields += m_strProductID;
	// Origin
	strFields += ";Origin=";
	strFields += m_strDatasetOrigin;
	// Start time
	strFields += ";Start time=";
	strFields += QString::number(m_lStartTime);
	clDateTime.setTime_t(m_lStartTime);
	strFields += " (" + clDateTime.toString("dd MMM yyyy hh:mm:ss");
	strFields += ")";
	// Finish time
	strFields += ";Finish time=";
	strFields += QString::number(m_lFinishTime);
	clDateTime.setTime_t(m_lFinishTime);
	strFields += " (" + clDateTime.toString("dd MMM yyyy hh:mm:ss");
	strFields += ")";
	// Parts
	strFields += ";Parts=";
	strFields += QString::number(m_lTotalParts);
	// Program Runs
	strFields += ";Program Runs=";
	strFields += QString::number(m_uiProgramRuns);
	// LotID
	strFields += ";LotID=";
	strFields += m_strLotID;
	// SublotID
	strFields += ";SublotID=";
	strFields += m_strSubLotID;
	// WaferID
	strFields += ";WaferID=";
	strFields += m_strWaferID;
	// TesterName
	strFields += ";TesterName=";
	strFields += m_strTesterName;
	// TesterType
	strFields += ";TesterType=";
	strFields += m_strTesterType;
	// OperatorName
	strFields += ";OperatorName=";
	strFields += m_strOperatorName;
	// JobName
	strFields += ";JobName=";
	strFields += m_strJobName;
	// JobRevision
	strFields += ";JobRevision=";
	strFields += m_strJobRevision;
	// ExecType
	strFields += ";ExecType=";
	strFields += m_strExecType;
	// ExecVersion
	strFields += ";ExecVersion=";
	strFields += m_strExecVersion;
	// TestCode
	strFields += ";TestCode=";
	strFields += m_strTestCode;
	// TestTemperature
	strFields += ";TestTemperature=";
	strFields += m_strTestTemperature;
	// Parts per site
	strFields += ";PartsPerSite=";
	for(int i=0 ; i<256 ; i++)
	{
		if(m_puiPartsPerSite[i] != 0)
		{
			strPartsPerSite.sprintf("[%d]%d ", i, m_puiPartsPerSite[i]);
			strFields += strPartsPerSite;
		}
	}

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Data Result definition
///////////////////////////////////////////////////////////
GP_DataResult::GP_DataResult()	// Default constructor
{
	m_uiProgramRunIndex = 0;
	m_nHeadNum = -1;
	m_nSiteNum = -1;
	m_bTestType = '-';
	m_nParameterIndex = -1;
	m_lfValue = 0.0;
	m_bIsOutlier = false;
	m_bPassFailStatusValid = false;
	m_bTestFailed = false;
}

GP_DataResult::GP_DataResult( const GP_DataResult *source )  // copy constructor
{
	m_uiProgramRunIndex = source->m_uiProgramRunIndex;
	m_nHeadNum = source->m_nHeadNum;
	m_nSiteNum = source->m_nSiteNum;
	m_bTestType = source->m_bTestType;
	m_nParameterIndex = source->m_nParameterIndex;
	m_lfValue = source->m_lfValue;
	m_bIsOutlier = source->m_bIsOutlier;
	m_bPassFailStatusValid = source->m_bPassFailStatusValid;
	m_bTestFailed = source->m_bTestFailed;
}

GP_DataResult& GP_DataResult::operator=( const GP_DataResult &s )  // assignment operator
{
	m_uiProgramRunIndex = s.m_uiProgramRunIndex;
	m_nHeadNum = s.m_nHeadNum;
	m_nSiteNum = s.m_nSiteNum;
	m_bTestType = s.m_bTestType;
	m_nParameterIndex = s.m_nParameterIndex;
	m_lfValue = s.m_lfValue;
	m_bIsOutlier = s.m_bIsOutlier;
	m_bPassFailStatusValid = s.m_bPassFailStatusValid;
	m_bTestFailed = s.m_bTestFailed;

	return *this; 
}

void GP_DataResult::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "GP_DataResult";
	// Program Run
	strFields += ";Program Run Index=";
	strFields += QString::number(m_uiProgramRunIndex);	
	// Head
	strFields += ";Head=";
	strFields += QString::number(m_nHeadNum);
	// Site
	strFields += ";Site=";
	strFields += QString::number(m_nSiteNum);
	// Index
	strFields += ";Index=";
	strFields += QString::number(m_nParameterIndex);
	// Type
	strFields += ";Type=";
	QChar clChar((char)m_bTestType);
	strFields += clChar;
	// Value
	strFields += ";Value=";
	strFields += QString::number(m_lfValue);
	// Outlier
	strFields += ";Outlier=";
	if(m_bIsOutlier)
		strFields += "Y";
	else
		strFields += "N";
	// Pass/Fail status valid
	strFields += ";Pass/Fail status valid=";
	if(m_bPassFailStatusValid)
		strFields += "Y";
	else
		strFields += "N";
	// Pass/Fail status
	strFields += ";Pass/Fail status=";
	if(m_bTestFailed)
		strFields += "F";
	else
		strFields += "P";

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// Part Result definition
///////////////////////////////////////////////////////////
GP_PartResult::GP_PartResult()	// Default constructor
{
	m_uiProgramRunIndex = 0;
	m_nHeadNum = 1;		
	m_nSiteNum = 1;		
	m_nPart_X = -32768;
	m_nPart_Y = -32768;	
	m_nHardBin = -1;	
	m_nSoftBin = -1;	
	m_strPartID = "";
	m_bPassFailStatusValid = false;
	m_bPartFailed = false;
}

GP_PartResult::GP_PartResult( const GP_PartResult *source )  // copy constructor
{
	m_uiProgramRunIndex = source->m_uiProgramRunIndex;
	m_nHeadNum = source->m_nHeadNum;
	m_nSiteNum = source->m_nSiteNum;
	m_nPart_X = source->m_nPart_X;
	m_nPart_Y = source->m_nPart_Y;
	m_nHardBin = source->m_nHardBin;
	m_nSoftBin = source->m_nSoftBin;
	m_strPartID = source->m_strPartID;
	m_bPassFailStatusValid = source->m_bPassFailStatusValid;
	m_bPartFailed = source->m_bPartFailed;
}

GP_PartResult& GP_PartResult::operator=( const GP_PartResult &s )  // assignment operator
{
	m_uiProgramRunIndex = s.m_uiProgramRunIndex;
	m_nHeadNum = s.m_nHeadNum;
	m_nSiteNum = s.m_nSiteNum;
	m_nPart_X = s.m_nPart_X;
	m_nPart_Y = s.m_nPart_Y;
	m_nHardBin = s.m_nHardBin;
	m_nSoftBin = s.m_nSoftBin;
	m_strPartID = s.m_strPartID;
	m_bPassFailStatusValid = s.m_bPassFailStatusValid;
	m_bPartFailed = s.m_bPartFailed;

	return *this; 
}

void GP_PartResult::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "GP_PartResult";
	// Program Run
	strFields += ";Program Run Index=";
	strFields += QString::number(m_uiProgramRunIndex);
	// Head Num
	strFields += ";Head=";
	strFields += QString::number(m_nHeadNum);
	// Site Num
	strFields += ";Site=";
	strFields += QString::number(m_nSiteNum);
	// X coordinate
	strFields += ";X=";
	strFields += QString::number(m_nPart_X);
	// Y coordinate
	strFields += ";Y=";
	strFields += QString::number(m_nPart_Y);
	// Hardware binning
	strFields += ";HardBin=";
	strFields += QString::number(m_nHardBin);
	// Software binning
	strFields += ";SoftBin=";
	strFields += QString::number(m_nSoftBin);
	// Part ID
	strFields += ";PartID=";
	strFields += m_strPartID;
	// Pass/Fail status valid
	strFields += ";Pass/Fail status valid=";
	if(m_bPassFailStatusValid)
		strFields += "Y";
	else
		strFields += "N";
	// Pass/Fail status
	strFields += ";Pass/Fail status=";
	if(m_bPartFailed)
		strFields += "F";
	else
		strFields += "P";

	// Add empty field if object dumps are concatenated
	strFields += ";----------=----------;";
}

///////////////////////////////////////////////////////////
// PRIVATE TYPES AND DEFINES
///////////////////////////////////////////////////////////
// Error map
GBEGIN_ERROR_MAP(CExaminatorBasePlugin)
	GMAP_ERROR(eLibraryNotFound,"Plug-in library not found: %s")
	GMAP_ERROR(eUnresolvedFunctions,"Unresolved function %s in plug-in library %s")
	GMAP_ERROR(eInit,"Error initializing plug-in library %s")
GEND_ERROR_MAP(CExaminatorBasePlugin)

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CExaminatorBasePlugin::CExaminatorBasePlugin(QString strPluginFullPath, QString strPluginDirectory)
{
	LibGetPluginID = NULL;
	LibGetPluginName = NULL;
	m_strPluginFullPath = strPluginFullPath;
	m_strPluginDirectory = strPluginDirectory;
	m_strPluginCommand = "";

#ifdef QT_DEBUG
	// For Debug
	char	*ptChar;
	QString strEnv;

	m_bPluginDebug_EventPopups = false;
	m_bPluginDebug_DumpData = false;
	m_bPluginDebug_HtmlReport = false;

	ptChar = getenv("GEX_PLUGINDEBUG_EVENTPOPUPS");
	if(ptChar != NULL)
	{
		strEnv = ptChar;
		m_bPluginDebug_EventPopups = (strEnv.toUpper() == "true");
	}
	ptChar = getenv("GEX_PLUGINDEBUG_DUMPDATA");
	if(ptChar != NULL)
	{
		strEnv = ptChar;
		m_bPluginDebug_DumpData = (strEnv.toUpper() == "true");
	}
	ptChar = getenv("GEX_PLUGINDEBUG_HTMLREPORT");
	if(ptChar != NULL)
	{
		strEnv = ptChar;
		m_bPluginDebug_HtmlReport = (strEnv.toUpper() == "true");
	}
#endif
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CExaminatorBasePlugin::~CExaminatorBasePlugin()
{
}

///////////////////////////////////////////////////////////
// Common plugin functions (specialized for each plug-in)
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Plugin initialization...
///////////////////////////////////////////////////////////
bool CExaminatorBasePlugin::Init(void)
{
	// Pure virtual: This method must be defined in the derived class
	return false;
}

///////////////////////////////////////////////////////////
// Check if standard GEX report should be created
///////////////////////////////////////////////////////////
int CExaminatorBasePlugin::BuildGEXReport(void)
{
	// Virtual: this method can be overwritten in the derived class
	// By default, if not implemented, report will be created.
	return 1;
}

///////////////////////////////////////////////////////////
// Check if only HTML format supported
///////////////////////////////////////////////////////////
bool CExaminatorBasePlugin::HTMLOutputOnly(void)
{
	// Virtual: this method can be overwritten in the derived class
	// By default, if not implemented, support only HTML format.
	return true;
}

///////////////////////////////////////////////////////////
// Data collection functions
///////////////////////////////////////////////////////////
// Calling sequence:
// 1:			eventBeginData
// 2:			eventBeginDataset
// 3...n:		eventDefineParameter
// n+1:			eventLot
// n+2....m:	eventParameterResult
//				eventPartResult
// m+1:			eventLot
// m+2....p:	eventParameterResult
// ...
// r:			eventEndDataset
// r+1:			eventEndData
void CExaminatorBasePlugin::eventBeginData(void)
{
	// Pure virtual: This method must be defined in the derived class
}

// Debug: Message Box with event details
void CExaminatorBasePlugin::eventBeginData_Display()
{
#ifdef QT_DEBUG
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_BEGINDATA))
	{
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_BEGINDATA, QString(PLUGIN_EVENTNAME_BEGINDATA), QString(""));
	}
#endif
}

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventBeginDataset_Display(const GP_DatasetDef & DatasetDef)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_BEGINDATASET))
	{
		QString strDump;
		DatasetDef.GetDump(strDump);
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_BEGINDATASET, QString(PLUGIN_EVENTNAME_BEGINDATASET), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventBeginDataset_Display(const GP_DatasetDef&)
{
}
#endif

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventDefineParameter_Display(const GP_ParameterDef & ParameterDefinition)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_DEFINEPARAMETER))
	{
		QString strDump;
		ParameterDefinition.GetDump(strDump);
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_DEFINEPARAMETER, QString(PLUGIN_EVENTNAME_DEFINEPARAMETER), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventDefineParameter_Display(const GP_ParameterDef&)
{
}
#endif

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventLot_Display(const GP_LotDef & LotDefinition, const GP_SiteDescription& clGlobalEquipmentID, const GP_SiteDescriptionMap* pSiteEquipmentIDMap)
{
	if(m_bPluginDebug_EventPopups)
	{
		QString strDump, strDumpObject;
		LotDefinition.GetDump(strDump);
		clGlobalEquipmentID.GetDump(strDumpObject);
		strDump += strDumpObject;
		for(GP_SiteDescriptionMap::const_iterator it = pSiteEquipmentIDMap->begin(); it != pSiteEquipmentIDMap->end(); ++it)
		{
            (*it).GetDump(strDumpObject);
			strDump += strDumpObject;
		}

		m_clEventDebug.NewLot();
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_LOT, QString(PLUGIN_EVENTNAME_LOT), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventLot_Display(const GP_LotDef&,
                                             const GP_SiteDescription&,
                                             const GP_SiteDescriptionMap*)
{
}
#endif

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventParameterResult_Display(const GP_DataResult & DataResult)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_PARAMETERRESULT))
	{
		QString strDump;
		DataResult.GetDump(strDump);
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_PARAMETERRESULT, QString(PLUGIN_EVENTNAME_PARAMETERRESULT), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventParameterResult_Display(const GP_DataResult&)
{
}
#endif

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventPartResult_Display(const GP_PartResult & PartResult)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_PARTRESULT))
	{
		QString strDump;
		PartResult.GetDump(strDump);
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_PARTRESULT, QString(PLUGIN_EVENTNAME_PARTRESULT), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventPartResult_Display(const GP_PartResult&)
{
}
#endif

void CExaminatorBasePlugin::eventEndDataset(void)	// End of current Dataset
{
	// Pure virtual: This method must be defined in the derived class
}

// Debug: Message Box with event details
void CExaminatorBasePlugin::eventEndDataset_Display()
{
#ifdef QT_DEBUG
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_ENDDATASET))
	{
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_ENDDATASET, QString(PLUGIN_EVENTNAME_ENDDATASET), QString(""));
	}
#endif
}

void CExaminatorBasePlugin::eventEndData(void)
{
	// Pure virtual: This method must be defined in the derived class
}

// Debug: Message Box with event details
void CExaminatorBasePlugin::eventEndData_Display()
{
#ifdef QT_DEBUG
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_ENDDATA))
	{
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_ENDDATA, QString(PLUGIN_EVENTNAME_ENDDATA), QString(""));
	}
#endif
}

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventSetCommand_Display(const QString & strCommand)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_SETCOMMAND))
	{
		QString strDump = "Command=";
		strDump += strCommand;
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_SETCOMMAND, QString(PLUGIN_EVENTNAME_SETCOMMAND), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventSetCommand_Display(const QString&)
{
}
#endif

void CExaminatorBasePlugin::eventSetSetting(const QString& /*strSetting*/)
{
	// Virtual: this method can be overwritten in the derived class
	// By default, if not implemented, do nothing.
}

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventSetSetting_Display(const QString & strSetting)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_SETSETTING))
	{
		QString strDump = "Setting=";
		strDump += strSetting;
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_SETSETTING, QString(PLUGIN_EVENTNAME_SETSETTING), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventSetSetting_Display(const QString&)
{
}
#endif

void CExaminatorBasePlugin::eventExecCommand(void)
{
	// Pure virtual: This method must be defined in the derived class
}

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventExecCommand_Display(const QString & strCommand)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_EXECCOMMAND))
	{
		QString strDump = "Command=";
		strDump += strCommand;
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_EXECCOMMAND, QString(PLUGIN_EVENTNAME_EXECCOMMAND), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventExecCommand_Display(const QString&)
{
}
#endif

void CExaminatorBasePlugin::eventAbort(void)
{
	// Virtual: this method can be overwritten in the derived class
	// By default, if not implemented, do nothing.
}

// Debug: Message Box with event details
void CExaminatorBasePlugin::eventAbort_Display()
{
#ifdef QT_DEBUG
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_ABORT))
	{
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_ABORT, QString(PLUGIN_EVENTNAME_ABORT), QString(""));
	}
#endif
}

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventAction_Display(const QString & strAction)
{
	if(m_bPluginDebug_EventPopups)
	{
		QString strDump = "Type=";
		strDump += strAction;
		m_clEventDebug.Reset();
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_ACTION, QString(PLUGIN_EVENTNAME_ACTION), strDump);
	}
}
#else
void CExaminatorBasePlugin::eventAction_Display(const QString&)
{
}
#endif

void CExaminatorBasePlugin::eventGenerateReport(void)
{
	// Pure virtual: This method must be defined in the derived class
}

// Debug: Message Box with event details
void CExaminatorBasePlugin::eventGenerateReport_Display()
{
#ifdef QT_DEBUG
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_GENERATEREPORT))
	{
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_GENERATEREPORT, QString(PLUGIN_EVENTNAME_GENERATEREPORT), QString(""));
	}
#endif
}

void
CExaminatorBasePlugin::
eventInteractiveCmd(GP_InteractiveCmd& /*InteractiveCmd*/)
{
	// Virtual: this method can be overwritten in the derived class
	// By default, if not implemented, do nothing.
}

// Debug: Message Box with event details
#ifdef QT_DEBUG
void CExaminatorBasePlugin::eventInteractiveCmd_Display(const GP_InteractiveCmd & InteractiveCmd)
{
	if(m_bPluginDebug_EventPopups && m_clEventDebug.DisplayEvent_Enabled(PLUGIN_EVENTID_INTERACTIVECMD))
	{
		QString strDump;
		InteractiveCmd.GetDump(strDump);
		m_clEventDebug.DisplayEvent(PLUGIN_EVENTID_INTERACTIVECMD, QString(PLUGIN_EVENTNAME_INTERACTIVECMD), strDump);
	}
}
#else
void
CExaminatorBasePlugin::eventInteractiveCmd_Display(const GP_InteractiveCmd&)
{
}
#endif

