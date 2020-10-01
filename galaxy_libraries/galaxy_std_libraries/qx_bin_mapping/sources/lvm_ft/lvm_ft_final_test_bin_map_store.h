#ifndef LVM_FT_BIN_MAP_STORE_H
#define LVM_FT_BIN_MAP_STORE_H

#include "bin_mapping_api.h"
#include "bin_map_store_base.h"
#include "queryable_by_test_number_store.h"
#include "validity_check_for_bin.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmFtFinalTestBinMapStore : public BinMapStoreBase,
                                                          public QueryableByTestNumber,
                                                          public ValidatableByBin
{
public :
    LvmFtFinalTestBinMapStore(const std::string &aBinMapFilePath, const std::string &aConverterExternalFilePath );

    const BinMapItemBase &  GetBinMapItemByTestNumber( int aTestNumber ) const;
    bool                    IsBinCorrect(int aNumBin, const std::string& aBinName) const;
    virtual std::string     RetrieveBinName(int aNumBin) const;

private :
    LvmFtFinalTestBinMapStore( const LvmFtFinalTestBinMapStore & );
    LvmFtFinalTestBinMapStore & operator =( const LvmFtFinalTestBinMapStore & );

    bool IsComment(const std::string &aLine) const;
    bool CanAddItemInBinMap(int aTestNumber) const;
    bool IsHeader( const std::string &aLine ) const;
    bool IsEmpty( const std::string &aLine ) const;
    void AppendBinMapItemForTest(int aTestNumber, const TabularFileLineFields &aFields);
    void ProcessReadLine( const std::string &aLine );
    std::vector<int> ExtractAllTestNumbersFromString(const std::string &aNumbers) const;
    bool AreAllFieldsRelevant( const TabularFileLineFields &aFields ) const;
    std::vector< int > GetAllTestNumberFromFields( const TabularFileLineFields &aFields ) const;
    void ProcessReadLineForAllTestNumbersInFields( const TabularFileLineFields &aFields );
};

}
}

#endif // LVM_FT_BIN_MAP_STORE_H
