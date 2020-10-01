#ifndef PAT_JSON_OUTLIER_H
#define PAT_JSON_OUTLIER_H

#include <QJsonObject>
#include "pat_json_failing_test.h"
#include "pat_outliers.h"
#include "pat_info.h"

namespace GS
{
    namespace Gex
    {
        class PATRecipe;
    }
}

namespace GS
{
    namespace PAT
    {

        class IPatJsonOutlier
        {
        public:
            virtual ~IPatJsonOutlier(){}
            virtual void buildJson(const CPatOutlierPart* outlier, int patBinType,  IPatJsonFailingTest* patJsonFailingTest, QJsonObject& objectToFill) = 0;
            virtual void buildJson(const CPatDieCoordinates& outlier, QJsonObject& objectToFill) = 0;
            virtual void buildJson(const GS::Gex::PATMVOutlier& outlier, QJsonObject& objectToFill) = 0;
        };


        class PatJsonOutlierV1: public IPatJsonOutlier
        {
        public:
            PatJsonOutlierV1();

            /**
             * @brief build a json document report containing the result of outiler
             * @param outlier : informations about the outlier
             * @param patBinType : enable the retrive of the bin name
             * @param patJsonFailingTest: contain the informations about the failing tests
             * @param objectToFill : JSon object to be filled
             */
            void buildJson(const CPatOutlierPart* outlier, int patBinType, IPatJsonFailingTest* patJsonFailingTest, QJsonObject& objectToFill);

            /**
             * @brief build a json document report containing the result of outiler
             * @param outlier : informations about the outlier cordinate
             * @param objectToFill : JSon object to be filled
             */
            void buildJson(const CPatDieCoordinates &outlier, QJsonObject& objectToFill);

            /**
             * @brief build a json document report containing the result of MV outiler
             * @param outlier : informations about the outlier
             * @param objectToFill : JSon object to be filled
             */
            void buildJson(const GS::Gex::PATMVOutlier& outlier, QJsonObject& objectToFill);


        };
    }
}

#endif // PAT_JSON_OUTLIER_H
