#include <gqtl_log.h>
#include <license_provider.h>
#include <product_info.h>

#include "admin_gui.h"
#include "db_engine.h"
#include "db_external_database.h"
#include "engine.h"
#include "gexmo_constants.h"
#include "gex_shared.h"
#include "gex_database_entry.h"
#include "import_all.h"
#include "message.h"
#include "datapump/datapump_task.h"
#include "browser_dialog.h"
#include "mo_scheduler_gui.h"
#include "mo_task_create_gui.h"
#include "pickproduct_id_dialog.h"
#include "qscriptsyntaxhighlighter_p.h"

extern GexMainwindow* pGexMainWindow;
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

GexMoCreateTaskDataPump::GexMoCreateTaskDataPump(int nDatabaseType, QWidget* parent, bool modal,
                                                 Qt::WindowFlags fl) : QDialog(parent, fl), cDataPumpData(this)
{
    setupUi(this);
    setModal(modal);

    tabWidget->setCurrentIndex(0);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(pushButtonBrowse,          SIGNAL(clicked()),      this, SLOT(OnBrowse()));
    QObject::connect(checkBoxExecutionWindow,   SIGNAL(clicked()),      this, SLOT(OnExecutionWindow()));
    QObject::connect(buttonBox_OkCancel,        SIGNAL(accepted()),     this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,        SIGNAL(rejected()),     this, SLOT(OnCancel()));
    QObject::connect(groupBoxYieldMonitoring,   SIGNAL(clicked()),      this, SLOT(OnYieldMonitoring()));
    QObject::connect(groupBoxPreInsertionScript,SIGNAL(clicked()),      this, SLOT(OnPreInsertion()));
    QObject::connect(groupBoxPostInsertionScript,SIGNAL(clicked()),     this, SLOT(OnPostInsertion()));
    QObject::connect(pushButtonMailingList,     SIGNAL(clicked()),      this, SLOT(OnMailingList()));
    QObject::connect(comboBoxTaskFrequency,     SIGNAL(activated(int)), this, SLOT(OnTaskFrequency()));
    QObject::connect(comboBoxDatabaseTarget,    SIGNAL(activated(int)), this, SLOT(OnDatabaseTarget()));
    QObject::connect(comboDataType,             SIGNAL(activated(int)), this, SLOT(OnDataType()));
    QObject::connect(comboBoxTaskPostImport,    SIGNAL(activated(int)), this, SLOT(OnPostImportMove()));
    QObject::connect(comboBoxTaskPostImportFailure,         SIGNAL(activated(int)), this, SLOT(OnPostImportMove()));
    QObject::connect(comboBoxTaskPostImportDelay,           SIGNAL(activated(int)), this, SLOT(OnPostImportMove()));
    QObject::connect(pushButtonBrowsePostInsertion,         SIGNAL(clicked()),
                     this, SLOT(OnBrowsePostImportFolder()));
    QObject::connect(pushButtonBrowsePostInsertionFailure,  SIGNAL(clicked()),
                     this, SLOT(OnBrowsePostImportFailureFolder()));
    QObject::connect(pushButtonBrowsePostInsertionDelay,    SIGNAL(clicked()),
                     this, SLOT(OnBrowsePostImportDelayFolder()));
    QObject::connect(buttonSelectInsertionBatch,            SIGNAL(clicked()),
                     this, SLOT(OnSelectInsertionBatch()));
    QObject::connect(checkBoxRejectFilesOnPassBinlist,      SIGNAL(clicked()),
                     this, SLOT(OnRejectFilesOnPassBinList()));

    QString strExtensions;
    // Load list of databases
    // For Trigger Pump, no database

    mPreScriptHighlighter.setDocument(mPreInsertionScript->document());
    mPostScriptHighlighter.setDocument(mPostInsertionScript->document());

    mPostInsertionScript->setPlainText(GexMoDataPumpTaskData::sDefaultPostScript);
    if(nDatabaseType>0)
    {
        mPreInsertionScript->setPlainText(GexMoDataPumpTaskData::sDefaultDataPumpPreScript);

        strExtensions=GS::Gex::ConvertToSTDF::GetListOfSupportedFormat().join(";");	// *.stdf;*.wat;*.pcm;*.gdf etc...
        comboBoxDatabaseTarget->clear();
        comboBoxDatabaseTarget->show();
        pGexMainWindow->pWizardAdminGui->ListDatabasesInCombo(comboBoxDatabaseTarget, nDatabaseType);
    }
    else
    {
        // Trigger Pump
        mPreInsertionScript->setPlainText(GexMoDataPumpTaskData::sDefaultPreScript);

        strExtensions="*.gtf; *.js;";
        comboBoxDatabaseTarget->clear();
        comboBoxDatabaseTarget->hide();
        labelDatabaseTarget->hide();
        labelDataType->hide();
        comboDataType->hide();
        comboTestingStage->hide();
        checkBoxExecutionWindow->hide();
        checkBoxExecutionWindow->setChecked(false);
        labelDataPump->setText("Task: Automatic Trigger Execution");
        tabWidget->removeTab(tabWidget->indexOf(tab_Other));
        tabWidget->removeTab(tabWidget->indexOf(tab_Yield));
        // Let s now show pre and post scrit for DataPump and PatPump:
        //tabWidget->removeTab(tabWidget->indexOf(PostinsertionScript));

        labelGeneral->setText(QApplication::translate("monitor_datapump_basedialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Specify general TriggerPump settings:</span></p></body></html>", 0));

        labelHousekeeping->setText(QApplication::translate("monitor_datapump_basedialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Specify what to do with the files after processing by the TriggerPump:</span></p></body></html>", 0));

        TextLabelFrequency->hide();
        comboBoxTaskFrequency->hide();
        TextLabelDayOfWeek->hide();
        comboBoxTaskFrequencyDayOfWeek->hide();
        TextLabelPriority->hide();
        comboBoxTaskPriority->hide();
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
        {
            // With YM_ADMIN_DB: always uploaded
            TextLabelPriority->show();
            comboBoxTaskPriority->show();
        }
        else
        {
            // Without YM_ADMIN_DB: always local
            TextLabelFrequency->show();
            comboBoxTaskFrequency->show();
        }
    }
    bool lEnabled = GS::Gex::Engine::GetInstance().
                        GetAdminEngine().IsActivated();
    tabWidget->setTabEnabled(tabWidget->indexOf(PreinsertionScript),
                             lEnabled);
    tabWidget->setTabEnabled(tabWidget->indexOf(PostinsertionScript),
                             lEnabled);

    // Type of files to import
    lineEditInputFileExtensions->setText(strExtensions);

    // Default Title
    LineEditTitle->setText("Import Data");

    comboBoxTaskPostImport->clear();
    comboBoxTaskPostImport->addItem("Rename processed file to '.read'", GEXMO_POSTIMPORT_RENAME);
    comboBoxTaskPostImport->addItem("Delete data files processed", GEXMO_POSTIMPORT_DELETE);
    comboBoxTaskPostImport->addItem("Move files to folder...", GEXMO_POSTIMPORT_MOVE);
    comboBoxTaskPostImport->addItem("FTP files...", GEXMO_POSTIMPORT_FTP);

    // Default data type is test data
    comboDataType->setCurrentIndex(0);
    comboDataType->setEnabled(false);
    comboTestingStage->setEnabled(false);

    // Load list of tesk frequencies (1minute,2min,3min, 1hour, etc...)
    comboBoxTaskFrequency->clear();
    int nItem = 0;

    while (gexMoLabelTaskFrequency[nItem])
        comboBoxTaskFrequency->addItem(gexMoLabelTaskFrequency[nItem++]);

    // Load list of Days of the week
    comboBoxTaskFrequencyDayOfWeek->clear();

    nItem = 0;
    while (gexMoLabelTaskFrequencyDayOfWeek[nItem])
        comboBoxTaskFrequencyDayOfWeek->addItem(gexMoLabelTaskFrequencyDayOfWeek[nItem++]);

    // Default frequency: every 30 minutes.
    comboBoxTaskFrequency->setCurrentIndex(GEXMO_TASKFREQ_30MIN);

    // Load list of tesk frequencies (1minute,2min,3min, 1hour, etc...)
    comboBoxTaskPriority->clear();
    comboBoxTaskPriority->addItem("Low");
    comboBoxTaskPriority->addItem("Medium");
    comboBoxTaskPriority->addItem("High");

    // Default Priority: Medium.
    comboBoxTaskPriority->setCurrentIndex(1);

    QString strYieldAlarmPercentage;
    comboBoxYieldLevel->clear();
    for(int i=100;i>=0;i--)
    {
        strYieldAlarmPercentage = QString::number(i) + " %";
        comboBoxYieldLevel->addItem(strYieldAlarmPercentage);
    }

    // Bins list to monitor
    lineEditYieldBins->setText("1");

    // Minimum parts required
    lineEditYieldMinimumParts->setText("100");

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lineEditEmailFrom->setText(GEX_EMAIL_PAT_MAN);
    else
        lineEditEmailFrom->setText(GEX_EMAIL_YIELD_MAN);

    // Show/Hide relevant fields
    OnExecutionWindow();
    OnTaskFrequency();
    OnPostImportMove();
    OnYieldMonitoring();
    OnDatabaseTarget();
    OnDataType();
}

