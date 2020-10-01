#ifndef LVM_FT_LOTLIST_PROMIS_INTERPRETER_H
#define LVM_FT_LOTLIST_PROMIS_INTERPRETER_H

#include "promis_interpreter_base.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmFtLotlistPromisInterpreter : public PromisInterpreterBase
{
public:
    LvmFtLotlistPromisInterpreter( const std::string &aKey,
                                   const std::string &aPromisFilePath,
                                   const std::string &aConverterExternalFilePath );
    virtual ~LvmFtLotlistPromisInterpreter();

    const PromisItemBase & GetPromisItem() const;

private:
    LvmFtLotlistPromisInterpreter( const LvmFtLotlistPromisInterpreter & );
    LvmFtLotlistPromisInterpreter & operator =( const LvmFtLotlistPromisInterpreter & );

    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool CanSetItem( const std::string &lKey ) const;
    void SetItem( const TabularFileLineFields &aFields );
};

}
}

#endif // LVM_FT_LOTLIST_PROMIS_INTERPRETER_H
