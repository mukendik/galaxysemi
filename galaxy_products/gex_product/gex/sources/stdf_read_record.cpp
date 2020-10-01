#include "stdf.h"
#include "stdfrecord.h"
#include "stdf_read_record.h"
#include "stdf_head_and_site_number_decipher.h"
#include "gex_constants.h"

void STDFFileRecord::reset()
{
    mHead           = 1;					// HEAD_NUM
    mSite            = 1;					// SITE_NUM

    resetSpecialized();
}

void PRRFileRecord::resetSpecialized()
{
    partId[0] = 0;
    partTxt[0] = 0;
    partFix[0] = 0;
    timeRes = false;


    partFlag        = 0;                // PRR.PART_FLG
    numberOfTests	= 0;                // PRR.NUM_TEST
    hbin            = 0;				// PRR.HARD_BIN
    sbin            = STDF_MAX_U2;		// PRR.SOFT_BIN
    dieX            = INVALID_SMALLINT;	// PRR.X_COORD
    dieY            = INVALID_SMALLINT;	// PRR.Y_COORD
}

void PRRFileRecord::readPRR(GS::StdLib::Stdf &stdfFile)
{
    BYTE lSite;
    stdfFile.ReadByte(&mHead);	// head number
    stdfFile.ReadByte(&lSite);	// site number
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    stdfFile.ReadByte(&partFlag);       // part flag
    stdfFile.ReadWord(&numberOfTests);	// number of tests
    stdfFile.ReadWord(&hbin);           // HBIN
    stdfFile.ReadWord(&sbin);           // SBIN

    if(stdfFile.ReadWord(&dieX) != GS::StdLib::Stdf::NoError)
        dieX = -32768;	// no DIE X

    if(stdfFile.ReadWord(&dieY) != GS::StdLib::Stdf::NoError)
        dieY = -32768;	// no DIE Y

    // Read test time, and get test time info if GOOD binning (bit3,4=0)
    if(stdfFile.ReadDword(&time) == GS::StdLib::Stdf::NoError)
        timeRes = true;

    stdfFile.ReadString(partId);        // PART ID.
    stdfFile.ReadString(partTxt);       // PART TXT.
    stdfFile.ReadString(partFix);       // PART FIX.
}

void PCRFileRecord::resetSpecialized()
{
    mPartCount              = -1;
    mRetestPartCount        = -1;
    mAbortCount             = -1;
    mGoodCount              = -1;
    mFunctCount             = -1;
    mPartCountRes           = false;
    mRetestPartCountRes     = false;
    mAbortCountRes          = false;
    mGoodCountRes           = false;
    mFunctCountRes          = false;
}

void PCRFileRecord::readPCR(GS::StdLib::Stdf &stdfFile)
{
    BYTE lSite;
    stdfFile.ReadByte(&mHead);	// head number
    stdfFile.ReadByte(&lSite);	// site number
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    if(stdfFile.ReadDword(&mPartCount) == GS::StdLib::Stdf::NoError)
        mPartCountRes = true;// PART_CNT

    if(stdfFile.ReadDword(&mRetestPartCount)== GS::StdLib::Stdf::NoError)       // RST_CNT
        mRetestPartCountRes = true;

    if(stdfFile.ReadDword(&mAbortCount)== GS::StdLib::Stdf::NoError)
        mAbortCountRes = true;

    if(stdfFile.ReadDword(&mGoodCount)== GS::StdLib::Stdf::NoError)
        mGoodCountRes = true;

    if(stdfFile.ReadDword(&mFunctCount)== GS::StdLib::Stdf::NoError)
        mFunctCountRes = true;
}

void PTRFileRecord::resetSpecialized()
{
    alarmId[0]  = 0;
    dummy[0]    = 0;

    testDef.clear();
    testUnit.clear();
    testResultNan   = false;
    optFlagRes      = false;
    resScalRes      = false;
    loLimScalRes    = false;
    hiLimScalRes    = false;
    loLimitRes      = false;
    hiLimitRes      = false;
    fmtRes          = false;
    testUnitRes     = false;
    loSpecRes       = false;
    hiSpecRes       = false;

    optFlag         = 0xFF; //GQTL_STDF::Stdf_FTR_V4::eOPT_FLAG_ALL

    testNumber	= 0;		// PTR.TEST_NUM
    testFlag	= 0;		// PTR.TEST_FLG
    paramFlg	= 0;		// PTR.PARM_FLG
    testResult	= 0.0;		// PTR.RESULT

    resScal		= 0;		// PTR.RES_SCAL
    loLimScal	= 0;		// PTR.LLM_SCAL
    hiLimScal	= 0;		// PTR.HLM_SCAL
    loLimit		= 0.0;		// PTR.LO_LIMIT
    hiLimit		= 0.0;		// PTR.HI_LIMIT
    loSpec      = 0.0;		// PTR.LO_SPEC
    hiSpec		= 0.0;		// PTR.HI_SPEC

    mContainTestDefinition = false;
}

