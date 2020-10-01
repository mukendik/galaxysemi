#include "ui_license_activiation.h"
#include <QProcess>
#include <QDate>

#ifndef LICENSE_ACTIVATION_DIALOG_H
#define LICENSE_ACTIVATION_DIALOG_H

class QProgressBar;
class QLabel;

namespace GS
{
namespace ActivationGui
{

class ProcessWorker;

class LicenseActivation : public QMainWindow, public Ui::ActivationWindow
{
    Q_OBJECT
public:
    LicenseActivation();
    virtual ~LicenseActivation();
    int getError(QString &error);
public:
    enum LicenseActivationError{
        eNoError,
        eMissingUtilsDir,
        eMissingUtils
    };

    enum LicenseActivatedType{
        eLocalActivated = 0,
        eLocalActivatedCPU = 1,
        eLocalActivatedFloating = 2,
        eServerActivated = 3
    };

    enum LicenseItemType{
        eProduct,
        eFeature
    };
    enum eStatusBarUpdate{
        eLabel,
        eProgress,
        eBoth,
        eHide
    };

    enum eActivationMode{
        eOnline,
        eOffline
    };

protected:
    int initActivation();
    void intErrorCodeMapping();
    int initTabs();
    void setError(int error, const QString &message);
    void processFoundLicense(QTreeWidget *productsTree, LicenseActivatedType type, const QString &licenseNodeLabel, QByteArray &result);

public slots:
    void activatedLicenseType(int);
    void viewActivatedLicense();
    void viewRawData();
    void viewBuildProperty();

    void updateTerminal(const QByteArray &result);
    void activateLicense();
    void requestEvaluation();
    void operationMenu(const QPoint &);
    void activationModeGUIUpdate(int);
    void specifyQty(int);
    void activationModeGUIUpdateReqGB(bool );
    void activationModeGUIUpdateRspGB(bool );
    void getRspFileName(bool );
    void getReqFileName(bool );
    void getOpGenFileFileName(bool );
    void localRepair();
    void resetApp();
    void resetServer();
    void additionalKey();
    void removeAdditionalKey();
    void viewDetails() ;
    void viewProductProperty();



protected:
    void updateStatusBarLabel(eStatusBarUpdate type, const QString &label, int val, int max=-1, int min=0);
    void addLicenseToTreeWidget(QTreeWidget *productsTree,LicenseActivatedType type, const QString &licenseNodeLabel, QVariantMap &productData, QStringList &featureData);
    void information(const QString &info);
    QDate converToDate(const QString &dateString);
    QString dateToString(const QDate &date);
    void checkItem(QTreeWidgetItem *item, LicenseItemType type, QMap<QString, int> &headerItemContent);
    void doBorrow(const QString &commServer, const QString &entitlementID, const QString & productID );
    void returnBorrow(const QString &fulfillmentID, const QString &orgCommServer);
    QMap<QString, int> getHeaderItemContent(QTreeWidget *productsTree);
    bool getBorrowExpirationDate(const QString &message, QDate &expirationDate);
    bool getProxyDetails(QString &proxyDetails);
    void activateLicenseOnline(const QString &proxyDetails, const QStringList &keysList);
    void activateLicenseOffline(const QString &proxyDetails);
    void repairLicense (LicenseActivatedType itemType, const QString &fulfillmentID, const QString &orgCommServer);
    void returnLicenseTo(LicenseActivatedType itemType, const QString &fulfillmentID, const QString &orgCommServer);
    void deleteFulfillment(LicenseActivatedType itemType, const QString &fulfillment);
    void deleteProduct(LicenseActivatedType itemType, const QString &product, const QString &fulfillment);
    bool getOfflineOperation(QString &genFile);
    int getLicenseQty(int &licenseQty, const QString &entitlementID, bool &bEnterManualy);
    QTreeWidgetItem *addActivationEntry(const QString &entitlementID, const QString &qty);
    void addActivationDetails(QTreeWidgetItem *treeWidgetItem, const QString &data);
private:
    QString mFNPUtilsDir;
    int mError;
    QString mErrorMessage;
    QStringList mUtilsNeeded;
    QProcess *mFNPUtility;
    QByteArray mProcessData;
    ProcessWorker *mProcessWorker;
    QLabel *mStatusLabel;
    QProgressBar *mProgressBar;
    QString mGS_ACTIVIATION_URL;
    QMap<int, QString> mErrorCodeMapping;


};

// TODO : appacutil => -served (offline done) -return(to be done) -repair(to be done)
// TODO : serveracutil => -served (offline done) -return(to be done) -repair(to be done)
}
}
#endif // LICENSE_ACTIVATION_DIALOG_H

