#ifndef PAT_RECIPE_HISTORICAL_DATA_GUI_H
#define PAT_RECIPE_HISTORICAL_DATA_GUI_H

#include <QDialog>

class GexOneQueryWizardPage1;

namespace Ui {
class PATRecipeHistoricalDataGui;
}

namespace GS
{
namespace Gex
{

class PATRecipeHistoricalDataGui : public QDialog
{
    Q_OBJECT

public:
    explicit PATRecipeHistoricalDataGui(QWidget *parent = 0);
    ~PATRecipeHistoricalDataGui();

    enum DataSource
    {
        DataFromFiles,
        DataFromDatabases
    };

    enum TestKey
    {
        TestNumber,
        TestName,
        TestMix
    };

    DataSource          GetDataSource() const;
    QStringList         GetHistoricalData() const;
    TestKey             GetTestKey() const;

    void                SetTestingStage(const QString& lTestingStage);
    void                SetDataSource(DataSource lSource);
    void                SetTestKey(TestKey lKey);
    void                SetTestKeyEnabled(bool Enable);

protected slots:

    void                OnDataSourceChanged();
    void                OnSelectTestDataFile();
    void                OnRemoveDataFile();
    void                UpdateUI();

protected:

    void                dragEnterEvent(QDragEnterEvent * lEvent);
    void                dropEvent(QDropEvent * lEvent);
    bool                eventFilter(QObject * lObj, QEvent * lEvent);

private:

    Ui::PATRecipeHistoricalDataGui *    mUi;
    GexOneQueryWizardPage1 *            mQueryWidget;
};


}
}

#endif // PAT_RECIPE_HISTORICAL_DATA_GUI_H
