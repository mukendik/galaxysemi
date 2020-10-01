#include "gex_options_handler.h"
#include "gex_constants.h"
#include <gqtl_log.h>
#include "report_options.h"
#include "product_info.h"

#ifdef _WIN32
  #include <windows.h>
  // #define used to readback computer paper size
  #ifndef LOCALE_IPAPERSIZE
   #define LOCALE_IPAPERSIZE             0x0000100A
  #endif
#endif


// in script_wizard.h
extern void ConvertToScriptString(QString &strFile);

namespace GS
{
namespace Gex
{

OptionsHandler::ContainerOptionsWithNoRebuil OptionsHandler::CreateOptionsWithNoRebuildReport()
{
    ContainerOptionsWithNoRebuil lOptions;
    lOptions.insert(std::make_pair("output", "embed_fonts_in_pdf"));
    lOptions.insert(std::make_pair("output", "pdf_printer"));
    lOptions.insert(std::make_pair("output", "format"));
    lOptions.insert(std::make_pair("output", "front_page_text"));
    lOptions.insert(std::make_pair("output", "front_page_image"));
    lOptions.insert(std::make_pair("output", "paper_size"));
    lOptions.insert(std::make_pair("output", "paper_format"));
    lOptions.insert(std::make_pair("output", "truncate_names"));
    lOptions.insert(std::make_pair("output", "location"));
    lOptions.insert(std::make_pair("output", "location_path"));
    lOptions.insert(std::make_pair("output", "generation_mode"));
    lOptions.insert(std::make_pair("output", "sub_folders"));
    lOptions.insert(std::make_pair("output", "file_name"));
    lOptions.insert(std::make_pair("output", "html_page_breaks"));
    lOptions.insert(std::make_pair("output", "output"));
    return lOptions;
}

OptionsHandler::ContainerOptionsWithNoRebuil
OptionsHandler::mOptionsWithNoRebuildReport = CreateOptionsWithNoRebuildReport();

bool OptionsHandler::sNeedToRebuild = false;

void OptionsHandler::UpdateReportOption(CReportOptions& lReportOption)
{
    ContainerOptionsWithNoRebuil::iterator lIter(mOptionsWithNoRebuildReport.begin());
    ContainerOptionsWithNoRebuil::iterator lIterEnd(mOptionsWithNoRebuildReport.end());

    for(; lIter != lIterEnd; ++lIter)
    {
        QString lSection = (*lIter).first.c_str();
        if (mOptionsMaps.m_map.find(lSection) != mOptionsMaps.m_map.end())
        {
            QString lName = (*lIter).second.c_str();
            if( mOptionsMaps.m_map[lSection].find(lName) != mOptionsMaps.m_map[lSection].end())
            {
                lReportOption.GetOptionsHandler().SetOption(lSection, lName, mOptionsMaps.m_map[lSection][lName]);
            }
        }
    }
}

void OptionsHandler::ActiveNeedToRebuild(const std::string &section, const std::string &name)
{
    // if the option is not in the list of options that doesn't nned to rebuild the report
    // set the flag NeedToRebuild to true
    if(IsOptionReportRebuildLess(section, name) == false)
        sNeedToRebuild = true;
}

 bool OptionsHandler::IsOptionReportRebuildLess(const std::string &section, const std::string &name)
 {
     if(mOptionsWithNoRebuildReport.find(std::make_pair(section, name)) == mOptionsWithNoRebuildReport.end())
         return false; // option hasn't been found

     // option has been found as no need to rebuild the report
     return true;
 }

OptionsHandler::OptionsHandler()
    : mOptionsTypeDefinition(QString(":/gex/xml/gex_options.xml"))
{
}

OptionsHandler::OptionsHandler(const OptionsHandler& other)
{
    *this = other;
}

OptionsHandler::~OptionsHandler()
{

}

void OptionsHandler::ClearOptionsMap()
{
    mOptionsMaps.clear();
}

const OptionsMap& OptionsHandler::GetOptionsMap() const
{
    return mOptionsMaps;
}

const OptionsTypeDefinition &OptionsHandler::GetOptionsTypeDefinition() const
{
    return mOptionsTypeDefinition;
}

bool OptionsHandler::SetOptionsMap(const OptionsMap &optionsMap)
{
    bool bSetOptionsMapRslt = true;
    QString strSectionValue, strFieldValue, strValueValue;

    QMapIterator<QString, QMap<QString, QString> >	qmiSectionIterator(optionsMap.m_map);

    while(qmiSectionIterator.hasNext())
    {
        qmiSectionIterator.next();

        strSectionValue = qmiSectionIterator.key();
        QMapIterator<QString, QString> qmiFieldIterator = qmiSectionIterator.value();
        while(qmiFieldIterator.hasNext())
        {
            qmiFieldIterator.next();

            strFieldValue = qmiFieldIterator.key();
            strValueValue = qmiFieldIterator.value();

            bSetOptionsMapRslt &= SetOption(strSectionValue, strFieldValue, strValueValue);
        }

    }

    return bSetOptionsMapRslt;
}

bool OptionsHandler::IsReady() const
{
    return mOptionsTypeDefinition.isReady();
}

bool OptionsHandler::SetOption(const QString& section, const QString& option, const QString& value)
{
    QString lstrValue = value;
    QString lstrOption = option;
    QString lstrSection = section;

    //GCORE-17460 renaming outlier options but keeping retro-compatibility with old csl files
    if(lstrSection == "statistics")
    {
        if(lstrOption == "outlier_removal_mode")
            lstrOption = "data_cleaning_mode";
        if(lstrOption == "outlier_removal_value")
            lstrOption = "data_cleaning_value";
    }

    if(!mOptionsTypeDefinition.isValidSetOption(lstrSection, lstrOption, lstrValue))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Try to set invalid option (section='%1', field='%2', value='%3'")
              .arg(lstrSection).arg(lstrOption).arg(value).toLatin1().constData());
//        GEX_ASSERT(false);
        return false;
    }

