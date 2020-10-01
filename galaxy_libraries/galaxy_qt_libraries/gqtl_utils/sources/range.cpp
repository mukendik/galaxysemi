#include <stdio.h>
#include <gqtl_utils.h>

namespace GS
{
namespace QtLib
{
//todo : move to sub namespace QtLib

///////////////////////////////////////////////////////////
// Class that buils a list of ranges from a string holding
// the list in ASCII form.
///////////////////////////////////////////////////////////
Range::Range() : m_pRangeList(NULL)
{
}

Range::Range(const char * szRangeList) : m_pRangeList(NULL)
{
    if (szRangeList)
        SetRange(szRangeList);
    else
        SetRange("*");
}

Range::Range(const QString& list) : m_pRangeList(NULL)
{
    SetRange(list);
}

Range::Range(const Range& other) : m_pRangeList(NULL)
{
    *this = other;
}

void Range::Clear()
{
    freeMemory();

    m_strRangeList.clear();
}

bool Range::IsEmpty() const
{
    return (m_pRangeList == NULL);
}

bool Range::Contains(unsigned long lValue) const
{
    S_gexRange *pRangeCell = m_pRangeList;

    // Buils each range cell
    while(pRangeCell != NULL)
    {
        if((pRangeCell->lFrom <= lValue) && (lValue <= pRangeCell->lTo))
            return true;	// Found it!

        // Move to next range description
        pRangeCell = pRangeCell->pNextRange;
    };

    // Didn't find value in list
    return false;
}

void Range::SetRange(const QString& list)
{
    S_gexRange *	pRangeCell			= NULL;
    S_gexRange *	pPrevRangeCell		= NULL;
    QString			strList;

    freeMemory();	// delete pRangList; pRangList = NULL;

    if(list.isEmpty() || list == "*") // lets add * support for 6619
    {
        // If no range specified or *, take it all!
        pRangeCell = new S_gexRange;
        pRangeCell->pNextRange = NULL;
        pRangeCell->lFrom = 0;
        pRangeCell->lTo   = quint32(0xFFFFFFFF);//2147483647// highest 31bits value.//4294967295

        strList.sprintf("%lu %lu", pRangeCell->lFrom, pRangeCell->lTo);
        m_pRangeList = pRangeCell;
        return;
    }

    QString strRange;
    unsigned long	lFrom,lTo,lSwap;
    int	iParameters;
    // Parse string to extract each range set.
    strList = m_strRangeList = list;
    strList += ",";	// Ensure it ends with a delimiter!
    strList = strList.replace( ";", "," ); // ensure only one type of delimiter!
    strList = strList.replace( "to", " " ); // remove 'to' strings
    strList = strList.replace( "-", " " ); // remove '-' strings
    strList = strList.replace( "..", " " ); // remove '..' strings


    int i=0;
    while(1)
    {
        // Extract one section at a time....
        strRange = strList.section(',',i,i);
        if(strRange.isEmpty())
            break;	// No more data to process!
        // Check if we have a range or a single value
        iParameters = sscanf(strRange.toLatin1().constData(),"%lu %lu",
                             &lFrom,
                             &lTo);
        if(iParameters >= 1)
        {
            // This entry is valid...add it to our list.
            pRangeCell = new S_gexRange;
            pRangeCell->pNextRange = NULL;

            // Only one value....so lTo = lFrom!
            if(iParameters == 1)
                pRangeCell->lTo = lFrom;
            else
            {
                if(lFrom > lTo)
                {
                    // Swap values if they are in wrong order!
                    lSwap = lFrom;
                    lFrom = lTo;
                    lTo = lSwap;
                }
                pRangeCell->lTo   = lTo;
            }
            pRangeCell->lFrom = lFrom;

            // Add structure to the list
            if(pPrevRangeCell == NULL)
            {
                pPrevRangeCell = m_pRangeList = pRangeCell;
            }
            else
            {
                pPrevRangeCell->pNextRange = pRangeCell;
                pPrevRangeCell = pRangeCell;
            }
        }

        // Move to next section
        i++;
    };
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
Range::~Range()
{
    freeMemory();
}

Range& Range::operator=(const Range& other)
{
    if (this != &other)
    {
        m_strRangeList	= other.m_strRangeList;

        copyMemory(other.m_pRangeList);
    }

    return *this;
}

void Range::copyMemory(const S_gexRange * pRangeCopy)
{
    // free memory previously allocated
    freeMemory();

    //
    S_gexRange * pRangeList			= NULL;
    S_gexRange * pPrevRangeList		= NULL;
    S_gexRange * pNextRangeCopy		= NULL;

    while(pRangeCopy != NULL)
    {
        // Get pointer to next cell
        pNextRangeCopy = pRangeCopy->pNextRange;

        // Allocate new pointer in memory
        pRangeList				= new S_gexRange;
        pRangeList->lFrom		= pRangeCopy->lFrom;
        pRangeList->lTo			= pRangeCopy->lTo;
        pRangeList->pNextRange	= NULL;

        if (pPrevRangeList == NULL)
        {
            pPrevRangeList = m_pRangeList = pRangeList;
        }
        else
        {
            pPrevRangeList->pNextRange = pRangeList;
            pPrevRangeList = pRangeList;
        }

        // Move to next cell
        pRangeCopy = pNextRangeCopy;
    };
}

void Range::freeMemory()
{
    S_gexRange *pRangeCell,*pNextRangeCell;

    pRangeCell = m_pRangeList;
    while(pRangeCell != NULL)
    {
        // Get pointer to next cell
        pNextRangeCell = pRangeCell->pNextRange;

        // Destroy current cell
        delete pRangeCell;

        // Move to next cell
        pRangeCell = pNextRangeCell;
    };

    m_pRangeList = NULL;
}


///////////////////////////////////////////////////////////
// Return the range list selected for this instance
///////////////////////////////////////////////////////////
QString Range::GetRangeList(char cDelimiter/*=','*/) const
{
    // Range string...delimiter is ','
    QString strString = m_strRangeList;

    // Replace delimiter with user delimiter (eg ';' for CSV file so to avoid conflicting delimiters)
    strString = strString.replace(',',cDelimiter);

    return strString;
}

///////////////////////////////////////////////////////////
// Class that buils a list of ranges from a string holding
// the list in ASCII form.
///////////////////////////////////////////////////////////
QString Range::BuildListString(const char *szText,char cDelimiter/*=','*/)
{
    //static QString	strString;
    QString	strString;
    int				iRangeIndex	= 0;
    S_gexRange *	pRangeCell	= m_pRangeList;

    strString = szText;

    // Buils each range cell
    while(pRangeCell!=NULL)
    {
        // Blocs Separator
        if(iRangeIndex)
            strString += cDelimiter;

        if(pRangeCell->lFrom != pRangeCell->lTo)
            strString += QString::number(pRangeCell->lFrom) + "-" + QString::number(pRangeCell->lTo);
        else
            strString += QString::number(pRangeCell->lFrom);

        // Move to next range description
        pRangeCell = pRangeCell->pNextRange;

        // Keep track of the number of ranges specified
        iRangeIndex++;
    };

    // Return String
    //return (char *)strString.toLatin1().data();
    return strString;
}

} // gqtlib
} // GS
