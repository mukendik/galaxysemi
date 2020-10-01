#ifndef LVM_WT_PROMIS_INTERPRETER_H
#define LVM_WT_PROMIS_INTERPRETER_H

#include "promis_interpreter_base.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL LvmWtPromisInterpreter: public PromisInterpreterBase
{
public:
    LvmWtPromisInterpreter( const std::string &aKey,
                            const std::string &aPromisFilePath,
                            const std::string &aConverterExternalFilePath );

    virtual ~LvmWtPromisInterpreter();

    const PromisItemBase & GetPromisItem() const;

private:
    LvmWtPromisInterpreter( const LvmWtPromisInterpreter & );
    LvmWtPromisInterpreter & operator =( const LvmWtPromisInterpreter & );

    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool CanSetItem( const std::string &lKey ) const;
    void SetItem( const TabularFileLineFields &aFields );
};

}
}
#endif // LVM_WT_PROMIS_INTERPRETER_H
