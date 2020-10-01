#ifndef LICENSE_PROVIDER_H
#define LICENSE_PROVIDER_H

#include "license_provider_global.h"
#include "server_file_descriptorIO_factory.h"
#include "server_file_descriptorIO.h"
#include <QObject>
#include <QMap>
#include <QVariant>

class QThread;
namespace GS
{
  namespace LPPlugin
  {
    //the following strings are used when trying to retrieve the GTL increment when gex runs in GTM mode
    static const char *const GTLOperations = "GTL:";
    static const char *const GTLRequestLicense = "REQUEST:";
    static const char *const GTLReleaseLicense = "RELEASE:";
    static const char *const GTLResponseNOLicense = "RESPONSE_NOLIC:";
    static const char *const GTLResponseLicense = "RESPONSE_LIC:";

    class ProductInfo;
    class LicenseProviderPrivate;
    class GEXMessage;
    class LPMessage;

    ////
    /// \brief The LicenseProvider class is the abstract class defining the interface for a license provider
    ///
    class LICENSE_PROVIDERSHARED_EXPORT LicenseProvider : public QObject
    {
      Q_OBJECT
      Q_DISABLE_COPY(LicenseProvider)

    public:

        ///
        /// \brief The GexProducts enum is defining the different supported products
        ///
      enum GexProducts
      {
        eNoProduct                  = 0x00000,
        eLtxcOEM                    = 0x00001,
        eAslOEM                     = 0x00002,
        eSzOEM                      = 0x00004,
        eSapphireOEM                = 0x00008,
        eTerOEM                     = 0x00010,
        eExaminatorEval             = 0x00100,
        eExaminator                 = 0x00200,
        eExaminatorPro              = 0x00400,
        eTerProPlus                 = 0x00800,
        eExaminatorPAT              = 0x01000,
        eYieldMan                   = 0x02000,
        ePATMan                     = 0x04000,
        eGTM                        = 0x08000,
        eYieldManEnterprise         = 0x10000,
        ePATManEnterprise           = 0x20000

      };
      ///
      /// \brief The GexOptions enum is defining the different options supported
      ///
      enum GexOptions
      {
        eNoOptions             = 0x00,
        eSYA                   = 0x01,
        eTDR                   = 0x02,
        eGenealogy             = 0x04
      };

      ////
      /// \brief The LPError enum defines a list of encountred error when trying to load the LP plugin
      ///
      enum LPError
      {
        eLPLibNoError          = 0x0,   // No error found
        eLPLibCallBackNotFound = 0x01,  // is set when a call back function is not found in the LP plugin loaded
        eLPLibNotFound         = 0x02,  // is set when the library is not found
        eLPSymbolNotFound      = 0x04,  // is set when a symbol is not found inside the library
        eLPInternalError       = 0x08,  // is set when a interbal error occured
        eLPProductNotFound     = 0x10,  // is set when the asked product is not found
        eLPLibMismatch         = 0x12,  // is set when lib version mismatch is found
        eLPAlloc               = 0x14,  // is set when failed to allocate some objects
        eLPWrongEdition        = 0x18,  // is set when the provider doesn't support the requested product edition (ie fnp_lp with TER edition)
      };

    private:
      /// Data class used to store the different attributs of the LP provider
      LicenseProviderPrivate *mPrivate;

    protected:
      // Default constructor
      LicenseProvider(QObject* ,
                      GexProducts ,
                      const QVariantMap &);
      // destructor
      virtual ~LicenseProvider();

    public:
      // Function used to checkout (retrieve a license)
      virtual bool checkLicense(ProductInfo *) = 0;
      // Function used for evaluation mode
      virtual bool createEvaluationLicense(ProductInfo *);
      // Function used to get the last error encoutred
      virtual int getExtendedError(QString &m) = 0;
      // Function used to bypass the signal/slot mecanism between gex and lp
      virtual void DirectRequest(const GEXMessage &,LPMessage &);

    protected :

      virtual int initialize() = 0;
      virtual int cleanup() = 0;
      // this the methode used to process the GEX messages
      virtual void processGexMessage(const GEXMessage &) = 0;

      // set the error
      virtual void setLastError(int , const QString &);

      ServerFileDescriptorIO* mServersDescription;

    public :
      // Getter/Setter  for error
      virtual int getLastErrorCode();
      virtual QString getLastError();
      const QString getLPType() const {return property(LP_TYPE).toString();}
      // return the last license(type+config) used
      QVariant getLastChoice();
      // Save the last license(type+config) used
      void saveLastChoice(const QString &runningMode, const QString &editField1, const QString &editField2);
      // Save the last license status
      void SaveLastChoiceStatus(bool status);
      // Get the last license status
      bool GetLastChoiceStatus();

    public slots:
      virtual QVariant getLPData (const QString &);
      virtual void setLPData (const QString &, const QVariant &);
      virtual QVariant getAppConfigData (const QString &);
      GexProducts getProduct();
      void setProduct(LicenseProvider::GexProducts val);

#ifndef QT_NO_DEBUG
      QVariantMap &getFullLPData();
#endif
    protected:
      QVariantMap &getFullAppConfigData();

    signals:
      // sent an LP Message
      void sendLPMessage(const GS::LPPlugin::LPMessage &);
    public slots:
      // receive the the GEXMessage
      void receiveGexMessage(const GS::LPPlugin::GEXMessage &);

    public:
      //Customization functions
      virtual QVariantList getRunningModeAvailable() = 0;
      virtual QString getWelcomePage() = 0;

      void ConvertAndSaveToJsonFormat();
    signals://USER interaction signals
      void userChoice(const QStringList *choices, int &choice);
      // signal used to show a splash durring a background action that will conatins the message "message"
      //mode parameter will be used to hide or show the splash
      void ShowWaitingMessage(const QString &message, int mode);
      void userAvailableChoices(const QString *, const QStringList *, bool activate ,QString *);
      // blocking signal emmited to get the the evaluation order id
      void userEvalLicense(const QString *, QString *);
      // signal used to notify the user
      void notifyUser(const QString *);
      // signal to show a dialog for the user
      void userInteraction(GS::LPPlugin::LicenseProvider *provider, QString *, QString *,
                           int *selectionMode, bool &b, bool &result, bool legacy);
      // signal to notify in the deamon mode
      void notifyDaemon();
      // signal to open a URL
      void openURL(const QString *);

    };



    typedef GS::LPPlugin::LicenseProvider* (*lp_initialize_function)(QObject* ,
                                                                     GS::LPPlugin::LicenseProvider::GexProducts ,
                                                                     const QVariantMap &,
                                                                     int &,
                                                                     QString &);
    typedef void (*lp_destroy_function)();

  }
}

Q_DECLARE_METATYPE(GS::LPPlugin::LicenseProvider::GexProducts)
Q_DECLARE_METATYPE(GS::LPPlugin::LicenseProvider::GexOptions)
Q_DECLARE_METATYPE(GS::LPPlugin::LicenseProvider::LPError)

#endif // LICENSE_PROVIDER_H
