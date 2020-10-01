#ifndef MULTILIMITITEM_H
#define MULTILIMITITEM_H

#include "test_defines.h"
#include <QList>

#if defined(_WIN32) || defined(_WIN64)
    #include<windows.h>
#endif

#if defined unix || __MACH__
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
#endif

///////////////////////////////////////////////////////////////////////////////
// bLimitFlag details
// Mask
// 0x01 Bit 0 = 1 for No Low Test Limit
// 0x02 Bit 1 = 1 for No High Test Limit
// 0x04 Bit 2 = 1 for Limits already defined
// 0x08 Bit 3 = 1 for Low test limit is NOT strict
// 0x10 Bit 4 = 1 for High test limit is NOT strict
// 0x20 Bit 5 = 1 for No Low Spec Limit
// 0x40 Bit 6 = 1 for No High Spec Limit

// Masks
#define CTEST_LIMITFLG_NOLTL		0x01	// 0x01 Bit 0 = 1 for No Low Test Limit
#define CTEST_LIMITFLG_NOHTL		0x02	// 0x02 Bit 1 = 1 for No High Test Limit
#define CTEST_LIMITFLG_GOTTL		0x04	// 0x04 Bit 2 = 1 for Limits already defined
#define CTEST_LIMITFLG_LTLNOSTRICT	0x08	// 0x08 Bit 3 = 1 for Low test limit is NOT strict
#define CTEST_LIMITFLG_HTLNOSTRICT	0x10	// 0x10 Bit 4 = 1 for High test limit is NOT strict
#define CTEST_LIMITFLG_NOLSL		0x20	// 0x20 Bit 5 = 1 for No Low Spec Limit
#define CTEST_LIMITFLG_NOHSL		0x40	// 0x40 Bit 6 = 1 for No High Spec Limit

#define	INFINITE_PERCENTAGE	(double) 999	// Any value higher than 999% will show 999%

/*! \class MultiLimitItem
 * \brief store a multi limit item most of the time extracted from a DTR record,
 * provide method to extract data from JSon object of type multi limit
 *
 */

#include <QJsonObject>

namespace GS
{
namespace Core
{

struct MLShift
{
    MLShift()
    {
        mSigmaShift = mCpkShift = mCpShift = mCrShift = mMeanShiftPct = 0;
        mMeanShiftPct = mSigmaShiftPct = mCpkShiftPct = mCpShiftPct = mCrShiftPct = 0;
    }

    //double mMeanShift;
    double mSigmaShift;
    double mCrShift;
    double mCpShift;
    double mCpkShift;
    double mMeanShiftPct;
    double mSigmaShiftPct;
    double mCrShiftPct;
    double mCpShiftPct;
    double mCpkShiftPct;

    double GetSigmaShift() const
    { return mSigmaShift; }

    double GetCrShift() const
    { return mCrShift; }

    double GetCpShift() const
    { return mCpShift; }

    double GetCpkShift() const
    { return mCpkShift; }

    double GetMeanShiftPct() const
    { return mMeanShiftPct; }

    double GetSigmaShiftPct() const
    { return mSigmaShiftPct; }

    double GetCrShiftPct() const
    { return mCrShiftPct; }

    double GetCpShiftPct() const
    { return mCpShiftPct; }

    double GetCpkShiftPct() const
    { return mCpkShiftPct; }


    void SetSigmaShiftValue(double value)
    { mSigmaShift = value; }

    void SetCrShiftValue(double value)
    { mCrShift = CleanBigValues(value); }

    void SetCpShiftValue(double value)
    { mCpShift = CleanBigValues(value); }

    void SetCpkShiftValue(double value)
    { mCpkShift = CleanBigValues(value); }

    void SetMeanShiftPct(double value)
    { mMeanShiftPct = CleanBigValues(value); }

    void SetSigmaShiftPct(double value)
    { mSigmaShiftPct = CleanBigValues(value); }

    void SetCrShiftPct(double value)
    { mCrShiftPct = CleanBigValues(value); }

    void SetCpShiftPct(double value)
    { mCpShiftPct = CleanBigValues(value); }

    void SetCpkShiftPct(double value)
    { mCpkShiftPct = CleanBigValues(value); }

