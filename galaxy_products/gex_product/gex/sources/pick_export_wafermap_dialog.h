#ifndef _PICK_EXPORT_WAFERMAP_DIALOG_H_
#define _PICK_EXPORT_WAFERMAP_DIALOG_H_
#include "ui_pick_export_wafermap_dialog.h"

class PickExportWafermapDialog : public QDialog, public Ui::PickExportWafermapDialogBase
{
	Q_OBJECT 

public:

	enum output //Format
	{
        outputTSMCink			  = 0x0001,
        outputSemiG85inkAscii	  = 0x0002,
        outputSemiG85inkXml		  = 0x0004,
        outputSemiE142ink		  = 0x0008,
        outputKLA_INF			  = 0x0010,
        outputHtml			  = 0x0020,
        outputPng				  = 0x0040,
        outputSTIF				  = 0x0080,
        outputLaurierDieSort1D	  = 0x0100,
        outputSemiE142inkInteger2 = 0x0200,
        outputAll				  = 0xffff
	};

	Q_DECLARE_FLAGS(outputFormat, output)

	enum notch
	{
		notchDefault			= -1,
		notchUp					= 12,
		notchDown				= 6,
		notchRight				= 3,
		notchLeft				= 9		
	};
	
	enum mode
	{
		modeOneWaferPerFile,
		modeAllWaferInOneFile
	};

	PickExportWafermapDialog(outputFormat eFormat = PickExportWafermapDialog::outputAll, QWidget * pParent = 0, Qt::WindowFlags fl = 0);

	outputFormat		format() const;
	notch				notchDirection() const;
	mode				exportMode() const;

private:
	
	void				fillFormat(outputFormat eMask);
	void				fillNotch();
	void				fillMode();

protected slots:
	
	void				onUpdateUI();
};

#endif // _PICK_EXPORT_WAFERMAP_DIALOG_H_
