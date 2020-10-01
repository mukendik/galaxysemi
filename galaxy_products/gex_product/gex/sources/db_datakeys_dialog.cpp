#include "db_datakeys_dialog.h"
#include <QFileDialog>
#include <QDateTime>
#include <QTextStream>

// Galaxy modules includes
#include <gqtl_skin.h>
#include <gqtl_log.h>

// report_build.cpp
extern CGexSkin*		pGexSkin;			// holds the skin settings

///////////////////////////////////////////////////////////
// Manage the Database Keys
///////////////////////////////////////////////////////////
GexDatabaseEditKeys::GexDatabaseEditKeys( QWidget* parent, bool modal, Qt::WindowFlags fl)
    : QDialog( parent, fl )
{
    setupUi(this);
    setModal(modal);

    // Set Examinator skin
    if (pGexSkin)
        pGexSkin->applyPalette(this);

    setWindowFlags(Qt::FramelessWindowHint);

    QObject::connect(PushButtonOk,					SIGNAL(clicked()),				this, SLOT(accept()));
    QObject::connect(pushButtonReloadDefault,		SIGNAL(clicked()),				this, SLOT(OnReloadDefault()));
    QObject::connect(pushButtonLoadFromConfigFile,	SIGNAL(clicked()),				this, SLOT(OnLoadFromFile()));
    QObject::connect(PushButtonCancel,				SIGNAL(clicked()),				this, SLOT(reject()));
    QObject::connect(lineEditDataOrigin,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditLotID,					SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditSublotID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditFamilyID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditProductID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditFacilityID,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditFreqStepID,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditProcessID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditFloorID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditPackage,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditLoadBoardID,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditProberID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditProgram,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditTemperature,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditTester,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditWaferID,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditLoadBoardType,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditProberType,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditProgramRevision,		SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditTestingCode,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditTesterType,			SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditBurnin,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditOperator,				SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditUser1,					SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditUser2,					SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditUser3,					SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditUser4,					SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));
    QObject::connect(lineEditUser5,					SIGNAL(textChanged(QString)),	this, SLOT(OnEdits(QString)));

    // At startup, No default to reload!
    pushButtonReloadDefault->setEnabled(false);
    bEdited = false;
}

///////////////////////////////////////////////////////////
// Load Keys into GUI
///////////////////////////////////////////////////////////
void	GexDatabaseEditKeys::setKeys(GS::QtLib::DatakeysContent *pKeys)
{
    if(pKeys == NULL)
        return;

    // File name
    textLabelDataFile->setText(pKeys->Get("FileName").toString());

    // General info
    lineEditDataOrigin->setText(pKeys->Get("DataOrigin").toString());
    lineEditFamilyID->setText(pKeys->Get("Family").toString());
    lineEditLotID->setText(pKeys->Get("Lot").toString());
    lineEditProductID->setText(pKeys->Get("Product").toString());
    lineEditSublotID->setText(pKeys->Get("SubLot").toString());

    // Manufacturing/Test info
    lineEditFacilityID->setText(pKeys->Get("Facility").toString());
    lineEditFloorID->setText(pKeys->Get("Floor").toString());
    lineEditFreqStepID->setText(pKeys->Get("FrequencyStep").toString());
    lineEditPackage->setText(pKeys->Get("PackageType").toString());
    lineEditProcessID->setText(pKeys->Get("Process").toString());

    // Test info
    lineEditLoadBoardID->setText(pKeys->Get("LoadBoard").toString());
    lineEditLoadBoardType->setText(pKeys->Get("LoadBoardType").toString());
    lineEditProberID->setText(pKeys->Get("ProberName").toString());
    lineEditProberType->setText(pKeys->Get("ProberType").toString());
    lineEditProgram->setText(pKeys->Get("ProgramName").toString());
    lineEditProgramRevision->setText(pKeys->Get("ProgramRevision").toString());
    lineEditTemperature->setText(pKeys->Get("Temperature").toString());
    lineEditTestingCode->setText(pKeys->Get("TestingCode").toString());
    lineEditTester->setText(pKeys->Get("TesterName").toString());
    lineEditTesterType->setText(pKeys->Get("TesterType").toString());
    lineEditWaferID->setText(pKeys->Get("Wafer").toString());
    comboBoxRetestNumber->setCurrentIndex(pKeys->Get("RetestIndex").toInt());	// Retest Lot instance

    // Misc, & User defined
    lineEditBurnin->setText(pKeys->Get("BurninTime").toString());
    lineEditOperator->setText(pKeys->Get("Operator").toString());
    lineEditUser1->setText(pKeys->Get("User1").toString());
    lineEditUser2->setText(pKeys->Get("User2").toString());
    lineEditUser3->setText(pKeys->Get("User3").toString());
    lineEditUser4->setText(pKeys->Get("User4").toString());
    lineEditUser5->setText(pKeys->Get("User5").toString());

    if(!pKeys->Get("User1Label").toString().isEmpty())
        textLabelUser1->setText(pKeys->Get("User1Label").toString());	// Label to use for 'User1' field
    if(!pKeys->Get("User2Label").toString().isEmpty())
        textLabelUser2->setText(pKeys->Get("User2Label").toString());	// Label to use for 'User2' field
    if(!pKeys->Get("User3Label").toString().isEmpty())
        textLabelUser3->setText(pKeys->Get("User3Label").toString());	// Label to use for 'User3' field
    if(!pKeys->Get("User4Label").toString().isEmpty())
        textLabelUser4->setText(pKeys->Get("User4Label").toString());	// Label to use for 'User4' field
    if(!pKeys->Get("User5Label").toString().isEmpty())
        textLabelUser5->setText(pKeys->Get("User5Label").toString());	// Label to use for 'User5' field


    // Save default keys
    cDefaultKeys = *pKeys;
}

