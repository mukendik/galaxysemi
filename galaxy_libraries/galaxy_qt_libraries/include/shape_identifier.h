#ifndef SHAPE_IDENTIFIER_H
#define SHAPE_IDENTIFIER_H

#include "r_algo.h"
#include "r_vector.h"

namespace GS
{
namespace SE
{

class ShapeIdentifier: public RAlgo
{
public:
    /// \brief Constructor
    ShapeIdentifier();
    /// \brief Destructor
    ~ShapeIdentifier();
    /// \brief return shape name
    RVector GetShapeName(bool& ok) const;
    /// \brief return confidence level
    RVector GetConfidenceLevel(bool& ok) const;
    /// \brief return bimodal shape split value
    RVector GetBimodalSplitValue(bool& ok) const;
    ///\brief build histogram png
    bool BuildHistogram(RVector &data,
                       const QString& filePath,
                       const QSize& size,
                       const QString &title = "");

private:
    enum ResultItem
    {
        SHAPE        = 0,
        CONFIDENCE   = 1,
        BIMODAL_SPLIT= 2
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

#endif // SHAPE_IDENTIFIER_H
