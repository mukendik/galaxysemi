#include <QScriptValueIterator>
#include <gqtl_sysutils.h>

#include "cbinning.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_pat_processing.h"
#include "gex_report.h"
#include "gex_scriptengine.h"
#include "gqtl_log.h"
#include "pat_engine.h"
#include "wafer_export.h"
#include "parserFactory.h"
#include "parserAbstract.h"
#include "pat_outliers.h"
#include <gqtl_archivefile.h>
#include "gex_version.h"
#include <QTemporaryDir>
#include "plugin_base.h"


extern CGexReport *         gexReport;
extern CReportOptions       ReportOptions;
extern GexScriptEngine *    pGexScriptEngine;
//extern CPatInfo *           lPatInfo;

namespace GS
{
namespace Gex
{

WaferExport::WaferExport(QObject* parent)
    : QObject(parent), mRotateWafer(false), mWaferOrientation(PATProcessing::WOrientationDefault),
      mAxisXDirection(PATProcessing::eDefault), mAxisYDirection(PATProcessing::eDefault)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "new WaferExport");
}

WaferExport::WaferExport(const WaferExport& lOther): QObject(lOther.parent())
{
    mRotateWafer=lOther.mRotateWafer;
    mWaferOrientation=lOther.mWaferOrientation;
    mAxisXDirection=lOther.mAxisXDirection;
    mAxisYDirection=lOther.mAxisYDirection;
    mErrorMessage=lOther.mErrorMessage;
    mOutputFilename=lOther.mOutputFilename;
    mCustomer=lOther.mCustomer;
    mSupplier=lOther.mSupplier;
    mOriginalFiles=lOther.mOriginalFiles;
    mSINFInfo=lOther.mSINFInfo;
}

WaferExport::~WaferExport()
{

}

const QString &WaferExport::GetErrorMessage() const
{
    return mErrorMessage;
}

QString WaferExport::GenerateOutputName(Output lFormat, int lGroupIdx /*= 0*/,
                                        int lFileIdx /*= 0*/)
{
    CGexGroupOfFiles *  lGroup  = NULL;
    CGexFileInGroup *   lFile   = NULL;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
        lGroup = NULL;
    else
        lGroup = gexReport->getGroupsList().at(lGroupIdx);

    if(lGroup == NULL)
        return QString();	// Should never happen!

    if(lFileIdx < 0)
    {
        // Get handle to first file in group
        lFileIdx = 0;

        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.at(lFileIdx);
        if(lFile == NULL)
            return QString();	// Should never happen!
    }
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile =	NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
            return QString();	// Should never happen!
    }

    // Build custom file name
    QString lOutputName = GetCustomWaferMapFilename();

    // Build the ouput file name depending on the output format
    switch(lFormat)
    {
        case outputTSMCink:
            lOutputName = GenerateTSMCOutputName(lOutputName, lFile);
            break;

        case outputSemiG85inkAscii:
            lOutputName = GenerateG85OutputName(lOutputName, lFile, false);
            break;

        case outputSemiG85inkXml:
            lOutputName = GenerateG85OutputName(lOutputName, lFile, true);
            break;

        case outputSemiE142ink:
            lOutputName = GenerateE142OutputName(false, lOutputName, lFile);
        break;

        case outputSemiE142inkInteger2:
            lOutputName = GenerateE142OutputName(true, lOutputName, lFile);
        break;

        case outputKLA_INF:
            lOutputName = GenerateSINFOutputName(lOutputName, lFile);
            break;

        case outputXlsHtml:
            lOutputName = GenerateHTMLOutputName(lOutputName, lFile);
            break;

        case outputPng:
            lOutputName = GeneratePNGOutputName(lOutputName, lFile);
            break;

        case outputSTIF:
            lOutputName = GenerateSTIFOutputName(lOutputName, lFile);
            break;

        case outputLaurierDieSort1D:
            lOutputName = GenerateLaurierDieSort1DOutputName(lOutputName, lFile);
            break;

        case outputTELP8:
            lOutputName = GenerateTELP8OutputName(lOutputName, lFile);
            break;
        case outputSumMicron:
            lOutputName = GenerateSumMicronOutputName(lOutputName, lFile);
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unknown wafermap output format %1").arg(lFormat).toLatin1().constData());
            break;
    }

    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(lOutputName);

    return lOutputName;
}

QString WaferExport::CreateOlympusAL2kOutputMap(GS::Gex::PATProcessing& lPP, CGexFileInGroup* lFile)
{
    if (!pGexScriptEngine)
        return "error: null ScriptEngine";

    QScriptValue lPatProcessingSV = pGexScriptEngine->newQObject(&lPP);
    if (lPatProcessingSV.isNull())
        return "error: cannot newQObject PATProcessing object";

    if (!lFile)
        return "error: file null";

    // Let's transfer some mandatory keys to the PATProcessing object
    lPatProcessingSV.setProperty("Lot", lFile->getMirDatas().szLot);
    //if (!lFile->MirData.m_lstWaferID.isEmpty())
      //  lPatProcessingSV.setProperty("Wafer", lFile->MirData.m_lstWaferID.first()); // empty, dont know why.
    lPatProcessingSV.setProperty("Wafer", lFile->getWaferMapData().szWaferID);
    lPP.Set("WaferStartTime", QVariant((qlonglong)lFile->getWaferMapData().lWaferStartTime));
    lPP.Set("WaferFinishTime", QVariant((qlonglong)lFile->getWaferMapData().lWaferEndTime));
    lPP.Set("WaferCenterX", lFile->getWaferMapData().GetCenterX());
    lPP.Set("WaferCenterY", lFile->getWaferMapData().GetCenterY());
    lPP.Set("TotalParts", lFile->getWaferMapData().GetTotalPhysicalDies());
    lPatProcessingSV.setProperty("WaferSize", lFile->getWaferMapData().GetDiameter());
    lPatProcessingSV.setProperty("ChipSizeX", lFile->getWaferMapData().GetDieWidth());  // bug : GetSizeX());
    lPatProcessingSV.setProperty("ChipSizeY", lFile->getWaferMapData().GetDieHeight()); // bug : GetSizeY());
    lPatProcessingSV.setProperty("Operator", lFile->getMirDatas().szOperator);
    lPatProcessingSV.setProperty("WaferUnits",
        lFile->getWaferMapData().bWaferUnits ); // Hervé: no Get ?
    // "OutputDistanceX/Y" : it could be in the JS DTR
    // GCORE-2133
    if (!lFile->property(CGexFileInGroup::sConcatenatedJavaScriptDTRs.toLatin1().data()).isNull())
    {
        lPatProcessingSV.setProperty("GalaxySemiJSDTR",
                        lFile->property(CGexFileInGroup::sConcatenatedJavaScriptDTRs.toLatin1().data()).toString() );
    }

    pGexScriptEngine->globalObject().setProperty("CurrentGSPATProcessing", lPatProcessingSV);
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Pat info is empty");
    }
    QFileInfo lSTDFFileInfo(lPatInfo->GetOutputDataFilename());

    QScriptValue lResSV=pGexScriptEngine->evaluate(
                "generate_olympus_al2000(CurrentGSPATProcessing, '"
                        //+GS::Gex::Engine::GetInstance().Get("TempFolder").toString().replace("\\", "/")
                        + lSTDFFileInfo.absolutePath()
                        +"')");

    if (pGexScriptEngine->hasUncaughtException())
        GSLOG(SYSLOG_SEV_WARNING, QString("Exception in generate_olympus_al2000: line %1 : %2")
              .arg(pGexScriptEngine->uncaughtExceptionLineNumber())
              .arg(pGexScriptEngine->uncaughtException().toString())
              .toLatin1().data());
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("generate_olympus_al2000 returned '%1'").arg(lResSV.toString())
          .toLatin1().data()
          );

    QString lReturn=lResSV.toString();
    if (pGexScriptEngine->hasUncaughtException())
        lReturn="error: exception line "+ QString::number(pGexScriptEngine->uncaughtExceptionLineNumber())+" :"
                +pGexScriptEngine->uncaughtException().toString();
        /*
         * GS::Gex::Message::warning("Olympus output map failed",
          "Failed to output map to Olympus AL2000 format:\n"+
          (pGexScriptEngine->hasUncaughtException()?pGexScriptEngine->uncaughtException().toString()+"\n":QString(""))
          +"Result: "+lRes.toString()
                              );
        */
    return lReturn;
}

bool WaferExport::CreateOutputMap(Output lFormat,
                                  const QString &lOutputFileName,
                                  int lGroupIdx /*= 0*/, int lFileIdx /*= 0*/)
{
    GSLOG(5, QString("CreateOutputMap format %1 into '%2'").arg(lFormat).arg(lOutputFileName).toLatin1().data() );
    bool lStatus = true;

    switch(lFormat)
    {
        case outputTSMCink:
            lStatus = CreateTSMCOutputMap(lOutputFileName, lGroupIdx, lFileIdx);
            break;

        case outputSemiG85inkAscii:
            lStatus = CreateG85OutputMap(lOutputFileName, lGroupIdx, lFileIdx, false);
            break;

        case outputSemiG85inkXml:
            lStatus = CreateG85OutputMap(lOutputFileName, lGroupIdx, lFileIdx, true);
            break;

        case outputSemiE142ink:
            lStatus = CreateE142OutputMap(false,lOutputFileName, lGroupIdx, lFileIdx);
            break;
        case outputSemiE142inkInteger2:
            lStatus = CreateE142OutputMap(true,lOutputFileName, lGroupIdx, lFileIdx);
            break;
        case outputKLA_INF:
            lStatus = CreateSINFOutputMap(lOutputFileName, lGroupIdx, lFileIdx);
            break;

        case outputXlsHtml:
            break;

        case outputPng:
            break;

        case outputSTIF:
            lStatus = CreateSTIFOutputMap(lOutputFileName, lGroupIdx, lFileIdx);
            break;

        case outputLaurierDieSort1D:
            break;

        case outputTELP8:
            lStatus = CreateTELP8OutputMap(lOutputFileName, lGroupIdx, lFileIdx);
            break;
        case outputSumMicron:
            lStatus = GenerateSumMicronOutputMap(lOutputFileName, lGroupIdx, lFileIdx);
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unknown wafermap output format %1").arg(lFormat).toLatin1().constData());
            break;
    }

    return lStatus;
}

void WaferExport::SetRotateWafer(bool lRotate)
{
    mRotateWafer = lRotate;
}

void WaferExport::SetOrientation(int lOrientation)
{
    mWaferOrientation = lOrientation;
}

void WaferExport::SetXAxisDirection(int lXDirection)
{
    mAxisXDirection = lXDirection;
}

void WaferExport::SetYAxisDirection(int lYDirection)
{
    mAxisYDirection = lYDirection;
}

void WaferExport::SetCustomer(const QString &lCustomer)
{
    mCustomer = lCustomer;
}

void WaferExport::SetSupplier(const QString &lSupplier)
{
    mSupplier = lSupplier;
}

void WaferExport::SetSINFInfo(const PAT::SINFInfo &lInfo)
{
    mSINFInfo = lInfo;
}

void WaferExport::SetOriginalFiles(const QStringList &lOriginalFiles)
{
    mOriginalFiles = lOriginalFiles;
}

