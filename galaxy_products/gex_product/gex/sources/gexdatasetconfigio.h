#ifndef _GEX_ABSTRACT_DATASET_CONFIG_IO_H_
#define _GEX_ABSTRACT_DATASET_CONFIG_IO_H_

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gextestmapping.h"
#include "gexdatasetconfig.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QDomDocument>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexAbstractDatasetConfigIO
//
// Description	:	Base class to read and write config files
//
///////////////////////////////////////////////////////////////////////////////////
class CGexAbstractDatasetConfigIO
{

public:

	CGexAbstractDatasetConfigIO(const QString& strFileName);
	virtual ~CGexAbstractDatasetConfigIO();

	virtual void							read(CGexDatasetConfig * pDatasetConfig) = 0;
	virtual void							write(CGexDatasetConfig * pDatasetConfig) = 0;

	static CGexAbstractDatasetConfigIO *	create(const QString& strFileName);

protected:

	QString		m_strFileName;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTxtDatasetConfigIO
//
// Description	:	Class which reads config files
//
///////////////////////////////////////////////////////////////////////////////////
class CGexTxtDatasetConfigIO : public CGexAbstractDatasetConfigIO
{

public:

	CGexTxtDatasetConfigIO(const QString& strFileName);
	virtual ~CGexTxtDatasetConfigIO();

	void	read(CGexDatasetConfig * pDatasetConfig);
	void	write(CGexDatasetConfig * pDatasetConfig);
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexXmlDatasetConfigIO
//
// Description	:	Class which reads config files (V1.0)
//
///////////////////////////////////////////////////////////////////////////////////
class CGexXmlDatasetConfigIO : public CGexAbstractDatasetConfigIO
{

public:

	CGexXmlDatasetConfigIO(const QString& strFileName);
	virtual ~CGexXmlDatasetConfigIO();

	void	read(CGexDatasetConfig * pDatasetConfig);
	void	write(CGexDatasetConfig * pDatasetConfig);

protected:

	void        readTestMappingSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);
	void        readTestFilterSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);
	void        readPartFilterSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);
	void        readTestToCreateSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);
    void        readTestToUpdateSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);


	void        writeComment(QDomDocument& xmlDocument);
	void        writeTestMappingSection(QDomDocument& xmlDocument, QDomElement& testElement, CGexDatasetConfig * pDatasetConfig);
	void        writeTestFilterSection(QDomDocument& xmlDocument, QDomElement& testElement, CGexDatasetConfig * pDatasetConfig);
	void        writePartFilterSection(QDomDocument& xmlDocument, QDomElement& partElement, CGexDatasetConfig * pDatasetConfig);
    ///> brief : write test section in xml file
    void writeTestToCreateSection(QDomDocument& xmlDocument,QDomElement& partElement,CGexDatasetConfig *pDatasetConfig);

    virtual void readTestNumberMapping(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);
    virtual void readTestNameMapping(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig);

    virtual void writeTestNumberMapping(QDomDocument& xmlDocument, QDomElement& mappingElement, CGexDatasetConfig * pDatasetConfig);
    virtual void writeTestNameMapping(QDomDocument& xmlDocument, QDomElement& mappingElement, CGexDatasetConfig * pDatasetConfig);
};

#endif // _GEX_ABSTRACT_DATASET_CONFIG_IO_H_
