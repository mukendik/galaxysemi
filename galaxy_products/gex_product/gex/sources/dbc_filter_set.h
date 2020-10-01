#ifndef DBC_FILTER_SET_H
#define DBC_FILTER_SET_H

#include <QFrame>


class DbcFilter;
class QSortFilterProxyModel;
class ParamSqlTableModel;
class QDomElement;
class QDomDocument;
class DbcTransaction;
class QSize;

namespace Ui {
    class DbcFilterSet;
}

class DbcFilterSet : public QFrame
{
    Q_OBJECT

public:
	DbcFilterSet(DbcTransaction *pDbcTransac, ParamSqlTableModel *pTableModel, int iIndex = 0, QWidget *parent = 0);
	~DbcFilterSet();
	
	void resetView();
	
	QString filterLogic()const		{return m_strFilterLogic;}
	QString value()const	{return m_strValue;}
	QString paramName()const		{return m_strParamName;}
	QString stepList()const			{return m_strStepList;}
	int		paramNumber()const		{return m_iParamNumber;}
	int		index()const			{return m_iIndex;}
	bool	isAdvanced()const		{return m_bIsAdvanced;}
	bool	isValueBasedOnFormula();
	
	void setFilterLogic(QString strFilterLogic);
	void setValue(QString strValue);
	void setParamName(QString strParamName);
	void setParamNumber(int iParamNumber);
	void setIndex(int iIndex);
	void setValueVisible(bool bIsVisible);
	void setFrameParameterVisible(bool bIsVisible);
	void setStepList(QString strStepList);
	void setAdvanced(bool bIsAdvanced);
	void setAddButtonVisible(bool bIsVisible);
	
	void loadData(const QDomElement& domElement);
	QDomElement data(QDomDocument& domDocument);
	
	QString buildSimpleStepListQuery();
	QString buildFilterQuery(int iIndex);
	QString buildFilterLogicQuery();

public slots:
	void onDeleteFilter(int iIndex);
	void onChanged();
	void onAutoDelete();
	void onAddFilter();
	void onCheckBoxAdvanced(bool);

signals:
	void sChanged();
	void sAdd();
	void sDelete(int);
	
private:
	
	void init();
	void addFilter(DbcFilter* newFilter);
	void resetComboParamNumber();
	void initComboParamNumber();
	void resetFilterListView();
	void retrieveUIValue();
	
    Ui::DbcFilterSet	*m_ui;
	int					m_iIndex;
	int					m_iParamNumber;
	int					m_iLastFilterIndex;
	bool				m_bIsAdvanced;
	QString				m_strParamName;
	QString				m_strTitle;
	QList<DbcFilter*>	m_lstDbcFilters;
	QString				m_strFilterLogic;
	QString				m_strValue;
	QString				m_strStepList;
	ParamSqlTableModel *m_pTableModelParam;
	QSortFilterProxyModel *m_pProxyModelParamNumber;
	DbcTransaction		*m_pDbcTransac;
};

#endif // DBC_FILTER_SET_H
