
#include <QSortFilterProxyModel>
#include <QTextEdit>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QClipboard>
#include <QTableView>
#include <QFileDialog>

#include "qguiapplication.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_task.h"
#include "statistical_monitoring_limits_widget.h"
#include "statistical_monitoring_engine.h"
#include "statistical_monitoring_tables.h"
#include "common_widgets/filterable_table_view.h"
#include "gex_database_entry.h"
#include "gexdb_plugin_base.h"
#include "db_external_database.h"
#include "common_widgets/filterable_table_view.h"
#include "gqtl_log.h"
#include "mo_task.h"
#include "limits_model.h"
#include "common_widgets/table_view.h"
#include "engine.h"
#include "db_engine.h"
#include "admin_engine.h"
#include "message.h"
#include "gex_constants.h"

#include "iostream"

namespace GS
{
namespace Gex
{

StatisticalMonitoringLimitsWidget::StatisticalMonitoringLimitsWidget(
        CGexMoTaskStatisticalMonitoring *task,
        const QString backupFile, QWidget *parent):
    mTask(task),
    mLimitsView(new FilterableTableView(backupFile, parent)),
    QWidget(parent),
    mReadOnly(false),
    mModel(0),
    mTaskDraft(0),
    mComment(0),
    mContextMenu(0),
    mPushButtonCopyToClipboard(0),
    mPushButtonSaveToCsv(0),
    mPushButtonUpdate(0),
    mPushButtonSendProd(0),
    mDisableAlarmAction(0),
    mEnableAlarmAction(0),
    mDisableHighLimitAction(0),
    mEnableHighLimitAction(0),
    mChangeHighLimitValueAction(0),
    mDisableLowLimitAction(0),
    mEnableLowLimitAction(0),
    mChangeLowLimitValueAction(0),
    mChangeParamAction(0)
{
}

StatisticalMonitoringLimitsWidget::~StatisticalMonitoringLimitsWidget()
{
    delete mTaskDraft;
    // do not delete mLimitsView or buttons, done by parent widget

    // Clean actions
    delete mDisableAlarmAction;
    delete mEnableAlarmAction;
    delete mDisableHighLimitAction;
    delete mEnableHighLimitAction;
    delete mChangeHighLimitValueAction;
    delete mDisableLowLimitAction;
    delete mEnableLowLimitAction;
    delete mChangeLowLimitValueAction;
    for (int lIt = 0; lIt < mChangeAlgoActions.size(); ++lIt)
    {
        delete mChangeAlgoActions[lIt].second;
    }
    delete mChangeParamAction;
    delete mContextMenu;
}

void StatisticalMonitoringLimitsWidget::CreateDataUI(QLayout *layout)
{
    layout->setContentsMargins(0,0,5,0);

    if (!mReadOnly)
    {
        QFrame *lLimitsButtonsFrame = new QFrame();
        lLimitsButtonsFrame->setLayout(new QHBoxLayout(this));
        lLimitsButtonsFrame->layout()->setContentsMargins(0, 0, 0, 0);
        static_cast<QVBoxLayout*>(lLimitsButtonsFrame->layout())->addSpacerItem(
                    new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        mPushButtonCopyToClipboard = new QPushButton();
        mPushButtonCopyToClipboard->setText("copy to clipboard");
        lLimitsButtonsFrame->layout()->addWidget(mPushButtonCopyToClipboard);
        mPushButtonSaveToCsv = new QPushButton();
        mPushButtonSaveToCsv->setText("save to csv");
        lLimitsButtonsFrame->layout()->addWidget(mPushButtonSaveToCsv);
        mPushButtonUpdate = new QPushButton();
        mPushButtonUpdate->setText("update limits");
        lLimitsButtonsFrame->layout()->addWidget(mPushButtonUpdate);
        layout->addWidget(lLimitsButtonsFrame);
    }

    QWidget* lTableView = CreateTableView();
    if(lTableView)
    {
        layout->addWidget(lTableView);
    }

    // create item delegate for table view to represent combo/lineedit...
    QMap<QString, QString> lAvailableAlgos = AddItemDelegateOutlierAlgo();

    // Controls
    // Computation buttons
    connect(mPushButtonCopyToClipboard, SIGNAL(clicked(bool)),
            this, SLOT(OnCopyToClipboardRequested()));
    connect(mPushButtonSaveToCsv, SIGNAL(clicked(bool)),
            this, SLOT(OnSaveToCsvRequested()));
    connect(mPushButtonUpdate, SIGNAL(clicked(bool)),
            this, SLOT(OnUpdateLimitRequested()));
    connect(mPushButtonSendProd, SIGNAL(clicked(bool)),
            this, SLOT(OnValidateVersionRequested()));
    // Table view controls
    connect(mLimitsView->GetTableView(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnCustomMenuRequested(QPoint)));

    // Init contextMenu
    InitContextMenu(lAvailableAlgos);
}

void StatisticalMonitoringLimitsWidget::CreateSendButton(QLayout *layout)
{
    mPushButtonSendProd = new QPushButton();
    mPushButtonSendProd->setText("send limits to production >>");
    layout->addWidget(mPushButtonSendProd);
}

void StatisticalMonitoringLimitsWidget::CreateCommentArea(QLayout *layout)
{
    layout->setContentsMargins(0,0,0,0);
    mComment = new QTextEdit();
    mComment->setReadOnly(mReadOnly);
    layout->addWidget(mComment);
}

bool StatisticalMonitoringLimitsWidget::IsDataModelEmpty()
{
    return (!mModel || mModel->rowCount() == 0);
}

void StatisticalMonitoringLimitsWidget::RefreshUI(bool enableUpdateButtons)
{
    mLimitsView->RefreshView();
    mLimitsView->GetTableView()->SortData();

    if (mPushButtonUpdate)
        mPushButtonUpdate->setEnabled(!enableUpdateButtons);
    if (mPushButtonSendProd)
        mPushButtonSendProd->setEnabled(enableUpdateButtons);

    if (mComment)
        mComment->setPlainText(mTask->GetAttribute(C_VERSION_LABEL).toString());
}

bool StatisticalMonitoringLimitsWidget::LoadLimitsModel(bool readyOnly /* =false */,
                                                        bool lastProdVersion/* =false */)
{
    QSqlQuery lQuery(QSqlDatabase::database(GS::Gex::Engine::GetInstance().
                                            GetAdminEngine().
                                            m_pDatabaseConnector->m_strConnectionName));

    QString lQueryString = mTask->GetLimitsExtractionQuery(mShownFields,
                                                           mHiddenFields,
                                                           lastProdVersion);

    if(!lQuery.exec(lQueryString))
    {
        Engine::GetInstance().GetAdminEngine().SetLastError(
                    AdminEngine::eDB_Query,
                    lQueryString,
                    QString::number(lQuery.lastError().number()),
                    lQuery.lastError().text());
        GS::Gex::Message::warning("Error getting limits",
                                  "Error getting limits from database: " +
                                  lQuery.lastError().text() + "\nWith query: " +
                                  lQuery.lastQuery());

        return false;
    }

    mModel->clear();

    // Fill table model with query result
    FillTableModel(lQuery, readyOnly);

    return true;
}

void StatisticalMonitoringLimitsWidget::UpdateLimitsTableView()
{
    if (mLimitsView && !mLimitsView->GetTableView()->model())
    {
        mLimitsView->SetModel(mModel, GetDefaultColumnToSort());
    }

    // Hide IDs columns
    for (int lIt = 0; lIt < mModel->columnCount(); ++lIt)
    {
        if (mHiddenFields.contains(mModel->headerData(lIt,Qt::Horizontal,
                                                      Qt::UserRole).toString()))
            mLimitsView->GetTableView()->setColumnHidden(lIt, true);
    }
    UpdateSpecificsTableView();
}

int StatisticalMonitoringLimitsWidget::GetDefaultColumnToSort()
{
    return 0;
}

bool StatisticalMonitoringLimitsWidget::LoadDraftTask(
        int versionId,
        CGexMoTaskStatisticalMonitoring* draftTask,
        bool lastIdVersion, QString &outProduct, QString &outLabel)
{
    if (mTaskDraft)
    {
        delete mTaskDraft;
        mTaskDraft = 0;
    }

    mTaskDraft = mTask = draftTask->Clone();
    mTask->LoadTaskDetails(versionId);

    outProduct = mTask->GetAttribute(C_VERSION_PRODUCTS).toString().split("|").join("\n");
    outLabel = mTask->GetAttribute(C_VERSION_LABEL).toString();

    return LoadLimitsFromDatabasePrivate(true, lastIdVersion);
}

TableModel *StatisticalMonitoringLimitsWidget::GetModel() const
{
    return mModel;
}

TableView *StatisticalMonitoringLimitsWidget::GetTableView() const
{
    return mLimitsView->GetTableView();
}

void StatisticalMonitoringLimitsWidget::SetReadOnly(bool readOnly)
{
    mReadOnly = readOnly;
}

bool StatisticalMonitoringLimitsWidget::LoadLimitsFromDatabasePrivate(
        bool readOnly /* =false */,
        bool lastProdVersion/* =false */)
{
    if (!LoadLimitsModel(readOnly, lastProdVersion))
        return false;

    UpdateLimitsTableView();

    return true;
}

bool StatisticalMonitoringLimitsWidget::DumpFields()
{
    if(mTask && mComment)
    {
        mTask->UpdateAttribute(C_VERSION_LABEL, mComment->toPlainText());
    }
    return true;
}

void StatisticalMonitoringLimitsWidget::OnCopyToClipboardRequested()
{
    QString textData;
    QAbstractItemModel* model = mLimitsView->GetTableView()->model();
    int rows = model->rowCount();
    int columns = model->columnCount();

    for(int j = 0; j < columns; j++)
    {
        textData += model->headerData(j, Qt::Horizontal).toString();
        textData += ", ";
    }
    textData += "\n";

    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < columns; j++)
        {
                textData += model->data(model->index(i,j)).toString();
                textData += ", ";
        }
        textData += "\n";
    }

