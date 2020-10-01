///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "browser_dialog.h"
#include "auto_repair_dialog.h"
#include "report_build.h"
#include "gex_report.h"
#include <gqtl_log.h>

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CGexReport* gexReport;

///////////////////////////////////////////////////////////////////////////////////
// Class CGexAutoRepairDialog - class which prompts user how to manage corrupted stdf files
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexAutoRepairDialog::CGexAutoRepairDialog(QWidget * pParent, const QString& strFileName) : QDialog(pParent)
{
	// Setup the dialog
	setupUi(this);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	// Set to the default option
	m_nAutoRepairOption = CGexReport::repairTempFile;

	radioButtonTempFile->setChecked(true);
	pushButtonYes->setChecked(true);

	// Set stdf file to repair
	labelFileName->setText(strFileName);

	// Initialize connections
	connect(pushButtonYes,		SIGNAL(clicked()), this, SLOT(accept()));
	connect(pushButtonYesAll,	SIGNAL(clicked()), this, SLOT(onClickYesAll()));
	connect(pushButtonNo,		SIGNAL(clicked()), this, SLOT(reject()));

	connect(radioButtonTempFile,	SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
	connect(radioButtonReplaceFile,	SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
	connect(radioButtonCreateFile,	SIGNAL(clicked(bool)), this, SLOT(onOptionChanged()));
}

///////////////////////////////////////////////////////////
// Desctructor
///////////////////////////////////////////////////////////
CGexAutoRepairDialog::~CGexAutoRepairDialog()
{
}

///////////////////////////////////////////////////////////
// SLOTS
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onOptionChanged()
//
// Description	:	Called when user click on an option buttons
//
///////////////////////////////////////////////////////////////////////////////////
void CGexAutoRepairDialog::onOptionChanged()
{
	// Set the option checked
	if (radioButtonTempFile->isChecked())
		m_nAutoRepairOption = CGexReport::repairTempFile;
	else if (radioButtonReplaceFile->isChecked())
		m_nAutoRepairOption = CGexReport::repairReplaceFile;
	else if (radioButtonCreateFile->isChecked())
		m_nAutoRepairOption = CGexReport::repairCreateFile;
	else
	{
		GEX_ASSERT(false);
		GSLOG(SYSLOG_SEV_WARNING, " option checked is not yet managed...");
		m_nAutoRepairOption = CGexReport::repairTempFile;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void onClickYesAll()
//
// Description	:	Called when user click on 'Yes to all' button
//
///////////////////////////////////////////////////////////////////////////////////
void CGexAutoRepairDialog::onClickYesAll()
{
	if (gexReport)
		gexReport->setAutoRepair((CGexReport::autoRepairOption) m_nAutoRepairOption);

	// Call accept method to exit from the dialog
	accept();
}
