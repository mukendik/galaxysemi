#include <set>
#include <QFile>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QList>
#include <QTemporaryFile>
#include <QJsonArray>
#include <QFileDialog>
#include "browser_dialog.h"
#include "ofr_controller.h"
#include "ofr_manager.h"
#include "composite.h"
#include "component.h"
#include "gqtl_log.h"
#include "section_element.h"
#include "basic_report_builder.h"
#include "message.h"
#include "gex_report.h"
#include "script_wizard.h"
#include "html_renderer.h"
#include "product_info.h"

extern CGexReport* gexReport;

extern GexMainwindow *	pGexMainWindow;
extern void ConvertToScriptString(QString &strFile);

GS::Gex::OFR_Controller *GS::Gex::OFR_Controller::mInstance = NULL;

/**
 * @brief here we don't return a pointer. This enable to avoid the management of the descruction
 * It will be done at the shut down of GEX
 */
GS::Gex::OFR_Controller *GS::Gex::OFR_Controller::GetInstance()
{
    if(!mInstance)
        mInstance = new OFR_Controller();

   return mInstance;
}

GS::Gex::OFR_Controller::OFR_Controller() :
    mRoot(new Composite("",QJsonObject(), 0, T_ROOT)),
    mIndex(0),
    mLastIndexSectionUpdated(0),
    mManager(0)
{
}

void GS::Gex::OFR_Controller::Destroy()
{
    if(mInstance)
    {
        delete mInstance;
        mInstance = 0;
    }
}

bool GS::Gex::OFR_Controller::IsEmptyReportBuilder() const
{
    return (mRoot->GetElements().count() > 0);
}


GS::Gex::OFR_Controller::~OFR_Controller()
{
    delete(mRoot);
    OFRManager::Destroy();
}

QJsonObject &GS::Gex::OFR_Controller::GetSettings()
{
    return mSettings;
}

void GS::Gex::OFR_Controller::AddElementToSection(int indexSection, const QJsonObject &eltSettings)
{
    if(indexSection > mRoot->GetElements().count())
        return;

    pGexMainWindow->mAddElementInReportBuilderMovie->start();
    SectionElement* lSection = static_cast<SectionElement*>(mRoot->GetElements()[indexSection]);
    lSection->AddElement(eltSettings);

    mLastIndexSectionUpdated = indexSection;

    OFRManager::GetInstance()->AddElementInView(lSection);
}

void GS::Gex::OFR_Controller::Hide()
{
   OFRManager::GetInstance()->hide();
}

void GS::Gex::OFR_Controller::AddElementSettings(const QJsonObject &eltSettings)
{
    pGexMainWindow->mAddElementInReportBuilderMovie->start();
    // Update the structure
    QString lSectionName("Section_"+ QString::number(++mIndex));
    SectionElement* lSection = new SectionElement(lSectionName, mRoot);
    mRoot->AddElement(lSection);

    mLastIndexSectionUpdated = mRoot->GetElements().count() - 1;

    lSection->AddElement(eltSettings);

    mManager = OFRManager::GetInstance();

    // If first element inserted, show the save window
    if (mRoot->GetElementsNumber() == 1 || mManager->isHidden())
    {
        mManager->LoadUI(mRoot);
        mManager->show();
        mManager->SaveIsNeeded();
    }
    else
    {
        mManager->AddElementInView(lSection);
    }
}


void GS::Gex::OFR_Controller::UpdateLastIndexUsedAfterADeletion(int indexDeleted)
{
    if(indexDeleted <= mLastIndexSectionUpdated)
    {
        --mLastIndexSectionUpdated;
    }

    if(mLastIndexSectionUpdated < 0)
    {
        mLastIndexSectionUpdated = 0;
    }
}

bool GS::Gex::OFR_Controller::CanAddNewItem(const QString& sectionName)
{
   return (!mSectionsWithTable.contains(sectionName));
}

