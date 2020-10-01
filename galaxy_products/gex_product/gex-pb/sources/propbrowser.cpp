#include "gex_scriptengine.h"

#include <QTreeWidget>
#include <QApplication>

#include "libgexpb.h"
#include "propbrowser.h"
#include <gqtl_log.h>

CPropBrowser::CPropBrowser(QWidget* p, GexScriptEngine* cgsePtrScriptEngine) : QtTreePropertyBrowser(p)
{
	m_cgsePtrScriptEngine = cgsePtrScriptEngine;

	m_cpimPtrPropertyIdManager = new CPropertyIdManager();

	m_cvmPtrVariantManager = new CVariantManager(m_cpimPtrPropertyIdManager, this);
	m_cvefPtrVariantFactory = new CVariantEditorFactory(m_cpimPtrPropertyIdManager, this);

    //GSLOG(7,(char*)"\tconfiguring propbrowser...");
	setFactoryForManager((QtVariantPropertyManager*)m_cvmPtrVariantManager, m_cvefPtrVariantFactory);

	BuildFunctionPointerMaps();
	Connections();
}

CPropBrowser::~CPropBrowser()
{
    /*if (m_cvmPtrVariantManager)
		delete m_cvmPtrVariantManager;

	if (m_cvefPtrVariantFactory)
        delete m_cvefPtrVariantFactory;*/

	if(m_cpimPtrPropertyIdManager)
        delete m_cpimPtrPropertyIdManager;
}




///////////////////////////////////
// global methods
///////////////////////////////////
/// TO REVIEW
bool CPropBrowser::Clear()
{
	/// TODO
	return false;
}

bool CPropBrowser::Refresh()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Refreshing Property Browser...");

	QList<int> qlIDList = m_cpimPtrPropertyIdManager->GetPropertyIDList();
	QListIterator<int> qliIDIterator(qlIDList);
	int nPropertyID;
	QVariant qvPropertyValue;

	bool bPropertyRefreshRslt = false;

	while(qliIDIterator.hasNext())
	{
        nPropertyID = qliIDIterator.next();
        bPropertyRefreshRslt = UpdatePropertyAttributes(nPropertyID);

		if(!bPropertyRefreshRslt)
		{
            GSLOG(SYSLOG_SEV_WARNING, "Can't refresh property attributes !");
			return false;
		}
	}

	// everything went well !
	return true;
}


bool CPropBrowser::Reset()
{
	QList<int> qlIdList = m_cpimPtrPropertyIdManager->GetPropertyIDList();
	QListIterator<int> qliIdIterator(qlIdList);

	bool bResetRslt=true;

	while(qliIdIterator.hasNext())
	{bResetRslt &= SetPropertyDefaultValue(qliIdIterator.next());}

	return bResetRslt;
}

void CPropBrowser::SetExpandedAllProperties(const bool bIsExpanded)
{
	QList<int> qlIDList = m_cpimPtrPropertyIdManager->GetPropertyIDList();
	QListIterator<int> qliIDIterator(qlIDList);

	QtVariantProperty* qvpPtrVariantProperty  = NULL;
	QList<QtBrowserItem* > qlBroserItemList;



	while(qliIDIterator.hasNext())		// for each referenced property
	{
		qvpPtrVariantProperty = m_cpimPtrPropertyIdManager->GetPrivateProperty(qliIDIterator.next());
		if(!qvpPtrVariantProperty)
		{
			GEX_ASSERT(false);
			return;
		}

		qlBroserItemList = items(qvpPtrVariantProperty);
		QListIterator<QtBrowserItem* > qliBrowserItemIterator(qlBroserItemList);
		while(qliBrowserItemIterator.hasNext())		// for each browser item (// treeWidgetItem) setExpanded
			setExpanded(qliBrowserItemIterator.next(), bIsExpanded);

	}

	return;
}


/////////////////////////////////
// accessors
/////////////////////////////////
bool	CPropBrowser::SetPropertyValue(int nPropertyId,const QVariant& qvNewValue)
{
	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyId, true))
		return false;

	QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty = GetProperty(nPropertyId);
	if(!cgppShPtrGexProperty)
	{
        GSLOG(SYSLOG_SEV_ERROR, "Invalid gex public property found !");
		return false;
	}

	QtVariantProperty* qvpPtrProperty = m_cpimPtrPropertyIdManager->GetPrivateProperty(nPropertyId);
	if(!qvpPtrProperty)
	{
        GSLOG(SYSLOG_SEV_ERROR, "Invalid private property found !");
		GEX_ASSERT(false);
		return false;
	}

	// get the specific function pointer
	QString strTypeAttribute = cgppShPtrGexProperty->GetAttributeValue(QString("type"));
	bool(CPropBrowser::*fPtrSpecificSetPropertyFunction)(QExplicitlySharedDataPointer<CGexPublicProperty>, QtVariantProperty* , const QVariant) = m_qmSpecificSetValue.value(strTypeAttribute, NULL);
	if(!fPtrSpecificSetPropertyFunction)
	{
        GSLOG(SYSLOG_SEV_ERROR, QString("NULL pointer found ! Can't set property value, '%1', '%2'")
                 .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label")).toLatin1().constData())
                 .arg(qvNewValue.toString()).toLatin1().constData());
		GEX_ASSERT(false);
		return false;
	}

	blockSignals(true);
	bool bSetPropertyRslt = (this->*fPtrSpecificSetPropertyFunction)(cgppShPtrGexProperty, qvpPtrProperty, qvNewValue);
	blockSignals(false);


	if(!bSetPropertyRslt)
	{
        GSLOG(SYSLOG_SEV_ERROR, (QString("Set property value failed, '%1', '%2'")
                        .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label")))
                        .arg(qvNewValue.toString())).toLatin1().constData());
		return false;
	}

	// everything went well !!
	return true;
}


