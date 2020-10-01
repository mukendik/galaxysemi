// AtdfToStdf.cpp: ATDF to STDF converter implementation
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes :
//			* About AnalyzeXXX() functions (where XXX is the record type name; e.g.: MIR)
//				1) Before to call these functions, mSourceData must point to the first
//				character after the record header (so after the 4 first bytes).
//				2) When exiting these functions, mSourceData must point to the first
//				character of the next record header.
//
//			*** RTN_STAT in record MPR optionaly use comma. So hanle this!!!
//
//			*** Verifier que tous les array utilise la methode comme pour le record SDR, field: SITE_CNT & SITE_NUM
//
//			*** gex doit etre corrige pour traiter de la meme maniere un record SDR pas complet ou un record SDR
//				qui contient uniquement des champ à "invalid value" sur la fin.
//				Et remettre MACRO dans gstdf4
//
//			*** Date: decallage de 1heure... : Solution implementer dans RetrieveDate(), à tester.
//
// ----------------------------------------------------------------------------------------------------------




#if defined(_WIN32)

#	include <sys/types.h>
#	include <sys/timeb.h>

//#	pragma intrinsic(strcpy,memcpy) // All these functions calls will be replace by inline functions

#elif defined(__unix__)


//#	pragma inline(strcpy,memcpy)

#endif // defined(_WIN32)

#include <QFileInfo>
#include <gstdl_filemap.h>
#include <gqtl_log.h>
#include "importAtdfToStdf.h"
#include "converter_external_file.h"


using namespace std;

#define _GATS_ERRORMSG_SIZE		1024
#define _GATS_STRICT_			0	// 0 = no error if a required field is missing; 1 = error if a required field is missing

// ----------------------------------------------------------------------------------------------------------
// MACRO DEFINITIONS
// ----------------------------------------------------------------------------------------------------------

// Test if mSourceData point to the beginning of a new record header
// *** THIS MACRO ONLY WORKS IF IT IS IMPOSSIBLE TO FIND SEVERAL CONSECUTIVE LINE FEED CHARACTERS
#define _GATS_IS_NEW_RECORD()	( (mSourceData ==  mStartOfData) ||\
                                  ( *(mSourceData-1) == '\n' &&\
                                   *mSourceData != ' ' ) )
// Test if mSourceData point to the end of a record field
#define _GATS_IS_ENDOF_FIELD()	( *mSourceData == mSeparator ||\
                                ( ((*mSourceData == '\n') ||\
                                    (*mSourceData == '\r')) &&\
                                    (RecordContinueOnNextLine() == false)) ||\
                                  mSourceData >= mEndOfData )
// Write a specific record
// Arguments:
//				rec:	A CGStdf4_Record derived object
//				pstdf:  A pointer to a GS::StdLib::Stdf object
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///  To be modified
#define _GATS_WRITE_RECORD(rec,pstdf)	if(rec.Write(*pstdf) == false) {\
                                        GSET_ERROR0(AtdfToStdf, eErrWriteRecord, NULL);\
                                        return eErrWriteRecord;}
// Test if the specified buffer is overflow
// Arguments:
//				bufStart: Address of the beginning of the buffer
//				bufCurrent: The current address in the buffer
//				size: The buffer size
#define _GATS_IS_BUFFER_OVERFLOW(bufStart,bufCurrent,size)\
                                        (bufCurrent-bufStart == size &&\
                                        *mSourceData !=  mSeparator &&\
                                        mSourceData < mEndOfData &&\
                                        *mSourceData != '\n' &&\
                                        *mSourceData != '\r' )
// Test if the specified error code represent an error value for a retrieve operation result
// (return a zero value if the field is missing or not but has been correctly retrieved)
// Argument:
//				status: The error code to test
#define _GATS_ERROR_RETRIEVE(status) (status != eErrOkay && status != eErrEmpty && status != eErrEndOfFile)

// Test if the specified error code represent an error value for a retrieve operation result
// (return a zero value only if the the has been correctly retrieved; a missing field
//  give a postive value)
#if _GATS_STRICT_ == 1
#	define _GATS_ERROR_RETRIEVE_REQUIRED(status)	(status != eErrOkay)
#else
#	define _GATS_ERROR_RETRIEVE_REQUIRED(status)	_GATS_ERROR_RETRIEVE(status)
#endif

#ifdef _DEBUG
#	define ASSERT_VALIDINTEGER(s)		_AssertValidInteger(s)
#	define ASSERT_VALIDUNSIGNED(s)		_AssertValidUnsigned(s)
#	define ASSERT_VALIDHEXADECIMAL(s)	_AssertValidHexadecimal(s)
#	define ASSERT_VALIDFLOAT(s)			_AssertValidFloat(s)
#else
#	define ASSERT_VALIDINTEGER(s)
#	define ASSERT_VALIDUNSIGNED(s)
#	define ASSERT_VALIDHEXADECIMAL(s)
#	define ASSERT_VALIDFLOAT(s)
#endif

#define _GATS_READ_Ux_ARRAY_FIELD(status,count,dest,expectedsize,readfunc,setfunc)\
    {\
        status = readfunc(lCount, &dest);\
        if(_GATS_ERROR_RETRIEVE(status))\
            return lStatus;\
        if(lCount != expectedsize)\
        {\
            GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(expectedsize),int(count));\
            return eErrInvalidArraySize;\
        }\
        for (stdf_type_u2 lIdx = 0; lIdx < expectedsize; ++lIdx)\
            setfunc(lIdx, dest[lIdx]);\
    }

#define _GATS_READ_CN_ARRAY_FIELD(status,stringlist,size,setfunc)\
    {\
        status = RetrieveStringArray(stringlist, size);\
        if(_GATS_ERROR_RETRIEVE(lStatus))\
            return lStatus;\
        for (int lIdx = 0; lIdx < stringlist.count(); ++lIdx)\
            setfunc(lIdx, stringlist.at(lIdx));\
    }

const QString cFinalTest = "final_tests";
const QString cSortEntries = "sort_entries";