void PTRFileRecord::readPTR(GS::StdLib::Stdf &stdfFile)
{
    char lString[257];
    BYTE lSite;

    stdfFile.ReadDword(&testNumber);	// test number
    stdfFile.ReadByte(&mHead);	// head number
    stdfFile.ReadByte(&lSite);	// site number
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

   // if(site == 2)
     //   site = 2;
    stdfFile.ReadByte(&bData);          // test_flg
    testFlag = (int) bData & 0xff;
    stdfFile.ReadByte(&paramFlg);               // parm_flg
    stdfFile.ReadFloat(&testResult, &testResultNan);// test result

    // Read Test definition: name, Limits, etc...
    stdfFile.ReadString(lString);

    if (lString[0])
    {
        // Replaces '\t' by ' '.
        char *ptChar;
        do
        {
            ptChar = strchr(lString,'\t');
            if(ptChar != NULL)
                *ptChar = ' ';
        }
        while(ptChar != NULL);
    }

    testDef.append(lString);

    // alarm_id
    stdfFile.ReadString(alarmId);

    // OPT_FLAG
    if(stdfFile.ReadByte(&optFlag) == GS::StdLib::Stdf::NoError)
    {
        optFlagRes = true;
        // RES_SCALE
        if(stdfFile.ReadByte(&resScal) == GS::StdLib::Stdf::NoError)
        {
            resScalRes = true;
        }
    }

    // llm_scal
    if(stdfFile.ReadByte(&loLimScal) == GS::StdLib::Stdf::NoError)
        loLimScalRes = true;

    // hlm_scal
    if(stdfFile.ReadByte(&hiLimScal) == GS::StdLib::Stdf::NoError)
        hiLimScalRes = true;

    // If result scale is invalid, the 2 limit scales have to be invalid
    if (resScalRes == false)
    {
        loLimScalRes = false;
        hiLimScalRes = false;
    }

    // low limit
    if(stdfFile.ReadFloat(&loLimit) == GS::StdLib::Stdf::NoError)
        loLimitRes = true;

    // low limit
    if(stdfFile.ReadFloat(&hiLimit) == GS::StdLib::Stdf::NoError)
        hiLimitRes = true;

    // testUnit
    if(stdfFile.ReadString(lString) ==  GS::StdLib::Stdf::NoError)
    {
        lString[GEX_UNITS - 1] = 0;
        testUnitRes = true;
        testUnit = lString;
        testUnit = testUnit.trimmed();
        mContainTestDefinition = true;
    }

    // C_RESFMT
    if(stdfFile.ReadString(dummy)  == GS::StdLib::Stdf::NoError)
        fmtRes = true;
    // C_LLMFMT
    if(stdfFile.ReadString(dummy)  == GS::StdLib::Stdf::NoError)
       fmtRes = true;
    // C_HLMFMT
    if(stdfFile.ReadString(dummy)  == GS::StdLib::Stdf::NoError)
        fmtRes = true;

    //LO_SPEC
    if(stdfFile.ReadFloat(&loSpec) == GS::StdLib::Stdf::NoError)
        loSpecRes = true;

    //HI_SPEC
    if(stdfFile.ReadFloat(&hiSpec) == GS::StdLib::Stdf::NoError)
        hiSpecRes = true;

    if (optFlagRes || fmtRes || hiSpecRes || loSpecRes)
    {
        mContainTestDefinition = true;
    }
}

void BinFileRecord::resetSpecialized()
{
    mBinNum         = 0;		// HBR.HBIN_NUM
    mBinCount       = 0;		// HBR.HBIN_CNT
    mPassFailInfo   = 0;		// HBR.HBIN_PF
    mBinLabel[0]	= 0;		// HBR.HBIN_NAM
}

void BinFileRecord::readBin(GS::StdLib::Stdf &stdfFile)
{
    BYTE lSite;
    stdfFile.ReadByte(&mHead);	// head number
    stdfFile.ReadByte(&lSite);	// site number
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    stdfFile.ReadWord(&mBinNum);	// SBIN/HBIN #
    stdfFile.ReadDword(&mBinCount);	// SBIN/HBIN count

    stdfFile.ReadByte(&mPassFailInfo);	// Pass/Fail info
    stdfFile.ReadString(mBinLabel);	// BIN name label
}


void WIRFileRecord::resetSpecialized()
{
    mStart_T    = 0;
    mWaferID[0] = 0;
}

void WIRFileRecord::readWIR(GS::StdLib::Stdf &stdfFile)
{
    BYTE lSite;
    stdfFile.ReadByte(&mHead);	// head number
    stdfFile.ReadByte(&lSite);	// site number
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    stdfFile.ReadDword(&mStart_T);	// START_T

    *mWaferID=0;

    if(stdfFile.ReadString(mWaferID)  == GS::StdLib::Stdf::NoError)
    {
        mWaferID[MIR_STRING_SIZE-1] = 0;
    }
}

void WRRFileRecord::resetSpecialized()
{


    mFinish_T	= 0;            // WRR.FINISH_T
    mPartCount	= 0;            // WRR.PART_CNT
    mRstCount	= INVALID_INT;	// WRR.RTST_CNT
    mAbrCount	= INVALID_INT;	// WRR.ABRT_CNT
    mGoodCount	= INVALID_INT;	// WRR.GOOD_CNT
    mFuncCount	= INVALID_INT;	// WRR.FUNC_CNT
    mWaferID[0]	= 0;           // WRR.WAFER_ID

    mRstCountRes  = true;
    mAbrCountRes  = true;
    mGoodCountRes = true;
    mFuncCountRes = true;
    mPartCountRes = true;
}

void WRRFileRecord::readWRR(GS::StdLib::Stdf &stdfFile)
{
    BYTE lSite;
    stdfFile.ReadByte(&mHead);	// Head
    stdfFile.ReadByte(&lSite);	// test site
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, lSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }


    stdfFile.ReadDword(&mFinish_T);	// FINISH_T

    if(stdfFile.ReadDword(&mPartCount) != GS::StdLib::Stdf::NoError)     // PART_CNT
    {
        mPartCountRes = false;
        return;
    }

    if(stdfFile.ReadDword(&mRstCount) != GS::StdLib::Stdf::NoError) //RST_CNT
    {
        mRstCountRes = false;
        return;
    }

    if(stdfFile.ReadDword(&mAbrCount) != GS::StdLib::Stdf::NoError) //ABR_COUNT
    {
        mAbrCountRes = false;
        return;
    }

    if(stdfFile.ReadDword(&mGoodCount) != GS::StdLib::Stdf::NoError) //GOOD_COUNT
    {
        mGoodCountRes = false;
        return;
    }

    if(stdfFile.ReadDword(&mFuncCount) != GS::StdLib::Stdf::NoError) //FUNC_COUNT
    {
        mFuncCountRes = false;
        return;
    }

    *mWaferID = 0;

    if(stdfFile.ReadString(mWaferID)  != GS::StdLib::Stdf::NoError)
    {
      *mWaferID = 0;
      return;
    }
    // Security: ensures we do not overflow destination buffer !
    mWaferID[MIR_STRING_SIZE-1] = 0;

}


