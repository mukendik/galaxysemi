#include "ui_statistical_monitoring_version_widget.h"
#include <QSqlTableModel>
//#include <QDate>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include "common_widgets/table_view.h"
#include "info_widget.h"

#include "statistical_monitoring_task.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_version_widget.h"


namespace GS
{
namespace Gex
{


StatisticalMonitoringVersionWidget::StatisticalMonitoringVersionWidget(CGexMoTaskStatisticalMonitoring *task, const QString& backupFile, QWidget* parent):
    QWidget(parent),
    mUI( new Ui::StatisticalMonitoringVersionWidget),
    mTask(task),
    mTableView(new TableView(0)),
    mModel(0),
    mProxyModel(0),
    mStateLoaded(false),
    mIsUiInitialized(false),
    mSelectedId(-1),
    mInfoWidget(0)
{
    mUI->setupUi(this);

    //-- Fill The comboxBox
    FillComboBoxFilter();
    connect(mUI->pushButtonRefresh,        SIGNAL(clicked(bool)),          this, SLOT(OnRefresh()));
    connect(mUI->dateEditFrom,             SIGNAL(dateChanged(QDate)),     this, SLOT(OnDateFrom(QDate)));

    connect(mUI->pushButtonImport, SIGNAL(clicked(bool)), this, SLOT(ViewVersion()));
}

StatisticalMonitoringVersionWidget::~StatisticalMonitoringVersionWidget()
{
    delete mUI;
    delete mModel;
    delete mTableView;
}

//! ----------------------------------- !//
//!         GetIDFromSelectedRow        !//
//! ----------------------------------- !//
int StatisticalMonitoringVersionWidget::GetIDFromSelectedRow()
{
    QModelIndexList lIndexes = mTableView->selectionModel()->selectedRows();
    if(lIndexes.isEmpty() == false)
    {
        QModelIndex lIndex = mProxyModel->index(lIndexes[0].row(), 1);
        if(lIndex.isValid())
        {
            return lIndex.data(Qt::DisplayRole).toInt();
        }
    }
    return -1;
}

//! ----------------------------------- !//
//!                InitUI               !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::InitUI(QSqlTableModel* model, bool forceReload)
{

    if( mIsUiInitialized == false || forceReload == true)
    {
        delete mModel;
        delete mProxyModel;

        mModel = model;
        mModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
        mModel->setSort(1, Qt::DescendingOrder);
        mModel->select();

        mUI->frame->setLayout(new QVBoxLayout());
        mUI->frame->layout()->addWidget(mTableView);
        mUI->frame->layout()->setContentsMargins(0,0,0,0);
        mTableView->InitializeUI();
        mTableView->verticalHeader()->setVisible(false);
        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::SingleSelection);
        mProxyModel = new QSortFilterProxyModel(this);
        mProxyModel->setSourceModel(mModel);
        mTableView->setModel(mProxyModel);


        // -- hide the id  and draft version since all row have the same regarding the query
        mTableView->hideColumn(0);
        mTableView->hideColumn(2);
        mTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        UpdateWidgetInfo(0);

        connect(mTableView->selectionModel(),
                SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(OnSelectedRow(const QModelIndex&, const QModelIndex&)), Qt::UniqueConnection);


        connect( mUI->comboBoxColumToFilter, SIGNAL(activated(int)), this, SLOT(ChangeFilter(int)), Qt::UniqueConnection);

        QVariant lValue = mTask->GetAttribute(C_VERSION_COMPUTEFROM);
        if(!lValue.isNull())
        {
            mUI->dateEditFrom->setDate(lValue.toDateTime().date());
        }

        lValue = mTask->GetAttribute(C_VERSION_COMPUTETO);
        if(!lValue.isNull())
        {
            mUI->dateEditTo->setDate(lValue.toDateTime().date());
        }

        // -- For this version those button won't be used. Kept until the flow is decided once for all
        mUI->pushButtonDelete->hide();
        mUI->pushButtonImport->hide();

        mIsUiInitialized = true;
        ChangeFilter(mUI->comboBoxColumToFilter->currentIndex());
    }
}

//! ----------------------------------- !//
//!         ChangeFilter                !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::ChangeFilter(int index)
{
    if(index == 0)
    {
        mUI->dateEditFrom->setEnabled(false);
        mUI->dateEditTo->setEnabled(false);
    }
    else
    {
        mUI->dateEditFrom->setEnabled(true);
        mUI->dateEditTo->setEnabled(true);
    }

    OnRefresh();
}

//! ----------------------------------- !//
//!         FillComboBoxFilter          !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::FillComboBoxFilter()
{
    mUI->comboBoxColumToFilter->addItem("No Filter");
    mDicoColumTable[0] = E_None;

    mUI->comboBoxColumToFilter->addItem("Creation");
    mDicoColumTable[mUI->comboBoxColumToFilter->count() -1] = E_Creation;

    mUI->comboBoxColumToFilter->addItem("Start");
    mDicoColumTable[mUI->comboBoxColumToFilter->count() - 1] = E_Start;

    mUI->comboBoxColumToFilter->addItem("Expiration");
    mDicoColumTable[mUI->comboBoxColumToFilter->count() - 1] = E_Expiration;

    mUI->comboBoxColumToFilter->addItem("Computation_from");
    mDicoColumTable[mUI->comboBoxColumToFilter->count() - 1] = E_ComputationFrom;

    mUI->comboBoxColumToFilter->addItem("Computation_to");
    mDicoColumTable[mUI->comboBoxColumToFilter->count() - 1] = E_ComputationTo;
}

//! ----------------------------------- !//
//!             OnFilterDate            !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::OnFilterDate(int index)
{
    if( index > mDicoColumTable.size())
        return;

    E_COLUMN_VERSION lColumFiltered = mDicoColumTable[index];

    QDate lDateFrom = mUI->dateEditFrom->date();
    QDate lDateTo   = mUI->dateEditTo->date();

    int lNbRow = mProxyModel->rowCount();

    for (int i = 0; i< lNbRow; ++i)
    {
        QModelIndex lIndex = mProxyModel->index(i, lColumFiltered);
        if(lIndex.isValid())
        {
            QDate lDate = lIndex.data().toDate();
            if( (lDate < lDateFrom || lDate > lDateTo) )
            {
                mTableView->hideRow(i);
            }
            else
            {
               mTableView->showRow(i);
            }
        }
        else
        {
             mTableView->showRow(i);
        }
    }

}

//! ----------------------------------- !//
//!                OnRefresh            !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::OnRefresh()
{
    OnFilterDate(mUI->comboBoxColumToFilter->currentIndex());
}


//! ----------------------------------- !//
//!             OnDateFrom              !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::OnDateFrom(QDate date)
{
    mUI->dateEditTo->setDate(date);
}

//! ----------------------------------- !//
//!             OnSelectedRow           !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::OnSelectedRow(const QModelIndex& selectedIndex, const QModelIndex& )
{
    if(selectedIndex.isValid())
    {
        //-- the index selected does not correspond necesseraly to the ipm field
        //-- but we can retrieve the row. The column of the ipm field is known
        int lRowSelected = selectedIndex.row();

        QModelIndex lIndex = mProxyModel->index(lRowSelected, 1);
        if(lIndex.isValid())
        {
             int lId = mProxyModel->data(lIndex).toInt();
             if(lId== mTask->GetActiveProdVersionId())
             {
                   mUI->pushButtonDelete->setEnabled(false);
             }
             else
             {
                 mUI->pushButtonDelete->setEnabled(true);
             }
             mUI->pushButtonImport->setEnabled(true);


             //-- update the information in the right banner
            UpdateWidgetInfo(lRowSelected);

            //--Load the limit in the preview
            emit OnVersionId(lId);
        }
    }
}

//! ----------------------------------- !//
//!             OnSelectedRow           !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::UpdateWidgetInfo(int rowSelected)
{
    if(mInfoWidget)
    {
        QModelIndex lIndex = mProxyModel->index(rowSelected, 1);
        if(lIndex.isValid())
        {
            mSelectedId = mProxyModel->data(lIndex).toInt();

            QString lLabelId = QString::number(mSelectedId);
            if(mSelectedId == mTask->GetActiveProdVersionId())
            {
                lLabelId +=" (Active)";
                mInfoWidget->ProductionVersion(true);
            }
            else
            {
                mInfoWidget->ProductionVersion(false);
            }
            mInfoWidget->UpdateIdLabel(lLabelId);
        }
        else
        {
            return;
        }

        //-- Update Creation Date
        lIndex = mProxyModel->index(rowSelected, 6);
        if(lIndex.isValid())
        {
            mInfoWidget->UpdateCreationLabel(mProxyModel->data(lIndex).toDateTime().toString(Qt::ISODate));
        }

        //-- Update Expiration Label
        lIndex = mProxyModel->index(rowSelected, 8);
        if(lIndex.isValid())
        {
            mInfoWidget->UpdateExpirationLabel(mProxyModel->data(lIndex).toDate().toString(Qt::ISODate));
        }

        //-- Update Computation Label from
        lIndex = mProxyModel->index(rowSelected, 11);
        if(lIndex.isValid())
        {
            QString lLabel = mProxyModel->data(lIndex).toDate().toString(Qt::ISODate);

            //-- Update Computation Label to
            lIndex = mProxyModel->index(rowSelected, 12);
            if(lIndex.isValid())
            {
                lLabel +=" to " + mProxyModel->data(lIndex).toDate().toString(Qt::ISODate);
            }

            mInfoWidget->UpdateComputationLabel(lLabel);
        }
    }
}

//! ------------------------------- !//
//!             SetInfoWidget       !//
//! ------------------------------- !//
void StatisticalMonitoringVersionWidget::SetInfoWidget(InfoWidget *infoWidget)
{
    mInfoWidget = infoWidget;
}

//! ---------------------------------- !//
//!             SelectedId             !//
//! ---------------------------------- !//
int StatisticalMonitoringVersionWidget::SelectedId() const
{
    return mSelectedId;
}

//! ----------------------------------- !//
//!             ViewVersion             !//
//! ----------------------------------- !//
void StatisticalMonitoringVersionWidget::ViewVersion()
{
    int lId = GetIDFromSelectedRow();
    if(lId == -1)
        return;

    emit OnVersionId(lId);
}

}
}
