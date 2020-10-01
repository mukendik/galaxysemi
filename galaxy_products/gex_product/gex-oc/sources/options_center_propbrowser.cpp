#include <QTreeWidget>
#include <QApplication>

#include "options_center_propbrowser.h"
#include <gqtl_log.h>
#include "gex_scriptengine.h"

#include "gex_pixmap.h"
#include "gex_pixmap_extern.h"

extern GexScriptEngine* s_gse;

OptionsCenterPropBrowser::OptionsCenterPropBrowser(QWidget* p) : QWidget(p)//: QtTreePropertyBrowser(p)
{
    GSLOG(7, "new OptionsCenter PropBrowser...");
    m_ocw=(OptionsCenterWidget*)p;

    m_cgpPtrPropBrowser = CGexPB::CreatePropertyBrowser(s_gse, p, 40);
    setLayout(new QVBoxLayout());
    layout()->addWidget(m_cgpPtrPropBrowser);

    connect(m_cgpPtrPropBrowser, SIGNAL(SignalGPbPropertyValueChanged(int,const QVariant&)), this, SLOT(ValueChanged(int,const QVariant&)));
    QObject::connect(m_cgpPtrPropBrowser, SIGNAL(SignalMessage(QString)), this, SIGNAL(SignalMessage(QString)));

    //GSLOG(7,"configuring propbrowser...");
}

OptionsCenterPropBrowser::~OptionsCenterPropBrowser()
{
    GSLOG(6," ");

    if(m_cgpPtrPropBrowser)
        CGexPB::DestroyPropertyBrowser(m_cgpPtrPropBrowser);
}


QString OptionsCenterPropBrowser::BuildFromGOXML(QDomElement *docElem)
{
    emit SignalMessage("OptionsCenterPropBrowser Build from xml...");

    bool bParseDomEltRslt = ParseDomElem(docElem, -1);		// recursive !
    QString strReturnValue;

    if(bParseDomEltRslt)
    {
        strReturnValue = QString("ok");
    }
    else
    {
        strReturnValue = QString("error : problem occured during options file parsing");
    }

    /// TO REVIEW : refresh pb not mandatory now each property is specifically refreshed when it is inserted
    if(!m_cgpPtrPropBrowser->RefreshPB())
    {
        GSLOG(3, "can't refresh gex property browser !");
        GEX_ASSERT(false);
    }

    m_cgpPtrPropBrowser->SlotSetExpanded(false);

    return strReturnValue;
}

bool OptionsCenterPropBrowser::SetOption(QString s, QString f, QString newvalue)
{
    GSLOG(7, QString(" '%1'-'%2' to '%3'")
           .arg(s.toLatin1().data())
           .arg(f.toLatin1().data())
           .arg(newvalue).toLatin1().constData());

    if (m_CSLToPropertyMap.find(s+"_"+f)==m_CSLToPropertyMap.end())
        return false;

    int nPropertyID=m_CSLToPropertyMap.value(s+"_"+f);
    if (nPropertyID==-1)
        return false;

    if (m_CSLToSOptionMap.find(s+"_"+f)==m_CSLToSOptionMap.end())
        return false;
    SOption so=m_CSLToSOptionMap[s+"_"+f];

    bool bSetValueRslt = m_cgpPtrPropBrowser->SetValue(nPropertyID, QVariant(newvalue));
    return bSetValueRslt;
}

QVariant OptionsCenterPropBrowser::GetOption(QString section, QString field)
{
    if (m_CSLToPropertyMap.find(section+"_"+field)==m_CSLToPropertyMap.end())
        return QVariant();
    int nPropertyID=m_CSLToPropertyMap.value(section+"_"+field);
    if (nPropertyID==-1)
        return QVariant();

    //if (m_CSLToSOptionMap.find(s+"_"+f)==m_CSLToSOptionMap.end())
      //  return false;

    return m_cgpPtrPropBrowser->GetCurrentValue(nPropertyID);
}

