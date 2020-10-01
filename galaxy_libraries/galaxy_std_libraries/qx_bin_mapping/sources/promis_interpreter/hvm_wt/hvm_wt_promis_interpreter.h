#ifndef HVM_WT_PROMIS_INTERPRETER_H
#define HVM_WT_PROMIS_INTERPRETER_H

#include "promis_interpreter_base.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL HvmWtPromisInterpreter: public PromisInterpreterBase
{
public:
    HvmWtPromisInterpreter( const std::string &aKey,
                            const std::string& aPromisFilePath,
                            const std::string &aConverterExternalFilePath );
    virtual ~HvmWtPromisInterpreter();

    const PromisItemBase & GetPromisItem() const;

private:
    HvmWtPromisInterpreter( const HvmWtPromisInterpreter & );
    HvmWtPromisInterpreter & operator =( const HvmWtPromisInterpreter & );

    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool CanSetItem( const std::string &lKey ) const;
    void SetItem( const TabularFileLineFields &aFields );
};

}
}

#endif // HVM_WT_PROMIS_INTERPRETER_H
