#ifndef IMPORTATDFTOSTDF_H
#define IMPORTATDFTOSTDF_H

#include "parserBase.h"
#include <gstdl_macro.h>
#include "parserBinning.h"
#include "vishay_stdf_record_overwrite.h"

namespace GS
{
namespace Parser
{

class AtdfToStdf : public ParserBase
{
// Constructor / Destructor
public:
    AtdfToStdf();
    virtual ~AtdfToStdf();

// Attributes
public:
    GDECLARE_ERROR_MAP(AtdfToStdf)
    {
        eErrOkay,				// Successful
        eErrOpen,				// Cannot open file
        eErrMapFile,			// Cannot map file
        eErrPrematureEOF,		// Premature end of file found
        eErrBadRecordHeader,	// Bad record header found
        eErrBadCpu,				// Unsupported CPU type
        eErrBadChar,			// Bad character definition found
        eErrBadStdfVersion,		// Bad STDF version found
        eErrBadAtdfVersion,		// Bad ATDF version found
        eErrWriteHeader,		// Cannot write record header to memory
        eErrWriteRecord,		// Cannot write record to STDF file
        eErrBadDateFormat,		// Invalid date format found
        eErrFieldOverflow,		// Field larger than expected
        eErrBadArrayFormat,		// Invalid array definition found
        eErrInvalidArraySize,	// Found an unexpected array size
        eErrInvalidArrayValue,	// Unexpected array value
        eErrEmpty,				// The current field is empty
        eErrNotValidFlag,		// An invalid flag value has been found
        eErrUnknownRec,			// Unknown record found
        eErrGDR,				// Error analyze GDR record
        eErrEndOfFile,			// End of file reach
        eErrInvalidHex,			// Invalid hexadecimal character found
        eErrNullPointer,        // Null pointer passed to function
        eErrBufferOverflow		// Buffer overflow
    }
    GDECLARE_END_ERROR_MAP(AtdfToStdf)


    enum Flags {
        eFlagOverwrite	= 0x1	// Set this flag to overwrite the destination file is exists
    };

    unsigned int		GetLastAnalyzedLineNbr() const;

public:

    bool    ConvertoStdf(const QString& aAtdfFile,  QString &aStdfFile);

     static bool IsCompatible(const QString &aFileName);

protected:

    bool               mUseExternalFile;
    VishayStdfRecordOverWrite mVishayStdfRecordOverWrite;
    bool               mWaferFound;
    bool    		   mBinMapFileEnabled;
    VishayStdfRecordOverWrite::Type  mTypeFile;
    QString            mTypeFileStr;
    QString            mInputFile;
    int                mFailedTest;

    GS::StdLib::Stdf	*mStdf;			// The STDF object manager
    const char	*mSourceData;	// Used to go through the ATDF file
    const char	*mStartOfData;	// Point to the beginninf of the ATDF file
    const char	*mEndOfData;		// Point to the end of the ATDF file
    char		mSeparator;		// Contains the separator character used in the current ATDF file analyzed
    unsigned int		mCurrentLineNbr;	// The current line number during ATDF file analyze
    bool		mScaledData;		// true is parametric test results in PTR and MPR are scaled; otherwise false.

    typedef std::map<DWORD, float, std::less<DWORD> > MapTests;
    MapTests	mMapPTR_LL;
    MapTests	mMapPTR_HL;
    MapTests	mMapMPR_LL;
    MapTests	mMapMPR_HL;

    GQTL_STDF::Stdf_PTR_V4	*mData; // Because PTR is the most used record, avoid constructor call each time. Simply reset all fields to use it

    bool ValidityCheck();
    int		AnalyzeRecord();	// GINLINE most used function
    int		AnalyzeFAR();
    int		AnalyzeATR();
    int		AnalyzeMIR();
    int		AnalyzeMRR();
    int		AnalyzePCR();
    int		AnalyzeHBR();
    int		AnalyzeSBR();
    int		AnalyzePMR();
    int		AnalyzePGR();
    int		AnalyzePLR();
    int		AnalyzeRDR();
    int		AnalyzeSDR();
    int		AnalyzeWIR();
    int		AnalyzeWRR();
    int		AnalyzeWCR();
    int		AnalyzePIR();
    int		AnalyzePRR();
    int		AnalyzeTSR();
    int		AnalyzePTR();
    int		AnalyzeMPR();
    int		AnalyzeFTR();
    int		AnalyzeBPS();
    int		AnalyzeEPS();
    int		AnalyzeGDR();
    int		AnalyzeDTR();
    int     AnalyzeVUR();
    int     AnalyzeCNR();
    int     AnalyzeSTR();
    int     AnalyzePSR();
    int     AnalyzeSSR();
    int     AnalyzeNMR();
    int     AnalyzeCDR();

