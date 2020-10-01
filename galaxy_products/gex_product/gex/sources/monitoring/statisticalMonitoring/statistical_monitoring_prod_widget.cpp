
#include <QAbstractTextDocumentLayout>
#include <QTableView>
#include <QStringList>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

#include "statistical_monitoring_version_widget.h"
#include "statistical_monitoring_limits_widget.h"
#include "statistical_monitoring_prod_widget.h"
#include "statistical_monitoring_taskdata.h"
#include "statistical_monitoring_task.h"
#include "common_widgets/table_view.h"
#include "limits_model.h"
#include "common_widgets/collapsible_button.h"
#include "info_widget.h"
#include "sm_sql_table_model.h"
#include "engine.h"
#include "db_engine.h"
#include "sm_legend_widget.h"
#include "message.h"

namespace GS
{
namespace Gex
{

StatisticalMonitoringProdWidget::StatisticalMonitoringProdWidget(
        CGexMoTaskStatisticalMonitoring *task, QWidget *parent) :
    SplittedWindowWidget(parent),
    mLimitUnFoldButton(0),
    mVersionUnFoldButton(0),
    mVersionWidget(0),
    mLimitsWidget(0),
    mInfoWidget(0),
    mTableInfoWidget(0),
    mIdViewed(-1)
{
    mTask = task;
}

StatisticalMonitoringProdWidget::~StatisticalMonitoringProdWidget()
{
    // -- if parent == 0, means thant this is an unpin window
    // --  and that we need to delete it manually
    if(mVersionUnFoldButton && mVersionUnFoldButton->parent() == 0)
    {
        mVersionUnFoldButton->hide();
        delete mVersionUnFoldButton;
    }

    if(mLimitUnFoldButton && mLimitUnFoldButton->parent() == 0)
    {
        mLimitUnFoldButton->hide();
        delete mLimitUnFoldButton;
    }
}

void StatisticalMonitoringProdWidget::CopyTaskVersion()
{
    if(mTask == 0)
        return;

    int lIdVersion = mVersionWidget->SelectedId();
    bool lOk;

    GS::Gex::Message::request("", QString("Confirm the limit copy of the version Id %1 into the draft.\n"
                                  "All your current changes will be lost.").arg(lIdVersion), lOk);
    if (lOk)
    {
        if(mTask->DuplicateVersionToDraft(lIdVersion))
            emit NewLimitCopied();
    }
}

void StatisticalMonitoringProdWidget::UpdateUIAfterDelete(int versionId)
{
    //--if current display version is the one deleted, reload on the draft version
    if(mIdViewed == versionId)
    {
       mLimitsWidget->GetModel()->clear();
       mLimitUnFoldButton->close();
       mLimitUnFoldButton->setTitle(QString("Data Preview"));
    }
}

void StatisticalMonitoringProdWidget::LoadVersionId(int versionId)
{
    LoadLimitsFromDatabase(versionId);
    mIdViewed = versionId;
    mLimitUnFoldButton->open();
}

bool StatisticalMonitoringProdWidget::LoadLimitsFromDatabase(int versionId)
{
    if(mTask == 0)
        return false;

    setCursor(Qt::WaitCursor);

    bool lLastProdVersion = false;
    if(versionId == -1)
    {
        versionId = mTask->GetActiveProdVersionId();
        lLastProdVersion = true;
    }

    bool    lStatus = false;
    QString lProducts = "n/a";
    QString lLabel = "n/a";
    if (versionId > 0)
    {
        lStatus = mLimitsWidget->LoadDraftTask(versionId, mTask, lLastProdVersion, lProducts, lLabel);
    }

    mLimitUnFoldButton->setTitle(QString("Data Preview (Id: %1)").arg(versionId));

    LimitsModel* lModel = dynamic_cast<LimitsModel*>(mLimitsWidget->GetModel());
    bool lEnableButtons = lModel &&
            (lModel->rowCount() > 0) &&
            !lModel->hasNonManualLimitsToUpdate();
    mLimitsWidget->RefreshUI(lEnableButtons);

    setCursor(Qt::ArrowCursor);

    return lStatus;
}

bool StatisticalMonitoringProdWidget::LoadVersionsFromDatabase(bool forceReload)
{
    if (mTask && mVersionWidget)
    {
        SMSQLTableModel *lModel = mTask->CreateVersionsModel();

        if(lModel)
        {
            mVersionWidget->InitUI(lModel, forceReload);
            return true;
        }
    }

    return false;
}

void StatisticalMonitoringProdWidget::CustomizeLeftBannerUI(QLayout*)
{
    //--1. Init Version table fold/unfold
    mVersionWidget       = CreateVersionWidget();
    mVersionUnFoldButton = CreateUnfoldWidgetInLeftBanner("Versions", mVersionWidget);

    connect(mVersionWidget, SIGNAL(OnVersionId(int)),       this, SLOT(LoadVersionId(int)));
    connect(mVersionWidget, SIGNAL(DeletedVersion(int)),    this, SLOT(UpdateUIAfterDelete(int)));

    //--2. Init limit fold/unfold
    QFrame *lLimitsFrame = new QFrame();
    lLimitsFrame->setLayout(new QVBoxLayout());
    mLimitsWidget->CreateDataUI(lLimitsFrame->layout());
    mLimitUnFoldButton = CreateUnfoldWidgetInLeftBanner("Data Preview", lLimitsFrame);
}

void StatisticalMonitoringProdWidget::CustomizeRightBannerUI(QLayout* layout)
{
    QPushButton* lPushButtonCopy = new QPushButton("<< Copy selected limits to draft");
    lPushButtonCopy->setToolTip("copy this version limits into the draft");
    layout->addWidget(lPushButtonCopy);

    connect(lPushButtonCopy , SIGNAL(clicked(bool)), this, SLOT(CopyTaskVersion()));

    //-- create the info part
    mInfoWidget = new InfoWidget();
    CreateUnfoldWidgetInRightBanner("Informations", mInfoWidget);

     // -- Init Comment part
    QFrame *lComment = new QFrame();
    lComment->setLayout(new QVBoxLayout());
    mLimitsWidget->CreateCommentArea(lComment->layout());
    CreateUnfoldWidgetInRightBanner("Comment", lComment);

    // ---Legend area
    mLegend = new SMLegendWidget();
    mLegend->InitUI();
    CreateUnfoldWidgetInRightBanner("Legend", mLegend);
}

void StatisticalMonitoringProdWidget::InitializeUI()
{
    mLimitsWidget = CreateLimitsWidget();
    mLimitsWidget->SetReadOnly(true);
    if(mLimitsWidget)
    {
        InitUI();
        mVersionWidget->SetInfoWidget(mInfoWidget);
    }
}

bool StatisticalMonitoringProdWidget::IsDataModelEmpty()
{
    if(mLimitsWidget)
    {
        return mLimitsWidget->IsDataModelEmpty();
    }
    return true;
}

} // namespace Gex
} // namespace GS