    QMap<QString, QString> m;
    if (mOptionsMaps.m_map.find(lstrSection) != mOptionsMaps.m_map.end())
        m = mOptionsMaps.m_map[lstrSection];

    m.insert(lstrOption, lstrValue);
    mOptionsMaps.m_map.insert(lstrSection, m);

    return true;
}

bool OptionsHandler::SetSpecificFlagOption(const QString& lSection,
                                           const QString& lOption,
                                           const QString& lFlag,
                                           bool lEnabled)
{
    QVariant	qvFlagOptions = mOptionsMaps.GetOption(lSection, lOption);

    if(!qvFlagOptions.isValid())
    {
        bool bIsVeryWellSetted = SetOption(lSection, lOption, QString(""));
        if(!bIsVeryWellSetted)
            return bIsVeryWellSetted;
        else if(!lEnabled)
            return bIsVeryWellSetted;
    }

    QString		strFlagOptions		= qvFlagOptions.toString();
    QStringList qstrlFlagOptionList = strFlagOptions.split(QString("|"));

    if (lFlag.contains(QString("|")))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Set specific flag option unvalid, %1 %2 %3")
              .arg(lSection).arg(lOption).arg(lFlag).toLatin1().constData());

        return false;
    }


    if(qstrlFlagOptionList.contains(lFlag))
    {
        if(!lEnabled)	// extract 'strValue' from the option flag list, else nothing to do
        {
            QString qstrNewOptionValue = QString("");
            for(int ii=0; ii<qstrlFlagOptionList.count(); ii++)
            {
                QString qstrStringListElement = qstrlFlagOptionList.at(ii);
                if(qstrStringListElement != lFlag)
                    qstrNewOptionValue += qstrStringListElement + QString("|");
            }

            if(qstrNewOptionValue.endsWith("|"))
            {
                int nStrSize = qstrNewOptionValue.size();
                qstrNewOptionValue.remove(nStrSize-1, 1);
            }

            SetOption(lSection, lOption, qstrNewOptionValue);
            return true;
        }
    }
    else
    {
        if(lEnabled)	// add 'strValue' to the option flag list, else nothing to do
        {
            QString qstrNewOptionValue = QString("");

            if(strFlagOptions.isEmpty())
                qstrNewOptionValue = lFlag;
            else
                qstrNewOptionValue += strFlagOptions + QString("|") + lFlag;

            SetOption(lSection, lOption, qstrNewOptionValue);
            return true;
        }
    }

    // everything went well !! (just nothing to do)
    return true;
}

bool OptionsHandler::WriteOptionsToFile(FILE* hFile) const
{
    if (!hFile)
        return false;

    QMapIterator<QString, QMap<QString, QString> > it(mOptionsMaps.m_map);
    if (!it.hasNext())
        return false;

    fprintf(hFile,"\n  // ReportOptions dynamic options :\n");

    while (it.hasNext())
    {
        it.next();
        fprintf(hFile,"  // Section %s \n", it.key().toLatin1().data());
        QMap<QString, QString> m=it.value();
        QMapIterator<QString, QString> it2(m);
        QString strValue;

        while (it2.hasNext())
        {
            it2.next();

            // Make sure value is CSL compatible
            strValue = it2.value();
            ConvertToScriptString(strValue);
            if(mOptionsTypeDefinition.toSaveOption(it.key(), it2.key()))
            {
                fprintf(hFile,"  gexOptions('%s','%s','%s');\n",
                        it.key().toLatin1().data(),
                        it2.key().toLatin1().data(),
                        strValue.toLatin1().data() );
            }
        }

    }
    fprintf(hFile,"  // end of ReportOptions dynamic options.\n\n");
    return true;
}

