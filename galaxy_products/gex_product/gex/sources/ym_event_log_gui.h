#ifndef YM_EVENT_LOG_GUI_H
#define YM_EVENT_LOG_GUI_H

#include <QFile>
#include <QMap>
#include <QTextStream>
#include <QDomDocument>
#include <QGridLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <QCheckBox>
#include <QSpinBox>
#include <QSqlQuery>
#include <QProgressBar>
#include <QThread>

#include "ui_filter_log_dialog.h"

namespace GS{
namespace Gex
{
class AdminEngine;
}
}
class GexYMEventLogLoader;
//-------------GexYMEventBasicLogFilter---------------------
class GexYMEventLogFilter{
public:
    static QStringList m_soDataTimeOperator;
    static QStringList m_soIntegerOperator;
    static QStringList m_soEnumOperator;
    static QStringList m_soDoubleOperator;
    static QStringList m_soTextOperator;
    static int m_iGexYMEventLogFilterCounter;
public:
    enum YMEventLogFilterType{
        unknownType,
        datetimeType,
        integerType,
        enumType,
        doubleType,
        textType
    };

public:
    GexYMEventLogFilter();
    virtual ~GexYMEventLogFilter();

    void setIcon(const QString &strVal);
    QString getIcon();

    void setColor(const QString &strVal);
    QString getColor();

    void setXMLName(const QString &strName);
    QString getXMLName();

    void setLabel(const QString &strLabel);
    const QString &getLabel();

    void setIndex(int iIdx);
    int getIndex();

    bool setType(const QString &strVar);
    int getType();
    QString getStringType();


    void setFilterTree(bool bVal);
    bool getFilterTree();

    virtual void save(QDomElement &oElement, QDomDocument &oXMLDoc);

protected:
    QString m_strXMLName, m_strColor, m_strIcon, m_strLabel;
    YMEventLogFilterType m_iFilterType;
    bool m_bFilterTree;
    int m_iIdx;

};

//-------------GexYMEventBasicLogFilterElem------------------
class GexYMEventBasicLogFilterElem{
public:
    static QString m_strValuesSeparator;
    static int m_iGexYMEventBasicLogFilterElemCount;
public:
    QString m_strOperator, m_strBasicFilter, m_strOperand, m_strValue;
    GexYMEventBasicLogFilterElem(const QString &strOperator,
                            const QString &strBasicFilter,
                            const QString &strOperand,
                            const QString &strValue){

        m_strOperator   = strOperator;
        m_strBasicFilter = strBasicFilter;
        m_strOperand    = strOperand;
        m_strValue     = strValue;
        m_iGexYMEventBasicLogFilterElemCount++;
    }
    ~GexYMEventBasicLogFilterElem(){
        m_iGexYMEventBasicLogFilterElemCount--;

    }
    QString buildClause();
    QString buildOperand();


};

//--------------------GexYMEventBasicLogFilter-----------------
class GexYMEventBasicLogFilter : public GexYMEventLogFilter{

public:
    static GexYMEventBasicLogFilter *loadFromXML(QDomNode &oFilterNode, int &iError, QString &strError);
    QStringList fetchFromDBTable(const QString &strField, const QString &strTable, const QString &strFriendlyName,bool bRefresh=false);
    QStringList fetchFromEventTable(const QString &strField, bool bRefresh=false);

    GexYMEventBasicLogFilter();
    virtual ~GexYMEventBasicLogFilter();

    bool addEnum(int iIdx, const QString &strName, const QString &strLabel, const QString &strIcon, const QString & strColor);
    QMap<QString, QString> &getEnum(int iIdx);
    QList <int> getEnumIdx();
    QStringList getEnumList();

    void setFetchFrom(const QString &strFetchFrom);
    QString getFetchFrom();

    void setFetchFromEventTable(bool bVal);
    bool getFetchFromEventTable();

    void setFriendlyName(const QString &strName){
        m_strFriendlyName = strName;
    }
    QString getFriendlyName(){
        return m_strFriendlyName;
    }