bool WaferExport::CreateTSMCOutputMap(const QString& lOutputFileName, int lGroupIdx /*= 0*/,
                                      int lFileIdx /*= 0*/)
{
    int                 lWaferSizeX;
    int                 lWaferSizeY;
    CGexGroupOfFiles *  lGroup  = NULL;
    CGexFileInGroup *   lFile   = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
        lGroup = NULL;
    else
        lGroup = gexReport->getGroupsList().at(lGroupIdx);

    if(lGroup == NULL)
        return true;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool lMergedWafer = false;

    if(lFileIdx < 0)
    {
        lMergedWafer    = true;
        lWaferSizeX     = lGroup->cStackedWaferMapData.SizeX;
        lWaferSizeY     = lGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        lFileIdx = 0;

        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.at(lFileIdx);
        if(lFile == NULL)
            return true;	// Should never happen!
    }
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile =	NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
            return true;	// Should never happen!

        lWaferSizeX = lFile->getWaferMapData().SizeX;
        lWaferSizeY = lFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = lFile->getWaferMapData();

    // Check if allow user to customize output path & name.
//    if(bSaveAs)
//    {
//        strWaferFileFullName = QFileDialog::getSaveFileName(strWaferFileFullName, "TSMC inkless format (*.tsm)",
//                                                            pGexMainWindow, "save wafermap dialog","Save Wafer data as..." );

//        // If no file selected, ignore command.
//        if(strWaferFileFullName.isEmpty())
//            return NoError;

//        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
//    }

    // Create file.
     // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    QFile lOuputFile(lOutputFileName);

    if (lOuputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&lOuputFile);	// Assign file handle to data stream

    // Header section
    hWafermapFile << "TSMC" << endl;						// Header line
    hWafermapFile << lFile->getMirDatas().szPartType << endl;		// Product name
    hWafermapFile << lWafer.szWaferID << endl;              // WaferID
    hWafermapFile << lOutputFileName << endl;				// Wafermap file name created (eg: A4523403.TSM)

    // Check if PosX/PosY have to be overloaded
    if(mAxisXDirection == PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(mAxisXDirection == PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(mAxisXDirection == PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(mAxisXDirection == PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }

    if(mAxisYDirection == PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(mAxisYDirection == PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(mAxisYDirection == PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(mAxisYDirection == PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap until Notch is at 6hour (Down)...unless custom direction is set
    if(mWaferOrientation <= 0)
        mWaferOrientation = 6;	// Default notch direction is DOWN

    if(mRotateWafer)
    {
        while(lWafer.GetWaferNotch() != mWaferOrientation)
            lWafer.RotateWafer();
    }

    lWaferSizeX = lWafer.SizeX;
    lWaferSizeY = lWafer.SizeY;

    // Wafermap
    int     lBinCode;
    int     lStartCol;
    int     lEndCol;
    int     lColStep;
    int     lStartLine;
    int     lEndLine;
    int     lLineStep;

    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        lStartCol   = 0;
        lEndCol     = lWaferSizeX;
        lColStep    = 1;
    }
    else
    {
        // X direction = 'L' (left)
        lStartCol   = lWaferSizeX-1;
        lEndCol     = -1;
        lColStep    = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        lStartLine  = 0;
        lEndLine    = lWaferSizeY;
        lLineStep   = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        lStartLine  = lWaferSizeY-1;
        lEndLine    = -1;
        lLineStep   = -1;
    }

    for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
    {
        // Processing a wafer line.
        for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(lMergedWafer)
                lBinCode = lGroup->cStackedWaferMapData.cWafMap[(lColIdx+(lLineIdx*lWaferSizeX))].ldCount;
            else
                lBinCode = lWafer.getWafMap()[(lColIdx+(lLineIdx*lWaferSizeX))].getBin();

            switch(lBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:	// -1: Die not tested
                    hWafermapFile << ".";
                    break;

                default:	// Check if PASS or FAIL die
                    if(lMergedWafer)
                    {
                        // Merged wafer
                        if(lBinCode != lGroup->cStackedWaferMapData.iTotalWafermaps)
                            hWafermapFile << "1";	// All dies matching
                        else
                            hWafermapFile << "X";	// Not 100% matching...
                    }
                    else
                    {
                        // Single wafer
                        if(lGroup->isPassingBin(true, lBinCode))
                        {
                            if(lBinCode >= 0 && lBinCode <= 9)
                                hWafermapFile << QString::number(lBinCode);
                            else
                                hWafermapFile << "1";
                        }
                        else
                            hWafermapFile << "X";
                    }
                    break;
            }
        }

        // Insert line break
        hWafermapFile << endl;
    }

    // Update/Create TSMC summary file.
    lOuputFile.close();

    return true;
}

bool WaferExport::CreateE142OutputMap(bool isInteger2, const QString &lOutputFileName, int lGroupIdx, int lFileIdx)
{
    int                 lWaferSizeX;
    int                 lWaferSizeY;
    CGexFileInGroup *   lFile   = NULL;
    CGexGroupOfFiles *  lGroup  = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
        lGroup = NULL;
    else
        lGroup = gexReport->getGroupsList().at(lGroupIdx);

    if(lGroup == NULL)
        return true;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool lMergedWafer = false;
    if(lFileIdx < 0)
    {
        lMergedWafer    = true;
        lWaferSizeX     = lGroup->cStackedWaferMapData.SizeX;
        lWaferSizeY     = lGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        lFileIdx = 0;
        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.at(lFileIdx);
        if(lFile == NULL)
            return true;	// Should never happen!
    }
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile = NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
            return true;	// Should never happen!

        lWaferSizeX = lFile->getWaferMapData().SizeX;
        lWaferSizeY = lFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = lFile->getWaferMapData();

    CBinning * lBinningList = NULL;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        if (lPatInfo && lPatInfo->GetSoftBins())
            lBinningList = lPatInfo->GetSoftBins();
        else
            lBinningList = lGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        if (lPatInfo && lPatInfo->GetHardBins())
            lBinningList = lPatInfo->GetHardBins();
        else
            lBinningList = lGroup->cMergedData.ptMergedHardBinList;
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING,
              "Unable to export into E142 format, Wafer map is not filled with Soft or Hard bins values");
        return false;
    }

    QString lLotID      = lFile->getMirDatas().szLot;
    QString lWaferID    = lFile->getWaferMapData().szWaferID;
    QString lSubLotID   = lFile->getMirDatas().szSubLot;

    // Check if allow user to customize output path & name.
//    if(bSaveAs)
//    {
//        strWaferFileFullName = QFileDialog::getSaveFileName(strWaferFileFullName, "SEMI E142 XML inkless format (*.xml)",
//                                                            pGexMainWindow, "save wafermap dialog", "Save Wafer data as..." );

//        // If no file selected, ignore command.
//        if(strWaferFileFullName.isEmpty())
//            return NoError;

//        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
//    }

    // Create file.
    // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    QFile lOutputFile(lOutputFileName);
    if (lOutputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&lOutputFile);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(mAxisXDirection == PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(mAxisXDirection == PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(mAxisXDirection == PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(mAxisXDirection == PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }

    if(mAxisYDirection == PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(mAxisYDirection == PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(mAxisYDirection == PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(mAxisYDirection == PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(mWaferOrientation <= 0)
        mWaferOrientation = lWafer.GetWaferNotch();
    else if(mRotateWafer)
    {
        while(lWafer.GetWaferNotch() != mWaferOrientation)
            lWafer.RotateWafer();
    }

    lWaferSizeX = lWafer.SizeX;
    lWaferSizeY = lWafer.SizeY;

    // Define wafer orientation
    int	lOrientation = 0;
    switch(mWaferOrientation)
    {
        case 12:
            lOrientation = 180;
            break;
        case 3:
            lOrientation = 270;
            break;
        case 6:
            lOrientation = 0;
            break;
        case 9:
            lOrientation = 90;
            break;
    }

    // Find First die coordinates in wafer
    int lFirstDieX  = 0;
    int lFirstDieY  = 0;

    for(lFirstDieX = 0; lFirstDieX < lWaferSizeX; ++lFirstDieX)
    {
        // Find first valid die on X axis...
        if(lWafer.getWafMap()[lFirstDieX].getBin() != GEX_WAFMAP_EMPTY_CELL)
            break;
    }

    // Define location and direction for the coordinate system
    QString lOriginLocation;
    QString lAxisDirection;
    int     lOriginX    = lWafer.iLowDieX;
    int     lOriginY    = lWafer.iLowDieY;

    // Semi E142 Xml specification
    // AxisDirection specify the increment direction for X and Y
    // OriginLocation specify the position of the 0.0 in the Axis
    // AxisDirection and OriginLocation allow to start the WaferMap
    // with negative or positive coordinates
    if(lWafer.GetPosXDirection() == true)
    {
        if (lWafer.GetPosYDirection() == true)
        {
            lAxisDirection      = GEX_WAFMAP_EXPORT_AXIS_DOWNRIGHT;
            lOriginLocation     = GEX_WAFMAP_EXPORT_ORIGIN_UPPERLEFT;
        }
        else
        {
            lAxisDirection      = GEX_WAFMAP_EXPORT_AXIS_UPRIGHT;
            lOriginLocation     = GEX_WAFMAP_EXPORT_ORIGIN_LOWERLEFT;
        }
    }
    else
    {
        if (lWafer.GetPosYDirection() == true)
        {
            lAxisDirection	= GEX_WAFMAP_EXPORT_AXIS_DOWNLEFT;
            lOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_UPPERRIGHT;
        }
        else
        {
            lAxisDirection	= GEX_WAFMAP_EXPORT_AXIS_UPLEFT;
            lOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_LOWERRIGHT;
        }
    }

    // Check if the OriginLocation is the center
    if(((lWafer.iHighDieX + lWafer.iLowDieX) <= 1)
       && ((lWafer.iHighDieY + lWafer.iLowDieY) <= 1))
    {
        lOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_CENTER;
        lOriginX        = 0;
        lOriginY        = 0;

        if(lWafer.GetPosXDirection() == true)
            lFirstDieX--;
        if(lWafer.GetPosYDirection() == true)
            lFirstDieY--;

        lFirstDieX -= (lWafer.iHighDieX - lWafer.iLowDieX)/2;
        lFirstDieY -= (lWafer.iHighDieY - lWafer.iLowDieY)/2;
    }
    else
    {
        lFirstDieX += lOriginX;
        lFirstDieY += lOriginY;
    }

    // Header section
    hWafermapFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
    hWafermapFile << "<MapData xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                  << "xmlns=\"urn:semi-org:xsd.E142-1.V0105.SubstrateMap\" "
                  << "xmlns:sme=\"urn:semi-org:xsd.E142-1.V0105.SubstrateMap_ACMEProductsInc\" "
                  << "xsi:schemaLocation=\"urn:semi-org:xsd.E142-1.V0105.SubstrateMap_ACMEProductsInc E142-1-V0105-Schema-Extension.xsd\" "
                  << ">" << endl;

    // Layout****
    hWafermapFile << "\t<Layouts>" << endl;
    hWafermapFile << "\t\t<Layout LayoutId=\"WaferLayout\" DefaultUnits=\"mm\" TopLevel=\"true\">" << endl;
    hWafermapFile << "\t\t\t<Dimension X=\"1\" Y=\"1\"/>" << endl;
    // Wafer sizes info
    // Size in mm
    double lWaferDiameter = CWaferMap::ConvertToMM(lWafer.GetDiameter(),
                                                   lWafer.bWaferUnits);
    hWafermapFile << "\t\t\t<DeviceSize X=\"" << QString::number(lWaferDiameter)
                  << "\" Y=\"" << QString::number(lWaferDiameter) << "\"/>" << endl;
    hWafermapFile << "\t\t\t<ChildLayouts>" << endl;
    hWafermapFile << "\t\t\t\t<ChildLayout LayoutId=\"Devices\"/>" << endl;
    hWafermapFile << "\t\t\t</ChildLayouts>" << endl;
    hWafermapFile << "\t\t</Layout>" << endl;
    hWafermapFile << "\t\t<Layout LayoutId=\"Devices\" DefaultUnits=\"micron\">" << endl;
    hWafermapFile << "\t\t\t<Dimension X=\"" << lWaferSizeX << "\" Y=\"" << lWaferSizeY << "\"/>" << endl;

    // Wafer sizes info
    // Size in uM
    double lDieWidth    = 1e3 * CWaferMap::ConvertToMM(lWafer.GetDieWidth(), lWafer.bWaferUnits);
    double lDieHeight   = 1e3 * CWaferMap::ConvertToMM(lWafer.GetDieHeight(), lWafer.bWaferUnits);
    hWafermapFile << "\t\t\t<DeviceSize X=\"" << QString::number(lDieWidth)
                  << "\" Y=\"" << QString::number(lDieHeight) << "\"/>" << endl;
    hWafermapFile << "\t\t\t<StepSize X=\"" << QString::number(lDieWidth)
                  << "\" Y=\"" << QString::number(lDieHeight) << "\"/>" << endl;
    hWafermapFile << "\t\t\t<ProductId>" << lFile->getMirDatas().szPartType <<"</ProductId>" << endl;
    hWafermapFile << "\t\t</Layout>" << endl;
    hWafermapFile << "\t</Layouts>" << endl;

    // Semi E142 Substrate Extension
    hWafermapFile << "\t<Substrates>" << endl;
    hWafermapFile << "\t\t<Substrate SubstrateType=\"Wafer\" SubstrateId=\"" << lWaferID <<"\"" << endl;
    hWafermapFile << "\t\t				xsi:type=\"sme:SubstrateTypeExtension\" >" << endl;

    if(!lLotID.isEmpty())
        hWafermapFile << "\t\t\t<LotId>" << lLotID << "</LotId>" << endl;

    QDateTime lDateTime;
    lDateTime.setTimeSpec(Qt::UTC);
    hWafermapFile << "\t\t\t<AliasIds>" << endl;
    lDateTime.setTime_t(lFile->getMirDatas().lStartT);
    hWafermapFile << "\t\t\t\t<AliasId Type=\"TestStartTime\" Value=\""
                  << lDateTime.toString("yyyyMMddhhmmsszzz") <<"\" />" <<endl;
    lDateTime.setTime_t(lFile->getMirDatas().lEndT);
    hWafermapFile << "\t\t\t\t<AliasId Type=\"TestEndTime\" Value=\""
                  << lDateTime.toString("yyyyMMddhhmmsszzz") <<"\" />" <<endl;
    hWafermapFile << "\t\t\t</AliasIds>" << endl;
    hWafermapFile << "\t\t\t<CarrierType> "<<"NA"<<" </CarrierType>"<< endl;
    hWafermapFile << "\t\t\t<CarrierId> "<<"NA"<<" </CarrierId>"<< endl;
    hWafermapFile << "\t\t\t<SlotNumber> "<<"1"<<" </SlotNumber>"<< endl;
    hWafermapFile << "\t\t\t<SubstrateNumber> "<<"1"<<" </SubstrateNumber>"<< endl;

    if (lPatInfo)
        hWafermapFile << "\t\t\t<GoodDevices> "<<QString::number(lPatInfo->GetTotalGoodPartsPostPAT())
                      <<" </GoodDevices>"<< endl;
    else
        hWafermapFile << "\t\t\t<GoodDevices> "<<QString::number(lFile->getPcrDatas().lGoodCount) <<" </GoodDevices>"<< endl;

    hWafermapFile << "\t\t\t<SupplierName> "
      <<(!QString(lFile->getMirDatas().szFacilityID).isEmpty() ? QString(lFile->getMirDatas().szFacilityID) : QString(""))
      <<" </SupplierName>"<< endl;
    hWafermapFile << "\t\t\t<Status> "<<"NA"<<" </Status>"<< endl;

    hWafermapFile << "\t\t\t<sme:SupplierData>" << endl;

    hWafermapFile << "\t\t\t\t<sme:SetupFile Value=\""
      << (!QString(lFile->getMirDatas().szAuxFile).isEmpty() ? QString(lFile->getMirDatas().szAuxFile) : QString(""))
      << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:TestSystem Value=\""
      <<(!QString(lFile->getMirDatas().szNodeName).isEmpty() ? QString(lFile->getMirDatas().szNodeName) : QString(""))
      << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:TestProgram Value=\""
      <<(!QString(lFile->getMirDatas().szJobName).isEmpty() ? QString(lFile->getMirDatas().szJobName) : QString(""))
      << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:Prober Value=\""
      << (!QString(lFile->getMirDatas().szHandlerProberID).isEmpty() ? QString(lFile->getMirDatas().szHandlerProberID) : QString(""))
      << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:Operator Value=\""
      << (!QString(lFile->getMirDatas().szOperator).isEmpty() ? QString(lFile->getMirDatas().szOperator) : QString(""))
      << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:ProbeCard Value=\""
      <<(!QString(lFile->getMirDatas().szProbeCardID).isEmpty() ? QString(lFile->getMirDatas().szProbeCardID) : QString(""))
      << "\"/>" << endl;
    lDateTime.setTime_t(lFile->getMirDatas().lStartT);
    hWafermapFile << "\t\t\t\t<sme:TestStartTime Value=\"" << lDateTime.toString("yyyyMMddhhmmsszzz") << "\"/>" << endl;
    lDateTime.setTime_t(lFile->getMirDatas().lEndT);
    hWafermapFile << "\t\t\t\t<sme:TestEndTime Value=\"" << lDateTime.toString("yyyyMMddhhmmsszzz") << "\"/>" << endl;
    hWafermapFile << "\t\t\t</sme:SupplierData>" << endl;
    hWafermapFile << "\t\t</Substrate>" << endl;
    hWafermapFile << "\t</Substrates>" << endl;
    // Semi E142 Substrate Extension

    hWafermapFile << "\t<SubstrateMaps>" << endl;
    hWafermapFile << "\t\t<SubstrateMap SubstrateType=\"Wafer\" SubstrateId=\"" << lWaferID << "\" ";
    hWafermapFile << "Orientation=\"" << lOrientation << "\" OriginLocation=\"" << lOriginLocation << "\" ";
    hWafermapFile << "AxisDirection=\"" << lAxisDirection << "\" LayoutSpecifier=\"WaferLayout/Devices\">" << endl;

    //	hWafermapFile << "<Overlay MapName=\"SortGrade\" MapVersion=\"1\">" << endl;

    // CHECK WITH PHILIPPE IF THIS WILL NOT DISTURB ST PROCESS
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
        hWafermapFile << "\t\t\t<Overlay MapName=\"HARD BIN MAP\" MapVersion=\"1\">" << endl;
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
        hWafermapFile << "\t\t\t<Overlay MapName=\"SOFT BIN MAP\" MapVersion=\"1\">" << endl;

    hWafermapFile << "\t\t\t\t<ReferenceDevices>" << endl;

    hWafermapFile << "\t\t\t\t\t<ReferenceDevice Name=\"OriginLocation\">" << endl;
    hWafermapFile << "\t\t\t\t\t\t<Coordinates X=\"" << QString::number(lOriginX) << "\"";
    hWafermapFile << " Y=\""  << QString::number(lOriginY) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t\t</ReferenceDevice>" << endl;

    hWafermapFile << "\t\t\t\t\t<ReferenceDevice Name=\"FirstDevice\">" << endl;
    hWafermapFile << "\t\t\t\t\t\t<Coordinates X=\"" << lFirstDieX << "\" Y=\"" << lFirstDieY << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t\t</ReferenceDevice>" << endl;
    hWafermapFile << "\t\t\t\t</ReferenceDevices>" << endl;

    // Identify highest bin#
    int           lHighestBin   = 0;
    CBinning *    lBinCell      = lBinningList;
    while(lBinCell != NULL)
    {
        lHighestBin = qMax(lHighestBin, lBinCell->iBinValue);
        lBinCell = lBinCell->ptNextBin;
    };
    QString	lNullBin;
    char    lBinStringSize[10];
    int		lBinFieldSize=4;
    QString binType = "Integer2";

    if(!isInteger2)
    {
        binType = "HexaDecimal";
        if(lHighestBin <= 0xff)
            lBinFieldSize = 2;
        else
        {
            mErrorMessage = "Unable to export into E142 format: Highest bin (";
            mErrorMessage += QString::number(lHighestBin) + ") is greater than 0xff ";
            mErrorMessage += "and is not supported with Hexadecimal bin type.";

            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return false;
        }
    }

    sprintf(lBinStringSize,"%%0%dX", lBinFieldSize);
    lNullBin = lNullBin.fill('F', lBinFieldSize);

    hWafermapFile << "\t\t\t\t<BinCodeMap BinType=\""<<binType<<"\" NullBin=\"" << lNullBin << "\" ";
    //hWafermapFile << "MapType=\"" << GEX_WAFMAP_EXPORT_BIN_MAPTYPE_2D ;
    hWafermapFile << ">" << endl;

    hWafermapFile << "\t\t\t\t\t<BinDefinitions>" << endl;

    // Detail list of bins...
    QString lTmpString;
    QString lDie;

    lBinCell = lBinningList;
    while(lBinCell != NULL)
    {
        // Display Bin#
        hWafermapFile << "\t\t\t\t\t\t<BinDefinition BinCode= \"";
        lDie.sprintf(lBinStringSize, lBinCell->iBinValue);	// Hexa value
        hWafermapFile << lDie << "\" BinCount=\"" << lBinCell->ldTotalCount << "\" BinQuality=\"";

        // Display Bin quality
        if (lBinCell->cPassFail == 'P')
            lTmpString = GEX_WAFMAP_EXPORT_BIN_QUALITY_PASS;
        else if (lBinCell->cPassFail == 'F')
            lTmpString = GEX_WAFMAP_EXPORT_BIN_QUALITY_FAIL;
        else
            lTmpString = GEX_WAFMAP_EXPORT_BIN_QUALITY_NULL;

        hWafermapFile << lTmpString << "\" BinDescription=\"";

        // Bin description.
        if(lBinCell->strBinName.isEmpty() == FALSE)
            hWafermapFile << lBinCell->strBinName << "\"";
        else
            hWafermapFile << "\"";

        // End of Bin line info
        hWafermapFile << "/>" << endl;

        // Move to next Bin cell
        lBinCell = lBinCell->ptNextBin;
    };

    // Close Bin definitions
    hWafermapFile << "\t\t\t\t\t</BinDefinitions>" << endl;

    int     lBinCode;
    int     lStartCol;
    int     lEndCol;
    int     lColStep;
    int     lStartLine;
    int     lEndLine;
    int     lLineStep;

    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        lStartCol   = 0;
        lEndCol     = lWaferSizeX;
        lColStep    = 1;
    }
    else
    {
        // X direction = 'L' (left)
        lStartCol   = lWaferSizeX-1;
        lEndCol     = -1;
        lColStep    = -1;
    }

    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        lStartLine  = 0;
        lEndLine    = lWaferSizeY;
        lLineStep   = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        lStartLine  = lWaferSizeY-1;
        lEndLine    = -1;
        lLineStep   = -1;
    }

    for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
    {
        // Processing a wafer line.
        hWafermapFile << "\t\t\t\t\t<BinCode>";

        for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(lMergedWafer)
                lBinCode = lGroup->cStackedWaferMapData.cWafMap[(lColIdx+(lLineIdx * lWaferSizeX))].ldCount;
            else
                lBinCode = lWafer.getWafMap()[(lColIdx+(lLineIdx * lWaferSizeX))].getBin();

            if(lBinCode == GEX_WAFMAP_EMPTY_CELL)
                lDie = lNullBin;
            else
                lDie.sprintf(lBinStringSize, lBinCode);	// Hexa value

            // Write die value
            hWafermapFile << lDie;
        }
        // Insert XML end of line + line break
        hWafermapFile << "</BinCode>" << endl;
    }

    hWafermapFile << "\t\t\t\t</BinCodeMap>" << endl;
    hWafermapFile << "\t\t\t</Overlay>" << endl;
    hWafermapFile << "\t\t</SubstrateMap>" << endl;
    hWafermapFile << "\t</SubstrateMaps>" << endl;
    hWafermapFile << "</MapData>" << endl;

    lOutputFile.close();

    return true;
}

QString WaferExport::GenerateTSMCOutputName(const QString &lCustomName, CGexFileInGroup *lFile)
{
    // Build file name: <path>/<lot>-<WaferID>.tsm
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode
        lOutputName = lFile->getMirDatas().szLot;	// LotID
        lOutputName = lFile->getMirDatas().szLot;
        lOutputName += "_";
        lOutputName += lFile->getMirDatas().szSubLot;		// Sublot
        lOutputName += "_";
        lOutputName += lFile->getWaferMapData().szWaferID;	// WaferID
    }

    lOutputName += ".TSM";	// File extension.

    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(lOutputName);

    return lOutputName;
}

QString WaferExport::GenerateG85OutputName(const QString& lCustomName, CGexFileInGroup *lFile, bool lXMLFormat)
{
    // Build file name: <path>/<lot>-<WaferID>.tsm
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        lOutputName = lFile->getMirDatas().szLot;	// LotID

        // Build file extension
        QString lWaferId(lFile->getWaferMapData().szWaferID);
        QString lSubLotId(lFile->getMirDatas().szSubLot);

        if((lWaferId.isEmpty()) && (lSubLotId.isEmpty()))
            GSLOG(SYSLOG_SEV_WARNING, "undefined wafer id and sublot id");

        if(lXMLFormat)
        {
            // G85 in XML format
            lOutputName += "-";
            if(!lWaferId.isEmpty())
                lOutputName += lWaferId;
            else
                lOutputName += lSubLotId;
        }
        else
        {
            //  modify this line and every thing will be OK
            // G85 Ascii format: Wafer# encoded in extension.
            lOutputName += ".0";
            if(!lWaferId.isEmpty())
                lOutputName += lWaferId;
            else
                lOutputName += lSubLotId;
        }
    }

    // Add the File extension.
    if(lXMLFormat)
        lOutputName += ".xml";
    else
        lOutputName += ".dat";

    return lOutputName;
}

QString WaferExport::GenerateHTMLOutputName(const QString &lCustomName, CGexFileInGroup *lFile)
{
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode;
        // Product ID
        if (*lFile->getMirDatas().szPartType)
        {
            lOutputName = lFile->getMirDatas().szPartType;
            lOutputName += "_";
        }

        // LotID
        if (*lFile->getMirDatas().szLot)
        {
            lOutputName += lFile->getMirDatas().szLot;
            lOutputName += "_";
        }

        // Sublot
        if (*lFile->getMirDatas().szSubLot)
        {
            lOutputName += lFile->getMirDatas().szSubLot;
            lOutputName += "_";
        }

        // WaferID
        if (*lFile->getWaferMapData().szWaferID)
            lOutputName += lFile->getWaferMapData().szWaferID;
    }
    // File extension
    lOutputName += ".htm";

    return lOutputName;
}

QString WaferExport::GenerateLaurierDieSort1DOutputName(const QString &lCustomName, CGexFileInGroup *lFile)
{
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode
        lOutputName = lFile->getMirDatas().szLot;
        lOutputName += "_";
        lOutputName += lFile->getMirDatas().szSubLot;	// Sublot
        lOutputName += "_";
        lOutputName += lFile->getWaferMapData().szWaferID;	// WaferID
    }

    // Add the file extension
    lOutputName += ".col";

    return lOutputName;
}

QString WaferExport::GeneratePNGOutputName(const QString &lCustomName, CGexFileInGroup *lFile)
{
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode;
        // Product ID
        if (*lFile->getMirDatas().szPartType)
        {
            lOutputName = lFile->getMirDatas().szPartType;
            lOutputName += "_";
        }

        // LotID
        if (*lFile->getMirDatas().szLot)
        {
            lOutputName += lFile->getMirDatas().szLot;
            lOutputName += "_";
        }

        // Sublot
        if (*lFile->getMirDatas().szSubLot)
        {
            lOutputName += lFile->getMirDatas().szSubLot;
            lOutputName += "_";
        }

        // WaferID
        if (*lFile->getWaferMapData().szWaferID)
            lOutputName += lFile->getWaferMapData().szWaferID;
    }

    lOutputName += ".png";

    return lOutputName;
}

QString WaferExport::GenerateE142OutputName(bool isInteger2, const QString &lCustomName, CGexFileInGroup *lFile)
{
    // Build Wafer name: lotID-waferID
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode ;
        QString lLot    = lFile->getMirDatas().szLot;
        QString lWafer  = lFile->getWaferMapData().szWaferID;
        QString lSubLot = lFile->getMirDatas().szSubLot;

        if((lWafer.isEmpty()) && (lSubLot.isEmpty()))
        {
            GSLOG(SYSLOG_SEV_WARNING, "undefined wafer id and sublot id");
        }

        if((lLot.isEmpty() == false) && (lWafer.count(lLot) > 0))
        {
            // If WaferID includes LotID (or equal), only keep WaferID
            // ( if wafer id undefined, take sub lot id )
            if(!lWafer.isEmpty())
                lOutputName = lWafer;
            else
                lOutputName = lSubLot;
        }
        else
        {
            lOutputName = lLot + "-";
            if(!lWafer.isEmpty())
                lOutputName += lWafer;
            else
                lOutputName += lSubLot;
        }
    }

    if(isInteger2)
    {
        lOutputName += ".integer2";
    }
    lOutputName += ".xml";

    return lOutputName;
}

QString WaferExport::GenerateSTIFOutputName(const QString &lCustomName, CGexFileInGroup * lFile)
{
    // Build file name: <path>/<lot>.0xx with 'xx': <WaferID>
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode
        lOutputName = lFile->getMirDatas().szLot;	// LotID

        lOutputName += "-";
        lOutputName += lFile->getMirDatas().szSubLot;
        lOutputName += "-";
        lOutputName += lFile->getMirDatas().szperFrq;
    }

    // Add file extension
    lOutputName += ".STIF";

    return lOutputName;
}

