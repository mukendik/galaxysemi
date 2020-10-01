#ifndef CGEXPB_H
#define CGEXPB_H

#include <QMap>
#include <QWidget>
#include <QVariant>

#include <QtCore/qglobal.h>


class CPropertyIdManager;		// "cpropertyidmanager.h"
class CPropBrowser;
class CGexScriptEngine;
class CGexPublicProperty;		// "cgexpublicproperty.h"

#ifndef GEXPBSHARED_EXPORT
#if defined(GEXPB_LIBRARY)
#  define GEXPBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GEXPBSHARED_EXPORT Q_DECL_IMPORT
#endif
#endif


/**
\file		cgexpb.h
\author		PYC, WT
\version	1.0
\date		07/04/2011
\brief		development interface with property browser gui
\details	built over QtNokiaPropertyBrowser
*/

class GEXPBSHARED_EXPORT CGexPB : public QWidget
{
	Q_OBJECT

public:
	/**
	  \brief		instantiate your PB with this function :

	  \param	cgsePtrScriptEngine		: Global script engine used in all the gex devices
	  \param	parent					: parent widget of the created property browser
	  \param	nIndentation			: Indentation used in property browser
	  \return	The CGexPB pointer created (memory allocated internally)

	  */
  static CGexPB* CreatePropertyBrowser(class GexScriptEngine* cgsePtrScriptEngine, QWidget* parent, int nIndentation = 20);

	/**
	  \brief	Destroy your CGexPB

	  \param	cgpPtr CGexPB pointer to delete
	  \return	true if every thing went well

	  */
	static bool DestroyPropertyBrowser(CGexPB* cgpPtr);

	/**
	  \brief	Add a property to your CGexPB object using given attributes

      \param	cgppNewProperty : property to add;
                if the internal property id isn't valid/free in the internal structure,
                the CGexPB overload it.
	  \return	the internal property id; -1 if problem occured

	  \details
				* Add a property using given attributes
				\n\n
				\b Mandatory attributes are :
					<ul>
					<li> 'type' :
						<ul>
						<li>"Group"
						<li>"Int"
						<li>"Float"
						<li>"Enum"
						<li>"Flag"
						<li>"Bool"
						<li>"String"
						<li>"Color"
						<li>"Font"
						<li>"HTML"
						<li>"Path"
                        <li>"JavaScript" !! currently managed has a String type !!
						</ul>
					<li> 'label' : a simple text that will be display at the left part of the property. exple : "Chart color" (JavaScript)
					<li> codingvalues : for Enum and Flag	(values separated by '|' symbol) \n		!! mandatory for Enum and Flag properties only !!
					<li> 'displayvalues' : for Enum and Flag	(values separated by '|' symbol) \n		!! mandatory for Enum and Flag properties only !!
					</ul>
				\n
				* \b Optional attributes :
				\list
					<ul>
					<li> 'defaultvalue' : will be converted to QVariant (JavaScript) \n \b exemple of use :
						<ul>
							<li>"Group" property : no value so no defaultvalue
							<li>"Int" property : * defaultvalue="32" *
							<li>"Float" property : * defaultvalue="32.32" *
							<li>"Enum" property : * defaultvalue="'defaultcodingvalue'" *
							<li>"Flag" property : * defaultvalue="'firstdefaultcodingvalue|seconddefaultcodingvalue|thirdd...'" *
							<li>"Bool" property : * defaultvalue="true" *
							<li>"String" property : * defaultvalue="'blabla my string'" *
							<li>"Color" property : * defaultvalue="'#FF0000'" *
							<li>"Font" property : * defaultvalue="'Helvetica'" * possible values : 'Helvetica', 'Times', 'Courier', 'OldEnglish' (cf. <a href="http://doc.qt.nokia.com/4.7/qfont.html#StyleHint-enum">qt documentation</a>)
							<li>"HTML" property : * defaultvalue="'blablabla my html code'" *
							<li>"Path" property : * defaultvalue="'blabla my path'" *
							<li>"JavaScript" !! currently managed has a String type !!
						</ul>
					<li> 'regExp' : for String			\n!! not assumed yet !!
					<li> 'hideif' : javascript expression (javascript wich evaluation resulting in boolean)
					<li> 'minimum', 'maximum', 'singleStep' : for Int, Float
					<li> 'icon' : either in resources or external file (String interpreted has icon path)
					<li> 'tooltip' : description displayed to the user (String)
					<li> 'filter' : !! to review : cf. vareditor.cpp, l.65 !!
						<ul>
							<li> used for PATH property;
							<li> string
							<li> if filter value = ""; the path property is a directory path
							<li> else if, the path property is a file path and the filter attribute limits the searched files by extension. (cf. QFileDialog class, http://doc.qt.nokia.com/4.7/qfiledialog.html#setNameFilter)
							\n For example : filter value "All C++ files (*.cpp *.cc *.C *.cxx *.c++)" only allow to choose files with right extension and the dialog combobox displays "All C++ files"
							<li> you can use this attribute with "|" character to concatenate multi-filter;
							<li> Image files (*.png *.xpm *.jpg)|Text files (*.txt)|Any files (*)" create a combo box in dialog widget with the three choices "Image files (*.png *.xpm *.jpg)", "Text files (*.txt)" and "Any files (*)"
						</ul>
					<li> 'enumicons' : for Enum type properties (values separated by '|' symbol) each string value interpreted has icon path (resource or external file)
					</ul>

	  returns a Property ID, -1 on fail; use this property ID to handle your property, so keep it in memory !!
	  \n
	  After adding properties or changing some attributes of your properties, you have to call RefreshPB() to update the gui.

	  */
	int AddProperty(CGexPublicProperty cgppNewProperty);	
	int AddProperty(QMap< QString, QString > attributes, int nParent_id=-1);		// the id of the parent property if any : MUST be a group type property

