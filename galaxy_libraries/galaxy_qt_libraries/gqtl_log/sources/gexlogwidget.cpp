#include <QMenu>
#include <QLabel>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDesktopWidget>

#include "coutput.h"
#include "gexlogwidget.h"


extern QStringList s_LevelsStrings;
 /* = (QStringList()
                 << "EMERG" << "ALERT"
                 << "CRITIC" << "ERROR"
                 << "WARN" << "NOTICE"
                 << "INFO" << "DEBUG" );
                 */

extern QMap<QString, int> s_LogLevels;

CGexLogWidget::CGexLogWidget(QWidget* p): QWidget(p), m_currentRow(0)
{
	setObjectName("CGexLogWidget");
	setLayout(new QVBoxLayout(this));
	m_tw.setParent(this);
	m_tw.setRowCount(10);
	m_tw.setColumnCount(6);
	//m_tw.setMidLineWidth();
	m_tw.setAlternatingRowColors(true);
	//m_tw.setContentsMargins();
	m_tw.setMidLineWidth(1);
	//m_tw.setInputMethodEnabled();
	m_tw.setHorizontalHeaderLabels(QStringList()<<"Time"<<"Sev"<<"Module"<<"Message"<<"Function"<<"File");

	m_llpb.setText("Logs menu");
	layout()->addWidget(&m_llpb);

	QMenu *menu = new QMenu(&m_llpb);
	menu->setObjectName("LogLevelMenu");
	m_llpb.setMenu(menu);

	foreach(const QString &log, s_LogLevels.keys())
	{
		addLogInMenu(log, s_LogLevels.value(log));
	}

	/*
		menu->addAction(tr("&First Item"));
		menu->addAction(tr("&Second Item"));
		menu->addAction(tr("&Third Item"));
		menu->addAction(tr("F&ourth Item"));
		popupButton->setMenu(menu);

		 QAction *newAction = menu->addAction(tr("Submenu"));
		 QMenu *subMenu = new QMenu(tr("Popup Submenu"));
		 subMenu->addAction(tr("Item 1"));
		 subMenu->addAction(tr("Item 2"));
		 subMenu->addAction(tr("Item 3"));
		 newAction->setMenu(subMenu);
	 */

	layout()->addWidget(&m_tw);

	QObject::connect(&m_tw, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onCellDoubleClicked(int,int)));

    if (QApplication::desktop())
        if (QApplication::desktop()->screenCount()>1)
        {
            QDesktopWidget* dw=QApplication::desktop();
            //this->move( QApplication::desktop()->screen(1)?QApplication::desktop()->screen(1)->pos():QApplication::desktop()->screen(0)->pos() );
            this->move( dw->screen()->width(), 0 );
        }
}

bool CGexLogWidget::addLogInMenu(QString mn, int l)
{
	if (mn.isEmpty())
		return false;

	QMenu *m=m_llpb.menu();
	if (!m)
		return false;

	QAction* a=0;
	foreach(a, m->actions())
	{
		if (a->text()==mn)
			return false;
	}

	QAction* loga=m->addAction(mn);
	QMenu* logMenu = new QMenu(mn, &m_llpb);

	logMenu->setTitle(mn);
	logMenu->setObjectName(mn);
	QActionGroup* ag=new QActionGroup(logMenu);

	a=logMenu->addAction("Emergency");
		a->setParent(logMenu);
		a->setCheckable(true);	//SYSLOG_SEV_EMERGENCY = 0,
		a->setActionGroup(ag);

	a=logMenu->addAction("Alert"); //, this, SLOT(onLogLevelActionTriggered()));
		a->setCheckable(true);
		a->setActionGroup(ag);

	a=logMenu->addAction("Critical");		//SYSLOG_SEV_CRITICAL = 2,
		a->setCheckable(true);
		a->setActionGroup(ag);

	a=logMenu->addAction("Error");		//SYSLOG_SEV_ERROR = 3,
		a->setCheckable(true);
		a->setActionGroup(ag);

	a=logMenu->addAction("Warning");		//SYSLOG_SEV_WARNING = 4,
		a->setCheckable(true);
		a->setActionGroup(ag);

	a=logMenu->addAction("Notice");		//SYSLOG_SEV_NOTICE = 5,
		a->setCheckable(true);
		a->setActionGroup(ag);

	a=logMenu->addAction("Informational");	//SYSLOG_SEV_INFORMATIONAL = 6,
		a->setCheckable(true);
		a->setActionGroup(ag);

	a=logMenu->addAction("Debug");			//SYSLOG_SEV_DEBUG = 7,
		a->setCheckable(true);
		a->setActionGroup(ag);

	//int ll=s_LogLevels[log];
	if (l<logMenu->actions().size())
		logMenu->actions().at(l)->setChecked(true);	//setItemChecked(s_LogLevels[log], true);	//setCheckable(true);

	QObject::connect(ag, SIGNAL(triggered(QAction*)), this, SLOT(onLogLevelActionTriggered(QAction*)) );

	loga->setMenu(logMenu);

	return true;
}


