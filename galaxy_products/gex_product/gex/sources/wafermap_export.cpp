#include <QFileDialog>

#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_sysutils.h>
#include <gqtl_log.h>
#include "browser_dialog.h"
#include "plugin_base.h"

#ifdef GCORE15334
    #include "pat_engine.h"
#endif

#include "cbinning.h"
#include "wafermap_export.h"

extern GexMainwindow *	pGexMainWindow;	// main.cpp
extern CGexReport *		gexReport;				// report_build.cpp: Handle to report class
extern GexScriptEngine*	pGexScriptEngine;


int	WafermapExport::CreateWafermapOutput(QString &strOutputWafermapPath,
                                         QString &strWaferFileFullName,
                                         const QString &strOutputWafermapFormat,
                                         int iBinType,int iNotch,int iPosX,int iPosY,bool bRotateWafer,
                                         QString &strErrorMessage,
                                         QString strCustomer/*=""*/,QString strSupplier/*=""*/)
{
    QString lOutputFotmat = strOutputWafermapFormat.toLower().trimmed();

    if(lOutputFotmat.isEmpty() == true)
        return NoError;
    if(lOutputFotmat.indexOf("disabled") >= 0)
        return NoError;
    if(lOutputFotmat.indexOf("none") >= 0)
        return NoError;

    // Check if must output SOFT-BIN Map or HARD-BIN Map
    switch(iBinType)
    {
        case GEX_WAFMAP_EXPORT_SOFTBIN: // Export Soft-Bin wafermap
            break;
        case GEX_WAFMAP_EXPORT_HARDBIN: // Export Soft-Bin wafermap
            break;
        case GEX_WAFMAP_EXPORT_CURRENTBIN: // Export current wafermap strcture (whatever it is: HardBin or SoftBin)
            break;
    }

    // Build output file path
    if(lOutputFotmat.startsWith("tsmc", Qt::CaseInsensitive))
    {
        return CreateWafermapOutput_TSMC(strOutputWafermapPath,strWaferFileFullName,
                                         iNotch,iPosX,iPosY,bRotateWafer,strErrorMessage);
    }

    if(lOutputFotmat.startsWith("g85", Qt::CaseInsensitive))
    {
        // G85 for Semi85 inkless assembly map format / Amkor
        return CreateWafermapOutput_G85(strOutputWafermapPath,strWaferFileFullName,
                                        iNotch,iPosX,iPosY,bRotateWafer,strErrorMessage,strCustomer,strSupplier);
    }

    if(lOutputFotmat.startsWith("xml_g85", Qt::CaseInsensitive))
    {
        // XML G85 for Semi85 inkless assembly map format / Amkor
        return CreateWafermapOutput_G85(strOutputWafermapPath,strWaferFileFullName,
                                        iNotch,iPosX,iPosY,bRotateWafer,strErrorMessage,strCustomer,strSupplier,true);
    }

    if(lOutputFotmat.startsWith("stif", Qt::CaseInsensitive))
    {
        // STMicroelectronics inkless format
        return CreateWafermapOutput_STIF(strOutputWafermapPath,strWaferFileFullName,
                                         iNotch,iPosX,iPosY,bRotateWafer,strErrorMessage);
    }

    if(lOutputFotmat.startsWith("semi_e142", Qt::CaseInsensitive))
    {
        bool isInteger2 = lOutputFotmat.endsWith("integer2", Qt::CaseInsensitive);
        // SEMI E142 XML inkless assembly map format
        return CreateWafermapOutput_SEMI142(isInteger2, strOutputWafermapPath,strWaferFileFullName,
                                            iNotch,iPosX,iPosY,bRotateWafer,strErrorMessage,strCustomer,strSupplier);
    }

    if(lOutputFotmat.startsWith("sinf", Qt::CaseInsensitive))
    {
        // Clear info related to KLA optional source!
        m_cSinfInfo.strNewWafermap = "";	// Clear wafermap created from KLA/INF file

        m_cSinfInfo.iRefPX = -32768;		// Clear DieX offset read from KLA/INF file
        m_cSinfInfo.iRefPY = -32768;		// Clear DieY offset read from KLA/INF file

        // Simple INF KLA/INF format
        return CreateWafermapOutput_SINF(strOutputWafermapPath,strWaferFileFullName,iNotch,
                                         iPosX,iPosY,bRotateWafer,strErrorMessage,m_cSinfInfo);
    }

    if(lOutputFotmat.startsWith("kla_sinf", Qt::CaseInsensitive))
    {
        // Simple INF KLA/INF format, but using same offsets in in KLA/INF file!
        return CreateWafermapOutput_SINF(strOutputWafermapPath,strWaferFileFullName,iNotch,
                                         iPosX,iPosY,bRotateWafer,strErrorMessage,m_cSinfInfo);
    }

    if(lOutputFotmat.startsWith("telp8", Qt::CaseInsensitive))
    {
        // TEL-P8 for TEL P8 wafer prober binary format
        return CreateWafermapOutput_TELP8(strOutputWafermapPath,strWaferFileFullName,
                                          iNotch,iPosX,iPosY,bRotateWafer,strErrorMessage);
    }

    return NoError;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: TSMC ASCII inkless file
// Note: return error code
// Format:
// TSMC
// TMN178
// A45234-03
// A4523403.TSM
// ...............112221...............
// ............111111111111............
// etc...
// Note: The wafer notch MUST always be at located at '6' oclock (DOWN)
///////////////////////////////////////////////////////////
int
WafermapExport::CreateWafermapOutput_TSMC(QString& strOutputWafermapPath,
                                          QString& strWaferFileFullName,
                                          int iNotch,
                                          int iPosX,
                                          int iPosY,
                                          bool bRotateWafer,
                                          QString& /*strErrorMessage*/,
                                          bool bSaveAs /*= false*/,
                                          int iGroupID /*= 0*/,
                                          int iFileID /*= 0*/)
{
    int iWaferSizeX,iWaferSizeY;
    CGexGroupOfFiles *  pGroup  = NULL;
    CGexFileInGroup *   pFile   = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool bMergedWafer=false;
    if(iFileID < 0)
    {
        bMergedWafer=true;
        iWaferSizeX = pGroup->cStackedWaferMapData.SizeX;
        iWaferSizeY = pGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        iFileID = 0;

        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile =	NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
        iWaferSizeX = pFile->getWaferMapData().SizeX;
        iWaferSizeY = pFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Build file name: <path>/<lot>-<WaferID>.tsm
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode
        strWaferFileName = pFile->getMirDatas().szLot;	// LotID
        strWaferFileName = pFile->getMirDatas().szLot;
        strWaferFileName += "_";
        strWaferFileName += pFile->getMirDatas().szSubLot;		// Sublot
        strWaferFileName += "_";
        strWaferFileName += lWafer.szWaferID;	// WaferID
    }
    strWaferFileFullName += ".TSM";	// File extension.

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                            strWaferFileFullName, "TSMC inkless format (*.tsm)");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Header section
    hWafermapFile << "TSMC" << endl;						// Header line
    hWafermapFile << pFile->getMirDatas().szPartType << endl;		// Product name
    hWafermapFile << lWafer.szWaferID << endl;              // WaferID
    hWafermapFile << strWaferFileName << endl;				// Wafermap file name created (eg: A4523403.TSM)

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap until Notch is at 6hour (Down)...unless custom direction is set
    if(iNotch <= 0)
        iNotch = 6;	// Default notch direction is DOWN
    if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }
    iWaferSizeX = lWafer.SizeX;
    iWaferSizeY = lWafer.SizeY;

    // Wafermap
    int	iLine,iCol,iBinCode;
    int	iStartCol,iEndCol,iColStep;
    int	iStartLine,iEndLine,iLineStep;
    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        iStartCol = 0;
        iEndCol = iWaferSizeX;
        iColStep = 1;
    }
    else
    {
        // X direction = 'L' (left)
        iStartCol = iWaferSizeX-1;
        iEndCol = -1;
        iColStep = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        iStartLine = 0;
        iEndLine = iWaferSizeY;
        iLineStep = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        iStartLine = iWaferSizeY-1;
        iEndLine = -1;
        iLineStep = -1;
    }

    for(iLine = iStartLine; iLine != iEndLine;)
    {
        // Processing a wafer line.
        for(iCol = iStartCol; iCol != iEndCol;)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(bMergedWafer)
                iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*iWaferSizeX))].ldCount;
            else
                iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();
            switch(iBinCode)
            {
                case GEX_WAFMAP_EMPTY_CELL:	// -1: Die not tested
                    hWafermapFile << ".";
                    break;

                default:	// Check if PASS or FAIL die
                    if(bMergedWafer)
                    {
                        // Merged wafer
                        if(iBinCode != pGroup->cStackedWaferMapData.iTotalWafermaps)
                            hWafermapFile << "1";	// All dies matching
                        else
                            hWafermapFile << "X";	// Not 100% matching...
                    }
                    else
                    {
                        // Single wafer
#if 0
                        if(pGroup->isPassingBin(true,iBinCode))
                            hWafermapFile << "1";
                        else
                            hWafermapFile << "X";
#else
                        if(pGroup->isPassingBin(true,iBinCode))
                        {
                            if(iBinCode >= 0 && iBinCode <= 9)
                                hWafermapFile << QString::number(iBinCode);
                            else
                                hWafermapFile << "1";
                        }
                        else
                            hWafermapFile << "X";
#endif
                    }
                    break;
            }

            // Next column
            iCol += iColStep;
        }

        // Insert line break
        hWafermapFile << endl;

        // Next line
        iLine += iLineStep;
    }

    // Update/Create TSMC summary file.
    file.close();

    return NoError;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: Laurier Die Sort 1D
// Note: return error code
///////////////////////////////////////////////////////////
int
WafermapExport::
CreateWafermapOutput_LaurierDieSort1D(QString& strOutputWafermapPath,
                                      QString& strWaferFileFullName,
                                      int iNotch,
                                      int iPosX,
                                      int iPosY,
                                      bool bRotateWafer,
                                      QString& /*strErrorMessage*/,
                                      bool bSaveAs /*= false*/,
                                      int iGroupID /*= 0*/,
                                      int iFileID /*= 0*/)
{
    int iWaferSizeX,iWaferSizeY;
    CGexGroupOfFiles *  pGroup  = NULL;
    CGexFileInGroup *   pFile   = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool bMergedWafer=false;
    if(iFileID < 0)
    {
        bMergedWafer=true;
        iWaferSizeX = pGroup->cStackedWaferMapData.SizeX;
        iWaferSizeY = pGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        iFileID = 0;
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
        iWaferSizeX = pFile->getWaferMapData().SizeX;
        iWaferSizeY = pFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Build file name: <path>/<lot>_<sublot>_<WaferID>
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode
        strWaferFileName = pFile->getMirDatas().szLot;
        strWaferFileName += "_";
        strWaferFileName += pFile->getMirDatas().szSubLot;	// Sublot
        strWaferFileName += "_";
        strWaferFileName += lWafer.szWaferID;	// WaferID
    }
    strWaferFileName += ".col";

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                            strWaferFileFullName, "Laurier Die Sort 1D format (*.col)");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate WaferMap, and set notch location
    //FIXME: not used ?
    //int iFlatNotchDirection=0; // 0=down,90=left,180=top,270=right

    // Rotate wafermap, unless default should be used for the notch
    if(iNotch <= 0)
        iNotch = lWafer.GetWaferNotch();
    else if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }
    iWaferSizeX = lWafer.SizeX;
    iWaferSizeY = lWafer.SizeY;
    // Compute notch direction
    //FIXME: not used ?
    /*switch(iNotch)
  {
    case 12:	// Up
      iFlatNotchDirection = 180;
      break;
    case 6:	// Down
      iFlatNotchDirection = 0;
      break;
    case 3:	// Right
      iFlatNotchDirection = 270;
      break;
    case 9:	// LEFT
      iFlatNotchDirection = 90;
      break;
  }*/

    // Version
    hWafermapFile << "# Version = " << pFile->getMirDatas().szJobRev << endl;

    // Spec Revision
    hWafermapFile << "# Spec Rev = " << "1" << endl;

    // Eng Fail Rev
    hWafermapFile << "# Eng Fail Rev = " << "4" << endl;

    // Product ID
    hWafermapFile << "# productid = " << pFile->getMirDatas().szPartType << endl;

    // File Creation Date
    hWafermapFile << "# date = " << QDate::currentDate().toString("M/d/yyyy") << endl;

    // Die Size X
    hWafermapFile << "# Die Size X = " << "2554.0" << endl;

    // Die Size Y
    hWafermapFile << "# Die Size Y = " << "2554.0" << endl;

    // Wafermap
    QString	strDie;

    // Wafermap
    int	iLine,iCol,iBinCode;
    int	iStartCol,iEndCol,iColStep;
    int	iStartLine,iEndLine,iLineStep;
    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        iStartCol = 0;
        iEndCol = iWaferSizeX;
        iColStep = 1;
    }
    else
    {
        // X direction = 'L' (left)
        iStartCol = iWaferSizeX-1;
        iEndCol = -1;
        iColStep = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        iStartLine = 0;
        iEndLine = iWaferSizeY;
        iLineStep = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        iStartLine = iWaferSizeY-1;
        iEndLine = -1;
        iLineStep = -1;
    }

    for(iCol = iStartCol; iCol != iEndCol;)
    {
        for(iLine = iStartLine; iLine != iEndLine;)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(bMergedWafer)
            {
                iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*iWaferSizeX))].lStatus;

                if (iBinCode == GEX_WAFMAP_PASS_CELL)
                    iBinCode = 1;
                else if (iBinCode == GEX_WAFMAP_FAIL_CELL)
                    iBinCode = 11;
            }
            else
                iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();

            if(iBinCode != GEX_WAFMAP_EMPTY_CELL)
            {
                // X-Coord
                hWafermapFile << qSetFieldWidth(11) << QString::number(iCol + lWafer.iLowDieX);

                // Y-Coord
                hWafermapFile << qSetFieldWidth(11) << QString::number(iLine + lWafer.iLowDieY);

                // HBin #
                hWafermapFile << qSetFieldWidth(11) << QString::number(iBinCode);

                // Insert line break
                hWafermapFile << endl;
            }

            // Next line
            iLine += iLineStep;
        }

        // Next column
        iCol += iColStep;
    }

    file.close();
    return NoError;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: HTML code (for easy export to MS-Office)
