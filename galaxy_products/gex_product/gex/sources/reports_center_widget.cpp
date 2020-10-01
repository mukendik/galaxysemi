//#include <QLibrary>
#include <QTreeView>
#include <QMutex>
#include <QMessageBox>
#include <QScriptEngine> // to evaluate fromdate, todate,...
#include <QXmlStreamReader>

#include "jasper_params_widget.h"
#include "reports_center_thread.h"
#include "reports_center_widget.h"
#include "reports_center_params_widget.h"
#include "engine.h"
#include "gex_shared.h"
#include "db_engine.h"
#include "jasper_threads.h"
#include "ui_reports_center_widget.h"
#include "settings_dialog.h"
//#include "libjasper.h"
#include <gqtl_log.h>
#include "browser_dialog.h"
#include "gex_version.h"	// for GR_VERSION...
#include "db_transactions.h"	// for GexDatabseEntry
#include "gex_database_entry.h"
#include "report_options.h"
#include "message.h"

extern void				WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *	pGexMainWindow;
extern CReportOptions	ReportOptions;
extern bool				FillComboBox(QComboBox * pCombo, const char * szTextTab[]);
extern const char*		gexTimePeriodChoices[];		// tables for the time period combobox

GexMainwindow* ReportsCenterWidget::s_pGexMainWindow=NULL;
ReportsCenterTreeModel *ReportsCenterWidget::s_model=NULL;
QWidget* ReportsCenterWidget::s_pCurrentParamsWidget=NULL; //ReportsCenterParamsDialog* ReportsCenterWidget::m_pCurrentParamsWidget=NULL;

QMap<QString, QWidget*> ReportsCenterWidget::s_JasperReportsParamsWidgets;
//QMap<QString, ReportsCenterParamsDialog*> ReportsCenterWidget::s_ReportsCenterParamsDialogs;
QMap<QString, CReportsCenterParamsWidget*> ReportsCenterWidget::s_ReportsCenterParamsWidgets;

ReportsCenterWidget* ReportsCenterWidget::GetInstance()
{
    if (s_pGexMainWindow)
        return s_pGexMainWindow->m_pReportsCenter;
    else
        return NULL;
}

ReportsCenterWidget::ReportsCenterWidget(GexMainwindow* gmw, QWidget *parent) : QWidget(parent),
    //s_pGexMainWindow(gmw),
    m_pCurrentDatabaseEntry(NULL),  m_ui(new Ui::ReportsCenterWidget)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, " ctor...");
    s_pGexMainWindow=gmw;
    s_pCurrentParamsWidget=NULL;
    m_actionRC=NULL;
    m_rc_thread=NULL;	//new CReportsCenterThread(this);

    m_ui->setupUi(this);

    if (s_model!=NULL)
        GSLOG(SYSLOG_SEV_NOTICE, "warning : s_model not null. Possible memory leaks !");
    s_model = new ReportsCenterTreeModel(this);

    if (m_ui->treeView)
        m_ui->treeView->setModel(s_model);
    s_model->m_pTreeView=m_ui->treeView;
    m_ui->treeView->setUniformRowHeights(true);
    m_ui->treeView->setFont(QFont("Times", 11, QFont::Normal));
    m_ui->treeView->expandAll();
    //m_ui->treeView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_ui->treeView->resizeColumnToContents(0);
    m_ui->treeView->setMouseTracking(true);
    m_ui->progressBar->hide();

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // Fill output combobox
    m_ui->m_output_comboBox->addItem(QIcon(QString::fromUtf8(":/gex/icons/explorer.png")),		"Web Report",
                                     GEX_SETTINGS_OUTPUT_HTML);
    m_ui->m_output_comboBox->addItem(QIcon(QString::fromUtf8(":/gex/icons/word.png")),			"Word Document",
                                     GEX_SETTINGS_OUTPUT_WORD);
    m_ui->m_output_comboBox->addItem(QIcon(QString::fromUtf8(":/gex/icons/powerpoint.png")),	"PowerPoint slides",
                                     GEX_SETTINGS_OUTPUT_PPT);
    m_ui->m_output_comboBox->addItem(QIcon(QString::fromUtf8(":/gex/icons/pdf.png")),			"PDF Document",
                                     GEX_SETTINGS_OUTPUT_PDF);
    m_ui->m_output_comboBox->addItem(QIcon(QString::fromUtf8(":/gex/icons/csv_spreadsheet.png")),			"Spreadsheet CSV",
                                     GEX_SETTINGS_OUTPUT_CSV);

    QObject::connect( m_ui->m_output_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOutputFormatChanged(int)));
    QObject::connect( m_ui->database_comboBox, SIGNAL(currentIndexChanged(QString)),
                      this, SLOT(OnDatabaseComboBoxChanged(QString)));
    QObject::connect( m_ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(TreeViewClicked(QModelIndex)) );
    QObject::connect( m_ui->treeView, SIGNAL(entered(QModelIndex)), this, SLOT(TreeViewEntered(QModelIndex)) );
    QObject::connect( m_ui->treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(OnExpanded(QModelIndex)) );
    // Just to be sure that all RC widgets will deleted when app quits
    if (qApp)
        QObject::connect( qApp, SIGNAL(aboutToQuit()), this, SLOT(DeleteAllParamsWidgets()) );

    //m_rc_thread->start();
}

