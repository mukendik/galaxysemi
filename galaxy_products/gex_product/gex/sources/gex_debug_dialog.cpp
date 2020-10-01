#include <qlayout.h>
#include <qcombobox.h>
#include "message.h"
#include "gex_shared.h"
#include "gex_debug_dialog.h"
#include "db_engine.h"
#include "browser_dialog.h"
#include "gex_database_entry.h"
#include "admin_gui.h"
#include "engine.h"

// in main.cpp
extern GexMainwindow	*pGexMainWindow;

///////////////////////////////////////////////////////////
// OBJECT: GexDebugDialog
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexDebugDialog::GexDebugDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    connect(pushButtonClose, SIGNAL(clicked()), this, SLOT(accept()));
    connect(comboBoxDatabase, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnDatabaseChanged(const QString &)));
    connect(comboBoxTestingStage, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(OnTestingStageChanged(const QString &)));
    connect(pushButtonGetSplitlots, SIGNAL(clicked()), this, SLOT(OnGetSplitlots()));
    connect(pushButtonPurgeSplitlots, SIGNAL(clicked()), this, SLOT(OnPurgeSplitlots()));
    connect(pushButtonConsolidateWafers, SIGNAL(clicked()), this, SLOT(OnConsolidateWafers()));

    //QObject::connect(comboChartType,	SIGNAL(activated(int)), this, SLOT(OnChartSelection(int)));
    //QObject::connect(buttonOk,			SIGNAL(clicked()),		this, SLOT(accept()));

    // Make sure ExaminatorDB has the latest list of databases (do not repaint Admin GUI page)
    int nDatabasesEntries = pGexMainWindow->ReloadDatabasesList(false);

    // If No database available...display the HTML message instead
    if(nDatabasesEntries == 0)
        return;

    // Fill the combo box with the list of databases
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(comboBoxDatabase,DB_TYPE_SQL|DB_STATUS_CONNECTED);

    // Reset
    Reset();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexDebugDialog::~GexDebugDialog()
{
}

///////////////////////////////////////////////////////////
// Reset
///////////////////////////////////////////////////////////
void GexDebugDialog::Reset()
{
    // Reset variables
    m_pDatabaseEntry = NULL;
    m_bFinalTest = false;
    pushButtonConsolidateWafers->setEnabled(false);

    // Clear controls
    comboBoxTestingStage->clear();
    tableWidget_Results->clearContents();

    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry=NULL;
    QString				strDatabaseLogicalName = comboBoxDatabase->currentText();

    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName);

    // Do nothing if database not found
    if(!pDatabaseEntry || (!pDatabaseEntry->IsExternal()))
        return;

    m_pDatabaseEntry = pDatabaseEntry;

    // For selecting data type to query (wafer sort, or final test,...)
    if(pDatabaseEntry->m_pExternalDatabase->IsTestingStagesSupported())
    {
        QStringList strlSupportedTestingStages;
        pDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(strlSupportedTestingStages);
//        comboBoxTestingStage->insertStringList(strlSupportedTestingStages);
        comboBoxTestingStage->insertItems(strlSupportedTestingStages.count(), strlSupportedTestingStages);
        comboBoxTestingStage->setEnabled(true);
    }

    pushButtonPurgeSplitlots->setEnabled(false);
    pushButtonConsolidateWafers->setEnabled(true);
}

void GexDebugDialog::OnDatabaseChanged(const QString& /*strDatabase*/)
{
    Reset();
}

