#ifndef IMPORTWATSMIC_H
#define IMPORTWATSMIC_H

#include <QFile>

#include "gqtl_global.h"
#include "importWat.h"

namespace GS
{
    namespace Parser
    {
        class WatSmicToStdf : public WatToStdf
        {
            public:
                WatSmicToStdf();
                ~WatSmicToStdf();

              static bool  IsCompatible(const QString& fileName);

        private:
                QList<WatScaledParameter*>   mSmicParameterList;
                QFile                        mFile;
                QTextStream                  mWatSmicFile;

                bool            ParseFile              (const QString &fileName);
                bool            WriteStdfFile          (const char *stdFileName);
        };
    }
}

#endif // IMPORTWATSMIC_H