ReportsCenterWidget::~ReportsCenterWidget()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, " start deleting...");
    /*
    if (m_rc_thread)
        m_rc_thread->bStopRequested=true;
    */

    //JasperListDirThread::StopAndDeleteListDirThread();
    //JasperRunReportThread::StopAndDeleteThread();

    DeleteAllParamsWidgets();

    delete m_ui;

    if (s_model)
        delete s_model;
    s_model=0;

    if (pclJasperLibrary)
    {
        if (!pclJasperLibrary->unload())
        {	GSLOG(SYSLOG_SEV_ERROR, " cant unload libjasper !");
        }
        else
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, " libjasper unloaded. ");
            delete  pclJasperLibrary;
            pclJasperLibrary=NULL;
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, " ok");
}

//bool ReportsCenterWidget::AddIconInToolbar()
//{
//    if (!s_pGexMainWindow)
//        return false;

//    GSLOG(SYSLOG_SEV_INFORMATIONAL, "adding ReportCenter toolbar icon...");

//    //pGexMainWindow->toolBarButton->addAction("RC");
//    if (m_actionRC==NULL)
//        m_actionRC = new QAction(s_pGexMainWindow);
//    m_actionRC->setObjectName(QString::fromUtf8("actionRC"));
//    m_actionRC->setToolTip("show Reports Center");
//    QIcon icon;
//    //icon.addFile("./images/icon-report.png", QSize(), QIcon::Normal, QIcon::Off);
//    icon.addFile(QString::fromUtf8(":/gex/icons/icon-report.jpg"), QSize(), QIcon::Normal, QIcon::Off);
//    m_actionRC->setIcon(icon);
//        //m_pGexMainWindow->toolBarButton->addAction(QIcon("./images/icon-report.jpg"), "ReportCenter");
//    s_pGexMainWindow->toolBarButton->addAction(m_actionRC);
//    connect(m_actionRC,		SIGNAL(activated()), s_pGexMainWindow, SLOT(ShowReportsCenter()));

//    return true;
//}

QString ReportsCenterWidget::ScanUsualFolders()
{
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("ReportsCenter : scanning usual folders..."));
    QCoreApplication::processEvents();

    QString r=ScanFolder(GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString()+QDir::separator()+ "galaxy_reports",
                              s_model->m_rootItem, true);

    QString f(GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString() +QDir::separator()+ "galaxy_reports");
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("in %1 : %2").
          arg(f).arg(r).toLatin1().constData());

    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("ReportsCenter : %1").arg(r));
    QCoreApplication::processEvents();

    if (!r.startsWith("error"))
        r.clear();

    QString c=GS::Gex::Engine::GetInstance().Get("UserFolder").toString()
            + QDir::separator()+ TEMPLATES_FOLDER + QDir::separator() + "mytemplates";
    r+= ScanFolder(c, s_model->m_rootItem, true );

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("in %1 : %2").
          arg(c).arg(r).toLatin1().constData());

    return r;
}