// Note: return error code
// Note: The wafer notch MUST always be at located at '6' oclock (DOWN)
///////////////////////////////////////////////////////////
int
WafermapExport::CreateWafermapOutput_HTML(QString& strOutputWafermapPath,
                                          QString& strWaferFileFullName,
                                          int iNotch,
                                          int iPosX,
                                          int iPosY,
                                          bool bRotateWafer,
                                          QString& /*strErrorMessage*/,
                                          bool bSaveAs /*= false*/,
                                          int iGroupID /*= 0*/,
                                          int iFileID /*= 0*/)
{
    int	iWaferSizeX;
    int	iWaferSizeY;

    // Pointer to Data file so to extract  Wafermap Bin results.
    bool	bDumpAllGroups = false;
    if(iGroupID < 0)
    {
        iGroupID = 0;			// Start dump on first wafermap in firstgroup
        bDumpAllGroups = true;	// Will dump all groups!
    }

    CGexGroupOfFiles *  pGroup  = NULL;
    CGexFileInGroup *   pFile   = NULL;
    CWaferMap           lWafer;

    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if dump one specific wafermap, or ALL wafermaps
    bool	bDumpAllWafers = false;
    if(iFileID < 0)
    {
        iFileID = 0;	// Start dump on first wafermap
        bDumpAllWafers = true;	// Will dump them all!
    }

    if (pGroup->pFilesList.isEmpty() || (iFileID >= pGroup->pFilesList.size()))
        pFile = NULL;
    else
        pFile = pGroup->pFilesList.at(iFileID);
    if(pFile == NULL)
        return NoError;	// Should never happen!

    // Build file name: <path>/<product>_<lot>_<sublot>_<WaferID>.tsm
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode;
        // Product ID
        if (*pFile->getMirDatas().szPartType)
        {
            strWaferFileName = pFile->getMirDatas().szPartType;
            strWaferFileName += "_";
        }

        // LotID
        if (*pFile->getMirDatas().szLot)
        {
            strWaferFileName += pFile->getMirDatas().szLot;
            strWaferFileName += "_";
        }

        // Sublot
        if (*pFile->getMirDatas().szSubLot)
        {
            strWaferFileName += pFile->getMirDatas().szSubLot;
            strWaferFileName += "_";
        }

        // WaferID
        if (*pFile->getWaferMapData().szWaferID)
            strWaferFileName += pFile->getWaferMapData().szWaferID;
    }
    // File extension
    strWaferFileName += ".htm";

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                            strWaferFileFullName, "HTML format (*.htm)");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Header section
    hWafermapFile << "<html>" << endl;						// Header line
    hWafermapFile << "<head>" << endl;						// Header line
    hWafermapFile << "</head>" << endl;						// Header line
    hWafermapFile << "<body>" << endl;						// Header line
    hWafermapFile << "<h1 align=\"left\"><font color=\"#006699\">Welcome to Quantix Examinator HTML Wafermap!</font></h1><br>" << endl;

    QString	strNotchPosition;
    while(pGroup)
    {
        if (pGroup->pFilesList.isEmpty() || (iFileID >= pGroup->pFilesList.size()))
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);

        while(pFile)
        {
            // Create a local copy of the wafermap
            lWafer = pFile->getWaferMapData();

            // Check if PosX/PosY have to be overloaded
            if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
                lWafer.SetPosXDirection(false);
            else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
                lWafer.SetPosXDirection(true);
            else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
                    lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
            {
                if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
                    lWafer.SetPosXDirection(false);
                else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
                    lWafer.SetPosXDirection(true);
            }
            if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
                lWafer.SetPosYDirection(false);
            else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
                lWafer.SetPosYDirection(true);
            else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
                    lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
            {
                if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
                    lWafer.SetPosYDirection(false);
                else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
                    lWafer.SetPosYDirection(true);
            }

            // Rotate wafermap, unless default should be used for the notch
            if(iNotch <= 0)
                iNotch = lWafer.GetWaferNotch();
            else if(bRotateWafer)
            {
                while(lWafer.GetWaferNotch() != iNotch)
                    lWafer.RotateWafer();
            }
            iWaferSizeX = lWafer.SizeX;
            iWaferSizeY = lWafer.SizeY;
            switch(iNotch)
            {
                case 12:
                    strNotchPosition = "Up";
                    break;
                case 3:
                    strNotchPosition = "Right";
                    break;
                case 6:
                    strNotchPosition = "Down";
                    break;
                case 9:
                    strNotchPosition = "Left";
                    break;
            }

            hWafermapFile << "<p>Product: <b>" << pFile->getMirDatas().szPartType << "</b><br>" << endl;		// Product name
            if(bDumpAllGroups && gexReport->getGroupsList().count() > 1)
                hWafermapFile << "Dataset: <b>" << pGroup->strGroupName << "</b><br>" << endl;	// Group name (in case multiple groups dumped)
            hWafermapFile << "Wafer ID: <b>" << lWafer.szWaferID << "</b><br>" << endl;	// WaferID
            hWafermapFile << "Notch position: <b>" << strNotchPosition << "</b></p>" << endl;	// Notch
            hWafermapFile << "<table border=\"0\" width=\"98%%\">" << endl;

            // Wafermap
            int	iLine,iCol,iBinCode;
            int	iStartCol,iEndCol,iColStep;
            int	iStartLine,iEndLine,iLineStep;
            // Check for X direction
            if(lWafer.GetPosXDirection() == true)
            {
                // X direction = 'R' (right)
                iStartCol = 0;
                iEndCol = iWaferSizeX;
                iColStep = 1;
            }
            else
            {
                // X direction = 'L' (left)
                iStartCol = iWaferSizeX-1;
                iEndCol = -1;
                iColStep = -1;
            }
            // Check for Y direction
            if(lWafer.GetPosYDirection() == true)
            {
                // Y direction = 'D' (down)
                iStartLine = 0;
                iEndLine = iWaferSizeY;
                iLineStep = 1;
            }
            else
            {
                // Y direction = 'U' (up)
                iStartLine = iWaferSizeY-1;
                iEndLine = -1;
                iLineStep = -1;
            }

            for(iLine = iStartLine; iLine != iEndLine;)
            {
                // Open table line
                hWafermapFile << "<tr>" << endl;

                // Processing a wafer line.
                for(iCol = iStartCol; iCol != iEndCol;)
                {
                    // Get PAT-Man binning at location iRow,iCol.
                    iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();
                    switch(iBinCode)
                    {
                        case GEX_WAFMAP_EMPTY_CELL:	// -1: Die not tested
                            hWafermapFile << "<td>&nbsp;</td>";
                            break;

                        default:	// FAILING die
                            if(pGroup->isPassingBin(true,iBinCode))
                                hWafermapFile << "<td bgcolor=\"#00FF00\">";
                            else
                                hWafermapFile << "<td bgcolor=\"#FF0000\">";
                            hWafermapFile << QString::number(iBinCode) << "</td>";
                            break;
                    }

                    // Next column
                    iCol += iColStep;
                }

                // End table line
                hWafermapFile << "</tr>" << endl;

                // Next line
                iLine += iLineStep;
            }

            // Close table
            hWafermapFile << "</table><p></p>" << endl;

            // Go to next wafermap (unless we have to only dump one)
            if(bDumpAllWafers)
                pFile   = (++iFileID >= pGroup->pFilesList.size()) ? NULL : pGroup->pFilesList.at(iFileID);
            else
                pFile = NULL;	// Force to exit now!
        };
        // Reset fileID.
        iFileID = 0;
        if(bDumpAllGroups)
            pGroup = (++iGroupID >= gexReport->getGroupsList().size()) ? NULL : gexReport->getGroupsList().at(iGroupID);
        else
            pGroup = NULL;	// Force to exit now!
    };

    // close HTML page
    hWafermapFile << "</body>" << endl;
    hWafermapFile << "</html>" << endl;

    file.close();
    return NoError;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: PNG image
// Note: return error code
// Note: The wafer notch MUST always be at located at '6' oclock (DOWN)
///////////////////////////////////////////////////////////
int
WafermapExport::
CreateWafermapOutput_IMAGE_PNG(QString& strOutputWafermapPath,
                               QString& strWaferFileFullName,
                               int iNotch,
                               int iPosX,
                               int iPosY,
                               bool bRotateWafer,
                               QString& /*strErrorMessage*/,
                               CGexReport::wafermapMode eWafermapMode,
                               bool bSaveAs /*= false*/,
                               int iGroupID /*= 0*/,
                               int iFileID /*= 0*/)
{
    CGexFileInGroup *   pFile   = NULL;
    CGexGroupOfFiles *  pGroup  = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    if(iFileID < 0)
    {
        // Get handle to first file in group
        iFileID = 0;
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!

        // Force to rebuild the stacked wafermap
        if (eWafermapMode == CGexReport::stackedWafermap)
            pGroup->BuildStackedWaferMap(&ReportOptions);
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Build file name: <path>/<product>_<lot>_<sublot>_<WaferID>.tsm
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode;
        // Product ID
        if (*pFile->getMirDatas().szPartType)
        {
            strWaferFileName = pFile->getMirDatas().szPartType;
            strWaferFileName += "_";
        }

        // LotID
        if (*pFile->getMirDatas().szLot)
        {
            strWaferFileName += pFile->getMirDatas().szLot;
            strWaferFileName += "_";
        }

        // Sublot
        if (*pFile->getMirDatas().szSubLot)
        {
            strWaferFileName += pFile->getMirDatas().szSubLot;
            strWaferFileName += "_";
        }

        // WaferID
        if (*lWafer.szWaferID)
            strWaferFileName += lWafer.szWaferID;
    }
    strWaferFileName += ".png";

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                            strWaferFileFullName, "Image / PNG format (*.png)");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(iNotch <= 0)
        iNotch = lWafer.GetWaferNotch();
    else if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }

    gexReport->CreateWaferMapImage(eWafermapMode, pGroup, pFile, false, strWaferFileFullName.toLatin1().constData(), NULL, true, false);

    return NoError;
}

