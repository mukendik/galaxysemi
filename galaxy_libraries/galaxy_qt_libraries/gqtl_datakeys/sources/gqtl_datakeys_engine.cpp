///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QDir>
#include <QTextStream>
#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////////
// GS Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_log.h>

#include "gqtl_datakeys_engine.h"
#include "gqtl_datakeys_content.h"
#include "gqtl_datakeys_file.h"
#include "gqtl_datakeys.h"

namespace GS
{
namespace QtLib
{

struct DatakeysEnginePrivate
{
    bool                            mUseDbKeysContent;
    DatakeysEngine::TestCondOrigin  mTestCondOrigin;    ///< stores test conditions origin: NONE, DTR or gexdbkeys File
    DatakeysFile                    mDbKeysFile;
    DatakeysContent                 mDbKeysContent;

    inline DatakeysEnginePrivate()
        : mUseDbKeysContent(true),
          mTestCondOrigin(DatakeysEngine::NONE)  {}

    inline DatakeysEnginePrivate(const DatakeysContent & dbKeysContent)
        : mUseDbKeysContent(true),
          mTestCondOrigin(DatakeysEngine::NONE),
          mDbKeysContent(dbKeysContent) {}
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	DatakeysEngine
//
// Description	:	The DatakeysEngine class provides an environment for evaluating DbKeys expression
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
DatakeysEngine::DatakeysEngine(QObject* parent)
    : QObject(parent), mPrivate(new DatakeysEnginePrivate)
{
}

DatakeysEngine::DatakeysEngine(const GS::QtLib::DatakeysContent & dbKeysContent)
    : mPrivate(new DatakeysEnginePrivate(dbKeysContent))
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
DatakeysEngine::~DatakeysEngine()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////
bool DatakeysEngine::loadConfigDbKeysFile(const QString &configFileName, int& lineError, QString &error)
{
    if (mPrivate)
    {
        QList<GS::QtLib::DatakeysError> lErrors;

        // Keep name of the config file used
        mPrivate->mDbKeysFile = DatakeysFile(configFileName);

        if (mPrivate->mDbKeysFile.Read(lErrors) == false)
        {
            if (lErrors.isEmpty() == false)
            {
                lineError   = lErrors.at(0).mLine;
                error       = lErrors.at(0).mMessage;
            }

            return false;
        }

        DatakeysInfo dynamicKey;
        // Init dynamic db keys with no value
        for (int idx = 0; idx < mPrivate->mDbKeysFile.CountDynamicKeys(); ++idx)
        {
            if (mPrivate->mDbKeysFile.GetDynamicKeysAt(dynamicKey, idx))
            {
                mPrivate->mDbKeysContent.SetDbKeyContent(dynamicKey.mKeyName, "");
                QRegExp lTestCondRx(REGEXP_TEST_CONDITION_KEY, Qt::CaseInsensitive);
                if (lTestCondRx.exactMatch(dynamicKey.mKeyName))
                    mPrivate->mTestCondOrigin = DatakeysEngine::CONFIG_FILE;
            }
        }

        return true;
    }

    error = "Unstable dbKeys engine";
    return false;
}

bool DatakeysEngine::evaluateStaticDbKeys(bool &validationFail,
                                              int &lineError, QString &error)
                                                const
{
    if (mPrivate)
    {
        int         lStaticFlowCount   = mPrivate->mDbKeysFile.CountStaticKeys();
        DatakeysInfo  lKeyInfo;

        for(int idx = 0; idx < lStaticFlowCount; ++idx)
        {
            if (mPrivate->mDbKeysFile.GetStaticKeysAt(lKeyInfo, idx))
            {
                // Evaluation the dbKey, and update the dbKey content
                // with the new result
                if (evaluateDbKeys(lKeyInfo.mKeyName, lKeyInfo.mKeyExpression, validationFail, error) == false)
                {
                    lineError = lKeyInfo.mLine;
                    return false;
                }
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Index %1 is out of bound for Static Keys")
                      .arg(idx).toLatin1().constData());
                return false;
            }
        }
        return true;
    }

