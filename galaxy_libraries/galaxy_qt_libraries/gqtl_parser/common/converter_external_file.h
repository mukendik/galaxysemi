#ifndef CONVERTER_EXTERNAL_FILE_H
#define CONVERTER_EXTERNAL_FILE_H

#include <QDomDocument>

class ConverterExternalFile
{
public:

    // IN: strPath contains the location where the converter_external_file.xml is stored
    static bool Exists(const QString& aPath);

    // IN: strPath contains the location where the converter_external_file.xml is stored
    // IN: strType must be "final" or "wafer"
    // IN: strMode must be "prod" or "eng"
    // OUT: strErrorMsg contains the error message is any
    // OUT: strFileName and strFileFormat contain file path and format information
    static bool GetPromisFile(QString strPath, QString strType,
                              QString &strFileName, QString &strFileFormat,
                              QString &strErrorMsg, QString strCategory="");

    static bool GetPromisFile(QString strPath, QString strType, QString strMode,
                              QString &strFileName, QString &strFileFormat,
                              QString &strErrorMsg, QString strCategory="");

    // IN: strPath contains the location where the converter_external_file.xml is stored
    // IN: strType must be "final" or "wafer"
    // OUT: strErrorMsg contains the error message is any
    // OUT: strFileName and strFileFormat contain file path and format information
    static bool GetBinmapFile(QString strPath, QString strType,
                              QString &strFileName, QString &strFileFormat,
                              QString &strErrorMsg);

    static bool GetBinmapFile(QString strPath, QString strType, QString strMode,
                              QString &strFileName, QString &strFileFormat,
                              QString &strErrorMsg);

    static bool GetBinmapFile(QString strPath, QString strType, QString strMode, QString strCategory,
                              QString &strFileName, QString &strFileFormat,
                              QString &strErrorMsg);
    static QStringList LoadAttributs(const QString& aExternalFilPath, const QString& aAttribut, QString& lErrorMsg);
    static QStringList LoadOptionnalDefaultVals(const QString& aExternalFilPath, QString& aStrErrorMsg);

    static bool DecryptLot(const QString &aCryptedLot, QString &aLot, int &aWaferNumber, QString& aErrorMsg);

    static QString GetExternalFileName(QString strFilePath);

private:

    static bool OpenExternalDomFile(QString strFileName, QDomDocument &doc, QString &strErrorMsg);

    static bool GetExternalFileInfo(QDomElement docElem, QString strData, QString strType, QString strMode, QString strCategory,
                                    QString &strFile, QString &strFormat,
                                    QString &strErrorMsg);
};

#endif // CONVERTER_EXTERNAL_FILE_H

