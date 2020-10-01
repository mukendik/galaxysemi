///////////////////////////////////////////////////////////
// All classes used for 'Analyze ONE file'
///////////////////////////////////////////////////////////

#ifndef SQL_HOUSEKEEPING_DIALOG_H
#define SQL_HOUSEKEEPING_DIALOG_H

#include "ui_db_sql_housekeeping_dialog.h"
#include <QDialogButtonBox>
#include <QItemDelegate>
#include <QPainter>

#define UPDATE_ITEM_INDEX               0
#define INCREMENTAL_ITEM_INDEX          1
#define BACKGROUND_TRANSFER_ITEM_INDEX  2
#define METADATA_ITEM_INDEX             3
#define GLOBAL_SETTINGS_ITEM_INDEX      4
#define RELEASE_HISTORY_ITEM_INDEX      5
#define PURGE_ITEM_INDEX                6
#define CONSOLIDATION_ITEM_INDEX        7


class GexDatabaseEntry;
class GexRemoteDatabase;
class GexDbPlugin_Base;

class BackgroundProgressDelegate : public QItemDelegate
    {
        Q_OBJECT
    public:
        BackgroundProgressDelegate (QObject *parent);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    };

class BackgroundStatusDelegate : public QItemDelegate
    {
        Q_OBJECT
    public:
        BackgroundStatusDelegate (QObject *parent);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};



class GexSqlHousekeepingDialog : public QDialog, public Ui::sql_housekeeping_basedialog
{
    Q_OBJECT

public:
    // Constructor / destructor functions
    GexSqlHousekeepingDialog(const QString & strDatabaseName, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~GexSqlHousekeepingDialog();

private:
    /// \brief enable/disable button in list depending on rules
    void ResetItemSelectionList(bool isDbUpToDate, unsigned int databaseBuild);
    void fillCellData(QTableWidget *ptTable,int iRow,int iCol,QString strText,QString strTooltip="",bool bReadOnly=false,bool bAlarm=false);
    void LoadIncrementalSettings();
    void LoadOptionSettings();
    void UpdateInnoDbControls(GexDbPlugin_Base *pDbPlugin);
    void UpdateSecurityDbControls(GexDbPlugin_Base *pDbPlugin);
    int  UpdateDbSummaryDialog(QString strCommand, QTextEdit *pTextEdit_Log=NULL);
    /// \brief Manage the Event Scheduler for the Backgroung Transfer
    QString EventStatus();
    void CreateBackgroundEvent();
    void DropBackgroundEvent();
    void DisableBackgroundEvent();
    void EnableBackgroundEvent();
    void UpdateStartStopBGEventButton();
    /// \brief Trace message into the background_transfer_logs table
    void UpdateBGLogMessages(const QString& action, const QString& desc,
                             const QString& status, const QString& message);

    GexDatabaseEntry    *m_pTdrDatabaseEntry;
    GexDbPlugin_Base    *m_TdrDbPlugin;
    bool                m_bHaveAdr;
    GexDatabaseEntry    *m_pAdrDatabaseEntry;
    GexDbPlugin_Base    *m_AdrDbPlugin;

    QMap<int,QString>   m_mapOptionsDefined;
    QPushButton         *m_pToInnoDBPushButton;
    QLabel              *m_pEngineLabel;
    QLineEdit           *m_pEngineValueLabel;

    QList<int>          m_lstOptionsAllowed;

    bool                m_bUpdateProcessRunning;
    bool                m_bAbortRequested;
    QProgressBar        *mCurrentProgressBar;
    QString             m_strCurrentTestingStage;
    int                 m_nCurrentSection;
    /// \brief to show/hide the background update tables
    bool                mShowMoreBackgroundTableInfo;
    /// \brief to show/hide the background update partitions
    bool                mShowMoreBackgroundPartitionInfo;
    /// \brief to show/hide the background update Logs
    bool                mShowMoreBackgroundLogs;

    // To cancel close or change section when options are updated without apply
    // Ask if have to cancel change requested
    bool                CancelChangeRequested();

private slots:
    // Update selected TextEdit
    void UpdateLogMessage(QTextEdit *textEdit, const QString &message, bool isPlainText);
    // Call UpdateLogMessage() on editLog with message
    void UpdateStandardLog(const QString &message, bool isPlainText);
    // Call UpdateLogMessage() on editLog_Incremental with message
    void IncrementalUpdateLog(const QString &message, bool isPlainText);
    // Call UpdateLogMessage() on editLog_MetaData with message
    void UpdateMetadataLog(const QString &message, bool isPlainText);
    // Update GexProgressBar progress
    void UpdateProgress(int prog);
    // Reset GexProgressBar progress
    void ResetProgress(bool forceCompleted);
    // Set Maximumfor GexProgressBar progress
    void SetMaxProgress(int max);

    /// \brief fill all background update informations
    void FillBackgroundTransferInformations();
    void FillProgressInfo(int currentUpdateId);

public slots:
    void OnDatabaseRefresh();
    void OnButtonUpdate();
    // slot called when user clicks "Update to InnoDB"
    void OnButtonUpdateToInnoDB();
    void OnButtonIncremental_Update(QString strIncrementalName=QString());
    void OnButtonIncremental_Check();
    void OnButtonIncremental_Apply();
    void OnButtonIncremental_Cancel();
    void OnIncrementalUpdateContextualMenu(const QPoint& ptMousePoint);
    void OnItemSelected(int nItem);
    void OnTestingStageSelected();
    void OnButtonMetaData_Cancel();
    void OnButtonMetaData_Apply();
    void OnButtonBackgroundMoreInfo();
    void OnButtonBackgroundMorePartitionInfo();
    void OnButtonBackgroundMoreLogs();
    void OnCellMetaData(int nRow,int nCol);
    void OnTableMetaDataContextualMenu(const QPoint& ptMousePoint);
    void OnButtonOptionSettings_Cancel();
    void OnButtonOptionSettings_Apply();
    void OnCellOptionSettings(int nRow,int nCol);
    void OnCellIncrementalSettings(int nRow,int nCol);
    void OnTableOptionSettingsContextualMenu(const QPoint& ptMousePoint);
    void OnPurgeDataBase();
    void OnStartStopBGEvent();

    void    onAccept(void);
};
#endif
