///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////

#include "browser_dialog.h"
#include "gex_advanced_enterprise_report.h"
#include "gex_advanced_enterprise_report_grt_parser.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "report_build.h"
#include "report_options.h"
#include "browser.h"
#include "gex_report.h"
#include "engine.h"
#include <gqtl_log.h>

extern CReportOptions	ReportOptions;
extern CGexReport*		gexReport;
extern const char *		gexTimePeriodChoices[];

///////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

///////////////////////////////////////////////////////////
// Statics members
///////////////////////////////////////////////////////////
int GexAdvancedEnterpriseReportSection::m_nGlobalSectionID = -1;

QScriptValue gexDecimalNumber(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue	svNumber(0);
    QScriptValue	svPrecision(2);
    QString			strPrecision;

    if (context->argumentCount() > 0)
        svNumber = context->argument(0);

    if (context->argumentCount() > 1)
        svPrecision	= context->argument(1);

    QTextStream(&strPrecision) << "%." << svPrecision.toInteger() << "f";

    return QScriptValue(engine, QString().sprintf(strPrecision.toLatin1().constData(), svNumber.toNumber()));
}

class PrivateCustomSettings
{
public:

    PrivateCustomSettings(const QString& strLabel, const QString& strJSValue)
        : m_strLabel(strLabel), m_strJSValue(strJSValue)
    { };

