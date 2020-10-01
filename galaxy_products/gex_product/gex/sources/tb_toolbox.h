///////////////////////////////////////////////////////////
// Examinator ToolBox: Convert files page
///////////////////////////////////////////////////////////

#ifndef GEXTB_TOOLBOX_H
#define GEXTB_TOOLBOX_H

#include <QComboBox>
#include "ui_tb_convert_dialog.h"
#include <QTableWidget>
#include <QItemDelegate>
#include "pat_limits_dialog.h"

#include "stdfparse.h"

#define OLD_PARAMETER_NUMBER	5
#define NEW_PARAMETER_NUMBER	6




class GexToolboxTable : public QTableWidget
{
public:
    GexToolboxTable( QWidget * parent = 0);
    void paintCell ( int row, int col );
    int firstSelectedColumn();
    int firstSelectedRow();
};

class GexTbToolBox: public QDialog, public Ui::tb_convert_basedialog
{
    Q_OBJECT
public:
    GexTbToolBox( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~GexTbToolBox();
    void	showWindows(void);

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void	dragEnterEvent(QDragEnterEvent *);
    void	dropEvent(QDropEvent *);
    void	LoadExcelTable(QString strFile="");
    void	ForceAttachWindow(bool bAttach = true, bool bToFront = true);

    // To save old recipe format we need just to move test pinmap and test type near test name at loadind
    // And move them to the their initial position when saving
    void moveTestDetailsColumn();

    bool	mChildWindow;					// 'false' if the user has detached the window from the Examinator frame.
    GexToolboxTable *mExcelTable;			// Table to display & edit text
    long	mLineTestNumberParameter;		// Holds the line# that starts with 'parameter'...in case we have a Examinator-CSV compliant file.
    long	mNbParamHeader;					// Holds the number of parameter header
    long	mFirstTestNumberParameterLine;	// Holds the line# of the FIRST parameter found in the list.
    long	mLastTestNumberParameterLine;	// Holds the line# of the LAST parameter found in the list.
    bool	m_bLinePatternsExists;			// Holds that the line patterns exists in the table
    bool	m_bTableModified;				// Set to true if the recipe changed edited/modified
    bool	m_bNewMajorRelease;				// Set to true if want to force to move from release X.Y to X+1.0
    QString m_strEditFile;					// Holds Data file Edited in Excel-like Table.
    QString m_strToolboxFolder;				// Toolbox Working folder (convert files, etc)

private:
    long	lEditorRows;			// Total lines (Rows) in the editor table
    long	lEditorCols;			// Total Columns in the editor table

    void    Init();
    void	ShowPage(void);
    QComboBox *builtCellComboList(const char *szComboItems[]);
    void	showConvertFields(bool bConvertPage);
    bool	SaveTableToFile(QString strFile,bool bSplitFile=false,
                            bool bAskOverwrite=true,bool bQuietSuccess=false);
    void	UpdateHeaderTitles(void);

    // Test program & project instrumentation...
    bool	ConvertProgramForPat(QString strfile, QString &strErrorMessage);
    bool	ConvertProgramForPat_Image_TL(QString strTestProgram, QString &strErrorMessage);
    bool	ConvertProgramForPat_Image_Load(QString strProjectFile, QString &strErrorMessage);

    // Close event: if window is detached, this doesn close it but rather reparent it!
    void	closeEvent(QCloseEvent *e);
    void	resizeEvent(QResizeEvent *e);

    // Return the text of item if it is not null, else return ""
    QString GetTextFromItem(int row, int col);
    void initExcelTableItemFlags(int row, int col, QTableWidgetItem *item);
    void initExcelTableColumnEditor(int row, int col);

public slots:
    // Convert selected files into CSV or STDF, or test program to PAT...
    void	ConvertFiles(QStringList &sFilesSelected);

    void	reject(void);						// Called if user hits the ESCAPE key.
    void	OnDetachWindow(void);
    void	OnConvertButton(void);
    void	OnProperties(void);
    void	OnSaveButton(void);
    void	OnPivoteTable(void);
    void	OnDelete(bool bLines);			// Delete selected lines or columns
    void	OnSortByPartID(bool bAscending);// Sort Table by PartID
    void	OnTableModified();				// Called each time the table is edited/modified (requires save to disk)
    void	OnTableValueChanged(int,int);	// Table content being changed by user
    void	onCustomContextMenuRequested( const QPoint & pos);	// TreeWidget contextual menu
    void	OnEditChanged(const QString&);	// Edit field being changed by user
    void	OnContextualMenu(const QPoint & pos );						// Table contextual menu
    void	onEditTests(int iOpenMode = 0);
    void	onAddTestBeforeSelected();
    void	onAddTestAfterSelected();
};
#endif