///////////////////////////////////////////////////////////
// Get Keys from GUI
///////////////////////////////////////////////////////////
void	GexDatabaseEditKeys::getKeys(GS::QtLib::DatakeysContent *pKeys)
{
    if(pKeys == NULL)
        return;

    // General info
    pKeys->Set("DataOrigin", lineEditDataOrigin->text());
    pKeys->Set("Family", lineEditFamilyID->text());
    pKeys->Set("Lot", lineEditLotID->text());
    pKeys->Set("Product", lineEditProductID->text());
    pKeys->Set("SubLot", lineEditSublotID->text());

    // Manufacturing/Test info
    pKeys->Set("Facility", lineEditFacilityID->text());
    pKeys->Set("Floor", lineEditFloorID->text());
    pKeys->Set("FrequencyStep", lineEditFreqStepID->text());
    pKeys->Set("PackageType", lineEditPackage->text());
    pKeys->Set("Process", lineEditProcessID->text());

    // Test info
    pKeys->Set("LoadBoard", lineEditLoadBoardID->text());
    pKeys->Set("LoadBoardType", lineEditLoadBoardType->text());
    pKeys->Set("ProberName", lineEditProberID->text());
    pKeys->Set("ProberType", lineEditProberType->text());
    pKeys->Set("ProgramName", lineEditProgram->text());
    pKeys->Set("ProgramRevision", lineEditProgramRevision->text());
    pKeys->Set("Temperature", lineEditTemperature->text());
    pKeys->Set("TestingCode", lineEditTestingCode->text());
    pKeys->Set("TesterName", lineEditTester->text());
    pKeys->Set("TesterType", lineEditTesterType->text());
    pKeys->Set("Wafer", lineEditWaferID->text());
    pKeys->Set("RetestIndex", comboBoxRetestNumber->currentIndex());	// Retest Lot instance

    // Misc, & User defined
    pKeys->Set("BurninTime", lineEditBurnin->text());
    pKeys->Set("Operator", lineEditOperator->text());
    pKeys->Set("User1", lineEditUser1->text());
    pKeys->Set("User2", lineEditUser2->text());
    pKeys->Set("User3", lineEditUser3->text());
    pKeys->Set("User4", lineEditUser4->text());
    pKeys->Set("User5", lineEditUser5->text());
}

///////////////////////////////////////////////////////////
// A key field just got edited...
///////////////////////////////////////////////////////////
void GexDatabaseEditKeys::OnEdits(const QString& /*strNewString*/)
{
    // Allow to reload defaults!
    bEdited = true;
    pushButtonReloadDefault->setEnabled(true);
}

///////////////////////////////////////////////////////////
// Load Keys values from file...
///////////////////////////////////////////////////////////
void	GexDatabaseEditKeys::OnLoadFromFile(void)
{
    QFileDialog cFileOpen(this);
    QString strKeysFile = cFileOpen.getOpenFileName(NULL, "Database Keys configuration file", "", "*.gexdbkeys");

    if(strKeysFile.isEmpty() == true)
        return;	// No file selected...ignore task!

    // Allow to reload defaults!
    bEdited = true;
    pushButtonReloadDefault->setEnabled(true);
}

