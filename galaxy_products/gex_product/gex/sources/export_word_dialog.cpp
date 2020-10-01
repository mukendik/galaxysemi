// Export to WORD format

#include <qfiledialog.h>
#include <qstring.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "message.h"
#include "export_word_dialog.h"
#include "engine.h"
#include "gex_word_report.h"
#include "browser_dialog.h"

ExportWordDialog::ExportWordDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
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
    QObject::connect(this,				SIGNAL(sHtmlFileSelected(QString)), editFileName,	SLOT(setText(QString)));
    QObject::connect(this,				SIGNAL(sHtmlFileSelected(QString)), this,			SLOT(OnHtmlFileSelected()));

	m_strHtmlFileName = "";

	// Disable Export button
	buttonExport->setEnabled(false);

	// Disable Properties button
	buttonProperties->setEnabled(false);

	// Set some validators
    m_pMarginValidator = new QDoubleValidator( 0.0, 10.0, 2, this );
	editMargin_left->setValidator(m_pMarginValidator);
	editMargin_right->setValidator(m_pMarginValidator);
	editMargin_top->setValidator(m_pMarginValidator);
	editMargin_bottom->setValidator(m_pMarginValidator);
}

ExportWordDialog::~ExportWordDialog()
{
}

void ExportWordDialog::OnButtonSelect()
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

void ExportWordDialog::OnButtonProperties()
{
	return;
}

void ExportWordDialog::OnButtonExport()
{
	CGexWordReport	clWordReport;
	QString			strDestination;

	strDestination = m_strHtmlFileName + ".doc";

	// Retrieve Word Options
	GexWordOptions	stGexWordOptions;
	// Show Word?
	stGexWordOptions.m_bShowWordApplication = checkBoxShowWordApplication->isChecked();
	stGexWordOptions.m_bMinimizeApplication = checkBoxMinimizeWordApplication->isChecked();
	// Paper size
	if(comboPaperSize->currentText() == "A4")
		stGexWordOptions.m_nPaperFormat = GexWordOptions::ePaperFormatA4;
	else
		stGexWordOptions.m_nPaperFormat = GexWordOptions::ePaperFormatLetter;
	// Paper orientation
	if(comboPaperOrientation->currentText() == "Portrait")
		stGexWordOptions.m_nPaperOrientation = GexWordOptions::ePaperOrientationPortrait;
	else
		stGexWordOptions.m_nPaperOrientation = GexWordOptions::ePaperOrientationLandscape;
	// Margins
	stGexWordOptions.m_lfMargin_Left = editMargin_left->text().toDouble();
	stGexWordOptions.m_lfMargin_Right = editMargin_right->text().toDouble();
	stGexWordOptions.m_lfMargin_Top = editMargin_top->text().toDouble();
	stGexWordOptions.m_lfMargin_Bottom = editMargin_bottom->text().toDouble();
	// Footer
    stGexWordOptions.m_strFooter.sprintf("--Report created with: %s - www.mentor.com--",
      GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );

	// Generate Word document
	int nStatus = clWordReport.GenerateDocFromHtml(this, stGexWordOptions, m_strHtmlFileName, strDestination);

	switch(nStatus)
	{
		case CGexWordReport::NoError:
            GS::Gex::Message::information(
                "Export",
                "Success exporting HTML file to Word.");
			break;
		case CGexWordReport::ConversionCancelled:
            GS::Gex::Message::warning(
                "Export",
                "Error exporting HTML file to Word: ConversionCancelled.");
			break;
		case CGexWordReport::NotSupported:
            GS::Gex::Message::warning(
                "Export",
                "Error exporting HTML file to Word: NotSupported.");
			break;
		case CGexWordReport::Err_RemoveDestFile:
            GS::Gex::Message::warning(
                "Export",
                "Error exporting HTML file to Word: Err_RemoveDestFile.");
			break;
		case CGexWordReport::Err_LaunchScriptProcess:
            GS::Gex::Message::warning(
                "Export",
                "Error exporting HTML file to Word: Err_LaunchScriptProcess.");
			break;
		case CGexWordReport::Err_CreateScript:
            GS::Gex::Message::warning(
                "Export",
                "Error exporting HTML file to Word: Err_CreateScript.");
			break;
		case CGexWordReport::Err_ScriptError:
            GS::Gex::Message::warning(
                "Export",
                "Error exporting HTML file to Word: Err_ScriptError.");
			break;
	}

	progressBar->reset();
}

void ExportWordDialog::OnHtmlFileSelected()
{
	// Enable 'Export button'
	buttonExport->setEnabled(true);

	// Enable Properties button
	buttonProperties->setEnabled(true);
}
