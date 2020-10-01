// gexdb_plugin_medtronic_options.cpp: implementation of the GexDbPlugin_Medtronic_Options class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

// Standard includes

// Qt includes

// Galaxy modules includes

// Local includes
#include "gexdb_plugin_medtronic_options.h"

////////////////////////////////////////////////////////////////////////////////////
// Constants and Macro definitions
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Medtronic_Options class: Holds options specific to this plug-in
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_Medtronic_Options::GexDbPlugin_Medtronic_Options()
{
    Reset();
}

//////////////////////////////////////////////////////////////////////
// Reset options
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_Options::Reset()
{
    m_eTestMergeOption = eKeepLastResult;
}

//////////////////////////////////////////////////////////////////////
// Init options
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Medtronic_Options::Init(const QString & strOptionsString)
{
    QStringList				strlOptions = strOptionsString.split(";");
    QStringList::iterator	it;
    QString					strOption, strValue;

    Reset();

    for(it=strlOptions.begin(); it!=strlOptions.end(); it++)
    {
        strOption = (*it).trimmed();
        if(strOption.section("=", 0, 0).trimmed().toLower() == "multiresultstest")
        {
            strValue = strOption.section("=", 1, 1).trimmed().toLower();
            if(strValue == "keepfirst")
                m_eTestMergeOption = eKeepFirstResult;
            else if(strValue == "keepall")
                m_eTestMergeOption = eKeepAllResults;
            else
                m_eTestMergeOption = eKeepLastResult;

        }
    }
}

