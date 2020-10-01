#include <QProgressDialog>
#include <QCoreApplication>
#include <gqtl_log.h>

#include "abstract_csv_converter.h"
#include "csv_converter_v1x.h"
#include "csv_converter_v2x.h"
#include "engine.h"

AbstractCsvConverter::AbstractCsvConverter(unsigned int majorVersion, unsigned int minorVersion)
    : mMajorVersion(majorVersion), mMinorVersion(minorVersion)
{
    ptTestList          = NULL;
    ptPinmapList        = NULL;

    m_bSplitExport      = false;
    m_eStdfCompliancy   = STRINGENT;
    m_eMPRMergeMode     = MERGE;
    m_eMPRMergeCriteria = FIRST;
    m_eTestMergeRule    = NEVER_MERGE_TEST;
    m_eSortingField     = SortOnTestID;
    m_eUnitsMode        = UnitsNormalized;
    m_poProgDialog = 0;
    Clear();

    QStringList qslFormatTestName = ReportOptions.GetOption(QString("dataprocessing"), QString("format_test_name"))
                                    .toString().split(QString("|"));

    m_bRemoveSeqName = qslFormatTestName.contains(QString("remove_sequencer_name"));
    m_bRemovePinName = qslFormatTestName.contains(QString("remove_pin_name"));

    // Set number formatting options
    int lPrecision = ReportOptions.GetOption("toolbox", "precision").toInt();
    mNumberFormat.setOptions('g', lPrecision, false);
}

AbstractCsvConverter::~AbstractCsvConverter()
{

}

AbstractCsvConverter * AbstractCsvConverter::CreateConverter(int majorVersion, int minorVersion)
{
    if (majorVersion == GALAXY_CSV_VERSION_V1_MAJOR && minorVersion == 0)
        return new CsvConverterV1x();
    else if (majorVersion == GALAXY_CSV_VERSION_V2_MAJOR)
    {
        if (minorVersion == 0)
            return new CsvConverterV2x(CsvConverterV2x::CsvV2Minor0);
        else if (minorVersion == 1)
            return new CsvConverterV2x(CsvConverterV2x::CsvV2Minor1);
    }

    return NULL;
}

