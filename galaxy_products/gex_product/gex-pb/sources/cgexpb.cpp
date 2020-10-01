#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

#include "gex_scriptengine.h"
#include <gqtl_log.h>

#include "propbrowser.h"
#include "cgexpb.h"
#include "cgexpublicproperty.h"
#include "cpropertyidmanager.h"

int CGexPB::n_instances=0;

CGexPB::CGexPB(QWidget* qwParent, GexScriptEngine* cgseScriptEngine, int nIndentation /* = 20 */) : QWidget()
{
	n_instances++;

	m_cgsePtrScriptEngine=cgseScriptEngine;
	m_cpbPtrPropertyBrowser=new CPropBrowser(qwParent, m_cgsePtrScriptEngine);

	setLayout(new QVBoxLayout());
	layout()->addWidget(m_cpbPtrPropertyBrowser);

	/// TO DO : accessors
	m_cpbPtrPropertyBrowser->setPropertiesWithoutValueMarked(false);
	m_cpbPtrPropertyBrowser->setRootIsDecorated(false);
	m_cpbPtrPropertyBrowser->setIndentation(nIndentation);
	m_cpbPtrPropertyBrowser->setResizeMode(CPropBrowser::ResizeToContents);

	Connections();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("now %1 instances of GexPropertyBrowser")
          .arg(n_instances).toLatin1().constData());
}

CGexPB::~CGexPB()
{
/*	if(m_cpbPtrPropertyBrowser)
  {
    delete m_cpbPtrPropertyBrowser;
		m_cpbPtrPropertyBrowser=0;
    }*/
	n_instances--;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("still %1 instances of GexPropertyBrowser")
          .arg(n_instances).toLatin1().constData());
}

CGexPB* CGexPB::CreatePropertyBrowser(
  GexScriptEngine* cgseScriptEngine, QWidget* qwParent, int nIndentation /* = 20 */)
{
	if (!cgseScriptEngine || !qwParent)
		return NULL;

	CGexPB*	pb=new CGexPB(qwParent, cgseScriptEngine, nIndentation);

	return pb;
}

bool CGexPB::DestroyPropertyBrowser(CGexPB* cgpPtr)
{
    if (cgpPtr)
        delete cgpPtr;
	return true;
}

int CGexPB::AddProperty(CGexPublicProperty cgppNewProperty)
{
    blockSignals(true);
    int nAddPropertyRslt = m_cpbPtrPropertyBrowser->AddProperty(cgppNewProperty);
    blockSignals(false);

    return nAddPropertyRslt;
}


int CGexPB::AddProperty(QMap< QString, QString > qmAttributes, int nParent_id /* =-1 */)
{
    blockSignals(true);
    int nAddPropertyRslt =  m_cpbPtrPropertyBrowser->AddProperty(qmAttributes, nParent_id);
    blockSignals(false);

    return nAddPropertyRslt;
}


bool CGexPB::SetValue(int id, QVariant newvalue)
{
	blockSignals(true);
	bool bSetPropertyRslt = m_cpbPtrPropertyBrowser->SetPropertyValue(id, newvalue);
	blockSignals(false);

	return bSetPropertyRslt;
}

bool CGexPB::SetAttribute(QString strAttributeLabel, QString strAttributeValue, int nPropertyId)
{return (m_cpbPtrPropertyBrowser->GetProperty(nPropertyId))->SetAttribute(strAttributeLabel, strAttributeValue);}

bool CGexPB::SetAttribute(const QMap<QString, QString > qmNewAttributeMap, const int nPropertyId)
{return (m_cpbPtrPropertyBrowser->GetProperty(nPropertyId))->SetAttribute(qmNewAttributeMap);}

//bool CGexPB::SetParentIdProperty(const int nPropertyId, const int nNewParentIdProperty)
//{
//	return (m_cpbPtrPropertyBrowser->GetProperty(nPropertyId))->SetParentPropertyId(nPropertyId);
//}

QExplicitlySharedDataPointer<CGexPublicProperty>	CGexPB::GetProperty(const int nPropertyId)
{return (m_cpbPtrPropertyBrowser->GetProperty(nPropertyId));}

// Get the current value of the property
QVariant	CGexPB::GetCurrentValue(int nPropertyId) const
{return m_cpbPtrPropertyBrowser->GetCurrentValue(nPropertyId);}