    QClipboard *lClipBoard = QGuiApplication::clipboard();
    if (!lClipBoard)
    {
        GSLOG(SYSLOG_SEV_WARNING, "failed to retrieve application clipboard");
        return;
    }
    lClipBoard->setText(textData, QClipboard::Clipboard);
}

void StatisticalMonitoringLimitsWidget::OnSaveToCsvRequested()
{
    QString textData;
    QAbstractItemModel* model = mLimitsView->GetTableView()->model();
    int rows = model->rowCount();
    int columns = model->columnCount();

    for(int j = 0; j < columns; j++)
    {
        textData += model->headerData(j, Qt::Horizontal).toString();
        textData += ", ";
    }
    textData += "\n";

    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < columns; j++)
        {
                textData += model->data(model->index(i,j)).toString();
                textData += ", ";
        }
        textData += "\n";
    }

    QString strFile = QFileDialog::getSaveFileName(this,
        "Save csv results to...",
        QDir::homePath(),    //strFile,
        "csv (*.csv)",
        NULL, 0 //QFileDialog::DontConfirmOverwrite
        );
    GSLOG(SYSLOG_SEV_NOTICE, QString("saving csv to %1...").arg( strFile).toLatin1().constData());
    if (strFile.isEmpty())
        return;

    QFile			clCsvFile(strFile);
    if(clCsvFile.open(QIODevice::WriteOnly))
    {
        QTextStream	streamCsvFile(&clCsvFile);
        //streamCsvFile.setString(&mComputeResults);
        streamCsvFile << textData;
        streamCsvFile.flush();
        clCsvFile.close();
    }
    else
       GSLOG(SYSLOG_SEV_ERROR, QString("cant open '%1'").arg(strFile).toLatin1().data() );
}

