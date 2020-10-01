#ifndef BIN_MAPPING_EXCEPTIONS_H
#define BIN_MAPPING_EXCEPTIONS_H

#include <exception>
#include <cstring>
#include <string>
#include <cstdio>

namespace Qx
{
namespace BinMapping
{

class BinMappingExceptionForUserBase : public std::exception
{
public :
    virtual const char * what() const throw() = 0;

protected :
    BinMappingExceptionForUserBase( const char * const aBinMapFilePath,
                                    const char * const aConverterExternalFilePath );

protected:
    char mBinMapFilePath[256];
    char mConverterExternalFilePath[256];
};

class InvalidBinMapFilePath : public BinMappingExceptionForUserBase
{
public :
    InvalidBinMapFilePath( const char * const aBinMapFilePath,
                           const char * const aConverterExternalFilePath );
    const char * what() const throw();
};

class CannotOpenBinMappingFile : public BinMappingExceptionForUserBase
{
public :
    CannotOpenBinMappingFile( const char * const aBinMapFilePath,
                              const char * const aConverterExternalFilePath );
    const char * what() const throw();
};

template< typename BinMapItemKeyType >
class BinMapItemNotFound;

template<>
class BinMapItemNotFound< int > : public BinMappingExceptionForUserBase
{
public :
    BinMapItemNotFound( const char * const aBinMapFilePath,
                        const char * const aConverterExternalFilePath,
                        int aKey ) :
        BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath ),
        mKey( aKey ) {}

    const char * what() const throw()
    {
        static char lBuffer[ 256 + 256 + 11 ];

        std::sprintf( &lBuffer[ 0 ],
                      "Cannot find the bin map item key: '%d' "
                      "in the bin map file: '%s' "
                      "specified in: '%s",
                      mKey, mBinMapFilePath, mConverterExternalFilePath );

        return lBuffer;
    }

private :
    const int mKey;
};

class BinMapItemNotFoundStringKey : public BinMappingExceptionForUserBase
{
public :
    const char * what() const throw();

protected :
    BinMapItemNotFoundStringKey( const char * const aBinMapFilePath,
                                 const char * const aConverterExternalFilePath,
                                 const char * const aKey );

protected :
    char mKey[ 256 ];
};

template<>
class BinMapItemNotFound< std::string > : public BinMapItemNotFoundStringKey
{
public :
    BinMapItemNotFound( const char * const aBinMapFilePath,
                        const char * const aConverterExternalFilePath,
                        const std::string &aKey ) :
        BinMapItemNotFoundStringKey( aBinMapFilePath, aConverterExternalFilePath, aKey.c_str() ) {}
};

template<>
class BinMapItemNotFound< const char * > : public BinMapItemNotFoundStringKey
{
public :
    BinMapItemNotFound( const char * const aBinMapFilePath,
                        const char * const aConverterExternalFilePath,
                        const char * const aKey ) :
        BinMapItemNotFoundStringKey( aBinMapFilePath, aConverterExternalFilePath, aKey ) {}
};

class BadFieldFormat : public std::exception
{
public :
    const char * what() const throw() { return "BadFieldFormat"; }
};

class FieldIndexOutOfRange : public std::exception
{
public :
    const char * what() const throw() { return "FieldIndexOutOfRange"; }
};

class InvalidHvmWsBinMappingFileFormat : public BinMappingExceptionForUserBase
{
public :
    InvalidHvmWsBinMappingFileFormat( const char * const aBinMapFilePath,
                                      const char * const aConverterExternalFilePath,
                                      const char * const aArguments );
    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidLvmFtFtBinMappingFileFormat : public BinMappingExceptionForUserBase
{
public :
    InvalidLvmFtFtBinMappingFileFormat( const char * const aBinMapFilePath,
                                        const char * const aConverterExternalFilePath,
                                        const char * const aArguments );
    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidLvmFtSeBinMappingFileFormat : public BinMappingExceptionForUserBase
{
public :
    InvalidLvmFtSeBinMappingFileFormat( const char * const aBinMapFilePath,
                                        const char * const aConverterExternalFilePath,
                                        const char * const aArguments );

    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidLvmWsBinMappingFileFormat : public BinMappingExceptionForUserBase
{
public :
    InvalidLvmWsBinMappingFileFormat( const char * const aBinMapFilePath,
                                      const char * const aConverterExternalFilePath,
                                      const char * const aArguments );
    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class InvalidHvmFtBinMappingFileFormat : public BinMappingExceptionForUserBase
{
public :
    InvalidHvmFtBinMappingFileFormat( const char * const aBinMapFilePath,
                                      const char * const aConverterExternalFilePath,
                                      const char * const aArguments );
    const char * what() const throw();

private :
    char mArguments[ 128 ];
};

class DuplicatedTestInLvmWsBinMappingFile : public BinMappingExceptionForUserBase
{
public :
    DuplicatedTestInLvmWsBinMappingFile( int aDuplicatedTestNumber,
                                         const char * const aBinMapFilePath,
                                         const char * const aConverterExternalFilePath );
    const char * what() const throw();

private :
    int mArguments;
};

class DuplicatedTestInLvmFtBinMappingFile : public BinMappingExceptionForUserBase
{
public :
    DuplicatedTestInLvmFtBinMappingFile( int aDuplicatedTestNumber,
                                         const char * const aBinMapFilePath,
                                         const char * const aConverterExternalFilePath );

    const char * what() const throw();

private :
    int mDuplicatedTestNumber;
};

class DuplicatedTestInLvmSeBinMappingFile : public BinMappingExceptionForUserBase
{
public :
    DuplicatedTestInLvmSeBinMappingFile( int aDuplicatedTestNumber,
                                         const char * const aBinMapFilePath,
                                         const char * const aConverterExternalFilePath );

    const char * what() const throw();

private :
    int mDuplicatedTestNumber;
};

class InvalidTestInLvmFtBinMappingFile : public BinMappingExceptionForUserBase
{
public :
    InvalidTestInLvmFtBinMappingFile( const char * const aBinMapFilePath,
                                      const char * const aConverterExternalFilePath,
                                      int aInvalidTestNumber );
    const char * what() const throw();

private :
    char mArguments[ 11 ];
};

}
}

#endif // BIN_MAPPING_EXCEPTIONS_H
