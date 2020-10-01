#ifndef IMPORTWATTSMXML_H
#define IMPORTWATTSMXML_H

#include <QFile>
#include "importWat.h"

namespace GS
{
    namespace Parser
    {
        class WatTsmXmlToStdf : public WatToStdf
        {
            public:
                WatTsmXmlToStdf();
                ~WatTsmXmlToStdf();

                static bool  IsCompatible(const QString& fileName);

        private:
                bool            ParseFile              (const QString &fileName);
                bool            WriteStdfFile          (const char *stdFileName);

                QString         ReadNextTag             ();
                bool            GotoMarker              (const char *szEndMarker);
                void            NormalizeValues         (QString &strUnits, double &fValue, int &nScale);

                QMap<int, WatParameter> mTests;
                QMap<QString,QString>	mTagAttributes;
                QString					mTagValue;
                QString					mTagName;
                QString                 mCurrentLine;
                QFile                   mFile;
                QTextStream             mWatTsmXmlFile;
                bool                    mEndOfFile;

                QString                 mSpecVer;
                QString                 mSpecName;
                QString					mProbId;
                QString					mEquipmentId;
                QString					mWaferFlat;
                QString					mOwnerName;

        };
    }
}

#endif // IMPORTWATTSMXML_H

