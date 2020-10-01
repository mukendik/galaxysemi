///////////////////////////////////////////////////////////
// GEX QT utilities classes: header file
///////////////////////////////////////////////////////////

#ifndef GQTL_UTILS_H
#define GQTL_UTILS_H

// QT headers
#include <QObject>
#include <QString>

///////////////////////////////////////////////////////////
// Structure used to define a range of values <from> <to>
///////////////////////////////////////////////////////////
struct sgexRange
{
    unsigned long		lFrom;
    unsigned long		lTo;
    struct sgexRange *	pNextRange;
};
typedef  struct sgexRange S_gexRange;

///////////////////////////////////////////////////////////
// Class that buils a list of ranges from a string holding
// the list in ASCII form.
///////////////////////////////////////////////////////////
namespace GS
{
    namespace QtLib
    {
        //! \brief generate a JSON representation of this QObject into output string.
        //! \param lIncludeChildren set if it must go into all children and subchildren of this QObject or not.
        //! \param lIndent is just an indentation tab string for nice forlrmating if desired: default is 2 spaces per hierarchy
        //! \return "ok" or "error"
        QString QObjectToJSON(QObject* lQObject, QString &lOutput, bool lIncludeChildren, QString &lIndent);

        class	Range
        {
        public:

            /*!
              @brief    Constructs a empty range object
              */
            Range();

            /*!
              @brief    Constructs a valid integer range object
              */
            Range(const char * szRangeList);

            /*!
              @brief    Constructs a valid integer range object
              */
            Range(const QString& list);

            /*!
              @brief    Copy constructor
              */
            Range(const Range& other);

            /*!
              @brief    Destructor
              */
            ~Range();

            /*!
              @brief    Clears the contents of the range and makes it empty.

              @sa       IsEmpty
              */
            void                Clear();

            /*!
              @brief    Returns whether the range is empty

              @return   True if the range has no bound, otherwise returns false
              */
            bool                IsEmpty() const;

            /*!
              @brief    Returns whether this range contains the given value

              @param    lValue  The value to look for in the range

              @return   True if this range contains an occurence of the value, otherwise false.
              */
            bool                Contains(unsigned long lValue) const;

            /*!
              @brief    Sets a positive integer range
                        (empty) or "*" : all
                        "x" : 1 value
                        "x-y,z-a" : ranges
                        "x,y,z,a" : list of values
                        with all these numbers POSITIVE

              @param    list  A string containing the range description

              @return   True if this range contains an occurence of the value, otherwise false.
              */
            void				SetRange(const QString &list);

            QString				BuildListString(const char *szText,char cDelimiter=',');
            QString				GetRangeList(char cDelimiter=',') const;

            /*!
              @brief    Assignement operator
              */
            Range&              operator=(const Range& other);

        private:

            void				copyMemory(const S_gexRange * pRangeCopy);
            void				freeMemory();

            QString				m_strRangeList;
            S_gexRange *		m_pRangeList;
        };

        class NumberFormat
        {

        public:
            NumberFormat();
            ~NumberFormat();
            QString formatNumericValue(int, bool bFormat);
            QString formatNumericValue(uint, bool bFormat);
            QString formatNumericValue(long, bool bFormat);
            QString formatNumericValue(ulong, bool bFormat);
            QString formatNumericValue(qlonglong, bool bFormat);
            QString formatNumericValue(qulonglong, bool bFormat);
            // ?
            QString formatNumericValue(double, bool bFormat, const QString &strDefault="" );
            void    setOptions(char Format='g', int Precision=-1, bool ComplexFormat=true);
        protected:
            bool    mComplexFormat; // Set to true if we should use the complex legacy formatting codes
            char    mFormat;        // 'e', 'f', or 'g'
            int     mPrecision;     // -1 will use max precision
        public:
            static QString displaySigFigs(const QString &strFloat, int iSigFigs, int iSigDecs, bool bScientific);
        private:
            static int parseOrder(const QString &strString);
            static QString parseMantissa(const QString &strString);
            static bool parseSign(const QString &strString);
            static QString round(const QString &strMantissa, int iDigits);
            static int trailingZeros(const QString &strMantissa);
        };
    }
}

#endif // !defined(GQTL_UTILS_H)
