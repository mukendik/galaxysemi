#ifndef WAFERMAP_H
#define WAFERMAP_H

#define GEX_WAFMAP_EMPTY_CELL			-1		// Wafermap cell value if empty.
#define GEX_WAFMAP_FAIL_CELL			0		// Wafermap cell value if fail.
#define GEX_WAFMAP_PASS_CELL			1		// Wafermap cell value if pass.

#define GEX_WAFMAP_INVALID_COORD		-32768

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_constants.h"
#include "wafer_transform.h"
#include "wafer_coordinate.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QTextStream>
#include <QVector>
#include <QList>
#include <vector>

class CReportOptions;

namespace GS
{
    namespace QtLib
    {
        class Range;
    }
}

//////////////////////////////////////////////////////
// WaferMap class
//////////////////////////////////////////////////////
class CWafMapArray
{
protected:
    CWafMapArray();

public:
    // Could return 0 on failure
    static	CWafMapArray *	allocate(uint uiSize, CWafMapArray * pOther = NULL);

    void    setBin(int iVal);
    int     getBin() const;
    void    setOrgBin(int iVal);
    int     getOrgBin() const;
    void    setValue(double dVal);
    double  getValue() const;
    /*!
     * \fn SetReticleDieX
     */
    void SetReticleDieX(int x);
    /*!
     * \fn SetReticleDieY
     */
    void SetReticleDieY(int y);
    /*!
     * \fn GetReticleDieX
     */
    int GetReticleDieX();
    /*!
     * \fn GetReticleDieY
     */
    int GetReticleDieY();
    /*!
     * \fn SetReticlePosX
     */
    void SetReticlePosX(int x);
    /*!
     * \fn SetReticlePosY
     */
    void SetReticlePosY(int y);
    /*!
     * \fn GetReticlePosX
     */
    int GetReticlePosX();
    /*!
     * \fn GetReticlePosY
     */
    int GetReticlePosY();

protected:
    int		mBin;               // Binning result (or last bin value if retested...)
    int		mOriginalBin;       // -1 if die not retested...original bin otherwise
    double	mValue;             // die result (for parametric test)
    int     mReticleDieY;
    int     mReticleDieX;
    int     mReticlePosY;
    int     mReticlePosX;
};

struct CStackedWafMapArray
{
    int     lStatus;        // Pass/Fail status of the stacked dies. True when pass, otherwise false
    int		ldCount;		// Number of tests results merged for this die, or # of bins merged.
    double	dValue;			// value of the merge.
    int		nBinToBinDie;	// Use to store the both binning for test and retest when doing a bin to bin mismatch wafermap.
                            // High word contains the bin for the initial test, and low word contains binning for retest

    // For each die (if Parametric test)
    double			lfParamTotal;		// Sum of X
    double			lfParamTotalSquare;// Sum of X*X
};



// Let mutate this to QObject to be dev rules compliant and to prepare Denso request
class CWaferMap : public QObject
{
    Q_OBJECT

public:
    CWaferMap(QObject *parent=0);
    // copy constructor
    // Includes all data related to the STDF WaferMap records
    CWaferMap(const CWaferMap & cwmWaferMapToCopy);
    ~CWaferMap();

    enum BinRetestInstance
    {
        BeforeRetest=0,
        FirstTest=1,
        LastTest=2
    };

    /*!
     *  The enum describes the type of ring
     */
    enum Ring
    {
        NoRing = -1,        /*!< This means there is no ring */
        RingFromCenter = 0, /*!< The ring is located on the wafer center */
        RingFromEdge        /*!< The ring is located on the wafer edge*/
    };

    /*!
     *  The enum defines what kind of edge die should be kept.
     */
    enum EdgeDie
    {
        EdgeDieAdjacent = 0x01, /*!< Die having only side in thouch with the wafer edge  */
        EdgeDieCorner = 0x02,   /*!< Die having at least two sides in touch with the wafer edge  */
        EdgeDieBoth = 0x03      /*!< All edge dies */
    };

    enum AroundDieType
    {
        AdjacentDie = 0x01,
        DiagonalDie = 0x02,
        BothDie		= 0x03
    };

    enum AlignmentType
    {
        AlignNone           = 0x00,
        AlignGeometry       = 0x01,
        AlignAxisDirection  = 0x02,
        AlignOrientation    = 0x04,
        AlignRefLocation    = 0x08,
        AlignBoundaryBins   = 0x10,
        AlignAll            = 0xFF
    };

