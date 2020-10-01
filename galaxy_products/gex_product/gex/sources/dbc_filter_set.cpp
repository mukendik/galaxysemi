#include <QSortFilterProxyModel>

#include <gqtl_log.h>
#include "dbc_filter.h"
#include "dbc_dataset_editor.h"
#include "dbc_transaction.h"
#include "gate_event_manager.h"


#include "dbc_filter_set.h"
#include "ui_dbc_filter_set.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcFilterSet::DbcFilterSet(DbcTransaction *pDbcTransac, ParamSqlTableModel *pTableModel, int iIndex, QWidget *parent) :
    QFrame(parent),
    m_ui(new Ui::DbcFilterSet)
{
    m_ui->setupUi(this);
	if (!pTableModel)
		GSLOG(SYSLOG_SEV_ERROR, "no table model.");
	
	m_pDbcTransac = pDbcTransac;
	m_pTableModelParam = pTableModel;
	m_iIndex = iIndex;
	m_iLastFilterIndex = 0;
	m_iParamNumber = -1;
	m_strParamName = "";
	m_strTitle = "";
	m_strFilterLogic = "";
	m_strValue = "";
	m_strStepList = "*";
	m_pProxyModelParamNumber = NULL;
	m_bIsAdvanced = false;
	m_lstDbcFilters.clear();
	m_ui->lineEditValue->setFocus();
	resetView();
	init();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcFilterSet::~DbcFilterSet()
{
	while (!m_lstDbcFilters.isEmpty())
	     delete m_lstDbcFilters.takeFirst();
    delete m_ui;
}

///////////////////////////////////////////////////////////
// Set filter logic
///////////////////////////////////////////////////////////
void DbcFilterSet::setFilterLogic(QString strFilterLogic)
{
	m_strFilterLogic = strFilterLogic;
	m_ui->lineEditFilterLogic->setText(m_strFilterLogic);
}

///////////////////////////////////////////////////////////
// Set associated value
///////////////////////////////////////////////////////////
void DbcFilterSet::setValue(QString strValue)
{
	m_strValue = strValue;
	m_ui->lineEditValue->setText(m_strValue);
}

///////////////////////////////////////////////////////////
// Set Parameter name
///////////////////////////////////////////////////////////
void DbcFilterSet::setParamName(QString strParamName)
{
	m_strParamName = strParamName;
	resetComboParamNumber();
}

///////////////////////////////////////////////////////////
// Set parameter number
///////////////////////////////////////////////////////////
void DbcFilterSet::setParamNumber(int iParamNumber)
{
	m_iParamNumber = iParamNumber;
	resetComboParamNumber();
}

///////////////////////////////////////////////////////////
// Set index
///////////////////////////////////////////////////////////
void DbcFilterSet::setIndex(int iIndex)
{
	m_iIndex = iIndex;
}

///////////////////////////////////////////////////////////
// Set step List
///////////////////////////////////////////////////////////
void DbcFilterSet::setStepList(QString strStepList)
{
	m_strStepList = strStepList;
	m_ui->lineEditStepList->setText(m_strStepList);
}

///////////////////////////////////////////////////////////
// Set if is advanced filter
///////////////////////////////////////////////////////////
void DbcFilterSet::setAdvanced(bool bIsAdvanced)
{
	m_bIsAdvanced = bIsAdvanced;
	m_ui->pushButtonAdvanced->setChecked(m_bIsAdvanced);
}

///////////////////////////////////////////////////////////
// Set Add button visible/hidden
///////////////////////////////////////////////////////////
void DbcFilterSet::setAddButtonVisible(bool bIsVisible)
{
	m_ui->pushButtonAdd->setVisible(bIsVisible);
}

///////////////////////////////////////////////////////////
// enable/disable combo box associated value
///////////////////////////////////////////////////////////
void DbcFilterSet::setValueVisible(bool bIsVisible)
{
	m_ui->frameValue->setVisible(bIsVisible);
}

///////////////////////////////////////////////////////////
// enable/disable combo box parameter
///////////////////////////////////////////////////////////
void DbcFilterSet::setFrameParameterVisible(bool bIsVisible)
{
	m_ui->frameParameter->setVisible(bIsVisible);
}

///////////////////////////////////////////////////////////
// Refresh view
///////////////////////////////////////////////////////////
void DbcFilterSet::resetView()
{
	resetComboParamNumber();
	resetFilterListView();
	m_ui->lineEditValue->setText(m_strValue);
	m_ui->lineEditFilterLogic->setText(m_strFilterLogic);
	m_ui->lineEditStepList->setText(m_strStepList);
	m_ui->pushButtonAdvanced->setChecked(m_bIsAdvanced);
	onCheckBoxAdvanced(m_bIsAdvanced);
}

///////////////////////////////////////////////////////////
// Init ui 
///////////////////////////////////////////////////////////
void DbcFilterSet::init()
{
	initComboParamNumber();
	connect(m_ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(onAutoDelete()));
	connect(m_ui->pushButtonDelete, SIGNAL(clicked()), this, SIGNAL(sChanged()));
	connect(m_ui->pushButtonAdd, SIGNAL(clicked()), this, SIGNAL(sAdd()));
	connect(m_ui->pushButtonAdd, SIGNAL(clicked()), this, SIGNAL(sChanged()));
	connect(m_ui->lineEditFilterLogic, SIGNAL(textEdited(QString)), this, SIGNAL(sChanged()));
	connect(m_ui->lineEditValue, SIGNAL(textEdited(QString)), this, SIGNAL(sChanged()));
	connect(m_ui->lineEditStepList, SIGNAL(textEdited(QString)), this, SIGNAL(sChanged()));
	connect(m_ui->comboBoxParamNameNumber, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sChanged()));
	connect(m_ui->pushButtonAdvanced, SIGNAL(toggled(bool)), this,SLOT(onCheckBoxAdvanced(bool)));
	connect(this, SIGNAL(sChanged()), this, SLOT(onChanged()));
}