    QStringList getOperatorChoice();

private:
    QMap<int, QMap<QString, QString> > m_oEnumValues;
    QString m_strFetchFrom;
    QString m_strFriendlyName;
    bool m_bFetchFromEventTable;

    bool m_bEventFieldLoaded;
    bool m_bDBFieldLoaded;
    QStringList m_oDBFieldValues, m_oEventFieldValues;

};

//--------------GexYMEventExpressionLogFilter--------
class GexYMEventExpressionLogFilter : public GexYMEventLogFilter{

public:

    static GexYMEventExpressionLogFilter *loadFromXML(QDomNode &oFilterNode, int &iError, QString &strError);
    GexYMEventExpressionLogFilter();
    virtual ~GexYMEventExpressionLogFilter();

    void clear();

    void addElem(const QString &strOperator,
                 const QString &strBasicFilter,
                 const QString &strOperand,
                 const QString &strValue){
        m_oExpression.append(new GexYMEventBasicLogFilterElem(strOperator,strBasicFilter,strOperand,strValue));
    }
    void addDynamicElem(const QString &strOperator,
                 const QString &strBasicFilter,
                 const QString &strOperand,
                 const QString &strValue){
        m_oDynamicExpression.append(new GexYMEventBasicLogFilterElem(strOperator,strBasicFilter,strOperand,strValue));
    }
    void save(QDomElement &oElement, QDomDocument &oXMLDoc);
protected:

    QList <GexYMEventBasicLogFilterElem *> m_oExpression;
    QList <GexYMEventBasicLogFilterElem *> m_oDynamicExpression;

public:
    QList <GexYMEventBasicLogFilterElem *> &getStaticPart () {
        return m_oExpression;
    }
    QList <GexYMEventBasicLogFilterElem *> &getDynamicPart(){
        return m_oDynamicExpression;
    }

    QString buildWhereClause();
};

//-------------GexYMEventLogSettings---------------------
class GexYMEventLogSettings{
    QStringList m_oViewerFields;
    QString m_strLogLabel;
    QString m_strIcon;
    QMap<int, GexYMEventBasicLogFilter *> m_oFilters;
    QMap<int, GexYMEventExpressionLogFilter *> m_oFiltersExpression;
    QMap<int, GexYMEventExpressionLogFilter *> m_oNewUserFilters;

public:
    static int m_iGexYMEventLogSettingsCount;

    GexYMEventLogSettings();
    ~GexYMEventLogSettings();
    void setLogLabel(const QString &strLabel);
    QString  getLogLabel();
    void setLogIcon(const QString &strIcon);
    QString  getLogIcon();

    bool addFilter(int iIdx, GexYMEventBasicLogFilter *poFilter);
    QMap<int, GexYMEventBasicLogFilter *> &getFilters();
    QString getFriendlyName(const QString &strName);
    GexYMEventBasicLogFilter *getFilter(const QString &strXMLName);

    bool addPredefinedFilters(int iIdx, GexYMEventExpressionLogFilter *poFilter);
    QMap<int, GexYMEventExpressionLogFilter *> &getPredefinedFilters();

    QMap<int, GexYMEventExpressionLogFilter *> &getNewUserFilters(){
        return m_oNewUserFilters;
    }
    QStringList getViewerFields();
    void setViewerFields(const QStringList &oFields);

};

//------------------GexYMEventLog------------------------
class GexYMEventLogLoader{
public:
    enum YMEventLogError{
        noError,
        invalidPath,
        invalidXML,
        invalidVersion,
        missingAttributes,
        missingTag
    };
public:
    static int m_iGexYMEventLogLoaderCount;
    GexYMEventLogLoader();
    ~GexYMEventLogLoader();
    bool saveEventTableConfig();
private:
    GexYMEventLogSettings m_oGexYMEventLogSettings;
    static QString m_strEventTableVersion;
    int m_iError;
    QString m_strError;
    QDomDocument m_oXMLDoc;

protected:
    bool loadFilters();
    bool loadBasicFilters(QDomNode &oEventTable);
    bool loadPredinedFilters(QDomNode &oEventTable);
    bool loadExtensions(QDomNode &oEventTable);
    bool loadViewer(QDomNode &oEventTable);

