#ifndef DEF_HISTOGRAM_H
#define DEF_HISTOGRAM_H

namespace GS
{
namespace Gex
{

class HistogramData
{
public:
    HistogramData();		// Constructeur
    HistogramData(int, double, double);
    ~HistogramData();		// Destructeur
	int		GetTotalBars() const;
	double	GetMinRange() const;
	double	GetMaxRange() const;
    HistogramData& operator=(const HistogramData& other);

    bool	AddValue(double, bool lForce = false);
	int		GetClassCount(int) const;
    double  GetClassRange(int lCell) const;

private:
    static unsigned sNumOfInstances;
    int		mTotalBars;			// Number of bars in the Histogram
	double	mMinRange;			// (Double)
	double	mMaxRange;			// (Double)
	int	*	mHistoData;
};
}
}
#endif
