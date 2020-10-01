//////////////////////////////////////////////////////////////////////
// import_semi_g85.cpp: Convert a SEMI_G85 file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importMicron.h"
#include "importConstants.h"
#include <gqtl_archivefile.h>
#include <gqtl_log.h>

#include <QFileInfo>
#include <QTemporaryDir>

//$$_BEGIN_HEADER
//ASSET_OID: 0x05a4e6f398030080
//ATTRIBUTE_SAMPLING: ESDA
//COMPILE_HOST: f08m6pc1d0.mava.micron.com
//CONTINUITY_FAILURE: 0
//CONTINUITY_TEST: 0
//CONVERT_ETIME: 1418637353
//CONVERT_SITE: MANASSAS
//DATA_SET_ID: YVTL5XrBG1uhL84oQ
//DESIGN_ID: V80A
//DIE_SIZE: 0.7048
//FAB: 6
//FID_LOT: 8680156
//FINISH_DATETIME: 12/15/2014 04:26:05
//FINISH_ETIME: 1418635565
//FLAT_POSITION: 270
//GOOD_DIE: 821
//INTERFACE: NA
//LEGACY_REPROBE: 0
//LIBRARY_REVISION: UNKNOWN
//LOG_REV: 2.0_4894
//LOT: 8680156.0
//LOT_ID: 8680156.006
//LOT_PREFIX: 8680156
//MAJOR_PROBE_PROGRAM_REV: 50
//MAKE_OPT: UND
//MAP_REV: 6F069
//MAP_VERS: 6f069
//MAX_FAILS_PER_REGION: 547072
// .
// .
// .
// ...
//$$_END_HEADER
//$$_BEGIN_SUMMARY 2.0
//#define ENGREG_DEF FE_PTRES_DIE_STDEV 00 The Ptres Standard Deviation for the die.
//#define FM_BIN_DEF . PASS BIN_1 00 (REG# 1) Good die
//#define FM_BIN_DEF H FAIL BIN_H 00 (REG# 100) Opens, Inputs
//#define FM_BIN_DEF i FAIL BIN_i 00 (REG# 200) Force fail on die with rde > 8
//#define FM_BIN_DEF I FAIL BIN_I 00 (REG# 300) Vcc Shorts
//#define FM_BIN_DEF X FAIL BIN_X 00 (REG# 400) IBB/INWL/IISO fail, passes bin V

