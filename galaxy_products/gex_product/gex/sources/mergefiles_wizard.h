///////////////////////////////////////////////////////////
// All classes used for 'Merge multiple files'
///////////////////////////////////////////////////////////

#ifndef ADDFILES_WIZARD_H
#define ADDFILES_WIZARD_H
#include <qfiledialog.h> 
#include "ui_mergefiles_fileslist_dialog.h"
#include "timeperiod_dialog.h"

class GexAddFileWizardPage1 : public QDialog, public Ui::addfiles_fileslist_basedialog
{
	Q_OBJECT 

public:

	GexAddFileWizardPage1( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
	
	QString strWorkingPath;

	void ShowPage(void);
    void OnAddFiles(QStringList sFilesSelected, bool bClearList=false, QString strNewGroup="", QString strNewSite="",
                    QString strNewParts="", QString strNewRange="", QString strNewMapTests="", QString strTemperature="",
                    QString strWaferId="", QString strDataSetName="");
	void WriteProcessFilesSection(FILE *hFile);
	void ReadProcessFilesSection(QString strScriptFile);

    /**
     * \name FillDataTab
     * \brief fill the data tab with the appropriate files and options.
     *
     * \param dataDescription, a string that contains the description of the section
     */
    bool FillDataTab(const QStringList &dataDescription);

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:

	void UpdateSkin(int lProductID);
	int	totalSelections(void);
	
	// Import files dialog box (calendar type dialog)
	TimePeriodDialog TimePeriodFiles;
    QString strDatasetListFile;
	bool	bDataSetList;

public slots:

	void	reject(void);		// Called if user hits the ESCAPE key.
	void	OnRemoveFiles(void);
	void	OnRemoveAll(void);
	void	OnAddFiles(void);
	void	OnMoveFileUp(void);
	void	OnMoveFileDown(void);
	void	OnTimePeriod(void);
	void	OnProperties(void);
	void	OnLoadDatasetList(void);
	void	OnSaveDatasetList(void);
	void	onContextualMenu(const QPoint & pos);
	void	onItemChanged(QTreeWidgetItem*,int);

};
#endif
