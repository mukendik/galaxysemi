#ifndef EXPORT_ASCII_DIALOG_H
#define EXPORT_ASCII_DIALOG_H

#include <qmenu.h>

#include "ui_export_ascii_dialog.h"
#include "export_ascii_file_info.h"
#include "export_ascii_input_file.h"
#include "export_ascii_input_filelist.h"
#include "export_ascii_dialog_lookup_item.h"

#include "export_ascii.h"
#include "export_atdf.h"
#include "stdfparse.h"


namespace GS
{
namespace Gex
{

class ExportAsciiDialog : public QDialog, public Ui::ExportAsciiDialog_base
{
	Q_OBJECT

public:
    enum STDFFields
    {
        STDF_UNKNOWN,
        STDF_V4,
        STDF_V3,
        STDF_MIXED
    };

    ExportAsciiDialog( bool bEvaluationMode, QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ExportAsciiDialog();

	// Overwrite virtual fcts from main class for Drag&Drop support.
	void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
	void						SetRecordsToProcess(CSTDFtoASCII & clStdfToAscii);
	void						SetRecordsToProcess(CSTDFtoATDF & clStdfToAtdf);
	bool						ConvertFile(CSTDFtoASCII & clStdfToAscii, ExportAscii_FileInfo *pclFileInfo);
	bool						ConvertFile(CSTDFtoATDF & clStdfToAtdf, ExportAscii_FileInfo *pclFileInfo);
	ExportAscii_FileInfo*		GetCurrentFileInfo();
	QTreeWidgetItem*			GetItem(ExportAscii_FileInfo *pclFileInfo);
	void						DisplayFileProperties(ExportAscii_FileInfo *pclFileInfo);
	void						EditAsciiFile(ExportAscii_FileInfo *pclFileInfo);
	void						AddFileToListView(ExportAscii_InputFile* pclInputFile);
	void						Dump(CSTDFtoASCII & clStdfToAscii, ExportAscii_FileInfo *pclFileInfo=NULL);
	void						Dump(CSTDFtoATDF & clStdfToAtdf, ExportAscii_FileInfo *pclFileInfo=NULL);
	bool						RepairFile(ExportAscii_FileInfo *pclFileInfo);
	void						UpdateButtons();
    /// \brief  Update the list of the records to show depending on the STDF type.
    /// \param field is the stdf version
    void UpdateSTDFFields(const STDFFields &field);

    /// \brief Init the record label fields list.
    /// \param field : the fields type
    /// \param fields : the fields label list
    /// \param additionalFields : additional fields list if exist
    /// \param additionalFieldsLabel : additional fields labels list if exist
    void GetSTDFFields(const STDFFields &field, QStringList &fields,
                       QStringList &additionalFields, QString &additionalFieldsLabel);
    /// \brief Update all checbox with parameter status
    /// \param status : the status value to be used to update the status
    void UpdateRecordStatus(bool status);



private slots:
    void OnButtonSelect();
    void OnButtonDump();
    void OnFilesSelected();
    void OnButtonSetAllRecords();
    void OnButtonClearAllRecords();
    void OnFileProperties();
	void OnFileEditAscii();
    void OnFileDoubleClicked();
    void OnButtonBrowseOutpuDir();
    void OnChangeOutputFormat();
	void onContextMenuRequested(const QPoint&);
	void OnButtonRepair();
    void OnFileDump();
    void OnFileRepair();
    void updateDialogButton(int);
    void SetDumpEnabled();
    void SetDumpEnabled(bool);
	
private:
    class ExportAsciiDialogPrivate *mPrivate;

signals:

	void sFilesSelected();
};

}
}
#endif // EXPORT_ASCII_DIALOG_H
