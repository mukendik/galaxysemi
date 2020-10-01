#ifndef DRILL_EDITCHART_OPTIONS_DIALOG_H
#define DRILL_EDITCHART_OPTIONS_DIALOG_H

#include "ui_drill_editchart_options_dialog.h"
#include "interactive_charts.h"		// Layer classes, etc

/////////////////////////////////////////////////////////////////////////////
class EditChartOptionsDialog : public QDialog, public Ui::drill_editchart_options_basedialog
{
	Q_OBJECT

public:
	
	EditChartOptionsDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	
	void setVariables(CGexChartOverlays *);
	void getVariables(CGexChartOverlays *);

private:
	
	CGexChartOptions cOptions;

public slots:

	void	OnOk(void);
	void	OnToggleTitle(bool);
	void	OnToggleAxisX(bool);
	void	OnToggleAxisY(bool);
};

#endif // DRILL_EDITCHART_OPTIONS_DIALOG_H
