///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "wafermap.h"
#include "report_options.h"
#include "gex_report.h"
#include "gexperformancecounter.h"
#include <gqtl_log.h>
#include <gqtl_global.h>
#include <gs_types.h>

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QStringList>
#include <QFile>

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport *		gexReport;
extern CReportOptions	ReportOptions;

extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
// Define for regular expression
///////////////////////////////////////////////////////////////////////////////////
#define REGEXP_REF_LOCATION_FROM_COORD      "Coord\\((\\-?\\d{1,5}),(\\-?\\d{1,5})\\)"
#define REGEXP_REF_LOCATION_FROM_BIN        "Bin\\((\\d{1,5})\\)"
#define REGEXP_REF_LOCATION_FROM_DROPOUTS   "PCMDropOut"

///////////////////////////////////////////////////////////////////////////////////
// Class CWafMapArray
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CWafMapArray::CWafMapArray()
    : mBin(GEX_WAFMAP_EMPTY_CELL),
      mOriginalBin(GEX_WAFMAP_EMPTY_CELL),
      mValue(GEX_C_DOUBLE_NAN),
      mReticleDieY(0),
      mReticleDieX(0),
      mReticlePosY(0),
      mReticlePosX(0)
{
}

///////////////////////////////////////////////////////////
// Static Methods
///////////////////////////////////////////////////////////
CWafMapArray * CWafMapArray::allocate(uint uiSize, CWafMapArray * pOther /* = NULL */)
{
    quint64		uiMemorySizeLimit		= 2UL * 1024UL * 1024UL * 1024UL; // Max memory to allocate (defined value is 2GB)
    quint64		uiMemorySizeToAllocate	= quint64(uiSize) * sizeof(CWafMapArray);

    CWafMapArray * pWafmapArray = NULL;

    try
    {
        // Do not try to allocate to much memory, behave like the system cannot allocate memory
        if (uiMemorySizeToAllocate > uiMemorySizeLimit)
        {
            GSLOG(SYSLOG_SEV_CRITICAL, (QString("trying to allocate too much memory : %1, max is %2")
                  .arg( uiMemorySizeToAllocate).arg( uiMemorySizeLimit)).toLatin1().constData());
            throw std::bad_alloc();
        }

        pWafmapArray = new CWafMapArray[uiSize];

        if (pOther)
            memcpy(pWafmapArray, pOther, uiSize * sizeof(CWafMapArray));
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        pWafmapArray=0;
    }
    catch(...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        pWafmapArray=0;
    }

    return pWafmapArray;
}

///////////////////////////////////////////////////////////////////////////////////
// Class CWafMap
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Includes all data related to the STDF WaferMap records
///////////////////////////////////////////////////////////
CWaferMap::CWaferMap(QObject *lParent):QObject(lParent)
{
    mVisualOptions    = 0;
    cWafMap             = NULL;
    m_piCellTestCounter = 0;

    UpdateOptions(&ReportOptions);

    clear();
}

CWaferMap::CWaferMap(const CWaferMap & cwmWaferMapToCopy): QObject(cwmWaferMapToCopy.parent())
{
    mVisualOptions = 0;
    cWafMap			= NULL;
    m_piCellTestCounter = 0;

    UpdateOptions(&ReportOptions);

    clear();

    *this = cwmWaferMapToCopy;
}

///////////////////////////////////////////////////////////
CWaferMap::~CWaferMap()
{
    clear();
}

void CWaferMap::SwapWaferNotchPosition()
{
    char lNewPosition = toupper(cWaferFlat_Stdf);
    if (mVisualOptions & Mirror_Y)
    {
        if (lNewPosition == 'U') lNewPosition = 'D';
        else if (lNewPosition == 'D') lNewPosition = 'U';
    }

    if (mVisualOptions & Mirror_X)
    {
        if (lNewPosition == 'R') lNewPosition = 'L';
        else if (lNewPosition == 'L') lNewPosition = 'R';
    }

    cWaferFlat_Stdf = lNewPosition;
}

bool CWaferMap::UpdateOptions(CReportOptions * pReportOptions)
{
    if (pReportOptions == NULL)
        return false;

    bool bOk	= true;	// set this to false if you detect an GetOption() error

    // Get Wafermap visual options
    QStringList lstVisualOptions = pReportOptions->GetOption("wafer", "visual_options").toString().split("|");
    mVisualOptions = 0;

    if (lstVisualOptions.contains("mirror_x"))
        mVisualOptions	|= Mirror_X;

    if (lstVisualOptions.contains("mirror_y"))
        mVisualOptions	|= Mirror_Y;

    if (lstVisualOptions.contains("all_parts"))
        mVisualOptions	|= AllParts;

    if (lstVisualOptions.contains("shape_round"))
        mVisualOptions	|= ShapeRound;

    if (lstVisualOptions.contains("swap_notch"))
        mVisualOptions	|= SwapNotch;

    return bOk;
}

void CWaferMap::clear(void)
{
    bPartialWaferMap	= FALSE;
    bStripMap			= FALSE;
    bWaferMapExists		= FALSE;
    bWirExists			= FALSE;
    iLowDieX			= 33000;
    iHighDieX			= -33000;
    iLowDieY			= 33000;
    iHighDieY			= -33000;
    lWaferStartTime		= -1;
    lWaferEndTime		= -1;
    *szWaferID			=0;
    *szWaferIDFilter	=0;		// Will hold the WaferID filter (only wafer to read) in case multiple Wafers in a ssame STDF file

    iTotalPhysicalDies  =0;		// Total number of physical dies tested on the wafer (dosn't counts retests)

    mDiameter           = -1;   // Wafer size in 'WF_UNITS'
    mDieHeight          = -1;	// Die Height in WF_UNITS
    mDieWidth           = -1;	// Die Width in WF_UNITS
    bWaferUnits			= 0;		// WF_UNITS:0 (unknown) 1(inches) 2(cm) 3(mm) 4(mils)
    SizeX = SizeY		= 0;
    cPos_X				= ' ';
    cPos_Y				= ' ';
    mPosXDirection		= true;	// TRUE if no Mirror when reading X line
    mPosYDirection		= true;	// TRUE if no Mirror when reading Y line
    cWaferFlat_Stdf		= ' ';	// WF_FLAT from Stdf file
    cWaferFlat_Detected	= ' ';	// Detected Flat orientation
    cWaferFlat_Active	= ' ';	// Active Flat orientation

    mCenterDie          = GS::Gex::WaferCoordinate();

    lTestNumber			= -1;

    bFirstSubLotInFile	= TRUE;

    // Destroy wafer map buffer
    if(cWafMap != NULL)
    {
        delete [] cWafMap;
    }
    cWafMap=NULL;
    freeCellTestCounter();
    mReticleMinX  = GS_MAX_INT16;
    mReticleMaxX  = GS_MIN_INT16;
    mReticleMinY  = GS_MAX_INT16;
    mReticleMaxY  = GS_MIN_INT16;

}

///////////////////////////////////////////////////////////
// Preset wafer with emptycells IDs
///////////////////////////////////////////////////////////
void CWaferMap::clearWafer(void)
{
    for(int iIndex=0; iIndex < SizeX*SizeY; iIndex++)
    {
        cWafMap[iIndex].setBin	( GEX_WAFMAP_EMPTY_CELL);	// Initializes each wafermap cell.
        cWafMap[iIndex].setOrgBin( GEX_WAFMAP_EMPTY_CELL);	// Initializes each wafermap cell retest info
        cWafMap[iIndex].setValue( GEX_C_DOUBLE_NAN);
    }
}

///////////////////////////////////////////////////////////
// Loads a wafermap file (E-Test data file) into the structure.
///////////////////////////////////////////////////////////
bool	CWaferMap::loadFromEtestfile(QString &strWaferFile)
{
    QString	strString;

    // If wafer already in memory: fush it!
    clear();

    QFile file(strWaferFile); // Read wafermap file
    if(file.open(QIODevice::ReadOnly) == false)
        return false;	// Failed opening wafermap file.

    // Read wafermap File
    QTextStream hWafMap;
    hWafMap.setDevice(&file);	// Assign file handle to data stream

    // Check if valid header...or empty!
    strString = hWafMap.readLine();

    // Check if file includes the 'TSMC' header signature.
    if(strString != "TSMC")
        return false;

    // Skip first 3 lines (include Product, wafer ID,...)
    hWafMap.readLine();	// Product name
    hWafMap.readLine();	// WaferID
    hWafMap.readLine();	// Name of this wafermap file

    // Step#1: read the file once to identify the wafermap size
    int	iLength;
    SizeX=0;	// Member variable
    SizeY=0;	// Member variable
    do
    {
        // Read one line of the wafer map. Can be like:
        // ............................11XXXX1XX...........................
        strString = hWafMap.readLine();
        iLength = strString.length();
        if(iLength)
        {
            SizeX = gex_max(SizeX,(int)strString.length());	// Line legnth is wafermap width (as we have one character per die)

            // Valid line, keep track of the total lines (number of lines in wafermap)
            SizeY++;
        }
    }
    while(hWafMap.atEnd() == false);

    // Step#2: rewind
    file.seek(0);
    hWafMap.setDevice(&file);	// Assign file handle to data stream

    // Step#3: allocate wafermap memory
    int	iIndex;
    cWafMap = CWafMapArray::allocate(SizeX*SizeY);
    /* HTH - case 4156 - Catch memory allocation exception
    cWafMap = new CWafMapArray[SizeX*SizeY];
    */
    if(cWafMap==NULL)
    {
        // Failed allocating memory for wafermap===> corrupted wafermap, ignore it!
        bWaferMapExists = FALSE;
        return false;
    }

    /* HTH - case 4156 - Catch memory allocation exception - ALREADY DONE IN THE ALLOCATE(...) METHOD
    // Load wafer buffer with 'empty cell' IDs
    clearWafer();
    */

    // Step#4: load wafermap in buffer
    hWafMap.readLine();	// Skip: 'TSMC' signature
    hWafMap.readLine();	// Skip: Product name
    hWafMap.readLine();	// Skip: WaferID
    hWafMap.readLine();	// Skip: Name of this wafermap file
    char	cChar;
    int		iLine;
    for(iLine=0;iLine < SizeY; iLine++)
    {
        // Read one line of the wafer map. Can be like:
        // ............................11XXXX1XX...........................
        strString = hWafMap.readLine();
        iLength = strString.length();

        // Parse line and store it in memory
        if(iLength)
        {
            for(iIndex=0;iIndex<iLength;iIndex++)
            {
                cChar = strString[iIndex].toLatin1();
                switch(cChar)
                {
                    case '.' :	// Empty cell; no die at this location
                        break;

                    default:
                    case '1' :	// Good die
                    case 'X' :	// Bad die (reject)
                    case '@' :	// Bad die (visual reject)
                        cWafMap[iIndex + (SizeX*iLine)].setBin (cChar);
                        break;
                }
            }
        }
    }

    // Wafermap loaded.
    return true;
}

