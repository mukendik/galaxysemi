
#include <QApplication>

#include <QPainter>
#include <QImage>
#include <QDir>

//#include "gqtl_log.h"
#include "gqtl_svgrenderer.h"

///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include <gqtl_service.h>

class POCService : public QtServiceBase
{
public:
    POCService(int argc, char **argv, const QString &name)
        : QtServiceBase(argc, argv, name)
    {
        QtServiceBase::setServiceDescription(name);
        QtServiceBase::setServiceFlags(QtServiceBase::CanBeSuspended);
        QtServiceBase::setStartupType(QtServiceController::AutoStartup);
    }
    ~POCService()
    {
    }

protected:

    void	start()
    {
        QPainter    	myPainter;
        QImage      	myImage(400, 400, QImage::Format_ARGB32);
        QFile           myLog(qApp->applicationDirPath() + QDir::separator() + "log.txt");

        if (myLog.open(QIODevice::ReadWrite))
        {
            myLog.write(QDir::homePath().toLatin1()+ "\n");

            if (myPainter.begin(&myImage))
            {
                myPainter.drawText(10, 10, 300, 50, Qt::AlignLeft | Qt::AlignVCenter, "My painter works from within a service!!");
                myPainter.end();

                QString imagePath = qApp->applicationDirPath() + QDir::separator() + "myImage.png";

                myLog.write(imagePath.toLatin1() + "\n");

                if (myImage.save(imagePath, "PNG") == false)
                    myLog.write("failed to create image\n");
                else
                    myLog.write("image created\n");
            }

            QString svgfilepath = qApp->applicationDirPath() + QDir::separator() + "mySVGImage.svg";
            myLog.write(svgfilepath.toLatin1() + "\n");
            QFile   file(svgfilepath);
            QImage image(400, 400, QImage::Format_RGB32);
            if (file.open(QIODevice::ReadOnly ))
            {
                // using QSvg library
                QSvgRenderer renderer(svgfilepath);
                if (!renderer.isValid())
                    myLog.write("QSvgRenderer is NOT Valid");

                QPainter painter(&image);

                painter.setRenderHint(QPainter::Antialiasing, false);

                renderer.render(&painter);

                file.close();
            }
            else
                myLog.write("Failed to open output svg file");

            if (image.save(qApp->applicationDirPath() + QDir::separator()  + "waferzone.bmp") == false)
                myLog.write("Failed to save bmp output");


            myLog.close();
        }



        //GSLOG(SYSLOG_SEV_ERROR, QDir::homePath().toLatin1().data());
    }

    virtual void createApplication(int &argc, char **argv)
    {
#if defined (PAINTER_AS_SERVICE)
        new QGuiApplication(argc, argv);
#else
        new QApplication(argc, argv);
#endif
    }

    virtual int executeApplication()
    {
        return qApp->exec();
    }

private:
//    QApplication *app;
};


int main(int argc, char **argv)
{
#ifdef PAINTER_AS_SERVICE

    POCService pocService(argc, argv, "PocQPainter");

    return pocService.exec();
#else

    QApplication myApp(argc, argv, false);

    QPainter    	myPainter;
    QImage      	myImage(400, 400, QImage::Format_ARGB32);
    QFile           myLog(qApp->applicationDirPath() + QDir::separator() + "log.txt");

    if (myLog.open(QIODevice::ReadWrite))
    {
        myLog.write(QDir::homePath().toLatin1()+ "\n");

        if (myPainter.begin(&myImage))
        {
            myPainter.drawText(10, 10, 300, 50, Qt::AlignLeft | Qt::AlignVCenter, "My painter works from within a service!!");
            myPainter.end();

            QString imagePath = qApp->applicationDirPath() + QDir::separator() + "myImage.png";

            myLog.write(imagePath.toLatin1() + "\n");

            if (myImage.save(imagePath, "PNG") == false)
                myLog.write("failed to create image\n");
            else
                myLog.write("image created\n");
        }

        QString svgfilepath = qApp->applicationDirPath() + QDir::separator() + "mySVGImage.svg";
        myLog.write(svgfilepath.toLatin1() + "\n");
        QFile   file(svgfilepath);
        QImage image(400, 400, QImage::Format_RGB32);
        if (file.open(QIODevice::ReadOnly ))
        {
            // using QSvg library
            QSvgRenderer renderer(svgfilepath);
            if (!renderer.isValid())
                myLog.write("QSvgRenderer is NOT Valid");

            QPainter painter(&image);

            painter.setRenderHint(QPainter::Antialiasing, false);

            renderer.render(&painter);

            file.close();
        }
        else
            myLog.write("Failed to open output svg file");

        if (image.save(qApp->applicationDirPath() + QDir::separator()  + "waferzone.bmp") == false)
            myLog.write("Failed to save bmp output");


        myLog.close();
    }


    return 0;
#endif


}