void GexMoCreateTaskDataPump::OnTaskFrequency(void)
{
    if(comboBoxTaskFrequency->isHidden())
    {
        TextLabelDayOfWeek->hide();
        comboBoxTaskFrequencyDayOfWeek->hide();
        return;
    }

    switch(comboBoxTaskFrequency->currentIndex())
    {
    case    GEXMO_TASKFREQ_1MIN:            // Task every: 1 minute
    case    GEXMO_TASKFREQ_2MIN:            // Task every: 2 minutes
    case    GEXMO_TASKFREQ_3MIN:            // Task every: 3 minutes
    case    GEXMO_TASKFREQ_4MIN:            // Task every: 4 minutes
    case    GEXMO_TASKFREQ_5MIN:            // Task every: 5 minutes
    case    GEXMO_TASKFREQ_10MIN:           // Task every: 10 minutes
    case    GEXMO_TASKFREQ_15MIN:           // Task every: 15 minutes
    case    GEXMO_TASKFREQ_30MIN:           // Task every: 30 minutes
    case    GEXMO_TASKFREQ_1HOUR:           // Task every: 1 hour
    case    GEXMO_TASKFREQ_2HOUR:           // Task every: 2 hours
    case    GEXMO_TASKFREQ_3HOUR:           // Task every: 3 hours
    case    GEXMO_TASKFREQ_4HOUR:           // Task every: 4 hours
    case    GEXMO_TASKFREQ_5HOUR:           // Task every: 5 hours
    case    GEXMO_TASKFREQ_6HOUR:           // Task every: 6 hours
    case    GEXMO_TASKFREQ_12HOUR:          // Task every: 12 hours
    case    GEXMO_TASKFREQ_1DAY:            // Task every: 1 day
    case    GEXMO_TASKFREQ_2DAY:            // Task every: 2 days
    case    GEXMO_TASKFREQ_3DAY:            // Task every: 3 days
    case    GEXMO_TASKFREQ_4DAY:            // Task every: 4 days
    case    GEXMO_TASKFREQ_5DAY:            // Task every: 5 days
    case    GEXMO_TASKFREQ_6DAY:            // Task every: 6 days
        TextLabelDayOfWeek->hide();
        comboBoxTaskFrequencyDayOfWeek->hide();
        break;

    case    GEXMO_TASKFREQ_1WEEK:           // Task every: 1 week
    case    GEXMO_TASKFREQ_2WEEK:           // Task every: 2 weeks
    case    GEXMO_TASKFREQ_3WEEK:           // Task every: 3 weeks
    case    GEXMO_TASKFREQ_1MONTH:          // Task every: 1 month
        TextLabelDayOfWeek->show();
        comboBoxTaskFrequencyDayOfWeek->show();
        break;
    }
}

