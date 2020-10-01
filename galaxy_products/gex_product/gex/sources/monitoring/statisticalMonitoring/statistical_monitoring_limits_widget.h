#ifndef SATISTICAL_MONITORING_LIMITS_WIDGET_H
#define SATISTICAL_MONITORING_LIMITS_WIDGET_H

#include <QModelIndex>
#include <QStringList>

#include <QWidget>

#include "statistical_monitoring_task.h"

class QPushButton;
class QLayout;
class QTextEdit;
class QAction;
class QMenu;

namespace GS
{
namespace Gex
{

class FilterableTableView;
class TableModel;
class TableView;

class StatisticalMonitoringLimitsWidget : public QWidget
{
    Q_OBJECT
public:
    /// \brief Constructor
    explicit StatisticalMonitoringLimitsWidget(CGexMoTaskStatisticalMonitoring *task,
            const QString backupFile, QWidget* parent = 0);
    /// \brief Destructor
    virtual ~StatisticalMonitoringLimitsWidget();
    /// \brief Create Table view and return it
    virtual QWidget*    CreateTableView() = 0;
    /// \brief create with table view and filters
    void CreateDataUI(QLayout *layout);
    /// \brief Create send button and connect it
    void CreateSendButton(QLayout *layout);
    /// \brief Create comment area
    void CreateCommentArea(QLayout *layout);
    /// \brief True if data model is empty
    bool IsDataModelEmpty();
    /// \brief Reload buttons and apply filters
    void RefreshUI(bool enableUpdateButtons);
    /// \brief dump the verion label in the task
    bool DumpFields();
    /// \brief Load a draft task
    bool LoadDraftTask(int versionId, CGexMoTaskStatisticalMonitoring* draftTask,
                       bool lastIdVersion, QString &outProduct, QString &outLabel);
    /// \brief Customize the limit view with an item delegate to generate
    /// combo while editing algo
    /// return the list of supported outlier algorithm
    virtual QMap<QString, QString> AddItemDelegateOutlierAlgo() = 0;
    /// /brief load limits from DB
    bool LoadLimitsFromDatabase();

    /// \brief Getters
    TableModel* GetModel() const;
    TableView *GetTableView()  const;
    void SetReadOnly(bool readOnly);

signals:
    void sUpdateClicked();
    /// \brief emitted when validation succeeded
    void sValidationSucceeded();

public slots:
    /// \brief when model updated call function to update the DB
    void OnModelUpdated(QModelIndex topLeft, QModelIndex bottomRight,
                        QVector<int> roles);
    /// \brief Update limits in DB and refresh UI
    void OnCopyToClipboardRequested();
    /// \brief Update limits in DB and refresh UI
    void OnSaveToCsvRequested();
    /// \brief Update limits in DB and refresh UI
    void OnUpdateLimitRequested();
    /// \brief Validate version
    void OnValidateVersionRequested();

    void OnChangeAlarmStatusRequested(QAction *action);
    void OnChangeLimitRequested(QAction *action);
    void OnChangeAlgoRequested(QAction *action);
    void OnCustomMenuRequested(QPoint pos);
protected:
    /// \brief Update DB when model is updated
    void UpdateModel(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles);
    /// \brief Fill table model with query model
    void FillTableModel(QSqlQuery &query, bool readOnly = false);
    /// \brief Load limit from DB
    bool LoadLimitsFromDatabasePrivate(bool readOnly = false,
                                       bool lastProdVersion = false);
    /// \brief Load limit model
    bool LoadLimitsModel(bool readyOnly = false, bool lastProdVersion = false);
    /// \brief Update view
    void UpdateLimitsTableView();
    /// \brief Return index of default column to sort
    virtual int GetDefaultColumnToSort();
    void InitContextMenu(const QMap<QString, QString> availableAlgos);
    /// \brief Update specific table view
    virtual void UpdateSpecificsTableView() = 0;

    FilterableTableView*                mLimitsView;                ///< Holds ptr limits table view
    CGexMoTaskStatisticalMonitoring*    mTask;                      ///< Holds ptr to task
    CGexMoTaskStatisticalMonitoring*    mTaskDraft;                 ///< Holds ptr to a draft task
    QList<QPair<QString, QAction* > >   mChangeAlgoActions;         ///< Holds ptr to actions list for context menu
    TableModel*                         mModel;                     ///< Holds ptr to model
    QStringList                         mShownFields;               ///< Holds shown fields in the model
    QStringList                         mHiddenFields;              ///< Holds hidden fields in the model
    QTextEdit*                          mComment;                   ///< Holds ptr to comment
    QMenu*                              mContextMenu;               ///< Holds ptr to context menu
    QPushButton*                        mPushButtonUpdate;          ///< Holds ptr to button to update limits
    QPushButton*                        mPushButtonCopyToClipboard; ///< Holds ptr to button to update limits
    QPushButton*                        mPushButtonSaveToCsv;       ///< Holds ptr to button to update limits
    QPushButton*                        mPushButtonSendProd;        ///< Holds ptr to button record limit as prod limits
    QAction*                            mDisableAlarmAction;        ///< Holds ptr to action for context menu
    QAction*                            mEnableAlarmAction;         ///< Holds ptr to action for context menu
    QAction*                            mDisableHighLimitAction;    ///< Holds ptr to action for context menu
    QAction*                            mEnableHighLimitAction;     ///< Holds ptr to action for context menu
    QAction*                            mChangeHighLimitValueAction;///< Holds ptr to action for context menu
    QAction*                            mDisableLowLimitAction;     ///< Holds ptr to action for context menu
    QAction*                            mEnableLowLimitAction;      ///< Holds ptr to action for context menu
    QAction*                            mChangeLowLimitValueAction; ///< Holds ptr to action for context menu
    QAction*                            mChangeParamAction;         ///< Holds ptr to action for context menu
    bool                                mReadOnly;                  ///< True if widget in read only mode
};


} // NAMESPACE Gex
} // NAMESPACE GS


#endif