QList<QString> GS::Gex::OFR_Controller::BuildListSection()
{
    QList<QString> lListSections;

    mSectionsWithTable.clear();
    const QList<Component*> lSections = mRoot->GetElements();
    QList<Component*>::const_iterator lIterBegin(lSections.begin()), lIterEnd(lSections.end());

    for(;lIterBegin != lIterEnd; ++lIterBegin )
    {
        // We add only in sections that doesn't contain any capability table.
        QList<Component*> lListOfElements = (*lIterBegin)->GetElements();
        // -- may be empty
        if(lListOfElements.count() > 0)
        {
            // -- since we are looking for section with table, only the first element is intersting
            Component* lComponent = lListOfElements[0];
            if (lComponent->GetType() == T_TABLE || lComponent->GetType() == T_CAPABILITY_TABLE_CONNECTED)
                mSectionsWithTable.append((*lIterBegin)->GetName());
        }
        lListSections.append((*lIterBegin)->GetName());
    }

    return lListSections;
}

bool GS::Gex::OFR_Controller::IsReportEmpty()
{
    return mRoot->GetElements().size() == 0;
}


QMenu* GS::Gex::OFR_Controller::BuildContextualMenu(QMenu* parent,
                                                    QAction** lastUpdatedSectionAction,
                                                    QAction** newSectionAction,
                                                    QList<QAction *> &sectionsActions)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
        return NULL;
    QMenu* lMenuSection = new QMenu("Add to the report builder session", parent);
    int lLastIndexSectionUpdated = GetLastIndexSectionUpdated();

    // -- last updated section
    QList<QString> lListSectionsAvalaible = BuildListSection();

    // -- last test, check if lastUpdated is not e table section. We can't add new element in it
    if (lListSectionsAvalaible.size() > 0   &&
            lastUpdatedSectionAction        &&
            CanAddNewItem(lListSectionsAvalaible[lLastIndexSectionUpdated]))
    {
        *lastUpdatedSectionAction = new QAction(lListSectionsAvalaible[lLastIndexSectionUpdated], this);
    }

    // -- new section
    if(newSectionAction)
    {
        *newSectionAction = new QAction(QIcon(":/gex/icons/add.png"), "New section", this);
    }

    // -- existing sections (if equal, already in last updated section)
    if(lListSectionsAvalaible.count() > 1)
    {
        lMenuSection->addSeparator();
        QList<QString>::iterator lIterBegin(lListSectionsAvalaible.begin()), lIterEnd(lListSectionsAvalaible.end());
        for(int lIndex =0; lIterBegin != lIterEnd; ++lIterBegin, ++lIndex)
        {
            if(GS::Gex::OFR_Controller::GetInstance()->CanAddNewItem(*lIterBegin) == false)
            {
                continue;
            }

            QAction* lAction = new QAction((*lIterBegin), lMenuSection);
            lAction->setData(lIndex);
            sectionsActions.append(lAction);
        }
    }

    return lMenuSection;
}

void GS::Gex::OFR_Controller::RefreshView()
{
    if(mManager)
    {
        mManager->UpdateCurrentSelection();
    }
}

void GS::Gex::OFR_Controller::Reset()
{
    mLastIndexSectionUpdated = 0;
    mRoot->Clear();
    mIndex =0;

}

void GS::Gex::OFR_Controller::ResetLastSectionSelected()
{
    mLastIndexSectionUpdated = 0;
}

Component *GS::Gex::OFR_Controller::GetRoot() const
{
    return mRoot;
}

bool GS::Gex::OFR_Controller::GenerateReport(const QString& jsonFileName,
                                             const QString& reportName,
                                             const QMap<QString, QString >& params)
{
    if (GS::LPPlugin::ProductInfo::getInstance()->isExaminator())
        return false;
    QMap<QString, QString >::const_iterator lFormat = params.find("report_format");
    if (lFormat == params.end() )
    {
        GSLOG(SYSLOG_SEV_ERROR, "The output format has to be filled");
    }
    else if(lFormat.value().compare("pdf", Qt::CaseInsensitive) != 0)
    {
        QString lErrorMessage(QString("Unsupported report format (%1)").arg(lFormat.value()));
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    if(!OpenJsonFile(jsonFileName))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error opening the input JSON file %1").arg(jsonFileName)
              .toLatin1().constData());
    }

    if (!LoadJsonFile(jsonFileName))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error loading the input JSON file %1").arg(jsonFileName)
              .toLatin1().constData());
    }

    return GenerateReport(reportName);
}


