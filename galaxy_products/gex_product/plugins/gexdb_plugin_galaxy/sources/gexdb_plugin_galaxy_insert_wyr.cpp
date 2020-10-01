//////////////////////////////////////////////////////////////////////
// gexdb_plugin_galaxy_insert_wyr.cpp: Insert a Weekly Yield Report file in GexDb WYR table
//////////////////////////////////////////////////////////////////////

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "import_constants.h"

// Qt Include
#include <QSqlError>
#include <QApplication>
#include <QProgressBar>

// File format:
//Date In,Date Out,Part Name,Test Setup,Tester ID,Test Program,Date Code,WM Lot Number,Site Lot,Qty IN,Qty OUT,Final Yield(%),Bin01,Bin07,Bin08,MR,SR,No. Insertions,Comments
//07/06/2007,09/06/2007,WM8711LGEFL/R(WM8721LV.U05),WTS000320G, WMS_CAT_3,wm8711lv_4_16,75AS9RD,18652,SL54840,31797,30650,96.39,30650,1107,39,1,12,32512,
//08/06/2007,13/06/2007,WM8711LGEFL/R(WM8721LV.U05),WTS000320G, WMS_CAT_3,wm8711lv_4_16,75AT9RD,18652A,SL54847,25341,24705,97.49,24705,617,16,3,3,25733,
//06/06/2007,11/06/2007,WM8711LGEFL/R(WM8721LV.U05),WTS000320G, WMS_CAT_3,WM8711lv_4_16,75AX9RD,18776A,SL54884,19711,18844,95.6,18844,815,41,11,25,20164,