///////////////////////////////////////////////////////////
// Loads a wafermap file (Galaxy composite data file) into the structure.
///////////////////////////////////////////////////////////
bool	CWaferMap::loadFromCompositefile(QTextStream &hCompositeFile,int &iWaferID)
{
    // Step#1: Get wafer size
    QString	strString;
    SizeX=0;	// Member variable
    SizeY=0;	// Member variable
    iWaferID = -1;
    do
    {
        // Read one line of the wafer map. Can be like:
        // . . . . . 1 1 0 0 0 1 1 . . . . . .
        strString = hCompositeFile.readLine();
        if(strString.startsWith("iSizeX:", Qt::CaseInsensitive))
        {
            SizeX = strString.section(':',1,1).toLong();
        }
        if(strString.startsWith("iSizeY:", Qt::CaseInsensitive))
        {
            SizeY = strString.section(':',1,1).toLong();
        }
        if(strString.startsWith("iRowMin:", Qt::CaseInsensitive))
        {
            iLowDieY = strString.section(':',1,1).toLong();
        }
        if(strString.startsWith("iColMin:", Qt::CaseInsensitive))
        {
            iLowDieX = strString.section(':',1,1).toLong();
        }

        if(strString.startsWith("WaferID:", Qt::CaseInsensitive))
        {
            iWaferID = strString.section(':',1,1).toLong();
        }

        // As soon as X and Y size are defined, exit.
        if(SizeX && SizeY)
            break;
    }
    while(hCompositeFile.atEnd() == false);

    // Check if valid wafermap...
    if(SizeX==0 || SizeY == 0)
        return false;

    // Step#2: allocate wafermap memory
    int	iIndex;
    cWafMap = CWafMapArray::allocate(SizeX*SizeY);
    /* HTH - case 4156 - Catch memory allocation exception
    cWafMap = new CWafMapArray[SizeX*SizeY];
    */
    if(cWafMap==NULL)
    {
        // Failed allocating memory for wafermap===> corrupted wafermap, ignore it!
        bWaferMapExists = FALSE;
        return false;
    }

    /* HTH - case 4156 - Catch memory allocation exception - ALREADY DONE IN THE ALLOCATE(...) METHOD
    // Load wafer buffer with 'empty cell' IDs
    clearWafer();
    */

    int		iBinCode;
    bool	bFlag;
    int		iLine;
    QStringList	strSplitDies;
    for(iLine=0;iLine < SizeY; iLine++)
    {
        // Read one line of the wafer map. Can be like:
        // RowData:.. ....  E  1  1  1  18E  1  1  1  17F  1  1  6  1  1  1  1  1 etc...
        strString = hCompositeFile.readLine();

        // Parse line and store it in memory
        if(strString.startsWith("RowData:", Qt::CaseInsensitive))
        {
            // Get Wafermap info.
            strString = strString.section(':',1);

            // Split into X-Cells ('space' separator)
            strSplitDies = strString.split(" ");
        }

        for(iIndex=0;iIndex<SizeX;iIndex++)
        {
            iBinCode = strSplitDies[iIndex].toInt(&bFlag,16);
            if(bFlag == true)
            {
                cWafMap[iIndex + (SizeX*iLine)].setBin (iBinCode);
                iTotalPhysicalDies++;
            }
        }
    }

    // Set/Update variables
    iHighDieX = iLowDieX + SizeX -1;
    iHighDieY = iLowDieY + SizeY -1;
    bWaferMapExists = true;

    // Wafermap loaded.
    return true;
}

///////////////////////////////////////////////////////////
bool CWaferMap::isDieOnEdge(int nIndex, int nEdgeDieType) const
{
    int     nCoordX     = 0;
    int     nCoordY     = 0;
    bool    bEdgeDie    = false;
    int     lEdgeDieTypeFound = 0;

    if (coordFromIndex(nIndex, nCoordX, nCoordY))
    {
        if (nCoordX == iLowDieX || nCoordX == iHighDieX || nCoordY == iLowDieY || nCoordY == iHighDieY)
            lEdgeDieTypeFound = EdgeDieAdjacent;
        else
        {
            int lDieXCenter = SizeX/2;
            int lDieYCenter	= SizeY/2;
            int lXInc       = 0;
            int lYInc       = 0;
            int lArrayIndex = nIndex;

            // Check to which quarter the die belongs
            if ((nCoordX - iLowDieX) < lDieXCenter)
            {
                // Left part of the wafer
                lXInc = -1;
            }
            else
            {
                // Right part of the wafer
                lXInc = 1;
            }

            if ((nCoordY - iLowDieY) < lDieYCenter)
            {
                // Top part of the wafer
                lYInc = -SizeX;
            }
            else
            {
                // Bottom part of the wafer
                lYInc = SizeX;
            }

            lArrayIndex += lXInc + lYInc;

            if (isDieOutOfWafer(lArrayIndex))
            {
                if (HasEmptyDieAround(nIndex, AdjacentDie))
                    lEdgeDieTypeFound |= EdgeDieAdjacent;
                else if (HasEmptyDieAround(nIndex, DiagonalDie))
                    lEdgeDieTypeFound |= EdgeDieCorner;
            }
        }
    }

    if ((nEdgeDieType & lEdgeDieTypeFound) != 0)
    {
        bEdgeDie = true;
    }

    return bEdgeDie;
}

bool CWaferMap::isDieOutOfWafer(int nIndex) const
{
    int     nCoordX     = 0;
    int     nCoordY     = 0;

    if (coordFromIndex(nIndex, nCoordX, nCoordY))
    {
        bool lEmptyCell = cWafMap[nIndex].getBin() == GEX_WAFMAP_EMPTY_CELL;

        // We found a empty cell, is it really a die outside a wafer?
        if (lEmptyCell)
        {
            int lDieXCenter = SizeX/2;
            int lDieYCenter	= SizeY/2;
            int lXInc       = 0;
            int lYInc       = 0;
            int lArrayIndex = nIndex;

            // Check to which quarter the die belongs
            if ((nCoordX - iLowDieX) < lDieXCenter)
            {
                // Left part of the wafer
                lXInc = -1;
            }
            else
            {
                // Right part of the wafer
                lXInc = 1;
            }

            if ((nCoordY - iLowDieY) < lDieYCenter)
            {
                // Top part of the wafer
                lYInc = -SizeX;
            }
            else
            {
                // Bottom part of the wafer
                lYInc = SizeX;
            }

            int     lMinRowIndex    = (lArrayIndex / SizeX) * SizeX;
            int     lRowIndex       = lArrayIndex + lXInc;
            bool    lRowEmptyCell   = true;
            bool    lColEmptyCell   = true;

            while (lRowIndex >= lMinRowIndex &&
                   lRowIndex < (lMinRowIndex + SizeX) && lRowEmptyCell)
            {
                lRowEmptyCell &= (cWafMap[lRowIndex].getBin() == GEX_WAFMAP_EMPTY_CELL);

                lRowIndex += lXInc;
            }

            int lColIndex = lArrayIndex + lYInc;
            while (lColIndex >= 0 && lColIndex < SizeX*SizeY && lColEmptyCell)
            {
                lColEmptyCell &= (cWafMap[lColIndex].getBin() == GEX_WAFMAP_EMPTY_CELL);

                lColIndex += lYInc;
            }

            if (lColEmptyCell == false && lRowEmptyCell == false)
                return false;
        }
        else
            return false;
    }

    return true;
}

bool CWaferMap::isValidCoord(int nDieX, int nDieY) const
{
    return isValidXCoord(nDieX) && isValidYCoord(nDieY);
}

bool CWaferMap::isValidXCoord(int nDieX) const
{
    return ((nDieX >= iLowDieX) && (nDieX <= iHighDieX));
}

bool CWaferMap::isValidYCoord(int nDieY) const
{
    return ((nDieY >= iLowDieY) && (nDieY <= iHighDieY));
}

///////////////////////////////////////////////////////////
bool CWaferMap::indexFromCoord(int& nIndex, int nDieX, int nDieY) const
{
    if (isValidCoord(nDieX, nDieY))
    {
        int nXOffset;
        int nYOffset;

        // Check if X-axis swap option enabled...
        if(mVisualOptions & CWaferMap::Mirror_X)
            nXOffset = iHighDieX - nDieX;
        else
            nXOffset = nDieX - iLowDieX;

        // Check if Y-axis swap option enabled...
        if(mVisualOptions & CWaferMap::Mirror_Y)
            nYOffset = iHighDieY - nDieY;
        else
            nYOffset = nDieY - iLowDieY;

        nIndex = nXOffset + (SizeX * nYOffset);
        return true;
    }

    return false;
}

