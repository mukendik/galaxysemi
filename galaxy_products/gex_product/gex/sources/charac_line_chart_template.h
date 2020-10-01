#ifndef CHARAC_LINE_CHART_TEMPLATE_H
#define CHARAC_LINE_CHART_TEMPLATE_H

#include <QStringList>
#include <QColor>

namespace GS
{
namespace Gex
{

class CharacLineChartSerie
{

public:
    CharacLineChartSerie();
    CharacLineChartSerie(const CharacLineChartSerie& other);
    ~CharacLineChartSerie();

    CharacLineChartSerie& operator=(const CharacLineChartSerie& other);

    const QStringList&  GetConditions() const;
    const QString&      GetName() const;
    const QString&      GetVariable() const;
    const QColor&       GetColor() const;

    void                SetConditions(const QStringList& conditions);
    void                SetColor(const QColor& lColor);
    void                SetVariable(const QString& lVariable);
    void                SetName(const QString& lName);

private:

    QColor      mColor;
    QStringList mConditions;
    QString     mVariable;
    QString     mName;
};

class CharacLineChartTemplate
{
public:

    CharacLineChartTemplate();
    CharacLineChartTemplate(const CharacLineChartTemplate& other);
    ~CharacLineChartTemplate();

    CharacLineChartTemplate& operator=(const CharacLineChartTemplate& other);

    int                         GetSerieDefinitionCount() const;
    const CharacLineChartSerie& GetSerieDefinitionAt(int lIndex) const;
    const QStringList&          GetSerieConditions() const;
    const QString&              GetVariable() const;

    void                        Clear();
    void                        AddSerieDefinition(const CharacLineChartSerie& lSerieDef);
    void                        SetSerieCondition(const QStringList& lSerieCondition);
    void                        SetVariable(const QString& lVariable);

private:

    QList<CharacLineChartSerie> mSerieDefinition;
    QStringList                 mSerieCondition;
    QString                     mVariable;
};

}   // namespace Gex
}   // namespace GS

#endif // CHARAC_LINE_CHART_TEMPLATE_H
