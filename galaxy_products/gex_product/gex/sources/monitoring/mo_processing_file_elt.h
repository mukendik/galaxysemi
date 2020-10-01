#ifndef MO_PROCESSINGFILEELT_H
#define MO_PROCESSINGFILEELT_H

#include <QObject>
#include <QDomElement>
#include <QMap>
#include <QVariant>

class ProcessingElt
{
public:
    ProcessingElt();

    // Load the object using the dom element
    bool        LoadFromDom(const QDomElement &node, const QString Tag);
    // return the dom element with the object description
    QDomElement GetDomElt(QDomDocument &domDoc) const ;

    QString     mTag;
    QString     mType;

    QVariant    GetAttribute(const QString &key)                    { return mAttributes.value(key); }
    void        SetAttribute(const QString &key, QVariant value)    { mAttributes.insert(key, value); }
    const QMap<QString, QVariant> GetAttributes()                   {return mAttributes;}

private:
    // Map containing the parameters of this command
    QMap<QString, QVariant> mAttributes;

    bool        LoadEltFromDom(const QDomElement &node);
    QDomElement GetEltDom(QDomDocument &doc) const;
};



class ProcessingAction
{
public:
    // Load the object using the dom element
    bool        LoadFromDom(const QDomElement &node);
    // return the dom element with the object description
    QDomElement GetDomElt(QDomDocument &domDoc) const ;

    QString             mType;
    ProcessingElt       mCommand;
    ProcessingElt       mStatus;

    bool LoadCommandFromDom(const QDomElement &node);
    bool LoadStatusFromDom(const QDomElement &node);

    QDomElement GetCommandDom(QDomDocument &doc) const;
    QDomElement GetStatusDom(QDomDocument &doc) const;

};


#endif // MO_PROCESSINGFILEELT_H