QString ReportsCenterWidget::ScanFolder(QString f, ReportsCenterItem *Item, bool recursive)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1").arg( f).toLatin1().constData());
    if (!Item)
        return "error : Item NULL !";

    QDir d(f);
    if (!d.exists())
        return QString("ok : but folder '%1' does not exist !").arg(f);
        //d.mkdir(strUserFolder+QDir::separator()+ TEMPLATES_FOLDER );

    //QStringList fl; fl<<"*.grxml"<<"*.GRXML";
    //d.setNameFilters(fl);
    d.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks); // QDir::Hidden |
    //dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = d.entryInfoList();
    int iGRXML_ok=0;
    for (int i=0; i<list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isFile())
        {
            if (fileInfo.fileName().endsWith(".grxml", Qt::CaseInsensitive))
                if (LoadGRXML(fileInfo.absoluteFilePath(), Item))
                    iGRXML_ok++;
        }
        else if ( (fileInfo.isDir()) && (fileInfo.fileName()!="..") && (fileInfo.fileName()!=".") )
        {
            ReportsCenterItem* it=NULL;
            if (!Item->hasDirChild(fileInfo.fileName()))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tadding folder %1").arg( fileInfo.fileName()).toLatin1().constData());
                QList<QVariant> data;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" folder %1").arg( fileInfo.filePath()).toLatin1().constData());
                QImage i(fileInfo.filePath()+"/icon.png");
                if (!i.isNull())
                    i=i.scaled(17,17, Qt::IgnoreAspectRatio);
                data << fileInfo.fileName() << " " << i;
                //data << QPixmap("help/images/open.png") << fileInfo.fileName() << " ";
                //data << QPixmap(QString::fromUtf8(":/gex/icons/script_new.png")) << fileInfo.fileName() << " ";

                it=new ReportsCenterItem( data, Item, Item->m_fullpath+"/"+fileInfo.fileName(), "folder" );
                Item->appendChild(it);
                //s_model->setData( QVariant(QPixmap("help/images/open.png")), Qt::DecorationRole);
                //QModelIndexList il=s_model->match( s_model->m_rootItem, );
                //s_model->

            }
            else
                it=Item->getChild(fileInfo.fileName());
            // Recursive ?
            if (recursive)
            {
                QString r=ScanFolder(f+QDir::separator()+fileInfo.fileName(), it);
                if (r.startsWith("error"))
                    GSLOG(SYSLOG_SEV_NOTICE, QString(" %1").arg( r).toLatin1().constData() );
            }
        }
    }

    if (iGRXML_ok>0)
        GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString("ReportsCenter : %1 grxml loaded").arg(iGRXML_ok));
    QCoreApplication::processEvents();

    s_model->UpdateGUI();
    m_ui->treeView->expandAll();
    m_ui->treeView->resizeColumnToContents(0);
    m_ui->treeView->collapseAll();

    UpdateGUI();

    return (iGRXML_ok>0?QString("ok (%1 GRXML loaded)").arg(iGRXML_ok):QString("ok"));
}

