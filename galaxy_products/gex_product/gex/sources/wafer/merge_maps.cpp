#include <QScriptValue>
#include <QRect>
#include "merge_maps.h"
#include "wafermap.h"
#include "cbinning.h"
#include "bin_description.h"
#include "gqtl_log.h"
#include "gex_scriptengine.h"

extern GexScriptEngine*	pGexScriptEngine ;
namespace GS
{
namespace  Gex
{

MergeMaps::MergeMaps()
{
}

bool MergeMaps::MergeWaferMaps(const QList<CWaferMap *> & waferMaps,
                               const QList<CBinning *> & binsMaps,
                               const QString &mergeBinJSHook,
                               QString &errorMessage,
                               CWaferMap& outputMap,
                               CBinning* outputBinning,
                               int missingBin /* = -1 */)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Merge maps with %1 bins using missing bin no %2")
          .arg(binsMaps[0]?binsMaps[0]->CountBins():0)
          .arg(missingBin)
          .toLatin1().data());

    QScriptValue        lMergeBinJSFunction;
    QScriptValue        lJSBinArray;
    QScriptValueList    lJSFunctionArguments;

    QList<GS::Gex::BinDescription> lBinDescriptions;
    GS::Gex::BinDescription *       lMergedBin  = NULL;

    // If JS hook function defined for bin merge, check if it is available in the script engine
    if (mergeBinJSHook.isEmpty() == false)
    {
        if (pGexScriptEngine)
        {
            lMergeBinJSFunction = pGexScriptEngine->globalObject().property(mergeBinJSHook);

            // Function not found, stop here
            if (lMergeBinJSFunction.isValid() == false)
            {
                errorMessage = QString("Failed to retrieve Merge Bin JS Hook function %1").arg(mergeBinJSHook);
                GSLOG(SYSLOG_SEV_ERROR, errorMessage.toLatin1().constData());
                return false;
            }

            // Script value found is not a function, stop here
            if (lMergeBinJSFunction.isFunction() == false)
            {
                errorMessage = QString("%1 is not a JS function").arg(mergeBinJSHook);
                GSLOG(SYSLOG_SEV_ERROR, errorMessage.toLatin1().constData());
                return false;
            }

            lJSBinArray = pGexScriptEngine->newArray(binsMaps.size());
            for (int i = 0; i < binsMaps.size(); ++i)
            {
                lBinDescriptions.append(GS::Gex::BinDescription());
                lJSBinArray.setProperty(i, pGexScriptEngine->newQObject((QObject*)&lBinDescriptions[i]));
            }

            lJSFunctionArguments.append(lJSBinArray);
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, "GS Script Engine not available");
            errorMessage = "GS script engine not available";
            return false;
        }
    }

    // The wafer maps list has to has at least 2 maps
    if (waferMaps.size() < 2)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Unable to merge maps, need at least 2 maps.");
        errorMessage = "*PAT Error* Need at least 2 maps to do merge.";
        return false;
    }

    // The wafer maps list and the corresponding binning list has to has the same number of elements
    if (waferMaps.size() != binsMaps.size())
    {
        GSLOG(SYSLOG_SEV_ERROR, "The binning collector and the maps collector do not have the same number of elements");
        errorMessage = "*PAT Error* The binning collector and the maps collector do not have the same number of elements.";
        return false;
    }

    int lListMapSize = waferMaps.size();

    // Check all bin list is correctly instantiated and check geometries
    for (int i=0; i<lListMapSize; ++i)
    {
        if (binsMaps[i] == NULL || waferMaps[i] == NULL)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Unable to merge maps, Bins list or wafermap is empty. ");
            errorMessage = "*PAT Error* Missing binning or map list for merge";
            return false;
        }
    }

    // to be deleted
    CWaferMap* lReferenceMap = waferMaps[0];
    outputMap = *lReferenceMap;

    // Make the stdfMap containing the max of the 2 input maps
    QRect lGeometryRef(lReferenceMap->iLowDieX, lReferenceMap->iLowDieY, lReferenceMap->SizeX, lReferenceMap->SizeY);

    // The first wafer map is already in the reference map
    for (int i=1; i<lListMapSize; ++i)
    {
        QRect lGeometryMap(waferMaps[i]->iLowDieX, waferMaps[i]->iLowDieY, waferMaps[i]->SizeX, waferMaps[i]->SizeY);
        if (lGeometryMap.contains(lGeometryRef))
        {
            outputMap.iLowDieX            = waferMaps[i]->iLowDieX;
            outputMap.iLowDieY            = waferMaps[i]->iLowDieY;
            outputMap.iHighDieX           = waferMaps[i]->iHighDieX;
            outputMap.iHighDieY           = waferMaps[i]->iHighDieY;
        }
        else if (lGeometryRef.contains(lGeometryMap))
        {
            outputMap.iLowDieX            = lReferenceMap->iLowDieX;
            outputMap.iLowDieY            = lReferenceMap->iLowDieY;
            outputMap.iHighDieX           = lReferenceMap->iHighDieX;
            outputMap.iHighDieY           = lReferenceMap->iHighDieY;
        }
        else
        {
            outputMap.iLowDieX            = qMin(lReferenceMap->iLowDieX, waferMaps[i]->iLowDieX);
            outputMap.iLowDieY            = qMin(lReferenceMap->iLowDieY, waferMaps[i]->iLowDieY);
            outputMap.iHighDieX           = qMax(lReferenceMap->iHighDieX, waferMaps[i]->iHighDieX);
            outputMap.iHighDieY           = qMax(lReferenceMap->iHighDieY, waferMaps[i]->iHighDieY);
        }

        // Check bin category conflicts (Bin 5 pass in first map and bin 5 fail in the second one)
        CBinning*  lBinInfo     = binsMaps[i];
        CBinning*  lRefBinInfo  = NULL;
        while(lBinInfo)
        {
            lRefBinInfo = outputBinning->Find(lBinInfo->iBinValue);
            // all the bin number in the corresponding binMap "i" must be
            // present in the outpuBinning and must have the same status "cPassFaill"
            if (lRefBinInfo && lRefBinInfo->cPassFail != lBinInfo->cPassFail)
            {
                QString rootCause = QString("Bin category conflict on bin %1").arg(lBinInfo->iBinValue);
                GSLOG(SYSLOG_SEV_WARNING, rootCause.toLatin1().constData());
                GSLOG(SYSLOG_SEV_WARNING, QString("Bin %1 status in STDF is '%2' but '%3' in external")
                      .arg(lBinInfo->iBinValue)
                      .arg((char)lRefBinInfo->cPassFail).arg((char)lBinInfo->cPassFail)
                      .toLatin1().data()
                      );
                errorMessage = "*PAT Error* Merge map failed: " + rootCause;
                return false;
            }
            lBinInfo = lBinInfo->ptNextBin;
        };
    }

    outputMap.SizeX               = outputMap.iHighDieX - outputMap.iLowDieX + 1;
    outputMap.SizeY               = outputMap.iHighDieY - outputMap.iLowDieY + 1;
    outputMap.iTotalPhysicalDies  = 0;

    outputMap.SetReferenceDieLocation(lReferenceMap->GetReferenceDieLocation());

    // detroy the wafmap coming from the reference one
    if (outputMap.getWafMap())
        delete [] outputMap.getWafMap();

    // allocate the new one
    outputMap.setWaferMap(CWafMapArray::allocate(outputMap.SizeX*outputMap.SizeY));
    outputMap.allocCellTestCounter(outputMap.SizeX*outputMap.SizeY);


    int         lDieX               = 0;
    int         lDieY               = 0;
    int         lBinNumberRef       = GEX_WAFMAP_EMPTY_CELL;
    bool        lTestedExternalMap  = false;
    bool        lPassExternalMap    = true;
    CBinning    *lRefBinInfo        = NULL;
    QList<int>  lBinNumberList;
    QList<CBinning*> lCBinningList;

    // Loop on all dies in the wafer (the union of the 2 wafers) and tag it if needed
    int lWaferMapSize = outputMap.SizeX * outputMap.SizeY;
    for (int lIdxRef=0; lIdxRef < lWaferMapSize; ++lIdxRef)
    {
        lTestedExternalMap  = false;
        lPassExternalMap    = true;
        lBinNumberList.clear();
        lCBinningList.clear();

        if (!RetrieveRefBin(outputMap, lReferenceMap, binsMaps, lIdxRef, lDieX, lDieY, lBinNumberRef, &lRefBinInfo, errorMessage))
        {
            return false;
        }

        // Loop on all other maps
        // Get the coordinate corresponding to the index
        if (!RetrieveMapBin(waferMaps, binsMaps, lDieX, lDieY, lBinNumberList,
                            lCBinningList, lTestedExternalMap, lPassExternalMap, errorMessage))
        {
            return false;
        }

        // Use JS hook to determine which binning to assign to this die
        if (lMergeBinJSFunction.isValid())
        {
            // Reset binnings
            lBinDescriptions[0].SetNumber(lBinNumberRef);
            if (lRefBinInfo)
            {
                lBinDescriptions[0].SetCategory(lRefBinInfo->cPassFail);
                lBinDescriptions[0].SetName(lRefBinInfo->strBinName);
            }

            for (int i=1; i < lListMapSize; ++i)
            {
                // Reset binnings
                lBinDescriptions[i].SetNumber(lBinNumberList[i-1]);
                if (lCBinningList[i-1])
                {
                    lBinDescriptions[i].SetCategory(lCBinningList[i-1]->cPassFail);
                    lBinDescriptions[i].SetName(lCBinningList[i-1]->strBinName);
                }
            }

            //Call JS hook here
            QScriptValue lResult = lMergeBinJSFunction.call(QScriptValue(), lJSFunctionArguments);

            if (lResult.isValid() == false)
            {
                QString rootCause = QString("Failed to call JS function %1").arg(mergeBinJSHook);
                GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());
                errorMessage  = "*PAT Error* Merge map failed: "+ rootCause;
                return false;
            }

            if (pGexScriptEngine->hasUncaughtException())
            {
                QString rootCause = QString("Exception caught by Script Engine at line %1: %2")
                        .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
                        .arg(pGexScriptEngine->uncaughtException().toString());
                GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());
                errorMessage  = "*PAT Error* Merge map failed: "+ rootCause;
                return false;
            }

            lMergedBin = qobject_cast<GS::Gex::BinDescription *>(lResult.toQObject());

            if (lMergedBin == NULL)
            {
                QString rootCause = QString("Value returned by %1 JS function is not a GSBinInfo type").arg(mergeBinJSHook);
                GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());
                errorMessage  = "*PAT Error* Merge map failed: "+ rootCause;
                return false;
            }

            // Bin number >= 0 means a tested die
            if (lMergedBin->GetNumber() >= 0)
                ++outputMap.iTotalPhysicalDies;

            // Assign bin to the die on the wafermap
            outputMap.getWafMap()[lIdxRef].setBin(lMergedBin->GetNumber());

            // Update bin list count
            if (lMergedBin->GetNumber() != lBinNumberRef)
            {
                // Decrease current bin count
                if (lBinNumberRef != GEX_WAFMAP_EMPTY_CELL)
                {
                    lRefBinInfo = outputBinning->Find(lBinNumberRef);
                }
                if (lRefBinInfo)
                    --lRefBinInfo->ldTotalCount;

                if (lMergedBin->GetNumber() >= 0)
                {
                    // Update STDF bin list
                    lRefBinInfo = outputBinning->Find(lMergedBin->GetNumber());

                    if (lRefBinInfo)
                        ++lRefBinInfo->ldTotalCount;
                    else
                    {
                        QChar lPassFail;

                        if (lMergedBin->GetCategory().isEmpty() == false)
                            lPassFail = lMergedBin->GetCategory().at(0);

                        outputBinning->Append(lMergedBin->GetNumber(),
                                               lPassFail.toLatin1(),
                                               1,
                                               lMergedBin->GetName());
                    }
                }
            }
        }
        else
        {
            // Use default C++ algortihm
            SetBinning(lTestedExternalMap,
                       lBinNumberRef,
                       outputMap,
                       outputBinning,
                       lIdxRef,
                       lPassExternalMap,
                       missingBin,
                       lCBinningList,
                       lBinNumberList,
                       lRefBinInfo);
         }
    }
    return true;
}


