///////////////////////////////////////////////////////////
// STDF Checker
// Identifies STDF discrepancies (and all other Data formats supported)
///////////////////////////////////////////////////////////
#include <QApplication>
#include <QRegExp>

#include <time.h>
#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#endif

#include <gqtl_log.h>
#include <stdfparse.h> // actually a GS Qt lib

#include "browser_dialog.h"
#include "report_build.h"
#include "report_page_file_audit.h"
#include "report_options.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "patman_lib.h"
#include "cstats.h"
#include "classes.h"
#include "product_info.h"
#include "report_template.h"
#include "engine.h"
//#include "pat_info.h"
#include "pat_engine.h"
#include "pat_options.h"
#include "product_info.h"
#include "gex_shared.h"
#include "message.h"

// in report_build.cpp
extern CGexReport *		gexReport;			// Handle to report class
extern QString			formatHtmlImageFilename(const QString& strImageFileName);

#define	LF_DISABLED_VALUE	(double) -979.8e99

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexFileAudit::GexFileAudit(
    GS::Gex::CustomReportFileAuditSection* pFileAudit,
    FILE* hReportFile,
    bool bCheckPAT,
    int iTotalSites,
    bool bCsvOutput)
{
    m_pFileAudit = pFileAudit;
    m_bCheckPAT = bCheckPAT;
    m_bCsvOutput = bCsvOutput;
    m_hReportFile = hReportFile;
    m_iTotalSites = iTotalSites;

    // Clear variables
    m_FlexFormatMultiSites = false;
    m_ReportPageIndex = 0;		// Keeps track of HTML Page# being written
    m_ErrType_MIR = 0;			// Keep track of total MIR (should be only one!)
    m_ErrType_WIR = 0;			// Keeps track of total WIR
    m_ErrType_PCR = 0;			// Keeps track of total PCR
    m_ErrType_MRR = 0;			// Keeps track of total MRR
    m_ErrType_EndOfRecord = 0;	// Keeps track of total 'Unexpected end of record' errors.
    m_ErrType_TestFullNameIssue = 0;// Keep track of total test names discrepancies.
    m_ErrType_TestNameRootNameIssue = 0;// Keep track of total test names discrepancies, even removing possible leading pin#
    m_ErrType_TestLimitIssue=0; // Keep track of total test limits discrepancies.
    m_ErrType_PTR_No_PIR = 0;	// Keep track of total PTR outside of PIR/PRR records.
    m_ErrType_PRR_No_DieLoc = 0;// Keeps track of total PRR wihtout die coordinates
    m_ErrType_UnknownRec = 0;	// Keep track of total unknown records.

}

///////////////////////////////////////////////////////////
// Log error message (errors when analyzing STDF records)
///////////////////////////////////////////////////////////
void GexFileAudit::StdfPatCompliance_LogFileRecordError(bool bValidRecord,bool bPatCritical,QString strErrorType,QString strComment)
{
    QString			strLine;
    const char *	ptColor;	// Used to hold pointer to backroudn color (can be RED is PAT alarm)

    // Keep track of total errors logged
    m_iTotalLogRecordErrors++;

    // Ignore PAT warnings if this report is created with Examinator (unlike Examiantor-PAT).
    if(m_bCheckPAT == false)
        bPatCritical = false;

    // If first error, display table header !
    if(m_iTotalLogRecordErrors == 1)
    {
        if(m_bCsvOutput == false)
        {
            fprintf(m_hReportFile,"<h1 align=\"left\"><font color=\"#006699\">Issues Detected In File</font></h1>\n");
            fprintf(m_hReportFile,"<table border=\"0\" width=\"98%%\">\n");
            fprintf(m_hReportFile,"<tr>\n");
            fprintf(m_hReportFile,"<td width=\"10%%\" align=\"center\" bgcolor=%s>Record#</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"10%%\" align=\"center\" bgcolor=%s>Error Type</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"80%%\" bgcolor=%s>Details</td>\n",szFieldColor);
            fprintf(m_hReportFile,"</tr>\n");
        }
        else
        {
            fprintf(m_hReportFile,"\nIssues Detected In File:\n");
            fprintf(m_hReportFile,"Record#,Error Type,Details\n");
            fprintf(m_hReportFile,"--------------,---------------,----------------------\n");
        }
    }

    if(m_bCsvOutput)
    {
        // Report is CSV: <record#>,<Error type>,<Comment>
        if(bValidRecord)
            strLine = QString::number(m_clStdfParse.GetRecordNumber());
        strLine += ",";

        strLine += strErrorType;
        strLine += ",";
        if(bPatCritical)
            strLine += "PAT CRITICAL: ";
        strLine += strComment + "\n";
    }
    else
    {
        // Record#
        if(bValidRecord)
            strLine = "<tr>\n<td width=\"10%%\" align=\"center\" bgcolor=" + QString(szDataColor) + ">" + QString::number(m_clStdfParse.GetRecordNumber()) + "</td>\n";
        else
            strLine = "<tr>\n<td width=\"10%%\" align=\"center\" bgcolor=" + QString(szDataColor) + ">-</td>\n";

        // Error type
        if(bPatCritical)
            ptColor = szAlarmColor;	// Red background
        else
            ptColor = szDataColor;
        strLine += "<td width=\"10%%\" align=\"center\" bgcolor=" + QString(ptColor) + ">" + strErrorType + "</td>\n",

        // Comment (may include 'PAT CRITICAL' header if flag says so)
        strLine += "<td width=\"80%%\" bgcolor=" + QString(szDataColor) + ">";
        if(bPatCritical)
            strLine += "<b>PAT CRITICAL: </b>";

        strLine += strComment + "</td>\n</tr>\n";
    }

    fprintf(m_hReportFile,"%s", strLine.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Extract record
///////////////////////////////////////////////////////////
int GexFileAudit::StdfPatCompliance_ReadRecord(int /*nRecordType*/,
                                               GQTL_STDF::Stdf_Record* pclRecord,
                                               QString& /*strErrorMessage*/)
{
    // Read record, check if successful load.
    if(m_clStdfParse.ReadRecord(pclRecord))
        return 0;

    // Unexpected end of record....
    QString	strAsciiString;
    // Error reading STDF file. Dump data read and add an error message.
    // Make sure to get only data that has been read (FieldFlag_Present).
    pclRecord->GetAsciiRecord(strAsciiString, GQTL_STDF::Stdf_Record::FieldFlag_Present);

    // Keep track of this error type.
    m_ErrType_EndOfRecord++;

    // Check if error to be reported
    if(m_pFileAudit->bCheckEndRecords == false)
        return 0;
    if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_EndOfRecord > 1))
        return 0;

    // Build error message.
    QString strComment = "Unexpected end of record!!";
    if(m_bCsvOutput)
        strComment += ".";
    else
        strComment += "<br>";
    strComment += strAsciiString;
    StdfPatCompliance_LogFileRecordError(true,true,"End Of Record",strComment);

    // Error
    return 1;
}

///////////////////////////////////////////////////////////
// Extract MIR record
///////////////////////////////////////////////////////////
int GexFileAudit::StdfPatCompliance_ProcessMIR(void)
{
    // Extract Product name, program name, etc...
    if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfMIR) == false)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"MIR","Failed reading MIR record.");
        return 1;
    }

    // Check if this is a Flex/J750 data file (if so, we MAY need to remove leading pin# in test names!)
    if((m_iTotalSites > 1) &&
        ((m_clStdfMIR.m_cnTSTR_TYP == "J750") ||
        (m_clStdfMIR.m_cnTSTR_TYP == "IntegraFlex")))
    {
        m_FlexFormatMultiSites = true;
    }

    // Keep track of MIR count
    m_ErrType_MIR++;

    // Check if to report invalid records
    if((m_ErrType_MIR == 1) || (m_pFileAudit->bCheckBadRecords == false))
        return 0;

    // Do not report this error if already logged, and repport mode = Short
    if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_MIR > 1))
        return 0;


    // Report error
    StdfPatCompliance_LogFileRecordError(true,true,"MIR","MIR record appears multiple time. Only one MIR is allowed!");

    // Report Error
    return 1;
}

