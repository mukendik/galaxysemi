#ifndef GEX_TEST_FILTER_PRIVATE_H
#define GEX_TEST_FILTER_PRIVATE_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "classes.h"
#include "gextestfilter.h"

namespace GS
{
    namespace Gex
    {
    ///////////////////////////////////////////////////////////////////////////////////
    //
    // Name			:	TestFilterPrivate
    //
    // Description	:	Class holding the rule to filter a test list
    //
    ///////////////////////////////////////////////////////////////////////////////////
    class TestFilterPrivate
    {

    public:

        TestFilterPrivate(const QString& strTestRange, TestFilter::filterType eType);
        virtual ~TestFilterPrivate();

        const CGexTestRange&			testRange() const			{ return m_testRange; }
        TestFilter::filterType          type() const				{ return m_eType; }

        virtual bool					contains(int nTestNumber) const = 0;

        static TestFilterPrivate *		create(const QString& strTestRange,
                                               TestFilter::filterType eFilter);

    protected:

        CGexTestRange					m_testRange;
        TestFilter::filterType			m_eType;
    };

    ///////////////////////////////////////////////////////////////////////////////////
    //
    // Name			:	TestFilterIgnorePrivate
    //
    // Description	:	Class holding the rule to filter a test list
    //
    ///////////////////////////////////////////////////////////////////////////////////
    class TestFilterIgnorePrivate : public TestFilterPrivate
    {

    public:

        TestFilterIgnorePrivate(const QString& strTestRange);
        ~TestFilterIgnorePrivate();

        bool	contains(int nTestNumber) const;
    };

    ///////////////////////////////////////////////////////////////////////////////////
    //
    // Name			:	TestFilterSelectPrivate
    //
    // Description	:	Class holding the rule to filter a test list
    //
    ///////////////////////////////////////////////////////////////////////////////////
    class TestFilterSelectPrivate : public TestFilterPrivate
    {

    public:

        TestFilterSelectPrivate(const QString& strTestRange);
        ~TestFilterSelectPrivate();

        bool	contains(int nTestNumber) const;
    };

    } // end namespace Gex
} // end namespace GS

#endif // GEX_TEST_FILTER_PRIVATE_H