    QString		m_strLabel;
    QString		m_strJSValue;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportSection
//
// Description	:	Class which represents a section in a group of a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportSection::GexAdvancedEnterpriseReportSection()
{
    m_nSectionID = ++m_nGlobalSectionID;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportSection::~GexAdvancedEnterpriseReportSection()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportSection::setDatasetProperty(const GexAdvancedEnterpriseReportDataset *pDataset)
{
    m_aerScriptEngine.scriptEngine()->globalObject().setProperty("$PGroup" ,		QScriptValue(pDataset->settings().groups().join(",")));
    m_aerScriptEngine.scriptEngine()->globalObject().setProperty("$PSerie" ,		QScriptValue(pDataset->settings().series().join(",")));
    m_aerScriptEngine.scriptEngine()->globalObject().setProperty("$PAggregate" ,	QScriptValue(pDataset->settings().aggregates().join(",")));
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportSection::closeSection(const QString& strPPTName) const
//
// Description	:	Close section
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportSection::closeSection(const QString& strPPTName) const
{
    // If creating a Powerpoint presentation, save Section title.
    gexReport->SetPowerPointSlideName(strPPTName);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportSection::exportToCsv(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const
//
// Description	:	export section into csv stream
//
///////////////////////////////////////////////////////////////////////////////////
void
GexAdvancedEnterpriseReportSection::
exportToCsv(QTextStream& txtStream,
            const GexDbPluginERDatasetGroup& /*datasetGroup*/) const
{
    txtStream << "unable to export this type of section" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportSection::exportToSpreadsheet(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const
//
// Description	:	export into a stream
//
///////////////////////////////////////////////////////////////////////////////////
void
GexAdvancedEnterpriseReportSection::
exportToSpreadsheet(QTextStream& txtStream,
                    const GexDbPluginERDatasetGroup& /*datasetGroup*/) const
{
    txtStream << "unable to export this type of section" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportGroup
//
// Description	:	Class which represents a group in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportGroup::GexAdvancedEnterpriseReportGroup(GexAdvancedEnterpriseReportDataset * pDataset) : m_nBreakPage(1), m_pDataset(pDataset)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportGroup::~GexAdvancedEnterpriseReportGroup()
{
    while (m_lstSection.isEmpty() == false)
        delete m_lstSection.takeFirst();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::addSection(GexAdvancedEnterpriseReportSection * pSection)
//
// Description	:	add a section to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGroup::addSection(GexAdvancedEnterpriseReportSection * pSection)
{
    m_lstSection.append(pSection);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::addException(const QString& strName, const QString& strJSExpression)
//
// Description	:	add an exception to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGroup::addException(const QString& strName, const QString& strJSExpression)
{
    m_mapException.insert(strName, strJSExpression);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::addOrder(const QString &strOrder)
//
// Description	:	add a section to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGroup::addOrder(const QString &strOrder)
{
    m_lstOrder.append(strOrder);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::manageException(const GexDbPluginERDatasetGroup &datasetGroup)
//
// Description	:	Manage exceptions and add result to the script engine
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGroup::manageException(const GexDbPluginERDatasetGroup &datasetGroup)
{
    if (m_mapException.isEmpty() == false)
    {
        QMap<QString, bool>	mapExceptionResult;
        QMapIterator<QString, QString> it(m_mapException);
        it.toFront();
        while (it.hasNext())
        {
            it.next();

            mapExceptionResult.insert(it.key(), false);
        }

        GexAdvancedEnterpriseReportScriptEngine			tmpAERScriptEngine;
        QList<GexDbPluginERDatasetRow>::const_iterator	itRow = datasetGroup.rows().begin();

        while (itRow != datasetGroup.rows().end())
        {
            // Fill script variables for this group
            tmpAERScriptEngine.fillScriptGroupVariables(datasetGroup, (*itRow).aggregate());

            // Fill script variables for this serie
            int nSerieIndex = datasetGroup.indexOfSerie((*itRow).serie());
            tmpAERScriptEngine.fillScriptSerieVariables(datasetGroup.serieAt(nSerieIndex), (*itRow).aggregate());

            it.toFront();
            while (it.hasNext())
            {
                it.next();

                if (tmpAERScriptEngine.scriptEngine()->canEvaluate(it.value()))
                {
                    if (tmpAERScriptEngine.scriptEngine()->evaluate(it.value()).toBool())
                        mapExceptionResult[it.key()] = true;
                }
            }

            ++itRow;
        }

        QString		strJSProperty;
        QMapIterator<QString, bool> itResult(mapExceptionResult);
        itResult.toFront();
        while (itResult.hasNext())
        {
            itResult.next();

            strJSProperty = "$E" + itResult.key();
            m_aerScriptEngine.scriptEngine()->globalObject().setProperty(strJSProperty, itResult.value());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::exportToHtml(QTextStream& txtStream)
//
// Description	:	export sections of the group into an html file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGroup::exportToHtml(QTextStream& txtStream)
{
    int nSectionCount = 0;

    // Sort dataset results
    m_pDataset->sortResults();

    QString of=ReportOptions.GetOption("output", "format").toString();

    for (int nDatasetGroup = 0; nDatasetGroup < m_pDataset->results().groups().count(); ++nDatasetGroup)
    {
        // If exeption exists, add the value to the script engine
        manageException(m_pDataset->results().groups().at(nDatasetGroup));

        m_aerScriptEngine.fillScriptGroupVariables(m_pDataset->results().groups().at(nDatasetGroup));

        // Print when condition matches
        if(m_strJSPrintWhen.isEmpty() || m_aerScriptEngine.scriptEngine()->evaluate(m_strJSPrintWhen).toBool())
        {
            if (name().isEmpty() == false)
            {
                if (of == "CSV")
                    txtStream << m_aerScriptEngine.scriptEngine()->evaluate(name()).toString() << endl << endl;
                else
                    txtStream << "<font size=\"6\" face=\"Arial\"><b>" << m_aerScriptEngine.scriptEngine()->evaluate(name()).toString() << "</b></font><br>" << endl;
            }
            else if (m_pDataset->results().groups().at(nDatasetGroup).group().isEmpty() == false)
            {
                // Chart image or table is too high to fit in the first page with the PPT output format, Just add a page break before the first section.
                if (of == "PPT" && nDatasetGroup == 0)
                {
                    // If creating a Powerpoint presentation, save Section title.
                    gexReport->SetPowerPointSlideName("Quantix Reports Center");

                    // Add a page break
                    gexReport->WritePageBreak();
                }

                if (of != "CSV")
                    txtStream << "<font size=\"6\" face=\"Arial\"><b>";

                for (int nGroupSet = 0; nGroupSet < m_pDataset->settings().groups().count(); ++nGroupSet)
                {
                    if (nGroupSet != 0)
                        txtStream << " - ";

                     txtStream << m_pDataset->settings().groups().at(nGroupSet) << " " << m_pDataset->results().groups().at(nDatasetGroup).group().section(",", nGroupSet, nGroupSet);
                }

                if (of == "CSV")
                    txtStream << endl << endl;
                else
                    txtStream << "</b></font>" << "<br>" << endl;
            }

            for (int nSection = 0; nSection < m_lstSection.count(); ++nSection)
            {
                nSectionCount++;

                m_lstSection.at(nSection)->setDatasetProperty(m_pDataset);

                if (of=="CSV")
                {
                    m_lstSection.at(nSection)->exportToCsv(txtStream, m_pDataset->results().groups().at(nDatasetGroup));

                    txtStream << endl;
                }
                else
                {
                    m_lstSection.at(nSection)->exportToHtml(txtStream, m_pDataset->results().groups().at(nDatasetGroup));

                    if (of=="HTML"||of=="INTERACTIVE") // if (ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_HTML | GEX_OPTION_OUTPUT_INTERACTIVEONLY))
                        txtStream << "<br><br>" << endl;
                    else
                    {
                        if (nSectionCount >= m_nBreakPage)
                        {
                            // Add a page break
                            gexReport->WritePageBreak();
                            nSectionCount = 0;
                        }
                        else
                            txtStream << "<br>" << endl;
                    }
                }
            }
        }
    }

    // Add a page break
    if (nSectionCount > 0)
        gexReport->WritePageBreak();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexAdvancedEnterpriseReportGroup::exportToCSV(QTextStream& txtStream, int nSectionID, int nDatasetID)
//
// Description	:	export a section into a csv stream
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReportGroup::exportToCSV(QTextStream& txtStream, int nSectionID, int nDatasetID)
{
    // Looking for dataset group
    for (int nDatasetGroup = 0; nDatasetGroup < m_pDataset->results().groups().count(); ++nDatasetGroup)
    {
        // Dataset group found
        if (m_pDataset->results().groups().at(nDatasetGroup).groupId() == nDatasetID)
        {
            // Looking for section
            for (int nSection = 0; nSection < m_lstSection.count(); ++nSection)
            {
                if (m_lstSection.at(nSection)->sectionID() == nSectionID)
                {
                    m_lstSection.at(nSection)->exportToCsv(txtStream, m_pDataset->results().groups().at(nDatasetGroup));
                    return true;
                }
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::exportToSpreadSheet(QTextStream& txtStream, int nSectionID, int nDatasetID)
//
// Description	:	export a section into a stream
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReportGroup::exportToSpreadSheet(QTextStream& txtStream, int nSectionID, int nDatasetID)
{
    // Looking for dataset group
    for (int nDatasetGroup = 0; nDatasetGroup < m_pDataset->results().groups().count(); ++nDatasetGroup)
    {
        // Dataset group found
        if (m_pDataset->results().groups().at(nDatasetGroup).groupId() == nDatasetID)
        {
            // Looking for section
            for (int nSection = 0; nSection < m_lstSection.count(); ++nSection)
            {
                if (m_lstSection.at(nSection)->sectionID() == nSectionID)
                {
                    m_lstSection.at(nSection)->exportToSpreadsheet(txtStream, m_pDataset->results().groups().at(nDatasetGroup));
                    return true;
                }
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportDataset
//
// Description	:	Class which represents a dataset in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportDataset::GexAdvancedEnterpriseReportDataset(const GexDbPluginERDatasetSettings& datasetSettings, int nID) : m_bDataStatus(false), m_settings(datasetSettings), m_nID(nID)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportDataset::~GexAdvancedEnterpriseReportDataset()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::addSection(GexAdvancedEnterpriseReportSection * pSection)
//
// Description	:	add a section to the group
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportDataset::sortResults()
{
    m_results.sortGroups();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::addSection(GexAdvancedEnterpriseReportSection * pSection)
//
// Description	:	add a section to the group
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReportDataset::requestData()
{
    QTextStream txtStream(&m_strErrorMsg);

    // Retreive database handle
    GexDatabaseEntry * pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(m_settings.database());

    if(pDatabaseEntry == NULL)
    {
        // Failed finding database
        txtStream << "<b>*ERROR*</b> Failed finding database '" << m_settings.database().toLatin1().constData() << "'<br>" << endl;
        return m_bDataStatus;
    }

    // Check if the selected DB is a SQL DB
    if(!pDatabaseEntry->IsExternal())
    {
        // Not a SQL DB
        txtStream << "<b>*ERROR*</b> The database '" << m_settings.database().toLatin1().constData() << "' doesn't support SQL queries. Report can't be created<br>" << endl;
        return m_bDataStatus;
    }

    // Check external DB pointer
    if(!pDatabaseEntry->m_pExternalDatabase)
    {
        // Not a SQL DB
        txtStream << "<b>*ERROR*</b> Database pointer not available for database '" << m_settings.database().toLatin1().constData() << "%s'<br>" << endl;
        return m_bDataStatus;
    }

    if (pDatabaseEntry->m_pExternalDatabase->AER_GetDataset(m_settings, m_results) == false)
    {
        QString strErrorMsg;
        pDatabaseEntry->m_pExternalDatabase->GetLastError(strErrorMsg);
        strErrorMsg.replace("\n", "<br>");

        txtStream << "<b>*WARNING*</b>Failed retrieving data:<br><br>" << endl;
        txtStream << "Dataset=" << m_settings.name().toLatin1().constData() <<"<br>" << endl;
        txtStream << "Database=" << m_settings.database().toLatin1().constData() <<"<br>" << endl;
        txtStream << "Error=" << strErrorMsg.toLatin1().constData() << "<br>" << endl;

        return m_bDataStatus;
    }
    else
        m_bDataStatus = true;

    // Export Raw data
    if (m_settings.exportRawData())
        exportRawData(pDatabaseEntry);

    return m_bDataStatus;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString GexAdvancedEnterpriseReportDataset::rawDataFilename() const
//
// Description	:	Build the raw data filename
//
///////////////////////////////////////////////////////////////////////////////////
QString GexAdvancedEnterpriseReportDataset::rawDataFilename() const
{
    QString strExportFile = ReportOptions.strReportDirectory + "/pages/" + m_settings.name() + QString("_%1.csv").arg(m_nID);

    return strExportFile;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportGroup::exportRawData(GexDatabaseEntry * pDatabaseEntry) const
//
// Description	:	Export raw data in a csv file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportDataset::exportRawData(GexDatabaseEntry * pDatabaseEntry) const
{
    if (m_results.groups().count())
    {
        QFile queryFile(rawDataFilename());

        if(queryFile.open(QIODevice::WriteOnly))
        {
            QTextStream stream(&queryFile);

            // Header
            stream << "# Semiconductor Yield Analysis is easy with Quantix!" << endl;
            stream << "# Check latest news: www.mentor.com" << endl;
            stream << "#" << endl;
            stream << "# Examinator Reports Center raw data" << endl;
            stream << "#" << endl;
            stream << "# Report created with: " << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << " - www.mentor.com" << endl;
            stream << endl;

            // Global information
            stream << "# Global information" << endl;
            stream << "Date," << QDate::currentDate().toString("dd MMM yyyy") << endl;
            stream << "Time," << QTime::currentTime().toString("hh:mm:ss") << endl;
            stream << "Report," << m_settings.report() << endl;
            stream << endl;

            // Dataset filters
            stream << "# Dataset Filters" << endl;

            // Period
            if(m_settings.timePeriod() == GEX_QUERY_TIMEPERIOD_CALENDAR)
                stream << "Time Period," << "From " << m_settings.dateFrom().toString("dd MMM yyyy") << " To " << m_settings.dateTo().toString("dd MMM yyyy") << endl;
            else if(m_settings.timePeriod() == GEX_QUERY_TIMEPERIOD_LAST_N_X)
                stream << "Time Period," << "Last " << m_settings.timeNFactor() << " " << m_settings.timeStep() << endl;
            else
                stream << "Time Period," << gexTimePeriodChoices[m_settings.timePeriod()] << endl;

            if (m_settings.binType() != GexDbPluginERDatasetSettings::NoBin)
            {
                if(m_settings.binType() == GexDbPluginERDatasetSettings::HardBin)
                    stream << "Binning," << "Hard Bin" << endl;
                else
                    stream << "Binning," << "Soft Bin" << endl;
            }

            // Serie
            if (m_settings.series().count() > 0)
            {
                QString strFilter;

                for (int nSerie = 0; nSerie < m_settings.series().count(); ++nSerie)
                {
                    if (nSerie == 0)
                        stream << "Serie,";
                    else
                        stream << ",";

                    if (m_settings.isFilteredField(m_settings.series().at(nSerie), strFilter))
                        stream << strFilter << endl;
                    else
                        stream << m_settings.series().at(nSerie) << endl;
                }
            }

            // Additionnal filters
            if (m_settings.filters().count() > 0)
            {
                bool bFirstFilter = true;
                for (int nFilter = 0; nFilter < m_settings.filters().count(); ++nFilter)
                {
                    if (m_settings.isSerieField(m_settings.filters().at(nFilter).field()) == false)
                    {
                        if (bFirstFilter)
                            stream << "Filter,";
                        else
                            stream << ",";

                        stream << m_settings.filters().at(nFilter).toString() << endl;
                        bFirstFilter = false;
                    }
                }
            }
            stream << endl;

            // Raw data section
            stream << "# Raw Data" << endl;

            for (int nGroup = 0; nGroup < m_settings.groups().count(); nGroup++)
                stream << m_settings.groups().at(nGroup) << ",";

            for (int nAggregate = 0; nAggregate < m_settings.aggregates().count(); nAggregate++)
                stream << m_settings.aggregates().at(nAggregate) << ",";

            for (int nSerie = 0; nSerie < m_settings.series().count(); nSerie++)
                stream << m_settings.series().at(nSerie) << ",";

            if (pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(m_settings.testingStage()))
            {
                if (m_settings.binType() != GexDbPluginERDatasetSettings::NoBin)
                    stream << "Bin#, Bin Cat, Bin Name, Total parts, Bin parts, Bin Yield (%)" << endl;
                else
                    stream << "Total parts, Good parts, Yield (%)" << endl;
            }
            else
            {
                if (m_settings.binType() != GexDbPluginERDatasetSettings::NoBin)
                    stream << "Bin#, Bin Cat, Bin Name, Total parts, Gross Die, Bin parts, Bin Yield (%), Gross Bin Yield (%)" << endl;
                else
                    stream << "Total parts, Gross Die, Good parts, Yield (%), Gross Yield (%)" << endl;
            }

            QList<GexDbPluginERDatasetGroup>::const_iterator		itGroup		= m_results.groups().begin();
            QList<GexDbPluginERDatasetRow>::const_iterator			itRow;

            while (itGroup != m_results.groups().end())
            {
                itRow = (*itGroup).rows().begin();

                while (itRow != (*itGroup).rows().end())
                {
                    if (m_settings.groups().count() > 0)
                        stream << (*itGroup).group() << ",";

                    if (m_settings.aggregates().count() > 0)
                        stream << (*itRow).aggregate() << ",";

                    if (m_settings.series().count() > 0)
                        stream << (*itRow).serie() << ",";

                    if (pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(m_settings.testingStage()))
                    {
                        if (m_settings.binType() != GexDbPluginERDatasetSettings::NoBin)
                            stream << (*itRow).field("bin_no").value().toString() << "," << (*itRow).field("bin_cat").value().toString() << "," << (*itRow).field("bin_name").value().toString() << "," << (*itRow).volume() << "," << (*itRow).binVolume() << "," << (*itRow).binYield() << endl;
                        else
                            stream << (*itRow).volume() << "," << "," << (*itRow).goodVolume() << "," << (*itRow).yield() << "," << endl;
                    }
                    else
                    {
                        if (m_settings.binType() != GexDbPluginERDatasetSettings::NoBin)
                            stream << (*itRow).field("bin_no").value().toString() << "," << (*itRow).field("bin_cat").value().toString() << "," << (*itRow).field("bin_name").value().toString() << "," << (*itRow).volume() << "," << (*itRow).grossVolume() << "," << (*itRow).binVolume() << "," << (*itRow).binYield() << "," << (*itRow).grossBinYield() << endl;
                        else
                            stream << (*itRow).volume() << "," << (*itRow).grossVolume() << "," << (*itRow).goodVolume() << "," << (*itRow).yield() << "," << (*itRow).grossYield() << endl;
                    }

                    ++itRow;
                }

                ++itGroup;
            }

            queryFile.close();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReport
//
// Description	:	Class which contains information about a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReport::GexAdvancedEnterpriseReport() : m_eDataStatus(dataNotRequested), m_nRequestTime(0)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReport::~GexAdvancedEnterpriseReport()
{
    while(m_lstDataset.isEmpty() == false)
        delete m_lstDataset.takeFirst();

    while(m_lstGroup.isEmpty() == false)
        delete m_lstGroup.takeFirst();

    while(m_lstCustomSettings.isEmpty() == false)
        delete m_lstCustomSettings.takeFirst();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const GexAdvancedEnterpriseReportDataset * GexAdvancedEnterpriseReport::addDataset(const GexDbPluginERDatasetSettings &m_settings)
//
// Description	:	add a dataset to the report
//
///////////////////////////////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportDataset * GexAdvancedEnterpriseReport::addDataset(const GexDbPluginERDatasetSettings &m_settings)
{
    GexAdvancedEnterpriseReportDataset * pDataset = new GexAdvancedEnterpriseReportDataset(m_settings, m_lstDataset.count());

    m_lstDataset.append(pDataset);

    return pDataset;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReport::addGroup(GexAdvancedEnterpriseReportGroup *pGroup)
//
// Description	:	add a dataset to the report
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReport::addGroup(GexAdvancedEnterpriseReportGroup *pGroup)
{
    pGroup->setGroupID(m_lstGroup.count());
    m_lstGroup.append(pGroup);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReport::addCustomSettings(const QString& strLabel, const QString& strValue)
//
// Description	:	add a new custom settings line
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReport::addCustomSettings(const QString& strLabel, const QString& strValue)
{
    m_lstCustomSettings.append(new PrivateCustomSettings(strLabel, strValue));
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReport::load(const QString& strAERFile)
//
// Description	:	Load report settings
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReport::load(const QString& strAERFile)
{
    bool r=GexAbstractAdvancedEnterpriseReportGrtParser::load(*this, strAERFile, m_strErrorMsg);
    if (!r)
      GSLOG(4, "GexAbstractAdvancedEnterpriseReportGrtParser load failed");
    return r;
}

void GexAdvancedEnterpriseReport::requestData()
{
    bool bRequestOk = true;

    // Request Data
    for (int nItem = 0; nItem < m_lstDataset.count(); nItem++)
        bRequestOk &= m_lstDataset.at(nItem)->requestData();

    if (bRequestOk)
        m_eDataStatus = dataRequested;
    else
        m_eDataStatus = dataError;

    m_nRequestTime = gexReport->processTime().elapsed();					// compute elapsed time to create report
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexAdvancedEnterpriseReport::generate(QTextStream& txtStream)
//
// Description	:	add a group to the report
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReport::generate(QTextStream& txtStream)
{
    bool		bRequestOk = true;

    if (m_strErrorMsg.isEmpty())
    {
        // Request Data
        if (m_eDataStatus == dataNotRequested)
            requestData();

        // Write report header
        writeHeader(txtStream);

        QString of=ReportOptions.GetOption("output", "format").toString();
        if (m_eDataStatus == dataRequested)
        {
            if (m_lstTitle.count() > 0)
            {
                GexReportTable titleTable;

                titleTable.setBorder(0);
                titleTable.setWidth(HTML_TABLE_WIDTH);
                titleTable.setCellPadding(0);
                titleTable.setCellSpacing(1);

                QScriptEngine   scriptEngine;
                QString         strTitle;

                for (int nTitle = 0; nTitle < m_lstTitle.count(); ++nTitle)
                {
                    GexReportTableLine titleTableLine;
                    GexReportTableCell titleTableCell;
                    GexReportTableText titleTableText;

                    titleTableText.setSize(6);
                    titleTableText.setFontStyle(GexReportTableText::BoldStyle);

                    strTitle = scriptEngine.evaluate(m_lstTitle.at(nTitle)).toString();

                    if (scriptEngine.hasUncaughtException())
                        strTitle = m_lstTitle.at(nTitle);

                    titleTableText.setText(strTitle);

                    titleTableCell.setAlign("center");
                    titleTableCell.addData(titleTableText);

                    titleTableLine.addCell(titleTableCell);
                    titleTable.addLine(titleTableLine);
                }

                if (of=="CSV")
                    titleTable.toOutput(txtStream, GexReportTable::CsvOutput);
                else
                    titleTable.toOutput(txtStream, GexReportTable::HtmlOutput);

                //if (ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_HTML | GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_PDF))
                if ( (of=="HTML")||(of=="INTERACTIVE")||(of=="PDF") )
                    txtStream << "<br>" << endl;
            }

            // Write settings to describe dataset
            writeMinimalSettings(txtStream);

            //if (ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_HTML | GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_PDF))
            if ( (of=="HTML")||(of=="INTERACTIVE")||(of=="PDF") )
                txtStream << "<br>" << endl;
            else if (of == "CSV")
                txtStream << endl;

            for(int nGroup = 0; nGroup < m_lstGroup.count(); nGroup++)
                m_lstGroup.at(nGroup)->exportToHtml(txtStream);

            if (m_lstSummary.count() > 0)
            {
                for (int nSummary = 0; nSummary < m_lstSummary.count(); ++nSummary)
                    txtStream << m_lstSummary.at(nSummary) << "<br>" << endl;
            }
        }
        else
        {
            for (int nDataset = 0; nDataset < m_lstDataset.count(); nDataset++)
            {
                if (m_lstDataset.at(nDataset)->dataStatus() == false)
                    txtStream << m_lstDataset.at(nDataset)->errorMessage() << endl;
            }
        }
    }
    else
    {
        // Failed to open grt file
        txtStream << "<b>*ERROR*</b> '" << m_strErrorMsg << "'<br>" << endl;
        bRequestOk = false;
    }

    return bRequestOk;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexAdvancedEnterpriseReport::exportToCsv(QTextStream& txtStream, int nSectionID, int nDatasetID)
//
// Description	:	Export a section to a csv file
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReport::exportToCsv(QTextStream& txtStream, int nSectionID, int nDatasetID)
{
    bool bExported = false;

    for(int nGroup = 0; nGroup < m_lstGroup.count() && bExported == false; nGroup++)
        bExported = m_lstGroup.at(nGroup)->exportToCSV(txtStream, nSectionID, nDatasetID);

    return bExported;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexAdvancedEnterpriseReport::exportToSpreadSheet(QTextStream& txtStream, int nSectionID, int nDatasetID)
//
// Description	:	Export a section to a spreadsheet
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAdvancedEnterpriseReport::exportToSpreadSheet(QTextStream& txtStream, int nSectionID, int nDatasetID)
{
    bool bExported = false;

    for(int nGroup = 0; nGroup < m_lstGroup.count() && bExported == false; nGroup++)
        bExported = m_lstGroup.at(nGroup)->exportToSpreadSheet(txtStream, nSectionID, nDatasetID);

    return bExported;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReport::writeSettings(QTextStream& txtStream)
//
// Description	:	Write the html settings
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReport::writeSettings(QTextStream& txtStream)
{
    QString of=ReportOptions.GetOption("output", "format").toString();
    txtStream << "<table border=\"1\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"1\" cellpadding=\"1\">" << endl;

    for (int nItem = 0; nItem < m_lstDataset.count(); nItem++)
    {
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Dataset</td>" << endl;
        txtStream << "<td width=\"80%%\" bgcolor=\"#F8F8F8\">" << endl;
        txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Database</td>" << endl;
        txtStream << "<td width=\"2%%\" bgcolor=\"#F8F8F8\">:" << "</td>" << endl;
        txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().database() << "</td>" << endl;
        txtStream << "</tr>" << endl;
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"20%%\" bgcolor=\"#F8F8F8\">DataType</td>" << endl;
        txtStream << "<td width=\"2%%\" bgcolor=\"#F8F8F8\">:" << "</td>" << endl;
        txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().testingStage() << "</td>" << endl ;
        txtStream << "</tr>" << endl;
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"20%%\" bgcolor=\"#F8F8F8\">TimePeriod</td>" << endl;
        txtStream << "<td width=\"2%%\" bgcolor=\"#F8F8F8\">:" << "</td>" << endl;
        if(m_lstDataset.at(nItem)->settings().timePeriod() == GEX_QUERY_TIMEPERIOD_LAST_N_X)
            txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">Last " << m_lstDataset.at(nItem)->settings().timeNFactor() << " " << m_lstDataset.at(nItem)->settings().timeStep() << "</td>" << endl;
        else
            txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << gexTimePeriodChoices[m_lstDataset.at(nItem)->settings().timePeriod()]<< "</td>" << endl;
        txtStream << "</tr>" << endl;

        if(m_lstDataset.at(nItem)->settings().timePeriod() == GEX_QUERY_TIMEPERIOD_CALENDAR)
        {
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"20%%\" bgcolor=\"#F8F8F8\">From</td>" << endl;
            txtStream << "<td width=\"2%%\" bgcolor=\"#F8F8F8\">:" << "</td>" << endl;
            txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().dateFrom().toString("dd MMM yyyy") <<" </td>" << endl;
            txtStream << "</tr>" << endl;
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"20%%\" bgcolor=\"#F8F8F8\">To</td>" << endl;
            txtStream << "<td width=\"2%%\" bgcolor=\"#F8F8F8\">:" << "</td>" << endl;
            txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().dateTo().toString("dd MMM yyyy") << "</td>" << endl;
            txtStream << "</tr>" << endl;
        }

        for (int nFilter = 0; nFilter < m_lstDataset.at(nItem)->settings().filters().count(); ++nFilter)
        {
            txtStream << "<tr>" << endl;
            txtStream << "<td vAlign=\"top\" width=\"20%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().filters().at(nFilter).field() << " </td>" << endl;
            if (m_lstDataset.at(0)->settings().filters().at(nFilter).filterOperator() == GexDbPluginERDatasetSettings::OpEqual)
            {
                txtStream << "<td vAlign=\"top\" width=\"2%%\" bgcolor=\"#F8F8F8\">=" << "</td>" << endl;
                txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << QString(m_lstDataset.at(nItem)->settings().filters().at(nFilter).value()).replace("|", " or ") << "</td>" << endl;
            }
            else
            {
                txtStream << "<td vAlign=\"top\" width=\"2%%\" bgcolor=\"#F8F8F8\">&ne;" << "</td>" << endl;
                txtStream << "<td width=\"78%%\" bgcolor=\"#F8F8F8\">" << QString(m_lstDataset.at(nItem)->settings().filters().at(nFilter).value()).replace("|", " and ") << "</td>" << endl;
            }
            txtStream << "</tr>" << endl;
        }

        txtStream << "</table>" << endl;
        txtStream << "</td>" << endl;
        txtStream << "</tr>" << endl;

        if (m_lstDataset.at(nItem)->settings().groups().count() > 0)
        {
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Split</td>" << endl;
            txtStream << "<td width=\"80%%\" bgcolor=\"#F8F8F8\">" << endl;
            txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"100%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().groups().join(",") << "</td>" << endl;
            txtStream << "</tr>" << endl;
            txtStream << "</table>" << endl;
            txtStream << "</td>" << endl;
            txtStream << "</tr>" << endl;
        }

        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Aggregate</td>" << endl;
        txtStream << "<td width=\"80%%\" bgcolor=\"#F8F8F8\">" << endl;
        txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"100%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().aggregates().join(",") << "</td>" << endl;
        txtStream << "</tr>" << endl;
        txtStream << "</table>" << endl;
        txtStream << "</td>" << endl;
        txtStream << "</tr>" << endl;

        if (m_lstDataset.at(nItem)->settings().series().count() > 0)
        {
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Serie</td>" << endl;
            txtStream << "<td width=\"80%%\" bgcolor=\"#F8F8F8\">" << endl;
            txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"100%%\" bgcolor=\"#F8F8F8\">" << m_lstDataset.at(nItem)->settings().series().join(",") << "</td>" << endl;
            txtStream << "</tr>" << endl;
            txtStream << "</table>" << endl;
            txtStream << "</td>" << endl;
            txtStream << "</tr>" << endl;
        }

        if (m_lstDataset.at(nItem)->settings().binType() != GexDbPluginERDatasetSettings::NoBin)
        {
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Bin type</td>" << endl;

            if(m_lstDataset.at(nItem)->settings().binType() == GexDbPluginERDatasetSettings::HardBin)
                txtStream <<  "<td width=\"80%%\" bgcolor=\"#F8F8F8\">Hard Bin</td>" << endl;
            else
                txtStream << "<td width=\"80%%\" bgcolor=\"#F8F8F8\">Soft Bin</td>" << endl;

            txtStream << "</tr>" << endl;
        }
    }

    txtStream << "</table><br>" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReport::writeMinimalSettings(QTextStream& txtStream)
//
// Description	:	Write the html settings with minimal information
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReport::writeMinimalSettings(QTextStream& txtStream)
{
    QString of=ReportOptions.GetOption("output", "format").toString();

    if (of == "CSV")
    {
        // Limit to the first dataset for now
        for (int nItem = 0; nItem < m_lstDataset.count() && nItem < 1; nItem++)
        {
            txtStream << "TimePeriod :" << ",";

            if(m_lstDataset.at(nItem)->settings().timePeriod() == GEX_QUERY_TIMEPERIOD_CALENDAR)
                txtStream << "From " << m_lstDataset.at(nItem)->settings().dateFrom().toString("dd MMM yyyy") << " To " << m_lstDataset.at(nItem)->settings().dateTo().toString("dd MMM yyyy") << endl;
            else if(m_lstDataset.at(nItem)->settings().timePeriod() == GEX_QUERY_TIMEPERIOD_LAST_N_X)
            {
                txtStream << "Last " << m_lstDataset.at(nItem)->settings().timeNFactor() << " " << m_lstDataset.at(nItem)->settings().timeStep() << endl;
            }
            else
                txtStream << gexTimePeriodChoices[m_lstDataset.at(nItem)->settings().timePeriod()] << endl;

            if (m_lstDataset.at(nItem)->settings().binType() != GexDbPluginERDatasetSettings::NoBin)
            {
                txtStream << "Binning :" << ",";

                if(m_lstDataset.at(nItem)->settings().binType() == GexDbPluginERDatasetSettings::HardBin)
                    txtStream << "Hard Bin" << endl;
                else
                    txtStream << "Soft Bin" << endl;
            }

            if (m_lstDataset.at(nItem)->settings().series().count() > 0)
            {
                QString strFilter;

                for (int nSerie = 0; nSerie < m_lstDataset.at(nItem)->settings().series().count(); ++nSerie)
                {
                    if (nSerie == 0)
                        txtStream << "Serie :" << ",";
                    else
                        txtStream << ",";

                    if (m_lstDataset.at(nItem)->settings().isFilteredField(m_lstDataset.at(nItem)->settings().series().at(nSerie), strFilter))
                        txtStream << strFilter << endl;
                    else
                        txtStream << m_lstDataset.at(nItem)->settings().series().at(nSerie) << endl;
                }
            }

            if (m_lstDataset.at(nItem)->settings().filters().count() > 0)
            {
                bool bFirstFilter = true;
                for (int nFilter = 0; nFilter < m_lstDataset.at(nItem)->settings().filters().count(); ++nFilter)
                {
                    if (m_lstDataset.at(nItem)->settings().isSerieField(m_lstDataset.at(nItem)->settings().filters().at(nFilter).field()) == false)
                    {
                        if (bFirstFilter)
                            txtStream << "Filter :" << ",";
                        else
                            txtStream << ",";

                        txtStream <<  m_lstDataset.at(nItem)->settings().filters().at(nFilter).toString() << endl;

                        bFirstFilter = false;
                    }
                }
            }

            for (int nCustom = 0; nCustom < m_lstCustomSettings.count(); ++nCustom)
            {
                QScriptEngine scriptEngine;
                QString strValue;

                if (scriptEngine.canEvaluate(m_lstCustomSettings.at(nCustom)->m_strJSValue))
                    strValue = scriptEngine.evaluate(m_lstCustomSettings.at(nCustom)->m_strJSValue).toString();
                else
                    strValue = m_lstCustomSettings.at(nCustom)->m_strJSValue;

                txtStream << m_lstCustomSettings.at(nCustom)->m_strLabel << ",";
                txtStream << strValue << endl;
            }

            txtStream << endl;
        }
    }
    else
    {
        txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"1\" cellpadding=\"1\">" << endl;

        // Limit to the first dataset for now
        for (int nItem = 0; nItem < m_lstDataset.count() && nItem < 1; nItem++)
        {
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"30%%\" bgcolor=\"#FFFFFF\"></td>" << endl;
            txtStream << "<td width=\"70%%\" bgcolor=\"#FFFFFF\">" << endl;
            txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
            txtStream << "<tr>" << endl;

            txtStream << "<td width=\"20%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">TimePeriod : </font></td>" << endl;

            if(m_lstDataset.at(nItem)->settings().timePeriod() == GEX_QUERY_TIMEPERIOD_CALENDAR)
            {
                txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">From " << m_lstDataset.at(nItem)->settings().dateFrom().toString("dd MMM yyyy") << " To " << m_lstDataset.at(nItem)->settings().dateTo().toString("dd MMM yyyy") << "</font></td>" << endl;
            }
            else if(m_lstDataset.at(nItem)->settings().timePeriod() == GEX_QUERY_TIMEPERIOD_LAST_N_X)
                txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">Last " << m_lstDataset.at(nItem)->settings().timeNFactor() << " " << m_lstDataset.at(nItem)->settings().timeStep() << "</font></td>" << endl;
            else
                txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">" << gexTimePeriodChoices[m_lstDataset.at(nItem)->settings().timePeriod()] << "</font></td>" << endl;

            txtStream << "</tr>" << endl;
            txtStream << "</table>" << endl;
            txtStream << "</td>" << endl;
            txtStream << "</tr>" << endl;

            if (m_lstDataset.at(nItem)->settings().binType() != GexDbPluginERDatasetSettings::NoBin)
            {
                txtStream << "<tr>" << endl;
                txtStream << "<td width=\"30%%\" bgcolor=\"#FFFFFF\"></td>" << endl;
                txtStream << "<td width=\"70%%\" bgcolor=\"#FFFFFF\">" << endl;
                txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;
                txtStream << "<tr>" << endl;

                txtStream << "<td width=\"20%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">Binning :" << "</font></td>" << endl;

                if(m_lstDataset.at(nItem)->settings().binType() == GexDbPluginERDatasetSettings::HardBin)
                    txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">Hard Bin</font></td>" << endl;
                else
                    txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">Soft Bin</font></td>" << endl;

                txtStream << "</tr>" << endl;
                txtStream << "</table>" << endl;
                txtStream << "</td>" << endl;
                txtStream << "</tr>" << endl;
            }

            if (m_lstDataset.at(nItem)->settings().series().count() > 0)
            {
                QString strFilter;

                txtStream << "<tr>" << endl;
                txtStream << "<td width=\"30%%\" bgcolor=\"#FFFFFF\"></td>" << endl;
                txtStream << "<td width=\"70%%\" bgcolor=\"#FFFFFF\">" << endl;
                txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;

                for (int nSerie = 0; nSerie < m_lstDataset.at(nItem)->settings().series().count(); ++nSerie)
                {
                    txtStream << "<tr>" << endl;

                    if (nSerie == 0)
                        txtStream << "<td vAlign=\"top\" width=\"20%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">Serie :" << "</font></td>" << endl;
                    else
                        txtStream << "<td vAlign=\"top\" width=\"20%%\" bgcolor=\"#FFFFFF\">" << "</td>" << endl;

                    if (m_lstDataset.at(nItem)->settings().isFilteredField(m_lstDataset.at(nItem)->settings().series().at(nSerie), strFilter))
                        txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">" << strFilter << "</font></td>" << endl;
                    else
                        txtStream << "<td align=\"left\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">" << m_lstDataset.at(nItem)->settings().series().at(nSerie) << "</font></td>" << endl;

                    txtStream << "</tr>" << endl;
                }

                txtStream << "</table>" << endl;
                txtStream << "</td>" << endl;
                txtStream << "</tr>" << endl;
            }

            if (m_lstDataset.at(nItem)->settings().filters().count() > 0)
            {
                txtStream << "<tr>" << endl;
                txtStream << "<td width=\"30%%\" bgcolor=\"#FFFFFF\"></td>" << endl;
                txtStream << "<td width=\"70%%\" bgcolor=\"#FFFFFF\">" << endl;
                txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;

                bool bFirstFilter = true;
                for (int nFilter = 0; nFilter < m_lstDataset.at(nItem)->settings().filters().count(); ++nFilter)
                {
                    if (m_lstDataset.at(nItem)->settings().isSerieField(m_lstDataset.at(nItem)->settings().filters().at(nFilter).field()) == false)
                    {

                        if (bFirstFilter)
                            txtStream << "<td vAlign=\"top\" width=\"20%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">Filter :" << "</font></td>" << endl;
                        else
                            txtStream << "<td vAlign=\"top\" width=\"20%%\" bgcolor=\"#FFFFFF\">" << "</td>" << endl;

                        txtStream << "<td vAlign=\"top\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">" << m_lstDataset.at(nItem)->settings().filters().at(nFilter).toString() << "</font></td>" << endl;
                        txtStream << "</tr>" << endl;

                        bFirstFilter = false;
                    }
                }

                txtStream << "</table>" << endl;
                txtStream << "</td>" << endl;
                txtStream << "</tr>" << endl;
            }

            if (m_lstCustomSettings.count())
            {
                QScriptEngine	scriptEngine;
                QString			strValue;

                txtStream << "<tr>" << endl;
                txtStream << "<td width=\"30%%\" bgcolor=\"#FFFFFF\"></td>" << endl;
                txtStream << "<td width=\"70%%\" bgcolor=\"#FFFFFF\">" << endl;
                txtStream << "<table border=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellspacing=\"0\" cellpadding=\"0\">" << endl;

                for (int nCustom = 0; nCustom < m_lstCustomSettings.count(); ++nCustom)
                {
                    txtStream << "<td vAlign=\"top\" width=\"20%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">" << m_lstCustomSettings.at(nCustom)->m_strLabel << "</font></td>" << endl;

                    if (scriptEngine.canEvaluate(m_lstCustomSettings.at(nCustom)->m_strJSValue))
                        strValue = scriptEngine.evaluate(m_lstCustomSettings.at(nCustom)->m_strJSValue).toString();
                    else
                        strValue = m_lstCustomSettings.at(nCustom)->m_strJSValue;

                    txtStream << "<td vAlign=\"top\" width=\"80%%\" bgcolor=\"#FFFFFF\"><font size=\"3\">" << strValue << "</font></td>" << endl;
                }

                txtStream << "</table>" << endl;
                txtStream << "</td>" << endl;
                txtStream << "</tr>" << endl;
            }
        }

        txtStream << "</table>" << endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReport::writeHeader(QTextStream& txtStream)
//
// Description	:	Write the html header
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReport::writeHeader(QTextStream& txtStream)
{
    // Table of content is only for HTML based files (HTML, WORD, PDF, etc...)
    QString of=ReportOptions.GetOption("output", "format").toString();
    //if (ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if (ReportOptions.isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        QString strTitle = "Quantix Reports";

        // Set Slide Title
        gexReport->SetPowerPointSlideName(strTitle);

        // Open table to fit all the Table of Content text
        txtStream << "<table border=\"0\" cellspacing=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" style=\"font-size: " << gexReport->iHthmSmallFontSizePixels << "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">" << endl;

        txtStream << "<tr>" << endl;
        txtStream << "<td align=";

        // If PPT file, center all text in the table, otherwise, left justified
        if (of=="PPT") //if (ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_PPT)
            txtStream << "\"center\">" << endl;
        else
            txtStream << "\"left\">" << endl;

        // Write slide title (900 pixel wide)
        txtStream << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"" << HTML_TABLE_WIDTH << "\" bgcolor=\"#F8F8F8\">" << endl;
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"35%%\"><img border=\"0\" src=\"../images/quantix.png\"></td>" << endl;
        txtStream << "<td width=\"65%%\" align=\"right\"><b><font face=\"Times New Roman\" size=\"6\" color=\"#0070C0\">Welcome to " << strTitle << " !</font></b></td>" << endl;
        txtStream << "</tr>" << endl;
        txtStream << "</table>" << endl;

        //if(ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
        if ( (of=="INTERACTIVE")||(of=="HTML") )
        {
            // Sets default background color = white, text color given in argument.
            time_t iTime = time(NULL);
            txtStream << "<align=\"left\"><font color=\"#000000\" size=\"" << gexReport->iHthmNormalFontSize << "\"><b>Date: " << ctime(&iTime) << "</b><br></font>" << endl;

            // Write Examinator release (except if PDF file which already writes each page header with this string)
            if (of!="PDF") //if(ReportOptions.iOutputFormat != GEX_OPTION_OUTPUT_PDF)
                txtStream << "<align=\"left\"><font color=\"#000000\" size=\""
                          << gexReport->iHthmNormalFontSize
                          << "\"><b>Report created with: </b>"
                          << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
                          << " - www.mentor.com<br></font>" << endl;

            // Wtrite Data processing duration
            char	szString[256];
            gexReport->HMSString(m_nRequestTime / 1000, m_nRequestTime % 1000, szString);	// convert processing time (in seconds) to string
            txtStream << "<align=\"left\"><font color=\"#000000\" size=\"" << gexReport->iHthmNormalFontSize << "\"><b>Data processed in: </b>" << szString << "<br><br><br></font>" << endl;
        }

        //******** User Template Text + Logo (if defined....)
        bool bUserLogo = QFile::exists(m_strFrontPageImage);
        bool bUserText = !m_strFrontPageText.isEmpty();

        if (bUserLogo || bUserText)
        {
            // HTML code to open table, Width 98%, cell spacing=0
            txtStream << "<table border=\"0\" cellspacing=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" style=\"font-size: " << gexReport->iHthmSmallFontSizePixels << "pt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">" << endl;
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"20%%\" align=\"left\">" << endl;

            // Logo exists...write it!
            if(bUserLogo)
                txtStream << "<img border=\"0\" src=\"" << formatHtmlImageFilename(m_strFrontPageImage) << "\" width=\"20%%\">" << endl;

            txtStream << "</td>" << endl;

            // Empty separator cell to have some space between the logo & the text...
            txtStream << "<td width=\"3%%\" align=\"left\"> </td>" << endl;

            // Text
            txtStream << "<td width=\"77%%\" align=\"left\">" << endl;

            // Text exists...write it!
            if(bUserText)
                txtStream << "<align=\"left\" valign=\"top\"><font color=\"#000000\" size=\"" << gexReport->iHthmSmallFontSize << "\">" << m_strFrontPageText << "</font>" << endl;

            //******** Close Template table
            txtStream << "</td>" << endl;
            txtStream << "</tr>" << endl;
            txtStream << "</table>" << endl;
        }

        txtStream << "<br>" << endl;

        // Next table cell
        txtStream << "</td>" << endl;
        txtStream << "</tr>\n" << endl;

        // Add a save button for Enterprise Report
        //if(ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
        if ( (of=="INTERACTIVE")||(of=="HTML") )
        {
            QString strReference = QString("%1%2--action=modify_params").arg(GEX_BROWSER_ACTIONBOOKMARK).arg(GEX_BROWSER_ACT_ADV_ENTERPRISE);

            txtStream << "<table border=\"0\" cellspacing=\"0\" width=\"" << HTML_TABLE_WIDTH << "\" cellpadding=\"0\">" << endl;
            txtStream << "<tr>" << endl;
            txtStream << "<td width=\"70%%\" align=left>";
            txtStream << "<p><a href=\"_gex_save.htm\"><img src=\"../images/save_icon.png\" border=\"0\" width=\"22\" height=\"19\"></a>";
            txtStream << "<a href=\"_gex_save.htm\">Save :</a> this report's script for ";
            txtStream << "future playback or auto-report under Yield-Man!</p>";
            txtStream << "</td>" << endl;
            txtStream << "<td width=\"30%%\" align=right height=\"19\">" << endl;
            txtStream << "<p><a href=\"" << strReference << "\">Report options...</a>" << endl ;
            txtStream << "<a href=\"" << strReference << "\"><img border=\"0\" src=\"../images/report_properties.png\" border=\"0\"></a></p>";
            txtStream << "</td>" << endl;
            txtStream << "</tr>" << endl;

            for (int nDataset = 0; nDataset < m_lstDataset.count(); nDataset++)
            {
                if (m_lstDataset.at(nDataset)->settings().exportRawData() && m_lstDataset.at(nDataset)->results().groups().count() > 0)
                {
                    txtStream << "<tr>" << endl;
                    txtStream << "<td width=\"70%%\" align=left>" << endl;
                    txtStream << "<p><a href=\"file:///" << m_lstDataset.at(nDataset)->rawDataFilename() << "\"><img src=\"../images/csv_spreadsheet_icon.png\" border=\"0\" width=\"22\" height=\"19\"></a> ";
                    txtStream << "<a href=\"file:///" << m_lstDataset.at(nDataset)->rawDataFilename() << "\">" << m_lstDataset.at(nDataset)->rawDataFilename() << "</a></p>" << endl;
                    txtStream << "</td>" << endl;
                    txtStream << "<td width=\"30%%\"></td>" << endl;
                    txtStream << "</tr>" << endl;
                }
            }
            txtStream << "</table>" << endl;
        }

        txtStream << "</table>" << endl;
        txtStream << "<br>" << endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString GexAdvancedEnterpriseReport::htmlTitle() const
//
// Description	:	returns the title html compliant
//
///////////////////////////////////////////////////////////////////////////////////
QString GexAdvancedEnterpriseReport::htmlName() const
{
    QString strHtmlName = m_strName;

    strHtmlName.replace("<", "&lt;");
    strHtmlName.replace(">", "&gt;");

    return strHtmlName;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportScriptEngine
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportScriptEngine::GexAdvancedEnterpriseReportScriptEngine()
{
    m_pScriptEngine = new QScriptEngine();

    // todo: rename this function as GSDecimalNumber
    m_pScriptEngine->globalObject().setProperty("gexDecimalNumber",	m_pScriptEngine->newFunction(gexDecimalNumber));

    #ifdef QT_DEBUG
    //	m_scriptDebugger.attachTo(m_pScriptEngine);
    #endif
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportScriptEngine::~GexAdvancedEnterpriseReportScriptEngine()
{
    if (m_pScriptEngine)
    {
        delete m_pScriptEngine;
        m_pScriptEngine = NULL;
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportScriptEngine::fillScriptGroupVariables(const GexDbPluginERDatasetGroup& datasetGroup, const QString& strAggregate /*= QString()*/) const
{
    m_pScriptEngine->globalObject().setProperty("$FGroup", QScriptValue(datasetGroup.group()));

    QStringList lstGroup = datasetGroup.group().split(",");

    for(int nGroup = 0; nGroup < lstGroup.count(); ++nGroup)
        m_pScriptEngine->globalObject().setProperty(QString("$FGroup%1").arg(nGroup),	QScriptValue(lstGroup.at(nGroup)));

    m_pScriptEngine->globalObject().setProperty("$FAggregate",				QScriptValue(strAggregate));
    m_pScriptEngine->globalObject().setProperty("$FGroupTime",				QScriptValue(datasetGroup.startTime()));
    m_pScriptEngine->globalObject().setProperty("Worst",					QScriptValue(datasetGroup.worstSerie().serie()));
    m_pScriptEngine->globalObject().setProperty("Best",						QScriptValue(datasetGroup.bestSerie().serie()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumYield",			QScriptValue(datasetGroup.yield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumBinYield",		QScriptValue(datasetGroup.binYield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumGrossYield",		QScriptValue(datasetGroup.grossYield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumGrossBinYield",	QScriptValue(datasetGroup.grossBinYield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumVolume",			QScriptValue(datasetGroup.volume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumBinVolume",		QScriptValue(datasetGroup.binVolume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumGoodVolume",		QScriptValue(datasetGroup.goodVolume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumGrossVolume",	QScriptValue(datasetGroup.grossVolume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupSumWaferCount",		QScriptValue(datasetGroup.waferCount()));
    m_pScriptEngine->globalObject().setProperty("$FAggregateCount",			QScriptValue(datasetGroup.aggregateValues().count()));

    int nAggregatePosition = datasetGroup.indexOfAggregate(strAggregate);

    if (nAggregatePosition != -1)
        fillScriptRowGroupVariables(datasetGroup.aggregateAt(nAggregatePosition));
    else
        fillScriptRowGroupVariables(GexDbPluginERDatasetRow());
}

void GexAdvancedEnterpriseReportScriptEngine::fillScriptRowGroupVariables(const GexDbPluginERDatasetRow& datasetRow) const
{
    m_pScriptEngine->globalObject().setProperty("$FGroupYield",				QScriptValue(datasetRow.yield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupBinYield",			QScriptValue(datasetRow.binYield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupGrossYield",		QScriptValue(datasetRow.grossYield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupGrossBinYield",		QScriptValue(datasetRow.grossBinYield()));
    m_pScriptEngine->globalObject().setProperty("$FGroupVolume",			QScriptValue(datasetRow.volume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupBinVolume",			QScriptValue(datasetRow.binVolume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupGoodVolume",		QScriptValue(datasetRow.goodVolume()));
    m_pScriptEngine->globalObject().setProperty("$FGroupGrossVolume",		QScriptValue(datasetRow.grossVolume()));
    m_pScriptEngine->globalObject().setProperty("$FBinNo",					QScriptValue(datasetRow.field("bin_no").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FBinName",				QScriptValue(datasetRow.field("bin_name").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FBinCat",					QScriptValue(datasetRow.field("bin_cat").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FTime",					QScriptValue(datasetRow.startTime()));
    m_pScriptEngine->globalObject().setProperty("$FGroupWaferCount",		QScriptValue(datasetRow.waferCount()));

    QSet<GexDbPluginERDatasetField>::const_iterator itBegin = datasetRow.fields().begin();
    QSet<GexDbPluginERDatasetField>::const_iterator itEnd	= datasetRow.fields().end();

    while (itBegin != itEnd)
    {
        m_pScriptEngine->globalObject().setProperty("$FGroup_" + (*itBegin).name(), (*itBegin).value().toString());

        ++itBegin;
    }
}

void GexAdvancedEnterpriseReportScriptEngine::fillScriptSerieVariables(const GexDbPluginERDatasetSerie& datasetSerie, const QString& strAggregate /*= QString()*/) const
{
    m_pScriptEngine->globalObject().setProperty("$FAggregate",			QScriptValue(strAggregate));

    QStringList lstSerie = datasetSerie.serie().split(",");

    for(int nSerie = 0; nSerie < lstSerie.count(); ++nSerie)
        m_pScriptEngine->globalObject().setProperty(QString("$FSerie%1").arg(nSerie),	QScriptValue(lstSerie.at(nSerie)));

    m_pScriptEngine->globalObject().setProperty("$FSerie",				QScriptValue(datasetSerie.serie()));
    m_pScriptEngine->globalObject().setProperty("$FSerieTime",			QScriptValue(datasetSerie.startTime()));
    m_pScriptEngine->globalObject().setProperty("$FSumYield",			QScriptValue(datasetSerie.yield()));
    m_pScriptEngine->globalObject().setProperty("$FSumBinYield",		QScriptValue(datasetSerie.binYield()));
    m_pScriptEngine->globalObject().setProperty("$FSumGrossYield",		QScriptValue(datasetSerie.grossYield()));
    m_pScriptEngine->globalObject().setProperty("$FSumGrossBinYield",	QScriptValue(datasetSerie.grossBinYield()));
    m_pScriptEngine->globalObject().setProperty("$FSumVolume",			QScriptValue(datasetSerie.volume()));
    m_pScriptEngine->globalObject().setProperty("$FSumBinVolume",		QScriptValue(datasetSerie.binVolume()));
    m_pScriptEngine->globalObject().setProperty("$FSumGoodVolume",		QScriptValue(datasetSerie.goodVolume()));
    m_pScriptEngine->globalObject().setProperty("$FSumGrossVolume",		QScriptValue(datasetSerie.grossVolume()));
    m_pScriptEngine->globalObject().setProperty("$FSumWaferCount",		QScriptValue(datasetSerie.waferCount()));
    m_pScriptEngine->globalObject().setProperty("$FSerieBinNo",			QScriptValue(datasetSerie.field("bin_no").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FSerieBinName",		QScriptValue(datasetSerie.field("bin_name").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FSerieBinCat",		QScriptValue(datasetSerie.field("bin_cat").value().toString()));

    int nAggregatePosition = datasetSerie.indexOfAggregate(strAggregate);

    if (nAggregatePosition != -1)
        fillScriptRowSerieVariables(datasetSerie.aggregateAt(nAggregatePosition));
    else
        fillScriptRowSerieVariables(GexDbPluginERDatasetRow());
}

void GexAdvancedEnterpriseReportScriptEngine::fillScriptRowSerieVariables(const GexDbPluginERDatasetRow& datasetRow) const
{
    m_pScriptEngine->globalObject().setProperty("$FYield",				QScriptValue(datasetRow.yield()));
    m_pScriptEngine->globalObject().setProperty("$FBinYield",			QScriptValue(datasetRow.binYield()));
    m_pScriptEngine->globalObject().setProperty("$FGrossYield",			QScriptValue(datasetRow.grossYield()));
    m_pScriptEngine->globalObject().setProperty("$FGrossBinYield",		QScriptValue(datasetRow.grossBinYield()));
    m_pScriptEngine->globalObject().setProperty("$FBinVolume",			QScriptValue(datasetRow.binVolume()));
    m_pScriptEngine->globalObject().setProperty("$FVolume",				QScriptValue(datasetRow.volume()));
    m_pScriptEngine->globalObject().setProperty("$FGoodVolume",			QScriptValue(datasetRow.goodVolume()));
    m_pScriptEngine->globalObject().setProperty("$FGrossVolume",		QScriptValue(datasetRow.grossVolume()));
    m_pScriptEngine->globalObject().setProperty("$FBinNo",				QScriptValue(datasetRow.field("bin_no").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FBinName",			QScriptValue(datasetRow.field("bin_name").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FBinCat",				QScriptValue(datasetRow.field("bin_cat").value().toString()));
    m_pScriptEngine->globalObject().setProperty("$FTime",				QScriptValue(datasetRow.startTime()));
    m_pScriptEngine->globalObject().setProperty("$FWaferCount",			QScriptValue(datasetRow.waferCount()));

    QSet<GexDbPluginERDatasetField>::const_iterator itBegin = datasetRow.fields().begin();
    QSet<GexDbPluginERDatasetField>::const_iterator itEnd	= datasetRow.fields().end();

    while (itBegin != itEnd)
    {
        m_pScriptEngine->globalObject().setProperty("$F" + (*itBegin).name(), (*itBegin).value().toString());

        ++itBegin;
    }
}
