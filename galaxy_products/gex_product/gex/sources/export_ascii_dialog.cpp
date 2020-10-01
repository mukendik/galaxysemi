#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include "export_ascii_dialog.h"
#include "browser_dialog.h"

#include <QStringList>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QProgressDialog>
#include <QApplication>
#include <QDesktopServices>
#include <gqtl_archivefile.h>
#include <gqtl_sysutils.h>

#include "message.h"
#include "browser_dialog.h"
#include "engine.h"
#include "report_build.h"
#include "report_options.h"
#include "stdf_record_dialog.h"
#include "stdf.h"

// report_build.h
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)
// main.cpp


namespace GS
{
namespace Gex
{

class ExportAsciiDialogPrivate
{
public:
    //Constructor
    ExportAsciiDialogPrivate();
    ~ExportAsciiDialogPrivate()
    {

    }

    //Get the record type from it is label
    int GetRecordType(const ExportAsciiDialog::STDFFields &fields, const QString &name);
    // Return the file type
    ExportAsciiDialog::STDFFields GetFileType(ExportAscii_InputFile *inputFile);

    bool									m_bEvaluationMode;
    QStringList								m_strlistInputFiles;
    ExportAscii_InputFileList				m_pInputFiles;
    QList<ExportAsciiDialog_LookupItem*>	m_pLookup;
    QString									m_strOutputDir;
    ExportAsciiDialog::STDFFields mFieldsType;


    // List of STDF V4 Record
    static QStringList mSTDFV4Records;
    static QList<int>  mSTDFV4RecordsCode;
    // List of STDF V4 Additional Record
    static QStringList mSTDFV4AdditionalRecords;
    static QList<int>  mSTDFV4AdditionalRecordsCode;
    // List of STDF V3 Record
    static QStringList mSTDFV3Records;
    static QList<int>  mSTDFV3RecordsCode;
};

QStringList ExportAsciiDialogPrivate::mSTDFV4Records = QStringList()
        << "ATR" << "BPS" << "DTR" << "EPS" << "FAR" << "FTR" << "GDR" << "HBR" << "MIR" << "MPR" << "MRR" << "PCR"
        << "PGR" << "PIR" << "PLR" << "PMR" << "PRR" << "PTR" << "RDR" << "SBR" << "SDR" << "TSR" << "WCR" << "WIR"
        << "WRR" << "Unknown" ;
QList<int> ExportAsciiDialogPrivate::mSTDFV4RecordsCode = QList<int>()
        << GQTL_STDF::Stdf_Record::Rec_ATR << GQTL_STDF::Stdf_Record::Rec_BPS<< GQTL_STDF::Stdf_Record::Rec_DTR
        << GQTL_STDF::Stdf_Record::Rec_EPS << GQTL_STDF::Stdf_Record::Rec_FAR << GQTL_STDF::Stdf_Record::Rec_FTR
        << GQTL_STDF::Stdf_Record::Rec_GDR << GQTL_STDF::Stdf_Record::Rec_HBR << GQTL_STDF::Stdf_Record::Rec_MIR
        << GQTL_STDF::Stdf_Record::Rec_MPR << GQTL_STDF::Stdf_Record::Rec_MRR << GQTL_STDF::Stdf_Record::Rec_PCR
        << GQTL_STDF::Stdf_Record::Rec_PGR << GQTL_STDF::Stdf_Record::Rec_PIR << GQTL_STDF::Stdf_Record::Rec_PLR
        << GQTL_STDF::Stdf_Record::Rec_PMR << GQTL_STDF::Stdf_Record::Rec_PRR << GQTL_STDF::Stdf_Record::Rec_PTR
        << GQTL_STDF::Stdf_Record::Rec_RDR << GQTL_STDF::Stdf_Record::Rec_SBR << GQTL_STDF::Stdf_Record::Rec_SDR
        << GQTL_STDF::Stdf_Record::Rec_TSR << GQTL_STDF::Stdf_Record::Rec_WCR << GQTL_STDF::Stdf_Record::Rec_WIR
        << GQTL_STDF::Stdf_Record::Rec_WRR << GQTL_STDF::Stdf_Record::Rec_UNKNOWN ;

QStringList ExportAsciiDialogPrivate::mSTDFV4AdditionalRecords = QStringList()
        << "VUR" << "PSR" << "NMR" << "CNR" << "SSR" << "CDR" << "STR";
QList<int>  ExportAsciiDialogPrivate::mSTDFV4AdditionalRecordsCode = QList<int>()
        << GQTL_STDF::Stdf_Record::Rec_VUR << GQTL_STDF::Stdf_Record::Rec_PSR << GQTL_STDF::Stdf_Record::Rec_NMR
        << GQTL_STDF::Stdf_Record::Rec_CNR << GQTL_STDF::Stdf_Record::Rec_SSR << GQTL_STDF::Stdf_Record::Rec_CDR
        << GQTL_STDF::Stdf_Record::Rec_STR;

QStringList ExportAsciiDialogPrivate::mSTDFV3Records = QStringList()
        << "BPS" << "DTR" << "EPS" << "FAR" << "FDR" << "FTR" << "GDR" << "HBR" << "MIR" << "MRR" << "PDR" << "PIR"
        << "PMR" << "PRR" << "PTR" << "SBR" << "SCR" << "SHB" << "SSB" << "STS" << "TSR" << "WCR" << "WIR" << "WRR" ;
QList<int>  ExportAsciiDialogPrivate::mSTDFV3RecordsCode = QList<int>()
        << GQTL_STDF::Stdf_Record::Rec_BPS << GQTL_STDF::Stdf_Record::Rec_DTR << GQTL_STDF::Stdf_Record::Rec_EPS
        << GQTL_STDF::Stdf_Record::Rec_FAR << GQTL_STDF::Stdf_Record::Rec_FDR << GQTL_STDF::Stdf_Record::Rec_FTR
        << GQTL_STDF::Stdf_Record::Rec_GDR << GQTL_STDF::Stdf_Record::Rec_HBR << GQTL_STDF::Stdf_Record::Rec_MIR
        << GQTL_STDF::Stdf_Record::Rec_MRR << GQTL_STDF::Stdf_Record::Rec_PDR << GQTL_STDF::Stdf_Record::Rec_PIR
        << GQTL_STDF::Stdf_Record::Rec_PMR << GQTL_STDF::Stdf_Record::Rec_PRR << GQTL_STDF::Stdf_Record::Rec_PTR
        << GQTL_STDF::Stdf_Record::Rec_SBR << GQTL_STDF::Stdf_Record::Rec_SCR << GQTL_STDF::Stdf_Record::Rec_SHB
        << GQTL_STDF::Stdf_Record::Rec_SSB << GQTL_STDF::Stdf_Record::Rec_STS << GQTL_STDF::Stdf_Record::Rec_TSR
        << GQTL_STDF::Stdf_Record::Rec_WCR << GQTL_STDF::Stdf_Record::Rec_WIR << GQTL_STDF::Stdf_Record::Rec_WRR ;


ExportAsciiDialogPrivate::ExportAsciiDialogPrivate()
{
    mFieldsType = ExportAsciiDialog::STDF_V4;
}

int ExportAsciiDialogPrivate::GetRecordType(const ExportAsciiDialog::STDFFields &fields, const QString &name)
{
    int lIdx = -1;
    if(fields == ExportAsciiDialog::STDF_V4)
    {
        lIdx = mSTDFV4Records.indexOf(name);
        if(lIdx == -1)
        {
            lIdx = mSTDFV4AdditionalRecords.indexOf(name);
            if(lIdx == -1)
            {
                return GQTL_STDF::Stdf_Record::Rec_COUNT;
            }
            else
            {
                return mSTDFV4AdditionalRecordsCode[lIdx];
            }
        }
        else
        {
            return mSTDFV4RecordsCode[lIdx];
        }
    }
    else if(fields == ExportAsciiDialog::STDF_V3)
    {
         lIdx = mSTDFV3Records.indexOf(name);
         if(lIdx == -1)
         {
             return GQTL_STDF::Stdf_Record::Rec_COUNT;
         }
         else
         {
             return mSTDFV3RecordsCode[lIdx];
         }
    }
    return GQTL_STDF::Stdf_Record::Rec_COUNT;
}
ExportAsciiDialog::STDFFields ExportAsciiDialogPrivate::GetFileType(ExportAscii_InputFile *inputFile)
{

    ExportAscii_FileInfo::FileTypes lType = ExportAscii_FileInfo::FileType_Unknown;
    ExportAsciiDialog::STDFFields lFieldsType = ExportAsciiDialog::STDF_UNKNOWN;

    if(inputFile && inputFile->m_pclFileInfo)
    {
        lType = inputFile->m_pclFileInfo->m_eFileType;
        if(lType == ExportAscii_FileInfo::FileType_Compressed) //FileType_Compressed take the type of the first one
        {
            if(!inputFile->m_pExtractedFiles.isEmpty())
                lType = (*(inputFile->m_pExtractedFiles.begin()))->m_eFileType;

        }

        if(lType == ExportAscii_FileInfo::FileType_STDF_V3)
        {
            lFieldsType = ExportAsciiDialog::STDF_V3;
        }
        else if(lType == ExportAscii_FileInfo::FileType_STDF_V4
                || lType == ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst)
        {
            lFieldsType = ExportAsciiDialog::STDF_V4;
        }
        else if(lType == ExportAscii_FileInfo::FileType_STDF_NoFAR || lType == ExportAscii_FileInfo::FileType_Unknown)
        {
            lFieldsType = ExportAsciiDialog::STDF_UNKNOWN;
        }

    }
    return lFieldsType;
}
///////////////////////////////////////////////////////////
// Constructors / Destructors
///////////////////////////////////////////////////////////
ExportAsciiDialog::ExportAsciiDialog( bool bEvaluationMode, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
    mPrivate = new ExportAsciiDialogPrivate;
    setupUi(this);
    setModal(modal);

    // Apply Examinator palette
    GexMainwindow::applyPalette(this);

    QObject::connect(checkBoxCommonOutputDir,		SIGNAL(toggled(bool)),		buttonBrowseOutpuDirectory,			SLOT(setEnabled(bool)));
    QObject::connect(buttonRepair,					SIGNAL(clicked()),			this,								SLOT(OnButtonRepair()));
    QObject::connect(buttonDump,					SIGNAL(clicked()),			this,								SLOT(OnButtonDump()));
    QObject::connect(buttonClose,					SIGNAL(clicked()),			this,								SLOT(accept()));
    QObject::connect(buttonSelect,					SIGNAL(clicked()),			this,								SLOT(OnButtonSelect()));
    QObject::connect(buttonProperties,				SIGNAL(clicked()),			this,								SLOT(OnFileProperties()));
    QObject::connect(this,							SIGNAL(sFilesSelected()),	this,								SLOT(OnFilesSelected()));
    QObject::connect(buttonSetAllRecords,			SIGNAL(clicked()),			this,								SLOT(OnButtonSetAllRecords()));
    QObject::connect(buttonClearAllRecords,			SIGNAL(clicked()),			this,								SLOT(OnButtonClearAllRecords()));
    QObject::connect(buttonEdit,					SIGNAL(clicked()),			this,								SLOT(OnFileEditAscii()));
    QObject::connect(buttonBrowseOutpuDirectory,	SIGNAL(clicked()),			this,								SLOT(OnButtonBrowseOutpuDir()));
    QObject::connect(comboOutputFormat,				SIGNAL(activated(int)),		this,								SLOT(OnChangeOutputFormat()));
    QObject::connect(treeWidgetInputFiles,			SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),		this,	SLOT(OnFileDoubleClicked()));
    QObject::connect(treeWidgetInputFiles,			SIGNAL(customContextMenuRequested(const QPoint&)),		this,	SLOT(onContextMenuRequested(const QPoint&)));
    QObject::connect(checkBoxTNFilter,				SIGNAL(toggled(bool)),		cmdTNFilterEdit,					SLOT(setEnabled(bool)));