void	GexMoCreateTaskDataPump::OnBrowse(void)
{
    QString s;

    // Get current path entered (if any).
    s = LineEditPath->text();

    // If no path define yet, start from default directory.
    if(s.isEmpty() == true)
        s = ".";

    // Popup directory browser
    s = QFileDialog::getExistingDirectory(
                this,
                "Choose a directory (path to data files to import)",
                s,
                QFileDialog::ShowDirsOnly );

    // Check if valid selection
    if(s.isEmpty() == true)
        return;

    // Save folder selected.
    LineEditPath->setText(s);
}

void GexMoCreateTaskDataPump::OnMailingList(void)
{
    // Info message about Mailing list file format.
    GS::Gex::Message::information(
                "", "Mailing list file format:\n\n"
                " o It is an ASCII file\n"
                " o It can hold multiple emails per line\n"
                " o email format is <address>@<domain>\n"
                " o email addresses separator is ';'\n\n");

    QString strMailingList;
    QFileDialog cFileDialog(this);
    strMailingList = cFileDialog.getOpenFileName(this, "Select mailing list", "", "Mailing list *.txt;;All Files (*.*)");
    if(strMailingList.isEmpty() == true)
        return;	// No mailing list file selected...return!

    // Save folder selected.
    lineEditEmailList->setText(strMailingList);
}

void	GexMoCreateTaskDataPump::OnExecutionWindow(void)
{
    if(checkBoxExecutionWindow->isChecked() == true)
    {
        TextLabelStartWindow->show();
        timeEditStart->show();
        TextLabelStopWindow->show();
        timeEditStop->show();
    }
    else
    {
        TextLabelStartWindow->hide();
        timeEditStart->hide();
        TextLabelStopWindow->hide();
        timeEditStop->hide();
    }

    // Refresh Frequency & Day of Week shown/hiddem status
    OnTaskFrequency();
}

