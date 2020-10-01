
#include <QDomDocument>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>

#include <gqtl_log.h>
#include "dbc_filter_set.h"
#include "dbc_transaction.h"
#include "gate_event_manager.h"
#include "db_sqlite_man.h"

#include "dbc_parameter.h"
#include "ui_dbc_parameter.h"



///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcParameter::DbcParameter(DbcTransaction *pDbcTransac, ParamSqlTableModel *pTableModel, DbcParameterMode eMode/*=eCreate*/, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DbcParameter)
{
    m_ui->setupUi(this);
	m_pTableModel = pTableModel;
	m_iLastFilterSetIndex = 0;
	m_lstFilterSet.clear();
	m_pDbcTransac = pDbcTransac;
	m_iParamNumber = 0;
	m_strParamName = "parameter name";
	m_strUnit = "unit";
	m_strType = "";
	m_strValue = "";
	m_bIsAdvanced = false;
	m_lstParameterType.clear();
	m_lfHigL = 0.0;
	m_lfLowL = 0.0;
	m_eMode = eMode;
	
	resetView();
	init();

	m_ui->lineEditParamName->setFocus();
	if (m_eMode == DbcParameter::eCreate)
		setWindowTitle("Create parameter");
	else if (m_eMode == DbcParameter::eUpdate)
		setWindowTitle("Update parameter");
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcParameter::~DbcParameter()
{
	while (!m_lstFilterSet.isEmpty())
	     delete m_lstFilterSet.takeFirst();
    delete m_ui;
}

///////////////////////////////////////////////////////////
// Set advanced
///////////////////////////////////////////////////////////
void DbcParameter::setAdvanced(bool bIsAdvanced)
{
	m_bIsAdvanced = bIsAdvanced;
	m_ui->pushButtonAdvanced->setChecked(m_bIsAdvanced);
}

///////////////////////////////////////////////////////////
// Set param number
///////////////////////////////////////////////////////////
void DbcParameter::setParamNumber(int iParamNumber)
{
	m_iParamNumber = iParamNumber;
	m_ui->lineEditParamNumber->setText(QString::number(m_iParamNumber));
}

///////////////////////////////////////////////////////////
// Set low limit
///////////////////////////////////////////////////////////
void DbcParameter::setLowL(double lfLowL)
{
	m_lfLowL = lfLowL;
	m_ui->doubleSpinBoxLowL->setValue(m_lfLowL);
}

///////////////////////////////////////////////////////////
// Set high limit
///////////////////////////////////////////////////////////
void DbcParameter::setHighL(double lfHighL)
{
	m_lfHigL = lfHighL;
	m_ui->doubleSpinBoxHighL->setValue(m_lfHigL);
}

///////////////////////////////////////////////////////////
// Set param name
///////////////////////////////////////////////////////////
void DbcParameter::setParamName(QString strParamName)
{
	m_strParamName = strParamName;
	m_ui->lineEditParamName->setText(m_strParamName);
}

///////////////////////////////////////////////////////////
// Set unit
///////////////////////////////////////////////////////////
void DbcParameter::setUnit(QString strUnit)
{
	m_strUnit = strUnit;
	m_ui->lineEditParamUnit->setText(m_strUnit);
}

///////////////////////////////////////////////////////////
// Set type
///////////////////////////////////////////////////////////
void DbcParameter::setType(QString strType)
{
	m_strType = strType;
	m_ui->comboBoxParamType->setCurrentText(m_strType);
}

///////////////////////////////////////////////////////////
// Set param value
///////////////////////////////////////////////////////////
void DbcParameter::setValue(QString strValue)
{
	m_strValue = strValue;
	m_ui->lineEditValue->setText(m_strValue);
}


///////////////////////////////////////////////////////////
// Set step list
///////////////////////////////////////////////////////////
void DbcParameter::setStepLit(QString strStepList)
{
	if (!m_lstFilterSet.isEmpty())
	{
		m_lstFilterSet.first()->setStepList(strStepList);
	}
}


///////////////////////////////////////////////////////////
// Init ui
///////////////////////////////////////////////////////////
void DbcParameter::init()
{
	initComboParameterType();
	m_ui->groupBoxAdvanced->setVisible(false);
	connect(m_ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onSave()));
	connect(this, SIGNAL(sChanged()), this, SLOT(onChanged()));
	connect(m_ui->lineEditValue, SIGNAL(textChanged(QString)), this, SLOT(onChanged()));
	connect(m_ui->lineEditParamName, SIGNAL(textChanged(QString)), this, SLOT(onChanged()));
	connect(m_ui->lineEditParamNumber, SIGNAL(textChanged(QString)), this, SLOT(onChanged()));
	connect(m_ui->lineEditParamUnit, SIGNAL(textChanged(QString)), this, SLOT(onChanged()));
	connect(m_ui->comboBoxParamType, SIGNAL(currentIndexChanged(QString)), this, SLOT(onChanged()));
	connect(m_ui->doubleSpinBoxHighL, SIGNAL(valueChanged(double)), this, SLOT(onChanged()));
	connect(m_ui->doubleSpinBoxLowL, SIGNAL(valueChanged(double)), this, SLOT(onChanged()));
	connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(onApplyRequested()));
	connect(m_ui->pushButtonAdvanced, SIGNAL(toggled(bool)), this, SLOT(onCheckAdvanced(bool)));
	connect(m_ui->pushButtonOpenFile, SIGNAL(clicked()), this, SLOT(onLoadParameter()));
}

