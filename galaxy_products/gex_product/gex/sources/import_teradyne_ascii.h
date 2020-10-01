/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Notes :                                                                              */
/*                                                                                      */
/****************************************************************************************/

///////////////////////////////////////////////////////////
// CGTERADYNE_ASCIItoSTDF class HEADER :
// This class contains a converter from the teradyne ascii
// format to stdf v4 format
///////////////////////////////////////////////////////////
#ifndef GEX_IMPORT_TERADYNE_ASCII_H
#define GEX_IMPORT_TERADYNE_ASCII_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "gex_constants.h"
#include "stdf.h"
#include "stdfrecords_v4.h"


/**
 * \struct FTRFields
 * \brief structure contains all FTR fields.
 *
 * In some cases we have to store the list of the finded FTR record.
 * We have to concatenate the TEST_TXT with a suffix if it is found.
 */
struct FTRFields
{
    stdf_type_u4	mTEST_NUM;
    stdf_type_u1	mHEAD_NUM;
    stdf_type_u1	mSITE_NUM;
    stdf_type_b1	mTEST_FLG;
    stdf_type_b1	mOPT_FLAG;
    stdf_type_u4	mCYCL_CNT;
    stdf_type_cn	mTEST_TXT;
    stdf_type_cn	mVECT_NAM;
};


///////////////////////////////////////////////////////////
/// PTRFieldsPositions structure to store the positions of
///                     each table's field
///////////////////////////////////////////////////////////
/**
* \struct PTRFieldsPositions
* \brief Stores the positions of each table's field.
*
* To read the input file, we have to find the position of each field in the table.
*/
struct PTRFieldsPositions
{
     int mNumber;
     int mSite;
     int mResult;
     int mTestName;
     int mPin;
     int mChannel;
     int mLow;
     int mMeasured;
     int mHigh;
     int mForce;
     int mLoc;
     int mPattern;
};


/*! \class CGTERADYNE_ASCIItoSTDF
  * \brief class representing the Teradyne ascii file
  *
  *  The class supports the reading of the teradyne ascii file and writing the corresponding
  *  stdf file
  */
class CGTERADYNE_ASCIItoSTDF
{
public:
    /**
    * \fn CGTERADYNE_ASCIItoSTDF()
    * \brief constructure
    */

    CGTERADYNE_ASCIItoSTDF();

    /**
     * \fn bool	Convert(const char *lTeradyneAsciiFileName, const char *lFileNameSTDF)
     * \brief Function to convert the input file to stdf.
     * \param lTeradyneAsciiFileName is the input file, contains the teradyne ascii file.
     * \param lFileNameSTDF is the output file, contains the stdf file.
     * \return true if the convertion is correct. Otherwise return false.
     */
    bool	Convert(const char *lTeradyneAsciiFileName, const char *lFileNameSTDF);

    /**
     * \fn QString GetLastError()
     * \brief return the last founded error.
     * \return String containing the last founded error.
     */
	QString GetLastError();

    /**
     * \fn static bool	IsCompatible(const char *lFileName)
     * \brief static function called by the import all to check if the input file is compatible with the teradyne
     *          ascii format.
     * \param lFileName is the input file, contains the teradyne ascii file.
     * \return true if the input file is coompatible with the teradyne format. Otherwise return false.
     */
    static bool	IsCompatible(const char *lFileName);

private:
    /**
     * \fn void NormalizeValues(QString &lStrUnits, float &lValue, int&lScale, bool &lIsNumber)
     * \brief Normalize the value of the value
     * \param lStrUnits unit string.
     * \param lValue returned value.
     * \param lScale scale value.
     * \param lIsNumber true if the value is a number.
     */
    void NormalizeValues(QString &lStrUnits, float &lValue, int& lScale, bool &lIsNumber);

    /**
     * \fn bool ReadTeradyneAsciiFile(const char *lTeradyneAsciiFileName,const char*lFileNameSTDF)
     * \brief Read the teradyne ascii file.
     * \param lTeradyneAsciiFileName is the input file, contains the teradyne ascii file.
     * \param lFileNameSTDF is the output file, contains the stdf file.
     * \return true if the file has been correctly read. Otherwise return false.
     */
    bool ReadTeradyneAsciiFile(const char *lTeradyneAsciiFileName, const char *lFileNameSTDF);

    /**
     * \fn bool WriteStdfFile(QTextStream &lTeradyneAsciiFile, const char *lFileNameSTDF)
     * \brief Write the stdf file.
     * \param lTeradyneAsciiFile is the input file, contains the teradyne ascii file.
     * \param lFileNameSTDF is the output file, contains the stdf file.
     * \return true if the stdf file has been correctly written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &lTeradyneAsciiFile, const char *lFileNameSTDF);

    /**
     * \fn QString ReadLine(QTextStream& lFile)
     * \brief Read line : skip empty line.
     * \param lFile is the input file.
     * \return the line that has been read.
     */
    QString ReadLine(QTextStream& lFile);

    /**
     * \fn bool WritePTR(QString lStrString, GS::StdLib::Stdf &lStdfFile, QList<stdf_type_u1> &lSiteList)
     * \brief Write the the PTR record.
     * \param lStrString is the input string. A line from the input file.
     * \param lStdfFile is the output file, contains the stdf file.
     * \param lSiteList list of sites.
     * \return true if the stdf PTR record has been correctly written. Otherwise return false.
     */
    bool WritePTR(QString lStrString, GS::StdLib::Stdf &lStdfFile, QList<stdf_type_u1> &lSiteList);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
    int		mProgressStep;
    int		mFileSize;
    int		mNextFilePos;


    QString mStrLastError;					// Holds last error string during ADVANTEST_T2000->STDF convertion
    int		mIdLastError;					// Holds last error ID
	enum  errCodes
	{
		errNoError,							// No erro (default)
		errOpenFail,						// Failed Opening ADVANTEST_T2000 file
		errInvalidFormat,					// Invalid ADVANTEST_T2000 format
		errLicenceExpired,					// File date out of Licence window!
		errWriteSTDF						// Failed creating STDF intermediate file
	};

    long		mStartTime;				// Startup time

    GQTL_STDF::Stdf_MIR_V4	mStdfMIRv4;     // STDF MIR record instance
    GQTL_STDF::Stdf_MRR_V4	mStdfMRRv4;     // STDF MRR record instance
    GQTL_STDF::Stdf_SDR_V4	mStdfSDRv4;     // STDF SDR record instance
    GQTL_STDF::Stdf_PTR_V4	mStdfPTRv4;     // STDF PTR record instance
    GQTL_STDF::Stdf_FTR_V4	mStdfFTRv4;     // STDF FTR record instance
    GQTL_STDF::Stdf_PIR_V4	mStdfPIRv4;     // STDF PIR record instance
    GQTL_STDF::Stdf_PRR_V4	mStdfPRRv4;     // STDF PRR record instance
    GQTL_STDF::Stdf_FAR_V4	mStdfFARv4;     // STDF FAR record instance
    GQTL_STDF::Stdf_ATR_V4	mStdfATRv4;     // STDF ATR record instance

    PTRFieldsPositions      mPtrFields;     // conatins the position of PTR' fields
};


#endif
