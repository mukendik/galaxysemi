// Dialog box displaying the output of a process (Stdout and Stderr)
#include "processoutput_dialog.h"
#include "browser_dialog.h"

ProcessOutputDialog::ProcessOutputDialog( const QString& strCaption, const QString& strCommandLine, QWidget* parent, bool modal, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
	setupUi(this);
	setModal(modal);

	// Apply Examinator palette
	GexMainwindow::applyPalette(this);

	QObject::connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));

	// Set dialog caption
    setWindowTitle(strCaption);

	// Set command line
	editCommandLine->setText(strCommandLine);
}

/*
 *  Destroys the object and frees any allocated resources
 */
ProcessOutputDialog::~ProcessOutputDialog()
{
}

void ProcessOutputDialog::OnStdoutChanged(const QString& strStdout)
{
	editStdout->setText(strStdout);
}

void ProcessOutputDialog::OnStderrChanged(const QString& strStderr)
{
	editStderr->setText(strStderr);
}
