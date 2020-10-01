#ifndef PROMIS_INTERPRETER_FACTORY_H
#define PROMIS_INTERPRETER_FACTORY_H

#include "lvm_ft_subcon_data_promis_interpreter.h"
#include "lvm_ft_lotlist_promis_interpreter.h"
#include "lvm_wt_promis_interpreter.h"
#include "hvm_wt_promis_interpreter.h"
#include "lvm_ft_promis_intepreter.h"
#include "hvm_ft_promis_interpreter.h"

namespace Qx
{
namespace BinMapping
{

enum PromisIntepreterTypes
{
    promis_lvm_ft_subcon_data,
    promis_lvm_ft_lotlist,
    promis_lvm_wt,
    promis_hvm_wt,
    promis_lvm_ft,
    promis_hvm_ft
};

template< PromisIntepreterTypes >
struct PromisInterpreterFactory;

template<>
struct PromisInterpreterFactory< promis_lvm_ft_subcon_data >
{
    static PromisInterpreterBase * MakePromisInterpreter( const std::string & aKey,
                                                          const std::string &aFilePath,
                                                          const std::string &aConverterExternalFilePath )
    { return new LvmFtSubconDataPromisInterpreter( aKey, aFilePath, aConverterExternalFilePath ); }
};

template<>
struct PromisInterpreterFactory< promis_lvm_ft_lotlist >
{
    static PromisInterpreterBase * MakePromisInterpreter( const std::string & aKey,
                                                          const std::string &aFilePath,
                                                          const std::string &aConverterExternalFilePath )
    { return new LvmFtLotlistPromisInterpreter( aKey, aFilePath, aConverterExternalFilePath ); }
};

template<>
struct PromisInterpreterFactory< promis_lvm_wt >
{
    static PromisInterpreterBase * MakePromisInterpreter( const std::string & aKey,
                                                          const std::string &aFilePath,
                                                          const std::string &aConverterExternalFilePath )
    { return new LvmWtPromisInterpreter( aKey, aFilePath, aConverterExternalFilePath ); }
};

template<>
struct PromisInterpreterFactory< promis_lvm_ft >
{
    static PromisInterpreterBase * MakePromisInterpreter( const std::string &aKey,
                                                          const std::string &aFilePath,
                                                          const std::string &aConverterExternalFilePath )
    { return new LvmFtPromisInterpreter( aKey, aFilePath, aConverterExternalFilePath ); }
};

template<>
struct PromisInterpreterFactory< promis_hvm_wt >
{
    static PromisInterpreterBase * MakePromisInterpreter( const std::string &aKey,
                                                          const std::string &aFilePath,
                                                          const std::string &aConverterExternalFilePath )
    { return new HvmWtPromisInterpreter( aKey, aFilePath, aConverterExternalFilePath ); }
};

template<>
struct PromisInterpreterFactory< promis_hvm_ft >
{
    static PromisInterpreterBase * MakePromisInterpreter( const std::string & aKey,
                                                          const std::string &aFilePath,
                                                          const std::string &aConverterExternalFilePath )
    { return new HvmFtPromisInterpreter( aKey, aFilePath, aConverterExternalFilePath ); }
};

}
}

#endif // PROMIS_INTERPRETER_FACTORY_H