///////////////////////////////////////////////////////////
// Refresh gui is user choose advanced / normal
///////////////////////////////////////////////////////////
void DbcFilterSet::onCheckBoxAdvanced(bool bIsToggled)
{
	m_ui->frameAdvancedStepList->setVisible(bIsToggled);
	m_ui->frameStepList->setHidden(bIsToggled);
	m_ui->stackedWidget->setCurrentIndex(bIsToggled);
	m_ui->stackedWidget->adjustSize();
}

///////////////////////////////////////////////////////////
// init combobox parameter
///////////////////////////////////////////////////////////
void DbcFilterSet::initComboParamNumber()
{
	m_pProxyModelParamNumber = new QSortFilterProxyModel(this);
	m_pProxyModelParamNumber->setSourceModel(m_pTableModelParam);
	m_ui->comboBoxParamNameNumber->setModel(m_pProxyModelParamNumber);
	m_ui->comboBoxParamNameNumber->setModelColumn(m_pTableModelParam->fieldIndex(QString(DBC_TEST_F_NAME)));
}

///////////////////////////////////////////////////////////
// Refresh combobox parameter
///////////////////////////////////////////////////////////
void DbcFilterSet::resetComboParamNumber()
{
	if (m_ui->comboBoxParamNameNumber->findText(QString(m_strParamName + "-" + QString::number(m_iParamNumber))) >= 0)
		m_ui->comboBoxParamNameNumber->setCurrentIndex(m_ui->comboBoxParamNameNumber->findText(QString(m_strParamName + "-" + QString::number(m_iParamNumber))));
	else
		m_ui->comboBoxParamNameNumber->setCurrentIndex(0);
}

///////////////////////////////////////////////////////////
// Refresh filter list view
///////////////////////////////////////////////////////////
void DbcFilterSet::resetFilterListView()
{
	// If no filter filter -> add one
	if (m_lstDbcFilters.isEmpty())
	{
		m_iLastFilterIndex = 0;
		// Create empty filter
		onAddFilter();
	}
	// Else load existing and add one
	else
	{
		// Clear layout and let the spacer
		while (((QBoxLayout*)m_ui->frameFilter->layout())->count() > 1)
			((QBoxLayout*)m_ui->frameFilter->layout())->takeAt(0);
		
		DbcFilter* newFilter;
		foreach (newFilter, m_lstDbcFilters)
		{
			((QBoxLayout*) m_ui->frameFilter->layout())->insertWidget(((QBoxLayout*) m_ui->frameFilter->layout())->count() - 1,newFilter);
		}
	}
}

///////////////////////////////////////////////////////////
// Called when change detected on ui
///////////////////////////////////////////////////////////
void DbcFilterSet::onChanged()
{
	retrieveUIValue();
}

