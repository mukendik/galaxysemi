#include <QMap>
#include "gexdb_plugin_base.h"
#include <gqtl_log.h>

GexDbPlugin_Filter::GexDbPlugin_Filter(QObject* parent) //: QObject(parent)
{
    GSLOG(7, QString("new GexDbPlugin_Filter from parent %1").arg(parent?parent->objectName():"NULL").toLatin1().data() );
	Reset();
}

GexDbPlugin_Filter& GexDbPlugin_Filter::operator=(const GexDbPlugin_Filter & source)
{
    if (this != &source)
    {
        strlQueryFilters		= source.strlQueryFilters;
        iTimePeriod				= source.iTimePeriod;
        iTimeNFactor			= source.iTimeNFactor;
        m_eTimeStep				= source.m_eTimeStep;
        strDataTypeQuery		= source.strDataTypeQuery;
        calendarFrom			= source.calendarFrom;
        calendarTo				= source.calendarTo;
        calendarFrom_Time		= source.calendarFrom_Time;
        calendarTo_Time			= source.calendarTo_Time;
        tQueryFrom				= source.tQueryFrom;
        tQueryTo				= source.tQueryTo;
        bUseTimePeriod			= source.bUseTimePeriod;
        bConsolidatedExtraction	= source.bConsolidatedExtraction;
        strSiteFilterValue		= source.strSiteFilterValue;
        strTestList				= source.strTestList;
        strOptionsString		= source.strOptionsString;
        nNbQueryFields			= source.nNbQueryFields;
        mQueryFields            = source.mQueryFields;
        m_gexQueries            = source.m_gexQueries;
    }

	return *this;
}

void GexDbPlugin_Filter::Reset()
{
	strlQueryFilters.clear();
	iTimePeriod				= 8;					// corresponds to 'all_dates'
	iTimeNFactor			= 1;
	m_eTimeStep				= GexDbPlugin_Filter::WEEKS;
	strDataTypeQuery		= "";
	calendarFrom			= QDate::currentDate();
	calendarTo				= QDate::currentDate();
	calendarFrom_Time		= QTime(0, 0);
	calendarTo_Time			= QTime(23, 59, 59);
	tQueryFrom				= 0;
	tQueryTo				= 0;
	bUseTimePeriod			= false;
	bConsolidatedExtraction	= false;	// Fix Me : would it be true ?
	strSiteFilterValue		= "";
	strTestList				= "";
	strOptionsString		= "";
	nNbQueryFields			= 1;
	bConsolidatedData		= false;
	m_gexQueries.clear();
    mQueryFields.clear();
}

void GexDbPlugin_Filter::Reset(const QStringList & strlFilters)
{
	Reset();
	SetFilters(strlFilters);
}

void	GexDbPlugin_Filter::SetFilters(const QStringList & strlFilters)
{
	strlQueryFilters = strlFilters;
}

void	GexDbPlugin_Filter::SetFields(const QString& fieldName)
{
    mQueryFields = QStringList(fieldName);
}

void	GexDbPlugin_Filter::SetFields(const QStringList & fieldsName)
{
    mQueryFields = fieldsName;
}

QString GexDbPlugin_Filter::ContainsFilterOnTable(const QString& table_name, GexDbPlugin_Base* plugin, bool &b) const
{
    if (!plugin)
        return "error ";

    const GexDbPlugin_Mapping_FieldMap* mapFields_GexToRemote = plugin->GetMappingFieldsMap();
    if (!mapFields_GexToRemote)
        return "error";

    foreach(const QString &s, strlQueryFilters)
    {
        if (s.isEmpty())
            continue;
        QStringList	strlElements = s.split("=");
        if (strlElements.size()!=2)
            continue;
        QString		strField = strlElements[0];
        if(strField.isEmpty())
            continue;

        const GexDbPlugin_Mapping_FieldMap::const_iterator itMapping = mapFields_GexToRemote->find(strField);
        if(itMapping == mapFields_GexToRemote->end())
            continue;

        GexDbPlugin_Mapping_Field mf=itMapping.value();
        if (mf.m_strSqlTable != table_name)
            continue;
        b=true;
        return "ok";
    }
    b=false;
    return "ok";
}
