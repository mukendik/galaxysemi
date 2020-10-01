#ifndef CPROPERTYIDMANAGER_H
#define CPROPERTYIDMANAGER_H

#include "qtpropertybrowser.h"
#include "qtvariantproperty.h"

#include "cgexpublicproperty.h"

class CPropertyIdManager
{
public:
	////////////////////////////////////
	// constructor(s) / destructor
	////////////////////////////////////
    CPropertyIdManager();
	~CPropertyIdManager();

	//////////////////////////
	// accessors
	//////////////////////////
	QExplicitlySharedDataPointer<CGexPublicProperty>	GetPublicProperty(const int nPropertyId);
	QtVariantProperty*			GetPrivateProperty(const int nPropertyId);
	QList<int>					GetPropertyIDList();
	int							GetPrivatePropertyID(QtProperty*);

	// int	CreateProperty();
	int CreateProperty(CGexPublicProperty& cgppRefProperty, QtVariantProperty* qvpPtrProperty);
	int CreateProperty(QMap<QString, QString > qmAttributeMap, QtVariantProperty* qvpPtrProperty, int nParentPropertyId = -1);

	/// TO REVIEW : refresh questions if properties dynamically deleted ... hide instead of delete ? and what about the id ?
	/// cf. void QtAbstractPropertyBrowser::removeProperty
	// bool	DeletePropertiesFromId(const int nPropertyId);
	bool	IsUsedPropertyID(const int nPropertyId, bool bHasToBeUsed=false) const;

private:
	QMap<int, QExplicitlySharedDataPointer<CGexPublicProperty> >	m_qmPublicPropertyMap;
	QMap<int, QtVariantProperty* >	m_qmPrivatePropertyMap;

	///////////////////////////////
	// internal methods
	///////////////////////////////
	bool IsValid();
	int GenerateValidID();
};

#endif // CPROPERTYIDMANAGER_H
