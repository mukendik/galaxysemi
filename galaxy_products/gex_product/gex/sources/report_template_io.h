/******************************************************************************!
 * \file report_template_io.h
 ******************************************************************************/
#ifndef REPORT_TEMPLATE_IO_H
#define REPORT_TEMPLATE_IO_H

#include <QString>
#include <QTextStream>

namespace GS {
namespace Gex {

class ReportTemplate;
class ReportTemplateGui;
class CustomReportEnterpriseReportSection;
class ReportTemplateSection;

/******************************************************************************!
 * \class ReportTemplateIO
 ******************************************************************************/
class ReportTemplateIO : public QObject
{
public:
    /*!
     * \fn ReportTemplateIO
     * \brief Constructor
     */
    ReportTemplateIO();
    /*!
     * \fn ReadTemplateFromDisk
     */
    bool ReadTemplateFromDisk(ReportTemplate* reportTemplate,
                              QString& strTemplateName);
    /*!
     * \fn WriteTemplateToDisk
     */
    bool WriteTemplateToDisk(ReportTemplate* reportTemplate,
                             QString& strTemplateName,
                             ReportTemplateGui* gui);

private:
    Q_DISABLE_COPY(ReportTemplateIO)

    /*!
     * \fn ReadTemplateFromDisk_HomePage
     */
    bool ReadTemplateFromDisk_HomePage(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Aggregate
     */
    bool ReadTemplateFromDisk_Aggregate(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Wafmap
     */
    bool ReadTemplateFromDisk_Wafmap(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Binning
     */
    bool ReadTemplateFromDisk_Binning(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Pareto
     */
    bool ReadTemplateFromDisk_Pareto(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Pearson
     */
    bool ReadTemplateFromDisk_Pearson(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_TesterCorrelationGB
     */
    bool ReadTemplateFromDisk_TesterCorrelationGB(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Production
     */
    bool ReadTemplateFromDisk_Production(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_GlobalInfo
     */
    bool ReadTemplateFromDisk_GlobalInfo(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_FileAudit
     */
    bool ReadTemplateFromDisk_FileAudit(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_Datalog
     */
    bool ReadTemplateFromDisk_Datalog(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_SQL
     */
    bool ReadTemplateFromDisk_SQL(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_SQL_Filters
     */
    QStringList ReadTemplateFromDisk_SQL_Filters(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_SQL_Options
     */
    void ReadTemplateFromDisk_SQL_Options(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER
     */
    bool ReadTemplateFromDisk_ER(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_ER_Data
     */
    void ReadTemplateFromDisk_ER_Data(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_DataFilters
     */
    QStringList ReadTemplateFromDisk_ER_DataFilters(QTextStream& hTemplate);
    /*!
     * \fn ReadTemplateFromDisk_ER_Std_Style
     */
    void ReadTemplateFromDisk_ER_Std_Style(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Std_Advanced
     */
    void ReadTemplateFromDisk_ER_Std_Advanced(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_Std
     */
    void ReadTemplateFromDisk_ER_Prod_Std(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_WyrStandard
     */
    void ReadTemplateFromDisk_ER_Prod_WyrStandard(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard
     */
    void ReadTemplateFromDisk_ER_Prod_YieldWizard(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_GlobalStyle
     */
    void ReadTemplateFromDisk_ER_Prod_YieldWizard_GlobalStyle(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_VolumeStyle
     */
    void ReadTemplateFromDisk_ER_Prod_YieldWizard_VolumeStyle(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_BinparetoStyle
     */
    void ReadTemplateFromDisk_ER_Prod_YieldWizard_BinparetoStyle(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Prod_YieldWizard_Serie
     */
    void ReadTemplateFromDisk_ER_Prod_YieldWizard_Serie(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Genealogy_YieldVsYield
     */
    void ReadTemplateFromDisk_ER_Genealogy_YieldVsYield(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn ReadTemplateFromDisk_ER_Genealogy_YieldVsParameter
     */
    void ReadTemplateFromDisk_ER_Genealogy_YieldVsParameter(
        QTextStream& hTemplate,
        CustomReportEnterpriseReportSection* pEnterpriseReport);
    /*!
     * \fn WriteTemplateToDisk_HomePage
     */
    void WriteTemplateToDisk_HomePage(QTextStream& hTemplate);
    /*!
     * \fn WriteTemplateToDisk_Aggregate
     */
    void WriteTemplateToDisk_Aggregate(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_Wafmap
     */
    void WriteTemplateToDisk_Wafmap(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_Binning
     */
    void WriteTemplateToDisk_Binning(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_Pareto
     */
    void WriteTemplateToDisk_Pareto(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_Pearson
     */
    void WriteTemplateToDisk_Pearson(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_TesterCorrelationGB
     */
    void WriteTemplateToDisk_TesterCorrelationGB(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_Production
     */
    void WriteTemplateToDisk_Production(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_GlobalInfo
     */
    void WriteTemplateToDisk_GlobalInfo(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_FileAudit
     */
    void WriteTemplateToDisk_FileAudit(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_ER
     */
    void WriteTemplateToDisk_ER(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);
    /*!
     * \fn WriteTemplateToDisk_Datalog
     */
    void WriteTemplateToDisk_Datalog(
        QTextStream& hTemplate,
        ReportTemplateSection* pNewSection);

    /*!
     * \var mReportTemplate
     */
    ReportTemplate* mReportTemplate;
    /*!
     * \var mReportTemplateGui
     */
    ReportTemplateGui* mReportTemplateGui;
};

}
}

#endif