void GexMoCreateTaskDataPump::OnPostImportMove(void)
{
    switch(comboBoxTaskPostImport->itemData(comboBoxTaskPostImport->currentIndex()).toInt())
    {
    case GEXMO_POSTIMPORT_RENAME:	// Rename file to ".read".
    case GEXMO_POSTIMPORT_DELETE:	// Delete files after import
        lineEditPostInsertion->hide();
        pushButtonBrowsePostInsertion->hide();
        break;

    case GEXMO_POSTIMPORT_MOVE:		// Move file to custom folder
    case GEXMO_POSTIMPORT_FTP:		// FTP file (in fact, move file to other folder + create trigger file for side FTP application)
        lineEditPostInsertion->show();
        pushButtonBrowsePostInsertion->show();
        break;
    }

    switch(comboBoxTaskPostImportFailure->currentIndex())
    {
    case GEXMO_BAD_POSTIMPORT_RENAME:	// Rename file to ".bad".
        lineEditPostInsertionFailure->hide();
        pushButtonBrowsePostInsertionFailure->hide();
        break;

    case GEXMO_BAD_POSTIMPORT_MOVE:		// Move bad file to custom folder
        lineEditPostInsertionFailure->show();
        pushButtonBrowsePostInsertionFailure->show();
        break;
    }

    switch(comboBoxTaskPostImportDelay->currentIndex())
    {
    case GEXMO_DELAY_POSTIMPORT_LEAVE:	// Leave file".
    case GEXMO_DELAY_POSTIMPORT_RENAME:	// Rename file to ".delay".
        lineEditPostInsertionDelay->hide();
        pushButtonBrowsePostInsertionDelay->hide();
        break;

    case GEXMO_DELAY_POSTIMPORT_MOVE:		// Move delayed file to custom folder
        lineEditPostInsertionDelay->show();
        pushButtonBrowsePostInsertionDelay->show();
        break;
    }
}

void GexMoCreateTaskDataPump::OnBrowsePostImportFolder(void)
{
    QString strString;

    // Get current path entered (if any).
    strString = lineEditPostInsertion->text();

    // If no path define yet, start from default directory.
    if(strString.isEmpty() == true)
        strString = ".";

    // Popup directory browser
    strString = QFileDialog::getExistingDirectory(
                this,
                "Choose a directory (path where to move files)",
                strString,
                QFileDialog::ShowDirsOnly );

    // Check if valid selection
    if(strString.isEmpty() == true)
        return;

    // Save folder selected.
    lineEditPostInsertion->setText(strString);
}

void	GexMoCreateTaskDataPump::OnBrowsePostImportFailureFolder(void)
{
    QString strString;

    // Get current path entered (if any).
    strString = lineEditPostInsertionFailure->text();

    // If no path define yet, start from default directory.
    if(strString.isEmpty() == true)
        strString = ".";

    // Popup directory browser
    strString = QFileDialog::getExistingDirectory(
                this,
                "Choose a directory (path where to move bad files)",
                strString,
                QFileDialog::ShowDirsOnly );

    // Check if valid selection
    if(strString.isEmpty() == true)
        return;

    // Save folder selected.
    lineEditPostInsertionFailure->setText(strString);
}

void GexMoCreateTaskDataPump::OnBrowsePostImportDelayFolder(void)
{
    QString strString;

    // Get current path entered (if any).
    strString = lineEditPostInsertionDelay->text();

    // If no path define yet, start from default directory.
    if(strString.isEmpty() == true)
        strString = ".";

    // Popup directory browser
    strString = QFileDialog::getExistingDirectory(
                this,
                "Choose a directory (path where to move delayed files)",
                strString,
                QFileDialog::ShowDirsOnly );

    // Check if valid selection
    if(strString.isEmpty() == true)
        return;

    // Save folder selected.
    lineEditPostInsertionDelay->setText(strString);
}

void GexMoCreateTaskDataPump::OnRejectFilesOnPassBinList(void)
{
    if(checkBoxRejectFilesOnPassBinlist->isChecked())
        lineEditPassBinlistForRejectTest->setEnabled(true);
    else
        lineEditPassBinlistForRejectTest->setEnabled(false);
}

void GexMoCreateTaskDataPump::OnYieldMonitoring(void)
{
    return;

    if(groupBoxYieldMonitoring->isChecked() == true)
    {
        TextLabelYieldBins->show();
        lineEditYieldBins->show();
        TextLabelYieldLevel->show();
        comboBoxYieldLevel->show();
        TextLabelYieldMinimumParts->show();
        lineEditYieldMinimumParts->show();
        TextLabelEmailList->show();
        lineEditEmailList->show();
        comboBoxMailFormat->show();
        pushButtonMailingList->show();

        // Shown full size
        resize(715,412);
    }
    else
    {
        TextLabelYieldBins->hide();
        lineEditYieldBins->hide();
        TextLabelYieldLevel->hide();
        comboBoxYieldLevel->hide();
        TextLabelYieldMinimumParts->hide();
        lineEditYieldMinimumParts->hide();
        TextLabelEmailList->hide();
        lineEditEmailList->hide();
        comboBoxMailFormat->hide();
        pushButtonMailingList->hide();

        // Shown reduced size
        resize(715,336);
    }
}

void GexMoCreateTaskDataPump::OnPreInsertion()
{
    mPreInsertionScript->setEnabled(groupBoxPreInsertionScript->isChecked());
}

void GexMoCreateTaskDataPump::OnPostInsertion()
{
    mPostInsertionScript->setEnabled(groupBoxPostInsertionScript->isChecked());
}

void GexMoCreateTaskDataPump::OnCancel(void)
{
    done(0);
}

