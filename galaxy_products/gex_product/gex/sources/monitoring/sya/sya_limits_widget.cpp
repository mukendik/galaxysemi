
#include <QComboBox>


#include "statistical_monitoring_tables.h"
#include "common_widgets/table_view.h"
#include "admin_engine.h"
#include "limits_delegate.h"
#include "gex_constants.h"
#include "sya_task.h"
#include "limits_model.h"
#include "sya_engine.h"
#include "sya_limits_widget.h"
#include "common_widgets/filterable_table_view.h"
#include "engine.h"

namespace GS
{
namespace Gex
{

SYALimitsWidget::SYALimitsWidget(CGexMoTaskStatisticalMonitoring *syaTask,
                                 const QString backupFile,
                                 QWidget *parent) :
    StatisticalMonitoringLimitsWidget(syaTask, backupFile, parent)
{
    // Build ordered list of shown fields
    for(int lIt = 0; lIt < SM_LIMITS_MODEL_NUM_SHOWN_COL; ++lIt)
    {
        mShownFields << LimitsModel::rawColumnName(lIt);
    }
    // Build ordered list of hidden fields
    for(int lIt = SM_LIMITS_MODEL_NUM_SHOWN_COL; lIt < SM_LIMITS_MODEL_NUM_TOT_COL; ++lIt)
    {
        mHiddenFields << LimitsModel::rawColumnName(lIt);
    }
    // Build list of pairs table names/decorated tables names
    QList<QPair<QString, QString > > lDecoratedFields;
    for(int lIt = 0;lIt < SM_LIMITS_MODEL_NUM_TOT_COL; ++lIt )
    {
        lDecoratedFields.append(qMakePair(LimitsModel::rawColumnName(lIt),
                                          LimitsModel::decoractedColumnName(lIt)));
    }
    // Instanciate model
    mModel = new LimitsModel(Engine::GetInstance().GetSYAEngine().GetSupportedStats(),
                                lDecoratedFields, parent);
}

SYALimitsWidget::~SYALimitsWidget()
{
    delete mModel;
}

int SYALimitsWidget::GetDefaultColumnToSort() const
{
    return SM_ITEM_NUM_COL;
}

QWidget *SYALimitsWidget::CreateTableView()
{
    QMap<int , QPair<QString, QString> > lColumnFiltered;
    for (int lIt = 0; lIt < mShownFields.size(); ++lIt)
    {
        lColumnFiltered.insert(lIt, qMakePair(LimitsModel::decoractedColumnName(lIt),
                              LimitsModel::rawColumnName(lIt)));
    }
    return mLimitsView->CreateView(lColumnFiltered, GetDefaultColumnToSort());
}

QMap<QString, QString> SYALimitsWidget::AddItemDelegateOutlierAlgo()
{
    TableView* lTableView = mLimitsView->GetTableView();
    LimitsDelegate* lItemDelegate = new LimitsDelegate(lTableView);
    QMap<QString, QString> lAvailableAlgos = Engine::GetInstance().
            GetSYAEngine().GetSupportedOutlierAlgorithms();
    lItemDelegate->setAlgoValues(lAvailableAlgos.keys());
    lItemDelegate->setAlgoNames(lAvailableAlgos.values());
    lTableView->setItemDelegate(lItemDelegate);

    return lAvailableAlgos;
}

void SYALimitsWidget::UpdateSpecificsTableView()
{

}

} // namespace Gex
} // namespace GS
