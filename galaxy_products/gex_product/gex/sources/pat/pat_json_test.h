#ifndef PAT_JSON_TEST_H
#define PAT_JSON_TEST_H

#include "pat_definition.h"
#include <QJsonObject>

namespace GS
{
    namespace PAT
    {

        class IPatJsonTest
        {
        public:
            virtual ~IPatJsonTest(){}
            virtual void buildJson(const CPatDefinition* testPATDef, QJsonObject& objectToFill) = 0;
        };


        class PatJsonTestV1: public IPatJsonTest
        {
        public:
            PatJsonTestV1();

            /**
             * @brief build a json document report containing the description of the test PAT
             * @param testPATDef : informations about the pat tests
             * @param sumObjectToFill : JSon object to be filled
             */
            void buildJson(const CPatDefinition* testPATDef, QJsonObject& objectToFill);
        };
    }
}
#endif // PAT_JSON_TEST_H
