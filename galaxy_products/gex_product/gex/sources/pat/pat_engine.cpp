#include <QScriptValueIterator>
#include "pat_engine.h"
#include "pat_engine_private.h"
#include "pat_report_ws.h"
#include "stdfparse.h"
#include "report_options.h"
#include "pat_recipe.h"
#include "pat_recipe_io.h"
#include "outlierremoval/outlierremoval_taskdata.h"
#include "engine.h"
#include "scheduler_engine.h"
#include "product_info.h"
#include "stdf_content_utils.h"

extern CReportOptions	ReportOptions;

namespace GS
{
namespace Gex
{

PATEngine * PATEngine::mInstance = NULL;

PATEngine &PATEngine::GetInstance()
{

    if(!mInstance)
    {
        mInstance = new PATEngine(NULL);
    }
    return *mInstance;
}

PATEngine::PATEngine(QObject *parent)
    : QObject(parent), mPrivate(new PATEnginePrivate())
{
    setObjectName("GSPATEngine");
}

PATEngine::~PATEngine()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

CPatInfo *PATEngine::GetContext() const
{
    return mPrivate->mContext;
}

bool PATEngine::CreateContext()
{
    if (mPrivate->mContext == NULL)
    {
        mPrivate->mContext = new CPatInfo;
        if (mPrivate->mContext != NULL)
            return true;
    }
    return false;
}

void PATEngine::DeleteContext()
{
    if (mPrivate->mContext)
    {
        delete mPrivate->mContext;
        mPrivate->mContext = NULL;
    }
}

const QString &PATEngine::GetErrorMessage() const
{
    return mPrivate->mErrorMessage;
}

#ifdef GCORE15334

QString PATEngine::BuildPATReport(PATProcessing *lFields)
{
    if (lFields == NULL)
        return "Error: No PATProcessing object is null.";

    if (mPrivate->mContext == NULL)
        return "Error: No PAT context found";

    if (mPrivate->mContext->GetOutputDataFilename().isEmpty())
        return "Error: No output patman file defined";

    if (!QFile::exists(mPrivate->mContext->GetOutputDataFilename()))
        return "Error: output patman file "+ mPrivate->mContext->GetOutputDataFilename() +" does not exist";

    // Set the processed sites to split the report by site
    // Split file over each site
    QList<int>  lSites;
    QString     lSTDFCompliancy = ReportOptions.GetOption("dataprocessing",
                                                          "stdf_compliancy").toString();
    bool        lValidSiteOnly  = false;

    if (lSTDFCompliancy == "stringent")
        lValidSiteOnly = true;

    std::vector<int> lSite;
    if (GS::Gex::StdfContentUtils::GetSites((std::string)(mPrivate->mContext->GetOutputDataFilename().toLatin1().constData()),
                                            lSite,
                                            lValidSiteOnly) == false)
        return "Error: Cannot extract site from output file";

    lSites = QList<int>::fromVector( QVector<int>::fromStdVector(lSite));
    if (BuildPATReport(mPrivate->mContext->GetOutputDataFilename(), *lFields, lSites) == false)
        return "Error: " + GetErrorMessage();

    return "ok";
}

bool PATEngine::BuildPATReport(const QString &lPatmanFile, PATProcessing &lFields,
                               QList<int> lSites, bool lOpenReportViewer /*= false*/)
{
    if (lPatmanFile.isEmpty())
    {
        mPrivate->mErrorMessage = "No output patman file defined";
        return false;
    }

    PATReportWS lPATReportWS(lOpenReportViewer);

    lPATReportWS.SetSites(lSites);

    // Generate the PAT report
    if (lPATReportWS.Generate(lPatmanFile, lFields) == false)
    {
        mPrivate->mErrorMessage = lPATReportWS.GetErrorMessage();
        return false;
    }

    return true;
}
#endif

// gcore-1100
QString PATEngine::CreateZPATCompositeFile(const QString& lRecipeFile, const QScriptValue& lFields)
{
    mPrivate->mErrorMessage.clear();

    if (lFields.isNull())
        return "error: parameter null";
    if (!lFields.isArray())
        return "error: parameter not an array";
    if (!lFields.isObject())
        return "error: parameter not an object";

    //QString lOutput;
    CGexCompositePatProcessing lCompositePatProc;
    lCompositePatProc.strRecipeFile=lRecipeFile;
    QScriptValueIterator it(lFields);
    CGexCompositePatWaferInfo lWafer;

    bool lOk=false;
    while (it.hasNext())
    {
         it.next();
         unsigned lWaferId=it.name().toUInt(&lOk);
         if (lOk)
         {
             lWafer.clear();
             lWafer.iWaferID = lWaferId;
             QScriptValue lListOfFiles=it.value();
             if (!lListOfFiles.isArray())
                 return "error: list of files for wafer must be array of path";
             QScriptValueIterator itWafer(lListOfFiles);
             while(itWafer.hasNext())
             {
                 itWafer.next();
                 if (itWafer.name()=="length")
                     continue;
                 if (itWafer.name()=="OptionalSource")
                 {
                     lWafer.strOptionalSource=itWafer.value().toString();
                     continue;
                 }
                 lWafer.strSources+=itWafer.value().toString();
             }

             lCompositePatProc.cWaferMaps.insert(lWaferId, lWafer);
         }
         else
         {
            if (it.name()=="length")
                 continue;
            if (!lCompositePatProc.Set(it.name(), it.value().toString()))
                return "error: fail to set "+it.name();
         }
    }

    // Call Z-PAT wafer generation
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteCompositePatProcessing(lCompositePatProc,
                                                                                      mPrivate->mErrorMessage);

    // Save composite file name created
    //QString lCompositeFile = lFields.strCompositeFile;

    // Return error message (if any)
    if (mPrivate->mErrorMessage.isEmpty())
        return lCompositePatProc.strCompositeFile;

    return "error: "+mPrivate->mErrorMessage;
}

bool PATEngine::CreateZPATCompositeFile(const QString &lRecipeFile, const QStringList &lFiles, QString &lCompositeFile)
{
    mPrivate->mErrorMessage.clear();

    CGexCompositePatProcessing  cFields;
    CGexCompositePatWaferInfo   cWafer;
    int                         iWaferID=0;

    // Set options
    cFields.strRecipeFile       = lRecipeFile;		// Recipe name
    cFields.strLogFilePath      = "";				// No .log file created.
    cFields.strCompositeFile    = lCompositeFile; //

    // Loop over all data files selected in the GUI...
    for(iWaferID = 0; iWaferID < lFiles.count(); ++iWaferID)
    {
        cWafer.clear();
        cWafer.iWaferID =  iWaferID;
        cWafer.strSources += lFiles[iWaferID];
        cWafer.strDataFile = lFiles[iWaferID];

        // Add file entry to list
        cFields.cWaferMaps[iWaferID] = cWafer;
    }

    // Call Z-PAT wafer generation
    GS::Gex::Engine::GetInstance().GetSchedulerEngine().ExecuteCompositePatProcessing(cFields, mPrivate->mErrorMessage);

    // Save composite file name created
    lCompositeFile = cFields.strCompositeFile;

    // Return error message (if any)
    if (mPrivate->mErrorMessage.isEmpty())
        return true;
    else
        return false;
}

bool PATEngine::ExportZPATCompositeFileToSINF(const QString &lCompositeFile, const QString &lSINF,
                                              const QString &lDeviceName, const QString &lLotID,
                                              const QString &lWaferID)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
    {
        mPrivate->mErrorMessage = "This feature is not supported by this version of the product.";
        return false;
    }

