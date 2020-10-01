#ifndef PAT_JSON_REPORT_VERSION_H
#define PAT_JSON_REPORT_VERSION_H

#include <map>
#include "pat_json_binning.h"
#include "pat_json_failing_test.h"
#include "pat_json_outlier.h"
#include "pat_json_test.h"
#include "pat_json_totalcount.h"

namespace GS
{
    namespace PAT
    {

         /**
         * @brief The T_Version enum define the diferent type of version that are managed
         */

        enum T_Version { v1_0_0= 0, undef };


        /**
        * @brief JsonReportVersion struct is an empty patern that must be specialized for
        * each new version of Json report.
        * The typedef inside must define the object that are used for the specific version
        */
        template <T_Version>
        struct JsonReportVersion
        {
        };

        template<>
        struct JsonReportVersion<v1_0_0>
        {
            typedef PatJsonBinningV1        T_PatJsonBinning;
            typedef PatJsonTotalCountV1     T_PatJsonTotalCount;
            typedef PatJsonTestV1           T_PatJsonTest;
            typedef PatJsonOutlierV1        T_PatJsonOutlier;
            typedef PatJsonFailingTestV1    T_PatJsonFailingTest;
        };

        /**
         * If the version is undef, we used the V1 version
         */
        template<>
        struct JsonReportVersion<undef>
        {
            typedef PatJsonBinningV1        T_PatJsonBinning;
            typedef PatJsonTotalCountV1     T_PatJsonTotalCount;
            typedef PatJsonTestV1           T_PatJsonTest;
            typedef PatJsonOutlierV1        T_PatJsonOutlier;
            typedef PatJsonFailingTestV1    T_PatJsonFailingTest;
        };

    }
}

#endif // PAT_JSON_REPORT_VERSION_H

