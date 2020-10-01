#ifndef WAFER_EXPORT_H
#define WAFER_EXPORT_H

#include "pat_sinf_info.h"
#include "gex_pat_processing.h"
#include "pat_definition.h"
#include <QString>
#include <QObject>

class QScriptEngine;
class CGexGroupOfFiles;
class CGexFileInGroup;

namespace GS
{
namespace Gex
{

class WaferExport : public QObject
{
    Q_OBJECT

public:

    WaferExport(QObject* parent=0);
    //! \brief Destructor: actually nothing to be done.
    ~WaferExport();
    //! \brief Copy constructor needed for scripting
    WaferExport(const WaferExport&);

    enum Output
    {
        outputTSMCink               = 0x0001,
        outputSemiG85inkAscii       = 0x0002,
        outputSemiG85inkXml         = 0x0004,
        outputSemiE142ink           = 0x0008,
        outputKLA_INF               = 0x0010,
        outputXlsHtml               = 0x0020,
        outputPng                   = 0x0040,
        outputSTIF                  = 0x0080,
        outputLaurierDieSort1D      = 0x0100,
        outputTELP8                 = 0x0200,
        outputSemiE142inkInteger2   = 0x0400,
        outputOlympusAL2000         = 0x0800,
        outputSumMicron             = 0x1000
    };

    Q_INVOKABLE const QString& GetErrorMessage() const;
    //! \brief Try to generate the output filename of the map and return it if possible, else an empty string
    Q_INVOKABLE QString        GenerateOutputName(Output lFormat, int lGroupIdx = 0, int lFileIdx = 0);
    //! \brief Try to create the output map using given format, outputing to given filename using data in given gourp/file
    Q_INVOKABLE bool           CreateOutputMap(Output lFormat,
                                               const QString& lOutputFileName,
                                               int lGroupIdx = 0,
                                               int lFileIdx = 0);
    //! \brief Generate an Olympus AL2000 map file.
    //! \return Returns "ok" or "error: ...."
    Q_INVOKABLE QString        CreateOlympusAL2kOutputMap(GS::Gex::PATProcessing& lPP, CGexFileInGroup* lFile);

    //! \brief Set rotation of the wafer but does not rotate anything for the moment
    Q_INVOKABLE void SetRotateWafer(bool lRotate);
    //! \brief Set the orientation : 12=up 6=down, 3=right, 9=left ?
    Q_INVOKABLE void SetOrientation(int lOrientation);
    Q_INVOKABLE void SetXAxisDirection(int lXDirection);
    Q_INVOKABLE void SetYAxisDirection(int lYDirection);
    Q_INVOKABLE void SetCustomer(const QString& lCustomer);
    Q_INVOKABLE void SetSupplier(const QString& lSupplier);
    Q_INVOKABLE void SetSINFInfo(const PAT::SINFInfo& lInfo);
    Q_INVOKABLE void SetOriginalFiles(const QStringList& lOriginalFiles);

    static bool IsSupportedOutputFormat(const QString& lStringFormat, int& lFormat);

protected:

    Q_INVOKABLE bool CreateTSMCOutputMap(const QString& lOutputFileName, int lGroupIdx, int lFileIdx);
    Q_INVOKABLE bool CreateE142OutputMap(bool isInteger2, const QString& lOutputFileName, int lGroupIdx, int lFileIdx);
    Q_INVOKABLE bool CreateG85OutputMap(const QString& lOutputFileName, int lGroupIdx, int lFileIdx,
                                       bool lXMLFormat);
    Q_INVOKABLE bool CreateSINFOutputMap(const QString& lOutputFileName, int lGroupIdx, int lFileIdx);
    Q_INVOKABLE bool CreateSTIFOutputMap(const QString& lOutputFileName, int lGroupIdx, int lFileIdx);
    Q_INVOKABLE bool CreateTELP8OutputMap(const QString& lOutputFileName, int lGroupIdx, int lFileIdx);