bool AbstractCsvConverter::UpdateOptions(CReportOptions * pReportOptions)
{
    if (pReportOptions)
    {
        bool ok = true;	// set this to false if you detect an GetOption() error
        //
        QString sc = pReportOptions->GetOption("dataprocessing", "stdf_compliancy").toString();

        if (sc == "stringent")
            m_eStdfCompliancy = AbstractCsvConverter::STRINGENT;
        else if (sc == "flexible")
            m_eStdfCompliancy = AbstractCsvConverter::FLEXIBLE;
        else
        {
            GSLOG(SYSLOG_SEV_WARNING,
                   QString("Unknown option value '%1' for option (dataprocessing, stdf_compliancy)")
                   .arg(sc).toLatin1().data() );
            ok = false;
        }

        QString mm = pReportOptions->GetOption("dataprocessing","multi_parametric_merge_mode").toString();
        if (mm == "merge")
            m_eMPRMergeMode = AbstractCsvConverter::MERGE;
        else if (mm == "no_merge")
            m_eMPRMergeMode = AbstractCsvConverter::NO_MERGE;
        else
        {
            ok = false;
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unknown option value '%1' for option (dataprocessing,multi_parametric_merge_mode)")
                     .arg(mm).toLatin1().data());
        }

        QString mc = pReportOptions->GetOption("dataprocessing","multi_parametric_merge_criteria").toString();
        if (mc == "min")
            m_eMPRMergeCriteria = AbstractCsvConverter::MIN;
        else if (mc == "max")
            m_eMPRMergeCriteria = AbstractCsvConverter::MAX;
        else if (mc == "mean")
            m_eMPRMergeCriteria = AbstractCsvConverter::MEAN;
        else if (mc == "median")
            m_eMPRMergeCriteria = AbstractCsvConverter::MEDIAN;
        else if (mc == "first")
            m_eMPRMergeCriteria = AbstractCsvConverter::FIRST;
        else if (mc == "last")
            m_eMPRMergeCriteria = AbstractCsvConverter::LAST;
        else
        {
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Unknown option value '%1' for (dataprocessing,multi_parametric_merge_criteria)")
                     .arg(mc).toLatin1().data() );
            ok = false;
        }

        // Sorting field
        QString strSortingField = pReportOptions->GetOption("toolbox", "csv_sorting").toString();

        if (strSortingField == "test_id")
            m_eSortingField = SortOnTestID;
        else if (strSortingField == "flow_id")
            m_eSortingField = SortOnFlowID;
        else
        {
            ok = false;
        }

        // Duplicate test
        QString strTestMergeRule = pReportOptions->GetOption("dataprocessing","duplicate_test").toString();
        if (strTestMergeRule == "merge")
            m_eTestMergeRule = AbstractCsvConverter::MERGE_TEST_NUMBER;
        else if (strTestMergeRule == "merge_name")
            m_eTestMergeRule = AbstractCsvConverter::MERGE_TEST_NAME;
        else if (strTestMergeRule == "no_merge")
            m_eTestMergeRule = AbstractCsvConverter::NEVER_MERGE_TEST;
        else
        {
            ok = false;
            GSLOG(SYSLOG_SEV_WARNING, QString("unknown option value '%1' for (dataprocessing,duplicate_test)")
              .arg(strTestMergeRule).toLatin1().data());
        }

        // Units Mode
        QString strUnits = pReportOptions->GetOption("toolbox", "csv_units_mode").toString();

        if (strUnits == "normalized")
            m_eUnitsMode = UnitsNormalized;
        else if (strUnits == "scaling_factor")
            m_eUnitsMode = UnitsScalingFactor;
        else
        {
            GSLOG(SYSLOG_SEV_WARNING, QString("error : unknown option value '%1' for (toolbox,csv_units_mode)")
                     .arg(strUnits).toLatin1().data());
            ok = false;
        }

        return ok;
    }

    return false;
}
//////////////////////////////////////////////////////////////////////
// Clear variables
//////////////////////////////////////////////////////////////////////
void AbstractCsvConverter::Clear(void)
{
    // Reset variables
    m_bIsResultsHeaderWritten = false;  // set to 'true' when the result header is written
    iLastError				= errNoError;
    PartInfo.lExecutionTime = 0;
    PartInfo.lPartNumber	= 0;
    m_FlexFormatMultiSites	= false;
    m_nFlowID				= 0;

    // Clear some fields
    m_cFileData.clear();

    // Reset list of sites detected in data file
    m_cSitesUsed.clear();

    // Reset test flow list
    m_cFullTestFlow.clear();

    // Reset channel names
    mChannelNames.clear();

    CShortTest *ptCell;
    while(ptTestList != NULL)
    {
        ptCell = ptTestList->ptNextTest;
        delete ptTestList;
        ptTestList = ptCell;
    };

    CPinmap * ptPinmap;
    while(ptPinmapList != NULL)
    {
        ptPinmap = ptPinmapList->ptNextPinmap;
        delete ptPinmapList;
        ptPinmapList = ptPinmap;
    };

    m_mapVariableValue.clear();
    m_mapVariableSeq.clear();
    m_cRecordCount.clear();
}

