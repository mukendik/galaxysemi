///////////////////////////////////////////////////////////
// Plug-in to interface with YIELD-123.
///////////////////////////////////////////////////////////

// Standard includes
#include <QProgressDialog>
#include <QDateTime>
#include <QTextStream>

// Local includes
#include "gate_event_manager.h"
#include "gate_event_constants.h"
#include "gate_data_model.h"
#include "gate_data_stats.h"



///////////////////////////////////////////////////////////
// Class for display of event contents (for DEBUG)
///////////////////////////////////////////////////////////
Gate_EventDebug::Gate_EventDebug()	// Default constructor
{
	Reset();
}

void Gate_EventDebug::Reset()		// Reset all flags
{
	m_bDisplay_AllEvents = TRUE;
	m_bDiscard_AllEvents = FALSE;
	for(int i=0 ; i<=PLUGIN_EVENTID_MAX ; i++)
	{
		m_bDisplay_SingleEvent[i] = TRUE;
		m_bDiscard_SingleEvent[i] = FALSE;
	}
}

void Gate_EventDebug::NewLot()	// New lot event received
{
	m_bDisplay_AllEvents = !m_bDiscard_AllEvents;
	for(int i=0 ; i<=PLUGIN_EVENTID_MAX ; i++)
	{
		m_bDisplay_SingleEvent[i] = !m_bDiscard_SingleEvent[i];
	}
}

void Gate_EventDebug::DisplayEvent(int /*nEventID*/,
								   const QString& /*strEventName*/,
								   const QString& /*strEventDump*/)
// Display event content
{
	/*if(m_bDisplay_AllEvents && m_bDisplay_SingleEvent[nEventID])
	{
	}*/
}

///////////////////////////////////////////////////////////
// Data containers
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Plugin Info Details
///////////////////////////////////////////////////////////
Gate_PluginDef::Gate_PluginDef()	// Default constructor
{
	m_lPluginModuleID = -1;
	m_strPluginName = "";
}

Gate_PluginDef::Gate_PluginDef( const Gate_PluginDef *source )  // copy constructor
{
	m_lPluginModuleID = source->m_lPluginModuleID;
	m_strPluginName = source->m_strPluginName;
}

Gate_PluginDef& Gate_PluginDef::operator=( const Gate_PluginDef &s )	// assignment operator
{
	m_lPluginModuleID = s.m_lPluginModuleID;
	m_strPluginName = s.m_strPluginName;

	 return *this;
}

///////////////////////////////////////////////////////////
// Interactive command Definition
///////////////////////////////////////////////////////////
Gate_InteractiveCmd::Gate_InteractiveCmd()	// Default constructor
{
	m_uiSiteIndex				= 0;
	m_nSiteNb					= 0;
	m_uiParameterIndex			= 0;
	m_uiParameterNumber			= 0;
	m_strParameterName			= "";
	m_nChartType				= 0;
	m_nAdvancedReportSettings	= 0;
}

Gate_InteractiveCmd::Gate_InteractiveCmd( const Gate_InteractiveCmd *source )  // copy constructor
{
	m_uiSiteIndex = source->m_uiSiteIndex;
	m_nSiteNb = source->m_nSiteNb;
	m_uiParameterIndex = source->m_uiParameterIndex;
	m_uiParameterNumber = source->m_uiParameterNumber;
	m_strParameterName = source->m_strParameterName;
	m_nChartType = source->m_nChartType;
	m_nAdvancedReportSettings = source->m_nAdvancedReportSettings;
}

Gate_InteractiveCmd& Gate_InteractiveCmd::operator=( const Gate_InteractiveCmd &s )	// assignment operator
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

void Gate_InteractiveCmd::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_InteractiveCommand";
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
Gate_DatasetDef::Gate_DatasetDef()	// Default constructor
{
	m_lGroupID = 0;
	m_strGroupName = "";
	m_lTotalParameters = 0;
	m_lTotalBatches = 0;
}

Gate_DatasetDef::Gate_DatasetDef( const Gate_DatasetDef *source )  // copy constructor
{
	m_lGroupID = source->m_lGroupID;
	m_strGroupName = source->m_strGroupName;
	m_lTotalParameters = source->m_lTotalParameters;
	m_lTotalBatches = source->m_lTotalBatches;
}