bool CWaferMap::coordFromIndex(int nIndex, int& nDieX, int& nDieY) const
{
    if (nIndex >= 0 && nIndex < (SizeX * SizeY))
    {
        nDieX = nIndex % SizeX;
        nDieY = nIndex / SizeX;

        // Check if X-axis swap option enabled...
        if(mVisualOptions & CWaferMap::Mirror_X)
            nDieX = iHighDieX - nDieX;
        else
            nDieX = nDieX + iLowDieX;

        // Check if Y-axis swap option enabled...
        if(mVisualOptions & CWaferMap::Mirror_Y)
            nDieY = iHighDieY - nDieY;
        else
            nDieY = nDieY + iLowDieY;

        return true;
    }

    nDieX = GEX_WAFMAP_INVALID_COORD;
    nDieY = GEX_WAFMAP_INVALID_COORD;

    return false;
}

int	CWaferMap::GetLastTestBinValue(const int &nDieX, const int &nDieY)
{
    return binValue(nDieX, nDieY, LastTest);
}

int CWaferMap::binValue(int nDieX, int nDieY,BinRetestInstance eTestInstance)
{
    // Valid DieX?
    if(isValidXCoord(nDieX) == false)
        return -1;
    // Valid DieY?
    if(isValidYCoord(nDieY) == false)
        return -1;

    // Get bin# at location DieX,DieY
    int		nArrayIndex = (nDieX - iLowDieX) + ((nDieY - iLowDieY) * SizeX);

    // Report Original Bin (will be -1 if die location didn't experience a Retest)
    if(eTestInstance == BeforeRetest)
        return cWafMap[nArrayIndex].getOrgBin();

    // Return last Bin instance
    if(eTestInstance == LastTest)
        return cWafMap[nArrayIndex].getBin();

    // Return original (if retest occured), not -1 if no retest!
    if(eTestInstance == FirstTest)
    {
        if(cWafMap[nArrayIndex].getOrgBin() >= 0)
            return cWafMap[nArrayIndex].getOrgBin();	// This die experienced retest, return original bin
        else
            return cWafMap[nArrayIndex].getBin();		// This die didn't experience retest, return its unique bin result
    }

    return -1;
}

bool CWaferMap::HasValidDieAround(int lIndex, AroundDie eAroundDie /*= BothDie*/)
{
    if (SizeX == 0 || SizeY == 0)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Wafermap size is incorrect size X (%) and size Y (%)");
        return false;
    }

    int	lColIdx     = lIndex % SizeX;
    int lLineIdx    = lIndex / SizeX;
    int lDieOffset  = -1;

    if (eAroundDie & AdjacentDie)
    {
        // Algorithm checking all 4 dies adjacent to center die
        for(int lColRange = -1; lColRange <= 1; lColRange += 2)
        {
            lDieOffset = (lColIdx + lColRange + (lLineIdx *SizeX));
            if(lDieOffset >= 0 && lDieOffset < SizeX * SizeY)
            {
                // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                if(cWafMap[lDieOffset].getBin() != GEX_WAFMAP_EMPTY_CELL)
                    return true;	// This die exists and is around the center die.
            }
        }

        for(int lLineRange = -1; lLineRange <= 1; lLineRange += 2)
        {
            lDieOffset = (lColIdx + ((lLineIdx + lLineRange) * SizeX));
            if(lDieOffset >= 0 && lDieOffset < SizeX * SizeY)
            {
                // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                if(cWafMap[lDieOffset].getBin() != GEX_WAFMAP_EMPTY_CELL)
                    return true;	// This die exists and is around the center die.
            }
        }
    }

    if (eAroundDie & DiagonalDie)
    {
        // Algorithm checking all 4 dies adjacent to center die
        for(int lColRange = -1; lColRange <= 1; lColRange += 2)
        {
            for(int lLineRange = -1; lLineRange <= 1; lLineRange += 2)
            {
                lDieOffset = (lColIdx + lColRange + (lLineIdx + lLineRange) * SizeX);
                if(lDieOffset >= 0 && lDieOffset < SizeX * SizeY)
                {
                    // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                    if(cWafMap[lDieOffset].getBin() != GEX_WAFMAP_EMPTY_CELL)
                        return true;	// This die exists and is around the center die.
                }
            }
        }
    }

    return false;
}

bool CWaferMap::HasEmptyDieAround(int lIndex, AroundDie eAroundDie) const
{
    if (SizeX == 0 || SizeY == 0)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Wafermap size is incorrect size X (%) and size Y (%)");
        return false;
    }

    int	lColIdx     = lIndex % SizeX;
    int lLineIdx    = lIndex / SizeX;
    int lDieOffset  = -1;

    if (eAroundDie & AdjacentDie)
    {
        // Algorithm checking all 4 dies adjacent to center die
        for(int lColRange = -1; lColRange <= 1; lColRange += 2)
        {
            lDieOffset = (lColIdx + lColRange + (lLineIdx *SizeX));
            if(lDieOffset >= 0 && lDieOffset < SizeX * SizeY)
            {
                // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                if(cWafMap[lDieOffset].getBin() == GEX_WAFMAP_EMPTY_CELL)
                    return true;	// This die exists and is around the center die.
            }
        }

        for(int lLineRange = -1; lLineRange <= 1; lLineRange += 2)
        {
            lDieOffset = (lColIdx + ((lLineIdx + lLineRange) * SizeX));
            if(lDieOffset >= 0 && lDieOffset < SizeX * SizeY)
            {
                // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                if(cWafMap[lDieOffset].getBin() == GEX_WAFMAP_EMPTY_CELL)
                    return true;	// This die exists and is around the center die.
            }
        }
    }

    if (eAroundDie & DiagonalDie)
    {
        // Algorithm checking all 4 dies adjacent to center die
        for(int lColRange = -1; lColRange <= 1; lColRange += 2)
        {
            for(int lLineRange = -1; lLineRange <= 1; lLineRange += 2)
            {
                lDieOffset = (lColIdx + lColRange + (lLineIdx + lLineRange) * SizeX);
                if(lDieOffset >= 0 && lDieOffset < SizeX * SizeY)
                {
                    // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                    if(cWafMap[lDieOffset].getBin() == GEX_WAFMAP_EMPTY_CELL)
                        return true;	// This die exists and is around the center die.
                }
            }
        }
    }

    return false;
}

bool CWaferMap::ComputeReferenceDieLocation(const QString &lRefDieLocation)
{
    QList<GS::Gex::WaferCoordinate> referenceDies;
    QRegExp referenceDieCoordRegExp(REGEXP_REF_LOCATION_FROM_COORD, Qt::CaseInsensitive);
    QRegExp referenceDieBinRegExp(REGEXP_REF_LOCATION_FROM_BIN, Qt::CaseInsensitive);
    QRegExp referenceDiePCMDropOuts(REGEXP_REF_LOCATION_FROM_DROPOUTS, Qt::CaseInsensitive);

    // Check syntax
    if (lRefDieLocation.compare("WCR", Qt::CaseInsensitive) == 0)
    {
        if (GetCenterDie().IsValid())
            referenceDies.append(GetCenterDie());
        else
        {
            return false;
        }
    }
    else if (referenceDieCoordRegExp.exactMatch(lRefDieLocation.trimmed()))
    {
        referenceDies.append(GS::Gex::WaferCoordinate(referenceDieCoordRegExp.capturedTexts().at(1).toInt(),
                                                      referenceDieCoordRegExp.capturedTexts().at(2).toInt()));
    }
    else if (referenceDieBinRegExp.exactMatch(lRefDieLocation.trimmed()))
    {
        bool    lIsInteger;
        int     lBinRef = referenceDieBinRegExp.capturedTexts().at(1).toInt(&lIsInteger);

        if (!lIsInteger)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not a valid bin number (%1) for External reference location")
                          .arg(referenceDieBinRegExp.capturedTexts().at(1)).toLatin1().constData());
            return false;
        }

        // Get the coordinates for this bin.
        // It must have only one coordinate corresponding to this reference bin.
        QList<GS::Gex::WaferCoordinate> coordinates = GetBinCoordinates(lBinRef);

        if (coordinates.count() == 0)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("No coordinate found with bin number (%1) for External reference location")
                  .arg(lBinRef).toLatin1().constData());
            return false;
        }
        else if (coordinates.count() > 1)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Multiple coordinates found with bin number (%1) for External reference location")
                  .arg(lBinRef).toLatin1().constData());

            return false;
        }
        else
        {
            referenceDies.append(coordinates.first());
        }
    }
    else if (referenceDiePCMDropOuts.exactMatch(lRefDieLocation.trimmed()))
    {
        referenceDies = GetDropOuts();

        if (referenceDies.count() <= 1)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough drop outs found as reference die (%1) from external map")
                  .arg(referenceDies.count()).toLatin1().constData());

            return false;
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Reference die location invalid format: %1.")
              .arg(lRefDieLocation).toLatin1().constData());

        return false;
    }

    SetReferenceDieLocation(referenceDies);

    return true;
}

QList<GS::Gex::WaferCoordinate> CWaferMap::GetDropOuts() const
{
    QList<GS::Gex::WaferCoordinate> dropOuts;
    int lDieX;
    int lDieY;

    // Loop on the while wafermap array
    for (int lIdx = 0; lIdx < SizeX * SizeY; ++lIdx)
    {
        // Find any non-tested die
        if (cWafMap[lIdx].getBin() == GEX_WAFMAP_EMPTY_CELL )
        {
            // If non-tested die is inside the wafermap boundaries, it is considered as a drop out
            if (isDieOutOfWafer(lIdx) == false)
            {
                if (coordFromIndex(lIdx, lDieX, lDieY))
                    dropOuts.append(GS::Gex::WaferCoordinate(lDieX, lDieY));
                else
                    GSLOG(SYSLOG_SEV_ERROR,
                             QString("No valid coordinates for index %1 in the map")
                             .arg(lIdx).toLatin1().constData());
            }
        }
    }

    return dropOuts;
}

