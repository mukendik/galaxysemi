// gexdb_plugin_galaxy_admin.cpp: implementation of the administration functions of GexDbPlugin_Galaxy class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------


// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_option.h"
#include "import_constants.h"
#include "consolidation_tree.h"
#include "gqtl_datakeys_content.h"
#include "gqtl_datakeys_engine.h"
#include "test_filter.h"
#include <fstream>

// Standard includes
#include <math.h>

// Qt includes
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QTextEdit>
#include <QRegExp>
#include <QDir>
#include <QApplication>
#include <QProgressBar>
#include <QMessageBox>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gstdl_utils_c.h>
#include <gqtl_log.h>
#include "gexdbthreadquery.h"

QString CSVConditionDumper::m_strTempFileSep = "\\";

void CSVConditionDumper::clear(){

    foreach(const QString &strGroup, m_oTestsGroup)
    {
        if(!strGroup.isEmpty())
            QFile::remove(QString(m_oFile.fileName() + strGroup));
    }

    QFile::remove(QString(m_oFile.fileName() + ".SQL") );

    m_oFile.close();
}

bool CSVConditionDumper::entryToStringList(GexDbPlugin_Query *poObj, QStringList& oTestEntries){

    oTestEntries.clear();

    if (poObj)
    {
        int iIdx = 0;
        QVariant oVal;
        while(iIdx < poObj->record().count())
        {
            oVal = poObj->value(iIdx++);

            if(oVal.isNull() || oVal.toString().isNull() || oVal.toString().isEmpty())
                oTestEntries.append("#");
            else
                oTestEntries.append(oVal.toString());
        }
    }
    return true;
}

int CSVConditionDumper::addConditionHeader(const QString &strCondition){
    int iIdx = m_oConditionHeader.indexOf(strCondition);
    if(iIdx==-1){
        m_oConditionHeader.append(strCondition);
        iIdx = m_oConditionHeader.count()-1;
    }
    return iIdx;
}

int CSVConditionDumper::addTestHeader(const QString &strTest){
    int iIdx = m_oTestHeader.indexOf(strTest);
    if(iIdx==-1){
        m_oTestHeader.append(strTest);
        iIdx = m_oTestHeader.count()-1;
    }
    return iIdx;
}

int CSVConditionDumper::groupCount() const
{
    return m_oTestsGroup.count();
}

QString CSVConditionDumper::addGroup(const QString &strGroup){
    if(!m_oTestsGroup.contains(strGroup)){
        m_oTestsGroup.insert(strGroup, QString("group_%1").arg(m_oTestsGroup.count()));
    }
    return m_oTestsGroup[strGroup];
}

