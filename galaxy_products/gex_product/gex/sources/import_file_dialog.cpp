#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <qfiledialog.h>
#include <qstring.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <QDragEnterEvent>
#include <QUrl>
#include <QMimeData>
#include "message.h"
#include "engine.h"
#include "import_file_dialog.h"
#include "import_dl4.h"
#include "browser_dialog.h"

ImportFileDialog::ImportFileDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(buttonClose,	SIGNAL(clicked()),				this,			SLOT(reject()));
    QObject::connect(buttonImport,	SIGNAL(clicked()),				this,			SLOT(OnButtonImport()));
    QObject::connect(buttonSelect,	SIGNAL(clicked()),				this,			SLOT(OnButtonSelect()));
    QObject::connect(this,			SIGNAL(sFileSelected(QString)), editFileName,	SLOT(setText(QString)));
    QObject::connect(this,			SIGNAL(sFileSelected(QString)), this,			SLOT(OnFileSelected()));

	m_strFileName = "";

	// Disable Import button
	buttonImport->setEnabled(false);

	// Support for Drag&Drop
	setAcceptDrops(true);
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void ImportFileDialog::dragEnterEvent(QDragEnterEvent *e)
{
	// Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
		e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void ImportFileDialog::dropEvent(QDropEvent *e)
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
ImportFileDialog::~ImportFileDialog()
{
}

///////////////////////////////////////////////////////////
// Button to select a file to import has been clicked
///////////////////////////////////////////////////////////
void ImportFileDialog::OnButtonSelect()
{
	// Display Open file dialog so the user can select a file
	QString strFileName = QFileDialog::getOpenFileName(this, "Select the file you want to import (convert to STDF)", "", "DL4 files (*.dl4)");

	// Validate file selected
	OnSelectFile(strFileName);
}

///////////////////////////////////////////////////////////
// Validate selected (or dragged) file.
///////////////////////////////////////////////////////////
void ImportFileDialog::OnSelectFile(QString strFileName)
{
	// Check if new file selected
	if((strFileName.isEmpty() == true) || (m_strFileName == strFileName))
		return;

	// Make sure the file has a valid extension
	QFileInfo	fileInfo(strFileName);
    QString		strExtension = fileInfo.suffix();

	if(strExtension != "dl4")
	{
        GS::Gex::Message::warning(
            "Import file",
            "Invalid file extension.\n"
            "Please select a file with a supported extension (*.dl4)");
		return;
	}

	// The selected file is a valid file, emit signal telling a new file has been selected
	m_strFileName = strFileName;
	emit sFileSelected(strFileName);
}

void ImportFileDialog::OnButtonImport()
{
	CDL4toSTDF		clDl4ToStdf;
	QString			strDestination;
	QDir			cDir;
	bool			bStatus;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
	strDestination = m_strFileName + ".gex.stdf";
	cDir.remove(strDestination);	// Make sure destination doesn't exist.

	// Convert
    bStatus = clDl4ToStdf.Convert(m_strFileName.toLatin1().constData(),
                                  strDestination.toLatin1().constData());
    QGuiApplication::restoreOverrideCursor();

	if(bStatus == true)
	{
		QString strMessage;
		strMessage = "Success converting file to STDF:\n";
		strMessage += strDestination;
        GS::Gex::Message::information("", strMessage);
	}
	else
	{
		QString strMessage;
		QString	strError = clDl4ToStdf.GetLastError();
		strMessage.sprintf("Error converting file to STDF.\n%s", strError.toLatin1().constData());
        GS::Gex::Message::warning("", strMessage);
	}

	progressBar->reset();
}

void ImportFileDialog::OnFileSelected()
{
	// Enable 'Import button'
	buttonImport->setEnabled(true);
}
