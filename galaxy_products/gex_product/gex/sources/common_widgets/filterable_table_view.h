#ifndef FILTERABLETABLEVIEW_H
#define FILTERABLETABLEVIEW_H

class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;

#include <QWidget>
#include <QModelIndexList>

namespace GS
{
namespace Gex
{

class TableView;

class FilterableTableView : public QWidget
{
    Q_OBJECT
public:
    explicit FilterableTableView(const QString &backupFile, QWidget *parent = 0);
    ~FilterableTableView();
    TableView* GetTableView()  const;

    FilterableTableView *CreateView(QMap<int, QPair<QString, QString> > columnFiltered, int currentIndex);
    /// \brief init and return the research bar
    QWidget* CreateSearchBar(QMap<int, QPair<QString, QString> > columnFiltered, int currentIndex);
    /// \brief refesh table after update of search and filter
    void OnSearchStmtChanged();
    /// \brief create all search controls
    void CreateSearchControls(QLayout* layout);
    /// \brief initialize the combo filter with specific data
    void InitComboFilter(QMap<int, QPair<QString, QString> > columnNames, int currentIndex);
    /// \brief Refresh table according to filter/search and model
    void RefreshView();

    void SetModel(QAbstractTableModel *model, int sortedColumn);
protected:
    QLabel*                                mLabelSearch;           ///< Holds ptr to label that show match count
    QLabel*                                mLabelRows;             ///< Holds ptr to label that show rows count
    TableView*                             mTableView;             ///< Holds ptr to table view
    QPushButton*                           mPushButtonPrec;        ///< Holds ptr to prec button
    QPushButton*                           mPushButtonNext;        ///< Holds ptr to next button
    QLineEdit*                             mLineEditSearchString;  ///< Holds ptr line edit of search
    QComboBox*                             mComboFilterSearch;     ///< Holds ptr filter/search combo
    QComboBox*                             mComboFilterTarget;     ///< Holds ptr to combo filter target
    QModelIndexList                        mSearchMatch;           ///< Holds list of indexes to search matches
    int                                    mSearchMatchIndex;      ///< Holds index of current index in mSearchMatch

protected slots:
    /// \brief Update table when search text changed
    void OnSearchTextChanged(QString);
    /// \brief Update table when target changed
    void OnSearchTargetChanged(int);
    /// \brief Update table when filter changed
    void OnSearchFilterChoiceChanged(int);
    /// \brief switch to next item matching the search
    void PrecMatchClicked();
    /// \brief switch to prec item matching the search
    void NextMatchClicked();
};

} // NAMESPACE Gex
} // NAMESPACE GS


#endif // FILTERABLETABLEVIEW_H