QExplicitlySharedDataPointer<CGexPublicProperty>	CPropBrowser::GetProperty(const int nPropertyId)
{
	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyId, true))
		return QExplicitlySharedDataPointer<CGexPublicProperty>();
	else
		return m_cpimPtrPropertyIdManager->GetPublicProperty(nPropertyId);
}


QVariant	CPropBrowser::GetCurrentValue(const int nPropertyId) const
{
	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyId, false))
	{
        GSLOG(5, "Invalid property id used to get property current value !");
			// Can occure with signal/slot tools and internal property browser device
			// see "void QtVariantPropertyManager::setValue(QtProperty *property, const QVariant &val)" in qtvariantproperty.cpp l. 1573
		return QVariant();
	}

	QtVariantProperty* qvpPtrProperty = m_cpimPtrPropertyIdManager->GetPrivateProperty(nPropertyId);
	if(!qvpPtrProperty)
		return QVariant();		/// TO DO : write a debug message if this case occures

	QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty = m_cpimPtrPropertyIdManager->GetPublicProperty(nPropertyId);
	if(!cgppShPtrGexProperty)
	{
        GSLOG(SYSLOG_SEV_WARNING, "Invalid gex public property !!");
		return QVariant();
	}


	// get the specific function pointer
	QString strTypeAttribute = cgppShPtrGexProperty->GetAttributeValue(QString("type"));
	QVariant(CPropBrowser::*fPtrSpecificGetPropertyFunction)(const QExplicitlySharedDataPointer<CGexPublicProperty> ,const QtVariantProperty* )const = m_qmSpecificGetValue.value(strTypeAttribute, NULL);
	if(!fPtrSpecificGetPropertyFunction)
	{
        //GSLOG...
		//Q_ASSERT(false);		/// TO DO error msg
		return QVariant();
	}


	QVariant qvPropertyValue = (this->*fPtrSpecificGetPropertyFunction)(cgppShPtrGexProperty, qvpPtrProperty);
	if(!qvPropertyValue.isValid())
	{
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid get property value return, '%1'")
              .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData());
		GEX_ASSERT(false);
		return QVariant();
	}

	// everything went well !!
	return qvPropertyValue;
}


bool	CPropBrowser::SetPropertyDefaultValue(int nPropertyId)
{
	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyId, true))
		return false;		// the called id is wrong !

	QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty = m_cpimPtrPropertyIdManager->GetPublicProperty(nPropertyId);
	if(!cgppShPtrGexProperty)
	{
        GSLOG(SYSLOG_SEV_WARNING, "Invalid gex public property !!");
		return false;		// invalid behaviour, the called property should exist
	}

	QString strDefaultValue = cgppShPtrGexProperty->GetAttributeValue(QString("defaultvalue"));
	if(strDefaultValue.isEmpty())
	{
        GSLOG(5, QString("Try to set default value to property '%1' without defaultvalue attribute defined.")
                 .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData());
		return true;		// defaultvalue attribute isn't mandatory
	}

	QScriptValue qsvScriptValue = m_cgsePtrScriptEngine->evaluate(strDefaultValue);
	//if ( (qsvScriptValue.isError()) || (!qsvScriptValue.isString() ) )
	/// TO REVIEW : validity checks
	if ( (qsvScriptValue.isError())) // || (!qsvScriptValue.isVariant()) )
	{
        GSLOG(SYSLOG_SEV_WARNING,QString("While evaluating javascript expression : '%1'  '%2'; javascript error : '%3'")
                 .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label")).toLatin1().constData())
                 .arg("defaultvalue")
                 .arg(m_cgsePtrScriptEngine->uncaughtException().toString()).toLatin1().data() );
		return false;
	}

	QVariant qvPropertyDefaultValue = qsvScriptValue.toVariant();
	return SetPropertyValue(nPropertyId, qvPropertyDefaultValue);
}