    // Activate the dump all if any of the parameter has been changed
    QObject::connect(comboFieldFilter,				SIGNAL(activated(int)),		this,                               SLOT(SetDumpEnabled()));
    QObject::connect(comboSizeLimit,				SIGNAL(activated(int)),		this,                               SLOT(SetDumpEnabled()));
    QObject::connect(checkBoxDumpFirstOnly,			SIGNAL(toggled(bool)),		this,                               SLOT(SetDumpEnabled(bool)));
    QObject::connect(checkBoxCommonOutputDir,		SIGNAL(toggled(bool)),		this,                               SLOT(SetDumpEnabled(bool)));
    QObject::connect(checkBoxTNFilter,              SIGNAL(toggled(bool)),		this,                               SLOT(SetDumpEnabled(bool)));


    // Disable Export button
    buttonDump->setEnabled(false);

    // Set default common output directory to user home
    if(	(CGexSystemUtils::GetUserHomeDirectory(mPrivate->m_strOutputDir) != CGexSystemUtils::NoError) &&
        (CGexSystemUtils::GetApplicationDirectory(mPrivate->m_strOutputDir) != CGexSystemUtils::NoError))
        mPrivate->m_strOutputDir = "";
    editOutputDir->setText(mPrivate->m_strOutputDir);

    // Hide buttons
    buttonProperties->setEnabled(false);
    buttonEdit->setEnabled(false);

    // Check all record buttons
    OnButtonSetAllRecords();

    // Evaluation mode ??
    mPrivate->m_bEvaluationMode = bEvaluationMode;

    // Disable file size limit option in evaluation mode
    if(mPrivate->m_bEvaluationMode)
        comboSizeLimit->setEnabled(false)	;

    // Support for Drag&Drop
    setAcceptDrops(true);
    // Disable 'Repair' button
    buttonRepair->setEnabled(false);

