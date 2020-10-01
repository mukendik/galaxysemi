#include "db_key_data.h"

namespace GS
{
namespace Gex
{
DbKeyData::DbKeyData(QString name, QString value, int flowId)
{
    mName = name;
    mValue = value;
    mFlowId = flowId;
    mIsValidExpression = true;
    mIsValidName = true;
}

DbKeyData::DbKeyData(const DbKeyData *source)
{
    mName               = source->mName;
    mValue              = source->mValue;
    mFlowId             = source->mFlowId;
    mIsValidExpression  = source->mIsValidExpression;
    mIsValidName        = source->mIsValidName;
    mExpression         = source->mExpression;
    mEvaluatedValue     = source->mEvaluatedValue;
}

DbKeyData::~DbKeyData()
{
}

void DbKeyData::SetName(QString name)
{
    mName = name.trimmed();
    emit DataChanged(mFlowId);
}
void DbKeyData::SetValue(QString value)
{
    mValue = value.trimmed();
    emit DataChanged(mFlowId);
}
void DbKeyData::SetExpression(QString expression)
{
    mExpression = expression.trimmed();
    emit DataChanged(mFlowId);
}
void DbKeyData::SetEvaluatedValue(QString evaluatedValue, bool isValidExpression)
{
    mEvaluatedValue = evaluatedValue.trimmed();
    mIsValidExpression = isValidExpression;
    emit DataChanged(mFlowId);
}
void DbKeyData::SetFlowId(int flowId)
{
    mFlowId = flowId;
}

void DbKeyData::SetNameIsValid(bool isValid)
{
    mIsValidName = isValid;
}

void DbKeyData::SetExpressionIsValid(bool isValid)
{
    mIsValidExpression = isValid;
}

} // END Gex
} // END GS

