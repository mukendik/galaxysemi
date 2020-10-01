#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrlQuery>
#include <QCoreApplication>

#include "download_license.h"
#include "gqtl_log.h"

namespace GS
{
namespace LPPlugin
{

const char *sIsBadLicense = "Invalid Order ID";
const char *sLicenseStartsWith = "# License for";

DownloadLicense::DownloadLicense(const QString &url, const QString &fileName,QObject *parent):
    QObject(parent),
    mDownloadFileName(fileName),
    mDownloadURL(url)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Constructor ...");
}

DownloadLicense::~DownloadLicense()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Destructor ...");
}

DownloadLicense::DownloadLicenseError DownloadLicense::DownloadLicenseFile(QString &licContent, QString &message)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start to download file at url: %1 and save it into: %2")
          .arg(mDownloadURL).arg(mDownloadFileName).toLatin1().constData());


    QUrl lDownloadUrl;
    lDownloadUrl.setUrl(mDownloadURL);

    GSLOG(SYSLOG_SEV_DEBUG, QString("lDownloadUrl.Url: %1").arg(lDownloadUrl.toString()).toLatin1().constData());

    //Start the request with the URL specified.
    QNetworkRequest lRequest(lDownloadUrl);
    //Save the network reply returned by the manager
    QNetworkAccessManager lManager;
    QNetworkReply *lCurrentDownload = lManager.get(lRequest);
    //Download status when trying to download the requested file
    DownloadLicenseError lDownloadStatus = DOWNLOAD_NO_ERROR;
    while(lCurrentDownload)
    {
        if(lCurrentDownload->isFinished())
        {
            //Download finished
            QUrl lReplyUrl = lCurrentDownload->url();
            GSLOG(SYSLOG_SEV_DEBUG, QString("Downloading %1 finished").arg(lReplyUrl.toString()).toLatin1().constData());
            if (lCurrentDownload->error() != QNetworkReply::NoError)
            {
                //Error with network problem
                GSLOG(SYSLOG_SEV_DEBUG, QString("Download of (%1) failed with error %2: %3")
                      .arg(lReplyUrl.toString())
                      .arg(lCurrentDownload->error())
                      .arg(lCurrentDownload->errorString()).toLatin1().constData());
                message = QString("Can not download the license file due to a network error %1: %2")
                        .arg(lCurrentDownload->error())
                        .arg(lCurrentDownload->errorString());
                lDownloadStatus = DOWNLOAD_NETWORK_ERROR;
            }
            else
            {
                QByteArray lData = lCurrentDownload->readAll();
                licContent = lData;
                GSLOG(SYSLOG_SEV_DEBUG, QString("Reply data : %1").arg(QString(lData)).toLatin1().constData());
                //Check flexera license content
                if(!CheckReplyContent(lData,message))
                {
                    lDownloadStatus = DOWNLOAD_LICENSE_ERROR;
                }
                else
                {
                    //Check the file
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Saving file into %1").arg(mDownloadFileName).toLatin1().constData());
                    if (SaveFile(lData, message))
                    {
                        GSLOG(SYSLOG_SEV_DEBUG, "Saving file succeed");
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_DEBUG, "Saving file failed");
                        lDownloadStatus = DOWNLOAD_FILE_ERROR;

                    }
                }
            }
            break;
        }
        qApp->QCoreApplication::processEvents();
    }

    if(lCurrentDownload)
        lCurrentDownload->deleteLater();
    return lDownloadStatus;
}


bool DownloadLicense::CheckReplyContent(const QByteArray &data, QString &message)
{
    GSLOG(SYSLOG_SEV_DEBUG, "CheckReplyContent ...");

    QString lLicense = data;
    //Raise an error if the license is empty or not valid
    if(lLicense.isEmpty() || lLicense.contains(sIsBadLicense) || (!lLicense.startsWith(sLicenseStartsWith)))
    {
        message = "Bad license file content";
        return false;
    }
    else
    {
        return true;
    }
}

bool DownloadLicense::SaveFile(const QByteArray &data, QString &message)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("SaveFile ... %1").arg(mDownloadFileName).toLatin1().constData());
    //Check if the file Exist and change the name if needed.

    QFile lFile(mDownloadFileName);
    if (!lFile.open(QIODevice::WriteOnly))
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Could not open %1 for writing: %2\n")
                                .arg(mDownloadFileName).arg(lFile.errorString()).toLatin1().constData());
        message = QString("Could not open %1 for writing %2: %3\n").arg(mDownloadFileName).arg(lFile.error()).arg(lFile.errorString());
        return false;
    }

    QByteArray lData = data;

    lFile.write(lData);
    lFile.close();

    return true;
}

QString DownloadLicense::GenerateFileName(const QString &initFileName)
{
    QString lFileName = initFileName;

    if(lFileName.isEmpty())
    {
        lFileName = QDir::homePath() + QDir::separator() + "eval_license.lic";
    }

    //Check if the file name exist and increment to make it unique.
    QString lPath = QFileInfo(lFileName).path();
    QString lName = QFileInfo(lFileName).baseName();
    QString lExtension = QFileInfo(lFileName).completeSuffix();

    int lIncr = 0;
    QString lNewFileName = lPath + QDir::separator() + lName + "_" + QString::number(lIncr) + "." + lExtension;
    if (QFile::exists(lFileName))
    {
        // already exists, don't overwrite
        while (QFile::exists(lNewFileName))
        {
            lNewFileName = lPath + QDir::separator() + lName + "_" + QString::number(++lIncr) + "." + lExtension;
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Returned name %1").arg(lNewFileName).toLatin1().constData());
    return lNewFileName;
}

}
}
