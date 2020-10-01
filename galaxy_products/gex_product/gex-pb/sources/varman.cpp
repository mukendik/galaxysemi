#include <gqtl_log.h>
#include "propbrowser.h"
#include <QFileDialog>
#include <QTextEdit>

//CVariantManager::CVariantManager(QObject *parent)
CVariantManager::CVariantManager(CPropertyIdManager* cpimPtrIDManager, QObject *parent /*=0*/)
	: QtVariantPropertyManager(parent)
{
	m_cpimIdManager = cpimPtrIDManager;
    //GSLOG(7, " ");
	connect(this, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
				this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
	connect(this, SIGNAL(propertyDestroyed(QtProperty *)),
				this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

CVariantManager::~CVariantManager()
{
    //GSLOG(6, (char*)"");
	//clear(); ?
}

void CVariantManager::slotValueChanged(QtProperty* property,
									   const QVariant& /*value*/)
{
	/*
    GSLOG(7,(char*)" for '%s' : %s",
			 property->propertyName().toLatin1().data(),
			 value.toString().toLatin1().data());
	*/

	const int type = propertyType(property);
	if (type==QtVariantPropertyManager::enumTypeId())
	{
		// let s find the SOption later
	}

	//((OptionsCenterPropBrowser*)this->parent())->ValueChanged(property, value);

	/*
	if (xToProperty.contains(property))
	{
		QtProperty *pointProperty = xToProperty[property];
		QVariant v = this->value(pointProperty);
		QPointF p = qVariantValue<QPointF>(v);
		p.setX(qVariantValue<double>(value));
		setValue(pointProperty, p);
	} else if (yToProperty.contains(property)) {
		QtProperty *pointProperty = yToProperty[property];
		QVariant v = this->value(pointProperty);
		QPointF p = qVariantValue<QPointF>(v);
		p.setY(qVariantValue<double>(value));
		setValue(pointProperty, p);
	}
	*/
}

void CVariantManager::slotPropertyDestroyed(QtProperty* /*property*/)
{
    GSLOG(7, (char*)" ");
	/*
	if (xToProperty.contains(property)) {
		QtProperty *pointProperty = xToProperty[property];
		propertyToData[pointProperty].x = 0;
		xToProperty.remove(property);
	} else if (yToProperty.contains(property)) {
		QtProperty *pointProperty = yToProperty[property];
		propertyToData[pointProperty].y = 0;
		yToProperty.remove(property);
	}
	*/
}

bool CVariantManager::isPropertyTypeSupported(int propertyType) const
{
	if ( (propertyType== PATH_TYPE) || (propertyType== HTML_TYPE) || (propertyType== JS_TYPE))
		return true;
	return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int CVariantManager::valueType(int propertyType) const
{
	if (propertyType == PATH_TYPE)
		return PATH_TYPE;
	if (propertyType == HTML_TYPE)
		return 	HTML_TYPE;
	if(propertyType == JS_TYPE)
		return JS_TYPE;
	return QtVariantPropertyManager::valueType(propertyType);
}

QVariant CVariantManager::value(const QtProperty *property) const
{
	int nPropertyType = propertyType(property);

	if (propertyToData.contains(property))
		return propertyToData[property].value;
	else if (nPropertyType == PATH_TYPE)
	{
		QVariant qvReturnedValue((QVariant::Type)PATH_TYPE);
		qvReturnedValue.setValue(QString(""));
		return qvReturnedValue;
	}
	else if (nPropertyType == HTML_TYPE)
	{
		QVariant qvReturnedValue((QVariant::Type)HTML_TYPE);
		qvReturnedValue.setValue(QString(""));
		return qvReturnedValue;
	}
	else if (nPropertyType == JS_TYPE)
	{
		QVariant qvReturnedValue((QVariant::Type)JS_TYPE);
		qvReturnedValue.setValue(QString(""));
		return qvReturnedValue;
	}
	else
		return QtVariantPropertyManager::value(property);
}

QString CVariantManager::valueText(const QtProperty *property) const
{
	if (propertyToData.contains(property))
	{
		QVariant v = propertyToData[property].value;
		//QPointF p = qVariantValue<QPointF>(v);
		//return QString(tr("(%1, %2)").arg(QString::number(p.x()))
		//						 .arg(QString::number(p.y())));
		return v.toString();
	}
	return QtVariantPropertyManager::valueText(property);
}

void CVariantManager::setValue(QtProperty *property, const QVariant &val)
{
	if (!property)
	{
        GSLOG(4, "QtProperty NULL !");
		return;
	}

    //GSLOG(7,(char*)" for '%s' ", property->propertyName().toLatin1().data());
	const int type = propertyType(property);

	if ( (type==PATH_TYPE) || (type==HTML_TYPE) || (type==JS_TYPE))
	{
		Data d = propertyToData[property];
		d.value=val;
		propertyToData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, val);
		return;
	}


	if (propertyToData.contains(property))
	{
		//if (val.type() != QVariant::PointF && !val.canConvert(QVariant::PointF))
		//	return;
		//QPointF p = qVariantValue<QPointF>(val);
        GSLOG(7, "VariantManager::setValue: ?");
		Data d = propertyToData[property];
		d.value = val;
		propertyToData[property] = d;
		emit propertyChanged(property);
		emit valueChanged(property, val);
		return;
	}
	QtVariantPropertyManager::setValue(property, val);
}

void CVariantManager::initializeProperty(QtProperty *property)
{
	if (propertyType(property) == PATH_TYPE)
	{
		/// TO DO  ??
        // GSLOG(6, (char*) " PATH : ToDo ?");
		//Data d; d.value=".";
		//VariantManager *that = (VariantManager *)this;
		return;
	}

	if (propertyType(property) == HTML_TYPE)
	{
		/// TO DO  ??
		return;
	}

	if (propertyType(property) == JS_TYPE)
	{
		/// TODO ?
		return;
	}

	/*
	if (propertyType(property) == QVariant::PointF)
	{
		Data d;

		d.value = QPointF(0, 0);

		VariantManager *that = (VariantManager *)this;

		d.x = that->addProperty(QVariant::Double);
		d.x->setPropertyName(tr("Position X"));
		property->addSubProperty(d.x);
		xToProperty[d.x] = property;

		d.y = that->addProperty(QVariant::Double);
		d.y->setPropertyName(tr("Position Y"));
		property->addSubProperty(d.y);
		yToProperty[d.y] = property;

		propertyToData[property] = d;
	}
	*/
	QtVariantPropertyManager::initializeProperty(property);
}

void CVariantManager::uninitializeProperty(QtProperty *property)
{
	qDebug("VariantManager::uninitializeProperty");
	if (!property)
		return;
	if (propertyToData.contains(property))
	{
		Data d = propertyToData[property];
		//if (d.x)
		//	xToProperty.remove(d.x);
		//if (d.y)
		//	yToProperty.remove(d.y);
		propertyToData.remove(property);
	}
	QtVariantPropertyManager::uninitializeProperty(property);
}