///////////////////////////////////////////////////////////
// Extract PTR record
///////////////////////////////////////////////////////////
int GexFileAudit::StdfPatCompliance_ProcessPTR(void)
{
    // Extract record
    if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfPTR) == false)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"PTR","Failed reading PTR record.");
        return 1;
    }

    // Get test-number and test name.
    QString strTestName = m_clStdfPTR.m_cnTEST_TXT;
    long	ldTestNumber = m_clStdfPTR.m_u4TEST_NUM;

    // Check if within PIR/PRR. If not, return warning
    if(m_InPIR == 0)
    {
        // Keep track of this error: PTR outside of PIR/PRR
        m_ErrType_PTR_No_PIR++;

        // Check if error to be reported
        if(m_pFileAudit->bCheckBadRecords == false)
            return 0;
        if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_PTR_No_PIR > 1))
            return 0;

        // Error reading STDF file
        QString strText = "This PTR record is outside of PIR/PRR bloc. This will make this test result ignored!";
        if(m_bCsvOutput)
            strText += ". ";
        else
            strText += "<br>";
        strText += "Test# " + QString::number(ldTestNumber);
        if(!strTestName.isEmpty())
          strText += " - " + strTestName;
        StdfPatCompliance_LogFileRecordError(true,false,"Outside PIR/PRR",strText);
        return 1;
    }

    // If no test name specified, nothing more to check!
    if(strTestName.isEmpty())
        return 0;

    // Test name specified, check if already seen in previous PTR
    if(m_clTestNames.find(ldTestNumber) == m_clTestNames.end())
    {
        // First time we have this test number: save its test name
        m_clTestNames[ldTestNumber] = strTestName;
    }
    else
    {
        // This test# has been seen previously, check if test name remains the same!
        if((m_clTestNames[ldTestNumber] != strTestName) && (m_clTestNames[ldTestNumber] != "galaxy test disabled"))
        {
            // Check if error to be reported
            if(m_pFileAudit->bCheckTestNames == false)
                return 0;

            // If short-report and fatal error already reported, exit now!
            if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestNameRootNameIssue > 1))
                return 0;

            // First: See if test name has leading pin#, in which case compare names without pin# info.
            bool	bValue1,bValue2;
            QString strSection1 = m_clTestNames[ldTestNumber].section(' ',-1,-1).trimmed();
            strSection1.toInt(&bValue1);
            QString strSection2 = strTestName.section(' ',-1,-1).trimmed();
            strSection2.toInt(&bValue2);
            if(bValue1 && bValue2)
            {
                QString strName1 = m_clTestNames[ldTestNumber].section(' ',0,-2).trimmed();
                QString strName2 = strTestName.section(' ',0,-2).trimmed();
                if(strName1 != strName2)
                {
                    // Record this error event
                    m_ErrType_TestNameRootNameIssue++;

                    QString strText = "Name mismatch (ignoring channel #) detected for same test#:";
                    if(m_bCsvOutput)
                        strText += ". ";
                    else
                        strText += "<br>";
                    strText += "Test# " + QString::number(ldTestNumber);
                    strText += " - " + strName1;
                    if(m_bCsvOutput)
                        strText += ", and ";
                    else
                        strText += "<br>";
                    strText += "Test# " + QString::number(ldTestNumber);
                    strText += " - " + strName2;

                    StdfPatCompliance_LogFileRecordError(true,true,"Test Name",strText);

                    // Disable this test# so no additional error returned on it.
                    m_clTestNames[ldTestNumber] = "galaxy test disabled";
                    return 1;

                }
            }

            // Error: test number found with different names!: keep track of this error type
            m_ErrType_TestFullNameIssue++;

            // If too many errors reported already, or flex data file (ignore discrepancies due to channel#)
            if( m_FlexFormatMultiSites || ((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestFullNameIssue > 1)))
                return 0;

            QString strText = "Multiple names detected for same test#:";
            if(m_bCsvOutput)
                strText += ". ";
            else
                strText += "<br>";
            strText += "Test# " + QString::number(ldTestNumber);
            strText += " - " + m_clTestNames[ldTestNumber];
            if(m_bCsvOutput)
                strText += ", and ";
            else
                strText += "<br>";
            strText += "Test# " + QString::number(ldTestNumber);
            strText += " - " + strTestName;

            StdfPatCompliance_LogFileRecordError(true,false,"Test Name",strText);

            // Disable this test# so no additional error returned on it.
            m_clTestNames[ldTestNumber] = "galaxy test disabled";
            return 1;
        }
    }

    // If we have a High Limit... check the test limits, do it!
    if(m_clStdfPTR.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHI_LIMIT))
    {
        if(m_clTestHighLimits.find(ldTestNumber) == m_clTestHighLimits.end())
            m_clTestHighLimits[ldTestNumber] = m_clStdfPTR.m_r4HI_LIMIT;
        else
        {
            // This test has aready limits set...see if they are different from previous ones!
            if((m_clTestHighLimits[ldTestNumber] != m_clStdfPTR.m_r4HI_LIMIT) && (m_clTestHighLimits[ldTestNumber] != LF_DISABLED_VALUE))
            {
                // Error: test number found with limits mismatch: keep track of this error type
                m_ErrType_TestLimitIssue++;

                // Check if error to be reported
                if(m_pFileAudit->bCheckTestLimits == false)
                    return 0;
                if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestLimitIssue > 1))
                    return 0;

                QString strText = "Multiple High Limits detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
                if(m_bCsvOutput)
                    strText += ". ";
                else
                    strText += "<br>";
                strText += "Old HL: " + QString::number(m_clTestHighLimits[ldTestNumber]) + m_clStdfPTR.m_cnUNITS;
                strText += " New HL: " + QString::number(m_clStdfPTR.m_r4HI_LIMIT) + m_clStdfPTR.m_cnUNITS;

                StdfPatCompliance_LogFileRecordError(true,false,"Test Limit",strText);

                // Disable this test# HighLimitso no additional error returned on it.
                m_clTestHighLimits[ldTestNumber] = LF_DISABLED_VALUE;
                return 1;
            }
        }
    }

    // If we have a Low Limit... check the test limits, do it!
    if(m_clStdfPTR.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLO_LIMIT))
    {
        if(m_clTestLowLimits.find(ldTestNumber) == m_clTestLowLimits.end())
            m_clTestLowLimits[ldTestNumber] = m_clStdfPTR.m_r4LO_LIMIT;
        else
        {
            // This test has aready limits set...see if they are different from previous ones!
            if((m_clTestLowLimits[ldTestNumber] != m_clStdfPTR.m_r4LO_LIMIT) && (m_clTestLowLimits[ldTestNumber] != LF_DISABLED_VALUE))
            {
                // Error: test number found with limits mismatch: keep track of this error type
                m_ErrType_TestLimitIssue++;

                // Check if error to be reported
                if(m_pFileAudit->bCheckTestLimits)
                    return 0;
                if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestLimitIssue > 1))
                    return 0;

                QString strText = "Multiple Low Limits detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
                if(m_bCsvOutput)
                    strText += ". ";
                else
                    strText += "<br>";
                strText += "Old LL: " + QString::number(m_clTestLowLimits[ldTestNumber]) + m_clStdfPTR.m_cnUNITS;
                strText += " New LL: " + QString::number(m_clStdfPTR.m_r4LO_LIMIT) + m_clStdfPTR.m_cnUNITS;

                StdfPatCompliance_LogFileRecordError(true,false,"Test Limit",strText);

                // Disable this test# LowLimit so no additional error returned on it.
                m_clTestLowLimits[ldTestNumber] = LF_DISABLED_VALUE;
                return 1;
            }
        }
    }

    // If Missing Low limit is a warning condition, check for it
    bool	bMissingLL=false,bMissingHL=false;

    if(m_pFileAudit->bCheckMissingLimits &&
        (m_clTestHighLimits[ldTestNumber] != LF_DISABLED_VALUE) &&
        (m_clStdfPTR.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposHI_LIMIT) == false))
    {
        // Error: test number found with limits missing: keep track of this error type
        m_ErrType_LimitMissingIssue++;

        // Check if error to be reported
        if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_LimitMissingIssue > 1))
            return 0;

        // Flag missing limit
        bMissingHL = true;
    }

    if(m_pFileAudit->bCheckMissingLimits &&
        (m_clTestLowLimits[ldTestNumber] != LF_DISABLED_VALUE) &&
        (m_clStdfPTR.IsFieldValid(GQTL_STDF::Stdf_PTR_V4::eposLO_LIMIT) == false))
    {
        // Error: test number found with limits missing: keep track of this error type
        m_ErrType_LimitMissingIssue++;

        // Check if error to be reported
        if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_LimitMissingIssue > 1))
            return 0;

        // Flag missing limit
        bMissingLL = true;
    }

    if(bMissingLL || bMissingHL)
    {
        QString strText;
        if(bMissingLL && bMissingHL)
        {
            strText = "Missing High & Low Limits for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
            // Disable this test# HighLimit & low so no additional error returned on it.
            m_clTestHighLimits[ldTestNumber] = m_clTestLowLimits[ldTestNumber] = LF_DISABLED_VALUE;
        }
        else
        if(bMissingHL)
        {
            strText = "Missing HIGH Limit detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
            // Disable this test# HighLimit &so no additional error returned on it.
            m_clTestHighLimits[ldTestNumber] = LF_DISABLED_VALUE;
        }
        else
        {
            strText = "Missing LOW Limit detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
            // Disable this test# LowLimit &so no additional error returned on it.
            m_clTestLowLimits[ldTestNumber] = LF_DISABLED_VALUE;
        }

        if(m_bCsvOutput)
            strText += ". ";
        else
            strText += "<br>";

        StdfPatCompliance_LogFileRecordError(true,false,"Test Limit",strText);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////
// Extract MPR record
///////////////////////////////////////////////////////////
int GexFileAudit::StdfPatCompliance_ProcessMPR(void)
{
    // Check if within PIR/PRR. If not, reutrn warning
    if(m_InPIR == 0)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"MPR","Failed reading MPR record.");
        return 1;
    }

    // Read MPR record
    if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfMPR) == false)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"MPR","Failed reading MPR record.");
        return 1;
    }

    // Get test-number and test name.
    QString strTestName = m_clStdfMPR.m_cnTEST_TXT;
    long	ldTestNumber = m_clStdfMPR.m_u4TEST_NUM;

    // If no test name specified, nothing more to check!
    if(strTestName.isEmpty())
        return 0;

    // Test name specified, check if already seen in previous PTR/MPR
    if(m_clTestNames.find(ldTestNumber) == m_clTestNames.end())
    {
        // First time we have this test number: save its test name
        m_clTestNames[ldTestNumber] = strTestName;
    }
    else
    {
        // This test# has been seen previously, check if test name remains the same!
        if((m_clTestNames[ldTestNumber] != strTestName) && (m_clTestNames[ldTestNumber] != "galaxy test disabled"))
        {
            // Check if error to be reported
            if(m_pFileAudit->bCheckTestNames == false)
                return 0;

            // If short-report and fatal error already reported, exit now!
            if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestNameRootNameIssue > 1))
                return 0;

            // First: See if test name has leading pin#, in which case compare names without pin# info.
            bool	bValue1,bValue2;
            QString strSection1 = m_clTestNames[ldTestNumber].section(' ',-1,-1).trimmed();
            strSection1.toInt(&bValue1);
            QString strSection2 = strTestName.section(' ',-1,-1).trimmed();
            strSection2.toInt(&bValue2);
            if(bValue1 && bValue2)
            {
                QString strName1 = m_clTestNames[ldTestNumber].section(' ',0,-2).trimmed();
                QString strName2 = strTestName.section(' ',0,-2).trimmed();
                if(strName1 != strName2)
                {
                    // Record this error event
                    m_ErrType_TestNameRootNameIssue++;

                    QString strText = "Name mismatch (ignoring channel #) detected for same test#:";
                    if(m_bCsvOutput)
                        strText += ". ";
                    else
                        strText += "<br>";
                    strText += "Test# " + QString::number(ldTestNumber);
                    strText += " - " + strName1;
                    if(m_bCsvOutput)
                        strText += ", and ";
                    else
                        strText += "<br>";
                    strText += "Test# " + QString::number(ldTestNumber);
                    strText += " - " + strName2;

                    StdfPatCompliance_LogFileRecordError(true,true,"Test Name",strText);

                    // Disable this test# so no additional error returned on it.
                    m_clTestNames[ldTestNumber] = "galaxy test disabled";
                    return 1;

                }
            }

            // Error: test number found with different names!: keep track of this error type
            m_ErrType_TestFullNameIssue++;

            // If too many errors reported already, or flex data file (ignore discrepancies due to channel#)
            if( m_FlexFormatMultiSites || ((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestFullNameIssue > 1)))
                return 0;

            QString strText = "Multiple names detected for same test#:";
            if(m_bCsvOutput)
                strText += ". ";
            else
                strText += "<br>";
            strText += "Test# " + QString::number(ldTestNumber);
            strText += " - " + m_clTestNames[ldTestNumber];
            if(m_bCsvOutput)
                strText += ", and ";
            else
                strText += "<br>";
            strText += "Test# " + QString::number(ldTestNumber);
            strText += " - " + strTestName;

            StdfPatCompliance_LogFileRecordError(true,false,"Test Name",strText);

            // Disable this test# so no additional error returned on it.
            m_clTestNames[ldTestNumber] = "galaxy test disabled";
            return 1;
        }
    }

    // If we have a High Limit... check the test limits, do it!
    if(m_clStdfMPR.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_LIMIT))
    {
        if(m_clTestHighLimits.find(ldTestNumber) == m_clTestHighLimits.end())
            m_clTestHighLimits[ldTestNumber] = m_clStdfMPR.m_r4HI_LIMIT;
        else
        {
            // This test has aready limits set...see if they are different from previous ones!
            if((m_clTestHighLimits[ldTestNumber] != m_clStdfMPR.m_r4HI_LIMIT) && (m_clTestHighLimits[ldTestNumber] != LF_DISABLED_VALUE))
            {
                // Error: test number found with limits mismatch: keep track of this error type
                m_ErrType_TestLimitIssue++;

                // Check if error to be reported
                if(m_pFileAudit->bCheckTestLimits == false)
                    return 0;
                if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestLimitIssue > 1))
                    return 0;

                QString strText = "Multiple High Limits detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
                if(m_bCsvOutput)
                    strText += ". ";
                else
                    strText += "<br>";
                strText += "Old HL: " + QString::number(m_clTestHighLimits[ldTestNumber]) + m_clStdfMPR.m_cnUNITS;
                strText += " New HL: " + QString::number(m_clStdfMPR.m_r4HI_LIMIT) + m_clStdfMPR.m_cnUNITS;

                StdfPatCompliance_LogFileRecordError(true,false,"Test Limit",strText);

                // Disable this test# HighLimitso no additional error returned on it.
                m_clTestHighLimits[ldTestNumber] = LF_DISABLED_VALUE;
                return 1;
            }
        }
    }

    // If we have a Low Limit... check the test limits, do it!
    if(m_clStdfMPR.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_LIMIT))
    {
        if(m_clTestLowLimits.find(ldTestNumber) == m_clTestLowLimits.end())
            m_clTestLowLimits[ldTestNumber] = m_clStdfMPR.m_r4LO_LIMIT;
        else
        {
            // This test has aready limits set...see if they are different from previous ones!
            if((m_clTestLowLimits[ldTestNumber] != m_clStdfMPR.m_r4LO_LIMIT) && (m_clTestLowLimits[ldTestNumber] != LF_DISABLED_VALUE))
            {
                // Error: test number found with limits mismatch: keep track of this error type
                m_ErrType_TestLimitIssue++;

                // Check if error to be reported
                if(m_pFileAudit->bCheckTestLimits)
                    return 0;
                if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestLimitIssue > 1))
                    return 0;

                QString strText = "Multiple Low Limits detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
                if(m_bCsvOutput)
                    strText += ". ";
                else
                    strText += "<br>";
                strText += "Old LL: " + QString::number(m_clTestLowLimits[ldTestNumber]) + m_clStdfMPR.m_cnUNITS;
                strText += " New LL: " + QString::number(m_clStdfMPR.m_r4LO_LIMIT) + m_clStdfMPR.m_cnUNITS;

                StdfPatCompliance_LogFileRecordError(true,false,"Test Limit",strText);

                // Disable this test# LowLimit so no additional error returned on it.
                m_clTestLowLimits[ldTestNumber] = LF_DISABLED_VALUE;
                return 1;
            }
        }
    }

    // If Missing Low limit is a warning condition, check for it
    bool	bMissingLL=false,bMissingHL=false;

    if(m_pFileAudit->bCheckMissingLimits &&
        (m_clTestHighLimits[ldTestNumber] != LF_DISABLED_VALUE) &&
        (m_clStdfMPR.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_LIMIT) == false))
    {
        // Error: test number found with limits missing: keep track of this error type
        m_ErrType_LimitMissingIssue++;

        // Check if error to be reported
        if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_LimitMissingIssue > 1))
            return 0;

        // Flag missing limit
        bMissingHL = true;
    }

    if(m_pFileAudit->bCheckMissingLimits &&
        (m_clTestLowLimits[ldTestNumber] != LF_DISABLED_VALUE) &&
        (m_clStdfMPR.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_LIMIT) == false))
    {
        // Error: test number found with limits missing: keep track of this error type
        m_ErrType_LimitMissingIssue++;

        // Check if error to be reported
        if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_LimitMissingIssue > 1))
            return 0;

        // Flag missing limit
        bMissingLL = true;
    }

    if(bMissingLL || bMissingHL)
    {
        QString strText;
        if(bMissingLL && bMissingHL)
        {
            strText = "Missing High & Low Limits for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
            // Disable this test# HighLimit & low so no additional error returned on it.
            m_clTestHighLimits[ldTestNumber] = m_clTestLowLimits[ldTestNumber] = LF_DISABLED_VALUE;
        }
        else
        if(bMissingHL)
        {
            strText = "Missing HIGH Limit detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
            // Disable this test# HighLimit &so no additional error returned on it.
            m_clTestHighLimits[ldTestNumber] = LF_DISABLED_VALUE;
        }
        else
        {
            strText = "Missing LOW Limit detected for Test#: " + QString::number(ldTestNumber) + " - " + strTestName;
            // Disable this test# LowLimit &so no additional error returned on it.
            m_clTestLowLimits[ldTestNumber] = LF_DISABLED_VALUE;
        }

        if(m_bCsvOutput)
            strText += ". ";
        else
            strText += "<br>";

        StdfPatCompliance_LogFileRecordError(true,false,"Test Limit",strText);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////
// Extract FTR record
///////////////////////////////////////////////////////////
int GexFileAudit::StdfPatCompliance_ProcessFTR(void)
{
    // Check if within PIR/PRR. If not, reutrn warning
    if(m_InPIR == 0)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"FTR","Failed reading FTR record.");
        return 1;
    }

    // Read FTR record
    if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfFTR) == false)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"FTR","Failed reading FTR record.");
        return 1;
    }

    // Get test-number and test name.
    QString strTestName = m_clStdfFTR.m_cnTEST_TXT;
    long	ldTestNumber = m_clStdfFTR.m_u4TEST_NUM;

    // If no test name specified, nothing more to check!
    if(strTestName.isEmpty())
        return 0;

    // Test name specified, check if already seen in previous PTR/MPR
    if(m_clTestNames.find(ldTestNumber) == m_clTestNames.end())
    {
        // First time we have this test number: save its test name
        m_clTestNames[ldTestNumber] = strTestName;
    }
    else
    {
        // This test# has been seen previously, check if test name remains the same!
        if((m_clTestNames[ldTestNumber] != strTestName) && (m_clTestNames[ldTestNumber] != "galaxy test disabled"))
        {
            // Check if error to be reported
            if(m_pFileAudit->bCheckTestNames == false)
                return 0;

            // If short-report and fatal error already reported, exit now!
            if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestNameRootNameIssue > 1))
                return 0;

            // First: See if test name has leading pin#, in which case compare names without pin# info.
            bool	bValue1,bValue2;
            QString strSection1 = m_clTestNames[ldTestNumber].section(' ',-1,-1).trimmed();
            strSection1.toInt(&bValue1);
            QString strSection2 = strTestName.section(' ',-1,-1).trimmed();
            strSection2.toInt(&bValue2);
            if(bValue1 && bValue2)
            {
                QString strName1 = m_clTestNames[ldTestNumber].section(' ',0,-2).trimmed();
                QString strName2 = strTestName.section(' ',0,-2).trimmed();
                if(strName1 != strName2)
                {
                    // Record this error event
                    m_ErrType_TestNameRootNameIssue++;

                    QString strText = "Name mismatch (ignoring channel #) detected for same test#:";
                    if(m_bCsvOutput)
                        strText += ". ";
                    else
                        strText += "<br>";
                    strText += "Test# " + QString::number(ldTestNumber);
                    strText += " - " + strName1;
                    if(m_bCsvOutput)
                        strText += ", and ";
                    else
                        strText += "<br>";
                    strText += "Test# " + QString::number(ldTestNumber);
                    strText += " - " + strName2;

                    StdfPatCompliance_LogFileRecordError(true,true,"Test Name",strText);

                    // Disable this test# so no additional error returned on it.
                    m_clTestNames[ldTestNumber] = "galaxy test disabled";
                    return 1;

                }
            }

            // Error: test number found with different names!: keep track of this error type
            m_ErrType_TestFullNameIssue++;

            // If too many errors reported already
            // Note: No channel# included in FTR from Flex testers...
            if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_TestFullNameIssue > 1))
                return 0;

            QString strText = "Multiple names detected for same test#:";
            if(m_bCsvOutput)
                strText += ". ";
            else
                strText += "<br>";
            strText += "Test# " + QString::number(ldTestNumber);
            strText += " - " + m_clTestNames[ldTestNumber];
            if(m_bCsvOutput)
                strText += ", and ";
            else
                strText += "<br>";
            strText += "Test# " + QString::number(ldTestNumber);
            strText += " - " + strTestName;

            StdfPatCompliance_LogFileRecordError(true,false,"Test Name",strText);

            // Disable this test# so no additional error returned on it.
            m_clTestNames[ldTestNumber] = "galaxy test disabled";
            return 1;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////
