
#include <QFile>
#include <QFileInfo>
#include "stdfparse.h"
#include "importWoburnSECSIIMap.h"

/**
 *WaferId: 6929559-8  [<lot_id>-<wafer_num>] [STDF - <lot_id> MIR.lot_id;  <lot_id>-<wafer_num> MIR.sblot_id, WIR.wafer_id, WCR.wafer_id, WRR.wafer_id]
Orient: East [STDF - WCR.WF_FLAT North=U, South=D, East=R, West=L, Other=space]
MaxXY:  273  210 [max YYXX, use for verification] [STDF - no stdf record equivalent]
TotDie: 37077 [STDF - WRR.PART_CNT, PCR.PART_CNT]
Tested: 37077 [STDF - WRR.PART_CNT, PCR.PART_CNT]
Pickable: 35966 [passing die count - pat-man potentially updated this] [STDF - WRR.GOOD_CNT, PCR.GOOD_CNT]
TestInputMap: 13015_5MM_C.MP1 [STDF - no stdf record equivalent]
InkMap: 13015_5mm_c.mp1 [STDF - no stdf record equivalent]
MapRev: C [STDF - no stdf record equivalent]
[.=off_wafer X=exclusion P=PCM R=Reference 0=Fail (1-9|A-Z|a-z excluding X, P and R)=Variant or Bin Number]
[anything except 0.XPR are passing die and could updated by PAT to 0 for PAT fail]
[PAT-Man puts a '0' for a fail]
[first '.' in upper left is reticle_row=1, reticle_col=1, die_row=1, die_col=1]
[first '.' in upper left corner is (x,y)=(0,0) and increments to the right and down]


Example :


 */




