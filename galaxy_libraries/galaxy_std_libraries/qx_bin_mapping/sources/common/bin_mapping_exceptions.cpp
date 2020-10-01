#include "bin_mapping_exceptions.h"

#include <algorithm>
#include <cstring>

namespace Qx
{
namespace BinMapping
{

InvalidHvmWsBinMappingFileFormat::InvalidHvmWsBinMappingFileFormat( const char * const aBinMapFilePath,
                                                                    const char * const aConverterExternalFilePath,
                                                                    const char * const aArguments) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char *InvalidHvmWsBinMappingFileFormat::what() const throw()
{
    static char lMessageBuffer[ 256 + 256 + 128 + 128 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Incorrect BinMap format [HVM WS] for the bin mapping file '%s' "
                  "specified in the file '%s'\n"
                  "Supported format: Bin #,Bin Names (Spektra),TEST# (FETTEST),Std. Bin Name\n"
                  "Ex: 2,ISGS@5V,63,IGSS@5V\n"
                  "Line was: %s",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mArguments );

    return lMessageBuffer;
}

InvalidLvmFtFtBinMappingFileFormat::InvalidLvmFtFtBinMappingFileFormat( const char * const aBinMapFilePath,
                                                                        const char * const aConverterExternalFilePath,
                                                                        const char * const aArguments ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char *InvalidLvmFtFtBinMappingFileFormat::what() const throw()
{
    static char lMessageBuffer[ 256 + 256 + 128 + 128 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Incorrect BinMap format [LVM FT] for the bin mapping file "
                  "%s specified in converter external file %s.\n"
                  "Supported format: Enabled,Family,STD_Tests#,Tests#,PowerTech,STATEC/SPEKTA,Mapped Test,Bin#,Bin Name,Condition\n"
                  "Ex: 1,Igss,1/101,1,1 Igss,1_Igss,1,10,IGSS @ + 100%%VGS,IGSS @ + 100%%VGS\n"
                  "Line was :\n%s",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mArguments );

    return lMessageBuffer;
}

InvalidLvmWsBinMappingFileFormat::InvalidLvmWsBinMappingFileFormat( const char * const aBinMapFilePath,
                                                                    const char * const aConverterExternalFilePath,
                                                                    const char * const aArguments ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char *InvalidLvmWsBinMappingFileFormat::what() const throw()
{
    static char lMessageBuffer[ 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Incorrect BinMap format [LVM WS] for the bin mapping file '%s' "
                  "specified in the file '%s'\n"
                  "Supported format: Test Name,STD TEST NUMBERS,Assigned  BIN#,Bin Name\n"
                  "Ex: GS SHORT,5,29,GS SHORT\n"
                  "Line was: %s",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mArguments );

    return lMessageBuffer;
}

InvalidHvmFtBinMappingFileFormat::InvalidHvmFtBinMappingFileFormat( const char * const aBinMapFilePath,
                                                                    const char * const aConverterExternalFilePath,
                                                                    const char * const aArguments ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char *InvalidHvmFtBinMappingFileFormat::what() const throw()
{
    static char lMessageBuffer[ 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Incorrect BinMap format [HVM FT] for the bin mapping file '%s' "
                  "specified in the file '%s'\n"
                  "Supported format: HW BIN,SW BIN,Bin Name (Spektra),Standard Bin Name\n"
                  "Ex: 3,3,267 NO AAC BOAR,NO AAC BOAR\n"
                  "Line was: %s",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mArguments );

    return lMessageBuffer;
}

DuplicatedTestInLvmWsBinMappingFile::DuplicatedTestInLvmWsBinMappingFile(int aDuplicatedTestNumber,
                                                                         const char * const aBinMapFilePath,
                                                                         const char * const aConverterExternalFilePath ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath ),
    mArguments( aDuplicatedTestNumber ) {}

const char *DuplicatedTestInLvmWsBinMappingFile::what() const throw()
{
    static char lMessageBuffer[ 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "The [LVM WS] bin mapping file: '%s' specified in the file: '%s' contains a duplicated test number: '%d'",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mArguments );

    return lMessageBuffer;
}

DuplicatedTestInLvmFtBinMappingFile::DuplicatedTestInLvmFtBinMappingFile(int aDuplicatedTestNumber,
                                                                         const char * const aBinMapFilePath,
                                                                         const char * const aConverterExternalFilePath ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath ),
    mDuplicatedTestNumber( aDuplicatedTestNumber ) {}

const char *DuplicatedTestInLvmFtBinMappingFile::what() const throw()
{
    static char lMessageBuffer[ 256 + 256 + 11 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "The [LVM FT] bin mapping file: '%s' specified in the file: '%s' contains a duplicated test number: '%d'",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mDuplicatedTestNumber );

    return lMessageBuffer;
}

DuplicatedTestInLvmSeBinMappingFile::DuplicatedTestInLvmSeBinMappingFile(int aDuplicatedTestNumber,
                                                                         const char * const aBinMapFilePath,
                                                                         const char * const aConverterExternalFilePath ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath ),
    mDuplicatedTestNumber( aDuplicatedTestNumber ) {}

const char *DuplicatedTestInLvmSeBinMappingFile::what() const throw()
{
    static char lMessageBuffer[ 256 + 256 + 11 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "The [LVM FT SE] bin mapping file: '%s' specified in the file: '%s' contains a duplicated test number: '%d'",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mDuplicatedTestNumber );

    return lMessageBuffer;
}

InvalidTestInLvmFtBinMappingFile::InvalidTestInLvmFtBinMappingFile( const char * const aBinMapFilePath,
                                                                    const char * const aConverterExternalFilePath,
                                                                    int aInvalidTestNumber ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mArguments[ 0 ], "%d", aInvalidTestNumber );
}

const char *InvalidTestInLvmFtBinMappingFile::what() const throw()
{
    static char lMessageBuffer[ 256 ] = {};

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Incorrect BinMap format [LVM FT].\nAn invalid test number has been found : %s",
                  mArguments );

    return lMessageBuffer;
}

CannotOpenBinMappingFile::CannotOpenBinMappingFile( const char * const aBinMapFilePath,
                                                    const char * const aConverterExternalFilePath ):
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath ) {}

const char *CannotOpenBinMappingFile::what() const throw()
{
    static char lMessageBuffer[ 256 + 256 + 128 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Cannot open the bin mapping file: '%s' specified in: '%s'",
                  mBinMapFilePath, mConverterExternalFilePath );

    return lMessageBuffer;
}

const char *BinMapItemNotFoundStringKey::what() const throw()
{
    static char lBuffer[ 256 + 256 + 256 ];

    std::sprintf( &lBuffer[ 0 ],
                  "Cannot find the bin map item: '%s' "
                  "in the file: '%s' "
                  "specified in: '%s'",
                  mKey, mBinMapFilePath, mConverterExternalFilePath );

    return lBuffer;
}

BinMapItemNotFoundStringKey::BinMapItemNotFoundStringKey(const char * const aBinMapFilePath,
                                                         const char * const aConverterExternalFilePath,
                                                         const char * const aKey) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mKey[ 0 ], "%s", aKey );
}

