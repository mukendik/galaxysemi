
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>

#include "statistical_monitoring_alarms_widget.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_task.h"
#include "common_widgets/filterable_table_view.h"
#include "product_info.h"
#include "common_widgets/table_model.h"
#include "common_widgets/table_view.h"
#include "gqtl_log.h"
#include "gex_shared.h"
#include "alarms_model.h"



namespace GS
{
namespace Gex
{

StatisticalMonitoringAlarmsWidget::StatisticalMonitoringAlarmsWidget(
        CGexMoTaskStatisticalMonitoring *task,
        const QString &backupFile,
        QWidget *parent):
  mTask(task),
  mAlarmsView(new FilterableTableView(backupFile, parent)),
  mModel(0),
  QWidget(parent)
{
    // Build ordered list of shown fields
    for (int lIt = 0; lIt < SM_ALARMS_MODEL_NUM_SHOWN_COL; ++lIt)
    {
        mShownFields << AlarmsModel::rawColumnName(lIt);
    }
    // Build ordered list of hidden fields
    for (int lIt = SM_ALARMS_MODEL_NUM_SHOWN_COL;
         lIt < SM_ALARMS_MODEL_NUM_TOT_COL ; ++lIt)
    {
        mHiddenFields << AlarmsModel::rawColumnName(lIt);
    }
    // Build list of pairs table names/decorated tables names
    QList<QPair<QString, QString > > lDecoratedFields;
    for (int lIt = 0;lIt < SM_ALARMS_MODEL_NUM_TOT_COL; ++lIt )
    {
        lDecoratedFields.append(qMakePair(AlarmsModel::rawColumnName(lIt),
                                          AlarmsModel::decoractedColumnName(lIt)));
    }
    // Instanciate model
    mModel = new AlarmsModel(lDecoratedFields);
}

StatisticalMonitoringAlarmsWidget::~StatisticalMonitoringAlarmsWidget()
{
    // do not delete mAlarmsView already done by parent widget
    delete mModel;
}

void StatisticalMonitoringAlarmsWidget::RefreshUI(bool enableUpdateButtons)
{
    mAlarmsView->RefreshView();
    mRunSimulate->setEnabled(enableUpdateButtons);
}

QWidget* StatisticalMonitoringAlarmsWidget::CreateTableView()
{
    QMap<int , QPair<QString, QString> > columnFiltered;
    for (int lIt = 0; lIt < mShownFields.size(); ++lIt)
    {
        columnFiltered.insert(lIt, qMakePair(AlarmsModel::decoractedColumnName(lIt),
                              AlarmsModel::rawColumnName(lIt)));
    }
    return mAlarmsView->CreateView(columnFiltered, GetDefaultColumnToSort());
}

void StatisticalMonitoringAlarmsWidget::CreateDataUI(QLayout *target)
{
    target->setContentsMargins(0,0,5,0);
    QFrame* lSimulateControls = new QFrame();
    lSimulateControls->setLayout(new QHBoxLayout());
    QLabel* lLabel = new QLabel("Select date range for simulation");
    lSimulateControls->layout()->addWidget(lLabel);
    mDateSimulateFrom = new QDateEdit(QDate::currentDate().addDays(-180));
    mDateSimulateFrom->setCalendarPopup(true);
    mDateSimulateFrom->setDisplayFormat("yyyy-MM-dd");
    mDateSimulateFrom->setToolTip("ISODate");
    lSimulateControls->layout()->addWidget(mDateSimulateFrom);
    lLabel = new QLabel("to");
    lSimulateControls->layout()->addWidget(lLabel);
    mDateSimulateTo = new QDateEdit(QDate::currentDate());
    mDateSimulateTo->setCalendarPopup(true);
    mDateSimulateTo->setDisplayFormat("yyyy-MM-dd");
    mDateSimulateTo->setToolTip("ISODate");
    lSimulateControls->layout()->addWidget(mDateSimulateTo);

    static_cast<QHBoxLayout*>(lSimulateControls->layout())->addSpacerItem(
                new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    mRunSimulate = new QPushButton("run simulation");
    lSimulateControls->layout()->addWidget(mRunSimulate);
    lSimulateControls->layout()->setContentsMargins(0, 0, 0, 0);
    target->addWidget(lSimulateControls);

    QWidget* lTableView = CreateTableView();
    if(lTableView)
    {
        target->addWidget(lTableView);
    }

    LoadAlarmsModel(QList<StatMonAlarm>());
    UpdateLimitsTableView();

    connect(mRunSimulate, SIGNAL(clicked()), this, SLOT(Simulate()));
}

void StatisticalMonitoringAlarmsWidget::CreateSummaryWidget(QLayout *target)
{
    mSimulateSummary = new QTextEdit();
    mSimulateSummary->setReadOnly(true);
    mSimulateSummary->setFrameStyle(QFrame::NoFrame);
    target->addWidget(mSimulateSummary);
}

void StatisticalMonitoringAlarmsWidget::Simulate()
{
    emit SimulateClicked(mDateSimulateFrom->dateTime(), mDateSimulateTo->dateTime());
}

void StatisticalMonitoringAlarmsWidget::UpdateSimulateResult(QMap<QString, QVariant> logSummary, QList<StatMonAlarm> alarms)
{
    QMapIterator<QString, QVariant> lIt(logSummary);
    mSimulateSummary->clear();
    QMap<int, QString> lEditContent;
    while (lIt.hasNext())
    {
        lIt.next();
        QString lKey = lIt.key();
        int lPosition = -1;
        QString lLine = "<b>";
        if (lKey == "status")
        {
            lLine += "status";
            lPosition = 0;
        }
        else if (lKey == "summary")
        {
            lLine += "summary";
            lPosition = 1;
        }
        else if (lKey == "matched_products")
        {
            lLine += "matched products";
            lPosition = 2;
        }
        else if (lKey == "matched_lots")
        {
            lLine += "matched lots";
            lPosition = 3;
        }
        else if (lKey == "matched_sublots")
        {
            lLine += "matched sublots";
            lPosition = 4;
        }
        else if (lKey == "matched_wafers")
        {
            lLine += "matched wafers";
            lPosition = 5;
        }
        else if (lKey == "nb_parts")
        {
            lLine += "parts";
            lPosition = 6;
        }
        else if (lKey == "nb_critical_alarms")
        {
            lLine += "wafers/sublots with critical alarm";
            lPosition = 7;
        }
        else if (lKey == "nb_standard_alarms")
        {
            lLine += "wafers/sublots with standard alarm";
            lPosition = 8;
        }
        else if(lKey == "threshold")
        {
            lLine += "threshold activated";
            lPosition = 9;
        }
        else
            continue;

        if (lIt.value().toString().contains("|") && (lKey != "matched_sublots") &&  (lKey != "matched_wafers"))
        {
            lLine.append(" (" + QString::number(lIt.value().toString().split("|").size()) + ")");
            lLine.append(":</b> " + lIt.value().toString());
        }
        else if (lKey == "matched_sublots" || lKey == "matched_wafers")
        {
            lLine.append(":</b> " + QString::number(lIt.value().toString().split(")|(").size()));
        }
        else if( lKey == "threshold" )
            lLine.append(":</b> " + lIt.value().toString() + "%");
        else
            lLine.append(":</b> " + lIt.value().toString());

        lEditContent.insert(lPosition, lLine);
    }

    QMap<int, QString>::iterator lIterBegin(lEditContent.begin()), lIterEnd(lEditContent.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin)
    {
        mSimulateSummary->append(lIterBegin.value());
    }

    LoadAlarmsModel(alarms);

    UpdateLimitsTableView();
}

bool StatisticalMonitoringAlarmsWidget::LoadAlarmsModel(const QList<StatMonAlarm>& lAlarms)
{
    mModel->clear();

    if (lAlarms.isEmpty())
        return false;

    for (int lRowIt = 0; lRowIt < lAlarms.size(); ++lRowIt)
    {
        mModel->insertRow(lRowIt);
        for (int lColIt = 0; lColIt < mShownFields.size(); ++lColIt)
        {
            mModel->setData(mModel->index(lRowIt, lColIt),
                AlarmsModel::GetValue(lAlarms.at(lRowIt), mShownFields.at(lColIt)));
        }
    }

    return true;
}

void StatisticalMonitoringAlarmsWidget::UpdateLimitsTableView()
{
    if (mAlarmsView &&
            !mAlarmsView->GetTableView()->model())
    {
        mAlarmsView->SetModel(mModel, GetDefaultColumnToSort());
    }

    // Hide IDs columns
    for (int lIt = 0; lIt < mModel->columnCount(); ++lIt)
    {
        if (mHiddenFields.contains(mModel->headerData(lIt,Qt::Horizontal,
                                                      Qt::UserRole).toString()))
            mAlarmsView->GetTableView()->setColumnHidden(lIt, true);
    }
}

int StatisticalMonitoringAlarmsWidget::GetDefaultColumnToSort() const
{
    return SM_ALARM_PRODUCT_COL;
}

} // namespace Gex
} // namespace GS