void GexMoCreateTaskDataPump::OnOk(void)
{
    // Get Task title.
    cDataPumpData.strTitle = LineEditTitle->text();
    if(cDataPumpData.strTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    // Get Data source folder path
    cDataPumpData.strDataPath = LineEditPath->text();
    if(cDataPumpData.strDataPath.isEmpty())
    {
        // Need to enter a folder path!
        GS::Gex::Message::
                warning("", "You must specify the test data incoming path!");
        LineEditPath->setFocus();
        return;
    }

    // Get 'process sub-folder' info
    if(checkBoxReadSubFolders->isChecked())
        cDataPumpData.bScanSubFolders = true;
    else
        cDataPumpData.bScanSubFolders = false;

    // Sort files by
    if(comboBoxTaskFileOrder->currentText().contains("FileName",Qt::CaseInsensitive))
        cDataPumpData.eSortFlags = QDir::Name;
    else
        if(!comboBoxTaskFileOrder->currentText().contains("FileName",Qt::CaseInsensitive)
                &&  comboBoxTaskFileOrder->currentText().contains("to newest",Qt::CaseInsensitive))
            cDataPumpData.eSortFlags = (QDir::Time|QDir::Reversed);
        else
            cDataPumpData.eSortFlags = QDir::Time;

    // Define if Import ALL files, or only know extensions
    cDataPumpData.strImportFileExtensions = lineEditInputFileExtensions->text();

    if(!comboBoxDatabaseTarget->isHidden())
    {
        // Get targetted database name,Skip the [Local]/[Server] info
        QString strDatabaseLogicalName = comboBoxDatabaseTarget->currentText();
        if(strDatabaseLogicalName == "Unknown")
        {
            // Need to enter a folder path!
            GS::Gex::Message::warning("", "You must specify a valid database!");
            comboBoxDatabaseTarget->setFocus();
            return;
        }
        if(strDatabaseLogicalName.startsWith("[Local]")
                || strDatabaseLogicalName.startsWith("[Server]"))
            strDatabaseLogicalName = strDatabaseLogicalName.section(" ",1);
        // Get targetted database name,Skip the [Local]/[Server] info
        cDataPumpData.strDatabaseTarget = strDatabaseLogicalName;

        // Get Data type
        cDataPumpData.uiDataType = comboDataType->currentIndex();

        // Get Testing Stage if applicable
        if(cDataPumpData.uiDataType == GEX_DATAPUMP_DATATYPE_WYR)
            cDataPumpData.strTestingStage = comboTestingStage->currentText();
    }
    // Get task Priority
    cDataPumpData.mPriority = comboBoxTaskPriority->currentIndex();

    // Get task Frequency
    cDataPumpData.iFrequency = comboBoxTaskFrequency->currentIndex();

    // Get Day of week to execute task.
    cDataPumpData.iDayOfWeek = comboBoxTaskFrequencyDayOfWeek->currentIndex();

    // Get Time window info.
    if(checkBoxExecutionWindow->isChecked())
        cDataPumpData.bExecutionWindow = true;
    else
        cDataPumpData.bExecutionWindow = false;
    cDataPumpData.cStartTime = timeEditStart->time();
    cDataPumpData.cStopTime = timeEditStop->time();

    // What do to with data files once processed:
    cDataPumpData.iPostImport= comboBoxTaskPostImport->itemData(comboBoxTaskPostImport->currentIndex()).toInt();

    // Save Move/FTP folder
    cDataPumpData.strPostImportMoveFolder= lineEditPostInsertion->text();

    // What do to with BAD data files
    cDataPumpData.iPostImportFailure = comboBoxTaskPostImportFailure->currentIndex();

    // Where to move bad files
    cDataPumpData.strPostImportFailureMoveFolder= lineEditPostInsertionFailure->text();

    // What do to with DELAY data files
    cDataPumpData.iPostImportDelay = comboBoxTaskPostImportDelay->currentIndex();
    // Where to move DELAY files
    cDataPumpData.strPostImportDelayMoveFolder= lineEditPostInsertionDelay->text();

    // Get Yield monitoring info
    if(groupBoxYieldMonitoring->isChecked())
        cDataPumpData.bCheckYield = true;
    else
        cDataPumpData.bCheckYield = false;
    cDataPumpData.strYieldBinsList = lineEditYieldBins->text();
    cDataPumpData.iAlarmLevel = 100 - comboBoxYieldLevel->currentIndex();
    QString strString = lineEditYieldMinimumParts->text();
    cDataPumpData.lMinimumyieldParts = strString.toLong();
    cDataPumpData.strEmailFrom = lineEditEmailFrom->text();
    if ( cDataPumpData.strEmailFrom.isEmpty() == true )
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            cDataPumpData.strEmailFrom = GEX_EMAIL_PAT_MAN;
        else
            cDataPumpData.strEmailFrom = GEX_EMAIL_YIELD_MAN;
    }
    cDataPumpData.strEmailNotify = lineEditEmailList->text();
    if(cDataPumpData.strYieldBinsList.isEmpty() && groupBoxYieldMonitoring->isChecked())
    {
        // Need to enter a folder path!
        GS::Gex::Message::
                warning("", "You must specify the list of Good Bins!");
        lineEditYieldBins->setFocus();
        return;
    }
    cDataPumpData.bHtmlEmail = (comboBoxMailFormat->currentIndex() == 0) ? true : false;

    // Get other options
    cDataPumpData.bRejectSmallSplitlots_NbParts = checkBoxRejectSmallSplitlots_NbParts->isChecked();
    cDataPumpData.uiRejectSmallSplitlots_NbParts = spinBoxRejectSmallSplitlots_NbParts->value();
    cDataPumpData.bRejectSmallSplitlots_GdpwPercent = checkBoxRejectSmallSplitlots_GdpwPercent->isChecked();
    cDataPumpData.lfRejectSmallSplitlots_GdpwPercent = comboBoxRejectSmallSplitlots_GdpwPercent->currentText().remove(QChar('%')).toDouble()/100.0;
    cDataPumpData.bExecuteBatchAfterInsertion = groupBoxExecuteBatch->isChecked();
    cDataPumpData.strBatchToExecuteAfterInsertion = lineEditInsertionBatch->text();
    cDataPumpData.nMaxPartsForTestResultInsertion = spinBoxMaxPartForTestResultInsertion->value();
    cDataPumpData.bRejectFilesOnPassBinlist = checkBoxRejectFilesOnPassBinlist->isChecked();
    cDataPumpData.strPassBinListForRejectTest = lineEditPassBinlistForRejectTest->text();

    cDataPumpData.SetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName, groupBoxPreInsertionScript->isChecked());
    cDataPumpData.SetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName, mPreInsertionScript->toPlainText());

    cDataPumpData.SetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName, groupBoxPostInsertionScript->isChecked());
    cDataPumpData.SetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName, mPostInsertionScript->toPlainText());

    // add statistical agents configuration value to datapump task properties
    const QString tdrLogicalName = comboBoxDatabaseTarget->currentText();

    const QString adrLogicalName =
        GS::Gex::Engine::GetInstance()
        .GetDatabaseEngine()
        .GetADRDBNameFromTDRDBName( tdrLogicalName );

    if( ! adrLogicalName.isEmpty() )
    {
        cDataPumpData.SetAttribute
            ( GexMoDataPumpTaskData::sStatisticalAgentsConfigurationADRAttrName,
              adrLogicalName );
    }


    done(1);
}

