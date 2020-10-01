// QT includes
#include <qcombobox.h>

#include "ctest.h"
#include "classes.h"
#include "gex_constants.h"

// in patman_lib.cpp
extern int ExtractTestRange(const char *szValue,unsigned long *lFromTest,long *lFromPinmapIndex,unsigned long *lToTest,long *lToPinmapIndex);

// Set current item in combo box on item havin specified text
bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false)
{
    int nItem;

    QString strComboItem;
    for(nItem=0; nItem<pCombo->count(); nItem++)
    {
        strComboItem = pCombo->itemText(nItem);
        if(strComboItem == strItem)
        {
            pCombo->setCurrentIndex(nItem);
            pCombo->setEditText(strItem);
            return true;
        }
    }

    if(!bInsertIfMissing)
        return false;

    // Insert/select item
    pCombo->insertItem(pCombo->count(), strItem);
    for(nItem=0; nItem<pCombo->count(); nItem++)
    {
        if(pCombo->itemText(nItem) == strItem)
        {
            pCombo->setCurrentIndex(nItem);
            pCombo->setEditText(strItem);
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////
// Class that buils a list of Test ranges from a string holding
// the list in ASCII form.
///////////////////////////////////////////////////////////
CGexTestRange::CGexTestRange(const char * szTestRangeList)
    : pTestRangeList(NULL)
{
    QString strRange;

    if(szTestRangeList)
        strRange = szTestRangeList;

    setRange(strRange);
}

CGexTestRange::CGexTestRange(const CTest *pTestCell)
{
    if (pTestCell)
    {
        // This entry is valid...add it to our list.
        pTestRangeList = new S_gexTestRange;
        pTestRangeList->pNextTestRange	= NULL;

        // Values are always sorted in the function 'ExtractTestRange'
        pTestRangeList->lFromTest		= pTestCell->lTestNumber;
        pTestRangeList->lFromPinmap		= pTestCell->lPinmapIndex;
        pTestRangeList->lToTest			= pTestCell->lTestNumber;
        pTestRangeList->lToPinmap		= pTestCell->lPinmapIndex;
    }
    else
        setRange("");
}

CGexTestRange::CGexTestRange(const CGexTestRange& other) : pTestRangeList(NULL)
{
    *this = other;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexTestRange::~CGexTestRange()
{
    freeMemory();
}

CGexTestRange& CGexTestRange::operator=(const CGexTestRange& other)
{
    if (this != &other)
    {
        m_strTestRangeList	= other.m_strTestRangeList;

        copyMemory(other.pTestRangeList);
    }

    return *this;
}

void CGexTestRange::copyMemory(const S_gexTestRange * pTestRangeCopy)
{
    // free memory previously allocated
    freeMemory();

    // Init local pointers
    S_gexTestRange * pTestRange				= NULL;
    S_gexTestRange * pPrevTestRange			= NULL;
    S_gexTestRange * pNextTestRangeCopy		= NULL;

    while(pTestRangeCopy != NULL)
    {
        // Get pointer to next cell
        pNextTestRangeCopy = pTestRangeCopy->pNextTestRange;

        // Allocate new pointer in memory
        pTestRange					= new S_gexTestRange;
        pTestRange->lFromTest		= pTestRangeCopy->lFromTest;
        pTestRange->lToTest			= pTestRangeCopy->lToTest;
        pTestRange->lFromPinmap		= pTestRangeCopy->lFromPinmap;
        pTestRange->lToPinmap		= pTestRangeCopy->lToPinmap;
        pTestRange->pNextTestRange	= NULL;

        if (pPrevTestRange == NULL)
        {
            pPrevTestRange = pTestRangeList = pTestRange;
        }
        else
        {
            pPrevTestRange->pNextTestRange = pTestRange;
            pPrevTestRange = pTestRange;
        }

        // Move to next cell
        pTestRangeCopy = pNextTestRangeCopy;
    };
}

void CGexTestRange::freeMemory()
{
    S_gexTestRange *	pTestRangeCell		= pTestRangeList;
    S_gexTestRange *	pNextTestRangeCell	= NULL;

    while(pTestRangeCell != NULL)
    {
        // Get pointer to next cell
        pNextTestRangeCell = pTestRangeCell->pNextTestRange;

        // Destroy current cell
        delete pTestRangeCell;

        // Move to next cell
        pTestRangeCell = pNextTestRangeCell;
    };

    pTestRangeList = NULL;
}

void CGexTestRange::setRange(QString strTestRangeList)
{
    S_gexTestRange *	pTestRangeCell		= NULL;
    S_gexTestRange *	pPrevTestRangeCell	= NULL;

    // free memory already allocated
    freeMemory();

    if(strTestRangeList.isEmpty())
    {
        // If no range specified, take it all!
        try
        {
            pTestRangeCell = new S_gexTestRange;
        }
        catch(const std::bad_alloc &e)
        {
            //GSLOG(SYSLOG_SEV_ERROR, e.what() );
            printf("\nCGexTestRange::setRange: %s\n", e.what());
            return; // todo : return a int, string, enum,... but return a status error with root cause.
        }

        pTestRangeCell->pNextTestRange = NULL;
        pTestRangeCell->lFromTest = 0;
        pTestRangeCell->lFromPinmap = GEX_PTEST;

        pTestRangeCell->lToTest = 2147483647;	// highest 31bits value.
        pTestRangeCell->lToPinmap = GEX_PTEST;
        pTestRangeList = pTestRangeCell;
        return;
    }

    QString			strTestRange;
    unsigned long	lFromTest,lToTest;
    long			lFromPinmap,lToPinmap;
    int				iParameters;

    // Parse string to extract each range set.
    m_strTestRangeList = strTestRangeList;
    m_strTestRangeList = m_strTestRangeList.replace( ";", "," ); // ensure only one type of delimiter!
    m_strTestRangeList = m_strTestRangeList.replace( "to", " " ); // remove 'to' strings
    m_strTestRangeList = m_strTestRangeList.replace( "-", " " ); // remove '-' strings
    m_strTestRangeList = m_strTestRangeList.replace( "..", " " ); // remove '..' strings

    int i=0;
    while(1)
    {
        // Extract one section at a time....
        strTestRange = m_strTestRangeList.section(',',i,i);
        if(strTestRange.isEmpty())
            break;	// No more data to process!

        // Extract test.pinmap
        iParameters = ExtractTestRange(strTestRange.toLatin1().constData(),&lFromTest,&lFromPinmap,&lToTest,&lToPinmap);
        if(iParameters >= 1)
        {
            // This entry is valid...add it to our list.
            pTestRangeCell = new S_gexTestRange;
            pTestRangeCell->pNextTestRange = NULL;

            // Values are always sorted in the function 'ExtractTestRange'
            pTestRangeCell->lFromTest = lFromTest;
            pTestRangeCell->lFromPinmap = lFromPinmap;
            pTestRangeCell->lToTest = lToTest;
            pTestRangeCell->lToPinmap = lToPinmap;

            // Add structure to the list
            if(pPrevTestRangeCell == NULL)
            {
                pPrevTestRangeCell = pTestRangeList = pTestRangeCell;
            }
            else
            {
                pPrevTestRangeCell->pNextTestRange = pTestRangeCell;
                pPrevTestRangeCell = pTestRangeCell;
            }
        }

        // Move to next section
        i++;
    };
}

///////////////////////////////////////////////////////////
// Class that builds a list of Test ranges from a string holding
// the list in ASCII form.
///////////////////////////////////////////////////////////
QString CGexTestRange::BuildTestListString(const char *szText)
{
    QString				strString;
    char				szRange[128];
    char				szRangeTo[128];
    int					iTestRangeIndex=0;
    S_gexTestRange		*pTestRangeCell=pTestRangeList;

    strString = szText;
    // Buils each range cell
    while(pTestRangeCell!=NULL)
    {
        if(pTestRangeCell->lFromTest == pTestRangeCell->lToTest)
        {
            // Same test#...check if same pinmap#!
            if(pTestRangeCell->lFromPinmap == pTestRangeCell->lToPinmap)
            {
                // Check if pinmap is GEX_PTEST
                if(pTestRangeCell->lFromPinmap == GEX_PTEST)
                    sprintf(szRange,"%lu",pTestRangeCell->lFromTest);
                else
                    sprintf(szRange,"%lu.%ld",pTestRangeCell->lFromTest,pTestRangeCell->lFromPinmap);
            }
            else
            {
                // Same test but different pinmap#...

                // Check if 'From' pinmap is GEX_PTEST
                if(pTestRangeCell->lFromPinmap == GEX_PTEST)
                    sprintf(szRange,"%lu",pTestRangeCell->lFromTest);
                else
                    sprintf(szRange,"%lu.%ld",pTestRangeCell->lFromTest,pTestRangeCell->lFromPinmap);

                // Check if 'To' pinmap is GEX_PTEST
                if(pTestRangeCell->lToPinmap == GEX_PTEST)
                    sprintf(szRangeTo,"-%lu",pTestRangeCell->lToTest);
                else
                    sprintf(szRangeTo,"-%lu.%ld",pTestRangeCell->lToTest,pTestRangeCell->lToPinmap);

                strcat(szRange,szRangeTo);
            }
        }
        else
        {
            // Different test#...
            // Check if pinmap is GEX_PTEST
            if(pTestRangeCell->lFromPinmap == GEX_PTEST)
                sprintf(szRange,"%lu",pTestRangeCell->lFromTest);
            else
                sprintf(szRange,"%lu.%ld",pTestRangeCell->lFromTest,pTestRangeCell->lFromPinmap);


            // Check if 'From' pinmap is GEX_PTEST
            if(pTestRangeCell->lFromPinmap == GEX_PTEST)
                sprintf(szRangeTo,"-%lu",pTestRangeCell->lToTest);
            else
                sprintf(szRangeTo,"-%lu.%ld",pTestRangeCell->lToTest,pTestRangeCell->lToPinmap);

            strcat(szRange,szRangeTo);
        }

        // Blocs Separator
        if(iTestRangeIndex)
            strString += ";";

        // Update string built with each range found
        strString += szRange;

        // Move to next range description
        pTestRangeCell = pTestRangeCell->pNextTestRange;

        // Keep track of the number of Test ranges specified
        iTestRangeIndex++;
    };

    // Return string
    return strString;
}

///////////////////////////////////////////////////////////
// Tells if given Test value/pinmap belongs to the list...
///////////////////////////////////////////////////////////
bool CGexTestRange::IsTestInList(unsigned long lTestValue, long lPinmap) const
{
    S_gexTestRange *pTestRangeCell=pTestRangeList;

    // Buils each Test range cell
    while(pTestRangeCell!=NULL)
    {
        // Test# lower than LowWindow value
        if(lTestValue < pTestRangeCell->lFromTest)
            goto check_next_range;

        // Test# higher than HighWindow value
        if(lTestValue > pTestRangeCell->lToTest)
            goto check_next_range;

        // Test# is in window range...but what about Pinmap#...
        if(lPinmap == GEX_PTEST || lPinmap == GEX_FTEST ||
           lPinmap == GEX_UNKNOWNTEST || lPinmap == GEX_INVALIDTEST)
        {
            // Standard test, no pinmap...see if range window is that whide...
            if((pTestRangeCell->lFromPinmap == GEX_PTEST) || (pTestRangeCell->lToPinmap == GEX_PTEST))
                return true;	// Yes found test!
        }

        // Test has a pinmap#...check if higher or equal to low window test#.pinmap
        if((lTestValue == pTestRangeCell->lFromTest) && (lPinmap < pTestRangeCell->lFromPinmap))
            goto check_next_range;

        // High window has no pinmap#...
        if(pTestRangeCell->lToPinmap == GEX_PTEST)
            return true;	// Yes found test!

        // Test# = high window...but pinmap# is too high.
        if((lTestValue == pTestRangeCell->lToTest) && (lPinmap > pTestRangeCell->lToPinmap))
            goto check_next_range;

        return true;	// Yes found test!

check_next_range:
        // Move to next range description
        pTestRangeCell = pTestRangeCell->pNextTestRange;
    };

    // didn't find test/pinmap in list
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexRangeCoord
//
// Description	:	Class to holds the min and max coordinate
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexRangeCoord::CGexRangeCoord(const QString& strRangeCoordinate /* = QString() */)
{
    m_nXMin = -32768;
    m_nYMin = -32768;
    m_nXMax = 32768;
    m_nYMax = 32768;

    fromString(strRangeCoordinate);
}

CGexRangeCoord::CGexRangeCoord(int nXMin, int nXMax, int nYMin, int nYMax)
{
    fromCoordinates(nXMin, nXMax, nYMin, nYMax);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexRangeCoord::~CGexRangeCoord()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexRangeCoord&	operator=(const CGexRangeCoord& rangeCoordinates)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
CGexRangeCoord&	CGexRangeCoord::operator=(const CGexRangeCoord& rangeCoordinates)
{
    if (this != &rangeCoordinates)
    {
        m_nXMin		= rangeCoordinates.m_nXMin;
        m_nXMax		= rangeCoordinates.m_nXMax;
        m_nYMin		= rangeCoordinates.m_nYMin;
        m_nYMax		= rangeCoordinates.m_nYMax;
        m_eValidity	= rangeCoordinates.m_eValidity;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString toString() const
//
// Description	:	Returns a formatted string containing coordinates selected
//
///////////////////////////////////////////////////////////////////////////////////
QString CGexRangeCoord::toString() const
{
    QString strRangeCoord;

    if (isValid())
        strRangeCoord = strRangeCoord.sprintf("(%d;%d);(%d;%d)", m_nXMin, m_nYMin, m_nXMax, m_nYMax);

    return strRangeCoord;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString toString(int nXMin, int nXMax, int nYMin, int nYMax)
//
// Description	:	Returns a formatted string containing coordinates selected
//
///////////////////////////////////////////////////////////////////////////////////
QString CGexRangeCoord::toString(int nXMin, int nXMax, int nYMin, int nYMax)
{
    return CGexRangeCoord(nXMin, nXMax, nYMin, nYMax).toString();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fromCoordinates(int nXMin, int nXMax, int nYMin, int nYMax)
//
// Description	:	Init range of coordinates
//
///////////////////////////////////////////////////////////////////////////////////
void CGexRangeCoord::fromCoordinates(int nXMin, int nXMax, int nYMin, int nYMax)
{
    m_nXMin = nXMin;
    m_nYMin = nYMin;
    m_nXMax = nXMax;
    m_nYMax = nYMax;

    updateValidity();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void fromString(const QString& strCoordinates)
//
// Description	:	Parse a formatted string containing coordinates
//
///////////////////////////////////////////////////////////////////////////////////
void CGexRangeCoord::fromString(const QString& strCoordinates)
{
    QString		strRangeCoord	= strCoordinates;
    QString		strField;

    strRangeCoord.remove("(");
    strRangeCoord.remove(")");

    if (strRangeCoord.count(";") == 3)
    {
        m_nXMin = strRangeCoord.section(";", 0, 0).toInt();
        m_nYMin = strRangeCoord.section(";", 1, 1).toInt();
        m_nXMax = strRangeCoord.section(";", 2, 2).toInt();
        m_nYMax = strRangeCoord.section(";", 3, 3).toInt();

        updateValidity();
    }
    else
        m_eValidity = InvalidStringCoordinate;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void updateValidity()
//
// Description	:	Update the validity status
//
///////////////////////////////////////////////////////////////////////////////////
void CGexRangeCoord::updateValidity()
{
    if (m_nXMin >= m_nXMax)
        m_eValidity = InvalidXCoordinate;
    else if (m_nYMin >= m_nYMax)
        m_eValidity = InvalidYCoordinate;
    else
        m_eValidity = ValidCoordinates;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexFilterRangeCoord
//
// Description	:	Base class to filter a part according its coordinates
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFilterRangeCoord::CGexFilterRangeCoord(const QString& strRangeCoordinate) : m_rangeCoordinate(strRangeCoordinate)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFilterRangeCoord::~CGexFilterRangeCoord()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexFilterRangeCoord * createFilterRangeCoord(int nFilterType, const QString& strRangeCoordinate)
//
// Description	:	Create the object holding the coordiantes range.
//
///////////////////////////////////////////////////////////////////////////////////
CGexFilterRangeCoord * CGexFilterRangeCoord::createFilterRangeCoord(int nFilterType, const QString& strRangeCoordinate)
{
    CGexFilterRangeCoord * pRangeCoord = NULL;

    switch(nFilterType)
    {
        case GEX_PROCESSPART_PARTSINSIDE	:	pRangeCoord = new CGexFilterInsideRangeCoord(strRangeCoordinate);
                                                break;

        case GEX_PROCESSPART_PARTSOUTSIDE	:	pRangeCoord = new CGexFilterOutsideRangeCoord(strRangeCoordinate);
                                                break;

        default								:	break;
    }

    return pRangeCoord;
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexFilterInsideRangeCoord
//
// Description	:	Class to holds the min and max coordinate inside which
//					a die is filtered
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFilterInsideRangeCoord::CGexFilterInsideRangeCoord(const QString& strRangeCoordinate) : CGexFilterRangeCoord(strRangeCoordinate)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFilterInsideRangeCoord::~CGexFilterInsideRangeCoord()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isFilteredCoordinate(int nXCoord, int nYCoord)
//
// Description	:	Check if coordinate are inside the range
//
///////////////////////////////////////////////////////////////////////////////////
bool CGexFilterInsideRangeCoord::isFilteredCoordinate(int nXCoord, int nYCoord)
{
    if (m_rangeCoordinate.isValid())
    {
        if (nXCoord >= m_rangeCoordinate.coordXMin() && nXCoord <= m_rangeCoordinate.coordXMax() &&
            nYCoord >= m_rangeCoordinate.coordYMin() && nYCoord <= m_rangeCoordinate.coordYMax())
            return true;

        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString buildProcessString()
//
// Description	:	Returns a string containing the label of the process and the
//					min/max coordinates
//
///////////////////////////////////////////////////////////////////////////////////
QString CGexFilterInsideRangeCoord::buildProcessString()
{
    QString strProcess = ProcessPartsItems[GEX_PROCESSPART_PARTSINSIDE] + m_rangeCoordinate.toString();

    return strProcess;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexFilterOutsideRangeCoord
//
// Description	:	Class to holds the min and max coordinate outside which
//					a die is filtered
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexFilterOutsideRangeCoord::CGexFilterOutsideRangeCoord(const QString& strRangeCoordinate) : CGexFilterRangeCoord(strRangeCoordinate)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexFilterOutsideRangeCoord::~CGexFilterOutsideRangeCoord()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool CGexPartRangeOutsideCoord(int nXCoord, int nYCoord)
//
// Description	:	Check if coordinate are outside the range
//
///////////////////////////////////////////////////////////////////////////////////
bool CGexFilterOutsideRangeCoord::isFilteredCoordinate(int nXCoord, int nYCoord)
{
    if (m_rangeCoordinate.isValid())
    {
        if (nXCoord < m_rangeCoordinate.coordXMin() || nXCoord > m_rangeCoordinate.coordXMax() ||
            nYCoord < m_rangeCoordinate.coordYMin() || nYCoord > m_rangeCoordinate.coordYMax())
            return true;

        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	QString buildProcessString()
//
// Description	:	Returns a string containing the label of the process and the
//					min/max coordinates
//
///////////////////////////////////////////////////////////////////////////////////
QString CGexFilterOutsideRangeCoord::buildProcessString()
{
    QString strProcess = ProcessPartsItems[GEX_PROCESSPART_PARTSOUTSIDE] + m_rangeCoordinate.toString();

    return strProcess;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CBinColor
//
// Description	:	Class to holds the color assigned to each Bin
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CBinColor::CBinColor() : cBinRange(NULL)
{
}

CBinColor::CBinColor(const CBinColor &other)
{
    cBinRange = NULL;

    *this = other;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CBinColor::~CBinColor()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CBinColor&	operator=(const CBinColor& other)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
CBinColor& CBinColor::operator =(const CBinColor& other)
{
    if (this != &other)
    {
        if (other.cBinRange)
            cBinRange = new GS::QtLib::Range(*(other.cBinRange));

        cBinColor = other.cBinColor;
    }

    return *this;
}
