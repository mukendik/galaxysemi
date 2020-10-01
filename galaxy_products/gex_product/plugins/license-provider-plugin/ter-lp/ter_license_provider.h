#ifdef WIN32

#include "license_provider.h"
#include "product_info.h"

#include <QDate>
#include <QtGlobal>
#include <QMetaType>
#include <QAbstractSocket>

#ifndef GEX_LICENSE_PROVIDER_H
#define GEX_LICENSE_PROVIDER_H

class QTcpSocket;

namespace GS
{
    namespace LPPlugin
    {
      class GEXMessage;

      /**
       * \brief Teradyne license provider plugin for use with our Teradyne Examinator versions built on Windows
       * with the TER_GEX define.
       */
      class TerLicenseProvider : public GS::LPPlugin::LicenseProvider
      {
          // Required for signal/slot support.
          Q_OBJECT
          Q_DISABLE_COPY(TerLicenseProvider)

      public :
          /**
           * \brief Creates a TerLicenseProvider singleton. Returns the pointer if singleton already created.
           *
           * \param parent pointer to the parent of the license provider objet (can be 0)
           * \param product the product instantiating the license provider
           * (see LicenseProvider::GexProducts for possible values)
           * \param appConfig a QVariantMap with key/value application properties
           * \param libErrorCode error code set by the function
           * \param libError error message set by the function
           *
           * \return a pointer to the TerLicenseProvider singleton on success, 0 else
           */
          static TerLicenseProvider *initializeLP(QObject* parent,
                                                  GexProducts product,
                                                  const QVariantMap &appConfig,
                                                  int &libErrorCode, QString &libError);

          /**
           * \brief Destroys the TerLicenseProvider singleton if it exists.
           */
          static void destroy();

      protected:
          /**
           * \brief Pointer on the TerLicenseProvider singleton.
           */
          static TerLicenseProvider* mInstance;

          /**
           * \brief Pointer on private class data.
           */
          class  TerLicenseProviderPrivate *mPrivate;

          /**
           * \brief Constructor of the TerLicenseProvider license provider class.
           *
           * \param parent pointer to the parent of the license provider objet (can be 0)
           * \param product the product instantiating the license provider
           * (see LicenseProvider::GexProducts for possible values)
           * \param appConfig a QVariantMap with key/value application properties
           */
          TerLicenseProvider(QObject *parent,
                             GexProducts product,
                             const QVariantMap &appConfig);

          /**
           * \brief Initialization of the license provider object (mainly instantiates the private data).
           *
           */
          int initialize();

          /**
           * \brief Cleans the license provider object (mainly deletes the private data).
           *
           */
          int cleanup();

          /**
           * \brief Processes a message sent by the application to the license provider.
           *
           * \param gexMessage the message sent by the application to the license provider
           * (see GEXMessage::MessageType for a list of possible message types)
           */
          void processGexMessage(const GEXMessage &gexMessage);

      private:
          /**
           * \brief Destructor of the TerLicenseProvider license provider class.
           */
           ~TerLicenseProvider();

      public slots:
          /**
           * \brief Checks if a license is available for the specified product.
           *
           * \param productInfo pointer to an object containing information about the product requesting the license
           * (such as the product ID, the edition, optional modules ...)
           *
           * \return true if the license could be granted, false else
           */
          bool checkLicense(ProductInfo *productInfo);

      public:
          //Customization functions
          /**
           * \brief returns the running modes supported by the licese provider (such as Evaluation, Standalone, Floating).
           *
           * \return the list of available running modes
           */
          QVariantList  getRunningModeAvailable();

          /**
           * \brief returns the absolute file name of the html page to be displayed in the license activation
           * Welcome dialog if the license provider supports selecting amongst multiple running modes.
           *
           * \return the absolute file name of the html page to be displayed in the license activation
           * Welcome dialog
           */
          QString       getWelcomePage();

      protected:
          /**
           * \brief Sets the internal error code and message.
           *
           * \param ErrorCode error code to set
           * \param ErrorMsg error message to set
           */
          void  setInternalError(int ErrorCode, const QString &ErrorMsg);

          /**
           * \brief Gets the internal error code and message.
           *
           * \param ErrorMsg will receive the internal error message
           *
           * \return the internal error code
           */
          int   getInternalError(QString &ErrorMsg);

      public:
          int   getExtendedError(QString &);

      private slots:
          /**
           * \brief Exception handler for TAGLMProxy Ter COM object.
           *
           * \param code the code of the error raised by the TAGLMProxy Ter COM object
           * \param source the source of the error raised by the TAGLMProxy Ter COM object
           * \param desc the description of the error raised by the TAGLMProxy Ter COM object
           * \param help some help text concerning the error raised by the TAGLMProxy Ter COM object
           */
          void  onException(int code, const QString& source, const QString& desc, const QString& help);
      };
    }
}

#endif // GEX_LICENSE_PROVIDER_H

#endif
