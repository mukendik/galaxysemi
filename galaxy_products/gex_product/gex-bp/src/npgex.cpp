/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

#include <QtGui>
#include "qhttp.h"
#include "qtbrowserplugin.h"
#include <QLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QtWidgets>
#ifdef QAXSERVER
#warning  AXSERVER
#include <ActiveQt/QAxBindable>
#include <ActiveQt/QAxFactory>
#include <qt_windows.h>
#include <Urlmon.h>
#endif


void gexlog(const char* s)
{
	if (!s)
		return;
	FILE *h=fopen("gexlog.txt","at");
	if (!h)
	{
		h=fopen("gexlog.txt","wt");
		if (!h)
			return;
	}
	fprintf(h,s);
	fclose(h);
}


class GexBPWidget : public QWidget, public QtNPBindable
#ifdef QAXSERVER
            , public QAxBindable
#endif
{
    Q_OBJECT
    Q_ENUMS(GraphStyle)
    Q_PROPERTY(GraphStyle graphStyle READ graphStyle WRITE setGraphStyle)
    Q_PROPERTY(QString src READ dataSourceUrl WRITE setDataSourceUrl)

    Q_CLASSINFO("ClassID", "{2e5b2715-46b2-4831-ba9b-6a3b195d5ec8}")
    Q_CLASSINFO("InterfaceID", "{94581136-3c0c-46cc-97a1-066061356d43}")
    Q_CLASSINFO("EventsID", "{8c191b77-1894-45c7-9d6b-201dede95410}")

	Q_CLASSINFO("MIME", "application/gex:g1n:Graphable ASCII numeric data")
public:
	GexBPWidget(QWidget *parent = 0);
	~GexBPWidget();

    enum GraphStyle
    {
		Bar, Pie
    };
    void setGraphStyle(GraphStyle style);
    GraphStyle graphStyle() const;

    void setDataSourceUrl(const QString &url);
    QString dataSourceUrl() const;

    bool readData(QIODevice *source, const QString &format);
    bool writeData(QIODevice *sink);

    void transferComplete(const QString &url, int id, Reason r);

protected:
    void timerEvent(QTimerEvent*);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *me);

    void paintWait();
    void paintBar(QPaintEvent*);
    void paintPie(QPaintEvent*);

private slots:
    void stylePie();
    void styleBar();
    void aboutPlugin();
    void aboutData();
    void aboutQt();
    void aboutQtDevelopmentFrameworks();
	void Finished(int requestId, bool error)
	{
		gexlog( QString("GexBPWidget::Finished: request %d\n").arg(requestId).toLatin1().data() );
		if (Request==requestId)
		{
			gexlog((char*)"GexBPWidget::Finished:\n");
			if (error)
			{
				gexlog("GexBPWidget::Finished: but error : ");
				gexlog(m_http->errorString().toLatin1().data());
				gexlog("GexBPWidget::Finished: retry...\n");
				Request=m_http->get(m_url->path(), buffer);
				return;
			}
			else
			{
				img.loadFromData(bytes);
				gexlog(QString(" %1 %2 \n").arg(img.size().width()).arg(img.size().height()).toLatin1().data());
				m_tw->insertTab(10,
								new QLabel("GalaxySemi is simply the best.", this),
								QIcon(QPixmap::fromImage(img)),
								"About"
								);
			}
		}
	}

private:
	QTabWidget* m_tw;
	QImage img;
	QBuffer *buffer;
	QByteArray bytes;
	QHttp *m_http;
	QUrl* m_url;
	int Request;

    struct Datum {
        double value;
        QString label;
    };
    QList<Datum> data;
    void processData(QTextStream &in);

    QMenu *menu;
    QStatusBar *statusbar;
    QAction *pie, *bar;

    int pieRotation;
    int pieTimer;
    GraphStyle m_style;

    // Developer information for the About Data dialog:
    QString sourceUrl;
    int lastReqId, lastConfId;
    QString lastConfUrl;
    Reason lastConfReason;
};

