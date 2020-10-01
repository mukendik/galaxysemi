#ifndef IMPORTWOBURNSECSIIMAP
#define IMPORTWOBURNSECSIIMAP

#include <QVector>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QMap>

#include "stdf.h"
#include "parserBase.h"


namespace GS
{
    namespace Parser
    {

        class WoburnSECSIIMap : public ParserBase
        {
        public:
            WoburnSECSIIMap();
            ~WoburnSECSIIMap();

            bool	ConvertoStdf(const QString &woburnSECSIIMapFileName,  QString &StdfFileName);

            static bool IsPatValidDie               (QChar charBin);
            static void UpdateMapData               (QStringList& fileInList, const QString &newValue);
            static bool	IsCompatible                (const QString &FileName);
            static bool IsInTheCoordinateSystem     (QChar )  { return true;}
            static void SetOffsetOrigin             (int &, int &, int, int ) {}

            /**
             * @brief CopyHeader - Copy the hearder of the input file into the outputFile
             * @param inputStream the original inputfile
             * @param outPutStream the ouptu file that is a copy of the original. Use when parameters have to be updated
             */
            static void CopyHeader  (QTextStream& inputStream, QStringList &outPutStream);
            static bool IsReverseXRead() { return false;}

        private:

            static void             initCompatibleKeys();
            static QVector<QString> mIsCompatibleKeys;


            /**
             * @brief IsCompatibleMapUpdater
             * @return true if this file format is also compatible for the map updater
             */
            bool        IsCompatibleMapUpdater() const {return true;}

            bool        ReadWoburnSECSIIMapFile (const QString &woburnSECSIIMapFileName, const QString &);
            bool        WriteStdfFile           (QTextStream &woburnSECSIIMapFile, const QString &StdfFileName);
            bool        IsInvalidDie            (const QChar &value);

            QFile       mFile;
            QTextStream mWoburnSECSIITextStream;
            QString     mStrLastErrorSpecification;
            QString     mLotId;
            QString     mWaferId;
            QString     mTestInputMap;
            QString     mInkMap;
            QString     mMapRev;
            QChar       mPosX;
            QChar       mPosY;
            QChar       mOrient;
            int         mWaferRows;
            int         mWaferColumns;
            int         mTotDie;
            int         mTested;
            int         mPickable;


        };

        template<>
        struct ParserDef<typeWoburnSECSIIMap>
        {
            typedef WoburnSECSIIMap     Parser;
        };
    }
}

#endif // IMPORTWOBURNSECSIIMAP