QList<GS::Gex::WaferCoordinate> CWaferMap::GetBinCoordinates(int lBin) const
{
    QList<GS::Gex::WaferCoordinate> lCoordinates;

    if (cWafMap)
    {
        if (lBin != GEX_WAFMAP_EMPTY_CELL)
        {
            int lDieX;
            int lDieY;

            // Loop on the while wafermap array
            for (int lIdx = 0; lIdx < SizeX * SizeY; ++lIdx)
            {
                // Find a corresponding bin.
                // Append the coordinate of this bin to the list
                if (lBin == cWafMap[lIdx].getBin())
                {
                    if (coordFromIndex(lIdx, lDieX, lDieY))
                        lCoordinates.append(GS::Gex::WaferCoordinate(lDieX, lDieY));
                    else
                        GSLOG(SYSLOG_SEV_ERROR,
                                 QString("No valid coordinates for index %1 in the map")
                                 .arg(lIdx).toLatin1().constData());
                }
            }
        }
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Wafermap array is not allocated.");

    return lCoordinates;
}

const QList<GS::Gex::WaferCoordinate>& CWaferMap::GetReferenceDieLocation() const
{
    return mRefDieLocation;
}

const GS::Gex::WaferCoordinate &CWaferMap::GetCenterDie() const
{
    return mCenterDie;
}

double CWaferMap::GetDiameter() const
{
    return mDiameter;
}

double CWaferMap::GetDieHeight() const
{
    return mDieHeight;
}

double CWaferMap::GetDieWidth() const
{
    return mDieWidth;
}

bool CWaferMap::GetPosXDirection() const
{
    return mPosXDirection;
}

bool CWaferMap::GetPosYDirection() const
{
    return mPosYDirection;
}

void CWaferMap::SetReferenceDieLocation(const QList<GS::Gex::WaferCoordinate>& dieLocation)
{
    mRefDieLocation = dieLocation;
}

void CWaferMap::SetCenterDie(GS::Gex::WaferCoordinate lCenterDie)
{
    mCenterDie = lCenterDie;
}

void CWaferMap::SetDiameter(double lDiameter)
{
    mDiameter = lDiameter;
}

void CWaferMap::SetDieHeight(double lDieHeight)
{
    mDieHeight = lDieHeight;
}

void CWaferMap::SetDieWidth(double lDieWidth)
{
    mDieWidth = lDieWidth;
}

void CWaferMap::SetPosXDirection(bool xDirection)
{
    mPosXDirection = xDirection;
}

void CWaferMap::SetPosYDirection(bool yDirection)
{
    mPosYDirection = yDirection;
}

bool CWaferMap::Translate(int x, int y)
{
    if (!SizeX || !SizeY)
        return false;

    iLowDieX    += x;
    iHighDieX   += x;

    iLowDieY    += y;
    iHighDieY   += y;

    for (int lIdx = 0; lIdx < mRefDieLocation.count(); ++lIdx)
    {
        mRefDieLocation[lIdx].Translate(x, y);
    }

    return true;
}

bool CWaferMap::Reflect(bool lXAxis, bool lYAxis)
{
    if (!SizeX || !SizeY)
        return false;

    // No reflection to apply
    if (lXAxis == false && lYAxis == false)
        return true;

    int lBin;
    int lOrgBin;
    int lIndex = 0;

    // Create teporary buffer to receive new wafermap image
    int                     lWaferSize  = SizeX * SizeY;
    CWafMapArray *          pNewWafMap  = CWafMapArray::allocate(lWaferSize);
    GS::Gex::WaferTransform lWFTransform;

    // Rotate wafermap
    for(int idxY = 0; idxY < SizeY; ++idxY)
    {
        for(int idxX = 0; idxX < SizeX; ++idxX)
        {
            // Original Bin
            lIndex  = idxX + (idxY*SizeX);
            lBin    = cWafMap[lIndex].getBin();
            lOrgBin = cWafMap[lIndex].getOrgBin();

            // Y Axis reflection (change x coordinates)
            if (lYAxis)
                lIndex = SizeX - idxX - 1;
            else
                lIndex = idxX;

            // X Axis reflection (change y coordinates)
            if (lXAxis)
                lIndex += (SizeY - idxY - 1) * SizeX;
            else
                lIndex += idxY * SizeX;

            if (lIndex >= 0 && lIndex < lWaferSize)
            {
                pNewWafMap[lIndex].setBin(lBin);
                pNewWafMap[lIndex].setOrgBin(lOrgBin);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Invalid index computed when applying reflection")
                      .toLatin1().constData());

                delete[] pNewWafMap;
                pNewWafMap = NULL;

                return false;
            }
        }
    }

    // Overwrite old wafermap with new one
    memcpy(cWafMap, pNewWafMap,(SizeX*SizeY)*sizeof(CWafMapArray));

    // Set the origin point for the transformation
    lWFTransform.SetOrigin(GS::Gex::WaferCoordinate(0,0));
    lWFTransform.Reflect(lXAxis, lYAxis);

    // Transform low and high coordinates
    int x1, x2, y1, y2;
    lWFTransform.Map(iLowDieX, iLowDieY, x1, y1);
    lWFTransform.Map(iHighDieX, iHighDieY, x2, y2);

    // Update low/high X and Y
    iLowDieX    = qMin(x1, x2);
    iHighDieX   = qMax(x1, x2);
    iLowDieY    = qMin(y1, y2);
    iHighDieY   = qMax(y1, y2);

    // Update Size X and Y
    SizeX   = iHighDieX - iLowDieX + 1;
    SizeY   = iHighDieY - iLowDieY + 1;

    // Update Reference die locations
    for (int lIdx = 0; lIdx < mRefDieLocation.count(); ++lIdx)
    {
        mRefDieLocation[lIdx] = lWFTransform.Map(mRefDieLocation.at(lIdx));
    }

    // Check if orientation was Left or Right, if so invert orientation
    if (lYAxis)
    {
        if (toupper(cWaferFlat_Active) == 'L')
            cWaferFlat_Active = 'R';
        else if (toupper(cWaferFlat_Active) == 'R')
            cWaferFlat_Active = 'L';

        if (toupper(cWaferFlat_Detected) == 'L')
            cWaferFlat_Detected = 'R';
        else if (toupper(cWaferFlat_Detected) == 'R')
            cWaferFlat_Detected = 'L';
    }

    // Check if orientation was Up or Down, if so invert orientation
    if (lXAxis)
    {
        if (toupper(cWaferFlat_Detected) == 'U')
            cWaferFlat_Detected = 'D';
        else if (toupper(cWaferFlat_Detected) == 'D')
            cWaferFlat_Detected = 'U';

        if (toupper(cWaferFlat_Detected) == 'U')
            cWaferFlat_Detected = 'D';
        else if (toupper(cWaferFlat_Detected) == 'D')
            cWaferFlat_Detected = 'U';
    }

    delete[] pNewWafMap;
    pNewWafMap = NULL;

    return true;
}

bool CWaferMap::Transform(const GS::Gex::WaferTransform &transform)
{
    if (Reflect(transform.GetXReflection(), transform.GetYReflection()) == false)
        return false;

    if (Rotate(transform.GetRotation()) == false)
        return false;

    if (Translate(transform.GetXTranslation(), transform.GetYTranslation()) == false)
        return false;

    return true;
}

CWaferMap::Alignment CWaferMap::CompareAlignment(const CWaferMap &other,
                                                 Alignment alignRule,
                                                 const GS::QtLib::Range &boundaryBins) const
{
    // Check geometry alignment
    if (alignRule & AlignGeometry)
    {
        QRect geometryRef(iLowDieX, iLowDieY, SizeX, SizeY);
        QRect geometryOther(other.iLowDieX, other.iLowDieY, other.SizeX, other.SizeY);

        if (geometryRef.contains(geometryOther) == false)
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Map geometries don't match");

            return AlignGeometry;
        }
    }

    // Check Axis Direction
    if (alignRule & AlignAxisDirection)
    {
        if (GetPosXDirection() != other.GetPosXDirection())
        {
            GSLOG(SYSLOG_SEV_WARNING, "Positive X direction differs");
            return AlignAxisDirection;
        }

        if (GetPosYDirection() != other.GetPosYDirection())
        {
            GSLOG(SYSLOG_SEV_WARNING, "Positive Y direction differs");
            return AlignAxisDirection;
        }
    }

    // Check orientation alignment
    if (alignRule & AlignOrientation)
    {
        if (cWaferFlat_Active == ' ' || other.cWaferFlat_Active == ' ')
        {
            GSLOG(SYSLOG_SEV_ERROR, "One of the map orientation is not defined");

            return AlignOrientation;
        }

        if (cWaferFlat_Active != other.cWaferFlat_Active)
        {
            GSLOG(SYSLOG_SEV_NOTICE, "Map orientation (flat or notch) differs.");

            return AlignOrientation;
        }
    }

    // Check reference die location alignment
    if (alignRule & AlignRefLocation)
    {
        if (GetReferenceDieLocation().count() != other.GetReferenceDieLocation().count())
        {
            GSLOG(SYSLOG_SEV_ERROR, "Reference die location count doesn't match between the two maps");

            return AlignRefLocation;
        }

        QList<GS::Gex::WaferCoordinate> lRefLocation     = GetReferenceDieLocation();
        QList<GS::Gex::WaferCoordinate> lOtherLocation   = other.GetReferenceDieLocation();

        // Sort reference die location, so we can compare each die
        qSort(lRefLocation);
        qSort(lOtherLocation);

        for (int lIdx = 0; lIdx < lRefLocation.count(); ++lIdx)
        {
            if (lRefLocation.at(lIdx).IsValid() == false)
            {
                GSLOG(SYSLOG_SEV_ERROR, "STDF reference die location is not defined");

                return AlignRefLocation;
            }

            if (lOtherLocation.at(lIdx).IsValid() == false)
            {
                GSLOG(SYSLOG_SEV_ERROR, "External Map reference die location is not defined");

                return AlignRefLocation;
            }

            if (lRefLocation.at(lIdx) != lOtherLocation.at(lIdx))
            {
                GSLOG(SYSLOG_SEV_NOTICE, "Reference Die Location differs.");

                return AlignRefLocation;
            }
        }
    }

    // Check boundary bins alignment
    if (alignRule & AlignBoundaryBins)
    {
        // If boundary bins list defined
        if (boundaryBins.IsEmpty() == false)
        {
            int lIdxRef     = 0;
            int lIdxOther   = 0;
            int lBinRef     = GEX_WAFMAP_EMPTY_CELL;
            int lBinOther   = GEX_WAFMAP_EMPTY_CELL;
            int lDieX       = GEX_WAFMAP_INVALID_COORD;
            int lDieY       = GEX_WAFMAP_INVALID_COORD;

            // Loop on all the map data
            for (lIdxRef = 0; lIdxRef < SizeX * SizeY; ++lIdxRef)
            {
                lBinRef  = getWafMap()[lIdxRef].getBin();

                if (lBinRef != GEX_WAFMAP_EMPTY_CELL && boundaryBins.Contains(lBinRef))
                {
                    // Get the coordinate corresponding to the index
                    if (coordFromIndex(lIdxRef, lDieX, lDieY))
                    {
                        // Get the STDF map index correpsonding to the coordinates
                        if (other.indexFromCoord(lIdxOther, lDieX, lDieY))
                        {
                            lBinOther   = other.getWafMap()[lIdxOther].getBin();

                            if (lBinOther != GEX_WAFMAP_EMPTY_CELL)
                            {
                                GSLOG(SYSLOG_SEV_NOTICE,
                                         (QString("Boundary bins overload at [%1,%2].")
                                         .arg(lDieX)
                                         .arg(lDieY)).toLatin1().constData());

                                return AlignBoundaryBins;
                            }
                        }
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_ERROR,
                               QString("No valid coordinates for index %1 in the map")
                              .arg(lIdxRef).toLatin1().constData());

                        return AlignBoundaryBins;
                    }
                }
            }
        }
    }

    return AlignNone;
}

