#include "license_activation_dialog.h"
#include "gex_version.h"

#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QThread>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QDateEdit>
#include <QInputDialog>
#include <QClipboard>
#include <QFileDialog>
#include <QToolButton>
#include <QPainter>
#include <QPaintEngine>
#include <QPolygon>
#include <QDesktopWidget>
#include <QTemporaryFile>
#include "FlxActError.h"
#include "rborrow_dialog.h"

#include <gqtl_sysutils.h>


namespace GS
{
namespace ActivationGui
{
#define GS_ACTIVIATION_APP_NAME "Quantix License Activation Utility"

#define	GS_ACTIVIATION_APP_VERSION_MAJOR	2
#define	GS_ACTIVIATION_APP_VERSION_MINOR	0

#define GS_ACTIVIATION_FNP_UTILS "fnp_utils"

#define GS_APPACTUTIL_ACTIVATION "appactutil"
#define GS_SERVERACTUTIL_ACTIVATION "serveractutil"
#define GS_RESETAPP_ACTIVATION  "tsreset_app"
#define GS_RESETSVR_ACTIVATION "tsreset_svr"

#ifdef Q_OS_WIN
#define GS_ACTIVIATION_EXT ".exe"
#else
#define GS_ACTIVIATION_EXT ""
#endif

#define GS_FNP_LICENSE_TYPE "-hybrid"



#define GS_ACTIVIATION_PRODUCT_START "Trust Flags:"
#define GS_ACTIVIATION_PRODUCT_FEATURES "Feature line(s):"
#define GS_ACTIVIATION_PRODUCT_PRODUCT_ID "Product ID:"
#define GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID "Entitlement ID:"
#define GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID "Fulfillment ID:"
#define GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE "Fulfillment Type:"
#define GS_ACTIVIATION_PRODUCT_STATUS "Status:"
#define GS_ACTIVIATION_PRODUCT_SUITE_ID "Suite ID:"
#define GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE "Expiration date:"
#define GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL1 "INCREMENT"
#define GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL2 "FEATURE"
#define GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN "Activation Server Chain:"
#define GS_ACTIVIATION_PRODUCT_DEDUCTION "Deduction Type:"

#define GS_ACTIVIATION_PRODUCT_CONCURENT    "Concurrent:"
#define GS_ACTIVIATION_PRODUCT_HYBRID       "Hybrid:"
#define GS_ACTIVIATION_PRODUCT_ACTIVATABLE  "Activatable:"
#define GS_ACTIVIATION_PRODUCT_REPAIR  "Repairs:"
#define GS_ACTIVIATION_PRODUCT_DSN "Destination System Name:"
#define GS_ACTIVIATION_PRODUCT_DEDUCTION_UKNOWN "Deduction Type: UNKNOWN"

#define GS_ACTIVIATION_PRODUCT_INCREMENT_SEPERATOR "!"


#define GS_ACTIVIATION_UAT_URL "https://galaxysemiuat.flexnetoperations.com/control/glxy/ActivationService"
#define GS_ACTIVIATION_LIVE_URL "https://galaxysemi.flexnetoperations.com/control/glxy/ActivationService"

#define GS_ACTIVIATION_SHOW_CMD_DATA "-srd"
#define GS_ACTIVIATION_ENV_PORTAL "-uat"
#define GS_ACTIVIATION_APP "-appact"
#define GS_ACTIVIATION_SERVER "-srvact"


class ProcessWorker : public QThread
{
public:
    ProcessWorker(QObject *parent =0 );
    virtual ~ProcessWorker();
    bool getProcessStartedSatus()
    {
        return mProcessStartedSatus;
    }

    bool getProcessFinishedSatus()
    {
        return mProcessFinishedSatus;
    }
    QProcess *getProcess();
    QByteArray getStandardOutput();
    QByteArray getErrorOutput();
    void updateTempFiles();
    void startPorcess(const QString &program, const QStringList &arguments);

private:
   void run();
   QProcess *mProcess;
   QTemporaryFile mStandardTempFile;
   QTemporaryFile mErrorTempFile;
   bool mProcessStartedSatus;
   bool mProcessFinishedSatus;

};

QProcess *ProcessWorker::getProcess()
{
    return mProcess;
}

void ProcessWorker::updateTempFiles()
{
    mStandardTempFile.close();
    mStandardTempFile.open();
    mErrorTempFile.close();
    mErrorTempFile.open();
    mProcess->setStandardOutputFile(mStandardTempFile.fileName());
    mProcess->setStandardErrorFile(mErrorTempFile.fileName());
}

void ProcessWorker::startPorcess(const QString &program, const QStringList &arguments)
{
    updateTempFiles();
    mProcess->start(program,arguments);

}

QByteArray ProcessWorker::getStandardOutput()
{
     QTextStream lStream(&mStandardTempFile);
     return lStream.readAll().toLatin1();

}

QByteArray ProcessWorker::getErrorOutput()
{
    QTextStream lStream(&mErrorTempFile);
    return lStream.readAll().toLatin1();
}

ProcessWorker::ProcessWorker(/*QProcess *utilityProcess, */QObject *parent):QThread(parent)
{
    mProcess = new QProcess;
    mProcess->moveToThread(this);
    mStandardTempFile.open();
    mErrorTempFile.open();
    mProcess->setStandardOutputFile(mStandardTempFile.fileName());
    mProcess->setStandardErrorFile(mErrorTempFile.fileName());
    Q_ASSERT(mProcess);
    mProcessStartedSatus = true;
    mProcessFinishedSatus = true;

}

ProcessWorker::~ProcessWorker()
{
    mProcess->deleteLater();
}

void ProcessWorker::run()
{
    mProcessStartedSatus = true;
    mProcessFinishedSatus = true;

    if (!mProcess->waitForStarted(-1))
        mProcessStartedSatus = false ;

    if (!mProcess->waitForFinished(-1))
        mProcessFinishedSatus = false ;

}

LicenseActivation::LicenseActivation():QMainWindow()
{
    QStringList appArgument;
    if (qApp)
        appArgument = qApp->arguments();

    setupUi(this);
    initActivation();
    QString error;
    if(getError(error) != eNoError)
        return ;

    QString lGexBuild = QString("Built with Quantix Products (%1)").arg(GEX_APP_VERSION);

    setWindowTitle( QCoreApplication::applicationName() + " - "
                    + QCoreApplication::applicationVersion() + " - " + lGexBuild);

    connect(mActivateLicense, SIGNAL(clicked()), this, SLOT(activateLicense()));
    connect(mRequestEvaluation, SIGNAL(clicked()), this, SLOT(requestEvaluation()));
    connect(mRawData, SIGNAL(clicked()), this, SLOT(viewRawData()));
    connect(mGalaxyProdProperty, SIGNAL(clicked()), this, SLOT(viewProductProperty()));

    mLocalProductsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    mServerProductsTree->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(mLocalProductsTree,      SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(operationMenu(const QPoint &)));
    QObject::connect(mServerProductsTree,      SIGNAL(customContextMenuRequested(const QPoint &)),
                     this, SLOT(operationMenu(const QPoint &)));

    mTerminalOutput->setVisible(false);

    connect(mLocalRepair, SIGNAL(clicked()), this, SLOT(localRepair()));
    connect(mResetApp, SIGNAL(clicked()), this, SLOT(resetApp()));
    connect(mResetServer, SIGNAL(clicked()), this, SLOT(resetServer()));

    mGlobalOperation->hide();
    mActivatedLicenseType->clear();
    int appAct = appArgument.indexOf(GS_ACTIVIATION_APP);
    int srvAct = appArgument.indexOf(GS_ACTIVIATION_SERVER);

//    if(appAct != -1 )
//        mActivatedLicenseType->insertItem(0,QIcon(QString::fromUtf8(":/gsacutil/icons/local_license.png")),"Local activated Licenses", eLocalActivated);
//    if(srvAct != -1)
//        mActivatedLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/server_license.png")),"Served activated Licenses", eServerActivated);

//    if(appAct == -1 && srvAct == -1)
//    {
        mActivatedLicenseType->insertItem(0,
            QIcon(QString::fromUtf8(":/gsacutil/icons/local_license.png")),"Local activated Licenses", eLocalActivated);
        mActivatedLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/server_license.png")),
                                          "Served activated Licenses", eServerActivated);
//    }


//    mActivatedLicenseType->insertItem(0,QIcon(QString::fromUtf8(":/gsacutil/icons/local_license.png")),"Local activated Licenses", eLocalActivated);
//    mActivatedLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/server_license.png")),"Served activated Licenses", eServerActivated);
    mActivatedLicenseType->setCurrentIndex(0);
    connect(mViewActivatedLicense, SIGNAL(clicked()), this, SLOT(viewActivatedLicense()));
    connect(mActivatedLicenseType, SIGNAL(currentIndexChanged(int)), this, SLOT(activatedLicenseType(int)));
    activatedLicenseType(mActivatedLicenseType->currentIndex());




    mActivationLicenseType->clear();
    if(appAct != -1 )
    {
        mActivationLicenseType->insertItem(0,QIcon(QString::fromUtf8(":/gsacutil/icons/node_locked_license.png")),"Node-locked License", GS_APPACTUTIL_ACTIVATION);
//        mActivationLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/node_locked_license.png")),"Node-locked counted License", GS_APPACTUTIL_ACTIVATION);
    }
    if(srvAct != -1)
        mActivationLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/floating_license.png")),"Floating Licenses", GS_SERVERACTUTIL_ACTIVATION);
    if(appAct == -1 && srvAct == -1)
    {
        mActivationLicenseType->insertItem(0,QIcon(QString::fromUtf8(":/gsacutil/icons/node_locked_license.png")),"Node-locked License", GS_APPACTUTIL_ACTIVATION);
//        mActivationLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/node_locked_license.png")),"Node-locked counted License", GS_APPACTUTIL_ACTIVATION);
        mActivationLicenseType->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/floating_license.png")),"Floating Licenses", GS_SERVERACTUTIL_ACTIVATION);
    }

    mActivationLicenseType->setCurrentIndex(0);
    specifyQty(0);
    connect(mActivationLicenseType, SIGNAL(currentIndexChanged(int)), this, SLOT(specifyQty(int)));

    mActivationModeCombo->clear();
    mActivationModeCombo->insertItem(0,QIcon(QString::fromUtf8(":/gsacutil/icons/online.png")),"Online activation", eOnline);
    mActivationModeCombo->insertItem(1,QIcon(QString::fromUtf8(":/gsacutil/icons/offline.png")),"Offline(manual) activation", eOffline);
    connect(mActivationModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(activationModeGUIUpdate(int)));
    connect(mActivationReqGB, SIGNAL(clicked(bool)) ,this,SLOT(activationModeGUIUpdateRspGB(bool)));
    connect(mActivationRspGB, SIGNAL(clicked(bool)) ,this,SLOT(activationModeGUIUpdateReqGB(bool)));
    mActivationModeCombo->setCurrentIndex(0);
    activationModeGUIUpdate(mActivationModeCombo->currentIndex());


    mActivationReqGB->setChecked(true);
    connect(mGenActiveReqFile, SIGNAL(clicked(bool)),this,SLOT(getReqFileName(bool)));
    connect(mRdActiveRspFile, SIGNAL(clicked(bool)),this,SLOT(getRspFileName(bool)));
    connect(mOpGenFile, SIGNAL(clicked(bool)),this,SLOT(getOpGenFileFileName(bool)));

    mStatusLabel = new QLabel (statusBar());
    mProgressBar = new QProgressBar(statusBar());
    mStatusLabel->hide();
    mProgressBar->hide();
    statusBar()->addPermanentWidget(mStatusLabel);
    statusBar()->addPermanentWidget(mProgressBar);

    mRawData->setParent(statusBar());
    statusBar()->addPermanentWidget(mRawData);

    mGalaxyProdProperty->setParent(statusBar());
    statusBar()->addPermanentWidget(mGalaxyProdProperty);
    //mGalaxyProdProperty->hide();

    int argIdx = appArgument.indexOf(GS_ACTIVIATION_SHOW_CMD_DATA);
    if( argIdx == -1)
    {
        mRawData->hide();
    }

    argIdx = appArgument.indexOf(GS_ACTIVIATION_ENV_PORTAL);
    if( argIdx == -1)
    {
        mGS_ACTIVIATION_URL = GS_ACTIVIATION_LIVE_URL;
    }
    else
    {
        mGS_ACTIVIATION_URL = GS_ACTIVIATION_UAT_URL;
    }

    connect(mAddKey, SIGNAL(clicked()), this, SLOT(additionalKey()));
    mActivationView->hide();

    QMap<QString, int> lHeaderItemsContent = getHeaderItemContent(mLocalProductsTree);
    int lColIdx = lHeaderItemsContent["Version"];
    mLocalProductsTree->hideColumn(lColIdx);

