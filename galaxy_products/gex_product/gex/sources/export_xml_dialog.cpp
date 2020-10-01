#include <qfiledialog.h>
#include <qstring.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qapplication.h>

#include "browser_dialog.h"
#include "engine.h"
#include "export_xml_dialog.h"
#include "stdf_record_dialog.h"
#include "export_xml.h"
#include "export_ascii.h"
#include "message.h"

ExportXmlDialog::ExportXmlDialog( QWidget* parent, bool modal, Qt::WindowFlags fl )
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
    QObject::connect(this,				SIGNAL(sStdfFileSelected(QString)), editFileName,	SLOT(setText(QString)));
    QObject::connect(this,				SIGNAL(sStdfFileSelected(QString)), this,			SLOT(OnStdfFileSelected()));

	m_strStdfFileName = "";

	// Disable Export button
	buttonExport->setEnabled(false);

	// Disable Properties button
	buttonProperties->setEnabled(false);
}

ExportXmlDialog::~ExportXmlDialog()
{
}

void ExportXmlDialog::OnButtonSelect()
{
	// Display Open file dialog so the user can select a file
	QString strFileName = QFileDialog::getOpenFileName(this, "Select the STDF file you want to convert", "", "STDF files (*.std *.stdf)");

	// Check if new file selected
	if((strFileName.isEmpty() == true) || (m_strStdfFileName == strFileName))
		return;

	// Make sure the file has a valid STDF extension
	QFileInfo	fileInfo(strFileName);
	QString		strExtension = fileInfo.suffix();

	if((strExtension != "std") && (strExtension != "stdf"))
	{
        GS::Gex::Message::warning(
            "STDF file",
            "The selected file has not a valid STDF extension.\n"
            "Please select a file with a valid STDF extension (*.std *.stdf)");
		return;
	}

	// Make sure the file is a valid STDF file
    GQTL_STDF::StdfParse	stdfParser;
	if(stdfParser.Open(strFileName.toLatin1().constData()) == false)
	{
        GS::Gex::Message::warning(
            "STDF file",
            "The selected file is not a valid STDF file!\n"
            "Please select a valid STDF file");
		return;
	}
	stdfParser.Close();

	// The selected file is a valid STDF file, emit signal telling a new file has been selected
	m_strStdfFileName = strFileName;
	emit sStdfFileSelected(strFileName);
}

void ExportXmlDialog::OnButtonProperties()
{
    StdfRecordDialog dialog((GQTL_STDF::Stdf_Record *)&m_clMIR);
	dialog.exec();
	return;
}

void ExportXmlDialog::OnButtonExport()
{
	CSTDFtoXML	clStdfToXml;
	QString		strDestination;
	QDir		cDir;
	bool		bStatus;

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
	strDestination = m_strStdfFileName + ".xml";
	cDir.remove(strDestination);	// Make sure destination doesn't exist.
    bStatus = clStdfToXml.Convert(m_strStdfFileName.toLatin1().constData(), strDestination.toLatin1().constData(), progressBar);
    QGuiApplication::restoreOverrideCursor();

	if(bStatus == true)
        GS::Gex::Message::information("Export",
                                      "Success converting STDF file to XML.");
	else
	{
		QString strMessage;
		strMessage.sprintf("Error converting STDF file to XML.\n%s", clStdfToXml.GetLastError().toLatin1().constData());
        GS::Gex::Message::warning("Export", strMessage);
	}

	progressBar->reset();
}

void ExportXmlDialog::OnStdfFileSelected()
{
	// Enable 'Export button'
	buttonExport->setEnabled(true);

	// Enable Properties button
	buttonProperties->setEnabled(true);

	// Reset MIR
	m_clMIR.Reset();

	// Read MIR
    GQTL_STDF::StdfParse	stdfParser;
	if(stdfParser.Open(m_strStdfFileName.toLatin1().constData()) == false)
		return;
    if(stdfParser.LoadNextRecord(GQTL_STDF::Stdf_Record::Rec_MIR, true) == GQTL_STDF::StdfParse::NoError)
        stdfParser.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clMIR);
	stdfParser.Close();
}
