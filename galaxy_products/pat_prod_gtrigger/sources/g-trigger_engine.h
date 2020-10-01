/****************************************************************************
** G-Trigger engine classes
****************************************************************************/

#ifndef G_TRIGGER_ENGINE_H
#define G_TRIGGER_ENGINE_H

#include <QObject>
#include <QTimer>

#include <pat_recipe_io.h>
#include "g-trigger.h"
#include "bin_map_store_factory.h"

class CStdf;

namespace GS
{
namespace GTrigger
{

enum ProdFlow
{
    eFlow_Lvm_FetTest_Pat,
    eFlow_Lvm_FetTest_NoPat,
    eFlow_Lvm_Eagle_Pat,
    eFlow_Lvm_Eagle_NoPat,
    eFlow_Hvm_FetTest_Pat,
    eFlow_Hvm_Spektra_Pat
};

class GTriggerEngine : public QObject
{
    Q_OBJECT

public:
    GTriggerEngine(const QString & AppName, const QString & AppPath);
    ~GTriggerEngine();
    /*!
     * \fn setQA
     */
    void setQA(bool b) { mQA = b; }

private:
    enum LogMsgType
    {
        eInfo,
        eWarning,
        eError
    };
    enum TesterType
    {
        eFetTest,
        eEagle,
        eSpektra
    };
    enum FileFormat
    {
        javascriptFormat,
        gtfFormat
    };
    enum SummaryInputFormat
    {
        SummaryInputFormatFetTest,
        SummaryInputFormatEagle,
        SummaryInputFormatSpektra
    };

    // Application name
    QString     mAppName;
    // Application path
    QString     mAppPath;

    QTimer		mTimer;
    QString		mPrnFolder;                 // Folder where .PRN files are found
    QString		mPrnFolderHVM;              // Folder where HVM .PRN files are found
    QString		mPrnFolderNoPat;            // Folder for .PRN NO-PAT files.
    QString		mPrnFolderNoPatHVM;         // Folder for HVM .PRN NO-PAT files.
    QString		mPrnRetestFolder;           // Folders where .PRN Retest files are found
    QString		mStdfInFolder;
    QString		mStdfEagleSumFolder;        // Eagle Summary STDF and associated files must be found here...
    // Eagle STDF are moved to this location as soon as the trigger file is created.
    QString		mStdfEagleInFolder;
    QString		mStdfEagleOutFolderPat;     // Eagle PAT STDF files should be created here
    QString		mStdfSpektraCntFolder;      // Spektra CNT files must be found here...
    // Spektra files converted to STDF will be located in this folder, where the GTF files will also be created.
    QString		mStdfSpektraInFolder;
    QString		mStdfSpektraOutFolderPat;   // Spektra PAT STDF files should be created here

    QString		mOutFolderLog;
    QString		mOutFolderInf;
    QString		mOutFolderStdf;
    QString		mOutFolderEngStdf;

    QString		mRecipeFolder;
    QString		mLog;
    QString		mProduction;
    QString		mInspectionMapsFolder;
    QString		mShell;
    QString		mShell_HVM;
    QString		mShell_Fet_NoPAT;
    QString		mShell_Fet_NoPAT_HVM;
    QString     mOutput_Pat_Composite_Maps_Subfolder;
    QString     mShell_Post_Composite;
    int			mDelay_Fet_NoPAT_LogFiles;
    int			mDeletePrn;
    int			mPrnDelay;
    int			mSummaryDelay;
    int			mUpdateTimeFields;
    bool        mFirstCall;
    bool        mSettingsModified;
    SummaryList	mSummaryList;
    float       mValidWaferThreshold_GrossDieRatio;
    bool        mDeleteIncompleteWafers;
    bool        mAddSiteLocationToInfOutputDir;
    QString     mDelayed_Files_Subfolder;
    QString     mRejected_Files_Subfolder;
    int         mLogLevel;
    bool        mMergeTestRetest;
    bool		mBinMapFileEnabled;
    QString     mBinningMapFile;
    QMap<QString, QDateTime>    mLastModifiedLogForSummary;
    /*!
     * \var mOutputTriggerFileFormat
     */
    int mOutputTriggerFileFormat;
    /*!
     * \var mMapMergeScript
     */
    QString mMapMergeScript;
    /*!
     * \var mQA
     */
    bool mQA;

    QScopedPointer< Qx::BinMapping::BinMapStoreContract > mBinMapStore;