bool GS::Gex::OFR_Controller::GenerateReport(const QString& pdfReportName)
{
    HtmlRenderer lHtmlRenderer;
    QTemporaryFile lTempoFile;

    if(lTempoFile.open() == false)
    {
        GS::Gex::Message::critical("Error", QString("Unable to open temporary html file %1")
                                            .arg(lTempoFile.fileName()));
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("temporary html file created in %1").arg(lTempoFile.fileName()).toLatin1().data());

    lHtmlRenderer.CreateHTMLDoc(*mRoot, lTempoFile );

    lTempoFile.close();

    QString lApplicationDir = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    QString lImageArgument =  lApplicationDir + QString("/images/gex_logo_transparent.png");

    gexReport->ConvertHtmlToPDF(pdfReportName, lTempoFile.fileName(), lImageArgument,  lTempoFile.fileName());

    // Don't remove this code, it can be used when we use the PDF generator
    /*GenerateHtmldocArgument*/
    /*GS::Gex::PdfReportBuilder report_builder(mSettings);

    // build the report accordingly to the description
    if (mRoot)
    {
        report_builder.Build(*mRoot);
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "The root is empty");
        return false;
    }

    // cancel pressed, exit right now
    if(reportName.isEmpty())
    {
        GSLOG(SYSLOG_SEV_ERROR, "Report file name is empty");
        return false;
    }

    // save the report into a file
    report_builder.SaveAs(reportName.toStdString());*/

    return true;
}

int GS::Gex::OFR_Controller::GetLastIndexSectionUpdated() const
{
    return mLastIndexSectionUpdated;
}

