#ifndef TEST_RESULT_ITEM_H
#define TEST_RESULT_ITEM_H

///////////////////////////////////////////////////////////
// System include
///////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#endif

///////////////////////////////////////////////////////////
// QT includes
///////////////////////////////////////////////////////////
#include <QtGlobal>

#if defined unix || __MACH__
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
#endif



namespace GEX {
    enum ResultStatus
    {
        initialized = 0x01,
        valid		= 0x02,
        forced		= 0x04,
        passed		= 0x08,
        failed		= 0x10,
        deleted     = 0x20
    };
    Q_DECLARE_FLAGS(Status, ResultStatus)
}

class CTest;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CTestMultiResultItem
//
// Description	:	Class holding results for a sample
//
///////////////////////////////////////////////////////////////////////////////////
class CTestMultiResultItem
{
public:

    enum MultiResultValue
    {
        firstMultiResult = 0,
        lastMultiResult,
        minMultiResult,
        maxMultiResult,
        meanMultiResult,
        medianMultiResult
    };

    CTestMultiResultItem(CTest* parent, WORD wSize = 2);
    CTestMultiResultItem(const CTestMultiResultItem& other, CTest* parent);

    ~CTestMultiResultItem();

    int                     count() const;
    const double&           multiResultAt(WORD wIndex) const;

    void                    pushResult(const double& dValue);

    double                  processMultiResult(MultiResultValue eValue);
    void                    rescale(int nOldScale, int nNewScale);
    void                    clone(const CTestMultiResultItem& other, CTest* parent);

private:
    Q_DISABLE_COPY(CTestMultiResultItem)

    WORD			m_wSize;			// Max size allocated
    WORD			m_wValueCount;		// Count of value inserted in the result array
    double *		m_pValue;			// Array of value for this run
    BYTE*			m_pFlag;            // Holds flags values for all runs
    CTest*          mParent;            // Holds ptr to CTest parent

    double			minValue();			// return the min value of the array
    double			maxValue();			// return the max value of the array
    double			meanValue();		// return the mean value of the array
    double			medianValue();		// return the median value of the array
public:
    void invalidateResultAt(WORD wIndex);
    void validateResultAt(WORD wIndex);
    bool isValidResultAt(WORD wIndex) const;
};

#endif // TEST_RESULT_ITEM_H
