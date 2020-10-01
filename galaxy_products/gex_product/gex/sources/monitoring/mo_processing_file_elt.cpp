#include "mo_processing_file_elt.h"
//#include "libgexlog.h"
#include <gqtl_log.h>
#include <QDir>
#include <QDomDocument>
#include <QHostAddress>


ProcessingElt::ProcessingElt()
{
    mTag = "invalid_tag";
}

bool ProcessingElt::LoadEltFromDom(const QDomElement &node)
{
    QDomNode lNodeTable = node.firstChild();
     while (!lNodeTable.isNull())
     {
         if (lNodeTable.isElement())
         {
             QDomElement lEltTable = lNodeTable.toElement();
             if (!lEltTable.isNull())
                 SetAttribute(lEltTable.tagName(),lEltTable.text());
         }
         lNodeTable = node.nextSibling();
     }

    return true;
}

QDomElement ProcessingElt::GetEltDom(QDomDocument &doc) const
{
    QDomText domText;
    QDomElement domElt;

    foreach(QString lKey, mAttributes.keys())
    {
        domElt = doc.createElement(lKey);
        domText = doc.createTextNode(mAttributes[lKey].toString());
        domElt.appendChild(domText);
    }

    return domElt;
}

bool ProcessingElt::LoadFromDom(const QDomElement &node, const QString Tag)
{
    if (node.isNull())
        return false;

    mTag = Tag;

    QDomElement eltNode = node.firstChildElement(mTag);
    if (eltNode.isNull())
        return false;

    mType = eltNode.attribute("type");

    bool lIsValidDatabase = true;
    if (!LoadEltFromDom(eltNode))
        lIsValidDatabase = false;
    if (!lIsValidDatabase)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
               arg(mTag).toLatin1().data() );
        return false;
    }
    return true;
}

QDomElement ProcessingElt::GetDomElt(QDomDocument &doc) const
{
    QDomElement domEltDatabase = doc.createElement(mTag);

    domEltDatabase.setAttribute("type",mType);

    domEltDatabase.appendChild(GetEltDom(doc));

    return domEltDatabase;
}


bool ProcessingAction::LoadFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    QDomElement eltNode = node.firstChildElement("action");
    if (eltNode.isNull())
        return false;

    mType = eltNode.attribute("type");

    bool lIsValidDatabase = true;
    if (!LoadCommandFromDom(eltNode))
        lIsValidDatabase = false;
    if (!LoadStatusFromDom(eltNode))
        lIsValidDatabase = false;
    if (!lIsValidDatabase)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Invalid element %1").
               arg("action").toLatin1().data() );
        return false;
    }
    return true;
}

QDomElement ProcessingAction::GetDomElt(QDomDocument &doc) const
{
    QDomElement domEltDatabase = doc.createElement("action");

    domEltDatabase.setAttribute("type",mType);

    domEltDatabase.appendChild(GetCommandDom(doc));
    domEltDatabase.appendChild(GetStatusDom(doc));

    return domEltDatabase;
}

bool ProcessingAction::LoadCommandFromDom(const QDomElement &node)
{
    return mCommand.LoadFromDom(node,"command");
}

bool ProcessingAction::LoadStatusFromDom(const QDomElement &node)
{
    return mStatus.LoadFromDom(node,"status");
}

QDomElement ProcessingAction::GetCommandDom(QDomDocument &doc) const
{
    return mCommand.GetDomElt(doc);
}

QDomElement ProcessingAction::GetStatusDom(QDomDocument &doc) const
{
    return mStatus.GetDomElt(doc);
}
