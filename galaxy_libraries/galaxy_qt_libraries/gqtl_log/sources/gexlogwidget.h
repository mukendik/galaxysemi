#ifndef GEXLOGWIDGET_H
#define GEXLOGWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "coutput.h"

//extern QMap<QString, int>* s_LogLevels;

class CGexLogWidget : public QWidget
{
	Q_OBJECT

public:
	CGexLogWidget(QWidget* p);//:QWidget(p), m_currentRow(0);
	~CGexLogWidget();
	static CGexLogWidget* s_pWidget;

	// Set the level for the given module
	bool setLogLevel(QString mn, int l);

	//
	bool addLogInMenu(QString mn, int l);

public slots:
	void onLogLevelActionTriggered(QAction*);

	void onShow()
	{
		qDebug("onShow");
	}

	void onCellDoubleClicked(int x, int y);

private:
	QTableWidget m_tw;
	QPushButton m_llpb;
	int m_currentRow;
	QTableWidgetItem* GetItem(int x, int y);

public:
	//
	QString AppendMessage(const SMessage &sm);
	//
	//QString AddLogLevelMenu(const QString &modulename, int currentlevel);
};

#endif // GEXLOGWIDGET_H
