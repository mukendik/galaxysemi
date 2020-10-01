#ifndef GEX_CHARAC_HISTO_CHART_H
#define GEX_CHARAC_HISTO_CHART_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

class GexCharacHistoChart : public GexAbstractChart
{
public:

    GexCharacHistoChart(int nSizeMode, GexWizardChart * lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    ~GexCharacHistoChart();

    ///////////////////////////////////////////////////////////
    // Methods
    ///////////////////////////////////////////////////////////

    QString		makeTooltip();                                              // generate the label tooltip's
    void		setViewportModeFromChartMode(int nChartMode);               // convert chart mode from Gex into viewport mode

    void        ForceBottomMargin(int bottom);

protected:

    void		buildOwnerChart();                                          // build the owner chart, must be redefined by sub classes
    void        scaleChart();

    void		computeDataset();                                           // compute the owner dataset
    bool        computeMinMax();                                            // compute min/max samples values over groups
    void		keepXTestViewport();                                        // Keep the viewport values for the reference test X

    GexInternalChartInfo     mInternalChartInfo;

    int                      mClassesCount;

private:

    class privateDataset
    {
    public:
        privateDataset(const QString& key);
        privateDataset(const QString& key, int size);
        privateDataset(const privateDataset& other);
        ~privateDataset();

        bool            resize(int size);

        privateDataset& operator=(const privateDataset& other);
        bool operator==(const privateDataset& other) const;

        const QString&  GetKey() const;
        int             GetColor() const;
        int             GetSize() const;
        double          GetData(int index) const;
        double *        GetDataArray() const;

        void            setColor(int color);
        void            setData(int index, double value);

    private:

        QString     mKey;
        double *    mDataArray;
        int         mSize;
        int         mColor;
    };

protected:

    QList<privateDataset>   mDatasets;
    int                     mForcedBottomMargin;
};

#endif // GEX_CHARAC_HISTO_CHART_H
