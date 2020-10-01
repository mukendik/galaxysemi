///////////////////////////////////////////////////////////
// All classes used for 'Scripting'
///////////////////////////////////////////////////////////

#ifndef GEX_SCRIPTING_H
#define GEX_SCRIPTING_H
#include "ui_script_dialog.h"
#include <stdio.h>

class GexScripting : public QDialog, public Ui::script_basedialog
{
    Q_OBJECT

public:
    GexScripting( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    void	WritePreferencesSection(FILE *hScriptFile);
    void	WriteMessage(QString);
    void	EraseFavoriteList(void);
    void	InsertFavoriteList(const char *szPath, const char *szTitle);
    //! \brief Clears Scripting console output content+variables executed when launching a script...
    void	ResetScriptConsole(void);
    QString	GetScriptConsoleMessage(QString strField);
    void	BuildHomePath(void);
    void	EnableScriptToolBar(bool bEnable);
    void	CloseScriptConsole();
    void	ShowHtmlConsole(bool bShow);
    void	ShowHtmlPage(const char *szPage);
    void	SetConfigurationField(char *szSection,char *szField,char *szValue);

    QString	strShowReportPage;	// name of the page to show at report launch ('home', etc...)


    QString	strGexConfLastFilePathSelected;	// Holds path to the last file selected under Gex
    long	lGexConfLastReminder;			// Holds time/date of last message reminder (Galaxy ads!).

protected:
    void	RunStartupScript(const QString& strScriptName);

private:

    bool	mInputReady;		// true when input Editbox has a message entered by user
    QString	mDatabaseField;     // holds Query field name if caller is 'sysPromptQuery'

    /*
     * \brief This function checks if the flag analyse mode has been setted and populate the appropiate
     *        data tab.
     *
     */
    bool PopulateDataTab(const QString &fileName);

    void RunScript(const QString &fileName);
public slots:
    void	reject(void);		// Called if user hits the ESCAPE key.
    void	OnConsoleInputReady(void);
    void	OnConsoleInputFileName(void);
    void	OnHtmlLink(const QUrl&);
    void	OnAddFile(void);
    void	OnProperties(void);
    void	OnRemoveFile(void);
    void	OnNewScript(void);
    void	OnEditScript(void);
    void	OnRunScript(void);
    void	OnLoadRunScript(void);
    void	OnEditExecutedScript(void);
    void	OnRunScriptAgain(void);


protected slots:

    void    OnScriptStarted(const QString&, bool);
    void    OnScriptFinished();
    void    OnScriptLog(const QString&);

signals:
    void OptionChanged(); // emitted when a favorite is edited/added/removed
};


class GexScriptingTreeItem : public QTreeWidgetItem
{
public:

    GexScriptingTreeItem(QTreeWidget * pTreeWidget, QStringList strTreeListItem);
    virtual ~GexScriptingTreeItem();

    bool operator< ( const QTreeWidgetItem & other ) const;

};

#endif
