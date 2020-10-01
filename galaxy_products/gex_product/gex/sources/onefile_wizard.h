///////////////////////////////////////////////////////////
// All classes used for 'Analyze ONE file'
///////////////////////////////////////////////////////////

#ifndef ONEFILE_WIZARD_H
#define ONEFILE_WIZARD_H
#include <qfiledialog.h> 
#include <qevent.h> 
#include "ui_onefile_fileslist_dialog.h"

class GexOneFileWizardPage1 : public QDialog, public Ui::onefile_fileslist_basedialog
{
	Q_OBJECT

public:
	GexOneFileWizardPage1( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
	void ShowPage(void);
	void UpdateSkin(int lProductID);
	void OnRemoveFile(void);
	void WriteProcessFilesSection(FILE *hFile);
	QString strWorkingPath;
	void OnSelectFile(QString & strFileName);
	void SetWaferIDFilter(QString strWaferIdFilter);

    /**
     * \name FillDataTab
     * \brief fill the data tab with the appropriate files and options.
     *
     * \param dataDescription, a string that contains the description of the section
     */
    bool FillDataTab(const QStringList &dataDescription);
	

	bool m_bSplitPerSiteFileSelection;			// set to 'true' if file selected must be split over its testing sites (only apply to 'select one file' Wizard
	QString m_strWaferIdFilter;

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
		void SplitTestingSites(void);

public slots:
	void reject(void);		// Called if user hits the ESCAPE key.
	void OnAddFiles(void);
	void OnProperties(void);
	void onContextualMenu(const QPoint &);
	void onItemChanged(QTreeWidgetItem*,int);
};
#endif
