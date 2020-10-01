#ifndef DBC_FORM_PARAM_GROUPS_H
#define DBC_FORM_PARAM_GROUPS_H

#include <QFrame>
#include <QMap>

class DbcTransaction;
class QDomDocument;
class QDomElement;
class DbcGroup;
class QAction;

namespace Ui {
    class DbcFormParamGroups;
}

class DbcFormParamGroups : public QFrame
{
	Q_OBJECT

public:
	DbcFormParamGroups(DbcTransaction *pDbcTransac, QWidget *parent = 0);
	~DbcFormParamGroups();
	
	bool loadGroupList();
	bool refreshTreeWidget();
	bool loadData(QDomDocument &domDocument);
	QDomElement data(QDomDocument& domDocument);
	
public slots:
		void onEditRequested();
		void onAddRequested();
		void onRemoveRequested();
	
private slots:
	void onCustomContextMenuRequested();
	void onAddParameterToGroup(QAction* pAction);
	

private:
	Ui::DbcFormParamGroups *m_ui;
	DbcTransaction *m_pDbcTransac;
	QMap<int, DbcGroup*> m_mapIdpGroup;
};

#endif // DBC_FORM_PARAM_GROUPS_H