    return false;
}

bool DatakeysEngine::evaluateDynamicDbKeys(int &lineError,
                                               QString &error) const
{
    lineError = 0;
    if (mPrivate)
    {
        bool        lValidationFail      = false;
        int         lDynamicFlowCount   = mPrivate->mDbKeysFile.CountDynamicKeys();
        DatakeysInfo  lKeyInfo;
        QRegExp     testConditionRegExp(REGEXP_ATTR_TEST_CONDITION, Qt::CaseInsensitive);

        // Reset dynamic keys
        for(int idx = 0; idx < lDynamicFlowCount; ++idx)
        {
            if (mPrivate->mDbKeysFile.GetDynamicKeysAt(lKeyInfo, idx))
            {
                // Reset dynamic key first.
                if (testConditionRegExp.exactMatch(lKeyInfo.mKeyName.trimmed()))
                    mPrivate->mDbKeysContent.SetDbKeyContent(lKeyInfo.mKeyName.trimmed(), "");
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Index %1 is out of bound for Dynamic Keys")
                      .arg( idx).toLatin1().constData());
                return false;
            }
        }

        for(int idx = 0; idx < lDynamicFlowCount; ++idx)
        {
            if (mPrivate->mDbKeysFile.GetDynamicKeysAt(lKeyInfo, idx))
            {
                // Evaluation the dbKey, and update the dbKey content
                // with the new result
                if (evaluateDbKeys(lKeyInfo.mKeyName, lKeyInfo.mKeyExpression, lValidationFail, error) == false)
                {
                    lineError = lKeyInfo.mLine;
                    return false;
                }
            }
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Index %1 is out of bound for Dynamic Keys")
                      .arg( idx).toLatin1().constData());
                return false;
            }
        }

        return true;
    }

    return false;
}

DatakeysContent &DatakeysEngine::dbKeysContent()
{
    return mPrivate->mDbKeysContent;
}

QString DatakeysEngine::configFileName() const
{
    return mPrivate->mDbKeysFile.GetFileName();
}

int DatakeysEngine::GetCountDynamicKeys() const
{
    return mPrivate->mDbKeysFile.CountDynamicKeys();
}

DatakeysEngine::TestCondOrigin DatakeysEngine::GetTestConditionsOrigin() const
{
    return mPrivate->mTestCondOrigin;
}

void DatakeysEngine::SetTestConditionsOrigin(DatakeysEngine::TestCondOrigin origin)
{
    mPrivate->mTestCondOrigin = origin;
}