int
WafermapExport::CreateWafermapOutput_G85(QString& strOutputWafermapPath,
                                         QString& strWaferFileFullName,
                                         int iNotch,
                                         int iPosX,
                                         int iPosY,
                                         bool bRotateWafer,
                                         QString& /*strErrorMessage*/,
                                         QString strCustomer /*= ""*/,
                                         QString strSupplier /*= ""*/,
                                         bool bXmlFormat /*= false*/,
                                         bool bSaveAs /*= false*/,
                                         int iGroupID /*= 0*/,
                                         int iFileID /*= 0*/)
{
    int iWaferSizeX,iWaferSizeY;
    CGexFileInGroup *   pFile=0;
    CGexGroupOfFiles *  pGroup=0;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool bMergedWafer=false;
    if(iFileID < 0)
    {
        bMergedWafer=true;
        iWaferSizeX = pGroup->cStackedWaferMapData.SizeX;
        iWaferSizeY = pGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        iFileID = 0;
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
        iWaferSizeX = pFile->getWaferMapData().SizeX;
        iWaferSizeY = pFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Build file name: <path>/<lot>.0xx with 'xx': <WaferID>
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode
        strWaferFileName = pFile->getMirDatas().szLot;	// LotID

        // Build file extension
        QString strWaferId(lWafer.szWaferID), strSubLotId(pFile->getMirDatas().szSubLot);		// used to build a specific file name
        if( (strWaferId.isEmpty()) && (strSubLotId.isEmpty()))
        {
            QString strErrorMsg("WARNING : undefined wafer id and sublot id");
            GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
        }

        if(bXmlFormat)
        {
            // G85 in XML format
            strWaferFileName += "-";
            if(!strWaferId.isEmpty())
                strWaferFileName += strWaferId;
            else
                strWaferFileName += strSubLotId;
        }
        else
        {

            //  modify this line and evry thing will be OK
            // G85 Ascii format: Wafer# encoded in extension.
            strWaferFileName += ".0";
            if(!strWaferId.isEmpty())
                strWaferFileName += strWaferId;
            else
                strWaferFileName += strSubLotId;
        }
    }

    if(bXmlFormat)
        strWaferFileName += ".xml";
    else
        strWaferFileName += ".dat";

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        if(bXmlFormat)
        {
            // G85 XML format
            strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                                strWaferFileFullName, "G85 XML Semi85 inkless format (*.xml)");
        }
        else
        {
            // G85 Ascii format
            strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                                strWaferFileFullName, "G85 Semi85 inkless format (*.dat)");
        }

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(iNotch <= 0)
        iNotch = lWafer.GetWaferNotch();
    else if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }


    iWaferSizeX = lWafer.SizeX;
    iWaferSizeY = lWafer.SizeY;

    // Compute notch direction accorinng to G85 specs.
    int	iFlatNotchDirection=0;	// 0=down,90=left,180=top,270=right
    switch(iNotch)
    {
        case 12:	// Up
            iFlatNotchDirection = 180;
            break;
        case 6:		// Down
            iFlatNotchDirection = 0;
            break;
        case 3:		// Right
            iFlatNotchDirection = 270;
            break;
        case 9:		// LEFT
            iFlatNotchDirection = 90;
            break;
    }

    // Find First die coordinates in wafer
    int iFirstDieX;
    int iFirstDieY=0;
    for(iFirstDieX=0; iFirstDieX < iWaferSizeX; iFirstDieX++)
    {
        // Find first valid die on X axis...
        if(lWafer.getWafMap()[iFirstDieX].getBin() != GEX_WAFMAP_EMPTY_CELL)
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
    int iOriginLocation = 0;

    if(lWafer.GetPosXDirection() == true)
    {
        if (lWafer.GetPosYDirection() == true)
            iOriginLocation	= 2;
        else
            // DEFAULT Semi G85 Inkless (Ascci)
            iOriginLocation	= 3;
    }
    else
    {
        if (lWafer.GetPosYDirection() == true)
            iOriginLocation	= 1;
        else
            iOriginLocation	= 4;
    }

    // Check if the OriginLocation is the center
    if(((lWafer.iHighDieX + lWafer.iLowDieX) <= 1)
       && ((lWafer.iHighDieY + lWafer.iLowDieY) <= 1))
    {
        iOriginLocation = 0;
        if(lWafer.GetPosXDirection() == true)
            iFirstDieX--;
        if(lWafer.GetPosYDirection() == true)
            iFirstDieY--;

        iFirstDieX -= (lWafer.iHighDieX - lWafer.iLowDieX)/2;
        iFirstDieY -= (lWafer.iHighDieY - lWafer.iLowDieY)/2;
    }


    // Build Wafer name: lotID-waferID...but if WaferID already has a '-' sign, then assume it already holds the ful lstring we want.
    QString strWafer = lWafer.szWaferID;
    if(strWafer.count("-") <= 0)
        strWafer = QString(pFile->getMirDatas().szLot) + QString("-") + QString(lWafer.szWaferID);

    // Header section
    double	lfValue;
    if(bXmlFormat)
    {
        // XML output format
        //FIXME: not used ?
        //QDate cDate = QDate::currentDate();
        hWafermapFile << "<?xml version=\"1.0\"?>" << endl;
        hWafermapFile << "<Maps>" << endl;
        hWafermapFile << "  <Map xmlns:semi=\"http://www.semi.org\" WaferId=\"" << lWafer.szWaferID << "\" FormatRevision=\"G85-1101\">" << endl;
        //eg: <Device ProductId="00P41M9309" LotId="6D5KT00B31" CreateDate="2007091020514700" SupplierName="IBM-BTV" Rows="44" Columns="43" Orientation="0" OriginLocation="3" BinType="Decimal" NullBin="255" >
        hWafermapFile << "    <Device ProductId=\"" << pFile->getMirDatas().szPartType << "\"";
        hWafermapFile << " LotId=\"" << pFile->getMirDatas().szLot << "\"";
        QDateTime cDateTime;
        if(pFile->getMirDatas().lSetupT != 0)
            cDateTime.setTime_t(pFile->getMirDatas().lSetupT);
        else
            cDateTime.setTime_t(pFile->getMirDatas().lStartT);
        hWafermapFile << " CreateDate=\"" << cDateTime.toString("yyyyMMddhhmmsszzz") << "\"";
        hWafermapFile << " SupplierName=\"" << strSupplier << "\"";
        hWafermapFile << " Rows=\"" << iWaferSizeY << "\"";
        hWafermapFile << " Columns=\"" << iWaferSizeX << "\"";
        hWafermapFile << " Orientation=\"" << iFlatNotchDirection << "\"";
        // Original Location
        hWafermapFile << " OriginLocation=\"" << iOriginLocation << "\"";
        // Wafer sizes info
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDiameter(), lWafer.bWaferUnits);
        if(lfValue > 0)
            hWafermapFile << " WaferSize=\"" << lfValue << "\"" << endl;	// Size in mm

        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(), lWafer.bWaferUnits);
        if(lfValue > 0)
            hWafermapFile << " DeviceSizeX=\"" << lfValue*1e3 << "\"" << endl;		// Size in uM

        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(), lWafer.bWaferUnits);
        if(lfValue > 0)
            hWafermapFile << " DeviceSizeY=\"" << lfValue*1e3  << "\"" << endl;		// Size in uM

        hWafermapFile << " BinType=\"" << "Decimal" << "\"";
        hWafermapFile << " NullBin=\"255\" >";
        hWafermapFile << endl;

        // First device tested
        hWafermapFile << "<ReferenceDevice";
        hWafermapFile << " ReferenceDeviceX=\"" << iFirstDieX << "\"";
        hWafermapFile << " ReferenceDeviceY=\"" << iFirstDieY << "\" />";
        hWafermapFile << endl;


    }
    else
    {
        // Ascii output format
        hWafermapFile << "WAFER_MAP = {" << endl;
        hWafermapFile << "WAFER_ID = \"" << strWafer << "\"" << endl;
        hWafermapFile << "MAP_TYPE = \"ASCII\"" << endl;
        hWafermapFile << "NULL_BIN = \".\"" << endl;
        hWafermapFile << "ROWS = " << iWaferSizeY << endl;
        hWafermapFile << "COLUMNS = "<< iWaferSizeX << endl;
        hWafermapFile << "FLAT_NOTCH = " << iFlatNotchDirection << endl;
        if(strCustomer.isEmpty())
            strCustomer = "?";
        hWafermapFile << "CUSTOMER_NAME = \"" << strCustomer << "\"" << endl;
        hWafermapFile << "FORMAT_REVISION = \"ADI0811D\"" << endl;
        if(strSupplier.isEmpty())
            strSupplier = "Quantix - www.mentor.com";
        hWafermapFile << "SUPPLIER_NAME = \"" << strSupplier << "\""  << endl;
        hWafermapFile << "LOT_ID = \"" << pFile->getMirDatas().szLot << "\"" << endl;

        // Wafer sizes info
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDiameter(), lWafer.bWaferUnits);
        if(lfValue > 0)
            hWafermapFile << "WAFER_SIZE = " << lfValue << endl;	// Size in mm

        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(), lWafer.bWaferUnits);
        if(lfValue > 0)
            hWafermapFile << "X_SIZE = " << lfValue*1e3 << endl;		// Size in uM

        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(), lWafer.bWaferUnits);
        if(lfValue > 0)
            hWafermapFile << "Y_SIZE = " << lfValue*1e3 << endl;		// Size in uM

        // Total dies on wafer
        hWafermapFile << "DIES = " << pGroup->cMergedData.lTotalSoftBins << endl;
    }

    // Total bin classes
    CBinning * ptBinCell = NULL;

    #ifdef GCORE15334
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        if (lPatInfo && lPatInfo->GetSTDFSoftBins())
            ptBinCell = lPatInfo->GetSTDFSoftBins();
        else
            ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        if (lPatInfo && lPatInfo->GetSTDFHardBins())
            ptBinCell = lPatInfo->GetSTDFHardBins();
        else
            ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
    }
    #else
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
    }
    #endif
    int	iTotalBinClasses=0;
    int nFailCount = 0;
    int nPassCount = 0;

    if (bMergedWafer)
    {
        iTotalBinClasses = 2;

        // Merge this wafermap to the stacked array.
        for(int nIndex = 0; pGroup && nIndex < pGroup->cStackedWaferMapData.SizeX * pGroup->cStackedWaferMapData.SizeY; nIndex++)
        {
            // Get die value (Bin# or Parametric % value)
            if (pGroup->cStackedWaferMapData.cWafMap[nIndex].lStatus == GEX_WAFMAP_PASS_CELL)
                nPassCount++;
            else if (pGroup->cStackedWaferMapData.cWafMap[nIndex].lStatus == GEX_WAFMAP_FAIL_CELL)
                nFailCount++;
        }
    }
    else
    {
        while(ptBinCell != NULL)
        {
            iTotalBinClasses++;

            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        };
    }

    if(!bXmlFormat)
    {
        // Ascii output format
        hWafermapFile << "BINS = " << iTotalBinClasses << endl;
    }

    // Detail list of bins...
    char	cDie=' ';
    QString strString;

    #ifdef GCORE15334
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        if (lPatInfo && lPatInfo->GetSTDFSoftBins())
            ptBinCell = lPatInfo->GetSTDFSoftBins();
        else
            ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        if (lPatInfo && lPatInfo->GetSTDFHardBins())
            ptBinCell = lPatInfo->GetSTDFHardBins();
        else
            ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
    }
    #else
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        ptBinCell = pGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
    }
    #endif

    if (bMergedWafer)
    {
        if(bXmlFormat)
        {
            // XML output format
            // eg: <Bin BinCode="1" BinCount="48" BinQuality="Pass" BinDescription="Pickable Site"/>
            hWafermapFile << "      <Bin BinCode=\"0\" BinQuality=\"Fail\" BinCount=\"" << nFailCount << "\" BinDescription=\"\"" << endl;
            hWafermapFile << "      <Bin BinCode=\"1\" BinQuality=\"Pass\" BinCount=\"" << nPassCount << "\" BinDescription=\"\"" << endl;
        }
        else
        {
            // ASCII output format
            hWafermapFile << "BIN = \"0\" " << strString.sprintf("%-6d", nFailCount) << "\"Fail\" \"\" " << endl;
            hWafermapFile << "BIN = \"1\" " << strString.sprintf("%-6d", nPassCount) << "\"Pass\" \"\" " << endl;
        }
    }
    else
    {
        while(ptBinCell != NULL)
        {
            if(bXmlFormat)
            {
                // XML output format
                // eg: <Bin BinCode="1" BinCount="48" BinQuality="Pass" BinDescription="Pickable Site"/>
                hWafermapFile << "      <Bin BinCode=\"";
                if(ptBinCell->iBinValue > 254)
                    hWafermapFile << "254";
                else
                    hWafermapFile << ptBinCell->iBinValue;
                hWafermapFile << "\"";

                // Bin type: Pass/Fail
                hWafermapFile << " BinQuality=\"";
                if(ptBinCell->cPassFail == 'P')
                    hWafermapFile << "Pass\"";
                else
                    if(ptBinCell->cPassFail == 'F')
                        hWafermapFile << "Fail\"";
                    else
                        hWafermapFile << "\"";

                // Total parts in bin.
                hWafermapFile << " BinCount=\"" << ptBinCell->ldTotalCount << "\"";

                // Bin name.
                hWafermapFile << " BinDescription=\"" << ptBinCell->strBinName << "\"/>" << endl;
            }
            else
            {
                // Display Bin#
                hWafermapFile << "BIN = \"";

                // case 7332
                if (strCustomer.compare("ADI", Qt::CaseInsensitive) == 0)
                {
                    if(ptBinCell->iBinValue <= 9)
                        cDie = '0' + ptBinCell->iBinValue;
                    else if(ptBinCell->iBinValue <= 32)
                        cDie = 'A' + ptBinCell->iBinValue - 10;
                    else
                    {
                        // For Bin# > 32, display die as 'X'
                        cDie = 'X';
                    }
                }
                else
                {
                    /*
                        // case 6132
                        // Incuding extended ASCII table
                        if(ptBinCell->iBinValue <= 9)
                            cDie = '0' + ptBinCell->iBinValue;
                        else if(ptBinCell->iBinValue <= 189)
                            cDie = 'A' + ptBinCell->iBinValue - 10;
                        else
                        {
                            // For Bin# > 189, display die as '#'
                            cDie = '#';
                        }
                    */
                    // PAT-63
                    if(ptBinCell->iBinValue <= 9)
                      cDie = '0' + ptBinCell->iBinValue;
                    else if(ptBinCell->iBinValue <= 32)
                        cDie = 'A' + ptBinCell->iBinValue - 10;
                      else
                      {
                        // For Bin# > 32, display die as 'W' if good bin, or 'X' if failing bin : WT : ?
                        cDie = 'X';
                      }
                }

                hWafermapFile << cDie << "\" ";

                // Display Bin count
                strString = strString.sprintf("%-6d",ptBinCell->ldTotalCount);
                hWafermapFile << strString;

                #ifdef GCORE15334
                // Pass/Fail info (if no P/F flag, then check if we can find the Bin# from the recipe 'Good bins' list...
                if(ptBinCell->cPassFail == 'P' ||
                   (lPatInfo && (lPatInfo->GetRecipeOptions().pGoodSoftBinsList->Contains(ptBinCell->iBinValue))))
                    hWafermapFile << "\"Pass\" ";
                else
                    hWafermapFile << "\"Fail\" ";
                #else
                // Pass/Fail info (if no P/F flag, then check if we can find the Bin# from the recipe 'Good bins' list...
                if(ptBinCell->cPassFail == 'P')
                    hWafermapFile << "\"Pass\" ";
                else
                    hWafermapFile << "\"Fail\" ";
                #endif

                // Bin description.
                if(ptBinCell->strBinName.isEmpty() == FALSE)
                    hWafermapFile << "\"" << ptBinCell->strBinName << "\"";
                else
                    hWafermapFile << "\"\"";

                // End of Bin line info
                hWafermapFile << endl;
            }

            // Move to next Bin cell
            ptBinCell = ptBinCell->ptNextBin;
        };
    }

    // Wafermap
    if(bXmlFormat)
        hWafermapFile << "      <Data MapName=\"Map\" MapVersion=\"6\">" << endl;
    else
        hWafermapFile << "MAP = {" << endl;

    int	iLine,iCol,iBinCode;
    int	iStartCol,iEndCol,iColStep;
    int	iStartLine,iEndLine,iLineStep;
    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        iStartCol = 0;
        iEndCol = iWaferSizeX;
        iColStep = 1;
    }
    else
    {
        // X direction = 'L' (left)
        iStartCol = iWaferSizeX-1;
        iEndCol = -1;
        iColStep = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        iStartLine = 0;
        iEndLine = iWaferSizeY;
        iLineStep = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        iStartLine = iWaferSizeY-1;
        iEndLine = -1;
        iLineStep = -1;
    }

    for(iLine = iStartLine; iLine != iEndLine;)
    {
        // If XML, write line prefix: <Row><![CDATA[
        if(bXmlFormat)
            hWafermapFile << "        <Row><![CDATA[";

        // Processing a wafer line.
        for(iCol = iStartCol; iCol != iEndCol;)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(bMergedWafer)
                iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*iWaferSizeX))].ldCount;
            else
                iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();
            if(bMergedWafer)
            {
                if(iBinCode == GEX_WAFMAP_EMPTY_CELL)
                    cDie = '.';
                else
                {
                    if(bXmlFormat)
                    {
                        strString.sprintf("%03d",iBinCode);
                        hWafermapFile << strString << " ";			//XML output
                    }
                    else
                        hWafermapFile << QString::number(iBinCode);	//Ascii output
                }
            }
            else
            {
                // case 7332
                if (strCustomer.compare("ADI", Qt::CaseInsensitive) == 0)
                {
                    if(iBinCode == GEX_WAFMAP_EMPTY_CELL)
                        cDie = '.';
                    else if(iBinCode <= 9)
                        cDie = '0' + iBinCode;
                    else if(iBinCode <= 32)
                        cDie = 'A' + iBinCode - 10;
                    else
                    {
                        // For Bin# > 32, display die as 'X'
                        cDie = 'X';
                    }
                }
                else
                {
                    if(iBinCode == GEX_WAFMAP_EMPTY_CELL)
                        cDie = '.';
                    else if(iBinCode <= 9)
                        cDie = '0' + iBinCode;
                    else
                        // PAT-63
                        if(iBinCode <= 32)
                          cDie = 'A' + iBinCode - 10;
                     else
                        {
                            // WT: Strange comment probably from Sandrine:
                            // For Bin# > 32, display die as 'W' if good bin, or 'X' if failing bin
                            cDie = 'X';
                        }
                        /*
                         // case 6132 : Incuding extended ASCII table
                         if(iBinCode <= 189)
                            cDie = 'A' + iBinCode - 10;
                         else
                         {
                            // For Bin# > 189, display die as '#'
                            cDie = '#';
                         }
                        */
                }
            }

            // Write die value
            if(bXmlFormat)
            {
                // XML output format
                if(iBinCode < 0)
                    strString = "255";	// NulBin;
                else
                    if(iBinCode > 254)
                        strString = "254";	// Maximum bin# allowed in this format is 254
                    else
                        strString.sprintf("%03d",iBinCode);
                hWafermapFile << strString << " ";			//XML output
            }
            else
                hWafermapFile << cDie;	// Ascii output

            // Next column
            iCol += iColStep;
        }

        // Insert line break
        if(bXmlFormat)
            hWafermapFile << "]]></Row>";
        hWafermapFile << endl;

        // Next line
        iLine += iLineStep;
    }

    // End of map.
    if(bXmlFormat)
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

    file.close();
    return NoError;
}

