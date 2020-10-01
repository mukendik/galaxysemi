#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h> // for M_PI
#include <gstdl_mailer.h> // to send an email
#include <gstdl_systeminfo.h>

#include <QFileInfo>
#include <QTextStream>
#include <QDate>
#include <QFile>

#include "stdf.h"
#include "stdfparse.h"
#include "importAtdfToStdf.h"

QFile lAsciiFile;
QTextStream	m_hAsciiStream;
QFile lAtdfFile;
QTextStream	m_hAtdfStream;
GQTL_STDF::StdfParse	m_clStdfParse;		// STDF V4 parser

GQTL_STDF::Stdf_FAR_V4	m_clStdfFAR;
GQTL_STDF::Stdf_ATR_V4	m_clStdfATR;
GQTL_STDF::Stdf_MIR_V4	m_clStdfMIR;
GQTL_STDF::Stdf_MRR_V4	m_clStdfMRR;
GQTL_STDF::Stdf_PCR_V4	m_clStdfPCR;
GQTL_STDF::Stdf_HBR_V4	m_clStdfHBR;
GQTL_STDF::Stdf_SBR_V4	m_clStdfSBR;
GQTL_STDF::Stdf_PMR_V4	m_clStdfPMR;
GQTL_STDF::Stdf_PGR_V4	m_clStdfPGR;
GQTL_STDF::Stdf_PLR_V4	m_clStdfPLR;
GQTL_STDF::Stdf_RDR_V4	m_clStdfRDR;
GQTL_STDF::Stdf_SDR_V4	m_clStdfSDR;
GQTL_STDF::Stdf_WIR_V4	m_clStdfWIR;
GQTL_STDF::Stdf_WRR_V4	m_clStdfWRR;
GQTL_STDF::Stdf_WCR_V4	m_clStdfWCR;
GQTL_STDF::Stdf_PIR_V4	m_clStdfPIR;
GQTL_STDF::Stdf_PRR_V4	m_clStdfPRR;
GQTL_STDF::Stdf_TSR_V4	m_clStdfTSR;
GQTL_STDF::Stdf_PTR_V4	m_clStdfPTR;
GQTL_STDF::Stdf_MPR_V4	m_clStdfMPR;
GQTL_STDF::Stdf_FTR_V4	m_clStdfFTR;
GQTL_STDF::Stdf_BPS_V4	m_clStdfBPS;
GQTL_STDF::Stdf_EPS_V4	m_clStdfEPS;
GQTL_STDF::Stdf_GDR_V4	m_clStdfGDR;
GQTL_STDF::Stdf_DTR_V4	m_clStdfDTR;
GQTL_STDF::Stdf_RESERVED_IMAGE_V4	m_clStdfRESERVED_IMAGE;
GQTL_STDF::Stdf_RESERVED_IG900_V4	m_clStdfRESERVED_IG900;
GQTL_STDF::Stdf_UNKNOWN_V4	m_clStdfUNKNOWN;
GQTL_STDF::Stdf_VUR_V4	m_clStdfVUR;
GQTL_STDF::Stdf_PSR_V4 	m_clStdfPSR;
GQTL_STDF::Stdf_NMR_V4	m_clStdfNMR;
GQTL_STDF::Stdf_CNR_V4	m_clStdfCNR;
GQTL_STDF::Stdf_SSR_V4	m_clStdfSSR;
GQTL_STDF::Stdf_CDR_V4	m_clStdfCDR;
GQTL_STDF::Stdf_STR_V4	m_clStdfSTR;

