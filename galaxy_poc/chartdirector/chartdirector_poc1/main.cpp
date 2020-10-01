#include <QtCore/QCoreApplication>

//#include <QApplication>
#include <QPainter> // needs Qt GUI
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPicture>
#include <QPrinter>
#include <QSvgGenerator>
#include <QTextDocument>
#include <QTextStream>
#include <QFile>
#include <QSvgRenderer>
#include <QImage>

//#include <QPixmap>
//#include <QtGui>

#include <chartdir.h>


class QChartDirPaintEngine : public QPaintEngine
{
    QChartDirPaintEngine() {}
    bool begin(QPaintDevice* pd) { return false; }
    bool end() { return false; }
    void updateState(const QPaintEngineState &state) {}
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr) { }
    Type type() const { return (Type)50; }
};

int main(int argc, char *argv[])
{
    // CoreApp or App ?
    QCoreApplication a(argc, argv);
    //QApplication a(argc, argv, false);

    qDebug("hello");

    QChartDirPaintEngine cdpe();

    //QWidget w; // no way

    qDebug("Painting to a QImage ? QPicture ? QPixmap ? QSvgGenerator ? ...");
    QSvgGenerator svgg;
        svgg.setFileName("wm.svg");
    QImage i;
    QPicture picture;
    //QPixmap pix(200,200); // no way
    QPrinter pr;
        pr.setOutputFileName("wm.pdf");

    QPainter p  (&svgg); // on image, picture, pixmap, svggenerator,... ?
        qDebug("Painter active ? %s", p.isActive()?"true":"false");

    QPaintDevice* pd=p.device();
    qDebug("PaintDevice = %p", pd);
    if (pd)
        qDebug("PaintDevice: type=%d depth=%d", pd->devType(), pd->depth());

    QPaintEngine *pe=p.paintEngine();
    qDebug("PaintEngine=%p", pe);
    if (pe)
        qDebug("PaintEngine type : %d", pe->type());

    //p.begin(&svgg);
        p.drawText( QPoint(10,10), "It works");
        p.drawLine(0,0, 200, 200);
    p.end();

    // test : print html with a svg into a pdf
    /*
        QTextDocument doc;
        QFile f("svg.html");
        f.open(QIODevice::ReadOnly);
        QTextStream ts(&f);
        doc.setHtml(ts.readAll());
        f.close();
        pr.setOutputFileName("wm.pdf");
        doc.print(&pr); // will output : "cannot intantiate QPixmap in non GUI app"
    */

    QSvgRenderer svgr(QString("wm.svg"));
    qDebug("SvgRenderer valid=%d", svgr.isValid());
        QImage svgi;
        QPainter svgp(&svgi);
        qDebug("SVG Painter active : %d", svgp.isActive() );
        if (svgp.isActive()) // wont be active in non GUI app
        {
            svgr.render(&svgp);
            svgi.save("wm_from_svg.png");
        }

    //bool b=picture.save("wm.svg"); // gif bmp png
    //qDebug("save : %d", b);

    MultiChart mc(800, 140);

    return a.exec();
}
