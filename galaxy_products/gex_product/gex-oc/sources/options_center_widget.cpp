#include <QMenu>
#include <QApplication>
#include <QAction>
#include <QFile>
#include <QLabel>
#include <QDomDocument>
#include "options_center_widget.h"
#include "options_center_propbrowser.h"

#include <gqtl_log.h>

OptionsCenterWidget::OptionsCenterWidget(GexMainwindow* gmw, QWidget* p)
    : QWidget(p), m_vlayout(NULL), m_propBrowser(NULL)
{
    GSLOG(SYSLOG_SEV_DEBUG, "new OptionsCenterWidget ");
    m_pGexMainWindow=gmw;
    //m_dont_emit_change=false;

    m_vlayout=new QVBoxLayout(this);
    if (!m_vlayout)
        return;
    this->setLayout(m_vlayout);

    m_propBrowser=new OptionsCenterPropBrowser(this);
    if (!m_propBrowser)
    {
        GSLOG(4, "can't instantiate a OptionsCenterPropBrowser !");
        return;		/// TO REVIEW : return in constructor can induce crash (not initialized pointer)
    }

    QObject::connect(m_propBrowser, SIGNAL(SignalMessage(QString)), this, SIGNAL(signalMessage(QString)));

    GSLOG(SYSLOG_SEV_DEBUG, " creating GUI...");
    // Remove warning by replacing this with NULL: "QLayout: Attempting to add QLayout \"%s\" to %s \"%s\", which"" already has a layout"
    // The m_tophlayout is a child of m_vlayout, not a child of this.
    // This is done later with this code : m_vlayout->addLayout(m_tophlayout);
    m_tophlayout=new QHBoxLayout(NULL);

    m_CollapseButton=new QPushButton("Collapse", this);
    m_CollapseButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_tophlayout->addWidget(m_CollapseButton);
    /// TO REVIEW
    m_CollapseButton->hide();

//	m_title=new QLabel("Options Center", this);
//	m_title->setAlignment(Qt::AlignHCenter);
//	m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
//	QFont f(m_title->font()); f.setBold(true);
//	m_title->setFont(f);
//	m_tophlayout->addWidget(m_title);

    m_LoadProfile=new QPushButton(this);
    m_LoadProfile->setToolTip("Load options...");
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_LoadProfile->sizePolicy().hasHeightForWidth());
    m_LoadProfile->setSizePolicy(sizePolicy);
    m_LoadProfile->setMinimumSize(QSize(32, 26));
    m_LoadProfile->setMaximumSize(QSize(32, 26));
    QIcon icon;
        icon.addFile(QString(":/gex/icons/file_open.png"),
                     QSize(), QIcon::Normal, QIcon::Off);
    m_LoadProfile->setIcon(icon);
    m_LoadProfile->setIconSize(QSize(32, 32));
    m_tophlayout->addWidget(m_LoadProfile);

    m_SaveButton=new QPushButton(this);
    m_SaveButton->setToolTip("Save options...");
    m_SaveButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_SaveButton->setMinimumSize(QSize(32, 26));
    m_SaveButton->setMaximumSize(QSize(32, 26));
    QIcon icon1;
    icon1.addFile(QString(":/gex/icons/file_save.png"),
                  QSize(), QIcon::Normal, QIcon::Off);
    m_SaveButton->setIcon(icon1);
    m_SaveButton->setIconSize(QSize(32, 32));
    m_tophlayout->addWidget(m_SaveButton);

    m_SaveProfile=new QPushButton(this);
    m_SaveProfile->setToolTip("Save options as...");
    //QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_SaveProfile->sizePolicy().hasHeightForWidth());
    m_SaveProfile->setSizePolicy(sizePolicy);
    m_SaveProfile->setMinimumSize(QSize(32, 26));
    m_SaveProfile->setMaximumSize(QSize(32, 26));
    QIcon icon2;
    icon2.addFile(QString(":/gex/icons/file_save_as.png"),
                  QSize(), QIcon::Normal, QIcon::Off);
    m_SaveProfile->setIcon(icon2);
    m_SaveProfile->setIconSize(QSize(32, 32));
    m_tophlayout->addWidget(m_SaveProfile);

    m_ResetButton=new QPushButton("Reset", this);
    m_ResetButton->setToolTip("Reset to default options...");
    m_ResetButton->setMinimumSize(QSize(60, 26));
    m_ResetButton->setMaximumSize(QSize(60, 26));
    m_tophlayout->addWidget(m_ResetButton);

    QWidget* lSpacer = new QWidget();
    lSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_tophlayout->addWidget(lSpacer);

    m_BuildReport=new QPushButton("Build report!", this);
    m_BuildReport->setToolTip("Build report / Abort report creation");
    m_BuildReport->setMinimumSize(QSize(100, 26));
    m_BuildReport->setMaximumSize(QSize(100, 26));
    m_tophlayout->addWidget(m_BuildReport);

    m_vlayout->addLayout(m_tophlayout);

    QHBoxLayout* lSourceLayout = new QHBoxLayout(NULL);
    mSourceLabel=new QLabel("", this);
    mSourceLabel->setAlignment(Qt::AlignLeft);
    mSourceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    lSourceLayout->addWidget(mSourceLabel);

    m_propBrowser->layout()->setMargin(0);
    m_vlayout->addWidget(m_propBrowser);

    m_vlayout->addLayout(lSourceLayout);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_propBrowser, SIGNAL(SignalOcPbValueChanged(QString,QString,QString)), this, SLOT(EmitOptionChanged(QString,QString,QString)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(SlotOpenCustomContextMenu(const QPoint)) );

    emit signalMessage("OptionsCenter created");
    QCoreApplication::processEvents();

    GSLOG(SYSLOG_SEV_DEBUG, "OptionsCenterWidget::OptionsCenterWidget ok");
}

