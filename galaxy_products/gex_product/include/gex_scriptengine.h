#ifndef GEX_SCRIPTENGINE_H
#define GEX_SCRIPTENGINE_H

// DEFINES for Properties registered in the ScriptEngine by Gex or OptionsCenter or ...
// Warning : if the property is unfindable, the Property() function will return an invalid QVariant

// SystemOS can be : WV_NT, WV_2000, WV_XP, WV_2003, WV_VISTA, WV_WINDOWS7, Sun-Solaris, HP-UX, Linux
#define PROP_OS				"SystemOS"
//#define PROP_APP_FULL_NAME		"AppFullName" // Moved to GexEngine::GetAppFullName()

// the language and country of the os locale as a string of the form "language_country"
#define PROP_LOCALE_NAME		"LocaleName"
// Language as defined in QLocale
#define PROP_LOCALE_LANGUAGE		"LocaleLanguage"
// Country as defined in QLocale
#define PROP_LOCALE_COUNTRY		"LocaleCountry"
// as defined in QLocale : '.' or ',' or ...
#define PROP_LOCALE_DECIMALPOINT	"LocaleDecimalPoint"

#define PROP_GEX_USER_FOLDER            "GexUserFolder"
#define PROP_APP_DIR			"ApplicationDir"
// version of our product
#define PROP_GEXPRODUCT			"GexProduct"
//  PROP_GEXPRODUCT will have one and only one of those values :
#define	PRODUCT_YIELDMAN		"PRODUCT_YIELDMAN"
#define	PRODUCT_PATSERVER		"PRODUCT_PATSERVER"
#define	PRODUCT_GEXPAT			"PRODUCT_GEXPAT"
#define	PRODUCT_GEX             "PRODUCT_GEX"
#define	PRODUCT_GEXPRO			"PRODUCT_GEXPRO"
#define	PRODUCT_GTM             "PRODUCT_GTM"
//
#define PROP_OPTIONAL_MODULES  "OptionalModules"

// can be true or false
#define PROP_DEBUG				"Debug"
//
#define	 PROP_PRODUCT_ID		"ProductID"
#define	 PROP_LOT_ID			"LotID"
#define	 PROP_SUBLOT_ID			"SubLotID"

// Contains list of additional modules separated by "|"
#define	PROP_ADDITIONAL_MODULES	"AdditionalModules" 
// ...

// Contains list of loglevel specified modules separated by "|"
#define PROP_MODULE_LOGLEVEL "ModuleLogLevel"

#include <QScriptEngine>

//extern
template <typename T>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, T const &qobject);

//extern
template <typename T>
void qScriptValueToQObject(const QScriptValue &value, T &qobject);

//extern
template <typename T>
extern int qScriptRegisterQObjectMetaType( QScriptEngine *engine, const QScriptValue &prototype = QScriptValue(), T* /* dummy */ = 0);

class GexScriptEngine : public QScriptEngine
{
    //Q_PROPERTY(bool )
public:
    GexScriptEngine(QObject* p):QScriptEngine(p)
	{
        //GSLOG(5, QString("Building the GexScriptEngine : parent is '%1'").arg(p?p->objectName():"NULL").toLatin1().data() );
	}
    ~GexScriptEngine()
	{
        //GSLOG(4, " Deleting the GexScriptEngine...");
	}

    Q_INVOKABLE bool Reset() {  return true; }
    Q_INVOKABLE void CollectGarbage() { QScriptEngine::collectGarbage(); }
};

#endif // GEX_SCRIPTENGINE_H