GQTL_STDF::Stdf_FAR_V3	m_clStdfV3FAR;
GQTL_STDF::Stdf_MIR_V3	m_clStdfV3MIR;
GQTL_STDF::Stdf_MRR_V3	m_clStdfV3MRR;
GQTL_STDF::Stdf_HBR_V3	m_clStdfV3HBR;
GQTL_STDF::Stdf_SBR_V3	m_clStdfV3SBR;
GQTL_STDF::Stdf_PMR_V3	m_clStdfV3PMR;
GQTL_STDF::Stdf_WIR_V3	m_clStdfV3WIR;
GQTL_STDF::Stdf_WRR_V3	m_clStdfV3WRR;
GQTL_STDF::Stdf_WCR_V3	m_clStdfV3WCR;
GQTL_STDF::Stdf_PIR_V3	m_clStdfV3PIR;
GQTL_STDF::Stdf_PRR_V3	m_clStdfV3PRR;
GQTL_STDF::Stdf_PDR_V3	m_clStdfV3PDR;
GQTL_STDF::Stdf_FDR_V3	m_clStdfV3FDR;
GQTL_STDF::Stdf_TSR_V3	m_clStdfV3TSR;
GQTL_STDF::Stdf_PTR_V3	m_clStdfV3PTR;
GQTL_STDF::Stdf_FTR_V3	m_clStdfV3FTR;
GQTL_STDF::Stdf_BPS_V3	m_clStdfV3BPS;
GQTL_STDF::Stdf_EPS_V3	m_clStdfV3EPS;
GQTL_STDF::Stdf_SHB_V3	m_clStdfV3SHB;
GQTL_STDF::Stdf_SSB_V3	m_clStdfV3SSB;
GQTL_STDF::Stdf_STS_V3	m_clStdfV3STS;
GQTL_STDF::Stdf_SCR_V3	m_clStdfV3SCR;
GQTL_STDF::Stdf_GDR_V3	m_clStdfV3GDR;
GQTL_STDF::Stdf_DTR_V3	m_clStdfV3DTR;

// Extract record data and dump it
bool ProcessRecordASCII(/*int nRecordType,*/ GQTL_STDF::Stdf_Record* lRecord)
{
    QString	lAsciiString;
    // precision 12 does not allow ut to pass on all platforms:
    // exple MPR:  [005] = 0.000000001956239836786722 |	 [005] = 0.000000001956240280875932
    // exple if precision 12: [004] = 0.0000000029063				      |	        [004] = 0.0000000029062
    // exple if precision 12: [001] = -0.000000001552				      |	        [001] = -0.000000001553
    //                    11: [005] = -0.00000000500				      |	        [005] = -0.00000000499
    lRecord->SetPrecision(10);
    // a:Hexa float/double e:exponent : 'a' format is cross platform, 'e' is not. 'q' is special qa output
    lRecord->SetFormat('q');
    if (m_clStdfParse.ReadRecord(lRecord) == false)
    {
        // Error reading STDF file. Dump data read and add an error message.
        // Make sure to get only data that has been read (FieldFlag_Present).
        // todo: add an argument to specify precision when numbers record (PTR, MPR,...) or a SetPrecision(...) ?
        // precision of 6 is not enough
        lRecord->GetAsciiRecord(lAsciiString, 1 | GQTL_STDF::Stdf_Record::FieldFlag_Present);
        // Note: On Windows, all '\n' characters are written as '\r\n'
        m_hAsciiStream << '\n'; //endl; DO NOT USE endl which is OS dependant: CRLF, LF or CR !
        m_hAsciiStream << lAsciiString;
        m_hAsciiStream << "** ERROR: unexpected end of record!!\n";
        return false;
    }
    else
    {
        lRecord->GetAsciiRecord(lAsciiString, 0x02);
        m_hAsciiStream  << '\n'; //endl; DO NOT USE endl which is OS dependant: CRLF, LF or CR !
        m_hAsciiStream  << lAsciiString;
        return true;
    }

}