void WCRFileRecord::resetSpecialized()
{

    mWaferSize	= 0.0;					// WCR.WAFR_SIZ
    mDieHeight	= 0.0;					// WCR.DIE_HT
    mDieWidth	= 0.0;					// WCR.DIE_WID
    mUnit       = 0;					// WCR.WF_UNITS
    mFlat		= 0;					// WCR.WF_FLAT
    mCenterX	= INVALID_SMALLINT;		// WCR.CENTER_X
    mCenterY	= INVALID_SMALLINT;		// WCR.CENTER_Y
    mPosX		= 0;					// WCR.POS_X
    mPosY		= 0;					// WCR.POS_Y

    mWaferSizeRes   = true;
    mDieHeightRes   = true;
    mDieWidthRes    = true;
    mUnitRes        = true;
    mFlatRes        = true;
    mCenterXRes     = true;
    mCenterYRes     = true;
    mPosXRes        = true;
    mPosYRes        = true;
}

void WCRFileRecord::readWCR(GS::StdLib::Stdf &stdfFile)
{

    if(stdfFile.ReadFloat(&mWaferSize) != GS::StdLib::Stdf::NoError)
    {
        mWaferSizeRes = false;
        return;	// WaferSize.
    }

    if(stdfFile.ReadFloat(&mDieHeight) != GS::StdLib::Stdf::NoError)
    {
        mDieHeightRes = false;
        return;	// DIE Heigth in WF_UNITS.
    }

    if(stdfFile.ReadFloat(&mDieWidth) != GS::StdLib::Stdf::NoError)
    {
        mDieWidthRes = false;
        return;	// DIE Width in WF_UNITS.
    }

    if(stdfFile.ReadByte(&mUnit) != GS::StdLib::Stdf::NoError)
    {
        mUnitRes = false;
        return;	// WF_UNITS
    }

    if(stdfFile.ReadByte(&mFlat) != GS::StdLib::Stdf::NoError)
    {
        mFlatRes = false;
        return; // WF_FLAT
    }

    if(stdfFile.ReadWord(&mCenterX) != GS::StdLib::Stdf::NoError)
    {
        mCenterXRes = false;
        return; // CENTER_X
    }

    if(stdfFile.ReadWord(&mCenterY) != GS::StdLib::Stdf::NoError)
    {
        mCenterYRes = false;
        return; // CENTER_Y
    }

    if(stdfFile.ReadByte(&mPosX) != GS::StdLib::Stdf::NoError)
    {
        mPosXRes = false;
        return; // POS_X
    }


    if(stdfFile.ReadByte(&mPosY) != GS::StdLib::Stdf::NoError)
    {
        mPosYRes = false;
        return; // POS_Y
    }

}


void TSRFileRecord::resetSpecialized()
{
    mTestType	= 0;				// TSR.TEST_TYP
    mTestNum	= 0;				// TSR.TEST_NUM
    mExecCount	= INVALID_INT;		// TSR.EXEC_CNT
    mFailCount	= INVALID_INT;		// TSR.FAIL_CNT
    mAlarmCount	= INVALID_INT;		// TSR.ALRM_CNT
    mTestName	= "";				// TSR.TEST_NAM
    mSeqName	= "";				// TSR.SEQ_NAME
    mTestLabel	= "";				// TSR.TEST_LBL
    mOptFlag	= 0;				// TSR.OPT_FLAG
    mTestTim	= 0.0;				// TSR.TEST_TIM
    mTestMin	= 0.0;				// TSR.TEST_MIN
    mTestMax	= 0.0;				// TSR.TEST_MAX
    mTestSum	= 0.0;				// TSR.TST_SUMS
    mTestSquare	= 0.0;				// TSR.TST_SQRS
    szTestName[0] = 0;
    mExecCountRes = false;
    mFailCountRes = false;
    mTestMinRes = false;
    mTestMaxRes = false;
    mTestSumRes = false;
    mTestSquareRes = false;
    mTestNameRes = false;
}


void TSRFileRecord::readTSR(GS::StdLib::Stdf &stdfFile)
{
    BYTE lSite;
    stdfFile.ReadByte(&mHead);	// head number
    stdfFile.ReadByte(&lSite);	// site number
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    // TSR.TEST_TYP
    stdfFile.ReadByte(&mTestType);

    // TSR.TEST_NUM
    stdfFile.ReadDword(&mTestNum);

    // TSR.EXEC_CNT
    if(stdfFile.ReadDword(&mExecCount) == GS::StdLib::Stdf::NoError)
        mExecCountRes = true;

    // TSR.FAIL_CNT
    if(stdfFile.ReadDword(&mFailCount)  == GS::StdLib::Stdf::NoError)
        mFailCountRes = true;

    // TSR.ALRM_CNT
    stdfFile.ReadDword(&mAlarmCount);

    char szString[257];
    // TSR.TEST_NAM
    if( stdfFile.ReadString(szString) == GS::StdLib::Stdf::NoError)
    {
       mTestName = QString(szString).trimmed();
       mTestNameRes = true;
    }

    if(mTestName.isEmpty() == false)
    {
        strcpy(szTestName, mTestName.left(256).toLatin1().constData());
        szTestName[256] = 0;
    }
    else
        szTestName[0] = 0;


    // TSR.SEQ_NAME
    if( stdfFile.ReadString(szString) == GS::StdLib::Stdf::NoError)
       mSeqName =  QString(szString);


    // TSR.TEST_LBL
    if( stdfFile.ReadString(szString) == GS::StdLib::Stdf::NoError)
       mTestLabel =  QString(szString);

    // TSR.OPT_FLAG
    stdfFile.ReadByte(&mOptFlag);

    // TSR.TEST_TIM
    stdfFile.ReadFloat(&mTestTim);

    // TSR.TEST_MIN
    if( stdfFile.ReadFloat(&mTestMin) == GS::StdLib::Stdf::NoError)
        mTestMinRes = true;

    // TSR.TEST_MAX
    if( stdfFile.ReadFloat(&mTestMax) == GS::StdLib::Stdf::NoError)
        mTestMaxRes = true;

    // TSR.TST_SUMS
    if( stdfFile.ReadFloat(&mTestSum)  == GS::StdLib::Stdf::NoError)
        mTestSumRes = true;

    // TSR.TST_SQRS
    if( stdfFile.ReadFloat(&mTestSquare)  == GS::StdLib::Stdf::NoError)
        mTestSquareRes = true;
}


