#undef QT3_SUPPORT
///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "pick_export_wafermap_dialog.h"
#include "gqtl_skin.h"

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CGexSkin* pGexSkin;

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	PickExportWafermapDialog
//
// Description	:	Class used to define the output format for wafermap
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PickExportWafermapDialog::PickExportWafermapDialog(outputFormat eFormat /*= outputFormat::outputAll*/, QWidget * pParent /*= NULL*/, Qt::WindowFlags fl /*= 0*/) : QDialog(pParent, fl)
{
	setupUi(this);

	// Apply Examinator palette
	if (pGexSkin)
		pGexSkin->applyPalette(this);
	
	QObject::connect(PushButtonOk,				SIGNAL(clicked()),					this, SLOT(accept()));
    QObject::connect(PushButtonCancel,			SIGNAL(clicked()),					this, SLOT(reject()));
	QObject::connect(checkBoxExportAllWafers,	SIGNAL(stateChanged(int)),			this, SLOT(onUpdateUI()));
	QObject::connect(comboBoxOutputFormat,		SIGNAL(currentIndexChanged(int)),	this, SLOT(onUpdateUI()));

	// Fill the output format combo box
	fillFormat(eFormat);

	// Fill the notch combo box
	fillNotch();

	// Fill the mode combo box
	fillMode();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void onUpdateUI()
//
// Description	:	
//
///////////////////////////////////////////////////////////////////////////////////
void PickExportWafermapDialog::onUpdateUI()
{
    // Show mode combo box if HTML format
    if (checkBoxExportAllWafers->isChecked() && comboBoxOutputFormat->itemData(comboBoxOutputFormat->currentIndex()).toInt() == outputHtml)
		comboBoxExportMode->show();
	else
	{
		comboBoxExportMode->hide();
		comboBoxExportMode->setCurrentIndex(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void fillFormat(outputFormat eMask)
//
// Description	:	Fill the format combo box
//
///////////////////////////////////////////////////////////////////////////////////
void PickExportWafermapDialog::fillFormat(outputFormat eMask)
{
	if ((outputTSMCink & eMask) == outputTSMCink)
		comboBoxOutputFormat->addItem("TSMC inkless", QVariant(outputTSMCink));

	if ((outputSemiG85inkAscii & eMask) == outputSemiG85inkAscii)
		comboBoxOutputFormat->addItem("SEMI G85 inkless - Ascii", QVariant(outputSemiG85inkAscii));
	
	if ((outputSemiG85inkXml & eMask) == outputSemiG85inkXml)
		comboBoxOutputFormat->addItem("SEMI G85 inkless - XML Format", QVariant(outputSemiG85inkXml));
	
	if ((outputSemiE142ink & eMask) == outputSemiE142ink)
		comboBoxOutputFormat->addItem("SEMI E142 inkless", QVariant(outputSemiE142ink));

    if ((outputSemiE142inkInteger2 & eMask) == outputSemiE142inkInteger2)
        comboBoxOutputFormat->addItem("SEMI E142-Integer2 inkless", QVariant(outputSemiE142inkInteger2));

	if ((outputKLA_INF & eMask) == outputKLA_INF)
		comboBoxOutputFormat->addItem("KLA/INF", QVariant(outputKLA_INF));
	
    if ((outputHtml & eMask) == outputHtml)
        comboBoxOutputFormat->addItem("HTML", QVariant(outputHtml));
	
	if ((outputLaurierDieSort1D & eMask) == outputLaurierDieSort1D)
		comboBoxOutputFormat->addItem("Laurier Die Sort 1D", QVariant(outputLaurierDieSort1D));

	if ((outputPng & eMask) == outputPng)
		comboBoxOutputFormat->addItem("Image / PNG", QVariant(outputPng));

	if ((outputSTIF & eMask) == outputSTIF)
		comboBoxOutputFormat->addItem("STIF - ST Inkless Format", QVariant(outputSTIF));

}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void fillNotch()
//
// Description	:	Fill the notch combo box
//
///////////////////////////////////////////////////////////////////////////////////
void PickExportWafermapDialog::fillNotch()
{
	comboBoxNotchDirection->addItem("Default",	QVariant(notchDefault));
	comboBoxNotchDirection->addItem("Up",		QVariant(notchUp));
	comboBoxNotchDirection->addItem("Down",		QVariant(notchDown));
	comboBoxNotchDirection->addItem("Right",	QVariant(notchRight));
	comboBoxNotchDirection->addItem("Left",		QVariant(notchLeft));
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void fillMode()
//
// Description	:	Fill the mode combo box
//
///////////////////////////////////////////////////////////////////////////////////
void PickExportWafermapDialog::fillMode()
{
	comboBoxExportMode->addItem("One wafer per file",		QVariant(modeOneWaferPerFile));
	comboBoxExportMode->addItem("All wafer in one file",	QVariant(modeAllWaferInOneFile));
	
	comboBoxExportMode->hide();
}
	
///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	PickExportWafermapDialog::outputFormat format() const
//
// Description	:	Get the selected output format
//
///////////////////////////////////////////////////////////////////////////////////
PickExportWafermapDialog::outputFormat PickExportWafermapDialog::format() const
{
	return (PickExportWafermapDialog::outputFormat) comboBoxOutputFormat->itemData(comboBoxOutputFormat->currentIndex()).toInt();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	PickExportWafermapDialog::outputFormat notchDirection() const
//
// Description	:	Get the selected output format
//
///////////////////////////////////////////////////////////////////////////////////
PickExportWafermapDialog::notch PickExportWafermapDialog::notchDirection() const
{
	return (PickExportWafermapDialog::notch) comboBoxNotchDirection->itemData(comboBoxNotchDirection->currentIndex()).toInt();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	PickExportWafermapDialog::outputFormat exportMode() const
//
// Description	:	Fill the mode combo box
//
///////////////////////////////////////////////////////////////////////////////////
PickExportWafermapDialog::mode PickExportWafermapDialog::exportMode() const
{
	return (PickExportWafermapDialog::mode) comboBoxExportMode->itemData(comboBoxExportMode->currentIndex()).toInt();
}

