
#include "sm_legend_widget.h"
#include "ui_sm_legend_widget.h"


namespace GS
{
namespace Gex
{

SMLegendWidget::SMLegendWidget(QWidget *parent) :
    QWidget(parent),
    mUI(new Ui::SMLegend)
{
    mUI->setupUi(this);
}

SMLegendWidget::~SMLegendWidget()
{
    delete mUI;
}

void SMLegendWidget::InitUI()
{
    // Init legend area
    QPalette lPalette;
    lPalette.setColor(QPalette::Background, Qt::lightGray);
    mUI->frameColDisabled->setPalette(lPalette);
    mUI->frameColDisabled->setAutoFillBackground(true);
    lPalette.setColor(QPalette::Background, QColor("#81ffa2"));
    mUI->frameColUpdated->setPalette(lPalette);
    mUI->frameColUpdated->setAutoFillBackground(true);
    lPalette.setColor(QPalette::Background, Qt::red);
    mUI->frameCritical->setPalette(lPalette);
    mUI->frameCritical->setAutoFillBackground(true);
    lPalette.setColor(QPalette::Background, QColor("#FFA500"));
    mUI->frameStandard->setPalette(lPalette);
    mUI->frameStandard->setAutoFillBackground(true);
    lPalette.setColor(QPalette::Background, Qt::yellow);
    mUI->frameColToBeRecomputed->setPalette(lPalette);
    mUI->frameColToBeRecomputed->setAutoFillBackground(true);
}

} // namespace Gex
} // namespace GS
