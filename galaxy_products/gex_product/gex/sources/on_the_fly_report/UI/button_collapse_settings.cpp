#include <QVBoxLayout>

#include "button_collapse_settings.h"
#include "ui_button_collapse_settings.h"

button_collapse_settings::button_collapse_settings(const QString &text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::button_collapse_settings),
    mLayout(0)
{
    ui->setupUi(this);
    ui->pushButtonSettings->setText(text);
    ui->pushButtonSettings->setIcon(QIcon(":/gex/icons/RightTriangle.png"));
    ui->frame->hide();

    connect(ui->pushButtonSettings, SIGNAL(clicked(bool)), this, SLOT(updateButtonSettings()));
}

button_collapse_settings::~button_collapse_settings()
{
    delete ui;
}

void button_collapse_settings::addWidget(QWidget* widget)
{
    if(mLayout == 0)
    {
        mLayout = new QVBoxLayout();
        mLayout->setContentsMargins(0,0,0,0);
        ui->frame->setLayout(mLayout);
    }
    mLayout->addWidget(widget);
}

void button_collapse_settings::close()
{
    ui->pushButtonSettings->setIcon(QIcon(":/gex/icons/RightTriangle.png"));
    ui->frame->hide();
}

void button_collapse_settings::open()
{
    ui->pushButtonSettings->setIcon(QIcon(":/gex/icons/DownTriangle.png"));
    ui->frame->show();
}

void button_collapse_settings::updateButtonSettings()
{
    if(ui->frame->isHidden() == false)
    {
        close();
    }
    else
    {
        open();
    }
}