void CWaferMap::GetMatchingDie(int &lMatch, int &lUnmatch, QString lBinList)
{
    // If no Pass/Fail info available, then assume
    // Pass = Bin1, Fail = other bins
    if (lBinList.isEmpty())
        lBinList = "1";

    // Create Range object for fast bin classification
    GS::QtLib::Range * lBinRange = new GS::QtLib::Range(lBinList);

    // Clear variables
    lMatch   = 0;
    lUnmatch = 0;

    // Scan all the wafermap buffer, see how many time bin 'iBin' is found.
    int lBinCode;
    int lWaferSize = SizeX * SizeY;

    for (int lIdx = 0; lIdx < lWaferSize; ++lIdx)
    {
        lBinCode = getWafMap()[lIdx].getBin();

        switch (lBinCode)
        {
            case GEX_WAFMAP_EMPTY_CELL:  // -1: Die not tested
                break;

            default:  // valid bin
                if (lBinRange->Contains(lBinCode))
                    ++lMatch;
                else
                    ++lUnmatch;
                break;
        }
    }

    // Free memory
    delete lBinRange;
    lBinRange = NULL;
}

bool CWaferMap::UpdateBin(int lBin, int lX, int lY)
{
    int lIdx = -1;

    // Overload binning with PAT failure
    if (indexFromCoord(lIdx, lX, lY))
    {
        cWafMap[lIdx].setBin(lBin);
        return true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING,
               QString("Invalid coordinates found in wafermap [%1,%2]")
               .arg(lX).arg(lY).toLatin1().constData());

        return false;
    }
}

void CWaferMap::UpdateActiveOrientation()
{
    if (mVisualOptions & SwapNotch)
        SwapWaferNotchPosition();

    if (!SizeX || !SizeY)
        cWaferFlat_Active = 'D';
    else
    {
        if (cWaferFlat_Detected == ' ')
        {
            // DETECT FLAT!
            switch(DetectFlatLocation())
            {
                case 3:
                    cWaferFlat_Detected='R';
                    break;
                case 9:
                    cWaferFlat_Detected='L';
                    break;
                case 12:
                    cWaferFlat_Detected='U';
                    break;
                default:
                case 6:
                    cWaferFlat_Detected='D';
            }
        }

        if (cWaferFlat_Active == ' ')
        {
            // Check notch options to assign active notch
            QString strOptionNL = (gexReport->getReportOptions()->GetOption("wafer","notch_location")).toString();
            QString strOptionDN = (gexReport->getReportOptions()->GetOption("wafer","default_notch")).toString();

            if(strOptionNL == "file_or_detected")
            {
                if(cWaferFlat_Stdf== ' ')
                    cWaferFlat_Active = cWaferFlat_Detected;
                else
                    cWaferFlat_Active = cWaferFlat_Stdf;
            }
            else if(strOptionNL == "file_or_default")
            {
                if(cWaferFlat_Stdf== ' ')
                    cWaferFlat_Active = strOptionDN.at(0).toLatin1();
                else
                    cWaferFlat_Active = cWaferFlat_Stdf;
            }
            else if(strOptionNL == "default")
            {
                cWaferFlat_Active = strOptionDN.at(0).toLatin1();
            }
            else // default is detected
                cWaferFlat_Active = cWaferFlat_Detected;
        }
    }
}

bool CWaferMap::Rotate(int quarters)
{
    if (!SizeX || !SizeY)
        return false;

    quarters = quarters % 4;

    // No rotation required.
    if (quarters == 0)
        return true;

    int lBin;
    int lOrgBin;
    int lIndex = 0;

    // Create temporary buffer to receive new wafermap image
    int                     lWaferSize  = SizeX * SizeY;
    CWafMapArray *          pNewWafMap  = CWafMapArray::allocate(lWaferSize);
    GS::Gex::WaferTransform lWFTransform;

    // Rotate wafermap
    for(int idxY = 0; idxY < SizeY; ++idxY)
    {
        for(int idxX = 0; idxX < SizeX; ++idxX)
        {
            // Original Bin
            lIndex  = idxX + (idxY*SizeX);
            lBin    = cWafMap[lIndex].getBin();
            lOrgBin = cWafMap[lIndex].getOrgBin();

            // clockwise rotation
            if (quarters == 1 || quarters == -3)
                lIndex = idxX * SizeY + (SizeY - idxY - 1);
            else if (quarters == 2 || quarters == -2)
                lIndex = (SizeX - idxX -1)  + ((SizeY - idxY - 1) * SizeX);
            else if (quarters == 3 || quarters == -1)
                lIndex = ((SizeX - idxX -1) * SizeY) + (idxY);

            if (lIndex >= 0 && lIndex < lWaferSize)
            {
                pNewWafMap[lIndex].setBin(lBin);
                pNewWafMap[lIndex].setOrgBin(lOrgBin);
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Invalid index computed when applying rotation of %1")
                      .arg(quarters * 90).toLatin1().constData());

                delete[] pNewWafMap;
                pNewWafMap = NULL;

                return false;
            }
        }
    }

    // Overwrite old wafermap with new one
    memcpy(cWafMap, pNewWafMap,(SizeX*SizeY)*sizeof(CWafMapArray));

    // Set the origin point for the transformation
    lWFTransform.SetOrigin(GS::Gex::WaferCoordinate(0,0));
    lWFTransform.Rotate(quarters);

    // Transform low and high coordinates
    int x1, x2, y1, y2;
    lWFTransform.Map(iLowDieX, iLowDieY, x1, y1);
    lWFTransform.Map(iHighDieX, iHighDieY, x2, y2);

    // Update low/high X and Y
    iLowDieX    = qMin(x1, x2);
    iHighDieX   = qMax(x1, x2);
    iLowDieY    = qMin(y1, y2);
    iHighDieY   = qMax(y1, y2);

    // Update Size X and Y
    SizeX   = iHighDieX - iLowDieX + 1;
    SizeY   = iHighDieY - iLowDieY + 1;

    // Update Reference die locations
    for (int lIdx = 0; lIdx < mRefDieLocation.count(); ++lIdx)
    {
        mRefDieLocation[lIdx] = lWFTransform.Map(mRefDieLocation.at(lIdx));
    }

    if (quarters < 0)
        quarters += 4;

    // Rotate Active orientation
    char lWaferOrientation = ' ';

    // Get normalized orientation
    lWaferOrientation = CWaferMap::ToggleOrientation(cWaferFlat_Active, mPosXDirection, mPosYDirection);
    // Apply rotation
    lWaferOrientation = CWaferMap::RotateOrientation(lWaferOrientation, quarters);
    // Toogle normalized orientation to current viewport orientation
    cWaferFlat_Active = CWaferMap::ToggleOrientation(lWaferOrientation, mPosXDirection, mPosYDirection);

    // Get normalized orientation
    lWaferOrientation = CWaferMap::ToggleOrientation(cWaferFlat_Detected, mPosXDirection, mPosYDirection);
    // Apply rotation
    lWaferOrientation = CWaferMap::RotateOrientation(lWaferOrientation, quarters);
    // Toogle normalized orientation to current viewport orientation
    cWaferFlat_Detected = CWaferMap::ToggleOrientation(lWaferOrientation, mPosXDirection, mPosYDirection);

    delete[] pNewWafMap;
    pNewWafMap = NULL;

    return true;
}

///////////////////////////////////////////////////////////
bool CWaferMap::isDieInsideRing(Ring eRing, int nWidth, int nIndex) const
{
    int nCoordX;
    int nCoordY;

    // Get coordinates from index.
    if (coordFromIndex(nIndex, nCoordX, nCoordY))
        return isDieInsideRing(eRing, nWidth, nCoordX, nCoordY);
    else
        return false;
}

