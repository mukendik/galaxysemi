#include <math.h>
#include <float.h>
#include "gstdl_linreg.h"

namespace GS
{
namespace StdLib
{
LinearRegression::LinearRegression(Point2D *p, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addPoint(p[i]);
}

LinearRegression::LinearRegression(double *x, double *y, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addXY(x[i], y[i]);
}

void LinearRegression::addXY(const double& x, const double& y)
{
    n++;
    sumX += x;
    sumY += y;
    sumXsquared += x * x;
    sumYsquared += y * y;
    sumXY += x * y;
    Calculate();
}

void LinearRegression::Calculate()
{
    if (haveData())
    {
        if (fabs( double(n) * sumXsquared - sumX * sumX) > DBL_EPSILON)
        {
            b = ( double(n) * sumXY - sumY * sumX) /
                    ( double(n) * sumXsquared - sumX * sumX);
            a = (sumY - b * sumX) / double(n);

            double sx = b * ( sumXY - sumX * sumY / double(n) );
            double sy2 = sumYsquared - sumY * sumY / double(n);
            double sy = sy2 - sx;

            coefD = sx / sy2;
            coefC = sqrt(coefD);
            stdError = sqrt(sy / double(n - 2));
        }
        else
        {
            a = b = coefD = coefC = stdError = 0.0;
        }
    }
}

std::ostream& operator<<(std::ostream& out, LinearRegression& lr)
{
    if (lr.haveData())
        out << "f(x) = " << lr.getA()
            << " + ( " << lr.getB()
            << " * x )";
    return out;
}

double x[] = { 71,  73,  64,  65,  61,  70,  65,  72,  63,  67,  64};
double y[] = {160, 183, 154, 168, 159, 180, 145, 210, 132, 168, 141};

LinearRegression::Point2D p[] = {
    LinearRegression::Point2D(71, 160), LinearRegression::Point2D(73, 183), LinearRegression::Point2D(64, 154),
    LinearRegression::Point2D(65, 168), LinearRegression::Point2D(61, 159), LinearRegression::Point2D(70, 180),
    LinearRegression::Point2D(65, 145), LinearRegression::Point2D(72, 210), LinearRegression::Point2D(63, 132),
    LinearRegression::Point2D(67, 168), LinearRegression::Point2D(64, 141)
};

// Unit test
void LinearRegressionUnitTest()
{
    std::cout << "Linear Regression Test\n" << std::endl;
    LinearRegression lr(x, y, 11);  // create with two arrays
    std::cout << "Number of x,y pairs = " << lr.items() << std::endl;
    std::cout << lr << std::endl;
    std::cout << "Coefficient of Determination = "         << lr.getCoefDeterm() << std::endl;
    std::cout << "Coefficient of Correlation = "         << lr.getCoefCorrel() << std::endl;
    std::cout << "Standard Error of Estimate = "         << lr.getStdErrorEst() << std::endl;

    std::cout << "\nLinear Regression Test Part 2 (using Point2Ds)\n" << std::endl;
    LinearRegression lr2(p, 11);  // create with array of points
    std::cout << "Number of x,y pairs = " << lr2.items() << std::endl;
    std::cout << lr2 << std::endl;
    std::cout << "Coefficient of Determination = "         << lr2.getCoefDeterm() << std::endl;
    std::cout << "Coefficient of Correlation = "         << lr2.getCoefCorrel() << std::endl;
    std::cout << "Standard Error of Estimate = "         << lr2.getStdErrorEst() << std::endl;

    std::cout << "\nLinear Regression Test Part 3 (empty instance)\n" << std::endl;
    LinearRegression lr3;   // empty instance of linear regression
    for (int i = 0; i < 11; i++)
        lr3.addPoint(p[i]);
    std::cout << "Number of x,y pairs = " << lr3.items() << std::endl;
    std::cout << lr3 << std::endl;
    std::cout << "Coefficient of Determination = " << lr3.getCoefDeterm() << std::endl;
    std::cout << "Coefficient of Correlation = " << lr3.getCoefCorrel() << std::endl;
    std::cout << "Standard Error of Estimate = " << lr3.getStdErrorEst() << std::endl;
}
}
}
