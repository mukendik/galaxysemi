#include <QMessageBox>
#include <QCheckBox>
#include <QKeyEvent>
#include "product_info.h"
#include "picktest_item.h"
#include "picktest_dialog.h"
#include "section_report_settings.h"
#include "ui_section_settings.h"
#include "common_widgets/multi_select_input_dialog.h"

#include "statistical_table.h"

const int DEFAULT_TOP_N=10;

section_report_settings::section_report_settings(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::section_settings),
    mSectionElement(0), mDislayMsgOfNoChildTied(true)
{
    mUi->setupUi(this);

    connect(mUi->pushButtonSelectTests,  SIGNAL(clicked()),                      this, SLOT(onPickTest()));
    connect(mUi->pushButtonSelectGroups, SIGNAL(clicked()),                      this, SLOT(onPickGroups()));

    connect(mUi->radioButtonNone,        SIGNAL(clicked(bool)),                  this, SLOT(updateSectionSettings()));
    connect(mUi->radioButtonTestList,    SIGNAL(clicked(bool)),                  this, SLOT(updateSectionSettings()));
    connect(mUi->radioButtonTopN,        SIGNAL(clicked(bool)),                  this, SLOT(updateSectionSettings()));

    connect(mUi->radioButtonGroupNoFilter,SIGNAL(toggled(bool)),                 this, SLOT(updateGroupFilterType()));
    connect(mUi->radioButtonGroupList,    SIGNAL(toggled(bool)),                 this, SLOT(updateGroupFilterType()));

    connect(mUi->radioButtonNone,        SIGNAL(toggled(bool)),                  this, SLOT(updateTestFilterType()));
    connect(mUi->radioButtonTestList,    SIGNAL(toggled(bool)),                  this, SLOT(updateTestFilterType()));
    connect(mUi->radioButtonTopN,        SIGNAL(toggled(bool)),                  this, SLOT(updateTestFilterType()));

    connect(mUi->radioButtonNone,        SIGNAL(clicked(bool)),                  this, SLOT(informUser()));
    connect(mUi->radioButtonTestList,    SIGNAL(clicked(bool)),                  this, SLOT(informUser()));
    connect(mUi->radioButtonTopN,        SIGNAL(clicked(bool)),                  this, SLOT(informUser()));

    connect(mUi->lineEditName,           SIGNAL(textChanged(const QString&)),    this, SLOT(nameChanged(const QString&)));
    connect(mUi->spinBox,                SIGNAL(valueChanged(int)),              this, SLOT(updateTopNValue(int)) );
    connect(mUi->checkBoxSplitByGroup,   SIGNAL(clicked(bool)),                  this, SLOT(updateSplitRule(bool)));
    connect(mUi->textEdit,               SIGNAL(textChanged()),                  this,  SLOT(commentChanged()));

    mUi->radioButtonGroupNoFilter->setChecked(true);
    mUi->radioButtonGroupList->setChecked(false);

    mUi->spinBox->setValue(DEFAULT_TOP_N);
}

section_report_settings::~section_report_settings()
{
    delete mUi;
}

void section_report_settings::updateSectionSettings()
{
    if(mSectionElement)
    {
        if(mUi->radioButtonTestList->isChecked())
        {
            emit testsSet(mSectionElement, true);
        }
        else if(mUi->radioButtonTopN->isChecked())
        {
            emit testsSet(mSectionElement, true);
        }
        else
        {
           emit testsSet(mSectionElement, false);
        }
    }
}

void section_report_settings::clear()
{
   blockSignals(true);
   mSectionElement = 0;
   mUi->lineEditName->clear();
   mUi->listWidgetTests->clear();
   mUi->listWidgetGroups->clear();
   blockSignals(false);
}


void section_report_settings::commentChanged()
{
    if(mSectionElement)
    {
        mSectionElement->SetComment(mUi->textEdit->toPlainText());
        emit changesHasBeenMade();
    }
}

void section_report_settings::fillListTests()
{
    if(mSectionElement)
    {
        QList<Test*> lTestFilter = mSectionElement->GetTestsListFilter();

        for(int i = 0; i< lTestFilter.count(); ++i)
        {
            if (lTestFilter[i])
            {
                mUi->listWidgetTests->addItem( lTestFilter[i]->mNumber + " " + lTestFilter[i]->mName);
            }
        }
    }
}

