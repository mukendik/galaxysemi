#include "charac_box_whisker_template_dialog.h"
#include "ui_charac_box_whisker_template_dialog.h"
#include "gex_report.h"

#include <QColorDialog>

#define AGGREGATE_COLOR_ROLE    Qt::UserRole+1

namespace GS
{
namespace Gex
{

CharacBoxWhiskerTemplateDialog::CharacBoxWhiskerTemplateDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::CharacBoxWhiskerTemplateDialog)
{
    mUi->setupUi(this);

    // Init aggregate tree widget
    mUi->treeWidgetAggregate->setColumnCount(1);
    mUi->treeWidgetAggregate->setHeaderLabel("Color");
    mUi->treeWidgetAggregate->setRootIsDecorated(false);
}

CharacBoxWhiskerTemplateDialog::~CharacBoxWhiskerTemplateDialog()
{
    delete mUi;
}

const CharacBoxWhiskerTemplate &GS::Gex::CharacBoxWhiskerTemplateDialog::GetTemplate() const
{
    return mTemplate;
}

void CharacBoxWhiskerTemplateDialog::SetTemplate(const CharacBoxWhiskerTemplate &lTemplate)
{
    mTemplate = lTemplate;
}

void CharacBoxWhiskerTemplateDialog::SetConditionsLevel(const QStringList &conditionsLevel)
{
    mConditionsLevel = conditionsLevel;
}

void CharacBoxWhiskerTemplateDialog::SetConditionsValues(const QList<QMap<QString, QString> > &conditionsValues)
{
    mConditionsValues = conditionsValues;
}

void CharacBoxWhiskerTemplateDialog::FillGui()
{
    mUi->treeWidgetAggregate->clear();
    mUi->treeWidgetAggregate->setColumnCount(1);

    // Initialize template if not already done
    if (mTemplate.GetTopLevelAggregates().count() == 0)
        FillDefaultTemplate();

    // Fill Headers
    QStringList lAggregateLabels("Color");

    if (mTemplate.GetTopLevelCondition().isEmpty() == false)
        lAggregateLabels.append(mTemplate.GetTopLevelCondition());

    mUi->treeWidgetAggregate->setHeaderLabels(lAggregateLabels);

    // Fill series values
    foreach(const QString &lCondition, mTemplate.GetTopLevelAggregates())
    {
        QTreeWidgetItem * pItem = new QTreeWidgetItem(mUi->treeWidgetAggregate);

        // Create pixmap to show serie color
        QPixmap pixmap(10,10);
        pixmap.fill(mTemplate.GetTopLevelColor(lCondition));

        pItem->setIcon(0, pixmap);
        pItem->setData(0, AGGREGATE_COLOR_ROLE, mTemplate.GetTopLevelColor(lCondition));

        // Condition value (top level aggregate)
        pItem->setText(1, lCondition);
    }

    // Adjust column width to contents
    for (int lIdx = 0; lIdx < mUi->treeWidgetAggregate->columnCount(); ++lIdx)
        mUi->treeWidgetAggregate->resizeColumnToContents(lIdx);

    // Connect Signal/Slots
    connect(mUi->treeWidgetAggregate,   SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this,                       SLOT(OnAggregateDoubleClicked(QTreeWidgetItem*,int)));
}

void CharacBoxWhiskerTemplateDialog::accept()
{
    FillTemplateFromGUI();

    QDialog::accept();
}

void CharacBoxWhiskerTemplateDialog::FillTemplateFromGUI()
{
    QTreeWidgetItem *   pItem = NULL;
    mTemplate.Clear();

    // Template is empty if no condition defined
    if (mConditionsLevel.count() > 0)
    {
        // Set top level condition
        mTemplate.SetTopLevelCondition(mConditionsLevel.first());

        // Get Aggregate information
        for (int lIdx = 0; lIdx < mUi->treeWidgetAggregate->topLevelItemCount(); ++lIdx)
        {
            pItem = mUi->treeWidgetAggregate->topLevelItem(lIdx);

            QColor      lColor;
            QVariant    lVar            = pItem->data(0, AGGREGATE_COLOR_ROLE);
            QString     lConditionValue = pItem->text(1);

            if (lVar.isValid())
                lColor = lVar.value<QColor>();

            if (lConditionValue.isEmpty() == false)
                mTemplate.SetTopLevelColor(lConditionValue, lColor);
        }
    }
}

void CharacBoxWhiskerTemplateDialog::FillDefaultTemplate()
{
    mTemplate.Clear();

    // Template is empty if no condition defined
    if (mConditionsLevel.count() > 0)
    {
        // Set top level condition
        mTemplate.SetTopLevelCondition(mConditionsLevel.first());

        // Assign default color for all aggregate
        // Create the series to add to the template
        QStringList aggregateKey;

        for (int lItem = 0; lItem < mConditionsValues.count(); ++lItem)
        {
            QString     key = mConditionsValues.at(lItem).value(mTemplate.GetTopLevelCondition());

            // Aggregate not existing, so create it
            if (aggregateKey.indexOf(key) == -1)
            {
                // Append the key to the list
                aggregateKey.append(key);

                mTemplate.SetTopLevelColor(key,
                                           CGexReport::GetChartingColor(aggregateKey.count()));
            }
        }
    }
}

void CharacBoxWhiskerTemplateDialog::OnAggregateDoubleClicked(QTreeWidgetItem * pItem,
                                                              int lColumn)
{
    if (lColumn == 0)
    {
        QColor      newColor;
        QColor      currentColor;
        QVariant    var         = pItem->data(0, AGGREGATE_COLOR_ROLE);

        if (var.isValid())
            currentColor = var.value<QColor>();

        newColor = QColorDialog::getColor(currentColor, this);

        if(newColor.isValid() == true)
        {
            // Create pixmap to show Bin color
            QPixmap pixmap(10,10);
            pixmap.fill(newColor);

            pItem->setIcon(0, pixmap);
            pItem->setData(0, AGGREGATE_COLOR_ROLE, newColor);
        }
    }
}

}   // namespace Gex
}   // namespace GS
