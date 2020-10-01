#ifndef GEX_IMPORT_ALL_H
#define GEX_IMPORT_ALL_H

#include <QObject>
#include "progressHandlerAbstract.h"
#include <QStringList>

// Galaxy modules includes
#include <gstdl_errormgr.h>

namespace GS
{
namespace Gex
{

class SelectDataFiles
{
public:
    QString		GetSingleFile(QWidget * parent,const QString &strPath, const QString &strCaption);
    QStringList GetFiles(QWidget * parent, const QString &strPath, const QString& strCaption);
};

class ProgressBarHandler : public GS::Parser::ProgressHandlerAbstract
{
public:
    ProgressBarHandler();
    void Start(const long minValue, const long maxValue);
    void SetValue(const long value);
    void Increment();
    void Finish();
    void SetMessage(const std::string message);
};


class ConvertToSTDF : public QObject
{
    Q_OBJECT

public:
    ConvertToSTDF(QObject* parent=0);
    ConvertToSTDF(const ConvertToSTDF&); // copy constructor needed for JS

    enum eConvertStatus
    {
        eConvertSuccess,		// Conversion successful
        eConvertWarning,		// Conversion successful with warning
        eConvertDelay,			// Delay conversion
        eConvertError			// Conversion failed
    };

    // Convert 'strFileName' to STDF 'strFileNameSTDF' file
    Q_INVOKABLE int	Convert(bool bMonitoring, bool bAllowWizard, bool bDatabaseAccessMode,
      bool bAllowExtendedCsv, QString strFileName, QString &strFileNameSTDF,
      QString strNewExtension, bool &bFileCreated, QString &strMessage,
      bool bAllowOnlyOneFile=true, bool bAllowDestFolderOverwriteFromOptions=true);

    // Convert 'strFileName' to STDF 'strFileNameSTDF' file
    Q_INVOKABLE int	Convert(bool bMonitoring, bool bAllowWizard, bool bDatabaseAccessMode,
      bool bAllowExtendedCsv, QString strFileName, QStringList &lstFileNameSTDF,
      QString strNewExtension, bool &bFileCreated, QString &strMessage,
      bool bAllowOnlyOneFile=false, bool bAllowDestFolderOverwriteFromOptions=true);

    // Check if the file is a GalaxySemi txt ToolBox converted file
    Q_INVOKABLE static QString IsGalaxySemiTxtFormat(const QString &f);
    Q_INVOKABLE static QStringList GetListOfSupportedFormat(bool bWithFormatDescription = false);

    //! \brief convert input to output
    Q_INVOKABLE QString Convert(const QString &lInput, const QString &lOutput);

};

}
}

#endif
