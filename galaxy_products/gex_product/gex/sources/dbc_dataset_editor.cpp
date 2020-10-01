#include <QDir>
#include <QMenu>
#include <QFileInfo>
#include <QSqlTableModel>
#include <QFileDialog>
#include <QDomDocument>
#include <QTextStream>

#include <gqtl_log.h>
#include "dbc_filter.h"
#include "dbc_parameter.h"
#include "db_sqlite_man.h"
#include "gate_event_manager.h"
#include "dbc_form_param_results.h"
#include "dbc_form_param_groups.h"
#include "dbc_report_editor.h"

#include "dbc_dataset_editor.h"
#include "ui_dbc_dataset_editor.h"



///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcDatasetEditor::DbcDatasetEditor(DbcTransaction& dbTransac, QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::DbcDatasetEditor)
{
	m_pDbcTransac = NULL;
	m_pTableModelPtest = NULL;
	m_pTableModelStest = NULL;
	m_pTableModelParam = NULL;
	m_pFormParamGroups = NULL;
	m_ui->setupUi(this);
	m_strFilterLogicOnSteps = "";
	m_strFiltersXmlFilePath = "";
	m_strFilterQueryOnSteps = "";
	
	setDbcTransac(dbTransac);
	initPTestTableModel();
	initSTestTableModel();
	initParamTableModel();
	m_strFiltersXmlFilePath = QString(m_pDbcTransac->dbFolder() + "filters.xml");
	loadXmlFilterFile(m_strFiltersXmlFilePath);
	initGui();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcDatasetEditor::~DbcDatasetEditor()
{
	while (!m_lstDbcFilters.isEmpty())
	     delete m_lstDbcFilters.takeFirst();
	delete m_pTableModelPtest;
	delete m_pTableModelStest;
	delete m_pTableModelParam;
	delete m_pFormParamGroups;
	delete m_ui;
}

///////////////////////////////////////////////////////////
// Set transac *
///////////////////////////////////////////////////////////
void DbcDatasetEditor::setDbcTransac(DbcTransaction& dbTransac)
{
	m_pDbcTransac = &dbTransac;
	m_pDbcTransac->loadSessionInfo();
}

///////////////////////////////////////////////////////////
// Init ui
///////////////////////////////////////////////////////////
void DbcDatasetEditor::initGui()
{
	m_ui->progressBar->setVisible(false);
	
	setWindowTitle(m_pDbcTransac->dbName());
       Qt::WindowFlags flags = 0;
       flags |= Qt::WindowMaximizeButtonHint;
       flags |= Qt::WindowMinimizeButtonHint;
       setWindowFlags( flags );
	m_selectedParam.clear();
	loadParamInfo();
	m_ui->groupBoxResults->setVisible(false);
	
	m_pFormParamGroups = new DbcFormParamGroups(m_pDbcTransac, this);
	m_ui->tabWidgetGroups->layout()->addWidget(m_pFormParamGroups);
	
	connect(m_ui->treeWidgetParamInfo, SIGNAL(itemSelectionChanged()), this, SLOT(onParamInfoSelectionChanged()));
	connect(m_ui->treeWidgetParamInfo, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested()));
	connect(m_ui->pushButtonAddParameter, SIGNAL(clicked()), this, SLOT(onAddParamFilterRequested()));
	connect(m_ui->pushButtonRemoveParameter, SIGNAL(clicked()), this, SLOT(onRemoveParamFilterRequested()));
	connect(m_ui->pushButtonSelectFile, SIGNAL(clicked()), this, SLOT(onSelectFileRequested()));
	connect(m_ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onSaveRequested()));
	connect(this, SIGNAL(sChangeDetected()), this, SLOT(onFiltersChanged()));
	connect(m_ui->lineEditFilterLogicOnSteps, SIGNAL(textChanged(QString)), this, SLOT(onFiltersChanged()));
	connect(m_ui->pushButtonApplyFilters, SIGNAL(clicked()), this, SLOT(onApplyFiltersRequested()));
	connect(m_pTableModelPtest, SIGNAL(dataChanged(QModelIndex,QModelIndex)),this, SLOT(onDataChanged()));
	connect(m_pTableModelStest, SIGNAL(dataChanged(QModelIndex,QModelIndex)),this, SLOT(onDataChanged()));
	connect(m_ui->pushButtonReportEditor, SIGNAL(clicked()), this, SLOT(onReportEditorRequested()));
}

