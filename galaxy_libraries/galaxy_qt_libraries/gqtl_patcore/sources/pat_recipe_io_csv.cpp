#include "pat_recipe_io_csv.h"
#include "pat_recipe_io_json.h"
#include <gqtl_log.h>
#include "pat_global.h"
#include "pat_definition.h"
#include "pat_options.h"
#include "pat_recipe_io.h"
#include "pat_rules.h"
#include "pat_recipe.h"
#include "test_defines.h"

#include "gqtl_utils.h"

#include <QFile>
#include <QFileInfo>

#include <math.h>

namespace GS
{
namespace Gex
{

PATRecipeIOCsv::PATRecipeIOCsv()
    : PATRecipeIO(PATRecipeIO::CSV), mCurrentLine(-1)
{
}

PATRecipeIOCsv::~PATRecipeIOCsv()
{

}

bool PATRecipeIOCsv::Write(PATRecipe &lPATRecipe, const QString &recipeName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Write recipe into '%1'").arg(recipeName).toLatin1().data() );
    PATRecipeIOJson lRecipeIO;

    if (lRecipeIO.Write(lPATRecipe, recipeName) == false)
    {
        mErrorMessage = lRecipeIO.GetErrorMessage();
        return false;
    }

    return true;
}

bool PATRecipeIOCsv::Write(COptionsPat &lPatOptions, const QString &recipeName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Write recipe into '%1'").arg(recipeName).toLatin1().data() );

    PATRecipeIOJson lRecipeIO;

    if (lRecipeIO.Write(lPatOptions, recipeName) == false)
    {
        mErrorMessage = lRecipeIO.GetErrorMessage();
        return false;
    }

    return true;
}

bool PATRecipeIOCsv::Read(QTextStream & lRecipeStream, PATRecipe &lPATRecipe)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("PAT Recipe Csv Read from device '%1'")
          .arg(lRecipeStream.device()->metaObject()->className()).toLatin1().data());

    // Assign file I/O stream
    QString lRecipeLine;
    int     lSectionFound = 0;

    // Reset internal variable
    mErrorMessage.clear();
    mCurrentLine = -1;
    // GCORE-8 :
    // if the recipe reader cannot parse the outlier option section then the test def could fail
    QRegExp lOutlierOptionsRegExp("^" GEX_TPAT_OPEN_SECTION_OPTIONS ".*>.*");

    do
    {
        // Read line.
        lRecipeLine = lRecipeStream.readLine().trimmed();
        ++mCurrentLine;

        // Check if comment line
        if(lRecipeLine.startsWith("#"))
        {
            // comment line...simply do nothing! Ignore this line!
        }
        // GCORE-8
        //else if(lRecipeLine.startsWith(GEX_TPAT_OPEN_SECTION_OPTIONS, Qt::CaseInsensitive)
                //&& lRecipeLine.endsWith(">"))
        // sometimes the csv recipe are very dirty as for example:
        // <Outlier_Options>,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
        // Let s use a regexp to find for the key
        else if (lOutlierOptionsRegExp.indexIn(lRecipeLine)!=-1)
        {
            QString lRecipeOptionDetails
                    = lRecipeLine.simplified().section(GEX_TPAT_OPEN_SECTION_OPTIONS,1,1).remove(">");

            //Retrieve Testing stage
            if (lRecipeOptionDetails.contains("TestingStage="))
            {
                QString lValue = lRecipeOptionDetails.section("TestingStage=",1,1).section("\"",1,1).remove("\"");
                if (lValue.compare("Final Test", Qt::CaseInsensitive) == 0)
                    lPATRecipe.GetOptions().clear(PAT::RecipeFinalTest);
                else if (lValue.compare("Wafer Sort", Qt::CaseInsensitive) == 0)
                    lPATRecipe.GetOptions().clear(PAT::RecipeWaferSort);
                else
                {
                    lPATRecipe.GetOptions().SetRecipeType(PAT::RecipeUnknown);
                    mErrorMessage = QString("Unknown Testing Stage %1.").arg(lValue);
                    GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                    return false;
                }
            }
            else
                lPATRecipe.GetOptions().clear(PAT::RecipeWaferSort);

            //Retrieve Version
            if (lRecipeOptionDetails.contains("Version="))
            {
                QString lValue = lRecipeLine.section("Version=",1,1).section("\"", 1,1).remove("\"");
                lPATRecipe.GetOptions().SetRecipeVersion(lValue);
            }
            else
                lPATRecipe.GetOptions().SetRecipeVersion("");


            // We've seen a PAT recipe section!
            ++lSectionFound;

            // Section: <Outlier_Options>
            if (ReadOptions(lRecipeStream, lPATRecipe.GetOptions()) == false)
                return false;

            // Update bin# and colors for each geographical rules
            UpdateGeographicalRules(lPATRecipe.GetOptions());
        }
        else if(lRecipeLine.startsWith(GEX_TPAT_OPEN_SECTION_RULES, Qt::CaseInsensitive))
        {
            // We've seen a PAT recipe section!
            ++lSectionFound;
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Outlier rules section found at line %1").arg(mCurrentLine)
                  .toLatin1().data());
            // Section: <Outlier_Rules>
            if (ReadRules(lRecipeStream, lPATRecipe.GetOptions(), lPATRecipe.GetUniVariateRules()) == false)
                return false;
        }
    }
    while(lRecipeStream.atEnd() == false);

    // No error...unless recipe was empty!
    if(lSectionFound == 0)
    {
        mErrorMessage = "Failed loading recipe file... Probably not a valid recipe file.\nFile: " + mRecipeName;
        return false;
    }

    // Build outlier failure type for PAT soft and hard bins
    BuildPATBinsFailType(lPATRecipe.GetOptions(), lPATRecipe.GetUniVariateRules(), lPATRecipe.GetMultiVariateRules());

    return true;
}

