#ifndef IMPORTMEXICALIMAP_H
#define IMPORTMEXICALIMAP_H

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

        class MexicaliMap : public ParserBase
        {
        public:

            enum  ErrCodesMexicaliMap
            {
                errInvalidMapCharacter  = errParserSpecific + 1    /// \param Invalid character detected in Map
            };

            enum T_MEXICALIFIELD { T_MASK = 0, T_LOTID, T_WAFERID, T_FORIENT, T_MAXXY, T_TOTDIE, T_TESTED, T_PICKABLE, T_WSTART, T_WEND, T_MAPREV};

            MexicaliMap();
            ~MexicaliMap();

            bool	ConvertoStdf(const QString &woburnSECSIIMapFileName, QString &StdfFileName);

            /**
            * @brief inform if this parser can be used in the pat updater map process
            */
            bool    IsCompatibleMapUpdater() const {return true;}

            /**
            * @brief inform if the the value of the Die imply that the die can be pat proceessing
            */
            static bool IsPatValidDie       (QChar charBin);

            /**
            * @brief inform if the value is a die
            */
            static bool IsInTheCoordinateSystem           (QChar lCharBin);

            /**
            * @brief Update information in the map date
            */
            static void UpdateMapData       (QStringList& fileInList, const QString& newValue);

            /**
            * @brief inform if the file is compatible with the format handle by the parser
            * @param the file
            */
            static bool	IsCompatible        (const QString &FileName);

            /**
            * @brief CopyHeader - Copy the hearder of the input file into the outputFile
            * @param inputStream the original inputfile
            * @param outPutStream the ouptu file that is a copy of the original. Use when parameters have to be updated
            */
            static void CopyHeader          (QTextStream& inputStream, QStringList &outPutStream);

            /**
            * @brief Set the offset to the origin when the origin is not the x0, y0 of map
            */
            static void SetOffsetOrigin        (int &xOffset, int &yOffset, int lX, int lY);

            static bool IsReverseXRead          () { return true;}

        protected:
            /**
             * \fn std::string GetErrorMessage()
             * \brief Getter
             * \return The error message corresponding to the given error code
             */
             std::string GetErrorMessage(const int ErrorCode) const;

        private:

            /**
             * @brief initialize the list of the keys that must be present in the header file
             * to fullfile the compatibility
             */
            static void             initCompatibleKeys();
            static QVector<QString> mIsCompatibleKeys;

            bool        GetDateFromString       (QString dateString, gstime &dataTime);
            bool        ReadMexicaliMapFile     (const QString &mexicaliMapFile, const QString &);
            bool        WriteStdfFile           (QTextStream &mexicaliMapFile, const QString &StdfFileName);
            bool        IsNotTestedDie          (const QChar &value);
            bool        IsInvalidDie            (const QChar &value);

            QFile       mFile;
            QTextStream mMexicaliTextStream;
            QString     mStrLastErrorSpecification;
            QString     mLotId;
            QString     mPartType;
            QString     mWaferId;
            QString     mMapRev;
            gstime        mWaferEndTime;
            QChar       mOrient;
            int         mWaferRows;
            int         mWaferColumns;
            int         mTotDie;
            int         mTested;
            int         mPickable;
        };

        template<>
        struct ParserDef<typeMexicaliMap>
        {
            typedef MexicaliMap         Parser;
        };
    }
}

#endif // IMPORTMEXICALIMAP_H

