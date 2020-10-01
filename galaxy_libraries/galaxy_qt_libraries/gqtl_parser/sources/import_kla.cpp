#include <QSet>
#include <QFile>

#include "import_kla.h"

namespace GS
{
namespace Parser
{

static const QString KLA_DEVICE("DEVICE");
static const QString KLA_LOT("LOT");
static const QString KLA_WAFER("WAFER");
static const QString KLA_FNLOC("FNLOC");
static const QString KLA_ROWCT("ROWCT");
static const QString KLA_COLCT("COLCT");
static const QString KLA_BCEQU("BCEQU");
static const QString KLA_REFPX("REFPX");
static const QString KLA_REFPY("REFPY");
static const QString KLA_DUTMS("DUTMS");
static const QString KLA_XDIES("XDIES");
static const QString KLA_YDIES("YDIES");
static const QString KLA_ROWDATA("ROWDATA");

ImportKLAMap::ImportKLAMap()
    : ParserBase( typeKLA, "typeKLA" )
{
    mRows = 0;
    mColumns = 0;
    mFlatNotch = 0;
    mXDiesSize = 0.0;
    mYDiesSize = 0.0;
    mRefDieX = 0;
    mRefDieY = 0;
    mGoodBins.append(1);
}

ImportKLAMap::~ImportKLAMap()
{

}

bool ImportKLAMap::ConvertoStdf(const QString &aInputFile, QString &aStdfFileName)
{
    QFile lFile(aInputFile);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        return false;
    }

    // Assign file I/O stream
    QTextStream aTextStream(&lFile);

    if (ReadHeader(aTextStream) == false)
    {
        lFile.close();
        return false;
    }

    if (GenerateOutput(aTextStream, aStdfFileName) == false)
    {
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    return true;
}

bool ImportKLAMap::IsCompatible(const QString &aInputFile)
{
    bool	lIsCompatible = false;

    QFile lFile(aInputFile);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        return false;
    }

    //-- init list of keys that must be found to be compatible
    QSet<QString> lAuthorizedHeaderKeys;
    lAuthorizedHeaderKeys << KLA_DEVICE << KLA_LOT << KLA_WAFER << KLA_FNLOC << KLA_ROWCT << KLA_COLCT
                    << KLA_BCEQU << KLA_REFPX << KLA_REFPY << KLA_DUTMS << KLA_XDIES << KLA_YDIES;

    // Assign file I/O stream
    QTextStream lKLAMapFile(&lFile);
    int         lAuthorizedKeysFound = 0;
    QPair<QString, QString> lPair;

    while(!lKLAMapFile.atEnd() && lIsCompatible == false)
    {
        lPair = ParseLine(lKLAMapFile.readLine());

        if (lPair.first.isEmpty() == false)
        {
            if (lAuthorizedKeysFound == lAuthorizedHeaderKeys.count() && lPair.first.compare(KLA_ROWDATA) == 0)
            {
                lIsCompatible = true;
            }
            else if (lAuthorizedHeaderKeys.find(lPair.first) != lAuthorizedHeaderKeys.constEnd())
            {
                ++lAuthorizedKeysFound;
            }
            else
            {
                break;
            }
        }
    }

    // Close file
    lFile.close();

