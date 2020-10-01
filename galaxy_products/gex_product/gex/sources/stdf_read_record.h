#ifndef STDF_READ_RECORD_H
#define STDF_READ_RECORD_H

#include "plugin_base.h"
#include <QString>
#include <QVector>
#include <QJsonObject>
#include "multi_limit_item.h"
#include "stdfrecords_v4.h"
#include "stdfrecord.h"


class STDFFileRecord
{
public:
    STDFFileRecord(){}
    virtual ~STDFFileRecord(){}

    void reset();

    BYTE mHead;
    int  mSite;

private:
    virtual void resetSpecialized() = 0;

};

class PRRFileRecord: public STDFFileRecord
{
    public:
        PRRFileRecord():STDFFileRecord() { reset();}

        char partId[1024];
        char partTxt[1024];
        char partFix[1024];
        long time;
        int sbin;
        int hbin;
        int numberOfTests;
        int dieX;
        int dieY;
        bool timeRes;

        BYTE partFlag;

        void readPRR(GS::StdLib::Stdf &stdfFile);

private:
    void resetSpecialized();
};

class PCRFileRecord: public STDFFileRecord
{
    public:
        PCRFileRecord():STDFFileRecord() { reset();}

        long mAbortCount;
        long mPartCount;
        long mRetestPartCount;
        long mGoodCount;
        long mFunctCount;
        bool mPartCountRes;
        bool mRetestPartCountRes;
        bool mAbortCountRes;
        bool mGoodCountRes;
        bool mFunctCountRes;

        BYTE partFlag;

        void readPCR(GS::StdLib::Stdf &stdfFile);

    private:
        void resetSpecialized();
};

class PTRFileRecord: public STDFFileRecord
{
public:
    PTRFileRecord():STDFFileRecord() { reset();}

    void readPTR(GS::StdLib::Stdf &stdfFile);

    QString testDef;
    QString testUnit;
    char alarmId[257];
    char dummy[257];
    long testNumber;
    int  testFlag;
    float testResult;
    float loLimit;
    float hiLimit;
    float hiSpec;
    float loSpec;
    bool testResultNan;
    bool optFlagRes;
    bool resScalRes;
    bool loLimScalRes;
    bool hiLimScalRes;
    bool loLimitRes;
    bool hiLimitRes;
    bool testUnitRes;
    bool fmtRes;
    bool loSpecRes;
    bool hiSpecRes;
    BYTE bData;
    BYTE paramFlg;
    BYTE optFlag;
    BYTE resScal;
    BYTE loLimScal;
    BYTE hiLimScal;
    bool mContainTestDefinition;

private:
    void resetSpecialized();
};

class BinFileRecord: public STDFFileRecord
{
public:
    BinFileRecord():STDFFileRecord() { reset();}

    char	mBinLabel[257];
    long    mBinCount;
    int     mBinNum;
    BYTE    mPassFailInfo;

    void readBin(GS::StdLib::Stdf &stdfFile);

private:
    void resetSpecialized();
};


class WIRFileRecord: public STDFFileRecord
{
public:
    WIRFileRecord():STDFFileRecord() { reset();}

   long     mStart_T;
   char     mWaferID[257];


    void readWIR(GS::StdLib::Stdf &stdfFile);

private:
    void resetSpecialized();
};

class WRRFileRecord: public STDFFileRecord
{
public:
    WRRFileRecord():STDFFileRecord() { reset(); }


    char mWaferID[257];
    long mFinish_T;	// FINISH_T
    long mPartCount;
    long mRstCount;
    long mAbrCount;
    long mGoodCount;
    long mFuncCount;

    bool mRstCountRes;
    bool mAbrCountRes;
    bool mGoodCountRes;
    bool mFuncCountRes;
    bool mPartCountRes;


    void readWRR(GS::StdLib::Stdf &stdfFile);

private:
   void resetSpecialized();

};

class WCRFileRecord: public STDFFileRecord
{
public:
    WCRFileRecord():STDFFileRecord() {}


    int mCenterX;
    int mCenterY;
    float mWaferSize;
    float mDieHeight;
    float mDieWidth;
    BYTE mUnit;
    BYTE mFlat;
    BYTE mPosX;
    BYTE mPosY;

    bool mWaferSizeRes;
    bool mDieHeightRes;
    bool mDieWidthRes;
    bool mUnitRes;
    bool mFlatRes;
    bool mCenterXRes;
    bool mCenterYRes;
    bool mPosXRes;
    bool mPosYRes;

    void readWCR(GS::StdLib::Stdf &stdfFile);

private:
    void resetSpecialized();
};

class TSRFileRecord: public STDFFileRecord
{
public:
    TSRFileRecord():STDFFileRecord() {}

