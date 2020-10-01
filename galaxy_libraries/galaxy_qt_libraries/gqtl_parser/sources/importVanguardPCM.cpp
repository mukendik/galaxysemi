#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "gqtl_log.h"
#include "importVanguardPCM.h"




// File format:
//                                                  W.A.T. DATA ATTACHED
// Customer:, B94 ICsense, VIS Product ID:, VP5698A-LZNAZ1
// Customer Product ID:, Robin A1
// VIS Lot ID:, F6K559.3
// Process:, 025HB35LE5, PCM Spec:, VP5698
// WAT Date:, 11/20/2016, Wafer Count:, 3
// Ship Date:, 11/22/2016
// Wafer, Site, BV_20uA, BV_80DN1C, BV_80DN2W, BV_80DP1, BV_N2I2, BV_N35, BV_N4, BV_N5I2, BV_P2I2, BV_P35, BV_P4, BV_P5I2, CONTI_M1,
// ID, ID, hdio5, V 10nA, V 10nA, V 10nA, V 10/.24, V 10/0.5, V 10/0.24, V 10/.5, V 10/.24, V 10/0.5, V 10/0.24, V 10/.5, OHM.32/.32,
// 10, 1, 5.6000, 99.5000, 95.0000, -130.0000, 6.7000, 9.5200, 6.0700, 9.6000, -7.1000, -8.3700, -7.0700, -8.4500, 260.8580, 166.0160,
// 10, 2, 5.6000, 101.0000, 98.0000, -130.0000, 6.6500, 9.5900, 6.1000, 9.6000, -7.1500, -8.3600, -7.0700, -8.4500, 248.7560, 159.5150,
// ....


namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
VanguardPCMToStdf::~VanguardPCMToStdf()
{
    mOutputFiles.clear();
    mParameterList.clear();
}

VanguardPCMToStdf::VanguardPCMToStdf():ParserBase(typeVanguardPcm, "VanguardPcm")
{
    mOutputFiles.clear();
    mParameterList.clear();
    mParameterDirectory.SetFileName(GEX_VANGUARD_PCM_PARAMETERS);
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT format
//////////////////////////////////////////////////////////////////////
bool VanguardPCMToStdf::IsCompatible(const QString& aFileName)
{
    // Open Vanguard PCM file
    QFile lInputFile(aFileName);
    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        // Failed Opening input file
        return false;
    }

    // Find the correct Vanguard PCM header in the 10 first lines ...
    QTextStream lVanguardPCMStream(&lInputFile);
    QString lString;

    do
    {
        lString = lVanguardPCMStream.readLine();
    }
    while(!lString.isNull() && lString.isEmpty());

    lString = lString.trimmed();	// remove leading spaces.
    if(!lString.contains("Customer:", Qt::CaseInsensitive)
            || !lString.contains("VIS Product ID:", Qt::CaseInsensitive))
    {
        lInputFile.close();
        return false;
    }

    // Read the second non empty line
    do
    {
        lString = lVanguardPCMStream.readLine();
    }
    while(!lString.isNull() && lString.isEmpty());

    if(!lString.contains("Customer Product ID:", Qt::CaseInsensitive))
    {
        lInputFile.close();
        return false;
    }

    // Incorrect header...this is not a Vanguard PCM file!
    lInputFile.close();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert the Vanguard PCM file to a STDF file
//////////////////////////////////////////////////////////////////////
bool VanguardPCMToStdf::ConvertoStdf(const QString& aFileName, QString &aStdfFileName)
{
    UpdateProgressMessage(QString("Converting data (Parsing Vanguard PCM file "+ QFileInfo(aFileName).fileName()+")"));
    if(ReadInputFile(aFileName))
    {
        UpdateProgressMessage(QString("Converting data (Writing STDF file "+ QFileInfo(aStdfFileName).fileName()+")"));
        if(WriteStdfFiles(aFileName, aStdfFileName))
        {
            return true;
        }
        if (!QFile::remove(aStdfFileName))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to remove the output file %1")
                                            .arg(aStdfFileName).toLatin1().constData());
            return false;
        }
    }
    return false;

}