// Extract PPR record
///////////////////////////////////////////////////////////
int GexFileAudit::StdfPatCompliance_ProcessPRR(void)
{
    // Keep track of PIR/PRR netsting.
    m_InPIR--;

    // Read PRR record
    if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&m_clStdfPRR) == false)
    {
        // Error reading STDF file
        StdfPatCompliance_LogFileRecordError(true,true,"PRR","Failed reading PRR record.");
        return 1;
    }

    // If Die location checking disabled, return quietly!
    if(m_pFileAudit->bCheckDieLoc == false)
        return 0;

    if(m_clStdfPRR.IsFieldValid(GQTL_STDF::Stdf_PRR_V4::eposX_COORD) && m_clStdfPRR.IsFieldValid(GQTL_STDF::Stdf_PRR_V4::eposY_COORD))
    {
        if((m_clStdfPRR.m_i2X_COORD == -32768) || (m_clStdfPRR.m_i2Y_COORD == -32768))
        {
            // Invalid die coordinates specified
            m_ErrType_PRR_No_DieLoc++;
            return 1;
        }
        // Valid DieXY loc info.
        return 0;
    }
    else
    {
        // DieXY, not evailable....keep track of total exceptions found
        m_ErrType_PRR_No_DieLoc++;
        return 1;
    }

}

