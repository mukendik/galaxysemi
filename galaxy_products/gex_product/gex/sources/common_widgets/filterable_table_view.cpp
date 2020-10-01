#include "filterable_table_view.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>

#include "table_view.h"
#include "table_model.h"

namespace GS
{
namespace Gex
{

FilterableTableView::FilterableTableView(const QString &backupFile, QWidget *parent) :
    mTableView(new TableView(backupFile)),
    mLineEditSearchString(0),
    mPushButtonPrec(0),
    mPushButtonNext(0),
    mComboFilterSearch(0),
    mComboFilterTarget(0),
    mLabelSearch(0),
    mLabelRows(0),
    QWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    layout()->setContentsMargins(0,0,0,0);
}

FilterableTableView::~FilterableTableView()
{
    if (mTableView)
    {
        delete mTableView;
        mTableView = 0;
    }
}

TableView *FilterableTableView::GetTableView() const
{
    return mTableView;
}

FilterableTableView* FilterableTableView::CreateView(
        QMap<int , QPair<QString, QString> > columnFiltered,
        int currentIndex)
{
    mTableView->InitializeUI();
    QWidget* lSearchBar = CreateSearchBar(columnFiltered, currentIndex);

    layout()->addWidget(mTableView);
    layout()->addWidget(lSearchBar);

    connect(mLineEditSearchString, SIGNAL(textChanged(QString)),
            this, SLOT(OnSearchTextChanged(QString)));
    connect(mComboFilterTarget, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnSearchTargetChanged(int)));
    connect(mComboFilterSearch, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnSearchFilterChoiceChanged(int)));
    connect(mPushButtonNext, SIGNAL(clicked(bool)),
            this, SLOT(NextMatchClicked()));
    connect(mPushButtonPrec, SIGNAL(clicked(bool)),
            this, SLOT(PrecMatchClicked()));

    return this;
}

QWidget *FilterableTableView::CreateSearchBar(
        QMap<int , QPair<QString, QString> > columnFiltered,
        int currentIndex)
{
    QFrame* lFrameSearch = new QFrame(this);
    lFrameSearch->setLayout(new QHBoxLayout(this));

    // Instanciate search controls
    CreateSearchControls(lFrameSearch->layout());

    InitComboFilter(columnFiltered, currentIndex);

    return lFrameSearch;
}

void FilterableTableView::CreateSearchControls(QLayout *layout)
{
    // combo search/filter
    mComboFilterSearch = new QComboBox(layout->widget());
    layout->addWidget(mComboFilterSearch);
    mComboFilterSearch->clear();
    mComboFilterSearch->addItem("search", "search");
    mComboFilterSearch->addItem("filter", "filter");

    // combo target
    mComboFilterTarget = new QComboBox(layout->widget());
    layout->addWidget(mComboFilterTarget);

    // line edit string
    mLineEditSearchString = new QLineEdit(layout->widget());
    mLineEditSearchString->setMaximumWidth(250);
    layout->addWidget(mLineEditSearchString);

    // button prec
    mPushButtonPrec = new QPushButton(layout->widget());
    mPushButtonPrec->setMaximumWidth(20);
    mPushButtonPrec->setMaximumHeight(26);
    mPushButtonPrec->setText("");
    mPushButtonPrec->setIcon(QIcon(":/gex/icons/LeftTriangle.png"));
    mPushButtonPrec->setFlat(true);
    layout->addWidget(mPushButtonPrec);

    // button next
    mPushButtonNext = new QPushButton(layout->widget());
    mPushButtonNext->setMaximumWidth(20);
    mPushButtonNext->setMaximumHeight(26);
    mPushButtonNext->setText("");
    mPushButtonNext->setIcon(QIcon(":/gex/icons/RightTriangle.png"));
    mPushButtonNext->setFlat(true);
    layout->addWidget(mPushButtonNext);

    // label matches
    mLabelSearch = new QLabel(layout->widget());
    mLabelSearch->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    mLabelSearch->setText("          ");
    layout->addWidget(mLabelSearch);

    // label rows
    mLabelRows = new QLabel(layout->widget());
    mLabelRows->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    layout->addWidget(mLabelRows);

    layout->setContentsMargins(0, 0, 0, 0);
}

void FilterableTableView::InitComboFilter(
        QMap<int , QPair<QString, QString> > columnNames,
        int currentIndex)
{
    mComboFilterTarget->clear();
    for (int lIt = 0; lIt < columnNames.size(); ++lIt)
    {
        mComboFilterTarget->addItem(columnNames.value(lIt).first,
                                    columnNames.value(lIt).second);
    }
    mComboFilterTarget->setCurrentIndex(currentIndex);
}