void StatisticalMonitoringLimitsWidget::OnUpdateLimitRequested()
{
    if (mTask)
    {
        DumpFields();
        emit sUpdateClicked();
    }
}

void StatisticalMonitoringLimitsWidget::OnValidateVersionRequested()
{
    if (mTask)
    {
        DumpFields();

        if (mTask->ValidateCurrentDraft())
        {
            GS::Gex::Message::information("Version validated",
                                          "Version validated with the expiration date set to\n" +
                                          mTask->GetAttribute(C_VERSION_EXPIRATIONDATE).
                                          toDate().toString(Qt::ISODate) + ".");
            LoadLimitsFromDatabase();
            emit sValidationSucceeded();
        }
        else
        {
            QString lErrorMsg;
            Engine::GetInstance().GetAdminEngine().GetLastError(lErrorMsg);
            GS::Gex::Message::critical("Error", "Error while validating version:\n" + lErrorMsg);
        }
    }
}

void StatisticalMonitoringLimitsWidget::OnModelUpdated(
        QModelIndex topLeft,
        QModelIndex bottomRight,
        QVector<int> roles)
{
    UpdateModel(topLeft, bottomRight, roles);
}

bool StatisticalMonitoringLimitsWidget::LoadLimitsFromDatabase()
{
    setCursor(Qt::WaitCursor);

    // Disconnect model before update
    disconnect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
               this, SLOT(OnModelUpdated(QModelIndex,QModelIndex,QVector<int>)));

    if (!LoadLimitsFromDatabasePrivate())
    {
        setCursor(Qt::ArrowCursor);
        return false;
    }

    // Reconnect model after update
    connect(mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(OnModelUpdated(QModelIndex,QModelIndex,QVector<int>)));


    LimitsModel* lModel = dynamic_cast<LimitsModel*>(mModel);
    bool lEnableButtons = lModel &&
            (lModel->rowCount() > 0) &&
            !lModel->hasNonManualLimitsToUpdate();
    RefreshUI(lEnableButtons);
    setCursor(Qt::ArrowCursor);

    return true;
}