bool VanguardPCMToStdf::CloseFileWithError(QFile& aFile)
{
    aFile.close();
    mLastError = errInvalidFormatParameter;
    return false;
}
//////////////////////////////////////////////////////////////////////
// Parse the Vanguard PCM file
//////////////////////////////////////////////////////////////////////
bool VanguardPCMToStdf::ReadInputFile(const QString& aFileName)
{

    QString lDebugMessage, lLine;
    QString lItem;

    // Debug trace
    lDebugMessage = "---- VanguardPCMToStdf::ReadInputFile(): reading Vanguard PCM file (";
    lDebugMessage += aFileName;
    lDebugMessage += ")";
    GSLOG(SYSLOG_SEV_DEBUG, lDebugMessage.toLatin1().constData());

    // Open Vanguard PCM file
    QFile lFile(aFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Vanguard PCM file
        mLastError = errOpenFail;
        return false;
    }
    // Assign file I/O stream
    QTextStream lInputPCMFile(&lFile);

    // Read the Header. The brackets are only for understanding and not algorithmic perpus.
    {
        // Read line : "Customer:, B94 ICsense, VIS Product ID:, VP5698A-LZNAZ1"
        QString lKeyWord;
        lLine = ReadLine(lInputPCMFile);
        // Read until "Customer Product ID:"
        while (!lLine.contains("Customer Product ID:", Qt::CaseInsensitive) && !lInputPCMFile.atEnd())
        {
            lLine = ReadLine(lInputPCMFile);
        }
        if(lLine.contains("Customer Product ID:", Qt::CaseInsensitive))
        {
            // Get the customer product ID
            lKeyWord = lLine.section(":",1,1);
            // Remove "," if exists
            lKeyWord = lKeyWord.remove(",").trimmed();
            mMIRRecord.SetPART_TYP(lKeyWord);
        }
        else
        {
            return CloseFileWithError(lFile);
        }

        lLine = ReadLine(lInputPCMFile);
        if(lLine.contains("VIS Lot ID:", Qt::CaseInsensitive))
        {
            // Get the VIS Lot ID
            lKeyWord = lLine.section(":",1,1);
            // Remove "," if exists
            lKeyWord =lKeyWord.remove(",").trimmed();
            mMIRRecord.SetLOT_ID(lKeyWord);
        }
        else
        {
            return CloseFileWithError(lFile);
        }


        lLine = ReadLine(lInputPCMFile);
        if (lLine.contains("Process", Qt::CaseInsensitive))
        {
            // Read and add the PROC_ID
            lKeyWord = lLine.section(",",1,1);
            // Remove "," if exists
            lKeyWord = lKeyWord.remove(",").trimmed();
            mMIRRecord.SetPROC_ID(lKeyWord);

            // Read and add the SPEC_VER
            lKeyWord = lLine.section(",",3,3);
            // Remove "," if exists
            lKeyWord = lKeyWord.remove(",").trimmed();
            mMIRRecord.SetSPEC_VER(lKeyWord);

        }
        else
        {
            return CloseFileWithError(lFile);
        }

        lLine = ReadLine(lInputPCMFile);
        if(lLine.contains("WAT Date:", Qt::CaseInsensitive))
        {
            // Get the WAT Date
            lKeyWord = lLine.section(",",1,1);
            // Remove "," if exists
            lKeyWord = lKeyWord.remove(",").trimmed();
            QDateTime lDateTime = QDateTime::fromString(lKeyWord, "MM/dd/yyyy");
            lDateTime.setTimeSpec(Qt::UTC);
            mStartTime = lDateTime.toTime_t();
            mMIRRecord.SetSETUP_T(mStartTime);
            mMIRRecord.SetSTART_T(mStartTime);
        }
        else
        {
            return CloseFileWithError(lFile);
        }

        // Add the User TXT
        QString	lUserTxt(GEX_IMPORT_DATAORIGIN_LABEL);
        lUserTxt += ":";
        lUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
        mMIRRecord.SetUSER_TXT(lUserTxt);                   // user-txt
    }


    // ALREADY READ - Read line with list of Parameters names
    // eg: " Wafer, Site, BV_20uA, BV_80DN1C, BV_80DN2W, BV_80DP1, BV_N2I2, BV_N35, BV_N4, BV_N5I2, BV_P2I2, BV_P35, BV_P4,..."

    // Extract the N column names
    while (!lLine.startsWith("Wafer", Qt::CaseInsensitive) && !lInputPCMFile.atEnd())
    {
        lLine = ReadLine(lInputPCMFile);
    }
    if (!lLine.startsWith("Wafer", Qt::CaseInsensitive))
    {
        return CloseFileWithError(lFile);
    }

    QStringList lTestList = lLine.split(",");
    ParserParameter lParam;
    // The last element is empty. The line endes with a ","
    for (int lIndex=2; lIndex<lTestList.size() - 1; ++lIndex)
    {
        lItem = lTestList[lIndex].trimmed();	// Remove spaces
        lParam.SetTestNumber(mParameterDirectory.UpdateParameterIndexTable(lItem));
        lParam.SetTestName(lItem);
        mParameterList.append(lParam);
    }

    // Read the Unit line
    while (!lLine.contains("ID", Qt::CaseInsensitive) && !lInputPCMFile.atEnd())
    {
        lLine = ReadLine(lInputPCMFile);
    }
    if (!lLine.contains("ID", Qt::CaseInsensitive))
    {
        return CloseFileWithError(lFile);
    }

    lTestList = lLine.split(",", QString::KeepEmptyParts);
    if (mParameterList.size() > (lTestList.size() -2))
    {
        return CloseFileWithError(lFile);
    }
    for (int lIndex=0; lIndex<mParameterList.size(); ++lIndex)
    {
        ParserParameter& lParameter = mParameterList[lIndex];
        lItem = lTestList[lIndex+2].trimmed();	// Remove spaces
        lParameter.SetTestUnit(lItem);
    }

    // To read this file, we have to options:
    // Option 1: read sequentially the file: means, we have to store all values in the tables and write them when we'll
    //    read the high and low limits
    // Option2 : Read all lines without doing any thing until arriving to the HL and LL. Save them and return to the
    //    beginning of the file to read all results and write PTRs. (This is the implemented solution).

    while (!lLine.contains("Spec High", Qt::CaseInsensitive) && !lInputPCMFile.atEnd())
    {
        lLine = ReadLine(lInputPCMFile);
    }
    if (!lLine.contains("Spec High", Qt::CaseInsensitive))
    {
        return CloseFileWithError(lFile);
    }

    lTestList = lLine.split(",", QString::KeepEmptyParts);
    if (mParameterList.size() > (lTestList.size() -2))
    {
        return CloseFileWithError(lFile);
    }
    for (int lIndex=0; lIndex<mParameterList.size(); ++lIndex)
    {
        ParserParameter& lParameter = mParameterList[lIndex];
        lItem = lTestList[lIndex+2].trimmed();	// Remove spaces
        if (lItem.isEmpty())
        {
            lParameter.SetValidHighLimit(false);
        }
        else
        {
            lParameter.SetHighLimit(lItem.toDouble());
            lParameter.SetValidHighLimit(true);
        }
    }

    // Read Low limits
    while (!lLine.contains("Spec Low", Qt::CaseInsensitive) && !lInputPCMFile.atEnd())
    {
        lLine = ReadLine(lInputPCMFile);
    }
    if (!lLine.contains("Spec Low", Qt::CaseInsensitive))
    {
        return CloseFileWithError(lFile);
    }

    lTestList = lLine.split(",", QString::KeepEmptyParts);
    if (mParameterList.size() > (lTestList.size() -2))
    {
        return CloseFileWithError(lFile);
    }
    for (int lIndex=0; lIndex<mParameterList.size(); ++lIndex)
    {
        ParserParameter& lParameter = mParameterList[lIndex];
        lItem = lTestList[lIndex+2].trimmed();	// Remove spaces
        if (lItem.isEmpty())
        {
            lParameter.SetValidLowLimit(false);
        }
        else
        {
            lParameter.SetLowLimit(lItem.toDouble());
            lParameter.SetValidLowLimit(true);
        }
    }

    // All Vanguard file read...check if need to update the Vanguard Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Close file
    lFile.close();

    // Success parsing Vanguard PCM file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from WAT data parsed
//////////////////////////////////////////////////////////////////////
bool VanguardPCMToStdf::WriteStdfFiles(const QString& aInputFileName, const QString& aStdfFileName)
{
    // Open Vanguard PCM file
    QFile lFile(aInputFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Vanguard PCM file
        mLastError = errOpenFail;
        return false;
    }

    QString lLine("");
    QString lPreviousWafer("-1");
    bool lFirstWafer(true);
    QStringList lItems;
    int lFailBin(0), lPassBin(0), lPartCount(0);

    // Assign file I/O stream
    QTextStream lInputPCMFile(&lFile);

    // Read until the first result line
    while (!lLine.startsWith("ID", Qt::CaseInsensitive) && !lInputPCMFile.atEnd())
    {
        lLine = ReadLine(lInputPCMFile);
    }
    lLine = ReadLine(lInputPCMFile);
    if (lLine.isEmpty())
    {
        return CloseFileWithError(lFile);
    }

    GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
    lWIRRecord.SetHEAD_NUM(1);
    lWIRRecord.SetSITE_GRP(255);
    lWIRRecord.SetSTART_T(mStartTime);

    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
    lPIRRecord.SetHEAD_NUM(1);

    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
    lPRRRecord.SetHEAD_NUM(1);

    GQTL_STDF::StdfParse lStdfParser;

    if(lStdfParser.Open(aStdfFileName.toLatin1().constData(), STDF_WRITE) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can't open the input output file' %1")
                                        .arg(aStdfFileName).toLatin1().constData());
        mLastError = errWriteSTDF;
        return false;
    }

    // Write MIR and WIR for the next file
    lStdfParser.WriteRecord(&mMIRRecord);
    while (!lInputPCMFile.atEnd() && !lLine.contains("Spec High", Qt::CaseInsensitive))
    {
        lItems = lLine.split(",");

        // If the line contains more than the number of parameters, go to the next line
        if ((lItems.size() <= 3) || ((lItems.size()-3) > mParameterList.size()))
            continue;

        // If new wafer map, close the last one and open a new one
        if (lPreviousWafer != lItems[0])
        {
            if(lFirstWafer != true)
            {
                WriteSummaries(lStdfParser, lPartCount);
            }
            else
            {
                lFirstWafer = false;
            }

            lPreviousWafer = lItems[0];

            // set the flag in the parameters to write the header in first tests
            for (int lParamIndex=0; lParamIndex<mParameterList.size(); ++lParamIndex)
            {
                mParameterList[lParamIndex].SetStaticHeaderWritten(true);
            }

            lPassBin = lFailBin = lPartCount = 0;

            // Write WIR
            lWIRRecord.SetWAFER_ID(lItems[0]);
            lStdfParser.WriteRecord(&lWIRRecord);
        }

        // Write PIR
        lPIRRecord.SetSITE_NUM(lItems[1].toInt());
        lStdfParser.WriteRecord(&lPIRRecord);

        bool lFailDie(false);
        // Write PTRs
        for (int i=2; i<lItems.size()-1; ++i)
        {
            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            lPTRRecord.SetHEAD_NUM(1);
            // The second flag is reserved and no spec limits
            gsuchar lOptFlg = 0x02 | 0x04 | 0x08;
            ParserParameter& lTest = mParameterList[i-2];
            lPTRRecord.SetTEST_NUM(lTest.GetTestNumber());
            lPTRRecord.SetSITE_NUM(lItems[1].toInt());
            if (lTest.GetValidHighLimit())
            lPTRRecord.SetTEST_FLG(0);
            lPTRRecord.SetPARM_FLG(0);
            gsfloat64 lValue = lItems[i].toDouble();
            lPTRRecord.SetRESULT(lValue);
            if ((lTest.GetValidLowLimit() && ((lTest.GetLowLimit() - lValue) > 0.00001))
                || (lTest.GetValidHighLimit() && ((lValue - lTest.GetHighLimit()) > 0.00001)))
            {
                lFailDie = true;
                lPTRRecord.SetTEST_FLG(0x80);
            }
            lTest.IncrementExecTest();

            if (lTest.GetStaticHeaderWritten() == true)
            {
                lPTRRecord.SetTEST_TXT(lTest.GetTestName());
                if (lTest.GetValidLowLimit())
                    lPTRRecord.SetLO_LIMIT(lTest.GetLowLimit());// R*4 Low test limit value OPT_FLAGbit 4 or 6 = 1
                else
                    lOptFlg |= 0x50;
                if (lTest.GetValidHighLimit())
                    lPTRRecord.SetHI_LIMIT(lTest.GetHighLimit());// R*4 High test limit value OPT_FLAGbit 5 or 7 = 1
                else
                    lOptFlg |= 0xA0;

                lPTRRecord.SetOPT_FLAG(lOptFlg);
                lPTRRecord.SetUNITS(lTest.GetTestUnits());
                lTest.SetStaticHeaderWritten(false);
            }
            lStdfParser.WriteRecord(&lPTRRecord);
        }

        lPRRRecord.SetSITE_NUM(lItems[1].toInt());

        if (lFailDie == true)
        {
            lPRRRecord.SetSOFT_BIN(2);
            lPRRRecord.SetHARD_BIN(2);
            ++lFailBin;
        }
        else
        {
            lPRRRecord.SetSOFT_BIN(1);
            lPRRRecord.SetHARD_BIN(1);
            ++lPassBin;
        }
        ++lPartCount;
        lStdfParser.WriteRecord(&lPRRRecord);

        // Read the next line
        lLine = ReadLine(lInputPCMFile);
    }

    // Close the last Wafer
    WriteSummaries(lStdfParser, lPartCount);

    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    lMRRRecord.SetFINISH_T(mStartTime);
    lStdfParser.WriteRecord(&lMRRRecord);

    // Close the previous stdf file
    lStdfParser.Close();

    // Close file
    lFile.close();


    // Success
    return true;
}


bool VanguardPCMToStdf::WriteSummaries(GQTL_STDF::StdfParse& aStdfParser, int aPartCount)
{
    GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
    lWRRRecord.SetHEAD_NUM(1);
    lWRRRecord.SetSITE_GRP(255);
    lWRRRecord.SetFINISH_T(mStartTime);
    lWRRRecord.SetPART_CNT(aPartCount);

    aStdfParser.WriteRecord(&lWRRRecord);

    return true;
}



void VanguardPCMToStdf::SpecificReadLine(QString& aLine)
{
    aLine = aLine.trimmed().simplified();
}

}
}
