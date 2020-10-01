#include "reports_center_widget.h"
#include "reports_center_item.h"

#include "jasper_threads.h"

#include "ui_reports_center_widget.h"
#include <QLibrary>
#include <QTreeView>
#include <QMutex>
#include <QMessageBox>

#include "libjasper.h"
#include "browser_dialog.h"


extern void		WriteDebugMessageFile(const QString & strMessage);

// static
JasperListDirThread* JasperListDirThread::s_singleton=NULL;
QMap<QString, ReportsCenterItem*> JasperListDirThread::s_folders_to_scan;
bool JasperListDirThread::s_stop_request=false;
extern GexMainwindow *	pGexMainWindow;

// static
bool JasperListDirThread::LaunchListDirThread(QString dir, ReportsCenterItem *r)
{
    WriteDebugMessageFile(QString("JasperListDirThread::LaunchListDirThread %1").arg(dir));

    if (!s_singleton)
		s_singleton=new JasperListDirThread();

    if (!s_singleton->isRunning())
    {
        if (s_folders_to_scan.find(dir)==s_folders_to_scan.end())
            s_folders_to_scan[dir]=r;

		s_singleton->start(QThread::LowestPriority); //
		qDebug("JasperListDirThread::LaunchListDirThread: thread id = %d", (int)s_singleton->currentThreadId());
    }
	else
		qDebug("JasperListDirThread::LaunchListDirThread: already running.");

    JasperListDirThread::s_stop_request=false;

    return true;
}

void JasperListDirThread::run()
{
    WriteDebugMessageFile("JasperListDirThread::run");

	if (!ReportsCenterWidget::pclJasperLibrary)
    {
		WriteDebugMessageFile("JasperListDirThread::run: error : JasperLib NULL !");
		SetStatusLabel("JasperListDirThread::run: libjasper null !");
		#ifndef QT_DEBUG
			PopMessageBox("JasperListDirThread::run: libjasper null !\n Please be sure the libjasper.dll/so is available.",
						  (int)QMessageBox::Critical);
		#endif
		return;
    }

	list_directory_f ldf=(list_directory_f)ReportsCenterWidget::pclJasperLibrary->resolve("jasper_list_directory");
    if (!ldf)
    {
		WriteDebugMessageFile("JasperListDirThread::run:  cant resolve list_dir function !");
		SetStatusLabel("Cant resolve list_dir function !");
		#ifndef QT_DEBUG
			PopMessageBox("cant resolve list_dir function in libjasper !\n Please be sure the libjasper.dll/so is the latest.",
						  (int)QMessageBox::Critical);
		#endif
		return;
    }

	char o[MAX_NUM_RESOURCES][4][MAX_STRING_SIZE]; // buffer that will be send to the libjasper for writing

    do
    {
        if (s_folders_to_scan.size()==0)
        {
			this->msleep(1000);
			continue;
        }

		QMap <QString, ReportsCenterItem*>::const_iterator	it=s_folders_to_scan.begin();
        QString dir=it.key();
		ReportsCenterItem* rootItem=it.value();

        int nResources=-1;

        if (!ldf(dir.toLatin1().data(), (char***)o, nResources))
        {
            qDebug("JasperListDirThread::run: jasper list dir failed !");
			//JasperListDirThread::s_stop_request=true;
			SetStatusLabel("JasperListDirThread::run: jasper list dir failed !");
			#ifndef QT_DEBUG
				PopMessageBox("Jasper list directory operation failed !\n Please be sure the jasper server is running.",
						  (int)QMessageBox::Critical);
			#endif
			return;
        }

		if (nResources==-1)
		{
			qDebug("JasperListDirThread::run: jasper list dir returned -1!");
			//JasperListDirThread::s_stop_request=true;
			SetStatusLabel("Jasper list directory failed !");
			#ifndef QT_DEBUG
			 PopMessageBox("Jasper list directory operation failed !\n Please be sure the jasper server is running.",
						  (int)QMessageBox::Critical);
			#endif
			return;
		}

		WriteDebugMessageFile(QString("JasperListDirThread::run: jasper list dir ok : %1 resources in %2")
			      .arg(nResources).arg(dir) );

        QMutex m; m.lock();

        for (int i=0; i<nResources; i++)
        {
			//qDebug( QString(" n:%1 t:%2 l:%3 d:%4").arg(o[i][0]).arg(o[i][1]).arg(o[i][2]).arg(o[i][3]) );
			QString fp=rootItem->m_fullpath=="/"?("/"):(rootItem->m_fullpath+"/");
			fp.append(o[i][0]);
			QList<QVariant> data;
			data << o[i][2] << o[i][3] << fp ; // label, description, fullpath
				//data << o[i][2] << fp << o[i][3]; // label, fullpath, desc

			ReportsCenterItem* item=new ReportsCenterItem( data,it.value(), fp, o[i][1] );
			if ( (o[i][0]) && strcmp(o[i][1],"folder")==0)
			{
				if (s_folders_to_scan.find(o[i][0])==s_folders_to_scan.end())
					s_folders_to_scan[QString("%1%2").arg(dir=="/"?dir:dir+"/").arg(o[i][0])]=item;
			}
			it.value()->appendChild( item );
        }

		//qDebug("JasperListDirThread::run ok (%d res added, %d childs)", nResources, it.value()->childCount());

        s_folders_to_scan.remove(it.key());

		emit UpdateGUI();
		m.unlock();

		this->msleep(100);

        //qDebug(" Column 0 Width = %d", JasperCenterWidget::s_treeview->columnWidth(0));
    }
    while(!JasperListDirThread::s_stop_request);

    WriteDebugMessageFile("JasperListDirThread::run end.");
}

bool JasperListDirThread::StopAndDeleteListDirThread()
{
    JasperListDirThread::s_stop_request=true;
    if (!s_singleton)
	 return true;
    while (s_singleton->isRunning())
	 JasperListDirThread::s_stop_request=true;
    delete s_singleton;
    s_singleton=NULL;
    WriteDebugMessageFile("JasperListDirThread::StopAndDeleteListDirThread ok.");
    return true;
}
