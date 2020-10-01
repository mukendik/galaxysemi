#include <QFileInfo>
#include <gqtl_log.h>
#include "stdfrecords_v4.h"
#include "bin_map_item_base.h"
#include "converter_external_file.h"
#include "bin_mapping_exceptions.h"

#include "vishay_stdf_record_overwrite.h"

const QString cFinalTest = "final_tests";
const QString cSortEntries = "sort_entries";
const char cPassChar = 'P';
const char cFailChar = 'F';
const QString cFormat("format");
const QString cType("type");
const QString cCategory("category");
const QString cPassBinName("PASS");

namespace GS
{
namespace Parser
{

VishayStdfRecordOverWrite::VishayStdfRecordOverWrite():
    mTypeExternalFile(undefinedType),
    mLastBinMappingFileName(""),
    mDefaultBinMappingName(""),
    mDefaultBinMappingNumber(9999),
    mDefaultBinMappingValuesSet(false)
{
}

bool VishayStdfRecordOverWrite::SetFormat(Type lType, const QString& lFormat)
{
    if(lType == finalTestType)
    {
        if(lFormat.contains("hvm"))
        {
            mBinMapStoreFormat = Qx::BinMapping::hvm_ft;
        }
        else if(lFormat.contains("lvm"))
        {
            mBinMapStoreFormat = Qx::BinMapping::lvm_ft_ft;
        }
    }
    if(lType == waferType)
    {
        if(lFormat.contains("hvm"))
        {
            mBinMapStoreFormat = Qx::BinMapping::hvm_ws_fet_test;
        }
        else if(lFormat.contains("lvm"))
        {
            mBinMapStoreFormat = Qx::BinMapping::lvm_ws;
        }
    }

    return true;

}

void VishayStdfRecordOverWrite::SetType(const QString& lType)
{
    if(lType.toLower() == QStringLiteral("final")) mTypeExternalFile = finalTestType;
    else if(lType.toLower() == QStringLiteral("wafer")) mTypeExternalFile = waferType;
    else if(lType.toLower() == QStringLiteral("etest")) mTypeExternalFile = eTestType;
    else mTypeExternalFile = undefinedType;
}

bool VishayStdfRecordOverWrite::LoadPromisFile(const QString& aPromisKey, QString &aOutErrorMsg)
{
    QString lOutExternalPromiseFile;
    QString lOutExternalFileFormat;

    return
        LoadPromisFile( aPromisKey,
                        mPathExternalFile,
                        TypeToString(mTypeExternalFile),
                        QStringLiteral("prod"),
                        lOutExternalPromiseFile,
                        lOutExternalFileFormat,
                        aOutErrorMsg );
}

void VishayStdfRecordOverWrite::GetSoftBins(QList<QSharedPointer<GQTL_STDF::Stdf_SBR_V4> > & aSoftBins)
{
    QMap<int, ParserBinning>::iterator lIter (mSoftBinning.begin()), lIterEnd(mSoftBinning.end());
    for(;lIter != lIterEnd; ++lIter)
    {
        ParserBinning &lParserBinning = lIter.value();

        QSharedPointer<GQTL_STDF::Stdf_SBR_V4> lRecord(new GQTL_STDF::Stdf_SBR_V4);
        lRecord->SetHEAD_NUM(255);
        lRecord->SetSITE_NUM(255);
        lRecord->SetSBIN_NUM(lIter.key());
        lRecord->SetSBIN_CNT(lParserBinning.GetBinCount());

        if(lParserBinning.GetPassFail())
        {
            lRecord->SetSBIN_PF(cPassChar);
        }
        else
        {
            lRecord->SetSBIN_PF(cFailChar);
        }

        lRecord->SetSBIN_NAM(lParserBinning.GetBinName().toLatin1().constData());
        aSoftBins.push_back(lRecord);
    }
}


bool VishayStdfRecordOverWrite::LoadPromisFile(const QString &aPromisLotID,
                                               const QString &aPath,
                                                const QString &aType,
                                                const QString &aMode,
                                                QString& aOutExternalPromisFile,
                                                QString& aOutExternalFileFormat,
                                                QString& aOutExternalFileError)
{
    if(ConverterExternalFile::GetPromisFile(aPath, aType, aMode, aOutExternalPromisFile, aOutExternalFileFormat, aOutExternalFileError) )
    {
        if(!aOutExternalPromisFile.isEmpty())
        {
            bool lReadPromisResult = mParserPromis.ReadPromisDataFile( aOutExternalPromisFile,
                                                                       aType,
                                                                       aOutExternalFileFormat,
                                                                       aPromisLotID,
                                                                       ConverterExternalFile::GetExternalFileName( aPath ) );

            if( ! lReadPromisResult )
                aOutExternalFileError = mParserPromis.GetLastErrorMessage();

            return lReadPromisResult;
        }
    }

    return false;
}


QString  VishayStdfRecordOverWrite::RetrieveExternalFilePath(const QString& aFile,
                                                             Type lType,
                                                             const QString aCategory,
                                                             QString &lOutError)
{
    QFileInfo clFile(aFile);
    QString lPathExternalFile = clFile.absolutePath();

    // Check if converter_external_file exists
    QString lOutExternalBinFile;
    QString lOutExternalFileFormat;

    // Check if Test->Binning mapping file to overload softbin
    ConverterExternalFile::GetBinmapFile( lPathExternalFile, TypeToString(lType), "prod", aCategory,
                                          lOutExternalBinFile, lOutExternalFileFormat, lOutError);

    return lOutExternalBinFile;
}

bool VishayStdfRecordOverWrite::LoadSortEntrieBinmappingFile(const QString& aFile,
                                                             Type lType,
                                                             QString &lOutError)
{
    QString lOutExternalBinFile = RetrieveExternalFilePath(aFile, lType, cSortEntries, lOutError);
    const QString &lConverterExternalFilePath = ConverterExternalFile::GetExternalFileName( QFileInfo( aFile ).absolutePath() );
    if(lOutError.isEmpty() == false)
    {
        return false;
    }

    mSortEntriesFile = lOutExternalBinFile;
    if(lType == finalTestType)
    {
        return ReadBinMapFile_LVM_FT_SE(lConverterExternalFilePath, lOutExternalBinFile, lOutError) ;
    }

    return true;
}

bool VishayStdfRecordOverWrite::LoadBinmappingFile(const QString& aFile,
                                                            Type aType,
                                                            Qx::BinMapping::BinMapStoreTypes aBinMapType,
                                                            Qx::BinMapping::BinMapStoreAccessMode aAccessMode,
                                                            const QString& aCat,
                                                            QString &lOutError)
{
    QString lOutExternalBinFile = RetrieveExternalFilePath(aFile, aType, aCat, lOutError);
    const QString &lConverterExternalFilePath = ConverterExternalFile::GetExternalFileName( QFileInfo( aFile ).absolutePath() );

    if(lOutError.isEmpty() == false)
    {
        return false;
    }

    if(!lOutExternalBinFile.isEmpty())
    {
        mFinalTestFile = lOutExternalBinFile;
        if(aType == finalTestType)
        {
            if(aBinMapType == Qx::BinMapping::lvm_ft_ft)
            {
                return ReadBinMapFile_LVM_FT_FT(lConverterExternalFilePath, lOutExternalBinFile, aAccessMode, lOutError) ;
            }
        }
        else if(aType == waferType)
        {
            if(aBinMapType == Qx::BinMapping::lvm_ws)
            {
                return ReadBinMapFile_LVM_WS(lConverterExternalFilePath, lOutExternalBinFile, lOutError);
            }
            else if(aBinMapType == Qx::BinMapping::hvm_ws_fet_test)
            {
                return ReadBinMapFile_HVM_WS_FETTEST(lConverterExternalFilePath, lOutExternalBinFile, lOutError);
            }
            else if(aBinMapType == Qx::BinMapping::hvm_ws_spektra)
            {
                return ReadBinMapFile_HVM_WS_SPEKTRA(lConverterExternalFilePath, lOutExternalBinFile, lOutError);
            }
        }
        return false;
    }

    return true;
}

bool VishayStdfRecordOverWrite::ReadBinMapFile_LVM_WS(const QString &aConverterExternalFilePath,
                                                      const QString& aBinmapFileName, QString &lOutError)
{
    mLastBinMappingFileName = aBinmapFileName;
    try
    {
        mBinMapStoreByTestNumber.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ws >
                            ::MakeBinMapStore( aBinmapFileName.toStdString(),
                                               aConverterExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        lOutError = lException.what();
        return false;
    }

    return true;
}

bool VishayStdfRecordOverWrite::ReadBinMapFile_HVM_WS_FETTEST(const QString &aConverterExternalFilePath,
                                                              const QString &aBinmapFileName, QString &lOutError)
{
    mLastBinMappingFileName = aBinmapFileName;
    try
    {
        mBinMapStoreByTestNumber.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::hvm_ws_fet_test >
                            ::MakeBinMapStore( aBinmapFileName.toStdString(),
                                               aConverterExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        lOutError = lException.what();
        return false;
    }

    return true;
}

bool VishayStdfRecordOverWrite::ReadBinMapFile_HVM_WS_SPEKTRA(const QString &aConverterExternalFilePath,
                                                              const QString &aBinmapFileName, QString &lOutError)
{
    mLastBinMappingFileName = aBinmapFileName;
    try
    {
        mBinMapStoreByTestNumber.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::hvm_ws_spektra >
                            ::MakeBinMapStore( aBinmapFileName.toStdString(),
                                               aConverterExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        lOutError = lException.what();
        return false;
    }

    return true;
}

bool VishayStdfRecordOverWrite::ReadBinMapFile_LVM_FT_FT(const QString &aConverterExternalFilePath,
                                                         const QString& aBinmapFileName,
                                                         Qx::BinMapping::BinMapStoreAccessMode aAccessMode,
                                                         QString &lOutError)
{
    mLastBinMappingFileName = aBinmapFileName;
    try
    {
        if(aAccessMode ==  Qx::BinMapping::queryable)
        {
            mBinMapStoreByTestNumber.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ft_ft, Qx::BinMapping::queryable >
                                ::MakeBinMapStore( aBinmapFileName.toStdString(),
                                                   aConverterExternalFilePath.toStdString() ) );
        }
        else if(aAccessMode == Qx::BinMapping::validity_check)
        {
            mBinMapStoreFinalTestChecker.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ft_ft, Qx::BinMapping::validity_check >
                                               ::MakeBinMapStore( aBinmapFileName.toStdString(),
                                                                  aConverterExternalFilePath.toStdString() ) );
        }
    }
    catch( const std::exception &lException )
    {
        lOutError = lException.what();
        return false;
    }

    return true;
}

bool VishayStdfRecordOverWrite::ReadBinMapFile_LVM_FT_SE(const QString &aConverterExternalFilePath,
                                                         const QString& aBinmapFileName,
                                                         QString &lOutError)
{
    try
    {
        mBinMapStoreSortEntriesChecker.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ft_se_new, Qx::BinMapping::validity_check >
                                              ::MakeBinMapStore( aBinmapFileName.toStdString(),
                                                                 aConverterExternalFilePath.toStdString() ) );
    }
    catch( const std::exception &lException )
    {
        lOutError = lException.what();
        return false;
    }

    return true;
}

bool VishayStdfRecordOverWrite::DefineFormatfromExternalFile(Type lType, QString &aError)
{
    QStringList lFormatList = ConverterExternalFile::LoadAttributs(mPathExternalFile, cFormat, aError);

    // External file are loaded only if only one type is defined.
    // Inded since the format has to be deduce from the external file they can't be more than once
    if(lFormatList.size() == 1)
    {
        return SetFormat(lType, lFormatList[0]) ;
    }

    return false;
}

QStringList VishayStdfRecordOverWrite::LoadCategoriesfromExternalFile(QString &aError)
{
    QStringList lCategoriesList = ConverterExternalFile::LoadAttributs(mPathExternalFile, cCategory, aError);

    return lCategoriesList;
}

bool VishayStdfRecordOverWrite::DefineTypefromExternalFile(QString &aError)
{
    QStringList lTypeList = ConverterExternalFile::LoadAttributs(mPathExternalFile, cType, aError);

    // External file are loaded only if only one type is defined.
    // Inded since the type has to be deduce from the external file they can't be more than once
    if(lTypeList.size() == 1)
    {
        QString lType = lTypeList[0];

        SetType(lType);

        if(mTypeExternalFile == undefinedType)
        {
             return false;
        }
        return true;
    }

    return false;
}


QPair<QString, QString> VishayStdfRecordOverWrite::RetrieveBinNameAndSource(int aNumBin) const
{
    QPair<QString, QString> lNameAndSource = qMakePair(QString(), QString());

    if(mBinMapStoreFinalTestChecker.isNull() == false)
    {
        try
        {
            lNameAndSource.first = QString(mBinMapStoreFinalTestChecker->RetrieveBinName(aNumBin).c_str());
            lNameAndSource.second = cFinalTest;
        }
        catch (const Qx::BinMapping::BinMappingExceptionForUserBase & )
        {
            // DONT DO ANYTHING HERE WHEN TEST DOES NOT HAVE A MAPPED ENTRY IN BINMAP FILE
        }
    }

    if(lNameAndSource.first.isEmpty() && mBinMapStoreSortEntriesChecker.isNull() == false)
    {
        try
        {
            lNameAndSource.first  = QString(mBinMapStoreSortEntriesChecker->RetrieveBinName(aNumBin).c_str());
            lNameAndSource.second = cSortEntries;
        }
        catch (const Qx::BinMapping::BinMappingExceptionForUserBase & )
        {
            // DONT DO ANYTHING HERE WHEN TEST DOES NOT HAVE A MAPPED ENTRY IN BINMAP FILE
        }
    }

    return lNameAndSource;
}

bool VishayStdfRecordOverWrite::IsBinCorrect(int aNumBin, const QString& aBinName) const
{
    bool lFound = false;
    if(mBinMapStoreFinalTestChecker.isNull() == false)
    {
        lFound = mBinMapStoreFinalTestChecker->IsBinCorrect(aNumBin, aBinName.toStdString());
    }

    if(lFound == false && mBinMapStoreSortEntriesChecker.isNull() == false)
    {
        lFound = mBinMapStoreSortEntriesChecker->IsBinCorrect(aNumBin, aBinName.toStdString());
    }

    return lFound;
}


bool VishayStdfRecordOverWrite::LoadExternalFiles(const QString& aFile,
                                                  Qx::BinMapping::BinMapStoreAccessMode aAccessMode,
                                                  QString& lErrorMsg,
                                                  int& aLastError)
{
    lErrorMsg.clear();
    QFileInfo clFile(aFile);
    mPathExternalFile = clFile.absolutePath();
    if(ConverterExternalFile::Exists(mPathExternalFile))
    {
        if(DefineTypefromExternalFile(lErrorMsg) == false)
        {
            return false;
        }

        if(DefineFormatfromExternalFile (mTypeExternalFile, lErrorMsg) == false)
        {
            return false;
        }

        QStringList lCategories = LoadCategoriesfromExternalFile(lErrorMsg);

        if(lErrorMsg.isEmpty() == false)
        {
            return false;
        }

        /// Load sortEntri. Even if pb during the loading, continue.
        if(lCategories.contains(cSortEntries))
        {
            if(LoadSortEntrieBinmappingFile(aFile, GetType(), lErrorMsg) == false)
            {
                GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
                return false;
            }
        }

        QString lSearchedCat;
        /// Load sortEntri. Even if pb during the loading, continue.
        if(lCategories.contains(cFinalTest))
        {
            lSearchedCat = cFinalTest;
        }

        /// Load binMapFile Final test.
        if(LoadBinmappingFile(aFile, GetType(), mBinMapStoreFormat, aAccessMode, lSearchedCat, lErrorMsg) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, qPrintable(lErrorMsg));
            return false;
        }

        return true;
    }

