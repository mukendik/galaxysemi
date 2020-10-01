//////////////////////////////////////////////////////////////////////
// import_csv.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "import_csv_skyworks.h"
#include <gqtl_log.h>
#include <math.h>
#include <qfileinfo.h>

namespace GS
{
namespace Parser
{

const char * sLowLimitPattern   = "Lower Limit For Case (\\d+):";
const char * sHighLimitPattern  = "Upper Limit For Case (\\d+):";

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGCSVSkyworkstoSTDF::CGCSVSkyworkstoSTDF(): ParserBase(typeSkyFasterCSV, "typeSkyFasterCSV")
{
    mCGCsvParameter = NULL;
    mParameterDirectory.SetFileName(GEX_SKYWORKS_CSV_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGCSVSkyworkstoSTDF::~CGCSVSkyworkstoSTDF()
{
    // Destroy list of Parameters tables.
    if(mCGCsvParameter!=NULL)
        delete [] mCGCsvParameter;
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TERADYNE_ASCII format
//////////////////////////////////////////////////////////////////////
bool CGCSVSkyworkstoSTDF::IsCompatible(const QString &lFileName)
{
    bool	lIsCompatible(false);

    QFile lFile(lFileName);
    if(!lFile.open(QIODevice::ReadOnly ))
    {
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);


    // Check the compatibility
    QString lStrString = lCsvFile.readLine();
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (lStrCells.count() >= CSV_RAW_FIRST_DATA
        && lStrCells[0].contains("PRODUCT", Qt::CaseInsensitive)
        && lStrCells[1].contains("SERIALNUMBER", Qt::CaseInsensitive)
        && lStrCells[2].contains("FRAMEWORK", Qt::CaseInsensitive)
        && lStrCells[3].contains("PROGRAM", Qt::CaseInsensitive)
        && lStrCells[CSV_RAW_SBIN].contains("SFBIN", Qt::CaseInsensitive))
        lIsCompatible = true;
    else
        lIsCompatible =false;

    // Close file
    lFile.close();

    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the CSV file
//////////////////////////////////////////////////////////////////////
bool CGCSVSkyworkstoSTDF::ConvertoStdf(const QString& lCsvFileName,
                                      QString &lFileNameSTDF)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg(lCsvFileName).toLatin1().constData());

    QString lString;

    // Open CSV file
    QFile lFile(lCsvFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);


    // Check the compatibility
    lString = ReadLine(lCsvFile);
    QStringList lCells = lString.split(",",QString::KeepEmptyParts);
    if (lCells.count() >= CSV_RAW_FIRST_DATA
        && lCells[0].contains("PRODUCT", Qt::CaseInsensitive)
        && lCells[1].contains("SERIALNUMBER", Qt::CaseInsensitive)
        && lCells[2].contains("FRAMEWORK", Qt::CaseInsensitive)
        && lCells[3].contains("PROGRAM", Qt::CaseInsensitive)
        && lCells[CSV_RAW_SBIN].contains("SFBIN", Qt::CaseInsensitive))
    {
        mTotalParameters=0;
        mTotalParameters = lCells.count() - CSV_RAW_FIRST_DATA;
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("%1 Total Parameters").arg( mTotalParameters).toLatin1().constData());
        // Allocate the buffer to hold the N parameters & results.
        try
        {
            mCGCsvParameter = new ParserParameter[mTotalParameters];	// List of parameters
        }
        catch(const std::bad_alloc& e)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
            lFile.close();
            return false;
        }
        catch(...)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Exception caught ");
            lFile.close();
            return false;
        }

        // Set test names in the csv parameters
        for(unsigned i=0; i<mTotalParameters; ++i)
        {
            mCGCsvParameter[i].SetTestName(lCells[CSV_RAW_FIRST_DATA + i]);

            int lTestNumber = mParameterDirectory.UpdateParameterIndexTable(mCGCsvParameter[i].GetTestName());

            if (lTestNumber >= 0)
                mCGCsvParameter[i].SetTestNumber(lTestNumber);
            else
            {
                mLastError = errInvalidFormatParameter;
                return false;
            }
        }
    }
    else
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // read units
    lString = ReadLine(lCsvFile);
    if (lString.contains("UNIT", Qt::CaseInsensitive))
    {
        lCells = lString.split(",",QString::KeepEmptyParts);
        for(unsigned i=0; i<mTotalParameters; ++i)
        {
            mCGCsvParameter[i].SetTestUnit(lCells[CSV_RAW_FIRST_DATA + i]);
        }
    }