///////////////////////////////////////////////////////////
// Hide show advanced step list editor
///////////////////////////////////////////////////////////
void DbcParameter::onCheckAdvanced(bool bIsChecked)
{
	m_ui->groupBoxAdvanced->setVisible(bIsChecked);
	m_ui->lineEditValue->setDisabled(bIsChecked);
	if (!bIsChecked)
		m_ui->lineEditValue->setFocus();
	if (!m_lstFilterSet.isEmpty())
		m_lstFilterSet.first()->setFocus();
}

///////////////////////////////////////////////////////////
// Init combobox parameter type
///////////////////////////////////////////////////////////
void DbcParameter::initComboParameterType()
{
	m_lstParameterType << "P";
	m_lstParameterType << "S";
	
	m_ui->comboBoxParamType->clear();
	m_ui->comboBoxParamType->addItems(m_lstParameterType);

	/// TODO
	// Read xml file with type list
}

///////////////////////////////////////////////////////////
// Refresh ui
///////////////////////////////////////////////////////////
void DbcParameter::resetView()
{
	resetFilterSetView();
	m_ui->lineEditValue->setText(m_strValue);
	m_ui->lineEditParamName->setText(m_strParamName);
	m_ui->lineEditParamNumber->setText(QString::number(m_iParamNumber));
	m_ui->lineEditParamUnit->setText(m_strUnit);
	m_ui->comboBoxParamType->setCurrentText(m_strType);
	m_ui->pushButtonAdvanced->setChecked(m_bIsAdvanced);
	m_ui->doubleSpinBoxHighL->setValue(m_lfHigL);
	m_ui->doubleSpinBoxLowL->setValue(m_lfLowL);
}

///////////////////////////////////////////////////////////
// Refresh ui
///////////////////////////////////////////////////////////
void DbcParameter::resetFilterSetView()
{
	// If no filter filter -> add one
	if (m_lstFilterSet.isEmpty())
	{
		m_iLastFilterSetIndex = 0;
		// Create empty filter
		onAddFilterSetRequested();
	}
	// Else load existing and add one
	else
	{
		// Clear layout and let the spacer
		while (((QBoxLayout*)m_ui->frameFilterSet->layout())->count() > 0)
			((QBoxLayout*)m_ui->frameFilterSet->layout())->takeAt(0);
		
		DbcFilterSet* newFilterSet;
		foreach (newFilterSet, m_lstFilterSet)
		{
			((QBoxLayout*) m_ui->frameFilterSet->layout())->insertWidget(((QBoxLayout*) m_ui->frameFilterSet->layout())->count() ,newFilterSet);
		}
	}
}

///////////////////////////////////////////////////////////
// Insert filter in gui
///////////////////////////////////////////////////////////
void DbcParameter::addFilterSet(DbcFilterSet* newFilterSet)
{
	newFilterSet->setFrameParameterVisible(false);
	newFilterSet->setAddButtonVisible(true);
	// Hide add button of the last filter set
	if (!m_lstFilterSet.isEmpty())
		m_lstFilterSet.last()->setAddButtonVisible(false);
	m_lstFilterSet.append(newFilterSet);

	// Insert widget between the last DbcFilter widget and the spacer
	((QBoxLayout*) m_ui->frameFilterSet->layout())->insertWidget(((QBoxLayout*) m_ui->frameFilterSet->layout())->count() ,newFilterSet);
	if ((newFilterSet->minimumWidth() + 10) > m_ui->frameFilterSet->minimumWidth())
		m_ui->frameFilterSet->setMinimumWidth(newFilterSet->minimumWidth() + 10);
	m_ui->scrollArea->setMinimumHeight(newFilterSet->minimumHeight()*3 + 10);
	
	connect(newFilterSet, SIGNAL(sDelete(int)), this, SLOT(onDeleteFilterSet(int)));
	connect(newFilterSet, SIGNAL(sAdd()), this, SLOT(onAddFilterSetRequested()));
	connect(newFilterSet, SIGNAL(sChanged()), this, SIGNAL(sChanged()));
	if (newFilterSet->index() > m_iLastFilterSetIndex)
		m_iLastFilterSetIndex = newFilterSet->index();
}

///////////////////////////////////////////////////////////
// Add filter set 
///////////////////////////////////////////////////////////
void DbcParameter::onAddFilterSetRequested()
{
	m_iLastFilterSetIndex++;
	DbcFilterSet* newFilterSet = new DbcFilterSet(m_pDbcTransac, m_pTableModel, m_iLastFilterSetIndex, this);
	addFilterSet(newFilterSet);
}

///////////////////////////////////////////////////////////
// Delete filter set and last index
///////////////////////////////////////////////////////////
void DbcParameter::onDeleteFilterSet(int iIndex)
{
	bool bFilterFound = false;
	int iFilter = 0;
	
	// Ensure to keep at least one filter in the list
	if (m_lstFilterSet.size() > 1)
	{
		while (!bFilterFound && iFilter < m_lstFilterSet.size())
		{
			if (m_lstFilterSet.at(iFilter)->index() == iIndex)
			{
				bFilterFound = true;
				// If this is the last filter -> change last index
				if (iFilter == (m_lstFilterSet.size() - 1))
				{
					if (iFilter > 0)
					{
						m_iLastFilterSetIndex = m_lstFilterSet.at(iFilter - 1)->index();
						m_lstFilterSet.at(iFilter - 1)->setAddButtonVisible(true);
					}
					else
						m_iLastFilterSetIndex = 0;
				}
				delete m_lstFilterSet.takeAt(iFilter);
			}
			iFilter++;
		}
		
		emit sChanged();
	}
}

