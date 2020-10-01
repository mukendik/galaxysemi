#ifndef DBC_PARAMETER_H
#define DBC_PARAMETER_H

#include <QDialog>

class DbcFilterSet;
class ParamSqlTableModel;
class DbcTransaction;
class QDomElement;
class QDomDocument;

namespace Ui {
    class DbcParameter;
}

class DbcParameter : public QDialog
{
    Q_OBJECT

public:
	enum DbcParameterMode
	{
		eCreate,			// Use to create new parameter
		eUpdate				// Use to update existing parameter
	};
    DbcParameter(DbcTransaction *pDbcTransac,ParamSqlTableModel *pTableModel, DbcParameterMode eMode = eCreate, QWidget *parent = 0);
    ~DbcParameter();
	bool saveToFile(const QString& strFilePath);
	bool loadXml(const QString& strFilePath);
	bool loadData(QDomDocument &domDocument);
	QDomElement data(QDomDocument& domDocument);
	
	void setAdvanced(bool bIsAdvanced);
	void setParamNumber(int iParamNumber);
	void setLowL(double lfLowL);
	void setHighL(double lfHighL);
	void setParamName(QString strParamName);
	void setUnit(QString strUnit);
	void setType(QString strType);
	void setValue(QString strValue);
	void setStepLit(QString strStepList);

public slots:
	void onAddFilterSetRequested();
	void onDeleteFilterSet(int iIndex);
	void onSave();
	void onChanged();
	void onApplyRequested();
	bool insertParameter();
	bool updateParameter();
	bool onLoadParameter();
	void onCheckAdvanced(bool bIsChecked);
	
signals:
	void sChanged();
	
private:
	void addFilterSet(DbcFilterSet* newFilterSet);
	void init();
	void initComboParameterType();
	void resetView();
	void resetFilterSetView();
	bool buildInsertParameterPTestResultsQuery(const QString& strTestInfoId);
	bool buildUpdateParameterPTestResultsQuery(const QString& strTestInfoId);
	bool buildInsertParameterSTestResultsQuery(const QString& strTestInfoId);
	bool buildUpdateParameterSTestResultsQuery(const QString& strTestInfoId);
	QString buildSimpleStepListQuery();
	QString selectQueryWithoutFormula(const QString& strTestInfoId, const QString& strValue, const QString& strFilterSetQuery = QString());
	QString selectQueryWithFormula(const QString& strTestInfoId, const QString& strValue,const QString& strFilterSetQuery = QString());
	
	Ui::DbcParameter *m_ui;
	bool m_bIsAdvanced;
	int m_iLastFilterSetIndex;
	int m_iParamNumber;
	double m_lfLowL;
	double m_lfHigL;
	QString m_strParamName;
	QString m_strUnit;
	QString m_strType;
	QString m_strValue;
	QStringList m_lstParameterType;
	QList<DbcFilterSet*> m_lstFilterSet;
	ParamSqlTableModel *m_pTableModel;
	DbcTransaction *m_pDbcTransac;
	DbcParameterMode m_eMode;
};

#endif // DBC_PARAMETER_H