void PMRFileRecord::reset()
{

    mIndex	= 0;		// PMR.PMR_INDX
    mChannelType	= 0;		// PMR.CHAN_TYP
    mChannelName[0]	= 0;		// PMR.CHAN_NAM
    mPysicalName[0]	= 0;		// PMR.PHY_NAM
    mLogicalName[0] = 0;		// PMR.LOG_NAM


    mChannelNameRes = true;
    mPysicalNameRes = true;
    mLogicalNameRes = true;
}

void PMRFileRecord::readPMR(GS::StdLib::Stdf &stdfFile)
{
    stdfFile.ReadWord(&mIndex);	// PMR_INDX: Pinmap index

    stdfFile.ReadWord(&mChannelType);	// CHAN_TYP
    if(stdfFile.ReadString(mChannelName)  != GS::StdLib::Stdf::NoError)
    {
        mChannelNameRes = false;
        return;
    }

    if(stdfFile.ReadString(mPysicalName)  != GS::StdLib::Stdf::NoError)
    {
        mPysicalNameRes = false;
        return;	// PHY_NAM
    }

    if(stdfFile.ReadString(mLogicalName)  != GS::StdLib::Stdf::NoError)
    {
        mLogicalNameRes = false;
        return;	// PHY_NAM
    }

    BYTE lSite;

    stdfFile.ReadByte(&mHead);	// Head
    stdfFile.ReadByte(&lSite);	// test site
    mSite = lSite;

    // get the deciphering mode
    if(stdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }
}

void PGRFileRecord::reset()
{
    mGroupIndex     = 0;			// PGR.GRP_INDX
    mIndexCount     = 0;
    mGroupName[0]   = 0;			// PGR.GRP_NAM
    mGroupNameRes	= true;			// PGR.INDX_CNT
}


void PGRFileRecord::readPGR(GS::StdLib::Stdf &stdFile)
{
    reset();
    stdFile.ReadWord(&mGroupIndex);	// GRP_INDX
    if(stdFile.ReadString(mGroupName)  != GS::StdLib::Stdf::NoError)
    {
        mGroupNameRes= false;
        return;	// GTP_NAM
    }

    stdFile.ReadWord(&mIndexCount);	// INDX_CNT

    int iCount = mIndexCount;
    int wData;
    while(iCount)
    {
        stdFile.ReadWord(&wData);	// PMR_INDX: index pin belonging to this group
        iCount--;
    };
}

void MPRFileRecord::reset()
{
    //mTestResultRes = false;
    mIndexCountRes          = true;
    mReturnResultCountRes   = true;
    mTestTxtRes             = true;
    mAlarmIdRes             = true;
    mOptFlagRes             = true;
    mResScalRes             = true;
    mHighLimScalRes         = true;
    mLowLimScalRes          = true;
    mStartInRes             = true;
    mLoLimitRes             = true;
    mHiLimitRes             = true;
    mIncrInRes              = true;
    mUnitsInRes             = true;
    mUnitsRes               = true;
    mLowSpecRes             = true;
    mHighSpecRes            = true;
    mHLMFMTRes              = true;
    mLLMFMTRes              = true;
    mResFMTRes              = true;

    mReturnResults.clear();
    mReturnResultsRes.clear();
    mPMRINdexes.clear();


    mTestNumber	= 0;            // MPR.TEST_NUM
    mHead       = 1;            // MPR.HEAD_NUM
    mSite       = 1;            // MPR.SITE_NUM
    mTestFlag	= 0;            // MPR.TEST_FLG
    mParamFlag	= 0;            // MPR.PARM_FLG
    mReturnResultCount	= 0;    // MPR.RTN_ICNT
    mIndexCount	= 0;		// MPR.RSLT_CNT
    mTestTxt	= "";           // MPR.TEST_TXT
    mAlarmId	= "";           // MPR.ALARM_ID
    mOptFlag	= 0xFF;         // MPR.OPT_FLAG
    mResScal	= 0;            // MPR.RES_SCAL
    mLowLimScal	= 0;            // MPR.LLM_SCAL
    mHighLimScal = 0;		// MPR.HLM_SCAL
    mLoLimit	= 0.0;          // MPR.LO_LIMIT
    mHiLimit	= 0.0;          // MPR.HI_LIMIT
    mStartIn	= 0.0;          // MPR.START_IN
    mIncrIn		= 0.0;          // MPR.INCR_IN
    mUnits		= "";           // MPR.UNITS
    mUnitsIn	= "";           // MPR.UNITS_IN
    mLowSpec    = 0.0;          // MPR.LO_SPEC
    mHighSpec	= 0.0;          // MPR.HI_SPEC


}