QString WaferExport::GenerateSINFOutputName(const QString &lCustomName, CGexFileInGroup *lFile)
{
    // Build file name: <path>/<lot>-<WaferID>
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode
        if(mSINFInfo.mWaferID.isEmpty())
        {
            QString lLotId(lFile->getMirDatas().szLot);
            QString lWaferId(lFile->getWaferMapData().szWaferID);
            QString lSubLotId(lFile->getMirDatas().szSubLot);

            if((lWaferId.isEmpty()) && (lSubLotId.isEmpty()))
                GSLOG(SYSLOG_SEV_WARNING, "Undefined wafer id and sublot id");

            lOutputName = lLotId + ".";
            if(lWaferId.isEmpty() == false)
                lOutputName += lWaferId;		// <LotID>.<waferID>
            else
                lOutputName += lSubLotId;       // <LotId>.<SubLotId>
        }
        else
        {
            bool	lFlag;

            // A KLA/INF optional source was defined, then use it to build the file name.
            mSINFInfo.mWaferID.toLong(&lFlag);
            if(lFlag)
                // <LotID>.<waferID> : No OCR enabled because Wafer string only holds a number!
                lOutputName = mSINFInfo.mLot + "." + mSINFInfo.mWaferID;
            else
                lOutputName = mSINFInfo.mWaferID;	// <OCR_WaferID>
        }
    }

    // Build file extension
    lOutputName += ".sinf";

    return lOutputName;
}

QString WaferExport::GenerateTELP8OutputName(const QString &lCustomName, CGexFileInGroup *lFile)
{
    // Build file name: <path>/<lot>-xx.dat with 'xx': <WaferID>
    QString lOutputName = lCustomName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode
        // LotID
        lOutputName = lFile->getMirDatas().szLot;

        QString lWaferId(lFile->getWaferMapData().szWaferID);
        QString lSubLotId(lFile->getMirDatas().szSubLot);

        if((lWaferId.isEmpty()) && (lSubLotId.isEmpty()))
        {
            GSLOG(SYSLOG_SEV_WARNING, "Undefined wafer id and sublot id");
        }

        lOutputName += "-";

        if(lWaferId.isEmpty() == false)
            lOutputName += lWaferId;
        else
            lOutputName += lSubLotId;
    }

    // Add file extension
    lOutputName += ".dat";

    return lOutputName;
}

QString WaferExport::GenerateSumMicronOutputName(const QString &customName, CGexFileInGroup * file)
{
    // Build file name: <path>/<lot>.<waferID>.FQQP.sum
    // Example: 8698926.006.8926-22.FQQP.sum
    QString lOutputName = customName;

    if(lOutputName.isEmpty())
    {
        //Legacy mode
        lOutputName = file->getMirDatas().szLot;	// LotID
        lOutputName += ".";
        lOutputName += file->getWaferMapData().szWaferID;
        lOutputName += ".FQQP";
    }

    // Add file extension
    lOutputName += ".sum";

    return lOutputName;
}


QString WaferExport::GetCustomWaferMapFilename() const
{
    QString lFileNameJSExpr = ReportOptions.GetOption("wafer", "wafer_file_name").toString();
    QString lFileName = "";

    if(lFileNameJSExpr.isEmpty() || !pGexScriptEngine)
        return QString();

    QScriptEngine           lScriptEngine;
    QScriptValueIterator    it(pGexScriptEngine->globalObject());
    while (it.hasNext())
    {
        it.next();

        lScriptEngine.setProperty(it.name().toLatin1().constData(),
                                  it.value().toVariant());
    }

    if(!InitScriptEngineProperties(lScriptEngine))
        return QString();

    if (lScriptEngine.canEvaluate(lFileNameJSExpr))
        lFileName = lScriptEngine.evaluate(lFileNameJSExpr).toString();

    if (lScriptEngine.hasUncaughtException())
    {
        QString lErrorMsg = "Error evaluating: " + lFileNameJSExpr;
        GSLOG(SYSLOG_SEV_ERROR, lErrorMsg.toLatin1().constData());

        return QString();
    }

    // Remove windows forbiden char
    lFileName.remove(QRegExp("[\\/:*?<>|]"));
    // Begins with space,-,dot
    lFileName.remove(QRegExp("^[ -.]+"));
    // Ends with space,-,dot
    lFileName.remove(QRegExp("[ -.]+$"));

    return lFileName;
}

QString WaferExport::GetBCDTime(long lTimeStamp) const
{
    QString     lBCDDateTime;
    QDateTime   lDateTime;
    lDateTime.setTime_t(lTimeStamp);
    QDate       lDate = lDateTime.date();
    QTime       lTime = lDateTime.time();

    lBCDDateTime  = lDate.year() / 10;      // YY 1st digit
    lBCDDateTime += lDate.year() % 10;      // YY 2nd digit
    lBCDDateTime  = lDate.month() / 10;     // MM 1st digit
    lBCDDateTime += lDate.month() % 10;     // MM 2nd digit
    lBCDDateTime  = lDate.day() / 10;		// DD 1st digit
    lBCDDateTime += lDate.day() % 10;		// DD 2nd digit

    lBCDDateTime  = lTime.hour() / 10;      // HH 1st digit
    lBCDDateTime += lTime.hour() % 10;      // HH 2nd digit
    lBCDDateTime  = lTime.minute() / 10;	// MM 1st digit
    lBCDDateTime += lTime.minute() % 10;	// MM 2nd digit
    lBCDDateTime  = lTime.second() / 10;	// SS 1st digit
    lBCDDateTime += lTime.second() % 10;	// SS 2nd digit

    return lBCDDateTime;
}

bool WaferExport::InitScriptEngineProperties(QScriptEngine &lScriptEngine) const
{
    CGexGroupOfFiles *  lGroup  = NULL;
    CGexFileInGroup *   lFile   = NULL;
    QString             lProductId;
    QString             lLotId;
    QString             lWaferId;
    QString             lSubLotId;
    QString             lProgName;
    QString             lTesterName;
    QString             lTesterType;
    QString             lFacilityId;
    QString             lPackageType;
    QString             lFamilyId;

    foreach(lGroup, gexReport->getGroupsList())
    {
        if(lGroup)
        {
            foreach(lFile, lGroup->pFilesList)
            {
                if (lFile)
                {
                    // Set product ID
                    if (!lProductId.isEmpty() && lProductId != QString(lFile->getMirDatas().szPartType))
                        lProductId = "MultiProduct";
                    else
                        lProductId = QString(lFile->getMirDatas().szPartType);

                    // Set lot ID
                    if (!lLotId.isEmpty() && lLotId != QString(lFile->getMirDatas().szLot))
                        lLotId= "MultiLot";
                    else
                        lLotId = QString(lFile->getMirDatas().szLot);

                    // Set wafer ID
                    if (!lWaferId.isEmpty() && lWaferId != QString(lFile->getWaferMapData().szWaferID))
                        lWaferId = "MultiWafer";
                    else
                        lWaferId = lFile->getWaferMapData().szWaferID;

                    // Set sublot ID
                    if (!lSubLotId.isEmpty() && lSubLotId != QString(lFile->getMirDatas().szSubLot))
                        lSubLotId = "MultiSubLot";
                    else
                        lSubLotId = QString(lFile->getMirDatas().szSubLot);

                    // Set programme name
                    if (!lProgName.isEmpty() && lProgName != QString(lFile->getMirDatas().szJobName))
                        lProgName = "MultiProgramme";
                    else
                        lProgName = QString(lFile->getMirDatas().szJobName);

                    // Set tester name
                    if (!lTesterName.isEmpty() && lTesterName != QString(lFile->getMirDatas().szNodeName))
                        lTesterName = "MultiTesterName";
                    else
                        lTesterName = QString(lFile->getMirDatas().szNodeName);

                    // Set tester type
                    if (!lTesterType.isEmpty() && lTesterType != QString(lFile->getMirDatas().szTesterType))
                        lTesterType = "MultiTesterType";
                    else
                        lTesterType = QString(lFile->getMirDatas().szTesterType);

                    // Set facility ID
                    if (!lFacilityId.isEmpty() && lFacilityId != QString(lFile->getMirDatas().szFacilityID))
                        lFacilityId = "MultiFacility";
                    else
                        lFacilityId = QString(lFile->getMirDatas().szFacilityID);

                    // Set package type
                    if (!lPackageType.isEmpty() && lPackageType != QString(lFile->getMirDatas().szPkgType))
                        lPackageType = "MultiPackage";
                    else
                        lPackageType = QString(lFile->getMirDatas().szPkgType);

                    // Set family ID
                    if (!lFamilyId.isEmpty() && lFamilyId != QString(lFile->getMirDatas().szFamilyID))
                        lFamilyId = "MultiFamiliy";
                    else
                        lFamilyId = QString(lFile->getMirDatas().szFamilyID);
                }
            }
        }
    }

    // Set product ID
    lScriptEngine.globalObject().setProperty("$PRODUCT_ID", lProductId);
    // Set lot ID
    lScriptEngine.globalObject().setProperty("$LOT_ID", lLotId);
    // Set wafer ID
    lScriptEngine.globalObject().setProperty("$WAFER_ID", lWaferId);
    // Set sublot ID
    lScriptEngine.globalObject().setProperty("$SUBLOT_ID", lSubLotId);
    // Set programme name
    lScriptEngine.globalObject().setProperty("$PROGRAM_NAME", lProgName);
    // Set tester name
    lScriptEngine.globalObject().setProperty("$TESTER_NAME", lTesterName);
    // Set tester type
    lScriptEngine.globalObject().setProperty("$TESTER_TYPE", lTesterType);
    // Set facility ID
    lScriptEngine.globalObject().setProperty("$FACILITY_ID", lFacilityId);
    // Set package type
    lScriptEngine.globalObject().setProperty("$PACKAGE_TYPE", lPackageType);
    // Set family ID
    lScriptEngine.globalObject().setProperty("$FAMILY_ID", lFamilyId);

    return true;
}

bool WaferExport::CreateG85OutputMap(const QString &lOutputFileName, int lGroupIdx, int lFileIdx,
                                     bool lXMLFormat)
{
    int                 lWaferSizeX;
    int                 lWaferSizeY;
    CGexFileInGroup *   lFile=0;
    CGexGroupOfFiles *  lGroup=0;
    CWaferMap           lWafer;
    CBinning *          lBinCell = NULL;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
        lGroup = NULL;
    else
        lGroup = gexReport->getGroupsList().at(lGroupIdx);

    if(lGroup == NULL)
        return true;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool lMergedWafer = false;

    if(lFileIdx < 0)
    {
        lMergedWafer    = true;
        lWaferSizeX     = lGroup->cStackedWaferMapData.SizeX;
        lWaferSizeY     = lGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        lFileIdx = 0;

        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.at(lFileIdx);
        if(lFile == NULL)
            return true;	// Should never happen!
    }
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile = NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
            return true;	// Should never happen!

        lWaferSizeX = lFile->getWaferMapData().SizeX;
        lWaferSizeY = lFile->getWaferMapData().SizeY;
    }

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    // Create a local copy of the wafermap
    lWafer = lFile->getWaferMapData();

    // Check if allow user to customize output path & name.
//    if(bSaveAs)
//    {
//        if(bXmlFormat)
//        {
//            // G85 XML format
//            strWaferFileFullName = QFileDialog::getSaveFileName(strWaferFileFullName, "G85 XML Semi85 inkless format (*.xml)",
//                                                                pGexMainWindow, "save wafermap dialog", "Save Wafer data as..." );
//        }
//        else
//        {
//            // G85 Ascii format
//            strWaferFileFullName = QFileDialog::getSaveFileName(strWaferFileFullName, "G85 Semi85 inkless format (*.dat)",
//                                                                pGexMainWindow, "save wafermap dialog", "Save Wafer data as..." );
//        }

//        // If no file selected, ignore command.
//        if(strWaferFileFullName.isEmpty())
//            return NoError;