void ReportsCenterWidget::ReplaceOutputCombobox()
{
    // Hide/Show edit fields, depending of the Report output format.
    // small heck to ensure old behaviour
    QString strOutputFormatOption	= (GS::Gex::Engine::GetInstance().GetOptionsHandler().GetOptionsMap().GetOption("output", "format")).toString();
    int		nIndex					= -1;

    if (strOutputFormatOption==QString("HTML"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_HTML);	// HTML output format
    else if (strOutputFormatOption==QString("DOC"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_WORD);	// DOC output format
    else if (strOutputFormatOption==QString("PPT"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_PPT);	// HTML output format
    else if (strOutputFormatOption==QString("PDF"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_PDF);	// HTML output format
    else if (strOutputFormatOption==QString("CSV"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_CSV);	// CSV output format
    else if (strOutputFormatOption==QString("INTERACTIVE"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_HTML);	// HTML output format
    else if (strOutputFormatOption==QString("ODT"))
        nIndex = m_ui->m_output_comboBox->findData(GEX_SETTINGS_OUTPUT_ODT);

    if (nIndex != -1)
        m_ui->m_output_comboBox->setCurrentIndex(nIndex);
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, "WARNING : Undefined 'output' 'format' option");
        GEX_ASSERT(false);
    }
}


QComboBox* ReportsCenterWidget::GetDatabaseComboBox()
{	return m_ui->database_comboBox;
}

GexDatabaseEntry* ReportsCenterWidget::GetCurrentDatabaseEntry()
{
    QString		strDatabaseLogicalName = this->m_ui->database_comboBox->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();
    this->m_pCurrentDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName);

    if(!m_pCurrentDatabaseEntry)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("ReportsCenterWidget: OnDatabaseComboBoxChanged: "
                      "error : Database Entry for '%1' NULL !").
              arg(strDatabaseLogicalName).toLatin1().constData());
        // Could be a local DB "not connected".
        return NULL;
    }

    if (!m_pCurrentDatabaseEntry->IsExternal())
        return NULL;

    return m_pCurrentDatabaseEntry;
}

void	ReportsCenterWidget::OnOutputFormatChanged(int)
{
    GS::Gex::OptionsHandler optionsHandler(GS::Gex::Engine::GetInstance().GetOptionsHandler());

    // Have the 'Options' page reflect the new report format selected from this 'Reports Center' page.
    if (!optionsHandler.SetOption("output", "format", GetCurrentOutputFormat()))
        GEX_ASSERT(false);

    // Update default options map
    GS::Gex::Engine::GetInstance().SetOptionsMap(optionsHandler.GetOptionsMap());
    if (!pGexMainWindow->RefreshOptionsCenter())
        GEX_ASSERT(false);
}

void	ReportsCenterWidget::OnDatabaseComboBoxChanged(QString s)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("ReportsCenterWidget::OnDatabaseComboBoxChanged: %1").arg( s).toLatin1().constData());
    QString		strDatabaseLogicalName = this->m_ui->database_comboBox->currentText();
    if(strDatabaseLogicalName.startsWith("[Local]") || strDatabaseLogicalName.startsWith("[Server]"))
        strDatabaseLogicalName = strDatabaseLogicalName.section("]",1).trimmed();

    this->m_pCurrentDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDatabaseLogicalName);

    if(!m_pCurrentDatabaseEntry )
    {
        GSLOG(SYSLOG_SEV_ERROR,
          QString("ReportsCenterWidget::OnDatabaseComboBoxChanged: error : Database Entry for %1 NULL")
            .arg(strDatabaseLogicalName).toLatin1().constData() );
        // Could be a local DB "not connected".
        return;
    }

    QGuiApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    if(m_pCurrentDatabaseEntry->IsExternal())
    {
        GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterWidget::OnDatabaseComboBoxChanged: external db found.");
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE, "ReportsCenterWidget::OnDatabaseComboBoxChanged: non external db !");
    }

    // to do : recalculate all widgets for the filters to be updated !
    DeleteAllParamsWidgets();
    ReportsCenterWidget::s_pCurrentParamsWidget=NULL;

    s_model->removeAllChildOfType("GalaxyReport");
    if (m_ui)
        if (m_ui->treeView)
        {
            m_ui->treeView->resizeColumnToContents(0);
            m_ui->treeView->collapseAll();
        }

    QString r=ScanUsualFolders();
    if (r.startsWith("error"))
    {
        GS::Gex::Message::critical("Error !", r);
    }
    else
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("%1 templates successfully added.").
              arg(s_ReportsCenterParamsWidgets.size()).toLatin1().constData());

    QGuiApplication::restoreOverrideCursor();
}