bool GS::Gex::OFR_Controller::OpenJsonFile(const QString &jsonConfigFile)
{
    if(jsonConfigFile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Empty input file");
        return false;
    }
    mJsonFileName = jsonConfigFile;

    QFile json_file( jsonConfigFile );

    // Check file exists
    if (!json_file.exists())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("input file: %1 doesn't exist").arg(jsonConfigFile).toLatin1().constData());
        return false;
    }

    if(! json_file.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cannot open %1 file").arg(jsonConfigFile).toLatin1().constData());
        return false;
    }

    QTextStream lJsonConfigStream(&json_file);
    lJsonConfigStream.setCodec("UTF-8");
    QString lJsonConfig = lJsonConfigStream.readAll();

    QJsonParseError error;
    QJsonDocument lDocument = QJsonDocument::fromJson(lJsonConfig.toUtf8(), &error);
    if (error.error != error.NoError)
    {
        QString lErrorMessage("The Report Builder config file %1 is not JSON format: %2");
        lErrorMessage = lErrorMessage.arg(jsonConfigFile).arg(error.errorString());
        GSLOG(SYSLOG_SEV_WARNING, lErrorMessage.toLatin1().constData());
        return false;
    }


    mSettings = lDocument.object();
    QJsonValue lVersion = mSettings["Version"];
    // Version can be with 1 or 2 decimals. (ie: 2.00 or 2.0)
    QRegExp lRegExpFull("^\\d\\.\\d{1,2}$");
    float   lJsonFormat;

    if (lVersion.isDouble())
    {
        lJsonFormat = (float) lVersion.toDouble();
    }
    else if (lRegExpFull.exactMatch(lVersion.toString()))
    {
        lJsonFormat = lVersion.toString().toFloat();
    }
    else
    {
        QString lErrorMessage = QString("Unable to detect the OFR config version: %1").arg(lVersion.toString());
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    if (lJsonFormat < MIN_OFR_JSON_FORMAT || lJsonFormat > MAX_OFR_JSON_FORMAT)
    {
        QString lErrorMessage =
                QString("Unsupported OFR config version detected: %1. Supported OFR config versions are from %2 to %3")
                .arg(lJsonFormat).arg(MIN_OFR_JSON_FORMAT).arg(MAX_OFR_JSON_FORMAT);
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    return true;
}

bool GS::Gex::OFR_Controller::LoadJsonFile(const QString &jsonConfigFile)
{

    /*if(jsonConfigFile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Empty input file");
        return false;
    }
    mJsonFileName = jsonConfigFile;

    QFile json_file( jsonConfigFile );

    // Check file exists
    if (!json_file.exists())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("input file: %1 doesn't exist").arg(jsonConfigFile).toLatin1().constData());
        return false;
    }

    if(! json_file.open( QIODevice::ReadOnly ))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Cannot open %1 file").arg(jsonConfigFile).toLatin1().constData());
        return false;
    }

    QTextStream lJsonConfigStream(&json_file);
    lJsonConfigStream.setCodec("UTF-8");
    QString lJsonConfig = lJsonConfigStream.readAll();

    QJsonParseError error;
    QJsonDocument lDocument = QJsonDocument::fromJson(lJsonConfig.toUtf8(), &error);
    if (error.error != error.NoError)
    {
        QString lErrorMessage("The Report Builder config file %1 is not JSON format: %2");
        lErrorMessage = lErrorMessage.arg(jsonConfigFile).arg(error.errorString());
        GSLOG(SYSLOG_SEV_WARNING, lErrorMessage.toLatin1().constData());
        return false;
    }


    mSettings = lDocument.object();


    QJsonValue lVersion = mSettings["Version"];
    // Version can be with 1 or 2 decimals. (ie: 2.00 or 2.0)
    QRegExp lRegExpFull("^\\d\\.\\d{1,2}$");
    float   lJsonFormat;

    if (lVersion.isDouble())
    {
        lJsonFormat = (float) lVersion.toDouble();
    }
    else if (lRegExpFull.exactMatch(lVersion.toString()))
    {
        lJsonFormat = lVersion.toString().toFloat();
    }
    else
    {
        QString lErrorMessage = QString("Unable to detect the OFR config version: %1").arg(lVersion.toString());
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    if (lJsonFormat < MIN_OFR_JSON_FORMAT || lJsonFormat > MAX_OFR_JSON_FORMAT)
    {
        QString lErrorMessage =
                QString("Unsupported OFR config version detected: %1. Supported OFR config versions are from %2 to %3")
                .arg(lJsonFormat).arg(MIN_OFR_JSON_FORMAT).arg(MAX_OFR_JSON_FORMAT);
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    */
    delete mRoot;
    mRoot = new Composite(mSettings["Name"].toString(), QJsonObject(), 0, T_ROOT);
    mRoot->SetComment(mSettings["Comment"].toString());


    //-- as the json format does not garanti the order
    //-- we need to sort the Component based on their IndexPostion
    // -- (used in the SortComponent struct
    std::set<Component*, SortComponent> lComponents;

    //-- Loop on sections and create a composite for each one
    QJsonArray lSections = mSettings["Sections"].toArray();
    QJsonArray::const_iterator lSecIter(lSections.begin()), lSecEndIter(lSections.end());
    for (; lSecIter != lSecEndIter; ++lSecIter)
    {
        QJsonObject lSectionObj = (*lSecIter).toObject();
        if (!lSectionObj.empty())
        {
            SectionElement* lSectionElt = new SectionElement(lSectionObj["Name"].toString(), mRoot);
            lSectionElt->LoadJson(lSectionObj);
            lComponents.insert(lSectionElt);
        }
    }

    //--  we can add them in the right order
    std::set<Component*, SortComponent>::iterator lIterBegin(lComponents.begin()), lIterEnd(lComponents.end());
    for(; lIterBegin != lIterEnd; ++lIterBegin)
    {
        mRoot->AddElement(*lIterBegin);
    }

    //-- init the index used to set the name section
    mIndex = lComponents.size();

    //-- build the list of the section that will be displayed
    //-- in the contextual menu
    BuildListSection();

    return true;
}

bool GS::Gex::OFR_Controller::SaveJsonObject(const QString& jsonOFRConfigFile, bool forCsl/*=false*/)
{
    QByteArray lObjectSerializition;
    QFile lFile(jsonOFRConfigFile);
    if (!forCsl)
    {
        mJsonFileName = jsonOFRConfigFile;
    }

    if (lFile.open(QIODevice::WriteOnly) == false)
    {
        QString lErrorMessage = "Can't open to write in the Report Builder config file:" + jsonOFRConfigFile;
        GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().constData());
        return false;
    }

    // convert the json object to a document and write it
    QJsonDocument lDocument;
    QJsonObject lRootObj = mRoot->ToJson();
    lRootObj.insert("Version", Component::mActualVersion);
    lRootObj.insert("Report_name", mReportName);
    lDocument.setObject(lRootObj);
    lObjectSerializition = lDocument.toJson();
    lFile.write(lObjectSerializition);
    lFile.close();

    return true;
}