    // Generic functions
    int		RetrieveChar(char& cData, bool bAsSignedInteger, bool bFailSeveral = true);
    int		RetrieveByte(stdf_type_u1 &bData, bool bHexValue = false);
    int		RetrieveWord(stdf_type_u2& wData, bool bHexValue = false);
    int		RetrieveDWord(stdf_type_u4 &dwData, bool bHexValue = false);
    int		RetrieveLong(stdf_type_i4& iData);
    int		RetrieveShort(stdf_type_i2& sData);
    int		RetrieveFloat(stdf_type_r4& fData);
    int		RetrieveDouble(stdf_type_r8& dfData);
    int     RetrieveULongLong(stdf_type_u8& lData);

    //! \brief Extract a string from the current m_pcSourceData position.
    //	*** m_pcSourceData must point to the first character of the string!
    //	When exiting this function, m_pcSourceData point to the next field
    //	available or to the beginning of a new record header.
    //! \arg String : Receive the string.
    //! \return eErrOkay is successful.
    //			eErrEmpty if there is no string field at the current position.
    //			another code if an error occured.
    int		RetrieveString(QString & String);
    //! \brief Extract a string list from the current m_pcSourceData position.
    //	*** m_pcSourceData must point to the first character of the string list!
    //	When exiting this function, m_pcSourceData point to the next field
    //	available or to the beginning of a new record header.
    //! \arg StringArray : Receive the string list
    //! \return eErrOkay is successful.
    //			eErrEmpty if there is no string field at the current position.
    //			another code if an error occured.
    int		RetrieveStringArray(QStringList &StringList, WORD nCount);

    int     RetrieveChar(char* szString, int MaxSize);
    int		GetMonthNumber(char* szMonth);
    int		RetrieveDate(DWORD &dwDate);
    int		RetrieveGenData(GQTL_STDF::stdf_type_vn &genData);
    int		RetrieveCharArray(WORD& wCount, char** ppcArray, bool bKeepEmptyField = true, char cPadChar = '0');
    int		RetrieveNibbleArray(WORD& wCount, stdf_type_n1 **ppbArray);
    int     RetrieveByteArray(WORD& wCount, BYTE ** ppbArray);
    int		RetrieveWordArray(WORD& wCount, WORD** ppwArray);
    int     RetrieveDWordArray(WORD &wCount, stdf_type_u4 **ppdwArray);
    int		RetrieveFloatArray(WORD& wCount, float** ppfArray);
    int     RetrieveULongLongArray(WORD& wCount, unsigned long long ** ppwArray);
    int		RetrieveIndexArray(GQTL_STDF::stdf_type_dn & dnField);
    int		RetrieveHexaString(GQTL_STDF::stdf_type_bn & bnField);
    int		ReachNextRecordHeader();
    void	ScalValue(float& fNumber, QString &lUnits, char& cScalValue, bool bKeepUnit = true);
    int		GetCurrentLineIndex() const;

    // Specific extraction function
    int     RetrievePLR_CHAL_CHAR_Array(QString **CHAL, QString **CHAR, WORD nCount);
    int     RetrieveHexString(char* szString);

    // Inline functions
    int     RetrieveField(QString& lField);
    int     RetrieveField(char* Field, int MaxSize);
    bool	RecordContinueOnNextLine();
    int     CheckRecordHeader();
    int     ReachNextField();

    // Convertion functions
    DWORD	gatodw(PCSTR szDWord);
    DWORD	gahextodw(PCSTR szDWord);
    int		gatoi(PCSTR szInt);
    float	gatof(PCSTR szFloat);
    unsigned long long gatoull(PCSTR szNumber);

#ifdef _DEBUG
    void	_AssertValidInteger(const char* szInteger);
    void	_AssertValidUnsigned(const char* szUnsigned);
    void	_AssertValidHexadecimal(const char* szHexadecimal);
    void	_AssertValidFloat(const char* szFloat);
#endif
    QString mAnalysedParam; // delete me
};
} // Parser
} // GS

#endif // IMPORTATDFTOSTDF_H
