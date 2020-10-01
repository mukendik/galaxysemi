#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <QDate>
#include <QEvent>
#include <QDir>
#include <QThread>
#include <QMutex>
#include <QList>
#include <QMap>
#include <QDomDocument>
#include <QDebug>
#include <QCoreApplication>
#include <QScriptEngine>
#include <QObject>

#include "gqtl_log.h"
#include "coutput.h"
// #include "httpdaemon.h"

#ifdef _WIN32
    #include <windows.h>
#endif

//#define QUOTE_ME_(x) #x
//#define JOIN_STR(a,b) a:b

static bool s_bConfFileParsed=false;
static bool s_bThreaded=true;
static int s_iThreadPriority=3;

QMutex* s_pMutex=0;

static QMutex lpMutex;
//CGexLogThread*	s_pThread=NULL;

extern QString _log_to_rtf(int sev, const char* file, const char* m, const char* func, QString path);

extern QString _log_to_csv(int sev, const char* file, const char* m, const char* /*func*/, QString /*path*/);

extern QString _log_to_txt(int sev, const char* file, const char* m, const char* func, QString p);

extern QString _log_to_xml(int sev, const char* func, const char* m, QString path);

extern QString _log_to_sql(int sev, const char* file, const char* func, const char* m);

extern QString _log_to_console(int sev, const char* file, const char* m, const char* func, QString /*path*/);

extern QString _log_to_syslog(int sev, const char* file, const char* m, const char* func, QMap< QString, QString>);