///////////////////////////////////////////////////////////
// Reload default values into Key's fields
///////////////////////////////////////////////////////////
void	GexDatabaseEditKeys::OnReloadDefault(void)
{
    // Reload default keys.
    setKeys(&cDefaultKeys);

    // Reset flags/GUI aspect.
    bEdited = false;
    pushButtonReloadDefault->setEnabled(false);
}

///////////////////////////////////////////////////////////
// Erase a file-specific config file (if exists)
///////////////////////////////////////////////////////////
void	GexDatabaseEditKeys::deleteConfigFileKeys(GS::QtLib::DatakeysContent *pKeys)
{
    if(pKeys == NULL)
        return;

    QFileInfo   fileInfo(pKeys->Get("ConfigFileName").toString());

    // Do not remove default config file!
    if (GS::QtLib::DatakeysContent::isDefaultConfigFile(fileInfo.fileName()))
        return;

    // Erase file-specific configuration file.
    if(fileInfo.exists())
    {
        if (!QFile::remove(fileInfo.absoluteFilePath()))
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to remove %1")
                   .arg(fileInfo.absoluteFilePath())
                  .toLatin1().constData());
    }
}

///////////////////////////////////////////////////////////
// Rename a file-specific config file (if exists)
///////////////////////////////////////////////////////////
void GexDatabaseEditKeys::renameConfigFileKeys(GS::QtLib::DatakeysContent *pKeys
                                               , const QString& newExtension)
{
    if(pKeys == NULL)
        return;

    QFileInfo   fileInfo(pKeys->Get("ConfigFileName").toString());

    // Do not rename default config file!
    if (GS::QtLib::DatakeysContent::isDefaultConfigFile(fileInfo.fileName()))
        return;

    // Rename file-specific configuration file. if has not already the extension
    if(fileInfo.exists() && !fileInfo.absoluteFilePath().endsWith(newExtension))
    {
        if (!QFile::rename(fileInfo.absoluteFilePath(),
                    fileInfo.absoluteFilePath() + "." +newExtension))
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to rename %1 to %2")
                   .arg(fileInfo.absoluteFilePath()
                   .arg(fileInfo.absoluteFilePath() + "." +newExtension))
                  .toLatin1().constData());
    }
}

///////////////////////////////////////////////////////////
// Move a file-specific config file (if exists)
///////////////////////////////////////////////////////////
void GexDatabaseEditKeys::moveConfigFileKeys(GS::QtLib::DatakeysContent *pKeys
                                             , const QString &destDir)
{
    if(pKeys == NULL)
        return;

    QFileInfo   fileInfo(pKeys->Get("ConfigFileName").toString());
    QString     newFilePath;

    // Do not move default config file!
    if (GS::QtLib::DatakeysContent::isDefaultConfigFile(fileInfo.fileName()))
        return;

    newFilePath = destDir + QDir::separator() + fileInfo.fileName();

    // Delete destination file in case it already exists!
    if (QFile::exists(newFilePath))
    {
        if (!QFile::remove(newFilePath))
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to remove %1")
                   .arg(newFilePath)
                  .toLatin1().constData());
    }

      // Erase file-specific configuration file.
    if(fileInfo.exists())
    {
        if (!QFile::rename(fileInfo.absoluteFilePath(), newFilePath))
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to rename %1 to %2")
                   .arg(fileInfo.absoluteFilePath()
                   .arg(newFilePath))
                  .toLatin1().constData());
    }
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
bool	GexDatabaseEditKeys::parseValue(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,bool &bMatch,bool &bValidResult,QString &strError)
{
    // Init output variables
    bMatch = bValidResult = true;
    strResult = strExpression;

    // Check type of expression parser to use

    if(strExpression.indexOf(".Section",Qt::CaseInsensitive) >= 0)
    {
        // string like: 'Lot.Section(1-10)' or 'Facility.Section(%3_)'
        bValidResult = parseValueFieldSection(pKeys,strExpression,strResult,strKeyValue,strError);
        return bValidResult;
    }

    if(strExpression.indexOf(".RegExp",Qt::CaseInsensitive) >= 0)
        // string like: 'Lot.RegExp(Pattern,IfMatch,IfNotMatch)'
        return parseValueFieldRegExp(pKeys,strExpression,strResult,strKeyValue,bMatch,bValidResult,strError);

    if(strExpression.indexOf(".Date",Qt::CaseInsensitive) >= 0)
    {
        // string like: 'StartTime.Date(QT Date format)'
        bValidResult = parseValueDate(pKeys,strExpression,strResult,strKeyValue,strError);
        return bValidResult;
    }

    // We have a direct expression, which could be either a constant (ie TrackingLot, ABC), or a dbkey variable (ie TrackingLot, Lot)
    // In either cases, the result is valid
    bValidResult = true;
    if(pKeys && !pKeys->GetDbKeyContent(strExpression, strKeyValue))
        strResult = strExpression;
    else
        strResult = strKeyValue;

    return true;
}