///////////////////////////////////////////////////////////
// Tells if a valid die is around the given die (used for
// wafermap STIF generation)
///////////////////////////////////////////////////////////
bool WafermapExport::isValidDieAround(int iLine,int iCol,int iWaferSizeX,int iWaferSizeY,CWaferMap *ptEnlargedMap)
{
    int iDieOffset;
    int iColRange,iLineRange;

#if 0
    // Algorithm checking all 9 dies around center die
    for(iColRange = -1; iColRange <= 1; iColRange++)
    {
        for(iLineRange = -1; iLineRange <= 1; iLineRange++)
        {
            iDieOffset = (iCol+iColRange+((iLine+iLineRange)*(iWaferSizeX)));
            if(iColRange && iLineRange && iDieOffset >= 0 && iDieOffset < iWaferSizeX*iWaferSizeY)
            {
                // Valide die location in array and NOT the center die analyzed: check if die exists at that location
                if(ptEnlargedMap->getWafMap()[iDieOffset].iBin != GEX_WAFMAP_EMPTY_CELL)
                    return true;	// This die exists and is around the center die.
            }
        }
    }
#endif
#if 1
    // Algorithm checking all 4 dies adjacent to center die
    for(iColRange = -1; iColRange <= 1; iColRange+=2)
    {
        iDieOffset = (iCol+iColRange+((iLine)*(iWaferSizeX)));
        if(iDieOffset >= 0 && iDieOffset < iWaferSizeX*iWaferSizeY)
        {
            // Valide die location in array and NOT the center die analyzed: check if die exists at that location
            if(ptEnlargedMap->getWafMap()[iDieOffset].getBin() != GEX_WAFMAP_EMPTY_CELL)
                return true;	// This die exists and is around the center die.
        }
    }
    for(iLineRange = -1; iLineRange <= 1; iLineRange+=2)
    {
        iDieOffset = (iCol+((iLine+iLineRange)*(iWaferSizeX)));
        if(iDieOffset >= 0 && iDieOffset < iWaferSizeX*iWaferSizeY)
        {
            // Valide die location in array and NOT the center die analyzed: check if die exists at that location
            if(ptEnlargedMap->getWafMap()[iDieOffset].getBin() != GEX_WAFMAP_EMPTY_CELL)
                return true;	// This die exists and is around the center die.
        }
    }
#endif

    // No valid die around the center die analyzed
    return false;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: STIF (STMicroelectronics inkless format)
// Note: return error code
///////////////////////////////////////////////////////////
int	WafermapExport::CreateWafermapOutput_STIF(QString &strOutputWafermapPath,QString &strWaferFileFullName,int iNotch,int iPosX,int iPosY,bool bRotateWafer,QString &strErrorMessage,bool bSaveAs/*=false*/,int iGroupID/*=0*/,int iFileID/*=0*/)
{
    int iWaferSizeX,iWaferSizeY;
    CGexFileInGroup *   pFile   = NULL;
    CGexGroupOfFiles *  pGroup  = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    if(iFileID < 0)
    {
        return NoError;	// mode not supported!
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
        iWaferSizeX = pFile->getWaferMapData().SizeX;
        iWaferSizeY = pFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Build file name: <path>/<lot>.0xx with 'xx': <WaferID>
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode
        strWaferFileName = pFile->getMirDatas().szLot;	// LotID
        // Build file extension
        // G85 Ascii format: Wafer# encoded in extension.
        strWaferFileName += "-";
        strWaferFileName += pFile->getMirDatas().szSubLot;
        strWaferFileName += "-";
        strWaferFileName += pFile->getMirDatas().szperFrq;
    }
    strWaferFileName += ".STIF";

    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {

        // STIF Ascii format
        strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                            strWaferFileFullName, "STIF inkless format");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Compute probing quadrant used
    int	iQuadrant;
    if(lWafer.GetPosXDirection() == true)
    {
        if(lWafer.GetPosYDirection() == true)
            iQuadrant = 2;
        else
            iQuadrant = 3;
    }
    else
    {
        if(lWafer.GetPosYDirection() == true)
            iQuadrant = 1;
        else
            iQuadrant = 4;
    }
    // Rotate wafermap, unless default should be used for the notch
    if(iNotch <= 0)
        iNotch = lWafer.GetWaferNotch();
    else if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }
    // Get wafer size, add 2 rows & cols as wafer to be printed with one extra ring of null dies.
    //int nRowsToAdd = 2;
    int nRowsToAdd = 0;
    iWaferSizeX = nRowsToAdd + lWafer.SizeX;
    iWaferSizeY = nRowsToAdd + lWafer.SizeY;

    // Compute notch direction according to G85 specs.
    int	iFlatNotchDirection=0;	// 0=down,90=left,180=top,270=right
    switch(iNotch)
    {
        case 12:	// Up
            iFlatNotchDirection = 180;
            break;
        case 6:	// Down
            iFlatNotchDirection = 0;
            break;
        case 3:	// Right
            iFlatNotchDirection = 270;
            break;
        case 9:	// LEFT
            iFlatNotchDirection = 90;
            break;
    }

    // Total bin classes: HARD-BIN Only.
    QString     strGoodBins         = "";
    CBinning *  ptBinCell           = NULL;
    int         iTotalBinClasses    = 0;
    int         iTotalBinStatus     = 0;

    #ifdef GCORE15334
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lPatInfo && lPatInfo->GetSTDFHardBins())
        ptBinCell = lPatInfo->GetSTDFHardBins();
    else
        ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
    #else
        ptBinCell = pGroup->cMergedData.ptMergedHardBinList;
    #endif

    while(ptBinCell != NULL)
    {
        // Keep track of total Hard-Bin numbers used in wafermap)
        if(ptBinCell->ldTotalCount)
            iTotalBinClasses++;

        // Keep track of total Hard-Bin entries with a Pass/Fail info
        if(ptBinCell->ldTotalCount && (ptBinCell->cPassFail == 'P'))
        {
            iTotalBinStatus++;

            // Build list of good bins
            strGoodBins += QString::number(ptBinCell->iBinValue) + ",";
        }
        else
            if(ptBinCell->ldTotalCount && (ptBinCell->cPassFail == 'F'))
                iTotalBinStatus++;

        // Move to next Bin cell
        ptBinCell = ptBinCell->ptNextBin;
    };

    // Check if total bin count or Highest bin# is compatible with STIF format!
    if(iTotalBinClasses == 0)
    {
        strErrorMessage += "STIF map generation: Missing HardBin Pass/Fail info (STDF.HBR record incomplete)";
        return WriteError;	// Failed writing to wafermap file.
    }
    if(iTotalBinClasses > 90)
    {
        strErrorMessage += "STIF map generation: Too many Bins classes for this wafermap format (Max. allowed: 90 classes)";
        return WriteError;	// Failed writing to wafermap file.
    }
    if(iTotalBinClasses != iTotalBinStatus)
    {
        strErrorMessage += "STIF map generation: Not all HardBin records include the Pass/Fail info: STDF.HBR records incomplete";
        return WriteError;	// Failed writing to wafermap file.
    }

    // Build Wafer name: lotID-waferID...but if WaferID already has a '-' sign, then assume it already holds the full string we want.
    QString strWaferId, strSubLot;
    strWaferId = strSubLot = lWafer.szWaferID;
    if(strSubLot.count("-") <= 0)
        strSubLot = QString(pFile->getMirDatas().szLot) + QString("-") + QString(lWafer.szWaferID);
    else
        strWaferId = strSubLot.section("-",1,1);

    // Wafermap probing direction
    int	iLine,iCol,iBinCode;
    int	iStartCol,iEndCol,iColStep;
    int	iStartLine,iEndLine,iLineStep;
    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        iStartCol = 0;
        iEndCol = iWaferSizeX;
        iColStep = 1;
    }
    else
    {
        // X direction = 'L' (left)
        iStartCol = iWaferSizeX-1;
        iEndCol = -1;
        iColStep = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        iStartLine = 0;
        iEndLine = iWaferSizeY;
        iLineStep = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        iStartLine = iWaferSizeY-1;
        iEndLine = -1;
        iLineStep = -1;
    }

    // STIF signature
    // Note: Buffer to hold the STIF output (we first save it in a string so we can compute the Chekcsum)
    QString strStifOutput = "WM - V1.1 - STMicroelectronics Wafer Map File\n\n";

    // STIF Header section
    double	lfValue;
    strStifOutput += "LOT\t" + QString(pFile->getMirDatas().szLot) + "\n";
    strStifOutput += "WAFER\t" + strWaferId + "\n";
    strStifOutput += "PRODUCT\t" + QString(pFile->getMirDatas().szPartType) + "\n";
    strStifOutput += "READER\t" + strSubLot + "\n";

    // Convert to MM
    if(lWafer.bWaferUnits != 4)
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(),
                                lWafer.bWaferUnits) / 0.0254;
    else
        lfValue = lWafer.GetDieHeight() * 10.0;
    if(lfValue > 0)
        strStifOutput += "XSTEP\t" +  QString::number(lfValue,'f',0) + "\tUNITS\t(0.1)MIL\n";		// Size in 0.1Mils

    // Convert to MM
    if(lWafer.bWaferUnits != 4)
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(),
                                lWafer.bWaferUnits) / 0.0254;
    else
        lfValue = lWafer.GetDieWidth() * 10.0;
    if(lfValue > 0)
        strStifOutput += "YSTEP\t" + QString::number(lfValue,'f',0) + "\tUNITS\t(0.1)MIL\n";		// Size in 0.1Mils

    strStifOutput += "FLAT\t" + QString::number(iFlatNotchDirection) + "\n";
    if(lWafer.GetCenterDie().IsValidX())
        strStifOutput += "XFRST\t" + QString::number(lWafer.GetCenterDie().GetX()) + "\n";
    if(lWafer.GetCenterDie().IsValidY())
        strStifOutput += "YFRST\t" + QString::number(lWafer.GetCenterDie().GetY()) + "\n";
    strStifOutput += "PRQUAD\t" + QString::number(iQuadrant) + "\n";	// Probing direction (quadrant)
    strStifOutput += "COQUAD\t" + QString::number(iQuadrant) + "\n";	// Probing direction (quadrant)

    // Wafer diameter info (in MM)
    if(lWafer.bWaferUnits != 4)
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDiameter(),
                                lWafer.bWaferUnits) / 0.0254;
    else
        lfValue = lWafer.GetDiameter() * 10.0;
    if(lfValue > 0)
        strStifOutput += "DIAM\t" + QString::number(lfValue/10.0,'f',0) + "\n";	// Size in Mils

    // Wafer top-left corner. Note: one level of empty dies added all around the wafer so upper-left corner offset by one die!
    if(lWafer.GetPosXDirection() == true)
        strStifOutput += "XSTRP\t" + QString::number(lWafer.iLowDieX-(nRowsToAdd/2)) + "\n";
    else
        strStifOutput += "XSTRP\t" + QString::number(lWafer.iHighDieX-(nRowsToAdd/2)) + "\n";
    if(lWafer.GetPosYDirection() == true)
        strStifOutput += "YSTRP\t" + QString::number(lWafer.iLowDieY-(nRowsToAdd/2)) + "\n";
    else
        strStifOutput += "YSTRP\t" + QString::number(lWafer.iHighDieY-(nRowsToAdd/2)) + "\n";

    // Empty die cell code: ~
    strStifOutput += "NULBC\t126\n";

    // Compute Good yield
    int iGood,iFail;
    pFile->GetWafermapYieldInfo(true,iGood,iFail);
    strStifOutput += "GOODS\t" + QString::number(iGood) + "\n";

    // Date & Time
    QDateTime cDateTime;
    cDateTime.setTimeSpec(Qt::UTC);
    cDateTime.setTime_t(pFile->getMirDatas().lStartT);
    strStifOutput += "DATE\t" + cDateTime.toString("yyyy-MM-dd") + "\n";
    strStifOutput += "TIME\t" + cDateTime.toString("hh:mm:ss") + "\n";
    strStifOutput += "SETUP FILE\t" + QString(pFile->getMirDatas().szSetupID) + "\n";;
    strStifOutput += "OPERATOR\t" + QString(pFile->getMirDatas().szOperator) + "\n";
    strStifOutput += "TEST SYSTEM\t" + QString(pFile->getMirDatas().szNodeName) + "\n";
    strStifOutput += "TEST PROG\t" + QString(pFile->getMirDatas().szJobName) + "\n";

    // If Loadboard definition exists
    if(pFile->m_pSiteEquipmentIDMap != NULL && pFile->m_pSiteEquipmentIDMap->isEmpty() == false)
    {
        GP_SiteDescription	cSiteDescription;

        cSiteDescription = pFile->m_pSiteEquipmentIDMap->begin().value();
        strStifOutput += "PROBE CARD\t" + QString(cSiteDescription.m_strProbeCardID) + "\n";
        strStifOutput += "PROBER\t" + QString(cSiteDescription.m_strHandlerProberID) + "\n";
    }

    // WAFERMAP size: add 2 rows & cols as we add one ring of empty dies.
    strStifOutput += "\n";	// Empty line
    strStifOutput += "WMXDIM=" + QString::number(iWaferSizeX) + "\n";
    strStifOutput += "WMYDIM=" + QString::number(iWaferSizeY) + "\n";
    strStifOutput += "\n";	// Empty line

    // Create wafermap 2 rows & cols bigger so to include the outer ring of empty dies
    int	iIndex;
    int	iWafSize = iWaferSizeX*iWaferSizeY;
    CWaferMap cEnlargedMap;
    cEnlargedMap.setWaferMap (CWafMapArray::allocate(iWafSize));
    if (!cEnlargedMap.getWafMap())
        GSLOG(SYSLOG_SEV_ERROR, "WafMap allocation failed !");
    if(!cEnlargedMap.allocCellTestCounter(iWafSize))
        GSLOG(SYSLOG_SEV_ERROR, "CellTestCounter allocation failed !");

    /* HTH - case 4156 - Catch memory allocation exception
  cEnlargedMap.getWafMap() = new CWafMapArray[iWafSize];
  for(iIndex=0;iIndex< iWafSize;iIndex++)
    cEnlargedMap.getWafMap()[iIndex].iBin= GEX_WAFMAP_EMPTY_CELL;
  */

    // Fill map with original (smaller) map
    for(iLine = 0; iLine < iWaferSizeY-(nRowsToAdd/2); iLine++)
    {
        // Processing a wafer line.
        for(iCol = 0; iCol < iWaferSizeX-(nRowsToAdd/2); iCol++)
        {
            iIndex = (iCol-(nRowsToAdd/2)+((iLine-(nRowsToAdd/2))*(iWaferSizeX-nRowsToAdd)));	// Index in original map
            iBinCode = lWafer.getWafMap()[iIndex].getBin();
            iIndex = (iCol+(iLine*(iWaferSizeX)));	// Index in new map (with added 2 cols & rows)
            cEnlargedMap.getWafMap()[iIndex].setBin( iBinCode);
        }
    }


    // Build list of good bins
    GS::QtLib::Range cRange(strGoodBins);
    QString cDie;
    for(iLine = iStartLine; iLine != iEndLine;)
    {
        // Processing a wafer line.
        for(iCol = iStartCol; iCol != iEndCol;)
        {
            iBinCode = cEnlargedMap.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();
            if(iBinCode == GEX_WAFMAP_EMPTY_CELL)
            {
                // Empty die: if valid die adjacent or diagonal then change it force-inking 'z' code
                if(isValidDieAround(iLine,iCol,iWaferSizeX,iWaferSizeY,&cEnlargedMap))
                    cDie = 'z';
                else
                    cDie = '~';

                // Force to '~'
                if(nRowsToAdd == 0)
                    cDie = '~';
            }
            else
            {
                //Due to the constraint of the STIF format bin code usage is restricted to a given number of 90
                //possibilities. Indeed, The Bin code MUST be printable ASCII code. So the first value for bin
                //codes is 32. If the bin codes range provide by the probers begin at 0, the STIF generator add
                //32 to the prober Bin codes.
                //Furthermore, to differentiate the good bin codes from the bad bin codes, the STIF generator
                //adds 128 to the good bin codes
                if(!cRange.GetRangeList().isEmpty() && cRange.Contains(iBinCode))
                {
                    // Good die: set 'Bit8', and make it printable (add 32)
                    // Good bin can range from 161-232
                    // set 'Bit8'
                    if(iBinCode < 128)
                        iBinCode += 128;
                    // make it printable (add 32)
                    if(iBinCode < 160)
                        iBinCode += 32;
                    if(iBinCode > 232)
                    {
                        strErrorMessage += "STIF map generation: Pass HardBin# exceeds limit (Max. allowed Pass-Bin# is Bin72)";
                        return WriteError;	// Failed writing to wafermap file.
                    }
                }
                else
                {
                    // Fail bin: set 'Bit8'=0,make it printable (add 32)
                    // Fail bin can range from 32-121
                    // Special case for PAT Bins 140-150 can range now from 100-121 (<128 for fail)
                    if(iBinCode >= 140)
                        iBinCode -= 40;
                    // set 'Bit8'=0
                    if(iBinCode >= 128)
                        iBinCode -= 128;
                    // make it printable (add 32)
                    if(iBinCode < 32)
                        iBinCode += 32;
                    if(iBinCode > 232)
                        iBinCode = 232;	// Clamp failing bins to 232 (as they may be PAT Soft bins...)
                }
                cDie = QChar(iBinCode);
            }

            strStifOutput += cDie;	// Ascii output

            // Next column
            iCol += iColStep;
        }

        // Insert line break
        strStifOutput += "\n";

        // Next line
        iLine += iLineStep;
    }
    strStifOutput += "\n";

    // Wafermap footer
    cDateTime.setTime_t(pFile->getMirDatas().lEndT);
    //cDateTime.setTimeSpec(Qt::UTC);
    strStifOutput += "EDATE\t" + cDateTime.toString("yyyy-MM-dd") + "\n";
    strStifOutput += "ETIME\t" + cDateTime.toString("hh:mm:ss") + "\n";
    strStifOutput += "CHECKSUM\t";

    // Compute STIF file checksum
    int	iSTIF_CheckSum=0;
    int	iChar;
    for(iIndex=0;iIndex<strStifOutput.size(); iIndex++)
    {
        iChar = strStifOutput[iIndex].toAscii();
        if(iChar != '\r' && iChar != '\n')
        {
            iSTIF_CheckSum += iChar;
            iSTIF_CheckSum *=16;
            iSTIF_CheckSum = iSTIF_CheckSum % 251;
        }
    }
    // Add necessary logic including 'AA' checksum in count
    iSTIF_CheckSum += 65;
    iSTIF_CheckSum *=16;
    iSTIF_CheckSum = iSTIF_CheckSum % 251;
    iSTIF_CheckSum += 65;
    iSTIF_CheckSum = iSTIF_CheckSum % 251;

    // Negate checksum
    if(iSTIF_CheckSum)
        iSTIF_CheckSum = 251-iSTIF_CheckSum;
    // Convert checksum to two-character string & add to file
    strStifOutput += QString(65+(iSTIF_CheckSum / 16));		// 1st character of the checksum
    strStifOutput += QString(65+(iSTIF_CheckSum % 16));		// 2nd character of the checksum
    strStifOutput += "\n";

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Open Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Write full wafermap array to disk.
    hWafermapFile << strStifOutput;

    file.close();
    return NoError;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: SEMI E142 (Semi E142 XML inkless assembly map format)