GexBPWidget::GexBPWidget(QWidget *parent)
: QWidget(parent), pieRotation(0), pieTimer(0)
{
	gexlog((char*)"GexBPWidget::GexBPWidget\n");

	setLayout(new QVBoxLayout(this));

	//QUrl u("http://localhost/galaxysemibanner.jpg"); u.setPort(8080);
	m_url=new QUrl("http://galaxysemi.com/images/galaxy-logo.gif"); m_url->setPort(80);
	//m_url=new QUrl("http://static.ecosia.org/img/sprite_start_banner.jpg"); m_url->setPort(80);
	//QUrl u("http://galaxyec7.com/helpconsole2010/GalaxyUserAssistance/images/Galaxy%20Logo.png");
	if (!m_url->isValid())
		gexlog("GexBPWidget::GexBPWidget: URL not valid !");
	m_http=new QHttp(this);
	connect(m_http, SIGNAL(requestFinished(int, bool)), this, SLOT(Finished(int, bool)));
	buffer = new QBuffer(&bytes);
	buffer->open(QIODevice::WriteOnly);
	m_http->setHost(m_url->host(), m_url->port());
	Request=m_http->get(m_url->path(), buffer);

	//layout()->addWidget(new QImage())
	layout()->addWidget(new QLabel("Welcome to GalaxySemi BrowserPlugin", this));

	setStyleSheet("background-image:url(http://galaxysemi.com/images/galaxy-logo.gif)");

	m_tw=new QTabWidget(this);
	m_tw->setWindowOpacity((qreal)0.5);
	//m_tw->setStyleSheet("background-image:url(http://galaxysemi.com/images/galaxy-logo.gif)");
	m_tw->insertTab(0, new QWidget(this), "Home");
	m_tw->insertTab(1, new QWidget(this), "Data");
	m_tw->insertTab(2, new QWidget(this), "Settings");
	m_tw->insertTab(3, new QWidget(this), "Report");
	m_tw->insertTab(4, new QWidget(this), "Options");
	m_tw->insertTab(5, new QWidget(this), "Help");
	layout()->addWidget(m_tw);

	//layout()->addWidget(new QLabel("Welcome to GalaxySemi BrowserPlugin", this));

    menu = new QMenu(this);
    QMenu *styles = menu->addMenu("&Styles");

    pie = styles->addAction("&Pie",this,SLOT(stylePie()));
    pie->setShortcut(QString("Ctrl+P"));
    pie->setCheckable(true);

    bar = styles->addAction("&Bar", this, SLOT(styleBar()));
    bar->setShortcut(QString("Ctrl+B"));
    bar->setCheckable(true);

    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);

    group->addAction(pie);
    group->addAction(bar);

    QMenu* help = menu->addMenu("&Help");

	help->addAction("About GalaxySemi &plugin...", this, SLOT(aboutPlugin()))->setShortcut(QString("Ctrl+A"));

	//help->addAction("About &data...", this, SLOT(aboutData()));
    help->addAction("About &Qt...", this, SLOT(aboutQt()));
	//help->addAction("About Qt &Development Frameworks...", this, SLOT(aboutQtDevelopmentFrameworks()));

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+F5"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(aboutQt()));

    statusbar = new QStatusBar(this);

    setFocusPolicy(Qt::StrongFocus);
	setGraphStyle(Bar);

    QPalette pal = palette();
    pal.setColor(QPalette::Window,Qt::white);
    setPalette(pal);

    lastReqId = 0;
    lastConfId = 0;
    lastConfReason = ReasonUnknown;
}

GexBPWidget::~GexBPWidget()
{
	if (m_url)
		delete m_url;
}

void GexBPWidget::setGraphStyle(GraphStyle style)
{
    if (pieTimer)
        killTimer(pieTimer);

    m_style = style;
    switch(m_style) {
    case Pie:
        pieTimer = startTimer(50);
        pie->setChecked(true);
        break;
    case Bar:
        bar->setChecked(true);
        break;
    }

    repaint();
}

GexBPWidget::GraphStyle GexBPWidget::graphStyle() const
{
    return m_style;
}

void GexBPWidget::setDataSourceUrl(const QString &url)
{
    sourceUrl = url;

#ifdef QAXSERVER
    // IE does not call our readData() if plugin is fullpage, so do it here
	if (QAxFactory::isServer() && clientSite())
	{
        TCHAR cFileName[512];
        if (URLDownloadToCacheFile(0, url.utf16(), cFileName, 512, 0, 0) == S_OK)
            readData(&QFile(QString::fromUtf16(cFileName)), QString());
    }
#endif
}

QString GexBPWidget::dataSourceUrl() const
{
    return sourceUrl;
}

void GexBPWidget::aboutPlugin()
{
    openUrl("http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtbrowserplugin/");
}

void GexBPWidget::aboutData()
{
    QString page = parameters().value("datapage").toString();
    if (!page.isEmpty()) {
	openUrl(page);
    } else {
        QByteArray table;
        table += "<b>Data loaded from " + sourceUrl.toLatin1() + "</b>\n";
        table += "<p>This data has been loaded with streammode = '";
        table += parameters().contains("streammode") ? parameters().value("streammode").toByteArray() : QByteArray("Default");
        table += "'</p>\n";
        table += "<table>\n";
        for (int i = 0; i < data.count(); ++i) {
            Datum datum = data.at(i);
            table += "<tr><td>" + datum.label + "</td><td>" + QString::number(datum.value) + "</td></tr>\n";
        }
        table += "</table>\n";

        table += "<p><b>OpenURL() API usage information:</b>\n";
        table += "<br>Last OpenURL() request id: " + QString::number(lastReqId);
        table += "<br>Last confirmation id: " + QString::number(lastConfId);
        table += " Reason: " + QString::number((int)lastConfReason);
        table += "<br>URL: " + lastConfUrl;
        table += "</p>\n";
        QMessageBox::information(this, "Data information", QLatin1String(table));
    }
}

void GexBPWidget::transferComplete(const QString &url, int id, Reason r)
{
	gexlog("GexBPWidget::transferComplete\n");
    lastConfId = id;
    lastConfUrl = url;
    lastConfReason = r;
}