    UpdateSTDFFields(mPrivate->mFieldsType);

}

/*
 *  Destroys the object and frees any allocated resources
 */
ExportAsciiDialog::~ExportAsciiDialog()
{
    // Clear list of input files
    mPrivate->m_pInputFiles.clear();

    // Clear lookup list
    while(!mPrivate->m_pLookup.isEmpty())
    {
        ExportAsciiDialog_LookupItem *lItem = mPrivate->m_pLookup.takeFirst();
        if(lItem)
            delete lItem;
    }
}

///////////////////////////////////////////////////////////
// Starting DRAG sequence
///////////////////////////////////////////////////////////
void ExportAsciiDialog::dragEnterEvent(QDragEnterEvent *e)
{
    // Accept Drag if files list dragged over.
    //if(e->mimeData()->formats().contains("text/uri-list"))
    if(e->mimeData()->formats().contains("text/uri-list"))
        e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Dropping files in Widget (Drag & Drop)
///////////////////////////////////////////////////////////
void ExportAsciiDialog::dropEvent(QDropEvent *e)
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

    // Emit signal telling new files have been selected
    mPrivate->m_strlistInputFiles.clear();
    mPrivate->m_strlistInputFiles = strFileList;
    emit sFilesSelected();

    e->acceptProposedAction();
}

///////////////////////////////////////////////////////////
// Button to select a STDF files to convert has been clicked
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnButtonSelect()
{
    // Display Open file dialog so the user can select a file
    QStringList strlistFiles = QFileDialog::getOpenFileNames(this, "Select the STDF file you want to convert", "", "STDF files (*.std *.stdf *.zip *.gz *.Z *)");

    if(strlistFiles.count() > 0)
    {
        // Emit signal telling new files have been selected
        mPrivate->m_strlistInputFiles.clear();
        mPrivate->m_strlistInputFiles = strlistFiles;
        emit sFilesSelected();
    }
}

///////////////////////////////////////////////////////////
// Button to select a common output directory has been clicked
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnButtonBrowseOutpuDir()
{
    QString strDir = QFileDialog::getExistingDirectory(
                    this,
                    "Choose a directory to store dump files",
                    mPrivate->m_strOutputDir,
                    QFileDialog::ShowDirsOnly);

    if(!strDir.isEmpty())
    {
        mPrivate->m_strOutputDir = strDir;
        editOutputDir->setText(mPrivate->m_strOutputDir);
    }
}

///////////////////////////////////////////////////////////
// Button to clear all Record Selection check boxes  has
// been clicked
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnButtonClearAllRecords()
{
   UpdateRecordStatus(false);
}

///////////////////////////////////////////////////////////
// Button to set all Record Selection check boxes  has
// been clicked
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnButtonSetAllRecords()
{
    UpdateRecordStatus(true);
}

void ExportAsciiDialog::UpdateRecordStatus(bool status)
{
    QList<QCheckBox *> lRecordChecks = mFieldsCheckBox->findChildren<QCheckBox *> ();//  QCheckBox
    for(int lIdx=0; lIdx<lRecordChecks.count(); ++lIdx){
        lRecordChecks[lIdx]->setChecked(status);
        QObject::connect(lRecordChecks[lIdx], SIGNAL(stateChanged(int)), this, SLOT(updateDialogButton(int)));
    }

}

///////////////////////////////////////////////////////////
// Returns ptr on FileInfo class corresponding to the
// current item selected in the list view
///////////////////////////////////////////////////////////
ExportAscii_FileInfo* ExportAsciiDialog::GetCurrentFileInfo()
{
    // Check if valid current item
    QTreeWidgetItem * pCurrentItem = treeWidgetInputFiles->currentItem();
    if (!pCurrentItem)
        return NULL;

    // Go through lookup list
    QList<ExportAsciiDialog_LookupItem *>::iterator itBegin	= mPrivate->m_pLookup.begin();
    QList<ExportAsciiDialog_LookupItem *>::iterator itEnd	= mPrivate->m_pLookup.end();

    while(itBegin != itEnd)
    {
        if((*itBegin)->m_pclListViewItem == pCurrentItem)
        {
            // Make the input file current input file in the list
            mPrivate->m_pInputFiles.SetCurrentInputFile((*itBegin)->m_pclInputFile);

            return (*itBegin)->m_pclFileInfo;
        }

        itBegin++;
    }

    return NULL;
}

///////////////////////////////////////////////////////////
// Returns ptr on the list view item corresponding to the
// given FileInfo ptr
///////////////////////////////////////////////////////////
QTreeWidgetItem* ExportAsciiDialog::GetItem(ExportAscii_FileInfo *pclFileInfo)
{
    // Check if valid FileInfo ptr
    if(!pclFileInfo)
        return NULL;

    // Go through lookup list
    QList<ExportAsciiDialog_LookupItem *>::iterator itBegin	= mPrivate->m_pLookup.begin();
    QList<ExportAsciiDialog_LookupItem *>::iterator itEnd	= mPrivate->m_pLookup.end();

    while(itBegin != itEnd)
    {
        if((*itBegin)->m_pclFileInfo == pclFileInfo)
            return (*itBegin)->m_pclListViewItem;

        itBegin++;
    }

    return NULL;
}

///////////////////////////////////////////////////////////
// User has double-clicked on one list view item:
// eventually launch appropriate action (STDF file properties
// or ascii dump edit) depending on selected item in the
// list view
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnFileDoubleClicked()
{
    ExportAscii_FileInfo *pclFileInfo = GetCurrentFileInfo();

    // Check if item found
    if(!pclFileInfo)
        return;

    // Display STDF properties, or edit ascii file??
    if(pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4)
    {
        if(pclFileInfo->m_bFileDumped)
            EditAsciiFile(pclFileInfo);
        else if(pclFileInfo->m_pclMIR)
            DisplayFileProperties(pclFileInfo);
    }
}

///////////////////////////////////////////////////////////
// The ascii dump corresponding to the currently selected
// item in the list view should be edited
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnFileEditAscii()
{
    ExportAscii_FileInfo *pclFileInfo = GetCurrentFileInfo();

    // Check if item found
    if(pclFileInfo)
        EditAsciiFile(pclFileInfo);
}

///////////////////////////////////////////////////////////
// The STDF properties corresponding to the currently selected
// item in the list view should be displayed
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnFileProperties()
{
    ExportAscii_FileInfo *pclFileInfo = GetCurrentFileInfo();

    // Check if item found
    if(pclFileInfo)
        DisplayFileProperties(pclFileInfo);
}

///////////////////////////////////////////////////////////
// The files corresponding to the currently selected
// item in the list view should be dumped
void ExportAsciiDialog::OnFileDump()
{
    ExportAscii_FileInfo *pclFileInfo = GetCurrentFileInfo();

    // Check if item found
    if(pclFileInfo && (pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4))
    {
        if(comboOutputFormat->currentIndex() == 0)
        {
            CSTDFtoASCII clStdfToAscii(mPrivate->m_bEvaluationMode);
            Dump(clStdfToAscii, pclFileInfo);
        }
        else
        {
            CSTDFtoATDF clStdfToAtdf(mPrivate->m_bEvaluationMode);
            Dump(clStdfToAtdf, pclFileInfo);
        }
    }
}

///////////////////////////////////////////////////////////
// The files corresponding to the currently selected
// item in the list view should be repaired
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnFileRepair()
{
    ExportAscii_FileInfo *pclFileInfo = GetCurrentFileInfo();

    // Check if item found
    if(pclFileInfo && (pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst))
    {
        // Set wait cursor
        QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        // Repair file
        RepairFile(pclFileInfo);

        // Conversion done
        progressBar->reset();
        labelStatus->setText("Status: Ready");
        QCoreApplication::processEvents();

        // Reset cursor
        QGuiApplication::restoreOverrideCursor();

        // Update buttons
        UpdateButtons();
    }
}

///////////////////////////////////////////////////////////
// Display the STDF properties corresponding to the given
// FileInfo ptr
///////////////////////////////////////////////////////////
void ExportAsciiDialog::DisplayFileProperties(ExportAscii_FileInfo *pclFileInfo)
{
    // Check if item is a STDF V4 file
    if((pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4) && (pclFileInfo->m_pclMIR != NULL))
    {
        StdfRecordDialog dialog((GQTL_STDF::Stdf_Record *)(pclFileInfo->m_pclMIR));
        dialog.exec();
    }
}

///////////////////////////////////////////////////////////
// Edit the ascii dump corresponding to the given FileInfo
// ptr
///////////////////////////////////////////////////////////
void ExportAsciiDialog::EditAsciiFile(ExportAscii_FileInfo *pclFileInfo)
{
    // Check if item is a STDF V4 file, and has been converted
    if((pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4) && pclFileInfo->m_bFileDumped)
    {
        QString strFileToEdit = pclFileInfo->m_strFullFileName_Ascii;
        GexMainwindow::OpenFileInEditor(strFileToEdit, GexMainwindow::eDocText);
//        if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strFileToEdit)))) //Case 4619 - Use OS's default program to open file
//          return;
//#if defined unix || __MACH__
//		char	szString[2048];
//		sprintf(szString,"%s %s&",
//			ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().constData(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//			strFileToEdit.toLatin1().constData());
//    if (system(szString) == -1) {
//      //FIXME: send error
//    }
//#else
//		// Under windows, propose to lanch wordpad (unless custom editor defined)!

//		// If file includes a space, we need to bath it between " "
//		if(strFileToEdit.indexOf(' ') != -1)
//		{
//			strFileToEdit = "\"" + strFileToEdit;
//			strFileToEdit = strFileToEdit + "\"";
//		}

//		// Launch text editor
//		ShellExecuteA(NULL,
//			   "open",
//			ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().constData(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//			   strFileToEdit.toLatin1().constData(),
//			   NULL,
//			   SW_SHOWNORMAL);
//#endif
    }
}

///////////////////////////////////////////////////////////
// The 'Repair' button has been clicked
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnButtonRepair()
{
    // Set wait cursor
    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    // Repair all files
    ExportAscii_FileInfo	*pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst);
    while(pclFileInfo)
    {
        RepairFile(pclFileInfo);
        pclFileInfo = mPrivate->m_pInputFiles.NextStdf(ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst);
    }

