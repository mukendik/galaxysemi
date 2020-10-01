///////////////////////////////////////////////////////////
// Examinator Monitoring: RDB Yield Check dialog box
///////////////////////////////////////////////////////////
#ifndef GEXMO_RDB_YIELDCHECK_H
#define GEXMO_RDB_YIELDCHECK_H

#include <QDate>

#include "mo_yieldcheck.h"


class QTableComboBox: public QComboBox
{
	Q_OBJECT
		
public:
	QTableComboBox( QWidget* parent = 0, bool modal = FALSE, Qt::WFlags fl = 0 ){};

	// Holds Row,Col location in table.
	QTableWidgetItem *ptTableItem;

public slots:
	void OnComboChange(int);

signals:
	void comboValueChange(QTableComboBox*);

};

class QTablePushButton: public QPushButton
{
	Q_OBJECT
		
public:
	QTablePushButton( QWidget* parent = 0, bool modal = FALSE, Qt::WFlags fl = 0 ){};

	// Holds Row,Col location in table.
	QTableWidgetItem *ptTableItem;

public slots:
	void OnButtonPressed();

signals:
	void buttonPressed(QTablePushButton*);

};

#endif