// Note: return error code
///////////////////////////////////////////////////////////
int
WafermapExport::CreateWafermapOutput_SEMI142(bool isInteger2,
                                             QString& strOutputWafermapPath,
                                             QString& strWaferFileFullName,
                                             int iNotch,
                                             int iPosX,
                                             int iPosY,
                                             bool bRotateWafer,
                                             QString& strErrorMessage,
                                             QString /*strCustomer = ""*/,
                                             QString /*strSupplier = ""*/,
                                             bool bSaveAs /*= false*/,
                                             int iGroupID /*= 0*/,
                                             int iFileID /*= 0*/)
{
    int iWaferSizeX,iWaferSizeY;
    CGexFileInGroup *   pFile   = NULL;
    CGexGroupOfFiles *  pGroup  = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool bMergedWafer=false;
    if(iFileID < 0)
    {
        bMergedWafer=true;
        iWaferSizeX = pGroup->cStackedWaferMapData.SizeX;
        iWaferSizeY = pGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        iFileID = 0;
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
        iWaferSizeX = pFile->getWaferMapData().SizeX;
        iWaferSizeY = pFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    CBinning * pBinningList = NULL;

    #ifdef GCORE15334
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        if (lPatInfo && lPatInfo->GetSTDFSoftBins())
            pBinningList = lPatInfo->GetSTDFSoftBins();
        else
            pBinningList = pGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        if (lPatInfo && lPatInfo->GetSTDFHardBins())
            pBinningList = lPatInfo->GetSTDFHardBins();
        else
            pBinningList = pGroup->cMergedData.ptMergedHardBinList;
    }
    else
    {
        QString strErrorMsg("WARNING : Unable to export into E142 format, Wafer map is not filled with Soft or Hard bins values");
        GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
        return WriteError;
    }
    #else
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
    {
        pBinningList = pGroup->cMergedData.ptMergedSoftBinList;
    }
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
    {
        pBinningList = pGroup->cMergedData.ptMergedHardBinList;
    }
    else
    {
        QString strErrorMsg("WARNING : Unable to export into E142 format, Wafer map is not filled with Soft or Hard bins values");
        GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
        return WriteError;
    }
    #endif

    QString strWafer, strLot, strSubLotId;
    // Build Wafer name: lotID-waferID
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode ;
        strLot= pFile->getMirDatas().szLot;
        strWafer=lWafer.szWaferID;
        strSubLotId = pFile->getMirDatas().szSubLot;
        if( (strWafer.isEmpty()) && (strSubLotId.isEmpty()) )
        {
            QString strErrorMsg("WARNING : undefined wafer id and sublot id");
            GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
        }

        if((strLot.isEmpty() == false) && (strWafer.count(strLot) >0))
        {
            // If WaferID includes LotID (or equal), only keep WaferID ( if wafer id undefined, take sub lot id )
            if(!strWafer.isEmpty())
                strWaferFileName = strWafer;
            else
                strWaferFileName = strSubLotId;
        }
        else
        {
            strWaferFileName = strLot + "-";
            if(!strWafer.isEmpty())
                strWaferFileName += strWafer;
            else
                strWaferFileName += strSubLotId;
        }
    }
    strWaferFileName += ".xml";

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false && strOutputWafermapPath.endsWith(".xml", Qt::CaseInsensitive))
        strWaferFileFullName = strOutputWafermapPath;	// Full XML file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        strWaferFileFullName = QFileDialog::getSaveFileName(pGexMainWindow, "Save Wafer data as...",
                                                            strWaferFileFullName, "SEMI E142 XML inkless format (*.xml)");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(iNotch <= 0)
        iNotch = lWafer.GetWaferNotch();
    else if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }
    iWaferSizeX = lWafer.SizeX;
    iWaferSizeY = lWafer.SizeY;

    // Define wafer orientation
    int	nOrientation = 0;
    switch(iNotch)
    {
        case 12:
            nOrientation = 180;
            break;
        case 3:
            nOrientation = 270;
            break;
        case 6:
            nOrientation = 0;
            break;
        case 9:
            nOrientation = 90;
            break;
    }

    // Find First die coordinates in wafer
    int iFirstDieX;
    int iFirstDieY=0;
    for(iFirstDieX=0; iFirstDieX < iWaferSizeX; iFirstDieX++)
    {
        // Find first valid die on X axis...
        if(lWafer.getWafMap()[iFirstDieX].getBin() != GEX_WAFMAP_EMPTY_CELL)
            break;
    }

    // Define location and direction for the coordinate system
    QString strOriginLocation;
    QString strAxisDirection;
    int iOriginX=0;
    int iOriginY=0;

    iOriginX = lWafer.iLowDieX;
    iOriginY = lWafer.iLowDieY;

    // Semi E142 Xml specification
    // AxisDirection specify the increment direction for X and Y
    // OriginLocation specify the position of the 0.0 in the Axis
    // AxisDirection and OriginLocation allow to start the WaferMap
    // with negative or positive coordinates
    if(lWafer.GetPosXDirection() == true)
    {
        if (lWafer.GetPosYDirection() == true)
        {
            strAxisDirection	= GEX_WAFMAP_EXPORT_AXIS_DOWNRIGHT;
            strOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_UPPERLEFT;
        }
        else
        {
            strAxisDirection	= GEX_WAFMAP_EXPORT_AXIS_UPRIGHT;
            strOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_LOWERLEFT;
        }
    }
    else
    {
        if (lWafer.GetPosYDirection() == true)
        {
            strAxisDirection	= GEX_WAFMAP_EXPORT_AXIS_DOWNLEFT;
            strOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_UPPERRIGHT;
        }
        else
        {
            strAxisDirection	= GEX_WAFMAP_EXPORT_AXIS_UPLEFT;
            strOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_LOWERRIGHT;
        }
    }

    // Check if the OriginLocation is the center
    if(((lWafer.iHighDieX + lWafer.iLowDieX) <= 1)
       && ((lWafer.iHighDieY + lWafer.iLowDieY) <= 1))
    {
        strOriginLocation = GEX_WAFMAP_EXPORT_ORIGIN_CENTER;
        iOriginX = iOriginY = 0;
        if(lWafer.GetPosXDirection() == true)
            iFirstDieX--;
        if(lWafer.GetPosYDirection() == true)
            iFirstDieY--;

        iFirstDieX -= (lWafer.iHighDieX - lWafer.iLowDieX)/2;
        iFirstDieY -= (lWafer.iHighDieY - lWafer.iLowDieY)/2;
    }
    else
    {
        iFirstDieX += iOriginX;
        iFirstDieY += iOriginY;
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
    float lfWaferSize = CWaferMap::ConvertToMM(lWafer.GetDiameter(),
                                      lWafer.bWaferUnits);
    hWafermapFile << "\t\t\t<DeviceSize X=\"" << QString::number(lfWaferSize) << "\" Y=\"" << QString::number(lfWaferSize) << "\"/>" << endl;
    hWafermapFile << "\t\t\t<ChildLayouts>" << endl;
    hWafermapFile << "\t\t\t\t<ChildLayout LayoutId=\"Devices\"/>" << endl;
    hWafermapFile << "\t\t\t</ChildLayouts>" << endl;
    hWafermapFile << "\t\t</Layout>" << endl;
    hWafermapFile << "\t\t<Layout LayoutId=\"Devices\" DefaultUnits=\"micron\">" << endl;
    hWafermapFile << "\t\t\t<Dimension X=\"" << iWaferSizeX << "\" Y=\"" << iWaferSizeY << "\"/>" << endl;
    //hWafermapFile << "	<LowerLeft X=\"" << lWafer.iHighDieX << "\" Y=\"" << lWafer.iLowDieY << "\"/>" << endl;
    // Wafer sizes info
    // Size in uM
    float lfDieWidth = 1e3 * CWaferMap::ConvertToMM(lWafer.GetDieWidth(),
                                           lWafer.bWaferUnits);
    float lfDieHeight = 1e3 * CWaferMap::ConvertToMM(lWafer.GetDieHeight(),
                                            lWafer.bWaferUnits);
    hWafermapFile << "\t\t\t<DeviceSize X=\"" << QString::number(lfDieWidth) << "\" Y=\"" << QString::number(lfDieHeight) << "\"/>" << endl;
    hWafermapFile << "\t\t\t<StepSize X=\"" << QString::number(lfDieWidth) << "\" Y=\"" << QString::number(lfDieHeight) << "\"/>" << endl;
    hWafermapFile << "\t\t\t<ProductId>" << pFile->getMirDatas().szPartType <<"</ProductId>" << endl;
    hWafermapFile << "\t\t</Layout>" << endl;
    hWafermapFile << "\t</Layouts>" << endl;

    // Semi E142 Substrate Extension
    hWafermapFile << "\t<Substrates>" << endl;
    hWafermapFile << "\t\t<Substrate SubstrateType=\"Wafer\" SubstrateId=\"" << strWafer <<"\"" << endl;
    hWafermapFile << "\t\t				xsi:type=\"sme:SubstrateTypeExtension\" >" << endl;

    if(!strLot.isEmpty())
        hWafermapFile << "\t\t\t<LotId>" << strLot << "</LotId>" << endl;
    QDateTime cDateTime;
    hWafermapFile << "\t\t\t<AliasIds>" << endl;
    cDateTime.setTime_t(pFile->getMirDatas().lStartT);
    hWafermapFile << "\t\t\t\t<AliasId Type=\"TestStartTime\" Value=\""<<cDateTime.toString("yyyyMMddhhmmsszzz")<<"\" />" <<endl;
    cDateTime.setTime_t(pFile->getMirDatas().lEndT);
    hWafermapFile << "\t\t\t\t<AliasId Type=\"TestEndTime\" Value=\""<<cDateTime.toString("yyyyMMddhhmmsszzz")<<"\" />" <<endl;
    hWafermapFile << "\t\t\t</AliasIds>" << endl;
    hWafermapFile << "\t\t\t<CarrierType> "<<"NA"<<" </CarrierType>"<< endl;
    hWafermapFile << "\t\t\t<CarrierId> "<<"NA"<<" </CarrierId>"<< endl;
    hWafermapFile << "\t\t\t<SlotNumber> "<<"1"<<" </SlotNumber>"<< endl;
    hWafermapFile << "\t\t\t<SubstrateNumber> "<<"1"<<" </SubstrateNumber>"<< endl;
    hWafermapFile << "\t\t\t<GoodDevices> "<<QString::number(pFile->getPcrDatas().lGoodCount) <<" </GoodDevices>"<< endl;
    hWafermapFile << "\t\t\t<SupplierName> "<<(!QString(pFile->getMirDatas().szFacilityID).isEmpty() ? QString(pFile->getMirDatas().szFacilityID) : QString(""))<<" </SupplierName>"<< endl;
    hWafermapFile << "\t\t\t<Status> "<<"NA"<<" </Status>"<< endl;

    hWafermapFile << "\t\t\t<sme:SupplierData>" << endl;

    hWafermapFile << "\t\t\t\t<sme:SetupFile Value=\"" << (!QString(pFile->getMirDatas().szAuxFile).isEmpty() ? QString(pFile->getMirDatas().szAuxFile) : QString("")) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:TestSystem Value=\"" <<(!QString(pFile->getMirDatas().szNodeName).isEmpty() ? QString(pFile->getMirDatas().szNodeName) : QString("")) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:TestProgram Value=\"" <<(!QString(pFile->getMirDatas().szJobName).isEmpty() ? QString(pFile->getMirDatas().szJobName) : QString("")) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:Prober Value=\"" << (!QString(pFile->getMirDatas().szHandlerProberID).isEmpty() ? QString(pFile->getMirDatas().szHandlerProberID) : QString("")) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:Operator Value=\"" << (!QString(pFile->getMirDatas().szOperator).isEmpty() ? QString(pFile->getMirDatas().szOperator) : QString("")) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t<sme:ProbeCard Value=\"" <<(!QString(pFile->getMirDatas().szProbeCardID).isEmpty() ? QString(pFile->getMirDatas().szProbeCardID) : QString("")) << "\"/>" << endl;
    cDateTime.setTime_t(pFile->getMirDatas().lStartT);
    hWafermapFile << "\t\t\t\t<sme:TestStartTime Value=\"" << cDateTime.toString("yyyyMMddhhmmsszzz") << "\"/>" << endl;
    cDateTime.setTime_t(pFile->getMirDatas().lEndT);
    hWafermapFile << "\t\t\t\t<sme:TestEndTime Value=\"" << cDateTime.toString("yyyyMMddhhmmsszzz") << "\"/>" << endl;
    hWafermapFile << "\t\t\t</sme:SupplierData>" << endl;
    hWafermapFile << "\t\t</Substrate>" << endl;
    hWafermapFile << "\t</Substrates>" << endl;
    // Semi E142 Substrate Extension

    hWafermapFile << "\t<SubstrateMaps>" << endl;
    hWafermapFile << "\t\t<SubstrateMap SubstrateType=\"Wafer\" SubstrateId=\"" << strWafer << "\" ";
    hWafermapFile << "Orientation=\"" << nOrientation << "\" OriginLocation=\"" << strOriginLocation << "\" ";
    hWafermapFile << "AxisDirection=\"" << strAxisDirection << "\" LayoutSpecifier=\"WaferLayout/Devices\">" << endl;

    //	hWafermapFile << "<Overlay MapName=\"SortGrade\" MapVersion=\"1\">" << endl;

    // CHECK WITH PHILIPPE IF THIS WILL NOT DISTURB ST PROCESS
    if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_HBIN)
        hWafermapFile << "\t\t\t<Overlay MapName=\"HARD BIN MAP\" MapVersion=\"1\">" << endl;
    else if (lWafer.lTestNumber == GEX_TESTNBR_OFFSET_EXT_SBIN)
        hWafermapFile << "\t\t\t<Overlay MapName=\"SOFT BIN MAP\" MapVersion=\"1\">" << endl;

    hWafermapFile << "\t\t\t\t<ReferenceDevices>" << endl;

    hWafermapFile << "\t\t\t\t\t<ReferenceDevice Name=\"OriginLocation\">" << endl;
    hWafermapFile << "\t\t\t\t\t\t<Coordinates X=\"" << QString::number(iOriginX) << "\"";
    hWafermapFile << " Y=\""  << QString::number(iOriginY) << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t\t</ReferenceDevice>" << endl;

    hWafermapFile << "\t\t\t\t\t<ReferenceDevice Name=\"FirstDevice\">" << endl;
    hWafermapFile << "\t\t\t\t\t\t<Coordinates X=\"" << iFirstDieX << "\" Y=\"" << iFirstDieY << "\"/>" << endl;
    hWafermapFile << "\t\t\t\t\t</ReferenceDevice>" << endl;
    hWafermapFile << "\t\t\t\t</ReferenceDevices>" << endl;

    // Identify highest bin#
    int           iHighestBin = 0;
    CBinning *    ptBinCell   = pBinningList;
    while(ptBinCell != NULL)
    {
        iHighestBin = qMax(iHighestBin,ptBinCell->iBinValue);
        ptBinCell = ptBinCell->ptNextBin;
    };

    QString	strNullBin;
    char    szBinStringSize[10];
    int		iBinFieldSize=4;
    QString binType = "Integer2";

    if(!isInteger2)
    {
        binType = "HexaDecimal";
        if(iHighestBin <= 0xff)
            iBinFieldSize = 2;
        else
        {
            strErrorMessage = "Unable to export into E142 format: Highest bin (";
            strErrorMessage += QString::number(iHighestBin) + ") is greater than 0xff ";
            strErrorMessage += "and is not supported with Hexadecimal bin type.";

            GSLOG(SYSLOG_SEV_ERROR, strErrorMessage.toLatin1().constData());
            return WaferExport;
        }
    }

    sprintf(szBinStringSize,"%%0%dX",iBinFieldSize);
    strNullBin = strNullBin.fill('F',iBinFieldSize);

    hWafermapFile << "\t\t\t\t<BinCodeMap BinType=\""<<binType<<"\" NullBin=\"" << strNullBin << "\" ";
    //hWafermapFile << "MapType=\"" << GEX_WAFMAP_EXPORT_BIN_MAPTYPE_2D ;
    hWafermapFile << ">" << endl;

    hWafermapFile << "\t\t\t\t\t<BinDefinitions>" << endl;
    // Detail list of bins...
    QString strString;
    QString strDie;
    ptBinCell = pBinningList;
    while(ptBinCell != NULL)
    {
        // Display Bin#
        hWafermapFile << "\t\t\t\t\t\t<BinDefinition BinCode= \"";
        strDie.sprintf(szBinStringSize,ptBinCell->iBinValue);	// Hexa value
        hWafermapFile << strDie << "\" BinCount=\"" << ptBinCell->ldTotalCount << "\" BinQuality=\"";

        // Display Bin quality
        if (ptBinCell->cPassFail == 'P')
            strString = GEX_WAFMAP_EXPORT_BIN_QUALITY_PASS;
        else if (ptBinCell->cPassFail == 'F')
            strString = GEX_WAFMAP_EXPORT_BIN_QUALITY_FAIL;
        else
            strString = GEX_WAFMAP_EXPORT_BIN_QUALITY_NULL;

        hWafermapFile << strString << "\" BinDescription=\"";

        // Bin description.
        if(ptBinCell->strBinName.isEmpty() == FALSE)
            hWafermapFile << ptBinCell->strBinName << "\"";
        else
            hWafermapFile << "\"";

        // End of Bin line info
        hWafermapFile << "/>" << endl;

        // Move to next Bin cell
        ptBinCell = ptBinCell->ptNextBin;
    };

    // Close Bin definitions
    hWafermapFile << "\t\t\t\t\t</BinDefinitions>" << endl;

    // Wafermap
    int	iLine,iCol,iBinCode;
    int	iStartCol,iEndCol,iColStep;
    int	iStartLine,iEndLine,iLineStep;
    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        iStartCol = 0;
        iEndCol = iWaferSizeX;
        iColStep = 1;
    }
    else
    {
        // X direction = 'L' (left)
        iStartCol = iWaferSizeX-1;
        iEndCol = -1;
        iColStep = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        iStartLine = 0;
        iEndLine = iWaferSizeY;
        iLineStep = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        iStartLine = iWaferSizeY-1;
        iEndLine = -1;
        iLineStep = -1;
    }

    for(iLine = iStartLine; iLine != iEndLine;)
    {
        // Processing a wafer line.
        hWafermapFile << "\t\t\t\t\t<BinCode>";
        for(iCol = iStartCol; iCol != iEndCol;)
        {
            // Get PAT-Man binning at location iRow,iCol.
            if(bMergedWafer)
                iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*iWaferSizeX))].ldCount;
            else
                iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();

            if(iBinCode == GEX_WAFMAP_EMPTY_CELL)
                strDie = strNullBin;
            else
                strDie.sprintf(szBinStringSize,iBinCode);	// Hexa value

            // Write die value
            hWafermapFile << strDie;

            // Next column
            iCol += iColStep;
        }
        // Insert XML end of line + line break
        hWafermapFile << "</BinCode>" << endl;

        // Next line
        iLine += iLineStep;
    }

    hWafermapFile << "\t\t\t\t</BinCodeMap>" << endl;
    hWafermapFile << "\t\t\t</Overlay>" << endl;
    hWafermapFile << "\t\t</SubstrateMap>" << endl;
    hWafermapFile << "\t</SubstrateMaps>" << endl;
    hWafermapFile << "</MapData>" << endl;

    file.close();
    return NoError;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: SINF (Simple INF)
