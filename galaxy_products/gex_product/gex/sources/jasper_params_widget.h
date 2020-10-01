#ifndef PARAMS_WIDGET_H
#define PARAMS_WIDGET_H

#define MAX_NUM_RESOURCES 256
#define MAX_STRING_SIZE 128

#include <QWidget>
#include <QPushButton>
//#include "libjasper.h"
#include <gqtl_log.h>

class ParamsWidget : public QWidget
{
	Q_OBJECT

	public:
		static ParamsWidget* GetParamsWidgetForReport(QString report_uri, QString report_name);
		static ParamsWidget* CreateParamsWidgetForReport(QString& report_uri, QString report_name,
			char o[MAX_NUM_RESOURCES][6][MAX_STRING_SIZE], int nParam);
		QString m_report_uri;
		bool GetCurrentParams(QVector< QPair <QString, QString> > &p); // write current params in p
		bool SaveCurrentParamsToFile();
		bool LoadParamsFromFile();
		QPushButton *m_ok_pb;

	private:
		ParamsWidget(QWidget* p, char[MAX_NUM_RESOURCES][6][MAX_STRING_SIZE], int nParams, QString& report_uri);
		~ParamsWidget() { GSLOG(7, " "); }
		QString m_report_label;

		struct SParam
		{	QString m_label;
			QString m_uri;
			QString m_IC_type;
			QString m_DataType;
			QString m_description;
			QWidget *m_value_widget;
		};
		QVector<SParam> m_params;
		static QVector<ParamsWidget*> s_ParamsWidgets;
	public slots:
		void ReportGenerated(const QString report_uri, const QString output);
};

#endif // PARAMS_WIDGET_H
