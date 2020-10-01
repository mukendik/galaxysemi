#ifndef IMPORT_KLA_H
#define IMPORT_KLA_H

#include "parserBase.h"

namespace GS
{
    namespace Parser
    {
        class ImportKLAMap : public ParserBase
        {
        public:
            ImportKLAMap();
            ~ImportKLAMap();

            /**
             * \fn bool ConvertoStdf(const QString &aInputFile, QString &aStdfFileName)
             * \brief Read the KLA Map file.
             * \param aInputFile is the input file, contains the KLA map file.
             * \param aStdfFileName is the output file, contains the stdf file.
             * \return true if the file has been successfully read. Otherwise return false.
             */
            bool ConvertoStdf(const QString &aInputFile, QString &aStdfFileName);

            /**
            * @brief inform if the file is compatible with the format handle by the parser
            * @param the file
            */
            static bool	IsCompatible(const QString &aInputFile);

            static QPair<QString, QString> ParseLine(const QString& aLine);

        private:

            bool            ConvertHeaderToFloat(const QString &lKey, const QString& lStringValue,
                                                 float& lConvertedValue);
            bool            ConvertHeaderToInteger(const QString &lKey, const QString& lStringValue,
                                                   int& lConvertedValue);
            bool            ConvertHeaderToIntegerArray(const QString &lKey, const QString& lStringValue,
                                                    QList<int>& lConvertedValue);
            bool            GenerateOutput(QTextStream& aTextStream, const QString &aStdfFileName);
            stdf_type_c1    GetSTDFWaferFlat() const;
            stdf_type_u1    GetSTDFWaferUnit() const;
            bool            ProcessDie(const QString& aDie, ushort& aBin);
            bool            ReadHeader(QTextStream& aTextStream);

            QString     mDevice;            //DEVICE:CU738BAR
            QString     mLot;               //LOT:R114792
            QString     mWafer;             //WAFER:R114792-02
            int         mFlatNotch;         //FNLOC:0
            int         mRows;              //ROWCT:42
            int         mColumns;           //COLCT:43
            QList<int>	mGoodBins;          //BCEQU:01 - Case 6571: we now also support a list, ie BCEQU:01 02 03 04
            int         mRefDieX;           //REFPX:4
            int         mRefDieY;           //REFPY:-45
            QString     mDiesUnit;          //DUTMS:mm
            float       mXDiesSize;         //XDIES:0
            float       mYDiesSize;         //YDIES:0

            QMap<int, ParserBinning>  mBinning;       /// \param List of Bin tables.
        };

    }
}

#endif // IMPORT_KLA_H