bool CSVConditionDumper::dumpTempGroupFile (const QStringList &oValuesOrder){

    //m_poProgress: 45 => 100
    QStringList  oTestList ;
    QStringList oTestName,oNumRow , oUnitsRow, oLLRow, oHLRow;
    QList <QStringList> oTestResult;
    QList <QStringList> oTestRun;
    QList<QStringList> oTestPartId;

    QMap< QString, QPair<int,int> > oGroupsIdx;
    QMap< QString, int> oGroupsRunCountIdx;
    QStringList oGroupProcessed ;
    QStringList oGroupToDump;

    for(int iValues = 0; iValues < oValuesOrder.count(); iValues++){

        if(m_poProgress){
            m_poProgress->setValue((int)(55 + 0.5* ((45.0/oValuesOrder.count()) * (100.0 * (double)iValues/(double)oValuesOrder.count()))));
            if(cancel())
                return false;
        }
        QCoreApplication::processEvents();

        QString strCurrentValues = oValuesOrder[iValues];
        strCurrentValues.replace("|","*");

        oGroupProcessed.clear();
        foreach(const QString &strGroupValue, m_oTestsGroup.keys())
        {
            if(strGroupValue.endsWith(strCurrentValues))
            {
                oGroupProcessed.append(m_oTestsGroup[strGroupValue]);
                oGroupToDump.append( m_oTestsGroup[strGroupValue] + "/" + QString(strCurrentValues).replace("*",","));
            }
        }
        if(oGroupProcessed.isEmpty())
            continue;
        for(int iGroup=0; iGroup <oGroupProcessed.count(); iGroup++){
            QString strCurrentGroup = oGroupProcessed [iGroup];
            QFile oFile(m_oFile.fileName() + strCurrentGroup );
            if(!oFile.open(QIODevice::ReadOnly)){
                return false;
            }
            QTextStream oStream;
            oStream.setDevice(&oFile);

            int iLast = oTestList.count();
            while (!oStream.atEnd()){
                oTestList.append(oStream.readLine());
            }
            oFile.close();

            //Dump tnum, units,ll,hl
            int iMaxCount = 0;
            for(int iIdx=iLast; iIdx < oTestList.count(); iIdx++){
                QStringList oEntries = oTestList[iIdx].split(m_strTempFileSep);

                oTestPartId.append(oEntries[0].split(","));
                if(!oTestPartId.isEmpty() && !oTestPartId.last().isEmpty())
                    oTestPartId.last().removeFirst();

                oTestName.append(oEntries[3]);
                oNumRow.append(oEntries[4]);
                if(oEntries[1] != "F"){
                    oUnitsRow.append(oEntries[5]);
                    oLLRow.append(oEntries[6]);
                    oHLRow.append(oEntries[7]);
                    oTestRun.append(oEntries[8].split(","));

                    if(!oTestRun.isEmpty() && !oTestRun.last().isEmpty())
                        oTestRun.last().removeFirst();

                    oTestResult.append(oEntries[9].split(","));
                    if(!oTestResult.isEmpty() && !oTestResult.last().isEmpty())
                        oTestResult.last().removeFirst();
                    if(!oTestResult.isEmpty() && !oTestResult.last().isEmpty())
                        iMaxCount = qMax(iMaxCount, oTestResult.last().count());
                }else {
                    oUnitsRow.append("#");
                    oLLRow.append("#");
                    oHLRow.append("#");

                    oTestRun.append(oEntries[5].split(","));
                    if(!oTestRun.isEmpty() && !oTestRun.last().isEmpty())
                        oTestRun.last().removeFirst();

                    oTestResult.append(oEntries[6].split(","));
                    if(!oTestResult.isEmpty() && !oTestResult.last().isEmpty())
                        oTestResult.last().removeFirst();

                    if(!oTestResult.isEmpty() && !oTestResult.last().isEmpty())
                        iMaxCount = qMax(iMaxCount, oTestResult.last().count());
                }
            }

            oGroupsIdx.insert(strCurrentGroup, QPair<int, int>(iLast,oTestList.count()) );
            oGroupsRunCountIdx.insert(strCurrentGroup, iMaxCount );
        }
    }

    m_oStream << endl;

    // Fix inconsistant TNum/TName
    // Remove the TestNum: from the TestName
    QStringList lCleanTestNameList;
    foreach(QString lTestName, m_oTestHeader)
    {
        lCleanTestNameList += lTestName.section(":",1);
    }

    m_oStream << m_oConditionHeader.join(",") << ",run_id," << lCleanTestNameList.join(",")<< endl;
    int iConditionHeaderCount = m_oConditionHeader.count();
    int iAdditionalHeaderCount = 1;

    QStringList oNumAddRow, oUnitAddRow, oLLAddRow, oHLAddRow;
    for(int iTestIdx=0; iTestIdx < oTestList.count(); iTestIdx++){
        QCoreApplication::processEvents();
        int iTestHeaderIdx = m_oTestHeader.indexOf(oTestName[iTestIdx]);
        if(iTestHeaderIdx == -1)
            continue;
        if(oNumAddRow.count() > iTestHeaderIdx)
            continue; //already proccessed;
        oNumAddRow.append(oNumRow[iTestIdx]);
        oUnitAddRow.append(oUnitsRow[iTestIdx]);
        oLLAddRow.append(oLLRow[iTestIdx]);
        oHLAddRow.append(oHLRow[iTestIdx]);
    }


    m_oStream << QString().fill(',',iConditionHeaderCount+iAdditionalHeaderCount) <<oNumAddRow.join(",") << endl;
    m_oStream << QString().fill(',',iConditionHeaderCount+iAdditionalHeaderCount) <<oUnitAddRow.join(",") << endl;
    m_oStream << QString().fill(',',iConditionHeaderCount+iAdditionalHeaderCount) <<oLLAddRow.join(",")<< endl;
    m_oStream << QString().fill(',',iConditionHeaderCount+iAdditionalHeaderCount) <<oHLAddRow.join(",")<< endl;


    for(int iGroup = 0; iGroup < oGroupToDump.count(); iGroup++){
        if(m_poProgress){
            m_poProgress->setValue((int)(75.5 + 0.5* ((45.0/oGroupToDump.count()) * (100.0 * (double)iGroup/(double)oGroupToDump.count()))));
            if(cancel())
                return false;
        }
        QString strCurrentGroup = oGroupToDump[iGroup].section("/",0,0);
        QString strConditionsValues = oGroupToDump[iGroup].section("/",1);
        int iStart = oGroupsIdx[strCurrentGroup].first;
        int iEnd = oGroupsIdx[strCurrentGroup].second;
        int iRunMaxCount = oGroupsRunCountIdx[strCurrentGroup];

        for(int iRunIdx=0; iRunIdx < iRunMaxCount; iRunIdx++){
            QCoreApplication::processEvents();
            QStringList oCurrentRow = m_oTestHeader;
            qFill(oCurrentRow, "#");
            QString strCurrentRowRunIdx = "#";
            QString strCurrentPartIdx = "#";

            for(int iTestIdx = iStart; iTestIdx<iEnd ; iTestIdx++){
                int iTestIdxInCSV = m_oTestHeader.indexOf(oTestName[iTestIdx]);
                if(iTestIdxInCSV == -1)
                    continue;

                QStringList oResults = oTestResult[iTestIdx];
                if(iRunIdx < oResults.count()){
                    oCurrentRow[iTestIdxInCSV] = oResults[iRunIdx];
                }
                if(iRunIdx < oTestRun[iTestIdx].count() && strCurrentRowRunIdx == "#"){
                    strCurrentRowRunIdx = (oTestRun[iTestIdx])[iRunIdx];
                }

                if(iRunIdx < oTestPartId[iTestIdx].count() && strCurrentPartIdx == "#"){
                    strCurrentPartIdx = (oTestPartId[iTestIdx])[iRunIdx];
                }

            }
            m_oStream << strCurrentPartIdx << "," << strConditionsValues <<","<< strCurrentRowRunIdx <<","<< oCurrentRow.join(",") << endl;
        }
    }

    return true;
}

