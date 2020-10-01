#include "QFile"
#include "pat_json_updater.h"


namespace GS
{
namespace PAT
{

    PatJsonUpdater::PatJsonUpdater():
                                     mPatJsonTotalCount(0),
                                     mPatJsonBinning(0),
                                     mPatJsonOutlier(0),
                                     mPatJsonFailingTest(0),
                                     mPatJsonTest(0)
    {}

    QJsonDocument PatJsonUpdater::buildJsonReport(const CPatInfo* patInfo, const QString& version)
    {

        QJsonDocument lJjsonDocument;

        if(patInfo == 0)
            return lJjsonDocument;

        //-- initialize the json converter depending of the version
        initJSonReport(version);

        //-- create the json document
        QJsonObject lJSObjectRoot = lJjsonDocument.object();

         QJsonObject lPatDetail;
         //-- outliers
         processAllOutliers(patInfo, lPatDetail);
         //-- test
         writeTest(patInfo, lPatDetail);
         lJSObjectRoot.insert("Detail", lPatDetail);

        // -- summary
        QJsonObject lSummary;
        mPatJsonTotalCount->buildJson(patInfo, lSummary);

        // ---- Pat Binning
        bool lExpectedPatBin = true;
        QJsonObject lPatBins;
        // -- Pat Hard Bin
        writeBinning( patInfo, lPatBins, "Hard", true, lExpectedPatBin);
        // -- Pat Soft Bin
        writeBinning( patInfo, lPatBins, "Soft", false, lExpectedPatBin);

        lSummary.insert("PATBins", lPatBins);

        //----- Other Binning
        lExpectedPatBin = false;
        QJsonObject lOtherBins;
        // --  Hard Bin except pat bin
        writeBinning(patInfo, lOtherBins, "Hard", true, lExpectedPatBin);
        // --  Soft Bin except pat bin
        writeBinning(patInfo, lOtherBins, "Soft", false, lExpectedPatBin);
        lSummary.insert("OtherBins", lOtherBins);


        lJSObjectRoot.insert("Summary", lSummary);

        lJSObjectRoot.insert("Version", QString("1.0.0"));
        // -- set in the jsondocument
        lJjsonDocument.setObject(lJSObjectRoot);

        return lJjsonDocument;
    }

    //!-----------------------------//
    //!         initJSonReport      //
    //!-----------------------------//
    void PatJsonUpdater::initJSonReport(const QString& version)
    {
        if(version == "1.0.0")
            initJSonReportElements<JsonReportVersion<v1_0_0> >();
        else
            initJSonReportElements<JsonReportVersion<undef> >();

    }

    //!-------------------------------//
    //!         processAllOutliers    //
    //!-------------------------------//
    void PatJsonUpdater::processAllOutliers(const CPatInfo* patInfo, QJsonObject& jsonDetail)
    {
        QList<CPatOutlierPart*>  lOutiliers = patInfo->m_lstOutlierParts;
        const COptionsPat&  lPatOption =  patInfo->GetRecipeOptions();
        QList<CPatOutlierPart*>::iterator lIterB(lOutiliers.begin()), lIterE(lOutiliers.end());
        QJsonArray lPatFails;
        for(; lIterB!=lIterE; ++lIterB)
        {
            QJsonObject lPatFail;
            mPatJsonOutlier->buildJson(*lIterB, lPatOption.GetPATSoftBinFailType((*lIterB)->iPatSBin), mPatJsonFailingTest, lPatFail);
            lPatFails.append(lPatFail);
        }

        writeOutlier(patInfo->mGDBNOutliers,        lPatFails);
        writeOutlier(patInfo->mClusteringOutliers,  lPatFails);
        writeOutlier(patInfo->mZPATOutliers,        lPatFails);
        writeOutlier(patInfo->mReticleOutliers,     lPatFails);
        writeOutlier(patInfo->mIDDQOutliers,        lPatFails);
        writeOutlier(patInfo->mNNROutliers,         lPatFails);

        writeOutlier(patInfo->GetMVOutliers(),      lPatFails);

        jsonDetail.insert("PATFails", lPatFails);
    }

    //!-----------------------------//
    //!         writeBinning        //
    //!-----------------------------//
    void PatJsonUpdater::writeBinning(const CPatInfo* patInfo, QJsonObject& jsonBinObject, const QString &jsonNameKey, bool expectedSoftBin, bool expectedPatBin)
    {
        QJsonArray lArrayBinning;

        //-- retrieve the expected binning (either hard or soft)
        const CBinning* lBinning = 0;
        if(expectedSoftBin)
             lBinning =   patInfo->GetSoftBins();
        else
             lBinning =   patInfo->GetHardBins();

        const COptionsPat&  lPatOption =  patInfo->GetRecipeOptions();

        while (lBinning)
        {
            QJsonObject lObject;

            bool lIsPatBin = true;
            int  lBinType = 0;
            //-- filter on the expected bin type (PAT or other)
            if(expectedSoftBin)
                lBinType = lPatOption.GetPATSoftBinFailType(lBinning->iBinValue);
            else
                lBinType = lPatOption.GetPATHardBinFailType(lBinning->iBinValue);

            lIsPatBin = (lBinType==0)?false:true;
            //-- write only if : expected pat Bin and current bin is a pat bin
            //--                 expected no pat bin and current bin is not a pat bin
            if (expectedPatBin == lIsPatBin)
            {
                mPatJsonBinning->buildJson(lBinning, lObject, lBinType);
                lArrayBinning.append(lObject);
            }
            lBinning = lBinning->ptNextBin;
        }

        jsonBinObject.insert(jsonNameKey,lArrayBinning);
    }


    //!-----------------------------//
    //!         writeTest           //
    //!-----------------------------//
    void PatJsonUpdater::writeTest( const CPatInfo* patInfo, QJsonObject& jsonDetail)
    {
       const GS::Gex::PATRecipe* lPatRecipe = patInfo->GetRecipe();

       if(lPatRecipe == 0)
           return;

       QHash<QString, CPatDefinition*>::const_iterator  lIterB(lPatRecipe->GetUniVariateRules().begin()),
                                                        lIterE(lPatRecipe->GetUniVariateRules().end());

        QJsonArray lTestsInformations;
        for(; lIterB != lIterE; ++lIterB)
        {
            QJsonObject lTestInformation;
            mPatJsonTest->buildJson(*lIterB, lTestInformation);
            lTestsInformations.append(lTestInformation);
        }

        jsonDetail.insert("TestInformation", lTestsInformations);
    }

    //!-----------------------------//
    //!         writeOutlier        //
    //!-----------------------------//
    template <typename T>
    void PatJsonUpdater::writeOutlier(const T& elementToWrite, QJsonArray& json)
    {
        typename T::const_iterator lIterBegin = elementToWrite.constBegin();
        typename T::const_iterator lIterEnd   = elementToWrite.constEnd();

        for(; lIterBegin!=lIterEnd; ++lIterBegin)
        {
            QJsonObject lPatFail;
            mPatJsonOutlier->buildJson(*lIterBegin, lPatFail);
            json.append(lPatFail);
        }
    }
}
}
