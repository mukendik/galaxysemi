#include <algorithm>
#include "cslscriptparser.h"

namespace GEX
{

    namespace CSL {


        CslScriptParser::CslScriptParser()
        {

        }

         bool CslScriptParser::startWith(const QString& strStartWith)
         {
             return mStartWith.startsWith(strStartWith, Qt::CaseInsensitive);
         }

         bool CslScriptParser::init (const QString &inputFillString)
         {
             mParameterList.clear();

             if(inputFillString.isEmpty() || inputFillString.startsWith("//"))
                 return false;

             //-- retroeve the begin of the string (gexGroup, gexQuery, gexCondition ...
             mStartWith = inputFillString.mid(0, inputFillString.indexOf('(')).trimmed();

             // -- to manage the case "group_id = gexGroup('insert_query'"
             // -- remove the "group_id ="
             int lStartIndex = mStartWith.indexOf('=') +1;
             if(lStartIndex > 0)
             {
                mStartWith = mStartWith.mid(lStartIndex).trimmed();
             }

             //-- retrieve string between "(" and ")"
             int lIndexFirstBracket = inputFillString.indexOf('(');
             int lIndexLastBracket  = inputFillString.lastIndexOf(')');
             if(lIndexFirstBracket<0 || lIndexLastBracket<0 )
                 return false;

             QString lContent = inputFillString.mid(lIndexFirstBracket + 1, lIndexLastBracket - lIndexFirstBracket - 1);

             //-- remove all the "'" character
             lContent.replace("'", "");

             //-- extract the list of content
             mParameterList = lContent.split(",");

             return ((mParameterList.empty()==false) && mStartWith.isEmpty() == false);
         }

         QString CslScriptParser::getElementFile(T_GEX_FILE elementNum)
         {
            return getElementInt(elementNum);
         }

         QString CslScriptParser::getElementGroup(T_GEX_GROUP elementNum)
         {
             return getElementInt(elementNum);
         }

         QString CslScriptParser::getElementQuery(T_GEX_QUERY elementNum)
         {
            return getElementInt(elementNum);
         }

         QString CslScriptParser::getElementCondition(T_GEX_CONDITION elementNum)
         {
             return getElementInt(elementNum);
         }


         QString CslScriptParser::getElementInt(int elementNum)
         {
             if(elementNum > mParameterList.size() )
                    return "";
             else
                 return mParameterList[elementNum].trimmed();
         }

    }
}
