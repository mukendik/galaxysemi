//////////////////////////////////////////////////////////////////////
// import_all.cpp: Converts input data file (WAT,PCM,ATDF,CSV,...) into STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <QLabel>

#include "engine.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "report_build.h"
#include "report_options.h"
#include "import_all.h"
#include "import_csv.h"                 // CSV to STDF converter
#include "import_pcm.h"                 // PCM_TSMC to STDF converter
#include "import_pcm_hynix.h"           // PCM_Hynix to STDF converter
#include "import_pcm_magnachip.h"       // PCM Magnachip (ex Hynix) KDF summary format
#include "import_pcm_jazz.h"            // WAT from JAZZ foundry to STDF converter
#include "import_csm.h"                 // CSM to STDF converter
#include "import_csm_type_2.h"          // CSM type 2 to STDF converter
#include "import_csmc.h"                // CSMC to STDF converter
#include "import_wat.h"                 // WAT_UMC to STDF converter
#include "import_gdf.h"                 // GDF to STDF converter
#include "import_spd.h"                 // SPD (TMT Credence) to STDF converter
#include "import_dl4.h"                 // DL4 (ASL Credence old format) to STDF converter
#include "import_teststand.h"           // National Instruments TestStand data format
#include "import_spektra.h"             // Tesec data format to STDF converter
#include "import_7c7.h"                 // 7C7 data format from Rosettanet to STDF converter
#include "import_semi_g85_xml.h"        // SemiG85Xml data format from Analog Devices to STDF converter
#include "import_klarf.h"               // KLA-TENCOR Defect Analysis Data to STDF converter
#include "import_kla_wafer_xml.h"       // KLA-WAFER-XML data format to STDF converter
#include "import_prober_tsmc.h"         // PROBER TSMC to STDF converter
#include "import_teradyne_image.h"      // Teradyne Image Data to STDF converter
#include "import_pcm_x_fab.h"           // X-Fab - PCM Data to STDF converter
#include "import_wat_asmc.h"            // WAT-ASMC Data to STDF converter
#include "import_asl1000.h"             // ASL1000 TMT Data to STDF converter
#include "import_eagle_datalog.h"       // Eagle Datalog to STDF converter
#include "import_pcm_magnachip_type_2.h"// PCM Magnachip type2 format
#include "import_tango.h"               // Advanced Silicon TANGO data .XML format
#include "import_yokogawa.h"            // Yokogawa data .dat format
//#include "import_pcm_dongbu.h"          // Yokogawa data .dat format
#include "import_advantest_t2000.h"     // Advantest T2000 Data to STDF converter
#include "import_verigy_edl.h"          // Verigy EDL data to STDF converter
#include "import_verigy_edf.h"          // Verigy EDF ASCII data to STDF converter
#include "import_tessera.h"             // Tessera data to STDF converter
#include "import_tessera_inspection.h"  // Tessera data to STDF converter
#include "import_tessera_qv302.h"       // Tessera data to STDF converter
#include "import_tessera_diffractive.h" // Tessera data to STDF converter
#include "import_tessera_microlupi.h"   // Tessera data to STDF converter
#include "import_tessera_evg40.h"       // Tessera data to STDF converter
#include "import_tessera_wavemaster.h"  // Tessera data to STDF converter
#include "import_semi_e142_xml.h"       // SemiG85Xml data format from Analog Devices to STDF converter
#include "import_spea_datalog.h"        // SpeaDatalog data to STDF converter
#include "import_spil_ws.h"             // SpilWs Wafer data format from Analog Devices to STDF converter
#include "import_laurier_die_sort.h"    // Laurier Die Sort 1D data format to STDF converter
#include "importSipex.h"                // Sipex ETest data format to STDF converter
#include "import_pcm_gsmc.h"            // Gsmc - PCM Data to STDF converter
#include "import_pcm_silan.h"           // Silan - PCM Data to STDF converter
#include "import_prober_tel.h"          // Tel - WAFER PROBER Data to STDF converter
#include "import_prober_tsk.h"          // Tsk - WAFER PROBER Data to STDF converter
#include "import_pcm_exar.h"            // Exar - PCM Data to STDF converter
#include "import_amida.h"               // Amida - TTL/GPIB Data to STDF converter
#include "import_acco.h"                // Acco - S8107/S8200 Data to STDF converter
#include "import_tsmc_wpr.h"            // WPR TSMC to STDF converter
#include "import_kvd.h"                 // KVD - WAFER PROBER to STDF converter
#include "import_mcube.h"               // MCUBE - E320 datalog to STDF converter
#include "import_nd1nd2_datalog.h"      // ND1/ND2 datalog to STDF converter
#include "import_kla_inf_layers.h"      // KLA/INF file to STDF converter
#include "import_tessera_yoda.h"        // Yoda-Imagemaster file to STDF converter
#include "import_wif.h"                 // WIF file to STDF converter
#include "import_pcm_hjtc.h"            // PCM Hejian Technology to STDF converter
#include "import_fresco_sum.h"          // FRESCO SUMMARY data to STDF converter
#include "import_shibasoku_dlk.h"       // SHIBASOKU DLK data to STDF converter
#include "import_etest_fab7.h"          // ETEST FAB7 data to STDF converter
#include "import_stif.h"                // ST inkless format to STDF converter
#include "import_sdi.h"                 // Silicon Data Integration format to STDF converter
#include "import_freescale_maxwell.h"   // Freescale Maxwell format to STDF converter
#include "import_kvd_xml_to_stdf.h"
#include "import_93ktab.h"
#include "import_pcm_magnachip_column.h"// PCM Magnachip fixed column format
#include "import_pcm_tower.h"           // PCM Tower CSV format
#include "import_csmc_spdm.h"           // CSMC_SPDM format
#include "import_stdf_v3.h"
#include "import_teradyne_ascii.h"      // TERADYNE_ASCII
#include "export_ascii.h"               // needed for GALAXYSEMI_ASCII_HEADER
#include "import_pcm_cypress.h"         // csv ETest Cypress
#include "import_pcm_hjtc.h"
#include "importSpinstand.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "converter_external_file.h"

#include "parserFactory.h"
#include "parserAbstract.h"

// Support of Compressed files. Those includes should be placed after the above ones,
// to avoid re-definition warnings
#include <gqtl_archivefile.h>
#include "temporary_files_manager.h"