bool AbstractCsvConverter::Convert(const QString& stdfFileName, const QString& csvFileName)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Convert '%1' to csv").arg(stdfFileName).toLatin1().data() );

    // Reset variables
    Clear();

    if (!UpdateOptions(&ReportOptions))
        GSLOG(4, "Failed to update options !");

    // If CSV file already exists...do not rebuild it!
    QFile f(csvFileName);
    if(f.exists() == true)
        return true;

    // Open CSV Parameter table file
    if(!f.open(QIODevice::WriteOnly))
    {
        // Error. Can't create CSV file!
        iLastError = errOpenWriteFail;
        return false;
    }

    // Assign file I/O stream
    hCsvTableFile.setDevice(&f);

    double dFileSize = StdfFile.GetFileSize(stdfFileName.toLatin1().constData());
    // Read all file...and build CSV file as we go...
    int		iStatus;
    for(lPass = 1;lPass <=2; lPass++)
    {
        // Open STDF file to read...
        iStatus = StdfFile.Open(stdfFileName.toLatin1().data(), STDF_READ, 1000000L);
        if(m_poProgDialog)
        {
            m_poProgDialog->setMinimum(0);
            m_poProgDialog->setMaximum(100);
            m_poProgDialog->setLabelText(
              QString("Processing file : %1\n(%2/%3)")
                .arg(stdfFileName.toLatin1().constData())
                .arg(m_iFileNumber).arg(m_iFileCount));
            m_poProgDialog->setWindowTitle(
              GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
            m_poProgDialog->show();

            if(m_poProgDialog->wasCanceled())
            {
                StdfFile.Close();
                break;
            }
        }
        if(iStatus != GS::StdLib::Stdf::NoError)
        {
            // Error. Can't open STDF file in read mode!
            iLastError = errOpenReadFail;
            StdfFile.Close();	// Clean close.
            f.close();
            return false;
        }

        // Read one record from STDF file.
        iStatus = StdfFile.LoadRecord(&StdfRecordHeader);
        m_cRecordCount.clear();
        while(iStatus == GS::StdLib::Stdf::NoError)
        {
            if(m_poProgDialog)
            {
                double dProgress = 100 * (( StdfFile.GetPos() + (dFileSize*(lPass-1)) )/(2*dFileSize));
                int iProgressValue = 0;
                iProgressValue = static_cast<int>(dProgress);

                if(m_poProgDialog->value() != iProgressValue)
                {
                    m_poProgDialog->setValue(iProgressValue);
                    if(m_poProgDialog->wasCanceled())
                        break;
                }
                QCoreApplication::processEvents();
            }

            // set Record count
            setRecordCount(StdfRecordHeader.iRecordType,StdfRecordHeader.iRecordSubType);

            // Process STDF record read.
            switch(StdfRecordHeader.iRecordType)
            {
                case 1:
                    switch(StdfRecordHeader.iRecordSubType)
                    {
                        case 10:// Process MIR records... Type = 1:10
                            if(m_cRecordCount.contains("MIR")) m_cRecordCount["MIR"]=0;
                            if(ProcessMIR() != true)
                            {
                                StdfFile.Close();	// Clean close.
                                f.close();
                                return false;	// File timestamp is invalid, and higher than expiration date!
                            }
                            break;
                        case 20:// Process MRR records... Type = 1:20
                            ProcessMRR();
                            break;
                        case 40:// Process HBR records... Type = 1:40
                            ProcessHBR();
                            break;
                        case 50:// Process SBR records... Type = 1:50
                            ProcessSBR();
                            break;
                        case 60:// Process PMR records... Type = 1:60
                            ProcessPMR();
                            break;
                        case 80:// Process SDR records... Type = 1:80
                            ProcessSDR();
                            break;
                        default:
                            break;
                    }
                    break;

                case 2:
                    switch(StdfRecordHeader.iRecordSubType)
                    {
                        case 10:// Process WIR records... Type = 2:10
                            ProcessWIR();
                            break;
                        case 30:// Process WCR records... Type = 2:30
                            ProcessWCR();
                            break;
                        default:
                            break;
                    }
                    break;

                case 5:
                    switch(StdfRecordHeader.iRecordSubType)
                    {
                        case 10:// Process PIR records... Type = 5:10
                            ProcessPIR();
                            break;
                        case 20:// Process PRR records... Type = 5:20
                            ProcessPRR();
                            break;
                        default:
                            break;
                    }
                    break;

                case 10:
                    switch(StdfRecordHeader.iRecordSubType)
                    {
                        case 10:// Process PDR records... Type = 10:10 ==== STDF V3 only.
                            ProcessPDR();
                            break;
                        case 20:// Process FDR records... Type = 10:20 ==== STDF V3 only.
                            ProcessFDR();
                            break;
                        case 30:// Process TSR records... Type = 10:30
                            ProcessTSR();
                            break;
                        default:
                            break;
                    }
                    break;

                case 15:
                    switch(StdfRecordHeader.iRecordSubType)
                    {
                        case 10:// Process PTR records... Type = 15:10
                            ProcessPTR();
                            break;
                        case 15:// Process MPR records... Type = 15:15
                            ProcessMPR();
                            break;
                        case 20:// Process FTR records... Type = 15:20
                            ProcessFTR();
                            break;
                        default:
                            break;
                    }
                    break;

                case 50:
                    switch(StdfRecordHeader.iRecordSubType)
                    {
                        case 30:// Process DTR records... Type = 50:30
                            ProcessDTR();
                            break;

                        case 10:// Process GDR records... Type = 50:10
                            ProcessGDR();
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }

            // Read one record from STDF file.
            iStatus = StdfFile.LoadRecord(&StdfRecordHeader);
        };

        // Close input STDF file between passes.
        StdfFile.Close();
    }	// 2 passes.

    // Convertion successful
    f.close();
    if(m_poProgDialog &&m_poProgDialog->wasCanceled())
    {
        QFile::remove(csvFileName);
        return false;
    }

    return true;
}

QString AbstractCsvConverter::GetLastError() const
{
    QString strLastError = "Export CSV: ";

    switch(iLastError)
    {
        default:
        case errNoError:
            strLastError += "No Error";
            break;
        case errOpenReadFail:
            strLastError += "Failed to open file";
            break;
        case errOpenWriteFail:
            strLastError += "Failed to create CSV data file";
            break;
        case errInvalidFormatParameter:
            strLastError += "Invalid file format: Didn't find 'Parameter' line\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
            break;
        case errInvalidFormatLowInRows:
            strLastError += "Invalid file format: 'Parameter' line too short, missing rows\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
            break;
        case errInvalidFormatMissingUnit:
            strLastError += "Invalid file format: 'Unit' line missing\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
            break;
        case errInvalidFormatMissingUSL:
            strLastError += "Invalid file format: 'USL' line missing\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
            break;
        case errInvalidFormatMissingLSL:
            strLastError += "Invalid file format: 'LSL' line missing\n\nCheck the Examinator 'Help' page tab to review\nthe 'Input formats' section and read the .CSV or PCM\nformat specifications supported";
            break;
        case errWriteSTDF:
            strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            strLastError += "License has expired or Data file out of date...";
            break;
    }

    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// // Formats test name (in case too int, or leading spaces)..only take non-empty strings (since at init. time it is already set to empty)
//////////////////////////////////////////////////////////////////////
QString AbstractCsvConverter::FormatTestName(const char * szString) const
{
    QString strTestName(szString);

    strTestName.replace(",", ";");

    return strTestName;
}

/////////////////////////////////////////////////////////////////////////////
// Read string from STDF MIR record, save in into relevant MIR buffer
/////////////////////////////////////////////////////////////////////////////
int	AbstractCsvConverter::ReadStringToField(char * szField)
{
    char	szString[257];	// A STDF string is 256 bytes long max!

    *szField=0;

    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return -1;
    // Security: ensures we do not overflow destination buffer !
    szString[MIR_STRING_SIZE-1] = 0;
    strcpy(szField,szString);
    return 1;
}

//////////////////////////////////////////////////////////////////////
// Build the test name string
//////////////////////////////////////////////////////////////////////
QString AbstractCsvConverter::buildTestNameString(CShortTest * ptTestCell)
{
    QString strTestName = ptTestCell->strTestName;

    switch(ptTestCell->lPinmapIndex)
    {
    case GEX_FTEST	:	// Functional test
    case GEX_PTEST	:	// Standard Parametric test: no PinmapIndex#
    case GEX_MPTEST	:	// Multi-result parametric test,...but no PinmapIndex defined!
        break;

    default			:
    {
        CPinmap *ptPinmapCell;
        int lPinIndex = -1;
        // If MPR test found find master test where the pin map index is stored
        if (ptTestCell->lPinmapIndex >= 0)
        {
            CShortTest *lMasterTest = NULL;
            int lResult = FindTestCell(ptTestCell->lTestNumber, GEX_MPTEST, &lMasterTest, false, false);
            if (lResult == -1)
            {
                GSLOG(SYSLOG_SEV_ERROR, "Failed to find test cell");
                return QString();
            }
            if (lMasterTest->mResultArrayIndexes.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR, "Unable to find pin map index in master test");
                return QString();
            }
            lPinIndex = lMasterTest->mResultArrayIndexes.first()[ptTestCell->lPinmapIndex];
        }
        else
            lPinIndex = ptTestCell->lPinmapIndex;

        if(FindPinmapCell(&ptPinmapCell, lPinIndex) == 1)
        {
            // Found pinmap details...show them!
            if(ptPinmapCell->strChannelName.isEmpty() == false)
            {
                // Channel name exists, add it to string
                strTestName += " - ";
                strTestName += ptPinmapCell->strChannelName;
            }
            else if(ptPinmapCell->strLogicName.isEmpty() == false)
            {
                // Logical pin name exists, add it to string
                strTestName += " - ";
                strTestName += ptPinmapCell->strLogicName;
            }
        }
        break;
    }
    }

    return strTestName;
}

/////////////////////////////////////////////////////////////////////////////
// Find Test Cell structure in Test List.
/////////////////////////////////////////////////////////////////////////////
int	AbstractCsvConverter::FindTestCell(unsigned int lTestNumber,int lPinmapIndex, CShortTest **ptTestCellFound,BOOL bCreateIfNew,BOOL bResetList, const QString& strNameInit /*= QString()*/)
{
    static CShortTest * ptPrevCell;
    static CShortTest * ptCellFound;
    CShortTest *        ptTestCell;
    CShortTest *        ptNewCell;
    QString strName = strNameInit;

    formatSequencerAndPinTestName(strName);
    if (m_eTestMergeRule == MERGE_TEST_NAME)
        lTestNumber = findMappedTestName((lPinmapIndex > 0) ? GEX_MPTEST : lPinmapIndex, lTestNumber, strName);
    if(ptTestList == NULL)
    {
        // First test : list is currently empty.
        ptTestList			= new CShortTest;
        ptTestList->lTestNumber		= lTestNumber;
        ptTestList->lPinmapIndex	= lPinmapIndex;
        ptTestList->strTestName		= strName;
        ptTestList->m_nTestFlowID	= ++m_nFlowID;
        ptTestList->ptNextTest		= NULL;
        *ptTestCellFound		= ptTestList;
        ptCellFound			= ptTestList;
        ptPrevCell			= NULL;

        return 1;	// Success
    }

    if(bResetList)
        goto rewind_list;

    // Loop until test cell found, or end of list reached.
    ptTestCell = ptCellFound;

    // Check if starting cell has a test already higher than the one we look for...
    //if(ptTestCell->lTestNumber > lTestNumber) PYC, case 4732, 06/05/2011
    if(ptTestCell == NULL || ptTestCell->lTestNumber > lTestNumber || (ptTestCell->lTestNumber == lTestNumber && ptTestCell->lPinmapIndex > lPinmapIndex) ||
        (ptTestCell->lTestNumber == lTestNumber && ptTestCell->lPinmapIndex == lPinmapIndex && ptTestCell->strTestName.compare(strName) > 0))
        goto rewind_list;	// We need to start from the beginnnig of the list!

    while(ptTestCell != NULL)
    {
        if(ptTestCell->lTestNumber > lTestNumber)
        {
            switch(lPinmapIndex)
            {
            case GEX_PTEST:	// This test is not a Multiple-resut parametric...so create test!
            case GEX_FTEST:	// Functional test
                goto create_test;

            case GEX_MPTEST: // Multiple-result parametric test...but no PinmapIndex
            default:	// Multiple-result parametric test...we need to look for
                goto rewind_list;
            }
        }
        else
            if(ptTestCell->lTestNumber == lTestNumber)
            {
                switch(lPinmapIndex)
                {
                case GEX_PTEST:	// This test is not a Multiple-resut parametric...
                case GEX_FTEST:	// Functional test

                    if (ptTestCell->lPinmapIndex > lPinmapIndex)
                        goto create_test;

                    // Test entry of a different type (eg: Functiona vs Parametric)
                    if(ptTestCell->lPinmapIndex == lPinmapIndex)
                    {
                        // If searching for a given Name
                        if(strName.isEmpty() == true || m_eTestMergeRule == MERGE_TEST_NUMBER || ptTestCell->strTestName == strName)
                        {
                            *ptTestCellFound	= ptTestCell;
                            ptCellFound			= ptTestCell;

                            return 1; // Test+PinmapIndex# found, pointer to it returned.
                        }
                        else if (ptTestCell->strTestName.compare(strName) > 0)
                            goto create_test;
                    }

                    break;

                case GEX_MPTEST: // Multiple-result parametric test...but no PinmapIndex
                default:	// Multiple-result parametric test...we need to look for
                    // the test record ALSO matching the PinmapIndex# !
                    while(ptTestCell != NULL)
                    {
                        // If gone too far (passed test cell or PinmapIndex #) rewind list!
                        if((ptTestCell->lTestNumber > lTestNumber) || (ptTestCell->lPinmapIndex > lPinmapIndex))
                            goto rewind_list;
                        else
                            if(ptTestCell->lPinmapIndex == lPinmapIndex)
                            {
                                // If searching for a given Name
                                if(strName.isEmpty() == true || m_eTestMergeRule == MERGE_TEST_NUMBER || ptTestCell->strTestName == strName)
                                {
                                    *ptTestCellFound = ptTestCell;
                                    ptCellFound = ptTestCell;
                                    return 1; // Test+PinmapIndex# found, pointer to it returned.
                                }
                                else if (ptTestCell->strTestName.compare(strName) > 0)
                                    goto create_test;
                            }

                        ptPrevCell = ptTestCell;
                        ptTestCell = ptTestCell->ptNextTest;
                    };
                    // Reached the end of list without finding Test#+PinmapIndex#
                    goto rewind_list;
                }
            }
        ptPrevCell = ptTestCell;
        ptTestCell = ptTestCell->ptNextTest;
    };

    // We have scanned the list without finding the test...so we may have to create it! (unless it is a MP test)
    if(lPinmapIndex == GEX_PTEST || lPinmapIndex == GEX_FTEST)
        goto create_test;

    rewind_list:
        // Start from first test in list
        ptTestCell = ptTestList;
    ptCellFound = ptTestList;
    ptPrevCell = NULL;
    // Check from the beginning of the list if can find the test.
    while(ptTestCell != NULL)
    {
        if(ptTestCell->lTestNumber > lTestNumber)
            goto create_test;
        else
            if(ptTestCell->lTestNumber == lTestNumber)
            {
                switch(lPinmapIndex)
                {
                case GEX_PTEST:	// This test is not a Multiple-resut parametric...
                case GEX_FTEST:	// Functional test

                    // PYC, case 4732, 06/05/2011
                    if (ptTestCell->lPinmapIndex > lPinmapIndex)
                        goto create_test;


                    // Test entry of a different type (eg: Functiona vs Parametric)
                    if(ptTestCell->lPinmapIndex == lPinmapIndex)
                    {
                        // If searching for a given Name
                        if(strName.isEmpty() == true || m_eTestMergeRule == MERGE_TEST_NUMBER || ptTestCell->strTestName == strName)
                        {
                            *ptTestCellFound	= ptTestCell;
                            ptCellFound			= ptTestCell;

                            return 1; // Test+PinmapIndex# found, pointer to it returned.
                        }
                        else if (ptTestCell->strTestName.compare(strName) > 0)
                            goto create_test;
                    }

                    break;

                case GEX_MPTEST: // Multiple-result parametric test...but no PinmapIndex
                default:	// Multiple-result parametric test...we need to look for
                    // the test record ALSO matching the PinmapIndex# !
                    while(ptTestCell != NULL)
                    {
                        if((ptTestCell->lTestNumber > lTestNumber) || (ptTestCell->lPinmapIndex > lPinmapIndex))
                            goto create_test;
                        else
                            if(ptTestCell->lPinmapIndex == lPinmapIndex)
                            {
                                // If searching for a given Name
                                if(strName.isEmpty() == true || m_eTestMergeRule == MERGE_TEST_NUMBER || ptTestCell->strTestName == strName)
                                {
                                    *ptTestCellFound = ptTestCell;
                                    ptCellFound = ptTestCell;
                                    return 1; // Test+PinmapIndex# found, pointer to it returned.
                                }
                                else if (ptTestCell->strTestName.compare(strName) > 0)
                                    goto create_test;
                            }
                        ptPrevCell = ptTestCell;
                        ptTestCell = ptTestCell->ptNextTest;
                    };
                    // Reached the end of list AGAIN without finding Test#+PinmapIndex#...so append it to current list
                    goto create_test;
                }
            }
        ptPrevCell = ptTestCell;
        ptTestCell = ptTestCell->ptNextTest;
    };

    create_test:
        if(bCreateIfNew == false)
      return 0;	// Test not in current list...

    // Test not in list: insert in list.
    ptCellFound = ptNewCell		= new CShortTest;
    ptNewCell->lTestNumber		= lTestNumber;
    ptNewCell->lPinmapIndex		= lPinmapIndex;
    ptNewCell->strTestName		= strName;
    ptNewCell->m_nTestFlowID	= ++m_nFlowID;
    ptNewCell->ptNextTest		= NULL;

    if(ptPrevCell == NULL)
    {
        // This cell becomes head of list
        ptNewCell->ptNextTest = ptTestList;
        ptTestList = ptNewCell;
    }
    else
    {
        // Insert cell in list
        ptPrevCell->ptNextTest = ptNewCell;
        ptNewCell->ptNextTest  = ptTestCell;
    }

    *ptTestCellFound = ptNewCell;
    return 1;	// Success
}

/////////////////////////////////////////////////////////////////////////////
// Find and/or create Pinmap entry in the Pinmap List.
/////////////////////////////////////////////////////////////////////////////
int	AbstractCsvConverter::FindPinmapCell(CPinmap **ptPinmapCellFound,int iPinmapIndex)
{
    CPinmap *   ptNewCell           = NULL;
    CPinmap *   ptPinmapCell        = NULL;
    CPinmap *   ptPrevPinmapCell    = NULL;

    // Pinmap must be a positive number!
    if(iPinmapIndex < 0)
        return -1;

    if(ptPinmapList == NULL)
    {
        // First Pinmap cell : list is currently empty.
        ptNewCell = ptPinmapList = new CPinmap;
        ptNewCell->iPinmapIndex = iPinmapIndex;
        ptPinmapList->ptNextPinmap  = NULL;
        *ptPinmapCellFound = ptPinmapList;	// Pointer to cell just created
        return 1;	// Success
    }

    // Loop until test cell found, or end of list reached.
    ptPinmapCell = ptPinmapList;
    ptPrevPinmapCell = NULL;
    while(ptPinmapCell != NULL)
    {
        if(ptPinmapCell->iPinmapIndex > iPinmapIndex)
            break;
        else
            if(ptPinmapCell->iPinmapIndex == iPinmapIndex)
            {
                // Entry already exists...return pointer to the cell.
                *ptPinmapCellFound = ptPinmapCell;	// pointer to matching pinmap cell
                return 1;
            }
        ptPrevPinmapCell = ptPinmapCell;
        ptPinmapCell = ptPinmapCell->ptNextPinmap;
    };

    // Pinmap not in list: insert in list.
    ptNewCell = new CPinmap;
    ptNewCell->iPinmapIndex = iPinmapIndex;
    ptNewCell->ptNextPinmap  = NULL;

    if(ptPrevPinmapCell == NULL)
    {
        // This cell becomes head of list
        ptNewCell->ptNextPinmap = ptPinmapList;
        ptPinmapList = ptNewCell;
    }
    else
    {
        // Insert cell in list
        ptPrevPinmapCell->ptNextPinmap = ptNewCell;
        ptNewCell->ptNextPinmap  = ptPinmapCell;
    }

    *ptPinmapCellFound = ptNewCell;	// pointer to new pinmap cell created

    return 1;
}

unsigned int AbstractCsvConverter::findMappedTestName(long lTestType, unsigned int nTestNumber, const QString& strTestName)
{
    unsigned int nMappedTestNumber = nTestNumber;

    if (m_testNameMap.contains(lTestType))
    {
        CSVTestNameMappingPrivate& testNameMap = m_testNameMap[lTestType];

        // Find the mapped test number
        nMappedTestNumber = testNameMap.findTestNumber(nTestNumber, strTestName);
    }
    else
    {
        CSVTestNameMappingPrivate testNameMap;

        // Find the mapped test number
        nMappedTestNumber = testNameMap.findTestNumber(nTestNumber, strTestName);

        // Add to the test type map
        m_testNameMap.insert(lTestType, testNameMap);
    }

    return nMappedTestNumber;
}

bool AbstractCsvConverter::ProcessMIR()
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessMIR not implemented" );
    return false;
}