Gate_DatasetDef& Gate_DatasetDef::operator=( const Gate_DatasetDef &s )	// assignment operator
{
	m_lGroupID = s.m_lGroupID;
	m_strGroupName = s.m_strGroupName;
	m_lTotalParameters = s.m_lTotalParameters;
	m_lTotalBatches = s.m_lTotalBatches;

	return *this;
}

void Gate_DatasetDef::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_DatasetDef";
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
Gate_ParameterDef::Gate_ParameterDef()	// Default constructor
{
	ResetData();
}

void Gate_ParameterDef::ResetData()		// Reset object data
{
	m_nParameterIndex = -1;
	m_nParameterNumber = -1;
	m_nPinArrayIndex = -1;
	m_strName = "";
	m_strUnits = "";
	m_lfLowL = 0.0;
	m_lfHighL = 0.0;
	m_bTestType = ' ';
	m_bHasLowL = FALSE;
	m_bHasHighL = FALSE;
	m_bStrictLowL = TRUE;
	m_bStrictHighL = TRUE;
	m_nLowLScaleFactor = 0;
	m_nHighLScaleFactor = 0;
	m_nResultScaleFactor = 0;
	m_uiFlags = 0;
	m_nId = 0;
}

Gate_ParameterDef::Gate_ParameterDef( const Gate_ParameterDef *source )  // copy constructor
{
	m_nParameterIndex = source->m_nParameterIndex;
	m_nParameterNumber = source->m_nParameterNumber;
	m_nPinArrayIndex = source->m_nPinArrayIndex;
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
	m_uiFlags = source->m_uiFlags;
	m_nId = source->m_nId;
}

Gate_ParameterDef& Gate_ParameterDef::operator=( const Gate_ParameterDef &s )	// assignment operator
{
	m_nParameterIndex = s.m_nParameterIndex;
	m_nParameterNumber = s.m_nParameterNumber;
	m_nPinArrayIndex = s.m_nPinArrayIndex;
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
	m_uiFlags = s.m_uiFlags;
	m_nId = s.m_nId;

	return *this;
}

void Gate_ParameterDef::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_ParameterDef";
	// ParameterIndex
	strFields += ";Index=";
	strFields += QString::number(m_nParameterIndex);
	// ParameterNumber
	strFields += ";Number=";
	strFields += QString::number(m_nParameterNumber);
	// Name
	strFields += ";Name=";
	strFields += m_strName;
	// PinArrayIndex
	strFields += ";PinArrayIndex=";
	strFields += QString::number(m_nPinArrayIndex);
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
// Binning definition
///////////////////////////////////////////////////////////
Gate_BinningDef::Gate_BinningDef()	// Default constructor
{
	ResetData();
}

void Gate_BinningDef::ResetData()		// Reset object data
{
	m_bBinType = 'H';
	m_nBinNumber = -1;
	m_strBinName = "";
	m_bBinCat = 'P';
}

Gate_BinningDef::Gate_BinningDef( const Gate_BinningDef *source )  // copy constructor
{
	m_bBinType = source->m_bBinType;
	m_nBinNumber = source->m_nBinNumber;
	m_strBinName = source->m_strBinName;
	m_bBinCat = source->m_bBinCat;
}

Gate_BinningDef& Gate_BinningDef::operator=( const Gate_BinningDef &s )	// assignment operator
{
	m_bBinType = s.m_bBinType;
	m_nBinNumber = s.m_nBinNumber;
	m_strBinName = s.m_strBinName;
	m_bBinCat = s.m_bBinCat;

	return *this;
}


///////////////////////////////////////////////////////////
// Parameter statistics
///////////////////////////////////////////////////////////
Gate_ParameterStats::Gate_ParameterStats()	// Default constructor
{
	ResetData();
}

void Gate_ParameterStats::ResetData()	// Reset object data
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

Gate_ParameterStats::Gate_ParameterStats( const Gate_ParameterStats *source )  // copy constructor
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

Gate_ParameterStats& Gate_ParameterStats::operator=( const Gate_ParameterStats &s )	// assignment operator
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

void Gate_ParameterStats::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_ParameterStats";
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
Gate_SiteDescription::Gate_SiteDescription()
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

Gate_SiteDescription::~Gate_SiteDescription()
{
}

Gate_SiteDescription& Gate_SiteDescription::operator+=(const Gate_SiteDescription& aSite)
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

