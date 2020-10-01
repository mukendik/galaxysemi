/****************************************************************************
** Form interface generated from reading ui file 'chartdirector_dialog_base.ui'
**
** Created: Wed Mar 3 12:44:40 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.3   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CHARTDIRECTOR_DIALOG_H
#define CHARTDIRECTOR_DIALOG_H

#include <qframe.h>

#include "ui_chartdirector_dialog.h"

class QPaintEvent;

class ChartDirectorFrame : public QFrame
{
public:
    ChartDirectorFrame( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~ChartDirectorFrame();

	void	UpdateChart(int nChartType);

protected:
	int		m_nChartType;
	void	paintEvent(QPaintEvent *pPaintEvent);
	void	DrawBasicTestChart();
	void	DrawBasicHistogram();
	void	DrawBasicTrendChart();
	void	DrawUnderConstruction(const QString & strChartName);
	void	DrawChartDirDisabled();
};

class ChartDirectorDialog : public QDialog, public Ui::ChartDirectorDialog_base
{
	Q_OBJECT

public:

    ChartDirectorDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags fl = 0 );
    ~ChartDirectorDialog();

protected:

    ChartDirectorFrame* m_pFrame;

protected slots:

    void	OnChartSelection(int nChartType);
};

#endif // CHARTDIRECTOR_DIALOG_H
