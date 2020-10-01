#include <gqtl_log.h>
#include "gexdb_plugin_base.h"

int GexDbPlugin_ER_Parts::s_numInstances=0;

GexDbPlugin_ER_Parts::GexDbPlugin_ER_Parts()
{

	s_numInstances++;
//	setAutoDelete(true);
	m_bSoftBin = false;
    GSLOG(SYSLOG_SEV_DEBUG, QString("GexDbPlugin_ER_Parts has now %1 instances.")
          .arg( s_numInstances).toLatin1().constData());
}

GexDbPlugin_ER_Parts::~GexDbPlugin_ER_Parts()
{
	s_numInstances--;
//    qDeleteAll(*this);
//    clear();
}

void GexDbPlugin_ER_Parts::Init(	QStringList & strlFields_GraphSplit, QStringList & strlFields_LayerSplit,
			QString & strField_Aggregate,
			bool bSoftBin, GexDbPlugin_ER_Parts_Series & plistSeries, QString & strMaverickWafer_AlarmType,
			int nMaverickWafer_AlarmCount, int nMaverickLot_WaferCount, QString & strGenealogy_Granularity)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString(" Aggregate=%1 Granularity=%2 AlarmType=%3 ")
             .arg(strField_Aggregate.toLatin1().data())
             .arg(strGenealogy_Granularity.toLatin1().data())
             .arg(strMaverickWafer_AlarmType.toLatin1().data())).toLatin1().constData() );
	QStringList::iterator			it;
	GexDbPlugin_ER_Parts_SerieDef	*pSerieDef_Source, *pSerieDef_New;
	QString							strBinList;
	m_strlFields_GraphSplit = strlFields_GraphSplit;
	m_strlFields_LayerSplit = strlFields_LayerSplit;
	m_strField_Aggregate = strField_Aggregate;
	m_bSoftBin = bSoftBin;
	m_bWorkOnBinnings = false;
	m_uiSeries = 0;
	m_uiNbAxis = 0;
//	for(pSerieDef_Source = plistSeries.first(); pSerieDef_Source; pSerieDef_Source = plistSeries.next())
    foreach(pSerieDef_Source, plistSeries)
	{
		m_uiSeries++;
		// We have to append a copy of the serie definition, because the original will be cleared after the report has been generated
		pSerieDef_New = new GexDbPlugin_ER_Parts_SerieDef();
		*(pSerieDef_New) = *(pSerieDef_Source);
		m_plistSerieDefs.append(pSerieDef_New);
		strBinList = pSerieDef_New->m_strBinnings.toLower();
		if((strBinList.indexOf("pass") == -1) && (strBinList.indexOf("fail") == -1) && (strBinList.indexOf("all") == -1))
			pSerieDef_New->m_bWorkOnBinnings = m_bWorkOnBinnings = true;
	}
	m_strMaverickWafer_AlarmType = strMaverickWafer_AlarmType;
	m_nMaverickWafer_AlarmCount = nMaverickWafer_AlarmCount;
	m_nMaverickLot_WaferCount = nMaverickLot_WaferCount;
	m_strGenealogy_Granularity = strGenealogy_Granularity;
}

void GexDbPlugin_ER_Parts::Clear()
{
	m_uiNbAxis = 0;
	m_strField_Aggregate = "";
	m_strYieldQuery = "";
	m_strSplitCountQuery = "";
	m_strlFields_GraphSplit.clear();
	m_strlFields_LayerSplit.clear();
	m_bSoftBin = false;
	m_bWorkOnBinnings = false;
	m_plistSerieDefs.clear();
	m_uiSeries = 0;
	m_strBinlist_Pass = "";
	m_strBinlist_Fail = "";
	m_uiNbAxis = 0;
	m_strMaverickWafer_AlarmType = "";
	m_nMaverickWafer_AlarmCount = 0;
	m_nMaverickLot_WaferCount = 0;
	m_strGenealogy_Granularity = "";
	clear();
}