///////////////////////////////////////////////////////////
// Check STDF file compliancy: PAT and Data sets.
///////////////////////////////////////////////////////////
void GexFileAudit::DisplayHeaderDetails(void)
{
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    CGexFileInGroup *pFile   = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    QString strProduct = (*pFile->getMirDatas().szPartType) ? pFile->getMirDatas().szPartType: "n/a";;

    // Before writing table with all errors detected, write global info (file name, Product, Lot, etc...)
    if(m_bCsvOutput)
    {
            fprintf(m_hReportFile,"Field,Value\n");
            fprintf(m_hReportFile,"---------,-----------------------\n");
            fprintf(m_hReportFile,"File,%s\n",pFile->strFileName.toLatin1().constData());
            fprintf(m_hReportFile,"Product,%s\n",strProduct.toLatin1().constData());
            if(*pFile->getMirDatas().szLot)
                fprintf(m_hReportFile,"LotID,%s\n",pFile->getMirDatas().szLot);
            if(*pFile->getWaferMapData().szWaferID)
                fprintf(m_hReportFile,"WaferID,'%s\n",pFile->getWaferMapData().szWaferID);
            if(*pFile->getMirDatas().szJobName)
                fprintf(m_hReportFile,"Program name,%s\n",pFile->getMirDatas().szJobName);
            if(*pFile->getMirDatas().szJobRev)
                fprintf(m_hReportFile,"Program Rev.,'%s\n",pFile->getMirDatas().szJobRev);

            // Skip line
            fprintf(m_hReportFile,"\n");
    }
    else
    {
        // open table
        fprintf(m_hReportFile,"<table border=\"0\" width=\"98%%\">\n");

        // table header
        fprintf(m_hReportFile,"<tr>\n");
        fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>Field</td>\n",szFieldColor);
        fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>Value</td>\n",szFieldColor);
        fprintf(m_hReportFile,"</tr>\n");

        // File
        fprintf(m_hReportFile,"<tr>\n");
        fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>File</td>\n",szFieldColor);
        fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,pFile->strFileName.toLatin1().constData());
        fprintf(m_hReportFile,"</tr>\n");

        // Product
        fprintf(m_hReportFile,"<tr>\n");
        fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>Product</td>\n",szFieldColor);
        fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,strProduct.toLatin1().constData());
        fprintf(m_hReportFile,"</tr>\n");

        // Lot ID
        if(*pFile->getMirDatas().szLot)
        {
            fprintf(m_hReportFile,"<tr>\n");
            fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>LotID</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,pFile->getMirDatas().szLot);
            fprintf(m_hReportFile,"</tr>\n");
        }

        // Wafer ID
        if(*pFile->getWaferMapData().szWaferID)
        {
            fprintf(m_hReportFile,"<tr>\n");
            fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>WaferID</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,pFile->getWaferMapData().szWaferID);
            fprintf(m_hReportFile,"</tr>\n");
        }

        // Job Name
        if(*pFile->getMirDatas().szJobName)
        {
            fprintf(m_hReportFile,"<tr>\n");
            fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>Program name</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,pFile->getMirDatas().szJobName);
            fprintf(m_hReportFile,"</tr>\n");
        }

        // Job Rev
        if(*pFile->getMirDatas().szJobRev)
        {
            fprintf(m_hReportFile,"<tr>\n");
            fprintf(m_hReportFile,"<td width=\"10%%\" bgcolor=%s>Program rev.</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"90%%\" bgcolor=%s>%s</td>\n",szDataColor,pFile->getMirDatas().szJobRev);
            fprintf(m_hReportFile,"</tr>\n");
        }
        // Close header table
        fprintf(m_hReportFile,"</table>\n<br>\n<br>\n");
    }
}