    void		LoadINI(void);
    void		SetVariable(QString & strVariable, const QString & strValue);
    void		SetVariable(int & nVariable, const int & nValue);
    void		SetVariable(float & fVariable, const float & fValue);
    void		SetVariable(bool & bVariable, const bool & bValue);
    bool		openFile(QFile &fFile,QIODevice::OpenMode openMode,QString &strFileName, int iTimeoutSec=15);
    void		LogMessage(const QString &Message,LogMsgType MsgType=eInfo);
    void        LogMessage_Debug(const QString &strErrorMessage);
    void		InsertStatusLog(const QString & strItem, bool bAddTimestamp=true);
    void		InsertSettingsLog(const QString & strItem);
    void		extractDateTime_FetTest(QDateTime &cDateTime,const QString &FileName, const QString &RegExp);
    void		extractDateTime_Eagle(QDateTime &cDateTime,const QString &FileName, const QString &RegExp);
    void        extractDateTime_Spektra(QDateTime &cDateTime,const QString &FileName, const QString &RegExp);
    bool		extractDateTime_FetTest_NoPAT(QDateTime &cDateTime,QString &strDateTime);
    bool		extractDateTime_EagleStdf(QDateTime &cDateTime, QString &FileNameRoot, const QString &FileName);
    bool		CheckForSummaryFiles(QString & strSummaryFile);
    bool		CheckForSummaryFileAvailable(QString & strSummaryFolder, const char *szFilter);
    void		CheckSummaryLogsCreation(QString &strFolder);
    void		CreateSummaryLog(QString & strFolder, QString & strPattern, QString & strRegExp, QString & strLotID, QString & strSplitLotID, TesterType eTesterType);
    void		ReadStringToField(CStdf *pStdfFile,char *szField);
    bool        GetStdfHeaderData(QString &StdfFileName,QString &LotID,QString &SubLotID,QString &ProductID, QDateTime &StartTime);
    bool		getProductName(QString &strPrnFile,QString &strRunName,bool &bGalaxyRetest,bool &bDisablePromis,bool &bEngineeringRecipe,WaferFilesList &WaferList);
    QString     getRecipeFile(const QString &ProductID, const QString &RunName, const bool EngRecipe, const ProdFlow ProdFlow);
    bool        getRecipeFile(const QString &ProductID, const QString &RunName, const bool EngRecipe, const ProdFlow ProdFlow,
                              const GS::Gex::PATRecipeIO::RecipeFormat RecipeFormat, QString &RecipeFile);
    bool        isCompositePatEnabled(const QString & RecipeFile);
    bool		getPROMIS_WafersInSplitLot(const QString &strFolder,const QString &strLotID,CPromisTextFile &cPromis,
                                           WaferFilesList &WaferList,bool bEngPromis,QString & strPromisLine,
                                           QString & strPromisFormat,
                                           const GS::GTrigger::WaferType Type = GS::GTrigger::eUnknownWafer);
    void		CreateTriggers(QString &strPrnFile);
    void		CreateTriggers_FetTest(const QString & SummaryFile);
    void		CreateTriggers_Eagle(const QString & SummaryFile);
    void		MoveDataFiles_Eagle(void);
    void		CreateTriggers_Spektra(const QString & SpektraCntFile);
    bool        GetCntData(const QString & strSpektraCntFile, CInputFile *pclInputFile, QString & strLotID);
    bool        CreateCompositeTrigger(const QString &CompositeRootFolder, const WaferFilesList &WaferList,
                                     const QString &RecipeFileName, const QString &TimeStamp,
                                     QString &CompositeMapFileName, const QString &ProductID);
    bool        AddOptionalSourceToGtf(QTextStream & Trigger, const QString & LotID, const QString & WaferID);

    // FetTest: NO-PAT Flow
    bool        TryGetBinMapItemFromBinMapStore(int aTestNumber, QString &aOutputTestName);
    bool		CheckForFetTestSummary_NoPatFlow(const QString & lPrnFolder);
    bool		getFetTestName(int iTestNumber, QString & strTestName);
    bool		ConvertFetTestNoPat_ToStdf(const QString & lPrnFolder, const QString & lFullFileName);
    bool		LoadBinMapFile(const QString & lFolder);
    bool		ReadBinMapFile_LVM(const QString & lBinMapFile);
    void		ParsingError(QString & strErrorMessage, CInputFile *pclInputFile=NULL, bool bFileRejected=false);
    void		ExecutePostWaferShell(const QString & shell, const QString & waferLogFile);
    /*!
     * \fn CreateTriggerFile
     */
    unsigned int
        CreateTriggerFile(const QString& lSummaryFile,
                          const WaferFiles& lWaferFiles,
                          const QString& lDataFile,
                          enum SummaryInputFormat lSummaryInputFormat,
                          CPromisTextFile& lPromis,
                          const QString& lStdfLot,
                          const QString& lRunName,
                          const QString& lCompositeMapFileName,
                          QString& lRecipeFile,
                          bool lGalaxyRetest,
                          bool lCompositePatEnabled,
                          bool lDisablePromis,
                          bool lEngPromis,
                          QString lPromisFormat);
    /*!
     * \fn CreateJSTriggerFile
     */
    unsigned int
        CreateJSTriggerFile(const QString& lSummaryFile,
                            const WaferFiles& lWaferFiles,
                            const QString& lDataFile,
                            enum SummaryInputFormat lSummaryInputFormat,
                            CPromisTextFile& lPromis,
                            const QString& lStdfLot,
                            const QString& lRunName,
                            const QString& lCompositeMapFileName,
                            QString& lRecipeFile,
                            bool lGalaxyRetest,
                            bool lCompositePatEnabled,
                            bool lDisablePromis,
                            bool lEngPromis,
                            QString lPromisFormat);
    /*!
     * \fn AddOptionalSourceToJs
     */
    bool AddOptionalSourceToJs(QTextStream& Trigger,
                               const QString& LotID,
                               const QString& WaferID);

signals:
    void        sStatusLog(const QString & Line);
    void        sSettingsLog(const QString & Line);

public slots:
    bool        OnStart();

protected slots:
    void		OnResetTimer(bool bLaunchNow=false);
    virtual void OnTimerEvent();
};

} // namespace GS
} // namespace GTrigger

#endif // G_TRIGGER_ENGINE_H