// POSX direction is always RIGHT and POSY direction is always DOWN
// Note: return error code
///////////////////////////////////////////////////////////
int
WafermapExport::CreateWafermapOutput_SINF(QString& strOutputWafermapPath,
                                          QString& strWaferFileFullName,
                                          int iNotch,
                                          int /*iPosX*/,
                                          int /*iPosY*/,
                                          bool bRotateWafer,
                                          QString& /*strErrorMessage*/,
                                          const GexTbPatSinf& lSinfInfo,
                                          bool bSaveAs /*= false*/,
                                          int iGroupID /*= 0*/,
                                          int iFileID /*= 0*/)
{
    int iWaferSizeX,iWaferSizeY;
    CGexGroupOfFiles *  pGroup  = NULL;
    CGexFileInGroup *   pFile   = NULL;
    CWaferMap           lWafer;

    // Pointer to Data file so to extract  Wafermap Bin results.
    if ((iGroupID < 0) || (iGroupID >= gexReport->getGroupsList().size()))
        pGroup = NULL;
    else
        pGroup = gexReport->getGroupsList().at(iGroupID);
    if(pGroup == NULL)
        return NoError;	// Should never happen!

    // Check if wafer to export is 'merged wafer'
    bool bMergedWafer=false;
    if(iFileID < 0)
    {
        bMergedWafer=true;
        iWaferSizeX = pGroup->cStackedWaferMapData.SizeX;
        iWaferSizeY = pGroup->cStackedWaferMapData.SizeY;

        // Get handle to first file in group
        iFileID = 0;
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
    }
    else
    {
        if (iFileID >= pGroup->pFilesList.size())
            pFile = NULL;
        else
            pFile = pGroup->pFilesList.at(iFileID);
        if(pFile == NULL)
            return NoError;	// Should never happen!
        iWaferSizeX = pFile->getWaferMapData().SizeX;
        iWaferSizeY = pFile->getWaferMapData().SizeY;
    }

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Build file name: <path>/<lot>-<WaferID>
    bool	bFlag;
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode
        if(lSinfInfo.strWafer.isEmpty())
        {
            QString strLotId(pFile->getMirDatas().szLot);
            QString strWaferId(lWafer.szWaferID), strSubLotId(pFile->getMirDatas().szSubLot);
            if( (strWaferId.isEmpty()) && (strSubLotId.isEmpty()) )
            {
                QString strErrorMsg((char*)"WARNING : undefined wafer id and sublot id");
                GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
            }

            strWaferFileName = strLotId + QString(".");
            if( !strWaferId.isEmpty() )
                strWaferFileName += strWaferId;		// <LotID>.<waferID>
            else
                strWaferFileName += strSubLotId;	// <LotId>.<SubLotId>
        }
        else
        {
            // A KLA/INF optional source was defined, then use it to build the file name.
            lSinfInfo.strWafer.toLong(&bFlag);
            if(bFlag)
                strWaferFileName = lSinfInfo.strLot + QString(".") + lSinfInfo.strWafer;	// <LotID>.<waferID> : No OCR enabled because Wafer string only holds a number!
            else
                strWaferFileName = lSinfInfo.strWafer;	// <OCR_WaferID>
        }
    }

    // Build file extension
    strWaferFileName += ".sinf";

#if 0 // Case 3898
#if defined(Q_OS_WIN32)
    // Clean up all invalid characters
    strWaferFileName.replace(QRegExp("[^A-Za-z0-9^&'@{}[],$=!#()%.+~_ -"), "");
