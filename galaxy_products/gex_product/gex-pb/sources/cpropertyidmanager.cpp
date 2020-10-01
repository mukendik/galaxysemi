#include "cpropertyidmanager.h"
#include <gqtl_log.h>

CPropertyIdManager::CPropertyIdManager()
{
}

CPropertyIdManager::~CPropertyIdManager()
{
}

//////////////////////////
// accessors
//////////////////////////
QExplicitlySharedDataPointer<CGexPublicProperty>	CPropertyIdManager::GetPublicProperty(const int nPropertyId)
{
	if(IsUsedPropertyID(nPropertyId))
		return m_qmPublicPropertyMap.value(nPropertyId);
	else
		return QExplicitlySharedDataPointer<CGexPublicProperty>();
}

QtVariantProperty*	CPropertyIdManager::GetPrivateProperty(const int nPropertyId)
{
	if(IsUsedPropertyID(nPropertyId))
		return m_qmPrivatePropertyMap.value(nPropertyId);
	else
		return NULL;
}

int	CPropertyIdManager::GetPrivatePropertyID(QtProperty* qpPtrProperty)
{
	QtVariantProperty* qvpPtrCastedProperty = dynamic_cast<QtVariantProperty*>(qpPtrProperty);

	if(!qvpPtrCastedProperty)
	{
		GEX_ASSERT(false);		/// TO REVIEW
		return -1;
	}

	int nSearchedPropertyId = m_qmPrivatePropertyMap.key(qvpPtrCastedProperty, -1);

	return nSearchedPropertyId;
}

QList<int>		CPropertyIdManager::GetPropertyIDList()
{
	if(!IsValid())
		return	QList<int>();
	else
		return m_qmPublicPropertyMap.keys();
}

//int	CPropertyIdManager::CreateProperty()
//{
//	/// TODO
//	return -1;
//}


int CPropertyIdManager::CreateProperty(CGexPublicProperty& cgppRefProperty, QtVariantProperty* qvpPtrProperty)
{
	const int nPropertyProposedID(cgppRefProperty.GetPropertyId());
	int nNewPropertyID;

	if( (nPropertyProposedID!=-1) && (!IsUsedPropertyID(nPropertyProposedID)) )
		nNewPropertyID=nPropertyProposedID;
	else
		nNewPropertyID = GenerateValidID();

	CGexPublicProperty cgppNewProperty(cgppRefProperty, nNewPropertyID);
	QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrInternalProperty;
	cgppShPtrInternalProperty = new CGexPublicProperty(cgppNewProperty);

	m_qmPublicPropertyMap.insert(nNewPropertyID, cgppShPtrInternalProperty);
	m_qmPrivatePropertyMap.insert(nNewPropertyID, qvpPtrProperty);

	return nNewPropertyID;
}


int CPropertyIdManager::CreateProperty(QMap<QString, QString > qmAttributeMap, QtVariantProperty* qvpPtrProperty, int nParentPropertyId /*= -1*/)
{
	CGexPublicProperty	cgppProperty(qmAttributeMap, nParentPropertyId);
	return CreateProperty(cgppProperty, qvpPtrProperty);
}

//bool CPropertyIdManager::DeletePropertiesFromId(const int nPropertyId)
//{
//	/// TODO
//	// comment : delete a QtVariantProperty suppress it from QtPropertyManager, cf. ~QtProperty()
//	return false;
//}

bool CPropertyIdManager::IsUsedPropertyID(const int nPropertyId, bool bHasToBeUsed/*=false*/) const
{
	if( (m_qmPrivatePropertyMap.contains(nPropertyId)) && (m_qmPublicPropertyMap.contains(nPropertyId)) )
		return true;
	else
	{
		if(bHasToBeUsed)
            GSLOG(4, "Warning : Invalid property id detected !");
		return false;
	}
}

///////////////////////////////
// internal methods
///////////////////////////////
bool CPropertyIdManager::IsValid()
{
	if(m_qmPrivatePropertyMap.count() != m_qmPublicPropertyMap.count())
	{
		GEX_ASSERT(false);
		return false;
	}

	QMapIterator<int, QExplicitlySharedDataPointer<CGexPublicProperty> > qmiIterator(m_qmPublicPropertyMap);
	while(qmiIterator.hasNext())
	{
		qmiIterator.next();
		if(!m_qmPrivatePropertyMap.contains(qmiIterator.key()))
		{
			GEX_ASSERT(false);
			return false;
		}
	}

	// everything went well !
	return true;
}

int CPropertyIdManager::GenerateValidID()
{
	if(!IsValid())
		return -1;

	for(int nIDIterator=0; nIDIterator <= m_qmPublicPropertyMap.count(); nIDIterator++)
	{
		if( (!m_qmPrivatePropertyMap.contains(nIDIterator)) && (!m_qmPublicPropertyMap.contains(nIDIterator)) )
			return nIDIterator;
	}

	// it isn't normal we didn't find a valid id with n+1 tries if only n id are already used
	GEX_ASSERT(false);
	return -1;		// invalid ID
}

