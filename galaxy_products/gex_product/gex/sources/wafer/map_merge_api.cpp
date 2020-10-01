/******************************************************************************!
 * \file map_merge_api.cpp
 ******************************************************************************/
#include <QFileInfo>
#include <QDir>
#include <QScriptValue>
#include "map_merge_api.h"
#include "gex_scriptengine.h"
#include "wafermap.h"
#include "cbinning.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "gex_report.h"
#include "gex_file_in_group.h"
#include "map_alignment.h"
#include "merge_maps.h"
#include "wafer_export.h"
#include "gqtl_log.h"

extern CGexReport* gexReport;
extern GexScriptEngine* pGexScriptEngine;
extern void ConvertToScriptString(QString& strFile);

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \fn MapMergeApi
 ******************************************************************************/
MapMergeApi::MapMergeApi(QObject* parent) :
    QObject(parent),
    mOutputBin(NULL)
{
}

/******************************************************************************!
 * \fn ~MapMergeApi
 ******************************************************************************/
MapMergeApi::~MapMergeApi()
{
    this->Clear();
}

/******************************************************************************!
 * \fn AddMap
 ******************************************************************************/
const QString MapMergeApi::AddMap(const QString& filename,
                                  const QString& refLocation)
{
    QFileInfo lFileInfo(filename);

    if (! lFileInfo.isFile() ||
        ! lFileInfo.isReadable())
    {
        mErrorMessage =
            QString("%1 is not readable").arg(lFileInfo.absoluteFilePath());
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toUtf8().constData());
        return QString("error: %1").arg(mErrorMessage);
    }

    mMapList.push_back(lFileInfo.absoluteFilePath());
    mRefLoc.push_back(refLocation);
    return QString("ok");
}

/******************************************************************************!
 * \fn SetMergeBinRule
 ******************************************************************************/
const QString MapMergeApi::SetMergeBinRule(const QString& mergeBinHook)
{
    mMergeBinRule = mergeBinHook;
    return QString("ok");
}


/******************************************************************************!
 * \fn deleteCBinning
 ******************************************************************************/
static void deleteCBinning(CBinning** ptr)
{
    if (*ptr != NULL)
    {
        ::CBinning* lTmpBin;
        while (*ptr != NULL)
        {
            lTmpBin = (*ptr)->ptNextBin;
            delete *ptr;
            *ptr = lTmpBin;
        }
        *ptr = NULL;
    }
}

/******************************************************************************!
 * \fn Clear
 ******************************************************************************/
const QString MapMergeApi::Clear()
{
    while (! mMapList.empty())
    {
        mMapList.pop_front();
    }
    while (! mRefLoc.empty())
    {
        mRefLoc.pop_front();
    }

    mMergeBinRuleFile = "";
    mMergeBinRule = "";

    deleteCBinning(&mOutputBin);

    mErrorMessage = "";

    return QString("ok");
}

/******************************************************************************!
 * \fn deleteWaferAndBinLists
 ******************************************************************************/
static void deleteWaferAndBinLists(QList<CWaferMap*>& lWaferMaps,
                                   QList<CBinning*>& lBinsMaps)
{
    ::CBinning* lBin;
    while (! lWaferMaps.empty())
    {
        delete lWaferMaps.takeFirst();
    }
    while (! lBinsMaps.empty())
    {
        lBin = lBinsMaps.takeFirst();
        deleteCBinning(&lBin);
    }
}

/******************************************************************************!
 * \fn Merge
 ******************************************************************************/
