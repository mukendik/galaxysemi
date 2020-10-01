#ifndef IMPORTWATASMC_H
#define IMPORTWATASMC_H

#include "importWat.h"

namespace GS
{
    namespace Parser
    {
        class WatAsmcToStdf : public WatToStdf
        {
            public:
                WatAsmcToStdf();
                ~WatAsmcToStdf();

              static bool  IsCompatible(const QString& fileName);

        private:

                bool            ParseFile              (const QString &fileName);
                bool            WriteStdfFile          (const char *stdFileName);

                WatParameter*   CreateWatParameter     (const QString& paramName, const QString& units);
                void            SaveParameterResult    (int waferID,int siteID, QString name, QString strUnits, QString& value);
                void            Extract                (QTextStream &hWatAsmcFile, QString* toFill, bool append = false);
        };
    }
}

#endif // IMPORTWATASMC_H

