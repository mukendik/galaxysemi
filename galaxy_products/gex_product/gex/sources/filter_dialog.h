#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include "ui_filter_dialog.h"

#include <QList>
#include <QTreeWidget>

// List of flags for GUI fields enabled/Disabled
#define	GEX_FILE_FILTER_ALLOWSPLIT		1	// Allow spliting per site
#define	GEX_FILE_FILTER_NOSITE			2	// Disable site filtering
#define	GEX_FILE_FILTER_NOGROUPNAME		4	// Disable group name editing
#define	GEX_FILE_FILTER_NOADVANCEDTAB	8	// Disable advanced tab
#define	GEX_FILE_FILTER_ALLOWGROUPTYPE 	16	// Allow group type editing
#define GEX_FILE_FILTER_NOCONTROL       32  // Disable Samples and controls

namespace GS{namespace StdLib{class Stdf;}}

/////////////////////////////////////////////////////////////////////////////
class FilterDialog : public QDialog, public Ui::FilterDialogBase
{
	Q_OBJECT

public:
    FilterDialog(QTreeWidget* pTreeWidgetFiles = 0,
                 QWidget* parent = 0,
                 bool modal = false,
                 Qt::WindowFlags f = 0 );
    ~FilterDialog();

    void	SetParameters(int selectedItems,
                          bool editGroupName,
                          int columnOffset,
                          QTreeWidgetItem * treeWidgetItemSelection,
                          int flags = 0, QStringList sampleGroups=QStringList());
	void	selectFileName(QString &strFileName);
	bool	SpitValidSites(void);
    bool	GetValidSites(QString strDataFile="", bool bQuietMode=true);
	QString GetWaferIdFilter() const;
	void	SetWaferIdFilter(QString strWaferIDFilter);

    QList <int>         mSitesList;

private:    
    void				ReadStringToField(GS::StdLib::Stdf *pStdfFile,char *szField);
    void				GetDataFileHeader(int mSelectedItems);

    QTreeWidget		*	mTreeWidgetFiles;
    QTreeWidgetItem	*	mTreeWidgetItemFileFilter;
    int					mColumnOffset;          ///< Tells which first column has the info we look for
    int					mSelectedItems;         ///< Number of STDF files selected in the list
    QString				mGroupName;             ///< Group name
    QString				mFile;                  ///< Data File name processed
    QString				mStdfFile;              ///< Data File name processed: STDF version in case original name is Not STDF!
    QString				mOriginalFile;          ///< Original file name (as listed in Files list box)
    QString				mMapTests;              ///< .CSV Test Number mapping file
    QString				mWaferIdToFilter;       ///< isEmpty() == true if no wafer id to filter		(case 3935)
    bool				mIsSamplesEdited;       ///< Data file edited.
    bool				mIsGroupNameEditable;   ///< 'true' if user can edit group name...
    bool				mIsGroupTypeEditable;   ///< 'true' if user can edit group type...

public slots:
	virtual void accept();
	virtual void reject();
	void	OnMapTests();
	void	OnCreateMapTests();
	void	OnSuggestDatasetName();
	void	OnPickParts();
	void	OnPickSoftBins();
	void	OnPickHardBins();
	void	OnPickCoord();
	void	OnComboProcessChange(void);
	void	OnComboSitesChange(void);
    void	OnComboGroupTypeChange(void);
};

#endif
