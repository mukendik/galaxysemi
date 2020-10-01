#ifndef TASK_PROPERTIES_H
#define TASK_PROPERTIES_H

#include <QVariant>
#include <QObject>
#include <QMap>
#include "task_properties.h"

class SchedulerEngine;

class TaskProperties : public QObject
{
    Q_OBJECT
public:
    TaskProperties(QObject* parent);

    TaskProperties& operator= (const TaskProperties &copy);

public slots:
    /*! \brief Get/Set attribute :
           - OutlieredDistribs : list of distribs to consider when identifying outliers (-1, *, 2-10, ...)
           - ProductID :
           - ExpirationDate
      */
    QVariant                        GetAttribute(const QString &key) const;
    bool                            SetAttribute(const QString &key, const QVariant &value);
    const QMap<QString, QVariant>   GetAttributes();

private:

    // Map containing the READ-WRITE parameters of the task
    // exple : "CheckType"="FixedYieldTreshold", "Title"="Coucou",...
    // mAttributes.insert( "PostImportCrash", 1 );
    // mAttributes.insert( "PostImportCrashMoveFolder", "path" );

    QMap<QString, QVariant> mAttributes;
    // To manage CASE INSENSITIVITY
    // Contains mypropertykey = MyPropertyKey
    QMap<QString, QString> mAttributesKey;

    // To manage READ-ONLY parameters
    // Contains all READ-ONDY task properties
    // ie all private members in lower case
    // ie all properties not into mAttributes
    // accesible with GetAttribute method
    // Contains MyPropertyKey = value
    QMap<QString, QVariant> mPrivateAttributes;
    // To manage CASE INSENSITIVITY
    // Contains mypropertykey = MyPropertyKey
    QMap<QString, QString> mPrivateAttributesKey;

public:

    /*! \brief Reset attributes :
           - reset public and private attributes
           - never in public slot
      */
    void ResetAllAttributes();
    /*! \brief Reset Private attributes :
           - reset private attributes
           - never in public slot
      */
    void ResetPrivateAttributes();
    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    virtual void UpdatePrivateAttributes()=0;
    /*! \brief Set Private attribute :
           - Update the value of a private attribute
           - never in public slot
      */
    bool SetPrivateAttribute(const QString &key, const QVariant &value);
    bool UpdateAttribute(const QString &key, const QVariant &value);
};

#endif // TASK_PROPERTIES_H
