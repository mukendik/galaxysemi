#include "engine.h"
#include "mo_admin.h"
#include "gex_shared.h"
#include "browser_dialog.h"
#include "ym_event_log_gui.h"
#include "admin_engine.h"
#include "gstdl_errormgr.h"
#include "admin_engine.h"
#include "db_engine.h"
#include "gexdb_plugin_base.h"
#include "message.h"
#include <QLabel>
#include <QSpinBox>
#include <QSqlError>
#include <QDateEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QScrollArea>
#include <QProgressBar>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include <QtDebug>

extern QProgressBar	*	GexProgressBar;

//-------------------GexYMEventLogFilter---------------------
QStringList GexYMEventLogFilter::m_soDataTimeOperator=QStringList()<<"Most recent hours"
                                                                  <<"Most recent days"
                                                                 <<"Most recent weeks"
                                                                <<"Most recent months"
                                                               <<"Date range"
                                                              <<"All dates";

QStringList GexYMEventLogFilter::m_soIntegerOperator=QStringList()<<"Equal"
                                                                 <<"In";

QStringList GexYMEventLogFilter::m_soEnumOperator=QStringList()<<"Equal"
                                                              <<"In";

QStringList GexYMEventLogFilter::m_soDoubleOperator=QStringList()<<"equals"
                                                                <<"not equal to"
                                                               <<"greater than"
                                                              <<"less or equal"
                                                             <<"greater or equal";

QStringList GexYMEventLogFilter::m_soTextOperator=QStringList()<<"starts with"
                                                              <<"ends with"
                                                             <<"contains"
                                                            <<"does not contain"
                                                           <<"like";

int GexYMEventLogFilter::m_iGexYMEventLogFilterCounter = 0;
int GexYMEventBasicLogFilterElem::m_iGexYMEventBasicLogFilterElemCount = 0;
GexYMEventLogFilter::GexYMEventLogFilter(){
    m_iFilterType = GexYMEventBasicLogFilter::unknownType;
    m_iIdx = -1;
    m_iGexYMEventLogFilterCounter++;

}

GexYMEventLogFilter::~GexYMEventLogFilter(){
    m_iGexYMEventLogFilterCounter--;
}

void GexYMEventLogFilter::save(QDomElement &oElement, QDomDocument &/*oXMLDoc*/){

    oElement.setAttribute("name", m_strXMLName);
    oElement.setAttribute("label",m_strLabel);
    oElement.setAttribute("filter_tree", m_bFilterTree ? "true" : "false");
    oElement.setAttribute("color",m_strColor);
    oElement.setAttribute("icon",m_strIcon);
    oElement.setAttribute("idx",m_iIdx);
    oElement.setAttribute("var",getStringType());

}

void GexYMEventExpressionLogFilter::save(QDomElement &oElement, QDomDocument &oXMLDoc){
    GexYMEventLogFilter::save(oElement,oXMLDoc);
    QDomElement oExpression = oXMLDoc.createElement("expression");

    foreach(GexYMEventBasicLogFilterElem *poElem, m_oExpression){
        QDomElement oElem = oXMLDoc.createElement("elem");
        oElem.setAttribute("operator", poElem->m_strOperator);
        oElem.setAttribute("basic_filter", poElem->m_strBasicFilter);
        oElem.setAttribute("operand", poElem->m_strOperand);
        oElem.setAttribute("value", poElem->m_strValue);
        oExpression.appendChild(oElem);
    }

    foreach(GexYMEventBasicLogFilterElem *poElem, m_oDynamicExpression){
        QDomElement oDynamicElem= oXMLDoc.createElement("dynamic_elem");
        oDynamicElem.setAttribute("operator", poElem->m_strOperator);
        oDynamicElem.setAttribute("basic_filter", poElem->m_strBasicFilter);
        oDynamicElem.setAttribute("operand", poElem->m_strOperand);
        oDynamicElem.setAttribute("value", poElem->m_strValue);
        oExpression.appendChild(oDynamicElem);

    }

    oElement.appendChild(oExpression);
}

void GexYMEventLogFilter::setIcon(const QString &strVal){
    m_strIcon = strVal;
}

QString GexYMEventLogFilter::getIcon(){
    return m_strIcon;

}

void GexYMEventLogFilter::setColor(const QString &strVal){
    m_strColor = strVal;

}

QString GexYMEventLogFilter::getColor(){
    return m_strColor;
}

void GexYMEventLogFilter::setFilterTree(bool bVal){
    m_bFilterTree = bVal;
}

bool GexYMEventLogFilter::getFilterTree(){
    return m_bFilterTree;
}

void GexYMEventLogFilter::setXMLName(const QString &strName){
    m_strXMLName = strName;
}

QString GexYMEventLogFilter::getXMLName(){
    return m_strXMLName;
}

void GexYMEventLogFilter::setLabel(const QString &strLabel){
    m_strLabel = strLabel;

}

const QString &GexYMEventLogFilter::getLabel(){
    return m_strLabel;
}


void GexYMEventLogFilter::setIndex(int iIdx){
    m_iIdx = iIdx;
}

int GexYMEventLogFilter::getIndex(){
    return m_iIdx;
}

bool GexYMEventLogFilter::setType(const QString &strVar){

    if(strVar == "datetime"){
        m_iFilterType = GexYMEventBasicLogFilter::datetimeType;
    } else if(strVar == "integer"){
        m_iFilterType = GexYMEventBasicLogFilter::integerType;
    } else if(strVar == "enum"){
        m_iFilterType = GexYMEventBasicLogFilter::enumType;
    } else if(strVar == "double"){
        m_iFilterType = GexYMEventBasicLogFilter::doubleType;
    } else if(strVar == "text") {
        m_iFilterType = GexYMEventBasicLogFilter::textType;
    } else {
        m_iFilterType = GexYMEventBasicLogFilter::unknownType;
        return false;
    }

    return true;
}

int GexYMEventLogFilter::getType(){
    return m_iFilterType;
}

QString GexYMEventLogFilter::getStringType(){

    if(m_iFilterType == GexYMEventBasicLogFilter::datetimeType)
        return "datetime";
    if(m_iFilterType == GexYMEventBasicLogFilter::integerType)
        return  "integer";
    if(m_iFilterType == GexYMEventBasicLogFilter::enumType)
        return "enum";
    if(m_iFilterType == GexYMEventBasicLogFilter::doubleType)
        return "double";
    if(m_iFilterType == GexYMEventBasicLogFilter::textType)
        return "text";

    if(m_iFilterType == GexYMEventBasicLogFilter::unknownType)
        return "";
    return "";
}

//----------------------GexYMEventBasicLogFilter----------------------

GexYMEventBasicLogFilter::GexYMEventBasicLogFilter() : GexYMEventLogFilter(){
    m_bFetchFromEventTable = false;

    m_bEventFieldLoaded = false;
    m_bDBFieldLoaded = false;

}

GexYMEventBasicLogFilter::~GexYMEventBasicLogFilter(){
    ;
}

GexYMEventBasicLogFilter *GexYMEventBasicLogFilter::loadFromXML(QDomNode &oFilterNode, int &iError, QString &strError){

    int iIdx = oFilterNode.toElement().attribute("idx","-1").toInt();
    if(iIdx==-1){
        iError = GexYMEventLogLoader::missingAttributes;
        strError = QString("Missing attribute %1 in tag %2").arg("idx").arg(oFilterNode.nodeName());
        return 0;
    }

    QString strLabel = oFilterNode.toElement().attribute("label","NA");
    if(strLabel=="NA"){
        iError = GexYMEventLogLoader::missingAttributes;
        strError = QString("Missing attribute %1 in tag %2").arg("label").arg(oFilterNode.nodeName());
        return 0;
    }

    QString strVar = oFilterNode.toElement().attribute("var","NA");
    if(strVar=="NA"){
        iError = GexYMEventLogLoader::missingAttributes;
        strError = QString("Missing attribute %1 in tag %2").arg("var").arg(oFilterNode.nodeName());
        return 0;
    }

    GexYMEventBasicLogFilter *poFilter = new GexYMEventBasicLogFilter;

    poFilter->setXMLName(oFilterNode.nodeName());
    poFilter->setIndex(iIdx);
    if(!poFilter->setType(strVar)){
        delete poFilter;
        iError = GexYMEventLogLoader::invalidXML;
        strError = QString("Unknow filter Type %1. check XML file").arg(strVar);
        return 0;
    }
    poFilter->setLabel(strLabel);

    QString strIcon = oFilterNode.toElement().attribute("icon","");
    QString strColor = oFilterNode.toElement().attribute("color","");
    QString strFetchFrom = oFilterNode.toElement().attribute("fetch_from","");
    bool bFetchFromEventTable = (oFilterNode.toElement().attribute("fetch_from_event_table","") == QString("true"));
    poFilter->setIcon(strIcon);
    poFilter->setColor(strColor);
    poFilter->setFetchFrom(strFetchFrom);
    if(!strFetchFrom.isEmpty()){
        poFilter->setFriendlyName(oFilterNode.toElement().attribute("friendly_name",""));
        if(poFilter->getFriendlyName().isEmpty()){
            delete poFilter;
            iError = GexYMEventLogLoader::invalidXML;
            strError = QString("Friendly name is empty");
            return 0;
        }
    }
    poFilter->setFetchFromEventTable(bFetchFromEventTable);

    QStringList oStrList;
    if(!poFilter->getFetchFrom().isEmpty()){
        oStrList = poFilter->fetchFromDBTable(oFilterNode.nodeName(), poFilter->getFetchFrom(), poFilter->getFriendlyName());
    }else if(poFilter->getFetchFromEventTable()){
        oStrList = poFilter->fetchFromEventTable(oFilterNode.nodeName());
    }

    foreach(const QString &strField, oStrList){

        QString strLabel = strField;
        if(strField.contains(GexYMEventLogDB::getInstance()->m_strSeparator))
            strLabel = strField.section(GexYMEventLogDB::getInstance()->m_strSeparator, 1,1);
        poFilter->addEnum(oStrList.indexOf(strField),
                          strField,
                          strLabel,
                          "",
                          "");
    }

    if(poFilter->getType() == GexYMEventBasicLogFilter::enumType && oStrList.isEmpty()){
        QDomNodeList oEnumList = oFilterNode.childNodes();
        if(!oEnumList.count()){
            delete poFilter;
            iError = GexYMEventLogLoader::invalidXML;
            strError = QString("No Enum list found");
            return 0;
        }
        for(int iIdx=0; iIdx<oEnumList.count(); iIdx++){

            QDomElement oEnum = oEnumList.at(iIdx).toElement();
            if(oEnum.isComment())
                continue;
            poFilter->addEnum((oEnum.attribute("idx").isEmpty() ? -1 : oEnum.attribute("idx").toInt()),
                              oEnum.nodeName(),
                              oEnum.attribute("label"),
                              oEnum.attribute("icon"),
                              oEnum.attribute("color"));
        }
    }

    return poFilter;
}

QStringList GexYMEventBasicLogFilter::fetchFromDBTable(const QString &strField, const QString &strTable, const QString &strFriendlyName, bool bRefresh){

    if(!m_bDBFieldLoaded || bRefresh){
        m_oDBFieldValues.clear();
        m_oDBFieldValues = GexYMEventLogDB::getInstance()->getFieldValues( "ym_events", strTable, strField, strFriendlyName);
        m_bDBFieldLoaded = true;
    }

    return m_oDBFieldValues;
}

QStringList GexYMEventBasicLogFilter::fetchFromEventTable(const QString &strField, bool bRefresh){

    if(!m_bEventFieldLoaded || bRefresh){
        m_oEventFieldValues.clear();
        m_oEventFieldValues = GexYMEventLogDB::getInstance()->getFieldValues("ym_events", strField);
        m_bEventFieldLoaded = true;
    }
    return m_oEventFieldValues;
}

void GexYMEventBasicLogFilter::setFetchFrom(const QString &strFetchFrom){
    m_strFetchFrom = strFetchFrom;

}
QString GexYMEventBasicLogFilter::getFetchFrom(){
    return m_strFetchFrom;
}

void GexYMEventBasicLogFilter::setFetchFromEventTable(bool bVal){
    m_bFetchFromEventTable = bVal;
}

bool GexYMEventBasicLogFilter::getFetchFromEventTable(){
    return m_bFetchFromEventTable;
}

bool GexYMEventBasicLogFilter::addEnum(int iXmlIdx, const QString &strName, const QString &strLabel, const QString &strIcon, const QString & strColor){
    int iIdx = m_oEnumValues.count();;
    if(iXmlIdx>=0)
        iIdx = iXmlIdx;

    m_oEnumValues.insert(iIdx,  QMap<QString, QString> ());
    (m_oEnumValues[iIdx])["name"] = strName;
    if(strLabel.isEmpty())
        (m_oEnumValues[iIdx])["label"] = strName;
    else
        (m_oEnumValues[iIdx])["label"] = strLabel;

    (m_oEnumValues[iIdx])["icon"] = strIcon;
    (m_oEnumValues[iIdx])["color"] = strColor;

    return true;
}

//QStringList fetchFromDBTable(const QString &strField, const QString &strTable, const QString &strFriendlyName,bool bRefresh=false);
//QStringList fetchFromEventTable(const QString &strField, bool bRefresh=false);

QStringList GexYMEventBasicLogFilter::getEnumList(){

    if(!getFetchFrom().isEmpty() || getFetchFromEventTable()){
        //Connect to db to retrieve data
        QStringList oValuesList;
        if(getFetchFromEventTable())
            //oValuesList = GexYMEventLogDB::getInstance()->getFieldValues("ym_events", m_strXMLName);
            oValuesList =  fetchFromEventTable(m_strXMLName);
        else if(!getFetchFrom().isEmpty())
            //oValuesList = GexYMEventLogDB::getInstance()->getFieldValues("ym_events", getFetchFrom() , m_strXMLName, m_strFriendlyName);
            oValuesList =  fetchFromDBTable(m_strXMLName,getFetchFrom(),m_strFriendlyName);
        if(!oValuesList.isEmpty())
            return oValuesList;
    }



    if(!m_oEnumValues.isEmpty()){
        QStringList oEnums ;
        foreach(int iIdx, m_oEnumValues.keys()){
            oEnums.append((m_oEnumValues[iIdx])["label"]);
        }
        return oEnums;
    }

    return QStringList();
}

QList <int> GexYMEventBasicLogFilter::getEnumIdx(){
    return m_oEnumValues.keys();
}

QMap<QString, QString> &GexYMEventBasicLogFilter::getEnum(int iIdx){
    return m_oEnumValues[iIdx];
}

QStringList GexYMEventBasicLogFilter::getOperatorChoice(){

    if(m_iFilterType == GexYMEventBasicLogFilter::datetimeType){
        return m_soDataTimeOperator;
    }else if(m_iFilterType == GexYMEventBasicLogFilter::integerType){
        return m_soIntegerOperator;
    }else if(m_iFilterType == GexYMEventBasicLogFilter::enumType){
        return m_soEnumOperator;
    }else if(m_iFilterType == GexYMEventBasicLogFilter::doubleType){
        return m_soDoubleOperator;
    }else if(m_iFilterType == GexYMEventBasicLogFilter::textType){
        return m_soTextOperator;
    }

    return QStringList();
}


//--------------GexYMEventExpressionLogFilter---------------

GexYMEventExpressionLogFilter *GexYMEventExpressionLogFilter::loadFromXML(QDomNode &oFilterNode, int &iError, QString &strError){

    QString strName = oFilterNode.toElement().attribute("name","NA");
    if(strName=="NA"){
        iError = GexYMEventLogLoader::missingAttributes;
        strError = QString("Missing attribute %1 in tag %2").arg("name").arg(oFilterNode.nodeName());
        return 0;
    }
    QString strLabel = oFilterNode.toElement().attribute("label","NA");
    if(strLabel=="NA"){
        iError = GexYMEventLogLoader::missingAttributes;
        strError = QString("Missing attribute %1 in tag %2").arg("label").arg(oFilterNode.nodeName());
        return 0;
    }
    QString strIcon = oFilterNode.toElement().attribute("icon","");
    QString strColor = oFilterNode.toElement().attribute("color","");


    QDomNode oExpressions = oFilterNode.namedItem("expression");
    if(oExpressions.isNull()){
        iError = GexYMEventLogLoader::missingTag;
        strError = QString("Missing tag %1 in tag %2").arg("expression").arg(oFilterNode.nodeName());
        return 0;
    }

    //loop on expression
    QDomNodeList oExpressionElems = oExpressions.childNodes();
    if(!oExpressionElems.count()){
        iError = GexYMEventLogLoader::invalidXML;
        strError = QString("No Expression list found");
        return 0;
    }

    GexYMEventExpressionLogFilter *poExpression = new GexYMEventExpressionLogFilter();
    poExpression->setColor(strColor);
    poExpression->setIcon(strIcon);
    poExpression->setLabel(strLabel);
    poExpression->setXMLName(strName);
    poExpression->setFilterTree (oFilterNode.toElement().attribute("filter_tree","") == "true");


    for(int iIdx=0; iIdx<oExpressionElems.count(); iIdx++){
        QDomElement oExpression = oExpressionElems.at(iIdx).toElement();
        if(oExpression.isComment() ||(oExpression.nodeName() != "elem" && oExpression.nodeName() != "dynamic_elem"))
            continue;
        QString strOperator = oExpression.attribute("operator","");
        QString strBasicFilter = oExpression.attribute("basic_filter","");
        QString strOperand = oExpression.attribute("operand","");
        QString strValue = oExpression.attribute("value","");
        if(strBasicFilter.isEmpty() || strOperand.isEmpty()){
            iError = GexYMEventLogLoader::invalidXML;
            strError = QString("Expression is missing");
            delete poExpression;
            return 0;
        }
        if( oExpression.nodeName() == "elem")
            poExpression->addElem(strOperator, strBasicFilter, strOperand, strValue);
        else if(oExpression.nodeName() == "dynamic_elem")
            poExpression->addDynamicElem(strOperator, strBasicFilter, strOperand, strValue);
    }


    return poExpression;
}


GexYMEventExpressionLogFilter::GexYMEventExpressionLogFilter():GexYMEventLogFilter(){

}

void GexYMEventExpressionLogFilter::clear(){
    qDeleteAll(m_oExpression);
    m_oExpression.clear();

    qDeleteAll(m_oDynamicExpression);
    m_oDynamicExpression.clear();
}