bool CGexLogWidget::setLogLevel(QString mn, int l)
{
	#ifdef QT_DEBUG
	 qDebug("CGexLogWidget::setLogLevel: set %s to %d...", mn.toLatin1().data(), l);
	#endif
	QMenu* m=m_llpb.menu();
	if (!m)
		return false;
	foreach(QAction* a, m->actions())
	{
		if (a->text()!=mn)
			continue;

		if (!a->menu())
			continue;
		if (l<a->menu()->actions().size())
			a->menu()->actions().at(l)->setChecked(true);
	}

	return true;
}

CGexLogWidget::~CGexLogWidget()
{
	s_pWidget=0;
	//qDebug("CGexLogWidget::~CGexLogWidget()");
}

/*
QString CGexLogWidget::AddLogLevelMenu(const QString &modulename, int currentlevel)
{
	#ifdef QT_DEBUG
	 qDebug("CGexLogWidget::AddLogLevelMenu module %s level %d", modulename.toLatin1().data(), currentlevel);
	#endif

	return "ok";
}
*/

QString CGexLogWidget::AppendMessage(const SMessage &sm)
{
	int y=m_currentRow++; // m_tw.rowCount()

	if (m_tw.rowCount()<m_currentRow)
		m_tw.setRowCount(m_currentRow);

	m_tw.scrollToBottom();	//setCurrentIndex();

	QTableWidgetItem* twi=GetItem(0, y);
	if (twi)
		twi->setData(Qt::DisplayRole, sm.m_time.toString());

	twi=GetItem(1, y);
	if (twi)
    {
        if (s_LevelsStrings.size()>sm.m_sev)
            twi->setData(Qt::DisplayRole, s_LevelsStrings.at(sm.m_sev));
        else
            twi->setData(Qt::DisplayRole, sm.m_sev);
        QColor c; // QColor()
        int s=sm.m_sev;
        int w=s*50; if (w>255) w=255;
        c.setRgb(255, w, w); // 255 0 0 = red     255 255 255=white
        twi->setBackground(QBrush( c )); // 0 0 0=black
        //twi->setForeground(QBrush(QColor((7-sm.m_sev)*36, 150, 150, 255)));
    }

	twi=GetItem(2, y);
	if (twi)
		twi->setData(Qt::DisplayRole, sm.m_atts["module"]);

	twi=GetItem(3, y);
	if (twi)
		twi->setData(Qt::DisplayRole, sm.m_atts["msg"]);

	twi=GetItem(4, y);
	if (twi)
		twi->setData(Qt::DisplayRole, sm.m_atts["func"]);

	twi=GetItem(5, y);
	if (twi)
		twi->setData(Qt::DisplayRole, sm.m_atts["file"]);

	//m_tw.adjustSize();
	m_tw.resizeColumnsToContents();
  m_tw.resizeRowToContents(y);
  //m_tw.setBackgroundColor();

	return "ok";
}

void CGexLogWidget::onCellDoubleClicked(int row, int col)
{
	#ifdef QT_DEBUG
   //qDebug(" %d %d doubleClicked", row,col);
	#endif
	//if (y==0)
		//m_tw.sortItems(x);
	m_tw.resizeColumnToContents(col);
	//m_tw.rowResized();
	row++;
}

void CGexLogWidget::onLogLevelActionTriggered(QAction* a)
{
	if (!a)
		return;

	#ifdef QT_DEBUG
	 qDebug("CGexLogWidget::onLogLevelActionTriggered : menu %s : %d",
		 a->parentWidget()?((QMenu*)a->parentWidget())->title().toLatin1().data():"?",
		 a->actionGroup()?a->actionGroup()->actions().indexOf(a):-1
			);
	#endif

	if (a->parentWidget() && a->actionGroup())
	{
		QString t=((QMenu*)a->parentWidget())->title();
		if (s_LogLevels.contains(t))
		{
			//int ll=s_LogLevels[a->menu()->title()];
			s_LogLevels.insert(t, a->actionGroup()->actions().indexOf(a));
		}
		else
		{
			#ifdef QT_DEBUG
			 qDebug(" %s not found", t.toLatin1().data());
			#endif
		}
	}
}

QTableWidgetItem*
CGexLogWidget::GetItem(int x, int y)
{
	QTableWidgetItem* twi=m_tw.item( y, x );
	if (!twi)
	{
		twi=new QTableWidgetItem();
		twi->setTextAlignment(Qt::AlignLeft); //twi->setTextAlignment(Qt::AlignHCenter);
		twi->setFlags( twi->flags() & ~Qt::ItemIsEditable);
		twi->setFlags( twi->flags() | Qt::ItemIsSelectable);
		twi->setFlags( twi->flags() | Qt::ItemIsEnabled );
		m_tw.setItem(y, x, twi);
	}
	return twi;
}
