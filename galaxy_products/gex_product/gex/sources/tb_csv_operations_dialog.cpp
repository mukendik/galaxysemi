#include <QRegExp>
#include "math.h"
#include <qmath.h>
#include "message.h"
#include "tb_toolbox.h"
#include "gex_test_creator.h"
#include "product_info.h"
#include "gex_shared.h"
#include <gqtl_global.h>

#include "tb_csv_operations_dialog.h"
#include "ui_tb_csv_operations_dialog.h"

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
ToolBoxCSVOperationsDialog::ToolBoxCSVOperationsDialog(GexToolboxTable &excelTable, bool bIsColumnUsed, long lStartingSelection, long lParameterNameLine, int iOpenMode/* = 0*/, QWidget *parent/* = 0*/) :
	QDialog(parent),
	m_ui(new Ui::ToolBoxCSVOperationsDialog),
	m_excelTable(excelTable)
{
	m_bIsColumnUsed = bIsColumnUsed;
	m_lStartingSelection = lStartingSelection;
	if (m_bIsColumnUsed)
		m_lParameterNameLine = lParameterNameLine;
	else
		m_lParameterNameLine = 0;

	m_ui->setupUi(this);
    setWindowTitle("Edit Test"); // Tittle

	m_dScale = 1.0;
	m_dOffset = 0.0;
	m_bUpdateLimits = false;
	m_strSelection.clear();

	m_strTestName = "New Test";
	m_strTestNumber = "0";
	m_strTestPattern = "";
	m_strTestUnit = "";
	m_dHighLimit = 1.0;
	m_dLowLimit = 0.0;
	m_strFormula = "";

	retrieveTableData();
	initGui();
	connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAcceptDialog()));
	connect(m_ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentChanged()));
	m_ui->tabWidget->setCurrentIndex(iOpenMode);
	onCurrentChanged();
}

///////////////////////////////////////////////////////////
// Retrieve data from spreadsheet table
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::retrieveTableData()
{
    int iSelection = (m_iSelection.empty()) ? 0 : m_iSelection.first();
    if (m_bIsColumnUsed)
    {
        retrieveSelectedColumns(m_iSelection, m_excelTable);
        if (m_excelTable.item(m_lParameterNameLine, iSelection))
            m_strTestName = m_excelTable.item(m_lParameterNameLine, iSelection)->text();
        if (m_excelTable.item(m_lParameterNameLine + 1, iSelection))
            m_strTestNumber = m_excelTable.item(m_lParameterNameLine + 1, iSelection)->text();
        if (m_excelTable.item(m_lParameterNameLine + 2, iSelection))
            m_strTestPattern = m_excelTable.item(m_lParameterNameLine + 2, iSelection)->text();
        if (m_excelTable.item(m_lParameterNameLine + 3, iSelection))
            m_strTestUnit = m_excelTable.item(m_lParameterNameLine + 3, iSelection)->text();
        if (m_excelTable.item(m_lParameterNameLine + 4, iSelection))
            m_dHighLimit = m_excelTable.item(m_lParameterNameLine + 4, iSelection)->text().toDouble();
        if (m_excelTable.item(m_lParameterNameLine + 5, iSelection))
            m_dLowLimit = m_excelTable.item(m_lParameterNameLine + 5, iSelection)->text().toDouble();
    }
    else
    {
        retrieveSelectedRows(m_iSelection, m_excelTable);
        if (m_excelTable.item(iSelection, 0))
            m_strTestName = m_excelTable.item(iSelection, 0)->text();
        if (m_excelTable.item(iSelection, 1))
            m_strTestNumber = m_excelTable.item(iSelection, 1)->text();
        if (m_excelTable.item(iSelection, 2))
            m_strTestPattern = m_excelTable.item(iSelection, 2)->text();
        if (m_excelTable.item(iSelection, 3))
            m_strTestUnit = m_excelTable.item(iSelection, 3)->text();
        if (m_excelTable.item(iSelection, 4))
            m_dHighLimit = m_excelTable.item(iSelection, 4)->text().toDouble();
        if (m_excelTable.item(iSelection, 5))
            m_dLowLimit = m_excelTable.item(iSelection, 5)->text().toDouble();
}
}