void GexDebugDialog::OnTestingStageChanged(const QString & strTestingStage)
{
    if(!m_pDatabaseEntry)
        return;

    m_bFinalTest = m_pDatabaseEntry->m_pExternalDatabase->IsTestingStage_FinalTest(comboBoxTestingStage->currentText());

    // Reset column headers and SQL query
    if(m_bFinalTest)
    {
        tableWidget_Results->hideColumn(2);
        plainTextEdit_Query_1->setPlainText(QCoreApplication::translate("Gex_Debug_basedialog", "SELECT\n"
        "  SL.splitlot_id,\n"
        "  SL.lot_id,\n"
        "  SL.tester_type,\n"
        "  SL.nb_parts,\n"
        "  date_format(from_unixtime(SL.start_t),'%Y-%m-%d') AS test_day,\n"
        "  date_format(from_unixtime(SL.insertion_time),'%Y-%m-%d') AS insertion_day,\n"
        "  incremental_update,\n"
        "  SL.file_path,\n"
        "  SL.file_name\n"
        "FROM\n"
        "  ft_splitlot SL\n"
        "", 0, QCoreApplication::UnicodeUTF8));
    }
    else
    {
        tableWidget_Results->showColumn(2);
        plainTextEdit_Query_1->setPlainText(QCoreApplication::translate("Gex_Debug_basedialog", "SELECT\n"
        "  SL.splitlot_id,\n"
        "  SL.lot_id,\n"
        "  SL.wafer_id,\n"
        "  SL.tester_type,\n"
        "  SL.nb_parts,\n"
        "  date_format(from_unixtime(SL.start_t),'%Y-%m-%d') AS test_day,\n"
        "  date_format(from_unixtime(SL.insertion_time),'%Y-%m-%d') AS insertion_day,\n"
        "  incremental_update,\n"
        "  SL.file_path,\n"
        "  SL.file_name\n"
        "FROM\n"
        "  wt_splitlot SL\n"
        "", 0, QCoreApplication::UnicodeUTF8));
    }

    m_strTestingStage = strTestingStage;
}

