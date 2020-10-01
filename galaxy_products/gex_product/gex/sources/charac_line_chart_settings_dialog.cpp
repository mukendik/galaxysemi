#include "charac_line_chart_settings_dialog.h"
#include "ui_charac_line_chart_settings_dialog.h"

#include "gex_report.h"

#include <QColorDialog>
#include <QPushButton>

#define SERIE_COLOR_ROLE    Qt::UserRole+1

namespace GS
{
namespace Gex
{

CharacLineChartSettingsDialog::CharacLineChartSettingsDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::CharacLineChartSettingsDialog)
{
    mUi->setupUi(this);

    // Init conditions tree widget
    mUi->treeWidgetConditions->setColumnCount(1);
    mUi->treeWidgetConditions->setHeaderLabel("Conditions");
    mUi->treeWidgetConditions->setRootIsDecorated(false);

    // Init series tree widget
    mUi->treeWidgetSeries->setColumnCount(1);
    mUi->treeWidgetSeries->setHeaderLabel("Color");
    mUi->treeWidgetSeries->setRootIsDecorated(false);

    // Init Variable to draw
    mUi->comboBoxVariable->addItem("Use mean", "mean");
    mUi->comboBoxVariable->addItem("Use median", "median");
}

CharacLineChartSettingsDialog::~CharacLineChartSettingsDialog()
{
    delete mUi;
}

const CharacLineChartTemplate &CharacLineChartSettingsDialog::GetTemplate() const
{
    return mTemplate;
}

void CharacLineChartSettingsDialog::SetConditionsLevel(const QStringList &conditionsLevel)
{
    mConditionsLevel = conditionsLevel;
}

void CharacLineChartSettingsDialog::SetConditionsValues(const QList<QMap<QString, QString> > &conditionsValues)
{
    mConditionsValues = conditionsValues;
}

void CharacLineChartSettingsDialog::SetTemplate(const CharacLineChartTemplate &lTemplate)
{
    mTemplate = lTemplate;
}

void CharacLineChartSettingsDialog::SetDefaultVariable(const QString &lVariable)
{
    mDefaultVariable = lVariable;
}

void CharacLineChartSettingsDialog::FillGui()
{
    // Fill condition widget
    FillConditionWidget();

    // Fill series widget
    FillSeriesWidget();

    // Set variable
    int lIndex = -1;

    if (mTemplate.GetVariable().isEmpty() == false)
        lIndex = mUi->comboBoxVariable->findData(mTemplate.GetVariable());

    if (lIndex == -1 && mDefaultVariable.isEmpty())
        lIndex = mUi->comboBoxVariable->findData(mDefaultVariable);

    if (lIndex != -1)
        mUi->comboBoxVariable->setCurrentIndex(lIndex);
    else
        mUi->comboBoxVariable->setCurrentIndex(0);

    // Update button status
    UpdateUI();

    // Connect Signal/Slots
    connect(mUi->treeWidgetConditions,  SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this,                       SLOT(OnConditionClicked()));
    connect(mUi->treeWidgetSeries,      SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this,                       SLOT(OnSerieDoubleClicked(QTreeWidgetItem*,int)));
}

void CharacLineChartSettingsDialog::accept()
{
    FillTemplateFromGUI();

    QDialog::accept();
}

void CharacLineChartSettingsDialog::FillConditionWidget()
{
    mUi->treeWidgetConditions->clear();

    foreach(const QString &condition, mConditionsLevel)
    {
        QTreeWidgetItem * pItem = new QTreeWidgetItem(mUi->treeWidgetConditions);

        pItem->setText(0, condition);

        if (mTemplate.GetSerieConditions().contains(condition))
            pItem->setCheckState(0, Qt::Checked);
        else
            pItem->setCheckState(0, Qt::Unchecked);
    }
}

void CharacLineChartSettingsDialog::FillSeriesWidget()
{
    mUi->treeWidgetSeries->clear();
    mUi->treeWidgetSeries->setColumnCount(1);

    // Fill Headers
    QStringList lSerieLabels("Color");

    lSerieLabels.append(mTemplate.GetSerieConditions());
    mUi->treeWidgetSeries->setHeaderLabels(lSerieLabels);

    // Fill series values
    for (int lItem = 0; lItem < mTemplate.GetSerieDefinitionCount(); ++lItem)
    {
        QTreeWidgetItem * pItem = new QTreeWidgetItem(mUi->treeWidgetSeries);

        // Create pixmap to show serie color
        QPixmap pixmap(10,10);
        pixmap.fill(mTemplate.GetSerieDefinitionAt(lItem).GetColor());

        pItem->setIcon(0, pixmap);
        pItem->setData(0, SERIE_COLOR_ROLE, mTemplate.GetSerieDefinitionAt(lItem).GetColor());

        for(int lCol = 0; lCol < mTemplate.GetSerieDefinitionAt(lItem).GetConditions().count(); ++lCol)
            pItem->setText(lCol+1, mTemplate.GetSerieDefinitionAt(lItem).GetConditions().at(lCol));
    }

    // Adjust column width to contents
    for (int lIdx = 0; lIdx < mUi->treeWidgetSeries->columnCount(); ++lIdx)
        mUi->treeWidgetSeries->resizeColumnToContents(lIdx);
}

void CharacLineChartSettingsDialog::FillDefaultTemplate()
{
    QTreeWidgetItem *   pItem = NULL;
    QStringList         lSerieConditions;

    mTemplate.Clear();

    // Get Conditions used as a serie
    for (int lIdx = 0; lIdx < mUi->treeWidgetConditions->topLevelItemCount(); ++lIdx)
    {
        pItem = mUi->treeWidgetConditions->topLevelItem(lIdx);

        if (pItem->checkState(0) == Qt::Checked)
            lSerieConditions.append(pItem->text(0));
    }

    mTemplate.SetSerieCondition(lSerieConditions);

    // Get the variable to draw
    int     lIndex = mUi->comboBoxVariable->currentIndex();

    if (lIndex >= 0)
        mTemplate.SetVariable(mUi->comboBoxVariable->itemData(lIndex).toString());

    if (lSerieConditions.count() > 0)
    {
        // Create the series to add to the template
        QStringList serieKey;

        for (int lItem = 0; lItem < mConditionsValues.count(); ++lItem)
        {
            QString     key;
            QStringList conditionValues;

            foreach(const QString &serie, mTemplate.GetSerieConditions())
            {
                key.append(mConditionsValues.at(lItem).value(serie));
            }

            // Serie not existing, so create it
            if (serieKey.indexOf(key) == -1)
            {
                // Append the key to the list
                serieKey.append(key);

                for(int lIdx = 0; lIdx < mTemplate.GetSerieConditions().count(); ++lIdx)
                    conditionValues.append(mConditionsValues.at(lItem).value(mTemplate.GetSerieConditions().at(lIdx)));

                // Fill serie definition
                CharacLineChartSerie chartSerie;

                chartSerie.SetColor(CGexReport::GetChartingColor(serieKey.count()));
                chartSerie.SetVariable(mDefaultVariable);
                chartSerie.SetConditions(conditionValues);

                mTemplate.AddSerieDefinition(chartSerie);
            }
        }
    }
}

void CharacLineChartSettingsDialog::FillTemplateFromGUI()
{
    QTreeWidgetItem *   pItem = NULL;
    QStringList         lSerieConditions;

    mTemplate.Clear();

    // Get Conditions used as a serie
    for (int lIdx = 0; lIdx < mUi->treeWidgetConditions->topLevelItemCount(); ++lIdx)
    {
        pItem = mUi->treeWidgetConditions->topLevelItem(lIdx);

        if (pItem->checkState(0) == Qt::Checked)
            lSerieConditions.append(pItem->text(0));
    }

    mTemplate.SetSerieCondition(lSerieConditions);

    // Get the variable to draw
    int     lIndex = mUi->comboBoxVariable->currentIndex();

    if (lIndex >= 0)
        mTemplate.SetVariable(mUi->comboBoxVariable->itemData(lIndex).toString());

    // Get Serie definition
    for (int lIdx = 0; lIdx < mUi->treeWidgetSeries->topLevelItemCount(); ++lIdx)
    {
        pItem = mUi->treeWidgetSeries->topLevelItem(lIdx);

        CharacLineChartSerie serie;
        QStringList lSerieValues;
        QVariant    var         = pItem->data(0, SERIE_COLOR_ROLE);
        QColor      color;

        if (var.isValid())
            color = var.value<QColor>();

        for (int lCol = 1; lCol < mUi->treeWidgetSeries->columnCount(); ++lCol)
            lSerieValues.append(pItem->text(lCol));

        serie.SetColor(color);
        serie.SetConditions(lSerieValues);

        mTemplate.AddSerieDefinition(serie);
    }
}

void CharacLineChartSettingsDialog::UpdateUI()
{
    QPushButton * pOkButton = mUi->buttonBox->button(QDialogButtonBox::Ok);

    if (pOkButton)
    {
        if (mTemplate.GetSerieConditions().count() == 0 ||
            mTemplate.GetSerieConditions().count() == mConditionsLevel.count())
            pOkButton->setEnabled(false);
        else
            pOkButton->setEnabled(true);
    }
}

void CharacLineChartSettingsDialog::OnConditionClicked()
{
    // Update template
    FillDefaultTemplate();

    // Update the series widget
    FillSeriesWidget();

    // Update button status
    UpdateUI();
}

void CharacLineChartSettingsDialog::OnSerieDoubleClicked(QTreeWidgetItem * pItem,
                                                         int lColumn)
{
    if (lColumn == 0)
    {
        QColor      newColor;
        QColor      currentColor;
        QVariant    var         = pItem->data(0, SERIE_COLOR_ROLE);

        if (var.isValid())
            currentColor = var.value<QColor>();

        newColor = QColorDialog::getColor(currentColor, this);

        if(newColor.isValid() == true)
        {
            // Create pixmap to show Bin color
            QPixmap pixmap(10,10);
            pixmap.fill(newColor);

            pItem->setIcon(0, pixmap);
            pItem->setData(0, SERIE_COLOR_ROLE, newColor);
        }
    }
}

}   // namespace Gex
}   // namespace GS