    aLastError = ParserBase::errInvalidFormatParameter;
    return false;
}

void VishayStdfRecordOverWrite::UpdateSBRRecord(GQTL_STDF::Stdf_SBR_V4 &aSBR, const ParserBinning &aBin)
{
    aSBR.Reset();
    aSBR.SetHEAD_NUM(255);						// Test Head = ALL
    aSBR.SetSITE_NUM(255);						// Test sites = ALL
    aSBR.SetSBIN_NUM(aBin.GetBinNumber());		// HBIN
    aSBR.SetSBIN_CNT(aBin.GetBinCount());		// Total Bins
    if(aBin.GetPassFail())
    {
        aSBR.SetSBIN_PF(cPassChar);
        aSBR.SetSBIN_NAM(cPassBinName);
    }
    else
    {
        aSBR.SetSBIN_PF(cFailChar);
        aSBR.SetSBIN_NAM(aBin.GetBinName());
    }
}

void VishayStdfRecordOverWrite::UdpateHBRRecord(GQTL_STDF::Stdf_HBR_V4 &aHBR, const ParserBinning &aBin)
{
    aHBR.Reset();
    aHBR.SetHEAD_NUM(255);						// Test Head = ALL
    aHBR.SetSITE_NUM(255);						// Test sites = ALL
    aHBR.SetHBIN_CNT(aBin.GetBinCount());		// Total Bins
    if(aBin.GetPassFail())
    {
        aHBR.SetHBIN_NUM(mPassHardBin);		// HBIN
        aHBR.SetHBIN_PF(cPassChar);
        aHBR.SetHBIN_NAM(cPassBinName);
    }
    else
    {
        aHBR.SetHBIN_NUM(aBin.GetBinNumber());		// HBIN
        aHBR.SetHBIN_PF(cFailChar);
        aHBR.SetHBIN_NAM(aBin.GetBinName());
    }
}

/*
 * note on variables used in updatePRR...():
 * - 'aFailTest' is last fail test (did the last PIR/PRR suite contained at least one fail PTR?)
 * - 'lFailPart' is current PRR final Pass/Fail status, independantly from PTRs
 * - 'lOutFailBin' will be the bin finally mapped in the end, after all Vishay specific workflow has been applied (GCore 16654)
 * For algo details, check http://orw-qx-conf-01:8090/confluence/pages/viewpage.action?pageId=141296014#VishayMappingformats&algorithms-LVMAMKORQA:stdf(std)(P_FT_LVM1,B_FT_LVM)
*/
bool VishayStdfRecordOverWrite::UpdatePRRRecordWithBinMapping(GQTL_STDF::Stdf_PRR_V4 &aPRR,
                                                              int aFailTest,
                                                              QString& aErrorMsg,
                                                              const QString& aBinMapError/*=""*/)
{
    if(mBinMapStoreByTestNumber.isNull() == false)
    {
        // if Invalid PRR do not update
        if((aPRR.m_u2HARD_BIN == STDF_MAX_U2) && (aPRR.m_u2SOFT_BIN == STDF_MAX_U2)) return true;

        int lOutFailBin = -1;
        bool lPartIsGood = false;

        if ((aPRR.m_b1PART_FLG & STDF_MASK_BIT4) == 0)
        {
            lPartIsGood = true;
            if (aPRR.m_b1PART_FLG & STDF_MASK_BIT3)
            {
                lPartIsGood = false;
            }
        }

        if(!lPartIsGood)
        {
            //Part is a fail part
            if(aFailTest > 0)
            {
                if(RetrieveBinMapItemAndUpdateBinning(lOutFailBin, aFailTest))
                {   /// Apply bin from the binmap
                    aPRR.SetSOFT_BIN(static_cast<stdf_type_u2>(lOutFailBin));
                    aPRR.SetHARD_BIN(static_cast<stdf_type_u2>(lOutFailBin));
                    return true;
                }
                else
                {
                    //test case 5 of gcore 16654
                    aErrorMsg = "Unknown test number " + QString::number(aFailTest) + " in the bin map file "
                                + mLastBinMappingFileName + aBinMapError;
                    return false;
                }
            }
            else
            {
                /* aFailTest == -1 : no fail among PTRs -> test case 1 of gcore 16654
                 * or because last fail test had no number -> test case 3
                 * aFailTest == 0 : last failed test had no number -> test case 4
                 * anyway, let's check if we can use default bin mapping value
                 *
                 */
                if(mDefaultBinMappingValuesSet)
                {
                    //test case 1a of gcore 16654 (+test case 3)
                    UpdateSoftBinMap(mDefaultBinMappingNumber, mDefaultBinMappingName, false);
                    aPRR.SetSOFT_BIN(mDefaultBinMappingNumber);
                    aPRR.SetHARD_BIN(mDefaultBinMappingNumber);
                }
                else
                {
                    //test case 1b of gcore 16654 (+test case 3)
                    aErrorMsg = "PartID " + QString(aPRR.m_cnPART_ID)
                               + " is FAIL, but all tests PASS, and there is no default bin mapping value defined in "
                               + ConverterExternalFile::GetExternalFileName(mPathExternalFile);
                    return false;
                }
            }
        }
        else
        {
            mPassHardBin = aPRR.m_u2HARD_BIN;
            UpdateSoftBinMap(aPRR.m_u2SOFT_BIN, "", true);
        }
    }
    else
    {
        aErrorMsg = "Error no Bin Map loaded";
        return false;
    }
    return true;
}

bool VishayStdfRecordOverWrite::readPTRRecordWithBinMapping(GQTL_STDF::Stdf_PTR_V4 &aPTR, int& aLastFailTest, QString& aErrorMsg)
{
    if (mTypeExternalFile == VishayStdfRecordOverWrite::finalTestType)
    {
        // extract test number from test name
        QString lStringNumber = QString(aPTR.m_cnTEST_TXT).section(" ", 0, 0);
        bool lOk = false;
        int tempNumber =lStringNumber.toInt(&lOk);
        if (lOk == false || tempNumber == 0)
        {
            /*
             * either test case 3 of gcore 16654: test fail has number == 0
             * or test case 4 with a fail test that has no number (this was formerly
             *  treated as an error, but now must simply be ignored: if there is no
             *  other fail test that has a number, default bin value will be taken for part)
             */
            aErrorMsg = "Unable to extract test # from test name: " + QString(aPTR.m_cnTEST_TXT)
                        + " test is failed and ignored";
        }
        else
        {
            try
            {
                const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStoreByTestNumber->GetBinMapItemByTestNumber( tempNumber );
                //lItem.GetSoftBinNumber();//don't need that, just needed to check if there is a mapping
                if(lItem.GetEnabled())
                {
                    //...and if its enabled

                    //case 6 of gcore 16654
                    aLastFailTest = tempNumber;
                }
                else{
	                //else do nothing for now: it will hopefully have default bin number
    	            //during PRR's treatment -> case 2 of gcore 16654
                }
            }
            catch (const Qx::BinMapping::BinMappingExceptionForUserBase &)
            {
                //case ? of gcore 16654
                aLastFailTest = tempNumber;
            }
            catch( ... )
            {
                aErrorMsg = "Found unknown error with test number " + QString(tempNumber) + " during bin mapping defined in the "
                           + ConverterExternalFile::GetExternalFileName(mPathExternalFile);
                return false;
            }
        }
    }
    else
    {
        aLastFailTest = static_cast<int>(aPTR.m_u4TEST_NUM);
    }

    return true;
}

bool VishayStdfRecordOverWrite::UpdateMIRRecordWithPromis(GQTL_STDF::Stdf_MIR_V4& aMIR, const QString& aPromisKey,
                                                          QString& aErrorMsg)
{
    if (LoadPromisFile(aPromisKey, aErrorMsg))
    {
        if(mParserPromis.GetProcID().isEmpty() == false)
        {
            aMIR.SetPROC_ID(mParserPromis.GetProcID());
        }

        if(mParserPromis.GetDateCode().isEmpty() == false)
        {
            aMIR.SetDATE_COD(mParserPromis.GetDateCode());
        }

        if(mParserPromis.GetRomCode().isEmpty() == false)
        {
            aMIR.SetROM_COD(mParserPromis.GetRomCode());
        }

        if(mParserPromis.GetEquipmentID().isEmpty() == false)
        {
            aMIR.SetNODE_NAM(mParserPromis.GetEquipmentID());
        }

        if(mParserPromis.GetTesterType().isEmpty() == false)
        {
            aMIR.SetTSTR_TYP(mParserPromis.GetTesterType());
        }

        if(mParserPromis.GetSublotId().isEmpty() == false)
        {
            aMIR.SetSBLOT_ID(mParserPromis.GetSublotId());
        }

        if(mParserPromis.GetTestCode().isEmpty() == false)
        {
            aMIR.SetTEST_COD(mParserPromis.GetTestCode());
        }

        if(mParserPromis.GetProductId().isEmpty() == false)
        {
            aMIR.SetPART_TYP(mParserPromis.GetProductId());
        }

        if(mParserPromis.GetFacilityID().isEmpty() == false)
        {
            aMIR.SetFACIL_ID(mParserPromis.GetFacilityID());
        }

        if(mParserPromis.GetPackageType().isEmpty() == false)
        {
            aMIR.SetPKG_TYP(mParserPromis.GetPackageType());
        }

        return true;
    }

    return false;
}

bool VishayStdfRecordOverWrite::WritePromisGrossDieCount(  GQTL_STDF::Stdf_DTR_V4 &aData)
{
     if(mParserPromis.GetGrossDieCount() > 0)
     {
        aData.SetTEXT_DAT( "<cmd> gross_die=" + QString::number(mParserPromis.GetGrossDieCount()));
        return true;
     }

     return false;
}

bool VishayStdfRecordOverWrite::UpdateSBRRecordWithPromis(GQTL_STDF::Stdf_SDR_V4& aSdrData)
{
    if(!mParserPromis.GetEquipmentID().isEmpty())
    {
        aSdrData.SetEXTR_ID(mParserPromis.GetEquipmentID().toLatin1().constData());
        return true;
    }

    return false;
}

void VishayStdfRecordOverWrite::WritePromisDieTracking(QList<GQTL_STDF::Stdf_DTR_V4*>&	aDtrRecords)
{
    // Write die-tracking DTRs
    if(!mParserPromis.GetPackageType().isEmpty())
    {
        QString lTemp = mParserPromis.GetSublotId().section('.', 0, 0);

        GQTL_STDF::Stdf_DTR_V4* lDtrRecord = new GQTL_STDF::Stdf_DTR_V4();
        QString lStrData = "<cmd> die-tracking die=1;wafer_product=" + mParserPromis.GetProductId();
        lStrData += ";wafer_lot=" + lTemp;
        lStrData += ";wafer_sublot=" + mParserPromis.GetSublotId();

        lDtrRecord->SetTEXT_DAT(lStrData);
        aDtrRecords.push_back(lDtrRecord);

        if(!mParserPromis.GetPromisLotId_D2().isEmpty() && !mParserPromis.GetGeometryName_D2().isEmpty())
        {
            lTemp = mParserPromis.GetPromisLotId_D2().section('.', 0, 0);
            lStrData = "<cmd> die-tracking die=2;wafer_product=" + mParserPromis.GetGeometryName_D2();
            lStrData += ";wafer_lot=" + lTemp;
            lStrData += ";wafer_sublot=" + mParserPromis.GetPromisLotId_D2();
            lDtrRecord = new GQTL_STDF::Stdf_DTR_V4();
            lDtrRecord->SetTEXT_DAT(lStrData);
            aDtrRecords.push_back(lDtrRecord);

            if(!mParserPromis.GetPromisLotId_D3().isEmpty() && !mParserPromis.GetGeometryName_D3().isEmpty())
            {
                lTemp = mParserPromis.GetPromisLotId_D3().section('.', 0, 0);
                lStrData = "<cmd> die-tracking die=3;wafer_product=" + mParserPromis.GetGeometryName_D3();
                lStrData += ";wafer_lot=" + lTemp;
                lStrData += ";wafer_sublot=" + mParserPromis.GetPromisLotId_D3();
                lDtrRecord = new GQTL_STDF::Stdf_DTR_V4();
                lDtrRecord->SetTEXT_DAT(lStrData);
                aDtrRecords.push_back(lDtrRecord);

                if(!mParserPromis.GetPromisLotId_D4().isEmpty() && !mParserPromis.GetGeometryName_D4().isEmpty())
                {
                    lTemp = mParserPromis.GetPromisLotId_D4().section('.', 0, 0);
                    lStrData = "<cmd> die-tracking die=4;wafer_product=" + mParserPromis.GetGeometryName_D4();
                    lStrData += ";wafer_lot=" + lTemp;
                    lStrData += ";wafer_sublot=" + mParserPromis.GetPromisLotId_D4();

                    lDtrRecord = new GQTL_STDF::Stdf_DTR_V4();
                    lDtrRecord->SetTEXT_DAT(lStrData);
                    aDtrRecords.push_back(lDtrRecord);
                }
            }
        }
    }
}

bool VishayStdfRecordOverWrite::RetrieveBinMapItemAndUpdateBinning( int &iBin_Soft, int lTestFailed )
{
    try
    {
        QString lBinName;

        const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStoreByTestNumber->GetBinMapItemByTestNumber( lTestFailed );
        iBin_Soft = lItem.GetSoftBinNumber();
        lBinName = QString::fromStdString( lItem.GetBinName() );

        UpdateSoftBinMap(iBin_Soft, lBinName, false );
    }
    catch( ... )
    {
        return false;
    }

    return true;
}

void VishayStdfRecordOverWrite::UpdateSoftBinMap(int aBinNum, const QString &aBinName, bool aIsPassBin)
{
    if(mSoftBinning.contains(aBinNum) == false)
    {
        ParserBinning pBin;

        pBin.SetBinName(aBinName);
        pBin.SetBinNumber(static_cast<unsigned short>(aBinNum));
        pBin.SetBinCount(0);
        pBin.SetPassFail(aIsPassBin);
        mSoftBinning[aBinNum] = pBin;
    }

    mSoftBinning[aBinNum].IncrementBinCount(1);
}

void VishayStdfRecordOverWrite::UpdateBin(const GQTL_STDF::Stdf_SBR_V4& aRecord)
{
    if(mSoftBinning.contains(aRecord.m_u2SBIN_NUM))
    {
        if (mSoftBinning.value(aRecord.m_u2SBIN_NUM).GetBinName().isEmpty())
        {
            mSoftBinning[aRecord.m_u2SBIN_NUM].SetBinName(aRecord.m_cnSBIN_NAM);
        }
        if (aRecord.m_c1SBIN_PF == cFailChar)
            mSoftBinning[aRecord.m_u2SBIN_NUM].SetPassFail(false);
    }
}


QString VishayStdfRecordOverWrite::TypeToString(Type lType)
{
    if(lType == finalTestType) return  QStringLiteral("final");
    else if(lType == waferType) return QStringLiteral("wafer");
    else if(lType == eTestType) return QStringLiteral("etest");
    else return QStringLiteral("undefined");
}

void VishayStdfRecordOverWrite::setDefaultBinValues(const QString& aDefaultBinName, stdf_type_u2 aDefaultBinNumber)
{
    mDefaultBinMappingName = aDefaultBinName;
    mDefaultBinMappingNumber = aDefaultBinNumber;
    mDefaultBinMappingValuesSet = true;
}

}
}