void AbstractCsvConverter::ProcessSDR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessSDR not implemented");
}

void AbstractCsvConverter::ProcessPDR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessPDR not implemented");
}

void AbstractCsvConverter::ProcessFDR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessFDR not implemented");
}

void AbstractCsvConverter::ProcessFTR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessFTR not implemented");
}

void AbstractCsvConverter::ProcessPTR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessPTR not implemented");
}

void AbstractCsvConverter::ProcessMPR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessMPR not implemented");
}

void AbstractCsvConverter::ProcessPIR(void)
{
    // Reset all variables value
    resetVariables();

    // assume that first PIR in pass 2 trigger off result header writting
    if(lPass!=2)
        return;
    else
    {
        if(!m_bIsResultsHeaderWritten)
            WriteResultsHeader();
    }
}


void AbstractCsvConverter::ProcessPRR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessPRR not implemented");
}

void AbstractCsvConverter::ProcessTSR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessTSR not implemented");
}

void AbstractCsvConverter::ProcessWCR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessWCR not implemented");
}

void AbstractCsvConverter::ProcessWIR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessWIR not implemented");
}

void AbstractCsvConverter::ProcessMRR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessMRR not implemented");
}

void AbstractCsvConverter::ProcessPMR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessPMR not implemented");
}

void AbstractCsvConverter::ProcessDTR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessDTR not implemented");
}

