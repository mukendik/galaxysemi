// Export to Powerpoint format

#include <qfiledialog.h>
#include <qstring.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "message.h"
#include "export_ppt_dialog.h"
#include "engine.h"
#include "gex_ppt_report.h"
#include "browser_dialog.h"

ExportPptDialog::ExportPptDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(buttonExport,		SIGNAL(clicked()),					this,			SLOT(OnButtonExport()));
    QObject::connect(buttonClose,		SIGNAL(clicked()),					this,			SLOT(accept()));
    QObject::connect(buttonSelect,		SIGNAL(clicked()),					this,			SLOT(OnButtonSelect()));
    QObject::connect(buttonProperties,	SIGNAL(clicked()),					this,			SLOT(OnButtonProperties()));
    QObject::connect(this,				SIGNAL(sHtmlFileSelected(QString)),	editFileName,	SLOT(setText(QString)));
    QObject::connect(this,				SIGNAL(sHtmlFileSelected(QString)), this,			SLOT(OnHtmlFileSelected()));

	m_strHtmlFileName = "";

	// Disable Export button
	buttonExport->setEnabled(false);

	// Disable Properties button
	buttonProperties->setEnabled(false);
}

ExportPptDialog::~ExportPptDialog()
{
}

void ExportPptDialog::OnButtonSelect()
{
	// Display Open file dialog so the user can select a file
    QString strFileName = QFileDialog::getOpenFileName(this, "Select the HTML file you want to convert", "",
                                                       "HTML files (*.htm *.html)");

	// Check if new file selected
	if((strFileName.isEmpty() == true) || (m_strHtmlFileName == strFileName))
		return;

	// Make sure the file has a valid HTML extension
	QFileInfo	fileInfo(strFileName);
	QString		strExtension = fileInfo.suffix();

	if((strExtension != "htm") && (strExtension != "html"))
	{
        GS::Gex::Message::warning(
            "HTML file",
            "The selected file has not a valid HTML extension.\n"
            "Please select a file with a valid HTML extension (*.htm *.html)");
		return;
	}

	// The selected file is a valid HTML file, emit signal telling a new file has been selected
	m_strHtmlFileName = strFileName;
	emit sHtmlFileSelected(strFileName);
}

void ExportPptDialog::OnButtonProperties()
{
	return;
}

void ExportPptDialog::OnButtonExport()
{
	CGexPptReport	clPptReport;
	QString			strDestination;

	strDestination = m_strHtmlFileName + ".ppt";

	// Retrieve Powerpoint Options
	GexPptOptions	stGexPptOptions;
	// Show Powerpoint?
	stGexPptOptions.m_bShowPptApplication = checkBoxShowPptApplication->isChecked();
	stGexPptOptions.m_bMinimizeApplication = checkBoxMinimizePptApplication->isChecked();
	// Paper size
	if(comboPaperSize->currentText() == "A4")
		stGexPptOptions.m_nPaperFormat = ePaperFormatA4;
	else
		stGexPptOptions.m_nPaperFormat = ePaperFormatLetter;
	// Paper orientation
	if(comboPaperOrientation->currentText() == "Portrait")
		stGexPptOptions.m_nPaperOrientation = ePaperOrientationPortrait;
	else
		stGexPptOptions.m_nPaperOrientation = ePaperOrientationLandscape;
	// Footer
    stGexPptOptions.m_strFooter.sprintf("--Report created with: %s - www.mentor.com--",
      GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );

	// Generate Powerpoint document
	int nStatus = clPptReport.GeneratePptFromHtml(this, stGexPptOptions, m_strHtmlFileName, strDestination);

	switch(nStatus)
	{
    case CGexPptReport::NoError:
        GS::Gex::Message::warning(
            "Export",
            "Success exporting HTML file to Powerpoint.");
        break;
    case CGexPptReport::ConversionCancelled:
        GS::Gex::Message::warning(
            "Export",
            "Error exporting HTML file to Powerpoint: ConversionCancelled.");
        break;
    case CGexPptReport::NotSupported:
        GS::Gex::Message::warning(
            "Export",
            "Error exporting HTML file to Powerpoint: NotSupported.");
        break;
    case CGexPptReport::Err_RemoveDestFile:
        GS::Gex::Message::warning(
            "Export",
            "Error exporting HTML file to Powerpoint: Err_RemoveDestFile.");
        break;
    case CGexPptReport::Err_LaunchScriptProcess:
        GS::Gex::Message::warning(
            "Export",
            "Error exporting HTML file to Powerpoint: "
            "Err_LaunchScriptProcess.");
        break;
    case CGexPptReport::Err_CreateScript:
        GS::Gex::Message::warning(
            "Export",
            "Error exporting HTML file to Powerpoint: Err_CreateScript.");
        break;
    case CGexPptReport::Err_ScriptError:
        GS::Gex::Message::warning(
            "Export",
            "Error exporting HTML file to Powerpoint: Err_ScriptError.");
        break;
	}

	progressBar->reset();
}

void ExportPptDialog::OnHtmlFileSelected()
{
	// Enable 'Export button'
	buttonExport->setEnabled(true);

	// Enable Properties button
	buttonProperties->setEnabled(true);
}
