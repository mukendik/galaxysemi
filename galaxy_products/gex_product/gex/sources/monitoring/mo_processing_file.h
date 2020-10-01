#ifndef MO_PROCESSINGFILE_H
#define MO_PROCESSINGFILE_H

#include <QObject>
#include <QDomElement>
#include <QMap>

#include <mo_processing_file_elt.h>

#define XML_PROCESS_FILE             ".process.xml"

class ProcessingElt;
class ProcessingAction;

class GexMoProcessingFile : public QObject
{
    Q_OBJECT
public:
    GexMoProcessingFile(QObject *parent = 0);

    // Load process file from .xml file
    bool        LoadFromXmlFile(QString processedFile);
    // Save process file to .xml file into destination folder
    bool        SaveToXmlFile(const QString &processedFile) const;

    QVariant    GetNodeAttribute(const QString &key);
    void        SetNodeAttribute(const QString &key, QVariant value);

    QVariant    GetActionCommandAttribute(const QString &key);
    void        SetActionCommandAttribute(const QString &key, QVariant value);

    QVariant    GetActionStatusAttribute(const QString &key);
    void        SetActionStatusAttribute(const QString &key, QVariant value);
private:

    QString             mVersion;
    ProcessingElt       mNode;
    ProcessingAction    mAction;

    // Load the object using the dom element
    bool        LoadFromDom(QDomDocument &domDocument);
    // return the dom element with the object description
    QDomElement GetDomElt(QDomDocument &domDoc) const ;
    // LOAD QDomElement Functions
    // Load main TAGS
    bool LoadNodeFromDom(const QDomElement &node);     // node TAG
    bool LoadActionFromDom(const QDomElement &node);   // action TAG

    // DUMP QDomElement Functions
    // Dump main TAGS
    QDomElement GetNodeDom(QDomDocument &doc) const;   // node TAG
    QDomElement GetActionDom(QDomDocument &doc) const; // action TAG

};

#endif // MO_PROCESSINGFILE_H