GexYMEventExpressionLogFilter::~GexYMEventExpressionLogFilter(){
    clear();

}

QString GexYMEventExpressionLogFilter::buildWhereClause(){
    QString strWhereClause;
    QString strFirst;
    QStringList oOthers;
    bool bFirstFilter = true;
    foreach(GexYMEventBasicLogFilterElem *poElem, m_oExpression){
        if(bFirstFilter){
            QString strTemp = poElem->m_strOperator;
            poElem->m_strOperator.clear();
            strFirst = poElem->buildClause();
            poElem->m_strOperator = strTemp;
            bFirstFilter = false;
        }
        else
            oOthers.append(poElem->buildClause());

    }

    foreach(GexYMEventBasicLogFilterElem *poElem, m_oDynamicExpression){
        if(bFirstFilter){
            QString strTemp = poElem->m_strOperator;
            poElem->m_strOperator.clear();
            strFirst = poElem->buildClause();
            poElem->m_strOperator = strTemp;
            bFirstFilter = false;
        }
        else
            oOthers.append(poElem->buildClause());
    }

    strWhereClause = strFirst + oOthers.join(" ");
    return strWhereClause;
}

QString GexYMEventBasicLogFilterElem::m_strValuesSeparator = "|";

QString GexYMEventBasicLogFilterElem::buildClause(){
    QString strOperand = buildOperand();
    if(strOperand.isEmpty())
        return "";
    else
        return m_strOperator + " ( " + strOperand + " ) ";
}

QString GexYMEventBasicLogFilterElem::buildOperand(){

    int iIdx = -1;
    if((iIdx = GexYMEventLogFilter::m_soDataTimeOperator.indexOf(m_strOperand)) != -1){
        QDateTime oDateTime;
        QDate Date = QDate::currentDate();
        switch(iIdx){
        case 0 :
        {
            // Current Time - N hours
            oDateTime = QDateTime::currentDateTime().addSecs(-(3600*m_strValue.toInt()));
            return m_strBasicFilter + ">=" +GexYMEventLogDB::getInstance()->getAdminServerConnector()->NormalizeDateToSql(oDateTime);
        }
        case 1 :
        {
            // Current Day - N days
            // start at the begining of the day
            oDateTime.setDate(Date.addDays(1-m_strValue.toInt()));
            return m_strBasicFilter + ">=" +GexYMEventLogDB::getInstance()->getAdminServerConnector()->NormalizeDateToSql(oDateTime);
        }
        case 2 :
        {
            // Current Week - N weeks
            // start at the begining of the week
            Date = Date.addDays(1-Date.dayOfWeek());
            oDateTime.setDate(Date.addDays((1-m_strValue.toInt())*7));
            return m_strBasicFilter + ">=" +GexYMEventLogDB::getInstance()->getAdminServerConnector()->NormalizeDateToSql(oDateTime);
        }
        case 3 :
        {
            // Current Month - N month
            // start at the begining of the month
            Date = QDate(Date.year(),Date.month(),1);
            oDateTime.setDate(Date.addMonths(1-m_strValue.toInt()));
            return m_strBasicFilter + ">=" +GexYMEventLogDB::getInstance()->getAdminServerConnector()->NormalizeDateToSql(oDateTime);
        }

        case 4 : //Data range
        {
            QDateTime oDateTime1 = QDateTime::fromString(m_strValue.section(m_strValuesSeparator,0,0),"yyyy-MM-dd hh:mm:ss");
            QDateTime oDateTime2 = QDateTime::fromString(m_strValue.section(m_strValuesSeparator,1,1),"yyyy-MM-dd hh:mm:ss");
            return QString("(") +
                    QString("(") + m_strBasicFilter + ">=" +GexYMEventLogDB::getInstance()->getAdminServerConnector()->NormalizeDateToSql(oDateTime1) + QString(")") +
                    QString(" AND (") + m_strBasicFilter + "<=" +GexYMEventLogDB::getInstance()->getAdminServerConnector()->NormalizeDateToSql(oDateTime2) + QString(")")+
                    QString(")");
        }
        case 5 : //All dates
            return "";
        }
    }else if((iIdx = GexYMEventLogFilter::m_soIntegerOperator.indexOf(m_strOperand)) != -1){

        switch(iIdx){
        case 0:
            return m_strBasicFilter + "='" + m_strValue + "'";
        case 1:
        {
            QStringList oValues = m_strValue.split(m_strValuesSeparator);
            QString strClause = QString("(") + m_strBasicFilter + "='" + oValues[0] +  QString("')");
            for(int iIdx=1; iIdx<oValues.count(); iIdx++){
                strClause += QString(" OR (") + m_strBasicFilter + "='" + oValues[iIdx] +  QString("')");

            }
            return strClause;
        }
        }

    }else if((iIdx = GexYMEventLogFilter::m_soEnumOperator.indexOf(m_strOperand)) != -1){

        switch(iIdx){
        case 0:
            return m_strBasicFilter + "='" + m_strValue +"'";
        case 1:
        {
            QStringList oValues = m_strValue.split(m_strValuesSeparator);

            QString strClause = QString("(") + m_strBasicFilter + "='" + oValues[0] +  QString("')");
            for(int iIdx=1; iIdx<oValues.count(); iIdx++){
                strClause += QString(" OR (") + m_strBasicFilter + "='" + oValues[iIdx] +  QString("')");

            }
            return strClause;
        }
        }

    }else if((iIdx = GexYMEventLogFilter::m_soDoubleOperator.indexOf(m_strOperand)) != -1){
        switch(iIdx){
        case 0:
            return m_strBasicFilter + "=" + m_strValue;
        case 1:
            return m_strBasicFilter + "!=" + m_strValue;
        case 2:
            return m_strBasicFilter + ">" + m_strValue;
        case 3:
            return m_strBasicFilter + "<=" + m_strValue;
        case 4:
            return m_strBasicFilter + ">=" + m_strValue;
        }

    }else if((iIdx = GexYMEventLogFilter::m_soTextOperator.indexOf(m_strOperand)) != -1){
        switch(iIdx){
        case 0:
            return m_strBasicFilter + "like" + QString("'%1%%'").arg(m_strValue);
        case 1:
            return m_strBasicFilter + "like" + QString("'%%%1'").arg(m_strValue);;
        case 2:
            return m_strBasicFilter + "like" + QString("'%%%1%%'").arg(m_strValue);
        case 3:
            return m_strBasicFilter + "not like" + QString("'%%%1%%'").arg(m_strValue);;
        case 4:
            return m_strBasicFilter + "like" + QString("'%1'").arg(m_strValue);
        }
    }

    return "";
}
//------------------------------GexYMEventLogSettings------------------------------
int GexYMEventLogSettings::m_iGexYMEventLogSettingsCount = 0;
GexYMEventLogSettings::GexYMEventLogSettings(){
    m_iGexYMEventLogSettingsCount++;

}

GexYMEventLogSettings::~GexYMEventLogSettings(){
    m_iGexYMEventLogSettingsCount--;
    qDeleteAll(m_oFilters.values());
    m_oFilters.clear();
    qDeleteAll(m_oFiltersExpression.values());
    m_oFiltersExpression.clear();
    qDeleteAll(m_oNewUserFilters.values());
    m_oNewUserFilters.clear();

}

void GexYMEventLogSettings::setLogLabel(const QString &strLabel){
    m_strLogLabel = strLabel;
}

QString  GexYMEventLogSettings::getLogLabel(){
    return m_strLogLabel;
}

void GexYMEventLogSettings::setLogIcon(const QString &strIcon){
    m_strIcon = strIcon;
}

QString  GexYMEventLogSettings::getLogIcon(){
    return m_strIcon;
}

void GexYMEventLogSettings::setViewerFields(const QStringList &oFields){
    m_oViewerFields = oFields;
}

QStringList  GexYMEventLogSettings::getViewerFields(){
    return m_oViewerFields;
}
bool GexYMEventLogSettings::addFilter(int iIdx, GexYMEventBasicLogFilter *poFilter){
    if(m_oFilters.contains(iIdx)){
        return false;
    }
    m_oFilters.insert(iIdx, poFilter);

    return true;
}

QMap<int, GexYMEventBasicLogFilter *> &GexYMEventLogSettings::getFilters(){
    return m_oFilters;
}

bool GexYMEventLogSettings::addPredefinedFilters(int iIdx, GexYMEventExpressionLogFilter *poFilter){
    if(m_oFiltersExpression.contains(iIdx)){
        return false;
    }
    m_oFiltersExpression.insert(iIdx, poFilter);
    return true;
}

QString GexYMEventLogSettings::getFriendlyName(const QString &strName){
    foreach(int iIdx, m_oFilters.keys()){
        if(m_oFilters[iIdx]->getXMLName() == strName)
            return m_oFilters[iIdx]->getLabel();
    }
    return strName;
}

GexYMEventBasicLogFilter *GexYMEventLogSettings::getFilter(const QString &strXMLName){
    foreach(int iIdx, m_oFilters.keys()){
        if(m_oFilters[iIdx]->getXMLName() == strXMLName)
            return m_oFilters[iIdx];
    }
    return 0;

}

QMap<int, GexYMEventExpressionLogFilter *> &GexYMEventLogSettings::getPredefinedFilters(){
    return m_oFiltersExpression;
}


//--------------------------------GexYMEventLog--------------------------------
int GexYMEventLogLoader::getErrorCode(){
    return m_iError;
}
const QString &GexYMEventLogLoader::getError(){
    return m_strError;
}
void GexYMEventLogLoader::setError(int iError, const QString &strError ){
    m_iError = iError;
    m_strError = strError;
}

QMap<QString, int> GexYMEventLogLoader::getFilters(){
    QMap<QString, int> oFilters;
    QMap<int, GexYMEventBasicLogFilter *> &oFiltersMap = m_oGexYMEventLogSettings.getFilters();
    QList<int> oFiltersIdx = oFiltersMap.keys();
    qSort(oFiltersIdx);
    foreach(int iIdx, oFiltersIdx){
        oFilters.insert(oFiltersMap[iIdx]->getLabel(), iIdx);
    }
    return oFilters;
}

QString GexYMEventLogLoader::m_strEventTableVersion = "0.0";

int GexYMEventLogLoader::m_iGexYMEventLogLoaderCount = 0;

GexYMEventLogLoader::GexYMEventLogLoader()
    : m_iStepCount(1)
{
    m_iGexYMEventLogLoaderCount++;
    m_iStepCount = 0;
    m_iStep = 0;
    setError(GexYMEventLogLoader::noError,"");
    if(!loadFilters())
        return ;

}

GexYMEventLogLoader::~GexYMEventLogLoader(){
    m_iGexYMEventLogLoaderCount--;
}

bool GexYMEventLogLoader::isCompatible(const QString &strVersion){
    return  (strVersion == m_strEventTableVersion);
}

void GexYMEventLogLoader::updateProgress(int iLocalProgress, int iStep){
    double dProgress = 100;
    if(m_iStepCount){
        double dProgressStart = 100 * double(iStep)/double(m_iStepCount);
        dProgress = dProgressStart +  ((double(100)/double(m_iStepCount)) * (double(iLocalProgress)/double(100)));
    }
    if(GexProgressBar)
        GexProgressBar->setValue((int)dProgress);
}

bool GexYMEventLogLoader::loadBasicFilters(QDomNode &oEventTable){

    QDomNode oBasicFilters = oEventTable.namedItem("basic_filters");
    QDomNodeList oFiltersList = oBasicFilters.childNodes();
    if(!oFiltersList.count()){
        setError(GexYMEventLogLoader::invalidXML, QString("No filters found"));
        updateProgress(100,m_iStep);
        return false;
    }

    for(int iIdx=0; iIdx <oFiltersList.count(); iIdx++){
        updateProgress((int)(100 * double(iIdx)/double(oFiltersList.count())),m_iStep);
        QDomNode oFilterNode = oFiltersList.at(iIdx);
        if(oFilterNode.isComment())
            continue;
        int iError;
        QString strError;
        GexYMEventBasicLogFilter *poFilter = GexYMEventBasicLogFilter::loadFromXML(oFilterNode, iError, strError);
        bool bStatus = (poFilter != 0);
        if(!bStatus){
            setError(iError, strError);
            updateProgress(100, m_iStep);
            return false;
        }else {
            if(!m_oGexYMEventLogSettings.addFilter(iIdx, poFilter)){
                setError(GexYMEventLogLoader::invalidXML, QString("The filter %1 already exist. check XML file").arg(poFilter->getLabel()));
                delete poFilter;
                poFilter = 0;
                return false;
            }
        }
    }
    return true;

}

bool GexYMEventLogLoader::loadPredinedFilters(QDomNode &oEventTable){

    QDomNode oPredifinedFilters = oEventTable.namedItem("predifined_filters");
    QDomNodeList oFiltersList = oPredifinedFilters.childNodes();
    if(!oFiltersList.count()){
        setError(GexYMEventLogLoader::invalidXML, QString("Unable to find item <predifined_filters>"));
        updateProgress(100,m_iStep);
        return false;
    }

    for(int iIdx=0; iIdx <oFiltersList.count() ; iIdx++){
        updateProgress((int)(100 * double(iIdx)/double(oFiltersList.count())),m_iStep);
        QDomNode oFilterNode = oFiltersList.at(iIdx);
        if(oFilterNode.isComment() || oFilterNode.nodeName() != "filter")
            continue;
        int iError;
        QString strError;
        GexYMEventExpressionLogFilter *poFilter = GexYMEventExpressionLogFilter::loadFromXML(oFilterNode, iError, strError);
        bool bStatus = (poFilter != 0);
        if(!bStatus){
            setError(iError, strError);
            updateProgress(100, m_iStep);
            return false;
        }else {
            if(!m_oGexYMEventLogSettings.addPredefinedFilters(iIdx, poFilter)){
                setError(GexYMEventLogLoader::invalidXML, QString("The filter %1 already exist. check XML file").arg(poFilter->getLabel()));
                delete poFilter;
                poFilter = 0;
                updateProgress(100, m_iStep);
                return false;
            }
        }
    }
    return true;

}

bool GexYMEventLogLoader::loadExtensions(QDomNode &oEventTable){

    QDomNode oExtensionFilters = oEventTable.namedItem("extension_filter");
    QDomNodeList oFiltersList = oExtensionFilters.childNodes();
    for(int iIdx=0; iIdx <oFiltersList.count(); iIdx++){
        updateProgress((int)(100 * double(iIdx)/double(oFiltersList.count())),m_iStep);
        QDomNode oFilterNode = oFiltersList.at(iIdx);
        if(oFilterNode.isComment() || oFilterNode.nodeName() != "xml_files")
            continue;
        QString strFile = oFilterNode.toElement().attribute("file");
        QString strTag = oFilterNode.toElement().attribute("tag");
        QFile oFile(strFile);
        if(strFile.isEmpty())
            continue;
        if(!oFile.open(QIODevice::ReadOnly))
            continue;
        QDomDocument oXMLDoc;
        if(!oXMLDoc.setContent(&oFile))
            continue;
        QDomNode oEventTable = oXMLDoc.namedItem(strTag);
        if(oEventTable.isNull())
            continue;
        loadBasicFilters(oEventTable);
        loadPredinedFilters(oEventTable);
    }
    updateProgress(100,m_iStep);

    return true;
}

bool GexYMEventLogLoader::loadViewer(QDomNode &oEventTable){
    QDomNode oViewer = oEventTable.namedItem("log_viewer");
    if(oViewer.isNull()){
        setError(GexYMEventLogLoader::invalidXML, QString("No Viewer fields found"));
        updateProgress(100,m_iStep);
        return false;
    }

    QDomNode oViewerFields = oViewer.namedItem("viewer_fields");
    if(oViewerFields.isNull()){
        setError(GexYMEventLogLoader::invalidXML, QString("No Viewer fields found"));
        updateProgress(100,m_iStep);
        return false;
    }

    QDomNodeList oViewerFieldsList = oViewerFields.childNodes();
    if(!oViewerFieldsList.count()){
        setError(GexYMEventLogLoader::invalidXML, QString("No Viewer fields found"));
        updateProgress(100,m_iStep);
        return false;
    }

    QStringList oFields;
    QMap<int, QString> oRawFields;

    for(int iIdx=0; iIdx <oViewerFieldsList.count(); iIdx++){
        updateProgress((int)(100 * double(iIdx)/double(oViewerFieldsList.count())),m_iStep);
        QDomNode oFilterNode = oViewerFieldsList.at(iIdx);
        if(oFilterNode.isComment())
            continue;
        int iIdxml = oFilterNode.toElement().attribute("idx","-1").toInt();
        if(iIdxml<0)
            oRawFields.insert(oRawFields.count(), oFilterNode.nodeName());
        else
            oRawFields.insert(iIdxml, oFilterNode.nodeName());
    }
    QList<int> oSortedIdx = oRawFields.keys();
    qSort(oSortedIdx);
    foreach (int iIdx, oSortedIdx) {
        oFields.append(oRawFields[iIdx]);
    }

    if(!oFields.count()){
        setError(GexYMEventLogLoader::invalidXML, QString("No Viewer fields found"));
        updateProgress(100,m_iStep);
        return false;
    }

    m_oGexYMEventLogSettings.setViewerFields(oFields);
    return true;
}

bool GexYMEventLogLoader::saveBasicFilters(QDomDocument &/*oEventTable*/){

    return true;
}

bool GexYMEventLogLoader::savePredinedFilters(QDomDocument &oXMLDoc){

    QDomNode oEventTable = oXMLDoc.namedItem("ym_events");
    if(oEventTable.isNull()){
        setError(GexYMEventLogLoader::invalidXML, QString("Invalid XML file"));
        return false;
    }


    QDomNode oPredifinedFilters = oEventTable.namedItem("predifined_filters");
    if(!oPredifinedFilters.isNull()){
        //remove and create again
        oEventTable.removeChild(oPredifinedFilters);
    }
    QDomElement oNewPredifinedFilters = oXMLDoc.createElement("predifined_filters");

    QMap<int, GexYMEventExpressionLogFilter *> & oPredefinedFiltersMap = m_oGexYMEventLogSettings.getPredefinedFilters();
    foreach(int iKey, oPredefinedFiltersMap.keys()){
        GexYMEventExpressionLogFilter *poExpression = oPredefinedFiltersMap[iKey];
        QDomElement oElem = oXMLDoc.createElement("filter");
        poExpression->save(oElem, m_oXMLDoc);
        oNewPredifinedFilters.appendChild(oElem);

    }

    QMap<int, GexYMEventExpressionLogFilter *> & oNewFiltersMap = m_oGexYMEventLogSettings.getNewUserFilters();
    foreach(int iKey, oNewFiltersMap.keys()){
        GexYMEventExpressionLogFilter *poExpression = oNewFiltersMap[iKey];
        QDomElement oElem = oXMLDoc.createElement("filter");
        poExpression->save(oElem, m_oXMLDoc);
        oNewPredifinedFilters.appendChild(oElem);
    }

    oEventTable.appendChild(oNewPredifinedFilters);
    oXMLDoc.replaceChild(oEventTable, oEventTable);
    return true;
}