    // Conversion done
    progressBar->reset();
    labelStatus->setText("Status: Ready");
    QCoreApplication::processEvents();

    // Reset cursor
    QGuiApplication::restoreOverrideCursor();

    // Update buttons
    UpdateButtons();
}

///////////////////////////////////////////////////////////
// Repair one STDF file
///////////////////////////////////////////////////////////
bool ExportAsciiDialog::RepairFile(ExportAscii_FileInfo *pclFileInfo)
{
    QString					strStatus;
    QTreeWidgetItem*		pItem;
    GS::StdLib::Stdf					clStdf;

    // Set status
    strStatus = "Status: Repairing file ";
    if(pclFileInfo->m_strFullFileName.length()>80)
        strStatus += "...";
    strStatus += pclFileInfo->m_strFullFileName.right(80);
    strStatus += "...";
    labelStatus->setText(strStatus);
    progressBar->reset();
    QCoreApplication::processEvents();

    // Repair file
    if(clStdf.RepairStdfFile(pclFileInfo->m_strFullFileName.toLatin1().constData(), pclFileInfo->m_strFullFileName_Repaired.toLatin1().constData(), pclFileInfo->m_nStdf_Cpu) == true)
    {
        // Update file info
        mPrivate->m_pInputFiles.Repaired(pclFileInfo);

        // Update list view
        pItem = GetItem(pclFileInfo);
        if(pItem)
        {
            pItem->setText(0, pclFileInfo->m_strFileName);
            pItem->setText(1, pclFileInfo->m_strFileType);
            strStatus = QString::number(pclFileInfo->m_uiFileSize) + " bytes";
            pItem->setText(3, strStatus);
        }
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////
// The 'Dump' button has been clicked
void ExportAsciiDialog::OnButtonDump()
{
    if(comboOutputFormat->currentIndex() == 0)
    {
        CSTDFtoASCII clStdfToAscii(mPrivate->m_bEvaluationMode);
        Dump(clStdfToAscii);
    }
    else
    {
        CSTDFtoATDF clStdfToAtdf(mPrivate->m_bEvaluationMode);
        Dump(clStdfToAtdf);
    }
}

///////////////////////////////////////////////////////////
// Dump to ASCII format
///////////////////////////////////////////////////////////
void ExportAsciiDialog::Dump(CSTDFtoASCII & clStdfToAscii, ExportAscii_FileInfo *pclFileInfo/*=NULL*/)
{
    // Set wait cursor
    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    // Set options
    SetRecordsToProcess(clStdfToAscii);
    clStdfToAscii.SetFieldFilterOptionText(comboFieldFilter->currentText());
    switch(comboFieldFilter->currentIndex())
    {
        case 0:
            clStdfToAscii.SetFieldFilter(GQTL_STDF::Stdf_Record::FieldFlag_Present);
            break;
        case 1:
            clStdfToAscii.SetFieldFilter(GQTL_STDF::Stdf_Record::FieldFlag_None);
            break;
    }
    clStdfToAscii.SetSizeFileLimitOptionText(comboSizeLimit->currentText());
    switch(comboSizeLimit->currentIndex())
    {
        case 0:	// No limit
            clStdfToAscii.SetSizeFileLimit(0);
            break;
        case 1: // 500 kb
            clStdfToAscii.SetSizeFileLimit(500*1024);
            break;
        case 2: // 1 Mb
            clStdfToAscii.SetSizeFileLimit(1024*1024);
            break;
        case 3: // 2 Mb
            clStdfToAscii.SetSizeFileLimit(2*1024*1024);
            break;
        case 4: // 3 Mb
            clStdfToAscii.SetSizeFileLimit(3*1024*1024);
            break;
        case 5: // 4 Mb
            clStdfToAscii.SetSizeFileLimit(4*1024*1024);
            break;
        case 6: // 5 Mb
            clStdfToAscii.SetSizeFileLimit(5*1024*1024);
            break;
        case 7: // 10 Mb
            clStdfToAscii.SetSizeFileLimit(10*1024*1024);
            break;
    }
    clStdfToAscii.SetConsecutiveRecordOption(checkBoxDumpFirstOnly->isChecked());

    // Convert all files
    ExportAscii_FileInfo	*pclFileInfo_LastFileConverted = NULL;

    unsigned int			uiConverted 	= 0;
    bool					bWasConverted	= false;
    if (pclFileInfo)
    {
        bWasConverted = pclFileInfo->m_bFileDumped;
        if(ConvertFile(clStdfToAscii, pclFileInfo))
        {
            uiConverted++;
            if(!bWasConverted)
                mPrivate->m_pInputFiles.m_uiNbFiles_Converted++;
            pclFileInfo_LastFileConverted = pclFileInfo;
        }
    }
    else
    {
        pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4);
        if(!pclFileInfo)
        {
            pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V3);
        }
        while(pclFileInfo)
        {
            if(!pclFileInfo->m_bFileDumped && ConvertFile(clStdfToAscii, pclFileInfo))
            {
                uiConverted++;
                mPrivate->m_pInputFiles.m_uiNbFiles_Converted++;
                pclFileInfo_LastFileConverted = pclFileInfo;
            }
            pclFileInfo = mPrivate->m_pInputFiles.NextStdf(ExportAscii_FileInfo::FileType_STDF_V4);
            if(!pclFileInfo)
            {
                pclFileInfo = mPrivate->m_pInputFiles.NextStdf(ExportAscii_FileInfo::FileType_STDF_V3);
            }
        }
    }

    // Conversion done
    progressBar->reset();
    labelStatus->setText("Status: Ready");
    QCoreApplication::processEvents();

    // Reset cursor
    QGuiApplication::restoreOverrideCursor();

    // Update buttons
    UpdateButtons();

    // If only 1 file converted, offer to edit it
    if((uiConverted == 1) && (pclFileInfo_LastFileConverted != NULL))
    {
        QString strMessage, strAsciiFile = pclFileInfo_LastFileConverted->m_strFullFileName_Ascii;
        strMessage = "Success converting STDF file to ASCII:\n";
        strMessage += strAsciiFile;
        strMessage += "\nDisplay file now?";

        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (! lOk)
        {
            return;
        }

        GexMainwindow::OpenFileInEditor(strAsciiFile, GexMainwindow::eDocText);

//        if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strAsciiFile)))) //Case 4619 - Use OS's default program to open file
//          return;
//		if (QDesktopServices::openUrl(QUrl(strAsciiFile)))
//			return;

//		// Using old school open method
//#if defined unix || __MACH__
//		char	szString[2048];
//		sprintf(szString,"%s %s&",
//			ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().constData(), //ReportOptions.strTextEditor.toLatin1().constData(),
//			strAsciiFile.toLatin1().constData());
//    if (system(szString) == -1) {
//      //FIXME: send error
//    }
//#else
//		// Under windows, propose to lanch wordpad (unless custom editor defined)!

//		// If file includes a space, we need to bath it between " "
//		if(strAsciiFile.indexOf(' ') != -1)
//		{
//			strAsciiFile = "\"" + strAsciiFile;
//			strAsciiFile = strAsciiFile + "\"";
//		}

//		// Launch text editor
//		ShellExecuteA(NULL,
//			   "open",
//			ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//			   strAsciiFile.toLatin1().constData(),
//			   NULL,
//			   SW_SHOWNORMAL);
//#endif
    }
}