void ReportsCenterWidget::changeEvent(QEvent *e)
{
    GSLOG(SYSLOG_SEV_DEBUG, "ReportsCenterWidget::changeEvent");
    QWidget::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange: m_ui->retranslateUi(this); break;
        default: break;
    }
}

// slots
void ReportsCenterWidget::SetStatusLabel(QString s)
{
    m_ui->label->setText(s);
}

void ReportsCenterWidget::PopMessageBox(const QString s, int severity)
{
    QMessageBox mb;
    mb.setIcon((QMessageBox::Icon)severity);
    mb.setText(s);
    mb.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    mb.exec();

    if ((QMessageBox::Icon)severity==QMessageBox::Critical)
        pGexMainWindow->ShowWizardDialog(GEX_BROWSER);
}

void ReportsCenterWidget::ReportGenerated(const QString report_uri, const QString output)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("ReportsCenterWidget::ReportGenerated %1").
          arg(report_uri).toLatin1().constData());

    QString o(output);

    if (output.endsWith("html", Qt::CaseInsensitive))
    {
        QString lUrl = QUrl::fromLocalFile(output).toString();
        pGexMainWindow->LoadUrl(lUrl);
    }
    else
    {
        pGexMainWindow->Wizard_OpenReportFile( o);
    }

}

QString ReportsCenterWidget::GetCurrentOutputFormat()
{
    // Have the 'Options' page reflect the new report format selected from this 'Reports Center' page.
    QVariant	varItemData = m_ui->m_output_comboBox->itemData(m_ui->m_output_comboBox->currentIndex());
    QString		strOutputFormat;

    switch(varItemData.toInt())
    {
        case GEX_SETTINGS_OUTPUT_HTML	:	strOutputFormat = "HTML";
                                            break;
        case GEX_SETTINGS_OUTPUT_PDF	:	strOutputFormat = "PDF";
                                            break;
        case GEX_SETTINGS_OUTPUT_PPT	:	strOutputFormat = "PPT";
                                            break;
        case GEX_SETTINGS_OUTPUT_WORD	:	strOutputFormat = "DOC";
                                            break;
        case GEX_SETTINGS_OUTPUT_CSV	:	strOutputFormat = "CSV";
                                            break;
        case GEX_SETTINGS_OUTPUT_ODT	:	strOutputFormat = "ODT";
                                            break;
        default							:	GSLOG(SYSLOG_SEV_WARNING, "Unknown output format");
                                            GEX_ASSERT(false);
                                            break;
    }

    return strOutputFormat;
}

void ReportsCenterWidget::UpdateGUI()
{
    //if (s_model)
    //	s_model->UpdateGUI();
    if (m_ui)
        if (m_ui->treeView)
            m_ui->treeView->resizeColumnToContents(0);
}

bool ReportsCenterWidget::ShowLatestParamsWindow()
{
    //if (!s_pGexMainWindow)

    //if (!s_pGexMainWindow->pReportsCenter)
    //	return false;

    //if (!s_pCurrentParamsWidget)
    //	return false;

    if (s_pCurrentParamsWidget)
    {
        s_pCurrentParamsWidget->show();
        return true;
    }

    return false;
}

bool ReportsCenterWidget::DeleteAllParamsWidgets()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" %1 widgets to delete...").
          arg(s_ReportsCenterParamsWidgets.size()).toLatin1().constData());

    foreach (CReportsCenterParamsWidget* pw, ReportsCenterWidget::s_ReportsCenterParamsWidgets)
    {
        if (pw)
        {
            pw->hide();
            delete pw;
            pw=NULL;
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, " error : found a NULL ReportsCenterParamsWidgets in cache !");
        }
    }
    ReportsCenterWidget::s_ReportsCenterParamsWidgets.clear();

    ReportsCenterWidget::s_pCurrentParamsWidget=NULL;
    return true;
}