bool CSVConditionDumper::dumpSQL(const QString &lQuery){
    QFile oFile(m_oFile.fileName() + ".SQL" );
    if(!oFile.open(QIODevice::Append)){
        return false;
    }
    QTextStream oStream;
    oStream.setDevice(&oFile);
    oStream << lQuery << endl;
    oFile.close();
    return true;
}

bool CSVConditionDumper::createTempGroupFile(const QString &strGroup, const QMap<QString, QString> &oData){
    QFile oFile(m_oFile.fileName() + strGroup );
    if(!oFile.open(QIODevice::Append)){
        return false;
    }
    QTextStream oStream;
    oStream.setDevice(&oFile);

    QStringList oTestEntries;
    oTestEntries.append(oData["part_id"]);
    oTestEntries.append(oData["type"]);
    oTestEntries.append(oData["idx"]);
    oTestEntries.append(oData["name"]);
    oTestEntries.append(oData["tnum"]);
    if(oData["type"] != "F"){
        oTestEntries.append(oData["units"]);
        oTestEntries.append(oData["ll"]);
        oTestEntries.append(oData["hl"]);
    }
    oTestEntries.append(oData["run_id"]);
    oTestEntries.append(oData["value"]);

    oStream << oTestEntries.join(m_strTempFileSep) << endl;
    oFile.close();
    return true;
}

bool CSVConditionDumper::create(const QString &strCSVFileName){
    m_oFile.setFileName(strCSVFileName);
    if(!m_oFile.open(QIODevice::WriteOnly)){
        return false;
    }
    m_oStream.setDevice(&m_oFile);

    // CSV Header comment.
    m_oStream << "# Semiconductor Yield Analysis is easy with Quantix! " << endl;
    m_oStream << "# Check latest news: www.mentor.com " << endl;

    m_oStream << "--- Csv version:" << endl;
    m_oStream << "Major," << /*mMajorVersion*/ 0 << endl;
    m_oStream << "Minor," << /*mMinorVersion*/ 0 << endl;

    m_oStream << "--- Global Info:" << endl;
    m_oStream << "Date," << QDateTime::currentDateTime().toString("yyyy_MM_dd hh:mm:ss")  << endl;
    return true;

}