///////////////////////////////////////////////////////////
// Dump one STDF file to a Ascii file
///////////////////////////////////////////////////////////
bool ExportAsciiDialog::ConvertFile(CSTDFtoASCII & clStdfToAscii, ExportAscii_FileInfo *pclFileInfo)
{
    QString				strStatus;
    QTreeWidgetItem*	pItem;

    // Set status
    strStatus = "Status: Converting file ";
    if(pclFileInfo->m_strFullFileName.length()>80)
        strStatus += "...";
    strStatus += pclFileInfo->m_strFullFileName.right(80);
    strStatus += "...";
    labelStatus->setText(strStatus);
    progressBar->reset();
    progressBar->setMaximum(100);
    QCoreApplication::processEvents();

    // Convert file
    if(pclFileInfo->Convert(clStdfToAscii,"", progressBar) == true)
    {
        // Update list view
        pItem = GetItem(pclFileInfo);
        if(pItem)
            pItem->setText(2, "Y");
        return true;
    }

    QString strMessage;
    QString	strError;
    clStdfToAscii.GetLastError(strError);
    strMessage.sprintf("Error converting STDF file to ASCII.\n%s", strError.toLatin1().constData());
    GS::Gex::Message::warning("", strMessage);

    return false;
}

void ExportAsciiDialog::SetDumpEnabled()
{
    buttonDump->setEnabled(true);
    ExportAscii_FileInfo *pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4);
    if(pclFileInfo)
        pclFileInfo->m_bFileDumped = false;
}