    // Indicates we have read all limit definitions
    bool    lLimitsFullyRead    = false;
    bool    lOk                 = false;
    qint64  lLastFilePos        = 0;
    double  lValue              = 0;
    QRegExp lLowLimitRegExp(sLowLimitPattern, Qt::CaseInsensitive);
    QRegExp lHighLimitRegExp(sHighLimitPattern, Qt::CaseInsensitive);
    QMap<int, QPair<QString,QString> > lLimitsCase;

    do
    {
        // read next line
        lLastFilePos = lCsvFile.pos();
        lString = ReadLine(lCsvFile);

        // Split line
        lCells = lString.split(",",QString::KeepEmptyParts);

        if (lCells.count() < (int) (CSV_RAW_FIRST_DATA + mTotalParameters))
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input csv lines (%1 instead of %2 expected)")
                  .arg(lCells.count()).arg(CSV_RAW_FIRST_DATA + mTotalParameters).toLatin1().constData());
            return false;
        }

        // read low limits
        if (lLowLimitRegExp.exactMatch(lCells.at(CSV_RAW_SBIN).simplified()))
        {
            int lCase = lLowLimitRegExp.cap(1).toInt();

            if (lCase == 1)
            {
                for(unsigned i=0; i<mTotalParameters; ++i)
                {
                    lValue = lCells[CSV_RAW_FIRST_DATA + i].toDouble(&lOk);
                    if (lOk && lValue > -1e+50)
                    {
                        mCGCsvParameter[i].SetLowLimit(lValue);
                        mCGCsvParameter[i].SetValidLowLimit(true);
                    }
                    else
                    {
                        mCGCsvParameter[i].SetLowLimit(0);
                        mCGCsvParameter[i].SetValidLowLimit(false);
                    }
                }
            }

            if (lLimitsCase.contains(lCase))
            {
                // Lower limits for this case already defined
                if (lLimitsCase.value(lCase).first.isEmpty() == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR,
                          QString("Lower Limit redefinition for case %1")
                          .arg(lCase).toLatin1().constData());
                    mLastError = errInvalidFormatParameter;
                    return false;
                }
                else
                    lLimitsCase[lCase].first = lString;
            }
            else
                lLimitsCase.insert(lCase, QPair<QString,QString>(lString, ""));
        }
        else if (lHighLimitRegExp.exactMatch(lCells.at(CSV_RAW_SBIN).simplified()))
        {
            int lCase = lHighLimitRegExp.cap(1).toInt();

            if (lCase == 1)
            {
                for(unsigned i=0; i<mTotalParameters; ++i)
                {
                    lValue = lCells[CSV_RAW_FIRST_DATA + i].toDouble(&lOk);
                    if (lOk && lValue < 1e+50)
                    {
                        mCGCsvParameter[i].SetHighLimit(lValue);
                        mCGCsvParameter[i].SetValidHighLimit(true);
                    }
                    else
                    {
                        mCGCsvParameter[i].SetHighLimit(0);
                        mCGCsvParameter[i].SetValidHighLimit(false);
                    }
                }
            }

            if (lLimitsCase.contains(lCase))
            {
                // Lower limits for this case already defined
                if (lLimitsCase.value(lCase).second.isEmpty() == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR,
                          QString("Upper Limit redefinition for case %1")
                          .arg(lCase).toLatin1().constData());
                    mLastError = errInvalidFormatParameter;
                    return false;
                }
                else
                    lLimitsCase[lCase].second = lString;
            }
            else
                lLimitsCase.insert(lCase, QPair<QString,QString>("", lString));
        }
        else
            lLimitsFullyRead = true;
    }
    while (lCsvFile.atEnd() == false && lLimitsFullyRead == false);

    // Reach the ned of file wihtout read all limit definitions
    if(lCsvFile.atEnd() == true || lLimitsFullyRead == false)
    {
        // Incorrect header...this is not a valid CSV file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lFile.close();
        return false;
    }

    // Process multi-limits informations
    if (ProcessMultiLimits(lLimitsCase) == false)
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // All tests names read...check if need to update the csv Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Rewing to file begin.
    lCsvFile.seek(lLastFilePos);
    if(WriteStdfFile(lCsvFile, lFileNameSTDF) != true)
    {
        QFile::remove(lFileNameSTDF);
        // Close file
        lFile.close();
        return false;
    }

    mLastError = errNoError;
    // Success parsing CSV file
    return true;
}