int CPropBrowser::AddProperty(CGexPublicProperty& cgppProperty)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Adding property (id=%1)...")
          .arg(cgppProperty.GetPropertyId()).toLatin1().constData());

	if(!IsPropertyBrowserValidProperty(cgppProperty))
	{
        GSLOG(SYSLOG_SEV_WARNING, "Try to add invalid property to the browser !");
		return -1;
	}

	int nParentPropertyID = cgppProperty.GetParentPropertyId();
	if(	(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nParentPropertyID)) && (nParentPropertyID!=-1) )
	{
        GSLOG(SYSLOG_SEV_ERROR, QString("Try to set a property (%1) with invalid parent !")
                 .arg(cgppProperty.GetAttributeValue(QString("label"))).toLatin1().constData());
		return -1;
	}

	QtVariantProperty *qvpPtrVariantProperty=NULL;
	QString strPropertyType = cgppProperty.GetAttributeValue(QString("type"));
	QString strPropertyLabel =  cgppProperty.GetAttributeValue(QString("label"));

	/// TO REVIEW : should be done more simply
	if ( strPropertyType == QString("Group") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), strPropertyLabel);
	else if ( strPropertyType == QString("Int") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QVariant::Int, strPropertyLabel);
	else if ( strPropertyType == QString("Float") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QVariant::Double, strPropertyLabel);
	else if ( strPropertyType == QString("Enum") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), strPropertyLabel);
	else if ( strPropertyType == QString("Flag") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QtVariantPropertyManager::flagTypeId(), strPropertyLabel);
	else if ( strPropertyType == QString("Bool") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QVariant::Bool, strPropertyLabel);
	else if ( strPropertyType == QString("String") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QVariant::String, strPropertyLabel);
	else if ( strPropertyType == QString("Color") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QVariant::Color, strPropertyLabel);
	else if ( strPropertyType == QString("Font") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(QVariant::Font, strPropertyLabel);
	else if ( strPropertyType == QString("HTML") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(HTML_TYPE, strPropertyLabel);
	else if ( strPropertyType == QString("Path") )
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(PATH_TYPE, strPropertyLabel);
	else if ( strPropertyType == QString("JavaScript") )		/// TO DEFINE
		qvpPtrVariantProperty = m_cvmPtrVariantManager->addProperty(JS_TYPE, strPropertyLabel);
	else
	{
		QString strErrorMsg = QString("Unknown property type : ") + strPropertyType;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
		return -1;
	}

	if(!qvpPtrVariantProperty)
	{
        GSLOG(SYSLOG_SEV_ERROR, "Property wasn't added correctly !" );
		return -1;
	}

	int nPropertyID = m_cpimPtrPropertyIdManager->CreateProperty(cgppProperty, qvpPtrVariantProperty);

	// Property tree building
	if(nParentPropertyID == -1)
	{
		QtTreePropertyBrowser::addProperty(qvpPtrVariantProperty);
	}
	else
	{
		QtVariantProperty* qvpPtrParentProperty =  m_cpimPtrPropertyIdManager->GetPrivateProperty(nParentPropertyID);
		qvpPtrParentProperty->addSubProperty(qvpPtrVariantProperty);
	}


	/// debug experience, to remove
//	if( (strPropertyType == QString("String")) || (strPropertyType == QString("Flag")) )
//		return nPropertyID;

	// refresh property display when it is inserted
    if(!UpdatePropertyAttributes(nPropertyID))
    {
        GEX_ASSERT(false);
    }

    // Set property default value if defined
    QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrCreatedProperty = GetProperty(nPropertyID);
    if( !(cgppShPtrCreatedProperty->GetAttributeValue(QString("defaultvalue"))).isEmpty() )
    {
            bool bSetDefaultValueRslt = SetPropertyDefaultValue(nPropertyID);
            if(!bSetDefaultValueRslt)
            {
                    GSLOG(SYSLOG_SEV_ERROR, "Can't set the default value after adding a property");
            }
    }

	return nPropertyID;
}

int CPropBrowser::AddProperty(QMap< QString, QString > qmAttributes, int nParentId /* =-1 */)
{
    QCoreApplication::processEvents();

    CGexPublicProperty cgppIntermediateProperty(qmAttributes, nParentId);
    return AddProperty(cgppIntermediateProperty);
}

/// TO REVIEW
bool CPropBrowser::RemoveProperty(int /*nPropertyId*/)
{
	/// TODO
	return false;
}


//////////////////////////////////
// public slots
//////////////////////////////////
void CPropBrowser::SlotPbPropertyValueChanged(int nPropertyId, const QVariant& qvPropertyValue)
{
	/// TO REVIEW : SetPropertyValue rslt ?

	bool bSetPropertyValueRslt = SetPropertyValue(nPropertyId, qvPropertyValue);
	if(!bSetPropertyValueRslt)
	{
        GSLOG(SYSLOG_SEV_ERROR, "Set property value rejected");
		GEX_ASSERT(false);
	}

	emit SignalPbPropertyValueChanged(nPropertyId, qvPropertyValue);
}

///////////////////////////////////
// private slots
///////////////////////////////////
void CPropBrowser::SlotPbPropertyValueChanged(QtProperty* property,
											  const QVariant& /*value*/)
{
	int nPropertyID = m_cpimPtrPropertyIdManager->GetPrivatePropertyID(property);

	if(nPropertyID != -1)
	{
		QVariant qvNewPropertyValue = GetCurrentValue(nPropertyID);		// convert flag / enum values !
		emit SignalPbPropertyValueChanged(nPropertyID,qvNewPropertyValue);
	}
}



////////////////////////////////
// internal methods
////////////////////////////////
void CPropBrowser::Connections()
{
	connect(m_cvmPtrVariantManager, SIGNAL(valueChanged(QtProperty*,const QVariant&)), this, SLOT(SlotPbPropertyValueChanged(QtProperty*,const QVariant&)));
}


void CPropBrowser::BuildFunctionPointerMaps()
{
	// specific update methods
	m_qmUpdateSpecificPropertyAttributes.clear();
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Group"),&CPropBrowser::Group_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Int"), &CPropBrowser::Int_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Float"), &CPropBrowser::Float_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Enum"), &CPropBrowser::Enum_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Flag"), &CPropBrowser::Flag_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Bool"), &CPropBrowser::Bool_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("String"), &CPropBrowser::String_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Color"), &CPropBrowser::Color_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Font"), &CPropBrowser::Font_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("HTML"), &CPropBrowser::HTML_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("Path"), &CPropBrowser::Path_UpdateSpecificPropertyAttributes);
	m_qmUpdateSpecificPropertyAttributes.insert(QString("JavaScript"), &CPropBrowser::JavaScript_UpdateSpecificPropertyAttributes);

	// specific get methods
	m_qmSpecificGetValue.clear();
	m_qmSpecificGetValue.insert(QString("Group"), &CPropBrowser::Specific_Group_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Int"), &CPropBrowser::Specific_Int_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Float"), &CPropBrowser::Specific_Float_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Enum"), &CPropBrowser::Specific_Enum_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Flag"), &CPropBrowser::Specific_Flag_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Bool"), &CPropBrowser::Specific_Bool_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("String"), &CPropBrowser::Specific_String_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Color"), &CPropBrowser::Specific_Color_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Font"), &CPropBrowser::Specific_Font_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("HTML"), &CPropBrowser::Specific_HTML_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("Path"), &CPropBrowser::Specific_Path_GetCurrentValue);
	m_qmSpecificGetValue.insert(QString("JavaScript"), &CPropBrowser::Specific_JavaScript_GetCurrentValue);

	// specific set methods
	m_qmSpecificSetValue.clear();
	m_qmSpecificSetValue.insert(QString("Group"), &CPropBrowser::Specific_Group_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Int"), &CPropBrowser::Specific_Int_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Float"), &CPropBrowser::Specific_Float_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Enum"), &CPropBrowser::Specific_Enum_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Flag"), &CPropBrowser::Specific_Flag_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Bool"), &CPropBrowser::Specific_Bool_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("String"), &CPropBrowser::Specific_String_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Color"), &CPropBrowser::Specific_Color_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Font"), &CPropBrowser::Specific_Font_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("HTML"), &CPropBrowser::Specific_HTML_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("Path"), &CPropBrowser::Specific_Path_SetPropertyValue);
	m_qmSpecificSetValue.insert(QString("JavaScript"), &CPropBrowser::Specific_JavaScript_SetPropertyValue);
}