    //! \brief Generate a .SUM Micron map file.
    //! \return Returns true if the output map is correctly generated. Otherwise, false
    Q_INVOKABLE bool GenerateSumMicronOutputMap(const QString& lOutputFileName,
                                                int lGroupIdx,
                                                int lFileIdx);
    //! \brief Generate a .SUM Micron map name.
    //! \return Returns the output name
    Q_INVOKABLE QString GenerateSumMicronOutputName(const QString &customName, CGexFileInGroup *file);


    Q_INVOKABLE QString GenerateE142OutputName(bool isInteger2, const QString &lCustomName, CGexFileInGroup * lFile);
    Q_INVOKABLE QString GenerateG85OutputName(const QString &lCustomName, CGexFileInGroup *lFile,
                                          bool lXMLFormat);
    Q_INVOKABLE QString GenerateHTMLOutputName(const QString& lCustomName, CGexFileInGroup *lFile);
    Q_INVOKABLE QString GenerateLaurierDieSort1DOutputName(const QString& lCustomName,
                                                       CGexFileInGroup *lFile);
    Q_INVOKABLE QString GeneratePNGOutputName(const QString& lCustomName, CGexFileInGroup *lFile);
    Q_INVOKABLE QString GenerateSINFOutputName(const QString &lCustomName, CGexFileInGroup * lFile);
    Q_INVOKABLE QString GenerateSTIFOutputName(const QString &lCustomName, CGexFileInGroup * lFile);
    Q_INVOKABLE QString GenerateTELP8OutputName(const QString &lCustomName, CGexFileInGroup * lFile);
    Q_INVOKABLE QString GenerateTSMCOutputName(const QString &lCustomName, CGexFileInGroup * lFile);

    Q_INVOKABLE QString GetCustomWaferMapFilename() const;
    Q_INVOKABLE QString GetBCDTime(long lTimeStamp) const;
    bool            InitScriptEngineProperties(QScriptEngine& lScriptEngine) const;

private:

    //! \brief Write the header of the .SUM Micron map
    //! \return Returns true if the header has been writen correctly
    bool WriteMicronSumHeader(QString headerFileName, QFile &outputFile, CGexFileInGroup* stdfFile);

    //! \brief Write the summary of the .SUM Micron map
    //! \return Returns true if the summary has been writen correctly
    bool WriteMicronSumSummary(QString summaryFileName,
                               QFile &outputFile,
                               QMap<QString, CPatDefinition *> &enabledPatTests,
                               QMap<QString, QString> &enabledPatRules);

    //! \brief Write the trend log of the .SUM Micron map
    //! \return Returns true if the trend log has been writen correctly
    bool WriteMicronSumTrendLog(QStringList& diesList,
                                QStringList &binsList,
                                QFile &outputFile,
                                QMap<QString, CPatDefinition *> &enabledPatTests,
                                QMap<QString, QString> &enabledPatRules);

    //! \brief Get the list of the outlier per test
    bool GetOutliersPerTest(QHash<QString, int> &outliersPerTest);

    //! \brief Get the list of dies string from the trend log file
    bool GetDiesListFromTrendLog(QString lTrendLog, QStringList& diesList, QStringList &binsList);

    //! \brief list of the number of outliers in NNR, IDDQ_Delta, GDBN, Reticle, Clusteringa and ZPAT Pat Algorithm
    bool GetNumberOutliersPerPatAlgo(QList<int> &patOutlierPerAlgo);

    //! \brief Constract the die string
    bool GetDieString(const QString dieElement, const QString originalBin, QString& dieString);

    //! \brief Get the coordinates X and Y ffrom the die string
    bool GetXYFromMicronDieString(QString dieString, qint16& lX, qint16& lY);


    // Copy Needed for scripting instantiation
    //Q_DISABLE_COPY(WaferExport)

    bool            mRotateWafer;
    int             mWaferOrientation;
    int             mAxisXDirection;
    int             mAxisYDirection;
    QString         mErrorMessage;
    QString         mOutputFilename;
    QString         mCustomer;
    QString         mSupplier;
    QStringList     mOriginalFiles;
    PAT::SINFInfo   mSINFInfo;
};

} // end namespace Gex
} // end namespace GS

#endif // WAFER_EXPORT_H