//        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
//    }

    // Create file.
    // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    QFile lOutputfile(lOutputFileName);
    if (lOutputfile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&lOutputfile);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(mAxisXDirection == PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(mAxisXDirection == PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(mAxisXDirection == PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(mAxisXDirection == PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }

    if(mAxisYDirection == PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(mAxisYDirection == PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(mAxisYDirection == PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(mAxisYDirection == PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(mWaferOrientation <= 0)
        mWaferOrientation = lWafer.GetWaferNotch();
    else if(mRotateWafer)
    {
        while(lWafer.GetWaferNotch() != mWaferOrientation)
            lWafer.RotateWafer();
    }

    lWaferSizeX = lWafer.SizeX;
    lWaferSizeY = lWafer.SizeY;

    // Compute notch direction accorinng to G85 specs.
    int	lNotchDirection = 0;	// 0=down,90=left,180=top,270=right
    switch(mWaferOrientation)
    {
        case 12:	// Up
            lNotchDirection = 180;
            break;
        case 6:		// Down
            lNotchDirection = 0;
            break;
        case 3:		// Right
            lNotchDirection = 270;
            break;
        case 9:		// LEFT
            lNotchDirection = 90;
            break;
    }

    // Find First die coordinates in wafer
    int lFirstDieX = 0;
    int lFirstDieY = 0;

    for(lFirstDieX = 0; lFirstDieX < lWaferSizeX; ++lFirstDieX)
    {
        // Find first valid die on X axis...
        if(lWafer.getWafMap()[lFirstDieX].getBin() != GEX_WAFMAP_EMPTY_CELL)
            break;
    }

    // The WaferMap Coordinate start at 0.0
    // Define location and direction for the coordinate system
    //	OriginPosition	- Direction
    // 0 : Center		- DownRight
    // 1 : UpperRight	- DownLeft
    // 2 : UpperLeft	- DownRight
    // 3 : DownLeft		- UpRight
    // 4 : DownRight	- UpLeft
    int lOriginLocation = 0;

    if(lWafer.GetPosXDirection() == true)
    {
        if (lWafer.GetPosYDirection() == true)
            lOriginLocation	= 2;
        else
            // DEFAULT Semi G85 Inkless (Ascci)
            lOriginLocation	= 3;
    }
    else
    {
        if (lWafer.GetPosYDirection() == true)
            lOriginLocation	= 1;
        else
            lOriginLocation	= 4;
    }

    // Check if the OriginLocation is the center
    if(((lWafer.iHighDieX + lWafer.iLowDieX) <= 1)
       && ((lWafer.iHighDieY + lWafer.iLowDieY) <= 1))
    {
        lOriginLocation = 0;
        if(lWafer.GetPosXDirection() == true)
            lFirstDieX--;
        if(lWafer.GetPosYDirection() == true)
            lFirstDieY--;

        lFirstDieX -= (lWafer.iHighDieX - lWafer.iLowDieX)/2;
        lFirstDieY -= (lWafer.iHighDieY - lWafer.iLowDieY)/2;
    }


    // Build Wafer name: lotID-waferID...but if WaferID already has a '-' sign, then assume it already holds the ful lstring we want.
    QString lWaferID = lWafer.szWaferID;
    if(lWaferID.count("-") <= 0)
        lWaferID = QString(lFile->getMirDatas().szLot) + QString("-") + QString(lWafer.szWaferID);

    // Header section
    double	lValue;
    if(lXMLFormat)
    {
        // XML output format
        hWafermapFile << "<?xml version=\"1.0\"?>" << endl;
        hWafermapFile << "<Maps>" << endl;
        hWafermapFile << "  <Map xmlns:semi=\"http://www.semi.org\" WaferId=\""
                      << lWafer.szWaferID << "\" FormatRevision=\"G85-1101\">" << endl;
        //eg: <Device ProductId="00P41M9309" LotId="6D5KT00B31" CreateDate="2007091020514700" SupplierName="IBM-BTV" Rows="44" Columns="43" Orientation="0" OriginLocation="3" BinType="Decimal" NullBin="255" >
        hWafermapFile << "    <Device ProductId=\"" << lFile->getMirDatas().szPartType << "\"";
        hWafermapFile << " LotId=\"" << lFile->getMirDatas().szLot << "\"";
        QDateTime lDateTime;
        if(lFile->getMirDatas().lSetupT != 0)
            lDateTime.setTime_t(lFile->getMirDatas().lSetupT);
        else
            lDateTime.setTime_t(lFile->getMirDatas().lStartT);

        hWafermapFile << " CreateDate=\"" << lDateTime.toString("yyyyMMddhhmmsszzz") << "\"";
        hWafermapFile << " SupplierName=\"" << mSupplier << "\"";
        hWafermapFile << " Rows=\"" << lWaferSizeY << "\"";
        hWafermapFile << " Columns=\"" << lWaferSizeX << "\"";
        hWafermapFile << " Orientation=\"" << lNotchDirection << "\"";
        // Original Location
        hWafermapFile << " OriginLocation=\"" << lOriginLocation << "\"";
        // Wafer sizes info
        lValue = CWaferMap::ConvertToMM(lWafer.GetDiameter(), lWafer.bWaferUnits);
        if(lValue > 0)
            hWafermapFile << " WaferSize=\"" << lValue << "\"" << endl;	// Size in mm

        lValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(), lWafer.bWaferUnits);
        if(lValue > 0)
            hWafermapFile << " DeviceSizeX=\"" << lValue*1e3 << "\"" << endl;		// Size in uM

        lValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(), lWafer.bWaferUnits);
        if(lValue > 0)
            hWafermapFile << " DeviceSizeY=\"" << lValue*1e3  << "\"" << endl;		// Size in uM

        hWafermapFile << " BinType=\"" << "Decimal" << "\"";
        hWafermapFile << " NullBin=\"255\" >";
        hWafermapFile << endl;

        // First device tested
        hWafermapFile << "<ReferenceDevice";
        hWafermapFile << " ReferenceDeviceX=\"" << lFirstDieX << "\"";
        hWafermapFile << " ReferenceDeviceY=\"" << lFirstDieY << "\" />";
        hWafermapFile << endl;


    }
    else
    {
        // Ascii output format
        hWafermapFile << "WAFER_MAP = {" << endl;
        hWafermapFile << "WAFER_ID = \"" << lWaferID << "\"" << endl;
        hWafermapFile << "MAP_TYPE = \"ASCII\"" << endl;
        hWafermapFile << "NULL_BIN = \".\"" << endl;
        hWafermapFile << "ROWS = " << lWaferSizeY << endl;
        hWafermapFile << "COLUMNS = "<< lWaferSizeX << endl;
        hWafermapFile << "FLAT_NOTCH = " << lNotchDirection << endl;

        if(mCustomer.isEmpty())
            mCustomer = "?";
        hWafermapFile << "CUSTOMER_NAME = \"" << mCustomer << "\"" << endl;
        hWafermapFile << "FORMAT_REVISION = \"ADI0811D\"" << endl;

        if(mSupplier.isEmpty())
            mSupplier = "Galaxy - www.mentor.com";
        hWafermapFile << "SUPPLIER_NAME = \"" << mSupplier << "\""  << endl;
        hWafermapFile << "LOT_ID = \"" << lFile->getMirDatas().szLot << "\"" << endl;

        // Wafer sizes info
        lValue = CWaferMap::ConvertToMM(lWafer.GetDiameter(), lWafer.bWaferUnits);
        if(lValue > 0)
            hWafermapFile << "WAFER_SIZE = " << lValue << endl;	// Size in mm

        lValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(), lWafer.bWaferUnits);
        if(lValue > 0)
            hWafermapFile << "X_SIZE = " << lValue * 1e3 << endl;		// Size in uM

        lValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(), lWafer.bWaferUnits);
        if(lValue > 0)
            hWafermapFile << "Y_SIZE = " << lValue * 1e3 << endl;		// Size in uM

        // Total dies on wafer
        int lTotalDies = 0;
        if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
        {
            if (lPatInfo && lPatInfo->GetSoftBins())
                lBinCell = lPatInfo->GetSoftBins();
            else
                lBinCell = lGroup->cMergedData.ptMergedSoftBinList;
        }
        else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
        {
            if (lPatInfo && lPatInfo->GetHardBins())
                lBinCell = lPatInfo->GetHardBins();
            else
                lBinCell = lGroup->cMergedData.ptMergedHardBinList;
        }

        while(lBinCell != NULL)
        {
            lTotalDies += lBinCell->ldTotalCount;

            // Move to next Bin cell
            lBinCell = lBinCell->ptNextBin;
        };

        hWafermapFile << "DIES = " << lTotalDies << endl;
    }

    // Total bin classes
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        if (lPatInfo && lPatInfo->GetSoftBins())
            lBinCell = lPatInfo->GetSoftBins();
        else
            lBinCell = lGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        if (lPatInfo && lPatInfo->GetHardBins())
            lBinCell = lPatInfo->GetHardBins();
        else
            lBinCell = lGroup->cMergedData.ptMergedHardBinList;
    }

    int	lTotalClasses   = 0;
    int lFailCount      = 0;
    int lPassCount      = 0;

    if (lMergedWafer)
    {
        lTotalClasses = 2;

        // Merge this wafermap to the stacked array.
        for(int lIdx = 0; lIdx < lGroup->cStackedWaferMapData.SizeX * lGroup->cStackedWaferMapData.SizeY; ++lIdx)
        {
            // Get die value (Bin# or Parametric % value)
            if (lGroup->cStackedWaferMapData.cWafMap[lIdx].lStatus == GEX_WAFMAP_PASS_CELL)
                lPassCount++;
            else if (lGroup->cStackedWaferMapData.cWafMap[lIdx].lStatus == GEX_WAFMAP_FAIL_CELL)
                lFailCount++;
        }
    }
    else
    {
        while(lBinCell != NULL)
        {
            lTotalClasses++;

            // Move to next Bin cell
            lBinCell = lBinCell->ptNextBin;
        };
    }

    if(!lXMLFormat)
    {
        // Ascii output format
        hWafermapFile << "BINS = " << lTotalClasses << endl;
    }

    // Detail list of bins...
    char	lDie = ' ';
    QString lTmpString;

    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        if (lPatInfo && lPatInfo->GetSoftBins())
            lBinCell = lPatInfo->GetSoftBins();
        else
            lBinCell = lGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        if (lPatInfo && lPatInfo->GetHardBins())
            lBinCell = lPatInfo->GetHardBins();
        else
            lBinCell = lGroup->cMergedData.ptMergedHardBinList;
    }

    if (lMergedWafer)
    {
        if(lXMLFormat)
        {
            // XML output format
            // eg: <Bin BinCode="1" BinCount="48" BinQuality="Pass" BinDescription="Pickable Site"/>
            hWafermapFile << "      <Bin BinCode=\"0\" BinQuality=\"Fail\" BinCount=\""
                          << lFailCount << "\" BinDescription=\"\"" << endl;
            hWafermapFile << "      <Bin BinCode=\"1\" BinQuality=\"Pass\" BinCount=\""
                          << lPassCount << "\" BinDescription=\"\"" << endl;
        }
        else
        {
            // ASCII output format
            hWafermapFile << "BIN = \"0\" " << lTmpString.sprintf("%-6d", lFailCount) << "\"Fail\" \"\" " << endl;
            hWafermapFile << "BIN = \"1\" " << lTmpString.sprintf("%-6d", lPassCount) << "\"Pass\" \"\" " << endl;
        }
    }
    else
    {
        while(lBinCell != NULL)
        {
            if(lXMLFormat)
            {
                // XML output format
                // eg: <Bin BinCode="1" BinCount="48" BinQuality="Pass" BinDescription="Pickable Site"/>
                hWafermapFile << "      <Bin BinCode=\"";
                if(lBinCell->iBinValue > 254)
                    hWafermapFile << "254";
                else
                    hWafermapFile << lBinCell->iBinValue;
                hWafermapFile << "\"";

                // Bin type: Pass/Fail
                hWafermapFile << " BinQuality=\"";
                if(lBinCell->cPassFail == 'P')
                    hWafermapFile << "Pass\"";
                else
                    if(lBinCell->cPassFail == 'F')
                        hWafermapFile << "Fail\"";
                    else
                        hWafermapFile << "\"";

                // Total parts in bin.
                hWafermapFile << " BinCount=\"" << lBinCell->ldTotalCount << "\"";

                // Bin name.
                hWafermapFile << " BinDescription=\"" << lBinCell->strBinName << "\"/>" << endl;
            }
            else
            {
                // Display Bin#
                hWafermapFile << "BIN = \"";

                // case 7332
                if (mCustomer.compare("ADI", Qt::CaseInsensitive) == 0)
                {
                    if(lBinCell->iBinValue <= 9)
                        lDie = '0' + lBinCell->iBinValue;
                    else if(lBinCell->iBinValue <= 32)
                        lDie = 'A' + lBinCell->iBinValue - 10;
                    else
                    {
                        // For Bin# > 32, display die as 'X'
                        lDie = 'X';
                    }
                }
                else
                {
                    /*
                    // case 6132
                    // Incuding extended ASCII table
                    if(lBinCell->iBinValue <= 9)
                        lDie = '0' + lBinCell->iBinValue;
                    else if(lBinCell->iBinValue <= 189)
                        lDie = 'A' + lBinCell->iBinValue - 10;
                    else
                    {
                        // For Bin# > 189, display die as '#'
                        lDie = '#';
                    }
                    */
                    // PAT-63
                    if(lBinCell->iBinValue <= 9)
                      lDie = '0' + lBinCell->iBinValue;
                    else if(lBinCell->iBinValue <= 32)
                        lDie = 'A' + lBinCell->iBinValue - 10;
                    else
                      {
                        // For Bin# > 32, display die as 'W' if good bin, or 'X' if failing bin : WT : ?
                        lDie = 'X';
                      }
                }

                hWafermapFile << lDie << "\" ";

                // Display Bin count
                lTmpString = lTmpString.sprintf("%-6d", lBinCell->ldTotalCount);
                hWafermapFile << lTmpString;

                // Pass/Fail info (if no P/F flag, then check if we can find the Bin# from the recipe 'Good bins' list...
                if(lBinCell->cPassFail == 'P' ||
                   (lPatInfo && (lPatInfo->GetRecipeOptions().pGoodSoftBinsList->Contains(lBinCell->iBinValue))))
                    hWafermapFile << "\"Pass\" ";
                else
                    hWafermapFile << "\"Fail\" ";

                // Bin description.
                if(lBinCell->strBinName.isEmpty() == FALSE)
                    hWafermapFile << "\"" << lBinCell->strBinName << "\"";
                else
                    hWafermapFile << "\"\"";

                // End of Bin line info
                hWafermapFile << endl;
            }

            // Move to next Bin cell
            lBinCell = lBinCell->ptNextBin;
        };
    }

    // Wafermap
    if(lXMLFormat)
        hWafermapFile << "      <Data MapName=\"Map\" MapVersion=\"6\">" << endl;
    else
        hWafermapFile << "MAP = {" << endl;

    int     lBinCode;
    int     lStartCol;
    int     lEndCol;
    int     lColStep;
    int     lStartLine;
    int     lEndLine;
    int     lLineStep;

    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        lStartCol   = 0;
        lEndCol     = lWaferSizeX;
        lColStep    = 1;
    }
    else
    {
        // X direction = 'L' (left)
        lStartCol   = lWaferSizeX-1;
        lEndCol     = -1;
        lColStep    = -1;
    }

    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        lStartLine  = 0;
        lEndLine    = lWaferSizeY;
        lLineStep   = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        lStartLine  = lWaferSizeY-1;
        lEndLine    = -1;
        lLineStep   = -1;
    }

    for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
    {
        // If XML, write line prefix: <Row><![CDATA[
        if(lXMLFormat)
            hWafermapFile << "        <Row><![CDATA[";

        // Processing a wafer line.
        for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(lMergedWafer)
            {
                lBinCode = lGroup->cStackedWaferMapData.cWafMap[(lColIdx+(lLineIdx * lWaferSizeX))].ldCount;

                if(lBinCode == GEX_WAFMAP_EMPTY_CELL)
                    lDie = '.';
                else
                {
                    if(lXMLFormat)
                    {
                        lTmpString.sprintf("%03d", lBinCode);
                        hWafermapFile << lTmpString << " ";			//XML output
                    }
                    else
                        hWafermapFile << QString::number(lBinCode);	//Ascii output
                }
            }
            else
            {
                lBinCode = lWafer.getWafMap()[(lColIdx+(lLineIdx * lWaferSizeX))].getBin();

                // case 7332
                if (mCustomer.compare("ADI", Qt::CaseInsensitive) == 0)
                {
                    if(lBinCode == GEX_WAFMAP_EMPTY_CELL)
                        lDie = '.';
                    else if(lBinCode <= 9)
                        lDie = '0' + lBinCode;
                    else if(lBinCode <= 32)
                        lDie = 'A' + lBinCode - 10;
                    else
                    {
                        // For Bin# > 32, display die as 'X'
                        lDie = 'X';
                    }
                }
                else
                {
                    /*
                    // case 6132
                    // Incuding extended ASCII table
                    if(lBinCode == GEX_WAFMAP_EMPTY_CELL)
                        lDie = '.';
                    else if(lBinCode <= 9)
                        lDie = '0' + lBinCode;
                    else if(lBinCode <= 189)
                        lDie = 'A' + lBinCode - 10;
                    else
                    {
                        // For Bin# > 189, display die as '#'
                        lDie = '#';
                    }
                    */
                    // PAT-63
                    if(lBinCode == GEX_WAFMAP_EMPTY_CELL)
                        lDie = '.';
                    else if(lBinCode <= 9)
                        lDie = '0' + lBinCode;
                    else if(lBinCode <= 32)
                          lDie = 'A' + lBinCode - 10;
                    else
                        {
                            // WT: Strange comment probably from Sandrine:
                            // For Bin# > 32, display die as 'W' if good bin, or 'X' if failing bin
                            lDie = 'X';
                        }
                }
            }

            // Write die value
            if(lXMLFormat)
            {
                // XML output format
                if(lBinCode < 0)
                    lTmpString = "255";	// NulBin;
                else if(lBinCode > 254)
                    lTmpString = "254";	// Maximum bin# allowed in this format is 254
                else
                    lTmpString.sprintf("%03d", lBinCode);

                hWafermapFile << lTmpString << " ";			//XML output
            }
            else
                hWafermapFile << lDie;	// Ascii output
        }

        // Insert line break
        if(lXMLFormat)
            hWafermapFile << "]]></Row>";

        hWafermapFile << endl;
    }

    // End of map.
    if(lXMLFormat)
    {
        hWafermapFile << "      </Data>" << endl;
        hWafermapFile << "    </Device>" << endl;
        hWafermapFile << "  </Map>" << endl;
        hWafermapFile << "</Maps>" << endl;
    }
    else
    {
        hWafermapFile << "}" << endl;
        hWafermapFile << "}" << endl;
    }

    lOutputfile.close();

    return true;
}