    bool saveBasicFilters(QDomDocument &oXMLDoc);
    bool savePredinedFilters(QDomDocument &oXMLDoc);
    bool saveExtensions(QDomDocument &oXMLDoc);
    bool saveViewer(QDomDocument &oXMLDoc);

    bool isCompatible(const QString &strVersion);
    void updateProgress(int iLocalProgress, int iStep);

public:
    int getErrorCode();
    const QString &getError();
    void setError(int iError, const QString &strError );
    QMap<QString, int> getFilters();
    GexYMEventLogSettings &getGexYMEventLogSettings(){
        return m_oGexYMEventLogSettings;
    }
    int m_iStepCount;
    int m_iStep;
};

//---------------------GexQueryThread----------------------
class GexQueryThread: public QThread
{
public:
    GexQueryThread(QObject* poObj, const QString &strConnectionName);
    virtual ~GexQueryThread();
    bool exec(const QString& strQuery);

    void setQuery(const QString &strQuery){
        m_strQuery = strQuery;
    }
    QString getQuery(){
        return m_strQuery;
    }

    bool getQueryStatus(){
        return m_bStatus;
    }
    QString getQueryLastError();

    QSqlQuery *getQueryResult(){
        return m_poQuery;
    }
private:
   void run();
private:
   QString m_strQuery;
   QString m_strConnectionName;
   bool m_bStatus;
   QSqlQuery *m_poQuery;

public:
   static int m_iGexQueryThreadCount;
};
//---------------------GexYMEventLogDB----------------------
class GexYMEventLogDB{
public:
    static QString m_strSeparator;
    static int m_iGexYMEventLogDBCount;
protected:
    GexYMEventLogDB(GS::Gex::AdminEngine *poYM);
    ~GexYMEventLogDB();

public:
    static GexYMEventLogDB *getInstance();
    static void deleteInstance();

    GS::Gex::AdminEngine *getAdminServerConnector(){
        return m_poAdminEngine;
    }

    QStringList getFieldValues(const QString &strTable, const QString &strField);
    QStringList getFieldValues(const QString &strTable1, const QString &strTable2, const QString &strFieldJoin, const  QString &strField);
    QStringList getMainEventsList(const QStringList &oFields, const QString &strLimitTo, QStringList &oLinkedEventList);
    QStringList getLinkedEventsListTo(const QString &strEvent,const QStringList &strFields);
    QString getLinkedEventsStatus(const QString &strEvent);
    QStringList getResult(const QStringList &strFields, GexYMEventExpressionLogFilter *poElem);
protected:
    GS::Gex::AdminEngine *m_poAdminEngine;
public:
    static GexYMEventLogDB *m_poInstance;
};

//------------------GexYMEventLogBasicFilterWidget------------------------
class GexYMEventLogBasicFilterWidget : public QWidget{
    Q_OBJECT
public:
    static int m_iGexYMEventLogBasicFilterWidgetCount;
    GexYMEventLogBasicFilterWidget(QWidget *poParent, GexYMEventBasicLogFilter *poFilter,GexYMEventLogLoader *poGexYMEventLog=0);
    virtual ~GexYMEventLogBasicFilterWidget();
    void saveElemFilter(GexYMEventExpressionLogFilter *poExpressionFilter);
    void initValues(GexYMEventBasicLogFilterElem *poElem);
    bool isDynamic();
public slots :
    void buildOperatorField(int );
    void buildFieldChoice(int );
    void removeFilter();
protected:
    QString getOperator();
    QWidget *getNumreicValues(int iType, bool bRange ,QWidget *poParent, const QString &strText);
    QWidget *getDateValues(bool bRange, QWidget *poParent, const QString &strText);
    QWidget *getTextValues(QWidget *poParent, const QString &strText);
    QWidget *getEnumValues(QWidget *poParent, const QStringList &oValues, bool bSet, const QString &/*strText*/);

