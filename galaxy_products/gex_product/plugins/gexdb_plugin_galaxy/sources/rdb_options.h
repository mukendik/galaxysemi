// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or 
// distribution of this program, or any portion of it,may 
// result in severe civil and criminal penalties, and will be 
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------

#ifndef RDB_OPTIONS_HEADER_H
#define RDB_OPTIONS_HEADER_H


// Standard includes
#include <QMap>
#include <QString>
#include <QStringList>

namespace GS
{
namespace DbPluginGalaxy
{
// CLASS: GexDbPlugin_Galaxy_Options
// Holds options specific to this plug-in

class RdbOptions
{
    public:
        RdbOptions();
        static const QString mOptionExtractRawData;
        static const QString mOptionExtractPartsIfNoResults;
        static const QString mOptionExtractionGroupBy;
        static const QString mOptionSimulateWSifDieTracedFTExtraction;

    public:
        void	Init(const QString & strOptionsString);

    protected:
        void	Reset();

    public:
        //bool	m_bExtractRawData;	// True if plugin should extract all raw data
        QMap< QString, QVariant > mOptions;
};

} // END DbPluginGalaxy
} // END GS

#endif // RDB_OPTIONS_HEADER_H
