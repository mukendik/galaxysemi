#ifndef UTILITY_H
#define UTILITY_H

namespace GS
{
namespace Gex
{
/**
    \brief this is a special class allowing user to benefit the empty base class
    optimization when having a state being an empty type

    \tparam MAYBE_STATELESS An user type that may be empty (doesn't contain any
    state)
    \tparam MEMBER_TYPE A state of an using type that is embedded here
 */
template< class MAYBE_STATELESS, class MEMBER_TYPE >
    struct base_opt :
    MAYBE_STATELESS
    {
        /**
            \brief The state originally belonging to the user type and that is
            embedded here.
         */
        MEMBER_TYPE m_member;

        /**
            \brief explicit constructor initializing the maybe empty base and
            the embedded member

            \param base const ref to base instance used to initialize the base
            \param member const ref to embedded member instance
         */
        explicit base_opt
            (
                const MAYBE_STATELESS &base = MAYBE_STATELESS(),
                const MEMBER_TYPE &member = MEMBER_TYPE()
            ) :
            MAYBE_STATELESS( base ),
            m_member( member ) {}
    };
}
}

#endif // UTILITY_H
