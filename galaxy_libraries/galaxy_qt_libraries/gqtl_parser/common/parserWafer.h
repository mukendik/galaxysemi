#ifndef PARSERWAFER_H
#define PARSERWAFER_H

#include <QString>
#include "parserParameter.h"


namespace GS
{
namespace Parser
{

class ParserWafer
{
public:
    ~ParserWafer();

    int				mWaferID;					// WaferID E.g: 6
    int				mLowestSiteID;			// Lowest SiteID found in PCM_HJTC file for this wafer
    int				mHighestSiteID;			// Highest SiteID found in PCM_HJTC file for this wafer
    QList<GS::Parser::ParserParameter*> mParameterList;	// List of Parameters in Wafer
};

}
}
#endif // PARSERWAFER_H
