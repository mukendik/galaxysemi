#ifndef PAT_SINF_INFO_H
#define PAT_SINF_INFO_H

#include <QString>
#include <QMap>

// KLA/INF beginning of wafermap line keyword.
#define	KLA_ROW_DATA_TAB_STRING		"\t    RowData:"
#define	KLA_ROW_DATA_STRING			"RowData:"

namespace GS
{
namespace Gex
{
namespace PAT
{

// Used to hold info from KLA file required if generating SINF output file
class SINFInfo
{
public:

    SINFInfo();
    ~SINFInfo();

    SINFInfo& operator=(const SINFInfo&);

    void    Clear();

    int		mColRdc;                // Offset to reference X-die
    int		mFlatOrientation;       // wafermap flat orientation
    int		mRefPX;                 // Reference X-die loc
    int		mRefPY;                 // Reference Y-die loc
    int		mRowRdc;                // Offset to reference Y-die
    int		mWaferAndPaddingCols;	// Total dies & padding dies over the X axis
    int		mWaferAndPaddingRows;	// Total dies & padding dies over the Y axis
    double	mDieSizeX;              // Die size in X (in mm)
    double	mDieSizeY;              // Die size in Y (in mm)
    QString	mBCEQ;                  // List of Good bins
    QString mDeviceName;
    QString	mINKONLYBCBC;           // List of Ink only bins
    QString	mLot;
    QString	mNOTOUCHBC;             // List of No touch bins
    QString	mSKIPBC;                // List of Skip bins
    QString	mWaferID;
    QString	mNewWafermap;           // Wafermap ASCII array in SINF format, ready to use!
};

}   // namespace PAT
}   // namespace Gex
}   // namespace GS
#endif // PAT_SINF_INFO_H
