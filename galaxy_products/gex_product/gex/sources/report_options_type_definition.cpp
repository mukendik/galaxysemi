#include "report_options_type_definition.hpp"
#include <gqtl_log.h>
#include "gex_scriptengine.h"

extern GexScriptEngine* pGexScriptEngine;		// global Gex QScriptEngine

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////
///	"OptionsTypeDefinition" class
///////////////////////////////////////////////////

///////////////////////////////////////////////
/// constructor(s) / destructor
OptionsTypeDefinition::OptionsTypeDefinition()
{
    clear();
}

OptionsTypeDefinition::OptionsTypeDefinition(QFile qfOptionFile)
{
    clear();
    loadOptionFile(&qfOptionFile);
}

OptionsTypeDefinition::OptionsTypeDefinition(const QString strOptionFilePath)
{
    clear();
    loadOptionFile(strOptionFilePath);
}

// copy constructor
OptionsTypeDefinition::OptionsTypeDefinition(const OptionsTypeDefinition& other)
{
    clear();
    *this = other;
}



// accessors
QString OptionsTypeDefinition::getDefaultOptionValue(const QString strSection, const QString strField) const
{
    QDomElement qdeConsidearatedOption = getOptionDomElement(strSection, strField);
    QString strOptionDefaultValue = qdeConsidearatedOption.attribute(QString("defaultvalue"));
    QString strOptionType = qdeConsidearatedOption.attribute(QString("type"));


    if (!pGexScriptEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Invalid script engine");
        GEX_ASSERT(false);
        strOptionDefaultValue = QString();		// Default default value will be used at the end of method
    }

    if (!strOptionDefaultValue.isEmpty())
    {
        QScriptValue qsvScriptValue=pGexScriptEngine->evaluate(strOptionDefaultValue);
        if ( (qsvScriptValue.isError()) || (!qsvScriptValue.isValid()) || pGexScriptEngine->hasUncaughtException())
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("error while evaluating javascript expression for option: section='%1', field='%2', value='%3'")
                  .arg(strSection).arg(strField).arg(strOptionDefaultValue).toLatin1().constData());
            return QString();
        }
        else
        {
            return qsvScriptValue.toString();
        }
    }

    // unexpected output
    GSLOG( SYSLOG_SEV_INFORMATIONAL, QString("No default value for option (section='%1', field='%2')")
           .arg(strSection).arg(strField).toLatin1().constData());

    QString(OptionsTypeDefinition::*exceptionDefaultValue_specificFunction)(const QDomElement) const = m_qmSpecificGetExceptionDefaultValue.value(strOptionType);
    QString strDefaultValue = (*this.*exceptionDefaultValue_specificFunction)(qdeConsidearatedOption);

    return strDefaultValue;
}

QString OptionsTypeDefinition::getOptionType(const QString strSection, const QString strField) const
{
    QDomElement qdeConsidearatedOption = getOptionDomElement(strSection, strField);
    return qdeConsidearatedOption.attribute(QString("type"));
}

bool OptionsTypeDefinition::toSaveOption(const QString strSection, const QString strField) const
{
    QDomElement qdeConsidearatedOption = getOptionDomElement(strSection, strField);
    QString strSaveAttributeValue = qdeConsidearatedOption.attribute(QString("write_csl"));


    if (!pGexScriptEngine)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Invalid script engine");
        GEX_ASSERT(false);		// option will be saved
    }
    else
    {
        if (!strSaveAttributeValue.isEmpty())
        {
            QScriptValue r=pGexScriptEngine->evaluate(strSaveAttributeValue);
            if (r.isError() || pGexScriptEngine->hasUncaughtException())
            {
                GSLOG(SYSLOG_SEV_WARNING, QString("error while evaluating javascript expression : '%1' : %2")
                      .arg(strSaveAttributeValue)
                      .arg(pGexScriptEngine->uncaughtException().toString()).toLatin1().data());
                // GEX_ASSERT(false); it is possible that some JS variables are not defined.
            }
            else if (r.toBool()== false)
            {
                return false;
            }
        }
    }

    // default value is save option !
    return true;
}

bool OptionsTypeDefinition::isReady(void) const
{
    return m_bIsReady;
}


// validators
bool OptionsTypeDefinition::isValidSetOption(const QString strSection, const QString strField, const QString strValue) const
{
    if( (strSection.isEmpty())		||
        (strField.isEmpty())		)
        return false;

    if(!isDefineOption(strSection, strField))
        return false;

    if(!isValidValue(strSection, strField, strValue))
        return false;

    // enjoy ! everything seems ok !
    return true;
}



