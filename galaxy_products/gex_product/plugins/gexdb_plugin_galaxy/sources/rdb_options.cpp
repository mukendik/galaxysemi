// gexdb_plugin_galaxy_options.cpp: implementation of the GexDbPlugin_Galaxy_Options class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

// Standard includes

#include <QVariant>

// Local includes
#include "rdb_options.h"

namespace GS
{
namespace DbPluginGalaxy
{

const QString RdbOptions::mOptionExtractRawData="ExtractRawData";
const QString RdbOptions::mOptionExtractPartsIfNoResults="ExtractPartsIfNoResults";
const QString RdbOptions::mOptionExtractionGroupBy="ExtractionGroupBy";
const QString RdbOptions::mOptionSimulateWSifDieTracedFTExtraction="SimulateWSifDieTracedFTExtraction";

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
RdbOptions::RdbOptions()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////
// Reset options
//////////////////////////////////////////////////////////////////////
void RdbOptions::Reset()
{
    //m_bExtractRawData = true;
    mOptions.clear();
    mOptions.insert(RdbOptions::mOptionExtractRawData, QVariant(true) );
    mOptions.insert(RdbOptions::mOptionExtractPartsIfNoResults, QVariant(false) );
    //mOptions.insert(GexDbPlugin_Galaxy_Options::mOptionExtraction, QVariant(false) );

}

//////////////////////////////////////////////////////////////////////
// Init options
//////////////////////////////////////////////////////////////////////
void RdbOptions::Init(const QString & strOptionsString)
{
    QStringList				strlOptions = strOptionsString.split(";");
	QStringList::iterator	it;
    QString					strOption, strKey, strValue;

	Reset();

	for(it=strlOptions.begin(); it!=strlOptions.end(); it++)
	{
        strOption = (*it).trimmed();
        if (strOption.isEmpty())
            continue;

        strKey = strOption.section("=", 0, 0).trimmed(); // .lower() ?
        strValue = strOption.section("=", 1, 1).trimmed().toLower();

        mOptions.insert(strKey, strValue);
        /*
        if(strOption.section("=", 0, 0).trimmed().lower() == "extractrawdata")
		{
            strValue = strOption.section("=", 1, 1).trimmed().lower();
			if(strValue == "false")
				m_bExtractRawData = false;
		}
        */
	}
}

} // END DbPluginGalaxy
} // END GS

