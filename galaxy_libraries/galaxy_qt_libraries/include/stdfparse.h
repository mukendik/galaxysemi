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
// StdfParse class HEADER :
// This class contains high-level routines to retrieve
// information from an stdf file
///////////////////////////////////////////////////////////

#ifndef _StdfParse_h_
#define _StdfParse_h_

 // Galaxy modules includes
#include <gstdl_errormgr.h>

#include "stdf.h"
#include "stdfrecord.h"
#include "stdfrecords_v4.h"
#include "stdfrecords_v3.h"
#include "stdf_head_and_site_number_decipher.h"

// Some constant definition
// STDF version
#define STDF_V_UNKNOWN			0
#define STDF_V_3				3
#define STDF_V_4				4

class QString;
class QStringList;

namespace GQTL_STDF
{
// StdfParse class definition
class StdfParse
{
// METHODS
public:
    GDECLARE_ERROR_MAP(StdfParse)
    {
        eFileClosed,			// STDF File not opened (so access not allowed)
        eErrorOpen,				// Error opening/creating file
        eRecordCorrupted,		// File corrupted...unexpected end of file in record
        eFileCorrupted,			// File corrupted...other reasons
        eRecordNotFound,		// Couldn't find specified record
        eUnexpectedRecord,		// Unexpected record found in STDF file
        eEndOfFile,				// End of file reached.
        eErrorRead,				// Error reading record
        eBadRecordLoaded,		// Request to read a record type that is not currently loaded
        eBadStdfVersion,		// Bad STDF version (this parser supports only STDF V4)
        eErrorDumpRecord		// Error dumping record
    }
    GDECLARE_END_ERROR_MAP(StdfParse)

    // Constructor / destructor functions
    StdfParse();
    ~StdfParse();
    // Opens STDF file
    bool	Open(const char *szFileName, int iAccessMode = STDF_READ, int iForce_CPU_Type = -1);
    // Closes STDF file
    void	Close(void);
    // Rewinds STDF file
    bool	Rewind(void);
    // Get Version of STDF file (this parser supports only STDF V4)
    bool	GetVersion(int *nVersion);
    // Get CPU type of STDF file
    bool	GetCpuType(int *nCpuType);
    // Get STDF file name
    bool	GetFileName(QString *pstrFileName);
    // Load Next record and return informations about record type/subtype
    int		LoadNextRecord(int *nRecordType, const bool bRewind = false);
    // Load Next record of specific type/subtype
    int		LoadNextRecord(const int nRecordType, const bool bRewind = false);
    // Return record type & subtype
    int     GetRecordType(int *nRecordType,int *nRecordSubType);
    // Read STDF Record
    bool	ReadRecord(Stdf_Record* pclStdfRecord);
    // Write STDF Record
    bool	WriteRecord(Stdf_Record* pclStdfRecord);
    // Get handle on STDF file
    bool	GetStdfHandle(GS::StdLib::Stdf **ppclStdfHandle);
    // Dump current STDF record to a destination STDF file
    bool	DumpRecord(StdfParse *pclStdfDest);
    // Return record #
    long    GetRecordNumber(void);

    // return the current deciphering mode for site number
    GS::StdLib::DecipheringModes GetHeadAndSiteNumberDecipheringMode() const
    { return m_SiteNumberDecipheringMode; }

    // set the site number deciphering mode
    void SetHeadAndSiteNumberDecipheringMode
        ( GS::StdLib::DecipheringModes mode )
    { m_SiteNumberDecipheringMode = mode; }

    //******************************************
    // Toolbox functions over STDF file

    // Clone STDF into a 'Light STDF' only holding WAFERMAP data (FAR+MIR+WIR+PIR+PRR+WRR+MRR)
    bool	toolExtractWafermap(QString strInputStdf,QString strOutputStdf);

protected:

private:

// DATA
public:
    // Error codes returned by StdfParse functions
    enum ErrorCode {
            NoError,			// No error
            FileClosed,			// STDF File not opened (so access not allowed)
            UnexpectedRecord,	// Unexpected record found in STDF file
            EndOfFile,			// End of file reached.
            FileCorrupted		// File corrupted...unexpected end of file in record
    };

    // Tells current position in STDF record READ
    long GetReadRecordPos(void) { return m_clStdf.GetReadRecordPos(); }
    // Tekks current position in the STDF file read
    long GetFilePos(void)       { return m_clStdf.GetPos(); }
    // Action : Tells file size.
    // return :	NoError or error code
    long GetFileSize()  {  return m_clStdf.GetFileSize(m_strFileName.toLatin1().data()); }

private:
    int					m_nStdfVersion;		// Version of STDF file
    int					m_nStdfCpuType;		// CPU type of STDF file
    GS::StdLib::Stdf				m_clStdf;			// Stdf object (for low level access to Stdf file)
    int					m_nStdfOpened;		// Set if m_clStdf opened
    int					m_nCurrentRecord;	// ID of current loaded record
    GS::StdLib::StdfRecordReadInfo	m_stRecordInfo;		// Record info for current loaded record
    QString				m_strFileName;		// STDF file name
    GS::StdLib::DecipheringModes m_SiteNumberDecipheringMode;
};
}
#endif // #ifndef _StdfParse_h_
