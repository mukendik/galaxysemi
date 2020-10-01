#ifndef PICKSERIE_DIALOG_H
#define PICKSERIE_DIALOG_H

#include <qvalidator.h>
#include <QTreeWidget>

#include "ui_pickserie_dialog.h"

class	CPickSerieFields
{
public:
	CPickSerieFields()
	{
		fAlarmLimit = 100;
	};
	
	QString	strTitle;			// Serie title
	QString	strBinType;			// Bin type (eg: PASS bins only)
	QString	strBinList;

	// Plotting options
	bool	bPlotSerie;			// Charting enabled for this serie.
	QString	strPlotStyle;		// 'Bars' or 'Lines'
	QColor	cSerieColor;		// Serie color.
	QString strDataLabels;		// Label position (none, top, left...)
	QString	strLineStyle;		// Line style (line, spline...)
	QString	strLineSpots;		// Spot shape (none, square...)
	QString	strLineProperty;	// Line property (solid, dashed,...)

	// Binning pareto over yield alarm...
	bool	bBinningParetoOverException;
	QString	strAlarmType;	// 'Yield less than..., 'Yield Greater than...'
	float	fAlarmLimit;	// number in 0-100

	// Table options
	QString	strTableData;	// 'yield', 'volume', 'yield & volume', etc...	
};

/////////////////////////////////////////////////////////////////////////////
class PickSerieDialog : public QDialog, public Ui::PickSerieDialogBase
{
	Q_OBJECT

public:
	PickSerieDialog(QTreeWidget* qtwPtrSQLYieldWizardSeriesView, QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0);

	void setBinningType(bool bSoftBin);			// Define if working over HBIN or SBIN
	void setFields(CPickSerieFields &cFields);
	void getFields(CPickSerieFields &cFields);

private:
	void		RefreshGuiFields(void);
	bool		m_WorkOverSOFT_BIN;
    QValidator*	m_pYieldLimitValidator;
	QTreeWidget*	m_qtwPtrSeriesView;
	
public slots:
	void OnBinType(void);
	void OnPickBinList(void);
    void OnButtonOK(void);
};

#endif // PICKSERIE_DIALOG_H
