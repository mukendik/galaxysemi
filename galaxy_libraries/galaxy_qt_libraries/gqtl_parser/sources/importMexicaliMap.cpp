
#include <QFile>
#include <QFileInfo>
#include "gqtl_log.h"
#include "stdfparse.h"
#include "importMexicaliMap.h"

/**
 *
Mask=PK1001-B                           [STDF - MIR.part_typ]
Lot ID=J89283                           [STDF - MIR.lot_id]
Wafer ID=J89283-03E6                    [STDF - WIR.wafer_id, WRR.wafer_id, MIR.sublot_id]
Flat Orientation=North                  [STDF - WCR.WF_FLAT North=U, South=D, East=R, West=L, Other=space]
MaxXY=133 108                           [max YYXX, use for verification] [STDF - no stdf record equivalent]
TotDie=11447                            [STDF - Used to calculate WRR.ABRT_CNT = (TotDie - Tested)]
Tested=10969                            [STDF - WRR.PART_CNT and PCR.PART_CNT]
Pickable=7639                           [passing die count - pat-man potentially updates this] [STDF - WRR.GOOD_CNT, PCR.GOOD_CNT]
Wafer Start Time=15 Nov 2015 11:45:08   [STDF - MIR.START_T and WIR.START_T]
Wafer End Time=15 Nov 2015 12:57:35     [STDF - MRR.FINISH_T and WRR.FINISH_T]
Map Rev=A                               [MIR.JOB_REV]

First digit going 'top-down' and 'right-to-left' direction is x=0, y=1
Postive x from right to left
Positive y top to down
Map and STDF line up.
The only possible values of the 'map' below are: X, 0, 1, and .

 */




