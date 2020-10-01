#include "rborrow_dialog.h"
#include "ui_rborrow_dialog.h"
#include <QPushButton>

RBorrow_dialog::RBorrow_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rborrow_dialog)
{
    ui->setupUi(this);

    connect(ui->lineEditServer, SIGNAL(textChanged(QString)), this, SLOT(UpdateUI()));
}

QString RBorrow_dialog::GetServer() const
{
    return ui->lineEditServer->text();
}

int RBorrow_dialog::GetPort() const
{
    return ui->spinBoxPort->value();
}

void RBorrow_dialog::SetServer(const QString &server)
{
    ui->lineEditServer->setText(server);
}

void RBorrow_dialog::SetPort(int port)
{
    ui->spinBoxPort->setValue(port);
}

void RBorrow_dialog::UpdateUI()
{
    QPushButton * lButtonOK = ui->buttonBox->button(QDialogButtonBox::Ok);

    if (lButtonOK)
    {
        if (ui->lineEditServer->text().isEmpty())
            lButtonOK->setEnabled(false);
        else
            lButtonOK->setEnabled(true);
    }
}

RBorrow_dialog::~RBorrow_dialog()
{
    delete ui;
}
