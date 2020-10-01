#include "charac_line_chart_template.h"

namespace GS
{
namespace Gex
{

CharacLineChartSerie::CharacLineChartSerie()
{
}

CharacLineChartSerie::CharacLineChartSerie(const CharacLineChartSerie &other)
{
    *this = other;
}

CharacLineChartSerie::~CharacLineChartSerie()
{
}

CharacLineChartSerie &CharacLineChartSerie::operator =(const CharacLineChartSerie &other)
{
    if (this != &other)
    {
        mColor      = other.mColor;
        mConditions = other.mConditions;
        mVariable   = other.mVariable;
        mName       = other.mName;
    }

    return *this;
}

const QStringList &CharacLineChartSerie::GetConditions() const
{
    return mConditions;
}

const QString &CharacLineChartSerie::GetName() const
{
    return mName;
}

const QString &CharacLineChartSerie::GetVariable() const
{
    return mVariable;
}

const QColor &CharacLineChartSerie::GetColor() const
{
    return mColor;
}

void CharacLineChartSerie::SetConditions(const QStringList &conditions)
{
    mConditions = conditions;
}

void CharacLineChartSerie::SetColor(const QColor &lColor)
{
    mColor = lColor;
}

void CharacLineChartSerie::SetVariable(const QString &lVariable)
{
    mVariable = lVariable;
}

void CharacLineChartSerie::SetName(const QString &lName)
{
    mName = lName;
}

CharacLineChartTemplate::CharacLineChartTemplate()
{
}

CharacLineChartTemplate::CharacLineChartTemplate(const CharacLineChartTemplate &other)
{
    *this = other;
}

CharacLineChartTemplate::~CharacLineChartTemplate()
{
}

CharacLineChartTemplate &CharacLineChartTemplate::operator =(const CharacLineChartTemplate &other)
{
    if (this != &other)
    {
        mSerieDefinition    = other.mSerieDefinition;
        mSerieCondition     = other.mSerieCondition;
        mVariable           = other.mVariable;
    }

    return *this;
}

int CharacLineChartTemplate::GetSerieDefinitionCount() const
{
    return mSerieDefinition.count();
}

const CharacLineChartSerie &CharacLineChartTemplate::GetSerieDefinitionAt(int lIndex) const
{
    return mSerieDefinition.at(lIndex);
}

const QStringList &CharacLineChartTemplate::GetSerieConditions() const
{
    return mSerieCondition;
}

const QString &CharacLineChartTemplate::GetVariable() const
{
    return mVariable;
}

void CharacLineChartTemplate::AddSerieDefinition(const CharacLineChartSerie &lSerieDef)
{
    mSerieDefinition.append(lSerieDef);
}

void CharacLineChartTemplate::Clear()
{
    mSerieDefinition.clear();
    mSerieCondition.clear();
    mVariable.clear();
}

void CharacLineChartTemplate::SetSerieCondition(const QStringList &lSerieCondition)
{
    mSerieCondition = lSerieCondition;
}

void CharacLineChartTemplate::SetVariable(const QString &lVariable)
{
    mVariable = lVariable;
}

}   // namespace Gex
}   // namespace GS