void ExportAsciiDialog::SetDumpEnabled(bool)
{
    buttonDump->setEnabled(true);
    ExportAscii_FileInfo *pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4);
    if(pclFileInfo)
        pclFileInfo->m_bFileDumped = false;
}
///////////////////////////////////////////////////////////
// Dump to ASCII format
///////////////////////////////////////////////////////////
void ExportAsciiDialog::Dump(CSTDFtoATDF & clStdfToAtdf, ExportAscii_FileInfo *pclFileInfo/*=NULL*/)
{
    // Set wait cursor
    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    // Set options
    SetRecordsToProcess(clStdfToAtdf);
    clStdfToAtdf.SetSizeFileLimitOptionText(comboSizeLimit->currentText());
    switch(comboSizeLimit->currentIndex())
    {
        case 0:	// No limit
            clStdfToAtdf.SetSizeFileLimit(0);
            break;
        case 1: // 500 kb
            clStdfToAtdf.SetSizeFileLimit(500*1024);
            break;
        case 2: // 1 Mb
            clStdfToAtdf.SetSizeFileLimit(1024*1024);
            break;
        case 3: // 2 Mb
            clStdfToAtdf.SetSizeFileLimit(2*1024*1024);
            break;
        case 4: // 3 Mb
            clStdfToAtdf.SetSizeFileLimit(3*1024*1024);
            break;
        case 5: // 4 Mb
            clStdfToAtdf.SetSizeFileLimit(4*1024*1024);
            break;
        case 6: // 5 Mb
            clStdfToAtdf.SetSizeFileLimit(5*1024*1024);
            break;
        case 7: // 10 Mb
            clStdfToAtdf.SetSizeFileLimit(10*1024*1024);
            break;
    }
    clStdfToAtdf.SetConsecutiveRecordOption(checkBoxDumpFirstOnly->isChecked());

    // Convert all files
    ExportAscii_FileInfo	*pclFileInfo_LastFileConverted = NULL;
    QString					strLastAsciiFileCreated;
    unsigned int			uiConverted 	= 0;
    bool					bWasConverted 	= false;

    if(pclFileInfo)
    {
        bWasConverted = pclFileInfo->m_bFileDumped;
        if(ConvertFile(clStdfToAtdf, pclFileInfo))
        {
            uiConverted++;
            if(!bWasConverted)
                mPrivate->m_pInputFiles.m_uiNbFiles_Converted++;
            pclFileInfo_LastFileConverted = pclFileInfo;
        }
    }
    else
    {
        pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4);
        while(pclFileInfo)
        {
            if(!pclFileInfo->m_bFileDumped && ConvertFile(clStdfToAtdf, pclFileInfo))
            {
                uiConverted++;
                mPrivate->m_pInputFiles.m_uiNbFiles_Converted++;
                pclFileInfo_LastFileConverted = pclFileInfo;
            }
            pclFileInfo = mPrivate->m_pInputFiles.NextStdf(ExportAscii_FileInfo::FileType_STDF_V4);
        }
    }

    // Conversion done
    progressBar->reset();
    labelStatus->setText("Status: Ready");
    QCoreApplication::processEvents();

    // Reset cursor
    QGuiApplication::restoreOverrideCursor();

    // Update buttons
    UpdateButtons();

    // If only 1 file converted, offer to edit it
    if((uiConverted == 1) && (pclFileInfo_LastFileConverted != NULL))
    {
        QString strMessage, strAsciiFile = pclFileInfo_LastFileConverted->m_strFullFileName_Ascii;
        strMessage = "Success converting STDF file to ASCII:\n";
        strMessage += strAsciiFile;
        strMessage += "\nDisplay file now?";

        bool lOk;
        GS::Gex::Message::request("", strMessage, lOk);
        if (! lOk)
        {
            return;
        }

        GexMainwindow::OpenFileInEditor(strAsciiFile, GexMainwindow::eDocText);

//        if (ReportOptions.GetOption("preferences","use_os_default").toBool() && QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(strAsciiFile)))) //Case 4619 - Use OS's default program to open file
//          return;

//#if defined unix || __MACH__
//		char	szString[2048];
//		sprintf(szString,"%s %s&",
//			ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//			strAsciiFile.toLatin1().constData());
//    if (system(szString) == -1) {
//      //FIXME: send error
//    }
//#else
//		// Under windows, propose to lanch wordpad (unless custom editor defined)!

//		// If file includes a space, we need to bath it between " "
//		if(strAsciiFile.indexOf(' ') != -1)
//		{
//			strAsciiFile = "\"" + strAsciiFile;
//			strAsciiFile = strAsciiFile + "\"";
//		}
//		// Launch text editor
//		ShellExecuteA(NULL,
//			   "open",
//			ReportOptions.GetOption("preferences", "text_editor").toString().toLatin1().data(),	//ReportOptions.strTextEditor.toLatin1().constData(),
//			   strAsciiFile.toLatin1().constData(),
//			   NULL,
//			   SW_SHOWNORMAL);
//#endif
    }
}

///////////////////////////////////////////////////////////
// Dump one STDF file to a Atdf file
///////////////////////////////////////////////////////////
bool ExportAsciiDialog::ConvertFile(CSTDFtoATDF & clStdfToAtdf, ExportAscii_FileInfo *pclFileInfo)
{
    QString				strAsciiFileName, strStatus;
    QTreeWidgetItem*	pItem;
    QFileInfo			clFileInfo;

    // Set status
    strStatus = "Status: Converting file ";
    if(pclFileInfo->m_strFullFileName.length()>80)
        strStatus += "...";
    strStatus += pclFileInfo->m_strFullFileName.right(80);
    strStatus += "...";
    labelStatus->setText(strStatus);
    progressBar->reset();
    QCoreApplication::processEvents();

    // Convert file
    if(pclFileInfo->Convert(clStdfToAtdf,"", progressBar) == true)
    {
        // Update list view
        pItem = GetItem(pclFileInfo);
        if(pItem)
            pItem->setText(2, "Y");
        return true;
    }

    QString strMessage;
    QString	strError;
    clStdfToAtdf.GetLastError(strError);
    strMessage.sprintf("Error converting STDF file to ASCII.\n%s", strError.toLatin1().constData());
    GS::Gex::Message::warning("", strMessage);

    return false;
}

///////////////////////////////////////////////////////////
// Set records to process options in the StdfToAscii dump
// object
///////////////////////////////////////////////////////////
void ExportAsciiDialog::SetRecordsToProcess(CSTDFtoASCII & clStdfToAscii)
{
    QList<QCheckBox *> lRecordChecks = mFieldsCheckBox->findChildren<QCheckBox *> ();
    // Set the Record to be dumped depending on the check box associated status
    for(int lIdx=0; lIdx<lRecordChecks.count(); ++lIdx)
    {
        QString lName = lRecordChecks[lIdx]->text();
        bool lIsChecked = lRecordChecks[lIdx]->isChecked();
        if(lIsChecked)
        {
            int lCode = GQTL_STDF::Stdf_Record::Rec_COUNT;
            if(mPrivate->mFieldsType == ExportAsciiDialog::STDF_MIXED)
            {
                lCode = mPrivate->GetRecordType(ExportAsciiDialog::STDF_V4,lName);
                if(lCode == GQTL_STDF::Stdf_Record::Rec_COUNT)
                    lCode = mPrivate->GetRecordType(ExportAsciiDialog::STDF_V3,lName);
            }
            else if(mPrivate->mFieldsType == ExportAsciiDialog::STDF_UNKNOWN)
            {
                lCode = GQTL_STDF::Stdf_Record::Rec_COUNT;
            }
            else
            {
                lCode = mPrivate->GetRecordType(mPrivate->mFieldsType,lName);
            }
            clStdfToAscii.SetProcessRecord(lCode);
        }
    }


    // test number filter
    if (checkBoxTNFilter->isChecked())
        clStdfToAscii.setTNFilteringList(cmdTNFilterEdit->text());
    else
        clStdfToAscii.setTNFilteringList("");
}


///////////////////////////////////////////////////////////
// Set records to process options in the StdfToAtdf dump
// object
///////////////////////////////////////////////////////////
void ExportAsciiDialog::SetRecordsToProcess(CSTDFtoATDF & clStdfToAtdf)
{
    QList<QCheckBox *> lRecordChecks = mFieldsCheckBox->findChildren<QCheckBox *> ();
    // Set the Record to be dumped depending on the check box associated status
    for(int lIdx=0; lIdx<lRecordChecks.count(); ++lIdx)
    {
        QString lName = lRecordChecks[lIdx]->text();
        bool lIsChecked = lRecordChecks[lIdx]->isChecked();
        if(lIsChecked)
        {
            int lCode = GQTL_STDF::Stdf_Record::Rec_COUNT;
            if(mPrivate->mFieldsType == ExportAsciiDialog::STDF_MIXED)
            {
                lCode = mPrivate->GetRecordType(ExportAsciiDialog::STDF_V4,lName);
                if(lCode == GQTL_STDF::Stdf_Record::Rec_COUNT)
                    lCode = mPrivate->GetRecordType(ExportAsciiDialog::STDF_V3,lName);
            }
            else if(mPrivate->mFieldsType == ExportAsciiDialog::STDF_UNKNOWN)
            {
                lCode = GQTL_STDF::Stdf_Record::Rec_COUNT;
            }
            else
            {
                lCode = mPrivate->GetRecordType(mPrivate->mFieldsType,lName);
            }
            clStdfToAtdf.SetProcessRecord(lCode);
        }
    }


    // test number filter
    if (checkBoxTNFilter->isChecked())
        clStdfToAtdf.setTNFilteringList(cmdTNFilterEdit->text());
    else
        clStdfToAtdf.setTNFilteringList("");
}

