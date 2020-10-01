#include <QObject>

#include "parserAbstract.h"

class ParserTestExpectedResult;

namespace GQTL_STDF
{
    class Stdf_Record;
}

class TestGqtlParser: public QObject
{
    Q_OBJECT
private:
    bool ProcessRecordATDF(GQTL_STDF::Stdf_Record*);
    void ProcessTestResult( const QString &aErrorMessage,
                            GS::Parser::ConverterStatus aStatus,
                            const ParserTestExpectedResult &aExpectedResult ) const;

private slots:
    void convert_data();
    void convert();
};
