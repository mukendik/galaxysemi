///////////////////////////////////////////////////////////
// Examinator Monitoring: Task gui
///////////////////////////////////////////////////////////
#include <QFileDialog>
#include <QGroupBox>

#include <gqtl_log.h>

#include "browser_dialog.h"
#include "import_all.h"
#include "mo_scheduler_gui.h"
#include "status/status_taskdata.h"
#include "mo_task_create_gui.h"
#include "mo_task.h"
#include "converter/converter_task.h"
#include "admin_gui.h"
#include "db_external_database.h"
#include "db_gexdatabasequery.h"
#include "admin_engine.h"
#include "gexmo_constants.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "gex_database_entry.h"
#include "gex_database_filter.h"
#include "engine.h"
#include "product_info.h"
#include "pickproduct_id_dialog.h"
#include "picktest_dialog.h"
#include "message.h"
#include "onefile_wizard.h"
#include "browser.h"

extern GexMainwindow *	pGexMainWindow;
extern CGexSkin* pGexSkin;      // holds the skin settings

// in classes.cpp
extern bool SetCurrentComboItem(QComboBox *pCombo, const QString & strItem, bool bInsertIfMissing=false);

/* Mutate to GexMoDataPumpTaskData static const QString
#define PRE_INSERTION_TEXT " \n"\
        "Enter here any JavaScript codes you'd like to run just before insertion. \n"\
        "Most of the singleton objects available in the Quantix JS API are available.\n"\
        "The key content object to configure this insertion is called 'CurrentGSKeysContent' \n"\
        "Example:\n"\
        "    if (....)\n"\
        "        CurrentGSKeysContent.Set('key', 'value');\n"\
        "Status values (int):\n"\
        "    0 - Passed: File can be inserted\n"\
        "    1 - Failed: File must be rejected (eg: file corrupted)\n"\
        "    2 - FailedValidationStep: File not corrupted but doesn't match with the validation step\n"\
        "    3 - Delay: Process failed but file not corrupted (eg: copy problem, etc), delay insertion to try again later\n"\
        "\n"\
        "CurrentGSKeysContent.Set('Status',0);\n"\
        "CurrentGSKeysContent.Set('Error','');"
*/

/*
 Mutate to GexMoDataPumpTaskData static const QString
#define POST_INSERTION_TEXT " / * \n"\
        "Enter here any JavaScript codes you'd like to run after insertion. \n"\
        "Most of the singleton objects available in the GEX JS API are available. \n"\
*/



