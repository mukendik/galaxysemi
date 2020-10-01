///////////////////////////////////////////////////////////
// GEX utilities classes using QT: implementation file
///////////////////////////////////////////////////////////

#ifdef _WIN32
    // overloading the WINVER/_WIN32_WINNT to access new EX API
    #define WINVER 0x0500
    #include <windows.h>
#endif

// Std headers
#include <stdio.h>
#include <QVariant>
#include <QMetaProperty>

// Project headers
#include "gqtl_utils.h"

namespace GS
{
  namespace QtLib
  {

    QString QObjectToJSON(QObject* lQObject, QString &lOutput, bool lIncludeChildren, QString &lIndent)
    {
        if (!lQObject)
            return "error: QObject null";
        // dyn properties
        foreach (QString key, lQObject->dynamicPropertyNames())
        {
            lOutput.append(lIndent+QString("\"%1\":\"%2\", \n").arg(key)
                           .arg(lQObject->property(key.toLatin1().data()).toString().replace('\\','/') ) );
        }
        // static prop
        const QMetaObject* lMO=lQObject->metaObject();
        for(int i=0; i<lMO->propertyCount(); i++)
        {
            lOutput.append(lIndent+QString("\"%1\":\"%2\" ").arg(lMO->property(i).name())
                           .arg(lQObject->property( lMO->property(i).name() ).toString() ) );
            if (i<lMO->propertyCount()-1)
                lOutput.append(", \n");
            //lOutput.append("\n");
        }
        //
        if(lIncludeChildren)
        {
            if (lQObject->children().size()>0)
                lOutput.append(lIndent+",");

            for (QList<QObject*>::const_iterator lIt=lQObject->children().begin(); lIt!=lQObject->children().end(); lIt++)
            {
                if (*lIt)
                {
                    QString lI=QString(lIndent+"  ");
                    lOutput.append(lIndent+ " \""+(*lIt)->objectName()+"\":" + "{\n");
                    QObjectToJSON(*lIt, lOutput, true, lI);
                    lOutput.append(lIndent+" }");
                    if (lIt!=(--lQObject->children().end()--))
                        lOutput.append(",");
                    lOutput.append("\n");
                }
            }
        }

        return "ok";
    }

  } // QtLib
} // GS

#ifdef _WIN32
// Prototype of Windows GetUserProfileDirectory() function
typedef BOOL (STDMETHODCALLTYPE FAR * LPFNGETUSERPROFILEDIR) (
	HANDLE hToken,
	LPTSTR lpProfileDir,
	LPDWORD lpcchSize
);
#endif


