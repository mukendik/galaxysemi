///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gexdatasetconfigio.h"
#include "browser_dialog.h"
#include <gqtl_log.h>
#include "message.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QFile>
#include <QTextStream>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexAbstractDatasetConfigIO
//
// Description	:	Base class to read and write config files
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexAbstractDatasetConfigIO::CGexAbstractDatasetConfigIO(const QString& strFileName)
{
    m_strFileName = strFileName;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexAbstractDatasetConfigIO::~CGexAbstractDatasetConfigIO()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexAbstractFileConfigIO * create(const QString& strFileName)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
CGexAbstractDatasetConfigIO * CGexAbstractDatasetConfigIO::create(const QString& strFileName)
{
    CGexAbstractDatasetConfigIO *	pDatasetConfigIO = NULL;
    QDomDocument					xmlDocument;
    QFile							xmlFile(strFileName);

    if (xmlFile.open(QIODevice::ReadOnly))
    {
        // Not a xml file, use txt reader
        if (!xmlDocument.setContent(&xmlFile))
            pDatasetConfigIO = new CGexTxtDatasetConfigIO(strFileName);
        else
        {
            if (xmlDocument.documentElement().tagName() == "gex_dataset_config")
            {
                if (xmlDocument.documentElement().attribute("version") == "1.0" ||
                    xmlDocument.documentElement().attribute("version") == "1.1")
                    pDatasetConfigIO = new CGexXmlDatasetConfigIO(strFileName);
            }
        }

        xmlFile.close();
    }

    return pDatasetConfigIO;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexTxtDatasetConfigIO
//
// Description	:	Class to read and write txt config files
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexTxtDatasetConfigIO::CGexTxtDatasetConfigIO(const QString& strFileName) : CGexAbstractDatasetConfigIO(strFileName)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexTxtDatasetConfigIO::~CGexTxtDatasetConfigIO()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void read(const QString& strFileName)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexTxtDatasetConfigIO::read(CGexDatasetConfig * pDatasetConfig)
{
    QFile file(m_strFileName);

    if (file.open(QIODevice::ReadOnly))
    {
        // Assign file I/O stream
        QTextStream hConfigFile(&file);
        QString		strLine;
        QString		strTestList;
        QString		strField;
        QString		strOldTestName;
        QString		strNewTestName;
        bool		bOkOld;
        bool		bOkNew;
    //FIXME: not used ?
    //int nOldTestNumber;
        int			nNewTestNumber;
        int			nLineRead = 0;

        // Read all the file
        do
        {
            // Read line
            strLine = hConfigFile.readLine();
            nLineRead++;

            // Remove spaces (beginning & leading)
            strLine = strLine.trimmed();

            if(strLine[0] != '#')
            {
                // This is a valid line (not a comment one)
                strTestList		= strLine.section(',',0,0).trimmed();
        /*nOldTestNumber = */strTestList.toInt(&bOkOld);
                strField			= strLine.section(',',1,1).trimmed();
                nNewTestNumber	= strField.toInt(&bOkNew);

                if ((bOkOld == true) && (bOkNew == true))
                    pDatasetConfig->m_testMapping.addMappedNumber(strTestList, nNewTestNumber, CGexTestMappingNumber::typeNumber);
                else
                {
                    // Check for keyword
                    strField = strLine.section(',',0,0).trimmed();
                    if(strField.startsWith("Test#", Qt::CaseInsensitive))
                    {
                        // test#,<Old_testID> = <New_Test_ID>
                        strLine			= strLine.section(',',1).trimmed();
                        strTestList		= strLine.section('=',0,0).trimmed();
                        strField		= strLine.section('=',1,1).trimmed();
                        /*nOldTestNumber = */strTestList.toInt(&bOkOld);
                        nNewTestNumber	= strField.toInt(&bOkNew);

                        if(bOkOld == true && bOkNew == true)
                            pDatasetConfig->m_testMapping.addMappedNumber(strTestList, nNewTestNumber, CGexTestMappingNumber::typeNumber);
                    }
                    else  if(strField.startsWith("TestName", Qt::CaseInsensitive))
                    {
                        // TestName,<Old_testName> = <New_Test_Name>
                        strLine			= strLine.section(',',1).trimmed();
                        strOldTestName	= strLine.section('=',0,0).trimmed();
                        strNewTestName	= strLine.section('=',1,1).trimmed();

                        if((strOldTestName.isEmpty() == false) && (strNewTestName.isEmpty() == false))
                            pDatasetConfig->m_testMapping.addMappedName(strOldTestName, strNewTestName);
                    }
                    else
                    {
                        // Something was wrong with this line...notify
                        strLine = "Failed importing all Mapping File (invalid line)\nFile: ";
                        strLine += m_strFileName;
                        strLine += "\nLine: " + QString::number(nLineRead);

                        GS::Gex::Message::information("", strLine);

                        // Close file
                        file.close();
                        return;
                    }
                }
            }
        }
        while(hConfigFile.atEnd() == false);

        // Close file
        file.close();
    }

    write(pDatasetConfig);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void write(CGexDatasetConfig * pDatasetConfig)
//
// Description	:	write config in a txt file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexTxtDatasetConfigIO::write(CGexDatasetConfig * pDatasetConfig)
{
    CGexXmlDatasetConfigIO xmlFileConfigIO(m_strFileName);

    xmlFileConfigIO.write(pDatasetConfig);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexXmlDatasetConfigIO
//
// Description	:	Class to read and write xml config files
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexXmlDatasetConfigIO::CGexXmlDatasetConfigIO(const QString& strFileName) : CGexAbstractDatasetConfigIO(strFileName)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexXmlDatasetConfigIO::~CGexXmlDatasetConfigIO()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void read(CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::read(CGexDatasetConfig * pDatasetConfig)
{
    QDomDocument				xmlDoc;
    QFile						xmlFile(m_strFileName);

    if (xmlFile.open(QIODevice::ReadOnly))
    {
        if (xmlDoc.setContent(&xmlFile))
        {
            readTestMappingSection(xmlDoc.documentElement(), pDatasetConfig);
            readTestFilterSection(xmlDoc.documentElement(), pDatasetConfig);
            readPartFilterSection(xmlDoc.documentElement(), pDatasetConfig);
            readTestToCreateSection(xmlDoc.documentElement(), pDatasetConfig);
            readTestToUpdateSection(xmlDoc.documentElement(), pDatasetConfig);
        }

        xmlFile.close();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readTestMappingSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read test section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::readTestMappingSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    QDomElement		testElement		= domElement.firstChildElement("test");
    QDomElement		mappingElement	= testElement.firstChildElement("mapping");

    // mapped number
    readTestNumberMapping(mappingElement, pDatasetConfig);

    // mapped name
    readTestNameMapping(mappingElement, pDatasetConfig);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readTestFilterSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read test section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::readTestFilterSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    QDomElement		testElement		= domElement.firstChildElement("test");
    QDomElement		filterElement	= testElement.firstChildElement("filter");

    // mapped number
    QDomNodeList	listNodes		= filterElement.elementsByTagName("list");

    // User can define only one filter type
    for (int i = 0; i < 1; i++)
    {
        QDomElement element			= listNodes.item(i).toElement();

        if (element.attribute("type")== "ignore")
            pDatasetConfig->m_testFilter.addFilter(element.text(), GS::Gex::TestFilter::filterIgnore);
        else if (element.attribute("type") == "select")
            pDatasetConfig->m_testFilter.addFilter(element.text(), GS::Gex::TestFilter::filterSelect);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readPartFilterSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read part filter section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::readPartFilterSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    QDomElement		partElement		= domElement.firstChildElement("part");
    QDomElement		filterElement	= partElement.firstChildElement("filter");

    // filtered values
    QDomNodeList	ValueNodes		= filterElement.elementsByTagName("value");

    // User can define only one filter type
    for (int i = 0; i < 1; i++)
    {
        QDomElement element				= ValueNodes.item(i).toElement();
        QDomElement testElement			= element.firstChildElement("test");
        QDomElement lowValueElement		= element.firstChildElement("LL");
        QDomElement highValueElement	= element.firstChildElement("HL");

        if (element.attribute("type") == "over")
        {
            if (testElement.isNull() == false && lowValueElement.isNull() == false)
                pDatasetConfig->m_partFilter.addFilter(testElement.text().toLong(), lowValueElement.text().toDouble(), 0, CGexPartFilter::filterOver);
        }
        else if (element.attribute("type") == "under")
        {
            if (testElement.isNull() == false && highValueElement.isNull() == false)
                pDatasetConfig->m_partFilter.addFilter(testElement.text().toLong(), 0, highValueElement.text().toDouble(), CGexPartFilter::filterUnder);
                }else if(element.attribute("type") == "inside")
                {
                        if (testElement.isNull() == false && highValueElement.isNull() == false && lowValueElement.isNull() == false)
                                pDatasetConfig->m_partFilter.addFilter(testElement.text().toLong(), lowValueElement.text().toDouble(), highValueElement.text().toDouble(), CGexPartFilter::filterInside);

                }else if(element.attribute("type") == "outside")
                {
                        if (testElement.isNull() == false && highValueElement.isNull() == false)
                                pDatasetConfig->m_partFilter.addFilter(testElement.text().toLong(), lowValueElement.text().toDouble(), highValueElement.text().toDouble(), CGexPartFilter::filterOutside);
                }
    }
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readTestCreateSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read create test section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::readTestToCreateSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    QDomElement	testElement = domElement.firstChildElement("test");

    // create tests
    QDomNodeList testToCreateNodes = testElement.elementsByTagName("create");

    for (int i = 0; i < testToCreateNodes.size(); i++)
    {
        QDomElement element			= testToCreateNodes.item(i).toElement();
        QDomElement numberElement	= element.firstChildElement("number");
        QDomElement nameElement		= element.firstChildElement("name");
        QDomElement unitElement		= element.firstChildElement("units");
        QDomElement lowLimitElement	= element.firstChildElement("LL");
        QDomElement highLimitElement= element.firstChildElement("HL");
        QDomElement formulaElement	= element.firstChildElement("formula");

        GexTestToCreate* ptGexTestToCreate = new GexTestToCreate();
        ptGexTestToCreate->setTestName(nameElement.text());
        ptGexTestToCreate->setTestNumber(numberElement.text());
        ptGexTestToCreate->setTestUnit(unitElement.text());
        ptGexTestToCreate->setTestLowLimit(lowLimitElement.text());
        ptGexTestToCreate->setTestHighLimit(highLimitElement.text());
        ptGexTestToCreate->setFormula(formulaElement.text());

        pDatasetConfig->m_testToCreateList.addTestToCreate(ptGexTestToCreate);
    }
}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void write(CGexDatasetConfig * pDatasetConfig)
//
// Description	:	write config in a txt file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::write(CGexDatasetConfig * pDatasetConfig)
{
    QDomDocument	xmlDocument;

    // Write a comment to describe format file
    writeComment(xmlDocument);

    // Write all config data
    QDomElement		rootElement = xmlDocument.createElement("gex_dataset_config");

    rootElement.setAttribute("version", "1.0");
    xmlDocument.appendChild(rootElement);

    QDomElement testElement = xmlDocument.createElement("test");
    rootElement.appendChild(testElement);

    if (pDatasetConfig->m_testToCreateList.m_lstTestToCreate.size() != 0)
        writeTestToCreateSection(xmlDocument, testElement, pDatasetConfig);

    if (pDatasetConfig->m_testMapping.isActivated())
        writeTestMappingSection(xmlDocument, testElement, pDatasetConfig);

    if (pDatasetConfig->m_testFilter.isActivated())
        writeTestFilterSection(xmlDocument, testElement, pDatasetConfig);

    QDomElement partElement = xmlDocument.createElement("part");
    rootElement.appendChild(partElement);

    if (pDatasetConfig->m_partFilter.isActivated())
        writePartFilterSection(xmlDocument, partElement, pDatasetConfig);

    QFile file(m_strFileName);

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream txtStream(&file);

        xmlDocument.save(txtStream, 4);

        file.close();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writeComment(QDomDocument& xmlDocument)
//
// Description	:	write a comment to describe file format
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writeComment(QDomDocument& xmlDocument)
{
    QString strComment;

    strComment =	"\n Quantix Examinator dataset config file\n\n";
    strComment +=	" V1.0 of the dataset config XML file allows you to:\n";
    strComment +=	"		o map tests/parameters (number, name)\n";
    strComment +=	"		o filter tests/parameters (by number)\n";
    strComment +=	"		o filter parts (by value)\n\n";
    strComment +=	"	Format (for mapping test numbers):\n";
    strComment +=	"		<test>\n";
    strComment +=	"			<mapping>\n";
    strComment +=	"				<number type = \"test#\">\n";
    strComment +=	"					<list>1000-1002</list>\n";
    strComment +=	"					<value>5000</value>\n";
    strComment +=	"				</number>\n";
    strComment +=	"			</mapping>\n";
    strComment +=	"		</test>\n\n";
    strComment +=	"	Format (for adding an offset to each test number in a list):\n";
    strComment +=	"		<test>\n";
    strComment +=	"			<mapping>\n";
    strComment +=	"				<number type = \"offset\">\n";
    strComment +=	"					<list>1000-1002</list>\n";
    strComment +=	"					<value>1000</value>\n";
    strComment +=	"				</number>\n";
    strComment +=	"			</mapping>\n";
    strComment +=	"		</test>\n\n";
    strComment +=	"	Format (for mapping test names):\n";
    strComment +=	"		<test>\n";
    strComment +=	"			<mapping>\n";
    strComment +=	"				<number>\n";
    strComment +=	"					...\n";
    strComment +=	"				</number>\n";
    strComment +=	"				<name>\n";
    strComment +=	"					<old>Old test name</old>\n";
    strComment +=	"					<new>New test name</new>\n";
    strComment +=	"				</name>\n";
    strComment +=	"				<name>\n";
    strComment +=	"					<function>RegExp((.*)_tm(.*),\1_uf\2)</function>\n";
    strComment +=	"				</name>\n";
    strComment +=	"			</mapping>\n";
    strComment +=	"		</test>\n\n";
    strComment +=	"	Characters \"<\" and \"&\" are strictly illegal in XML\n";
    strComment +=	"	If you place a character like \"<\" or \"&\" inside a test name, it will generate an XML error.\n";
    strComment +=	"	Following test name will generate an XML error:\n";
    strComment +=	"		<old>glxy_SS_IH     <> glxy_pin2</old>\n";
    strComment +=	"	to avoid this error, replace the \"<\" character with an entity reference:\n";
    strComment +=	"		<old>glxy_SS_IH     &lt;> glxy_pin2</old>\n\n";
    strComment +=	"	Replace \"<\" character with \"&lt;\" \n";
    strComment +=	"	Replace \"&\" character with \"&amp;\" \n\n";
    strComment +=	"	format (for filtering test numbers):\n";
    strComment +=	"		<test>\n";;
    strComment +=	"			<mapping>\n";
    strComment +=	"			.....\n";
    strComment +=	"			</mapping>\n";
    strComment +=	"			<filter>\n";
    strComment +=	"				<list type = \"ignore\">1000-1050</list>	(ignores specified tests)\n";
    strComment +=	"				OR\n";
    strComment +=	"				<list type = \"select\">1000-1050</list>	(keeps only specified tests)\n";
    strComment +=	"			</filter>\n";
    strComment +=	"		</test>\n\n";
    strComment +=	"	format (for filtering parts):\n";
    strComment +=	"		<part>\n";
    strComment +=	"			<filter>\n";
    strComment +=	"				<value type = \"over\">\n";
    strComment +=	"					<test>1000</test>\n";
    strComment +=	"					<LL>-0.5</LL>\n";
    strComment +=	"				</value>\n";
    strComment +=	"			</filter>\n";
    strComment +=	"		</part>\n\n\n";
    strComment +=	"	Replace the following lines with your own list of tests/Parameters\n";

    QDomComment	comment = xmlDocument.createComment(strComment);

    xmlDocument.appendChild(comment);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writeTestMappingSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read mapping section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writeTestMappingSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    QDomElement mappingElement = xmlDocument.createElement("mapping");

    // Write test number mapping
    writeTestNumberMapping(xmlDocument, mappingElement, pDatasetConfig);

    // Write test name mapping
    writeTestNameMapping(xmlDocument, mappingElement, pDatasetConfig);

    // Add mapping element to the xml doc
    if (mappingElement.isNull() == false)
        domElement.appendChild(mappingElement);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writeTestFilterSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read test section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writeTestFilterSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    if (pDatasetConfig->m_testFilter.isActivated())
    {
        QDomElement filterElement	= xmlDocument.createElement("filter");
        QDomElement listElement		= xmlDocument.createElement("list");
        QDomText	listText		= xmlDocument.createTextNode(pDatasetConfig->m_testFilter.GetRangeList());

        switch (pDatasetConfig->m_testFilter.GetType())
        {
            case GS::Gex::TestFilter::filterIgnore	:	listElement.setAttribute("type", "ignore");
                                                        break;

            case GS::Gex::TestFilter::filterSelect	:	listElement.setAttribute("type", "select");
                                                        break;

            default                                 :	GSLOG(SYSLOG_SEV_WARNING, "Invalid type of test filter");
                                                        break;
        }

        listElement.appendChild(listText);
        filterElement.appendChild(listElement);

        domElement.appendChild(filterElement);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writePartFilterSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read test section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writePartFilterSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    if (pDatasetConfig->m_partFilter.isActivated())
    {
        QDomElement filterElement		= xmlDocument.createElement("filter");
        QDomElement valueElement		= xmlDocument.createElement("value");
        QDomElement overValueElement	= xmlDocument.createElement("LL");;
        QDomElement underValueElement	= xmlDocument.createElement("HL");
        QDomElement testElement			= xmlDocument.createElement("test");
        QDomText	testText			= xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->testNumber()));
        QDomText	overValueText;
        QDomText	underValueText;

        switch (pDatasetConfig->m_partFilter.partFilterPrivate()->type())
        {
            case CGexPartFilter::filterOver		:	valueElement.setAttribute("type", "over");
                                                    overValueText = xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->lowValue()));
                                                    break;

            case CGexPartFilter::filterUnder	:	valueElement.setAttribute("type", "under");
                                                    underValueText = xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->highValue()));
                                                                                                        break;
                        case CGexPartFilter::filterInside	:	valueElement.setAttribute("type", "inside");
                                                                        overValueText = xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->lowValue()));
                                                                        underValueText = xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->highValue()));
                                                                        break;
                        case CGexPartFilter::filterOutside	:	valueElement.setAttribute("type", "outside");
                                                                        overValueText = xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->lowValue()));
                                                                        underValueText = xmlDocument.createTextNode(QString::number(pDatasetConfig->partFilter().partFilterPrivate()->highValue()));
                                                                        break;
            default								:	break;
        }

        testElement.appendChild(testText);
        valueElement.appendChild(testElement);

        if (overValueText.isNull() == false)
        {
            overValueElement.appendChild(overValueText);
            valueElement.appendChild(overValueElement);
        }
        if (underValueText.isNull() == false)
        {
            underValueElement.appendChild(underValueText);
            valueElement.appendChild(underValueElement);
        }

        filterElement.appendChild(valueElement);

        domElement.appendChild(filterElement);
    }
}




