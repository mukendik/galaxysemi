#ifndef GEX_IMPORT_STDF_V3_H
#define GEX_IMPORT_STDF_V3_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <qlist.h>

#include "gex_constants.h"
#include "stdf.h"
#include "stdfrecords_v3.h"
#include "stdfparse.h"


class CGSTDFV3toSTDF
{
public:
    CGSTDFV3toSTDF();
    ~CGSTDFV3toSTDF();
	
    ///
    ///  \brief Convert the stdf v3 file to stdf V4 file
    ///  \param CsmcFileName contains the name of the input file (stdf v3)
    ///  \param strFileNameSTDFV4 contains the name of the output file (stdf v4)
    bool		Convert(const char *CsmcFileName, const char *strFileNameSTDFV4);

    //! \brief Return the Last error
	QString		GetLastError();

private:
    //! \brief Read the FAR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteFAR(GQTL_STDF::Stdf_FAR_V3& record);

    //! \brief Read the MIR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteMIR(GQTL_STDF::Stdf_MIR_V3& record);

    //! \brief Read the MRR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteMRR(GQTL_STDF::Stdf_MRR_V3& record);

    //! \brief Read the HBR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteHBR(GQTL_STDF::Stdf_HBR_V3& record);

    //! \brief Read the SBR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteSBR(GQTL_STDF::Stdf_SBR_V3& record);

    //! \brief Read the PMR record from the STDF V3 and write the STDF V4's one
    bool ReadPMR(GQTL_STDF::Stdf_PMR_V3& record);

    //! \brief Read the WIR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteWIR(GQTL_STDF::Stdf_WIR_V3& record);

    //! \brief Read the WRR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteWRR(GQTL_STDF::Stdf_WRR_V3& record);

    //! \brief Read the WCR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteWCR(GQTL_STDF::Stdf_WCR_V3& record);

    //! \brief Read the PIR record from the STDF V3 and write the STDF V4's one
    bool ReadWritePIR(GQTL_STDF::Stdf_PIR_V3& record);

    //! \brief Read the PRR record from the STDF V3 and write the STDF V4's one
    bool ReadWritePRR(GQTL_STDF::Stdf_PRR_V3& record);

    //! \brief Read the PDR record from the STDF V3
    bool ReadPDR(GQTL_STDF::Stdf_PDR_V3& record);

    //! \brief Read the FDR record from the STDF V3
    bool ReadFDR(GQTL_STDF::Stdf_FDR_V3& record);

    //! \brief Read the TSR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteTSR(GQTL_STDF::Stdf_TSR_V3& record);

    //! \brief Read the PTR record from the STDF V3 and write the STDF V4's one
    bool ReadWritePTR(GQTL_STDF::Stdf_PTR_V3& record);

    //! \brief Read the FTR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteFTR(GQTL_STDF::Stdf_FTR_V3& record);

    //! \brief Read the BPS record from the STDF V3 and write the STDF V4's one
    bool ReadWriteBPS(GQTL_STDF::Stdf_BPS_V3& record);

    //! \brief Read the EPS record from the STDF V3 and write the STDF V4's one
    bool ReadWriteEPS(GQTL_STDF::Stdf_EPS_V3& record);

    //! \brief Read the SHB record from the STDF V3 and write the STDF V4's one
    bool ReadWriteSHB(GQTL_STDF::Stdf_SHB_V3& record);

    //! \brief Read the SSB record from the STDF V3 and write the STDF V4's one
    bool ReadSSBWriteSSB(GQTL_STDF::Stdf_SSB_V3& record);

    //! \brief Read the STS record from the STDF V3
    bool ReadWriteSTS(GQTL_STDF::Stdf_STS_V3& record);

    //! \brief Read the SCR record from the STDF V3 and write the STDF V4's PCR
    bool ReadSCRWritePCR(GQTL_STDF::Stdf_SCR_V3& record);

    //! \brief Read the GDR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteGDR(GQTL_STDF::Stdf_GDR_V3 &record);

    //! \brief Read the DTR record from the STDF V3 and write the STDF V4's one
    bool ReadWriteDTR(GQTL_STDF::Stdf_DTR_V3& record);


	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
//    int		mProgressStep;
//    int		mFileSize;
//    int		mNextFilePos;
//    int		mNextParameter;
//    int		mTotalParameters;

    QString mStrLastError;                  /// \param Holds last error string during STDF V3->STDF V4convertion
    int		mIntLastError;					/// \param Holds last error ID
    enum  errCodes
    {
        errNoError,							/// \param No erro (default)
        errOpenFail,						/// \param Failed Opening CSMC file
        errInvalidFormat,					/// \param Invalid CSMC format
        errInvalidFormatLowInRows,			/// \param Didn't find parameter rows
        errLicenceExpired,					/// \param File date out of Licence window!
        errWriteSTDF						/// \param Failed creating STDF intermediate file
    };
    enum eLimitType
    {
        eLowLimit,							/// \param Flag to specify to save the CSMC Parameter LOW limit
        eHighLimit							/// \param Flag to specify to save the CSMC Parameter HIGH limit
    };