    return lIsCompatible;
}

QPair<QString, QString> ImportKLAMap::ParseLine(const QString &aLine)
{
    QPair<QString, QString> lPair;

    if (!aLine.isNull() && !aLine.isEmpty())
    {
        lPair.first     = aLine.section(":", 0, 0).simplified().trimmed().toUpper();
        lPair.second    = aLine.section(":", 1, 1).simplified().trimmed();
    }

    return lPair;
}

bool ImportKLAMap::ReadHeader(QTextStream& aTextStream)
{
    QPair<QString, QString> lPair;
    bool    lBinmapFound = false;
    qint64  lLastFilePos = 0;

    while(!aTextStream.atEnd() && lBinmapFound == false)
    {
        lLastFilePos = aTextStream.pos();

        lPair = ParseLine(ReadLine(aTextStream));

        if(lPair.first.compare(KLA_DEVICE, Qt::CaseInsensitive) == 0)
        {
            mDevice = lPair.second;
        }
        else if(lPair.first.compare(KLA_LOT, Qt::CaseInsensitive) == 0)
        {
            mLot = lPair.second;
        }
        else if(lPair.first.compare(KLA_WAFER, Qt::CaseInsensitive) == 0)
        {
            mWafer = lPair.second;
        }
        else if(lPair.first.compare(KLA_FNLOC, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToInteger(lPair.first, lPair.second, mFlatNotch) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_ROWCT, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToInteger(lPair.first, lPair.second, mRows) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_BCEQU, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToIntegerArray(lPair.first, lPair.second, mGoodBins) == false)
                return false;
        }
        else if(lPair.first.compare(KLA_COLCT, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToInteger(lPair.first, lPair.second, mColumns) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_REFPX, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToInteger(lPair.first, lPair.second, mRefDieX) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_REFPY, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToInteger(lPair.first, lPair.second, mRefDieY) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_DUTMS, Qt::CaseInsensitive) == 0)
        {
            mDiesUnit = lPair.second;
        }
        else if(lPair.first.compare(KLA_XDIES, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToFloat(lPair.first, lPair.second, mXDiesSize) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_YDIES, Qt::CaseInsensitive) == 0)
        {
            if (ConvertHeaderToFloat(lPair.first, lPair.second, mYDiesSize) == false)
            {
                return false;
            }
        }
        else if(lPair.first.compare(KLA_ROWDATA, Qt::CaseInsensitive) == 0)
        {
            lBinmapFound = true;

            // Rewind to last line read, so binning map information will be all read when processing the map
            aTextStream.seek(lLastFilePos);
        }
        else
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("Unsupported key %1 found in KLA file").arg(lPair.first);
            return false;
        }
    }

    if (lBinmapFound == false)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QString("Unable to find RowData information");

        return false;
    }

    return true;
}