///////////////////////////////////////////////////////////
// When current tabwidget change -> change widget title
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::onCurrentChanged()
{
	QString strSelectionType;
	if (m_bIsColumnUsed)
		strSelectionType = "Column";
	else
		strSelectionType = "Row";

	// Scale offset tab
	if (m_ui->tabWidget->currentIndex() == 0)
	{
        setWindowTitle("Edit " + strSelectionType + "(s) " + strSelectionType.left(1) + m_strSelection.join("-" + strSelectionType.left(1)));
	}
	// Apply formula tab
	else if (m_ui->tabWidget->currentIndex() == 1)
	{
        QString selection = (m_strSelection.empty()) ? "" : m_strSelection.first();
        setWindowTitle("Edit " + strSelectionType + " " + strSelectionType.left(1) + selection);
	}
}

///////////////////////////////////////////////////////////
// Initialize Gui value
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::initGui()
{
	// Scale/Offset Table
	m_ui->lineEditScale->setText(QString::number(m_dScale, 'g',15) + ".0");
	m_ui->lineEditOffset->setText(QString::number(m_dOffset, 'g',15) + ".0");
	m_ui->checkBoxUpdateLimits->setChecked(m_bUpdateLimits);
	// Apply Formula Table
	m_ui->lineEditTestName->setText(m_strTestName);
	m_ui->lineEditTestNumber->setText(m_strTestNumber);
	m_ui->lineEditTestPattern->setText(m_strTestPattern);
	m_ui->lineEditTestUnit->setText(m_strTestUnit);
	m_ui->lineEditHighLimit->setText(QString::number(m_dHighLimit, 'g',15));
	m_ui->lineEditLowLimit->setText(QString::number(m_dLowLimit, 'g',15));
	m_ui->lineEditFormula->setText(m_strFormula);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
ToolBoxCSVOperationsDialog::~ToolBoxCSVOperationsDialog()
{
	delete m_ui;
}

///////////////////////////////////////////////////////////
// Set Scale
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setScale(double dScale)
{
	m_dScale = dScale;
}

///////////////////////////////////////////////////////////
// Set Offset
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setOffset(double dOffset)
{
	m_dOffset = dOffset;
}

///////////////////////////////////////////////////////////
// Set update limits
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setUpdateLimits(bool bUpdateLimits)
{
	m_bUpdateLimits = bUpdateLimits;
}

///////////////////////////////////////////////////////////
// Set Test Number
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setTestNumber(QString strTestNumber)
{
	m_strTestNumber = strTestNumber;
}

///////////////////////////////////////////////////////////
// Set High Limit
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setHighLimit(double dLimit)
{
	m_dHighLimit = dLimit;
}

///////////////////////////////////////////////////////////
// Set Low Limit
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setLowLimit(double dLimit)
{
	m_dLowLimit = dLimit;
}

///////////////////////////////////////////////////////////
// Set formula
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setFormula(QString strFormula)
{
	m_strFormula = strFormula;
}


///////////////////////////////////////////////////////////
// Set Test Name
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setTestName(QString strTestName)
{
	m_strTestName = strTestName;
}

///////////////////////////////////////////////////////////
// Set Test Pattern
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setTestPattern(QString strTestPattern)
{
	m_strTestPattern = strTestPattern;
}

///////////////////////////////////////////////////////////
// Set Test Unit
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::setTestUnit(QString strTestUnit)
{
	m_strTestUnit = strTestUnit;
}


///////////////////////////////////////////////////////////
// Retrieve field of the dialog to the member attributes
///////////////////////////////////////////////////////////
bool ToolBoxCSVOperationsDialog::retrieveGuiData()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

	bool ok;
	double dScale, dOffset, dHLimit, dLLimit;
	QString strTestName, strTestPattern, strTestUnit, strFormula, strTestNumber;

	// User on Scale/Offset
	if (m_ui->tabWidget->currentIndex() == 0)
	{
		dScale = m_ui->lineEditScale->text().toDouble(&ok);
		if (ok == false) // If it's not a valid double
			return false;
		dOffset = m_ui->lineEditOffset->text().toDouble(&ok);
		if (ok == false) // If it's not a valid double
			return false;
		this->setScale(dScale);
		this->setOffset(dOffset);
		this->setUpdateLimits(m_ui->checkBoxUpdateLimits->isChecked());
	}
	// User on Apply formula
	else if (m_ui->tabWidget->currentIndex() == 1)
	{
		dHLimit = m_ui->lineEditHighLimit->text().toDouble(&ok);
		if (ok == false) // If it's not a valid double
			return false;
		dLLimit = m_ui->lineEditLowLimit->text().toDouble(&ok);
		if (ok == false) // If it's not a valid double
			return false;
		strTestNumber = m_ui->lineEditTestNumber->text();
		strTestName = m_ui->lineEditTestName->text();
		strTestPattern = m_ui->lineEditTestPattern->text();
		strTestUnit = m_ui->lineEditTestUnit->text();
		strFormula = m_ui->lineEditFormula->text();

		this->setTestName(strTestName);
		this->setTestPattern(strTestPattern);
		this->setTestUnit(strTestUnit);
		this->setTestNumber(strTestNumber);
		this->setHighLimit(dHLimit);
		this->setLowLimit(dLLimit);
		this->setFormula(strFormula);
	}
	return true;
}

