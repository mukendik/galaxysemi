#ifndef CHARAC_BOX_WHISKER_TEMPLATE_H
#define CHARAC_BOX_WHISKER_TEMPLATE_H

#include <QMap>
#include <QColor>

namespace GS
{
namespace Gex
{

class CharacBoxWhiskerTemplate
{
public:

    CharacBoxWhiskerTemplate();
    CharacBoxWhiskerTemplate(const CharacBoxWhiskerTemplate& other);
    ~CharacBoxWhiskerTemplate();

    CharacBoxWhiskerTemplate& operator=(const CharacBoxWhiskerTemplate& other);

    void                        Clear();

    QString                     GetTopLevelCondition() const;
    QStringList                 GetTopLevelAggregates() const;
    QColor                      GetTopLevelColor(const QString& lConditionValue) const;
    void                        SetTopLevelColor(const QString& lConditionValue,
                                                 const QColor& lColor);
    void                        SetTopLevelCondition(const QString& lCondition);

private:

    QMap<QString, QColor>       mTopLevelColors;
    QString                     mTopLevelCondition;
};

}   // namespace Gex
}   // namespace GS
#endif // CHARAC_BOX_WHISKER_TEMPLATE_H
