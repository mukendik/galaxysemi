#ifndef TESTABLE_PROMIS_INTERPRETER_H
#define TESTABLE_PROMIS_INTERPRETER_H

#include "promis_item_base.h"
using Qx::BinMapping::PromisItemBase;

#include <QString>
#include <QTest>

#include <fstream>
#include <vector>

struct SerializableToFile
{
    virtual ~SerializableToFile();

    virtual void SerializeToFile( const QString &aFilePath ) const = 0;
};

std::ofstream & operator << ( std::ofstream &aOutputFileStream, const PromisItemBase &aPromisItem );

template< typename PromisInterpreterType >
class TestablePromisInterpreter : public PromisInterpreterType, public SerializableToFile
{
public :
    TestablePromisInterpreter( const std::string &aKey,
                               const QString &aPromisInputFile,
                               const QString &aConvertExternalFilePath ) :
        PromisInterpreterType( aKey, aPromisInputFile.toStdString(), aConvertExternalFilePath.toStdString() ) {}

    void SerializeToFile( const QString &aFilePath ) const
    {
        std::ofstream lOutputFileStream( aFilePath.toStdString().c_str(), std::ios_base::out );
        if( ! lOutputFileStream.is_open() )
            QFAIL( QString( "Cannot open %1 file with write permissions" )
                   .arg( aFilePath ).toStdString().c_str() );

        lOutputFileStream << this->GetPromisItem() << '\n';
    }
};

#endif // TESTABLE_PROMIS_INTERPRETER_H