///////////////////////////////////////////////////////////
// Set member with ui value
///////////////////////////////////////////////////////////
void DbcFilterSet::retrieveUIValue()
{
	m_strFilterLogic = m_ui->lineEditFilterLogic->text();
	m_strValue = m_ui->lineEditValue->text();
	m_strStepList = m_ui->lineEditStepList->text();
	m_iParamNumber = m_pTableModelParam->index(m_ui->comboBoxParamNameNumber->currentIndex(), m_pTableModelParam->fieldIndex(QString(DBC_TEST_F_NUMBER))).data(Qt::DisplayRole).toInt();
	m_strParamName = m_pTableModelParam->index(m_ui->comboBoxParamNameNumber->currentIndex(), m_pTableModelParam->fieldIndex(QString(DBC_TEST_F_NAME))).data(Qt::DisplayRole).toString();
	m_strParamName.remove(- (QString::number(m_iParamNumber).size() + 1),QString::number(m_iParamNumber).size() + 1);
	m_bIsAdvanced = m_ui->pushButtonAdvanced->isChecked();
}

///////////////////////////////////////////////////////////
// Remove filter from ui & list, update QList
///////////////////////////////////////////////////////////
void DbcFilterSet::onDeleteFilter(int iIndex)
{
	bool bFilterFound = false;
	int iFilter = 0;

	// Ensure to keep at least one filter in the list
	if (m_lstDbcFilters.size() > 1)
	{
		while (!bFilterFound && iFilter < m_lstDbcFilters.size())
		{
			// Match with
			if (m_lstDbcFilters.at(iFilter)->index() == iIndex)
			{
				bFilterFound = true;
				// If this is the last filter -> change last index
				if (iFilter == (m_lstDbcFilters.size() - 1))
				{
					if (iFilter > 0)
					{
						m_iLastFilterIndex = m_lstDbcFilters.at(iFilter - 1)->index();
						m_lstDbcFilters.at(iFilter - 1)->setAddButtonVisible(true);
					}
					else
						m_iLastFilterIndex = 0;
				}
				delete m_lstDbcFilters.takeAt(iFilter);
			}
			iFilter++;
		}
		
		emit sChanged();
	}
}

///////////////////////////////////////////////////////////
// Emit signal for parent 
///////////////////////////////////////////////////////////
void DbcFilterSet::onAutoDelete()
{
	emit sDelete(m_iIndex);
}

///////////////////////////////////////////////////////////
// Insert new filter in gui and connect signal/slot
///////////////////////////////////////////////////////////
void DbcFilterSet::addFilter(DbcFilter* newFilter)
{
	newFilter->setAddButtonVisible(true);
	// Hide add button of the last filter set
	if (!m_lstDbcFilters.isEmpty())
		m_lstDbcFilters.last()->setAddButtonVisible(false);

	// Add filter to the list
	m_lstDbcFilters.append(newFilter);
	// Insert widget between the last DbcFilter widget and the spacer
	((QBoxLayout*) m_ui->frameFilter->layout())->insertWidget(((QBoxLayout*) m_ui->frameFilter->layout())->count() - 1,newFilter);
	connect(newFilter, SIGNAL(sDelete(int)), this, SLOT(onDeleteFilter(int)));
	connect(newFilter, SIGNAL(sAdd()), this, SLOT(onAddFilter()));
	if (newFilter->index() > m_iLastFilterIndex)
		m_iLastFilterIndex = newFilter->index();
	adjustSize();
}

///////////////////////////////////////////////////////////
// Add new filter
///////////////////////////////////////////////////////////
void DbcFilterSet::onAddFilter()
{
	m_iLastFilterIndex++;
	if (!m_pTableModelParam)
		GSLOG(SYSLOG_SEV_ERROR, "no table model.");
	
	DbcFilter* newFilter = new DbcFilter(m_pTableModelParam, m_iLastFilterIndex, this);
	addFilter(newFilter);
}

bool DbcFilterSet::isValueBasedOnFormula()
{
	bool bIsConversionOk = false;
	m_strValue.toFloat(&bIsConversionOk);
	
	return !bIsConversionOk;
}

