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
// StdfParse class IMPLEMENTATION :
// This class contains routines to read records from an
// stdf file
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>

#include "stdfparse.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"
namespace GQTL_STDF
{
// Error map
GBEGIN_ERROR_MAP(StdfParse)
	GMAP_ERROR(eFileClosed,"No STDF file is opened.")
	GMAP_ERROR(eErrorOpen,"Couldn't open file %s.")
	GMAP_ERROR(eRecordCorrupted,"Error reading file %s.\nUnexpected end of file in record %s.")
	GMAP_ERROR(eFileCorrupted,"Error reading file %s.\nFile is corrupted.")
	GMAP_ERROR(eRecordNotFound,"Record %s not found in file %s.")
	GMAP_ERROR(eUnexpectedRecord,"Unexpected record (%d/%d) found in file %s.")
	GMAP_ERROR(eErrorRead,"Error reading record %s from file %s.")
	GMAP_ERROR(eBadRecordLoaded,"The requested record is not loaded.")
	GMAP_ERROR(eBadStdfVersion,"File %s:\nUnsupported STDF version (%d).\nThis parser supports only STDF V4.")
	GMAP_ERROR(eErrorDumpRecord,"Couldn't dump record from %s to %s.")
GEND_ERROR_MAP(StdfParse)


///////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////
StdfParse::StdfParse()
{
	m_nStdfVersion = STDF_V_UNKNOWN;
	m_nStdfCpuType = 0;
	m_nStdfOpened = 0;
    m_nCurrentRecord = Stdf_Record::Rec_UNKNOWN;
    m_SiteNumberDecipheringMode = GS::StdLib::no_deciphering;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
StdfParse::~StdfParse()
{
	m_clStdf.Close();
}

///////////////////////////////////////////////////////////
// Opens STDF file
///////////////////////////////////////////////////////////
bool StdfParse::Open(const char *szFileName, int iAccessMode /*= STDF_READ*/, int iForce_CPU_Type /*= -1*/)
{
	int nStatus;
	
    // Open Stdf
	nStatus = m_clStdf.Open(szFileName, iAccessMode, 1000000L);		// 1MB cache
    if(nStatus == GS::StdLib::Stdf::FileOpened)
	{
		m_clStdf.Close();
		nStatus = m_clStdf.Open(szFileName, iAccessMode, 1000000L);	// 1MB cache
	}
	
	// Check status
    if(nStatus == GS::StdLib::Stdf::NoError)
	{
		int nSTDFVersion;
		
		if(iAccessMode == STDF_WRITE)
			nSTDFVersion = 4;
		else
			nSTDFVersion= m_clStdf.GetStdfVersion();
		
        if(nSTDFVersion == STDF_V_3)
		{
            m_nStdfVersion = nSTDFVersion;
            m_nStdfCpuType = m_clStdf.GetStdfCpuType();
            if((iAccessMode == STDF_WRITE) && (iForce_CPU_Type != -1))
                m_clStdf.SetStdfCpuType(iForce_CPU_Type);
            m_nStdfCpuType = m_clStdf.GetStdfCpuType();
            m_nStdfOpened = 1;
            m_strFileName = szFileName;
		}
        else
            if (nSTDFVersion == STDF_V_4)
            {
                m_nStdfVersion = nSTDFVersion;
                m_nStdfCpuType = m_clStdf.GetStdfCpuType();
                if((iAccessMode == STDF_WRITE) && (iForce_CPU_Type != -1))
                    m_clStdf.SetStdfCpuType(iForce_CPU_Type);
                m_nStdfCpuType = m_clStdf.GetStdfCpuType();
                m_nStdfOpened = 1;
                m_strFileName = szFileName;

                // If write mode, write FAR record
                if(iAccessMode == STDF_WRITE)
                {
                    Stdf_FAR_V4	clFAR;
                    clFAR.SetCPU_TYPE(m_nStdfCpuType);
                    clFAR.SetSTDF_VER(m_nStdfVersion);
                    if(!WriteRecord(&clFAR))
                        return false;
                }
            }
            else
            {
                m_clStdf.Close();
                GSET_ERROR2(StdfParse, eBadStdfVersion, NULL, szFileName, nSTDFVersion);
                return false;
            }


		return true;
	}
	
    if(nStatus == GS::StdLib::Stdf::Corrupted)
	{
        GSET_ERROR1(StdfParse, eFileCorrupted, NULL, szFileName);
		return false;
	}

    GSET_ERROR1(StdfParse, eErrorOpen, NULL, szFileName);
	return false;
}


///////////////////////////////////////////////////////////
// Closes STDF file
///////////////////////////////////////////////////////////
void StdfParse::Close(void)
{
	m_clStdf.Close();
	m_nStdfOpened = 0;
	m_nStdfVersion = STDF_V_UNKNOWN;
	m_nStdfCpuType = 0;
    m_nCurrentRecord = Stdf_Record::Rec_UNKNOWN;
}

///////////////////////////////////////////////////////////
// Rewinds STDF file
///////////////////////////////////////////////////////////
bool StdfParse::Rewind(void)
{
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	m_clStdf.RewindFile();
    m_nCurrentRecord = Stdf_Record::Rec_UNKNOWN;
	return true;
}

///////////////////////////////////////////////////////////
// Get Version of STDF file (3 or 4)
///////////////////////////////////////////////////////////
bool StdfParse::GetVersion(int *nVersion)
{
	*nVersion = STDF_V_UNKNOWN;
	
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	*nVersion = m_nStdfVersion;
    return true;
}

///////////////////////////////////////////////////////////
// Get CPU type of STDF file (3 or 4)
///////////////////////////////////////////////////////////
bool StdfParse::GetCpuType(int *nCpuType)
{
	*nCpuType = 0;
	
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	*nCpuType = m_nStdfCpuType;
	return true;
}

///////////////////////////////////////////////////////////
// Get handle on Stdf base class
///////////////////////////////////////////////////////////
bool StdfParse::GetStdfHandle(GS::StdLib::Stdf **ppclStdfHandle)
{
	*ppclStdfHandle = NULL;
	
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	*ppclStdfHandle = &m_clStdf;
	return true;
}

///////////////////////////////////////////////////////////
// Get STDF file name
///////////////////////////////////////////////////////////
bool StdfParse::GetFileName(QString *pstrFileName)
{
	*pstrFileName = "";
	
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	*pstrFileName = m_strFileName;
	return true;
}

///////////////////////////////////////////////////////////
// Get record Type & SubType of last loaded record
///////////////////////////////////////////////////////////
int StdfParse::GetRecordType(int *nRecordType,int *nRecordSubType)
{
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return FileClosed;
	}

	*nRecordType = m_stRecordInfo.iRecordType;
	*nRecordSubType = m_stRecordInfo.iRecordSubType;

	return NoError;
}

///////////////////////////////////////////////////////////
// Get record #
///////////////////////////////////////////////////////////
long StdfParse::GetRecordNumber(void)
{
	return m_clStdf.GetReadRecordNumber();
}

///////////////////////////////////////////////////////////
// Load Next record and return informations about record type/subtype
///////////////////////////////////////////////////////////
int StdfParse::LoadNextRecord(int *nRecordType, const bool bRewind /*= false*/)
{
    int	nStatus;
	
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return FileClosed;
	}
	
