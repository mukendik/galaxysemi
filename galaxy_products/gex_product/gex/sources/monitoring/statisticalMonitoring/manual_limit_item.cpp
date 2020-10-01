#include "manual_limit_item.h"
#include "ui_manual_limit_item.h"
#include <limits>


ManualLimitItem::ManualLimitItem(int index, int num, QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::manual_limit_item)
{
    ui->setupUi(this);

    ui->lineEditTestNumber->setValidator( new QIntValidator(-1,  std::numeric_limits<int>::max(), this) );
    if(num != -2)
    {
        ui->lineEditTestNumber->setText(QString::number(num));
    }
    ui->lineEdit->setText(name);
    QObject::connect(ui->pushButtonAddTest, SIGNAL(clicked()), this, SLOT(onAddItem()), Qt::UniqueConnection);
    QObject::connect(ui->pushButtonDeleteItem, SIGNAL(clicked()), this, SLOT(onDeleteItem()), Qt::UniqueConnection);
    mIndex = index;
}

ManualLimitItem::~ManualLimitItem()
{
    delete ui;
}

void ManualLimitItem::onAddItem()
{
    emit sAddItem();
}

void ManualLimitItem::setPlaceHolderLabelNumber(const QString& label)
{
    ui->lineEditTestNumber->setPlaceholderText(label);
}

void ManualLimitItem::setPlaceHolderLabelName(const QString& label)
{
    ui->lineEdit->setPlaceholderText(label);
}

void ManualLimitItem::onDeleteItem()
{
    emit sDeleteItem(mIndex);
}

gsint32 ManualLimitItem::GetTestNumber() const
{
    if (ui->lineEditTestNumber && ui->lineEditTestNumber->text().isEmpty() == false)
        return static_cast<gsint32>(ui->lineEditTestNumber->text().toInt());
    else
        return -2;
}

QString ManualLimitItem::GetTestName()
{
    return ui->lineEdit->text();
}

