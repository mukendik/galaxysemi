#ifndef TESTABLE_BIN_MAP_STORE_H
#define TESTABLE_BIN_MAP_STORE_H

#include "bin_map_item_base.h"
using Qx::BinMapping::BinMapItemBase;

#include <QString>
#include <QTest>

#include <fstream>
#include <vector>

struct SerializableToFile
{
    virtual ~SerializableToFile();

    virtual void SerializeToFile( const QString &aFilePath ) const = 0;
};

std::ofstream & operator << ( std::ofstream &aOutputFileStream, const BinMapItemBase &aBinMapItem );

template< typename BinMapStoreType >
class TestableBinMapStore : public BinMapStoreType, public SerializableToFile
{
public :
    TestableBinMapStore( const QString &aBinMapInputFile,
                         const QString &aConverterExternalFilePath ) : BinMapStoreType( aBinMapInputFile.toStdString(),
                                                                                        aConverterExternalFilePath.toStdString() ) {}

    void SerializeToFile( const QString &aFilePath ) const
    {
        std::ofstream lOutputFileStream( aFilePath.toStdString().c_str(), std::ios_base::out );
        if( ! lOutputFileStream.is_open() )
            QFAIL( QString( "Cannot open %1 file with write permissions" )
                   .arg( aFilePath ).toStdString().c_str() );

        typedef typename BinMapStoreType::BinMapContainer::const_iterator const_iterator;

        for( const_iterator lIterator = this->mBinMapItems.begin(), lEnd = this->mBinMapItems.end();
             lIterator != lEnd;
             ++lIterator )
            lOutputFileStream << **lIterator << '\n';
    }
};

#endif // TESTABLE_BIN_MAP_STORE_H