    // Open Zmap file
    if(QFile::exists(lCompositeFile) == false)
    {
      if(lCompositeFile.isEmpty() == true)
          mPrivate->mErrorMessage = "*PAT Error* Zmap file missing.";
      else
          mPrivate->mErrorMessage = "*PAT Error* Zmap file not found:" + lCompositeFile;

      return false;
    }

    // Open ZMap file
    QFile f_z(lCompositeFile);
    if(!f_z.open( QIODevice::ReadOnly ))
    {
      mPrivate->mErrorMessage = "*PAT Error* Zmap file access denied:" + lCompositeFile;
      return false;
    }

    // Read Zmap and build SINF!
    QString		lLine;
    QTextStream hZmapFile(&f_z);
    bool        bValidZmap = false;

    do
    {
      // Read line.
      lLine = hZmapFile.readLine().trimmed();

      // Zmap file marker
      if(lLine.startsWith("<Exclusion_mask>", Qt::CaseInsensitive))
      {
          bValidZmap = true;
          break;
      }
    }
    while(hZmapFile.atEnd() == false);

    // Check if this is a valid Zmap file
    if(!bValidZmap)
    {
      mPrivate->mErrorMessage = "*PAT Error* Invalid Zmap file :" + lCompositeFile;
      return false;
    }

    // Read Zmap file header
    int	iRowMin = 0;
    int iColMin = 0;
    int iSizeX  = 0;
    int iSizeY  = 0;

