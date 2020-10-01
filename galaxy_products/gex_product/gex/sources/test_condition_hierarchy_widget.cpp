#include "test_condition_hierarchy_widget.h"
#include "ui_test_condition_hierarchy_widget.h"

#include "gex_algorithms.h"

namespace GS
{
namespace Gex
{

TestConditionHierarchyWidget::TestConditionHierarchyWidget(QWidget *parent)
    : QWidget(parent), mUi(new Ui::TestConditionHierarchyWidget)
{
    mUi->setupUi(this);

    mUi->treeWidgetHierarchy->header()->setMovable(false);

    // Update buttoons status
    InitUI();

    connect(mUi->pushButtonTop,         SIGNAL(clicked()), this,
            SLOT(OnMoveTop()));
    connect(mUi->pushButtonUp,          SIGNAL(clicked()), this,
            SLOT(OnMoveUp()));
    connect(mUi->pushButtonDown,        SIGNAL(clicked()),
            this,    SLOT(OnMoveDown()));
    connect(mUi->pushButtonBottom,      SIGNAL(clicked()),
            this,    SLOT(OnMoveBottom()));
    connect(mUi->treeWidgetHierarchy,   SIGNAL(itemSelectionChanged()),
            this,    SLOT(InitUI()));
}

TestConditionHierarchyWidget::~TestConditionHierarchyWidget()
{
    if (mUi)
    {
        delete mUi;
        mUi = NULL;
    }
}

QStringList TestConditionHierarchyWidget::GetItems() const
{
    QVector<QString>    lValue(mUi->treeWidgetHierarchy->columnCount());
    QStringList         lItems;

    QTreeWidgetItem *   pParent = NULL;
    QTreeWidgetItem *   pItem   = NULL;
    int                 lLevel  = 0;
    int                 lIndex  = 0;

    for(int nTopLevel = 0; nTopLevel < mUi->treeWidgetHierarchy->topLevelItemCount(); ++nTopLevel)
    {
        lLevel  = 0;
        lIndex  = 0;
        pParent = mUi->treeWidgetHierarchy->topLevelItem(nTopLevel);
        pItem   = NULL;

        while(pParent)
        {
            lValue[lLevel] = pParent->text(lLevel);

            if (pParent->childCount())
            {
                if (pItem)
                    lIndex = pParent->indexOfChild(pItem)+1;
                else
                    lIndex = 0;

                if (lIndex < pParent->childCount())
                {
                    pParent = pParent->child(lIndex);
                    pItem   = NULL;
                    ++lLevel;
                }
                else
                {
                    pItem   = pParent;
                    pParent = pItem->parent();
                    --lLevel;
                }
            }
            else
            {
                QStringList items;
                for(int lIdx = 0; lIdx < lValue.count(); ++lIdx)
                    items += lValue.at(lIdx);

                lItems.append(items.join("|"));

                pItem   = pParent;
                pParent = pItem->parent();
                --lLevel;
            }
        }
    }

    return lItems;
}

QStringList TestConditionHierarchyWidget::GetHierarchy()
{
    QStringList                     lHashKeys;
    QMap<QString, QString>          lConditionMap;

    QTreeWidgetItem *   pParent = NULL;
    QTreeWidgetItem *   pItem   = NULL;
    int                 lLevel  = 0;
    int                 lIndex  = 0;

    for(int nTopLevel = 0; nTopLevel < mUi->treeWidgetHierarchy->topLevelItemCount(); ++nTopLevel)
    {
        lLevel  = 0;
        lIndex  = 0;
        pParent = mUi->treeWidgetHierarchy->topLevelItem(nTopLevel);
        pItem   = NULL;

        while(pParent)
        {
            lConditionMap.insert(mConditionLevel.at(lLevel), pParent->text(lLevel));

            if (pParent->childCount())
            {
                if (pItem)
                    lIndex = pParent->indexOfChild(pItem)+1;
                else
                    lIndex = 0;

                if (lIndex < pParent->childCount())
                {
                    pParent = pParent->child(lIndex);
                    pItem   = NULL;
                    ++lLevel;
                }
                else
                {
                    pItem   = pParent;
                    pParent = pItem->parent();
                    --lLevel;
                }
            }
            else
            {
                lHashKeys.append(MakeHashKey(lConditionMap, mConditionLevel));

                pItem   = pParent;
                pParent = pItem->parent();
                --lLevel;
            }
        }
    }

    return lHashKeys;
}

void TestConditionHierarchyWidget::Clear()
{
    mVisibleNode.clear();
    mConditionsGroup.clear();
    mConditionLevel.clear();
    mUi->treeWidgetHierarchy->clear();
    mUi->treeWidgetHierarchy->setColumnCount(0);
}


void TestConditionHierarchyWidget::SetConditionsGroups(const QList<QMap<QString, QString> >& conditionsGroups)
{
    QList<QMap<QString, QString> > lLocalCG = conditionsGroups;

    // Sort using numerical value if possible
    qSort(lLocalCG.begin(), lLocalCG.end(), OrderConditionsGroups);

    if (mConditionsGroup != lLocalCG)
    {
        // Clear existing tree
        Clear();

        mConditionsGroup = lLocalCG;

        for (int lIdx = 0; lIdx < mConditionsGroup.count(); ++lIdx)
        {
            if (IsMatchingFilter(mConditionsGroup.at(lIdx)))
                AddItem(mConditionsGroup.at(lIdx));
        }
    }
}

void TestConditionHierarchyWidget::SetFilters(const QHash<QString, QString> &filters)
{
    if (mFilters != filters)
    {
        mFilters = filters;

        QString lHashKey;

        for (int lIdx = 0; lIdx < mConditionsGroup.count(); ++lIdx)
        {
            lHashKey = MakeHashKey(mConditionsGroup.at(lIdx), mConditionLevel);

            if (IsMatchingFilter(mConditionsGroup.at(lIdx)))
            {
                if (mVisibleNode.contains(lHashKey) == false)
                    AddItem(mConditionsGroup.at(lIdx));
            }
            else
                RemoveItem(lHashKey);
        }

        // Adjust column width to contents
        for (int lIdx = 0; lIdx < mUi->treeWidgetHierarchy->columnCount(); ++lIdx)
            mUi->treeWidgetHierarchy->resizeColumnToContents(lIdx);
    }
}

bool TestConditionHierarchyWidget::OrderConditionsGroups(const QMap<QString, QString> &lMap1, const QMap<QString, QString> &lMap2)
{
    QString lS1;
    QString lS2;

    // Compare the value for all keys
    foreach(const QString &key, lMap1.keys())
    {
        lS1 = lMap1.value(key);

        // Should always contains the same keys
        // If not, consider Map1 is greater than Map2
        if (lMap2.contains(key))
        {
            lS2 = lMap2.value(key);

            // Compare the two string as numerical value if possible
            double lCompare = algorithms::gexCompareDoubleString(lS1, lS2);

            if (lCompare)
                return lCompare < 0.0;
        }
        else
            return false;
    }

    return false;
}

void TestConditionHierarchyWidget::AddItem(const QMap<QString, QString>& conditionGroup)
{
    if (mConditionLevel.count() > 0)
    {
        QTreeWidgetItem *   pParent         = NULL;
        QTreeWidgetItem *   pItem           = NULL;
        QString             lConditionName  = mConditionLevel.isEmpty()? "" : mConditionLevel.first();
        QString             lConditionValue = conditionGroup.value(lConditionName);
        QStringList         lConditionWithNullValue;

        // Only display combination where all condition have a valid value (not empty)
        for(int lIdx = 0; lIdx < mConditionLevel.count(); ++lIdx)
        {
            if (conditionGroup.value(mConditionLevel.at(lIdx)).isEmpty())
            {
                emit sEmptyCond(mConditionLevel.at(lIdx));
                return;
            }
        }

        for (int lIndex = 0; lIndex < mUi->treeWidgetHierarchy->topLevelItemCount() && pItem == NULL; ++lIndex)
        {
            if (mUi->treeWidgetHierarchy->topLevelItem(lIndex)->text(0) == lConditionValue)
                pItem = mUi->treeWidgetHierarchy->topLevelItem(lIndex);
        }

        if (pItem == NULL)
        {
            pParent = new QTreeWidgetItem(mUi->treeWidgetHierarchy);
            pParent->setText(0, lConditionValue);
            pParent->setExpanded(true);
        }
        else
            pParent = pItem;

        for(int lIdx = 1; lIdx < mConditionLevel.count(); ++lIdx)
        {
            pItem           = NULL;
            lConditionName  = mConditionLevel.at(lIdx);
            lConditionValue = conditionGroup.value(lConditionName);
            if (lConditionValue.isEmpty()) lConditionValue = "<NULL>";

            for (int lChild = 0; lChild < pParent->childCount() && pItem == NULL; ++lChild)
            {
                if (pParent->child(lChild)->text(lIdx) == lConditionValue)
                    pItem = pParent->child(lChild);
            }

            if (pItem == NULL)
            {
                pItem = new QTreeWidgetItem(pParent);
                pItem->setText(lIdx, lConditionValue);
                pItem->setExpanded(true);
            }

            pParent = pItem;
        }

        mVisibleNode.insert(MakeHashKey(conditionGroup, mConditionLevel), pItem);
    }
}

void TestConditionHierarchyWidget::RemoveItem(const QString& lHashKey)
{
    if (mConditionLevel.count() > 0 && mVisibleNode.contains(lHashKey))
    {
        mUi->treeWidgetHierarchy->setUpdatesEnabled(false);
        mUi->treeWidgetHierarchy->blockSignals(true);

        QTreeWidgetItem *   pItem           = mVisibleNode.value(lHashKey);

        if (pItem)
        {
            while (pItem->parent() && pItem->parent()->childCount() == 1)
                pItem = pItem->parent();

            if (pItem->parent())
            {
                pItem->parent()->removeChild(pItem);
                delete pItem;
                pItem = NULL;
            }
            else
            {
                int lIndex = mUi->treeWidgetHierarchy->indexOfTopLevelItem(pItem);
                mUi->treeWidgetHierarchy->takeTopLevelItem(lIndex);
                delete pItem;
                pItem = NULL;
            }
        }

        mVisibleNode.remove(lHashKey);

        mUi->treeWidgetHierarchy->blockSignals(false);
        mUi->treeWidgetHierarchy->setUpdatesEnabled(true);
    }
}

void TestConditionHierarchyWidget::ReloadHierarchy(const QStringList& lHierarchy,
                                                   const QStringList& lOldConditionLevel)
{
    mVisibleNode.clear();
    mUi->treeWidgetHierarchy->clear();
    mUi->treeWidgetHierarchy->setColumnCount(0);
    mUi->treeWidgetHierarchy->setHeaderLabels(mConditionLevel);

    if (lHierarchy.count())
    {
        QHash<QString, QMap<QString, QString> > lHashTmp;
        QString                                 lHashKey;
        for (int lCG = 0; lCG < mConditionsGroup.count(); ++lCG)
        {
            lHashKey = MakeHashKey(mConditionsGroup.at(lCG), lOldConditionLevel);

            lHashTmp.insertMulti(lHashKey, mConditionsGroup.at(lCG));
        }

        for(int lIdx = 0; lIdx < lHierarchy.count(); ++lIdx)
        {
            if (lHashTmp.contains(lHierarchy.at(lIdx)))
            {
                QList<QMap<QString, QString> > lConditionGroup = lHashTmp.values(lHierarchy.at(lIdx));

                for (int lValue = 0; lValue < lConditionGroup.count(); ++lValue)
                {
                    if (IsMatchingFilter(lConditionGroup.at(lValue)))
                        AddItem(lConditionGroup.at(lValue));
                }
            }
        }
    }
    else
    {
        for (int lIdx = 0; lIdx < mConditionsGroup.count(); ++lIdx)
        {
            AddItem(mConditionsGroup.at(lIdx));
        }
    }

    // Adjust column width to contents
    for (int lIdx = 0; lIdx < mUi->treeWidgetHierarchy->columnCount(); ++lIdx)
        mUi->treeWidgetHierarchy->resizeColumnToContents(lIdx);
}

bool TestConditionHierarchyWidget::IsFiltered(const QString &field, const QString &value)
{
    if (mFilters.contains(field))
    {
        QStringList values = mFilters.value(field).split(",", QString::SkipEmptyParts);

        if (values.contains(value) == false)
            return false;
    }

    return true;
}

bool TestConditionHierarchyWidget::IsMatchingFilter(const QMap<QString, QString> &conditionsGroup)
{
    if (mFilters.isEmpty() == false)
    {
        QHashIterator<QString, QString> itFilter(mFilters);
        QStringList                     values;

        while (itFilter.hasNext())
        {
            itFilter.next();

            values = itFilter.value().split(",", QString::SkipEmptyParts);

            if (values.contains(conditionsGroup.value(itFilter.key())) == false)
                return false;
        }
    }

    return true;
}

QString TestConditionHierarchyWidget::MakeHashKey(const QMap<QString, QString> &conditionsGroup,
                                                  const QList<QString>& conditionLevel)
{
    QString lHashKey;

    for(int lIdx = 0; lIdx < conditionLevel.count(); ++lIdx)
    {
        lHashKey += conditionsGroup.value(conditionLevel.at(lIdx));
    }

    return lHashKey;
}

void TestConditionHierarchyWidget::SetConditionLevel(const QStringList &conditionLevel)
{
    // Set new condition level
    if (mConditionLevel != conditionLevel)
    {
        // Get the current hierarchy in the tree
        QStringList lHierarchy = GetHierarchy();
        QStringList lOldConditionLevel = mConditionLevel;

        // Update condition level
        mConditionLevel = conditionLevel;

        ReloadHierarchy(lHierarchy, lOldConditionLevel);
    }
}

void TestConditionHierarchyWidget::OnMoveUp()
{
    QTreeWidgetItem * pItemSelected = mUi->treeWidgetHierarchy->currentItem();

    // If no selection, just ignore action.
    if(pItemSelected == NULL)
        return;

    // Get the parent item
    QTreeWidgetItem * pParent = pItemSelected->parent();

    if (pParent)
    {
        int lIndex = pParent->indexOfChild(pItemSelected);

        if (lIndex > 0)
        {
            pParent->takeChild(lIndex);
            pParent->insertChild(lIndex-1, pItemSelected);
        }
    }
    else
    {
        int lIndex = mUi->treeWidgetHierarchy->indexOfTopLevelItem(pItemSelected);

        if (lIndex > 0)
        {
            mUi->treeWidgetHierarchy->takeTopLevelItem(lIndex);
            mUi->treeWidgetHierarchy->insertTopLevelItem(lIndex-1, pItemSelected);
        }
    }

    pItemSelected->setExpanded(true);

    HierarchyChanged(pItemSelected);

    mUi->treeWidgetHierarchy->setCurrentItem(pItemSelected);
}

void TestConditionHierarchyWidget::OnMoveDown()
{
    QTreeWidgetItem * pItemSelected = mUi->treeWidgetHierarchy->currentItem();

    // If no selection, just ignore action.
    if(pItemSelected == NULL)
        return;

    // Get the parent item
    QTreeWidgetItem * pParent = pItemSelected->parent();

    if (pParent)
    {
        int lIndex = pParent->indexOfChild(pItemSelected);

        if (lIndex < pParent->childCount()-1)
        {
            pParent->takeChild(lIndex);
            pParent->insertChild(lIndex+1, pItemSelected);
        }
    }
    else
    {
        int lIndex = mUi->treeWidgetHierarchy->indexOfTopLevelItem(pItemSelected);

        if (lIndex < mUi->treeWidgetHierarchy->topLevelItemCount()-1)
        {
            mUi->treeWidgetHierarchy->takeTopLevelItem(lIndex);
            mUi->treeWidgetHierarchy->insertTopLevelItem(lIndex+1, pItemSelected);
        }
    }

    pItemSelected->setExpanded(true);

    HierarchyChanged(pItemSelected);

    mUi->treeWidgetHierarchy->setCurrentItem(pItemSelected);
}

void TestConditionHierarchyWidget::OnMoveTop()
{
    QTreeWidgetItem * pItemSelected = mUi->treeWidgetHierarchy->currentItem();

    // If no selection, just ignore action.
    if(pItemSelected == NULL)
        return;

    // Get the parent item
    QTreeWidgetItem * pParent = pItemSelected->parent();

    if (pParent)
    {
        int lIndex = pParent->indexOfChild(pItemSelected);

        if (lIndex > 0)
        {
            pParent->takeChild(lIndex);
            pParent->insertChild(0, pItemSelected);
        }
    }
    else
    {
        int lIndex = mUi->treeWidgetHierarchy->indexOfTopLevelItem(pItemSelected);

        if (lIndex > 0)
        {
            mUi->treeWidgetHierarchy->takeTopLevelItem(lIndex);
            mUi->treeWidgetHierarchy->insertTopLevelItem(0, pItemSelected);
        }
    }

    pItemSelected->setExpanded(true);

    HierarchyChanged(pItemSelected);

    mUi->treeWidgetHierarchy->setCurrentItem(pItemSelected);
}

void TestConditionHierarchyWidget::OnMoveBottom()
{
    QTreeWidgetItem * pItemSelected = mUi->treeWidgetHierarchy->currentItem();

    // If no selection, just ignore action.
    if(pItemSelected == NULL)
        return;

    // Get the parent item
    QTreeWidgetItem * pParent = pItemSelected->parent();

    if (pParent)
    {
        int lIndex = pParent->indexOfChild(pItemSelected);

        if (lIndex < pParent->childCount()-1)
        {
            pParent->takeChild(lIndex);
            pParent->addChild(pItemSelected);
        }
    }
    else
    {
        int lIndex = mUi->treeWidgetHierarchy->indexOfTopLevelItem(pItemSelected);

        if (lIndex < mUi->treeWidgetHierarchy->topLevelItemCount()-1)
        {
            mUi->treeWidgetHierarchy->takeTopLevelItem(lIndex);
            mUi->treeWidgetHierarchy->addTopLevelItem(pItemSelected);
        }
    }

    pItemSelected->setExpanded(true);

    HierarchyChanged(pItemSelected);

    mUi->treeWidgetHierarchy->setCurrentItem(pItemSelected);
}

void TestConditionHierarchyWidget::InitUI()
{
    QList<QTreeWidgetItem *> lItems = mUi->treeWidgetHierarchy->selectedItems();

    bool                lTop    = true;
    bool                lUp     = true;
    bool                lDown   = true;
    bool                lBottom = true;

    // If no selection, just ignore action.
    if(lItems.isEmpty() == false)
    {
        // Get the parent item
        QTreeWidgetItem *   pParent = lItems.isEmpty()? NULL : lItems.first()->parent();

        if (pParent)
        {
            int lIndex = pParent->indexOfChild(lItems.first());

            if (lIndex == 0)
            {
                lTop    = false;
                lUp     = false;
            }

            if (lIndex == pParent->childCount()-1)
            {
                lDown   = false;
                lBottom = false;
            }
        }
        else
        {
            int lIndex = mUi->treeWidgetHierarchy->indexOfTopLevelItem(lItems.first());

            if (lIndex == 0)
            {
                lTop    = false;
                lUp     = false;
            }

            if (lIndex == mUi->treeWidgetHierarchy->topLevelItemCount()-1)
            {
                lDown   = false;
                lBottom = false;
            }
        }
    }
    else
    {
        lTop    = false;
        lUp     = false;
        lDown   = false;
        lBottom = false;
    }


    mUi->pushButtonTop->setEnabled(lTop);
    mUi->pushButtonUp->setEnabled(lUp);
    mUi->pushButtonDown->setEnabled(lDown);
    mUi->pushButtonBottom->setEnabled(lBottom);
}

int TestConditionHierarchyWidget::GetItemLevel(QTreeWidgetItem *pItem)
{
    int                 lLevel  = -1;
    QTreeWidgetItem *   pTemp   = pItem;

    while (pTemp)
    {
        ++lLevel;

        pTemp = pTemp->parent();
    }

    return lLevel;
}

QList<QTreeWidgetItem *> TestConditionHierarchyWidget::GetItemsAtLevel(int lLevel)
{
    int                         lTmpLevel = 0;
    int                         lIndex    = 0;
    QList<QTreeWidgetItem *>    list;

    QTreeWidgetItem * pItem     = NULL;
    QTreeWidgetItem * pParent   = NULL;

    for (int lChild = 0; lChild < mUi->treeWidgetHierarchy->topLevelItemCount(); ++lChild)
    {
        if (lTmpLevel == lLevel)
        {
            list.append(mUi->treeWidgetHierarchy->topLevelItem(lChild));
        }
        else
        {
            pParent = mUi->treeWidgetHierarchy->topLevelItem(lChild);

            while(pParent)
            {
                if (pItem)
                    lIndex = pParent->indexOfChild(pItem)+1;
                else
                    lIndex = 0;

                if (lIndex < pParent->childCount())
                {
                    pParent = pParent->child(lIndex);
                    pItem   = NULL;
                    ++lTmpLevel;

                    if (lTmpLevel == lLevel)
                    {
                        list.append(pParent);

                        pItem   = pParent;
                        pParent = pItem->parent();
                        --lTmpLevel;
                    }
                }
                else
                {
                    pItem   = pParent;
                    pParent = pItem->parent();
                    --lTmpLevel;
                }
            }
        }
    }

    return list;
}

void TestConditionHierarchyWidget::HierarchyChanged(QTreeWidgetItem *pItem)
{
    // If no selection, just ignore action.
    if(pItem != NULL)
    {
        int lLevel = GetItemLevel(pItem);

        if (lLevel > 0)
        {
            QStringList lHierarchy;

            for (int lChild = 0; lChild < pItem->parent()->childCount(); ++lChild)
                lHierarchy.append(pItem->parent()->child(lChild)->text(lLevel));

            ApplyLevelHierarchy(lHierarchy, lLevel);
        }
    }
}

void TestConditionHierarchyWidget::ApplyLevelHierarchy(const QStringList &lHierarchy,
                                                       int lLevel)
{
    QList<QTreeWidgetItem*> items = GetItemsAtLevel(lLevel-1);

    foreach(QTreeWidgetItem * pParent, items)
    {
        int lIndex = 0;

        foreach(const QString &lValue, lHierarchy)
        {
            for(int lChild = 0; lChild < pParent->childCount(); ++lChild)
            {
                if (lValue == pParent->child(lChild)->text(lLevel))
                {
                    QTreeWidgetItem * pItem = pParent->takeChild(lChild);

                    pParent->insertChild(lIndex, pItem);
                    ++lIndex;

                    break;
                }
            }
        }
    }

    // Adjust column width to contents
    for (int lIdx = 0; lIdx < mUi->treeWidgetHierarchy->columnCount(); ++lIdx)
        mUi->treeWidgetHierarchy->resizeColumnToContents(lIdx);
}

}
}