// internal validators
bool OptionsTypeDefinition::isDefineOption(const QString strSection, const QString strField) const
{
    QDomElement qdeConsidearatedOption = getOptionDomElement(strSection, strField);

    if(qdeConsidearatedOption.isNull())
        return false;
    else
        return true;
}

bool OptionsTypeDefinition::isValidValue(const QString strSection, const QString strField, const QString strValue) const
{
    QDomElement qdeConsidearatedOption = getOptionDomElement(strSection, strField);
    QString strOptionType = qdeConsidearatedOption.attribute(QString("type"));

    bool(OptionsTypeDefinition::*isValid_SpecificFunction)(const QDomElement, const QString) const =
            m_qmSpecificOptionValidators.value(strOptionType);

    bool bValidationRslt = (*this.*isValid_SpecificFunction)(qdeConsidearatedOption, strValue);
    return bValidationRslt;
}



////////////////////////////////////
// internal specific validators
bool
OptionsTypeDefinition::isValid_Group(const QDomElement /*qdeOptionElement*/,
                                      const QString /*strOptionValue*/) const
{
    // 'Group' type option isn't a valid option to set
    return false;
}

bool OptionsTypeDefinition::isValid_Int(const QDomElement qdeOptionElement, const QString strOptionValue) const
{
    bool bIsValid = true;

    int nOptionValue = strOptionValue.toInt(&bIsValid);
    if(!bIsValid)
        return false;


    QString strMin, strMax;

    strMin = qdeOptionElement.attribute(QString("minimum"));
    strMax = qdeOptionElement.attribute(QString("maximum"));

    int nOptionMinValue = strMin.toInt(&bIsValid);
    if(!bIsValid)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid minimum attribute of int option '%1'")
              .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }
    int nOptionMaxValue = strMax.toInt(&bIsValid);
    if(!bIsValid)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid maximum attribute of int option '%1'")
              .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }

    if( (nOptionValue<nOptionMinValue) || (nOptionValue>nOptionMaxValue) )
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Invalid value (%1) for '%2' Int value")
              .arg(nOptionValue).arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        return false;
    }

    // everythings seems good !
    return true;
}

bool OptionsTypeDefinition::isValid_Float(const QDomElement qdeOptionElement, const QString strOptionValue) const
{
    bool bIsValid = true;

  int nOptionValue = static_cast<int>(strOptionValue.toFloat(&bIsValid));
    if(!bIsValid)
        return false;

    QString strMin, strMax;

    strMin = qdeOptionElement.attribute(QString("minimum"));
    strMax = qdeOptionElement.attribute(QString("maximum"));

  int nOptionMinValue = static_cast<int>(strMin.toFloat(&bIsValid));
    if(!bIsValid)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid minimum attribute of option '%1' ")
              .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }

  int nOptionMaxValue = static_cast<int>(strMax.toFloat(&bIsValid));
    if(!bIsValid)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid maximum attribute of option '%1'")
              .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }

    if( (nOptionValue<nOptionMinValue) || (nOptionValue>nOptionMaxValue) )
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Invalid value (%1) for '%2' Float value")
              .arg(nOptionValue).arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        return false;
    }

    // everythings seems good !
    return true;
}

bool OptionsTypeDefinition::isValid_Enum(const QDomElement qdeOptionElement, const QString strOptionValue) const
{
    QString strOptionPossibleCodingValues = qdeOptionElement.attribute(QString("codingvalues"));
    if(strOptionPossibleCodingValues.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid enum attribute of option '%1'")
              .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }

    QStringList qslOptionCodingValuesList = strOptionPossibleCodingValues.split(QString("|"));

    if( !qslOptionCodingValuesList.contains(strOptionValue) )
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Invalid value (%1) for '%2' Enum value")
              .arg(strOptionValue).arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        return false;
    }

    // everythings seems good !
    return true;
}

bool OptionsTypeDefinition::isValid_Flag(const QDomElement qdeOptionElement, const QString strOptionValue) const
{
    QString strOptionPossibleCodingValues = qdeOptionElement.attribute(QString("codingvalues"));
    if(strOptionPossibleCodingValues.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid flag attribute of option '%1'")
              .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }

    if(strOptionValue.isEmpty())
        return true;

    QStringList qslOptionCodingValuesList = strOptionPossibleCodingValues.split(QString("|"));
    QStringList qslOptionValuesList = strOptionValue.split(QString("|"));

    for(int ii=0; ii<qslOptionValuesList.count(); ii++)
    {
        QString strSpecificFlagValue = qslOptionValuesList.at(ii);
        if( !qslOptionCodingValuesList.contains(strSpecificFlagValue) )
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString("Invalid value (%1) for '%2' Flag value").arg(strSpecificFlagValue)
                  .arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
            return false;
        }
    }

    // everythings seems good !
    return true;
}