Gate_SiteDescription& Gate_SiteDescription::operator=(const Gate_SiteDescription& aSite)
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

void Gate_SiteDescription::GetDump(QString & strFields) const
{
	QString strPartsPerSite;

	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_SiteDescription";
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
Gate_LotDef::Gate_LotDef()	// Default constructor
{
	m_strProductID			= "";
	m_strDatasetOrigin		= "";
	m_strLotID				= "";
	m_strProcId				= "";
	m_strSubLotID			= "";
	m_strWaferID			= "";
	m_strTesterName			= "";
	m_strTesterType			= "";
	m_strOperatorName		= "";
	m_strJobName			= "";
	m_strJobRevision		= "";
	m_strExecType			= "";
	m_strExecVersion		= "";
	m_strTestCode			= "";
	m_strTestTemperature	= "";
	m_lStartTime			= 0;
	m_lFinishTime			= 0;
	m_lTotalParts			= 0;
	m_uiProgramRuns			= 0;
	m_uiNbSites				= 0;
	m_puiPartsPerSite		= NULL;


}

Gate_LotDef::Gate_LotDef( const Gate_LotDef *source )  // copy constructor
{
	m_strProductID		= source->m_strProductID;
	m_strDatasetOrigin	= source->m_strDatasetOrigin;
	m_strLotID			= source->m_strLotID;
	m_strProcId			= source->m_strProcId;
	m_strSubLotID		= source->m_strSubLotID;
	m_strWaferID		= source->m_strWaferID;
	m_strTesterName		= source->m_strTesterName;
	m_strTesterType		= source->m_strTesterType;
	m_strOperatorName	= source->m_strOperatorName;
	m_strJobName		= source->m_strJobName;
	m_strJobRevision	= source->m_strJobRevision;
	m_strExecType		= source->m_strExecType;
	m_strExecVersion	= source->m_strExecVersion;
	m_strTestCode		= source->m_strTestCode;
	m_strTestTemperature = source->m_strTestTemperature;
	m_lStartTime		= source->m_lStartTime;
	m_lFinishTime		= source->m_lFinishTime;
	m_lTotalParts		= source->m_lTotalParts;
	m_uiProgramRuns		= source->m_uiProgramRuns;
	m_puiPartsPerSite	= source->m_puiPartsPerSite;
}

Gate_LotDef& Gate_LotDef::operator=( const Gate_LotDef &s )  // assignment operator
{
	m_strProductID		= s.m_strProductID;
	m_strDatasetOrigin	= s.m_strDatasetOrigin;
	m_strLotID			= s.m_strLotID;
	m_strProcId			= s.m_strProcId;
	m_strSubLotID		= s.m_strSubLotID;
	m_strWaferID		= s.m_strWaferID;
	m_strTesterName		= s.m_strTesterName;
	m_strTesterType		= s.m_strTesterType;
	m_strOperatorName	= s.m_strOperatorName;
	m_strJobName		= s.m_strJobName;
	m_strJobRevision	= s.m_strJobRevision;
	m_strExecType		= s.m_strExecType;
	m_strExecVersion	= s.m_strExecVersion;
	m_strTestCode		= s.m_strTestCode;
	m_strTestTemperature = s.m_strTestTemperature;
	m_lStartTime		= s.m_lStartTime;
	m_lFinishTime		= s.m_lFinishTime;
	m_lTotalParts		= s.m_lTotalParts;
	m_uiProgramRuns		= s.m_uiProgramRuns;
	m_puiPartsPerSite	= s.m_puiPartsPerSite;

	return *this;
}

void Gate_LotDef::GetDump(QString & strFields) const
{
	QString		strPartsPerSite;
	QDateTime	clDateTime;

	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_LotDef";
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
Gate_DataResult::Gate_DataResult()	// Default constructor
{
	m_uiProgramRunIndex = 0;
	m_nFlowId = 0;
	m_nHeadNum = -1;
	m_nSiteNum = -1;
	m_nStepNum = -1;
	m_bTestType = '-';
	m_nParameterIndex = -1;
	m_lfValue = 0.0;
	m_strValue = "-";
	m_bValidValue = FALSE;
	m_bIsOutlier = FALSE;
	m_bPassFailStatusValid = FALSE;
	m_bTestFailed = FALSE;
}

Gate_DataResult::Gate_DataResult( const Gate_DataResult *source )  // copy constructor
{
	m_uiProgramRunIndex = source->m_uiProgramRunIndex;
	m_nFlowId = source->m_nFlowId;
	m_nHeadNum = source->m_nHeadNum;
	m_nSiteNum = source->m_nSiteNum;
	m_nStepNum = source->m_nStepNum;
	m_bTestType = source->m_bTestType;
	m_nParameterIndex = source->m_nParameterIndex;
	m_lfValue = source->m_lfValue;
	m_strValue = source->m_strValue;
	m_bValidValue = source->m_bValidValue;
	m_bIsOutlier = source->m_bIsOutlier;
	m_bPassFailStatusValid = source->m_bPassFailStatusValid;
	m_bTestFailed = source->m_bTestFailed;
}

Gate_DataResult& Gate_DataResult::operator=( const Gate_DataResult &s )  // assignment operator
{
	m_uiProgramRunIndex = s.m_uiProgramRunIndex;
	m_nFlowId = s.m_nFlowId;
	m_nHeadNum = s.m_nHeadNum;
	m_nSiteNum = s.m_nSiteNum;
	m_nStepNum = s.m_nStepNum;
	m_bTestType = s.m_bTestType;
	m_nParameterIndex = s.m_nParameterIndex;
	m_lfValue = s.m_lfValue;
	m_strValue = s.m_strValue;
	m_bValidValue = s.m_bValidValue;
	m_bIsOutlier = s.m_bIsOutlier;
	m_bPassFailStatusValid = s.m_bPassFailStatusValid;
	m_bTestFailed = s.m_bTestFailed;

	return *this;
}

void Gate_DataResult::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_DataResult";
	// Program Run
	strFields += ";Program Run Index=";
	strFields += QString::number(m_uiProgramRunIndex);
	// Flow ID
	strFields += ";Flow ID=";
	strFields += QString::number(m_nFlowId);
	// Head
	strFields += ";Head=";
	strFields += QString::number(m_nHeadNum);
	// Site
	strFields += ";Site=";
	strFields += QString::number(m_nSiteNum);
	// Step
	strFields += ";Step=";
	strFields += QString::number(m_nStepNum);
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
	// String Value
	strFields += ";SValue=";
	strFields += m_strValue;
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
Gate_PartResult::Gate_PartResult()	// Default constructor
{
	m_uiProgramRunIndex = 0;
	m_nHeadNum = 1;
	m_nSiteNum = 1;
	m_nPart_X = -32768;
	m_nPart_Y = -32768;
	m_nHardBin = -1;
	m_nSoftBin = -1;
	m_strPartID = "";
	m_bPassFailStatusValid = FALSE;
	m_bPartFailed = FALSE;
}

Gate_PartResult::Gate_PartResult( const Gate_PartResult *source )  // copy constructor
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

Gate_PartResult& Gate_PartResult::operator=( const Gate_PartResult &s )  // assignment operator
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

void Gate_PartResult::GetDump(QString & strFields) const
{
	// Object name
	strFields = "OBJECT=";
	strFields += "Gate_PartResult";
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
GBEGIN_ERROR_MAP(CGate_EventManager)
	GMAP_ERROR(eLibraryNotFound,"Plug-in library not found: %s")
	GMAP_ERROR(eUnresolvedFunctions,"Unresolved function %s in plug-in library %s")
	GMAP_ERROR(eInit,"Error initializing plug-in library %s")
GEND_ERROR_MAP(CGate_EventManager)

///////////////////////////////////////////////////////////
// Data collection functions
///////////////////////////////////////////////////////////
// Calling sequence:
// 1:			EventManager_BeginDataset
// 2...n:		EventManager_DefineParameter
// n+1:			EventManager_Lot
// n+2....m:	EventManager_ParameterResult
// m+1:			EventManager_Lot
// m+2....p:	EventManager_ParameterResult
// ...
// r:			EventManager_EndDataset

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGate_EventManager::CGate_EventManager()
{
	m_pTmpParameterSet = NULL;
	m_pclData = NULL;
	m_pEngineGeneric = NULL;
	m_pProgress = NULL;
	m_bInvalidData = false;

	m_nPluginCommandID = GEX_YIELD123_COMMAND_ID_YIELD;

	ClearData();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGate_EventManager::~CGate_EventManager()
{
	ClearData();
}

////////////////////////////////////////////////////
// clear all data structures
///////////////////////////////////////////////////////////
bool CGate_EventManager::ClearData(void)
{
	// Free allocated ressources
	if(m_pTmpParameterSet != NULL)
	{
		delete m_pTmpParameterSet; m_pTmpParameterSet = NULL;
	}
	if(m_pclData != NULL)
	{
		delete m_pclData;
		m_pclData = NULL;
	}

	// Reset variables
	m_strGroupName = "";
	m_pCurrentParameterSet = NULL;
	m_pCurrentTestingStage = NULL;
	m_pCurrentLot = NULL;
	m_pCurrentSublot = NULL;
	m_pCurrentBatch = NULL;
	m_eStatus = eStatusOK;
	m_strError = "";
	m_bInvalidData = false;

	return true;
}

///////////////////////////////////////////////////////////
// Common plugin functions (specialized for YIELD123 plug-in)
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Plugin initialization...
///////////////////////////////////////////////////////////
bool CGate_EventManager::Init(void)
{
	return true;
}

///////////////////////////////////////////////////////////
// Event: Start reporting all datasets
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventBeginData()
{
	try
	{
		if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
		{
			m_bInvalidData = false;
			m_eStatus = eStatus_Error_CommandNotSupported;
			m_strError = "eventBeginData";
			return false;
		}

		// Clear all data structures
		ClearData();

		// Create Data object
		m_pclData = new C_Gate_DataModel(m_pProgress);

		return true;
	} // catch
	catch(...)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_TryCatch;
		m_strError = "eventBeginData";
		return false;
	}
}

///////////////////////////////////////////////////////////
// Event: Start reporting a dataset data (group)
///////////////////////////////////////////////////////////
bool	CGate_EventManager::eventBeginDataset(const Gate_DatasetDef & DatasetDef)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventBeginData";
		return false;
	}

	// Create temporary parameter set (delete it if necessary)
	if(m_pTmpParameterSet != NULL)
	{
		delete m_pTmpParameterSet;
		m_pTmpParameterSet = NULL;
	}

	// Check if dataset has some parameters
	if(DatasetDef.m_lTotalParameters == 0)
	{
		m_bInvalidData = true;
		SetErrorMessage(eStatus_Error_NoParameterSet, "No Paramater set for current lot");
		return false;
	}

	// Create new temporary parameter set
	m_pTmpParameterSet = new C_Gate_DataModel_ParameterSet;
	if(m_pTmpParameterSet == NULL)
	{
		m_eStatus = eStatus_Error_Malloc;
		m_strError = "Memory allocation failure";
		return false;
	}

	// Init parameter set (allocates memory for parameter definition)
	if(!m_pTmpParameterSet->Init(DatasetDef.m_lTotalParameters))
	{
		if(GGET_LASTERRORCODE(C_Gate_DataModel_ParameterSet, m_pTmpParameterSet) == C_Gate_DataModel_ParameterSet::eNotAllParametersDefined)
			m_bInvalidData = true;
		SetErrorMessage(eStatus_Error_ParameterSet_Init,GGET_LASTERRORMSG(C_Gate_DataModel_ParameterSet, m_pTmpParameterSet));
		delete m_pTmpParameterSet;
		m_pTmpParameterSet = NULL;
		return false;
	}

	// Init other members
	m_strGroupName = DatasetDef.m_strGroupName;
	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventBeginDataset";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: Define one Parameter static information (name, limits,...)
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventDefineParameter(const Gate_ParameterDef & ParameterDefinition)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventDefineParameter";
		return false;
	}

	// Check if we have all needed objects
	if((m_pclData == NULL) || (m_pTmpParameterSet == NULL))
		return false;

	// Check if no error occured so far
	if(m_eStatus != eStatusOK)
		return false;

	// Set parameter definition
	if(m_pTmpParameterSet->SetParameter(ParameterDefinition) == FALSE)
	{
		if(GGET_LASTERRORCODE(C_Gate_DataModel_ParameterSet, m_pTmpParameterSet) == C_Gate_DataModel_ParameterSet::eNotAllParametersDefined)
			m_bInvalidData = true;
		SetErrorMessage(eStatus_Error_ParameterDefinition,GGET_LASTERRORMSG(C_Gate_DataModel_ParameterSet, m_pTmpParameterSet));
		delete m_pTmpParameterSet;
		m_pTmpParameterSet = NULL;
		return false;
	}
	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventDefineParameter";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: Define one Binning static information (name, limits,...)
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventDefineBinning(const Gate_BinningDef & BinDefinition)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventDefineBinning";
		return false;
	}

	// Check if we have all needed objects
	if((m_pclData == NULL) || (m_pCurrentBatch == NULL))
		return false;

	// Check if no error occured so far
	if(m_eStatus != eStatusOK)
		return false;

	// Set parameter definition
	m_pCurrentBatch->SetBinning(BinDefinition);
	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventDefineBinning";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: New Lot/Sub-lot
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventLot(const Gate_LotDef & LotDefinition, const Gate_SiteDescription& clGlobalEquipmentID, const Gate_SiteDescriptionMap* pSiteEquipmentIDMap, bool bIgnoreThisSubLot)
{
try {
	if(bIgnoreThisSubLot)
		return true;

	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventBeginData";
		return false;
	}

	// Check if we have all needed objects
	if(m_pclData == NULL)
		return false;

	// Check if no error occured so far
	if(m_eStatus != eStatusOK)
		return false;

	// Get testing stage to be used for data to be received
	m_pCurrentTestingStage = m_pclData->GetTestingStage(LotDefinition);
	if(m_pCurrentTestingStage == NULL)
	{
		if(GGET_LASTERRORCODE(C_Gate_DataModel, m_pclData) == C_Gate_DataModel::eMultipleProducts)
			m_bInvalidData = true;
		SetErrorMessage(eStatus_Error_GetTestingStage,GGET_LASTERRORMSG(C_Gate_DataModel, m_pclData));
		return false;
	}

	// Check if a parameter set has been created
	if(m_pTmpParameterSet != NULL)
	{
		// Check if parameter set is OK
		if(m_pTmpParameterSet->Check() == FALSE)
		{
			m_bInvalidData = true;
			SetErrorMessage(eStatus_Error_InvalidParameterSet,GGET_LASTERRORMSG(C_Gate_DataModel_ParameterSet, m_pTmpParameterSet));
			delete m_pTmpParameterSet;
			m_pTmpParameterSet = NULL;
			return false;
		}

		// Get the Parameter set to use
		bool bAlreadyInList;
		m_pCurrentParameterSet = m_pCurrentTestingStage->GetParameterSet(m_pTmpParameterSet, &bAlreadyInList);
		if(bAlreadyInList)
			delete m_pTmpParameterSet;
		m_pTmpParameterSet = NULL;
	}

	// Get lot, sublot and batch to be used for data to be received
	m_pCurrentLot = m_pCurrentTestingStage->GetLot(LotDefinition);
	m_pCurrentSublot = m_pCurrentLot->GetSublot(LotDefinition);

	// Add batch to current lot
	m_pCurrentBatch = m_pCurrentSublot->AddBatch(LotDefinition, m_pCurrentParameterSet, clGlobalEquipmentID, pSiteEquipmentIDMap);
	if(!m_pCurrentBatch)
	{
		int eStatus = GGET_LASTERRORCODE(C_Gate_DataModel_Sublot,m_pCurrentSublot);
		if(eStatus != C_Gate_DataModel_Sublot::eInitSite)
			m_bInvalidData = true;
		SetErrorMessage(eStatus_Error_AddBatch,GGET_LASTERRORMSG(C_Gate_DataModel_Sublot, m_pCurrentSublot));
		return false;
	}

	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventLot";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: Datalog one Parameter result
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventParameterResult(const Gate_DataResult & DataResult)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventBeginData";
		return false;
	}

	// Check if we have all needed objects
	if((m_pclData == NULL) || (m_pCurrentBatch == NULL))
		return false;

	// Check if no error occured so far
	if(m_eStatus != eStatusOK)
		return false;

	// Don't use outliers
	if(DataResult.m_bIsOutlier == TRUE)
		return true;

	// Set parameter result
	if(m_pCurrentBatch->SetTestResult(DataResult) == FALSE)
	{
		SetErrorMessage(eStatus_Error_SetTestResult,GGET_LASTERRORMSG(C_Gate_DataModel_Batch, m_pCurrentBatch));
		return false;
	}

	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventParameterResult";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: Datalog one Part result
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventPartResult(const Gate_PartResult & PartResult)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventBeginData";
		return false;
	}

	// Check if we have all needed objects
	if((m_pclData == NULL) || (m_pCurrentBatch == NULL))
		return false;

	// Check if no error occured so far
	if(m_eStatus != eStatusOK)
		return false;

	// Set part result
	if(m_pCurrentBatch->SetPartResult(PartResult) == FALSE)
	{
		SetErrorMessage(eStatus_Error_SetPartResult,GGET_LASTERRORMSG(C_Gate_DataModel_Batch, m_pCurrentBatch));
		return false;
	}

	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventPartResult";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: End of current data set
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventEndDataset(void)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventBeginData";
		return false;
	}

	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventEndDataset";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Event: End of ALL datasets
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventEndData(void)
{
try {
	if(m_nPluginCommandID == GEX_YIELD123_COMMAND_ID_NONE)
	{
		m_bInvalidData = false;
		m_eStatus = eStatus_Error_CommandNotSupported;
		m_strError = "eventBeginData";
		return false;
	}

	// Cleanup data set
	if(m_pclData)
		m_pclData->Cleanup();
	return true;

	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventEndData";
	return false;
	}
}

