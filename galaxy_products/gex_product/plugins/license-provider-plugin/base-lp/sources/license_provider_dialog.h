#include <gqtl_sysutils.h>
#include <gqtl_skin.h>
#include "gex_version.h"

#include "ui_license_provider_dialog.h"
#include "license_provider_global.h"

#ifndef LICENSE_PROVIDER_DIALOG_H
#define LICENSE_PROVIDER_DIALOG_H


namespace GS
{
    namespace LPPlugin
    {
      class LicenseProvider;
      ///
      /// \brief The LicenseProviderDialog class is the dialog for the welcome page
      ///
      class LICENSE_PROVIDERSHARED_EXPORT LicenseProviderDialog : public QDialog, public Ui::LicenseProviderDialog
      {
        Q_OBJECT
        QString	mDetectedArguments;
        // Holds skins parametes for Gex application,...
        CGexSkin		m_gexSkin;
        LicenseProvider *mProvider;
        bool tryLegacy;

      public:
        LicenseProviderDialog(LicenseProvider *provider,QWidget* parent = 0, bool modal = false, Qt::WindowFlags f= 0);
        virtual ~LicenseProviderDialog();
      public slots:
        void	LoadWelcomePage(void);
        bool	Init(void);
        void	OnOkButton(void);	// User clicked the OK button
        void	OnExitButton(void);	// User clicked the Exit button
        void	OnLegacyButton(void);	// User clicked the Exit button
        void	OnRunningMode(int);// User changed the running mode selection
        void    hideLegacyButton(bool hide);
        bool    getLegacyStatus();

      public:
        CGexSkin &getGexSkin();
        QString getEditField2(){
          return editField2->text();
        }
        QString getEditField1(){
          return editField1->text();
        }
        int getSelectionMode();

      };
    }
}

#endif
