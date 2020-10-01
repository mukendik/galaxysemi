#ifndef GQTL_GLOBAL
#define GQTL_GLOBAL

#include <stddef.h>
#include <math.h>

#define GS_POW(x,y) pow(x,y)

#include <QString>

namespace GS
{
namespace Core
{
    template<typename UnitPrefixToScale>
    QString NormalizeUnit(const QString& unit, int& scale, UnitPrefixToScale f)
    {
        scale = 0;

        if (unit.length() > 1)
        {
            scale = f(unit.at(0).toLatin1());
        }

        QString lNormalizedUnit;

        if (scale != 0)
            lNormalizedUnit = unit.mid(1);
        else
            lNormalizedUnit = unit;

        return lNormalizedUnit;
    }

    QString NormalizeUnit(const QString& unit, int& scale);
    QString ScaleUnit(const QString& unit, int scale);

    double  NormalizeValue(double value, int scale);
    double  ScaleValue(double value, int scale);

    int     STDFUnitPrefixToScale(char prefix);
}
}

#endif // GQTL_GLOBAL