const QString MapMergeApi::Merge(bool autoAlignment,
                                 bool hardBin)
{
    QList<CWaferMap*> lWaferMaps;
    QList<CBinning*> lBinsMaps;
    QList<QString>::const_iterator lMapIter;
    QList<QString>::const_iterator lLocIter;
    MergeMaps lMergeMaps;

    // Load maps
    CWaferMap lMapRef;
    CBinning* lBinRef;
    GS::Gex::MapAlignment lAlignment;
    int lPass = 0;
    for (lMapIter = mMapList.begin(),
             lLocIter = mRefLoc.begin();
         lMapIter != mMapList.end(); ++lMapIter, ++lLocIter)
    {
        CWaferMap lMapCur;
        CBinning* lBinCur;
        if (! this->LoadMap(*lMapIter, lMapCur, &lBinCur, hardBin))
        {
            mErrorMessage = QString("Failed to load map %1 : ").
                arg(*lMapIter).
                arg(mErrorMessage);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toUtf8().constData());
            deleteWaferAndBinLists(lWaferMaps, lBinsMaps);
            return QString("error: %1").arg(mErrorMessage);
        }
        lMapCur.ComputeReferenceDieLocation(*lLocIter);
        if (lPass == 0)
        {
            lMapRef = lMapCur;
            lBinRef = lBinCur;
        }
        else if (autoAlignment)
        {
            WaferTransform lTransform;
            if (! lAlignment.PerformMapAlignment(lMapRef,
                                                 lMapCur,
                                                 lTransform))
            {
                mErrorMessage = QString("Failed to align map %1 : ").
                    arg(*lMapIter).
                    arg(lAlignment.GetErrorMessage());
                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toUtf8().constData());
                deleteWaferAndBinLists(lWaferMaps, lBinsMaps);
                return QString("error: %1").arg(mErrorMessage);
            }
        }
        lWaferMaps.append(new CWaferMap(lMapCur));
        lBinsMaps.append(lBinCur);
        ++lPass;
    }

    // Output
    deleteCBinning(&mOutputBin);
    mOutputBin = lBinRef->Clone();

    // Merge
    if (lMergeMaps.MergeWaferMaps(lWaferMaps,
                                  lBinsMaps,
                                  mMergeBinRule,
                                  mErrorMessage,
                                  mOutputMap,
                                  mOutputBin) == false)
    {
        mErrorMessage = QString("Failed to merge maps : %1").arg(mErrorMessage);
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toUtf8().constData());
        deleteWaferAndBinLists(lWaferMaps, lBinsMaps);
        return QString("error: %1").arg(mErrorMessage);
    }

    deleteWaferAndBinLists(lWaferMaps, lBinsMaps);
    return QString("ok");
}

/******************************************************************************!
 * \fn LoadMap
 ******************************************************************************/
bool MapMergeApi::LoadMap(const QString& filename,
                          CWaferMap& map,
                          CBinning** bins,
                          bool hardBin)
{
    FILE* hFile = NULL;
    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup* pFile = NULL;
    QString lMapFile;
    QString lScriptFile;

    // Create script
    lMapFile = filename;
    ConvertToScriptString(lMapFile);
    lScriptFile = Engine::GetInstance().GetAssistantScript();
    hFile = fopen(lScriptFile.toLatin1().constData(), "w");
    if (hFile == NULL)
    {
        mErrorMessage =
            QString("Failed to create script file : %1").arg(lScriptFile);
        return false;
    }

    // SetOptions section
    if (! ReportOptions.WriteOptionSectionToFile(hFile))
    {
        mErrorMessage = QString("Can't write option section");
        return false;
    }

    // SetProcessData section
    fprintf(hFile, "SetProcessData()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  var group_id;\n");
    fprintf(hFile, "  gexGroup('reset','all');\n");
    // Write all groups (one per site)
    // Make sure any '\' in string is doubled
    fprintf(hFile, "  group_id = gexGroup('insert','DataSet_1');\n");
    fprintf(hFile, "  gexFile(group_id,'insert','%s','all','all',' ','');\n\n",
            lMapFile.toLatin1().constData());
    fprintf(hFile, "}\n\n");

    // Main section
    fprintf(hFile, "main()\n");
    fprintf(hFile, "{\n");
    fprintf(hFile, "  SetOptions();\n");
    // Use wafer map as binning source information
    fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");
    // Disable STDF.FTR processing (for PAT speed improvement since FTR are not used in PAT algorihtms)
    fprintf(hFile,"  gexOptions('dataprocessing', 'functional_tests', 'disabled');\n");
    // Force Part identification to use xy coordinates
    fprintf(hFile,"  gexOptions('dataprocessing', 'part_identification', 'xy');\n");
    // Ensure FULL wafermap is created in any case,
    // non matter the filtering of parts
    fprintf(hFile, "  gexOptions('wafer','visual_options','all_parts');\n");
    // Use 'auto' mode for positive X direction
    fprintf(hFile, "  gexOptions('wafer','positive_x','auto');\n");
    // Use 'auto' mode for positive Y direction
    fprintf(hFile, "  gexOptions('wafer','positive_y','auto');\n");
    // Use 'From data file if specified, else detect' mode for
    // flat/notch orientation
    fprintf(hFile,
            "  gexOptions('wafer','notch_location','file_or_detected');\n");
    fprintf(hFile, "  SetProcessData();\n");
    // Only data analysis, no report created!
    fprintf(hFile, "  gexOptions('report','build','false');\n");
    // Do NOT show report tab
    fprintf(hFile, "  gexBuildReport('hide','0');\n");
    fprintf(hFile, "}\n\n");
    fclose(hFile);

    // Execute script
    if (CSLEngine::GetInstance().RunScript(lScriptFile).IsFailed())
    {
        mErrorMessage =
            QString("Failed to convert external map %1").arg(lMapFile);
        return false;
    }

    // Get wafermap and bins
    pGroup = (gexReport->getGroupsList().isEmpty()) ? NULL :
        gexReport->getGroupsList().first();
    if (pGroup == NULL)
    {
        mErrorMessage =
            QString("Failed to get pGroup for map %1").arg(lMapFile);
        return false;
    }
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL :
        pGroup->pFilesList.first();
    if (pFile == NULL)
    {
        mErrorMessage =
            QString("Failed to get pFile for map %1").arg(lMapFile);
        return false;
    }

    // Load Wafermap array with adequat bins
    if (hardBin)
    {
        gexReport->FillWaferMap(pGroup, pFile, NULL, GEX_WAFMAP_HARDBIN, true);
        *bins = pFile->m_ParentGroup->cMergedData.ptMergedHardBinList->Clone();
    }
    else
    {
        gexReport->FillWaferMap(pGroup, pFile, NULL, GEX_WAFMAP_SOFTBIN, true);
        *bins = pFile->m_ParentGroup->cMergedData.ptMergedSoftBinList->Clone();
    }

    map = pFile->getWaferMapData();

    return true;
}

