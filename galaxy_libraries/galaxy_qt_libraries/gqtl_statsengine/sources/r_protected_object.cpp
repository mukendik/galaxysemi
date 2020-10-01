
#include "r_protected_object.h"

namespace GS
{
namespace SE
{

RProtectedObject::RProtectedObject(SEXP object, const QString& name)
{
    R_PreserveObject(object);
    mName = name;
    mSexp = object;
}

RProtectedObject::~RProtectedObject()
{
    R_ReleaseObject(mSexp);
}

QString RProtectedObject::GetName()
{
    return mName;
}

SEXP RProtectedObject::GetSexp()
{
    return mSexp;
}

} // namespace SE
} // namespace GS

