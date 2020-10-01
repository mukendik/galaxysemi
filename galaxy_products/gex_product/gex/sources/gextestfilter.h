#ifndef _GEX_TEST_FILTER_H_
#define _GEX_TEST_FILTER_H_

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QString>

namespace GS
{
    namespace Gex
    {

    class TestFilterPrivate;

    ///////////////////////////////////////////////////////////////////////////////////
    //
    // Name			:	TestFilter
    //
    // Description	:	Class holding the filtering for tests
    //
    ///////////////////////////////////////////////////////////////////////////////////
    class TestFilter
    {

    public:

        enum filterType
        {
            filterUndefined = -1,
            filterIgnore    = 0,
            filterSelect
        };

        TestFilter();
        ~TestFilter();

        QString                     GetRangeList() const;
        TestFilter::filterType 		GetType() const;

        bool	isFiltered(int nTestNumber) const;
        bool	isActivated() const;

        void	addFilter(const QString& strTestRange, filterType eType);

    private:

        TestFilterPrivate *	m_pTestFilterPrivate;
    };
    } // namespace Gex
} // namespace GS

#endif // _GEX_TEST_FILTER_H_
