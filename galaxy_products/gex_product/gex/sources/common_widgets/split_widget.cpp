
#include <QPushButton>
#include <QTableView>
#include "split_widget.h"
#include "common_widgets/collapsible_button.h"
#include "spm_version_widget.h"

namespace GS
{
namespace Gex
{

SplittedWindowWidget::SplittedWindowWidget( QWidget *parent):
                           QWidget(parent),
                           mUI(new Ui::SplitWidget)

{
    mUI->setupUi(this);
}


SplittedWindowWidget::~SplittedWindowWidget()
{
    delete mUI;
    qDeleteAll(mRightBannerUnfoldButtons);
}


void SplittedWindowWidget::InitRightBannerUI()
{
    mUI->scrollAreaRight->setLayout(new QVBoxLayout());
    // Set minimum opening size of the right part
    mUI->scrollAreaRight->setMinimumWidth(300);
    // Make sure all widgets aren't centered
    mUI->scrollAreaRight->layout()->setAlignment(Qt::AlignTop);
    mUI->scrollAreaRight->layout()->setContentsMargins(2, 2, 2, 2);

    //-- add the button in charge of launching the copy
    CustomizeRightBannerUI(mUI->scrollAreaRight->layout());

    static_cast<QVBoxLayout*>(mUI->scrollAreaRight->layout())->addSpacerItem(
                new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

}

CollapsibleButton* SplittedWindowWidget::CreateUnfoldWidgetInLeftBanner(const QString &title, QWidget* widget)
{
    CollapsibleButton* lButton = new CollapsibleButton(title);
    lButton->addWidget(widget);

    mUI->scrollAreaLeft->layout()->addWidget(lButton);

    lButton->SetDetachable(qobject_cast<QVBoxLayout*>(mUI->scrollAreaLeft->layout()));

    return lButton;
}

CollapsibleButton* SplittedWindowWidget::CreateUnfoldWidgetInRightBanner(const QString &title, QWidget* widget)
{
    CollapsibleButton* lButton = new CollapsibleButton(title, T_Informative);
    lButton->addWidget(widget);

    //-- add in the list of button
    mRightBannerUnfoldButtons.push_back(lButton);

    mUI->scrollAreaRight->layout()->addWidget(lButton);
    return lButton;
}

void SplittedWindowWidget::InitUI()
{
    InitRightBannerUI();

    mUI->scrollAreaLeft->setLayout(new QVBoxLayout());
    mUI->scrollAreaLeft->setWidgetResizable(false);
    mUI->scrollAreaLeft->layout()->setAlignment(Qt::AlignTop);
    mUI->scrollAreaLeft->layout()->setContentsMargins(2, 2, 2, 2);

   CustomizeLeftBannerUI(mUI->scrollAreaLeft->layout());

   // make sure left and right are well splitted
   mUI->splitter->setStretchFactor(0, 4);
   mUI->splitter->setStretchFactor(1, 0);
}


}
}