bool CPropBrowser::IsPropertyBrowserValidProperty(const CGexPublicProperty& cgppProperty)	const
{
	/// TO REVIEW : validity management for properties used by CPropBrowser
	// const int nParentPropertyID(cgppProperty.GetParentPropertyId());
	// const int nPropertyID(cgppProperty.GetPropertyId());
	const QList<QString> qlAttributeList(cgppProperty.GetAttributeLabelList());



//	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyID))
//	{
//		/// TO DO : error message
//		Q_ASSERT(false);
//		return false;
//	}

//	if( (!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nParentPropertyID))	&&
//		(nParentPropertyID != -1)											)
//	{
//		/// TO DO : error message
//		Q_ASSERT(false);
//		return false;
//	}

	if( (!qlAttributeList.contains(QString("type")))		||
		(!qlAttributeList.contains(QString("label")))		)
	{
        GSLOG(SYSLOG_SEV_WARNING, "Invalid property browser property found (no 'type' nor 'label') !");
		return false;
	}

	/// TO DO :
	/// check group or flag property attributes


	// every test is ok !
	return true;
}

bool CPropBrowser::IsPropertyBrowserValidProperty(const int nPropertyID)	const
{
	CGexPublicProperty cgppShPtrProperty(m_cpimPtrPropertyIdManager->GetPublicProperty(nPropertyID));
	return IsPropertyBrowserValidProperty(cgppShPtrProperty);
}

/////////////////////////////////////////
// global property management methods
bool CPropBrowser::UpdatePropertyAttributes(const int nPropertyID)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Updating property attributes (id=%1)...").arg(nPropertyID).toLatin1().constData());

    bool bUpdatePropertyRslt = true;
    QVariant qvPropertyValue = GetCurrentValue(nPropertyID);

	bUpdatePropertyRslt &= UpdateGenericPropertyAttributes(nPropertyID);
	bUpdatePropertyRslt &= UpdateSpecificPropertyAttributes(nPropertyID);
    bUpdatePropertyRslt &= SetPropertyValue(nPropertyID, qvPropertyValue);		// necessary for enum and flag properties (reseted when name changed )

	return bUpdatePropertyRslt;
}


bool CPropBrowser::UpdateGenericPropertyAttributes(const int nPropertyID)
{
	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyID, true))
		return false;

	QtVariantProperty* qvpPtrVariantProperty = m_cpimPtrPropertyIdManager->GetPrivateProperty(nPropertyID);
	QList<QtBrowserItem *> qlBrowserItemPtrList = items(qvpPtrVariantProperty);
	if( (!qvpPtrVariantProperty) || (qlBrowserItemPtrList.count()!=1) )
	{
        GSLOG(SYSLOG_SEV_WARNING, "Invalid internal architecture detected in gex property browser !");
		GEX_ASSERT(false);		/// TO DO : error msg
		return false;
	}

	QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty = m_cpimPtrPropertyIdManager->GetPublicProperty(nPropertyID);
	if(!cgppShPtrGexProperty)
	{
        GSLOG(SYSLOG_SEV_WARNING, "Can't find a valid gex public property !");
		GEX_ASSERT(false);
		return false;
	}

    QTreeWidgetItem *qtwiPtrWidgetItem = getWidgetItem(qlBrowserItemPtrList.first());
	if(!qtwiPtrWidgetItem)
	{
        GSLOG(SYSLOG_SEV_CRITICAL, "NULL pointer found instead of QTreeWidgetItem pointer !");
		GEX_ASSERT(false);
		return false;
	}


	////////////////////////////////////////////
	// process the generic attribute list
	QString strAttributeLabel, strAttributeValue;



	//////////////////////////////////////////////////////////////////////
	/// TO REMOVE LATER, when assumed by validity dedicated method.
	// PYC, 01/06/2011

	// 1. type attribute
	strAttributeLabel = QString("type");
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);

	QStringList qslPossibleTypeAttributeValue;
	qslPossibleTypeAttributeValue << QString("Group") << QString("Int") << QString("Float") << QString("Enum");
	qslPossibleTypeAttributeValue << QString("Flag") << QString("Bool") << QString("String") << QString("Color");
	qslPossibleTypeAttributeValue << QString("Font") << QString("HTML") << QString("Path") << QString("JavaScript");

	if(!qslPossibleTypeAttributeValue.contains(strAttributeValue))
	{
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Wrong property type : '%1'").arg( strAttributeValue).toLatin1().constData());
		GEX_ASSERT(false);
	}


	// 2. label attribute
	strAttributeLabel = QString("label");
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);

	if(strAttributeValue.isEmpty())
	{
        GSLOG(SYSLOG_SEV_CRITICAL, "Empty property label !");
		GEX_ASSERT(false);
	}


	// 3. icon attribute
	strAttributeLabel = QString("icon");
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);

	if( (!QFile::exists(strAttributeValue)) && (!strAttributeValue.isEmpty()) )
	{
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Can't find icon : %1").arg(strAttributeValue).toLatin1().constData());
	}

	strAttributeLabel = QString("label");
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
	/// TO REVIEW


	/// TO REVIEW : is it possible to change and so update 'type' attribute ? how ? what does it mean ?
	/// TO REVIEW : enhance property browser attribute management.


	strAttributeLabel = QString("hideif");
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
	if(!strAttributeValue.isEmpty())
	{
		QScriptValue qsvScriptValue=m_cgsePtrScriptEngine->evaluate(strAttributeValue);
		if ( (qsvScriptValue.isError()) || (!qsvScriptValue.isBool()) )
		{
            GSLOG(SYSLOG_SEV_WARNING, QString("Error while evaluating javascript expression : '%1' %2' : %3")
                     .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label")).toLatin1().constData())
                     .arg(strAttributeLabel.toLatin1().data())
                     .arg(m_cgsePtrScriptEngine->uncaughtException().toString()).toLatin1().data() );
			//Q_ASSERT(false);
			return false;
		}

        qtwiPtrWidgetItem->setHidden(qsvScriptValue.toBool());
	}

	strAttributeLabel = QString("tooltip");
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
	if(!strAttributeValue.isEmpty())
	{
		qvpPtrVariantProperty->setToolTip(strAttributeValue);
	}

    strAttributeValue = cgppShPtrGexProperty->GetAttributeValue("disableif");
    if(!strAttributeValue.isEmpty())
    {
        QScriptValue qsvScriptValue=m_cgsePtrScriptEngine->evaluate(strAttributeValue);
        if ( (qsvScriptValue.isError()) || (!qsvScriptValue.isBool()) )
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Error while evaluating javascript expression : '%1' %2' : %3")
                     .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label")).toLatin1().constData())
                     .arg(strAttributeLabel.toLatin1().data())
                     .arg(m_cgsePtrScriptEngine->uncaughtException().toString()).toLatin1().data() );
            return false;
        }

        bool r=qsvScriptValue.toBool();

        GSLOG(5, QString("Disabling property %1 : %2 ? %3")
                .arg(nPropertyID)
                .arg(strAttributeValue.toLatin1().data())
                .arg(r?"true":"false").toLatin1().constData());
        //qtwiPtrWidgetItem->setDisabled(true); // does not seems to work
        //qtwiPtrWidgetItem->setFlags( qtwiPtrWidgetItem->flags() & !Qt::ItemIsEnabled); // does not work
        // todo : try    prop browser private disableItem(...)
        //if (r) disableItem(qtwiPtrWidgetItem); // does not work !
        //setItemVisible(qlBrowserItemPtrList.first(), !r ); // works but we want to disable !
        setItemEnable(qlBrowserItemPtrList.first(), !r ); // not works
    }

    strAttributeLabel = QString("icon");	// WARNING : must be done at the end to overload the internal update flow
	strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
	if(!strAttributeValue.isEmpty())
	{
		QIcon	qiIcon;
		qiIcon.addFile(strAttributeValue);
		qtwiPtrWidgetItem->setIcon(0, qiIcon);
	}

	// everything went well !!
	return true;
}