bool GexYMEventLogLoader::saveExtensions(QDomDocument &/*oEventTable*/){
    return true;
}

bool GexYMEventLogLoader::saveViewer(QDomDocument &/*oEventTable*/){
    return true;
}

bool GexYMEventLogLoader::saveEventTableConfig(){

    m_iStepCount = 1;
    m_iStep = 0;
    updateProgress(100, m_iStep);

    if(!savePredinedFilters(m_oXMLDoc))
        return false;

    QString strFileDir = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
            +QDir::separator()+ "xml" + QDir::separator() ;
    QString strFileName = "ym_event_table.xml";
    QString strDestFilname = strFileDir + strFileName;


    QFile oFile(strDestFilname);
    if(!oFile.open(QIODevice::WriteOnly)){
        setError(GexYMEventLogLoader::invalidXML, "XML file can not be saved");
        updateProgress(100,0);
        oFile.close();
        return false;
    }

    QTextStream oStream( &oFile );
    m_oXMLDoc.save(oStream, 4);
    oFile.close();

    updateProgress(100,5);
    return true;
}

bool GexYMEventLogLoader::loadFilters(){


    updateProgress(0,0);
    bool bCopyFile = false;
    QString strDestFilname;

    QString strFileDir = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
            +QDir::separator()+ "xml" + QDir::separator() ;
    QString strFileName = "ym_event_table.xml";

    if(!QFile::exists(strFileDir + strFileName)){
        bCopyFile = true;
        strDestFilname = strFileDir + strFileName;
        strFileName = ":/gex/xml/ym_event_table.xml";
    }else
        strFileName = strFileDir + strFileName;

    m_iStepCount = 1;
    QFile oFile(strFileName);
    if(!oFile.open(QIODevice::ReadOnly)){
        setError(GexYMEventLogLoader::invalidXML, "XML file not found");
        updateProgress(100,0);
        return false;
    }


    if(bCopyFile){
        QDir oDir(strFileDir);
        bool bIsOk = false;
        if(!(bIsOk = oDir.exists())){
            if((bIsOk = oDir.mkpath(strFileDir)))
                bIsOk = QFile::copy ( strFileName, strDestFilname);
        }else
            bIsOk = QFile::copy ( strFileName, strDestFilname);
        if(!bIsOk){
            setError(GexYMEventLogLoader::invalidPath, "Can not create Yield Man Admin DB viewer configuration file");
            updateProgress(100,0);
            return false;
        }else{
            oFile.close();
            QFile::setPermissions(strDestFilname, QFile::ReadOwner|QFile::WriteOwner);
            oFile.setFileName(strDestFilname);
            if(!oFile.open(QIODevice::ReadOnly)){
                setError(GexYMEventLogLoader::invalidXML, "XML file not found");
                updateProgress(100,0);
                return false;
            }
        }
    }

    QString strErrorMessage;
    int iErrorLine=-1, iErrorColumn=-1;
    if(!m_oXMLDoc.setContent(&oFile, &strErrorMessage, &iErrorLine, &iErrorColumn)){
        setError(GexYMEventLogLoader::invalidXML, QString("Invalid XML file Error(%1) at Line(%2) and Column(%3)").arg(strErrorMessage).arg(iErrorLine).arg(iErrorColumn));
        updateProgress(100,0);
        return false;
    }

    QDomNode oEventTable = m_oXMLDoc.namedItem("ym_events");
    if(oEventTable.isNull()){
        setError(GexYMEventLogLoader::invalidXML, QString("Invalid XML file"));
        return false;
    }
    m_iStepCount = 5;
    m_iStep = 0;

    updateProgress(100, m_iStep);
    //Init Gui Label
    if(!isCompatible(oEventTable.toElement().attribute("version","NA"))){
        setError(GexYMEventLogLoader::invalidXML, QString("XML file not aligned with current event table"));
        return false;
    }
    m_oGexYMEventLogSettings.setLogLabel(oEventTable.toElement().attribute("label","Events Logs"));
    m_oGexYMEventLogSettings.setLogIcon(oEventTable.toElement().attribute("icon",":/gex/icons/db_events_log.png"));

    //loop to find filters
    m_iStep = 1;
    if(!loadBasicFilters(oEventTable))
        return false;

    m_iStep = 2;
    if(!loadPredinedFilters(oEventTable))
        return false;

    m_iStep = 3;
    if(!loadExtensions(oEventTable))
        return false;

    m_iStep = 4;
    if(!loadViewer(oEventTable))
        return false;

    oFile.close();

    updateProgress(100,5);
    return true;
}

//-------------------------------------GexYMEventLogView----------------------


//----------------------------GexYMEventLogDB------------------------------
GexYMEventLogDB *GexYMEventLogDB::m_poInstance = 0;
QString GexYMEventLogDB::m_strSeparator = "*!";

GexYMEventLogDB *GexYMEventLogDB::getInstance(){
    if(!m_poInstance)
        m_poInstance = new GexYMEventLogDB(&GS::Gex::Engine::GetInstance().GetAdminEngine());
    return m_poInstance;
}

void GexYMEventLogDB::deleteInstance(){
    if (m_poInstance){
        delete m_poInstance;
        m_poInstance = NULL;
    }
}

int     GexYMEventLogDB::m_iGexYMEventLogDBCount = 0;
GexYMEventLogDB::GexYMEventLogDB(GS::Gex::AdminEngine *poYM){
    m_poAdminEngine = poYM;
    m_iGexYMEventLogDBCount++;

}

GexYMEventLogDB::~GexYMEventLogDB(){
    m_poAdminEngine = 0;
    m_iGexYMEventLogDBCount--;
}

QStringList GexYMEventLogDB::getFieldValues(const QString &strTable, const QString &strField){

    QString strQuery;
    GexQueryThread oNodeQuery(0, m_poAdminEngine->m_pDatabaseConnector->m_strConnectionName);
    QStringList oValuesList;

    strQuery = QString("SELECT DISTINCT %1 FROM %2 ").arg(strField).arg(strTable);
    strQuery = QString("SELECT %1 FROM %2 GROUP BY %3").arg(strField).arg(strTable).arg(strField);

    if(!oNodeQuery.exec(strQuery))
    {
        return QStringList();
    }

    while(oNodeQuery.getQueryResult()->next()){
        QString strVal = oNodeQuery.getQueryResult()->value(0).toString().trimmed();
        oValuesList.append(strVal);
    }
    return oValuesList;
}

QStringList GexYMEventLogDB::getFieldValues(const QString &strTable1, const QString &strTable2, const QString &strFieldJoin, const  QString &strField){

    QString strQuery;
    GexQueryThread oNodeQuery(0, m_poAdminEngine->m_pDatabaseConnector->m_strConnectionName);
    QStringList oValuesList;

    // With this join, ignore all original name not already present into the ym_events table
    // Time consumming when the ym_events will be very BIG
    strQuery =  QString("SELECT %1, %2 \n").arg(QString("%1.%2").arg(strTable1).arg(strFieldJoin)).arg(strField);
    strQuery += QString("FROM %1, %2 \n").arg(strTable1).arg(strTable2);
    strQuery += QString("WHERE %1 = %2 \n").arg(QString("%1.%2").arg(strTable1).arg(strFieldJoin)).arg(QString("%1.%2").arg(strTable2).arg(strFieldJoin));
    strQuery += QString("GROUP BY %1").arg(QString("%1.%2").arg(strTable1).arg(strFieldJoin));

    // Just return the complet list of original name from the original table
    strQuery =  QString("SELECT %1, %2 \n").arg(strFieldJoin).arg(strField);
    strQuery += QString("FROM %1 \n").arg(strTable2);

    if(!oNodeQuery.exec(strQuery))
    {
        return QStringList();
    }

    while(oNodeQuery.getQueryResult()->next()){
        QString strJoinRes = oNodeQuery.getQueryResult()->value(0).toString().trimmed();
        QString strFieldRes = oNodeQuery.getQueryResult()->value(1).toString().trimmed();
        oValuesList.append(strJoinRes+m_strSeparator+strFieldRes);
    }
    return oValuesList;

}

QStringList GexYMEventLogDB::getMainEventsList(const QStringList &oFields, const QString &strLimitTo, QStringList &oLinkedEventList){

    QString strQuery;
    GexQueryThread oNodeQuery(0, m_poAdminEngine->m_pDatabaseConnector->m_strConnectionName);
    QStringList oValuesList;
    // Add \"\" for all reserved words
    // For oracle: size is a reserved word
    // To extract the speed value, we need to replace 'speed' by 'size/duration'
    // For oracle: div by 0 is not allowed
    // For mysql: the result is NULL
    QString Fields;
    QString Speed = "(size/(duration+0.1)) AS speed";
    if(m_poAdminEngine->m_pDatabaseConnector->IsOracleDB())
        Speed = "(\"size\"/(duration+0.1)) AS speed";
    foreach(QString Word,oFields){
        Word = Word.toLower();
        if(Word == "speed")
            Word = Speed;
        else if(Word == "type")
            Word = "type"; // exception ?
        else if(m_poAdminEngine->m_pDatabaseConnector->m_lstSqlReservedWords.contains(Word.toUpper()))
            Word = QString("\"%1\"").arg(Word);
        if(!Fields.isEmpty()) Fields += ",";
        Fields += Word;
    }


    if(!oFields.isEmpty())
        strQuery =  QString("SELECT event_id, link, %1 \nFROM ym_events \n").arg(Fields);
    else
        strQuery =  QString("SELECT event_id, link \nFROM ym_events \n");

    // Get all events from this Limit
    if(!strLimitTo.isEmpty())
        strQuery += QString("WHERE %1 \n").arg(strLimitTo);
    strQuery += QString("ORDER BY event_id DESC");

    if(!oNodeQuery.exec(strQuery))
    {
        return QStringList();
    }

    QMap<QString,QString> StatusEvent;
    QMap<QString,QStringList> LinkedEvent;
    while(oNodeQuery.getQueryResult()->next()){
        int OffSet = 2;
        QString strEvents = oNodeQuery.getQueryResult()->value(0).toString().trimmed();
        QString strLink = oNodeQuery.getQueryResult()->value(1).toString().trimmed();
        if(strLink.isEmpty() || strLink=="0")
        {
            // Link event is null
            // This is a Main event
            QString strMainEvent = strEvents;
            // Add 2 to iIdx - ignore the event_id, link fields
            for(int iIdx=0; iIdx<oFields.count(); iIdx++){
                QString strFieldValue;
                if(oNodeQuery.getQueryResult()->value(iIdx+OffSet).isValid() && !oNodeQuery.getQueryResult()->value(iIdx+OffSet).isNull())
                    strFieldValue = oNodeQuery.getQueryResult()->value(iIdx+OffSet).toString().trimmed();
                // Use status and summary from StatusEvent[MainEvent] if exist
                if((oFields[iIdx] == "status") && (StatusEvent.contains(strMainEvent)))
                    strFieldValue = StatusEvent[strMainEvent].section(m_strSeparator,iIdx,iIdx);
                if((oFields[iIdx] == "summary") && (StatusEvent.contains(strMainEvent)))
                    strFieldValue = StatusEvent[strMainEvent].section(m_strSeparator,iIdx,iIdx);
                if((oFields[iIdx] == "duration") && (StatusEvent.contains(strMainEvent)))
                    strFieldValue = StatusEvent[strMainEvent].section(m_strSeparator,iIdx,iIdx);
                if((oFields[iIdx] == "speed") && (StatusEvent.contains(strMainEvent)))
                    strFieldValue = StatusEvent[strMainEvent].section(m_strSeparator,iIdx,iIdx);
                if((oFields[iIdx] == "size") && (StatusEvent.contains(strMainEvent)))
                    strFieldValue = StatusEvent[strMainEvent].section(m_strSeparator,iIdx,iIdx);
                strEvents += m_strSeparator + strFieldValue;
            }
            oValuesList.append(strEvents);
            // Ignore the linked events, we already have the last status, summary
            oLinkedEventList.append(QString());
            // Remove linked events
            // the main and all linked to the main
            StatusEvent.remove(strMainEvent);
            foreach(strLink, LinkedEvent[strMainEvent])
                StatusEvent.remove(strLink);
            LinkedEvent.remove(strMainEvent);
        }
        else
        {
            LinkedEvent[strLink] << strEvents;
            // Link event associated with the Main event
            // Keep info from this line
            if(!StatusEvent.contains(strLink)){
                strEvents = "";
                for(int iIdx=0; iIdx<oFields.count(); iIdx++){
                    QString strFieldValue;
                    if(oNodeQuery.getQueryResult()->value(iIdx+OffSet).isValid() && !oNodeQuery.getQueryResult()->value(iIdx+OffSet).isNull())
                        strFieldValue = oNodeQuery.getQueryResult()->value(iIdx+OffSet).toString().trimmed();
                    strEvents += strFieldValue + m_strSeparator;
                }
                StatusEvent[strLink] = strEvents;
            }
            // If Status is PASS and have WARNING
            // Use it
            else {
                strEvents = "";
                for(int iIdx=0; iIdx<oFields.count(); iIdx++){
                    QString strFieldValue;
                    if(oFields[iIdx] == "status"){
                        if(oNodeQuery.getQueryResult()->value(iIdx+OffSet).toString().trimmed() != "WARNING")
                            break;
                        // Replace PASS to WARNING
                        strFieldValue = StatusEvent[strLink];
                        StatusEvent[strLink] = strFieldValue.replace(m_strSeparator+"PASS"+m_strSeparator,m_strSeparator+"WARNING"+m_strSeparator);
                        break;
                    }
                }
            }
        }
    }

    // Special case for AutoRefresh if an MainEvent is logged but not ended
    foreach(QString strEvents , StatusEvent.keys())
    {
        QString strEventValues = StatusEvent[strEvents];
        for(int iIdx=0; iIdx<oFields.count(); iIdx++)
        {
            QString strFieldValue;
            strFieldValue = strEventValues.section(m_strSeparator,iIdx,iIdx);
            strEvents += m_strSeparator + strFieldValue;
        }
        oValuesList.append(strEvents);
        // Ignore the linked events, we already have the last status, summary
        oLinkedEventList.append(QString());
    }
    return oValuesList;

}
QString GexYMEventLogDB::getLinkedEventsStatus(const QString &strEvent){

    QString strQuery;
    GexQueryThread oNodeQuery(0, m_poAdminEngine->m_pDatabaseConnector->m_strConnectionName);

    QString strStatusEvent ;
    strQuery =  QString("SELECT status  \nFROM ym_events \n");
    strQuery += QString("WHERE link = %1 \n").arg(strEvent.toInt());
    strQuery += QString("ORDER BY event_id DESC ");
    //qDebug(QString("strQuery: %1").arg(strQuery));
    if(!oNodeQuery.exec(strQuery))
    {
        return QString();
    }

    while(oNodeQuery.getQueryResult()->next()){
        if(oNodeQuery.getQueryResult()->value(0).isNull()
                || !oNodeQuery.getQueryResult()->value(0).isValid()
                || oNodeQuery.getQueryResult()->value(0).toString().isEmpty() )
            strStatusEvent = "";
        else
            strStatusEvent = oNodeQuery.getQueryResult()->value(0).toString().trimmed();
        //qDebug(QString("strStatusEvent : %1").arg(strStatusEvent));
        break;
    }

    return strStatusEvent;
}


QStringList GexYMEventLogDB::getLinkedEventsListTo(const QString &strEvent,const QStringList &strFields){

    QString strQuery;
    GexQueryThread oNodeQuery(0, m_poAdminEngine->m_pDatabaseConnector->m_strConnectionName);
    QStringList oValuesList;

    // Get all linked event_id
    QStringList oEventsList;
    //    2012-11-13 17:16:55	11055		EXECUTION	PATPUMP
    //    2012-11-13 17:16:56	11057	11055	EXECUTION	PATPUMP
    //    2012-11-13 17:16:56	11058	11055	EXECUTION	OUTLIER_REMOVAL
    //    2012-11-13 17:16:56	11059	11058	EXECUTION	OUTLIER_REMOVAL
    //    2012-11-13 17:16:56	11060	11055	EXECUTION	PATPUMP
    strQuery =  QString("SELECT event_id \nFROM ym_events \n");
    strQuery += QString("WHERE (event_id=%1) OR (link = %1)\n").arg(strEvent.toInt());
    if(!oNodeQuery.exec(strQuery))
    {
        return QStringList();
    }
    while(oNodeQuery.getQueryResult()->next())
        oEventsList += oNodeQuery.getQueryResult()->value(0).toString().trimmed();

    // Add \"\" for all reserved words
    // For oracle: size is a reserved word
    // To extract the speed value, we need to replace 'speed' by 'size/duration'
    // For oracle: div by 0 is not allowed
    // For mysql: the result is NULL
    QString Fields;
    QString Speed = "(size/(duration+0.1)) AS speed";
    if(m_poAdminEngine->m_pDatabaseConnector->IsOracleDB())
        Speed = "(\"size\"/(duration+0.1)) AS speed";
    foreach(QString Word,strFields)
    {
        Word = Word.toLower();
        if(Word == "speed")
            Word = Speed;
        else if(Word == "type")
            Word = "type"; // exception ?
        else if(m_poAdminEngine->m_pDatabaseConnector->m_lstSqlReservedWords.contains(Word.toUpper()))
            Word = QString("\"%1\"").arg(Word);
        if(!Fields.isEmpty()) Fields += ",";
        Fields += Word;
    }


    // Get all linked events (also 11059)
    if(!strFields.isEmpty())
        strQuery =  QString("SELECT event_id, %1 \nFROM ym_events \n").arg(Fields);
    else
        strQuery =  QString("SELECT event_id \nFROM ym_events \n");

    strQuery += QString("WHERE (event_id=%1) OR (link IN (%2))\n").arg(strEvent.toInt()).arg(oEventsList.join(","));
    strQuery += QString("ORDER BY event_id ASC ");

    if(!oNodeQuery.exec(strQuery))
    {
        return QStringList();
    }

    while(oNodeQuery.getQueryResult()->next()){
        QString strLinkedEvents;
        strLinkedEvents += oNodeQuery.getQueryResult()->value(0).toString().trimmed();
        for(int iIdx=1; iIdx<strFields.count()+1; iIdx++){
            strLinkedEvents += m_strSeparator + oNodeQuery.getQueryResult()->value(iIdx).toString().trimmed();
        }
        if(!strLinkedEvents.isEmpty())
            oValuesList.append(strLinkedEvents);
    }
    return oValuesList;
}