void FilterableTableView::SetModel(QAbstractTableModel* model, int sortedColumn)
{
    if (mTableView)
    {
        mTableView->SetModel(model, sortedColumn);
    }

    RefreshView();
}

void FilterableTableView::RefreshView()
{
    if (!mSearchMatch.isEmpty())
    {
        if (mSearchMatchIndex == 0)
            mPushButtonPrec->setEnabled(false);
        else
            mPushButtonPrec->setEnabled(true);

        if(mSearchMatchIndex == (mSearchMatch.size() - 1))
            mPushButtonNext->setEnabled(false);
        else
            mPushButtonNext->setEnabled(true);
    }
    else
    {
        mPushButtonPrec->setEnabled(false);
        mPushButtonNext->setEnabled(false);
    }

    if (!mTableView->model())
    {
        mPushButtonPrec->setEnabled(false);
        mPushButtonNext->setEnabled(false);
        return;
    }

    // Retrieve search / filter choice
    QString lFilterChoice = mComboFilterSearch->currentData().toString();
    QSortFilterProxyModel* lProxyModel =
            static_cast<QSortFilterProxyModel*>(mTableView->model());
    QString lSearchString = mLineEditSearchString->text().trimmed();

    if (lProxyModel)
    {
        mLabelRows->setText(QString::number(lProxyModel->rowCount()) + " row(s)");
    }

    if (lSearchString.isEmpty() || lFilterChoice == "filter")
    {
        mLabelSearch->setText("");
        mPushButtonNext->setHidden(true);
        mPushButtonPrec->setHidden(true);
    }
    else if (lFilterChoice == "search")
    {
        mLabelSearch->setText(QString::number(mSearchMatch.count()) + " match(es)");
        mPushButtonNext->setHidden(false);
        mPushButtonPrec->setHidden(false);
    }
}

void FilterableTableView::OnSearchTextChanged(QString)
{
    OnSearchStmtChanged();
}

void FilterableTableView::OnSearchTargetChanged(int)
{
    OnSearchStmtChanged();
}

void FilterableTableView::OnSearchFilterChoiceChanged(int)
{
    OnSearchStmtChanged();
}


void FilterableTableView::OnSearchStmtChanged()
{
    QSortFilterProxyModel* lProxyModel =
            static_cast<QSortFilterProxyModel*>(mTableView->model());
    TableModel* lModel = dynamic_cast<TableModel*>(lProxyModel->sourceModel());

    if (!lProxyModel)
        return ;

    // Retrieve search / filter choice
    QString lChoice = mComboFilterSearch->currentData().toString();
    // Retrieve column to impact
    QString lColumn = mComboFilterTarget->currentData().toString();
    // Retrieve string to search / filter
    QString lSearchString = mLineEditSearchString->text().trimmed();

    if (lChoice == "search")
    {
        lProxyModel->setFilterFixedString("");
        mSearchMatchIndex = 0;
        mSearchMatch = mTableView->model()->match(
                    lModel->index(0,lModel->indexOfColumn(lColumn)),
                    Qt::DisplayRole, QVariant(lSearchString),
                    -1,Qt::MatchFlags(Qt::MatchContains));

        if (mSearchMatch.isEmpty())
        {
            RefreshView();
            return;
        }
        mTableView->setCurrentIndex(mSearchMatch.first());
    }
    else
    {
        mSearchMatch.clear();
        mSearchMatchIndex = 0;

        lProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        lProxyModel->setFilterKeyColumn(lModel->indexOfColumn(lColumn));
        lProxyModel->setFilterFixedString(lSearchString);
    }

    RefreshView();
}

void FilterableTableView::NextMatchClicked()
{
    if (mSearchMatch.isEmpty() || (mSearchMatchIndex >= (mSearchMatch.size() - 1)))
        return;

    ++mSearchMatchIndex;
    mTableView->setCurrentIndex(mSearchMatch.at(mSearchMatchIndex));

    RefreshView();
}

void FilterableTableView::PrecMatchClicked()
{
    if (mSearchMatch.isEmpty() || (mSearchMatchIndex <= 0))
        return;

    --mSearchMatchIndex;
    mTableView->setCurrentIndex(mSearchMatch.at(mSearchMatchIndex));

    RefreshView();
}

} // NAMESPACE Gex
} // NAMESPACE GS