///////////////////////////////////////////////////////////
// Save data
///////////////////////////////////////////////////////////
void DbcParameter::onSave()
{
	QString strFilePath;
	strFilePath = QString(m_pDbcTransac->dbFolder() + m_strParamName +  + ".xml");
	if (!saveToFile(strFilePath))
	{
		GSLOG(SYSLOG_SEV_ERROR, QString("Unable to save %1.")
								.arg(strFilePath));
		m_ui->labelMessage->setText(QString("Unable to save %1...").arg(strFilePath));
	}
	else
		m_ui->labelMessage->setText(QString("%1 succesfuly saved!").arg(strFilePath));
}

///////////////////////////////////////////////////////////
// Retrieve all ui changes
///////////////////////////////////////////////////////////
void DbcParameter::onChanged()
{
	m_strValue = m_ui->lineEditValue->text();
	m_strParamName = m_ui->lineEditParamName->text();
	m_iParamNumber = m_ui->lineEditParamNumber->text().toInt();
	m_strUnit = m_ui->lineEditParamUnit->text();
	m_strType = m_ui->comboBoxParamType->currentText();
	m_bIsAdvanced = m_ui->pushButtonAdvanced->isChecked();
	m_lfHigL = m_ui->doubleSpinBoxHighL->value();
	m_lfLowL = m_ui->doubleSpinBoxLowL->value();
}

///////////////////////////////////////////////////////////
// Save new parameter QDomElement
///////////////////////////////////////////////////////////
QDomElement DbcParameter::data(QDomDocument& domDocument)
{
	// Root node
	QDomElement elmtRoot = domDocument.createElement("gex_dbc_new_parameter");
	elmtRoot.setAttribute("version", "1.0");
	// Parameter name
	QDomElement elmtParameterName = domDocument.createElement("parameter_name");
	elmtParameterName.appendChild(domDocument.createTextNode(m_strParamName));
	elmtRoot.appendChild(elmtParameterName);
	// Parameter number
	QDomElement elmtParameterNumber = domDocument.createElement("parameter_number");
	elmtParameterNumber.appendChild(domDocument.createTextNode(QString::number(m_iParamNumber)));
	elmtRoot.appendChild(elmtParameterNumber);
	// Parameter Unit
	QDomElement elmtParameterUnit = domDocument.createElement("parameter_unit");
	elmtParameterUnit.appendChild(domDocument.createTextNode(m_strUnit));
	elmtRoot.appendChild(elmtParameterUnit);
	// Parameter Type
	QDomElement elmtParameterType = domDocument.createElement("parameter_type");
	elmtParameterType.appendChild(domDocument.createTextNode(m_strType));
	elmtRoot.appendChild(elmtParameterType);
	// Parameter High limit
	QDomElement elmtParameterHighL = domDocument.createElement("high_l");
	elmtParameterHighL.appendChild(domDocument.createTextNode(QString::number(m_lfHigL)));
	elmtRoot.appendChild(elmtParameterHighL);
	// Parameter Low limit
	QDomElement elmtParameterLowL = domDocument.createElement("low_l");
	elmtParameterLowL.appendChild(domDocument.createTextNode(QString::number(m_lfLowL)));
	elmtRoot.appendChild(elmtParameterLowL);
	// Value
	QDomElement elmtParameterValue = domDocument.createElement("parameter_value");
	elmtParameterValue.appendChild(domDocument.createTextNode(m_strValue));
	elmtRoot.appendChild(elmtParameterValue);
	// Step List
	QDomElement elmtParameterIsAdvanced = domDocument.createElement("is_advanced");
	elmtParameterIsAdvanced.appendChild(domDocument.createTextNode(QString::number(m_bIsAdvanced)));
	elmtRoot.appendChild(elmtParameterIsAdvanced);
	// Filter set list
	QDomElement elmtFilterSetList = domDocument.createElement("filter_set_list");
	elmtRoot.appendChild(elmtFilterSetList);
	for (int i = 0; i < m_lstFilterSet.size(); i++)
	{
		QDomElement elmtFilterSet = m_lstFilterSet.at(i)->data(domDocument);
		elmtFilterSetList.appendChild(elmtFilterSet);
	}
	
	return elmtRoot;
}

