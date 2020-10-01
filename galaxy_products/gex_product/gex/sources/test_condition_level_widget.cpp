#include "test_condition_level_widget.h"
#include "ui_test_condition_level_widget.h"

#include "pick_fields_dialog.h"
#include <gqtl_log.h>

#include <QShortcut>

namespace GS
{
namespace Gex
{
TestConditionLevelWidget::TestConditionLevelWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::TestConditionLevelWidget)
{
    mUi->setupUi(this);

    mUi->treeWidgetField->setRootIsDecorated(false);
    mUi->treeWidgetField->setColumnCount(2);
    mUi->treeWidgetField->setHeaderLabels(QStringList() << "" << "Conditions");
    mUi->treeWidgetField->setColumnHidden(0, true);
    mUi->treeWidgetField->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Update button status
    UpdateUI();

    connect(mUi->pushButtonTop,         SIGNAL(clicked()),      SLOT(OnTop()));
    connect(mUi->pushButtonUp,          SIGNAL(clicked()),      SLOT(OnUp()));
    connect(mUi->pushButtonDown,        SIGNAL(clicked()),      SLOT(OnDown()));
    connect(mUi->pushButtonBottom,      SIGNAL(clicked()),      SLOT(OnBottom()));
    connect(mUi->toolButtonEditAdd,     SIGNAL(clicked()),      SLOT(OnEditAdd()));
    connect(mUi->toolButtonEditRemove,  SIGNAL(clicked()),      SLOT(OnEditRemove()));
    connect(mUi->treeWidgetField,       SIGNAL(itemSelectionChanged()),
                                        SLOT(UpdateUI()));

    // Create shortcuts
    new QShortcut(Qt::Key_Insert, this, SLOT(OnEditAdd()));
    new QShortcut(Qt::Key_Delete, this, SLOT(OnEditRemove()));
}

TestConditionLevelWidget::~TestConditionLevelWidget()
{
    delete mUi;
}

QStringList TestConditionLevelWidget::GetSelectedFields() const
{
    QStringList selectedFields;

    for (int lIdx = 0; lIdx < mUi->treeWidgetField->topLevelItemCount(); ++lIdx)
        selectedFields.append(mUi->treeWidgetField->topLevelItem(lIdx)->text(1));

    return selectedFields;
}

void TestConditionLevelWidget::AddSelectedField(const QString &field)
{
    AddSelectedFields(QStringList(field));
}

void TestConditionLevelWidget::AddSelectedFields(const QStringList &fields)
{
    QString lField;
    bool    lUpdated = false;

    for (int lIdx = 0; lIdx < fields.count(); ++lIdx)
    {
        lField = fields.at(lIdx);

        if (mFields.contains(lField, Qt::CaseInsensitive))
        {
            if (mUi->treeWidgetField->findItems(lField, Qt::MatchExactly).count() == 0)
            {
                QTreeWidgetItem * pItem = new QTreeWidgetItem(mUi->treeWidgetField);

                pItem->setText(1, lField);

                lUpdated = true;
            }
            else
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Condition %1 already selected")
                      .arg(lField).toLatin1().constData());
        }
        else
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Condition %1 not available")
                  .arg(lField).toLatin1().constData());
    }

    if (lUpdated)
    {
        UpdateUI();

        emit levelChanged(GetSelectedFields());
    }
}

void TestConditionLevelWidget::SetAvailableFields(const QStringList &fields,
                                                  bool keepSelection)
{
    mFields = fields;

    if (keepSelection)
    {
        for (int lIdx = mUi->treeWidgetField->topLevelItemCount()-1; lIdx >=0; --lIdx)
        {
            if (mFields.contains(mUi->treeWidgetField->topLevelItem(lIdx)->text(1),
                                 Qt::CaseInsensitive) == false)
                mUi->treeWidgetField->takeTopLevelItem(lIdx);
        }
    }
    else
        mUi->treeWidgetField->clear();

    UpdateUI();
}

void TestConditionLevelWidget::SetSelectedFields(const QStringList& fields)
{
    if (fields != GetSelectedFields())
    {
        mUi->treeWidgetField->clear();

        if (fields.count() > 0)
            AddSelectedFields(fields);
        else
            UpdateUI();
    }
}

void TestConditionLevelWidget::OnEditAdd()
{
    if (mFields.count() != mUi->treeWidgetField->topLevelItemCount())
    {
        QStringList lUnselected = mFields;
        QStringList lSelection  = GetSelectedFields();

        foreach(const QString &condition, lSelection)
            lUnselected.removeAll(condition);

        // Open the dialog to select the fields
        PickFieldsDialog fieldsDialog(lUnselected , this);

        // Show the dialog box
        if (fieldsDialog.exec() == QDialog::Accepted)
            AddSelectedFields(fieldsDialog.selectedFieldList());
    }
}