bool CPropBrowser::UpdateSpecificPropertyAttributes( const int nPropertyID)
{
	if(!m_cpimPtrPropertyIdManager->IsUsedPropertyID(nPropertyID, true))
		return false;

	QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty = m_cpimPtrPropertyIdManager->GetPublicProperty(nPropertyID);
	if(!cgppShPtrGexProperty)
	{
        GSLOG(SYSLOG_SEV_ERROR, "Invalid gex public property found !");
		return false;
	}

	QtVariantProperty* qvpPtrVariantProperty = m_cpimPtrPropertyIdManager->GetPrivateProperty(nPropertyID);
	if(!qvpPtrVariantProperty)
	{
        GSLOG(SYSLOG_SEV_ERROR, "Invalid variant property found !");
		return false;
	}

	// get the specific function pointer
	QString strTypeAttribute = cgppShPtrGexProperty->GetAttributeValue(QString("type"));
	bool(CPropBrowser::*fPtrUpdateSpecificPropertyAttributes)(QExplicitlySharedDataPointer<CGexPublicProperty>, QtVariantProperty*) = m_qmUpdateSpecificPropertyAttributes.value(strTypeAttribute, NULL);
	if(!fPtrUpdateSpecificPropertyAttributes)
	{
        GSLOG(SYSLOG_SEV_ERROR, "NULL pointer find for specific property attributes update!");
		return false;
	}

	bool bUpdateSpecificPropertyAttRslt = (this->*fPtrUpdateSpecificPropertyAttributes)(cgppShPtrGexProperty, qvpPtrVariantProperty);
	if(!bUpdateSpecificPropertyAttRslt)
	{
        GSLOG(SYSLOG_SEV_ERROR, QString("Problem occured when update specific property ('%1') attributes !")
                 .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData() );
		return false;
	}

	// everything went well !
	return true;
}


////////////////////////////////////////////////
// specific property management methods


// Update methods
bool CPropBrowser::Group_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}

