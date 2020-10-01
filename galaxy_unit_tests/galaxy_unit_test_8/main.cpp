#include <QDir>
#include <QFile>

#include <time.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <gstdl_compat.h> // contains namespace : cannot be included in .c file
#include <gstdl_mailer.h>
#include <gstdl_systeminfo.h>
#include <gqtl_log.h>

int main()
{
    qDebug("GQTL log unit test...");

    qDebug("Home = '%s'", QDir::homePath().toLatin1().data());

    //QFile lXml();

    QString lXmlPath(QDir::homePath()+"/GalaxySemi/gslog.xml");

    qDebug("(HOME)/GalaxySemi/gslog.xml exists ? %s", QFile::exists(lXmlPath)?"yes":"no" );

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Unit test 8");

    return EXIT_SUCCESS;
}