    lHeaderItemsContent.clear();
    lHeaderItemsContent = getHeaderItemContent(mServerProductsTree);
    lColIdx = lHeaderItemsContent["Version"];
    mServerProductsTree->hideColumn(lColIdx);
}

LicenseActivation::~LicenseActivation()
{

}

int LicenseActivation::initActivation()
{
    QCoreApplication::setOrganizationName("GalaxySemi");
    QCoreApplication::setOrganizationDomain("galaxysemi.com");
    QCoreApplication::setApplicationName(GS_ACTIVIATION_APP_NAME);
    QCoreApplication::setApplicationVersion(QString("v%1.%2")
                                        .arg(GS_ACTIVIATION_APP_VERSION_MAJOR).arg(GS_ACTIVIATION_APP_VERSION_MINOR));
    setError(LicenseActivation::eNoError, "");
    intErrorCodeMapping();
    QString applicationDir;
    CGexSystemUtils::GetApplicationDirectory(applicationDir);
    mFNPUtilsDir = applicationDir + QDir::separator() + GS_ACTIVIATION_FNP_UTILS;

    if(!QDir().exists(mFNPUtilsDir))
    {
        setError(LicenseActivation::eMissingUtilsDir, QString("The Utils dir is missing in <%1>").arg(mFNPUtilsDir));
        return LicenseActivation::eMissingUtilsDir;
    }

    mUtilsNeeded.append(QString(GS_APPACTUTIL_ACTIVATION) + GS_ACTIVIATION_EXT);
    mUtilsNeeded.append(QString(GS_SERVERACTUTIL_ACTIVATION) + GS_ACTIVIATION_EXT);
    mUtilsNeeded.append(QString(GS_RESETAPP_ACTIVATION) + GS_ACTIVIATION_EXT);
    mUtilsNeeded.append(QString(GS_RESETSVR_ACTIVATION) + GS_ACTIVIATION_EXT);

    foreach(QString prog, mUtilsNeeded)
    {
        if(!QFile::exists(mFNPUtilsDir + QDir::separator() + prog))
        {
            setError(LicenseActivation::eMissingUtils,
                     QString("The utility \"%1\" is missing from %2").arg(prog).arg(mFNPUtilsDir));
            return LicenseActivation::eMissingUtils;
        }
    }

    initTabs();

    mProcessWorker = new ProcessWorker();
    mFNPUtility = mProcessWorker->getProcess();
    return LicenseActivation::eNoError;
}

int LicenseActivation::initTabs()
{
    mActivationTabs->removeTab(mActivationTabs->indexOf(mEvaluation));
    mEvaluation->deleteLater();

    mActivationTabs->setCurrentWidget(mActivation);
    return LicenseActivation::eNoError;
}

int LicenseActivation::getError(QString &error)
{
    error = mErrorMessage;
    return mError;
}

void LicenseActivation::setError(int error, const QString &message)
{
    mError = error;
    mErrorMessage = message;
}

void LicenseActivation::viewRawData()
{
    bool visible = mTerminalOutput->isVisible();
    mTerminalOutput->setVisible(!visible);

}

void LicenseActivation::viewBuildProperty()
{


}

void LicenseActivation::updateTerminal(const QByteArray &result)
{
    mTerminalOutput->clear();
    mTerminalOutput->setText(result);
}

void LicenseActivation::specifyQty(int)
{
    QString utilityName = mActivationLicenseType->itemData(mActivationLicenseType->currentIndex()).toString();
    if(utilityName == GS_SERVERACTUTIL_ACTIVATION)
    {
        mSpecifyQuantityLicense->show();
        if(mActivationModeCombo->itemData(mActivationModeCombo->currentIndex()).toInt() == eOnline)
             mSpecifyQuantityLicense->setDisabled(false);
        else
        {
            mSpecifyQuantityLicense->setChecked(true);
            mSpecifyQuantityLicense->setDisabled(true);
        }
    }
    else
        mSpecifyQuantityLicense->hide();
}

void LicenseActivation::activationModeGUIUpdate(int)
{
    if( mActivationModeCombo->itemData(mActivationModeCombo->currentIndex()).toInt() == eOnline)
    {
        mActivationReqGB->setVisible(false);
        mActivationRspGB->setVisible(false);
        mAddKey->setVisible(true);

        mSpecifyQuantityLicense->setDisabled(false);
        mSpecifyQuantityLicense->setChecked(false);

    }
    else
    {
        mActivationReqGB->setVisible(true);
        mActivationRspGB->setVisible(true);
        mAddKey->setVisible(false);
        QList<QWidget *> addKeyItems = mActKeyWidget->findChildren <QWidget *> ( "AdditionalKey");
        if(!addKeyItems.isEmpty())
        {
            foreach(QWidget *suppKey, addKeyItems)
            {
                if(suppKey)
                {
                    if(suppKey->parentWidget()){
                        if(suppKey->parentWidget()->layout())
                            suppKey->parentWidget()->layout()->removeWidget(suppKey);
                        suppKey->deleteLater();
                        suppKey->setParent(0);
                    }

                }

            }
            addKeyItems.clear();
        }

        QString utilityName = mActivationLicenseType->itemData(mActivationLicenseType->currentIndex()).toString();
        if(utilityName == GS_SERVERACTUTIL_ACTIVATION)
        {
            mSpecifyQuantityLicense->show();
            mSpecifyQuantityLicense->setChecked(true);
            mSpecifyQuantityLicense->setDisabled(true);
        }
        else
            mSpecifyQuantityLicense->hide();

    }
}

void LicenseActivation::activationModeGUIUpdateRspGB(bool )
{
    mActivationRspGB->setChecked(!mActivationReqGB->isChecked());

}

void LicenseActivation::activationModeGUIUpdateReqGB(bool )
{
    mActivationReqGB->setChecked(!mActivationRspGB->isChecked());
}

void LicenseActivation::getReqFileName(bool )
{
    QString reqFileName =  QFileDialog::getSaveFileName(this,
                                "Generation of activation request file",
                                QDir::homePath () + QDir::separator() + "activation_request.xml",
                                "XML file(*.xml)");
    if(!reqFileName.isEmpty())
    {
        mActiveReqFile->setText(QDir::toNativeSeparators(reqFileName));
        if(QFile::exists(reqFileName))
            QFile::remove(reqFileName);
    }
}

void LicenseActivation::getOpGenFileFileName(bool )
{
    QString opGenFileName =  QFileDialog::getSaveFileName(this,
                                                          "Out put filename for offline operation",
                                                          QDir::homePath () + QDir::separator() + "output_file.xml",
                                                          "XML file(*.xml)");
    if(!opGenFileName.isEmpty())
    {
        mOpFile->setText(QDir::toNativeSeparators(opGenFileName));
        if(QFile::exists(opGenFileName))
            QFile::remove(opGenFileName);
    }


}

void LicenseActivation::localRepair()
{
    mTerminalOutput->clear();

    QString utilityName = GS_APPACTUTIL_ACTIVATION;
    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument =  QStringList()<< "-localrepair";

    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                      utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Local Repair", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not local repair");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not local repair");
        return ;
    }

    QByteArray result =mProcessWorker->getStandardOutput();
    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() + result +
            QByteArray("\n-------------------------------\n");

    if(result.contains("ERROR:"))
    {
        //ERROR: Local repair - (%d,%d,%d)
        if(result.startsWith("ERROR: Local repair - ("))
        {
            QString error = result;
            error = error.section("ERROR: Local repair - (",1,1);
            int errorCode = error.section(",",0,0).toInt();
            QString displayedError = result;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Can not localy repair license. Error %1 :%2\n ").arg(errorCode).arg(displayedError));
        }
        else
        {
            QString resultString = result;
            QString error = resultString.section("ERROR:",1,-1);
            information(QString("Can not localy repair licenses \n error : %2").arg(error));
        }
    }
    else
        information("locally repair done");

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);


}

void LicenseActivation::resetApp()
{
    int ret = QMessageBox::critical(this,"Reset node-locked licenses" ,
                                    "Are you sure to reset node-locked licenses? This operation can not be repaired",
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(ret == QMessageBox::No)
        return ;

    mTerminalOutput->clear();

    QString utilityName = GS_RESETAPP_ACTIVATION;
    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument =  QStringList()<< "-delete" << "all";

    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Local Repair", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not reset licenses");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not reset licenses");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();
    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() + result
            + QByteArray("\n-------------------------------\n");

    //    Performing reset operation, please wait...
    //    Trusted Storage Contents have been reset...

    if(result.contains("Trusted Storage Contents have been reset..."))
    {
        information("Licenses Contents have been reset...");
    }
    else
        information(QString("Can not reset Licenses content and return : \n %1").arg(QString(result)));

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);


}

void LicenseActivation::resetServer()
{
    int ret = QMessageBox::critical(this,"Reset floating licenses" ,
                                    "Are you sure to reset floating licenses? This operation can not be repaired",
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(ret == QMessageBox::No)
        return ;

    mTerminalOutput->clear();

    QString utilityName = GS_RESETSVR_ACTIVATION;
    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument =  QStringList()<< "-delete" << "all";;

    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Local Repair", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not reset licenses");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not reset licenses");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();
    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() + result
            + QByteArray("\n-------------------------------\n");

    //    Performing reset operation, please wait...
    //    Trusted Storage Contents have been reset...

    if(result.contains("Trusted Storage Contents have been reset..."))
    {
        information("Licenses Contents have been reset...");
    }
    else
        information(QString("Can not reset Licenses content and return : \n %1").arg(QString(result)));

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);

}

void LicenseActivation::getRspFileName(bool )
{
    QString rspFileName = QFileDialog::getOpenFileName(this, "Processing of activation response file",
                                                   QDir::homePath () + QDir::separator() + "activation_response.xml",
                                                       "XML file(*.xml)");

    if(!rspFileName.isEmpty())
        mActiveRspFile->setText(QDir::toNativeSeparators(rspFileName));
}

void LicenseActivation::activatedLicenseType(int)
{
    if( mActivatedLicenseType->itemData(mActivatedLicenseType->currentIndex()).toInt() == eServerActivated)
    {
        mLicenseServerWidget->show();
        mGlobalOperation->hide();
        mStackedLicense->setCurrentWidget(mServerLicense);
    }
    else
    {
        mLicenseServerWidget->hide();
        mGlobalOperation->show();
        mStackedLicense->setCurrentWidget(mLocalLicense);
    }
}

void LicenseActivation::viewActivatedLicense()
{
    centralWidget()->setEnabled(false);
    if (qApp) qApp->processEvents();

    if( mActivatedLicenseType->itemData(mActivatedLicenseType->currentIndex()).toInt() == eLocalActivated)
    {
        mLocalProductsTree->clear();
        //node-locked license
        QString utilityName = GS_APPACTUTIL_ACTIVATION;
        QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;

        mProcessWorker->startPorcess(utility, QStringList() << "-view" << "-long");
        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            updateStatusBarLabel(eBoth, "Retrieving license information", 5);
            if (qApp) qApp->processEvents();
        }
        if(!mProcessWorker->getProcessStartedSatus())
        {
            information("Can not retrieve license installed");
            return ;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            information("Can not retrieve license installed");
            return ;
        }
        //QByteArray result = mFNPUtility->readAll();
        QByteArray result = mProcessWorker->getStandardOutput();
        result += mProcessWorker->getErrorOutput();
        QByteArray rawData = QByteArray("\n---Local node locked licenses ---\n") + result +
                QByteArray("\n-------------------------------\n");

        bool bNoLocalNodeLocked = false;
        bool lAppLibNotFound = false;
        if(result.contains("No fulfillment records in trusted storage") || result.isEmpty())
            bNoLocalNodeLocked = true;
        else if(result.contains("ERROR: Activation library initialization failed"))
            lAppLibNotFound = true;
        else
        {
            processFoundLicense(mLocalProductsTree, eLocalActivatedCPU,"Node-locked license", result);

        }

        result.clear();

        //floating license
        utilityName = GS_SERVERACTUTIL_ACTIVATION;
        utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;

        mProcessWorker->startPorcess(utility, QStringList() << "-view" << "-long");
        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            updateStatusBarLabel(eBoth, "Retrieving license information", 5);
            if (qApp) qApp->processEvents();
        }
        if(!mProcessWorker->getProcessStartedSatus())
        {
            information("Can not retrieve license installed");
            return ;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            information("Can not retrieve license installed");
            return ;
        }
        //result = mFNPUtility->readAll();
        result = mProcessWorker->getStandardOutput();
        result += mProcessWorker->getErrorOutput();
        rawData += QByteArray("\n---Local floating licenses ---\n") + result +  QByteArray("\n-------------------------------\n");

        bool bNoLocalFloating = false;
        bool lSrvLibNotFound = false;
        if(result.contains("No fulfillment records in trusted storage") || result.isEmpty())
            bNoLocalFloating = true;
        else if(result.contains("ERROR: Activation library initialization failed"))
            lSrvLibNotFound = true;
        else
        {
            processFoundLicense(mLocalProductsTree, eLocalActivatedFloating,"Floating license", result);

        }

        if(bNoLocalFloating && bNoLocalNodeLocked)
            information("No Installed licenses found");
        if(lSrvLibNotFound || lAppLibNotFound)
            information("Internal error please check that the installed Galaxy-la is not corrupted and that it contains libFNP library");

        updateTerminal(rawData);
    }
    else if( mActivatedLicenseType->itemData(mActivatedLicenseType->currentIndex()).toInt() == eServerActivated)
    {
        mServerProductsTree->clear();
        if(mLicenseServerName->text().isEmpty())
        {
            information("Please enter a server name or IP");
            mLicenseServerName->setFocus();
        }
        else if(mLicenseServerPort->text().isEmpty())
        {
            information("Please enter a server port");
            mLicenseServerPort->setFocus();
        }
        else
        {
            QString utilityName = GS_APPACTUTIL_ACTIVATION;
            QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
            //-serverview -commServer 27000@localhost -long
            mProcessWorker->startPorcess(utility,
                               QStringList() << "-serverview"
                               << "-commServer"
                               << QString("%1@%2").arg(mLicenseServerPort->text()).arg(mLicenseServerName->text())
                               << "-long");
            mProcessWorker->start();
            while(mProcessWorker->isRunning())
            {
                updateStatusBarLabel(eBoth, "Retrieving license information", 5);
                if (qApp) qApp->processEvents();
            }
            if(!mProcessWorker->getProcessStartedSatus())
            {
                information("Can not retrieve license installed");
                return ;
            }

            if(!mProcessWorker->getProcessFinishedSatus())
            {
                information("Can not retrieve license installed");
                return ;
            }

            //QByteArray result = mFNPUtility->readAll();
            QByteArray result = mProcessWorker->getStandardOutput();
            result += mProcessWorker->getErrorOutput();
            QByteArray rawData =
                    QByteArray("\n--Server licenses  ---\n")
                    + QString("Server : %1@%2\n").arg(mLicenseServerPort->text()).arg(mLicenseServerName->text()).toLatin1()
                    + result +  QByteArray("\n-------------------------------\n");

            if(result.contains("No fulfillment records in server trusted storage. License Server") || result.isEmpty())
            {
                information(QString("No Installed licenses found in license server %1").arg(mLicenseServerName->text()));
            }
            else if(result.contains("ERROR:") || result.isEmpty())
            {
                if(result.contains("ERROR: Activation library initialization failed"))
                {
                    information("Internal error please check that the installed Galaxy-la is not corrupted and that it contains libFNP library");
                }
                else
                {
                    if(result.contains("ERROR: flxActCommonLicSpcPopulateAllFromServerTS"))
                    {
                        //ERROR: flxActCommonLicSpcPopulateAllFromServerTS (50041,25008,0)
                        QString error = result;
                        error = error.section("ERROR: flxActCommonLicSpcPopulateAllFromServerTS ",1,1);
                        int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                        QString displayedError = error;
                        if(mErrorCodeMapping.contains(errorCode))
                            displayedError = mErrorCodeMapping[errorCode];

                        information(QString("Can not view installed licenses in server %1. Error %2 :%3\n ")
                                    .arg(mLicenseServerPort->text()+"@"+mLicenseServerName->text()).arg(errorCode).arg(displayedError));

                    }
                    else
                    {

                        QString resultString = result;
                        QString error = resultString.section("ERROR:",1,1);
                        information(QString("Can not view installed licenses in server %1 due to error :\n %2")
                                    .arg(mLicenseServerPort->text()+"@"+mLicenseServerName->text()).arg(error));
                    }
                }
            }
            else
            {
                processFoundLicense(mServerProductsTree, eServerActivated,"Server Activated Licenses", result);

            }

            updateTerminal(rawData);

        }
    }
    else
    {
        information(QString("Internal error uknown view mode")); //Should never happen
    }
    updateStatusBarLabel(eHide,"",0);
    centralWidget()->setEnabled(true);
}

