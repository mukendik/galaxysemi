#include <QFile>
#include <QPainter>
#include "gqtl_svgrenderer.h"	//#include <QSvgRenderer>	// using our patched QSvg lib
#include <gqtl_log.h>

QString ConvertSvgToQImageUsingWebkit(QString svgfile, QImage& /*i*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("%1").arg( svgfile.replace('\n',' ')).toLatin1().constData());

	/* Webkit solution
	QGraphicsWebView view;	// = new QGraphicsWebView;
	view.setResizesToContents(true);
	// view.load(url); // load is asynchronous !
	QString s;
	s.prepend("<html><body><object data=\"");
	s.append(svgfile);
	s.append("\" ");
	s.append(" type=\"image/svg+xml\" />");
	s.append("</body></html>");
	//QByteArray ba;
    //GSLOG(SYSLOG_SEV_DEBUG, QString("%1").arg( s).toLatin1().constData()));
	view.setHtml(s);
	//view.setContent(ba);
	// crispEdges or geometricPrecision
	const QString js = QString("document.getElementsByTagName(\"svg\")[0].style.shapeRendering = \"crispEdges\"");
	//QVariant v=view.page()->mainFrame()->evaluateJavaScript(js);
	QImage image(view.page()->viewportSize(), QImage::Format_RGB32);
	image.fill(-1); //white background
	QPainter p(&image);
	view.page()->mainFrame()->render(&p);
	//image.save("o.png");
	*/

	return "ok";
}


QString ConvertSvgToQImageUsingQSvgLib(QString svgfilepath, QImage& i)
{
	QFile f(svgfilepath);
	if (!f.open(QIODevice::ReadOnly))
		return QString("error : file '%1' can't be opened").arg(svgfilepath);

	// using QSvg library
	QSvgRenderer svgr(svgfilepath);
  if (!svgr.isValid())
    GSLOG(SYSLOG_SEV_WARNING, " QSvgRenderer is NOT Valid" );

	/*
	QImage i(svgr.defaultSize(),
			 //QImage::Format_RGB32
			 QImage::Format_RGB16
			 //QImage::Format_Indexed8
			 );
	*/

	QPainter p(&i);
    GSLOG(SYSLOG_SEV_DEBUG, QString(" QPainter isActive %1").arg( p.isActive()).toLatin1().constData());

	p.setRenderHint(QPainter::Antialiasing, false);

	svgr.render(&p);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString(" colorCount = %1 numColors = %2")
          .arg( i.colorCount())
          .arg(i.colorCount() )).toLatin1().constData());

	f.close();

	return "ok";
}

