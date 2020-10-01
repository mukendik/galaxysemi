#include "pat_json_failing_test.h"

namespace GS
{
    namespace PAT
    {


PatJsonFailingTestV1::PatJsonFailingTestV1()
{
}

void PatJsonFailingTestV1::buildJson(const CPatFailingTest* failingTest, QJsonObject& objectToFill)
{
    if(failingTest == 0)
        return;

    objectToFill.insert("Number",  static_cast<qint64>(failingTest->mTestNumber));
    objectToFill.insert("Name",    failingTest->mTestName);
    objectToFill.insert("Pin",     failingTest->mPinIndex);
}

}
}
