#ifndef STATISTICAL_MONITORING_MANUAL_TESTS_WIDGET_H
#define STATISTICAL_MONITORING_MANUAL_TESTS_WIDGET_H

#include <QWidget>
#include <QDialog>
#include <QItemDelegate>
#include <QList>
#include <QTableView>
#include <QStyledItemDelegate>

#include "manual_limit_item.h"

namespace Ui {
class StatisticalMonitoringManualTestsWidget;
}

class StatisticalMonitoringManualTestsWidget : public QDialog
{
    Q_OBJECT

public:
    ///
    /// \brief default constructor
    ///
    explicit StatisticalMonitoringManualTestsWidget(const QString& numberPlaceHolderLabel, const QString& namePlaceHolderLabel, QWidget *parent = 0);
    ///
    /// \brief default destructor
    ///
    ~StatisticalMonitoringManualTestsWidget();

    void AddItems(QList<QPair<int, QString> > &list);

    ///
    /// \brief Create a list of pair of test number, test name from the items in the UI
    /// \param the output list
    ///
    void GetNewElements(QList<QPair<int, QString> > &list);
private slots:
    void OnDeleteItem(int index);
    void OnAddItem(int num = -2, QString name="");
    void OnOKClicked();
private:
    Ui::StatisticalMonitoringManualTestsWidget *ui;
    QHash<int, ManualLimitItem*> mTests;             // The list of index of the item and a pointer to this item
    ///
    /// \brief Initialize the UI
    ///
    void InitGUI();

    int nextItemIndex;

    QString mNumberPlaceHolderLabel;
    QString mNamePlaceHolderLabel;
};



#endif // STATISTICAL_MONITORING_MANUAL_TESTS_WIDGET_H