void StatisticalMonitoringLimitsWidget::InitContextMenu(const QMap<QString, QString> availableAlgos)
{
    mDisableAlarmAction = new QAction("disabled", this);
    mEnableAlarmAction = new QAction("enabled", this);
    mDisableHighLimitAction = new QAction("disabled", this);
    mEnableHighLimitAction = new QAction("enabled", this);
    mChangeHighLimitValueAction = new QAction("value", this);
    mDisableLowLimitAction = new QAction("disabled", this);
    mEnableLowLimitAction = new QAction("enabled", this);
    mChangeLowLimitValueAction = new QAction("value", this);
    foreach(QString algoValue, availableAlgos.keys())
    {
        mChangeAlgoActions << qMakePair(algoValue,
                                        new QAction(availableAlgos.value(algoValue), this));
    }
    mChangeParamAction = new QAction("change parameter value", this);

    mContextMenu = new QMenu(this);
    // Alarm status section
    QMenu* lSubMenu = mContextMenu->addMenu("change alarm status");
    lSubMenu->addAction(mDisableAlarmAction);
    lSubMenu->addAction(mEnableAlarmAction);
    connect(lSubMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(OnChangeAlarmStatusRequested(QAction*)),Qt::UniqueConnection);
    // high limit section
    lSubMenu = mContextMenu->addMenu("change high limit");
    lSubMenu->addAction(mDisableHighLimitAction);
    lSubMenu->addAction(mEnableHighLimitAction);
    lSubMenu->addAction(mChangeHighLimitValueAction);
    connect(lSubMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(OnChangeLimitRequested(QAction*)),Qt::UniqueConnection);
    // low limit section
    lSubMenu = mContextMenu->addMenu("change low limit");
    lSubMenu->addAction(mDisableLowLimitAction);
    lSubMenu->addAction(mEnableLowLimitAction);
    lSubMenu->addAction(mChangeLowLimitValueAction);
    connect(lSubMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(OnChangeLimitRequested(QAction*)),Qt::UniqueConnection);
    // Algorithm section
    lSubMenu = mContextMenu->addMenu("change algorithm");
    for (int lIt = 0; lIt < mChangeAlgoActions.size(); ++lIt)
    {
        lSubMenu->addAction(mChangeAlgoActions.at(lIt).second);
    }
    lSubMenu->addAction(mChangeParamAction);
    connect(lSubMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(OnChangeAlgoRequested(QAction*)),Qt::UniqueConnection);
}

void StatisticalMonitoringLimitsWidget::OnCustomMenuRequested(QPoint pos)
{
    if (mContextMenu)
        mContextMenu->popup(mLimitsView->GetTableView()->viewport()->mapToGlobal(pos));
}

void StatisticalMonitoringLimitsWidget::OnChangeAlarmStatusRequested(QAction* action)
{
    int lNewStatus;
    if (action == mDisableAlarmAction)
        lNewStatus = 0;
    else if (action == mEnableAlarmAction)
        lNewStatus = 1;

    QModelIndexList lSelectedIndexes = mLimitsView->GetTableView()->selectionModel()->selection().indexes();
    QSortFilterProxyModel* lModel = static_cast<QSortFilterProxyModel*>(mLimitsView->GetTableView()->model());
    for(int lIt = 0; lIt < lSelectedIndexes.size(); ++lIt)
    {
        QModelIndex lCurrentIndex = lSelectedIndexes.at(lIt);
        lModel->setData(lModel->index(lCurrentIndex.row(),
                SM_STAT_ENABLED_COL), lNewStatus, Qt::EditRole);
    }
}