    Q_DECLARE_FLAGS(AroundDie, AroundDieType)
    Q_DECLARE_FLAGS(Alignment, AlignmentType)

    // Public functions
    bool isDieInsideRing(Ring eRing, int nWidth, int nIndex) const;
    bool isDieInsideRing(Ring eRing, int nWidth, int nDieX, int nDieY) const;
    bool isDieOnEdge(int nIndex, int nEdgeDieType) const;
    bool isDieOutOfWafer(int nIndex) const;


    //! \brief Check is both x and y are 'valid' but does NOT say whether there is a die or not
    Q_INVOKABLE bool isValidCoord(int nDieX, int nDieY) const;
    // Check if DieX is between low and high
    Q_INVOKABLE bool isValidXCoord(int nDieX) const;
    Q_INVOKABLE bool isValidYCoord(int nDieX) const;
    //! \brief Return bin# at location DieX,DieY. -1 returned if invalid location
    Q_INVOKABLE int	 binValue(int nDieX, int nDieY, BinRetestInstance eTestInstance);
    Q_INVOKABLE int	 GetLastTestBinValue(const int &nDieX, const int &nDieY);
    //! \brief ?
    bool HasValidDieAround(int lIndex, AroundDie eAroundDie = BothDie);
    bool HasEmptyDieAround(int lIndex, AroundDie eAroundDie = BothDie) const;

    /*!
      @brief    Parse the given string and compute the reference die location.

      @param    lRefDieLocation     The reference die location information given as a string

      @return   True if the reference die location is successfully computed, otherwise false.
      */
    bool  ComputeReferenceDieLocation(const QString& lRefDieLocation);

    /*!
      @brief    Retrieve the list of drop outs

      @return   A list of coordinates representing the drop outs.
      */
    QList<GS::Gex::WaferCoordinate> GetDropOuts() const;

    /*!
      @brief    Retrieve the index \nIndex of a die based on the \a nDieX and \a nDieY coordinate.

      @param    nIndex      The index computed
      @param    nDieX       The X coordinate reference
      @param    nDieY       The Y coordinate reference

      @return   True if the coordinate are valid and corresponds to a valid index, otherwise false.
      */
    Q_INVOKABLE bool indexFromCoord(int& nIndex, int nDieX, int nDieY) const;

    /*!
      @brief    Retrieve the coordinate \a nDieX and \a nDieY of a die based on the \a nIndex
                index in the map array.

      @param    nIndex      The reference index of the die in the array
      @param    nDieX       The X coordinate computed
      @param    nDieY       The Y coordinate computed

      @return   True if the index corresponds to a valid coordinate, otherwise false.
      */
    Q_INVOKABLE bool coordFromIndex(int nIndex, int& nDieX, int& nDieY) const;

    /*!
      @brief    Retrieve the list of coordinates corresponding to the given bin

      @param    lBin    The bin to retrieve.

      @return   A list of coordinates having the given bin.
      */
    QList<GS::Gex::WaferCoordinate>     GetBinCoordinates(int lBin) const;

    /*!
      @brief    Returns a list coordinates for reference die locations
      */
    Q_INVOKABLE const QList<GS::Gex::WaferCoordinate>&  GetReferenceDieLocation() const;

    /*!
      @brief    Returns the coordinate of the center die
      */
    Q_INVOKABLE const GS::Gex::WaferCoordinate&     GetCenterDie() const;

    /*!
      @brief    Returns the diameter of the wafer.

      @return   The diameter of the wafer in WF_UNITS
      */
    Q_INVOKABLE double                      GetDiameter() const;

    /*!
      @brief    Returns the height of a die.

      @return   The height of a die in WF_UNITS
      */
    Q_INVOKABLE double                      GetDieHeight() const;

    /*!
      @brief    Returns the width of a die.

      @return   The width of a die in WF_UNITS
      */
    Q_INVOKABLE double                      GetDieWidth() const;

    /*!
      @brief    Returns the positive X direction

      @return   True when X increase to right, otherwise false
      */
    Q_INVOKABLE bool                        GetPosXDirection() const;

    /*!
      @brief    Returns the positive Y direction

      @return   True when Y increase to top, otherwise false
      */
    Q_INVOKABLE bool                        GetPosYDirection() const;

    /*!
      @brief    Sets the coordinates of the reference die locations

      @param    dieLocation    The list of coordinates of the reference die
      */
    Q_INVOKABLE void                        SetReferenceDieLocation(const QList<GS::Gex::WaferCoordinate>& dieLocation);

