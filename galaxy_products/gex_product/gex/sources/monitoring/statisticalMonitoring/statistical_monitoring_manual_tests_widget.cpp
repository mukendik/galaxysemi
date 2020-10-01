#include <QLineEdit>
#include <QMessageBox>

#include "statistical_monitoring_manual_tests_widget.h"
#include "ui_statistical_monitoring_manual_tests_widget.h"


StatisticalMonitoringManualTestsWidget::StatisticalMonitoringManualTestsWidget(const QString& numberPlaceHolderLabel,const QString& namePlaceHolderLabel, QWidget *parent) : QDialog(parent),
    ui(new Ui::StatisticalMonitoringManualTestsWidget),
    mNumberPlaceHolderLabel(numberPlaceHolderLabel),
    mNamePlaceHolderLabel(namePlaceHolderLabel),
    nextItemIndex(0)
{
    ui->setupUi(this);

    QObject::connect(ui->pushButtonOK ,     SIGNAL(clicked()), this, SLOT(OnOKClicked()));
    QObject::connect(ui->pushButtonCancel , SIGNAL(clicked()), this, SLOT(reject()));
    InitGUI();

}

StatisticalMonitoringManualTestsWidget::~StatisticalMonitoringManualTestsWidget()
{
    QWidget* lSettings = ui->scrollArea->widget();
    if (!lSettings)
        return;
    QLayout* lLayout = lSettings->layout();
    if (!lLayout)
        return;

    qDeleteAll(mTests);
}

void StatisticalMonitoringManualTestsWidget::InitGUI()
{
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
    OnAddItem();
    OnAddItem();
    OnAddItem();
    OnAddItem();
}

void StatisticalMonitoringManualTestsWidget::OnAddItem(int num, QString name)
{
    ManualLimitItem* lNewTest = new ManualLimitItem(nextItemIndex, num, name);
    ui->scrollAreaWidgetContents->layout()->addWidget(lNewTest);
    lNewTest->setPlaceHolderLabelNumber(mNumberPlaceHolderLabel);
    lNewTest->setPlaceHolderLabelName(mNamePlaceHolderLabel);
    QObject::connect(lNewTest, SIGNAL(sDeleteItem(int)), this, SLOT(OnDeleteItem(int)));
    QObject::connect(lNewTest, SIGNAL(sAddItem()), this, SLOT(OnAddItem()));
    mTests.insert(nextItemIndex, lNewTest);
    ++nextItemIndex;
    update();
}


void StatisticalMonitoringManualTestsWidget::OnDeleteItem(int index)
{
    // Don't delete the last item
    if (mTests.size() == 1)
        return;

    QWidget* lSettings = ui->scrollArea->widget();
    if (!lSettings)
        return;
    QLayout* lLayout = lSettings->layout();
    if (!lLayout)
        return;

    lLayout->removeWidget(mTests.value(index));
    delete mTests[index];
    mTests.remove(index);
    lLayout->update();
    lSettings->update();
    this->update();
}

void StatisticalMonitoringManualTestsWidget::OnOKClicked()
{
    unsigned int lValidTests = 0;
    QHash<int, ManualLimitItem*>::iterator lIterBegin(mTests.begin()), lIterEnd(mTests.end());
    for (; lIterBegin != lIterEnd; ++lIterBegin)
    {
        if (lIterBegin.value()->GetTestNumber() == -2 && lIterBegin.value()->GetTestName().isEmpty())
        {
            continue;
        }
        else if (lIterBegin.value()->GetTestNumber() == -2)
        {
            QMessageBox::warning(0, tr("Invalid test found"),
                          tr("Missing 'number' field"));
            return;
        }
        else
        {
            lValidTests++;
        }
    }
    if(lValidTests == 0)
    {
        QMessageBox::warning(0, tr("Empty test list"),
                      tr("No valid test has been found"));
    }
    else
    {
        accept();
    }
}

void  StatisticalMonitoringManualTestsWidget::GetNewElements(QList<QPair<int, QString> >& list)
{
    list.clear();
    QHash<int, ManualLimitItem*>::iterator lIterBegin(mTests.begin()), lIterEnd(mTests.end());
    for (; lIterBegin != lIterEnd; ++lIterBegin)
    {
        if ((lIterBegin.value()->GetTestNumber() >= -1))
        {
            list.append(QPair<int, QString>(lIterBegin.value()->GetTestNumber(), lIterBegin.value()->GetTestName()));
        }
    }
}

void StatisticalMonitoringManualTestsWidget::AddItems(QList<QPair<int, QString> > &list)
{
    if(list.size() != 0)
    {
        // Clear the existing items
        QListIterator<int> testIter(mTests.keys());
        while(testIter.hasNext())
        {
            int itemIndex = testIter.next();
            OnDeleteItem(itemIndex);
        }

        // Create new items
        QListIterator<QPair<int, QString> > iter(list);
        while(iter.hasNext())
        {
            QPair<int, QString> item = iter.next();
            OnAddItem(item.first, item.second);
        }
    }
}