void MergeMaps::SetBinning(bool testedExternalMap,
                           int binNumberRef,
                           CWaferMap& outputMap,
                           CBinning* outputBinning,
                           int idxRef,
                           bool passExternalMap,
                           int missingBin,
                           const QList<CBinning*>& CBinningList,
                           const QList<int>& binNumberList,
                           CBinning* refBinInfo)
{
    // map externe: not tested
    if (testedExternalMap == false)
    {
        // If ref die is tested, use ref bin
        // if ref die is untested, keep die as untested
        if (binNumberRef != GEX_WAFMAP_EMPTY_CELL)
        {
            outputMap.getWafMap()[idxRef].setBin(binNumberRef);
            ++outputMap.iTotalPhysicalDies;
        }
    }
    // map externe and pass
    else if (passExternalMap == true)
    {
        // If ref die is tested, use ref bin
        // if ref die is untested, use missing bin if defined
        // else keep external map bin
        if (binNumberRef != GEX_WAFMAP_EMPTY_CELL)
        {
            outputMap.getWafMap()[idxRef].setBin(binNumberRef);
        }
        else if (missingBin != -1)
        {
            outputMap.getWafMap()[idxRef].setBin(missingBin);

            // Update STDF bin list
            refBinInfo = outputBinning->Find(missingBin);

            if (refBinInfo)
                ++refBinInfo->ldTotalCount;
            else
                outputBinning->Insert(missingBin, 'F', 1, "STDF Missing");
        }
        else
        {
            for (int i=0; i <CBinningList.size(); ++i)
            {
                if (CBinningList[i])
                {
                    outputMap.getWafMap()[idxRef].setBin(binNumberList[i]);

                    // Update STDF bin list
                    refBinInfo = outputBinning->Find(binNumberList[i]);

                    if (refBinInfo)
                        ++refBinInfo->ldTotalCount;
                    else
                        outputBinning->Insert(CBinningList[i]->iBinValue,
                                               CBinningList[i]->cPassFail,
                                               1,
                                               CBinningList[i]->strBinName);
                    break;
                }
            }

        }

        ++outputMap.iTotalPhysicalDies;
    }
    // fail in external maps, take the first fail
    else
    {
        for (int i=0; i <CBinningList.size(); ++i)
        {
            if (CBinningList[i])
            {
                outputMap.getWafMap()[idxRef].setBin(binNumberList[i]);

                if (binNumberRef != GEX_WAFMAP_EMPTY_CELL)
                {
                    refBinInfo = outputBinning->Find(binNumberRef);
                }
                // Decrease current bin count
                if (refBinInfo)
                    --refBinInfo->ldTotalCount;

                // Update STDF bin list
                refBinInfo = outputBinning->Find(binNumberList[i]);

                if (refBinInfo)
                    ++refBinInfo->ldTotalCount;
                else
                    outputBinning->Insert(CBinningList[i]->iBinValue,
                                           CBinningList[i]->cPassFail,
                                           1,
                                           CBinningList[i]->strBinName);
                break;
            }
        }

        ++outputMap.iTotalPhysicalDies;
    }
}