///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writeTestToCreateSection(QDomDocument& xmlDocument, QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	write test section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writeTestToCreateSection(QDomDocument& xmlDocument,
                                                      QDomElement& domElement,
                                                      CGexDatasetConfig * pDatasetConfig)
{
    QList<GexTestToCreate *> lstTestToCreate = pDatasetConfig->m_testToCreateList.m_lstTestToCreate;
    for (QList<GexTestToCreate *>::iterator it = lstTestToCreate.begin(); it != lstTestToCreate.end(); ++it)
    {
        QDomElement newElement          = xmlDocument.createElement("test");
        QDomElement createElement       = xmlDocument.createElement("create");
        QDomElement numberElement       = xmlDocument.createElement("number");
        QDomElement nameElement         = xmlDocument.createElement("name");
        QDomElement unitElement         = xmlDocument.createElement("units");;
        QDomElement lowLimitElement     = xmlDocument.createElement("LL");
        QDomElement highLimitElement    = xmlDocument.createElement("HL");
        QDomElement formulaElement      = xmlDocument.createElement("formula");


        QDomText	numberText	= xmlDocument.createTextNode((*it)->number());
        QDomText	nameText    = xmlDocument.createTextNode((*it)->name()) ;
        QDomText	unitText    = xmlDocument.createTextNode((*it)->unit());
        QDomText	LLText      = xmlDocument.createTextNode((*it)->lowLimit());
        QDomText	HLText      = xmlDocument.createTextNode((*it)->highLimit());
        QDomText	formulaText = xmlDocument.createTextNode((*it)->formula());


        numberElement.appendChild(numberText);
        nameElement.appendChild(nameText);
        unitElement.appendChild(unitText);
        lowLimitElement.appendChild(LLText);
        highLimitElement.appendChild(HLText);
        formulaElement.appendChild(formulaText);

        createElement.appendChild(numberElement);
        createElement.appendChild(nameElement);
        createElement.appendChild(unitElement);
        createElement.appendChild(lowLimitElement);
        createElement.appendChild(highLimitElement);
        createElement.appendChild(formulaElement);

        newElement.appendChild(createElement);
        domElement.appendChild(newElement);
   }
}




















