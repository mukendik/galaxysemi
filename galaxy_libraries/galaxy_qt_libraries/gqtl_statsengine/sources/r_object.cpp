
#include <Rinternals.h>
#include "r_object.h"
#include "r_object_private.h"
#include "r_protected_object.h"
#include "g_sexp.h"

namespace GS
{
namespace SE
{

RObjectPrivate::RObjectPrivate()
{
    mGSexp = new GSexp();
    mProtectedObject = 0;
}


RObjectPrivate::RObjectPrivate(GSexp* gsexp)
{
    mGSexp = gsexp;
    mProtectedObject = 0;
}

RObjectPrivate::~RObjectPrivate()
{

    delete mGSexp;
    // if nothing protected to clean
    if (!mProtectedObject)
        return;

    QString lObjectName = mProtectedObject->GetName();


    SEXP lVar = Rf_findVarInFrame( R_GlobalEnv, Rf_install(lObjectName.toLatin1().data()));
    if( lVar == R_UnboundValue)
    {
        mErrors.append(QString("Error %1 not allocated").arg(lObjectName));
        return;
    }

    if (lObjectName.isEmpty())
        return;
    // release object protection
    delete mProtectedObject;
    mProtectedObject = 0;
    int lErrorOccurred;
    // remove object from environment
    R_tryEvalSilent(Rf_lang2(
                Rf_install("try"),
                Rf_lang2(
                    Rf_install("rm"),
                    Rf_install(lObjectName.toLatin1().data()))),
            R_GlobalEnv,
            &lErrorOccurred);
    if (lErrorOccurred)
        return;

    // force garbage collection (required ?)

    /*R_tryEvalSilent(Rf_lang2(
                Rf_install("try"),
                Rf_lang1(Rf_install("gc"))),
              R_GlobalEnv,
              &lErrorOccurred);*/

}


RObject::RObject():mPrivate(0)
{
}


RObject::RObject(RObjectPrivate &lPrivateData)
    :mPrivate(&lPrivateData)
{

}

RObject::RObject(RObjectPrivate &lPrivateData, GSexp* sexp)
    :mPrivate(&lPrivateData)
{
    mPrivate->mGSexp = sexp;
}




RObject::~RObject()
{
    CleanRMemory();
}

QString RObject::GetName() const
{
    QString lName;
    if (d_func() && d_func()->mProtectedObject)
        lName = d_func()->mProtectedObject->GetName();

    return lName;
}

GSexp *RObject::GetSexp() const
{
    return d_func()->mGSexp;
}

bool RObject::IsNull() const
{
    return !d_func() || !d_func()->mGSexp;
}

void RObject::CleanRMemory()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
    }
}

bool RObject::Install(const QString &name)
{
    Q_D(RObject);
    // assign this data in the global environment
    SEXP lSymbol = Rf_install(name.toLatin1().data());
    if (!lSymbol)
    {
        d->mErrors.append(QString("Unable to install %1").arg(name));
        return false;
    }
    // define var
    Rf_defineVar(lSymbol, d->mGSexp->mSexp, R_GlobalEnv);

    return true;
}

bool RObject::Protect(const QString& name)
{
    Q_D(RObject);
    if (d->mProtectedObject)
        CleanRMemory();

    if (!Install(name))
        return false;

    if(d->mProtectedObject == 0)
        d->mProtectedObject = new RProtectedObject(d->mGSexp->mSexp, name);
    else
        d->mProtectedObject = d->mProtectedObject;
    return true;
}

QString RObject::GetLastError() const
{
    QString lError;
    if (!d_func()->mErrors.isEmpty())
        lError = d_func()->mErrors.last();

    return lError;
}

QStringList RObject::GetErrors() const
{
    return d_func()->mErrors;
}

void RObject::ClearErrors()
{
    Q_D(RObject);
    d->mErrors.clear();
}

bool RObject::IsProtected() const
{
    return (d_func()->mProtectedObject != 0);
}

} // namespace SE
} // namespace GS

