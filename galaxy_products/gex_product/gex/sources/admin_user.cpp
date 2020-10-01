#include "admin_engine.h"
#include "engine.h"

bool AdminUser::TraceUpdate(QString Command,QString Status,QString Summary)
{
    // Update the Events flow
    GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewEvent("UPDATE","USER",
                         0,m_nUserId,0,Command,
                         Status,Summary,"");
    return true;
}

void AdminUser::SetAttribute(const QString &key, QVariant value)
{
    mAttributes.remove(key);
    if(!value.toString().isEmpty())
        mAttributes.insert(key, value);
}
