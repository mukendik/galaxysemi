#ifndef DBC_FILTER_H
#define DBC_FILTER_H

#include <QFrame>
#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QAbstractButton>

class QSqlTableModel;
class QSortFilterProxyModel;
class ParamSqlTableModel;

namespace Ui {
    class DbcFilter;
}

class DbcFilter : public QFrame {
    Q_OBJECT
public:
	DbcFilter(ParamSqlTableModel *pTableModel, int iIndex = 0, QWidget *parent = 0);
	~DbcFilter();
	
	QString lastError() {return m_lstError.last();}
	void clearErrors() {m_lstError.clear();}
	
	// Getters
	int index() {return m_iIndex;}
	int paramNumber() {return m_iParamNumber;}
	QString paramName() {return m_strParamName;}
	QString filterOperator() {return m_strOperator;}
	QString paramValue() {return m_strParamValue;}
	
	// Setters
	void setIndex(int iIndex);
	void setParamNumber(int iParamNumber);
	void setParamName(QString strParamName);
	void setOperator(QString strFilterOperator);
	void setParamValue(QString strParamValue);
	void setTableModel(ParamSqlTableModel *pTableModel);
	void setAddButtonVisible(bool bIsVisible);
	
	static QString escapesXmlUnavailableChars(const QString& strText);
	static QString restoresXmlUnavailableChars(const QString& strText);
	static void initIndex();
	static int lastIndex() {return m_iLastIndex;}
	
signals:
	void sAdd();
	void sDelete(int iIndex);
	void sChanged();
	
public slots:
	void onAutoDelete();
	void onChanged();
	
private slots:

private:
	void init();
	void initComboOperator();
	void initComboParamNumber();
	void updateLabelIndex();
	void updateComboParamNumber();
	void updateComboOperator();
	void updateEditParamValue();
	void retrieveUIValue();
	
	int m_iIndex;
	int m_iParamNumber;
	QString m_strParamName;
	QString m_strOperator;
	QString m_strParamValue;
	QStringList m_lstError;
	QStringList m_lstOperator;
	static int m_iLastIndex;
	
	ParamSqlTableModel *m_pTableModel;
	QSortFilterProxyModel *m_pProxyModelParamNumber;
	Ui::DbcFilter *m_ui;
	
};


#endif // DBC_FILTER_H