InvalidLvmFtSeBinMappingFileFormat::InvalidLvmFtSeBinMappingFileFormat( const char * const aBinMapFilePath,
                                                                    const char * const aConverterExternalFilePath,
                                                                    const char * const aArguments ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath )
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char *InvalidLvmFtSeBinMappingFileFormat::what() const throw()
{
    static char lMessageBuffer[ 256 + 256 + 128 + 128 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Incorrect BinMap format [LVM FT] for the bin mapping file '%s' "
                  "specified in the file '%s'\n"
                  "Supported format: Enabled,Family,Tests name,Test#,ASE,AMKOR,BIN #\n"
                  "Ex: 1,G-D-S (Short),GDS sh,1000,1000_GDS sh,1000 GDS sh,1000\n"
                  "Line was: %s",
                  mBinMapFilePath,
                  mConverterExternalFilePath,
                  mArguments );

    return lMessageBuffer;
}

BinMappingExceptionForUserBase::BinMappingExceptionForUserBase( const char * const aBinMapFilePath,
                                                                const char * const aConverterExternalFilePath )
{
    std::sprintf( &mBinMapFilePath[ 0 ], "%s", aBinMapFilePath );
    std::sprintf( &mConverterExternalFilePath[ 0 ], "%s", aConverterExternalFilePath );
}

InvalidBinMapFilePath::InvalidBinMapFilePath( const char * const aBinMapFilePath,
                                              const char * const aConverterExternalFilePath ) :
    BinMappingExceptionForUserBase( aBinMapFilePath, aConverterExternalFilePath ) {}

const char *InvalidBinMapFilePath::what() const throw()
{
    static char lBuffer[ 256 + 256 + 64 ];

    std::sprintf( &lBuffer[ 0 ],
                  "Invalid bin mapping file specified in the file '%s'",
                  mConverterExternalFilePath );

    return lBuffer;
}

}
}