///////////////////////////////////////////////////////////
// Dump Y123 Data
///////////////////////////////////////////////////////////
bool CGate_EventManager::eventDumpData(const QString & strReportFile)
{
try {

	// Open dump file
	QFile f( strReportFile );
	if(f.open( QIODevice::WriteOnly ) == FALSE)
		return false;

	// Assign file I/O stream
	QTextStream hDumpFile(&f);
	if(m_pclData)
		m_pclData->Dump(hDumpFile);
//	if(m_pEngineGeneric)
//		m_pEngineGeneric->Dump(hDumpFile);

	// Close Dump file
	f.close();

	return true;
	} // catch
catch(...) {
	m_bInvalidData = false;
	m_eStatus = eStatus_Error_TryCatch;
	m_strError = "eventDumpData";
	return false;
	}
}


bool CGate_EventManager::eventInitAnalyzeDataModel()
{
//try {
//	if(m_pEngineGeneric == NULL)
//		return false;

	// Check if EngineGeneric was correctly initialized
//	if(m_pEngineGeneric->m_pclData == NULL)
//		m_pEngineGeneric->m_pclData = m_pclData;

	// Compute basic statistics
//	if(!m_pEngineGeneric->Init())
//	{
//		int eStatus = GGET_LASTERRORCODE(C_Y123_Engine_Generic,m_pEngineGeneric);
//		if(eStatus == C_Y123_Engine_Generic::eDatasetNotCompatible)
//			m_bInvalidData = true;
//
//		SetErrorMessage(eStatus_Error_InitEngine,GGET_LASTERRORMSG(C_Y123_Engine_Generic,m_pEngineGeneric));
//		return false;
//	}
//
//	return TRUE;
//
//	} // catch
//catch(...) {
//	m_bInvalidData = false;
//	m_eStatus = eStatus_Error_TryCatch;
//	m_strError = "eventInitAnalyzeDataModel : m_pEngineGeneric->Init()";
//	return false;
//	}
	return true;
}

bool CGate_EventManager::eventRunAnalyzeDataModel()
{
//try {
//	// Execute rules for each analysis object
//	if(!m_pEngineGeneric->Run())
//	{
//		int eStatus = GGET_LASTERRORCODE(C_Y123_Engine_Generic,m_pEngineGeneric);
//		if(eStatus == C_Y123_Engine_Generic::eDatasetNotCompatible)
//			m_bInvalidData = true;
//
//		SetErrorMessage(eStatus_Error_InitEngine,GGET_LASTERRORMSG(C_Y123_Engine_Generic,m_pEngineGeneric));
//		return false;
//	}
//
//	return TRUE;
//
//	} // catch
//catch(...) {
//	m_bInvalidData = false;
//	m_eStatus = eStatus_Error_TryCatch;
//	m_strError = "eventRunAnalyzeDataModel : m_pEngineGeneric->Run()";
//	return false;
//	}
	return true;
}

bool CGate_EventManager::SetErrorMessage(enum Status eStatus, const char *szError)
{
	m_eStatus = eStatus;
	m_strError = m_strAnalyzeName + " analysis - ";
	m_strError += szError;
	return TRUE;
}

