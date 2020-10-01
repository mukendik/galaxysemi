#include <QApplication>
#include <QShortcut>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDialog>
#include <QLineEdit>
#include <QMainWindow>

#include "qscriptsyntaxhighlighter_p.h"
#include "javascriptcenter.h"
#include <gqtl_log.h>
#include "file.h"
#include "engine.h"

#include <QtScriptTools>
#include <QScriptEngineDebugger>

JavaScriptCenter::JavaScriptCenter(QWidget *p):QWidget(p),
    mJSTextEdit(this), mStatus(this), mEvaluateShortcut(&mJSTextEdit),
    mAttachDebuggerCB(this), mDebuggerPasswordOk(false)
{
  setLayout(&mVBLayout);
  setObjectName("GSJavaScriptCenter");

  static QFont lF=mJSTextEdit.currentFont();
  lF.setPointSize( 10 );
  mJSTextEdit.setCurrentFont(lF);
  mJSTextEdit.setTabStopWidth(30);
  mJSTextEdit.setAcceptRichText(false);
  //layout()->setM

  mHighlighter.setDocument(mJSTextEdit.document());

  layout()->addWidget(&mJSTextEdit);

  //QHBoxLayout* hbl=new
  mVBLayout.addLayout(&mHBLayout);

  mEvaluateButton.setText("Evaluate");
  mEvaluateButton.setParent(this);

  mAbortButton.setText("Abort");
  mAbortButton.setParent(this);

  mHBLayout.addWidget(&mEvaluateButton);
  mHBLayout.addWidget(&mAbortButton);

  mProcessEventsInterval.setParent(this);
  mProcessEventsInterval.setMinimum(-1);
  mProcessEventsInterval.setValue(1);
  QString tt("A interval of -1 indicates that no process events will be done at all during script execution," \
             " 0 means the events will be processed as soos as possible. Else enter an interval in ms.");
  mProcessEventsInterval.setToolTip(tt);
  mProcessEventsLabel.setText("Process events interval");
  mProcessEventsLabel.setAlignment(Qt::AlignRight);
  mProcessEventsLabel.setToolTip(tt);
  mHBLayout.addWidget(&mProcessEventsLabel);
  mHBLayout.addWidget(&mProcessEventsInterval);

  mAttachDebuggerCB.setText("Attach debugger");
  mAttachDebuggerCB.setChecked(false);
  mHBLayout.addWidget(&mAttachDebuggerCB);
  connect(&mAttachDebuggerCB, SIGNAL(clicked()), // already tested : released()
          this, SLOT(OnAttachDebuggerRequested()));

  mStatus.setReadOnly(true);
  // Minimum : mStatus is 25% of the GUI
  // Fixed : 25%
  // Maximum : 25%
  // Prefered : 25%
  // Expanding : 50%
  // MinimumExpanding : 50%
  // Ignored : 0% !
  mStatus.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout()->addWidget(&mStatus); //layout()->addWidget(&mConsoleTextEdit);

  layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

  QObject::connect(&mEvaluateButton, SIGNAL(released()), this, SLOT(OnEvaluate()));
  mStatus.setText("Ready");

  //QShortcut* s1=
          new QShortcut(Qt::CTRL+Qt::Key_Return, this, SLOT(OnEvaluate()));
  //QObject::connect(s1, SIGNAL(activated()), this, SLOT(OnEvaluate()));

  new QShortcut(Qt::CTRL+Qt::Key_S, this, SLOT(OnSave()));

  //new QShortcut(Qt::SHIFT+Qt::Key_Enter, this, SLOT(OnEvaluate()));
  //QShortcut* s2=new QShortcut(Qt::CTRL+Qt::Key_Enter, &mJSTextEdit, SLOT(OnEvaluate()));
  //b=QObject::connect(s2, SIGNAL(activated()), this, SLOT(OnEvaluate()));

  //mEvaluateShortcut.setKey(Qt::CTRL+Qt::Key_Enter);
  //b=QObject::connect(&mEvaluateShortcut, SIGNAL(activated()), this, SLOT(OnEvaluate()));
  //if (!b)
  //  GSLOG(SYSLOG_SEV_WARNING, "Unable to connect Evaluate shortcut");

  QFile f(QDir::homePath()+"/GalaxySemi/temp/javascriptcenter.txt");
  if (f.open(QIODevice::ReadOnly))
  {
      mJSTextEdit.setText(f.readAll());
      f.close();
  }
}

JavaScriptCenter::~JavaScriptCenter()
{
    OnSave();
}

void JavaScriptCenter::OnAbort()
{
    QScriptValue lSV;
    pGexScriptEngine->abortEvaluation(lSV);
    mStatus.setText("Abort: "+lSV.toString());
}