QFile s_gexloglogfile(QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp"+QDir::separator()+"gslog.txt");

// cant be static bacause Qt will refuse to load the lib if so
QMap<QString, int> s_LogLevels;

bool ReplaceWithUsualVariables(QString &s, QMap< QString, QString> attsMap)
{
    s.replace("$HOME", QDir::homePath());
    s.replace("$ISODATE", QDate::currentDate().toString(Qt::ISODate) );
    s.replace("$DATE", QDate::currentDate().toString(Qt::TextDate) );
    s.replace("$PID", QString::number(QCoreApplication::applicationPid()));
    s.replace("$MODULE", attsMap["module"]);
    s.replace("$FILE", attsMap["file"]);
    //s.replace("$FUNCTION", attsMap["function"]);

    // GSP
    QString lGSP(getenv("GEX_SERVER_PROFILE"));
    if (lGSP.isEmpty())
        lGSP=QDir::homePath();
    s.replace("$GEX_SERVER_PROFILE", lGSP);

    // GTM
    QString lGTMSP(getenv("GTM_SERVER_PROFILE"));
    if (lGTMSP.isEmpty())
        lGTMSP=QDir::homePath();
    s.replace("$GTM_SERVER_PROFILE", lGTMSP);

    // YM
    QString lYMSP(getenv("YM_SERVER_PROFILE"));
    if (lYMSP.isEmpty())
      lYMSP=QDir::homePath();
    s.replace("$YM_SERVER_PROFILE", lYMSP);

    // PM
    QString lPMSP(getenv("PM_SERVER_PROFILE"));
    if (lPMSP.isEmpty())
      lPMSP=QDir::homePath();
    s.replace("$PM_SERVER_PROFILE", lPMSP);

    // Client Profile
    QString lGCP(getenv("GEX_CLIENT_PROFILE"));
    if (lGCP.isEmpty())
      lGCP=QDir::homePath();
    s.replace("$GEX_CLIENT_PROFILE", lGCP);

    return true;
}

QString GetApplicationDirectory(QString& strDirectory)
{

#ifdef _WIN32
    // Windows: use SDK functions to get application path
    char szApplicationDirectory[1024];

    // Get application full name
    if(GetModuleFileNameA(NULL, szApplicationDirectory, 1024) == 0)
        return "error : GetModuleFileNameA failed";

    // Retrieve Path information
    strDirectory = szApplicationDirectory;
    QFileInfo clFileInfo(strDirectory);
    strDirectory = clFileInfo.absolutePath();
    //NormalizePath(strDirectory);
    return "ok";
#endif

#if defined __unix__ || __MACH__
    // unix: get GEX_PATH environment variable, path to where the STDF Examinator is installed
    char *ptChar;

    ptChar = getenv("GEX_PATH");
    if(ptChar == NULL)
        return "error ";
    strDirectory = ptChar;
    //NormalizePath(strDirectory);
    return "ok";
#endif
    return "error";
}

QString open_conf_file(QString f, QMap< QString, QString> attsMap)
{
    QFile file(f);
    if (!file.open(QIODevice::ReadOnly))
        return QString("error : cant open file %1 !").arg(f);

    s_gexloglogfile.write( QString("%1 : open_conf_file : %2 opened...\n")
            .arg(QTime::currentTime().toString(Qt::ISODate))
            .arg(f)
            .toLatin1().data() );

    QDomDocument doc("xml");
    QString errorMsg; int errorLine=0, errorColumn=0;

    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        QString error=QString("error : file %1 is not xml compliant (line %2 col %3) : %4 !")
            .arg(f).arg(errorLine).arg(errorColumn).arg(errorMsg);
        file.close();
        return error;
    }

    file.close();

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName()!="gslog")
        return QString("error : the first tag of %1 is not 'gslog'").arg(f);

    if (docElem.attribute("threaded")=="false")
        s_bThreaded=false;
    else
        s_bThreaded=true;

    // does not work on Windows...
    if (s_bThreaded && docElem.hasAttribute("thread_priority"))
    {
        bool ok=false;
        int tp=docElem.attribute("thread_priority").toInt(&ok);
        if (ok)
            s_iThreadPriority=tp;
    }

    QDomNode node= docElem.firstChildElement("log");
    QDomNamedNodeMap atts;
    QDomElement elmt;
    while (!node.isNull())
    {
        //COutput *o=0;
        if (!node.isElement())
            goto nextnode;
        elmt = node.toElement();
        if (elmt.isNull())
            goto nextnode;
        atts=elmt.attributes();
        if (atts.size()==0)
            goto nextnode;

        for (int i=0; i<atts.size(); i++)
        {
            QDomNode n=atts.item(i);
            attsMap.insert( n.nodeName(), n.nodeValue());
        }

        /*
        if (l.m_atts["type"]=="httpdaemon")
        {
            l.m_output=new HttpDaemon(8080);
            if (! ((HttpDaemon*)l.m_output)->isListening())
            {
                qDebug(" Failed to bind HttpDaemon to port %d", ((HttpDaemon*)l.m_output)->serverPort() );
                delete l.m_output;
                goto nextnode;
            }
        }
        */

        if (attsMap["type"]=="console")
            /*o=*/new CConsoleOutput(attsMap);
        else if (attsMap["type"]=="txt")
            /*o=*/new CTxtOutput(attsMap);
        else if (attsMap["type"]=="rtf")
            /*o=*/new CRtfOutput(attsMap);
        else if (attsMap["type"]=="csv")
            /*o=*/new CCsvOutput(attsMap);
        else if (attsMap["type"]=="xml")
            /*o=*/new CXmlOutput(attsMap);
        else if (attsMap["type"]=="syslog")
            /*o=*/new CSyslogOutput(attsMap);
        else if (attsMap["type"]=="sql")
            /*o=*/new CSqlOutput(attsMap);
            #ifdef GS_DEBUG
        else
            printf("Warning : unknown log type '%s'\n", attsMap["type"].toLatin1().data());
          #endif

        nextnode:
            node=node.nextSibling();
    }

    return QString("ok : %1 successfully read : %2 logs found").arg(f).arg(COutput::s_ListCOutput.size() );
}