bool ProcessRecordATDF(GQTL_STDF::Stdf_Record* pclRecord)
{
    QString	lAtdfString;
    //pclRecord->SetFormat('q'); // qa output mode
    //pclRecord->SetPrecision(4); // yes I know but no choice
    if(m_clStdfParse.ReadRecord(pclRecord) == false)
    {
        // Error reading STDF file. Dump data read and add an error message.
        pclRecord->GetAtdfString(lAtdfString);
        m_hAtdfStream  << lAtdfString;
        m_hAtdfStream  <<  "** ERROR: unexpected end of record!!\n";
        return false;
    }
    else
    {
        pclRecord->GetAtdfString(lAtdfString);
        m_hAtdfStream  << lAtdfString;
        return true;
    }
}


bool ConvertToASCII(const QString& strFileNameSTDF, const QString& strAsciiFileName)
{
    qDebug("ConvertToASCII '%s' to '%s'", strFileNameSTDF.toLatin1().data(), strAsciiFileName.toLatin1().data() );

    int nRecordType=0;

    // Open ASCII files

    lAsciiFile.setFileName(strAsciiFileName);
    // Guys: open with Text mode in order to impose CRLF and not LF
    // If not the UT is failing on Windows but passing on Linux (or contrary)
    if(!lAsciiFile.open(QIODevice::WriteOnly)) // with QIODevice::Text : CRLF, without LF.
    {
        return false;
    }

    // Assign file I/O stream
    m_hAsciiStream.setDevice(&lAsciiFile);

    // Open STDF file to read...
    int iStatus = m_clStdfParse.Open(strFileNameSTDF.toLatin1().constData());
    if(iStatus == false)
    {
        // Close ASCII files
        lAsciiFile.close();
        return false;
    }
    iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        // Process STDF record read.
        switch(nRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfFAR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_ATR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfATR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfMIR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_MRR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfMRR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PCR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPCR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfHBR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfSBR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPMR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PGR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPGR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PLR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPLR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RDR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfRDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SDR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfSDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfWIR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfWRR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfWCR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPIR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPRR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfTSR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MPR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfMPR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfFTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfBPS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfEPS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfGDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfDTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfRESERVED_IMAGE);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfRESERVED_IG900);
                break;
            case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfUNKNOWN);
                break;
            case GQTL_STDF::Stdf_Record::Rec_VUR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfVUR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_PSR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfPSR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_NMR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfNMR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_CNR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfCNR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_SSR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfSSR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_CDR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfCDR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_STR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfSTR);
                    break;

        }
        // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    };

    // Convertion successful, close ASCII files.
    lAsciiFile.close();

    return true;
}