namespace GS
{
namespace Parser {


QVector<QString> MexicaliMap::mIsCompatibleKeys;

MexicaliMap::MexicaliMap() : ParserBase(typeMexicaliMap, "MexicaliMap")
{
    mStartTime      = 0;
}

MexicaliMap::~MexicaliMap()
{
}


void MexicaliMap::initCompatibleKeys()
{
    if(mIsCompatibleKeys.isEmpty() == false)
        return;

    mIsCompatibleKeys.insert(T_MASK,    "Mask");
    mIsCompatibleKeys.insert(T_LOTID,   "Lot ID");
    mIsCompatibleKeys.insert(T_WAFERID, "Wafer ID");
    mIsCompatibleKeys.insert(T_FORIENT, "Flat Orientation");
    mIsCompatibleKeys.insert(T_MAXXY,   "MaxXY");
    mIsCompatibleKeys.insert(T_TOTDIE,  "TotDie");
    mIsCompatibleKeys.insert(T_TESTED,  "Tested");
    mIsCompatibleKeys.insert(T_PICKABLE,"Pickable");
    mIsCompatibleKeys.insert(T_WSTART,  "Wafer Start Time");
    mIsCompatibleKeys.insert(T_WEND,    "Wafer End Time");
    mIsCompatibleKeys.insert(T_MAPREV,  "Map Rev");
}

bool MexicaliMap::GetDateFromString(QString dateString, gstime& dataTime)
{

    QMap<QString, int>  lMonth;
    lMonth.insert("Jan", 1);
    lMonth.insert("Feb", 2);
    lMonth.insert("Mar", 3);
    lMonth.insert("Apr", 4);
    lMonth.insert("May", 5);
    lMonth.insert("Jun", 6);
    lMonth.insert("Jul", 7);
    lMonth.insert("Aug", 8);
    lMonth.insert("Sep", 9);
    lMonth.insert("Oct", 10);
    lMonth.insert("Nov", 11);
    lMonth.insert("Dec", 12);

    // --  ex :14 Nov 2015 09:38:03
    QList<QString> lListElements = dateString.split(" ");

    if(lListElements.size() != 4 && lMonth.contains(lListElements[1]) == false)
        return false;

    QDate lDate;
    lDate.setDate(lListElements[2].toInt(), lMonth[lListElements[1]], lListElements[0].toInt());

    QTime lTime = QTime::fromString(lListElements[3], "hh:mm:ss");
    dataTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    return true;
}

void MexicaliMap::CopyHeader( QTextStream& input,  QStringList &outPut)
{
    QString strString;
    while(!input.atEnd())
    {
        strString = input.readLine().simplified();
        outPut<< strString;

        if(strString.contains(mIsCompatibleKeys[T_MAPREV], Qt::CaseInsensitive))
            return;
    }
}

void MexicaliMap::SetOffsetOrigin(int &xOffset, int &yOffset, int lX, int lY)
{
    xOffset = lX;
    yOffset = lY - 1;
}

bool MexicaliMap::IsInTheCoordinateSystem(QChar lCharBin)
{
    return (lCharBin != '.' &&
            lCharBin != 'X');
}

bool MexicaliMap::IsPatValidDie(QChar charBin)
{
    return (charBin != '0' &&
            charBin != '.' &&
            charBin != 'X');
}

void  MexicaliMap::UpdateMapData(QStringList& fileInList, const QString& newValue)
{
    //-- retrive the string containing the key "Pickable"
    QString& lPickable = fileInList[T_PICKABLE];
    lPickable.clear();
    lPickable = "Pickable=" + newValue;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with MexicaliMap format
//////////////////////////////////////////////////////////////////////
bool MexicaliMap::IsCompatible(const QString &FileName)
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
    QTextStream lMexicaliMapFile(&lFile);

    // Check if first N line is the correct Mexicaly header...
    int     lLine = 0;
    QString lString;
    while(!lMexicaliMapFile.atEnd())
    {
        lString = lMexicaliMapFile.readLine().simplified();

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
// Convert 'FileName' MexicaliMap file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool MexicaliMap::ConvertoStdf(const QString &mexicaliMapFileName,  QString &StdfFileName)
{
    // No erro (default)
    mLastError     = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo lInputFile(mexicaliMapFileName);
    QFileInfo lOutputFile(StdfFileName);

    QFile lFile( StdfFileName );
    if((lFile.exists() == true) && (lInputFile.lastModified() < lOutputFile.lastModified()))
        return true;

    if(ReadMexicaliMapFile(mexicaliMapFileName, StdfFileName) != true)
    {
        return false;
    }

    // have a wafermap to save
    // write STDF file
    // Loop reading file until end is reached & generate STDF file dynamically.
    if(!WriteStdfFile(mMexicaliTextStream,StdfFileName))
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

    // Convertion successful
    return true;
}

bool MexicaliMap::ReadMexicaliMapFile(const QString &mexicaliMapFileName, const QString &)
{
    mFile.setFileName(mexicaliMapFileName );
    if(!mFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SEMI_G85 file
        mLastError = errOpenFail;
        mStrLastErrorSpecification = mFile.errorString();

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    mMexicaliTextStream.setDevice(&mFile);

    QString lStrSection, lStrValue;
    int lFoundCpt = 0;
    while(!mMexicaliTextStream.atEnd())
    {
        lStrSection = ReadLine(mMexicaliTextStream).simplified();
        lStrValue   = lStrSection.section("=",1).trimmed();
        lStrSection = lStrSection.left(lStrSection.indexOf("=")).trimmed().toUpper();

        if(lStrSection.compare(mIsCompatibleKeys[T_MASK], Qt::CaseInsensitive) == 0)
        {
            mPartType = lStrValue.trimmed();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_LOTID], Qt::CaseInsensitive) == 0)
        {
            mLotId   = lStrValue.trimmed();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_WAFERID], Qt::CaseInsensitive) == 0)
        {
            mWaferId = lStrValue.trimmed();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_FORIENT], Qt::CaseInsensitive) == 0)
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
        else if(lStrSection.compare(mIsCompatibleKeys[T_MAXXY], Qt::CaseInsensitive) == 0)
        {
            mWaferRows    = lStrValue.section(" ",0, 0).trimmed().toInt();
            mWaferColumns = lStrValue.section(" ",1).trimmed().toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_TOTDIE], Qt::CaseInsensitive) == 0)
        {
            mTotDie = lStrValue.toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_TESTED], Qt::CaseInsensitive) == 0)
        {
            mTested = lStrValue.toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_PICKABLE], Qt::CaseInsensitive) == 0)
        {
            mPickable = lStrValue.toInt();
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_WSTART], Qt::CaseInsensitive) == 0)
        {
            GetDateFromString(lStrValue, mStartTime);
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_WEND], Qt::CaseInsensitive) == 0)
        {
            GetDateFromString(lStrValue, mWaferEndTime);
            ++lFoundCpt;
        }
        else if(lStrSection.compare(mIsCompatibleKeys[T_MAPREV], Qt::CaseInsensitive) == 0)
        {
            mMapRev = lStrValue;
            ++lFoundCpt;
        }

        if(lFoundCpt == mIsCompatibleKeys.size())
            break;
    }

    return true;
}

bool MexicaliMap::IsNotTestedDie(const QChar& value)
{
    return (value == 'X' || value == '.');
}

bool MexicaliMap::IsInvalidDie(const QChar& value)
{
    // invalid die if not '0','1','R' or '.'
    return (QRegExp("[01X\\.]{1}").exactMatch(value) == false);
}