	if(bRewind == true)
		// Rewind STDF file
		m_clStdf.RewindFile();

	// Load next record
    m_nCurrentRecord = *nRecordType = Stdf_Record::Rec_UNKNOWN;
	nStatus = m_clStdf.LoadRecord(&m_stRecordInfo);
    if(nStatus == GS::StdLib::Stdf::NoError)
	{
		if((m_stRecordInfo.iRecordType == STDF_FAR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_FAR_STYPE))
            *nRecordType = Stdf_Record::Rec_FAR;
		else if((m_stRecordInfo.iRecordType == STDF_ATR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_ATR_STYPE))
            *nRecordType = Stdf_Record::Rec_ATR;
		else if((m_stRecordInfo.iRecordType == STDF_MIR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_MIR_STYPE))
            *nRecordType = Stdf_Record::Rec_MIR;
		else if((m_stRecordInfo.iRecordType == STDF_MRR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_MRR_STYPE))
            *nRecordType = Stdf_Record::Rec_MRR;
		else if((m_stRecordInfo.iRecordType == STDF_PCR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PCR_STYPE))
            *nRecordType = Stdf_Record::Rec_PCR;
		else if((m_stRecordInfo.iRecordType == STDF_HBR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_HBR_STYPE))
            *nRecordType = Stdf_Record::Rec_HBR;
		else if((m_stRecordInfo.iRecordType == STDF_SBR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_SBR_STYPE))
            *nRecordType = Stdf_Record::Rec_SBR;
		else if((m_stRecordInfo.iRecordType == STDF_PMR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PMR_STYPE))
            *nRecordType = Stdf_Record::Rec_PMR;
		else if((m_stRecordInfo.iRecordType == STDF_PGR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PGR_STYPE))
            *nRecordType = Stdf_Record::Rec_PGR;
		else if((m_stRecordInfo.iRecordType == STDF_PLR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PLR_STYPE))
            *nRecordType = Stdf_Record::Rec_PLR;
		else if((m_stRecordInfo.iRecordType == STDF_RDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_RDR_STYPE))
            *nRecordType = Stdf_Record::Rec_RDR;
		else if((m_stRecordInfo.iRecordType == STDF_SDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_SDR_STYPE))
            *nRecordType = Stdf_Record::Rec_SDR;
		else if((m_stRecordInfo.iRecordType == STDF_WIR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_WIR_STYPE))
            *nRecordType = Stdf_Record::Rec_WIR;
		else if((m_stRecordInfo.iRecordType == STDF_WRR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_WRR_STYPE))
            *nRecordType = Stdf_Record::Rec_WRR;
		else if((m_stRecordInfo.iRecordType == STDF_WCR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_WCR_STYPE))
            *nRecordType = Stdf_Record::Rec_WCR;
		else if((m_stRecordInfo.iRecordType == STDF_PIR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PIR_STYPE))
            *nRecordType = Stdf_Record::Rec_PIR;
		else if((m_stRecordInfo.iRecordType == STDF_PRR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PRR_STYPE))
            *nRecordType = Stdf_Record::Rec_PRR;
		else if((m_stRecordInfo.iRecordType == STDF_TSR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_TSR_STYPE))
            *nRecordType = Stdf_Record::Rec_TSR;
		else if((m_stRecordInfo.iRecordType == STDF_PTR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PTR_STYPE))
            *nRecordType = Stdf_Record::Rec_PTR;
		else if((m_stRecordInfo.iRecordType == STDF_MPR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_MPR_STYPE))
            *nRecordType = Stdf_Record::Rec_MPR;
		else if((m_stRecordInfo.iRecordType == STDF_FTR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_FTR_STYPE))
            *nRecordType = Stdf_Record::Rec_FTR;
		else if((m_stRecordInfo.iRecordType == STDF_BPS_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_BPS_STYPE))
            *nRecordType = Stdf_Record::Rec_BPS;
		else if((m_stRecordInfo.iRecordType == STDF_EPS_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_EPS_STYPE))
            *nRecordType = Stdf_Record::Rec_EPS;
		else if((m_stRecordInfo.iRecordType == STDF_GDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_GDR_STYPE))
            *nRecordType = Stdf_Record::Rec_GDR;
		else if((m_stRecordInfo.iRecordType == STDF_DTR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_DTR_STYPE))
            *nRecordType = Stdf_Record::Rec_DTR;
		else if(m_stRecordInfo.iRecordType == STDF_RESERVED_IMAGE_TYPE)
            *nRecordType = Stdf_Record::Rec_RESERVED_IMAGE;
		else if(m_stRecordInfo.iRecordType == STDF_RESERVED_IG900_TYPE)
            *nRecordType = Stdf_Record::Rec_RESERVED_IG900;
		else  if((m_stRecordInfo.iRecordType == STDF_VUR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_VUR_STYPE))
            *nRecordType = Stdf_Record::Rec_VUR;
		else if((m_stRecordInfo.iRecordType == STDF_PSR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PSR_STYPE))
            *nRecordType = Stdf_Record::Rec_PSR;
		else  if((m_stRecordInfo.iRecordType == STDF_NMR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_NMR_STYPE))
            *nRecordType = Stdf_Record::Rec_NMR;
		else  if((m_stRecordInfo.iRecordType == STDF_CNR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_CNR_STYPE))
            *nRecordType = Stdf_Record::Rec_CNR;
		else  if((m_stRecordInfo.iRecordType == STDF_SSR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_SSR_STYPE))
            *nRecordType = Stdf_Record::Rec_SSR;
		else  if((m_stRecordInfo.iRecordType == STDF_CDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_CDR_STYPE))
            *nRecordType = Stdf_Record::Rec_CDR;
		else  if((m_stRecordInfo.iRecordType == STDF_STR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_STR_STYPE))
            *nRecordType = Stdf_Record::Rec_STR;
        else  if((m_stRecordInfo.iRecordType == STDF_SCR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_SCR_STYPE))
            *nRecordType = Stdf_Record::Rec_SCR;
        else  if((m_stRecordInfo.iRecordType == STDF_PDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PDR_STYPE))
            *nRecordType = Stdf_Record::Rec_PDR;
        else  if((m_stRecordInfo.iRecordType == STDF_SHB_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_SHB_STYPE))
            *nRecordType = Stdf_Record::Rec_SHB;
        else  if((m_stRecordInfo.iRecordType == STDF_SSB_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_SSB_STYPE))
            *nRecordType = Stdf_Record::Rec_SSB;
        else  if((m_stRecordInfo.iRecordType == STDF_PDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_PDR_STYPE))
            *nRecordType = Stdf_Record::Rec_PDR;
        else  if((m_stRecordInfo.iRecordType == STDF_FDR_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_FDR_STYPE))
            *nRecordType = Stdf_Record::Rec_FDR;
        else  if((m_stRecordInfo.iRecordType == STDF_STS_TYPE) && (m_stRecordInfo.iRecordSubType == STDF_STS_STYPE))
            *nRecordType = Stdf_Record::Rec_PDR;


		else
		{
            GSET_ERROR3(StdfParse, eUnexpectedRecord, NULL, m_stRecordInfo.iRecordType, m_stRecordInfo.iRecordSubType, m_strFileName.toLatin1().constData());
			return UnexpectedRecord;
		}
		m_nCurrentRecord = *nRecordType;
		
		return NoError;
	}
	