OptionsHandler &OptionsHandler::operator =(const OptionsHandler &other)
{
    if (this != &other)
    {
        mOptionsMaps               = other.mOptionsMaps;
        mOptionsTypeDefinition = other.mOptionsTypeDefinition;
    }

    return *this;
}

void OptionsHandler::Reset(bool lDefault)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reset options(%1)").arg(lDefault?"default":"clear").toLatin1().data());

    // When resetting the options handler, we need to make sure that the options definitions have
    // been correctly loaded, if not try to reload it.
    if (mOptionsTypeDefinition.isReady() == false)
        mOptionsTypeDefinition = OptionsTypeDefinition(QString(":/gex/xml/gex_options.xml"));

    QString strSection, strOption, strDefaultValue, strOptionType;
    QStringList qslOptionList;

    QMap< QString, QStringList > qmOptionMap = mOptionsTypeDefinition.getOptionMap();
    QMapIterator< QString, QStringList > qmiSectionIterator(qmOptionMap);

    while(qmiSectionIterator.hasNext())
    {
        qmiSectionIterator.next();
        qslOptionList = qmiSectionIterator.value();
        strSection = qmiSectionIterator.key();

        for(int ii=0; ii<qslOptionList.count(); ii++)
        {
            strOption = qslOptionList.at(ii);
            strDefaultValue = mOptionsTypeDefinition.getDefaultOptionValue(strSection, strOption);
            strOptionType = mOptionsTypeDefinition.getOptionType(strSection, strOption);

            if(strOptionType == QString("HTML"))
                continue;       // never clear thoose kind of options

            if(!lDefault)       // manage clear / default option of reset method
                if(strOptionType == QString("Flag"))
                    strDefaultValue.clear();

            if(!strDefaultValue.startsWith("error"))
                SetOption(strSection, strOption, strDefaultValue);     // some options shouldn't be reset
        }
    }
    return;

    /// first auto-validation returns
    // 'output', 'paper_size' => TO CHECK : change os country
    // 'output', 'embed_fonts_in_pdf' => no default value in v6.5
    // java type options cleared by reset in v6.6 not in v6.5 ('output', {sub_folders, file_name})
    // 'dataprocessing', 'stdf_intermediate'
    // 'histogram', 'markers'
    // 'adv_trend', 'markers', {min/max/q1/q3}
    // 'adv_correlation', 'markers', {min/max/q1/q3}
    // 'adv_probabilityplot, 'markers', {min/max/q1/q3}
    // 'adv_boxplot, 'markers', {min/max/q1/q3}
    // 'adv_boxplot_ex, 'field', {av/r&r/max_xdiff}
    // 'adv_pearson', 'min_samples' => default value was '5' in xml file
    // 'adv_pearson', 'test_stats' => no default value
    // 'preferences', {text_editor ,ssheet_editor ,pdf_editor} : problem if a path is set, can't reset
    /// ! auto validation


    /// no bDefault management for flag options : to check with hard coded reports
    // "monitoring", "home_page", "show_testers|show_products"
    // "monitoring", "product_page", "yield|alarm_limit|yield_chart"
    // "monitoring", "tester_page", "product|program_name|yield|total_parts|total_good|total_fail|yield_chart"
    // "wafer", "chart_show", "all_individual|stacked|bin_mismatch|bin_to_bin"
    // "wafer", "visual_options", "shape_round"
    // "wafer", "compare", "any_size|diemismatch_table|deltayield_section"
    // "pareto", "section", "cp|cpk|failures|failure_signature|soft_bin|hard_bin"
    /// ! TOCHECK


    /// TO CHECK : no default value !
    // doesn't appear in 6.5
    // 'dataprocessing', 'clean_samples', false; type : Bool
    // 'dataprocessing', 'stdf_intermediate', /; type : Enum
    // 'dataprocessing', 'stdf_intermediate_path', /; type : Path

    // doesn't appear in 6.5 neither 6.6
    // 'adv_correlation', 'chart_type', /
    // 'adv_correlation', 'boxwhisker_orientation', /
    /// ! TO CHECK


    /// TODO : Path options behavior
    //    QString drnrf = strUserFolder + QDir::separator() + GEX_DEFAULT_DIR + "/.galaxy_rr.txt";
    //    SetOption("adv_boxplot","r&r_file", QDir::cleanPath(drnrf));
    /// ! TODO
}

}   // namespace Gex
}   // namespace GS