bool PATRecipeIOCsv::ReadOptions(QTextStream &lRecipeStream, COptionsPat &lPATRecipe)
{
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("PAT ReadOptions from device %1")
          .arg(lRecipeStream.device()->metaObject()->className()).toLatin1().data());

    bool	lOk             = false;
    int		lIdx            = 0;
    QString	lLine;
    QString	lSection;
    QString	lKeyword;
    QString	lValue;
    PATOptionReticle lReticle;

    // Set the old default rule name
    lReticle.SetRuleName("Rule-1");

    do
    {
        // Read line.
        lLine   = lRecipeStream.readLine().trimmed();

        if (lLine.isEmpty())
            continue;

        // If end of section, return
        if(lLine.startsWith(GEX_TPAT_CLOSE_SECTION_OPTIONS, Qt::CaseInsensitive))
            return true;

        lKeyword    = lLine.section(',',0,0).toLower();

        if(lKeyword.compare("Product", Qt::CaseInsensitive) == 0)
        {
            QFileInfo lFile(mRecipeName);

            // Do not read Product ID from default config File (5951)
            if (lFile.fileName() != GS::Gex::PAT::GetDefaultRecipeName(lPATRecipe.GetRecipeType()))
                lPATRecipe.strProductName = lLine.section(',',1,1);
            else
                lPATRecipe.strProductName.clear();
        }
        else if(lKeyword.compare("RecipeVersion", Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.iRecipeVersion = lSection.toInt();
            lSection = lLine.section(',',2,2);
            lPATRecipe.iRecipeBuild= lSection.toInt(&lOk);
            if(lOk == false)
                lPATRecipe.iRecipeBuild = DEFAULT_RECIPE_BUILD;
        }
        else if(lKeyword.compare("StaticPatRule", Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.bStaticPAT = true;
            else
                lPATRecipe.bStaticPAT = false;
        }
        else if(lKeyword.compare("StaticPatBin", Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.iFailStatic_SBin = lPATRecipe.iFailStatic_HBin = lSection.toLong();
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty() == false)
                lPATRecipe.iFailStatic_HBin = lSection.toLong();
        }
        else if(lKeyword.compare("DynamicPatRule", Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.bDynamicPAT = true;
            else
                lPATRecipe.bDynamicPAT = false;
        }
        else if(lKeyword.compare("DynamicPatBin", Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.iFailDynamic_SBin = lPATRecipe.iFailDynamic_HBin = lSection.toLong();
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty() == false)
                lPATRecipe.iFailDynamic_HBin = lSection.toLong();
        }
        else if(lKeyword.compare("StaticFailColor", Qt::CaseInsensitive) == 0)
        {
            // Default color: Red
            int iR=255;
            int iG=0;
            int iB=0;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.cStaticFailColor = QColor(iR,iG,iB);
        }
        else if(lKeyword.compare("DynamicFailColor", Qt::CaseInsensitive) == 0)
        {
            // Default color: Red
            int iR=255;
            int iG=0;
            int iB=0;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.cDynamicFailColor = QColor(iR,iG,iB);
        }
        else if(lKeyword.compare("CustomAlgorithmLibEnabled", Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.bCustomPatLib = true;
            else
                lPATRecipe.bCustomPatLib = false;
        }
        else if(lKeyword.compare("CustomAlgorithmLibName", Qt::CaseInsensitive) == 0)
        {
            lPATRecipe.strCustomPatLibName = lLine.section(',',1,1);
        }
        else if(lKeyword.compare("CustomAlgorithmLibFile", Qt::CaseInsensitive) == 0)
        {
            lPATRecipe.strCustomPatLibFile = lLine.section(',',1,1);
        }
        else if(lKeyword.compare("CustomAlgorithmLibRevision", Qt::CaseInsensitive) == 0)
        {
            lPATRecipe.strCustomPatLibRevision = lLine.section(',',1,1);
        }
        else if(lKeyword.compare("BadClusterCustomLib", Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.mGDBNCustomLib = true;
            else
                lPATRecipe.mGDBNCustomLib = false;
        }
        else if(lKeyword.compare("GoodBinList", Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lLine = lLine.trimmed();
            if(lLine.isEmpty())
                lLine = "1";	// Force good bin = Bin1 if invalid string extracted.

            // Erase any previous Bin list stored.
            if(lPATRecipe.pGoodSoftBinsList != NULL)
                delete lPATRecipe.pGoodSoftBinsList;

            lPATRecipe.pGoodSoftBinsList = new GS::QtLib::Range(lLine.toLatin1().constData());
        }
        else if(lKeyword.compare("GoodHardBinList",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lLine = lLine.trimmed();
            if(lLine.isEmpty())
                lLine = "1";	// Force good Hard bin = Bin1 if invalid string extracted.

            // Erase any previous Bin list stored.
            if(lPATRecipe.pGoodHardBinsList != NULL)
                delete lPATRecipe.pGoodHardBinsList;

            lPATRecipe.pGoodHardBinsList = new GS::QtLib::Range(lLine.toLatin1().constData());
        }
        else if(lKeyword.compare("TestKey",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mOptionsTestKey   = lLine.toLong();
            lPATRecipe.mTestKey          = lLine.toLong();
        }
        else if(lKeyword.compare("PatLimitsFromBin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lSection = lSection.trimmed();
            if(lSection.isEmpty())
                lSection = "goodbins";	// Force good bin
            lSection = lSection.toLower();
            if(lSection == "all")
                lPATRecipe.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_ALLBINS;
            else
            if(lSection == "goodbins")
                lPATRecipe.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_GOODSOFTBINS;
            else
            if(lSection == "bins")
                lPATRecipe.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_LISTSOFTBINS;
            else
            if(lSection == "allbins_except")
                lPATRecipe.iPatLimitsFromBin = GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS;
            lSection = lLine.section(',',2);
            lSection = lSection.remove(";;"); // Remove any double ;;
            lPATRecipe.strPatLimitsFromBin = lSection.trimmed();
        }
        else if(lKeyword.compare("MinimumSamples",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iMinimumSamples = lLine.toLong();
        }
        else if(lKeyword.compare("IgnoreIQR0",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.bIgnoreIQR0 = true;
            else
                lPATRecipe.bIgnoreIQR0 = false;
        }
        else if(lKeyword.compare("IgnoreH_Category",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong() > 0)
                lPATRecipe.bIgnoreHistoricalCategories = true;
            else
                lPATRecipe.bIgnoreHistoricalCategories = false;
        }
        else if(lKeyword.compare("IgnoreH_Cpk",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.lfIgnoreHistoricalCpk = lLine.toDouble(&lOk);
            if(!lOk)
                lPATRecipe.lfIgnoreHistoricalCpk = -1;

            if (lPATRecipe.lfSmart_IgnoreHighCpk < 0)
                lPATRecipe.mSmart_IgnoreHighCpkEnabled = false;
            else
                lPATRecipe.mSmart_IgnoreHighCpkEnabled = true;

        }
        else if(lKeyword.compare("ScanGoodPartsOnly",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.bScanGoodPartsOnly = true;
            else
                lPATRecipe.bScanGoodPartsOnly = false;
        }
        else if(lKeyword.compare("StickWithinSpecLimits",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bStickWithinSpecLimits = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bStickWithinSpecLimits = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("StopOnFirstFail",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bStopOnFirstFail = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bStopOnFirstFail = false;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_Stats",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_Stats = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_Stats = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_Histo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_Histo = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_Histo = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_Wafermap",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_Wafermap = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_Wafermap = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_WafermapType",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iReport_WafermapType = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.iReport_WafermapType = 0;	// Invalid field, force to default state: SBIN
        }
        else if(lKeyword.compare("Report_Pareto",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_Pareto = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_Pareto = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_Binning",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_Binning = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_Binning = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_SPAT_Limits",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_SPAT_Limits = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_SPAT_Limits = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_DPAT_Limits_Outliers",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_DPAT_Limits_Outliers = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_DPAT_Limits_Outliers = true;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("Report_DPAT_Limits_NoOutliers",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bReport_DPAT_Limits_NoOutliers = lLine.toInt(&lOk);
            if(!lOk)
                lPATRecipe.bReport_DPAT_Limits_NoOutliers = false;	// Invalid field, force to default state
        }
        else if(lKeyword.compare("GaussianShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
        }
        else if(lKeyword.compare("GaussianTailedShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;
        }
        else if(lKeyword.compare("GaussianDoubleTailedShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;
        }
        else if(lKeyword.compare("LogNormalShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_7_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_10_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_14_SIGMA;
        }
        else if(lKeyword.compare("MultiModalShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
        }
        else if(lKeyword.compare("ClampedShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
        }
        else if(lKeyword.compare("DoubleClampedShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_3_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_4_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_6_SIGMA;
        }
        else if(lKeyword.compare("CategoryShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
        }
        else if(lKeyword.compare("UnknownShape",Qt::CaseInsensitive) == 0)
        {
            // Get outlier limits: NEAR outliers
            lSection = lLine.section(',',1,1);
            lPATRecipe.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;
            lSection = lLine.section(',',2,2);
            lPATRecipe.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] = GEX_TPAT_6_SIGMA;

            // Get outlier limits: MEDIUM outliers
            lSection = lLine.section(',',3,3);
            lPATRecipe.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;
            lSection = lLine.section(',',4,4);
            lPATRecipe.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM] = GEX_TPAT_9_SIGMA;

            // Get outlier limits: FAR outliers
            lSection = lLine.section(',',5,5);
            lPATRecipe.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
            lSection = lLine.section(',',6,6);
            lPATRecipe.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = fabs(lSection.toDouble(&lOk));
            if(!lOk)
                // Invalid field, force meaningfull value.
                lPATRecipe.lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR] = GEX_TPAT_12_SIGMA;
        }
        else if(lKeyword.compare("GaussianAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_Gaussian = lLine.toInt();
        }
        else if(lKeyword.compare("GaussianTailedAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_GaussianTailed = lLine.toInt();
        }
        else if(lKeyword.compare("GaussianDoubleTailedAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_GaussianDoubleTailed = lLine.toInt();
        }
        else if(lKeyword.compare("LogNormalAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_LogNormal = lLine.toInt();
        }
        else if(lKeyword.compare("MultiModalAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_MultiModal = lLine.toInt();
        }
        else if(lKeyword.compare("ClampedAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_Clamped = lLine.toInt();
        }
        else if(lKeyword.compare("DoubleClampedAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_DoubleClamped = lLine.toInt();
        }
        else if(lKeyword.endsWith("CategoryAlgo", Qt::CaseInsensitive))
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_Category = lLine.toInt();
        }
        else if(lKeyword.endsWith("UnknownAlgo", Qt::CaseInsensitive))
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iAlgo_Unknown = lLine.toInt();
        }
        else if(lKeyword.compare("GaussianPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_Gaussian = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("GaussianTailedPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_GaussianTailed = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("GaussianDoubleTailedPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_GaussianDoubleTailed = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("LogNormalPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_LogNormal = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("MultiModalPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_MultiModal = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("ClampedPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_Clamped = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("DoubleClampedPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_DoubleClamped = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("CategoryPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_Category = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("UnknownPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bPAT_Unknown= (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("MinConfidenceThreshold",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mMinConfThreshold= (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("IgnoreHighCpk",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.lfSmart_IgnoreHighCpk = lLine.toDouble(&lOk);
            if(!lOk)
                lPATRecipe.lfSmart_IgnoreHighCpk = -1;	// Invalid field, disable option!
        }
        else if(lKeyword.compare("AssumeIntegerCategory",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bAssumeIntegerCategory = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("CategoryValueCount",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iCategoryValueCount = lLine.toInt();
        }
        else if(lKeyword.compare("MinimumOutliersToFail",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iMinimumOutliersPerPart = lLine.toLong();
        }
        else if(lKeyword.compare("NNR_Enabled",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.SetNNREnabled((bool)(lLine.toInt()));
        }
        else if(lKeyword.compare("NRR_ClusterBin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.SetNNRSoftBin(lSection.toLong());
            lPATRecipe.SetNNRHardBin(lSection.toLong());
            lSection = lLine.section(',',2,3);
            if(lSection.isEmpty() == false)
                lPATRecipe.SetNNRHardBin(lSection.toLong());
        }
        else if(lKeyword.compare("NNR_Color",Qt::CaseInsensitive) == 0)
        {
            // Default color: pale pink
            int iR=255;
            int iG=200;
            int iB=180;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.SetNNRColor(QColor(iR,iG,iB));
        }
        else if(lKeyword.startsWith("NNR_RuleID_", Qt::CaseInsensitive))
        {
            // Format: Rulename,
            CNNR_Rule lNnrRule;;

            // Rule name
            QString lRuleName       = lLine.section(',',1,1).trimmed();
            QString lBaseRuleName   = lRuleName;
            int     lBaseIndex      = 1;

            // Ensure the rule name is unique
            while(lPATRecipe.FindNNRRuleByName(lRuleName) != -1)
            {
                lRuleName = lBaseRuleName + "-" + QString::number(lBaseIndex);
                ++lBaseIndex;
            }
            lNnrRule.SetRuleName(lRuleName);

            lNnrRule.SetLA(lLine.section(',',2,2).trimmed().toDouble());	// Test density
            lNnrRule.SetAlgorithm(lLine.section(',',3,3).trimmed().toInt());		// NNR Algo.
            lNnrRule.SetNFactor(lLine.section(',',4,4).trimmed().toDouble());		// N factor
            lNnrRule.SetClusterSize(lLine.section(',',5,5).trimmed().toInt());		// NNR Algo.

            switch(lNnrRule.GetClusterSize())
            {
                case -1:
                case 5:
                case 7:
                case 9:
                case 11:
                case 13:
                case 15:    lOk = true;
                            break;

                default:    lOk = false;
                            break;

            }

            if(!lOk)
            {
                // Invalid line...
                mErrorMessage = "Outliers configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid 'NNR cluster size' (";
                mErrorMessage += QString::number(lNnrRule.GetClusterSize()) + ").";
                return false;
            }

            // Rule Enabled
            lNnrRule.SetIsEnabled((bool) lLine.section(',',7,7).toInt(&lOk));
            if(!lOk)
                lNnrRule.SetIsEnabled(true); // Make it enabled if missing info.

            lPATRecipe.GetNNRRules().append(lNnrRule);
        }
        else if(lKeyword.compare("IDDQ_Delta_Enabled",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mIsIDDQ_Delta_enabled = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("IDDQ_Delta_Bin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.mIDDQ_Delta_SBin = lPATRecipe.mIDDQ_Delta_HBin = lSection.toLong();
            lSection = lLine.section(',',2,3);
            if(lSection.isEmpty() == false)
                lPATRecipe.mIDDQ_Delta_HBin = lSection.toLong();
        }
        else if(lKeyword.compare("IDDQ_Delta_Color",Qt::CaseInsensitive) == 0)
        {
            // Default color: pale pink
            int iR=255;
            int iG=200;
            int iB=180;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.mIDDQ_Delta_Color = QColor(iR,iG,iB);
        }
        else if(lKeyword.startsWith("IDDQ_Delta_RuleID_", Qt::CaseInsensitive))
        {
            // Format: Rulename,
            CIDDQ_Delta_Rule lRule;

            // Rule name
            lRule.SetRuleName(lLine.section(',',1,1).trimmed());
            lRule.SetPreStress(lLine.section(',',2,2).trimmed());			// Pre-Stress
            lRule.SetPostStress(lLine.section(',',3,3).trimmed());			// Post-Stress
            lRule.SetCaseSensitive((lLine.section(',',4,4).trimmed().toInt() != 0) ? true:false);
            // IDDQ-Delta Algorithm (N*Sigma, IQR,...)
            lRule.SetAlgorithm(lLine.section(',',5,5).trimmed().toInt());
            lRule.SetNFactor(lLine.section(',',6,6).trimmed().toDouble());	// N factor
            //ptRule->iDataSource = lLine.section(',',7,7).trimmed().toInt();				// Data source
            // Rule Enabled
            lRule.SetIsEnabled((bool) lLine.section(',',8,8).toInt(&lOk));
            if(!lOk)
                lRule.SetIsEnabled(true); // Make it enabled if missing info.

            lPATRecipe.mIDDQ_Delta_Rules.append(lRule);
        }
        else if(lKeyword.compare("EWS_Flow_Names",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1).toUpper();
            lPATRecipe.strFlowNames = lLine.split(",",QString::KeepEmptyParts,Qt::CaseInsensitive);
        }
        else if(lKeyword.compare("EWS_Flow_Comments",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1);
            lPATRecipe.strFlowComments = lLine.split(",",QString::KeepEmptyParts,Qt::CaseInsensitive);
        }
        else if(lKeyword.compare("EWS_Flow_Kill_STDF_SBIN",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1);
            lPATRecipe.strFlowKillSTDF_SBIN = lLine.split(",",QString::KeepEmptyParts,Qt::CaseInsensitive);
        }
        else if(lKeyword.compare("EWS_Flow_Kill_STDF_HBIN",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1);
            lPATRecipe.strFlowKillSTDF_HBIN = lLine.split(",",QString::KeepEmptyParts,Qt::CaseInsensitive);
        }
        else if(lKeyword.compare("EWS_Flow_Kill_MAP_BIN",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1);
            lPATRecipe.strFlowKillMAP_BIN = lLine.split(",",QString::KeepEmptyParts,Qt::CaseInsensitive);
        }
        else if(lKeyword.compare("UPDATE_MAP_Flow_Name",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iUpdateMap_FlowNameID = lLine.toLong();
            if(lPATRecipe.iUpdateMap_FlowNameID <0)
                lPATRecipe.iUpdateMap_FlowNameID = 0;
        }
        else if(lKeyword.compare("EWS_Rules_Precedence",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1);
            lPATRecipe.strRulePrecedence = lLine.split(",",QString::SkipEmptyParts,Qt::CaseInsensitive);
        }
        else if(lKeyword.compare("GPAT_IgnorePPatBins",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mGPAT_IgnorePPatBins = (bool) lLine.toInt();
        }
        else if(lKeyword.compare("BadClusterRule",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            if(lLine.toLong())
                lPATRecipe.mIsGDBNEnabled = true;
            else
                lPATRecipe.mIsGDBNEnabled = false;
        }
        else if(lKeyword.compare("BadClusterBin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.mGDBNPatSBin = lPATRecipe.mGDBNPatHBin = lSection.toLong();
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty() == false)
                lPATRecipe.mGDBNPatHBin = lSection.toLong();
        }
        else if(lKeyword.compare("BadClusterColor",Qt::CaseInsensitive) == 0)
        {
            // Default color: pale pink
            int iR=255;
            int iG=200;
            int iB=180;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.mGDBNColor = QColor(iR,iG,iB);
        }
        else if(lKeyword.compare("BadClusterYield",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule;
            lLine = lLine.section(',',1,1);
            lRule.mYieldThreshold = lLine.toDouble();
            lPATRecipe.mGDBNRules.append(lRule);
            lPATRecipe.mIsGDBNEnabled = true;
        }
        else if(lKeyword.compare("BadClusterOverSoftBins",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mWafermapSource = lLine.toLong();
            if(lRule.mWafermapSource<0)
                lRule.mWafermapSource = 0;
        }
        else if(lKeyword.compare("BadClusterSize",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mClusterSize = lLine.toLong();
            switch(lRule.mClusterSize)
            {
                case 3:
                case 5:
                case 7:
                    break;
                default:
                    lRule.mYieldThreshold = 100;
                    lRule.mClusterSize = -1;	// Disable it as we've god an invalid value
            }
            lRule.mFailCount = 2;
        }
        else if(lKeyword.compare("BadClusterCount",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mFailCount = lLine.toLong();
            if( lRule.mFailCount < 1)
                lRule.mFailCount = 2;	// Default is 2 bad neighbors
            if(lRule.mFailCount >= lRule.mClusterSize*lRule.mClusterSize)
                lRule.mFailCount = lRule.mClusterSize-1;
        }
        else if (lKeyword.compare("BadBinNeighbors", Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lLine = lLine.trimmed();
            if(lLine.isEmpty())
                lLine = "0,2-65535";	// Force bad bins list of neighbors.

            // Erase any previous Bin list stored.
            if(lRule.mBadBinList != NULL)
                delete lRule.mBadBinList;

            lRule.mBadBinList = new GS::QtLib::Range(lLine.toLatin1().constData());
        }
        else if(lKeyword.compare("BadNeighborsAlgorithm",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mAlgorithm = lLine.toInt();
        }
        else if(lKeyword.compare("BadNeighborsFailWaferEdge",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mFailWaferEdges = (bool)(lLine.toInt());
        }
        else if(lKeyword.compare("BadClusterAdjacentWeighting",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mAdjWeightLst.clear();
            lRule.mAdjWeightLst.append(lLine.toInt());
        }
        else if(lKeyword.compare("BadClusterDiagonalWeighting",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mDiagWeightLst.clear();
            lRule.mDiagWeightLst.append(lLine.toInt());
        }
        else if(lKeyword.compare("BadClusterMinimumWeighting",Qt::CaseInsensitive) == 0)
        {
            CGDBN_Rule lRule = lPATRecipe.mGDBNRules.first();
            lLine = lLine.section(',',1,1);
            lRule.mMinimumWeighting = lLine.toInt();
        }
        else if(lKeyword.startsWith("GDBN_RuleID_", Qt::CaseInsensitive))
        {
            // Format: Rulename,
            CGDBN_Rule lRule;

            // Rule name
            lRule.mRuleName = lLine.section(',',1,1).trimmed();
            lRule.mYieldThreshold = lLine.section(',',2,2).trimmed().toDouble();

            // Binlist
            lSection = lLine.section(',',3,3).trimmed();
            if(lSection.isEmpty())
                lSection = "0;2-65535";	// Force bad bins list
            if(lRule.mBadBinList != NULL)
                delete lRule.mBadBinList;
            lRule.mBadBinList = new GS::QtLib::Range(lSection.toLatin1().constData());

            lRule.mWafermapSource = lLine.section(',',4,4).trimmed().toInt();
            lRule.mAlgorithm = lLine.section(',',5,5).trimmed().toInt();
            lRule.mFailWaferEdges = (lLine.section(',',6,6).trimmed().toInt()) ? true:false;
            // Squeeze algorithm
            lRule.mClusterSize = lLine.section(',',7,7).trimmed().toInt();
            lRule.mFailCount = lLine.section(',',8,8).trimmed().toInt();
            // Weithing algorithm
            // Adjacent weights for bad dies
            lSection = lLine.section(',',9,9).trimmed();
            lRule.mAdjWeightLst.clear();
            for(lIdx=0;lIdx <= lSection.count("/");lIdx++)
            {
                lValue = lSection.section('/',lIdx,lIdx).trimmed();
                lRule.mAdjWeightLst << lValue.toInt();
            }

            // Diagonal weight for bad dies
            lSection = lLine.section(',',10,10).trimmed();
            lRule.mDiagWeightLst.clear();
            for(lIdx=0;lIdx <= lSection.count("/");lIdx++)
            {
                lValue = lSection.section('/',lIdx,lIdx).trimmed();
                lRule.mDiagWeightLst << lValue.toInt();
            }

            lRule.mMinimumWeighting = lLine.section(',',11,11).trimmed().toInt();
            lRule.mEdgeDieWeighting = lLine.section(',',12,12).trimmed().toInt();
            lRule.mEdgeDieWeightingScale = lLine.section(',',13,13).trimmed().toDouble();

            // Rule Enabled
            lRule.mIsEnabled = (bool) lLine.section(',',14,14).toInt(&lOk);
            if(!lOk)
                lRule.mIsEnabled = true; // Make it enabled if missing info.

            // Mask
            lValue = lLine.section(',',15,15);
            if(lValue.isEmpty() == false)
                lRule.mMaskName = lValue;

            //Edge die type
            lRule.mEdgeDieType = lLine.section(',', 16, 16).trimmed().toInt();

            lPATRecipe.mGDBNRules.append(lRule);
            lPATRecipe.mIsGDBNEnabled = true;
        }
        else if(lKeyword.compare("ReticleEnabled",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.SetReticleEnabled((bool) lLine.toLong());
            lReticle.SetReticleEnabled((bool) lLine.toLong());
        }
        else if(lKeyword.compare("ReticleWafermapSource",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);

            switch(lLine.toLong())
            {
                case 0:
                        lReticle.SetReticle_WafermapSource(GEX_PAT_WAFMAP_SRC_HARDBIN);
                        break;

                case 1:
                        lReticle.SetReticle_WafermapSource(GEX_PAT_WAFMAP_SRC_SOFTBIN);
                        break;

                case 2:
                        lReticle.SetReticle_WafermapSource(GEX_PAT_WAFMAP_SRC_PROBER);
                        break;

                default:
                        lReticle.SetReticle_WafermapSource(GEX_PAT_WAFMAP_SRC_SOFTBIN);
                        break;
            }
        }
        else if(lKeyword.compare("ReticleYieldThreshold",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lReticle.SetReticleYieldThreshold(lLine.toDouble());
        }
        else if(lKeyword.compare("ReticleSizeX",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.SetReticleSizeX(lLine.toLong());
        }
        else if(lKeyword.compare("ReticleSizeY",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.SetReticleSizeY(lLine.toLong());
        }
        else if(lKeyword.compare("BadBinReticle",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lLine = lLine.trimmed();
            if(lLine.isEmpty())
                lLine = "0,2-65535";	// Force bad bins list of neighbors.

            lReticle.SetBadBinsReticleList(GS::QtLib::Range(lLine.toLatin1().constData()));
        }
        else if(lKeyword.compare("ReticleBin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.SetReticleSoftBin(lSection.toLong());
            lPATRecipe.SetReticleHardBin(lSection.toLong());
            lReticle.SetReticleHBin(lSection.toLong());
            lReticle.SetReticleSBin(lSection.toLong());
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty() == false)
            {
                lPATRecipe.SetReticleHardBin(lSection.toLong());
                lReticle.SetReticleHBin(lSection.toLong());
            }
        }
        else if(lKeyword.compare("ReticleColor",Qt::CaseInsensitive) == 0)
        {
            // Default color: pale pink
            int iR=255;
            int iG=0;
            int iB=255;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);

            lPATRecipe.SetReticleColor(QColor(iR,iG,iB));
            lReticle.SetReticleColor(QColor(iR,iG,iB));
        }
        else if(lKeyword.compare("BadBinZPAT",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lLine = lLine.trimmed();
            if(lLine.isEmpty())
                lLine = "0,2-65535";	// Force bad bins list.

            // Erase any previous Bin list stored.
            if(lPATRecipe.pBadBinsZPAT_List != NULL)
                delete lPATRecipe.pBadBinsZPAT_List;

            lPATRecipe.pBadBinsZPAT_List = new GS::QtLib::Range(lLine.toLatin1().constData());
        }
        else if(lKeyword.compare("ZPAT_OnSoftBin",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bZPAT_SoftBin = (bool) lLine.toLong();
        }
        else if(lKeyword.compare("CompositeExclusionZoneYield",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.lfCompositeExclusionZoneYieldThreshold = lLine.toDouble();

            if (lPATRecipe.lfCompositeExclusionZoneYieldThreshold > 0)
                lPATRecipe.SetExclusionZoneEnabled(true);
            else
                lPATRecipe.SetExclusionZoneEnabled(false);
        }
        else if(lKeyword.compare("CompositeZoneBin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.iCompositeZone_SBin = lPATRecipe.iCompositeZone_HBin = lSection.toLong();
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty()==false)
                lPATRecipe.iCompositeZone_HBin = lSection.toLong();
        }
        else if(lKeyword.compare("MergeEtestStdf",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bMergeEtestStdf = (bool) lLine.toLong();
        }
        else if(lKeyword.compare("MergeEtestStdfBin",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.iCompositeEtestStdf_SBin = lPATRecipe.iCompositeEtestStdf_HBin = lSection.toLong();
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty()==false)
                lPATRecipe.iCompositeEtestStdf_HBin = lSection.toLong();
        }
        else if(lKeyword.compare("ZPAT_GDBN_Enabled", Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bZPAT_GDBN_Enabled = (bool) lLine.toLong();
        }
        else if(lKeyword.compare("ZPAT_Reticle_Enabled",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bZPAT_Reticle_Enabled = (bool) lLine.toLong();
        }
        else if(lKeyword.compare("ZPAT_Clustering_Enabled",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.bZPAT_Clustering_Enabled = (bool) lLine.toLong();
        }
        else if(lKeyword.compare("ZPAT_Color",Qt::CaseInsensitive) == 0)
        {
            // Default color: pale pink
            int iR=255;
            int iG=0;
            int iB=255;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.cZpatColor = QColor(iR,iG,iB);
        }
        else if(lKeyword.compare("ClusteringPotatoEnabled",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mClusteringPotato = (bool) lLine.toInt();
            // Ensure we clear any previous rule history...
            lPATRecipe.mClusterPotatoRules.clear();
        }
        else if(lKeyword.compare("BadBinClusteringPotato",Qt::CaseInsensitive) == 0)
        {
            lSection = lLine.section(',',1,1);
            lPATRecipe.mClusteringPotatoSBin = lPATRecipe.mClusteringPotatoHBin = lSection.toLong();
            lSection = lLine.section(',',2,2);
            if(lSection.isEmpty()==false)
                lPATRecipe.mClusteringPotatoHBin = lSection.toLong();
        }
        else if(lKeyword.compare("BadClusteringPotatoColor",Qt::CaseInsensitive) == 0)
        {
            // Default color: pale pink
            int iR=255;
            int iG=0;
            int iB=255;
            lLine = lLine.section(',',1,1);
            if(lLine.isEmpty() == false)
                sscanf(lLine.toLatin1().constData(),"%d %d %d",&iR,&iG,&iB);
            lPATRecipe.mClusteringPotatoColor = QColor(iR,iG,iB);
        }
        else  if(lKeyword.startsWith("ClusteringPotato_RuleID_", Qt::CaseInsensitive))
        {
            // Format: Rulename, Binlist, BinType, ClusterSize, Outline width, Adj.weight,
            // Diag.weight, Fail weight, Ignore one-die line, Ignore one-die cols,
            CClusterPotatoRule lClusteringRule;

            // Rule name
            lClusteringRule.mRuleName = lLine.section(',',1,1).trimmed();

            // Binlist to Identify a Cluster
            lSection = lLine.section(',',2,2).trimmed();
            if(lSection.isEmpty())
                lSection = "0;2-65535";	// Force bad bins list
            if(lClusteringRule.mBadBinIdentifyList != NULL)
                delete lClusteringRule.mBadBinIdentifyList;
            lClusteringRule.mBadBinIdentifyList= new GS::QtLib::Range(lSection.toLatin1().constData());

            // Wafer source: STDF Softbin (0) STDF Hardbin (1), Prober map (2)
            lSection = lLine.section(',',3,3).trimmed();
            lClusteringRule.mWaferSource = lSection.toInt();

            // Cluster size
            // Note: negative number is '%' of wafer die count
            lClusteringRule.mClusterSize = lLine.section(',',4,4).toDouble();

            // Outline width
            // Number of good-die rings to remove around the cluster (default is 1)
            lClusteringRule.mOutlineWidth = lLine.section(',',5,5).toInt();

            // Light outline ?
            lClusteringRule.mIsLightOutlineEnabled = (bool) lLine.section(',',6,6).toInt();

            // Adjacent weights for bad dies
            lSection = lLine.section(',',7,7);
            lClusteringRule.mAdjWeightLst.clear();
            for(lIdx=0;lIdx <= lSection.count("/");lIdx++)
            {
                lValue = lSection.section('/',lIdx,lIdx).trimmed();
                lClusteringRule.mAdjWeightLst << lValue.toInt();
            }

            // Diagonal weight for bad dies
            lSection = lLine.section(',',8,8);
            lClusteringRule.mDiagWeightLst.clear();
            for(lIdx=0;lIdx <= lSection.count("/");lIdx++)
            {
                lValue = lSection.section('/',lIdx,lIdx).trimmed();
                lClusteringRule.mDiagWeightLst << lValue.toInt();
            }

            // Fail weight threshold of bad dies.
            lClusteringRule.mFailWeight = lLine.section(',',9,9).toInt();

            // Ignore one-die scratch lines?
            // True if ignore one-die scratch lines
            lClusteringRule.mIgnoreScratchLines = (bool) lLine.section(',',10,10).toInt();

            // Ignore one-die scratch rows?
            // True if ignore one-die scratch rows
            lClusteringRule.mIgnoreScratchRows = (bool) lLine.section(',',11,11).toInt();

            // Ignore diagonal bad dies?
            // True if ignore diagonal bad dies when doing cluster identification
            lClusteringRule.mIgnoreDiagonalBadDies = (bool) lLine.section(',',12,12).toInt();

            // Rule Enabled
            lClusteringRule.mIsEnabled = (bool) lLine.section(',',13,13).toInt(&lOk);
            if(!lOk)
                lClusteringRule.mIsEnabled = true; // Make it enabled if missing info.

            // Outline matrix size
            lClusteringRule.mOutlineMatrixSize = lLine.section(',',14,14).toInt(&lOk);
            if(!lOk)
                lClusteringRule.mOutlineMatrixSize = 0; // Make it default: 3x3 matrix.

            // Mask
            lValue = lLine.section(',',15,15);
            if(lValue.isEmpty() == false)
                lClusteringRule.mMaskName = lValue;

            // Edge-die rule type
            lValue = lLine.section(',',16,16);
            if(lValue.isEmpty() == false)
                lClusteringRule.mEdgeDieWeighting = lValue.toInt(&lOk);

            // Edge-die factor
            lValue = lLine.section(',',17,17);
            if(lValue.isEmpty() == false)
                lClusteringRule.mEdgeDieWeightingScale = lValue.toDouble(&lOk);

            // Edge-die type
            lValue = lLine.section(',',18,18);
            if(lValue.isEmpty() == false)
                lClusteringRule.mEdgeDieType = lValue.toInt(&lOk);

            // Binlist Inking a Cluster
            lSection = lLine.section(',',19,19).trimmed();
            if(lSection.isEmpty())
                lSection = lClusteringRule.mBadBinIdentifyList->GetRangeList(';');	// Force bad bins list
            if(lClusteringRule.mBadBinInkingList != NULL)
                delete lClusteringRule.mBadBinInkingList;
            lClusteringRule.mBadBinInkingList= new GS::QtLib::Range(lSection.toLatin1().constData());

            lPATRecipe.mClusterPotatoRules.append(lClusteringRule);
        }
        else if(lKeyword.startsWith("Mask_RuleID_", Qt::CaseInsensitive))
        {
            // Format: Rulename, Enabled, Areatype, Radius
            CMask_Rule *ptRule = new CMask_Rule();;

            // Mask Rule name
            ptRule->mRuleName = lLine.section(',',1,1).trimmed();

            // Enabled?
            ptRule->mIsEnabled = (bool) lLine.section(',',2,2).toInt();

            // Mask area type
            ptRule->mWorkingArea = lLine.section(',',3,3).toInt();

            // Radius
            ptRule->mRadius = lLine.section(',',4,4).toInt();

            lPATRecipe.mMaskRules.append(ptRule);
        }
        else if(lKeyword.compare("FT_BaseLine",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_BaseLine = lLine.toLong(&lOk);

            if (!lOk)
            {
                mErrorMessage = QString("Value %1 for key %2 is not numerical").arg(lLine).arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_BaseLine < FT_PAT_BASELINE_MIN_SIZE)
            {
                mErrorMessage = QString("Baseline value (%1) is lesser than the minimum value allowed (%2)")
                                .arg(lPATRecipe.mFT_BaseLine).arg(FT_PAT_BASELINE_MIN_SIZE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_BaseLine > FT_PAT_BASELINE_MAX_SIZE)
            {
                mErrorMessage = QString("Baseline value (%1) is greater than the maximum value allowed (%2)")
                                .arg(lPATRecipe.mFT_BaseLine).arg(FT_PAT_BASELINE_MAX_SIZE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        else if(lKeyword.compare("FT_SubsequentBaseLine",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_SubsequentBL = lLine.toLong(&lOk);

            if (!lOk)
            {
                mErrorMessage = QString("Value %1 for key %2 is not numerical").arg(lLine).arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_SubsequentBL < FT_PAT_SUBSEQUENT_BL_MIN_SIZE)
            {
                mErrorMessage = QString("Subsequent Baseline value (%1) is lesser than the minimum value allowed (%2)")
                                .arg(lPATRecipe.mFT_SubsequentBL).arg(FT_PAT_SUBSEQUENT_BL_MIN_SIZE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_SubsequentBL > FT_PAT_SUBSEQUENT_BL_MAX_SIZE)
            {
                mErrorMessage = QString("Subsequent Baseline value (%1) is greater than the maximum value allowed (%2)")
                                .arg(lPATRecipe.mFT_SubsequentBL).arg(FT_PAT_SUBSEQUENT_BL_MAX_SIZE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        else if(lKeyword.compare("FT_BaseLineAlgo",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_BaseLineAlgo = lLine.toLong(&lOk);

            if (!lOk)
            {
                mErrorMessage = QString("Value %1 for key %2 is not numerical").arg(lLine)
                                .arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_BaseLineAlgo != FT_PAT_SINGLE_SITE_ALGO &&
                lPATRecipe.mFT_BaseLineAlgo != FT_PAT_MERGED_SITE_ALGO)
            {
                mErrorMessage = QString("Invalid value (%1) for keyword %2")
                                .arg(lPATRecipe.mFT_BaseLineAlgo).arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        else  if(lKeyword.compare("FT_BaseLineMaxOutliers",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_BaseLineMaxOutliers = lLine.toLong();
        }
        else  if(lKeyword.compare("FT_MinSamplesPerSite",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_MinSamplesPerSite = lLine.toLong(&lOk);

            if (!lOk)
            {
                mErrorMessage = QString("Value %1 for key %2 is not numerical").arg(lLine)
                                .arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_MinSamplesPerSite < FT_PAT_MINIMUM_SAMPLES_PER_SITE)
            {
                mErrorMessage = QString("Minimum Sample per Site value (%1) is lesser than the minimum value allowed (%2)")
                                .arg(lPATRecipe.mFT_MinSamplesPerSite).arg(FT_PAT_MINIMUM_SAMPLES_PER_SITE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        else if(lKeyword.compare("FT_Tuning",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_Tuning = lLine.toLong();
        }
        else if(lKeyword.compare("FT_TuningType",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_TuningType = lLine.toLong(&lOk);

            if (!lOk)
            {
                mErrorMessage = QString("Value %1 for key %2 is not numerical").arg(lLine)
                                .arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_TuningType != FT_PAT_TUNING_EVERY_N_PARTS &&
                lPATRecipe.mFT_TuningType != FT_PAT_TUNING_EVERY_N_OUTLIERS)
            {
                mErrorMessage = QString("Invalid value (%1) for keyword %2")
                                .arg(lPATRecipe.mFT_TuningType).arg(lKeyword);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }
        }
        else if(lKeyword.compare("FT_TuningSamples",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_TuningSamples = lLine.toLong(&lOk);

            if (!lOk)
            {
                mErrorMessage = QString("Value %1 for key %2 is not numerical").arg(lLine)
                                .arg(lKeyword);

                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

            if (lPATRecipe.mFT_TuningSamples < FT_PAT_TUNING_MIN_SIZE)
            {
                mErrorMessage = QString("Tuning Samples value (%1) is lesser than the minimum value allowed (%2)")
                                .arg(lPATRecipe.mFT_TuningSamples).arg(FT_PAT_TUNING_MIN_SIZE);
                GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
                return false;
            }

        }
        else if(lKeyword.compare("FT_MaxOutlierParts",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_MaxOutlierParts = lLine.toLong();
        }
        else if(lKeyword.compare("FT_YieldLevel",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            // Remove leading '%' character so we can extract the number from the string
            lLine = lLine.replace('%',' ');
            lPATRecipe.mFT_YieldLevel = lLine.toLong();
        }
        else if(lKeyword.compare("FT_CriticalBinsYield",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            // Remove leading '%' character so we can extract the number from the string
            lLine = lLine.replace('%',' ');
            lPATRecipe.mFT_CriticalBinsYield = lLine.toLong();
        }
        else if(lKeyword.compare("FT_CriticalBinsList",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            // Erase any previous Bin list stored.
            if(lPATRecipe.mFT_CriticalBinsList != NULL)
                delete lPATRecipe.mFT_CriticalBinsList;
            lPATRecipe.mFT_CriticalBinsList = new GS::QtLib::Range(lLine.toLatin1().constData());
        }

        // 7103 : deprecated : still read for compatibility but to be removed for 7.2
        else if(lKeyword.compare("FT_Alarm_Timeout_Outliers",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_Outliers = lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_TOO_MUCH_OUTLIERS_IN_BASELINE, QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Timeout_Shapes",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_Shapes = lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_DISTRIB_SHAPE_CHANGED, QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Timeout_PatDrift",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_PatDrift = lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_PAT_DRIFT, QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Timeout_LowGoodYield",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_LowGoodYield= lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_LOW_GOOD_YIELD, QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Timeout_YieldLoss",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_YieldLoss = lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_YIELD_LOSS, QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Timeout_ParamDrift",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_ParamDrift = lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_PARAM_DRIFT, QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Timeout_ParamCpk",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            //lPATRecipe.iFT_Alarm_Timeout_ParamCpk = lLine.toLong();
            lPATRecipe.setProperty(ALARM_SEVERITY_PARAM_CPK, QVariant(lLine.toLongLong()) );
        }
        // 7103
        else if (lKeyword.startsWith("FT_Alarm_Severity", Qt::CaseInsensitive))
        {
            lLine = lLine.section(',',1,1);
            // generic key loader:
            GSLOG(6, QString("PAT Read options : %1 %2").arg(lKeyword).arg(lLine).toLatin1().data() );
            lPATRecipe.setProperty(lKeyword.toLatin1().constData(), QVariant(lLine.toLongLong()) );
        }
        else if(lKeyword.compare("FT_Alarm_Email_Outliers",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_Outliers = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Alarm_Email_Shapes",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_Shapes = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Alarm_Email_PatDrift",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_PatDrift = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Alarm_Email_LowGoodYield",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_LowGoodYield = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Alarm_Email_YieldLoss",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_YieldLoss = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Alarm_Email_ParamDrift",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_ParamDrift = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Alarm_Email_ParamCpk",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.iFT_Alarm_Email_ParamCpk = lLine.toLong();
        }
        else  if(lKeyword.compare("FT_RunsPerPacket",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_RunsPerPacket = lLine.toLong();
        }
        else if(lKeyword.compare("FT_Traceability",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_Traceability = lLine.toLong();

            if (lPATRecipe.mFT_Traceability != GEX_TPAT_TRACEABILITY_HTML)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Unsupported traceability mode %1. Traceability has been disabled")
                      .arg(lPATRecipe.mFT_Traceability).toLatin1().constData());

                lPATRecipe.mFT_Traceability = GEX_TPAT_TRACEABILITY_NONE;
            }
        }
        else if(lKeyword.compare("FT_MailingList",Qt::CaseInsensitive) == 0)
        {
            lPATRecipe.strFT_MailingList = lLine.section(',',1);
            // Remove any ',' at the end!
            while(lPATRecipe.strFT_MailingList.endsWith(","))
                lPATRecipe.strFT_MailingList.chop(1);
        }
        else if(lKeyword.compare("FT_EmailFormat",Qt::CaseInsensitive) == 0)
        {
            lLine = lLine.section(',',1,1);
            lPATRecipe.mFT_EmailFormat = lLine.toLong();
        }
    }
    while(lRecipeStream.atEnd() == false);

    // Add the reticle rule.
    lPATRecipe.GetReticleRules().append(lReticle);

    // Success
    return true;
}

bool PATRecipeIOCsv::ReadRules(QTextStream &lRecipeStream,
                               COptionsPat &lPATRecipe,
                               QHash<QString, CPatDefinition *> &lPATDefinitions)
{
    // Read Static PAT limits file...created manually or with the ToolBox!
    bool            lOk;
    bool            lDisableSPAT        = false;
    char            szUnits[50]         = "";
    int             lArgIdx             = 0;
    int             lKeyOnTestName      = 0;
    int             lKeyOnTestNumber    = 0;
    long            lTestSequenceID     = 0;
    double          lHistoricalCPK;
    QString         lKeyword;
    QString         lLine;
    QStringList     lParsedLine;
    CPatDefinition  lPatDef;	// To hold a PAT definition
    bool            lRemoveSeqName      = mReadOptions.contains("TESTNAME_REMOVE_SEQUENCER");

    do
    {
        // Read line.
        lLine = lRecipeStream.readLine().trimmed();

        // If end of section, return
        if(lLine.startsWith(GEX_TPAT_CLOSE_SECTION_RULES, Qt::CaseInsensitive))
            return true;

        // comment line: ignore it.
        if(lLine.isEmpty() || lLine.startsWith("#"))
            goto next_line;

        // Line format is:
        // <FailStaticBin>,<FailDynamicBin>,<test #>,<Test name>,<LowL>,<HighL>,Distribution Shape,
        // Historical Cpk,Median,Robust Sigma, Samples to ignore, Outliers to keep,
        // Outlier Limits set, Outlier Rule, Outlier space,....
        lParsedLine = lLine.split(",", QString::KeepEmptyParts);
        lArgIdx     = 0;

        if(lParsedLine.count() < 12)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine) + ") : too few parameters in line";
            return false;
        }

        // Clear stucture to hold definition
        lPatDef.clear();

        // Extract Static Fail Bin info from line
        lPatDef.m_lFailStaticBin = lParsedLine[lArgIdx++].trimmed().toLong(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid 'Static Bin' value";
            return false;
        }

        // Extract Dynamic Fail Bin info from line
        lPatDef.m_lFailDynamicBin = lParsedLine[lArgIdx++].trimmed().toLong(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid 'Dynamic Bin' value";
            return false;
        }

        // Extract Testnumber info from line (Test number may include a pinmap index)
        if(lPATRecipe.GetRecipeVersion().isEmpty())
        {
            // It is an old recipe. Keep using the old way of parsing
            if(lParsedLine.at(2) == QChar('*') || lParsedLine.at(lArgIdx).isEmpty())
            {
                // Ignore test number, and only consider test name.
                lPatDef.m_lTestNumber = lPatDef.mPinIndex = (unsigned) GEX_PTEST;
            }
            else
            {
                // Test list key is on 'test number'
                lKeyOnTestNumber++;

                int lStatus = sscanf(lParsedLine[lArgIdx].toLatin1().constData(), "%lu%*c%ld",
                                     &lPatDef.m_lTestNumber, &lPatDef.mPinIndex);

                if(lStatus == 1)
                    lPatDef.mPinIndex = GEX_PTEST;	// No pinmap index (-1 value)
                else if(lStatus == 0)
                {
                    // Invalid line...
                    mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid 'Test Number'";
                    return false;
                }
            }
            // Point to next argument in line
            lArgIdx++;
        }
        else
        {
            bool lOk;
            QString lLine = lParsedLine[lArgIdx++].trimmed();
            lPatDef.m_lTestNumber = lLine.toULong(&lOk);
            if(!lOk || lLine.contains("."))
            {
                // Invalid line...
                mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid 'Test Number'";
                return false;
            }
        }



        //  Extract test name.
        lLine = lParsedLine[lArgIdx++].trimmed();
        // Replace the _; with the comma  (comma being a separator, it can't be written as-is in the file!
        lLine = lLine.replace("_;",",");

        // Only applies to Real-time (where test program such as image reads test names without sequencer name)
        if(lRemoveSeqName)
        {
            // check if Teradyne test name with sequencer string at the end. If so, remove it!
            int lIdx = lLine.indexOf(" <>");
            if(lIdx >= 0)
                lLine.truncate(lIdx);
        }

        lPatDef.m_strTestName = lLine.replace("__","_");

        //        if (lPATRecipe.GetRecipeType()!=GS::Gex::PAT::RecipeFinalTest)
        //        {
        //            // Replace the  double '_' with the '_'. Why ?
        //            lPatDef.m_strTestName = lLine.replace("__","_");

        //        }
        //        else
        //        {
        //            /* Let s not modifying TestName as :
        //            - it prevents TestKey 2 to work and
        //            - it is anyway a bad idea to modify test name between GTM and GTL
        //            */
        //            lPatDef.m_strTestName = lLine;
        //        }

        // Test list key is on 'test name'
        if(!lPatDef.m_strTestName.isEmpty())
            lKeyOnTestName++;

        if((lPatDef.m_lTestNumber == (unsigned)GEX_PTEST) && (lPatDef.m_strTestName.isEmpty()))
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine)
                    + ") : need to define at least a valid test number or test name";
            return false;
        }

        // Get the Low Limit
        lLine = lParsedLine[lArgIdx++].trimmed();
        lPatDef.m_lfLowLimit = -GEX_TPAT_DOUBLE_INFINITE;
        if(lLine.isEmpty() == false)
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf%10s",&lPatDef.m_lfLowLimit,szUnits) == 2)
                lPatDef.m_strUnits = szUnits;	// Read test units
        }

        // Get the High Limit
        lLine = lParsedLine[lArgIdx++].trimmed();
        lPatDef.m_lfHighLimit = GEX_TPAT_DOUBLE_INFINITE;
        if(lLine.isEmpty() == false)
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf%10s",&lPatDef.m_lfHighLimit,szUnits)
                    == 2 && lPatDef.m_strUnits.isEmpty())
                // Read test units unless already read from previous block (when reading Low Limit)
                lPatDef.m_strUnits = szUnits;
        }

        // Get Historical shape
        lPatDef.m_iDistributionShape = PAT::GetDistributionID(lParsedLine[lArgIdx++].trimmed());

        // Get Historical Cpk
        lHistoricalCPK = lParsedLine[lArgIdx++].trimmed().toDouble(&lOk);

        // Extract Median value
        lPatDef.m_lfMedian = lParsedLine[lArgIdx++].trimmed().toDouble(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid Robust mean (Median)";
            return false;
        }

        // Extract Robust sigma value
        lPatDef.m_lfRobustSigma = lParsedLine[lArgIdx++].trimmed().toDouble(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid Robust Sigma";
            return false;
        }

        // Extract Mean value
        lPatDef.m_lfMean = lParsedLine[lArgIdx++].trimmed().toDouble(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid Mean";
            return false;
        }

        // Extract Sigma value
        lPatDef.m_lfSigma = lParsedLine[lArgIdx++].trimmed().toDouble(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid Sigma";
            return false;
        }

        // Extract Range value
        lPatDef.m_lfRange = lParsedLine[lArgIdx++].trimmed().toDouble(&lOk);
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid Range";
            return false;
        }

        // Extract 'Samples to ignore' Rule ("none", "Negative values", "Positive values")
        lLine = lParsedLine[lArgIdx++].trimmed();
        lPatDef.m_SamplesToIgnore= PAT::GetSamplesToIgnoreIndex(lLine, &lOk);
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'Samples To Ignore' rule selected";
            return false;
        }

        // Extract 'Outliers to keep' Rule ("none", "low values", "high values")
        lLine = lParsedLine[lArgIdx++].trimmed();
        lPatDef.m_OutliersToKeep = PAT::GetOutlierToKeepIndex(lLine, &lOk);
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'Outliers To Keep' rule selected";
            return false;
        }

        // Extract 'Outlier Limits set': near, medium, far
        lLine = lParsedLine[lArgIdx++].trimmed();
        lPatDef.m_iOutlierLimitsSet = PAT::GetOutlierLimitsSetIndex(lLine, &lOk);
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'Outlier Limits set' rule selected";
            return false;
        }

        // Extract Outlier Rule (% of limits, N*Sigma, Smart, disabled...)
        lLine = lParsedLine[lArgIdx++].trimmed();
        lPatDef.mOutlierRule = PAT::GetRuleIndex(lLine, &lOk);
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'Outlier Rule' selected";
            return false;
        }

        // Extract Outlier space number (N factor / Head factor)...if it exists!
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no N factor specified, no more fields parsed on line!
        lPatDef.m_lfOutlierNFactor = 0;
        if(lLine.isEmpty())
            lOk = false;
        else
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf",&lPatDef.m_lfOutlierNFactor) == 1)
            {
                // Unless N factor is a cutom limit, remove its sign.
                if(lPatDef.mOutlierRule != GEX_TPAT_RULETYPE_NEWLIMITSID)
                    lPatDef.m_lfOutlierNFactor = fabs(lPatDef.m_lfOutlierNFactor);
                if(lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_SMARTID)
                    lPatDef.mTailMngtRuleType = GEX_TPAT_TAIL_MNGT_LIBERAL;
                else
                    lPatDef.mTailMngtRuleType = GEX_TPAT_TAIL_MNGT_DISABLED;
                lPatDef.m_lfOutlierNFactor = lPatDef.m_lfOutlierNFactor;
                lOk = true;	// Good value read.
            }
            else
                lOk = false;
        }
        if(!lOk &&
            ((lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_SIGMAID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_ROBUSTSIGMAID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_Q1Q3IQRID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_NEWLIMITSID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_RANGEID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_LIMITSID)))
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid/missing 'Outlier N*Factor' value";
            return false;
        }

        // Extract T factor (Tail)
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no N factor specified, no more fields parsed on line!
        lPatDef.m_lfOutlierTFactor = 0;
        if(lLine.isEmpty())
            lOk = false;
        else
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf",&lPatDef.m_lfOutlierTFactor) == 1)
            {
                // Unless T factor is a cutom limit, remove its sign.
                if(lPatDef.mOutlierRule != GEX_TPAT_RULETYPE_NEWLIMITSID)
                    lPatDef.m_lfOutlierTFactor = fabs(lPatDef.m_lfOutlierTFactor);
                lPatDef.m_lfOutlierTFactor = lPatDef.m_lfOutlierTFactor;
                lOk = true;	// Good value read.
            }
            else
                lOk = false;
        }
        // If 'T' factor not specified while a 'N' factor was, then force 'T' = 'N'
        if ((!lOk && (lPatDef.m_lfOutlierNFactor != 0)) &&
                // do not copy if not necessary
                !( (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_SIGMAID)
                  || (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_ROBUSTSIGMAID)
                  || (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_Q1Q3IQRID)
                  || (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_LIMITSID)
                  || (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_RANGEID) )
                )
            lPatDef.m_lfOutlierTFactor = lPatDef.m_lfOutlierNFactor;

        if(!lOk && ((lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_GAUSSIANID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_GAUSSIANTAILID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_GAUSSIANDUALTAILID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_LOGNORMALID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_BIMODALID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_MULTIMODALID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_CLAMPEDID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_DUALCLAMPEDID) ||
            (lPatDef.mOutlierRule == GEX_TPAT_RULETYPE_CATEGORYID)) )
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid/missing 'Outlier T*Factor' value";
            return false;
        }

        ///////////////////////////////////////////////////////
        ///////////// SPC Settings ////////////////////////////
        ///////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // Extract PAT-Median Drift alarm threshold
        lPatDef.m_SPC_PatMedianDriftAlarm = -1;	// Default: disabled
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no more fields available on this line!
        if(lLine.isEmpty())
            lOk = true;	// Ignore if field is empty!
        else
        {
            lPatDef.m_SPC_PatMedianDriftAlarm = fabs(lLine.toDouble(&lOk));
            lOk = true;
        }
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'PAT-Median Drift Alarm' selected";
            return false;
        }

        /////////////////////////////////////////////////////////////////////
        // Extract PatMedian Drift units (% of limits, N*Sigma...)
        lPatDef.m_SPC_PatMedianDriftAlarmUnits = GEX_TPAT_DRIFT_UNITS_NONE;	// Default: no units
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no more fields available on this line!
        if(lLine.isEmpty())
            lOk = true;	// Ignore if field is empty!
        else
        {
            lPatDef.m_SPC_PatMedianDriftAlarmUnits = PAT::GetMedianDriftUnitsIndex(lLine, &lOk);
            lOk = true;
        }
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'PAT-Median Drift units' selected";
            return false;
        }


        /////////////////////////////////////////////////////////////////////
        // Extract Test Mean Drift alarm threshold
        lPatDef.m_SPC_TestMeanDriftAlarm = -1;	// Default: disabled
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no more fields available on this line!
        if(lLine.isEmpty())
            lOk = true;	// Ignore if field is empty!
        else
        {
            lPatDef.m_SPC_TestMeanDriftAlarm = fabs(lLine.toDouble(&lOk));;
            lOk = true;
        }
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'Test Mean Drift Alarm' selected";
            return false;
        }

        /////////////////////////////////////////////////////////////////////
        // Extract Mean Drift units (% of limits, N*Sigma...)
        lPatDef.m_SPC_TestMeanDriftAlarmUnits = GEX_TPAT_DRIFT_UNITS_NONE;	// Default: no units
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no more fields available on this line!
        if(lLine.isEmpty())
            lOk = true;	// Ignore if field is empty!
        else
        {
            lPatDef.m_SPC_TestMeanDriftAlarmUnits = PAT::GetMedianDriftUnitsIndex(lLine, &lOk);
            lOk = true;
        }
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'Test Mean Drift units' selected";
            return false;
        }

        /////////////////////////////////////////////////////////////////////
        // Extract Cpk threshold alarm...if it exists!
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "";	// There is no Cpk specified, no more fields parsed on line!
        lPatDef.m_SPC_CpkAlarm = 0.0;
        if(lLine.isEmpty())
            lOk = true;	// Quielty ignore if empty field
        else
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf",&lPatDef.m_SPC_CpkAlarm) == 1)
            {
                lPatDef.m_SPC_CpkAlarm = fabs(lPatDef.m_SPC_CpkAlarm);
                lOk = true;	// Good value read.
            }
            else
                lOk = false;
        }
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid/missing 'Cpk alarm' value";
            return false;
        }

        /////////////////////////////////////////////////////////////////////
        // Read Low limit scale factor
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "0";	// There is no scale factor defined...then force to 0
        lPatDef.m_llm_scal= 0;
        if(lLine.isEmpty())
            lOk = true;
        else
        {
            if(sscanf(lLine.toLatin1().constData(),"%d",&lPatDef.m_llm_scal) == 1)
                lOk = true;	// Good value read.
            else
                lOk = false;
        }
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid/missing 'Low-Limit scale' value";
            return false;
        }

        /////////////////////////////////////////////////////////////////////
        // Read High limit scale factor
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "0";	// There is no scale factor defined...then force to 0
        lPatDef.m_hlm_scal= 0;
        if(lLine.isEmpty())
            lOk = true;
        else
        {
            if(sscanf(lLine.toLatin1().constData(),"%d",&lPatDef.m_hlm_scal) == 1)
                lOk = true;	// Good value read.
            else
                lOk = false;
        }
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid/missing 'High-Limit scale' value";
            return false;
        }

        /////////////////////////////////////////////////////////////////////
        // Read SPAT rule: AEC (default), or Sigma or Range, etc
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "0";	// Default to AEC SPAT rule
        lPatDef.m_SPATRuleType = GEX_TPAT_SPAT_ALGO_ROBUSTSIGMA;
        if(lLine.isEmpty() == true)
            lOk = true;
        else
        {
            lPatDef.m_SPATRuleType = PAT::GetSPATRuleIndex(lLine, NULL);
            lOk = true;	// Good value read.
        }
        if(!lOk)
        {
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid/missing 'SPAT rule' value";
            return false;
        }

        // Extract Outlier space number (N factor / Head factor)...if it exists!
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "6";	// There is no N factor specified, no more fields parsed on line!
        lPatDef.m_lfSpatOutlierNFactor = 6;
        if(lLine.isEmpty() == false)
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf",&lPatDef.m_lfSpatOutlierNFactor) == 1)
            {
                if(lPatDef.m_SPATRuleType != GEX_TPAT_SPAT_ALGO_NEWLIMITS)
                    lPatDef.m_lfSpatOutlierNFactor = fabs(lPatDef.m_lfSpatOutlierNFactor);
            }
        }

        // Extract T factor (Tail)
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "6";	// There is no T factor specified, no more fields parsed on line!
        lPatDef.m_lfSpatOutlierTFactor = 6;
        if(lLine.isEmpty() == false)
        {
            if(sscanf(lLine.toLatin1().constData(),"%lf",&lPatDef.m_lfSpatOutlierTFactor) == 1)
            {
                if(lPatDef.m_SPATRuleType != GEX_TPAT_SPAT_ALGO_NEWLIMITS)
                    lPatDef.m_lfSpatOutlierTFactor = fabs(lPatDef.m_lfSpatOutlierTFactor);
            }
        }

        // Skip DPAT comment
        lArgIdx++;

        // Skip SPAT comment
        lArgIdx++;

        // Extract NNR rule
        if(lArgIdx < lParsedLine.count())
            lLine = lParsedLine[lArgIdx++].trimmed();
        else
            lLine = "enabled";	// There is no NNR rule, so default to "ENABLED"
        lPatDef.m_iNrrRule = PAT::GetNNRRuleIndex(lLine, &lOk);
        if(lOk == false)
        {
            // Invalid rule specified, not in our list...
            // Invalid line...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "(line " + QString::number(mCurrentLine) + ") : invalid 'NNR Rule' selected";
            return false;
        }

        if(lPATRecipe.GetRecipeVersion().isEmpty())
        {
            lPatDef.SetTestTypeLegacy('P');
            lPatDef.SetPinmapNumber(-1);
        }
        else
        {
            //Extract TestType
            if(lArgIdx < lParsedLine.count())
            {
                lLine = lParsedLine[lArgIdx++].trimmed();
                if(!lLine.isEmpty())
                {
                    lPatDef.SetTestTypeLegacy(lLine.at(0).toLatin1());
                }

                if(lPatDef.GetTestType() == CPatDefinition::UnknownTest
                        || lPatDef.GetTestType() == CPatDefinition::InvalidTest)
                {
                    // Invalid Test Type
                    mErrorMessage = "Outlier Configuration file parsing error :\n File:" + mRecipeName;
                    mErrorMessage += " (line " + QString::number(mCurrentLine) + ") : invalid 'Test Type' value";
                    return false;

                }
            }
            else
            {
                // Invalid Test Type
                mErrorMessage = "Outlier Configuration file parsing error :\nFile: " + mRecipeName;
                mErrorMessage += " (line " + QString::number(mCurrentLine) + ")";
                mErrorMessage += ": 'Test Type' field value can not be found";
                return false;
            }

            //Extract PinIndex
            if(lArgIdx < lParsedLine.count())
            {
                lLine = lParsedLine[lArgIdx++].trimmed();
                long lPinNumber = lLine.toLong(&lOk);
                if(lLine.isEmpty() || !lOk)
                {
                    // Invalid Test PinmapNumber
                    mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                    mErrorMessage += "(" + QString::number(mCurrentLine) + ") : invalid 'PinIndex' value";
                    return false;
                }

                lPatDef.SetPinmapNumber(lPinNumber);
            }
            else
            {
                // Invalid Test PinmapNumber
                mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
                mErrorMessage += "(" + QString::number(mCurrentLine)
                        + ") : invalid 'PinIndex' field value can not be found";
                return false;
            }
        }

        // Check if test number or test name or both
        if(lKeyOnTestName && lKeyOnTestNumber && (lKeyOnTestName != lKeyOnTestNumber))
        {
            // Invalid file...
            mErrorMessage = "Outlier Configuration file parsing error :\nFile:\n" + mRecipeName;
            mErrorMessage += "Some lines provide test numbers/names and some don't. You have to chose either one for all tests";
            return false;
        }

        // Compute key access mode
        if(lPATRecipe.mOptionsTestKey == GEX_TBPAT_KEY_DETECT)
        {
            if(lKeyOnTestName == lKeyOnTestNumber)
                lPATRecipe.mTestKey = GEX_TBPAT_KEY_TESTMIX;
            else if(lKeyOnTestName)
                lPATRecipe.mTestKey = GEX_TBPAT_KEY_TESTNAME;
            else
                lPATRecipe.mTestKey = GEX_TBPAT_KEY_TESTNUMBER;
        }
        else
            lPATRecipe.mTestKey = lPATRecipe.mOptionsTestKey;

        // See if we consider tests based on the 'Test name' or 'test number' (the access key)
        switch(lPATRecipe.mTestKey)
        {
            case GEX_TBPAT_KEY_TESTNUMBER:
                lKeyword = QString::number(lPatDef.m_lTestNumber);
                if(lPatDef.mPinIndex >= 0)
                    lKeyword += "." + QString::number(lPatDef.mPinIndex);
                break;
            case GEX_TBPAT_KEY_TESTNAME:
                lKeyword = lPatDef.m_strTestName.trimmed();
                if(lPatDef.mPinIndex >= 0)
                    lKeyword += "." + QString::number(lPatDef.mPinIndex);
                break;
            case GEX_TBPAT_KEY_TESTMIX:
                lKeyword = lPatDef.m_strTestName.trimmed();
                lKeyword += "." + QString::number(lPatDef.m_lTestNumber);
                if(lPatDef.mPinIndex >= 0)
                    lKeyword += "." + QString::number(lPatDef.mPinIndex);
                break;
        }

        // Compute Static PAT limits...see if disabled test or not
        lDisableSPAT = false;

        // If IQR is 0 (means loads of data are identical),
        // then we may simply ignore PAT on this test (if option is set to do so)
        if(lPatDef.m_lfRobustSigma == 0 && lPATRecipe.bIgnoreIQR0)
            lDisableSPAT = true;

        // Check If Cpk high enough, and option set to ignore tests with high cpk...
        if(lPATRecipe.lfIgnoreHistoricalCpk >=0 && lHistoricalCPK >= lPATRecipe.lfIgnoreHistoricalCpk)
            lDisableSPAT = true;

        // Check if historical distribution shape is 'Categories' and option set to ignore such tests
        if(lPATRecipe.bIgnoreHistoricalCategories && lPatDef.m_iDistributionShape == PATMAN_LIB_SHAPE_CATEGORY)
            lDisableSPAT = true;

        if(lDisableSPAT)
        {
            // Give infinite limits: Static PAT disabled!
            lPatDef.m_lfLowStaticLimit = -GEX_TPAT_DOUBLE_INFINITE;
            lPatDef.m_lfHighStaticLimit = GEX_TPAT_DOUBLE_INFINITE;
        }
        else
        {
            // Compute SPAT limits
            lPatDef.ComputeStaticLimits();
            // Check SPAT limits (exceed std limits?)
            lPatDef.CheckStaticLimits(lPATRecipe);
        }

        // Keep track of test order in configuration file (as it is the execution flow order)
        lPatDef.m_lSequenceID = lTestSequenceID++;

        // Save PAT definition in our list
        // Holds the Static PAT limits for each test
        if (lPATDefinitions.contains(lKeyword) == false)
            lPATDefinitions.insert(lKeyword, new CPatDefinition(lPatDef));
        else
        {
            // Invalid file...
            mErrorMessage = "Outlier Configuration file parsing error :\n" + mRecipeName + "\n";
            mErrorMessage += "Duplicated test rules for";

            switch(lPATRecipe.mTestKey)
            {
                case GEX_TBPAT_KEY_TESTNUMBER:
                    mErrorMessage += " test number " + QString::number(lPatDef.m_lTestNumber);
                    if(lPatDef.mPinIndex >= 0)
                        mErrorMessage += " and pin index " + QString::number(lPatDef.mPinIndex);
                    break;
                case GEX_TBPAT_KEY_TESTNAME:
                    mErrorMessage += " test name '" + lPatDef.m_strTestName.trimmed() + "'";
                    if(lPatDef.mPinIndex >= 0)
                        mErrorMessage += " and pin index " + QString::number(lPatDef.mPinIndex);
                    break;
                case GEX_TBPAT_KEY_TESTMIX:
                    mErrorMessage += " test number " + QString::number(lPatDef.m_lTestNumber);
                    mErrorMessage += " with test name '" + lPatDef.m_strTestName.trimmed() + "'";
                    if(lPatDef.mPinIndex >= 0)
                        mErrorMessage += " and pin index" + QString::number(lPatDef.mPinIndex);
                    break;
            }

            return false;
        }


next_line:
        // Keep track of line number
        mCurrentLine++;
    }
    while(lRecipeStream.atEnd() == false);

    // success
    return true;
}

void PATRecipeIOCsv::UpdateGeographicalRules(COptionsPat &lPATRecipe)
{
    // NNR
    QList<CNNR_Rule>::iterator itNNRBegin = lPATRecipe.GetNNRRules().begin();
    QList<CNNR_Rule>::iterator itNNREnd   = lPATRecipe.GetNNRRules().end();
    for (; itNNRBegin != itNNREnd; ++itNNRBegin)
    {
        (*itNNRBegin).SetSoftBin(lPATRecipe.GetNNRSoftBin());
        (*itNNRBegin).SetHardBin(lPATRecipe.GetNNRHardBin());
        (*itNNRBegin).SetFailBinColor(lPATRecipe.GetNNRColor());
    }

    // IDDQ
    QList<CIDDQ_Delta_Rule>::iterator itIDDQBegin = lPATRecipe.mIDDQ_Delta_Rules.begin();
    QList<CIDDQ_Delta_Rule>::iterator itIDDQEnd   = lPATRecipe.mIDDQ_Delta_Rules.end();
    for (; itIDDQBegin != itIDDQEnd; ++itIDDQBegin)
    {
        (*itIDDQBegin).SetSoftBin(lPATRecipe.mIDDQ_Delta_SBin);
        (*itIDDQBegin).SetHardBin(lPATRecipe.mIDDQ_Delta_HBin);
        (*itIDDQBegin).SetFailBinColor(lPATRecipe.mIDDQ_Delta_Color);
    }

    // GDBN
    QList<CGDBN_Rule>::iterator itGDBNBegin = lPATRecipe.mGDBNRules.begin();
    QList<CGDBN_Rule>::iterator itGDBNEnd   = lPATRecipe.mGDBNRules.end();
    for (; itGDBNBegin != itGDBNEnd; ++itGDBNBegin)
    {
        (*itGDBNBegin).mSoftBin = lPATRecipe.mGDBNPatSBin;
        (*itGDBNBegin).mHardBin = lPATRecipe.mGDBNPatHBin;
        (*itGDBNBegin).mFailBinColor = lPATRecipe.mGDBNColor;
    }

    // Clustering
    QList<CClusterPotatoRule>::iterator itClusteringBegin = lPATRecipe.mClusterPotatoRules.begin();
    QList<CClusterPotatoRule>::iterator itClusteringEnd   = lPATRecipe.mClusterPotatoRules.end();
    for (; itClusteringBegin != itClusteringEnd; ++itClusteringBegin)
    {
        (*itClusteringBegin).mSoftBin = lPATRecipe.mClusteringPotatoSBin;
        (*itClusteringBegin).mHardBin = lPATRecipe.mClusteringPotatoHBin;
        (*itClusteringBegin).mFailBinColor = lPATRecipe.mClusteringPotatoColor;
    }

    // Reticle
    QList<PATOptionReticle>::iterator itReticleBegin = lPATRecipe.GetReticleRules().begin();
    QList<PATOptionReticle>::iterator itReticleEnd   = lPATRecipe.GetReticleRules().end();
    for (; itReticleBegin != itReticleEnd; ++itReticleBegin)
    {
        (*itReticleBegin).SetReticleSBin(lPATRecipe.GetReticleSoftBin());
        (*itReticleBegin).SetReticleHBin(lPATRecipe.GetReticleHardBin());
        (*itReticleBegin).SetReticleColor(lPATRecipe.GetReticleColor());
    }
}


}   // namespace Gex
}   // namespace GS
