#ifndef R_OBJECT_PRIVATE_H
#define R_OBJECT_PRIVATE_H

#include <QStringList>

namespace GS
{
namespace SE
{

class RProtectedObject;
class GSexp;

class RObjectPrivate
{
public:
    RObjectPrivate();
    RObjectPrivate(GSexp* gsexp);
    virtual ~RObjectPrivate();

    RProtectedObject*   mProtectedObject;   ///< holds pointer to protected object if protected
    GSexp*              mGSexp;             ///< holds SEXP (R object)
    QStringList         mErrors;            ///< holds errors
};

} // namespace SE
} // namespace GS

#endif // R_OBJECT_PRIVATE_H