void LicenseActivation::processFoundLicense(QTreeWidget *productsTree, LicenseActivatedType type, const QString &licenseNodeLabel, QByteArray &result)
{
    QTextStream resultData(result);
    QVariantMap productData;
    QStringList featureData;
    QString line;
    do
    {
        if(line.startsWith(GS_ACTIVIATION_PRODUCT_START))//We found a new product process it
        {
            //This is a new product read until we reach GS_ACTIVIATION_PRODUCT_START or end of file
            do
            {
                if(line.startsWith(GS_ACTIVIATION_PRODUCT_DEDUCTION))
                {
                    do
                    {
                        line = resultData.readLine();
                        //                        "Activatable:"
                        //                        "Repairs:"
                        //                        "Destination System Name:"
                        //                        "Deduction Type: UNKNOWN"
                        if(line.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVATABLE)
                                || line.startsWith(GS_ACTIVIATION_PRODUCT_REPAIR)
                                || line.startsWith(GS_ACTIVIATION_PRODUCT_DSN)
                                || line.startsWith(GS_ACTIVIATION_PRODUCT_DEDUCTION_UKNOWN) )
                        {
                            if(line.startsWith(GS_ACTIVIATION_PRODUCT_DSN))
                            {
                                line = resultData.readLine();
                                if(!line.startsWith("Expiration Date:"))
                                    break;
                            }
                            else
                                break;
                        }
                    }
                    while(!line.isNull() && !resultData.atEnd());

                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_FEATURES))
                {
                    //Retrive the feature item
                    do
                    {
                        line = resultData.readLine();
                    }
                    while(line.isEmpty() && !resultData.atEnd());

                    QString featureItem;
                    while(line.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL2) || line.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL1))
                    {
                        featureItem.clear();
                        while(true)
                        {
                            featureItem += line + GS_ACTIVIATION_PRODUCT_INCREMENT_SEPERATOR;
                            line = resultData.readLine();
                            if(line.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL2) || line.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL1))
                                break;
                            if(line.isNull())
                                break;
                            if(line.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN))
                            {
                                productData.insert(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN, line.section(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN,1,1));
                                break;
                            }
                        }
                        featureData.append(featureItem);
                    }
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_PRODUCT_ID))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_PRODUCT_ID, line.section(GS_ACTIVIATION_PRODUCT_PRODUCT_ID,1,1));

                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID, line.section(GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID, line.section(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE, line.section(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_STATUS))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_STATUS, line.section(GS_ACTIVIATION_PRODUCT_STATUS,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_SUITE_ID))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_SUITE_ID, line.section(GS_ACTIVIATION_PRODUCT_SUITE_ID,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE, line.section(GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN, line.section(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVATABLE))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_ACTIVATABLE, line.section(GS_ACTIVIATION_PRODUCT_ACTIVATABLE,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_HYBRID))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_HYBRID, line.section(GS_ACTIVIATION_PRODUCT_HYBRID,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_CONCURENT))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_CONCURENT, line.section(GS_ACTIVIATION_PRODUCT_CONCURENT,1,1));
                }
                else if(line.startsWith(GS_ACTIVIATION_PRODUCT_START))
                {
                    productData.insert(GS_ACTIVIATION_PRODUCT_START, line.section(GS_ACTIVIATION_PRODUCT_START,1,1));
                }
                line = resultData.readLine();
            }while(!line.isNull() && !line.startsWith(GS_ACTIVIATION_PRODUCT_START) && !resultData.atEnd());
            //We have all the data for current product show it in the GUI
            if(!productData.isEmpty() && !featureData.isEmpty())
                addLicenseToTreeWidget(productsTree, type, licenseNodeLabel, productData, featureData);
            productData.clear();
            featureData.clear();
        }
        else
            line = resultData.readLine();

    }while(!line.isNull() && !resultData.atEnd());


    productsTree->expandAll();
    for(int columnIdx=0; columnIdx<productsTree->columnCount();++columnIdx)
    {
        productsTree->resizeColumnToContents(columnIdx);
    }
}

QMap<QString, int> LicenseActivation::getHeaderItemContent(QTreeWidget *productsTree)
{
    QTreeWidgetItem *headerItem = productsTree->headerItem();
    QMap<QString, int> headerItemContent ;
    for(int idx=0; idx<headerItem->columnCount(); ++idx)
    {
        headerItemContent.insert(headerItem->text(idx), idx);
    }
    return headerItemContent;
}

void LicenseActivation::addLicenseToTreeWidget(QTreeWidget *productsTree, LicenseActivatedType type, const QString &licenseNodeLabel,QVariantMap &productData, QStringList &featureData)
{
    QTreeWidgetItem *headerItem = productsTree->headerItem();
    QMap<QString, int> headerItemContent ;
    for(int idx=0; idx<headerItem->columnCount(); ++idx)
    {
        headerItemContent.insert(headerItem->text(idx), idx);
    }

    int licenseTypeColIdx = headerItemContent["License Type"];

    QTreeWidgetItem *itemProduct = 0;

    QTreeWidgetItem *rootNodeItem = 0;
    for(int rootItemsIdx = 0; rootItemsIdx < productsTree->topLevelItemCount() ; ++rootItemsIdx )
    {
        if(productsTree->topLevelItem(rootItemsIdx)->text(licenseTypeColIdx) == licenseNodeLabel)
            rootNodeItem = productsTree->topLevelItem(rootItemsIdx);
    }

    if(!rootNodeItem)
    {
        rootNodeItem = new QTreeWidgetItem(productsTree);
        rootNodeItem->setText(licenseTypeColIdx, licenseNodeLabel.trimmed());
    }
    itemProduct = new QTreeWidgetItem(rootNodeItem);



    foreach(QString productKey, productData.keys())
    {
        QString itemData = productData[productKey].toString();
        QString columnLabel = productKey.remove(":");

        if(headerItemContent.contains(columnLabel))
        {
            int columIdx = headerItemContent[columnLabel];
            itemProduct->setText(columIdx, itemData.trimmed());
        }
    }

    checkItem(itemProduct, eProduct, headerItemContent);


    int productMulti = 0;
    if(type != eLocalActivatedCPU)
    {
        if(productData.contains(GS_ACTIVIATION_PRODUCT_CONCURENT) && productData[GS_ACTIVIATION_PRODUCT_CONCURENT].toInt() != 0)
        {
            productMulti = productData[GS_ACTIVIATION_PRODUCT_CONCURENT].toInt();

        }
        else if(productData.contains(GS_ACTIVIATION_PRODUCT_HYBRID) && productData[GS_ACTIVIATION_PRODUCT_HYBRID].toInt() != 0)
        {
            productMulti = productData[GS_ACTIVIATION_PRODUCT_HYBRID].toInt();

        }
        else if(productData.contains(GS_ACTIVIATION_PRODUCT_ACTIVATABLE) && productData[GS_ACTIVIATION_PRODUCT_ACTIVATABLE].toInt() != 0)
        {
            productMulti = productData[GS_ACTIVIATION_PRODUCT_ACTIVATABLE].toInt();
        }
    }
    else
    {
        productMulti = 1;
    }

    int qtyIdx = headerItemContent["Quantity"];
    itemProduct->setText(qtyIdx, QString::number(productMulti));
    if(!productMulti)
    {
        for(int col =0; col < itemProduct->columnCount(); ++col)
            itemProduct->setBackgroundColor(col, QColor(Qt::red));
    }

    //INCREMENT GEX galaxy 7.10000 19-apr-2014 1 VENDOR_STRING
    int featureIdx = headerItemContent["Feature"];
    int versionIdx = headerItemContent["Version"];
    int expirationIdx = headerItemContent["Expiration date"];
    foreach(QString current, featureData)
    {
        QStringList itemsString = current.split(" ");
        if(itemsString.count()<6)
            continue;
        QTreeWidgetItem *itemFeature = new QTreeWidgetItem(itemProduct);
        itemFeature->setText(featureIdx, itemsString[1].trimmed());
        itemFeature->setText(versionIdx, itemsString[3].trimmed());
        itemFeature->setText(expirationIdx, itemsString[4].trimmed());
        itemFeature->setText(qtyIdx, itemsString[5].trimmed());
        checkItem(itemFeature, eFeature, headerItemContent);
    }
    if (qApp) qApp->processEvents();

}

void LicenseActivation::checkItem(QTreeWidgetItem *item, LicenseItemType type, QMap<QString, int> &headerItemContent )
{
    int experationIdx = headerItemContent["Expiration date"];
    int statusIdx = headerItemContent["Status"];
    int trustFlag = headerItemContent["Trust Flags"];
    if(type == eProduct)
    {
        if(item->text(statusIdx) == "**DISABLED**")
        {
            for(int col =0; col < item->columnCount(); ++col)
                item->setBackgroundColor(col,Qt::red);
            return;
        }
        if(item->text(trustFlag) != "FULLY TRUSTED")
        {
            for(int col =0; col < item->columnCount(); ++col)
                item->setBackgroundColor(col,Qt::red);
            return;
        }


    }

    if(type == eFeature || type == eProduct)
    {
        QDate expirationDate = converToDate(item->text(experationIdx));
        QDate currentDate = QDate::currentDate();
        int daysCount = currentDate.daysTo(expirationDate);

        QColor color(Qt::white);
        bool updateColor = false;

        if(daysCount <0 )
        {
            //expired
            color = QColor(Qt::red);
            updateColor = true;
        }
        else if(daysCount <= 10)
        {
            color = QColor::fromRgb(255,165,0);//Orange	255-165-0	ffa500
            updateColor = true;

        }
        else if(daysCount < 30)
        {
            color = QColor::fromRgb(255,140,0); //Dark Orange	255-140-0	ff8c00
            updateColor = true;
        }

        if(updateColor)
        {
            for(int col =0; col < item->columnCount(); ++col)
                item->setBackgroundColor(col,color);
            return;
        }

    }
}

void LicenseActivation::activateLicense()
{
    if(mActivationLicenseCode->text().isEmpty())
    {
        information("Please enter the Activation Code");
        mActivationLicenseCode->setFocus();
        return ;
    }
    QStringList keysList;

    keysList.append(mActivationLicenseCode->text());

    if (qApp) qApp->processEvents();
    QString proxyDetails = "";
    if(!getProxyDetails(proxyDetails))
        return;

    centralWidget()->setEnabled(false);
    if(mActivationModeCombo->itemData(mActivationModeCombo->currentIndex()).toInt() == eOnline)
    {
        QList<QWidget *> addKeyItems = mActKeyWidget->findChildren <QWidget *> ( "AdditionalKey");
        if(!addKeyItems.isEmpty())
        {
            foreach(QWidget *suppKey, addKeyItems)
            {
                QLineEdit *keyField = suppKey->findChild<QLineEdit*>();
                if(keyField)
                {
                    if(keyField->text().isEmpty())
                    {
                        information("Please enter the Activation Code");
                        keyField->setFocus();
                        return ;
                    }
                }
                keysList.append(keyField->text());
            }
            addKeyItems.clear();
        }

        activateLicenseOnline(proxyDetails, keysList);
    }
    else
    {
        activateLicenseOffline(proxyDetails);
    }
    centralWidget()->setEnabled(true);
}