///////////////////////////////////////////////////////////
bool CWaferMap::isDieInsideRing(Ring eRing, int nWidth, int nDieX, int nDieY) const
{
    bool	bInside     = false;
    int		nDieXCenter = SizeX/2;
    int		nDieYCenter	= SizeY/2;
    int		nArrayIndex = 0;
    int     lXInc       = 0;
    int     lYInc       = 0;

    if (nWidth < 0)
        return true;

    if (indexFromCoord(nArrayIndex, nDieX, nDieY))
    {
        switch (eRing)
        {
            // Use a trigonometric formula to find out if the die is inside a circle with the given radius.
            case RingFromCenter		:
                bInside = (GS_POW((nDieX - iLowDieX - nDieXCenter), 2.0) + GS_POW((nDieY - iLowDieY - nDieYCenter), 2.0) <= GS_POW(nWidth, 2.0));
                break;

            // Try to find out if the die is in an edge ring of a given width
            case RingFromEdge		:
                {
                    int nIndex      = 0;

                    // Check to which quarter the die belongs
                    if ((nDieX - iLowDieX) < nDieXCenter)
                    {
                        // Left part of the wafer
                        if ((nDieX - nWidth) < iLowDieX)
                            return true;

                        lXInc = -1;
                    }
                    else
                    {
                        // Right part of the wafer
                        if ((nDieX + nWidth) > iHighDieX)
                            return true;

                        lXInc = 1;
                    }

                    if ((nDieY - iLowDieY) < nDieYCenter)
                    {
                        // Top part of the wafer
                        if ((nDieY - nWidth) < iLowDieY)
                            return true;

                        lYInc = -SizeX;
                    }
                    else
                    {
                        // Bottom part of the wafer
                        if ((nDieY + nWidth) > iHighDieY)
                            return true;

                        lYInc = SizeX;
                    }

                    while (nIndex < nWidth && bInside == false)
                    {
                        if (isDieOnEdge(nArrayIndex, EdgeDieBoth))
                            bInside = true;
                        else
                            nArrayIndex += lXInc + lYInc;

                        nIndex++;
                    }
                }
                break;

            case NoRing:
                bInside = true;
                break;

            default:
                break;
        }
    }

    return bInside;
}

///////////////////////////////////////////////////////////
CWaferMap& CWaferMap::operator=( const CWaferMap &s )	// assignment operator
{
    if(this != &s)
    {
        // Duplicate Wafermap array
        int	iWaferArraysize = s.SizeX*s.SizeY;
        if(cWafMap)
            delete [] cWafMap;
        freeCellTestCounter();

        cWafMap = CWafMapArray::allocate(iWaferArraysize, ((CWaferMap &)s).getWafMap());

        m_piCellTestCounter = allocCellTestCounter(((CWaferMap &)s).getCellTestCounterSize());
        for(int iIdx=0; iIdx<((CWaferMap &)s).getCellTestCounterSize(); iIdx++)
            m_piCellTestCounter[iIdx] = ((CWaferMap &)s).getCellTestCounter()[iIdx];

        /* HTH - case 4156 - Catch memory allocation exception
 cWafMap = new CWafMapArray[iWaferArraysize];			// Pointer to wafermap BIN results array
 memcpy(cWafMap,s.getWafMap(),iWaferArraysize*sizeof(CWafMapArray));
 */

        iLowDieX    = s.iLowDieX;
        iHighDieX   = s.iHighDieX;
        iLowDieY    = s.iLowDieY;
        iHighDieY   = s.iHighDieY;
        SizeX       = s.SizeX;
        SizeY       = s.SizeY;                      // Wafermap size (cells in X, and Y)
        iTotalPhysicalDies = s.iTotalPhysicalDies;	// Total number of physical dies tested on the wafer (dosn't counts retests)

        cPos_X      = s.cPos_X;
        cPos_Y      = s.cPos_Y;

        lTestNumber = s.lTestNumber;

        lWaferStartTime = s.lWaferStartTime;
        lWaferEndTime = s.lWaferEndTime;
        strcpy(szWaferID,s.szWaferID);
        strcpy(szWaferIDFilter,s.szWaferIDFilter);	// Holds the only WaferID to process if multiple wafers in same file. Empty if no filter
        bWaferUnits = s.bWaferUnits;				// WF_UNITS:0 (unknown) 1(inches) 2(cm) 3(mm) 4(mils)
        cWaferFlat_Stdf = s.cWaferFlat_Stdf;		// STDF WF_FLAT: U, D, R or L (notch direction)
        cWaferFlat_Detected = s.cWaferFlat_Detected;// Detected WF_FLAT: U, D, R or L (notch direction)
        cWaferFlat_Active = s.cWaferFlat_Active;	// Active WF_FLAT: U, D, R or L (notch direction)
        bPartialWaferMap = s.bPartialWaferMap;		//=TRUE if data outside of buffer allocated
        bStripMap = s.bStripMap;					//=TRUE if we have to build a StripMap (final test wafer-map)
        bWaferMapExists = s.bWaferMapExists;		//=TRUE if Wafer map data exist
        bWirExists = s.bWirExists;					//=TRUE if Wafer map WIR record exists (so valid Wafer sort data file)
        bFirstSubLotInFile = s.bFirstSubLotInFile;	//=TRUE if this Wafer/Sub-lot is the first one found in the file processed.

        mCenterDie      = s.mCenterDie;
        mRefDieLocation = s.mRefDieLocation;
        mPosXDirection  = s.mPosXDirection;
        mPosYDirection  = s.mPosYDirection;

        mDiameter       = s.mDiameter;
        mDieHeight      = s.mDieHeight;
        mDieWidth       = s.mDieWidth;
        mReticleMinX    = s.mReticleMinX;
        mReticleMaxX    = s.mReticleMaxX;
        mReticleMinY    = s.mReticleMinY;
        mReticleMaxY    = s.mReticleMaxY;
    }

    return *this;
}

///////////////////////////////////////////////////////////
// Detect Wafer Notch position
// 12 = 12Hour = UP
// 3 = 3Hour = RIGHT
// 6 = 6Hour = DOWN (default)
// 9 = 9Hour = LEFT
///////////////////////////////////////////////////////////
int CWaferMap::GetWaferNotch()
{
    // If Wafer notch/flat already assigned, use it
    switch(cWaferFlat_Active)
    {
        case 'D':
        case 'd':
            return 6;	// Notch = DOWN (6 Hour)
        case 'L':
        case 'l':
            return 9;	// Notch = LEFT (9 Hour)
        case 'U':
        case 'u':
            return 12;	// Notch = TOP (12 Hour)
        case 'R':
        case 'r':
            return 3;	// Notch = RIGHT (3 Hour)
    }

    // If no wafer data available, say notch is at 6Hour.
    if(!SizeX || !SizeY)
    {
        return 6;
    }

    // DETECT FLAT!
    switch(DetectFlatLocation())
    {
    case 3:
        cWaferFlat_Detected='R';
        break;
    case 9:
        cWaferFlat_Detected='L';
        break;
    case 12:
        cWaferFlat_Detected='U';
        break;
    default:
    case 6:
        cWaferFlat_Detected='D';
    }

    // Check notch options to assign active notch
    QString strOptionNL = (gexReport->getReportOptions()->GetOption("wafer","notch_location")).toString();
    QString strOptionDN = (gexReport->getReportOptions()->GetOption("wafer","default_notch")).toString();

    if(strOptionNL == "file_or_detected")
    {
        if(cWaferFlat_Stdf== ' ')
            cWaferFlat_Active = cWaferFlat_Detected;
        else
            cWaferFlat_Active = cWaferFlat_Stdf;
    }
    else if(strOptionNL == "file_or_default")
    {
        if(cWaferFlat_Stdf== ' ')
            cWaferFlat_Active = strOptionDN.at(0).toLatin1();
        else
            cWaferFlat_Active = cWaferFlat_Stdf;
    }
    else if(strOptionNL == "default")
    {
        cWaferFlat_Active = strOptionDN.at(0).toLatin1();
    }
    else // default is detected
        cWaferFlat_Active = cWaferFlat_Detected;

    // Return assigned notch
    switch(cWaferFlat_Active)
    {
        case 'D':
        case 'd':
            return 6;	// Notch = DOWN (6 Hour)
        case 'L':
        case 'l':
            return 9;	// Notch = LEFT (9 Hour)
        case 'U':
        case 'u':
            return 12;	// Notch = TOP (12 Hour)
        case 'R':
        case 'r':
            return 3;	// Notch = RIGHT (3 Hour)
    }

    return 6;
}

