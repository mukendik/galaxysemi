#ifdef GCORE15334
#ifndef PAT_PROCESS_H
#define PAT_PROCESS_H


#include <QString>

class CWaferMap;
class CPatInfo;

namespace GS
{
namespace Gex
{

class PATProcessing;
class PATProcessWSPrivate;

namespace PAT
{
class ExternalMapDetails;
}

class PATProcessWS
{
public:

    PATProcessWS(CPatInfo*);
    PATProcessWS(const PATProcessing &lSettings);
    ~PATProcessWS();

    bool            Execute(PATProcessing &lSettings, const QString& lDataFile = QString());
    const QString&  GetErrorMessage() const;
    const PAT::ExternalMapDetails&  GetExternalMapDetails() const;

    static bool     CreateSTDFDataFile(const PATProcessing &lSettings, QString &lSTDFDataFile,
                                       QString &lErrorMessage);

protected:

    bool    ApplyInclusionDiesList();
    void    CheckAxisDirection();
    bool    CheckMapMerge();
    bool    CheckReferenceDieLocation();
    bool    CheckWafermapAlignment();
    void    CheckWafermapOrientation();
    bool    ConvertToSTDF(const QString& lFileName, QString &lSTDFFileName);
    bool    CreateSTDFDataFile(QString &lSTDFDataFile, QString &lExternalMap);
    bool    ExcludeIrrelevantDies();
    bool    FillInclusionDies();
    bool    FilterExternalMap(const QString &lMapFile, const QString &lFilteredMap, int lCPUType,
                              int lBin, QString lBinName) const;
    bool    GenerateOutputMaps();
    bool    ImportCompositeWaferLotRules();
    bool    LoadExternalMap(const QString& lFileName);
    bool    LoadRecipeFile();
    bool    LoadSTDF(const QString& lFileName);
    bool    MergeInputSTDFFiles(const QStringList& lInputFiles, QString &lMergedInput);
    //    bool    OverloadRecipeFile();
    bool    PrepareData();
    bool    PreProcessing();
    bool    Processing();
    bool    PostProcessing();
    void    RemoveDiesInMap(CWaferMap& map, const QString& bins);
    bool    RemoveStdfRunCompositePAT(int lCoordX, int lCoordY);
    bool    ReadWafermapConfiguration(const QString& lSTDFFile, CWaferMap& lWafer, bool &lWCRFound);
    bool    StackWafermaps();

    //-- Map Updater
    bool    UpdateExternalMapFile           (const QString& inputExternalMap, QString &outputExternalMap);
    bool    UpdateE142ExternalMap           (const QString& inputExternalMap, const QString& outputExternalMap);
    bool    UpdateKLAINFExternalMap         (const QString& inputExternalMap, const QString& outputExternalMap);
    bool    UpdateTELP8ExternalMap          (const QString& inputExternalMap, const QString& outputExternalMap);
    bool    UpdateSkyNPExternalMap          (const QString& inputExternalMap, const QString& outputExternalMap);
    bool    UpdateWoburnSECSIIExternalMap   (const QString& inputExternalMap, const QString& outputExternalMap);

    template <typename ParserDef>
    bool    UpdateExternalMap               ( const QString &inputExternalMap, const QString &outputExternalMap);

private:

    PATProcessWSPrivate * mPrivate;

    friend class MapMergeApi;
};

}   // namespace Gex
}   // namespace GS

#endif // PAT_PROCESS_H
#endif
