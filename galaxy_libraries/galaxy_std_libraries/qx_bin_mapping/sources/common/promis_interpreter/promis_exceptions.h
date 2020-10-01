#ifndef PROMIS_EXCEPTIONS_H
#define PROMIS_EXCEPTIONS_H

#include "bin_mapping_api.h"

#include <exception>
#define MARGUMENTSSIZE_PROMIS_EXCEPTIONS 128

namespace Qx
{
namespace BinMapping
{

struct InvalidPromisFilePath : public std::exception
{
    InvalidPromisFilePath( const char *aConverterExternalFilePath );

    const char * what() const throw();

private :
    char mConverterExternalFilePath[ 256 ];
};

struct CannotOpenPromisFile : public std::exception
{
    CannotOpenPromisFile( const char *aPromisFilePath, const char *aConverterExternalFilePath );

    const char * what() const throw();

private :
    char mPromisFilePath[ 256 ];
    char mConverterExternalFilePath[ 256 ];
};

struct PromisKeyCannotBeEmpty: public std::exception
{
    const char * what() const throw() { return "Promis key cannot be empty"; }
};

struct PromisItemNotFound: public std::exception
{
    PromisItemNotFound( const char *aKey, const char *aPromisFilePath, const char *aConverterExternalFilePath );

    const char * what() const throw();

private :
    char mKey[ 64 ];
    char mPromisFilePath[ 256 ];
    char mConverterExternalFilePath[ 256 ];
};

class InvalidLvmFtSubconDataPromisFileFormat: public std::exception
{
public :
    InvalidLvmFtSubconDataPromisFileFormat( const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS ];
};

class InvalidLvmFtFieldNbPromisFileFormat: public std::exception
{
public :
    InvalidLvmFtFieldNbPromisFileFormat(const char *aPromisPath, const char* aExtFilePath, const int aMinFieldsNb);

    const char * what() const throw();

private :
    char mPromisPath[ 256 ];
    char mExtFilePath[ 256 ];
    int mMinFieldsNb;
};

class InvalidLvmFtLotlistPromisFileFormat : public std::exception
{
public :
    InvalidLvmFtLotlistPromisFileFormat( const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidLvmWtPromisFileFormat: public std::exception
{
public :
    InvalidLvmWtPromisFileFormat( const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidHvmWtPromisFileFormat: public std::exception
{
public :
    InvalidHvmWtPromisFileFormat( const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidLvmFtPromisFileFormat: public std::exception
{
public :
    InvalidLvmFtPromisFileFormat( const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidHvmFtPromisFileFormat: public std::exception
{
public :
    InvalidHvmFtPromisFileFormat( const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

}
}

#endif // PROMIS_EXCEPTIONS_H
