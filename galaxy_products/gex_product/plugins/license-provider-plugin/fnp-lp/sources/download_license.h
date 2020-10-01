#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#ifndef DOWNLOAD_LICENSE_H
#define DOWNLOAD_LICENSE_H

namespace GS
{
namespace LPPlugin
{
/*!
    \class DownloadLicense
    \brief The DownloadLicense class is the class used to connect to an the URL
    and then will generate the flexera file based license.
    (implemented for the GCORE-186/ GCORE-330)
*/
class DownloadLicense: public QObject
{
    Q_OBJECT
public:
    enum DownloadLicenseError
    {
        DOWNLOAD_NO_ERROR      ,// No error during the download
        DOWNLOAD_NETWORK_ERROR ,// A network error occured during the download
        DOWNLOAD_FILE_ERROR    ,// An error occured when saving the file
        DOWNLOAD_LICENSE_ERROR // An error ocurred during the communication with the license portal
    };

    /// \brief Constructor
    DownloadLicense(const QString &url, const QString &fileName,QObject *parent=0);
    /// \brief Destructor
    ~DownloadLicense();
    /// \brief entry point of the object to start downloading a lic file.
    ///  This method will start the Request/Reply to download a file from the specified url
    /// licContent will store the license content if the saving does not work
    /// message is used to store an error message if it occurs and will return a error status.
    DownloadLicenseError DownloadLicenseFile(QString &licContent, QString &message);
    /// \brief static function to generate a unique filename based on the initFileName provided.
    static QString GenerateFileName(const QString &initFileName);
private:
    /// \brief method to save the data returned by the url to a filename and store an error message if it occured
    /// return true if OK false if not
    bool SaveFile(const QByteArray &data, QString &message);
    /// \brief Check the license reply content of the data and store an error message if it occured
    /// return true if OK false if not
    bool CheckReplyContent(const QByteArray &data, QString &message);
    ///< Store the the download filename.
    QString mDownloadFileName;
    ///< Store the the download url location.
    QString mDownloadURL;
};
}
}

Q_DECLARE_METATYPE(GS::LPPlugin::DownloadLicense::DownloadLicenseError);

#endif // DOWNLOAD_LICENSE_H
