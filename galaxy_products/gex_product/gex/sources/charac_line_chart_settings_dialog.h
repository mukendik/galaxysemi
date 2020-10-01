#ifndef CHARAC_LINE_CHART_SETTINGS_DIALOG_H
#define CHARAC_LINE_CHART_SETTINGS_DIALOG_H

#include "charac_line_chart_template.h"

#include <QDialog>

class QTreeWidgetItem;

namespace Ui {
class CharacLineChartSettingsDialog;
}

namespace GS
{
namespace Gex
{

class CharacLineChartSettingsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit CharacLineChartSettingsDialog(QWidget *parent = 0);
    ~CharacLineChartSettingsDialog();

    const CharacLineChartTemplate&  GetTemplate() const;

    void        SetConditionsLevel(const QStringList &conditionsLevel);
    void        SetConditionsValues(const QList<QMap<QString, QString> >& conditionsValues);
    void        SetTemplate(const CharacLineChartTemplate& lTemplate);
    void        SetDefaultVariable(const QString& lVariable);

    void        FillGui();

public slots:

    void        accept();

protected:

    void        FillConditionWidget();
    void        FillSeriesWidget();
    void        FillDefaultTemplate();
    void        FillTemplateFromGUI();
    void        UpdateUI();

protected slots:

    void        OnConditionClicked();
    void        OnSerieDoubleClicked(QTreeWidgetItem *,int);

private:

    Ui::CharacLineChartSettingsDialog * mUi;
    QStringList                         mConditionsLevel;
    QList<QMap<QString, QString> >      mConditionsValues;
    CharacLineChartTemplate             mTemplate;
    QString                             mDefaultVariable;
};

}   // namespace Gex
}   // namespace GS

#endif // CHARAC_LINE_CHART_SETTINGS_DIALOG_H
