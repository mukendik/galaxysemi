#ifndef REPORTS_CENTER_MULTIFIELDS_H
#define REPORTS_CENTER_MULTIFIELDS_H

#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QGroupBox>
#include <QMap>
#include <QVariant>
#include <QHBoxLayout>
#include "reports_center_params_widget.h"
#include "reports_center_params_dataset.h"
#include "reports_center_field.h"
#include <stdio.h>

class CReportsCenterMultiFields : public QGroupBox
{
	CReportsCenterDataset* m_pDataset;
	QString m_label;
	QString m_role;
	QVBoxLayout m_layout;
	QCheckBox* m_full_range;
//	bool m_filterable;
	static int n_instances;
	QMap<QString, QString> m_Attributes;
	QList< class CReportsCenterField* > m_Fields;
public:
	// The parent must be the dataset widget
	CReportsCenterMultiFields(QWidget* p, CReportsCenterDataset* ds, QMap<QString,QString> atts);
	~CReportsCenterMultiFields();
	// Get the dataset owner of this MF. Each MF should have a dataset owner.
	CReportsCenterDataset* GetDataset() { return m_pDataset; }
	// check if a mandatory field has not been yet choosen by the user
	bool HasAtLeastOneMandatoryFieldUnknown();
	// add/impose a field with attributes could be field_readonly, field_mandatory, orderdir,...
	bool AddField( QMap<QString,QString> atts );
	// retrieve the selected filters (if any) in the GexDbPlugin_Filter style : "field=value"
	QStringList GetFiltersStrings();
	// Get position of the given field
	//int GetPosition(CReportsCenterField* f) { return m_Fields.indexOf(f); }
	bool IsLastField(CReportsCenterField* f) { return (m_Fields.indexOf(f)==(m_Fields.count()-1));  }
	bool IsAutoRepeat() { return (m_Attributes["autorepeat"]=="true"); }

	// ExportToDom
	bool ExportToDom(QDomDocument&, QDomElement &de);
	//
	QString ExportToCsl(FILE*);

	//
	bool ReplaceBinCatInComboBox(QString bincat);
	//
	static int GetNumberOfInstances() { return n_instances; }
	//
	QString GetAttribute(QString a) { return m_Attributes[a]; }

};


#endif // REPORTS_CENTER_MULTIFIELDS_H