void TestConditionLevelWidget::OnEditRemove()
{
    QList<QTreeWidgetItem*> lItems = mUi->treeWidgetField->selectedItems();

    foreach(QTreeWidgetItem * pItem, lItems)
    {
        int lIndex  = mUi->treeWidgetField->indexOfTopLevelItem(pItem);

        if (lIndex >= 0)
            delete mUi->treeWidgetField->takeTopLevelItem(lIndex);
    }

    if (lItems.count())
        emit levelChanged(GetSelectedFields());
}

void TestConditionLevelWidget::OnTop()
{
    QTreeWidgetItem *   pItem   = mUi->treeWidgetField->currentItem();
    int                 lIndex  = mUi->treeWidgetField->indexOfTopLevelItem(pItem);

    if (lIndex > 0)
    {
        if (pItem)
        {
            mUi->treeWidgetField->takeTopLevelItem(lIndex);
            mUi->treeWidgetField->insertTopLevelItem(0, pItem);
            mUi->treeWidgetField->setCurrentItem(pItem);

            emit levelChanged(GetSelectedFields());
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Invalid flow");

}

void TestConditionLevelWidget::OnUp()
{
    QTreeWidgetItem *   pItem   = mUi->treeWidgetField->currentItem();
    int                 lIndex  = mUi->treeWidgetField->indexOfTopLevelItem(pItem);

    if (lIndex > 0)
    {
        if (pItem)
        {
            mUi->treeWidgetField->takeTopLevelItem(lIndex);
            mUi->treeWidgetField->insertTopLevelItem(lIndex-1, pItem);
            mUi->treeWidgetField->setCurrentItem(pItem);

            emit levelChanged(GetSelectedFields());
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Invalid flow");
}

void TestConditionLevelWidget::OnDown()
{
    QTreeWidgetItem *   pItem   = mUi->treeWidgetField->currentItem();
    int                 lIndex  = mUi->treeWidgetField->indexOfTopLevelItem(pItem);

    if (lIndex < mUi->treeWidgetField->topLevelItemCount()-1)
    {
        if (pItem)
        {
            mUi->treeWidgetField->takeTopLevelItem(lIndex);
            mUi->treeWidgetField->insertTopLevelItem(lIndex+1, pItem);
            mUi->treeWidgetField->setCurrentItem(pItem);

            emit levelChanged(GetSelectedFields());
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Invalid flow");
}

void TestConditionLevelWidget::OnBottom()
{
    QTreeWidgetItem *   pItem   = mUi->treeWidgetField->currentItem();
    int                 lIndex  = mUi->treeWidgetField->indexOfTopLevelItem(pItem);

    if (lIndex < mUi->treeWidgetField->topLevelItemCount()-1)
    {
        if (pItem)
        {
            mUi->treeWidgetField->takeTopLevelItem(lIndex);
            mUi->treeWidgetField->addTopLevelItem(pItem);
            mUi->treeWidgetField->setCurrentItem(pItem);

            emit levelChanged(GetSelectedFields());
        }
    }
    else
        GSLOG(SYSLOG_SEV_WARNING, "Invalid flow");
}

void TestConditionLevelWidget::UpdateUI()
{
    int                     lIndex;
    int                     lCount  = mUi->treeWidgetField->topLevelItemCount();
    QList<QTreeWidgetItem*> lItems  = mUi->treeWidgetField->selectedItems();

    // Enable/Disable add/remove buttons
    mUi->toolButtonEditAdd->setEnabled(mFields.count() != lCount);
    mUi->toolButtonEditRemove->setEnabled(lItems.count() != 0);

    if (lItems.count() == 1)
    {
        lIndex  = mUi->treeWidgetField->indexOfTopLevelItem(lItems.first());

        // Enable/Disable up/top buttons
        mUi->pushButtonTop->setEnabled(lIndex > 0);
        mUi->pushButtonUp->setEnabled(lIndex > 0);

        // Enable/Disable down/bottom buttons
        mUi->pushButtonDown->setEnabled(lIndex >= 0 && lIndex < lCount-1);
        mUi->pushButtonBottom->setEnabled(lIndex >= 0 && lIndex < lCount-1);
    }
    else
    {
        // Disable up/top buttons
        mUi->pushButtonTop->setEnabled(false);
        mUi->pushButtonUp->setEnabled(false);

        // Disable down/bottom buttons
        mUi->pushButtonDown->setEnabled(false);
        mUi->pushButtonBottom->setEnabled(false);
    }

    // Keep focus on the tree widget
    mUi->treeWidgetField->setFocus();
}

}
}
