#ifndef _NULLABLEOF_H_
#define _NULLABLEOF_H_

#include <utility>

namespace GS
{
namespace Gex
{
/**
 * @brief The NullType represents the concept of the null value type
 */
struct NullType {};

/**
 * @brief Null is a variable with static linkage, instance of an empty object usable as a placeholder in a
 * NullableOf< T > constructor to initialize a null value, no matter its underlying type
 */
static NullType Null;

/**
 * @brief simple concept of a nullable value. Represented by an underlying pair
 * containg both the type of the value and a boolean indicating the null-state.
 * The pair is < T, bool > (not < bool, T >) to avoid unnecessary padding
 *
 * @tparam T type of the value to get nullable
 */
template< class T >
    class NullableOf :
    public std::pair< T, bool >
{
public :
    /**
     * @brief standard alias
     */
    typedef T value_type;

    /**
     * @brief standard alias
     */
    typedef const T & const_reference;

    /**
     * @brief standard alias
     */
    typedef T & reference;

    /**
     * @brief handful alias on the base type
     */
    typedef std::pair< T, bool > base_type;

    /**
     * @brief NullableOf constructor taking a placeholder to initialize this instance to null. Same effect as the
     * default constructor
     */
    NullableOf( const NullType & ) :
        base_type( value_type(), false ) { ( void ) Null; } // this void statement is to avoid a warning about unused
                                                            // variable in this compilation unit
    /**
     * @brief default construction, object is null
     */
    NullableOf() :
        base_type( value_type(), false ) {}

    /**
     * @brief Parametric construction, object is initialized and not null
     * @param data data to initialize the object
     */
    NullableOf( const_reference data ) :
        base_type( data, true ) {}

    /**
     * @brief get null state of this instance
     * @return true if null, false otherwise
     */
    bool is_null() const { return ! this->second; }

    /**
     * @brief set this instance null. Doen't touch to the underlying object,
     * even if initialized
     */
    void set_null() { this->second = false; }

    /**
     * @brief non const access to the underlying object, don't worry about
     * null-state
     * @return a mutable reference to the underlying object
     */
    reference get_data() { return this->first; }

    /**
     * @brief const access to the underlying object, don't worry about
     * null-state
     * @return a constant reference on the underlying object
     */
    const_reference get_data() const { return this->first; }

    /**
     * @brief set the underlying object to an initialized value, removing the
     * null-state by the way
     * @param data data to initialize the underlying object with
     */
    void set_data( const_reference data )
    { this->second = true, this->first = data; }
};

/**
 * @brief Disable the NullableOf type for references
 *
 * @tparam T reference that is (or not) cv-qualified
 */
template< class T >
    class NullableOf< T & >;

/**
 * @brief Disable the NullableOf type for pointers. It exists native way to
 * check the nullity of pointers dude.
 *
 * @tparam T pointer that is (or not) cv-qualified
 */
template< class T >
    class NullableOf< T * >;

} // namespace Gex
} // namespace GS

#endif // _NULLABLEOF_H_