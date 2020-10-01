#include "pat_recipe_historical_data_gui.h"
#include "ui_pat_recipe_historical_data_gui.h"
#include "browser_dialog.h"
#include "db_onequery_wizard.h"
#include "db_gexdatabasequery.h"
#include "engine.h"
#include "db_engine.h"
#include "admin_gui.h"
#include "gex_shared.h"
#include "import_all.h"
#include "gqtl_log.h"

#include <QShortcut>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

extern GexMainwindow *  pGexMainWindow;

namespace GS
{
namespace Gex
{

PATRecipeHistoricalDataGui::PATRecipeHistoricalDataGui(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    mUi(new Ui::PATRecipeHistoricalDataGui), mQueryWidget(new GexOneQueryWizardPage1())
{
    mUi->setupUi(this);

    GexMainwindow::applyPalette(this);

    mUi->radioButtonFromFiles->setChecked(true);
    mUi->stackedWidget->setCurrentIndex(0);

    // Fill Test Key combo box
    mUi->comboBoxTestKey->clear();
    mUi->comboBoxTestKey->addItem("Test Number", QVariant(TestNumber));
    mUi->comboBoxTestKey->addItem("Test Name", QVariant(TestName));
    mUi->comboBoxTestKey->addItem("Test Number + Test Name", QVariant(TestMix));
    mUi->comboBoxTestKey->setCurrentIndex(0);

    // Call member to resize & hide/show relevant fields when displayed as a PopUp dialog box.
    pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(mQueryWidget->comboBoxDatabaseName, DB_SELECT_FOR_EXTRACTION);
    mQueryWidget->PopupSkin(GexOneQueryWizardPage1::ePATRecipe, false);

    mUi->pageFromDatabases->layout()->addWidget(mQueryWidget);

    mUi->radioButtonFromDatabases->setEnabled(mQueryWidget->comboBoxDatabaseName->count() != 0);

    // Create a key shortcut to remove data fime
    new QShortcut(Qt::Key_Delete, this, SLOT(OnRemoveDataFile()));

    // Install event filter on list widget to catch delete key press
    mUi->listWidgetDataFiles->installEventFilter(this);

    // Enable drops on lineEdit widget
    mUi->listWidgetDataFiles->setAcceptDrops(true);

    // Support for Drag&Drop
    setAcceptDrops(true);

    // Update UI
    UpdateUI();

    connect(mUi->buttonGroupDataSource,     SIGNAL(buttonToggled(QAbstractButton*,bool)),
            this,                           SLOT(OnDataSourceChanged()));
    connect(mUi->listWidgetDataFiles,       SIGNAL(itemSelectionChanged()),
            this,                           SLOT(UpdateUI()));
    connect(mUi->buttonBrowseDataFile,      SIGNAL(clicked()),
            this,                           SLOT(OnSelectTestDataFile()));
    connect(mUi->buttonRemoveDataFile,      SIGNAL(clicked()),
            this,                           SLOT(OnRemoveDataFile()));
}

PATRecipeHistoricalDataGui::~PATRecipeHistoricalDataGui()
{
    delete mUi;
}

PATRecipeHistoricalDataGui::DataSource PATRecipeHistoricalDataGui::GetDataSource() const
{
    if (mUi->radioButtonFromFiles->isChecked())
    {
        return DataFromFiles;
    }
    else
    {
        return DataFromDatabases;
    }
}

QStringList PATRecipeHistoricalDataGui::GetHistoricalData() const
{
    QStringList lHistoricalFiles;

    if (GetDataSource() == DataFromFiles)
    {
        for (int lIdx = 0; lIdx < mUi->listWidgetDataFiles->count(); ++lIdx)
        {
            if (mUi->listWidgetDataFiles->item(lIdx))
                lHistoricalFiles << mUi->listWidgetDataFiles->item(lIdx)->text();
        }
    }
    else
    {
        GexDatabaseQuery lQuery;
        QString lErrorMessage;

        mQueryWidget->GetQueryFields(lQuery);
        lQuery.strTestList = "*";

        lHistoricalFiles = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFiles(&lQuery, lErrorMessage);
    }

    return lHistoricalFiles;
}

PATRecipeHistoricalDataGui::TestKey PATRecipeHistoricalDataGui::GetTestKey() const
{
    return static_cast<PATRecipeHistoricalDataGui::TestKey>(mUi->comboBoxTestKey->currentData().toInt());
}

void PATRecipeHistoricalDataGui::SetTestingStage(const QString &lTestingStage)
{
    // Force testing stage selection
    int lIdxTestingStage = mQueryWidget->comboBoxDatabaseType->findText(lTestingStage);

    if (lIdxTestingStage != -1)
    {
        mQueryWidget->comboBoxDatabaseType->setCurrentIndex(lIdxTestingStage);
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("Unsupported testing stage (%1) requested").arg(lTestingStage).toLatin1().constData());
    }
}

void PATRecipeHistoricalDataGui::OnDataSourceChanged()
{
    if (mUi->radioButtonFromFiles->isChecked())
    {
        mUi->stackedWidget->setCurrentWidget(mUi->pageFromFiles);
    }
    else if (mUi->radioButtonFromDatabases->isChecked())
    {
            mUi->stackedWidget->setCurrentWidget(mUi->pageFromDatabases);
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, "Unsupported data source");
    }