///////////////////////////////////////////////////////////
// When the button ok is clicked in the dialog button box
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::onAcceptDialog()
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return;
    }

	retrieveGuiData();
	// User on Scale/Offset
	if (m_ui->tabWidget->currentIndex() == 0)
	{
		applyScaleOffset();
	}
	// User on Apply formula
	else if (m_ui->tabWidget->currentIndex() == 1)
	{
		applyFormula();
	}
}

///////////////////////////////////////////////////////////
// Process scale and offset on exceltable
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::applyScaleOffset()
{
	QList<int> iSelection;
	long lStartedSelection;
	lStartedSelection = m_lStartingSelection;
	iSelection = m_iSelection;

	if (updateLimits())
		lStartedSelection = lStartedSelection - 2;

	if (m_bIsColumnUsed)
		applyScaleOffsetOnColumns(iSelection, m_excelTable, lStartedSelection);
	else
		applyScaleOffsetOnRows(iSelection, m_excelTable, lStartedSelection);
}

///////////////////////////////////////////////////////////
// Retrieve from excelTable the selected columns by the user
// and hilight each one
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::retrieveSelectedColumns(QList<int> &iSelectedColumns, GexToolboxTable &excelTable)
{
    // Collect selected columns
    QList<QTableWidgetSelectionRange> selectedRegion = excelTable.selectedRanges();
    QList<QTableWidgetSelectionRange>::iterator itTableRange;
    for(itTableRange = selectedRegion.begin(); itTableRange != selectedRegion.end(); ++itTableRange)
    {
        int selectedCol = (*itTableRange).leftColumn();
        do
        {
            iSelectedColumns << selectedCol;
            QTableWidgetItem *item = excelTable.horizontalHeaderItem(selectedCol);
            if(item)
                m_strSelection << item->text();
            else
                m_strSelection <<  QString::number(selectedCol,10);
            ++selectedCol;
        }
        while(selectedCol <= (*itTableRange).rightColumn());
    }
}

///////////////////////////////////////////////////////////
// Retrieve from excelTable the selected rows by the user
// and hilight each one
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::retrieveSelectedRows(QList<int> &iSelectedRows, GexToolboxTable &excelTable)
{
    // Collect selected rows
    QList<QTableWidgetSelectionRange> selectedRegion = excelTable.selectedRanges();
    QList<QTableWidgetSelectionRange>::iterator itTableRange;
    for(itTableRange = selectedRegion.begin(); itTableRange != selectedRegion.end(); ++itTableRange)
    {
        int selectedRow = (*itTableRange).topRow();
        do
        {
            iSelectedRows << selectedRow;
            QTableWidgetItem *item = excelTable.verticalHeaderItem(selectedRow);
            if(item)
                m_strSelection << item->text();
            else
                m_strSelection <<  QString::number(selectedRow,10);
            ++selectedRow;
        }
        while(selectedRow <= (*itTableRange).bottomRow());
    }
}