    QString		m_strNodeName;				/// \param ProcessID
    long		m_lStartTime;				/// \param Startup time

    GQTL_STDF::Stdf_FAR_V3	mStdfV3FAR;     /// \param mStdfV3FAR FAR V3 record
    GQTL_STDF::Stdf_MIR_V3	mStdfV3MIR;     /// \param mStdfV3MIR MIR V3 record
    GQTL_STDF::Stdf_MRR_V3	mStdfV3MRR;     /// \param mStdfV3MRR MRR V3 record
    GQTL_STDF::Stdf_HBR_V3	mStdfV3HBR;     /// \param mStdfV3HBR HBR V3 record
    GQTL_STDF::Stdf_SBR_V3	mStdfV3SBR;     /// \param mStdfV3SBR SBR V3 record
    GQTL_STDF::Stdf_PMR_V3	mStdfV3PMR;     /// \param mStdfV3PMR PMR V3 record
    GQTL_STDF::Stdf_WIR_V3	mStdfV3WIR;     /// \param mStdfV3WIR WIR V3 record
    GQTL_STDF::Stdf_WRR_V3	mStdfV3WRR;     /// \param mStdfV3WRR WRR V3 record
    GQTL_STDF::Stdf_WCR_V3	mStdfV3WCR;     /// \param mStdfV3WCR WCR V3 record
    GQTL_STDF::Stdf_PIR_V3	mStdfV3PIR;     /// \param mStdfV3PIR PIR V3 record
    GQTL_STDF::Stdf_PRR_V3	mStdfV3PRR;     /// \param mStdfV3PRR PRR V3 record
    GQTL_STDF::Stdf_PDR_V3	mStdfV3PDR;     /// \param mStdfV3PDR PDR V3 record
    GQTL_STDF::Stdf_FDR_V3	mStdfV3FDR;     /// \param mStdfV3FDR FDR V3 record
    GQTL_STDF::Stdf_TSR_V3	mStdfV3TSR;     /// \param mStdfV3TSR TSR V3 record
    GQTL_STDF::Stdf_PTR_V3	mStdfV3PTR;     /// \param mStdfV3PTR PTR V3 record
    GQTL_STDF::Stdf_FTR_V3	mStdfV3FTR;     /// \param mStdfV3FTR FTR V3 record
    GQTL_STDF::Stdf_BPS_V3	mStdfV3BPS;     /// \param mStdfV3BPS BPS V3 record
    GQTL_STDF::Stdf_EPS_V3	mStdfV3EPS;     /// \param mStdfV3EPS EPS V3 record
    GQTL_STDF::Stdf_SHB_V3	mStdfV3SHB;     /// \param mStdfV3SHB SHB V3 record
    GQTL_STDF::Stdf_SSB_V3	mStdfV3SSB;     /// \param mStdfV3SSB SSB V3 record
    GQTL_STDF::Stdf_STS_V3	mStdfV3STS;     /// \param mStdfV3STS STS V3 record
    GQTL_STDF::Stdf_SCR_V3	mStdfV3SCR;     /// \param mStdfV3SCR SCR V3 record
    GQTL_STDF::Stdf_GDR_V3	mStdfV3GDR;     /// \param mStdfV3GDR GDR V3 record
    GQTL_STDF::Stdf_DTR_V3	mStdfV3DTR;     /// \param mStdfV3DTR DTR V3 record

    GQTL_STDF::StdfParse	mStdfParse;		/// \param mStdfParse STDF parser
    GS::StdLib::Stdf        mStdfFile;      /// \param mStdfFile STDF file reader and writer

    QMap<stdf_type_u4, GQTL_STDF::Stdf_PDR_V3> mPDRList; /// \param mPDRList Map containg the list of learnt PDR.
                                                          /// The key is the test number
    QMap<stdf_type_u4, GQTL_STDF::Stdf_FDR_V3> mFDRList; /// \param mFDRList Map containg the list of learnt FDR.
                                                          /// The key is the test number

    QList<stdf_type_u4>         mPTRList;                    /// \param mPTRList STDF file reader and writer
    int                         mNbrePart;
    bool                        mFARWritten;
    QMap<int, char>             mSBinPassFail;
    QMap<int, char>             mHBinPassFail;
    QMap<stdf_type_u4, char>    mTSRTestTyp;
};


#endif
