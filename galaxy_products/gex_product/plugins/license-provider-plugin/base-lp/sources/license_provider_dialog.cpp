#include "license_provider_dialog.h"
#include "license_provider_global.h"
#include "license_provider.h"
#include "license_provider_manager.h"
#include "server_file_descriptorIO_factory.h"
#include "server_file_descriptorIO.h"
#include "gqtl_log.h"
#include "license_provider.h"
#include "gex_constants.h"

#include <QMessageBox>
#ifdef _WIN32
#include <io.h>
#endif

///////////////////////////////////////////////////////////
// User changed the 'Running mode' option
///////////////////////////////////////////////////////////
namespace GS
{
namespace LPPlugin
{

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
LicenseProviderDialog::LicenseProviderDialog(LicenseProvider *provider, QWidget* parent, bool modal, Qt::WindowFlags f )
    : QDialog(parent, f)
{
    GEX_ASSERT(provider);
    GSLOG(7, "ActivationKeyDialog constructor...");
    //QString strString;

    setupUi(this);

    tryLegacy = false;

    mProvider = provider;
    QString gbTite = groupBox->title();
    groupBox->setTitle(gbTite + ": " + mProvider->property(LP_FRIENDLY_NAME).toString());

    setModal(modal);
    GSLOG(7, QString(" modal = %1").arg(modal?"true":"false").toLatin1().data() );

    // Apply detected skin
    m_gexSkin.applyPalette(this);

    // Fills the types of GEX running mode available.
    ComboBoxRunningMode->clear();
    // Ensure edit fields are cleared
    editField1->clear();
    editField2->clear();

    // Fix me: get the available modes from the gs_lp.xml or gs_lp.json
    QString lxmlFileDir = mProvider->getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "xml" + QDir::separator() ;
    QString ljsonFileDir = mProvider->getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "json" + QDir::separator() ;

    QString lFileName = ljsonFileDir + JSON_LP_SETTING_FILE;

    if(!QFile::exists(lFileName))
    {
        lFileName = lxmlFileDir + XML_LP_SETTING_FILE;
    }
    ServerFileDescriptorIOFactory* lFactory = ServerFileDescriptorIOFactory::GetInstance();
    ServerFileDescriptorIO* lServerFile = lFactory->GetServerDescriptor(lFileName);

    if(QFile::exists(lFileName) && lServerFile->LoadFile(lFileName) == false)
    {
        QMessageBox::critical(this, "Error", QString("Error while loading file %1, bad format.").arg(lFileName).toLatin1().constData());
    }

    QVariantList lAvailableModes = mProvider->getRunningModeAvailable();
    for (int lIndex=0; lIndex<lAvailableModes.size(); ++lIndex)//, itemList)
    {
        QVariant lMode = lAvailableModes[lIndex];
        QString lText = lMode.toString().section("|",0,0);
        QVariant lUserData = lMode.toString().section("|",1);
        ComboBoxRunningMode->insertItem(lIndex, lText, lUserData);
    }

    QObject::connect(ComboBoxRunningMode, SIGNAL(activated(int)), this, SLOT(OnRunningMode(int)));
    QObject::connect(buttonOk, SIGNAL(clicked()), this, SLOT(OnOkButton()));
    QObject::connect(ButtonExit, SIGNAL(clicked()), this, SLOT(OnExitButton()));
    QObject::connect(buttonLegacy, SIGNAL(clicked()), this, SLOT(OnLegacyButton()));

    // Get the last choice from the gs_lp.xml or .json
//    QVariant lastChoice = mProvider->getLastChoice();
    QString lLastProvider =  lServerFile->LicenseTypeToStr(lServerFile->GetLastUsedLicenseType());

    if(!lLastProvider.isEmpty())
    {
        int idx = ComboBoxRunningMode->findData(lLastProvider, Qt::UserRole, Qt::MatchStartsWith);
//        int idx = ComboBoxRunningMode->findText(lLastProvider, Qt::MatchStartsWith);
        if(idx>=0)
        {
            ComboBoxRunningMode->setCurrentIndex(idx);
            // Fix me: to be changed : getLastUsedProvider to a new function that return "Floating|Server name / IP:|localhost|Socket Port:|4242"
            const QString& lLastChoice = lServerFile->GetLastChoice();

            if(!lLastChoice.isEmpty())
                ComboBoxRunningMode->setItemData(idx, lLastChoice);
            //            ComboBoxRunningMode->setItemData(idx,lastChoice);
        }
        else
            ComboBoxRunningMode->setCurrentIndex(0);
    }
    else
        ComboBoxRunningMode->setCurrentIndex(0);
}

LicenseProviderDialog::~LicenseProviderDialog()
{

    GSLOG(SYSLOG_SEV_DEBUG, "~LicenseProviderDialog ...");
    mProvider = 0;
}

void LicenseProviderDialog::hideLegacyButton(bool legacy)
{
    buttonLegacy->setVisible(legacy);

}

bool LicenseProviderDialog::getLegacyStatus()
{
    return tryLegacy;
}

void	LicenseProviderDialog::OnRunningMode(int)
{
    // -- Set focus on Licensee Name/ Server name edit field
    editField1->setFocus();
    editField1->setEnabled(true);
    editField2->setEnabled(true);

    if(ComboBoxRunningMode->currentIndex()<0)
        return ;

    QStringList fields = ComboBoxRunningMode->itemData(ComboBoxRunningMode->currentIndex()).toString().split("|");

    TextLabel1->hide();
    editField1->hide();
    TextLabel2->hide();
    editField2->hide();

    // -- standalone
   // QString type = property(LP_TYPE).toString();
    if (! fields[0].compare("standalone", Qt::CaseInsensitive) &&
        fields.count() == 5 &&
        ! property(LP_TYPE).toString().compare("gs_lp"))
    {
        TextLabel1->setText(fields[1]);
        TextLabel2->setText(fields[2]);
        editField1->setText(fields[3]);
        editField2->setText(fields[4]);

        TextLabel1->show();
        editField1->show();
        TextLabel2->show();
        editField2->show();
    }

    // -- floating
    if(!fields[0].compare("floating", Qt::CaseInsensitive))
    {
        TextLabel1->setText(fields[1]);
        if(fields.count()==2)
            editField1->setText("");
        else if(fields.count() == 3)
            editField1->setText(fields[2] );

        TextLabel1->show();
        editField1->show();
    }
}

bool LicenseProviderDialog::Init(void)
{

    // Load Welcome HTML page.
    LoadWelcomePage();

    // Updates Dialog box selections.
    OnRunningMode(0);

    return true;
}


///////////////////////////////////////////////////////////
// Load the correct HTML page into the Welcome dialog box.
///////////////////////////////////////////////////////////
void	LicenseProviderDialog::LoadWelcomePage(void)
{
    QString lWelcomePage = mProvider->getWelcomePage();
    if(!lWelcomePage.isEmpty())
        StartupText->setSource(QUrl::fromLocalFile(lWelcomePage) );
}

CGexSkin &LicenseProviderDialog::getGexSkin(){
    return m_gexSkin;
}

int LicenseProviderDialog::getSelectionMode()
{
    QString lModeData   = ComboBoxRunningMode->currentData().toString();
    int     lMode       = GEX_RUNNINGMODE_STANDALONE;

    if (lModeData.startsWith("floating", Qt::CaseInsensitive))
        lMode = GEX_RUNNINGMODE_CLIENT;
    else if (lModeData.startsWith("standalone", Qt::CaseInsensitive))
        lMode = GEX_RUNNINGMODE_STANDALONE;
    else if (lModeData.startsWith("evaluation", Qt::CaseInsensitive))
        lMode = GEX_RUNNINGMODE_EVALUATION;

    return lMode;
}

///////////////////////////////////////////////////////////
// User clicked the 'Next' button
///////////////////////////////////////////////////////////
void	LicenseProviderDialog::OnOkButton(){

    if(editField1->isVisible() && editField1->text().isEmpty())
    {
        QMessageBox::information(0,mProvider->getAppConfigData("AppFullName").toString(),QString("Please fill the field \"%1\"").arg(TextLabel1->text()));
        editField1->setFocus();
        return;
    }
    if(editField2->isVisible() && editField2->text().isEmpty())
    {
        QMessageBox::information(0,mProvider->getAppConfigData("AppFullName").toString(),QString("Please fill the field \"%1\"").arg(TextLabel2->text()));
        editField2->setFocus();
        return;
    }

    mProvider->saveLastChoice(ComboBoxRunningMode->itemData(ComboBoxRunningMode->currentIndex()).toString(),
                              editField1->isVisible() ? QString("%1|%2").arg(TextLabel1->text()).arg(editField1->text()) : QString(),
                              editField2->isVisible() ? QString("%1|%2").arg(TextLabel2->text()).arg(editField2->text()) : QString());

    done(QDialog::Accepted);
}
///////////////////////////////////////////////////////////
// User clicked the 'Exit' button
///////////////////////////////////////////////////////////
void	LicenseProviderDialog::OnExitButton(void)
{
    // Return 0 so to exit from the application...
    done(QDialog::Rejected);
}

void	LicenseProviderDialog::OnLegacyButton(void)
{
    tryLegacy = true;
    done(QDialog::Accepted);
}

}
}