namespace GS
{
namespace Parser {


WORD toWord(QChar charBin)
{
    if(charBin.isLetter())
    {
        // A = ascii(65)
        // a = ascii(97)
        return charBin.unicode() - 55;
    }
    else
        return charBin.unicode() - 48;
}

QVector<QString> WoburnSECSIIMap::mIsCompatibleKeys;

WoburnSECSIIMap::WoburnSECSIIMap() : ParserBase(typeWoburnSECSIIMap, "WoburnSECSIIMap")
{
    mPosX			= 'R';
    mPosY			= 'U';
    mStartTime      = 0;
}

WoburnSECSIIMap::~WoburnSECSIIMap()
{
}


void WoburnSECSIIMap::initCompatibleKeys()
{
    if(mIsCompatibleKeys.isEmpty() == false)
        return;

    mIsCompatibleKeys.push_back("WaferId");
    mIsCompatibleKeys.push_back("Orient");
    mIsCompatibleKeys.push_back("MaxXY");
    mIsCompatibleKeys.push_back("TotDie");
    mIsCompatibleKeys.push_back("Tested");
    mIsCompatibleKeys.push_back("Pickable");
    mIsCompatibleKeys.push_back("TestInputMap");
    mIsCompatibleKeys.push_back("InkMap");
    mIsCompatibleKeys.push_back("MapRev");

}

//////////////////////////////////////////////////////////////////////
// Copy the header into an outpout
//////////////////////////////////////////////////////////////////////
void WoburnSECSIIMap::CopyHeader( QTextStream& input,  QStringList &outPut)
{
    QString strString;
    while(!input.atEnd())
    {
        strString = input.readLine().simplified();
        outPut<< strString;

        if(strString.contains("MapRev"))
            return;
    }
}


bool WoburnSECSIIMap::IsPatValidDie(QChar charBin)
{
    return (    charBin != '0' &&
                charBin != '.' &&
                charBin != 'X' &&
                charBin != 'P' &&
                charBin != 'R' );
}


void  WoburnSECSIIMap::UpdateMapData(QStringList& fileInList, const QString& newValue)
{
    //-- retrive the string containing the key "Pickable", 6th element and update the associated value
    QString& lPickable = fileInList[5];
    lPickable.clear();
    lPickable = "Pickable: " + newValue;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WoburnSECSIIMap format
//////////////////////////////////////////////////////////////////////
bool WoburnSECSIIMap::IsCompatible(const QString &FileName)
{
    bool	lIsCompatible = false;

    QFile lFile( FileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        return false;
    }

    //-- init list of keys that must be found to be compatible
    initCompatibleKeys();

    // Assign file I/O stream
    QTextStream woburnSECSIIMapFile(&lFile);

    // Check if first N line is the correct SEMI_G85 header...
    int     lLine = 0;
    QString lString;
    while(!woburnSECSIIMapFile.atEnd())
    {
        lString = woburnSECSIIMapFile.readLine().simplified();

        if(lString.startsWith(mIsCompatibleKeys[lLine],Qt::CaseInsensitive) == false)
        {
            break;
        }

        ++lLine;

        if(lLine >= mIsCompatibleKeys.size())
        {
            lIsCompatible = true;
            break;
        }
    }

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
// Convert 'FileName' woburnSECSIIMap file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool WoburnSECSIIMap::ConvertoStdf(const QString &woburnSECSIIMapFileName, QString &StdfFileName)
{
    // No erro (default)
    mLastError     = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo lInputFile(woburnSECSIIMapFileName);
    QFileInfo lOutputFile(StdfFileName);

    QFile lFile( StdfFileName );
    if((lFile.exists() == true) && (lInputFile.lastModified() < lOutputFile.lastModified()))
        return true;

    if(ReadWoburnSECSIIMapFile(woburnSECSIIMapFileName, StdfFileName) != true)
    {
        return false;
    }

    // have a wafermap to save
    // write STDF file
    // Loop reading file until end is reached & generate STDF file dynamically.
    if(!WriteStdfFile(mWoburnSECSIITextStream,StdfFileName))
    {
        QFile::remove(StdfFileName);
        // Close file
        mFile.close();
        lFile.close();
        return false;
    }

    // Close file
    mFile.close();
    lFile.close();

    return true;
}

bool WoburnSECSIIMap::ReadWoburnSECSIIMapFile(const QString &woburnSECSIIMapFileName, const QString &)
{
    mFile.setFileName(woburnSECSIIMapFileName );
    if(!mFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SEMI_G85 file
        mLastError = errOpenFail;
        mStrLastErrorSpecification = mFile.errorString();

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    mWoburnSECSIITextStream.setDevice(&mFile);

    QString lStrSection, lStrValue;
    int lFoundCpt = 0;
    while(!mWoburnSECSIITextStream.atEnd())
    {
        lStrSection = ReadLine(mWoburnSECSIITextStream).simplified();
        lStrValue   = lStrSection.section(":",1).trimmed();
        lStrSection = lStrSection.left(lStrSection.indexOf(":")).trimmed().toUpper();

        if(lStrSection.compare(mIsCompatibleKeys[0], Qt::CaseInsensitive) == 0)
        {
            mLotId   = lStrValue.section("-",0, 0).trimmed();
            mWaferId = lStrValue.section("-",1).trimmed();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[1], Qt::CaseInsensitive) == 0)
        {
            if(lStrValue.compare("EAST",  Qt::CaseInsensitive) == 0)
                mOrient = 'R';
            else if(lStrValue.compare("SOUTH",  Qt::CaseInsensitive) == 0)
                mOrient = 'D';
            else if(lStrValue.compare("WEST",  Qt::CaseInsensitive) == 0)
                mOrient = 'L';
            else if(lStrValue.compare("NORTH",  Qt::CaseInsensitive) == 0)
                mOrient = 'U';
            else
                mOrient = ' ';
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[2], Qt::CaseInsensitive) == 0)
        {
            mWaferRows    = lStrValue.section(" ",0, 0).trimmed().toInt();
            mWaferColumns = lStrValue.section(" ",1).trimmed().toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[3], Qt::CaseInsensitive) == 0)
        {
            mTotDie = lStrValue.toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[4], Qt::CaseInsensitive) == 0)
        {
            mTested = lStrValue.toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[5], Qt::CaseInsensitive) == 0)
        {
            mPickable = lStrValue.toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[6], Qt::CaseInsensitive) == 0)
        {
            mTestInputMap = lStrValue;
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[7], Qt::CaseInsensitive)== 0)
        {
            mInkMap = lStrValue;
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[8], Qt::CaseInsensitive) == 0)
        {
            mMapRev = lStrValue;
            ++lFoundCpt;
        }

        if(lFoundCpt == mIsCompatibleKeys.size())
            break;
    }

    return true;
}

