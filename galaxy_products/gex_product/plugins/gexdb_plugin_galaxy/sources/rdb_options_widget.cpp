#include "libgexpb.h"
#include <gqtl_log.h>
#include "rdb_options_widget.h"

namespace GS
{
namespace DbPluginGalaxy
{

///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
RdbOptionsWidget::RdbOptionsWidget(
        const GexScriptEngine& se, QWidget* parent, Qt::WindowFlags fl )
    : QWidget( parent, fl ), mGSE(se)
{
    setupUi(this);

    mPropBrowser=CGexPB::CreatePropertyBrowser((GexScriptEngine*)&mGSE, this);
    layout()->addWidget(mPropBrowser);

    QMap< QString, QString > attributes;

    attributes.insert("type", "Bool");
    attributes.insert("label", "Extract parameter results");
    attributes.insert("tooltip", "Extract the parameter test results of not. If not, only summaries and some part info (X,Y,...) will be extracted.");
    attributes.insert("defaultvalue", "true");
    int r=mPropBrowser->AddProperty(attributes, -1); // mExtractRawDataPropID
    mOptionKeyToPropID.insert( RdbOptions::mOptionExtractRawData, r);

    attributes.clear();
    attributes.insert("type", "Bool");
    attributes.insert("label", "Extract parts without any result");
    attributes.insert("tooltip", "Some parts could have no results at all in the DB (because of sampling, stop on fail, ...). Do you still want to extract this part if no result ?");
    attributes.insert("defaultvalue", "true");
    r=mPropBrowser->AddProperty(attributes, -1);
    mOptionKeyToPropID.insert( RdbOptions::mOptionExtractPartsIfNoResults, r);

    // todo : add these options only if die trace ?
        attributes.clear();
        attributes.insert("type", "Enum");
        attributes.insert("label", "FT extraction group by");
        attributes.insert("tooltip",
          "Select here the FT extraction granularity: \n"
          "the DB plug-in will try to merge all data into a single file for the selected group.\n" \
          "The 'wafer' option will work only when die traced data is available in the TDR.\n" \
          "The 'splitlot' option will extract by FT splitlot (legacy behavior)\n" \
          "The 'automatic' option will try to intelligently select the extraction granularity:\n" \
          "- either 1 file by wafer (if die trace data is available)\n" \
          "- or 1 file by FT splitlot (if no die trace data is available");
        attributes.insert("defaultvalue", "'automatic'");
        attributes.insert("codingvalues", "automatic|splitlot|wafer");
        attributes.insert("displayvalues", "automatic|splitlot|wafer");
        r=mPropBrowser->AddProperty(attributes, -1);
        mOptionKeyToPropID.insert( RdbOptions::mOptionExtractionGroupBy, r);

        attributes.clear();
        attributes.insert("type", "Enum");
        attributes.insert("label", "FT dietraced extraction mode");
        attributes.insert("tooltip",
          "Select here how the die traced FT extraction must consider the wafer ID when performing a die traced FT extraction:\n" \
          "- either simulate a wafer sort file (with the wafer IDs WIR records)\n"
          "- or omit the wafer sort records, so no wafer ID will appear inside the report");
        attributes.insert("defaultvalue", "'SimulateWS'");
        attributes.insert("codingvalues", "simulatews|donotsimulatews");
        attributes.insert("displayvalues", "Simulate WaferSort|Do not simulate WaferSort");
        r=mPropBrowser->AddProperty(attributes, -1);
        mOptionKeyToPropID.insert( RdbOptions::mOptionSimulateWSifDieTracedFTExtraction, r);
}

RdbOptionsWidget::~RdbOptionsWidget()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Deleting GexDbPlugin_Galaxy_Rdb_OptionsWidget...");
    // delete mPropBrowser
    CGexPB::DestroyPropertyBrowser(mPropBrowser);
}

void RdbOptionsWidget::GetOptionsString(QString & strOptionString)
{
    strOptionString.clear();

    foreach(const QString &k, mOptionKeyToPropID.keys())
        strOptionString.append(k + "=" + mPropBrowser->GetCurrentValue( mOptionKeyToPropID.value(k) ).toString()+";");
}

void RdbOptionsWidget::SetOptionsString(const RdbOptions & clOptions)
{
    if (!mPropBrowser)
        return;
    foreach(const QString &k, clOptions.mOptions.keys())
    {
        if (mOptionKeyToPropID.find(k)==mOptionKeyToPropID.end())
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Property for key %1 unfindable in GUI").arg( k).toLatin1().constData());
            continue;
        }

        mPropBrowser->SetValue(mOptionKeyToPropID.value(k), clOptions.mOptions.value(k) );
    }

    //checkBoxExtractRawData->setChecked(clOptions.m_bExtractRawData);
}

} // END DbPluginGalaxy
} // END GS