    /*!
      @brief    Sets the coordinate of the center die

      @param    lCenterDie      The coordinate of the center die
      */
    Q_INVOKABLE void                        SetCenterDie(GS::Gex::WaferCoordinate lCenterDie);

    /*!
      @brief    Sets the diameter of the wafer in WF_UNITS

      @param    lDiameter       The diameter of the wafer.
      */
    Q_INVOKABLE void                        SetDiameter(double lDiameter);

    /*!
      @brief    Sets the height of a die in WF_UNITS

      @param    lDieHeight      The height of a die.
      */
    Q_INVOKABLE void                        SetDieHeight(double lDieHeight);

    /*!
      @brief    Sets the width of a die in WF_UNITS

      @param    lDieWidth       The width of a die.
      */
    Q_INVOKABLE void                        SetDieWidth(double lDieWidth);

    /*!
      @brief    Sets positive X direction

      @param    xDirection    The positive X direction.
      */
    Q_INVOKABLE void                        SetPosXDirection(bool xDirection);

    /*!
      @brief    Sets positive Y direction

      @param    yDirection    The positive Y direction.
      */
    Q_INVOKABLE void                        SetPosYDirection(bool yDirection);

    /*!
      @brief    Rotate the wafer orientation 90° counterclockwise in the current coordinate system.

      @return   True if the rotation succeed, otherwise false.
      */
    Q_INVOKABLE bool                        Rotate(int quarters);

    /*!
      @brief    Moves the wafer coordinates x along the X axis and y along the Y axis.

      @param    x   The translation along the X axis
      @param    y   The translation along the Y axis
      */
    Q_INVOKABLE bool                        Translate(int x, int y);

    /*!
      @brief    Reflects the wafer coordinates x and y over the given mirror lines (X and/or Y axis).
                If reflection has to be done accross both axis, it consists in reflection through a
                point.

      @param    lXAxis   True if X axis has to be used as mirror line
      @param    lYAxis   True if Y axis has to be used as mirror line

      @return   True if the reflection succeed, otherwise false.
      */
    Q_INVOKABLE bool                        Reflect(bool lXAxis, bool lYAxis);

    /*!
      @brief    Apply the given transformation to the wafer.

      @param    transform   Transformation to apply

      @return   True if the transormation succeed, otherwise false
      */
    Q_INVOKABLE bool                        Transform(const GS::Gex::WaferTransform& transform);

    /*!
      @brief    Compare wafermaps alignment depending on the given rule.

      @param    other           Wafermap used to check the alignment
      @param    alignRule       Alignment rule to check
      @param    boundaryBins    Bins list that shouldn't be overloaded by the second wafer.

      @return   Rule that doesn't match the alignment
      */
    Q_INVOKABLE Alignment                   CompareAlignment(const CWaferMap& other,
                                                 Alignment alignRule,
                                                 const GS::QtLib::Range& boundaryBins) const;

    /*!
      @brief    Get the count of die matching/unmatching with the given \a lBinList

      @param    lMatch          Count of die matching with the bins list
      @param    lUnmatch        Count of die unmatching with the bins list
      @param    lBinList        Bins list
     */
    Q_INVOKABLE void                        GetMatchingDie(int& lMatch, int& lUnmatch, QString lBinList);

    /*!
      @brief    Update the die with the \a lBin number at \a lX and \a lY coordinates.

      @param    lBin   The bin number to assign to the targeted die
      @param    lX     The X coordinate of the targeted die
      @param    lY     The Y coordinate of the targeted die

      @return   True if the bin of the die is updated, otherwise false.
      */
    Q_INVOKABLE bool                        UpdateBin(int lBin, int lX, int lY);

    /*!
      @brief    Update active orientation.
      */
    Q_INVOKABLE void                        UpdateActiveOrientation();

    /*!
      @brief    Returns the orientation rotated by \a aQuarters * 90° clockwise

      @param    lOrientation    The orientation to rotate. Orientation must be normalized.
      @param    aQuarters       Numbers of 90° rotation to perform

      @return   The rotated wafer orientation
      */
    static char                 RotateOrientation(char lOrientation, int aQuarters);

    /*!
      @brief    Returns a wafer size info in milli-meters

      @param    lValue          Wafer size to convert.
      @param    lWaferUnits     The wafer units of the value to convert

      @return   The converted value
      */
    static double               ConvertToMM(double lValue, int lWaferUnits);