bool ConvertToATDF(const QString& strFileNameSTDF, const QString& strAtdfFileName)
{
    qDebug("ConvertToATDF %s to %s", strFileNameSTDF.toLatin1().data(), strAtdfFileName.toLatin1().data());

    int nRecordType;

    // Open ASCII files

    lAtdfFile.setFileName(strAtdfFileName);
    if(!lAtdfFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        return false;
    }

    // Assign file I/O stream
    m_hAtdfStream.setDevice(&lAtdfFile);

    // Open STDF file to read...
    int iStatus = m_clStdfParse.Open(strFileNameSTDF.toLatin1().constData());
    if(iStatus == false)
    {
        // Close ASCII files
        lAtdfFile.close();
        return false;
    }
    iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        // Process STDF record read.
        switch(nRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfFAR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_ATR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfATR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                if(ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfMIR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAtdfFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_MRR:
                if(ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfMRR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAtdfFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_PCR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPCR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfHBR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSBR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPMR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PGR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPGR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PLR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPLR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RDR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfRDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SDR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                if(ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfWIR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAtdfFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                if(ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfWRR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAtdfFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfWCR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPIR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPRR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfTSR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MPR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfMPR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfFTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfBPS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfEPS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfGDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfDTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfRESERVED_IMAGE);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfRESERVED_IG900);
                break;
            case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfUNKNOWN);
                break;
            case GQTL_STDF::Stdf_Record::Rec_VUR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfVUR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_PSR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfPSR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_NMR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfNMR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_CNR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfCNR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_SSR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSSR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_CDR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfCDR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_STR :
                ProcessRecordATDF((GQTL_STDF::Stdf_Record*)&m_clStdfSTR);
                    break;

        }
        // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    };

    // Convertion successful, close ASCII files.
    lAtdfFile.close();

    return true;
}


bool ConvertToV3ASCII(const QString& strFileNameSTDF, const QString& strAsciiFileName)
{
    qDebug("ConvertToV3ASCII %s to %s", strFileNameSTDF.toLatin1().data(), strAsciiFileName.toLatin1().data() );
    int nRecordType=0;

    // Open ASCII files
    lAsciiFile.setFileName(strAsciiFileName);
    if(!lAsciiFile.open(QIODevice::WriteOnly)) // |QIODevice::Text is imposing local OS dependant end of lines
    {
        return false;
    }

    // Assign file I/O stream
    m_hAsciiStream.setDevice(&lAsciiFile);

    // Open STDF V3 file to read...
    int iStatus = m_clStdfParse.Open(strFileNameSTDF.toLatin1().constData());
    if(iStatus == false)
    {
        // Close ASCII files
        lAsciiFile.close();
        return false;
    }
    iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        // Process STDF V3 record read.
        switch(nRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3FAR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_MIR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3MIR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_MRR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3MRR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3HBR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3SBR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3PMR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3WIR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                if(ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3WRR) != true)
                {
                    // File timestamp is invalid, and higher than expiration date!
                    // Close STDF file
                    m_clStdfParse.Close();
                    // Close ASCII files
                    lAsciiFile.close();
                    return false;
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3WCR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3PIR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3PRR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PDR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3PDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_FDR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3FDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3TSR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3PTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3FTR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3BPS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3EPS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SHB:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3SHB);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SSB:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3SSB);
                break;
            case GQTL_STDF::Stdf_Record::Rec_STS:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3STS);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SCR :
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3SCR);
                    break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3GDR);
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                ProcessRecordASCII((GQTL_STDF::Stdf_Record*)&m_clStdfV3DTR);
                break;
            }
        // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    };

    // Convertion successful, close ASCII files.
    lAsciiFile.close();

    return true;
}

const char* SendEmail(char* lSubject, char* lMessage, char* lFrom, char* lTo, char* lAttachment)
{
    jwsmtp::initNetworking(); // usefull only on windows but anyway...
    jwsmtp::mailer lMailer(lTo, lFrom, lSubject, lMessage,
                           (char*)"mail.galaxysemi.com", jwsmtp::mailer::SMTP_PORT, false);

    if (lAttachment)
        lMailer.attach(std::string(lAttachment));

    lMailer.send();
    if (lMailer.response().substr(0,3)==std::string("250"))
        return "ok";
    return lMailer.response().c_str();
}

//#define SMALL_DEBUG_FILES