///////////////////////////////////////////////////////////
// Load data from QDomElement and refresh view
///////////////////////////////////////////////////////////
void DbcFilterSet::loadData(const QDomElement& domElement)
{
	// Clear filter list
	while (!m_lstDbcFilters.isEmpty())
		delete m_lstDbcFilters.takeFirst();

	m_iLastFilterIndex = 0;
	// Load filter set
	if (!domElement.isNull())
	{
		setIndex(domElement.attribute("id").toInt());
	}
	// Load step list
	QDomElement elmtStepList = domElement.firstChildElement("step_list");
	if (!elmtStepList.isNull())
	{
		setStepList(elmtStepList.text());
	}
	// Load filter logic
	QDomElement elmtFilterLogic = domElement.firstChildElement("filter_logic");
	if (!elmtFilterLogic.isNull())
	{
		setFilterLogic(elmtFilterLogic.text());
	}
	// Load param name
	QDomElement elmtParamName = domElement.firstChildElement("parameter_name");
	if (!elmtParamName.isNull())
	{
		setParamName(elmtParamName.text());
	}
	// Load param number
	QDomElement elmtParamNumber = domElement.firstChildElement("parameter_number");
	if (!elmtParamNumber.isNull())
	{
		setParamNumber(elmtParamNumber.text().toInt());
	}
	// Load associated value
	QDomElement elmtValue = domElement.firstChildElement("value");
	if (!elmtValue.isNull())
	{
		setValue(elmtValue.text());
	}
	// Load is advanced
	QDomElement elmtAdvanced = domElement.firstChildElement("is_advanced");
	if (!elmtAdvanced.isNull())
	{
		setAdvanced(elmtAdvanced.text().toInt());
	}
	// Load Filters
	QDomNode nodeFilterList = domElement.firstChildElement("filter_list");
	if (!nodeFilterList.isNull())
	{
		QDomNode nodeFilter = nodeFilterList.firstChildElement("filter");
		while (!nodeFilter.isNull())
		{
			if (nodeFilter.isElement())
			{
				QDomElement elmtFilter = nodeFilter.toElement();
				if (!elmtFilter.isNull())
				{
					DbcFilter* newFilter = new DbcFilter(m_pTableModelParam, elmtFilter.attribute("id").toInt(), this);
					// name
					newFilter->setParamName(elmtFilter.firstChildElement("parameter_name").text());
					// number
					newFilter->setParamNumber(elmtFilter.firstChildElement("parameter_number").text().toInt());
					// operator
					newFilter->setOperator(elmtFilter.firstChildElement("operator").text());
					// value
					newFilter->setParamValue(elmtFilter.firstChildElement("value").text());
					addFilter(newFilter);
				}
			}
			nodeFilter = nodeFilter.nextSibling();
		}
	}
	
	resetView();
}