bool ImportKLAMap::GenerateOutput(QTextStream &aTextStream, const QString &aStdfFileName)
{
    // Assign file I/O stream
    GQTL_STDF::StdfParse lStdfParser;

    if(lStdfParser.Open(aStdfFileName.toLatin1().constData(), STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        return false;
    }

    time_t lCurrentTime = QDateTime::currentDateTime().toTime_t();

    GQTL_STDF::Stdf_FAR_V4 lFARRecord;
    lFARRecord.SetCPU_TYPE(1);
    lFARRecord.SetSTDF_VER(4);
    lStdfParser.WriteRecord(&lFARRecord);

    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    lMIRRecord.SetSETUP_T((stdf_type_u4) lCurrentTime);
    lMIRRecord.SetSTART_T((stdf_type_u4) lCurrentTime);
    lMIRRecord.SetSTAT_NUM(1);
    lMIRRecord.SetMODE_COD('P');
    lMIRRecord.SetLOT_ID(mLot);
    lMIRRecord.SetPART_TYP(mDevice);
    lMIRRecord.SetSBLOT_ID(mWafer);
    lMIRRecord.SetTEST_COD("WAFER");
    QString	lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += ":";
    lUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    lUserTxt += ":KLA/INF";
    lMIRRecord.SetUSER_TXT(lUserTxt);
    lStdfParser.WriteRecord(&lMIRRecord);

    GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
    lWIRRecord.SetHEAD_NUM(1);
    lWIRRecord.SetSITE_GRP(255);
    lWIRRecord.SetSTART_T((stdf_type_u4) lCurrentTime);
    lWIRRecord.SetWAFER_ID(mWafer);
    lStdfParser.WriteRecord(&lWIRRecord);

    QPair<QString, QString> lPair;
    int             lRow = 0;
    int             lCol = 0;
    int             lPartNumber = 0;
    stdf_type_u4    lTotalGoodBin = 0;
    ushort          lBin;

    while(aTextStream.atEnd() == false)
    {
        lPair = ParseLine(ReadLine(aTextStream));

        if (lPair.first.compare(KLA_ROWDATA) == 0)
        {
            QStringList lDies = lPair.second.split(" ");

            if (lDies.count() != mColumns) {
                mLastError = errInvalidFormatParameter;
                mLastErrorMessage = QString("%1 were expected on rowdata %2 but only %3 found")
                        .arg(mColumns).arg(lRow).arg(lDies.count());

                return false;
            }

            lCol = 0;
            for(QStringList::const_iterator lIter = lDies.cbegin(), lEnd = lDies.cend(); lIter != lEnd; ++lIter)
            {
                if (ProcessDie(*lIter, lBin))
                {
                    ++lPartNumber;

                    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
                    lPIRRecord.SetHEAD_NUM(1);
                    lPIRRecord.SetSITE_NUM(1);
                    lStdfParser.WriteRecord(&lPIRRecord);

                    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
                    lPRRRecord.SetHEAD_NUM(1);
                    lPRRRecord.SetSITE_NUM(1);
                    lPRRRecord.SetX_COORD(mRefDieX + lCol);
                    lPRRRecord.SetY_COORD(mRefDieY + lRow);
                    lPRRRecord.SetHARD_BIN(lBin);
                    lPRRRecord.SetSOFT_BIN(lBin);
                    lPRRRecord.SetPART_ID(QString::number(lPartNumber));

                    if (mGoodBins.contains(lBin))
                    {
                        lPRRRecord.SetPART_FLG(0);
                        ++lTotalGoodBin;
                    }
                    else
                        lPRRRecord.SetPART_FLG(8);

                    lStdfParser.WriteRecord(&lPRRRecord);
                }

                ++lCol;
            }

            ++lRow;
        }
    }

    GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
    lWRRRecord.SetHEAD_NUM(1);
    lWRRRecord.SetSITE_GRP(255);
    lWRRRecord.SetFINISH_T((stdf_type_u4) lCurrentTime);
    lWRRRecord.SetABRT_CNT(0);
    lWRRRecord.SetRTST_CNT(0);
    lWRRRecord.SetGOOD_CNT(lTotalGoodBin);
    lWRRRecord.SetPART_CNT(lPartNumber);
    lWRRRecord.SetWAFER_ID(mWafer);
    lStdfParser.WriteRecord(&lWRRRecord);

    GQTL_STDF::Stdf_WCR_V4 lWCRRecord;
    lWCRRecord.SetWAFR_SIZ(0);
    lWCRRecord.SetDIE_HT(mYDiesSize);
    lWCRRecord.SetDIE_WID(mXDiesSize);
    lWCRRecord.SetWF_UNITS(GetSTDFWaferUnit());
    lWCRRecord.SetWF_FLAT(GetSTDFWaferFlat());
    lWCRRecord.SetPOS_X('R');
    lWCRRecord.SetPOS_Y('D');
    lStdfParser.WriteRecord(&lWCRRecord);

    // HBR/SBR
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;

    for (QMap<int, ParserBinning>::const_iterator lIter = mBinning.cbegin(), lEnd = mBinning.cend();
         lIter != lEnd; ++lIter)
    {
        char lFlag = (lIter.value().GetPassFail() ? 'P' : 'F');

        lHBRRecord.SetHEAD_NUM(255);
        lHBRRecord.SetSITE_NUM(1);
        lHBRRecord.SetHBIN_NUM(lIter.value().GetBinNumber());
        lHBRRecord.SetHBIN_CNT(lIter.value().GetBinCount());
        lHBRRecord.SetHBIN_PF(lFlag);
        lStdfParser.WriteRecord(&lHBRRecord);

        lSBRRecord.SetHEAD_NUM(255);
        lSBRRecord.SetSITE_NUM(1);
        lSBRRecord.SetSBIN_NUM(lIter.value().GetBinNumber());
        lSBRRecord.SetSBIN_CNT(lIter.value().GetBinCount());
        lSBRRecord.SetSBIN_PF(lFlag);
        lStdfParser.WriteRecord(&lSBRRecord);
    }

    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    lMRRRecord.SetFINISH_T((stdf_type_u4) lCurrentTime);
    lStdfParser.WriteRecord(&lMRRRecord);

    lStdfParser.Close();
    return true;
}

bool ImportKLAMap::ConvertHeaderToIntegerArray(const QString &lKey, const QString &lStringValue,
                                               QList<int> &lConvertedValue)
{
    bool lConversionStatus = false;
    QStringList lBinList = lStringValue.split(" ");

    lConvertedValue.clear();

    for(QStringList::const_iterator lIter = lBinList.cbegin(), lEnd = lBinList.cend(); lIter != lEnd; ++lIter)
    {
        lConvertedValue.append((*lIter).toInt(&lConversionStatus, 16));

        if (lConversionStatus == false)
        {
            mLastError = errInvalidFormatParameter;
            mLastErrorMessage = QString("Key %1 must be a list of integer values. Found %2 instead")
                    .arg(lKey).arg(lStringValue);

            return false;
        }
    }

    return true;
}

bool ImportKLAMap::ConvertHeaderToInteger(const QString& lKey, const QString &lStringValue, int &lConvertedValue)
{
    bool lConversionStatus = false;

    lConvertedValue = lStringValue.toInt(&lConversionStatus);

    if (lConversionStatus == false)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QString("Key %1 must be an integer value. Found %2 instead").arg(lKey).arg(lStringValue);

        return false;
    }

    return true;
}

