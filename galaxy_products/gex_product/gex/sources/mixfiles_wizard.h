///////////////////////////////////////////////////////////
// All classes used for 'Merge+Compare multiple files'
///////////////////////////////////////////////////////////

#ifndef MIXFILES_WIZARD_H
#define MIXFILES_WIZARD_H
#include <qfiledialog.h>
#include "ui_mixfiles_fileslist_dialog.h"
#include "timeperiod_dialog.h"

class GexMixFileWizardPage1 : public QDialog, public Ui::mixfiles_fileslist_basedialog
{
    Q_OBJECT

public:

    GexMixFileWizardPage1( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );

        void ShowPage(void);
        void UpdateSkin(int lProductID);

    QString strWorkingPath;

    void CreateNewGroup(QString strGroupName);
    void OnAddFiles(QStringList sFilesSelected, bool bClearList=false, QString strNewGroup="",
                    QString strNewSite="", QString strNewParts="", QString strNewRange="",
                    QString strNewMapTests="", QString strTemperature="", QString strNewWaferId="");
    void WriteProcessFilesSection(FILE *hFile);
    void ReadProcessFilesSection(QString strScriptFile);

    /**
     * \name FillDataTab
     * \brief fill the data tab with the appropriate files and options.
     *
     * \param dataDescription, a string that contains the description of the section
     */
    bool FillDataTab(const QStringList &lFileStrings);

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:

    int			totalSelections(void);
    QString		nextDataSetName();

    int	iDataSet;	// Suggests a group ID.
    // Import files dialog box (calendar type dialog)
    TimePeriodDialog TimePeriodFiles;
    QString strDatasetListFile;
    bool	bDataSetList;
    QMap <QString, QMap<QString, QVariant> > m_oGroupConditions;

public slots:

    void	reject(void);		// Called if user hits the ESCAPE key.
    void	OnTimePeriod(void);
    void	OnMoveFileUp(void);
    void	OnMoveFileDown(void);
    void	OnProperties(void);
    void	OnAddFiles(void);
    void	OnRemoveFiles(void);
    void	OnRemoveAll(void);
    void	OnLoadDatasetList(void);
    void	OnSaveDatasetList(void);
    void	onContextualMenu(const QPoint & pos);
    void	OnDoubleClick(void);
    void	OnNewGroup(void);
    void	onItemChanged(QTreeWidgetItem*,int);
public:
    QStringList getGroupNameList(bool bAll);
};


#endif