QStringList GexYMEventLogDB::getResult(const QStringList &strFields, GexYMEventExpressionLogFilter *poFilter){
    QString strQuery;
    GexQueryThread oNodeQuery(0, m_poAdminEngine->m_pDatabaseConnector->m_strConnectionName);
    QStringList oValuesList;

    // Add \"\" for all reserved words
    // For oracle: size is a reserved word
    // To extract the speed value, we need to replace 'speed' by 'size/duration'
    // For oracle: div by 0 is not allowed
    // For mysql: the result is NULL
    QString Fields;
    QString Speed = "(size/(duration+0.1)) AS speed";
    if(m_poAdminEngine->m_pDatabaseConnector->IsOracleDB())
        Speed = "(\"size\"/(duration+0.1)) AS speed";
    foreach(QString Word,strFields)
    {
        Word = Word.toLower();
        if(Word == "speed")
            Word = Speed;
        else if(Word == "type")
            Word = "type"; // exception ?
        else if(m_poAdminEngine->m_pDatabaseConnector->m_lstSqlReservedWords.contains(Word.toUpper()))
            Word = QString("\"%1\"").arg(Word);
        if(!Fields.isEmpty())
            Fields += ",";
        Fields += Word;
    }


    strQuery =  QString("SELECT %1 \nFROM ym_events \n").arg(Fields);
    strQuery += QString("WHERE %1 AND (link is null)").arg(poFilter->buildWhereClause());

    if(!oNodeQuery.exec(strQuery))
    {
        return QStringList();
    }

    while(oNodeQuery.getQueryResult()->next()){
        QString strResult;
        strResult += oNodeQuery.getQueryResult()->value(0).toString().trimmed() ;
        for(int iIdx=1; iIdx<strFields.count(); iIdx++){
            strResult += m_strSeparator + oNodeQuery.getQueryResult()->value(iIdx).toString().trimmed();
        }
        oValuesList.append(strResult);
    }
    return oValuesList;
}
int GexYMEventLogBasicFilterWidget::m_iGexYMEventLogBasicFilterWidgetCount = 0;
GexYMEventLogBasicFilterWidget::GexYMEventLogBasicFilterWidget(QWidget *poParent,
                                                               GexYMEventBasicLogFilter *poFilter, GexYMEventLogLoader *poGexYMEventLog):
    QWidget(poParent){
    m_iGexYMEventLogBasicFilterWidgetCount++;
    m_poComboBoxFilters = 0;
    m_poComboBoxOperator= 0;
    m_poFieldChoice= 0;
    m_poRemoveFilter= 0;
    m_poIsDynamic = 0;
    m_poGexYMEventLog = poGexYMEventLog;
    m_poSingleFilter = poFilter;
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);

    QWidget *poWidget = this;
    QHBoxLayout *poHBox = new QHBoxLayout(poWidget);
    poHBox->setContentsMargins(0, 0, 0, 0);

    //filter name
    if(!poFilter){
        m_poComboBoxFilters = new QComboBox(poWidget);
        m_poComboBoxFilters->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
        m_poComboBoxFilters->setObjectName("filter_name");
        QMap<int, GexYMEventBasicLogFilter *> oFilters = m_poGexYMEventLog->getGexYMEventLogSettings().getFilters();
        QList<int> oFiltersIdx = oFilters.keys();
        qSort(oFiltersIdx);
        foreach (int iFilterIdx, oFiltersIdx) {
            GexYMEventBasicLogFilter * poFilter = m_poGexYMEventLog->getGexYMEventLogSettings().getFilters()[iFilterIdx];
            if(!poFilter->getIcon().isEmpty()){
                QIcon oIcon;
                oIcon.addFile(poFilter->getIcon(), QSize(10,10), QIcon::Normal, QIcon::Off);
                m_poComboBoxFilters->addItem(oIcon, poFilter->getLabel(),iFilterIdx);
            } else
                m_poComboBoxFilters->addItem(poFilter->getLabel(),iFilterIdx);

        }

        poHBox->addWidget(m_poComboBoxFilters,0,Qt::AlignTop);
    }else {
        QLabel *poLabel = new QLabel(poFilter->getLabel(), poWidget);
        poHBox->addWidget(poLabel,0,Qt::AlignTop);
    }

    //Build Operator List depending on filter type
    m_poComboBoxOperator = new QComboBox(poWidget);
    m_poComboBoxOperator->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_poComboBoxOperator->setObjectName("filter_operator");
    poHBox->addWidget(m_poComboBoxOperator,0,Qt::AlignTop);

    m_poFieldChoice = new QWidget(poWidget);
    m_poFieldChoice->setObjectName("field_choice");
    QHBoxLayout *poGridLayoutTemp = new QHBoxLayout(m_poFieldChoice);
    poGridLayoutTemp->setContentsMargins(0, 0, 0, 0);
    m_poFieldChoice->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::Fixed, QSizePolicy::Frame));//QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    poHBox->addWidget(m_poFieldChoice,0,Qt::AlignTop);

    buildOperatorField(0);

    if(!poFilter){

        m_poIsDynamic =  new QCheckBox("Dynamic", poWidget);
        poHBox->addWidget(m_poIsDynamic,0,Qt::AlignTop);

        m_poRemoveFilter = new QPushButton(poWidget);
        m_poRemoveFilter->setMaximumSize(QSize(16, 16));
        QIcon oIcon;
        oIcon.addFile(QString::fromUtf8(":/gex/icons/funnel_minus.png"), QSize(), QIcon::Normal, QIcon::Off);
        m_poRemoveFilter->setIcon(oIcon);
        m_poRemoveFilter->setIconSize(QSize(10, 10));
        m_poRemoveFilter->setFlat(true);
        poHBox->addWidget(m_poRemoveFilter,0,Qt::AlignTop);
    }

    QSpacerItem *poSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    poHBox->addSpacerItem(poSpacer);

    if(m_poComboBoxFilters)
        QObject::connect(m_poComboBoxFilters, SIGNAL(currentIndexChanged(int)), this, SLOT(buildOperatorField(int)));

    QObject::connect(m_poComboBoxOperator, SIGNAL(currentIndexChanged(int)), this, SLOT(buildFieldChoice(int)));

    if(m_poRemoveFilter)
        QObject::connect(m_poRemoveFilter, SIGNAL(clicked()), this, SLOT(removeFilter()));
    poWidget->adjustSize();
}

bool GexYMEventLogBasicFilterWidget::isDynamic(){
    if(m_poIsDynamic)
        return m_poIsDynamic->isChecked();
    return false;
}

void GexYMEventLogBasicFilterWidget::initValues(GexYMEventBasicLogFilterElem *poElem){
    //    if(m_poComboBoxFilters)
    //        QObject::disconnect(m_poComboBoxFilters, SIGNAL(currentIndexChanged(int)), this, SLOT(buildOperatorField(int)));
    //    if(m_poComboBoxOperator)
    //        QObject::disconnect(m_poComboBoxOperator, SIGNAL(currentIndexChanged(int)), this, SLOT(buildFieldChoice(int)));
    //    if(m_poRemoveFilter)
    //        QObject::disconnect(m_poRemoveFilter, SIGNAL(clicked()), this, SLOT(removeFilter()));

    int iOperatorIdx = m_poComboBoxOperator->findText(poElem->m_strOperand);
    m_poComboBoxOperator->setCurrentIndex(iOperatorIdx);

    if(m_poSingleFilter){

        //        QString strName = m_poSingleFilter->getXMLName();
        //        QString strOperator =  m_poComboBoxOperator->currentText();;
        //        QString strVal ;

        GexYMEventBasicLogFilter *poFilter = m_poSingleFilter;
        if(poFilter->getType() == GexYMEventBasicLogFilter::datetimeType){
            setDateValues(m_poFieldChoice, poElem->m_strValue);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::integerType){
            if(poFilter->getEnumList().isEmpty())
                setNumreicValues(m_poFieldChoice, poElem->m_strValue);
            else
                setEnumValues(m_poFieldChoice, poElem->m_strValue);

        }else if(poFilter->getType() == GexYMEventBasicLogFilter::enumType){
            setEnumValues(m_poFieldChoice, poElem->m_strValue);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::doubleType){
            setNumreicValues(m_poFieldChoice, poElem->m_strValue);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::textType){
            setTextValues(m_poFieldChoice, poElem->m_strValue);
        }
    }



    //    if(m_poComboBoxFilters)
    //        QObject::connect(m_poComboBoxFilters, SIGNAL(currentIndexChanged(int)), this, SLOT(buildOperatorField(int)));
    //    if(m_poComboBoxOperator)
    //        QObject::connect(m_poComboBoxOperator, SIGNAL(currentIndexChanged(int)), this, SLOT(buildFieldChoice(int)));

    //    if(m_poRemoveFilter)
    //        QObject::connect(m_poRemoveFilter, SIGNAL(clicked()), this, SLOT(removeFilter()));

}

void GexYMEventLogBasicFilterWidget::removeFilter(){

    if(parentWidget()){
        if(parentWidget()->layout())
            parentWidget()->layout()->removeWidget(this);
        deleteLater();
        setParent(0);
    }

}

void GexYMEventLogBasicFilterWidget::saveElemFilter(GexYMEventExpressionLogFilter *poExpressionFilter){

    //QComboBox *poName =  poWidget->findChild<QComboBox *>("filter_name");
    if(m_poSingleFilter){

        QString strName = m_poSingleFilter->getXMLName();
        QString strOperator =  m_poComboBoxOperator->currentText();;

        QString strVal ;
        GexYMEventBasicLogFilter *poFilter = m_poSingleFilter;
        if(poFilter->getType() == GexYMEventBasicLogFilter::datetimeType){
            strVal = toDateValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::integerType){
            if(poFilter->getEnumList().isEmpty())
                strVal = toNumreicValues(m_poFieldChoice);
            else
                strVal = toEnumValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::enumType){
            strVal = toEnumValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::doubleType){
            strVal = toNumreicValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::textType){
            strVal = toTextValues(m_poFieldChoice);
        }
        if(!isDynamic())
            poExpressionFilter->addElem(getOperator(), strName, strOperator, strVal );
        else
            poExpressionFilter->addDynamicElem(getOperator(), strName, strOperator, strVal );


    } else {

        GexYMEventBasicLogFilter *poFilter = m_poGexYMEventLog->getGexYMEventLogSettings().getFilters()[m_poComboBoxFilters->itemData(m_poComboBoxFilters->currentIndex()).toInt()];
        QString strName = poFilter->getXMLName();
        //QComboBox *poOperator =  poWidget->findChild<QComboBox *>("filter_operator");
        QString strOperator =  m_poComboBoxOperator->currentText();
        //QWidget *poValues =  poWidget->findChild<QWidget *>("field_choice");

        QString strVal ;
        if(poFilter->getType() == GexYMEventBasicLogFilter::datetimeType){
            strVal = toDateValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::integerType){
            if(poFilter->getEnumList().isEmpty())
                strVal = toNumreicValues(m_poFieldChoice);
            else
                strVal = toEnumValues(m_poFieldChoice);

        }else if(poFilter->getType() == GexYMEventBasicLogFilter::enumType){
            strVal = toEnumValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::doubleType){
            strVal = toNumreicValues(m_poFieldChoice);
        }else if(poFilter->getType() == GexYMEventBasicLogFilter::textType){
            strVal = toTextValues(m_poFieldChoice);
        }
        if(!isDynamic())
            poExpressionFilter->addElem(getOperator(), strName, strOperator, strVal );
        else
            poExpressionFilter->addDynamicElem(getOperator(), strName, strOperator, strVal );
    }
}

QString GexYMEventLogBasicFilterWidget::getOperator(){

    return QString("AND");
}

void GexYMEventLogBasicFilterWidget::buildOperatorField(int ){

    GexYMEventBasicLogFilter * poFilter = 0;
    if(m_poComboBoxFilters){
        int iFilterIdx = m_poComboBoxFilters->itemData(m_poComboBoxFilters->currentIndex()).toInt();
        poFilter = m_poGexYMEventLog->getGexYMEventLogSettings().getFilters()[iFilterIdx];
    }else {
        poFilter = m_poSingleFilter;
    }

    m_poComboBoxOperator->clear();
    if(!poFilter->getOperatorChoice().isEmpty()){
        m_poComboBoxOperator->addItems(poFilter->getOperatorChoice());
        buildFieldChoice(0);
    }

}

void GexYMEventLogBasicFilterWidget::buildFieldChoice(int ){
    //QComboBox *poComboFilter = poOperator->parent()->findChild<QComboBox *>("filter_name");
    GexYMEventBasicLogFilter * poFilter = 0;
    if(m_poSingleFilter){
        poFilter = m_poSingleFilter;
    }else {
        int iFilterIdx = m_poComboBoxFilters->itemData(m_poComboBoxFilters->currentIndex()).toInt();
        poFilter = m_poGexYMEventLog->getGexYMEventLogSettings().getFilters()[iFilterIdx];
    }
    //Depending in Operator stored in poOperator update GUI

    QLayoutItem *poChild = 0;
    while((poChild = m_poFieldChoice->layout()->takeAt(0))){
        QWidget *poItem = poChild->widget();
        if(poItem){
            poItem->setParent(0);
            delete poItem;
        }
        delete poChild;
    }
    delete m_poFieldChoice->layout();
    QHBoxLayout *poGridLayout = new QHBoxLayout();
    //poGridLayout->setSpacing(0);
    poGridLayout->setContentsMargins(0, 0, 0, 0);
    m_poFieldChoice->setLayout(poGridLayout);

    //QStringList oOperatorChoice;
    int iOperatorIdx = m_poComboBoxOperator->currentIndex();
    QString strOperatorText = m_poComboBoxOperator->currentText();

    if(poFilter->getType() == GexYMEventBasicLogFilter::datetimeType){

        if(iOperatorIdx >=0 && iOperatorIdx<=3)
            getNumreicValues(0, false,m_poFieldChoice, strOperatorText.section(" ",2,2));
        else if(iOperatorIdx == 5){

        }else {
            getDateValues(1, m_poFieldChoice, "<->");
        }
    }else if(poFilter->getType() == GexYMEventBasicLogFilter::integerType){
        if(poFilter->getFetchFrom().isEmpty() || poFilter->getEnumList().isEmpty()){
            if(iOperatorIdx == 0){
                getNumreicValues(0, false,m_poFieldChoice, "");
            }else if(iOperatorIdx == 1){
                getNumreicValues(0, true,m_poFieldChoice, "");
            }

        }else {
            if(iOperatorIdx == 0){
                getEnumValues(m_poFieldChoice, poFilter->getEnumList(), false,"");
            }else if(iOperatorIdx == 1){
                getEnumValues(m_poFieldChoice, poFilter->getEnumList(), true,"");
            }
        }
    }else if(poFilter->getType() == GexYMEventBasicLogFilter::enumType){
        if(iOperatorIdx == 0){
            getEnumValues(m_poFieldChoice, poFilter->getEnumList(), false,"");
        }else if(iOperatorIdx == 1){
            getEnumValues(m_poFieldChoice, poFilter->getEnumList(), true,"");
        }
    }else if(poFilter->getType() == GexYMEventBasicLogFilter::doubleType){
        getNumreicValues(1,false, m_poFieldChoice, "Label");

    }else if(poFilter->getType() == GexYMEventBasicLogFilter::textType){
        getTextValues(m_poFieldChoice, "");
    }

    m_poFieldChoice->adjustSize();
    poFilter = 0;

}

GexYMEventLogBasicFilterWidget::~GexYMEventLogBasicFilterWidget(){
    m_iGexYMEventLogBasicFilterWidgetCount--;
}

void GexYMEventLogBasicFilterWidget::setEnumValues(QWidget *poWidget, const QString &strValues){
    QListWidget *poListWidget = poWidget->findChild<QListWidget *>();
    if(!poListWidget)
        return ;
    QStringList oList = strValues.split(GexYMEventBasicLogFilterElem::m_strValuesSeparator);
    for(int iIdx=0; iIdx<poListWidget->count(); iIdx++){
        QListWidgetItem *poItem = poListWidget->item(iIdx);
        if(!poItem->data(Qt::UserRole).isNull()){
            if(oList.contains(poItem->data(Qt::UserRole).toString()))
                poItem->setSelected(true);
        } else {
            if(oList.contains(poItem->text()))
                poItem->setSelected(true);
        }
    }
}


QString GexYMEventLogBasicFilterWidget::toEnumValues(QWidget *poWidget){

    QListWidget *poListWidget = poWidget->findChild<QListWidget *>();
    QStringList oList;

    foreach(QListWidgetItem *poItem, poListWidget->selectedItems()){
        if(!poItem->data(Qt::UserRole).isNull()){
            oList.append(poItem->data(Qt::UserRole).toString());
        }
        else
            oList.append(poItem->text());

    }
    return oList.join(GexYMEventBasicLogFilterElem::m_strValuesSeparator);
}