bool WaferExport::CreateSINFOutputMap(const QString &lOutputFileName, int lGroupIdx, int lFileIdx)
{
    int                 lWaferSizeX;
    int                 lWaferSizeY;
    CGexGroupOfFiles *  lGroup  = NULL;
    CGexFileInGroup *   lFile   = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
        lGroup = NULL;
    else
        lGroup = gexReport->getGroupsList().at(lGroupIdx);

    if(lGroup == NULL)
        return true;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool lMergedWafer = false;

    if(lFileIdx < 0)
    {
        lMergedWafer    = true;
        lWaferSizeX     = lGroup->cStackedWaferMapData.SizeX;
        lWaferSizeY     = lGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        lFileIdx = 0;

        lFile = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.at(lFileIdx);
        if(lFile == NULL)
            return true;	// Should never happen!
    }
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile =	NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
            return true;	// Should never happen!

        lWaferSizeX = lFile->getWaferMapData().SizeX;
        lWaferSizeY = lFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = lFile->getWaferMapData();

//    if(bSaveAs)
//    {
//        strWaferFileFullName =
//                QFileDialog::getSaveFileName(pGexMainWindow,
//                                             "Save Wafer data as...",
//                                             strWaferFileFullName,
//                                             "SINF format (*.sinf *.inf)");

//        // If no file selected, ignore command.
//        if(strWaferFileFullName.isEmpty())
//            return NoError;

//        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
//    }

    // Create file.
    // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    QFile lOutputFile(lOutputFileName);

    if (lOutputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&lOutputFile);	// Assign file handle to data stream

    // FORCE POSX TO RIGHT AND POSY TO DOWN
    // THE SINF FORMAT DOESN'T SUPPORT ANY OTHER COORDINATE SYSTEM
    lWafer.SetPosXDirection(true);
    lWafer.SetPosYDirection(true);

    // Rotate WaferMap, and set notch location
    int	lNotchDirection = 0;	// 0=down,90=left,180=top,270=right

    // Check if notch location read from KLA INF file selected as optional source
    if(mSINFInfo.mFlatOrientation >= 0)
    {
        // KLA INF file selected as optional source, so do not make custom rotation
        lNotchDirection = mSINFInfo.mFlatOrientation;
    }
    else
    {
        // Rotate wafermap, unless default should be used for the notch
        if(mWaferOrientation <= 0)
            mWaferOrientation = lWafer.GetWaferNotch();
        else if(mRotateWafer)
        {
            while(lWafer.GetWaferNotch() != mWaferOrientation)
                lWafer.RotateWafer();
        }

        lWaferSizeX = lWafer.SizeX;
        lWaferSizeY = lWafer.SizeY;

        // Compute notch direction
        switch(mWaferOrientation)
        {
            case 12:	// Up
                lNotchDirection = 180;
                break;
            case 6:	// Down
                lNotchDirection = 0;
                break;
            case 3:	// Right
                lNotchDirection = 270;
                break;
            case 9:	// LEFT
                lNotchDirection = 90;
                break;
        }
    }

    // Device
    hWafermapFile << "DEVICE:";
    if(mSINFInfo.mDeviceName.isEmpty())
        hWafermapFile << lFile->getMirDatas().szPartType << endl;
    else
        hWafermapFile << mSINFInfo.mDeviceName << endl;

    // Lot
    hWafermapFile << "LOT:";
    if(mSINFInfo.mLot.isEmpty())
        hWafermapFile << lFile->getMirDatas().szLot << endl;
    else
        hWafermapFile << mSINFInfo.mLot << endl;

    // Wafer
    hWafermapFile << "WAFER:";
    if(mSINFInfo.mWaferID.isEmpty())
        hWafermapFile << lWafer.szWaferID << endl;
    else
        hWafermapFile << mSINFInfo.mWaferID << endl;

    hWafermapFile << "FNLOC:" << lNotchDirection << endl;

    int	lWaferWidth     = (mSINFInfo.mWaferAndPaddingCols > 0) ? mSINFInfo.mWaferAndPaddingCols :
                                                                 lWaferSizeX;
    int	lWaferHeight    = (mSINFInfo.mWaferAndPaddingRows > 0) ? mSINFInfo.mWaferAndPaddingRows :
                                                                 lWaferSizeY;

    hWafermapFile << "ROWCT:" << lWaferHeight << endl;
    hWafermapFile << "COLCT:"<< lWaferWidth << endl;

    // List of good bins
    if(mSINFInfo.mBCEQ.isEmpty())
        hWafermapFile << "BCEQU:01" << endl;
    else
        hWafermapFile << "BCEQU:" << mSINFInfo.mBCEQ << endl;

    // Die reference location
    if(mSINFInfo.mRefPX != -32768)
    {
        // Report Refernence die as in KLA/INF file
        hWafermapFile << "REFPX:" << mSINFInfo.mRefPX << endl;
        hWafermapFile << "REFPY:" << mSINFInfo.mRefPY << endl;
    }
    else
    {
        if(mSINFInfo.mColRdc || mSINFInfo.mRowRdc)
        {
            // Refernce die from KLA/INF but offset if required (as here we do not have any empty starting wafermap line or col.).
            //m_cSinfInfo.iRefPX = 1 + m_cSinfInfo.iColRdc - iColMin;
            //m_cSinfInfo.iRefPY = 1 + m_cSinfInfo.iRowRdc - iRowMin;
            hWafermapFile << "REFPX:" << (mSINFInfo.mColRdc - lWafer.iLowDieX) << endl;
            hWafermapFile << "REFPY:" << (mSINFInfo.mRowRdc - lWafer.iLowDieY) << endl;
        }
        else
        {
            hWafermapFile << "REFPX:" << lWafer.iLowDieX << endl;
            hWafermapFile << "REFPY:" << lWafer.iLowDieY << endl;
        }
    }
    hWafermapFile << "DUTMS:mm"  << endl;

    // Wafer sizes info: Size in X
    double	lValue;
    if(mSINFInfo.mDieSizeX > 0)
        lValue = mSINFInfo.mDieSizeX;
    else
        lValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(),
                                         lWafer.bWaferUnits);
    if(lValue <= 0)
        lValue = 0;
    hWafermapFile << "XDIES:" << lValue << endl;		// Size in cm

    // Wafer sizes info: Size in Y
    if(mSINFInfo.mDieSizeY > 0)
        lValue = mSINFInfo.mDieSizeY;
    else
        lValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(),
                                         lWafer.bWaferUnits);
    if(lValue <= 0)
        lValue = 0;
    hWafermapFile << "YDIES:" << lValue << endl;		// Size in cm

    // Wafermap
    QString	lDie;

    // Check if Wafer created from a KLA/INF source file
    if(mSINFInfo.mNewWafermap.isEmpty() == false)
    {
        // Simply duplicate the same ASCII wafermap as in KLA/INF
        lDie = mSINFInfo.mNewWafermap;	// Full wafermap
        // Eliminate tabs preceeding the 'RowData:' text wafermap
        lDie = lDie.replace(KLA_ROW_DATA_TAB_STRING, KLA_ROW_DATA_STRING);
        hWafermapFile << lDie << endl;
    }
    else
    {
        // Wafermap
        int     lBinCode;
        int     lStartCol;
        int     lEndCol;
        int     lColStep;
        int     lStartLine;
        int     lEndLine;
        int     lLineStep;

        // Check for X direction
        if(lWafer.GetPosXDirection() == true)
        {
            // X direction = 'R' (right)
            lStartCol   = 0;
            lEndCol     = lWaferSizeX;
            lColStep    = 1;
        }
        else
        {
            // X direction = 'L' (left)
            lStartCol   = lWaferSizeX-1;
            lEndCol     = -1;
            lColStep    = -1;
        }
        // Check for Y direction
        if(lWafer.GetPosYDirection() == true)
        {
            // Y direction = 'D' (down)
            lStartLine  = 0;
            lEndLine    = lWaferSizeY;
            lLineStep   = 1;
        }
        else
        {
            // Y direction = 'U' (up)
            lStartLine  = lWaferSizeY-1;
            lEndLine    = -1;
            lLineStep   = -1;
        }

        // Detect highest bin# so to decide how many digits it takes in the ASCII Hex encoding
        int lBinLength = 0;
        for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
        {
            for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
            {
                // Get PAT-Man binning at location iRow,iCol.
                if(lMergedWafer)
                    lBinCode = lGroup->cStackedWaferMapData.cWafMap[(lColIdx+(lLineIdx*lWaferSizeX))].ldCount;
                else
                    lBinCode = lWafer.getWafMap()[(lColIdx+(lLineIdx*lWaferSizeX))].getBin();

                if (lBinCode != GEX_WAFMAP_EMPTY_CELL)
                    lDie.sprintf("%X", lBinCode);	// Hexa value

                lBinLength = qMax(lBinLength, lDie.length());
            }
        }

        // Make sure bin# takes minimum of 2 digits
        if(lBinLength < 2)
            lBinLength = 2;

        // Write SINF file
        for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
        {
            // Starting line
            hWafermapFile << "RowData:";

            for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
            {
                // Get PAT-Man binning at location iRow,iCol.
                if(lMergedWafer)
                    lBinCode = lGroup->cStackedWaferMapData.cWafMap[(lColIdx+(lLineIdx*lWaferSizeX))].ldCount;
                else
                    lBinCode = lWafer.getWafMap()[(lColIdx+(lLineIdx*lWaferSizeX))].getBin();

                if (lBinCode == GEX_WAFMAP_EMPTY_CELL)
                    lDie = lDie.fill('_', lBinLength);
                else
                    lDie.sprintf("%X", lBinCode);	// Hexa value

                lDie = lDie.rightJustified(lBinLength,'0');

                // Write die Hex. value + space
                hWafermapFile << lDie << " ";
            }

            // Insert line break
            hWafermapFile << endl;
        }
    }

    lOutputFile.close();

    return true;
}

