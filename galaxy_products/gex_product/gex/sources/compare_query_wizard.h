///////////////////////////////////////////////////////////
// All classes used for 'Analyze ONE file'
///////////////////////////////////////////////////////////

#ifndef CMPQUERY_WIZARD_H
#define CMPQUERY_WIZARD_H
#include "ui_db_compare_query_dialog.h"
#include "db_onequery_wizard.h"
#include <qdatetime.h>
#include <QComboBox>
#include <QMap>
#include <QString>
#include <QVariant>

#include <stdio.h>

class GexOneQueryWizardPage1;
class GexDatabaseEntry;

class GexCmpQueryWizardPage1 : public QDialog, public Ui::compare_query_basedialog
{
    Q_OBJECT

public:
    GexCmpQueryWizardPage1( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    void ShowPage(void);
    void WriteProcessFilesSection(FILE *hFile, bool bIgnoreStarFilters = true);
    GexDatabaseEntry *FindFirstSqlDataset(QTreeWidgetItem ** ppTreeWidgetItem);
    QString SqlPickParameterList(bool bParametricOnly);

    /**
     * \name FillDataTab
     * \brief fill the data tab with the appropriate files and options.
     *
     * \param dataDescription, a string that contains the description of the section
     */
    bool FillDataTab(const QString& dataDescription);

private:
    void UpdateSkin(int lProductID);
    int totalSelections(void);
    // Add a Query to the list of Queries to compare
    void OnAddQuery(QString &strNewName,QString &strNewDatabase,QString &strNewPeriod,
                    QString &strNewFilters,QString &strNewMinimumPartsPerFile,QString &strNewParts,
                    QString &strNewMappingFile,QString &strNewTestList,QString &strNewDataType,
                    bool bNewOffline,QString &strNewSplitField,QString &strNewPluginOptionsString,
                    GexOneQueryWizardPage1::eExtractionMode eNewExtractionMode, const QString &ExtractionModeParameter);
    void GetQueryFields(GexOneQueryWizardPage1 *pQueryDialog);
    void SetQueryFields(GexOneQueryWizardPage1 *pQueryDialog);
    bool GetFilterSelection(QString &strFilter, QComboBox *pFilter,QComboBox *pFilterString);
    void SetFilterSelection(QString &strFilter, QComboBox *pFilter,QComboBox *pFilterString);
    void WriteFilterScriptLine(GexDatabaseEntry	*pDatabaseEntry,FILE *hFile,QComboBox *pFilter,QComboBox *pFilterString,bool bOfflineQuery, bool bIgnoreStarFilters);
    void WriteSplitFieldScriptLine(GexDatabaseEntry	*pDatabaseEntry,FILE *hFile, const QString& splitFields, bool bOfflineQuery);
    void ReadProcessFilesSection(QString strScriptFile);
    void clearQueries();

    // Overwrite virtual fcts from main class for Drag&Drop support.
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    QString	strDatasetListFile;
    bool	bSortingCapability;	// Disabled (and hidden) under Yield123 as this is of no use.

    // Holds fields when creating a new Query entry..
    QString									strNewName;
    QString									strNewDatabase;
    QString									strNewPeriod;
    QString									strNewFilters;
    QString									strNewParts;
    QString									strNewMappingFile;
    QString									strNewMinimumPartsPerFile;
    QString									strNewTestList;
    QString 								strNewDataType;
    bool									bNewOffline;
    QString									strNewSplitField;
    QString 								strNewPluginOptionsString;
    GexOneQueryWizardPage1::eExtractionMode	eNewExtractionMode;
    QString                                 strExtractionModeParam; // LineEdit near to extractionmode combobox

public slots:

    void	reject(void);		// Called if user hits the ESCAPE key.
    void	OnAddQuery(void);
    void	OnProperties(void);
    void	OnRemoveQuery(void);
    void	OnRemoveAll(void);
    void	OnMoveQueryUp(void);
    void	OnMoveQueryDown(void);
    void	OnDuplicateQuery(void);
    void	onContextualMenu(const QPoint & pos);
    void	OnLoadDatasetList(void);
    void	OnSaveDatasetList(void);
    void	onItemChanged(QTreeWidgetItem*,int);

protected:
    QMap <QString, QMap<QString, QVariant> > m_oGroupConditions;
public:
    QStringList getGroupNameList(bool bAll );

};
#endif
