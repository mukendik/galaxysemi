/**
\file		gexdb_plugin_galaxy_cfgwizard.h
\author		PYC, SC
\date		10/11/2011
\brief		GUI used to connect / create databases (gexdb, ym_admin_db)
*/

#ifndef GEXDB_PLUGIN_GALAXY_CFGWIZARD_H
#define GEXDB_PLUGIN_GALAXY_CFGWIZARD_H

// Galaxy modules includes
//#include <gqtl_sysutils.h>

#include "ui_gexdb_plugin_galaxy_cfg_wizard.h"
#include "gexdb_plugin_galaxy_database_settings.h"
#include "gexdb_plugin_base.h"
#include "gexdb_plugin_galaxy.h"
#include <QPushButton>
#include <QWizard>

class CGexSkin;

/**
    \brief      interface used to create / modify a database link
    */
class GexDbPlugin_Galaxy_CfgWizard : public QWizard,
        public Ui::GexDbPlugin_Galaxy_CfgWizard_Base
{
    Q_OBJECT

public:
    /**
        \param strHostName          : IP / host name
        \param strApplicationPath   : application directory
        \param strUserProfile       : user profile directory
        \param bCustomerDebugMode   : used to write specific debug messages
        \param bWizardReadOnly      : used to specify gui options
        \param pclDbPluginGalaxy    : pointer to the associated plugin galaxy
        \param pGexSkin             : pointer to the current skin used in gex
        \param parent               : pointer to the parent widget
      */
    GexDbPlugin_Galaxy_CfgWizard( const QString & strHostName,
                                  const QString & strApplicationPath,
                                  const QString & strUserProfile,
                                  const bool bCustomerDebugMode,
                                  const QMap<QString,QString> &guiOptions,
                                  GexDbPlugin_Galaxy *pclDbPluginGalaxy,
                                  CGexSkin * pGexSkin,
                                  QWidget* parent = 0);

    ~GexDbPlugin_Galaxy_CfgWizard();


    /**
          \brief method used to set class options

          \param	clDbConnector : Connector used to set internal connector
          \param	bAutomaticStartup : Database link option

          \return   no return
          */
    void Set(const GexDbPlugin_Connector & clDbConnector, bool bAutomaticStartup);

    /**
          \brief    method to get database connection parameters
          \details  developper note : connector should be updated at
                the right time during process
                (after succesfull connect or succesfull create).
                If the user 'reject()' all gui fields should be empty

          \param	clDbConnector : connector reference
                    (used to copy the internal connector)
          \param	pbAutomaticStartup :


          */
    void Get(GexDbPlugin_Connector & clDbConnector, bool *pbAutomaticStartup);

    // accessors
    bool            SetDataBaseType(const GexDbPlugin_Galaxy::DataBaseType dbtEnuDataBaseType);
    GexDbPlugin_Galaxy::DataBaseType    GetDataBaseType() const;
    // Gui option
    /// \return true if the wizard has been opened in read only mode
    bool            IsReadOnly();
    /// \return true if the wizard has been opened in creation mode
    bool            IsOpenModeCreation();
    /// \return true if the db creation is allowed
    bool            IsLinkCreationAllowed();

private:
    // internal datas attributes
    GexDbPlugin_Connector       *m_pclCopyConnector;            // Database connector
    GexDbPlugin_Galaxy          *m_pclDbPluginGalaxy;
    GexDbPlugin_Galaxy::DataBaseType  m_eDataBaseType;
    bool                        m_bCustomerDebugMode;
    const QString               m_strApplicationPath;         // Application directory
    const QString               m_strUserProfile;             // User profile directory

    // gui attributes
    const CGexSkin            *m_pGexSkin;

    // gui and internal datas attributes
    QString                   m_strHostName;                  // Host name
    QStringList               m_strlDrivers;                  // List of QT SQL database drivers. Ie "QOCI8"
    QStringList               m_strlDrivers_FriendlyNames;    // List of QT SQL database drivers (Friendly names). Ie "Oracle (QOCI8)"
    QMap<QString, QString>    mGuiOptions;


    ////////////////////////////////
    // internal methods
    ////////////////////////////////

    /////////////////////////////////////////
    // processing methods
    /** \todo sqlite database management */
    bool	GenericCreateDatabase();
    bool    GenericConnectDataBase();


    /// \brief Check the database with the right type with a hash s key
    /// \param dbType requested type
    /// \param adminConnector connector to the database
    /// \param sqlQuery object to query the database
    bool    CheckDbTdrType(QString dbType);

    void	InsertIntoUpdateLog(const QString & strMessage, bool bRed=false);
    void	WriteDebugMessageFile(const QString & strMessage);
    void    Connect();


    /////////////////////////////////////////////////////////////////
    // update methods return false if a problem occurs during update
    /////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // methods used to synchronize connector and gui
    bool    UpdateConnectorFromGui(GexDbPlugin_Connector& gdpcRefConnector) const;
    bool    UpdateGuiFromConnector();

    /////////////////////////////////////////////////////
    // page specific methods used to update the gui
    bool    UpdateSettingPage();
    bool    UpdateCreateDbPage();
    bool    UpdateConnectDbPage();
    void    InitComboTdrDbType();

    // has to respect gexdb_plugin_galaxy_cfg_wizard.ui ID
    enum    WizardPageID { eDbSettingsPage = 0,
                           eDbCreationPage = 1,
                           eDbConnectionPage = 2 };

    bool CreateDatabase(const GexDbPlugin_Connector settingConnector,
                        QStringList &lstUninstall,
                        bool lActiveKeepComment = false,
                        bool adrDB = false);
private slots:
    /////////////////////////////////////////////////
    // slots used to dynamically manage some datas
    void        OnDatabaseDriverChanged();
    void        OnDbCreationChanged();
    void        OnSettingsChange();
    void        OnSettingsChange(QString);
    void        OnAdminNameChanged(QString strNewAdminName);

    /**
      \brief generic slot used to manage update gui according to curreent page.
      */
    void        UpdateGui(const int nPageToUpdateId = -1);

    void        Clear();
    void        ClearLogs();
    // Update mUi_qteCreateDbLog withe the message
    void        UpdateLogMessage(const QString &message, bool isPlainText);
    // Update GexProgressBar progress
    void        UpdateProgress(int prog);
    // Reset GexProgressBar progress
    void        ResetProgress(bool forceCompleted);
    // Set Maximumfor GexProgressBar progress
    void        SetMaxProgress(int max);

protected slots:
    /**
      \brief back method is 'overloaded' to restart wizard navigation
            (run the settings page). See GexDbPlugin_Galaxy_CfgWizard::Connect() method
      */
    void        back();

protected:
    /**
      \brief nextId method is overloaded to manage a specific navigation. \n
      <ul>
      <li>Setting page -> Create Db page if create db check box is checked
      <li>Setting page -> Connect Db page if create db check box is not checked
      <li>Create Db page -> Connect db page
      </ul>
      \n
      Buttons text are dynamically changed to help user to understand flow.
      see methods used to update gui.
      */
    int        nextId() const;


};

#endif // GEXDB_PLUGIN_GALAXY_CFGWIZARD_H