///////////////////////////////////////////////////////////
// New files have been selected by the user
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnFilesSelected()
{
    // Empty list of currently selected files
    mPrivate->m_pInputFiles.clear();
    treeWidgetInputFiles->clear();

    while (!mPrivate->m_pLookup.isEmpty())
        delete mPrivate->m_pLookup.takeFirst();

    // Extract selected files
    ExportAscii_InputFile	*pclInputFile;
    QString					strStatus;
    int						nFilesExtracted = 0;

    // Reset progress bar
    progressBar->reset();
    progressBar->setMaximum(mPrivate->m_strlistInputFiles.count());
    QCoreApplication::processEvents();

    // Save the type of the file to be dumped
    ExportAsciiDialog::STDFFields lFieldsType = ExportAsciiDialog::STDF_UNKNOWN;
    // Go through list of selected files
    for(QStringList::Iterator it = mPrivate->m_strlistInputFiles.begin(); it != mPrivate->m_strlistInputFiles.end(); ++it )
    {
        strStatus = "Status: Extracting file ";
        if((*it).length()>80)
            strStatus += "...";
        strStatus += (*it).right(80);
        strStatus += "...";
        labelStatus->setText(strStatus);
        progressBar->setValue(nFilesExtracted++);
        QCoreApplication::processEvents();

        if(checkBoxCommonOutputDir->isChecked())
            pclInputFile = mPrivate->m_pInputFiles.Append(*it, mPrivate->m_strOutputDir);
        else
            pclInputFile = mPrivate->m_pInputFiles.Append(*it);

        AddFileToListView(pclInputFile);

        if(pclInputFile && pclInputFile->m_pclFileInfo)
        {
            STDFFields lType = mPrivate->GetFileType(pclInputFile);
            if(lFieldsType!=lType)
            {
                if(lFieldsType == STDF_UNKNOWN)
                {
                    lFieldsType = lType;
                }
                else if(lFieldsType == STDF_V3 && lType == STDF_V4)
                {
                    lFieldsType = STDF_MIXED;
                }
                else if(lFieldsType == STDF_V4 && lType == STDF_V3)
                {
                    lFieldsType = STDF_MIXED;
                }
            }
        }
    }
    if(lFieldsType != mPrivate->mFieldsType)
    {
        UpdateSTDFFields(lFieldsType);
    }

    // Extraction done
    progressBar->reset();
    labelStatus->setText("Status: Ready");
    QCoreApplication::processEvents();

    // Update buttons
    UpdateButtons();
}

///////////////////////////////////////////////////////////
// Update buttons (enabled,disabled)
///////////////////////////////////////////////////////////
void ExportAsciiDialog::UpdateButtons()
{
    buttonEdit->setEnabled(false);
    buttonDump->setEnabled(false);
    buttonRepair->setEnabled(false);
    buttonProperties->setEnabled(false);

    // Edit button enabled if files already converted
    if(mPrivate->m_pInputFiles.m_uiNbFiles_Converted > 0)
        buttonEdit->setEnabled(true);

    // Properties button enabled if some STDF V4 files available
    if(mPrivate->m_pInputFiles.m_uiNbStdfFiles_V4 > 0)
        buttonProperties->setEnabled(true);

    // Dump all button enabled if some undumped files available
    if((mPrivate->m_pInputFiles.m_uiNbStdfFiles_V4 - mPrivate->m_pInputFiles.m_uiNbFiles_Converted) > 0)
        buttonDump->setEnabled(true);

    // Repair button available if some repairable files available
    if(mPrivate->m_pInputFiles.m_uiNbStdfFiles_V4_Repairable > 0)
        buttonRepair->setEnabled(true);
}

///////////////////////////////////////////////////////////
// Add files from InputFile to the list view
///////////////////////////////////////////////////////////
void ExportAsciiDialog::AddFileToListView(ExportAscii_InputFile* pclInputFile)
{
    ExportAscii_FileInfo	*pclFileInfo = pclInputFile->m_pclFileInfo;
    QString					strFileSize = QString::number(pclFileInfo->m_uiFileSize) + " bytes";
    QTreeWidgetItem			*pItem, *pItemChild;

    pItem = new QTreeWidgetItem(treeWidgetInputFiles);
    pItem->setText(0, pclFileInfo->m_strFileName);
    pItem->setText(1, pclFileInfo->m_strFileType);
    pItem->setText(2, "N");
    pItem->setText(3, strFileSize);

    mPrivate->m_pLookup.append(new ExportAsciiDialog_LookupItem(pItem, pclInputFile->m_pclFileInfo, pclInputFile));

    switch(pclFileInfo->m_eFileType)
    {
        case ExportAscii_FileInfo::FileType_Compressed:
            {
                pItem->setText(2, "");
                pItem->setDisabled(false);

                QList<ExportAscii_FileInfo*>::iterator itBegin	= pclInputFile->m_pExtractedFiles.begin();
                QList<ExportAscii_FileInfo*>::iterator itEnd	= pclInputFile->m_pExtractedFiles.end();

                while(itBegin != itEnd)
                {
                    strFileSize = QString::number((*itBegin)->m_uiFileSize) + " bytes";

                    pItemChild = new QTreeWidgetItem(pItem);
                    pItemChild->setText(0, (*itBegin)->m_strFileName);
                    pItemChild->setText(1, (*itBegin)->m_strFileType);
                    pItemChild->setText(2, "N");
                    pItemChild->setText(3, strFileSize);

                    mPrivate->m_pLookup.append(new ExportAsciiDialog_LookupItem(pItemChild, (*itBegin), pclInputFile));
                    if((*itBegin)->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4
                    || (*itBegin)->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V3)
                        pItemChild->setDisabled(false);
                    else
                        pItemChild->setDisabled(true);

                    itBegin++;
                }
            }
            break;

        case ExportAscii_FileInfo::FileType_STDF_V4:
        case ExportAscii_FileInfo::FileType_STDF_V3:
            pItem->setDisabled(false);
            break;

        default:
            pItem->setDisabled(true);
            break;
    }
}

///////////////////////////////////////////////////////////
// output format changed
///////////////////////////////////////////////////////////
void ExportAsciiDialog::OnChangeOutputFormat()
{
    SetDumpEnabled();
    if(comboOutputFormat->currentIndex() == 0)
    {
        comboFieldFilter->setEnabled(true);
    }
    else
    {
        comboFieldFilter->setEnabled(false);
    }
}