bool WoburnSECSIIMap::IsInvalidDie(const QChar& value)
{
    return (value == 'X'
            || value == '.'
            || value == 'P'
            || value == 'R');
}

bool WoburnSECSIIMap::WriteStdfFile (QTextStream &woburnSECSIIMapFile, const QString &StdfFileName)
{
    // now generate the STDF file...
    GQTL_STDF::StdfParse lStdfParser;

    if(lStdfParser.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        return false;
    }

    // Write Test results for each line read.
    QString		lString;
    int			lIndex = 0;				// Loop index
    WORD		lSoftBin = 0,lHardBin = 0;
    long        lPartNumber = 0;

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4          lMIRRecord;
    lMIRRecord.SetSETUP_T(mStartTime);			// Setup time
    lMIRRecord.SetSTART_T(mStartTime);			// Start time
    lMIRRecord.SetSTAT_NUM(1);                  // Station
    lMIRRecord.SetMODE_COD((BYTE)'P');          // Test Mode = PRODUCTION
    lMIRRecord.SetRTST_COD((BYTE)' ');          // rtst_cod
    lMIRRecord.SetPROT_COD((BYTE) ' ');			// prot_cod
    lMIRRecord.SetBURN_TIM(65535);				// burn_tim
    lMIRRecord.SetCMOD_COD((BYTE) ' ');			// cmod_cod
    lMIRRecord.SetLOT_ID(mLotId);               // Lot ID
    lMIRRecord.SetSBLOT_ID(mLotId);             // SubLotID
    lMIRRecord.SetPART_TYP("");                 // Part Type / Product ID
    lMIRRecord.SetNODE_NAM("");                 // Node name
    lMIRRecord.SetTSTR_TYP("");                 // Tester Type
    lMIRRecord.SetJOB_NAM("");                  // Job name
    lMIRRecord.SetTEST_COD("WAFER");			// test-cod;

    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":WoburnSECSII";
    lMIRRecord.SetUSER_TXT((char *)strUserTxt.toLatin1().constData());	// user-txt

    lStdfParser.WriteRecord(&lMIRRecord);

    // Reset counters
    lPartNumber=0;

    // Write WIR of new Wafer.
    GQTL_STDF::Stdf_WIR_V4  lWIRRecord;
    lWIRRecord.SetHEAD_NUM(1);							// Test head
    lWIRRecord.SetSITE_GRP(255);						// Tester site (all)
    lWIRRecord.SetSTART_T(mStartTime);                  // Start time
    lWIRRecord.SetWAFER_ID(mWaferId);                   // WaferID
    lStdfParser.WriteRecord(&lWIRRecord);

    // Write WCR of new Wafer.
    GQTL_STDF::Stdf_WCR_V4  lWCRRecord;
    lWCRRecord.SetWF_FLAT( mOrient.toLatin1() );
    lStdfParser.WriteRecord(&lWCRRecord);

    int			lWaferColumns = 0;
    int			lWaferRows    = 0;

    GQTL_STDF::Stdf_PRR_V4  lPRRRecord;
    GQTL_STDF::Stdf_PIR_V4  lPIRRecord;
    while(!woburnSECSIIMapFile.atEnd())
    {
        // Read line
        lString = ReadLine(woburnSECSIIMapFile).simplified().toUpper();
        if(lString.startsWith("}"))
            break;	// Reach the end of valid data records...
        if( lString.length() != mWaferColumns)
            mWaferColumns = lString.length();

        // Read Parameter results for this record
        QChar lCurrentBinStr;
        for(lIndex=0, lWaferColumns = 0; lIndex<(int)lString.length(); ++lIndex, ++lWaferColumns)
        {

            lCurrentBinStr = lString.at(lIndex);
            if(IsInvalidDie(lCurrentBinStr))
                continue;

            ++lPartNumber;
            // Write PIR
            lPIRRecord.Reset();
            lPIRRecord.SetHEAD_NUM(1);			// Test head
            lPIRRecord.SetSITE_NUM(1);			// Tester site#:1
            lStdfParser.WriteRecord(&lPIRRecord);

            // Write PRR
            lPRRRecord.Reset();
            lPRRRecord.SetHEAD_NUM(1);			// Test head
            lPRRRecord.SetSITE_NUM(1);			// Tester site#:1
            if(lCurrentBinStr == '0')
            {
                lPRRRecord.SetPART_FLG(8);		// PART_FLG : FAILED
                lSoftBin = lHardBin = 0;	// default FAIL bin
            }
            else
            {
                lPRRRecord.SetPART_FLG(0);				// PART_FLG : PASSED
                lSoftBin = toWord(lCurrentBinStr.toLatin1());
                lHardBin = 1;	// PASS Hard bin
            }

            lPRRRecord.SetNUM_TEST((WORD)0);			// NUM_TEST
            lPRRRecord.SetHARD_BIN(lHardBin);           // HARD_BIN
            lPRRRecord.SetSOFT_BIN(lSoftBin);           // SOFT_BIN
            lPRRRecord.SetX_COORD(lWaferColumns);      // X_COORD
            lPRRRecord.SetY_COORD(lWaferRows);         // Y_COORD

            lPRRRecord.SetPART_ID(QString::number(lPartNumber).toLatin1().constData());		// PART_ID
            lPRRRecord.SetPART_TXT("");				// PART_TXT
            lPRRRecord.SetPART_FIX();				// PART_FIX
            lStdfParser.WriteRecord(&lPRRRecord);
        }
         ++lWaferRows;
    }

    GQTL_STDF::Stdf_PCR_V4  lPCRRecord;
    lPCRRecord.SetHEAD_NUM(1);
    lPCRRecord.SetSITE_NUM(255);                    // Tester site (all)
    lPCRRecord.SetPART_CNT(mTested);                // Parts tested: always 5
    if(mTotDie != mTested)
        lPCRRecord.SetABRT_CNT(mTotDie - mTested);	// Parts Aborted
    else
        lPCRRecord.SetABRT_CNT(0);                  // Parts Aborted
    lPCRRecord.SetGOOD_CNT(mPickable);
    lStdfParser.WriteRecord(&lPCRRecord);

    // Write WRR for last wafer inserted
    GQTL_STDF::Stdf_WRR_V4              lWRRRecord;
    lWRRRecord.SetHEAD_NUM(1);                                  // Test head
    lWRRRecord.SetSITE_GRP(255);                                // Tester site (all)
    lWRRRecord.SetFINISH_T(0);                                  // Time of last part tested
    lWRRRecord.SetPART_CNT(mTested);                            // Parts tested: always 5
    lWRRRecord.SetRTST_CNT(0);                                  // Parts retested

    if(mTotDie != mTested)
        lWRRRecord.SetABRT_CNT(mTotDie - mTested);              // Parts Aborted
    else
        lWRRRecord.SetABRT_CNT(0);                              // Parts Aborted
    lWRRRecord.SetGOOD_CNT(mPickable);                          // Good Parts
    lWRRRecord.SetFUNC_CNT(4294967295UL);                       // Functionnal Parts
    lWRRRecord.SetWAFER_ID(mWaferId.toLatin1().constData());	// WaferID
    lStdfParser.WriteRecord(&lWRRRecord);
    // Read all lines with valid data records in file

    GQTL_STDF::Stdf_MRR_V4  lMRRRecord;
    lMRRRecord.SetFINISH_T(mStartTime);
    lStdfParser.WriteRecord(&lMRRRecord);
    lStdfParser.Close();
    return true;
}

} // namespace Parser
} // namespace GS