int main()
{
    qDebug("Unit test 5: testing STDF V3 and V4, ATDF, GS ASCII convertion\n");

    double l1=0.00000031451236589554123456; // the last numbers (456) wont fit into the double but anyway
    double l2 = l1 * 10000000000.0;
    printf("percent+.6e: %+.6e : outputs on win32: +3.145124e-007 \n", l1);
    printf("percent+.6e: %+.6e : outputs on win32: +3.145124e+003\n", l2); // outputs on win32

    printf("percent.6e: %.6e %.6e\n", l1, l2); // outputs: 3.145124e-005 on win32
    printf("percent E: %E %E\n", l1, l2); // outputs: 3.145124e-005 on win32
    printf("percent f: %f %f\n", l1, l2); // outputs: 3.145124e-005 on win32
    printf("percent f: %f %f\n", l1, l2); // outputs:  on win32
    // no need more precision than 25 because even a double cannot be more precise.
    //printf("percent .24f: %.24f %.24f\n", l1, l2); // outputs:  on win32
    // M_PI is defined with 16 decimals digits on my win32...
    printf("percent .28f PI: %.28f %.28f\n", M_PI, M_PI*100000000); // outputs:  on win32
    // %a on win32: 0x1.921fb5p+1  on linux64: 0x1.921fb54442d18p+1
    //printf("M_PI percent a: %a \t percent .5a: %.5a \t percent .12a: %.12a \t percent .0a: %.0a\n", M_PI, M_PI, M_PI, M_PI);
    //float lM_PI=(float)M_PI;
    //printf("float M_PI percent a: %a \t percent .5a: %.5a \t percent .12a: %.12a\n", lM_PI, lM_PI, lM_PI);

    QString lTest=QString::number(M_PI, 'e', 24);
    printf("QString::number(M_PI,'e',24): %s\n", lTest.toLatin1().data());
    lTest.resize(4);
    printf("QString::number(M_PI,'e',24).resize(4): %s\n", lTest.toLatin1().data());

    //
    printf("Test about long : -1=%ld sizeof(stdf_type_i4)=%zd o\n",
           (long)-1,  sizeof(stdf_type_i4) );


    CGSystemInfo lSI;
    lSI.ReadSystemInfo();

    qDebug("Locale system is: %s", QLocale::system().name().toLatin1().data() );
    qDebug("Language is: %s", QLocale::languageToString(QLocale::system().language()).toLatin1().data() );
    qDebug("Monday was appearing in ascii output as: '%s'. Now forced to english days and month.",
           QDate::shortDayName(1).toLatin1().data());
    qDebug("Env var LANG=%s", getenv("LANG")?getenv("LANG"):"?");

    qDebug("Setting Locale C...");
    QLocale::setDefault(QLocale::C);
    QLocale::system().setDefault(QLocale::English);
    qDebug("Locale will be: %s", QLocale::system().name().toLatin1().data() );

    qDebug("main: setting input and output...");

#ifdef SMALL_DEBUG_FILES
    QString lInputAtdfFileName("./in_debug.atdf");
    QString lOutputStdfFileName("./out_debug.stdf");
    QString	lOutputAtdfFileName("./out_debug.atdf");
    QString	lOutputAsciiFileName("./out_debug.txt");
    QString	lRefAsciiFileName("./ref_debug.txt");
#else
    QString lInputAtdfFileName("./in.atdf");
    QString lOutputStdfFileName("./out.stdf");
    QString	lOutputAtdfFileName("./out.atdf");
    QString	lOutputAsciiFileName("./out.txt");
    QString	lRefAsciiFileName("./ref.txt");
#endif
     QString lInputStdfV3FileName("./in_v3.std");
     QString lOutputV3AsciiFileName("./out_v3.txt");
     QString lRefV3AsciiFileName("./ref_v3.txt");

    // 1) Check ATDFtoSTDF converter
    GS::Parser::AtdfToStdf lAtdf2Stdf;

    // 1.2) Convert input ATDF to STDF
    if(lAtdf2Stdf.ConvertoStdf(lInputAtdfFileName, lOutputStdfFileName) == FALSE)
    {
        qDebug("Atdf to Stdf convert returned false");
        return EXIT_FAILURE;
    }

    /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    // 2) Check STDF dump
    // 2.1) Dump STDF to ATDF
    QFileInfo clFileInfo;
    // Check if custom ATDF dump output filename
    QString lFileName_ATDF;
    clFileInfo.setFile(lOutputAtdfFileName);
    lFileName_ATDF = clFileInfo.fileName();
    // Convert file
    if(ConvertToATDF(lOutputStdfFileName, lOutputAtdfFileName) == false)
    {
        qDebug("ConverToATDF returned false");
        return EXIT_FAILURE;
    }

    // 2.2) Dump STDF to Ascii
    // Check if custom ascii dump output filename
    QString	m_strFileName_Ascii;
    clFileInfo.setFile(lOutputAsciiFileName);
    m_strFileName_Ascii = clFileInfo.fileName();
    // Convert file
    if(ConvertToASCII(lOutputStdfFileName, lOutputAsciiFileName) == false)
    {
        qDebug("ConvertToASCII returned false");
        return EXIT_FAILURE;
    }

    // 2.3) Dump STDF to XML

    // 2.4) Dump the STDF V3 to ASCII
    QString	m_strFileName_V3_Ascii;
    clFileInfo.setFile(lOutputV3AsciiFileName);
    m_strFileName_V3_Ascii = clFileInfo.fileName();
    // Convert file
    if(ConvertToV3ASCII(lInputStdfV3FileName, lOutputV3AsciiFileName) == false)
    {
        SendEmail((char*)"Unit test 5: ConvertToV3ASCII failed",
                  (char*)"ConvertToV3ASCII failed",
                  (char*)"gexprod@galaxysemi.com",
                  (char*)"wt@galaxysemi.com",0); // ;bg@galaxysemi.com
        qDebug("ConvertToV3ASCII returned false");
        return EXIT_FAILURE;
    }

    /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    // 3) Check CSV export

    // 3.1) Export STDF to CSV

    /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    // 4) If ref files exist, compare output with ref files

    // 4.1) compare V4 ascii files
    QFile   lFileRef(lRefAsciiFileName);
    if (!lFileRef.open(QIODevice::ReadOnly))
    {
        qDebug("Cannot open '%s'", lRefAsciiFileName.toLatin1().data());
        return EXIT_FAILURE;
    }
    QString lStringRef(lFileRef.readAll());
    lFileRef.close();
    QFile   lFileOut(lOutputAsciiFileName);
    if (!lFileOut.open(QIODevice::ReadOnly))
    {
        qDebug("Cannot open '%s'", lOutputAsciiFileName.toLatin1().data());
        return EXIT_FAILURE;
    }
    char lMsg[10240]="";
    QString lStringOut(lFileOut.readAll());
    lFileOut.close();
    if (lStringRef.compare(lStringOut) != 0)
    {
        sprintf( lMsg, "Ref('%s', %d chars) != output ('%s', %d chars)",
               lRefAsciiFileName.toLatin1().data(), lStringRef.size(),
               lOutputAsciiFileName.toLatin1().data(), lStringOut.size() );
        qDebug("%s", lMsg);

        if (system("diff -y --suppress-common-lines ref.txt out.txt>diff.txt"))
        {
        }

        SendEmail(
          QString("Unit test 5: failed on %1 %2").arg(lSI.m_strPlatform.c_str()).arg(sizeof(void*)*8).toLatin1().data(),
          (char*)lMsg,
          (char*)"gexprod@galaxysemi.com",
          (char*)"wt@galaxysemi.com",
          (char*)"diff.txt"); // ;bg@galaxysemi.com

        return EXIT_FAILURE;
    }

    // 4.2) compare V3 ascii files...
    qDebug("Compare V3 ascii files...");
    lFileRef.setFileName(lRefV3AsciiFileName);
    lFileRef.open(QIODevice::ReadOnly);
    lStringRef = lFileRef.readAll();
    lFileRef.close();
    lFileOut.setFileName(lOutputV3AsciiFileName);
    lFileOut.open(QIODevice::ReadOnly);
    lStringOut = lFileOut.readAll();
    lFileOut.close();
    if (lStringRef.compare(lStringOut) != 0)
    {
        sprintf( lMsg, "Ref('%s', %d chars) != output ('%s', %d chars)",
               lRefV3AsciiFileName.toLatin1().data(), lStringRef.size(),
               lOutputV3AsciiFileName.toLatin1().data(), lStringOut.size() );
        qDebug("%s", lMsg);
        if (system(QString("diff -y --suppress-common-lines %1 %2>diff.txt").
                arg(lRefV3AsciiFileName).
                arg(lOutputV3AsciiFileName).toLatin1().data()))
        {
        }
        SendEmail(
          QString("Unit test 5: failed on %1 %2").arg(lSI.m_strPlatform.c_str()).arg(sizeof(void*)*8).toLatin1().data(),
          (char*)lMsg,
          (char*)"gexprod@galaxysemi.com",
          (char*)"wt@galaxysemi.com",
          (char*)"diff.txt"); // ;bg@galaxysemi.com

        return EXIT_FAILURE;
    }

    /* Todo : this ut is for V3 only, move that to another ut
        // 4.3) compare atdf files
        lFileRef.setFileName(lInputAtdfFileName);
        lFileRef.open(QIODevice::ReadOnly);// |QIODevice::Text ?
        lStringRef = lFileRef.readAll();
        lFileRef.close();
        // windows and linux do not output scientific notation the same way: either e+/-000 or e+/-00.
        // Let s try a easy hacky fix...
        lStringRef.replace("\r\n", "\n");
        lStringRef.replace("e+00", "e+");
        lStringRef.replace("e+0", "e+");
        lStringRef.replace("e-00", "e-");
        lStringRef.replace("e-0", "e-");

        lFileOut.setFileName(lOutputAtdfFileName);
        lFileOut.open(QIODevice::ReadOnly); // |QIODevice::Text ?
        lStringOut = lFileOut.readAll();
        lFileOut.close();
        // windows and linux do not output scientific notation the same way: either e+/-000 or e+/-00.
        // Let s try a easy hacky fix...
        lStringOut.replace("\r\n", "\n");
        lStringOut.replace("e+00", "e+");
        lStringOut.replace("e+0", "e+");
        lStringOut.replace("e-00", "e-");
        lStringOut.replace("e-0", "e-");

        if (lStringRef.compare(lStringOut) != 0)
        {
            sprintf(lMsg, "Ref('%s', %d chars) != output ('%s', %d chars)",
                    lInputAtdfFileName.toLatin1().data(), lStringRef.size(),
                    lOutputAtdfFileName.toLatin1().data(), lStringOut.size() );
            qDebug( lMsg );

            QFile::remove("diff.txt");

            QFile lTmpFile(lInputAtdfFileName+".cleaned");
            if(lTmpFile.open(QIODevice::WriteOnly))
            {
                lTmpFile.write(lStringRef.toLatin1());
                lTmpFile.close();
                lTmpFile.setFileName(lOutputAtdfFileName+".cleaned");
                if(lTmpFile.open(QIODevice::WriteOnly))
                {
                    lTmpFile.write(lStringOut.toLatin1()); lTmpFile.close();
                    system( QString("diff -y --suppress-common-lines %1 %2>diff.txt")
                        .arg(lInputAtdfFileName+".cleaned").arg(lOutputAtdfFileName+".cleaned").toLatin1().data() );
                }
            }

            QFileInfo lDiffFI("diff.txt");
            SendEmail(
              QString("Unit test 5: failed on %1 %2").arg(lSI.m_strPlatform.c_str()).arg(sizeof(void*)*8).toLatin1().data(),
              (char*)lMsg,
              (char*)"gexprod@galaxysemi.com",
              (char*)"wt@galaxysemi.com", // ;bg@galaxysemi.com
              (char*)((lDiffFI.exists()&&lDiffFI.size()<1000000)?"diff.txt":"") );

            return EXIT_FAILURE;
        }
    */

    // Guys: Could you log a little bit ?

    qDebug("Send email: %s",
           SendEmail(QString("Unit test 5: success on %1 %2").arg(lSI.m_strPlatform.c_str()).arg(sizeof(void*)*8)
              .toLatin1().data(),
              (char*)"success", (char*)"gexprod@galaxysemi.com",
              (char*)"wt@galaxysemi.com", 0)); // ;bg@galaxysemi.com
    qDebug("Success.");

    return EXIT_SUCCESS;
}


