#include "ui_info_widget.h"
#include "info_widget.h"

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    mUI(new Ui::Info())

{
    mUI->setupUi(this);
    mUI->label_icon->setText("");
}

InfoWidget::~InfoWidget()
{
    delete mUI;
}


void InfoWidget::ProductionVersion(bool state)
{
    if(state == true)
    {
        //mUI->label_icon->setPixmap(QPixmap(":/gex/icons/enable.png"));
        mUI->label_icon->setPixmap(QPixmap(":/gex/icons/clean_undo_stack.png"));
    }
    else
    {
        mUI->label_icon->setPixmap(QPixmap());
    }
}

void InfoWidget::Clear()
{
     mUI->label_Id->setText("");
     mUI->creationDate->setText("");
     mUI->expirationDate->setText("");
     mUI->computation->setText("");
     ProductionVersion(false);
}

//QPushButton* SPMInfoWidget::PushButton()
//{
//    return mUI->pushButton;
//}

void InfoWidget::UpdateIdLabel(const QString& label)
{
    mUI->label_Id->setText(label);
}

void InfoWidget::UpdateCreationLabel(const QString& label)
{
    mUI->creationDate->setText(label);
}

void InfoWidget::UpdateExpirationLabel(const QString& label)
{
    mUI->expirationDate->setText(label);
}

void InfoWidget::UpdateComputationLabel(const QString& label)
{
    mUI->computation->setText(label);
}
