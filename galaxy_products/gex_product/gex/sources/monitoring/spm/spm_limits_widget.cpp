
#include <QComboBox>

#include "statistical_monitoring_tables.h"
#include "common_widgets/filterable_table_view.h"
#include "spm_limits_widget.h"
#include "limits_delegate.h"
#include "gex_constants.h"
#include "limits_model.h"
#include "admin_engine.h"
#include "spm_engine.h"
#include "common_widgets/table_view.h"
#include "spm_task.h"
#include "engine.h"

namespace GS
{
namespace Gex
{

SPMLimitsWidget::SPMLimitsWidget(CGexMoTaskStatisticalMonitoring *spmTask,
                                 const QString &backupFile,
                                 QWidget *parent) :
    StatisticalMonitoringLimitsWidget(spmTask, backupFile, parent)
{
    // Build ordered list of shown fields
    for (int lIt = 0; lIt < SM_LIMITS_MODEL_NUM_SHOWN_COL; ++lIt)
    {
            mShownFields << LimitsModel::rawColumnName(lIt);
    }
    // Build ordered list of hidden fields
    for (int lIt = SM_LIMITS_MODEL_NUM_SHOWN_COL;
         lIt < SM_LIMITS_MODEL_NUM_TOT_COL; ++lIt)
    {
        mHiddenFields << LimitsModel::rawColumnName(lIt);
    }
    // Build list of pairs table names/decorated tables names
    QList<QPair<QString, QString > > lDecoratedFields;
    for (int lIt = 0;lIt < SM_LIMITS_MODEL_NUM_TOT_COL; ++lIt )
    {
        lDecoratedFields.append(qMakePair(LimitsModel::rawColumnName(lIt),
                                          LimitsModel::decoractedColumnName(lIt)));
    }
    // Instanciate model
    mModel = new LimitsModel(Engine::GetInstance().GetSPMEngine().GetSupportedStats(),
                             lDecoratedFields,
                             parent);
}

SPMLimitsWidget::~SPMLimitsWidget()
{
    if (mModel)
    {
        delete mModel;
        mModel = 0;
    }
}

QWidget *SPMLimitsWidget::CreateTableView()
{
    QMap<int , QPair<QString, QString> > columnFiltered;
    for (int lIt = 0; lIt < mShownFields.size(); ++lIt)
    {
        columnFiltered.insert(lIt, qMakePair(LimitsModel::decoractedColumnName(lIt),
                              LimitsModel::rawColumnName(lIt)));
    }
    return mLimitsView->CreateView(
                        columnFiltered, GetDefaultColumnToSort());
}

int SPMLimitsWidget::GetDefaultColumnToSort() const
{
    return SM_ITEM_NUM_COL;
}

QMap<QString, QString> SPMLimitsWidget::AddItemDelegateOutlierAlgo()
{
    TableView* lTableView = mLimitsView->GetTableView();
    LimitsDelegate* lItemDelegate = new LimitsDelegate(lTableView);
    QMap<QString, QString> availableAlgos = Engine::GetInstance().
            GetSPMEngine().GetSupportedOutlierAlgorithms();
    lItemDelegate->setAlgoValues(availableAlgos.keys());
    lItemDelegate->setAlgoNames(availableAlgos.values());
    lTableView->setItemDelegate(lItemDelegate);

    return availableAlgos;
}

void SPMLimitsWidget::UpdateSpecificsTableView()
{
    if (mLimitsView && mLimitsView->GetTableView())
    {
        mLimitsView->GetTableView()->setColumnHidden(SM_ITEM_CAT_COL, true);
    }
}


} // namespace Gex
} // namespace GS
