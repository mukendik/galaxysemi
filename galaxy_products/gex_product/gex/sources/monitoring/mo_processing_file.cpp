#include "mo_processing_file.h"
#include "mo_processing_file_elt.h"
//#include "libgexlog.h"
#include <gqtl_log.h>
#include <QDir>
#include <QDomDocument>
#include <QHostAddress>



GexMoProcessingFile::GexMoProcessingFile(QObject *parent) :
    QObject(parent)
{

}

bool GexMoProcessingFile::LoadFromXmlFile(QString processedFile)
{
    QString xmlFile = processedFile+XML_PROCESS_FILE;
    if (xmlFile.isEmpty())
        return false;

    QFile lFile(xmlFile);
    if (!lFile.open(QIODevice::ReadOnly))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Unable to open %1").
               arg(xmlFile).
               toLatin1().data());
        return false;
    }

    QString lErrorMsg;
    int lErrorLine, lErrorColumn;
    QDomDocument lDomDocument;

    if (!lDomDocument.setContent(&lFile, &lErrorMsg, &lErrorLine, &lErrorColumn))
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("%1 at line %2, column %3.").
               arg(lErrorMsg).
               arg(QString::number(lErrorLine)).
               arg(QString::number(lErrorColumn)).
               toLatin1().data());
        return false;
    }

    // Load root elmt
    bool lStatus = LoadFromDom(lDomDocument);

    lFile.close();

    return lStatus;
}

bool GexMoProcessingFile::SaveToXmlFile(const QString &processedFile) const
{
    QString destinationFile = processedFile+XML_PROCESS_FILE;

    QFile file(destinationFile);
    QFileInfo fileInfo(file);
    QDir dir;

    // Check dir & create it if needed
    if (!fileInfo.dir().exists())
    {
        if (!dir.mkpath(fileInfo.dir().absolutePath()))
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("Unable to create  %1!")
                   .arg(fileInfo.dir().absolutePath())
                   .toLatin1().data());
            return false;
        }
    }
    // Check file & create it if needed
    if (!file.open(QIODevice::WriteOnly))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Unable to open %1! - %2")
               .arg(file.fileName()).arg(file.errorString())
               .toLatin1().data());
        return false;
    }

    // Create dom document
    QDomDocument domDocument;
    QDomElement elmtRoot = GetDomElt(domDocument);
    if (elmtRoot.isNull())
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Root Dom Element is NULL!")
               .toLatin1().data());
        return false;
    }
    // Set version
    elmtRoot.setAttribute("version", "1.0");
    domDocument.appendChild(elmtRoot);
    QTextStream textStream(&file);
    // Save dom doc to file
    domDocument.save(textStream, 4);
    file.close();
    return true;
}

bool GexMoProcessingFile::LoadFromDom(QDomDocument &domDocument)
{
    if (domDocument.isNull())
        return false;

    bool lIsValid = true;

    QDomElement lNodeRoot = domDocument.
            firstChildElement("galaxy_processing_file");

    mVersion = lNodeRoot.attribute("version");

    if(!LoadNodeFromDom(lNodeRoot))
        lIsValid = false;
    if(!LoadActionFromDom(lNodeRoot))
        lIsValid = false;

    if (!lIsValid)
        GSLOG(SYSLOG_SEV_ERROR, "Invalid galaxy_processing_file, unable to load");

    return lIsValid;
}

bool GexMoProcessingFile::LoadNodeFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    return mNode.LoadFromDom(node,"node");

}

bool GexMoProcessingFile::LoadActionFromDom(const QDomElement &node)
{
    if (node.isNull())
        return false;

    return mAction.LoadFromDom(node);

}

QDomElement GexMoProcessingFile::GetDomElt(QDomDocument &domDoc) const
{
    QDomElement domEltRoot = domDoc.createElement("galaxy_processing_file");

    // node
    domEltRoot.appendChild(GetNodeDom(domDoc));

    // action
    domEltRoot.appendChild(GetActionDom(domDoc));

    return domEltRoot;
}

QDomElement GexMoProcessingFile::GetNodeDom(QDomDocument &doc) const
{
    return mNode.GetDomElt(doc);
}

QDomElement GexMoProcessingFile::GetActionDom(QDomDocument &doc) const
{
    return mAction.GetDomElt(doc);
}


QVariant GexMoProcessingFile::GetNodeAttribute(const QString &key)
{
    return mNode.GetAttribute(key);
}

void GexMoProcessingFile::SetNodeAttribute(const QString &key, QVariant value)
{
    mNode.SetAttribute(key,value);
}

QVariant GexMoProcessingFile::GetActionCommandAttribute(const QString &key)
{
    return mAction.mCommand.GetAttribute(key);
}

void GexMoProcessingFile::SetActionCommandAttribute(const QString &key, QVariant value)
{
    mAction.mCommand.SetAttribute(key,value);
}

QVariant GexMoProcessingFile::GetActionStatusAttribute(const QString &key)
{
    return mAction.mStatus.GetAttribute(key);
}

void GexMoProcessingFile::SetActionStatusAttribute(const QString &key, QVariant value)
{
    mAction.mStatus.SetAttribute(key,value);
}