///////////////////////////////////////////////////////////
// Detect Wafer Flat location
// 12 = 12Hour = UP
// 3 = 3Hour = RIGHT
// 6 = 6Hour = DOWN (default)
// 9 = 9Hour = LEFT
///////////////////////////////////////////////////////////
int CWaferMap::DetectFlatLocation()
{
    int		iLine,iCol,iTemp;
    int		iTop,iRight,iDown,iLeft;

    // DETECT FLAT!
    // Count total dies on the Top edge
    iLine   = 0;	// First wafer line to analyze

    do
    {
        iTop    = 0;	// Clear die count on Top edge
        for(iCol=0;iCol < SizeX;iCol++)
        {
            if(cWafMap[(iCol+(iLine*SizeX))].getBin() != GEX_WAFMAP_EMPTY_CELL)
                iTop++;
        }

        // Move to next line in case this wafer lline was totally empty (no die at all tested)
        iLine++;
    }
    while(!iTop && iLine < SizeY);

    // iTop == 0 means that the wafermap is totally empty. Default noth orientation is 6 hour (Down).
    if (iTop == 0)
        return 6;	// Notch = DOWN (6 Hour)

    // Count total dies on the Bottom (Down) edge
    iLine = SizeY-1;	// First wafer line to analyze
    do
    {
        iDown = 0;	// Clear die count on Top edge
        for(iCol=0;iCol < SizeX;iCol++)
        {
            if(cWafMap[(iCol+(iLine*SizeX))].getBin() != GEX_WAFMAP_EMPTY_CELL)
                iDown++;
        }

        // Move to lower line in case this wafer lline was totally empty (no die at all tested)
        iLine--;
    }
    while(!iDown && iLine >= 0);

    // Count total dies on the Left edge
    iCol= 0;	// First wafer line to analyze
    do
    {
        iLeft = 0;	// Clear die count on Top edge
        for(iLine=0;iLine < SizeY;iLine++)
        {
            if(cWafMap[(iCol+(iLine*SizeX))].getBin() != GEX_WAFMAP_EMPTY_CELL)
                iLeft++;
        }

        // Move to next col in case this wafer col was totally empty (no die at all tested)
        iCol++;
    }
    while(!iLeft && iCol < SizeX);

    // Count total dies on the Right edge
    iCol= SizeX-1;	// First wafer line to analyze
    do
    {
        iRight = 0;	// Clear die count on Top edge
        for(iLine=0;iLine < SizeY;iLine++)
        {
            if(cWafMap[(iCol+(iLine*SizeX))].getBin() != GEX_WAFMAP_EMPTY_CELL)
                iRight++;
        }

        // Move to next line in case this wafer lline was totally empty (no die at all tested)
        iCol--;
    }
    while(!iRight  && iCol >= 0);

    // Check wafermap orientation
    if(mPosYDirection == false)
    {
        // Positive Y as we go to the UP direction: switch top/bottom.
        iTemp = iTop;
        iTop = iDown;
        iDown = iTemp;
    }
    if(mPosXDirection == false)
    {
        // Positive X as we go to LEFT direction: switch left/right.
        iTemp = iLeft;
        iLeft = iRight;
        iRight = iTemp;
    }

    // Check which direction holds the Notch: MUST check 'Down' notch position first!
    if(iDown >= iTop && iDown >= iRight && iDown >= iLeft)
        return 6;	// Notch = DOWN (6 Hour)
    if(iTop >= iRight && iTop >= iDown && iTop >= iLeft)
        return 12;	// Notch = TOP (12 Hour)
    if(iRight >= iTop && iRight >= iDown && iRight >= iLeft)
        return 3;	// Notch = RIGHT (3 Hour)

    // Else: Notch = LEFT (9 Hour)
    return 9;
}

char CWaferMap::RotateOrientation(char lOrientation, int aQuarters)
{
    // Convert orientation to angle in degree
    int lDegree = CWaferMap::OrientationToDegree(lOrientation);

    // Apply clockwise rotation
    lDegree += aQuarters * 90;
    lDegree = lDegree % 360;

    return CWaferMap::OrientationFromDegree(lDegree);
}

double CWaferMap::ConvertToMM(double lValue, int lWaferUnits)
{
    ///////////////////////////////////////////////////////////
    // Returns a wafer size info in milli-meters
    ///////////////////////////////////////////////////////////
    // Display wafer size in mm
    switch(lWaferUnits)
    {
        case 1:	// units=Inches
            lValue *= 25.4;// Convert inches to mm
            break;

        case 4:	// units=Milli-Inches
            lValue *= 0.254;// Convert Milli-inches to mm
            break;

        case 2:	// units=cm
            lValue *= 10.0;// Convert cm to mm
            break;

        case 0:	// Unknown units
        case 3:	// units=mm
            break;
    }

    // Return scales value.
    return lValue;
}

char CWaferMap::ToggleOrientation(char aOrientation, bool aPosX, bool aPosY)
{
    char lToggledOrientation = aOrientation;

    if (aPosX == false)
    {
        switch(aOrientation)
        {
            case 'L':
            case 'l':
                lToggledOrientation = 'R';
                break;
            case 'R':
            case 'r':
                lToggledOrientation = 'L';
                break;
            default:
                break;
        }
    }

    if (aPosY == false)
    {
        switch(aOrientation)
        {
            case 'U':
            case 'u':
                lToggledOrientation = 'D';
                break;
            case 'D':
            case 'd':
                lToggledOrientation = 'U';
                break;
            default:
                break;
        }
    }

    return lToggledOrientation;
}

int CWaferMap::OrientationToDegree(char aOrientation)
{
    int lOrientation = 270;

    switch(aOrientation)
    {
        case 'R':
        case 'r':
            lOrientation = 0;
            break;
        case 'D':
        case 'd':
            lOrientation = 90;
            break;
        case 'L':
        case 'l':
            lOrientation = 180;
            break;
        default:
        case 'U':
        case 'u':
            lOrientation = 270;
            break;
    }

    return lOrientation;
}

char CWaferMap::OrientationFromDegree(int aDegree)
{
    int lOrientation = ' ';

    switch(aDegree)
    {
        case 0:
            lOrientation = 'R';
            break;
        case 90:
            lOrientation = 'D';
            break;
        case 180:
            lOrientation = 'L';
            break;
        case 270:
            lOrientation = 'U';
            break;
        default:
            break;
    }

    return lOrientation;
}

///////////////////////////////////////////////////////////
// Rotate Wafer by +90degres
///////////////////////////////////////////////////////////
void CWaferMap::RotateWafer()
{
    // Do nothing if now wafer data available!
    if(!SizeX || !SizeY)
        return;

    int		iX_Org, iX_New, iY_Org, iY_New;
    int		iSizeX_New = SizeY, iSizeY_New = SizeX;
    int		iBin,iOrgBin;

    // Create teporary buffer to receive new wafermap image
    CWafMapArray *cNewWafMap = CWafMapArray::allocate(SizeX*SizeY);
    /* HTH - case 4156 - Catch memory allocation exception
    CWafMapArray *cNewWafMap = new CWafMapArray[SizeX*SizeY];
    */

    // Rotate wafermap
    for(iY_Org = 0; iY_Org < SizeY; iY_Org++)
    {
        for(iX_Org = 0; iX_Org < SizeX; iX_Org++)
        {
            // Original Bin
            iBin = cWafMap[(iX_Org+(iY_Org*SizeX))].getBin();
            iOrgBin = cWafMap[(iX_Org+(iY_Org*SizeX))].getOrgBin();

            // Rotated location
            iX_New = iY_Org;
            iY_New = (SizeX-1) - iX_Org;
            cNewWafMap[(iX_New+(iY_New*iSizeX_New))].setBin(iBin);
            cNewWafMap[(iX_New+(iY_New*iSizeX_New))].setOrgBin(iOrgBin);
        }
    }

    // Overwrite old wafermap with new one
    memcpy(cWafMap,cNewWafMap,(SizeX*SizeY)*sizeof(CWafMapArray));

    // Set new dimensions and Low/High values (keep LowX, HighY)
    SizeX = iSizeX_New;
    SizeY = iSizeY_New;
    iHighDieX = iLowDieX + iSizeX_New - 1;
    iLowDieY = iHighDieY - iSizeY_New + 1;

    std::swap(mDieHeight, mDieWidth);

    // Set new Pos_X, Pos_Y
    // It is the WaferMap image, after POS_X and POS_Y have been applied, that has to be rotated.
    // The above rotation was done on the normalized wafermap (X increasing right, Y increasing up)
    // To make sure the final image is rotated, POS_X and POS_Y have to be modified following way:
    // POS_X='R' (bPosX=True)  AND POS_Y='U' (bPosY=False)  => unchanged
    // POS_X='L' (bPosX=False) AND POS_Y='U' (bPosY=False)  => POS_X='R' (bPosX=True)  AND POS_Y='D' (bPosY=True)
    // POS_X='R' (bPosX=True)  AND POS_Y='D' (bPosY=True)   => POS_X='L' (bPosX=False) AND POS_Y='U' (bPosY=False)
    // POS_X='L' (bPosX=False) AND POS_Y='D' (bPosY=True)   => unchanged
    if(mPosXDirection && mPosYDirection)
        mPosXDirection = mPosYDirection = false;
    else if(!mPosXDirection && !mPosYDirection)
        mPosXDirection = mPosYDirection = true;

    // Update Wafer flat info as of +90° rotatation.
    switch(cWaferFlat_Active)
    {
        default:
        case 'D':
        case 'd':
            cWaferFlat_Active = 'L';
            break;
        case 'L':
        case 'l':
            cWaferFlat_Active = 'U';
            break;
        case 'U':
        case 'u':
            cWaferFlat_Active = 'R';
            break;
        case 'R':
        case 'r':
            cWaferFlat_Active = 'D';
            break;
    }
    switch(cWaferFlat_Detected)
    {
        default:
        case 'D':
        case 'd':
            cWaferFlat_Detected = 'L';
            break;
        case 'L':
        case 'l':
            cWaferFlat_Detected = 'U';
            break;
        case 'U':
        case 'u':
            cWaferFlat_Detected = 'R';
            break;
        case 'R':
        case 'r':
            cWaferFlat_Detected = 'D';
            break;
    }

#if 0
    int	iLine,iCol,iBin,iOrgBin;
    int	iMapedLine,iMapedCol;

    // Create teporary buffer to receive new wafermap image
    CWafMapArray *cNewWafMap = new CWafMapArray[SizeX*SizeY];

    for(iLine = 0; iLine < SizeY; iLine++)
    {
        for(iCol = 0; iCol < SizeX; iCol++)
        {
            // Original Bin
            iBin = cWafMap[(iCol+(iLine*SizeX))].iBin;
            iOrgBin = cWafMap[(iCol+(iLine*SizeX))].getOrgBin();

            // Rotated location
            iMapedCol = SizeY-iLine-1;
            iMapedLine = iCol;
            cNewWafMap[(iMapedCol+(iMapedLine*SizeY))].iBin = iBin;
            cNewWafMap[(iMapedCol+(iMapedLine*SizeY))].setOrgBin ( iOrgBin);
        }
    }

    // Overwrite old wafermap with new one
    memcpy(cWafMap,cNewWafMap,(SizeX*SizeY)*sizeof(CWafMapArray));
    // Swap SizeX and SizeY
    int iSave = SizeX;
    SizeX = SizeY;
    SizeY = iSave;
#endif

    delete [] cNewWafMap;
}

