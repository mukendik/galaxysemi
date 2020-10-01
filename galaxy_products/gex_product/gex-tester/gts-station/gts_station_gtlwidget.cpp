#include "gts_station_gtlwidget.h"
#include "ui_gts_station_gtlwidget.h"

#include "gtl_core.h"


GtsStationGtlWidget::GtsStationGtlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GtsStationGtlWidget)
{
    ui->setupUi(this);

    // Fill keys combo box
    ui->comboGtlKeys->addItem(GTL_KEY_OUTPUT_FOLDER);
    ui->comboGtlKeys->addItem(GTL_KEY_OUTPUT_FILENAME);
    ui->comboGtlKeys->addItem(GTL_KEY_DATA_FILENAME);
    ui->comboGtlKeys->addItem(GTL_KEY_LIB_VERSION);
    ui->comboGtlKeys->addItem(GTL_KEY_NUM_OF_MESSAGES_IN_STACK);
    ui->comboGtlKeys->addItem(GTL_KEY_OUTPUT_TYPE);
    ui->comboGtlKeys->addItem(GTL_KEY_OUTPUT_OPTIONS);
    ui->comboGtlKeys->addItem(GTL_KEY_TESTING_STAGE);
    ui->comboGtlKeys->addItem(GTL_KEY_SOCKET_TRACE);
    ui->comboGtlKeys->addItem(GTL_KEY_RECONNECTION_MODE);
    ui->comboGtlKeys->addItem("gtm_communication_mode"); // just to check
    ui->comboGtlKeys->addItem("<custom>");

    connect (ui->buttonGetKey, SIGNAL(clicked()), this, SLOT(OnButtonGetKey()));
    connect (ui->buttonSetKey, SIGNAL(clicked()), this, SLOT(OnButtonSetKey()));
    connect(ui->comboGtlKeys, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboGtlKeysChanged()));

    OnComboGtlKeysChanged();
}

GtsStationGtlWidget::~GtsStationGtlWidget()
{
    delete ui;
}

void GtsStationGtlWidget::OnComboGtlKeysChanged()
{
    // Check combo value
    if(ui->comboGtlKeys->currentText().toLower() == "<custom>")
    {
        ui->buttonGetKey->setEnabled(true);
        ui->editGtlKeyName->setEnabled(true);
    }
    else
    {
        ui->buttonGetKey->setEnabled(false);
        ui->editGtlKeyName->setEnabled(false);
        ui->editGtlKeyName->setText(ui->comboGtlKeys->currentText());
    }

    RefreshGui();
}

void GtsStationGtlWidget::OnButtonGetKey()
{
    RefreshGui();
}

void GtsStationGtlWidget::OnButtonSetKey()
{
    if(!ui->editGtlKeyName->text().isEmpty())
    {
        int r=gtl_set(ui->editGtlKeyName->text().toLatin1().data(), ui->editGtlKeyValue->text().toLatin1().data());
        ui->editStatus->setText(QString("%1").arg(r));
    }
    else
        ui->editGtlKeyValue->setText(QString("<unknown>"));
}

void GtsStationGtlWidget::RefreshGui()
{
    char lValue[1024];

    // Get current key value if key not empty
    if(!ui->editGtlKeyName->text().isEmpty())
    {
        int r=gtl_get(ui->editGtlKeyName->text().toLatin1().constData(), lValue);
        ui->editStatus->setText(QString("%1").arg(r));
        if(r==GTL_CORE_ERR_OKAY)
            ui->editGtlKeyValue->setText(lValue);
        else
            ui->editGtlKeyValue->setText(QString("<unknown>"));
    }
    else
        ui->editGtlKeyValue->setText(QString("<unknown>"));
}