///////////////////////////////////////////////////////////
// Init model linked to Ptest table
///////////////////////////////////////////////////////////
void DbcDatasetEditor::initPTestTableModel()
{
	m_pTableModelPtest = new QSqlTableModel(this, QSqlDatabase::database(m_pDbcTransac->dbConnection()->connectionName()));
	m_pTableModelPtest->setTable(QString(DBC_PTEST_RESULT_T));
	m_pTableModelPtest->setEditStrategy(QSqlTableModel::OnManualSubmit);
	m_pTableModelPtest->sort(0,Qt::AscendingOrder);
	resetPTestTableModel();
}

///////////////////////////////////////////////////////////
// Init model linked to Ptest table
///////////////////////////////////////////////////////////
void DbcDatasetEditor::initSTestTableModel()
{
	m_pTableModelStest = new QSqlTableModel(this, QSqlDatabase::database(m_pDbcTransac->dbConnection()->connectionName()));
	m_pTableModelStest->setTable(QString(DBC_STEST_RESULT_T));
	m_pTableModelStest->setEditStrategy(QSqlTableModel::OnManualSubmit);
	m_pTableModelStest->sort(0,Qt::AscendingOrder);
	resetSTestTableModel();
}

///////////////////////////////////////////////////////////
// Init model linked to Ptest table
///////////////////////////////////////////////////////////
void DbcDatasetEditor::initParamTableModel()
{
	m_pTableModelParam = new ParamSqlTableModel(this, QSqlDatabase::database(m_pDbcTransac->dbConnection()->connectionName()));
	m_pTableModelParam->setTable(QString(DBC_TEST_T));
	resetParamTableModel();
}

///////////////////////////////////////////////////////////
// Reset model linked to Ptest table
///////////////////////////////////////////////////////////
void DbcDatasetEditor::resetPTestTableModel()
{
	if (!m_strFilterQueryOnSteps.isEmpty())
	{
		QString strFilters = "";
		strFilters = QString(DBC_PTEST_RESULT_F_STEPID) + " NOT IN (" + m_strFilterQueryOnSteps + ")";
		m_pTableModelPtest->setFilter(strFilters);
	}
	else
		m_pTableModelPtest->setFilter("");
	
	m_pTableModelPtest->select();
	while (m_pTableModelPtest->canFetchMore())
		m_pTableModelPtest->fetchMore();
}

///////////////////////////////////////////////////////////
// Reset model linked to Ptest table
///////////////////////////////////////////////////////////
void DbcDatasetEditor::resetSTestTableModel()
{
	if (!m_strFilterQueryOnSteps.isEmpty())
	{
		QString strFilters = "";
		strFilters = QString(DBC_STEST_RESULT_F_STEPID) + " NOT IN (" + m_strFilterQueryOnSteps + ")";
		m_pTableModelStest->setFilter(strFilters);
	}
	else
		m_pTableModelStest->setFilter("");
	
	m_pTableModelStest->select();
	while (m_pTableModelStest->canFetchMore())
		m_pTableModelStest->fetchMore();
}

///////////////////////////////////////////////////////////
// Reset model linked to Ptest table
///////////////////////////////////////////////////////////
void DbcDatasetEditor::resetParamTableModel()
{
	m_pTableModelParam->select();
	m_ui->treeWidgetParamInfo->sortItems(DBC_PARAMETERS_COLUMN_NUMBER, Qt::AscendingOrder);
	while (m_pTableModelParam->canFetchMore())
		m_pTableModelParam->fetchMore();
}

