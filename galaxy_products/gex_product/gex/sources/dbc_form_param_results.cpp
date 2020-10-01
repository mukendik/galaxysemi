
#include <QSortFilterProxyModel>
#include <QSqlTableModel>
#include <QMenu>
#include "gate_event_manager.h"
#include "dbc_transaction.h"
#include "dbc_parameter.h"


#include "dbc_form_param_results.h"
#include "ui_dbc_form_param_results.h"



///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcFormParamResults::DbcFormParamResults(DbcTransaction& dbTransac, Gate_ParameterDef& paramInfo, QSqlTableModel *pTableModelResults, ParamSqlTableModel *pTableModelInfo, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::DbcFormParamResults)
{
	m_ui->setupUi(this);
	setDbcTransac(dbTransac);
	setParamInfo(paramInfo);
	setResultsModel(pTableModelResults);
	setInfoModel(pTableModelInfo);
	initModel();
	initGui();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcFormParamResults::~DbcFormParamResults()
{
    delete m_ui;
	delete m_paramInfo;
	delete m_pProxyModel;
}

///////////////////////////////////////////////////////////
// Set transaction *
///////////////////////////////////////////////////////////
void DbcFormParamResults::setDbcTransac(DbcTransaction& dbTransac)
{
	m_pDbcTransac = &dbTransac;
}

///////////////////////////////////////////////////////////
// Set param info
///////////////////////////////////////////////////////////
void DbcFormParamResults::setParamInfo(Gate_ParameterDef &paramInfo)
{
	m_paramInfo = &paramInfo;
}

///////////////////////////////////////////////////////////
// Set model
///////////////////////////////////////////////////////////
void DbcFormParamResults::setResultsModel(QSqlTableModel *pSqlTableModel)
{
	m_pTableModelResults = pSqlTableModel;
}

///////////////////////////////////////////////////////////
// Set model
///////////////////////////////////////////////////////////
void DbcFormParamResults::setInfoModel(ParamSqlTableModel *pParamTableModel)
{
	m_pTableModelInfo = pParamTableModel;
}

///////////////////////////////////////////////////////////
// Initialize model
///////////////////////////////////////////////////////////
void DbcFormParamResults::initModel()
{
	m_pProxyModel = new QSortFilterProxyModel(this);
	m_pProxyModel->setSourceModel(m_pTableModelResults);
	QString strTestInfoId = QString::number(m_paramInfo->m_lstnInfoId.at(0));
	for (int i = 1; i < m_paramInfo->m_lstnInfoId.size(); i++)
		strTestInfoId += "|" + QString::number(m_paramInfo->m_lstnInfoId.at(i));
	m_pProxyModel->setFilterRegExp("^(" + strTestInfoId + ")$");
	m_pProxyModel->setFilterKeyColumn(1);
	m_pProxyModel->sort(m_pTableModelResults->fieldIndex(QString(DBC_PTEST_RESULT_F_STEPID)));
	m_ui->tableView->setSortingEnabled(true);
	m_ui->tableView->setModel(m_pProxyModel);
//	connect(m_pTableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(sChanged()));
}

///////////////////////////////////////////////////////////
// Initialize Gui
///////////////////////////////////////////////////////////
void DbcFormParamResults::initGui()
{
	// Customize table view
//	m_ui->tableView->hideColumn(m_pTableModel->fieldIndex(QString(DBC_TEST_T) + "_" + QString(DBC_TEST_F_ID)));
	m_ui->tableView->verticalHeader()->hide();
	resizeTableView();
	m_ui->tableView->setItemDelegateForColumn(4, new ParamResultDelegate(this));
	
	// Load test result list
	m_ui->labelParamNumber->setText(QString::number(m_paramInfo->m_nParameterNumber));
	m_ui->labelParamUnit->setText(m_paramInfo->m_strUnits);
	m_ui->labelParamHLimit->setText(QString::number(m_paramInfo->m_lfHighL));
	m_ui->labelParamLLimit->setText(QString::number(m_paramInfo->m_lfLowL));
	m_ui->labelParamName->setText(m_paramInfo->m_strName);
	m_ui->labelParamType->setText(QChar(m_paramInfo->m_bTestType));
	m_ui->labelCountResults->setText(QString::number(m_pProxyModel->rowCount()) + " results");
	
	connect(m_ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested()));
	
}

///////////////////////////////////////////////////////////
// Resize table view to content
///////////////////////////////////////////////////////////
void DbcFormParamResults::resizeTableView()
{
	m_ui->tableView->resizeColumnsToContents();
	m_ui->tableView->adjustSize();
}

///////////////////////////////////////////////////////////
// Refresh ui
///////////////////////////////////////////////////////////
void DbcFormParamResults::refreshGui()
{
	m_ui->labelCountResults->setText(QString::number(m_pProxyModel->rowCount()) + " results");
}

///////////////////////////////////////////////////////////
// Show context menu
///////////////////////////////////////////////////////////
void DbcFormParamResults::onCustomContextMenuRequested()
{
	QMenu menu(this);
	
	menu.addAction("Edit", this, SLOT(onEditParameterRequested()));

	menu.exec(QCursor::pos());
}

///////////////////////////////////////////////////////////
// Load gui to edit the selected parameter
///////////////////////////////////////////////////////////
void DbcFormParamResults::onEditParameterRequested()
{
	QStringList lstStep;
	// Get selected step
	QModelIndexList indexes = m_ui->tableView->selectionModel()->selectedIndexes();
	QModelIndex index;
	foreach (index, indexes)
		lstStep << (m_pProxyModel->index(index.row(), 0)).data().toString();

	lstStep.removeDuplicates();
	
	// Load parameter editor
	DbcParameter newParameter(m_pDbcTransac, m_pTableModelInfo,DbcParameter::eUpdate, this);
	newParameter.setParamName(m_paramInfo->m_strName);
	newParameter.setParamNumber(m_paramInfo->m_nParameterNumber);
	newParameter.setType(QString(m_paramInfo->m_bTestType));
	newParameter.setUnit(m_paramInfo->m_strUnits);
	newParameter.setLowL(m_paramInfo->m_lfLowL);
	newParameter.setHighL(m_paramInfo->m_lfHighL);
	newParameter.setAdvanced(true);
	newParameter.setStepLit(lstStep.join(","));
	newParameter.exec();
	
	emit sChanged();
}

