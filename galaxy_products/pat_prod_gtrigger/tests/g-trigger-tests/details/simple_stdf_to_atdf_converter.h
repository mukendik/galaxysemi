#ifndef SIMPLE_STDF_TO_ATDF_CONVERTER_H
#define SIMPLE_STDF_TO_ATDF_CONVERTER_H

#include "stdfparse.h"

#include <QString>
#include <QScopedPointer>

class QTextStream;
class QFile;

class SimpleStdfToAtdfConverter
{
public :
    SimpleStdfToAtdfConverter( const QString &aStdfFilePath, const QString &aAtdfFilePath );

    void RunConversion();

private :
    void OpenFilesAndAcquireStream();
    void AcquireAtdfFileStream();
    void InitializeStdfParser();
    void Convert();
    void PushAtdfRecordInAtdfFileStream(int aRecordType );
    void WriteAtdfRecordInStream( GQTL_STDF::Stdf_Record *aRecord );

private :
    QString mStdfFilePath;
    QString mAtdfFilePath;
    QScopedPointer< QTextStream > mAtdfFileStream;
    QScopedPointer< QFile > mAtdfFile;
    GQTL_STDF::StdfParse mStdfParser;
};

#endif // SIMPLE_STDF_TO_ATDF_CONVERTER_H