bool OptionsTypeDefinition::isValid_Bool(const QDomElement qdeOptionElement, const QString strOptionValue) const
{
    if( (strOptionValue!=QString("true")) && (strOptionValue!=QString("false")) )
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("Invalid value (%1) for '%2' Bool value")
              .arg(strOptionValue).arg(qdeOptionElement.attribute(QString("cslname"))).toLatin1().constData());
        return false;
    }

    // everythings seems good !
    return true;
}

bool
OptionsTypeDefinition::isValid_String(const QDomElement /*qdeOptionElement*/,
                                       const QString /*strOptionValue*/) const
{
    // nothing to check once you are here ...

    // everythings seems good !
    return true;
}

bool
OptionsTypeDefinition::isValid_Color(const QDomElement /*qdeOptionElement*/,
                                      const QString /*strOptionValue*/) const
{
    // not used yet 2011 02 04

    // everythings seems good !
    return true;
}

bool
OptionsTypeDefinition::isValid_Font(const QDomElement /*qdeOptionElement*/,
                                     const QString /*strOptionValue*/) const
{
    // not used yet 2011 02 04

    // everythings seems good !
    return true;
}

bool
OptionsTypeDefinition::isValid_HTML(const QDomElement /*qdeOptionElement*/,
                                     const QString /*strOptionValue*/) const
{
    // not used yet 2011 02 04

    // everythings seems good !
    return true;
}

bool
OptionsTypeDefinition::isValid_Path(const QDomElement /*qdeOptionElement*/,
                                     const QString /*strOptionValue*/) const
{
    // no specific validation assumed 2011 02 04

    // everythings seems good !
    return true;
}


///////////////////////////////////////////////////////
// internal specific get default value accessors
QString
OptionsTypeDefinition::
getExceptionDefaultValue_Group(const QDomElement /*qdeOptionNode*/) const
{return QString();} // Group isn't an option type

QString OptionsTypeDefinition::getExceptionDefaultValue_Int(const QDomElement qdeOptionNode) const
{
    QString strReturnValue = qdeOptionNode.attribute(QString("minimum"));
    return strReturnValue;
}

QString OptionsTypeDefinition::getExceptionDefaultValue_Float(const QDomElement qdeOptionNode) const
{
    QString strReturnValue = qdeOptionNode.attribute(QString("minimum"));
    return strReturnValue;
}

QString OptionsTypeDefinition::getExceptionDefaultValue_Enum(const QDomElement qdeOptionNode) const
{
    QString strOptionCodingValues = qdeOptionNode.attribute(QString("codingvalues"));
    QStringList qslOptionStringList = strOptionCodingValues.split(QString("|"));

    return qslOptionStringList.first();		// never empty, built from split method, at least contains empty string
}

QString
OptionsTypeDefinition::
getExceptionDefaultValue_Flag(const QDomElement /*qdeOptionNode*/) const
{
    return QString();
}

QString
OptionsTypeDefinition::
getExceptionDefaultValue_Bool(const QDomElement /*qdeOptionNode*/) const
{
    return QString("false");
}

QString
OptionsTypeDefinition::
getExceptionDefaultValue_String(const QDomElement /*qdeOptionNode*/) const
{
    return QString();
}

QString
OptionsTypeDefinition::
getExceptionDefaultValue_Color(const QDomElement /*qdeOptionNode*/) const
{
    return QString();
}		// not define 2011/02/07

QString
OptionsTypeDefinition::
getExceptionDefaultValue_Font(const QDomElement /*qdeOptionNode*/) const
{
    return QString();
}		// not define 2011/02/07

QString
OptionsTypeDefinition::
getExceptionDefaultValue_HTML(const QDomElement /*qdeOptionNode*/) const
{
    return QString();
}		// not define 2011/02/07

QString
OptionsTypeDefinition::
getExceptionDefaultValue_Path(const QDomElement /*qdeOptionNode*/) const
{
    return QString();
}		// not define 2011/02/07

////////////////////////////////
// internal methods
void OptionsTypeDefinition::clear()
{
    m_qmOptionTypeMap.clear();
    initializeTypeList();
    m_bIsReady = false;
}