void MPRFileRecord::readMPR(GS::StdLib::Stdf &stdFile)
{
    BYTE lSite;

    stdFile.ReadDword(&mTestNumber);	// test number
    stdFile.ReadByte(&mHead);	// Head
    stdFile.ReadByte(&lSite);	// test site
    mSite = lSite;

    // get the deciphering mode
    if(stdFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    stdFile.ReadByte(&mTestFlag);		// test_flg
    stdFile.ReadByte(&mParamFlag);		// parm_flg

    // RTN_ICNT: Count (j) of PMR indexes
    if(stdFile.ReadWord(&mIndexCount) != GS::StdLib::Stdf::NoError)
    {
        mIndexCountRes = false;
        return;
    }

    // RSLT_CNT: Count (k) of returned results
    if(stdFile.ReadWord(&mReturnResultCount) != GS::StdLib::Stdf::NoError)
    {
        mReturnResultCountRes =false;
        return ;
    }

    // RTN_STAT (ignored ?)
    // Read array of states( Nibbles: 8 bits = 2 states of 4 bits!).
    int iCount = mIndexCount;
    BYTE bData;
    while(iCount>0)
    {
        stdFile.ReadByte(&bData);		// State[j]
        iCount-=2;
    }

    // -- RTN_RSLT
    mReturnResults.resize(mReturnResultCount);
    mReturnResultsRes.resize(mReturnResultCount);
    float fData;
    bool res;
    for(int i = 0; i< mReturnResultCount; ++i)
    {
        mReturnResultsRes[i] = false;

        stdFile.ReadFloat(&fData, &res);
        mReturnResults[i] = fData;
        mReturnResultsRes[i] = res;
    }

    //-- TEST_TXT
    char lString[257];

    if(stdFile.ReadString(lString) != GS::StdLib::Stdf::NoError)
    {
        mTestTxtRes = false;
        return;
    }
    mTestTxt = lString;

    //-- AlARM_ID
    if(stdFile.ReadString(lString) != GS::StdLib::Stdf::NoError)
    {
        mAlarmIdRes = false;
        return;
    }
    mAlarmId = lString;

    //--OPT_FLAG
    if(stdFile.ReadByte(&mOptFlag) != GS::StdLib::Stdf::NoError)
    {
        mOptFlagRes = false;
        return;
    }

    // RES_SCAL
    if(stdFile.ReadByte(&mResScal) != GS::StdLib::Stdf::NoError)
    {
        mResScalRes = false;
        return;
    }

    // LLM_SCAL
    if(stdFile.ReadByte(&mLowLimScal)!= GS::StdLib::Stdf::NoError)
    {
        mLowLimScalRes = false;
        return;
    }

    // HLM_SCAL
    if(stdFile.ReadByte(&mHighLimScal)!= GS::StdLib::Stdf::NoError)
    {
        mHighLimScalRes = false;
        return;
    }

    // LO_LIMIT
    if(stdFile.ReadFloat(&mLoLimit)!= GS::StdLib::Stdf::NoError)
    {
        mLoLimitRes = false;
        return;
    }

    // HI_LIMIT
    if(stdFile.ReadFloat(&mHiLimit)!= GS::StdLib::Stdf::NoError)
    {
        mHiLimitRes = false;
        return;
    }

    // START_IN
    if(stdFile.ReadFloat(&mStartIn)!= GS::StdLib::Stdf::NoError)
    {
        mStartInRes = false;
        return;
    }

    // INCR_IN
    if(stdFile.ReadFloat(&mIncrIn)!= GS::StdLib::Stdf::NoError)
    {
        mIncrInRes = false;
        return;
    }

    //--RNT_INDX
    int iData;
    mPMRINdexes.resize(mIndexCount);
    for(int i = 0; i < mIndexCount; ++i)
    {
        stdFile.ReadWord(&iData);
        mPMRINdexes[i] = iData;
    }

    //--UNITS
    if(stdFile.ReadString(lString)!= GS::StdLib::Stdf::NoError)
    {
        mUnitsRes = false;
        return;
    }
    lString[GEX_UNITS-1] = 0;
    mUnits = lString;

    //--UNITS_IN
    if(stdFile.ReadString(lString)!= GS::StdLib::Stdf::NoError)
    {
        mUnitsInRes = false;
        return;
    }
    lString[GEX_UNITS-1] = 0;
    mUnitsIn = lString;

    //Read unused info
    // C_RESFMT
    if(stdFile.ReadString(lString)  != GS::StdLib::Stdf::NoError)
    {
        mResFMTRes = false;
        return;
    }
    // C_LLMFMT
    if(stdFile.ReadString(lString)  != GS::StdLib::Stdf::NoError)
    {
        mLLMFMTRes = false;
        return;
    }
    // C_HLMFMT
    if(stdFile.ReadString(lString)  != GS::StdLib::Stdf::NoError)
    {
        mHLMFMTRes = false;
        return;
    }


    // LO_SPEC
    if(stdFile.ReadFloat(&mLowSpec) != GS::StdLib::Stdf::NoError)
    {
        mLowSpecRes = false;
        return;
    }

    // HI_SPEC
    if(stdFile.ReadFloat(&mHighSpec) != GS::StdLib::Stdf::NoError)
    {
        mHighSpecRes = false;
        return;
    }

}


bool MPRFileRecord::isTestDef()
{
    return (mTestTxtRes ||
            mOptFlagRes ||
            mLowLimScalRes ||
            mHighLimScalRes ||
            mLowSpecRes ||
            mHighSpecRes ||
            mHiLimitRes ||
            mLoLimitRes);

}

void FTRFileRecord::readFTR(GS::StdLib::Stdf &stdFile)
{
    unsigned int	i;

    // FTR.TEST_NUM
    stdFile.ReadDword(&mTestNumber);

    BYTE lSite;
    stdFile.ReadByte(&mHead);	// FTR.HEAD_NUM
    stdFile.ReadByte(&lSite);	// FTR.SITE_NUM
    mSite = lSite;

    // get the deciphering mode
    if(stdFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        mSite = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(mHead, mSite);
        mHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(mHead);
    }

    // FTR.TEST_FLG
    stdFile.ReadByte(&mTestFlag);

    // FTR.OPT_FLAG
    stdFile.ReadByte(&mOptFlag);

    // FTR.CYCL_CNT
    stdFile.ReadDword(&mCycleCount);

    // FTR.REL_VADR
    stdFile.ReadDword(&mRelativeVecAdress);

    // FTR.REPT_CNT
    stdFile.ReadDword(&mRepeatCount);

    // FTR.NUM_FAIL
    stdFile.ReadDword(&mNumberPinFailure);

    // FTR.XFAIL_AD
    stdFile.ReadDword(&mXFailAdress);

    // FTR.YFAIL_AD
    stdFile.ReadDword(&mYFailAdress);

    // FTR.VECT_OFF
    stdFile.ReadWord(&mVectorOffset);

    // FTR.RTN_ICNT
    stdFile.ReadWord(&mReturnCountIndex);

    // FTR.PGM_ICNT
    stdFile.ReadWord(&mPrograStateIndexCount);

    // FTR.RTN_INDX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    if(mReturnCountIndex > 0)
        mPMRIndexes = new unsigned short[mReturnCountIndex];

    int wData;
    for(i=0; i<(unsigned int)mReturnCountIndex; ++i)
    {
        stdFile.ReadWord(&wData);
        mPMRIndexes[i] = wData;
    }

    // FTR.RTN_STAT
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    if(mReturnCountIndex > 0)
        mReturnStates = new char[mReturnCountIndex];

    BYTE bData;
    for(i=0; i<(unsigned int)((mReturnCountIndex+1)/2); ++i)
    {
        stdFile.ReadByte(&bData);
        mReturnStates[i*2] = char(bData & 0x0f);
        if(i*2+1 < mReturnCountIndex)
            mReturnStates[i*2+1] = char((bData >> 4) & 0x0f);
    }

    // FTR.PGM_INDX
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.

    if(mPrograStateIndexCount > 0)
        mPGMIndexes = new unsigned short[mPrograStateIndexCount];
    for(i=0; i<(unsigned int)mPrograStateIndexCount; i++)
    {
        stdFile.ReadWord(&wData);
        mPGMIndexes[i] = wData;
    }

    // FTR.PGM_STAT
    // Set the field as present, even if count is 0. We use a dummy instruction as parameter 1 to avoid warning under gcc.
    if(mPrograStateIndexCount > 0)
        mProgrammedStates = new char[mPrograStateIndexCount];
    for(i=0; i<(unsigned int)((mPrograStateIndexCount+1)/2); i++)
    {
        stdFile.ReadByte(&bData);
        mProgrammedStates[i*2] = char(bData & 0x0f);
        if(i*2+1 < mPrograStateIndexCount)
            mProgrammedStates[i*2+1] = char((bData >> 4) & 0x0f);
    }

    // FTR.FAIL_PIN
    stdFile.ReadDBitField(&(mFailingPin.m_uiLength), mFailingPin.m_pBitField);

    char lString[256];
    // FTR.VECT_NAM
    stdFile.ReadString(lString);
    mVectorName = lString;

    // FTR.TIME_SET
    stdFile.ReadString(lString);
    mTimeSet = lString;

    // FTR.OP_CODE
    stdFile.ReadString(lString);
    mOPCode = lString;

    // FTR.TEST_TXT
    stdFile.ReadString(lString);
    mTestTxt = lString;
    mTestTxt = mTestTxt.trimmed();

    // FTR.ALARM_ID
    stdFile.ReadString(lString);
    mAlarmID = lString;

    // FTR.PROG_TXT
    stdFile.ReadString(lString);
    mProgTxt = lString;

    // FTR.RSLT_TXT
    stdFile.ReadString(lString);
    mResultTxt = lString;

    // FTR.PATG_NUM
    stdFile.ReadByte(&mPatGenNum);

    // FTR.SPIN_MAP
    stdFile.ReadDBitField(&(mPinMap.m_uiLength), mPinMap.m_pBitField);
}

void FTRFileRecord::reset()
{

    mFailingPin.Clear();
    mPinMap.Clear();

    mVectorName = "";
    mTimeSet = "";
    mOPCode = "";
    mTestTxt = "";
    mAlarmID = "";
    mProgTxt = "";
    mResultTxt = "";

    mTestNumber         = 0;		// FTR.TEST_NUM
    mHead               = 1;		// FTR.HEAD_NUM
    mSite               = 1;		// FTR.SITE_NUM
    mTestFlag           = 0;		// FTR.TEST_FLG
    mOptFlag            = 0;		// FTR.OPT_FLAG
    mCycleCount         = 0;		// FTR.CYCL_CNT
    mRelativeVecAdress	= 0;		// FTR.REL_VADR
    mRepeatCount        = 0;		// FTR.REPT_CNT
    mNumberPinFailure	= 0;		// FTR.NUM_FAIL
    mXFailAdress        = 0;		// FTR.XFAIL_AD
    mYFailAdress        = 0;		// FTR.YFAIL_AD
    mVectorOffset       = 0;		// FTR.VECT_OFF
    mReturnCountIndex	= 0;		// FTR.RTN_ICNT
    mPrograStateIndexCount	= 0;		// FTR.PGM_ICNT
    mOptFlag            = 0;
    mPatGenNum          = 0;


    mPMRIndexes = 0;
    mPGMIndexes = 0;
    mReturnStates = 0;
    mProgrammedStates = 0;
}

int	SDRFileRecord::readStringToField(GS::StdLib::Stdf &stdfile, char *szField)
{
  char	szString[257];	// A STDF string is 256 bytes long max!

  *szField=0;

  if(stdfile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
    return -1;
  // Security: ensures we do not overflow destination buffer !
  szString[MIR_STRING_SIZE-1] = 0;
  strcpy(szField,szString);
  return 1;
}



void SDRFileRecord::reset()
{
    mSiteNumbers.resize(0);
    mSiteDescription.m_strHandlerType.clear();
    mSiteDescription.m_strHandlerProberID.clear();
    mSiteDescription.m_strProbeCardType.clear();
    mSiteDescription.m_strProbeCardID.clear();
    mSiteDescription.m_strLoadBoardType.clear();
    mSiteDescription.m_strLoadBoardID.clear();
    mSiteDescription.m_strDibBoardID.clear();
    mSiteDescription.m_strInterfaceCableID.clear();
    mSiteDescription.m_strHandlerContactorID.clear();
    mSiteDescription.m_strLaserID.clear();
    mSiteDescription.m_strExtraEquipmentID  .clear();
}

void SDRFileRecord::readSDR(GS::StdLib::Stdf &stdfile)
{
    // Read SDR SDTF V4
    stdfile.ReadByte(&mHead);		// Head nb
    stdfile.ReadByte(&mSiteGroup);		// Site group nb
    stdfile.ReadByte(&mSiteCount);		// Site count
    if(mSiteCount != 0)
    {
        // Keep track of site numbers
        mSiteNumbers.resize(mSiteCount);
        BYTE bData;
        for(int nSite=0; nSite<mSiteCount; ++nSite)
        {
            stdfile.ReadByte(&bData);			// Site#
            mSiteNumbers[nSite] = (int)bData;
        }
    }

    char                  szString[256];
    if(readStringToField(stdfile, szString) != 1)		// Handler type
        return;
    mSiteDescription.m_strHandlerType = szString;

    // If reach this point then we have at least one SDR valid record, even if nSiteCount states otherwise!
    if(mSiteCount == 0)
    {
        // Overload site count
        mSiteCount = 1;

        // Overload site map I
        // Fill buffer
        mSiteNumbers.resize(mSiteCount);
        mSiteNumbers[0] = 1;
    }

    if(readStringToField(stdfile, szString) != 1)		// Handler ID
        return;
    mSiteDescription.m_strHandlerProberID = szString;
    if(readStringToField(stdfile, szString) != 1)		// Prober card type
        return;
    mSiteDescription.m_strProbeCardType = szString;
    if(readStringToField(stdfile, szString) != 1)		// Prober card ID
        return;
    mSiteDescription.m_strProbeCardID = szString;
    if(readStringToField(stdfile, szString) != 1)		// Loadboard type
        return;
    mSiteDescription.m_strLoadBoardType = szString;
    if(readStringToField(stdfile, szString) != 1)		// Loadboard ID
        return;
    mSiteDescription.m_strLoadBoardID = szString;
    if(readStringToField(stdfile, szString) != 1)		// DIBboard type
        return;
    if(readStringToField(stdfile, szString) != 1)		// DIBboard ID
        return;
    mSiteDescription.m_strDibBoardID = szString;
    if(readStringToField(stdfile, szString) != 1)		// Interface cable type
        return;
    if(readStringToField(stdfile, szString) != 1)		// Interface cable ID
        return;
    mSiteDescription.m_strInterfaceCableID = szString;
    if(readStringToField(stdfile, szString) != 1)		// Handler contactor type
        return;
    if(readStringToField(stdfile, szString) != 1)		// Handler contactor ID
        return;
    mSiteDescription.m_strHandlerContactorID = szString;
    if(readStringToField(stdfile, szString) != 1)		// Laser type
        return;
    if(readStringToField(stdfile, szString) != 1)		// Laser ID
        return;
    mSiteDescription.m_strLaserID = szString;
    if(readStringToField(stdfile, szString) != 1)		// Extra equipment type
        return;
    if(readStringToField(stdfile, szString) != 1)		// Extra equipment ID
        return;
    mSiteDescription.m_strExtraEquipmentID = szString;


}

//bool DTRFileRecord::extractLimit()
//{
//    QString lCommand = "ML"; // to search Multi-Limit object
//    QJsonObject lJson = mDTR_V4.GetGsJson(lCommand);

//    if(!lJson.isEmpty()) // if multi limit JSon oject found!
//    {
//        QString lError;
//        mLimit = new GS::Core::MultiLimitItem();
//        if (!mLimit->LoadFromJSon(lJson, lError) || !mLimit->IsValid())
//        {
//            GSLOG(SYSLOG_SEV_ERROR, QString("Error when reading DTR record: %1 - %2")
//                  .arg(lDTR_V4.m_cnTEXT_DAT)
//                  .arg(lError)
//                  .toLatin1().constData());
//            return false;
//        }
//        if (!mLimit->IsValidLowLimit() && !mLimit->IsValidHighLimit())
//        {
//            GSLOG(SYSLOG_SEV_NOTICE, QString("No Valid limit in DTR: %1")
//                  .arg(lDTR_V4.m_cnTEXT_DAT)
//                  .toLatin1().constData());
//            return false;
//        }

//        mTestNumber = GS::Core::MultiLimitItem::ExtractTestNumber(lJson);
//        mTestName = GS::Core::MultiLimitItem::ExtractTestName(lJson);
//        mSite = GS::Core::MultiLimitItem::ExtractSiteNumber(lJson);
//    }
//    return true;

//}

void DTRFileRecord::readDTR(GS::StdLib::Stdf &stdfile)
{
   // bool lIsDatalogString=true, lIsGexScriptingCommand=false, lIsFilteredPart;
    //CTest	*lTestCell = NULL;	// Pointer to test cell to receive STDF info.


    // ** check if contains a multi limits record
    // First read TSR completely
    mDTR_V4.Read(stdfile);

  //  QString lCommand = "ML"; // to search Multi-Limit object
   // mJson = mDTR_V4.GetGsJson(lCommand);

//    // Change \n to \r so HTML output (if report type) doesn't include empty lines.
//    mString = mDTR_V4.m_cnTEXT_DAT;
//    mString.replace('\n','\r');

//    //************************************************
//    // Check if scripting line
//    // Format: <cmd> set variable = value
//    // Ember should modify their test programs to use fommowing command instead:
//     mIsGexScriptingCommand = (lString.toLower().startsWith("<cmd>", Qt::CaseInsensitive));
//    if(!mIsGexScriptingCommand)
//        mIsGexScriptingCommand = (lString.toLower().indexOf("<cmd> gross_die") >= 0);
//    if(!mIsGexScriptingCommand)
//        mIsGexScriptingCommand = (lString.toLower().indexOf("<cmd> grossdiecount") >= 0);




//    if(!lIsGexScriptingCommand)
//    {
//        // GCORE-2133
//        if (lString.startsWith("/*GalaxySemiconductor*/") || lString.startsWith("/*GalaxySemiconductorJS*/") )
//        {
//            QVariant lConcatenatedJSDTR = property(sConcatenatedJavaScriptDTRs.toLatin1().data());
//            if (lConcatenatedJSDTR.isNull())
//            {
//                setProperty(sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant(lString));
//            }
//            else
//            {
//                setProperty(sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant(lConcatenatedJSDTR.toString()+lString) );
//            }
//        }
//        return true;
//    }

//    // DTR Gex scripting commands
//    long	lTestNumber=0;
//    double	lfResult;

//    // Parse command line (take string after the '<cmd>' prefix)
//    QString lAction = lString.mid(6);
//    //strAction = strAction.trimmed();
//    QString lKeyword = lAction.section(' ',0,0);
//    lKeyword = lKeyword.trimmed();

//    // Check if PAT datalog string
//    if(lKeyword.startsWith("logPAT", Qt::CaseInsensitive))
//    {
//        // Command line: '<cmd> logPAT <PAT datalog HTML string>'
//        if(lPass == 1 && ReportOptions.getAdvancedReport() == GEX_ADV_PAT_TRACEABILITY)
//        {
//            // Only consider this string during pass1. Ignore'd during pass 2!
//            lAction = lAction.mid(7);
//            gexReport->strPatTraceabilityReport += lAction;
//        }
//        return true;
//    }

//    int  lCommandIndex;

//    // Check if gross_die command
//    if (ReportOptions.
//        GetOption("binning",
//                  "total_parts_used_for_percentage_computation").toString() ==
//        "gross_die_if_available")
//    {
//        bool bOK;
//        int  nGrossDie;
//        if (lKeyword.toLower().startsWith("gross_die", Qt::CaseInsensitive))
//        {
//            bool bOK;
//            int  nGrossDie = lAction.section('=', 1, 1).trimmed().toInt(&bOK);
//            if (bOK)
//            {
//                m_nGrossDieCount = nGrossDie;
//                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
//                return true;
//            }
//        }

//        // Note: Vishay Eagle files have [0,15,"<cmd> GrossDieCount =8290",""]
//        // instead of [<cmd> gross_die=8290]
//        lCommand    = lString.trimmed().toLower();
//        lCommandIndex = lCommand.indexOf("<cmd> gross_die");
//        if (lCommandIndex >= 0)
//        {
//            // Parse command line
//            lCommand = lCommand.mid(lCommandIndex);
//            nGrossDie  =
//                    lCommand.section(QRegExp("[=\"]"), 1, 1).trimmed().toInt(&bOK);
//            if (bOK)
//            {
//                m_nGrossDieCount = nGrossDie;
//                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
//                return true;
//            }
//        }
//        lCommandIndex = lCommand.indexOf("<cmd> grossdiecount");
//        if (lCommandIndex >= 0)
//        {
//            // Parse command line
//            lCommand = lCommand.mid(lCommandIndex);
//            nGrossDie  =
//                    lCommand.section(QRegExp("[=\"]"), 1, 1).trimmed().toInt(
//                        &bOK);
//            if (bOK)
//            {
//                m_nGrossDieCount = nGrossDie;
//                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
//                return true;
//            }
//        }
//    }

//    // Ignore multi-die command
//    lCommandIndex = lCommand.indexOf("<cmd> multi-die");
//    if(lCommandIndex >= 0)
//        return true;

//    // Ignore die-tracking command
//    lCommandIndex = lCommand.indexOf("<cmd> die-tracking");
//    if(lCommandIndex >= 0)
//        return true;

//    // Extract <VariableName> & <TestNumber>
//    lAction = lAction.section(' ',1);
//    // E.g: string format: 'setValue <VariableName>.<TestNumber> = <value>'
//    QString lParameter = lAction.section('.',0,0);
//    QString lStrTestNumber = lAction.section('.',1);
//    lStrTestNumber = lStrTestNumber.section('=',0,0);
//    lStrTestNumber = lStrTestNumber.trimmed();
//    bool	lIsValid;
//    lTestNumber = lStrTestNumber.toLong(&lIsValid);
//    if(!lIsValid)
//    {
//        GSLOG(SYSLOG_SEV_NOTICE,
//              QString("Failed to extract test number...string parsed may be corrupted")
//              .toLatin1().constData());
//        return true;
//    }

//    // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
//    if(FindTestCell(lTestNumber,GEX_PTEST, &lTestCell,true,true, lParameter.toLatin1().data()) !=1)
//    {
//        GSLOG(SYSLOG_SEV_ERROR,
//              QString("Error when searching test: %1 - %2")
//              .arg(QString::number(lTestNumber))
//              .arg(lParameter)
//              .toLatin1().constData());
//        return true;	// Error
//    }

//    // Set test type
//    GEX_ASSERT(lTestCell->bTestType == 'P');

//    if(lKeyword.startsWith("setString", Qt::CaseInsensitive))
//    {
//        // String variable
//    }
//    if(lKeyword.startsWith("setValue", Qt::CaseInsensitive))
//    {
//        // Parametric variable
//        QString lValue = lAction.section('=',1,1);
//        lValue = lValue.trimmed();
//        lfResult = lValue.toDouble(&lIsValid);
//        if(!lIsValid)
//        {
//            GSLOG(SYSLOG_SEV_NOTICE,
//                  QString("Failed to extract test result...string parsed may be corrupted")
//                  .toLatin1().constData());
//            return true;
//        }

//        // During pass#1: keep track of Min/Max so histogram cells can be computed in pass#2.
//        if(lPass == 1)
//            UpdateMinMaxValues(lTestCell,lfResult);

//        if(lPass == 2)
//        {
//            // Command must be after a PIR and before a PRR
//            GEX_ASSERT(PartProcessed.PIRNestedLevel() != 0);

//            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
//            if(pReportOptions->bSpeedCollectSamples)
//            {
//                if (AdvCollectTestResult(lTestCell, lfResult, CTestResult::statusUndefined, 0, true) == false)
//                {
//                    GSLOG(SYSLOG_SEV_ERROR, "Failed to collect test result");
//                    return false;
//                }
//            }

//#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
//            // Keeps tracking the test# result.
//            ptTestCell->ldCurrentSample++;
//#endif

//            // Build Cpk
//            lTestCell->lfSamplesTotal += lfResult;				// Sum of X
//            lTestCell->lfSamplesTotalSquare += (lfResult*lfResult);// Sum of X*X


//            lTestCell->mHistogramData.AddValue(lfResult);
//            if((lfResult >= lTestCell->GetCurrentLimitItem()->lfHistogramMin) && (lfResult <= lTestCell->GetCurrentLimitItem()->lfHistogramMax))
//            {
//                // Incluse result if it in into the viewing window.
//                int lCell;
//                if (lTestCell->GetCurrentLimitItem()->lfHistogramMax == lTestCell->GetCurrentLimitItem()->lfHistogramMin)
//                {
//                    lCell = 0;
//                }
//                else
//                {
//                    lCell = (int)( (TEST_HISTOSIZE*(lfResult - lTestCell->GetCurrentLimitItem()->lfHistogramMin)) / (lTestCell->GetCurrentLimitItem()->lfHistogramMax-lTestCell->GetCurrentLimitItem()->lfHistogramMin));
//                }

//                // Incluse result if it in into the viewing window.
//                if( (lCell >= 0) && (lCell < TEST_HISTOSIZE) )
//                    lTestCell->lHistogram[lCell]++;
//                else if (lCell >= TEST_HISTOSIZE)
//                    lTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
//                else
//                {
//                    GSLOG(SYSLOG_SEV_ERROR,
//                             QString("Invalid value detected to build histogram for test %1 [%2]")
//                             .arg(lTestCell->lTestNumber)
//                             .arg(lTestCell->strTestName).toLatin1().constData());

//                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(lCell).toLatin1().constData());
//                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tDTR result: %1").arg( lfResult).toLatin1().constData());
//                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1")
//                          .arg(lTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
//                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
//                             lTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
//                }
//            }
//        }
//    }
//    else
//    {
//        GSLOG(SYSLOG_SEV_NOTICE,
//              QString("Unsupported command line: %1")
//              .arg(lKeyword)
//              .toLatin1().constData());
//        return true;
//    }

//    return true;
}
