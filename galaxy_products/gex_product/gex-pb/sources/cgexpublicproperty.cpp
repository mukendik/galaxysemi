#include "cgexpublicproperty.h"
#include "cpropertyidmanager.h"

#include <gqtl_log.h>

///////////////////////////////////
// constructor(s) / destructor
///////////////////////////////////

CGexPublicProperty::CGexPublicProperty(const int nParentPropertyID /*=-1*/, const int nPropertyId /*=-1*/) :
		 QSharedData(), m_nPropertyId(nPropertyId), m_nParentPropertyId(nParentPropertyID)
{
	Clear();
}

// COMMENT : QExplicitlySharedDataPointer used a copy of the object ... returned CGexPublicProperty ID is -1 !
CGexPublicProperty::CGexPublicProperty( const CGexPublicProperty& cgppProperty ) :
		 QSharedData(cgppProperty), m_nPropertyId(cgppProperty.m_nPropertyId), m_nParentPropertyId(cgppProperty.m_nParentPropertyId)
{
	Clear();
	//m_nParentPropertyId=cgppProperty.m_nParentPropertyId;
	m_qmAttributesMap=cgppProperty.m_qmAttributesMap;
}

CGexPublicProperty::CGexPublicProperty(const CGexPublicProperty& cgppProperty , const int nPropertyId) :
		QSharedData(cgppProperty), m_nPropertyId(nPropertyId), m_nParentPropertyId(cgppProperty.m_nParentPropertyId)
{
	Clear();
	m_qmAttributesMap=cgppProperty.m_qmAttributesMap;
}

CGexPublicProperty::CGexPublicProperty(const QMap<QString, QString > qmPropertyAttributes, const int nParentPropertyId /*=-1*/, const int nPropertyId /*=-1*/) :
		QSharedData(), m_nPropertyId(nPropertyId), m_nParentPropertyId(nParentPropertyId)
{
	Clear();
	m_qmAttributesMap=qmPropertyAttributes;
}

CGexPublicProperty::~CGexPublicProperty()
{
	Clear();
}


//////////////////
// accessors
//////////////////
bool	CGexPublicProperty::SetAttribute(const QString strAttributeLabel, const QString strAttributeValue)
{
	// TO REVIEW
	//if(m_nPropertyId!=-1)	// currently, the only indicator from property browser use of the property, need more precision
	// {
	if( (IsReservedAttributeLabel(strAttributeLabel))		&&
			(m_qmAttributesMap.contains(strAttributeLabel))		)
		GEX_ASSERT(false);
	return false;
	// }

	m_qmAttributesMap.insert(strAttributeLabel, strAttributeValue);
	return true;
}

bool	CGexPublicProperty::SetAttribute(const QMap<QString, QString > qmPropertyAttributes)
{
	QMapIterator<QString, QString >	qmIterator(qmPropertyAttributes);

	/// TO REVIEW
	// insert no attribute before checking each of them !
	//if(m_nPropertyId!=-1)	// currently, the only indicator from property browser use of the property, need more precision
	//{
	while(qmIterator.hasNext())
	{
		qmIterator.next();

		if( (IsReservedAttributeLabel(qmIterator.key()))		&&
				(m_qmAttributesMap.contains(qmIterator.key()))		)
		{
            GSLOG(4, QString("ERROR : try to set invalid property attribute, '%1'")
                  .arg (qmIterator.key()).toLatin1().constData());
			GEX_ASSERT(false);
			return false;
		}
	}
	// }

	qmIterator.toFront();
	while(qmIterator.hasNext())
	{
		qmIterator.next();
		m_qmAttributesMap.insert(qmIterator.key(), qmIterator.value());
	}

	return true;
}

//bool	CGexPublicProperty::SetParentPropertyId(const int nParentPropertyId)
//{
//	m_nParentPropertyId=nParentPropertyId;
//	return true;
//}

QString		CGexPublicProperty::GetAttributeValue(const QString strAttributeLabel) const
{
	return m_qmAttributesMap.value(strAttributeLabel, QString(""));
}

QList<QString>	CGexPublicProperty::GetAttributeLabelList() const
{
	return m_qmAttributesMap.keys();
}

int		CGexPublicProperty::GetPropertyId() const
{
	return m_nPropertyId;
}

int		CGexPublicProperty::GetParentPropertyId() const
{
	return m_nParentPropertyId;
}


////////////////////////
// global methods
////////////////////////
void	CGexPublicProperty::Clear()
{

    QMap< QString, QString >	lTempCopy;

    QMap< QString, QString >::iterator lIterBegin(m_qmAttributesMap.begin()), lIterEnd(m_qmAttributesMap.end());

    for(;lIterBegin != lIterEnd; ++lIterBegin)
    {
        if(IsReservedAttributeLabel(lIterBegin.key()))
        {
            lTempCopy.insert(lIterBegin.key(), lIterBegin.value() );
        }
    }

    m_qmAttributesMap.clear();
    m_qmAttributesMap.swap(lTempCopy);


    /*QMapIterator<QString, QString> qmiAttributesIterator(m_qmAttributesMap);
	QString strAttributeLabel;

	while(qmiAttributesIterator.hasNext())
	{
		strAttributeLabel = (qmiAttributesIterator.next()).key();
		if(!IsReservedAttributeLabel(strAttributeLabel))
		{
			m_qmAttributesMap.take(strAttributeLabel);
		}
    }*/

	//m_qmAttributesMap=QMap<QString, QString >();
}


// ID management to check with shared data ...
//bool	CGexPublicProperty::CloneProperty(QExplicitlySharedDataPointer<CGexPublicProperty*> cgppShPtrPropertyClone, int nPropertyCloneId/*=-1*/)
//{
//	/// TODO
//	return false;
//}


bool	CGexPublicProperty::IsReservedAttributeLabel(QString strAttributeLabel)
{
	/// TO REVIEW
	if(strAttributeLabel == QString("type"))
	{
		return true;
	}

	// not reserved !
	return false;
}