void section_report_settings::fillListGroups()
{
    if(mSectionElement)
    {
        QList<Group*> lGroupsFilter = mSectionElement->GetGroupsListFilter();

        for(int i = 0; i< lGroupsFilter.count(); ++i)
        {
            if (lGroupsFilter[i])
            {
                mUi->listWidgetGroups->addItem(lGroupsFilter[i]->mName);
            }
        }
    }
}

void    section_report_settings::loadSectionElement(SectionElement* sectionElement)
{
    blockSignals(true);
    mSectionElement = sectionElement;

    mUi->lineEditName->setText(sectionElement->GetName());
    mUi->textEdit->setText(sectionElement->GetComment());
    mUi->listWidgetTests->hide();
    mUi->listWidgetGroups->hide();
    T_TestListFilter lTestType = mSectionElement->GetTestListFilterType();
    switch(lTestType)
    {
        case T_TESTS_LIST:
        {
            mUi->radioButtonTestList->setChecked(true);
            mUi->listWidgetTests->show();
            break;
        }
        case T_TESTS_TOPN:
        {
            mUi->radioButtonTopN->setChecked(true);
            break;
        }
        case T_TESTS_NOFILTER:
        default:
        {
            mUi->radioButtonNone->setChecked(true);
            break;
        }
    }
    T_GroupListFilter lGroupType = mSectionElement->GetGroupListFilterType();
    switch(lGroupType)
    {
        case T_GROUPS_LIST:
        {
            mUi->radioButtonGroupList->setChecked(true);
            mUi->listWidgetGroups->show();
            break;
        }
        case T_GROUPS_NOFILTER:
        default:
        {
            mUi->radioButtonGroupNoFilter->setChecked(true);
            break;
        }
    }
    mUi->checkBoxSplitByGroup->setChecked(mSectionElement->IsSplitByGroup());
    mUi->groupBoxOptions->setVisible(mSectionElement->ContainsTable());
    updateTestFilterType();
    updateGroupFilterType();
    blockSignals(false);
}

void section_report_settings::updateTopNValue(int value)
{
    if(mSectionElement)
    {
        mSectionElement->SetTopNValue(value);
        if(value > 0)
            updateTestFilterType();

        emit changesHasBeenMade();
    }
}

void section_report_settings::updateSplitRule(bool value)
{
    mSectionElement->SetSplitByGroup(value);
}

void section_report_settings::nameChanged(const QString& name)
{
    if(mSectionElement)
    {
        mSectionElement->SetName(name);
        emit sectionNameChanged(name);
        emit changesHasBeenMade();
    }
}

void section_report_settings::informUser()
{
    // -- check that there is a child to apply this new settings
    //-- otherwise inform the user that there will be no impact
    if(mSectionElement->GetTestListFilterType() == T_TESTS_TOPN || mSectionElement->GetTestListFilterType() == T_TESTS_LIST)
    {
        if(mDislayMsgOfNoChildTied == true)
        {
            QMessageBox lMsgBox;
            lMsgBox.setText(QString(
                            " Do you want to apply the section [%1] settings to all elements in this section?")
                            .arg(mSectionElement->GetName()));
            lMsgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));

            QCheckBox lCheckBox(&lMsgBox);
            lCheckBox.setText("Do not show this message again.");
            lMsgBox.setCheckBox(&lCheckBox);

            lMsgBox.setIcon(QMessageBox::Information);
            lMsgBox.setStandardButtons(QMessageBox::Apply |QMessageBox::No);
            lMsgBox.setDefaultButton(QMessageBox::No);

            int lRes = lMsgBox.exec();

            if(lMsgBox.checkBox()->isChecked())
                mDislayMsgOfNoChildTied = false;

            if(lRes == QMessageBox::Apply)
            {
                mSectionElement->TiedAllChild();
                emit testsSet(mSectionElement, true);
            }
        }
    }
}

