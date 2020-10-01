#include <QMenu>
#include <QMap>
#include <QInputDialog>

#include <gqtl_log.h>
#include "dbc_group.h"
#include "dbc_transaction.h"
#include "gate_event_manager.h"

#include "dbc_form_param_groups.h"
#include "ui_dbc_form_param_groups.h"


///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DbcFormParamGroups::DbcFormParamGroups(DbcTransaction *pDbcTransac, QWidget *parent) :
    QFrame(parent),
    m_ui(new Ui::DbcFormParamGroups)
{
    m_ui->setupUi(this);
	m_pDbcTransac = pDbcTransac;
	m_mapIdpGroup.clear();
	loadGroupList();
	
	connect(m_ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested()));
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DbcFormParamGroups::~DbcFormParamGroups()
{
    delete m_ui;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
bool DbcFormParamGroups::loadGroupList()
{
	// Clear tree widget
	m_ui->treeWidget->clear();
	
	m_mapIdpGroup = m_pDbcTransac->groupList();
	
	// Load groups of group
	/// TODO
	
	// Load groups
	QMapIterator<int, DbcGroup*> itGroup(m_mapIdpGroup);
	while (itGroup.hasNext()) 
	{
		itGroup.next();
		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(itGroup.value()->groupName()));
		QList<int> lstParameter = itGroup.value()->parameterList();
		int iParamId;
		foreach (iParamId, lstParameter)
		{
			QStringList lstParamInfo;
			Gate_ParameterDef* paramInfo = m_pDbcTransac->testInfo(iParamId);
			if (paramInfo)
			{
				lstParamInfo << "" << QString::number(paramInfo->m_nParameterNumber) << paramInfo->m_strName;
				QTreeWidgetItem *itemParam = new QTreeWidgetItem(lstParamInfo);
				item->addChild(itemParam);
			}
			else
				GSLOG(SYSLOG_SEV_ERROR, QString("cannot load parameter id %1 to the group %2.")
				                  .arg(QString::number(iParamId))
				                  .arg(itGroup.value()->groupName()));
		}
		
		m_ui->treeWidget->addTopLevelItem(item);
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcFormParamGroups::onCustomContextMenuRequested()
{
	QMenu menu(this);
	QList<QTreeWidgetItem*> selectedItems = m_ui->treeWidget->selectedItems();
	if (selectedItems.size() <= 0)
		return;
		
	menu.addAction("New group", this, SLOT(onAddRequested()));
	
	// If its a parameter
	if (selectedItems.first()->parent())
	{
		QAction* pAction = new QAction("Add to group", this);
		QMenu *subMenu = new QMenu(this);
		QMapIterator<int, DbcGroup*> itGroup(m_mapIdpGroup);
		while (itGroup.hasNext()) 
		{
			itGroup.next();
			subMenu->addAction(itGroup.value()->groupName());
		}

		connect(subMenu, SIGNAL(triggered(QAction*)), this, SLOT(onAddParameterToGroup(QAction*)));
		pAction->setMenu(subMenu);
		menu.addAction(pAction);
		//menu.addAction("Remove from this group", this, SLOT(onRemoveRequested()));
	}
	else
	{
		//menu.addAction("Edit", this, SLOT(onEditRequested()));
		//menu.addAction("Remove", this, SLOT(onRemoveRequested()));
	}
	menu.exec(QCursor::pos());
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcFormParamGroups::onEditRequested()
{
	/// TODO
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcFormParamGroups::onAddRequested()
{
	bool ok = false;
	QString strGroupName = QInputDialog::getText(this, "New group","Enter group name: ", QLineEdit::Normal, "My_Group", &ok);
	if (ok && !strGroupName.isEmpty())
	{
		if (!m_pDbcTransac->insertGroup(strGroupName).isEmpty())
			loadGroupList();
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcFormParamGroups::onRemoveRequested()
{
	/// TODO
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
void DbcFormParamGroups::onAddParameterToGroup(QAction* pAction)
{
	QString strGroupName = pAction->text();
	
	QList<QTreeWidgetItem*> selectedItems = m_ui->treeWidget->selectedItems();
	if (selectedItems.size() <= 0)
		return;
	
	QTreeWidgetItem* item;
	foreach (item, selectedItems)
	{
		if (item->parent())
		{
			Gate_ParameterDef paramInfo;
			paramInfo.m_strName = item->text(2);
			paramInfo.m_nParameterNumber = item->text(1).toInt();
			QString strTestId = m_pDbcTransac->testId(paramInfo);
			// If the parameter is not already in this group
			if (!m_mapIdpGroup.value(m_pDbcTransac->groupId(strGroupName).toInt())->parameterList().contains(strTestId.toInt()))
				m_pDbcTransac->addTestToGroup(strTestId, strGroupName);
		}
	}
	
	loadGroupList();
}

