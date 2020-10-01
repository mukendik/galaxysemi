#include "reports_center_item.h"
#include <gqtl_log.h>

ReportsCenterItem::ReportsCenterItem(const QList<QVariant> &data, ReportsCenterItem *parent, QString fullpath, QString type)
{
    //GSLOG(SYSLOG_SEV_DEBUG, QString(" for '%1'").arg( fullpath).toLatin1().constData());
	parentItem = parent;
	itemData = data;
	m_fullpath=fullpath; m_type=type;
}

int ReportsCenterItem::removeAllChildsOfType(QString t)
{
    //GSLOG(SYSLOG_SEV_DEBUG, t.toLatin1().data());
	int n=0;
	QListIterator<ReportsCenterItem*> it(childItems);
	while (it.hasNext())
	{	ReportsCenterItem* rci=it.next();
		if (rci)
		{
			n=n+rci->removeAllChildsOfType(t);
			if (rci->m_type==t)
			{	childItems.removeOne(rci);
				n++;
				//delete rci;
			}
		}
		else
			GSLOG(SYSLOG_SEV_WARNING, "found a null ReportsCenterItem");
	}
	return n;
}

ReportsCenterItem* ReportsCenterItem::getChild(QString n)
{
	QListIterator<ReportsCenterItem*> it(childItems);
	while (it.hasNext())
	{	ReportsCenterItem* rci=it.next();
		if (rci)
            if ((!(rci->itemData).empty()) && (rci->itemData.first().toString()==n))
				return rci;
	}
	return NULL;
}

bool ReportsCenterItem::hasDirChild(QString dirname)
{
	QListIterator<ReportsCenterItem*> it(childItems);
	while (it.hasNext())
	{	ReportsCenterItem* rci=it.next();
		if (rci)
		{
            if ((!(rci->itemData).empty()) && (rci->itemData.first().toString()==dirname)) //if (rci->hasDirChild(dirname))
				return true;
		}
	}
	return false;
}
