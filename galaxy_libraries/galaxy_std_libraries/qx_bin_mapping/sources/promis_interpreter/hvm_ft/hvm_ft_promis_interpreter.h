#ifndef HVM_FT_PROMIS_INTERPRETER_H
#define HVM_FT_PROMIS_INTERPRETER_H

#include "promis_interpreter_base.h"

namespace Qx
{
namespace BinMapping
{

class QX_BIN_MAPPING_API_DECL HvmFtPromisInterpreter: public PromisInterpreterBase
{
public:
    HvmFtPromisInterpreter( const std::string &aKey,
                            const std::string &aPromisFilePath,
                            const std::string &aConverterExternalFilePath );
    virtual ~HvmFtPromisInterpreter();

    const PromisItemBase & GetPromisItem() const;

private:
    HvmFtPromisInterpreter( const HvmFtPromisInterpreter & );
    HvmFtPromisInterpreter & operator =( const HvmFtPromisInterpreter & );

    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool CanSetItem( const std::string &lKey ) const;
    void SetItem( const TabularFileLineFields &aFields );
};

}
}

#endif // HVM_FT_PROMIS_INTERPRETER_H
