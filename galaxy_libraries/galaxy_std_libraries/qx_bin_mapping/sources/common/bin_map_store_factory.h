#ifndef BIN_MAP_STORE_FACTORY_H
#define BIN_MAP_STORE_FACTORY_H

#include "bin_mapping_api.h"
#include "queryable_by_bin_name_store.h"
#include "queryable_by_test_number_store.h"
#include "queryable_by_test_name_store.h"
#include "validity_check_for_bin.h"

#include "hvm_ws_bin_map_store_fet_test.h"
#include "hvm_ws_bin_map_store_specktra.h"
#include "lvm_ws_bin_map_store.h"
#include "hvm_ft_bin_map_store.h"
#include "lvm_ft_final_test_bin_map_store.h"
#include "lvm_ft_sort_entries_bin_map_store.h"
#include "lvm_ft_sort_entries_new_bin_map_store.h"

#include <string>

namespace Qx
{
namespace BinMapping
{

enum BinMapStoreTypes
{
    lvm_ws,
    lvm_ft_ft,
    lvm_ft_se,
    lvm_ft_se_new,
    hvm_ft,
    hvm_ws_fet_test,
    hvm_ws_spektra
};

enum BinMapStoreAccessMode
{
    queryable,
    validity_check
};

template< BinMapStoreTypes,  BinMapStoreAccessMode = queryable>
struct BinMapStoreFactory;

template<>
struct BinMapStoreFactory< lvm_ws >
{
    static QueryableByTestNumber * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFile)
    { return new LvmWsBinMapStore( aFilePath, aConverterExternalFile ); }
};

template<>
struct BinMapStoreFactory< hvm_ft >
{
    static QueryableByBinName * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFile)
    { return new HvmFtBinMapStore( aFilePath, aConverterExternalFile ); }
};

template<>
struct BinMapStoreFactory< lvm_ft_ft >
{
    static QueryableByTestNumber * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFilePath )
    { return new LvmFtFinalTestBinMapStore( aFilePath, aConverterExternalFilePath); }
};

template<>
struct BinMapStoreFactory< lvm_ft_ft,  validity_check>
{
    static ValidatableByBin * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFilePath )
    { return new LvmFtFinalTestBinMapStore( aFilePath, aConverterExternalFilePath ); }
};

template<>
struct BinMapStoreFactory< lvm_ft_se >
{
    static QueryableByTestName * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFilePath )
    { return new LvmFtSortEntriesBinMapStore( aFilePath, aConverterExternalFilePath ); }
};

template<>
struct BinMapStoreFactory< lvm_ft_se,  validity_check>
{
    static ValidatableByBin * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFilePath)
    { return new LvmFtSortEntriesBinMapStore( aFilePath, aConverterExternalFilePath ); }
};

template<>
struct BinMapStoreFactory< lvm_ft_se_new >
{
    static QueryableByTestNumber * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFile)
    { return new LvmFtSortEntriesNewBinMapStore( aFilePath, aConverterExternalFile ); }
};

template<>
struct BinMapStoreFactory< lvm_ft_se_new,  validity_check>
{
    static ValidatableByBin * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFile)
    { return new LvmFtSortEntriesNewBinMapStore( aFilePath, aConverterExternalFile ); }
};

template<>
struct BinMapStoreFactory< hvm_ws_fet_test >
{
    static QueryableByTestNumber * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFile)
    { return new HvmWsBinMapStoreFetTest( aFilePath, aConverterExternalFile ); }

};

template<>
struct BinMapStoreFactory< hvm_ws_spektra >
{
    static QueryableByTestNumber * MakeBinMapStore( const std::string &aFilePath, const std::string &aConverterExternalFile)
    { return new HvmWsBinMapStoreSpecktra( aFilePath, aConverterExternalFile ); }

};
}
}

#endif // BIN_MAP_STORE_FACTORY_H