bool OptionsTypeDefinition::parseOptionFileElement(const QDomElement qdeParsedElement)
{
    QDomNode qdnNode = qdeParsedElement.firstChild();

    while(!qdnNode.isNull())
    {
        QDomElement qdeElement = qdnNode.toElement(); // try to convert the node to an element.
        QDomElement qdeParentElement = (qdnNode.parentNode()).toElement();

        QString strParentElementCslName, strElementCslName;
        QString strParentElementType, strElementType;
        if(!qdeElement.isNull())
        {
            strElementCslName = qdeElement.attribute(QString("cslname"));
            strElementType = qdeElement.attribute(QString("type"));
        }
        if(!qdeParentElement.isNull())
        {
            strParentElementCslName = qdeParentElement.attribute(QString("cslname"));
            strParentElementType = qdeParentElement.attribute(QString("type"));
        }

        QStringList qslElementTypeList;
        qslElementTypeList << QString("Int") << QString("Float") << QString("Enum") << QString("Flag") << QString("Bool");
        qslElementTypeList << QString("String") << QString("Color") << QString("Font") << QString("HTML") << QString("Path");
        // << QString("Group") // not really an option type

        if (  (qdeElement.tagName()!= QString("option"))		||
              (strElementCslName == QString(""))				||		// check .isnull() method at the same time
              (!qslElementTypeList.contains(strElementType))	||
              (qdeParentElement.tagName()!=QString("option"))	||
              (strParentElementCslName == QString(""))			||
              (strParentElementType != QString("Group"))		)
        {
//            GSLOG(SYSLOG_SEV_DEBUG, QString("Option Type definition : ignoring xml element '%1' '%2' ").arg(
//                     strElementCslName.toLatin1().constData(),
//                     strParentElementCslName.toLatin1().constData() );
        }
        else
        {
            if(!setOptionTypeDefinition(strParentElementCslName, strElementCslName, qdeElement))
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Option Type definition : can't save xml element '%1' '%2' ")
                      .arg(strElementCslName).arg(strParentElementCslName).toLatin1().constData());
                return false;
            }
        }

        if (qdeElement.hasChildNodes())
        {
            if(!parseOptionFileElement(qdeElement))			// recursive call
                return false;
        }

        qdnNode = qdnNode.nextSibling();
    }


    // everything went well !!
    return true;
}


OptionsTypeDefinition& OptionsTypeDefinition::operator=(const OptionsTypeDefinition& other)
{
    if (this != &other)
    {
        m_qmOptionTypeMap = other.m_qmOptionTypeMap;
        m_bIsReady = other.m_bIsReady;
        initializeTypeList();
    }

    return *this;
}

void OptionsTypeDefinition::initializeTypeList()
{
    m_qmSpecificOptionValidators.clear();
    m_qmSpecificOptionValidators.insert( QString("Group"), &OptionsTypeDefinition::isValid_Group );
    m_qmSpecificOptionValidators.insert( QString("Int"), &OptionsTypeDefinition::isValid_Int );
    m_qmSpecificOptionValidators.insert( QString("Float"), &OptionsTypeDefinition::isValid_Float );
    m_qmSpecificOptionValidators.insert( QString("Enum"), &OptionsTypeDefinition::isValid_Enum );
    m_qmSpecificOptionValidators.insert( QString("Flag"), &OptionsTypeDefinition::isValid_Flag );
    m_qmSpecificOptionValidators.insert( QString("Bool"), &OptionsTypeDefinition::isValid_Bool );
    m_qmSpecificOptionValidators.insert( QString("String"), &OptionsTypeDefinition::isValid_String );
    m_qmSpecificOptionValidators.insert( QString("Color"), &OptionsTypeDefinition::isValid_Color );
    m_qmSpecificOptionValidators.insert( QString("Font"), &OptionsTypeDefinition::isValid_Font );
    m_qmSpecificOptionValidators.insert( QString("HTML"), &OptionsTypeDefinition::isValid_HTML );
    m_qmSpecificOptionValidators.insert( QString("Path"), &OptionsTypeDefinition::isValid_Path );

    m_qmSpecificGetExceptionDefaultValue.clear();
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Group"), &OptionsTypeDefinition::getExceptionDefaultValue_Group );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Int"), &OptionsTypeDefinition::getExceptionDefaultValue_Int );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Float"), &OptionsTypeDefinition::getExceptionDefaultValue_Float );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Enum"), &OptionsTypeDefinition::getExceptionDefaultValue_Enum );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Flag"), &OptionsTypeDefinition::getExceptionDefaultValue_Flag );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Bool"), &OptionsTypeDefinition::getExceptionDefaultValue_Bool );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("String"), &OptionsTypeDefinition::getExceptionDefaultValue_String );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Color"), &OptionsTypeDefinition::getExceptionDefaultValue_Color );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Font"), &OptionsTypeDefinition::getExceptionDefaultValue_Font );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("HTML"), &OptionsTypeDefinition::getExceptionDefaultValue_HTML );
    m_qmSpecificGetExceptionDefaultValue.insert( QString("Path"), &OptionsTypeDefinition::getExceptionDefaultValue_Path );
}