void StatisticalMonitoringLimitsWidget::OnChangeLimitRequested(QAction *action)
{
    QModelIndexList lSelectedIndexes = mLimitsView->GetTableView()->selectionModel()->selection().indexes();
    QSortFilterProxyModel* lModel = static_cast<QSortFilterProxyModel*>(mLimitsView->GetTableView()->model());
    if ((action == mChangeHighLimitValueAction) || ((action == mChangeLowLimitValueAction)))
    {
        QInputDialog lInputDialog;
        lInputDialog.setInputMode(QInputDialog::TextInput);
        lInputDialog.setWindowTitle("Change limit");
        lInputDialog.setLabelText("Set new value");
        lInputDialog.adjustSize();
        lInputDialog.move(QCursor::pos());

        if (lInputDialog.exec() == QDialog::Rejected)
            return;

        QString lNewLimit = lInputDialog.textValue();
        for(int lIt = 0; lIt < lSelectedIndexes.size(); ++lIt)
        {
            QModelIndex lCurrentIndex = lSelectedIndexes.at(lIt);
            if (action == mChangeHighLimitValueAction)
            {
                if (lNewLimit.toDouble() >= lModel->
                    data(lModel->index(lCurrentIndex.row(), SM_LL_COL)).
                    toDouble())
                {
                    lModel->
                        setData(lModel->index(lCurrentIndex.row(), SM_HL_COL),
                                lNewLimit, Qt::EditRole);
                }
            }
            else
            {
                if (lNewLimit.toDouble() <= lModel->
                    data(lModel->index(lCurrentIndex.row(), SM_HL_COL)).
                    toDouble())
                {
                    lModel->
                        setData(lModel->index(lCurrentIndex.row(), SM_LL_COL),
                                lNewLimit, Qt::EditRole);
                }
            }
        }
    }
    else
    {
        int lColIndex = -1;
        int lValue = -1;
        if (action == mDisableHighLimitAction)
        {
            lColIndex = SM_HL_ENABLED_COL;
            lValue = 0;
        }
        else if (action == mEnableHighLimitAction)
        {
            lColIndex = SM_HL_ENABLED_COL;
            lValue = 1;
        }
        if (action == mDisableLowLimitAction)
        {
            lColIndex = SM_LL_ENABLED_COL;
            lValue = 0;
        }
        else if (action == mEnableLowLimitAction)
        {
            lColIndex = SM_LL_ENABLED_COL;
            lValue = 1;
        }
        for(int lIt = 0; lIt < lSelectedIndexes.size(); ++lIt)
        {
            QModelIndex lCurrentIndex = lSelectedIndexes.at(lIt);
            lModel->setData(lModel->index(lCurrentIndex.row(),
                lColIndex), lValue, Qt::EditRole);
        }
    }
}

void StatisticalMonitoringLimitsWidget::OnChangeAlgoRequested(QAction *action)
{
    QString lAlgo = "";
    for (int lIt = 0; lIt < mChangeAlgoActions.size() && lAlgo.isEmpty(); ++lIt)
    {
        if (mChangeAlgoActions.at(lIt).second == action)
        {
            lAlgo = mChangeAlgoActions.at(lIt).first;
        }
    }

    QModelIndexList lSelectedIndexes = mLimitsView->GetTableView()->selectionModel()->selection().indexes();
    QSortFilterProxyModel* lModel = static_cast<QSortFilterProxyModel*>(mLimitsView->GetTableView()->model());
    if (lAlgo.isEmpty())
    {
        QInputDialog lInputDialog;
        lInputDialog.setInputMode(QInputDialog::TextInput);
        lInputDialog.setWindowTitle("Change N");
        lInputDialog.setLabelText("Set new value");
        lInputDialog.adjustSize();
        lInputDialog.move(QCursor::pos());

        if (lInputDialog.exec() == QDialog::Rejected)
            return;

        QString lNewParam = lInputDialog.textValue();
        for(int lIt = 0; lIt < lSelectedIndexes.size(); ++lIt)
        {
            QModelIndex lCurrentIndex = lSelectedIndexes.at(lIt);
            lModel->setData(lModel->index(lCurrentIndex.row(),
                SM_N_COL), lNewParam, Qt::EditRole);
        }
    }
    else
    {
        for(int lIt = 0; lIt < lSelectedIndexes.size(); ++lIt)
        {
            QModelIndex lCurrentIndex = lSelectedIndexes.at(lIt);
            lModel->setData(lModel->index(lCurrentIndex.row(),
                SM_ALGO_COL), lAlgo, Qt::EditRole);
        }
    }
}