    /*!
      @brief    Returns toggled orientation of the wafer. Orientation is toggled between normalized viewport
                (where positive X axis direction is right and positice Y axis direction is down) and current viweport.
                If current viewport is normalized, toggling orientation will not have any effect.

      @param    aOrientation    Orientation of the wafer
      @param    aPosX           Positive X axis direction, true when going right, otherwise false
      @param    aPosY           Positive Y axis direction, true when going down, otherwise false

      @return   Toggled wafer orientation
      */
    static char                 ToggleOrientation(char aOrientation, bool aPosX, bool aPosY);

    /*!
      @brief    Returns the wafer orientation in degree with following value:
                 - Right is 0°
                 - Down is 90°
                 - Left is 180°
                 - Up is 270°

      @param    aOrientation    Orientation of the wafer

      @return   Orientation in degree
      */
    static int                  OrientationToDegree(char aOrientation);

    /*!
      @brief    Returns the wafer orientation from its values in degree with following value:
                 - Right is 0°
                 - Up is 90°
                 - Left is 180°
                 - Down is 270°

      @param    aDegree    Orientation of the wafer in degree

      @return   Orientation
      */
    static char                 OrientationFromDegree(int aDegree);

    void SwapWaferNotchPosition();


    // Get coordinates around a die
    std::vector<int>	aroundDieCoordinate(int nIndex,
         AroundDie eAroundDie = BothDie, int nRing = 0, int nWidth = 1,
         bool bKeepInvalid = false) const;

    Q_INVOKABLE void	clear(void);
    Q_INVOKABLE void	clearWafer(void);

    Q_INVOKABLE bool	loadFromEtestfile(QString &strWaferFile);	// Loads a wafermap E-test file into the structure.
    Q_INVOKABLE bool	loadFromCompositefile(QTextStream &hCompositeFile,int &iWaferID);	// Loads a wafermap Galaxy Composite file into the structure.

    Q_INVOKABLE int		GetWaferNotch();	// Get notch location to be used: 12 (12Hour = Up), 3 (3Hour = Right), 6 (6Hour = Down), 9 (9Hour= Left)
    Q_INVOKABLE void	RotateWafer();		// Rotate Wafer by +90degres

    //! \brief Is it automatically up to date ?
    int	iLowDieX,iHighDieX,iLowDieY,iHighDieY;
    Q_INVOKABLE int GetLowDieX() const {return iLowDieX;}
    Q_INVOKABLE int GetLowDieY() const {return iLowDieY;}
    Q_INVOKABLE int GetHighDieX() const {return iHighDieX;}
    Q_INVOKABLE int GetHighDieY() const {return iHighDieY;}

    //! \brief Wafermap size (cells in X, and Y) : todo: move me to private
    int	SizeX,SizeY;
    Q_INVOKABLE int GetSizeX() const {return SizeX;}
    Q_INVOKABLE int GetSizeY() const {return SizeY;}

    Q_INVOKABLE int GetCenterX() const {return mCenterDie.GetX(); }
    Q_INVOKABLE int GetCenterY() const {return mCenterDie.GetY(); }

    //! \brief Total number of physical dies tested on the wafer (dosn't counts retests) : todo: move me to private
    int	iTotalPhysicalDies;
    Q_INVOKABLE int GetTotalPhysicalDies() const {return iTotalPhysicalDies;}

    time_t	lWaferStartTime,lWaferEndTime;
    char	szWaferID[MIR_STRING_SIZE];
    char	szWaferIDFilter[MIR_STRING_SIZE];	// Holds the only WaferID to process if multiple wafers in same file. Empty if no filter
    uchar	bWaferUnits;		// WF_UNITS:0 (unknown) 1(inches) 2(cm) 3(mm) 4(mils)
    char	cWaferFlat_Stdf;		// STDF WF_FLAT (original from STDF file):'U' for Up, 'D' for Down, R or L for Right/Left; otherwise unkown
    char	cWaferFlat_Detected;	// Detected WF_FLAT (detected by GEX):'U' for Up, 'D' for Down, R or L for Right/Left; otherwise unkown
    char	cWaferFlat_Active;		// Active WF_FLAT (from file, detected, or forced)
    char	cPos_X, cPos_Y;			// WCR.POS_X, WCR.POS_Y


    bool	bPartialWaferMap;	//=TRUE if data outside of buffer allocated
    bool	bStripMap;			//=TRUE if we have to build a StripMap (final test wafer-map)
    bool 	bWaferMapExists;	//=TRUE if Wafer map data exist
    bool	bWirExists;			//=TRUE if Wafer map WIR record exists (so valid Wafer sort data file)
    bool	bFirstSubLotInFile;	//=TRUE if this Wafer/Sub-lot is the first one found in the file processed.