#endif
#else
    // Clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);
#endif

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Check if allow user to customize output path & name.
    if(bSaveAs)
    {
        strWaferFileFullName =
                QFileDialog::getSaveFileName(pGexMainWindow,
                                             "Save Wafer data as...",
                                             strWaferFileFullName,
                                             "SINF format (*.sinf *.inf)");

        // If no file selected, ignore command.
        if(strWaferFileFullName.isEmpty())
            return NoError;

        strOutputWafermapPath = QFileInfo(strWaferFileFullName).absolutePath();
    }

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // FORCE POSX TO RIGHT AND POSY TO DOWN
    // THE SINF FORMAT DOESN'T SUPPORT ANY OTHER COORDINATE SYSTEM
    lWafer.SetPosXDirection(true);
    lWafer.SetPosYDirection(true);

    // Rotate WaferMap, and set notch location
    int	iFlatNotchDirection=0;	// 0=down,90=left,180=top,270=right

    // Check if notch location read from KLA INF file selected as optional source
    if(lSinfInfo.iFlatDirection >= 0)
    {
        // KLA INF file selected as optional source, so do not make custom rotation
        iFlatNotchDirection = lSinfInfo.iFlatDirection;
    }
    else
    {
        // Rotate wafermap, unless default should be used for the notch
        if(iNotch <= 0)
            iNotch = lWafer.GetWaferNotch();
        else if(bRotateWafer)
        {
            while(lWafer.GetWaferNotch() != iNotch)
                lWafer.RotateWafer();
        }

        iWaferSizeX = lWafer.SizeX;
        iWaferSizeY = lWafer.SizeY;
        // Compute notch direction
        switch(iNotch)
        {
            case 12:	// Up
                iFlatNotchDirection = 180;
                break;
            case 6:	// Down
                iFlatNotchDirection = 0;
                break;
            case 3:	// Right
                iFlatNotchDirection = 270;
                break;
            case 9:	// LEFT
                iFlatNotchDirection = 90;
                break;
        }
    }

    // Device
    hWafermapFile << "DEVICE:";
    if(lSinfInfo.strDevice.isEmpty())
        hWafermapFile << pFile->getMirDatas().szPartType << endl;
    else
        hWafermapFile << lSinfInfo.strDevice << endl;

    // Lot
    hWafermapFile << "LOT:";
    if(lSinfInfo.strLot.isEmpty())
        hWafermapFile << pFile->getMirDatas().szLot << endl;
    else
        hWafermapFile << lSinfInfo.strLot << endl;

    // Wafer
    hWafermapFile << "WAFER:";
    if(lSinfInfo.strWafer.isEmpty())
        hWafermapFile << lWafer.szWaferID << endl;
    else
        hWafermapFile << lSinfInfo.strWafer << endl;

    hWafermapFile << "FNLOC:" << iFlatNotchDirection << endl;

    int	iWaferWidth = (lSinfInfo.iWaferAndPaddingCols > 0) ? lSinfInfo.iWaferAndPaddingCols : iWaferSizeX;
    int	iWaferHeigth = (lSinfInfo.iWaferAndPaddingRows > 0) ? lSinfInfo.iWaferAndPaddingRows : iWaferSizeY;

    hWafermapFile << "ROWCT:" << iWaferHeigth << endl;
    hWafermapFile << "COLCT:"<< iWaferWidth << endl;

    // List of good bins
    if(lSinfInfo.strBCEQ.isEmpty())
        hWafermapFile << "BCEQU:01" << endl;
    else
        hWafermapFile << "BCEQU:" << lSinfInfo.strBCEQ << endl;

    // Die reference location
    if(lSinfInfo.iRefPX != -32768)
    {
        // Report Refernence die as in KLA/INF file
        hWafermapFile << "REFPX:" << lSinfInfo.iRefPX << endl;
        hWafermapFile << "REFPY:" << lSinfInfo.iRefPY << endl;
    }
    else
    {
        if(lSinfInfo.iColRdc || lSinfInfo.iRowRdc)
        {
            // Refernce die from KLA/INF but offset if required (as here we do not have any empty starting wafermap line or col.).
            //m_cSinfInfo.iRefPX = 1 + m_cSinfInfo.iColRdc - iColMin;
            //m_cSinfInfo.iRefPY = 1 + m_cSinfInfo.iRowRdc - iRowMin;
            hWafermapFile << "REFPX:" << (lSinfInfo.iColRdc-lWafer.iLowDieX) << endl;
            hWafermapFile << "REFPY:" << (lSinfInfo.iRowRdc-lWafer.iLowDieY) << endl;
        }
        else
        {
            hWafermapFile << "REFPX:" << lWafer.iLowDieX << endl;
            hWafermapFile << "REFPY:" << lWafer.iLowDieY << endl;
        }
    }
    hWafermapFile << "DUTMS:mm"  << endl;

    // Wafer sizes info: Size in X
    double	lfValue;
    if(lSinfInfo.lfDieSiezX > 0)
        lfValue = lSinfInfo.lfDieSiezX;
    else
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieWidth(),
                                lWafer.bWaferUnits);
    if(lfValue <= 0)
        lfValue = 0;
    hWafermapFile << "XDIES:" << lfValue << endl;		// Size in cm

    // Wafer sizes info: Size in Y
    if(lSinfInfo.lfDieSiezY > 0)
        lfValue = lSinfInfo.lfDieSiezY;
    else
        lfValue = CWaferMap::ConvertToMM(lWafer.GetDieHeight(),
                                lWafer.bWaferUnits);
    if(lfValue <= 0)
        lfValue = 0;
    hWafermapFile << "YDIES:" << lfValue << endl;		// Size in cm

    // Wafermap
    QString	strDie;

    // Check if Wafer created from a KLA/INF source file
    if(lSinfInfo.strNewWafermap.isEmpty() == false)
    {
        // Simply duplicate the same ASCII wafermap as in KLA/INF
        strDie = lSinfInfo.strNewWafermap;	// Full wafermap
        strDie = strDie.replace(KLA_ROW_DATA_TAB_STRING,KLA_ROW_DATA_STRING);				// Eliminate tabs preceeding the 'RowData:' text wafermap
        hWafermapFile << strDie << endl;
    }
    else
    {
        // Wafermap
        int	iLine,iCol,iBinCode;
        int	iStartCol,iEndCol,iColStep;
        int	iStartLine,iEndLine,iLineStep;
        // Check for X direction
        if(lWafer.GetPosXDirection() == true)
        {
            // X direction = 'R' (right)
            iStartCol = 0;
            iEndCol = iWaferSizeX;
            iColStep = 1;
        }
        else
        {
            // X direction = 'L' (left)
            iStartCol = iWaferSizeX-1;
            iEndCol = -1;
            iColStep = -1;
        }
        // Check for Y direction
        if(lWafer.GetPosYDirection() == true)
        {
            // Y direction = 'D' (down)
            iStartLine = 0;
            iEndLine = iWaferSizeY;
            iLineStep = 1;
        }
        else
        {
            // Y direction = 'U' (up)
            iStartLine = iWaferSizeY-1;
            iEndLine = -1;
            iLineStep = -1;
        }

        // Detect highest bin# so to decide how many digits it takes in the ASCII Hex encoding
        int iBinLength=0;
        for(iLine = iStartLine; iLine != iEndLine;)
        {
            for(iCol = iStartCol; iCol != iEndCol;)
            {
                // Get PAT-Man binning at location iRow,iCol.
                if(bMergedWafer)
                    iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*iWaferSizeX))].ldCount;
                else
                    iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();
                if(iBinCode != GEX_WAFMAP_EMPTY_CELL)
                    strDie.sprintf("%X",iBinCode);	// Hexa value
                iBinLength = qMax(iBinLength, strDie.length());

                // Next column
                iCol += iColStep;
            }

            // Next line
            iLine += iLineStep;
        }

        // Make sure bin# takes minimum of 2 digits
        if(iBinLength < 2)
            iBinLength = 2;

        // Write SINF file
        for(iLine = iStartLine; iLine != iEndLine;)
        {
            // Starting line
            hWafermapFile << "RowData:";

            for(iCol = iStartCol; iCol != iEndCol;)
            {
                // Get PAT-Man binning at location iRow,iCol.
                if(bMergedWafer)
                    iBinCode = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*iWaferSizeX))].ldCount;
                else
                    iBinCode = lWafer.getWafMap()[(iCol+(iLine*iWaferSizeX))].getBin();
                if(iBinCode == GEX_WAFMAP_EMPTY_CELL)
                    strDie = strDie.fill('_',iBinLength);
                else
                    strDie.sprintf("%X",iBinCode);	// Hexa value
                strDie = strDie.rightJustified(iBinLength,'0');

                // Write die Hex. value + space
                hWafermapFile << strDie << " ";

                // Next column
                iCol += iColStep;
            }
            // Insert line break
            hWafermapFile << endl;

            // Next line
            iLine += iLineStep;
        }
    }

    file.close();
    return NoError;
}

///////////////////////////////////////////////////////////
// Convert Date&Time to 12 bytes BCD string.
///////////////////////////////////////////////////////////
static QString strBuildTimeBCD(long lTime)
{
    QString strBcdDateTime;

    QDateTime cDateTime;
    cDateTime.setTime_t(lTime);
    QDate cDate = cDateTime.date();
    QTime cTime = cDateTime.time();

    strBcdDateTime  = cDate.year() / 10;	// YY 1st digit
    strBcdDateTime += cDate.year() % 10;	// YY 2nd digit
    strBcdDateTime  = cDate.month() / 10;	// MM 1st digit
    strBcdDateTime += cDate.month() % 10;	// MM 2nd digit
    strBcdDateTime  = cDate.day() / 10;		// DD 1st digit
    strBcdDateTime += cDate.day() % 10;		// DD 2nd digit

    strBcdDateTime  = cTime.hour() / 10;	// HH 1st digit
    strBcdDateTime += cTime.hour() % 10;	// HH 2nd digit
    strBcdDateTime  = cTime.minute() / 10;	// MM 1st digit
    strBcdDateTime += cTime.minute() % 10;	// MM 2nd digit
    strBcdDateTime  = cTime.second() / 10;	// SS 1st digit
    strBcdDateTime += cTime.second() % 10;	// SS 2nd digit

    return strBcdDateTime;
}