void FreeLogs()
{
    #ifdef GS_DEBUG
        qDebug("FreeLogs()");
    #endif

    //if (s_pThread)
    //	s_pThread->exit();

    /*
    SLog l;
    foreach(l, s_logs)
    {
        if (l.m_output)
            delete l.m_output;

    }
    s_logs.clear();
    */
    QList<COutput*>::iterator outputIter;
    lpMutex.lock();
    for (outputIter = COutput::s_ListCOutput.begin();
         outputIter != COutput::s_ListCOutput.end(); ++outputIter)
    {
        delete *outputIter;
    }
    COutput::s_ListCOutput.clear();
    lpMutex.unlock();
}

void myMsgHandler(QtMsgType mt, const char* m);

QString getEnvString(const QString& name)
{
    char* ev = getenv(name.toLatin1().data());
    if (ev != NULL)
    {
        return QString(ev);
    }
    else
    {
        return QString();
    }
}
#ifdef GS_DEBUG
    // static or not ?
    QScriptValue GetSetLogLevel(QScriptContext *context, QScriptEngine */*engine*/)
    {
        //QScriptValue callee = context->callee();
        if (!context)
            return QScriptValue("error: context null");

        //printf("GetSetLogLevel argc=%d engine=%s\n", context->argumentCount(), engine?"0":"1");

        if (context->argumentCount()==0)
            return QScriptValue("error: no argument");
        if (context->argumentCount()>2)
            return QScriptValue("error: too much argument");

        QString lModuleName=context->argument(0).toString();
        if (lModuleName.isEmpty())
            return QScriptValue("error: first argument empty");

        if (context->argumentCount() == 1) // get
        {
            if (!s_LogLevels.contains(lModuleName))
                return QScriptValue("error: module unfoundable in current map");
            return QScriptValue(s_LogLevels.value(lModuleName));
        }

        if (context->argumentCount() == 2) // set
        {
            qint32 lLevel=context->argument(1).toInt32();
            if (lLevel<0 || lLevel>7)
                return QScriptValue("error: illegal loglevel (must be between 0 and 7)");
            s_LogLevels.insert(lModuleName, lLevel);
            return QScriptValue("ok");
        }
        return QScriptValue("error");
    }
#endif

