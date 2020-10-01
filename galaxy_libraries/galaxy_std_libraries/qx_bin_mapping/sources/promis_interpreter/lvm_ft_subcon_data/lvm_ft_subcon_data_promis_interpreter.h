#ifndef LVM_FT_SUBCON_DATA_PROMIS_INTERPRETER_H
#define LVM_FT_SUBCON_DATA_PROMIS_INTERPRETER_H

#include "promis_interpreter_base.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmFtSubconDataPromisInterpreter : public PromisInterpreterBase
{
public:
    LvmFtSubconDataPromisInterpreter( const std::string &aKey,
                                      const std::string &aPromisFilePath,
                                      const std::string &aConverterExternalFilePath );
    virtual ~LvmFtSubconDataPromisInterpreter();

    const PromisItemBase & GetPromisItem() const;

private:
    LvmFtSubconDataPromisInterpreter( const LvmFtSubconDataPromisInterpreter & );
    LvmFtSubconDataPromisInterpreter & operator =( const LvmFtSubconDataPromisInterpreter & );

    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool CanSetItem( const std::string &lKey ) const;
    void SetItem( const TabularFileLineFields &aFields );
};

}
}


#endif // LVM_FT_SUBCON_DATA_PROMIS_INTERPRETER_H
