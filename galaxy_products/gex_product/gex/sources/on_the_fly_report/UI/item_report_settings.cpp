#include <QKeyEvent>
#include "picktest_dialog.h"
#include "picktest_item.h"
#include "report_element.h"
#include "item_report_settings.h"
#include "ui_item_report_settings.h"

item_report_settings::item_report_settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::item_report_settings),
    mReportElement(0)
{
    ui->setupUi(this);

    // Keep for V2
    // connect(ui->pushButtonSelectTest, SIGNAL(clicked()),                    this,   SLOT(onPickTest()));
    // connect(ui->groupBox,             SIGNAL(clicked(bool)),                this,   SLOT(activeTest()));

    connect(ui->lineEditName,         SIGNAL(textChanged(const QString&)),  this,   SLOT(nameChanged(const QString&)));
    connect(ui->textEdit,             SIGNAL(textChanged()),                this,   SLOT(commentChanged()));
    connect(ui->lineEditTest,         SIGNAL(textChanged(QString)),         this,   SLOT(checkTestEmptyEntry()));
    connect(ui->pushButtonTie,        SIGNAL(clicked(bool)),                this,   SLOT(tieUntie()));

    ui->lineEditTest->setEnabled(false);
    //-- for this version No activation
    ui->pushButtonSelectTest->hide();

    mTieIcon.addPixmap(QPixmap(":/gex/icons/connect.png"));
    mUntieIcon.addPixmap(QPixmap(":/gex/icons/disconnect.png"));
    mIsTieWithSection = true;
}

item_report_settings::~item_report_settings()
{
    delete ui;
}

void item_report_settings::clear()
{
   blockSignals(true);
   mReportElement = 0;
   ui->lineEditName->clear();
   ui->lineEditTest->clear();
   blockSignals(false);
}

void item_report_settings::checkTestEmptyEntry()
{
    if(ui->lineEditTest->text().isEmpty() && mReportElement)
    {
        mReportElement->SetTestActivated(false);
    }
}

void item_report_settings::commentChanged()
{
    if(mReportElement)
    {
        mReportElement->SetComment(ui->textEdit->toPlainText());
        emit changesHasBeenMade();
    }
}

void item_report_settings::reLoad()
{
    if(mReportElement)
    {
        loadReportElement(mReportElement);
    }
}

void    item_report_settings::loadReportElement  (ReportElement* reportElement)
{
    blockSignals(true);
    mReportElement = reportElement;


    if(mReportElement->GetType() == T_TABLE || mReportElement->GetType() == T_CAPABILITY_TABLE_CONNECTED)
    {
        ui->pushButtonTie->hide();
        ui->labelTest->hide();
        ui->lineEditTest->hide();
    }
    else
    {
        ui->pushButtonTie->show();
        ui->labelTest->show();
        ui->lineEditTest->show();
    }

    ui->lineEditName->setText(mReportElement->GetName());
    ui->textEdit->setText(mReportElement->GetComment());

    mIsTieWithSection = mReportElement->IsTieWithSection();
    updateTieButton();
    if(mIsTieWithSection == false || reportElement->IsSectionSettingsControlActivated() == false)
    {
        fillTest();
        ui->textEdit->show();
    }
    else
    {
        ui->lineEditTest->setText("Linked to the section settings");
        ui->lineEditTest->setEnabled(false);
        ui->textEdit->hide();
    }
    blockSignals(false);
}

void item_report_settings::fillTest()
{
    if(mReportElement)
    {
        const Test& lTest = mReportElement->GetTestFilter();
        ui->lineEditTest->setText(lTest.mNumber + " " +lTest.mName);
    }
}

void item_report_settings::updateTieButton()
{
    if(mIsTieWithSection && mReportElement->IsSectionSettingsControlActivated())
    {
       ui->pushButtonTie->setIcon(mTieIcon);
    }
    else
    {
        ui->pushButtonTie->setIcon(mUntieIcon);
    }
}

void item_report_settings::tieUntie()
{
    if(mIsTieWithSection)
    {
       ui->pushButtonTie->setIcon(mUntieIcon);
       mIsTieWithSection = false;
       emit tieChanged(mReportElement, false);
    }
    else
    {
        ui->pushButtonTie->setIcon(mTieIcon);
        mIsTieWithSection = true;
        emit tieChanged(mReportElement, true);
    }
    emit changesHasBeenMade();
    reLoad();
}

void item_report_settings::nameChanged(const QString& name)
{
    if(mReportElement)
    {
        mReportElement->SetName(name);
        emit reportElementNameChanged(name);
        emit changesHasBeenMade();
    }
}

/******************************************************************************!
 * \fn keyPressEvent
 ******************************************************************************/
void item_report_settings::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        ui->lineEditName->clearFocus();
    }
}

// To keep for GUI V2
//void item_report_settings::onPickTest()
//{
//    if(mReportElement == 0)
//        return;

//    // Show TestList
//    PickTestDialog lPickTest;

//    // Allow/Disable Multiple selections
//    lPickTest.setMultipleSelection(false);
//    lPickTest.setMultipleGroups(true, false);
//    lPickTest.setUseInteractiveFilters(true);

//    // Check if List was successfuly loaded
//    if (lPickTest.fillParameterList())
//    {
//        // Prompt dialog box, let user pick tests from the list
//        if (lPickTest.exec() == QDialog::Accepted)
//        {
//            PickTestItem* lSelectedTest = lPickTest.testSelected()[0];
//            mReportElement->AddTestFilter(lSelectedTest->testNumber(), lSelectedTest->testName(), lSelectedTest->pinName(), lPickTest.getGroupID() );
//            emit newTest(mReportElement);
//            fillTest();
//        }
//    }
//}