namespace GS
{
namespace Parser
{


// ----------------------------------------------------------------------------------------------------------
// ERROR MAP
// ----------------------------------------------------------------------------------------------------------
GBEGIN_ERROR_MAP(AtdfToStdf)
    GMAP_ERROR(eErrOkay,"Successful")
    GMAP_ERROR(eErrOpen,"Error opening STDF file (error code = %d)") // To avoid redundant message, nothing at this level
    GMAP_ERROR(eErrMapFile," ") // idem
    GMAP_ERROR(eErrPrematureEOF,"Premature end of file found at line %d, index %d")
    GMAP_ERROR(eErrBadRecordHeader,"Bad record header found at line %d, index %d")
    GMAP_ERROR(eErrBadCpu,"Unexpected CPU type found")
    GMAP_ERROR(eErrBadChar,"Unexpected character definition found at line: %d, index %d")
    GMAP_ERROR(eErrBadStdfVersion,"Unexpected STDF version found")
    GMAP_ERROR(eErrBadAtdfVersion,"Unexpected ATDF version found")
    GMAP_ERROR(eErrWriteHeader,"Cannot write record header to memory")
    GMAP_ERROR(eErrWriteRecord,"Cannot write record to STDF file")
    GMAP_ERROR(eErrBadDateFormat,"Invalid date format found at line %d, index %d")
    GMAP_ERROR(eErrFieldOverflow,"Field larger than expected")
    GMAP_ERROR(eErrBadArrayFormat,"Invalid array definition found at line %d, index %d")
    GMAP_ERROR(eErrInvalidArraySize,"Found an unexpected array size found at line:%d, index %d.\nShould be: %d, found: %d")
    GMAP_ERROR(eErrInvalidArrayValue,"Unexpected array value found at line: %d, index %d")
    GMAP_ERROR(eErrEmpty,"Missing field at line %d, index %d")
    GMAP_ERROR(eErrNotValidFlag,"Unexpected flag value found at line: %d, index %d");
    GMAP_ERROR(eErrUnknownRec,"Unknown record (%s) found at line %d")
    GMAP_ERROR(eErrGDR,"Unexpected Generic Data Record field found at line: %d, index %d")
    GMAP_ERROR(eErrEndOfFile,"End of file reach")
    GMAP_ERROR(eErrInvalidHex,"Invalid hexadecimal character found at line: %d, index %d")
    GMAP_ERROR(eErrNullPointer,"Null pointer passed to function %s")
    GMAP_ERROR(eErrBufferOverflow,"Buffer overflow in function %s")
GEND_ERROR_MAP(AtdfToStdf)

// ----------------------------------------------------------------------------------------------------------
// PUBLIC CODE
// ----------------------------------------------------------------------------------------------------------

AtdfToStdf::AtdfToStdf() : ParserBase(typeAtdfToStdf, "AtdfToStdf"),
        mUseExternalFile(false),
        mTypeFile(VishayStdfRecordOverWrite::undefinedType),
        mFailedTest(-1),
        mStdf(NULL),
        mSourceData(NULL),
        mStartOfData(NULL),
        mEndOfData(NULL),
        mCurrentLineNbr(0),
        mScaledData(true) // If no scaled flag found, data is assumed to be scaled
{
    mStdf = new GS::StdLib::Stdf();
    mData = new GQTL_STDF::Stdf_PTR_V4();
    mWaferFound = false;
}

AtdfToStdf::~AtdfToStdf()
{
    if(mStdf != NULL)
        delete mStdf;
    if(mData != NULL)
        delete mData;
}

// Retrieve the number of line analyzed
unsigned int AtdfToStdf::GetLastAnalyzedLineNbr() const
{
    return mCurrentLineNbr;
}

bool AtdfToStdf::IsCompatible(const QString &aFileName)
{
    QFileInfo lFileInfo(aFileName);
    QString lExtension = lFileInfo.suffix().toLower();

    if((lExtension == "atd") || (lExtension == "atdf"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool AtdfToStdf::ValidityCheck()
{
    if(mUseExternalFile)
    {
        // The type define in the external file and the file must ne different from undefined and equal
        if(mVishayStdfRecordOverWrite.GetType() != VishayStdfRecordOverWrite::undefinedType && mTypeFile != VishayStdfRecordOverWrite::undefinedType)
        {
            return mVishayStdfRecordOverWrite.GetType() == mTypeFile;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Convert the specified ATDF file to STDF format.
//
// Argument(s) :
//      szAtdfFile: The name of the ATDF file to convert.
//      szStdfFile: The name of the STDF file created from the ATDF file.
//      uiFlags: Options flag used for convertion.
//				 You can combine the following values:
//					eFlagOverwrite -> If not specified, an error is return if attempting to
//									  overwrite the specified STDF file.
//
//
// Return: true if successful; otherwise return false.
// ----------------------------------------------------------------------------------------------------------
bool
AtdfToStdf::ConvertoStdf(const QString& aAtdfFile,  QString &aStdfFile)
{

    if(aAtdfFile==NULL || aStdfFile==NULL)
        return false;

    int 		iStatus=-1;
    CGFileMap	lFileMap;
    QString lErrorMsg;

    mInputFile = aAtdfFile;
    mUseExternalFile = mVishayStdfRecordOverWrite.LoadExternalFiles(aAtdfFile, Qx::BinMapping::validity_check, lErrorMsg, mLastError);
    // Reset the mLastError to 0 because this fix is mainly done for the STDF files and not for ATDF
    if (mLastError == errInvalidFormatParameter)
        mLastError = 0;

    if(mUseExternalFile)
    {
        if(!getOptDefaultBinValuesfromExternalFile(aAtdfFile, lErrorMsg))
        {
            return false;
            //GSLOG(SYSLOG_SEV_INFORMATIONAL, qPrintable("getOptDefaultBinValuesfromExternalFile return false: " + lErrorMsg));
        }
        if(mDefaultBinSet)
        {
            mVishayStdfRecordOverWrite.setDefaultBinValues(mDefaultBinName, mDefaultBinNumber);
        }
    }
    else{
        if(!lErrorMsg.isEmpty())
        {
            GSLOG(SYSLOG_SEV_ERROR, qPrintable("LoadExternalFiles return false: " + lErrorMsg));
            mLastError = errConverterExternalFile;
            mLastErrorMessage = lErrorMsg;
            return false;
        }
        if(mLastError == errReadBinMapFile)
        {
            mLastErrorMessage = lErrorMsg;
            return false;
        }
    }

    if(lErrorMsg.isEmpty() == false)
    {
        mLastErrorMessage = lErrorMsg;
        mLastError = errReadBinMapFile;
    }

    // Default initialization
    mCurrentLineNbr = 1;

    if(lFileMap.MapFile(aAtdfFile.toStdString().c_str()) == false)
    {
        GSET_ERROR0(AtdfToStdf,eErrMapFile,GGET_LASTERROR(CGFileMap,&lFileMap));
        return false;
    }

    mStartOfData = mSourceData = (char*)lFileMap.GetReadMapData();
    mEndOfData = mSourceData + lFileMap.GetFileSize();

    int lOpenStatus = mStdf->Open(aStdfFile.toStdString().c_str() ,STDF_WRITE);

    if(lOpenStatus != GS::StdLib::Stdf::NoError)
    {
        GSET_ERROR1(AtdfToStdf,eErrOpen,NULL,lOpenStatus);
        QFile::remove(aStdfFile);
        return false;
    }

    DWORD	dwCount = 0, dwGlobalCount = 0;

    mFailedTest = -1;
    while(mSourceData < mEndOfData)
    {
        iStatus = AnalyzeRecord();

        if(ValidityCheck() == false)
        {

            mLastErrorMessage =
                      QStringLiteral("Mismatch betwween the adtf file type [%1] and the external file type [%2]. "
                                     ).arg(mVishayStdfRecordOverWrite.TypeToString(mTypeFile)
                                      .arg(mVishayStdfRecordOverWrite.TypeToString(mVishayStdfRecordOverWrite.GetType())));

            mLastError = errInvalidFormatParameter;
            mStdf->Close();
            QFile::remove(aStdfFile);
            return false;
        }

        if(iStatus != eErrOkay && iStatus != eErrEndOfFile)
        {
            if(iStatus == eErrEmpty) // In this case, no error message could be formatted
            {	// So format it
                GSET_ERROR2(AtdfToStdf,eErrEmpty,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            }
            mStdf->Close();
            QFile::remove(aStdfFile);
            return false;
        }

        dwCount++;
        dwGlobalCount++;
        // Every 500 records, look if we need to call the progress virtual function
        if(dwCount > 500)
        {
            UpdateProgressBar();
            dwCount = 0;
        }
    }

    mStdf->Close();

    return true;
}


// ----------------------------------------------------------------------------------------------------------
// PROTECTED CODE
// ----------------------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------------------
// Description: Main function to analyze an ATDF record.
//				This function uses embedded switch/case statements for performance issue.
//
// Return: eErrOkay is successful.
//		   eErrEndOfFile if we have reach the end of the file.
//		   Another error code if an error occured when analyzing the current record.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeRecord()
{
    int		iStatus;

    switch(*mSourceData)
    {
    case 'P':
        switch(*(mSourceData+1))
        {
            case 'T':	// ---> PT
                // Find a PTR record. (put it in first position while it is the most common one)
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PTR";
                return AnalyzePTR();

            case 'M':	// ---> PM
                // Find a PMR record. (put it in first position while it is the most common one)
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PMR";
                return AnalyzePMR();

            case 'I':	// ---> PI
                // Find a PIR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PIR";
                return AnalyzePIR();

            case 'R':	// ---> PR
                // Find a PRR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PRR";
                return AnalyzePRR();

            case 'C':	// ---> PC
                // Find a PCR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                return AnalyzePCR();

            case 'G':	// ---> PG
                // Find a PGR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PGR";
                return AnalyzePGR();

            case 'L':	// ---> PL
                // Find a PLR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PLR";
                return AnalyzePLR();

            case 'S':	// ---> PS
                // Find a PSR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "PSR";
                return AnalyzePSR();

            default:
                break;

        } // switch(*(mSourceData+1))
        break;

    case 'V':
        switch(*(mSourceData+1))
        {
            case 'U':	// ---> VU
                // Find a VUR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "VUR";
                return AnalyzeVUR();

            default:
                break;
        }
        break;

    case 'T':
        // Find a TSR record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "TSR";
        return AnalyzeTSR();

    case 'N':
        // Find a NMR record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "NMR";
        return AnalyzeNMR();

    case 'M':
        switch(*(mSourceData+1))
        {
        case 'P':	// ---> MP
            // Find a MPR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "MPR";
            return AnalyzeMPR();

        case 'I':	// ---> MI
            // Find a MIR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "MIR";
            return AnalyzeMIR();

        case 'R':	// ---> MR
            // Find a MRR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "MRR";
            return AnalyzeMRR();
        }
        break;

    case 'F':
        switch(*(mSourceData+1))
        {
        case 'T':	// ---> FT
            // Find a FTR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "FTR";
            return AnalyzeFTR();

        case 'A':	// ---> FA
            // Find a FAR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "FAR";
            return AnalyzeFAR();
        }
        break;

    case 'H':
        // Find a HBR record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "HBR";
        return AnalyzeHBR();

    case 'S':
        switch(*(mSourceData+1))
        {
            case 'B':	// ---> SB
                // Find a SBR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "SBR";
                return AnalyzeSBR();

            case 'D':	// ---> SD
                // Find a SDR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "SDR";
                return AnalyzeSDR();

            case 'S':	// ---> SS
                // Find a SSR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "SSR";
                return AnalyzeSSR();

            case 'T':	// ---> ST
                // Find a STR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "STR";
                return AnalyzeSTR();

            default:
                break;
        }
        break;

    case 'R':
        // Find a RDR record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "RDR";
        return AnalyzeRDR();

    case 'W':
        switch(*(mSourceData+1))
        {
        case 'I':	// ---> WI
            // Find a WIR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "WIR";
            mWaferFound = true;

            if(mUseExternalFile)
            {
                mTypeFile = VishayStdfRecordOverWrite::waferType;
            }

            return AnalyzeWIR();

        case 'R':	// ---> WR
            // Find a WRR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "WRR";
            mWaferFound = true;
            if(mUseExternalFile)
            {
                mTypeFile = VishayStdfRecordOverWrite::waferType;
            }
            return AnalyzeWRR();

        case 'C':	// ---> WC
            // Find a WCR record?
            iStatus = CheckRecordHeader();
            if(iStatus != eErrOkay)
                return iStatus;
            mAnalysedParam = "WCR";
            mWaferFound = true;
            if(mUseExternalFile)
            {
                mTypeFile = VishayStdfRecordOverWrite::waferType;
            }
            return AnalyzeWCR();
        }
        break;

    case 'B':
        // Find a BPS record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "BPS";
        return AnalyzeBPS();

    case 'E':
        // Find a EPS record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "EPS";
        return AnalyzeEPS();

    case 'G':
        // Find a GDR record?
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "GDR";
        return AnalyzeGDR();

    case 'D':
        // Make sure the record header is valid
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "DTR";
        return AnalyzeDTR();

    case 'A':
        // Make sure the record header is valid
        iStatus = CheckRecordHeader();
        if(iStatus != eErrOkay)
            return iStatus;
        mAnalysedParam = "ATR";
        return AnalyzeATR();
    case 'C':
        switch(*(mSourceData+1))
        {
            case 'N':
                // Make sure the record header is valid
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "CNR";
                return AnalyzeCNR();

            case 'D':
                // Find a CDR record?
                iStatus = CheckRecordHeader();
                if(iStatus != eErrOkay)
                    return iStatus;
                mAnalysedParam = "CDR";
                return AnalyzeCDR();

            default:
                break;
        }
    }

    char szRec[4];
    szRec[0] = mSourceData[0];
    szRec[1] = mSourceData[1];
    szRec[2] = mSourceData[2];
    szRec[3] = '\0';
    GSET_ERROR2(AtdfToStdf,eErrUnknownRec,NULL,szRec,mCurrentLineNbr);
    return eErrUnknownRec;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a FAR record.
//				 mSourceData should point to the first character after the end of the
//				 FAR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeFAR()
{
    bool		bContinue = true;
    char		cBuffer;
    int			iStatus;
    GQTL_STDF::Stdf_FAR_V4	farData;
    int			iCurrentField = 0;

    // Reach premature end of file?
    if(mSourceData + 5 > mEndOfData) // At least 5 character in the record
    {
        GSET_ERROR2(AtdfToStdf,eErrPrematureEOF,NULL,mCurrentLineNbr,GetCurrentLineIndex());
        return eErrPrematureEOF;
    }
    // Get the separator used in this ATDF file
    mSeparator = *(mSourceData+1);

    do
    {
        switch(iCurrentField)
        {
        case 0: // CPU_TYPE
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            if(cBuffer != 'A')
            {
                GSET_ERROR(AtdfToStdf,eErrBadCpu);
                return eErrBadCpu;
            }
            // Always force to 1 in the STDF
            farData.SetCPU_TYPE(1);
            break;
        case 1: // STDF_VER
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            if(cBuffer != '4')
            {
                GSET_ERROR(AtdfToStdf,eErrBadStdfVersion);
                return eErrBadStdfVersion;
            }
            // Always force to 4
            farData.SetSTDF_VER(4);
            break;
        case 2: // ATDF version
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            if(cBuffer != '2')
            {
                GSET_ERROR(AtdfToStdf,eErrBadAtdfVersion);
                return eErrBadAtdfVersion;
            }
            break;
        case 3: // Scaling Flag
            iStatus = RetrieveChar(cBuffer,false);
            if(iStatus == eErrOkay)
            {
                if(cBuffer == 'S')
                    mScaledData = true;
                else
                if(cBuffer == 'U')
                    mScaledData = false;
                else
                {
                    GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                    return eErrNotValidFlag;
                }
            }
            else
            if(iStatus != eErrEmpty)	return iStatus; // Error
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop FAR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(farData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a ATR record.
//				 mSourceData should point to the first character after the end of the
//				 ATR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeATR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_ATR_V4	atrData;
    int			iCurrentField = 0;
    DWORD lModTime;

    do
    {
        switch(iCurrentField)
        {
        case 0: // MOD_TIM
        {
            iStatus = RetrieveDate(lModTime);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            break;
        }
        case 1: // CMD_LINE
            iStatus = RetrieveString(atrData.m_cnCMD_LINE);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop ATR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    atrData.SetMOD_TIM((time_t)lModTime);
    atrData.SetCMD_LINE();
    _GATS_WRITE_RECORD(atrData,mStdf);


    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a MIR record.
//				 mSourceData should point to the first character after the end of the
//				 MIR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeMIR()
{
    bool bContinue = true;
    int	iStatus;
    GQTL_STDF::Stdf_MIR_V4 mirData;
    int	iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // LOT_ID
        {
            iStatus = RetrieveString(mirData.m_cnLOT_ID);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            mirData.SetLOT_ID(mirData.m_cnLOT_ID);

            break;
        }
        case 1: // PART_TYP
        {
            iStatus = RetrieveString(mirData.m_cnPART_TYP);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            mirData.SetPART_TYP(mirData.m_cnPART_TYP);
            break;
        }
        case 2: // JOB_NAM
            iStatus = RetrieveString(mirData.m_cnJOB_NAM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            mirData.SetJOB_NAM(mirData.m_cnJOB_NAM);
            break;
        case 3: // NODE_NAM
        {
            iStatus = RetrieveString(mirData.m_cnNODE_NAM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            mirData.SetNODE_NAM(mirData.m_cnNODE_NAM);
            break;
        }
        case 4: // TSTR_TYP
        {
            iStatus = RetrieveString(mirData.m_cnTSTR_TYP);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            mirData.SetTSTR_TYP(mirData.m_cnTSTR_TYP);
            break;
        }
        case 5: // SETUP_T
        {
            DWORD lModTime;
            iStatus = RetrieveDate(lModTime);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mirData.SetSETUP_T((time_t)lModTime);
            break;
        }
        case 6: // START_T
        {
            DWORD lModTime;
            iStatus = RetrieveDate(lModTime);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mirData.SetSTART_T((time_t)lModTime);
            break;
        }
        case 7: // OPER_NAM
        {
            iStatus = RetrieveString(mirData.m_cnOPER_NAM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mirData.SetOPER_NAM(mirData.m_cnOPER_NAM);
            break;
        }
        case 8: // MODE_COD
            iStatus = RetrieveChar(mirData.m_c1MODE_COD,false);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mirData.SetMODE_COD(mirData.m_c1MODE_COD);
            break;
        case 9: // STAT_NUM
            iStatus = RetrieveByte(mirData.m_u1STAT_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mirData.SetSTAT_NUM(mirData.m_u1STAT_NUM);
            // initialize mundatory parameter in the stdf file
            mirData.SetRTST_COD(' ');
            mirData.SetPROT_COD(' ');
            mirData.SetBURN_TIM(65535);
            mirData.SetCMOD_COD(' ');
            break;
        case 10: // SBLOT_ID
        {
            iStatus = RetrieveString(mirData.m_cnSBLOT_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetSBLOT_ID(mirData.m_cnSBLOT_ID);
            break;
        }
        case 11: // TEST_COD
        {
            iStatus = RetrieveString(mirData.m_cnTEST_COD);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetTEST_COD(mirData.m_cnTEST_COD);
            break;
        }
        case 12: // RTST_COD
            iStatus = RetrieveChar(mirData.m_c1RTST_COD,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            if (mirData.m_c1RTST_COD != '\0')
                mirData.SetRTST_COD(mirData.m_c1RTST_COD);
            break;
        case 13: // JOB_REV
            iStatus = RetrieveString(mirData.m_cnJOB_REV);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetJOB_REV(mirData.m_cnJOB_REV);
            break;
        case 14: // EXEC_TYP
            iStatus = RetrieveString(mirData.m_cnEXEC_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetEXEC_TYP(mirData.m_cnEXEC_TYP);
            break;
        case 15: // EXEC_VER
            iStatus = RetrieveString(mirData.m_cnEXEC_VER);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetEXEC_VER(mirData.m_cnEXEC_VER);
            break;
        case 16: // PROT_COD
            iStatus = RetrieveChar(mirData.m_c1PROT_COD,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetPROT_COD(mirData.m_c1PROT_COD);
            break;
        case 17: // CMOD_COD
            iStatus = RetrieveChar(mirData.m_c1CMOD_COD,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetCMOD_COD(mirData.m_c1CMOD_COD);
            break;
        case 18: // BURN_TIM
            iStatus = RetrieveWord(mirData.m_u2BURN_TIM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetBURN_TIM(mirData.m_u2BURN_TIM);
            break;
        case 19: // TST_TEMP
            iStatus = RetrieveString(mirData.m_cnTST_TEMP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetTST_TEMP(mirData.m_cnTST_TEMP);
            break;
        case 20: // USER_TXT
        {
            iStatus = RetrieveString(mirData.m_cnUSER_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetUSER_TXT(mirData.m_cnUSER_TXT);

            /// extract the type of the file (final, wafer, etest) (possible only if generated by gex)
            if(mirData.m_cnUSER_TXT.contains(GEX_IMPORT_DATAORIGIN_LABEL))
            {
                QStringList lList = mirData.m_cnUSER_TXT.split(":");
                if(lList.size() >=2)
                {
                    if(lList[1] == GEX_IMPORT_DATAORIGIN_ETEST) {
                        if(mUseExternalFile)
                        {
                            mTypeFile = VishayStdfRecordOverWrite::eTestType;
                        }
                    }
                }
            }

            break;
        }
        case 21: // AUX_FILE
            iStatus = RetrieveString(mirData.m_cnAUX_FILE);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetAUX_FILE(mirData.m_cnAUX_FILE);
            break;
        case 22: // PKG_TYP
            iStatus = RetrieveString(mirData.m_cnPKG_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetPKG_TYP(mirData.m_cnPKG_TYP);
            break;
        case 23: // FAMLY_ID
            iStatus = RetrieveString(mirData.m_cnFAMLY_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetFAMLY_ID(mirData.m_cnFAMLY_ID);
            break;
        case 24: // DATE_COD
            iStatus = RetrieveString(mirData.m_cnDATE_COD);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetDATE_COD(mirData.m_cnDATE_COD);
            break;
        case 25: // FACIL_ID
            iStatus = RetrieveString(mirData.m_cnFACIL_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetFACIL_ID(mirData.m_cnFACIL_ID);
            break;
        case 26: // FLOOR_ID
            iStatus = RetrieveString(mirData.m_cnFLOOR_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetFLOOR_ID(mirData.m_cnFLOOR_ID);
            break;
        case 27: // PROC_ID
            iStatus = RetrieveString(mirData.m_cnPROC_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetPROC_ID(mirData.m_cnPROC_ID);
            break;
        case 28: // OPER_FRQ
            iStatus = RetrieveString(mirData.m_cnOPER_FRQ);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetOPER_FRQ(mirData.m_cnOPER_FRQ);
            break;
        case 29: // SPEC_NAM
            iStatus = RetrieveString(mirData.m_cnSPEC_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetSPEC_NAM(mirData.m_cnSPEC_NAM);
            break;
        case 30: // SPEC_VER
            iStatus = RetrieveString(mirData.m_cnSPEC_VER);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetSPEC_VER(mirData.m_cnSPEC_VER);
            break;
        case 31: // FLOW_ID
            iStatus = RetrieveString(mirData.m_cnFLOW_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetFLOW_ID(mirData.m_cnFLOW_ID);
            break;
        case 32: // SETUP_ID
            iStatus = RetrieveString(mirData.m_cnSETUP_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetSETUP_ID(mirData.m_cnSETUP_ID);
            break;
        case 33: // DSGN_REV
            iStatus = RetrieveString(mirData.m_cnDSGN_REV);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetDSGN_REV(mirData.m_cnDSGN_REV);
            break;
        case 34: // ENG_ID
            iStatus = RetrieveString(mirData.m_cnENG_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetENG_ID(mirData.m_cnENG_ID);
            break;
        case 35: // ROM_COD
            iStatus = RetrieveString(mirData.m_cnROM_COD);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetROM_COD(mirData.m_cnROM_COD);
            break;
        case 36: // SERL_NUM
            iStatus = RetrieveString(mirData.m_cnSERL_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetSERL_NUM(mirData.m_cnSERL_NUM);
            break;
        case 37: // SUPR_NAM
            iStatus = RetrieveString(mirData.m_cnSUPR_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mirData.SetSUPR_NAM(mirData.m_cnSUPR_NAM);
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop MIR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));


//    if(mUseExternalFile)
//    {
//        QString lError;
//        mVishayStdfRecordOverWrite.UpdateMIRRecordWithPromis(mirData, lError);
//        if(lError.isEmpty() == false)
//        {
//            mLastErrorMessage = lError;
//            mLastError = errReadPromisFile;
//        }
//    }

    _GATS_WRITE_RECORD(mirData,mStdf);

//    if(mUseExternalFile)
//    {
//        GQTL_STDF::Stdf_DTR_V4	lDtrData;
//        if( mVishayStdfRecordOverWrite.WritePromisGrossDieCount(lDtrData) )
//        {
//            _GATS_WRITE_RECORD(lDtrData, mStdf);
//        }

//        GQTL_STDF::Stdf_SDR_V4 lSdrData;
//        if( mVishayStdfRecordOverWrite.WritePromisProber(lSdrData) )
//        {
//            _GATS_WRITE_RECORD(lSdrData, mStdf);
//        }

//        QList<GQTL_STDF::Stdf_DTR_V4*>	aDtrRecords;
//        mVishayStdfRecordOverWrite.WritePromisDieTracking(aDtrRecords);

//        QList<GQTL_STDF::Stdf_DTR_V4*>::iterator lIter(aDtrRecords.begin()), lIterEnd(aDtrRecords.end());
//        for(; lIter != lIterEnd; ++lIter)
//        {
//            GQTL_STDF::Stdf_DTR_V4* lDTRRecord = *lIter;
//            _GATS_WRITE_RECORD( (*lDTRRecord), mStdf);
//        }

//        qDeleteAll(aDtrRecords);

//    }

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a MRR record.
//				 mSourceData should point to the first character after the end of the
//				 MIR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeMRR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_MRR_V4	mrrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // FINISH_T
        {
            DWORD lModTime;
            iStatus = RetrieveDate(lModTime);
            if(iStatus == eErrEndOfFile)
            {
                mrrData.SetFINISH_T((time_t)lModTime);
                bContinue = false;
                break;
            }
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            mrrData.SetFINISH_T((time_t)lModTime);
            break;
        }
        case 1: // DISP_COD
            iStatus = RetrieveChar(mrrData.m_c1DISP_COD,false);
            if(iStatus == eErrEndOfFile)
            {
                mrrData.SetDISP_COD(mrrData.m_c1DISP_COD);
                bContinue = false;
                break;
            }
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            mrrData.SetDISP_COD(mrrData.m_c1DISP_COD);
            break;
        case 2: // USR_DESC
            iStatus = RetrieveString(mrrData.m_cnUSR_DESC);
            if(iStatus == eErrEndOfFile)
            {
                mrrData.SetUSR_DESC(mrrData.m_cnUSR_DESC);
                bContinue = false;
                break;
            }
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            mrrData.SetUSR_DESC(mrrData.m_cnUSR_DESC);
            break;
        case 3: // EXC_DESC
            iStatus = RetrieveString(mrrData.m_cnEXC_DESC);
            if(iStatus == eErrEndOfFile)
            {
                mrrData.SetEXC_DESC(mrrData.m_cnEXC_DESC);
                bContinue = false;
                break;
            }
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            mrrData.SetEXC_DESC(mrrData.m_cnEXC_DESC);
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(!bContinue || _GATS_IS_NEW_RECORD())
            break; // Stop MRR analyze, write data extracted to STDF file

    } while (!((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false));

    _GATS_WRITE_RECORD(mrrData,mStdf);

    return eErrOkay;		// MRR is the last record !
    // return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PCR record.
//				 mSourceData should point to the first character after the end of the
//				 PCR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePCR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_PCR_V4	pcrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(pcrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pcrData.SetHEAD_NUM(pcrData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_NUM
            iStatus = RetrieveByte(pcrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pcrData.SetSITE_NUM(pcrData.m_u1SITE_NUM);
            break;
        case 2: // PART_CNT
            iStatus = RetrieveDWord(pcrData.m_u4PART_CNT);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            pcrData.SetPART_CNT(pcrData.m_u4PART_CNT);
            break;
        case 3: // RTST_CNT
            iStatus = RetrieveDWord(pcrData.m_u4RTST_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pcrData.SetRTST_CNT(pcrData.m_u4RTST_CNT);
            break;
        case 4: // ABRT_CNT
            iStatus = RetrieveDWord(pcrData.m_u4ABRT_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pcrData.SetABRT_CNT(pcrData.m_u4ABRT_CNT);
            break;
        case 5: // GOOD_CNT
            iStatus = RetrieveDWord(pcrData.m_u4GOOD_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pcrData.SetGOOD_CNT(pcrData.m_u4GOOD_CNT);
            break;
        case 6: // FUNC_CNT
            iStatus = RetrieveDWord(pcrData.m_u4FUNC_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pcrData.SetFUNC_CNT(pcrData.m_u4FUNC_CNT);
            break;

        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PCR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));


    _GATS_WRITE_RECORD(pcrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a HBR record.
//				 mSourceData should point to the first character after the end of the
//				 HBR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeHBR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_HBR_V4	hbrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(hbrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            hbrData.SetHEAD_NUM(hbrData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_NUM
            iStatus = RetrieveByte(hbrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            hbrData.SetSITE_NUM(hbrData.m_u1SITE_NUM);
            break;
        case 2: // HBIN_NUM
            iStatus = RetrieveWord(hbrData.m_u2HBIN_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            hbrData.SetHBIN_NUM(hbrData.m_u2HBIN_NUM);
            break;
        case 3: // HBIN_CNT
            iStatus = RetrieveDWord(hbrData.m_u4HBIN_CNT);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            hbrData.SetHBIN_CNT(hbrData.m_u4HBIN_CNT);
            break;
        case 4: // HBIN_PF
            iStatus = RetrieveChar(hbrData.m_c1HBIN_PF,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            hbrData.SetHBIN_PF(hbrData.m_c1HBIN_PF);
            break;
        case 5: // HBIN_NAM
            iStatus = RetrieveString(hbrData.m_cnHBIN_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            hbrData.SetHBIN_NAM(hbrData.m_cnHBIN_NAM);
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop HBR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(hbrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a SBR record.
//				 mSourceData should point to the first character after the end of the
//				 SBR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeSBR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_SBR_V4	sbrData;
    int			iCurrentField = 0;

    /// In the case of lvm ft, SBR/HBR write from spales and must be entirel rewrite
    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(sbrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            sbrData.SetHEAD_NUM(sbrData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_NUM
            iStatus = RetrieveByte(sbrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            sbrData.SetSITE_NUM(sbrData.m_u1SITE_NUM);
            break;
        case 2: // SBIN_NUM
            iStatus = RetrieveWord(sbrData.m_u2SBIN_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            sbrData.SetSBIN_NUM(sbrData.m_u2SBIN_NUM);
            break;
        case 3: // SBIN_CNT
            iStatus = RetrieveDWord(sbrData.m_u4SBIN_CNT);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))
                return iStatus;
            sbrData.SetSBIN_CNT(sbrData.m_u4SBIN_CNT);
            break;
        case 4: // SBIN_PF
            iStatus = RetrieveChar(sbrData.m_c1SBIN_PF,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            sbrData.SetSBIN_PF(sbrData.m_c1SBIN_PF);
            break;
        case 5: // SBIN_NAM
            iStatus = RetrieveString(sbrData.m_cnSBIN_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            sbrData.SetSBIN_NAM(sbrData.m_cnSBIN_NAM);
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop SBR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    if(mUseExternalFile)
    {
        /// check that FAIL bin number/name exists in the external files
        if(sbrData.m_u2SBIN_NUM != 1 && mVishayStdfRecordOverWrite.IsBinCorrect(sbrData.m_u2SBIN_NUM, sbrData.m_cnSBIN_NAM) ==false)
        {
            QPair<QString, QString> lBinNameAndSource = mVishayStdfRecordOverWrite.RetrieveBinNameAndSource(sbrData.m_u2SBIN_NUM);

            if (lBinNameAndSource.first.isEmpty())
            {
                mLastErrorMessage = QStringLiteral("ATDF bin mapping check rejected for file %1 - Bin [%2][%3] "
                                                        "was not found in the mapping files specified in %4")
                        .arg(mInputFile)
                        .arg(QString::number(sbrData.m_u2SBIN_NUM))
                        .arg(sbrData.m_cnSBIN_NAM)
                        .arg(ConverterExternalFile::GetExternalFileName(mVishayStdfRecordOverWrite.GetPathExternalFile()));
            }
            else
            {
                QString lBinSource;
                if (lBinNameAndSource.second == cFinalTest)
                {
                    lBinSource = mVishayStdfRecordOverWrite.mFinalTestFile;
                }
                else // if (lBinNameAndSource.second == cSortEntries)
                {
                    lBinSource = mVishayStdfRecordOverWrite.mSortEntriesFile;
                }


                mLastErrorMessage = QStringLiteral("ATDF bin mapping check rejected for file %1 - Name for Bin [%2][%3] "
                                                        "does not comply with the one defined in the mapping file [%2][%4] "
                                                        "%5 specified in %6")
                        .arg(mInputFile)
                        .arg(QString::number(sbrData.m_u2SBIN_NUM))
                        .arg(sbrData.m_cnSBIN_NAM)
                        .arg(lBinNameAndSource.first)
                        .arg(lBinSource)
                        .arg(ConverterExternalFile::GetExternalFileName(mVishayStdfRecordOverWrite.GetPathExternalFile()));
            }

            mLastError = errInvalidFormatParameter;
            return eErrBadAtdfVersion;
        }
    }


    _GATS_WRITE_RECORD(sbrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PMR record.
//				 mSourceData should point to the first character after the end of the
//				 PMR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePMR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_PMR_V4	pmrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // PMR_INDEX
            iStatus = RetrieveWord(pmrData.m_u2PMR_INDX);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            pmrData.SetPMR_INDX(pmrData.m_u2PMR_INDX);
            break;
        case 1: // CHAN_TYP
            iStatus = RetrieveWord(pmrData.m_u2CHAN_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pmrData.SetCHAN_TYP(pmrData.m_u2CHAN_TYP);
            break;
        case 2: // CHAN_NAM
            iStatus = RetrieveString(pmrData.m_cnCHAN_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pmrData.SetCHAN_NAM(pmrData.m_cnCHAN_NAM);
            break;
        case 3: // PHY_NAM
            iStatus = RetrieveString(pmrData.m_cnPHY_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pmrData.SetPHY_NAM(pmrData.m_cnPHY_NAM);
            break;
        case 4: // LOG_NAM
            iStatus = RetrieveString(pmrData.m_cnLOG_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pmrData.SetLOG_NAM(pmrData.m_cnLOG_NAM);
            break;
        case 5: // HEAD_NUM
            iStatus = RetrieveByte(pmrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pmrData.SetHEAD_NUM(pmrData.m_u1HEAD_NUM);
            break;
        case 6: // SITE_NUM
            iStatus = RetrieveByte(pmrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pmrData.SetSITE_NUM(pmrData.m_u1SITE_NUM);
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PMR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(pmrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PGR record.
//				 mSourceData should point to the first character after the end of the
//				 PGR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePGR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_PGR_V4	pgrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // GRP_INDX
            iStatus = RetrieveWord(pgrData.m_u2GRP_INDX);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            pgrData.SetGRP_INDX(pgrData.m_u2GRP_INDX);
            break;
        case 1: // GRP_NAM
            iStatus = RetrieveString(pgrData.m_cnGRP_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pgrData.SetGRP_NAM(pgrData.m_cnGRP_NAM);
            break;
        case 2: // PMR_INDX (and INDX_CNT for STDF)
            iStatus = RetrieveWordArray(pgrData.m_u2INDX_CNT,&pgrData.m_ku2PMR_INDX);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            pgrData.SetINDX_CNT(pgrData.m_u2INDX_CNT);
            pgrData.SetPMR_INDX();
            break;
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PGR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(pgrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PLR record.
//				 mSourceData should point to the first character after the end of the
//				 PLR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePLR()
{
    bool		bContinue = true;
    char		*pcTmpBuffer = NULL,*pcTmp;
    //BYTE		*pbBuffer;
    WORD		wBuffer,wLoop;
    int			iStatus;
    GQTL_STDF::Stdf_PLR_V4	plrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // GRP_INDX (and GRP_CNT for STDF file)
            iStatus = RetrieveWordArray(plrData.m_u2GRP_CNT,&plrData.m_ku2GRP_INDX);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            plrData.SetGRP_CNT(plrData.m_u2GRP_CNT);
            plrData.SetGRP_INDX();
            break;

        case 1: // GRP_MODE (optional field)
            iStatus = RetrieveWordArray(wBuffer,&plrData.m_ku2GRP_MODE);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;

            // Make sure this array has the same size as GRP_INDX
            if((wBuffer > 0) && (wBuffer != plrData.m_u2GRP_CNT))
            {
                GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),plrData.m_u2GRP_CNT,wBuffer);
                return eErrInvalidArraySize;
            }

            // If field is empty, assign default value.
            if (plrData.m_u2GRP_CNT > 0 && plrData.m_ku2GRP_MODE == NULL)
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Empty GRP_MODE field found in PLR record at line %1. Forced to 0 for each pin group.")
                      .arg(mCurrentLineNbr).toLatin1().constData());

                for (int lIdx = 0; lIdx < plrData.m_u2GRP_CNT; ++lIdx)
                    plrData.SetGRP_MODE(lIdx, 0);
            }
            else
                plrData.SetGRP_MODE();

            break;

        case 2: // GRP_RADX ( optional field)
            iStatus = RetrieveCharArray(wBuffer,&pcTmpBuffer);
            if(_GATS_ERROR_RETRIEVE(iStatus))
            {
                if(pcTmpBuffer)
                    delete [] pcTmpBuffer;
                return iStatus;
            }
            if(wBuffer > 0)
            {
                if(wBuffer != plrData.m_u2GRP_CNT)
                {
                    GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),plrData.m_u2GRP_CNT,wBuffer);
                    if(pcTmpBuffer)
                        delete [] pcTmpBuffer;
                    return eErrInvalidArraySize;
                }
                // Map ADTF value to STDF value
                pcTmp = pcTmpBuffer;
                for(wLoop=0;wLoop<wBuffer;wLoop++)
                {
                    switch(*pcTmp)
                    {
                    case 'B': plrData.SetGRP_RADX(wLoop, 2); break;
                    case 'O': plrData.SetGRP_RADX(wLoop, 8); break;
                    case 'D': plrData.SetGRP_RADX(wLoop, 10); break;
                    case 'H': plrData.SetGRP_RADX(wLoop, 16); break;
                    case 'S': plrData.SetGRP_RADX(wLoop, 20); break;
                    case '0': plrData.SetGRP_RADX(wLoop, 0); break;
                    default:
                        delete [] pcTmpBuffer;
                        GSET_ERROR2(AtdfToStdf,eErrInvalidArrayValue,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                        return eErrInvalidArrayValue;
                    }
                    pcTmp++;
                }
            }

            // If field is empty, assign default value.
            if (plrData.m_u2GRP_CNT > 0 && pcTmpBuffer == NULL)
            {
                for (int lIdx = 0; lIdx < plrData.m_u2GRP_CNT; ++lIdx)
                    plrData.SetGRP_RADX(lIdx, 0);
            }

            if(pcTmpBuffer)
            {
                delete [] pcTmpBuffer;
                pcTmpBuffer = NULL;
            }
            break;

        case 3: // PGM_CHAL and PGM_CHAR
                // ex1: HL,LL,LH/HX,XH,HL/HL,XX,XL
                // => CHAL = "HLL", "HXH", "HXX"
                // => CHAR = "LLH", "XHL", "LXL"
                // ex2: H,L,L/X,H,L/L,X,L
                // => CHAL = "", "", ""
                // => CHAR = "HLL", "XHL", "LXL"
        {
            iStatus = RetrievePLR_CHAL_CHAR_Array(&(plrData.m_kcnPGM_CHAL), &(plrData.m_kcnPGM_CHAR), plrData.m_u2GRP_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            plrData.SetPGM_CHAL();
            plrData.SetPGM_CHAR();
            break;
        }
        case 4: // RTN_CHAL and RTN_CHAR
        {
            iStatus = RetrievePLR_CHAL_CHAR_Array(&(plrData.m_kcnRTN_CHAL),&(plrData.m_kcnRTN_CHAR), plrData.m_u2GRP_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            plrData.SetRTN_CHAL();
            plrData.SetRTN_CHAR();
            break;
        }
        default:
            bContinue = false;
            break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PLR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(plrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a RDR record.
//				 mSourceData should point to the first character after the end of the
//				 PLR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeRDR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_RDR_V4	rdrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // NUM_BINS and RTST_BIN
            iStatus = RetrieveWordArray(rdrData.m_u2NUM_BINS,&rdrData.m_ku2RTST_BIN);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            rdrData.SetNUM_BINS(rdrData.m_u2NUM_BINS);
            rdrData.SetRTST_BIN();
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop RDR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(rdrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a SDR record.
//				 mSourceData should point to the first character after the end of the
//				 PLR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeSDR()
{
    bool		bContinue = true;
    WORD		wBuffer;
    int			iStatus;
    GQTL_STDF::Stdf_SDR_V4	sdrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(sdrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            sdrData.SetHEAD_NUM(sdrData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_GRP
            iStatus = RetrieveByte(sdrData.m_u1SITE_GRP);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            sdrData.SetSITE_GRP(sdrData.m_u1SITE_GRP);
            break;
        case 2: // SITE_CNT & SITE_NUM
        {
            BYTE * lSiteNum = NULL;
            iStatus = RetrieveByteArray(wBuffer, &lSiteNum);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus; // Even if the doc says it is required, found one without it!
            stdf_type_u1 lSize = static_cast<stdf_type_u1>(wBuffer);
            sdrData.SetSITE_CNT(lSize);
            for(stdf_type_u1 lIndex=0;lIndex<lSize;++lIndex)
                sdrData.SetSITE_NUM(lIndex, lSiteNum[lIndex]);
            break;
        }
        case 3: // HAND_TYP
            iStatus = RetrieveString(sdrData.m_cnHAND_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetHAND_TYP(sdrData.m_cnHAND_TYP);
            break;
        case 4: // HAND_ID
            iStatus = RetrieveString(sdrData.m_cnHAND_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetHAND_ID(sdrData.m_cnHAND_ID);
            break;
        case 5: // CARD_TYP
            iStatus = RetrieveString(sdrData.m_cnCARD_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetCARD_TYP(sdrData.m_cnCARD_TYP);
            break;
        case 6: // CARD_ID
            iStatus = RetrieveString(sdrData.m_cnCARD_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetCARD_ID(sdrData.m_cnCARD_ID);
            break;
        case 7: // LOAD_TYP
            iStatus = RetrieveString(sdrData.m_cnLOAD_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetLOAD_TYP(sdrData.m_cnLOAD_TYP);
            break;
        case 8: // LOAD_ID
            iStatus = RetrieveString(sdrData.m_cnLOAD_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetLOAD_ID(sdrData.m_cnLOAD_ID);
            break;
        case 9: // DIB_TYP
            iStatus = RetrieveString(sdrData.m_cnDIB_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetDIB_TYP(sdrData.m_cnDIB_TYP);
            break;
        case 10: // DIB_ID
            iStatus = RetrieveString(sdrData.m_cnDIB_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetDIB_ID(sdrData.m_cnDIB_ID);
            break;
        case 11: // CABL_TYP
            iStatus = RetrieveString(sdrData.m_cnCABL_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetCABL_TYP(sdrData.m_cnCABL_TYP);
            break;
        case 12: // CABL_ID
            iStatus = RetrieveString(sdrData.m_cnCABL_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetCABL_ID(sdrData.m_cnCABL_ID);
            break;
        case 13: // CONT_TYP
            iStatus = RetrieveString(sdrData.m_cnCONT_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetCONT_TYP(sdrData.m_cnCONT_TYP);
            break;
        case 14: // CONT_ID
            iStatus = RetrieveString(sdrData.m_cnCONT_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetCONT_ID(sdrData.m_cnCONT_ID);
            break;
        case 15: // LASR_TYP
            iStatus = RetrieveString(sdrData.m_cnLASR_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetLASR_TYP(sdrData.m_cnLASR_TYP);
            break;
        case 16: // LASR_ID
            iStatus = RetrieveString(sdrData.m_cnLASR_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetLASR_ID(sdrData.m_cnLASR_ID);
            break;
        case 17: // EXTR_TYP
            iStatus = RetrieveString(sdrData.m_cnEXTR_TYP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetEXTR_TYP(sdrData.m_cnEXTR_TYP);
            break;
        case 18: // EXTR_ID
            iStatus = RetrieveString(sdrData.m_cnEXTR_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            sdrData.SetEXTR_ID(sdrData.m_cnEXTR_ID);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop SDR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(sdrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a WIR record.
//				 mSourceData should point to the first character after the end of the
//				 WIR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeWIR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_WIR_V4	wirData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(wirData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            wirData.SetHEAD_NUM(wirData.m_u1HEAD_NUM);
            break;
        case 1: // START_T
        {
            DWORD lModTime;
            iStatus = RetrieveDate(lModTime);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            wirData.SetSTART_T((time_t)lModTime);
            break;
        }
        case 2: // SITE_GRP
            iStatus = RetrieveByte(wirData.m_u1SITE_GRP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wirData.SetSITE_GRP(wirData.m_u1SITE_GRP);
            break;
        case 3: // WAFER_ID
            iStatus = RetrieveString(wirData.m_cnWAFER_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wirData.SetWAFER_ID(wirData.m_cnWAFER_ID);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop WIR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(wirData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a WRR record.
//				 mSourceData should point to the first character after the end of the
//				 WRR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeWRR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_WRR_V4	wrrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(wrrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            wrrData.SetHEAD_NUM(wrrData.m_u1HEAD_NUM);
            break;
        case 1: // FINISH_T
        {
            DWORD lModTime;
            iStatus = RetrieveDate(lModTime);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            wrrData.SetFINISH_T((time_t)lModTime);
            break;
        }
        case 2: // PART_CNT
            iStatus = RetrieveDWord(wrrData.m_u4PART_CNT);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            wrrData.SetPART_CNT(wrrData.m_u4PART_CNT);
            break;
        case 3: // WAFER_ID
            iStatus = RetrieveString(wrrData.m_cnWAFER_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetWAFER_ID(wrrData.m_cnWAFER_ID);
            break;
        case 4: // SITE_GRP
            iStatus = RetrieveByte(wrrData.m_u1SITE_GRP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetSITE_GRP(wrrData.m_u1SITE_GRP);
            break;
        case 5: // RTST_CNT
            iStatus = RetrieveDWord(wrrData.m_u4RTST_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetRTST_CNT(wrrData.m_u4RTST_CNT);
            break;
        case 6: // ABRT_CNT
            iStatus = RetrieveDWord(wrrData.m_u4ABRT_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetABRT_CNT(wrrData.m_u4ABRT_CNT);
            break;
        case 7: // GOOD_CNT
            iStatus = RetrieveDWord(wrrData.m_u4GOOD_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetGOOD_CNT(wrrData.m_u4GOOD_CNT);
            break;
        case 8:	// FUNC_CNT
            iStatus = RetrieveDWord(wrrData.m_u4FUNC_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetFUNC_CNT(wrrData.m_u4FUNC_CNT);
            break;
        case 9: // FABWF_ID
            iStatus = RetrieveString(wrrData.m_cnFABWF_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetFABWF_ID(wrrData.m_cnFABWF_ID);
            break;
        case 10: // FRAME_ID
            iStatus = RetrieveString(wrrData.m_cnFRAME_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetFRAME_ID(wrrData.m_cnFRAME_ID);
            break;
        case 11: // MASK_ID
            iStatus = RetrieveString(wrrData.m_cnMASK_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetMASK_ID(wrrData.m_cnMASK_ID);
            break;
        case 12: // USR_DESC
            iStatus = RetrieveString(wrrData.m_cnUSR_DESC);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetUSR_DESC(wrrData.m_cnUSR_DESC);
            break;
        case 13: // EXC_DESC
            iStatus = RetrieveString(wrrData.m_cnEXC_DESC);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wrrData.SetEXC_DESC(wrrData.m_cnEXC_DESC);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop WRR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(wrrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a WCR record.
//				 mSourceData should point to the first character after the end of the
//				 WCR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeWCR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_WCR_V4	wcrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // WF_FLAT
            iStatus = RetrieveChar(wcrData.m_c1WF_FLAT,false,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetWF_FLAT(wcrData.m_c1WF_FLAT);
            break;
        case 1: // POS_X
            iStatus = RetrieveChar(wcrData.m_c1POS_X,false,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetPOS_X(wcrData.m_c1POS_X);
            break;
        case 2: // POS_Y
            iStatus = RetrieveChar(wcrData.m_c1POS_Y,false,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetPOS_Y(wcrData.m_c1POS_Y);
            break;
        case 3: // WAFR_SIZ
            iStatus = RetrieveFloat(wcrData.m_r4WAFR_SIZ);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetWAFR_SIZ(wcrData.m_r4WAFR_SIZ);
            break;
        case 4: // DIE_HT
            iStatus = RetrieveFloat(wcrData.m_r4DIE_HT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetDIE_HT(wcrData.m_r4DIE_HT);
            break;
        case 5: // DIE_WID
            iStatus = RetrieveFloat(wcrData.m_r4DIE_WID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetDIE_WID(wcrData.m_r4DIE_WID);
            break;
        case 6: // WF_UNITS
            iStatus = RetrieveByte(wcrData.m_u1WF_UNITS);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetWF_UNITS(wcrData.m_u1WF_UNITS);
            break;
        case 7: // CENTER_X
            iStatus = RetrieveShort(wcrData.m_i2CENTER_X);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetCENTER_X(wcrData.m_i2CENTER_X);
            break;
        case 8: // CENTER_Y
            iStatus = RetrieveShort(wcrData.m_i2CENTER_Y);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            wcrData.SetCENTER_Y(wcrData.m_i2CENTER_Y);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop WCR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(wcrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PIR record.
//				 mSourceData should point to the first character after the end of the
//				 PIR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePIR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_PIR_V4	pirData;
    int			iCurrentField = 0;

    // reset fail test
    mFailedTest = -1;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(pirData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            pirData.SetHEAD_NUM(pirData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_NUM
            iStatus = RetrieveByte(pirData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            pirData.SetSITE_NUM(pirData.m_u1SITE_NUM);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PIR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(pirData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PRR record.
//				 mSourceData should point to the first character after the end of the
//				 PRR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePRR()
{
    bool		bContinue = true;
    char		cBuffer;
    int			iStatus;
    GQTL_STDF::Stdf_PRR_V4	prrData;
    int			iCurrentField = 0;
    QString     lErrorMsg;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(prrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            prrData.SetHEAD_NUM(prrData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_NUM
            iStatus = RetrieveByte(prrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            prrData.SetSITE_NUM(prrData.m_u1SITE_NUM);
            break;
        case 2: // PART_ID
            iStatus = RetrieveString(prrData.m_cnPART_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetPART_ID(prrData.m_cnPART_ID);

            // Set all other mandatory fields
            prrData.SetSOFT_BIN(65535);
            prrData.SetX_COORD(-32768);
            prrData.SetY_COORD(-32768);
            prrData.SetTEST_T(0);
            break;
        case 3: // NUM_TEST
            iStatus = RetrieveWord(prrData.m_u2NUM_TEST);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            prrData.SetNUM_TEST(prrData.m_u2NUM_TEST);
            break;
        case 4: // Pass/Fail Code -> PART_FLG
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;

            if(cBuffer == 'F')
                prrData.m_b1PART_FLG |= GQTL_STDF::Stdf_PRR_V4::ePART_FAILED;
            else
            if(cBuffer == 'P')
                prrData.m_b1PART_FLG &= ~GQTL_STDF::Stdf_PRR_V4::ePART_FAILED;
            else
            {
                GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrNotValidFlag;
            }
            prrData.SetPART_FLG(prrData.m_b1PART_FLG);
            break;
        case 5: // HARD_BIN
        {
            iStatus = RetrieveWord(prrData.m_u2HARD_BIN);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            prrData.SetHARD_BIN(prrData.m_u2HARD_BIN);

            break;
        }
        case 6: // SOFT_BIN
            iStatus = RetrieveWord(prrData.m_u2SOFT_BIN);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetSOFT_BIN(prrData.m_u2SOFT_BIN);
            break;
        case 7: // X_COORD
            iStatus = RetrieveShort(prrData.m_i2X_COORD);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetX_COORD(prrData.m_i2X_COORD);
            break;
        case 8: // Y_COORD
            iStatus = RetrieveShort(prrData.m_i2Y_COORD);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetY_COORD(prrData.m_i2Y_COORD);
            break;
        case 9: // Retest Code -> PART_FLG
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;

            if(cBuffer == 'I')
            {
                prrData.m_b1PART_FLG &= ~GQTL_STDF::Stdf_PRR_V4::eREPEAT_COORD;
                prrData.m_b1PART_FLG |= GQTL_STDF::Stdf_PRR_V4::eREPEAT_PARTID;
            }
            else
            if(cBuffer == 'C')
            {
                prrData.m_b1PART_FLG &= ~GQTL_STDF::Stdf_PRR_V4::eREPEAT_PARTID;
                prrData.m_b1PART_FLG |= GQTL_STDF::Stdf_PRR_V4::eREPEAT_COORD;
            }
            prrData.SetPART_FLG( prrData.m_b1PART_FLG);
            break;
        case 10: // Abort Code -> PART_FLG
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;

            if(cBuffer == 'Y')
                prrData.m_b1PART_FLG |= GQTL_STDF::Stdf_PRR_V4::eABNORMAL_END;
            else
                prrData.m_b1PART_FLG &= ~GQTL_STDF::Stdf_PRR_V4::eABNORMAL_END;
            prrData.SetPART_FLG( prrData.m_b1PART_FLG);
            break;
        case 11: // TEST_T
            iStatus = RetrieveDWord(prrData.m_u4TEST_T);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetTEST_T(prrData.m_u4TEST_T);
            break;
        case 12: // PART_TXT
            iStatus = RetrieveString(prrData.m_cnPART_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetPART_TXT(prrData.m_cnPART_TXT);
            break;
        case 13: // PART_FIX
        {
            iStatus = RetrieveHexaString(prrData.m_bnPART_FIX);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            prrData.SetPART_FIX();
            break;
        }
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PRR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    // If no SBIN defined, use SBIN=HBIN
    if(prrData.m_u2SOFT_BIN == 0xFFFF)
        prrData.m_u2SOFT_BIN = prrData.m_u2HARD_BIN;

//    if(mUseExternalFile)
//    {
//        // if PRR valid and not wafer record found before indicate that this is final test file
//       if(prrData.m_u2HARD_BIN != STDF_MAX_U2 && prrData.m_u2SOFT_BIN != STDF_MAX_U2 && mWaferFound == false)
//       {
//           mTypeFile = VishayStdfRecordOverWrite::finalTestType;
//       }

//       mVishayStdfRecordOverWrite.UpdatePRRRecordWithBinMapping(prrData, mFailedTest);
//    }

    _GATS_WRITE_RECORD(prrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a TSR record.
//				 mSourceData should point to the first character after the end of the
//				 TSR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeTSR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_TSR_V4	tsrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // HEAD_NUM
            iStatus = RetrieveByte(tsrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetHEAD_NUM(tsrData.m_u1HEAD_NUM);
            break;
        case 1: // SITE_NUM
            iStatus = RetrieveByte(tsrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetSITE_NUM(tsrData.m_u1SITE_NUM);
            break;
        case 2: // TEST_NUM
            iStatus = RetrieveDWord(tsrData.m_u4TEST_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            tsrData.SetTEST_NUM(tsrData.m_u4TEST_NUM);
            break;
        case 3: // TEST_NAM
            iStatus = RetrieveString(tsrData.m_cnTEST_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetTEST_NAM(tsrData.m_cnTEST_NAM);
            break;
        case 4: // TEST_TYP
            iStatus = RetrieveChar(tsrData.m_c1TEST_TYP,false);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;

            if(tsrData.m_c1TEST_TYP != 'P' && tsrData.m_c1TEST_TYP != 'F' &&
               tsrData.m_c1TEST_TYP != 'M' && tsrData.m_c1TEST_TYP != ' '
                     && tsrData.m_c1TEST_TYP != '\0')
            {
                tsrData.m_c1TEST_TYP = ' ';
//                GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
//                return eErrNotValidFlag;
            }
            tsrData.SetTEST_TYP(tsrData.m_c1TEST_TYP);
            break;
        case 5: // EXEC_CNT
            iStatus = RetrieveDWord(tsrData.m_u4EXEC_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetEXEC_CNT(tsrData.m_u4EXEC_CNT);
            break;
        case 6: // FAIL_CNT
            iStatus = RetrieveDWord(tsrData.m_u4FAIL_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetFAIL_CNT(tsrData.m_u4FAIL_CNT);
            break;
        case 7: // ALRM_CNT
            iStatus = RetrieveDWord(tsrData.m_u4ALRM_CNT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetALRM_CNT(tsrData.m_u4ALRM_CNT);
            break;
        case 8: // SEQ_NAME
            iStatus = RetrieveString(tsrData.m_cnSEQ_NAME);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetSEQ_NAME(tsrData.m_cnSEQ_NAME);
            break;
        case 9: // TEST_LBL
            iStatus = RetrieveString(tsrData.m_cnTEST_LBL);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            tsrData.SetTEST_LBL(tsrData.m_cnTEST_LBL);
            break;
        case 10: // TEST_TIM
            iStatus = RetrieveFloat(tsrData.m_r4TEST_TIM);
            if(iStatus == eErrOkay)
                tsrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TEST_TIM;
            else
            if(iStatus == eErrEmpty)
                tsrData.m_b1OPT_FLAG |= GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TEST_TIM;
            else
            if(iStatus != eErrOkay)	return iStatus;
            tsrData.SetTEST_TIM(tsrData.m_r4TEST_TIM);
            tsrData.SetOPT_FLAG(tsrData.m_b1OPT_FLAG);
            break;
        case 11: // TEST_MIN
            iStatus = RetrieveFloat(tsrData.m_r4TEST_MIN);
            if(iStatus == eErrOkay)
                tsrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TEST_MIN;
            else
            if(iStatus == eErrEmpty)
                tsrData.m_b1OPT_FLAG |= GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TEST_MIN;
            else
                return iStatus;
            tsrData.SetOPT_FLAG(tsrData.m_b1OPT_FLAG);
            tsrData.SetTEST_MIN(tsrData.m_r4TEST_MIN);
            break;
        case 12: // TEST_MAX
            iStatus = RetrieveFloat(tsrData.m_r4TEST_MAX);
            if(iStatus == eErrOkay)
                tsrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TEST_MAX;
            else
            if(iStatus == eErrEmpty)
                tsrData.m_b1OPT_FLAG |= GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TEST_MAX;
            else
                return iStatus;
            tsrData.SetOPT_FLAG(tsrData.m_b1OPT_FLAG);
            tsrData.SetTEST_MAX(tsrData.m_r4TEST_MAX);
                    break;
        case 13: // TST_SUMS
            iStatus = RetrieveFloat(tsrData.m_r4TST_SUMS);
            if(iStatus == eErrOkay)
                tsrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TST_SUMS;
            else
            if(iStatus == eErrEmpty)
                tsrData.m_b1OPT_FLAG |= GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TST_SUMS;
            else
                return iStatus;
            tsrData.SetOPT_FLAG(tsrData.m_b1OPT_FLAG);
            tsrData.SetTST_SUMS(tsrData.m_r4TST_SUMS);
            break;
        case 14: // TST_SQRS
            iStatus = RetrieveFloat(tsrData.m_r4TST_SQRS);
            if(iStatus == eErrOkay)
                tsrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TST_SQRS;
            else
            if(iStatus == eErrEmpty)
                tsrData.m_b1OPT_FLAG |= GQTL_STDF::Stdf_TSR_V4::eNOTVALID_TST_SQRS;
            else
                return iStatus;
            tsrData.SetOPT_FLAG(tsrData.m_b1OPT_FLAG);
            tsrData.SetTST_SQRS(tsrData.m_r4TST_SQRS);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop TSR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(tsrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a PTR record.
//				 mSourceData should point to the first character after the end of the
//				 PTR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzePTR()
{
    bool		bContinue = true;
    char		cBuffer,*pszBuffer,lBuffer[256];
    SHORT		sBuffer;
    int			iStatus;
    int			iCurrentField = 0;

    mData->Reset();

    mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eHIGHLIM_NOTSTRICT;
    mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eLOWLIM_NOTSTRICT;

    do
    {
        switch(iCurrentField)
        {
        case 0: // TEST_NUM
            iStatus = RetrieveDWord(mData->m_u4TEST_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mData->SetTEST_NUM(mData->m_u4TEST_NUM);
            break;
        case 1: // HEAD_NUM
            iStatus = RetrieveByte(mData->m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mData->SetHEAD_NUM(mData->m_u1HEAD_NUM);
            break;
        case 2: // SITE_NUM
            iStatus = RetrieveByte(mData->m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mData->SetSITE_NUM(mData->m_u1SITE_NUM);
            break;
        case 3: // RESULT
            iStatus = RetrieveFloat(mData->m_r4RESULT);
            if(iStatus == eErrOkay)
                mData->m_b1TEST_FLG &= ~GQTL_STDF::Stdf_PTR_V4::eNOTVALID_RESULT;
            else
            if(iStatus == eErrEmpty)
                mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eNOTVALID_RESULT;
            else
                return iStatus;
            mData->SetRESULT(mData->m_r4RESULT);
            break;
        case 4: // Pass/Fail Flag
            iStatus = RetrieveChar(cBuffer,false);
            if(iStatus == eErrOkay)
            {
                switch(cBuffer)
                {
                case 'A':	mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::ePASS_ALTERNATE_LIM;	break;
                case 'F':	{
                    mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eTEST_FAILED;
                    mFailedTest = mData->m_u4TEST_NUM;
                    break;
                }
                case 'P':	break; // No flag to set
                case ' ':	mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eNO_PASSFAIL; break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
            }
            else
            {
                if(iStatus == eErrEmpty)
                    mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eNO_PASSFAIL;
                else
                    return iStatus;
            }
            mData->SetTEST_FLG(mData->m_b1TEST_FLG);
            mData->SetPARM_FLG(mData->m_b1PARM_FLG);
            break;
        case 5: // Alarm Flags
            iStatus = RetrieveChar(lBuffer, 256);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            pszBuffer = lBuffer;
            while(*pszBuffer != '\0')
            {
                switch(*pszBuffer)
                {
                case 'A':	mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eALARM;	break;
                case 'D':	mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eDRIFT_ERROR;	break;
                case 'H':	mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eOVER_HIGHLIMIT;	break;
                case 'L':	mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eUNDER_LOWLIMIT;	break;
                case 'N':	mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eTEST_NOTEXECUTED;	break;
                case 'O':	mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eOSCILLAT_DETECTED;	break;
                case 'S':	mData->m_b1PARM_FLG |= GQTL_STDF::Stdf_PTR_V4::eSCALE_ERROR;	break;
                case 'T':	mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eTIMEOUT;	break;
                case 'U':	mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eNOTRELIABLE_RESULT;	break;
                case 'X':	mData->m_b1TEST_FLG |= GQTL_STDF::Stdf_PTR_V4::eTEST_ABORTED;	break;
                case ' ':	break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
                pszBuffer++;
            }
            mData->SetTEST_FLG(mData->m_b1TEST_FLG);
            mData->SetPARM_FLG(mData->m_b1PARM_FLG);
            break;
        case 6: // TEST_TXT
            iStatus = RetrieveString(mData->m_cnTEST_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mData->SetTEST_TXT(mData->m_cnTEST_TXT);
            break;
        case 7: // ALARM_ID
            iStatus = RetrieveString(mData->m_cnALARM_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mData->SetALARM_ID(mData->m_cnALARM_ID);
            break;
        case 8: // Limit Compare
            iStatus = RetrieveChar(lBuffer, 256);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            pszBuffer = lBuffer;
            while(*pszBuffer != '\0')
            {
                switch(*pszBuffer)
                {
                case 'H':	mData->m_b1PARM_FLG &= ~GQTL_STDF::Stdf_PTR_V4::eHIGHLIM_NOTSTRICT;	break;
                case 'L':	mData->m_b1PARM_FLG &= ~GQTL_STDF::Stdf_PTR_V4::eLOWLIM_NOTSTRICT;     break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
                pszBuffer++;
            }
            mData->SetPARM_FLG(mData->m_b1PARM_FLG);
            break;
        case 9: // UNITS
            iStatus = RetrieveString(mData->m_cnUNITS);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mData->SetUNITS(mData->m_cnUNITS);
            break;
        case 10: // LO_LIMITS
            iStatus = RetrieveFloat(mData->m_r4LO_LIMIT);
            if(iStatus == eErrOkay)
            {
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LL;
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LL_USEFIRSTPTR;
                // If this test number has not been already inserted in the test LL map
                if(mMapPTR_LL.find(mData->m_u4TEST_NUM) == mMapPTR_LL.end())
                    mMapPTR_LL.insert(MapTests::value_type(mData->m_u4TEST_NUM,mData->m_r4LO_LIMIT));
            }
            else // No limit specified in this PTR
            if(iStatus == eErrEmpty)
            {
                MapTests::iterator	it;
                // A low limit for this test has been found in a previous PTR?
                it = mMapPTR_LL.find(mData->m_u4TEST_NUM);
                // If not,
                if(it == mMapPTR_LL.end() )
                    // That's mean that the test has no LL
                    mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LL_USEFIRSTPTR;
                //else
                    // The test has a low limit, but we need to use the first PTR found to retrieve it
                //	m_pptrData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LL;
            }
            else
                return iStatus;
            mData->SetLO_LIMIT(mData->m_r4LO_LIMIT);
            break;
        case 11: // HI_LIMITS
            iStatus = RetrieveFloat(mData->m_r4HI_LIMIT);
            if(iStatus == eErrOkay)
            {
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HL;
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HL_USEFIRSTPTR;
                // If this test number has not been already inserted in the test HL map
                if(mMapPTR_HL.find(mData->m_u4TEST_NUM) == mMapPTR_HL.end())
                    mMapPTR_HL.insert(MapTests::value_type(mData->m_u4TEST_NUM,mData->m_r4HI_LIMIT));
            }
            else // No limit specified in this PTR
            if(iStatus == eErrEmpty)
            {
                MapTests::iterator	it;
                // A high limit for this test has been found in a previous PTR?
                it = mMapPTR_HL.find(mData->m_u4TEST_NUM);
                // If not,
                if(it == mMapPTR_HL.end() )
                    // That's mean that the test has no HL
                    mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HL_USEFIRSTPTR;
                //else
                    // The test has a low limit, but we need to use the first PTR found to retrieve it
                //	m_pptrData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HL;
            }
            else
                return iStatus;
            mData->SetHI_LIMIT(mData->m_r4HI_LIMIT);
            break;
        case 12: // C_RESFMT
            iStatus = RetrieveString(mData->m_cnC_RESFMT);
            // Notes: On real STDF, a space seems to be add at the end of this string (give 3bytes length difference in the record)
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mData->SetC_RESFMT(mData->m_cnC_RESFMT);
            break;
        case 13: // C_LLMFMT
            iStatus = RetrieveString(mData->m_cnC_LLMFMT);
            // Notes: On real STDF, a space seems to be add at the end of this string (give 3bytes length difference in the record)
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mData->SetC_LLMFMT(mData->m_cnC_LLMFMT);
            break;
        case 14: // C_HLMFMT
            iStatus = RetrieveString(mData->m_cnC_HLMFMT);
            // Notes: On real STDF, a space seems to be add at the end of this string (give 3bytes length difference in the record)
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mData->SetC_HLMFMT(mData->m_cnC_HLMFMT);
            break;
        case 15: // LO_SPEC
            iStatus = RetrieveFloat(mData->m_r4LO_SPEC);
            if(iStatus == eErrOkay)
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LOWSPEC_LIMIT;
            else
            if(iStatus != eErrEmpty)	return iStatus;
            mData->SetOPT_FLAG(mData->m_b1OPT_FLAG);
            mData->SetLO_SPEC(mData->m_r4LO_SPEC);
            break;
        case 16: // HI_SPEC
            iStatus = RetrieveFloat(mData->m_r4HI_SPEC);
            if(iStatus == eErrOkay)
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HIGHSPEC_LIMIT;
            else
            if(iStatus != eErrEmpty)	return iStatus;
            mData->SetOPT_FLAG(mData->m_b1OPT_FLAG);
            mData->SetHI_SPEC(mData->m_r4HI_SPEC);
            break;
        case 17: // RES_SCAL
            iStatus = RetrieveShort(sBuffer);
            if(iStatus == eErrOkay)
            {
                mData->m_i1RES_SCAL = static_cast<char>(sBuffer);
                mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNOTVALID_RES_SCAL;
            }
            else
            if(iStatus != eErrEmpty)	return iStatus;
            mData->SetRES_SCAL(mData->m_i1RES_SCAL);
            mData->SetOPT_FLAG(mData->m_b1OPT_FLAG);
            break;
        case 18: // LLM_SCAL
            iStatus = RetrieveShort(sBuffer);
            if(iStatus == eErrOkay)
                mData->m_i1LLM_SCAL = static_cast<char>(sBuffer);
            else
            if(iStatus != eErrEmpty)	return iStatus;
            mData->SetLLM_SCAL(mData->m_i1LLM_SCAL);
            break;
        case 19: // HLM_SCAL
            iStatus = RetrieveShort(sBuffer);
            if(iStatus == eErrOkay)
                mData->m_i1HLM_SCAL = static_cast<char>(sBuffer);
            else
            if(iStatus != eErrEmpty)	return iStatus;
            mData->SetHLM_SCAL(mData->m_i1HLM_SCAL);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop PTR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    // Adjust Low Limit flags if necessary
    if( (mData->m_b1OPT_FLAG & GQTL_STDF::Stdf_PTR_V4::eNO_LL) &&
        (mData->m_b1OPT_FLAG & GQTL_STDF::Stdf_PTR_V4::eNO_LL_USEFIRSTPTR) )
    {
        MapTests::iterator	it;
        // A low limit for this test has been found in a previous PTR?
        it = mMapPTR_LL.find(mData->m_u4TEST_NUM);

        // If not,
        if(it == mMapPTR_LL.end() )
            // That's mean that the test has no LL
            mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LL_USEFIRSTPTR;
        //else
            // The test has a low limit, but we need to use the first PTR found to retrieve it
        //	m_pptrData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_LL;
    }

    // Adjust High Limit flags if necessary
    if( (mData->m_b1OPT_FLAG & GQTL_STDF::Stdf_PTR_V4::eNO_HL) &&
        (mData->m_b1OPT_FLAG & GQTL_STDF::Stdf_PTR_V4::eNO_HL_USEFIRSTPTR) )
    {
        MapTests::iterator	it;
        // A high limit for this test has been found in a previous PTR?
        it = mMapPTR_HL.find(mData->m_u4TEST_NUM);
        // If not,
        if(it == mMapPTR_HL.end() )
            // That's mean that the test has no HL
            mData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HL_USEFIRSTPTR;
        //else
            // The test has a high limit, but we need to use the first PTR found to retrieve it
        //	m_pptrData->m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_PTR_V4::eNO_HL;
    }

    // Need to scale values?
    if(mScaledData == false)
    {
        // Need to scale the RESULT field?
        if(!(mData->m_b1TEST_FLG & GQTL_STDF::Stdf_PTR_V4::eNOTVALID_RESULT))
            ScalValue(mData->m_r4RESULT, mData->m_cnUNITS, mData->m_i1RES_SCAL); // UNITS not modified for the moment

        ScalValue(mData->m_r4LO_LIMIT, mData->m_cnUNITS, mData->m_i1LLM_SCAL); // UNITS not modified for the moment
        ScalValue(mData->m_r4HI_LIMIT, mData->m_cnUNITS, mData->m_i1HLM_SCAL,false); // UNITS modified
    }

    _GATS_WRITE_RECORD((*mData),mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a MPR record.
//				 mSourceData should point to the first character after the end of the
//				 MPR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeMPR()
{
    bool		bContinue = true;
    char		cBuffer,*pszBuffer, lBuffer[256];
    WORD		wBuffer;
    SHORT		sBuffer;
    int			iStatus;
    GQTL_STDF::Stdf_MPR_V4	mprData;
    int			iCurrentField = 0,iMaxFieldIDToWrite = -1;
    bool		bRTN_INDX_Corrupted2Check = true;		// to check coorupted RTN_INDX field compared to optionnal

    mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eHIGHLIM_NOTSTRICT;
    mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eLOWLIM_NOTSTRICT;

    do
    {
        switch(iCurrentField)
        {
        case 0: // TEST_NUM
            iStatus = RetrieveDWord(mprData.m_u4TEST_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mprData.SetTEST_NUM(mprData.m_u4TEST_NUM);
            break;
        case 1: // HEAD_NUM
            iStatus = RetrieveByte(mprData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mprData.SetHEAD_NUM(mprData.m_u1HEAD_NUM);
            break;
        case 2: // SITE_NUM
            iStatus = RetrieveByte(mprData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            mprData.SetSITE_NUM(mprData.m_u1SITE_NUM);
            break;
        case 3: // RTN_STAT
            iStatus = RetrieveNibbleArray(mprData.m_u2RTN_ICNT, &mprData.m_kn1RTN_STAT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetRTN_ICNT(mprData.m_u2RTN_ICNT);
            for (stdf_type_u2 index=0; index<mprData.m_u2RTN_ICNT; ++index)
                mprData.SetRTN_STAT(index, mprData.m_kn1RTN_STAT[index]);
            break;
        case 4: // RTN_RSLT
            iStatus = RetrieveFloatArray(mprData.m_u2RSLT_CNT, &mprData.m_kr4RTN_RSLT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetRSLT_CNT(mprData.m_u2RSLT_CNT);
            for (stdf_type_u2 index=0; index<mprData.m_u2RSLT_CNT; ++index)
                mprData.SetRTN_RSLT(index, mprData.m_kr4RTN_RSLT[index]);
            break;
        case 5: // Pass/Fail Flag
            iStatus = RetrieveChar(cBuffer,false);
            if(iStatus == eErrOkay)
            {
                switch(cBuffer)
                {
                case 'A':	mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::ePASS_ALTERNATE_LIM;	break;
                case 'F':
                {
                    mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eTEST_FAILED;
                    mFailedTest = mprData.m_u4TEST_NUM;
                    break;
                }
                case 'P':	break; // No flag to set
                case ' ':	mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eNO_PASSFAIL; break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
            }
            else
            if(iStatus == eErrEmpty)
                mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eNO_PASSFAIL;
            else
                return iStatus;
            mprData.SetTEST_FLG(mprData.m_b1TEST_FLG);
            break;
        case 6: // Alarm Flags
            iStatus = RetrieveChar(lBuffer, 256);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            pszBuffer = lBuffer;
            while(*pszBuffer != '\0')
            {
                switch(*pszBuffer)
                {
                case 'A':	mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eALARM;	break;
                case 'D':	mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eDRIFT_ERROR;	break;
                case 'H':	mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eOVER_HIGHLIMIT;	break;
                case 'L':	mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eUNDER_LOWLIMIT;	break;
                case 'N':	mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eTEST_NOTEXECUTED;	break;
                case 'O':	mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eOSCILLAT_DETECTED;	break;
                case 'S':	mprData.m_b1PARM_FLG |= GQTL_STDF::Stdf_MPR_V4::eSCALE_ERROR;	break;
                case 'T':	mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eTIMEOUT;	break;
                case 'U':	mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eNOTRELIABLE_RESULT;	break;
                case 'X':	mprData.m_b1TEST_FLG |= GQTL_STDF::Stdf_MPR_V4::eTEST_ABORTED;	break;
                case ' ':	break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
                pszBuffer++;
            }
            mprData.SetTEST_FLG(mprData.m_b1TEST_FLG);
            mprData.SetPARM_FLG(mprData.m_b1PARM_FLG);
            break;
        case 7: // TEST_TXT
            iStatus = RetrieveString(mprData.m_cnTEST_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetTEST_TXT(mprData.m_cnTEST_TXT);
            break;
        case 8: // ALARM_ID
            iStatus = RetrieveString(mprData.m_cnALARM_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetALARM_ID(mprData.m_cnALARM_ID);
            break;
        case 9: // Limit Compare
            iStatus = RetrieveChar(lBuffer, 256);// Maximum of 2 characters in this field
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            pszBuffer = lBuffer;
            while(*pszBuffer != '\0')
            {
                switch(*pszBuffer)
                {
                case 'H':	mprData.m_b1PARM_FLG &= ~GQTL_STDF::Stdf_MPR_V4::eHIGHLIM_NOTSTRICT;	break;
                case 'L':	mprData.m_b1PARM_FLG &= ~GQTL_STDF::Stdf_MPR_V4::eLOWLIM_NOTSTRICT;     break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
                pszBuffer++;
            }
            mprData.SetPARM_FLG(mprData.m_b1PARM_FLG);
            break;
        case 10: // UNITS
            iStatus = RetrieveString(mprData.m_cnUNITS);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetUNITS(mprData.m_cnUNITS);
            break;
        case 11: // LO_LIMIT
            iStatus = RetrieveFloat(mprData.m_r4LO_LIMIT);
            if(iStatus == eErrOkay)
            {
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL;
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL_USEFIRSTMPR;
                // If this test number has not been already inserted in the test LL map
                if(mMapMPR_LL.find(mprData.m_u4TEST_NUM) == mMapMPR_LL.end())
                    mMapMPR_LL.insert(MapTests::value_type(mprData.m_u4TEST_NUM,1.f));
            }
            else // No limit specified in this MPR
            if(iStatus == eErrEmpty)
            {
                MapTests::iterator	it;
                // A low limit for this test has been found in a previous MPR?
                it = mMapMPR_LL.find(mprData.m_u4TEST_NUM);
                // If not,
                if(it == mMapMPR_LL.end() )
                    // That's mean that the test has no LL
                    mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL_USEFIRSTMPR;
                else
                    // The test has a low limit, but we need to use the first MPR found to retrieve it
                    mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL;
            }
            else
                return iStatus;
            mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);
            mprData.SetLO_LIMIT(mprData.m_r4LO_LIMIT);
            mprData.SetTEST_NUM(mprData.m_u4TEST_NUM);
            break;
        case 12: // HI_LIMIT
            iStatus = RetrieveFloat(mprData.m_r4HI_LIMIT);
            if(iStatus == eErrOkay)
            {
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL;
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL_USEFIRSTMPR;
                // If this test number has not been already inserted in the test LL map
                if(mMapMPR_HL.find(mprData.m_u4TEST_NUM) == mMapMPR_HL.end())
                    mMapMPR_HL.insert(MapTests::value_type(mprData.m_u4TEST_NUM,1.f));
            }
            else // No limit specified in this MPR
            if(iStatus == eErrEmpty)
            {
                MapTests::iterator	it;
                // A high limit for this test has been found in a previous MPR?
                it = mMapMPR_HL.find(mprData.m_u4TEST_NUM);
                // If not,
                if(it == mMapMPR_HL.end() )
                    // That's mean that the test has no HL
                    mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL_USEFIRSTMPR;
                else
                    // The test has a high limit, but we need to use the first MPR found to retrieve it
                    mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL;
            }
            else
                return iStatus;
            mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);
            mprData.SetHI_LIMIT(mprData.m_r4HI_LIMIT);
            mprData.SetTEST_NUM(mprData.m_u4TEST_NUM);
            break;
        case 13: // START_IN
            iStatus = RetrieveFloat(mprData.m_r4START_IN);
            if(iStatus == eErrOkay)
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNOTVALID_START_INCR;
            else
            if(iStatus != eErrEmpty)	return iStatus;
            mprData.SetSTART_IN(mprData.m_r4START_IN);
            break;
        case 14: // INCR_IN
            iStatus = RetrieveFloat(mprData.m_r4INCR_IN);
            if(iStatus == eErrOkay)
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNOTVALID_START_INCR;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetINCR_IN(mprData.m_r4INCR_IN);
            break;
        case 15: // UNITS_IN
            iStatus = RetrieveString(mprData.m_cnUNITS_IN);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetUNITS_IN(mprData.m_cnUNITS_IN);
            break;

        case 16: // RTN_INDEX
            iStatus = RetrieveWordArray(wBuffer,&mprData.m_ku2RTN_INDX);
            if(iStatus != eErrOkay)
                return iStatus;

            if(wBuffer == mprData.m_u2RTN_ICNT)
                bRTN_INDX_Corrupted2Check = false;
            // if RTN_INDX == NULL && card(RTN_INDX) != RTN_ICNT, check at the end if it isn't an optionnal field

            if( (iStatus == eErrOkay) && (!bRTN_INDX_Corrupted2Check) )
                mprData.SetRTN_ICNT(mprData.m_u2RTN_ICNT);
            else if( (iStatus != eErrEmpty) && (bRTN_INDX_Corrupted2Check) ) // iStatus == eErrempty <=> wBuffer = 0
            {
                GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(mprData.m_u2RTN_ICNT),int(wBuffer));
                return eErrInvalidArraySize;
            }
            else if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetRTN_ICNT(mprData.m_u2RTN_ICNT);
            for (stdf_type_u2 index=0; index<mprData.m_u2RTN_ICNT; ++index)
                mprData.SetRTN_INDX(index, mprData.m_ku2RTN_INDX[index]);
            break;
        case 17: // C_RESFMT
            iStatus = RetrieveString(mprData.m_cnC_RESFMT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetC_RESFMT(mprData.m_cnC_RESFMT);
            break;
        case 18: // C_LLMFMT
            iStatus = RetrieveString(mprData.m_cnC_LLMFMT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetC_LLMFMT(mprData.m_cnC_LLMFMT);
            break;
        case 19: // C_HLMFMT
            iStatus = RetrieveString(mprData.m_cnC_HLMFMT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            mprData.SetC_HLMFMT(mprData.m_cnC_HLMFMT);
            break;
        case 20: // LO_SPEC
            iStatus = RetrieveFloat(mprData.m_r4LO_SPEC);
            if(iStatus == eErrOkay)
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LOWSPEC_LIMIT;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetLO_SPEC(mprData.m_r4LO_SPEC);
            break;
        case 21: // HI_SPEC
            iStatus = RetrieveFloat(mprData.m_r4HI_SPEC);
            if(iStatus == eErrOkay)
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HIGHSPEC_LIMIT;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetHI_SPEC(mprData.m_r4HI_SPEC);
            mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);
            break;
        case 22: // RES_SCAL
            iStatus = RetrieveShort(sBuffer);
            if(iStatus == eErrOkay)
            {
                mprData.m_i1RES_SCAL = static_cast<char>(sBuffer);
                mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNOTVALID_RES_SCAL;
            }
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetRES_SCAL(mprData.m_i1RES_SCAL);
            mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);
            break;
        case 23: // LLM_SCAL
            iStatus = RetrieveShort(sBuffer);
            if(iStatus == eErrOkay)
            {
                mprData.m_i1LLM_SCAL = static_cast<char>(sBuffer);
                if(mprData.m_b1OPT_FLAG & GQTL_STDF::Stdf_MPR_V4::eNO_LL)
                    mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL_USEFIRSTMPR;
            }
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetLLM_SCAL(mprData.m_i1LLM_SCAL);
            mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);
            break;
        case 24: // HLM_SCAL
            iStatus = RetrieveShort(sBuffer);
            if(iStatus == eErrOkay)
            {
                mprData.m_i1HLM_SCAL = static_cast<char>(sBuffer);
                if(mprData.m_b1OPT_FLAG & GQTL_STDF::Stdf_MPR_V4::eNO_HL)
                    mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL_USEFIRSTMPR;
            }
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            mprData.SetHLM_SCAL(mprData.m_i1HLM_SCAL);
            mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop MPR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    // Adjust Low Limit flags if necessary
    if( (mprData.m_b1OPT_FLAG & GQTL_STDF::Stdf_MPR_V4::eNO_LL) &&
        (mprData.m_b1OPT_FLAG & GQTL_STDF::Stdf_MPR_V4::eNO_LL_USEFIRSTMPR) )
    {
        MapTests::iterator	it;
        // A low limit for this test has been found in a previous MPR?
        it = mMapMPR_LL.find(mprData.m_u4TEST_NUM);
        // If not,
        if(it == mMapMPR_LL.end() )
            // That's mean that the test has no LL
            mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL_USEFIRSTMPR;
        else
            // The test has a low limit, but we need to use the first MPR found to retrieve it
            mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_LL;
    }

    // Adjust High Limit flags if necessary
    if( (mprData.m_b1OPT_FLAG & GQTL_STDF::Stdf_MPR_V4::eNO_HL) &&
        (mprData.m_b1OPT_FLAG & GQTL_STDF::Stdf_MPR_V4::eNO_HL_USEFIRSTMPR) )
    {
        MapTests::iterator	it;
        // A high limit for this test has been found in a previous MPR?
        it = mMapMPR_HL.find(mprData.m_u4TEST_NUM);
        // If not,
        if(it == mMapMPR_HL.end() )
            // That's mean that the test has no HL
            mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL_USEFIRSTMPR;
        else
            // The test has a high limit, but we need to use the first MPR found to retrieve it
            mprData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_MPR_V4::eNO_HL;
    }
    mprData.SetOPT_FLAG(mprData.m_b1OPT_FLAG);

    // Need to scale values?
    if(mScaledData == false)
    {
        // Need to scale the RTN_RSLT field?
        if(mprData.m_kr4RTN_RSLT != NULL)
        {
            for(wBuffer=0;wBuffer<mprData.m_u2RTN_ICNT;wBuffer++)
                ScalValue(*(mprData.m_kr4RTN_RSLT+wBuffer), mprData.m_cnUNITS, mprData.m_i1RES_SCAL);
            // UNITS not modified for the moment
        }

        // UNITS not modified for the moment
        ScalValue(mprData.m_r4LO_LIMIT, mprData.m_cnUNITS, mprData.m_i1LLM_SCAL);

        // UNITS modified
        ScalValue(mprData.m_r4HI_LIMIT, mprData.m_cnUNITS, mprData.m_i1HLM_SCAL, false);
    }

    // if RTN_INDX == NULL && card(RTN_INDX) != RTN_ICNT, check if it isn't an optionnal field
    if( (bRTN_INDX_Corrupted2Check) && (iMaxFieldIDToWrite >= GQTL_STDF::Stdf_MPR_V4::eposRTN_INDX) )
    {
        GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(mprData.m_u2RTN_ICNT),int(wBuffer));
        return eErrInvalidArraySize;
    }


    _GATS_WRITE_RECORD(mprData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a FTR record.
//				 mSourceData should point to the first character after the end of the
//				 FTR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeFTR()
{
    bool		bContinue = true;
    char		cBuffer,*pszBuffer, lBuffer[256];
    WORD		wBuffer;
    int			iStatus;
    GQTL_STDF::Stdf_FTR_V4	ftrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // TEST_NUM
            iStatus = RetrieveDWord(ftrData.m_u4TEST_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            ftrData.SetTEST_NUM(ftrData.m_u4TEST_NUM);
            break;
        case 1: // HEAD_NUM
            iStatus = RetrieveByte(ftrData.m_u1HEAD_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            ftrData.SetHEAD_NUM(ftrData.m_u1HEAD_NUM);
            break;
        case 2: // SITE_NUM
            iStatus = RetrieveByte(ftrData.m_u1SITE_NUM);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            ftrData.SetSITE_NUM(ftrData.m_u1SITE_NUM);
            break;
        case 3: // Pass/Fail Flag
            iStatus = RetrieveChar(cBuffer,false);
            if(_GATS_ERROR_RETRIEVE_REQUIRED(iStatus))	return iStatus;
            switch(cBuffer)
            {
            case 'F':	ftrData.m_b1TEST_FLG |= GQTL_STDF::Stdf_FTR_V4::eTEST_FAILED;	break;
            case 'P': break; // No flag to set
            default: GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                     return eErrNotValidFlag;
            }
            ftrData.SetTEST_FLG(ftrData.m_b1TEST_FLG);
            break;
        case 4: // Alarm Flags
            iStatus = RetrieveChar(lBuffer, 256);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            pszBuffer = lBuffer;
            while(*pszBuffer != '\0')
            {
                switch(*pszBuffer)
                {
                case 'A':	ftrData.m_b1TEST_FLG |= GQTL_STDF::Stdf_FTR_V4::eALARM;	break;
                case 'N':	ftrData.m_b1TEST_FLG |= GQTL_STDF::Stdf_FTR_V4::eTEST_NOTEXECUTED;	break;
                case 'T':	ftrData.m_b1TEST_FLG |= GQTL_STDF::Stdf_FTR_V4::eTIMEOUT;	break;
                case 'U':	ftrData.m_b1TEST_FLG |= GQTL_STDF::Stdf_FTR_V4::eNOTRELIABLE_RESULT;	break;
                case 'X':	ftrData.m_b1TEST_FLG |= GQTL_STDF::Stdf_FTR_V4::eTEST_ABORTED;	break;
                default:	GSET_ERROR2(AtdfToStdf,eErrNotValidFlag,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                            return eErrNotValidFlag;
                }
                pszBuffer++;
            }
            ftrData.SetTEST_FLG(ftrData.m_b1TEST_FLG);
            // Set mandatory parameters in the stdf file
            ftrData.m_b1OPT_FLAG |=
                static_cast<char>(GQTL_STDF::Stdf_FTR_V4::eOPT_FLAG_ALL);
            ftrData.SetOPT_FLAG(ftrData.m_b1OPT_FLAG);
            break;
        case 5: // VECT_NAM
            iStatus = RetrieveString(ftrData.m_cnVECT_NAM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetVECT_NAM(ftrData.m_cnVECT_NAM);
            break;
        case 6: // TIME_SET
            iStatus = RetrieveString(ftrData.m_cnTIME_SET);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetTIME_SET(ftrData.m_cnTIME_SET);
            break;
        case 7: // CYCL_CNT
            iStatus = RetrieveDWord(ftrData.m_u4CYCL_CNT);
            if(iStatus == eErrOkay)
                ftrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_FTR_V4::eNOTVALID_CYCL_CNT;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            ftrData.SetCYCL_CNT(ftrData.m_u4CYCL_CNT);
            break;
        case 8: // REL_VADR
            iStatus = RetrieveDWord(ftrData.m_u4REL_VADR,true);
            if(iStatus == eErrOkay)
                ftrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_FTR_V4::eNOTVALID_REL_VADR;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            ftrData.SetREL_VADR(ftrData.m_u4REL_VADR);
            break;
        case 9: // REPT_CNT
            iStatus = RetrieveDWord(ftrData.m_u4REPT_CNT);
            if(iStatus == eErrOkay)
                ftrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_FTR_V4::eNOTVALID_REPT_CNT;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            ftrData.SetREPT_CNT(ftrData.m_u4REPT_CNT);
            break;
        case 10: // NUM_FAIL
            iStatus = RetrieveDWord(ftrData.m_u4NUM_FAIL);
            if(iStatus == eErrOkay)
                ftrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_FTR_V4::eNOTVALID_NUM_FAIL;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            ftrData.SetNUM_FAIL(ftrData.m_u4NUM_FAIL);
            break;
        case 11: // XFAIL_AD
            iStatus = RetrieveLong(ftrData.m_i4XFAIL_AD);
            if(iStatus == eErrOkay)
                ftrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_FTR_V4::eNOTVALID_xFAIL_AD;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            ftrData.SetXFAIL_AD(ftrData.m_i4XFAIL_AD);
            break;
        case 12: // YFAIL_AD
            iStatus = RetrieveLong(ftrData.m_i4YFAIL_AD);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetYFAIL_AD(ftrData.m_i4YFAIL_AD);
            break;
        case 13: // VECT_OFF
            iStatus = RetrieveShort(ftrData.m_i2VECT_OFF);
            if(iStatus == eErrOkay)
                ftrData.m_b1OPT_FLAG &= ~GQTL_STDF::Stdf_FTR_V4::eNOTVALID_VECT_OFF;
            else
            if(iStatus != eErrEmpty)
                return iStatus;
            ftrData.SetVECT_OFF(ftrData.m_i2VECT_OFF);
            break;
        case 14: // RTN_INDX and RTN_ICNT
            iStatus = RetrieveWordArray(ftrData.m_u2RTN_ICNT,&ftrData.m_ku2RTN_INDX);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetRTN_ICNT(ftrData.m_u2RTN_ICNT);
            for (int index=0; index<ftrData.m_u2RTN_ICNT; ++index)
                ftrData.SetRTN_INDX(index, ftrData.m_ku2RTN_INDX[index]);
            break;
        case 15: // RTN_STAT
            iStatus = RetrieveNibbleArray(wBuffer,&ftrData.m_kn1RTN_STAT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            if(wBuffer != ftrData.m_u2RTN_ICNT)
            {
                GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(ftrData.m_u2RTN_ICNT),int(wBuffer));
                return eErrInvalidArraySize;
            }
            ftrData.SetRTN_ICNT(ftrData.m_u2RTN_ICNT);
            for (int index=0; index<ftrData.m_u2RTN_ICNT; ++index)
                ftrData.SetRTN_STAT(index, ftrData.m_kn1RTN_STAT[index]);
            break;
        case 16: // PGM_INDX and PGM_ICNT
            iStatus = RetrieveWordArray(ftrData.m_u2PGM_ICNT,&ftrData.m_ku2PGM_INDX);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetPGM_ICNT(ftrData.m_u2PGM_ICNT);
            for (int index=0; index<ftrData.m_u2PGM_ICNT; ++index)
                ftrData.SetPGM_INDX(index, ftrData.m_ku2PGM_INDX[index]);
            break;
        case 17: // PGM_STAT
            iStatus = RetrieveNibbleArray(wBuffer,&ftrData.m_kn1PGM_STAT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            if(wBuffer != ftrData.m_u2PGM_ICNT)
            {
                GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(ftrData.m_u2PGM_ICNT),int(wBuffer));
                return eErrInvalidArraySize;
            }
            ftrData.SetPGM_ICNT(ftrData.m_u2PGM_ICNT);
            for (int index=0; index<ftrData.m_u2PGM_ICNT; ++index)
                ftrData.SetPGM_STAT(index, ftrData.m_kn1PGM_STAT[index]);
            break;
        case 18: // FAIL_PIN
        {
            iStatus = RetrieveIndexArray(ftrData.m_dnFAIL_PIN);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetFAIL_PIN();
            break;
        }
        case 19: // OP_CODE
            iStatus = RetrieveString(ftrData.m_cnOP_CODE);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetOP_CODE(ftrData.m_cnOP_CODE);
            break;
        case 20: // TEST_TXT
            iStatus = RetrieveString(ftrData.m_cnTEST_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetTEST_TXT(ftrData.m_cnTEST_TXT);
            break;
        case 21: // ALARM_ID
            iStatus = RetrieveString(ftrData.m_cnALARM_ID);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetALARM_ID(ftrData.m_cnALARM_ID);
            break;
        case 22: // PROG_TXT
            iStatus = RetrieveString(ftrData.m_cnPROG_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetPROG_TXT(ftrData.m_cnPROG_TXT);
            break;
        case 23: // RSLT_TXT
            iStatus = RetrieveString(ftrData.m_cnRSLT_TXT);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetRSLT_TXT(ftrData.m_cnRSLT_TXT);
            break;
        case 24: // PATG_NUM
            iStatus = RetrieveByte(ftrData.m_u1PATG_NUM);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetPATG_NUM(ftrData.m_u1PATG_NUM);
            break;
        case 25: // SPIN_MAP
        {
            iStatus = RetrieveIndexArray(ftrData.m_dnSPIN_MAP);
            if(_GATS_ERROR_RETRIEVE(iStatus))	return iStatus;
            ftrData.SetSPIN_MAP();
            break;
        }
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop FTR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(ftrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a BPS record.
//				 mSourceData should point to the first character after the end of the
//				 BPS record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeBPS()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_BPS_V4	bpsData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // SEQ_NAME
            iStatus = RetrieveString(bpsData.m_cnSEQ_NAME);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            bpsData.SetSEQ_NAME(bpsData.m_cnSEQ_NAME);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop BPS analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(bpsData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a EPS record.
//				 mSourceData should point to the first character after the end of the
//				 EPS record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeEPS()
{
    GQTL_STDF::Stdf_EPS_V4	epsData;

    _GATS_WRITE_RECORD(epsData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a GDR record.
//				 mSourceData should point to the first character after the end of the
//				 GDR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeGDR()
{
    bool		bContinue = true;
    int			iStatus;
    GQTL_STDF::Stdf_GDR_V4                  gdrData;
    std::vector<GQTL_STDF::stdf_type_vn>    lVnGenData;
    GQTL_STDF::stdf_type_vn                 lTmpGenData;

    do
    {
        // Reset object members
        lTmpGenData.Reset();

        // GEN_DATA
        iStatus = RetrieveGenData(lTmpGenData);
        if(_GATS_ERROR_RETRIEVE(iStatus))
        {
            return iStatus;
        }
        lVnGenData.push_back(lTmpGenData);

        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop BPS analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    gdrData.SetGEN_DATA(lVnGenData);

    _GATS_WRITE_RECORD(gdrData,mStdf);

    return ReachNextRecordHeader();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Analyze a DTR record.
//				 mSourceData should point to the first character after the end of the
//				 DTR record header.
//
// Return: eErrOkay is successful or an eErrXXX code if a problem occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::AnalyzeDTR()
{
    bool		bContinue = true;
    int     	iStatus;
    GQTL_STDF::Stdf_DTR_V4	dtrData;
    int			iCurrentField = 0;

    do
    {
        switch(iCurrentField)
        {
        case 0: // TEST_DAT
            iStatus = RetrieveString(dtrData.m_cnTEXT_DAT);
            if(_GATS_ERROR_RETRIEVE(iStatus))
                return iStatus;
            dtrData.SetTEXT_DAT(dtrData.m_cnTEXT_DAT);
            break;
        default:
            bContinue = false;
            break;
        }

        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop BPS analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));


    _GATS_WRITE_RECORD(dtrData,mStdf);

    return ReachNextRecordHeader();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// GENERIC FUNCTIONS
//
/////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------------------------------
// Description : returns month number corresponding to the 3 letter month passed as argument.
//				 The month given as argument should be the 3 first characters of the month name (in english).
//
//
// Argument(s) :
//      char* : MMM month name
//
// Return : month number [0-11] if successful.
//			-1 else.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::GetMonthNumber(char* szMonth)
{
    if( (szMonth[0] == 'J' || szMonth[0] == 'j') &&
        (szMonth[1] == 'A' || szMonth[1] == 'a') &&
        (szMonth[2] == 'N' || szMonth[2] == 'n') )
        return 0;

    if( (szMonth[0] == 'F' || szMonth[0] == 'f') &&
        (szMonth[1] == 'E' || szMonth[1] == 'e') &&
        (szMonth[2] == 'B' || szMonth[2] == 'b') )
        return 1;

    if( (szMonth[0] == 'M' || szMonth[0] == 'm') &&
        (szMonth[1] == 'A' || szMonth[1] == 'a') &&
        (szMonth[2] == 'R' || szMonth[2] == 'r') )
        return 2;

    if( (szMonth[0] == 'A' || szMonth[0] == 'a') &&
        (szMonth[1] == 'P' || szMonth[1] == 'p') &&
        (szMonth[2] == 'R' || szMonth[2] == 'r') )
        return 3;

    if( (szMonth[0] == 'M' || szMonth[0] == 'm') &&
        (szMonth[1] == 'A' || szMonth[1] == 'a') &&
        (szMonth[2] == 'Y' || szMonth[2] == 'y') )
        return 4;

    if( (szMonth[0] == 'J' || szMonth[0] == 'j') &&
        (szMonth[1] == 'U' || szMonth[1] == 'u') &&
        (szMonth[2] == 'N' || szMonth[2] == 'n') )
        return 5;

    if( (szMonth[0] == 'J' || szMonth[0] == 'j') &&
        (szMonth[1] == 'U' || szMonth[1] == 'u') &&
        (szMonth[2] == 'L' || szMonth[2] == 'l') )
        return 6;

    if( (szMonth[0] == 'A' || szMonth[0] == 'a') &&
        (szMonth[1] == 'U' || szMonth[1] == 'u') &&
        (szMonth[2] == 'G' || szMonth[2] == 'g') )
        return 7;

    if( (szMonth[0] == 'S' || szMonth[0] == 's') &&
        (szMonth[1] == 'E' || szMonth[1] == 'e') &&
        (szMonth[2] == 'P' || szMonth[2] == 'p') )
        return 8;

    if( (szMonth[0] == 'O' || szMonth[0] == 'o') &&
        (szMonth[1] == 'C' || szMonth[1] == 'c') &&
        (szMonth[2] == 'T' || szMonth[2] == 't') )
        return 9;

    if( (szMonth[0] == 'N' || szMonth[0] == 'n') &&
        (szMonth[1] == 'O' || szMonth[1] == 'o') &&
        (szMonth[2] == 'V' || szMonth[2] == 'v') )
        return 10;

    if( (szMonth[0] == 'D' || szMonth[0] == 'd') &&
        (szMonth[1] == 'E' || szMonth[1] == 'e') &&
        (szMonth[2] == 'C' || szMonth[2] == 'c') )
        return 11;

    return -1;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a date from the current mSourceData position.
//				 *** mSourceData must point to the first character date!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 The date format in a ATDF file is:
//						hh:mm:ss DD-MMM-YYYY
//
//				 Insignificant leading zeroes in all numbers are optional.
//
// Argument(s) :
//      unsigned DWORD& nDate : Receive the date in U*4 format.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no date field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveDate(DWORD& dwDate)
{
    char		szDate[32];
    char		szMonth[4];
    int			iResult=0,iHour=0,iMin=0,iSec=0,iYear=0,iMonth=0,iDay=0;

    // Set date to 1 Jan 1970 in case it's not present
    dwDate = (DWORD)0;


    // Pyc, 12.01.12, case 5743
    iResult = RetrieveField(szDate, 32);
    if(iResult != eErrOkay)
        return iResult;
    else
        iResult = 0;

    // If there is no date, just return with no error
    if((*szDate == '\0') || (strcmp(szDate, "0") == 0))
    {
        ReachNextField();
        return eErrOkay;
    }

    // First try standard ATDF format: hh:mm:ss DD-MMM-YYYY
    iResult = sscanf(szDate,"%d:%d:%d%*1c%d%*1c%3s%*1c%d",&iHour,&iMin,&iSec,&iDay,szMonth,&iYear);
    if((iResult == 6) && ((iMonth = GetMonthNumber(szMonth)) >= 0))
        goto tagGoodDateFormat;

    // Try alternate format: yyyy-mm-dd hh:mm
    iSec = 0;
    iResult = sscanf(szDate,"%d-%d-%d%*1c%d:%d",&iYear,&iMonth,&iDay,&iHour,&iMin);
    if(	(iResult == 5) && (iYear >= 1900) && (iMonth >= 1) && (iMonth <= 12) && (iDay >= 1) && (iDay <= 31) &&
        (iHour >= 0) && (iHour <= 23) && (iMin >= 0) && (iMin <= 59))
    {
        goto tagGoodDateFormat;
    }

    // Date/time format doesn't match the ATDF specs
    GSET_ERROR2(AtdfToStdf,eErrBadDateFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
    return eErrBadDateFormat;

tagGoodDateFormat:

    // case 4111 : update time management
    QDateTime	qdtConsideratedDateTime;
    qdtConsideratedDateTime.setTimeSpec(Qt::UTC);
    QDate		qdConsideratedDate;
    QTime		qtConsideratedTime;
    bool		bIsValidSet = false;


    bIsValidSet = qdConsideratedDate.setDate( iYear, (iMonth+1) , iDay );		// cf. int AtdfToStdf::GetMonthNumber(char* szMonth) for (iMonth+1)
    if(!bIsValidSet)
    {
        GSET_ERROR2(AtdfToStdf,eErrBadDateFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());		// Try to set an invalid date
        Q_ASSERT(false);
        return eErrBadDateFormat;
    }

    bIsValidSet = qtConsideratedTime.setHMS( iHour, iMin, iSec);
    if(!bIsValidSet)
    {
        GSET_ERROR2(AtdfToStdf,eErrBadDateFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());		// Try to set an invalid time
        Q_ASSERT(false);
        return eErrBadDateFormat;
    }

    qdtConsideratedDateTime.setDate(qdConsideratedDate);
    qdtConsideratedDateTime.setTime(qtConsideratedTime);
    if(!qdtConsideratedDateTime.isValid())
    {
        GSET_ERROR2(AtdfToStdf,eErrBadDateFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());		// Try to set an invalid dateTime
        Q_ASSERT(false);
        return eErrBadDateFormat;
    }

    // qdtConsideratedDateTime = qdtConsideratedDateTime.toTimeSpec(Qt::UTC);
    dwDate = qdtConsideratedDateTime.toTime_t();
    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a generic data from the current mSourceData position.
//				 *** mSourceData must point to the first character of the gen data!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      genData : Receive the generic data.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no date field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveGenData(GQTL_STDF::stdf_type_vn& genData)
{
    if( _GATS_IS_ENDOF_FIELD() )
    {	// No date value at the current position
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;
    }
    // Extract the generic data type
    char	cBuffer = *mSourceData++;
    int		iResult=0;
    //genData.Reset();

    switch(cBuffer)
    {
    case 'U':
        iResult = RetrieveByte(genData.m_u1Data);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeU1;
        break;
    case 'M':
        iResult = RetrieveWord(genData.m_u2Data);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeU2;
        break;
    case 'B':
        iResult = RetrieveDWord(genData.m_u4Data);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeU4;
        break;
    case 'I':
        iResult = RetrieveChar(genData.m_i1Data,true);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeI1;
        break;
    case 'S':
        iResult = RetrieveShort(genData.m_i2Data);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeI2;
        break;
    case 'L':
    {
        stdf_type_i4 lData;
        iResult = RetrieveLong(lData);
        genData.m_i4Data = static_cast<int>(lData);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeI4;
        break;
    }
    case 'F':
        iResult = RetrieveFloat(genData.m_r4Data);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeR4;
        break;
    case 'D':
        iResult = RetrieveDouble(genData.m_r8Data);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeR8;
        break;
    case 'T':
    {
        iResult = RetrieveString(genData.m_cnData);
        if(_GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeCN;
        break;
    }
    case 'X':
    {
        iResult = RetrieveHexaString(genData.m_bnData);
        if(iResult != eErrOkay || _GATS_ERROR_RETRIEVE(iResult))
        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeBN;
        break;
    }
    case 'Y':
    {
        iResult = RetrieveIndexArray(genData.m_dnData);
        if(iResult != eErrOkay || _GATS_ERROR_RETRIEVE(iResult))
        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeDN;
        break;
    }
    case 'N':
    {
        int lResult = RetrieveByte((stdf_type_u1&)genData.m_n1Data);
        if(lResult != eErrOkay || _GATS_ERROR_RETRIEVE(iResult))        {
            GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrGDR;
        }
        genData.uiDataTypeCode = GQTL_STDF::Stdf_GDR_V4::eTypeN1;
        break;
    }
    default:
        GSET_ERROR2(AtdfToStdf,eErrGDR,NULL,mCurrentLineNbr,GetCurrentLineIndex());
        break;
    }

    return eErrOkay;
}

int AtdfToStdf::RetrieveString(QString &String)
{
    int lStatus=0;
    char lString[256];
    *lString = '\0';

    lStatus = RetrieveField(lString, 256);
    String = QString(lString);
    if(mSourceData >= mEndOfData)
        return eErrEndOfFile;
    if(lStatus == eErrOkay)
        return ReachNextField();

    return lStatus;
}

int AtdfToStdf::RetrieveChar(char* szString, int MaxSize)
{
    int	iStatus;

    *szString = '\0';

    iStatus = RetrieveField(szString, MaxSize);
    if(mSourceData >= mEndOfData)
        return eErrEndOfFile;
    if(iStatus == eErrOkay)
        return ReachNextField();

    return iStatus;
}
// ----------------------------------------------------------------------------------------------------------
// Description : Extract a char value from the current mSourceData position.
//				 *** mSourceData must point to the character!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      bData : Receive the value.
//		bHexValue: true if the value to retrieve is in hexadecimal format
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no byte field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveByte(stdf_type_u1& bData, bool bHexValue /*= false*/)
{
    char	szByte[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szByte, 128);
    if(iStatus == eErrOkay)
    {
        if(bHexValue == true)
        {
            ASSERT_VALIDHEXADECIMAL(szByte);
            bData = static_cast<stdf_type_u1>(gahextodw(szByte));
        }
        else
        {
            ASSERT_VALIDUNSIGNED(szByte);
            bData = static_cast<stdf_type_u1>(gatodw(szByte));
        }
        return ReachNextField();
    }

    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a char value from the current mSourceData position.
//				 *** mSourceData must point to the first character representing the byte
//				     value!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      wData : Receive word value.
//		bAsSignedInteger: true if the data to retrieve is a signed integer
//		bFailSeveral: If true, the function will failed if there are more than one character
//					  at the current position.
//					  If false, the function will only extract the first character, and
//					  ignore the others.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no char field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveChar(char& cData, bool bAsSignedInteger, bool bFailSeveral /* = true */)
{
    char	szByte[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szByte, 128);

    if(iStatus!=eErrOkay)
        return iStatus;

    // If there is still a valid character to extract, means the current field is not a char
    if(bFailSeveral && (szByte[1]!='\0'))
    {
        GSET_ERROR2(AtdfToStdf,eErrBadChar,NULL,mCurrentLineNbr,GetCurrentLineIndex());
        return eErrBadChar;
    }

    if(bAsSignedInteger == true)
    {
        ASSERT_VALIDINTEGER(szByte);

        cData = static_cast<char>(gatoi(szByte));
        return ReachNextField();
    }
    else
    {
        cData = szByte[0];

        if(cData=='\0')
        {
            cData = ' ';
            return eErrEmpty;
        }

        return ReachNextField();
    }
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a word value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the word string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      wData : Receive word value.
//		bHexValue: true if the value to retrieve is in hexadecimal format
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no word field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveWord(stdf_type_u2& wData,bool bHexValue /* = false*/)
{
    char	szWord[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szWord, 128);
    if(iStatus == eErrOkay)
    {
        if(bHexValue == true)
        {
            ASSERT_VALIDHEXADECIMAL(szWord);
            wData = static_cast<stdf_type_u2>(gahextodw(szWord));
        }
        else
        {
            ASSERT_VALIDUNSIGNED(szWord);
            wData = static_cast<stdf_type_u2>(gatodw(szWord));
        }
        return ReachNextField();
    }

    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a dword value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the dword string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      dwData : Receive dword value.
//		bHexValue: true if the value to retrieve is in hexadecimal format
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no dword field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveDWord(stdf_type_u4& dwData,bool bHexValue /* = false*/)
{
    char	szDWord[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szDWord, 128);
    if(iStatus == eErrOkay)
    {
        if(bHexValue == true)
        {
            ASSERT_VALIDHEXADECIMAL(szDWord);
            dwData = gahextodw(szDWord);
        }
        else
        {
            ASSERT_VALIDUNSIGNED(szDWord);
            dwData = static_cast<stdf_type_u4>(gatodw(szDWord));
        }
        return ReachNextField();
    }

    return iStatus;
}


// ----------------------------------------------------------------------------------------------------------
// Description : Extract a short value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the short string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      sData : Receive short value.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no short field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveShort(stdf_type_i2& sData)
{
    char	szShort[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szShort, 128);
    if(iStatus == eErrOkay)
    {
        ASSERT_VALIDINTEGER(szShort);

        sData = static_cast<stdf_type_i2>(gatoi(szShort));
        return ReachNextField();
    }

    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an int4 value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the dword string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      iData : Receive int value.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no dword field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveLong(stdf_type_i4& iData)
{
    char	szInt4[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int    	iStatus;

    iStatus = RetrieveField(szInt4, 128);
    if(iStatus == eErrOkay)
    {
        ASSERT_VALIDINTEGER(szInt4);

        iData = gatoi(szInt4);
        return ReachNextField();
    }

    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a float value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the float string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      fData : Receive float value.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no float field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveFloat(stdf_type_r4& fData)
{
    char	szFloat[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szFloat, 128);
    if(iStatus == eErrOkay)
    {
        ASSERT_VALIDFLOAT(szFloat);

        fData = gatof(szFloat);
        return ReachNextField();
    }
    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a double float value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the double float string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      dfData : Receive double float value.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no float field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveDouble(stdf_type_r8& dfData)
{
    char	szFloat[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szFloat, 128);
    if(iStatus == eErrOkay)
    {
        ASSERT_VALIDFLOAT(szFloat);

        dfData = atof(szFloat);
        return ReachNextField();
    }
    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a unsigned long long value from the current mSourceData position.
//				 *** mSourceData must point to the first character of the ulonglong string!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      lData : Receive undigned long long value.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no ulonglong field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveULongLong(stdf_type_u8 &lData)
{
    char	szULongLong[128]; // No test for buffer overflow (to maximize speed), so take room, just in case
    int 	iStatus;

    iStatus = RetrieveField(szULongLong, 128);
    if(iStatus == eErrOkay)
    {
        ASSERT_VALIDUNSIGNED(szULongLong);

        lData = gatoull(szULongLong);
        return ReachNextField();
    }
    return iStatus;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of char values from the current mSourceData position.
//				 *** mSourceData must point to the first char of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					A,B,C,D,E,F
//					1,4,2,Q,V,3
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppcArray: A pointer to the array that will be allocated by this function.
//      cPadChar: A pad character to use when element is empty in the array. If empty, empty element are skipped
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no char array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveCharArray(WORD& wCount, char** ppcArray,
                                  bool bKeepEmptyField /* = true */, char cPadChar /* = '0'*/)
{
    if(*ppcArray != NULL)
    {
        delete [] *ppcArray;
        *ppcArray = NULL;
    }
    wCount = 0;

    if( _GATS_IS_ENDOF_FIELD() )
    {
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;	// The array is empty
    }
    *ppcArray = new char[0xFFFF];

    char*	pcArray     = *ppcArray;
    int     lTotalField = 0;

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF ) // Make sure to not overflow buffer
        {
            // At least one field detected.
            if (lTotalField == 0)
                lTotalField++;

            if(*mSourceData == ',')
            {
                mSourceData++;

                if (bKeepEmptyField && wCount < lTotalField)
                {
                    *pcArray++ = cPadChar;
                    wCount++;
                }

                // Comma detected, it means there is another field coming
                lTotalField++;
            }
            else
            {
                *pcArray++ = *mSourceData++;
                wCount++;
            }
        }
    } while (RecordContinueOnNextLine() == true);

    // We reached the end  of the string, if the last character was a comma and we have to keep empty parts.
    // Take it into account now
    if (bKeepEmptyField && wCount < lTotalField)
    {
        *pcArray++ = cPadChar;
        wCount++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of byte values from the current mSourceData position.
//				 *** mSourceData must point to the first character of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					1,2,3,56
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppbArray: A pointer to the array that will be allocated by this function.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no byte array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveByteArray(WORD& wCount, BYTE ** ppbArray)
{
    if(*ppbArray != NULL)
    {
        delete [] *ppbArray;
        *ppbArray = NULL;
    }
    wCount = 0;

    if( _GATS_IS_ENDOF_FIELD() )
    {
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;	// The array is empty
    }
    *ppbArray = new BYTE[0xFFFF];

    BYTE*	pbArray = *ppbArray;
    char	szDigit[4];				// Maximum value of BYTE is 255 (3 char max, + '\0')
    char*	pszDigit = szDigit;
    WORD	wBuffer;

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF &&
               (pszDigit-szDigit < 3) ) // Make sure to not overflow buffer
        {

            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';

                    ASSERT_VALIDUNSIGNED(szDigit);
                    wBuffer = static_cast<WORD>(gatodw(szDigit));

                    *pbArray++ = static_cast<BYTE>(wBuffer);
                    wCount++;
                    pszDigit = szDigit;
                }

                mSourceData++;
            }
            else if(*mSourceData < '0' || *mSourceData > '9')
            {
                GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrBadArrayFormat;
            }
            else
            {
                *pszDigit++ = *mSourceData++;
            }

        }

        if(pszDigit-szDigit == 3)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else if((*mSourceData != '\n') && (*mSourceData != '\r') && pszDigit != szDigit)
        {
            *pszDigit = '\0';

            ASSERT_VALIDUNSIGNED(szDigit);
            wBuffer = static_cast<WORD>(gatodw(szDigit));

            *pbArray++ = static_cast<BYTE>(wBuffer);
            wCount++;

            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);	// if record on 2 lines, *mSourceData++

    if(pszDigit != szDigit)
    {
        *pszDigit = '\0';

        ASSERT_VALIDUNSIGNED(szDigit);
        wBuffer = static_cast<WORD>(gatodw(szDigit));

        *pbArray = static_cast<BYTE>(wBuffer);
        wCount++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of Nibble values from the current mSourceData position.
//				 *** mSourceData must point to the first character of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					FFE0014C
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppbArray: A pointer to the array that will be allocated by this function.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no byte array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveNibbleArray(WORD& wCount, stdf_type_n1** ppbArray)
{
    if(*ppbArray != NULL)
    {
        delete [] *ppbArray;
        *ppbArray = NULL;
    }
    wCount = 0;

    if( _GATS_IS_ENDOF_FIELD() )
    {
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;	// The array is empty
    }

    *ppbArray = new stdf_type_n1[0xFFFF];

    stdf_type_n1*	pbArray = *ppbArray;
    char            szDigit[2];				// Maximum value of nibble is 127, but encoded in hexadecimal (1 char max, + '\0')
    char*           pszDigit = szDigit;
    WORD            wBuffer;

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF &&
               (pszDigit-szDigit < 2) ) // Make sure to not overflow buffer
        {

            if(*mSourceData == ',' || (pszDigit - szDigit == 1))
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';

                    ASSERT_VALIDHEXADECIMAL(szDigit);
                    wBuffer = static_cast<WORD>(gahextodw(szDigit));

                    *pbArray++ = static_cast<stdf_type_n1>(wBuffer);
                    wCount++;
                    pszDigit = szDigit;
                }

                // ',' is optionnal
                if(*(mSourceData) != ',')			// to count values it isn't a comma separated array
                    *pszDigit++ = *mSourceData++;
                else
                    mSourceData++;

            }
            else
            {
                *pszDigit++ = *mSourceData++;
            }

        }

        if(pszDigit-szDigit > 1)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else if((*mSourceData != '\n') && (*mSourceData != '\r') && pszDigit != szDigit)
        {
            *pszDigit = '\0';

            ASSERT_VALIDHEXADECIMAL(szDigit);
            wBuffer = static_cast<WORD>(gahextodw(szDigit));

            *pbArray++ = static_cast<stdf_type_n1>(wBuffer);
            wCount++;

            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);	// if record on 2 lines, *mSourceData++

    if(pszDigit != szDigit)
    {
        *pszDigit = '\0';

        ASSERT_VALIDHEXADECIMAL(szDigit);
        wBuffer = static_cast<WORD>(gahextodw(szDigit));

        *pbArray = static_cast<stdf_type_n1>(wBuffer);
        wCount++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of WORD values from the current mSourceData position.
//				 *** mSourceData must point to the first word of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					3,5,6,7,3,2
//					3452,23,1,435,342,60000,345
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppwArray: A pointer to the array that will be allocated by this function.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no word array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveWordArray(WORD& wCount, WORD** ppwArray)
{
    wCount = 0;

    if(*ppwArray != NULL)
    {
        delete [] *ppwArray;
        *ppwArray = NULL;
    }


    if( _GATS_IS_ENDOF_FIELD() )
    {
        ReachNextField();
        return eErrEmpty;	// The array is empty
    }

    *ppwArray = new WORD[0xFFFF];

    WORD*	pwArray = *ppwArray;
    char	szDigit[6]; // Only WORD values digit
    char*	pszDigit = szDigit;
    DWORD	dwValue;
    bool	bOverflow = false;

    do
    {
        bOverflow = false;

        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF &&
               !bOverflow ) // Make sure to not overflow buffer
        {
            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';
                    ASSERT_VALIDUNSIGNED(szDigit);
                    dwValue = gatodw(szDigit);
                    if(dwValue > 0xFFFF)
                        bOverflow = true;
                    else
                    {
                        *pwArray++ = static_cast<WORD>(dwValue);
                        wCount++;
                        pszDigit = szDigit;
                    }
                }
                mSourceData++;
            }
            else
            if(*mSourceData < '0' || *mSourceData > '9')
            {
                GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrBadArrayFormat;
            }
            else
            {
                *pszDigit++ = *mSourceData++;
            }
            if(pszDigit-szDigit > 5)
                bOverflow = true;
        }

        // Find a digit that is not a WORD?
        if(bOverflow)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else
        if( (*mSourceData != '\n') && (*mSourceData != '\r') && (pszDigit != szDigit) )
        {
            *pszDigit = '\0';
            ASSERT_VALIDUNSIGNED(szDigit);
            *pwArray++ = static_cast<WORD>(gatodw(szDigit));
            wCount++;

            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);

    if( pszDigit != szDigit )
    {
        *pszDigit = '\0';
        ASSERT_VALIDUNSIGNED(szDigit);
        *pwArray = static_cast<WORD>(gatodw(szDigit));
        wCount++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of DWORD values from the current mSourceData position.
//				 *** mSourceData must point to the first dword of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					3,5,6,7,3,2
//					3452,23,1,435,342,60000,345
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppdwArray: A pointer to the array that will be allocated by this function.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no word array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveDWordArray(WORD& wCount, stdf_type_u4** ppdwArray)
{
    wCount = 0;

    if(*ppdwArray != NULL)
    {
        delete [] *ppdwArray;
        *ppdwArray = NULL;
    }


    if( _GATS_IS_ENDOF_FIELD() )
    {
        ReachNextField();
        return eErrEmpty;	// The array is empty
    }

    *ppdwArray = new stdf_type_u4[0xFFFF];

    stdf_type_u4*	pdwArray = *ppdwArray;
    char            szDigit[11]; // Only DWORD values digit
    char*           pszDigit = szDigit;
    stdf_type_u4	dwValue;
    bool            bOverflow = false;

    do
    {
        bOverflow = false;

        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF &&
               !bOverflow ) // Make sure to not overflow buffer
        {
            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';
                    ASSERT_VALIDUNSIGNED(szDigit);
                    dwValue = gatodw(szDigit);
                    if(dwValue > 0xFFFF)
                        bOverflow = true;
                    else
                    {
                        *pdwArray++ = static_cast<stdf_type_u4>(dwValue);
                        wCount++;
                        pszDigit = szDigit;
                    }
                }
                mSourceData++;
            }
            else
            if(*mSourceData < '0' || *mSourceData > '9')
            {
                GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrBadArrayFormat;
            }
            else
            {
                *pszDigit++ = *mSourceData++;
            }
            if(pszDigit-szDigit > 10)
                bOverflow = true;
        }

        // Find a digit that is not a WORD?
        if(bOverflow)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else
        if( (*mSourceData != '\n') && (*mSourceData != '\r') && (pszDigit != szDigit) )
        {
            *pszDigit = '\0';
            ASSERT_VALIDUNSIGNED(szDigit);
            *pdwArray++ = static_cast<stdf_type_u4>(gatodw(szDigit));
            wCount++;

            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);

    if( pszDigit != szDigit )
    {
        *pszDigit = '\0';
        ASSERT_VALIDUNSIGNED(szDigit);
        *pdwArray = static_cast<stdf_type_u4>(gatodw(szDigit));
        wCount++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of unsigned long long  values from the current mSourceData position.
//				 *** mSourceData must point to the first word of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					3,5,6,7,3,2
//					3452,23,1,435,342,60000,345
//                  13339994,34948373838,3838,3
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppwArray: A pointer to the array that will be allocated by this function.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no word array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveULongLongArray(WORD& wCount, unsigned long long ** ppwArray)
{
    wCount = 0;

    if(*ppwArray != NULL)
    {
        delete [] *ppwArray;
        *ppwArray = NULL;
    }


    if( _GATS_IS_ENDOF_FIELD() )
    {
        ReachNextField();
        return eErrEmpty;	// The array is empty
    }

    *ppwArray = new unsigned long long[0xFFFF];

    unsigned long long *    pwArray = *ppwArray;
    char	szDigit[21]; // Only unsigned long long values digit
    char*	pszDigit = szDigit;
    bool	bOverflow = false;

    do
    {
        bOverflow = false;

        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF && bOverflow == false) // Make sure to not overflow buffer
        {
            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';
                    *pwArray++ = gatoull(szDigit);
                    wCount++;
                    pszDigit = szDigit;
                }
                mSourceData++;
            }
            else if(*mSourceData < '0' || *mSourceData > '9')
            {
                GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrBadArrayFormat;
            }
            else
            {
                *pszDigit++ = *mSourceData++;
            }
            if(pszDigit-szDigit > 20)
                bOverflow = true;
        }

        // Find a digit that is not a WORD?
        if(bOverflow)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else if( (*mSourceData != '\n') && (*mSourceData != '\r') && (pszDigit != szDigit) )
        {
            *pszDigit = '\0';
            *pwArray++ = gatoull(szDigit);
            wCount++;

            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);

    if( pszDigit != szDigit )
    {
        *pszDigit = '\0';
        *pwArray = gatoull(szDigit);
        wCount++;
    }

    return ReachNextField();
}


// ----------------------------------------------------------------------------------------------------------
// Description : Extract an array of float values from the current mSourceData position.
//				 *** mSourceData must point to the first word of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
//				 Examples of array that can be processed by this function:
//					3.65,5.5,6.2e3,7.4
//
// Argument(s) :
//      wCount: Receive the number of element in the extracted array.
//		ppwArray: A pointer to the array that will be allocated by this function.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no word array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveFloatArray(WORD& wCount, float** ppfArray)
{
    if(*ppfArray != NULL)
    {
        delete [] *ppfArray;
        *ppfArray = NULL;
    }
    wCount = 0;

    if( _GATS_IS_ENDOF_FIELD() )
    {
        ReachNextField();
        return eErrEmpty;	// The array is empty
    }

    *ppfArray = new float[0xFFFF];

    float*	pfArray = *ppfArray;
    char	szDigit[32];
    char*	pszDigit = szDigit;

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               wCount < 0xFFFF &&
               (pszDigit-szDigit < 31) ) // Make sure to not overflow buffer
        {
            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';
                    ASSERT_VALIDFLOAT(szDigit);
                    *pfArray++ = gatof(szDigit);
                    wCount++;
                    pszDigit = szDigit;
                }
                mSourceData++;
            }
            else
            if( (*mSourceData < '0' || *mSourceData > '9') &&
                (*mSourceData != 'e') && (*mSourceData != 'E') &&
                (*mSourceData != '.') && (*mSourceData != '-') &&
                (*mSourceData != '+') )
            {
                GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrBadArrayFormat;
            }
            else
                *pszDigit++ = *mSourceData++;
        }
        // Find a digit that is not a float?
        if(pszDigit-szDigit == 31)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else
        if( (*mSourceData != '\n') && (*mSourceData != '\r') && pszDigit != szDigit)
        {
            *pszDigit = '\0';
            ASSERT_VALIDFLOAT(szDigit);
            *pfArray++ = gatof(szDigit);
            wCount++;

            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);

    if( pszDigit != szDigit)
    {
        *pszDigit = '\0';
        ASSERT_VALIDFLOAT(szDigit);
        *pfArray = gatof(szDigit);
        wCount++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract the specific array found in PLR record (see ATDF specification).
//				 *** mSourceData must point to the first char of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Comments :
//      Memory is allocated by the function
//
// Argument(s) :
//      CHAL: First string that will receive arrays values
//		CHAR: Second string that will receive arrays values
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no PLR_CHAL_CHAR_Array field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrievePLR_CHAL_CHAR_Array(QString ** CHAL, QString ** CHAR, WORD wCount)
{
    // Check pointers
    if(CHAL==NULL || CHAR==NULL)
    {
        GSET_ERROR1(AtdfToStdf,eErrNullPointer,NULL,"AtdfToStdf::RetrievePLR_CHAL_CHAR_Array");
        return eErrNullPointer;
    }

    // Check if end of field
    if( _GATS_IS_ENDOF_FIELD() )
    {
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;	// The array is empty
    }

    // Get field value
    QString lValue;
    if(RetrieveField(lValue) != eErrOkay)
        return eErrInvalidArrayValue;

    QStringList qslAtdfCharChalList = lValue.split(QString("/"), QString::SkipEmptyParts);
    if(qslAtdfCharChalList.count()!=wCount)
        return eErrInvalidArrayValue;

    // Pointers should be NULL, else delete them, and reallocate
    if(*CHAL!=NULL)
        delete [] *CHAL;
    if(*CHAR!=NULL)
        delete [] *CHAR;
    *CHAL = new QString[wCount];
    *CHAR = new QString[wCount];

    // Go through all items
    for(int ii=0; ii<qslAtdfCharChalList.count(); ii++)
    {
        QStringList qslCharChalValueList = (qslAtdfCharChalList.at(ii)).split(QString(","), QString::SkipEmptyParts);
        int nValueListSize = qslCharChalValueList.count();
        if( (nValueListSize<0) || (nValueListSize>=GSIZE_DEFAULT) )
            return eErrBadArrayFormat;

        for(int jj=0; jj<qslCharChalValueList.count(); jj++)
        {
            QByteArray qbaValueListElt = (qslCharChalValueList.at(jj)).toLatin1();
            int nValueListEltSize = qbaValueListElt.size();

            if(nValueListEltSize == 1)
            {
                (*CHAL)[ii] = "";
                (*CHAR)[ii] += qbaValueListElt.at(0);
            }
            else if (nValueListEltSize>=2)
            {
                (*CHAL)[ii] += qbaValueListElt.at(0);
                (*CHAR)[ii] += qbaValueListElt.at(1);
            }
            else
                return eErrBadArrayFormat;
        }
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract a list of hexadecimal digits in ASCII format, and convert them
//				 to character values to a string.
//
// Argument(s) :
//			szString:	Receive the converted list of hex digits.
//			iMaxLength: The maximum string length.
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no Hex string field at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveHexString(char* szString)
{
    if( _GATS_IS_ENDOF_FIELD() )
    {
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;	// The array is empty
    }

    char	szBuffer[2];
    char*	pszString = szString;

    szBuffer[1] = '\0';

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData ) // Make sure to not overflow buffer
        {
            *szBuffer = *mSourceData++;
            ASSERT_VALIDHEXADECIMAL(szBuffer);
            *pszString++ = static_cast<char>(gahextodw(szBuffer));
        }

    } while (RecordContinueOnNextLine() == true);

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract the specific array of PMR index from the current position.
//				 *** mSourceData must point to the first character of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      GQTL_STDF::stdf_type_dn:	will contain the data retrieved from the array
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no array at the current position.
//			another code if an error occured.
// ----------------------------------------------------------------------------------------------------------
int	 AtdfToStdf::RetrieveIndexArray(GQTL_STDF::stdf_type_dn & dnField)
{
    dnField.Clear();

    // Check if we point to an end of record. If so, go to next atdf record
    if( _GATS_IS_ENDOF_FIELD() )
    {
        ReachNextField();
        return eErrEmpty;	// The array is empty
    }

    char	szDigit[5];
    char*	pszDigit = szDigit;
    WORD	wIndex;

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               (pszDigit-szDigit < 5) ) // Make sure to not overflow buffer
        {
            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';
                    ASSERT_VALIDUNSIGNED(szDigit);
                    wIndex = static_cast<WORD>(gatodw(szDigit));
                    // -1 because we need a 0 base index
                    //wIndex--;
                    ASSERT(wIndex >= 0);
                    div_t	lDiv = div( int(wIndex), 8 );
                    dnField.m_pBitField[lDiv.quot] |= (1 << lDiv.rem);
                    dnField.m_uiLength = qMax(dnField.m_uiLength, (unsigned int)(lDiv.quot+1));
                    pszDigit = szDigit;
                }
                mSourceData++;
            }
            else
                *pszDigit++ = *mSourceData++;
        }
        // Find a digit that is not a word?
        if(pszDigit-szDigit == 5)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else
        if((*mSourceData != '\n') && (*mSourceData != '\r') &&  pszDigit != szDigit)
        {
            *pszDigit = '\0';
            ASSERT_VALIDUNSIGNED(szDigit);
            wIndex = static_cast<WORD>(gatodw(szDigit));
            // -1 because we need a 0 base index
            //wIndex--;
            ASSERT(wIndex >= 0);
            div_t	lDiv = div( int(wIndex), 8 );
            dnField.m_pBitField[lDiv.quot] |= (1 << lDiv.rem);
            dnField.m_uiLength = qMax(dnField.m_uiLength, (unsigned int)(lDiv.quot+1));
            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);

    if(pszDigit != szDigit)
    {
        *pszDigit = '\0';
        ASSERT_VALIDUNSIGNED(szDigit);
        wIndex = static_cast<WORD>(gatodw(szDigit));
        // -1 because we need a 0 base index
        //wIndex--;
        ASSERT(wIndex >= 0);
        div_t	lDiv = div( int(wIndex), 8 );
        dnField.m_pBitField[lDiv.quot] |= (1 << lDiv.rem);
        dnField.m_uiLength = qMax(dnField.m_uiLength, (unsigned int)(lDiv.quot+1));
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Extract the specific array of B*n type from the current position.
//				 *** mSourceData must point to the first character of the array!
//				 When exiting this function, mSourceData point to the next field
//				 available or to the beginning of a new record header.
//
// Argument(s) :
//      GQTL_STDF::stdf_type_bn:	will contain the data retrieved from the field
//
// Return : eErrOkay is successful.
//			eErrEmpty if there is no array at the current position.
//			another code if an error occured.
//----------------------------------------------------------------------------------------------------------
int	 AtdfToStdf::RetrieveHexaString(GQTL_STDF::stdf_type_bn & bnField)
{
    // No test for buffer overflow (to maximize speed), so take room, just in case
    char            lString[1024], lTemp[3];
    int             lStatus=eErrOkay;
    unsigned int    lValue=0;

    lStatus = RetrieveField(lString, 1024);
    if(lStatus != eErrOkay)
        return lStatus;

    if(strlen(lString) == 0)
        return eErrEmpty;

    int     lIndex = strlen(lString)-1;
    while(lIndex >= 0)
    {
        // Initialize small temp string
        lTemp[0] = lTemp[1] = lTemp[2] = '\0';

        // Make sure we have a valid hex character
        if( (lString[lIndex] < '0' || lString[lIndex] > '9') &&
            (lString[lIndex] < 'A' || lString[lIndex] > 'F') &&
            (lString[lIndex] < 'a' || lString[lIndex] > 'f'))
        {
            GSET_ERROR2(AtdfToStdf,eErrInvalidHex,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrInvalidHex;
        }

        // We have 1 character
        lTemp[0] = lString[lIndex];
        --lIndex;

        // Check if we can get a second character
        if(lIndex >= 0)
        {
            // Make sure we have a valid hex character
            if( (lString[lIndex] < '0' || lString[lIndex] > '9') &&
                (lString[lIndex] < 'A' || lString[lIndex] > 'F') &&
                (lString[lIndex] < 'a' || lString[lIndex] > 'f'))
            {
                GSET_ERROR2(AtdfToStdf,eErrInvalidHex,NULL,mCurrentLineNbr,GetCurrentLineIndex());
                return eErrInvalidHex;
            }

            // We have second character
            lTemp[1] = lTemp[0];
            lTemp[0] = lString[lIndex];
            --lIndex;
        }

        // Now get decimal value corresponding to the ascii hex characters (1 or 2)
        if((sscanf(lTemp, "%x", &lValue) != 1) || (lValue > 255))
        {
            GSET_ERROR2(AtdfToStdf,eErrInvalidHex,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrInvalidHex;
        }

        // We have a correct value, add it to our byte array
        bnField.m_pBitField[bnField.m_bLength] = (char)(lValue & 0xff);
        ++bnField.m_bLength;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// Description : Call this function when you want to skip data in the current record, and
//				 set the position of mSourceData to the next record header.
//
// Return:
//			eErrOkay if mSourceData now point to the next record header.
//			eErrEndOfFile if the end of the file is reach.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::ReachNextRecordHeader()
{
    // If we are in a beginning to a record, nothing to do, the header is already reach!
    //bool bDebugTest( (mSourceData ==  m_pcStartOfData) || ( *(mSourceData-1) == '\n' && *mSourceData != ' ' ) );
    if( _GATS_IS_NEW_RECORD() )
        return eErrOkay;

    // Look for end of record
    while( *mSourceData != '\n' &&
           *mSourceData != '\r' &&
           mSourceData < mEndOfData )
        mSourceData++;

    // Reach end of file?
    if(mSourceData >= mEndOfData)
        return eErrEndOfFile; // Not an error; don't format anything

    // Skip CR and LF characters
    while( *mSourceData == '\n' ||
           *mSourceData == '\r' )
    {
        if(*mSourceData == '\n')
            mCurrentLineNbr++; // Only increment line number for '\n' char; so "\r\n" is correctly processed
        mSourceData++;
    }
    // Reach end of file?
    if(mSourceData >= mEndOfData)
        return eErrEndOfFile; // Not an error; don't format anything

    // The current character is a blank?
    if(*mSourceData == ' ')
        // The record is continuing on a new line, continue to search for the end...
        return ReachNextRecordHeader();

    // The current character is not blank, so it should be a new record header
    return eErrOkay;
}

// ----------------------------------------------------------------------------------------------------------
// Description : Use this function to reach the next field in the current record.
//
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::ReachNextField()
{
    for(;;)
    {
        if(mSourceData >= mEndOfData)
            return eErrEndOfFile;

        // If we have reach a separator
        if(*mSourceData == mSeparator)
        {
            mSourceData++;
            return eErrOkay;
        }
        else
        // If we have reach the end of a line
        if(*mSourceData == '\r' ||
           *mSourceData == '\n')
        {
            (void)RecordContinueOnNextLine();
            if(*mSourceData == mSeparator)
                mSourceData++;
            return eErrOkay;
        }
        else
        // If we have reach a new record definition
        if(_GATS_IS_NEW_RECORD())
            return eErrOkay;
        else
        // If we have reach the end of the file
        if(mSourceData >= mEndOfData)
            return eErrEndOfFile;
        else
            mSourceData++; // Skip current character and look for next one
    }
}


// ----------------------------------------------------------------------------------------------------------
// Description : Use this function to test if the record continue on the next line or not.
//				 You should call this function only if we have reach the end of the line:
//				 mSourceData point to a '\r' or '\n' character.
//
// Return type : true if the record continue -> mSourceData point to the first available
//					record character (on the same current record) in the new line.
//				 false * if the record does not continue (mSourceData point to the beginning
//					   of the next record).
//					   * If this function has been called, but mSourceData was not pointed
//					   to the end of a line (mSourceData has not been modified).
// ----------------------------------------------------------------------------------------------------------
bool AtdfToStdf::RecordContinueOnNextLine()
{
    if( *mSourceData != '\n' &&
        *mSourceData != '\r' )
        return false;

    // Skip CR and LF characters
    while( *mSourceData == '\n' ||
           *mSourceData == '\r' )
    {
        if(*mSourceData == '\n')
            mCurrentLineNbr++; // Only increment line number for '\n' char; so "\r\n" is correctly processed
        mSourceData++;

        if(mSourceData>=mEndOfData)
            return false;
    }

    if( *mSourceData == ' ' )
    {
        // The record continues on next line (ATDF spec: next line starts with a ' ')
        // Only remove first space. Next spaces are part of the continued field
        mSourceData++;
        return true;
    }

    return false; // Reach the beginning of a new record
}

// ----------------------------------------------------------------------------------------------------------
// Description : Make sure the current record definition is valid.
//				 mSourceData must point to the first record character name.
//
// Return : eErrOkay if the header is valid.
//			eErrPrematureEOF if we reach the end of file before the end of the header.
//			eErrBadRecordHeader if the header is not valid or corrupted
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::CheckRecordHeader()
{
    // Reach premature end of file?
    if(mSourceData + 4 > mEndOfData)
    {
        GSET_ERROR2(AtdfToStdf,eErrPrematureEOF,NULL,mCurrentLineNbr,GetCurrentLineIndex());
        return eErrPrematureEOF;
    }

    mSourceData += 3; // Skip record name
    if(*mSourceData++ == ':')
        return eErrOkay;
    else
    {
        GSET_ERROR2(AtdfToStdf,eErrBadRecordHeader,NULL,mCurrentLineNbr,GetCurrentLineIndex());
        return eErrBadRecordHeader; // Also skip the colon if exist
    }
}

// ----------------------------------------------------------------------------------------------------------
// Description: Extract a field from the current mSourceData position where the max size of the field is
//              unknown.
//.
//
// Arguments:
//				lField:	Buffer that received the field to extract
//
// Return: 	eErrOkay if successful; or eErrEmpty if tfield is empty
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveField(QString &lField)
{
    // Check pointers
    if(!mSourceData)
    {
        GSET_ERROR1(AtdfToStdf,eErrNullPointer,NULL,"AtdfToStdf::RetrieveField");
        return eErrNullPointer;
    }

    int    lFieldSize   = 1024;
    lField.reserve(lFieldSize);

    // Check if empty field
    if( _GATS_IS_ENDOF_FIELD() )
    {	// No field value at the current position
        if(*mSourceData == mSeparator)
            mSourceData++;

        return eErrEmpty;
    }

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData ) // Make sure to not overflow buffer
        {
            // For memory optimization, when string capacity is reached. Allocate another big block of memory
            // rather than reallocating for each append
            if(lField.size() == lField.capacity())
            {
                lField.reserve((lFieldSize *= 2));
            }

            // Copy byte
            lField.append(*mSourceData++);
        }

    } while (RecordContinueOnNextLine() == true);

    return eErrOkay;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Extract a field from the current mSourceData position.
//				If the field continue after a specified maximum size, an error is returned.
//
// Arguments:
//				szField:	Buffer that received the field to extract
//
// Return: 	eErrOkay if successful; or eErrBufferOverflow if the buffer cannot contain the
//			entire field.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::RetrieveField(char* Field, int MaxSize)
{
    char* pszField = Field;

    // Check pointers
    if(!mSourceData || !Field)
    {
        GSET_ERROR1(AtdfToStdf,eErrNullPointer,NULL,"AtdfToStdf::RetrieveField");
        return eErrNullPointer;
    }

    // Check if empty field
    if( _GATS_IS_ENDOF_FIELD() )
    {	// No field value at the current position
        if(*mSourceData == mSeparator)
            mSourceData++;
        return eErrEmpty;
    }

    // Check MaxSize
    if(MaxSize == 0)
    {
        GSET_ERROR1(AtdfToStdf, eErrBufferOverflow, NULL,"AtdfToStdf::RetrieveField");\
        return eErrBufferOverflow;
    }

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData ) // Make sure to not overflow buffer
        {
            // Check overflow
            if((pszField-Field) >= MaxSize)
            {
                GSET_ERROR1(AtdfToStdf, eErrBufferOverflow, NULL,"AtdfToStdf::RetrieveField");\
                return eErrBufferOverflow;
            }

            // Copy byte
            *pszField++ = *mSourceData++;
        }

    } while (RecordContinueOnNextLine() == true);

    // Check overflow
    if((pszField-Field) >= MaxSize)
    {
        GSET_ERROR1(AtdfToStdf, eErrBufferOverflow, NULL,"AtdfToStdf::RetrieveField");\
        return eErrBufferOverflow;
    }

    // Add end of string
    *pszField = '\0';

    return eErrOkay;
}


// ----------------------------------------------------------------------------------------------------------
// Description : Scal the specified value according given units.
//
// Argument(s) :
//      float& fNumber : The number to scal
//      char* szUnits : The unit used to scal the value
//		char& cScalValue: The scal factor applied to the number
//		bool bKeepUnit: if true, the szUnits argument will not be modified by this function.
//						if false, the unit prefix will be removed, if exists, from szUnits
//						string.
//
// ----------------------------------------------------------------------------------------------------------
void AtdfToStdf::ScalValue(float& fNumber, QString& lUnits, char& cScalValue, bool bKeepUnit /* = true*/)
{
    char	c = '\0';
    bool	bNothing = false;

    if (lUnits.length() > 0)
        c = lUnits.at(0).toLatin1();

    switch(c)
    {
        case 'f': // femto
            fNumber *= (float)1e-15; cScalValue = 15;	break;
        case 'p': // pico
            fNumber *= (float)1e-12; cScalValue = 12;	break;
        case 'n': // nano
            fNumber *= (float)1e-09; cScalValue = 9;	break;
        case 'u': // micro
            fNumber *= (float)1e-06; cScalValue = 6;	break;
        case 'm': // milli
            fNumber *= (float)1e-03; cScalValue = 3;	break;
        case '%': // percent
            // Keep Percent unit, only scale number.
            fNumber *= (float)1e-02;  cScalValue = 2;
            bNothing = true;
            break;
        case 'k': // kilo
        case 'K':
            fNumber *= (float)1e+03; cScalValue = -3;	break;
        case 'M': // mega
            fNumber *= (float)1e+06; cScalValue = -6;	break;
        case 'G': // giga
            fNumber *= (float)1e+09; cScalValue = -9;	break;
        case 'T': // tera
            fNumber *= (float)1e+12; cScalValue = -12;	break;
        default: // No unit prefix, so keep value and unit
            bNothing = true;  cScalValue = 0;	 break;
    }

    if(bNothing == false && bKeepUnit == true)
    {
        // Remove the first character which correspond to the scaling
        lUnits.remove(0, 1);
    }
}



// ----------------------------------------------------------------------------------------------------------
// Description: Retrieve the current position of mSourceData from the beginning of the current line.
//
// Return: The number of characters from the beginning of the line.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::GetCurrentLineIndex() const
{
    PCSTR	pcPos = mSourceData;
    int		iIndex = 0;

    while(pcPos > mStartOfData &&
          *pcPos != '\n')
    {
        iIndex++;
        pcPos--;
    }
    return iIndex;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Convert a string to a DWORD value (really faster than atoi() function)
//				*** WARNING: No control is performed about the string validity.
//
// Argument(s):
//      const char* szDWord: The string to convert.
//
// Return: A DWORD value.
// ----------------------------------------------------------------------------------------------------------
DWORD AtdfToStdf::gatodw(PCSTR szDWord)
{
    PCSTR	pszDWord = szDWord;
    DWORD	dwValue = 0;

    // Skip spaces if necessary (no '\t' on a ATDF file, so only control spaces)
    while( *pszDWord == ' ' )
        pszDWord++;

    // Get all digits
    while(*pszDWord >= '0' && *pszDWord <= '9')
    {
        dwValue = GMULTI10(dwValue);
        dwValue += *pszDWord++ - '0';
    }
    return dwValue;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Convert a string to a unsigned long long value
//				*** WARNING: No control is performed about the string validity.
//
// Argument(s):
//      const char* szNumber: The string to convert.
//
// Return: A unsigned long long value.
// ----------------------------------------------------------------------------------------------------------
unsigned long long AtdfToStdf::gatoull(PCSTR szNumber)
{
    PCSTR               pszNumber   = szNumber;
    unsigned long long	lValue     = 0;

    // Skip spaces if necessary (no '\t' on a ATDF file, so only control spaces)
    while( *pszNumber == ' ' )
        pszNumber++;

    // Get all digits
    while(*pszNumber >= '0' && *pszNumber <= '9')
    {
        lValue = GMULTI10(lValue);
        lValue += *pszNumber++ - '0';
    }

    return lValue;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Convert a string to a DWORD value (really faster than atoi() function)
//				The string represent a hexadecimal value.
//				*** WARNING: No control is performed about the string validity.
//
// Argument(s):
//      const char* szDWord: The string to convert.
//
// Return: A DWORD value.
// ----------------------------------------------------------------------------------------------------------
DWORD AtdfToStdf::gahextodw(PCSTR szDWord)
{
    PCSTR	pszDWord = szDWord;
    DWORD	dwValue = 0;

    // Skip spaces if necessary (no '\t' on a ATDF file, so only control spaces)
    while( *pszDWord == ' ' )
        pszDWord++;

    if(*pszDWord == 'X') // See if the optional 'X' field is present
        pszDWord++;	// Skip it

    // Get all digits
    while( (*pszDWord >= '0' && *pszDWord <= '9') ||
           (*pszDWord >= 'A' && *pszDWord <= 'F') ||
           (*pszDWord >= 'a' && *pszDWord <= 'f') )
    {
        while( (*pszDWord >= '0') && (*pszDWord <= '9') )
        {
            // GCORE-159 fix: even if this digit represents a decimal value, we are decoding
            // a hexadecimal number, so we have to multiply by 16, not by 10!!
            dwValue = GMULTI16(dwValue);
            dwValue += *pszDWord++ - '0';
        }
        while( *pszDWord >= 'A' && *pszDWord <= 'F' )
        {
            dwValue = GMULTI16(dwValue);
            dwValue += *pszDWord++ - 'A' + 10;
        }
        while( *pszDWord >= 'a' && *pszDWord <= 'f' )
        {
            dwValue = GMULTI16(dwValue);
            dwValue += *pszDWord++ - 'a' +10;
        }
    }
    return dwValue;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Convert a string to an integer value (really faster than atoi() function)
//				*** WARNING: No control is performed about the string validity.
//
// Argument(s):
//      const char* szInt: The string to convert.
//
// Return: A DWORD value.
// ----------------------------------------------------------------------------------------------------------
int AtdfToStdf::gatoi(PCSTR szInt)
{
    PCSTR	pszInt = szInt;
    int		iValue = 0;
    bool	bNegativeValue = false;

    // Skip spaces if necessary (no '\t' on a ATDF file, so only control spaces)
    while( *pszInt == ' ' )
        pszInt++;

    // Look for minus sign
    if(*pszInt == '-')
    {
        bNegativeValue = true;
        pszInt++;
    }

    // Get all digits
    while(*pszInt >= '0' && *pszInt <= '9')
    {
        iValue = GMULTI10(iValue);
        iValue += *pszInt++ - '0';
    }

    if(bNegativeValue == true)
        return -iValue;
    else
        return iValue;
}

// ----------------------------------------------------------------------------------------------------------
// Description: Convert a string to a float value (really faster than atof() function)
//				*** WARNING: No control is performed about the string validity.
//
// Argument(s):
//      const char* szFloat: The string to convert.
//
// Return: A float value.
// ----------------------------------------------------------------------------------------------------------
float AtdfToStdf::gatof(PCSTR szFloat)
{
    PCSTR	pszFloat = szFloat;
    float	fValue = 0.f;
    int		iExp = 0;
    bool	bNegativeValue = false,bNegativeExp = false;

    // Skip spaces if necessary (no '\t' on a ATDF file, so only control spaces)
    while( *pszFloat == ' ' )
        pszFloat++;
    // Look for minus sign
    if(*pszFloat == '-')
    {
        bNegativeValue = true;
        pszFloat++;
    }
    // Get all digits after the sign and before an optional point or exponent
    while(*pszFloat >= '0' && *pszFloat <= '9')
    {
        fValue = fValue*10.f;
        fValue += static_cast<float>(*pszFloat++ - '0');
    }
    // Find a point?
    if(*pszFloat == '.')
    {
        float	fMulti = 0.1f;
        pszFloat++; // Skip '.' character
        // If no digit follow the '.', considere the value after the '.' is zero
        while(*pszFloat >= '0' && *pszFloat <= '9')
        {
            fValue += fMulti * float(*pszFloat++ - '0');
            fMulti /= 10.f;
        }
    }

    // Find an exponant ?
    if((*pszFloat == 'e') || (*pszFloat == 'E'))
    {
        pszFloat++; // Skip exponant operator
        // Negative exponant ?
        if(*pszFloat == '-')
        {
            bNegativeExp = true;
            pszFloat++;	// Skip '-' sign
        }
        else
        if(*pszFloat == '+')
            pszFloat++; // Nothing special, just skip '+' sign

        // Extract the exponant value
        while(*pszFloat >= '0' && *pszFloat <= '9')
        {
            iExp = GMULTI10(iExp);
            iExp += *pszFloat++ - '0';
        }

        if(bNegativeExp == true)
        {
            for(int i=0;i<iExp;i++)
                fValue /= 10.f;
        }
        else
        {
            for(int i=0;i<iExp;i++)
                fValue *= 10.f;
        }
    }

    if(bNegativeValue == true)
        return -fValue;
    else
        return fValue;
}

int AtdfToStdf::AnalyzeVUR()
{
    bool		bContinue = true;
    int			iStatus;
    int			iCurrentField = 0;
    QString     lUPDName;
    GQTL_STDF::Stdf_VUR_V4	atrData;

    do
    {
        switch(iCurrentField)
        {
            case 0: // UPD_NAM
                iStatus = RetrieveString(lUPDName);
                if(_GATS_ERROR_RETRIEVE(iStatus))
                    return iStatus;
                atrData.SetUPD_NAM(lUPDName);
                break;
            default:
                bContinue = false;
                break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop ATR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(atrData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf:: AnalyzeCNR()
{
    bool		bContinue = true;
    int			iStatus;
    int			iCurrentField = 0;
    QString     lString;
    stdf_type_u2 lChnNum;
    stdf_type_u4 lBitPos;
    GQTL_STDF::Stdf_CNR_V4	atrData;

    do
    {
        switch(iCurrentField)
        {
            case 0: // CHN_NUM
                iStatus = RetrieveWord(lChnNum);
                if(_GATS_ERROR_RETRIEVE(iStatus))
                    return iStatus;
                atrData.SetCHN_NUM(lChnNum);
                break;

            case 1: // BIT_POS
                iStatus = RetrieveDWord(lBitPos);
                if(_GATS_ERROR_RETRIEVE(iStatus))
                    return iStatus;
                atrData.SetBIT_POS(lBitPos);
                break;

            case 2: // CELL_NAM
                iStatus = RetrieveString(lString);
                if(_GATS_ERROR_RETRIEVE(iStatus))
                    return iStatus;
                atrData.SetCELL_NAM(lString);
                break;

            default:
                bContinue = false;
                break;
        }
        iCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop ATR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(atrData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf::AnalyzeSTR()
{
    bool            lContinue = true;
    int             lStatus;
    int             lCurrentField = 0;
    stdf_type_u1    lU1Type;
    stdf_type_u2    lU2Type;
    stdf_type_u4    lU4Type;
    stdf_type_u8    lU8Type;
    stdf_type_cn    lCnType;
    QStringList     lStringList;
    WORD            lCount;
    GQTL_STDF::stdf_type_dn lDnType;
    GQTL_STDF::Stdf_STR_V4	lSTRData;

    do
    {
        switch(lCurrentField)
        {
            case 0: // CONT_FLG
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCONT_FLG(lU1Type);
                break;

            case 1: // TEST_NUM
                lStatus = RetrieveDWord(lU4Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetTEST_NUM(lU4Type);
                break;

            case 2: // HEAD_NUM
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetHEAD_NUM(lU1Type);
                break;

            case 3: // SITE_NUM
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetSITE_NUM(lU1Type);
                break;

            case 4: // PSR_REF
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetPSR_REF(lU2Type);
                break;

            case 5: // TEST_FLG
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetTEST_FLG(lU1Type);
                break;

            case 6: // LOG_TYP
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetLOG_TYP(lCnType);
                break;

            case 7: // TEST_TXT
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetTEST_TXT(lCnType);
                break;

            case 8: // ALARM_ID
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetALARM_ID(lCnType);
                break;

            case 9: // PROG_TXT
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetPROG_TXT(lCnType);
                break;

            case 10: // RSLT_TXT
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetRSLT_TXT(lCnType);
                break;

            case 11: // Z_VAL
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetZ_VAL(lU1Type);
                break;

            case 12: // FMU_FLG
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetFMU_FLG(lU1Type);
                break;

            case 13: // MASK_MAP
                lStatus = RetrieveIndexArray(lDnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetMASK_MAP(lDnType);
                break;

            case 14: // FAL_MAP
                lStatus = RetrieveIndexArray(lDnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetFAL_MAP(lDnType);
                break;

            case 15: // CYC_CNT
                lStatus = RetrieveULongLong(lU8Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCYC_CNT(lU8Type);
                break;

            case 16: // TOTF_CNT
                lStatus = RetrieveDWord(lU4Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetTOTF_CNT(lU4Type);
                break;

            case 17: // TOTL_CNT
                lStatus = RetrieveDWord(lU4Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetTOTL_CNT(lU4Type);
                break;

            case 18: // CYC_BASE
                lStatus = RetrieveULongLong(lU8Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCYC_BASE(lU8Type);
                break;

            case 19: // BIT_BASE
                lStatus = RetrieveDWord(lU4Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetBIT_BASE(lU4Type);
                break;

            case 20: // COND_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCOND_CNT(lU2Type);
                break;

            case 21: // LIM_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetLIM_CNT(lU2Type);
                break;

            case 22: // CYC_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCYC_SIZE(lU1Type);
                break;

            case 23: // PMR_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetPMR_SIZE(lU1Type);
                break;

            case 24: // CHN_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCHN_SIZE(lU1Type);
                break;

            case 25: // PAT_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetPAT_SIZE(lU1Type);
                break;

            case 26: // BIT_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetBIT_SIZE(lU1Type);
                break;

            case 27: // U1_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetU1_SIZE(lU1Type);
                break;

            case 28: // U2_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetU2_SIZE(lU1Type);
                break;

            case 29: // U3_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetU3_SIZE(lU1Type);
                break;

            case 30: // UTX_SIZE
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetUTX_SIZE(lU1Type);
                break;

            case 31: // CAP_BGN
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCAP_BGN(lU2Type);
                break;

            case 32: // LIM_INDX
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu2LIM_INDX, lSTRData.m_u2LIM_CNT, RetrieveWordArray, lSTRData.SetLIM_INDX);
                break;

            case 33: // LIM_SPEC
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu4LIM_SPEC, lSTRData.m_u2LIM_CNT, RetrieveDWordArray, lSTRData.SetLIM_SPEC);
                break;

            case 34: // COND_LST
                lStatus = RetrieveStringArray(lStringList, lSTRData.m_u2COND_CNT);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                for (int lIdx = 0; lIdx < lStringList.count(); ++lIdx)
                    lSTRData.SetCOND_LST(lIdx, lStringList.at(lIdx));
                break;

            case 35: // CYCO_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCYCO_CNT(lU2Type);
                break;

            case 36: // CYC_OFFSET
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8CYC_OFST, lSTRData.m_u2CYCO_CNT, RetrieveULongLongArray, lSTRData.SetCYC_OFST);
                break;

            case 37: // PMR_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetPMR_CNT(lU2Type);
                break;

            case 38: // PMR_INDX
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8PMR_INDX, lSTRData.m_u2PMR_CNT, RetrieveULongLongArray, lSTRData.SetPMR_INDX);
                break;

            case 39: // CHN_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCHN_CNT(lU2Type);
                break;

            case 40: // CHN_NUM
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8CHN_NUM, lSTRData.m_u2CHN_CNT, RetrieveULongLongArray, lSTRData.SetCHN_NUM);
                break;

            case 41: // EXP_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetEXP_CNT(lU2Type);
                break;

            case 42: // EXP_DATA
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu1EXP_DATA, lSTRData.m_u2EXP_CNT, RetrieveByteArray, lSTRData.SetEXP_DATA);
                break;

            case 43: // CAP_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetCAP_CNT(lU2Type);
                break;

            case 44: // CAP_DATA
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu1CAP_DATA, lSTRData.m_u2CAP_CNT, RetrieveByteArray, lSTRData.SetCAP_DATA);
                break;

            case 45: // NEW_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetNEW_CNT(lU2Type);
                break;

            case 46: // NEW_DATA
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu1NEW_DATA, lSTRData.m_u2NEW_CNT, RetrieveByteArray, lSTRData.SetNEW_DATA);
                break;

            case 47: // PAT_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetPAT_CNT(lU2Type);
                break;

            case 48: // PAT_NUM
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8PAT_NUM, lSTRData.m_u2PAT_CNT, RetrieveULongLongArray, lSTRData.SetPAT_NUM);
                break;

            case 49: // BPOS_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetBPOS_CNT(lU2Type);
                break;

            case 50: // BIT_POS
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8BIT_POS, lSTRData.m_u2BPOS_CNT, RetrieveULongLongArray, lSTRData.SetBIT_POS);
                break;

            case 51: // USR1_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetUSR1_CNT(lU2Type);
                break;

            case 52: // USR1
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8USR1, lSTRData.m_u2USR1_CNT, RetrieveULongLongArray, lSTRData.SetUSR1);
                break;

            case 53: // USR2_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetUSR2_CNT(lU2Type);
                break;

            case 54: // USR2
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8USR2, lSTRData.m_u2USR2_CNT, RetrieveULongLongArray, lSTRData.SetUSR2);
                break;

            case 55: // USR3_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetUSR3_CNT(lU2Type);
                break;

            case 56: // USR3
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lSTRData.m_pu8USR3, lSTRData.m_u2USR3_CNT, RetrieveULongLongArray, lSTRData.SetUSR3);
                break;

            case 57: // TXT_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSTRData.SetTXT_CNT(lU2Type);
                break;

            case 58: // USER_TXT
                _GATS_READ_CN_ARRAY_FIELD(lStatus, lStringList, lSTRData.m_u2TXT_CNT, lSTRData.SetUSR_TXT);
                break;

            default:
                lContinue = false;
                break;

        }
        lCurrentField++;

        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop ATR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (lContinue == true));

    _GATS_WRITE_RECORD(lSTRData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf::AnalyzePSR()
{
    bool            bContinue = true;
    int             lStatus;
    int             iCurrentField = 0;
    stdf_type_u1    lU1Type;
    stdf_type_u2    lU2Type;
    stdf_type_cn    lCnType;
    QStringList     lStringList;
    WORD            lCount;
    GQTL_STDF::Stdf_PSR_V4	lPSRData;

    do
    {
        switch(iCurrentField)
        {
            case 0: // CONT_FLG
                lStatus = RetrieveByte(lU1Type, false);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lPSRData.SetCONT_FLG(lU1Type);
                break;

            case 1: // PSR_INDX
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lPSRData.SetPSR_INDX(lU2Type);
                break;

            case 2: // PSR_NAM
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lPSRData.SetPSR_NAM(lCnType);
                break;

            case 3: // OPT_FLG
                lStatus = RetrieveByte(lU1Type, false);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lPSRData.SetOPT_FLG(lU1Type);
                break;

            case 4: // TOTP_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lPSRData.SetTOTP_CNT(lU2Type);
                break;

            case 5: // LOCP_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lPSRData.SetLOCP_CNT(lU2Type);
                break;

            case 6: // PAT_BGN
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lPSRData.m_pu8PAT_BGN, lPSRData.m_u2LOCP_CNT, RetrieveULongLongArray, lPSRData.SetPAT_BGN);
                break;

            case 7: // PAT_END
                _GATS_READ_Ux_ARRAY_FIELD(lStatus, lCount, lPSRData.m_pu8PAT_END, lPSRData.m_u2LOCP_CNT, RetrieveULongLongArray, lPSRData.SetPAT_END);
                break;

            case 8: // PAT_FILE
                _GATS_READ_CN_ARRAY_FIELD(lStatus, lStringList, lPSRData.m_u2LOCP_CNT, lPSRData.SetPAT_FILE);
                break;

            case 9: // PAT_LBL
                _GATS_READ_CN_ARRAY_FIELD(lStatus, lStringList, lPSRData.m_u2LOCP_CNT, lPSRData.SetPAT_LBL);
                break;

            case 10: // FILE_UID
                _GATS_READ_CN_ARRAY_FIELD(lStatus, lStringList, lPSRData.m_u2LOCP_CNT, lPSRData.SetFILE_UID);
            break;

            case 11: // ATPG_DSC
                _GATS_READ_CN_ARRAY_FIELD(lStatus, lStringList, lPSRData.m_u2LOCP_CNT, lPSRData.SetATPG_DSC);
            break;

            case 12: // SRC_ID
                _GATS_READ_CN_ARRAY_FIELD(lStatus, lStringList, lPSRData.m_u2LOCP_CNT, lPSRData.SetSRC_ID);
                break;

            default:
                bContinue = false;
                break;
        }
        iCurrentField++;

        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop ATR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (bContinue == true));

    _GATS_WRITE_RECORD(lPSRData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf::AnalyzeSSR()
{
    bool            lContinue = true;
    int             lStatus;
    int             lCurrentField = 0;
    WORD            lCount;
    stdf_type_cn    lCnType;
    stdf_type_u2    lU2Type;
    GQTL_STDF::Stdf_SSR_V4	lSSRData;

    do
    {
        switch(lCurrentField)
        {
            case 0: // SSR_NAM
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSSRData.SetSSR_NAM(lCnType);
                break;

            case 1: // CHN_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lSSRData.SetCHN_CNT(lU2Type);
                break;

            case 2: // CHN_LIST
                lStatus = RetrieveWordArray(lCount, &lSSRData.m_pu2CHN_LIST);
                if(lStatus != eErrOkay)
                    return lStatus;

                if(lCount != lSSRData.m_u2CHN_CNT)
                {
                    GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(lSSRData.m_u2CHN_CNT),int(lCount));
                    return eErrInvalidArraySize;
                }

                for (stdf_type_u2 lIdx = 0; lIdx < lSSRData.m_u2CHN_CNT; ++lIdx)
                    lSSRData.SetCHN_LIST(lIdx, lSSRData.m_pu2CHN_LIST[lIdx]);
                break;

            default:
                lContinue = false;
                break;
        }
        lCurrentField++;

        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop NMR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (lContinue == true));

    _GATS_WRITE_RECORD(lSSRData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf::AnalyzeNMR()
{
    bool            lContinue = true;
    int             lStatus;
    int             lCurrentField = 0;
    WORD            lCount;
    stdf_type_u1    lU1Type;
    stdf_type_u2    lU2Type;
    QStringList     lStringList;
    GQTL_STDF::Stdf_NMR_V4	lNMRData;

    do
    {
        switch(lCurrentField)
        {
            case 0: // CONT_FLG
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lNMRData.SetCONT_FLG(lU1Type);
                break;

            case 1: // NMR_INDX
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lNMRData.SetNMR_INDX(lU2Type);
                break;

            case 2: // TOT_MCNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lNMRData.SetTOTM_CNT(lU2Type);
                break;

            case 3: // LOC_MCNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lNMRData.SetLOCM_CNT(lU2Type);
                break;

            case 4: // PMR_INDX
                lStatus = RetrieveWordArray(lCount, &lNMRData.m_pu2PMR_INDX);
                if(lStatus != eErrOkay)
                    return lStatus;

                if(lCount != lNMRData.m_u2LOCM_CNT)
                {
                    GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(lNMRData.m_u2LOCM_CNT),int(lCount));
                    return eErrInvalidArraySize;
                }

                for (stdf_type_u2 lIdx = 0; lIdx < lNMRData.m_u2LOCM_CNT; ++lIdx)
                    lNMRData.SetPMR_INDX(lIdx, lNMRData.m_pu2PMR_INDX[lIdx]);
                break;

            case 5: // ATPG_NAM
                lStatus = RetrieveStringArray(lStringList, lNMRData.m_u2LOCM_CNT);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                for (int lIdx = 0; lIdx < lStringList.count(); ++lIdx)
                    lNMRData.SetATPG_NAM(lIdx, lStringList.at(lIdx));
                break;

            default:
                lContinue = false;
                break;
        }
        lCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop NMR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (lContinue == true));

    _GATS_WRITE_RECORD(lNMRData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf::AnalyzeCDR()
{
    bool            lContinue = true;
    int             lStatus;
    int             lCurrentField = 0;
    WORD            lCount;
    stdf_type_cn    lCnType;
    stdf_type_u1    lU1Type;
    stdf_type_u2    lU2Type;
    stdf_type_u4    lU4Type;
    QStringList     lStringList;
    GQTL_STDF::Stdf_CDR_V4	lCDRData;

    do
    {
        switch(lCurrentField)
        {
            case 0: // CONT_FLG
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lCDRData.SetCONT_FLG(lU1Type);
                break;

            case 1: // CDR_INDX
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lCDRData.SetCDR_INDX(lU2Type);
                break;

            case 2: // CHN_NAM
                lStatus = RetrieveString(lCnType);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lCDRData.SetCHN_NAM(lCnType);
                break;

            case 3: // CHN_LEN
                lStatus = RetrieveDWord(lU4Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lCDRData.SetCHN_LEN(lU4Type);
                break;

            case 4: // SIN_PIN
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lCDRData.SetSIN_PIN(lU2Type);
                break;

            case 5: // SOUT_PIN
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;
                lCDRData.SetSOUT_PIN(lU2Type);
                break;

            case 6: // MSTR_CNT
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                lCDRData.SetMSTR_CNT(lU1Type);
                break;

            case 7: // M_CLKS
                lStatus = RetrieveWordArray(lCount, &lCDRData.m_pu2M_CLKS);
                if(lStatus != eErrOkay)
                    return lStatus;

                if(lCount != lCDRData.m_u1MSTR_CNT)
                {
                    GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(lCDRData.m_u1MSTR_CNT),int(lCount));
                    return eErrInvalidArraySize;
                }

                for (stdf_type_u2 lIdx = 0; lIdx < lCDRData.m_u1MSTR_CNT; ++lIdx)
                    lCDRData.SetM_CLKS(lIdx, lCDRData.m_pu2M_CLKS[lIdx]);
                break;

            case 8: // SLAV_CNT
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                lCDRData.SetSLAV_CNT(lU1Type);
                break;

            case 9: // S_CLKS
                lStatus = RetrieveWordArray(lCount, &lCDRData.m_pu2S_CLKS);
                if(lStatus != eErrOkay)
                    return lStatus;

                if(lCount != lCDRData.m_u1SLAV_CNT)
                {
                    GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(),int(lCDRData.m_u1SLAV_CNT),int(lCount));
                    return eErrInvalidArraySize;
                }

                for (stdf_type_u2 lIdx = 0; lIdx < lCDRData.m_u1SLAV_CNT; ++lIdx)
                    lCDRData.SetS_CLKS(lIdx, lCDRData.m_pu2S_CLKS[lIdx]);
                break;

            case 10: // INV_VAL
                lStatus = RetrieveByte(lU1Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                lCDRData.SetINV_VAL(lU1Type);
                break;

            case 11: // LST_CNT
                lStatus = RetrieveWord(lU2Type);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                lCDRData.SetLST_CNT(lU2Type);
                break;

            case 12: // CELL_LST
                lStatus = RetrieveStringArray(lStringList, lCDRData.m_u2LST_CNT);
                if(_GATS_ERROR_RETRIEVE(lStatus))
                    return lStatus;

                for (int lIdx = 0; lIdx < lStringList.count(); ++lIdx)
                    lCDRData.SetCELL_LST(lIdx, lStringList.at(lIdx));
                break;

            default:
                lContinue = false;
                break;
        }

        lCurrentField++;
        // If we have reach a new record header,
        if(_GATS_IS_NEW_RECORD())
            break; // Stop NMR analyze, write data extracted to STDF file

    } while ( !((*mSourceData == '\r' || *mSourceData == '\n') && RecordContinueOnNextLine() == false) && (lContinue == true));

    _GATS_WRITE_RECORD(lCDRData,mStdf);

    return ReachNextRecordHeader();
}

int AtdfToStdf::RetrieveStringArray(QStringList& StringList, WORD nCount)
{
    // If string list not empty, clear it
    StringList.clear();

    if( _GATS_IS_ENDOF_FIELD() )
    {
        ReachNextField();
        return eErrEmpty;	// The array is empty
    }

    char	szDigit[256];
    char*	pszDigit = szDigit;
    int     lItem    = 0;

    do
    {
        while( *mSourceData !=  mSeparator &&
               *mSourceData != '\n' &&
               *mSourceData != '\r' &&
               mSourceData < mEndOfData &&
               lItem < nCount &&
               (pszDigit-szDigit < 255) ) // Make sure to not overflow buffer
        {
            if(*mSourceData == ',')
            {
                if(pszDigit != szDigit) // This expression is false if we find a ',' in the beginning
                {						// of a new line, just after a ' ' (record on 2 lines).
                    *pszDigit = '\0';
                    StringList.append(szDigit);
                    lItem++;
                    pszDigit = szDigit;
                }
                mSourceData++;
            }
            else
                *pszDigit++ = *mSourceData++;
        }

        //
        if (lItem >= nCount)
        {
            GSET_ERROR4(AtdfToStdf,eErrInvalidArraySize,NULL,mCurrentLineNbr,GetCurrentLineIndex(), nCount, lItem);
            return eErrInvalidArraySize;
        }
        else if(pszDigit-szDigit == 255)
        {
            GSET_ERROR2(AtdfToStdf,eErrBadArrayFormat,NULL,mCurrentLineNbr,GetCurrentLineIndex());
            return eErrBadArrayFormat;
        }
        else if( (*mSourceData != '\n') && (*mSourceData != '\r') && pszDigit != szDigit)
        {
            *pszDigit = '\0';
            StringList.append(szDigit);
            lItem++;
            pszDigit = szDigit;
        }

    } while (RecordContinueOnNextLine() == true);

    if( pszDigit != szDigit )
    {
        *pszDigit = '\0';
        StringList.append(szDigit);
        lItem++;
    }

    return ReachNextField();
}

// ----------------------------------------------------------------------------------------------------------
// FUNCTIONS ONLY AVAILABLE IN DEBUG MODE
// ----------------------------------------------------------------------------------------------------------

#ifdef _DEBUG

// Verify that the specified string is a valid integer number
void AtdfToStdf::_AssertValidInteger(const char* szInteger)
{
    const char* pcInteger = szInteger;

    while(*pcInteger != '\0')
    {
        if( !isdigit(*pcInteger) && *pcInteger != '-' )
            ASSERT(false);

        pcInteger++;
    }
}

// Verify that the specified string is a valid unsigned integer number
void AtdfToStdf::_AssertValidUnsigned(const char* szUnsigned)
{
    const char* pcUnsigned = szUnsigned;

    while(*pcUnsigned != '\0')
    {
        if( !isdigit(*pcUnsigned) )
            ASSERT(false);

        pcUnsigned++;
    }
}

// Verify that the specified string is a valid hexadecimal number
void AtdfToStdf::_AssertValidHexadecimal(const char* szHexDecimal)
{
    const char* pcHexDecimal = szHexDecimal;

    while(*pcHexDecimal != '\0')
    {
        if( !isdigit(*pcHexDecimal) &&
            !( (*pcHexDecimal >= 'A') && (*pcHexDecimal <= 'F') )  &&
            !( (*pcHexDecimal >= 'a') && (*pcHexDecimal <= 'f') ) )
            ASSERT(false);

        pcHexDecimal++;
    }

}

// Verify that the specified string is a valid floating point number
void AtdfToStdf::_AssertValidFloat(const char* szFloat)
{
    const char* pcFloat = szFloat;

    while(*pcFloat != '\0')
    {
        if(!isdigit(*pcFloat) && (*pcFloat != '.') && (*pcFloat != 'e') &&
           (*pcFloat != 'E') && (*pcFloat != '-') && (*pcFloat != '+') )
            ASSERT(false);

        pcFloat++;
    }

}

#endif // _DEBUG

} //namespace Parser

} //namespace GS