void GS::Gex::OFR_Controller::OFR_Admin()
{
    mManager = OFRManager::GetInstance();
    if(mManager->isHidden())
    {
        //lManager->Reset();
        mManager->LoadUI(mRoot);
        mManager->show();
    }
}


///////////////////////////////////////////////////////////
// Saves the Report settings into a Script file!
///////////////////////////////////////////////////////////
void GS::Gex::OFR_Controller::SaveReportScript(void)
{
    // Let's user tell where to save the configuration.
    QString lFileDialog = QFileDialog::getSaveFileName(NULL, "Save Examinator Script as...",
                                                       "quantix_report_builder_script.csl",
                                                       "Examinator Script file (*.csl)");

    // If no file selected, ignore command.
    if(lFileDialog.isEmpty())
        return;

    // Copy script xxx/.gex_assistant.csl into 'lFileDialog' file!
    FILE	*lSource;
    FILE	*lDest;
    char	lBuffer[256];

    lSource = fopen(GS::Gex::Engine::GetInstance().GetAssistantScript().toLatin1().constData(),"r");
    if(lSource == NULL)
    {
        GS::Gex::Message::critical("", GEX_T("ERROR: Failed reading script..."));
        return;	// Failed reading source
    }

    lDest = fopen(lFileDialog.toLatin1().constData(),"w");
    if(lDest == NULL)
    {
        fclose(lSource);
        GS::Gex::Message::critical("", GEX_T("ERROR: Failed creating a copy of the script..."));
        return;	// Failed writing to destination
    }

    // Copy file...
    while(!feof(lSource))
    {
        if(fgets(lBuffer,255,lSource) != NULL)
        {
            QString lBufferString(lBuffer);

            if(lBufferString.indexOf("SetReportType") != -1
               &&lBufferString.indexOf("SetReportType();") == -1)
            {
                fputs(lBuffer,lDest);
                fputs("{\n  gexOptions('output','format','pdf');\n", lDest);

                // format the json file
                QFileInfo	lCslFile(lFileDialog);
                QString		lFinalJsonFileName	= lCslFile.absolutePath() + QString("/")
                                                  + lCslFile.baseName() + QString(".json");

                // If destination .grt file already exists, delete it
                if (QFile::exists(lFinalJsonFileName) && QFile::remove(lFinalJsonFileName) == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("unable to remove file %1")
                                            .arg(lFinalJsonFileName).toLatin1().data());

                    GS::Gex::Message::critical("", GEX_T("ERROR: Failed removing an existing copy "
                                                "of the Quantix Report Template file..."));

                    // Close files
                    fclose(lSource);
                    fclose(lDest);

                    // Remove the destination file
                    QFile::remove(lFileDialog);
                    return;
                }

                if (!SaveJsonObject(lFinalJsonFileName, true)/*QFile::copy(mJsonFileName, lFinalJsonFileName) == false*/)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("unable to copy file %1 to %2").arg(mJsonFileName)
                                            .arg(lFinalJsonFileName).toLatin1().data() );

                    GS::Gex::Message::critical("", GEX_T("ERROR: Failed creating a copy "
                                               "of the Quantix Report Template file..."));

                    // Close files
                    fclose(lSource);
                    fclose(lDest);

                    // Remove the destination file
                    QFile::remove(lFileDialog);

                    return;
                }
                ConvertToScriptString(lFinalJsonFileName);

                fprintf(lDest, "  gexReportType('adv_report_builder','template','%s');\n", lFinalJsonFileName.toLatin1().constData());

                fputs("  sysLog('* Quantix Examinator Settings loaded! *');\n", lDest);
                while(lBufferString.indexOf("}") == -1 && !feof(lSource))
                {
                    if(fgets(lBuffer,255,lSource) != NULL)
                    {
                        lBufferString = QString(lBuffer);
                        continue;
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "uncorrect end of file");

                        GS::Gex::Message::critical("", GEX_T("ERROR: Failed creating a copy "
                                                   "of the Quantix Report Template file..."));

                        // Close files
                        fclose(lSource);
                        fclose(lDest);

                        // Remove the destination file
                        QFile::remove(lFileDialog);
                        return;
                    }
                }
            }
            fputs(lBuffer,lDest);
        }
    };

    // Close files
    fclose(lSource);
    fclose(lDest);
}