bool ImportKLAMap::ConvertHeaderToFloat(const QString &lKey, const QString &lStringValue, float &lConvertedValue)
{
    bool lConversionStatus = false;

    lConvertedValue = lStringValue.toFloat(&lConversionStatus);

    if (lConversionStatus == false)
    {
        mLastError = errInvalidFormatParameter;
        mLastErrorMessage = QString("Key %1 must be a number. Found %2 instead").arg(lKey).arg(lStringValue);

        return false;
    }

    return true;
}

stdf_type_c1 ImportKLAMap::GetSTDFWaferFlat() const
{
    stdf_type_c1 lOrientation = ' ';

    switch(mFlatNotch)
    {
        case 0: lOrientation = 'D';
            break;

        case 90: lOrientation = 'L';
            break;

        case 180: lOrientation = 'U';
            break;

        case 270: lOrientation = 'R';
            break;

        default:
            break;
    }

    return lOrientation;
}

stdf_type_u1 ImportKLAMap::GetSTDFWaferUnit() const
{
    stdf_type_u1 lWaferUnit = 0;

    if(mDiesUnit.compare("inch", Qt::CaseInsensitive) == 0)
        lWaferUnit = 1;
    else if(mDiesUnit.compare("cm", Qt::CaseInsensitive) == 0)
        lWaferUnit = 2;
    else if(mDiesUnit.compare("mm", Qt::CaseInsensitive) == 0)
        lWaferUnit = 3;
    else if(mDiesUnit.compare("um", Qt::CaseInsensitive) == 0)
        lWaferUnit = 4;

    return lWaferUnit;
}

bool ImportKLAMap::ProcessDie(const QString &aDie, ushort& aBin)
{
    bool lConversionStatus = false;

    aBin = aDie.toUShort(&lConversionStatus,16);

    // Conversion might fail when string do not represent a Bin#, could be "__" or "@@", or '@@@', etc...
    if (lConversionStatus)
    {
        if (mBinning.contains(aBin) == false)
        {
            ParserBinning lBinning;

            lBinning.SetBinNumber(aBin);
            lBinning.SetPassFail(mGoodBins.contains(aBin));

            mBinning.insert(aBin, lBinning);
        }

        mBinning[aBin].IncrementBinCount(1);
    }

    return lConversionStatus;
}

}
}
