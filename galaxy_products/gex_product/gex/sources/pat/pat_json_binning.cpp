#include "pat_global.h"
#include "pat_json_binning.h"


namespace GS
{
    namespace PAT
    {

        PatJsonBinningV1::PatJsonBinningV1()
        {}

        void PatJsonBinningV1::buildJson(const CBinning* binning, QJsonObject& sumObjectToFill, int failedType)
        {
            if(binning == 0)
                return ;

            if(failedType == 0)
                 sumObjectToFill.insert("Type",     QString("Stdf") );
            else
                sumObjectToFill.insert("Type",       GS::Gex::PAT::GetOutlierTypeName(failedType) );
            sumObjectToFill.insert("Name",           binning->strBinName );
            sumObjectToFill.insert("Number",         binning->iBinValue );
            sumObjectToFill.insert("Category",       QString(binning->cPassFail));
            sumObjectToFill.insert("Count",          binning->ldTotalCount);
        }
    }
}