///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readTestNumberMapping(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read test number mapping section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::readTestNumberMapping(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    // mapped number
    QDomNodeList	numberNodes		= domElement.elementsByTagName("number");

    for (int i = 0; i < numberNodes.size(); i++)
    {
        QDomElement element			= numberNodes.item(i).toElement();
        QDomElement listElement		= element.firstChildElement("list");
        QDomElement valueElement	= element.firstChildElement("value");

        if (listElement.isNull() == false && valueElement.isNull() == false)
        {
            if (element.attribute("type") == "offset")
                pDatasetConfig->m_testMapping.addMappedNumber(listElement.text(), valueElement.text().toInt(), CGexTestMappingNumber::typeOffset);
            else if (element.attribute("type") == "test#")
                pDatasetConfig->m_testMapping.addMappedNumber(listElement.text(), valueElement.text().toInt(), CGexTestMappingNumber::typeNumber);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readTestNameMapping(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
//
// Description	:	read test name mapping section in xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::readTestNameMapping(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig)
{
    QDomNodeList	nameNodes		= domElement.elementsByTagName("name");

    for (int i = 0; i < nameNodes.size(); i++)
    {
        QDomElement element		= nameNodes.item(i).toElement();

        QDomElement functionElement = element.firstChildElement("function");

        if (functionElement.isNull() == false)
        {
            QStringList functions;

            while(functionElement.isNull() == false)
            {
                functions.append(functionElement.text());

                functionElement = functionElement.nextSiblingElement("function");
            }

            pDatasetConfig->m_testMapping.addMappedName(functions);
        }
        else
        {
            QDomElement oldElement	= element.firstChildElement("old");
            QDomElement newElement	= element.firstChildElement("new");

            if (oldElement.isNull() == false && newElement.isNull() == false)
                pDatasetConfig->m_testMapping.addMappedName(oldElement.text(), newElement.text());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writeTestNumberMapping(QDomDocument &xmlDocument, QDomElement &mappingElement, CGexDatasetConfig *pDatasetConfig)
//
// Description	:	write test number mapping section into xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writeTestNumberMapping(QDomDocument &xmlDocument, QDomElement &mappingElement, CGexDatasetConfig *pDatasetConfig)
{
    for (int nIndex = 0; nIndex < pDatasetConfig->m_testMapping.lstMappedTestNumber().size(); nIndex++)
    {
        QDomElement numberElement = xmlDocument.createElement("number");

        switch (pDatasetConfig->m_testMapping.lstMappedTestNumber().at(nIndex)->type())
        {
            case CGexTestMappingNumber::typeOffset	:	numberElement.setAttribute("type", "offset");
                                                        break;

            case CGexTestMappingNumber::typeNumber	:	numberElement.setAttribute("type", "test#");
                                                        break;

            default									:	break;
        }

        QDomElement listElement		= xmlDocument.createElement("list");
        QDomElement valueElement	= xmlDocument.createElement("value");
        QDomText	listText		= xmlDocument.createTextNode(pDatasetConfig->m_testMapping.lstMappedTestNumber().at(nIndex)->testRange().GetRangeList());
        QDomText	valueText		= xmlDocument.createTextNode(QString::number(pDatasetConfig->m_testMapping.lstMappedTestNumber().at(nIndex)->value()));

        listElement.appendChild(listText);
        valueElement.appendChild(valueText);
        numberElement.appendChild(listElement);
        numberElement.appendChild(valueElement);

        mappingElement.appendChild(numberElement);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void writeTestNameMapping(QDomDocument &xmlDocument, QDomElement &mappingElement, CGexDatasetConfig *pDatasetConfig)
//
// Description	:	write test name mapping section into xml file
//
///////////////////////////////////////////////////////////////////////////////////
void CGexXmlDatasetConfigIO::writeTestNameMapping(QDomDocument &xmlDocument, QDomElement &mappingElement, CGexDatasetConfig *pDatasetConfig)
{
    for (int index = 0; index < pDatasetConfig->m_testMapping.lstMappedTestName().count(); ++index)
    {
        QDomElement nameElement = xmlDocument.createElement("name");

        if (pDatasetConfig->m_testMapping.lstMappedTestName().at(index)->type() == CGexTestMappingName::typeString)
        {
            QDomElement oldElement	= xmlDocument.createElement("old");
            QDomElement newElement	= xmlDocument.createElement("new");

            CGexTestMappingNameString * pMapping = (CGexTestMappingNameString *) pDatasetConfig->m_testMapping.lstMappedTestName().at(index);

            if (pMapping)
            {
                QDomText	oldText		= xmlDocument.createTextNode(pMapping->GetOldTestName());
                QDomText	newText		= xmlDocument.createTextNode(pMapping->GetNewTestName());

                oldElement.appendChild(oldText);
                newElement.appendChild(newText);
                nameElement.appendChild(oldElement);
                nameElement.appendChild(newElement);

                mappingElement.appendChild(nameElement);
            }
        }
        else if (pDatasetConfig->m_testMapping.lstMappedTestName().at(index)->type() == CGexTestMappingName::typeFunction)
        {
            CGexTestMappingNameFunction *   pMapping        = (CGexTestMappingNameFunction *) pDatasetConfig->m_testMapping.lstMappedTestName().at(index);

            if (pMapping)
            {
                for (int indexFunction = 0; indexFunction < pMapping->GetFunctions().count(); ++indexFunction)
                {
                    QDomElement functionElement	= xmlDocument.createElement("function");
                    QDomText	functionText	= xmlDocument.createTextNode(pMapping->GetFunctions().at(indexFunction));

                    functionElement.appendChild(functionText);
                    nameElement.appendChild(functionElement);
                }

                mappingElement.appendChild(nameElement);
            }
        }
    }
}

void CGexXmlDatasetConfigIO::readTestToUpdateSection(const QDomElement& domElement, CGexDatasetConfig * pDatasetConfig){

    QDomElement	testElement = domElement.firstChildElement("test");

    // create tests
    QDomNodeList testToUpdateNodes = testElement.elementsByTagName("update");

    for (int i = 0; i < testToUpdateNodes.size(); i++)
    {
        QDomElement element			= testToUpdateNodes.item(i).toElement();
        QString strUpdateSrc        = element.attribute("source");

        QDomElement numberElement	= element.firstChildElement("number");
        QString strData = numberElement.text();
        if(!numberElement.isNull())
            strData = numberElement.text();

        QDomElement nameElement		= element.firstChildElement("name");
        if(!nameElement.isNull())
            strData = nameElement.text();

        QDomElement lowLimitElement	= element.firstChildElement("LL");
        if(!lowLimitElement.isNull())
            strData = lowLimitElement.text();

        QDomElement highLimitElement= element.firstChildElement("HL");
        if(!highLimitElement.isNull())
            strData = highLimitElement.text();



        GexTestToUpdate* ptGexTestToUpdate = new GexTestToUpdate();
        ptGexTestToUpdate->setTestName(nameElement.text());
        ptGexTestToUpdate->setSource(strUpdateSrc);
        ptGexTestToUpdate->setTestNumber(numberElement.text());
        ptGexTestToUpdate->setTestLowLimit(lowLimitElement.text());
        ptGexTestToUpdate->setTestHighLimit(highLimitElement.text());

        pDatasetConfig->m_testToUpdateList.addTestToUpdate(ptGexTestToUpdate);
    }

}
