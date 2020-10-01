///////////////////////////////////////////////////////////
// All classes used for 'Interactive drill: What-If
///////////////////////////////////////////////////////////

#ifndef DRILL_TABLE_H
#define DRILL_TABLE_H
#include "ui_drilltable_dialog.h"
#include "ui_interactive_table_find_dialog.h"
#include "gs_types.h"
#include "QTitleFor.h"

#include "loading.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QPair>
#include <QMultiHash>
#include <QFile>
#include <QMenu>

class DrillParametricTable;
class DrillDeviceWidget;

// 'Bin ' statistics labels
#define	DRILL_BIN_GROUP		1	// Group name

// CELL TYPE ?
#define	GEX_DRILL_CELLTYPE_NORMAL		0	// Normal cell: use standard background color
#define	GEX_DRILL_CELLTYPE_ALARM		1	// Alarm cell: use RED background color
#define	GEX_DRILL_CELLTYPE_WARNING		2	// Warning cell: use YELLOW background color
#define	GEX_DRILL_CELLTYPE_USR_COLOR	4	// Cell with custom background color (eg: user defined alarm color)
#define GEX_DRILL_CELLTYPE_ALARM_LIGHT	5	// Cell with test failed and result inside limits

// 'Functional Test' statistics labels
#define	DRILL_FTEST_TNAME	1	// Test name
#define	DRILL_FTEST_GROUP	2	// Group name
#define	DRILL_FTEST_TYPE	3	// Test type
#define	DRILL_FTEST_VNAME	4	// Vector name
#define	DRILL_FTEST_EXEC	5	// Exec count
#define	DRILL_FTEST_FAIL	6	// Fail count
#define	DRILL_FTEST_FAILPERCENT	7	// Fail percentage
#define DRILL_FTEST_TESTFLOWID	8	// Test order in flow (Test flow ID)

class CGexGroupOfFiles;
class CGexFileInGroup;

namespace Gex
{
    class WizardHandler;
}

class DrillTableItem : public QTableWidgetItem
{
    static unsigned sNumOfIntances;
public:
    static unsigned GetNumOfInstances() { return sNumOfIntances; }

    DrillTableItem(int et, const QString &txt, int row = -1 );
    ~DrillTableItem();
    virtual QString& key() const;		// For re-implementing sorting!
    /// \brief This function overload the operator < to sort drillTable
    virtual bool operator<(QTableWidgetItem const &other) const;

    void			paint(int row = -1);

    unsigned char	mFailing;               ///< true if need to paint item with red or yellow background.
    QColor			mCustomBkColor;         ///< Allows user to define custom background color (eg: R&R alarm / Warning)


private:
    mutable QMap<int, QString> mSortedKey;
   //  GexWizardTable* mParentWizard;

};

class GexWizardTable;
class DrillTable : public QTableWidget
{
public:
    /// \brief Statistics Table constructor
    DrillTable(QWidget * parent = 0, Loading* lLoading = 0);
    DrillTable(GexWizardTable *parentWizard , QWidget * parent = 0, Loading* lLoading = 0);

    bool	SortOnAbsoluteValue() const	{ return mSortOnAbsoluteValue; }
    /// \brief Set sorting mode for numeric columns
    void	SetSortOnAbsoluteValue(bool sortOnAbsoluteValue);
    /// \brief Label for Unit column
    static const QString sUnitColumnLabel;

    void    SetClearSelectionAvalaible(bool avalaible) {mClearSelectionAvalaible = avalaible;}

    GexWizardTable* getWizardParent() const { return mParentWizard;}
protected:
    /// \brief This function add the unit column to the cell column
    void checkItemAndAddUnit(DrillTableItem *item);
    virtual void focusOutEvent(QFocusEvent *) { if(mClearSelectionAvalaible) clearSelection();}
    QMap<QString, int>  mColPosition;       ///< Store column position for each given column id

    void   inProgress() { if(mLoading) mLoading->update();}

    GexWizardTable* mParentWizard;
    Loading*        mLoading;
    void keyPressEvent(QKeyEvent *e);

private:
    bool            mSortOnAbsoluteValue;	///< True if sorting on numerical columns should be done using their absolute value
    bool            mClearSelectionAvalaible;

};

class DrillFunctionalTable : public DrillTable
{
public:
    DrillFunctionalTable(GexWizardTable *parentWizard,QWidget * parent = 0, Loading* loading = 0);
    void sortColumn( int col, bool ascending, bool wholeRows );
    void ResetTable(void);
};

class DrillBinningTable : public DrillTable
{
public:
    DrillBinningTable(GexWizardTable *parentWizard, QWidget * parent = 0, Loading* loading =0);
    void sortColumn( int col, bool ascending, bool wholeRows );
    void ResetTable(bool bSoftwareBinning);
};