int LicenseActivation::getLicenseQty(int &licenseQty, const QString &entitlementID, bool &bEnterManualy)
{
    if(mSpecifyQuantityLicense->isChecked() || bEnterManualy)
    {
        bool ok;
        licenseQty = QInputDialog::getInt(this, QString("Number of license to activate for activation code %1").arg(entitlementID),
                                          "Quantity:", 1, 1, 2000, 1, &ok);
        if (!ok)
            return false;
        else
            return true;
    }
    else {

        QString utilityName = mActivationLicenseType->itemData(mActivationLicenseType->currentIndex()).toString();
        QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;

        QString proxyDetails;
        getProxyDetails(proxyDetails);
        QStringList proxyDetailsList;
        if(!proxyDetails.isEmpty())
            proxyDetailsList = QStringList() << "-proxyDetails" << proxyDetails;
        QStringList utilArgument = QStringList() << "-served"
                                                 << "-comm" << "soap"
                                                 << "-commServer" << mGS_ACTIVIATION_URL
                                                 << proxyDetailsList
                                                 << "-entitlementID" << entitlementID; //mActivationLicenseCode->text();
        utilArgument.append(GS_FNP_LICENSE_TYPE);
        utilArgument.append("200000");

        mTerminalOutput->append(utility + " " +utilArgument.join(" "));

        mProcessWorker->startPorcess(utility, utilArgument);
        QString result;
        QString data;
        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            //data = mFNPUtility->readAllStandardOutput();
            data = mProcessWorker->getStandardOutput();
            result.append(data);
             mTerminalOutput->append(data);
            //data = mFNPUtility->readAllStandardError();
            data= mProcessWorker->getErrorOutput();
            result.append(data);
            mTerminalOutput->append(data);
            if (qApp) qApp->processEvents();
        }



        if(!mProcessWorker->getProcessStartedSatus())
        {
            bEnterManualy = true;
            return false;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            bEnterManualy = true;
            return false;
        }
        //data = mFNPUtility->readAll();
        data = mProcessWorker->getStandardOutput();
        data += mProcessWorker->getErrorOutput();

        result.append(data);
        mTerminalOutput->append(data);
        if (qApp) qApp->processEvents();


        if(result.isEmpty() || !result.contains("The quantity specified exceeds maximum quantity allowed"))
        {
            if(result.contains("Operations error: 0 That activation request yields no right to a license"))
            {
                 information(QString("That activation request (%1) yields no right to a license ").arg(entitlementID));
                 return false;
            }
            else
            {
                bEnterManualy = true;
                return false;
            }
        }
        else {
            QString quantitySring = result.section("The quantity specified exceeds maximum quantity allowed", 1,1);
            mTerminalOutput->append(quantitySring);
            bool ok;
            mTerminalOutput->append(quantitySring);
            quantitySring = quantitySring.section("(",1,1);
            mTerminalOutput->append(quantitySring);
            quantitySring = quantitySring.section(")",-2,-2);
            mTerminalOutput->append(quantitySring);
            int quantityCount = quantitySring.toInt(&ok);
            if(ok && quantityCount>0)
            {
                licenseQty = quantityCount;
                return true;
            }
            else
            {
                if(quantityCount == 0)
                {
                    information(QString("You have already activate all your licenses for entitelement  (%1) ").arg(entitlementID));
                }
                return false;
            }

        }


        return true;
    }
}

QTreeWidgetItem *LicenseActivation::addActivationEntry(const QString &entitlementID, const QString &qty)
{
    QTreeWidgetItem * treeWidgetItem = 0;
    treeWidgetItem = new QTreeWidgetItem(mActivationTreeWidget);
    treeWidgetItem->setText(0, entitlementID);
    treeWidgetItem->setText(1, qty.isEmpty() ? QString("NA") : qty);
    return treeWidgetItem;
}