void section_report_settings::onPickTest()
{
    if(mSectionElement == 0)
        return;

    bool bMultiselection  = true;
    bool bSelectAllGroups = true;

    // Show TestList
    PickTestDialog lPickTest;

    // Allow/Disable Multiple selections
    lPickTest.setMultipleSelection(bMultiselection);
    lPickTest.setMultipleGroups(true, bSelectAllGroups);
    lPickTest.setUseInteractiveFilters(true);

    // Check if List was successfuly loaded
    if (lPickTest.fillParameterList())
    {
        // Prompt dialog box, let user pick tests from the list
        if (lPickTest.exec() == QDialog::Accepted)
        {
            QList<PickTestItem* > lSelectedTests = lPickTest.testSelected();

            if(lSelectedTests.empty() == false)
            {
                mSectionElement->ClearTestsFilter();
                mUi->listWidgetTests->show();
                mUi->listWidgetTests->clear();
                QList<PickTestItem* >::iterator lIterBegin(lSelectedTests.begin()), lIterEnd(lSelectedTests.end());
                for(; lIterBegin != lIterEnd; ++lIterBegin)
                {
                    PickTestItem* lTest = *lIterBegin;

                    mSectionElement->AddTestFilter(lTest->testNumber(),
                                                   lTest->testName(),
                                                   lTest->pinName(),
                                                   lTest->GetGroupIndex(),
                                                   lTest->GetFileIndex());

                            emit changesHasBeenMade();
                }
            }

            updateTestFilterType();
        }
    }
}

void section_report_settings::onPickGroups()
{
    QList<QStringList> lGroups;
    QList<Group*> lGroupsElt = static_cast<StatisticalTable*>(mSectionElement->GetElements().at(0))->GetGroupList();

    for (int lGroupId = 0; lGroupId < lGroupsElt.size(); ++lGroupId)
    {
        QStringList lGroupInfos;
        lGroupInfos<< lGroupsElt.at(lGroupId)->mNumber << lGroupsElt.at(lGroupId)->mName;
        lGroups <<  lGroupInfos;
    }

    bool lOK;
    QStringList lHeader; lHeader << "Group #" << "Group name";
    QList<QStringList> lSelected = MultiSelectInputDialog::GetSelectedItems(
                this, "Groups selection", "Select groups to include", lGroups, lHeader, &lOK);
    if (lOK)
    {
        mSectionElement->ClearGroupsFilter();
        mUi->listWidgetGroups->show();
        mUi->listWidgetGroups->clear();
        for (int lGroupId = 0; lGroupId < lSelected.size(); ++lGroupId)
        {
            mSectionElement->AddGroupFilter(lSelected.at(lGroupId).at(0),
                                            lSelected.at(lGroupId).at(1));
            emit changesHasBeenMade();
        }
        updateGroupFilterType();
    }
}


void section_report_settings::updateTestFilterType()
{
    if(mSectionElement == 0 )
        return;

    mUi->listWidgetTests->clear();
    mUi->listWidgetTests->hide();

    //-- keep the value displayed even if the radio is not checked
    mUi->spinBox->setValue(mSectionElement->GetTopNValue());
    if(mUi->radioButtonTestList->isChecked())
    {
        mUi->listWidgetTests->show();
        fillListTests();
        mSectionElement->SetTestListFilterType(T_TESTS_LIST);
    }
    else if(mUi->radioButtonTopN->isChecked())
    {
        mSectionElement->SetTestListFilterType(T_TESTS_TOPN);
    }
    else
    {
        mUi->listWidgetTests->clear();
        mSectionElement->SetTestListFilterType(T_TESTS_NOFILTER);
    }

    emit changesHasBeenMade();
}

void section_report_settings::updateGroupFilterType()
{
    if(mSectionElement == 0 )
        return;

    if(mUi->radioButtonGroupList->isChecked())
    {
        mUi->listWidgetGroups->show();
        mUi->listWidgetGroups->clear();
        fillListGroups();
        mSectionElement->SetGroupListFilterType(T_GROUPS_LIST);
    }
    else
    {
        mUi->listWidgetGroups->hide();
        mSectionElement->SetGroupListFilterType(T_GROUPS_NOFILTER);
    }

    emit changesHasBeenMade();
}

/******************************************************************************!
 * \fn keyPressEvent
 ******************************************************************************/
void section_report_settings::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        mUi->lineEditName->clearFocus();
    }
}