void GexMoCreateTaskDataPump::LoadFields(CGexMoTaskDataPump *ptTaskItem)
{
    int iIndex=0;
    // Task Title
    LineEditTitle->setText(ptTaskItem->GetProperties()->strTitle);

    // Data source
    LineEditPath->setText(ptTaskItem->GetProperties()->strDataPath);

    // Process subfolders checkbox
    checkBoxReadSubFolders->setChecked(ptTaskItem->GetProperties()->bScanSubFolders);

    // Sort file by
    for(iIndex=0; iIndex<comboBoxTaskFileOrder->count(); iIndex++)
    {
        if((ptTaskItem->GetProperties()->eSortFlags & QDir::Time)
                && (ptTaskItem->GetProperties()->eSortFlags & QDir::Reversed))
        {
            if(!comboBoxTaskFileOrder->itemText(iIndex).contains("FileName",Qt::CaseInsensitive)
                    &&  comboBoxTaskFileOrder->itemText(iIndex).contains("to newest",Qt::CaseInsensitive))
                break;
        }
        else if(ptTaskItem->GetProperties()->eSortFlags & QDir::Time)
        {
            if(!comboBoxTaskFileOrder->itemText(iIndex).contains("FileName",Qt::CaseInsensitive)
                    && !comboBoxTaskFileOrder->itemText(iIndex).contains("to newest",Qt::CaseInsensitive))
                break;
        }
        else
        {
            if(comboBoxTaskFileOrder->itemText(iIndex).contains("FileName",Qt::CaseInsensitive))
                break;
        }
    }
    comboBoxTaskFileOrder->setCurrentIndex(iIndex);

    // Type of files to import
    lineEditInputFileExtensions->setText(ptTaskItem->GetProperties()->strImportFileExtensions);

    // Targeted database
    if(!comboBoxDatabaseTarget->isHidden())
    {
        QString strDatabaseName;
        QString strDatabaseLogicalName;
        strDatabaseName = ptTaskItem->m_strDatabaseName;
        if(strDatabaseName.isEmpty())
            strDatabaseName = "Unknown";
        for(iIndex=0; iIndex<comboBoxDatabaseTarget->count(); iIndex++)
        {
            strDatabaseLogicalName = comboBoxDatabaseTarget->itemText(iIndex);
            if(strDatabaseLogicalName == strDatabaseName)
                break;
            if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
                strDatabaseLogicalName = strDatabaseLogicalName.section(" ",1);

            if(strDatabaseLogicalName == strDatabaseName)
                break;
        }
        if(iIndex < comboBoxDatabaseTarget->count())
        {
            // Database was found in the list
            comboBoxDatabaseTarget->setCurrentIndex(iIndex);
        }
        else
        {
            // Add a dummy entry
            strDatabaseName = "Unknown";
            comboBoxDatabaseTarget->addItem(QIcon(QString::fromUtf8(":/gex/icons/error.png")), strDatabaseName);
            comboBoxDatabaseTarget->setCurrentIndex(comboBoxDatabaseTarget->count()-1);
        }
    }

    // Data Type
    comboDataType->setCurrentIndex(ptTaskItem->GetProperties()->uiDataType);
    OnDataType();

    // Testing stage
    if(ptTaskItem->GetProperties()->uiDataType == GEX_DATAPUMP_DATATYPE_WYR)
        SetCurrentComboItem(comboTestingStage, ptTaskItem->GetProperties()->strTestingStage);

    // Task priority
    comboBoxTaskPriority->setCurrentIndex(ptTaskItem->GetProperties()->mPriority);

    // Task frequency
    comboBoxTaskFrequency->setCurrentIndex(ptTaskItem->GetProperties()->iFrequency);

    // Day of week
    comboBoxTaskFrequencyDayOfWeek->setCurrentIndex(ptTaskItem->GetProperties()->iDayOfWeek);

    // Starting time window enabled?
    checkBoxExecutionWindow->setChecked(ptTaskItem->GetProperties()->bExecutionWindow);

    // Start, stop time
    timeEditStart->setTime(ptTaskItem->GetProperties()->cStartTime);
    timeEditStop->setTime(ptTaskItem->GetProperties()->cStopTime);

    // What to do after files are processed.
    comboBoxTaskPostImport->setCurrentIndex(comboBoxTaskPostImport->findData(ptTaskItem->GetProperties()->iPostImport));

    // Save Move/FTP folder
    lineEditPostInsertion->setText(ptTaskItem->GetProperties()->strPostImportMoveFolder);

    // What to do with BAD files.
    comboBoxTaskPostImportFailure->setCurrentIndex(ptTaskItem->GetProperties()->iPostImportFailure);

    // Where to move bad files
    lineEditPostInsertionFailure->setText(ptTaskItem->GetProperties()->strPostImportFailureMoveFolder);

    // What to do with DELAY files.
    comboBoxTaskPostImportDelay->setCurrentIndex(ptTaskItem->GetProperties()->iPostImportDelay);
    // Where to move DELAY files
    lineEditPostInsertionDelay->setText(ptTaskItem->GetProperties()->strPostImportDelayMoveFolder);

    // Check yield
    groupBoxYieldMonitoring->setChecked(ptTaskItem->GetProperties()->bCheckYield);

    // bins list to monitor
    lineEditYieldBins->setText(ptTaskItem->GetProperties()->strYieldBinsList);

    // Yield level alarm
    comboBoxYieldLevel->setCurrentIndex(100-ptTaskItem->GetProperties()->iAlarmLevel);

    // Minimum parts to triger alarm
    lineEditYieldMinimumParts->setText(QString::number(ptTaskItem->GetProperties()->lMinimumyieldParts));

    // Email from
    lineEditEmailFrom->setText(ptTaskItem->GetProperties()->strEmailFrom);

    // Emails to notify
    lineEditEmailList->setText(ptTaskItem->GetProperties()->strEmailNotify);

    // Email format: HTML or ASCII
    if(ptTaskItem->GetProperties()->bHtmlEmail)
        comboBoxMailFormat->setCurrentIndex(0);	// HTML (default)
    else
        comboBoxMailFormat->setCurrentIndex(1);	// TEXT

    // Other options
    checkBoxRejectSmallSplitlots_NbParts->setChecked(ptTaskItem->GetProperties()->bRejectSmallSplitlots_NbParts);
    spinBoxRejectSmallSplitlots_NbParts->setValue(ptTaskItem->GetProperties()->uiRejectSmallSplitlots_NbParts);
    checkBoxRejectSmallSplitlots_GdpwPercent->setChecked(ptTaskItem->GetProperties()->bRejectSmallSplitlots_GdpwPercent);
    QString strItemText = QString::number((unsigned int)(ptTaskItem->GetProperties()->lfRejectSmallSplitlots_GdpwPercent*100.0)) + "%";
    int		nItemIndex = comboBoxRejectSmallSplitlots_GdpwPercent->findText(strItemText);
    if (nItemIndex != -1)
        comboBoxRejectSmallSplitlots_GdpwPercent->setCurrentIndex(nItemIndex);
    groupBoxExecuteBatch->setChecked(ptTaskItem->GetProperties()->bExecuteBatchAfterInsertion);
    lineEditInsertionBatch->setText(ptTaskItem->GetProperties()->strBatchToExecuteAfterInsertion);
    spinBoxMaxPartForTestResultInsertion->setValue(ptTaskItem->GetProperties()->nMaxPartsForTestResultInsertion);
    checkBoxRejectFilesOnPassBinlist->setChecked(ptTaskItem->GetProperties()->bRejectFilesOnPassBinlist);
    lineEditPassBinlistForRejectTest->setText(ptTaskItem->GetProperties()->strPassBinListForRejectTest);

    // Priority option
    comboBoxTaskPriority->setCurrentIndex(ptTaskItem->GetProperties()->mPriority);

    // Pre Insertion JS
    groupBoxPreInsertionScript->setChecked(
                ptTaskItem->GetProperties()->GetAttribute(GexMoDataPumpTaskData::sPreScriptActivatedAttrName).toBool() );
    mPreInsertionScript->setPlainText(
                ptTaskItem->GetProperties()->GetAttribute(GexMoDataPumpTaskData::sPreScriptAttrName).toString()
                );

    // Post Insertion JS
    groupBoxPostInsertionScript->setChecked(
                ptTaskItem->GetProperties()->GetAttribute(GexMoDataPumpTaskData::sPostScriptActivatedAttrName).toBool() );
    mPostInsertionScript->setPlainText(
                ptTaskItem->GetProperties()->GetAttribute(GexMoDataPumpTaskData::sPostScriptAttrName).toString()
                );

    // Updates the dialog box size according to the fields visible/hidden.
    OnExecutionWindow();
    OnPostImportMove();
    OnYieldMonitoring();
    OnPreInsertion();
    OnPostInsertion();
    OnRejectFilesOnPassBinList();
    OnDatabaseTarget();
}