void StatisticalMonitoringLimitsWidget::FillTableModel(QSqlQuery& query, bool readOnly /* = false*/)
{
    int lRowId = 0;
    mModel->setRowCount(query.size());

    while(query.next())
    {
        for (int lColIt = 0; lColIt < mModel->columnCount(); ++ lColIt)
        {
            QModelIndex lModelIndex = mModel->index(lRowId, lColIt);
            QString lFieldName = LimitsModel::rawColumnName(lColIt);


            if (((lFieldName == SM_HL) || (lFieldName == SM_LL)) &&
                query.value(lFieldName).isNull())

            {
                mModel->setData(lModelIndex,
                                QVariant(GEX_C_DOUBLE_NAN), Qt::EditRole); // set NAN
            }
            else
            {
                mModel->setData(lModelIndex,
                                QVariant(query.value(lFieldName)), Qt::EditRole); // set data
            }

            if ((lFieldName == SM_HL_ENABLED) ||
                    (lFieldName == SM_LL_ENABLED) ||
                    (lFieldName == SM_ENABLED))
            {
                mModel->setData(lModelIndex,
                                QVariant(TableItem::COMBOBOX_ENABLED), Qt::DecorationRole);
            }
            else if (lFieldName == SM_ALGO)
            {
                mModel->setData(lModelIndex,
                                QVariant(TableItem::COMBOBOX_ALGO), Qt::DecorationRole);
            }
            else if ((lFieldName ==SM_HL) || (lFieldName == SM_LL))
            {
                mModel->setData(lModelIndex,
                                QVariant(TableItem::STD_LINEEDIT), Qt::DecorationRole);
            }
            else
            {
                if (lFieldName == SM_CRIT_LVL)
                {
                    mModel->setData(lModelIndex,
                                    QVariant(TableItem::LINEEDIT_CRIT), Qt::DecorationRole);
                }
                else
                {
                    mModel->setData(lModelIndex,
                                    QVariant(TableItem::STD_LINEEDIT), Qt::DecorationRole);
                }
                // let param value only be editable
                if (lFieldName !=SM_PARAM_VALUE)
                {
                    mModel->setFlags(lModelIndex,
                                     lModelIndex.flags()&~Qt::ItemIsEditable); // item not editable
                }
            }

            // If read only mode remove EDITABLE flag
            if (readOnly)
            {
                mModel->setFlags(lModelIndex,
                                 lModelIndex.flags()&~Qt::ItemIsEditable);
            }
        }
        ++lRowId;
    }
}