class FindDialog;
class GexWizardTable :
  public GS::Gex::QTitleFor< QDialog >, public Ui::drill_table_basedialog
{
    Q_OBJECT

public:
    GexWizardTable( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~GexWizardTable();
    Q_INVOKABLE bool isDataAvailable(void);
    //! \brief Load + show relevant Table into Frame (caller is Web link)
    Q_INVOKABLE void ShowTable(QString strLink);
    Q_INVOKABLE void ShowPage(void);
    Q_INVOKABLE void clear(void);
    bool	mChildWindow;		// 'false' if the user has detached the window from the Examinator frame.
    //! \brief Show results for a given run...
    Q_INVOKABLE void OnShowDeviceResults(long lRunOffset, int x=-32768, int y=-32768);

    int                  mIndex;
    bool                    canClearSelection();
    // This tab widget is public as it is removed/added by other classes
    //QWidget *mTabDeviceResults;

    /// \param Contains the list of all parts index of each coordinate ((x,y), list(index of part))
    QHash< QPair<gsint16,gsint16> , QList<gsint32> >    mDevicesByRun;

    /// \param The list of all (x,y) that have to be added to the device combo box
    QList< QPair<gsint16,gsint16> >                     mDevicesInComboBox;

    /// \param All items in the parts combo box
    QList<QString>                                      mItemsPartsComboBox;

    void setWizardHandler(Gex::WizardHandler* lWizardHandler) { mWizardHandler = lWizardHandler; }
    void focusInEvent( QFocusEvent* );

protected:
    /// \brief Close event: if window is detached, this doesn close it but rather reparent it!
    void	closeEvent(QCloseEvent *event);
    void    keyPressEvent(QKeyEvent *event);

public slots:
    void OnTabRenamed(QWidget *widget, const QString &text );
    void	OnSaveParametricTests(void);
    void	OnSaveFunctionalTests(void);
    void	OnSaveSoftBinning(void);
    void	OnSaveHardBinning(void);
    void	OnSaveDeviceResults(void);
    void	OnGetDeviceResults(int lSubLotIndex=-1);
    void	OnInteractiveCharts(void);
    void    OnInteractiveTables(void);
    void	OnInteractiveWafermap(void);
    void	OnExportExcelClipboard(void);
    void	OnExportSelectionExcelClipboard(const QModelIndexList&);
    void	OnContextualMenu(const QPoint&);
    void	OnChangeSortOnAbsoluteValue(bool);
    void    updatePartResultComboBox(int);
    void    updateTypeResultComboBox(int);
    void    OnChangeDeviceResults(int);
    void    FindBoxFinished(int) {SetClearSelectionAvalaible(true);}
    void    SelectionColor(QColor lColor);


    void OnSwitchInteractiveChart();
    void OnSwitchInteractiveWafermap();
protected slots:

    void	OnFilterParameters(bool bState);
    /// \brief Update view with parameter list
    void	onFilterChanged(bool bState);

    void    NextItem                ();
    void    PreviousItem            ();
    void FindTxt(const QString &textEdited);
    //void    ShowFindBox             ();
    void    OnRightCLicked          (const QPoint &pos);

    void    ChangeTabBar            (int indexTab);
    void    RazSelectionColumn      ();
    void    OnTableClicked          (QModelIndex modelIndex);
    void    OnAddCapabilityTablenOnReportBuilder();
    void    OnAddStatisticTablenOnReportBuilder();
    void    reject() { }  // Ignore the ESCAPE key
private:
    /// \brief Save a given table into CSV file
    void	ExportToExcel(QString strCsvFile, QTableWidget  *table);

    /// \brief Write the contents of the table into the file
    void    WriteInExcelFile(QFile& file, QTableWidget *table);

    /// \brief Open the file that has the name passed in parameter
    bool    OpenFile(QString csvFileName, QFile& file);

    /// \brief Write the contents of the table into a big string
    void    WriteTableInString(QString& lBigLine, QTableWidget* table);

    /// \brief Update the map mDevicesByRun
    void    UpdateDevicesByRun();

    void    InitInteractiveView();

    DrillParametricTable *          mTableParametricTests;
    DrillFunctionalTable *          mTableFunctionalTests;
    DrillBinningTable *             mTableSoftBinning;
    DrillBinningTable *             mTableHardBinning;
    QList<DrillDeviceWidget*>       mTableDeviceWidgets;        /// \param Will contain in most cases only 1 element
    bool                            mTablesLoaded;              /// \param
    gsuint8                         mMaxDevicesByRun;           /// \param Contains the max numbers of devices (splitters)
    FindDialog*                     mFindDialog;
    QColor                          mColorSelection;
    Loading*                        mLoading;
    Gex::WizardHandler*             mWizardHandler;
    QList<QTableWidgetItem *>       mListItems;
    QString                         mCurrentText;
    int                             mCurrentIndexTable;
    int                             mCurrentIndexItem;
    DrillTable*                     mCurrentDrillTable;
    bool                            mSelectionMode;
    int                             mCurrentSublotIndex;

    void                        SetSelectedItem             ();
    void                        InitListItemsResearched     ();
    void                        RazSearch                   ();
    void                        InitCurrentDrillTable       ();
    void                        ApplySelectionStylesheet    ();
    void                        SetClearSelectionAvalaible  (bool status);
    void                        HorizontalHeaderClicked     (const QPoint &pos);

    //-- serialize the current table to the json format
    void StatisticTableToJson(QJsonObject &lElt);
    // -- serialize a capability table to the json format
    void CapabiltyTableToJson(QJsonObject &lElt);
};


class FindDialog: public QDialog, public Ui::interactive_find_basedialog
{
    Q_OBJECT

public:
    FindDialog( QWidget* parent = 0);
    ~FindDialog();

    void SetStyleSheet  (bool emptyString);
    void Init           () ;
    void ColorFound     (bool found);
    void DisplayNbOccurences(int aNumOccurence, int aNbTotal);

public slots:
    void ChooseSelectionColor(bool);
    void PopupOption         (bool);

signals:
    void SelectionColor(QColor);

private:
   bool     mIsEmptyStyle;
   QColor   mColorChosen;
   QAction* mSelectionColor;
   QAction* mSearchSettings;
   QMenu    mOptions;
};

#endif
