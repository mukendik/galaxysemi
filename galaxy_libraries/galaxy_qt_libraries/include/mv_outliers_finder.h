#ifndef MV_OUTLIERS_FINDER_H
#define MV_OUTLIERS_FINDER_H

/*! \class MVOutliersFinder
 * \brief
 *
 */

#include <QSize>

#include "r_algo.h"
#include "r_vector.h"
#include "r_matrix.h"

namespace GS
{
namespace SE
{

class MVOutliersFinder: public RAlgo
{
public:
    /// \brief Constructor
    MVOutliersFinder();
    /// \brief Destructor
    ~MVOutliersFinder();
    /// \brief return vector with indexes of outliers found
    RVector GetOutliers(bool &ok) const;
    /// \brief return chi value
    double GetChi(bool& ok) const;
    /// \brief return vector of mahalanobis distances
    RVector GetMD(bool& ok) const;
    /// \brief return vector of squarred mahalanobis distances
    RVector GetMD2(bool& ok) const;
    /// \brief return PCA matrix
    RMatrix GetPCA(bool& ok) const;
    /// \brief return vector of zscore
    RVector GetZScores(bool& ok) const;
    /// \brief return sigma value used
    double GetSigma(bool& ok) const;
    /// \brief return id of parts detected as Not valid
    RVector GetNaParts(bool& ok) const;
    /// \brief return number of principale component used
    int GetPcNumber(bool& ok) const;
    ///\brief build histogram png
    bool BuildZScoresHisto(const QString& filePath,
                           const QSize& size,
                           const QString &title = "",
                           const QString &xLabel = "",
                           const QString &yLabel = "");
    ///\brief build trend png
    bool BuildZScoresTrend(const QString& filePath,
                           const RVector& labels,
                           const QSize& size,
                           const QString &title = "",
                           const QString &xLabel = "",
                           const QString &yLabel = "");
    ///\brief build qqplot png
    bool BuildZScoresQQPlot(const QString& filePath,
                            const QSize& size,
                            const QString &title = "",
                            const QString &xLabel = "",
                            const QString &yLabel = "");
    ///\brief build correlation chart
    bool BuildCorrelationChart(RMatrix &data,
                               int col1,
                               int col2,
                               const QString& filePath,
                               const RVector& labels,
                               const QSize& size,
                               const QString &title = "",
                               const QString &xLabel = "",
                               const QString &yLabel = "",
                               double chi = 0.0,
                               double sigma = 0.0);

private:
    enum ResultItem
    {
        OUTLIERS        = 0,
        CHI             = 1,
        MAHALANOBISD    = 2,
        MAHALANOBISD2   = 3,
        PCA             = 4,
        ZSCORES         = 5,
        SIGMAVALUE      = 6,
        NAROWS          = 7,
        PCNUMBER        = 8
    };

    /// \brief Init required data
    void InitRequiredData();
    /// \brief Init required parameters
    void InitRequiredParameters();
    /// \brief  compute algo
    bool DoExecute(QMap<Parameter, QVariant> params, StatsData *data);
};

} // namespace SE
} // namespace GS

#endif // MV_OUTLIERS_FINDER_H