bool MergeMaps::RetrieveRefBin(const CWaferMap& outputMap,
                               const CWaferMap* referenceMap,
                               const QList<CBinning *> & binsMaps,
                               int idxRef,
                               int& dieX,
                               int& dieY,
                               int& binNumberRef,
                               CBinning** refBinInfo,
                               QString& errorMessage)
{
    int lIdxReferenceMap    = 0;
    // Retrieve the array index from the coordinates in the merged map
    if (outputMap.coordFromIndex(idxRef, dieX, dieY) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("No valid coordinates for index %1 in the merger map").arg(idxRef).toLatin1().constData());
        errorMessage  = "*PAT Error* Merge map failed: internal error";
        return false;
    }

    // Get the reference map index correpsonding to the coordinates
    if (referenceMap->indexFromCoord(lIdxReferenceMap, dieX, dieY))
    {
        binNumberRef    = referenceMap->getWafMap()[lIdxReferenceMap].getBin();
        *refBinInfo      = binsMaps[0]->Find(binNumberRef);

        if (binNumberRef != GEX_WAFMAP_EMPTY_CELL && *refBinInfo == NULL)
        {
            QString rootCause = QString("Bin %1 from STDF map not found in the STDF bins list").arg(binNumberRef);
            GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());

            errorMessage  = "*PAT Error* Merge map failed: "+ rootCause;

            return false;
        }
        if (binNumberRef == GEX_WAFMAP_EMPTY_CELL || *refBinInfo == NULL)
        {
            binNumberRef    = GEX_WAFMAP_EMPTY_CELL;
            *refBinInfo      = NULL;
        }
    }
    else
    {
        binNumberRef    = GEX_WAFMAP_EMPTY_CELL;
        *refBinInfo      = NULL;
    }
    return true;
}