///////////////////////////////////////////////////////////
// Process scale and offset on selected columns
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::applyScaleOffsetOnColumns(QList<int> &iSelectedColumns, GexToolboxTable &excelTable, long lStartingSelection)
{
    excelTable.blockSignals(true);
	bool ok;
	int iRow;
	double dParameterValue;
	QList<int>::iterator itSelectedColumns;

	QLocale::setDefault(QLocale::C);
	// For selected column
	for (itSelectedColumns = iSelectedColumns.begin(); itSelectedColumns != iSelectedColumns.end(); ++itSelectedColumns)
	{
		// For each row of the selected column
        for(iRow = lStartingSelection; iRow <= excelTable.rowCount(); iRow++)
		{
			// If the cell is not empty
            if (excelTable.item(iRow, (*itSelectedColumns))
                && !excelTable.item(iRow, (*itSelectedColumns))->text().trimmed().isEmpty())
			{
                dParameterValue = excelTable.item(iRow, (*itSelectedColumns))->text().toDouble(&ok);
				// If the value is not double
				if (ok == false)
					return;
                excelTable.item(iRow, (*itSelectedColumns))->text().section('.',1,1).size();
				dParameterValue = (dParameterValue * scale()) + offset();
				// If int value
				if (floor(dParameterValue) == dParameterValue)
                    excelTable.item(iRow, (*itSelectedColumns))->setText(QString::number(dParameterValue));
				// Else if double value
				else
                    excelTable.item(iRow, (*itSelectedColumns))->setText(QString::number(dParameterValue, 'g', 15));
			}
		}
	}
    excelTable.blockSignals(false);
}

///////////////////////////////////////////////////////////
// Process scale and offset on selected rows
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::applyScaleOffsetOnRows(QList<int> &iSelectedRows, GexToolboxTable &excelTable, long lStartingSelection)
{
	bool ok;
	int iCol;
	double dParameterValue;
	QList<int>::iterator itSelectedRows;

	QLocale::setDefault(QLocale::C);
	// For each selected rows
	for (itSelectedRows = iSelectedRows.begin(); itSelectedRows != iSelectedRows.end(); ++itSelectedRows)
	{
		// For each col of the selected row
        for(iCol = lStartingSelection; iCol <= excelTable.columnCount(); iCol++)
        {
			// If the cell is not empty
            if (excelTable.item((*itSelectedRows), iCol)
                && !excelTable.item((*itSelectedRows), iCol)->text().trimmed().isEmpty())
			{
                dParameterValue = excelTable.item((*itSelectedRows), iCol)->text().toDouble(&ok);
				// If the value is not double
				if (ok == false)
					return;
                excelTable.item((*itSelectedRows), iCol)->text().section('.',1,1).size();
				dParameterValue = (dParameterValue * scale()) + offset();
				// If int value
				if (floor(dParameterValue) == dParameterValue)
                    excelTable.item((*itSelectedRows), iCol)->setText(QString::number(dParameterValue));
				// Else if double value
				else
                    excelTable.item((*itSelectedRows), iCol)->setText(QString::number(dParameterValue, 'g', 15));
			}
		}
	}
}


///////////////////////////////////////////////////////////
// Return column linked to the test number
///////////////////////////////////////////////////////////
int ToolBoxCSVOperationsDialog::getColumnLinkedToTestNumber(QString strTestNumber)
{
	int iCol;
	// For each row
    for(iCol = 0; iCol <= m_excelTable.columnCount(); iCol++)
	{
        if (m_excelTable.item(m_lParameterNameLine + 1, iCol)
            && strTestNumber == m_excelTable.item(m_lParameterNameLine + 1, iCol)->text())
			return iCol;
	}
	return -1;
}