void GexMoCreateTaskDataPump::OnDatabaseTarget(void)
{
    if(comboBoxDatabaseTarget->isHidden())
        return;

    TextLabelFrequency->hide();
    comboBoxTaskFrequency->hide();
    comboBoxTaskFrequencyDayOfWeek->hide();
    TextLabelDayOfWeek->hide();
    TextLabelPriority->hide();
    comboBoxTaskPriority->hide();
    checkBoxExecutionWindow->hide();
    // Clear testing stage combo
    comboTestingStage->clear();

    // Get database pointer
    GexDatabaseEntry    *pDatabaseEntry=NULL;
    QString             strDatabaseLogicalName = comboBoxDatabaseTarget->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName,false);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    // Check if SQL database
    if(pDatabaseEntry->IsExternal())
    {
        // SQL database,
        comboDataType->setCurrentIndex(0);
        comboDataType->setEnabled(true);
        comboTestingStage->setEnabled(false);
        checkBoxExecutionWindow->show();
    }
    else
    {
        // Standard file-based database
        comboDataType->setCurrentIndex(0);
        comboDataType->setEnabled(false);
        comboTestingStage->setEnabled(false);
    }

    // Check if stored in DB (for load-balancing)
    if(pDatabaseEntry->IsStoredInDb())
    {
        TextLabelPriority->show();
        comboBoxTaskPriority->show();
        bool lEnabled = GS::Gex::Engine::GetInstance().
                            GetAdminEngine().IsActivated();
        tabWidget->setTabEnabled(tabWidget->indexOf(PreinsertionScript),
                                 lEnabled);
        tabWidget->setTabEnabled(tabWidget->indexOf(PostinsertionScript),
                                 lEnabled);
        OnPreInsertion();
        OnPostInsertion();
    }
    else
    {
        TextLabelFrequency->show();
        comboBoxTaskFrequency->show();
        OnTaskFrequency();
        tabWidget->setTabEnabled(tabWidget->indexOf(PreinsertionScript), false);
        tabWidget->setTabEnabled(tabWidget->indexOf(PostinsertionScript), false);

        //mPreInsertionScript->setEnabled(false);
        //mPreInsertionScript->setPlainText("");
        //mPostInsertionScript->setEnabled(false);
        //mPostInsertionScript->setPlainText("");
    }
}

