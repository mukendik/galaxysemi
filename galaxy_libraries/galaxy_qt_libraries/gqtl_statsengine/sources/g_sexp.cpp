#include "g_sexp.h"

namespace GS
{
namespace SE
{

GSexp::GSexp():
    mSexp(0)
{

}

GSexp::GSexp(SEXP sexp):
    mSexp(sexp)
{
}

} // namespace SE
} // namespace GS