bool CPropBrowser::Int_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty)
{
	QList<QString> qlAttributeList = cgppShPtrGexProperty->GetAttributeLabelList();
	if(qlAttributeList.count() <= 0)
		return true;

	QString strAttributeLabel;
	QString strAttributeValue;
	bool bConversionRslt;

	// added for validation, to remove later ? (when specific validation methods added for example)
	int nMinimumAttribute=0, nMaximumAttribute=0, nSingleStepAttribute=0;
	bool bIsMinimumDefined=false, bIsMaximumDefined=false, bIsSingleStepDefined=false;


	// minimum attribute
	strAttributeLabel = QString("minimum");
	if(qlAttributeList.contains(strAttributeLabel))
	{
		strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
		nMinimumAttribute = strAttributeValue.toInt(&bConversionRslt);
		QVariant qvValue = QVariant(nMinimumAttribute);
		//QVariant qvValue = QVariant(strAttributeValue.toInt(&bConversionRslt));
		if( (!bConversionRslt) || (!qvValue.isValid()) )
		{
			/// TO DO error msg
			GEX_ASSERT(false);
			return false;
		}

		bIsMinimumDefined = true;
		qvpPtrVariantProperty->setAttribute(strAttributeLabel, qvValue);
	}

	// maximum attribute
	strAttributeLabel = QString("maximum");
	if(qlAttributeList.contains(strAttributeLabel))
	{
		strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
		nMaximumAttribute = strAttributeValue.toInt(&bConversionRslt);
		QVariant qvValue = QVariant(nMaximumAttribute);
		// QVariant qvValue = QVariant(strAttributeValue.toInt(&bConversionRslt));
		if( (!bConversionRslt) || (!qvValue.isValid()) )
		{
			/// TO DO error msg
			GEX_ASSERT(false);
			return false;
		}

		bIsMaximumDefined=true;
		qvpPtrVariantProperty->setAttribute(strAttributeLabel, qvValue);
	}

	// single step attribute
	strAttributeLabel = QString("singleStep");
	if(qlAttributeList.contains(strAttributeLabel))
	{
		strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
		nSingleStepAttribute = strAttributeValue.toInt(&bConversionRslt);
		QVariant qvValue = QVariant(nSingleStepAttribute);
		//QVariant qvValue = QVariant(strAttributeValue.toInt(&bConversionRslt));
		if( (!bConversionRslt) || (!qvValue.isValid()) )
		{
			/// TO DO error msg
			GEX_ASSERT(false);
			return false;
		}

		bIsSingleStepDefined=true;
		qvpPtrVariantProperty->setAttribute(strAttributeLabel, qvValue);
	}

	//////////////////////////////////////////////////////////////////////
	/// TO REMOVE LATER, when assumed by validity dedicated method.
	// PYC, 01/06/2011

	if(bIsMinimumDefined && bIsMaximumDefined)
	{
		if(nMinimumAttribute >= nMaximumAttribute)
		{
            GSLOG(SYSLOG_SEV_CRITICAL, QString("Int attribute minimum value (%1) is upper than maximum value (%2)")
                  .arg(nMinimumAttribute)
                  .arg(nMaximumAttribute).toLatin1().constData());
			GEX_ASSERT(false);
		}

		if(bIsSingleStepDefined && (nSingleStepAttribute > (nMaximumAttribute-nMinimumAttribute) ) )
		{
            GSLOG(SYSLOG_SEV_CRITICAL, QString("Int attribute singleStep value(%1) does not match with minimum(%2) and maximum(%3) values")
                  .arg(nSingleStepAttribute)
                  .arg(nMinimumAttribute)
                  .arg(nMaximumAttribute).toLatin1().constData());
			GEX_ASSERT(false);
		}
	}
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////


	// everything went well !
	return true;
}

bool CPropBrowser::Float_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty)
{
	QList<QString> qlAttributeList = cgppShPtrGexProperty->GetAttributeLabelList();
	if(qlAttributeList.count() <= 0)
		return true;

	QString strAttributeLabel;
	QString strAttributeValue;
	bool bConversionRslt;

	// added for validation, to remove later ? (when specific validation methods added for example)
  float nMinimumAttribute=0, nMaximumAttribute=0, nSingleStepAttribute=0;
	bool bIsMinimumDefined=false, bIsMaximumDefined=false, bIsSingleStepDefined=false;

	// minimum attribute
	strAttributeLabel = QString("minimum");
	if(qlAttributeList.contains(strAttributeLabel))
	{
		strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
		nMinimumAttribute = strAttributeValue.toFloat(&bConversionRslt);
		QVariant qvValue = QVariant(nMinimumAttribute);
		//QVariant qvValue = QVariant(strAttributeValue.toFloat(&bConversionRslt));
		if( (!bConversionRslt) || (!qvValue.isValid()) )
		{
			/// TO DO error msg
			GEX_ASSERT(false);
			return false;
		}

		bIsMinimumDefined = true;
		qvpPtrVariantProperty->setAttribute(strAttributeLabel, qvValue);
	}

	// maximum attribute
	strAttributeLabel = QString("maximum");
	if(qlAttributeList.contains(strAttributeLabel))
	{
		strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
		nMaximumAttribute = strAttributeValue.toFloat(&bConversionRslt);
		QVariant qvValue = QVariant(nMaximumAttribute);
		// QVariant qvValue = QVariant(strAttributeValue.toFloat(&bConversionRslt));
		if( (!bConversionRslt) || (!qvValue.isValid()) )
		{
			/// TO DO error msg
			GEX_ASSERT(false);
			return false;
		}

		bIsMaximumDefined = true;
		qvpPtrVariantProperty->setAttribute(strAttributeLabel, qvValue);
	}

	// single step attribute
	strAttributeLabel = QString("singleStep");
	if(qlAttributeList.contains(strAttributeLabel))
	{
		strAttributeValue = cgppShPtrGexProperty->GetAttributeValue(strAttributeLabel);
		nSingleStepAttribute = strAttributeValue.toFloat(&bConversionRslt);
		QVariant qvValue = QVariant(nSingleStepAttribute);
		// QVariant qvValue = QVariant(strAttributeValue.toFloat(&bConversionRslt));
		if( (!bConversionRslt) || (!qvValue.isValid()) )
		{
			/// TO DO error msg
			GEX_ASSERT(false);
			return false;
		}

		bIsSingleStepDefined=true;
		qvpPtrVariantProperty->setAttribute(strAttributeLabel, qvValue);
	}


	//////////////////////////////////////////////////////////////////////
	/// TO REMOVE LATER, when assumed by validity dedicated method.
	// PYC, 01/06/2011

	if(bIsMinimumDefined && bIsMaximumDefined)
	{
		if(nMinimumAttribute >= nMaximumAttribute)
		{
            GSLOG(SYSLOG_SEV_CRITICAL, QString("Float attribute minimum value (%1) is upper than maximum value (%2)")
                                               .arg(nMinimumAttribute)
                                               .arg(nMaximumAttribute).toLatin1().constData());
			GEX_ASSERT(false);
		}

		if(bIsSingleStepDefined && (nSingleStepAttribute > (nMaximumAttribute-nMinimumAttribute) ) )
		{
            GSLOG(SYSLOG_SEV_CRITICAL, QString("Float attribute singleStep value(%1) does not match with minimum(%2) and maximum(%3) values")
                                               .arg(nSingleStepAttribute)
                                               .arg(nMinimumAttribute)
                                               .arg(nMaximumAttribute).toLatin1().constData());
			GEX_ASSERT(false);
		}
	}
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////

	// everything went well !
	return true;
}