extern "C" GQTLLOGSHARED_EXPORT
int gqtl_log(int sev, const char* module, const char* file, const char* func, const char* m)
{
    lpMutex.lock();
    if (!m)
    {
        lpMutex.unlock();
        return -1;
    }

    if (sev==-1)
    {
        // special command to change a level
        QString mn=QString(m).section(' ', 0, 0);
        if (!mn.endsWith("_LOGLEVEL"))
        {
            lpMutex.unlock();
            return -1;
        }
        bool ok=false;
        int l=QString(m).section(' ', 1, 1).toInt(&ok);
        if (!ok)
        {
            lpMutex.unlock();
            return -1;
        }
        s_LogLevels.insert(mn, l);

        lpMutex.unlock();
        return 0;
    }


    QString mevn=QString("%1_LOGLEVEL").arg(module?(QString(module).isEmpty()?"OTHER":module):"OTHER");
    int ll=5;
    if (!s_LogLevels.contains(mevn))
    {
        QString ev(getEnvString(mevn));
        if (! ev.isEmpty())
        {
            #ifdef GS_DEBUG
                printf("gqtl_log: found %s : %s in env vars\n",
                       mevn.toLatin1().data(),
                       ev.toLatin1().data());
            #endif
            bool ok=false;
            ll = ev.toInt(&ok);
            if (!ok)
                ll=5;
        }
        #ifdef GS_DEBUG
            printf("gqtl_log: registering %s : %d (now %d modules)\n", mevn.toLatin1().data(), ll, s_LogLevels.size()+1);
        #endif
        s_LogLevels.insert(mevn, ll);
    }
    else
        ll=s_LogLevels.value(mevn);

    if (sev>ll)
    {
        lpMutex.unlock();
        return -1;
    }

    if (!s_gexloglogfile.isOpen())
    {
        if (!s_gexloglogfile.open(QIODevice::Append))	//  QIODevice::WriteOnly
        {
            //char* gsp=getenv("GEX_SERVER_PROFILE");
            QDir d(QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");
            d.mkpath(QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");
            if (!s_gexloglogfile.open(QIODevice::WriteOnly))
            {
                #ifdef GS_DEBUG
                    printf("gqtl_log: can't open gslog logfile %s !\n", s_gexloglogfile.fileName().toLatin1().data() );
                #endif
                s_gexloglogfile.setFileName(QDir::tempPath()+QDir::separator()+"gslog.txt");
                s_gexloglogfile.open(QIODevice::WriteOnly);
            }
        }
        else
            s_gexloglogfile.write( QString("\n%1 : gqtl_log : %2 opened...\n")
                .arg( QTime::currentTime().toString(Qt::ISODate) )
                .arg( s_gexloglogfile.fileName() )
                .toLatin1().data() );

        #ifdef GS_DEBUG
            qDebug("gqtl_log : %s %s",
                s_gexloglogfile.fileName().toLatin1().data(),
                s_gexloglogfile.isOpen()?"opened":"not opened !"
               );
        #endif
    }

    if (!s_bConfFileParsed)
    {
        s_bConfFileParsed=true;
        /*
        if(!isatty(fileno(stderr)))
          qInstallMsgHandler(myMsgHandler);
        if(!isatty(fileno(stdout)))
          qInstallMsgHandler(myMsgHandler);
        */

        QMap< QString, QString> attsMap;
        attsMap.insert("module", QString(module?module:"OTHER") );

        // case 7556
        //char* gsp=getenv("GEX_SERVER_PROFILE");
        QString f=QDir::homePath()+QDir::separator()+"GalaxySemi"+QDir::separator()+"gslog.xml";
        QString r=open_conf_file(f, attsMap);
        s_gexloglogfile.write(QString("%1 : open_conf_file  %2 : %3\n")
                              .arg(QTime::currentTime().toString(Qt::ISODate))
                              .arg(f)
                              .arg(r)
                              .toLatin1().data()
                              );

        if (r.startsWith("error"))
        {
            #ifdef GS_DEBUG
                printf("log : %s\n", r.toLatin1().data());
            #endif
            f="gslog.xml";
            r=open_conf_file(f, attsMap);
            if (r.startsWith("error"))
            {
                /*
                if (QCoreApplication::instance())
                    f=QCoreApplication::applicationDirPath()+QDir::separator()+"gslog.xml";
                r=open_conf_file(f);
                */

                // QCoreApplication::applicationDirPath() does not always work, specially on Microsoft
                QString strApplicationDir;
                QString ri=GetApplicationDirectory(strApplicationDir);
                if( ri == "ok" )
                {
                    s_gexloglogfile.write(QString("%1 : strApplicationDir = %2\n")
                       .arg(QTime::currentTime().toString(Qt::ISODate))
                       .arg(strApplicationDir).toLatin1().data() );
                    #ifdef GS_DEBUG
                        printf("log : strApplicationDir = %s\n", strApplicationDir.toLatin1().data());
                    #endif
                    f=strApplicationDir+QDir::separator()+"gslog.xml";
                    r=open_conf_file(f, attsMap);
                    if (r.startsWith("err"))
                    {
                        r=open_conf_file(strApplicationDir+QDir::separator()+"samples/gslog.xml", attsMap);
                    }
                }
                else
                {
                    s_gexloglogfile.write(QString("%1 : strApplicationDir unfindable\n")
                               .arg(QTime::currentTime().toString(Qt::ISODate))
                               .toLatin1().data());
                    #ifdef GS_DEBUG
                       printf("strApplicationDir unfindable : %s\n", ri.toLatin1().data());
                    #endif
                }
            }

            #ifdef GS_DEBUG
                printf("log : %s", r.toLatin1().data());
            #endif

            s_gexloglogfile.write( QString("%1 : log() : %2\n")
                    .arg(QTime::currentTime().toString(Qt::ISODate))
                    .arg(r)
                    .toLatin1().data() );
        }

        s_gexloglogfile.write( QString("%1 : log() : %2\n")
              .arg(QTime::currentTime().toString(Qt::ISODate))
                            .arg(r)
                            .toLatin1().data() );


        #ifdef GS_DEBUG
         qDebug(r.toLatin1().data(), 0);
        #endif
        s_gexloglogfile.flush();

        if (COutput::s_ListCOutput.size()>0)
        {
            // Let s try to simplify
            /*
                int r=atexit(FreeLogs);
                s_gexloglogfile.write( QString("%1 : atexit FreeLogs returned %2\n")
                            .arg(QTime::currentTime().toString(Qt::ISODate))
                            .arg(r)
                            .toLatin1().data() );
                #ifdef QT_DEBUG
                    qDebug("log : %d logs found. atexit returned %d.",  COutput::s_ListCOutput.size(), r);
                #endif
            */
        }

    }

    // Trying to retrieve the ScripEngine
#ifdef GS_DEBUG
    static QScriptEngine* lSE=0;
    // The scrip engine cannot exist before the QApp is. So let's wait for the QApp to be created...
    if (!lSE && !qApp->startingUp())
    {
        lSE=qApp->findChild<QScriptEngine*>("GSScriptEngine");
        if (lSE)
        {
            #ifdef GS_DEBUG
                qDebug("gqtl log found the ScriptEngine\n");
            #endif
            //lSE->globalObject().setProperty("LogLevels", s_LogLevels); // QMap vs QScriptValue
            //lSE->newArray() ?
            //lSE->newObject() ?
            //QScriptValue object = lSE->newObject();
            //object
            lSE->globalObject().setProperty("GSGetSetLogLevel", lSE->newFunction(GetSetLogLevel));
                               //QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
        } // if ScriptEngine not found, let s try later ?
    }
#endif

    /*
    if (s_bThreaded && !s_pThread)
    {
        s_pMutex=new QMutex();
        s_pThread=new CGexLogThread();
        if (s_pThread)
            s_pThread->start((QThread::Priority)s_iThreadPriority);	//QThread::LowestPriority

        //QObject::connect(this, SIGNAL(newMessage), s_pThread, SLOT(newMessage()) );
    }
    */

    SMessage sm;
    sm.m_sev=sev;
    sm.m_atts.insert( "msg", QString(m) );
    sm.m_atts.insert( "file", QString(file) );
    sm.m_atts.insert( "func", QString(func) );
    sm.m_atts.insert( "module", QString(module?module:"OTHER") );
    sm.m_date=QDate::currentDate();
    sm.m_time=QTime::currentTime();


    /*
    if (s_bThreaded && s_pThread)
    {
        COutput* o=0;
        foreach(o, COutput::s_ListCOutput)
        {
            s_pMutex->lock();
            o->m_buffer.push_back(sm);
            s_pMutex->unlock();
        }

        //QEvent e(QEvent::User);
        //QCoreApplication::sendEvent(s_pThread, &e);
        //s_pThread->newMessage(sm);
        //s_pThread->emitNewMessage(sm);
    }
    else
    */
    {
        COutput* o=0;
        foreach(o, COutput::s_ListCOutput)
        {
            o->m_buffer.push_back(sm);
            o->PopFront();
        }
    }
    lpMutex.unlock();
    return (int)1;
}

// Todo : install msh handler in orer to retireive all printf and std::cout and std::cerr...
void myMsgHandler(QtMsgType /*mt*/, const char* /*m*/)
{
    //_gslog((int)mt, "Qt", "Qt", "Qt", m);

    /*
    char* qtmt[]={ (char*)"QtDebugMsg", (char*)"QtWarningMsg", (char*)"QtCriticalMsg", (char*)"QtFatalMsg", (char*)"QtSystemMsg", (char*)"QtCriticalMsg"};
    #ifdef QT_DEBUG
     qDebug("%d %s : %s", mt, mt<6?qtmt[mt]:"?", m?m:"?");
    #endif
    */
}