bool GexFileAudit::CheckStdfFileRecordsAudit(QString &strStdfFile)
{
    GSLOG(5, QString("Check Stdf File RecordsAudit '%1'").arg(strStdfFile).toLatin1().constData() );
    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
      //  OEM mode, refuse to run this function!
      GS::Gex::Message::information(
          "", "This function is disabled in teradyne mode\n\nContact " +
          QString(GEX_EMAIL_SALES) + " for more information!");
        return false;
    }

    // Clear variables
    int iStatus=0;
    int nRecordType=0;
    QString	strErrorMessage;
    m_clTestNames.clear();
    m_clTestLowLimits.clear();
    m_clTestHighLimits.clear();
    m_InPIR = 0;
    m_InWIR = false;
    m_iTotalLogRecordErrors=0;
    CSiteBlockMap lMappingSiteHead;
    unsigned int siteHeadIndex;

    // Progress bar status line
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("File Audit: Data file records");
    QCoreApplication::processEvents();

    // Force HTML page change if creating regular HTML report
    CheckHtmlPageChange(true,"records");

    // Section Title
    WriteSectionTitle("File records audit");

    // Open STDF file to read...
    if(m_clStdfParse.Open(strStdfFile.toLatin1().constData()) == false)
    {
        fprintf(m_hReportFile,"Failed reading data file!\n");
        return false;
    }

    // Read one record from STDF file.
    iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    while((iStatus == GQTL_STDF::StdfParse::NoError) || (iStatus == GQTL_STDF::StdfParse::UnexpectedRecord))
    {
        // Process STDF record read.
        switch(nRecordType)
        {
            case GQTL_STDF::Stdf_Record::Rec_FAR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfFAR, strErrorMessage);
                break;

            case GQTL_STDF::Stdf_Record::Rec_ATR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfATR, strErrorMessage);
                break;

            case GQTL_STDF::Stdf_Record::Rec_MIR:
                GexFileAudit::StdfPatCompliance_ProcessMIR();
                break;

            case GQTL_STDF::Stdf_Record::Rec_MRR:
                m_ErrType_MRR++;	// Keep track of MRR (at least one is mandatory)
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfMRR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PCR:
                m_ErrType_PCR++;	// Keep track of PCR (at least one is mandatory)
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPCR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_HBR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfHBR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SBR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfSBR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PMR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPMR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PGR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPGR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PLR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPLR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RDR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfRDR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_SDR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfSDR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WIR:
                m_InWIR = true;
                m_ErrType_WIR++;	// Keep track of WIR found.
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfWIR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WRR:
                m_InWIR = false;
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfWRR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_WCR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfWCR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PIR:
                m_InPIR++;
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfPIR, strErrorMessage);
                lMappingSiteHead[(GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn(m_clStdfParse, m_clStdfPIR) << 16) |
                                  GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn(m_clStdfParse, m_clStdfPIR)] = 1;
                break;
            case GQTL_STDF::Stdf_Record::Rec_PRR:
                StdfPatCompliance_ProcessPRR();
                siteHeadIndex = (GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn( m_clStdfParse, m_clStdfPRR ) << 16) |
                                 GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn( m_clStdfParse, m_clStdfPRR );
                if (lMappingSiteHead.find(siteHeadIndex) ==
                    lMappingSiteHead.end())
                {
                    lMappingSiteHead[siteHeadIndex] = 0;
                    StdfPatCompliance_LogFileRecordError(
                        true, false, "PIR PRR mismatch",
                        "Cannot find (PRR.head_num, PRR.site_num)"
                        " in PIR records");
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_TSR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfTSR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_PTR:
                GexFileAudit::StdfPatCompliance_ProcessPTR();
                siteHeadIndex = (GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn( m_clStdfParse, m_clStdfPTR ) << 16) |
                                 GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn( m_clStdfParse, m_clStdfPTR );
                if (lMappingSiteHead.find(siteHeadIndex) ==
                    lMappingSiteHead.end())
                {
                    lMappingSiteHead[siteHeadIndex] = 0;
                    StdfPatCompliance_LogFileRecordError(
                        true, false, "PIR PTR mismatch",
                        "Cannot find (PTR.head_num, PTR.site_num)"
                        " in PIR records");
                }
                break;
            case GQTL_STDF::Stdf_Record::Rec_MPR:
                GexFileAudit::StdfPatCompliance_ProcessMPR();
                break;
            case GQTL_STDF::Stdf_Record::Rec_FTR:
                GexFileAudit::StdfPatCompliance_ProcessFTR();
                break;
            case GQTL_STDF::Stdf_Record::Rec_BPS:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfBPS, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_EPS:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfEPS, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_GDR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfGDR, strErrorMessage);

                // modify site number deciphering mode if needed
                GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInParser( m_clStdfGDR, m_clStdfParse );
                break;
            case GQTL_STDF::Stdf_Record::Rec_DTR:
                GexFileAudit::StdfPatCompliance_ReadRecord(nRecordType, (GQTL_STDF::Stdf_Record*)&m_clStdfDTR, strErrorMessage);
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
                break;
            case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
                break;
            case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
                // Get STDF Type & Sub type
                int i_stdf_RecType, i_stdf_RecSubType;
                m_clStdfParse.GetRecordType(&i_stdf_RecType,&i_stdf_RecSubType);

                // Keep track of unknown records
                m_ErrType_UnknownRec++;

                // Check if error to be reported
                if(m_pFileAudit->bCheckBadRecords == false)
                    break;

                if((m_pFileAudit->bLongReportMode == false) && (m_ErrType_UnknownRec > 1))
                    break;

                // Build error message
                QString strText = "Unknown STDF record. File corrupted (data lost).";
                if(m_bCsvOutput == false)
                    strText += "<br>";
                strText += "Record Type:" + QString::number(i_stdf_RecType) + "  ";
                if(m_bCsvOutput == false)
                    strText += "<br>";
                strText += "Record sub-type: " + QString::number(i_stdf_RecSubType);

                StdfPatCompliance_LogFileRecordError(true,false,"Unknown Record",strText);
                break;
        }

        // Read one record from STDF file.
        iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);
    };

    // Close input STDF file
    m_clStdfParse.Close();

    // Check for global error (eg: missing MIR, etc)
    if(m_pFileAudit->bCheckMissingRecords)
    {
        // Check if mandatory records are missing
        if(m_ErrType_MIR == 0)
            StdfPatCompliance_LogFileRecordError(false,true,"MIR Missing","The MIR record is mandatory but is missing in your file!");
        if(m_ErrType_WIR == 0)
            StdfPatCompliance_LogFileRecordError(false,true,"WIR Missing","The WIR record is mandatory for Wafer-Sort data (holds Wafer ID)");
        if(m_ErrType_PCR == 0)
            StdfPatCompliance_LogFileRecordError(false,true,"PCR Missing","The PCR record is mandatory but is missing in your file!");
        if(m_ErrType_MRR == 0)
            StdfPatCompliance_LogFileRecordError(false,true,"MRR Missing","The MRR record is mandatory but is missing in your file!. As this missing record must appear last, your file may be truncated and missing data.");
    }

    if(m_pFileAudit->bCheckProductID && (m_clStdfMIR.m_cnPART_TYP.isEmpty()))
        StdfPatCompliance_LogFileRecordError(false,true,"ProductID","No Product ID defined in your file! Your file should have its fields MIR.PART_TYP set to a valid Product name.");
    if(m_pFileAudit->bCheckLotID && (m_clStdfMIR.m_cnLOT_ID.isEmpty()))
        StdfPatCompliance_LogFileRecordError(false,true,"LotID","No LotID defined in your file! Your file should have its fields MIR.LOT_ID set to a valid Lot ID.");
    if(m_pFileAudit->bCheckWaferID && (m_clStdfWIR.m_cnWAFER_ID.isEmpty()))
        StdfPatCompliance_LogFileRecordError(false,true,"WaferID","No WaferID defined in your file! Your file should have its fields WIR.WAFER_ID set to a valid Wafer ID.");
    if(m_ErrType_PRR_No_DieLoc)
        StdfPatCompliance_LogFileRecordError(false,true,"Die XY","Missing Die XY coordinates. Your file should have its fields PRR.X_COORD and PRR.Y_COORD set to valid die locations.");

    // Report is in 'Short' mode, state it in the report so user knows more options are available.
    if(m_pFileAudit->bLongReportMode == false)
    {
        StdfPatCompliance_LogFileRecordError(false,false,"Short Report","Know that this is a 'Short Report' where only first instance of each error type is reported. For a 'Long Report' reporting the complete list of issues (this can be very long!), change the appropriate setting from the 'Settings' page (clicking the 'Wizard >>' button)");
    }

    QString of=gexReport->getReportOptions()->GetOption("output", "format").toString();

    // If errors reported...
    if(m_iTotalLogRecordErrors)
    {
        // Close error table (if HTML report)
        if(m_bCsvOutput == false)
        {
            fprintf(m_hReportFile,"</table>\n");

            // Write HTML URL links

            if(of == "HTML")
                fprintf(m_hReportFile,"<p align=\"center\"><a href=\"index.htm\">Home</a><br>\n");
        }

        // Return: failures detected
        return false;
    }


    // No error reported...
    StdfPatCompliance_LogFileRecordError(false,false,"SUCCESS","No error detected!");

    // Write HTML URL links
    if (of=="HTML") //if(gexReport->pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        fprintf(m_hReportFile,"<p align=\"center\"><a href=\"index.htm\">Home</a><br>\n");

    // Return: success
    return true;
}

///////////////////////////////////////////////////////////
// Truncate names to short names if option is set...
///////////////////////////////////////////////////////////
QString GexFileAudit::strTruncateName(QString &strName)
{
    QString strNewName = strName;

    QString tn=gexReport->getReportOptions()->GetOption("output","truncate_names").toString();

    bool ok; int tl=tn.toInt(&ok); if (!ok) tl=-1;
    if (tn=="yes") tl=32;

    if((tl > 0) && ((int)strName.length() >= tl))
    {
        int iValue = gex_max(4,tl/2) - 3;
        strNewName = strName.left(iValue) + "..." + strName.right(iValue);
    }
    return strNewName;
}