void GexDebugDialog::OnGetSplitlots(void)
{
    QList<QStringList> listResults;

    label_RowsFilled->setText(QString("0/0"));
    tableWidget_Results->clearContents();
    pushButtonPurgeSplitlots->setEnabled(false);
    m_strlSplitlots.clear();

    if(!m_pDatabaseEntry || !m_pDatabaseEntry->IsExternal() || !m_pDatabaseEntry->m_pExternalDatabase)
        return;

    QString strQuery = plainTextEdit_Query_1->toPlainText();
    strQuery += "\n";
    strQuery += plainTextEdit_Query_2->toPlainText();

    QGuiApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

    if(!m_pDatabaseEntry->m_pExternalDatabase->QuerySQL(strQuery, listResults))
    {
        QString strError;
        m_pDatabaseEntry->m_pExternalDatabase->GetLastError(strError);
        GS::Gex::Message::warning("Error", strError);
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    if(listResults.size() == 0 )
    {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    // Fill result table
    QStringList		strlRecord;
    QString			strCell;
    unsigned int	uiRow, uiColumn, uiTotalRows=listResults.size();

    tableWidget_Results->setRowCount(uiTotalRows);
    for(uiRow = 0; uiRow < uiTotalRows; uiRow++)
    {
        label_RowsFilled->setText(QString(QString::number(uiRow+1)+"/"+QString::number(uiTotalRows)));
        strlRecord = listResults.at(uiRow);
        for(uiColumn= 0 ; uiColumn < (unsigned int)strlRecord.size(); uiColumn++)
        {
            if(uiColumn == 0)
                m_strlSplitlots.append(strlRecord.at(uiColumn));

            if(m_bFinalTest)
            {
                if(uiColumn == 2)
                    // Skip Wafer_ID
                    uiColumn++;
                if(uiColumn > 8)
                    break;
            }
            else
            {
                if(uiColumn > 9)
                    break;
            }

            strCell = strlRecord.at(uiColumn);
            QTableWidgetItem *pNewItem = new QTableWidgetItem(strCell);
            tableWidget_Results->setItem(uiRow, uiColumn, pNewItem);
        }
    }

    QGuiApplication::restoreOverrideCursor();

    pushButtonPurgeSplitlots->setEnabled(true);
}

void GexDebugDialog::OnPurgeSplitlots()
{
    // Make sure DB id OK
    if(!m_pDatabaseEntry || !m_pDatabaseEntry->IsExternal() || !m_pDatabaseEntry->m_pExternalDatabase)
        return;

    // Show Log tab
    tabWidget->setCurrentIndex(3);

    // Purge!!
    QString strCaseTitle = lineEditCaseTitle->text();

    connect(m_pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin, SIGNAL(sLogRichMessage(QString,bool)),
            this, SLOT(UpdateLogMessage(QString,bool)));
    m_pDatabaseEntry->m_pExternalDatabase->PurgeSplitlots(m_strlSplitlots, m_strTestingStage, strCaseTitle);
    disconnect(m_pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin, SIGNAL(sLogRichMessage(QString,bool)),
            this, SLOT(UpdateLogMessage(QString,bool)));
}

void GexDebugDialog::OnConsolidateWafers()
{
    QString strLotID = lineEdit_LotID->text();

    // Make sure DB id OK
    if(!m_pDatabaseEntry || !m_pDatabaseEntry->IsExternal() || !m_pDatabaseEntry->m_pExternalDatabase)
        return;

    // Show Log tab
    tabWidget->setCurrentIndex(3);

    // Consolidate!!
    QString strCaseTitle = lineEditCaseTitle->text();
    connect(m_pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin, SIGNAL(sLogRichMessage(QString,bool)),
            this, SLOT(UpdateLogMessage(QString,bool)));
    m_pDatabaseEntry->m_pExternalDatabase->ConsolidateWafers(strLotID, strCaseTitle);
    disconnect(m_pDatabaseEntry->m_pExternalDatabase->GetPluginID()->m_pPlugin, SIGNAL(sLogRichMessage(QString,bool)),
            this, SLOT(UpdateLogMessage(QString,bool)));
}

void GexDebugDialog::UpdateLogMessage(const QString &message, bool isPlainText)
{
    QString lText = message;

    if(isPlainText)
        textEdit_Log->insertPlainText(lText);
    else
        textEdit_Log->insertHtml(lText);

    textEdit_Log->moveCursor(QTextCursor::End);
    textEdit_Log->ensureCursorVisible();

    QCoreApplication::processEvents();
}

#if 0
void GexDebugDialog::OnGetProducts(void)
{
    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry=NULL;
    QString				strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    // Get product List
    QStringList	strlProductList;

    // Fill external database query object
    GexDbPlugin_Filter clPluginFilter;
    clPluginFilter.strlQueryFilters.clear();
    clPluginFilter.strDataTypeQuery = comboBoxTestingStage->currentText();
    clPluginFilter.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;

    // Execute query on remote database
    pDatabaseEntry->m_pExternalDatabase->QueryProductList(clPluginFilter,strlProductList);
    if(strlProductList.count() == 0)
        return;

    // Fill Filter list with relevant strings
    PickProductIdSQLDialog dPickFilter;
    dPickFilter.fillList(strlProductList, false, false);

    // Prompt dialog box, let user pick Filter string from the list
    strlProductList.clear();
    if(dPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    dPickFilter.getProductList(strlProductList);

    // If a product has been selected, display it
    if(strlProductList.count() > 0)
    {
        lineEditProduct->setText(strlProductList.at(0));
        pushButtonGetBinnings->setEnabled(true);
        pushButtonGetYield->setEnabled(true);
    }
    else
    {
        GS::Gex::Message::warning("GEX debug", "No lots available!");
        pushButtonGetBinnings->setEnabled(false);
        pushButtonGetYield->setEnabled(false);
    }
}

void GexDebugDialog::OnGetLots(void)
{
    // Get database pointer
    GexDatabaseEntry	*pDatabaseEntry=NULL;
    QString				strDatabaseLogicalName = comboBoxDatabase->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    // Get lot List
    QStringList	strlLotList;
    QString		strValue, strFilter;

    // Fill external database query object
    GexDbPlugin_Filter clPluginFilter;
    clPluginFilter.strlQueryFilters.clear();
    clPluginFilter.strDataTypeQuery = comboBoxTestingStage->currentText();
    clPluginFilter.iTimePeriod = GEX_QUERY_TIMEPERIOD_ALLDATES;

    // Set filters
    clPluginFilter.strQueryField = "Lot ID";
    // Add query filter?
    strValue = lineEditLots->text();
    // Include main query field in list of filters (WHERE clause)
    if(!strValue.isEmpty())
    {
        strFilter = "Lot ID";
        strFilter += "=";
        strFilter += strValue;
        clPluginFilter.strlQueryFilters.append(strFilter);
    }

    // Execute query on remote database
    pDatabaseEntry->m_pExternalDatabase->QueryField(clPluginFilter,strlLotList);
    if(strlLotList.count() == 0)
        return;

    // Fill Filter list with relevant strings
    PickProductIdSQLDialog dPickFilter;
    dPickFilter.fillList(strlLotList, false, false);

    // Prompt dialog box, let user pick Filter string from the list
    strlLotList.clear();
    if(dPickFilter.exec() != 1)
        return;	// User 'Abort'

    // Set selected list
    dPickFilter.getProductList(strlLotList);

    // If a product has been selected, display it
    if(strlLotList.count() > 0)
    {
        lineEditLots->setText(strlLotList.at(0));
        pushButtonGetSplitlots->setEnabled(true);
    }
    else
    {
        GS::Gex::Message::warning("GEX debug", "No lots available!");
        pushButtonGetSplitlots->setEnabled(false);
    }
}
#endif
