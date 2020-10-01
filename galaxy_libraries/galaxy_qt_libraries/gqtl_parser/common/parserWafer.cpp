
#include "parserWafer.h"
#include "parserParameter.h"

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ParserWafer::~ParserWafer()
{
    while (!mParameterList.isEmpty())
        delete mParameterList.takeFirst();
}

}
}
