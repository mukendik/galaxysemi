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
// CSpektraParse_V3 class HEADER :
// This class contains high-level routines to retrieve
// information from an spektra file
///////////////////////////////////////////////////////////

#ifndef _CSpektraParse_V3_h_
#define _CSpektraParse_V3_h_

 // Galaxy modules includes
#include <gstdl_errormgr.h>

#include "cspektra.h"
#include "cspektrarecords_v3.h"

// Some constant definition

class QString;
class QStringList;

// CSpektraParse_V3 class definition
class CSpektraParse_V3
{
// METHODS
public:
	GDECLARE_ERROR_MAP(CSpektraParse_V3)
	{
		eFileClosed,			// SPEKTRA File not opened (so access not allowed)
		eErrorOpen,				// Error opening/creating file
		eRecordCorrupted,		// File corrupted...unexpected end of file in record
		eFileCorrupted,			// File corrupted...other reasons
		eUnexpectedRecord,		// Unexpected record found in SPEKTRA file
		eErrorRead,				// Error reading record
		eBadRecordLoaded		// Request to read a record type that is not currently loaded
	}
	GDECLARE_END_ERROR_MAP(CSpektraParse_V3)

	// Constructor / destructor functions
        CSpektraParse_V3();
        ~CSpektraParse_V3();
	// Opens SPEKTRA file
	BOOL	Open(const char *szFileName);
	// Closes SPEKTRA file
	void	Close(void);
	// Rewinds SPEKTRA file
	BOOL	Rewind(void);
	// Load Next record of specific type
	int		LoadNextRecord(const int nRecordType);
	// Read SPEKTRA Record
	BOOL	ReadRecord(CSpektra_Record_V3* pclSpektraRecord);
	
protected:
	
private:
	// Check if file is a valid Spektra file
        BOOL IsValidSpektraFile(void);

// DATA
public:
	// Error codes returned by CSpektraParse_V3 functions
	enum ErrorCode {
			NoError,			// No error
			FileClosed,			// SPEKTRA File not opened (so access not allowed)
			UnexpectedRecord,	// Unexpected record found in SPEKTRA file
			EndOfFile,			// End of file reached.
			FileCorrupted		// File corrupted...unexpected end of file in record
	};
	
private:
	CSpektra			m_clSpektra;		// CSpektra object (for low level access to Spektra file)
	int					m_nSpektraOpened;	// Set if m_clSpektra opened
	int					m_nCurrentRecord;	// ID of current loaded record
	QString				m_strFileName;		// SPEKTRA file name
};

#endif // #ifndef _CSpektraParse_V3_h_