bool CPropBrowser::Enum_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty)
{
	QList<QString> qlAttributeList = cgppShPtrGexProperty->GetAttributeLabelList();
	if(qlAttributeList.count() <= 0)
	{
        GSLOG(SYSLOG_SEV_CRITICAL, "no attribute found for this Enum property !");
		return false;
	}
	// codingvalues and displayvalues are mandatory for enum type property

	// added for validation, to remove later ? (when specific validation methods added for example)
	QStringList qslDisplayValueList, qslCodingValueList;

	QString strPropertyDisplayValues("displayvalues");
	if(!qlAttributeList.contains(strPropertyDisplayValues))
	{
        GSLOG(SYSLOG_SEV_CRITICAL, QString("displayvalues not found in enum property ('%1') attributes !")
               .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData());
		return false;
		// codingvalues and displayvalues are mandatory for enum type property
	}
	else
	{
		//QStringList qslDisplayValueList = (cgppShPtrGexProperty->GetAttributeValue(strPropertyDisplayValues)).split(QString("|"));
		qslDisplayValueList = (cgppShPtrGexProperty->GetAttributeValue(strPropertyDisplayValues)).split(QString("|"));
		qvpPtrVariantProperty->setAttribute(QString("enumNames"), qslDisplayValueList);		// Comment : set the property value to -1; cf. qtpropertymanager.cpp l. 4830
	}

	/// TO REVIEW : property value if possible values changed ?

	QString strPropertyEnumIcons("enumicons");
	if(qlAttributeList.contains(strPropertyEnumIcons))
	{
		QStringList qslEnumIconPathList = (cgppShPtrGexProperty->GetAttributeValue(strPropertyEnumIcons)).split(QString("|"));
		QtIconMap qimIconMap;
		QString strIconPath;
		QIcon qiIcon;

		for(int ii=0; ii<qslEnumIconPathList.count(); ii++)
		{
			strIconPath = qslEnumIconPathList.at(ii);
			qiIcon.addFile(strIconPath);
			qimIconMap.insert(ii, qiIcon);
		}
		// Comment : no problem if qimIconMap.count differ from enum list element count !!

		QVariant qvEnumIconAttributeValue;
		qVariantSetValue(qvEnumIconAttributeValue,qimIconMap);
		qvpPtrVariantProperty->setAttribute(QString("enumIcons"), qvEnumIconAttributeValue);
	}


	//////////////////////////////////////////////////////////////////////
	/// TO REMOVE LATER, when assumed by validity dedicated method.
	// PYC, 01/06/2011

	QString strCodingValues("codingvalues");
	qslCodingValueList = (cgppShPtrGexProperty->GetAttributeValue(strCodingValues)).split(QString("|"));
	if( qslDisplayValueList.count() != qslCodingValueList.count())
	{
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Enum '%1' codingvalues attribute count (%2) differ from displayvalues count (%3) !")
                .arg( (cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData())
                .arg( qslCodingValueList.count(), qslDisplayValueList.count() ).toLatin1().constData());
		GEX_ASSERT(false);
	}
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////


	// everything went well !
	return true;
}


bool CPropBrowser::Flag_UpdateSpecificPropertyAttributes(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty)
{
	QList<QString> qlAttributeList = cgppShPtrGexProperty->GetAttributeLabelList();
	if(qlAttributeList.count() <= 0)
		return false;		/// TO DO error msg
	// codingvalues and displayvalues are mandatory for flag type property

	// added for validation, to remove later ? (when specific validation methods added for example)
	QStringList qslDisplayValueList, qslCodingValueList;

	QString strPropertyDisplayValues("displayvalues");
	if(!qlAttributeList.contains(strPropertyDisplayValues))
	{
		return false;/// TO DO error msg
		// displayvalues are mandatory for flag type property
	}
	else
	{
		//QStringList qslDisplayValueList = (cgppShPtrGexProperty->GetAttributeValue(strPropertyDisplayValues)).split(QString("|"));
		qslDisplayValueList = (cgppShPtrGexProperty->GetAttributeValue(strPropertyDisplayValues)).split(QString("|"));
		qvpPtrVariantProperty->setAttribute(QString("flagNames"), qslDisplayValueList);
	}

	//////////////////////////////////////////////////////////////////////
	/// TO REMOVE LATER, when assumed by validity dedicated method.
	// PYC, 01/06/2011

	QString strCodingValues("codingvalues");
	qslCodingValueList = (cgppShPtrGexProperty->GetAttributeValue(strCodingValues)).split(QString("|"));
	if( qslDisplayValueList.count() != qslCodingValueList.count())
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Flag '%1' codingvalues attribute count (%2) differ from displayvalues count (%3) !")
                 .arg((cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData())
                 .arg(qslCodingValueList.count())
                 .arg(qslDisplayValueList.count() ).toLatin1().constData());
		GEX_ASSERT(false);

	}
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////


	/// TO REVIEW : property value if possible values changed ?
	// everything went well !
	return true;
}

bool CPropBrowser::Bool_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}

bool CPropBrowser::String_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	/// TODO : regExp key word
	return true;
}

bool CPropBrowser::Color_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}

bool CPropBrowser::Font_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}

bool CPropBrowser::HTML_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}

bool CPropBrowser::Path_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}

bool CPropBrowser::JavaScript_UpdateSpecificPropertyAttributes(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/)
{
	// nothing to do : 2011-03-15
	return true;
}


// specific get property value methods
QVariant CPropBrowser::Specific_Group_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* /*qvpPtrVariantProperty*/) const
{
	// nothing to return, no value associated with Group type properties !
	return QVariant((QVariant::Type)m_cvmPtrVariantManager->groupTypeId());;
}

