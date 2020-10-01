#include "gqtl_log.h"
#include <QFile>
#include "xml_parser.h"


XML::XMLParser::XMLParser():mScannedIndex(0), mProgressStep(0), mTotalNodes(0)
{
}

XML::XMLParser::~XMLParser()
{
}


/**
 * @brief XML::XMLManager::LoadXMLFile
 * @param fileToLoad - the absolute pathnam of the xml file to load
 * @return true if the loading succeed.
 */
bool XML::XMLParser::LoadXMLFile(const char *fileToLoad)
{
    if(!QFile::exists(fileToLoad)) {
        GSLOG(SYSLOG_SEV_ERROR, QString("File not created %1").arg(fileToLoad).toLatin1().constData());
        return false;
    }

    QFile lOFile(fileToLoad);
    if(!lOFile.open(QIODevice::ReadOnly)) {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can not open %1").arg(fileToLoad).toLatin1().constData());
        return false;
    }

    QString     lErrorMessage;
    int         lErrorLine=-1, lErrorColumn=-1;
    if(!mQDomDocument.setContent(&lOFile, true, &lErrorMessage, &lErrorLine, &lErrorColumn)) {
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid XML file Error(%1) at Line(%2) and Column(%3)").arg(lErrorMessage).arg(lErrorLine).arg(lErrorColumn).toLatin1().constData());
        return false;
    }

   CalculNbTotalNodes(mQDomDocument);
   mCurrentElement = mQDomDocument.documentElement();
   // QDomNodeList childList = mCurrentElement.childNodes();
   /*  QDomNodeList childList = mCurrentElement.elementsByTagName("SupplierData").at(0).childNodes();

     QString name;
    for(int i = 0; i < childList.size(); ++i)
    {
         name = childList.at(i).toElement().tagName();
         name = childList.at(i).toElement().localName();
    }

  int size = mCurrentElement.elementsByTagName("SetupFile").size();
    size = mCurrentElement.elementsByTagName("Substrate").size();
  size = mCurrentElement.elementsByTagName("SupplierName").size();*/
 // mCurrentElement.elementsByTagName("Substrate").at(0).toElement().

   return true;
}

void XML::XMLParser::CalculNbTotalNodes(QDomNode node)
{
    QDomNodeList lListChild = node.childNodes();
    for(int i = 0; i < lListChild.size(); ++i) {
        if(lListChild.at(i).hasChildNodes())
            CalculNbTotalNodes(lListChild.at(i));

        if(lListChild.at(i).hasAttributes())
            mTotalNodes += lListChild.at(i).attributes().count();
        ++mTotalNodes;
    }

}

QString XML::XMLParser::GetNameSpaceURI(const QString& tagName) const
{
    return GetNameSpaceURI(mCurrentElement, tagName);
}

QDomElement XML::XMLParser::GetDomElement(QDomElement element, const QString& tagName) const
{
    if(!element.tagName().compare(tagName))
        return element;
    else {
        QDomNodeList lListElement = element.elementsByTagName(tagName);
        if(lListElement.size() > 0)
            return lListElement.at(0).toElement();
        else
            return mNullElement;
    }
}

QString XML::XMLParser::GetAttribute(const QString& tagName, const QString& attributeName) const
{
    return GetAttribute(mCurrentElement, tagName, attributeName);
}

QString XML::XMLParser::GetAttribute(QDomElement element, const QString& tagName, const QString& attributeName) const
{
    UpdateProgressStep();
    return GetDomElement(element, tagName).attribute(attributeName);
}


QString XML::XMLParser::GetText(const QString &tagName) const
{
    return GetText(mCurrentElement, tagName);
}

QString XML::XMLParser::GetText(QDomElement element, const QString &tagName) const
{
    UpdateProgressStep();
    return GetDomElement(element, tagName).text();
}

QString XML::XMLParser::GetNameSpaceURI(QDomElement element, const QString& tagName) const
{
    UpdateProgressStep();
    return GetDomElement(element, tagName).namespaceURI();
}

QString XML::XMLParser::GetRepeatedNodeText() const
{
    UpdateProgressStep();
    if(mScannedIndex > mScannedElements.size()) {
        return QString();
    }
    else {
        return mScannedElements.at(mScannedIndex).toElement().text();
    }
}

void XML::XMLParser::UpdateProgressStep() const
{
    ++mProgressStep;
    QList<Observer*>::const_iterator lIterBegin(mListObservers.begin()), lIterEnd(mListObservers.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin){
        (*lIterBegin)->Notify();
    }
}

void  XML::XMLParser::SetRepeatedNodes(const QString& tagName)
{
    mScannedElements    = mQDomDocument.elementsByTagName(tagName);
    mScannedIndex       = -1;
}

QString XML::XMLParser::GetRepeatedNodeAttribut(const QString& attributeName) const
{
    UpdateProgressStep();
    if(mScannedIndex > mScannedElements.size()) {
        return QString();
    }
    else {
        return mCurrentElement.attribute(attributeName);
    }
}


bool  XML::XMLParser::NextRepeatedNode()  {
    ++mScannedIndex;
    if(mScannedIndex >= mScannedElements.size()) {
        mCurrentElement = mQDomDocument.documentElement();
        return false;
    }
    else {
        mCurrentElement = mScannedElements.at(mScannedIndex).toElement();
    }

    return true;
}