bool CGCSVSkyworkstoSTDF::ProcessMultiLimits(const QMap<int, QPair<QString, QString> > &lLimitsCase)
{
    QMapIterator<int, QPair<QString, QString> > itLimitsCase(lLimitsCase);
    QStringList     lLowLimitsFields;
    QStringList     lHighLimitsFields;
    bool            lOk;
    double          lValue;
    GS::Core::MultiLimitItem   lMLSet;
    int             lHardBin = -1;
    int             lSoftBin = -1;
    QRegExp         lLowLimitRegExp(sLowLimitPattern, Qt::CaseInsensitive);

    while (itLimitsCase.hasNext())
    {
        itLimitsCase.next();

        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Processing multi-limit for Limit Case %1").arg(itLimitsCase.key())
              .toLatin1().constData());

        // Reset variables
        lLowLimitsFields.clear();
        lHighLimitsFields.clear();

        // Get csv lines for low and high limit case
        if (itLimitsCase.value().first.isEmpty() == false)
            lLowLimitsFields = itLimitsCase.value().first.split(",", QString::KeepEmptyParts);

        if (itLimitsCase.value().second.isEmpty() == false)
            lHighLimitsFields = itLimitsCase.value().second.split(",", QString::KeepEmptyParts);

        if (lLowLimitsFields.count() > 0 &&
            lLowLimitsFields.count() < (int) (CSV_RAW_FIRST_DATA + mTotalParameters))
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields in Lower Limit For Case %1 (%2 instead of %3 expected)")
                  .arg(itLimitsCase.key()).arg(lLowLimitsFields.count())
                  .arg(CSV_RAW_FIRST_DATA + mTotalParameters).toLatin1().constData());
            return false;
        }

        if (lHighLimitsFields.count() > 0 &&
            lHighLimitsFields.count() < (int) (CSV_RAW_FIRST_DATA + mTotalParameters))
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields in Upper Limit For Case %1 (%2 instead of %3 expected)")
                  .arg(itLimitsCase.key()).arg(lHighLimitsFields.count())
                  .arg(CSV_RAW_FIRST_DATA + mTotalParameters).toLatin1().constData());
            return false;
        }

        // Extract Hbin and Sbin informations
        if (lLowLimitsFields.count() > 0)
        {
            lHardBin = lLowLimitsFields.at(CSV_RAW_HBIN).toInt(&lOk);

            if (!lOk)
                lHardBin = -1;

            if (lLowLimitRegExp.exactMatch(lLowLimitsFields.at(CSV_RAW_SBIN)))
            {
                lSoftBin = lLowLimitRegExp.cap(1).toInt(&lOk);

                if (!lOk)
                    lSoftBin = -1;
            }
        }
        else
        {
            lHardBin = lHighLimitsFields.at(CSV_RAW_HBIN).toInt(&lOk);

            if (!lOk)
                lHardBin = -1;

            if (lLowLimitRegExp.exactMatch(lHighLimitsFields.at(CSV_RAW_SBIN)))
            {
                lSoftBin = lLowLimitRegExp.cap(1).toInt(&lOk);

                if (!lOk)
                    lSoftBin = -1;
            }
        }

        // Add multi-limit set for each parameter
        for(unsigned int lIdx = 0; lIdx < mTotalParameters; ++lIdx)
        {
            lMLSet.Clear();

            lMLSet.SetHardBin(lHardBin);
            lMLSet.SetSoftBin(lSoftBin);

            // Extract Low limit value
            if (lLowLimitsFields.count() > 0)
            {
                lValue = lLowLimitsFields[CSV_RAW_FIRST_DATA + lIdx].toDouble(&lOk);
                if (lOk && lValue > -1e+50)
                {
                    lMLSet.SetLowLimit(lValue);
                }
            }

            // Extract High limit value
            if (lHighLimitsFields.count() > 0)
            {
                lValue = lHighLimitsFields[CSV_RAW_FIRST_DATA + lIdx].toDouble(&lOk);
                if (lOk && lValue < 1e+50)
                {
                    lMLSet.SetHighLimit(lValue);
                }
            }

            // When no low limit nor high limit defined, add multi-limit set only
            // if there were at leats on valid multi-limits set found
            if (lMLSet.IsValidHighLimit() == false && lMLSet.IsValidLowLimit() == false)
            {
                if (mCGCsvParameter[lIdx].GetMultiLimitCount() > 0)
                    mCGCsvParameter[lIdx].AddMultiLimitItem(lMLSet);
            }
            else
                mCGCsvParameter[lIdx].AddMultiLimitItem(lMLSet, ParserParameter::KeepDuplicateMultiLimit);
        }
    }

    return true;
}




