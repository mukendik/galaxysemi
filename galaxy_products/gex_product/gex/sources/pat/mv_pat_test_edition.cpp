#include "mv_pat_test_edition.h"
#include "pat_definition.h"
namespace GS
{
namespace Gex
{


bool CompareTests(CPatDefinition *test1, CPatDefinition *test2)
{
    return(test1->m_lTestNumber < test2->m_lTestNumber);
}

MVPATTestEdition::MVPATTestEdition(QWidget *parent, PATMultiVariateRule *rule, QHash<QString, CPatDefinition *> &univarTests)
    :QDialog(parent),mRule(rule),mUVTests(univarTests)
{
    setupUi(this);
    if(parent)
    {
        setWindowTitle(parent->windowTitle());
    }

    mEditedRule = *mRule;
    mRuleName->setText(mEditedRule.GetName());

    const QList<PATMultiVariateRule::MVTestData> &lTestData = mEditedRule.GetMVTestData();
    mIncludedTest->setRowCount(lTestData.count());
    QStringList lIncludedTest;
    for(int lIdx=0; lIdx<lTestData.count(); ++lIdx)
    {
        mIncludedTest->setItem(lIdx,0,new QTableWidgetItem(QString::number(lTestData[lIdx].GetTestNumber())));
        mIncludedTest->item(lIdx,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        mIncludedTest->setItem(lIdx,1,new QTableWidgetItem(QString::number(lTestData[lIdx].GetPinIdx())));
        mIncludedTest->item(lIdx,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        mIncludedTest->setItem(lIdx,2,new QTableWidgetItem(lTestData[lIdx].GetTestName()));
        mIncludedTest->item(lIdx,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        lIncludedTest.append(QString("%1 - %2 - %3").arg(lTestData[lIdx].GetTestNumber()).arg(lTestData[lIdx].GetPinIdx()).arg(lTestData[lIdx].GetTestName()));
    }

    QList<CPatDefinition *> lPatDef = mUVTests.values();
    qSort(lPatDef.begin(), lPatDef.end(), CompareTests);
    int lRow = 0;
    QList<CPatDefinition *>::Iterator lDefIterator = lPatDef.begin();
    while(lDefIterator != lPatDef.end())
    {
        CPatDefinition *lDef = *lDefIterator;
        QString lTestExcluded = QString("%1 - %2 - %3").arg(lDef->m_lTestNumber).arg(lDef->mPinIndex).arg(lDef->m_strTestName);
        if(!lIncludedTest.contains(lTestExcluded))
        {
            mExcludedTest->insertRow(lRow);

            mExcludedTest->setItem(lRow,0,new QTableWidgetItem(QString::number(lDef->m_lTestNumber)));
            mExcludedTest->item(lRow,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            mExcludedTest->setItem(lRow,1,new QTableWidgetItem(QString::number(lDef->mPinIndex)));
            mExcludedTest->item(lRow,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            mExcludedTest->setItem(lRow,2,new QTableWidgetItem(lDef->m_strTestName));
            mExcludedTest->item(lRow,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            ++lRow;
        }
        ++lDefIterator;
    }

    connect(mExcludeTest, SIGNAL(clicked()), this, SLOT(ExcludedTest()));
    connect(mIncludeTest, SIGNAL(clicked()), this, SLOT(IncludedTest()));
    connect(mButtonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(OnOk()));
    connect(mButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));


}

MVPATTestEdition::~MVPATTestEdition()
{

}

void MVPATTestEdition::OnOk()
{
    (*mRule) = mEditedRule;
    done(1);
}

void MVPATTestEdition::IncludedTest()
{
    if(!mExcludedTest->selectedItems().isEmpty())
    {
        QList<QTableWidgetItem*> lSelectedItem = mExcludedTest->selectedItems();
        QList<int> lRowList;
        for(int lIdx=0; lIdx<lSelectedItem.count(); ++lIdx)
        {
            QTableWidgetItem *lItem = lSelectedItem[lIdx];
            if(!lRowList.contains(lItem->row()))
            {
                lRowList.append(lItem->row());
            }
        }

        for(int lIdx = 0; lIdx<lRowList.count(); ++lIdx)
        {
            int lRow = lRowList[lIdx];
            int lInsertionRow = mIncludedTest->rowCount();
            mIncludedTest->insertRow(mIncludedTest->rowCount());

            int lTestNumber = mExcludedTest->item(lRow,0)->text().toInt();
            int lPinIdx     = mExcludedTest->item(lRow,1)->text().toInt();
            QString lName   = mExcludedTest->item(lRow,2)->text();

            for(int lCol=0;lCol<3;++lCol)
            {
                QTableWidgetItem *lRemoved = mExcludedTest->takeItem(lRow, lCol);
                mIncludedTest->setItem(lInsertionRow,lCol,lRemoved);
            }
            mEditedRule.AddTestData(PATMultiVariateRule::MVTestData(lName, lTestNumber, lPinIdx));
        }
        qSort(lRowList.begin(),lRowList.end());
        for(int lIdx = lRowList.count()-1; lIdx>=0; --lIdx)
        {
            int lRow = lRowList[lIdx];
            mExcludedTest->removeRow(lRow);
        }
    }

}

void MVPATTestEdition::ExcludedTest()
{
    if(!mIncludedTest->selectedItems().isEmpty())
    {
        QList<QTableWidgetItem*> lSelectedItem = mIncludedTest->selectedItems();
        QList<int> lRowList;
        for(int lIdx=0; lIdx<lSelectedItem.count(); ++lIdx)
        {
            QTableWidgetItem *lItem = lSelectedItem[lIdx];
            if(!lRowList.contains(lItem->row()))
            {
                lRowList.append(lItem->row());
            }
        }

        for(int lIdx = 0; lIdx<lRowList.count(); ++lIdx)
        {
            int lRow = lRowList[lIdx];
            int lInsertionRow = mExcludedTest->rowCount();
            mExcludedTest->insertRow(mExcludedTest->rowCount());
            int lTestNumber = mIncludedTest->item(lRow,0)->text().toInt();
            int lPinIdx     = mIncludedTest->item(lRow,1)->text().toInt();
            QString lName = mIncludedTest->item(lRow,2)->text();

            for(int lCol=0;lCol<3;++lCol)
            {
                QTableWidgetItem *lRemoved = mIncludedTest->takeItem(lRow, lCol);
                mExcludedTest->setItem(lInsertionRow,lCol,lRemoved);
            }
            mEditedRule.RemoveAllTestData(PATMultiVariateRule::MVTestData(lName, lTestNumber, lPinIdx));
        }
        qSort(lRowList.begin(),lRowList.end());
        for(int lIdx = lRowList.count()-1; lIdx>=0; --lIdx)
        {
            int lRow = lRowList[lIdx];
            mIncludedTest->removeRow(lRow);
        }
    }
}

}
}
