
#include <QSortFilterProxyModel>
#include <QAbstractTableModel>
#include <QHeaderView>
#include <QFile>
#include <QDir>

#include "table_view.h"
#include "engine.h"

namespace GS
{
namespace Gex
{

TableView::TableView(const QString backupFile, QWidget* parent):
    mBackupStateFile(backupFile),
    QTableView(parent)
{

}

TableView::~TableView()
{
    if (this->model() && !mBackupStateFile.isEmpty())
    {
        SaveState();
    }
}

bool TableView::LoadState()
{
    QFile lBackupFile(mBackupStateFile);
    // If the selected file is valid
    if (!mBackupStateFile.isEmpty())
    {
        if (!lBackupFile.open(QFile::ReadOnly))
        {
            return false;
        }

        // Send the QByteArray
        bool lStatus = true;
        if(!horizontalHeader()->restoreState(lBackupFile.readAll()))
        {
            horizontalHeader()->reset();
            lStatus = false;
        }
        lBackupFile.close();
        setFocus();
        return lStatus;
    }
    return false;
}

void TableView::InitializeUI()
{
    QHeaderView *lVHeader = verticalHeader();
    lVHeader->sectionResizeMode(QHeaderView::Fixed);
    lVHeader->setDefaultSectionSize(24);
    QHeaderView *lHHeader = horizontalHeader();
    lHHeader->setMovable(true);
    lHHeader->resizeSections(QHeaderView::Fixed);

    setEditTriggers(QAbstractItemView::DoubleClicked);
    setSortingEnabled(true);
    setMouseTracking(true);

    setStyleSheet(
                "QHeaderView::section {"
                    "text-align: center;"
                    "margin-left: 0px;"
                    "margin-right: 0px;"
                    "padding: 0px 0px 10px 0px;"
                "}"
                );

    QSizePolicy lPolicy = sizePolicy();
    lPolicy.setVerticalStretch(250);
    setSizePolicy(lPolicy);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

bool TableView::SaveState()
{
    QFile lBackupFile(mBackupStateFile);

    // Check that the path is valid
    if (!mBackupStateFile.isEmpty())
    {
        if (!lBackupFile.open(QFile::WriteOnly))
        {
            return false;
        }
        // Write contents of view state in file
        lBackupFile.write(horizontalHeader()->saveState());
        // Close the file
        lBackupFile.close();
        return true;
    }
    return false;
}

void TableView::SetBackupStateFile(const QString& name)
{
    mBackupStateFile = GS::Gex::Engine::GetInstance().Get("TempFolder").
            toString() + QDir::separator() + name;
}

void TableView::SetModel(QAbstractTableModel* model, int sortedColumn)
{
    QSortFilterProxyModel* lProxyModel = new QSortFilterProxyModel();
    lProxyModel->setSourceModel(model);
    lProxyModel->setSortRole(Qt::InitialSortOrderRole);
    setModel(lProxyModel);
    sortByColumn(sortedColumn, Qt::AscendingOrder);
    lProxyModel->sort(sortedColumn, Qt::AscendingOrder);
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    LoadState();

    SortData();
}

void TableView::SortData()
{
    // ###########################################
    // ###########################################
    // FIX QTableView/Model BUG
    // We have to change the sort order for it to
    // be taken in account
    // ###########################################
    // retrieving sorted column and order
    int lSortedCol = horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder lSortOrder = horizontalHeader()->sortIndicatorOrder();

    QSortFilterProxyModel* lProxyModel = static_cast<QSortFilterProxyModel*>(model());
    if(lProxyModel)
    {
        if (lSortOrder == Qt::AscendingOrder)
            lProxyModel->sort(lSortedCol, Qt::DescendingOrder);
        else
            lProxyModel->sort(lSortedCol, Qt::AscendingOrder);
        lProxyModel->sort(lSortedCol, lSortOrder);
    }
    lProxyModel->sort(lSortedCol, lSortOrder);
    // ###########################################
    // ###########################################
}

}//namespace Gex
}//namespace GS