///////////////////////////////////////////////////////////
// Apply all pending updates on the database
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onDataChanged()
{
	m_pTableModelPtest->submitAll();
	resetPTestTableModel();
	m_pTableModelStest->submitAll();
	resetSTestTableModel();
}

///////////////////////////////////////////////////////////
// Init line edit
///////////////////////////////////////////////////////////
void DbcDatasetEditor::initLineEditFilterLogicOnSteps()
{
	m_ui->lineEditFilterLogicOnSteps->setText(m_strFilterLogicOnSteps);
}

///////////////////////////////////////////////////////////
// Load parameters list
///////////////////////////////////////////////////////////
void DbcDatasetEditor::loadParamInfo()
{
	m_ui->treeWidgetParamInfo->hideColumn(0);
	m_ui->treeWidgetParamInfo->setSortingEnabled(true);

	// Load test list
	QListIterator<Gate_ParameterDef*> itTest(m_pDbcTransac->testList());
	QList<QTreeWidgetItem *> items; 
	
	// clean treewidget
	while (m_ui->treeWidgetParamInfo->topLevelItem(0))
		delete m_ui->treeWidgetParamInfo->takeTopLevelItem(0);
	// Load tree widget
	while(itTest.hasNext())
	{
		Gate_ParameterDef* paramInfo = itTest.next();
		QStringList lstItemInfo;
		lstItemInfo << QString::number(paramInfo->m_nId) << "Test" << QString::number(paramInfo->m_nParameterNumber) << paramInfo->m_strName <<  QString((QChar)paramInfo->m_bTestType);
		items.append(new QTreeWidgetItem(lstItemInfo));
	}
	
	m_ui->treeWidgetParamInfo->insertTopLevelItems(0, items);
	
	for (int i = 0; i < m_ui->treeWidgetParamInfo->columnCount(); i++)
		m_ui->treeWidgetParamInfo->resizeColumnToContents(i);
}

///////////////////////////////////////////////////////////
// Load parameter selection
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onParamInfoSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = m_ui->treeWidgetParamInfo->selectedItems();
	QList<int> selectedParamId;

	selectedParamId.clear();
	for (int i = 0; i < selectedItems.size(); ++i)
		selectedParamId << selectedItems.at(i)->data(DBC_PARAMETERS_COLUMN_TESTID, Qt::DisplayRole).toInt();

	// Remove deselected param
	QMap<int, DbcFormParamResults*>::const_iterator it = m_selectedParam.constBegin();
	while (it != m_selectedParam.constEnd())
	{
		if(!selectedParamId.contains(it.key()))
		{
			delete m_selectedParam.take(it.key());
			it = m_selectedParam.constBegin();
		}
		else
			++it;
	}
	// Show selected parameters
	for (int i = 0; i < selectedParamId.size(); ++i)
	{
		if (!m_selectedParam.contains(selectedParamId.at(i)))
		{
			// Get linked paramInfo object
			Gate_ParameterDef* paramInfo = m_pDbcTransac->testInfo(selectedParamId.at(i));
			
			// If we have results on this param
			if (paramInfo->m_lstnInfoId.size() > 0)
			{
				DbcFormParamResults* newFormParamResult = NULL;
				switch (paramInfo->m_bTestType)
				{
					case 'P':
						newFormParamResult = new DbcFormParamResults(*m_pDbcTransac, *paramInfo, m_pTableModelPtest, m_pTableModelParam, m_ui->splitterParamResults);
						break;
					case 'F':
						/// TODO
						break;
					case 'M':
						/// TODO
						break;
					case 'S':
						newFormParamResult = new DbcFormParamResults(*m_pDbcTransac, *paramInfo, m_pTableModelStest, m_pTableModelParam, m_ui->splitterParamResults);
						break;
					default:
						break;
				}
				if (newFormParamResult)
				{
					connect(this, SIGNAL(sChangeDetected()), newFormParamResult, SLOT(refreshGui()));
					connect(newFormParamResult, SIGNAL(sChanged()), this, SLOT(onDataChanged()));
					m_selectedParam.insert(selectedParamId.at(i), newFormParamResult);
					m_ui->splitterParamResults->addWidget(newFormParamResult);
				}
				else
					return;
			}
		}
	}
	if (!m_selectedParam.isEmpty())
		m_ui->groupBoxResults->setVisible(true);
	else
		m_ui->groupBoxResults->setVisible(false);
}