void GexBPWidget::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void GexBPWidget::aboutQtDevelopmentFrameworks()
{
    lastReqId = openUrl("http://qt.nokia.com");
}

void GexBPWidget::stylePie()
{
    setGraphStyle(Pie);
}

void GexBPWidget::styleBar()
{
    setGraphStyle(Bar);
}

bool GexBPWidget::readData(QIODevice *source, const QString &/*format*/)
{
    if (!source->open(QIODevice::ReadOnly|QIODevice::Text))
        return false;

    data.clear();

    QTextStream in(source);
    processData(in);

    update();

    return true;
}

void GexBPWidget::processData(QTextStream &in)
{
    while (!in.atEnd()) {
        Datum datum;
        QString value;
        in >> value;
        in >> datum.label;
        bool ok;
        datum.value = value.toDouble(&ok);
        if (ok)
            data += datum;
    }
}

bool GexBPWidget::writeData(QIODevice *target)
{
    if (!target->open(QIODevice::WriteOnly|QIODevice::Text))
        return false;

    QTextStream out(target);
    foreach(Datum datum, data) {
        out << datum.value << "\t" << datum.label << endl;
    }

    return true;
}

void GexBPWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == pieTimer) {
        pieRotation = (pieRotation + 1) % 360;
        update();
    }
    QWidget::timerEvent(e);
}

void GexBPWidget::enterEvent(QEvent *)
{
	gexlog((char*)"GexBPWidget::enterEvent\n");
	statusbar->showMessage(QString("Welcome in Gex Browser Plugin [%1]").arg(mimeType()));
}

void GexBPWidget::leaveEvent(QEvent *)
{
    if (!QApplication::activePopupWidget()) {
        statusbar->clearMessage();
    }
}

void GexBPWidget::paintEvent(QPaintEvent* event)
{
	QPainter p1(m_tw->currentWidget());
	p1.drawImage(m_tw->currentWidget()->rect(), img);

	QPainter p(this);
	/*
	QFont sansFont("Helvetica [Cronyx]", 14);
	p.setFont(sansFont);
	p.drawText(rect(), Qt::AlignCenter, "Welcome to GalaxySemi BrowserPlugin");
	*/

	//p.drawImage(rect(), img);

	if (!data.count())
	{
		//paintWait();
	}
	else
	{
        switch (m_style) {
        case Pie:
			//paintPie(event);
            break;
        default:
			paintBar(event);
            break;
        }
    }
}

void GexBPWidget::mousePressEvent(QMouseEvent *me)
{
    menu->exec(me->globalPos());
}

void GexBPWidget::paintWait()
{
    QPainter p(this);
    p.drawText(rect(), Qt::AlignCenter, "Loading...");
}

void GexBPWidget::paintBar(QPaintEvent* event)
{
    const int count = data.count();
    double max = 0.0;
    for (int i = 0; i < count; ++i) {
        double value = data.at(i).value;
        if (value > max)
            max = value;
    }

	QPainter painter(m_tw->currentWidget());
	//QPainter painter(this);
    painter.setClipRect(event->rect());
    painter.save();
    painter.setWindow(0, qRound(max), count * 20, qRound(-max));
	painter.setViewport(20, 5, m_tw->currentWidget()->width() - 40, m_tw->currentWidget()->height() - 40);
    for (int i = 0; i < count; ++i) {
        double value = data.at(i).value;
		QColor c;
		c.setHsv((i * 255)/count, 255, 255);// rainbow effect
        painter.setBrush(c);
        painter.drawRect(i * 20, 0, 20, qRound(value));
    }
    painter.restore();
    painter.setClipRect(QRect());
}

void GexBPWidget::paintPie(QPaintEvent* event)
{
    const int count = data.count();
    double total = 0.0;

    for (int i = 0; i < count; ++i) {
        double value = data.at(i).value;
        total += value;
    }

    int apos = (pieRotation-90)*16;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(event->rect());
    QRect r(rect());
    r.adjust(10, 10, -10, -10);

    for (int i = 0; i < count; ++i) {
        double value = data.at(i).value;
	QColor c;
	c.setHsv((i * 255)/count, 255, 255);// rainbow effect
	painter.setBrush( c );

	int a = int((value * 360.0) / total * 16.0 + 0.5);
	painter.drawPie(r, -apos, -a);
	apos += a;
    }
}

#include "npgex.moc"

QTNPFACTORY_BEGIN("GalaxySemi Browser Plugin", "A Qt-based NSAPI plug-in from GalaxySemi");
	QTNPCLASS(GexBPWidget)
QTNPFACTORY_END()

#ifdef QAXSERVER
#include <ActiveQt/QAxFactory>

QAXFACTORY_BEGIN("{89ab08da-df8c-4bd0-8327-72f73741c1a6}", "{082bd921-0832-4ca7-ab5a-ec06ca7f3350}")
    QAXCLASS(Graph)
QAXFACTORY_END()
#endif