// recursive
bool OptionsCenterPropBrowser::ParseDomElem(QDomElement *qdeParsedElt, int nParentPropertyID)
{
    if (!s_gse)
    {
        GSLOG(4, "GexScriptEngine NULL !");
        return false;
    }

    QDomNode qdnParsedEltChildNode = qdeParsedElt->firstChild();

    while(!qdnParsedEltChildNode.isNull())
    {
        emit SignalMessage(QString("Loading (%1)...").arg(nParentPropertyID));
        QCoreApplication::processEvents(); // to try to update QLabel in GUI....
        //QScriptValue sv=s_gse->evaluate(QString("GexMainWindow.UpdateScriptStatusLabel('Loading OptionsCenter (%1)...')").arg(nParentPropertyID) );
        //emit m_ocw->signalMessage(QString("Loading (%1)...").arg(nParentPropertyID) );

        QDomElement qdeParsedEltChildElement = qdnParsedEltChildNode.toElement(); // try to convert the node to an element.
        if (  (qdeParsedEltChildElement.isNull())
            || (qdeParsedEltChildElement.attribute("label")=="")
            || (qdeParsedEltChildElement.attribute("type")=="")
            || (qdeParsedEltChildElement.tagName()!="option")
            || (qdeParsedEltChildElement.attribute("cslname")=="") )
        {
            if (!qdeParsedEltChildElement.isNull())
            {
                GSLOG(2, QString("incomplete xml element tag:'%1' csl:'%2' label:'%3' type:'%4'")
                         .arg(qdeParsedEltChildElement.tagName().toLatin1().data())
                         .arg(qdeParsedEltChildElement.attribute("cslname").toLatin1().data())
                         .arg(qdeParsedEltChildElement.attribute("label").toLatin1().data())
                         .arg(qdeParsedEltChildElement.attribute("type").toLatin1().data()).toLatin1().constData()
                         );
                GEX_ASSERT (false);
                return false;
            }
            qdnParsedEltChildNode = qdnParsedEltChildNode.nextSibling();
            continue;
        }

        QString parentCSLname="";
        if (!qdeParsedEltChildElement.parentNode().isNull())
        {
            if (!qdeParsedEltChildElement.parentNode().toElement().isNull())
                parentCSLname=qdeParsedEltChildElement.parentNode().toElement().attribute("cslname");
        }

        QString cslField=qdeParsedEltChildElement.attribute("cslname");

        QScriptValue qsvScriptValue;
        if (!qdeParsedEltChildElement.attribute("defaultvalue").isEmpty())
        {
            qsvScriptValue=s_gse->evaluate(qdeParsedEltChildElement.attribute("defaultvalue"));
            if (qsvScriptValue.isError())
            {
                GSLOG(4, QString("error when evaluating defaultvalue for '%1' : '%2' : %3")
                             .arg(qdeParsedEltChildElement.attribute("label").toLatin1().data())
                             .arg(qdeParsedEltChildElement.attribute("defaultvalue").toLatin1().data())
                             .arg(s_gse->uncaughtException().toString().toLatin1().data()).toLatin1().constData()
                             );
            }
        }

        GSLOG(6, QString(" %1 %2 : '%3'")
                .arg(parentCSLname.toLatin1().data())
                .arg(cslField.toLatin1().data())
                .arg(qdeParsedEltChildElement.attribute("label")).toLatin1().constData() );

        int nPropertyItemID = -1;

        SOption so;
        so.m_nPropertyID=-1;
        so.m_CSLField=cslField;
        so.m_CSLSection=parentCSLname;

        QDomNamedNodeMap qdnnmAttributeNodeMap = qdeParsedEltChildElement.attributes();
        QMap<QString, QString > qmAttributesMap;

        for(int ii=0; ii<qdnnmAttributeNodeMap.count(); ii++)
        {
            QDomNode qdnAttributeNode = qdnnmAttributeNodeMap.item(ii);
            QDomAttr qdaAttributeElement = qdnAttributeNode.toAttr();

            QString strAttributeName = qdaAttributeElement.name();

            // Do not overwrite tooltip with desc (GCORE-3266)
            // if (strAttributeName == QString("desc"))
            //    qmAttributesMap.insert(QString("tooltip"), qdaAttributeElement.value());
            // else
                qmAttributesMap.insert(strAttributeName, qdaAttributeElement.value());

        }

        nPropertyItemID = m_cgpPtrPropBrowser->AddProperty(qmAttributesMap, nParentPropertyID);
        if (nPropertyItemID==-1)
        {
            GSLOG(4, QString("option type '%1' unknown/unsupported. Ignoring.")
                     .arg(qdeParsedEltChildElement.attribute("type")).toLatin1().constData());
            qdnParsedEltChildNode = qdnParsedEltChildNode.nextSibling();
            continue;
        }

        QCoreApplication::processEvents();

        so.m_nPropertyID=nPropertyItemID;

        if (cslField!="")
        {
            m_CSLToSOptionMap.insert(parentCSLname+"_"+cslField, so);
            m_CSLToPropertyMap.insert( parentCSLname+"_"+cslField, nPropertyItemID);
            m_PropertyToCslKeyMap.insert(nPropertyItemID, parentCSLname+"_"+cslField);
        }

        if (qdeParsedEltChildElement.hasChildNodes())
        {
            if(!ParseDomElem(&qdeParsedEltChildElement, nPropertyItemID))		// recursive call
                return false;
        }
        qdnParsedEltChildNode = qdnParsedEltChildNode.nextSibling();
    }

    return true;
}


void OptionsCenterPropBrowser::ValueChanged(int nPropertyID, const QVariant &v)
{
    if(nPropertyID==-1)
    {
        //GSLOG(5, (char*)"WARNING : invalid property ID !");
        /// TO REVIEW : warning is emitted during the pb building
        return;
    }

    QString cslkey=m_PropertyToCslKeyMap.value(nPropertyID);

    if (cslkey=="")
    {
        GSLOG(5, QString("no csl key for property (# %1)!").arg( nPropertyID).toLatin1().constData());
        return;
    }

    GSLOG(6, QString("Value changed : found csl key '%1'").arg(cslkey).toLatin1().constData());

    if (m_CSLToSOptionMap.find(cslkey)==m_CSLToSOptionMap.end())
        return;

    SOption so=m_CSLToSOptionMap[cslkey];

    QString value=v.toString();

    emit SignalOcPbValueChanged(so.m_CSLSection, so.m_CSLField, value);

    //if (m_ocw)
    //	m_ocw->EmitOptionChanged(so.m_CSLSection, so.m_CSLField, value);

    // in order to refresh hideif attribute. Check me !
    m_cgpPtrPropBrowser->RefreshPB();
}


void OptionsCenterPropBrowser::SlotExpandAll(bool bExpandAll)
{
    m_cgpPtrPropBrowser->SlotSetExpanded(bExpandAll);
}
