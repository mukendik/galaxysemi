#ifndef IMPORTWATUMC_H
#define IMPORTWATUMC_H

#include "importWat.h"

namespace GS
{
    namespace Parser
    {
        class WatUmcToStdf : public WatToStdf
        {
            public:
                WatUmcToStdf();
                ~WatUmcToStdf();

              static bool  IsCompatible(const QString& fileName);

        private:
                bool            ParseFile              (const QString &fileName);
                bool            WriteStdfFile          (const char *stdFileName);
        };
    }
}

#endif // IMPORTWATUMC_H

