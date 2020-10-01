///////////////////////////////////////////////////////////
// All classes used for 'Interactive drill: What-If
///////////////////////////////////////////////////////////

#ifndef DRILL_WHAT_IF_H
#define DRILL_WHAT_IF_H
#include "ui_drill_guardbanding_dialog.h"
#include "ui_suggestlimits_dialog.h"
#include "gex_group_of_files.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QStyledItemDelegate>
#include <stdio.h>

#define	DRILL_WHAT_IF_RULE_NSIGMA			1
#define DRILL_WHAT_IF_RULE_XPERCENT			2
#define	DRILL_WHAT_IF_RULE_CP_TARGET		3
#define	DRILL_WHAT_IF_RULE_CPK_TARGET		4
#define	DRILL_WHAT_IF_RULE_CP				5
#define	DRILL_WHAT_IF_RULE_CPK				6
// First line written (or expected when reading) in a .CSV What-If file
#define WHAT_IF_CSV_HEADER "What-If Header: DO NOT REMOVE LINE!"
#define WHAT_IF_CSV_HEADER_V6 "What-If Header V6: DO NOT REMOVE LINE!"
#define WHAT_IF_CSV_HEADER_V61 "What-If Header V6.1: DO NOT REMOVE LINE!"

class TableItem : public QTableWidgetItem
{
public:
    TableItem(int et, const QString &txt);

    void            paint();
    void            SetWhatIfFlags(unsigned char flag);
    unsigned char   WhatIfFlags() const;
    //unsigned char   WhatIfPreviousFlags() const;
    virtual QString key() const; // For re-implementing sorting!
private:
    unsigned char mFlags;
    unsigned char mPreviousFlags;
};

class Table : public QTableWidget
{
public:
    Table(QWidget * parent = 0);
    void sortColumn( int col, bool ascending, bool wholeRows );
    void ResetTable(bool bLoadDefaultValues);
};

class WhatIfItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class GexSuggestLimits : public QDialog, public Ui::SuggestLimitsDialogBase
{
    Q_OBJECT

public:

    GexSuggestLimits( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );

    int		GetRuleMode();
    double	GetTarget();

public slots:
    void	OnRuleTypeChange();
};


class GexDrillWhatIf : public QDialog, public Ui::drill_guardbanding_basedialog
{
    Q_OBJECT

public:
    GexDrillWhatIf( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    bool	bChildWindow;		// 'false' if the user has detached the window from the Examinator frame.
    void	ForceAttachWindow(bool bAttach = true, bool bToFront = true);
    void	OnUpdateTableColors(void);
    void	RescaleValue(double *lfLimit,int iPower, char *szScale, char * szTestUnit);
    void	ShowPage(void);

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void	dragEnterEvent(QDragEnterEvent *);
    void	dropEvent(QDropEvent *);

protected:
    // Close event: if window is detached, this doesn't close it but rather reparent it!
    void	closeEvent(QCloseEvent *e);

public slots:
    void	OnComputeWhatIf(void);
    void	OnContextualMenu(const QPoint&);
    void	OnCellChanged( int row, int col);
    void	OnResetTable(void);
    void	OnSnapshot(void);
    void	OnSaveTableToDisk(void);
    void	OnSaveLimitsToMapFile(void);
    void	OnLoadTableFromDisk(void);
    void	OnSaveTableToExcelClipboard(void);
    void	OnSuggestIdealLimits(void);
    void	OnDetachWindow(void);

private:

    enum LimitFileVersion
    {
        versionUnknown = -1,
        versionV5 = 0,
        versionV6,
        versionV61
    };

    Table *TableGuardBanding;
    bool	bResetAllTable;
    GexSuggestLimits *pSuggestLimits;
    void	OnResetOriginalLimits(bool bResetAllLines);
    void	OnLoadTableFromDisk(QString strFile);
    void	loadLimitFile(QTextStream& txtStream, LimitFileVersion eFileVersion);
    void	loadCompatibleLimitFile(QTextStream& txtStream);
    void	formatNewLimit(QString& strString, int nLine, int nType);		// Format new limit columns after editing
    void    ComputeRefValues(int line , CGexFileInGroup *file);
    /**
     * @brief Used at least in what if export map functionnality,
     * where we need to retain only the normalized version of a value,
     * and then forget about its unit (map file so-created will be later
     * used to overwrite the value of something, while keeping at least its unit)
     * i.e.: giving '600' and 'ua' as parameters -> gives back '0.0006' as a result
     * @return void (changes 1st arg in place)
     */
    void normalizeValueAccordingToUnitPrefix(QString& aLocalValue, const QString& aLocalUnit);


private slots:

    void	OnExportExcelClipboard(void);
    void	OnExportSelectionExcelClipboard(void);

};

#endif
