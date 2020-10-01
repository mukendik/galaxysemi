#ifndef STATISTICAL_MONITORING_VERSIONWIDGET_H
#define STATISTICAL_MONITORING_VERSIONWIDGET_H

#include <QWidget>
#include <QMap>
#include <QDate>


class CGexMoTaskStatisticalMonitoring;

class InfoWidget;
class QSqlTableModel;
class QSortFilterProxyModel;

namespace Ui {
    class StatisticalMonitoringVersionWidget;
}

namespace GS
{
namespace Gex
{
class TableView;


enum E_COLUMN_VERSION
{
    E_Id                = 0,
    E_Version_id        = 1,
    E_DraftVersion      = 2,
    E_Label             = 3,
    E_MatchedProduct    = 4,
    E_SiteMerge         = 5,
    E_Creation          = 6,
    E_Start             = 7,
    E_Expiration        = 8,
    E_ExpirationW       = 9,
    E_ExpirationWDone   = 10,
    E_ComputationFrom   = 11,
    E_ComputationTo     = 12,
    E_None

};

/**
 * \brief The SettingsWidget class provides a widget for letting the user customize the settings of a  task
 */
class StatisticalMonitoringVersionWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit StatisticalMonitoringVersionWidget(CGexMoTaskStatisticalMonitoring* task, const QString &backupFile, QWidget* parent = 0);
        virtual ~StatisticalMonitoringVersionWidget();

        /// \brief Init all the connect and other UI stuff
        void    InitUI(QSqlTableModel *model, bool forceReload);
        /// \brief Delete the selectec version
        void DeleteVersion();
        /// \brief return the selected Id or -1 if none
        int  SelectedId() const;
        void SetInfoWidget(InfoWidget *infoWidget);

signals:
    void OnVersionId      (int versionId);
    /// \brief emit on the deleted version id
    void DeletedVersion (int versionId);

public slots:
        void OnRefresh();
        void OnDateFrom(QDate date);
        void ViewVersion();
        void OnSelectedRow(const QModelIndex&, const QModelIndex&);
        void ChangeFilter(int);

    protected:

        /// \Brief Fill the combo box with the list of column filter
        void FillComboBoxFilter();
        /// \brief filter the view according to the column selected between the date selected
        void OnFilterDate(int);
        /// \brief get the id from the row that hes been selected
        /// \return the id or -1 if invalid selection
        int GetIDFromSelectedRow();
        /// \brief focus on the selected item
        void SetSelectedItem();
        /// \brief update the label in the info widget according to the current selected row
        void UpdateWidgetInfo(int rowSelected);

        QMap<int, E_COLUMN_VERSION>               mDicoColumTable;        ///< Rely the combo filter index to a tupe of version table column
        Ui::StatisticalMonitoringVersionWidget    *mUI;
        CGexMoTaskStatisticalMonitoring           *mTask;
        TableView                                 *mTableView;
        QSqlTableModel                            *mModel;                ///< Holds the sql table version model
        QSortFilterProxyModel                     *mProxyModel;           ///< Enables the column sort
        bool                                      mStateLoaded;           ///< Holds true if state loaded
        bool                                      mIsUiInitialized;       ///< Holds if the UI has already been initialized
        int                                       mSelectedId;            ///< Holds the current selected Id version. -1 is none
        InfoWidget                                *mInfoWidget;           ///< Holds ptr to the info widget
};

}
}



#endif
