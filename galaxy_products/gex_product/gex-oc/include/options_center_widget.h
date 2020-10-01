#ifndef OPTIONS_CENTER_WIDGET_H
#define OPTIONS_CENTER_WIDGET_H

#include <QLabel>
#include <QString>
#include <QAction>
#include <QVBoxLayout>
#include <QVariant>
#include <QPushButton>

#include "libgexoc.h"
#include "libgexpb.h"

#if defined(LIBGEXOC_LIBRARY)
#  define LIBGEXOCSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBGEXOCSHARED_EXPORT Q_DECL_IMPORT
#endif

class GexMainwindow;

class LIBGEXOCSHARED_EXPORT
		OptionsCenterWidget : public QWidget
{
	Q_OBJECT
public:
	OptionsCenterWidget(GexMainwindow* gmw, QWidget* p);
	~OptionsCenterWidget();
	void ShowPage(void) { show(); }
	QString BuildFromGOXML(const QString file);
	// Search the corresponding option property with the given cslname and try to set the new value
	bool SetOption(QString section, QString field, QString newvalue);
	QPushButton* m_LoadProfile;
	QPushButton* m_SaveProfile;
	QPushButton* m_ResetButton;	// was button_defaultOption
	QPushButton* m_SaveButton;
	QPushButton* m_CollapseButton;
    QPushButton* m_BuildReport;
    QLabel* mSourceLabel;

public slots:
	void EmitOptionChanged(QString s, QString f, QString nv); //{ emit signalOptionChanged(s, f,nv); };
	void SlotExpandAll(bool bExpandAll);			// true to expand all properties
	void SlotOpenCustomContextMenu(const QPoint & pos);
    QVariant GetOption(QString section, QString field);

signals:
	// this signal will be emited each time an option is changed by the user
	// The option name is the csl name
	void signalOptionChanged(QString section, QString field, QString new_value);
    // catch this signal to know what OC is doing...
    void signalMessage(const QString &m);

private:
	//bool m_dont_emit_change;	// says if the OCW will emit any option change
    QVBoxLayout* m_vlayout;
	QHBoxLayout* m_tophlayout;
	QAction* m_action;

	class OptionsCenterPropBrowser* m_propBrowser;

	GexMainwindow* m_pGexMainWindow;
};

#endif // OPTIONS_CENTER_WIDGET_H
