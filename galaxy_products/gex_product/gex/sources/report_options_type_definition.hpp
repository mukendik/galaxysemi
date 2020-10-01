#ifndef REPORT_OPTIONS_TYPE_DEFINITION_HPP
#define REPORT_OPTIONS_TYPE_DEFINITION_HPP

/*!
 * \file report_options_type_definition.hpp
 * \author PYC
 * \date 07 february 2011
 */


#include <QtCore>
#include <QtXml>

/*! \class OptionsTypeDefinition
 * \brief gex_options.xml handler
 *
 *
 * 1/ has to validate SetOptions(..) arguments from gex_options.xml file
 * \n
 * 2/ has to assume some option attribute accesses (defaulValue, write_csl, ...)
 *
 */

namespace GS
{
namespace Gex
{

class OptionsTypeDefinition
{
public:
    OptionsTypeDefinition();
    OptionsTypeDefinition(QFile qfOptionFile);
    OptionsTypeDefinition(const QString strOptionFilePath);
    OptionsTypeDefinition(const OptionsTypeDefinition& other);

    // operator(s)
    OptionsTypeDefinition& operator=(const OptionsTypeDefinition& other);

    // accessor (s)
    /*!
      *	\brief Option default value accessor
      *
      *	\param strSection	: option section
      *	\param strField		: option field
      *
      *	\return depend on option type and option node 'defaultvalue' attribute value
      *
      */
    QString getDefaultOptionValue(const QString strSection, const QString strField) const;

    /*!
      *	\brief Option type accessor
      *
      *	\param strSection	: option section
      *	\param strField		: option field
      *
      *	\return depend on option 'type' attribute
      *
      */
    QString getOptionType(const QString strSection, const QString strField) const;


    /*!
      *	\brief Option write_csl accessor
      *
      *	\param strSection	: option section
      *	\param strField		: option field
      *
      *	\return true if write_csl option attribute (java script expression) return true
      *
      */
    bool toSaveOption(const QString strSection, const QString strField) const;

    /*!
      *	\brief	accessor used to validate class instance able to be used
      *	\return true if ready to be used (xml option file well loaded ...)
      */
    bool isReady(void) const;

    /*!
      *	\brief option map accessor
      *	\return a map with option lists associated with corresponding sections
      */
    QMap< QString, QStringList > getOptionMap() const;

    // validators
    /*!
      *	\brief SetOption() method validator
      *
      *	\param strSection	: option section
      *	\param strField		: option field
      *	\param strValue		: value to set
      *
      *	\return true if value is in keeping with option definition
      *
      */
    bool isValidSetOption(const QString strSection, const QString strField, const QString strValue) const;

private:
    // attributes
    QMap<QString, QMap<QString, QDomElement> >					m_qmOptionTypeMap;		/*!< option xml description map*/

    QMap<QString, bool(OptionsTypeDefinition::*)(const QDomElement, const QString) const >	m_qmSpecificOptionValidators;	/*!< internal specific validators (functions) map */
    QMap<QString, QString(OptionsTypeDefinition::*)(const QDomElement) const > m_qmSpecificGetExceptionDefaultValue;	/*!< internal specific getDefaulValue (functions) map */

    bool m_bIsReady;		/*!< true if ready to be used (xml option file well loaded ...) */

    // internal validators
    bool isDefineOption(const QString strSection, const QString strField) const;
    bool isValidValue(const QString strSection, const QString strField, const QString strValue) const;
    // internal specific validators
    bool isValid_Group(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Int(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Float(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Enum(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Flag(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Bool(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_String(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Color(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Font(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_HTML(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    bool isValid_Path(const QDomElement qdeOptionElement, const QString strOptionValue) const;
    // internal specific get default value accessors
    QString getExceptionDefaultValue_Group(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Int(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Float(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Enum(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Flag(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Bool(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_String(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Color(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Font(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_HTML(const QDomElement qdeOptionNode) const;
    QString getExceptionDefaultValue_Path(const QDomElement qdeOptionNode) const;


    // internal methods
    void clear();
    bool loadOptionFile(QFile* qfOptionFile);
    bool loadOptionFile(const QString strOptionFilePath);
    bool parseOptionFileElement(const QDomElement qdeParsedElement);
    void initializeTypeList();

    // internal accessors
    bool setOptionTypeDefinition(const QString strSection, const QString strField, const QDomElement qdeOptionNode);
    QDomElement getOptionDomElement(const QString strSection, const QString strField) const;
};

}   // namespace Gex
}   // namespace GS

#endif // REPORT_OPTIONS_TYPE_DEFINITION_HPP