void AbstractCsvConverter::ProcessGDR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessGDR not implemented");
}

bool AbstractCsvConverter::ProcessHBR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessHBR not implemented");

    return true;
}

bool AbstractCsvConverter::ProcessSBR(void)
{
    GSLOG(SYSLOG_SEV_NOTICE, "ProcessSBR not implemented");

    return true;
}

unsigned int AbstractCsvConverter::CSVTestNameMappingPrivate::findTestNumber(unsigned int nTestNumber, const QString& strTestName)
{
    unsigned int nMappedTestNumber = nTestNumber;

    // Test name specified, see if already in Name list
    if(!strTestName.isEmpty() && !strTestName.isNull())
    {
        if(m_mapTestName.contains(strTestName) == false)
        {
            // No....so add entry!
            m_mapTestName.insert(strTestName, nTestNumber);

            // Test# mapping.
            m_mapTestNumber.insert(nTestNumber, nTestNumber);
        }
        else
        {
            // This test name already in list, get its root test# assigned to it.
            nMappedTestNumber = m_mapTestName.value(strTestName);
            m_mapTestNumber.insert(nTestNumber, nMappedTestNumber);
        }
    }
    else
    {
        if (m_mapTestNumber.contains(nTestNumber) == false)
            nMappedTestNumber = nTestNumber;
        else
            // No test name....specified (run#2 or higher....)....get root test#
            nMappedTestNumber = m_mapTestNumber.value(nTestNumber);
    }

    return nMappedTestNumber;
}