	/**
	  \brief	Set the new value to the given property

	  \param	nPropertyId : id of the property that you want to change the value
	  \param	newvalue : value to set to the given property

	  \return	true if the value has been set correctly

	  */
	bool SetValue(int nPropertyId, QVariant newvalue);

	/**
	  \brief	Set the attribute value for the given property

	  \details	Change the attribute value if already exists for the given property. Otherwise, create the attribute in the property attribute list
	  \n\n
	  You can use GetProperty() and change directly attributes

	  \param	strAttributeLabel : label of the attribute to change (label, codingvalue, ...) \n !! the 'type' attribute can't be changed once it has been set !!
	  \param	strAttributeValue : new value of the selected attribute
	  \param	nPropertyId : property that you want to change an attribute

	  \return	true if the attribute value has been set correctly
	  */
	bool SetAttribute(QString strAttributeLabel, QString strAttributeValue, int nPropertyId);
	bool SetAttribute(const QMap<QString, QString >, const int nPropertyId);

	// bool SetParentIdProperty(const int nPropertyId, const int nNewParentIdProperty);

	/**
	  \brief	Method used to work directly with property : CGexPublicProperty::SetAttribute(), ... Don't forget the RefreshPB() call if you change attributes !

	  \param	nPropertyId : id of the property to return

	  \return	QExplicitlySharedDataPointer<CGexPublicProperty> (work has a pointer without memory problems) cibling the property associated with the given id.
	  */
	QExplicitlySharedDataPointer<CGexPublicProperty>	GetProperty(const int nPropertyId);

	/**
	  \brief	Get the current value of the property

	  \param	nPropertyId : id of the property that you want the value

	  \return	QVariant with the property value, invalid QVariant if encounter a problem
	  */
	QVariant	GetCurrentValue(int nPropertyId) const;

	//////////////////////
	// global methods
	//////////////////////

	/**
	  \brief	Refresh all the property browser gui interpreting all assumed property browser attributes

	  \return	true if everything went well
	  */
	bool RefreshPB();


	/**
	  \brief	Reset the value of all properties using defaultvalue (javascript) attribute if defined

	  \return	true if everything went well
	  */
	bool Reset();


    // CGexScriptEngine* is mandatory
  CGexPB(QWidget* parent, class GexScriptEngine*, int nIndentation=20);

    //
    ~CGexPB();
    //

public slots:
	/**
	  \brief	Slot used to collapse / expand all properties in the property browser

	  \param	bIsExpanded : true to expand all; false to collapse all
	  */
	void SlotSetExpanded(const bool bIsExpanded);

private slots:
	void SlotPropertyValueChanged(int id,const QVariant& new_value);

signals:
	/**
	  \brief	Signal used to tell the user a property value changed

	  \param	nPropertyId : id of the property that the value changed
	  \param	new_value : new value of the property

	  */
	void SignalGPbPropertyValueChanged(int nPropertyId,const QVariant& new_value);

    /**
      \brief    Signal emited in order to send a message (working, glanding, ...)
      */
    void SignalMessage(const QString&);

private :


	///////////////////////////
	// class members
	///////////////////////////
	CPropBrowser*			m_cpbPtrPropertyBrowser;
	static int				n_instances;
    GexScriptEngine*		m_cgsePtrScriptEngine;


	/////////////////////////////
	// internal methods
	/////////////////////////////
	void Connections();
};

#endif // CGEXPB_H