void ReportsCenterWidget::ShowPage(void)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "ReportsCenterWidget::ShowPage()");
    //m_pTreeView->resizeColumnToContents(0);
    show();
    QCoreApplication::processEvents();
}

void ReportsCenterWidget::OnExpanded(QModelIndex /*mi*/)
{
    m_ui->treeView->resizeColumnToContents(0);
}

void ReportsCenterWidget::TreeViewClicked(const QModelIndex & index)
{
    ReportsCenterItem *item=s_model->GetItem(index);
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( item?QString(item->m_fullpath+" "+item->m_type):"?").toLatin1().constData());

    if (!item)
    {
        GSLOG(SYSLOG_SEV_ERROR,"\terror : Item NULL !");
        m_ui->treeView->resizeColumnToContents(0); //m_pTreeView->resizeColumnToContents(0);
        return;
    }

    HideAllParamsWidget();

    if (item->m_type=="reportUnit")
    {
        GSLOG(SYSLOG_SEV_ERROR, "Jasper support removed");

        /*
        QDir d(strUserFolder+QDir::separator()+ REPORTS_FOLDER);
        if (!d.exists())
            d.mkdir(strUserFolder+QDir::separator()+ REPORTS_FOLDER );

        retrieve_params_for_report_f rpfr_f=(retrieve_params_for_report_f)
            ReportsCenterWidget::pclJasperLibrary->resolve("jasper_retrieve_params_for_report");
        if (!rpfr_f)
        {
            WriteDebugMessageFile("\terror : unable to resolve jasper_retrieve_params_for_report function !");
            return;
        }

        char o[MAX_NUM_RESOURCES][6][MAX_STRING_SIZE];
        int nIC=rpfr_f(item->m_fullpath.toLatin1().data(), (char***)o, d.absolutePath().toLatin1().data());
        GSLOG(SYSLOG_SEV_DEBUG, QString("%1 has %2 params...").arg( item->m_fullpath).toLatin1().constData(), nIC));.arg(        GSLOG(SYSLOG_SEV_DEBUG, "%1 has %2 params...").arg( item->m_fullpath).toLatin1().constData().arg( nIC);
        if (nIC<0)
        {
            WriteDebugMessageFile(QString(
                    "\terror : unable to retrieve %1 params !").arg(item->m_fullpath));
            return;
        }

        if (nIC==0)	// no params !
        {
            QVector< QPair <QString, QString> > params;

            JasperRunReportThread::RequestReport(item->m_fullpath,
                strUserFolder+QDir::separator()+REPORTS_FOLDER+QDir::separator()+item->m_fullpath.section('/',-1)+".pdf",
                params); //for html: "o.html"
            return;
        }
        */
        /*
        ParamsWidget* pw=ParamsWidget::GetParamsWidgetForReport(
            item->m_fullpath, index.data(0).toString() ); // m_pTreeView->currentIndex().data(0).toString() );
        if (!pw)
            pw=ParamsWidget::CreateParamsWidgetForReport(
                    item->m_fullpath, index.data(0).toString(), o, nIC); //m_pTreeView->currentIndex().data(0).toString(), o, nIC);
        pw->show();
        */
    }
    else if(item->m_type=="GalaxyReport")
    {
        /*
        if (ReportsCenterWidget::s_ReportsCenterParamsDialogs.find(item->m_fullpath)
                !=ReportsCenterWidget::s_ReportsCenterParamsDialogs.end())
        {
            ReportsCenterParamsDialog* pd=ReportsCenterWidget::s_ReportsCenterParamsDialogs.find(item->m_fullpath).value();
            if (pd)
            {
                pd->show();
                ReportsCenterWidget::s_pCurrentParamsWidget=pd;
            }
            else
                GSLOG(SYSLOG_SEV_ERROR, "\terror : cant find corresponding ReportsCenterParamsDialog...");
        }
        else
        */
        if (ReportsCenterWidget::s_ReportsCenterParamsWidgets.find(item->m_fullpath)
                != ReportsCenterWidget::s_ReportsCenterParamsWidgets.end()
                )
            {
                CReportsCenterParamsWidget* pw=ReportsCenterWidget::s_ReportsCenterParamsWidgets.find(item->m_fullpath).value();
                if (pw)
                {
                    if (pw->IsEnabled())
                        pw->show();
                    else
                    {
                        GSLOG(SYSLOG_SEV_WARNING, "template disabled");
                        GS::Gex::Message::information(
                            "", "Your license does not allow this feature.\n"
                            "Contact " +
                            QString(GEX_EMAIL_SALES)+" for more information.");
                        return;
                    }
                }
                else
                {	GSLOG(SYSLOG_SEV_ERROR, "cant find a ReportsCenterParams for this entry !");
                    return;
                }

                ReportsCenterWidget::s_pCurrentParamsWidget=pw;
            }
        else
            GSLOG(SYSLOG_SEV_ERROR, "\terror: cant find a ReportsCenterParams for this entry !");
    }
    else
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" item of type %1 cliked").arg( item->m_type).toLatin1().constData());

    m_ui->treeView->resizeColumnToContents(0);

}

