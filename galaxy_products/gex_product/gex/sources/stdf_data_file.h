#ifndef STDFDATAFILE_H
#define STDFDATAFILE_H


#include "gex_file_in_group.h"
#include "cmir.h"
#include "stdf.h"
#include "stdf_read_record.h"

#include <QMap>
#include <QString>


class StdfDataFile
{

public:
    StdfDataFile(QString & fileName,
                 FILE*  advFile,
                 CReportOptions* globalReportOptions,
                 const QString & waferToExtract);
    ~StdfDataFile();

    /**
     * @brief insertFileInGroup add a CGexFileInGroup* into the mFileInGroups container
     * @param site will be used as a key insertion
     * @param file will be used as a value inserted
     */
    void insertFileInGroup(int site, CGexFileInGroup* file);

    /**
     * @brief loadFile open the file define by mFileName
     * @return true if the open succeed, false if not
     */
    bool loadFile();

    /**
     * @brief processFile, will process a physical file (reading) and initialized the memory to generate the report
     * @param iPass
     * @return
     */
    bool processFile(int iPass);


    void closeFile() { mStdfFile.Close();}



    CMir&           getMirDatas()  { return mMirData;}

    void initPass();
    CReportOptions*                 mReportOptions;
    int                             mMRRCount;
    FILE*                           mAdvFile;

    CPinmap*                        getPinmap() { return ptPinmapList; }
    void                            setPinmap(CPinmap* pinMap) { ptPinmapList = pinMap; }

    const QString&                         getFileName() { return mFileName;}
private:

    enum T_Record {T_PIR, T_PCR, T_PRR, T_PTR, T_WIR, T_WRR, T_WCR, T_PMR, T_TSR, T_MPR, T_FTR};
    bool                            mTestDef;
    int                             mPTRCount;

    /**
      * @brief mFileInGroups ordered by site. Can have two entries for the same site in the case of
      * different filtering
      */
    typedef QMap<int, QList<CGexFileInGroup*> >     FileInGroupContainer;
    typedef FileInGroupContainer::iterator          FileInGroupContainerIter;

    FileInGroupContainer            mFileInGroupsBySite;
    QList<PTRFileRecord*>           mPTRRecords;
    PRRFileRecord                   mPRRRecord;
    PTRFileRecord                   mPTRRecord;
    PCRFileRecord                   mPCRRecord;
    BinFileRecord                   mBinRecord;
    WIRFileRecord                   mWIRRecord;
    WRRFileRecord                   mWRRRecord;
    WCRFileRecord                   mWCRRecord;
    TSRFileRecord                   mTSRRecord;
    PMRFileRecord                   mPMRRecord;
    PGRFileRecord                   mPGRRecord;
    MPRFileRecord                   mMPRRecord;
    FTRFileRecord                   mFTRRecord;
    SDRFileRecord                   mSDRRecord;
    DTRFileRecord                   mDTRRecord;
    QString                         mFileName;
    QString                         mWaferToExtract;
    GS::StdLib::Stdf                mStdfFile;
    GS::StdLib::StdfRecordReadInfo  mRecordHeader;
    CMir                            mMirData;
    long                            mRecordIndex;
    bool                            mIgnoreNextWaferRecord;
    CPinmap*                        ptPinmapList;

    /**
     * @brief mCurrentIndexWaferMap is tue current index of the wafermap that is currently read inside a file
     * Used mainly when there are several wafermpa inside a same file
     */
    int                             mCurrentIndexWaferMap;

    /**
     * @brief mMultiWaferMap indicate that there is several wafer description
     */
    bool                            mMultiWaferMap;

    bool readStringToField  (char *szField);

    //-- ATR
     void processATR        (int lPass);

    //--MIR
    void readMIR            (int pass);

    //--MRR
    void readMRR            (int lPass);


    /**
     * @brief processXXX will parse a record XXX and put on the XXXFileRecord and initialize all the associated fileInGroup
     * @param lPass is the pass number that will determine which step has to be done or not
     */
    /**
     * @brief initFileInGroupXXX initialize the XXX data of a CGexFileInGroup with the data in the XXXFileRecord
     * @param lFile, the pFileInGroup instance that will be initialized
     */

    //-- WIR
    void processWIR(int lPass);
    void initFileInGroupWIR(CGexFileInGroup *lFile);

    //-- WRR
    void processWRR(int lPass);
    void initFileInGroupWRR(CGexFileInGroup *lFile, int lPass);

    //--WCR
    void processWCR(int lPass);
    void initFileInGroupWCR(CGexFileInGroup *lFile);

    //--TSR
    void processTSR(int lPass);
    void initFileInGroupTSR(CGexFileInGroup *lFile, int lPass);

    //--PMR
    void processPMR(int lPass);
    void initFileInGroupPMR(CGexFileInGroup *lFile);

    //--MPR
    void processMPR(int lPass);
    void initFileInGroupMPR(CGexFileInGroup *pFile, int lPass);

    bool readStaticDataMPR(CGexFileInGroup *pFile,
                            CTest ** ptParamTestCell,
                              unsigned long lTestNumber,
                              long iPinmapMergeIndex,
                              long nJcount,
                              long nKcount,
                              int *piCustomScaleFactor,
                              bool bStrictLL, bool bStrictHL, int lPass);

    //--PGR
    void processPGR(int lPass);
    //-- PLR
    void processPLR(int lPass);
    //--GDR
    void processGDR();
    //--SDR
    void processSDR(int lPass);

    //--FTR
    void processFTR(int lPass);
    void initFileInGroupFTR(CGexFileInGroup *pFile, int lPass);

    //--DTR
    void processDTR(int lPass);
    void initFileInGroupDTR(CGexFileInGroup *pFile, int lPass);

    //--PCR
    bool processPCR(int lPass);
    void initFileInGroupPCR(CGexFileInGroup *lFile);

    //-- PIR
    void processPIR             ();
    void initFileInGroupPIR     (CGexFileInGroup *lFile, CPartInfo &PartInfo);

    //--PRR
    void processPRR             (int lPass);
    void initFileInGroupPRR     (CGexFileInGroup *lFile, int lPass);

    //-- PTR
    bool processPTR             (int lPass, bool itsATestDefinition);
    bool initFileInGroupPTR (CGexFileInGroup *lFile, int lPass);

    void processBinning(int lPass, bool isHBR);
    void initFileInGroupBin(CGexFileInGroup *pFile,
                                          int lPass, CBinning **ptBinList,
                                          int *piRecords,
                                          bool *pbMerge);




    bool isMatchingWafer    (const QString& strFilter, const QString& strWaferID);

    /**
     * @brief readFile will read the file and decode each record
     * @param iPass is the number of the pass. According to the value some process will be done or not
     * @return
     */
    bool readFile           (int iPass);

    void initFileInGroup(CGexFileInGroup *lFile, T_Record type, int lPass = -1, CPartInfo *partInfo = 0);

    /**
     * @brief loopOverFileInGroup enables to loop throughout the list of pFileInGroup associated with this file and to call
     * the related initFileInGroupXXX
     * According to the argument, either all pFileInGroup instance will be used or only the one withe the same site number
     * @param siteNumber is the site decoded in the record. If eqaul to -1, this means that all pFile have to be used,
     * otherwise a comparaison is done between the siteNumber and the processSite of the pFile
     * @param type, the type of the record
     * @param lPass
     * @param partInfo
     */
    void loopOverFileInGroup(int siteNumber, T_Record type, int lPass = -1,  CPartInfo *partInfo = 0);
};
#endif // STDFDATAFILE_H
