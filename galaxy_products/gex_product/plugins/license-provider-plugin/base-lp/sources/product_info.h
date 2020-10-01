#ifndef PRODUCT_INFO_H
#define PRODUCT_INFO_H

#include <QObject>
#include "license_provider_global.h"
#include "license_provider.h"


#define ErrorNotAllowFunction "error : Your licence doesn't allow this function"


namespace GS
{
  namespace LPPlugin
  {

    class LICENSE_PROVIDERSHARED_EXPORT ProductInfo : public QObject
    {
      Q_OBJECT

    public:
        enum Capabilities {
            ftrCorrolation,
            advHisto,
            advMultiChart,
            advPearson,
            advProbabilityPlot,
            waferStack,
            toolbox,
            waferMap
        };

    private:

      // Constructor
      ProductInfo();
      // Copy constructor
      ProductInfo(const ProductInfo& other);
      //Destructor
      virtual ~ProductInfo();

      /// !brief This function is called only from the constructor.
      /// It fill the list of product and their restrictions
      void CreateCapabilityRestrictionByProduct();

    public slots:

      static ProductInfo* getInstance();
      static void Destroy();

      QString   GetProfileVariable() const;
      QString   GetProfileFolder() const;

      // Properties accessors
      GS::LPPlugin::LicenseProvider::GexProducts getProductID() const;
      void  setProductID(GS::LPPlugin::LicenseProvider::GexProducts productID);

      unsigned int getOptionalModules() const;
      QString setOptionalModules(unsigned int optionalModules);
      //! \brief Clear optional modules, set to 0.
      void ClearOptionalModules();

      long  getEditionID() const;
      void  setEditionID(long editionID);

      long  getLicenseRunningMode() const;
      void  setLicenseRunningMode(long licenseRunningMode);

      long  getMonitorProducts() const;
      void  setMonitorProducts(long monitoredProducts);

      // Methods to check the product type
      bool  isExaminatorEval() const;
      bool  isExaminator () const;
      bool  isExaminatorPRO() const;
      bool  isExaminatorTerProPlus() const;
      bool  isExaminatorTer() const;
      bool  isExaminatorPAT() const;
      bool  isYieldMan() const;
      bool  isPATMan() const;
      bool  isEnterprise() const;
      bool  isGTM() const;
      bool  isMonitoring() const;
      bool  isOEM(qlonglong ignore = 0) const;
      bool  isDBPluginAllowed() const;
      // Methode to check the option used
      bool  isGenealogyAllowed() const;
      bool  isSYLSBLAllowed() const;
      bool  isTDRAllowed() const;



      bool  isNotSupportedCapability(Capabilities capability);


/*
 * unused code
      bool isAdditionalFeatureAllowed(const QString &feature) const;

      void addAdditionalFeature(const QString &feature);
      void updateAdditionalFeatureStatus(const QString &feature, bool allowed);

      QMap<QString, bool> &AdditionalFeature();
*/
    private:

      static ProductInfo *mProductInfo;			// Static memeber used by the singleton
      // Members
      //! \brief Data type allowed to processby GEX (eg: any, or SZ, or Credence, etc..., ExaminatorDB)
      GS::LPPlugin::LicenseProvider::GexProducts mProductID;
      //! \brief Examinator Package Edition: Standard, Advanced,...
      long  mEditionID;
      //! \brief Running mode: evaluation (4days), or standalone or client/server
      long  mLicenseRunningMode;
      //! \brief Total number of products allowed for monitoring.
      long  mMonitorProducts;
      //! \brief Flags/bits Telling which optional modules are allowed: Plugin, PAT etc... (info read from license)
      unsigned int  mOptionalModules;
      // QMap<QString, bool> mAdditionalFeature; // unused member
      QMap<GS::LPPlugin::LicenseProvider::GexProducts, QList<Capabilities> > mCapabilityRestrictions;
    };


  }
}

#endif // PRODUCT_INFO_H
