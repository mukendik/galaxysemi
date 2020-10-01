#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "db_insert_file_dialog.h"
#include "db_external_database.h"

#include <QFileDialog>
#include <QProgressDialog>
#include <QCursor>
//#include <QLibrary>
#include <QDragEnterEvent>
#include <QUrl>
#include <QMimeData>
#include "message.h"
#include "gexdb_plugin_base.h"

// gex_constants.cpp
extern const char	*gexLabelFilterChoices[];

InsertFileDialog::InsertFileDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    QObject::connect(buttonClose,	SIGNAL(clicked()),				this,			SLOT(reject()));
    QObject::connect(buttonInsert,	SIGNAL(clicked()),				this,			SLOT(OnButtonInsert()));
    QObject::connect(buttonSelect,	SIGNAL(clicked()),				this,			SLOT(OnButtonSelect()));
    QObject::connect(this,			SIGNAL(sFileSelected(QString)), editFileName,	SLOT(setText(QString)));
    QObject::connect(this,			SIGNAL(sFileSelected(QString)), this,			SLOT(OnFileSelected()));

    m_strFileName = "";

    // Disable Import button
    buttonInsert->setEnabled(false);

    // Support for Drag&Drop
    setAcceptDrops(true);
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void InsertFileDialog::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void InsertFileDialog::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->formats().contains("text/uri-list"))
    {
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
InsertFileDialog::~InsertFileDialog()
{
}

///////////////////////////////////////////////////////////
// Button to select a file to import has been clicked
///////////////////////////////////////////////////////////
void InsertFileDialog::OnButtonSelect()
{
    // Display Open file dialog so the user can select a file
    QString strFileName = QFileDialog::getOpenFileName(this, "Select the STDF file you want to insert into the GEX DB", "", "STDF files (*.std *.stdf)");

    // Validate file selected
    OnSelectFile(strFileName);
}

///////////////////////////////////////////////////////////
// Validate selected (or dragged) file.
///////////////////////////////////////////////////////////
void InsertFileDialog::OnSelectFile(QString strFileName)
{
    // Check if new file selected
    if((strFileName.isEmpty() == true) || (m_strFileName == strFileName))
        return;

    // Make sure the file has a valid extension
    QFileInfo	fileInfo(strFileName);
    QString		strExtension = fileInfo.suffix();

    if((strExtension != "stdf") && (strExtension != "std"))
    {
        GS::Gex::Message::warning(
            "Insert file",
            "Invalid file extension.\n"
            "Please select a file with a supported extension (*.stdf, *.std)");
        return;
    }

    // Check if valid STDF V4 file


    // The selected file is a valid file, emit signal telling a new file has been selected
    m_strFileName = strFileName;
    emit sFileSelected(strFileName);
}

void InsertFileDialog::OnButtonInsert()
{
#if 0
    // Display Open file dialog so the user can select a DB config file
    QString strFileName = QFileDialog::getOpenFileName(this, "Select the config file of the GEX DB to use", "", "DB config file (*)");

    if(strFileName.isEmpty())
        return;
#endif

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    GexRemoteDatabase *pclRemoteDb = new GexRemoteDatabase;

    if(pclRemoteDb->LoadPlugin("C:\\Work\\ExaminatorDB\\Server\\Databases\\DialogSemi__Oracle_10_\\.gexdb_extern") == false)
        return;

    //clRemoteDb.InsertDataFile(m_strFileName);

    delete pclRemoteDb; pclRemoteDb=0;

    QGuiApplication::restoreOverrideCursor();
}

void InsertFileDialog::OnFileSelected()
{
    // Enable 'Insert button'
    buttonInsert->setEnabled(true);
}