QVariant CPropBrowser::Specific_Int_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_Float_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant	CPropBrowser::Specific_Enum_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const
{
	QString strCodingValues = cgppShPtrGexProperty->GetAttributeValue(QString("codingvalues"));
	QStringList qslCodingValueList = strCodingValues.split(QString("|"));
	QString strRequestedValue(QString(""));

	bool bConversionRslt=false;
	QVariant qvVariantManagerValue = m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
	int nEnumIntValue = qvVariantManagerValue.toInt(&bConversionRslt);
	if(		(!qvVariantManagerValue.isValid())			||
			(!bConversionRslt)							||
			(nEnumIntValue < -1)						||		// 'no selection' value
			(qslCodingValueList.count()<nEnumIntValue)	)		/// TO CHECK : nEnumIntValue == 0 for first enum value
	{
        GSLOG(SYSLOG_SEV_ERROR, QString("Problem occured in get current enum value; property : %1")
               .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData());
		//Q_ASSERT(false);
		return QVariant();
	}

	if(nEnumIntValue == -1)		// no selection
		return QVariant((QVariant::Type)m_cvmPtrVariantManager->enumTypeId());

	strRequestedValue = qslCodingValueList.at(nEnumIntValue);

	return QVariant(strRequestedValue);
}

QVariant	CPropBrowser::Specific_Flag_GetCurrentValue(const QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty,const QtVariantProperty* qvpPtrVariantProperty) const
{
	QString strCodingValues = cgppShPtrGexProperty->GetAttributeValue(QString("codingvalues"));
	QStringList qslCodingValueList = strCodingValues.split(QString("|"));
	QString strRequestedValue(QString(""));

	bool bConversionRslt=false;
	QVariant qvVariantManagerValue = m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
	int nFlagIntValue = qvVariantManagerValue.toInt(&bConversionRslt);
	if( (!qvVariantManagerValue.isValid()) || (!bConversionRslt) )
	{
        GSLOG(SYSLOG_SEV_WARNING, QString("Find an invalid Flag value; property : '%1'")
              .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData());
		GEX_ASSERT(false);
		return false;
	}

	for(int ii=0; ii<qslCodingValueList.count(); ii++)
	{
		int nFocusedCodingValue = 1<<ii;
		bool bIsFocusedValue = nFlagIntValue&nFocusedCodingValue;
		if( bIsFocusedValue )
		{
			if(strRequestedValue!=QString(""))
				strRequestedValue = strRequestedValue + QString("|") + qslCodingValueList.at(ii);
			else
				strRequestedValue = qslCodingValueList.at(ii);		// first 'value'
		}
	}

	return QVariant(strRequestedValue);
}

QVariant CPropBrowser::Specific_Bool_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_String_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_Color_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_Font_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_HTML_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_Path_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}

QVariant CPropBrowser::Specific_JavaScript_GetCurrentValue(
const QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
const QtVariantProperty* qvpPtrVariantProperty) const
{
	return m_cvmPtrVariantManager->value(qvpPtrVariantProperty);
}


// specific set property value methods
bool CPropBrowser::Specific_Group_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* /*qvpPtrVariantProperty*/,
	const QVariant /*qvNewValue*/)
{
	// nothing to set, no value associated with Group type properties !
	return true;
}

bool CPropBrowser::Specific_Int_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_Float_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool	CPropBrowser::Specific_Enum_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue)
{
	// Enum options are used in propertyBrowser with an int value
	QString strNewValue = qvNewValue.toString();

	QString strPossibleValues = cgppShPtrGexProperty->GetAttributeValue(QString("codingvalues"));
	QStringList qslPossibleValueList = strPossibleValues.split(QString("|"));

	if(strNewValue.isEmpty())
	{
        GSLOG(6, QString("Set no selection for an Enum property ('%1')")
               .arg(cgppShPtrGexProperty->GetAttributeValue(QString("label"))).toLatin1().constData());
		m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, QVariant(-1));
		return true;
	}

	if(!qslPossibleValueList.contains(strNewValue))
	{
        GSLOG(SYSLOG_SEV_WARNING, QString("Try to set an invalid Enum value (%1)")
               .arg(strNewValue).toLatin1().constData());
		return false;
	}

	int nEnumValueInt = qslPossibleValueList.indexOf(strNewValue);
	if(nEnumValueInt<0)
	{
        GSLOG(SYSLOG_SEV_WARNING, QString("Find an invalid Enum int value; value to set : %1")
               .arg(strNewValue).toLatin1().constData());
		return false;
	}

	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, QVariant(nEnumValueInt));
	/// TO DO : validator
	return true;
}
bool	CPropBrowser::Specific_Flag_SetPropertyValue(QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrGexProperty, QtVariantProperty* qvpPtrVariantProperty, const QVariant qvNewValue)
{
	// Flag options are used in propertyBrowser with an int value
	QString strNewValues = qvNewValue.toString();
	QStringList qslNewValueList = strNewValues.split(QString("|"));

	QString strCodingValues = cgppShPtrGexProperty->GetAttributeValue(QString("codingvalues"));
	QStringList qslCodingValueList = strCodingValues.split(QString("|"));

	int nFlagValueInt=0;

	for(int ii=0; ii<qslCodingValueList.count(); ii++)
	{
		QString qsCodingValue = qslCodingValueList.at(ii);
		if(qslNewValueList.contains(qsCodingValue, Qt::CaseInsensitive))
			nFlagValueInt += (int)1<<ii;
	}

	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, QVariant(nFlagValueInt));
	/// TO DO : validator
	return true;
}

bool CPropBrowser::Specific_Bool_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_String_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_Color_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_Font_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_HTML_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_Path_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}

bool CPropBrowser::Specific_JavaScript_SetPropertyValue(
	QExplicitlySharedDataPointer<CGexPublicProperty> /*cgppShPtrGexProperty*/,
	QtVariantProperty* qvpPtrVariantProperty,
	const QVariant qvNewValue)
{
	/// TO DO : validator
	m_cvmPtrVariantManager->setValue(qvpPtrVariantProperty, qvNewValue);
	return true;
}


