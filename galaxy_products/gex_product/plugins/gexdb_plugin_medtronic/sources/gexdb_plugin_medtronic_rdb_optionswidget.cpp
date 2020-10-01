#include "gexdb_plugin_medtronic_rdb_optionswidget.h"

///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
GexDbPlugin_Medtronic_Rdb_OptionsWidget::GexDbPlugin_Medtronic_Rdb_OptionsWidget( QWidget* parent, Qt::WindowFlags fl )
    : QWidget( parent, fl )
{
    setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
GexDbPlugin_Medtronic_Rdb_OptionsWidget::~GexDbPlugin_Medtronic_Rdb_OptionsWidget()
{
}

void GexDbPlugin_Medtronic_Rdb_OptionsWidget::GetOptionsString(QString & strOptionString)
{
    if (buttonKeepFirstResult->isChecked())
        strOptionString = "MultiResultsTest=KeepFirst";
    else if(buttonKeepAllResults->isChecked())
        strOptionString = "MultiResultsTest=KeepAll";
    else if(buttonKeepLastResult->isChecked())
        strOptionString = "MultiResultsTest=KeepLast";
}

void GexDbPlugin_Medtronic_Rdb_OptionsWidget::SetOptionsString(const GexDbPlugin_Medtronic_Options & clOptions)
{
    switch(clOptions.m_eTestMergeOption)
    {
        case GexDbPlugin_Medtronic_Options::eKeepFirstResult:
            buttonKeepFirstResult->setChecked(true);
            break;
        case GexDbPlugin_Medtronic_Options::eKeepAllResults:
            buttonKeepAllResults->setChecked(true);
            break;
        case GexDbPlugin_Medtronic_Options::eKeepLastResult:
        default:
            buttonKeepLastResult->setChecked(true);
            break;
    }
}
