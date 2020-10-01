#ifndef GEX_CHARAC_LINE_CHART_H
#define GEX_CHARAC_LINE_CHART_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexabstractchart.h"

namespace GS
{
namespace Gex
{

class CharacLineChart : public GexAbstractChart
{
public:

    /*!
        @brief  Constructs a line chart used for Characterization report.
    */
    CharacLineChart(int nSizeMode, GexWizardChart *lWizardParent, CGexChartOverlays * pChartOverlays = NULL);
    ~CharacLineChart();

    /*!
        @brief  Returns the label used as a tooltip

        @return QString containing the formatted tooltip
    */
    QString		makeTooltip();

    /*!
        \fn void CharacLineChart::setViewportModeFromChartMode(int nChartMode)

        Sets the viewport mode to use.
    */
    void		setViewportModeFromChartMode(int nChartMode);

    /*!
        @brief  Sets the variable name used for serie onto the chart

        @param  variables   List of the variable
    */
    void        SetSerieVariables(const QStringList& variables);

protected:

    /*!
        \fn void CharacLineChart::buildOwnerChart()

        Build the chart image
    */
    void		buildOwnerChart();

    /*!
        \fn void CharacLineChart::drawLayers()

        Draw the layers
    */
    bool        drawLayers();
//    bool        drawMarkers(CGexFileInGroup * pFileX, CTest * pTestX);
//    bool        drawMarkersLimits(CGexFileInGroup * pFileX, CTest * pTestX);

    /*!
        \fn void CharacLineChart::drawTitles();

        Draws the titles onto the chart (chart and axis titles)
    */
    void        drawTitles();

    /*!
        \fn void CharacLineChart::drawXAxisLabels()

        Draws the labels on the x-axis
    */
    void        drawXAxisLabels();

    /*!
        @brief Draw the markers limit onto the chart
    */
    void        drawMarkersLimits();

    /*!
        \fn void CharacLineChart::computeDataset()

        Computes the dataset to plot in the chart
    */
    void		computeDataset();

    /*!
        \fn void CharacLineChart::keepXTestViewport()

        Keep the viewport values for the reference test on X-axis
    */
    void		keepXTestViewport();

    /*!
        @brief Scale the x and y axis depending on the viewport.
    */
    void        scaleChart();

    /*!
        @brief fit the plot area depending on the axix thickness
    */
    void        fitPlotArea();

    /*!
        @brief Create Datasets from the template
    */
    void        CreateDatasetsFromTemplate();

private:

    void        CleanLabels();

private:

    class privateDataset
    {
    public:
        privateDataset(const QString& key);
        privateDataset(const QString& key, int size);
        privateDataset(const privateDataset& other);
        ~privateDataset();

        bool            Resize(int size);

        privateDataset& operator=(const privateDataset& other);
        bool operator==(const privateDataset& other) const;

        const QString&  GetVariable() const;
        const QString&  GetName() const;
        const QString&  GetKey() const;
        int             GetColor() const;
        int             GetSize() const;
        double          GetData(int index) const;
        double          GetMinData() const;
        double          GetMaxData() const;
        double *        GetDataArray() const;

        void            SetColor(int color);
        void            SetData(int index, double value);
        void            SetName(const QString& lName);
        void            SetVariable(const QString& lVariable);

    private:

        QString         mKey;
        QString         mName;
        QString         mVariable;
        double *        mDataArray;
        int             mSize;
        int             mColor;
        double          mMinData;
        double          mMaxData;
    };

private:

    QStringList                     mAggregateVariable;
    QStringList                     mSerieVariable;
    QList< QMap<QString, QString> > mAggregate;
    QList<privateDataset>           mDatasets;
    char **                         mLabels;
    GexInternalChartInfo            mInternalChartInfo;
};

} // namespace Gex
} // namespace GS


#endif // GEX_CHARAC_LINE_CHART_H