///////////////////////////////////////////////////////////
// User has right-clicked on one list view item: eventually
// display appropriate popup menu
///////////////////////////////////////////////////////////
void ExportAsciiDialog::onContextMenuRequested(const QPoint& ptPoint)
{
    ExportAscii_FileInfo *	pclFileInfo = GetCurrentFileInfo();
    QMenu					popupMenu(this);
    bool					bDisplayMenu = false;

    // Check if item found
    if(!pclFileInfo)
        return;

    if(pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4)
    {
        bDisplayMenu = true;

        // Create popup menu
        QAction * pActionMenu = popupMenu.addAction("Properties", this, SLOT(OnFileProperties()));
        pActionMenu->setWhatsThis("Show file properties");

        pActionMenu = popupMenu.addAction("Dump", this, SLOT(OnFileDump()));
        pActionMenu->setWhatsThis("Dump STDF file");

        if(pclFileInfo->m_bFileDumped)
        {
            pActionMenu  = popupMenu.addAction("Edit", this, SLOT(OnFileEditAscii()));
            pActionMenu->setWhatsThis("Edit converted ascii file");
        }
    }

        // Check if item is a STDF V4 file that can be repaired
    if(pclFileInfo->m_eFileType == ExportAscii_FileInfo::FileType_STDF_V4_FAR_NotFirst)
    {
        bDisplayMenu = true;

        // Create popup menu
        QAction * pActionMenu = popupMenu.addAction("Repair", this, SLOT(OnFileRepair()));
        pActionMenu->setWhatsThis("Repair corrupted STDF file");
    }

    // Show popup menu
    if (bDisplayMenu)
    {
        popupMenu.setMouseTracking(true);
        popupMenu.exec(treeWidgetInputFiles->mapToGlobal(ptPoint));
        popupMenu.setMouseTracking(false);
    }
}

void ExportAsciiDialog::updateDialogButton(int){

    buttonDump->setEnabled(true);
    ExportAscii_FileInfo *pclFileInfo = mPrivate->m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4);
    while(pclFileInfo)
    {
    pclFileInfo->m_bFileDumped = false;
    pclFileInfo = mPrivate->m_pInputFiles.NextStdf(ExportAscii_FileInfo::FileType_STDF_V4);
    }
}

void ExportAsciiDialog::UpdateSTDFFields(const STDFFields &field)
{
    mPrivate->mFieldsType = field;
    // Remove the Container of the record List
    if(mRecordsGroupLayout && mFieldsCheckBox)
    {
        // Remove the mFieldsCheckBox Widget
        mRecordsGroupLayout->removeWidget(mFieldsCheckBox);
        mFieldsCheckBox->setParent(0);
        mFieldsCheckBox->deleteLater();
    }

    //Re instantiate the mFieldsCheckBox
    mFieldsCheckBox = new QFrame(mRecordsGroup);
    mFieldsCheckBoxLayout = new QGridLayout(mFieldsCheckBox);
    mRecordsGroupLayout->addWidget(mFieldsCheckBox, 0, 0, 1, 1);

    QStringList lFields;
    QStringList lAdditionalFields;
    QString lAdditionalFieldsLabel;
    // Init the fieldsList
    GetSTDFFields(field, lFields, lAdditionalFields, lAdditionalFieldsLabel);
    //Build the GUI depending on the  lFields content
    if(lFields.isEmpty())//Temporary if the lFields is empty show the STDFv4 Record and disable the GUI
    {
        lFields = mPrivate->mSTDFV4Records;
        lAdditionalFields = mPrivate->mSTDFV4AdditionalRecords;
        lAdditionalFieldsLabel = "Additional STDF V4 - 2007 Record";
    }
    int lMainRecordColNumber = (lFields.count()/5);//Number of column to be displayed
    if((lFields.count()%5)>0)
        lMainRecordColNumber+=1;

    QGridLayout *lRecordGridSet = new QGridLayout();
    for(int lIdx=0; lIdx<lMainRecordColNumber;++lIdx)
    {
        QFormLayout *lRecordForm = new QFormLayout();
        //Show CheckBox for Records from X to X+4
        int lStart = lIdx*5;
        int lEnd = qMin((lIdx*5) + 5,lFields.count());
        int lItemRow = 0;
        // Insert the check box
        for(int lRecord=lStart;lRecord<lEnd;++lRecord)
        {
            QCheckBox *lCheckBox = new QCheckBox(lFields[lRecord]);
            lCheckBox->setChecked(true);
            lRecordForm->setWidget(lItemRow++, QFormLayout::LabelRole,lCheckBox);
        }
        // add the group of check box created to the layout
        lRecordGridSet->addLayout(lRecordForm,0,lIdx,1,1);
    }

    // Update the layout conntent
    mFieldsCheckBoxLayout->addLayout(lRecordGridSet,0,1,1,1);

    if(!lAdditionalFields.isEmpty())
    {
        int lAddionalRecordColNumber = (lAdditionalFields.count()/3);//Number of column to be displayed
        if((lAdditionalFields.count()%3)>0)
            lAddionalRecordColNumber+=1;
        QGroupBox *lAdditionalFieldGroup = new QGroupBox (lAdditionalFieldsLabel);
        QGridLayout *lAdditionalFieldGroupLayout = new QGridLayout(lAdditionalFieldGroup);

        for(int lIdx=0; lIdx<lMainRecordColNumber;++lIdx)
        {
            QFormLayout *lRecordForm = new QFormLayout();
            //Show CheckBox for Records from X to X+2
            int lStart = lIdx*3;
            int lEnd = qMin((lIdx*3) + 3,lAdditionalFields.count());
            int lItemRow = 0;
            for(int lRecord=lStart;lRecord<lEnd;++lRecord)
            {
                QCheckBox *lCheckBox = new QCheckBox(lAdditionalFields[lRecord]);
                lCheckBox->setChecked(true);
                lRecordForm->setWidget(lItemRow++, QFormLayout::LabelRole,lCheckBox);
            }
            lAdditionalFieldGroupLayout->addLayout(lRecordForm,0,lIdx,1,1);
        }

        mFieldsCheckBoxLayout->addWidget(lAdditionalFieldGroup,1,1,1,1);
    }



}

void ExportAsciiDialog::GetSTDFFields(const STDFFields &field, QStringList &fields,
                                      QStringList &additionalFields, QString &additionalFieldsLabel)
{
    if(field == STDF_V4)
    {
        fields = mPrivate->mSTDFV4Records;
        additionalFields = mPrivate->mSTDFV4AdditionalRecords;
        additionalFieldsLabel = "Additional STDF V4 - 2007 Record";

    }
    else if(field == STDF_V3)
    {
        fields = mPrivate->mSTDFV3Records;
        additionalFields.clear();
        additionalFieldsLabel.clear();
    }
    else if(field == STDF_MIXED)
    {
        fields = mPrivate->mSTDFV4Records+mPrivate->mSTDFV4AdditionalRecords;
        qSort(fields.begin(), fields.end());
        fields.removeDuplicates();
        additionalFields = mPrivate->mSTDFV4AdditionalRecords;
        additionalFieldsLabel = "Additional STDF V4 - 2007 Record";
    }
    else if(field == STDF_UNKNOWN)
    {
        fields.clear();
        additionalFields.clear();
    }
}

}
}