///////////////////////////////////////////////////////////
// Return row linked to the test number
///////////////////////////////////////////////////////////
int ToolBoxCSVOperationsDialog::getRowLinkedToTestNumber(QString strTestNumber)
{
	int iRow;
	// For each row
    for(iRow = 0; iRow <= m_excelTable.rowCount(); iRow++)
	{
        if (m_excelTable.item(iRow, m_lParameterNameLine + 1)
            && strTestNumber == m_excelTable.item(iRow, m_lParameterNameLine + 1)->text())
			return iRow;
	}
	return -1;
}

///////////////////////////////////////////////////////////
// Evaluate if var has a valide value and link it to the name
///////////////////////////////////////////////////////////
bool ToolBoxCSVOperationsDialog::evaluateVars(int iPosition)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      // OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

	QRegExp regExpColPosition("(C|c)\\d+");
	QRegExp regExpRowPosition("(R|r)\\d+");
	QRegExp regExpTestNumber("(T|t)\\d+(.\\d+)?");
	bool ok;
	int iPos = 0;

	m_lstVarValue.clear();

	if (m_bIsColumnUsed)
	{
		// While find matching expression
		while ((iPos = regExpTestNumber.indexIn(m_strFormula, iPos)) != -1)
		{
			QString strTestNumber = regExpTestNumber.cap().mid(1);
			int iCol = getColumnLinkedToTestNumber(strTestNumber);
            if ((iCol < 0) || (iCol > m_excelTable.columnCount()))
				return false;
            if (m_excelTable.item(iPosition, iCol)->text().trimmed().isEmpty())
				return false;
			else
                m_lstVarValue.insert(regExpTestNumber.cap(), m_excelTable.item(iPosition, iCol)->text().toDouble());
			iPos += regExpTestNumber.matchedLength();
		}

		iPos = 0;
		// While find matching expression
        while ((iPos = regExpColPosition.indexIn(m_strFormula, iPos)) != -1)
        {
            iPos += regExpColPosition.matchedLength();
			int iCol = regExpColPosition.cap().mid(1).toInt(&ok) - 1;
            if ((ok == false) || (iCol < 0) || (iCol > m_excelTable.columnCount()))
                return false;
            // if the current item is null or the content is empty
            if (!m_excelTable.item(iPosition, iCol) || m_excelTable.item(iPosition, iCol)->text().trimmed().isEmpty())
                return false;
            else
                m_lstVarValue.insert(regExpColPosition.cap(), m_excelTable.item(iPosition, iCol)->text().toDouble());
        }
	}
	else
	{
		// While find matching expression
		while ((iPos = regExpTestNumber.indexIn(m_strFormula, iPos)) != -1)
		{
			QString strTestNumber = regExpTestNumber.cap().mid(1);
			int iRow = getRowLinkedToTestNumber(strTestNumber);
            if ((iRow < 0) || (iRow > m_excelTable.rowCount()))
				return false;
            if (m_excelTable.item(iRow, iPosition)->text().trimmed().isEmpty())
				return false;
			else
                m_lstVarValue.insert(regExpTestNumber.cap(), m_excelTable.item(iRow, iPosition)->text().toDouble());
			iPos += regExpTestNumber.matchedLength();
		}

		iPos = 0;
		// While find matching expression
		while ((iPos = regExpRowPosition.indexIn(m_strFormula, iPos)) != -1)
		{
			int iRow = regExpRowPosition.cap().mid(1).toInt(&ok) - 1;
            if ((ok == false) || (iRow < 0) || (iRow > m_excelTable.rowCount()))
                return false;
            // if the current item is null or the content is empty
            if (!m_excelTable.item(iRow, iPosition) || m_excelTable.item(iRow, iPosition)->text().trimmed().isEmpty())
				return false;
			else
                m_lstVarValue.insert(regExpRowPosition.cap(), m_excelTable.item(iRow, iPosition)->text().toDouble());
			iPos += regExpRowPosition.matchedLength();
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
// Apply formula on selection
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::applyFormula()
{
    int selection = (m_iSelection.empty()) ? 0 : m_iSelection.first();
	if (m_bIsColumnUsed)
	{
		applyFomulaOnColumns();
//		m_excelTable.clearSelection(true);
        m_excelTable.selectColumn(selection);
	}
	else
	{
		applyFomulaOnRows();
//		m_excelTable.clearSelection(true);
        m_excelTable.selectRow(selection);
	}
}

///////////////////////////////////////////////////////////
// Apply formula on selected Columns
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::applyFomulaOnColumns()
{
    int selection = (m_iSelection.empty()) ? 0 : m_iSelection.first();
    QTableWidgetItem *item = m_excelTable.item(m_lParameterNameLine, selection);
    if (NULL == item)
        m_excelTable.setItem(m_lParameterNameLine, selection, new QTableWidgetItem());
    m_excelTable.item(m_lParameterNameLine, selection)->setText(m_strTestName + " : " + m_strFormula);

    item = m_excelTable.item(m_lParameterNameLine + 1, selection);
    if (NULL == item)
        m_excelTable.setItem(m_lParameterNameLine + 1, selection, new QTableWidgetItem());
    m_excelTable.item(m_lParameterNameLine + 1, selection)->setText( m_strTestNumber);

    item = m_excelTable.item(m_lParameterNameLine + 2, selection);
    if (NULL == item)
        m_excelTable.setItem(m_lParameterNameLine + 2, selection, new QTableWidgetItem());
    m_excelTable.item(m_lParameterNameLine + 2, selection)->setText( m_strTestPattern);

    item = m_excelTable.item(m_lParameterNameLine + 3, selection);
    if (NULL == item)
        m_excelTable.setItem(m_lParameterNameLine + 3, selection, new QTableWidgetItem());
    m_excelTable.item(m_lParameterNameLine + 3, selection)->setText( m_strTestUnit);

    item = m_excelTable.item(m_lStartingSelection - 2, selection);
    if (NULL == item)
        m_excelTable.setItem(m_lStartingSelection - 2, selection, new QTableWidgetItem());
    m_excelTable.item(m_lStartingSelection - 2, selection)->setText( QString::number(m_dHighLimit));

    item = m_excelTable.item(m_lStartingSelection - 1, selection);
    if (NULL == item)
        m_excelTable.setItem(m_lStartingSelection - 1, selection, new QTableWidgetItem());
     m_excelTable.item(m_lStartingSelection - 1, selection)->setText( QString::number(m_dLowLimit));

    // If no formule -> exit
	if (m_strFormula.trimmed().isEmpty())
		return;

	int iRow;
	CExEv cExpression;
	double dRes;
	Variant vres;

	// For each row
    for(iRow = m_lStartingSelection; iRow < m_excelTable.rowCount(); ++iRow)
	{
        int iSelection = (m_iSelection.empty()) ? 0 : m_iSelection.first();
		if (evaluateVars(iRow) == true)
		{
			ExpressionEvaluatorOnGex eectx(m_lstVarValue);
			cExpression.setContext(&eectx);
			int iStatus = cExpression.evalExpression(m_strFormula, vres);

			if(iStatus == CExEv::E_OK)
			{
				vres.asReal(dRes);
                if (!m_excelTable.item(iRow, iSelection))
                    m_excelTable.setItem(iRow, iSelection, new QTableWidgetItem());
				if (floor(dRes) == dRes)
                    m_excelTable.item(iRow, iSelection)->setText(QString::number(dRes));
				else
                    m_excelTable.item(iRow, iSelection)->setText(QString::number(dRes, 'g', 15));
			}
			else
			{
				onErrorOccured(QString(cExpression.getErrorMessage()));
				return;
			}
		}
		else
		{
            if (!m_excelTable.item(iRow, iSelection))
                m_excelTable.setItem(iRow, iSelection, new QTableWidgetItem());
            m_excelTable.item(iRow, iSelection)->setText("");
		}
	}
}

///////////////////////////////////////////////////////////
// Apply formula on selected Rows
///////////////////////////////////////////////////////////
void ToolBoxCSVOperationsDialog::applyFomulaOnRows()
{
    int iSelection = (m_iSelection.empty()) ? 0 : m_iSelection.first();
    if(!m_excelTable.item(iSelection, 0))
        m_excelTable.setItem(iSelection, 0, new QTableWidgetItem());
    m_excelTable.item(iSelection, 0)->setText( m_strTestName + " : " + m_strFormula);

    if(!m_excelTable.item(iSelection, 1))
        m_excelTable.setItem(iSelection, 1, new QTableWidgetItem());
    m_excelTable.item(iSelection, 1)->setText( m_strTestNumber);

    if(!m_excelTable.item(iSelection, 2))
        m_excelTable.setItem(iSelection, 2, new QTableWidgetItem());
    m_excelTable.item(iSelection, 2)->setText( m_strTestPattern);

    if(!m_excelTable.item(iSelection, 3))
        m_excelTable.setItem(iSelection, 3, new QTableWidgetItem());
    m_excelTable.item(iSelection, 3)->setText( m_strTestUnit);

    if(!m_excelTable.item(iSelection, m_lStartingSelection - 2))
        m_excelTable.setItem(iSelection, m_lStartingSelection - 2, new QTableWidgetItem());
    m_excelTable.item(iSelection, m_lStartingSelection - 2)->setText( QString::number(m_dHighLimit));

    if(!m_excelTable.item(iSelection, m_lStartingSelection - 1))
        m_excelTable.setItem(iSelection, m_lStartingSelection - 1, new QTableWidgetItem());
    m_excelTable.item(iSelection, m_lStartingSelection - 1)->setText( QString::number(m_dLowLimit));

	// If no formule -> exit
	if (m_strFormula.trimmed().isEmpty())
		return;

	int iCol;
	CExEv cExpression;
	Variant vres;
	double dRes;
//	QString strString;

	// For each col
    for(iCol = m_lStartingSelection; iCol < m_excelTable.columnCount(); iCol++)
	{
		if (evaluateVars(iCol) == true)
		{
			ExpressionEvaluatorOnGex eectx(m_lstVarValue);
			cExpression.setContext(&eectx);
			int iStatus = cExpression.evalExpression(m_strFormula, vres);

			if(iStatus == CExEv::E_OK)
			{
				vres.asReal(dRes);
                if(!m_excelTable.item(iSelection, iCol))
                    m_excelTable.setItem(iSelection, iCol, new QTableWidgetItem());
				if (floor(dRes) == dRes)
                    m_excelTable.item(iSelection, iCol)->setText( QString::number(dRes));
				else
                    m_excelTable.item(iSelection, iCol)->setText( QString::number(dRes, 'g', 15));
			}
			else
			{
				onErrorOccured(QString(cExpression.getErrorMessage()));
				return;
			}
		}
		else
		{
            if (!m_excelTable.item(iSelection, iCol))
                m_excelTable.setItem(iSelection, iCol, new QTableWidgetItem());
            m_excelTable.item(iSelection, iCol)->setText("");
		}
	}
}

void ToolBoxCSVOperationsDialog::onErrorOccured(QString strErrorMsg)
{
	QString strError = "Failed to evaluate formula!  ";
	strError += strErrorMsg + ": '";
	strError += m_strFormula + "'";
    GS::Gex::Message::warning("Error", strError);
}

///////////////////////////////////////////////////////////
// Round to nearest with iPrecision digit after the separator
///////////////////////////////////////////////////////////
double ToolBoxCSVOperationsDialog::roundDouble(double dValue, int iPrecision)
{
	if (dValue >= 0)
		return floor(dValue * GS_POW(10.0, iPrecision) + 0.5) * (1 / GS_POW(10, iPrecision));
	else
		return ceil(dValue * GS_POW(10.0, iPrecision) - 0.5) * (1 / GS_POW(10, iPrecision));
}