//////////////////////////////////////////////////////////////////////
// Insert specified Data File into the current GALAXY database
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::InsertWyrDataFile(const QString& strDataFileName,
                                           const QString& strSiteName,
                                           const QString& strTestingStage,
                                           unsigned int /*uiWeekNb*/,
                                           unsigned int /*uiYear*/,
                                           bool* pbDelayInsertion)
{

    // Debug message
    QString strMessage = "---- GexDbPlugin_Galaxy::InsertWyrDataFile: ";
    strMessage += strDataFileName;
    WriteDebugMessageFile(strMessage);

    InitStdfFields();

    // Set testing stage
    if(strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        SetTestingStage(eElectTest);
    else if(strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
        SetTestingStage(eWaferTest);
    else if(strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
        SetTestingStage(eFinalTest);
    else
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR1(GexDbPlugin_Base, eDB_InvalidTestingStage, NULL, strTestingStage.toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }

    m_strSiteName = strSiteName;
    //m_uiWeekNb = uiWeekNb;
    //m_uiYear = uiYear;
    m_pbDelayInsertion = pbDelayInsertion;
    mProgress = -1;

    // First check if GEXDB is up-to-date
    QString         strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int    uiGexDbBuild, uiBuildSupportedByPlugin;
    bool            bDbUpToDate;
    if(!IsDbUpToDateForInsertion(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        *m_pbDelayInsertion = true;
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }
    if(!bDbUpToDate)
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        ErrorMessage(GGET_LASTERRORMSG(GexDbPlugin_Base, this));
        return false;
    }

    // Check DB connection
    SetAdminLogin(true);
    if(!ConnectToCorporateDb())
    {
        *m_pbDelayInsertion = true;
        return false;
    }

    ResetProgress(false);

    // Insert STDF file
    return InsertWyrFile(strDataFileName);
}

//////////////////////////////////////////////////////////////////////
// Check WYR tables
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::CheckWyrTables()
{
    QString             strQuery;
    GexDbPlugin_Query   clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Add WaferID column Wafer-Sort table if column not already exists
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        // Verify if exist before create it
        strQuery = "SELECT WAFER_ID FROM " + NormalizeTableName("WT_WYR",false);
        if(!clGexDbQuery.Execute(strQuery))
        {
            // not exist
            strQuery = "ALTER TABLE " + NormalizeTableName("WT_WYR",false);
            strQuery +="  ADD (";
            strQuery +="	WAFER_ID			VARCHAR2(255)";
            strQuery +="      )";
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }
    }
    if(m_pclDatabaseConnector->IsMySqlDB())
    {
        // Verify if exist before create it
        strQuery = "SELECT WAFER_ID FROM " + NormalizeTableName("WT_WYR",false);
        if(!clGexDbQuery.Execute(strQuery))
        {
            // not exist
            strQuery = "ALTER TABLE " + NormalizeTableName("WT_WYR",false);
            strQuery +="  ADD (";
            strQuery +="	WAFER_ID 			varchar(255)			DEFAULT NULL";
            strQuery +="      )";
            if(!clGexDbQuery.Execute(strQuery))
            {
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(1024).toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                return false;
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the WYR file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::InsertWyrFile(const QString & strDataFileName)
{
    QString strString;
    QString strMessage;

    // Make sure Wafer_ID added to WT_WYR, ET_WYR
    if(!CheckWyrTables())
        return false;

    // Open file
    QFile f( strDataFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        GSET_ERROR0(GexDbPlugin_Base, eWyr_FileOpen, NULL);
        ErrorMessage(f.errorString());
        // Insertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    int iFileSize = f.size() + 1;


    // Assign file I/O stream
    QTextStream hWyrFile(&f);


    int                     nMaxColumns;
    int                     nLine;
    int                     nColId, nColNb;
    QString                 strQuery;
    QString                 strDataType;
    QString                 strColumnName;
    QStringList             lstField;
    QMap<int,int>           mapColNb_ColId;
    QMap<int,QString>       mapColNb_DataType;
    QString                 strValue, strDateOut, strDateIn, strInsertions, strYield;
    QString                 strDateInFormat, strDateOutFormat;
    QSqlQuery               clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));

    // Debug message
    strMessage = "---- Retreive Mapping information: ";
    strMessage += m_strSiteName;
    WriteDebugMessageFile(strMessage);

    // Retreive Mapping information
    // Retreive mapping from XX_WYR_FORMAT
    strQuery = "SELECT COLUMN_ID, COLUMN_NB, DATA_TYPE, COLUMN_NAME FROM ";
    strQuery += NormalizeTableName("_WYR_FORMAT");
    strQuery += " WHERE lower(SITE_NAME)="+TranslateStringToSqlVarChar(m_strSiteName.toLower());
    if(!clQuery.exec(strQuery))
    {
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(), clQuery.lastError().text().left(256).toLatin1().constData());
        f.close();
        return false;
    }

    if(!clQuery.first())
    {
        // No specific information for this SiteName
        // Take the default
        strQuery = "SELECT COLUMN_ID, COLUMN_NB, DATA_TYPE, COLUMN_NAME FROM ";
        strQuery += NormalizeTableName("_WYR_FORMAT");
        strQuery += " WHERE lower(SITE_NAME)='default'";
        if(!clQuery.exec(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(), clQuery.lastError().text().left(256).toLatin1().constData());
            f.close();
            return false;
        }
    }

    if(!clQuery.first())
    {
        // No information for this SiteName
        *m_pbDelayInsertion = true;
        GSET_ERROR2(GexDbPlugin_Base, eWyr_MissingSite, NULL, m_strSiteName.toLatin1().constData(), NormalizeTableName("_WYR_FORMAT").toLatin1().constData());
        f.close();
        return false;
    }

    // Check the first line (table header)
    strString = hWyrFile.readLine();
    lstField = strString.split(",");
    nMaxColumns = 0;
    do
    {
        nColId = clQuery.value(0).toInt();
        nColNb = clQuery.value(1).toInt();
        strDataType = clQuery.value(2).toString();
        strColumnName = clQuery.value(3).toString();

        if(nMaxColumns < nColNb)
            nMaxColumns = nColNb;

        if(nColId < 0)
            continue;
        if(strDataType.isEmpty())
            continue;

        // Check if the format is OK
        if((nColNb >= lstField.count())
                || (lstField[nColNb].trimmed().toUpper() != strColumnName.trimmed().toUpper()))
        {
            // No information for this SiteName
            *m_pbDelayInsertion = true;
            QString strError = " [column="+QString::number(nColNb)+"]";
            if(nColNb >= lstField.count())
            {
                strError+= " not exists into the WYR file ";
            }
            else
            {
                strError+= " found '"+lstField[nColNb].trimmed();
                strError+= "' into the WYR file instead of '"+strColumnName+"'";
            }
            strError+= " as defined into the "+NormalizeTableName("_WYR_FORMAT")+" table";
            GSET_ERROR1(GexDbPlugin_Base, eWyr_IncorrectFormat, NULL, strError.toLatin1().constData());
            f.close();
            return false;
        }

        if(strDataType.toUpper() == "USER")
            strDataType = "USER_SPLIT";

        if(strDataType.startsWith("DATE_",Qt::CaseInsensitive))
        {
            // All date in WYR_FORMAT have to be append with a QT::DateFormat
            int iPos;
            iPos = strDataType.indexOf("(");
            if(iPos > 0)
            {
                strValue = strDataType.mid(iPos+1);
                iPos = strValue.indexOf(")");
                if(iPos > 0)
                    strValue = strValue.left(iPos);
                else
                    iPos = 0;
            }
            if(iPos < 0)
            {
                // No information for this SiteName
                *m_pbDelayInsertion = true;
                GSET_ERROR2(GexDbPlugin_Base, eWyr_MissingFormat, NULL, strDataType.toLatin1().constData(), NormalizeTableName("_WYR_FORMAT").toLatin1().constData());
                f.close();
                return false;
            }
            if(strDataType.startsWith("DATE_IN"))
            {
                strDataType = "DATE_IN";
                strDateInFormat = strValue;
            }
            else
            {
                strDataType = "DATE_OUT";
                strDateOutFormat = strValue;
            }
        }

        strDataType = strDataType.toUpper();


        mapColNb_ColId[nColNb] = nColId;
        mapColNb_DataType[nColNb] = strDataType;
    }
    while(clQuery.next());

    // Check if the format is OK
    if((nMaxColumns+1) != lstField.count())
    {
        *m_pbDelayInsertion = true;
        QString strError = " [column mismatch]";
        strError+= " found '"+QString::number(lstField.count());
        strError+= "' columns into the WYR file instead of '"+QString::number(nMaxColumns+1)+"'";
        strError+= " columns as defined into the "+NormalizeTableName("_WYR_FORMAT")+" table";
        GSET_ERROR1(GexDbPlugin_Base, eWyr_IncorrectFormat, NULL, strError.toLatin1().constData());
        f.close();
        return false;
    }

    // Debug message
    strMessage = "---- Read WYR Data: ";
    WriteDebugMessageFile(strMessage);

    // Debug message
    strMessage = "LotId - ";
    strMessage+= "SubConLotId - ";
    if(m_eTestingStage == eWaferTest)
        strMessage+= "WaferId - ";
    strMessage+= "DateOut - ";
    strMessage+= "DateInt - ";
    strMessage+= "Insertions - ";
    strMessage+= "Yield";
    WriteDebugMessageFile(strMessage);
    nLine = 0;
    while(!hWyrFile.atEnd())
    {
        ReadNextWyrLine(hWyrFile, strString);
        nLine++;

        QCoreApplication::processEvents();

        int     nProgress;
        long    lPos;

        lPos = f.pos();
        nProgress = (int)((float)(lPos*100.0)/(float)iFileSize);

        if(nProgress>100)
            nProgress=100;
        if(nProgress > 0 && (mProgress != nProgress))
        {
            mProgress = nProgress;
            SetProgress(mProgress);
        }

        if(strString.isEmpty())
            continue;

        lstField = strString.split(",");

        // Check if the format is OK
        if((nMaxColumns+1) != lstField.count())
        {
            *m_pbDelayInsertion = true;
            QString strError = " [column mismatch at line "+QString::number(nLine)+"]";
            strError+= " found '"+QString::number(lstField.count());
            strError+= "' columns into the WYR file instead of '"+QString::number(nMaxColumns+1)+"'";
            strError+= " columns as defined into the "+NormalizeTableName("_WYR_FORMAT")+" table";
            GSET_ERROR1(GexDbPlugin_Base, eWyr_IncorrectFormat, NULL, strError.toLatin1().constData());
            f.close();
            return false;
        }


        // Verify if the line already inserted
        // LOT_ID, SUBCON_LOT_ID, INSERTIONS, YIELD
        strDateOut = ExtractFieldValue((char*)"DATE_OUT",lstField,mapColNb_DataType,mapColNb_ColId);
        strDateIn = ExtractFieldValue((char*)"DATE_IN",lstField,mapColNb_DataType,mapColNb_ColId);
        strInsertions = ExtractFieldValue((char*)"INSERTIONS",lstField,mapColNb_DataType,mapColNb_ColId,false,true);
        strYield = ExtractFieldValue((char*)"YIELD",lstField,mapColNb_DataType,mapColNb_ColId,false,true);

        // Debug message
        strMessage = "'"+ExtractFieldValue((char*)"LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId);
        strMessage+= "' - '";
        strMessage+= ExtractFieldValue((char*)"SUBCON_LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId);
        if(m_eTestingStage == eWaferTest)
        {
            strMessage+= "' - '";
            strMessage+= ExtractFieldValue((char*)"WAFER_ID",lstField,mapColNb_DataType,mapColNb_ColId);
        }
        strMessage+= "' - '";
        strMessage+= strDateOut;
        strMessage+= "' - '";
        strMessage+= strDateIn;
        strMessage+= "' - ";
        strMessage+= strInsertions;
        strMessage+= " - ";
        strMessage+= strYield;
        WriteDebugMessageFile(strMessage);

        if(m_pclDatabaseConnector->IsOracleDB())
        {
            QString strValue, strNullValue;
            strQuery =  "SELECT * FROM "+NormalizeTableName("_WYR");
            strValue = ExtractFieldValue((char*)"LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId);
            strNullValue = "";
            if(strValue.isEmpty())
                strNullValue = " OR (LOT_ID IS NULL)";
            strQuery += " WHERE (upper(LOT_ID)="+TranslateStringToSqlVarChar(strValue).toUpper() + strNullValue + ") ";
            strValue = ExtractFieldValue((char*)"SUBCON_LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId);
            strNullValue = "";
            if(strValue.isEmpty())
                strNullValue = " OR (SUBCON_LOT_ID IS NULL)";
            strQuery += " AND (upper(SUBCON_LOT_ID)="+TranslateStringToSqlVarChar(strValue).toUpper() + strNullValue + ")";
            if(m_eTestingStage == eWaferTest)
            {
                strValue = ExtractFieldValue((char*)"WAFER_ID",lstField,mapColNb_DataType,mapColNb_ColId);
                strNullValue = "";
                if(strValue.isEmpty())
                    strNullValue = " OR (WAFER_ID IS NULL)";
                strQuery += " AND (upper(WAFER_ID)="+TranslateStringToSqlVarChar(strValue).toUpper() + strNullValue + ")";
            }
            strQuery += " AND DATE_IN=to_date("+TranslateStringToSqlVarChar(strDateIn)+", "+TranslateStringToSqlVarChar(strDateInFormat)+")";
            strQuery += " AND DATE_OUT=to_date("+TranslateStringToSqlVarChar(strDateOut)+", "+TranslateStringToSqlVarChar(strDateOutFormat)+")";
            strQuery += " AND INSERTIONS="+strInsertions;
            strQuery += " AND YIELD="+strYield;
        }
        else
        {
            strQuery =  "SELECT * FROM "+NormalizeTableName("_WYR");
            strQuery += " WHERE upper(LOT_ID)=upper("+TranslateStringToSqlVarChar(ExtractFieldValue((char*)"LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId)) + ") ";
            strQuery += " AND upper(SUBCON_LOT_ID)=upper("+TranslateStringToSqlVarChar(ExtractFieldValue((char*)"SUBCON_LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId)) + ")";
            if(m_eTestingStage == eWaferTest)
                strQuery += " AND upper(WAFER_ID)=upper("+TranslateStringToSqlVarChar(ExtractFieldValue((char*)"WAFER_ID",lstField,mapColNb_DataType,mapColNb_ColId)) + ")";
            strQuery += " AND DATE_IN=STR_TO_DATE("+TranslateStringToSqlVarChar(strDateIn)+", "+TranslateStringToSqlVarChar(strDateInFormat)+")";
            strQuery += " AND DATE_OUT=STR_TO_DATE("+TranslateStringToSqlVarChar(strDateOut)+", "+TranslateStringToSqlVarChar(strDateOutFormat)+")";
            strQuery += " AND INSERTIONS="+strInsertions;
            strQuery += " AND YIELD="+strYield;
        }

        if(!clQuery.exec(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(), clQuery.lastError().text().left(256).toLatin1().constData());
            f.close();
            return false;
        }

        if(clQuery.first())
            continue;


        // Extract the current WeekNb and the current Year from DATE_OUT
        //
        strQuery = "SELECT ";
        if(m_pclDatabaseConnector->IsOracleDB())
        {
            // STR to Date
            strValue = "to_date("+TranslateStringToSqlVarChar(strDateOut)+", "+TranslateStringToSqlVarChar(strDateOutFormat)+")";

            strQuery += " to_char("+strValue+",'WW') , ";
            strQuery += " to_char("+strValue+",'YY') ";
            strQuery += " FROM dual";

        }
        else
        {
            // STR to Date
            strValue = "STR_TO_DATE("+TranslateStringToSqlVarChar(strDateOut)+", "+TranslateStringToSqlVarChar(strDateOutFormat)+")";

            strQuery += " DATE_FORMAT("+strValue+",'%V'), ";
            strQuery += " DATE_FORMAT("+strValue+",'%y') ";
        }

        if(!clQuery.exec(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(), clQuery.lastError().text().left(256).toLatin1().constData());
            f.close();
            return false;
        }

        if(clQuery.first())
        {
            m_uiWeekNb =	clQuery.value(0).toInt();
            m_uiYear =		clQuery.value(1).toInt();
        }

        strQuery = "INSERT INTO "+NormalizeTableName("_WYR");
        // Add columns definition
        strQuery += " (SITE_NAME,WEEK_NB,YEAR,DATE_IN,DATE_OUT,PRODUCT_NAME,PROGRAM_NAME,TESTER_NAME,LOT_ID,";
        strQuery += "SUBCON_LOT_ID,USER_SPLIT,YIELD,PARTS_RECEIVED,PRETEST_REJECTS,PRETEST_REJECTS_SPLIT,PARTS_TESTED,";
        strQuery += "PARTS_PASS,PARTS_PASS_SPLIT,PARTS_FAIL,PARTS_FAIL_SPLIT,PARTS_RETEST,PARTS_RETEST_SPLIT,";
        strQuery += "INSERTIONS,POSTTEST_REJECTS,POSTTEST_REJECTS_SPLIT,PARTS_SHIPPED";
        if(m_eTestingStage == eWaferTest)
            strQuery += ",WAFER_ID";
        strQuery += ")";

        strQuery += " VALUES(";
        // SITE_NAME
        strQuery += TranslateStringToSqlVarChar(m_strSiteName) + ",";
        // WEEK_NB
        strQuery += QString::number(m_uiWeekNb) + ",";
        // YEAR
        strQuery += QString::number(m_uiYear) + ",";
        // DATE_IN
        if(m_pclDatabaseConnector->IsOracleDB())
            strQuery += "to_date("+TranslateStringToSqlVarChar(strDateIn)+", "+TranslateStringToSqlVarChar(strDateInFormat)+"),";
        else
            strQuery += "STR_TO_DATE("+TranslateStringToSqlVarChar(strDateIn)+", "+TranslateStringToSqlVarChar(strDateInFormat)+"),";
        // DATE_OUT
        if(m_pclDatabaseConnector->IsOracleDB())
            strQuery += "to_date("+TranslateStringToSqlVarChar(strDateOut)+", "+TranslateStringToSqlVarChar(strDateOutFormat)+"),";
        else
            strQuery += "STR_TO_DATE("+TranslateStringToSqlVarChar(strDateOut)+", "+TranslateStringToSqlVarChar(strDateOutFormat)+"),";
        // PRODUCT_NAME
        strValue = ExtractFieldValue((char*)"PRODUCT_NAME",lstField,mapColNb_DataType,mapColNb_ColId);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // PROGRAM_NAME
        strValue = ExtractFieldValue((char*)"PROGRAM_NAME",lstField,mapColNb_DataType,mapColNb_ColId);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // TESTER_NAME
        strValue = ExtractFieldValue((char*)"TESTER_NAME",lstField,mapColNb_DataType,mapColNb_ColId);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // LOT_ID
        strValue = ExtractFieldValue((char*)"LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // SUBCON_LOT_ID
        strValue = ExtractFieldValue((char*)"SUBCON_LOT_ID",lstField,mapColNb_DataType,mapColNb_ColId);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // USER_SPLIT
        strValue = ExtractFieldValue((char*)"USER_SPLIT",lstField,mapColNb_DataType,mapColNb_ColId,true);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // YIELD
        strQuery += strYield + ",";
        // PARTS_RECEIVED
        strValue = ExtractFieldValue((char*)"PARTS_RECEIVED",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
        if(strValue.isEmpty())
        {
            bool bIsNumber;
            QString strResult;
            int nSum;
            strValue = ExtractFieldValue((char*)"PARTS_TESTED",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
            strResult += (strResult.isEmpty()?"":"+") + strValue;
            nSum = strValue.toInt(&bIsNumber);
            strValue = ExtractFieldValue((char*)"PRETEST_REJECTS",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
            strResult += (strResult.isEmpty()?"":"+") + strValue;
            if(bIsNumber)
                nSum += strValue.toInt(&bIsNumber);
            if(bIsNumber)
                strValue = QString::number(nSum);
            else
                strValue = strResult;
        }
        strQuery += strValue + ",";
        // PRETEST_REJECTS
        strValue = ExtractFieldValue((char*)"PRETEST_REJECTS",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
        strQuery += strValue + ",";
        // PRETEST_REJECTS_SPLIT
        strValue = ExtractFieldValue((char*)"PRETEST_REJECTS",lstField,mapColNb_DataType,mapColNb_ColId,true);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // PARTS_TESTED
        strValue = ExtractFieldValue((char*)"PARTS_TESTED",lstField,mapColNb_DataType,mapColNb_ColId,false,true);
        strQuery += strValue + ",";
        // PARTS_PASS
        strValue = ExtractFieldValue((char*)"PARTS_PASS",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
        strQuery += strValue + ",";
        // PARTS_PASS_SPLIT
        strValue = ExtractFieldValue((char*)"PARTS_PASS",lstField,mapColNb_DataType,mapColNb_ColId,true);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // PARTS_FAIL
        strValue = ExtractFieldValue((char*)"PARTS_FAIL",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
        strQuery += strValue + ",";
        // PARTS_FAIL_SPLIT
        strValue = ExtractFieldValue((char*)"PARTS_FAIL",lstField,mapColNb_DataType,mapColNb_ColId,true);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // PARTS_RETEST
        strValue = ExtractFieldValue((char*)"PARTS_RETEST",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
        strQuery += strValue + ",";
        // PARTS_RETEST_SPLIT
        strValue = ExtractFieldValue((char*)"PARTS_RETEST",lstField,mapColNb_DataType,mapColNb_ColId,true);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // INSERTIONS
        strQuery += strInsertions + ",";
        // POSTTEST_REJECTS
        strValue = ExtractFieldValue((char*)"POSTTEST_REJECTS",lstField,mapColNb_DataType,mapColNb_ColId,true,true);
        strQuery += strValue + ",";
        // POSTTEST_REJECTS_SPLIT
        strValue = ExtractFieldValue((char*)"POSTTEST_REJECTS",lstField,mapColNb_DataType,mapColNb_ColId,true);
        strQuery += TranslateStringToSqlVarChar(strValue) + ",";
        // PARTS_SHIPPED
        strValue = ExtractFieldValue((char*)"PARTS_SHIPPED",lstField,mapColNb_DataType,mapColNb_ColId,false,true);
        strQuery += strValue;
        if(m_eTestingStage == eWaferTest)
        {
            // WAFER_ID
            strValue = ExtractFieldValue((char*)"WAFER_ID",lstField,mapColNb_DataType,mapColNb_ColId);
            strQuery += "," + TranslateStringToSqlVarChar(strValue);
        }

        strQuery += ")";
        if(!clQuery.exec(strQuery))
        {
            *m_pbDelayInsertion = true;
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.left(256).toLatin1().constData(), clQuery.lastError().text().left(256).toLatin1().constData());
            f.close();

            clQuery.exec("COMMIT");

            return false;
        }
        WriteDebugMessageFile(strQuery);
    }

    f.close();

    clQuery.exec("COMMIT");
    // Success parsing WYR file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Extract corresponding value for DataType Field from FileStringField
// with mapping Col_Id and Col_DataType
// and bSplitDataType true if merge field
// and bNumericalSum true if numerical field (Int, except for YIELD (float))
//////////////////////////////////////////////////////////////////////
QString GexDbPlugin_Galaxy::ExtractFieldValue(char *szDataType, QStringList& lstField, QMap<int,QString>	&mapColNb_DataType, QMap<int,int> &mapColNb_ColId, bool bSplitDataType, bool bNumericalSum)
{
    int         iIndex;
    bool        bIsNumber;
    bool        bFoundInvalidNumber;
    float       fSum, fValue;
    QString     strResult, strValue;
    QStringList strlColumnDataType;
    QMap<int,QString>::Iterator itMap;

    fSum = 0;
    bFoundInvalidNumber = false;
    for(itMap=mapColNb_DataType.begin(); itMap!=mapColNb_DataType.end(); itMap++)
    {
        iIndex = itMap.key();
        strlColumnDataType = itMap.value().split(",");
        // Check if we have a column in the format with the specified data type
        // AND MAKE SURE THE LINE WE ARE PROCESSING HAS THIS COLUMN
        if((iIndex < lstField.size()) && (strlColumnDataType.contains(QString(szDataType), Qt::CaseInsensitive)))
        {
            strValue = (lstField[iIndex]).simplified();
            if(bSplitDataType)
            {
                if(bNumericalSum)
                {
                    strResult += (strResult.isEmpty()?"":"+") + strValue;
                    fValue = strValue.toFloat(&bIsNumber);
                    if(!bIsNumber) bFoundInvalidNumber = true;
                    fSum += fValue;
                }
                else
                {
                    if(!strValue.isEmpty())
                    {
                        strValue = "C"+QString::number(mapColNb_ColId[iIndex])+"="+strValue;
                        if(!strResult.isEmpty())
                            strResult += ",";
                        strResult += strValue;
                    }
                }
            }
            else
            {
                if(bNumericalSum)
                {
                    strValue = strValue.simplified().remove(" ");
                    if(strValue.endsWith("%"))
                        strValue.chop(1);
                    strResult += (strResult.isEmpty()?"":"+") + strValue;
                    fValue = strValue.toFloat(&bIsNumber);
                    if(!bIsNumber) bFoundInvalidNumber = true;
                    fSum += fValue;
                }
                else
                {
                    strResult = strValue;
                }
                break;
            }
        }
    }

    if(bNumericalSum)
    {
        // Check if found an invalid number then return the original string (already in strResult)
        // This is the SQL query that return the error
        if(bFoundInvalidNumber)
            strResult = "'"+strResult+"'";
        else{
            if(QString(szDataType) == "YIELD")
                strResult = QString::number(fSum);
            else
                strResult = QString::number((int)fSum);
        }
    }
    return strResult;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ReadNextWyrLine(QTextStream& hFile, QString & strLine)
{
    QString strString, strTemp;

    // Empty line first
    strLine = "";

    // Read file, and return first non-empty line, different from ",,,,,,,,,..."
    do
    {
        strLine = hFile.readLine().trimmed();
        strString = strLine;
        strString = strString.remove(",").simplified();
    }
    while(!strLine.isNull() && strString.isEmpty());

    return true;

}
