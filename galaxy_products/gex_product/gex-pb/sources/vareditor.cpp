#include "propbrowser.h"
#include <gqtl_log.h>
#include <QFileDialog>
#include <QTextEdit>

#include "pb_directory_widget.h"

//CVariantEditorFactory::CVariantEditorFactory(QObject *parent) : QtVariantEditorFactory(parent)
CVariantEditorFactory::CVariantEditorFactory(CPropertyIdManager* cpimPtrIdManager, QObject *parent /*= 0*/) :
    QtVariantEditorFactory(parent)
{
    //GSLOG(6, (char*)" ");
    m_cpimPtrIdManager = cpimPtrIdManager;
}

CVariantEditorFactory::~CVariantEditorFactory()
{
    //GSLOG(5, (char*)" to do : delete %d QTextEdit !", m_PropertyToTextEditMap.size() );
  //  QMap<QtProperty*, QTextEdit*>::iterator it;
    //m_PropertyToTextEditMap
}

void CVariantEditorFactory::SlotDirectoryEditDestroyed(QObject * obj)
{
    const QMap<PbDirectoryWidget*, QtProperty*>::iterator ecend = m_DirectoryEditorToProperty.end();
    for (QMap<PbDirectoryWidget*, QtProperty*>::iterator itEditor = m_DirectoryEditorToProperty.begin(); itEditor !=  ecend; ++itEditor)
    {
        if (itEditor.key() == obj)
        {
            m_DirectoryEditorToProperty.erase(itEditor);
            return;
        }
    }
}

void CVariantEditorFactory::SlotDirectoryEditChanged(const QString& value)
{
    PbDirectoryWidget * dw = (PbDirectoryWidget*) this->sender();

    if (dw==NULL)
        return;

    QtProperty * p = m_DirectoryEditorToProperty.value(dw, 0);
    if (!p)
        return;

    CVariantManager* cvmPtrVariantManager =  (CVariantManager*)propertyManager(p);

    if(!cvmPtrVariantManager)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid variant manager pointer !!");
        GEX_ASSERT(false);
    }

    cvmPtrVariantManager->setValue(p, QVariant(value));
}

QWidget* CVariantEditorFactory::createEditor(QtVariantPropertyManager *manager,
                                            QtProperty *property, QWidget *parent)
{

    /// TO REVIEW : create our custom widget to include in the property browser ..

    GSLOG(SYSLOG_SEV_DEBUG, QString(" for property '%1' ")
           .arg(property?property->propertyName().toLatin1().data():"?").toLatin1().constData() );
    if ( (!property) || (!manager) )
        return NULL;

    const int type = manager->propertyType(property);

    GSLOG(SYSLOG_SEV_DEBUG, QString("type = %1 for property '%2' ")
          .arg(type)
           .arg(property->propertyName()).toLatin1().constData());


    //OptionsCenterPropBrowser::SOption soOptionStructure;

//	if ( (type!=QtVariantPropertyManager::flagTypeId())
//		 && (type!=QVariant::Bool)
//		 &&	(!m_OCPB->GetOptionForProperty((QtVariantProperty*) property, soOptionStructure))
//		)
//	{
//		GSLOG(4,"error : cant find SOption for this property. Quiting.");
//		return NULL;
//	}

    if (type==PATH_TYPE)
    {
        int nPropertyId = m_cpimPtrIdManager->GetPrivatePropertyID(property);
        QExplicitlySharedDataPointer<CGexPublicProperty> cgppShPtrProperty = m_cpimPtrIdManager->GetPublicProperty(nPropertyId);

        QString             lDefaultValue   = cgppShPtrProperty->GetAttributeValue("defaultvalue");
        QStringList         qslFiltersList  = cgppShPtrProperty->GetAttributeValue("filter").split("|", QString::SkipEmptyParts);
        PbDirectoryWidget*  dw              = m_DirectoryEditorToProperty.key(property, 0);

        if (dw == NULL)
        {
            dw = new PbDirectoryWidget(parent);
            dw->setValue(manager->value(property).toString());
            dw->setFilters(qslFiltersList);
            dw->setDefaultValue(lDefaultValue);

            connect(dw, SIGNAL(valueChanged(const QString&)),
                    this, SLOT(SlotDirectoryEditChanged(const QString &)));
            connect(dw, SIGNAL(destroyed(QObject*)),
                    this, SLOT(SlotDirectoryEditDestroyed(QObject*)));

            m_DirectoryEditorToProperty.insert(dw, property);
        }

        return dw;
    }
    else if (type==HTML_TYPE)		/// TO IMPROVE : cf. charac project
    {
        qDebug("To Do : HTML editor");
        //QTextEdit te(manager->value(property).toString(), 0);
        QTextEdit* te=NULL;
        if (m_PropertyToTextEditMap.find(property)==m_PropertyToTextEditMap.end())
        {
            te=new QTextEdit(manager->value(property).toString(), 0);
            m_PropertyToTextEditMap.insert(property, te);
            connect(te, SIGNAL(textChanged()), this, SLOT(SlotTextEditChanged()) );
            //te->setAcceptRichText(true);
        }
        else
            te=m_PropertyToTextEditMap.value(property);
        te->show();

        return NULL;
    }
    /// TO DO : type == JS_TYPE
    // */

    QWidget *editor = 0;
    editor = QtVariantEditorFactory::createEditor(manager, property, parent);
    if (!editor)
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("no editor registered for '%1' with this type (%2) !")
                 .arg(property->propertyName().toLatin1().data())
                 .arg(type).toLatin1().constData());

    return editor;
}

void CVariantEditorFactory::SlotTextEditChanged()
{
    /// TO REVIEW : text display management in gexpb
    GSLOG(SYSLOG_SEV_INFORMATIONAL, " ");
    QTextEdit *te=(QTextEdit*)this->sender();
    if (te==NULL)
        return;
    QtProperty *p=m_PropertyToTextEditMap.key(te);
    if (!p)
        return;
    CVariantManager* cvmPtrVariantManager =  (CVariantManager*)propertyManager(p);
    if(!cvmPtrVariantManager)
    {
        GSLOG(SYSLOG_SEV_CRITICAL, "Invalid variant manager pointer !!");
        GEX_ASSERT(false);
    }

    QString strOptionValue = te->toPlainText();
    strOptionValue.replace("\n", "<br>");
    cvmPtrVariantManager->setValue(p, QVariant(strOptionValue));
    // PYC, 08/09/2011,
    // replace \n characters by <br> to assume text insertion in a html report

    //cvmPtrVariantManager->setValue(p, QVariant(te->toHtml()));
    //m_OCPB->m_variantManager->setValue(p, QVariant(te->toHtml()));
    qDebug("SlotTextEditChanged: property found !");
}