//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool CGCSVSkyworkstoSTDF::WriteStdfFile(QTextStream& lCsvFile, const QString& lFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;

    if(lStdfFile.Open(lFileNameSTDF.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }

    // Ensure CPU type (for encoding STDF records) is the one of the computer platform running the codes.
    lStdfFile.SetStdfCpuType(lStdfFile.GetComputerCpuType());

    // Write FAR
    GQTL_STDF::Stdf_FAR_V4 lFARrecord;
    lFARrecord.SetCPU_TYPE(lStdfFile.GetComputerCpuType());	 // Force CPU type to current computer platform.
    lFARrecord.SetSTDF_VER(4);                               // STDF V4
    lFARrecord.Write(lStdfFile);


    QStringList lFields;
    do
    {
        lFields = ReadLine(lCsvFile).split(",");
    }while (lFields[0].simplified() == "");

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    lMIRRecord.SetPART_TYP(lFields[0]);
    lMIRRecord.SetJOB_NAM(lFields[3]);
    lMIRRecord.SetMODE_COD(lFields[4].toShort());
    lMIRRecord.SetNODE_NAM(lFields[7]);
    lMIRRecord.SetOPER_NAM(lFields[CSV_RAW_OPERATOR]);
    lMIRRecord.SetLOT_ID(lFields[CSV_RAW_LOT_ID]);
    // Date MM/DD/YY
    QStringList lDateList = lFields[11].split('/');
    QDate lDate;
    if (lDateList.count() == 3)
    {
        int lYear = lDateList[2].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;

        if (! lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt()))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lFields[11]).toUtf8().constData());
        }
    }
    QTime lTime = QTime::fromString(lFields[12], "hh:mm:ss");
    mStartTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    lMIRRecord.SetSTART_T(mStartTime);

    // Add undefined values in the cvs and mandatory to the STDF
    lMIRRecord.SetSETUP_T(mStartTime);
    lMIRRecord.SetSTAT_NUM(1);
    lMIRRecord.SetMODE_COD(' ');
    lMIRRecord.SetRTST_COD(' ');
    lMIRRecord.SetPROT_COD(' ');
    lMIRRecord.SetBURN_TIM(65535);
    lMIRRecord.SetCMOD_COD(' ');
    lMIRRecord.SetTSTR_TYP("");
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":CSV";
    lMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    lMIRRecord.Write(lStdfFile);

    // Write SDR
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
    lSDRRecord.SetLOAD_ID(lFields[6]);
    // Set undefined values
    lSDRRecord.SetHEAD_NUM(1);
    lSDRRecord.SetSITE_GRP(1);
    lSDRRecord.SetSITE_CNT(1);
    lSDRRecord.SetSITE_NUM(1, 1);
    lSDRRecord.Write(lStdfFile);


    // Write Test results for each line read.
    QString strString;
    BYTE	lValidLimit;
    bool    lMultiLimitWritten = false;

    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    do
    {
        // Write PIR
        GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
        lPIRRecord.SetHEAD_NUM(1);
        lPIRRecord.SetSITE_NUM(lFields[CSV_RAW_SITE].toUShort());
        lPIRRecord.Write(lStdfFile);

        // Read Parameter results for this record
        for(unsigned lIndex=0; lIndex<mTotalParameters; ++lIndex)
        {
            // If it's a PTR (pattern name empty)
            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            lPTRRecord.SetTEST_NUM(mCGCsvParameter[lIndex].GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            lPTRRecord.SetSITE_NUM(lFields[CSV_RAW_SITE].toUShort());

            // GCORE-8570 : HTH
            // Limit must not be strict. Measurament equal to the low or high limit must be considered as
            // PASS
            lPTRRecord.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80)); // B*1 Parametric test flags (drift, etc.)

            bool ok;
            float lValue = lFields[CSV_RAW_FIRST_DATA + lIndex].toFloat(&ok);
            if (ok)
            {
                lPTRRecord.SetTEST_FLG(0x40);       // No result
                lPTRRecord.SetRESULT(lValue);
            }
            else
            {
                lPTRRecord.SetTEST_FLG(0x42);       // No result, no Pass/fail indication
                lPTRRecord.SetRESULT(0);
            }
            lPTRRecord.SetTEST_TXT(mCGCsvParameter[lIndex].GetTestName());

            lPTRRecord.SetRES_SCAL(0);
            lPTRRecord.SetLLM_SCAL(0);
            lPTRRecord.SetHLM_SCAL(0);
            lValidLimit = 0x0e;
            if(mCGCsvParameter[lIndex].GetValidLowLimit() == false)
                lValidLimit |=0x40;
            if(mCGCsvParameter[lIndex].GetValidHighLimit() == false)
                lValidLimit |=0x80;
            lPTRRecord.SetOPT_FLAG(lValidLimit);
            lPTRRecord.SetLO_LIMIT(mCGCsvParameter[lIndex].GetLowLimit());
            lPTRRecord.SetHI_LIMIT(mCGCsvParameter[lIndex].GetHighLimit());
            lPTRRecord.SetUNITS(mCGCsvParameter[lIndex].GetTestUnits());
            lPTRRecord.Write(lStdfFile);

            if (lMultiLimitWritten == false)
            {
                for (int lML = 0; lML < mCGCsvParameter[lIndex].GetMultiLimitCount(); ++lML)
                {
                     GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
                     QJsonObject            lMultiLimit;
                     const GS::Core::MultiLimitItem&   lMLSet = mCGCsvParameter[lIndex].GetMultiLimitSetAt(lML);

                     lMultiLimit.insert("TYPE", QJsonValue(QString("ML")));
                     lMultiLimit.insert("TNUM", QJsonValue((int)mCGCsvParameter[lIndex].GetTestNumber()));

                     if (lMLSet.IsValidHardBin())
                        lMultiLimit.insert("HBIN", QJsonValue(lMLSet.GetHardBin()));

                     if (lMLSet.IsValidSoftBin())
                        lMultiLimit.insert("SBIN", QJsonValue(lMLSet.GetSoftBin()));

                     if (lMLSet.IsValidLowLimit())
                         lMultiLimit.insert("LL", QJsonValue(lMLSet.GetLowLimit()));

                     if (lMLSet.IsValidHighLimit())
                         lMultiLimit.insert("HL", QJsonValue(lMLSet.GetHighLimit()));

                     lDTRRecord.SetTEXT_DAT(lMultiLimit);

                     if (lDTRRecord.Write(lStdfFile) == false)
                     {
                         GSLOG(SYSLOG_SEV_ERROR,
                               QString("Failed to write multi-limits DTR").toLatin1().constData());

                         mLastError = errWriteSTDF;
                         return false;
                     }
                }
            }
        };

        // Multi-limits are only written once. Flag them as written now.
        lMultiLimitWritten = true;

        // Write PRR
        GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
        ushort lHardBin = lFields[CSV_RAW_HBIN].toUShort();
        lPRRRecord.SetPART_TXT(lFields[2]);
        lPRRRecord.SetPART_ID(lFields[10]);
        lPRRRecord.SetTEST_T(lFields[13].toULong());
        lPRRRecord.SetSITE_NUM(lFields[CSV_RAW_SITE].toUShort());
        lPRRRecord.SetHARD_BIN(lHardBin);
        lPRRRecord.SetSOFT_BIN(lFields[CSV_RAW_SBIN].toUShort());
        // Part is considered good as long its hard bin is within the range 1 to 3
        if (lHardBin >= 1 && lHardBin <= 3)
            lPRRRecord.SetPART_FLG(0x00);
        else
            lPRRRecord.SetPART_FLG(0x08);

        lPRRRecord.Write(lStdfFile);

        // Read lines while empty
        strString = ReadLine(lCsvFile);
        while (strString.trimmed().isEmpty() && !lCsvFile.atEnd())
        {
            strString = ReadLine(lCsvFile);
        }

        if (!strString.trimmed().isEmpty())
        {    // Split line if not empty
            lFields = strString.split(",",QString::KeepEmptyParts);
        }
    }
    while(lCsvFile.atEnd() == false); // Read all lines in file


    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    // use the last date in the csv file
    time_t lFinishTime = -1;
    if (lFields.count() > 11)
    {
        QDate lDate;
        if (lDateList.count() == 3)
        {
            int lYear = lDateList[2].toInt();
            if (lYear < 70)
                lYear += 2000;
            else if (lYear < 99)
                lYear += 1900;

            // MM/DD/YY
            if (! lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt()))
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("Invalid date %1").arg(lFields[11]).toUtf8().constData());
            }
        }
        lTime = QTime::fromString(lFields[12], "hh:mm:ss");
        lFinishTime = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    }
    lMRRRecord.SetFINISH_T(lFinishTime);
    lMRRRecord.SetDISP_COD(' ');
    lMRRRecord.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}


//////////////////////////////////////////////////////////////////////
// Skip empty line
//////////////////////////////////////////////////////////////////////
void  CGCSVSkyworkstoSTDF::SpecificReadLine (QString &strString)
{
    if(strString.left(3) == ",,," && (strString.simplified().count(",")==strString.simplified().length()))
        strString = "";
}

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool CGCSVSkyworkstoSTDF::EmptyLine(const QString& line)
{
    bool lEmpty(true);
    if (!line.isEmpty())
    {
        QStringList lCells = line.split(",", QString::KeepEmptyParts);
        for(int lIndexCell=0; lIndexCell<lCells.count(); ++lIndexCell)
        {
            if (!lCells[lIndexCell].isEmpty())
            {
                lEmpty = false;
                break;
            }
        }
    }
    return lEmpty;
}

}
}