//////////////////////
// global methods
//////////////////////
bool CGexPB::RefreshPB()
{
	blockSignals(true);
	bool bRefreshRslt = m_cpbPtrPropertyBrowser->Refresh();
	blockSignals(false);

	return bRefreshRslt;
}


/////////////////////////
// public slots
/////////////////////////
void CGexPB::SlotPropertyValueChanged(int id,const QVariant& new_value)
{emit SignalGPbPropertyValueChanged(id, new_value);}

void CGexPB::SlotSetExpanded(const bool bIsExpanded)
{m_cpbPtrPropertyBrowser->SetExpandedAllProperties(bIsExpanded);}

bool CGexPB::Reset()
{
	blockSignals(true);
	bool bResetRslt = m_cpbPtrPropertyBrowser->Reset();
	blockSignals(false);

	return bResetRslt;
}



/////////////////////////////
// internal methods
/////////////////////////////
void CGexPB::Connections()
{
	connect(m_cpbPtrPropertyBrowser, SIGNAL(SignalPbPropertyValueChanged(int,const QVariant&)), this, SLOT(SlotPropertyValueChanged(int,const QVariant&)));
}



/// TO REMOVE : latter
// AddProperty ...
//	if (
//		(qmAttributes.find("label")==qmAttributes.end())
//		|| (qmAttributes.find("type")==qmAttributes.end())
//		)
//	{
//		GSLOG(4, " no label or type attribute !");
//		return -1;
//	}

//	if (qmAttributes.find("hideif")!=qmAttributes.end())
//	{
//		QScriptValue qsvScriptValue=m_cgsePtrScriptEngine->evaluate( qmAttributes["hideif"] );
//		if (qsvScriptValue.isError())
//		{
//			GSLOG(4, (char*)" error while evaluating javascript expression : '%s' : %s",
//					qmAttributes["hideif"].toLatin1().data(),
//					m_cgsePtrScriptEngine->uncaughtException().toString().toLatin1().data() );
//			// What do we have to do ? Hide or not ?
//		}
//		else if (qsvScriptValue.toBool()==true)
//		{
//			return -1;
//		}
//	}

//	QScriptValue qsvScriptValue;
//	if (qmAttributes.find("defaultvalue")!=qmAttributes.end())
//	{
//		qsvScriptValue=m_cgsePtrScriptEngine->evaluate(qmAttributes["defaultvalue"] );
//		if (qsvScriptValue.isError())
//		{	GSLOG(5, (char*)"error when evaluating defaultvalue for '%s' : '%s' : %s",
//						 qmAttributes["label"].toLatin1().data(),
//						 qmAttributes["defaultvalue"].toLatin1().data(),
//						 m_cgsePtrScriptEngine->uncaughtException().toString().toLatin1().data()
//						 );
//		}
//	}

//	QtVariantProperty *qvpPtrVariantProperty=NULL;

//	if (qmAttributes["type"]=="Group")
//	{
//		qvpPtrVariantProperty = m_cpbPtrPropertyBrowser->m_cvmPtrVariantManager->addProperty(
//				QtVariantPropertyManager::groupTypeId(), qmAttributes["label"]
//				);
//		//item->setValue(QVariant("sdfgsdfgs"));
//		if (!qvpPtrVariantProperty)
//			return -1;
//		//vp->setStatusTip("sqdfqsdv"); // unuseful : status tip
//		//vp->setToolTip("qdfvsfvszfv");
//		//item->setEnabled(false);
//	}
//	else
//	if (qmAttributes["type"]=="Int")
//	{
//		qvpPtrVariantProperty = m_cpbPtrPropertyBrowser->m_cvmPtrVariantManager->addProperty(QVariant::Int, qmAttributes["label"] );
//		//item->setValue(20);
//		//item->setAttribute(QLatin1String("minimum"), QVariant(de.attribute("minimum")) );
//		//item->setAttribute(QLatin1String("maximum"), QVariant(de.attribute("maximum")) );
//		//item->setAttribute(QLatin1String("singleStep"), QVariant(de.attribute("singleStep")) );
//		if ( (!qsvScriptValue.isError()) && qsvScriptValue.isValid())
//				qvpPtrVariantProperty->setValue( QVariant( qsvScriptValue.toInt32()) );
//	}

//	m_cpbPtrPropertyBrowser->addProperty(qvpPtrVariantProperty);

//	return 1;