bool DatakeysEngine::evaluateDbKeys(const QString &dbKeyName,
                                           const QString &dbKeyExpression,
                                           bool &validationFail,
                                           QString &error) const
{
    // Copy with section or regexp selection
    // Key1, Key2
    // Key1, Key2.Section()
    // Key1, Key2.RegExp()
    // Concat with section or regexp selection
    // Key1, +Key2
    // Key1, +Key2.Section()
    // Key1, +Key2.RegExp()

    bool    concat          = false;
    bool    match           = false;
    bool    validResult     = false;
    bool    status          = false;
    QString dbKeyTemp;
    QString expression;
    QString result;

    // Verify if it's concat expression
    if(dbKeyExpression.startsWith("+"))
    {
        concat = true;
        expression = dbKeyExpression.mid(1).trimmed();
    }
    else
        expression = dbKeyExpression;

    //
    status = parseValue(expression, result, dbKeyTemp, match, validResult, error);

    // Check if no error syntax
    if(!status)
        return false;

    // If section "Validation", RegExp have to match else Validation_Error
    if(dbKeyName.compare("Validation", Qt::CaseInsensitive) == 0)
    {
        if(!match)
        {
            validationFail = true;
            error.sprintf("regexp doesn't match (for %s = %s)", expression.toLatin1().constData(), dbKeyTemp.toLatin1().constData());
            return false;
        }
        else
            return true;
    }

    // Case 4544: do not affect key if invalid result retrieved
    if(!validResult)
        return true;

    // Check if literal string or substring to extract from strValue.
    // Check which field to overload
    if(concat)
    {
        QString dbKeyValue;
        mPrivate->mDbKeysContent.GetDbKeyContent(dbKeyName, dbKeyValue);

        // Have to additionate Value and Result if INT
        bool	bValueIsInt;
        bool    bResultIsInt;

        dbKeyValue.toInt(&bValueIsInt);
        result.toInt(&bResultIsInt);

        if(bValueIsInt && bResultIsInt)
        {
            int	nValue = dbKeyValue.toInt() + result.toInt();
            result = QString::number(nValue);
        }
        else
            result = dbKeyValue + result;
    }

    // Case 4544: we have to set the key even if the expression does not match, given the retrieved result is valid
    if(mPrivate->mDbKeysContent.SetDbKeyContent(dbKeyName, result) == false)
    {
        error = mPrivate->mDbKeysContent.Get("Error").toString();
        if (error.isEmpty())
            error = QString("Invalid Key %1 or Invalid Value %2").arg(dbKeyName).arg(result);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Case 4544: add bValidResult and strKeyValue parameters
///////////////////////////////////////////////////////////
// Overload MIR type fields from data file and/or file name.
///////////////////////////////////////////////////////////
// Return value
//	true if no error, false else (invalid expression, invalid key...)
///////////////////////////////////////////////////////////
// Input
//	pKeys			: ptr on keys structure
//	strExpression	: expression to parse. Valid expressions are:
//						<parameter>.Section(<section definition>)					=> Facility.Section(%3_)
//						<parameter>.Regexp(<regexp>[,<true_value>[,<false_value>]]) => FileName.RegExp(\w+_ft_(\d+).*,\1)
//						<parameter>.Date(<QT date format specifier>)				=> StartTime.Date(yyyy hh:ss:mm)
//						<parameter>
///////////////////////////////////////////////////////////
// Output
//	strResult		: result of the expression
//	strKeyValue		: value of <parameter>
//	bMatch			: true if expression matched, false else
//	bValidResult	: true if strResult is valid and should be used, false else
//	strError		: error string if any
///////////////////////////////////////////////////////////
bool DatakeysEngine::parseValue(const QString &expression, QString& result,
                                       QString& keyValue, bool &isMatching,
                                       bool& isValidResult, QString &error) const
{
    // Init output variables
    isMatching   = true;
    isValidResult= false;
    result       = expression;

    // Check type of expression parser to use
    if(expression.indexOf(".Section",false) >= 0)
    {
        // string like: 'Lot.Section(1-10)' or 'Facility.Section(%3_)'
        isValidResult = parseValueFieldSection(expression, result,
                                              keyValue, error);
        return isValidResult;
    }

    if(expression.indexOf(".RegExp",false) >= 0)
    {
        // string like: 'Lot.RegExp(Pattern,IfMatch,IfNotMatch)'
        bool lStatus = parseValueFieldRegExp(expression, result,
                              keyValue, isMatching,
                              isValidResult, error,
                              QRegExp::RegExp);
        return lStatus;
    }

    if(expression.indexOf(".Date",false) >= 0)
    {
        // string like: 'StartTime.Date(QT Date format)'
        isValidResult = parseValueDate(expression,result,keyValue,error);
        return isValidResult;
    }

    // We have a direct expression, which could be either a constant (ie TrackingLot, ABC), or a dbkey variable (ie TrackingLot, Lot)
    // In either cases, the result is valid
    isValidResult = true;

    if(mPrivate->mUseDbKeysContent && !mPrivate->mDbKeysContent.GetDbKeyContent(expression, keyValue))
        result = expression;
    else
        result = keyValue;

    return true;
}

///////////////////////////////////////////////////////////
// Overload MIR fields parsing Field section
///////////////////////////////////////////////////////////
bool DatakeysEngine::parseValueFieldSection(const QString &strExpression,
                                                   QString &strResult,
                                                   QString &strKeyValue,
                                                   QString &strError)  const
{
    strResult = strExpression;

    // Extract the field to parse (the 'xx' string in 'xx.Section()'
    int iIndex = strExpression.indexOf('.');
    if(iIndex <=0)
    {
        strError = "missing '.'";
        return false;
    }

    QString strKey;
    strKey = strExpression.left(iIndex).trimmed();

    // Check which value is in field to overload
    if(!mPrivate->mDbKeysContent.GetDbKeyContent(strKey, strKeyValue))
    {
        strError.sprintf("unknown meta-data (%s)", strKey.toLatin1().constData());
        return false;
    }

    // Skip 'xxx.Section(' string
    iIndex = strExpression.indexOf('(')+1;
    if(iIndex <=0)
    {
        strError = "missing '('";
        return false;
    }

    // Extract string: '1-10)' or '%3_)'
    strResult = strExpression.mid(iIndex).trimmed();
    if(strResult.isEmpty())
    {
        strError = "invalid section format";
        return false;
    }

    // Check if parsing is based on string offset or sub-strings
    if(strResult[0] == '%')
    {
        // We have to parse the Nth sub-string in the file name
        // Extract Sub-string# and sub-string separator
        char cSeparator;
        if(sscanf(strResult.toLatin1().constData(),"%*c%d%c",&iIndex,&cSeparator) != 2)
        {
            strError = "invalid section format";
            return false;
        }

        // Extract sub-string.
        strResult = strKeyValue.section(cSeparator,iIndex-1,iIndex-1);
    }
    else
    {
        // We have to extract a sub-string based on character position offsets
        int iFrom,iTo;
        if(sscanf(strResult.toLatin1().constData(),"%d%*c%d",&iFrom,&iTo) != 2)
        {
            strError = "invalid section format";
            return false;
        }

        // Extract sub-string.
        strResult = strKeyValue.mid(iFrom-1,1+iTo-iFrom);
    }
    return true;
}

///////////////////////////////////////////////////////////
// Overload MIR fields parsing Field section
///////////////////////////////////////////////////////////
bool DatakeysEngine::parseValueDate(const QString &strExpression,
                                           QString &strResult,
                                           QString &strKeyValue,
                                           QString &strError)  const
{
    // Init result with a default string
    strResult = "nodate";

    // Extract the field to parse (the 'xx' string in 'xx.Date()'
    int iIndex = strExpression.indexOf('.');
    if(iIndex <=0)
    {
        strError = "missing '.'";
        return false;
    }

    QString strKey;
    QString strDateFormat;
    strKey = strExpression.left(iIndex).trimmed();

    // Check which value is in field to overload
    if(mPrivate->mUseDbKeysContent && !mPrivate->mDbKeysContent.GetDbKeyContent(strKey, strKeyValue))
    {
        strError.sprintf("unknown meta-data (%s)", strKey.toLatin1().constData());
        return false;
    }

    // Make sure the field is a date field
    if((strKey.toLower() != "starttime") && (strKey.toLower() != "setuptime") && (strKey.toLower() != "finishtime"))
    {
        strError.sprintf("invalid meta-data for date function (%s)", strKey.toLatin1().constData());
        return false;
    }

    // Get argument of the Date function 'xxx.Date(arg)' string
    iIndex = strExpression.indexOf('(')+1;
    if(iIndex <=0)
    {
        strError = "missing '('";
        return false;
    }
    strDateFormat = strExpression.mid(iIndex).trimmed();
    iIndex = strDateFormat.indexOf(')');
    if(iIndex <=0)
    {
        strError = "missing ')'";
        return false;
    }
    strDateFormat = strDateFormat.left(iIndex).trimmed();

    // Get the date
    bool			bOK;
    unsigned int	uiTimestamp;
    QDateTime		clDateTime;
    uiTimestamp = strKeyValue.toUInt(&bOK);
    if(!bOK)
    {
        strError.sprintf("field %s doesn't contain a valid timestamp (%s)", strKey.toLatin1().constData(), strKeyValue.toLatin1().constData());
        return false;
    }
    clDateTime.setTime_t(uiTimestamp);
    strResult = clDateTime.toString(strDateFormat);

    return true;
}

///////////////////////////////////////////////////////////
// Case 4544: add bValidResult and strKeyValue parameters and review rules
///////////////////////////////////////////////////////////
// Overload MIR fields parsing Field section
///////////////////////////////////////////////////////////
// Return value
//	true if no error, false else (invalid expression, invalid key...)
///////////////////////////////////////////////////////////
// Input
//	pKeys           : ptr on keys structure
//	strExpression   : expression to parse. Valid expressions are:
//                      <parameter>.Section(<section definition>)					=> Facility.Section(%3_)
//                      <parameter>.Regexp(<regexp>[,<true_value>[,<false_value>]]) => FileName.RegExp(\w+_ft_(\d+).*,\1)
//                      <parameter>.Date(<QT date format specifier>)				=> StartTime.Date(yyyy hh:ss:mm)
//                      <parameter>
///////////////////////////////////////////////////////////
// Output
//	strResult       : result of the expression
//	bMatch          : true if expression matched, false else
//	bValidResult    : true if strResult is valid and should be used, false else
//	strError        : error string if any
///////////////////////////////////////////////////////////
// General form of the expression: <Parameter>.RegExp(<regexp>,<true_expression>,<false_expression>)
// Following rules apply:
//	1)	General expression wrong (no ".", no "("...)
//		strResult="", bMatch=false, bValidResult=false, strError=<error msg>
//		return false
//	2)	Empty regular expression (ie FileName.Regexp())
//		strResult="", bMatch=false, bValidResult=false, strError=<error msg>
//		return false
//	3)	Invalid regular expression (ie FileName.Regexp(final|qagate1(,0))
//		strResult="", bMatch=false, bValidResult=false, strError=<error msg>
//		return false
//	4)	Expression matches, no <true_expression> specified
//		strResult=value of <Parameter>, bMatch=true, bValidResult=true, strError=""
//		return true
//	5)	Expression matches, <true_expression> is an empty string
//		strResult="", bMatch=true, bValidResult=true, strError=""
//		return true
//	6)	Expression matches, <true_expression> is a valid key
//		strResult=<key> value, bMatch=true, bValidResult=true, strError=""
//		return true
//	7)	Expression matches, <true_expression> is a constant string
//		strResult=<true_expression>, bMatch=true, bValidResult=true, strError=""
//		return true
//	8)	Expression matches, <true_expression> is a capture
//		strResult=<capture> if any, "" else, bMatch=true, bValidResult=true, strError=""
//		return true
//	9)	Expression does not match, no <false_expression> specified
//		strResult="", bMatch=false, bValidResult=false, strError=""
//		return true
//	10)	Expression does not match, <false_expression> is an empty string
//		strResult="", bMatch=false, bValidResult=true, strError=""
//		return true
//	11) Expression does not match, <false_expression> is a valid key
//		strResult=<key> value, bMatch=false, bValidResult=true, strError=""
//		return true
//	12)	Expression does not match, <false_expression> is a constant string
//		strResult=<false_expression>, bMatch=false, bValidResult=true, strError=""
//		return true
//	13)	Expression does not match, <false_expression> is a capture
//		strResult=<capture> if any, "" else, bMatch=false, bValidResult=true, strError=""
//		return true
///////////////////////////////////////////////////////////
bool DatakeysEngine::parseValueFieldRegExp(const QString &strExpression,
                                                  QString &strResult,
                                                  QString &strKeyValue,
                                                  bool &bMatch,
                                                  bool &bValidResult,
                                                  QString &strError,
                                                  QRegExp::PatternSyntax PatternSyntax
                                               )  const
{
    // Init output variables. Will be overwritten if they should be different.
    bMatch = bValidResult = false;
    strResult = strError = "";

    // Extract the field to parse (the 'xx' string in 'xx.RegExp()'
    int iIndex = strExpression.indexOf(".RegExp",0,Qt::CaseInsensitive);
    if(iIndex <=0)
    {
        strError = "missing '.'";
        return false;
    }

    QString strKey;
    strKey = strExpression.left(iIndex).trimmed();

    // Check which value is in field to overload
    if(mPrivate->mUseDbKeysContent &&
            !mPrivate->mDbKeysContent.GetDbKeyContent(strKey, strKeyValue))
    {
        strError.sprintf("unknown key (%s)", strKey.toLatin1().constData());
        return false;
    }

    // Make sure we have a '(' after the .RegExp
    iIndex = strExpression.indexOf("(",iIndex)+1;
    if(iIndex <=0)
    {
        strError = "missing '('";
        return false;
    }

    // Skip 'xxx.RegExp(' string, and make sure we have a ')' at the end
    QString strRegExp = strExpression.mid(iIndex).trimmed();
    if(!strRegExp.endsWith(")"))
    {
        strError = "missing ')'";
        return false;
    }

    // Remove ending ')' and trim
    strRegExp.chop(1);
    strRegExp = strRegExp.trimmed();

    // If empty expression, returned result will be empty, but valid, and matching
    if(strRegExp.isEmpty())
    {
        strError = "empty regular expression";
        return false;
    }

    // Extract string: expression, 'Y','N'
    // Lot.RegExp(reg_expression, true_expression, false_expression)
    // Lot.RegExp("reg_expression", "true_expression", "false_expression")
    // " is obligatory if an expression contains ',' then when find " have to find the next "
    // remove encaplsuled " or ' after extraction

    // extract Pattern for regular expression
    int                 iPos;
    QRegExp             qRegExp("", Qt::CaseInsensitive, PatternSyntax);
    QRegExp             qRegExpPattern("", Qt::CaseInsensitive, PatternSyntax);
    QString             regExpMatch, strPattern;
    QMap<int,QString>   mapRegExp;

    // Extract all parameters
    iPos = 0;
    while(!strRegExp.isEmpty())
    {
        // Encapsuled String with " or '

        if(strRegExp.startsWith("\"") || strRegExp.startsWith("'"))
        {
            // Encapsuled string
            QString strStringPattern;
            iIndex = 0;
            if(strRegExp.startsWith("\""))
            {
                strStringPattern = "\"([^\"]*)\"";
                iIndex = 2;
            }
            if(strRegExp.startsWith("'"))
            {
                strStringPattern = "'([^']*)'";
                iIndex = 2;
            }
            if(!strStringPattern.isEmpty())
                qRegExp.setPattern(strStringPattern);

            if(qRegExp.indexIn(strRegExp)>=0)
            {
                mapRegExp[iPos] = qRegExp.cap(1);
                strRegExp = strRegExp.mid(mapRegExp[iPos].length()+iIndex).trimmed();
                strRegExp = strRegExp.section(",",1).trimmed();
            }
            else
            {
                mapRegExp[iPos] = strRegExp.section(",",0,0).trimmed();
                strRegExp = strRegExp.section(",",1).trimmed();
            }
        }
        else
        {
            int iNextComma=0;
            strPattern = strRegExp.section(",",0,0).trimmed();
            qRegExp.setPattern(strPattern);
            while(!qRegExp.isValid())
            {
                iNextComma++;
                if(strPattern == strRegExp.section(",",0,iNextComma).trimmed())
                    break;

                strPattern = strRegExp.section(",",0,iNextComma).trimmed();
                qRegExp.setPattern(strPattern);
            }
            mapRegExp[iPos] = strRegExp.section(",",0,iNextComma).trimmed();
            strRegExp = strRegExp.section(",",iNextComma+1).trimmed();
        }
        mapRegExp[iPos] = mapRegExp[iPos].trimmed();

        // Check for empty string value
        if(mapRegExp[iPos] == "\"\"" || mapRegExp[iPos] == "''")
            mapRegExp[iPos] = "";

        qRegExp.setPattern(mapRegExp[iPos]);
        if((iPos==0) && (!qRegExp.isValid()))
        {
            strError = QString("invalid regular expression (%1): %2").
                    arg(mapRegExp[iPos]).
                    arg(qRegExp.errorString());
            return false;
        }

        iPos++;
    }

    // Apply ExpPattern and extract the result
    // mapRegExp[0] =  regular expression
    // mapRegExp[1] =  value to use if expression matches
    // mapRegExp[2] =  value to use if expression doesn't match

//    strExpression = strKey; // Is it usefull?
    strPattern = mapRegExp[0];

    // Check which regular expression is in a key
    if(mPrivate->mUseDbKeysContent && mPrivate->mDbKeysContent.GetDbKeyContent(strPattern, strRegExp))
        strPattern = strRegExp;

    // Set pattern for regular expression
    qRegExpPattern.setPattern(strPattern);

    // Check match
    bMatch = qRegExpPattern.exactMatch(strKeyValue);
    if(bMatch)
    {
        // Expression is matching, check if we have a <true_expression>
        if(!mapRegExp.contains(1))
        {
            // No <true_expression>, use <Parameter> value
            strResult = strKeyValue;
            bValidResult = true;
            return true;
        }

        // We have a <true_expression>, but we don't know its value yet, except if empty string
        regExpMatch = mapRegExp[1];
        if(regExpMatch.isEmpty())
        {
            bValidResult = true;
            return true;
        }
    }
    else
    {
        // Expression is not matching, check if we have a <false_expression>
        if(!mapRegExp.contains(2))
            // No valid result, key will not be oveloaded
            return true;

        // We have a <false_expression>, but we don't know its value yet, except if empty string
        regExpMatch = mapRegExp[2];
        if(regExpMatch.isEmpty())
        {
            bValidResult = true;
            return true;
        }
    }

    // Check if any captures specified
    qRegExp.setPattern("\\\\(\\d+)");
    if(qRegExp.indexIn(regExpMatch)<0)
    {
        // No capture from RegExp
        // Check if it is a GexDb key
        if(mPrivate->mUseDbKeysContent && !mPrivate->mDbKeysContent.GetDbKeyContent(regExpMatch, strResult))
            strResult = regExpMatch;
        bValidResult = true;
        return true;
    }

    // Retrieve captures
    QString  lRegExpMatch = regExpMatch;
    int iCap;
    strResult = "";
    while(qRegExp.indexIn(lRegExpMatch)>=0)
    {
        iPos = qRegExp.indexIn(lRegExpMatch);
        strResult+=lRegExpMatch.left(iPos);
        iCap = qRegExp.cap(1).toInt();
        strResult += qRegExpPattern.cap(iCap);
        lRegExpMatch = lRegExpMatch.mid(iPos+1+QString::number(iCap).length());
    }
    strResult+=lRegExpMatch;
    strResult = strResult.trimmed();

    // If Match and have a Capture
    // Check if have the same result with QRegExp::RegExp2
    QString lFirstResult = strResult;
    QRegExp::PatternSyntax lPatternSyntax;
    if(PatternSyntax == QRegExp::RegExp)
        lPatternSyntax = QRegExp::RegExp2;
    else
        lPatternSyntax = QRegExp::RegExp;

    qRegExpPattern.setPattern(strPattern);
    qRegExpPattern.setPatternSyntax(lPatternSyntax);
    qRegExpPattern.exactMatch(strKeyValue);
    strResult = "";
    lRegExpMatch = regExpMatch;
    while(qRegExp.indexIn(lRegExpMatch)>=0)
    {
        iPos = qRegExp.indexIn(lRegExpMatch);
        strResult+=lRegExpMatch.left(iPos);
        iCap = qRegExp.cap(1).toInt();
        strResult += qRegExpPattern.cap(iCap);
        lRegExpMatch = lRegExpMatch.mid(iPos+1+QString::number(iCap).length());
    }
    strResult+=lRegExpMatch;
    strResult = strResult.trimmed();

    if(lFirstResult != strResult)
    {
        strError = QString("the regular expression in %1 applied to %2 is ambiguous."
                        " Possible values: %3 or %4").
                arg(strExpression).
                arg(strKeyValue).
                arg(lFirstResult).
                arg(strResult);
        return false;
    }

    bValidResult = true;
    return true;
}

///////////////////////////////////////////////////////////
// Erase a file-specific config file (if exists)
///////////////////////////////////////////////////////////
bool DatakeysEngine::evaluateDbKeys(DatakeysContent &dbKeysContent, const QString &dbKeyName, const QString &dbKeyExpression, QString &error)
{
    DatakeysEngine  dbKeysEngine(dbKeysContent);
    QString             dbKeyValue;
    bool                validationFail = false;

    if(dbKeysEngine.evaluateDbKeys(dbKeyName, dbKeyExpression, validationFail, error) &&
        dbKeysEngine.dbKeysContent().GetDbKeyContent(dbKeyName, dbKeyValue))
    {
        return dbKeysContent.SetDbKeyContent(dbKeyName, dbKeyValue);
    }

    if(dbKeysEngine.evaluateDbKeys(dbKeyName, dbKeyExpression, validationFail, error))
    {
        if (!validationFail && dbKeyName.compare("Validation", Qt::CaseInsensitive) == 0)
            return true;
        else if (dbKeysEngine.dbKeysContent().GetDbKeyContent(dbKeyName, dbKeyValue))
        {
            return dbKeysContent.SetDbKeyContent(dbKeyName, dbKeyValue);
        }
    }

    return false;
}

bool DatakeysEngine::evaluateExpression(const QString &strExpression,
                                            QString& keyValue,
                                            QString &strResult,
                                            bool &bMatch,
                                            bool &bValidResult,
                                            QString &strError)
{
    DatakeysEngine  dbKeysEngine(NULL);

    // Do not use DbKeyContent here
    dbKeysEngine.mPrivate->mUseDbKeysContent = false;

    return dbKeysEngine.parseValue(strExpression, strResult, keyValue, bMatch, bValidResult, strError);
}

///////////////////////////////////////////////////////////
// Erase a file-specific config file (if exists)
///////////////////////////////////////////////////////////
void DatakeysEngine::deleteConfigDbKeysFile()
{
    if (mPrivate)
    {
        QFileInfo   fileInfo(mPrivate->mDbKeysFile.GetFileName());

        // Do not remove default config file!
        if (DatakeysContent::isDefaultConfigFile(fileInfo.fileName()) == false)
        {
            // Erase file-specific configuration file.
            if(fileInfo.exists())
            {
                if (!QFile::remove(fileInfo.absoluteFilePath()))
                    GSLOG(SYSLOG_SEV_WARNING, QString("Unable to remove %1").
                          arg(fileInfo.absoluteFilePath()).
                          toLatin1().constData());
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Rename a file-specific config file (if exists)
///////////////////////////////////////////////////////////
void DatakeysEngine::renameConfigDbKeysFile(const QString& newExtension)
{
    if(mPrivate)
    {
        QFileInfo   fileInfo(mPrivate->mDbKeysFile.GetFileName());

        // Do not rename default config file!
        if (DatakeysContent::isDefaultConfigFile(fileInfo.fileName()) == false)
        {
            // Rename file-specific configuration file. if has not already the extension
            if(fileInfo.exists() && !fileInfo.absoluteFilePath().endsWith(newExtension))
            {
                if (!QFile::rename(fileInfo.absoluteFilePath(),
                            fileInfo.absoluteFilePath() + "." +newExtension))
                    GSLOG(SYSLOG_SEV_WARNING,
                          QString("Unable to rename %1 to %2").
                          arg(fileInfo.absoluteFilePath()).
                          arg(fileInfo.absoluteFilePath() +
                          "." + newExtension).
                          toLatin1().constData());
            }
        }
    }
}

/******************************************************************************!
 * \fn moveConfigDbKeysFile
 * \brief Move a file-specific config file (if exists)
 ******************************************************************************/
void DatakeysEngine::moveConfigDbKeysFile(const QString& destDir)
{
    if (mPrivate)
    {
        QString configFileName(mPrivate->mDbKeysFile.GetFileName());
        if (configFileName.isEmpty())
        {
            configFileName = mPrivate->mDbKeysContent.
                Get("fulldestinationname").toString() + ".gexdbkeys";
        }
        QFileInfo fileInfo(configFileName);
        QString   newFilePath;

        // Do not move default config file
        if (! DatakeysContent::isDefaultConfigFile(fileInfo.fileName()))
        {
            newFilePath = destDir + QDir::separator() + fileInfo.fileName();

            // Delete destination file in case it already exists
            if (QFile::exists(newFilePath))
            {
                if (! QFile::remove(newFilePath))
                {
                    GSLOG(SYSLOG_SEV_WARNING, QString("Unable to remove %1").
                          arg(newFilePath).toLatin1().constData());
                }
            }

            // Erase file-specific configuration file
            if (fileInfo.exists())
            {
                if (! QFile::rename(fileInfo.absoluteFilePath(), newFilePath))
                {
                    GSLOG(SYSLOG_SEV_WARNING,
                          QString("Unable to rename %1 to %2").
                          arg(fileInfo.absoluteFilePath()).
                          arg(newFilePath).toLatin1().constData());
                }
            }
//            else
//            {
////                GSLOG(SYSLOG_SEV_DEBUG, QString("No '%1' to rename").
////                       arg(fileInfo.filePath()));
//                QStringList dbKeysList(mPrivate->mDbKeysContent.toList());
//                QStringList::const_iterator iter;
//                for (iter  = dbKeysList.begin();
//                     iter != dbKeysList.end(); ++iter)
//                {
//                    GSLOG(SYSLOG_SEV_DEBUG, QString("dbKeysList: ") + *iter);
//                }
//            }
        }
    }
}

bool DatakeysEngine::findConfigDbKeysFile(QString &configFileName,
                                            DatakeysContent& keyContent,
                                            const QString &configFolder)
{
    // 0) Use ConfigFileName if not empty
    configFileName = keyContent.Get("ConfigFileName").toString();

    // Config file found?
    if(QFile::exists(configFileName))
        return true;

    // 1) Check if file-specific configuration file
    // name: <datafile>.gexdbkeys
    configFileName = keyContent.Get("SourceArchive").toString() + ".gexdbkeys";

    // Config file found?
    if(QFile::exists(configFileName))
        return true;

    // 2) If no file-specific, check for a product linked configuration file in folder
    // Build file name: <data file path>/<product_name>.gexdbkeys
    configFileName = QDir::cleanPath(QFileInfo(keyContent.Get("SourceArchive").toString()).
                        absolutePath() +
                        QDir::separator() +
                        keyContent.Get("Product").toString() +
                        ".gexdbkeys");

    // Config file found?
    if(QFile::exists(configFileName))
        return true;

    // 3) If no product-specific, check for a global configuration file in folder
    // name: "config.gexdbkeys"
    QFileInfo configFile(keyContent.Get("SourceArchive").toString());

    configFileName = QDir::cleanPath(configFile.absolutePath() +
                        QDir::separator() +
                        DatakeysContent::defaultConfigFileName());

    // Config file found?
    if(QFile::exists(configFileName))
        return true;

    // 4) If no file-specific and no global configuration file in folder,
    // check for a global configuration file in task spooling dir
    if(!configFolder.isEmpty())
    {
        // Build file name: <Mo Task spooling dir>/config.gexdbkeys
        configFileName = QDir::cleanPath(configFolder +
                            QDir::separator() +
                            keyContent.Get("Product").toString() +
                            ".gexdbkeys");
        // Config file found?
        if(QFile::exists(configFileName))
            return true;

        configFileName = QDir::cleanPath(configFolder +
                            QDir::separator() +
                            DatakeysContent::defaultConfigFileName());

        // Config file found?
        if(QFile::exists(configFileName))
            return true;
    }

    // No config file found
    configFileName.clear();
    return false;
}

} //END namespace QtLib
} //END namespace GS