    UpdateUI();
}

void PATRecipeHistoricalDataGui::OnSelectTestDataFile()
{
    SelectDataFiles   lSelectFile;
    QStringList         lDataFiles;

    // Get single Test Data File
    lDataFiles = lSelectFile.GetFiles(this, "", "Select Historical data file");

    if(lDataFiles.isEmpty() == false)
    {
        mUi->listWidgetDataFiles->addItems(lDataFiles);
        UpdateUI();
    }
}

void PATRecipeHistoricalDataGui::OnRemoveDataFile()
{
    QList<QListWidgetItem*> lSelectedItems = mUi->listWidgetDataFiles->selectedItems();

    for (int lIdx = 0; lIdx < lSelectedItems.count(); ++lIdx)
    {
        delete mUi->listWidgetDataFiles->takeItem(mUi->listWidgetDataFiles->row(lSelectedItems.at(lIdx)));
    }

    UpdateUI();
}

void PATRecipeHistoricalDataGui::UpdateUI()
{
    mUi->buttonRemoveDataFile->setEnabled(mUi->listWidgetDataFiles->selectedItems().count() > 0);

    if (GetDataSource() == DataFromFiles && mUi->listWidgetDataFiles->count() == 0)
    {
        mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void PATRecipeHistoricalDataGui::SetDataSource(DataSource lSource)
{
    switch (lSource)
    {
        case DataFromFiles:
            mUi->radioButtonFromFiles->setChecked(true);
            break;

        case DataFromDatabases:
            mUi->radioButtonFromDatabases->setChecked(true);
            break;

        default:
            mUi->radioButtonFromFiles->setChecked(true);
            GSLOG(SYSLOG_SEV_WARNING, "Unknown data source selected, using from files by default");
            break;
    }
}

void PATRecipeHistoricalDataGui::SetTestKey(PATRecipeHistoricalDataGui::TestKey lKey)
{
    int lIndex = mUi->comboBoxTestKey->findData(QVariant(lKey));

    if (lIndex >= 0)
        mUi->comboBoxTestKey->setCurrentIndex(lIndex);
    else
        GSLOG(SYSLOG_SEV_WARNING, "Unknown Test Key selected, using default one");
}

void PATRecipeHistoricalDataGui::SetTestKeyEnabled(bool Enable)
{
    mUi->comboBoxTestKey->setEnabled(Enable);
}

void PATRecipeHistoricalDataGui::dragEnterEvent(QDragEnterEvent *lEvent)
{
    if (GetDataSource() == DataFromFiles)
    {
        // Accept Drag if files list dragged over.
        if((lEvent->mimeData()->formats().contains("text/uri-list")))
        {
            lEvent->acceptProposedAction();
        }
    }
}

void PATRecipeHistoricalDataGui::dropEvent(QDropEvent *lEvent)
{
    if (GetDataSource() == DataFromFiles)
    {
        if((lEvent->mimeData()->formats().contains("text/uri-list")))
        {
            QString     lFile;
            QStringList lFiles;
            QList<QUrl> lUrls = lEvent->mimeData()->urls();

            for (int lIdx = 0; lIdx < lUrls.count(); ++lIdx)
            {
                lFile = lUrls.at(lIdx).toLocalFile();

                if (!lFile.isEmpty())
                    lFiles << lFile;
            }

            // Insert files selected into the listbox
            mUi->listWidgetDataFiles->addItems(lFiles);

            lEvent->acceptProposedAction();

            UpdateUI();
        }
    }
}

bool PATRecipeHistoricalDataGui::eventFilter(QObject *lObj, QEvent *lEvent)
{
    if (lObj == mUi->listWidgetDataFiles)
    {
        if (lEvent->type() == QEvent::KeyRelease)
        {
            QKeyEvent * lKeyEvent = static_cast<QKeyEvent *>(lEvent);

            if (lKeyEvent->key() == Qt::Key_Delete)
            {
                OnRemoveDataFile();
            }

            return true;
        }
    }

    // standard event processing
    return QObject::eventFilter(lObj, lEvent);
}

}
}
