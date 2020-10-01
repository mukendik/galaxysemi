#ifndef G_SEXP_H
#define G_SEXP_H

#include <Rinternals.h>

namespace GS
{
namespace SE
{

class GSexp
{
public:
    GSexp();
    GSexp(SEXP sexp);

    SEXP mSexp;
};

} // namespace SE
} // namespace GS

#endif // G_SEXP_H
