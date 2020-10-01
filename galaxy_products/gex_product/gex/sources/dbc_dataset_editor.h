#ifndef DBC_PARAMETERS_H
#define DBC_PARAMETERS_H

#include <QDialog>
#include <QTreeWidget>
#include <QMap>
#include <QByteArray>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDomDocument>
#include "dbc_transaction.h"



class DbcTransaction;
class Gate_ParameterDef;
class DbcFormParamResults;
class DbcFormParamGroups;

#define DBC_PARAMETERS_COLUMN_TESTID	0
#define DBC_PARAMETERS_COLUMN_NUMBER	2

#define DBC_TAB_PARAMFILTER_PARAM		0
#define DBC_TAB_PARAMFILTER_FILTER		1
#define DBC_TAB_PARAMFILTER_GROUP		2


class DbcFilter;
class DbcParameter;
class ParamSqlTableModel;

namespace Ui {
	class DbcDatasetEditor;
}

class DbcDatasetEditor : public QDialog {
	Q_OBJECT
public:
	DbcDatasetEditor(DbcTransaction& dbTransac, QWidget *parent = 0);
	~DbcDatasetEditor();
	void setDbcTransac(DbcTransaction& dbTransac);
	void loadParamInfo();
	bool loadXmlFilterFile(QString &strFilePath);
	bool loadFilterList(QDomDocument &document);
	bool saveXmlFilterFile(QString &strFilePath);
	QDomDocument saveFilterList();
	void initGui();
	void initPTestTableModel();
	void initSTestTableModel();
	void initParamTableModel();
	void initLineEditFilterLogicOnSteps();
	void editParameter(Gate_ParameterDef* paramInfo);
	void addParameter(DbcParameter* newParameter);
	void removeParameter(Gate_ParameterDef* paramInfo);
	void addFilter(DbcFilter* newFilter);
	QString filterLogicOnSteps() {return m_strFilterLogicOnSteps;}
	QString xmlFilterFile() {return m_strFiltersXmlFilePath;}
	void setFilterLogicOnSteps(QString strFilterLogic);
	QString filterString(int iIndex);

signals:
	void sChangeDetected();
	
public slots:
	void saveXmlUpdate();
	void onFiltersChanged();
	void onDeleteFilter(int iIndex);
	void resetPTestTableModel();
	void resetSTestTableModel();
	void resetParamTableModel();
	void onDataChanged();

private slots:
	void onCustomContextMenuRequested();
	void onParamInfoSelectionChanged();
	void onEditParamFilterRequested();
	void onRemoveParamFilterRequested();
	void onAddParamFilterRequested();
	void onEditParameterRequested();
	void onRemoveParameterRequested();
	void onAddParameterRequested();
	void onAddFilterRequested();
	void onSelectFileRequested();
	void onXmlFilterRequested();
	void onXmlParameterRequested();
	void onSaveRequested();
	void onSaveFiltersRequested();
	void onApplyFiltersRequested();
	void applyFiltersOnSteps();
	void onReportEditorRequested();
 
private:
	Ui::DbcDatasetEditor *m_ui;
	DbcTransaction		*m_pDbcTransac;
	QSqlTableModel		*m_pTableModelPtest;
	QSqlTableModel		*m_pTableModelStest;
	ParamSqlTableModel	*m_pTableModelParam;
	QString				m_strFiltersXmlFilePath;
	QString				m_strFilterLogicOnSteps;
	QString				m_strFilterQueryOnSteps;
	QMap<int, DbcFormParamResults*>	m_selectedParam;
	QList<DbcFilter*>	m_lstDbcFilters;
	QDomDocument		m_domDocXmlFilters;
	DbcFormParamGroups*	m_pFormParamGroups;
};


class ParamSqlTableModel: public QSqlTableModel
{
	Q_OBJECT
public:

	ParamSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase()):QSqlTableModel(parent, db)
	{

	}
	virtual ~ParamSqlTableModel()
	{

	}

	QVariant data( const QModelIndex & index, int role = Qt::DisplayRole) const
	{
		if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
			return QVariant();

		if ((index.column() == fieldIndex(QString(DBC_TEST_F_NAME))) && (role == Qt::DisplayRole))
		{
			return QVariant(QString(QSqlTableModel::data(QSqlTableModel::index(index.row(), fieldIndex(QString(DBC_TEST_F_NAME))), role).toString() + "-" + 
							QSqlTableModel::data(QSqlTableModel::index(index.row(), fieldIndex(QString(DBC_TEST_F_NUMBER))), role).toString()));
		}

		return QSqlTableModel::data(index, role);
	}
};


#endif // DBC_PARAMETERS_H