bool MergeMaps::RetrieveMapBin(const QList<CWaferMap *>& waferMaps,
                               const QList<CBinning *> & binsMaps,
                               int& dieX,
                               int& dieY,
                               QList<int>& binNumberList,
                               QList<CBinning*>& CBinningList,
                               bool& testedExternalMap,
                               bool& passExternalMap,
                               QString& errorMessage)
{
    int         lBinFromMap         = GEX_WAFMAP_EMPTY_CELL;
    int         lIdxProber          = 0;
    CBinning    *lExternalBinInfo   = NULL;
    for (int i=1; i<waferMaps.size(); ++i)
    {
        if (waferMaps[i]->indexFromCoord(lIdxProber, dieX, dieY))
        {
            lBinFromMap         = waferMaps[i]->getWafMap()[lIdxProber].getBin();
            lExternalBinInfo    = binsMaps[i]->Find(lBinFromMap);

            // fails means that the bin found in the external map does not exist in the list of binning of the external map
            if (lBinFromMap != GEX_WAFMAP_EMPTY_CELL && lExternalBinInfo == NULL)
            {
                QString rootCause = QString("Bin %1 from external map not found in the bins list").arg(lBinFromMap);
                GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());
                errorMessage  = "*PAT Error* Merge map failed: "+ rootCause;
                return false;
            }
            if (lBinFromMap == GEX_WAFMAP_EMPTY_CELL || lExternalBinInfo == NULL)
            {
                lBinFromMap         = GEX_WAFMAP_EMPTY_CELL;
                lExternalBinInfo    = NULL;
            }
            else
            {
                testedExternalMap = true;
                if (lExternalBinInfo->cPassFail == 'F')
                {
                    passExternalMap = false;
                }
            }
        }
        else
        {
            lBinFromMap         = GEX_WAFMAP_EMPTY_CELL;
            lExternalBinInfo    = NULL;
        }
        binNumberList.append(lBinFromMap);
        CBinningList.append(lExternalBinInfo);
    }
    return true;
}

}
}