// List of valid extensions allowed when selecting a file
#define	GEX_VALID_DATAFILES_TYPES   "Data Files (*.*);;\
Advantest T2000 (*.dl *.txt);;\
Acco - S8107/S8200 Data (*.csv);;\
Amida - TTL/GPIB Data (*.log);;\
ASE Subcon FT (*.csv *.log2 *.sum);;\
ASL Credence tester (*.dl4);;\
ASMC - WAT data (*.prt *.prn *.txt *.csv *.wat);;\
ATDF (*.atd *.atdf);;\
AVI - Automated Visual Inspection (*.dat);;\
CSMC, Chartered Semi. - Etest data (*.txt);;\
Compressed file (*.zip *.gz *.Z);;\
CSV spreadsheet (*.csv);;\
Cypress E-test data (*.csv);;\
Dongbu Electronics - PCM Data (*.csv);;\
Eagle Datalog - ETS Data (*.log *.txt *.csv);;\
Etest Fab7 - WAT Data (*.csv);;\
EXAR - PCM data (*.csv *.txt);;\
FetTest Data (*.dat);;\
FetTest Summary data (*.csv);;\
Freescale Maxwell Data (*.csv);;\
FrescoSummary Data (*.csv);;\
GDF Agilent HP93K/HP83K (*.gdf *.gdf_0 *.gdf_1);;\
GSMC - PCM data (*.csv *.txt);;\
Hejian Technology - PCM data (*.csv);;\
HVM Lot Summary data (*.csv);;\
Hynix - PCM data (*.pcm *.csv *.txt);;\
JAZZ - Etest data (*.csv);;\
KLA/INF wafer map file (*.inf);;\
KLA/INF layers file (*.dat);;\
KLARF wafer defects file (*.txt *.krf *.klarf);;\
KLAR Xml wafer defects file (*.xml);;\
KVD - WAFER data (*.log);;\
LAURIER Die Sort 1D file (*.col);;\
MagnaChip - PCM data (*.pcm *.csv *.txt *.dat);;\
Mcube - E320 datalog (*.log *.csv);;\
ND1/ND2 datalog (*.log *.datalog);;\
Prober data - TSMC format (*.tsm);;\
Prober data - TEL/TSK format (*.dat);;\
SDI - Silicon Data Integration (*.sdi *.out);;\
SEMI E142 Wafer map (*.xml);;\
SEMI G85 Wafer map (*.00? *.01? *.02? *.dat *.map *.xml);;\
SHIBASOKU - DLK data (*.dlk);;\
SILAN - PCM data (*.csv *.txt);;\
SIPEX - ETest data (*.csv);;\
SMIC - WAT data (*.csv *.dat *.txt);;\
SPEA - Datalog data (*.log);;\
SPEKTRA Lot Summary files (*.csv);;\
SPEKTRA Datalog files (*.csv);;\
SPIL - WAFER data (*.map);;\
SPINSTAND data (*.025);;\
STDF (*.std *.stdf);;\
STIF - ST inkless format (*.dat);;\
TANGO Data files (*.xml);;\
Teradyne Image (*.data);;\
Tesec files (*.dta);;\
Tessera files (*.dat *.csv *.txt *.pts);;\
Text raw data(*.txt *.dat);;\
TMT ASL1000 (*.dat);;\
TMT Credence tester (*.spd);;\
TOWER - PCM data (*.csv *.txt);;\
TSM - WAT data (*.tsl);;\
TSMC - WAT data (*.wat);;\
TSMC - WPR data (*.wpr);;\
UMC - PCM data (*.pcm *.csv *.txt);;\
UMC - WAT data (*.csv);;\
Verigy EDF Data (*.edl *.edf *.in);;\
WIF Data (*.txt);;\
SKYNP - X-Fab - PCM data (*.pcm *.csv *.txt);;\
Yokogawa data (*.dat *.log_a);;\
7C7 RosettaNet files (*.xml);;\
SkyWorks NP (*.csv);;\
SkyWorks IFF (*bm *pcmd *pcme *prbd *ftpi);;\
TriQuint RF (*.txt);;\
TriQuint DC (*.txt);;\
Micron Wafer prober(*.zip);;\
All Files (*.*);;"


#if 0
// For Sequoia, uncomment following line and add to above list in right place, and with the '\' at the end
// Sequoia data (*.csv);;
#endif

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
#if MODULE == GEX
#include <QFileDialog>
#endif

// in report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

namespace GS
{
namespace Gex
{

#if MODULE == GEX
//////////////////////////////////////////////////////////////////////
// Select one file from disk
//////////////////////////////////////////////////////////////////////
QString SelectDataFiles::GetSingleFile(QWidget * parent,
                                       const QString &strPath, const QString& strCaption)
{
    return QFileDialog::getOpenFileName(parent, strCaption, strPath,
                                        ConvertToSTDF::GetListOfSupportedFormat(true).join(";;"));
}

//////////////////////////////////////////////////////////////////////
// Select multiple files from disk
//////////////////////////////////////////////////////////////////////
QStringList SelectDataFiles::GetFiles(QWidget * parent, const QString &strPath,
                                      const QString& strCaption)
{
    return QFileDialog::getOpenFileNames(parent, strCaption, strPath, ConvertToSTDF::GetListOfSupportedFormat(true).join(";;"));
}
#endif

ProgressBarHandler::ProgressBarHandler()
{
    mMinValue = 0;
    mMaxValue = 0;
}

void ProgressBarHandler::Start(const long minValue, const long maxValue)
{
    mMaxValue = maxValue;
    mMinValue = minValue;
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true, maxValue, 1);
    QCoreApplication::processEvents();
}

void ProgressBarHandler::Increment()
{
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(true, -1, -1);
    QCoreApplication::processEvents();
}


void ProgressBarHandler::SetValue(const long value)
{
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false, 100, value);
    QCoreApplication::processEvents();
}

void ProgressBarHandler::Finish()
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus();
    GS::Gex::Engine::GetInstance().HideProgress();
    QCoreApplication::processEvents();
}

void ProgressBarHandler::SetMessage(const std::string message)
{

    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(message.c_str()));
    QCoreApplication::processEvents();
}


//////////////////////////////////////////////////////////////////////
// Check where intermediate STDF file has to be created...update path accordingly
//////////////////////////////////////////////////////////////////////
static void ValidateIntermediateDataPath(QString &strFileNameSTDF)
{
    // Check where to unzip files: custom folder or same as data file.
    QString strOutputPath;

    // Get custom path if defined
    if (ReportOptions.GetOption("dataprocessing", "stdf_intermediate").toString() == "custom")
        strOutputPath =  ReportOptions.GetOption("dataprocessing", "stdf_intermediate_path").toString();

    if(QFile::exists(strOutputPath) == false)
        return;	// Keep default intermediate data file name

    // Custom output path for Intermediate files exist, then use it!
    QFileInfo cFileInfo(strFileNameSTDF.toLatin1().constData());
    cFileInfo.setFile(strOutputPath,cFileInfo.fileName());
    strFileNameSTDF = cFileInfo.filePath();
}

ConvertToSTDF::ConvertToSTDF(QObject* parent):QObject(parent)
{
    //this->setParent(parent);
}

ConvertToSTDF::ConvertToSTDF(const ConvertToSTDF& o):QObject(o.parent())
{
    // nothing to do because no member!
}

int ConvertToSTDF::Convert(bool bMonitoring, bool bAllowWizard, bool bDatabaseAccessMode, bool bAllowExtendedCsv,
                           QString strFileName, QString &strFileNameSTDF, QString strNewExtension,
                           bool &bFileCreated, QString &strMessage,
                           bool bAllowOnlyOneFile, bool bAllowDestFolderOverwriteFromOptions/*=true*/)
{
    QStringList lstFileNameSTDF;
    if(!strFileNameSTDF.isEmpty())
        lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());

    int nStatus = Convert(bMonitoring, bAllowWizard, bDatabaseAccessMode, bAllowExtendedCsv,
                          strFileName, lstFileNameSTDF, strNewExtension, bFileCreated, strMessage,
                          bAllowOnlyOneFile, bAllowDestFolderOverwriteFromOptions);
    strFileNameSTDF = lstFileNameSTDF.first();

    return nStatus;
}

QString ConvertToSTDF::Convert(const QString &lInput, const QString &lOutput)
{
    QStringList lstFileNameSTDF;
    if(!lOutput.isEmpty())
        lstFileNameSTDF.append(lOutput.toLatin1().constData());

    bool bFileCreated=false;
    QString strMessage;
    int nStatus = Convert(false, false, true, true,
                          lInput, lstFileNameSTDF, "", bFileCreated, strMessage,
                          false, true);
    if (nStatus==eConvertSuccess)
        return "ok";
    if (nStatus==eConvertWarning)
        return "ok but "+strMessage;
    if (nStatus==eConvertDelay)
        return "delay:"+strMessage;
    if (nStatus==eConvertError)
        return "error:"+strMessage;
    return "error: unknown status";
}

QString ConvertToSTDF::IsGalaxySemiTxtFormat(const QString &f)
{
    if (!QFile::exists(f))
        return "error : file does not exist";
    QFile fi(f);
    if (!fi.open(QIODevice::ReadOnly))
        return "error : file cant be opened";

    QTextStream ts(&fi);
    QString s=ts.readLine();
    fi.close();

    if (s.startsWith(GALAXYSEMI_ASCII_HEADER))
    {
        return "yes";
    }

    return "no";
}

QStringList ConvertToSTDF::GetListOfSupportedFormat(bool bWithFormatDescription)
{
    QStringList lstSupportedFormat = QString(GEX_VALID_DATAFILES_TYPES).split(";;");
    QStringList lstSupportedExt;

    if(bWithFormatDescription)
        return lstSupportedFormat;

    foreach(const QString &strFormat, lstSupportedFormat)
    {
        lstSupportedExt += strFormat.section("(",1).section(")",0,0).split(" ");
    }
    // Remove *.*
    lstSupportedExt.removeAll("*.*");
    lstSupportedExt.removeAll("");

    // Add compressed format
    lstSupportedExt += QString("*.zip;*.gz;*.tar;*.Z").split(";");

    // Remove all duplicated entries
    lstSupportedExt.removeDuplicates();

    // Sort
    lstSupportedExt.sort();

    return lstSupportedExt;
}

