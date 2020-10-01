#include <exception>

namespace GS
{
namespace Gex
{

/**
 * @brief basic exception class used when statistical agent job data is badly
 * initialized
 */
class bad_testing_stage_used :
    public std::exception
{
public :
    /**
     * @brief wtf message for this exception
     * @return a wtf message
     */
    const char * what() const throw() { return "Invalid testing stage used."; }
};

/**
 * @brief basic exception class used when statistical agent job data is badly
 * initialized
 */
class bad_agent_type_used :
    public std::exception
{
public :
    /**
     * @brief wtf message for this exception
     * @return a wtf message
     */
    const char * what() const throw() { return "Invalid agent type used."; }
};

/**
 * @brief basic exception class used when an unhandled testing stage is
 * specified, should never occur, normally
 */
class unhandled_testing_stage_specified :
    public std::exception
{
public :
    /**
     * @brief wtf message for this exception
     * @return a wtf message
     */
    const char * what() const throw()
    { return "Unhandled testing stage specified."; }
};


} // namespace Gex
} // namespace GS