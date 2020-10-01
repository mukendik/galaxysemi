#ifndef R_PROTECTED_OBJECT_H
#define R_PROTECTED_OBJECT_H

/*! \class RProtectedObject
 * \brief
 *
 */

#include <QString>
#include <Rinternals.h>

namespace GS
{
namespace SE
{

class RProtectedObject
{
public:
    /// \brief Constructor
    RProtectedObject(SEXP object, const QString& name);
    /// \brief Destructor
    ~RProtectedObject();
    /// \brief return name of object in R mem
    QString GetName();
    /// \brief return SEXP (R object)
    SEXP GetSexp();

protected:
    QString mName;      ///< holds name in R mem
    SEXP    mSexp;      ///< holds R object
};

} // namespace SE
} // namespace GS

#endif // R_PROTECTED_OBJECT_H
