#ifndef OPTIONS_CENTER_PROPBROWSER_H
#define OPTIONS_CENTER_PROPBROWSER_H

#include "options_center_widget.h"
#include "libgexpb.h"
#include <QTextEdit>
#include <QDomDocument>

class OptionsCenterPropBrowser : public QWidget
{
	Q_OBJECT

public:
	OptionsCenterPropBrowser(QWidget* p);
	~OptionsCenterPropBrowser();

	/// TO REVIEW
	struct SOption
	{
		QString m_CSLSection;
		QString m_CSLField;
		int		m_nPropertyID;
		//QtProperty *m_property;
		QStringList m_codingValuesList;
		QString m_filter; // for Path/File selector
	};
	/// ! TO REVIEW

	QMap<QString, int > m_CSLToPropertyMap; // key = section_field
	QMap<int , QString> m_PropertyToCslKeyMap; // key = propertyID
	QMap<QString, SOption> m_CSLToSOptionMap; // key = section_field !


	CGexPB*	m_cgpPtrPropBrowser;
	OptionsCenterWidget* m_ocw;

	bool GetOptionForProperty(int nPropertyID, SOption &o)
	{
		QString k=m_CSLToPropertyMap.key(nPropertyID);
		if (k=="")
			return false;
		if (m_CSLToSOptionMap.find(k)==m_CSLToSOptionMap.end())
			return false;
		o=m_CSLToSOptionMap[k];
		return true;
	}

	// Build from gex_options.xml
	QString BuildFromGOXML(QDomElement *);
	// Set option
	bool SetOption(QString section, QString cslname, QString newvalue);
    // Get option
    QVariant GetOption(QString section, QString field);
	// recursive
	bool ParseDomElem(class QDomElement *e, int nPropertyID);



public slots:
	void ValueChanged(int nPropertyID, const QVariant &v) ;
	void SlotExpandAll(bool bExpandAll);		// true to expand all properties

signals:
	void SignalOcPbValueChanged(QString, QString, QString);
    void SignalMessage(const QString&);
};

#endif // OPTIONS_CENTER_PROPBROWSER_H