///////////////////////////////////////////////////////////
// Show context menu
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onCustomContextMenuRequested()
{
	QMenu menu(this);
	
	menu.addAction("Edit", this, SLOT(onEditParamFilterRequested()));
	menu.addAction("Add new", this, SLOT(onAddParamFilterRequested()));
	menu.addAction("Remove", this, SLOT(onRemoveParamFilterRequested()));

	menu.exec(QCursor::pos());
}

///////////////////////////////////////////////////////////
// Load gui to edit parameter or filter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onEditParamFilterRequested()
{
	switch(m_ui->tabWidgetParamFilter->currentIndex())
	{
	case DBC_TAB_PARAMFILTER_PARAM:
		onEditParameterRequested();
		break;
	case DBC_TAB_PARAMFILTER_FILTER:
//		onEditFilterRequested();
		break;
	}
	emit sChangeDetected();
}

///////////////////////////////////////////////////////////
// Load gui to edit the selected parameter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onEditParameterRequested()
{
	int iSelectedParamId = m_ui->treeWidgetParamInfo->selectedItems().first()->data(DBC_PARAMETERS_COLUMN_TESTID, Qt::DisplayRole).toInt();
	// Get linked paramInfo object
	Gate_ParameterDef* paramInfo = m_pDbcTransac->testInfo(iSelectedParamId);
	if (m_pDbcTransac->pTestInfo(paramInfo))
		editParameter(paramInfo);
	else
	{
		GSLOG(SYSLOG_SEV_ERROR, QString("unable to find test: %1 in the database.")
		       .arg(m_ui->treeWidgetParamInfo->selectedItems().first()->data(DBC_PARAMETERS_COLUMN_NUMBER, Qt::DisplayRole).toString()));
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcDatasetEditor::editParameter(Gate_ParameterDef* paramInfo)
{
	DbcParameter newParameter(m_pDbcTransac, m_pTableModelParam,DbcParameter::eUpdate, this);
	newParameter.setParamName(paramInfo->m_strName);
	newParameter.setParamNumber(paramInfo->m_nParameterNumber);
	newParameter.setType(QString(paramInfo->m_bTestType));
	newParameter.setUnit(paramInfo->m_strUnits);
	newParameter.setLowL(paramInfo->m_lfLowL);
	newParameter.setHighL(paramInfo->m_lfHighL);
	newParameter.exec();
}

///////////////////////////////////////////////////////////
// Load gui to add parameter or filter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onAddParamFilterRequested()
{
	switch(m_ui->tabWidgetParamFilter->currentIndex())
	{
	case DBC_TAB_PARAMFILTER_PARAM:
		onAddParameterRequested();
		break;
	case DBC_TAB_PARAMFILTER_FILTER:
		onAddFilterRequested();
		break;
	case DBC_TAB_PARAMFILTER_GROUP:
		m_pFormParamGroups->onAddRequested();
		break;
	}
	emit sChangeDetected();
}

///////////////////////////////////////////////////////////
// Create new parameter and add it
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onAddParameterRequested()
{
	DbcParameter newParameter(m_pDbcTransac, m_pTableModelParam,DbcParameter::eCreate, this);
	addParameter(&newParameter);
}

///////////////////////////////////////////////////////////
// Create new filter and add it
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onAddFilterRequested()
{
	DbcFilter* newFilter = new DbcFilter(m_pTableModelParam, DbcFilter::lastIndex() + 1, this);
	addFilter(newFilter);
}

///////////////////////////////////////////////////////////
//  Add new filter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::addFilter(DbcFilter* newFilter)
{
	m_lstDbcFilters.append(newFilter);
	// Insert widget between the last DbcFilter widget and the spacer
	((QBoxLayout*) m_ui->scrollAreaWidgetContents->layout())->insertWidget(((QBoxLayout*) m_ui->scrollAreaWidgetContents->layout())->count() - 1,newFilter);
	connect(newFilter, SIGNAL(sDelete(int)), this, SLOT(onDeleteFilter(int)));
	connect(newFilter, SIGNAL(sAdd()), this, SLOT(onAddFilterRequested()));
	connect(newFilter, SIGNAL(sChanged()), this, SLOT(onFiltersChanged()));
}

///////////////////////////////////////////////////////////
// Add new parameter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::addParameter(DbcParameter* newParameter)
{
	newParameter->exec();
	// refresh param info view
	onDataChanged();
	loadParamInfo();
}

///////////////////////////////////////////////////////////
// Remove parameter or filter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onRemoveParamFilterRequested()
{
	switch(m_ui->tabWidgetParamFilter->currentIndex())
	{
	case DBC_TAB_PARAMFILTER_PARAM:
		onRemoveParameterRequested();
		break;
	case DBC_TAB_PARAMFILTER_FILTER:
//		onRemoveFilterRequested();
		break;
	}
	emit sChangeDetected();
}

///////////////////////////////////////////////////////////
// Remove selected parameters
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onRemoveParameterRequested()
{
	QList<QTreeWidgetItem*> selectedItems = m_ui->treeWidgetParamInfo->selectedItems();
	QList<int> selectedParamId;

	selectedParamId.clear();
	for (int i = 0; i < selectedItems.size(); ++i)
		selectedParamId << selectedItems.at(i)->data(DBC_PARAMETERS_COLUMN_TESTID, Qt::DisplayRole).toInt();
	
	// Show selected parameters
	for (int i = 0; i < selectedParamId.size(); ++i)
	{
		// Get linked paramInfo object
		Gate_ParameterDef* paramInfo = m_pDbcTransac->testInfo(selectedParamId.at(i));
		if (m_pDbcTransac->pTestInfo(paramInfo))
		{
			removeParameter(paramInfo);
		}
	}
}

///////////////////////////////////////////////////////////
// Remove parameter paramInfo
///////////////////////////////////////////////////////////
void DbcDatasetEditor::removeParameter(Gate_ParameterDef* paramInfo)
{
	m_pDbcTransac->removeTest(paramInfo);
	loadParamInfo();
}

///////////////////////////////////////////////////////////
// Launch gui to load new filter or parameter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onSelectFileRequested()
{
	switch(m_ui->tabWidgetParamFilter->currentIndex())
	{
	case DBC_TAB_PARAMFILTER_PARAM:
		onXmlParameterRequested();
		break;
	case DBC_TAB_PARAMFILTER_FILTER:
		onXmlFilterRequested();
		break;
	}
	emit sChangeDetected();
}

///////////////////////////////////////////////////////////
// Save param or filter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onSaveRequested()
{
	switch(m_ui->tabWidgetParamFilter->currentIndex())
	{
	case DBC_TAB_PARAMFILTER_PARAM:
		break;
	case DBC_TAB_PARAMFILTER_FILTER:
		onSaveFiltersRequested();
		break;
	}
}

///////////////////////////////////////////////////////////
// Save filter
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onSaveFiltersRequested()
{
	if (saveXmlFilterFile(m_strFiltersXmlFilePath))
		m_ui->pushButtonSave->setEnabled(false);
	else
		GSLOG(SYSLOG_SEV_ERROR, QString("unable to save %1.")
		       .arg(m_strFiltersXmlFilePath));
}

///////////////////////////////////////////////////////////
// Transform filter logic to filter logic query
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onApplyFiltersRequested()
{
	if (m_ui->pushButtonApplyFilters->isChecked())
		applyFiltersOnSteps();
	else
		m_strFilterQueryOnSteps = "";
	
	onDataChanged();
	
	emit sChangeDetected();
}

///////////////////////////////////////////////////////////
// Apply filter on database
///////////////////////////////////////////////////////////
void DbcDatasetEditor::applyFiltersOnSteps()
{
	m_strFilterLogicOnSteps = m_ui->lineEditFilterLogicOnSteps->text();
	QString strFilterLogic = m_strFilterLogicOnSteps;
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
		QString strFilter = filterString(rx.cap(1).toInt());
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
	
	m_strFilterQueryOnSteps = strFilterLogicQuery;
	
	GSLOG(SYSLOG_SEV_DEBUG, QString("result: %1.")
	       .arg(strFilterLogicQuery));
}

///////////////////////////////////////////////////////////
// Launch gui to select new xml to lad new filters
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onXmlFilterRequested()
{
	// Display Open file dialog so the user can select an xml file
	QString strFilePath = QFileDialog::getOpenFileName(this, "Select the xml file to define filters", m_strFiltersXmlFilePath);
	// Check if new file selected
	if((strFilePath.isEmpty() == true) || (m_strFiltersXmlFilePath == strFilePath))
		return;

	loadXmlFilterFile(strFilePath);
}

///////////////////////////////////////////////////////////
// Load xml to load new filters
///////////////////////////////////////////////////////////
bool DbcDatasetEditor::loadXmlFilterFile(QString &strFilePath)
{
	if (!strFilePath.isEmpty())
	{
		QFile file(strFilePath);
		if (file.open(QIODevice::ReadOnly))
		{
			QString strErrorMsg;
			int iErrorLine, iErrorColumn;
			if (m_domDocXmlFilters.setContent(&file, &strErrorMsg, &iErrorLine, &iErrorColumn)) 
			{
				loadFilterList(m_domDocXmlFilters);
			}
			else
				GSLOG(SYSLOG_SEV_ERROR, QString("%1 at line %2 column %SYSLOG_SEV_ERROR.")
				       .arg(strErrorMsg)
				       .arg(QString::number(iErrorLine))
				       .arg(QString::number(iErrorColumn)));
			file.close();
		}
	}

	return true;
}

///////////////////////////////////////////////////////////
// Save filters to xml file
///////////////////////////////////////////////////////////
bool DbcDatasetEditor::saveXmlFilterFile(QString &strFilePath)
{
	if (!strFilePath.isEmpty())
	{
		QFile file(strFilePath);
		if (file.open(QIODevice::WriteOnly))
		{
			QTextStream textStream(&file);
			QDomDocument document = saveFilterList();
			document.save(textStream, 0);
			file.close();
			return true;
		}
	}
	return false;
}


///////////////////////////////////////////////////////////
// Load filter list based on Qdomdocument
///////////////////////////////////////////////////////////
bool DbcDatasetEditor::loadFilterList(QDomDocument &document)
{
	// Clear filter list
	while (!m_lstDbcFilters.isEmpty())
		delete m_lstDbcFilters.takeFirst();
	DbcFilter::initIndex();
	
	QDomElement elmtDbcFilters = document.firstChildElement("gex_dbc_filters");
	
	// Load filter logic
	QDomNode nodeFilterLogic = elmtDbcFilters.firstChildElement("filter_logic");
	if (!nodeFilterLogic.isNull())
	{
		QDomElement elmtFilterLogicOnSteps = nodeFilterLogic.firstChildElement("on_steps"); // try to convert the node to an element.
		if (!elmtFilterLogicOnSteps.isNull())
		{
			setFilterLogicOnSteps(elmtFilterLogicOnSteps.text());
		}
	}
	// Load Filters
	QDomNode nodeFilterList = elmtDbcFilters.firstChildElement("filter_list");
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
					elmtFilter.attribute("id");
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
	return true;
}

///////////////////////////////////////////////////////////
// Save filters to QDomeDocuments
///////////////////////////////////////////////////////////
QDomDocument DbcDatasetEditor::saveFilterList()
{
	QDomDocument document;

	// Root node
	QDomElement elmtRoot = document.createElement("gex_dbc_filters");
	elmtRoot.setAttribute("version", "1.0");
	document.appendChild(elmtRoot);
	
	// Filter logic 
	QDomElement elmtFilterLogic = document.createElement("filter_logic");
	elmtRoot.appendChild(elmtFilterLogic);
	
	QDomElement elmtFilterLogicOnSteps = document.createElement("on_steps");
	elmtFilterLogicOnSteps.appendChild(document.createTextNode(m_ui->lineEditFilterLogicOnSteps->text()));
	elmtFilterLogic.appendChild(elmtFilterLogicOnSteps);
	
	QDomElement elmtFilterList = document.createElement("filter_list");
	elmtRoot.appendChild(elmtFilterList);

	for (int i = 0; i < m_lstDbcFilters.size(); i++)
	{
		m_lstDbcFilters.at(i)->onChanged();
		QDomElement elmtFilter = document.createElement("filter");
		elmtFilter.setAttribute("id", m_lstDbcFilters.at(i)->index());
		elmtFilterList.appendChild(elmtFilter);
		
		QDomElement elmtParamName = document.createElement("parameter_name");
		elmtParamName.appendChild(document.createTextNode(m_lstDbcFilters.at(i)->paramName()));
		elmtFilter.appendChild(elmtParamName);

		QDomElement elmtParamNumber = document.createElement("parameter_number");
		elmtParamNumber.appendChild(document.createTextNode(QString::number(m_lstDbcFilters.at(i)->paramNumber())));
		elmtFilter.appendChild(elmtParamNumber);
		
		QDomElement elmtOperator = document.createElement("operator");
		elmtOperator.appendChild(document.createTextNode(m_lstDbcFilters.at(i)->filterOperator()));
		elmtFilter.appendChild(elmtOperator);
		
		QDomElement elmtParamValue = document.createElement("value");
		elmtParamValue.appendChild(document.createTextNode(m_lstDbcFilters.at(i)->paramValue()));
		elmtFilter.appendChild(elmtParamValue);
	}
	
	return document;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcDatasetEditor::setFilterLogicOnSteps(QString strFilterLogic)
{
	m_strFilterLogicOnSteps = strFilterLogic;
	initLineEditFilterLogicOnSteps();
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
QString DbcDatasetEditor::filterString(int iIndex)
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
// Load new parameter from xml
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onXmlParameterRequested()
{
	DbcParameter newParameter(m_pDbcTransac, m_pTableModelParam,DbcParameter::eCreate, this);
	if (newParameter.onLoadParameter())
		newParameter.exec();
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onFiltersChanged()
{
	m_ui->pushButtonSave->setEnabled(true);
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onDeleteFilter(int iIndex)
{
	bool bFilterFound = false;
	int iFilter = 0;
	while (!bFilterFound && iFilter < m_lstDbcFilters.size())
	{
		if (m_lstDbcFilters.at(iFilter)->index() == iIndex)
		{
			bFilterFound = true;
			delete m_lstDbcFilters.takeAt(iFilter);
		}
		iFilter++;
	}
	
	if (m_lstDbcFilters.size() == 0)
		DbcFilter::initIndex();
	
	emit sChangeDetected();
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcDatasetEditor::saveXmlUpdate()
{
	m_ui->pushButtonSave->setEnabled(false);
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcDatasetEditor::onReportEditorRequested()
{
	DbcReportEditor* reportEditor = new DbcReportEditor(this);
	reportEditor->exec();
}