void StatisticalMonitoringLimitsWidget::UpdateModel(QModelIndex topLeft,
                                  QModelIndex bottomRight,
                                  QVector<int> roles)
{
    if ((topLeft != bottomRight) || !topLeft.isValid())
        return;

    QModelIndex lCurrentIndex = topLeft;
    QStringList lModelFields;
    lModelFields << mShownFields << mHiddenFields;

    // Retrieve IDs from model
    QString lTaskId = mModel->data(mModel->index(lCurrentIndex.row(),
                                lModelFields.indexOf(SM_ID))).toString();
    QString lVersionId = mModel->data(mModel->index(lCurrentIndex.row(),
                                lModelFields.indexOf(SM_VERSION_ID))).toString();
    QString lLimitId = mModel->data(mModel->index(lCurrentIndex.row(),
                                lModelFields.indexOf(SM_LIMIT_ID))).toString();
    QString lAlgo = mModel->data(mModel->index(lCurrentIndex.row(),
                                lModelFields.indexOf(SM_ALGO)), Qt::UserRole).toString();

    // GetUpdated field
    QString lUpdatedField = mModel->headerData(lCurrentIndex.column(),
                                Qt::Horizontal, Qt::UserRole).toString().trimmed();
    QString lUpdatedValue = mModel->data(lCurrentIndex, Qt::UserRole).toString().trimmed();
    QList<QPair<QString, QString> > lUpdates;
    QList<QPair<QString, QString> > lFilters;
    lFilters.append(qMakePair(SM_ID, lTaskId));
    lFilters.append(qMakePair(SM_VERSION_ID, lVersionId));
    lFilters.append(qMakePair(SM_LIMIT_ID, lLimitId));

    QString recomputeString;
    int recomputeInt;

    // Update matching fields if needed
    if ((lUpdatedField == SM_ALGO))
    {
        lUpdates.append(qMakePair(lUpdatedField, lUpdatedValue));
        if (lUpdatedValue != C_OUTLIERRULE_MANUAL)
        {
            recomputeString = "1";
            recomputeInt = 1;
        }
        else
        {
            recomputeString = "2";
            recomputeInt = 2;
        }
        lUpdates.append(qMakePair(SM_RECOMPUTE, recomputeString));
        mTask->UpdateStatTable(SM_TABLE_LIMIT, lUpdates, lFilters);

        mModel->setData(mModel->index(lCurrentIndex.row(), SM_STAT_RECOMPUTE_COL), recomputeInt, Qt::EditRole);
    }
    else if (lUpdatedField == SM_PARAM_VALUE)
    {
        lUpdates.append(qMakePair(lUpdatedField, lUpdatedValue.replace(",",".")));
        mTask->UpdateStatTable(SM_TABLE_LIMIT_PARAM, lUpdates, lFilters);
        if (lAlgo != C_OUTLIERRULE_MANUAL)
        {
            recomputeString = "1";
            recomputeInt = 1;
        }
        else
        {
            recomputeString = "2";
            recomputeInt = 2;
        }
        lUpdates.clear();
        lUpdates.append(qMakePair(SM_RECOMPUTE, recomputeString));
        mTask->UpdateStatTable(SM_TABLE_LIMIT, lUpdates, lFilters);

        mModel->setData(mModel->index(lCurrentIndex.row(), SM_STAT_RECOMPUTE_COL), recomputeInt, Qt::EditRole);
    }
    else if ((lUpdatedField == SM_HL) || (lUpdatedField == SM_LL))
    {
        lUpdates.append(qMakePair(lUpdatedField, lUpdatedValue.replace(",",".")));
        mTask->UpdateStatTable(SM_TABLE_LIMIT, lUpdates, lFilters);

        mModel->setData(mModel->index(lCurrentIndex.row(), SM_ALGO_COL), C_OUTLIERRULE_MANUAL, Qt::EditRole);
    }
    else if ((lUpdatedField == SM_LL_ENABLED) || (lUpdatedField == SM_HL_ENABLED))
    {
        lUpdates.append(qMakePair(lUpdatedField, lUpdatedValue));
        lUpdates.append(qMakePair(SM_RECOMPUTE, QString("2")));
        mTask->UpdateStatTable(SM_TABLE_LIMIT, lUpdates, lFilters);

        mModel->setData(mModel->index(lCurrentIndex.row(), SM_STAT_RECOMPUTE_COL), 2, Qt::EditRole);
    }
    else if (lUpdatedField == SM_ENABLED)
    {
        lUpdates.append(qMakePair(lUpdatedField, lUpdatedValue));
        mTask->UpdateStatTable(SM_TABLE_LIMIT, lUpdates, lFilters);
    }
    else if (lUpdatedField == SM_RECOMPUTE)
    {
        // just ignore if it's RECOMPTE as it has been updated in another query with the concerned field
    }
    else
    {
        Engine::GetInstance().GetAdminEngine().
                SetLastError(AdminEngine::eDB_NoFields,
                             QString("Field not updatable:" + lUpdatedField));
    }

    LimitsModel* lModel = dynamic_cast<LimitsModel*>(mModel);
    bool lEnableButtons = lModel &&
            (lModel->rowCount() > 0) &&
            !lModel->hasNonManualLimitsToUpdate();

    RefreshUI(lEnableButtons);
}

} // NAMESPACE Gex
} // NAMESPACE GS
