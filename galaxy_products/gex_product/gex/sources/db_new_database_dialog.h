///////////////////////////////////////////////////////////
// Class to create a New Database Entry
///////////////////////////////////////////////////////////

#ifndef GEX_NEW_DATABASE_H
#define GEX_NEW_DATABASE_H

#include <QPalette>


#include "ui_db_new_database_dialog.h"
#include "db_external_database.h"

#define PAGE_COORP_DATABASE 0
#define PAGE_PROTECTION     1

class CreateNewDatabaseDialog : public QDialog, public Ui::NewDatabaseDialogBase
{
    Q_OBJECT

public:
    CreateNewDatabaseDialog( QWidget* parent = 0, bool modal = false);
    ~CreateNewDatabaseDialog();

    bool						GexDbPlugin_WriteSettings(const QString & strSettingsFile);
    void						GexDbPlugin_GetLastError(QString & strError);
    GexDbPlugin_ID				*m_pCurrentPlugin;		// Plugin currently selected
    GexDbPlugin_ID				*m_pAdrPlugin;          // Plugin ADR is any

private:
    GexRemoteDatabase			*m_pRemoteDatabase;		// Remote database object
    QList<GexDbPlugin_ID*>		m_pPluginList;			// List of available plugins
    bool						m_bExpanded;			// true if Dialog box shows all fields (expanded)
    QString						m_strExportDatabaseSettings; // Path to Database settings exported (XML file).

private:
    void	UpdateGui();
    void	CorporateDb_UpdateDataFromGui(bool bUpdateData_DB=true, bool bUpdateData_FTP=true);
    bool    IsDbLinkNameValid(QString& strMessageToDisplay);

private slots:
    void	OnButtonConfigurePlugin();
    void	UpdateCreateButton();
    void	OnPasswordImportFiles(void);
    void	OnPasswordDeleteDatabase(void);
    void	OnMore(void);
    void	OnStorageType();
    void	OnImportCorporateDatabase();
    void	OnExportCorporateDatabase();
    void	OnDatabasePluginChanged();
    void	OnButtonCreate();
};

#endif