///////////////////////////////////////////////////////////
// Build and return QDomElement to save data
///////////////////////////////////////////////////////////
QDomElement DbcFilterSet::data(QDomDocument& domDocument)
{
	// Root node
	QDomElement elmtRoot = domDocument.createElement("filter_set");
	elmtRoot.setAttribute("id", m_iIndex);
	
	// Step List
	QDomElement elmtStepList = domDocument.createElement("step_list");
	elmtStepList.appendChild(domDocument.createTextNode(m_strStepList));
	elmtRoot.appendChild(elmtStepList);
	
	// Filter logic 
	QDomElement elmtFilterLogic = domDocument.createElement("filter_logic");
	elmtFilterLogic.appendChild(domDocument.createTextNode(m_strFilterLogic));
	elmtRoot.appendChild(elmtFilterLogic);
	
	if (m_ui->frameParameter->isVisible())
	{
		// Param name
		QDomElement elmtParamName = domDocument.createElement("parameter_name");
		elmtParamName.appendChild(domDocument.createTextNode(m_strParamName));
		elmtRoot.appendChild(elmtParamName);
		
		// Param number 
		QDomElement elmtParamNumber = domDocument.createElement("parameter_number");
		elmtParamNumber.appendChild(domDocument.createTextNode(QString::number(m_iParamNumber)));
		elmtRoot.appendChild(elmtParamNumber);
	}
	
	if (m_ui->frameValue->isVisible())
	{
		// Value 
		QDomElement elmtValue = domDocument.createElement("value");
		elmtValue.appendChild(domDocument.createTextNode(m_strValue));
		elmtRoot.appendChild(elmtValue);
	}
	
	// Value 
	QDomElement elmtAdvanced = domDocument.createElement("is_advanced");
	elmtAdvanced.appendChild(domDocument.createTextNode(QString::number(m_bIsAdvanced)));
	elmtRoot.appendChild(elmtAdvanced);
	
	// Filter List
	QDomElement elmtFilterList = domDocument.createElement("filter_list");
	elmtRoot.appendChild(elmtFilterList);
	
	if (!m_lstDbcFilters.isEmpty())
	{
		for (int i = 0; i < m_lstDbcFilters.size(); i++)
		{
			m_lstDbcFilters.at(i)->onChanged();
			QDomElement elmtFilter = domDocument.createElement("filter");
			elmtFilter.setAttribute("id", m_lstDbcFilters.at(i)->index());
			elmtFilterList.appendChild(elmtFilter);
			
			QDomElement elmtParamName = domDocument.createElement("parameter_name");
			elmtParamName.appendChild(domDocument.createTextNode(m_lstDbcFilters.at(i)->paramName()));
			elmtFilter.appendChild(elmtParamName);
			
			QDomElement elmtParamNumber = domDocument.createElement("parameter_number");
			elmtParamNumber.appendChild(domDocument.createTextNode(QString::number(m_lstDbcFilters.at(i)->paramNumber())));
			elmtFilter.appendChild(elmtParamNumber);
			
			QDomElement elmtOperator = domDocument.createElement("operator");
			elmtOperator.appendChild(domDocument.createTextNode(m_lstDbcFilters.at(i)->filterOperator()));
			elmtFilter.appendChild(elmtOperator);
			
			QDomElement elmtParamValue = domDocument.createElement("value");
			elmtParamValue.appendChild(domDocument.createTextNode(m_lstDbcFilters.at(i)->paramValue()));
			elmtFilter.appendChild(elmtParamValue);
		}
	}
	
	return elmtRoot;
}

///////////////////////////////////////////////////////////
// Build and return step, number separate by - or ,
///////////////////////////////////////////////////////////
QString DbcFilterSet::buildSimpleStepListQuery()
{
	// separator "," range "-"
	QString strFilterSet = m_strStepList;
	
	QStringList lstStep = strFilterSet.split(",");
	QString strFilterQuery = "SELECT " + QString(DBC_STEP_F_ID) + " FROM " + QString(DBC_STEP_T);
	// If select ALL
	if (strFilterSet.trimmed().isEmpty() || strFilterSet.trimmed() == "*")
	{
		return strFilterQuery;
	}
	
	QString strStepNumber;
	QString strWhereStmnt = "";
	// Build where statement
	foreach (strStepNumber, lstStep)
	{
		bool bIsStepId = false;
		// If it's not the first where statement
		if (!strWhereStmnt.isEmpty())
			strWhereStmnt += " OR ";
		
		// If it's a step range
		if ((strStepNumber).contains("-"))
		{
			QString strLeftStep, strRightStep = "";
			strLeftStep = strStepNumber.section("-", 0, 0); // Take first step
			strRightStep = strStepNumber.section("-", -1); // Take last step
			strWhereStmnt += "(" + QString(DBC_STEP_F_ID) + ">=\'" + strLeftStep + "\' AND " 
							+ QString(DBC_STEP_F_ID) + "<=\'" + strRightStep + "\')";
			strLeftStep.toInt(&bIsStepId);
			// If previous check was OK
			if (bIsStepId)
				strRightStep.toInt(&bIsStepId);
		}
		else
		{
			strWhereStmnt += " " + QString(DBC_STEP_F_ID) + "=\'" + strStepNumber + "\' ";
			strStepNumber.toInt(&bIsStepId);
		}
		if (!bIsStepId)
		{
			GSLOG(SYSLOG_SEV_ERROR, QString("Wrong step id in list: %1.")
			       .arg(strStepNumber));
			return strFilterQuery;
		}
	}
	strFilterQuery += " WHERE " + strWhereStmnt;
	
	return strFilterQuery;
}

