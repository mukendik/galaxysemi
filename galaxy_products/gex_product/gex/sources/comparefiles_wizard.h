///////////////////////////////////////////////////////////
// All classes used for 'Comparing multiple files'
///////////////////////////////////////////////////////////

#ifndef CMPFILES_WIZARD_H
#define CMPFILES_WIZARD_H
#include <QFileDialog>
#include "ui_comparefiles_fileslist_dialog.h"

#include <QList>

class GexCmpFileWizardPage1 : public QDialog, public Ui::cmpfiles_fileslist_basedialog
{
    Q_OBJECT

public:
    GexCmpFileWizardPage1( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    enum ViewMode {COMPARE,SHIFT};
    void ShowPage(void);
    void SetMode(const ViewMode mode);

    void SplitFileSitesEx(QList <int> *sitesList, QString parts, QString range,
                          QString temperature, QString file, QString mapTests,
                          QString waferId, QString isControlsGroup);
    void OnAddFiles(QStringList filesSelected, bool clearList=false, QString newGroup="",
                    QString newSite="", QString newParts="", QString newRange="",
                    QString newMapTests="", QString temperature="", QString newWaferId = "",
                    QString newType="");
    void WriteProcessFilesSection(FILE *hFile);
    void ReadProcessFilesSection(QString scriptFile);
    QString GetWorkingPath() const;
    void    SetWorkingPath(const QString& path);

    /**
     * \name FillDataTab
     * \brief fill the data tab with the appropriate files and options.
     *
     * \param dataDescription, a string that contains the description of the section
     */
    bool FillDataTab(QStringList &lFileStrings);

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:

    void				UpdateSkin(int lProductID);
    int					totalSelections(void);
    QString				nextDataSetName();

    QTreeWidgetItem *	insertFiles(QTreeWidgetItem * newWidgetItem, QStringList filesSelected,
                                    QString newGroup, QString sites,
                                    QString parts, QString range, QString mapTests,
                                    QString temperature, QString waferId, QString isControlGroup);
    void				SplitFileSites(QList <int> *pSitesList, QString parts, QString range,
                                       QString temperature, QString file, QString mapTests,
                                       QString waferId, QString group ="", QString isControlGroup="");
    QString IsValidInputParameters();

    QString				mDatasetListFile;
    bool				mIsDataSetList;
    bool				mSortingCapability;		// Disabled (and hidden) under Yield123 as this is of no use.
    int					mDataSetNumber;		// Number of dataset inserted
    QString             mWorkingPath;
    ViewMode            mMode;

public slots:

    void				reject(void);		// Called if user hits the ESCAPE key.
    void				OnAddFiles(void);
    void				OnMoveFileUp(void);
    void				OnMoveFileDown(void);
    void				OnProperties(void);
    void				OnRemoveFiles(void);
    void				OnRemoveAll(void);
    void				OnLoadDatasetList(void);
    void				OnSaveDatasetList(void);
    void				onContextualMenu(const QPoint & pos);
    void				onItemChanged(QTreeWidgetItem*,int);
};




#endif