bool MexicaliMap::WriteStdfFile (QTextStream &mexicaliMapFile, const QString &StdfFileName)
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
    int 		lSoftBin = 0,lHardBin = 0;
    long        lPartNumber = 0;

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4          lMIRRecord;
    lMIRRecord.SetSETUP_T(mStartTime);			// Setup time
    lMIRRecord.SetSTART_T(mStartTime);     // Start time
    lMIRRecord.SetSTAT_NUM(1);                  // Station
    lMIRRecord.SetMODE_COD((BYTE)'P');          // Test Mode = PRODUCTION
    lMIRRecord.SetLOT_ID(mLotId);               // Lot ID
    lMIRRecord.SetSBLOT_ID(mWaferId);             // SubLotID
    lMIRRecord.SetPART_TYP(mPartType);          // Part Type / Product ID
    lMIRRecord.SetJOB_REV(mMapRev);             // Job Rev
    lMIRRecord.SetTEST_COD("WAFER");			// test-cod;

    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":Mexicali";
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
    lWCRRecord.SetPOS_X('L');
    lStdfParser.WriteRecord(&lWCRRecord);

    int			lWaferColumns = 0;
    int			lWaferRows    = 0;

    bool origineSet = false;
    int fromXOrigine = 0;
    int fromYOrigine = 0;

    GQTL_STDF::Stdf_PRR_V4  lPRRRecord;
    GQTL_STDF::Stdf_PIR_V4  lPIRRecord;
    while(!mexicaliMapFile.atEnd())
    {
        // Read line
        lString = ReadLine(mexicaliMapFile).simplified().toUpper();
        if(lString.startsWith("}"))
            break;	// Reach the end of valid data records...
        if( lString.length() != mWaferColumns)
            mWaferColumns = lString.length();

        // Read Parameter results for this record
        QChar lCurrentBinStr;
        //for(lIndex=0, lWaferColumns = 0; lIndex<(int)lString.length(); ++lIndex, ++lWaferColumns)
        int nbColumn = (int)lString.length() - 1;
        for(lIndex=nbColumn, lWaferColumns = 0; lIndex>=0; --lIndex, ++lWaferColumns)
        {

            lCurrentBinStr = lString.at(lIndex);
            if (IsInvalidDie(lCurrentBinStr))
            {
                mLastError = errInvalidMapCharacter;
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid %1 character found").arg(lCurrentBinStr).toLatin1().constData());

                return false;
            }

            if(IsNotTestedDie(lCurrentBinStr))
                continue;

            if(origineSet == false)
            {
                fromXOrigine = lWaferColumns;
                fromYOrigine = lWaferRows;
                origineSet = true;
            }

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
                lSoftBin = lHardBin = 0;        // default FAIL bin
            }
            else
            {
                lPRRRecord.SetPART_FLG(0);				// PART_FLG : PASSED
                lSoftBin = lHardBin = 1;                // PASS  bin
            }

            lPRRRecord.SetNUM_TEST((WORD)0);			// NUM_TEST
            lPRRRecord.SetHARD_BIN(lHardBin);           // HARD_BIN
            lPRRRecord.SetSOFT_BIN(lSoftBin);           // SOFT_BIN
            lPRRRecord.SetX_COORD(lWaferColumns - fromXOrigine);      // X_COORD
            lPRRRecord.SetY_COORD(lWaferRows - fromYOrigine + 1);         // Y_COORD

            lPRRRecord.SetPART_ID(QString::number(lPartNumber).toLatin1().constData());		// PART_ID
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
    lWRRRecord.SetFINISH_T(mWaferEndTime);                               // Time of last part tested
    lWRRRecord.SetPART_CNT(mTested);                      // Parts tested: always 5

    if(mTotDie != mTested)
        lWRRRecord.SetABRT_CNT(mTotDie - mTested);              // Parts Aborted
    else
        lWRRRecord.SetABRT_CNT(0);                              // Parts Aborted
    lWRRRecord.SetGOOD_CNT(mPickable);                          // Good Parts
    lWRRRecord.SetWAFER_ID(mWaferId.toLatin1().constData());	// WaferID
    lStdfParser.WriteRecord(&lWRRRecord);
    // Read all lines with valid data records in file

    GQTL_STDF::Stdf_MRR_V4  lMRRRecord;
    lMRRRecord.SetFINISH_T(mWaferEndTime);
    lStdfParser.WriteRecord(&lMRRRecord);
    lStdfParser.Close();
    return true;
}

std::string MexicaliMap::GetErrorMessage(const int ErrorCode) const
{
    QString lError;

    switch(ErrorCode)
    {
        case errInvalidMapCharacter:
            lError += "Invalid Map character detected";
            break;

        default:
            lError += QString::fromStdString(ParserBase::GetErrorMessage(ErrorCode));
            break;
    }

    return lError.toStdString();
}

} // namespace Parser
} // namespace GS
