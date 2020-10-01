#include <QValidator>
#include <QMessageBox>

#include <time.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect signal/slots
    connect(ui->mCheckOUT, SIGNAL(clicked()), this, SLOT(onCheckOUT()));
    connect(ui->mCheckIN, SIGNAL(clicked()), this, SLOT(onCheckIN()));

#ifdef WIN32
    // Init COM objects
    //mTAGLMProxy.setControl("TAGLMProxyLib.TAGLMProxy");
    mTAGLMProxy.setControl("{6CF443EB-0BA0-43EA-AC69-16650FF833D3}");

    if(mTAGLMProxy.isNull())
        ui->mStatus->setText("FAILED INITIALIZING COM OBJECT \"TAGLMProxyLib.TAGLMProxy\"");
    else
    {
        ui->mTAGLMProxyDoc->setText(mTAGLMProxy.generateDocumentation());
        QObject::connect(&mTAGLMProxy, SIGNAL(exception(int, const QString&, const QString&, const QString&)),
                         this, SLOT(onException(int, const QString&, const QString&, const QString&)));
    }
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::getInputs(QString & inc)
{
    // Make sure we have required inputs
    inc = ui->mIncrement->text();
    if(inc.isEmpty())
    {
        ui->mStatus->setText(QStringLiteral("Empty incremet, please enter an increment."));
        ui->mIncrement->setFocus();
        return false;
    }
    return true;
}

void MainWindow::onCheckOUT()
{
    ui->mException->setText("");
    ui->mStatus->setText("");

    // Make sure we have required inputs
    QString lInc;
    if(!getInputs(lInc))
        return;

#ifdef WIN32
    // Try to checkout specified increment
    QString lMessage;
    QAxObject* lLicInfo = checkOUT(lInc, lMessage);
    ui->mStatus->setText(lMessage);
    if(lLicInfo)
    {
        // Update map
        mIncrements.insertMulti(lInc, lLicInfo);
        // Refresh summary table
        refreshSummary();
        // Set doc and connect exception handler
        ui->mLicInfoDoc->setText(lLicInfo->generateDocumentation());
        QObject::connect(lLicInfo, SIGNAL(exception(int, const QString&, const QString&, const QString&)),
                         this, SLOT(onException(int, const QString&, const QString&, const QString&)));
    }
#else
    ui->mStatus->setText(QStringLiteral("Only supported on Windows."));
#endif
    
    return;
}

void MainWindow::onCheckIN()
{
    ui->mException->setText("");
    ui->mStatus->setText("");

    // Make sure we have required inputs
    QString lInc;
    if(!getInputs(lInc))
        return;

#ifdef WIN32
    // Make sure we have a license
    QMap<QString, QAxObject*>::iterator i = mIncrements.find(lInc);
    if((i == mIncrements.end()) || (i.value() == NULL))
    {
        ui->mStatus->setText("FAIL!\n\nError = Increment not checked in.");
        return;
    }

    // Try to checkin specified increment
    QString lMessage;
    QAxObject* lLicInfo = i.value();
    bool lStatus=checkIN(lLicInfo, lMessage);
    ui->mStatus->setText(lMessage);
    if(lStatus)
    {
        // Update map
        mIncrements.erase(i);
        // Refresh summary table
        refreshSummary();
        // Disconnect exception handler
        QObject::disconnect(lLicInfo, SIGNAL(exception(int, const QString&, const QString&, const QString&)),
                         this, SLOT(onException(int, const QString&, const QString&, const QString&)));
    }
#else
    ui->mStatus->setText(QStringLiteral("Only supported on Windows."));
#endif

    return;
}

#ifdef WIN32
void MainWindow::refreshSummary()
{
    // Reset table
    ui->mIncrementsSummary->clearContents();

    // Add elements from map
    QTableWidgetItem *lItem=NULL;
    int lRow=0;
    QList<QString> lKeys = mIncrements.uniqueKeys();
    ui->mIncrementsSummary->setRowCount(lKeys.size());
    for(lRow=0; lRow<lKeys.size(); ++lRow)
    {
        lItem = new QTableWidgetItem(lKeys.at(lRow));
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->mIncrementsSummary->setItem(lRow, 0, lItem);
        lItem = new QTableWidgetItem(QString::number(mIncrements.count(lKeys.at(lRow))));
        lItem->setTextAlignment(Qt::AlignCenter);
        ui->mIncrementsSummary->setItem(lRow, 1, lItem);
    }
}

//#############################################################################################

QAxObject* MainWindow::checkOUT(const QString & inc, QString & msg)
{
    // Make sure COM wrapper is not NULL
    if(mTAGLMProxy.isNull())
    {
        msg = QString("FAIL!\n\nError = Increment already checked out.");
        return NULL;
    }

    // Call COM checkout
    QString lErr;
    QAxObject* lLicInfo = mTAGLMProxy.querySubObject("CheckOut(QString, QString&, QString, QString, bool)", inc, lErr, QString("0.0"), QString(""), QString("true"));
    if(!lLicInfo)
    {
        msg = QString("FAIL!\n\nError = %1").arg(lErr);
        return NULL;
    }

    msg = QString("SUCCESS!");
    return lLicInfo;
}

bool MainWindow::checkIN(QAxObject* licInfo, QString & msg)
{
    // Check if we have a license
    if(!licInfo)
    {
        msg = QString("FAIL (no license to checkin)!");
        return false;
    }

    QVariant lParam = licInfo->asVariant();

    //QVariant lParam = mLicInfo->asVariant();
    mTAGLMProxy.dynamicCall("CheckIn(IDispatch*", lParam);
//    if(lReturn.isNull() || !lReturn.isValid() || !lReturn.toBool())
//    {
//        msg = QString("FAIL!");
//        return false;
//    }

    msg = QString("SUCCESS!");
    return true;
}
#endif

void MainWindow::onException(int code, const QString& source, const QString& desc, const QString& help)
{
    QMessageBox::information(this, "TerPOC", "Exception");
    ui->mException->setText(QString("EXCEPTION:\ncode = %1\nsource = %2\ndesc = %3\nhelp = %4").arg(code).arg(source).arg(desc).arg(help));
}