///////////////////////////////////////////////////////////
// Check the wafer flat read from stdf file is valid
///////////////////////////////////////////////////////////
bool CWaferMap::isValidWaferFlatOrigin() const
{
    bool bValid = false;

    switch(toupper(cWaferFlat_Stdf))
    {
        case 'R'	:
        case 'U'	:
        case 'D'	:
        case 'L'	:	bValid = true;
                        break;

        default		:	break;
    }

    return bValid;
}

unsigned int CWaferMap::GetReticleWidth() const
{
    if (mReticleMaxX > mReticleMinX)
        return (mReticleMaxX - mReticleMinX + 1);
    else
        return 0;
}

unsigned int CWaferMap::GetReticleHeight() const
{
    if (mReticleMaxY > mReticleMinY)
        return (mReticleMaxY - mReticleMinY + 1);
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	std::vector<int> aroundDieCoordinate(int nIndex, AroundDie eAroundDie /*= BothDie*/, int nRing /*= 0*/, int nWidth /*= 1*/)
//
// Description	:	Get valid coordinate around a die
//
///////////////////////////////////////////////////////////////////////////////////
std::vector<int> CWaferMap::aroundDieCoordinate(int nIndex, AroundDie eAroundDie /*= BothDie*/, int nRing /*= 0*/, int nWidth /*= 1*/, bool bKeepInvalid /*= false*/) const
{
    std::vector<int>	vecCoordinate;
    int					nDieIndex;
    int					nXPos       = nIndex % SizeX;
    int                 nWaferSize	= SizeX * SizeY;

    if (eAroundDie & AdjacentDie)
    {
        for (int nBorder = 1 + nRing; nBorder <= (nRing + nWidth); nBorder++)
        {
            // Not the first on a line
            if (nXPos >= nBorder)
            {
                // left die
                nDieIndex = nIndex - nBorder;
                vecCoordinate.push_back(nDieIndex);
            }
            else if (bKeepInvalid)
                vecCoordinate.push_back(-1);

            // up die
            nDieIndex = nIndex - (nBorder * SizeX);
            if (nDieIndex >= 0)
                vecCoordinate.push_back(nDieIndex);
            else if (bKeepInvalid)
                vecCoordinate.push_back(-1);

            // down die
            nDieIndex = nIndex + (nBorder * SizeX);
            if (nDieIndex < nWaferSize)
                vecCoordinate.push_back(nDieIndex);
            else if (bKeepInvalid)
                vecCoordinate.push_back(-1);

            // not the last on a line
            if (nXPos < SizeX - nBorder)
            {
                // right die
                nDieIndex = nIndex + nBorder;
                vecCoordinate.push_back(nDieIndex);
            }
            else if (bKeepInvalid)
                vecCoordinate.push_back(-1);
        }
    }

    if (eAroundDie & DiagonalDie)
    {
        for (int nBorderX = 1; nBorderX <= (nRing + nWidth); nBorderX++)
        {
            for (int nBorderY = 1; nBorderY <= (nRing + nWidth); nBorderY++)
            {
                if (nBorderX > nRing || nBorderY > nRing)
                {
                    // Not the first on a line
                    if (nXPos >= nBorderX)
                    {
                        // up left die
                        nDieIndex = nIndex - nBorderX - (nBorderY * SizeX);
                        if (nDieIndex >= 0 )
                            vecCoordinate.push_back(nDieIndex);
                        else if (bKeepInvalid)
                            vecCoordinate.push_back(-1);

                        // down left die
                        nDieIndex = nIndex - nBorderX + (nBorderY * SizeX);
                        if (nDieIndex < nWaferSize)
                            vecCoordinate.push_back(nDieIndex);
                        else if (bKeepInvalid)
                            vecCoordinate.push_back(-1);
                    }
                    else if (bKeepInvalid)
                    {
                        vecCoordinate.push_back(-1);
                        vecCoordinate.push_back(-1);
                    }


                    // not the last on a line
                    if (nXPos < SizeX - nBorderX)
                    {
                        // up right die
                        nDieIndex = nIndex + nBorderX - (nBorderY * SizeX);
                        if (nDieIndex >= 0 )
                            vecCoordinate.push_back(nDieIndex);
                        else if (bKeepInvalid)
                            vecCoordinate.push_back(-1);

                        // down right die
                        nDieIndex = nIndex + nBorderX + (nBorderY * SizeX);
                        if (nDieIndex < nWaferSize)
                            vecCoordinate.push_back(nDieIndex);
                        else if (bKeepInvalid)
                            vecCoordinate.push_back(-1);
                    }
                    else if (bKeepInvalid)
                    {
                        vecCoordinate.push_back(-1);
                        vecCoordinate.push_back(-1);
                    }
                }
            }
        }
    }

    return vecCoordinate;
}

///////////////////////////////////////////////////////////
// Includes all data related to the MERGED WaferMaps in
// a same group (query dataset)
///////////////////////////////////////////////////////////
CStackedWaferMap::CStackedWaferMap()
{
    cWafMap = NULL;
    pGexBinList = NULL;

    // Reset variables.
    clear();
}

///////////////////////////////////////////////////////////
void CStackedWaferMap::clear(void)
{
    iTotalWafermaps		= 0;
    iTotaltime			= 0;
    iHighestDieCount	= 0;
    iTotalPhysicalDies	= 0;	// Total number of physical dies tested on the wafer (dosn't counts retests)
    bPartialWaferMap	= FALSE;
    bStripMap			= FALSE;
    bWaferMapExists		= FALSE;
    iLowDieX			= 33000;
    iHighDieX			= -33000;
    iLowDieY			= 33000;
    iHighDieY			= -33000;
    lfLowWindow			= C_INFINITE;
    lfHighWindow		= -C_INFINITE;

    // Handle to list of binnings to merge.
    if(pGexBinList != NULL)
    {
        delete pGexBinList;
        pGexBinList = NULL;
    }

    bPosX				= TRUE;	// TRUE if no Mirror when reading X line
    bPosY				= TRUE;	// TRUE if no Mirror when reading Y line

    // Destroy stacked wafer map buffer
    if(cWafMap != NULL)
    {
        delete [] cWafMap;
        cWafMap=NULL;
    }
}

///////////////////////////////////////////////////////////
CStackedWaferMap::~CStackedWaferMap()
{
    clear();
}

void CWafMapArray::setBin(int iVal)
{
    mBin = iVal;
}

int CWafMapArray::getBin() const
{
    return mBin;
}

void CWafMapArray::setOrgBin(int iVal)
{
    mOriginalBin = iVal;
}

int CWafMapArray::getOrgBin() const
{
    return mOriginalBin;
}

void CWafMapArray::setValue(double dVal)
{
    mValue = dVal;
}

double CWafMapArray::getValue() const
{
    return mValue;
}

/******************************************************************************!
 * \fn setReticlePosX
 ******************************************************************************/
void CWafMapArray::SetReticleDieX(int x)
{
    mReticleDieX = x;
}

/******************************************************************************!
 * \fn setReticlePosY
 ******************************************************************************/
void CWafMapArray::SetReticleDieY(int y)
{
    mReticleDieY = y;
}

/******************************************************************************!
 * \fn getReticlePosX
 ******************************************************************************/
int CWafMapArray::GetReticleDieX()
{
    return mReticleDieX;
}

/******************************************************************************!
 * \fn getReticlePosY
 ******************************************************************************/
int CWafMapArray::GetReticleDieY()
{
    return mReticleDieY;
}

void CWafMapArray::SetReticlePosX(int x)
{
    mReticlePosX = x;
}

void CWafMapArray::SetReticlePosY(int y)
{
    mReticlePosY = y;
}

int CWafMapArray::GetReticlePosX()
{
    return mReticlePosX;
}

int CWafMapArray::GetReticlePosY()
{
    return mReticlePosY;
}

int *CWaferMap::getCellTestCounter(){
    return m_piCellTestCounter;
}

int CWaferMap::getCellTestCounterSize(){
    return m_iCellTestCounterSize;
}

int *CWaferMap::allocCellTestCounter(int iSize , int *pCellCounter){

    freeCellTestCounter();

    quint64		uiMemorySizeLimit		= 2UL * 1024UL * 1024UL * 1024UL; // Max memory to allocate (defined value is 2GB)
    quint64		uiMemorySizeToAllocate	= quint64(iSize) * sizeof(int);

    try
    {
        // Do not try to allocate to much memory, behave like the system cannot allocate memory
        if (uiMemorySizeToAllocate > uiMemorySizeLimit)
        {
            GSLOG(SYSLOG_SEV_CRITICAL, (QString("trying to allocate too much memory : %1, max is %2")
                  .arg( uiMemorySizeToAllocate).arg( uiMemorySizeLimit)).toLatin1().constData());
            throw std::bad_alloc();
        }

        m_piCellTestCounter = new int [iSize];
        memset(m_piCellTestCounter, 0, iSize * sizeof(int));
        m_iCellTestCounterSize = iSize;
        if(pCellCounter)
            memcpy(m_piCellTestCounter, pCellCounter, iSize * sizeof(int));
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
        m_piCellTestCounter = 0;
    }
    catch(...)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
        m_piCellTestCounter = 0;
    }

    return m_piCellTestCounter;
}

void CWaferMap::freeCellTestCounter()
{
    if(m_piCellTestCounter)
        delete []m_piCellTestCounter;
    m_piCellTestCounter = 0;
    m_iCellTestCounterSize = 0;
}

const CWafMapArray * CWaferMap::getWafMap() const
{
    return cWafMap;
}

CWafMapArray *CWaferMap::getWafMap(){
    return cWafMap;
}

CWafMapArray CWaferMap::getWafMapDie(int index) const
{
    // to do: check the index
    return cWafMap[index];
}

void CWaferMap::setWaferMap(CWafMapArray *poWafer){
    cWafMap = poWafer;
}

bool CWaferMap::HasReticle() const
{
    return (GetReticleHeight() > 0 && GetReticleWidth() > 0);
}