#define MSG "Your license is restricted to only process files from a specific tester brand or type.\nTo remove this limitation, you need to upgrade your license.\n\nPlease contact " GEX_EMAIL_SALES


int
ConvertToSTDF::Convert(bool bMonitoring,
                       bool bAllowWizard,
                       bool bDatabaseAccessMode,
                       bool bAllowExtendedCsv,
                       QString strFileName,
                       QStringList &lstFileNameSTDF,
                       QString strNewExtension,
                       bool &bFileCreated,
                       QString &strMessage,
                       bool bAllowOnlyOneFile,
                       bool /*bAllowDestFolderOverwriteFromOptions = true*/)
{
    QFileInfo cFileInfo(strFileName);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Convert '%1'...").arg( strFileName).toLatin1().constData());
    int			nConvertStatus, nStatus;
    bool		bResult=false;
    bool		bOnlyAcceptSTDF=false;	// Set to true for the OEM versions.
    bool		bAcceptDL4=false;		// Credence-OEM must accept DL4 files on top of Credence's STDF!
    bool		bCompressedFile=false;
    bool        lConvertionWithExternalFile = false;
    QString		strTemp;
    QString		strFileNameSTDF;
    QString		r;

    if(lstFileNameSTDF.isEmpty())
        strFileNameSTDF = strFileName+strNewExtension;
    else
        strFileNameSTDF = lstFileNameSTDF.first();

    // Check if the destination file was diff to original file
    if(strFileName == strFileNameSTDF.toLatin1().constData())
        strFileNameSTDF += strNewExtension;
    // If no extension defined, add standard ext
    if(strFileName == strFileNameSTDF.toLatin1().constData())
        strFileNameSTDF += GEX_TEMPORARY_STDF;


    lstFileNameSTDF.clear();

    // Debug message
    strTemp =	"Converting: In=" + strFileName;
    strTemp +=	", Out=" + strFileNameSTDF;
    strTemp.replace('%', "%%");
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strTemp.toLatin1().data() );

    // Init some variables
    bFileCreated = false;
    nConvertStatus = eConvertError;
    strMessage = "File corrupted or unknown format or low memory.\nIf you want further help then please send your file \nto Quantix support at "+QString(GEX_EMAIL_SUPPORT);;

    // No extension...then suppose it is a STDF file.
    QString strExtension = cFileInfo.suffix().toLower();
    //	if(strExtension.isEmpty())
    //		return eConvertSuccess;

    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
    case GS::LPPlugin::LicenseProvider::eLtxcOEM :       // OEM-Examinator for LTXC
        bOnlyAcceptSTDF = true;
        bAcceptDL4 = true;
        break;

    case GS::LPPlugin::LicenseProvider::eSzOEM:			// OEM-Examinator for Credence SZ
        bOnlyAcceptSTDF = true;
        break;

    default:
        bAcceptDL4 = true;
    }

    // Extract path from Test data file
    QString strFilePath = cFileInfo.absolutePath();

    // Create the parser variables
    GS::Parser::ParserFactory *lParserFactory = GS::Parser::ParserFactory::GetInstance();
    GS::Parser::ConverterStatus lStatus = GS::Parser::ConvertError;
    std::string lParameterFolder = (GS::Gex::Engine::GetInstance().Get("UserFolder").toString())
            .toLatin1().constData();

    // Check if compressed file...if so, uncompress it first!
    // Check if it is a compressed file....
    CArchiveFile	clZipCheck(strFilePath);

    QFile lOFile(strFileName);
    // Check if the file exists and if it is readable
    if(!QFile::exists(strFileName))
    {
        strMessage = "Convert file: File doesn't exist:\n" + strFileName;
        nConvertStatus = eConvertError;
        goto labelEndOfProcess;
    }

    if(!lOFile.open(QIODevice::ReadOnly))
    {
        strMessage = "Convert file: File cannot be opened:\n" + strFileName;
        nConvertStatus = eConvertError;
        goto labelEndOfProcess;
    }
    lOFile.close();

    if((!GS::LPPlugin::ProductInfo::getInstance()->isOEM())
            &&clZipCheck.IsCompressedFile(strFileName))
    {
        // First: Check if the file match with an existing parser
        if(lParserFactory != NULL)
        {
            QScopedPointer<GS::Parser::ParserAbstract> lParser(lParserFactory->CreateParser(strFileName.toLatin1().constData()));

            if(lParser.isNull() == false)
            {
                ProgressBarHandler lHandler;

                lParser->SetProgressHandler(&lHandler);
                lParser->SetParameterFolder(lParameterFolder);
                std::string lFileNameSTDF = strFileNameSTDF.toStdString();
                lStatus = lParser->Convert(strFileName.toStdString(), lFileNameSTDF);

                if (lStatus == GS::Parser::ConvertSuccess)
                {
                    nConvertStatus = eConvertSuccess;
                    lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
                    bFileCreated = true;
                }
                else
                {
                    std::string lErrorMsg;
                    lParser->GetLastError(lErrorMsg);
                    strMessage = lErrorMsg.c_str();
                }

                goto labelEndOfProcess;
            }
        }

        // Check if allowed to manage advanced formats...
        if((bAllowWizard == false) && (bMonitoring == false))
        {
            strMessage ="Your version of the product does not support compressed files\nIf you would like to upgrade your license, please contact quantix_sales@mentor.com for more information.";
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // Check where to unzip files: custom folder or same as data file.
        QString strUnzipFolder;
        // Get custom path if defined
        if (ReportOptions.GetOption("dataprocessing", "stdf_intermediate").toString() == "custom")
        {
            strUnzipFolder =  ReportOptions.GetOption("dataprocessing", "stdf_intermediate_path").toString();
            if(!strUnzipFolder.isEmpty())
            {
                if(!QFile::exists(strUnzipFolder))
                {
                    QDir lDir;
                    // If option defined, create the path
                    if(!lDir.mkpath(strUnzipFolder))
                    {
                        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to create custom path %1").arg(strUnzipFolder).toLatin1().constData());
                        strUnzipFolder = "";
                    }
                }
            }
        }
        // Get the output file
        if(strUnzipFolder.isEmpty())
        {
            // Try to use the OutPut folder defined
            strUnzipFolder = QFileInfo(strFileNameSTDF.toLatin1().constData()).absolutePath();
            if(!QFile::exists(strUnzipFolder))
            {
                QDir lDir;
                // If option defined, create the path
                if(!lDir.mkpath(strUnzipFolder))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Unable to create default path %1").arg(strUnzipFolder).toLatin1().constData());
                    strUnzipFolder = "";
                }
            }
        }
        // If custom path doesn't exist or is not defined, use the default path (same as the data file)
        if(strUnzipFolder.isEmpty())
            strUnzipFolder = strFilePath;	// Default path:same as data file

        // Check if compressed file...if so, uncompress it first!
        // Check if it is a compressed file....
        CArchiveFile	clZip(strUnzipFolder);

        GSLOG(SYSLOG_SEV_DEBUG, QString("Unzip file in path %1").arg(strUnzipFolder).toLatin1().constData());

        // Uncompress file...
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateStatusMessage(" Unzipping data...");

        QStringList		strUncompressedFiles;
        QString         strZipError;

        bResult = clZip.Uncompress(strFileName, strUncompressedFiles);
        if(bResult == false)
        {
            strMessage = "Import Compressed file: Failed to uncompress data file:\n" + strFileName;
            strZipError = clZip.GetLastErrorMsg();
            if(!strZipError.isEmpty())
                strMessage += "\n\n" + strZipError;

            // Error uncompressing
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        if(strUncompressedFiles.count() > 1)
        {
            strMessage = "Import Compressed file: Too many files in compressed file, only one file per compressed file supported:\n" + strFileName;
            // Error uncompressing
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        if(strUncompressedFiles.count() != 1)
        {
            strMessage = "Import Compressed file: No file in compressed file:\n" + strFileName;
            // Error uncompressing
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // Build full path to zip file.
        bCompressedFile = true;
        cFileInfo.setFile(strUnzipFolder,strUncompressedFiles.first());
        strFileName = cFileInfo.filePath();

        // If compressed data are STDF files, append custom extension so they are deleted on Examinator exit.
        if(GS::StdLib::Stdf::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Append custom extension so we can erase this STDF temporary file later on...
            strFileNameSTDF = strFileName;
            strFileNameSTDF += ".gextb.std";
            QDir cDir;
            cDir.rename(strFileName,strFileNameSTDF.toLatin1().constData());
            bFileCreated = true;

            // Add file to list of temporary files to erase on Examinator exit...
            GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strFileNameSTDF.toLatin1().constData(), TemporaryFile::BasicCheck);

            nConvertStatus = eConvertSuccess;
            goto labelEndOfProcess;
        }

        // Add file to list of temporary files to erase on Examinator exit...
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strFileName, TemporaryFile::BasicCheck);
    }

    if(cFileInfo.size() == 0)
    {
        strMessage = "";
        if(bCompressedFile)
            strMessage = "Import Compressed file: Compressed file contains an empty file:\n" + strFileName;
        else
            strMessage = "Empty file:\n" + strFileName;

        // Error
        bResult = false;
        nConvertStatus = eConvertError;
        goto labelEndOfProcess;
    }

    r=IsGalaxySemiTxtFormat(cFileInfo.absoluteFilePath());
    if ( r.startsWith("error")||(r.startsWith("yes")) )
    {
        bResult = false;
        nConvertStatus = eConvertError;
        strMessage = "Quantix txt/ASCII format not supported for reading\n";
        goto labelEndOfProcess;
    }

    strExtension = cFileInfo.suffix().toLower();
    // Step#1: Verify if it is a STDF V4 file
    bResult = GS::StdLib::Stdf::IsCompatible(strFileName.toLatin1().constData());
    // Check if stdf has to be updated
    lConvertionWithExternalFile = ConverterExternalFile::Exists(cFileInfo.absolutePath());
    if(bResult && GS::StdLib::Stdf::GetStaticStdfVersion() == 4 && !lConvertionWithExternalFile)
    {
        // No convertion
        nConvertStatus = eConvertSuccess;
        goto labelEndOfProcess;
    }
    // STDF V3 file
    else if (bResult && GS::StdLib::Stdf::GetStaticStdfVersion() == 3 && !lConvertionWithExternalFile)
    {
        CGSTDFV3toSTDF StdfV3Convert;
        bResult = StdfV3Convert.Convert(strFileName.toLatin1().constData(),strFileNameSTDF.toLatin1().constData()/*, &ed*/);
        if(bResult)
        {
            nConvertStatus = eConvertSuccess;
            bFileCreated = true;
            goto labelEndOfProcess;
        }
        strMessage = StdfV3Convert.GetLastError();
    }

    // Step#2: Extension detection ...
    // Step#2.1: Call the parser library to try to convert teh input file
    if((!GS::LPPlugin::ProductInfo::getInstance()->isOEM()) && lParserFactory != NULL)
    {
        QScopedPointer<GS::Parser::ParserAbstract> lParser(lParserFactory->CreateParser(strFileName.toLatin1().constData()));

        if(lParser.isNull() == false)
        {
            ProgressBarHandler lHandler;
            lParser->SetProgressHandler(&lHandler);
            lParser->SetParameterFolder(lParameterFolder);
            std::string lFileNameSTDF = strFileNameSTDF.toStdString();
            lStatus = lParser->Convert(strFileName.toStdString(), lFileNameSTDF);
            strFileNameSTDF = QString::fromStdString(lFileNameSTDF);

            // Check if the parser generate a list of output file instead of one output file
            if (lParser->GetListStdfFiles().size() >= 1)
            {
                lstFileNameSTDF.clear();
                std::list<std::string> lFiles = lParser->GetListStdfFiles();
                std::list<std::string>::iterator lFirst(lFiles.begin()),
                        lEnd(lFiles.end());
                while(lFirst!=lEnd)
                {
                    lstFileNameSTDF.append(QString::fromStdString(*lFirst));
                    ++lFirst;
                }
                nConvertStatus = eConvertSuccess;
                bFileCreated = true;
            }
            else if(lStatus == GS::Parser::ConvertSuccess)
            {
                lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
                nConvertStatus = eConvertSuccess;
                bFileCreated = true;
            }
            else
            {
                std::string lErrorMsg;
                lParser->GetLastError(lErrorMsg);
                strMessage = lErrorMsg.c_str();
            }

            // The file matched with one of the Factory Parser
            // Do not try another one
            goto labelEndOfProcess;
        }
    }

    // Step#2.2: Call the old parser with the extention
    // Convert a ATDF file to STDF!
    if((strExtension == "csv") || (strExtension == "txt"))
    {
        // Convert a CSV file to STDF (unless already exists)!
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;
            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // Check if CSV text signature is of UMC-PCM file
        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGPCMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCMtoSTDF			PcmConvert;
            bResult = PcmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmConvert.GetLastError();
        }

        if(CGPCM_Hynix_toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_Hynix_toSTDF	Pcm_HynixConvert;
            bResult = Pcm_HynixConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = Pcm_HynixConvert.GetLastError();
        }

        if(CGPCM_JAZZtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_JAZZtoSTDF	Pcm_JazzConvert;
            bResult = Pcm_JazzConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = Pcm_JazzConvert.GetLastError();
        }

        // PCM from MagnaChip KDF data  format
        if(CGPCM_KDFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_KDFtoSTDF		Pcm_MagnachipKdfConvert;
            bResult = Pcm_MagnachipKdfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = Pcm_MagnachipKdfConvert.GetLastError();
        }

        // PCM from MagnaChip Type2 data  format
        if(CGPCM_MAGNACHIP_TYPE_2toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_MAGNACHIP_TYPE_2toSTDF		PcmMagnachipType2Convert;
            bResult = PcmMagnachipType2Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmMagnachipType2Convert.GetLastError();
        }

        // PCM from X-Fab data  format
        if(CGPCM_X_FABtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_X_FABtoSTDF		PcmXFabConvert;
            bResult = PcmXFabConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmXFabConvert.GetLastError();
        }

        // PCM from Gsmc data  format
        if(CGPcmGsmctoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPcmGsmctoSTDF		PcmGsmcConvert;
            bResult = PcmGsmcConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmGsmcConvert.GetLastError();
        }

        // PCM from Silan data  format
        if(CGPcmSilantoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPcmSilantoSTDF		PcmSilanConvert;
            bResult = PcmSilanConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmSilanConvert.GetLastError();
        }

        // PCM from Exar data  format
        if(CGPcmExartoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPcmExartoSTDF		clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // PCM from Cypress data format
        if(CGPcmCypresstoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPcmCypresstoSTDF		clConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // Eagle datalog  format
        if(CGEAGLE_DATALOGtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGEAGLE_DATALOGtoSTDF		EagleConvert;
            bResult = EagleConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = EagleConvert.GetLastError();
        }


        // Tessera Inspection format
        if(CGTesseraInspectiontoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraInspectiontoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera QV302 format
        if(CGTesseraQV302toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraQV302toSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Diffractive format
        if(CGTesseraDiffractivetoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraDiffractivetoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Microlupi format
        if(CGTesseraMicrolupitoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraMicrolupitoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Evg40 format
        if(CGTesseraEvg40toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraEvg40toSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Tessera WaveMaster format
        if(CGTesseraWavemastertoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraWavemastertoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Tessera Yoda-Imagemaster format
        if(CGTesseraYodatoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraYodatoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Acco - S8107/S8200 Data format
        if(CGAccotoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGAccotoSTDF		AccoConvert;
            bResult = AccoConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = AccoConvert.GetLastError();
        }

        // Mcube
        if(CGMcubetoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGMcubetoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // HejianTechnology
        if(GS::Parser::CGPcmHjtctoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            GS::Parser::CGPcmHjtctoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName, strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            std::string lErrorMsg;
            clConvert.GetLastError(lErrorMsg);
            strMessage = lErrorMsg.c_str();
        }

        // Fresco Summary data
        if(CGFrescoSummarytoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGFrescoSummarytoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // Etest Fab7 WAT data
        if(CGEtestFab7toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGEtestFab7toSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // Freescale Maxwell format to STDF converter
        if(CGFreescaletoSTDF::IsCompatible(strFileName.toLatin1().data()))
        {
            CGFreescaletoSTDF	eConvert;
            bResult = eConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                // Check if have some warnings
                strMessage = eConvert.GetLastError();
                if(strMessage.isEmpty())
                    nConvertStatus = eConvertSuccess;
                else
                    nConvertStatus = eConvertWarning;
                goto labelEndOfProcess;
            }
            strMessage = eConvert.GetLastError();
        }

        // PCM Tower CSV format to STDF converter
        if(CGPCM_TOWERtoSTDF::IsCompatible(strFileName.toLatin1().data()))
        {
            CGPCM_TOWERtoSTDF	eConvert;
            bResult = eConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = eConvert.GetLastError();
        }

        if(strExtension == "txt")
        {
            // Convert a CSM file to STDF (unless already exists)!
            if(CGCSMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGCSMtoSTDF	CsmConvert;
                bResult = CsmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = CsmConvert.GetLastError();
            }

            // Convert a CSM type 2 file to STDF (unless already exists)!
            if(CGCSM2toSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGCSM2toSTDF	CsmConvert;
                bResult = CsmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = CsmConvert.GetLastError();
            }

            // Convert a CSMC file to STDF (unless already exists)!
            if(CGCSMCtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGCSMCtoSTDF	CsmcConvert;
                bResult = CsmcConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = CsmcConvert.GetLastError();
            }

            // TMT ASL1000
            if(CGASL1000toSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGASL1000toSTDF	AslConvert;
                bResult = AslConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = AslConvert.GetLastError();
            }

            // KLA Inspection failures
            if(CGKLARFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGKLARFtoSTDF	KlarfConvert;
                bResult = KlarfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = KlarfConvert.GetLastError();
            }

            // ADVANTEST_T2000 Inspection failures
            if(CGADVANTEST_T2000toSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGADVANTEST_T2000toSTDF	T2000Convert;
                bResult = T2000Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = T2000Convert.GetLastError();
            }

            // WIF format
            if(CGWiftoSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGWiftoSTDF	clConvert;
                bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = clConvert.GetLastError();
            }

            // CSMC_SPDM format
            if(CGCSMC_SPDMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGCSMC_SPDMtoSTDF lSpdmConvert;
                bResult = lSpdmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = lSpdmConvert.GetLastError();
            }

            // TERADYNE_ASCII format
            if(CGTERADYNE_ASCIItoSTDF::IsCompatible(strFileName.toLatin1().constData()))
            {
                CGTERADYNE_ASCIItoSTDF lTeradyneAsciiConvert;
                bResult = lTeradyneAsciiConvert.Convert(strFileName.toLatin1().constData(),
                                                        strFileNameSTDF.toLatin1().constData());
                if(bResult)
                {
                    nConvertStatus = eConvertSuccess;
                    goto labelEndOfProcess;
                }
                strMessage = lTeradyneAsciiConvert.GetLastError();
            }
        }

        // Unknown CSV format...
        bFileCreated = false;
    }
    else if((strExtension == "krf") || (strExtension == "klarf"))
    {
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // As we create a STDF target file, we add the user defined extension.
        bFileCreated = true;

        // KLA Inspection failures
        if(CGKLARFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGKLARFtoSTDF	KlarfConvert;
            bResult = KlarfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = KlarfConvert.GetLastError();
        }

        bFileCreated = false;
    }
    else if((strExtension == "pcm"))
    {
        // Convert a PCM (UMC Foundry) file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // As we create a STDF target file, we add the user defined extension.
        bFileCreated = true;
        //strFileNameSTDF.toLatin1().constData() += strNewExtension;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        // PCM from UMC format
        if(CGPCMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCMtoSTDF	PcmConvert;
            bResult = PcmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmConvert.GetLastError();
        }

        // Check if this not rather a HYNIX PCM format
        if(CGPCM_Hynix_toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_Hynix_toSTDF	Pcm_HynixConvert;
            bResult = Pcm_HynixConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = Pcm_HynixConvert.GetLastError();
        }

        // PCM from MagnaChip KDF data  format
        if(CGPCM_KDFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_KDFtoSTDF		Pcm_MagnachipKdfConvert;
            bResult = Pcm_MagnachipKdfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = Pcm_MagnachipKdfConvert.GetLastError();
        }

        // PCM from X-Fab data  format
        if(CGPCM_X_FABtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_X_FABtoSTDF		PcmXFabConvert;
            bResult = PcmXFabConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmXFabConvert.GetLastError();
        }

        // Unknown PCM format...
        bFileCreated = false;
    }
    else if((strExtension == "wat"))
    {
        // Convert a WAT file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // As we create a STDF target file, we add the user defined extension.
        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGWATtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGWATtoSTDF	WatConvert;
            bResult = WatConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = WatConvert.GetLastError();
        }

        bFileCreated = false;
    }
    else if((strExtension == "spd"))
    {
        // Convert a SPD (TMT Credence) file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        // As we create a STDF target file, we add the user defined extension.
        bFileCreated = true;
        //strFileNameSTDF.toLatin1().constData() += strNewExtension;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGSPDtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSPDtoSTDF	SpdConvert;
            bResult = SpdConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = SpdConvert.GetLastError();
        }

        // Unknown WAT format
        bFileCreated = false;
    }
    else if((strExtension == "dl4"))
    {
        // Convert a DL4 (ASL Credence old format) file to STDF (unless already exists)!

        if(bAcceptDL4 == false)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CDL4toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CDL4toSTDF	Dl4Convert;
            bResult = Dl4Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = Dl4Convert.GetLastError();
        }

        // Unknown DL4 format

        bFileCreated = false;
    }
    else if((strExtension == "gdf") || (strExtension == "gdf_0") || (strExtension == "gdf_1") || (strExtension == "gdf_2"))
    {
        // Convert a GDF file to STDF (unless already exists)!
        // Note: .gdf is first testing data file, gdf_0 is 1st retest, .gdf_1 is 2nd restest, etc...
        bool	bUseTestNumber;	// true if FILE type analysis, false if database analysis mode.

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // GEX_PARSER_GDF_USEMAPPINGFILE
        // Force YieldMan/Examinator to user test_name index from gdf_parameter.txt file
        char *ptChar = getenv("GEX_PARSER_GDF_USEMAPPINGFILE");
        if(ptChar)
        {
            // Use gdf_parameter.txt for test_number
            bUseTestNumber = true;
        }
        else if(bDatabaseAccessMode == true)
            // Convert. 3rd parameter =false to use 'test_numbers', 'true' to use test name index instead.
            bUseTestNumber = false;
        else
            bUseTestNumber = true;

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGGDFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGGDFtoSTDF	GdfConvert;
            int nStatus;
            nStatus = GdfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData(), bUseTestNumber, bMonitoring||bDatabaseAccessMode);
            if(nStatus == CGGDFtoSTDF::errNoError)
            {
                // Convert OK
                bResult = true;
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else if(nStatus == CGGDFtoSTDF::errWarningEndOfFile)
            {
                // Convert OK with warning
                bResult = true;
                nConvertStatus = eConvertWarning;
                strMessage = GdfConvert.GetLastError();
                goto labelEndOfProcess;
            }

            // No convertion
            bResult = false;
            strMessage = GdfConvert.GetLastError();
        }
        // Unknown GDF format
        bFileCreated = false;
    }
    else if(strExtension == "dat")
    {
        // .DAT: G85, or generic .CSV file

        //
        // Convert a Semi G85 file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // As we create a STDF target file, we add the user defined extension.
        //strFileNameSTDF.toLatin1().constData() += strNewExtension;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        // TMT ASL1000
        if(CGASL1000toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGASL1000toSTDF	AslConvert;
            bResult = AslConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = AslConvert.GetLastError();
        }

        // Tessera format
        if(CGTesseratoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseratoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Diffractive format
        if(CGTesseraDiffractivetoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraDiffractivetoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Microlupi format
        if(CGTesseraMicrolupitoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraMicrolupitoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Evg40 format
        if(CGTesseraEvg40toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraEvg40toSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera WaveMaster format
        if(CGTesseraWavemastertoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraWavemastertoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Tessera Yoda-Imagemaster format
        if(CGTesseraYodatoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraYodatoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }

        // Wafer Prober TEL format (thousand dies format)
        if(CGProberTelThousandToSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGProberTelThousandToSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // Wafer Prober TEL format (Million dies format)
        if(CGProberTelMillionToSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Wafer Prober TEL format
            CGProberTelMillionToSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // Wafer Prober TSK format
        if(CGProberTsktoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGProberTsktoSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // KAL/INF Layers format
        if(CGKlaInfLayerstoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGKlaInfLayerstoSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // STIF - ST inkless format
        if(CGSTIFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSTIFtoSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // Magnachip PCM data
        if(CGPCM_MAGNACHIP_COLUMNtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_MAGNACHIP_COLUMNtoSTDF    cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        // Unknown TXT format...
        bFileCreated = false;
    }
    else if(strExtension == "pts")
    {
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        // Tessera format
        if(CGTesseraQV302toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraQV302toSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Unknown TXT format...
        bFileCreated = false;
    }
    else if((strExtension == "dta"))
    {
        // Convert a Tesec file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CSPEKTRAtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CSPEKTRAtoSTDF	SpektraConvert;
            bResult = SpektraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            SpektraConvert.GetLastError(strMessage);
        }
        bFileCreated = false;
    }
    else if((strExtension == "xml"))
    {
        // Convert a 7C7 file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        // Check if 7C7 XML file
        if(C7C7toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            C7C7toSTDF	cl7C7Convert;
            bResult = cl7C7Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cl7C7Convert.GetLastError();
        }

        // Check if TANGO XML
        if(CTANGOtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CTANGOtoSTDF	clTangoConvert;
            bResult = clTangoConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clTangoConvert.GetLastError();
        }

        // Check if Kla Wafer XML
        if(CGKlaWaferXmltoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGKlaWaferXmltoSTDF	clGKlaWaferXmlConvert;
            bResult = clGKlaWaferXmlConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clGKlaWaferXmlConvert.GetLastError();
        }

        // Check if SEMI G85 XML
        if(CGSemiG85XmltoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSemiG85XmltoSTDF	G85Convert;
            bResult = G85Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = G85Convert.GetLastError();
        }

        // Check if SEMI E142 XML
        if(CGSemiE142XmltoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSemiE142XmltoSTDF	E142Convert;
            bResult = E142Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = E142Convert.GetLastError();
        }
        if(CGKVDXMLtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGKVDXMLtoSTDF	KVDXMLConvert;
            bResult = KVDXMLConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = KVDXMLConvert.GetLastError();
        }

        if(Parser93kTab::IsCompatible(strFileName.toLatin1().constData()))
        {
            Parser93kTab	FujitsuConvert;
            bResult = FujitsuConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = FujitsuConvert.GetLastError();
        }

        // Unknown XML format

        bFileCreated = false;
    }
    else if(strExtension == "map")
    {
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");



        // Check if Spil Ws
        if(CGSpilWstoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSpilWstoSTDF	SpilWsConvert;
            bResult = SpilWsConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = SpilWsConvert.GetLastError();
        }

        // Unknown TXT format...
        bFileCreated = false;
    }
    else if(strExtension == "tsm")
    {
        // Convert a Prober TSMC file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGPROBER_TSMCtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPROBER_TSMCtoSTDF	clTsmcConvert;
            bResult = clTsmcConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clTsmcConvert.GetLastError();
        }
        // Unknown TSMC format
        bFileCreated = false;
    }
    else if(strExtension == "data")
    {
        // Convert a Teradyne Image file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGTERADYNE_IMAGEtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTERADYNE_IMAGEtoSTDF	clDataConvert;
            bResult = clDataConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clDataConvert.GetLastError();
        }
        bFileCreated = false;
    }
    else if((strExtension == "log") || (strExtension == "datalog"))
    {
        // Convert a Asmc Wat file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGEAGLE_DATALOGtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGEAGLE_DATALOGtoSTDF	EagleConvert;
            bResult = EagleConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = EagleConvert.GetLastError();
        }

        if(CGSpeaDatalogtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSpeaDatalogtoSTDF	SpeaDatalogConvert;
            bResult = SpeaDatalogConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = SpeaDatalogConvert.GetLastError();
        }

        if(CGAmidatoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGAmidatoSTDF	AmidaConvert;
            bResult = AmidaConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = AmidaConvert.GetLastError();
        }

        if(CGKVDtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGKVDtoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        if(CGMcubetoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGMcubetoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        if(CGNd1Nd2DatalogtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGNd1Nd2DatalogtoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // Unknown Data format
        bFileCreated = false;
    }
    else if((strExtension == "in") || (strExtension == "edf") || (strExtension == "edl"))
    {
        // 2 formats: ASCII or BINARY
        // Convert a Verigy EDL file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGVERIGY_EDLtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGVERIGY_EDLtoSTDF	EdlConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            int nStatus = EdlConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF, bAllowOnlyOneFile);
            if(nStatus == CGVERIGY_EDLtoSTDF::eConvertSuccess)
            {
                bResult = true;
                nConvertStatus = eConvertSuccess;

                if(EdlConvert.GetLastErrorCode() == CGVERIGY_EDLtoSTDF::errWarning)
                {
                    // Convert OK with warning
                    nConvertStatus = eConvertWarning;
                    strMessage = EdlConvert.GetLastError();
                }

                goto labelEndOfProcess;
            }
            strMessage = EdlConvert.GetLastError();
            bResult = false;
            if(EdlConvert.GetLastErrorCode() == CGVERIGY_EDLtoSTDF::errMultiLot)
            {
                nConvertStatus = eConvertError;
                goto labelEndOfProcess;
            }
        }

        if(CGVERIGY_EDFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGVERIGY_EDFtoSTDF	EdfConvert;
            bResult = EdfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = EdfConvert.GetLastError();
        }
        // Unknown EDL format
        bFileCreated = false;
    }
    else if((strExtension == "dl"))
    {
        // Convert a Advantest T2000 file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGADVANTEST_T2000toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGADVANTEST_T2000toSTDF	AdvantestT2000Convert;
            bResult = AdvantestT2000Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = AdvantestT2000Convert.GetLastError();
        }
        // Unknown EDF format
        bFileCreated = false;
    }
    else if((strExtension == "col"))
    {
        // Convert a Laurier Die Sort 1D file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGLaurierDieSorttoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Laurier Die Sort 1D format
            CGLaurierDieSorttoSTDF	LaurierConvert;
            bResult = LaurierConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = LaurierConvert.GetLastError();
        }
        // Unknown Laurier format
        bFileCreated = false;
    }
    else if((strExtension == "wpr"))
    {
        // Convert a TSMC WPR file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(CGTsmcWprtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Laurier Die Sort 1D format
            CGTsmcWprtoSTDF	cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        // Unknown Laurier format
        bFileCreated = false;
    }
    else if((strExtension == "tsl"))
    {
        // Convert a TSM WAT file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        // Unknown Laurier format
        bFileCreated = false;

    }
    else if((strExtension == "dlk"))
    {
        // Convert a SHIBASOKU - DLK file to STDF (unless already exists)!

        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        // Check if Tower Semiconductor SHIBASOKU - DLK
        if(CGDLKtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGDLKtoSTDF	clConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // Unknown Laurier format
        bFileCreated = false;

    }
    else if((strExtension == "sdi") || (strExtension == "out"))
    {
        // Convert a Silicon Data Integration file to STDF (unless already exists)!
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;
            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }
        bFileCreated = true;
        // As we create a STDF target file, we add the user defined extension.
        //strFileNameSTDF.toLatin1().constData() += strNewExtension;
        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);
        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");
        // Check if Silicon Data Integration
        if(CGSditoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSditoSTDF	clConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        // Unknown Silicon Data Integration format
        bFileCreated = false;
    }

    // Step#3: Try Automatic format detection (ignoring file extension...)
    if(!bResult)
    {
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG;

            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(C7C7toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a 7C7 file to STDF (unless already exists)!
            C7C7toSTDF	cl7C7Convert;
            bResult = cl7C7Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = cl7C7Convert.GetLastError();
        }
        if(CGASL1000toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // TMT ASL1000
            CGASL1000toSTDF	AslConvert;
            bResult = AslConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = AslConvert.GetLastError();
        }
        if(CGCSMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a CSM file to STDF (unless already exists)!
            CGCSMtoSTDF	CsmConvert;
            bResult = CsmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = CsmConvert.GetLastError();
        }
        if(CGCSM2toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a CSM file to STDF (unless already exists)!
            CGCSM2toSTDF	CsmConvert;
            bResult = CsmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = CsmConvert.GetLastError();
        }
        if(CGCSMCtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a CSMC file to STDF (unless already exists)!
            CGCSMCtoSTDF	CsmcConvert;
            bResult = CsmcConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = CsmcConvert.GetLastError();
        }
        if(CGEAGLE_DATALOGtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Eagle Datalog file to STDF (unless already exists)!
            CGEAGLE_DATALOGtoSTDF	EagleConvert;
            bResult = EagleConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = EagleConvert.GetLastError();
        }
        if(CGKLARFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Klarf file to STDF (unless already exists)!
            CGKLARFtoSTDF	KlarfConvert;
            bResult = KlarfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = KlarfConvert.GetLastError();
        }
        if(CGKlaWaferXmltoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Klarf file to STDF (unless already exists)!
            CGKlaWaferXmltoSTDF	GKlaWaferXmlConvert;
            bResult = GKlaWaferXmlConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = GKlaWaferXmlConvert.GetLastError();
        }
        if(CGPCMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Pcm file to STDF (unless already exists)!
            CGPCMtoSTDF	PcmConvert;
            bResult = PcmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = PcmConvert.GetLastError();
        }
        if(CGPCM_Hynix_toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Pcm Hynix file to STDF (unless already exists)!
            CGPCM_Hynix_toSTDF	Pcm_HynixConvert;
            bResult = Pcm_HynixConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = Pcm_HynixConvert.GetLastError();
        }
        if(CGPCM_JAZZtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Pcm Jazz file to STDF (unless already exists)!
            CGPCM_JAZZtoSTDF	Pcm_JazzConvert;
            bResult = Pcm_JazzConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = Pcm_JazzConvert.GetLastError();
        }
        if(CGPCM_KDFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a MagnaChip KDF file to STDF (unless already exists)!
            CGPCM_KDFtoSTDF	Pcm_MagnachipKdfConvert;
            bResult = Pcm_MagnachipKdfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
                goto labelEndOfProcess;
            else
                strMessage = Pcm_MagnachipKdfConvert.GetLastError();
        }
        if(CGPCM_MAGNACHIP_TYPE_2toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a PCM from MagnaChip Type2 file to STDF (unless already exists)!
            CGPCM_MAGNACHIP_TYPE_2toSTDF	PcmMagnachipType2Convert;
            bResult = PcmMagnachipType2Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = PcmMagnachipType2Convert.GetLastError();
        }
        if(CGPCM_X_FABtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a PCM from X-Fab file to STDF (unless already exists)!
            CGPCM_X_FABtoSTDF	PcmXFabConvert;
            bResult = PcmXFabConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = PcmXFabConvert.GetLastError();
        }
        if(CGPcmGsmctoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // PCM from Gsmc data  format
            CGPcmGsmctoSTDF		PcmGsmcConvert;
            bResult = PcmGsmcConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmGsmcConvert.GetLastError();
        }
        if(CGPcmSilantoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // PCM from Silan data  format
            CGPcmSilantoSTDF		PcmSilanConvert;
            bResult = PcmSilanConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = PcmSilanConvert.GetLastError();
        }
        if(CGPcmExartoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // PCM from Exar data  format
            CGPcmExartoSTDF		clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        if(CGPcmCypresstoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPcmCypresstoSTDF		clConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        if(CGPROBER_TSMCtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Prober TSMC file to STDF (unless already exists)!
            CGPROBER_TSMCtoSTDF	clTsmcConvert;
            bResult = clTsmcConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = clTsmcConvert.GetLastError();
        }
        if(CGSemiG85XmltoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Check if SEMI G85 XML
            CGSemiG85XmltoSTDF	G85Convert;
            bResult = G85Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = G85Convert.GetLastError();
        }
        if(Parser93kTab::IsCompatible(strFileName.toLatin1().constData()))
        {
            Parser93kTab	FujitsuConvert;
            bResult = FujitsuConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = FujitsuConvert.GetLastError();
        }

        if(CGKVDXMLtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGKVDXMLtoSTDF	KVDXMLConvert;
            bResult = KVDXMLConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = KVDXMLConvert.GetLastError();
        }

        if(CGSemiE142XmltoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Check if SEMI E142 XML
            CGSemiE142XmltoSTDF	E142Convert;
            bResult = E142Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = E142Convert.GetLastError();
        }
        if(CGSpeaDatalogtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSpeaDatalogtoSTDF	SpeaDatalogConvert;
            bResult = SpeaDatalogConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = SpeaDatalogConvert.GetLastError();
        }
        if(CGAmidatoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGAmidatoSTDF	AmidaConvert;
            bResult = AmidaConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = AmidaConvert.GetLastError();
        }
        if(CGAccotoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGAccotoSTDF	AccoConvert;
            bResult = AccoConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = AccoConvert.GetLastError();
        }

        if(CGSpilWstoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Spil Ws file to STDF (unless already exists)!
            CGSpilWstoSTDF	SpilWsConvert;
            bResult = SpilWsConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = SpilWsConvert.GetLastError();
        }
        if(CGSPDtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a SPD (TMT Credence) file to STDF (unless already exists)!
            CGSPDtoSTDF	SpdConvert;
            bResult = SpdConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = SpdConvert.GetLastError();
        }
        if(CTANGOtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Tango file to STDF (unless already exists)!
            CTANGOtoSTDF	clTangoConvert;
            bResult = clTangoConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = clTangoConvert.GetLastError();
        }
        if(CGTERADYNE_IMAGEtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Teradyne Image file to STDF (unless already exists)!
            CGTERADYNE_IMAGEtoSTDF	clDataConvert;
            bResult = clDataConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = clDataConvert.GetLastError();
        }
        if(CGNItoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a National Instruments / TestStand platform file to STDF (unless already exists)!
            CGNItoSTDF	NationalInstruments_Convert;
            bResult = NationalInstruments_Convert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = NationalInstruments_Convert.GetLastError();
        }
        if(CGWATtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Wat file to STDF (unless already exists)!
            CGWATtoSTDF	WatConvert;
            bResult = WatConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = WatConvert.GetLastError();
        }
        if(CGYOKOGAWAtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Wat Smic file to STDF (unless already exists)!
            CGYOKOGAWAtoSTDF	YokogawaConvert;
            bResult = YokogawaConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                strMessage = YokogawaConvert.GetLastError();
        }
        if(CGVERIGY_EDLtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Verigy EDL file to STDF (unless already exists)!
            CGVERIGY_EDLtoSTDF	EdlConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            int nStatus = EdlConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF, bAllowOnlyOneFile);
            if(nStatus == CGVERIGY_EDLtoSTDF::eConvertSuccess)
            {
                bResult = true;
                nConvertStatus = eConvertSuccess;

                if(EdlConvert.GetLastErrorCode() == CGVERIGY_EDLtoSTDF::errWarning)
                {
                    // Convert OK with warning
                    nConvertStatus = eConvertWarning;
                    strMessage = EdlConvert.GetLastError();
                }

                goto labelEndOfProcess;
            }
            strMessage = EdlConvert.GetLastError();
            bResult = false;
            if(EdlConvert.GetLastErrorCode() == (int)CGVERIGY_EDLtoSTDF::errMultiLot)
            {
                nConvertStatus = eConvertError;
                goto labelEndOfProcess;
            }
        }
        if(CGVERIGY_EDFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Convert a Verigy EDL file to STDF (unless already exists)!
            CGVERIGY_EDFtoSTDF	EdfConvert;
            bResult = EdfConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = EdfConvert.GetLastError();
        }
        if(CGTesseratoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Tessera format
            CGTesseratoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        if(CGTesseraInspectiontoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Tessera Inspection format
            CGTesseraInspectiontoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        if(CGTesseraQV302toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Tessera QV302 format
            CGTesseraQV302toSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Tessera Diffractive format
        if(CGTesseraDiffractivetoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraDiffractivetoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Tessera Microlupi format
        if(CGTesseraMicrolupitoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraMicrolupitoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Tessera Evg40 format
        if(CGTesseraEvg40toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraEvg40toSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        // Tessera WaveMaster format
        if(CGTesseraWavemastertoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTesseraWavemastertoSTDF		TesseraConvert;
            bResult = TesseraConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = TesseraConvert.GetLastError();
        }
        if(CGLaurierDieSorttoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Laurier Die Sort 1D format
            CGLaurierDieSorttoSTDF	LaurierConvert;
            bResult = LaurierConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = LaurierConvert.GetLastError();
        }
        if(CGProberTelThousandToSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Wafer Prober TEL format
            CGProberTelThousandToSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        if(CGProberTelMillionToSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Wafer Prober TEL format
            CGProberTelMillionToSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        if(CGProberTsktoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Wafer Prober TSK format
            CGProberTsktoSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        if(CGTsmcWprtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Tsmc WPR format
            CGTsmcWprtoSTDF	cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        if(CGKVDtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // KVD WAFER data
            CGKVDtoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        if(CGMcubetoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // Mcube E320 datalog
            CGMcubetoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        if(CGNd1Nd2DatalogtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGNd1Nd2DatalogtoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        if(CGKlaInfLayerstoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // KAL/INF Layers format
            CGKlaInfLayerstoSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }
        if(CGWiftoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            // WIF format
            CGWiftoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // HejianTechnology
        if(GS::Parser::CGPcmHjtctoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            GS::Parser::CGPcmHjtctoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName, strFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            std::string lErrorMsg;
            clConvert.GetLastError(lErrorMsg);
            strMessage = lErrorMsg.c_str();
        }
        // Fresco Summary data
        if(CGFrescoSummarytoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGFrescoSummarytoSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        // SHIBASOKU - DLK data
        if(CGDLKtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGDLKtoSTDF	clConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }
        // Etest Fab7 - WAT data
        if(CGEtestFab7toSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGEtestFab7toSTDF	clConvert;
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // STIF - ST inkless format
        if(CGSTIFtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSTIFtoSTDF		cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // Silicon Data Integration
        if(CGSditoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGSditoSTDF	clConvert;
            lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            bResult = clConvert.Convert(strFileName.toLatin1().constData(), lstFileNameSTDF);
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = clConvert.GetLastError();
        }

        // Freescale Maxwell format to STDF converter
        if(CGFreescaletoSTDF::IsCompatible(strFileName.toLatin1().data()))
        {
            CGFreescaletoSTDF	eConvert;
            bResult = eConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                // Check if have some warnings
                strMessage = eConvert.GetLastError();
                if(strMessage.isEmpty())
                    nConvertStatus = eConvertSuccess;
                else
                    nConvertStatus = eConvertWarning;
                goto labelEndOfProcess;
            }
            strMessage = eConvert.GetLastError();
        }

        // Magnachip PCM data
        if(CGPCM_MAGNACHIP_COLUMNtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGPCM_MAGNACHIP_COLUMNtoSTDF    cConvert;
            bResult = cConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = cConvert.GetLastError();
        }

        // PCM Tower CSV format to STDF converter
        if(CGPCM_TOWERtoSTDF::IsCompatible(strFileName.toLatin1().data()))
        {
            CGPCM_TOWERtoSTDF	eConvert;
            bResult = eConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = eConvert.GetLastError();
        }

        // CSMC_SPDM format
        if(CGCSMC_SPDMtoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGCSMC_SPDMtoSTDF lSpdmConvert;
            bResult = lSpdmConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = lSpdmConvert.GetLastError();
        }

        // TERADYNE_ASCII format
        if(CGTERADYNE_ASCIItoSTDF::IsCompatible(strFileName.toLatin1().constData()))
        {
            CGTERADYNE_ASCIItoSTDF lTeradyneAsciiConvert;
            bResult = lTeradyneAsciiConvert.Convert(strFileName.toLatin1().constData(),
                                                    strFileNameSTDF.toLatin1().constData());
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            strMessage = lTeradyneAsciiConvert.GetLastError();
        }
        bFileCreated = false;
    }

    // No convertion found
    // Step#4: Try Csv format detection
    if(!bResult)
    {
        if(bOnlyAcceptSTDF == true)
        {
            strMessage = MSG ;
            // Error
            nConvertStatus = eConvertError;
            goto labelEndOfProcess;
        }

        bFileCreated = true;

        // Check where intermediate STDF file has to be created...
        ValidateIntermediateDataPath(strFileNameSTDF);

        // Parsing file...
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Parsing data...");

        if(
                (strExtension == "dat")
                || (strExtension == "csv")
                || (strExtension == "txt")
                )
        {
            // Check if CSV generic data
            CGCSVtoSTDF	CsvConvert;
            QStringList splitCsv(strFileName);
            CsvConvert.SplitLots(strFileName,splitCsv);
            foreach(strFileName,splitCsv)
            {
                // Update the Stdf file for each new csv
                if(splitCsv.count() > 1)
                {
                    QString splitFile;
                    splitFile = QFileInfo(strFileNameSTDF.toLatin1().constData()).path();
                    splitFile += QDir::separator();
                    splitFile += QFileInfo(strFileName).baseName();
                    splitFile += "." + QFileInfo(strFileNameSTDF.toLatin1().constData()).suffix();
                    strFileNameSTDF = splitFile;
                    // Check where intermediate STDF file has to be created...
                    ValidateIntermediateDataPath(strFileNameSTDF);
                }
                bResult = CsvConvert.Convert(strFileName.toLatin1().constData(), strFileNameSTDF.toLatin1().constData(),bAllowExtendedCsv,bMonitoring,bAllowWizard);
                if(!bResult)
                    break;
                lstFileNameSTDF.append(strFileNameSTDF.toLatin1().constData());
            }
            if(bResult)
            {
                nConvertStatus = eConvertSuccess;
                goto labelEndOfProcess;
            }
            else
                // Unknown TXT format...
            {
                if(strMessage.isEmpty())
                    strMessage = "Abort or failure processing DAT/CSV/TXT file\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV\nformat specifications supported.\n\nIf your file complies with Examinator's specs, then\nplease send it to Quantix support at "+QString(GEX_EMAIL_SUPPORT);
            }
        }
        bFileCreated = false;
    }


labelEndOfProcess:

    if(lstFileNameSTDF.isEmpty())
    {
        if(bResult && bFileCreated)
            lstFileNameSTDF.append(strFileNameSTDF);
        else
            lstFileNameSTDF.append(strFileName);
    }

    // If file was compressed and none of the above types detected, then assume it's STDF !
    if((bResult == true) && (bCompressedFile == true) && (bFileCreated == false))
    {
        // Append custom extension so we can erase this STDF temporary file later on...
        strFileNameSTDF = strFileName;
        strFileNameSTDF += ".gextb.std";
        QDir cDir;
        cDir.rename(strFileName,strFileNameSTDF.toLatin1().constData());
        bFileCreated = true;

        // Add file to list of temporary files to erase on Examinator exit...
        GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strFileNameSTDF.toLatin1().constData(), TemporaryFile::BasicCheck);

        // Clear error message
        strMessage = "";

        return eConvertSuccess;
    }

    if(bAllowOnlyOneFile && (lstFileNameSTDF.count()>1))
    {
        // Add file to list of temporary files to erase on Examinator exit...
        for(int i=0; i<lstFileNameSTDF.count(); i++)
            GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(lstFileNameSTDF[i], TemporaryFile::BasicCheck);
        strMessage = "Convertion file: Multi files generation not allowed";
        return eConvertError;
    }

    // Convertion status (true = success)
    if( nConvertStatus == eConvertSuccess)
        strMessage = "";	// Clear error message
    else
    {
        if(strMessage.contains("Invalid file format",Qt::CaseInsensitive) && !strMessage.contains("@mentor.com",Qt::CaseInsensitive))
            strMessage += ".\n\nIf you want further help then please send your file \nto Quantix support at "+QString(GEX_EMAIL_SUPPORT);

        if(strMessage.contains("License has expired",Qt::CaseInsensitive) && !strMessage.contains("@mentor.com",Qt::CaseInsensitive))
            strMessage += ".\n\nPlease contact Quantix sales  at "+QString(GEX_EMAIL_SALES);

        strMessage.append(".\n\nFile: "+lstFileNameSTDF.first()+((lstFileNameSTDF.count()>1)?"...":""));
    }

    return nConvertStatus;
}

}
}