    QString toNumreicValues(QWidget *poWidget);
    QString toDateValues(QWidget *poWidget);
    QString toTextValues(QWidget *poWidget);
    QString toEnumValues(QWidget *poWidget);

    void setNumreicValues(QWidget *poWidget, const QString &strValues);
    void setDateValues(QWidget *poWidget, const QString &strValues);
    void setTextValues(QWidget *poWidget, const QString &strValues);
    void setEnumValues(QWidget *poWidget, const QString &strValues);


protected:
    QComboBox *m_poComboBoxFilters;
    QComboBox *m_poComboBoxOperator;
    QWidget   *m_poFieldChoice;
    QPushButton *m_poRemoveFilter;
    QCheckBox *m_poIsDynamic;

    GexYMEventLogLoader *m_poGexYMEventLog;
    GexYMEventBasicLogFilter *m_poSingleFilter;
};

class GexYMEventFilterDialog :  public QDialog, public Ui::FiltersDialog{
    Q_OBJECT
    QPushButton *m_poRemoveFilter;
    bool m_bRemoveFilter;
public:
    static int m_iGexYMEventFilterDialogCount;
    GexYMEventFilterDialog(QWidget *poParent, GexYMEventLogLoader *poGexYMEventLog);
    ~GexYMEventFilterDialog();
    GexYMEventLogLoader *m_poGexYMEventLog;
public slots:
    void save(GexYMEventExpressionLogFilter *poFilter);
    void addFilter();
    void reloadFilters(GexYMEventExpressionLogFilter *poFilter, bool bCanRemove=false);
    void accept();
    void removeFilter();
    bool isRemoved(){
        return m_bRemoveFilter;
    }
};

class GexYMEventLogSelection;
class GexYMEventLogViewer : public QWidget{
    Q_OBJECT
public:
    enum FixedFieldsPosition{
        StatusColumn  =0,
        DetailColumn  =1,
        EventIDColumn =2
    };
    static int m_iGexYMEventLogViewerCount;
    GexYMEventLogViewer(QWidget *poParent, GexYMEventLogSettings *poGexYMEventLogSettings, const QStringList &oFieldsList,GexYMEventExpressionLogFilter *poFilter=0);
    virtual ~GexYMEventLogViewer();

public slots:
    void refresh ();
    void autorefresh();
    void setAutoreferesh(int bAuto);
    void viewDetails();
    void findEntry();
    void clearFind();
    void exportToCSV();
    void updateProgressBar(int iValue=-1);
public:
    void setRefreshInterval(int iMsec);
    bool getAutorefresh();
    void setFieldsList(const QStringList &oFieldsList);
    void enableGui(bool bEnable);
    bool isEmpty(){
        return m_poTreeWidget->topLevelItemCount() == 0;
    }

protected:
    QString getIconFromStatus(const QString &strStatus);
    void addNewEntry(QStringList &oMainEventProperty,  QStringList &oLinkedEventList, int iInsertionIdx = -1);
    void generateDetails(QTreeWidgetItem *poMainEvents, const QStringList &oLinkedEventList);
    void addHtmlTableRow(const QString &strLablel, const QString &strData, QString &strHtmlDetails);
    QString getFriendlyName(int iIdx, const QString &strProp);

protected:
    bool m_bAutoRefresh;
    bool m_bRefreshingOnGoing;
    GexYMEventLogSettings *m_poGexYMEventLogSettings;
    GexYMEventExpressionLogFilter *m_poFilter;
    QTimer m_oTimer;
    QStringList m_oFieldsList;
    GexYMEventLogSelection *m_poSelection;
    QTreeWidget *m_poTreeWidget;
    QLineEdit *m_poSearch;
    QCheckBox *m_poUseRegExpression;
    QPushButton *m_poFind, *m_poFindClear, *m_poFilterExport;
    QProgressBar *m_poDBProgressBar;
    QString m_strMaxEventIdFetched;
public:
    void setLogSelection(GexYMEventLogSelection *poSelection){
        m_poSelection = poSelection;
    }
protected:
    void buildRefreshPart(QHBoxLayout *poHorizontalLayout, QWidget *poWidgetParent);
    void enableRefreshButton(bool bEnable);
    int getRefreshTime();