    if(nStatus == GS::StdLib::Stdf::EndOfFile)
		return EndOfFile;

    GSET_ERROR1(StdfParse, eFileCorrupted, NULL, m_strFileName.toLatin1().constData());
	return FileCorrupted;
}

///////////////////////////////////////////////////////////
// Load Next record of a specific type/subtype
///////////////////////////////////////////////////////////
int StdfParse::LoadNextRecord(const int nRecordType, const bool bRewind /*= false*/)
{
	int nStatus, nRecordRead;
	
	if(bRewind == true)
		// Rewind STDF file
		m_clStdf.RewindFile();

	// Find next record
	do
	{
		nStatus = LoadNextRecord(&nRecordRead);
	}
	while((nStatus == NoError) && (nRecordRead != nRecordType));
	
	// Check status
	return nStatus;
}

///////////////////////////////////////////////////////////
// Retrieve informations from current record.
///////////////////////////////////////////////////////////
bool StdfParse::ReadRecord(Stdf_Record* pclStdfRecord)
{
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	// Check current record
	if(m_nCurrentRecord != pclStdfRecord->GetRecordType())
	{
        GSET_ERROR0(StdfParse,eBadRecordLoaded,NULL);
		return false;
	}
	
	// Set record information retrieved when record was loaded
	pclStdfRecord->m_stRecordInfo = m_stRecordInfo;
	
	// Read Informations from record
	if(pclStdfRecord->Read(m_clStdf) == FALSE)
    {
        GSET_ERROR2(StdfParse, eRecordCorrupted, NULL, m_strFileName.toLatin1().constData(), pclStdfRecord->GetRecordShortName().toLatin1().constData());
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// Write record.
///////////////////////////////////////////////////////////
bool StdfParse::WriteRecord(Stdf_Record* pclStdfRecord)
{
    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	// Write record
	if(pclStdfRecord->Write(m_clStdf) == FALSE)
	{
        GSET_ERROR2(StdfParse, eRecordCorrupted, NULL, m_strFileName.toLatin1().constData(), pclStdfRecord->GetRecordShortName().toLatin1().constData());
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// Dump record from source.
///////////////////////////////////////////////////////////
bool StdfParse::DumpRecord(StdfParse *pclStdfDest)
{
    GS::StdLib::Stdf	*pclStdfHandle;
	QString	strDestFileName;

    // Check if Stdf opened
	if(!m_nStdfOpened)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}
	
	// Get handle on destination STDF base class
	if(pclStdfDest->GetStdfHandle(&pclStdfHandle) == false)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}

	// Get dest STDF file name
	if(pclStdfDest->GetFileName(&strDestFileName) == false)
	{
        GSET_ERROR0(StdfParse,eFileClosed,NULL);
		return false;
	}

	// Dump record
    if(pclStdfHandle->DumpRecord(&m_stRecordInfo, &m_clStdf, TRUE) != GS::StdLib::Stdf::NoError)
	{
        GSET_ERROR2(StdfParse, eErrorDumpRecord, NULL, m_strFileName.toLatin1().constData(), strDestFileName.toLatin1().constData());
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////
// Generate STDF file with ONLY Wafermap data records
///////////////////////////////////////////////////////////
bool StdfParse::toolExtractWafermap(QString strInputStdf,QString strOutputStdf)
{
    StdfParse cInputStdf;
    StdfParse cOutputStdf;
	int			  iStatus,nRecordType;

	// Open input STDF file to read...
	if(cInputStdf.Open(strInputStdf.toLatin1().constData()) == FALSE)
		return false;

	// Open output STDF file to read...
	int	iCpuType;
	cInputStdf.GetCpuType(&iCpuType);

	if(cOutputStdf.Open(strOutputStdf.toLatin1().constData(),STDF_WRITE,iCpuType) == FALSE)
		return false;

	// Read one record from STDF file.
	iStatus = cInputStdf.LoadNextRecord(&nRecordType);
    while((iStatus == StdfParse::NoError) || (iStatus == StdfParse::UnexpectedRecord))
	{
		// List of records to keep (note: FAR record is automatically added by Open function)
		switch(nRecordType)
		{
            case Stdf_Record::Rec_ATR:
            case Stdf_Record::Rec_MIR:
            case Stdf_Record::Rec_MRR:
            case Stdf_Record::Rec_PCR:
            case Stdf_Record::Rec_HBR:
            case Stdf_Record::Rec_SBR:
            case Stdf_Record::Rec_WIR:
            case Stdf_Record::Rec_WRR:
            case Stdf_Record::Rec_WCR:
            case Stdf_Record::Rec_PIR:
            case Stdf_Record::Rec_PRR:
			cInputStdf.DumpRecord(&cOutputStdf);
			break;
		}

		// Read one record from STDF file.
		iStatus = cInputStdf.LoadNextRecord(&nRecordType);
	};

	return true;
}
}