///////////////////////////////////////////////////////////
// Create a Wafermap output file: TEL P8 wafer prober binary format
// Note: return error code
///////////////////////////////////////////////////////////
int
WafermapExport::CreateWafermapOutput_TELP8(QString& strOutputWafermapPath,
                                           QString& strWaferFileFullName,
                                           int iNotch,
                                           int iPosX,
                                           int iPosY,
                                           bool bRotateWafer,
                                           QString& /*strErrorMessage*/)
{
    int	iWaferSizeX;
    int	iWaferSizeY;

    // Pointer to Data file so to extract  Wafermap Bin results.
    CGexGroupOfFiles *  pGroup  = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    CGexFileInGroup *   pFile   = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    CWaferMap           lWafer;

    if (pFile == NULL)
        return NoError;

    // Create a local copy of the wafermap
    lWafer = pFile->getWaferMapData();

    // Check if wafermap must be samed using old P8 format or new Million-die format...
    bool	bNewP8_Format=false;
    if(gex_max(lWafer.SizeX, lWafer.SizeY) > 127)
        bNewP8_Format = true;

    // Compute total pass/fail/dies
    int	iY,iX,iBinCode;
    int	iStartCol,iEndCol,iColStep;
    int	iStartLine,iEndLine,iLineStep;
    long	lTotalPass=0;
    long	lTotalFail=0;
    long	lTestTotal=0;
    for(iX=0;iX<lWafer.SizeX;iX++)
    {
        for(iY=0;iY<lWafer.SizeY;iY++)
        {
            iBinCode = lWafer.getWafMap()[(iX+(iY*lWafer.SizeX))].getBin();
            switch(iBinCode)
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

    // Build file name: <path>/<lot>-xx.dat with 'xx': <WaferID>
    QString strWaferFileName = getCustomWaferMapFilename();
    if(strWaferFileName.isEmpty()){//Legacy mode
        strWaferFileName = pFile->getMirDatas().szLot;	// LotID
        QString strWaferId(lWafer.szWaferID), strSubLotId(pFile->getMirDatas().szSubLot);
        if( (strWaferId.isEmpty()) && (strSubLotId.isEmpty()) )
        {
            QString strErrorMsg((char*)"WARNING : undefined wafer id and sublot id");
            GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
            GSLOG(SYSLOG_SEV_NOTICE, strErrorMsg.toLatin1().constData());
        }

        strWaferFileName += "-";
        if(!strWaferId.isEmpty())
            strWaferFileName += strWaferId;
        else
            strWaferFileName += strSubLotId;
    }
    strWaferFileName += ".dat";

    // Case 3898: clean up invalid characters (Win + unix)
    CGexSystemUtils::NormalizeFileName(strWaferFileName);

    // Full file path to create: <path>/<lot>-<wafer>.xml
    QFileInfo cFolder(strOutputWafermapPath);
    if(cFolder.isDir() == false)
        strWaferFileFullName = strOutputWafermapPath;	// Full file name+path already given in argument, keep it as-is
    else
        strWaferFileFullName = strOutputWafermapPath + "/" + strWaferFileName;

    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file
    if (file.open(QIODevice::WriteOnly) == false)
        return WriteError;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Check if PosX/PosY have to be overloaded
    if(iPosX == GS::Gex::PATProcessing::ePositiveForceLeft)
        lWafer.SetPosXDirection(false);
    else if(iPosX == GS::Gex::PATProcessing::ePositiveForceRight)
        lWafer.SetPosXDirection(true);
    else if(lWafer.cPos_X != 'L' && lWafer.cPos_X != 'l' &&
            lWafer.cPos_X != 'R' && lWafer.cPos_X != 'r')
    {
        if(iPosX == GS::Gex::PATProcessing::ePositiveLeft)
            lWafer.SetPosXDirection(false);
        else if(iPosX == GS::Gex::PATProcessing::ePositiveRight)
            lWafer.SetPosXDirection(true);
    }
    if(iPosY == GS::Gex::PATProcessing::ePositiveForceUp)
        lWafer.SetPosYDirection(false);
    else if(iPosY == GS::Gex::PATProcessing::ePositiveForceDown)
        lWafer.SetPosYDirection(true);
    else if(lWafer.cPos_Y != 'U' && lWafer.cPos_Y != 'u' &&
            lWafer.cPos_Y != 'D' && lWafer.cPos_Y != 'd')
    {
        if(iPosY == GS::Gex::PATProcessing::ePositiveUp)
            lWafer.SetPosYDirection(false);
        else if(iPosY == GS::Gex::PATProcessing::ePositiveDown)
            lWafer.SetPosYDirection(true);
    }

    // Rotate wafermap, unless default should be used for the notch
    if(iNotch <= 0)
        iNotch = lWafer.GetWaferNotch();
    else if(bRotateWafer)
    {
        while(lWafer.GetWaferNotch() != iNotch)
            lWafer.RotateWafer();
    }
    iWaferSizeX = lWafer.SizeX;
    iWaferSizeY = lWafer.SizeY;

    // Check for X direction
    if(lWafer.GetPosXDirection() == true)
    {
        // X direction = 'R' (right)
        iStartCol = 0;
        iEndCol = iWaferSizeX;
        iColStep = 1;
    }
    else
    {
        // X direction = 'L' (left)
        iStartCol = iWaferSizeX-1;
        iEndCol = -1;
        iColStep = -1;
    }
    // Check for Y direction
    if(lWafer.GetPosYDirection() == true)
    {
        // Y direction = 'D' (down)
        iStartLine = 0;
        iEndLine = iWaferSizeY;
        iLineStep = 1;
    }
    else
    {
        // Y direction = 'U' (up)
        iStartLine = iWaferSizeY-1;
        iEndLine = -1;
        iLineStep = -1;
    }

    QString	strString;
    char	szString[256];
    unsigned char uLsb,uMsb;
    QDateTime	cDateTime;
    if(bNewP8_Format)
    {
        // TEL-P8 Million-die format
        #ifdef GCORE15334
        CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
        if (lPatInfo)
        {
            // Total Pass
            sprintf(szString,"%011ld",lTotalPass - lPatInfo->m_lstOutlierParts.count());
            hWafermapFile << szString;

            // Total Fail
            sprintf(szString,"%011ld",lTotalFail + lPatInfo->m_lstOutlierParts.count());
            hWafermapFile << szString;
        }
        #endif

        // Total tested
        sprintf(szString,"%011ld",lTestTotal);
        hWafermapFile << szString;

        // Wafer starting time (BCD 12 bytes)
        // eg: 0 5 1 2 1 2  0 9 0 4 0 2
        //     Y Y M M D D  H H M M S S
        if(lWafer.lWaferStartTime > 0)
            cDateTime.setTime_t(lWafer.lWaferStartTime);
        else
            cDateTime = QDateTime::currentDateTime();
        strString = cDateTime.toString("yyMMddhhmmss");
        hWafermapFile << strString;

        // Wafer ending time (BCD 12 bytes)
        if(lWafer.lWaferEndTime > 0)
            cDateTime.setTime_t(lWafer.lWaferEndTime);
        else
        {
            // Valid Start time?...if so use it, otherwise use current time!
            if(lWafer.lWaferStartTime > 0)
                cDateTime.setTime_t(lWafer.lWaferStartTime);
            else
                cDateTime = QDateTime::currentDateTime();
        }
        strString = cDateTime.toString("yyMMddhhmmss");
        hWafermapFile << strString;

        // Wafer-ID (36 characters)
        strString.sprintf("%36s",lWafer.szWaferID);
        hWafermapFile << szString;

        // Wafer# (2 chars.)
        strString.sprintf("%2s",lWafer.szWaferID);
        hWafermapFile << szString;

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
        uLsb = (iWaferSizeY) & 0xff;
        uMsb = (iWaferSizeY >> 8) & 0xff;

        hWafermapFile << uLsb;	// LSB
        hWafermapFile << uMsb;	// MSB

        BYTE	ucFlag;
        unsigned long ulBin;

        // Wafermap
        for(iY = iStartLine; iY != iEndLine;)
        {
            // Location of first die in X (2 bytes) - LSB, MSB
            hWafermapFile << 0 << 0;

            // Dies in row
            uLsb = (iWaferSizeY) & 0xff;
            uMsb = (iWaferSizeY >> 8) & 0xff;
            hWafermapFile << uLsb;	// LSB
            hWafermapFile << uMsb;	// MSB

            // Processing a wafer line.
            for(iX = iStartCol; iX != iEndCol;)
            {
                // Read Die binning
                iBinCode = lWafer.getWafMap()[(iX+(iY*iWaferSizeX))].getBin();

                // If die tested
                switch(iBinCode)
                {
                    case GEX_WAFMAP_EMPTY_CELL:
                        ucFlag = 0x80;	// Set 'No test' flag
                        ulBin=0;
                        break;

                    case 1:		// Good die
                        ucFlag = 0x0;
                        ulBin = 2;
                        break;

                    default:	// Failing die: Force inking, set fail flag
                        if(iBinCode > 31)
                            iBinCode = 31;
                        ucFlag = 0x1;	// Set 'FAIL' flag
                        ulBin = 1UL << iBinCode;
                        break;
                }

                // Bin: 32 bits
                hWafermapFile << (BYTE) (ulBin & 0xff);
                hWafermapFile << (BYTE) ((ulBin >> 8) & 0xff);
                hWafermapFile << (BYTE) ((ulBin >> 16) & 0xff);
                hWafermapFile << (BYTE) ((ulBin >> 24) & 0xff);

                // Byte#1: B7= No Test, B5= Skip, B4= Force inking, B1= Special specs, B0= Fail
                // B6,B3,B2 = unused
                hWafermapFile << ucFlag;

                // Next column
                iX += iColStep;
            }

            // Next line
            iY += iLineStep;
        }
    }
    else
    {
        // TEL-P8 old format (less than 128 dies in X and Y)

        // Lot-ID (25 characters)
        QString	strString;
        strString.sprintf("%25s",pFile->getMirDatas().szLot);
        hWafermapFile << strString;

        // Wafer# (2 chars.)
        strString.sprintf("%2s",lWafer.szWaferID);
        hWafermapFile << strString;

        // Cassette# (1 char)
        hWafermapFile << "1";

        // Slot# (2 chars)
        hWafermapFile << "25";

        // Test count (1 char)
        hWafermapFile << "1";

        // Total pass (2 bytes): LSB, MSB
        unsigned char	uLsb,uMsb;
        uMsb = lTotalPass / 256;
        uLsb = lTotalPass % 256;
        hWafermapFile << uLsb << uMsb;

        // Total Fail (2 bytes): LSB, MSB
        uMsb = lTotalFail / 256;
        uLsb = lTotalFail % 256;
        hWafermapFile << uLsb << uMsb;

        // Total tests (2 bytes): LSB, MSB
        uMsb = (lTotalPass+lTotalFail) / 256;
        uLsb = (lTotalPass+lTotalFail) % 256;
        hWafermapFile << uLsb << uMsb;

        // Wafer starting time (BCD 12 bytes)
        // eg: 0 5 1 2 1 2  0 9 0 4 0 2
        //     Y Y M M D D  H H M M S S
        strString = strBuildTimeBCD(lWafer.lWaferStartTime);
        hWafermapFile << strString;

        // Wafer ending time (BCD 12 bytes)
        strString = strBuildTimeBCD(lWafer.lWaferEndTime);
        hWafermapFile << strString;

        // Number of records (1 byte).
        // Each record holds one line of dies (ie: having same X value)
        // Up to 127 dies allowed per X record (127*127 = 16129 dies max. supported per wafer)
        hWafermapFile << (BYTE) (iWaferSizeY);

        // X,Y Distance from origin (4 bytes)
        hWafermapFile << 0 << 0 << 0 << 0;

        BYTE	ucFlag;
        for(iY = iStartLine; iY != iEndLine;)
        {
            // Location of first die in X (2 bytes)
            hWafermapFile << 0 << 0;

            // Location of first die in Y (2 bytes)
            hWafermapFile << 0 << 0;

            // Write Total dies in this record (1 byte). 127 dies max. allowed per record.
            hWafermapFile << (BYTE) (iWaferSizeX & 0xFF);

            // Processing a wafer line.
            for(iX = iStartCol; iX != iEndCol;)
            {
                // Read Die binning
                iBinCode = lWafer.getWafMap()[(iX+(iY*iWaferSizeX))].getBin();

                // If die tested
                switch(iBinCode)
                {
                    case GEX_WAFMAP_EMPTY_CELL:
                        ucFlag = 0x80;	// Set 'No test' flag
                        break;

                    case 1:		// Good die
                        ucFlag = 0x0;
                        break;

                    default:	// Failing die: Force inking, set fail flag
                        ucFlag = 0x11;
                        break;
                }

                // Byte#1: B7= No Test, B5= Skip, B4= Force inking, B1= Special specs, B0= Fail
                // B6,B3,B2 = unused
                hWafermapFile << ucFlag;

                // Byte#2: Binning
                hWafermapFile << (BYTE) (iBinCode & 0xFF);

                // Next column
                iX += iColStep;
            }

            // Next line
            iY += iLineStep;
        }
    }

    file.close();
    return NoError;
}

bool WafermapExport::initScriptEngineProperties(GexScriptEngine*	poScriptEngine){

    if (!poScriptEngine)
        return false;

    CGexGroupOfFiles *pGroup = 0;
    CGexFileInGroup *pFile = 0;
    QString strProductId, strLotId, strWaferId, strSubLotId, strProgName,
            strTesterName, strTesterType, strFacilityId, strPackageType,
            strFamilyId;

    strProductId = strLotId = strWaferId = strSubLotId = strProgName =
            strTesterName = strTesterType = strFacilityId = strPackageType =
            strFamilyId = "";

    foreach(pGroup, gexReport->getGroupsList())
    {
        if(pGroup)
        {
            foreach(pFile, pGroup->pFilesList)
            {
                if (pFile)
                {
                    // Set product ID
                    if (!strProductId.isEmpty() && strProductId != QString(pFile->getMirDatas().szPartType))
                        strProductId = "MultiProduct";
                    else
                        strProductId = QString(pFile->getMirDatas().szPartType);

                    // Set lot ID
                    if (!strLotId.isEmpty() && strLotId != QString(pFile->getMirDatas().szLot))
                        strLotId= "MultiLot";
                    else
                        strLotId = QString(pFile->getMirDatas().szLot);

                    // Set wafer ID
                    if (!strWaferId.isEmpty() && strWaferId != QString(pFile->getWaferMapData().szWaferID))
                        strWaferId = "MultiWafer";
                    else
                        strWaferId = pFile->getWaferMapData().szWaferID;

                    // Set sublot ID
                    if (!strSubLotId.isEmpty() && strSubLotId != QString(pFile->getMirDatas().szSubLot))
                        strSubLotId = "MultiSubLot";
                    else
                        strSubLotId = QString(pFile->getMirDatas().szSubLot);

                    // Set programme name
                    if (!strProgName.isEmpty() && strProgName != QString(pFile->getMirDatas().szJobName))
                        strProgName = "MultiProgramme";
                    else
                        strProgName = QString(pFile->getMirDatas().szJobName);
                    // Set tester name
                    if (!strTesterName.isEmpty() && strTesterName != QString(pFile->getMirDatas().szNodeName))
                        strTesterName = "MultiTesterName";
                    else
                        strTesterName = QString(pFile->getMirDatas().szNodeName);

                    // Set tester type
                    if (!strTesterType.isEmpty() && strTesterType != QString(pFile->getMirDatas().szTesterType))
                        strTesterType = "MultiTesterType";
                    else
                        strTesterType = QString(pFile->getMirDatas().szTesterType);

                    // Set facility ID
                    if (!strFacilityId.isEmpty() && strFacilityId != QString(pFile->getMirDatas().szFacilityID))
                        strFacilityId = "MultiFacility";
                    else
                        strFacilityId = QString(pFile->getMirDatas().szFacilityID);

                    // Set package type
                    if (!strPackageType.isEmpty() && strPackageType != QString(pFile->getMirDatas().szPkgType))
                        strPackageType = "MultiPackage";
                    else
                        strPackageType = QString(pFile->getMirDatas().szPkgType);

                    // Set family ID
                    if (!strFamilyId.isEmpty() && strFamilyId != QString(pFile->getMirDatas().szFamilyID))
                        strFamilyId = "MultiFamiliy";
                    else
                        strFamilyId = QString(pFile->getMirDatas().szFamilyID);
                }
            }
        }
    }

    // Set product ID
    poScriptEngine->globalObject().setProperty("$PRODUCT_ID",strProductId);
    // Set lot ID
    poScriptEngine->globalObject().setProperty("$LOT_ID",strLotId);
    // Set wafer ID
    poScriptEngine->globalObject().setProperty("$WAFER_ID",strWaferId);
    // Set sublot ID
    poScriptEngine->globalObject().setProperty("$SUBLOT_ID",strSubLotId);
    // Set programme name
    poScriptEngine->globalObject().setProperty("$PROGRAM_NAME",strProgName);
    // Set tester name
    poScriptEngine->globalObject().setProperty("$TESTER_NAME",strTesterName);
    // Set tester type
    poScriptEngine->globalObject().setProperty("$TESTER_TYPE",strTesterType);
    // Set facility ID
    poScriptEngine->globalObject().setProperty("$FACILITY_ID",strFacilityId);
    // Set package type
    poScriptEngine->globalObject().setProperty("$PACKAGE_TYPE",strPackageType);
    // Set family ID
    poScriptEngine->globalObject().setProperty("$FAMILY_ID",strFamilyId);
    return true;
}

QString WafermapExport::getCustomWaferMapFilename()
{
    QString strFileNameJSExpr = ReportOptions.GetOption("wafer", "wafer_file_name").toString();
    if(strFileNameJSExpr.isEmpty() || !pGexScriptEngine)
        return QString();

    if(!initScriptEngineProperties(pGexScriptEngine))
        return QString();

    QString strFileName = "";
    if (pGexScriptEngine->canEvaluate(strFileNameJSExpr))
        strFileName = pGexScriptEngine->evaluate(strFileNameJSExpr).toString();

    if (pGexScriptEngine->hasUncaughtException())
    {
        QString strErrorMsg = "Error evaluating: " + strFileNameJSExpr;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
        return QString();
    }

    // Remove windows forbiden char
    strFileName.remove(QRegExp("[\\/:*?<>|]"));
    // Begins with space,-,dot
    strFileName.remove(QRegExp("^[ -.]+"));
    // Ends with space,-,dot
    strFileName.remove(QRegExp("[ -.]+$"));

    return strFileName;
}



