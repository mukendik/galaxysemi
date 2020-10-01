#include "gexdb_plugin_galaxy.h"
#include "consolidation_center.h"
#include "consolidation_tree_replies.h"
#include <gqtl_log.h>
#include "libgexpb.h"
#include "xmlsyntaxhighlighter.h"
#include <QLabel>
#include <QWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>

ConsolidationCenter::ConsolidationCenter(QWidget* pw, GexDbPlugin_Galaxy* p):QWidget(pw), mPlugin(p)
{
  setLayout(new QVBoxLayout());
  layout()->addWidget(new QLabel("Welcome to Consolidation Center"));
  mXMLTextEdit=new QTextEdit(this);
  mXMLTextEdit->setTabStopWidth(10);
  QObject::connect(mXMLTextEdit, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
  //mXMLTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  mXmlSyntaxHighlighter=new XmlSyntaxHighlighter(mXMLTextEdit->document());

  layout()->addWidget(mXMLTextEdit);

  mTableWidget=new QTableWidget(this);
  mTableWidget->setColumnCount(4);
  QStringList sl; sl<<"Type" <<"Line"<<"Col"<<"Cause";
  mTableWidget->setHorizontalHeaderLabels(  sl );
  mTableWidget->verticalHeader()->setVisible(false);
  mTableWidget->horizontalHeader()->setVisible(false);

  mTableWidget->hideColumn(1);
  mTableWidget->hideColumn(2);
  //mTableWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  layout()->addWidget( mTableWidget );

  // Create Text Edit used to Log DB update
  mLogTextEdit = new QTextEdit(this);

  QSplitter *s=new QSplitter(Qt::Vertical, this);
  s->addWidget(mXMLTextEdit);
  s->addWidget(mTableWidget);
  s->addWidget(mLogTextEdit);
  layout()->addWidget(s);

  QGroupBox* gb=new QGroupBox("Actions", this);
    QHBoxLayout *gblayout=new QHBoxLayout(gb);
    gb->setLayout(gblayout);

    mValidateButton=new QPushButton("Validate", gb);
    gblayout->addWidget(mValidateButton);
    QObject::connect(mValidateButton, SIGNAL(released()), this, SLOT(OnValidateButtonReleased()));

    mSendButton=new QPushButton("Send", gb);
    QObject::connect(mSendButton, SIGNAL(released()), this, SLOT(OnSendButtonReleased()));
    gblayout->addWidget(mSendButton);

    mRevertButton=new QPushButton("Revert", gb);
    gblayout->addWidget( mRevertButton );
    QObject::connect(mRevertButton, SIGNAL(released()), this, SLOT(OnRevertButtonReleased()));

    layout()->addWidget(gb);

  //setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
}

ConsolidationCenter::~ConsolidationCenter()
{
  if (mXmlSyntaxHighlighter)
    delete mXmlSyntaxHighlighter;
  if (mXMLTextEdit)
    delete mXMLTextEdit;
  if (mLogTextEdit)
      delete mLogTextEdit;

  foreach(QTableWidgetItem* i, mListTableWidgetItem)
    if (i)
      delete i;
  mListTableWidgetItem.clear();
}

QString ConsolidationCenter::Reload()
{
  QString xmlString;
  CTReplies rs;
  if (mPlugin->GetConsolidationTree(xmlString, rs))
  {
      mXMLTextEdit->setText(xmlString);
      mTableWidget->hide();
      mLogTextEdit->hide();
      mSendButton->setEnabled(false);
      Validate();
  }
  else
  {
      /*
      QString m;
      if (rs.count()>0)
        m=rs.reply(0).message();
      else
        m="Impossible to load Consolidation Tree!";
      mXMLTextEdit->setText( m );
      */
      mXMLTextEdit->setText( xmlString);
      if (rs.count()==0 || xmlString.isEmpty() )
        QMessageBox::critical(this, "Consolidation center",
           "Getting consolidation tree failed. Please contact Quantix support team.");
      Validate();
      return "error";
  }

  return "ok";
}

void ConsolidationCenter::OnValidateButtonReleased()
{
  QString r=Validate();
  GSLOG(SYSLOG_SEV_DEBUG, QString("Validate : %1").arg( r).toLatin1().constData());
}

void ConsolidationCenter::OnRevertButtonReleased()
{
  QString r=Reload();
  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Reload : %1").arg( r).toLatin1().constData());
}

void ConsolidationCenter::UpdateLogMessage(const QString &message, bool isPlainText)
{
    QString lText = message;

    if(isPlainText)
        mLogTextEdit->insertPlainText(lText);
    else
        mLogTextEdit->insertHtml(lText);

    mLogTextEdit->moveCursor(QTextCursor::End);
    mLogTextEdit->ensureCursorVisible();

    QCoreApplication::processEvents();
}

QString ConsolidationCenter::Validate()
{
  CTReplies rs;

  // Hide log text edit
  mLogTextEdit->hide();

  bool b=mPlugin->ValidateConsolidationTree(mXMLTextEdit->toPlainText(), rs);
  if (!b)
    GSLOG(SYSLOG_SEV_DEBUG, "ValidateConsolidationTree failed !");
    //QMessageBox::critical(this, "Error", "Cannot validate tree!");
    //return "error";

  if (b && rs.count()==0)
  {
    mSendButton->setEnabled(true);
    mTableWidget->hide();
    return "ok";
  }

  mTableWidget->show();
  mTableWidget->clear();
  mTableWidget->setRowCount(0);
  //mTableWidget->setHorizontalHeader();
  mTableWidget->horizontalHeader()->setStretchLastSection(true);
  for (int i=0; i<(int)rs.count(); i++)
  {
    CTReply r=rs.reply(i);

    mTableWidget->insertRow(i);

    QIcon icon;
    if (r.type()==CTReply::ReplyError)
      icon.addFile(QString::fromUtf8(":/gex/icons/stop.png"), QSize(), QIcon::Normal);
    else if (r.type()==CTReply::ReplyWarning)
      icon.addFile(QString::fromUtf8(":/gex/icons/warning.png"), QSize(), QIcon::Normal);
    //else
     // icon.addFile(QString::fromUtf8(":/gex/icons/gex_application.png"), QSize());

    QTableWidgetItem *it=0; //mListTableWidgetItem.isEmpty()?0:mListTableWidgetItem.takeLast(); //mTableWidget->itemAt(i, 0);
    if (!it)
    {
      it=new QTableWidgetItem(icon, "" );
      mListTableWidgetItem.append(it);
    }

    it->setIcon(icon); it->setText("");
    mTableWidget->setItem(i, 0, it);

    it=0; //mListTableWidgetItem.isEmpty()?0:mListTableWidgetItem.takeLast();
    if (!it)
    {
      it=new QTableWidgetItem( "" );
      mListTableWidgetItem.append(it);
    }
    it->setText(QString::number(r.line()));
    mTableWidget->setItem( i, 1, it );

    it=new QTableWidgetItem( QString::number(r.column()) );
    mListTableWidgetItem.append(it);
    mTableWidget->setItem( i, 2, it );

    mTableWidget->setCellWidget( i, 3, new QLabel(r.message(), mTableWidget)  );
  }
  //mTableWidget->takeHorizontalHeaderItem()
  mTableWidget->resizeColumnToContents(0);
  mTableWidget->resizeRowsToContents();

  return "ok";
}

QString ConsolidationCenter::Send()
{
    CTReplies rs;
    // Show text edit receiving log from DB update
    mLogTextEdit->clear();
    mLogTextEdit->show();

    mPlugin->InsertIntoUpdateLog("o Updating Consolidation Tree...");
    mPlugin->InsertIntoUpdateLog("");
    connect(mPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateLogMessage(QString,bool)));
    if (mPlugin->SendConsolidationTree(mXMLTextEdit->toPlainText(), rs) == false)
    {
        mPlugin->InsertIntoUpdateLog("o Consolidation tree update failed");
        return "error";
    }
    else
    {
        mPlugin->InsertIntoUpdateLog("");
        mPlugin->InsertIntoUpdateLog("o Status = SUCCESS");
    }
    disconnect(mPlugin, SIGNAL(sLogRichMessage(QString,bool)), this, SLOT(UpdateLogMessage(QString,bool)));

    return "ok";
}

void ConsolidationCenter::OnSendButtonReleased()
{
  QString r=Send();
  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Sending tree : %1").arg( r).toLatin1().constData());

}


bool GexDbPlugin_Galaxy::InitConsolidationWidget()
{
    if (!mGexScriptEngine->property("GS_DAEMON").toBool())
    {
        mConsolidationCenter = new ConsolidationCenter(0, this);
        if (!mConsolidationCenter)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to create a new ConsolidationCenter : mConsolidationCenter will be NULL !");
            return false;
        }
    }
    return true;
}

QWidget*  GexDbPlugin_Galaxy::GetConsolidationWidget()
{
  if (!mConsolidationCenter)
      InitConsolidationWidget();

  if (mConsolidationCenter)
      mConsolidationCenter->Reload();

  return mConsolidationCenter;
}