///////////////////////////////////////////////////////////
// Build and return step list based on the line edit
///////////////////////////////////////////////////////////
QString DbcFilterSet::buildFilterQuery(int iIndex)
{
	QString strFilterString = "";
	int iFilter = 0;
	while (iFilter < m_lstDbcFilters.size())
	{
		if (m_lstDbcFilters.at(iFilter)->index() == iIndex)
		{
			strFilterString = "SELECT ";
			Gate_ParameterDef* paramInfo = m_pDbcTransac->testInfo(m_lstDbcFilters.at(iFilter)->paramNumber() ,m_lstDbcFilters.at(iFilter)->paramName());
			switch (paramInfo->m_bTestType)
			{
				case 'P':
					strFilterString += QString(DBC_PTEST_RESULT_T) + "." + QString(DBC_PTEST_RESULT_F_STEPID) + " "
					        "FROM "
					        "" + QString(DBC_TEST_T) + " INNER JOIN " + QString(DBC_PTEST_INFO_T) + " "
					        "ON " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_PTEST_INFO_T) + "." + QString(DBC_PTEST_INFO_F_TESTID) + " "
					        "INNER JOIN " + QString(DBC_PTEST_RESULT_T) + " "
					        "ON " + QString(DBC_PTEST_INFO_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_PTEST_RESULT_T) + "." + QString(DBC_PTEST_RESULT_F_TESTINFOID) + " "
					        "WHERE ";
					break;
				case 'F':
					/// TODO
					break;
				case 'M':
					/// TODO
					break;
				case 'S':
				strFilterString += QString(DBC_STEST_RESULT_T) + "." + QString(DBC_STEST_RESULT_F_STEPID) + " "
				        "FROM "
				        "" + QString(DBC_TEST_T) + " INNER JOIN " + QString(DBC_STEST_INFO_T) + " "
				        "ON " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_STEST_INFO_T) + "." + QString(DBC_STEST_INFO_F_TESTID) + " "
				        "INNER JOIN " + QString(DBC_STEST_RESULT_T) + " "
				        "ON " + QString(DBC_STEST_INFO_T) + "." + QString(DBC_TEST_F_ID) + "=" + QString(DBC_STEST_RESULT_T) + "." + QString(DBC_STEST_RESULT_F_TESTINFOID) + " "
				        "WHERE ";
					break;
				default:
					break;
			}
			strFilterString += 
			"("
			// Name
			" " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_NAME) + "=\"" + m_lstDbcFilters.at(iFilter)->paramName() + "\" "
			" AND "
			// Number
			" " + QString(DBC_TEST_T) + "." + QString(DBC_TEST_F_NUMBER) + "=\"" + QString::number(m_lstDbcFilters.at(iFilter)->paramNumber()) + "\" "
			" AND "
			// Value
			" value " + m_lstDbcFilters.at(iFilter)->filterOperator() + " \"" + m_lstDbcFilters.at(iFilter)->paramValue() + "\" "
			")";
			return strFilterString;
		}
		iFilter++;
	}
	
	return QString();
}

///////////////////////////////////////////////////////////
// Build and return step list based on the line edit
///////////////////////////////////////////////////////////
QString DbcFilterSet::buildFilterLogicQuery()
{
	QString strFilterLogic = m_strFilterLogic;
	strFilterLogic = strFilterLogic.toLower();
	strFilterLogic.replace("and", " INTERSECT ");
	strFilterLogic.replace("or", " UNION ");
	QString strFilterLogicQuery = "";
	QRegExp rx("(\\d+)");
	int iCountCap = 0;
	
	int iIndexOfFilter = 0;
	int pos = rx.indexIn(strFilterLogic);
	while (pos > -1)
	{
		iIndexOfFilter = strFilterLogic.indexOf(rx.cap(1));
		// Get string before filter
		strFilterLogicQuery += strFilterLogic.left(iIndexOfFilter);
		strFilterLogic.remove(0, iIndexOfFilter);
		// Get filter
		QString strFilter = buildFilterQuery(rx.cap(1).toInt());
		if (strFilter.isEmpty())
			GSLOG(SYSLOG_SEV_ERROR, QString("unable to find filter associated to: %1.")
			       .arg(rx.cap(1)));
		
		// Add filter to query
		strFilterLogicQuery += strFilter;
		// Remove filter filter logic
		strFilterLogic.remove(0, rx.cap(1).size());
		pos = rx.indexIn(strFilterLogic);
		iCountCap++;
	}
	if (iCountCap > 1)
	{
		strFilterLogicQuery.prepend("SELECT * FROM (");
		strFilterLogicQuery.append(")");
	}
	
	return strFilterLogicQuery;
}

