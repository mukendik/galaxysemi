#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <qfiledialog.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <QDragEnterEvent>
#include <QUrl>
#include <QMimeData>
#include "message.h"
#include "engine.h"
#include "import_spektra_dialog.h"
#include "spektra_record_dialog.h"
#include "import_spektra.h"
#include "browser_dialog.h"

ImportSpektraDialog::ImportSpektraDialog( bool bEvaluationMode, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(buttonExport,			SIGNAL(clicked()),		this, SLOT(OnButtonExport()));
    QObject::connect(buttonClose,			SIGNAL(clicked()),		this, SLOT(accept()));
    QObject::connect(buttonSelect,			SIGNAL(clicked()),		this, SLOT(OnButtonSelect()));
    QObject::connect(buttonProperties,		SIGNAL(clicked()),		this, SLOT(OnButtonProperties()));
    QObject::connect(buttonSetAllRecords,	SIGNAL(clicked()),		this, SLOT(OnButtonSetAllRecords()));
    QObject::connect(buttonClearAllRecords, SIGNAL(clicked()),		this, SLOT(OnButtonClearAllRecords()));
    QObject::connect(comboOutputFormat,		SIGNAL(activated(int)), this, SLOT(OnOutputFormatChanged(int)));
    QObject::connect(this,					SIGNAL(sSpektraFileSelected(QString)), editFileName,	SLOT(setText(QString)));
    QObject::connect(this,					SIGNAL(sSpektraFileSelected(QString)), this,			SLOT(OnSpektraFileSelected()));

    m_strSpektraFileName = "";

    // Disable Export button
    buttonExport->setEnabled(false);

    // Disable Properties button
    buttonProperties->setEnabled(false);

    // Check all record buttons
    OnButtonSetAllRecords();

    // Evaluation mode ??
    m_bEvaluationMode = bEvaluationMode;

    // Disable file size limit option in evaluation mode
    if(m_bEvaluationMode)
        comboSizeLimit->setEnabled(false)	;

    // Set default destination format to ASCII
    comboOutputFormat->setCurrentIndex(0);

    // Support for Drag&Drop
    setAcceptDrops(true);
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void ImportSpektraDialog::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void ImportSpektraDialog::dropEvent(QDropEvent *e)
{
    if(!e->mimeData()->formats().contains("text/uri-list"))
    {
        // No files dropped...ignore drag&drop.
        e->ignore();
        return;
    }

    QString		strFileName;
    QStringList strFileList;
    QList<QUrl> lstUrls = e->mimeData()->urls();

    for (int nUrl = 0; nUrl < lstUrls.count(); nUrl++)
    {
        strFileName = lstUrls.at(nUrl).toLocalFile();

        if (!strFileName.isEmpty())
            strFileList << strFileName;
    }

    if(strFileList.count() <= 0)
    {
        // Items dropped are not regular files...ignore.
        e->ignore();
        return;
    }

    // Insert first file selected into the listbox
    QString strFile = strFileList[0];
    OnSelectFile(strFile);

    e->acceptProposedAction();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ImportSpektraDialog::~ImportSpektraDialog()
{
}

///////////////////////////////////////////////////////////
// Button to select a SPEKTRA files to convert has been clicked
///////////////////////////////////////////////////////////
void ImportSpektraDialog::OnButtonSelect()
{
    // Display Open file dialog so the user can select a file
    QString strFileName = QFileDialog::getOpenFileName(this, "Select the SPEKTRA file you want to convert", "", "SPEKTRA files (*.dta)");

    // Validate file selected
    OnSelectFile(strFileName);
}

///////////////////////////////////////////////////////////
// Validate selected (or dragged) file.
///////////////////////////////////////////////////////////
void ImportSpektraDialog::OnSelectFile(QString strFileName)
{
    // Check if new file selected
    if((strFileName.isEmpty() == true) || (m_strSpektraFileName == strFileName))
        return;

    // Make sure the file has a valid SPEKTRA extension
    QFileInfo	fileInfo(strFileName);
//    QString		strExtension = fileInfo.suffix();
    QString		strExtension = fileInfo.suffix();

    if(strExtension != "dta")
    {
        GS::Gex::Message::warning(
            "SPEKTRA file",
            "Invalid file extension.\n"
            "Please select a file with a valid SPEKTRA extension (*.dta)");
        return;
    }

    // Make sure the file is a valid SPEKTRA file
    CSpektraParse_V3	spektraParser;
    if(spektraParser.Open(strFileName.toLatin1().constData()) == false)
    {
        GS::Gex::Message::warning("SPEKTRA file",
                                  "Invalid SPAKTRA file (corrupted?).\n"
                                  "Please select a valid SPEKTRA file");
        return;
    }
    spektraParser.Close();

    // The selected file is a valid SPEKTRA file, emit signal telling a new file has been selected
    m_strSpektraFileName = strFileName;
    emit sSpektraFileSelected(strFileName);
}

void ImportSpektraDialog::OnOutputFormatChanged(int nItem)
{
    if(nItem == 0)
    {
        // ASCII
        groupRecords->setEnabled(true);
        groupOptions->setEnabled(true);
    }
    else
    {
        // STDF
        groupRecords->setEnabled(false);
        groupOptions->setEnabled(false);
    }
}

void ImportSpektraDialog::OnButtonClearAllRecords()
{
    checkLabel->setChecked(false);
    checkTestItem->setChecked(false);
    checkWaferID->setChecked(false);
    checkDeviceID->setChecked(false);
    checkTestData->setChecked(false);
}

void ImportSpektraDialog::OnButtonSetAllRecords()
{
    checkLabel->setChecked(true);
    checkTestItem->setChecked(true);
    checkWaferID->setChecked(true);
    checkDeviceID->setChecked(true);
    checkTestData->setChecked(true);
}

void ImportSpektraDialog::OnButtonProperties()
{
    SpektraRecordDialog dialog((CSpektra_Record_V3 *)&m_clLabel);
    dialog.exec();
    return;
}

void ImportSpektraDialog::OnButtonExport()
{
    if(comboOutputFormat->currentIndex() == 0)
        ExportToAscii();
    else
        ExportToStdf();
}

void ImportSpektraDialog::ExportToStdf()
{
    CSPEKTRAtoSTDF	clSpektraToStdf;
    QString			strDestination;
    QDir			cDir;
    bool			bStatus;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    strDestination = m_strSpektraFileName + ".gex.stdf";
    cDir.remove(strDestination);	// Make sure destination doesn't exist.

    // Convert
    bStatus = clSpektraToStdf.Convert(m_strSpektraFileName.toLatin1().constData(), strDestination.toLatin1().constData(), progressBar);
    QGuiApplication::restoreOverrideCursor();

    if(bStatus == true)
    {
        QString strMessage;
        strMessage = "Success converting SPEKTRA file to STDF:\n";
        strMessage += strDestination;
        GS::Gex::Message::information("", strMessage);
    }
    else
    {
        QString strMessage;
        QString	strError;
        clSpektraToStdf.GetLastError(strError);
        strMessage.sprintf("Error converting SPEKTRA file to STDF.\n%s", strError.toLatin1().constData());
        GS::Gex::Message::warning("", strMessage);
    }

    progressBar->reset();
}

void ImportSpektraDialog::ExportToAscii()
{
    CSPEKTRAtoASCII	clSpektraToAscii(m_bEvaluationMode);
    QString			strDestination;
    QDir			cDir;
    bool			bStatus;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    strDestination = m_strSpektraFileName + ".txt";
    cDir.remove(strDestination);	// Make sure destination doesn't exist.

    // Set options
    SetRecordsToProcess(clSpektraToAscii);
    clSpektraToAscii.SetFieldFilterOptionText(comboFieldFilter->currentText());
    switch(comboFieldFilter->currentIndex())
    {
        case 0:
            clSpektraToAscii.SetFieldFilter(CSpektra_Record_V3::FieldFlag_Present);
            break;
        case 1:
            clSpektraToAscii.SetFieldFilter(CSpektra_Record_V3::FieldFlag_None);
            break;
    }
    clSpektraToAscii.SetSizeFileLimitOptionText(comboSizeLimit->currentText());
    switch(comboSizeLimit->currentIndex())
    {
        case 0:	// No limit
            clSpektraToAscii.SetSizeFileLimit(0);
            break;
        case 1: // 500 kb
            clSpektraToAscii.SetSizeFileLimit(500*1024);
            break;
        case 2: // 1 Mb
            clSpektraToAscii.SetSizeFileLimit(1024*1024);
            break;
        case 3: // 2 Mb
            clSpektraToAscii.SetSizeFileLimit(2*1024*1024);
            break;
        case 4: // 3 Mb
            clSpektraToAscii.SetSizeFileLimit(3*1024*1024);
            break;
        case 5: // 4 Mb
            clSpektraToAscii.SetSizeFileLimit(4*1024*1024);
            break;
        case 6: // 5 Mb
            clSpektraToAscii.SetSizeFileLimit(5*1024*1024);
            break;
        case 7: // 10 Mb
            clSpektraToAscii.SetSizeFileLimit(10*1024*1024);
            break;
    }

    // Convert
    bStatus = clSpektraToAscii.Convert(m_strSpektraFileName.toLatin1().constData(),
                                       strDestination.toLatin1().constData(),
                                       progressBar);
    QGuiApplication::restoreOverrideCursor();

    if(bStatus == true)
    {
        QString strMessage;
        strMessage = "Success converting SPEKTRA file to ASCII:\n";
        strMessage += strDestination;
#if defined unix || __MACH__
        strMessage += "\nLauch 'textedit' editor on it?";

        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (! lOk)
        {
            return;
        }

        char	szString[2048];
        sprintf(szString,"textedit %s&", strDestination.toLatin1().constData());
    if (system(szString) == -1) {
      //FIXME: send error
    }
#else
        // Under windows, propose to lanch wordpad!
        strMessage += "\nDisplay file now?";

        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (! lOk)
        {
            return;
        }

        // Launch Wordpad
        ShellExecuteA(NULL,
               "open",
               strDestination.toLatin1().constData(),
               NULL,
               NULL,
               SW_SHOWNORMAL);
#endif
    }
    else
    {
        QString strMessage;
        QString	strError;
        clSpektraToAscii.GetLastError(strError);
        strMessage.sprintf("Error converting SPEKTRA file to ASCII.\n%s", strError.toLatin1().constData());
        GS::Gex::Message::warning("", strMessage);
    }

    progressBar->reset();
}

void ImportSpektraDialog::SetRecordsToProcess(CSPEKTRAtoASCII & clSpektraToAscii)
{
    if(checkLabel->isChecked() == true) clSpektraToAscii.SetProcessRecord(CSpektra_Record_V3::Rec_Label);
    if(checkTestItem->isChecked() == true) clSpektraToAscii.SetProcessRecord(CSpektra_Record_V3::Rec_TestItem);
    if(checkWaferID->isChecked() == true) clSpektraToAscii.SetProcessRecord(CSpektra_Record_V3::Rec_WaferID);
    if(checkDeviceID->isChecked() == true) clSpektraToAscii.SetProcessRecord(CSpektra_Record_V3::Rec_DeviceID);
    if(checkTestData->isChecked() == true) clSpektraToAscii.SetProcessRecord(CSpektra_Record_V3::Rec_TestData);
}

void ImportSpektraDialog::OnSpektraFileSelected()
{
    // Enable 'Export button'
    buttonExport->setEnabled(true);

    // Enable Properties button
    buttonProperties->setEnabled(true);

    // Reset Label record
    m_clLabel.Reset();

    // Read Label record
    CSpektraParse_V3	spektraParser;
    if(spektraParser.Open(m_strSpektraFileName.toLatin1().constData()) == false)
        return;
    if(spektraParser.LoadNextRecord(CSpektra_Record_V3::Rec_Label) == CSpektraParse_V3::NoError)
        spektraParser.ReadRecord((CSpektra_Record_V3 *)&m_clLabel);
    spektraParser.Close();
}
