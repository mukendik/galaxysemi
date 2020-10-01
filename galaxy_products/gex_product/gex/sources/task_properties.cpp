#include <QMap>

///////////////////////////////////////////////////////////////////////////////////
// GS Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_log.h>

#include "task_properties.h"

TaskProperties::TaskProperties(QObject *parent): QObject(parent)
{
}

TaskProperties &TaskProperties::operator=(const TaskProperties &copy)
{
    if(this != &copy)
    {
        mAttributes             = copy.mAttributes;
        mAttributesKey          = copy.mAttributesKey;
        mPrivateAttributes      = copy.mPrivateAttributes;
        mPrivateAttributesKey   = copy.mPrivateAttributesKey;
    }
    return *this;
}

void TaskProperties::ResetAllAttributes()
{
    mAttributes.clear();
    mAttributesKey.clear();
    ResetPrivateAttributes();
}

void TaskProperties::ResetPrivateAttributes()
{
    mPrivateAttributesKey.clear();
    mPrivateAttributes.clear();
}


bool TaskProperties::SetPrivateAttribute(const QString &key, const QVariant &value)
{
    QString lLowerKey = key.toLower();
    QString lFormattedKey = key;
    if(mPrivateAttributesKey.contains(lLowerKey))
        lFormattedKey = mPrivateAttributesKey.value(lLowerKey);
    else
        mPrivateAttributesKey.insert(lLowerKey,lFormattedKey);

    mPrivateAttributes.insert(lFormattedKey,value);

    return true;
}


QVariant TaskProperties::GetAttribute(const QString &key) const
{
    QString lLowerKey = key.toLower();
    QString lFormattedKey = key;

    // Manage CASE INSENSITIVITY
    if(mPrivateAttributesKey.contains(lLowerKey))
    {
        lFormattedKey = mPrivateAttributesKey.value(lLowerKey);

        // Get READ-ONLY key
        return mPrivateAttributes.value(lFormattedKey);
    }

    // Manage CASE INSENSITIVITY
    if(mAttributesKey.contains(lLowerKey))
    {
        lFormattedKey = mAttributesKey.value(lLowerKey);
        // Get the value
        return mAttributes.value(lFormattedKey);
    }

    return QVariant();
}

bool TaskProperties::SetAttribute(const QString &key, const QVariant &value)
{
    QString lLowerKey = key.toLower();
    QString lFormattedKey = key;

    // Check if it is a READ-ONLY key
    if(mPrivateAttributesKey.contains(lLowerKey))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Try to update a READ-ONLY property[%1]")
              .arg(key).toLatin1().constData());
        return false;
    }

    // Manage CASE INSENSITIVITY
    if(mAttributesKey.contains(lLowerKey))
        lFormattedKey = mAttributesKey.value(lLowerKey);
    else
        mAttributesKey.insert(lLowerKey,lFormattedKey);

    // Get the value
    mAttributes.insert(lFormattedKey, value);
    return true;
}

bool TaskProperties::UpdateAttribute(const QString &key, const QVariant &value)
{
    QString lLowerKey = key.toLower();

    // Update in the right attribute map (private/public)
    if(mPrivateAttributesKey.contains(lLowerKey))
        return SetPrivateAttribute(key, value);
    if(mAttributesKey.contains(lLowerKey))
        return SetAttribute(key, value);

    return false;
}

const QMap<QString, QVariant> TaskProperties::GetAttributes()
{
    QMap<QString, QVariant> lAllAttributes;
    lAllAttributes = mPrivateAttributes;
    QMap<QString, QVariant>::const_iterator itAttribute;
    for(itAttribute = mAttributes.begin(); itAttribute != mAttributes.end(); ++itAttribute)
    {
        if(mPrivateAttributesKey.contains(itAttribute.key().toLower()))
            continue;
        lAllAttributes.insert(itAttribute.key(),itAttribute.value());
    }

    return lAllAttributes;
}
