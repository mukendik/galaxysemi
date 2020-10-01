#ifndef MERGEMAPS_H
#define MERGEMAPS_H


#include <QString>

class CBinning;
class CWaferMap;

namespace GS
{
namespace  Gex
{

class MergeMaps
{
public:
    MergeMaps();

    /*!
      @brief    Merge input wafer maps and binning maps.

      @param    maps            list of maps to be merged
      @param    Bins            bins maps to be merged
      @param    mergeBinJSHook  Name of the Hook to a JS function used to override default behavior
      @param    errorMessage    Error message filled by the method
      @param    outputMap       the output merged map
      @param    outputBinning   the output merged binning map
      @param    missingBin      Bin number to a attribute to a pass die from the external map which is
                                missing in the STDF map. Default value is -1

      @return   True if the merge succeed, otherwise false
      */
    bool MergeWaferMaps(const QList<CWaferMap *> &maps,
                        const QList<CBinning *> &Bins,
                        const QString &mergeBinJSHook,
                        QString &errorMessage,
                        CWaferMap &outputMap,
                        CBinning *outputBinning,
                        int missingBin = -1);

private:
    /*!
      @brief    Set the binning using the C++ algorithm.

      @param    maps            list of maps to be merged
      @param    Bins            bins maps to be merged
      @param    externalMaps    external maps
      @param    missingBin      Bin number to a attribute to a pass die from the external map which is
                                missing in the STDF map.
      @param    errorMessage    Error message filled by the method
      @param    outputMap       the output merged map
      @param    outputBinning   the output merged binning map

      @return   True if the merge succeed, otherwise false
      */
    void SetBinning(bool testedExternalMap,
                    int binNumberRef,
                    CWaferMap &outputMap,
                    CBinning *outputBinning,
                    int idxRef,
                    bool passExternalMap,
                    int missingBin,
                    const QList<CBinning *> &CBinningList,
                    const QList<int> &binNumberList,
                    CBinning *refBinInfo);


    bool RetrieveRefBin(const CWaferMap &outputMap,
                        const CWaferMap *referenceMap,
                        const QList<CBinning *> & binsMaps,
                        int idxRef,
                        int &dieX,
                        int &dieY,
                        int &binNumberRef,
                        CBinning **refBinInfo,
                        QString &errorMessage);


    bool RetrieveMapBin(const QList<CWaferMap *> &waferMaps,
                        const QList<CBinning *> &binsMaps,
                        int &lDieX,
                        int &dieY,
                        QList<int> &binNumberList,
                        QList<CBinning *> &CBinningList,
                        bool& testedExternalMap,
                        bool& passExternalMap,
                        QString &errorMessage);
};

}
}
#endif // MERGEMAPS_H
