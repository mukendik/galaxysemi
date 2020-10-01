#ifndef VISHAY_STDF_RECORD_OVERWRITE_H
#define VISHAY_STDF_RECORD_OVERWRITE_H

#include "parserPromisFile.h"
#include "parserBase.h"
#include "bin_map_store_factory.h"
#include "queryable_by_test_number_store.h"
#include "queryable_by_bin_name_store.h"
#include "validity_check_for_bin.h"

namespace GS
{
namespace Parser
{
    class VishayStdfRecordOverWrite
    {
        public :


            enum Type {
                finalTestType = 0,
                waferType,
                eTestType,
                undefinedType
            };

            VishayStdfRecordOverWrite();

            bool UpdateMIRRecordWithPromis(GQTL_STDF::Stdf_MIR_V4& aMIR, const QString& aPromisKey,
                                           QString& aErrorMsg);
            bool UpdateSBRRecordWithPromis(GQTL_STDF::Stdf_SDR_V4 &aSdrData);
            bool UpdatePRRRecordWithBinMapping(GQTL_STDF::Stdf_PRR_V4& aPRR, int aFailTest, QString &aErrorMsg, const QString &aBinMapError="");
            bool readPTRRecordWithBinMapping(GQTL_STDF::Stdf_PTR_V4 &aPTR, int& aLastFailTest, QString& aErrorMsg);

            bool LoadExternalFiles(const QString& aFile, Qx::BinMapping::BinMapStoreAccessMode aAccessMode, QString &lErrorMsg, int &aLastError);

            bool IsBinCorrect(int aNumBin, const QString& aBinName) const;
            QPair<QString, QString> RetrieveBinNameAndSource(int aNumBin) const;


            Qx::BinMapping::BinMapStoreTypes GetFormat() const { return mBinMapStoreFormat;}
            Type GetType() const { return mTypeExternalFile;}
            QString GetPathExternalFile() const { return mPathExternalFile;}


            bool WritePromisGrossDieCount(GQTL_STDF::Stdf_DTR_V4 &aData);
            void WritePromisDieTracking(QList<GQTL_STDF::Stdf_DTR_V4 *> &aDtrRecords);
            void UpdateSBRRecord(GQTL_STDF::Stdf_SBR_V4 &aSBR, const ParserBinning &aBin);
            void UdpateHBRRecord(GQTL_STDF::Stdf_HBR_V4 &aHBR, const ParserBinning &aBin);


            bool RetrieveBinMapItemAndUpdateBinning(int &iBin_Soft, int lTestFailed);
            QString TypeToString(Type lType);

            void GetSoftBins(QList<QSharedPointer<GQTL_STDF::Stdf_SBR_V4> > &aSoftBins);
            void UpdateBin(const GQTL_STDF::Stdf_SBR_V4 &aRecord);
            QMap<int, ParserBinning> mSoftBinning;

            QString            mFinalTestFile;
            QString            mSortEntriesFile;
            void setDefaultBinValues(const QString& aDefaultBinName, stdf_type_u2 aDefaultBinNumber);

    private:
            ParserPromisFile   mParserPromis;
            QString            mPathExternalFile;
            int                mPassHardBin;

            Type               mTypeExternalFile;
            Qx::BinMapping::BinMapStoreTypes mBinMapStoreFormat;
            QMap<int, ParserBinning> mHardinning;
            QScopedPointer< Qx::BinMapping::QueryableByTestNumber > mBinMapStoreByTestNumber;
            QString            mLastBinMappingFileName;

            QScopedPointer< Qx::BinMapping::ValidatableByBin >  mBinMapStoreFinalTestChecker;
            QScopedPointer< Qx::BinMapping::ValidatableByBin >  mBinMapStoreSortEntriesChecker;

            QString				mDefaultBinMappingName;
            int					mDefaultBinMappingNumber;
            bool				mDefaultBinMappingValuesSet;

            void UpdateSoftBinMap(int aBinNum, const QString &aBinName, bool aIsPassBin);
           // bool IsTestNumberContainedInItemInBinMapStore(int aTestNumber) const;
            bool LoadBinmappingFile(const QString& aFile, Type aType, Qx::BinMapping::BinMapStoreTypes aBinMapType, Qx::BinMapping::BinMapStoreAccessMode aAccessMode, const QString &aCat, QString &lOutError);
            bool LoadPromisFile(const QString &aPromisKey, QString &aOutErrorMsg);

            bool ReadBinMapFile_LVM_WS(const QString &aConverterExternalFilePath, const QString& aBinmapFileName, QString &lOutError);
            bool ReadBinMapFile_HVM_WS_FETTEST(const QString &aConverterExternalFilePath, const QString& aBinmapFileName, QString &lOutError);
            bool ReadBinMapFile_HVM_WS_SPEKTRA(const QString &aConverterExternalFilePath, const QString& aBinmapFileName, QString &lOutError);
            bool ReadBinMapFile_HVM_FT(const QString& aBinmapFileName, QString &lOutError);
            bool ReadBinMapFile_LVM_FT_FT(const QString &aConverterExternalFilePath, const QString& aBinmapFileName, Qx::BinMapping::BinMapStoreAccessMode aAccessMode, QString &lOutError);
            bool ReadBinMapFile_LVM_FT_SE(const QString &aConverterExternalFilePath, const QString& aBinmapFileName, QString &lOutError);
            bool LoadPromisFile(const QString &aPromisLotID, const QString& aPath,
                                const QString& aType,
                                const QString& aMode,
                                QString &aOutExternalPromisFile,
                                QString &aOutExternalFileFormat,
                                QString &aOutExternalFileError);

            void SetType(const QString &lType);
            bool SetFormat(Type lType, const QString &lFormat);
            bool DefineTypefromExternalFile(QString &aError);
            bool DefineFormatfromExternalFile(Type lType, QString &aError);
            QStringList LoadCategoriesfromExternalFile(QString &aError);
            bool LoadSortEntrieBinmappingFile(const QString &aFile, Type lType, QString &lOutError);
            QString RetrieveExternalFilePath(const QString &aFile,  Type lType, const QString aCategory, QString &lOutError);
    };
}
}

#endif // VISHAY_STDF_RECORD_OVERWRITE_H
