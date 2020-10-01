#ifndef CBINNING_H
#define CBINNING_H

#include <QString>

#include "gstdl_type.h"

//////////////////////////////////////////////////////
// Binning summary cell
//////////////////////////////////////////////////////
class	CBinning
{
public:

    CBinning();

    void        Append(int lBin, BYTE lStatus, int lCount, const QString& lName);
    void        Insert(int lBin, BYTE lStatus, int lCount, const QString& lName);
    CBinning *  Clone() const;
    CBinning *  Find(int binNumber);
    //! \brief Cont the number of CBinning in this list if any.
    unsigned    CountBins();

    int         iBinValue;			// Software Binning Value
    BYTE        cPassFail;			// 'P' or 'F' or ' ' : Pass/Fail info
    int         ldTotalCount;		// Total parts in this bin.
    QString     strBinName;			// Binning string name.
    CBinning *  ptNextBin;          // Pointer to next CBinning structure

private:
    CBinning(CBinning const &);             // forbid copy constructor
    CBinning& operator=(CBinning const &);  // forbid = operator
};


#endif // CBINNING_H
