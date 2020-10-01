#ifndef BIN_MAP_STORE_BASE_H
#define BIN_MAP_STORE_BASE_H

#include "bin_mapping_api.h"
#include "tabular_file_line_fields.h"

#include <vector>

namespace Qx
{
namespace BinMapping
{

struct BinMapItemBase;

class QX_BIN_MAPPING_API_DECL BinMapStoreBase
{
public :
    typedef const BinMapItemBase *BinMapItemPointer;
    typedef std::vector< BinMapItemPointer > BinMapContainer;

public :
    virtual ~BinMapStoreBase();

protected :
    BinMapStoreBase( const std::string &aBinmapFilePath,
                     const std::string &aConverterExternalFilePath );
    BinMapStoreBase( const BinMapStoreBase & );
    BinMapStoreBase & operator =( const BinMapStoreBase & );

    void FillBinMapWithStream(std::ifstream &aFileStream );
    bool GetNextTrimmedLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine ) const;
    virtual void ProcessReadLine( const std::string &aLine ) = 0;
    bool ReadAndTrimLineFromStream(std::ifstream &aFileStream, std::string &aOutputReadLine ) const;
    bool IsRelevantLine(const std::string &aLine) const;
    virtual bool IsComment(const std::string &aLine) const = 0;
    virtual bool IsHeader( const std::string &aLine ) const = 0;
    virtual bool IsEmpty( const std::string &aLine ) const  = 0;
    bool IsEndOfLine( const std::string &aLine ) const;

    template< typename ExtractionException >
        TabularFileLineFields ExtractNFieldsFromLine( const std::string &aLine, unsigned int aFieldCount ) const
    {
        TabularFileLineFields lFields( aLine );

        if( lFields.Count() != aFieldCount )
            throw ExtractionException( mBinMapFilePath.c_str(),
                                       mConverterExternalFilePath.c_str(),
                                       aLine.c_str() );

        return lFields;
    }

protected :
    BinMapContainer mBinMapItems;

    // https://en.cppreference.com/w/cpp/named_req/InputIterator : dereferincing an input iterator can lead to create
    // a temporary object instead of returning a reference.
    // https://stackoverflow.com/questions/4050429/stl-iterator-dereferencing-iterator-to-a-temporary-is-it-possible
    mutable BinMapItemPointer mDereferencedItem;

    const std::string mBinMapFilePath;
    const std::string mConverterExternalFilePath;
};

}
}

#endif // BIN_MAP_STORE_BASE_H
