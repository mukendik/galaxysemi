//////////////////////////////////////////////////////////////////////
// import_semi_g85.cpp: Convert a SEMI_G85 file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importMicronSUM.h"
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

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
MicronSumToSTDF::MicronSumToSTDF() : MicronParserBase(typeMicronSum, "typeMicronSum")
{
    mParameterDirectory.SetFileName(GEX_TRI_MICROSUM_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
MicronSumToSTDF::~MicronSumToSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with EGL Skywork format
//////////////////////////////////////////////////////////////////////
bool MicronSumToSTDF::IsCompatible(const QString &FileName)
{
    bool lIsCompatible(false);

    if (!FileName.contains(".sum", Qt::CaseInsensitive))
    {
        return false;
    }

    QFile lFile(FileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can not open file %1").arg(FileName).toLatin1().constData());
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputFile(&lFile);


    // Check the compatibility
    QString lStrString("");
    while (lStrString.isEmpty()
           && !lInputFile.atEnd())
    {
        lStrString = lInputFile.readLine().trimmed();
    }

    if (lStrString.contains("$$_BEGIN_HEADER", Qt::CaseInsensitive))
    {
        lIsCompatible = true;
    }

    // Read the next line to confirm that we are reading the Micron Sum map
    if (lIsCompatible)
    {
        lStrString = lInputFile.readLine().trimmed();
        if (!lStrString.contains("ASSET_ID"))
        {
            lIsCompatible = false;
        }
    }

    // Close file
    lFile.close();
    return lIsCompatible;
}


//////////////////////////////////////////////////////////////////////
bool MicronSumToSTDF::ConvertoStdf(const QString &micronFileName,
                                    QString &StdfFileName)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg( micronFileName).toLatin1().constData());

    // Open Micron file
    QFile lFile( micronFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Micron file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lStreamFile(&lFile);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    //mFileSize = lFile.size()+1;

    if (!ReadHeaderSection(lStreamFile))
    {
        mLastError = errInvalidFormatParameter;
        QFile::remove(StdfFileName);
        // Close file
        lFile.close();
        return false;
    }


    if(WriteStdfFile(lStreamFile, StdfFileName) != true)
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
bool MicronSumToSTDF::WriteStdfFile(QTextStream& micronSumStreamFile, const QString &StdfFileName)
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
    strUserTxt += ":MICRON_SUM_MAP";
    mMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    mMIRRecord.Write(lStdfFile);

    mWIRRecord.SetHEAD_NUM(1);
    mWIRRecord.SetSITE_GRP(255);
    mWIRRecord.Write(lStdfFile);

    QList<GQTL_STDF::Stdf_SBR_V4*> lSBRRecordList;
    QMap<unsigned short, GQTL_STDF::Stdf_HBR_V4*> lHBRRecordMap;
    QList< QPair<unsigned int, QString> > lTestList;

    if (!ReadSummarySection(micronSumStreamFile, lSBRRecordList, lHBRRecordMap))
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    /// From the definitions beginning with TREND_DEF
    ReadTestsDefinitionSection(micronSumStreamFile, mParameterDirectory, lTestList, "TREND_DEF");

    // Read test results
    QHash< QPair<qint16, qint16>, QStringList > lDieList;
    ReadDieDataSection(micronSumStreamFile, lDieList, true, "^", lStdfFile, lTestList);


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


}
}