#define MICRON_HEADER_FILE                      "header"
#define MICRON_SUMMARY_FILE                     "summary"
#define MICRON_TREND_FILE                       "trend.prblog"
#define MICRON_TREND_DEFINES_FILE               "trend_defines.prblog"
#define MICRON_FUNCTIONAL_DIE_DATA_FILE         "functional_die_data"
#define MICRON_SECURE_FUNCTIONAL_DIE_DATA_FILE  "secure_functional_die_data"
#define MICRON_TREND_DEFINES_ECAT_FILE          "trend_defines_ecat"
#define MICRON_TREND_DATA_ECAT_FILE             "trend_data_ecat"

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
MicronToSTDF::MicronToSTDF() : MicronParserBase(typeMicron, "typeMicron")
{
    mParameterDirectory.SetFileName(GEX_TRI_MICRO_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
MicronToSTDF::~MicronToSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool MicronToSTDF::IsCompatible(const QString &FileName)
{
    bool            lIsCompatible(false);
    QStringList     lUncompressedFiles;
    QTemporaryDir   lTempDir;
    CArchiveFile    lArchiveFile(lTempDir.path());

    if(lArchiveFile.IsCompressedFile(FileName))
    {
        // Extract file list from the archive
        bool lResult = lArchiveFile.ExtractFileList(FileName, lUncompressedFiles);
        if(lResult == false)
        {
            GSLOG(SYSLOG_SEV_WARNING,
                 QString("Failed to uncompress file: %1").arg(FileName).toLatin1().constData());
        }
        else
        {
            lIsCompatible = MicronToSTDF::CheckMandatoryInputFiles(lUncompressedFiles);
        }
    }

    return lIsCompatible;
}

bool MicronToSTDF::IsCompressedFormat() const
{
    return true;
}

bool MicronToSTDF::CheckMandatoryInputFiles(const QStringList &lInputFiles)
{
    QStringList lMandatoryFiles;
    bool        lValid = false;

    // List of file names expected to be found in the compressed files
    // Archive file is considered as compatible with the Micron parser as soon as
    // those four files exist in the archive
    lMandatoryFiles << MICRON_HEADER_FILE << MICRON_SUMMARY_FILE << MICRON_TREND_DEFINES_FILE << MICRON_TREND_FILE;

    // If filelist in the compressed file is smaller than the expected one,
    // the archive is not compatible with Micron parser
    // Otherwise, check that every mandatory file exist in the archive
    if (lInputFiles.count() >=  lMandatoryFiles.count())
    {
        lValid = true;

        for(int lIdx = 0; lIdx < lMandatoryFiles.count() && lValid; ++lIdx)
        {
            lValid &= lInputFiles.contains(lMandatoryFiles.at(lIdx), Qt::CaseInsensitive);
        }
    }

    return lValid;
}

//////////////////////////////////////////////////////////////////////
bool MicronToSTDF::ConvertoStdf(const QString &micronFileName,  QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( micronFileName).toLatin1().constData());

    QTemporaryDir   lTempDir;
    QStringList     lUncompressedFiles;
    CArchiveFile    lArchive(lTempDir.path());

    if (lArchive.Uncompress(micronFileName, lUncompressedFiles) == false)
    {
        // Failed Opening Micron file
        mLastError = ErrUncompressFail;

        // Convertion failed.
        return false;
    }

    if (CheckMandatoryInputFiles(lUncompressedFiles) == false)
    {
        // Missing input files
        mLastError = ErrMissingMandatoryFile;

        // Conversion failed
        return false;
    }

    for (int lIdx = 0; lIdx < lUncompressedFiles.count(); ++lIdx)
    {
        QFileInfo lFileInfo(QDir(lTempDir.path()), lUncompressedFiles.at(lIdx));

        mInputFiles.insert(lFileInfo.fileName().toLower(), lFileInfo.absoluteFilePath());
    }

    // Open Micron file
    QString lHeaderFile = mInputFiles.value(MICRON_HEADER_FILE);
    QFile lFile( lHeaderFile );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Micron file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lStreamFile(&lFile);

    if (!ReadHeaderSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }


    if(WriteStdfFile(StdfFileName) != true)
    {
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }

    // Close file
    lFile.close();

    mLastError = errNoError;
    // Success parsing Micron file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from Micron data parsed
//////////////////////////////////////////////////////////////////////
bool MicronToSTDF::WriteStdfFile(const QString &StdfFileName)
{
    // now generate the STDF file...
    GS::StdLib::Stdf lStdfFile;
    if(lStdfFile.Open(StdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing Micron file into STDF database
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

    // Write MIR
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":MICRON";
    mMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    mMIRRecord.Write(lStdfFile);

    mWIRRecord.SetHEAD_NUM(1);
    mWIRRecord.SetSITE_GRP(255);
    mWIRRecord.Write(lStdfFile);

    QList<GQTL_STDF::Stdf_SBR_V4*> lSBRRecordList;
    QMap<unsigned short, GQTL_STDF::Stdf_HBR_V4*> lHBRRecordMap;
    QList< QPair<unsigned int, QString> > lTestList;
//    QString lLine("");

    QString lSummaryFileName = mInputFiles.value(MICRON_SUMMARY_FILE);
    if (!ReadSummary(lSummaryFileName, lSBRRecordList, lHBRRecordMap))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

        // to be decommented if needed
//        if (lLine.startsWith("$$_BEGIN_G85_MAP"))
//        {
//            QList<GQTL_STDF::Stdf_PRR_V4 *> lPRRRecordsList;
//            QList<short> passHBin;
//            GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
//            lPIRRecord.SetHEAD_NUM(1);
//            lPIRRecord.SetSITE_NUM(1);
//            if (ReadG85Maps(inputFile, lPRRRecordsList, passHBin))
//            {
//                for (int i=0; i<lPRRRecordsList.size(); ++i)
//                {
//                    // Write PIR
//                    lPIRRecord.Write(lStdfFile);

//                    // Write PRR
//                    lPRRRecordsList[i]->Write(lStdfFile);
//                }
//                qDeleteAll(lPRRRecordsList);
//                lPRRRecordsList.clear();
//            }

//        }

//    }

    /// Read the 3 definitions files
    /// Read the 2 smaller die results files (functional and secure functional)
    /// Read the probe log file and write the stdf PIR, PRR and PTR

    /// From the file trend_defines.prblog
    QString lSecureFunctionalDieFile = mInputFiles.value(MICRON_SECURE_FUNCTIONAL_DIE_DATA_FILE, "");
    if (!lSecureFunctionalDieFile.isEmpty())
        ReadTestsDefinition(lSecureFunctionalDieFile, mParameterDirectory, lTestList, "SFDD_DEF");
    QString lFunctionalDieFile = mInputFiles.value(MICRON_FUNCTIONAL_DIE_DATA_FILE, "");
    if (!lFunctionalDieFile.isEmpty())
        ReadTestsDefinition(lFunctionalDieFile, mParameterDirectory, lTestList, "FDD_DEF");

    // Read ECAT test defines file
    QString lEcatDefinesFile = mInputFiles.value(MICRON_TREND_DEFINES_ECAT_FILE, "");
    if (!lEcatDefinesFile.isEmpty())
        ReadTestsDefinition(lEcatDefinesFile, mParameterDirectory, lTestList, "TREND_DEF");

    QString lTrendDefinesFile = mInputFiles.value(MICRON_TREND_DEFINES_FILE, "");
    if (lTrendDefinesFile.isEmpty())
    {
        mLastError = ErrMissingMandatoryFile;
        return false;
    }
    ReadTestsDefinition(lTrendDefinesFile, mParameterDirectory, lTestList, "TREND_DEF");

    QHash< QPair<qint16, qint16>, QStringList > lDieData;
    if (!lSecureFunctionalDieFile.isEmpty())
        ReadDieData(lSecureFunctionalDieFile, lDieData, false, "SFDD", lStdfFile, lTestList);
    if (!lFunctionalDieFile.isEmpty())
        ReadDieData(lFunctionalDieFile, lDieData, false, "FDD", lStdfFile, lTestList);

    // Read ECAT data file
    QString lEcatDataFile = mInputFiles.value(MICRON_TREND_DATA_ECAT_FILE, "");
    if (!lEcatDataFile.isEmpty())
        ReadDieData(lEcatDataFile, lDieData, false, "^", lStdfFile, lTestList);

    QString lTrendLogFile = mInputFiles.value(MICRON_TREND_FILE, "");
    if (lTrendDefinesFile.isEmpty())
    {
        mLastError = ErrMissingMandatoryFile;
        return false;
    }
    ReadDieData(lTrendLogFile, lDieData, true, "^", lStdfFile, lTestList);


    // All tests names read...check if need to update the Micron Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();


    // Write SBR
    for (int i=0; i<lSBRRecordList.size(); ++i)
        lSBRRecordList[i]->Write(lStdfFile);
    // Write HBR
    QList<GQTL_STDF::Stdf_HBR_V4*> lHBRRecordList = lHBRRecordMap.values();
    for (int i=0; i<lHBRRecordList.size(); ++i)
        lHBRRecordList[i]->Write(lStdfFile);
    // Destroy list of Bin tables.
    qDeleteAll(lSBRRecordList);
    lSBRRecordList.clear();
    qDeleteAll(lHBRRecordMap.values());
    lHBRRecordMap.clear();

    // Write the last WRR
    mWRRRecord.SetHEAD_NUM(1);
    mWRRRecord.SetSITE_GRP(255);
    mWRRRecord.Write(lStdfFile);

    // Write MRR
    mMRRRecord.Write(lStdfFile);

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

std::string MicronToSTDF::GetErrorMessage(const int ErrorCode) const
{
    QString lError;

    switch(ErrorCode)
    {
        case ErrUncompressFail:
            lError += "Fail to uncompress Micron input file";
            break;

        case ErrMissingMandatoryFile:
            lError += "Some mandatory files are missing from the archive";
            break;

        case errInvalidFormatParameter:
            lError += "Some mandatory fiels are missing";

        default:
            lError += QString::fromStdString(ParserBase::GetErrorMessage(ErrorCode));
            break;
    }

    return lError.toStdString();
}



bool MicronToSTDF::ReadSummary(const QString &fileName,
                               QList<GQTL_STDF::Stdf_SBR_V4*>& lSBRRecordList,
                               QMap<unsigned short, GQTL_STDF::Stdf_HBR_V4*>& lHBRRecordList)
{
    QFile lInputSummaryFile(fileName);
    if(!lInputSummaryFile.open( QIODevice::ReadOnly))
    {
        // Failed Opening Micron file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFileStream(&lInputSummaryFile);
    bool lRet = ReadSummarySection(lInputFileStream, lSBRRecordList, lHBRRecordList);
    lInputSummaryFile.close();
    return lRet;
}


/*
 * to be decommented if needed
bool MicronToSTDF::ReadG85Maps(QTextStream &inputFile,
                            QList<GQTL_STDF::Stdf_PRR_V4 *> &lPRRRecordsList,
                            QList<short>& passHBin)
{
    QString lLine("");
    while(!inputFile.atEnd()
          && !lLine.startsWith("$$_END_G85_MAP"))
    {
        lLine = ReadLine(inputFile);
        // read the hard bin
        if (lLine.startsWith("<Data", Qt::CaseInsensitive)
            && lLine.contains("MapName", Qt::CaseInsensitive))
        {
            // Hard bins
            if (lLine.contains("\"ECM\""))
            {
                bool lOk;
                short lBin;
                while (!lLine.startsWith("<Row>", Qt::CaseInsensitive)
                       && !lLine.startsWith("</Data>", Qt::CaseInsensitive))
                {
                    if(lLine.contains("BinQuality=\"PASS\""))
                    {
                        passHBin.append(lLine.section("\"", 1, 1).toShort(&lOk, 16));
                    }
                    lLine = ReadLine(inputFile);
                }
                if (lLine.startsWith("<Row>", Qt::CaseInsensitive))
                {
                    int lColumn(0), lRow(0);
                    while (lLine.startsWith("<Row>", Qt::CaseInsensitive))
                    {
                        lColumn = 0;
                        lLine.remove("<Row>");
                        lLine.remove("</Row>");
                        // Read bin results for this record
                        for(int lIndex=0; lIndex<(int)lLine.length(); lIndex=lIndex+2)
                        {
                            lBin = (lLine.mid(lIndex, 2)).toShort(&lOk, 16);

                            if (lBin != 0)
                            {
                                GQTL_STDF::Stdf_PRR_V4 * lPRRRecord = new GQTL_STDF::Stdf_PRR_V4();
                                lPRRRecord->SetHEAD_NUM(1);
                                lPRRRecord->SetSITE_NUM(1);
                                char lFlag = 0;
                                if (!passHBin.contains(lBin)) lFlag = 0x8;
                                lPRRRecord->SetPART_FLG(lFlag);
                                lPRRRecord->SetNUM_TEST(0);
                                lPRRRecord->SetHARD_BIN(lBin);
                                lPRRRecord->SetX_COORD(lColumn);
                                lPRRRecord->SetY_COORD(lRow);
                                lPRRRecordsList.append(lPRRRecord);
                            }
                            ++lColumn;
                        }
                        lLine = ReadLine(inputFile);
                        ++lRow;
                    }
                }
            }


            // Soft bins
            if (lLine.contains("\"FMM\""))
            {
                bool lOk;
                short lBin;
                while (!lLine.startsWith("<Row>", Qt::CaseInsensitive)
                       && !lLine.startsWith("</Data>", Qt::CaseInsensitive))
                {
                    lLine = ReadLine(inputFile);
                }
                if (lLine.startsWith("<Row>", Qt::CaseInsensitive))
                {
                    int lDiePos(0);
                    while (lLine.startsWith("<Row>", Qt::CaseInsensitive))
                    {
                        lLine.remove("<Row>").remove("</Row>");

                        // Read bin results for this record
                        for(int lIndex=0; lIndex<(int)lLine.length(); lIndex=lIndex+2)
                        {
                            lBin = (lLine.mid(lIndex, 2)).toShort(&lOk, 16);
                            if (lDiePos<lPRRRecordsList.size())
                                lPRRRecordsList[lDiePos]->SetSOFT_BIN(lBin);
                            ++lDiePos;
                        }
                    }
                }
            }

        }
    }
    if (inputFile.atEnd())
        return false;
    else
        return true;

}*/

/// From the file trend_defines.prblog
bool MicronToSTDF::ReadTestsDefinition(const QString &inputTestsDefName,
                                       ParameterDictionary &parameterDirectory,
                                       QList< QPair<unsigned int, QString> >& testList,
                                       const QString keyWord)
{
    /// Open trend_defines.prblog file
    QFile lInputTestsDefFile(inputTestsDefName);
    if(!lInputTestsDefFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Micron file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lInputTestsDefFile);
    bool lRet = ReadTestsDefinitionSection(lInputFile, parameterDirectory, testList, keyWord);

    // Close STDF file.
    lInputTestsDefFile.close();
    return lRet;
}


bool MicronToSTDF::ReadDieData(QString fileName,
                               QHash< QPair<qint16, qint16>, QStringList > &dieData,
                               bool writeRecord,
                               const QString keyWord,
                               GS::StdLib::Stdf& lStdfFile,
                               QList< QPair<unsigned int, QString> >& testList)
{
    /// Open trend_defines.prblog file
    QFile lInputDieDataFile(fileName);
    if(!lInputDieDataFile.open( QIODevice::ReadOnly))
    {
        // Failed Opening Micron file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lInputDieDataFile);
    //mFileSize = lInputDieDataFile.size()+1;
    bool lRet = ReadDieDataSection(lInputFile, dieData, writeRecord, keyWord, lStdfFile, testList);

    // Close STDF file.
    lInputDieDataFile.close();
    return lRet;
}

}
}
