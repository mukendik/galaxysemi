#ifndef XYCHART_DATA_H
#define XYCHART_DATA_H

#include <QStringList>

/////////////////////////////////////////////////////////////////////////////////////////
// CLASS: GexDbPlugin_XYChart_Data
// Store data for one XY chart
/////////////////////////////////////////////////////////////////////////////////////////
class GexDbPlugin_XYChart_Data
{
public:
    // Constructor
    GexDbPlugin_XYChart_Data();
    // Destructor
    ~GexDbPlugin_XYChart_Data();
    // Add a data
    void Add(const double lfData, const QString & strLabel);
    void Add(const double lfData, const QString & strLabel, const double lfData_1);
    void Add(const double lfData, const QString & strLabel, const double lfData_1, const double lfData_2);
    void Add(const double lfData, const QString & strLabel, const double lfData_1, const double lfData_2, const double lfData_3);
    void Add(const double lfData, const QString & strLabel, const double lfData_1, const double lfData_2, const double lfData_3, const double lfData_4);
    // Allocate mem for pointer lists, and fill from private QT lists
    void Convert();
    // Public data
    QString                         m_strSplitValue;
    QString                         m_strYAxisLegend;
    QString                         m_strYAxisLegend_2;
    double                          *m_plfYAxisData;
    double                          *m_plfYAxisData_2;
    const char*                     *m_pcXAxisLabels;
    double                          *m_plfData;
    double                          *m_plfData_1;
    double                          *m_plfData_2;
    double                          *m_plfData_3;
    double                          *m_plfData_4;
    int                             m_nNbDataPoints;
    bool                            m_bDoubleYAxis;
    int                             m_nMarkerPrecision;
    int                             m_nMarkerPrecision_2;

    QStringList                     m_strlStrings;             // Store labels for X-Axis
    QList<double>                   m_lflist_Data;             // Store value for Y-Axis
    QList<double>                   m_lflist_Data_1;           // Store optional intermediate value
    QList<double>                   m_lflist_Data_2;           // Store optional intermediate value
    QList<double>                   m_lflist_Data_3;           // Store optional intermediate value
    QList<double>                   m_lflist_Data_4;           // Store optional intermediate value
};

#endif // XYCHART_DATA_H