void LicenseActivation::viewDetails ()
{
    QTreeWidgetItem *treeWidgetItemSelection = mActivationTreeWidget->itemAt(mActivationTreeWidget->viewport()->mapFromGlobal(QCursor::pos()));
    if(treeWidgetItemSelection)
        mActivationTreeWidget->setCurrentItem(treeWidgetItemSelection);
    else
        return ;
    QDialog dialog(this);
    dialog.setWindowTitle("Activation details");

    QGridLayout *gridLayout = new QGridLayout(&dialog);

    QTextEdit *textEdit = new QTextEdit(&dialog);
    textEdit->setText(treeWidgetItemSelection->data(3,Qt::UserRole).toString());
    gridLayout->addWidget(textEdit, 0, 0, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(&dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    gridLayout->addWidget(buttonBox, 1, 0, 1, 1);

    connect(buttonBox, SIGNAL(accepted()),&dialog, SLOT(accept()));

    dialog.exec();
}

void LicenseActivation::addActivationDetails(QTreeWidgetItem *treeWidgetItem, const QString &data)
{
    QWidget *widget = new QWidget;

    QHBoxLayout* horizontalLayout = new QHBoxLayout(widget);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *labelDetail = new QLabel(widget);
    labelDetail->setText("open details dialog");
    horizontalLayout->addWidget(labelDetail);


    QToolButton *details = new QToolButton(widget);
    details->setText("...");
    QObject::connect(details, SIGNAL(clicked()), this, SLOT(viewDetails()));

    horizontalLayout->addWidget(details);

    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);
    horizontalLayout->addItem(spacer);

    mActivationTreeWidget->setItemWidget(treeWidgetItem, 3, widget);
    treeWidgetItem->setData(3,Qt::UserRole,data);
}

void LicenseActivation::activateLicenseOnline(const QString &proxyDetails, const QStringList &keysList)
{

    //    *appactutil -served [-comm <flex|soap>]
    //                           [-commServer <comm server>]
    //                           [-proxyDetails "<host> <port> [<user id>] [<password>]"]
    //                           [-entitlementID <entitlement_ID>]
    //                           [-reason <reason_number>]
    //                           [-productID <product_ID>]
    //                           [-expiration <expiration_date>]
    //                           [-duration <duration_in_seconds>]
    //                           [-vendordata <key> <value>]
    //                           [-gen [<output_filename>]]

    // appactutil -served -comm soap -commServer https://galaxysemiuat.flexnetoperations.com/control/glxy/ActivationService -entitlementID 6463-65DE-6739-D267


    QString utilityName = mActivationLicenseType->itemData(mActivationLicenseType->currentIndex()).toString();
    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;

//    mActivationView->clear();
//    updateTerminal("");
    mActivationTreeWidget->clear();
    foreach(QString entitlementID, keysList)
    {
        mActivationView->clear();
        updateTerminal("");

        int licenseQty = 1;
        QString licenseQtyOption;
        if(utilityName == GS_SERVERACTUTIL_ACTIVATION)
        {
            bool bEnterManualy = false;
            if(!getLicenseQty(licenseQty, entitlementID,bEnterManualy))
            {
                if(bEnterManualy && qApp)
                {
                    QString lInfo = "The license activation application was not able to obtain the license quantity number from the license manager portal(Possible cause you are facing network issue)"
                                    "\nWould you like to enter it manually ?\n"
                                    "Either you can restart the activation shortly\n";
                    QMessageBox::StandardButton lButton = QMessageBox::information(this, qApp->applicationName() + " - " + qApp->applicationVersion(), lInfo, QMessageBox::Yes,QMessageBox::No);
                    if(lButton == QMessageBox::Yes)
                    {
                        getLicenseQty(licenseQty, entitlementID,bEnterManualy);
                    }
                    else
                    {
                        updateStatusBarLabel(eHide,"",0);
                        return ;
                    }

                }
                updateStatusBarLabel(eHide,"",0);
                return ;
            }

            licenseQtyOption = QString::number(licenseQty);
            centralWidget()->setEnabled(false);
        }
        QStringList proxyDetailsList;
        if(!proxyDetails.isEmpty())
            proxyDetailsList = QStringList() << "-proxyDetails" << proxyDetails;
        QStringList utilArgument = QStringList() << "-served"
                                                 << "-comm" << "soap"
                                                 << "-commServer" << mGS_ACTIVIATION_URL
                                                 << proxyDetailsList
                                                 << "-entitlementID" << entitlementID
                                                // Dummy expiration date to avoid error when launching the fnp utility the right will be retrieved from FNOOD
                                                 << "-expiration" << "31-dec-2090";


        if(!licenseQtyOption.isEmpty())
        {
            utilArgument.append(GS_FNP_LICENSE_TYPE);
            utilArgument.append(licenseQtyOption);
        }

        QTreeWidgetItem *currentItem = addActivationEntry(entitlementID, licenseQtyOption);

#ifdef QT_DEBUG
        mActivationView->append(utility + " " +utilArgument.join(" "));
        mTerminalOutput->append(utility + " " +utilArgument.join(" "));
#endif

        mProcessWorker->startPorcess(utility, utilArgument);

        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            //QString data = mFNPUtility->readAllStandardOutput();
            QString data = mProcessWorker->getStandardOutput();
            if(!data.isEmpty())
            {
                if(!data.startsWith("Expiration ")
                        && !data.startsWith("Activatable ")
                        && !data.startsWith("Concurrent ")
                        && !data.startsWith("Hybrid ")
                        && !data.startsWith("Repair ")
                        && !data.startsWith("Expiration "))
                {
                    mActivationView->append(data);
                    mTerminalOutput->append(data);
                }
            }
            //data = mFNPUtility->readAllStandardError();
            data = mProcessWorker->getErrorOutput();
            if(!data.isEmpty())
            {
                if(!data.startsWith("Expiration ")
                        && !data.startsWith("Activatable ")
                        && !data.startsWith("Concurrent ")
                        && !data.startsWith("Hybrid ")
                        && !data.startsWith("Repair ")
                        && !data.startsWith("Expiration "))
                {
                    mActivationView->append(data);
                    mTerminalOutput->append(data);
                }
            }

            updateStatusBarLabel(eBoth, "ACTIVATION REQUEST", 5);
            if (qApp) qApp->processEvents();
        }
        if(!mProcessWorker->getProcessStartedSatus())
        {
            information(QString("Can not start %1").arg(utility));
            return ;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            return ;
        }

        QString result = mActivationView->toPlainText();
        if(result.isEmpty() || result == utility + " " +utilArgument.join(" "))
        {
            //result = mFNPUtility->readAll();
            result = mProcessWorker->getStandardOutput();
            result += mProcessWorker->getErrorOutput();

            if(!result.isEmpty())
            {
                if(result.contains("Expiration = "))
                {
                    result.remove("Expiration = 31-dec-2090");
                }
                mActivationView->append(result);
            }
        }
        if(!result.contains("ACTIVATION REQUEST SUCCESSFULLY PROCESSED") && !result.contains("TRANSFER REQUEST SUCCESSFULLY PROCESSED"))
        {
            QString resultString = result;
            if(result.contains("Operations error:"))
            {
                QString error = resultString.section("Operations error: ",1,1);
                information(QString("Activation Fails and return \nOperations error: %1").arg(error));

            }
            else if(result.contains("ERROR: flxActAppActivationReasonSet "))
            {
                QString error = resultString.section("ERROR: flxActAppActivationReasonSet ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = error;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];

                information(QString("Activation Fails and return \nOperations error(%1): %2").arg(errorCode).arg(displayedError));

            }
            else if(result.contains("ERROR: flxActAppActivationReqSet "))
            {
                QString error = resultString.section("ERROR: flxActAppActivationReqSet ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = error;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];

                information(QString("Activation Fails and return \nOperations error(%1): %2").arg(errorCode).arg(displayedError));

            }
            else if(result.contains("ERROR: flxActAppActivationSend"))
            {
                QString error = resultString.section("ERROR: flxActAppActivationSend - ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = error;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];

                information(QString("Activation Fails and return \nOperations error(%1): %2").arg(errorCode).arg(displayedError));

            }
            else if(result.contains("ERROR: flxActSvrActivationCreate: Could not determine 3-Server status "))
            {
                QString error = resultString.section("ERROR: flxActSvrActivationCreate: Could not determine 3-Server status ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = error;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];

                information(QString("Activation Fails and return \nOperations error(%1): %2").arg(errorCode).arg(displayedError));

            }
            else if(result.contains("ERROR: flxActSvrActivationSend - "))
            {
                QString error = resultString.section("ERROR: flxActSvrActivationSend - ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = error;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];

                information(QString("Activation Fails and return \nOperations error(%1): %2").arg(errorCode).arg(displayedError));

            }
            else if(result.contains("ERROR: flxActSvrActivationReqSet - "))
            {
                QString error = resultString.section("ERROR: flxActSvrActivationReqSet - ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = error;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];

                information(QString("Activation Fails and return \nOperations error(%1): %2").arg(errorCode).arg(displayedError));

            }
            currentItem->setText(2,"FAIL");

        }
        else
        {
            currentItem->setText(2,"SUCCESS");
//            information("ACTIVATION REQUEST SUCCESSFULLY PROCESSED");
        }

        addActivationDetails(currentItem, result);
        mTerminalOutput->append(result);
        for(int idx=0; idx<mActivationTreeWidget->columnCount(); idx++)
        {
            mActivationTreeWidget->resizeColumnToContents(idx);
        }
    }
    updateStatusBarLabel(eHide,"",0);
}

void LicenseActivation::activateLicenseOffline(const QString &proxyDetails)
{
    //appactutil -served -comm soap -commServer https://galaxysemiuat.flexnetoperations.com/control/glxy/ActivationService -entitlementID 23B7-1A63-2FA8-3510 -gen D:\activation_request.txt

    mActivationTreeWidget->clear();

    if(mActivationReqGB->isChecked())
    {
        if(mActiveReqFile->text().isEmpty())
        {
            information("Please specify an activation request filename");
            mActiveReqFile->setFocus();
            return ;
        }

        //    *appactutil -served [-comm <flex|soap>]
        //                           [-commServer <comm server>]
        //                           [-proxyDetails "<host> <port> [<user id>] [<password>]"]
        //                           [-entitlementID <entitlement_ID>]
        //                           [-reason <reason_number>]
        //                           [-productID <product_ID>]
        //                           [-expiration <expiration_date>]
        //                           [-duration <duration_in_seconds>]
        //                           [-vendordata <key> <value>]
        //                           [-gen [<output_filename>]]

        // appactutil -served -comm soap -commServer https://galaxysemiuat.flexnetoperations.com/control/glxy/ActivationService -entitlementID 6463-65DE-6739-D267



        QString utilityName = mActivationLicenseType->itemData(mActivationLicenseType->currentIndex()).toString();
        QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;

        int licenseQty = 1;
        QString licenseQtyOption;
        if(utilityName == GS_SERVERACTUTIL_ACTIVATION)
        {
            bool bEnterManualy = true;
             if(!getLicenseQty(licenseQty, mActivationLicenseCode->text(), bEnterManualy))
             {
                 updateStatusBarLabel(eHide,"",0);
                 return ;
             }

             licenseQtyOption = QString::number(licenseQty);
        }


        QString activationReqFileName = mActiveReqFile->text();
        QStringList proxyDetailsList;
        if(!proxyDetails.isEmpty())
            proxyDetailsList = QStringList() << "-proxyDetails" << proxyDetails;
        QStringList utilArgument = QStringList() << "-served"
                                                 << "-comm" << "soap"
                                                 << "-commServer" << mGS_ACTIVIATION_URL
                                                 << proxyDetailsList
                                                 << "-entitlementID" << mActivationLicenseCode->text()
                                                 << "-gen" << activationReqFileName
                                                    // Dummy expiration date to avoid error when launching the fnp utility the right will be retrived from FNOOD
                                                 << "-expiration" << "31-dec-2090";

        if(!licenseQtyOption.isEmpty())
        {
            utilArgument.append(GS_FNP_LICENSE_TYPE);
            utilArgument.append(licenseQtyOption);
        }

        mActivationView->clear();
        updateTerminal("");

        QTreeWidgetItem *currentItem = addActivationEntry(mActivationLicenseCode->text(), licenseQtyOption);
#ifdef QT_DEBUG
        mActivationView->append(utility + " " +utilArgument.join(" "));
        mTerminalOutput->append(utility + " " +utilArgument.join(" "));
#endif

        mProcessWorker->startPorcess(utility, utilArgument);

        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            //QString data = mFNPUtility->readAllStandardOutput();
            QString data = mProcessWorker->getStandardOutput();
            if(!data.isEmpty())
            {
                mActivationView->append(data);
                mTerminalOutput->append(data);
            }
            //data = mFNPUtility->readAllStandardError();
            data = mProcessWorker->getErrorOutput();
            if(!data.isEmpty())
            {
                mActivationView->append(data);
                mTerminalOutput->append(data);
            }

            updateStatusBarLabel(eBoth, "Generating ACTIVATION REQUEST", 5);
            if (qApp) qApp->processEvents();
        }
        if(!mProcessWorker->getProcessStartedSatus())
        {
            information(QString("Can not start %1").arg(utility));
            return ;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            return ;
        }

        QString result = mActivationView->toPlainText();
        mTerminalOutput->append(result);
        if(result.isEmpty() || result == utility + " " +utilArgument.join(" "))
        {
            //result = mFNPUtility->readAll();
            result = mProcessWorker->getStandardOutput();
            result += mProcessWorker->getErrorOutput();
            if(!result.isEmpty())
            {
                mActivationView->append(result);
            }
        }
        if(result.contains("ERROR:"))
        {
            currentItem->setText(2,"FAIL");
            information(QString("Activation request generation fails for file :%1 \n").arg(mActiveReqFile->text()));
            return ;

        }
        else
        {
            currentItem->setText(2,"SUCCESS");
        }

        addActivationDetails(currentItem, result);
        mTerminalOutput->append(result);
        for(int idx=0; idx<mActivationTreeWidget->columnCount(); idx++)
        {
            mActivationTreeWidget->resizeColumnToContents(idx);
        }

        updateStatusBarLabel(eHide,"",0);


    }
    else if(mActivationRspGB->isChecked())
    {
        if(mActiveRspFile->text().isEmpty())
        {
            information("Please specify an activation response filename");
            mActiveRspFile->setFocus();
            return ;
        }
        //appactutil.exe -process d:\activationFNOODXml.xml
        QString utilityName = mActivationLicenseType->itemData(mActivationLicenseType->currentIndex()).toString();
        QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;

        QString activationReqFileName = mActiveRspFile->text();
        QStringList utilArgument = QStringList() << "-process" << activationReqFileName;
        mActivationView->clear();
        updateTerminal("");

        mActivationView->clear();
        updateTerminal("");

        QTreeWidgetItem *currentItem = addActivationEntry(mActivationLicenseCode->text(),"");

#ifdef QT_DEBUG
        mActivationView->append(utility + " " +utilArgument.join(" "));
        mTerminalOutput->append(utility + " " +utilArgument.join(" "));
#endif

        mProcessWorker->startPorcess(utility, utilArgument);

        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            //QString data = mFNPUtility->readAllStandardOutput();
            QString data = mProcessWorker->getStandardOutput();
            if(!data.isEmpty())
            {
                mActivationView->append(data);
                mTerminalOutput->append(data);
            }
            //data = mFNPUtility->readAllStandardError();
            data = mProcessWorker->getErrorOutput();
            if(!data.isEmpty())
            {
                mActivationView->append(data);
                mTerminalOutput->append(data);
            }

            updateStatusBarLabel(eBoth, "PROCESSING ACTIVATION RESPONSE", 5);
            if (qApp) qApp->processEvents();
        }
        if(!mProcessWorker->getProcessStartedSatus())
        {
            information(QString("Can not start %1").arg(utility));
            return ;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            return ;
        }

        QString result = mActivationView->toPlainText();
        mTerminalOutput->append(result);
        if(result.isEmpty() || result == utility + " " +utilArgument.join(" "))
        {
            //result = mFNPUtility->readAll();
            result = mProcessWorker->getStandardOutput();
            result += mProcessWorker->getErrorOutput();
            if(!result.isEmpty())
            {
                mActivationView->append(result);
            }
        }
        if(result.contains("ERROR:"))
        {

            if(result.contains("ERROR: Processing response"))//ERROR: Processing response - (50020,41094,0)
            {
                QString error = result.section("ERROR: Processing response - ",1,1).section("(",1,1).section(",",0,0);
                if(mErrorCodeMapping.contains(error.toInt()))
                {
                    QString errorDisplayed =  mErrorCodeMapping[error.toInt()];
                    information(QString("Processing activation response fails for file :%1.\nError %2 : %3")
                                .arg(mActiveRspFile->text()).arg(error).arg(errorDisplayed));
                }
            }
            else if(result.contains("flxActSvrActivationRespProcess"))//ERROR: flxActSvrActivationRespProcess - (50020,42061,0)
            {
                QString error = result.section("ERROR: flxActSvrActivationRespProcess - ",1,1).section("(",1,1).section(",",0,0);
                if(mErrorCodeMapping.contains(error.toInt()))
                {
                    QString errorDisplayed =  mErrorCodeMapping[error.toInt()];
                    information(QString("Processing activation response fails for file :%1.\nError %2 : %3")
                                .arg(mActiveRspFile->text()).arg(error).arg(errorDisplayed));
                }
            }
            else
                information(QString("Processing activation response fails for file :%1 \n").arg(mActiveRspFile->text()));

            currentItem->setText(2,"FAIL");
        }
        else
        {
            currentItem->setText(2,"SUCCESS");
        }
        addActivationDetails(currentItem, result);
        mTerminalOutput->append(result);
        for(int idx=0; idx<mActivationTreeWidget->columnCount(); idx++)
        {
            mActivationTreeWidget->resizeColumnToContents(idx);
        }

        updateStatusBarLabel(eHide,"",0);
    }else
        information("Should never Happen");

}

void LicenseActivation::operationMenu (const QPoint &/*pos*/)
{

    QTreeWidget *productsTree = qobject_cast<QTreeWidget *>(sender());
    if(!productsTree)
        return ;
    QTreeWidgetItem* selectedItem = 0;
    if( productsTree->selectedItems().count())
        selectedItem = productsTree->selectedItems().first();

    QTreeWidgetItem *itemAtPos = productsTree->itemAt(productsTree->viewport()->mapFromGlobal(QCursor::pos())) ;

    if ( !selectedItem || itemAtPos != selectedItem || productsTree->currentIndex().data().isNull())
        return ;

    QMap<QString, int> headerItemContent = getHeaderItemContent(productsTree);
    QMenu *contextMenu = new QMenu(productsTree);
    QAction *copyData = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/copy.png")),"Copy as text");    //add default item
    QAction *borrow = 0;
    QAction *rborrow = 0;
    QAction *repair = 0;
    QAction *returnLicense = 0;
    QAction *deleteFID = 0;
    QAction *deletePROD = 0;

    if(selectedItem && selectedItem->parent())
    {
        if( mActivatedLicenseType->itemData(mActivatedLicenseType->currentIndex()).toInt() == eServerActivated)
        {
            if(selectedItem->parent()->text(0) == "Server Activated Licenses")
            {
                borrow = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/borrow.png")),"Borrow license");
            }
        }
        else
        {
            if(selectedItem->parent()->text(0) == "Node-locked license")
            {
                QString fulfillmentType =  selectedItem->text(headerItemContent["Fulfillment Type"]);
                if(fulfillmentType == "SERVED ACTIVATION" || fulfillmentType == "SERVED OVERDRAFT ACTIVATION")
                {
                    rborrow = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/rborrow.png")),"Return borrow license");
                }
            }
            if(selectedItem->parent()->text(0) == "Node-locked license" || selectedItem->parent()->text(0) == "Floating license")
            {
                QString trustFlage = selectedItem->text(headerItemContent["Trust Flags"]);
                if(trustFlage != "FULLY TRUSTED")
                {
                    repair = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/repair.png")),"Repair broken license");
                }
                QString fulfillmentType =  selectedItem->text(headerItemContent["Fulfillment Type"]);
                if(fulfillmentType == "PUBLISHER ACTIVATION" || fulfillmentType == "PUBLISHER OVERDRAFT ACTIVATION")
                {
                    returnLicense = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/node_locked_license.png")),"Return license");
                }
                if(selectedItem->parent()->text(0) == "Node-locked license" )
                    deletePROD = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/warning_reset.png")),"Delete Product");

                deleteFID  = contextMenu->addAction(QIcon(QString::fromUtf8(":/gsacutil/icons/warning_reset.png")),"Delete Fulfillment");

            }
        }
    }



    //QRect rect = productsTree->visualItemRect ( selectedItem);

    QAction *action = contextMenu->exec(QCursor::pos());
    if(!action)
    {
        delete contextMenu;
        return ;
    }
    if(action == copyData)
    {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(productsTree->currentIndex().data().toString());
    }
    else if(action == borrow)
    {
        //appactutil.exe -borrow -commServer 27000@localhost -entitlementID ENTL-GEX-test1 -productID GEX-test -expiration 22-Apr-2013
        //doBorrow(const QString &commServer, const QString &entitlementID, const QString & productID )
        if(mLicenseServerName->text().isEmpty())
        {
            information("Please enter a server name or IP");
            mLicenseServerName->setFocus();
            delete contextMenu;
            return ;
        }
        else if(mLicenseServerPort->text().isEmpty())
        {
            information("Please enter a server port");
            mLicenseServerPort->setFocus();
            delete contextMenu;
            return ;
        }
        doBorrow(QString("%1@%2").arg(mLicenseServerPort->text()).arg(mLicenseServerName->text()),
                 selectedItem->text(headerItemContent["Entitlement ID"]),
                selectedItem->text(headerItemContent["Product ID"]));
    }
    else if(action == rborrow)
    {
        //appactutil -rborrow <fulfillmentID>
        //           [-commServer <comm server>]
        //appactutil -rborrow FID-1366422136 -commServer 27000@localhost
        QString commServer;
        if(selectedItem->text(headerItemContent["Activation Server Chain"]).split("|").count()>0)
        {
            commServer = selectedItem->text(headerItemContent["Activation Server Chain"]).split("|").first();
        }

        //CommServer seems to be optional.
        returnBorrow(selectedItem->text(headerItemContent["Fulfillment ID"]), commServer);
    }
    else if(action == repair)
    {
        LicenseActivatedType itemType = eLocalActivatedFloating;
        if( selectedItem->parent()->text(0) == "Node-locked license" )
        {
            itemType = eLocalActivatedCPU;
        }

        QString commServer = selectedItem->text(headerItemContent["Activation Server Chain"]);
        if(selectedItem->text(headerItemContent["Activation Server Chain"]).split("|").count()>0)
        {
            QString lTsComServer = selectedItem->text(headerItemContent["Activation Server Chain"]).split("|").first();
            if(!lTsComServer.startsWith("http"))
                commServer = "@"+lTsComServer;
            else
                commServer = lTsComServer;
        }

        repairLicense(itemType,
                      selectedItem->text(headerItemContent["Fulfillment ID"]),
                commServer);
    }
    else if(action == returnLicense)
    {
        LicenseActivatedType itemType = eLocalActivatedFloating;
        if( selectedItem->parent()->text(0) == "Node-locked license" )
        {
            itemType = eLocalActivatedCPU;
        }
        QString commServer = selectedItem->text(headerItemContent["Activation Server Chain"]);
        if(selectedItem->text(headerItemContent["Activation Server Chain"]).split("|").count()>0)
        {
            commServer = selectedItem->text(headerItemContent["Activation Server Chain"]).split("|").first();
        }

        returnLicenseTo(itemType,
                        selectedItem->text(headerItemContent["Fulfillment ID"]),
                commServer);
    }
    else if(action == deleteFID)
    {
        LicenseActivatedType itemType = eLocalActivatedFloating;
        if( selectedItem->parent()->text(0) == "Node-locked license" )
        {
            itemType = eLocalActivatedCPU;
        }
        deleteFulfillment(itemType, selectedItem->text(headerItemContent["Fulfillment ID"]));

    }
    else if(action == deletePROD)
    {
        LicenseActivatedType itemType = eLocalActivatedFloating;
        if( selectedItem->parent()->text(0) == "Node-locked license" )
        {
            itemType = eLocalActivatedCPU;
        }
        deleteProduct(itemType, selectedItem->text(headerItemContent["Product ID"]), selectedItem->text(headerItemContent["Fulfillment ID"]));
    }


    centralWidget()->setEnabled(true);
    delete contextMenu;
}



void LicenseActivation::doBorrow(const QString &commServer, const QString &entitlementID, const QString & productID )
{

    //appactutil.exe -borrow -commServer 27000@localhost -entitlementID ENTL-GEX-test1 -productID GEX-test -expiration 22-Apr-2013
    //<-expiration 22-Apr-2013> user input
    QString message = QString("Are you sure to borrow a license\n"
                              "from Server : %1\n"
                              "For Product ID: %2\n"
                              "and Entitlement ID : %3\n").arg(commServer).arg(productID).arg(entitlementID);

    QDate expirationDate = QDate::currentDate().addDays(30);
    if(!getBorrowExpirationDate(message, expirationDate))
        return;

    QString borrowExpirationDate = dateToString(expirationDate);

    mTerminalOutput->clear();

    QString utilityName = GS_APPACTUTIL_ACTIVATION;
    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument =  QStringList()<< "-borrow"
                                                <<"-commServer"
                                               << commServer
                                               <<"-entitlementID"
                                              << entitlementID
                                              <<"-productID"
                                             <<productID
                                            <<"-expiration"
                                           << borrowExpirationDate;
    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Borrow license", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not Borrow license");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not Borrow license");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();
    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() + result +  QByteArray("\n-------------------------------\n");

    if(result.contains("FulfillmentID"))
    {
        message = QString(result).section("FulfillmentID",1,-1);
        information(QString("Borrow Done with FulfillmentID %2").arg(message));
    }
    else if(result.contains("ERROR:") || result.isEmpty())
    {
        //ERROR: flxActBorrowActivate (%d,%d,%d)\n
        if(result.contains("ERROR: flxActBorrowActivate "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActBorrowActivate ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else
        {
            QString resultString = result;
            QString error = resultString.section("ERROR:",1,-1);
            message = QString(
                        "from Server : %1\n"
                        "For Product ID: %2\n"
                        "and Entitlement ID : %3\n").arg(commServer).arg(productID).arg(entitlementID);

            information(QString("Can not borrow licenses for \n %1 \n with error : %2").arg(message).arg(error));
        }
    }

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);

}


bool LicenseActivation::getBorrowExpirationDate(const QString &message, QDate &expirationDate)
{
    QDialog *expirationDateDialog = new QDialog(this);
    expirationDateDialog->setWindowTitle("Borrow expiration date");

    QGridLayout *gridLayout;
    QLabel *detailsLabel;
    QLabel *expirationDateLabel;
    QDateEdit *expirationDateEdit;
    QDialogButtonBox *userChoice;

    gridLayout = new QGridLayout(expirationDateDialog);
    detailsLabel = new QLabel(expirationDateDialog);
    detailsLabel->setText(message);
    detailsLabel->adjustSize();
    gridLayout->addWidget(detailsLabel, 0, 0, 1, 1);

    expirationDateLabel = new QLabel(expirationDateDialog);
    expirationDateLabel->setText("Enter the expiration date");
    expirationDateLabel->adjustSize();
    gridLayout->addWidget(expirationDateLabel, 1, 0, 1, 1);

    expirationDateEdit = new QDateEdit(expirationDateDialog);
    expirationDateEdit->setDate(expirationDate);
    expirationDateEdit->setCalendarPopup(true);
    expirationDateEdit->setDisplayFormat("dd-MMM-yyyy");

    gridLayout->addWidget(expirationDateEdit, 1, 1, 1, 1);

    userChoice = new QDialogButtonBox(expirationDateDialog);
    userChoice->setOrientation(Qt::Horizontal);
    userChoice->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    gridLayout->addWidget(userChoice, 2, 0, 1, 2);

    QObject::connect(userChoice, SIGNAL(accepted()), expirationDateDialog, SLOT(accept()));
    QObject::connect(userChoice, SIGNAL(rejected()), expirationDateDialog, SLOT(reject()));
    expirationDateDialog->adjustSize();

    int ret = expirationDateDialog->exec();
    expirationDate = expirationDateEdit->date();

    if(ret == QDialog::Rejected)
        return false;
    else
    {
        information(QString("Borrow until %1").arg(dateToString(expirationDate)));
        return true;
    }



}

void LicenseActivation::returnBorrow(const QString &fulfillmentID, const QString &orgCommServer)
{
    QString         lCommServer;
    RBorrow_dialog  lRBorrowDialog(this);

    lRBorrowDialog.setWindowTitle("Return borrowed license");
    lRBorrowDialog.SetServer(orgCommServer);
    lRBorrowDialog.SetPort(27000);

    if (lRBorrowDialog.exec() == QDialog::Rejected)
        return;

    lCommServer = QString::number(lRBorrowDialog.GetPort()) + "@" + lRBorrowDialog.GetServer();

    //appactutil -rborrow FID-1366422136 -commServer 27000@localhost
    int ret = QMessageBox::question(this,"Return borrowed license",
                                    QString("Are you sure to return borrowed license to \n Server :%1 \n for Fulfillment ID %2").arg(lCommServer).arg(fulfillmentID),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

    if(ret == QMessageBox::No)
        return ;
    else
    {
        //                    appactutil -rborrow <fulfillmentID>
        //                                       [-commServer <comm server>]
        //appactutil -rborrow FID-1366422136 -commServer 27000@localhost
        mTerminalOutput->clear();

        QString utilityName = GS_APPACTUTIL_ACTIVATION;
        QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
        QStringList utilityArgument =  QStringList()<< "-rborrow"
                                                    << fulfillmentID
                                                    << "-commServer"
                                                    << lCommServer;

        mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

        mProcessWorker->startPorcess(utility,
                           utilityArgument);
        centralWidget()->setEnabled(false);

        mProcessWorker->start();
        while(mProcessWorker->isRunning())
        {
            updateStatusBarLabel(eBoth, "Return borrow license", 5);
            if (qApp) qApp->processEvents();
        }
        if(!mProcessWorker->getProcessStartedSatus())
        {
            information("Can not return borrow license");
            return ;
        }

        if(!mProcessWorker->getProcessFinishedSatus())
        {
            information("Can not return borrow license");
            return ;
        }

        //QByteArray result = mFNPUtility->readAll();
        QByteArray result = mProcessWorker->getStandardOutput();
        result += mProcessWorker->getErrorOutput();
        QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() + result +  QByteArray("\n-------------------------------\n");

        if(result.contains("Done Borrow Return"))
        {
            QString message = result;
            information(QString("Done Borrow Return : \n%1").arg(message));
        }
        else if(result.contains("ERROR:") || result.isEmpty())
        {
            //ERROR: flxActBorrowReturn (%d,%d,%d)\n
            if(result.contains("ERROR: flxActBorrowReturn "))
            {
                QString errorString = result;
                QString error = errorString.section("ERROR: flxActBorrowReturn ",1,1);
                int errorCode = error.section("(",1,1).section(",",0,0).toInt();
                QString displayedError = errorString;
                if(mErrorCodeMapping.contains(errorCode))
                    displayedError = mErrorCodeMapping[errorCode];
                information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
            }
            else
            {
                QString resultString = result;
                QString error = resultString.section("ERROR:",1,-1);
                QString message = QString(
                            "Server : %1\n"
                            "and Fulfillment ID : %2\n").arg(lCommServer).arg(fulfillmentID);

                information(QString("Can not return borrow licenses for \n %1 \n with error : %2").arg(message).arg(error));
            }
        }

        updateStatusBarLabel(eHide,"",0);
        updateTerminal(rawData);

    }




}


bool LicenseActivation::getOfflineOperation(QString &genFile)
{
    genFile = QString();
    if(mOfflineOperation->isChecked())
    {
        if(mOpFile->text().isEmpty())
        {
            information("Please specify an the output filename for \"offline operation\"");
            mOpFile->setFocus();
            return false;
        }
        genFile = mOpFile->text();
    }

    return true;
}

void LicenseActivation::returnLicenseTo(LicenseActivatedType itemType, const QString &fulfillmentID, const QString &orgCommServer)
{
    QString commServer = orgCommServer;
    if(commServer.isEmpty())
    {
        information("The server name to repair the license found in trust storage is Empty please enter a valid server name");
        bool ok;
        commServer = QInputDialog::getText(this,
                                           "Repair license server",
                                           "Server Name/IP :",
                                           QLineEdit::Normal,
                                           QString(""),
                                           &ok);

        if (!ok || commServer.isEmpty())
            return ;
    }

    QString utilityName;
    utilityName = GS_SERVERACTUTIL_ACTIVATION;
    if(itemType == eLocalActivatedCPU)
        utilityName = GS_APPACTUTIL_ACTIVATION;

    QString proxyDetails = "";
    if(!getProxyDetails(proxyDetails))
        return;

    QString genFile;
    if(!getOfflineOperation(genFile))
        return ;

    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument = QStringList () << "-return" << fulfillmentID
                                                 << "-comm" << "soap"
                                                 << "-commServer" << orgCommServer;

    if(!proxyDetails.isEmpty())
        utilityArgument << "-proxyDetails" << proxyDetails;

    //If needed add an ofline mode.
    if(!genFile.isEmpty())
    {
        utilityArgument.append("-gen");
        utilityArgument.append(genFile);

    }

    mTerminalOutput->clear();
    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Returning license", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not return license");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not return license");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();
    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() +"\n" + result +  QByteArray("\n-------------------------------\n");

    if(result.contains("SUCCESSFULLY SENT RETURN REQUEST"))
    {
        QString message = result;
        information(QString("Return Succeed : \n%1").arg(message));

    }
    else if(result.contains("ERROR:") || result.isEmpty())
    {
        if(result.contains("ERROR: flxActAppActivationReasonSet "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActAppActivationReasonSet ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActAppReturnReqSet - "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActAppReturnReqSet - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActAppActivationSend - "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActAppActivationSend - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: Could not determine 3-Server status "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: Could not determine 3-Server status ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActAppActivationReasonSet "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActAppActivationReasonSet ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActSvrReturnReqSet - "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActSvrReturnReqSet - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActSvrReturnSend - "))
        {
            QString errorString = result;
            QString error = errorString.section("ERROR: flxActSvrReturnSend - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Returnning license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else
        {
            QString resultString = result;
            QString error = resultString.section("ERROR:",1,-1);
            QString message = QString(
                        "Server : %1\n"
                        "and Fulfillment ID : %2\n").arg(commServer).arg(fulfillmentID);

            information(QString("Returnning license Fails for \n %1 \n with error : %2").arg(message).arg(error));
        }
    }
    else if(result.contains("WARNING:") || result.isEmpty())
    {
        QString resultString = result;
        QString error = resultString.section("WARNING:",1,-1);
        QString message = QString(
                    "Server : %1\n"
                    "and Fulfillment ID : %2\n").arg(commServer).arg(fulfillmentID);

        information(QString("Can not return license for \n %1 \n with warning : %2").arg(message).arg(error));
    }else if(result.contains("Operations error:") || result.isEmpty())
    {
        QString resultString = result;
        QString error = resultString.section("Operations error: ",1,1);
        information(QString("Return Fails and return \nOperations error: %1").arg(error));

    }

    if(!genFile.isEmpty())
    {
        if(result.contains("Writing signed return request to"))
        {
            QString message = result;
            information(QString("Writing signed return request to: %1").arg(message.section("Writing signed return request to",1)));
        }
    }
    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);



}

void LicenseActivation::deleteFulfillment(LicenseActivatedType itemType, const QString &fulfillmentID)
{
    int ret = QMessageBox::critical(this,"Deleting Fulfillment" ,
                                    QString("Are you sure to delete the Fulfillment with ID : <%1> ? This operation can not be repaired").arg(fulfillmentID),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(ret == QMessageBox::No)
        return ;


    QString utilityName;
    utilityName = GS_SERVERACTUTIL_ACTIVATION;
    if(itemType == eLocalActivatedCPU)
        utilityName = GS_APPACTUTIL_ACTIVATION;

    QString proxyDetails = "";
    if(!getProxyDetails(proxyDetails))
        return;

    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument = QStringList () << "-delete" << fulfillmentID;

    mTerminalOutput->clear();
    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Deleting FID", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not delete specified FID");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not delete specified FID");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();
    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() +"\n" + result +  QByteArray("\n-------------------------------\n");

    if(result.contains("Successfully deleted fulfillment"))
    {
        QString message = result;
        information(QString("Delete Succeed : \n%1").arg(message));

    }
    else if(result.contains("ERROR:") || result.isEmpty())
    {
        QString resultString = result;
        QString error = resultString.section("ERROR:",1,-1);
        QString message = QString("Delete fails with error : %1").arg(error);
        information(message);
    }
    else if(result.contains("No fulfillment records in trusted storage"))
    {
        QString message = QString("No fulfillment records in trusted storage");
        information(message);
    }

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);

}

void LicenseActivation::deleteProduct(LicenseActivatedType itemType, const QString &product, const QString &fulfillment)
{
    int ret = QMessageBox::critical(this,"Deleting Product" ,
                                    QString("Are you sure to delete the Product <%1> with Fulfillment ID : <%2> ? This operation can not be repaired").arg(product).arg(fulfillment),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(ret == QMessageBox::No)
        return ;

    QString utilityName;
    utilityName = GS_SERVERACTUTIL_ACTIVATION;
    if(itemType == eLocalActivatedCPU)
        utilityName = GS_APPACTUTIL_ACTIVATION;

    QString proxyDetails = "";
    if(!getProxyDetails(proxyDetails))
        return;

    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument = QStringList () << "-delproduct " << product;

    mTerminalOutput->clear();
    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Deleting PRODUCT", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not delete product");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not delete product");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();

    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() +"\n" + result +  QByteArray("\n-------------------------------\n");

    if(result.contains("ERROR:") || result.isEmpty())
    {
        QString resultString = result;
        QString error = resultString.section("ERROR:",1,-1);
        QString message = QString("Delete fails with error : %1").arg(error);
        information(message);
    }
    else if(result.contains("No fulfillment records in trusted storage"))
    {
        QString message = QString("No fulfillment records in trusted storage");
        information(message);
    }
    else
    {
        QString message = result;
        information(QString("delete done : \n%1").arg(message));
    }

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);

}

void LicenseActivation::repairLicense (LicenseActivatedType itemType, const QString &fulfillmentID, const QString &orgCommServer)
{
    //    appactutil -repair <fulfillmentID>
    //                       [-comm <flex|soap>]
    //                       [-commServer <comm server>]
    //                       [-proxyDetails "<host> <port> [<user id>] [<password>]"]
    //                       [-gen [<output_filename>]]
    //                       [-vendordata <key> <value>]

    //   serveractutil -repair <fulfillmentID>
    //                  [-comm <flex|soap>]
    //                  [-commServer <comm server>]
    //                  [-proxyDetails "<host> <port> [<user id>] [<password>]"]
    //                  [-gen [<output_filename>]]
    //                  [-vendordata <key> <value>]

    QString commServer = orgCommServer;
    if(commServer.isEmpty())
    {
        information("The server name to repair the license found in trust storage is Empty please enter a valid server name");
        bool ok;
        commServer = QInputDialog::getText(this,
                                           "Repair license server",
                                           "Server Name/IP :",
                                           QLineEdit::Normal,
                                           QString(""),
                                           &ok);

        if (!ok || commServer.isEmpty())
            return ;
    }

    QString utilityName;
    utilityName = GS_SERVERACTUTIL_ACTIVATION;
    if(itemType == eLocalActivatedCPU)
        utilityName = GS_APPACTUTIL_ACTIVATION;

    QString proxyDetails = "";
    if(!getProxyDetails(proxyDetails))
        return;

    QString genFile;
    if(!getOfflineOperation(genFile))
        return ;

    QStringList proxyDetailsList;
    if(!proxyDetails.isEmpty())
        proxyDetailsList = QStringList() << "-proxyDetails" << proxyDetails;
    QString utility = mFNPUtilsDir + QDir::separator() + utilityName + GS_ACTIVIATION_EXT;
    QStringList utilityArgument = QStringList () << "-repair" << fulfillmentID
                                                 << "-comm" << "soap"
                                                 << "-commServer" << orgCommServer
                                                 << proxyDetailsList ;//If needed add an ofline mode.

    if(!genFile.isEmpty())
    {
        utilityArgument.append("-gen");
        utilityArgument.append(genFile);
    }

    mTerminalOutput->clear();
    mTerminalOutput->append(utility + "   " + utilityArgument.join(" "));

    mProcessWorker->startPorcess(utility,
                       utilityArgument);
    centralWidget()->setEnabled(false);

    mProcessWorker->start();
    while(mProcessWorker->isRunning())
    {
        updateStatusBarLabel(eBoth, "Return borrow license", 5);
        if (qApp) qApp->processEvents();
    }
    if(!mProcessWorker->getProcessStartedSatus())
    {
        information("Can not return borrow license");
        return ;
    }

    if(!mProcessWorker->getProcessFinishedSatus())
    {
        information("Can not return borrow license");
        return ;
    }

    //QByteArray result = mFNPUtility->readAll();
    QByteArray result = mProcessWorker->getStandardOutput();
    result += mProcessWorker->getErrorOutput();

    QByteArray rawData = mTerminalOutput->toPlainText().toLatin1() +"\n" + result +  QByteArray("\n-------------------------------\n");


    if(result.contains("SUCCESSFULLY SENT REPAIR REQUEST"))
    {
        QString message = result;
        information(QString("Repair Succeed : \n%1").arg(message));

    }
    else if(result.contains("ERROR:") || result.isEmpty())
    {
        //Server
        QString errorString = result;
        if(result.contains("ERROR: Could not determine 3-Server status "))
        {
            QString error = errorString.section("ERROR: Could not determine 3-Server status ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActSvrRepairReqSet - "))
        {
            QString error = errorString.section("ERROR: flxActSvrRepairReqSet - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActSvrRepairSend - "))
        {
            QString error = errorString.section("ERROR: flxActSvrRepairSend - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActAppActivationSend - "))
        {
            QString error = errorString.section("ERROR: flxActAppActivationSend - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else if(result.contains("ERROR: flxActAppRepairReqSet - "))
        {
            QString error = errorString.section("ERROR: flxActAppRepairReqSet - ",1,1);
            int errorCode = error.section("(",1,1).section(",",0,0).toInt();
            QString displayedError = errorString;
            if(mErrorCodeMapping.contains(errorCode))
                displayedError = mErrorCodeMapping[errorCode];
            information(QString("Repair license Fails. Error %1: %2").arg(errorCode).arg(displayedError));
        }
        else
        {
            QString resultString = result;
            QString error = resultString.section("ERROR:",1,-1);
            QString message = QString(
                        "Server : %1\n"
                        "and Fulfillment ID : %2\n").arg(commServer).arg(fulfillmentID);

            information(QString("Repair license Fails for \n %1 \n with error : %2").arg(message).arg(error));
        }
    }
    else if(result.contains("WARNING:") || result.isEmpty())
    {
        QString resultString = result;
        QString error = resultString.section("WARNING:",1,-1);
        QString message = QString(
                    "Server : %1\n"
                    "and Fulfillment ID : %2\n").arg(commServer).arg(fulfillmentID);

        information(QString("Can not repair license for \n %1 \n with warning : %2").arg(message).arg(error));
    }else if(result.contains("Operations error:") || result.isEmpty())
    {
        QString resultString = result;
        QString error = resultString.section("Operations error: ",1,1);
        information(QString("Repair Fails and return Operations error: %1").arg(error));

    }
    if(!genFile.isEmpty())
    {
        if(result.contains("Writing signed return request to"))
        {
            QString message = result;
            information(QString("Writing signed return request to: %1").arg(message.section("Writing signed return request to",1)));
        }
    }

    updateStatusBarLabel(eHide,"",0);
    updateTerminal(rawData);




}

void LicenseActivation::requestEvaluation()
{
    if(mFirstName->text().isEmpty())
    {
        information("Please enter your first name");
        mFirstName->setFocus();
        return ;
    }
    if(mLastName->text().isEmpty())
    {
        information("Please enter your last name");
        mLastName->setFocus();
        return ;
    }
    if(mCompany->text().isEmpty())
    {
        information("Please enter your company name");
        mCompany->setFocus();
        return ;
    }
    if(mEmail->text().isEmpty())
    {
        information("Please enter you email");
        mEmail->setFocus();
        return ;
    }

    information("Evaluation Mode: not yet supported");


}

void LicenseActivation::information(const QString &info)
{
    if (qApp)
        QMessageBox::information(this, qApp->applicationName() + " - " + qApp->applicationVersion(), info);
    centralWidget()->setEnabled(true);

}

QDate LicenseActivation::converToDate(const QString &dateString)
{
    //"dd-MMM-yyyy "
    if(dateString.isEmpty())
        return QDate();
    if(dateString.toUpper() == "PERMANENT")
        return QDate(2050,1,1);

    int day = dateString.section("-",0,0).toInt();
    int year = dateString.section("-",2,2).toInt();
    QString month = dateString.section("-",1,1);
    int monthVal = 1;
    if(month.toLower() == "jan") monthVal = 1;
    if(month.toLower() == "feb") monthVal = 2;
    if(month.toLower() == "mar") monthVal = 3;
    if(month.toLower() == "apr") monthVal = 4;
    if(month.toLower() == "may") monthVal = 5;
    if(month.toLower() == "jun") monthVal = 6;
    if(month.toLower() == "jul") monthVal = 7;
    if(month.toLower() == "aug") monthVal = 8;
    if(month.toLower() == "sep") monthVal = 9;
    if(month.toLower() == "oct") monthVal = 10;
    if(month.toLower() == "nov") monthVal = 11;
    if(month.toLower() == "dec") monthVal = 12;
    return QDate(year,monthVal,day);
}

QString LicenseActivation::dateToString(const QDate &date)
{
    //"dd-MMM-yyyy "
    QString qtDate = date.toString("dd-MMM-yyyy");

    QString dateString = qtDate.section("-",0,0);//day

    int monthVal = date.month();
    QString month;
    if(monthVal == 1  ) month =  "jan";
    if(monthVal == 2  ) month =  "feb";
    if(monthVal == 3  ) month =  "mar";
    if(monthVal == 4  ) month =  "apr";
    if(monthVal == 5  ) month =  "may";
    if(monthVal == 6  ) month =  "jun";
    if(monthVal == 7  ) month =  "jul";
    if(monthVal == 8  ) month =  "aug";
    if(monthVal == 9  ) month =  "sep";
    if(monthVal == 10 ) month =  "oct";
    if(monthVal == 11 ) month =  "nov";
    if(monthVal == 12 ) month =  "dec";

    dateString += "-" + month;
    dateString += "-" + qtDate.section("-",2,2);//year

    return dateString;
}

void LicenseActivation::updateStatusBarLabel(eStatusBarUpdate type, const QString &label, int val, int max, int min)
{
    if(type == eLabel || type == eBoth)
    {
        mStatusLabel->setText(label);
        mStatusLabel->show();
    }

    if(type == eProgress || type == eBoth)
    {
        mProgressBar->setRange(min, max);
        mProgressBar->setTextVisible(true);
        mProgressBar->setValue(val);
        mProgressBar->show();
    }
    if(type == eHide)
    {
        mStatusLabel->hide();
        mProgressBar->hide();
    }

}

bool LicenseActivation::getProxyDetails(QString &proxyDetails)
{
    if(!mProxyDetails->isChecked())
    {
        proxyDetails =  QString();
    }
    else
    {
        if(mProxyName->text().isEmpty())
        {
            information("Please Enter The Proxy Name/IP");
            mProxyName->setFocus();
            return false;
        }
        if(mProxyPort->text().isEmpty())
        {
            information("Please Enter the Proxy port");
            mProxyPort->setFocus();
            return false;
        }

        if(!(mProxyUserName->text().isEmpty()) && mProxyUserPassword->text().isEmpty())
        {
            information("Please enter the password for the username specified");
            mProxyUserPassword->setFocus();
            return false;
        }

        QString lProxyProperty = QString("%1 %2").arg(mProxyName->text()).arg(mProxyPort->text());
        if(!mProxyUserName->text().isEmpty())
        {
            lProxyProperty += QString(" %1 %2").arg(mProxyUserName->text()).arg(mProxyUserPassword->text());
        }

        proxyDetails = QString("%1").arg(lProxyProperty);
    }

    return true;
}

void LicenseActivation::additionalKey()
{
    QWidget *suppKey = new QWidget(mActKeyWidget);
    suppKey->setObjectName("AdditionalKey");
    QHBoxLayout *keyHBox = new QHBoxLayout(suppKey);
    keyHBox->setSpacing(0);
    keyHBox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hboxContent = new QHBoxLayout();

    QLineEdit *actKey = new QLineEdit(suppKey);

    hboxContent->addWidget(actKey);

    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

    hboxContent->addItem(spacer);


    QPushButton *remove = new QPushButton(suppKey);
    QIcon icon7;
    icon7.addFile(QString::fromUtf8(":/gsacutil/icons/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
    remove->setIcon(icon7);
    remove->setFlat(true);
    remove->setMaximumSize(QSize(18, 18));
    hboxContent->addWidget(remove);




    keyHBox->addLayout(hboxContent);

    int lineCount = mActKeyWidgetGridLayout->rowCount();
    mActKeyWidgetGridLayout->addWidget(suppKey, lineCount + 1, 0, 1, 1);

    connect(remove, SIGNAL(clicked()), this, SLOT(removeAdditionalKey()));
}

void LicenseActivation::removeAdditionalKey()
{

    QPushButton *remove = qobject_cast<QPushButton *>(sender());
    if(remove && remove->parent())
    {
        QWidget *suppKey = qobject_cast<QWidget *>(remove->parent());
        if(suppKey)
        {
            if(suppKey->parentWidget()){
                if(suppKey->parentWidget()->layout())
                    suppKey->parentWidget()->layout()->removeWidget(suppKey);
                suppKey->deleteLater();
                suppKey->setParent(0);
            }

        }
    }

}

void LicenseActivation::intErrorCodeMapping()
{
    mErrorCodeMapping.insert(LM_TS_BASE_ERROR											,"");
    mErrorCodeMapping.insert(LM_TS_BADPARAM												,"Error caused by an invalid parameter in the function. \nRecovery: verify parameters	");
    mErrorCodeMapping.insert(LM_TS_CANTMALLOC											,"Insufficient memory. \nRecovery: none	");
    mErrorCodeMapping.insert(LM_TS_OPEN_ERROR											,"This is an internal error. \nRecovery: none	");
    mErrorCodeMapping.insert(LM_TS_CLOSE_ERROR											,"This is an internal error. \nRecovery: none	");
    mErrorCodeMapping.insert(LM_TS_INIT													,"Initialization of API failed. \nRecovery: use flxActCommonLibraryInit. Note that this not set by flxActCommonLibraryInit.	");
    mErrorCodeMapping.insert(LM_TS_ACT_GEN_REQ											,"Generate activation request failed. \nRecovery: check parameters used for activation request.	");
    mErrorCodeMapping.insert(LM_TS_RETURN_GEN_REQ										,"Generate return request failed. \nRecovery: check parameters used for return request.	");
    mErrorCodeMapping.insert(LM_TS_REPAIR_GEN_REQ										,"Generate repair request failed. \nRecovery: check parameters used for repair request.	");
    mErrorCodeMapping.insert(LM_TS_RESERVED_9											,"");
    mErrorCodeMapping.insert(LM_TS_DELETE												,"Error when deleting a fulfillment record. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_CANT_FIND											,"No matching fulfillment record. \nRecovery: check fulfillment ID of fulfillment to be returned repaired.	");
    mErrorCodeMapping.insert(LM_TS_INTERNAL_ERROR										,"Corrupt or incomplete trusted storage or ASR. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_RESERVED_13											,"");
    mErrorCodeMapping.insert(LM_TS_INVALID_INDEX										,"Invalid index parameter in API function that requires an index value. \nRecovery: Ensure index value is within the range returned by the relevant API function.	");
    mErrorCodeMapping.insert(LM_TS_RESERVED_15											,"");
    mErrorCodeMapping.insert(LM_TS_NO_ASR_FOUND											,"Specified ASR not found.Recovery: ensure specified ASR and or path exist.	");
    mErrorCodeMapping.insert(LM_TS_UPDATING_TS											,"Failed to add ASR to trusted storage. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_SEND_RECEIVE											,"Failure to send request or receive a response for an unspecified reason. \nRecovery: check parameters used for the request.	");
    mErrorCodeMapping.insert(LM_TS_PROCESS_RESP											,"Failure to process response for an unspecified reason. This error may be returned when a valid failure response is received, for example when the activation server denies a request. \nRecovery: check parameters used for the request.	");
    mErrorCodeMapping.insert(LM_TS_UNKNOWN_RESP											,"Response corrupted. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_ASR_LOAD_ONCE										,"This ASR has been activated. It can only be activated once so this request has been refused. \nRecovery: none as this is the configured behaviour.	");
    mErrorCodeMapping.insert(LM_TS_RESERVED_22											,"");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_PENDING									,"Cannot perform requested action because there is an outstanding short code transaction. Only one short code transaction for a given alias can be in progress at any one time. \nRecovery: either wait for transaction to complete or use	");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_ACT_CREATE								,"flxActApp*ShortCodeCancel to cancel existing transaction. Create a short code activation failure. \nRecovery: check parameters used for short code activation request.	");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_REPAIR_CREATE								,"Create a short code repair failure. \nRecovery: check parameters used for short code repair request.	");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_RETURN_CREATE								,"Create a short code return failure. \nRecovery: check parameters used for short code return request.	");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_CANCEL									,"Cancel a short code transaction failure. \nRecovery: check parameters used for short code transaction cancel request.	");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_PROCESS									,"Error while processing a short code response. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_SHORT_CODE_UNSUPPORTED								,"Either: ASR specified in short code function does not support short codes. or Short code ASR specified for use in local activation. \nRecovery: specify correct ASR.	");
    mErrorCodeMapping.insert(LM_TS_LOAD													,"Failed to load trusted storage or specified ASR. \nRecovery: none	");
    mErrorCodeMapping.insert(LM_TS_FR_DISABLE											,"Failed to disable fulfillment record in trusted storage. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_TIMEDOUT												,"The connection to the license server or Operations server timed out. \nRecovery: check connection.	");
    mErrorCodeMapping.insert(LM_TS_INSUFFICIENT_RESOURCE								,"The license server or Operations server did not have enough licenses to satisfy the activation request. Note that the license server will only provide licenses that can be served from a single fulfillment record. \nRecovery: submit request with a smaller number of licenses	");
    mErrorCodeMapping.insert(LM_TS_INVALID_REQUEST_TYPE									,"Server does not support this request type. Normally this occurs when the request is corrupted. \nRecovery: resubmit request	");
    mErrorCodeMapping.insert(LM_TS_NO_MATCHING_FULFILLMENT								,"The license server has no fulfillment records that match this request. \nRecovery: submit request for a different entitlement.	");
    mErrorCodeMapping.insert(LM_TS_INVALID_REQUEST										,"The request is invalid. \nRecovery: check parameters in request.	");
    mErrorCodeMapping.insert(LM_TS_RETURN_OUT_OF_CHAIN									,"A return request has been sent to a license server that is not the one that served this fulfillment record. \nRecovery: examine fulfillment record to determine the correct license server and resubmit.	");
    mErrorCodeMapping.insert(LM_TS_MAX_COUNT_EXCEEDED									,"Return request rejected because maximum count will be exceeded. The maximum count records the original number of licenses in a fulfillment record on a license server. \nRecovery: none.	");
    mErrorCodeMapping.insert(LM_TS_INSUFFICIENT_REPAIR_COUNT							,"Repair request rejected because the specified number of repairs have already been completed for this fulfillment record. \nRecovery: none	");
    mErrorCodeMapping.insert(LM_TS_OPERATIONS											,"Error returned by the Operations server. See flxActCommonHandleGetLastOpsError and flxActCommonHandleGetLastOpsErrorString for details of how to retrieve details of the error. \nRecovery: see FlexNet Operations server documentation.	");
    mErrorCodeMapping.insert(LM_TS_CONNECTION_FAILED									,"Failed to connect to the license server or Operations server. \nRecovery: check connection and that server is operational.	");
    mErrorCodeMapping.insert(LM_TS_SSL_ERROR											,"Error with SSL certificate provided. See flxActCommonHandleSetSSLDetails for more details. \nRecovery: verify SSL settings.	");
    mErrorCodeMapping.insert(LM_TS_RETURN_INCOMPLETE									,"Return not allowed because it would orphan fulfillment records created from the fulfillment to be returned. See flxActSvrReturnSend. \nRecovery: either ensure all fulfillment records created from this fulfillment are returned or use flxActSvrReturnForceIncompleteSet.	");
    mErrorCodeMapping.insert(LM_TS_LOCAL_REPAIR											,"Error occurred during call to flxActCommonRepairLocalTrustedStorage. \nRecovery: none	");
    mErrorCodeMapping.insert(LM_TS_UNSUPPORTED_REQUEST_VERSION							,"A FlexEnabled application has requested activation from a license server that uses an earlier version of fulfillment records. See information about flexFulfillmentVersion in the Programming Reference for Trusted Storage-Based Licensing for more details. To recover, connect to a license server that is using the correct version of fulfillment records.	");
    mErrorCodeMapping.insert(LM_TS_UNSUPPORTED											,"A FlexEnabled application has received a response from a license server that contains a fulfillment record that it cannot process. See information about flexFulfillmentVersion in the Programming Reference for Trusted Storage-Based Licensing for more details. \nRecovery: connect to a license server that is using the correct version of fulfillment records.	");
    mErrorCodeMapping.insert(LM_TS_CONFIGURATION										,"The requested operation cannot be completed because of an invalid configuration. For example, a valid anchor is required in an ASR that specifies the use of a trial anchor. \nRecovery: ensure that a valid configuration is specified.	");
    mErrorCodeMapping.insert(LM_TS_NO_PRODUCT_SET										,"The requested operation cannot be completed because the fulfillment record has not been specified. \nRecovery: ensure that the fulfillment record is specified using the appropriate function call before calling the function that resulted in this error.	");
    mErrorCodeMapping.insert(LM_TS_TRIALPACKS_ASR_ENTRY									,"An error occurred while attempting to load or validate the TRIAL_PACK_IDS entry in an ASR. \nRecovery: ensure that you are using the XML snippet supplied by Flexera Software and that it has not been modified.	");
    mErrorCodeMapping.insert(LM_TS_VIRTUALIZATION_POLICY_MISMATCH						,"");
    mErrorCodeMapping.insert(LM_TS_VIRTUAL_INTERFACE									,"");
    mErrorCodeMapping.insert(LM_TS_SERVERQUERY_GEN_REQ									,"The request to query remote server-side trusted storage could not be generated.	");
    mErrorCodeMapping.insert(LM_TS_SERVERQUERY_RESPONSE_FAILED							,"The response to a remote server-side trusted-storage query could not be processed.	");
    mErrorCodeMapping.insert(LM_TS_SERVERQUERY_FR_NOT_FOUND								,"The fulfillment record intended to be queried in remote server-side trusted storage does not exist.	");

    mErrorCodeMapping.insert(LM_TS_CT_BASE												,"");
    mErrorCodeMapping.insert(LM_TS_CT_LAST												,"");

//    mErrorCodeMapping.insert(LM_TS_VIF_SESSION_HANDLE_ERROR								,"");
//    mErrorCodeMapping.insert(LM_TS_VIF_NOT_VIRTUAL_ERROR								,"");
//    mErrorCodeMapping.insert(LM_TS_VIF_INTERNAL_ERROR									,"");

    mErrorCodeMapping.insert(LM_TS_3SERVER_NOT_PERMITTED								,"");
    mErrorCodeMapping.insert(LM_TS_3SERVER_CONFIGURED									,"");
    mErrorCodeMapping.insert(LM_TS_3SERVER_NOT_CONFIGURED								,"");
    mErrorCodeMapping.insert(LM_TS_3SERVER_VALIDATION									,"");
    mErrorCodeMapping.insert(LM_TS_3SERVER_TWO_NEW_NODES_NOT_ALLOWED					,"");
    mErrorCodeMapping.insert(LM_TS_3SERVER_BUFFER_SIZE									,"");
}

void LicenseActivation::viewProductProperty()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    QString		strMessage;

    strMessage ="Information:\n";
    strMessage += QString("Build number: %1\n").arg(GEX_APP_VERSION_BUILD);
#ifdef GEX_APP_REVISION
    strMessage += QString("From revision: %1\n").arg(GEX_APP_REVISION);
#endif
    strMessage += "Based on Qt: "; strMessage += QT_VERSION_STR; strMessage += "\n";


#ifdef __GNUC__
    strMessage += QString("Built on %1 at %2\n").arg(__DATE__).arg(__TIME__);
    strMessage += QString("with GCC: %1.%2.%3\n\n").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#endif
    strMessage += QString("Word size=%1b sizeof(int)=%2o sizeof(void*)=%3o\n")
            .arg(QSysInfo::WordSize).arg(sizeof(int)).arg(sizeof(void*));
    strMessage += QString("Ideal thread count: %1 ").arg(QThread::idealThreadCount());
    strMessage += QString("Process ID: %1\n\n").arg(QCoreApplication::applicationPid());

#ifdef Q_OS_WIN
    strMessage += "Windows version : ";
    QString osname;
    switch (QSysInfo::windowsVersion())
    {
    case QSysInfo::WV_NT : osname="WV_NT"; break;
    case QSysInfo::WV_2000 : osname="WV_2000"; break;
    case QSysInfo::WV_XP : osname="WV_XP"; break;
    case QSysInfo::WV_2003 : osname="WV_2003"; break;
    case QSysInfo::WV_VISTA : osname="WV_VISTA"; break;
    case QSysInfo::WV_WINDOWS7 : osname="WV_WINDOWS7"; break;
    case QSysInfo::WV_WINDOWS8 : osname="WV_WINDOWS8"; break;
    default: osname="Not NT based windows (Win98, ME,...) !"; break;
    }
    strMessage += osname;
    strMessage += "\n";
    strMessage += "USERDOMAIN : "+QString(getenv("USERDOMAIN"))+" ";
    strMessage += "PROCESSOR_ARCHITECTURE : "+QString(getenv("PROCESSOR_ARCHITECTURE"))+" ";
    strMessage += "PROCESSOR_IDENTIFIER : "+QString(getenv("PROCESSOR_IDENTIFIER"))+" ";
    strMessage += "NUMBER_OF_PROCESSORS : "+QString(getenv("NUMBER_OF_PROCESSORS"))+"\n";
#endif

    strMessage+="Locale: "; strMessage+=QLocale::system().name(); strMessage += " ";
    strMessage+="Country: "; strMessage+=QLocale::countryToString(QLocale::system().country()); strMessage += " ";
    strMessage+="Language: "; strMessage+=QLocale::languageToString(QLocale::system().language()); strMessage+=" ";
    strMessage+="Decimal Point: ";	strMessage+=QLocale::system().decimalPoint(); strMessage+="\n";

    strMessage+=QString("\nHome dir : %1\n").arg(QDir::homePath());


#ifdef QT_DEBUG
    QPainter pa;
    // "X11","Windows", "QuickDraw", "CoreGraphics", "MacPrinter", "QWindowSystem", "PostScript", "OpenGL",
    // "Picture", "SVG", "Raster", "Direct3D", "Pdf", "OpenVG", "OpenGL2", "PaintBuffer"
    strMessage+=QString("\nPaint engine type : %1").arg(pa.paintEngine()?pa.paintEngine()->type():-1);

    //    QStringList atl=QStringList()<<"Application::Tty"<<"QApplication::GuiClient"<<"QApplication::GuiServer";
    //    strMessage+=QString("\nQApplication::type : %1").arg( atl.at( QApplication::type() ) );

    QDesktopWidget* dw=QApplication::desktop();
    if (dw)
    {
        strMessage += QString("\nDesktop screen count:%1").arg(dw->screenCount());
        strMessage += QString(" Virtual desktop:%1").arg(dw->isVirtualDesktop()?"true":"false");
        strMessage += QString(" Desktop geom:%1x%2").arg(dw->availableGeometry().width()).arg(dw->availableGeometry().height() );
        strMessage += QString(" Screen geom:%1x%2\n").arg(dw->screen()->width()).arg(dw->screen()->height());
    }
#endif

    // Do not perform the raw mem test in release
#ifdef QT_DEBUG
    QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(true, true);
#else
    QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, false);
#endif

    strMessage+=QString("Memory info : ");
    foreach(const QString &k, lMemInfo.keys())
        strMessage+=QString("%1:%2, ")
                .arg(k).arg(lMemInfo.value(k).toString());
    strMessage+="\n\n";

    QMessageBox messageBox(this);
    messageBox.setText(strMessage);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setWindowTitle(this->windowTitle());
    messageBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));

    QApplication::restoreOverrideCursor();

    messageBox.exec();
}

}
}
