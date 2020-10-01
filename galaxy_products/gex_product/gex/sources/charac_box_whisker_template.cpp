#include "charac_box_whisker_template.h"

namespace GS
{
namespace Gex
{

CharacBoxWhiskerTemplate::CharacBoxWhiskerTemplate()
{
}

CharacBoxWhiskerTemplate::CharacBoxWhiskerTemplate(const CharacBoxWhiskerTemplate &other)
{
    *this = other;
}

CharacBoxWhiskerTemplate::~CharacBoxWhiskerTemplate()
{
}

CharacBoxWhiskerTemplate &CharacBoxWhiskerTemplate::operator =(const CharacBoxWhiskerTemplate &other)
{
    if (this != &other)
    {
        mTopLevelColors     = other.mTopLevelColors;
        mTopLevelCondition  = other.mTopLevelCondition;
    }

    return *this;
}

void CharacBoxWhiskerTemplate::Clear()
{
    mTopLevelColors.clear();
    mTopLevelCondition.clear();
}

QString CharacBoxWhiskerTemplate::GetTopLevelCondition() const
{
    return mTopLevelCondition;
}

QStringList CharacBoxWhiskerTemplate::GetTopLevelAggregates() const
{
    return mTopLevelColors.keys();
}

QColor CharacBoxWhiskerTemplate::GetTopLevelColor(const QString &lConditionValue) const
{
    return mTopLevelColors.value(lConditionValue);
}

void CharacBoxWhiskerTemplate::SetTopLevelColor(const QString &lConditionValue, const QColor &lColor)
{
    mTopLevelColors.insert(lConditionValue, lColor);
}

void CharacBoxWhiskerTemplate::SetTopLevelCondition(const QString &lCondition)
{
    mTopLevelCondition = lCondition;
}

}   // namespace Gex
}   // namespace GS
