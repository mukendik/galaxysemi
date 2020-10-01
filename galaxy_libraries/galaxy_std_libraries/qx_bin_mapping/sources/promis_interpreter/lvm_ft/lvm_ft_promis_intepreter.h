#ifndef LVM_FT_PROMIS_INTERPRETER_H
#define LVM_FT_PROMIS_INTERPRETER_H

#include "promis_interpreter_base.h"

namespace Qx
{
namespace BinMapping
{

class LvmFtPromisItem;
class TabularFileLineFields;

class QX_BIN_MAPPING_API_DECL LvmFtPromisInterpreter: public PromisInterpreterBase
{
public:
    LvmFtPromisInterpreter( const std::string &aKey,
                            const std::string &aPromisFilePath,
                            const std::string &aConverterExternalFilePath );
    virtual ~LvmFtPromisInterpreter();

    const PromisItemBase & GetPromisItem() const;

private:
    LvmFtPromisInterpreter( const LvmFtPromisInterpreter & );
    LvmFtPromisInterpreter & operator =( const LvmFtPromisInterpreter & );

    void ProcessReadLine( const std::string &aLine );
    bool IsComment(const std::string &aLine) const;
    bool IsHeader( const std::string &aLine ) const;
    bool CanSetItem( const std::string &lKey ) const;
    void SetItem( const TabularFileLineFields &aFields );
    void ExtractMandatoryFieldsInto( LvmFtPromisItem *aItem, const TabularFileLineFields &aFields ) const;
    void ExtractOptionalFieldsInto( LvmFtPromisItem *aItem, const TabularFileLineFields &aFields ) const;
};

}
}
#endif // LVM_FT_PROMIS_INTERPRETER_H
