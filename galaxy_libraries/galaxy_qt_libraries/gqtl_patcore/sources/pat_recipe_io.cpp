#include <gqtl_log.h>
#include "pat_global.h"
#include "pat_definition.h"
#include "pat_options.h"
#include "pat_recipe_io.h"
#include "pat_rules.h"
#include "pat_recipe.h"
#include "test_defines.h"

#include "gqtl_utils.h"
#include "pat_recipe_io_json.h"
#include "pat_recipe_io_csv.h"

#include <QFile>

#include <math.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace GS
{
namespace Gex
{

PATRecipeIO* PATRecipeIO::CreateRecipeIo(QTextStream & lRecipeStream)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating a recipe IO for given recipe...");

    PATRecipeIO *lRecipeIO=NULL;
    QString lRecipeString;
    lRecipeString = lRecipeStream.readAll();
    for (int i=0; i<lRecipeString.size(); ++i)
    {
        if(lRecipeString[i] == '{')
        {
            lRecipeIO = new PATRecipeIOJson();
            break;
        }
        else if ( (lRecipeString[i] == '<') && (lRecipeString.contains("Outlier_Options")))
        {
            lRecipeIO = new PATRecipeIOCsv();
            break;
        }
    }
    if (lRecipeIO==NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("The input file is not in supported formats (csv/xml or JSON)")
              .toLatin1().constData());
    }
    return lRecipeIO;
}

PATRecipeIO* PATRecipeIO::CreateRecipeIo(const QString & lRecipeFileName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Creating a RecipeIO for file '%1'")
          .arg(lRecipeFileName).toLatin1().constData() );

    // Read file into string
    QFile file;
    file.setFileName(lRecipeFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error opening recipe file %1.").arg(lRecipeFileName).toLatin1().constData());
        return NULL;
    }
    QString lRecipeString = file.readAll();
    file.close();

    // Create recipe IO from string
    PATRecipeIO *lRecipeIO=NULL;
    QTextStream lRecipeStream(&lRecipeString, QIODevice::ReadOnly);
    lRecipeIO = PATRecipeIO::CreateRecipeIo(lRecipeStream);
    if (lRecipeIO)
        lRecipeIO->SetRecipeName(lRecipeFileName);

    return lRecipeIO;
}

PATRecipeIO* PATRecipeIO::CreateRecipeIo(const RecipeFormat type)
{
    PATRecipeIO* lRecipeIO = NULL;

    switch (type)
    {
        case JSON:
            lRecipeIO = new PATRecipeIOJson();
            break;

        case CSV:
        default:
            lRecipeIO = NULL;
    }

    return lRecipeIO;
}


PATRecipeIO::PATRecipeIO(PATRecipeIO::RecipeFormat lFormat)
    : mFormat(lFormat)
{
}

PATRecipeIO::~PATRecipeIO()
{

}

const QString &PATRecipeIO::GetErrorMessage() const
{
    return mErrorMessage;
}

PATRecipeIO::RecipeFormat PATRecipeIO::GetRecipeFormat() const
{
    return mFormat;
}

void PATRecipeIO::SetRecipeName(const QString &lRecipeName)
{
    mRecipeName = lRecipeName;
}

void PATRecipeIO::SetReadOption(const QString &lOptionName)
{
    QString lName   = lOptionName.section("=", 0, 0);
    QString lValue  = lOptionName.section("=", 1, 1);
    QRegExp lRegExp("1|Y|TRUE|YES", Qt::CaseInsensitive);

    if (lValue.isEmpty() || lRegExp.exactMatch(lValue))
        mReadOptions.insert(lName);
    else
        mReadOptions.remove(lName);
}

bool PATRecipeIO::HasReadOption(const QString &lOptionName) const
{
    return mReadOptions.contains(lOptionName);
}

bool PATRecipeIO::Read(PATRecipe &lPATRecipe)
{
    QFile       lRecipeFile(mRecipeName);

    // Check recipe file exists
    if (lRecipeFile.exists() == false)
    {
        // No configuration file with Static PAT limits specified....
        if(mRecipeName.isEmpty() == true)
            mErrorMessage = "No recipe file defined.";
        else
            mErrorMessage = "Recipe file not found:" + mRecipeName;
        return false;
    }

    // Open PAT config file (Static PAT limits)
    if(lRecipeFile.open(QIODevice::ReadOnly) == false)
    {
        mErrorMessage = "Recipe file access denied:" + mRecipeName;
        return false;
    }

    QTextStream lRecipeStream(&lRecipeFile);

    return Read(lRecipeStream, lPATRecipe);
}

bool PATRecipeIO::Write(PATRecipe &/*lPATRecipe*/, const QString& /*recipeName*/)
{
    return false;
}

