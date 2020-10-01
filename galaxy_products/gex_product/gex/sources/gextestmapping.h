#ifndef _GEX_TEST_MAPPING_H_
#define _GEX_TEST_MAPPING_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "classes.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QVector>
#include <QMap>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMappingName
//
// Description	:	Base class to map test name
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMappingName
{
public:

	enum MappingType
	{
		typeString = 0,
		typeFunction
	};

	CGexTestMappingName(MappingType eRule);
	virtual ~CGexTestMappingName();

	MappingType							type() const						{ return m_eType; }

	virtual bool                        map(QString& testName) const = 0;

private:

	MappingType                         m_eType;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMappingNameString
//
// Description	:	Class allowing to map a test name with a new one
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMappingNameString : public CGexTestMappingName
{
public:

	CGexTestMappingNameString(const QString& oldTestName, const QString& newTestName);
	virtual ~CGexTestMappingNameString();

	bool								map(QString& testName) const;

    const QString&                      GetOldTestName() const;
    const QString&                      GetNewTestName() const;

private:

    QString                             mOldTestName;
    QString                             mNewTestName;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMappingNameFunction
//
// Description	:	Class allowing to map a test name using a function
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMappingNameFunction : public CGexTestMappingName
{
public:

	CGexTestMappingNameFunction(const QStringList& functions);
	virtual ~CGexTestMappingNameFunction();

	bool								map(QString& testName) const;

    const QStringList&                  GetFunctions() const;

private:

    QStringList                         mFunctions;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMappingNumber
//
// Description	:	Base class to map test number list
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMappingNumber
{
public:

	enum MappingType
	{
		typeOffset = 0,
		typeNumber
	};

	CGexTestMappingNumber(const QString& strTestList, int nValue, MappingType eRule);
	virtual ~CGexTestMappingNumber();

	int									value() const						{ return m_nValue; }
	MappingType							type() const						{ return m_eType; }
	const CGexTestRange&				testRange() const					{ return m_rangeTest; }

	bool								contains(int nTestNumber) const;

	virtual int							map(int nTestNumber) const = 0;

	static CGexTestMappingNumber *		create(const QString& strTestList, int nValue, MappingType eType);

private:

	CGexTestRange					m_rangeTest;
	MappingType						m_eType;
	int								m_nValue;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMappingNumberOffset
//
// Description	:	Class allowing to map a test number with an offset for a test list
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMappingNumberOffset : public CGexTestMappingNumber
{
public:

	CGexTestMappingNumberOffset(const QString& strTestList, int nValue);
	virtual ~CGexTestMappingNumberOffset();

	int									map(int nTestNumber) const;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMappingNumberValue
//
// Description	:	Class allowing to map a test number with another one for a test list
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMappingNumberValue : public CGexTestMappingNumber
{
public:

	CGexTestMappingNumberValue(const QString& strTestList, int nValue);
	virtual ~CGexTestMappingNumberValue();

	int									map(int nTestNumber) const;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTestMapping
//
// Description	:	Class holding the mapping for tests
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTestMapping
{

public:

	CGexTestMapping();
	~CGexTestMapping();

	const QVector<CGexTestMappingNumber*>	lstMappedTestNumber() const		{ return m_lstMappedTestNumber; }
	const QVector<CGexTestMappingName*>		lstMappedTestName() const		{ return m_lstMappedTestName; }

	bool	isActivated() const												{ return m_bActivated; }
	bool	isMappedNumber(int nTestNumber) const;

	void	addMappedNumber(const QString& strTestList, int nValue, CGexTestMappingNumber::MappingType eActionType);
	void	addMappedName(const QString& strOldName, const QString& strNewName);
    void	addMappedName(const QStringList &functions);

	int		mapNumber(int nTestNumber) const;
	bool 	mapName(QString& strTestName) const;

private:

	QVector<CGexTestMappingNumber*>	m_lstMappedTestNumber;
    QVector<CGexTestMappingName*>	m_lstMappedTestName;

	bool							m_bActivated;
};

#endif // _GEX_TEST_MAPPING_H_