void GexMoCreateTaskDataPump::OnDataType(void)
{
    if(comboBoxDatabaseTarget->isHidden())
        return;

    // Get database pointer
    GexDatabaseEntry    *pDatabaseEntry=NULL;
    QString             strDatabaseLogicalName = comboBoxDatabaseTarget->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName,false);

    // Do nothing if database not found
    if(!pDatabaseEntry)
        return;

    // Clear testing stage combo
    comboTestingStage->clear();

    // Check if WYR data selected
    if(comboDataType->currentIndex() == 1)
    {
        QStringList strlSupportedTestingStages;
        if(pDatabaseEntry->m_pExternalDatabase)
            pDatabaseEntry->m_pExternalDatabase->GetSupportedTestingStages(strlSupportedTestingStages);
        comboTestingStage->addItems(strlSupportedTestingStages);
        comboTestingStage->setEnabled(true);
        return;
    }

    comboTestingStage->setEnabled(false);
}

void GexMoCreateTaskDataPump::OnSelectInsertionBatch(void)
{
    // Get current file from edit field
    QString strFile = lineEditInsertionBatch->text();

    // Let's user pick the shell to load
    strFile = QFileDialog::getOpenFileName(this, "Select shell...", strFile, "Shell applications/Executables (*)");

    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;

    lineEditInsertionBatch->setText(strFile);
}
