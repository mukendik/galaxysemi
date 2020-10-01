#ifndef WINDOWSZLEVELCOMPARE_H
#define WINDOWSZLEVELCOMPARE_H

class QDetachableTabWindow;

/**
 * \brief This is a compare object designed to be used in some standard
 * algorithms. Respects the Compare concept.
 */
struct WindowsZLevelCompare
{
    /**
     * \brief Prerequisite of the Compare concept, establishes a strict weak
     * ordering between parameters
     *
     * \param a the first operand of the comparison
     * \param b the second operand of the comparison
     *
     * \return true iff a < b, false otherwise
     */
    bool comp
        ( QDetachableTabWindow *a, QDetachableTabWindow *b );

    /**
     * \brief Prerequisite of the Compare concept, establishes an equivalence
     * between 2 operands
     *
     * \param a the first operand of the comparison
     * \param b the second operand of the comparison
     *
     * \return true iff a == b, false otherwise
     */
    bool equiv
        ( QDetachableTabWindow *a, QDetachableTabWindow *b );

    /**
     * \brief callable operator using comp method
     *
     * \param a the first operand of the comparison
     * \param b the second operand of the comparison
     *
     * \return true iff a < b, false otherwise
     */
    bool operator()
        ( QDetachableTabWindow *a, QDetachableTabWindow *b );
};

#endif // WINDOWSZLEVELCOMPARE_H