    double CleanBigValues(double value)
    {
        if(value > INFINITE_PERCENTAGE)
            return  INFINITE_PERCENTAGE;
        else if(value < -INFINITE_PERCENTAGE)
            return -INFINITE_PERCENTAGE;
        else
            return value;
    }
};

class MultiLimitItem
{
public:
    /// \brief Constructor
    MultiLimitItem();
    /// \brief Constructor by copy
    MultiLimitItem(const MultiLimitItem& other);
    /// \brief Destructor
    ~MultiLimitItem();

    static int sNbMaxLimits;
    static void UpdateNbMaxLimits(int size);

    /// \brief overload operator ==
    bool               operator==(const MultiLimitItem& other);
    /// \brief overload operator =
    MultiLimitItem&    operator=(const MultiLimitItem& other);

    /// \brief load item from JSon Object
    /// \return true if loaded succesfully
    bool LoadFromJSon(const QJsonObject& limitItem, QString &error);

    /// \brief Create a JSon Object from the multi limit item
    /// \return true if create succesfully
    bool CreateJsonFromMultiLimit(QJsonObject& limitItem, const int testNum);

    /// \brief return low limit
    double  GetLowLimit() const;
    /// \brief return high limit
    double  GetHighLimit() const;
    /// \brief return soft bin
    int     GetSoftBin() const;
    /// \brief retun hard bin
    int     GetHardBin() const;
    /// \brief return site
    int     GetSite() const;

    /// \brief set low limit
    void    SetLowLimit(double lowLimit);
    /// \brief set high limit
    void    SetHighLimit(double highLimit);
    /// \brief set soft bin
    void    SetSoftBin(int softBin);
    /// \brief set hard bin
    void    SetHardBin(int hardBin);
    /// \brief set site
    void    SetSite(int site);

    /// \brief clear all data from members
    void    Clear();

    /// \brief true if low limit is valid
    bool    IsValidLowLimit() const;
    /// \brief true if high limit is valid
    bool    IsValidHighLimit() const;
    /// \brief true if hard bin is valid
    bool    IsValidHardBin() const;
    /// \brief true if soft bin is valid
    bool    IsValidSoftBin() const;
    /// \brief true if site is valid
    bool    IsValidSite() const;

    /// \brief check if item is valid
    bool    IsValid() const;

    void SetValid(bool valid) ;

    /// \brief extract test name from JSon object
    /// \return "" if not exists
    static QString ExtractTestName(const QJsonObject& limitItem);
    /// \brief extract test number from JSon object
    /// \return -1 if not exists
    static int ExtractTestNumber(const QJsonObject& limitItem); /// TOCHECK why cast in double in other functions
    /// \brief extract site number from JSon object
    /// \return -1 if not exists
    static int ExtractSiteNumber(const QJsonObject& limitItem);

    bool IsEqual(const MultiLimitItem *other) const;

    bool isParametricItem() const;
    void setIsParametricItem(bool isParametricItem);

private:
    int     mHBin;      ///< store hard bin
    int     mSBin;      ///< store soft bin
    int     mSite;      ///< store site number
    bool    mIsValid;   ///< store item validity, defaultly true
    bool    mIsParametricItem; ///is MPR or merged PTR

public:
    int		ldSampleFails;          // Number of failures reported in the PTR.
    int		ldFailCount;            // Total failures in production
    int		ldOutliers;             // Total outliers
    double	lfLowLimit;             // Test Low Limit
    double	lfHighLimit;            // Test High Limit
    double	lfLowLimitOutlier;      // Outlier Low Limit
    double	lfHighLimitOutlier;     // Outlier High Limit
    double	lfCpk;                  // Cpk Built after pass2...
    double	lfCpkLow;               // CpkL Built after pass2...
    double	lfCpkHigh;              // CpkH Built after pass2...
    double	lfCp;                   // Cp Built after pass2...
    double	lfHistogramMin;         // Low window value in histogram
    double	lfHistogramMax;         // High window value in histogram
    BYTE    bLimitWhatIfFlag;       // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
    char	szLowL[GEX_LIMIT_LABEL];
    char	szHighL[GEX_LIMIT_LABEL];
    BYTE    bLimitFlag;             // SEE Details at end of this file
};

}
}
#endif // MULTILIMITITEM_H