/******************************************************************************!
 * \fn Export
 ******************************************************************************/
const QString MapMergeApi::Export(const QString& mapFormat,
                                  const QString& mapFileName)
{
    WaferExport lWaferExport;
    int lFormat;
    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup* pFile = NULL;

    if (mOutputBin == NULL)
    {
        mErrorMessage = QString("Merge must be done before export");
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toUtf8().constData());
        return QString("error: %1").arg(mErrorMessage);
    }

    // setWaferMapData
    pGroup = (gexReport->getGroupsList().isEmpty()) ? NULL :
        gexReport->getGroupsList().first();
    if (pGroup == NULL)
    {
        mErrorMessage = QString("Failed to get pGroup");
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return QString("error: %1").arg(mErrorMessage);
    }
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL :
        pGroup->pFilesList.first();
    if (pFile == NULL)
    {
        mErrorMessage = QString("Failed to get pFile");
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return QString("error: %1").arg(mErrorMessage);
    }

    // Update wafer map for export
    pFile->getWaferMapData() = mOutputMap;

    // Update bins list for export
    deleteCBinning(&pGroup->cMergedData.ptMergedSoftBinList);
    deleteCBinning(&pGroup->cMergedData.ptMergedHardBinList);

    pGroup->cMergedData.ptMergedSoftBinList = mOutputBin->Clone();
    pGroup->cMergedData.ptMergedHardBinList = mOutputBin->Clone();

    // Format
    if (! lWaferExport.IsSupportedOutputFormat(mapFormat, lFormat))
    {
        mErrorMessage = QString("%1 output map format not supported");
        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toUtf8().constData());
        return QString("error: %1").arg(mErrorMessage);
    }

    // Export
    if (! lWaferExport.
        CreateOutputMap((WaferExport::Output) lFormat, mapFileName))
    {
        mErrorMessage = QString("Failed to create output map %1: %2").
            arg(mapFileName).
            arg(lWaferExport.GetErrorMessage());
        GSLOG(SYSLOG_SEV_WARNING, mErrorMessage.toLatin1().constData());
        return QString("error: %1").arg(mErrorMessage);
    }

    return QString("ok");
}

/******************************************************************************!
 * \fn GetErrorMessage
 ******************************************************************************/
const QString& MapMergeApi::GetErrorMessage() const
{
    return mErrorMessage;
}

}
}