OptionsCenterWidget::~OptionsCenterWidget()
{
    GSLOG(SYSLOG_SEV_NOTICE, " ");
    if (m_vlayout)
        delete m_vlayout;
    if (m_propBrowser)
    {
        delete m_propBrowser;
        m_propBrowser=0;
    }
    GSLOG(SYSLOG_SEV_NOTICE, " ok");
}

QString OptionsCenterWidget::BuildFromGOXML(const QString filename)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("OptionsCenterWidget::BuildFromGOXML: %1").arg(filename).toLatin1().data());
    emit signalMessage("OptionsCenter building from xml...");
    QCoreApplication::processEvents();

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return QString("error : can't open %1 !").arg(filename);
    GSLOG(SYSLOG_SEV_DEBUG, "OptionsCenterWidget::BuildFromGOXML: opened...");

    QDomDocument doc("goxml");
    QString errorMsg; int errorLine=0;
    if (!doc.setContent(&f, &errorMsg, &errorLine))
    {
        f.close();
        return QString("error : xml not compliant (line %1) : ").arg(errorLine)+errorMsg;
    }
    f.close();

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName()!="gex_options")
    {
        return "error : not a 'gex_options' master element !";
    }

    /// TO REVIEW : does this function should be used ?
    //m_propBrowser->clear();
    // recursive

    QString strBuildFromGOXMLOutput = m_propBrowser->BuildFromGOXML(&docElem);

    emit signalMessage("OptionsCenter building from xml:"+strBuildFromGOXMLOutput);

    return strBuildFromGOXMLOutput;
}

bool OptionsCenterWidget::SetOption(QString s, QString f, QString newvalue)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,QString(" '%1' '%2' to '%3'")
            .arg(s.toLatin1().data())
            .arg(f.toLatin1().data())
            .arg(newvalue).toLatin1().constData());

    if (!m_propBrowser)
        return false;

    bool bSetOptionRslt=m_propBrowser->SetOption(s, f, newvalue);

    return bSetOptionRslt;
}

QVariant OptionsCenterWidget::GetOption(QString section, QString field)
{
    if (!m_propBrowser)
        return QVariant();

    return m_propBrowser->GetOption(section, field);
}

void OptionsCenterWidget::EmitOptionChanged(QString s, QString f, QString nv)
{
    //m_propBrowser->SetOption(s, f, nv);

    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1 - %2 = %3")
            .arg(s.toLatin1().data())
            .arg(f.toLatin1().data())
            .arg(nv).toLatin1().constData());
    emit signalOptionChanged(s, f,nv);
};

void OptionsCenterWidget::SlotExpandAll(bool bExpandAll)
{m_propBrowser->SlotExpandAll(bExpandAll);}

void OptionsCenterWidget::SlotOpenCustomContextMenu(const QPoint & pos)
{
    /// TODO: develop internal configurable menu in gexpb
    GEX_ASSERT(!pos.isNull());

    QAction qaExpandAllAction(QString("Expand all"), this);
    QAction qaCollapseAllAction(QString("Collapse all"), this);

    QList<QAction *> qlActionList;
    qlActionList << &qaExpandAllAction << &qaCollapseAllAction;

    QAction* qaPtrReturnAction = QMenu::exec(qlActionList, QCursor::pos(), &qaExpandAllAction, this);
    //QAction* qaPtrReturnAction = QMenu::exec(qlActionList, pos, &qaExpandAllAction);

    SlotExpandAll((&qaExpandAllAction==qaPtrReturnAction));
}