void ReportsCenterWidget::TreeViewEntered(const QModelIndex & index)
{
    ReportsCenterItem *item=s_model->GetItem(index);
    if (!item)
    {
        m_ui->treeView->setCursor(Qt::ArrowCursor);
        return;
    }
    if (item->m_type!="folder")
        m_ui->treeView->setCursor(Qt::PointingHandCursor);
    else
        m_ui->treeView->setCursor(Qt::ArrowCursor);

}



bool ReportsCenterWidget::InsertFieldsIntoComboBox(QComboBox *cb,
                                                         const QString &testingstage,
                                                         QString BinType, 	// BinType can be H, S or N (or even *)
                                                         QString Custom,	// Custom can be Y, N or *
                                                         QString TimeType,  // TimeType can be Y, N or *
                                                         QString ConsolidatedType // Y, N or *
                                                         )
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" TS=%1 Consolidated=%2").
          arg(testingstage).arg(ConsolidatedType).toLatin1().constData());
    if (!cb || testingstage=="")
        return false;
    GexDatabaseEntry* dbe=GetCurrentDatabaseEntry();
    if (!dbe)
    {
        GSLOG(SYSLOG_SEV_ERROR , "error : DatabaseEntry NULL !");
        //for (int i=0; i<m_filters.size(); i++)
        //	FillComboBox(cb,	gexLabelFilterChoices);	// add default fields ???
        return false;
    }

    if (BinType=="") BinType="*";
    if (Custom=="") Custom="*";
    if (TimeType=="") TimeType="*";
    if (ConsolidatedType=="")  ConsolidatedType="Y";

    QStringList strlFilters;
    //dbe->m_pExternalDatabase->GetLabelFilterChoices( QString(testingstage), strlFilters);
    dbe->m_pExternalDatabase->GetRdbFieldsList(QString(testingstage), strlFilters, "Y", BinType,
                                               Custom, TimeType, ConsolidatedType,"N");

    if (strlFilters.empty())
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("warning : no fields returned for "
                      "TS='%1' Bin='%2' Time='%3' !").arg(testingstage).
              arg(BinType).arg(TimeType).toLatin1().constData());
        //FillComboBox(cb,	gexLabelFilterChoices);	// Should nt happen, insert default static fields ???
        //m_filters[0].m_filter_cb->setCurrentItem(GEX_QUERY_FILTER_PRODUCT);
        return false;
    }
    else
    {
        cb->addItems(strlFilters);
    }
    return true;
}
