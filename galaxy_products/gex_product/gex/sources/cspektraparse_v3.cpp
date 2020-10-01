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
// CSpektraParse_V3 class IMPLEMENTATION :
// This class contains routines to read records from an
// stdf file
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QString>
#include <QStringList>
#include <QDateTime>

#include "cspektraparse_v3.h"

// Error map
GBEGIN_ERROR_MAP(CSpektraParse_V3)
	GMAP_ERROR(eFileClosed,"No SPEKTRA file is opened.")
	GMAP_ERROR(eErrorOpen,"Couldn't open file %s.")
	GMAP_ERROR(eRecordCorrupted,"Error reading file %s.\nUnexpected end of file in record %s.")
	GMAP_ERROR(eFileCorrupted,"Error reading file %s.\nFile is corrupted.")
	GMAP_ERROR(eUnexpectedRecord,"Coudn't load unvalid record %d from file %s.")
	GMAP_ERROR(eBadRecordLoaded,"The requested record is not loaded.")
GEND_ERROR_MAP(CSpektraParse_V3)

///////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////
CSpektraParse_V3::CSpektraParse_V3()
{
	m_nSpektraOpened = 0;
	m_nCurrentRecord = CSpektra_Record_V3::Rec_UNKNOWN;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CSpektraParse_V3::~CSpektraParse_V3()
{
	m_clSpektra.Close();
}

///////////////////////////////////////////////////////////
// Opens SPEKTRA file
///////////////////////////////////////////////////////////
BOOL CSpektraParse_V3::Open(const char *szFileName)
{
	int nStatus;
	
	// Open CSpektra
	nStatus = m_clSpektra.Open(szFileName, 1000000L);		// 1MB cache
	if(nStatus == CSpektra::FileOpened)
	{
		m_clSpektra.Close();
		nStatus = m_clSpektra.Open(szFileName, 1000000L);	// 1MB cache
	}
	
	// Check status
	if(nStatus == CSpektra::NoError)
	{
		m_nSpektraOpened = 1;
		m_strFileName = szFileName;
		if(IsValidSpektraFile() == false)
		{
			Close();
			GSET_ERROR1(CSpektraParse_V3, eFileCorrupted, NULL, szFileName);
			return false;
		}
		return true;
	}
	
	GSET_ERROR1(CSpektraParse_V3, eErrorOpen, NULL, szFileName);
	return false;
}


///////////////////////////////////////////////////////////
// Closes SPEKTRA file
///////////////////////////////////////////////////////////
void CSpektraParse_V3::Close(void)
{
	m_clSpektra.Close();
	m_nSpektraOpened = 0;
	m_nCurrentRecord = CSpektra_Record_V3::Rec_UNKNOWN;
}

///////////////////////////////////////////////////////////
// Rewinds SPEKTRA file
///////////////////////////////////////////////////////////
BOOL CSpektraParse_V3::Rewind(void)
{
	// Check if CSpektra opened
	if(!m_nSpektraOpened)
	{
		GSET_ERROR0(CSpektraParse_V3,eFileClosed,NULL);
		return false;
	}
	
	m_clSpektra.RewindFile();
	m_nCurrentRecord = CSpektra_Record_V3::Rec_UNKNOWN;
	return true;
}

///////////////////////////////////////////////////////////
// Load Next record of a specific type
///////////////////////////////////////////////////////////
int CSpektraParse_V3::LoadNextRecord(const int nRecordType)
{
	int nStatus;
	
	// Check if CSpektra opened
	if(!m_nSpektraOpened)
	{
		GSET_ERROR0(CSpektraParse_V3,eFileClosed,NULL);
		return FileClosed;
	}

	// Find next record
	switch(nRecordType)
	{
		case CSpektra_Record_V3::Rec_Label:
			nStatus = m_clSpektra.LoadRecord(128);
			break;
		case CSpektra_Record_V3::Rec_TestItem:
			nStatus = m_clSpektra.LoadRecord(64);
			break;
		case CSpektra_Record_V3::Rec_WaferID:
			nStatus = m_clSpektra.LoadRecord(6);
			break;
		case CSpektra_Record_V3::Rec_DeviceID:
			nStatus = m_clSpektra.LoadRecord(6);
			break;
		case CSpektra_Record_V3::Rec_TestData:
			nStatus = m_clSpektra.LoadRecord(6);
			break;
		default:
			GSET_ERROR2(CSpektraParse_V3, eUnexpectedRecord, NULL, nRecordType, m_strFileName.toLatin1().constData());
			return UnexpectedRecord;
	}

	// Check status
	if(nStatus == CSpektra::NoError)
	{
		m_nCurrentRecord = nRecordType;
		return NoError;
	}

	if(nStatus == CSpektra::EndOfFile)
		return EndOfFile;

	GSET_ERROR1(CSpektraParse_V3, eFileCorrupted, NULL, m_strFileName.toLatin1().constData());
	return FileCorrupted;
}

///////////////////////////////////////////////////////////
// Retrieve informations from current record.
///////////////////////////////////////////////////////////
BOOL CSpektraParse_V3::ReadRecord(CSpektra_Record_V3* pclSpektraRecord)
{
	// Check if CSpektra opened
	if(!m_nSpektraOpened)
	{
		GSET_ERROR0(CSpektraParse_V3,eFileClosed,NULL);
		return false;
	}
	
	// Check current record
	if(m_nCurrentRecord != pclSpektraRecord->GetRecordType())
	{
		GSET_ERROR0(CSpektraParse_V3,eBadRecordLoaded,NULL);
		return false;
	}
	
	// Read Informations from record
	if(pclSpektraRecord->Read(m_clSpektra) == false)
	{
		GSET_ERROR2(CSpektraParse_V3, eRecordCorrupted, NULL, m_strFileName.toLatin1().constData(), pclSpektraRecord->GetRecordShortName().toLatin1().constData());
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// Checks SPEKTRA file validity
///////////////////////////////////////////////////////////
BOOL CSpektraParse_V3::IsValidSpektraFile(void)
{
	int					nStatus;
	CSpektra_Label_V3	clSpektraLabel;

	// Check if CSpektra opened
	if(!m_nSpektraOpened)
		return false;

	// Rewind
	Rewind();

	// Load first record (Label)
	nStatus = LoadNextRecord(CSpektra_Record_V3::Rec_Label);
	if(nStatus != NoError)
		return false;

	// Read record
	if(ReadRecord((CSpektra_Record_V3 *)&clSpektraLabel) == false)
		return false;

	// Check record conformance to Spektra file format
	if(clSpektraLabel.m_u1DATETIME_Month > 11)		return false;
	if(clSpektraLabel.m_u1DATETIME_Day > 30)		return false;
	if(clSpektraLabel.m_u1DATETIME_Hour > 23)		return false;
	if(clSpektraLabel.m_u1DATETIME_Minute > 59)		return false;
	if(clSpektraLabel.m_u1DATETIME_Second > 59)		return false;
	/*if(	(clSpektraLabel.m_c1STATIONNAME != 'A') && (clSpektraLabel.m_c1STATIONNAME != 'B') &&
		(clSpektraLabel.m_c1STATIONNAME != 'C') && (clSpektraLabel.m_c1STATIONNAME != 'D') &&
		(clSpektraLabel.m_c1STATIONNAME != 'F'))	return false;
	*/
	if(clSpektraLabel.m_u2TIMEPOINT > 9999)			return false;
	if(clSpektraLabel.m_u2SETQUANTITY > 32767)		return false;
	if(clSpektraLabel.m_u2LOGGINGRATE > 250)		return false;
	if(clSpektraLabel.m_u2TESTMAX > 250)			return false;
	if(clSpektraLabel.m_u2LOGGEDQUANTITY > 32767)	return false;

	// Rewind
	Rewind();

	return true;
}

