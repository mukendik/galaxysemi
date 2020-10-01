#ifndef TABLE_VIEW_H
#define TABLE_VIEW_H

#include <QTableView>

namespace GS
{
namespace Gex
{

class TableView: public QTableView
{
    Q_OBJECT
public:
    /// \brief Constructor
    explicit TableView(const QString backupFile, QWidget* parent = 0);
    /// \brief Destructor
    virtual ~TableView();
    /// \brief init the table view header, policy..
    void InitializeUI();
    /// \brief Load the table arrangment
    bool LoadState();
    /// \brief Save the state of the table. Order of the column, sort ...
    bool SaveState();
    /// \brief Set tableview backup file name
    void SetBackupStateFile(const QString& name);
    /// \brief Set a model to the table view and add a proxy model for filtering
    void SetModel(QAbstractTableModel *model, int sortedColumn);
    /// \brief Sort data on source update to fix a Qt bug
    void SortData();

private:
    QString             mBackupStateFile;       ///< Holds file name where the state is save
};

}//namespace Gex
}//namespace GS
#endif
