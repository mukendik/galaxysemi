#ifndef PAT_JSON_TOTALCOUNT_H
#define PAT_JSON_TOTALCOUNT_H

#include <QJsonObject>
#include "pat_info.h"

namespace GS
{
    namespace PAT
    {

        class IPatJsonTotalCount
        {
        public:
            virtual ~IPatJsonTotalCount(){}
            virtual void buildJson(const CPatInfo* patInfo, QJsonObject& sumObjectToFill) = 0;
        };

        class PatJsonTotalCountV1: public IPatJsonTotalCount
        {
        public:
            PatJsonTotalCountV1();

            /**
             * @brief build a json document report containing the summary part count
             * @param patInfo : informations result of the pat processing
             * @param sumObjectToFill : JSon object to be filled
             */
            void buildJson(const CPatInfo* patInfo, QJsonObject& sumObjectToFill);
        };
    }
}

#endif // PAT_JSON_TOTALCOUNT_H
