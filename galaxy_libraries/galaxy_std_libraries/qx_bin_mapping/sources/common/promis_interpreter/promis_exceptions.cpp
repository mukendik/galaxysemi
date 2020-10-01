#include "promis_exceptions.h"

#include <cstdio>

namespace Qx
{
namespace BinMapping
{

InvalidLvmFtSubconDataPromisFileFormat::InvalidLvmFtSubconDataPromisFileFormat(const char * const aArguments)
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char * InvalidLvmFtSubconDataPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                   "Incorrect promis format (lvm_ft_subcon_data).\n"
                   "Supported format must contain 4 columns :\n"
                   "<LotID.SublotID>,<PartNumber>,<PackageType>,<GeometryName>\n"
                   "Line read was :\n"
                   "%s",
                   mArguments);

    return lMessageBuffer;
}

InvalidLvmFtFieldNbPromisFileFormat::InvalidLvmFtFieldNbPromisFileFormat(const char *aPromisPath, const char* aExtFilePath, const int aMinFieldsNb)
{
    std::sprintf( &mPromisPath[ 0 ], "%s", aPromisPath );
    std::sprintf( &mExtFilePath[ 0 ], "%s", aExtFilePath);
    mMinFieldsNb = aMinFieldsNb;
}

const char * InvalidLvmFtFieldNbPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ sizeof( mPromisPath ) + sizeof( mExtFilePath ) + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                    " specified in converter external file %s"
                ".\nMissing columns. At least %i columns are expected.",
                    mExtFilePath,
                mMinFieldsNb);
    return lMessageBuffer;
}

InvalidLvmFtLotlistPromisFileFormat::InvalidLvmFtLotlistPromisFileFormat(const char * const aArguments)
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char * InvalidLvmFtLotlistPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                   "Incorrect promis format (lvm_ft_lotlist).\n"
                   "Supported format must contain 3 columns :\n"
                   "<LotID.SublotID>,<DateCode>,<SiteLocation>\n"
                   "Line read was :\n"
                   "%s",
                   mArguments );

    return lMessageBuffer;
}

InvalidLvmWtPromisFileFormat::InvalidLvmWtPromisFileFormat(const char * const aArguments)
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char * InvalidLvmWtPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                   "Incorrect promis format (lvm_wt).\n"
                   "Supported format must contain 12 columns :\n"
                   "<LotID.WaferID>,<LotID.SublotID>,<NbWafers>,<FabSite>,<EquipmentID>,<PartID>,<GeometryName>,"
                   "<GrossDiePerWafer>,<DieWidth>,<DieHight>,<FlatOrientation>,<TestSite>\n"
                   "Line read was :\n"
                   "%s",
                   mArguments);

    return lMessageBuffer;
}

InvalidHvmWtPromisFileFormat::InvalidHvmWtPromisFileFormat(const char * const aArguments)
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char * InvalidHvmWtPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                   "Incorrect promis format (hvm_wt).\n"
                   "Supported format must contain 11 columns :\n"
                   "<LotID.WaferID>,<LotID.SublotID>,<NbWafers>,<FabLocation>,<DiePart>,<GeometryName>,"
                   "<GrossDiePerWafer>,<DieWidth>,<DieHight>,<FlatOrientation>,<Sitelocation>\n"
                   "Line read was :\n"
                   "%s",
                   mArguments);

    return lMessageBuffer;
}

InvalidLvmFtPromisFileFormat::InvalidLvmFtPromisFileFormat(const char * const aArguments)
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char * InvalidLvmFtPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                   "Incorrect promis format (lvm_ft).\n"
                   "Supported format must contain between 8 and 14 columns :\n"
                   "<LotID>,<DateCode>,<Package>,<EquipmentID>,<ProductID>,<SiteID>,<GeometryName>,<PackageType>"
                   "[,<LotID>,<GeometryName>[,<LotID>,<GeometryName>[,<LotID>,<GeometryName>]]]\n"
                   "Line read was :\n"
                   "%s",
                   mArguments);

    return lMessageBuffer;
}

InvalidHvmFtPromisFileFormat::InvalidHvmFtPromisFileFormat(const char * const aArguments)
{
    std::sprintf( &mArguments[ 0 ], "%s", aArguments );
}

const char * InvalidHvmFtPromisFileFormat::what() const throw()
{
    static char lMessageBuffer[ MARGUMENTSSIZE_PROMIS_EXCEPTIONS + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                   "Incorrect promis format (hvm_ft).\n"
                   "Supported format must contain 9 columns :\n"
                   "<LotID>.<SublotID>, <DateCode>,<PackageType>,<EquipmentID>,<PartNumber>,<SiteLocation>,<GeometryName>,"
                   "<DiePart>,<Division>\n"
                   "Line read was :\n"
                   "%s",
                   mArguments);

    return lMessageBuffer;
}

InvalidPromisFilePath::InvalidPromisFilePath(const char *aConverterExternalFilePath)
{
    std::sprintf( &mConverterExternalFilePath[ 0 ], "%s", aConverterExternalFilePath );
}

const char *InvalidPromisFilePath::what() const throw()
{
    static char lMessageBuffer[ sizeof ( mConverterExternalFilePath ) + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "The promis file path cannot be empty as specified in the %s file.",
                  mConverterExternalFilePath );

    return lMessageBuffer;
}

CannotOpenPromisFile::CannotOpenPromisFile(const char *aPromisFilePath, const char *aConverterExternalFilePath)
{

    std::sprintf( &mPromisFilePath[ 0 ], "%s", aPromisFilePath);
    std::sprintf( &mConverterExternalFilePath[ 0 ], "%s", aConverterExternalFilePath );
}

const char *CannotOpenPromisFile::what() const throw()
{
    static char lMessageBuffer[ sizeof( mPromisFilePath ) + sizeof( mConverterExternalFilePath ) + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Cannot open the promis file %s in read mode specified in %s",
                  mPromisFilePath,
                  mConverterExternalFilePath );

    return lMessageBuffer;
}

PromisItemNotFound::PromisItemNotFound(const char *aKey, const char *aPromisFilePath, const char *aConverterExternalFilePath)
{
    std::sprintf( &mKey[ 0 ], "%s", aKey );
    std::sprintf( &mPromisFilePath[ 0 ], "%s", aPromisFilePath );
    std::sprintf( &mConverterExternalFilePath[ 0 ], "%s", aConverterExternalFilePath );
}

const char *PromisItemNotFound::what() const throw()
{
    static char lMessageBuffer[ sizeof( mKey ) +
                                sizeof( mPromisFilePath ) +
                                sizeof( mConverterExternalFilePath ) + 256 ];

    std::sprintf( &lMessageBuffer[ 0 ],
                  "Cannot find the promis item with key : %s in promis file :  %s specified in file : %s",
                  mKey, mPromisFilePath, mConverterExternalFilePath );

    return lMessageBuffer;
}

}
}