void AbstractCsvConverter::setProgressDialog(QProgressDialog* poProgDialog, int iFileNumber, int iFileCount)
{
    m_poProgDialog = poProgDialog;
    m_iFileNumber = iFileNumber;
    m_iFileCount = iFileCount;
}

void AbstractCsvConverter::formatSequencerAndPinTestName(QString &strTestName)
{
    QString strString;
    QString strName;
    QString strSequencerName;

    if(strTestName.contains("<>"))
    {
        strSequencerName    = strTestName.section("<>",1).trimmed();
        strName             = strTestName.section("<>",0,0).trimmed();
    }
    else
        strName = strTestName;

    // Check if this is a Flex/J750 data file (if so, we MAY need to remove leading pin# in test names!)
    if(m_bRemovePinName)
    {
        // if PMR have been loaded from datalog, remove channel names from test names only when they
        // are present in PMR
        if (mChannelNames.isEmpty() == false)
        {
            QStringList lNewWords;

            // Split the test name to capture each word (separator is space)
            QStringList lWords = strName.split(" ");

            // Keep only words which are not channel names.
            for (int lIdx = 0; lIdx < lWords.count(); ++lIdx)
            {
                if (mChannelNames.contains(lWords.at(lIdx)) == false)
                    lNewWords.append(lWords.at(lIdx));
            }

            strName = lNewWords.join(" ");
        }
        else
        {
            // If ends with pin#, remove it eg: xxxxx 11.a30
            strString = strName.section(' ',-1,-1);
            if(strString.isEmpty() == false)
            {
                QString lPinName = strString;
                QString lPinPattern = "(\\d+\\.[^\\d]+\\d+|(\\+|-)?\\d+)";
                QRegExp::PatternSyntax lPatternSyntax = QRegExp::RegExp;
                QRegExp  lRegExpPattern("", Qt::CaseInsensitive, lPatternSyntax);
                lRegExpPattern.setPattern(lPinPattern);

                // Check match
                bool bMatch = lRegExpPattern.exactMatch(lPinName);
                if(bMatch)
                {
                    // Drop this pin number !
                    strName.truncate(strName.length() - lPinName.length());
                    strName = strName.trimmed();
                }
            }
        }
    }

    if(!m_bRemoveSeqName && strSequencerName.isEmpty() == false)
        strTestName = strName + " <> " + strSequencerName;
    else
        strTestName = strName;
}

int AbstractCsvConverter::setRecordCount(int recordType, int recordSubType)
{
    QString key = QString::number(recordType)+":"+QString::number(recordSubType);

    if(!m_cRecordCount.contains(key))
        m_cRecordCount[key] = 0;

    m_cRecordCount[key]++;

    return m_cRecordCount[key];
}

int AbstractCsvConverter::getRecordCount(int recordType, int recordSubType)
{
    QString key = QString::number(recordType)+":"+QString::number(recordSubType);

    if(!m_cRecordCount.contains(key))
        return 0;

    return m_cRecordCount[key];
}