///////////////////////////////////////////////////////////
// Overload MIR fields parsing Field section
///////////////////////////////////////////////////////////
bool GexDatabaseEditKeys::parseValueFieldSection(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,QString &strError)
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
    if(pKeys && !pKeys->GetDbKeyContent(strKey, strKeyValue))
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
bool GexDatabaseEditKeys::parseValueDate(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,QString &strError)
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
    if(pKeys && !pKeys->GetDbKeyContent(strKey, strKeyValue))
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
bool GexDatabaseEditKeys::parseValueFieldRegExp(GS::QtLib::DatakeysContent *pKeys,QString &strExpression,QString &strResult,QString &strKeyValue,bool &bMatch,bool &bValidResult,QString &strError)
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
    if(pKeys && !pKeys->GetDbKeyContent(strKey, strKeyValue))
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
    QRegExp             qRegExp("",Qt::CaseInsensitive), qRegExpPattern("",Qt::CaseInsensitive);
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
            strError.sprintf("Invalid regular expression (%s)", mapRegExp[iPos].toLatin1().constData());
            return false;
        }

        iPos++;
    }

    // Apply ExpPattern and extract the result
    // mapRegExp[0] =  regular expression
    // mapRegExp[1] =  value to use if expression matches
    // mapRegExp[2] =  value to use if expression doesn't match
    strExpression = strKey;
    strPattern = mapRegExp[0];

    // Check which regular expression is in a key
    if(pKeys && pKeys->GetDbKeyContent(strPattern, strRegExp))
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
        if(pKeys && !pKeys->GetDbKeyContent(regExpMatch, strResult))
            strResult = regExpMatch;
        bValidResult = true;
        return true;
    }

    // Retrieve captures
    int iCap;
    strResult = "";
    while(qRegExp.indexIn(regExpMatch)>=0)
    {
        iPos = qRegExp.indexIn(regExpMatch);
        strResult+=regExpMatch.left(iPos);
        iCap = qRegExp.cap(1).toInt();
        strResult += qRegExpPattern.cap(iCap);
        regExpMatch = regExpMatch.mid(iPos+1+QString::number(iCap).length());
    }
    strResult+=regExpMatch;

    bValidResult = true;
    return true;
}
///////////////////////////////////////////////////////////
// Overwrite Data file index keys with configuration file info (if exists)
///////////////////////////////////////////////////////////
bool	GexDatabaseEditKeys::loadConfigFileKeys(
        GS::QtLib::DatakeysContent *pKeys,
        bool bEraseConfigFile,
        bool &bFailedValidationStep,
        int *pnLineNb,
        QString &strError,
        QString strMoTaskSpoolingDir/*=""*/)
{
    if(pKeys == NULL)
        return false;

    QFile	cConfigFile;
    QString strConfigFile;

    bFailedValidationStep = false;

    // Reset line nb
    *pnLineNb = 0;

    // 0) Use ConfigFile if exists
    strConfigFile = pKeys->Get("ConfigFileName").toString();

    cConfigFile.setFileName(strConfigFile);

    // 1) Check if file-specific configuration file
    // name: <datafile>.gexdbkeys
    if(cConfigFile.exists() == false)
        strConfigFile = pKeys->Get("SourceArchive").toString() + ".gexdbkeys";

    cConfigFile.setFileName(strConfigFile);

    // 2) If no file-specific, check for a global configuration file in folder
    // name: "config.gexdbkeys"
    if(cConfigFile.exists() == false)
    {
        // Build file name: <data file path>/config.gexdbkeys
        QFileInfo cConfigFileInfo(pKeys->Get("SourceArchive").toString());
        strConfigFile = cConfigFileInfo.absolutePath();
        if((strConfigFile.endsWith("/") || strConfigFile.endsWith("\\")) == false)
            strConfigFile += "/";
        strConfigFile += GS::QtLib::DatakeysContent::defaultConfigFileName();
        cConfigFile.setFileName(strConfigFile);

        // 3) If no file-specific and no global configuration file in folder, check for a global configuration file in task spooling dir
        if((cConfigFile.exists() == false) && !strMoTaskSpoolingDir.isEmpty())
        {
            // Build file name: <Mo Task spooling dir>/config.gexdbkeys
            strConfigFile = strMoTaskSpoolingDir;
            if((strConfigFile.endsWith("/") || strConfigFile.endsWith("\\")) == false)
                strConfigFile += "/";
            strConfigFile += GS::QtLib::DatakeysContent::defaultConfigFileName();
            cConfigFile.setFileName(strConfigFile);
        }
    }

    // 4) If no config file exist at all: then keep current keys value!
    if(cConfigFile.exists() == false)
        return true;

    // Configuration file exists: read it!
    if(!cConfigFile.open(QIODevice::ReadOnly))
        return true;

    pKeys->SetInternal("ConfigFileName",strConfigFile);

    // Assign file I/O stream
    QTextStream hConfigFile(&cConfigFile);

    // Read config file, and load keys found.
    bool	bStatus;
    bool	bMatch, bValidResult;
    bool	bConcat;
    QString strLine,strKey,strExpression,strResult,strKeyValue;
    do
    {
        (*pnLineNb)++;
        strLine = hConfigFile.readLine();
        strKey = strLine.section(',',0,0);			// First field is 'Parameter'
        strKey = strKey.trimmed();					// remove leading spaces.
        strExpression = strLine.section(',',1);		// Rest of the line is the expression
        strExpression = strExpression.trimmed();	// remove leading spaces.

        // Check if comment line.
        if(strExpression.isEmpty() || strLine.startsWith("#"))
            continue;

        // Copy with section or regexp selection
        // Key1, Key2
        // Key1, Key2.Section()
        // Key1, Key2.RegExp()
        // Concat with section or regexp selection
        // Key1, +Key2
        // Key1, +Key2.Section()
        // Key1, +Key2.RegExp()

        bConcat = false;
        // Verify if it's concat expression
        if(strExpression.startsWith("+"))
        {
            bConcat = true;
            strExpression = strExpression.mid(1).trimmed();
        }
        bStatus = parseValue(pKeys,strExpression,strResult,strKeyValue,bMatch,bValidResult,strError);

        // Check if no error syntax
        if(!bStatus)
            return false;

        // If section "Validation", RegExp have to match else Validation_Error
        if(strKey.append(":").startsWith("Validation:", Qt::CaseInsensitive))
        {
            if(!bMatch)
            {
                bFailedValidationStep = true;
                strError.sprintf("regexp doesn't match (for %s=%s)", strExpression.toLatin1().constData(), strKeyValue.toLatin1().constData());
                return false;
            }
            else
                continue;
        }

        // Case 4544: do not affect key if invalid result retrieved
        if(!bValidResult)
            continue;

        // Check if literal string or substring to extract from strValue.
        // Check which field to overload
        if(bConcat)
        {
            pKeys->GetDbKeyContent(strKey,strExpression);
            // Have to additionate Value and Result if INT
            bool	bValueIsInt, bResultIsInt;
            strExpression.toInt(&bValueIsInt);
            strResult.toInt(&bResultIsInt);
            if(bValueIsInt && bResultIsInt)
            {
                int		nValue;
                nValue = strExpression.toInt() + strResult.toInt();
                strResult = QString::number(nValue);;
            }
            else
                strResult = strExpression + strResult;
        }

        // Case 4544: we have to set the key even if the expression does not match, given the retrieved result is valid

        // Check if DateTime is valid
        if((strKey.toLower() == "setuptime")
        || (strKey.toLower() == "starttime")
        || (strKey.toLower() == "finishtime"))
        {
            unsigned int time = strResult.toLong();
            QDateTime	clCurrentDateTime = QDateTime::currentDateTime().addDays(1);
            QDateTime	clStartDateTime = QDateTime(QDate(2000,1,1));
            if((time == 0)
            || (time < clStartDateTime.toTime_t())
            || (time > clCurrentDateTime.toTime_t()))
            {
                strError = "Invalid TimeStamp value for " + strKey + " : " + strResult;
                return false;
            }
        }
        if(!pKeys->SetDbKeyContent(strKey,strResult))
        {
            if(!pKeys->Get("Error").toString().isEmpty())
                strError = pKeys->Get("Error").toString();
            else
                strError = "Invalid Key word : " + strKey;
            return false;
        }
    }
    while((hConfigFile.atEnd() == false));

    // Close file
    cConfigFile.close();

    // Erase file-specific configuration file if need be
    if(bEraseConfigFile)
        deleteConfigFileKeys(pKeys);

    return true;
}