bool PATRecipeIO::Write(COptionsPat &/*lPatOptions*/, const QString& /*recipeName*/)
{
    return false;
}

bool PATRecipeIO::Read(const QByteArray &buffer, PATRecipe &lPATRecipe)
{
    QTextStream lRecipeStream(buffer, QIODevice::ReadOnly);

    return Read(lRecipeStream, lPATRecipe);
}

void PATRecipeIO::BuildPATBinsFailType(COptionsPat& lPATRecipe,
                                       const QHash<QString, CPatDefinition *> &lPATDefinitions,
                                       const QList<PATMultiVariateRule> &lPATMultivariate)
{
    // Outlier type...
    QHash<QString, CPatDefinition*>::const_iterator itPATDefinifion;
    CPatDefinition *                                lPatDef = NULL;

    for(itPATDefinifion = lPATDefinitions.constBegin();
        itPATDefinifion != lPATDefinitions.constEnd(); ++itPATDefinifion)
    {
        lPatDef = *itPATDefinifion;

        // Clean SPAT bins accordingly to SPAT status
        if (lPATRecipe.bStaticPAT == false && lPatDef->m_lFailStaticBin != -1)
            lPatDef->m_lFailStaticBin = -1;
        else if (lPatDef->m_lFailStaticBin != -1)
        {
            if (lPATRecipe.mPATHardBinFailType.contains(lPatDef->m_lFailStaticBin) == false)
                lPATRecipe.mPATHardBinFailType.insert(lPatDef->m_lFailStaticBin, GEX_TPAT_BINTYPE_STATICFAIL);

            if (lPATRecipe.mPATSoftBinFailType.contains(lPatDef->m_lFailStaticBin) == false)
                lPATRecipe.mPATSoftBinFailType.insert(lPatDef->m_lFailStaticBin, GEX_TPAT_BINTYPE_STATICFAIL);
        }

        // Clean DPAT bins accordingly to DPAT status
        if (lPATRecipe.bDynamicPAT == false && lPatDef->m_lFailDynamicBin != -1)
            lPatDef->m_lFailDynamicBin = -1;
        else if (lPatDef->m_lFailDynamicBin != -1)
        {
            if (lPATRecipe.mPATHardBinFailType.contains(lPatDef->m_lFailDynamicBin) == false)
                lPATRecipe.mPATHardBinFailType.insert(lPatDef->m_lFailDynamicBin, GEX_TPAT_BINTYPE_DYNAMICFAIL);

            if (lPATRecipe.mPATSoftBinFailType.contains(lPatDef->m_lFailDynamicBin) == false)
                lPATRecipe.mPATSoftBinFailType.insert(lPatDef->m_lFailDynamicBin, GEX_TPAT_BINTYPE_DYNAMICFAIL);
        }
    }

    // MV PAT bins
    if(lPATRecipe.GetMVPATEnabled())
    {
        for (int lIdx = 0; lIdx < lPATMultivariate.count(); ++lIdx)
        {
            if (lPATRecipe.mPATHardBinFailType.contains(lPATMultivariate.at(lIdx).GetBin()) == false)
                lPATRecipe.mPATHardBinFailType.insert(lPATMultivariate.at(lIdx).GetBin(), GEX_TPAT_BINTYPE_MVPAT);

            if (lPATRecipe.mPATSoftBinFailType.contains(lPATMultivariate.at(lIdx).GetBin()) == false)
                lPATRecipe.mPATSoftBinFailType.insert(lPATMultivariate.at(lIdx).GetBin(), GEX_TPAT_BINTYPE_MVPAT);
        }
    }

    if(lPATRecipe.mNNRIsEnabled)
    {
        QList<CNNR_Rule>::iterator itNNRBegin = lPATRecipe.GetNNRRules().begin();
        QList<CNNR_Rule>::iterator itNNREnd   = lPATRecipe.GetNNRRules().end();
        for (; itNNRBegin != itNNREnd; ++itNNRBegin)
        {
            if (lPATRecipe.mPATHardBinFailType.contains((*itNNRBegin).GetHardBin()) == false)
                lPATRecipe.mPATHardBinFailType.insert((*itNNRBegin).GetHardBin(), GEX_TPAT_BINTYPE_NNR);

            if (lPATRecipe.mPATSoftBinFailType.contains((*itNNRBegin).GetSoftBin()) == false)
                lPATRecipe.mPATSoftBinFailType.insert((*itNNRBegin).GetSoftBin(), GEX_TPAT_BINTYPE_NNR);
        }
    }

    if(lPATRecipe.mIsIDDQ_Delta_enabled)
    {
        QList<CIDDQ_Delta_Rule>::iterator itIDDQBegin = lPATRecipe.mIDDQ_Delta_Rules.begin();
        QList<CIDDQ_Delta_Rule>::iterator itIDDQEnd   = lPATRecipe.mIDDQ_Delta_Rules.end();
        for (; itIDDQBegin != itIDDQEnd; ++itIDDQBegin)
        {
            if (lPATRecipe.mPATHardBinFailType.contains((*itIDDQBegin).GetHardBin()) == false)
                lPATRecipe.mPATHardBinFailType.insert((*itIDDQBegin).GetHardBin(), GEX_TPAT_BINTYPE_IDDQ_DELTA);

            if (lPATRecipe.mPATSoftBinFailType.contains((*itIDDQBegin).GetSoftBin()) == false)
                lPATRecipe.mPATSoftBinFailType.insert((*itIDDQBegin).GetSoftBin(), GEX_TPAT_BINTYPE_IDDQ_DELTA);
        }
    }

    if(lPATRecipe.mIsGDBNEnabled)
    {
        QList<CGDBN_Rule>::iterator itGDBNBegin = lPATRecipe.mGDBNRules.begin();
        QList<CGDBN_Rule>::iterator itGDBNEnd   = lPATRecipe.mGDBNRules.end();
        for (; itGDBNBegin != itGDBNEnd; ++itGDBNBegin)
        {
            if (lPATRecipe.mPATHardBinFailType.contains((*itGDBNBegin).mHardBin) == false)
                lPATRecipe.mPATHardBinFailType.insert((*itGDBNBegin).mHardBin, GEX_TPAT_BINTYPE_BADNEIGHBORS);

            if (lPATRecipe.mPATSoftBinFailType.contains((*itGDBNBegin).mSoftBin) == false)
                lPATRecipe.mPATSoftBinFailType.insert((*itGDBNBegin).mSoftBin, GEX_TPAT_BINTYPE_BADNEIGHBORS);
        }
    }

    if(lPATRecipe.GetReticleEnabled())
    {
        QList<PATOptionReticle>::iterator itReticleBegin = lPATRecipe.GetReticleRules().begin();
        QList<PATOptionReticle>::iterator itReticleEnd   = lPATRecipe.GetReticleRules().end();
        for (; itReticleBegin != itReticleEnd; ++itReticleBegin)
        {
            if (lPATRecipe.mPATHardBinFailType.contains((*itReticleBegin).GetReticleHBin()) == false)
                lPATRecipe.mPATHardBinFailType.insert((*itReticleBegin).GetReticleHBin(), GEX_TPAT_BINTYPE_RETICLE);

            if (lPATRecipe.mPATSoftBinFailType.contains((*itReticleBegin).GetReticleSBin()) == false)
                lPATRecipe.mPATSoftBinFailType.insert((*itReticleBegin).GetReticleSBin(), GEX_TPAT_BINTYPE_RETICLE);
        }
    }

    if(lPATRecipe.mClusteringPotato)
    {
        QList<CClusterPotatoRule>::iterator itClusteringBegin = lPATRecipe.mClusterPotatoRules.begin();
        QList<CClusterPotatoRule>::iterator itClusteringEnd   = lPATRecipe.mClusterPotatoRules.end();
        for (; itClusteringBegin != itClusteringEnd; ++itClusteringBegin)
        {
            if (lPATRecipe.mPATHardBinFailType.contains((*itClusteringBegin).mHardBin) == false)
                lPATRecipe.mPATHardBinFailType.insert((*itClusteringBegin).mHardBin, GEX_TPAT_BINTYPE_BADCLUSTER);

            if (lPATRecipe.mPATSoftBinFailType.contains((*itClusteringBegin).mSoftBin) == false)
                lPATRecipe.mPATSoftBinFailType.insert((*itClusteringBegin).mSoftBin, GEX_TPAT_BINTYPE_BADCLUSTER);
        }
    }

    if(lPATRecipe.mExclusionZonesEnabled && lPATRecipe.lfCompositeExclusionZoneYieldThreshold > 0.0)
    {
        if (lPATRecipe.mPATHardBinFailType.contains(lPATRecipe.iCompositeZone_HBin) == false)
            lPATRecipe.mPATHardBinFailType.insert(lPATRecipe.iCompositeZone_HBin, GEX_TPAT_BINTYPE_ZPAT);

        if (lPATRecipe.mPATSoftBinFailType.contains(lPATRecipe.iCompositeZone_SBin) == false)
            lPATRecipe.mPATSoftBinFailType.insert(lPATRecipe.iCompositeZone_SBin, GEX_TPAT_BINTYPE_ZPAT);
    }
}

}   // namespace Gex
}   // namespace GS
