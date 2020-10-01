///////////////////////////////////////////////////////////
// QT includes
///////////////////////////////////////////////////////////
#include <gex_algorithms.h>
#include "gex_constants.h"
#include <gqtl_log.h>
#include <gqtl_global.h>
#include <qmath.h>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double gexMeanValue(const QVector<double> &vecValues)
//
// Description	:	return the mean value of the value list
//
///////////////////////////////////////////////////////////////////////////////////
double algorithms::gexMeanValue(const QVector<double> &vecValues)
{
    if (vecValues.count() == 0)
    {
        GSLOG(SYSLOG_SEV_WARNING, "vector empty");
        return GEX_C_DOUBLE_NAN;
    }

    double dValue = 0.0;

    for (int nCount = 0; nCount < vecValues.count(); ++nCount)
        dValue += vecValues[nCount];

    dValue /= (double) vecValues.count();

    return dValue;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double gexMedianValue(const QVector<double> &vecValues)
//
// Description	:	return the median value of the value list
//
///////////////////////////////////////////////////////////////////////////////////
double algorithms::gexMedianValue(const QVector<double> &vecValues, int lBegin, int lEnd)
{
    if (vecValues.count() == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "vecValues.count() == 0 ! returning NAN");
        return GEX_C_DOUBLE_NAN;
    }

    if (lBegin < 0)
        lBegin = 0;
    else if (lBegin >= vecValues.count())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Begin index for median is out of bounds");
        return GEX_C_DOUBLE_NAN;
    }

    if (lEnd == -1)
        lEnd = vecValues.count()-1;
    else if (lEnd >= vecValues.count())
    {
        GSLOG(SYSLOG_SEV_WARNING, "End index for median is out of bounds");
        return GEX_C_DOUBLE_NAN;
    }

    if (lEnd < lBegin)
    {
        GSLOG(SYSLOG_SEV_WARNING, "End index is lesser than Begin for median is out of bounds");
        return GEX_C_DOUBLE_NAN;
    }

    int     lCount  = lEnd - lBegin + 1;
    double	lValue  = GEX_C_DOUBLE_NAN;
    int		lMedian = lCount / 2;

    if (lMedian*2 == lCount)
        lValue = (vecValues[lBegin + lMedian-1] + vecValues[lBegin + lMedian]) / 2.0;
    else
        lValue = vecValues[lBegin + lMedian];

    return lValue;
}
///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double gexQ1Value(const QVector<double> &vecValues)
//
// Description	:	return the Q1 value of the value list
//
///////////////////////////////////////////////////////////////////////////////////
double algorithms::gexQ1Value(const QVector<double> &vecValues)
{
    if (vecValues.count() == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "vecValues.count() == 0 ! returning NAN");
        return GEX_C_DOUBLE_NAN;
    }

    unsigned int lQuartile1 = vecValues.count() >= 3 ? (int)(0.25*((double)vecValues.count()+1.0)) - 1 : 0;

    return vecValues[lQuartile1];
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double gexQ3Value(const QVector<double> &vecValues)
//
// Description	:	return the Q3 value of the value list
//
///////////////////////////////////////////////////////////////////////////////////
double algorithms::gexQ3Value(const QVector<double> &vecValues)
{
    if (vecValues.count() == 0)
    {
        GSLOG(SYSLOG_SEV_NOTICE, "vecValues.count() == 0 ! returning NAN");
        return GEX_C_DOUBLE_NAN;
    }

    unsigned int	lQuartile3 = vecValues.count() >= 1 ? (int)(0.75*((double)vecValues.count()+1.0)) - 1 : 0;

    return vecValues[lQuartile3];
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int gexCompareIntegerString(const QString& strLeft, const QString& strRight)
//
// Description	:	Compare two strings supposed to be integer
//						return negative value if left is lesser than right
//						return positive value if left is greater than right
//						return 0 if left is equal to right
//
///////////////////////////////////////////////////////////////////////////////////
int algorithms::gexCompareIntegerString(const QString& strLeft, const QString& strRight)
{
    bool	bOkLeft		= false;
    bool	bOkRight	= false;
    int		nLeft		= strLeft.toInt(&bOkLeft);
    int		nRight		= strRight.toInt(&bOkRight);

    if (bOkLeft && bOkRight)
        return (nLeft - nRight);

    return strLeft.compare(strRight);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double gexCompareDoubleString(const QString& strLeft, const QString& strRight)
//
// Description	:	Compare two strings supposed to be double
//						return negative value if left is lesser than right
//						return positive value if left is greater than right
//						return 0 if left is equal to right
//
///////////////////////////////////////////////////////////////////////////////////
double algorithms::gexCompareDoubleString(const QString& strLeft, const QString& strRight,
                                          Qt::CaseSensitivity lCaseSensitivity /*= Qt::CaseSensitive*/)
{
    bool	bOkLeft		= false;
    bool	bOkRight	= false;
    double	lLeft		= strLeft.toDouble(&bOkLeft);
    double	lRight		= strRight.toDouble(&bOkRight);

    if (bOkLeft && bOkRight)
        return (lLeft - lRight);

    return strLeft.compare(strRight, lCaseSensitivity);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double  gexRound(const double& lfValue, const double& lfPrecision)
//
// Description	:	return round to nearest towards plus infinity, with iPrecision decimals
//
///////////////////////////////////////////////////////////////////////////////////
double  algorithms::gexRound(const double& lfValue, const double& lfPrecision)
{
    double lfRoundedValue = 0.0;

    if (lfPrecision >= 0)
        lfRoundedValue = qRound(lfValue*GS_POW(10.0, lfPrecision))/GS_POW(10.0, lfPrecision);

    return lfRoundedValue;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString gexFormatDouble(const double& lfValue, const double& lfPrecision = 6.0, char cFormat = 'f')
//
// Description	:	return a double to a string with iPrecision on cFormat using gexRound
//
///////////////////////////////////////////////////////////////////////////////////
QString algorithms::gexFormatDouble(const double& lfValue, const double& lfPrecision /*= 6.0 */, char cFormat /*= 'f'*/)
{
  QString strFormatedValue =
    QString::number(gexRound(lfValue, lfPrecision), cFormat,
                    static_cast<int>(lfPrecision));

    return strFormatedValue;
}
