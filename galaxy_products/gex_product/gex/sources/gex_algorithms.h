#ifndef GEX_ALGORITHMS_H
#define GEX_ALGORITHMS_H

///////////////////////////////////////////////////////////
// QT includes
///////////////////////////////////////////////////////////
#include <QVector>
#include <QString>
#include <math.h>

///////////////////////////////////////////////////////////
// Algorithms functions
///////////////////////////////////////////////////////////
namespace algorithms
{
    double	gexQ1Value(const QVector<double>& vecValues);                                       // return the Q1 value of the value list
    double	gexQ3Value(const QVector<double>& vecValues);                                       // return the Q3 value of the value list
    double	gexMedianValue(const QVector<double>& vecValues,
                           int lBegin = -1,
                           int lEnd = -1);                                                      // return the median value of the value list
    double	gexMeanValue(const QVector<double>& vecValues);                                     // return the mean value of the value list
    int		gexCompareIntegerString(const QString& strLeft, const QString& strRight);           // Compare two strings supposed to be integer
    double	gexCompareDoubleString(const QString& strLeft, const QString& strRight,
                                   Qt::CaseSensitivity lCaseSensitivity = Qt::CaseSensitive);   // Compare two strings supposed to be double
    double  gexRound(const double& lfValue, const double& lfPrecision);							// return round to nearest towards plus infinity, with iPrecision decimals
    QString gexFormatDouble(const double& lfValue, const double& lfPrecision = 6.0, char cFormat = 'f');// return a double to a string with iPrecision on cFormat using gexRound
}
#endif // GEX_ALGORITHMS_H
