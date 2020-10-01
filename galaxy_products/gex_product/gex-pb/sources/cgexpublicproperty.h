#ifndef CGEXPUBLICPROPERTY_H
#define CGEXPUBLICPROPERTY_H

#include	<QtCore>

#ifndef GEXPBSHARED_EXPORT
#if defined(GEXPB_LIBRARY)
#  define GEXPBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GEXPBSHARED_EXPORT Q_DECL_IMPORT
#endif
#endif


// class CGexProperty
class GEXPBSHARED_EXPORT CGexPublicProperty : public QSharedData
{
public:
	///////////////////////////////////////////////////
	// constructor(s) / destructor (private)
	///////////////////////////////////////////////////
	CGexPublicProperty(const int nParentPropertyID=-1, const int nPropertyID=-1);
	CGexPublicProperty(const CGexPublicProperty& cgppProperty);
	CGexPublicProperty(const CGexPublicProperty& cgppProperty, const int nPropertyID);
	CGexPublicProperty(const QMap<QString, QString > qmPropertyAttributes, const int nParentPropertyId=-1, const int nPropertyId=-1);

	~CGexPublicProperty();


	//////////////////
	// accessors
	//////////////////
	/**
	  \brief Set an attribute (label and value) to the property
	  \details Work has QMap::insert() method except that reserved attributes can't be changed

	  \param strAttributeLabel : name of the attribute
	  \param strAttributeValue : (new) value of the attribute

	   \return true if the attribute correctly inserted / set to the attribute list
	  */
	bool	SetAttribute(const QString strAttributeLabel, const QString strAttributeValue);
	bool	SetAttribute(const QMap<QString, QString > qmPropertyAttributes);
	// bool	SetParentPropertyId(const int nParentPropertyId);	// parent property id allocation security ?

	QString			GetAttributeValue(const QString strAttributeLabel) const;
	QList<QString>	GetAttributeLabelList() const;
	int				GetPropertyId() const;				// normal use is to keep the id to call the property
	int				GetParentPropertyId() const;


	////////////////////////
	// global methods
	////////////////////////
	//bool	CloneProperty(QExplicitlySharedDataPointer<CGexPublicProperty*> cgppShPtrPropertyClone, int nPropertyCloneId=-1);
	/**
	  \brief Reset property attributes that are not reserved ( IsReservedAttributeLabel(attribute_label) return false )
	  */
	void	Clear();
	/**
	  \brief Method used to list attribute name that are reserved

	  \param	strAttributeLabel : attribute label to check

	  \return	true if the given attribute label is reserved
	  */
	bool	IsReservedAttributeLabel(QString strAttributeLabel);


private:
	///////////////////
	// members
	///////////////////
	const int					m_nPropertyId;
	const int					m_nParentPropertyId;
	QMap< QString, QString >	m_qmAttributesMap;

	//////////////////////////////
	// internal methods
	//////////////////////////////

};


#endif // CGEXPUBLICPROPERTY_H
