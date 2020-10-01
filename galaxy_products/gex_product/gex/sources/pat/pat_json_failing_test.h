#ifndef PAT_FAILING_TEST_H
#define PAT_FAILING_TEST_H

#include <QJsonObject>
#include "pat_outliers.h"

namespace GS
{
    namespace PAT
    {

        class IPatJsonFailingTest
        {
        public:
            virtual ~IPatJsonFailingTest(){}
            virtual void buildJson(const CPatFailingTest* failingTest, QJsonObject& objectToFill) = 0;
        };

        class PatJsonFailingTestV1: public IPatJsonFailingTest
        {
        public:
            PatJsonFailingTestV1();

            /**
             * @brief build a json document report containing the description of the failing tests
             * @param failingTest : informations about the failing tests
             * @param sumObjectToFill : JSon object to be filled
             */
            void buildJson(const CPatFailingTest* failingTest, QJsonObject& objectToFill);
        };
    }
}


#endif // PAT_FAILING_TEST_H
