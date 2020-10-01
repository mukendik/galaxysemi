#ifndef REPORTS_CENTER_FIELD_H
#define REPORTS_CENTER_FIELD_H

#include "reports_center_widget.h"
#include <QMap>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>

class CReportsCenterField : public QWidget
{
	Q_OBJECT

	QMap<QString,QString> m_atts;
	QHBoxLayout m_layout;
	//QSpacerItem m_spacer;
    QComboBox m_field;
    bool readonly_field;
	bool m_filterable;
    QComboBox m_value;
    bool readonly_value;
	QPushButton m_orderdir; //QLabel
	bool m_multivalue;
	QLabel m_indent;
    QLabel m_operator;
    bool readonly_operator;
    QToolButton m_pickbutton;
    bool pick_enabled;
	// hide all
	void Hide();
	// show what should be shown
	void Show();
	static int n_instances;
public slots:
	void SlotFieldChanged(QString);

public:
	// the parent is a CReportsCenterMultiFields
	CReportsCenterField(QWidget* p, QMap<QString, QString> atts, int i);
	~CReportsCenterField();
	// return the filter (if any) in the style "field=value"
	QString GetFilterString();
	// export to xml/dom
	bool ExportToDom(QDomDocument& doc, QDomElement &de, int index);
	//
	QString ExportToCsl(FILE*);
	// The value is Mandatory ?
	bool IsValueMandatory() { if (m_atts["value_mandatory"]=="true") return true; else return false; }
	// IsValueSelected
	bool IsValueSelected() { if (m_value.currentText().isEmpty()) return false; else return true; }
	// replace 'H' with 'S' or contrary
	bool ReplaceBinCatInComboBox(QString bincat);
	//
	static int GetNumberOfInstances() { return n_instances; }
public slots:
	void	OnPickFilter();	// On clicked a filter small tool button
	void	OnOrderClicked()
	{ if (m_orderdir.text()==QChar(0x2191)) m_orderdir.setText(QChar(0x2193)); else m_orderdir.setText(QChar(0x2191));
	}	// the order arrows

	//signals :
	//QString MandatoryValueSelected();
};


#endif // REPORTS_CENTER_FIELD_H
