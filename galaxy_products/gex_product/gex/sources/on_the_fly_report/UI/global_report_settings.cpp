#include <QKeyEvent>
#include "component.h"
#include "global_report_settings.h"
#include "ui_global_report_settings.h"

global_report_settings::global_report_settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::global_report_settings),
    mReportElement(0)
{
    ui->setupUi(this);
    connect(ui->lineEdit,         SIGNAL(textChanged(const QString&)),  this, SLOT(nameChanged(const QString&)));
    connect(ui->textEdit,         SIGNAL(textChanged()),                this, SLOT(commentChanged()));
}

global_report_settings::~global_report_settings()
{
    delete ui;
}

void global_report_settings::clear()
{
   blockSignals(true);
   mReportElement = 0;
   ui->lineEdit->clear();
   ui->textEdit->clear();
   blockSignals(false);
}

void global_report_settings::commentChanged()
{
    if(mReportElement)
    {
        mReportElement->SetComment(ui->textEdit->toPlainText());
        emit changesHasBeenMade();
    }
}

void global_report_settings::loadReportElement(Component* reportElement)
{
    blockSignals(true);
    mReportElement = reportElement;
    ui->lineEdit->setText(mReportElement->GetName());
    ui->textEdit->setText(mReportElement->GetComment());
    blockSignals(false);

    emit titleReportChanged(ui->lineEdit->text());
}


void global_report_settings::nameChanged(const QString& name)
{
    if(mReportElement)
    {
        mReportElement->SetName(name);
    }

    emit titleReportChanged(name);
    emit changesHasBeenMade();
}

/******************************************************************************!
 * \fn keyPressEvent
 ******************************************************************************/
void global_report_settings::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        ui->lineEdit->clearFocus();
    }
}
