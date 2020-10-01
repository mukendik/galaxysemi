#include "gqtl_global.h"

#include <QString>

namespace GS
{
namespace Core
{

int STDFUnitPrefixToScale(char prefix)
{
    int lScale = 0;

    switch(prefix)
    {
        // List of units we accept to scale to m, u, p, f, etc...
        case 'm':	// Milli...
            lScale = 3;
            break;

        case 'u':	// Micro...
            lScale = 6;
            break;

        case 'n':	// Nano...
            lScale = 9;
            break;

        case 'p':	// Pico...
            lScale = 12;
            break;

        case 'f':	// Fento...
            lScale = 15;
            break;

        case '%':
            lScale = 2;
            break;

        case 'K':	// Kilo...
            lScale = -3;
            break;

        case 'M':	// Mega...
            lScale = -6;
            break;

        case 'G':	// Giga...
            lScale = -9;
            break;

        case 'T':	// Tera...
            lScale = -12;
            break;

        default:
            break;
    }

    return lScale;
}

QString NormalizeUnit(const QString& unit, int& scale)
{
    return NormalizeUnit(unit, scale, STDFUnitPrefixToScale);
}

QString ScaleUnit(const QString& unit, int scale)
{
    QString lScaledUnit;

    if (unit.isEmpty() == false)
    {
        QChar   lUnitPrefix;

        switch(scale)
        {
            case 2:
                lUnitPrefix = '%';
                break;

            case 3:
                lUnitPrefix = 'm';
                break;

            case 6:
                lUnitPrefix = 'u';
                break;

            case 9:
                lUnitPrefix = 'n';
                break;

            case 12:
                lUnitPrefix = 'p';
                break;

            case -3:
                lUnitPrefix = 'K';
                break;

            case -6:
                lUnitPrefix = 'M';
                break;

            case -9:
                lUnitPrefix = 'G';
                break;

            case -12:
                lUnitPrefix = 'T';
                break;

            default:
                break;
        }

        lScaledUnit = lUnitPrefix + unit;
    }

    return lScaledUnit;
}

double  NormalizeValue(double value, int scale)
{
    if (scale != 0)
    {
        double lFactor          = 1.0;

        switch(scale)
        {
            case 2:
                lFactor = 1e-2;
                break;

            case 3:
                lFactor = 1e-3;
                break;

            case 6:
                lFactor = 1e-6;
                break;

            case 9:
                lFactor = 1e-9;
                break;

            case 12:
                lFactor = 1e-12;
                break;

            case -3:
                lFactor = 1e3;
                break;

            case -6:
                lFactor = 1e6;
                break;

            case -9:
                lFactor = 1e9;
                break;

            case -12:
                lFactor = 1e12;
                break;

            default:
                lFactor = GS_POW(10.0, -scale);
                break;
        }

        return value * lFactor;
    }

    return value;
}

double  ScaleValue(double value, int scale)
{
    if (scale != 0)
    {
        double lFactor = 1.0;

        switch(scale)
        {
            case 2:
                lFactor = 1e2;
                break;

            case 3:
                lFactor = 1e3;
                break;

            case 6:
                lFactor = 1e6;
                break;

            case 9:
                lFactor = 1e9;
                break;

            case 12:
                lFactor = 1e12;
                break;

            case -3:
                lFactor = 1e-3;
                break;

            case -6:
                lFactor = 1e-6;
                break;

            case -9:
                lFactor = 1e-9;
                break;

            case -12:
                lFactor = 1e-12;
                break;

            default:
                lFactor = GS_POW(10.0, scale);
                break;
        }

        return value * lFactor;
    }

    return value;
}


}
}