QWidget *GexYMEventLogBasicFilterWidget::getEnumValues(QWidget *poParent, const QStringList &oValues, bool bSet, const QString &/*strText*/){

    QWidget *poChoose = new QWidget();
    poChoose->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    QHBoxLayout *poHBox = new QHBoxLayout(poChoose);
    //poHBox->setSpacing(6);
    poHBox->setContentsMargins(0, 0, 0, 0);

    QListWidget *poListWidget = new QListWidget(poParent);
    poHBox->addWidget(poListWidget,0,Qt::AlignTop);

    //poListWidget->addItem("Select All ...");
    foreach (const QString &strVal, oValues) {
        QListWidgetItem *poItem = new QListWidgetItem;
        if(strVal.contains(GexYMEventLogDB::m_strSeparator)){
            QStringList oItems = strVal.split(GexYMEventLogDB::m_strSeparator);
            poItem->setText(oItems[1]);
            poItem->setData(Qt::UserRole, oItems[0]);
        }else
            poItem->setText(strVal);
        poListWidget->addItem(poItem);
    }

    if(bSet)
        poListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    else
        poListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    poListWidget->setResizeMode(QListView::Fixed);
    QHBoxLayout *poGrid = qobject_cast<QHBoxLayout *>(poParent->layout());
    poChoose->setParent(poParent);
    poGrid->addWidget(poChoose);//, 0, poGrid->columnCount(), 1, 1,Qt::AlignTop);
    return poChoose;
}

void GexYMEventLogBasicFilterWidget::setTextValues(QWidget *poWidget, const QString &strValues){
    QLineEdit *poEdit = poWidget->findChild<QLineEdit *>();
    if(!poEdit)
        return;
    poEdit->setText(strValues);
}

QString  GexYMEventLogBasicFilterWidget::toTextValues(QWidget *poWidget){
    QLineEdit *poEdit = poWidget->findChild<QLineEdit *>();
    return poEdit->text();
}

QWidget *GexYMEventLogBasicFilterWidget::getTextValues(QWidget *poParent, const QString &/*strText*/){

    QWidget *poChoose = new QWidget;
    poChoose->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    QHBoxLayout *poHBox = new QHBoxLayout(poChoose);
    //poHBox->setSpacing(6);
    poHBox->setContentsMargins(0, 0, 0, 0);

    QLineEdit *poEdit = new QLineEdit(poParent);
    poHBox->addWidget(poEdit,0,Qt::AlignTop);

    QHBoxLayout *poGrid = qobject_cast<QHBoxLayout *>(poParent->layout());
    poGrid->addWidget(poChoose);//, 0, poGrid->columnCount(), 1, 1,Qt::AlignTop);
    poChoose->setParent(poParent);
    return poChoose;

}

void GexYMEventLogBasicFilterWidget::setDateValues(QWidget *poWidget, const QString &strValues){
    QDateTimeEdit *poS1 = poWidget->findChild<QDateTimeEdit *>("1");
    QDateTimeEdit *poS2 = poWidget->findChild<QDateTimeEdit *>("2");
    if(poS1){
        if(strValues.contains(GexYMEventBasicLogFilterElem::m_strValuesSeparator)){
            poS1->setDateTime(QDateTime::fromString(strValues.section(GexYMEventBasicLogFilterElem::m_strValuesSeparator,0,0), "yyyy-MM-dd hh:mm:ss"));
            if(poS2){
                poS2->setDateTime(QDateTime::fromString(strValues.section(GexYMEventBasicLogFilterElem::m_strValuesSeparator,1,1), "yyyy-MM-dd hh:mm:ss"));
            }
        }else
            poS1->setDateTime(QDateTime::fromString(strValues, "yyyy-MM-dd hh:mm:ss"));

    }else
        setNumreicValues(poWidget, strValues);
}

QString GexYMEventLogBasicFilterWidget::toDateValues(QWidget *poWidget){


    QDateTimeEdit *poS1 = poWidget->findChild<QDateTimeEdit *>("1");
    QDateTimeEdit *poS2 = poWidget->findChild<QDateTimeEdit *>("2");
    if(poS1)
        return QString("%1").arg(poS1->dateTime().toString("yyyy-MM-dd hh:mm:ss")) + (poS2 ? QString("%1%2").arg(GexYMEventBasicLogFilterElem::m_strValuesSeparator).arg(poS2->dateTime().toString("yyyy-MM-dd hh:mm:ss")): QString(""));

    return toNumreicValues(poWidget);

}

QWidget *GexYMEventLogBasicFilterWidget::getDateValues(bool bRange, QWidget *poParent, const QString &strText){

    QWidget *poChoose = new QWidget();
    poChoose->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    QHBoxLayout *poHBox = new QHBoxLayout(poChoose);
    //poHBox->setSpacing(6);
    poHBox->setContentsMargins(0, 0, 0, 0);

    QDateTimeEdit *poDateInf = new QDateTimeEdit(QDateTime::currentDateTime(), poChoose);
    poDateInf->setObjectName("1");
    poDateInf->setCalendarPopup(true);
    poHBox->addWidget(poDateInf,0,Qt::AlignTop);
    if(bRange){
        QLabel *poLabel = new QLabel(poChoose);
        poLabel->setText(strText);
        poHBox->addWidget(poLabel,0,Qt::AlignTop);

        QDateTimeEdit *poDateSup = new QDateTimeEdit(QDateTime::currentDateTime(), poChoose);
        poDateSup->setObjectName("2");
        poDateSup->setCalendarPopup(true);
        poHBox->addWidget(poDateSup,0,Qt::AlignTop);
    }

    QHBoxLayout *poGrid = qobject_cast<QHBoxLayout *>(poParent->layout());
    poGrid->addWidget(poChoose);///, 0, poGrid->columnCount(), 1, 1,Qt::AlignTop);
    poChoose->setParent(poParent);
    return poChoose;
}

void GexYMEventLogBasicFilterWidget::setNumreicValues(QWidget *poWidget, const QString &strValues){
    if(poWidget->objectName() == "Double"){
        QDoubleSpinBox *poS1 = poWidget->findChild<QDoubleSpinBox *>("1");
        if(!poS1)
            return;
        QDoubleSpinBox *poS2 = poWidget->findChild<QDoubleSpinBox *>("2");
        if(strValues.contains(GexYMEventBasicLogFilterElem::m_strValuesSeparator)){
            poS1->setValue(strValues.section(GexYMEventBasicLogFilterElem::m_strValuesSeparator,0,0).toDouble());
            if(poS2)
                poS2->setValue(strValues.section(GexYMEventBasicLogFilterElem::m_strValuesSeparator,1,1).toDouble());
        }else
            poS1->setValue(strValues.toDouble());
    }else{
        QSpinBox *poS1 = poWidget->findChild<QSpinBox *>("1");
        if(!poS1)
            return;
        QSpinBox *poS2 = poWidget->findChild<QSpinBox *>("2");
        if(strValues.contains(GexYMEventBasicLogFilterElem::m_strValuesSeparator)){
            poS1->setValue(strValues.section(GexYMEventBasicLogFilterElem::m_strValuesSeparator,0,0).toInt());
            if(poS2)
                poS2->setValue(strValues.section(GexYMEventBasicLogFilterElem::m_strValuesSeparator,1,1).toInt());
        }else
            poS1->setValue(strValues.toInt());

    }
}

QString  GexYMEventLogBasicFilterWidget::toNumreicValues(QWidget *poWidget){

    if(poWidget->objectName() == "Double"){
        QDoubleSpinBox *poS1 = poWidget->findChild<QDoubleSpinBox *>("1");
        QDoubleSpinBox *poS2 = poWidget->findChild<QDoubleSpinBox *>("2");
        if(poS1)
            return QString("%1").arg(poS1->value()) + (poS2 ? QString("%1%2").arg(GexYMEventBasicLogFilterElem::m_strValuesSeparator).arg(poS2->value()): QString(""));
    }else{
        QSpinBox *poS1 = poWidget->findChild<QSpinBox *>("1");
        QSpinBox *poS2 = poWidget->findChild<QSpinBox *>("2");
        if(poS1)
            return QString("%1").arg(poS1->value()) + (poS2 ? QString("%1%2").arg(GexYMEventBasicLogFilterElem::m_strValuesSeparator).arg(poS2->value()): QString(""));
    }
    return QString();
}

QWidget *GexYMEventLogBasicFilterWidget::getNumreicValues(int iType, bool bRange ,QWidget *poParent, const QString &strText){

    QWidget *poChoose = new QWidget();
    poChoose->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    QHBoxLayout *poHBox = new QHBoxLayout(poChoose);
    //poHBox->setSpacing(6);
    poHBox->setContentsMargins(0, 0, 0, 0);
    if(iType == 0){
        poChoose->setObjectName("Int");
        QSpinBox *poSpinBox = new QSpinBox(poChoose);
        poSpinBox->setObjectName("1");
        poSpinBox->setMaximum(999999999);
        poSpinBox->setValue(1);
        poHBox->addWidget(poSpinBox,0,Qt::AlignTop);
    }else if(iType == 1){
        poChoose->setObjectName("Double");
        QDoubleSpinBox *poSpinBox = new QDoubleSpinBox(poChoose);
        poSpinBox->setObjectName("1");
        poSpinBox->setDecimals(5);
        poSpinBox->setMinimum(-1e+09);
        poSpinBox->setMaximum(1e+09);
        poHBox->addWidget(poSpinBox,0,Qt::AlignTop);
    }

    if(bRange){
        QLabel *poLabel = new QLabel(poChoose);
        poLabel->setText("<->");
        poHBox->addWidget(poLabel,0,Qt::AlignTop);
        if(iType == 0){
            QSpinBox *poSpinBox = new QSpinBox(poChoose);
            poSpinBox->setObjectName("2");
            poSpinBox->setMaximum(999999999);
            poSpinBox->setValue(1);
            poHBox->addWidget(poSpinBox,0,Qt::AlignTop);
        }else if(iType == 1){
            QDoubleSpinBox *poSpinBox = new QDoubleSpinBox(poChoose);
            poSpinBox->setObjectName("2");
            poSpinBox->setDecimals(5);
            poSpinBox->setMinimum(-1e+09);
            poSpinBox->setMaximum(1e+09);
            poHBox->addWidget(poSpinBox,0,Qt::AlignTop);
        }
    }

    QLabel *poLabel = new QLabel(poChoose);
    poLabel->setText(strText);
    poHBox->addWidget(poLabel,0,Qt::AlignTop);

    QHBoxLayout *poGrid = qobject_cast<QHBoxLayout *>(poParent->layout());
    poChoose->setParent(poParent);
    poGrid->addWidget(poChoose);//, 0, poGrid->columnCount(), 1, 1,Qt::AlignTop);
    return poChoose;
}