bool WaferExport::CreateSTIFOutputMap(const QString &lOutputFileName, int lGroupIdx, int lFileIdx)
{
    int                 lWaferSizeX;
    int                 lWaferSizeY;
    CGexFileInGroup *   lFile   = NULL;
    CGexGroupOfFiles *  lGroup  = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
        lGroup = NULL;
    else
        lGroup = gexReport->getGroupsList().at(lGroupIdx);

    if (lGroup == NULL)
        return true;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    if(lFileIdx < 0)
        return true;	// mode not supported!
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile = NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
            return true;	// Should never happen!

        lWaferSizeX = lFile->getWaferMapData().SizeX;
        lWaferSizeY = lFile->getWaferMapData().SizeY;
    }

    // Clean Error message
    mErrorMessage.clear();

    // Create a local copy of the wafermap
    lWafer = lFile->getWaferMapData();

    // Check if PosX/PosY have to be overloaded
    if(mAxisXDirection == PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(mAxisXDirection == PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(mAxisXDirection == PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(mAxisXDirection == PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }

    if(mAxisYDirection == PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(mAxisYDirection == PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(mAxisYDirection == PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(mAxisYDirection == PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Compute probing quadrant used
    int	lQuadrant;
    if(lWafer.GetPosXDirection() == true)
    {
        if(lWafer.GetPosYDirection() == true)
            lQuadrant = 2;
        else
            lQuadrant = 3;
    }
    else
    {
        if(lWafer.GetPosYDirection() == true)
            lQuadrant = 1;
        else
            lQuadrant = 4;
    }

    // Rotate wafermap, unless default should be used for the notch
    if(mWaferOrientation <= 0)
        mWaferOrientation = lWafer.GetWaferNotch();
    else if(mRotateWafer)
    {
        while(lWafer.GetWaferNotch() != mWaferOrientation)
            lWafer.RotateWafer();
    }

    // Get wafer size, add 2 rows & cols as wafer to be printed with one extra ring of null dies.
    int lRowsToAdd = 0;
    lWaferSizeX = lRowsToAdd + lWafer.SizeX;
    lWaferSizeY = lRowsToAdd + lWafer.SizeY;

    // Compute notch direction according to G85 specs.
    int	lNotchDirection = 0;	// 0=down,90=left,180=top,270=right
    switch(mWaferOrientation)
    {
        case 12:	// Up
            lNotchDirection = 180;
            break;
        case 6:	// Down
            lNotchDirection = 0;
            break;
        case 3:	// Right
            lNotchDirection = 270;
            break;
        case 9:	// LEFT
            lNotchDirection = 90;
            break;
    }

    // Total bin classes: HARD-BIN Only.
    QString     lGoodBins           = "";
    CBinning *  lBinCell            = NULL;
    int         lTotalBinClasses    = 0;
    int         lTotalBinStatus     = 0;

    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lPatInfo && lPatInfo->GetSTDFHardBins())
        lBinCell = lPatInfo->GetSTDFHardBins();
    else
        lBinCell = lGroup->cMergedData.ptMergedHardBinList;

    while(lBinCell != NULL)
    {
        // Keep track of total Hard-Bin numbers used in wafermap)
        if(lBinCell->ldTotalCount)
        {
            lTotalBinClasses++;

            // Keep track of total Hard-Bin entries with a Pass/Fail info
            if (lBinCell->cPassFail == 'P')
            {
                lTotalBinStatus++;

                // Build list of good bins
                lGoodBins += QString::number(lBinCell->iBinValue) + ",";
            }
            else if (lBinCell->cPassFail == 'F')
                lTotalBinStatus++;
            else
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Bin %1 has no or incorrect pass/fail status(%2)")
                      .arg(lBinCell->iBinValue).
                      arg((char) lBinCell->cPassFail).toLatin1().constData());
        }

        // Move to next Bin cell
        lBinCell = lBinCell->ptNextBin;
    };

    // Check if total bin count or Highest bin# is compatible with STIF format!
    if(lTotalBinClasses == 0)
    {
        mErrorMessage += "STIF map generation: Missing HardBin Pass/Fail info (STDF.HBR record incomplete)";
        return false;	// Failed writing to wafermap file.
    }
    if(lTotalBinClasses > 90)
    {
        mErrorMessage += "STIF map generation: Too many Bins classes for this wafermap format (Max. allowed: 90 classes)";
        return false;	// Failed writing to wafermap file.
    }
    if(lTotalBinClasses != lTotalBinStatus)
    {
        mErrorMessage += "STIF map generation: Not all HardBin records include the Pass/Fail info: STDF.HBR records incomplete";
        return false;	// Failed writing to wafermap file.
    }

    // Build Wafer name: lotID-waferID...but if WaferID already has a '-' sign, then assume it already holds the full string we want.
    QString lWaferId    = lWafer.szWaferID;;
    QString lSubLotId   = lWafer.szWaferID;;

    if(lSubLotId.count("-") <= 0)
        lSubLotId = QString(lFile->getMirDatas().szLot) + QString("-") + QString(lWafer.szWaferID);
    else
        lWaferId = lSubLotId.section("-",1,1);

    // Wafermap
    int     lBinCode;
    int     lStartCol;
    int     lEndCol;
    int     lColStep;
    int     lStartLine;
    int     lEndLine;
    int     lLineStep;

    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        lStartCol   = 0;
        lEndCol     = lWaferSizeX;
        lColStep    = 1;
    }
    else
    {
        // X direction = 'L' (left)
        lStartCol   = lWaferSizeX-1;
        lEndCol     = -1;
        lColStep    = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        lStartLine  = 0;
        lEndLine    = lWaferSizeY;
        lLineStep   = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        lStartLine  = lWaferSizeY-1;
        lEndLine    = -1;
        lLineStep   = -1;
    }

    // STIF signature
    // Note: Buffer to hold the STIF output (we first save it in a string so we can compute the Chekcsum)
    QString lStifOutput = "WM - V1.1 - STMicroelectronics Wafer Map File\n\n";

    // STIF Header section
    double	lValue;
    lStifOutput += "LOT\t" + QString(lFile->getMirDatas().szLot) + "\n";
    lStifOutput += "WAFER\t" + lWaferId + "\n";
    lStifOutput += "PRODUCT\t" + QString(lFile->getMirDatas().szPartType) + "\n";
    lStifOutput += "READER\t" + lSubLotId + "\n";

    // Convert to MM
    if(lWafer.bWaferUnits != 4)
        lValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(), lWafer.bWaferUnits) / 0.0254;
    else
        lValue = lWafer.GetDieHeight() * 10.0;

    if(lValue > 0)
        lStifOutput += "XSTEP\t" +  QString::number(lValue,'f',0) + "\tUNITS\t(0.1)MIL\n";		// Size in 0.1Mils

    // Convert to MM
    if(lWafer.bWaferUnits != 4)
        lValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(),
                                         lWafer.bWaferUnits) / 0.0254;
    else
        lValue = lWafer.GetDieWidth() * 10.0;

    if(lValue > 0)
        lStifOutput += "YSTEP\t" + QString::number(lValue,'f',0) + "\tUNITS\t(0.1)MIL\n";		// Size in 0.1Mils

    lStifOutput += "FLAT\t" + QString::number(lNotchDirection) + "\n";

    if(lWafer.GetCenterDie().IsValidX())
        lStifOutput += "XFRST\t" + QString::number(lWafer.GetCenterDie().GetX()) + "\n";

    if(lWafer.GetCenterDie().IsValidY())
        lStifOutput += "YFRST\t" + QString::number(lWafer.GetCenterDie().GetY()) + "\n";

    lStifOutput += "PRQUAD\t" + QString::number(lQuadrant) + "\n";	// Probing direction (quadrant)
    lStifOutput += "COQUAD\t" + QString::number(lQuadrant) + "\n";	// Probing direction (quadrant)

    // Wafer diameter info (in MM)
    if(lWafer.bWaferUnits != 4)
        lValue = CWaferMap::ConvertToMM(lWafer.GetDiameter(),
                                        lWafer.bWaferUnits) / 0.0254;
    else
        lValue = lWafer.GetDiameter() * 10.0;

    if(lValue > 0)
        lStifOutput += "DIAM\t" + QString::number(lValue/10.0,'f',0) + "\n";	// Size in Mils

    // Wafer top-left corner. Note: one level of empty dies added all around the wafer so upper-left corner offset by one die!
    if(lWafer.GetPosXDirection() == true)
        lStifOutput += "XSTRP\t" + QString::number(lWafer.iLowDieX-(lRowsToAdd/2)) + "\n";
    else
        lStifOutput += "XSTRP\t" + QString::number(lWafer.iHighDieX-(lRowsToAdd/2)) + "\n";

    if(lWafer.GetPosYDirection() == true)
        lStifOutput += "YSTRP\t" + QString::number(lWafer.iLowDieY-(lRowsToAdd/2)) + "\n";
    else
        lStifOutput += "YSTRP\t" + QString::number(lWafer.iHighDieY-(lRowsToAdd/2)) + "\n";

    // Empty die cell code: ~
    lStifOutput += "NULBC\t126\n";

    // Compute Good yield
    int         lGood;
    int         lFail;
    QString     lGoodBinsList;
    CBinning *  pBinning = NULL;

    // Create the good bin list
    if (lPatInfo && lPatInfo->GetSoftBins())
        pBinning = lPatInfo->GetSoftBins();
    else
        pBinning = lGroup->cMergedData.ptMergedSoftBinList;

    // Scan Bin list and identify list of Good and Fail bins
    while (pBinning)
    {
        if (pBinning->cPassFail == 'P')
            lGoodBinsList += QString::number(pBinning->iBinValue) + ",";

        pBinning = pBinning->ptNextBin;
    }

    lWafer.GetMatchingDie(lGood, lFail, lGoodBinsList);
    lStifOutput += "GOODS\t" + QString::number(lGood) + "\n";

    // Date & Time
    QDateTime lDateTime;
    lDateTime.setTimeSpec(Qt::UTC);
    lDateTime.setTime_t(lFile->getMirDatas().lStartT);
    lStifOutput += "DATE\t" + lDateTime.toString("yyyy-MM-dd") + "\n";
    lStifOutput += "TIME\t" + lDateTime.toString("hh:mm:ss") + "\n";
    lStifOutput += "SETUP FILE\t" + QString(lFile->getMirDatas().szSetupID) + "\n";;
    lStifOutput += "OPERATOR\t" + QString(lFile->getMirDatas().szOperator) + "\n";
    lStifOutput += "TEST SYSTEM\t" + QString(lFile->getMirDatas().szNodeName) + "\n";
    lStifOutput += "TEST PROG\t" + QString(lFile->getMirDatas().szJobName) + "\n";

    // If Loadboard definition exists
    if(lFile->m_pSiteEquipmentIDMap != NULL && lFile->m_pSiteEquipmentIDMap->isEmpty() == false)
    {
        GP_SiteDescription	lSiteDescription;

        lSiteDescription = lFile->m_pSiteEquipmentIDMap->begin().value();
        lStifOutput += "PROBE CARD\t" + QString(lSiteDescription.m_strProbeCardID) + "\n";
        lStifOutput += "PROBER\t" + QString(lSiteDescription.m_strHandlerProberID) + "\n";
    }

    // WAFERMAP size: add 2 rows & cols as we add one ring of empty dies.
    lStifOutput += "\n";	// Empty line
    lStifOutput += "WMXDIM=" + QString::number(lWaferSizeX) + "\n";
    lStifOutput += "WMYDIM=" + QString::number(lWaferSizeY) + "\n";
    lStifOutput += "\n";	// Empty line

    // Create wafermap 2 rows & cols bigger so to include the outer ring of empty dies
    int         lArrayIdx;
    int         lWafSize = lWaferSizeX * lWaferSizeY;
    CWaferMap   lEnlargedMap;

    lEnlargedMap.SizeX               = lWaferSizeX;
    lEnlargedMap.SizeY               = lWaferSizeY;
    lEnlargedMap.setWaferMap(CWafMapArray::allocate(lWafSize));

    if (!lEnlargedMap.getWafMap())
    {
        GSLOG(SYSLOG_SEV_ERROR, "WafMap allocation failed !");
        return false;
    }

    if(!lEnlargedMap.allocCellTestCounter(lWafSize))
    {
        GSLOG(SYSLOG_SEV_ERROR, "CellTestCounter allocation failed !");
        return false;
    }

    // HTH TO CHECK
    // Fill map with original (smaller) map
    for(int lLineIdx = 0; lLineIdx < lWaferSizeY-(lRowsToAdd/2); ++lLineIdx)
    {
        // Processing a wafer line.
        for(int lColIdx = 0; lColIdx < lWaferSizeX-(lRowsToAdd/2); ++lColIdx)
        {
            // Index in original map
            lArrayIdx   = (lColIdx-(lRowsToAdd/2)+((lLineIdx-(lRowsToAdd/2))*(lWaferSizeX-lRowsToAdd)));
            lBinCode    = lWafer.getWafMap()[lArrayIdx].getBin();
            lArrayIdx   = (lColIdx+(lLineIdx*(lWaferSizeX)));	// Index in new map (with added 2 cols & rows)

            lEnlargedMap.getWafMap()[lArrayIdx].setBin(lBinCode);
        }
    }
    // Build list of good bins
    GS::QtLib::Range    lBinRange(lGoodBins);
    QString             lDie;

    for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
    {
        // Processing a wafer line.
        for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
        {
            lArrayIdx   = lColIdx + (lLineIdx * lWaferSizeX);
            lBinCode    = lEnlargedMap.getWafMap()[lArrayIdx].getBin();

            if(lBinCode == GEX_WAFMAP_EMPTY_CELL)
            {
                // Empty die: if valid die adjacent then change it force-inking 'z' code
                if(lEnlargedMap.HasValidDieAround(lArrayIdx, CWaferMap::AdjacentDie))
                    lDie = 'z';
                else
                    lDie = '~';

                // Force to '~'
                if(lRowsToAdd == 0)
                    lDie = '~';
            }
            else
            {
                //Due to the constraint of the STIF format bin code usage is restricted to a given number of 90
                //possibilities. Indeed, The Bin code MUST be printable ASCII code. So the first value for bin
                //codes is 32. If the bin codes range provide by the probers begin at 0, the STIF generator add
                //32 to the prober Bin codes.
                //Furthermore, to differentiate the good bin codes from the bad bin codes, the STIF generator
                //adds 128 to the good bin codes
                if(!lBinRange.GetRangeList().isEmpty() && lBinRange.Contains(lBinCode))
                {
                    // Good die: set 'Bit8', and make it printable (add 32)
                    // Good bin can range from 161-232
                    // set 'Bit8'
                    if(lBinCode < 128)
                        lBinCode += 128;

                    // make it printable (add 32)
                    if(lBinCode < 160)
                        lBinCode += 32;
                    if(lBinCode > 232)
                    {
                        mErrorMessage += "STIF map generation: Pass HardBin# exceeds limit (Max. allowed Pass-Bin# is Bin72)";
                        return false;	// Failed writing to wafermap file.
                    }
                }
                else
                {
                    // Fail bin: set 'Bit8'=0,make it printable (add 32)
                    // Fail bin can range from 32-121
                    // Special case for PAT Bins 140-150 can range now from 100-121 (<128 for fail)
                    if(lBinCode >= 140)
                        lBinCode -= 40;
                    // set 'Bit8'=0
                    if(lBinCode >= 128)
                        lBinCode -= 128;
                    // make it printable (add 32)
                    if(lBinCode < 32)
                        lBinCode += 32;
                    if(lBinCode > 232)
                        lBinCode = 232;	// Clamp failing bins to 232 (as they may be PAT Soft bins...)
                }
                lDie = QLatin1Char(lBinCode);
            }

            lStifOutput += lDie;	// Ascii output
        }

        // Insert line break
        lStifOutput += "\n";
    }

    lStifOutput += "\n";

    // Wafermap footer
    lDateTime.setTime_t(lFile->getMirDatas().lEndT);

    lStifOutput += "EDATE\t" + lDateTime.toString("yyyy-MM-dd") + "\n";
    lStifOutput += "ETIME\t" + lDateTime.toString("hh:mm:ss") + "\n";
    lStifOutput += "CHECKSUM\t";

    // Compute STIF file checksum
    int	lSTIFCheckSum=0;
    int	lChar;
    for(int lIdx = 0; lIdx < lStifOutput.size(); lIdx++)
    {
        lChar = lStifOutput[lIdx].toAscii();
        if(lChar != '\r' && lChar != '\n')
        {
            lSTIFCheckSum += lChar;
            lSTIFCheckSum *=16;
            lSTIFCheckSum = lSTIFCheckSum % 251;
        }
    }

    // Add necessary logic including 'AA' checksum in count
    lSTIFCheckSum += 65;
    lSTIFCheckSum *=16;
    lSTIFCheckSum = lSTIFCheckSum % 251;
    lSTIFCheckSum += 65;
    lSTIFCheckSum = lSTIFCheckSum % 251;

    // Negate checksum
    if(lSTIFCheckSum)
        lSTIFCheckSum = 251 - lSTIFCheckSum;

    // Convert checksum to two-character string & add to file
    lStifOutput += QString(65+(lSTIFCheckSum / 16));		// 1st character of the checksum
    lStifOutput += QString(65+(lSTIFCheckSum % 16));		// 2nd character of the checksum
    lStifOutput += "\n";

    // Create file.
    // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    QFile lOutputFile(lOutputFileName);
    if (lOutputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;	// Failed writing to wafermap file.

    // Open Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&lOutputFile);	// Assign file handle to data stream

    // Write full wafermap array to disk.
    hWafermapFile << lStifOutput;

    lOutputFile.close();

    return true;
}

// TO CHECK Why don't we use the lGroupIdx and lFileIdx??
bool WaferExport::CreateTELP8OutputMap(const QString &lOutputFileName, int /*lGroupIdx*/, int /*lFileIdx*/)
{

    // Pointer to Data file so to extract  Wafermap Bin results.
    CGexGroupOfFiles *  lGroup  = gexReport->getGroupsList().isEmpty() ? NULL :
                                                                         gexReport->getGroupsList().first();
    CGexFileInGroup *   lFile   = (!lGroup || lGroup->pFilesList.isEmpty()) ? NULL :
                                                                              lGroup->pFilesList.first();
    CWaferMap           lWafer;
    int                 lWaferSizeX;
    int                 lWaferSizeY;

    if (lFile == NULL)
        return true;

    // Create a local copy of the wafermap
    lWafer = lFile->getWaferMapData();

    // Check if wafermap must be samed using old P8 format or new Million-die format...
    bool    lNewP8Format = false;

    if  (gex_max(lWafer.SizeX, lWafer.SizeY) > 127)
        lNewP8Format = true;

    // Compute total pass/fail/dies
    int     lBinCode;
    long	lTotalPass = 0;
    long	lTotalFail = 0;
    long	lTestTotal = 0;

    for(int lColIdx = 0; lColIdx < lWafer.SizeX; ++lColIdx)
    {
        for(int lLineIdx = 0; lLineIdx < lWafer.SizeY; ++lLineIdx)
        {
            lBinCode = lWafer.getWafMap()[(lColIdx+(lLineIdx*lWafer.SizeX))].getBin();
            switch(lBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:
                    break;

                case 1:		// Good die
                    lTotalPass++;
                    lTestTotal++;
                    break;

                default:	// Failing die
                    lTotalFail++;
                    lTestTotal++;
                    break;
            }
        }
    }

    // Create file.
    // Write the text to the file
    QFile lOutputFile(lOutputFileName);

    if (lOutputFile.open(QIODevice::WriteOnly) == false)
        return false;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&lOutputFile);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(mAxisXDirection == PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(mAxisXDirection == PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(mAxisXDirection == PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(mAxisXDirection == PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(mAxisYDirection == PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(mAxisYDirection == PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(mAxisYDirection == PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(mAxisYDirection == PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(mWaferOrientation <= 0)
        mWaferOrientation = lWafer.GetWaferNotch();
    else if(mRotateWafer)
    {
        while(lWafer.GetWaferNotch() != mWaferOrientation)
            lWafer.RotateWafer();
    }

    lWaferSizeX = lWafer.SizeX;
    lWaferSizeY = lWafer.SizeY;

    // Wafermap
    int     lStartCol;
    int     lEndCol;
    int     lColStep;
    int     lStartLine;
    int     lEndLine;
    int     lLineStep;

    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        lStartCol   = 0;
        lEndCol     = lWaferSizeX;
        lColStep    = 1;
    }
    else
    {
        // X direction = 'L' (left)
        lStartCol   = lWaferSizeX-1;
        lEndCol     = -1;
        lColStep    = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        lStartLine  = 0;
        lEndLine    = lWaferSizeY;
        lLineStep   = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        lStartLine  = lWaferSizeY-1;
        lEndLine    = -1;
        lLineStep   = -1;
    }

    unsigned char   lLSB;
    unsigned char   lMSB;
    QString         lTmpString;
    QDateTime       lDateTime;
    if(lNewP8Format)
    {
        // TEL-P8 Million-die format

        CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
        if (lPatInfo)
        {
            // Total Pass
            lTmpString.sprintf("%011ld", lTotalPass - lPatInfo->m_lstOutlierParts.count());
            hWafermapFile << lTmpString;

            // Total Fail
            lTmpString.sprintf("%011ld", lTotalFail + lPatInfo->m_lstOutlierParts.count());
            hWafermapFile << lTmpString;
        }

        // Total tested
        lTmpString.sprintf("%011ld",lTestTotal);
        hWafermapFile << lTmpString;

        // Wafer starting time (BCD 12 bytes)
        // eg: 0 5 1 2 1 2  0 9 0 4 0 2
        //     Y Y M M D D  H H M M S S
        if(lWafer.lWaferStartTime > 0)
            lDateTime.setTime_t(lWafer.lWaferStartTime);
        else
            lDateTime = QDateTime::currentDateTime();

        hWafermapFile << lDateTime.toString("yyMMddhhmmss");

        // Wafer ending time (BCD 12 bytes)
        if(lWafer.lWaferEndTime > 0)
            lDateTime.setTime_t(lWafer.lWaferEndTime);
        else
        {
            // Valid Start time?...if so use it, otherwise use current time!
            if(lWafer.lWaferStartTime > 0)
                lDateTime.setTime_t(lWafer.lWaferStartTime);
            else
                lDateTime = QDateTime::currentDateTime();
        }

        hWafermapFile << lDateTime.toString("yyMMddhhmmss");

        // Wafer-ID (36 characters)
        lTmpString.sprintf("%36s",lWafer.szWaferID);
        hWafermapFile << lTmpString;

        // Wafer# (2 chars.)
        lTmpString.sprintf("%2s",lWafer.szWaferID);
        hWafermapFile << lTmpString;

        // Cassette# (1 char)
        hWafermapFile << "1";

        // Slot# (2 chars)
        hWafermapFile << "25";

        // Test count (1 char)
        hWafermapFile << "1";

        // Map Coordinate System+BIN type + distance info + address info (58 chars.)
        hWafermapFile << "          ";
        hWafermapFile << "          ";
        hWafermapFile << "          ";
        hWafermapFile << "          ";
        hWafermapFile << "          ";
        hWafermapFile << "        ";

        // Number of records (2 bytes): one per X line
        lLSB = (lWaferSizeY) & 0xff;
        lMSB = (lWaferSizeY >> 8) & 0xff;

        hWafermapFile << lLSB;	// LSB
        hWafermapFile << lMSB;	// MSB

        BYTE            lFlag;
        unsigned long   lBin;

        // Wafermap
        for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
        {
            // Location of first die in X (2 bytes) - LSB, MSB
            hWafermapFile << 0 << 0;

            // TO CHECK if it should be lWaferSizeX instead
            // Dies in row
            lLSB = (lWaferSizeY) & 0xff;
            lMSB = (lWaferSizeY >> 8) & 0xff;

            hWafermapFile << lLSB;	// LSB
            hWafermapFile << lMSB;	// MSB

            // Processing a wafer line.
            for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
            {
                // Read Die binning
                lBinCode = lWafer.getWafMap()[(lColIdx + (lLineIdx * lWaferSizeX))].getBin();

                // If die tested
                switch(lBinCode)
                {
                    case GEX_WAFMAP_EMPTY_CELL:
                        lFlag   = 0x80;	// Set 'No test' flag
                        lBin    =0;
                        break;

                    case 1:		// Good die
                        lFlag   = 0x0;
                        lBin    = 2;
                        break;

                    default:	// Failing die: Force inking, set fail flag
                        if(lBinCode > 31)
                            lBinCode = 31;
                        lFlag   = 0x1;              // Set 'FAIL' flag
                        lBin    = 1UL << lBinCode;
                        break;
                }

                // Bin: 32 bits
                hWafermapFile << (BYTE) (lBin & 0xff);
                hWafermapFile << (BYTE) ((lBin >> 8) & 0xff);
                hWafermapFile << (BYTE) ((lBin >> 16) & 0xff);
                hWafermapFile << (BYTE) ((lBin >> 24) & 0xff);

                // Byte#1: B7= No Test, B5= Skip, B4= Force inking, B1= Special specs, B0= Fail
                // B6,B3,B2 = unused
                hWafermapFile << lFlag;
            }
        }
    }
    else
    {
        // TEL-P8 old format (less than 128 dies in X and Y)

        // Lot-ID (25 characters)
        lTmpString.sprintf("%25s", lFile->getMirDatas().szLot);
        hWafermapFile << lTmpString;

        // Wafer# (2 chars.)
        lTmpString.sprintf("%2s", lWafer.szWaferID);
        hWafermapFile << lTmpString;

        // Cassette# (1 char)
        hWafermapFile << "1";

        // Slot# (2 chars)
        hWafermapFile << "25";

        // Test count (1 char)
        hWafermapFile << "1";

        // Total pass (2 bytes): LSB, MSB
        lMSB = lTotalPass / 256;
        lLSB = lTotalPass % 256;
        hWafermapFile << lLSB << lMSB;

        // Total Fail (2 bytes): LSB, MSB
        lMSB = lTotalFail / 256;
        lLSB = lTotalFail % 256;
        hWafermapFile << lLSB << lMSB;

        // Total tests (2 bytes): LSB, MSB
        lMSB = (lTotalPass+lTotalFail) / 256;
        lLSB = (lTotalPass+lTotalFail) % 256;
        hWafermapFile << lLSB << lMSB;

        // Wafer starting time (BCD 12 bytes)
        // eg: 0 5 1 2 1 2  0 9 0 4 0 2
        //     Y Y M M D D  H H M M S S
        hWafermapFile << GetBCDTime(lWafer.lWaferStartTime);

        // Wafer ending time (BCD 12 bytes)
        hWafermapFile << GetBCDTime(lWafer.lWaferEndTime);

        // Number of records (1 byte).
        // Each record holds one line of dies (ie: having same X value)
        // Up to 127 dies allowed per X record (127*127 = 16129 dies max. supported per wafer)
        hWafermapFile << (BYTE) (lWaferSizeY);

        // X,Y Distance from origin (4 bytes)
        hWafermapFile << 0 << 0 << 0 << 0;

        BYTE    lFlag;
        for(int lLineIdx = lStartLine; lLineIdx != lEndLine; lLineIdx += lLineStep)
        {
            // Location of first die in X (2 bytes)
            hWafermapFile << 0 << 0;

            // Location of first die in Y (2 bytes)
            hWafermapFile << 0 << 0;

            // Write Total dies in this record (1 byte). 127 dies max. allowed per record.
            hWafermapFile << (BYTE) (lWaferSizeX & 0xFF);

            // Processing a wafer line.
            for(int lColIdx = lStartCol; lColIdx != lEndCol; lColIdx += lColStep)
            {
                // Read Die binning
                lBinCode = lWafer.getWafMap()[(lColIdx + (lLineIdx * lWaferSizeX))].getBin();

                // If die tested
                switch(lBinCode)
                {
                    case GEX_WAFMAP_EMPTY_CELL:
                        lFlag = 0x80;	// Set 'No test' flag
                        break;

                    case 1:		// Good die
                        lFlag = 0x0;
                        break;

                    default:	// Failing die: Force inking, set fail flag
                        lFlag = 0x11;
                        break;
                }

                // Byte#1: B7= No Test, B5= Skip, B4= Force inking, B1= Special specs, B0= Fail
                // B6,B3,B2 = unused
                hWafermapFile << lFlag;

                // Byte#2: Binning
                hWafermapFile << (BYTE) (lBinCode & 0xFF);
            }
        }
    }

    lOutputFile.close();

    return true;
}

bool WaferExport::GenerateSumMicronOutputMap(const QString &lOutputFileName,
                                             int lGroupIdx,
                                             int lFileIdx)
{
    // Pointer to Data file so to extract  Wafermap Bin results.
    CGexFileInGroup *   lFile   = NULL;
    CGexGroupOfFiles *  lGroup  = NULL;
    if ((lGroupIdx < 0) || (lGroupIdx >= gexReport->getGroupsList().size()))
    {
        lGroup = NULL;
    }
    else
    {
        lGroup = gexReport->getGroupsList().at(lGroupIdx);
    }

    if (lGroup == NULL)
    {
        return true;	// Should never happen!
    }

    // Check if wafer to export is 'merged wafer'
    if(lFileIdx < 0)
    {
        return true;	// mode not supported!
    }
    else
    {
        if (lFileIdx >= lGroup->pFilesList.size())
            lFile = NULL;
        else
            lFile = lGroup->pFilesList.at(lFileIdx);

        if(lFile == NULL)
        {
            return true;	// Should never happen!
        }
    }

    // we must have only one file
    if (mOriginalFiles.size() <= 0)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty input file list");
        return false;
    }
    std::string lOriginalFileName = mOriginalFiles[0].toStdString();
    GS::Parser::ParserFactory *lMicronFactory = GS::Parser::ParserFactory::GetInstance();
    if(lMicronFactory == NULL)
        return false;

    GS::Parser::ParserAbstract *lParser = lMicronFactory->CreateParser(lOriginalFileName);
    if(lParser == NULL || lParser->GetType() != GS::Parser::typeMicron)
    {
        lMicronFactory->ReleaseInstance();
        return false;
    }
    lParser->SetProgressHandler(NULL);


    // uncompressing the original directory
    GSLOG(SYSLOG_SEV_DEBUG, QString("Openning file %1")
                            .arg(QString::fromStdString(lOriginalFileName)).toLatin1().constData());
    QTemporaryDir   lTempDir;
    QStringList     lUncompressedFiles;
    CArchiveFile    lArchive(lTempDir.path());

    if (lArchive.Uncompress(QString::fromStdString(lOriginalFileName), lUncompressedFiles) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to uncompress the input file %1")
                                .arg(QString::fromStdString(lOriginalFileName)).toLatin1().constData());
        return false;
    }

    QMap<QString, QString>  lInputFiles;
    for (int lIdx = 0; lIdx < lUncompressedFiles.count(); ++lIdx)
    {
        QFileInfo lFileInfo(QDir(lTempDir.path()), lUncompressedFiles.at(lIdx));

        lInputFiles.insert(lFileInfo.fileName().toLower(), lFileInfo.absoluteFilePath());
    }

    QString lHeader("header");
    QString lSummary("summary");
    QString lTrendLog("trend.prblog");
    if (lInputFiles.contains(lHeader) == false
        || lInputFiles.contains(lSummary) == false
        || lInputFiles.contains(lTrendLog) == false)
    {
        // Missing input files
        GSLOG(SYSLOG_SEV_ERROR, QString("Missing input files in the folder %1")
                                .arg(QString::fromStdString(lOriginalFileName)).toLatin1().constData());
        return false;
    }

    // Create file.
    // Write the text to the file
    QFile lOutputFile(lOutputFileName);
    if (lOutputFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to open output file %1")
                                .arg(lOutputFileName).toLatin1().constData());
        return false;	// Failed writing to wafermap file.
    }

    // Read the header file and write the header section in the .sum file
    QString lHeaderFile = lInputFiles.value(lHeader);
    WriteMicronSumHeader(lHeaderFile, lOutputFile, lFile);

    // Read summary file and write the summary section
    QMap<QString, CPatDefinition *> lEnabledPatTests;
    QString lSummaryFile = lInputFiles.value(lSummary);
    QMap<QString, QString> lEnabledPatRules;
    WriteMicronSumSummary(lSummaryFile, lOutputFile, lEnabledPatTests, lEnabledPatRules);

    QString lTrendLogFile = lInputFiles.value(lTrendLog);
    // Read the input trend_log to save all the die definition (8698926:22:P13:11:12 H1S22 .:*)
    QStringList lDiesList, binsList;
    if (GetDiesListFromTrendLog(lTrendLogFile, lDiesList, binsList))
    {
        WriteMicronSumTrendLog(lDiesList, binsList, lOutputFile, lEnabledPatTests, lEnabledPatRules);
    }
    else
    {
        return false;
    }

    lOutputFile.close();
    return true;
}



bool WaferExport::WriteMicronSumHeader(QString headerFileName, QFile& outputFile, CGexFileInGroup* stdfFile)
{
    // Open header file
    QString lStrString("");
    QFile lFile(headerFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to open header file %1")
                                .arg(headerFileName).toLatin1().constData());
        return false;
    }
    // Assign file I/O stream
    QTextStream lHeaderStreamFile(&lFile);
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Empty pat info %1")
                                .arg(headerFileName).toLatin1().constData());
        return false;
    }


    // Open output file
    QTextStream lSumStreamFile;
    lSumStreamFile.setDevice(&outputFile);
    lSumStreamFile << "$$_BEGIN_HEADER" << endl;
    lSumStreamFile << "ASSET_ID: " << QFileInfo(outputFile).fileName().remove(".FQQP.sum") << endl;
    lSumStreamFile << "DESIGN_ID: " << stdfFile->getMirDatas().szDesignRev << endl;
    lSumStreamFile << "ENG_CONTACT: " << stdfFile->getMirDatas().szEngID << endl;
    lSumStreamFile << "FAB: " << stdfFile->getMirDatas().szFloorID << endl;


    short lReadLine(0);
    while (lStrString.compare("$$_END_HEADER") != 0
           && !lHeaderStreamFile.atEnd()
           && lReadLine < 400)
    {
        if (lStrString.startsWith("FID_LOT", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
            QDateTime lFinishTime(QDateTime::fromTime_t(stdfFile->getMirDatas().lEndT));
            lSumStreamFile << "FINISH_DATETIME: " << lFinishTime.toString("MM/dd/yyyy h:m:s") << endl;
            lSumStreamFile << "FINISH_ETIME: " << stdfFile->getMirDatas().lEndT << endl;
        }
        else if (lStrString.startsWith("FLAT_POSITION", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
            lSumStreamFile << "GOOD_DIE: " << lPatInfo->GetTotalGoodPartsPostPAT() << endl;
        }
        else if (lStrString.startsWith("LOG_REV", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("LOT:", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("LOT_ID:", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("LOT_PREFIX", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
            lSumStreamFile << "MAP_REV: " << QFileInfo(lPatInfo->GetRecipeFilename()).fileName() << endl;
            lSumStreamFile << "MAP_VERS: " << lPatInfo->GetRecipe().GetOptions().GetRecipeVersion() << endl;
            lSumStreamFile << "OPERATOR: Quantix "<< endl;
            lSumStreamFile << "PART_TYPE: " << stdfFile->getMirDatas().szPartType << endl;
            lSumStreamFile << "PRB_CARD: 0"<< endl;
            lSumStreamFile << "PRB_FACILITY: "<< stdfFile->getMirDatas().szFacilityID << endl;
            lSumStreamFile << "PROCESS_ID: FQQP"<< endl;
            lSumStreamFile << "PROGRAM: " << QFileInfo(lPatInfo->GetRecipeFilename()).fileName() << endl;
            lSumStreamFile << "RUN_ID: 00"<< endl;
        }
        else if (lStrString.startsWith("START_DATETIME", Qt::CaseInsensitive))
        {
            QDateTime lStartTime(QDateTime::fromTime_t(stdfFile->getMirDatas().lStartT));
            lSumStreamFile << "START_DATETIME: " << lStartTime.toString("MM/dd/yyyy h:m:s") << endl;
            lSumStreamFile << "START_ETIME: "<< stdfFile->getMirDatas().lStartT << endl;
        }
        else if (lStrString.startsWith("START_LOT_KEY", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
            lSumStreamFile << "STATION_NAME: " << GEX_APP_PATMAN <<endl;
            lSumStreamFile << "TESTER: " << GEX_APP_PATMAN <<endl;
            lSumStreamFile << "TOTAL_DIE: " << lPatInfo->GetSTDFTotalDies() <<endl;
            lSumStreamFile << "TOUCHDOWNS: 1" << endl;
        }

        else if (lStrString.startsWith("UNITS_EXPECTED", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("WAFER: ", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("WAFER_ID: ", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("WAFER_SCRIBE: ", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        else if (lStrString.startsWith("WAF_SIZE: ", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
        }
        lStrString = lHeaderStreamFile.readLine().trimmed();
        ++lReadLine;
    }
    lSumStreamFile << "$$_END_HEADER" << endl;

    return true;
}


bool WaferExport::WriteMicronSumSummary(QString summaryFileName,
                                        QFile &outputFile,
                                        QMap<QString, CPatDefinition *>& enabledPatTests,
                                        QMap<QString, QString>& enabledPatRules)
{

    // Open header file
    QString lStrString("");
    QString lListBin("");
    QFile lFile(summaryFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to open header file %1")
                                .arg(summaryFileName).toLatin1().constData());
        return false;
    }
    // Assign file I/O stream
    QTextStream lSummaryStreamFile(&lFile);
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty pat info ");
        return false;
    }

    const COptionsPat & lPatOption = lPatInfo->GetRecipe().GetOptions();

    // Open output file
    QTextStream lSumStreamFile;
    lSumStreamFile.setDevice(&outputFile);
    lSumStreamFile << "$$_BEGIN_SUMMARY 2.0" << endl;

    while (!lSummaryStreamFile.atEnd())
    {
        if (lStrString.contains("Good die", Qt::CaseInsensitive)
            && lStrString.contains("FM_BIN_DEF", Qt::CaseInsensitive)
            && lStrString.contains("BIN_1", Qt::CaseInsensitive))
        {
            lSumStreamFile << lStrString << endl;
            lSumStreamFile << "#define FM_BIN_DEF DPAT FAIL BIN_DPAT 00 (REG# "<< lPatOption.iFailDynamic_HBin
                           << ") Dynamic PPAT fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF SPAT FAIL BIN_SPAT 00 (REG# "<< lPatOption.iFailStatic_HBin
                           << ") Static PPAT fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF NPAT FAIL BIN_NPAT 00 (REG# "<< lPatOption.GetNNRHardBin()
                           << ") NNR fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF IPAT FAIL BIN_IPAT 00 (REG# "<< lPatOption.mIDDQ_Delta_HBin
                           << ") Iddq Drift Fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF GPAT FAIL BIN_GPAT 00 (REG# "<< lPatOption.mGDBNPatHBin
                           << ") Geographic fail Good Die Bad Neighborhood" << endl;
            lSumStreamFile << "#define FM_BIN_DEF RPAT FAIL BIN_RPAT 00 (REG# "<< lPatOption.GetReticleHardBin()
                           << ") Reticle based fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF CPAT FAIL BIN_CPAT 00 (REG# "<< lPatOption.mClusteringPotatoHBin
                           << ") Geographic Cluster Fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF ZPAT FAIL BIN_ZPAT 00 (REG# "<<lPatOption.iCompositeZone_HBin
                           << ") Z-PAT fail" << endl;
            lSumStreamFile << "#define FM_BIN_DEF MVPAT FAIL BIN_MVPAT 00 (REG# "<<lPatOption.GetMVPATHardBin()
                           << ") Multi-Variate PPAT Fail" << endl;
            lStrString = lSummaryStreamFile.readLine().trimmed();
        }

        while (lStrString.startsWith("#define")
               && lStrString.contains("FM_BIN_DEF", Qt::CaseInsensitive)
               && !lSummaryStreamFile.atEnd())
        {
            lSumStreamFile << lStrString << endl;
            lStrString = lSummaryStreamFile.readLine().trimmed();
        }

        while (lStrString.startsWith("#define")
               && lStrString.contains("EC_BIN_DEF", Qt::CaseInsensitive)
               && !lSummaryStreamFile.atEnd())
        {
            lSumStreamFile << lStrString << endl;
            lStrString = lSummaryStreamFile.readLine().trimmed();
        }

        if (lStrString.contains("%thiswaf_bin =")
            || lStrString.contains("%thiswaf_bin="))
        {
            // Write until the end of this section
            while (!lStrString.contains(");", Qt::CaseInsensitive)
                   && !lSummaryStreamFile.atEnd())
            {
                lSumStreamFile << lStrString << endl;
                lStrString = lSummaryStreamFile.readLine().trimmed();
            }
            // write PAT bins
            lSumStreamFile << "'DPAT'," << lPatInfo->GetDPATPartCount() << "," << endl;
            lSumStreamFile << "'SPAT'," << lPatInfo->GetSPATPartCount() << "," << endl;
            lSumStreamFile << "'NPAT'," << lPatInfo->mNNROutliers.count() << "," << endl;
            lSumStreamFile << "'MVPAT'," << lPatInfo->GetMVPATPartCount() << "," << endl;
            lSumStreamFile << "'IPAT'," << lPatInfo->mIDDQOutliers.count() << "," << endl;
            lSumStreamFile << "'GPAT'," << lPatInfo->mGDBNOutliers.count() << "," << endl;
            lSumStreamFile << "'CPAT'," << lPatInfo->mClusteringOutliers.count() << "," << endl;
            lSumStreamFile << "'ZPAT'," << lPatInfo->mZPATOutliers.count() << "," << endl;
            lSumStreamFile << "'RPAT'," << lPatInfo->mReticleOutliers.count() << "," << endl;
            lSumStreamFile << ");" << endl;
        }

        if (lStrString.contains("%thiswaf_binbit"))
        {
            // Write until the end of this section
            while (!lStrString.contains(");", Qt::CaseInsensitive)
                   && !lSummaryStreamFile.atEnd())
            {
                lSumStreamFile << lStrString << endl;
                lStrString = lSummaryStreamFile.readLine().trimmed();
            }
            // write PAT bins
            lSumStreamFile << "'DPAT:DPAT'," << lPatInfo->GetDPATPartCount() << "," << endl;
            lSumStreamFile << "'SPAT:SPAT'," << lPatInfo->GetSPATPartCount() << "," << endl;
            lSumStreamFile << "'NPAT:NPAT'," << lPatInfo->mNNROutliers.count() << "," << endl;
            lSumStreamFile << "'MVPAT:MVPAT'," << lPatInfo->GetMVPATPartCount() << "," << endl;
            lSumStreamFile << "'IPAT:IPAT'," << lPatInfo->mIDDQOutliers.count() << "," << endl;
            lSumStreamFile << "'GPAT:GPAT'," << lPatInfo->mGDBNOutliers.count() << "," << endl;
            lSumStreamFile << "'CPAT:CPAT'," << lPatInfo->mClusteringOutliers.count() << "," << endl;
            lSumStreamFile << "'ZPAT:ZPAT'," << lPatInfo->mZPATOutliers.count() << "," << endl;
            lSumStreamFile << "'RPAT:RPAT'," << lPatInfo->mReticleOutliers.count() << "," << endl;
            lSumStreamFile << ");" << endl;
        }

        if (lStrString.contains("%thiswaf_misc"))
        {
            while (!lStrString.contains("bin_seq")
                   && !lSummaryStreamFile.atEnd())
            {
                 lStrString = lSummaryStreamFile.readLine().trimmed();
            }
            if (lStrString.contains("bin_seq"))
            {
                lListBin = lStrString.section("[", 1).section("]", 0, 0);
            }

        }
        // Read next line
        lStrString = lSummaryStreamFile.readLine().trimmed();
    }

    // Write thiswaf_trend section
    lSumStreamFile << "%thiswaf_trend = (" << endl;

    QHash<QString, int> lOutliersPerTest;
    GetOutliersPerTest(lOutliersPerTest);
    int lNumberTests = lPatInfo->GetSTDFTotalDies();
    QList<CPatDefinition*> lUnivariateTests = lPatInfo->GetUnivariateRules().values();

    // We write this bloc multiple time to have all tests grouped by type (DPAT, SPAT, NPAT...)
    // Write DPAT tests
    for (int i=0; i<lUnivariateTests.size(); ++i)
    {
        CPatDefinition* lPatDef = lUnivariateTests[i];
        if (!lPatDef)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Pat definition null");
        }
        if (lPatDef->m_OutliersToKeep != GEX_TPAT_RULETYPE_IGNOREID
            && lPatDef->m_lFailDynamicBin != -1)
        {
            lSumStreamFile << "'DPAT_" << lPatDef->m_strTestName;
            enabledPatTests.insertMulti("DPAT", lPatDef);
            if (lOutliersPerTest.contains(lPatDef->m_strTestName))
            {
                int lOutliers = lOutliersPerTest.value(lPatDef->m_strTestName);
                lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
            }
            else
            {
                lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
            }
        }
    }

    // Write SPAT tests
    for (int i=0; i<lUnivariateTests.size(); ++i)
    {
        CPatDefinition* lPatDef = lUnivariateTests[i];
        if (!lPatDef)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Pat definition null");
        }
        if (lPatDef->m_lFailStaticBin != -1)
        {
            lSumStreamFile << "'SPAT_" << lPatDef->m_strTestName;
            enabledPatTests.insertMulti("SPAT", lPatDef);
            if (lOutliersPerTest.contains(lPatDef->m_strTestName))
            {
                int lOutliers = lOutliersPerTest.value(lPatDef->m_strTestName);
                lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
            }
            else
            {
                lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
            }
        }
    }

    // Write NNR tests
    for (int i=0; i<lUnivariateTests.size(); ++i)
    {
        CPatDefinition* lPatDef = lUnivariateTests[i];
        if (!lPatDef)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Pat definition null");
        }
        if (lPatDef->m_iNrrRule == GEX_TPAT_NNR_ENABLED)
        {
            lSumStreamFile << "'NPAT_" << lPatDef->m_strTestName;
            enabledPatTests.insertMulti("NPAT", lPatDef);
            if (lOutliersPerTest.contains(lPatDef->m_strTestName))
            {
                int lOutliers = lOutliersPerTest.value(lPatDef->m_strTestName);
                lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
            }
            else
            {
                lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
            }
        }
    }

    // write MVPAT tests
    const QList<GS::Gex::PATMultiVariateRule> lMultivariateTests = lPatInfo->GetMultiVariateRules();
    for (int i=0; i<lMultivariateTests.size(); ++i)
    {
        PATMultiVariateRule lMVRule = lMultivariateTests[i];
        if (lMVRule.GetEnabled())
        {
            lSumStreamFile << "'MVPAT_" << lMVRule.GetName();
            enabledPatRules.insertMulti("MVPAT", lMVRule.GetName());
            if (lOutliersPerTest.contains(lMVRule.GetName()))
            {
                int lOutliers = lOutliersPerTest.value(lMVRule.GetName());
                lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
            }
            else
            {
                lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
            }
        }
    }


    // write GDBN tests
    if (lPatOption.mIsGDBNEnabled)
    {
        for (int i=0; i<lPatOption.mGDBNRules.size(); ++i)
        {
            if (lPatOption.mGDBNRules[i].mIsEnabled)
            {
                QString lRuleName = lPatOption.mGDBNRules[i].mRuleName;
                enabledPatRules.insertMulti("GPAT", lRuleName);
                lSumStreamFile << "'GPAT_" << lRuleName;
                if (lOutliersPerTest.contains(lRuleName))
                {
                    int lOutliers = lOutliersPerTest.value(lRuleName);
                    lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
                }
                else
                {
                    lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
                }
            }
        }
    }

    // write clustering tests
    if (lPatOption.mClusteringPotato)
    {
        for (int i=0; i<lPatOption.mClusterPotatoRules.size(); ++i)
        {
            if (lPatOption.mClusterPotatoRules[i].mIsEnabled)
            {
                QString lRuleName = lPatOption.mClusterPotatoRules[i].mRuleName;
                lSumStreamFile << "'CPAT_" << lRuleName;
                enabledPatRules.insertMulti("CPAT", lRuleName);
                if (lOutliersPerTest.contains(lRuleName))
                {
                    int lOutliers = lOutliersPerTest.value(lRuleName);
                    lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
                }
                else
                {
                    lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
                }
            }
        }
    }

    // write iddq_delta tests
    if (lPatOption.mIsIDDQ_Delta_enabled)
    {
        for (int i=0; i<lPatOption.mIDDQ_Delta_Rules.size(); ++i)
        {
            if (lPatOption.mIDDQ_Delta_Rules[i].IsEnabled())
            {
                QString lRuleName = lPatOption.mIDDQ_Delta_Rules[i].GetRuleName();
                lSumStreamFile << "'IPAT_" << lRuleName;
                enabledPatRules.insertMulti("IPAT", lRuleName);
                if (lOutliersPerTest.contains(lRuleName))
                {
                    int lOutliers = lOutliersPerTest.value(lRuleName);
                    lSumStreamFile << "',[" << lOutliers << "," << lNumberTests << ",0," << lOutliers << "]," <<endl;
                }
                else
                {
                    lSumStreamFile << "',[0," << lNumberTests << ",0,0],"<< endl;
                }
            }
        }
    }

    QList<int> lPatOutlierPerAlgo;
    if(!GetNumberOutliersPerPatAlgo(lPatOutlierPerAlgo))
    {
        return false;
    }

    // write Reticle tests
    if (lPatOption.GetReticleEnabled())
    {
        lSumStreamFile << "'CPAT_Rule";
        enabledPatRules.insertMulti("CPAT", "CPAT_Rule");
        lSumStreamFile << "',[" << lPatOutlierPerAlgo[3] << "," << lNumberTests
                       << ",0," << lPatOutlierPerAlgo[3] << "]," <<endl;
    }

    // write ZPAT Clustering tests
    if (lPatOption.bZPAT_Clustering_Enabled)
    {
        lSumStreamFile << "'ZPAT_ClusteringRule";
        enabledPatRules.insertMulti("ZPAT", "ZPAT_ClusteringRule");
        lSumStreamFile << "',[" << lPatOutlierPerAlgo[5] << "," << lNumberTests
                       << ",0," << lPatOutlierPerAlgo[5] << "]," <<endl;
    }

    // write ZPAT GDBN tests
    if (lPatOption.bZPAT_GDBN_Enabled)
    {
        lSumStreamFile << "'ZPAT_GDBNRule";
        enabledPatRules.insertMulti("ZPAT", "ZPAT_GDBNRule");
        lSumStreamFile << "',[" << lPatOutlierPerAlgo[5] << "," << lNumberTests
                       << ",0," << lPatOutlierPerAlgo[5] << "]," <<endl;
    }

    // write ZPAT GDBN tests
    if (lPatOption.bZPAT_Reticle_Enabled)
    {
        lSumStreamFile << "'ZPAT_ReticleRule";
        enabledPatRules.insertMulti("ZPAT", "ZPAT_ReticleRule");
        lSumStreamFile << "',[" << lPatOutlierPerAlgo[5] << "," << lNumberTests
                       << ",0," << lPatOutlierPerAlgo[5] << "]," <<endl;
    }


    lSumStreamFile << ");" << endl;

    // Write thiswaf_misc section
    lSumStreamFile << "%thiswaf_misc = (" << endl;
    lSumStreamFile << "'bin_seq'," << endl;
    lSumStreamFile << "['.'," << lListBin << "'DPAT','SPAT','NPAT','MVPAT','IPAT','GPAT','CPAT','ZPAT','RPAT'],"<< endl;
    lSumStreamFile << ");" << endl;

    lSumStreamFile << "$$_END_SUMMARY" << endl;

    return true;
}

bool WaferExport::WriteMicronSumTrendLog(QStringList& diesList,
                                         QStringList& binsList,
                                         QFile &outputFile,
                                         QMap<QString, CPatDefinition*>& enabledPatTests,
                                         QMap<QString, QString>& enabledPatRules)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty pat info");
        return false;
    }

//    const COptionsPat & lPatOption = lPatInfo->GetRecipe().GetOptions();

    // Open output file
    QTextStream lSumStreamFile;
    lSumStreamFile.setDevice(&outputFile);
    lSumStreamFile << "$$_BEGIN_PROBE_LOG" << endl;
    lSumStreamFile << "#define FID_FORMAT LOT WAFER ROW COL WAFSIZE" << endl;
    lSumStreamFile << "#define BIN_SEQ ZPAT SPAT DPAT MVPAT";
    QStringList lRulePrecedenceString = lPatInfo->GetRecipeOptions().strRulePrecedence;
    QStringList lRulePrecedence;
    for (int i=0; i<lRulePrecedenceString.size(); ++i)
    {
        if (lRulePrecedenceString[i] == "NNR")
            lRulePrecedence.append("NPAT");
        if (lRulePrecedenceString[i] == "IDDQ-Delta")
            lRulePrecedence.append("IPAT");
        if (lRulePrecedenceString[i] == "GDBN")
            lRulePrecedence.append("GPAT");
        if (lRulePrecedenceString[i] == "Clustering")
            lRulePrecedence.append("CPAT");
        if (lRulePrecedenceString[i] == "Reticle")
            lRulePrecedence.append("RPAT");

    }
    for (int i=0; i<lRulePrecedence.size(); ++i)
    {
        lSumStreamFile << " " << lRulePrecedence[i];
    }
    lSumStreamFile << endl;

    // Write univariate tests
    // #define TREND_DEF 1 DPAT_IBB_SBY_V130_B150_WGND
    QMap<QString, CPatDefinition*>::const_iterator lPatTest = enabledPatTests.constBegin();
    int lTestIndex(0);
    while (lPatTest != enabledPatTests.end())
    {
        lSumStreamFile << "#define TREND_DEF " << lTestIndex << " " << lPatTest.key()
                       << "_" << lPatTest.value()->m_strTestName << endl;
        ++lTestIndex;
        ++lPatTest;
    }

//    // Write the rest of rules
//    // #define TREND_DEF 11 MVPAT_Rule1
//    QMap<QString, QString>::const_iterator lPatTest = lEnabledPatTests.constBegin();
//    int lTestIndex(0);
//    while (lPatTest != lEnabledPatTests.end())
//    {
//        lSumStreamFile << "#define TREND_DEF " << lTestIndex << " " << lPatTest.key()
//                       << "_" << lPatTest.value()->m_strTestName << endl;
//        ++lTestIndex;
//        ++lPatTest;
//    }




    // Write test results
    // ^ 8698926:22:P13:11:12 H1S22 .:* 3 0 0 0
    // ^ 8698926:22:P15:11:12 H1S23 DPAT:DPAT 3 0 0 1
    QString lDieString;
//    int lFailPatAlgo = 0;
    qint16 lX, lY;
    for (int lIndex=0; lIndex<diesList.size(); ++lIndex)
    {
        lPatTest = enabledPatTests.constBegin();
        GetDieString(diesList[lIndex], binsList[lIndex], lDieString);
        lSumStreamFile << "^ " << lDieString << " " << enabledPatTests.size();

        // We have to test first of all if the die is fail, all PAT' tests will not be executed
        if (!binsList[lIndex].contains(".")
                && !binsList[lIndex].contains("*"))
        {
            for (int i=0; i<enabledPatTests.size(); ++i)
            {
                lSumStreamFile << " X";
            }
        }
        else
        {
            GetXYFromMicronDieString(diesList[lIndex], lX, lY);
            while (lPatTest != enabledPatTests.end())
            {
                /*int lPATAlgo = lPatInfo->GetPPATAlgoTested(lX, lY,
                                                           lPatTest.value()->m_lTestNumber,
                                                           lPatTest.value()->mPinIndex,
                                                           lPatTest.value()->m_strTestName,
                                                           lFailPatAlgo);
                // Die is tested under SPAT, DPAT or NRR
                if (lPATAlgo & GEX_TPAT_BINTYPE_STATICFAIL
                    || lPATAlgo & GEX_TPAT_BINTYPE_DYNAMICFAIL
                    || lPATAlgo & GEX_TPAT_BINTYPE_NNR)
                {
                    if(lFailPatAlgo != 0)
                        lSumStreamFile  << " 1";
                    else
                        lSumStreamFile  << " 0";
                }
                else
                {
                    lSumStreamFile  << " X";
                }*/
                if (lPatTest.value()
                    && lPatInfo->IsPATTestExecutedWithPass(lX, lY,
                                                           lPatTest.value()->m_lTestNumber,
                                                           lPatTest.value()->mPinIndex,
                                                           lPatTest.value()->m_strTestName))
                {
                    lSumStreamFile  << " 0";
                }
                else
                {
                    lSumStreamFile << " 1";
                    // The rest of tested haven't been tested
                    ++lPatTest;
                    while (lPatTest != enabledPatTests.end())
                    {
                        lSumStreamFile << " X";
                        ++lPatTest;
                    }
                }
                if (lPatTest != enabledPatTests.end())
                    ++lPatTest;
            }
        }
        lSumStreamFile << endl;
    }

    lSumStreamFile << endl;

    // Write part results
    // + 8698926:22:P04:01:12 H1S0 I:I 9 X:X X:X X:X X:X X:X X:X X:X X:X X:X
    // + 8698926:22:P03:01:12 H1S1 I:I 9 X:X X:X X:X X:X X:X X:X X:X X:X X:X
    for (int lIndex=0; lIndex<diesList.size(); ++lIndex)
    {
        GetDieString(diesList[lIndex], binsList[lIndex], lDieString);
        // We have 9 differents pat algorithms
        lSumStreamFile << "+ " << lDieString << " 9 ";

        if (!binsList[lIndex].contains(".")
                && !binsList[lIndex].contains("*"))
        {
            for (int i=0; i<9; ++i)
            {
                lSumStreamFile << " X:X";
            }
        }
        else
        {
            GetXYFromMicronDieString(diesList[lIndex], lX, lY);

            // The order of execution of Pat tests is ZPAT SPAT DPAT MVPAT Precedence[from the recipe]
            lSumStreamFile << " X:X";  // for ZPAT
            int lPatBin;
            if (lPatInfo->isDieOutlier(lX, lY, lPatBin))
            {
                int lOutlierType = lPatInfo->getOutlierType(lPatBin);
                switch(lOutlierType)
                {
                case CPatInfo::Outlier_NNR:
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    if (enabledPatTests.contains("DPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " X:X";   // MVPAT
                    for (int lPrecIndex=0; lPrecIndex<lRulePrecedence.size(); ++lPrecIndex)
                    {
                        if (lRulePrecedence[lPrecIndex] == "NPAT")
                        {
                            lSumStreamFile << " F:X";
                        }
                        else
                            lSumStreamFile << " X:X";
                    }
                    break;
                case CPatInfo::Outlier_IDDQ_Delta:
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    if (enabledPatTests.contains("DPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " X:X";   // MVPAT
                    for (int lPrecIndex=0; lPrecIndex<lRulePrecedence.size(); ++lPrecIndex)
                    {
                        if (lRulePrecedence[lPrecIndex] == "IPAT")
                        {
                            lSumStreamFile << " F:X";
                        }
                        else
                            lSumStreamFile << " X:X";
                    }
                    break;
                case CPatInfo::Outlier_GDBN:
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    if (enabledPatTests.contains("DPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " X:X";   // MVPAT
                    for (int lPrecIndex=0; lPrecIndex<lRulePrecedence.size(); ++lPrecIndex)
                    {
                        if (lRulePrecedence[lPrecIndex] == "IPAT")
                        {
                            lSumStreamFile << " F:X";
                        }
                        else
                            lSumStreamFile << " X:X";
                    }
                    break;
                case CPatInfo::Outlier_Reticle:
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    if (enabledPatTests.contains("DPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " X:X";   // MVPAT
                    for (int lPrecIndex=0; lPrecIndex<lRulePrecedence.size(); ++lPrecIndex)
                    {
                        if (lRulePrecedence[lPrecIndex] == "RPAT")
                        {
                            lSumStreamFile << " F:X";
                        }
                        else
                            lSumStreamFile << " X:X";
                    }
                    break;
                case CPatInfo::Outlier_Clustering:
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    if (enabledPatTests.contains("DPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " X:X";   // MVPAT
                    for (int lPrecIndex=0; lPrecIndex<lRulePrecedence.size(); ++lPrecIndex)
                    {
                        if (lRulePrecedence[lPrecIndex] == "CPAT")
                        {
                            lSumStreamFile << " F:X";
                        }
                        else
                            lSumStreamFile << " X:X";
                    }
                    break;
                case CPatInfo::Outlier_ZPAT:
                    for (int lIndex=0; lIndex<8; ++lIndex)
                        lSumStreamFile << " X:X";
                    break;
                case CPatInfo::Outlier_MVPAT:
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    if (enabledPatTests.contains("DPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " F:X";
                    for (int lIndex=0; lIndex<5; ++lIndex)
                        lSumStreamFile << " X:X";
                    break;
                default: // DPAT failure
                    if (enabledPatTests.contains("SPAT"))
                        lSumStreamFile << " P:X";
                    else
                        lSumStreamFile << " X:X";
                    lSumStreamFile << " F:X";
                    for (int lIndex=0; lIndex<6; ++lIndex)
                        lSumStreamFile << " X:X";
                    break;
                }
            }
            else
            {
                if (enabledPatTests.contains("SPAT"))
                    lSumStreamFile << " P:X";
                else
                    lSumStreamFile << " X:X";
                if (enabledPatTests.contains("DPAT"))
                    lSumStreamFile << " P:X";
                else
                    lSumStreamFile << " X:X";
                if (enabledPatRules.contains("MVPAT"))
                    lSumStreamFile << " P:X";
                else
                    lSumStreamFile << " X:X";
                for (int lPrec=0; lPrec<lRulePrecedence.size(); ++lPrec)
                {
                    if (lRulePrecedence[lPrec] == "NPAT")
                    {
                        if (enabledPatTests.contains("NPAT"))
                            lSumStreamFile << " P:X";
                        else
                            lSumStreamFile << " X:X";
                    }
                    else
                    {
                        if (enabledPatRules.contains(lRulePrecedence[lPrec]))
                            lSumStreamFile << " P:X";
                        else
                            lSumStreamFile << " X:X";
                    }
                }
            }
        }
        lSumStreamFile << endl;
    }

    lSumStreamFile << "$$_END_PROBE_LOG" << endl;

    return true;
}



////////////////////////////////////////////////////////////////////////////////
bool WaferExport::GetOutliersPerTest(QHash<QString, int>& outliersPerTest)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty pat info ");
        return false;
    }

    // Get the test results from the list of ouliers
    for(int lOutlierIndex=0; lOutlierIndex<lPatInfo->m_lstOutlierParts.size(); ++lOutlierIndex)
    {
        QList<CPatFailingTest> lOutlierList = lPatInfo->m_lstOutlierParts[lOutlierIndex]->cOutlierList;
        for (int lPartIndex=0; lPartIndex<lOutlierList.size(); ++lPartIndex)
        {
            QString lTestName = lOutlierList[lPartIndex].mTestName;
            if (outliersPerTest.contains(lTestName))
            {
                int lVal = outliersPerTest.value(lTestName);
                ++lVal;
                outliersPerTest.remove(lTestName);
                outliersPerTest.insert(lTestName, lVal);
            }
            else
            {
                outliersPerTest.insert(lTestName, 1);
            }
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////////
bool WaferExport::GetDiesListFromTrendLog(QString lTrendLog, QStringList& diesList, QStringList& binsList)
{
    // Open header file
    QString lLine("");
    diesList.clear();
    QFile lFile(lTrendLog);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to open header file %1")
              .arg(lTrendLog).toLatin1().constData());
        return false;
    }

    // Assign file I/O stream
    QTextStream lTrendLogStreamFile(&lFile);

    while (!lTrendLogStreamFile.atEnd())
    {
        lLine = lTrendLogStreamFile.readLine().trimmed();
        if (!lLine.isEmpty()
                && !lLine.startsWith("^"))
        {
            GSLOG(SYSLOG_SEV_ERROR, "Uncompatible line which doesn't start with \"^\"");
            return false;
        }
        // add this section of the line "^ 8698926:22:P04:01:12 H1S0"
        diesList.append(lLine.section(" ", 1, 2));
        binsList.append(lLine.section(" ", 3, 3));
    }
    return true;
}

bool WaferExport::GetNumberOutliersPerPatAlgo(QList<int>& patOutlierPerAlgo)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty pat info ");
        return false;
    }

    int lPatBin;
    for (int i=0; i<6; ++i)
    {
        patOutlierPerAlgo.append(0);
    }

    // Get the test results from the list of ouliers
    for(int lOutlierIndex=0; lOutlierIndex<lPatInfo->m_lstOutlierParts.size(); ++lOutlierIndex)
    {
        if (lPatInfo->isDieOutlier(lPatInfo->m_lstOutlierParts[lOutlierIndex]->iDieX,
                                   lPatInfo->m_lstOutlierParts[lOutlierIndex]->iDieY,
                                   lPatBin))
        {
            int lOutlierType = lPatInfo->getOutlierType(lPatBin);
            switch(lOutlierType)
            {
            case CPatInfo::Outlier_NNR:
                patOutlierPerAlgo[0] = patOutlierPerAlgo[0] + 1;
                break;
            case CPatInfo::Outlier_IDDQ_Delta:
                patOutlierPerAlgo[1] = patOutlierPerAlgo[1] + 1;
                break;
            case CPatInfo::Outlier_GDBN:
                patOutlierPerAlgo[2] = patOutlierPerAlgo[2] + 1;
                break;
            case CPatInfo::Outlier_Reticle:
               patOutlierPerAlgo[3] = patOutlierPerAlgo[3] + 1;
                break;
            case CPatInfo::Outlier_Clustering:
                patOutlierPerAlgo[4] = patOutlierPerAlgo[4] + 1;
                break;
            case CPatInfo::Outlier_ZPAT:
                patOutlierPerAlgo[5] = patOutlierPerAlgo[5] + 1;
                break;
            default:
                break;
            }
        }
    }
    return true;
}


bool WaferExport::GetDieString(const QString dieElement, const QString originalBin, QString& dieString)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if(!lPatInfo)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Empty pat info ");
        return false;
    }

    dieString = dieElement + " ";
    qint16 lX, lY;
    GetXYFromMicronDieString(dieElement.section(" ", 0, 0), lX, lY);

    int lPatBin;
    if (lPatInfo->isDieOutlier(lX, lY, lPatBin))
    {
        int lOutlierType = lPatInfo->getOutlierType(lPatBin);
        QString lOutlierTypeString;
        switch(lOutlierType)
        {
        case CPatInfo::Outlier_NNR:
            lOutlierTypeString = "NPAT";
            break;
        case CPatInfo::Outlier_IDDQ_Delta:
            lOutlierTypeString = "IPAT";
            break;
        case CPatInfo::Outlier_GDBN:
            lOutlierTypeString = "GPAT";
            break;
        case CPatInfo::Outlier_Reticle:
            lOutlierTypeString = "RPAT";
            break;
        case CPatInfo::Outlier_Clustering:
            lOutlierTypeString = "CPAT";
            break;
        case CPatInfo::Outlier_ZPAT:
            lOutlierTypeString = "ZPAT";
            break;
        case CPatInfo::Outlier_MVPAT:
            lOutlierTypeString = "MVPAT";
            break;
        default:
            lOutlierTypeString = "DPAT";
            break;
        }

        dieString += lOutlierTypeString + ":" + lOutlierTypeString;
    }
    else
    {
        dieString += originalBin;
    }
    return true;
}

bool WaferExport::GetXYFromMicronDieString(QString dieString, qint16& lX, qint16& lY)
{
    QString lXString = dieString.section(":", 2, 2);
    if(lXString.contains("N"))
        lX = 0 - (lXString.remove("N")).toDouble();
    else
        lX = (lXString.remove("P")).toDouble();
    lY = dieString.section(":", 3, 3).toDouble();

    return true;
}

bool WaferExport::IsSupportedOutputFormat(const QString &lStringFormat, int &lFormat)
{
    if (lStringFormat.startsWith("tsmc", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputTSMCink;
        return true;
    }
    else if (lStringFormat.startsWith("g85", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputSemiG85inkAscii;
        return true;
    }
    else if (lStringFormat.startsWith("xml_g85", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputSemiG85inkXml;
        return true;
    }
    else if (lStringFormat.startsWith("stif", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputSTIF;
        return true;
    }
    else if (lStringFormat.startsWith("semi_e142", Qt::CaseInsensitive))
    {
        if(lStringFormat.endsWith("integer2", Qt::CaseInsensitive))
            lFormat = WaferExport::outputSemiE142inkInteger2;
        else
            lFormat = WaferExport::outputSemiE142ink;
        return true;
    }
    else if (lStringFormat.startsWith("sinf", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputKLA_INF;
        return true;
    }
    else if (lStringFormat.startsWith("kla_sinf", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputKLA_INF;
        return true;
    }
    else if (lStringFormat.startsWith("telp8", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputTELP8;
        return true;
    }
    else if (lStringFormat.startsWith("olympus_al2000", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputOlympusAL2000;
        return true;
    }
    else if (lStringFormat.startsWith("sum_micron", Qt::CaseInsensitive))
    {
        lFormat = WaferExport::outputSumMicron;
        return true;
    }

    return false;
}

} // end namespace Gex
} // end namespace GS
