///////////////////////////////////////////////////////////
// Class linked to a dialog, to select operation and validate
// some operation (initial use for csv operation)
///////////////////////////////////////////////////////////

#ifndef TB_CSV_OPERATIONS_DIALOG_H
#define TB_CSV_OPERATIONS_DIALOG_H

#include <QDialog>
#include <QList>
#include <QStringList>

#include <QMap>

class GexToolboxTable;

namespace Ui {
    class ToolBoxCSVOperationsDialog;
}

class ToolBoxCSVOperationsDialog : public QDialog {
    Q_OBJECT
public:
	ToolBoxCSVOperationsDialog(GexToolboxTable &excelTable, bool bIsColumnUsed, long lStartingSelection, long lParameterNameLine, int iOpenMode = 0, QWidget *parent = 0);
    ~ToolBoxCSVOperationsDialog();
	double scale()		{return m_dScale;}			// Return the scale
	double offset()		{return m_dOffset;}			// Return the offset
	bool updateLimits()	{return m_bUpdateLimits;}	// Return true if the limits have to be updated, else false
	void setScale(double dScale);					// Set the scale
	void setOffset(double dOffset);					// Set the offset
	void setUpdateLimits(bool bUpdateLimits);		// Set if the limits have to be updated or not
	void setTestNumber(QString strTestNumber);		// Set Test Number
	void setHighLimit(double dLimit);				// Set High Limit
	void setLowLimit(double dLimit);				// Set Low Limit
	void setFormula(QString strFormula);			// Set formula
	void setTestName(QString strName);				// Set Test Name
	void setTestPattern(QString strTestPatterns);	// Set Test Pattern
	void setTestUnit(QString strTestUnit);			// Set Test Unit

	void retrieveSelectedColumns(QList<int> &iSelectedColumns, GexToolboxTable &excelTable);
	void retrieveSelectedRows(QList<int> &iSelectedRows, GexToolboxTable &excelTable);
	void applyScaleOffset();
	void applyScaleOffsetOnColumns(QList<int> &iSelectedColumns, GexToolboxTable &excelTable, long lStartingSelection);
	void applyScaleOffsetOnRows(QList<int> &iSelectedRows, GexToolboxTable &excelTable, long lStartingSelection);
	bool evaluateVars(int iPosition);
	void applyFormula();
	void applyFomulaOnColumns();
	void applyFomulaOnRows();

	QMap<QString, double> m_lstVarValue;			// List of mapped variable name linked to value


public slots:
	void onAcceptDialog();							// When the button ok is clicked in the button box
	void onCurrentChanged();						// When current tabwidget change
private:
	void initGui();									// Init Gui Value
	bool retrieveGuiData();						// Retrieve field of the dialog to the member attributes
	void retrieveTableData();
	double roundDouble(double dValue, int iPrecision);
	int getColumnLinkedToTestNumber(QString strTestNumber);
	int getRowLinkedToTestNumber(QString strTestNumber);

	void onErrorOccured(QString strErrorMsg);
	Ui::ToolBoxCSVOperationsDialog	*m_ui;			// Pointer on Gui
	double							m_dScale;		// Scale
	double							m_dOffset;		// Offset
	bool							m_bUpdateLimits;// Limits to update or not
	double							m_dHighLimit;	// High limit
	double							m_dLowLimit;	// Low limit
	QString							m_strTestNumber;  // Test Number
	QString							m_strTestName;  // Test Name
	QString							m_strTestPattern;// Test Patterns
	QString							m_strTestUnit;	// Test Units
	QString							m_strFormula;	// Formula
	QStringList						m_strSelection; // List of selected rows or columns
	QList<int>						m_iSelection;	// List of selected columns
	GexToolboxTable					&m_excelTable;	// Table of data
	bool							m_bIsColumnUsed;// If test are in column or row
	long							m_lStartingSelection;
	long							m_lParameterNameLine;
};

#endif // TB_CSV_OPERATIONS_DIALOG_H