// internal accessors
bool OptionsTypeDefinition::setOptionTypeDefinition(const QString strSection, const QString strField, const QDomElement qdeOptionNode)
{
    if( (strSection.isEmpty()) || (strField.isEmpty()) || (qdeOptionNode.isNull()) )
    {
        GSLOG( SYSLOG_SEV_CRITICAL,
               QString("wrong call of setOptionTypeDefinition(): section='%1', field='%2', node='%3'")
               .arg(strSection).arg(strField).arg(qdeOptionNode.tagName()).toLatin1().constData());
        GEX_ASSERT(false);
        return false;
    }

    QMap<QString, QDomElement> qmFieldMap;
    if (m_qmOptionTypeMap.find(strSection)!=m_qmOptionTypeMap.end())
        qmFieldMap=m_qmOptionTypeMap.value(strSection);
    qmFieldMap.insert(strField, qdeOptionNode);
    m_qmOptionTypeMap.insert(strSection, qmFieldMap);

    // everything went well !!
    return true;
}


QDomElement OptionsTypeDefinition::getOptionDomElement(const QString strSection, const QString strField) const
{
    if( (strSection.isEmpty()) || (strField.isEmpty()) )
    {
        GSLOG( SYSLOG_SEV_ERROR, QString("wrong call of getOptionDomElement(): '%1', '%2'")
               .arg(strSection).arg(strField).toLatin1().constData());
        GEX_ASSERT(false);
        return QDomElement();
    }


    QMap<QString, QDomElement> qmFieldMap=m_qmOptionTypeMap.value(strSection);
    if (qmFieldMap.find(strField)==qmFieldMap.end())
        return QDomElement();

    QDomElement qdeSearchedElement(qmFieldMap.value(strField));

    return qdeSearchedElement;
}

QMap< QString, QStringList > OptionsTypeDefinition::getOptionMap() const
{
    QMap< QString, QStringList > qmMapToReturn;

    QList<QString > qlSectionList = m_qmOptionTypeMap.keys();
    QStringList qslOptionsList;
    QString strSection;

    QListIterator< QString > qliSectionIterator(qlSectionList);
    while(qliSectionIterator.hasNext())
    {
        strSection = qliSectionIterator.next();
        qslOptionsList = m_qmOptionTypeMap.value(strSection).keys();
        qmMapToReturn.insert(strSection, qslOptionsList);
    }

    return qmMapToReturn;
}


bool OptionsTypeDefinition::loadOptionFile(QFile* qfPtrOptionFile)
{
    if(!qfPtrOptionFile->exists())
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("%1 option file doesn't exist !! \n")
              .arg(qfPtrOptionFile->fileName()).toLatin1().constData());
        m_bIsReady = false;
        return false;
    }

    if(!qfPtrOptionFile->open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("can't open %1 file !! \n")
              .arg(qfPtrOptionFile->fileName()).toLatin1().constData());
        m_bIsReady = false;
        return false;
    }

    // file parsing
    QDomDocument qddDomDocument;
    QString		strError;
    int			nLine=0, nColumn=0;
    if(!qddDomDocument.setContent(qfPtrOptionFile, false, &strError, &nLine, &nColumn))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error loading %1 xml file (Error=%2; Line=%3; Column=%4) !! \n")
              .arg(qfPtrOptionFile->fileName()).arg(strError).arg(nLine).arg(nColumn).toLatin1().constData());
        qfPtrOptionFile->close();
        m_bIsReady = false;
        return false;
    }

    QDomElement qdeOptionRoot = qddDomDocument.documentElement();
    if(!parseOptionFileElement(qdeOptionRoot))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("can't parse %1 file !! \n")
              .arg(qfPtrOptionFile->fileName()).toLatin1().constData());
        qfPtrOptionFile->close();
        m_bIsReady = false;
        return false;
    }

    qfPtrOptionFile->close();

    // everything wen't well
    m_bIsReady = true;
    return true;
}

bool OptionsTypeDefinition::loadOptionFile(const QString strOptionFilePath)
{
    QFile qfOptionFile(strOptionFilePath);
    return loadOptionFile(&qfOptionFile);
}

}   // namespace Gex
}   // namespace GS