///////////////////////////////////////////////////////////
// Reporting task dialog box.
///////////////////////////////////////////////////////////
GexMoCreateTaskFileConverter::GexMoCreateTaskFileConverter( QWidget* parent, bool modal, Qt::WindowFlags fl) : QDialog(parent, fl), cConverterData(this)
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(buttonBox_OkCancel,                    SIGNAL(accepted()),     this, SLOT(OnOk()));
    QObject::connect(buttonBox_OkCancel,                    SIGNAL(rejected()),     this, SLOT(OnCancel()));
    QObject::connect(pushButtonBrowseInputFolder,           SIGNAL(clicked()),      this, SLOT(OnBrowseInputFolder()));
    QObject::connect(pushButtonBrowseOutputFolder,          SIGNAL(clicked()),      this, SLOT(OnBrowseOutputFolder()));
    QObject::connect(pushButtonBrowsePostConversion,        SIGNAL(clicked()),      this, SLOT(OnBrowsePostConversion()));
    QObject::connect(pushButtonBrowsePostConversionFailure, SIGNAL(clicked()),      this, SLOT(OnBrowseFailedConversion()));
    QObject::connect(comboBoxTaskPostConversion,            SIGNAL(activated(int)), this, SLOT(OnPostConversionMode()));
    QObject::connect(comboBoxTaskPostConversionFailure,     SIGNAL(activated(int)), this, SLOT(OnPostConversionMode()));
    QObject::connect(poRunScript, SIGNAL(stateChanged(int)), this, SLOT(runScript(int)));
    QObject::connect(poScriptToRunBrowser, SIGNAL(clicked()), this, SLOT(OnBrowseScriptToRun()));

    // Default Title
    LineEditTitle->setText("File Converter");
    lineEditInputFileExtensions->setText(GS::Gex::ConvertToSTDF::GetListOfSupportedFormat().join(";"));

    // Load list of tesk frequnecies (1minute,2min,3min, 1hour, etc...)
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
    comboBoxTaskFrequency->setCurrentIndex(GEXMO_TASKFREQ_1DAY);

    // Load list of tesk frequencies (1minute,2min,3min, 1hour, etc...)
    comboBoxTaskPriority->clear();
    // Load list of tesk frequencies (1minute,2min,3min, 1hour, etc...)
    comboBoxTaskPriority->clear();
    comboBoxTaskPriority->addItem("Low");
    comboBoxTaskPriority->addItem("Medium");
    comboBoxTaskPriority->addItem("High");

    // Default Priority: Medium.
    comboBoxTaskPriority->setCurrentIndex(1);

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        TextLabelTaskFrequency->hide();
        comboBoxTaskFrequency->hide();
        TextLabelDayOfWeek->hide();
        comboBoxTaskFrequencyDayOfWeek->hide();
        TextLabelPriorityLevel->show();
        comboBoxTaskPriority->show();
    }
    else
    {
        TextLabelTaskFrequency->show();
        comboBoxTaskFrequency->show();
        TextLabelDayOfWeek->show();
        comboBoxTaskFrequencyDayOfWeek->show();
        TextLabelPriorityLevel->hide();
        comboBoxTaskPriority->hide();
    }

    // Refresh GUI: Show/Hide fields
    OnPostConversionMode();
    runScript(0);
}

