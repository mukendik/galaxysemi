#ifndef DRILL_EDITLAYER_DIALOG_H
#define DRILL_EDITLAYER_DIALOG_H

#include "ui_drill_editlayer_dialog.h"
#include "interactive_charts.h"		// Layer classes, etc

/////////////////////////////////////////////////////////////////////////////
class EditLayerDialog : public QDialog, public Ui::drill_editlayer_basedialog
{
	Q_OBJECT

public:
	
	EditLayerDialog( QWidget* parent = 0, bool modal = false, Qt::WindowFlags f = 0 );
	
	void setLayerInfo(CGexSingleChart *pLayer, int nChartType);
	void getLayerInfo(CGexSingleChart *pLayer);
	CGexSingleChart *getLayerInfo(void);
	void resetVariables(int iEntry=0);
	bool	needResetViewport(void);

private:
	
	CGexSingleChart cLayer;
	// Flags that one axis now holds a new parameter...so Charts will need to be repaint + clear zoom.
	bool	m_bNewParameterSelected;
	int		m_MarkerSelected;
	bool OnPickTest(unsigned int &lLayerTestNumber,int &lLayerPinmapIndex,int &iGroupID,QString &strLayerTestName,QString &strLayerLabel);

public slots:
	
	void	OnPickTestX(void);
	void	OnPickTestY(void);
	void	OnPickTestZ(void);
	void	OnLayerTitle(const QString &);
	void	OnSelectMarker(int iMarker);
	void	OnMarkerWidth(int);
	void	OnMarkerColorChanged(const QColor&);
	void	OnOk(void);
};

#endif // DRILL_EDITLAYER_DIALOG_H
