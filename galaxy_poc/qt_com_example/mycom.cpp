#include "mycom.h"
#include <objbase.h>

MyCOM::MyCOM()
{
    mComSubObject = NULL;
    mComObject = NULL;
}

MyCOM::~MyCOM()
{
    if(mComObject)
        delete mComObject;
}

void MyCOM::onInit()
{
#ifdef USE_THREAD
    HRESULT lResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(lResult != S_OK)
        return;
#endif

    mComObject = new QAxObject;
    if(!mComObject)
        return;

#ifdef TERCOM
    mComObject->setControl("TAGLMProxyLib.TAGLMProxy");
//    mComObject->setControl("{6CF443EB-0BA0-43EA-AC69-16650FF833D3}");
#else
    mComObject->setControl("InternetExplorer.Application");
#endif

     if(mComObject->isNull())
     {
        emit sStatus(QString("ERROR: Error initializing COM object!"));
        return;
     }

     emit sComInitialized(mComObject->generateDocumentation());
     emit sStatus(QString("INIT OK!"));
}

void MyCOM::onQuery()
{
    if(!mComObject || mComObject->isNull())
    {
       emit sStatus(QString("ERROR: COM object not initialized!"));
       return;
    }

#ifdef TERCOM
    QString lErr;
    mComSubObject = mComObject->querySubObject("CheckOut(QString, QString&, QString, QString, bool)", QString("ZZZZ"), lErr, QString("0.0"), QString(""), QString("true"));
    if(!mComSubObject)
    {
       emit sStatus(QString("ERROR: Error querying COM subobject: %1").arg(lErr));
       return;
    }
    emit sSubObjectInitialized(mComSubObject->generateDocumentation());
#else
    mComSubObject = mComObject->querySubObject("Application()");
    if(!mComSubObject)
    {
       emit sStatus(QString("ERROR: Error querying COM subobject"));
       return;
    }
    emit sSubObjectInitialized(mComSubObject->generateDocumentation());
#endif

    emit sStatus(QString("QUERY OK!"));
}