///////////////////////////////////////////////////////////
// Log error message (errors when analyzing Parameters)
///////////////////////////////////////////////////////////
void GexFileAudit::StdfPatCompliance_LogFileDatasetError(CTest *ptTestCell,bool bShowHistogram,QString strErrorType,QString strComment)
{
    QString strLine;

    QString strOutputFormat=gexReport->getReportOptions()->GetOption("output", "format").toString();
    //int		iTextSize = (gexReport->pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML) ? 3: 2;
    int iTextSize = (strOutputFormat=="HTML")? 3:2;


    // Check if page break condition (only applies to Word, PDF or PPT)
    int		iChartPerPage=2;
    bool	bInsertPageBreak=false;
    if ( m_iTotalLogDatasetErrors
         && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
         // && (gexReport->pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML))
       )
    {
        QString pf=gexReport->getReportOptions()->GetOption("output", "paper_format").toString();

        // Define total charts per page allowed...
        if (strOutputFormat=="PDF")
        {
            //iChartPerPage = (gexReport->pReportOptions->bPortraitFormat) ? 5 : 2;
            // PDF: 5 charts/page in Portrait mode, 2 inlandscape mode
            iChartPerPage = (pf=="portrait") ? 5 : 2;
        }
        else if (strOutputFormat=="DOC"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
        {
            //iChartPerPage = (gexReport->pReportOptions->bPortraitFormat) ? 3 : 2;
            // Word,PPT: 3 charts/page in Portrait mode, 2 inlandscape mode
            iChartPerPage = (pf=="portrait") ? 3 : 2;
        }

        /*
        switch(gexReport->pReportOptions->iOutputFormat)
        {
            case GEX_OPTION_OUTPUT_PDF:
                //iChartPerPage = (gexReport->pReportOptions->bPortraitFormat) ? 5 : 2;
                // PDF: 5 charts/page in Portrait mode, 2 inlandscape mode
                iChartPerPage = (gexReport->pReportOptions->m_OutputOptionsMap["paper_format"]=="portrait") ? 5 : 2;
                break;
            case GEX_OPTION_OUTPUT_WORD:
            case GEX_OPTION_OUTPUT_PPT:
                //iChartPerPage = (gexReport->pReportOptions->bPortraitFormat) ? 3 : 2;
                // Word,PPT: 3 charts/page in Portrait mode, 2 inlandscape mode
                iChartPerPage = (gexReport->pReportOptions->m_OutputOptionsMap["paper_format"]=="portrait") ? 3 : 2;
                break;
        }
        */
        // Check if reached page-break condition
        if(m_iTotalLogDatasetErrors % iChartPerPage == 0)
            bInsertPageBreak = true;
    }

    // Keep track of total errors logged
    m_iTotalLogDatasetErrors++;

    // If first error, display table header !
    if(m_iTotalLogDatasetErrors == 1)
    {
        // Title
        strLine = "Dataset analysis:";

        // If multi-sites data, display site name too.
        if(gexReport->getGroupsList().count() > 1)
            strLine += m_pGroup->strGroupName;

        if(m_bCsvOutput == false)
        {
            fprintf(m_hReportFile,"<h1 align=\"left\"><font color=\"#006699\">%s</font></h1>\n",strLine.toLatin1().constData());
            fprintf(m_hReportFile,"<table border=\"1\" bordercolor=\"#DCDCDC\" width=\"98%%\">\n");
            fprintf(m_hReportFile,"<tr>\n");
            fprintf(m_hReportFile,"<td width=\"5%%\" align=\"center\" bgcolor=%s>Test#</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"25%%\" align=\"center\" bgcolor=%s>Test Name</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"10%%\" align=\"center\" bgcolor=%s>Analysis type</td>\n",szFieldColor);
            fprintf(m_hReportFile,"<td width=\"60%%\" bgcolor=%s>Details / Comments</td>\n",szFieldColor);
            fprintf(m_hReportFile,"</tr>\n");
        }
        else
        {
            fprintf(m_hReportFile,"\n%s\n",strLine.toLatin1().constData());
            fprintf(m_hReportFile,"Test#,Test name,Analysis Type,Details\n");
            fprintf(m_hReportFile,"--------------,---------------,---------------,----------------------\n");
        }
    }
    else
    // Every two lines (if landscape mode) or 4 lines (portrait page), insert page break if FLAT HTML (Word,PDF,PPT)
    //if((gexReport->pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML) && (bInsertPageBreak))
        if ( ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT"||strOutputFormat=="ODT") )
            &&  (bInsertPageBreak)
        )
    {
        // Close table
        fprintf(m_hReportFile,"</table>\n");

        // Insert page break
        gexReport->WritePageBreak();

        // Open table again
        fprintf(m_hReportFile,"<table border=\"1\" width=\"98%%\">\n");
    }

    if(m_bCsvOutput)
    {
        // Report is CSV: <Test#>,<Test name>,<Error type>,<Comment>
        if(ptTestCell == NULL)
            fprintf(m_hReportFile,",,%s,%s\n",strErrorType.toLatin1().constData(),strComment.toLatin1().constData());	// Comment, not a test related message
        else
            fprintf(m_hReportFile,"%d,%s,%s,%s\n",ptTestCell->lTestNumber,strTruncateName(ptTestCell->strTestName).toLatin1().constData(),strErrorType.toLatin1().constData(),strComment.toLatin1().constData());
    }
    else
    {
        QString strHyperlink;

        // Open table line
        fprintf(m_hReportFile,"<tr>\n");


        if(ptTestCell == NULL)
        {
            // Comment, not a test related message
            fprintf(m_hReportFile,"<td width=\"5%%\" align=\"center\" bgcolor=%s>&nbsp;</td>\n",szDataColor);
            fprintf(m_hReportFile,"<td width=\"25%%\" bgcolor=%s>&nbsp;</td>\n",szDataColor);
        }
        else
        {
            // If HTML report: create hyperlink to Intercative Histogram
            //if(gexReport->pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
            if (strOutputFormat=="HTML")
                strHyperlink.sprintf("<a href=\"#_gex_drill--drill_chart=adv_histo--data=%s\">",ptTestCell->szTestLabel);

            // Open table
            fprintf(m_hReportFile,"<td width=\"5%%\" align=\"center\" bgcolor=%s>",szDataColor);

            // Hyperlink definition (if any)
            if(strHyperlink.isEmpty() == false)
                fprintf(m_hReportFile,"%s",strHyperlink.toLatin1().constData());

            // Test#
            fprintf(m_hReportFile,"%d",ptTestCell->lTestNumber);

            // Close hyperlink definition
            if(strHyperlink.isEmpty() == false)
                fprintf(m_hReportFile,"</a>");

            fprintf(m_hReportFile,"</td>\n");
            fprintf(m_hReportFile,"<td width=\"25%%\" bgcolor=%s><font size=\"%d\">%s",szDataColor,iTextSize,strTruncateName(ptTestCell->strTestName).toLatin1().constData());

            if(bShowHistogram)
            {
                // If Histpgram show, then also display some statistics.
                fprintf(m_hReportFile,"<br><br>Low limit : %s",ptTestCell->GetCurrentLimitItem()->szLowL);
                fprintf(m_hReportFile,"<br>High limit: %s",ptTestCell->GetCurrentLimitItem()->szHighL);
                fprintf(m_hReportFile, "<br>Samples: %d",
                        ptTestCell->ldSamplesValidExecs);
                fprintf(m_hReportFile,"<br>Mean: %s",m_pFile->FormatTestResult(ptTestCell,ptTestCell->lfMean,ptTestCell->res_scal));
                fprintf(m_hReportFile,"<br>Sigma: %s",m_pFile->FormatTestResult(ptTestCell,ptTestCell->lfSigma,ptTestCell->res_scal));
                fprintf(m_hReportFile,"<br>Cp: %s",gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
                fprintf(m_hReportFile,"<br>Cpk: %s",gexReport->CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
            }
            fprintf(m_hReportFile,"</font></td>\n");

        }
        fprintf(m_hReportFile,"<td width=\"10%%\" align=\"center\" bgcolor=%s>%s</td>\n",szDataColor,strErrorType.toLatin1().constData());
        fprintf(m_hReportFile,"<td width=\"60%%\" bgcolor=%s><font size=\"%d\">%s</font>",szDataColor,iTextSize,strComment.toLatin1().constData());

        // Include Histogram image if requested...
        if(bShowHistogram)
        {
            // Build histogram image: build Image full path where to save the chart plot.
            QString strImage = gexReport->
                BuildImageUniqueName(gexReport->
                                     getReportOptions()->strReportDirectory +
                                     "/images/adv_h_", ptTestCell);
            strImage += m_pGroup->strGroupName;
            QString strImagePath = gexReport->getReportOptions()->strReportDirectory;
            strImagePath += "/images/";
            strImagePath += strImage;

            // Create Chart (paint into Widget)
            int iChartSize = GEX_CHARTSIZE_MEDIUM;//gexReport->pReportOptions->iHistoChartSize;
            // Create one single layer in layer list
            gexReport->m_pChartsInfo->removeCharts();

            CGexSingleChart *	pLayer = new CGexSingleChart(0);

            pLayer->iGroupX = m_uGroupID;
            pLayer->iTestNumberX = ptTestCell->lTestNumber;
            pLayer->iPinMapX = ptTestCell->lPinmapIndex;
            pLayer->strTestLabelX.sprintf("T%d : ",ptTestCell->lTestNumber);
            pLayer->strTestLabelX += ptTestCell->strTestName;
            pLayer->strTestNameX = ptTestCell->strTestName;
            gexReport->m_pChartsInfo->addChart(pLayer);
            // Reset viewport to current layer!
            gexReport->m_pChartsInfo->lfZoomFactorX = gexReport->m_pChartsInfo->lfZoomFactorY = -1.0;

            // Build chart from layer structure
            gexReport->CreateAdvHistoChartImageEx(gexReport->m_pChartsInfo,ptTestCell,iChartSize,false,gexReport->getReportOptions()->getAdvancedReportSettings(),strImagePath);

            // Write HTML code to display Histogram chart.
            fprintf(m_hReportFile,"<br>");

            // Hyperink to Interactive Charts
            if(strHyperlink.isEmpty() == false)
                fprintf(m_hReportFile,"%s",strHyperlink.toLatin1().constData());

            // Histogram chart
            fprintf(m_hReportFile,"<img border=\"0\" src=\"../images/%s\">", formatHtmlImageFilename(strImage).toLatin1().constData());

            // Clos hyperlink (if any)
            if(strHyperlink.isEmpty() == false)
                fprintf(m_hReportFile,"</a>");
        }
        fprintf(m_hReportFile,"</td>\n");
        fprintf(m_hReportFile,"</tr>\n");
    }
}

///////////////////////////////////////////////////////////
// Check/Force HTML page change if creating regular HTML report
///////////////////////////////////////////////////////////
void GexFileAudit::CheckHtmlPageChange(bool /*bForceChange = false*/,
                                       QString strPageName /*= ""*/)
{
    // Check if standard HTML report output or not...
    QString strOutputFormat=gexReport->getReportOptions()->GetOption("output","format").toString();

    //if((gexReport->pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML) == 0)
    if (strOutputFormat!="HTML")
        return;

    // HTML page count written
    m_ReportPageIndex++;

    // Write HTML URL links
    fprintf(m_hReportFile,"<p align=\"center\"><a href=\"index.htm\">Home</a> - <a href=\"page_%d.htm\">Next page &gt;&gt;</a><br>\n",m_ReportPageIndex);
    fprintf(m_hReportFile,"<b><a href=\"records.htm\"><font size=\"5\">File records audit</font></b></a>\n");

    fprintf(m_hReportFile,"<br><br></p></body>\n</html>\n");

    // close current report page
    fclose(m_hReportFile);

    // Open next page
    QFileInfo cFileInfo(gexReport->reportAbsFilePath());
    QString strFile = cFileInfo.absolutePath();
    if(strPageName.isEmpty())
        strFile += "/page_" + QString::number(m_ReportPageIndex);
    else
        strFile += "/" + strPageName;
    strFile += ".htm";

    m_hReportFile = fopen(strFile.toLatin1().constData(),"wt");
    if(m_hReportFile == NULL)
        return;

    // Ensure new file handle is updated in gexReport structure
    gexReport->setReportFile(m_hReportFile);
    gexReport->WriteHeaderHTML(m_hReportFile,"#000000");	// Default: Text is Black
}

///////////////////////////////////////////////////////////
// Writes section title
///////////////////////////////////////////////////////////
void GexFileAudit::WriteSectionTitle(QString strTitle)
{
    if(m_bCsvOutput == false)
        fprintf(m_hReportFile,"<h1 align=\"left\"><font color=\"#006699\">%s</font></h1>\n",strTitle.toLatin1().constData());
    else
        fprintf(m_hReportFile,"\n%s\n",strTitle.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Check all parameters in a given group (ditribution shape, zero range, etc)
///////////////////////////////////////////////////////////
bool GexFileAudit::CheckStdfFileDatasetAudit(unsigned uGroupID)
{
    QString strText;
    int		iShape;
    CGexStats cStats;

    // Ensure we use latest options set
    cStats.UpdateOptions(gexReport->getReportOptions());

    // Clear Dataset/parameters error count in group
    m_iTotalLogDatasetErrors = 0;

    // Save handle to group# & file (testing site)
    m_uGroupID = uGroupID;
    m_pGroup = (uGroupID >= (unsigned int) gexReport->getGroupsList().size()) ?
               NULL : gexReport->getGroupsList().at(uGroupID);
    m_pFile = (m_pGroup->pFilesList.isEmpty()) ? NULL : m_pGroup->pFilesList.first();

    // Build list of filters that match the product criteria
    QString	strProduct = m_pFile->getMirDatas().szPartType;	// Product name for the file analyzed (MIR.PART_TYP)
    QRegExp rx("", Qt::CaseInsensitive);	// NOT case sensitive.

    QList<GS::Gex::AuditParameterFilter*> cMatchingTestFilters;
    QList<GS::Gex::AuditParameterFilter*>::iterator itBegin =
        m_pFileAudit->cTestFilters.begin();
    QList<GS::Gex::AuditParameterFilter*>::iterator itEnd =
        m_pFileAudit->cTestFilters.end();

    while(itBegin != itEnd)
    {
        // If wildcar used, set its support.
        if((*itBegin)->strProduct.indexOf("*") >= 0 || (*itBegin)->strProduct.indexOf("?") >= 0)
            rx.setPatternSyntax(QRegExp::Wildcard);
        else
            rx.setPatternSyntax(QRegExp::RegExp);
        rx.setPattern((*itBegin)->strProduct);

        // If pattern match, keep this filter entry as it'll be appliexd to each test
        if(strProduct.isEmpty() || (rx.indexIn(strProduct) >= 0))
            cMatchingTestFilters.append((*itBegin));

        itBegin++;
    }

    // Make RegEx case sensitive (for test name analysis)
    rx.setCaseSensitivity(Qt::CaseSensitive);

    // Progress bar status line
    QString strStatusLine = "File Audit: " + m_pGroup->strGroupName + "...";
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(strStatusLine);
    QCoreApplication::processEvents();

    QString f=gexReport->getReportOptions()->GetOption("statistics", "cp_cpk_computation").toString();

    // List of tests in this group.
    CTest *ptTestCell = m_pGroup->cMergedData.ptMergedTestList;
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    while(ptTestCell != NULL)
    {
        // Check all optential problems for this test...
        // If neither PTR values, this test was not executed !
        if((ptTestCell->ldExecs == 0) && (ptTestCell->GetCurrentLimitItem()->ldOutliers == 0))
            goto NextTestCell;

        // If no samples
        if(ptTestCell->ldSamplesValidExecs == 0)
            goto NextTestCell;

        // IF Muti-result parametric test, do not show master test record
        if(ptTestCell->lResultArraySize > 0)
            goto NextTestCell;

        // If not a parametric / multiparametric (eg: functional) test, ignore!
        if(ptTestCell->bTestType == 'F')
            goto NextTestCell;

        // If custom Galaxy parameter (die loc, bin#, et), simply skip test!
        if(ptTestCell->bTestType == '-')
            goto NextTestCell;

        // Samples limit defined? (if so, only analyze tests with enough samples)
        if((m_pFileAudit->ldSamplesLimit >= 0) && (ptTestCell->ldSamplesValidExecs < m_pFileAudit->ldSamplesLimit))
            goto NextTestCell;

        // Cpk limit defined? (if so, only analyze tests with low Cpk)
        if((m_pFileAudit->lfCpkLimit >= 0) && (ptTestCell->GetCurrentLimitItem()->lfCpk >= m_pFileAudit->lfCpkLimit))
            goto NextTestCell;

        // Check for custom test filter (based on test name or test number)
        itBegin	= cMatchingTestFilters.begin();
        itEnd	= cMatchingTestFilters.end();

        while (itBegin != itEnd)
        {
            switch((*itBegin)->iFilterType)
            {
                case	GEX_AUDIT_FILTER_TNAME:
                    if((*itBegin)->strFilter.indexOf("*") >= 0 || (*itBegin)->strFilter.indexOf("?") >= 0)
                        rx.setPatternSyntax(QRegExp::Wildcard);
                    else
                        rx.setPatternSyntax(QRegExp::RegExp);
                    rx.setPattern((*itBegin)->strFilter);

                    // If pattern match, keep this filter entry as it'll be appliexd to each test
                    if(rx.indexIn(ptTestCell->strTestName) >= 0)
                        goto NextTestCell;	// This test belongs to the list of tests to ignore!
                    break;

                case	GEX_AUDIT_FILTER_TNUMBER:

                    if((*itBegin)->pTestList->Contains(ptTestCell->lTestNumber))
                        goto NextTestCell;	// This test belongs to the list of tests to ignore!
                    break;
            }

            itBegin++;
        }


        // Ensure Advanced statistics are computed (thay may not be computed yet if data size exceeds cut-off limit in options)
        cStats.ComputeAdvancedDataStatistics(ptTestCell,false, f=="percentile"?true:false,false); //cStats.ComputeAdvancedDataStatistics(ptTestCell,false,gexReport->pReportOptions->bStatsCpCpkPercentileFormula,false);

        if (lPatInfo)
        {
            iShape = patlib_GetDistributionType(ptTestCell,
                                                lPatInfo->GetRecipeOptions().iCategoryValueCount,
                                                lPatInfo->GetRecipeOptions().bAssumeIntegerCategory,
                                                lPatInfo->GetRecipeOptions().mMinConfThreshold);
        }
        else
            iShape = patlib_GetDistributionType(ptTestCell);

        // Check for: 'Zero' range?
        if(m_pFileAudit->bCheckZeroRange && (ptTestCell->lDifferentValues == 0))
        {
            strText = "All values for this test are identical (Zero-range) and equal to: ";
            strText += QString::number(ptTestCell->lfSamplesMin);
            strText += ptTestCell->szTestUnits;
            StdfPatCompliance_LogFileDatasetError(ptTestCell,m_pFileAudit->bShowHistogram,"Zero Range",strText);
            goto NextTestCell;
        }

        // Check for: Categories?
        if(m_pFileAudit->bCheckCategories)
        {
            // Only integer values?
            if(ptTestCell->bIntegerValues)
            {
                strText.sprintf("Category distribution exclusively made of INTEGER values (%ld different integer values over the %d samples)",ptTestCell->lDifferentValues+1,ptTestCell->ldSamplesValidExecs);
                StdfPatCompliance_LogFileDatasetError(ptTestCell,m_pFileAudit->bShowHistogram,"Integers",strText);
                goto NextTestCell;
            }

            // Only very few values
            if(iShape == PATMAN_LIB_SHAPE_CATEGORY)
            {
                strText.sprintf("Category distribution with only few different values (Only %ld different values for all %d samples)",ptTestCell->lDifferentValues+1,ptTestCell->ldSamplesValidExecs);
                StdfPatCompliance_LogFileDatasetError(ptTestCell,m_pFileAudit->bShowHistogram,"Category",strText);
                goto NextTestCell;
            }
        }


        // Check for: Measurement resolution / test stratification problems
        if(m_pFileAudit->bCheckMeasurementIssue)
        {
            // Only very few values  (less than 10% of classes compared to number of samples): Requires at least 30 samples in file!
            if((ptTestCell->lDifferentValues < 30) && (ptTestCell->lDifferentValues*10 < ptTestCell->ldSamplesValidExecs) && (ptTestCell->ldSamplesValidExecs > 30))
            {
                strText.sprintf("Test Stratification / Measurement resolution issue: values may suffer rounding problem (Only %ld different values for all %d samples)",ptTestCell->lDifferentValues+1,ptTestCell->ldSamplesValidExecs);
                StdfPatCompliance_LogFileDatasetError(ptTestCell,m_pFileAudit->bShowHistogram,"Resolution",strText);
                goto NextTestCell;
            }
        }

        // Check for strange shape:  (eg: Multi-modal)...keep this last as it is not ultra accurate!
        if(m_pFileAudit->bCheckMultiModal)
        {
            // Clear multi modal?
            if((iShape == PATMAN_LIB_SHAPE_BIMODAL) || (iShape == PATMAN_LIB_SHAPE_MULTIMODAL))
            {
                strText.sprintf("Multi-modal distribution. Statistics such as 'mean', 'median' or 'sigma' may be meaningless");
                StdfPatCompliance_LogFileDatasetError(ptTestCell,m_pFileAudit->bShowHistogram,"Multi-Modal",strText);
                goto NextTestCell;
            }

            // Detect very strange distributions (eg: two clear modes, or big gap in center)
            //double lfQ1,lfQ2,lfQ3,lfQ4;
            //patlib_ComputeQuartilePercentage(ptTestCell,&lfQ1,&lfQ2,&lfQ3,&lfQ4);	// Compute space used by each quartile (dropping few percent noise at each extremity
        }

        // Move to next test
        NextTestCell:
        ptTestCell = ptTestCell->GetNextTest();
    };

    // If no errors reported...
    if(m_iTotalLogDatasetErrors == 0)
    {
        // No problem found...display info/warning if total samples is under threshold!
        int iParts = gex_max(m_pGroup->cMergedData.lTotalHardBins,m_pGroup->cMergedData.lTotalSoftBins);
        if((m_pFileAudit->ldSamplesLimit >= 0) && (iParts < m_pFileAudit->ldSamplesLimit))
        {
            strText.sprintf("Very few samples/parts in your file: %d parts. Ideally, at least 30 samples should be available in your file (avoids skewed statistics).",iParts);
            StdfPatCompliance_LogFileDatasetError(NULL,false,"INFO",strText);
        }

        // Status
        StdfPatCompliance_LogFileDatasetError(NULL,false,"SUCCESS","No parameter issue to report");
    }

    // Close error table (if HTML report)
    if(m_bCsvOutput == false)
        fprintf(m_hReportFile,"</table>\n");

    // Force HTML page change if creating regular HTML report
    CheckHtmlPageChange(true);

    // Return: failures detected
    if(m_iTotalLogDatasetErrors)
        return false;
    else
        return true;
}

///////////////////////////////////////////////////////////
// Check all parameters in a given group: report  sites with high shift discrepency.
///////////////////////////////////////////////////////////
bool GexFileAudit::CheckStdfFileSitesCorrelationAudit()
{
    GSLOG(5, "Check Stdf File Sites Correlation Audit");

    // If single site, nothing to test!
    if(gexReport->getGroupsList().count() <= 1)
        return true;

    // Progress bar status line
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("File Audit: Sites variations");
    QCoreApplication::processEvents();

    // Section Title
    WriteSectionTitle("Test Statistics: Site shifts");

    // Open table (if HTML report)
    if(m_bCsvOutput == false)
        fprintf(m_hReportFile,"<table>\n");

    // Compute HTML hyperlink indexes between pages (eg: cross links between Stats & Histograms
    qtTestListStatistics	qtStatisticsList_Stats;
    qtTestListStatistics	qtStatisticsList_Histo;
    qtTestListStatistics	qtStatisticsList_Wafermap;
    gexReport->ComputePagesHyperlinks(&qtStatisticsList_Stats, &qtStatisticsList_Histo, &qtStatisticsList_Wafermap);

    // Creates **** TEST STATISTICS **** pages: maximum of 200 lines per page.
    int iCellIndex = 0;
    QString strOutputFormat=gexReport->getReportOptions()->GetOption("output","format").toString();
    do
    {
        int iStatus = gexReport->CreatePages_Stats(
                &qtStatisticsList_Stats,iCellIndex,true,true,
                200/gexReport->getGroupsList().count());

        // If not full table written, then insert page break (std HTML report only)...
        //if((gexReport->pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML) && (iCellIndex > 0))
        if ( (strOutputFormat=="HTML") && (iCellIndex > 0))
        {
            fprintf(m_hReportFile,"</table>\n");
            // Force HTML page change if creating regular HTML report
            CheckHtmlPageChange(true);
            fprintf(m_hReportFile,"<table>\n");
        }

        if(iStatus != GS::StdLib::Stdf::NoError)
            return false;
    }
    while(iCellIndex >= 0);

    // Close table (if HTML report)
    if(m_bCsvOutput == false)
        fprintf(m_hReportFile,"</table>\n");
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Creates ALL the .CSV or HTML pages for 'File Audit'
/////////////////////////////////////////////////////////////////////////////
int CGexReport::CreatePages_FileAudit(
    GS::Gex::CustomReportFileAuditSection* pFileAudit)
{
    // Progress bar status line
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("File Audit...");
    QCoreApplication::processEvents();

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Audit class instance
    //bool bCsvOutput = (pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV) ? true: false;;
    bool bCsvOutput = (strOutputFormat=="CSV")?true:false;
    bool bCheckPAT = (GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT()) ? true: false;
    GexFileAudit cFileAudit(pFileAudit,hReportFile,bCheckPAT,getGroupsList().count(),bCsvOutput);

    // Dispay header table (file name, product, etc...
    cFileAudit.DisplayHeaderDetails();

    // Insert page break
    WritePageBreak();

    // If PAT enabled, do data analysis
    // Analyze STDF file: check for Data discrepancies.(distributions, etc).
    unsigned uGroupID;
    CGexGroupOfFiles *pGroup;
    CGexFileInGroup *pFile;
    for (uGroupID = 0;
         uGroupID < (unsigned int) getGroupsList().count(); uGroupID++) {
        if(bCheckPAT)
        {
            // Analyze data set in given group
            cFileAudit.CheckStdfFileDatasetAudit(uGroupID);

            // Insert page break between groups
            gexReport->WritePageBreak();
        }
    }

    // If PAT enabled...
    // Check for site-to-site discrepancies (shift exceeding limits).
    if(bCheckPAT && pFileAudit->bCheckSiteCorrelation)
    {
        cFileAudit.CheckStdfFileSitesCorrelationAudit();

        // Insert page break
        gexReport->WritePageBreak();
    }

    // ALL Examinator execute the LOW-Level STDF analysis.
    // Analyze STDF file: check for Records discrepancies.
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    cFileAudit.CheckStdfFileRecordsAudit(pFile->strFileNameSTDF);

    return GS::StdLib::Stdf::NoError;
}