bool JavaScriptCenter::OnAttachDebuggerRequested()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("OnAttach Debugger Requested: checked? %1").
          arg(mAttachDebuggerCB.isChecked() ? "true" : "false").
          toLatin1().constData());

    if (!mAttachDebuggerCB.isChecked())
    {
        //GSLOG(SYSLOG_SEV_INFORMATIONAL, "OnAttach Debugger Requested: already checked : let s turn it off.");
        //mAttachDebuggerCB.setChecked(false);
        return true;
    }

    if (mDebuggerPasswordOk)
        return true;

    #ifdef QT_DEBUG
        mDebuggerPasswordOk=true;
        return true;
    #endif

    QDialog lD(0);
    QVBoxLayout lVBL;
    lD.setLayout(&lVBL);
    QLabel lLabel("Plese enter password to access this feature:", &lD);
    lD.layout()->addWidget(&lLabel);
    QLineEdit lLE(&lD);
    lD.layout()->addWidget(&lLE);
    QPushButton lPB("ok", &lD);
    connect(&lPB, SIGNAL(clicked()), &lD, SLOT(accept()) );
    lD.layout()->addWidget(&lPB);
    int r=lD.exec();
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Dialog exec returned %1").arg(r).toLatin1().constData());
    if (r!=QDialog::Accepted)
        return false;

    QNetworkRequest nr;
    nr.setUrl(QUrl(
      "http://galaxyec7.com/helpconsole2010/"+lLE.text()+".txt"));
    QNetworkReply* reply=GS::Gex::Engine::GetInstance().GetNAM().get(nr);
    if (!reply)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Network get returned a null reply. Check your internet connection.");
        return false;
    }
    while (!reply->isFinished())
    {
        //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("NetworkReply : running:%1, finished:%2").arg(.arg(        //GSLOG(SYSLOG_SEV_INFORMATIONAL, "NetworkReply : running:%1.arg( finished:%2").arg(
        //  reply->isRunning()?"yes":"no", reply->isFinished()?"yes":"no" );
        QCoreApplication::instance()->QCoreApplication::processEvents();
        //QThread::currentThread()->wait(100);
    }
    if (reply->error()!=QNetworkReply::NoError)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Network Reply %1. Bad password ?").
              arg(reply->error()).toLatin1().constData());
        return false;
    }
    mAttachDebuggerCB.setChecked(true);
    return true;
}

bool JavaScriptCenter::OnSave()
{
    QFile f(QDir::homePath()+"/GalaxySemi/temp/javascriptcenter.txt");
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(mJSTextEdit.toPlainText().toLatin1().constData());
        f.close();
        return true;
    }
    return false;
}

void JavaScriptCenter::OnEvaluate()
{
  // Q_FUNC_INFO = __FUNC__ or __PRETTY_FUNC__
  GSLOG(SYSLOG_SEV_INFORMATIONAL, "On Evaluate...");

  mStatus.setText("Running...");

  if (!pGexScriptEngine)
  {
    GSLOG(SYSLOG_SEV_ERROR, "GexScriptEngine NULL");
    mStatus.setText("GexScriptEngine NULL");
    return;
  }

  mJSTextEdit.setEnabled(false);
  mEvaluateButton.setEnabled(false);
  mProcessEventsLabel.setEnabled(false);
  mProcessEventsInterval.setEnabled(false);

  if (!UpdateGexScriptEngine(NULL))
  {
    GSLOG(SYSLOG_SEV_WARNING, "UpdateGexScriptEngine failed");
    mStatus.setText(" UpdateGexScriptEngine failed");
    mJSTextEdit.setEnabled(true);
    mProcessEventsLabel.setEnabled(true);
    mProcessEventsInterval.setEnabled(true);
    mEvaluateButton.setEnabled(true);
    return;
  }

  pGexScriptEngine->setProcessEventsInterval(mProcessEventsInterval.value());

  QString code=mJSTextEdit.textCursor().selectedText();
  if (code.isEmpty())
      code = mJSTextEdit.toPlainText();

  // Remember to detach the debugger after evaluation !!!
  if (mAttachDebuggerCB.isChecked())
  {
    GS::Gex::Engine::GetInstance().GetScriptEngineDebbugger().attachTo(pGexScriptEngine);
    GS::Gex::Engine::GetInstance().GetScriptEngineDebbugger().standardWindow()->show();
  }

  QScriptValue scriptValue = pGexScriptEngine->evaluate( code,
    GS::Gex::Engine::GetInstance().Get("TempFolder").toString()+QDir::separator()+"js_log.txt" );

  if (scriptValue.isError() || pGexScriptEngine->hasUncaughtException())
  {
      QString m=QString("%1").arg(pGexScriptEngine->uncaughtException().toString());
      GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
      mStatus.setText(m);
      mJSTextEdit.setEnabled(true);
      mEvaluateButton.setEnabled(true);
      mProcessEventsInterval.setEnabled(true);
      GS::Gex::Engine::GetInstance().GetScriptEngineDebbugger().detach();
      return;
  }

  GS::Gex::Engine::GetInstance().GetScriptEngineDebbugger().detach();

  mJSTextEdit.setEnabled(true);
  mEvaluateButton.setEnabled(true);
  mStatus.setText("ok : "+scriptValue.toString());
  mProcessEventsInterval.setEnabled(true);
  mJSTextEdit.setFocus();
}