///////////////////////////////////////////////////////////
// Load fields into GUI
///////////////////////////////////////////////////////////
void GexMoCreateTaskFileConverter::LoadFields(CGexMoTaskConverter *ptTaskItem)
{
    // Load fields into GUI.
    LineEditTitle->setText(ptTaskItem->GetProperties()->strTitle);
    LineEditInputPath->setText(ptTaskItem->GetProperties()->strInputFolder);
    lineEditInputFileExtensions->setText(ptTaskItem->GetProperties()->strFileExtensions);
    LineEditOutputPath->setText(ptTaskItem->GetProperties()->strOutputFolder);
    checkBoxTimeStampName->setChecked(ptTaskItem->GetProperties()->bTimeStampName);
    comboBoxFormat->setCurrentIndex(ptTaskItem->GetProperties()->iFormat);
    comboBoxTaskFrequency->setCurrentIndex(ptTaskItem->GetProperties()->iFrequency);
    comboBoxTaskFrequencyDayOfWeek->setCurrentIndex(ptTaskItem->GetProperties()->iDayOfWeek);
    comboBoxTaskPostConversion->setCurrentIndex(ptTaskItem->GetProperties()->iOnSuccess);
    lineEditPostConversion->setText(ptTaskItem->GetProperties()->strOutputSuccess);
    comboBoxTaskPostConversionFailure->setCurrentIndex(ptTaskItem->GetProperties()->iOnError);
    lineEditPostConversionFailure->setText(ptTaskItem->GetProperties()->strOutputError);

    // Sort file by
    int iIndex;
    QDir::SortFlags eSortFlags = (QDir::SortFlags) ptTaskItem->GetAttribute("SortFlags").toInt();

    for(iIndex=0; iIndex<comboBoxTaskFileOrder->count(); iIndex++)
    {
        if((eSortFlags & QDir::Time)
                && (eSortFlags & QDir::Reversed))
        {
            if(!comboBoxTaskFileOrder->itemText(iIndex).contains("FileName",Qt::CaseInsensitive)
                    &&  comboBoxTaskFileOrder->itemText(iIndex).contains("to newest",Qt::CaseInsensitive))
                break;
        }
        else if(eSortFlags & QDir::Time)
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

    // Priority option
    comboBoxTaskPriority->setCurrentIndex(ptTaskItem->GetProperties()->mPriority);

    comboBoxTaskFrequency->hide();
    comboBoxTaskFrequencyDayOfWeek->hide();
    comboBoxTaskFrequency->setCurrentIndex(GEXMO_TASKFREQ_30MIN);
    comboBoxTaskFrequencyDayOfWeek->setCurrentIndex(0);

    // Refresh GUI: Show/Hide fields
    OnPostConversionMode();
    poRunningMode->setCurrentIndex(ptTaskItem->GetProperties()->GetAttribute("RunningMode").toInt());
    poScriptToRunLinedit->setText(ptTaskItem->GetProperties()->GetAttribute("ScriptToRun").toString());
    poRunScript->setChecked(ptTaskItem->GetProperties()->GetAttribute("RunScript").toBool());
}

///////////////////////////////////////////////////////////
// OK button pressed: Copy GUI info into structure
///////////////////////////////////////////////////////////
void GexMoCreateTaskFileConverter::OnOk()
{
    // Get Task title.
    cConverterData.strTitle = LineEditTitle->text();
    if(cConverterData.strTitle.isEmpty())
    {
        // Need to enter a title!
        GS::Gex::Message::warning("", "You must specify a task title!");
        LineEditTitle->setFocus();
        return;
    }

    // Get Data source folder path
    cConverterData.strInputFolder = LineEditInputPath->text();
    if(cConverterData.strInputFolder.isEmpty())
    {
        // Need to enter a folder path!
        GS::Gex::Message::
                warning("", "You must specify the test data input folder!");
        LineEditInputPath->setFocus();
        return;
    }

    if(poRunScript->isChecked() && poScriptToRunLinedit->text().isEmpty())
    {
        GS::Gex::Message::warning("", "You must specify the script to run!");
        poScriptToRunLinedit->setFocus();
        return;

    }

    // Define if convert ALL files, or only know extensions
    cConverterData.strFileExtensions = lineEditInputFileExtensions->text();

    // Output folder
    cConverterData.strOutputFolder = LineEditOutputPath->text();

    // Output format: STDF or CSV?
    cConverterData.iFormat = comboBoxFormat->currentIndex();

    // Include Timestamp info in file name to create?
    cConverterData.bTimeStampName = checkBoxTimeStampName->isChecked();

    // Action on source files successfuly converted
    cConverterData.iOnSuccess = comboBoxTaskPostConversion->currentIndex();

    // Folder where to move source files (if successfuly converted)
    cConverterData.strOutputSuccess = lineEditPostConversion->text();

    // Action on source files failed conversion
    cConverterData.iOnError = comboBoxTaskPostConversionFailure->currentIndex();

    // Folder where to move source files (if successfuly converted)
    cConverterData.strOutputError = lineEditPostConversionFailure->text();

    // Get task Priority
    cConverterData.mPriority = comboBoxTaskPriority->currentIndex();

    // Get task Frequency
    cConverterData.iFrequency = comboBoxTaskFrequency->currentIndex();

    // Get Day of week to execute task.
    cConverterData.iDayOfWeek = comboBoxTaskFrequencyDayOfWeek->currentIndex();

    // Get task Sort By
    // Sort files by
    QDir::SortFlags eSortFlags;
    if(comboBoxTaskFileOrder->currentText().contains("FileName",Qt::CaseInsensitive))
        eSortFlags = QDir::Name;
    else
        if(!comboBoxTaskFileOrder->currentText().contains("FileName",Qt::CaseInsensitive)
                &&  comboBoxTaskFileOrder->currentText().contains("to newest",Qt::CaseInsensitive))
            eSortFlags = (QDir::Time|QDir::Reversed);
        else
            eSortFlags = QDir::Time;
    cConverterData.SetAttribute("SortFlags", QString::number((int)eSortFlags));

    cConverterData.SetAttribute("RunScript", poRunScript->isChecked());
    cConverterData.SetAttribute("RunningMode",poRunningMode->currentIndex());
    cConverterData.SetAttribute("ScriptToRun",poScriptToRunLinedit->text());

    // Accept
    done(1);
}

///////////////////////////////////////////////////////////
// User cancelled.
///////////////////////////////////////////////////////////
void GexMoCreateTaskFileConverter::OnCancel(void)
{
    done(0);
}
///////////////////////////////////////////////////////////
// Select script file to execute.
///////////////////////////////////////////////////////////
void	GexMoCreateTaskFileConverter::OnBrowseInputFolder(void)
{
    // Get folder
    QString strFolder = LineEditInputPath->text();
    strFolder = QFileDialog::getExistingDirectory(
                this,
                "Select Input folder",
                strFolder,
                QFileDialog::ShowDirsOnly );

    // If no folder selected, ignore command and exit.
    if(strFolder.isEmpty())
        return;

    // Save folder selected.
    LineEditInputPath->setText(strFolder);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void	GexMoCreateTaskFileConverter::OnBrowseOutputFolder(void)
{
    // Get folder
    QString strFolder = LineEditOutputPath->text();
    strFolder = QFileDialog::getExistingDirectory(
                this,
                "Select Output folder",
                strFolder,
                QFileDialog::ShowDirsOnly );

    // If no folder selected, ignore command and exit.
    if(strFolder.isEmpty())
        return;

    // Save folder selected.
    LineEditOutputPath->setText(strFolder);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void	GexMoCreateTaskFileConverter::OnBrowsePostConversion(void)
{
    // Get folder
    QString strFolder = lineEditPostConversion->text();
    strFolder = QFileDialog::getExistingDirectory(
                this,
                "Select where to move files converted",
                strFolder,
                QFileDialog::ShowDirsOnly );

    // If no folder selected, ignore command and exit.
    if(strFolder.isEmpty())
        return;

    // Save folder selected.
    lineEditPostConversion->setText(strFolder);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void	GexMoCreateTaskFileConverter::OnBrowseFailedConversion(void)
{
    // Get folder
    QString strFolder = lineEditPostConversionFailure->text();
    strFolder = QFileDialog::getExistingDirectory(
                this,
                "Select where to move rejected files",
                strFolder,
                QFileDialog::ShowDirsOnly );

    // If no folder selected, ignore command and exit.
    if(strFolder.isEmpty())
        return;

    // Save folder selected.
    lineEditPostConversionFailure->setText(strFolder);
}

///////////////////////////////////////////////////////////
// Refresh GUI: Show/Hide fields
///////////////////////////////////////////////////////////
void	GexMoCreateTaskFileConverter::OnPostConversionMode(void)
{
    switch(comboBoxTaskPostConversion->currentIndex())
    {
    case GEXMO_TASK_CONVERTER_SUCCESS_RENAME:
    case GEXMO_TASK_CONVERTER_SUCCESS_DELETE:
        lineEditPostConversion->hide();
        pushButtonBrowsePostConversion->hide();
        break;

    case GEXMO_TASK_CONVERTER_SUCCESS_MOVE:
        lineEditPostConversion->show();
        pushButtonBrowsePostConversion->show();
        break;
    }


    switch(comboBoxTaskPostConversionFailure->currentIndex())
    {
    case GEXMO_TASK_CONVERTER_FAIL_RENAME:
        lineEditPostConversionFailure->hide();
        pushButtonBrowsePostConversionFailure->hide();
        break;

    case GEXMO_TASK_CONVERTER_FAIL_MOVE:
        lineEditPostConversionFailure->show();
        pushButtonBrowsePostConversionFailure->show();
        break;
    }
}

void GexMoCreateTaskFileConverter::runScript(int)
{
    bool bShow = poRunScript->isChecked();
    poRunningMode->setVisible(bShow);
    poScriptToRunLabel->setVisible(bShow);
    poScriptToRunLinedit->setVisible(bShow);
    poScriptToRunBrowser->setVisible(bShow);
}

void GexMoCreateTaskFileConverter::OnBrowseScriptToRun(void){
    QString strFile = poScriptToRunLinedit->text();

    // Let's user pick the shell to load
    strFile = QFileDialog::getOpenFileName(this, "Select shell...", strFile, "Shell applications/Executables (*)");
    // If no file selected, ignore command.
    if(strFile.isEmpty())
        return;
    poScriptToRunLinedit->setText(strFile);
}