    QPushButton *m_poFilterRefresh;
    QCheckBox *m_poFilterAutoRefresh;
    QSpinBox *m_poFilterRefreshTime;
    QComboBox *m_poFilterRefreshTimeUnits;


};

class GexYMEventLogSelection : public QWidget{

    Q_OBJECT
    enum WidgetUsage{
        ePredefinedFilter,
        eEventLogFilter
    };

public:
    static int m_iGexYMEventLogSelectionCount;
    GexYMEventLogSelection(GexYMEventExpressionLogFilter *poPredinedFilter, GexYMEventLogSettings *poGexYMEventLogSettings,  GexYMEventLogLoader *poGexYMEventLog);
    GexYMEventLogSelection(GexYMEventLogSettings *poGexYMEventLogSettings, GexYMEventLogLoader *poGexYMEventLog);
    ~GexYMEventLogSelection();
    void connectTo(GexYMEventLogViewer *poViewer);
    GexYMEventExpressionLogFilter *buildFilter();
protected:
    void initAttributes();
    void buildSelectionPart(GexYMEventExpressionLogFilter *poPredinedFilter, GexYMEventLogSettings *poGexYMEventLogSettings);
    void buildSelectionPart(GexYMEventLogSettings *poGexYMEventLogSettings);

protected:
    QComboBox *m_poFilterSelection;
    QWidget *m_poDynamic;
    GexYMEventLogViewer *m_poViewer;
    GexYMEventLogLoader *m_poGexYMEventLog;

protected slots:
    void refreshGUI(int iIdx);
    void editFilter();
protected:
    void defineNewFilter(int iIdx);
    void updateLayout(QWidget *poWidget);
protected:
    GexYMEventExpressionLogFilter *m_poPredinedFilter;
    GexYMEventLogSettings *m_poGexYMEventLogSettings;
    int m_iUsage;
};

#include <QStackedWidget>
class GexYMEventLogViewManager : public QObject{
    Q_OBJECT
public:
    enum eLogViewManagerStatus{
        noError  = 0,
        xmlError,
        guiError
    };
    static int  m_iGexYMEventLogViewManagerCount;
    GexYMEventLogViewManager(QTreeWidgetItem *poItem, QWidget *poContainer);
    ~GexYMEventLogViewManager();
    void updateLogViewer(QTreeWidgetItem *poItem);
    QTreeWidgetItem *getYMEventLogTopNode();
public slots :
    void reloadViewManager();
    void saveViewManager();

protected:
    void clear();
    void initLogViewerManager();
    void setYMEventLogWidgetContainer (QWidget *poContainer);
    //Node Decoration
    void setYMEventLogTopNode(QTreeWidgetItem *poItem);
    void addFiltersSubNodes(QTreeWidgetItem *poYMEventLogTopNode);
    QString getYMEventLogTopNodeIcon();
    QString getYMEventLogTopNodeLabel();


protected:
    void connectFilterViewer(GexYMEventLogSelection *poSelection,  GexYMEventLogViewer *poViewer);

protected:
    QTreeWidgetItem *m_poTopNode;
    QWidget *m_poContainer;
    QVBoxLayout *m_poContainerGrid;

    GexYMEventLogDB *m_poDB;
    GexYMEventLogLoader *m_poLoader;

    QStackedWidget *m_poFiltersStack;
    QMap<int , QWidget *> m_oFiltersMapping;

    QString m_strError;
    int m_iError;

    QStringList m_oFields;

public:
    //Error Handling
    int getErrorCode(){
        return m_iError;
    }
    QString getError(){
        return m_strError;
    }
    void setError(int iError, const QString &strError){
        m_iError = iError;
        m_strError = strError;
    }
    void handleError(QWidget *poWidget);
};

#endif // YM_EVENT_LOG_GUI_H
