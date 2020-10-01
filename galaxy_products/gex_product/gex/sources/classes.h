#ifndef GEX_CLASSES_H
#define GEX_CLASSES_H
#include <stdio.h>
#include <QString>
#include <QColor>

// Galaxy includes
#include <gqtl_utils.h>

// forward definition
class CTest;

///////////////////////////////////////////////////////////
// Structure used to define a range of values <from> <to>
///////////////////////////////////////////////////////////
struct sgexTestRange
{
    unsigned long			lFromTest;
    long					lFromPinmap;
    unsigned long			lToTest;
    long					lToPinmap;
    struct sgexTestRange *	pNextTestRange;
};
typedef  struct sgexTestRange S_gexTestRange;

///////////////////////////////////////////////////////////
// Class that buils a list of Test ranges from a string holding
// the list in ASCII form.
///////////////////////////////////////////////////////////
class	CGexTestRange
{
public:

    CGexTestRange(const CTest * pTestCell);
    CGexTestRange(const char * szTestRangeList = NULL);
    CGexTestRange(const CGexTestRange& other);
    ~CGexTestRange();

    QString					GetRangeList(void) const			{ return m_strTestRangeList; }

    void					setRange(QString m_strTestRangeList);

    QString                 BuildTestListString(const char * szText);
    bool					IsTestInList(unsigned long lTestValue, long lPinmap) const;

    // Operator
    CGexTestRange&			operator=(const CGexTestRange& other);

    S_gexTestRange*			pTestRangeList;

private:

    void					copyMemory(const S_gexTestRange * pTestRangeCopy);
    void					freeMemory();

    QString					m_strTestRangeList;
};

///////////////////////////////////////////////////////////
// Class to holds the min and max coordinate
///////////////////////////////////////////////////////////
class CGexRangeCoord
{
public:

    enum Validity
    {
        ValidCoordinates = 0,
        InvalidXCoordinate,
        InvalidYCoordinate,
        InvalidStringCoordinate
    };

    CGexRangeCoord(const QString& strRangeCoordinate = QString());
    CGexRangeCoord(int nXMin, int nXMax, int nYMin, int nYMax);
    virtual ~CGexRangeCoord();

    int								coordXMin() const			{ return m_nXMin; }
    int								coordXMax() const			{ return m_nXMax; }
    int								coordYMin() const			{ return m_nYMin; }
    int								coordYMax() const			{ return m_nYMax; }
    Validity						validity() const			{ return m_eValidity; }

    bool							isValid() const				{ return m_eValidity == ValidCoordinates; }

    QString							toString() const;												// Returns a formatted string containing coordinates

    void							fromString(const QString& strCoordinates);						// Parse a formatted string containing coordinates
    void							fromCoordinates(int nXMin, int nXMax, int nYMin, int nYMax);	// Init range of coordinates

    CGexRangeCoord&					operator=(const CGexRangeCoord& rangeCoordinates);

    static	QString					toString(int nXMin, int nXMax, int nYMin, int nYMax);			// Returns a formatted string containing coordinates

protected:

    void							updateValidity();												// Update the validity status

    int			m_nXMin;
    int			m_nXMax;
    int			m_nYMin;
    int			m_nYMax;
    Validity	m_eValidity;
};

///////////////////////////////////////////////////////////
// Base class to filter a part according its coordinates
///////////////////////////////////////////////////////////
class CGexFilterRangeCoord
{
public:

    CGexFilterRangeCoord(const QString& strRangeCoordinate);
    virtual ~CGexFilterRangeCoord();

    virtual	bool					isFilteredCoordinate(int nXCoord, int nYCoord) = 0;
    virtual QString					buildProcessString() = 0;

    static	CGexFilterRangeCoord *	createFilterRangeCoord(int nFilterType, const QString& strRangeCoordinate);

protected:

    CGexRangeCoord					m_rangeCoordinate;
};

///////////////////////////////////////////////////////////
// Class to holds the min and max coordinate inside which
// a die is filtered
///////////////////////////////////////////////////////////
class CGexFilterInsideRangeCoord : public CGexFilterRangeCoord
{
public:

    CGexFilterInsideRangeCoord(const QString& strRangeCoordinate);
    ~CGexFilterInsideRangeCoord();

    bool	isFilteredCoordinate(int nXCoord, int nYCoord);		// Check if coordinate are inside the range
    QString	buildProcessString();
};

///////////////////////////////////////////////////////////
// Class to holds the min and max coordinate outside which
// a die is filtered
///////////////////////////////////////////////////////////
class CGexFilterOutsideRangeCoord : public CGexFilterRangeCoord
{
public:

    CGexFilterOutsideRangeCoord(const QString& strRangeCoordinate);
    ~CGexFilterOutsideRangeCoord();

    bool	isFilteredCoordinate(int nXCoord, int nYCoord);		// Check if coordinate are outside the range
    QString	buildProcessString();
};

///////////////////////////////////////////////////////////
// This class holds The color assigned to each Bin.
///////////////////////////////////////////////////////////
class	CBinColor
{
public:
    CBinColor();
    CBinColor(const CBinColor& other);
    ~CBinColor();

    CBinColor& operator=(const CBinColor& other);

    GS::QtLib::Range *	cBinRange;		// List of Bins
    QColor		cBinColor;		// Color assigned to the list of bins.

};

#endif
