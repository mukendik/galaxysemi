#ifdef GCORE15334
#ifndef PAT_STDF_UPDATE_H
#define PAT_STDF_UPDATE_H

#include <QObject>

class CPatDefinition;

namespace GQTL_STDF
{
class Stdf_DTR_V4;
}

namespace GS
{
namespace StdLib
{
struct StdfRecordReadInfo;
}

namespace Gex
{

class PATProcessing;
class PATStdfUpdatePrivate;

class PATStdfUpdate : public QObject
{
public:

    PATStdfUpdate(QObject * parent = NULL);
    virtual ~PATStdfUpdate();

    const QString&  GetErrorMessage() const;

    void            SetPATSettings(const PATProcessing& lSettings);

    bool            Execute(const QString& lInputFileName, const QString& lOutputFileName);

protected:

    bool    CheckParameterValue(CPatDefinition * lPatDef, int lSite, const CTest& lTest,
                                double lResult, CPatSite *lPATSite, bool &lWrittenRecord);

    // Return total SoftBin records for a given bin# and site.
    long    GetTotalBinCount(bool isSoftBin, int lBin, int lSite);
    bool    ProcessBinRecords(bool &lWrittenRecord);
    bool    ProcessFile();
    bool    ProcessMIR(bool &lWrittenRecord);
    bool    ProcessMPR(bool& lWrittenRecord);
    bool    ProcessMRR(bool &lWrittenRecord);
    void    ProcessPCR(bool &lWrittenRecord);
    void    ProcessPIR();
    bool    ProcessPRR(bool &lWrittenRecord);
    bool    ProcessPTR(bool &lWrittenRecord);
    bool    ProcessRecord(GS::StdLib::StdfRecordReadInfo &lRecordHeader);

    void ReadDTR( const GQTL_STDF::Stdf_DTR_V4 &/*dtr*/ );
//    void ProcessReticleInformationsIn( const GQTL_STDF::Stdf_DTR_V4 &dtr );

    bool    ProcessTSR(bool &lWrittenRecord);
    bool    ProcessWIR(bool &lWrittenRecord);
    bool    ProcessWRR(bool &lWrittenRecord);
    bool    WriteATR();
    bool    WriteFAR();
    bool    WritePATBinRecords();
    bool    WriteBinRecords(int lBinType);
    bool    WriteOutlierRemovalReport();
    bool    WritePATHBR();
    bool    WritePATRecords();
    bool    WritePATSBR();
    bool    WritePATTestList();
    bool    WritePCR();
    bool    WriteTSR();
    bool    WriteWCR();
    bool    WriteExternalMapFailures();

private:

    Q_DISABLE_COPY(PATStdfUpdate)

    PATStdfUpdatePrivate *  mPrivate;

    ///
    /// \brief ProcessMPRInSplitMode process the MPR in the case of split mode
    /// \param aPatDef the pat definition
    /// \param aMPRRecord The input MPR record
    /// \param aOutlier is true if one of the pins has an outlier
    /// \param aTest the test corresponding to the MPR test and the pin number
    /// \param aPATSite The site used in the MPR
    /// \param asiteNumber the site number
    /// \return true if the MPR has to be not written in the output file
    ///
    bool ProcessMPRInSplitMode(CPatDefinition*& aPatDef,
                               GQTL_STDF::Stdf_MPR_V4 aMPRRecord,
                               bool& aOutlier,
                               CTest*& aTest,
                               CPatSite* aPATSite,
                               unsigned short aSiteNumber);

    ///
    /// \brief ProcessMPRInMergeMode process the MPR in the case of merge mode
    /// \param aPatDef the pat definition
    /// \param aMPRRecord The input MPR record
    /// \param aOutlier is true if one of the pins has an outlier
    /// \param aTest the test corresponding to the MPR test
    /// \param aPATSite The site used in the MPR
    /// \param aSiteNumber the site number
    /// \return true if the MPR has to be not written in the output file
    ///
    bool ProcessMPRInMergeMode(CPatDefinition*& aPatDef,
                               GQTL_STDF::Stdf_MPR_V4 aMPRRecord,
                               bool& aOutlier,
                               CTest*& aTest,
                               CPatSite* aPATSite,
                               unsigned short aSiteNumber);
};

}   // namespace Gex
}   // namespace GS

#endif // PAT_STDF_UPDATE_H
#endif
