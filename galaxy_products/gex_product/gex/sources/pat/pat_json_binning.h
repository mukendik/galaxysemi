#ifndef PAT_JSON_BINNING_H
#define PAT_JSON_BINNING_H

#include <QJsonObject>
#include <QObject>
#include "cbinning.h"

namespace GS
{
    namespace PAT
    {

        class IPatJsonBinning
        {
        public:
            virtual ~IPatJsonBinning(){}
            virtual void buildJson(const CBinning* binning, QJsonObject& sumObjectToFill, int failedType) = 0;
        };

        class PatJsonBinningV1 : public IPatJsonBinning
        {
            public:
                PatJsonBinningV1();

                /**
                 * @brief build a json document report containing the description of the binning
                 * @param binning : informations about the binning
                 * @param sumObjectToFill : JSon object to be filled
                 * @param failedType : failedType that will enable to retrive the failed name
                 */
                void buildJson  (const CBinning* binning, QJsonObject& sumObjectToFill, int failedType);
        };
    }
}

#endif // PAT_JSON_BINNING_H
