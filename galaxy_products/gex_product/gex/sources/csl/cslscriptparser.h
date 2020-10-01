#ifndef CSLSCRIPTPARSER_H
#define CSLSCRIPTPARSER_H

#include <QString>
#include <QStringList>

namespace GEX
{
    namespace CSL {
        
        //-- gexFile(group_id,'insert','<file path>','<site#>','<parts type>','<part range>','<map file>', '<WaferIDfilter>', '<temperature>','dataset name');
        enum T_GEX_FILE{File_Filter = 0, File_Action, File_FilePath, File_Site, File_Part_Type, File_PartRange, File_MapFile, File_WaferIdFilter, File_Temperature, File_DataSetName, File_ControlGroup};
        
        //-- gexGroup('insert','<group name>'), gexGroup('declare_condition','<Test_condition>');
        enum T_GEX_GROUP{Group_Action = 0, Group_Value};
        
        //-- gexQuery('db_report','<Report name>'), gexQuery('dbf_sql','<filter_name>','<filter_value>')
        enum T_GEX_QUERY{Query_Tag = 0, Query_Value, Query_Value2};
        
        //-- (group_id,'<Test_condition_name>','<test_condition_value>');
        enum T_GEX_CONDITION{Condition_Filter= 0, Condition_Name, Condition_Value};
        
        
        class CslScriptParser
        {
        public:
            CslScriptParser();
            
            bool startWith(const QString& strStartWith);
            
            /**
             * @brief init. Set the input string. The element will be  extracted and put in a list
             * @param inputFillString : string directly read from the file ex : gexFile(group_id,'insert','<file....
             */
            bool init(const QString &inputFillString);
            
            
            QString getElementFile      (T_GEX_FILE         elementNum);
            QString getElementGroup     (T_GEX_GROUP        elementNum);
            QString getElementQuery     (T_GEX_QUERY        elementNum);
            QString getElementCondition (T_GEX_CONDITION    elementNum);
            
        private:
            QStringList     mParameterList;
            QString         mStartWith;
            
            /**
             * @brief getElement. Return the element at the position "elementNum". If no element return empty string
             * @param elementNum. Number of the element.
             * @return the element at the position or empty string
             */
            QString getElementInt(int elementNum);
        };
    }
}

#endif // CSLSCRIPTPARSER_H