///////////////////////////////////////////////////////////
// Load parameter from QDomElement
///////////////////////////////////////////////////////////
bool DbcParameter::loadData(QDomDocument &domDocument)
{
	// Clear filter list
	while (!m_lstFilterSet.isEmpty())
		delete m_lstFilterSet.takeFirst();
	
	m_iLastFilterSetIndex = 0;
	
	// Load root elmt
	QDomElement elmtRoot = domDocument.firstChildElement("gex_dbc_new_parameter");
	
	// Load param name
	QDomElement nodeParameterName = elmtRoot.firstChildElement("parameter_name");
	if (!nodeParameterName.isNull())
	{
		setParamName(nodeParameterName.text());
	}
	// Load param number
	QDomElement nodeParameterNumber = elmtRoot.firstChildElement("parameter_number");
	if (!nodeParameterNumber.isNull())
	{
		setParamNumber(nodeParameterNumber.text().toInt());
	}
	// Load param unit
	QDomElement nodeParameterUnit = elmtRoot.firstChildElement("parameter_unit");
	if (!nodeParameterUnit.isNull())
	{
		setUnit(nodeParameterUnit.text());
	}
	// Load param type
	QDomElement nodeParameterType = elmtRoot.firstChildElement("parameter_type");
	if (!nodeParameterType.isNull())
	{
		setType(nodeParameterType.text());
	}
	// Load param high limit
	QDomElement nodeParameterHighL = elmtRoot.firstChildElement("high_l");
	if (!nodeParameterHighL.isNull())
	{
		setHighL(nodeParameterHighL.text().toDouble());
	}
	// Load param low limit
	QDomElement nodeParameterLowL = elmtRoot.firstChildElement("low_l");
	if (!nodeParameterLowL.isNull())
	{
		setLowL(nodeParameterLowL.text().toDouble());
	}
	// Load param value
	QDomElement nodeParameterValue = elmtRoot.firstChildElement("parameter_value");
	if (!nodeParameterValue.isNull())
	{
		setValue(nodeParameterValue.text());
	}
	// Load param is advanced
	QDomElement nodeParameterAdvanced = elmtRoot.firstChildElement("is_advanced");
	if (!nodeParameterAdvanced.isNull())
	{
		setAdvanced(nodeParameterAdvanced.text().toInt());
	}
	
	// Load Filters
	QDomNode nodeFilterSetList = elmtRoot.firstChildElement("filter_set_list");
	if (!nodeFilterSetList.isNull())
	{
		QDomNode nodeFilterSet = nodeFilterSetList.firstChildElement("filter_set");
		while (!nodeFilterSet.isNull())
		{
			if (nodeFilterSet.isElement())
			{
				QDomElement elmtFilterSet = nodeFilterSet.toElement();
				if (!elmtFilterSet.isNull())
				{
					DbcFilterSet* newFilterSet = new DbcFilterSet(m_pDbcTransac, m_pTableModel, elmtFilterSet.attribute("id").toInt(), this);
					newFilterSet->loadData(elmtFilterSet);
					addFilterSet(newFilterSet);
				}
			}
			nodeFilterSet = nodeFilterSet.nextSibling();
		}
	}
	
	resetView();
	
	return true;
}