    long 	lTestNumber;		// Test number used to fill the wafermap


// Operators
public:
    CWaferMap&	operator=( const CWaferMap &s );	// assignment operator

    Q_INVOKABLE bool	isValidWaferFlatOrigin() const;		// Check the wafer flat read from stdf file is valid
    void                SetReticleMinX(int x)       { mReticleMinX = x;}
    void                SetReticleMaxX(int x)       { mReticleMaxX = x;}
    void                SetReticleMinY(int y)       { mReticleMinY = y;}
    void                SetReticleMaxY(int y)       { mReticleMaxY = y;}
    unsigned int        GetReticleWidth() const ;
    unsigned int        GetReticleHeight() const;
    int                 GetReticleMinX() const      {return mReticleMinX;}
    int                 GetReticleMaxX() const      {return mReticleMaxX;}
    int                 GetReticleMinY() const      {return mReticleMinY;}
    int                 GetReticleMaxY() const      {return mReticleMaxY;}
    bool                HasReticle() const;

protected:
    bool        UpdateOptions(CReportOptions*);		// update cached options
    int         DetectFlatLocation();				// Tells Flat locatioin: 12 (12Hour = Up), 3 (3Hour = Right), 6 (6Hour = Down), 9 (9Hour= Left)

private:

    enum VisualOption
    {
        Mirror_X	= 0x01,
        Mirror_Y	= 0x02,
        AllParts	= 0x04,
        ShapeRound	= 0x08,
        SwapNotch   = 0x10
    };

    int                         mVisualOptions;

    int                         mReticleMinX;
    int                         mReticleMaxX;
    int                         mReticleMinY;
    int                         mReticleMaxY;
    bool                        mPosXDirection;             // TRUE if X increase to right
    bool                        mPosYDirection;             // TRUE if Y increase to top

    double                      mDiameter;                  // Wafer diameter in 'WF_UNITS'
    double                      mDieHeight;                 // Die Height in WF_UNITS
    double                      mDieWidth;                  // Die Width in WF_UNITS

    GS::Gex::WaferTransform     mTransform;
    GS::Gex::WaferCoordinate    mCenterDie;

    QList<GS::Gex::WaferCoordinate> mRefDieLocation;

protected:

    int             *   m_piCellTestCounter;
    int                 m_iCellTestCounterSize;
    CWafMapArray *      cWafMap;			// Pointer to wafermap BIN results array

public:

    int *getCellTestCounter();
    int getCellTestCounterSize();
    int *allocCellTestCounter(int iSize, int *pCellCounter=0);
    void freeCellTestCounter();

    const CWafMapArray *    getWafMap() const;
    CWafMapArray *          getWafMap();
    CWafMapArray            getWafMapDie(int index) const;
    void                    setWaferMap(CWafMapArray *poWafer);
};

//////////////////////////////////////////////////////
// Stacked WaferMap class (stack wafers in dataset query)
//////////////////////////////////////////////////////
class	CStackedWaferMap
{
public:
    CStackedWaferMap();
    ~CStackedWaferMap();

    // Public functions
    void	clear(void);
    int		iTotalWafermaps;			// total wafermaps merged.
    int		iTotaltime;					// Cumulated time (in sec.) for testing all the wafers tested.
    int		iTotalPhysicalDies;			// Total number of physical dies tested on the wafer (dosn't counts retests)
    int		iLowDieX,iHighDieX,iLowDieY,iHighDieY;	// Merged wafermaps dimension
    int		SizeX,SizeY;				// Wafermap size (cells in X, and Y)
    int		iHighestDieCount;			// Holds the maximum of results on a single die
    double	lfLowWindow,lfHighWindow;	// If parametric wafermap, holds the value range for the given test
    // List of Binnings to merge (count)
    GS::QtLib::Range *pGexBinList;
    CStackedWafMapArray	*cWafMap;		// Pointer to wafermap BIN results array

    bool	bPosX;				// TRUE if no Mirror when reading X line
    bool	bPosY;				// TRUE if no Mirror when reading Y line
    bool	bPartialWaferMap;	//=TRUE if data outside of buffer allocated
    bool	bStripMap;			//=TRUE if we have to build a StripMap (final test wafer-map)
    bool	bWaferMapExists;	//=TRUE if Wafer map data exist

private:
    Q_DISABLE_COPY(CStackedWaferMap)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(CWaferMap::Alignment)

#endif // WAFERMAP_H