    bValidZmap = false;
    do
    {
      // Read line.
      lLine = hZmapFile.readLine().trimmed();

      // Wafer RowMin value
      if(lLine.startsWith("iRowMin:",Qt::CaseInsensitive))
      {
          lLine     = lLine.section(':',1);
          iRowMin   = lLine.toInt();
      }
      // Wafer ColMin value
      if(lLine.startsWith("iColMin:",Qt::CaseInsensitive))
      {
          lLine     = lLine.section(':',1);
          iColMin   = lLine.toInt();
      }
      // Wafer iSizeX value
      if(lLine.startsWith("iSizeX:",Qt::CaseInsensitive))
      {
          lLine     = lLine.section(':',1);
          iSizeX    = lLine.toInt();
      }
      // Wafer iSizeY value
      if(lLine.startsWith("iSizeY:",Qt::CaseInsensitive))
      {
          lLine     = lLine.section(':',1);
          iSizeY    = lLine.toInt();
      }
      // Wafer data starts now!
      if(lLine.startsWith("RowData:",Qt::CaseInsensitive))
      {
          bValidZmap = true;
          break;
      }
    }
    while(hZmapFile.atEnd() == false);

    // If invalid zmap, exit now
    if(!bValidZmap)
    {
      mPrivate->mErrorMessage = "*PAT Error* Invalid Zmap file: " + lCompositeFile;
      return false;
    }

    // Write SINF header file
    // Create SINF map
    QFile f_sinf(lSINF);
    if(!f_sinf.open( QIODevice::WriteOnly ))
    {
      mPrivate->mErrorMessage = "*PAT Error* Failed creating SINF file from Zmap: " + lCompositeFile;
      return false;
    }

    QTextStream hSinfFile(&f_sinf);
    hSinfFile << "DEVICE:" << lDeviceName << endl;
    hSinfFile << "LOT:" << lLotID << endl;
    hSinfFile << "WAFER:" << lWaferID << endl;
    hSinfFile << "FNLOC:0" << endl;
    hSinfFile << "ROWCT:" << iSizeY << endl;
    hSinfFile << "COLCT:" << iSizeX << endl;
    hSinfFile << "BCEQU:01" << endl;
    hSinfFile << "REFPX:" << iColMin << endl;
    hSinfFile << "REFPY:" << iRowMin << endl;
    hSinfFile << "DUTMS:mm" << endl;
    hSinfFile << "XDIES:0" << endl;
    hSinfFile << "YDIES:0" << endl;

    // Read wafermap data
    while(hZmapFile.atEnd() == false)
    {
      // Full wafer processed
      if(lLine.startsWith("</Exclusion_mask>", Qt::CaseInsensitive))
          break;

      // Each die is 1 digit, ' ' is separator: perform substitution
      lLine = lLine.replace(".","__");
      lLine = lLine.replace("0","00");
      lLine = lLine.replace("1","01");

      // Save to SINF
      hSinfFile << lLine << endl;

      // Read line.
      lLine = hZmapFile.readLine().trimmed();
    };

    return true;
}

QString PATEngine::ConvertPatRecipe(const QString & CsvRecipeName,
                              const QString & JsonRecipeName)
{
    QString lOutputFile(JsonRecipeName);
    QString ErrorMsg;
    if (CsvRecipeName == "")
    {
        ErrorMsg = "error : empty CSV input file";
        return ErrorMsg;
    }
    QFile file;
    file.setFileName(CsvRecipeName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ErrorMsg = "error : File " + CsvRecipeName + " doesn't exist or error opening it";
        return ErrorMsg;
    }
    file.close();

    // Create recipe IO from file
    PATRecipeIO* lPatRecipeIO = PATRecipeIO::CreateRecipeIo(CsvRecipeName);

    if(lPatRecipeIO == NULL)
    {
        ErrorMsg = "error : The input file : " + CsvRecipeName + " is not in supported formats (csv/xml or JSON) ";
        return ErrorMsg;
    }
    PATRecipe lPatRecipe;
    if (lPatRecipeIO->Read(lPatRecipe) == false)
    {
        ErrorMsg = lPatRecipeIO->GetErrorMessage();
        delete lPatRecipeIO;
        lPatRecipeIO = NULL;
        return ErrorMsg;
    }
    delete lPatRecipeIO;
    lPatRecipeIO = NULL;

    // write the new format
    // if the JsonRecipeName is empty, use <CsvRecipeName without the .csv extension>.json
    if(lOutputFile ==  "")
    {
        lOutputFile = CsvRecipeName.split(".")[0] + ".json";
    }
    lPatRecipeIO = PATRecipeIO::CreateRecipeIo(PATRecipeIO::JSON);
    if (lPatRecipeIO->Write(lPatRecipe, lOutputFile) == false)
        ErrorMsg = lPatRecipeIO->GetErrorMessage();
    delete lPatRecipeIO;
    lPatRecipeIO = NULL;

    if (ErrorMsg.isEmpty())
        return "ok";
    else
        return ErrorMsg;
}

}
}