///////////////////////////////////////////////////////////
// Save new parameter to xml file
///////////////////////////////////////////////////////////
bool DbcParameter::saveToFile(const QString& strFilePath)
{
	if (!strFilePath.isEmpty())
	{
		// Create dom document
		QDomDocument domDocument;
		domDocument.appendChild(this->data(domDocument));
		
		QFile file(strFilePath);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream textStream(&file);
			domDocument.save(textStream, 0);
			file.close();
			return true;
		}
	}
	
	return false;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcParameter::loadXml(const QString& strFilePath)
{
	if (!strFilePath.isEmpty())
	{
		QFile file(strFilePath);
		if (file.open(QIODevice::ReadOnly))
		{
			QString strErrorMsg;
			int iErrorLine, iErrorColumn;
			QDomDocument domDocument;
			if (domDocument.setContent(&file, &strErrorMsg, &iErrorLine, &iErrorColumn)) 
			{
				return loadData(domDocument);
			}
			else
				GSLOG(SYSLOG_SEV_ERROR, QString("%1 at line %2, column %SYSLOG_SEV_ERROR.")
				       .arg(strErrorMsg)
				       .arg(QString::number(iErrorLine))
				       .arg(QString::number(iErrorColumn)));

			file.close();
		}
	}
	
	return false;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcParameter::onLoadParameter()
{
	// Display Open file dialog so the user can select a parameter file
	QString strFilePath = QFileDialog::getOpenFileName(this, "Select a parameter file", m_pDbcTransac->dbFolder());
	// Check if new file selected
	if((strFilePath.isEmpty() == true) || (m_pDbcTransac->dbFolder() == strFilePath))
		return false;

	return loadXml(strFilePath);
}

///////////////////////////////////////////////////////////
// IfCreation mode try to insert new parameter else try to 
// update existing one
///////////////////////////////////////////////////////////
void DbcParameter::onApplyRequested()
{
	// If open to create new parameter
	if (m_eMode == DbcParameter::eCreate)
	{
		m_pDbcTransac->beginTransaction();
		if (insertParameter())
			m_pDbcTransac->commitTransaction();
		else
			m_pDbcTransac->rollbackTransaction();
	}
	// If open to update existing parameter
	else if (m_eMode == DbcParameter::eUpdate)
	{
		m_pDbcTransac->beginTransaction();
		if (updateParameter())
			m_pDbcTransac->commitTransaction();
		else
			m_pDbcTransac->rollbackTransaction();
	}
}

///////////////////////////////////////////////////////////
// Try to update parameter
///////////////////////////////////////////////////////////
bool DbcParameter::updateParameter()
{
	// UPDATE RECORD INTO TEST TABLE
	Gate_ParameterDef newParameter;
	newParameter.m_nParameterNumber = m_iParamNumber;
	newParameter.m_strName = m_strParamName;
	newParameter.m_bTestType = (BYTE) m_strType[0].toLatin1();
	if (!m_pDbcTransac->updateTest(newParameter))
	{
		m_ui->labelMessage->setText("Error while updating parameter, please verify the parameter number and name...");
		return false;
	}

	// UPDATE RECORD INTO TEST INFO TABLE
	newParameter.m_lfHighL = m_lfHigL;
	newParameter.m_lfLowL = m_lfLowL;
	newParameter.m_strUnits = m_strUnit;
	if (!m_pDbcTransac->updateTestInfo(newParameter))
	{
		m_ui->labelMessage->setText("Error while updating parameter, please verify limits and unit...");
		return false;
	}

	
	bool bIsUpdateOK;
	QString strTestInfoId = m_pDbcTransac->testInfoId(newParameter);
	// If Parametric parameter
	if (QChar(newParameter.m_bTestType) == 'P')
		bIsUpdateOK= buildUpdateParameterPTestResultsQuery(strTestInfoId);
	// If String parameter
	else if (QChar(newParameter.m_bTestType) == 'S')
		bIsUpdateOK= buildUpdateParameterSTestResultsQuery(strTestInfoId);
	else
	{
		m_ui->labelMessage->setText(QString("Error while updating parameter, Unknown type 1%...").arg(QChar(newParameter.m_bTestType)));
		return false;
	}
	
	if (bIsUpdateOK)
	{
		m_ui->labelMessage->setText(QString("%1 updated with success!").arg(m_strParamName));
		return true;
	}
	else
	{
		m_ui->labelMessage->setText(QString("Error while updating %1!").arg(m_strParamName));
		return false;
	}
}

///////////////////////////////////////////////////////////
// Insert the new parameter in the database
///////////////////////////////////////////////////////////
bool DbcParameter::insertParameter()
{
	// CREATE RECORD INTO TEST TABLE
	Gate_ParameterDef newParameter;
	newParameter.m_nParameterNumber = m_iParamNumber;
	newParameter.m_strName = m_strParamName;
	newParameter.m_bTestType = (BYTE) m_strType[0].toLatin1();
	QString strTestId = m_pDbcTransac->insertTest(newParameter);
	if (strTestId.isEmpty())
	{
		m_ui->labelMessage->setText("Error while inserting new parameter, please verify the parameter number and name...");
		return false;
	}

	// CREATE RECORD INTO TEST INFO TABLE
	newParameter.m_lfHighL = m_lfHigL;
	newParameter.m_lfLowL = m_lfLowL;
	newParameter.m_strUnits = m_strUnit;
	newParameter.m_nId = strTestId.toInt();
	QString strTestInfoId = m_pDbcTransac->insertTestInfo(newParameter);
	if (strTestInfoId.isEmpty())
	{
		m_ui->labelMessage->setText("Error while inserting new parameter, please verify limits and unit...");
		return false;
	}

	
	bool bIsInsertOK;
	
	// If Parametric parameter
	if (QChar(newParameter.m_bTestType) == 'P')
		bIsInsertOK= buildInsertParameterPTestResultsQuery(strTestInfoId);
	// If String parameter
	else if (QChar(newParameter.m_bTestType) == 'S')
		bIsInsertOK= buildInsertParameterSTestResultsQuery(strTestInfoId);
	else
	{
		m_ui->labelMessage->setText(QString("Error while inserting new parameter, Unknown type 1%...").arg(QChar(newParameter.m_bTestType)));
		return false;
	}
	
	if (bIsInsertOK)
	{
		m_ui->labelMessage->setText(QString("%1 inserted with success!").arg(m_strParamName));
		return true;
	}
	else
	{
		m_ui->labelMessage->setText(QString("Error while inserting %1!").arg(m_strParamName));
		return false;
	}
}

///////////////////////////////////////////////////////////
// Build SQL query to insert the new Ptest parameter
///////////////////////////////////////////////////////////
bool DbcParameter::buildInsertParameterPTestResultsQuery(const QString& strTestInfoId)
{
	QString strFilterSet = "";
	QString strErrorMsg = "";
	// CREATE RECORDS INTO TEST RESULT TABLE 
	QString strQuery = "";
	
	if (!m_bIsAdvanced)
	{
		strQuery = "INSERT INTO "
		        + QString(DBC_PTEST_RESULT_T) + " (" + QString(DBC_PTEST_RESULT_F_STEPID) + "," + QString(DBC_PTEST_RESULT_F_TESTINFOID)
		        + "," + QString(DBC_PTEST_RESULT_F_FLOWID) + "," + QString(DBC_PTEST_RESULT_F_FAILED) + "," + QString(DBC_PTEST_RESULT_F_VALUE) + ") ";
		// Build Select query with or without formula
		bool bIsConversionOk = false;
		m_strValue.toFloat(&bIsConversionOk);
		// If the field is completly numeric no need to parse formula
		if (bIsConversionOk)
			// Build step list without formula
			strQuery += selectQueryWithoutFormula(strTestInfoId, m_strValue);
		else
			// Build step list with formula
			strQuery += selectQueryWithFormula(strTestInfoId, m_strValue);
		
		// Exec INSERT Query 
		m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
		
		GSLOG(SYSLOG_SEV_DEBUG, strQuery);
		
		// If error in query in query ROLLBACK
		if (!strErrorMsg.isEmpty())
		{
			m_pDbcTransac->rollbackTransaction();
			m_ui->labelMessage->setText("Error while inserting new parameter, please verify the formula...");
			return false;
		}
	}
	else
	{
		DbcFilterSet* pFilterSet = NULL;
		strErrorMsg = "";
		foreach (pFilterSet, m_lstFilterSet)
		{
			strQuery = "INSERT INTO "
			        + QString(DBC_PTEST_RESULT_T) + " (" + QString(DBC_PTEST_RESULT_F_STEPID) + "," + QString(DBC_PTEST_RESULT_F_TESTINFOID)
			        + "," + QString(DBC_PTEST_RESULT_F_FLOWID) + "," + QString(DBC_PTEST_RESULT_F_FAILED) + "," + QString(DBC_PTEST_RESULT_F_VALUE) + ") ";
			if (!pFilterSet->isAdvanced())
				strFilterSet = pFilterSet->buildSimpleStepListQuery();
			else
				strFilterSet = pFilterSet->buildFilterLogicQuery();
			// If the field is completly numeric no need to parse formula
			if (!pFilterSet->isValueBasedOnFormula())
				// Build step list without formula
				strQuery += selectQueryWithoutFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
			else
				// Build step list with formula
				strQuery += selectQueryWithFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
			
			// Exec INSERT Query 
			m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
			
			GSLOG(SYSLOG_SEV_DEBUG, strQuery);
			
			// If error in query in query ROLLBACK
			if (!strErrorMsg.isEmpty())
			{
				m_pDbcTransac->rollbackTransaction();
				m_ui->labelMessage->setText("Error while inserting new parameter, please verify the formula...");
				return false;
			}
		}
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// Build SQL query to update the new Ptest parameter
///////////////////////////////////////////////////////////
bool DbcParameter::buildUpdateParameterPTestResultsQuery(const QString& strTestInfoId)
{
	QString strFilterSet = "";
	QString strStepList = "";
	QString strErrorMsg = "";
	QString strQuery = "";
	DbcFilterSet* pFilterSet = NULL;
	
	foreach (pFilterSet, m_lstFilterSet)
	{
		strErrorMsg = "";
		// Build filter set query
		if (!pFilterSet->isAdvanced())
			strFilterSet = pFilterSet->buildSimpleStepListQuery();
		else
			strFilterSet = pFilterSet->buildFilterLogicQuery();
		
		// If the field is completly numeric no need to parse formula
		if (!pFilterSet->isValueBasedOnFormula())
			// Build step list without formula
			strStepList = selectQueryWithoutFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
		else
			// Build step list with formula
			strStepList = selectQueryWithFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
		
		// DELETE PREVIOUS VALUE
		strQuery = "DELETE FROM " + QString(DBC_PTEST_RESULT_T) + " "
					"WHERE "
					+ QString(DBC_PTEST_RESULT_F_STEPID) + " IN "
					"( SELECT " + QString(DBC_STEP_F_ID) + " FROM (" + strStepList + ")) "
					"AND " + QString(DBC_PTEST_RESULT_F_TESTINFOID) + "=\'" + strTestInfoId + "\'";

		// Exec INSERT Query 
		m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
		
		GSLOG(SYSLOG_SEV_DEBUG, strQuery);
		
		// If error in query in query ROLLBACK
		if (!strErrorMsg.isEmpty())
		{
			m_pDbcTransac->rollbackTransaction();
			m_ui->labelMessage->setText("Error while updating new parameter, please verify the formula...");
			return false;
		}
		
		// INSERT NEW VALUE
		strQuery = "INSERT INTO "
		        + QString(DBC_PTEST_RESULT_T) + " (" + QString(DBC_PTEST_RESULT_F_STEPID) + "," + QString(DBC_PTEST_RESULT_F_TESTINFOID)
		        + "," + QString(DBC_PTEST_RESULT_F_FLOWID) + "," + QString(DBC_PTEST_RESULT_F_FAILED) + "," + QString(DBC_PTEST_RESULT_F_VALUE) + ") ";
		
		strQuery += strStepList;
		
		// Exec INSERT Query 
		m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
		
		GSLOG(SYSLOG_SEV_DEBUG, strQuery);
		
		// If error in query in query ROLLBACK
		if (!strErrorMsg.isEmpty())
		{
			m_pDbcTransac->rollbackTransaction();
			m_ui->labelMessage->setText("Error while updating new parameter, please verify the formula...");
			return false;
		}
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// Build SQL query to insert the new STest parameter
///////////////////////////////////////////////////////////
bool DbcParameter::buildInsertParameterSTestResultsQuery(const QString& strTestInfoId)
{
	QString strQuery = "";
	QString strFilterSet = "";
	QString strErrorMsg = "";
	
	if (!m_bIsAdvanced)
	{
		// CREATE RECORDS INTO TEST RESULT TABLE 
		strQuery = "INSERT INTO "
		        + QString(DBC_STEST_RESULT_T) + " (" + QString(DBC_STEST_RESULT_F_STEPID) + "," + QString(DBC_STEST_RESULT_F_TESTINFOID)
		        + "," + QString(DBC_STEST_RESULT_F_VALUE) + "," + QString(DBC_STEST_RESULT_F_FLOWID) + "," + QString(DBC_STEST_RESULT_F_FAILED) + ") ";
		
		// Check if there is a Parameter number in value (formula)
		//QRegExp regExpTestNumber("(T|t)\\d+(.\\d+)?"); // eg. T1050 or T1050.1
		//if (regExpTestNumber.indexIn(m_strValue) != -1)
		{
			// Create query based on a formula
			// Build step list without formula
			strQuery += selectQueryWithoutFormula(strTestInfoId, m_strValue);
		}
		//else
		//{
			// Create query based on all step
		//	strQuery += selectQueryWithFormula(strTestInfoId, m_strValue);
		//}
		
		// Exec INSERT Query 
		m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
		
		GSLOG(SYSLOG_SEV_DEBUG, strQuery);
		
		// If error in query in query ROLLBACK
		if (!strErrorMsg.isEmpty())
		{
			m_pDbcTransac->rollbackTransaction();
			m_ui->labelMessage->setText("Error while inserting new parameter, please verify the formula...");
			return false;
		}
	}
	else
	{
		DbcFilterSet* pFilterSet = NULL;
		foreach (pFilterSet, m_lstFilterSet)
		{
			strErrorMsg = "";
			// CREATE RECORDS INTO TEST RESULT TABLE 
			strQuery = "INSERT INTO "
			        + QString(DBC_STEST_RESULT_T) + " (" + QString(DBC_STEST_RESULT_F_STEPID) + "," + QString(DBC_STEST_RESULT_F_TESTINFOID)
			        + "," + QString(DBC_STEST_RESULT_F_FLOWID) + "," + QString(DBC_STEST_RESULT_F_FAILED) + "," + QString(DBC_STEST_RESULT_F_VALUE) + ") ";
			
			if (!pFilterSet->isAdvanced())
				strFilterSet = pFilterSet->buildSimpleStepListQuery();
			else
				strFilterSet = pFilterSet->buildFilterLogicQuery();
			
			// If the field is completly numeric no need to parse formula
			//if (!pFilterSet->isValueBasedOnFormula())
				// Build step list without formula
				strQuery += selectQueryWithoutFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
			//else
				// Build step list with formula
			//	strQuery += selectQueryWithFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
			
			// Exec INSERT Query 
			m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
			
			GSLOG(SYSLOG_SEV_DEBUG, strQuery);
			
			// If error in query in query ROLLBACK
			if (!strErrorMsg.isEmpty())
			{
				m_pDbcTransac->rollbackTransaction();
				m_ui->labelMessage->setText("Error while inserting new parameter, please verify the formula...");
				return false;
			}
		}
	}
	
	return true;
}


///////////////////////////////////////////////////////////
// Build SQL query to update the new STest parameter
///////////////////////////////////////////////////////////
bool DbcParameter::buildUpdateParameterSTestResultsQuery(const QString& strTestInfoId)
{
	QString strFilterSet = "";
	QString strStepList = "";
	QString strErrorMsg = "";
	QString strQuery = "";
	DbcFilterSet* pFilterSet = NULL;
	
	foreach (pFilterSet, m_lstFilterSet)
	{
		strErrorMsg = "";
		if (!pFilterSet->isAdvanced())
			strFilterSet = pFilterSet->buildSimpleStepListQuery();
		else
			strFilterSet = pFilterSet->buildFilterLogicQuery();
		
		// If the field is completly numeric no need to parse formula
		//if (!pFilterSet->isValueBasedOnFormula())
			// Build step list without formula
			strStepList = selectQueryWithoutFormula(strTestInfoId, pFilterSet->value(), strFilterSet);
		//else
			// Build step list with formula
		//	strStepList = selectQueryWithFormula(strTestInfoId, pFilterSet->value(), strFilterSet);

		// DELETE PREVIOUS VALUE
		strQuery = "DELETE FROM " + QString(DBC_STEST_RESULT_T) + " "
					"WHERE "
					+ QString(DBC_STEST_RESULT_F_STEPID) + " IN "
					"( SELECT " + QString(DBC_STEP_F_ID) + " FROM (" + strStepList + ")) "
					"AND " + QString(DBC_STEST_RESULT_F_TESTINFOID) + "=\'" + strTestInfoId + "\'";
		
		
		// Exec INSERT Query 
		m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
		
		GSLOG(SYSLOG_SEV_DEBUG, strQuery);
		
		// If error in query in query ROLLBACK
		if (!strErrorMsg.isEmpty())
		{
			m_pDbcTransac->rollbackTransaction();
			m_ui->labelMessage->setText("Error while updating new parameter, please verify the formula...");
			return false;
		}
		
		// CREATE RECORDS INTO TEST RESULT TABLE 
		strQuery = "INSERT INTO "
		        + QString(DBC_STEST_RESULT_T) + " (" + QString(DBC_STEST_RESULT_F_STEPID) + "," + QString(DBC_STEST_RESULT_F_TESTINFOID)
		        + "," + QString(DBC_STEST_RESULT_F_FLOWID) + "," + QString(DBC_STEST_RESULT_F_FAILED) + "," + QString(DBC_STEST_RESULT_F_VALUE) + ") ";
		
		strQuery += strStepList;
		
		// Exec INSERT Query 
		m_pDbcTransac->dbConnection()->execQuery(strQuery, &strErrorMsg);
		
		GSLOG(SYSLOG_SEV_DEBUG, strQuery);
		
		// If error in query in query ROLLBACK
		if (!strErrorMsg.isEmpty())
		{
			m_pDbcTransac->rollbackTransaction();
			m_ui->labelMessage->setText("Error while updating new parameter, please verify the formula...");
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
// Build a select query for all steps
///////////////////////////////////////////////////////////
QString DbcParameter::selectQueryWithoutFormula(const QString& strTestInfoId, const QString& strValue, const QString& strFilterSetQuery)
{
	// Define test value
	QString strTestValue = "\'" + strValue + "\' AS value ";
	QString strQuery;
	strQuery = "SELECT " + QString(DBC_STEP_F_ID) + ",\'" + strTestInfoId + "\'" + ", \'-1\', \'0\', " + strTestValue + " "
	        "FROM "
	        + QString(DBC_STEP_T) + " WHERE " + QString(DBC_STEP_F_SESSIONID) + "=\'" + m_pDbcTransac->sessionId() + "\' " ;
	
	
	if (!strFilterSetQuery.isEmpty())
	{
		strQuery += "AND " + QString(DBC_STEP_F_ID) + " IN ("+ strFilterSetQuery + ") ";
	}
	
	return strQuery;
}

///////////////////////////////////////////////////////////
// Build a select query for steps corresponding to the formula
///////////////////////////////////////////////////////////
QString DbcParameter::selectQueryWithFormula(const QString& strTestInfoId, const QString& strValue, const QString& strFilterSetQuery)
{
	QString strFormula = strValue;
	
	// Define test list for tests included in the formula
	QString strFromStatement = "";
	
	QRegExp regExpTestNumber("(T|t)\\d+(.\\d+)?"); // eg. T1050 or T1050.1
	Gate_ParameterDef newParameter;
	QString strTmpFormula = "";
	int iPos = 0;
	bool ok;
	bool bMultiParam = false;
	int iParameterId = 1;

	// While find matching expression
	while ((iPos = regExpTestNumber.indexIn(strFormula)) != -1)
	{
		// To avoid error with multiple use of one parameter
		QString strParamIdName = "Parameter" + QString::number(iParameterId);
		// build new formula with new param name
		strTmpFormula += strFormula.left(iPos) + strParamIdName;
		strFormula.remove(0, iPos);
		strFormula.remove(0, regExpTestNumber.cap().size());
		
		newParameter.ResetData();
		// Get test number
		newParameter.m_nParameterNumber = regExpTestNumber.cap().mid(1).toUInt(&ok);
		if (!ok)
			return QString("Error: unable to convert to int %1").arg(regExpTestNumber.cap());

		// Search test
		QString strTestId = m_pDbcTransac->testId(newParameter);
		if (strTestId.isEmpty())
			return QString("Error: unable to find %1").arg(regExpTestNumber.cap());
		// Get parameter info
		Gate_ParameterDef* parameterInfo = m_pDbcTransac->testInfo(strTestId.toInt());

		// If it's not the first parameter add inner join
		if (!strFromStatement.isEmpty())
		{
			strFromStatement += "INNER JOIN ";
			bMultiParam = true;
		}
		// build the table with param N results
		strFromStatement += "(";
		// If parametric parameter
		if (QChar(parameterInfo->m_bTestType) == 'P')
		{
			strFromStatement += "SELECT " + QString(DBC_PTEST_RESULT_F_STEPID) + ", value as " + strParamIdName + " "
					"FROM " + QString(DBC_TEST_T) + " "
					"INNER JOIN " + QString(DBC_PTEST_INFO_T) + " "
					"ON " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_PTEST_INFO_T) + "." + QString(DBC_PTEST_INFO_F_TESTID) + " "
					"INNER JOIN " + QString(DBC_PTEST_RESULT_T) + " "
					"ON " + QString(DBC_PTEST_INFO_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_PTEST_RESULT_T) + "." + QString(DBC_PTEST_RESULT_F_TESTINFOID) + " "
					"WHERE " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_NUMBER) + "=\'" + regExpTestNumber.cap().mid(1) + "\' ";
			if (!strFilterSetQuery.isEmpty())
				strFromStatement += "AND " + QString(DBC_PTEST_RESULT_F_STEPID) + " IN ("+ strFilterSetQuery + ") ";
		}
		// If String parameter if (QChar(parameterInfo->m_bTestType) == 'S')
		else 
		{
			strFromStatement += "SELECT " + QString(DBC_STEST_RESULT_F_STEPID) + ", value as " + strParamIdName + " "
					"FROM " + QString(DBC_TEST_T) + " "
					"INNER JOIN " + QString(DBC_STEST_INFO_T) + " "
					"ON " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_STEST_INFO_T) + "." + QString(DBC_STEST_INFO_F_TESTID) + " "
					"INNER JOIN " + QString(DBC_STEST_RESULT_T) + " "
					"ON " + QString(DBC_STEST_INFO_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_STEST_RESULT_T) + "." + QString(DBC_STEST_RESULT_F_TESTINFOID) + " "
					"WHERE " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_NUMBER) + "=\'" + regExpTestNumber.cap().mid(1) + "\' ";
			if (!strFilterSetQuery.isEmpty())
				strFromStatement += "AND " + QString(DBC_STEST_RESULT_F_STEPID) + " IN ("+ strFilterSetQuery + ") ";
		}

		strFromStatement += ") ";
		
		// If it's not the first parameter set the USING value
		if (bMultiParam)
			strFromStatement += "USING (step_id) ";
		
		iParameterId++;
	}
	strFormula = strTmpFormula + strFormula;
	
	// Define test value
	QString strTestValueFormula = " (" + strFormula + ") AS value ";
	// Build Query
	QString strQuery;
	strQuery = "SELECT " + QString(DBC_PTEST_RESULT_F_STEPID) + ",\'" + strTestInfoId + "\'" + ", \'-1\', \'0\', " + strTestValueFormula + " "
	        "FROM "
	        + strFromStatement + " ";
	
	return strQuery;
}

