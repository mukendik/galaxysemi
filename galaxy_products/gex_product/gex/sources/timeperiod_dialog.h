#ifndef TIMEPERIOD_DIALOG_H
#define TIMEPERIOD_DIALOG_H
#include <qdatetime.h>

#include "ui_timeperiod_dialog.h"
#include "stdf.h"

/////////////////////////////////////////////////////////////////////////////
class TimePeriodDialog : public QDialog, public Ui::TimePeriodDialogBase
{
	Q_OBJECT

public:
	TimePeriodDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	QStringList ImportFile;

    // Error codes returned by Stdf functions
    enum ConvertErrorCode
    {
        NoError,		// No error
        NoFile,			// Input file doesn't exist!
        ReadError,		// Failed reading files to merge
        ReadCorrupted,	// Input file (in STDF format) didn't read well. Probably corrupted.
        WriteSTDF,		// Failed creating output STDF file
        BinStatusMissing       // Missing bin status
    };

public slots:

	void	OnFromDateCalendar(void);
	void	OnToDateCalendar(void);
	void	OnFromDateSpinWheel(const QDate&);
	void	OnToDateSpinWheel(const QDate&);
	void	OnBrowseFolder(void);
	void	OnOk(void);
	void	OnCancel(void);

private:
	
	void	UpdateFromToFields(bool);
	void	CheckFilesInDirectory(QString strFolder);
	void	CheckImportFile(QString strFile);
    void	ReadStringToField(GS::StdLib::Stdf *pStdfFile,char *szField);
	void	UpdateStatus(void);
	QDate	From;
	QDate	To;
	long	lFilesChecked;
	long	lFilesSelected;
    int ConvertToSTDFV4( const QString &fileName, QString &convertedFileName ) const;
};
#endif
