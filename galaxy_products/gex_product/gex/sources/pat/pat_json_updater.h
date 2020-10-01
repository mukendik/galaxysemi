#ifndef PAT_JSON_UPDATER_H
#define PAT_JSON_UPDATER_H


#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "pat_json_report_version.h"
#include "pat_options.h"
#include "pat_json_binning.h"
#include "pat_json_totalcount.h"
#include "pat_json_failing_test.h"
#include "pat_json_outlier.h"
#include "pat_json_test.h"
#include "pat_info.h"

namespace GS
{
    namespace PAT
    {
        class PatJsonUpdater
        {
            public:
                PatJsonUpdater();

                /**
                 * @brief build a json document report containing the result of the PAT processing
                 * @param patInfo : informations result of the pat processing
                 * @param version : version of the expected report
                 */
                QJsonDocument buildJsonReport(const CPatInfo* patInfo, const QString& version);

            private :

                IPatJsonTotalCount  *mPatJsonTotalCount;
                IPatJsonBinning     *mPatJsonBinning;
                IPatJsonOutlier     *mPatJsonOutlier;
                IPatJsonFailingTest *mPatJsonFailingTest;
                IPatJsonTest        *mPatJsonTest;

                /**
                 * @brief convert the CBinning into json format and put it in the json document
                 * @param patInfo :         Contains the CBinning and CPatOption needed
                 * @param jsonBinObject :   Json object that will contain the resul convertion
                 * @param json name key :   name key of the json value inside jsonBinObject
                 * @param expectedSoftBin:  indicates whether we need soft bin or hard bin
                 * @param expectedPatBin:   indicates whether we need only Pat bin bin or the other bin
                 */
                void writeBinning       (const CPatInfo *patInfo, QJsonObject& jsonBinObject, const QString &jsonNameKey, bool expectedSoftBin, bool expectedPatBin);

                /**
                 * @brief write the test information into a json object
                 * @param patInfo :         Contains the PATRecipe needed
                 * @param jsonDetail :      Json object that will contain the resul convertion
                 */
                void writeTest          (const CPatInfo* patInfo, QJsonObject& jsonDetail);

                /**
                 * @brief provess all the different outlier structures in order to write them in a json object
                 * @param patInfo :         Contains all the different outlier structure needed
                 * @param jsonDetail :      Json object that will contain the resul convertion
                 */
                void processAllOutliers   (const CPatInfo* patInfo, QJsonObject& jsonDetail);

                /**
                 * @brief write an outlier structure in a json object
                 * @param elementToWrite :   Outlier structure that will be converted in json
                 * @param jsonDetail :       Json object that will contain the resul convertion
                 */
                template <typename T> void writeOutlier(const T& elementToWrite, QJsonArray& jsonDetail);

                /**
                 * @brief initialize the json report structure according to the version in parameter
                 * @param version : the expected json format version
                 */
                void initJSonReport     (const QString& version);

                /**
                 * @brief initialize each object used in the report structure according to the json version
                 */
                template <typename T_JSonVersion>
                void initJSonReportElements()
                {
                    mPatJsonTotalCount  = new typename T_JSonVersion::T_PatJsonTotalCount  ();
                    mPatJsonBinning     = new typename T_JSonVersion::T_PatJsonBinning     ();
                    mPatJsonOutlier     = new typename T_JSonVersion::T_PatJsonOutlier     ();
                    mPatJsonFailingTest = new typename T_JSonVersion::T_PatJsonFailingTest ();
                    mPatJsonTest        = new typename T_JSonVersion::T_PatJsonTest        ();
                }
        };

    }
}


#endif // PAT_JSON_UPDATER_H