int GexYMEventLogViewer::m_iGexYMEventLogViewerCount=0;
GexYMEventLogViewer::GexYMEventLogViewer(QWidget *poParent, GexYMEventLogSettings *poGexYMEventLogSettings, const QStringList &oFieldsList, GexYMEventExpressionLogFilter *poFilter)
    :QWidget(poParent){
    m_bRefreshingOnGoing = false;
    m_iGexYMEventLogViewerCount++;
    QVBoxLayout *poVBoxLayout = new QVBoxLayout(this);
    //poVBoxLayout->setSpacing(0);
    poVBoxLayout->setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *poHorizontalLayout = new QHBoxLayout();
    //Refresh buttons
    buildRefreshPart(poHorizontalLayout, this);

    QSpacerItem *poHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    poHorizontalLayout->addItem(poHorizontalSpacer);

    //Find Widgets
    m_poFindClear = new QPushButton(this);
    m_poFindClear->setFlat(true);
    QIcon oIcon2;
    oIcon2.addFile(":/gex/icons/event_log_clear_all.png", QSize(10,10), QIcon::Normal, QIcon::Off);
    m_poFindClear->setIcon(oIcon2);
    m_poFindClear->setMaximumSize(QSize(16, 16));
    m_poFindClear->hide();
    QObject::connect(m_poFindClear, SIGNAL(clicked()), this, SLOT(clearFind()));
    poHorizontalLayout->addWidget(m_poFindClear);


    m_poSearch = new QLineEdit(this);
    poHorizontalLayout->addWidget(m_poSearch);

    m_poUseRegExpression = new QCheckBox("Regular Expression", this);
    //m_poUseRegExpression->setMaximumSize(QSize(16, 16));
    QFont oFont1;
    oFont1.setPointSize(8);
    m_poUseRegExpression->setFont(oFont1);
    poHorizontalLayout->addWidget(m_poUseRegExpression);


    m_poFind = new QPushButton(this);
    m_poFind->setFlat(true);
    QIcon oIcon1;
    oIcon1.addFile(":/gex/icons/find.png", QSize(10,10), QIcon::Normal, QIcon::Off);
    m_poFind->setIcon(oIcon1);
    m_poFind->setMaximumSize(QSize(16, 16));
    QObject::connect(m_poFind, SIGNAL(clicked()), this, SLOT(findEntry()));
    poHorizontalLayout->addWidget(m_poFind);

    poHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    poHorizontalLayout->addItem(poHorizontalSpacer);

    //Find widgets


    //CSV
    m_poFilterExport = new QPushButton("Export to CSV",this);
    QIcon icon2;
    icon2.addFile(":/gex/icons/export_log.png", QSize(), QIcon::Normal, QIcon::Off);
    m_poFilterExport->setIcon(icon2);
    m_poFilterExport->setIconSize(QSize(10, 10));
    m_poFilterExport->setFlat(true);
    QObject::connect(m_poFilterExport, SIGNAL(clicked()), this, SLOT(exportToCSV()));
    poHorizontalLayout->addWidget(m_poFilterExport);
    //CSV

    poVBoxLayout->addLayout(poHorizontalLayout);

    //tree viewer
    m_poTreeWidget = new QTreeWidget(this);
    //m_poTreeWidget->setSortingEnabled(true);
    m_poTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    poVBoxLayout->addWidget(m_poTreeWidget);//,0,0,1,1);

    QObject::connect(m_poTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(viewDetails()));

    //Progress BAR
    m_poDBProgressBar = new QProgressBar(this);
    m_poDBProgressBar->setRange(0,-1);
    m_poDBProgressBar->setTextVisible(true);
    poVBoxLayout->addWidget(m_poDBProgressBar);
    m_poDBProgressBar->hide();
    //Progress BAR

    m_poFilter = 0;
    m_poGexYMEventLogSettings = poGexYMEventLogSettings;

    m_bAutoRefresh = false;
    m_poFilter = poFilter;
    setFieldsList(oFieldsList);

    QObject::connect(m_poFilterRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    setRefreshInterval(
                m_poFilterRefreshTime->value() *
                m_poFilterRefreshTimeUnits->itemData(m_poFilterRefreshTimeUnits->currentIndex()).toInt());
    QObject::connect(m_poFilterAutoRefresh, SIGNAL(stateChanged(int)), this, SLOT(setAutoreferesh(int)));

}

void GexYMEventLogViewer::updateProgressBar(int iValue){
    if(m_poDBProgressBar){
        if(iValue == -1){
            if(m_poDBProgressBar->value()+1 > m_poDBProgressBar->maximum())
                m_poDBProgressBar->setValue(int(0.75*m_poDBProgressBar->maximum()));
            m_poDBProgressBar->setValue(m_poDBProgressBar->value()+1);
        }else{
            m_poDBProgressBar->setValue(iValue);
        }
    }

}


void GexYMEventLogViewer::clearFind(){
    m_poSearch->clear();
    for(int iIdx=0; iIdx<m_poTreeWidget->topLevelItemCount(); iIdx++){
        QTreeWidgetItem *poItem = m_poTreeWidget->topLevelItem(iIdx);
        poItem->setHidden(false);
    }
    m_poFindClear->hide();
}

void GexYMEventLogViewer::exportToCSV(){
    if(!m_poTreeWidget->topLevelItemCount()){
        GS::Gex::Message::warning("GEX", "No log found to export");
        return ;
    }
    QString strDefaultPath =
            GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString()+QDir::separator();
    strDefaultPath += QString("events_log_%1.csv").arg(QDateTime::currentDateTime().toString("YYYY-MM-DD_HH-mm-ss"));

    QString strFileName = QFileDialog::getSaveFileName(this,
                                                       "Export To CSV",
                                                       strDefaultPath,
                                                       "CSV (*.csv)");
    if(strFileName.isEmpty())
        return;

    QFile oFile(strFileName);
    if(!oFile.open(QIODevice::WriteOnly)){
        return ;
    }
    QTextStream oStream;
    oStream.setDevice(&oFile);

    QString strLog ;
    strLog = "#CSV export of Yiel-Man Event LOG \n# version=0.0";
    oStream << strLog <<"\n";

    QTreeWidgetItem *poHeader = m_poTreeWidget->headerItem();
    strLog = poHeader->text(EventIDColumn);
    for(int iField = EventIDColumn+1; iField<poHeader->columnCount();iField++){
        strLog+="," + poHeader->text(iField);
    }
    strLog+=",details";
    oStream << strLog <<"\n";

    for(int iLogIdx=0; iLogIdx<m_poTreeWidget->topLevelItemCount();iLogIdx++){
        QTreeWidgetItem *poItem = m_poTreeWidget->topLevelItem(iLogIdx);
        if(!poItem)
            continue;
        QString strLog = poItem->text(EventIDColumn);
        for(int iTextIdx = EventIDColumn+1;iTextIdx<poItem->columnCount(); iTextIdx++)
            strLog += "," + poItem->text(iTextIdx);
        if(poItem->childCount() && m_poTreeWidget->itemWidget(poItem->child(0), 0)){
            QTextBrowser *poBrowser = qobject_cast<QTextBrowser *>(m_poTreeWidget->itemWidget(poItem->child(0), 0));
            if(poBrowser)
                strLog+= ";" + poBrowser->toPlainText();
        }

        oStream << strLog <<"\n";
    }

    oFile.close();
}



void GexYMEventLogViewer::findEntry(){
    if(m_poSearch->text().isEmpty() || !m_poTreeWidget->topLevelItemCount())
        return ;

    bool bUseRegExpression = m_poUseRegExpression->isChecked();

    QList<QTreeWidgetItem *> oItemList;
    for(int iColumn = EventIDColumn; iColumn<m_poTreeWidget->columnCount(); iColumn++){
        QList<QTreeWidgetItem *> oItemCurrentList = m_poTreeWidget->findItems(m_poSearch->text(),
                                                                              bUseRegExpression ? (Qt::MatchRegExp|Qt::MatchWildcard) : Qt::MatchContains ,
                                                                              iColumn);
        foreach(QTreeWidgetItem *poItem, oItemCurrentList){
            if(!oItemList.contains(poItem))
                oItemList.append(poItem);
        }
    }

    if(oItemList.isEmpty()){
        for(int iIdx=0; iIdx<m_poTreeWidget->topLevelItemCount(); iIdx++){
            if(m_poTreeWidget->topLevelItem(iIdx)->childCount()){
                QTextBrowser *poBrowser = qobject_cast<QTextBrowser *>(
                            m_poTreeWidget->itemWidget(m_poTreeWidget->topLevelItem(iIdx)->child(0), 0));
                if(poBrowser){
                    if(!bUseRegExpression){
                        if(poBrowser->find(m_poSearch->text())){
                            oItemList.append(m_poTreeWidget->topLevelItem(iIdx));
                        }
                    }else {
                        QRegExp oTextTofind(m_poSearch->text(), Qt::CaseInsensitive);
                        oTextTofind.setPatternSyntax(QRegExp::Wildcard);
                        QTextCursor oFindResult = poBrowser->document()->find(oTextTofind);
                        if(!oFindResult.isNull()){
                            poBrowser->setTextCursor(oFindResult);
                            oItemList.append(m_poTreeWidget->topLevelItem(iIdx));
                        }
                    }
                }
            }
        }
    }

    if(oItemList.isEmpty())
        return ;
    for(int iIdx=0; iIdx<m_poTreeWidget->topLevelItemCount(); iIdx++){
        QTreeWidgetItem *poItem = m_poTreeWidget->topLevelItem(iIdx);
        if(!oItemList.contains(poItem))
            poItem->setHidden(true);
    }

    m_poFindClear->show();
}

GexYMEventLogViewer::~GexYMEventLogViewer(){
    m_iGexYMEventLogViewerCount--;

}

void GexYMEventLogViewer::enableRefreshButton(bool bEnable){
    m_poFilterRefresh->setEnabled(bEnable);
    m_poFilterRefreshTime->setEnabled(bEnable);
    m_poFilterRefreshTimeUnits->setEnabled(bEnable);
}

void GexYMEventLogViewer::refresh (){
    autorefresh();
    if(m_bAutoRefresh){
        enableRefreshButton(false);
        m_oTimer.start();
    }
    else{
        m_oTimer.stop();
    }
}

void GexYMEventLogViewer::setAutoreferesh(int iAuto){
    m_bAutoRefresh = false;
    if(iAuto == Qt::Checked){
        m_bAutoRefresh = true;
    }
    enableGui(true);
    setRefreshInterval(getRefreshTime());
    m_poFilterRefreshTimeUnits->setEnabled(!m_bAutoRefresh);
    m_poFilterRefreshTime->setEnabled(!m_bAutoRefresh);
    if(m_bAutoRefresh)
        QObject::connect(&m_oTimer, SIGNAL(timeout()), this, SLOT(autorefresh()));
    else
        QObject::disconnect(&m_oTimer, SIGNAL(timeout()), this, SLOT(autorefresh()));

}

void GexYMEventLogViewer::setRefreshInterval(int iSec){
    m_oTimer.setInterval(iSec * 1000);
}

bool GexYMEventLogViewer::getAutorefresh(){
    return m_bAutoRefresh;
}

void GexYMEventLogViewer::setFieldsList(const QStringList &oFieldsList){
    m_oFieldsList = oFieldsList;
    QTreeWidgetItem *poHeader = new QTreeWidgetItem;

    poHeader->setText(StatusColumn, "");//0 status icon
    poHeader->setText(DetailColumn, "");//1 details icon
    poHeader->setText(EventIDColumn, m_poGexYMEventLogSettings->getFriendlyName("event_id"));//1 event id column
    int iIdx = EventIDColumn+1;
    foreach(const QString &strHeaderItem, m_oFieldsList){
        poHeader->setText(iIdx++, m_poGexYMEventLogSettings->getFriendlyName(strHeaderItem));
    }

    m_poTreeWidget->setHeaderItem(poHeader);
    m_poTreeWidget->clear();

}

void GexYMEventLogViewer::addHtmlTableRow(const QString &strLablel, const QString &strData, QString &strHtmlDetails){
    strHtmlDetails += "<tr>";
    strHtmlDetails += QString("<td width=\"80\" bgcolor=\"#CCECFF\"><b>%1</b></td>").arg(strLablel);
    strHtmlDetails += QString("<td bgcolor=\"#F8F8F8\">%1</td>").arg(QString(strData).replace("\n","<BR>\n").replace("|","<BR>\n"));
    strHtmlDetails += "</tr>";
}

void GexYMEventLogViewer::generateDetails(QTreeWidgetItem *poMainEvents, const QStringList &/*oLinkedEventList*/){

    QString strHtmlDetails = "<font color=\"#006699\"><b>Event Log Details</b><br>";
    strHtmlDetails += "<table border=\"0\" width=\"98%\" cellspacing=\"1\">";

    QStringList oFieldsDetails = m_oFieldsList;
    if(!oFieldsDetails.contains("status")){
        oFieldsDetails.append("status");
    }
    if(!oFieldsDetails.contains("summary")){
        oFieldsDetails.append("summary");
    }

    int iTimeIdx = oFieldsDetails.indexOf("creation_time")+1;
    int iTaskIdx = oFieldsDetails.indexOf("task_id")+1;
    int iTypeIdx = oFieldsDetails.indexOf("type")+1;
    int iStatusIdx = oFieldsDetails.indexOf("status")+1;
    int iSummaryIdx = oFieldsDetails.indexOf("summary")+1;

    QStringList oEventDetails = GexYMEventLogDB::getInstance()->getLinkedEventsListTo(poMainEvents->text(EventIDColumn), oFieldsDetails);

    if(!oEventDetails.isEmpty()){
        QString strTaskName;
        foreach(const QString &strDetail, oEventDetails){
            QStringList oLastEventDetailsEntry = strDetail.split( GexYMEventLogDB::getInstance()->m_strSeparator);
            QString strName = getFriendlyName(iTaskIdx-1, oLastEventDetailsEntry[iTaskIdx]);
            if(strTaskName != strName)
            {
                strTaskName = strName;
                if(strName == oLastEventDetailsEntry[iTaskIdx])
                    strName = oLastEventDetailsEntry[iTypeIdx];
                addHtmlTableRow(m_poGexYMEventLogSettings->getFriendlyName("task_id"), QString("<b>%1</b>").arg(strName), strHtmlDetails);
            }
            addHtmlTableRow(oLastEventDetailsEntry[iTimeIdx].section("T",1), oLastEventDetailsEntry[iStatusIdx], strHtmlDetails);
            if(!oLastEventDetailsEntry[iSummaryIdx].isEmpty())
                addHtmlTableRow(m_poGexYMEventLogSettings->getFriendlyName("summary"),oLastEventDetailsEntry[iSummaryIdx], strHtmlDetails);
            //int  iFieldIdx = -1;
            //if((iFieldIdx = m_oFieldsList.indexOf("status"))!=-1){
            //addHtmlTableRow(m_poGexYMEventLogSettings->getFriendlyName("status"), oLastEventDetailsEntry[iStatusIdx], strHtmlDetails);
            //}
            //if((iFieldIdx = m_oFieldsList.indexOf("summary"))!=-1){
            //addHtmlTableRow(m_poGexYMEventLogSettings->getFriendlyName("summary"), oLastEventDetailsEntry[iSummaryIdx], strHtmlDetails);
            //}
        }
    }
    else {
        addHtmlTableRow(m_poGexYMEventLogSettings->getFriendlyName("task_id"), QString("<b>%1</b>").arg(poMainEvents->text(EventIDColumn+iTaskIdx)), strHtmlDetails);
        addHtmlTableRow(poMainEvents->text(EventIDColumn+iTimeIdx).section("T",1), poMainEvents->text(EventIDColumn+iStatusIdx), strHtmlDetails);
        if(!poMainEvents->text(EventIDColumn+iSummaryIdx).isEmpty())
            addHtmlTableRow(m_poGexYMEventLogSettings->getFriendlyName("summary"), poMainEvents->text(EventIDColumn+iSummaryIdx), strHtmlDetails);
    }
    strHtmlDetails += "</table>";

    QTextBrowser *poHtml = new QTextBrowser();
    poHtml->setHtml(strHtmlDetails);
    poHtml->reload();
    poHtml->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    poHtml->setContentsMargins(0,0,0,0);
    poHtml->resize(poHtml->document()->documentLayout()->documentSize().toSize());
    QTreeWidgetItem *poDetails = new QTreeWidgetItem(poMainEvents);
    m_poTreeWidget->setItemWidget(poDetails, 0, poHtml);
    m_poTreeWidget->setFirstItemColumnSpanned(poDetails, true);
}


void GexYMEventLogViewer::viewDetails(){
    // Check if DB list has some elements
    if(m_poTreeWidget->topLevelItemCount() == 0)
        return;

    // Get selected database
    QTreeWidgetItem		*poItem;
    // Get selected item
    poItem = (m_poTreeWidget->selectedItems()).isEmpty() ? NULL : (m_poTreeWidget->selectedItems()).first();
    if(poItem == NULL) return;

    // 2 cases from Main item or Details item
    if(poItem->columnCount() > DetailColumn){
        // 1- this is a Main item
        // Check if expand allowed
        if(poItem->icon(DetailColumn).isNull()) return;

        //Populate Details
        if(!poItem->childCount())
            generateDetails(poItem, QStringList());
        else
            poItem->setExpanded(poItem->isExpanded());
    }
    else {
        // 2- this is a Details item
        poItem = poItem->parent();
        if(poItem == NULL) return;
        poItem->setExpanded(false);
    }

}

void GexYMEventLogViewer::addNewEntry(QStringList &oMainEventProperty,  QStringList &oLinkedEventList, int iInsertionIdx){

    QTreeWidgetItem *poItem = NULL;
    QString strEventId = oMainEventProperty[0].rightJustified(10,'0');
    if(iInsertionIdx == -1){
        poItem = new QTreeWidgetItem;
        poItem->setExpanded(false);
        m_poTreeWidget->addTopLevelItem(poItem);
    }
    else {
        // Check if the event already present into the list
        // We need to re-update the last event because this event can be not complete
        QList<QTreeWidgetItem *> oList = m_poTreeWidget->findItems(strEventId,Qt::MatchExactly,EventIDColumn);
        if(oList.isEmpty()){
            poItem = new QTreeWidgetItem;
            poItem->setExpanded(false);
            m_poTreeWidget->insertTopLevelItem(iInsertionIdx,poItem);
        }
        else
            poItem = oList.takeFirst();
    }
    if(poItem == NULL)
        return;

    QString strStatus;
    QString strIcon ;
    poItem->setText(EventIDColumn, strEventId);
    poItem->setTextAlignment(EventIDColumn,Qt::AlignRight);

    int iStatusColumn = -1;
    if(!oLinkedEventList.isEmpty()){

        strStatus = GexYMEventLogDB::getInstance()->getLinkedEventsStatus(strEventId);
        strIcon = getIconFromStatus(strStatus);

        if(!strIcon.isEmpty()){
            QIcon oIcon;
            oIcon.addFile(strIcon,QSize(13,13), QIcon::Normal, QIcon::Off);
            poItem->setIcon(StatusColumn, oIcon);
        }
        iStatusColumn = m_oFieldsList.indexOf("status");
        if(iStatusColumn>-1){
            poItem->setText(EventIDColumn+iStatusColumn+1, strStatus);
        }
        //add Detail button
        QIcon oIcon;
        oIcon.addFile(":/gex/icons/expand.png", QSize(16,16), QIcon::Normal, QIcon::Off);
        poItem->setIcon(DetailColumn, oIcon);
    }
    else
    {
        iStatusColumn = m_oFieldsList.indexOf("status");
        if(iStatusColumn>-1){
            strStatus = oMainEventProperty[iStatusColumn+1];
            poItem->setText(EventIDColumn+iStatusColumn+1, strStatus);
            strIcon = getIconFromStatus(strStatus);
        }

        if(!strIcon.isEmpty()){
            QIcon oIcon;
            oIcon.addFile(strIcon, QSize(13,13), QIcon::Normal, QIcon::Off);
            poItem->setIcon(StatusColumn, oIcon);
        }

        if((m_oFieldsList.indexOf("summary")>-1)
                && !oMainEventProperty[m_oFieldsList.indexOf("summary")+1].isEmpty())
        {
            //add Detail button
            QIcon oIcon;
            oIcon.addFile(":/gex/icons/expand.png", QSize(16,16), QIcon::Normal, QIcon::Off);
            poItem->setIcon(DetailColumn, oIcon);
        }
    }

    QString strToolTip;
    for(int iIdx=1; iIdx<oMainEventProperty.count(); iIdx++){
        if(iStatusColumn>-1){
            if(iIdx == iStatusColumn+1){
                continue;
            }
        }
        if((strStatus != "PASS") && (m_oFieldsList[iIdx-1] == "speed"))
            poItem->setText(EventIDColumn+iIdx,"");
        else if(oMainEventProperty[iIdx].isEmpty())
            poItem->setText(EventIDColumn+iIdx,"");
        else
            poItem->setText(EventIDColumn+iIdx,getFriendlyName(iIdx-1, oMainEventProperty[iIdx]).replace("\n","|"));

        // Align double to right
        if((m_oFieldsList[iIdx-1] == "size") || (m_oFieldsList[iIdx-1] == "duration") || (m_oFieldsList[iIdx-1] == "speed"))
            poItem->setTextAlignment(EventIDColumn+iIdx,Qt::AlignRight);

        // Add tool tip
        if(m_oFieldsList[iIdx-1] == "creation_time")
        {
            strToolTip = QString("StartTime=%1").arg(getFriendlyName(iIdx-1, oMainEventProperty[iIdx]));
            if(m_oFieldsList.indexOf("duration")>-1){
                QDateTime oDateTime = QDateTime::fromString(oMainEventProperty[iIdx],"yyyy-MM-ddThh:mm:ss");
                oDateTime = oDateTime.addSecs(oMainEventProperty[m_oFieldsList.indexOf("duration")+1].toInt());
                strToolTip+= QString("|EndTime=%1").arg(oDateTime.toString("yyyy-MM-ddThh:mm:ss"));
            }
        }
        else if(m_oFieldsList[iIdx-1].endsWith("_id"))
            strToolTip = QString("Id=%1|Name=%2")
                    .arg(oMainEventProperty[iIdx])
                    .arg(getFriendlyName(iIdx-1, oMainEventProperty[iIdx]));
        else
            strToolTip = QString("%1|%2")
                    .arg(m_poGexYMEventLogSettings->getFriendlyName(m_oFieldsList[iIdx-1]))
                    .arg(getFriendlyName(iIdx-1, oMainEventProperty[iIdx]));

        if(oMainEventProperty[iIdx].isEmpty())
            strToolTip = "";

        poItem->setToolTip(EventIDColumn+iIdx,strToolTip.replace("|","\n"));
    }
}

QString GexYMEventLogViewer::getFriendlyName(int iIdx, const QString &strProp){

    GexYMEventBasicLogFilter * poFilter = m_poGexYMEventLogSettings->getFilter(m_oFieldsList[iIdx]);
    if(poFilter){
        QStringList oStrList;
        if(poFilter->getXMLName()=="size"){
            double Size = strProp.toDouble();//in Byte
            if(Size == 0) return "";

            if(Size < 1024)
                return QString("%1B").arg(QString::number((int)Size));

            Size = Size/1024;
            if(Size < 1024)
                return QString("%1KB").arg(QString::number(Size,'f',2));

            Size = Size/1024;
            if(Size < 1024)
                return QString("%1MB").arg(QString::number(Size,'f',2));

            Size = Size/1024;
            return QString("%1GB").arg(QString::number(Size,'f',2));

        }else if(poFilter->getXMLName()=="duration"){
            double Duration = strProp.toDouble();//in Second
            //if(Duration == 0.0) return "";

            QTime oTime = QTime(0,0,0).addSecs((int)Duration);;
            QString Time = oTime.toString("h:m:s");
            int iSecond = Time.section(":",2,2).toInt();
            int iMinute = Time.section(":",1,1).toInt();
            int iHour = Time.section(":",0,0).toInt();

            Time = "";
            if(iHour > 0)
                Time += QString("%1h ").arg(iHour);
            if(iMinute > 0)
                Time += QString("%1m ").arg(iMinute);
            if((iSecond > 0) || Time.isEmpty())
                Time += QString("%1s ").arg(iSecond);

            return Time.trimmed();
        }else if(poFilter->getXMLName()=="speed"){
            double Speed = strProp.toDouble();//in Byte/s

            if(Speed == 0) return "";

            return QString("%1MB/s").arg(QString::number(Speed/1024/1024,'f',4));

        }else if(!poFilter->getFetchFrom().isEmpty()){
            oStrList = poFilter->fetchFromDBTable(poFilter->getXMLName(), poFilter->getFetchFrom(), poFilter->getFriendlyName());
        }else if(poFilter->getFetchFromEventTable()){
            oStrList = poFilter->fetchFromEventTable(poFilter->getXMLName());
        }
        if(oStrList.isEmpty())
            return strProp;

        foreach(const QString &strItem, oStrList){
            // Check if starts with: 0 in 290sep but not = to 0sep
            if(strItem.startsWith(strProp+GexYMEventLogDB::m_strSeparator))
                return strItem.section(GexYMEventLogDB::m_strSeparator,1,1);
        }
    }

    return strProp;

}

void GexYMEventLogViewer::enableGui(bool bEnable){
    //m_poTreeWidget->setEnabled(bEnable);
    m_poSearch->setEnabled(bEnable);
    m_poUseRegExpression->setEnabled(bEnable);
    m_poFind->setEnabled(bEnable);
    m_poFindClear->setEnabled(bEnable);
    m_poFilterExport->setEnabled(bEnable);
    enableRefreshButton(bEnable);
}

void GexYMEventLogViewer::autorefresh(){
    //qDebug(QString("m_bRefreshingOnGoing : %1").arg(m_bRefreshingOnGoing ? "True" : "False"));
    if(m_bRefreshingOnGoing)
        return;
    m_bRefreshingOnGoing = true;
    bool bDeleteFilter = false;
    if(m_poSelection){
        m_poFilter = m_poSelection->buildFilter();
        bDeleteFilter = true;
    }

    if(!m_poFilter){
        m_bRefreshingOnGoing = false;
        return ;
    }

    m_poTreeWidget->setSortingEnabled(false);
    enableGui(false);

    m_poDBProgressBar->show();
    m_poDBProgressBar->reset();
    QTimer oTimer;
    QObject::connect(&oTimer, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    oTimer.setInterval(1000);
    oTimer.start();

    QStringList oLinkedEvents;
    QString strQuery = m_poFilter->buildWhereClause();
    if(getAutorefresh()){
        if(m_poTreeWidget->topLevelItemCount() && !m_strMaxEventIdFetched.isEmpty()){
            if(strQuery.isEmpty())
                strQuery = QString("(event_id >= %1)").arg(m_strMaxEventIdFetched);
            else
                strQuery = QString("(event_id >= %1) AND %2 ").arg(m_strMaxEventIdFetched).arg(strQuery);
        } else {
            m_poTreeWidget->clear();
        }
    }else
        m_poTreeWidget->clear();

    //qDebug(QString("strQuery %1").arg(strQuery));
    QStringList oMainEventList = GexYMEventLogDB::getInstance()->getMainEventsList(m_oFieldsList, strQuery, oLinkedEvents);
    oTimer.stop();
    m_poDBProgressBar->setRange(0, oMainEventList.count());
    QString strMaxEventIdFetched;
    for(int iIdx=0; iIdx<oMainEventList.count(); ++iIdx){
        QString strMainEvent = oMainEventList[iIdx];
        QString strLinkedEvents = oLinkedEvents[iIdx];

        // Active the progress bar here
        updateProgressBar(iIdx);

        QStringList oMainEventProperty = strMainEvent.split(GexYMEventLogDB::m_strSeparator);
        QStringList oLinkedEventList = strLinkedEvents.split(GexYMEventLogDB::m_strSeparator,QString::SkipEmptyParts);
        if(m_strMaxEventIdFetched.isEmpty())
            addNewEntry(oMainEventProperty, oLinkedEventList);
        else
            addNewEntry(oMainEventProperty, oLinkedEventList, iIdx);
        if(iIdx==0)
            strMaxEventIdFetched = oMainEventProperty[0];
        // Adjust the size of the column just for the first lines
        // Before to update all the tree then the user directly see the good size
        if((iIdx==0)||(iIdx==20)||((iIdx+1==oMainEventList.count())&&(iIdx<20)))
        {
            for(int Index=0; Index<m_poTreeWidget->columnCount(); Index++)
            {
                m_poTreeWidget->resizeColumnToContents(Index);
                if(m_poTreeWidget->columnWidth(Index) > 175)
                    m_poTreeWidget->setColumnWidth(Index,175);
            }
        }
    }

    if(getAutorefresh()){
        if(!strMaxEventIdFetched.isEmpty())
            m_strMaxEventIdFetched = strMaxEventIdFetched;
    }

    if(!m_poTreeWidget->topLevelItemCount())
        m_strMaxEventIdFetched.clear();

    m_poTreeWidget->setSortingEnabled(true);
    updateProgressBar(oMainEventList.count());
    m_poDBProgressBar->hide();

    if(!getAutorefresh())
        enableGui(true);

    if(bDeleteFilter){
        delete m_poFilter;
        m_poFilter = 0;
    }
    m_bRefreshingOnGoing = false;
}

QString GexYMEventLogViewer::getIconFromStatus(const QString &strEventStatus){

    if(strEventStatus == "INFO")
        return ":/gex/icons/info.png";
    else if((strEventStatus == "LOGIN") || (strEventStatus == "LOGOUT"))
        return ":/gex/icons/info.png";
    else if((strEventStatus == "LAUNCH") || (strEventStatus == "CLOSE"))
        return ":/gex/icons/info.png";
    else if((strEventStatus == "RUNNING") || (strEventStatus == "STOP"))
        return ":/gex/icons/info.png";
    else if(strEventStatus == "START")
        return "";
    else if(strEventStatus == "FAIL" || strEventStatus == "UNEXPECTED" )
        return ":/gex/icons/error.png";
    else if(strEventStatus == "DELAY" )
        return ":/gex/icons/warning.png";
    else if(strEventStatus == "WARNING")
        return ":/gex/icons/enable_warning.png";

    return ":/gex/icons/enable.png";
}

void GexYMEventLogSelection::initAttributes(){
    m_poFilterSelection = 0;
    m_poDynamic = 0;
    m_poPredinedFilter = 0;
    m_poGexYMEventLogSettings = 0;

    QVBoxLayout *poBoxLayout = new QVBoxLayout(this);
    //poBoxLayout->setSpacing(0);
    poBoxLayout->setContentsMargins(0, 0, 0, 0);

}
GexYMEventExpressionLogFilter *GexYMEventLogSelection::buildFilter(){
    if(m_poPredinedFilter){
        GexYMEventExpressionLogFilter *poExpressionFilter= new GexYMEventExpressionLogFilter();
        foreach(GexYMEventBasicLogFilterElem *poElem, m_poPredinedFilter->getStaticPart()){
            poExpressionFilter->addElem(poElem->m_strOperator, poElem->m_strBasicFilter, poElem->m_strOperand, poElem->m_strValue);
        }

        QList<GexYMEventLogBasicFilterWidget *> oList = m_poDynamic->findChildren<GexYMEventLogBasicFilterWidget *>();
        foreach(GexYMEventLogBasicFilterWidget *poFilter, oList){
            poFilter->saveElemFilter(poExpressionFilter);
        }
        return poExpressionFilter;
    } else {
        GexYMEventExpressionLogFilter *poFilter = 0;
        int iIdx = m_poFilterSelection->currentIndex();
        if(m_poFilterSelection->itemData(iIdx).toString().contains("new_")){
            int iFilterIdx = m_poFilterSelection->itemData(iIdx).toString().remove("new_").toInt();
            poFilter = m_poGexYMEventLogSettings->getNewUserFilters()[iFilterIdx];
        }else
            poFilter = m_poGexYMEventLogSettings->getPredefinedFilters()[m_poFilterSelection->itemData(iIdx).toInt()];
        if(!poFilter)
            return 0;
        GexYMEventExpressionLogFilter *poExpressionFilter= new GexYMEventExpressionLogFilter();

        foreach(GexYMEventBasicLogFilterElem *poElem, poFilter->getStaticPart()){
            poExpressionFilter->addElem(poElem->m_strOperator, poElem->m_strBasicFilter, poElem->m_strOperand, poElem->m_strValue);
        }
        QList<GexYMEventLogBasicFilterWidget *> oList = m_poDynamic->findChildren<GexYMEventLogBasicFilterWidget *>();
        foreach(GexYMEventLogBasicFilterWidget *poFilter, oList){
            poFilter->saveElemFilter(poExpressionFilter);
        }
        return poExpressionFilter;
    }
}

int GexYMEventLogSelection::m_iGexYMEventLogSelectionCount=0;
GexYMEventLogSelection::GexYMEventLogSelection(GexYMEventExpressionLogFilter *poPredinedFilter, GexYMEventLogSettings *poGexYMEventLogSettings,GexYMEventLogLoader *poGexYMEventLog){
    initAttributes();
    m_iGexYMEventLogSelectionCount++;
    m_iUsage = ePredefinedFilter;
    m_poGexYMEventLog =  poGexYMEventLog;

    buildSelectionPart(poPredinedFilter,poGexYMEventLogSettings);
}

GexYMEventLogSelection::GexYMEventLogSelection(GexYMEventLogSettings *poGexYMEventLogSettings,GexYMEventLogLoader *poGexYMEventLog){
    initAttributes();
    m_iGexYMEventLogSelectionCount++;
    m_iUsage = eEventLogFilter;
    m_poGexYMEventLog =  poGexYMEventLog;

    buildSelectionPart(poGexYMEventLogSettings);
}

GexYMEventLogSelection::~GexYMEventLogSelection(){
    m_iGexYMEventLogSelectionCount--;

}

void GexYMEventLogSelection::buildSelectionPart(GexYMEventExpressionLogFilter *poPredinedFilter, GexYMEventLogSettings *poGexYMEventLogSettings){
    m_poPredinedFilter = poPredinedFilter;
    m_poGexYMEventLogSettings = poGexYMEventLogSettings;

    m_poDynamic = new QWidget(this);//showWidgetBorder(m_poDynamic);
    m_poDynamic->setSizePolicy(QSizePolicy ( QSizePolicy::Minimum,QSizePolicy::Fixed, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame);

    QVBoxLayout *poBoxLayout = new QVBoxLayout(m_poDynamic);
    poBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_poDynamic->hide();

    if(!m_poPredinedFilter->getDynamicPart().isEmpty())
    {
        foreach(GexYMEventBasicLogFilterElem *poElem, m_poPredinedFilter->getDynamicPart()){
            GexYMEventBasicLogFilter *poFilter = m_poGexYMEventLogSettings->getFilter(poElem->m_strBasicFilter);
            GexYMEventLogBasicFilterWidget *poTemp = new GexYMEventLogBasicFilterWidget(m_poDynamic,poFilter);
            poTemp->initValues(poElem);
            poTemp->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
            poTemp->setParent(m_poDynamic);
            poBoxLayout->addWidget(poTemp);
        }
        m_poDynamic->show();
    }

    poBoxLayout= qobject_cast<QVBoxLayout *>(layout());
    poBoxLayout->addWidget(m_poDynamic);
    m_poDynamic->adjustSize();
    m_poDynamic->update();
}

void GexYMEventLogSelection::buildSelectionPart(GexYMEventLogSettings *poGexYMEventLogSettings){
    m_poGexYMEventLogSettings = poGexYMEventLogSettings;
    QMap<int, GexYMEventExpressionLogFilter *> &oPredifinedFilters = m_poGexYMEventLogSettings->getPredefinedFilters();
    QMap<int, GexYMEventExpressionLogFilter *> &oNewUserFilters = m_poGexYMEventLogSettings->getNewUserFilters();

    QWidget *poTemp = new QWidget(this);
    QHBoxLayout *poHorizontalLayout = new QHBoxLayout(poTemp);
    poHorizontalLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *poLabel = new QLabel("Select a filter :", poTemp);
    poHorizontalLayout->addWidget(poLabel);

    m_poFilterSelection = new QComboBox(poTemp);
    m_poFilterSelection->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    foreach (int iFilterIdx, oPredifinedFilters.keys()) {
        m_poFilterSelection->addItem(oPredifinedFilters[iFilterIdx]->getLabel(), iFilterIdx);
    }
    foreach (int iFilterIdx, oNewUserFilters.keys()) {
        m_poFilterSelection->addItem(oNewUserFilters[iFilterIdx]->getLabel(), QString("new_%1").arg(iFilterIdx));
    }
    m_poFilterSelection->addItem("Add New Filter","<new_filter_definition>");
    poHorizontalLayout->addWidget(m_poFilterSelection);

    QPushButton *poEdit = new QPushButton(this);
    poEdit->setText("Edit Filter");
    QIcon oIcon;
    oIcon.addFile(QString::fromUtf8(":/gex/icons/funnel.png"), QSize(), QIcon::Normal, QIcon::Off);
    poEdit->setIcon(oIcon);
    poEdit->setIconSize(QSize(10, 10));
    poEdit->setFlat(true);
    poHorizontalLayout->addWidget(poEdit);

    QSpacerItem *poSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    poHorizontalLayout->addSpacerItem(poSpacer);

    QVBoxLayout *poVBLayout = qobject_cast<QVBoxLayout *>(layout());
    poVBLayout->addWidget(poTemp);

    m_poDynamic = new QWidget(this);
    m_poDynamic->setSizePolicy(QSizePolicy ( QSizePolicy::Minimum,QSizePolicy::Fixed, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    QVBoxLayout * poDynamicVBLayout = new QVBoxLayout(m_poDynamic);
    poDynamicVBLayout->setContentsMargins(0, 0, 0, 0);
    m_poDynamic->hide();

    poVBLayout->addWidget(m_poDynamic);

    QObject::connect(m_poFilterSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshGUI(int)));
    QObject::connect(poEdit, SIGNAL(clicked()), this, SLOT(editFilter()));
    refreshGUI(m_poFilterSelection->currentIndex());

    m_poDynamic->adjustSize();
}

void GexYMEventLogSelection::updateLayout(QWidget *poWidget){
    if(!poWidget->layout())
        return ;
    QLayoutItem *poChild = 0;
    while((poChild = poWidget->layout()->takeAt(0))){
        QWidget *poItem = poChild->widget();
        if(poItem){
            poItem->setParent(0);
            delete poItem;
        }
        delete poChild;
    }
    delete poWidget->layout();

    QVBoxLayout *poBoxLayout = new QVBoxLayout(m_poDynamic);
    //poBoxLayout->setSpacing(0);
    poBoxLayout->setContentsMargins(0, 0, 0, 0);

    poWidget->setLayout(poBoxLayout);
}

void GexYMEventLogSelection::editFilter(){

    int iIdx = m_poFilterSelection->currentIndex();

    if(m_poFilterSelection->itemData(iIdx).toString() == "<new_filter_definition>"){
        updateLayout(m_poDynamic);
        m_poDynamic->setSizePolicy(QSizePolicy ( QSizePolicy::Minimum,QSizePolicy::Fixed, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
        m_poDynamic->hide();
        defineNewFilter(0);
        return ;
    }

    GexYMEventExpressionLogFilter *poFilter = 0;
    bool bCanRemove = false;
    if(m_poFilterSelection->itemData(iIdx).toString().contains("new_")){
        int iFilterIdx = m_poFilterSelection->itemData(iIdx).toString().remove("new_").toInt();
        poFilter = m_poGexYMEventLogSettings->getNewUserFilters()[iFilterIdx];
        bCanRemove = true;
    }else
        poFilter = m_poGexYMEventLogSettings->getPredefinedFilters()[m_poFilterSelection->itemData(iIdx).toInt()];

    if(!poFilter)
        return ;

    GexYMEventFilterDialog oDialog(this, m_poGexYMEventLog);
    oDialog.reloadFilters(poFilter,bCanRemove);

    int iRet = oDialog.exec();

    if(iRet == QDialog::Rejected && oDialog.isRemoved()){
        if(m_poFilterSelection->itemData(iIdx).toString().contains("new_")){
            int iFilterIdx = m_poFilterSelection->itemData(iIdx).toString().remove("new_").toInt();
            m_poGexYMEventLogSettings->getNewUserFilters().remove(iFilterIdx);
        }else
            m_poGexYMEventLogSettings->getPredefinedFilters().remove(m_poFilterSelection->itemData(iIdx).toInt());

        QObject::disconnect(m_poFilterSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshGUI(int)));
        m_poFilterSelection->removeItem(iIdx);
        QObject::connect(m_poFilterSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshGUI(int)));
        m_poFilterSelection->setCurrentIndex(iIdx-1);
        return;
    }

    if(iRet == QDialog::Accepted){
        oDialog.save(poFilter);
        refreshGUI(iIdx);
        return;
    }
}

void GexYMEventLogSelection::refreshGUI(int iIdx){
    if(iIdx < 0 )
        return;
    if(m_poFilterSelection->itemData(iIdx).toString() == "<new_filter_definition>"){
        updateLayout(m_poDynamic);
        m_poDynamic->setSizePolicy(QSizePolicy ( QSizePolicy::Minimum,QSizePolicy::Fixed, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
        m_poDynamic->hide();
        defineNewFilter(0);
        return ;
    }
    GexYMEventExpressionLogFilter *poFilter = 0;
    if(m_poFilterSelection->itemData(iIdx).toString().contains("new_")){
        int iFilterIdx = m_poFilterSelection->itemData(iIdx).toString().remove("new_").toInt();
        poFilter = m_poGexYMEventLogSettings->getNewUserFilters()[iFilterIdx];
    }else
        poFilter = m_poGexYMEventLogSettings->getPredefinedFilters()[m_poFilterSelection->itemData(iIdx).toInt()];

    if(!poFilter)
        return ;

    if(!poFilter->getDynamicPart().isEmpty()){
        //clear dynamic container
        updateLayout(m_poDynamic);
        m_poDynamic->show();
        QVBoxLayout *poTemLayout = qobject_cast<QVBoxLayout *>(m_poDynamic->layout());
        foreach(GexYMEventBasicLogFilterElem *poElem, poFilter->getDynamicPart()){
            GexYMEventBasicLogFilter *poDynamicFilter = m_poGexYMEventLogSettings->getFilter(poElem->m_strBasicFilter);
            GexYMEventLogBasicFilterWidget *poTemp = new GexYMEventLogBasicFilterWidget(m_poDynamic, poDynamicFilter);
            poTemp->initValues(poElem);
            poTemp->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));
            poTemLayout->addWidget(poTemp);
        }
        m_poDynamic->setSizePolicy(QSizePolicy ( QSizePolicy::Minimum,QSizePolicy::Fixed, QSizePolicy::Frame));
    }else
        m_poDynamic->hide();

    m_poDynamic->adjustSize();
    m_poDynamic->update();
}

void GexYMEventLogSelection::defineNewFilter(int /*iIdx*/){
    //define Dialog to build a filter based on GexYMEventLogBasicFilterWidget
    //GexYMEventLogBasicFilterWidget
    GexYMEventFilterDialog oDialog(this, m_poGexYMEventLog);
    QObject::disconnect(m_poFilterSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshGUI(int)));

    int iRet = oDialog.exec();
    int iRefreshIdx = -1;
    if(iRet == QDialog::Accepted){
        GexYMEventExpressionLogFilter *poExpressionFilter = new GexYMEventExpressionLogFilter;
        oDialog.save(poExpressionFilter);
        int iIdx = m_poGexYMEventLogSettings->getNewUserFilters().count();
        m_poGexYMEventLogSettings->getNewUserFilters().insert(iIdx, poExpressionFilter);
        int iInsertionIdx = m_poFilterSelection->count()-1;
        m_poFilterSelection->insertItem(iInsertionIdx, poExpressionFilter->getLabel(), QString("new_%1").arg(iIdx));
        iRefreshIdx = iInsertionIdx;

    }else{
        iRefreshIdx = m_poFilterSelection->currentIndex()-1;
    }

    QObject::connect(m_poFilterSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshGUI(int)));
    m_poFilterSelection->setCurrentIndex(iRefreshIdx);
}

void GexYMEventLogViewer::buildRefreshPart(QHBoxLayout *poHorizontalLayout, QWidget *poWidgetParent){

    m_poFilterRefresh = new QPushButton(poWidgetParent);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/gex/icons/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_poFilterRefresh->setIcon(icon);
    m_poFilterRefresh->setIconSize(QSize(10, 10));
    m_poFilterRefresh->setFlat(true);

    poHorizontalLayout->addWidget(m_poFilterRefresh);

    m_poFilterAutoRefresh = new QCheckBox(poWidgetParent);
    //m_poFilterAutoReferesh->setMaximumSize(QSize(16, 16));
    QFont font;
    font.setPointSize(8);
    m_poFilterAutoRefresh->setFont(font);

    poHorizontalLayout->addWidget(m_poFilterAutoRefresh);

    m_poFilterRefreshTime = new QSpinBox(poWidgetParent);
    m_poFilterRefreshTime->setEnabled(true);
    m_poFilterRefreshTime->setMinimum(1);

    poHorizontalLayout->addWidget(m_poFilterRefreshTime);

    m_poFilterRefreshTimeUnits = new QComboBox(poWidgetParent);
    m_poFilterRefreshTimeUnits->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_poFilterRefreshTimeUnits->setEnabled(true);

    poHorizontalLayout->addWidget(m_poFilterRefreshTimeUnits);

    m_poFilterRefresh->setText("Refresh");
    m_poFilterAutoRefresh->setText("Auto refresh every");
    m_poFilterRefreshTimeUnits->clear();
    m_poFilterRefreshTimeUnits->addItem("Sec",1);
    m_poFilterRefreshTimeUnits->addItem("Min",60);
    m_poFilterRefreshTimeUnits->addItem("Hour",3600);


    //    QSpacerItem *poSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    //    poHorizontalLayout->addSpacerItem(poSpacer);

    //    QVBoxLayout *poGridLayout = qobject_cast<QVBoxLayout *>(layout());
    //    poGridLayout->addWidget(poWidget);//,poGridLayout->rowCount(),0,1,1);
}

void GexYMEventLogViewManager::handleError(QWidget* /*poWidget*/)
{
    GS::Gex::Message::warning(
        "GEX",
        QString("Error when initializing Yield-Man log events\n %1 : %2").
        arg(m_iError).arg(m_strError));
}

int  GexYMEventLogViewManager::m_iGexYMEventLogViewManagerCount=0;
GexYMEventLogViewManager::GexYMEventLogViewManager(QTreeWidgetItem *poItem, QWidget *poContainer){
    m_iGexYMEventLogViewManagerCount++;
    m_strError = "";
    m_iError = noError;
    m_poTopNode = 0;

    m_poDB = 0;
    m_poLoader = 0;

    m_poContainerGrid = 0;
    m_poFiltersStack = 0;
    m_oFiltersMapping.clear();

    m_strError = "";
    m_iError   = noError;
    m_oFields.clear();

    m_poTopNode = poItem;
    m_poContainer = poContainer;
    initLogViewerManager();


}

void GexYMEventLogViewManager::saveViewManager(){
    int iRet = QMessageBox::question(0, "Yield Admin DB event log viewer",
                                     "Do you want to export Yield Admin DB event log viewer settings",
                                     QMessageBox::Yes,QMessageBox::No);
    if(iRet == QMessageBox::Yes){
        if(m_poLoader)
            m_poLoader->saveEventTableConfig();
    }

}

void GexYMEventLogViewManager::reloadViewManager(){
    int iRet = QMessageBox::question(0, "Yield Admin DB event log viewer",
                                     "Do you want to reload Yield Admin DB event log viewer",
                                     QMessageBox::Yes,QMessageBox::No);
    if(iRet == QMessageBox::Yes){
        clear();
        m_strError = "";
        m_iError   = noError;
        initLogViewerManager();
    }
}

void GexYMEventLogViewManager::initLogViewerManager(){

    m_poLoader = new GexYMEventLogLoader();
    if(m_poLoader->getErrorCode() != GexYMEventLogLoader::noError){
        setError(GexYMEventLogViewManager::xmlError, m_poLoader->getError());
        return;
    }

    m_oFields = m_poLoader->getGexYMEventLogSettings().getViewerFields();

    if(!m_poTopNode || !m_poContainer || !m_poContainer->layout() || !qobject_cast<QVBoxLayout*> (m_poContainer->layout())){
        m_iError = guiError;
        m_strError = "Gui can not be initialized";
        return;
    }
    m_poDB = GexYMEventLogDB::getInstance();

    m_poFiltersStack = 0;
    setYMEventLogWidgetContainer (m_poContainer);
    setYMEventLogTopNode (m_poTopNode);

}

void GexYMEventLogViewManager::clear(){
    if(m_poDB)
        GexYMEventLogDB::deleteInstance();
    m_poDB = 0;

    if(m_poLoader)
        delete m_poLoader;

    m_poLoader = 0;
    if(m_poFiltersStack){
        m_poContainerGrid->removeWidget(m_poFiltersStack);
        m_poFiltersStack->setParent(0);
        delete m_poFiltersStack;
    }
    m_oFiltersMapping.clear();
    m_oFields.clear();
}

GexYMEventLogViewManager::~GexYMEventLogViewManager(){

    if(m_poLoader)
        m_poLoader->saveEventTableConfig();
    m_iGexYMEventLogViewManagerCount--;
    clear();

#ifdef QT_DEBUG
    QString strMemSummary;
    strMemSummary+="\n********************************************Start Memory Summary********************************************";
    strMemSummary+=QString("\nGexYMEventLogLoader	Counter : %1").arg(	GexYMEventLogLoader::m_iGexYMEventLogLoaderCount);
    strMemSummary+=QString("\nGexYMEventLogFilter	Counter : %1").arg(	GexYMEventLogFilter::m_iGexYMEventLogFilterCounter	);
    strMemSummary+=QString("\nGexYMEventBasicLogFilterElem	Counter : %1").arg(	GexYMEventBasicLogFilterElem::m_iGexYMEventBasicLogFilterElemCount	);
    strMemSummary+=QString("\nGexYMEventBasicLogFilter	Counter : %1").arg(	GexYMEventBasicLogFilter::m_iGexYMEventLogFilterCounter	);
    strMemSummary+=QString("\nGexYMEventExpressionLogFilter	Counter : %1").arg(	GexYMEventExpressionLogFilter::m_iGexYMEventLogFilterCounter	);
    strMemSummary+=QString("\nGexYMEventLogSettings	Counter : %1").arg(	GexYMEventLogSettings::m_iGexYMEventLogSettingsCount	);
    strMemSummary+=QString("\nGexYMEventLogDB	Counter : %1").arg(	GexYMEventLogDB::m_iGexYMEventLogDBCount	);
    strMemSummary+=QString("\nGexYMEventLogBasicFilterWidget	Counter : %1").arg(	GexYMEventLogBasicFilterWidget::m_iGexYMEventLogBasicFilterWidgetCount	);
    strMemSummary+=QString("\nGexYMEventFilterDialog	Counter : %1").arg(	GexYMEventFilterDialog::m_iGexYMEventFilterDialogCount	);
    strMemSummary+=QString("\nGexYMEventLogSelection	Counter : %1").arg(	GexYMEventLogSelection::m_iGexYMEventLogSelectionCount	);
    strMemSummary+=QString("\nGexYMEventLogViewer	Counter : %1").arg(	GexYMEventLogViewer::m_iGexYMEventLogViewerCount	);
    strMemSummary+=QString("\nGexYMEventLogViewManager	Counter : %1").arg(	GexYMEventLogViewManager::m_iGexYMEventLogViewManagerCount	);
    strMemSummary+=QString("\nGexQueryThread counter : %1").arg(GexQueryThread::m_iGexQueryThreadCount);
    strMemSummary+="\n********************************************End Memory Summary********************************************";

    //qDebug(QString("Allocation memory summary : %1").arg(strMemSummary).toLatin1().constData());
#endif
}


void GexYMEventLogViewManager::setYMEventLogWidgetContainer (QWidget *poContainer){
    m_poContainer = poContainer;
    m_poContainerGrid = qobject_cast<QVBoxLayout*> (m_poContainer->layout());

    m_poFiltersStack = new QStackedWidget(m_poContainer);
    m_poContainerGrid->addWidget(m_poFiltersStack);

}


//Node Decoration
void GexYMEventLogViewManager::setYMEventLogTopNode(QTreeWidgetItem *poItem){
    m_poTopNode = poItem;
    m_poTopNode->setFlags(Qt::ItemIsEnabled);
    m_poTopNode->setIcon(0, QIcon(getYMEventLogTopNodeIcon()));
    m_poTopNode->setText(0,getYMEventLogTopNodeLabel());
    addFiltersSubNodes(m_poTopNode);
}

void GexYMEventLogViewManager::addFiltersSubNodes(QTreeWidgetItem *poYMEventLogTopNode){

    if(poYMEventLogTopNode->childCount()){
        qDeleteAll(poYMEventLogTopNode->takeChildren());
    }

    foreach (int iFilterIdx, m_poLoader->getGexYMEventLogSettings().getPredefinedFilters().keys()) {

        QTreeWidgetItem *poItem = new QTreeWidgetItem(poYMEventLogTopNode);
        poItem->setText(0,m_poLoader->getGexYMEventLogSettings().getPredefinedFilters()[iFilterIdx]->getLabel());
        if(!m_poLoader->getGexYMEventLogSettings().getPredefinedFilters()[iFilterIdx]->getIcon().isEmpty())
            poItem->setIcon(0, QIcon(getYMEventLogTopNodeIcon()));
        poItem->setData(0,Qt::UserRole,iFilterIdx);
    }

    QTreeWidgetItem *poItem = new QTreeWidgetItem(poYMEventLogTopNode);
    poItem->setText(0, "Custom Events Logs");
    poItem->setData(0,Qt::UserRole,-1);


}
int GexYMEventLogViewer::getRefreshTime(){
    return m_poFilterRefreshTime->value() *
            m_poFilterRefreshTimeUnits->itemData(m_poFilterRefreshTimeUnits->currentIndex()).toInt();
}

void GexYMEventLogSelection::connectTo(GexYMEventLogViewer *poViewer){
    m_poViewer = poViewer;
    m_poViewer->setLogSelection(this);
}

void GexYMEventLogViewManager::connectFilterViewer(GexYMEventLogSelection *poSelection,  GexYMEventLogViewer *poViewer){
    poSelection->connectTo(poViewer);

}

void GexYMEventLogViewManager::updateLogViewer(QTreeWidgetItem *poItem){
    if(!poItem)
        return ;

    if(m_oFiltersMapping.contains(poItem->data(0, Qt::UserRole).toInt())){
        m_poFiltersStack->setCurrentWidget(m_oFiltersMapping[poItem->data(0, Qt::UserRole).toInt()]);
        GexYMEventLogViewer *poViewer = m_poFiltersStack->currentWidget()->findChild<GexYMEventLogViewer *>();
        if(poViewer && poViewer->isEmpty())
            poViewer->autorefresh();
        return;
    }

    QWidget *poWidget = new QWidget;
    QVBoxLayout *poVBoxLayout = new QVBoxLayout(poWidget);
    //poVBoxLayout->setSpacing(0);
    poVBoxLayout->setContentsMargins(0, 0, 0, 0);

    if( poItem->data(0, Qt::UserRole).toInt() == -1){
        GexYMEventLogSelection *poSelection = new GexYMEventLogSelection(&m_poLoader->getGexYMEventLogSettings(), m_poLoader);
        poSelection->setParent(poWidget);
        poVBoxLayout->addWidget(poSelection);

        GexYMEventLogViewer *poViewer = new GexYMEventLogViewer(poWidget,
                                                                &m_poLoader->getGexYMEventLogSettings(),
                                                                m_oFields);
        poViewer->setParent(poWidget);
        poVBoxLayout->addWidget(poViewer);

        connectFilterViewer(poSelection, poViewer);
        poViewer->autorefresh();
    }else {
        GexYMEventExpressionLogFilter *poFilter = m_poLoader->getGexYMEventLogSettings().getPredefinedFilters()[poItem->data(0, Qt::UserRole).toInt()];
        if(!poFilter){
            delete poWidget;
            return;
        }
        GexYMEventLogSelection *poSelection = new GexYMEventLogSelection(poFilter, &m_poLoader->getGexYMEventLogSettings(),m_poLoader);
        poSelection->setParent(poWidget);
        poVBoxLayout->addWidget(poSelection);

        GexYMEventLogViewer *poViewer = new GexYMEventLogViewer(poWidget,
                                                                &m_poLoader->getGexYMEventLogSettings(),
                                                                m_oFields,
                                                                poFilter);
        poViewer->setParent(poWidget);
        poVBoxLayout->addWidget(poViewer);

        connectFilterViewer(poSelection, poViewer);
        poViewer->autorefresh();
    }

    m_poFiltersStack->addWidget(poWidget);
    m_oFiltersMapping.insert(poItem->data(0, Qt::UserRole).toInt(), poWidget);
    m_poFiltersStack->setCurrentWidget(poWidget);
}

QTreeWidgetItem *GexYMEventLogViewManager::getYMEventLogTopNode(){
    return m_poTopNode;
}

QString GexYMEventLogViewManager::getYMEventLogTopNodeIcon(){
    if(!m_poLoader->getGexYMEventLogSettings().getLogIcon().isEmpty())
        return m_poLoader->getGexYMEventLogSettings().getLogIcon();
    return QString(":/gex/icons/db_events_log.png");
}

QString GexYMEventLogViewManager::getYMEventLogTopNodeLabel(){
    if(!m_poLoader->getGexYMEventLogSettings().getLogLabel().isEmpty())
        return m_poLoader->getGexYMEventLogSettings().getLogLabel();
    return QString("Events logs");
}

int GexYMEventFilterDialog::m_iGexYMEventFilterDialogCount=0;
GexYMEventFilterDialog::GexYMEventFilterDialog(QWidget *poParent,GexYMEventLogLoader *poGexYMEventLog) : QDialog(poParent,Qt::Dialog){
    m_iGexYMEventFilterDialogCount++;
    setupUi(this);
    m_bRemoveFilter = false;
    setWindowTitle("New Filter");
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    m_poGexYMEventLog = poGexYMEventLog;
    QObject::connect(m_poAddFilter, SIGNAL(clicked()), this, SLOT(addFilter()));
}

GexYMEventFilterDialog::~GexYMEventFilterDialog(){
    m_iGexYMEventFilterDialogCount--;

}
void GexYMEventFilterDialog::save(GexYMEventExpressionLogFilter *poExpressionFilter){

    QList<GexYMEventLogBasicFilterWidget *> oList
            =
            findChildren<GexYMEventLogBasicFilterWidget *>();
    if(oList.isEmpty())
        return;

    poExpressionFilter->clear();
    foreach(GexYMEventLogBasicFilterWidget *poFilter, oList){
        poFilter->saveElemFilter(poExpressionFilter);
    }
    if(!m_poFilterName->text().isEmpty()){
        poExpressionFilter->setLabel(m_poFilterName->text());
        poExpressionFilter->setXMLName(m_poFilterName->text());
    }
}

void GexYMEventFilterDialog::removeFilter(){
    m_bRemoveFilter = true;
    QDialog::reject();
}


void GexYMEventFilterDialog::addFilter(){

    QWidget *poTemp = new GexYMEventLogBasicFilterWidget(m_poFilterDialogFrame, 0,m_poGexYMEventLog);
    poTemp->setSizePolicy(QSizePolicy ( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding, QSizePolicy::Frame));//(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    m_poFiltersSelectionGrid->addWidget(poTemp, m_poFiltersSelectionGrid->rowCount(),0,1,1);
    m_poFilterDialogFrame->adjustSize();
}

void GexYMEventFilterDialog:: reloadFilters(GexYMEventExpressionLogFilter *poFilter, bool bCanRemove){

    if(bCanRemove){
        m_poRemoveFilter = m_poButtonBox->addButton("Remove",QDialogButtonBox::ResetRole);
        QObject::connect(m_poRemoveFilter, SIGNAL(clicked()), this, SLOT(removeFilter()));
    }

    setWindowTitle("Edit Filter");

    m_poFilterName->hide();
    m_poFilterNameLabel->hide();
    m_poAddFilter->hide();
    m_poAddFilterLabel->hide();
    foreach(GexYMEventBasicLogFilterElem *poExpr, poFilter->getStaticPart()){
        GexYMEventLogBasicFilterWidget *poTemp = new GexYMEventLogBasicFilterWidget(m_poFilterDialogFrame,m_poGexYMEventLog->getGexYMEventLogSettings().getFilter(poExpr->m_strBasicFilter));
        poTemp->initValues(poExpr);
        m_poFiltersSelectionGrid->addWidget(poTemp, m_poFiltersSelectionGrid->rowCount(),0);
    }

    foreach(GexYMEventBasicLogFilterElem *poExpr, poFilter->getDynamicPart()){
        GexYMEventLogBasicFilterWidget *poTemp = new GexYMEventLogBasicFilterWidget(m_poFilterDialogFrame,m_poGexYMEventLog->getGexYMEventLogSettings().getFilter(poExpr->m_strBasicFilter));
        poTemp->initValues(poExpr);
        m_poFiltersSelectionGrid->addWidget(poTemp, m_poFiltersSelectionGrid->rowCount(),0);
    }


}

void GexYMEventFilterDialog::accept(){
    if(!m_poFilterName->isHidden() &&m_poFilterName->text().isEmpty())
    {
        GS::Gex::Message::warning("", "You must specify filter name");
        return;
    }

    QList<GexYMEventLogBasicFilterWidget *> oList
            =
            findChildren<GexYMEventLogBasicFilterWidget *>();
    if(oList.isEmpty()){
        GS::Gex::Message::warning("", "You must specify at least one filter");
        return;
    }

    QDialog::accept();
}

int GexQueryThread::m_iGexQueryThreadCount = 0;

GexQueryThread::GexQueryThread(QObject* poObj, const QString &strConnectionName):QThread(poObj)
{
    m_strConnectionName = strConnectionName;
    m_poQuery = new QSqlQuery(QSqlDatabase::database(m_strConnectionName));
    m_iGexQueryThreadCount++;
    m_bStatus = false;
}

GexQueryThread::~GexQueryThread(){
    m_iGexQueryThreadCount--;
    delete m_poQuery;
}

bool GexQueryThread::exec(const QString& strQuery){
    m_strQuery = strQuery;
    //qDebug(QString("m_strQuery : %1").arg(m_strQuery).toLatin1().constData());
    if(m_strQuery.isEmpty() || m_strConnectionName.isEmpty())
        return false;
    start();
    while(!isFinished())
        QCoreApplication::processEvents();

    return m_bStatus;
}
void GexQueryThread::run(){
    if(m_poQuery)
        m_bStatus = m_poQuery->exec(m_strQuery);
}

QString GexQueryThread::getQueryLastError(){
    if(m_poQuery)
        return m_poQuery->lastError().text();
    return "";
}