    char szTestName[257];
    QString mTestName;
    QString mSeqName;
    QString mTestLabel;
    long  mTestNum;
    long  mExecCount;
    long  mFailCount;
    long  mAlarmCount;
    float mTestTim;
    float mTestMin;
    float mTestMax;
    float mTestSum;
    float mTestSquare;
    bool mTestNameRes;
    bool mExecCountRes;
    bool mFailCountRes;
    bool mTestMinRes;
    bool mTestMaxRes;
    bool mTestSumRes;
    bool mTestSquareRes;
    BYTE mTestType;
    BYTE mOptFlag;


    void readTSR(GS::StdLib::Stdf &stdfFile);

private:
        void resetSpecialized();
};


class PMRFileRecord
{
public:
    PMRFileRecord() { reset(); }

    char mChannelName[257];
    char mPysicalName[257];
    char mLogicalName[257];
    int mIndex;
    int mChannelType;
    bool mChannelNameRes;
    bool mPysicalNameRes;
    bool mLogicalNameRes;
    BYTE mHead;
    int  mSite;

    void reset();
    void readPMR(GS::StdLib::Stdf &stdFile);

};

class PGRFileRecord
{
public:
    PGRFileRecord(): mGroupNameRes(true) {}

    char mGroupName[257];
    int mGroupIndex;
    int mIndexCount;
    bool mGroupNameRes;


    void reset();

    void readPGR(GS::StdLib::Stdf &stdFile);
};

class MPRFileRecord
{
public:
    MPRFileRecord(){ reset();}


    QVector<float>          mReturnResults;
    QVector<bool>           mReturnResultsRes;
    QVector<stdf_type_u2>   mPMRINdexes;

    QString         mTestTxt;
    QString         mAlarmId;
    QString         mUnitsIn;
    QString         mUnits;
    long            mTestNumber;
    int             mIndexCount;
    int             mReturnResultCount;
    float           mLoLimit;
    float           mHiLimit;
    float           mStartIn;
    float           mIncrIn;
    float           mHighSpec;
    float           mLowSpec;
    BYTE            mHighLimScal;
    BYTE            mLowLimScal;

    bool    mIndexCountRes;
    bool    mReturnResultCountRes;
    bool    mTestTxtRes;
    bool    mAlarmIdRes;
    bool    mOptFlagRes;
    bool    mResScalRes;
    bool    mHighLimScalRes;
    bool    mLowLimScalRes;
    bool    mLoLimitRes;
    bool    mHiLimitRes;
    bool    mStartInRes;
    bool    mIncrInRes;
    bool    mUnitsInRes;
    bool    mUnitsRes;
    bool    mHLMFMTRes;
    bool    mLLMFMTRes;
    bool    mResFMTRes;
    bool    mHighSpecRes;
    bool    mLowSpecRes;

    BYTE mHead;
    int  mSite;
    BYTE mTestFlag;
    BYTE mParamFlag;
    BYTE mOptFlag;
    BYTE mResScal;

    void reset();
    void readMPR(GS::StdLib::Stdf &stdFile);
    bool isTestDef();
};

class FTRFileRecord
{
public:
    FTRFileRecord() { reset(); }
    ~FTRFileRecord() { delete mPMRIndexes; delete mReturnStates;}


     GQTL_STDF::stdf_type_dn mFailingPin;
     GQTL_STDF::stdf_type_dn mPinMap;
     QString  mVectorName;
     QString  mTimeSet;
     QString  mOPCode;
     QString  mTestTxt;
     QString  mAlarmID;
     QString  mProgTxt;
     QString  mResultTxt;
     long  mTestNumber;
     long  mCycleCount;
     long  mRelativeVecAdress;
     long  mRepeatCount;
     long  mNumberPinFailure;
     long  mXFailAdress;
     long  mYFailAdress;
     int   mVectorOffset;
     int   mReturnCountIndex;
     int   mPrograStateIndexCount;

     unsigned short* mPMRIndexes;
     unsigned short* mPGMIndexes;
     char *mReturnStates;
     char *mProgrammedStates;
     BYTE mHead;
     int  mSite;
     BYTE mTestFlag;
     BYTE mOptFlag;
     BYTE mPatGenNum;

     void readFTR(GS::StdLib::Stdf &stdFile);
     void reset();

};


class SDRFileRecord
{
public:
    SDRFileRecord(){}

    void readSDR(GS::StdLib::Stdf &stdfile);
    void reset();

    GP_SiteDescription	  mSiteDescription;
    QVector<int>          mSiteNumbers;
    BYTE                  mHead;
    BYTE                  mSiteGroup;
    BYTE                  mSiteCount;

private:
    int readStringToField(GS::StdLib::Stdf &stdfile, char *szField);
};


class DTRFileRecord
{

public:
    DTRFileRecord(){ reset();}

    GQTL_STDF::Stdf_DTR_V4 mDTR_V4;
    //QJsonObject mJson;

   /* GS::Core::MultiLimitItem* mLimit;
    int mTestNumber;
    QString mTestName;*/

    void reset() {
        mDTR_V4.Reset();
                 }
    void readDTR(GS::StdLib::Stdf &stdfile);
};

#endif // STDF_READ_RECORD_H
