#ifndef _DUMPABLEAS_H_
#define _DUMPABLEAS_H_

namespace GS
{
namespace Gex
{

/**
 * @brief base type for a object designed to be dumped.
 *
 * @tparam T the type representing a dumped object
 */
template< class T >
  class DumpableAs
{
public :
    /**
     * @brief standard alias
     */
    typedef T value_type;

    /**
     * @brief ~DumpableAs, ok need it to mute a fkng warning
     */
    virtual ~DumpableAs() {}

    /**
     * @brief virtual pure method giving the basic contract for dumping
     * @return the representation of the dumped object
     */
    virtual value_type Dump() const = 0;

    /**
     * @brief Clone this instance
     * @return a freshly cloned instance address
     */
    virtual DumpableAs * Clone() const = 0;
};

} // namespace Gex
} // namespace GS
#endif // _DUMPABLEAS_H_
